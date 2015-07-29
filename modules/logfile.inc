<?php
#==============================================================================
#
# File:         /var/vermont/modules/logfile.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for reading logfiles and alerting when matches are met
#
# Author:       Joe Honton © 2011
#
# Initial date: January 3, 2011
#
#
#     Keyword               Value
# ---------------	-----------------------
# logformat         one of these: 'syslog', 'iptables-log', apache-access-log', 'apache-error-log'
# outputlevel		either 'verbose' to show all loglines that match, or 'terse' to show a count
#                   of how many loglines matched, or 'verbose|terse' to show both; defaults to 'verbose'.
# message           the message to display if all matches are met, common to all rulesets
# messageN          the message to display if all matches are met
# match             specifies a common rule which will be applied to all rulesets ('match0' is an alias for 'match')
# matchN            specifies which lines will trigger an alert
#                   something like: 'match1', 'match2', 'match3', etc.; where the trailing number specifies a ruleset and must be a number from 1 to 9
#                   matches can be concatented, in which case _all_ matches of the ruleset must be met
# ignore            specifies a common exception to the normal rule which will be applied to all rulesets ('ignore0' is an alias for 'ignore')
# ignoreN           specifies exceptions to the normal rule; the exception supresses the triggering of an alert
#                   ignore exceptions can be concatenated, in which case _any_ exception will supress the trigger
#
#                   Matches and ignores are composed of three parts: a column name, an operator, and a value, where:
#                      column name is a column identifier, defined by the logformat, for example the syslog format defines columns: timestamp, host, program, pid and message
#                      operator is one of these: EQ, NE, LT, LE, GT, GE, RX, NR, LK, NL
#                        (where 'RX' means Regular Expression, 'NR' is the inverse of RX, 'LK' is LIKE, and 'NL' is NOT LIKE. )
#                      value is the value to compare against the specifed column name using the specified operator.
#
# Example of a ruleset:
#
# type = logfile
# logformat = syslog
# match1  = timestamp GT 2*24*60*60		# last two days
# match1 += message EQ SSH
# ignore1 = host EQ 173.11.93.101
#
#==============================================================================


class vmtLogfile extends vmtGlob
{
	protected $rulesets = array();		// an associative array of vmtRuleset objects, keyed with a $ruleset_name like 'ruleset1' through 'ruleset9'
	
	//-------------------------------------------
	public function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['logformat'] = 'required';
		$this->vocabulary['outputlevel'] = 'optional';
		
		$this->logformat = property_exists($this->kv, 'logformat') ? $this->kv->logformat : 'undeclared';
		$this->outputlevel = property_exists($this->kv, 'outputlevel') ? $this->kv->outputlevel : 'verbose';
		
		global $logfile_module_classes;
		if ( !array_key_exists( $this->logformat, $logfile_module_classes ) )
			return $this->specification_error( __METHOD__, 'logformat', 'Unknown logformat' );
		
		// loop over all keyword_value pairs specified by the user looking for 'message1', 'match1', 'ignore1', etc.
		foreach( $this->kv as $keyword => $value )
		{
			if ( strpos( $keyword, 'message' ) === 0 )
			{
				$ruleset = substr( $keyword, strlen('message') );
				if ( $ruleset == '' ) $ruleset = 0;		// the message, which applies to all rulesets
				if ( $ruleset >= 0 && $ruleset <= 9 )
				{
					$parts = explode( '|', $value );
					foreach( $parts as $one_message )
						$this->add_message_to_ruleset( $keyword, $ruleset, $one_message );
				}
			}
			else if ( strpos( $keyword, 'match' ) === 0 )
			{
				$ruleset = substr( $keyword, strlen('match') );
				if ( $ruleset == '' ) $ruleset = 0;		// the 'common rule', which applies to all rulesets
				if ( $ruleset >= 0 && $ruleset <= 9 )
				{
					$parts = explode( '|', $value );
					foreach( $parts as $one_rule )
						$this->add_match_to_ruleset( $keyword, $ruleset, $one_rule );
				}
			}
			else if ( strpos( $keyword, 'ignore' ) === 0 )
			{
				$ruleset = substr( $keyword, strlen('ignore') );
				if ( $ruleset == '' ) $ruleset = 0;		// the 'common rule', which applies to all rulesets
				if ( $ruleset >= 0 && $ruleset <= 9 )
				{
					$parts = explode( '|', $value );
					foreach( $parts as $one_rule )
						$this->add_ignore_to_ruleset( $keyword, $ruleset, $one_rule );
				}
			}
		}
		
		// now take the matches and ignores from the common 'ruleset0'
		// and merge them into each of the 'ruleset1' through 'ruleset9'
		if ( array_key_exists( 'ruleset0', $this->rulesets ) )
		{
			$ruleset0 = $this->rulesets['ruleset0'];
			
			foreach( $this->rulesets as $ruleset_name => $one_ruleset )
			{
				if ( $ruleset_name == 'ruleset0' )	// don't merge into iteslf
					continue;

				if ( $ruleset0->message_template != '' )
					$one_ruleset->message_template = $ruleset0->message_template . ' ' . $one_ruleset->message_template;

				$one_ruleset->matches = array_merge( $ruleset0->matches, $one_ruleset->matches );
				$one_ruleset->ignores = array_merge( $ruleset0->ignores, $one_ruleset->ignores );
			}
		}
		
		// if any ruleset consists of a message template only, and has no match or ignore conditions,
		// it should be removed because everything will match.
		foreach( $this->rulesets as $ruleset_name => $one_ruleset )
		{
			if ( $ruleset_name == 'ruleset0' )
				continue;

			if ( count($one_ruleset->matches) == 0 && count($one_ruleset->ignores) == 0 )
				unset( $this->rulesets[$ruleset_name] );
		}
		
	}

	//-------------------------------------------
	// The pre_perform function overrides the vmtModule default to scan for 'match' and 'ignore'
	// return true to continue, false to halt
	public function pre_perform()
	{
		// look for unrecognized keywords and issue warning
		foreach( $this->kv as $keyword => $value )
		{
			if ( !array_key_exists($keyword, $this->vocabulary) )
			{
				// 'message0' through 'message9'
				if ( strpos( $keyword, 'message' ) === 0 )
				{
					$ruleset = substr( $keyword, strlen('message') );
					if ( $ruleset == '' ) $ruleset = 0;		// the message, which applies to all rulesets
					if ( $ruleset < 0 || $ruleset > 9 )
						$this->vocabulary_warning( $keyword );
				}
				// 'match0' through 'match9'
				else if ( strpos( $keyword, 'match' ) === 0 )
				{
					$ruleset = substr( $keyword, strlen('match') );
					if ( $ruleset == '' ) $ruleset = 0;		// the 'common rule', which applies to all rulesets
					if ( $ruleset < 0 || $ruleset > 9 )
						$this->vocabulary_warning( $keyword );
				}
				// 'ignore0' through 'ignore9'
				else if ( strpos( $keyword, 'ignore' ) === 0 )
				{
					$ruleset = substr( $keyword, strlen('ignore') );
					if ( $ruleset == '' ) $ruleset = 0;		// the 'common rule', which applies to all rulesets
					if ( $ruleset < 0 || $ruleset > 9 )
						$this->vocabulary_warning( $keyword );
				}
				else
					$this->vocabulary_warning( $keyword );
			}
		}
		
		// look for missing required keywords and issue error
		foreach( $this->vocabulary as $keyword => $value )
		{
			if ( $value == 'required' && !array_key_exists($keyword, $this->kv) )
				$this->vocabulary_error( $keyword );
		}
		
		return $this->vocabulary_status;
	}
		
	//-------------------------------------------
	//^ The add_message_to_ruleset function
	//> $keyword is 'message1' through 'message9'
	//> $ruleset_number is a number from 1 to 9
	//> $string is the message, with possibly embedded dollar-sign variables, to display upon trigger
	protected function add_message_to_ruleset( $keyword, $ruleset, $string )
	{
		$ruleset_name = 'ruleset' . $ruleset;		// so 'message1' --> '1' --> 'ruleset1'
		
		// add a new ruleset to the collection if it doesn't already exist
		if ( !array_key_exists( $ruleset_name, $this->rulesets ) )
			$this->rulesets[$ruleset_name] = new vmtRuleset( $ruleset_name );
		
		$this->rulesets[$ruleset_name]->add_message( $string );
	}
	
	//-------------------------------------------
	//^ The add_match_to_ruleset function
	//> $keyword is 'match1' through 'match9'
	//> $ruleset_number is a number from 1 to 9
	//> $unparsed_value is an unparsed value containing a column name, operator, and value
	protected function add_match_to_ruleset( $keyword, $ruleset, $unparsed_value )
	{
		// Construct a new rule object, which will internally parse and verify
		// the unparsed value, storing the column name, operator and value in separate variables.
		$r = new vmtRule( $this, $keyword, $unparsed_value );
		
		$ruleset_name = 'ruleset' . $ruleset;		// so 'match1' --> '1' --> 'ruleset1'
		
		// add a new ruleset to the collection if it doesn't already exist
		if ( !array_key_exists( $ruleset_name, $this->rulesets ) )
			$this->rulesets[$ruleset_name] = new vmtRuleset( $ruleset_name );
		
		// Rulesets are composed of an array of matches and an array of ignores;
		// add this match to the ruleset array
		$this->rulesets[$ruleset_name]->add_match_rule( $r );
	}

	//-------------------------------------------
	//^ The add_ignore_to_ruleset function
	//> $keyword is 'ignore1' through 'ignore9'
	//> $ruleset_number is a number from 1 to 9
	//> $unparsed_value is an unparsed value containing a column name, operator, and value
	protected function add_ignore_to_ruleset( $keyword, $ruleset, $unparsed_value )
	{
		// Construct a new rule object, which will internally parse and verify
		// the unparsed value, storing the column name, operator and value in separate variables.
		$r = new vmtRule( $this, $keyword, $unparsed_value );
		
		$ruleset_name = 'ruleset' . $ruleset;		// so 'match1' --> '1' --> 'ruleset1'
		
		// add a new ruleset to the collection if it doesn't already exist
		if ( !array_key_exists( $ruleset_name, $this->rulesets ) )
			$this->rulesets[$ruleset_name] = new vmtRuleset( $ruleset_name );
		
		// Rulesets are composed of an array of matches and an array of ignores;
		// add this ignore to the ruleset array
		$this->rulesets[$ruleset_name]->add_ignore_rule( $r );
	}
	
	//-------------------------------------------
	// The verify_one_file function does the basic file verification for ownership,
	// permissions, and SELinux context, then does the logfile-specific stuff.
	//
	// This is a vmtGlob derived class override, called by vmtGlob::verify_one_path()
	//
	public function verify_one_file( $one_logfile )
	{
		$this->message_count = 0;		// reset the error message count from any previous file's errors
		
		// first perform all glob file-related checks
		parent::verify_one_file( $one_logfile );
		
		// loop through all files specified in the path keyword
		$this->scan_logfile( trim($one_logfile) );
	}

	//-------------------------------------------
	//^ The scan_logfile function opens the file, reads a line
	//  at a time, parses each line into the defined columns and
	//  sends the column values through the rulesets, before finally
	//  closing the file.
	//> $filename is a fully qualified logfile name
	//< does not return a value
	public function scan_logfile( $filename )
	{
		// construct a parser for the user-specified logformat
		global $logfile_module_classes;
		if ( !array_key_exists( $this->logformat, $logfile_module_classes ) )
			return $this->specification_error( __METHOD__, 'logformat', 'Unknown logformat' );

		$parse_class_name = $logfile_module_classes[$this->logformat];
		$p = new $parse_class_name();
		
		$f = fopen( $filename, 'r' );
		if ( $f == false )
			return $this->runtime_error( __METHOD__, 'path', "unable to read $filename" );

		$line_number = 0;
		$trippoint_count = 0;
		while ( ($line = fgets($f)) !== false )
		{
			$line_number++;
			$line = trim($line);
			
			// parse this line into a dynamic object whose property names are
			// the names of columns and whose property values come from the parsed line.
			$column_values = new vmtParseValues();
			if ( $p->parse( $line, $column_values ) == false )		// skip lines that could not be parsed by the normal rules
				continue;
			
			foreach ( $this->rulesets as $one_ruleset )
			{
				// skip 'ruleset0' which is the common ruleset whose matches and ignores have already been merged into 'ruleset1' through 'ruleset9'
				if ( $one_ruleset->ruleset_name == 'ruleset0' )
					continue;
					
				// apply 'matchN' and 'ignoreN', from 1 to 9
				if ( $this->apply_one_ruleset( $one_ruleset, $column_values ) == false )
				{
					$trippoint_count++;
					if ( strpos($this->outputlevel,'verbose') !== false )
						$this->logfile_match_trigger( $one_ruleset->ruleset_name, $column_values, $filename, $line_number, $line );
					break;
				}
			}
		}
		if ( strpos($this->outputlevel,'terse') !== false && $trippoint_count > 0 )
			$this->logfile_match_terse( $filename, $trippoint_count );
			
		fclose( $f );
	}
		
	//-------------------------------------------
	//^ The apply_one_ruleset function determines if the logfile line that is currently being processed
	//  meets _all_ matches and doesn't meet _any_ ignore.
	//> $ruleset is an object containing a collection of matches and a collection of ignores
	//> $parsed_column_values is an object containing the current line parsed into columns
	//  where the properties of the object are all dynamically created and are named according to
	//  the column names defined by this module's 'logformat'. Because these properties are dynamic
	//  this function uses them as "variable variables".
	//< returns false to trigger an error message
	//
	public function apply_one_ruleset( vmtRuleset $ruleset, vmtParseValues $parsed_column_values )
	{
		$b = $this->are_all_matches_met( $ruleset, $parsed_column_values );
		if ( $b == false )
			return true;
			
		$b = $this->are_any_ignores_met( $ruleset, $parsed_column_values );
		if ( $b == false )
			return false; 	// <-- trigger
		else
			return true;
	}
	
	//-------------------------------------------
	//^ The are_all_matches_met function determines if the logfile line that is currently being processed
	//  meets _all_ matches.
	//> $ruleset is an object containing a collection of matches and a collection of ignores
	//> $parsed_column_values is an object containing the current line parsed into columns
	//  where the properties of the object are all dynamically created and are named according to
	//  the column names defined by this module's 'logformat'. Because these properties are dynamic
	//  this function uses them as "variable variables".
	//< returns true if all matches for _this_ ruleset are met
	//< returns false if one or more matches for _this_ ruleset was not met
	//
	public function are_all_matches_met( vmtRuleset $ruleset, vmtParseValues $parsed_column_values )
	{
		foreach ( $ruleset->matches as $match_rule )
		{
			$rule_name = $match_rule->name;
			$rule_operator = $match_rule->operator;
			$rule_value = $match_rule->value;

			if ( $rule_name == 'undeclared' || $rule_operator == 'undeclared' )
				return false;
			
			// use the column name defined by this rule to lookup the column value parsed from the logfile
			$parsed_value = $parsed_column_values->$rule_name;
			switch ( $rule_operator )
			{
				case 'EQ':
					if ( !($parsed_value == $rule_value) )
						return false;
					break;
					
				case 'NE':
					if ( !($parsed_value != $rule_value) )
						return false;
					break;
						
				case 'LT':
					if ( !($parsed_value < $rule_value) )
						return false;
					break;
										
				case 'LE':
					if ( !($parsed_value <= $rule_value) )
						return false;
					break;
										
				case 'GT':
					if ( !($parsed_value > $rule_value) )
						return false;
					break;
										
				case 'GE';
					if ( !($parsed_value >= $rule_value) )
						return false;
					break;
									
				case 'RX';
				case 'REGEX':
					$rc = preg_match( $rule_value, $parsed_value );
					if ( $rc == 0 )
						return false;
					break;
										
				case 'NR';
				case 'NOTREGEX':
					$rc = preg_match( $rule_value, $parsed_value );
					if ( $rc == 1 )
						return false;
					break;
										
				case 'LK':
				case 'LIKE':
					$pos = strpos( $parsed_value, $rule_value );
					if ( $pos === false )
						return false;
					break;
									
				case 'NL':
				case 'NOTLIKE':
					$pos = strpos( $parsed_value, $rule_value );
					if ( $pos !== false )
						return false;
					break;
										
				default:
					return $this->runtime_error( __METHOD__, '', "Unknown operator '$rule_operator'" );
			}
		}
		
		return true;	// all matches for this ruleset have been met
	}
	
	//-------------------------------------------
	//^ The are_any_ignores_met function determines if the logfile line that is currently being processed
	//  doesn't match _any_ ignore.
	//> $ruleset is an object containing a collection of matches and a collection of ignores
	//> $parsed_column_values is an object containing the current line parsed into columns
	//  where the properties of the object are all dynamically created and are named according to
	//  the column names defined by this module's 'logformat'. Because these properties are dynamic
	//  this function uses them as "variable variables".
	//< returns true if any ignore for _this_ ruleset is matched
	//< returns false if no ignore for _this_ ruleset was matched
	//
	public function are_any_ignores_met( vmtRuleset $ruleset, vmtParseValues $parsed_column_values )
	{
		foreach ( $ruleset->ignores as $ignore_rule )
		{
			$rule_name = $ignore_rule->name;
			$rule_operator = $ignore_rule->operator;
			$rule_value = $ignore_rule->value;
			
			if ( $rule_name == 'undeclared' || $rule_operator == 'undeclared' )
				return true;
				
			// use the column name defined by this rule to lookup the column value parsed from the logfile
			$parsed_value = $parsed_column_values->$rule_name;
			
			switch ( $rule_operator )
			{
				case 'EQ':
					if ( $parsed_value == $rule_value )
						return true;
					break;
					
				case 'NE':
					if ( $parsed_value != $rule_value )
						return true;
					break;
						
				case 'LT':
					if ( $parsed_value < $rule_value )
						return true;
					break;
					
				case 'LE':
					if ( $parsed_value <= $rule_value )
						return true;
					break;
					
				case 'GT':
					if ( $parsed_value > $rule_value )
						return true;
					break;
					
				case 'GE';
					if ( $parsed_value >= $rule_value )
						return true;
					break;
				
				case 'RX';
				case 'REGEX':
					$rc = preg_match( $rule_value, $parsed_value );
					if ( $rc == 1 )
						return true;
					break;
					
				case 'NR';
				case 'NOTREGEX':
					$rc = preg_match( $rule_value, $parsed_value );
					if ( $rc == 0 )
						return true;
					break;
					
				case 'LK':
				case 'LIKE':
					$pos = strpos( $parsed_value, $rule_value );
					if ( $pos !== false )
						return true;
					break;
				
				case 'NL':
				case 'NOTLIKE':
					$pos = strpos( $parsed_value, $rule_value );
					if ( $pos === false )
						return true;
					break;
					
				default:
					return $this->runtime_error( __METHOD__, '', "Unknown operator '$rule_operator'" );
			}
						
		}
		
		return false;	// no ignore was matched, so return false
	}
	
	//-------------------------------------------
	//^ The logfile_match_trigger function should be called when _all_ matches are met
	//  and _no_ ignore rules apply. The first ruleset to trigger should call this function;
	//  and no further rulesets need to be checked.
	//> $ruleset_name is a string, something like 'ruleset1', 'ruleset2', etc.
	//> $column_values is an object containing the column values of the line that is currenly being processed
	//> $logfile_name is the fully qualified logfile name
	//> $logfile_line_number is the line within the logfile that triggered the match
	//> $logfile_data is the offending data
	public function logfile_match_trigger( $ruleset_name, vmtParseValues $column_values, $logfile_name, $logfile_line_number, $logfile_data )
	{
		// use the ruleset name to determine the line numbers of the matches and ignores that triggered this
		$ruleset_number = substr( $ruleset_name, strlen('ruleset') );
		$match_name = 'match' . $ruleset_number;
		$match_line_numbers = array_key_exists($match_name, $this->obj->tracking) ? $this->obj->tracking[$match_name] : '';
		$ignore_name = 'ignore' . $ruleset_number;
		$ignore_line_numbers = array_key_exists($ignore_name, $this->obj->tracking) ? $this->obj->tracking[$ignore_name] : '';
		
		$line_numbers = $match_line_numbers . $ignore_line_numbers;
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "scan_logfile(%s+%s)", $logfile_name, $logfile_line_number );
		
		$message_template = $this->rulesets[$ruleset_name]->message_template;
		$message = $this->replace_keywords2( $message_template, $column_values, $logfile_data );
		$message = sprintf( "'%s' %s",  $ruleset_name, $message );

		$this->talk( $this->trip_point( $line_numbers, $comment, $namespace_object, $method_string, $message ) );
		$this->validation_status = false;
		return false;
	}
	
	//-------------------------------------------
	protected function logfile_match_terse( $filename, $trippoint_count )
	{
		// exact line number not possible, just use the line number of the first attribute
		$line_number = ( count($this->obj->tracking) > 0 ) ? reset($this->obj->tracking) : '';
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "scan_logfile(%s)", $filename );
		
		$s = '';
		foreach ( $this->rulesets as $one_ruleset )
		{
			if ( $s != '' )
				$s .= ', ';
			$s .= $one_ruleset->ruleset_name;
		}
				
		$message = sprintf( "%s log entries matched the criteria specified in {%s}", $trippoint_count, $s );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		
		$this->validation_status = false;
		return false;
	}
	
	//-------------------------------------------
	//^ The replace_keywords2 function replaces each occurence of a dollar-sign prefixed column name
	//  with its parsed value, so if an logfile has a column name called 'host', a '$host' within the
	//  template would be replaced by a value like 'CE007'
	//  The special variable '$logline' can be used to display the logfile line that triggered the message.
	//  This function also performs vmtModule replacement of namespace object keywords and the
	//  additional keywords: $namespace, $object, $now.
	//  Unrecognized keywords are kept in the string
	//> $template is the template with the embedded variable names
	//> $logfile_data is the offending data
	//> $column_values is the current line parsed into columns
	//< returns the string with replaced values
	//
	protected function replace_keywords2( $template, vmtParseValues $column_values, $logfile_data )
	{
		$s = parent::replace_keywords( $template );
		
		global $logfile_module_classes;
		$parse_class_name = $logfile_module_classes[$this->logformat];
		$p = new $parse_class_name();
		
		foreach ( $p->column_names as $column_name )
		{
			$dollar_column_name = '$' . $column_name;
			while ( ($pos = strpos($s, $dollar_column_name)) !== false )
			{
				$len = strlen($dollar_column_name);
				
				// treat the column named '$timestamp' specially:
				if ( $dollar_column_name == '$timestamp' )
					$s = substr_replace( $s, strftime( "%c", $column_values->timestamp ), $pos, $len );
				else
					$s = substr_replace( $s, $column_values->$column_name, $pos, $len );
			}
		}

		$logline_keyword = '$logline';
		while ( ($pos = strpos($s, $logline_keyword)) !== false )
		{
			$len = strlen($logline_keyword);
			$s = substr_replace( $s, $logfile_data, $pos, $len );
		}
		
		return $s;
	}

}


#------------------------------------------------
// An instance of a vmtRuleset brings together all concatenated matches and exceptions
// that need to be satisfied for a line to trigger an alert. For example,
//   match1  = nameA OP valueA
//   match1 += nameB OP valueB
//   rule1  = nameC OP valueC
//   rule1 += nameD OP valueD,
// would result in $match_rule having two vmtRule objects in its array and $ignore_rule having two vmtRule objects in its array
//
class vmtRuleset
{
	public $ruleset_name;				// like 'ruleset1', 'ruleset2', etc
	public $message_template;			// a string, possibly containing embedded variables, to display when this rule is triggered
	public $matches = array();			// an array of zero or more vmtRule objects
	public $ignores = array();			// an array of zero or more vmtRule objects
	
	public function __construct( $ruleset_name )
	{
		$this->ruleset_name = $ruleset_name;
	}
		
	public function add_message( $message_template )
	{
		$this->message_template .= $message_template;
	}

	public function add_match_rule( vmtRule $match_rule )
	{
		$this->matches[] = $match_rule;
	}
	
	public function add_ignore_rule( vmtRule $ignore_rule )
	{
		$this->ignores[] = $ignore_rule;
	}
}


#------------------------------------------------
class vmtRule
{
	public $name;		// column name is a column identifier, defined by the logformat, for example the syslog format defines columns: timestamp, host, program, pid and message
	public $operator;	// operator is one of these: EQ|NE|LT|LE|GT|GE|RE|REGEX|NR|NOTREGEX|LK|LIKE|NL|NOTLIKE
	public $value;		// value is the value to compare against the specifed column name using the specified operator.
	
	static public $valid_operators = array( 'EQ', 'NE', 'LT', 'LE', 'GT', 'GE', 'RE', 'REGEX', 'NR', 'NOTREGEX', 'LK', 'LIKE', 'NL', 'NOTLIKE' );
		
		
	//-------------------------------------------
	//^ The constructor accepts an unparsed string and splits it into a column name, operator,
	//  and value.  Column names are validated against specific "logformat's" defined column names
	//  This constructor does not handle concatenated matches/ignores;
	//  they must be exploded before calling the constructor.
	//> $obj is the current module object (this is used for error message processing)
	//> $keyword is something like 'match1' or 'ignore1' (it is used for error message processing)
	//> $unparsed_string contains a column name, an operator, and a value
	public function __construct( vmtLogfile $obj, $keyword, $unparsed_string )
	{
		$this->name = 'undeclared';
		$this->operator = 'undeclared';
		$this->value = 'undeclared';
		
		$regex = '/\b(EQ|NE|LT|LE|GT|GE|RE|REGEX|NR|NOTREGEX|LK|LIKE|NL|NOTLIKE)\b/';
		
		// find the operator
		if ( preg_match( $regex, $unparsed_string, $matches ) == 0 )
			return $obj->specification_error( __METHOD__, $keyword, "Valid operator EQ|NE|LT|LE|GT|GE|RE|REGEX|NR|NOTREGEX|LK|LIKE|NL|NOTLIKE missing from '$unparsed_string'" );

		// parse the string into three parts and assign to the class variables
		$parts = preg_split( $regex, $unparsed_string, 2 );

		if ( count($parts) <> 2 )
			return $obj->specification_error( __METHOD__, $keyword, "syntax error '$unparsed_string'" );
					
		$this->name = trim( $parts[0] );
		$this->operator = trim( $matches[0] );
		$this->value = trim( $parts[1] );

		if ( $this->value == '' )
			return $obj->specification_error( __METHOD__, $keyword, "syntax error '$unparsed_string'" );

		// validate the column names
		global $logfile_module_classes;
		if ( !array_key_exists( $obj->logformat, $logfile_module_classes ) )
			return $obj->specification_error( __METHOD__, 'logformat', "There is no plugin for the logformat '{$obj->logformat}'" );

		$parser_classname = $logfile_module_classes[$obj->logformat];
		$p = new $parser_classname();
		if ( !in_array( $this->name, $p->column_names ) )
			return $obj->specification_error( __METHOD__, $keyword, "syntax error '{$this->name}' is not a valid column name for logformat '{$obj->logformat}'" );

		// if this is a 'timestamp' column, convert its value into a UNIX timestamp
		// so message = timestamp GT 2*24*60*60 means "the last two days"
		if ( $this->name == 'timestamp' )
		{
			$rc = preg_match( '/[^0-9|\*]/', $this->value );	 // only digits and splat
			if ( $rc == 1 )
				return $obj->specification_error( __METHOD__, $keyword, "the value of 'timestamp' should be the number of seconds before now, but '{$this->value}' was specified" );
				
			eval( "\$val = $this->value;" );				// a number of seconds
			$this->value = time() - $val;					// the number of seconds before now
		}
	}
}
	
?>
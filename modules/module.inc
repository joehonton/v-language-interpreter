<?php
#==============================================================================
#
# File:         /var/vermont/modules/module.inc
#
# Language:     Vermont module base class
#
# Contents:     A base module for processing objects
#
# Author:       Joe Honton © 2010
#
# Initial date: December 21, 2010
#
#
#
#     Keyword                  Value
# --------------------	-----------------------
# type                  The name of the plugin module, for example use 'file' to access the 'vmtFile' class defined in .../modules/file.inc
# comment               A text string that, if specified, is displayed as part of all error messages
# assert                An expression that must evaluate to true, like: $namespace:object.variable EQ 1234
# pass                  A text string, containing replacement variables, that is sent to output for any objects that passes all module tests.
#                       The replacement variables may be any module keyword preceded by a dollar sign, or may be the special variables $namespace or $object
# fail                  A text string, containing replacement variables, that is sent to output for any object that fails to pass all module tests.
# dependency            One or more strings, concatentated with '|', that specify objects that must be executed before this object.
#                       Use namespace:object syntax like [namespace:object]
# runtime_trace         Trace progress as each object is executed; true or false
# output_line_number    Show the show the filename and line number, in <filename+linenum> format, of the trip-point object
# output_object_name    Show the namespace and object name, in [namespace:object] format, of the trip-point object
# output_comment        Show the value of the object's "comment" keyword, in (comment) format, of the trip-point object
# output_method         Show the module's method and argument, in ->method(argument) format, of the trip-point object
# output_limit          Show no more than this many errors, optional, default value is 10, use 'false' for no limit
#
#==============================================================================


class vmtModule
{
	public $vocabulary = array();		// this list of possible attributes and whether they are required or optional
	public $message_count = 0;			// how many messages have been sent to output, in this module
	
	//-------------------------------------------
	// The constructor requires a managed object.  It should be
	// a composite object, assembled by the parsing script.
	public function __construct( vmtManagedObject $o )
	{
		global $config;
		
		$this->vocabulary['type'] = 'required';
		$this->vocabulary['comment'] = 'optional';
		$this->vocabulary['assert'] = 'optional';
		$this->vocabulary['pass'] = 'optional';
		$this->vocabulary['fail'] = 'optional';
		$this->vocabulary['dependency'] = 'optional';				// Note: dependencies are handled by the global Validate() function
		$this->vocabulary['vocabulary_check'] = 'optional';
		$this->vocabulary['runtime_trace'] = 'optional';
		$this->vocabulary['output_line_number'] = 'optional';
		$this->vocabulary['output_object_name'] = 'optional';
		$this->vocabulary['output_comment'] = 'optional';
		$this->vocabulary['output_method'] = 'optional';
		$this->vocabulary['output_limit'] = 'optional';
		
		$this->obj = $o;
		$this->kv = (object) $o->kv;
		$this->type = property_exists($this->kv, 'type') ? $this->kv->type : 'undeclared';
		$this->comment = property_exists($this->kv, 'comment') ? $this->kv->comment : 'undeclared';
		$this->assert = property_exists($this->kv, 'assert') ? $this->kv->assert : 'undeclared';
		$this->pass = property_exists($this->kv, 'pass') ? $this->kv->pass : 'undeclared';
		$this->fail = property_exists($this->kv, 'fail') ? $this->kv->fail : 'undeclared';
		$this->dependency = property_exists($this->kv, 'dependency') ? $this->kv->dependency : 'undeclared';
		$this->vocabulary_check = property_exists($this->kv, 'vocabulary_check') ? $this->kv->vocabulary_check : 'strict';
		$this->runtime_trace = property_exists($this->kv, 'runtime_trace') ? $this->kv->runtime_trace : 'undeclared';
		$this->output_line_number = property_exists($this->kv, 'output_line_number') ? $this->kv->output_line_number : 'undeclared';
		$this->output_object_name = property_exists($this->kv, 'output_object_name') ? $this->kv->output_object_name : 'undeclared';
		$this->output_comment = property_exists($this->kv, 'output_comment') ? $this->kv->output_comment : 'undeclared';
		$this->output_method = property_exists($this->kv, 'output_method') ? $this->kv->output_method : 'undeclared';
		$this->output_limit = property_exists($this->kv, 'output_limit') ? $this->kv->output_limit : 'undeclared';
		
		$this->vocabulary_status = true;
		$this->specification_status = true;
		$this->validation_status = true;
		$this->runtime_status = true;
	}
	
	//-------------------------------------------
	// The execute function is called by vermont's main driver
	public function execute()
	{
		if ( $this->runtime_trace == 'true' )
			$this->trace_message( 'enter' );
		
		if ( $this->pre_perform() == true )		// look for unrecognized and missing keywords
		{
			// perform only if the vocabulary check returned true
			$this->perform();
			$this->validate_assertions();
		}
		$rc = $this->post_perform();
		
		if ( $this->runtime_trace == 'true' )
			$this->trace_message( 'exit' );
			
		return $rc;
	}

	//-------------------------------------------
	// The pre_perform function is called before starting
	// return true to continue, false to halt
	public function pre_perform()
	{
		// The 'vocabulary_check' keyword controls strict versus lax vocabulary checking;
		// set this to 'lax' to allow the definition of keywords to be used as substituion variables
		if ( $this->vocabulary_check == 'strict' )
		{
			// look for unrecognized keywords and issue warning
			foreach( $this->kv as $keyword => $value )
			{
				if ( !array_key_exists($keyword, $this->vocabulary) )
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
	// The perform function must be defined by every module
	// returns void.  To indicate failure use one of the four status properties.
	public function perform()
	{
	}
	
	//-------------------------------------------
	// The post_perform function is called upon completion
	// return a boolean indicating the status of the performance
	public function post_perform()
	{
		if ( ($this->message_count > $this->output_limit) && ($this->output_limit != 'false') )
		{
			$line_number = '';
			$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
			$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
			$method_string = 'post_perform()';
			$diff = $this->message_count - $this->output_limit;
			$message = sprintf( "%s additional messages supressed by 'output_limit = %s' rule.", $diff, $this->output_limit );
			stdout( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		}

		// If the object's configuration specifies an explicit 'pass' or 'fail' attribute
		// use that attribute as a template and write to the output stream
		$rc = $this->status();
		if ( $rc == true )
		{
			if ( $this->pass != 'undeclared' && $this->pass != '' )
				$this->talk( $this->replace_keywords($this->pass) );
		}
		else // if ( $rc == false )
		{
			if ( $this->fail != 'undeclared'  && $this->fail != '' )
				$this->talk( $this->replace_keywords($this->fail) );
		}
		
		return $rc;
	}
	
	//-------------------------------------------
	//^ The validate_assertions function loops over all assert keywords
	//  and validates that their conditions are met.
	//  This could be done before or after any other vmtModule specific perform() processing,
	//  except in the case of vdtResource derived classes, which where it should probably be
	//  done after all variable evaluation occurs.
	public function validate_assertions()
	{
		// process the assertions one at a time
		$parts = explode( '|', $this->assert);
		foreach ( $parts as $one_assertion )
		{
			$one_assertion = trim($one_assertion);
			if ( $one_assertion != '' && $one_assertion != 'undeclared' )
				$this->verify_one_assertion( $one_assertion );
		}
	}
	
	//-------------------------------------------
	//^ The verify_one_assertion function validates one assertion.
	//> $one_assertion is an unparsed string containing an evaluation expression
	public function verify_one_assertion( $one_assertion )
	{
		$a = new vmtAssertion( $this, 'assert', $one_assertion );
		
		if ( $a->evaluate_assertion() == false )
		{
			// go back to the namespace stage to get the variable name before it was evaluated an substituted
			global $namespace_stage;
			$assert_phrases = '';
			$co_name = sprintf( "%s:%s", $this->obj->namespace_name, $this->obj->object_name );
			if ( array_key_exists( $co_name, $namespace_stage ) )
			{
				if ( array_key_exists( 'assert', $namespace_stage[$co_name]->kv ) )
					$assert_phrases = $namespace_stage[$co_name]->kv['assert'];
			}
			
			$extra_note = "($one_assertion) is not true";
			$this->validation_criteria_failed( __METHOD__, 'assert', $assert_phrases, $extra_note );
		}
	}
	
	//-------------------------------------------
	//^ The status function returns true if all status flags are true, or false if one or more flags are false
	protected function status()
	{
		return ($this->vocabulary_status && $this->specification_status && $this->validation_status && $this->runtime_status);
	}
	
	//-------------------------------------------
	//^ The replace_keywords function replaces each occurence of a dollar-sign prefixed keyword
	//  with its value, so if an object has a keyword called 'path', a '$path' within the
	//  template would be replaced by a value like "/path/to/file"
	//  This function also recognizes these additional keywords: $namespace, $object, $now
	//  Unrecognized keywords are kept in the string
	protected function replace_keywords( $template )
	{
		$s = $template;
		
		// using the list of keywords specified by the user for this object, look for the
		// dollarsign-prefixed variable name within the template, and replace.
		foreach ( $this->kv as $keyword => $value )
		{
			$dollar_keyword = '$' . $keyword;
			while ( ($pos = strpos($s, $dollar_keyword)) !== false )
			{
				$len = strlen($dollar_keyword);
				$s = substr_replace( $s, $value, $pos, $len );
			}
		}
		
		// also replace the namespace and object names
		$dollar_keyword = '$namespace';
		while ( ($pos = strpos($s, $dollar_keyword)) !== false )
		{
			$len = strlen($dollar_keyword);
			$s = substr_replace( $s, $this->obj->namespace_name, $pos, $len );
		}
		
		$dollar_keyword = '$object';
		while ( ($pos = strpos($s, $dollar_keyword)) !== false )
		{
			$len = strlen($dollar_keyword);
			$s = substr_replace( $s, $this->obj->object_name, $pos, $len );
		}
		
		$timestamp_keyword = '$now';
		while ( ($pos = strpos($s, $timestamp_keyword)) !== false )
		{
			$len = strlen($timestamp_keyword);
			$s = substr_replace( $s, strftime('%c'), $pos, $len );
		}
		
		return $s;
	}
	
	//-------------------------------------------
	// The whisper function should be called by functions that meet all their specified criteria
	public function whisper( $s )
	{
		$this->message_count++;
		if ( ($this->message_count <= $this->output_limit) || ($this->output_limit == 'false') )
			stdverbose( trim($s) );
	}

	//-------------------------------------------
	// The talk function should be called by user-defined objects that don't meet their specified criteria
	public function talk( $s )
	{
		$this->message_count++;
		if ( ($this->message_count <= $this->output_limit) || ($this->output_limit == 'false') )
			stdout( trim($s) );
	}

	//-------------------------------------------
	// The shout function should be called when unexpected problems occur
	public function shout( $s )
	{
		$this->message_count++;
		if ( ($this->message_count <= $this->output_limit) || ($this->output_limit == 'false') )
			stderr( trim($s) );
	}


	//-------------------------------------------
	//^ The trip_point function formats the error message output, including only what the configuration files specifies
	//> $line_number is the filename and line number, in <filename+linenum> format, of the trip-point object
	//> $comment is the value of the object's "comment" keyword, in (comment) format, of the trip-point object
	//> $namespace_object is the namespace and object name, in [namespace:object] format, of the trip-point object
	//> $method is the module's method and argument, in ->method(argument) format, of the trip-point object
	//> $message is the trip-point payload
	//< returns a string suitable for sending to whisper, talk or shout
	public function trip_point( $line_number, $comment, $namespace_object, $method, $message )
	{
		$sep = (TABS == true) ? "\t" : ' ';		// TABS is set by the commandline with the -t flag
		$sep2 = (TABS == true) ? "\t" : '';
		$s = '';
		
		if ( $this->output_line_number == 'true' && $line_number != '' )
			$s .= $line_number . $sep;
		
		if ( $this->output_comment == 'true' && $comment != '' )
			$s .= '(' . $comment . ')' . $sep;
		
		if ( $this->output_object_name == 'true' && $namespace_object != '' )
			$s .= $namespace_object . $sep2;
		
		if ( $this->output_method == 'true' && $method != '' )
		{
			if ( $this->output_object_name == 'true' && $namespace_object != '' )
				$s .= '->';
			
			$s .= $method . $sep;
		}
		
		$s .= $message;
			
		return $s;
	}
	
	//-------------------------------------------
	// The trace_message function is called at the top and bottom of execute() when
	// 'runtime_trace' is set to true in the configuration.
	// Provide the current start or stop time for each object as it is execute.
	//> $start_stop is either 'start' or ;stop'
	//
	protected function trace_message( $start_stop )
	{
		$line_number = reset($this->obj->tracking);		// exact line number not possible, just use the line number of the first attribute
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$message = sprintf( " %s at %s", $start_stop, strftime('%T') );
		stdout( $this->trip_point( $line_number, $comment, $namespace_object, '', $message ) );
	}
	
	//-------------------------------------------
	// The vocabulary_warning function should be called when an unrecognized keyword is encountered within an object.
	protected function vocabulary_warning( $attribute )
	{
		$line_number = array_key_exists($attribute, $this->obj->tracking) ? $this->obj->tracking[$attribute] : '';
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$message = sprintf( "unrecognized keyword '%s'", $attribute );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, '', $message ) );
		$this->vocabulary_status = true;
		return true;
	}
	
	//-------------------------------------------
	// The vocabulary_error function should be called when a required keyword is missing.
	protected function vocabulary_error( $attribute )
	{
		$line_number = '';
		if ( count($this->obj->tracking) > 0 )
			$line_number = reset($this->obj->tracking);		// exact line number not possible, just use the line number of the first attribute
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$message = sprintf( "missing required keyword '%s'", $attribute );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, '', $message ) );
		$this->vocabulary_status = false;
		return false;
	}
	
	//-------------------------------------------
	// The specification_error function should be called when a specification error is encountered.
	//> $method is the name of the vmtModule-derived object that was being executed when the criteria failed
	//> $attribute_keyword is an attribute associated with this error
	//> $extra_note as needed
	public function specification_error( $method, $attribute_keyword, $extra_note = '' )
	{
		$line_number = array_key_exists($attribute_keyword, $this->obj->tracking) ? $this->obj->tracking[$attribute_keyword] : '';
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "%s()", $method );
		$message = sprintf( "specification error %s = '%s' %s", $attribute_keyword, $this->kv->$attribute_keyword, $extra_note );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		$this->specification_status = false;
		return false;
	}

	//-------------------------------------------
	// The validation_criteria_failed function should be called when an object doesn't meet prescribed criteria
	//> $method is the name of the vmtModule-derived object that was being executed when the criteria failed
	//> $attribute_keyword is an attribute associated with this error
	//> $attribute_value is an attribute associated with this error
	//> $reason is the reason the criteria failed
	//> $rpn (Reverse Polish Notation), set to true when passing attribute/keyword instead of keyword/attribute
	//
	public function validation_criteria_failed( $method, $attribute_keyword, $attribute_value, $reason, $rpn = false )
	{
		if ( $rpn == true )
			$line_number = array_key_exists($attribute_value, $this->obj->tracking) ? $this->obj->tracking[$attribute_value] : '';
		else	// $rpn == false is the normal case
			$line_number = array_key_exists($attribute_keyword, $this->obj->tracking) ? $this->obj->tracking[$attribute_keyword] : '';
		
		// exact line number not possible, just use the line number of the first attribute
		if ( $line_number == '' )
			$line_number = ( count($this->obj->tracking) > 0 ) ? reset($this->obj->tracking) : '';
		
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "%s", $method );
		$attribute_keyword_spaced = ($attribute_keyword == '') ? '' : " $attribute_keyword";
		$attribute_value_quoted = ($attribute_value == '') ? '' : " '$attribute_value'";
		$message = sprintf( "validation criteria failed,%s%s %s", $attribute_keyword_spaced, $attribute_value_quoted, $reason );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		
		$this->validation_status = false;
		return false;
	}
	
	//-------------------------------------------
	//^ The validation_attribute_mismatch function should be called when an object's specified attribute value
	//  does not match the host computer's attribute value.
	//> $method is the name of the vmtModule-derived object that was being executed when the criteria failed
	//> $attribute_keyword is an attribute associated with this error
	//> $attribute_value is an attribute associated with this error
	//> $actual_value is the host computer's value
	//> $extra_note as needed
	//> $rpn (Reverse Polish Notation), set to true when passing attribute/keyword instead of keyword/attribute
	//
	public function validation_attribute_mismatch( $method, $attribute_keyword, $attribute_value, $actual_value, $extra_note = '', $rpn = false )
	{
		if ( $rpn == true )
			$line_number = array_key_exists($attribute_value, $this->obj->tracking) ? $this->obj->tracking[$attribute_value] : '';
		else	// $rpn == false is the normal case
			$line_number = array_key_exists($attribute_keyword, $this->obj->tracking) ? $this->obj->tracking[$attribute_keyword] : '';
		
		// exact line number not possible, just use the line number of the first attribute
		if ( $line_number == '' )
			$line_number = ( count($this->obj->tracking) > 0 ) ? reset($this->obj->tracking) : '';
		
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "%s", $method );
		$message = sprintf( "validation attribute mismatch, %s should be '%s', but host says '%s' %s", $attribute_keyword, $attribute_value, $actual_value, $extra_note );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		$this->validation_status = false;
		return false;
	}
	
	//-------------------------------------------
	// The runtime_error function should be called when an unexpected problem occurs that has nothing to do with the specification or its validation
	//> $method is the name of the vmtModule-derived object that was being executed when the criteria failed
	//> $attribute_keyword is an attribute associated with this error
	//> $extra_note as needed
	public function runtime_error( $method, $attribute_keyword, $extra_note = '' )
	{
		$line_number = array_key_exists($attribute_keyword, $this->obj->tracking) ? $this->obj->tracking[$attribute_keyword] : '';
		// exact line number not possible, just use the line number of the first attribute
		if ( $line_number == '' )
			$line_number = ( count($this->obj->tracking) > 0 ) ? reset($this->obj->tracking) : '';
		
		$attribute_pair = ($attribute_keyword == '') ? '' : sprintf( " %s = '%s'", $attribute_keyword, $this->$attribute_keyword );
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "%s", $method );
		$message = sprintf( "runtime problem%s %s", $attribute_pair, $extra_note );
		$this->shout( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		$this->runtime_status = false;
		return false;
	}
}

?>
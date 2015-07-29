<?php
//=============================================================================
//
// File:         /var/vermont/include/assertion.inc
//
// Language:     PHP
//
// Contents:     A class used by vmtModule derived classes for parsing and
//               evaluting user-defined assertions.  Each assertion is of the form:
//
//                 assert = (cast) subject operator expected-value
//
//               Where 'cast' is optional and if present is one of (string), (int), (float), or (timestamp)
//               Where 'subject' can be any valid keyword known to the specific
//                 vmtModule-derived class that is using this.
//               Where 'operator' is one of 'EQ', 'NE', 'LT', 'LE', 'GT', 'GE',
//                 'RE', 'REGEX', 'NR', 'NOTREGEX', 'LK', 'LIKE', 'NL', 'NOTLIKE'.
//               Where 'expected-value' is a string value used in the comparision.
//                 Expected-value is evaluated into a number before the comparison
//                 if no explicit cast has been specified, and if its range is
//                 limited to the values 0123456789.+-/*
//
// Author:       Joe Honton © 2011
//
// Initial date: February 2, 2011
//
//=============================================================================


//-------------------------------------
//^ The vmtAssertion class
//
class vmtAssertion
{
	public $cast;			// Optional, if present is one of (string), (int), (float), or (timestamp) and is used to evaluate the actual and expected values.
	public $subject;		// subject is a vmtModule keyword or a vmtParseBase column name.
	public $operator;		// operator is one of these: EQ|NE|LT|LE|GT|GE|RE|REGEX|NR|NOTREGEX|LK|LIKE|NL|NOTLIKE.
	public $value;			// value is the value to compare against the subject using the specified operator.
	
	static public $valid_operators = array( 'EQ', 'NE', 'LT', 'LE', 'GT', 'GE', 'RE', 'REGEX', 'NR', 'NOTREGEX', 'LK', 'LIKE', 'NL', 'NOTLIKE' );
		
		
	//-------------------------------------------
	//^ The constructor accepts an unparsed string and splits it into a cast, subject,
	//  operator and value.
	//> $obj is the current module object (this is used for error message processing)
	//> $keyword is something like 'assert' or 'match1' or 'ignore1' (also used only
	//    for error message processing)
	//> $unparsed_string contains an optional cast, a subject, an operator, and a value
	//
	public function __construct( vmtModule $obj, $keyword, $unparsed_string )
	{
		$this->cast = 'undeclared';
		$this->subject = 'undeclared';
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
					
		$raw_subject = trim( $parts[0] );
		$this->operator = trim( $matches[0] );
		$this->value = trim( $parts[1] );

		// parse the raw subject into the optional cast and the subject itself
		if ( preg_match( "/\(.*\)/", $raw_subject, $matches ) == 1 )
		{
			$this->cast = trim($matches[0]);			// (string) (int) (float) (timestamp)
			$this->subject = trim( str_replace( $this->cast, '', $raw_subject ) );
		}
		else
			$this->subject = $raw_subject;
	}

	//-------------------------------------------
	//^ The evaluate_assertion function
	//< returns true if the actual value meets the criteria specified by the operator and the
	//  expected value.
	//< returns true of the assert met the condition, or false if it didn't
	//
	public function evaluate_assertion()
	{
		$rule_cast = $this->cast;		// TODO handle this in the resource substitution phase
		$parsed_value = $this->subject;		// should already have been converted into the expected value
		$rule_operator = $this->operator;
		$rule_value = $this->value;		// TODO eval arithmetic

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
		return true;
	}
	
}


//TODO Feb 3

/*
		// if this is a 'timestamp' column, convert its value into a UNIX timestamp
		// so message = timestamp GT 2*24*60*60 means "the last two days"
		if ( $this->subject == 'timestamp' )
		{
			$rc = preg_match( '/[^0-9|\*]/', $this->value );	 // only digits and splat
			if ( $rc == 1 )
				return $obj->specification_error( __METHOD__, $keyword, "the value of 'timestamp' should be the number of seconds before now, but '{$this->value}' was specified" );
				
			eval( "\$val = $this->value;" );				// a number of seconds
			$this->value = mktime() - $val;					// the number of seconds before now
		}
*/

?>
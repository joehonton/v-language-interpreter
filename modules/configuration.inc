<?php
#==============================================================================
#
# File:         /var/vermont/modules/configuration.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for parsing a generic configuration file for the presence
#               of a configuration keyword and determining its value
#               The parser looks for case sensitive keywords that begin at the first byte of a line.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 25, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# path              the fully qualified path of the configuration file, like /etc/httpd/conf/httpd.conf
# keyword_value     A keyword value pair to verify. The pair is separated by a colon, like User => apache.
#                   Specify one or more of these using the += operator.
#                   To check configuration files that don't use the keyword => value format, simply specify
#                   the value to look for followed by '=>', like 'keyword_value = 173.11.93.101 =>'
#
#==============================================================================


class vmtConfiguration extends vmtFile
{
	//-------------------------------------------
	public function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['path'] = 'required';
		$this->vocabulary['keyword_value'] = 'optional';
		
		$this->path = property_exists($this->kv, 'path') ? $this->kv->path : 'undeclared';
		$this->keyword_value = property_exists($this->kv, 'keyword_value') ? $this->kv->keyword_value : 'undeclared';
	}

	//-------------------------------------------
	// The perform function first perform's all file-related verification,
	// then verifies the keyword_value pairs
	public function perform()
	{
		// do all the normal vmtFile verification
		parent::perform();
		
		// if this file passes all owner, permission, and selinux verification, continue
		if ( $this->status() == true )
			$this->verify_keyword_values();
	}


	//-------------------------------------------
	private function verify_keyword_values()
	{
		$f = fopen( $this->path, 'r' );
		if ( $f == false )
			return $this->runtime_error( __METHOD__, 'path', 'unable to read file' );

		// explode the concatenated namespace of keyword_value pairs
		$parts = explode( '|', $this->keyword_value );
		foreach( $parts as $one_pair )
		{
			// split the keyword/value pair on the special symbol '=>'
			$pos_colon = strpos( $one_pair, '=>' );
			if ( $pos_colon === false )
			{
				$this->specification_error( __METHOD__, 'keyword_value', "keyword and value should be separated by '=>', but '$one_pair' was specified" );
				continue;
			}
			else
			{
				$keyword_half = trim( substr( $one_pair, 0, $pos_colon ) );
				$value_half = trim( substr( $one_pair, $pos_colon+2 ) );
				$this->verify_keyword_value( $f, $keyword_half, $value_half );
			}
		}
		fclose( $f );
	}

	//-------------------------------------------
	//^ The verify_keyword_value scans the already-open file
	//  looking for the given keyword.  If the given keyword is found
	//  at the beginning of a line, examine the value and return true
	//  if the file's value is equal to the specified value
	//> $f an open configuration file
	//> $keyword_half is the keyword to scan for
	//> $expected_value is the value to scan for
	private function verify_keyword_value( $f, $keyword_half, $value_half )
	{
		$line_number = 0;
		rewind( $f );
		while ( ($line = fgets($f)) != false )
		{
			$line_number++;
			
			// look for the keyword at the beginning of a line
			$pos = strpos( $line, $keyword_half );
			if ( $pos !== false && $pos == 0 )
			{
				// in cases where the keyword-value pair is specified without a value, like '173.11.93.101:' just return true, because we've found it
				if ( $value_half == '' )
					return true;
				
				// jump over the keyword to get to the value
				$len = strlen($keyword_half);
				$configuration_value = trim( substr( $line, $len ) );
				
				// if the configuration value starts with an equals sign, discard it
				if ( $configuration_value{0} == '=' )
					$configuration_value = trim( substr( $configuration_value, 1 ) );
			
				// Instead of looking for an exact match, simply look for a partial match;
				// this will allow the configuration file to have comments.
				// Use case insensitive matching
				$match = stripos( $configuration_value, $value_half );
				
				if ( $match === false )
					return $this->validation_attribute_mismatch( __METHOD__ . "({$keyword_half})", 'value', $value_half, $configuration_value, "in path '{$this->path}' on line number $line_number" );
				else
					return true;		// found the keyword, and the value matches
			}
		}
		return $this->validation_criteria_failed( __METHOD__ . "({$keyword_half})", 'keyword', $keyword_half, "not found in path '{$this->path}'" );
	}
}

?>
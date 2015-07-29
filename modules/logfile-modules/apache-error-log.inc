<?php
#==============================================================================
#
# File:         /var/vermont/modules/parse/apache-error-log.inc
#
# Language:     Vermont plugin logfile parsing class
#
# Contents:     A class for parsing Apache httpd error logs where logformat = apache-error-log
#
# Author:       Joe Honton © 2011
#
# Initial date: January 8, 2011
#
#==============================================================================


class vmtLogfileApacheErrorLog extends vmtLogfileBase
{
	//-------------------------------------------
	public function __construct()
	{
		$this->column_names = array( 'timestamp', 'loglevel', 'client', 'message' );
	}

	//-------------------------------------------
	//^ The parse function accepts a single line from a logfile and returns
	//  an array containing columns values.
	//> $line is a string of text in a format specific to this class's logfile
	//< $column_values is an array, that is returned by reference, containing
	//  the parsed values, or an empty string if the column value is missing.
	//<	Normally returns true, even when some columns could not be parsed.
	//  Returns false only on an unrecoverable error
	public function parse( $line, vmtParseValues &$column_values )
	{
		// the default for all column values is an empty string
		foreach ( $this->column_names as $column_name )
			$column_values->$column_name = '';				// <-- these are dynamic members
		
		$len = strlen($line);
		$p = 0;
		
		// timestamp
		$tuple = $this->extract_bracketed_substring( $line, '[', ']' );
		$timepart = $tuple[1];
		$line = $tuple[2];
		
		$column_values->timestamp = $this->parse_apache_error_timestamp( $timepart );
		if ( $column_values->timestamp == 0 )
			return false;

		// loglevel
		$tuple = $this->extract_bracketed_substring( $line, '[', ']' );
		$column_values->loglevel = $tuple[1];
		$line = $tuple[2];
			
		// client may be missing
		$tuple = $this->extract_bracketed_substring( $line, '[', ']' );
		$column_values->client = $tuple[1];
		$line = $tuple[2];
		
		// the message
		$column_values->message = substr( $line, $p );
		
		return true;
	}
	
	//-------------------------------------------
	//^ The parse_apache_error_timestamp function
	public function parse_apache_error_timestamp( $line )
	{
		$len = strlen($line);
		$p = 0;
		// The timestamp is composed of:
		//  Three characters for the day of week
		//  space
		//  Three characters for the month
		//  space
		//  two digits for the day of month
		//  space
		//  HH:MM:SS
		//  space
		//  four digits for the year
		$log_day_of_week = $this->pop_fixed_length( $line, $p, 3, $len );
		$p++; // space
		$log_month = $this->pop_fixed_length( $line, $p, 3, $len );
		$p++; // space
		$log_day = $this->pop_fixed_length( $line, $p, 2, $len );
		$p++; // space
		$log_time = $this->pop_fixed_length( $line, $p, 8, $len );
		$p++; // space
		$log_year = $this->pop_fixed_length( $line, $p, 4, $len );

		static $three_letter_months = array(
			'Jan' => 1,
			'Feb' => 2,
			'Mar' => 3,
			'Apr' => 4,
			'May' => 5,
			'Jun' => 6,
			'Jul' => 7,
			'Aug' => 8,
			'Sep' => 9,
			'Oct' => 10,
			'Nov' => 11,
			'Dec' => 12
		);
		
		if ( !array_key_exists($log_month, $three_letter_months) )
			return 0;
		$log_month = $three_letter_months[$log_month];		// 'Jan' --> 1, ... 'Dec' --> 12
		
		$log_hour = substr($log_time, 0, 2 );
		$log_minute = substr($log_time, 3, 2 );
		$log_second = substr($log_time, 6, 2 );
		if ( $log_hour < 0 || $log_hour > 23 ||
			 $log_minute < 0 || $log_minute > 59 ||
			 $log_second < 0 || $log_second > 59 ||
			 $log_month < 1 || $log_month > 12 ||
			 $log_day < 1 || $log_day > 31 )
			 	return 0;
			 	
		return mktime(
				$log_hour,
				$log_minute,
				$log_second,
				$log_month,
				$log_day,
				$log_year );
	}
	
}

?>
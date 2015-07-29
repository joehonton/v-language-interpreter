<?php
#==============================================================================
#
# File:         /var/vermont/modules/parse/syslog.inc
#
# Language:     Vermont plugin logfile parsing class
#
# Contents:     A class for parsing 'syslog' file entries where logformat = syslog
#
# Author:       Joe Honton © 2011
#
# Initial date: January 5, 2011
#
#==============================================================================


class vmtLogfileSyslog extends vmtLogfileBase
{
	//-------------------------------------------
	public function __construct()
	{
		$this->column_names = array( 'timestamp', 'host', 'program', 'pid', 'message' );
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
			
		// The timestamp is composed of:
		//  Three characters for the month
		//  space
		//  one or two digits for the day of month
		//  space
		//  HH:MM:SS
		$log_month = $this->pop_fixed_length( $line, $p, 3, $len );
		$p++; // space
		$log_day = $this->pop_fixed_length( $line, $p, 2, $len );
		$p++; // space
		$log_time = $this->pop_fixed_length( $line, $p, 8, $len );
		$p++; // space
		$column_values->timestamp = $this->timestamp_from_syslog( $log_month, $log_day, $log_time );
		if ( $column_values->timestamp == 0 )
			return false;
			
		// the host
		$column_values->host = $this->pop_to_delimiter( $line, $p, ' ', $len );
		$p++; // space
		
		// the program and optional PID
		$program_and_pid = $this->pop_to_delimiter( $line, $p, ':', $len );
		$dual = $this->extract_bracketed_substring( $program_and_pid, '[', ']' );
		$column_values->program = $dual[0];
		$column_values->pid = $dual[1];
		$p++; // space
		
		// the message
		$column_values->message = substr( $line, $p );
		
		return true;
	}
	
	//-------------------------------------------
	//^ The timestamp_from_syslog function constructs a UNIX timestamp from the
	//  three fields provided by syslog (month, day, and time)
	//> $month is a three letter English abbreviation of the month, like 'Jan', ... 'Dec'
	//> $day is a one or two digit number from 1 to 31
	//> $time is an eight character time in HH:MM:SS format
	//< returns a UNIX timestamp or 0 (Dec 31, 1969) on failure
	public function timestamp_from_syslog( $log_month, $log_day, $log_time )
	{
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
		
		// Since there is no year part we'll need to use an algorithm for determining whether
		// the message was generated this year or last year.  Use localtime, not UTC.
		$current_month = strftime( '%m' );
		$current_day = strftime( '%e' );
		$log_year = strftime( '%Y' );
		if ( $log_month > $current_month )
			$log_year--;
		else if ( $log_month == $current_month && $log_day > $current_day )
			$log_year--;

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
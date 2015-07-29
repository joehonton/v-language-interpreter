<?php
#==============================================================================
#
# File:         /var/vermont/modules/parse/apache-access-log.inc
#
# Language:     Vermont plugin logfile parsing class
#
# Contents:     A class for parsing Apache httpd access logs where logformat = apache-access-log
#               This is coded to match the Apache 'common log format' (CLF)
#               Refer to http://httpd.apache.org/docs/current/logs.html
#
# Author:       Joe Honton © 2011
#
# Initial date: January 13, 2011
#
#==============================================================================


class vmtLogfileApacheAccessLog extends vmtLogfileBase
{
	//-------------------------------------------
	public function __construct()
	{
		$this->column_names = array( 'client_ip', 'identity', 'userid', 'timestamp', 'request', 'response', 'size' );
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
		// 220.181.7.19 - - [27/Dec/2010:15:28:17 -0800] "GET /robots.txt HTTP/1.1" 302 215
		
		// the default for all column values is an empty string
		foreach ( $this->column_names as $column_name )
			$column_values->$column_name = '';				// <-- these are dynamic members
		
		$len = strlen($line);
		$offset = 0;

		// client_ip, identity, userid
		$column_values->client_ip = $this->pop_to_delimiter( $line, $offset, ' ', $len );
		$offset++;
		$column_values->identity = $this->pop_to_delimiter( $line, $offset, ' ', $len );
		$offset++;
		$column_values->userid = $this->pop_to_delimiter( $line, $offset, ' ', $len );
		$offset++;
		
		// timestamp
		$tuple = $this->extract_bracketed_substring( $line, '[', ']' );
		$timepart = $tuple[1];
		$line = $tuple[2];
		$len = strlen($line);
		$offset = 0;
		$offset++;
		
		$column_values->timestamp = $this->parse_apache_access_timestamp( $timepart );
		if ( $column_values->timestamp == 0 )
			return false;

		// request
		$tuple = $this->extract_bracketed_substring( $line, '"', '"' );
		$column_values->request = $tuple[1];
		$line = $tuple[2];
		$len = strlen($line);
		$offset = 0;
		$offset++;
		
		// response
		$column_values->response = $this->pop_to_delimiter( $line, $offset, ' ', $len );
		$offset++;
		
		// size
		$column_values->size = substr( $line, $offset );
		
		//TODO extra  "- "-" "Apache/2.2.14 (Fedora) (internal dummy connection)""
		
//		var_dump( $column_values );
		return true;
	}
	
	//-------------------------------------------
	//^ The parse_apache_access_timestamp function
	public function parse_apache_access_timestamp( $line )
	{
		$len = strlen($line);
		$p = 0;
		// The timestamp is composed of: [27/Dec/2010:15:28:17 -0800]
		//  Two digits for day of month
		//  slash
		//  Three characters for the month
		//  slash
		//  four digits for the year
		//  colon
		//  HH:MM:SS
		//  space
		//  five digits for the timezone offset, like -0800
		$log_day = $this->pop_fixed_length( $line, $p, 2, $len );
		$p++; // slash
		$log_month = $this->pop_fixed_length( $line, $p, 3, $len );
		$p++; // slash
		$log_year = $this->pop_fixed_length( $line, $p, 4, $len );
		$p++; // colon
		$log_time = $this->pop_fixed_length( $line, $p, 8, $len );
		$p++; // space
		$log_timezone = $this->pop_fixed_length( $line, $p, 5, $len );

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
				
		// todo account for timezone
	}

}
?>
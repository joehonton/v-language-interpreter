<?php
#==============================================================================
#
# File:         /var/vermont/modules/parse/iptables-log.inc
#
# Language:     Vermont plugin logfile parsing class
#
# Contents:     A class for parsing iptables logs logs where logformat = iptables-log
#               Use this when /var/log/iptables.log is created by 'LOG' commands
#               that begin with the string "iptables", within /etc/sysconfig/iptables.
#
#               These are suppressed from /var/log/messages when these two lines
#               are added to /etc/rsyslog.conf:
#                  :msg, startswith, "iptables" -/var/log/iptables.log
#                  &~
#
# Author:       Joe Honton © 2011
#
# Initial date: January 13, 2011
#
#==============================================================================


class vmtLogfileIptablesLog extends vmtLogfileSyslog
{
	//-------------------------------------------
	public function __construct()
	{
		parent::__construct();		//	'timestamp', 'host', 'program', 'pid', 'message'
		
		$this->column_names = array_merge( $this->column_names, array( 'preamble', 'IN', 'OUT', 'MAC', 'SRC', 'DST', 'LEN', 'TOS', 'PREC', 'TTL', 'ID', 'DF', 'PROTO', 'SPT', 'DPT', 'WINDOW', 'RES', 'SYN', 'URGP' ) );
	}

	//-------------------------------------------
	//^ The parse function first calls the vmtSyslog base class to parse syslog fields,
	//  then parses the 'message' field to further split the logline into iptables fields.
	//> $line is a string of text in a format specific to this class's logfile
	//< $column_values is an array, that is returned by reference, containing
	//  the parsed values, or an empty string if the column value is missing.
	//<	Normally returns true, even when some columns could not be parsed.
	//  Returns false only on an unrecoverable error
	public function parse( $line, vmtParseValues &$column_values )
	{
		if ( parent::parse( $line, $column_values ) == false )
			return false;
		
		$message = $column_values->message;		// the syslog message, just parsed by vmtParseSyslog
		$line_length = strlen( $message );
		$offset = 0;

		// iptables LOG HASHLIMIT IN=eth1 OUT= MAC=00:14:22:1c:0d:33:00:26:f3:20:4b:86:08:00 SRC=68.87.76.185 DST=173.11.93.101 LEN=60 TOS=0x00 PREC=0x40 TTL=58 ID=0 DF PROTO=UDP SPT=24895 DPT=22 WINDOW=5840 RES=0x00 SYN URGP=0
		
		$column_values->preamble =	$this->pop_keyword_to_keyword( $message, $offset, '', 'IN=', $line_length );
		$column_values->IN =		$this->pop_keyword_to_keyword( $message, $offset, 'IN=', 'OUT=', $line_length );
		$column_values->OUT =		$this->pop_keyword_to_keyword( $message, $offset, 'OUT=', 'MAC=', $line_length );
		$column_values->MAC =		$this->pop_keyword_to_keyword( $message, $offset, 'MAC=', 'SRC=', $line_length );
		$column_values->SRC =		$this->pop_keyword_to_keyword( $message, $offset, 'SRC=', 'DST=', $line_length );
		$column_values->DST =		$this->pop_keyword_to_keyword( $message, $offset, 'DST=', 'LEN=', $line_length );
		$column_values->LEN =		$this->pop_keyword_to_keyword( $message, $offset, 'LEN=', 'TOS=', $line_length );
		$column_values->TOS =		$this->pop_keyword_to_keyword( $message, $offset, 'TOS=', 'PREC=', $line_length );
		$column_values->PREC =		$this->pop_keyword_to_keyword( $message, $offset, 'PREC=', 'TTL=', $line_length );
		$column_values->TTL =		$this->pop_keyword_to_keyword( $message, $offset, 'TTL=', 'ID=', $line_length );
		$column_values->ID =		$this->pop_keyword_to_keyword( $message, $offset, 'ID=', 'DF', $line_length );
		$column_values->DF =		$this->pop_keyword_to_keyword( $message, $offset, 'DF', 'PROTO=', $line_length );
		$column_values->PROTO =		$this->pop_keyword_to_keyword( $message, $offset, 'PROTO=', 'SPT=', $line_length );
		$column_values->SPT =		$this->pop_keyword_to_keyword( $message, $offset, 'SPT=', 'DPT=', $line_length );
		$column_values->DPT =		$this->pop_keyword_to_keyword( $message, $offset, 'DPT=', 'WINDOW=', $line_length );
		$column_values->WINDOW =	$this->pop_keyword_to_keyword( $message, $offset, 'WINDOW=', 'RES=', $line_length );
		$column_values->RES =		$this->pop_keyword_to_keyword( $message, $offset, 'RES=', 'SYN', $line_length );
		$column_values->SYN =		$this->pop_keyword_to_keyword( $message, $offset, 'SYN', 'URGP=', $line_length );
		$column_values->URGP =		$this->pop_keyword_to_keyword( $message, $offset, 'URGP=', '', $line_length );
		
		return true;
	}
}
?>
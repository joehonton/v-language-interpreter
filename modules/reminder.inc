<?php
#==============================================================================
#
# File:         /var/vermont/modules/reminder.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for setting an alarm/reminder which will be sent when
#               the specified date/time has been reached. This might be useful
#               for remembering to renew domain name registrations or SSL
#               certificates. Messages can be sent to the output stream or to an
#               email address.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 30, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# alarm_date        (YYYY-MM-DD) the date which triggers the reminder 
# alarm_time        (HH:MM) the optional time which triggers the reminder, expressed with a 24 hour clock, in local server time
# message           A message to be sent.  This is a template that may include the $timestamp variable.
# email_to          An optional email address where the message should be sent.
#                   If not specified, the message will appear in the standard output only.
# email_from        An optional email address of the sender
#
#==============================================================================


class vmtReminder extends vmtModule
{
	public $parsed_message;
	
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['alarm_date'] = 'required';
		$this->vocabulary['alarm_time'] = 'optional';
		$this->vocabulary['message'] = 'optional';
		$this->vocabulary['email_to'] = 'optional';
		$this->vocabulary['email_from'] = 'optional';
		
		$this->alarm_date = property_exists($this->kv, 'alarm_date') ? $this->kv->alarm_date : 'undeclared';
		$this->alarm_time = property_exists($this->kv, 'alarm_time') ? $this->kv->alarm_time : 'undeclared';
		$this->message = property_exists($this->kv, 'message') ? $this->kv->message : 'the time has elapsed';
		$this->email_to = property_exists($this->kv, 'email_to') ? $this->kv->email_to : 'undeclared';
		$this->email_from = property_exists($this->kv, 'email_from') ? $this->kv->email_from : 'undeclared';

		$this->parsed_message = $this->replace_keywords( $this->message );
	}

	//-------------------------------------------
	// The perform function
	public function perform()
	{
		$current_datetime = strftime( '%Y-%m-%d %H:%M', time() );
		
		if ($this->alarm_time == 'undeclared')
			$specified_datetime = $this->alarm_date . " 00:00";
		else
			$specified_datetime = sprintf( "%s %s", $this->alarm_date, $this->alarm_time );
		
		if ( $current_datetime >= $specified_datetime )
		{
			$this->validation_criteria_failed( 'vmtReminder::alarm', 'alarm_date', $specified_datetime, $this->parsed_message );
		
			if ( $this->email_to != 'undeclared' )
			{
				if ( $this->email_from != 'undeclared' )
					return mail( $this->email_to, $this->parsed_message, $this->parsed_message, "From: {$this->email_from}\r\nReply-To: {$this->email_from}" );
				else
					return mail( $this->email_to, $this->parsed_message, $this->parsed_message );
			}
		}
	}
}

?>
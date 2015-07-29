<?php
#==============================================================================
#
# File:         /var/vermont/modules/mail.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking that mail is being accepted, transferred,
#               and delivered.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 30, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# from              Send a test message from this user: a linux username or an email address.
# to                Send a test message to this user: a linux username or an email address.
# subject_line      Send a test message with this subject line.
#                   This is a template and may contain the any valid keyword, or the special value $timestamp
# arrival_timeout   The number of seconds to wait for the arrival of the message before giving up.
#                   Valid range is 1 to 3600.
#                   Use 'false' to skip this test.
# arrival_maildir   This host's fully qualified pathname to the maildir of the recipient.
#                   This must be specified if arrival_timeout is specified.
#                   The use of arrival_timeout and arrival_maildir are only meaningful when the recipient has a "Maildir" style mailbox located on this host.
#
#==============================================================================


class vmtMail extends vmtModule
{
	public $parsed_subject_line;
	
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['from'] = 'required';
		$this->vocabulary['to'] = 'required';
		$this->vocabulary['subject_line'] = 'required';
		$this->vocabulary['arrival_timeout'] = 'optional';
		$this->vocabulary['arrival_maildir'] = 'optional';
		
		$this->from = property_exists($this->kv, 'from') ? $this->kv->from : 'undeclared';
		$this->to = property_exists($this->kv, 'to') ? $this->kv->to : 'undeclared';
		$this->subject_line = property_exists($this->kv, 'subject_line') ? $this->kv->subject_line : 'undeclared';
		$this->arrival_timeout = property_exists($this->kv, 'arrival_timeout') ? $this->kv->arrival_timeout : 'false';
		$this->arrival_maildir = property_exists($this->kv, 'arrival_maildir') ? $this->kv->arrival_maildir : 'undeclared';
		
		$this->parsed_subject_line = $this->replace_keywords( $this->subject_line );
	}

	//-------------------------------------------
	public function perform()
	{
		$rc = $this->send_message();
		
		// Poll for receipt of the just-sent message, if the message was sent successfully,
		// and if an 'arrival_timeout' was specified.
		if ( $rc == true && $this->arrival_timeout != 'false' )
		{
			// divide the total wait time into 5 equal polling intervals
			$polling_interval = floor( $this->arrival_timeout / 5 );
			if ( $polling_interval < 1 || $polling_interval > 3600 )
				return $this->specification_error( __METHOD__, 'arrival_timeout', "valid values are between 1 and 3600 seconds" );
			
			$bArrived = false;
			for ( $i = 0; $i < 5; $i++ )
			{
				sleep( $polling_interval );
				$rc = $this->check_for_arrival();		// returns true to continue polling
				if ( $rc == false )
					break;
			}
			if ( $rc == true )
				$this->validation_criteria_failed( "vmtMail::check_for_arrival()", 'arrival_timeout', $this->arrival_timeout, "test message was sent, but didn't arrive within the specified {$this->arrival_timeout} second time limit" );
		}
	}
	
	//-------------------------------------------
	//^ The send_message function attempts to send a test message, using SMTP port 25, to the specified recipient.
	//< returns true if the message was accepted for delivery by the mail transfer agent.
	public function send_message()
	{
		return mail( $this->to, $this->parsed_subject_line, $this->parsed_subject_line, "From: {$this->from}\r\nReply-To: {$this->from}" );
	}

	//-------------------------------------------
	//^ The check_for_arrival function scans the specified Maildir looking for the message just sent
	//< returns false to stop the polling, true to continue polling
	public function check_for_arrival()
	{
		if ( $this->arrival_maildir == 'undeclared' )
			return $this->specification_error( __METHOD__, 'arrival_maildir', "you must specify 'arrival_maildir' when specifying 'arrival_timeout'" );
		
		if ( !file_exists($this->arrival_maildir) )
			return $this->specification_error( __METHOD__, 'arrival_maildir', "this Maildir does not exist" );

		$glob_pattern = $this->arrival_maildir . '/*';
		$glob_files = glob($glob_pattern);
		
		foreach( $glob_files as $one_filename )
		{
			if ( $this->scan_mail_file( $one_filename ) == true )
				return false;
		}
		
		return true;
	}
	
	//-------------------------------------------
	//^ The scan_mail_file function
	//< returne true if this is the mail message just sent
	//< returns false if this is not the mail message just sent
	private function scan_mail_file( $filename )
	{
		$f = fopen( $filename, 'r' );
		if ( $f == false )
			return $this->runtime_error( __METHOD__ . "($filename)", '', 'unable to read file' );

		$bFound = false;
		while ( ($line = fgets($f)) != false )
		{
			$pos = strpos( $line, $this->parsed_subject_line );
			if ( $pos !== false )
			{
				$bFound = true;
				break;
			}
		}
		fclose( $f );
		
		// if this is the file with the test message that we just sent, delete it and return true to the caller
		if( $bFound == true )
		{
			if ( unlink( $filename ) == false )
				return $this->runtime_error( __METHOD__ . "($filename)", '', 'message found, but not removed from mail queue' );
				
			return true;
		}
		else
			return false;
	}
}

?>
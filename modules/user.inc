<?php
#==============================================================================
#
# File:         /var/vermont/modules/user.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking a user's account setup.
#
# Author:       Joe Honton © 2011
#
# Initial date: January 28, 2011
#
#
#     Keyword               Value
# ---------------	-----------------------
# user_name         The name of the user's account, as defined by adduser.
# home_path         The fully qualified path to the user's home directory, or an
#                   empty string if the user does not have a home directory.
#                   Optional, if missing, does not perform any verification.
#                   If present, this path's owner is verified to be the user.
# mail_path         The fully qualified path to the user's Maildir or Mbox, optional.
# compare           A comparison equation to evaluate.  If the comparison evaluates to
#                   false, the trip-point is triggered and a message is emitted.
#                   Comparison equations have three parts: subject, operator, and value.
#                   Possible subjects are: home_filecount, home_filesize, mail_filecount, mail_filesize.
#                   Possible operators are: EQ, NE, LT, LE, GT, GE.
#                   Possible values can be simple integers or numeric expressions like 10*1024*1024
#
#==============================================================================


class vmtUser extends vmtModule
{
	
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['user_name'] = 'required';
		$this->vocabulary['home_path'] = 'optional';
		$this->vocabulary['mail_path'] = 'optional';
		$this->vocabulary['compare'] = 'optional';
		
		$this->user_name = property_exists($this->kv, 'user_name') ? $this->kv->user_name : 'undeclared';
		$this->home_path = property_exists($this->kv, 'home_path') ? $this->kv->home_path : 'undeclared';
		$this->mail_path = property_exists($this->kv, 'mail_path') ? $this->kv->mail_path : 'undeclared';
		$this->compare = property_exists($this->kv, 'compare') ? $this->kv->compare : 'undeclared';
	}

	//-------------------------------------------
	// The perform function
	public function perform()
	{
	}
}

?>
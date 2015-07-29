<?php
#==============================================================================
#
# File:         /var/vermont/modules/selinux.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking Security Enhanced Linux boolean values
#
# Author:       Joe Honton © 2010
#
# Initial date: December 24, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# on                a `getsebool` value that should be 'on'
# off               a `getsebool` value that should be 'off'
#
#==============================================================================


class vmtSelinux extends vmtModule
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['on'] = 'optional';
		$this->vocabulary['off'] = 'optional';
				
		$this->on = property_exists($this->kv, 'on') ? $this->kv->on : 'undeclared';
		$this->off = property_exists($this->kv, 'off') ? $this->kv->off : 'undeclared';
	}

	//-------------------------------------------
	// The perform function checks that each specified selinux value is either on or off as expected
	public function perform()
	{
		$this->verify_boolean( $this->on, 'on' );
		$this->verify_boolean( $this->off, 'off' );
	}

	//-------------------------------------------
	// The verify_booleans splits the given variable, which may have multiple
	// values or'ed together, into single values.
	// $string_value is a concatenated string containing one or more SELinux boolean keywords
	// $on_off is the value 'on' or 'off'
	//< return void
	public function verify_boolean( $concatenated_keywords, $on_off )
	{
		$parts = explode( '|', $concatenated_keywords );
		
		foreach ( $parts as $one_keyword )
		{
			$one_keyword = trim($one_keyword);
			if ( $one_keyword == 'undeclared' )
				continue;
			
			// getsebool httpd_can_network_connect returns "httpd_can_network_connect --> on"
			// 2>&1 redirects stderr to stdout
			// awk '{print $3}' prints only the third column, the value "on" or "off"
			//$command = "getsebool $one_keyword 2>&1 | awk '{print $3}'";
			$command = "getsebool $one_keyword 2>&1";
			$response = shell_exec( $command );
			$response_no_ws = str_replace( array("\t","\n"), ' ', $response );
			$response_no_ws = trim($response_no_ws);
			
			// getsebool returns "Error getting active value for $one_keyword"
			$pos = stripos( $response_no_ws, 'error' );
			if ( $pos !== false )
				return $this->validation_criteria_failed( __METHOD__ . "($one_keyword)", $one_keyword, $on_off, 'No such selinux boolean', true );
			
			$pos = stripos( $response_no_ws, "--> $on_off" );
			if ( $pos === false )
				return $this->validation_attribute_mismatch( __METHOD__ . "($one_keyword)", $one_keyword, $on_off, $response_no_ws, '', true );
		}
	}
	
}
?>
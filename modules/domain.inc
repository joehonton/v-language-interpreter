<?php
#==============================================================================
#
# File:         /var/vermont/modules/domain.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking that a DNS name and address properly match
#
# Author:       Joe Honton © 2010
#
# Initial date: December 24, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# record_type       ANY, A, AAAA, CNAME, MX, NS, PTR, SOA, TXT.  This is optional and will default to 'A'.
# request           The record to verify. This may be a concatentated list of names
#                   that all resolve to the same reply.
# reply             The reply that is expected from the DNS server
# dns_server        The IP address of the DNS server to use to resolve the name, optional
# throttle          The number of seconds to wait between subsequent requests, optional
#
#==============================================================================


class vmtDomain extends vmtModule
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['record_type'] = 'optional';
		$this->vocabulary['request'] = 'required';
		$this->vocabulary['reply'] = 'required';
		$this->vocabulary['dns_server'] = 'optional';
		$this->vocabulary['throttle'] = 'optional';
		
		$this->record_type = property_exists($this->kv, 'record_type') ? $this->kv->record_type : 'A';
		$this->request = property_exists($this->kv, 'request') ? $this->kv->request : 'undeclared';
		$this->reply = property_exists($this->kv, 'reply') ? $this->kv->reply : 'undeclared';
		$this->dns_server = property_exists($this->kv, 'dns_server') ? $this->kv->dns_server : 'undeclared';
		$this->throttle = property_exists($this->kv, 'throttle') ? $this->kv->throttle : 'undeclared';
	}

	//-------------------------------------------
	// The perform function loops through all queries that have been concatenated together
	public function perform()
	{
		// process the concatentated list of queries
		$parts = explode( '|', $this->request);
		foreach ($parts as $one_name)
		{
			$one_name = trim($one_name);
			$this->resolve($one_name);
		}
	}
	
	//-------------------------------------------
	// The perform function performs a DNS lookup using the dig utility to verify
	// that the request resolves to the specified reply
	public function resolve( $request )
	{
		$expected_reply = $this->reply;
		$dns_server = $this->dns_server != 'undeclared' ? "@{$this->dns_server}" : '';
		$record_type = $this->record_type;
		
		// delay for N seconds, so that we don't burst the DNS server with requests and get banned
		if( $this->throttle != 'undeclared' )
			sleep( $this->throttle );
		
		// 2>&1 redirects stderr to stdout
		$command = "dig +short $dns_server $request $record_type 2>&1";
		$response = shell_exec( $command );
		
		// $reponse is one or more lines containing IP addresses, or domain names
		$response = trim($response);
		$response_no_ws = str_replace( array(' ',"\t"), '', $response );
		$pos = strpos( $response_no_ws, $this->reply );
		
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__ . "({$request})", 'reply', $expected_reply, $response );
	}

}

?>
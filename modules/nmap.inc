<?php
#==============================================================================
#
# File:         /var/vermont/modules/nmap.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking open and closed TCP/UDP ports
#
# Author:       Joe Honton © 2011
#
# Initial date: January 16, 2011
#
#
#     Keyword               Value
# ---------------	-----------------------
# ip_address        one or more IP addresses, concatenated
# open_tcp_ports    a concatenated list list of ports to check, using TCP, to verify that they are open, may also include ranges.
# open_udp_ports    a concatenated list list of ports to check, using UDP, to verify that they are open,  may also include ranges.
# closed_tcp_ports  a concatenated list list of ports to check, using TCP, to verify that they are closed, may also include ranges.
# closed_udp_ports  a concatenated list list of ports to check, using UDP, to verify that they are closed,  may also include ranges.
# hidden_versions  Check to see if any open ports provide a response that includes application version information.
#                   true or false, defaults to false.  If true, perform the check, if false do not perform the check.
#                   Log an error if the check is performed and version info is revealed.
#
#==============================================================================


class vmtNmap extends vmtModule
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['ip_address'] = 'required';
		$this->vocabulary['open_tcp_ports'] = 'optional';
		$this->vocabulary['open_udp_ports'] = 'optional';
		$this->vocabulary['closed_tcp_ports'] = 'optional';
		$this->vocabulary['closed_udp_ports'] = 'optional';
		$this->vocabulary['hidden_versions'] = 'optional';
		
		$this->ip_address = property_exists($this->kv, 'ip_address') ? $this->kv->ip_address : 'undeclared';
		$this->open_tcp_ports = property_exists($this->kv, 'open_tcp_ports') ? $this->kv->open_tcp_ports : 'undeclared';
		$this->open_udp_ports = property_exists($this->kv, 'open_udp_ports') ? $this->kv->open_udp_ports : 'undeclared';
		$this->closed_tcp_ports = property_exists($this->kv, 'closed_tcp_ports') ? $this->kv->closed_tcp_ports : 'undeclared';
		$this->closed_udp_ports = property_exists($this->kv, 'closed_udp_ports') ? $this->kv->closed_udp_ports : 'undeclared';
		$this->hidden_versions = property_exists($this->kv, 'hidden_versions') ? $this->kv->hidden_versions : 'false';
	}

	//-------------------------------------------
	// The perform function
	public function perform()
	{
		// process the concatentated list of ip_addresses
		$parts = explode( '|', $this->ip_address);
		foreach ($parts as $ip_address)
		{
			$ip_address = trim($ip_address);
			
			if ($this->open_tcp_ports != 'undeclared')
				$this->verify_port_state( $ip_address, $this->open_tcp_ports, 'tcp', 'open' );
			
			if ($this->open_udp_ports != 'undeclared')
				$this->verify_port_state( $ip_address, $this->open_udp_ports, 'udp', 'open' );
			
			if ($this->closed_tcp_ports != 'undeclared')
				$this->verify_port_state( $ip_address, $this->closed_tcp_ports, 'tcp', 'closed' );
			
			if ($this->closed_udp_ports != 'undeclared')
				$this->verify_port_state( $ip_address, $this->closed_udp_ports, 'udp', 'closed' );
		}
	}
	
	//-------------------------------------------
	// The verify_port_state function
	//> $ip_address the host to scan
	//> $ports is a comma-separated list of ports to scan; ranges are also allowed.
	//> $protocol is 'tcp or 'udp'
	//> $state is 'open' or 'closed'
	//< does not return a value
	//
	public function verify_port_state( $ip_address, $ports, $protocol, $state )
	{
		// replace all pipes with a commas; this will allow the specification of multiple ports using regular concatenation rules
		$ports = str_replace( '|', ',', $ports );
		$a = $this->scan_ports( $ip_address, $ports, $protocol, $state );
		
		foreach ($a as $one_port)
		{
			if ( count($one_port) < 7 )
				return $this->runtime_error( __METHOD__, '', "Nmap failed: " . $one_port[0] );

			$response_port = $one_port[0];
			$response_state = $one_port[1];
			$response_protocol = $one_port[2];
			$response_version = $one_port[6];
			
			if ( strpos( $response_state, $state ) === false )		// the response may be something like 'open|filtered'
			{
				$reason = "$ip_address $response_protocol port $response_port state='$response_state'";
				$this->validation_criteria_failed( __METHOD__ . "($response_port)", '', '', $reason );
			}
			
			if ( $this->hidden_versions == 'true' && $protocol == 'tcp' && $state == 'open' )
			{
				$found = preg_match( '/[0-9].*[0-9]/', $response_version );
				if ( $found == 1 )
				{
					$reason = "$ip_address $response_protocol port $response_port exposed version='$response_version'";
					$this->validation_criteria_failed( __METHOD__ . "($response_port)", '', '', $reason );
				}
			}
		}
	}
	
	//-------------------------------------------
	//^ The scan_ports function
	//> $ip_address the host to scan
	//> $ports is a comma-separated list of ports to scan; ranges are also allowed.
	//> $protocol is 'tcp or 'udp'
	//> $state is 'open' or 'closed'
	//< returns a matrix where each row is a port with seven columns for:
	//    port number / state / protocol / owner / service / rpc info / version info
	//
	// See http://www.unspecific.com/nmap-oG-output/
	//
	public function scan_ports( $ip_address, $ports, $protocol, $state )
	{
		$sSU = ($protocol == 'tcp') ? '-sS' : '-sU';
		$sV = ($this->hidden_versions == 'true' && $protocol == 'tcp' && $state == 'open') ? '-sV' : '';
		
		// -oG - is grepable output sent to stdout
		// 2>&1 redirects stderr to stdout
		$command = "nmap -p$ports $sSU $sV -oG - $ip_address 2>&1";
		$response = shell_exec( $command );
		
		$response = trim($response);
		$response = str_replace( "\n", '|', $response );						// from multiline to single line

		$header = 'Ports: ';
		$header_pos = strpos( $response, $header );
		if ( $header_pos !== false )
		{
			$response = substr( $response, $header_pos + strlen($header) );		// everything after the header
			
			$footer = '# Nmap';
			$footer_pos = strpos( $response, $footer );
			if ( $footer_pos !== false )
				$response = substr( $response, 0, $footer_pos-1 );
		}
		
		// when requesting multiple ports, they will need to be split up by looking for the comma separator
		$a = array();
		$i = 0;
		$ports = explode( ', ', $response );
		foreach ( $ports as $one_port )
		{
			$one_port = trim($one_port);
			if ( $one_port != '' )
			{
				$a[$i] = array();
				
				// split the seven parts of the grepable output
				$columns = explode( '/', $one_port );
				$j = 0;
				foreach ( $columns as $one_column )
				{
					$a[$i][$j] = trim($one_column);
					$j++;
				}
				$i++;
			}
		}
		
		return $a;
	}

}

?>
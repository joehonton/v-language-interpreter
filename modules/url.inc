<?php
#==============================================================================
#
# File:         /var/vermont/modules/url.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for verifying whether URL's are reachable, blocked,
#               or not found.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 29, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# url               the URL of the resource to look up, including the protocol, like http://www.postvitae.com
# http_status       the HTTP status code expected
# follow            follow 302 redirects; true or false, defaults to true
# slow_response     fail if response takes more than this many milliseconds; optional, defaults to false
# simulate_cache    A time difference, in seconds, of the (simulated) most recent request for this page.
#                   Use 60*60*24 for 1 day ago.
#                   Use the default value of 'false' to skip this simulation.
#                   The expected http_status will be 302, anything else, such as 200, will trigger an error.
#
#==============================================================================

/* Common HTTP status codes:
200 OK
302 Found (redirected)			# this will only occur if 'follow = false'
304 Not modified (cached)		# this will only occur if 'simulate_cache' is specified
403 Forbidden
404 Not found
*/


class vmtUrl extends vmtModule
{
	public $cache_seconds = 0;
		
	//-------------------------------------------
	public function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['url'] = 'required';
		$this->vocabulary['http_status'] = 'optional';
		$this->vocabulary['follow'] = 'optional';
		$this->vocabulary['slow_response'] = 'optional';
		$this->vocabulary['simulate_cache'] = 'optional';
		
		$this->url = property_exists($this->kv, 'url') ? $this->kv->url : 'undeclared';
		$this->http_status = property_exists($this->kv, 'http_status') ? $this->kv->http_status : 'undeclared';
		$this->follow = property_exists($this->kv, 'follow') ? $this->kv->follow : 'true';
		$this->slow_response = property_exists($this->kv, 'slow_response') ? $this->kv->slow_response : 'false';
		$this->simulate_cache = property_exists($this->kv, 'simulate_cache') ? $this->kv->simulate_cache : 'false';
		
		if ( $this->simulate_cache != 'false' )
			eval("\$this->cache_seconds = floor({$this->simulate_cache});");
	}

	//-------------------------------------------
	// The perform function
	public function perform()
	{
		$parts = explode( '|', $this->url );
		
		foreach ( $parts as $one_url )
			$this->verify( $one_url );
	}

	//-------------------------------------------
	// The verify function
	public function verify( $url )
	{
		$ch = curl_init();
		curl_setopt( $ch, CURLOPT_URL, $url );
		curl_setopt( $ch, CURLOPT_HEADER, true );
		curl_setopt( $ch, CURLOPT_TIMEOUT, 10 );	// 10 seconds
		curl_setopt( $ch, CURLOPT_RETURNTRANSFER, true );
		
		// follow 302 responses
		if ( $this->follow == 'false' )
			curl_setopt( $ch, CURLOPT_FOLLOWLOCATION, false );
		else
			curl_setopt( $ch, CURLOPT_FOLLOWLOCATION, true );
		
		// when checking response times, get the page body, otherwise just issue a request for HEAD
		if ( $this->slow_response == 'false' )
			curl_setopt( $ch, CURLOPT_NOBODY, true );
		else
			curl_setopt( $ch, CURLOPT_NOBODY, false );
		
		// when simulating a conditional client request for a potential cached response
		if ( $this->simulate_cache != 'false' )
		{
			curl_setopt( $ch, CURLOPT_FILETIME, true );
			curl_setopt( $ch, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE );
    		curl_setopt( $ch, CURLOPT_TIMEVALUE, time() - $this->cache_seconds );
		}
			
		$data = curl_exec( $ch );
		$status_code = curl_getinfo( $ch, CURLINFO_HTTP_CODE );
		$response_time = floor( curl_getinfo( $ch, CURLINFO_TOTAL_TIME ) * 1000);	// convert seconds to milliseconds
		$filetime_ts = curl_getinfo( $ch, CURLINFO_FILETIME );
		curl_close($ch);
		
		if ( $data == false )
			return $this->runtime_error( __METHOD__, 'url', "operation failed, check the domain name for validity" );
		
		if ( $this->simulate_cache != 'false' && ($this->http_status != $status_code) )
		{
			$cachetime_fmt = strftime( "%c", time() - $this->cache_seconds );
			$filetime_fmt = strftime( "%c", $filetime_ts );
			
			if ($status_code == 200)
				$extra_msg = "Server file $filetime_fmt, is more recent than cached file $cachetime_fmt";
			else if ($status_code == 304)
				$extra_msg = "The cached file $cachetime_fmt is still valid";
			else
				$extra_msg = '';
			
			return $this->validation_attribute_mismatch( __METHOD__ . "({$url})", 'http_status', $this->http_status, $status_code, $extra_msg );
		}
					
		if ( ($this->http_status != 'undeclared') && ($this->http_status != $status_code) )
			return $this->validation_attribute_mismatch( __METHOD__ . "({$url})", 'http_status', $this->http_status, $status_code );

		if ( ($this->slow_response != 'false') && ($response_time > $this->slow_response) )
			return $this->validation_criteria_failed( __METHOD__ . "({$url})", 'slow_response', $this->slow_response, "actual response time was $response_time ms" );
			
		return true;
	}

}

?>

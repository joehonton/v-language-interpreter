<?php
#==============================================================================
#
# File:         /var/vermont/modules/resource-modules/cpu.inc
#
# Language:     Vermont resource module class
#
# Contents:     A resource module for obtaining average CPU load over 1, 5, 15
#               minute periods.
#
# Author:       Joe Honton © 2011
#
# Initial date: February 7, 2011
#
#==============================================================================

class vmtResourceCpu extends vmtResourceBase
{
	//-------------------------------------------
	public function __construct()
	{
		$vocabulary = array( 'average_load_1',		// percent
							 'average_load_5',		// percent
							 'average_load_15' );	// percent
	}
	
	//-------------------------------------------
	// The evaluate function is called by the vmtResource::perform() function
	//
	public function evaluate( vmtManagedObject $o )
	{
		// get all CPU load average statistics in one call
		$cpustats = $this->proc_loadavg();
		
		foreach( $o->kv as $resource => $value )			// myvar = average_load_15
		{
			switch( $value )
			{
				case 'average_load_1':
					$o->kv[$resource] = floor( $cpustats[0] * 100);
					break;
			
				case 'average_load_5':
					$o->kv[$resource] = floor( $cpustats[1] * 100);
					break;
				
				case 'average_load_15':
					$o->kv[$resource] = floor( $cpustats[2] * 100);
					break;
					
				default;
					break;
			}
		}
	}

	//-------------------------------------------
	//^ The proc_loadavg function will obtain the 1, 5, and 15 minute averages in a single call
	//< returns a simple array containing the values
	//
	public function proc_loadavg()
	{
		$f = fopen( "/proc/loadavg", 'r' );
		if ( $f == null )
			return array( 1, 1, 1 );	// defaults in case of problem
		
		// /proc/loadavg contains a single line, like:
		//  0.16 0.08 0.02 1/130 3787
		$cpustats = explode( ' ', fgets($f) );
		fclose( $f );
		return $cpustats;
	}
}
?>
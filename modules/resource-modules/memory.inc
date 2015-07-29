<?php
#==============================================================================
#
# File:         /var/vermont/modules/resource-modules/memory.inc
#
# Language:     Vermont resource module class
#
# Contents:     A resource module for obtaining instantaneous memory statistics
#               including physical memory usage and virtual memory swapfile usage.
#
# Author:       Joe Honton © 2011
#
# Initial date: February 4, 2011
#
#==============================================================================

class vmtResourceMemory extends vmtResourceBase
{
	//-------------------------------------------
	public function __construct()
	{
		$vocabulary = array( 'physical_memory_size',		// Kb
							 'physical_memory_used',		// Kb
							 'physical_memory_free',		// Kb
							 'physical_memory_percent',		// percent free
							 'virtual_memory_size',			// Kb
							 'virtual_memory_used',			// Kb
							 'virtual_memory_free',			// Kb
							 'virtual_memory_percent' );	// percent free
	}
	
	//-------------------------------------------
	// The evaluate function is called by the vmtResource::perform() function
	//
	public function evaluate( vmtManagedObject $o )
	{
		// get all instantaneous memory statistics in one call
		$memstats = $this->proc_meminfo();
		
		foreach( $o->kv as $resource => $value )			// myvar = physical_memory_size
		{
			switch( $value )
			{
				case 'physical_memory_size':
					if ( array_key_exists( 'MemTotal', $memstats ) )
						$o->kv[$resource] = $memstats['MemTotal'];
					break;
			
				case 'physical_memory_used':
					if ( array_key_exists( 'MemTotal', $memstats ) && array_key_exists( 'MemFree', $memstats ) )
						$o->kv[$resource] = $memstats['MemTotal'] - $memstats['MemFree'];
					break;
					
				case 'physical_memory_free':
					if ( array_key_exists( 'MemFree', $memstats ) )
						$o->kv[$resource] = $memstats['MemFree'];
					break;
					
				case 'physical_memory_percent':
					if ( array_key_exists( 'MemTotal', $memstats ) && array_key_exists( 'MemFree', $memstats ) )
						$o->kv[$resource] = floor($memstats['MemFree'] / $memstats['MemTotal'] * 100.0);
					break;
					
				case 'virtual_memory_size':
					if ( array_key_exists( 'SwapTotal', $memstats ) )
						$o->kv[$resource] = $memstats['SwapTotal'];
					break;
			
				case 'virtual_memory_used':
					if ( array_key_exists( 'SwapTotal', $memstats ) && array_key_exists( 'SwapFree', $memstats ) )
						$o->kv[$resource] = $memstats['SwapTotal'] - $memstats['SwapFree'];
					break;
					
				case 'virtual_memory_free':
					if ( array_key_exists( 'SwapFree', $memstats ) )
						$o->kv[$resource] = $memstats['SwapFree'];
					break;
					
				case 'virtual_memory_percent':
					if ( array_key_exists( 'SwapTotal', $memstats ) && array_key_exists( 'SwapFree', $memstats ) )
						$o->kv[$resource] = floor($memstats['SwapFree'] / $memstats['SwapTotal'] * 100.0);
					break;
					
				default;
					break;
			}
		}
	}

	//-------------------------------------------
	//^ The proc_meminfo function will obtain all resource attributes in a single call
	//< returns an associative array containing all of the values obtained by reading /proc/meminfo
	//  where the number returned is in units of kilobytes
	//  For reference see http://www.redhat.com/advice/tips/meminfo.html
	//
	public function proc_meminfo()
	{
		$memstats = array();
		
		$f = fopen( "/proc/meminfo", 'r' );
		if ( $f == null )
			return $memstats;
		
		while ($line = fgets($f) )
		{
			$dual = explode( ':', $line );
			$memory_keyword = trim( $dual[0] );
			$instantaneous_value = trim( $dual[1] );
			$pos_kb = strpos( $instantaneous_value, 'kB' );
			if ( $pos_kb !== false )
				$instantaneous_value = trim(substr( $instantaneous_value, 0, $pos_kb ));		// strip off the 'kB'
				
			$memstats[$memory_keyword] = $instantaneous_value;
		}
		
		fclose( $f );
		return $memstats;
	}
}
?>
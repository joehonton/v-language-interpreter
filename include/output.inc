<?php
//=============================================================================
//
// File:         /var/vermont/include/output.inc
//
// Language:     PHP
//
// Contents:     Output functions: stdout, stderr, stdverbose
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//=============================================================================
// Output functions
//=============================================================================

//-----------------------------------------------
function stdout( $s )
{
	if (!QUIET)
		echo "$s\n";
}


//-----------------------------------------------
function stdverbose( $s )
{
	if (!QUIET)
		echo "$s\n";

	stdlog( $s );
}


//-----------------------------------------------
function stderr( $s )
{
	if (!QUIET)
		echo "$s\n";

	stdlog( $s );
}


//-----------------------------------------------
function stdlog( $s )
{
	if (!NOLOG)
	{
		global $fstderr;
		if ( $fstderr === false )
		{
			global $config;
			$log_file = $config->kv['log_file'];
			$fstderr = fopen( $log_file, 'a+' );
			if ( $fstderr === false )
			{
				echo "Unable to open output log file $log_file\n";
				exit;
			}
		}
		
		fputs( $fstderr, sprintf( "%s %s\n", strftime('%Y-%m-%d %H:%M:%S'), $s ) );
	}
}

?>
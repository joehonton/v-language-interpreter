<?php
//=============================================================================
//
// File:         /var/vermont/include/commandline.inc
//
// Language:     PHP
//
// Contents:     Function for parsing the command line into an array or arrays
//               keyed on: 'commands', 'options', 'flags', 'arguments'.
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//=============================================================================
// Command line
//=============================================================================

//-----------------------------------------------
function ParseCommandline()
{
	global $commandline;
	$commandline = Arguments( $_SERVER['argv'] );

	if ( in_array('q', $commandline['flags']) || array_key_exists('quiet', $commandline['options']) )
		define( 'QUIET', true );
	else
		define( 'QUIET', false );
		
	if ( in_array('t', $commandline['flags']) || array_key_exists('tabs', $commandline['options']) )
		define( 'TABS', true );
	else
		define( 'TABS', false );
		
	if ( in_array('z', $commandline['flags']) || array_key_exists('suppress_log', $commandline['options']) )
		define( 'NOLOG', true );
	else
		define( 'NOLOG', false );
		
	if ( in_array('v', $commandline['flags']) || array_key_exists('version', $commandline['options']) )
	{
		stdout( "vv version 1.0" );
		exit;
	}
	if( QUIET && NOLOG )
	{
		echo( "-q --quiet and -z --suppress_log are mutually exclusive.  You must not specify both of them.\n" );
		exit;
	}
	
	$validLongOptions = array(
			'directory',
			'files',
			'help',
			'quiet',
			'rulebook',
			'tabs',
			'version',
			'noexec',
			'suppress_log'
	);
			
	$hasUnknownOptions = false;
	foreach( $commandline['options'] as $key => $value )
	{
		if ( !in_array( $key, $validLongOptions ) )
		{
			$hasUnknownOptions = true;
			echo( sprintf( "vv: unknown argument '--%s'\n", $key ) );
		}
	}
		
	$unknown_args = preg_grep( "/[dfhqrtvxz]/", $commandline['flags'], PREG_GREP_INVERT );
	if ( count($unknown_args) > 0 )
	{
		$hasUnknownOptions = true;
		foreach ( $unknown_args as $key => $value )
			echo( sprintf( "vv: unknown argument '-%s'\n", $value ) );
	}
	
	// convert the flag "-r filename" into the options "--rulebook=filename"
	// convert the flag "-d directory" into the options "--directory=path"
	// convert the flag "-f files" into the options "--files=filenames"
	$commands_index = 0;
	for( $i = 0; $i < count($commandline['flags']) && $commands_index < count($commandline['commands']); $i++ )
	{
		if ( $commandline['flags'][$i] == 'r' )
		{
			$commandline['options']['rulebook'] = $commandline['commands'][$commands_index];
			$commands_index++;
		}
		if ( $commandline['flags'][$i] == 'd' )
		{
			$commandline['options']['directory'] = $commandline['commands'][$commands_index];
			$commands_index++;
		}
		if ( $commandline['flags'][$i] == 'f' )
		{
			$commandline['options']['files'] = $commandline['commands'][$commands_index];
			$commands_index++;
		}
	}
	
	if ( $hasUnknownOptions == true  ||
		 in_array( 'h', $commandline['flags'] ) ||
		 array_key_exists('help', $commandline['options']) )
	{
		stdout( "usage: vv -dfhqrtvxz" );
		stdout( "Specify arguments using the short form, like '-r filename' or the long form, like '--rulebook=filename'" );
		stdout( " --rulebook     -r name of the rulebook options file, defaults to 'rulebook-options';" );
		stdout( "                   may be relative to the '--directory' argument or may be a fully qualified filename" );
		stdout( " --directory    -d path to the rulebook options file and the namespace object files" );
		stdout( " --files        -f {namespace_file1,namespace_file2,...} filenames within the '--directory' to execute, defaults to all files." );
		stdout( "                   (Filenames starting with # are always ignored.)" );
		stdout( " --help         -h Show this help text" );
		stdout( " --tabs         -t Tab separated output format, suitable for grep or awk" );
		stdout( " --version      -v Display version" );
		stdout( " --noexec       -x Parse namespaces, but do not execute" );
		stdout( " --quiet        -q Quiet mode, send errors to /var/log/vli.log only, not to stdout" );
		stdout( " --suppress_log -z Suppress error logging to /var/log/vli.log" );
		stdout( "" );
		stdout( "For example, to verify objects from a single namespace file with a given rulebook options file, use: " );
		stdout( "[user@host]# vv --directory=/path/to/files --rulebook=my-rulebook --files=my-object-rules.v" );
		stdout( "" );
		stdout( "To verify all objects within a given directory, using a standard rulebook options file named 'rulebook-options', use: " );
		stdout( "[user@host]# vv --directory=/path/to/files" );
		stdout( "" );
		exit;
	}

}

//-----------------------------------------------
function Arguments( $args )
{
	array_shift( $args );
	$endofoptions = false;
	
	$ret = array
	(
		'commands' => array(),
		'options' => array(),
		'flags'    => array(),
		'arguments' => array(),
	);
	
	while ( $arg = array_shift($args) )
	{
		// if we have reached end of options,
		// we cast all remaining argvs as arguments
		if ($endofoptions)
		{
			$ret['arguments'][] = $arg;
			continue;
		}
		
		// Is it a command? (prefixed with --)
		if ( substr( $arg, 0, 2 ) === '--' )
		{
			// is it the end of options flag?
			if (!isset ($arg[3]))
			{
				$endofoptions = true;; // end of options;
				continue;
			}
			
			$value = "";
			$com   = substr( $arg, 2 );
			// is it the syntax '--option=argument'?
			if (strpos($com,'='))
				list($com,$value) = split("=",$com,2);

			$ret['options'][$com] = !empty($value) ? $value : true;
			continue;
		}
		
		// Is it a flag or a serial of flags? (prefixed with -)
		if ( substr( $arg, 0, 1 ) === '-' )
		{
			for ($i = 1; isset($arg[$i]) ; $i++)
				$ret['flags'][] = $arg[$i];
			continue;
		}
		
		// finally, it is not option, nor flag, nor argument
		$ret['commands'][] = $arg;
		continue;
	}
	
	if (!count($ret['options']) && !count($ret['flags']))
	{
		$ret['arguments'] = array_merge($ret['commands'], $ret['arguments']);
		$ret['commands'] = array();
	}
	
	return $ret;
}

?>
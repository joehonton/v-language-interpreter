<?php
//=============================================================================
//
// File:         /var/vermont/include/namespaces.inc
//
// Language:     PHP
//
// Contents:     Function for parsing the namespace configuration files into
//               memory. Namespaces are indexed by a composite name which is
//               assembled by concatenating a namespace and an object, separated
//               by a colon, like [namespace:object].
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//=============================================================================
// Namespaces
//=============================================================================

//-----------------------------------------------
//^ The ReadNamespaces function finds all files in the --directory and parses them
//  into vmtManagedObject's.  If the --files commandline or configuration directive
//  is specified, read only those files.
//
function ReadNamespaces()
{
	global $config;
	global $namespace_stage;
	
	$directory = $config->kv['directory'];
	if ( !file_exists( $directory ) )
	{
		stderr( "'$directory' does not exist; use the '--directory' argument to specify which namespace files to read." );
		return;
	}
	
	// see if the 'files' argument was specified on the command line or in the configuration file
	if ( $config->kv['files'] != '' )
	{
		$specified_filenames = explode( ',', $config->kv['files'] );
		foreach( $specified_filenames as $file )
			ReadOneNamespaceFile( $directory . '/' . $file );
	}
	
	// read all namespace files in the directory
	else
	{
		$files = glob( $directory . '/*' );
		foreach( $files as $file )
		{
			// don't read files that begin with #, this is the "comment out this file" indicator
			$basename = basename($file);
			if ( $basename{0} != '#' )
				ReadOneNamespaceFile( $file );
		}
	}

	VerboseBaseObjects( $config, $namespace_stage );
}


//-----------------------------------------------
function ReadOneNamespaceFile( $filename )
{
	global $namespace_stage;
	global $reserved_keywords;
	
	if ( !file_exists($filename) )
	{
		stderr( "Missing namespace file $filename, skipping . . ." );
		return;
	}
	$f = fopen( $filename, 'r' );
	if ( $f == NULL )
	{
		stderr( "Unable to open namespace file $filename, skipping . . ." );
		return;
	}

	// keep these for execution specification error messages
	$base_filename = basename($filename);
	$line_number = 0;
	
	// these three change state whenever a bracketed object name is encountered
	$namespace_name = $base_filename;								// this is a default until the special [collection] name = abcd is encountered
	$object_name = 'anonymous';										// if the configuration file contains keyword value pairs before the first bracketed object, use this name
	$co_name = sprintf( "%s:%s", $namespace_name, $object_name );	// defaults to -> base_filename:anonymous
			
	while ( ($line = fgets($f)) !== false )
	{
		$line_number++;
		
		// remove comments
		$hash = strpos( $line, '#' );
		if ( $hash !== false )
			$line = substr( $line, 0, $hash );
		
		// each time a bracketed object name is encountered at the beginning of a line, like [efgh] or [abcd:efgh] or [abcd:abcd], switch to a new managed object
		$left_bracket = strpos( $line, '[' );
		$right_bracket = strpos( $line, ']' );
		if ($left_bracket !== false && $right_bracket !== false && $left_bracket == 0 )
		{
			$bracketed_name = substr( $line, $left_bracket+1, ($right_bracket - $left_bracket) - 1 );
			$colon = strpos( $bracketed_name, ':' );
			
			// fully qualified bracketed name
			if ( $colon !== false )															// abcd:efgh or abcd:abcd or :abcd
			{
				$namespace_name = trim( substr( $bracketed_name, 0, $colon ) );				// abcd			or namespace
				$object_name = trim( substr( $bracketed_name, $colon+1 ) );					// efgh			or abcd

				if ( $namespace_name == '' )												// :abcd
					$namespace_name = $object_name;
				if ( $object_name == '' )													// abcd:
					$object_name = $namespace_name;
			}
			
			// unqualified bracketed name, like [efgh]
			else
				$object_name = trim( $bracketed_name );
			
			// this namespace:object may already exist, create it only if it doesn't already exist
			$co_name = sprintf( "%s:%s", $namespace_name, $object_name );	// abcd:efgh
			if ( !array_key_exists( $co_name, $namespace_stage ) )
				$namespace_stage[$co_name] = new vmtManagedObject( $namespace_name, $object_name );
		}
		
		// all keyword-value pairs are assigned to the current namespace:object
		else
		{
			// split into keyword and value
			$equal_sign = strpos( $line, '=' );
			if ( $equal_sign !== false )
			{
				// if no bracketed object_name has been defined yet, we'll need to create an anonymous object to hold this keyword value pair
				if ( $object_name == 'anonymous' && !array_key_exists( $co_name, $namespace_stage ) )
					$namespace_stage[$co_name] = new vmtManagedObject( $namespace_name, $object_name );
				
				// += means concatenate
				$bConcatenate = ($equal_sign > 0 && $line{$equal_sign-1} == '+') ? true : false;

				// split the line into keyword and value
				$keyword_len = ($bConcatenate == true) ? $equal_sign-1 : $equal_sign;
				$keyword = trim( substr( $line, 0, $keyword_len ) );
				$value_string = trim( substr( $line, $equal_sign+1 ) );
				
				// the value-string may consist of a pipe-separated list of values, process them separately to eliminate possible duplicates
				$values = explode( '|', $value_string );
				foreach ( $values as $index => $v )
					$values[$index] = trim($v);
				
				if ( in_array( $keyword, $reserved_keywords ) )
					stderr( sprintf( "<%s+%s> [%s:%s] '%s' is a reserved word", $base_filename, $line_number, $namespace_name, $object_name, $keyword ) );
				else
				{
					// add the attribute to the keyword-value array
					$o = $namespace_stage[$co_name];
					if ( $bConcatenate == false )
						unset( $o->kv[$keyword] );
						
					ConcatenateValue( $o, $keyword, $value_string );
						
					// and add the concatenation/assign operator to the array (for the first occurence only)
					// (this will be used during the ComposeObject routine)
					if ( !array_key_exists( $keyword, $o->concatenate ) )
						$o->concatenate[$keyword] = ($bConcatenate == true) ? '+=' : '=';
					
					// and add the attribute to the line-number tracking array
					$fl = sprintf( "<%s+%s>", $base_filename, $line_number );
					if ( array_key_exists( $keyword, $o->tracking ) )
						$o->tracking[$keyword] .= '|' . $fl;
					else
						$o->tracking[$keyword] = $fl;
				}
			}
		}
	}
	
	fclose( $f );
}


//-----------------------------------------------
//^ The ConcatenateValue function properly merges two values,
//  each of which may be blank, may have a single value, or may
//  have already-concatenated values.
//  The result string may be exploded later on using the '|' character as separator.
//> $o is the object whose keyword-value attributes are being manipulated
//> $keyword is the attribute's keyword
//> $value is the new value to concatenate.  Note, to remove a previous value, pass an empty string
//
function ConcatenateValue( $o, $keyword, $value )
{
	$keyword = trim($keyword);
	$value = trim($value);
		
	if ( !array_key_exists( $keyword, $o->kv ) )
		$o->kv[$keyword] = $value;
	else
	{
		// if the new value is an empty string, it will wipe out any previously specified value
		if ( $value == '' )
			unset( $o->kv[$keyword] );

		// if the existing value is an empty string, just assign the new value
		else if ( $o->kv[$keyword] == '' )
			$o->kv[$keyword] = $value;
			
		else
		{
			$parts = explode( '|', $o->kv[$keyword] );
			foreach ( $parts as $index => $part )
				$parts[$index] = trim($part);
			
			// if the new value is already specified as a member of the existing value, skip it
			// otherwise, concatentate the new value onto the end of the existing value
			if ( !in_array( $value, $parts ) )
				$o->kv[$keyword] .= '|' . $value;
		}
	}
}

?>
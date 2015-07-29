<?php
//=============================================================================
//
// File:         /var/vermont/include/composite-objects.inc
//
// Language:     PHP
//
// Contents:     This contains the function to merge namespace objects
//               with the base objects to produce composites.
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//-----------------------------------------------
//^ The MergeIntoCompositeObjects function assembles composite objects by starting with defaults
//  from the configuration file, then overriding those defaults with 'namespace' keyword values,
//  then merging (with possible concatenation) all 'object' keywords.
function MergeIntoCompositeObjects()
{
	global $config;
	global $namespace_stage;
	global $composite_stage;
	
	foreach ( $namespace_stage as $o )
	{
		// skip namespace objects because they should not become part of the composite_stage collection
		if ( $o->isNamespace() )
			continue;
			
		$co_name = sprintf( "%s:%s", $o->namespace_name, $o->object_name );
		$co = new vmtManagedObject( $o->namespace_name, $o->object_name );
		$composite_stage[$co_name] = $co;
		
		// STEP 1:
		// Every object starts with the specified settings defined in the config file
		// They are completely overridden (and never concatenated to) by any optional
		// settings specified later on through the namespace files.
		$co->kv['comment'] = $config->kv['comment'];
		$co->kv['pass'] = $config->kv['pass'];
		$co->kv['fail'] = $config->kv['fail'];
		$co->kv['vocabulary_check'] = $config->kv['vocabulary_check'];
		$co->kv['runtime_trace'] = $config->kv['runtime_trace'];
		$co->kv['output_line_number'] = $config->kv['output_line_number'];
		$co->kv['output_object_name'] = $config->kv['output_object_name'];
		$co->kv['output_comment'] = $config->kv['output_comment'];
		$co->kv['output_method'] = $config->kv['output_method'];
		$co->kv['output_limit'] = $config->kv['output_limit'];

		// do not add any dependencies to the object itself; remove self-referential
		// dependencies by replacing them with an empty string
		$d = $config->kv['dependency'];
		$d = str_replace( sprintf( '[%s:%s]', $o->namespace_name, $o->object_name ), '', $d );
		$d = str_replace( sprintf( '%s:%s', $o->namespace_name, $o->object_name ), '', $d );
		$co->kv['dependency'] = $d;
		
		
		// STEP 2: get this object's namespace collection object
		$cc_name = sprintf( "%s:%s", $o->namespace_name, $o->namespace_name );
		if ( array_key_exists( $cc_name, $namespace_stage ) )
		{
			$cc = $namespace_stage[$cc_name];
			
			// add the "namespace's" keyword-value attributes to the new composite object
			foreach ( $cc->kv as $keyword => $value )
				$co->kv[$keyword] = $value;
				
			// add the "namespace's" line-number tracking array
			foreach ( $cc->tracking as $keyword => $value )
				$co->tracking[$keyword] = $value;
		}

		
		// STEP 3 now merge the base object's keywords into the configuration/namespace keywords
		foreach ( $o->kv as $keyword => $value_string )
		{
			// The $value_string may consist of a pipe-separated list of values,
			// process them separately to eliminate possible duplicates.
			
			// '+=' means concatenate the namespace's attributes with the base object's attributes
			// '=' means discard any namespace-defined attributes
			if ( array_key_exists($keyword, $o->concatenate) && $o->concatenate[$keyword] != '+=' )
				unset( $co->kv[$keyword] );

			ConcatenateValue( $co, $keyword, $value_string );
		}
		
		// and merge the base object's line-number tracking into the namespace's line-number tracking
		foreach ( $o->tracking as $keyword => $value )
		{
			if ( array_key_exists( $keyword, $co->tracking ) )
				$co->tracking[$keyword] .= '|' . $value;
			else
				$co->tracking[$keyword] = $value;
		}
	}

	VerboseCompositeObjects( $config, $composite_stage );
	VerboseObjectLineNumbers($config, $composite_stage );
}

?>
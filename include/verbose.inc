<?php
//=============================================================================
//
// File:         /var/vli/include/verbose.inc
//
// Language:     PHP
//
// Contents:     Functions for writing collections to the standard output based
//               on the 'verbose_*' configuration options.
//
// Author:       Joe Honton © 2011
//
// Initial date: February 6, 2011
//
//=============================================================================


//-----------------------------------------------
function VerboseRulebookOptions( $config, $filename )
{
	if ( $config->kv['verbose_headers'] == 'true' )
		stdverbose( "=== VLI version 1.0 ===" );
	
	if ( $config->kv['verbose_rulebook_options'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s = "\n=== Rulebook options ===\n";
		$s .= "Rulebook options file $filename\n";
		$s .= "{\n";
		foreach( $config->kv as $keyword => $value )
			$s .= sprintf( "  %s = %s\n", $keyword, $value );
		$s .= "}\n";
		stdverbose( $s );
	}
}
	
//-----------------------------------------------
function VerboseBaseObjects( $config, $namespace_stage )
{
	// if verbose, show the managed objects
	if ( $config->kv['verbose_base_objects'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s = "\n=== Base Objects ===";
		foreach( $namespace_stage as $co_name => $o )
		{
			$s .= sprintf( "\n[%s]\n{\n", $co_name );
			foreach( $o->kv as $key => $value )
				$s .= sprintf( "    %s => %s\n", $key, $value );
			$s .= "}";
		}
		stdverbose( $s );
	}
}
	
//-----------------------------------------------
function VerboseCompositeObjects( $config, $composite_stage )
{
	// if verbose, show the composite objects
	if ( $config->kv['verbose_composite_objects'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s = "\n=== Composite objects ===";
		foreach( $composite_stage as $co_name => $o )
		{
			$s .= sprintf( "\n[%s]\n{\n", $co_name );
			foreach( $o->kv as $key => $value )
				$s .= sprintf( "    %s => %s\n", $key, $value );
			$s .= "}";
		}
		stdverbose( $s );
	}
}
	
//-----------------------------------------------
function VerboseObjectLineNumbers( $config, $composite_stage )
{
	if ( $config->kv['verbose_object_line_numbers'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s = "\n=== Object line numbers ===";
		foreach( $composite_stage as $co_name => $o )
		{
			$s .= sprintf( "\n[%s]\n{\n", $co_name );
			foreach( $o->tracking as $key => $value )
				$s .= sprintf( "    %s => %s\n", $key, $value );
			$s .= "}";
		}
		stdverbose( $s );
	}
}

//-----------------------------------------------
function VerboseResourceObjects( $config, $resource_stage )
{
	// if verbose, show the resource objects
	if ( $config->kv['verbose_resource_objects'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s = "\n=== Resource objects ===";
		foreach( $resource_stage as $ro_name => $o )
		{
			$s .= sprintf( "\n[%s]\n{\n", $ro_name );
			foreach( $o->kv as $key => $value )
				$s .= sprintf( "    %s => %s\n", $key, $value );
			$s .= "}";
		}
		stdverbose( $s );
	}
}

//-----------------------------------------------
function VerboseSubstitutionObjects( $config, $substitution_stage )
{
	// if verbose, show the substitution objects
	if ( $config->kv['verbose_substitution_objects'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s = "\n=== Substitution objects ===";
		foreach( $substitution_stage as $co_name => $o )
		{
			$s .= sprintf( "\n[%s]\n{\n", $co_name );
			foreach( $o->kv as $key => $value )
				$s .= sprintf( "    %s => %s\n", $key, $value );
			$s .= "}";
		}
		stdverbose( $s );
	}
}
	
//-----------------------------------------------
function VerboseDependencyTree( $config, $s_tree )
{
	// if verbose, show the dependency tree
	if ( $config->kv['verbose_dependency_tree'] == 'true' )
	{
		if ( $config->kv['verbose_headers'] == 'true' )
			stdverbose( "\n=== Dependency tree ===" );
		stdverbose( $s_tree );
	}
}

//-----------------------------------------------
function VerboseAutoload( $config, $class_name )
{
	if ( $config->kv['verbose_plugins'] == 'true' )
		stdverbose( "autoloading $class_name" );
}


//-----------------------------------------------
function VerboseModules( $config, $plugin_module_files, $plugin_module_classes, $logfile_module_files, $logfile_module_classes, $resource_module_files, $resource_module_classes )
{
	if ( $config->kv['verbose_modules'] == 'true' )
	{
		$s = '';
		if ( $config->kv['verbose_headers'] == 'true' )
			$s .= "\n=== Plugin modules ===\n";
		foreach( $plugin_module_classes as $type => $class_name )
		{
			$fully_qualified_filename = $plugin_module_files[$class_name];
			$s .= sprintf( "%s => %s => %s\n", $type, $class_name, $fully_qualified_filename );
		}
		
		if ( $config->kv['verbose_headers'] == 'true' )
			$s .= "\n=== Logfile modules ===\n";
		foreach( $logfile_module_classes as $type => $class_name )
		{
			$fully_qualified_filename = $logfile_module_files[$class_name];
			$s .= sprintf( "%s => %s => %s\n", $type, $class_name, $fully_qualified_filename );
		}
		
		if ( $config->kv['verbose_headers'] == 'true' )
			$s .= "\n=== Resource modules ===\n";
		foreach( $resource_module_classes as $type => $class_name )
		{
			$fully_qualified_filename = $resource_module_files[$class_name];
			$s .= sprintf( "%s => %s => %s\n", $type, $class_name, $fully_qualified_filename );
		}
		
		stdverbose( $s );
	}
}

?>
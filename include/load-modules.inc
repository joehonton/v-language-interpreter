<?php
//=============================================================================
//
// File:         /var/vermont/include/modules.inc
//
// Language:     PHP
//
// Contents:     Function for parsing the executable modules.
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//-----------------------------------------------
// The autoload function will be used by Execute when instanitating class objects
function __autoload($class_name)
{
	global $plugin_module_files;
	global $logfile_module_files;
	global $resource_module_files;
	global $config;

	VerboseAutoload( $config, $class_name );
		
	if ( array_key_exists( $class_name, $plugin_module_files ) )
		require_once( $plugin_module_files[$class_name] );

	else if ( array_key_exists( $class_name, $logfile_module_files ) )
		require_once( $logfile_module_files[$class_name] );

	else if ( array_key_exists( $class_name, $resource_module_files ) )
		require_once( $resource_module_files[$class_name] );
}


//-----------------------------------------------
//^ The DiscoverAndLoadModules function determines the directory that contains the plugin modules
//  and reads all plugin-modules, all logfile parsing modules, and all resource modules
function DiscoverAndLoadModules()
{
	global $config;
	global $plugin_module_files;
	global $plugin_module_classes;
	global $logfile_module_files;
	global $logfile_module_classes;
	global $resource_module_files;
	global $resource_module_classes;
	
	// start by using the configuration file to determine the location of all modules
	$module_dir = $config->kv['module_dir'];
	if ( !file_exists( $module_dir ) )
	{
		// if the module_dir does not exist as specified, try an alternate default
		stderr( "Missing module directory $module_dir. Switching to the default location: ./modules" );
		$module_dir = dirname(__FILE__) . '/../modules';
		if ( !file_exists( $module_dir ) )
			return;
	}

	// the plugin modules
	$files = glob( $module_dir . '/*' );
	foreach( $files as $file )
	{
		if ( $file != '.' && $file != '..' )
			LoadOneModuleFile( $file, $plugin_module_files, $plugin_module_classes, 'vmt' );
	}

	// the logfile parsing modules
	$files = glob( $module_dir . '/logfile-modules/*' );
	foreach( $files as $file )
	{
		if ( $file != '.' && $file != '..' )
			LoadOneModuleFile( $file, $logfile_module_files, $logfile_module_classes, 'vmtLogfile' );
	}
	
	// the resource modules
	$files = glob( $module_dir . '/resource-modules/*' );
	foreach( $files as $file )
	{
		if ( $file != '.' && $file != '..' )
			LoadOneModuleFile( $file, $resource_module_files, $resource_module_classes, 'vmtResource' );
	}
	
	VerboseModules( $config, $plugin_module_files, $plugin_module_classes, $logfile_module_files, $logfile_module_classes, $resource_module_files, $resource_module_classes );
}


//-----------------------------------------------
//^ The LoadOneModuleFile function add a module file to the two associative arrays
//  for use by __autoload and Execute
//> fully_qualified_filename is a filename obtained from a glob call
//> module_files is a collection of plugin-, logfile-, or resource- files
//> module_classes is a collection of plugin-, logfile-, or resource- classes
//> classPrefix is 'vmt' or 'vmtLogfile' or 'vmtResource'
//
function LoadOneModuleFile( $fully_qualified_filename, &$module_files, &$module_classes, $classPrefix )
{
	if ( !file_exists($fully_qualified_filename) )
	{
		stderr( "Missing module $fully_qualified_filename, skipping . . ." );
		return;
	}

	$type = basename($fully_qualified_filename, '.inc' );				//  /var/vermont/modules/virtual-host.inc --> virtual-host
	$type_without_dashes = str_replace( array('-', '_'), ' ', $type );	//  virtual-host --> virtual host
	$type_uppercased = ucwords( $type_without_dashes );					//  virtual host --> Virtual Host
	$type_no_spaces = str_replace( ' ', '', $type_uppercased );			//  Virtual Host --> VirtualHost
	$class_name = $classPrefix . $type_no_spaces;						//  VirtualHost --> vmtVirtualHost

	if ( !array_key_exists( $class_name, $module_files ) )
		$module_files[$class_name] = $fully_qualified_filename;

	if ( !array_key_exists( $class_name, $module_classes ) )
		$module_classes[$type] = $class_name;
}

?>
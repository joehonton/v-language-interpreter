<?php
//=============================================================================
//
// File:         /var/vermont/include/rulebook-options.inc
//
// Language:     PHP
//
// Contents:     Function for parsing the master rulebook options file
//               into memory.
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//-------------------------------------
//^ The vmtVermontRulebookOptions class is the in-memory representation of the main config file
//  The constructor defaults are used only if the config file does not specify a value and the
//  command line does not prvode an explicit setting.
class vmtVermontRulebookOptions
{
	public $kv = array();
	
	public function __construct()
	{
		$this->kv['directory'] = dirname(dirname(__FILE__)) . '/etc/conf.d'; # all files in this directory are configuration files
		$this->kv['rulebook'] = 'rulebook-options';					# the configuration filename
		$this->kv['files'] = '';									# namespace objects to execute; if not specified, all files in --directory will be executed
		$this->kv['module_dir'] = '/var/vermont/modules';			# all files in this directory instruction modules
		$this->kv['log_file'] = '/var/vermont/log/vermont.log';		# stderr output file

		# options to help with verifying the installation configuration
		$this->kv['verbose_headers'] = 'true';						# add visual line separating headers between verbose sections
		$this->kv['verbose_rulebook_options'] = 'false';			# show this rulebook options file, as it was parsed
		$this->kv['verbose_modules'] = 'false';						# list all installed modules and parsers, as they are discovered during initialization
		$this->kv['verbose_plugins'] = 'false';						# list plugins as they are first autoloaded at runtime
		
		# options to help with debugging object specifications
		$this->kv['verbose_base_objects'] = 'false';				# show all objects, as they are parsed
		$this->kv['verbose_composite_objects'] = 'false';			# show all composite objects, where object and namespace attributes are merged
		$this->kv['verbose_substitution_objects'] = 'false';		# show all objects, where substitution variables have been replaced with values
		$this->kv['verbose_resource_objects'] = 'false';			# show all objects, where dynamic resource values have been substituted
		$this->kv['verbose_object_line_numbers'] = 'false';			# show filenames and line numbers for each parsed attribute of every objet
		$this->kv['verbose_dependency_tree'] = 'false';				# show the object dependency tree
		
		# options to help with runtime operation
		$this->kv['comment'] = 'undeclared';						# A text string that, if specified, is displayed as part of all error messages
		$this->kv['pass'] = 'undeclared';							# A text string, containing replacement variables, that is sent to output for any objects that passes all module tests. The replacement variables may be any module keyword preceded by a dollar sign, or may be the special variables $namespace or $object
		$this->kv['fail'] = 'undeclared';							# A text string, containing replacement variables, that is sent to output for any object that fails to pass all module tests.
		$this->kv['dependency']  = 'undeclared';					# One or more [namespace:object] specifications, concatentated with '|', that specify objects that must be validated before this object is validated.
		$this->kv['vocabulary_check']  = 'strict';					# Should objects be checked against a 'strict' set of allowable keywords, or should 'lax' rules be applied, where only required keyword are checked.
		$this->kv['runtime_trace'] = 'false';						# trace progress as each object is executed
		
		# options to help understand the trip-point output
		$this->kv['output_line_number'] = 'true';					# show the filename and line number, in <filename+linenum> format, of the trip-point object
		$this->kv['output_object_name'] = 'true';					# show the namespace and object name, in [namespace:object] format, of the trip-point object
		$this->kv['output_comment'] = 'true';						# show the value of the object's "comment" keyword, in (comment) format, of the trip-point object
		$this->kv['output_method'] = 'true';						# show the module's method and argument, in ->method(argument) format, of the trip-point object
		$this->kv['output_limit'] = '10';							# show no more than this many errors, optional, default value is 10, use 'false' for no limit
	}
}


//-----------------------------------------------
function ReadRulebookOptions()
{
	global $config;
	global $commandline;
	
	// --directory commandline argument
	if ( array_key_exists( 'directory', $commandline['options'] ) )					// did the user specify --directory or -d
		$directory = $commandline['options']['directory'];
	else
		$directory = dirname(dirname(__FILE__)) . '/etc';
	
	// --rulebook commandline argument
	if ( array_key_exists( 'rulebook', $commandline['options'] ) && strlen($commandline['options']['rulebook']) > 0 )
	{
		if ( $commandline['options']['rulebook']{0} == '/' )
			$filename = $commandline['options']['rulebook'];							// fully qualified filename
		else
			$filename = $directory . '/' . $commandline['options']['rulebook'];;		// relative filename
	}
	else
		$filename = $directory . '/rulebook-options';

	// Read the configuration file and override the constructor's settings
	InnerReadRulebookOptions( $filename );
		
	// Now loop through the command line arguments and override the configuration file settings
	foreach( $commandline['options'] as $key => $value )
		$config->kv[$key] = $value;

	VerboseRulebookOptions( $config, $filename );
}
	
		
//-----------------------------------------------
//^ The InnerReadRulebookOptions function parses the configuration
//  file and override the vmtVermontRulebookOptions constructor's settings
//> $filename is a configuration file
function InnerReadRulebookOptions( $filename )
{
	global $config;
	
	if ( !file_exists($filename) )
	{
		stderr( "Missing configuration file $filename, use the '--directory' and '--rulebook' options to specify its location. Using defaults, for now . . ." );
		return;
	}

	$f = fopen( $filename, 'r' );
	if ( $f == NULL )
	{
		stderr( "Unable to open configuration file $filename, skipping . . ." );
		return;
	}
	
	while ( ($line = fgets($f)) !== false )
	{
		// remove comments
		$hash = strpos( $line, '#' );
		if ( $hash !== false )
			$line = substr( $line, 0, $hash );
		
		// split into keyword and value
		$equal_sign = strpos( $line, '=' );
		if ( $equal_sign !== false )
		{
			$keyword = trim( substr( $line, 0, $equal_sign ) );
			$value = trim( substr( $line, $equal_sign+1 ) );
			$config->kv[$keyword] = $value;
		}
	}
	
	fclose( $f );
}


//-----------------------------------------------
function LogCommandline()
{
	global $argc, $argv;
	$cmdline = '';
	for ( $i = 0; $i < $argc; $i++ )
	{
		if ( $i > 0 )
			$cmdline .= ' ';
		$cmdline .= $argv[$i];
	}
	stdlog( "Start command-line='$cmdline'" );
}

?>
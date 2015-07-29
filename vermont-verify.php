<?php
//=============================================================================
//
// File:         /var/vermont/vermont-verify.php
//
// Language:     PHP
//
// Contents:     Executable script
//
// Author:       Joe Honton © 2010
//
// Initial date: December 15, 2010
//
//=============================================================================

require_once( dirname(__FILE__) . '/include/commandline.inc' );
require_once( dirname(__FILE__) . '/include/rulebook-options.inc' );
require_once( dirname(__FILE__) . '/include/managed-object.inc' );
require_once( dirname(__FILE__) . '/include/namespace-objects.inc' );
require_once( dirname(__FILE__) . '/include/composite-objects.inc' );
require_once( dirname(__FILE__) . '/include/resource-objects.inc' );
require_once( dirname(__FILE__) . '/include/substitution-objects.inc' );
require_once( dirname(__FILE__) . '/include/load-modules.inc' );
require_once( dirname(__FILE__) . '/include/output.inc' );
require_once( dirname(__FILE__) . '/include/verbose.inc' );
require_once( dirname(__FILE__) . '/include/evaluate-resources.inc' );
require_once( dirname(__FILE__) . '/include/validate-rules.inc' );
require_once( dirname(__FILE__) . '/include/assertion.inc' );

$fstderr = false;						// log file
$commandline = null;					// an array of arrays keyed on: 'commands', 'options', 'flags', 'arguments'.
$reserved_keywords = array( 'namespace', 'object' );	// Do not allow these in namespace configuration files
$config = new vmtVermontRulebookOptions();// the in-memory representation of the rulebook options file '/var/vermont/etc/rulebook-options'

$plugin_module_files = array();			// A list of fully qualified module filenames accessible via classname
										//   e.g. $plugin_module_files['vmtVirtualhost'] = '/var/vermont/modules/virtualhost.inc'
$plugin_module_classes = array();		// A list of module class names accessible via 'type'
										//   e.g. $plugin_module_classes['virtualhost'] = 'vmtVirtualhost';
$logfile_module_files = array();		// A list of fully qualified logfile parse filenames accessible via classname
										//   e.g. $logfile_module_files['vmtLogfileIptables'] = '/var/vermont/modules/logfile-modules/iptables.inc'
$logfile_module_classes = array();		// A list of logfile class names accessible via 'logformat'
										//   e.g. $logfile_module_classes['iptables'] = 'vmtLogfileIptables';
$resource_module_files = array();		// A list of fully qualified resource module filenames accessible via classname
										//   e.g. $resource_module_files['vmtResourceMemory'] = '/var/vermont/modules/resource-modules/memory.inc'
$resource_module_classes = array();		// A list of resource module class names accessible via 'subtype'
										//   e.g. $resource_module_classes['memory'] = 'vmtResourceMemory';

$namespace_stage = array();				// array of vmtManagedObjects parsed from the object/namespace files
$composite_stage = array();				// array of vmtManagedObjects where "namespace" attributes and "object" attributes are merged
$substitution_stage = array();			// array of vmtManagedObjects where static substitution variables have been replaced
$resource_stage = array();				// array of vmtManagedObjects where vmtResource dynamic substitution variables have been replaced


// Parse command line arguments which will override any default values specified
// in the rulebook options file.
ParseCommandline();
ReadRulebookOptions();
LogCommandline();

// Discover and load into memory all code modules, by reading them from disk.
DiscoverAndLoadModules();

// Read the namespace-object files from disk instantiating the namespaces object and base objects,
// then merge the namespace and base objects together
ReadNamespaces();
MergeIntoCompositeObjects();

// Copy all composite-stage object to the resource-stage collection,
// and evaluate all vmtResource-derived objects in dependency order.
CopyToResourceStage();
EvaluateResourceObjects();

// Substitute variables with their values
ApplyStaticSubstitutions();

// Loop through all non-vmtResource objects, in dependency order, and validate
// using the keyword-specified rules.
ValidateRuleObjects();
stdlog( "End" );

exit;

	

?>
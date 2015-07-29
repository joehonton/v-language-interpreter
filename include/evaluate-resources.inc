<?php
//=============================================================================
//
// File:         /var/vermont/include/evaluate-resources.inc
//
// Language:     PHP
//
// Contents:     Scan all vmtResource-based objects and evaluate their resource
//               variables.  Start with substitution stage objects and end up
//               with the resource-stage objects.
//
// Author:       Joe Honton © 2011
//
// Initial date: February 5, 2011
//
//=============================================================================


//-----------------------------------------------
//^ The EvaluateResourceObjects scans all vmtResource based objects, in dependency
//  order, and evaluates all resource keywords building a new collection of
//  objects with all dynamic resource variables resolved.
//
function EvaluateResourceObjects()
{
	global $config;
	global $resource_stage;
	
	// evaluate the resource objects, to substitute the keyword value with the current state of the resource
	$depth = 1;
	$s_tree = '';

	// loop over the object collection
	foreach( $resource_stage as $ro_name => $o )
		EvaluateOne( $o, $depth, $s_tree, 'resource', $resource_stage );

	VerboseResourceObjects( $config, $resource_stage );
}

?>
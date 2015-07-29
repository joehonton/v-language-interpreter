<?php
//=============================================================================
//
// File:         /var/vermont/include/resource-objects.inc
//
// Language:     PHP
//
// Contents:     Copy the composite-stage collection to the resource-statge collection
//
// Author:       Joe Honton © 2011
//
// Initial date: February 5, 2011
//
//=============================================================================


//-----------------------------------------------
//^ The CopyToResourceStage copies the composite stage collection to the
//  resource stage collection
//
function CopyToResourceStage()
{
	global $composite_stage;
	global $resource_stage;

	// copy the objects, as they are, from composite stage to resource stage
	foreach ( $composite_stage as $co )
	{
		$ro_name = sprintf( "%s:%s", $co->namespace_name, $co->object_name );
		$ro = new vmtManagedObject( $co->namespace_name, $co->object_name );
		$resource_stage[$ro_name] = $ro;

		// copy each keyword-value pair from the substitution object to the resource object
		foreach ( $co->kv as $keyword => $value )
			$ro->kv[$keyword] = $value;

		// copy the line-number tracking array
		foreach ( $co->tracking as $keyword => $value )
			$ro->tracking[$keyword] = $value;
	}
}

?>
<?php
//=============================================================================
//
// File:         /var/vermont/include/managed-object.inc
//
// Language:     PHP
//
// Contents:     A class that holds a memory representation of a managed object.
//
// Author:       Joe Honton © 2010
//
// Initial date: December 20, 2010
//
//=============================================================================


//-------------------------------------
//^ The vmtManagedObject class holds the in-memory representation of a single
//  managed object's attribute values
class vmtManagedObject
{
	public $kv = array();								// keyword-value pairs like owner = root:committers, becomes $kv['owner'] = 'root:committers';
	public $namespace_name;
	public $object_name;
	public $state;										// The current state: 'initialized', 'in-progress', 'complete'
	public $tracking = array();							// An associative array, keyed on 'keyword', containing the base filename
														// and line number where this keyword-value pairs was specified
	public $concatenate = array();						// An associative array, keyed on 'keyword', containing either '+=' or '=',
														// indicating whether to concatenate or simply assign the value during the ComposeObjects routine
														
	public function __construct( $namespace_name, $object_name )
	{
		$this->namespace_name = $namespace_name;		// The namespace name for this managed object
		$this->object_name = $object_name;				// The object name for this managed object
		$this->state = 'initialized';
	}

	//---------------------------------
	// The isNamespace function returns true for objects like [abcd:abcd]
	// These namespace objects whose keyword-value attribute pairs
	// are inherited, but are never executed themselves.
	//
	public function isNamespace()
	{
		return ( $this->namespace_name == $this->object_name ) ? true : false;
	}

	//---------------------------------
	// The isResource function returns true for objects of type = resource
	public function isResource()
	{
		return ( array_key_exists('type', $this->kv) && $this->kv['type'] == 'resource' ) ? true : false;
	}
}

?>
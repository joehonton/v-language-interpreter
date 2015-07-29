<?php
#==============================================================================
#
# File:         /var/vermont/modules/resource.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for obtaining the current status of specified resources.
#               In contrast to most other modules, this module does not have a
#               controlled vocabulary, instead vocabulary rules are delegated to
#               the resource module defined by the 'request' keyword.
#
# Author:       Joe Honton © 2011
#
# Initial date: February 4, 2011
#
#
#     Keyword               Value
# ---------------	-----------------------
# request           One of the resource modules: 'power', 'cpu', 'process', 'memory',
#                   'file', 'filesystem', 'mysql', 'apache', 'sendmail', 'host', 'interface', 'ip'
#
#==============================================================================


class vmtResource extends vmtModule
{
	
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );

		// override the vmtModule default of 'strict'
		$this->vocabulary_check = 'lax';
		
		$this->vocabulary['request'] = 'required';
		$this->request = property_exists($this->kv, 'request') ? $this->kv->request : 'undeclared';

		// make sure the request type has a module to handle it
		global $resource_module_classes;
		if ( !array_key_exists( $this->request, $resource_module_classes ) )
			return $this->specification_error( __METHOD__, 'request', 'Unknown resource request type' );
	}

	
	//-------------------------------------------
	// The perform function must be defined by every module
	// returns void.  To indicate failure use one of the four status properties.
	public function perform()
	{
		// get the vmtManagedObject
		$o = $this->obj;
		
		// build the list of resources to obtain, this will be every keyword that is not in the standard vocabulary
		foreach( $this->kv as $keyword => $value )
		{
			// skip if this keyword in the strict vmtModule/vmtResource vocabulary
			if ( !array_key_exists($keyword, $this->vocabulary) )
			{
				// the value side of the keyword-value pair becomes the keyword side of the resource-value collection
				$o->rv[$value] = 'unevaluated';
			}
		}
			
		global $resource_module_classes;

		// convert the 'request' attribute into a PHP class name
		$class_name = $resource_module_classes[$this->request];		// memory --> vmtResourceMemory
		$r = new $class_name( $this );								// this uses the __autoload function to find and include class vmtResourceMemory
		$r->evaluate( $o );
	}
	
}

?>
<?php
#==============================================================================
#
# File:         /var/vermont/modules/resource-modules/base.inc
#
# Language:     Vermont resource module class
#
# Contents:     A base class for all resource modules
#
# Author:       Joe Honton © 2011
#
# Initial date: February 4, 2011
#
#==============================================================================

class vmtResourceBase
{
	public $vocabulary = array();		// an array of valid resource requests
	
	//-------------------------------------------
	public function __construct()
	{
	}
	
	//-------------------------------------------
	// The evaluate function is called by the vmtResource::perform() function
	//
	public function evaluate( vmtManagedObject $o )
	{
	}
}
?>
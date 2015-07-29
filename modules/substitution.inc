<?php
#==============================================================================
#
# File:         /var/vermont/modules/substitution.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module designed for use as a general purpose collection of
#               substitution variables. This module does not perform any vocabulary
#               checking, so new keywords can be introduced and used without
#               having to reduce the strict vocabulary checking that all other
#               modules use by default.
#               This module does not perform any validation.
#
# Author:       Joe Honton © 2011
#
# Initial date: Febrary 1, 2011
#
#
#     Keyword               Value
# ---------------	-----------------------
#
#
#==============================================================================


class vmtSubstitution extends vmtModule
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
	}
	
	//-------------------------------------------
	public function pre_perform()
	{
		// override to skip vobabulary checking and return false to prevent execution/validation
		return false;
	}
}

?>
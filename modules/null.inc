<?php
#==============================================================================
#
# File:         /var/vermont/modules/null.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module that does not perform vocabulary checking and does not
#               execute anything. Possible use for this module is during the
#               creation of a namespace object when objects need to be
#               isolated or "commented out" to troubleshoot problems.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 27, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
#
#
#==============================================================================


class vmtNull extends vmtModule
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
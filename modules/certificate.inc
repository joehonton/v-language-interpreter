<?php
#==============================================================================
#
# File:         /var/vermont/modules/certificate.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking that an SSL certificate is installed in the
#               proper location.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 27, 2010
#
#
#          Keyword        	         Value
# -----------------------   -----------------------
# certificate				the path to the certificate itself, optional, but almost certainly needed in order to be meaningful.
# certificate_private_key	the path to the certificate's private key, optional.
# certificate_request		path to the certificat signing request, optional.
# expires_on       		  	the date on which this certificate expires, like 2011-12-31
#
#==============================================================================


class vmtCertificate extends vmtFile
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['path'] = 'optional';		// override this vmtFile keyword, in order to supress the vocabulary warning
		$this->vocabulary['certificate'] = 'optional';
		$this->vocabulary['certificate_private_key'] = 'optional';
		$this->vocabulary['certificate_request'] = 'optional';
		$this->vocabulary['expires_on'] = 'optional';
		
		$this->certificate = property_exists($this->kv, 'certificate') ? $this->kv->certificate : 'undeclared';
		$this->certificate_private_key = property_exists($this->kv, 'certificate_private_key') ? $this->kv->certificate_private_key : 'undeclared';
		$this->certificate_request = property_exists($this->kv, 'certificate_request') ? $this->kv->certificate_request : 'undeclared';
		$this->expires_on = property_exists($this->kv, 'expires_on') ? $this->kv->expires_on : 'undeclared';
	}

	//-------------------------------------------
	// The perform function checks that the certificate, key, and request all exist
	public function perform()
	{
		$this->verify_file( $this->certificate, 'certificate' );
		$this->verify_file( $this->certificate_private_key, 'certificate_private_key' );
		$this->verify_file( $this->certificate_request, 'certificate_request' );
		$this->expires_on( $this->certificate );
	}
	
	//-------------------------------------------
	//^ The verify_document_path function checks that the filename exists
	// and has the proper owner, permissions, security context.
	private function verify_file( $filename, $attribute_keyword )
	{
		if ($filename == 'undeclared')
			return;
		
		if (!file_exists($filename))
			return $this->validation_criteria_failed( __METHOD__, $attribute_keyword, $filename, 'does not exist' );
		else
		{
			$this->owner( $filename );
			$this->permissions( $filename );
			$this->selinux( $filename );
		}
	}

	private function expires_on( $filename )
	{
		if ($this->expires_on == 'undeclared')
			return;
			
		$today_plus_one = time() + (24*60*60);
		$tomorrow = strftime( '%Y-%m-%d', $today_plus_one );
		
		if ( $tomorrow >= $this->expires_on )
			return $this->validation_criteria_failed( __METHOD__, 'expires_on', $filename, "certificate expired on {$this->expires_on}" );
	}
}

?>
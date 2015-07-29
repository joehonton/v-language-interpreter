<?php
#==============================================================================
#
# File:         /var/vermont/modules/file.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking the existence of files and verifying their
#               ownership, permissions, and Security Enhanced Linux context.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 21, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# path              the fully qualified path of the file, like /var/vermont/sandbox/pail
# owner             { owner | owner:group | owner: | :group }
# permissions       the octal notation for the file's ACL, like 777
# selinux           the user:role:type:sensitivity-level, like 'unconfined_u:object_r:user_tmp_t:s0'
# selinux_user      the SELinux user, like 'unconfined_u'
# selinux_role      the SELinux role, like 'object_r'
# selinux_type      the SELinux type, like 'user_tmp_t'
# selinux_level     the SELinux sensitivity level, like 's0'
# max_filesize      the maximum size of the file, in bytes; may be something like 20*1024*1024 to express express 20 megabytes or 3*1024*1024*1024 to express 3 gigabytes
# exists            does the file exist? true or false, defaults to true
#
#==============================================================================


class vmtFile extends vmtModule
{
	//-------------------------------------------
	public function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['path'] = 'required';
		$this->vocabulary['owner'] = 'optional';
		$this->vocabulary['permissions'] = 'optional';
		$this->vocabulary['selinux'] = 'optional';
		$this->vocabulary['selinux_user'] = 'optional';
		$this->vocabulary['selinux_role'] = 'optional';
		$this->vocabulary['selinux_type'] = 'optional';
		$this->vocabulary['selinux_level'] = 'optional';
		$this->vocabulary['max_filesize'] = 'optional';
		$this->vocabulary['exists'] = 'optional';
		
		$this->path = property_exists($this->kv, 'path') ? $this->kv->path : 'undeclared';
		$this->owner = property_exists($this->kv, 'owner') ? $this->kv->owner : 'undeclared';
		$this->permissions = property_exists($this->kv, 'permissions') ? $this->kv->permissions : 'undeclared';
		$this->max_filesize = property_exists($this->kv, 'max_filesize') ? $this->kv->max_filesize : 'undeclared';
		$this->exists = property_exists($this->kv, 'exists') ? $this->kv->exists : 'true';
		
		$this->selinux_user = 'undeclared';
		$this->selinux_role = 'undeclared';
		$this->selinux_type = 'undeclared';
		$this->selinux_level = 'undeclared';
		if ( property_exists($this->kv, 'selinux') )
		{
			$parts = explode( ':', $this->kv->selinux );
			if ( count($parts) > 0 ) $this->selinux_user = $parts[0];
			if ( count($parts) > 1 ) $this->selinux_role = $parts[1];
			if ( count($parts) > 2 ) $this->selinux_type = $parts[2];
			if ( count($parts) > 3 ) $this->selinux_level = $parts[3];
		}
		if ( property_exists($this->kv, 'selinux_user') )
			$this->selinux_user = $this->kv->selinux_user;
		if ( property_exists($this->kv, 'selinux_role') )
			$this->selinux_role = $this->kv->selinux_role;
		if ( property_exists($this->kv, 'selinux_type') )
			$this->selinux_type = $this->kv->selinux_type;
		if ( property_exists($this->kv, 'selinux_level') )
			$this->selinux_level = $this->kv->selinux_level;
	}

	//-------------------------------------------
	// The pre_perform function is called before starting
	// return true to continue, false to halt
	public function pre_perform()
	{
		return parent::pre_perform();
	}
	
	//-------------------------------------------
	// The perform function checks for the existence of files and
	// verifies their ownership, permissions, and SELinux context.
	public function perform()
	{
		// process the concatentated list of document paths
		$parts = explode( '|', $this->path);
		foreach ($parts as $one_path)
		{
			$one_path = trim($one_path);
			if ($one_path != '')
				$this->verify_one_path( $one_path );
		}
	}

	//-------------------------------------------
	// The verify_one_path function
	//< does not return a value
	public function verify_one_path( $one_path )
	{
		if ( $this->exists($one_path) )
		{
			$this->owner($one_path);
			$this->permissions($one_path);
			$this->selinux($one_path);
			$this->max_filesize($one_path);
		}
	}
	
	//-------------------------------------------
	//^ The exists function verifies the existence of the file
	//< returns true or false
	protected function exists($path)
	{
		if ($path == 'undeclared')
			return $this->specification_error( __METHOD__, 'path' );
		
		$rc = file_exists($path);

		if ( $rc == true && $this->exists == 'false' )
			$this->validation_criteria_failed( __METHOD__, 'path', $path, 'file exists, but should not' );

		else if ( $rc == false && $this->exists == 'true' )
			$this->validation_criteria_failed( __METHOD__, 'path', $path, 'does not exist' );
		
		// always return false if the file does not exist so that verify_one_path() won't run any further tests
		return $rc;
	}
	
	//-------------------------------------------
	//^ The owner function verifies the owner and group of the file
	// Specify as "owner" or "owner:" or ":group" or "owner:group"
	//< return void
	protected function owner($path)
	{
		vmtFile::verify_owner( $this, $path, 'owner', $this->owner );
	}
	
	//-------------------------------------------
	//^ The verify_owner function verifies the owner and group of the file
	//> $m is a vmtModule object
	//> $path is the file to check
	//> $attribute_keyword is for error messages, usually 'owner', but may be an something like 'document_owner'
	//> $expected_value is the owner and group, specify as "owner" or "owner:" or ":group" or "owner:group"
	// This is a static function that can be called from outside the class
	//< return void
	public static function verify_owner( vmtModule $m, $path, $attribute_keyword, $expected_value )
	{
		if ($path == 'undeclared' || $expected_value == 'undeclared')
			return;
			
		// split the expected value into the owner and group, if they are both specified,
		// otherwise assume this is the owner only
		$colon = strpos($expected_value,':');
		if ( $colon === false )
		{
			$owner = $expected_value;
			$group = '';
		}
		else
		{
			$owner = substr( $expected_value, 0, $colon );
			$group = substr( $expected_value, $colon+1 );
		}
		if ( $owner != '' )
		{
			$actual_owner = vmtFile::get_fileowner( $path );
			if ( $actual_owner != $owner )
				$m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_keyword, $owner, $actual_owner, '(owner:)' );
		}
		
		if ( $group != '' )
		{
			$actual_group = vmtFile::get_filegroup( $path );
			if ( $actual_group != $group )
				$m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_keyword, $group, $actual_group, '(group:)' );
		}
	}
	
	//-------------------------------------------
	//^ The get_fileowner function
	public static function get_fileowner( $path )
	{
		$command = "stat --format=%U '$path'";
		$value = shell_exec( $command );
		return trim( $value );
	}
	
	//-------------------------------------------
	//^ The get_filegroup function
	public static function get_filegroup( $path )
	{
		$command = "stat --format=%G '$path'";
		$value = shell_exec( $command );
		return trim( $value );
	}
		
	//-------------------------------------------
	// The permissions function verifies the ACL access rights of the file
	//< return void
	protected function permissions($path)
	{
		vmtFile::verify_permissions( $this, $path, 'permissions', $this->permissions );
	}

	//-------------------------------------------
	//^ The verify_permissions function verifies the ACL access rights of the file
	//> $m is a vmtModule object
	//> $path is the file to check
	//> $attribute_keyword is for error messages, usually 'permissions', but may be an something like 'document_permissions'
	//> $expected_value is the octal number, like 777
	// This is a static function that can be called from outside the class
	//< return void
	public static function verify_permissions( vmtModule $m, $path, $attribute_keyword, $expected_value )
	{
		if ($path == 'undeclared' || $expected_value == 'undeclared')
			return;
			
		$perms = fileperms( $path );
		if ( $perms === false )
			return $m->runtime_error( __METHOD__, $attribute_keyword );
		
		$octal = substr( sprintf('%o', $perms), -3 );
	
		if ( $octal != $expected_value )
			return $m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_keyword, $expected_value, $octal );
	}
	
	//-------------------------------------------
	// The selinux function verifies the Security Enhanced Linux context of the file
	//< return void
	protected function selinux($path)
	{
		vmtFile::verify_selinux( $this, $path,
					'selinux_user', $this->selinux_user,
					'selinux_role', $this->selinux_role,
					'selinux_type', $this->selinux_type,
					'selinux_level', $this->selinux_level );
	}
	
	//-------------------------------------------
	// The verify_selinux function verifies the Security Enhanced Linux context of the file
	//> $m is a vmtModule object
	//> $path is the file to check
	//> $attribute_user_keyword is for error messages, usually 'selinux_user', but may be an something like 'document_selinux_user'
	//> $expected_user is expected SELinux user or 'undeclared'
	//> $expected_role is expected SELinux role or 'undeclared'
	//> $expected_type is expected SELinux type or 'undeclared'
	//> $expected_level is expected SELinux level or 'undeclared'
	// This is a static function that can be called from outside the class
	//< return void
	public static function verify_selinux( vmtModule $m, $path,
		 $attribute_user_keyword, $expected_user,
		 $attribute_role_keyword, $expected_role,
		 $attribute_type_keyword, $expected_type,
		 $attribute_level_keyword, $expected_level )
	{
		if ($path == 'undeclared')
			return;
			
		if ($expected_user == 'undeclared' &&
			$expected_role == 'undeclared' &&
			$expected_type == 'undeclared' &&
			$expected_level == 'undeclared')
				return;
			
		// ls --scontext returns the security context and the filename
		// awk '{print $1}' prints only the first column (the security context
		// the final output will look like 'unconfined_u:object_r:admin_home_t:s0'
		$command = "ls --scontext '$path' | awk '{print $1}'";
		$se_context = shell_exec( $command );
		$se_context = trim($se_context);		// return value contains a trailing linefeed
		
		$parts = explode( ':', $se_context );
		if ( count($parts) > 0 && $expected_user != 'undeclared' && $parts[0] != $expected_user )
			return $m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_user_keyword, $expected_user, $parts[0] );
		if ( count($parts) > 1 && $expected_role != 'undeclared' && $parts[1] != $expected_role )
			return $m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_role_keyword, $expected_role, $parts[1] );
		if ( count($parts) > 2 && $expected_type != 'undeclared' && $parts[2] != $expected_type )
			return $m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_type_keyword, $expected_type, $parts[2] );
		if ( count($parts) > 3 && $expected_level != 'undeclared' && $parts[3] != $expected_level )
			return $m->validation_attribute_mismatch( __METHOD__ . "($path)", $attribute_level_keyword, $expected_level, $parts[3] );
	}

	//-------------------------------------------
	//^ The max_filesize function verifies that the file is smaller than the specified value
	//< return void
	protected function max_filesize($path)
	{
		if ($this->max_filesize == 'undeclared')
			return;
		
		// the filesize may be specified using multiplication like 1024*1024
		$rc = preg_match( '/[^0-9|\*]/', $this->max_filesize );	 // only digits and splat
		if ( $rc == 1 )
			return $this->specification_error( __METHOD__, 'max_filesize', "the value of 'max_filesize' should be the maximum size of the file, in bytes, but '{$this->max_filesize}' was specified" );
			
		eval( "\$val = $this->max_filesize;" );		// a number of bytes
		$actual = filesize( $path );
		if ( $actual > $val )
			return $this->validation_criteria_failed( __METHOD__, 'max_filesize', $val, "$path is $actual bytes" );
	}
	
}

?>
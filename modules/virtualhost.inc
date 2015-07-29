<?php
#==============================================================================
#
# File:         /var/vermont/modules/virtualhost.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking that an Apache virtual host's documents
#               and log files are properly configured
#
# Author:       Joe Honton © 2010
#
# Initial date: December 24, 2010
#
#
#     Keyword                       Value
# ---------------           -----------------------
# vmtGlob                   all vmtGlob keywords can be used and apply to the virtual host's document hierarchy, the documents served to the public
#
# log_path                  the fully qualified directory where access and error log files are created
# log_owner                 { owner | owner:group | owner: | :group }, like 'apache:apache'
# log_directory_permissions the octal notation for the directory's ACL, like 750
# log_file_permissions      the octal notation for the file's ACL, like 640
# log_selinux_user          the SELinux user, like 'unconfined_u'
# log_selinux_role          the SELinux role, like 'object_r'
# log_selinux_type          the SELinux type, like 'httpd_log_t'
# log_selinux_level         the SELinux sensitivity level, like 's0'
#
#==============================================================================


class vmtVirtualhost extends vmtGlob
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['log_path'] = 'optional';
		$this->vocabulary['log_owner'] = 'optional';
		$this->vocabulary['log_directory_permissions'] = 'optional';
		$this->vocabulary['log_file_permissions'] = 'optional';
		$this->vocabulary['log_selinux_user'] = 'optional';
		$this->vocabulary['log_selinux_role'] = 'optional';
		$this->vocabulary['log_selinux_type'] = 'optional';
		$this->vocabulary['log_selinux_level'] = 'optional';
		
		$this->log_path = property_exists($this->kv, 'log_path') ? $this->kv->log_path : 'undeclared';
		$this->log_owner = property_exists($this->kv, 'log_owner') ? $this->kv->log_owner : 'undeclared';
		$this->log_directory_permissions = property_exists($this->kv, 'log_directory_permissions') ? $this->kv->log_directory_permissions : 'undeclared';
		$this->log_file_permissions = property_exists($this->kv, 'log_file_permissions') ? $this->kv->log_file_permissions : 'undeclared';
		$this->log_selinux_user = property_exists($this->kv, 'log_selinux_user') ? $this->kv->log_selinux_user : 'undeclared';
		$this->log_selinux_role = property_exists($this->kv, 'log_selinux_role') ? $this->kv->log_selinux_role : 'undeclared';
		$this->log_selinux_type = property_exists($this->kv, 'log_selinux_type') ? $this->kv->log_selinux_type : 'undeclared';
		$this->log_selinux_level = property_exists($this->kv, 'log_selinux_level') ? $this->kv->log_selinux_level : 'undeclared';
	}
	
	//-------------------------------------------
	// The perform function checks that the document root directory and log file directory exist
	// and have the proper owner, permissions, security context.
	public function perform()
	{
		// process the concatentated list of document paths
		$parts = explode( '|', $this->path );
		foreach ($parts as $document_path)
		{
			$document_path = trim($document_path);
			if ($document_path != '')
				$this->verify_one_path( $document_path );
		}
		
		// process the concatentated list of log paths
		$parts = explode( '|', $this->log_path );
		foreach ($parts as $log_path)
		{
			$log_path = trim($log_path);
			if ($log_path != '')
				$this->verify_log_path( trim($log_path) );
		}
	}

	
	//-------------------------------------------
	// The verify_log_path function checks that the apache access log and error log
	// directory exists and has the proper owner, permissions, security context.
	private function verify_log_path( $log_path )
	{
		if ($log_path == 'undeclared')
			return;
		
		if (!file_exists($log_path))
			return $this->validation_criteria_failed( __METHOD__, 'log_path', $log_path, 'does not exist' );
		else
		{
			// verify the directory itself
			$this->verify_log_file( $log_path );
			
			// verify all log files within the directory
			$glob_of_files = glob( $log_path . '/*' );
			foreach ($glob_of_files as $log_file)
				$this->verify_log_file( $log_file );
		}
	}
	
	
	//-------------------------------------------
	// The verify_log_file function checks that the apache access log and error log
	// directory exists and has the proper owner, permissions, security context.
	private function verify_log_file( $log_file )
	{
		$is_directory = is_dir($log_file);
		if ( $is_directory == true && $this->directory_permissions != 'undeclared' )
			vmtFile::verify_permissions($this, $log_file, 'log_directory_permissions', $this->log_directory_permissions);
		else if ( $is_directory == false && $this->file_permissions != 'undeclared' )
			vmtFile::verify_permissions($this, $log_file, 'log_file_permissions', $this->log_file_permissions);
		
		vmtFile::verify_owner($this, $log_file, 'log_owner', $this->log_owner);
		vmtFile::verify_selinux($this, $log_file,
					'log_selinux_user', $this->log_selinux_user,
					'log_selinux_role', $this->log_selinux_role,
					'log_selinux_type', $this->log_selinux_type,
					'log_selinux_level', $this->log_selinux_level);
	}
}

?>
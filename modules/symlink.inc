<?php
#==============================================================================
#
# File:         /var/vermont/modules/symlink.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking the validity of symbolic links
#
# Author:       Joe Honton © 2010
#
# Initial date: December 23, 2010
#
#
#        Keyword                 Value
# ---------------------  -----------------------
# symlink_path           The fully qualified name of the symlink file.
#                        Although uncommon, this _may_ be a concatenated list of symlinks,
#                        in which case each one will be evaluated against the owner, selinux,
#                        and validate rules for the symlink itself (but the target rules
#                        probably won't be meaningful in this scenario.)
# symlink_owner          { owner | owner:group | owner: | :group }, like 'apache:apache'
# #symlink_permissions   THIS IS NOT A KEYWORD. There is no such concept as permissions on the symlink, only on the target.
# symlink_selinux_user   the SELinux user, like 'unconfined_u'
# symlink_selinux_role   the SELinux role, like 'object_r'
# symlink_selinux_type   the SELinux type, like 'httpd_sys_content_t'		(Use chcon -h to change the symlink rather than its target.)
# symlink_selinux_level  the SELinux sensitivity level, like 's0'
#
# validate               validate that the symbolic link points to a file that actually exists;
#                        true or false; if true perform the validity check, if false,
#                        do not perform the validity check; defaults to true.
#
# target_path            The local path or the fully qualified path that the symbolic link should point to,
#                        like /usr/bin/vermont-verify, optional, if specified, the specified
#                        target will be compared to the symlink's actual target.
# target_owner           { owner | owner:group | owner: | :group }, like 'apache:apache'
# target_permissions     the octal notation for the file's ACL, like 755
# target_selinux_user    the SELinux user, like 'unconfined_u'
# target_selinux_role    the SELinux role, like 'object_r'
# target_selinux_type    the SELinux type, like 'httpd_sys_content_t'
# target_selinux_level   the SELinux sensitivity level, like 's0'
#
#
#==============================================================================


class vmtSymlink extends vmtModule
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['symlink_path'] = 'required';
		$this->vocabulary['symlink_owner'] = 'optional';
		$this->vocabulary['symlink_selinux_user'] = 'optional';
		$this->vocabulary['symlink_selinux_role'] = 'optional';
		$this->vocabulary['symlink_selinux_type'] = 'optional';
		$this->vocabulary['symlink_selinux_level'] = 'optional';

		$this->vocabulary['validate'] = 'optional';

		$this->vocabulary['target_path'] = 'optional';
		$this->vocabulary['target_owner'] = 'optional';
		$this->vocabulary['target_permissions'] = 'optional';
		$this->vocabulary['target_selinux_user'] = 'optional';
		$this->vocabulary['target_selinux_role'] = 'optional';
		$this->vocabulary['target_selinux_type'] = 'optional';
		$this->vocabulary['target_selinux_level'] = 'optional';

		$this->symlink_path = property_exists($this->kv, 'symlink_path') ? $this->kv->symlink_path : 'undeclared';
		$this->symlink_owner = property_exists($this->kv, 'symlink_owner') ? $this->kv->symlink_owner : 'undeclared';
		$this->symlink_selinux_user = property_exists($this->kv, 'symlink_selinux_user') ? $this->kv->symlink_selinux_user : 'undeclared';
		$this->symlink_selinux_role = property_exists($this->kv, 'symlink_selinux_role') ? $this->kv->symlink_selinux_role : 'undeclared';
		$this->symlink_selinux_type = property_exists($this->kv, 'symlink_selinux_type') ? $this->kv->symlink_selinux_type : 'undeclared';
		$this->symlink_selinux_level = property_exists($this->kv, 'symlink_selinux_level') ? $this->kv->symlink_selinux_level : 'undeclared';
				
		$this->validate = property_exists($this->kv, 'validate') ? $this->kv->validate : 'true';
		
		$this->target_path = property_exists($this->kv, 'target_path') ? $this->kv->target_path : 'undeclared';
		$this->target_owner = property_exists($this->kv, 'target_owner') ? $this->kv->target_owner : 'undeclared';
		$this->target_permissions = property_exists($this->kv, 'target_permissions') ? $this->kv->target_permissions : 'undeclared';
		$this->target_selinux_user = property_exists($this->kv, 'target_selinux_user') ? $this->kv->target_selinux_user : 'undeclared';
		$this->target_selinux_role = property_exists($this->kv, 'target_selinux_role') ? $this->kv->target_selinux_role : 'undeclared';
		$this->target_selinux_type = property_exists($this->kv, 'target_selinux_type') ? $this->kv->target_selinux_type : 'undeclared';
		$this->target_selinux_level = property_exists($this->kv, 'target_selinux_level') ? $this->kv->target_selinux_level : 'undeclared';
	}

	//-------------------------------------------
	// The perform function is the main driver
	public function perform()
	{
		// process the concatentated list of symlinks
		$parts = explode( '|', $this->symlink_path );
		foreach ($parts as $one_symlink)
			$this->verify_one_symlink( trim($one_symlink) );
	}
	
	//-------------------------------------------
	public function verify_one_symlink( $one_symlink )
	{
		if ($one_symlink == 'undeclared')
			return;
		
		// first perform all file-related checks on the symlink itself
		if (!file_exists($one_symlink))
			return $this->validation_criteria_failed( __METHOD__, 'symlink', $one_symlink, 'does not exist' );
		else
		{
			vmtFile::verify_owner($this, $one_symlink, 'symlink_owner', $this->symlink_owner);
			vmtFile::verify_selinux($this, $one_symlink,
						'symlink_selinux_user', $this->symlink_selinux_user,
						'symlink_selinux_role', $this->symlink_selinux_role,
						'symlink_selinux_type', $this->symlink_selinux_type,
						'symlink_selinux_level', $this->symlink_selinux_level);
		}
		
		// now perform all target-specific checks
		if ( $this->is_symlink($one_symlink) )
		{
			$this->target($one_symlink);
			$this->validate($one_symlink);
			
			if ( $this->status() )
			{
				$target = $this->absolute_target($one_symlink);
				
				vmtFile::verify_owner($this, $target, 'target_owner', $this->target_owner);
				vmtFile::verify_permissions($this, $target, 'target_permissions', $this->target_permissions);
				vmtFile::verify_selinux($this, $target,
							'target_selinux_user', $this->target_selinux_user,
							'target_selinux_role', $this->target_selinux_role,
							'target_selinux_type', $this->target_selinux_type,
							'target_selinux_level', $this->target_selinux_level);
			}
		}
	}
	
	//-------------------------------------------
	//^ The is_symlink function certifies that the file really is a symbolic link
	public function is_symlink($one_symlink)
	{
		if ( !is_link($one_symlink) )
			return $this->validation_criteria_failed( __METHOD__, 'symlink_path', $one_symlink, 'is not a symbolic link' );
			
		return true;
	}
	
	//-------------------------------------------
	//^ The target function verifies that the target file points to the user-specified path
	//< return void
	public function target($one_symlink)
	{
		if ($this->target_path == 'undeclared')
			return;

		$target = readlink($one_symlink);
		if ( $target != $this->target_path )
			return $this->validation_attribute_mismatch( __METHOD__, 'target_path', $this->target_path, $target );
	}

	//-------------------------------------------
	//^ The validate function verifies that the symlink points to a file that exists
	//< return void
	public function validate($one_symlink)
	{
		if ($this->validate != 'true')
			return;

		$target = $this->absolute_target($one_symlink);

		if (!file_exists($target))
			return $this->validation_criteria_failed( __METHOD__, 'target_path', $this->target_path, 'does not exist' );
	}
	
	//-------------------------------------------
	//^ The absolute_target function converts a relative target to a fully qualified filename
	//> $one_symlink is the path to a symlink file
	//< returns the fully qualified filename
	public function absolute_target($one_symlink)
	{
		$target = readlink($one_symlink);
		
		// if this is a relative path, construct a fully qualified path, using the symlink as the base
		if ( $target{0} != '/' )
			$target = dirname($one_symlink) . '/' . $target;
			
		return $target;
	}
	
}

?>
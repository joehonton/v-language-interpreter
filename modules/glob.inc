<?php
#==============================================================================
#
# File:         /var/vermont/modules/glob.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking a collection of files in a directory
#               where all files should have the same owner, permissions, and
#               se_linux context
#
# Author:       Joe Honton © 2010
#
# Initial date: December 27, 2010
#
#
#     Keyword                  Value
# ---------------------   -----------------------
# directory_pattern       A text string, not a POSIX pattern.  If this string matches any part of the path, it will be included; Optional, concatenation allowed, defaults to 'undeclared' meaning, all directories match.
# file_pattern            A POSIX filename pattern using '*' and '?'. Multiple patterns can be specified with the concatenation operator. Defaults to '*' if nothing is specified.
# directory_exclude       A text string, not a POSIX pattern.  If this string matches any part of the path, it will be excluded; Optional, concatenation allowed, defaults to 'undeclared' meaning, all directories pass.
# file_exclude            A POSIX filename pattern using '*' and '?'. Files to exclude from the pattern; multiples allowed; no default
# match_directories       Should the verification rules apply to directories? true or false, optional, defaults to true.
# match_files             Should the verification rules apply to files? true or false, optional, defaults to true.
# recursive               Scan subdirectories too? true or false, optional, defaults to false.
# trace_filenames         List the names of all files that match the specified rules.  true or false, defaults to false.
# follow_symlinks         When recursing through directories, should symbolic links be followed?  true or false, defaults to false.
# directory_permissions   What permissions should all directories have? Optional. Defaults to the value of the keyword "permissions".
# file_permissions        What permissions should all files have? Optional. Defaults to the value of the keyword "permissions".
# hidden_directories      Should the verification rules apply to directories starting with a dot? true or false, optional, defaults to true.
# hidden_files            Should the verification rules apply to files starting with a dot? true or false, optional, defaults to true.
#
# (see also vmtFile for additional keywords)
#
# In order to recursively change all directories to 700, issue this command:
#  cd /path/to/directory
#  find . -type d -exec chmod 700 {} \;
#
# In order to recursively change all files to 600, issue this command:
#  cd /path/to/directory
#  find . -type f -exec chmod 600 {} \;
#
#==============================================================================


class vmtGlob extends vmtFile
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		
		$this->vocabulary['directory_pattern'] = 'optional';
		$this->vocabulary['file_pattern'] = 'optional';
		$this->vocabulary['directory_exclude'] = 'optional';
		$this->vocabulary['file_exclude'] = 'optional';
		$this->vocabulary['match_directories'] = 'optional';
		$this->vocabulary['match_files'] = 'optional';
		$this->vocabulary['recursive'] = 'optional';
		$this->vocabulary['trace_filenames'] = 'optional';
		$this->vocabulary['follow_symlinks'] = 'optional';
		$this->vocabulary['directory_permissions'] = 'optional';
		$this->vocabulary['file_permissions'] = 'optional';
		$this->vocabulary['hidden_directories'] = 'optional';
		$this->vocabulary['hidden_files'] = 'optional';
		
		$this->directory_pattern = property_exists($this->kv, 'directory_pattern') ? $this->kv->directory_pattern : 'undeclared';
		$this->file_pattern = property_exists($this->kv, 'file_pattern') ? $this->kv->file_pattern : '*';
		$this->directory_exclude = property_exists($this->kv, 'directory_exclude') ? $this->kv->directory_exclude : 'undeclared';
		$this->file_exclude = property_exists($this->kv, 'file_exclude') ? $this->kv->file_exclude : 'undeclared';
		$this->match_directories = property_exists($this->kv, 'match_directories') ? $this->kv->match_directories : 'true';
		$this->match_files = property_exists($this->kv, 'match_files') ? $this->kv->match_files : 'true';
		$this->recursive = property_exists($this->kv, 'recursive') ? $this->kv->recursive : 'false';
		$this->trace_filenames = property_exists($this->kv, 'trace_filenames') ? $this->kv->trace_filenames : 'false';
		$this->follow_symlinks = property_exists($this->kv, 'follow_symlinks') ? $this->kv->follow_symlinks : 'false';
		$this->directory_permissions = property_exists($this->kv, 'directory_permissions') ? $this->kv->directory_permissions : 'undeclared';
		$this->file_permissions = property_exists($this->kv, 'file_permissions') ? $this->kv->file_permissions : 'undeclared';
		$this->hidden_directories = property_exists($this->kv, 'hidden_directories') ? $this->kv->hidden_directories : 'true';
		$this->hidden_files = property_exists($this->kv, 'hidden_files') ? $this->kv->hidden_files : 'true';
	}

	//-------------------------------------------
	// The pre_perform function is called before starting
	// return true to continue, false to halt
	public function pre_perform()
	{
		return parent::pre_perform();
	}
	
	//-------------------------------------------
	// The perform function loops through the path, or paths, specified by the configuration
	public function perform()
	{
		// the path variable may specify more than one path
		$parts = explode( '|', $this->path);
		
		// process the concatentated list of document paths
		foreach ($parts as $one_path)
		{
			$one_path = trim($one_path);
			if ($one_path != '')
				$this->verify_one_dir_path( $one_path );
		}
	}

	//-------------------------------------------
	// The verify_one_dir_path function uses the specified pattern to check a single path
	// by iterating over the possible directory patterns.
	public function verify_one_dir_path( $one_path )
	{
		// the directory_pattern variable may specify more than one pattern
		$parts = explode( '|', $this->directory_pattern);
		foreach ($parts as $one_pattern)
		{
			$one_pattern = trim($one_pattern);
			if ($one_pattern != '')
				$this->verify_one_dir_pattern( $one_path, $one_pattern );
		}
	}
	
	
	//-------------------------------------------
	// The verify_one_dir_pattern function uses the specified pattern to check the
	// glob of dirs that match that pattern, minus all excluded patterns
	public function verify_one_dir_pattern( $one_path, $one_pattern )
	{
		// Start by discovering all dirs that should be excluded.
		// Always exclude the two special dot directories.
		$excluded_dirs = array( $one_path . '/.', $one_path . '/..' );
		if ( $this->directory_exclude != 'undeclared' )
		{
			$parts = explode( '|', $this->directory_exclude );
			foreach ($parts as $one_exclude)
			{
				$one_exclude = trim($one_exclude);
				if ($one_exclude != '')
				{
					$x = glob( $one_path . '/' . $one_exclude, GLOB_ONLYDIR );
					if ( $x !== false )
						$excluded_dirs = array_merge( $excluded_dirs, $x );
				}
			}
		}
				
		// if no directory pattern was specified, accept all directories
		if ( $one_pattern == 'undeclared' )
			$this->verify_one_file_path( $one_path );
		// otherwise process the files in this path only if the directory_pattern was matched
		else if ( strpos( $one_path, $one_pattern ) !== false )
			$this->verify_one_file_path( $one_path );
				
				
		// now get all subdirs
		$subdirs = glob( $one_path . '/*' , GLOB_ONLYDIR );
		foreach ($subdirs as $one_subdir)
		{
			if ( !in_array($one_subdir,$excluded_dirs) )
			{
				// recurse all directories in this path
				if ( $this->recursive == 'true' )
					if ( $this->follow_symlinks == 'true' || !is_link($one_subdir) )
						$this->verify_one_dir_pattern( $one_subdir, $one_pattern );
			}
		}
		
		// and repeat for the hidden directories if asked
		if ( $this->hidden_directories == 'true' )
		{
			$subdirs = glob( $one_path . '/.*' , GLOB_ONLYDIR );
			foreach ($subdirs as $one_subdir)
			{
				if ( !in_array($one_subdir,$excluded_dirs) )
				{
					// recurse all directories in this path
					if ( $this->recursive == 'true' )
						if ( $this->follow_symlinks == 'true' || !is_link($one_subdir) )
							$this->verify_one_dir_pattern( $one_subdir, $one_pattern );
				}
			}
		}
				
	}


	//-------------------------------------------
	// The verify_one_file_path function uses the specified pattern to check a single path
	// by iterating over the possible file patterns.
	public function verify_one_file_path( $one_path )
	{
		// the file_pattern variable may specify more than one pattern
		$parts = explode( '|', $this->file_pattern);
		foreach ($parts as $one_pattern)
		{
			$one_pattern = trim($one_pattern);
			if ($one_pattern != '')
				$this->verify_one_file_pattern( $one_path, $one_pattern );
		}
	}
	

	//-------------------------------------------
	// The verify_one_dir_pattern function uses the specified pattern to check the
	// glob of files that match that pattern, minus all excluded patterns
	public function verify_one_file_pattern( $one_path, $one_pattern )
	{
		// Start by discovering all files that should be excluded.
		// Always exclude the two special dot files.
		$excluded_files = array( $one_path . '/.', $one_path . '/..' );
		if ( $this->file_exclude != 'undeclared' )
		{
			$parts = explode( '|', $this->file_exclude );
			foreach ($parts as $one_exclude)
			{
				$one_exclude = trim($one_exclude);
				if ($one_exclude != '')
				{
					$x = glob( $one_path . '/' . $one_exclude );
					if ( $x !== false )
						$excluded_files = array_merge( $excluded_files, $x );
				}
			}
		}

		// now get all files matching the default '*' pattern or the user-defined pattern
		$files = glob( $one_path . '/' . $one_pattern );
		foreach ($files as $one_file)
		{
			// but only process this file if it isn't excluded
			if ( !in_array($one_file,$excluded_files) )
				$this->verify_one_file( $one_file );
		}
	
		// now get all hidden files and dirs (if we are working on the default '*' pattern)
		if ( $one_pattern == '*' )
		{
			$files = glob( $one_path . '/.*' );
			foreach ($files as $one_file)
			{
				if ( (!is_dir($one_file) && $this->hidden_files == 'true') || (is_dir($one_file) && $this->hidden_directories == 'true') )
				{
					// but only process this file/dir if it isn't excluded
					if ( !in_array($one_file,$excluded_files) )
						$this->verify_one_file( $one_file );
				}
			}
		}
	}
	
	
	//-------------------------------------------
	// The verify_one_file function verifies a single file for ownership,
	// permissions, and SELinux context.
	public function verify_one_file( $one_file )
	{
		// last check to make sure any directories that are to be excluded, really are
		// (This may occur for the first level directories)
		if ( $this->directory_exclude != 'undeclared' )
		{
			$parts = explode( '|', $this->directory_exclude );
			foreach ($parts as $one_exclude)
			{
				if ( basename($one_file) == trim($one_exclude) )
					return;
			}
		}
		
		
		// if requested, send the filename to the console
		if ( $this->trace_filenames == 'true' )
			$this->trace( __METHOD__ . "()", 'trace_filenames', $one_file );
			
		if ($this->exists($one_file))
		{
			$is_directory = is_dir($one_file);

			if ( ( $is_directory == true && $this->match_directories == 'true' ) ||
				 ( $is_directory == false && $this->match_files == 'true' ) )
			{
				$this->owner($one_file);
				$this->selinux($one_file);
				
				if ( $is_directory == true && $this->directory_permissions != 'undeclared' )
					vmtFile::verify_permissions( $this, $one_file, 'directory_permissions', $this->directory_permissions );
				else if ( $is_directory == false && $this->file_permissions != 'undeclared' )
					vmtFile::verify_permissions( $this, $one_file, 'file_permissions', $this->file_permissions );
				else
					vmtFile::verify_permissions( $this, $one_file, 'permissions', $this->permissions );
					
				$this->max_filesize($one_file);
			}
		}
	}

	//-------------------------------------------
	// The trace function
	//> $method is the name of the vmtModule-derived object that was being executed when the criteria failed
	//> $attribute_keyword is an attribute associated with this error
	//> $attribute_value is an attribute associated with this error
	//
	public function trace( $method, $attribute_keyword, $attribute_value )
	{
		$line_number = array_key_exists($attribute_keyword, $this->obj->tracking) ? $this->obj->tracking[$attribute_keyword] : '';
		
		// exact line number not possible, just use the line number of the first attribute
		if ( $line_number == '' )
			$line_number = ( count($this->obj->tracking) > 0 ) ? reset($this->obj->tracking) : '';
		
		$comment = ( $this->comment == 'undeclared' ) ? '' : $this->comment;
		$namespace_object = sprintf( "[%s:%s]", $this->obj->namespace_name, $this->obj->object_name );
		$method_string = sprintf( "%s", $method );
		$message = sprintf( "%s '%s'", $attribute_keyword, $attribute_value );
		$this->talk( $this->trip_point( $line_number, $comment, $namespace_object, $method_string, $message ) );
		
		$this->validation_status = false;
		return false;
	}
	
}

?>
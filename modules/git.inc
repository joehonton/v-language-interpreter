<?php
#==============================================================================
#
# File:         /var/vermont/modules/git.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking the status of a git repository: whether it
#               has any outstanding commits waiting to be pushed to its origin,
#               and whether the working tree has any files not synchronized with
#               the local index. This module is not designed for use with 'bare'
#               repositories because they do not have a working tree.
#
# Author:       Joe Honton © 2011
#
# Initial date: January 14, 2011
#
#
#     Keyword               Value
# ---------------	-----------------------
# path              the fully qualified path of the directory that contains the repository.
# repo              the name of the repository directory; optional, defaults to '.git'
# modified_files    Perform a 'git status' to see if the working tree has any modified
#                   files that are not in synch with the index.  Optional.
#                   Specify either true or false: true means perform the check and trigger
#                   a log message if the status is not clean. False means do not perform the check.
# untracked_files   Determine if there are any untracked files that are in the working tree, but not
#                   in the index. Optional, true or false; defaults to true which means perform the
#                   check and trigger a log message if the git-status lists any untracked files.
# unstaged_changes  Determine if there are files that have been added to or removed from
#                   the staging area, but have not yet been committed to the local index.
# pending_commits   Perform a 'git status' to see of the local index contains any committed
#                   objects that are awaiting a push to the original repository.
#
# see also vmtModule for more options
#
#==============================================================================


class vmtGit extends vmtModule
{
	//-------------------------------------------
	public function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['path'] = 'required';
		$this->vocabulary['repo'] = 'optional';
		$this->vocabulary['modified_files'] = 'optional';
		$this->vocabulary['untracked_files'] = 'optional';
		$this->vocabulary['unstaged_changes'] = 'optional';
		$this->vocabulary['pending_commits'] = 'optional';
		
		$this->path  = property_exists($this->kv, 'path') ? $this->kv->path : 'undeclared';
		$this->repo  = property_exists($this->kv, 'repo') ? $this->kv->repo : '.git';
		$this->modified_files  = property_exists($this->kv, 'modified_files') ? $this->kv->modified_files : 'true';
		$this->untracked_files  = property_exists($this->kv, 'untracked_files') ? $this->kv->untracked_files : 'true';
		$this->unstaged_changes  = property_exists($this->kv, 'unstaged_changes') ? $this->kv->unstaged_changes : 'true';
		$this->pending_commits = property_exists($this->kv, 'pending_commits') ? $this->kv->pending_commits : 'true';
	}

/*	//-------------------------------------------
	// The pre_perform function is called before starting
	// return true to continue, false to halt
	public function pre_perform()
	{
		return parent::pre_perform();
	}
*/
	//-------------------------------------------
	// The perform function must be defined by every module
	// returns void.  To indicate failure use one of the four status properties.
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
	// The verify_one_path function uses the specified pattern to check all files
	// in the given path that are not excluded by any of the exclude patterns.
	// This is called by vmtGlob::perform();
	public function verify_one_path( $one_path )
	{
		$status_string = $this->git_status( $one_path );

		// check the string for modified files
		$pos1 = strpos( $status_string, 'Changed but not updated:' );			// git version 1.7.3.4
		$pos2 = strpos( $status_string, 'Changes not staged for commit:' );		// git version 1.7.4
		if ( $pos1 !== false || $pos2 !== false )
			$this->validation_criteria_failed( __METHOD__ . "($one_path)", '', '', "there are modified files in the working directory" );
		
		// check the string for untracked files
		$pos = strpos( $status_string, 'Untracked files:' );
		if ( $pos !== false )
			$this->validation_criteria_failed( __METHOD__ . "($one_path)", '', '', "there are untracked files in the working directory" );
		
		// check the string for uncommited files
		$pos = strpos( $status_string, 'Changes to be committed:' );
		if ( $pos !== false )
			$this->validation_criteria_failed( __METHOD__ . "($one_path)", '', '', "there are uncommitted changes in the staging area" );
			
		// check the string for commits that have not been pushed to origin
		$pos = strpos( $status_string, 'Your branch is ahead of' );
		if ( $pos !== false )
			$this->validation_criteria_failed( __METHOD__ . "($one_path)", '', '', "there are commits waiting to be pushed to the remote origin" );
	}
	
	
	//-------------------------------------------
	//^ The git_status function issues the call to 'git status'
	//  and returns a long string containing the output
	//
	// As an alternative to using 'git status' we could use,
	//  'git ls-files -c' for cached files
	//  'git ls-files -m' for modified files
	//  'git ls-files -d' for deleted files
	//  'git ls-files -o' for other files
	// but these don't map well to the keywords, 'modified_files', 'untracked_files', 'unstaged_changes', 'pending_commits'
	//
	public function git_status( $one_path )
	{
		if ( !file_exists($one_path) )
			return '';
			
		$command = sprintf( "cd %s; git status;", $one_path );

		// 2>&1 redirects stderr to stdout
		$status_string = shell_exec( "$command 2>&1" );
		$status_string = str_replace( "\n", '', $status_string );	// concatenate multi-line response into a single-line string
		$status_string = trim( $status_string );
		
		return $status_string;
	}
	
}
?>
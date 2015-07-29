<?php
#==============================================================================
#
# File:         /var/vermont/modules/mysql.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking database tables in a MySQL installation
#
# Author:       Joe Honton © 2010
#
# Initial date: December 29, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# user              A MySQL user account that has SELECT privileges for the database
# password          The password for that account
# database          The name of the database to inspect
# table             The concatenated list of table names to verify
#
#==============================================================================

/*
 The database user for this query can have "SELECT" privilege only, like this:
   CREATE USER vermont@'%' IDENTIFIED BY 'secret';
   GRANT SELECT ON *.* TO vermont@'%' IDENTIFIED BY 'secret';
 */

class vmtMysql extends vmtModule
{
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['user'] = 'required';
		$this->vocabulary['password'] = 'required';
		$this->vocabulary['database'] = 'required';
		$this->vocabulary['table'] = 'optional';
		
		$this->user = property_exists($this->kv, 'user') ? $this->kv->user : 'undeclared';
		$this->password = property_exists($this->kv, 'password') ? $this->kv->password : 'undeclared';
		$this->database = property_exists($this->kv, 'database') ? $this->kv->database : 'undeclared';
		$this->table = property_exists($this->kv, 'table') ? $this->kv->table : 'undeclared';
	}

	//-------------------------------------------
	// The perform function loops through all queries that have been concatenated together
	public function perform()
	{
		// process the concatentated list of table names
		$parts = explode( '|', $this->table);
		foreach ($parts as $one_table)
		{
			$one_table = trim($one_table);
			$this->verify_table( $this->user, $this->password, $this->database, $one_table );
		}
	}
	
	//-------------------------------------------
	// The verify_table function uses the MySQL command line interface to verify that the
	// tables are actually defined.
	public function verify_table( $user, $password, $database, $table )
	{
		$sql = sprintf( "select count(*) from information_schema.tables where table_schema='%s' and table_name='%s'", $database, $table );
		$command = sprintf( 'mysql --user=%s --password=%s --silent --execute="%s"', $user, $password, $sql );
		
		// 2>&1 redirects stderr to stdout
		$command = "$command 2>&1";
		$response = shell_exec( $command );
		
		// --silent turns off the box lines in the response,
		// when called from the command line the response returns two lines like:
		//   count(*)
		//   1
		// --silent turns off the first line when called via shell_exec, it returns one line like:
		//   1

		$response = trim( $response );
		
		if ( $response == '0' )
			$this->validation_criteria_failed( __METHOD__ . "($database.$table)", 'table', $table, 'table not found' );
		else if ( $response != '1' )
		{
			$response = str_replace( "\n", '', $response );
			$this->runtime_error( __METHOD__ . "($database.$table)", '', "MySQL says $response" );
		}
	}

}

?>
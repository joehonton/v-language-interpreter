<?php
#==============================================================================
#
# File:         /var/vermont/modules/software.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking that a software package has been installed
#               by yum, and for verifying that it is a particular version,
#               and that services are congifured for boot-time start and are running.
#               This is a replacement for 'package.inc' which worked on Fedora 14
#               using 'sysV'.  This module works on Fedora 15 which uses 'systemd'.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 24, 2010
#
#
#     Keyword               Value
# ---------------	-----------------------
# name              a yum package name, like 'mysql', 'php'
# version           a string value returned by the package's version command
# unit_file_state   'systemctl show' UnitFileState = static | enabled | disabled
# load_state        'systemctl show' LoadState = loaded | error 
# active_state      'systemctl show' ActiveState = active | reloading | inactive | error | activating | deactivating
# sub_state         'systemctl show' SubState = dead | start-pre | start | start-post | running | exited | reload | stop | stop-sigterm | stop-sigkill | stop-post | final-sigterm | final-sigkill | maintenance | auto-restart
#
#==============================================================================


class vmtSoftware extends vmtModule
{
	// the $packages associative array is keyed on a 'yum' name and points to
	// an array consisting of:
	//   [0] is a service name, or false if the package does not have a service
	//   [1] the command to show the package version
	public $packages = array(
		'apache'		=> array( 'httpd.service',				'rpm -qa | grep httpd'),			// 'apachectl -v' generates AVC denials in /var/log/audit/audit.log
		'audit'			=> array( 'auditd.service',				'rpm -qa | grep audit' ),			// 'auditctl -v' generates AVC denials in /var/log/audit/audit.log
		'cron'			=> array( 'crond.service',				'rpm -qa | grep cronie' ),
		'dkim-milter'	=> array( 'dkim-milter.service',		'rpm -qa | grep dkim-milter' ),
		'dovecot'		=> array( 'dovecot.service',			'dovecot --version' ),
		'fedora'		=> array( false,						'cat /etc/fedora-release' ),
		'git'			=> array( false,						'git version' ),
		'imagick'		=> array( false,						'convert -version' ),
		'iptables'		=> array( 'iptables.service',			'iptables --version' ),
		'kernel'		=> array( false,						'uname -a' ),
		'mysql'			=> array( 'mysqld.service',				'mysql -V' ),
		'named'			=> array( 'named.service',				'named -v' ),
		'ntp'			=> array( 'ntpd.service',				'ntpd --version' ),
		'NetworkManager'=> array( false,						'rpm -qa | grep NetworkManager' ),
		'network'		=> array( 'network.service',			'' ),
		'nmap'			=> array( false,						'nmap -V' ),
		'perl'			=> array( false,						'perl -v' ),
		'php'			=> array( false,						'php -v' ),
		'procmail'		=> array( false,						'procmail -v' ),
		'rpcbind'		=> array( 'rpcbind.service',			'rpm -qa | grep rpcbind' ),
		'rsyslog'		=> array( 'rsyslog.service',			'rsyslogd -v' ),
		'sendmail'		=> array( 'sendmail.service',			'sendmail -d0.1 < /dev/null' ),
		'sm-client'		=> array( 'sm-client.service',			'sendmail -d0.1 < /dev/null' ),
		'setroubleshoot'=> array( 'setroubleshoot.service',		'rpm -qa | grep setroubleshoot-server' ),
		'spam-assassin'	=> array( 'spamassassin.service',		'rpm -qa | grep spamassassin' ),	// 'spamd --version' generates AVC denials in /var/log/audit/audit.log
		'ssh'			=> array( 'sshd.service',				'ssh -V' ),
		'vermont'		=> array( false,						'vv --version' ),
		'vnstat'		=> array( 'vnstat.service',				'vnstat -v' ),
	);

	
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['name'] = 'required';
		$this->vocabulary['version'] = 'optional';
		$this->vocabulary['unit_file_state'] = 'optional';
		$this->vocabulary['load_state'] = 'optional';
		$this->vocabulary['active_state'] = 'optional';
		$this->vocabulary['sub_state'] = 'optional';
		
		$this->name = property_exists($this->kv, 'name') ? $this->kv->name : 'undeclared';
		$this->version = property_exists($this->kv, 'version') ? $this->kv->version : 'undeclared';
		$this->unit_file_state = property_exists($this->kv, 'unit_file_state') ? $this->kv->unit_file_state : 'undeclared';
		$this->load_state = property_exists($this->kv, 'load_state') ? $this->kv->load_state : 'undeclared';
		$this->active_state = property_exists($this->kv, 'active_state') ? $this->kv->active_state : 'undeclared';
		$this->sub_state = property_exists($this->kv, 'sub_state') ? $this->kv->sub_state : 'undeclared';
	}
	
	//-------------------------------------------
	// The perform function checks a package's version,
	// and if it has a service also checks for the proper states
	public function perform()
	{
		if ( !array_key_exists( $this->name, $this->packages ) )
			return $this->specification_error( __METHOD__, 'name', "vmtSoftware is not familiar with {$this->name}" );
		
		$this->verify_version();
			
		// if this package is a service
		$bIsService = $this->packages[$this->name][0];
		if ( $bIsService !== false )
		{
			if ( $this->status() == true )
				$this->verify_state( 'unit_file_state', 'UnitFileState' );
			
			if ( $this->status() == true )
				$this->verify_state( 'load_state', 'LoadState' );
			
			if ( $this->status() == true )
				$this->verify_state( 'active_state', 'ActiveState' );
			
			if ( $this->status() == true )
				$this->verify_state( 'sub_state', 'SubState' );
		}
	}
	
	//-------------------------------------------
	// The verify_version function
	public function verify_version()
	{
		if ($this->version == 'undeclared')
			return;

		$version_command = $this->packages[$this->name][1];
		
		// if no command is specified in the table, report as error
		if ( $version_command == '' )
			return $this->specification_error( __METHOD__, 'name', "vmtSoftware does not know how to get the version of {$this->name}" );
		
		// 2>&1 redirects stderr to stdout
		$version_reponse_string = shell_exec( "$version_command 2>&1" );
		$version_reponse_string = str_replace( "\n", '', $version_reponse_string );	// concatenate multi-line response into a single-line string
		$version_reponse_string = trim( $version_reponse_string );
		
		$pos = strpos( $version_reponse_string, $this->version );
		if ( $pos === false )
		{
			if ( strlen($version_reponse_string) > 50 )
				$version_reponse_string = substr( $version_reponse_string, 0, 50) . '...';
			$this->validation_attribute_mismatch( __METHOD__ . "({$this->name})", 'version', $this->version, $version_reponse_string );
		}
	}

	//-------------------------------------------
	//^ The verify_state function
	//> $vermont_keyword is 'unit_file_state', 'load_state', 'active_state', or 'sub_state'
	//> $systemctl_keyword is 'UnitFileState', 'LoadState', 'ActiveState', or 'SubState'
	public function verify_state( $vermont_keyword, $systemctl_keyword )
	{
		$expected_value = $this->$vermont_keyword;		// notes: this is a variable variable
		if ( $expected_value == 'undeclared' )
			return;
			
		// 2>&1 redirects stderr to stdout
		$service_name = $this->packages[$this->name][0];
		$command = "systemctl show $service_name 2>&1 | grep '$systemctl_keyword'";
		$response = shell_exec( $command );

		// $reponse looks like:
		// LoadState=loaded
		$response = trim($response);
		$pos = strpos( $response, $expected_value );
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__ . "({$this->name})", $vermont_keyword, $expected_value, $response );
	}

}

?>
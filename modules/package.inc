<?php
#==============================================================================
#
# File:         /var/vermont/modules/package.inc
#
# Language:     Vermont plugin module class
#
# Contents:     A module for checking that a software package has been installed
#               by yum, and for verifying that it is a particular version.
#               and that services are congifured for boot-time start and are running.
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
# chkconfig         the setting for the chkconfig run-levels (3,4,5), 'on' or 'off'
# service           should the service be 'running' or stopped'
#
#==============================================================================


class vmtPackage extends vmtModule
{
	// the $packages associative array is keyed on a 'yum' or 'service' name and points to
	// an array consisting of:
	//   [0] is a service name, or false if the package does not have a service
	//   [1] the command to show the package version
	public $packages = array(
		'apache'		=> array( 'httpd',				'rpm -qa | grep httpd'),			// 'apachectl -v' generates AVC denials in /var/log/audit/audit.log
		'audit'			=> array( 'auditd',				'auditctl -v' ),
		'cron'			=> array( 'crond',				'rpm -qa | grep cronie' ),
		'dovecot'		=> array( 'dovecot',			'dovecot --version' ),
		'fedora'		=> array( false,				'cat /etc/fedora-release' ),
		'git'			=> array( false,				'git version' ),
		'imagick'		=> array( false,				'convert -version' ),
		'iptables'		=> array( 'iptables',			'iptables --version' ),
		'kernel'		=> array( false,				'uname -a' ),
		'mysql'			=> array( 'mysqld',				'mysql -V' ),
		'named'			=> array( 'named',				'named -v' ),
		'ntp'			=> array( 'ntpd',				'ntpd --version' ),
		'NetworkManager'=> array( false,				'rpm -qa | grep NetworkManager' ),
		'network'		=> array( false,				'' ),
		'nmap'			=> array( false,				'nmap -V' ),
		'perl'			=> array( false,				'perl -v' ),
		'php'			=> array( false,				'php -v' ),
		'procmail'		=> array( false,				'procmail -v' ),
		'rpcbind'		=> array( 'rpcbind',			'rpm -qa | grep rpcbind' ),
		'rsyslog'		=> array( 'rsyslog',			'rsyslogd -v' ),
		'sendmail'		=> array( 'sendmail',			'sendmail -d0.1 < /dev/null' ),
		'setroubleshoot'=> array( 'setroubleshoot',		'rpm -qa | grep setroubleshoot-server' ),
		'spam-assassin'	=> array( 'spamassassin',		'rpm -qa | grep spamassassin' ),	// 'spamd --version' generates AVC denials in /var/log/audit/audit.log
		'ssh'			=> array( 'sshd',				'ssh -V' ),
		'vermont'		=> array( false,				'vv --version' ),
		'vnstat'		=> array( 'vnstat',				'vnstat -v' ),
	);

	
	//-------------------------------------------
	function __construct( vmtManagedObject $o )
	{
		parent::__construct( $o );
		$this->vocabulary['name'] = 'required';
		$this->vocabulary['version'] = 'optional';
		$this->vocabulary['chkconfig'] = 'optional';
		$this->vocabulary['service'] = 'optional';
		
		$this->name = property_exists($this->kv, 'name') ? $this->kv->name : 'undeclared';
		$this->version = property_exists($this->kv, 'version') ? $this->kv->version : 'undeclared';
		$this->chkconfig = property_exists($this->kv, 'chkconfig') ? $this->kv->chkconfig : 'undeclared';
		$this->service = property_exists($this->kv, 'service') ? $this->kv->service : 'undeclared';
	}
	
	//-------------------------------------------
	// The perform function checks a package's version,
	// and if it is a service also checks for the proper 'chkconfig' run-levels
	// and whether or not it is currenly running.
	public function perform()
	{
		if ( !array_key_exists( $this->name, $this->packages ) )
			return $this->specification_error( __METHOD__, 'name', "vmtPackage is not familiar with {$this->name}" );
		
		$this->verify_version();
			
		// if this package is a service
		$bIsService = $this->packages[$this->name][0];
		if ( $bIsService !== false )
		{
			$this->verify_chkconfig();
			$this->verify_service();
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
			return $this->specification_error( __METHOD__, 'name', "vmtPackage does not know how to get the version of {$this->name}" );
		
		// 2>&1 redirects stderr to stdout
		$version_reponse_string = shell_exec( "$version_command 2>&1" );
		$version_reponse_string = str_replace( "\n", '', $version_reponse_string );	// concatenate multi-line response into a single-line string
		$version_reponse_string = trim( $version_reponse_string );
		
		$pos = strpos( $version_reponse_string, $this->version );
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__ . "({$this->name})", 'version', $this->version, $version_reponse_string );
	}

	//-------------------------------------------
	// The verify_chkconfig function
	public function verify_chkconfig()
	{
		$expected_chkconfig = $this->chkconfig;		// 'on' or 'off'
		if ( $expected_chkconfig == 'undeclared' )
			return;
		
		// 2>&1 redirects stderr to stdout
		$service_name = $this->packages[$this->name][0];
		$command = "chkconfig --list $service_name 2>&1";
		$response = shell_exec( $command );
		
		// $reponse looks like:
		// iptables        0:off   1:off   2:on    3:on    4:on    5:on    6:off
		$response = trim($response);
		$response_no_ws = str_replace( array(' ',"\t"), '', $response );
		$pos = false;
		if ( $expected_chkconfig == 'on' )
			$pos = strpos( $response_no_ws, '3:on4:on5:on' );
		else if ( $expected_chkconfig == 'off' )
			$pos = strpos( $response_no_ws, '3:off4:off5:off' );
		
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__ . "({$this->name})", 'chkconfig', $expected_chkconfig, $response );
	}

	//-------------------------------------------
	// The verify_service function
	public function verify_service()
	{
		$expected_service = $this->service;			// 'running' or 'stopped'
		if ( $expected_service == 'undeclared' )
			return;
			
		$service_name = $this->packages[$this->name][0];
		if ( $service_name == 'iptables' )
			return $this->verify_iptables_service();		// this guy doesn't follow the standard for service status
		else if ( $service_name == 'sendmail' )
			return $this->verify_sendmail_service();		// sendmail also include sm-client
			
		// 2>&1 redirects stderr to stdout
		$command = "service $service_name status 2>&1";
		$response = shell_exec( $command );
		
		// $response looks like:
		// httpd (pid  32012) is running...
		$response = trim($response);
		$pos = strpos( $response, $expected_service );
		
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__ . "({$this->name})", 'service', $expected_service, $response );
	}

	//-------------------------------------------
	// The verify_iptables_service function is necessary because iptables is always
	// running on Fedora, so report 'stopped' only when no filtering is occuring.
	// This can be done by scanning for one or more IP addresses like 0.0.0.0
	// which will almost alway be present when running, but never be present when stopped.
	private function verify_iptables_service()
	{
		$expected_service = $this->service;			// 'running' or 'stopped'
		
		// 2>&1 redirects stderr to stdout
		$command = "service iptables status 2>&1";
		$response = shell_exec( $command );
		
		// When running, the $response will contain something like:
		//	Table: filter
		//	Chain INPUT (policy ACCEPT)
		//	num  target     prot opt source               destination
		//	1    ACCEPT     all  --  0.0.0.0/0            0.0.0.0/0
		//  ...
		$pos = strpos( $response, '0.0.0.0' );
		
		if ( ($expected_service == 'running' && $pos === false) ||
			 ($expected_service == 'stopped' && $pos !== false) )
				$this->validation_attribute_mismatch( __METHOD__, 'service', $expected_service, "not $expected_service" );
	}
	
	//-------------------------------------------
	// The verify_sendmail_service function checks that both the 'sendmail'
	// and the 'sm-client' daemons are running.
	private function verify_sendmail_service()
	{
		$expected_service = $this->service;			// 'running' or 'stopped'
		
		// 2>&1 redirects stderr to stdout
		$command = "service sendmail status 2>&1";
		$response = shell_exec( $command );
		
		// When running, the $response will contain something like:
		//  sendmail (pid  9359) is running...
		//  sm-client (pid  9368) is running...
		$dual = explode( "\n", $response );
		if ( count($dual) != 3 )
			return $this->runtime_error( __METHOD__, '', "expected two status lines, but instead got $response" );	// two lines plus the final "\n"
		
		$response = trim($dual[0]);
		$pos = strpos( $response, $expected_service );
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__, 'service', $expected_service, $response, "(sendmail)" );

		$response = trim($dual[1]);
		$pos = strpos( $response, $expected_service );
		if ( $pos === false )
			$this->validation_attribute_mismatch( __METHOD__, 'service', $expected_service, $response, "(sm-client)" );
			
	}

}

?>
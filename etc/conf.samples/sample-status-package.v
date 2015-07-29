#==============================================================================
#
# File:         /var/vermont/conf.d/sample-status-package
#
# Language:     Sample of vermont configuration file showing usage of the
#               package module
#
# Author:       Joe Honton © 2010
#
# Initial date: December 27, 2010
#
#==============================================================================

[package:package]
type = package
chkconfig = on
service = running

[package:audit]
name = audit
version = 1.7.8

[package:apache]
name = apache
version = 2.2.14

[package:cron]
name = cron
version = 1.2-4

[package:dovecot]
name = dovecot
version = 1.1.10

[package:fedora]
name = fedora
version = Cambridge

[package:iptables]
name = iptables
version = 1.4.3.2
service = running

[package:kernel]
name = kernel
version = 2.6.27.5-117

[package:mysql]
name = mysql
version = 5.0.88

[package:named]
name = named
version = 9.5.1-P2

[package:ntp]
name = ntp
version = 4.2.4p7

[package:NetworkManager]
name = NetworkManager
version = 0.7.0
service = stopped

[package:network]
name = network

[package:nmap]
name = nmap
version = 5.00

[package:php]
name = php
version = 5.2.9

[package:perl]
name = perl
version = 5.10.0

[package:rpcbind]
name = rpcbind
version = 0.1.6-2

[package:rsyslog]
name = rsyslog
version = 3.21.3

[package:sendmail]
name = sendmail
version = 8.14.3

[package:setroubleshoot]
name = setroubleshoot
version = 2.0.12-1

[package:spam-assassin]
name = spam-assassin
version = 3.2.5
chkconfig = off
service = stopped

[package:ssh]
name = ssh
version = 5.1p1

[package:vermont]
name = vermont
version = 1.0
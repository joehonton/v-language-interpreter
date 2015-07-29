#==============================================================================
#
# File:         /var/vermont/conf.d/sample-selinux-boolean
#
# Language:     Sample file show how to verify system-wide SELinux boolean values
#
# Author:       Joe Honton © 2010
#
# Initial date: December 27, 2010
#
#==============================================================================

[sample7]
type = selinux

on =  httpd_can_network_connect
on += httpd_can_sendmail

#off = httpd_can_network_connect

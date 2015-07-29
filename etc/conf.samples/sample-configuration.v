#==============================================================================
#
# File:         /var/vermont/conf.d/sample-configuration
#
# Language:     Sample of vermont configuration file showing how to verify 
#               the value of a configuration option in a generic config file.
#               Specify the expected keyword and its expected value using a
#               '=>' separator.  Specify multiple keyword-value pairs
#               using the += concatenation operator.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 25, 2010
#
#==============================================================================

[sample4:A]
type = configuration
path = /etc/httpd/conf/httpd.conf
keyword_value += User => apache
keyword_value += Group => apache

#==============================================================================
#
# File:         /var/vermont/conf.d/sample-logfile-apache-errors
#
# Language:     Sample usage of the 'logfile' module with logformat = apache-error-log
#
# Author:       Joe Honton © 2011
#
# Initial date: January 8, 2011
#
#==============================================================================

#------------------------------------------------
[sample23:sample23]
type = logfile
output_limit = false
path =  /var/log/httpd
pattern = error_log*
recursive = true
logformat = apache-error-log
#match  = timestamp GT 21*24*60*60		# last 21 days
#match += timestamp LT 0*24*60*60
message1  = $timestamp $loglevel $client $message
#outputlevel = verbose
outputlevel = terse

[sample23:errors]															# this is the master list of identified errors
match1 = loglevel EQ error 
ignore1  = message LIKE robots.txt
ignore1 += message LIKE File does not exist
ignore1 += message LIKE client denied by server configuration
ignore1 += message LIKE Directory index forbidden by Options directive
ignore1 += message LIKE not found or unable to stat
ignore1 += message LIKE Invalid URI in request								# these are infrequent, but recent
ignore1 += message LIKE request without hostname							# classic w00t
ignore1 += message LIKE SUSPICIOUS											# My trap
ignore1 += message LIKE PHP													# php errors
ignore1 += message LIKE Zend Debugger
ignore1 += message LIKE Bad file descriptor: apr_socket_accept				# Apache race condition on graceful restart


[sample23:invalid_uri]
type = null
match1 = loglevel EQ error 
match1 += message LIKE Invalid URI in request

[sample23:w00t]
type = null
match1 = loglevel EQ error 
match1 += message LIKE request without hostname

[sample23:suspicious]
type = null
match1 = loglevel EQ error 
match1 += message LIKE SUSPICIOUS

[sample23:php]
type = null
match1 = loglevel EQ error 
match1 += message LIKE PHP

[sample23:zend]
type = null
match1 = loglevel EQ error 
match1 += message LIKE Zend Debugger

[sample23:apache_graceful]
type = null
match1 = loglevel EQ error 
match1 += message LIKE Bad file descriptor: apr_socket_accept

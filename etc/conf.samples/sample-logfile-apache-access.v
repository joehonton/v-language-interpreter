#==============================================================================
#
# File:         /var/vermont/conf.d/sample-logfile-apache-accesss
#
# Language:     Sample usage of the 'logfile' module with logformat = apache-access-log
#
# Author:       Joe Honton © 2011
#
# Initial date: January 13, 2011
#
#==============================================================================

#------------------------------------------------
[sample27:sample27]
type = logfile
output_limit = false
path =  /var/log/httpd
file_pattern = *
recursive = true
logformat = apache-access-log
match  = timestamp GT 6*60*60		# last 6 hours
match += timestamp LT 0*24*60*60

[sample27:access]
#match1 = client_ip EQ 207.182.146.170
match1 = request LIKE robots
message1  = $client_ip -
#message1 += $identity ;
#message1 += $userid ;
message1 += $timestamp -
message1 += $request 
#message1 += $response ;
#message1 += $object_size ;

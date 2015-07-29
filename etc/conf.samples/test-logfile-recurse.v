#==============================================================================
#
# File:         /var/vermont/
#
# Language:     Vermont [collection:object] specification
#
# Purpose:      scan all logfiles recursively to find selected client_ip
#
# Author:       Joe Honton © 2011
#
# Initial date: February 5, 2011
#
#==============================================================================


[monitor]
type = logfile
logformat = apache-access-log
path = /var/log/httpd
pattern = *
exclude = error_log*
recursive = true
message = $logline
match  = timestamp GE 10*24*60*60
match1 = request LIKE w00tw00t
match2 = client_ip EQ 123.100.117.179
match3 = client_ip EQ 173.192.38.199
output_limit = 100
trace_f
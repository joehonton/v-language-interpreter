#==============================================================================
#
# File:         /var/vermont/conf.d/sample-logfile-iptables
#
# Language:     Sample usage of the 'logfile' module with logformat = iptables-log
#
# Author:       Joe Honton © 2011
#
# Initial date: January 13, 2011
#
#==============================================================================

#------------------------------------------------
[sample26:sample26]
type = logfile
logformat = iptables-log
output_limit = false
output_line_number = false
path = /var/log
pattern = iptables.log
match  = timestamp	GT		1*24*60*60
match += timestamp	LT		0*24*60*60
#outputlevel = verbose
outputlevel = terse

[sample26:debug]
match1  = message LIKE SSH
message1  = $timestamp ;
#message1 += $message ;
message1 += $preamble ;
#message1 += $IN ;
#message1 += $OUT ;
#message1 += $MAC ;
message1 += $SRC ;
#message1 += $DST ;
#message1 += $LEN ;
#message1 += $TOS ;
#message1 += $PREC ;
#message1 += $TTL ;
#message1 += $ID ;
#message1 += $DF ;
#message1 += $PROTO ;
#message1 += $SPT ;
#message1 += $DPT ;
#message1 += $WINDOW ;
#message1 += $RES ;
#message1 += $SYN ;
#message1 += $URGP ;
output_limit = 5
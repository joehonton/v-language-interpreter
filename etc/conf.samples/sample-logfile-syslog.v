#==============================================================================
#
# File:         /var/vermont/conf.d/sample-logfile-syslog
#
# Language:     Sample usage of the 'logfile' module with logformat=syslog
#
# Author:       Joe Honton © 2010
#
# Initial date: December 30, 2010
#
#==============================================================================

#------------------------------------------------
[sample22:sample22]						# these are vmtModule keywords
type = logfile
output_limit = false
path = /var/log
pattern = iptables.log*
logformat = syslog
match  = timestamp	GT		2*24*60*60
match += timestamp	LT		0*24*60*60
message = $timestamp		 # $logline
#outputlevel = terse | verbose

# I've set the ratelimit too tight for legitimate DNS, so port 53 needs to be excluded in iptables
[sample22:dns_hashlimit]
#type = null
match1  = message LIKE HASHLIMIT
match1  = message NOTLIKE DPT=53

[sample22:infected]
type = null
match2  =  message LIKE INFECTED
#match1  = message LIKE 71.39.39.38			# this guy attempted to send outbound requests via port 80
#match1 += message LIKE SPT=80
ignore2 =  message LIKE DPT=443				# 443 is well known, but why is it still happening?
ignore2 += message LIKE 63.245.209.92
message2 = $timestamp
#match3 =  message LIKE DST=173.11.93.102	# why is 101 trying to connect to 102's privileged ports?

[sample22:outside_ip]
type = null
pattern = secure*
match1 =  message NOTLIKE 64.221.255.194
match1 += message NOTLIKE 173.11.93.101

# examine /var/log/secure for all successful logins, but ignore 173.11.93.97 and 64.221.255.194
[sample22:secure]
type = null
pattern = secure*
match1 = message LIKE Accepted password for root
ignore1    = message LIKE 173.11.93.97
ignore1   += message LIKE 64.221.255.194


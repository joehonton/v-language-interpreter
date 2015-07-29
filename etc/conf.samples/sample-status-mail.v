#==============================================================================
#
# File:         /var/vermont/conf.d/sample-status-mail
#
# Language:     Sample vermont configuration file showing how to verify that
#               mail is being accepted, transferred, and delivered.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 30, 2010
#
#==============================================================================

#------------------------------------------------
[sample20:sample20]
type = mail
from = vermont@postvitae.net
to = joe@joe-honton.com
subject_line = $now: Vermont mail system verification. (Test message to verify sending and receiving emails.)

[sample20:send]
type = null
arrival_timeout = false												# this will cause the message to be kept in the queue for pickup by the user's email client

[sample20:receive]
arrival_timeout = 10												# check for arrival; wait for 20 seconds before giving up
arrival_maildir = /home/joe-joe-honton-com/procmail-dovecot/new		# scan this directory, looking for arrival of the message just sent
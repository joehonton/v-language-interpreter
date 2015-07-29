#==============================================================================
#
# File:         /var/vermont/conf.d/sample-reminder
#
# Language:     Sample of vermont configuration file showing how to specify a reminder
#
# Author:       Joe Honton © 2010
#
# Initial date: December 30, 2010
#
#==============================================================================

[sample21:reminder]
type = reminder
alarm_date = 2011-02-28
alarm_time = 17:00
message = The time has come, it is now $timestamp.
email_to = joe@joe-honton.com
email_from = vermont@postvitae.com

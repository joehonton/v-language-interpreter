#==============================================================================
#
# File:         /var/vermont/conf.d/sample-reserved-words
#
# Language:     Samples of vermont configuration file reserved words
#
# Author:       Joe Honton © 2010
#
# Initial date: December 19, 2010
#
#==============================================================================

[sample14]
type = file
path = /var/vermont/log/vermont.log
#namespace = one								# this reserved word will issue a warning and be ignored
#object = two									# this reserved word will issue a warning and be ignored

[sample15]
type = file
path = /var/vermont/log/vermont.log
#extra = something								# the 'extra' attribute is not a recognized attribute of the 'file' type
                                                # this will issue an 'Unrecognized keyword' warning 

[sample16]
type = file
path = 
#pathname = /var/vermont/log/vermont.log         # the 'path' attribute is required
                                                # this will issue a 'Missing required keyword' error


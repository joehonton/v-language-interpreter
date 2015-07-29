#==============================================================================
#
# File:         /var/vermont/conf.d/sample-collection-object
#
# Language:     Samples of vermont configuration file collection:object syntax
#
# Author:       Joe Honton © 2010
#
# Initial date: December 18, 2010
#
#==============================================================================

[virthost1]							# an object named [sample-collection-object:virthost1] derived from the filename 'sample-collection-object'
type = file
path = /var/www/

[sample2:sample2]					# this is an explicitly named collection [sample2:sample2]
path = /var

[:sample2]							# this is the same as specifying [sample2:sample2]
path = /var

[sample2:]							# this is the same as specifying [sample2:sample2]
path = /var

[virthost2]							# this expands to [sample2:virthost2]
type = file
path = /var/www

[sample2:virthost3]					# this is a fully qualified collection:object name
type = file
path = /var/www/virtual-hosts


# Results in these composite objects:
# [sample-collection-object:virthost1]
#    path => /var/www/
# [sample2:virthost2]
#    path => /var
# [sample2:virthost3]
#    path => /var/www/virtual-hosts

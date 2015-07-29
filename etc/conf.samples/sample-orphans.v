#==============================================================================
#
# File:         /var/vermont/conf.d/sample-orphans
#
# Language:     Sample of vermont configuration file showing an orphan that will
#               not become part of any dependancy tree or object.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 18, 2010
#
#==============================================================================

[sample7:sample7]
type = file
path = /var/www/virtual-hosts/	# This is an orphan, as far as this file is concerned, because collection attributes
                                # are never directly executed; but it will not remain an orphan if another collection
                                # file specifies an object in the [sample:] collection.

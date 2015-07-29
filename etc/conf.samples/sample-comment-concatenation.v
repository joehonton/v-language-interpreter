#==============================================================================
#
# File:         /var/vermont/conf.d/sample-concatenation
#
# Language:     Sample of how attribute concatenation works
#
# Author:       Joe Honton © 2010
#
# Initial date: December 22, 2010
#
#==============================================================================

#------------------------------------------------
[sample11:sample11]
comment = A collection-level comment
type = file
path = /var

[sample11:A]
comment = An object level comment				# this overrides the collection's value

[sample11:B]
comment = 										# this erases the collection's value

[sample11:C]
comment += An object level comment				# this concatenates the collection's value and the object's value

[sample11:D]
comment += thing one							
comment += thing two							# this will result in three concatenated values

[sample11:E]
comment = thing one
comment += thing two							# this will result in just two concatenated values


#------------------------------------------------
[sample12:sample12]
type = file
path = /var

[sample12:A]
comment = thing one
comment += thing two							# this concatenates two object level values

[sample12:B]
comment = thing one
comment = thing two								# this overrides the first line, leaving a single value

[sample12:C]
comment += thing one
comment = thing two								# this overrides the first line, leaving a single value


#------------------------------------------------
[sample13:sample13]
comment = collection one | collection two
type = file
path = /var

[sample13:A]										# both collection-level attributes retained

[sample13:B]
comment = thing one | thing two					# replace collection's attributes with two object attributes

[sample13:C]
comment += thing one | thing two				# merge collection attributes and object attributes: results in four attributes


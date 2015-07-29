#==============================================================================
#
# File:         /var/vermont/conf.d/sample-pass-fail
#
# Language:     Sample usage of the pass and fail templates
#
# Author:       Joe Honton © 2010
#
# Initial date: December 21, 2010
#
#==============================================================================

#------------------------------------------------
[sample8:sample8]
type = file
pass = [$collection:$object] File $path OK. 			# this will only be visible with the -v flag
fail = [$collection:$object] File $path FAILED.			# this will always be visible when any error occurs

[A]
path = /var/vermont/sandbox/shovel
permissions = 644

[B]
path = /var/vermont/sandbox/shovel
permissions = 645


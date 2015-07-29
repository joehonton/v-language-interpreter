#==============================================================================
#
# File:         /var/vermont/conf.d/sample-file
#
# Language:     Sample usage of the file module
#
# Author:       Joe Honton © 2010
#
# Initial date: December 21, 2010
#
#==============================================================================

#------------------------------------------------
[sample6:sample6]
comment = Testing routine
type = file
owner = root:root
permissions = 644
selinux_type = var_t
# pass = [$collection:$object] File $path OK. 
# fail = [$collection:$object] File $path FAILED.
max_filesize = 1024*1024*1024*4  					# 4 Gigabytes

[A]
path = /var/vermont/sandbox/shovel

[B]
path = /var/vermont/sandbox/pail

[C]
path = /var/vermont/sandbox/castle

[D]
path = /var/vermont/etc/conf.samples/config

#==============================================================================
#
# File:         /var/vermont/conf.d/sample-selinux-file
#
# Language:     Sample file show how to verify SELinux context
#
# Author:       Joe Honton © 2010
#
# Initial date: December 23, 2010
#
#==============================================================================

#------------------------------------------------
[sample9:sample9]
type = file

[A]
path = /var/vermont/sandbox/shovel				# specify each context individually
selinux_user = unconfined_u
selinux_role = object_r
selinux_type = var_t
selinux_level = s0

[B]
path = /var/vermont/sandbox/castle				# specify the entire context using a single attribute
selinux = unconfined_u:object_r:var_t:s0

[C]
path = /var/vermont/sandbox/pail				# specify only the context type, ignoring user, role and level
selinux_type = var_t


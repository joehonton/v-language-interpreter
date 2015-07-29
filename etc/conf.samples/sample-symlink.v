#==============================================================================
#
# File:         /var/vermont/conf.d/sample-symlink
#
# Language:     Sample file to show how to verify a symbolic link
#
# Author:       Joe Honton © 2010
#
# Initial date: December 23, 2010
#
#==============================================================================

#------------------------------------------------
[sample10:sample10]
type = symlink

[A]
symlink_path = /var/vermont/sandbox/points-to-castle	# simply check to see that the file is a symbolic link file

[B]
symlink_path = /var/vermont/sandbox/points-to-castle
target_path = /var/vermont/sandbox/castle			# is the target of the symlink exactly this name

[C]
symlink_path = /var/vermont/sandbox/points-to-castle
validate = true									# does the target exist (whatever it is)

[D]
symlink_path = /var/vermont/sandbox/points-to-castle
target_path = /var/vermont/sandbox/castle			# is the target of the symlink exactly this name and . . .
validate = true									# . . . does the target exist

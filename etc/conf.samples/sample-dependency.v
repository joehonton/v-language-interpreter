#==============================================================================
#
# File:         /var/vermont/conf.d/sample-dependency
#
# Language:     Samples of vermont configuration file dependency usage
#
# Author:       Joe Honton © 2010
#
# Initial date: December 19, 2010
#
#==============================================================================

#------------------------------------------------
[sample5:sample5]
type = file
path = /var
runtime_trace = false

[sample5:A]
dependency = sample5:B
runtime_trace = true

[sample5:B]
dependency = sample5:C | sample5:D		# both dependencies must be satisfied

[sample5:C]

[sample5:D]

#[sample5:E]
#dependency = sample5:F

#[sample5:F]
#dependency = sample5:E					# this is a cyclic dependency and will be ignored


[sample6:X]
type = null
runtime_trace = false


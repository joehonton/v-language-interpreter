#==============================================================================
#
# File:         /var/vermont/conf.d/sample-glob
#
# Language:     Sample usage of the 'glob' module
#
# Author:       Joe Honton © 2010
#
# Initial date: December 29, 2010
#
#==============================================================================


#------------------------------------------------
[sample16G:sample16G]
type = glob
path =  /var/vermont/etc #/logwatch
owner = root:root
#permissions = 644
selinux_type = var_t
output_limit = false
recursive = true
trace_filenames = true
follow_symlinks = true

#------------------------------------------------
[sample16G:A]
#file_pattern = v*|a*
#directory_pattern = 007 | 008
#file_exclude = *007* | *008*
directory_exclude = conf.hourly.007 | conf.hourly.008 | conf.daily.007 | conf.daily.008
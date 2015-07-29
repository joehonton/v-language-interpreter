#==============================================================================
#
# File:         /var/vermont/conf.d/sample-git
#
# Language:     Sample usage of the git module
#
# Author:       Joe Honton © 2011
#
# Initial date: January 14, 2011
#
#==============================================================================

#------------------------------------------------
[sample29:sample29]
type = git
owner = root:committers
permissions = 644
selinux_type = var_t

[sample29:A]
path = /var/www/virtual-hosts/postvitae.net/www/http
modified_files = true
untracked_files = true
unstaged_changes = true
pending_commits = true


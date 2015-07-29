#==============================================================================
#
# File:         /var/vermont/conf.d/sample-virtualhost
#
# Language:     Sample file to show how to verify an Apache virtual host
#
# Author:       Joe Honton © 2010
#
# Initial date: December 23, 2010
#
#==============================================================================

[sample]
#type = virtualhost
type = null
trace_filenames = false
output_limit = 50
recursive = true

path  = /var/www/virtual-hosts/postvitae.com/www/http
exclude = .git

owner = apache:apache
directory_permissions = 700
file_permissions = 600
selinux_type = httpd_sys_content_t

log_path  = /var/log/httpd/postvitae.com/www
log_owner = root:root
log_directory_permissions = 750
log_file_permissions = 640
log_selinux_type = httpd_log_t


#------------------------------------------------
# git for www.postvitae.com
#------------------------------------------------
[www.postvitae.com-git:www.postvitae.com-git]
type = glob
output_limit = 150
recursive = true
trace_filenames = true
path  = /var/www/virtual-hosts/postvitae.com/www/http/.git
owner = root:committers
directory_permissions = 770
selinux_type = git_system_content_t

[www.postvitae.com-git:not-objects]
directory_pattern = *
file_pattern = *
directory_exclude = objects
file_permissions = 660

[www.postvitae.com-git:objects]
directory_pattern = objects 
file_pattern = *
file_permissions = 440


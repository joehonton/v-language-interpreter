#==============================================================================
#
# File:         /var/vermont/conf.d/sample-mysql
#
# Language:     Sample of vermont configuration file showing how to verify 
#               the presence of MySQL database tables.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 29, 2010
#
#==============================================================================

[sample17:A]
type = mysql
user = vermont
password = ver8300mont
database = postvitae_search
table = unpacked_index
table += meta_schema
table += meta_columns

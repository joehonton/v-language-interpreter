#==============================================================================
#
# File:         /var/vermont/conf.d/sample-attribute-concatenation
#
# Language:     Samples of vermont configuration file attribute concatenation
#
# Author:       Joe Honton © 2010
#
# Initial date: December 19, 2010
#
#==============================================================================

[sample1:sample1]
type = file
path = /var/www

[sample1:virthostA]
path += /var/www/virtual-hosts		# concatenates to: /var/www|/var/www/virtual-hosts

[sample1:virthostB]                  # concatenates to: /var/www/|/var/www/virtual-hosts/joe-honton.com/|/var/www/virtual-hosts/full-stack.com
path += /var/www/virtual-hosts/joe-honton.com | /var/www/virtual-hosts/full-stack.com
                                   
[sample1:virthostC]
path += /var/www					# duplicate value is ignored, resulting in: /var/www

[sample1:virthostD]
path = 								# removes the attribute completely, overriding any parent value

[sample1:sample1]
#path += /var/www/postvitae.com		# caution: this will change the values of the first three objects, but not the fourth
									# sample1:virthostA => /var/www/|/var/www/postvitae.com|/var/www/A/
									# sample1:virthostB => /var/www/|/var/www/postvitae.com|/var/www/virtual-hosts/joe-honton.com|/var/www/virtual-hosts/full-stack.com
									# sample1:virthostC => /var/www/|/var/www/postvitae.com
									# sample1:virthostD => 

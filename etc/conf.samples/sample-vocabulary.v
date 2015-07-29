#==============================================================================
#
# File:         /var/vermont/conf.d/sample-vocabulary
#
# Language:     Sample file to show how to set vobabulary checking to lax
#
# Author:       Joe Honton © 2011
#
# Initial date: February 1, 2011
#
#==============================================================================

#------------------------------------------------
[sample27:sample27]
type = file
path = /var/vermont/sandbox/shovel

[A]
#newkeyword = hello

[B]
vocabulary_check = strict
#newkeyword = hello

[C]
vocabulary_check = lax
newkeyword = hello


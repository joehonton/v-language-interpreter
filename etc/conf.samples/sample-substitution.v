#==============================================================================
#
# File:         /var/vermont/conf.d/sample-substitution
#
# Language:     Vermont declarative language
#
# Purpose:      A sample showing how keyword substitution works.
#
# Author:       Joe Honton © 2011
#
# Initial date: January 31, 2011
#
#==============================================================================

#------------------------------------------------
[sample25:A]
type = substitution
mykeyword = virtualhosts-A

[sample26:sample26]
type = null

[sample26:B]
mykeyword = virtualhosts-B

[sample26:C]
pass = A simple comment

[sample26:D]
mykeyword = virtualhosts-D
pass = A simple substitution mykeyword=[mykeyword]

[sample26:E]
pass = An object.keyword substitution mykeyword=[B.mykeyword]

[sample26:F]
pass = A namespace:object.keyword substitition mykeyword=[sample25:A.mykeyword]

[sample26:G]
pass = Multiple substitutions mykeyword=[B.mykeyword] and mykeyword=[D.mykeyword]

# Cascading substitutions occur when the substitution value itself contains a substitution variable
# In these scenarios, the inner variable will take on the scope of the outer object so explict
# and implicit namespaces and objects have distinct meanings. Both have valid usage patterns.

[sample26:H]
pass = Cascading implicit D pass="[D.pass]"		# this contains an undeclared variable because the inner [mykeyword] resolves to [sample26:H.mykeyword] which doesn't exist.

[sample26:I]
mykeyword = virtualhosts-I
pass = Cascading implicit D pass="[D.pass]"

[sample26:J]
pass = Cascading explicit E pass="[E.pass]"

[sample27:K]
type = null
pass = Cascading explicit E pass="[E.pass]"		# this contains an undeclared variable because the inner [B.mykeyword] resolves to [sample27:B.mykeyword] which doesn't exists.


# Recursion is limited to two levels as a stop-gap to catch substitution variables that point to themselves

[sample26:L]
pass = sample26L->[pass]

[sample26:M]
pass = sample26M->[L.pass]

[sample26:N]
pass = sample26N->[sample26:L.pass]

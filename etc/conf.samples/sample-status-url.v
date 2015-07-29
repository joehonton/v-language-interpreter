#==============================================================================
#
# File:         /var/vermont/conf.d/sample-status-url
#
# Language:     Sample of vermont configuration file showing how to verify 
#               whether web pages are reachable, blocked, or not found.
#
# Author:       Joe Honton © 2010
#
# Initial date: December 29, 2010
#
#==============================================================================

[sample19:sample19]
type = url

[sample19:OK]
url = http://www.postvitae.com
http_status = 200

[sample19:no_follow]
url = http://graticule.postvitae.com
http_status = 302
follow = false

[sample19:simulate_cache1]
url = http://www.postvitae.com/favicon.ico
http_status = 200
simulate_cache = false

[sample19:simulate_cache2]
url = http://www.postvitae.com/favicon.ico
http_status = 200
simulate_cache = 60*60*24*365*2			# is the server's file less than 2 years old?

[sample19:simulate_cache3]
url = http://www.postvitae.com/favicon.ico
http_status = 304
simulate_cache = 60*60*365*1			# is the server's file more than 1 year old?		

[sample19:forbidden]
url = http://www.postvitae.net/php-libraries/ssk/ajf.inc
http_status = 403

[sample19:not_found]
url = http://www.postvitae.com/404
http_status = 404

[sample19:slow]
url = http://tenten-taiko.com
slow_response = 100


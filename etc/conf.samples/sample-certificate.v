#==============================================================================
#
# File:         /var/vermont/conf.d/sample-package
#
# Language:     Sample of vermont configuration file showing usage of the
#               certificate module
#
# Author:       Joe Honton © 2010
#
# Initial date: December 28, 2010
#
#==============================================================================

[sample15:sample15]
type = certificate
owner = root:root
permissions = 644
selinux_type = cert_t
expires_on = 2011-02-17

[sample15:signin]
certificate = /etc/pki/tls/certs/signin.postvitae.com.crt
certificate_private_key = /etc/pki/tls/private/signin.postvitae.com.key
certificate_request = /etc/pki/tls/csr/signin.postvitae.com.csr

[sample15:secure]
certificate = /etc/pki/tls/certs/secure.postvitae.com.crt
certificate_private_key = /etc/pki/tls/private/secure.postvitae.com.key
certificate_request = /etc/pki/tls/csr/secure.postvitae.com.csr

[sample21:reminder]
type = reminder
alarm_date = 2011-02-10
message = The SSL certificates will be expiring on Feb 17, 2010.  Time to renew.
email_to = joe@joe-honton.com
email_from = vermont@postvitae.com

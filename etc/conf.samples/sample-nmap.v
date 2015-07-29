#==============================================================================
#
# File:         /var/vermont/conf.d/sample-nmap
#
# Language:     Sample usage of the nmap module
#
# Author:       Joe Honton © 2011
#
# Initial date: January 17, 2011
#
#==============================================================================

#------------------------------------------------
[sample30:sample30]
type = nmap
ip_address = 173.11.93.101
output_limit = false

[sample30:A]
hidden_versions = false		 # no way to suppress OpenSSH version number without recompiling source
open_tcp_ports  = 22		 # SSH	

[sample30:B]
hidden_versions = true

closed_tcp_ports += 1-21
closed_tcp_ports += 23-24
open_tcp_ports   += 25		 # Sendmail
closed_tcp_ports += 26-52
open_tcp_ports   += 53		 # DNS/Bind
closed_tcp_ports += 54-79
open_tcp_ports   += 80		 # Apache
closed_tcp_ports += 81-109
open_tcp_ports   += 110		 # Dovecot POP3
#open_tcp_ports  += 111		 # rpcbind (open on local network, filtered from outside)
closed_tcp_ports += 112-122
closed_tcp_ports += 123		 # NTP does not use TCP
closed_tcp_ports += 124-142
#open_tcp_ports  += 143		 # Dovecot imap (open on local network, filtered from outside)
closed_tcp_ports += 144-442
open_tcp_ports   += 443		 # Apache SSH
closed_tcp_ports += 444-1024
#open_tcp_ports += 3306		 # MySQL (open on local network, filtered from outside)

closed_udp_ports += 1-52
open_udp_ports   += 53		 # DNS/Bind
closed_udp_ports += 54-122
open_udp_ports   += 123		 # NTP uses UDP
closed_udp_ports += 124-1024

# Jan 18, 2010
# scan from local network revealed open TCP ports 111,143,3306; and open|filered UDP ports 111,631,878
# scan from outside network lists TCP port 111,143,3306 as filtered; UDP 111,878 as closed; UDP port 631 as filtered; 

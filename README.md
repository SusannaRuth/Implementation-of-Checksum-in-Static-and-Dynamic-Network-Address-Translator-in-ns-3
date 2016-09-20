# Implementation of Checksum in Static and Dynamic Network Address Translator in ns-3

##Course Code: CS822

##Assignment: #FP5

###Overview:

NAT was proposed in RFC-1631 [1] to overcome the problem of IP address depletion. NAT is reponsible for maintaining a table mapping of local IP & port tuples to one or more globally unique IP and port tuples. NAT devices act as simple packet filter since it has no way to automatically determine the internal route, unless explicitly configured by the user through a port forwarding or similar mechanism. Three types of NAT are available: Static NAT, Dynamic NAT, Network Address Port Translation [2]. We will be working with static and dynamic NAT.

###Static NAT: 
Mapping of IP address happens statically, that is, every private IP address has a definite public IP specified by the computer.There is no need to look in pool.

###Dynamic NAT: 
Which ever public IP is free in pool is assigned to the private IP. Dynamic NAT helps to secure a network as it masks the internal configuration of a private network and makes it difficult for someone outside the network to monitor individual usage patterns. Advantage of dynamic NAT is that it allows a private network to use private IP addresses that are invalid on the Internet but useful as internal addresses.

NAT model has already been implemented in ns-3 framework [3].
We will be working to add checksum checks for static and dynamic NAT.


###References:
[1] https://www.ietf.org/rfc/rfc1631.txt

[2] http://ieeexplore.ieee.org/document/6394258/

[3] https://www.nsnam.org/wiki/GSOC2012NetworkAddressTranslation

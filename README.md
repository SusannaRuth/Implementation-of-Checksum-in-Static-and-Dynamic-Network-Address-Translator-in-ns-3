# network-address-translator
NAT is reponsible for maintaining a table mapping of local IP & port tuples to one or more globally unique IP and port tuples.
NAT devices act as simple packet filter since it has no way to automatically determine the internal route, unless explicitly configured by the user through a port forwarding or similar mechanism.
STATIC NAT:Mapping of ip address happens statically that is if something is private it has a definite public IP specified by the computer.There is no need to look in pool.
DYNAMIC NAT:Which ever public IP is free in pool is assigned.Dynamic NAT helps to secure a network as it masks the internal configuration of a private network and makes it difficult for someone outside the network to monitor individual usage patterns.Advantage of dynamic NAT is that it allows a private network to use private IP addresses that are invalid on the Internet but useful as internal addresses.

REFERENCES
https://www.nsnam.org/wiki/GSOC2012NetworkAddressTranslation
http://ieeexplore.ieee.org/document/6394258/
https://en.wikipedia.org/wiki/Network_address_translation

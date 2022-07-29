addr2ethers
===========

This is a tool similar to addrwatch. It main purpose is to monitor network and 
maintain /etc/ethers.

Main features of addr2ethers:

* IPv4  address monitoring
* Monitoring multiple network interfaces with one daemon
* Monitoring of VLAN tagged (802.1Q) packets.
* Output to stdout, plain text file


The main difference between addrwatch and addr2ethers is that add2ethers is only
ment for maintaining the /etc/ethers file in a dumb ap openwrt scenario, making
luci and ubus show the ip next to mac address of with connected wlan stations.

Addrwatch store the data in an incompatible format compared to /etc/ethers. This 
can be solved with 
`addrwatch --ipv4-only --ratelimit=-1 br-lan | gawk '{print $4, $5; system("")}' > /etc/ethers &`
addr2ethers makes it a bit easier to run it as a service, and removes unneeded
features.

addr2ethers do not keep persistent network pairings state but instead logs all 
the events that allow ethernet/ip pairing discovery. For IPv4 it is ARP 
requests, ARP replies and ARP ACD (Address Conflict Detection) packets.

The output file produced by addr2ethers is similar to arpwatch. Example of
addr2ethers output file:

```
1329486484 eth0 0 00:aa:bb:cc:dd:ee fe80::2aa:bbff:fecc:ddee ND_NS
1329486485 eth0 0 00:aa:bb:cc:dd:ee 192.168.1.1 ARP_REQ
1329486485 eth0 0 00:aa:bb:ff:00:11 192.168.1.3 ARP_ACD
1329486486 eth0 7 00:11:11:11:11:11 fe80::211:11ff:fe11:1111 ND_NS
1329486487 eth0 7 00:22:22:22:22:22 fe80::222:22ff:fe22:2222 ND_DAD
1329486488 eth0 7 00:33:33:33:33:33 192.168.2.2 ARP_REQ
```

For each pairing discovery event addr2ethers produce time-stamp, interface, 
vlan_tag (untagged packets are marked with 0 vlan_tag), ethernet address, IP 
address and packet type separated by spaces.

To prevent addr2ethers from producing too many duplicate output data in active
networks rate-imiting should be used. Read more in 'Ratelimit' section. 


Installation
------------

To compile addr2ethers you must have following shared libraries:

* libpcap
* libevent


To compile addr2ethers:

```
$ ./configure
$ make
$ make install
```

If you do not want to install addr2ethers to the system, skip the 'make install' 
step. You can find main addr2ethers binary and all output addr2ethers\_\* binaries
in 'src' directory.

Building from repo
------------------

If sources are obtained directly from the git repository (instead of
distribution source package) project has to be bootstrapped using
autoreconf/automake. A helper shell script `bootstrap.sh` is included in the
repository for that. Note that bootstraping autotools project requires autoconf
and automake to be available on the system.

Example command to bootstrap autotools:

```
./bootstrap.sh
```

To automatically check source style before commiting use provided pre-commit git
hook:

```
git config core.hooksPath .githooks
```

Usage
-----

To simply try out addr2ethers start it without any arguments:

```
$ addr2ethers
```

When started like this addr2ethers opens first non loopback interface and start
logging event to the console without writing anything to disk. All events
are printed to stdout, debug, warning, and err messages are sent to syslog and
printed to stderr.

If you get error message:
addr2ethers: ERR: No suitable interfaces found!

It usually means you started addr2ethers as normal user and do not have sufficient
privileges to start sniffing on network interface. You should start addr2ethers as
root:

```
$ sudo addr2ethers
```

You can specify which network interface or interfaces should be monitored by
passing interface names as arguments. For example:

```
$ addr2ethers eth0 tap0
```

To find out about more usage options:

```
$ addr2ethers --help
```

In production environment it is recommended to start main addr2ethers binary in a
daemon mode, and use separate output processes for logging data. Example:

```
$ ./addr2ethers -d eth0
$ ./addr2ethers_stdout
```

Ratelimiting
------------

If used without ratelimiting addr2ethers reports etherment/ip pairing everytime it
gets usable ARP or IPv6 ND packet. In actively used networks it generates many
duplicate pairings especially for routers and servers.

Ratelimiting option '-r NUM' or '--ratelimit=NUM' surpress output of duplicate
pairings for at least NUM seconds. In other words if addr2ethers have discovered 
some pairing (mac,ip) it will not report (mac,ip) again unless NUM seconds have
passed.

There is one exception to this rule to track ethernet address changes. If
addr2ethers have discovered pairings: (mac1,ip),(mac2,ip),(mac1,ip) within
ratelimit time window it will report all three pairings. By doing so 
ratelimiting will not loose any information about pairing changes.

For example if we have a stream of events:

| time | MAC address       | IP address
|------|-------------------|------------
| 0001 | 11:22:33:44:55:66 | 192.168.0.1
| 0015 | 11:22:33:44:55:66 | 192.168.0.1
| 0020 | aa:bb:cc:dd:ee:ff | 192.168.0.1
| 0025 | aa:bb:cc:dd:ee:ff | 192.168.0.1
| 0030 | 11:22:33:44:55:66 | 192.168.0.1
| 0035 | 11:22:33:44:55:66 | 192.168.0.1
| 0040 | aa:bb:cc:dd:ee:ff | 192.168.0.1
| 0065 | aa:bb:cc:dd:ee:ff | 192.168.0.1

With --ratelimit=100 we would get:

| time | MAC address       | IP address
|------|-------------------|------------
| 0001 | 11:22:33:44:55:66 | 192.168.0.1
| 0020 | aa:bb:cc:dd:ee:ff | 192.168.0.1
| 0030 | 11:22:33:44:55:66 | 192.168.0.1
| 0040 | aa:bb:cc:dd:ee:ff | 192.168.0.1

Without such exception output would be:

| time | MAC address       | IP address
|------|-------------------|------------
| 0001 | 11:22:33:44:55:66 | 192.168.0.1
| 0020 | aa:bb:cc:dd:ee:ff | 192.168.0.1

And we would loose information that address 192.168.0.1 was used by ethernet
address 11:22:33:44:55:66 between 30-40th seconds.

To sum up ratelimiting reduces amount of duplicate information without loosing
any ethernet address change events.

Ratelimit option essentially limits data granularity for IP address usage 
duration information (when and for what time period specific IP address was 
used). On the other hand without ratelimiting at all you would not get very 
precise IP address usage duration information anyways because some hosts might 
use IP address without sending ARP or ND packets as often as others 
do.


If NUM is set to 0, ratelimiting is disabled and all pairing discovery events
are reported.

If NUM is set to -1, ratelimiting is enabled with infinitely long time window
therefore all duplicate pairings are suppressed indefinitely. In this mode 
addr2ethers acts almost as arpwatch with the exception that ethernet address 
changes are still reported.

It might look tempting to always use addr2ethers with --ratelimit=-1 however by
doing so you loose the information about when and for what period of time 
specific IP address was used. There will be no difference between temporary IPv6
addressed which was used once and statically configured permanent addresses.

Event types
-----------

Ethernet/ip pairing discovery can be triggered by these types of events:

* ARP_REQ - ARP Request packet. Sender hardware address (ARP header) and sender
  protocol address (ARP header) is saved.
* ARP_REP - ARP Reply packet. Sender hardware address (ARP header) and sender
  protocol address (ARP header) is saved.
* ARP_ACD - ARP Address collision detection packet. Sender hardware address
  (ARP header) and target protocol address (ARP header) is saved.
* ND_NS - Neighbor Solicitation packet. Source link-layer address (NS option)
  and source address (IPv6 header) is saved.
* ND_NA - Neighbor Advertisement packet. Target link-layer address (NA option)
  and source address (IPv6 header) is saved.
* ND_DAD - Duplicate Address Detection packet. Source MAC (Ethernet header)
  and target address (NS header) is saved.


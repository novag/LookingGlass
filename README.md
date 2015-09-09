# CLG - LookingGlass

## Overview

CLG - LookingGlass is a user-friendly BGP looking glass for quagga and bird.

The reason for the development was my participation in [dn42](https://dn42.net/) using a small OpenWRT router. At this time there were no reasonable looking glasses for quagga.  
Due to the small memory of my OpenWRT router and to gather experience with C, I decided to write the demon in C.

Later, I migrated my peerings to a larger VPS and used bird as BGP daemon. Therefore LookingGlass now supports bird and quagga (partially).

ajax.php version: v1.3.0  
map.php version:  v1.0.0

## Demo

[CLG - LookingGlass](http://de-fra1.netrik.de/#traceroute/8.8.8.8)

## Features

* IPv4 & IPv6 support
* Live output (streaming)
* Multiple themes (must be added manually)
* Rate limiting of network commands

## Implemented commands (IPv4 & IPv6)

* whois
* tcpconnect
* tcpconnect6
* ping
* ping6
* traceroute
* traceroute6
* summary
* summary6
* route
* route6
* bgpmap (IPv4 only)
* as
* as6

## Requirements - PHP Frontend

* PHP >= 5.3
* [Image_GraphViz](https://pear.php.net/package/Image_GraphViz)
* PHP PDO with SQLite driver (required for rate-limit)

## Requirements - C Daemon

* libevent2

## Install

1. Clone LookingGlass frontend to the intended folder within your web directory
2. Configure LookingGlass (LookingGlass/Config.php)
3. Block public access to files under LookingGlass/*

## License

First, I'd like to thank Telephone and his work on his [Looking Glass](https://github.com/telephone/LookingGlass).  
It has a clear and structured theme, which I use here as well. I also use the rate limitting class from Telephone.

Code is licensed under GNU General Public License, version 3 (GPL-3.0), except for LookingGlass/RateLimit.php which is licensed under MIT Public License.
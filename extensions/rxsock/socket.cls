/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* Description: Simple classes to encalsulate stream sockets.                 */

::requires "rxsock" LIBRARY    -- need the rxsock library available to function

-- a holder class that provides common option tables
-- Note that this is NOT public
::class SocketOptions
::method activate class
  expose optionNames domainNames domainTypes protocols families
  -- check args
  optionNames = .set~of('SO_BROADCAST', 'SO_DEBUG', 'SO_DONTROUTE', 'SO_ERROR',,
                      'SO_KEEPALIVE', 'SO_LINGER', 'SO_OOBINLINE', 'SO_RCVBUF',,
                      'SO_RCVLOWAT', 'SO_RCVTIMEO', 'SO_REUSEADDR', 'SO_SNDBUF',,
                      'SO_SNDLOWAT', 'SO_SNDTIMEO', 'SO_TYPE', 'SO_USELOOPBACK')
  domainNames = .set~of('AF_INET')
  families = domainNames
  domainTypes = .set~of('SOCK_STREAM', 'SOCK_DGRAM', 'SOCK_RAW')
  protocols = .set~of(0, 'IPPROTO_UDP', 'IPPROTO_TCP')

::attribute optionNames GET CLASS
::attribute domainNames GET CLASS
::attribute domainTypes GET CLASS
::attribute protocols   GET CLASS
::attribute families    GET CLASS


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Socket - a class for stream sockets                                 */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class 'Socket' public
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Socket                                                              */
/*        Class methods                                                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: gethostbyaddr                                                      */
/* Description: get a host information by address                             */
/* Arguments:                                                                 */
/*         hostaddr - host ip address                                         */
/*----------------------------------------------------------------------------*/

::method gethostbyaddr class
use strict arg hostaddr
return .HostInfo~new(hostaddr)

/*----------------------------------------------------------------------------*/
/* Method: gethostbyname                                                      */
/* Description: get a host information by name                                */
/* Arguments:                                                                 */
/*         hostname - host name (must be a known DNS entry)                   */
/*----------------------------------------------------------------------------*/

::method gethostbyname class
use strict arg hostname
return .HostInfo~new(hostname)

/*----------------------------------------------------------------------------*/
/* Method: gethostid                                                          */
/* Description: get the local host ip address                                 */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method gethostid class
use strict arg
return SockGetHostID()

/*----------------------------------------------------------------------------*/
/* Method: gethostname                                                        */
/* Description: get the local host name                                       */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method gethostname class
use strict arg
return SockGetHostName()

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Socket                                                              */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: convert_address_family                                             */
/* Description: convert a number to an address family.                        */
/*----------------------------------------------------------------------------*/

::method convert_address_family private
use strict arg fam
if fam~datatype('W') <> 1 then return fam
select
   when fam = 0 then return 'AF_UNSPEC'
   when fam = 1 then return 'AF_LOCAL'
   when fam = 2 then return 'AF_INET'
   when fam = 3 then return 'AF_AX25'
   when fam = 4 then return 'AF_IPX'
   when fam = 5 then return 'AF_APPLETALK'
   when fam = 6 then return 'AF_NETROM'
   when fam = 7 then return 'AF_BRIDGE'
   when fam = 8 then return 'AF_ATMPVC'
   when fam = 9 then return 'AF_X25'
   when fam = 10 then return 'AF_INET6'
   when fam = 11 then return 'AF_ROSE'
   when fam = 12 then return 'AF_DECnet'
   when fam = 13 then return 'AF_NETBEUI'
   when fam = 14 then return 'AF_SECURITY'
   when fam = 15 then return 'AF_KEY'
   when fam = 16 then return 'AF_NETLINK'
   when fam = 17 then return 'AF_PACKET'
   when fam = 18 then return 'AF_ASH'
   when fam = 19 then return 'AF_ECONET'
   when fam = 20 then return 'AF_ATMSVC'
   when fam = 22 then return 'AF_SNA'
   when fam = 23 then return 'AF_IRDA'
   when fam = 24 then return 'AF_PPPOX'
   when fam = 25 then return 'AF_WANPIPE'
   when fam = 31 then return 'AF_BLUETOOTH'
   otherwise nop
   end
return fam

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Socket                                                              */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute errno get

/*----------------------------------------------------------------------------*/
/* Method: accept                                                             */
/* Description: accept a connection                                           */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method accept unguarded
expose s errno
use strict arg
newsocket = SockAccept(s)
if newsocket = -1 then
   return .nil
else
   return .socket~new(newsocket)

/*----------------------------------------------------------------------------*/
/* Method: bind                                                               */
/* Description: bind a socket to a address/port                               */
/* Arguments:                                                                 */
/*         hostaddr - a host address of class InetAddress                     */
/*----------------------------------------------------------------------------*/

::method bind
expose s errno
use strict arg address
.validate~classType('hostaddr', address, .InetAddress)
stem. = address~makeStem()
retc = SockBind(s, 'stem.!')
-- It should be noted here that the SockBind() function can give a false
-- indication of success when in fact it did not bind to anything.
return retc

/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: shutdown and close a socket                                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method close
expose s errno
use strict arg
call SockShutDown s, 2
retc = SockClose(s)
s = -1
return retc

/*----------------------------------------------------------------------------*/
/* Method: connect                                                            */
/* Description: connect a socket to a remote address                          */
/* Arguments:                                                                 */
/*         hostaddr - a host address of class InetAddress                     */
/*----------------------------------------------------------------------------*/

::method connect
expose s errno
use strict arg address

.validate~classType('hostaddr', address, .InetAddress)
stem. = address~makeStem()
retc = SockConnect(s, 'stem.!')
return retc

/*----------------------------------------------------------------------------*/
/* Method: getOption                                                          */
/* Description: return a socket option                                        */
/* Arguments:                                                                 */
/*         name - the name of an option                                       */
/*----------------------------------------------------------------------------*/

::method getOption
expose s errno
use strict arg name
name = name~upper()
-- check args
if .SocketOptions~optionNames~hasItem(name) = .false then ,
 raise syntax 93.914 array (1, .SocketOptions~optionNames~makeArray~toString, name)
-- get the value
retc = SockGetSockOpt(s, 'SOL_SOCKET', name, "value")
-- some values may not be gettable
if retc = -1 then
   return .nil
return value

/*----------------------------------------------------------------------------*/
/* Method: getPeerName                                                        */
/* Description: get the peer name connected to a socket                       */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method getPeerName
expose s errno
use strict arg
retc = SockGetPeerName(s, 'stem.!')
if retc = -1 then
   return .nil
return .InetAddress~new(stem.!addr, stem.!port, self~convert_address_family(stem.!family))

/*----------------------------------------------------------------------------*/
/* Method: getSockName                                                        */
/* Description: get the name of the socket                                    */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method getSockName
expose s errno
use strict arg
retc = SockGetSockName(s, 'stem.!')
if retc = -1 then
   return .nil
return .InetAddress~new(stem.!addr, stem.!port, self~convert_address_family(stem.!family))

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*              If only one argument is supplied it is assumed to be an       */
/*              existing socket. Otherwise a new socket is created.           */
/* Arguments:                                                                 */
/*         domain   - (optional) must be AF_INET                              */
/*         type     - (optional) must be SOCK_STREAM, SOCK_DGRAM or SOCK_RAW  */
/*         protocol - (optional, if not supplied will be set to 0)            */
/*----------------------------------------------------------------------------*/

::method init
expose s errno
-- are we being passed an existing socket?
if arg() = 1 then do
   use strict arg s
   .validate~positiveWholeNumber('socket', s)
   return
   end
-- assume we want a new socket created
use strict arg domain = 'AF_INET', type = 'SOCK_STREAM', protocol = 0
domain = domain~upper()
type= type~upper()
protocol = protocol~upper()
-- check arguments
if .SocketOptions~domainNames~hasItem(domain) = .false then ,
 raise syntax 93.914 array ('domain', .SocketOptions~domainNames~makearray~toString, domain)
if .SocketOptions~domainTypes~hasItem(type) = .false then ,
 raise syntax 93.914 array ('type', .SocketOptions~domainTypes~makearray~toString, type)
if .SocketOptions~protocols~hasItem(protocol) = .false then ,
 raise syntax 93.914 array ('protocol', .SocketOptions~protocols~makearray~toString, protocol)
-- create the socket
s = SockSocket(domain, type, protocol)
return

/*----------------------------------------------------------------------------*/
/* Method: ioctl                                                              */
/* Description: perform a special operation on a socket                       */
/* Arguments:                                                                 */
/*         cmd  - the command to send                                         */
/*         data - the command date                                            */
/*----------------------------------------------------------------------------*/

::method ioctl
expose s errno
use strict arg cmd, data
retc = SockIoctl(s, cmd, data)
if retc = -1 then
   return .nil
return retc

/*----------------------------------------------------------------------------*/
/* Method: listen                                                             */
/* Description: listen for connections on a socket                            */
/* Arguments:                                                                 */
/*         backlog - the backlog to use for pending connection requests       */
/*----------------------------------------------------------------------------*/

::method listen
expose s errno
use strict arg backlog
.validate~nonNegativeWholeNumber('backlog', backlog)
retc = SockListen(s, backlog)
return retc

/*----------------------------------------------------------------------------*/
/* Method: recv                                                               */
/* Description: receive data on a socket                                      */
/* Arguments:                                                                 */
/*         len - the maximum amount of data to receive in bytes               */
/*----------------------------------------------------------------------------*/

::method recv
expose s errno
use strict arg len
.validate~length('length', len)
retc = SockRecv(s, 'xxx', len)
if retc = -1 then
   return .nil
if retc = 0 then return ''
return xxx

/*----------------------------------------------------------------------------*/
/* Method: recvFrom                                                           */
/* Description: receive data on a socket from a specified address             */
/* Arguments:                                                                 */
/*         len      - the maximum amount of data to receive in bytes          */
/*         addressr - a host address of class InetAddress                     */
/*----------------------------------------------------------------------------*/

::method recvFrom
expose s errno
use strict arg len, address
.validate~positiveWholeNumber('length', len)
.validate~classType('address', address, .InetAddress)
stem. = address~makeStem()
retc = SockRecvFrom(s, 'xxx', len, 'stem.!')
if retc = -1 then
   return .nil
address~family= self~convert_address_family(stem.!family)
address~address=Stem.!addr
address~port=   Stem.!port
if retc = 0 then return ''
return xxx

/*----------------------------------------------------------------------------*/
/* Method: select                                                             */
/* Description: monitor activity on a set of sockets                          */
/* Arguments:                                                                 */
/*         reads   - an array of sockets                                      */
/*         writes  - an array of sockets                                      */
/*         excepts - an array of sockets                                      */
/*         timeout - timeout in seconds                                       */
/*----------------------------------------------------------------------------*/

::method select
expose errno
use strict arg reads, writes, excepts, timeout

.validate~classType('reads', reads, .Array)
.validate~classType('writes', writes, .Array)
.validate~classType('excepts', excepts, .Array)
reads.0 = reads~items
do i = 1 to reads~items
   reads.i = reads[i]~string
   end
writes.0 = writes~items
do i = 1 to writes~items
   writes.i = writes[i]~string
   end
excepts.0 = excepts~items
do i = 1 to excepts~items
   excepts.i = excepts[i]~string
   end
retc = SockSelect('reads.', 'writes.', 'excepts.', timeout)
if retc = -1 then
   return .nil
reads~empty()
do i = 1 to reads.0
   reads[i] = reads.i
   end
writes~empty()
do i = 1 to writes.0
   writes[i] = writes.i
   end
excepts~empty()
do i = 1 to excepts.0
   excepts[i] = excepts.i
   end
return retc

/*----------------------------------------------------------------------------*/
/* Method: send                                                               */
/* Description: send data on a socket                                         */
/* Arguments:                                                                 */
/*         data - data to send over the socket                                */
/*----------------------------------------------------------------------------*/

::method send
expose s errno
use strict arg data
retc = SockSend(s, data)
return retc

/*----------------------------------------------------------------------------*/
/* Method: setOption                                                          */
/* Description: set a socket option                                           */
/* Arguments:                                                                 */
/*         name - the name of the option to set                               */
/*         val  - the new value for the option                                */
/*----------------------------------------------------------------------------*/

::method setOption
expose s errno
use strict arg name, val
name = name~upper()
if .SocketOptions~optionNames~hasItem(name) = .false then ,
 raise syntax 93.914 array (1, .SocketOptions~optionNames~makearray~toString, name)
if val~datatype('W') <> 1 & name <> 'SO_LINGER' then raise syntax 93.907 array(2, val)
retc = SockSetSockOpt(s, 'SOL_SOCKET', name, val)
if retc = -1 then
   return .nil
return retc

/*----------------------------------------------------------------------------*/
/* Method: string                                                             */
/* Description: returns a string representing the socket.                     */
/*----------------------------------------------------------------------------*/

::method string
expose s
return s

/*----------------------------------------------------------------------------*/
/* Method: uninit                                                             */
/* Description: close the socket.                                             */
/*----------------------------------------------------------------------------*/

::method uninit
expose s
if s <> -1 then retc = self~close()
return

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: InetAddress - internet address encapsulation.                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class InetAddress public

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: InetAddress                                                         */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute address get
::attribute family  get
::attribute port    get

/*----------------------------------------------------------------------------*/
/* Method: address=                                                           */
/* Description: the InetAddress address                                       */
/* Arguments:                                                                 */
/*         value - the ip address or host name                                */
/*----------------------------------------------------------------------------*/

::method 'address='
expose address
use strict arg address
if address~upper() <> 'INADDR_ANY' then do
   retc = SockGetHostByName(address, 'hostinfo.!')
   if retc = 1 then address = hostinfo.!addr
   if address~verify('0123456789.') <> 0 then ,
    raise syntax 93.953 array(1, 'dotted decimal internet address')
   end
return

/*----------------------------------------------------------------------------*/
/* Method: family=                                                            */
/* Description: the InetAddress family                                        */
/* Arguments:                                                                 */
/*         value - the address family                                         */
/*----------------------------------------------------------------------------*/

::method 'family='
expose family
use strict arg family
family = family~upper()
if .SocketOptions~families~hasItem(family) = .false then ,
 raise syntax 93.914 array ("family", .SocketOptions~families~makearray~toString, family)
return

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: initialization of the InetAddress                             */
/* Arguments:                                                                 */
/*         address - the ip address or hostname                               */
/*         port    - the port to be used                                      */
/*         family  - (optional) the address family to use                     */
/*----------------------------------------------------------------------------*/

::method init
use strict arg a, p, f = 'AF_INET'
-- we use the self methods to get error checking for address, port and family
self~address = a
self~port = p
self~family = f
return

/*----------------------------------------------------------------------------*/
/* Method: makeStem                                                           */
/* Description: make a stem out of the InetAddress                            */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method makeStem
expose address family port
use strict arg
stem.!addr = address
stem.!family = family
stem.!port = port
return stem.

/*----------------------------------------------------------------------------*/
/* Method: port=                                                              */
/* Description: the InetAddress port                                          */
/* Arguments:                                                                 */
/*         value - the port number                                            */
/*----------------------------------------------------------------------------*/

::method 'port='
expose port
use strict arg port
.Validate~wholeNumberRange('port', port, 0, 65535)
return


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: HostInfo - encapsulate host information.                            */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class HostInfo public

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: HostInfo                                                            */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: getInfo                                                            */
/* Description: get the hostinfo                                              */
/* Arguments:                                                                 */
/*         hostname - (optional, required if self~name has no value)          */
/*----------------------------------------------------------------------------*/

::method getInfo private
expose address alias addr name
-- initialize our attributes
alias~empty()
address = ''
addr~empty()
-- make sure we have a hostname to use
if self~name = '' | arg() > 0 then use strict arg self~name
-- get the information and set our attributes
if name~verify('0123456789.') = 0 then ,
 retc = SockGetHostByAddr(name, 'hostinfo.!')
else retc = SockGetHostByName(name, 'hostinfo.!')
if retc = -1 then return
address = hostinfo.!addr
-- sometimes no alias or addr info is returned, test for that
if hostinfo.!alias.0~datatype('W') = 1 then do i = 1 to hostinfo.!alias.0
   alias[i] = hostinfo.!alias.i
   end
if hostinfo.!addr.0~datatype('W') = 1 then do i = 1 to hostinfo.!addr.0
   addr[i] = hostinfo.!addr.i
   end
return

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: HostInfo                                                            */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute address get  -- the main address of the host
::attribute name    get  -- the standard host name
::attribute addr    get  -- an array of internet addresses of the host
::attribute alias   get  -- an array of alias names of the host

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: initialization of the HostInfo                                */
/* Arguments:                                                                 */
/*         address - the ip address or hostname                               */
/*----------------------------------------------------------------------------*/

::method init
expose address alias addr name
name = ''
alias = .array~new()
address = ''
addr = .array~new()
use strict arg name
self~getInfo()
return

/*----------------------------------------------------------------------------*/
/* Method: makeStem                                                           */
/* Description: make a stem out of the HostInfo                               */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method makeStem
expose address alias addr name
use strict arg
stem.!name = name
stem.!addr = address
stem.!addrtype = 'AF_INET'
stem.!addr.0 = addr~items()
do i = 1 to addr~items()
   stem.!addr.i = addr[i]
   end
stem.!alias.0 = alias~items()
do i = 1 to alias~items()
   stem.!alias.i = alias[i]
   end
return stem.

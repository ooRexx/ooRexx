/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Classes to encalsulate Ipv4 and IPv6 sockets.                 */
/*                                                                            */
/* Copyright (c) 2014-2018 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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


::requires "rxsock6" LIBRARY

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Socket - a class for sockets                                        */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

-- a class for socket operations
::class 'Socket' public

-- protocol family constants
::constant PF_UNSPEC
::constant PF_LOCAL
::constant PF_UNIX
::constant PF_FILE
::constant PF_INET
::constant PF_IMPLINK
::constant PF_AX25
::constant PF_PUP
::constant PF_CHAOS
::constant PF_APPLETALK
::constant PF_NETROM
::constant PF_IPX
::constant PF_NS
::constant PF_ISO
::constant PF_OSI
::constant PF_BRIDGE
::constant PF_ECMA
::constant PF_ATMPVC
::constant PF_DATAKIT
::constant PF_X25
::constant PF_CCITT
::constant PF_INET6
::constant PF_SNA
::constant PF_ROSE
::constant PF_DECnet
::constant PF_DLI
::constant PF_NETBEUI
::constant PF_SECURITY
::constant PF_KEY
::constant PF_NETLINK
::constant PF_ROUTE
::constant PF_PACKET
::constant PF_ASH
::constant PF_ECONET
::constant PF_ATMSVC
::constant PF_RDS
::constant PF_IRDA
::constant PF_PPPOX
::constant PF_WANPIPE
::constant PF_LLC
::constant PF_CAN
::constant PF_TIPC
::constant PF_BLUETOOTH
::constant PF_IUCV
::constant PF_RXRPC
::constant PF_ISDN
::constant PF_PHONET
::constant PF_IEEE802154
::constant PF_CAIF
::constant PF_ALG
::constant PF_NFC
::constant PF_VSOCK
::constant PF_KCM
::constant PF_QIPCRTR
::constant PF_SMP
::constant PF_MAX

-- socket type constants
::constant SOCK_DGRAM
::constant SOCK_STREAM
::constant SOCK_RAW
::constant SOCK_RDM
::constant SOCK_SEQPACKET
::constant SOCK_DCCP
::constant SOCK_PACKET
::constant SOCK_CLOEXEC
::constant SOCK_NONBLOCK

-- protocol constants
::constant IPPROTO_IP
::constant IPPROTO_HOPOPTS
::constant IPPROTO_ICMP
::constant IPPROTO_IGMP
::constant IPPROTO_IPIP
::constant IPPROTO_TCP
::constant IPPROTO_EGP
::constant IPPROTO_PUP
::constant IPPROTO_UDP
::constant IPPROTO_IDP
::constant IPPROTO_TP
::constant IPPROTO_DCCP
::constant IPPROTO_IPV6
::constant IPPROTO_ROUTING
::constant IPPROTO_FRAGMENT
::constant IPPROTO_RSVP
::constant IPPROTO_GRE
::constant IPPROTO_ESP
::constant IPPROTO_AH
::constant IPPROTO_ICMPV6
::constant IPPROTO_NONE
::constant IPPROTO_DSTOPTS
::constant IPPROTO_MTP
::constant IPPROTO_ENCAP
::constant IPPROTO_PIM
::constant IPPROTO_COMP
::constant IPPROTO_SCTP
::constant IPPROTO_UDPLITE
::constant IPPROTO_MPLS
::constant IPPROTO_RAW
::constant IPPROTO_MAX

-- setsoctopt option constants
::constant SOL_SOCTET
::constant SO_DEBUG
::constant SO_ACCEPTCONN
::constant SO_REUSEADDR
::constant SO_KEEPALIVE
::constant SO_DONTROUTE
::constant SO_BROADCAST
::constant SO_USELOOPBACK
::constant SO_LINGER
::constant SO_OOBINLINE

::constant SO_SNDBUF
::constant SO_RCVBUF
::constant SO_SNDLOWAT
::constant SO_RCVLOWAT
::constant SO_SNDTIMEO
::constant SO_RCVTIMEO
::constant SO_ERROR
::constant SO_TYPE
::constant SO_BSP_STATE

::constant SO_GROUP_ID
::constant SO_GROUP_PRIORITY
::constant SO_MAX_MSG_SIZE
::constant SO_PROTOCOL_INFOA
::constant SO_PROTOCOL_INFOW

::constant PVD_CONFIG
::constant SO_CONDITIONAL_ACCEPT

::constant SO_CONNDATA
::constant SO_CONNOPT
::constant SO_DISCDATA
::constant SO_DISCOPT
::constant SO_CONNDATALEN
::constant SO_CONNOPTLEN
::constant SO_DISCDATALEN
::constant SO_DISCOPTLEN

::constant SO_SNDBUFFORCE
::constant SO_RCVBUFFORCE
::constant SO_NO_CHECK
::constant SO_PRIORITY
::constant SO_BSDCOMPAT
::constant SO_REUSEPORT
::constant SO_PASSCRED
::constant SO_PEERCRED
::constant SO_SECURITY_AUTHENTICATION
::constant SO_SECURITY_ENCRYPTION_TRANSPORT
::constant SO_SECURITY_ENCRYPTION_NETWORK
::constant SO_BINDTODEVICE
::constant SO_ATTACH_FILTER
::constant SO_DETACH_FILTER
::constant SO_GET_FILTER
::constant SO_PEERNAME
::constant SO_TIMESTAMP
::constant SCM_TIMESTAMP
::constant SO_PEERSEC
::constant SO_PASSSEC
::constant SO_TIMESTAMPNS
::constant SCM_TIMESTAMPNS
::constant SO_MARK
::constant SO_TIMESTAMPING
::constant SCM_TIMESTAMPING
::constant SO_PROTOCOL
::constant SO_DOMAIN
::constant SO_RXQ_OVFL
::constant SO_WIFI_STATUS
::constant SCM_WIFI_STATUS
::constant SO_PEEK_OFF
::constant SO_NOFCS
::constant SO_LOCK_FILTER
::constant SO_SELECT_ERR_QUEUE
::constant SO_BUSY_POLL


-- public attributes
::attribute socketfd GET               -- socket file descriptor
::attribute address  GET               -- the bound address
::attribute retc     GET               -- last socket API return code
::attribute errno    GET               -- last socket API errno

-- private access only
::attribute socketfd SET               -- set a new socket descriptor

/*----------------------------------------------------------------------------*/
/* Methods                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/* Arguments:                                                                 */
/*         domain   - socket domain, like PF_INET6                            */
/*         type     - socket type, like SOCK_STREAM                           */
/*         protocol - socket protocol, usually zero                           */
/*----------------------------------------------------------------------------*/

::method init
  expose socketfd retc errno address

  socketfd = -1
  retc = 0
  errno = 0
  address = .nil

  use strict arg domain = (self~PF_INET), type = (self~SOCK_STREAM), protocol = (self~IPPROTO_IP)
  socketfd = self~private_init(domain, type, protocol)


/*----------------------------------------------------------------------------*/
/* Method: private_init                                                       */
/* Description: call the private init method.                                 */
/* Arguments:                                                                 */
/*         domain   - socket domain, like PF_INET6                            */
/*         type     - socket type, like SOCK_STREAM                           */
/*         protocol - socket protocol, usually zero                           */
/*----------------------------------------------------------------------------*/

::method private_init private external "LIBRARY rxsock6 socket_init"

/*----------------------------------------------------------------------------*/
/* Method: accept                                                             */
/* Description: accept a connection                                           */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance uninitialized (optional)              */
/*----------------------------------------------------------------------------*/

::method accept unguarded external "LIBRARY rxsock6 socket_accept"

/*----------------------------------------------------------------------------*/
/* Method: bind                                                              */
/* Description: bind a socket to an address.                                  */
/* Arguments:                                                                 */
/*----------------------------------------------------------------------------*/

::method bind external "LIBRARY rxsock6 socket_bind"

/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: shutdown and close a socket                                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method close external "LIBRARY rxsock6 socket_close"

/*----------------------------------------------------------------------------*/
/* Method: connect                                                            */
/* Description: connect a socket to a remote address                          */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance initialized with the address          */
/*----------------------------------------------------------------------------*/

::method connect external "LIBRARY rxsock6 socket_connect"

/*----------------------------------------------------------------------------*/
/* Method: getPeer                                                            */
/* Description: get the peer name connected to a socket                       */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

::method getpeer external "LIBRARY rxsock6 socket_getPeer"

/*----------------------------------------------------------------------------*/
/* Method: getSockName                                                        */
/* Description: get the socketpeer name of the socket.                       */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

::method getsockname external "LIBRARY rxsock6 socket_getSockName"

/*----------------------------------------------------------------------------*/
/* Method: getsockopt                                                         */
/* Description: get a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*----------------------------------------------------------------------------*/

::method getsockopt external "LIBRARY rxsock6 socket_getSockOpt"

/*----------------------------------------------------------------------------*/
/* Method: listen                                                             */
/* Description: listen for connections on a socket                            */
/* Arguments:                                                                 */
/*         backlog - the backlog to use for pending connection requests       */
/*----------------------------------------------------------------------------*/

::method listen external "LIBRARY rxsock6 socket_listen"

/*----------------------------------------------------------------------------*/
/* Method: recv                                                               */
/* Description: recieve data on a socket                                      */
/* Arguments:                                                                 */
/*         len - the maximum amount of data to recieve in bytes               */
/*----------------------------------------------------------------------------*/

::method recv external "LIBRARY rxsock6 socket_receive"

/*----------------------------------------------------------------------------*/
/* Method: recvFrom                                                           */
/* Description: recieve data on a socket from a specified address             */
/* Arguments:                                                                 */
/*         len         - the maximum amount of data to recieve in bytes       */
/*         inetaddress - initialized InetAddress instance                     */
/*----------------------------------------------------------------------------*/

::method recvfrom external "LIBRARY rxsock6 socket_receiveFrom"

/*----------------------------------------------------------------------------*/
/* Method: select                                                             */
/* Description: find out if file operations are available.                    */
/* Arguments:                                                                 */
/*         maxfd    - max file descriptor + 1                                 */
/*         readfds  - an array of read file descriptors to check              */
/*         writefds - an array of read file descriptors to check              */
/*         excptfds - an array of exception file descriptors to check         */
/*         timeout  - timeout in milliseconds                                 */
/*----------------------------------------------------------------------------*/

::method select external "LIBRARY rxsock6 socket_select"

/*----------------------------------------------------------------------------*/
/* Method: send                                                               */
/* Description: write a block of text to the socket.                          */
/* Arguments:                                                                 */
/*         text - the bytes to be written to the socket                       */
/*         flags - (optional) flags, default is zero                          */
/*----------------------------------------------------------------------------*/

::method send external "LIBRARY rxsock6 socket_send"

/*----------------------------------------------------------------------------*/
/* Method: sentTo                                                             */
/* Description: send data on a socket to specified address                    */
/* Arguments:                                                                 */
/*         data        - the data to be sent                                  */
/*         inetaddress - initialized InetAddress instance                     */
/*----------------------------------------------------------------------------*/

::method sendto external "LIBRARY rxsock6 socket_sendTo"

/*----------------------------------------------------------------------------*/
/* Method: setOption                                                          */
/* Description: set a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*         val   - value for the option                                       */
/*----------------------------------------------------------------------------*/

::method setOption external "LIBRARY rxsock6 socket_setSockOpt"

/*----------------------------------------------------------------------------*/
/* Method: string                                                             */
/* Description: returns a string representing the socket.                     */
/*----------------------------------------------------------------------------*/

::method string
expose socketfd
return socketfd

/*----------------------------------------------------------------------------*/
/* Method: uninit                                                             */
/* Description: close the socket.                                             */
/*----------------------------------------------------------------------------*/

::method uninit
expose socketfd
if socketfd <> -1 then retc = self~close()

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: INetAddress - internet address encapsulation.                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class "InetAddress" public

-- class methods
::method getHost class external "LIBRARY rxsock6 inetaddress_getHost"
::method getHostName class external "LIBRARY rxsock6 inetaddress_getHostName"
::method getAddrinfo class external "LIBRARY rxsock6 inetaddress_getAddrInfo"
::method validateAddrType private external "LIBRARY rxsock6 inetaddress_validateAddrType"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

-- address family constants
::constant AF_UNSPEC
::constant AF_LOCAL
::constant AF_UNIX
::constant AF_FILE
::constant AF_INET
::constant AF_AX25
::constant AF_IPX
::constant AF_APPLETALK
::constant AF_NETROM
::constant AF_BRIDGE
::constant AF_ATMPVC
::constant AF_X25
::constant AF_INET6
::constant AF_ROSE
::constant AF_DECnet
::constant AF_NETBEUI
::constant AF_SECURITY
::constant AF_KEY
::constant AF_NETLINK
::constant AF_ROUTE
::constant AF_PACKET
::constant AF_ASH
::constant AF_ECONET
::constant AF_ATMSVC
::constant AF_RDS
::constant AF_SNA
::constant AF_IRDA
::constant AF_PPPOX
::constant AF_WANPIPE
::constant AF_LLC
::constant AF_CAN
::constant AF_TIPC
::constant AF_BLUETOOTH
::constant AF_IUCV
::constant AF_RXRPC
::constant AF_ISDN
::constant AF_PHONET
::constant AF_IEEE802154
::constant AF_CAIF
::constant AF_ALG
::constant AF_NFC
::constant AF_VSOCK
::constant AF_MAX

-- misc constants
::constant INADDR_ANY
::constant AI_PASSIVE
::constant AI_CANONNAME
::constant AI_NUMERICHOST
::constant AI_V4MAPPED
::constant AI_ALL
::constant AI_ADDRCONFIG

/*----------------------------------------------------------------------------*/
/* Attributes                                                                 */
/*----------------------------------------------------------------------------*/

::attribute address
::attribute family  get
::attribute port    get

-- a private attribute for setting the socket descriptor
::attribute socketfd set private

/*----------------------------------------------------------------------------*/
/* Method: family=                                                            */
/* Description: the InetAddress family                                        */
/* Arguments:                                                                 */
/*         value - the address family                                         */
/*----------------------------------------------------------------------------*/

-- set the family name with validation
::attribute family SET
  expose family
  use strict arg family

  if \self~validateFamily(family) then do
     raise syntax 93.974 array ("family", family)
  end

-- set the port number for the address
::attribute port SET
  expose port
  use strict arg port

  .Validate~nonNegativeWholeNumber("port", port)


/*----------------------------------------------------------------------------*/
/* Methods                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: initialization of the InetAddress                             */
/* Arguments:                                                                 */
/*         address - the ip address or hostname                               */
/*         port    - the port to be used                                      */
/*         family  - the address family to use, like AF_INET6                 */
/*----------------------------------------------------------------------------*/

::method init
  expose address port family
  use strict arg address, port = 0, family = (self~AF_INET)

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Linger - linger socket option.                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class Linger public

/*----------------------------------------------------------------------------*/
/* Attributes                                                                 */
/*----------------------------------------------------------------------------*/

::attribute l_onoff
::attribute l_linger


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: AddrInfo - Address info struct.                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class AddrInfo public
/*----------------------------------------------------------------------------*/
/* Attributes                                                                 */
/*----------------------------------------------------------------------------*/

-- values for the flags
::constant AI_PASSIVE '00000001'x
::constant AI_CANONNAME '00000002'x
::constant AI_NUMERICHOST '00000004'x
::constant AI_NUMERICSERV '00000008'x
::constant AI_ALL '00000100'x
::constant AI_ADDRCONFIG '00000400'x
::constant AI_V4MAPPED '00000800'x

::attribute flags
::attribute family
::attribute socktype
::attribute protocol
::attribute canonname
::attribute address
::attribute addressFamily
::attribute addressPort
::attribute addresses
::attribute errno

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: initialization of the AddrInfo                                */
/* Arguments:                                                                 */
/*----------------------------------------------------------------------------*/

::method init
  use local
  use strict arg

  flags = '00000000'x
  family = .InetAddress~AF_UNSPEC
  socktype = .Socket~SOCK_STREAM
  -- "unspecified"
  protocol = 0
  canonname = ''
  family = .Socket~AF_UNSPECIFIED
  address = ''
  addressPort = 0
  -- no address list
  addresses = .nil
  errno = 0

-- set one of the addrInfo flags
::method setFlag
  expose flags
  use strict arg flag

  flags = flags~bitor(flag)

-- clear one of the flag values
::method clearFlag
  expose flags
  use strict arg flag

  flags = flags~bitand(flag~bitxor('ffffffff'x))

-- test if a given flag is set
::method testFlags
  expose flags
  use strict arg flag

  return flags~bitand(flag) == flag

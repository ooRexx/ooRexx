/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Classes to encalsulate Ipv4 and IPv6 sockets.                 */
/*                                                                            */
/* Copyright (c) 2014-2014 Rexx Language Association. All rights reserved.    */
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
/* Author: W. David Ashley                                                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/


::requires "orxsock6" LIBRARY


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: Socket - a class for sockets                                        */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class 'Socket' public

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

-- protocol family constants
::constant PF_UNSPEC             0
::constant PF_LOCAL              1
::constant PF_UNIX               1
::constant PF_FILE               1
::constant PF_INET               2
::constant PF_AX25               3
::constant PF_IPX                4
::constant PF_APPLETALK          5
::constant PF_NETROM             6
::constant PF_BRIDGE             7
::constant PF_ATMPVC             8
::constant PF_X25                9
::constant PF_INET6              10
::constant PF_ROSE               11
::constant PF_DECnet             12
::constant PF_NETBEUI            13
::constant PF_SECURITY           14
::constant PF_KEY                15
::constant PF_NETLINK            16
::constant PF_ROUTE              16
::constant PF_PACKET             17
::constant PF_ASH                18
::constant PF_ECONET             19
::constant PF_ATMSVC             20
::constant PF_RDS                21
::constant PF_SNA                22
::constant PF_IRDA               23
::constant PF_PPPOX              24
::constant PF_WANPIPE            25
::constant PF_LLC                26
::constant PF_CAN                29
::constant PF_TIPC               30
::constant PF_BLUETOOTH          31
::constant PF_IUCV               32
::constant PF_RXRPC              33
::constant PF_ISDN               34
::constant PF_PHONET             35
::constant PF_IEEE802154         36
::constant PF_CAIF               37
::constant PF_ALG                38
::constant PF_NFC                39
::constant PF_VSOCK              40
::constant PF_MAX                41

-- socket type constants
::constant SOCK_STREAM           1
::constant SOCK_DGRAM            2
::constant SOCK_RAW              3
::constant SOCK_RDM              4
::constant SOCK_SEQPACKET        5
::constant SOCK_DCCP             6
::constant SOCK_PACKET           10
::constant SOCK_CLOEXEC          02000000
::constant SOCK_NONBLOCK         00004000

-- protocol constants
::constant IPPROTO_IP            0
::constant IPPROTO_HOPOPTS       0
::constant IPPROTO_ICMP          1
::constant IPPROTO_IGMP          2
::constant IPPROTO_IPIP          4
::constant IPPROTO_TCP           6
::constant IPPROTO_EGP           8
::constant IPPROTO_PUP           12
::constant IPPROTO_UDP           17
::constant IPPROTO_IDP           22
::constant IPPROTO_TP            29
::constant IPPROTO_DCCP          33
::constant IPPROTO_IPV6          41
::constant IPPROTO_ROUTING       43
::constant IPPROTO_FRAGMENT      44
::constant IPPROTO_RSVP          46
::constant IPPROTO_GRE           47
::constant IPPROTO_ESP           50
::constant IPPROTO_AH            51
::constant IPPROTO_ICMPV6        58
::constant IPPROTO_NONE          59
::constant IPPROTO_DSTOPTS       60
::constant IPPROTO_MTP           92
::constant IPPROTO_ENCAP         98
::constant IPPROTO_PIM           103
::constant IPPROTO_COMP          108
::constant IPPROTO_SCTP          132
::constant IPPROTO_UDPLITE       136
::constant IPPROTO_RAW           255

-- setsoctopt option constants
::constant SOL_SOCTET            1
::constant SO_DEBUG              1
::constant SO_REUSEADDR          2
::constant SO_TYPE               3
::constant SO_ERROR              4
::constant SO_DONTROUTE          5
::constant SO_BROADCAST          6
::constant SO_SNDBUF             7
::constant SO_RCVBUF             8
::constant SO_SNDBUFFORCE        32
::constant SO_RCVBUFFORCE        33
::constant SO_KEEPALIVE          9
::constant SO_OOBINLINE          10
::constant SO_NO_CHECK           11
::constant SO_PRIORITY           12
::constant SO_LINGER             13
::constant SO_BSDCOMPAT          14
::constant SO_REUSEPORT          15
::constant SO_PASSCRED           16
::constant SO_PEERCRED           17
::constant SO_RCVLOWAT           18
::constant SO_SNDLOWAT           19
::constant SO_RCVTIMEO           20
::constant SO_SNDTIMEO           21
::constant SO_SECURITY_AUTHENTICATION       22
::constant SO_SECURITY_ENCRYPTION_TRANSPORT 23
::constant SO_SECURITY_ENCRYPTION_NETWORK   24
::constant SO_BINDTODEVICE       25
::constant SO_ATTACH_FILTER      26
::constant SO_DETACH_FILTER      27
::constant SO_GET_FILTER         26
::constant SO_PEERNAME           28
::constant SO_TIMESTAMP          29
::constant SCM_TIMESTAMP         29
::constant SO_ACCEPTCONN         30
::constant SO_PEERSEC            31
::constant SO_PASSSEC            34
::constant SO_TIMESTAMPNS        35
::constant SCM_TIMESTAMPNS       35
::constant SO_MARK               36
::constant SO_TIMESTAMPING       37
::constant SCM_TIMESTAMPING      37
::constant SO_PROTOCOL           38
::constant SO_DOMAIN             39
::constant SO_RXQ_OVFL           40
::constant SO_WIFI_STATUS        41
::constant SCM_WIFI_STATUS       41
::constant SO_PEEK_OFF           42
::constant SO_NOFCS              43
::constant SO_LOCK_FILTER        44
::constant SO_SELECT_ERR_QUEUE   45
::constant SO_BUSY_POLL          46



/*----------------------------------------------------------------------------*/
/* Attributes                                                                 */
/*----------------------------------------------------------------------------*/

::attribute socketfd                   -- socket file descriptor
::attribute retc                       -- last socket API return code
::attribute errno                      -- last socket API errno

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
expose socketfd
if arg() = 1 then do
   use strict arg socketfd
   end
else if arg() = 3 then do
   use strict arg domain, type, protocol
   socketfd = self~priviate_init(domain, type, protocol)
   end
else do
   raise syntax 88.900 array('Invalid number of arguments found.')
   end
return

/*----------------------------------------------------------------------------*/
/* Method: private_init                                                       */
/* Description: call the priviate init method.                                */
/* Arguments:                                                                 */
/*         domain   - socket domain, like PF_INET6                            */
/*         type     - socket type, like SOCK_STREAM                           */
/*         protocol - socket protocol, usually zero                           */
/*----------------------------------------------------------------------------*/

::method priviate_init private external "LIBRARY orxsock6 orxSocket6"

/*----------------------------------------------------------------------------*/
/* Method: accept                                                             */
/* Description: accept a connection                                           */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance uninitialized (optional)              */
/*----------------------------------------------------------------------------*/

::method accept unguarded external "LIBRARY orxsock6 orxAccept6"

/*----------------------------------------------------------------------------*/
/* Method: bind                                                              */
/* Description: bind a socket to an address.                                  */
/* Arguments:                                                                 */
/*----------------------------------------------------------------------------*/

::method bind external "LIBRARY orxsock6 orxBind6"

/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: shutdown and close a socket                                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method close external "LIBRARY orxsock6 orxClose6"

/*----------------------------------------------------------------------------*/
/* Method: connect                                                            */
/* Description: connect a socket to a remote address                          */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance initialized with the address          */
/*----------------------------------------------------------------------------*/

::method connect external "LIBRARY orxsock6 orxConnect6"

/*----------------------------------------------------------------------------*/
/* Method: convert_protocol_family                                            */
/* Description: convert a number to a protocol family.                        */
/* Arguments:                                                                 */
/*         fam - protocol number                                              */
/*----------------------------------------------------------------------------*/

::method convert_protocol_family class
use strict arg fam
if fam~datatype('W') <> 1 then return fam
select
   when fam = 0 then return 'PF_UNSPEC'
   when fam = 1 then return 'PF_LOCAL'
   when fam = 2 then return 'PF_INET'
   when fam = 3 then return 'PF_AX25'
   when fam = 4 then return 'PF_IPX'
   when fam = 5 then return 'PF_APPLETALK'
   when fam = 6 then return 'PF_NETROM'
   when fam = 7 then return 'PF_BRIDGE'
   when fam = 8 then return 'PF_ATMPVC'
   when fam = 9 then return 'PF_X25'
   when fam = 10 then return 'PF_INET6'
   when fam = 11 then return 'PF_ROSE'
   when fam = 12 then return 'PF_DECnet'
   when fam = 13 then return 'PF_NETBEUI'
   when fam = 14 then return 'PF_SECURITY'
   when fam = 15 then return 'PF_KEY'
   when fam = 16 then return 'PF_NETLINK'
   when fam = 17 then return 'PF_PACKET'
   when fam = 18 then return 'PF_ASH'
   when fam = 19 then return 'PF_ECONET'
   when fam = 20 then return 'PF_ATMSVC'
   when fam = 21 then return 'PF_RDS'
   when fam = 22 then return 'PF_SNA'
   when fam = 23 then return 'PF_IRDA'
   when fam = 24 then return 'PF_PPPOX'
   when fam = 25 then return 'PF_WANPIPE'
   when fam = 26 then return 'PF_LLC'
   when fam = 27 then return 'PF_CAN'
   when fam = 30 then return 'PF_TIPC'
   when fam = 31 then return 'PF_BLUETOOTH'
   when fam = 32 then return 'PF_IUCV'
   when fam = 33 then return 'PF_RXRPC'
   when fam = 34 then return 'PF_ISDN'
   when fam = 35 then return 'PF_PHONET'
   when fam = 36 then return 'PF_IEEE802154'
   when fam = 37 then return 'PF_CAIF'
   when fam = 38 then return 'PF_ALG'
   when fam = 39 then return 'PF_NFC'
   when fam = 40 then return 'PF_VSOCK'
   otherwise nop
   end
return fam

/*----------------------------------------------------------------------------*/
/* Method: convert_err_number                                                 */
/* Description: convert an error number to a short string.                    */
/* Arguments:                                                                 */
/*         error_number - errno number                                        */
/*----------------------------------------------------------------------------*/

::method convert_err_number
use strict arg err
if err~datatype('W') <> 1 then return err
select
   when err = 1 then return 'EPERM'            /* Operation not permitted */
   when err = 2 then return 'ENOENT'           /* No such file or directory */
   when err = 3 then return 'ESRCH'            /* No such process */
   when err = 4 then return 'EINTR'            /* Interrupted system call */
   when err = 5 then return 'EIO'              /* I/O error */
   when err = 6 then return 'ENXIO'            /* No such device or address */
   when err = 7 then return 'E2BIG'            /* Argument list too long */
   when err = 8 then return 'ENOEXEC'          /* Exec format error */
   when err = 9 then return 'EBADF'            /* Bad file number */
   when err = 10 then return 'ECHILD'          /* No child processes */
   when err = 11 then return 'EAGAIN'          /* Try again */
   when err = 12 then return 'ENOMEM'          /* Out of memory */
   when err = 13 then return 'EACCES'          /* Permission denied */
   when err = 14 then return 'EFAULT'          /* Bad address */
   when err = 15 then return 'ENOTBLK'         /* Block device required */
   when err = 16 then return 'EBUSY'           /* Device or resource busy */
   when err = 17 then return 'EEXIST'          /* File exists */
   when err = 18 then return 'EXDEV'           /* Cross-device link */
   when err = 19 then return 'ENODEV'          /* No such device */
   when err = 20 then return 'ENOTDIR'         /* Not a directory */
   when err = 21 then return 'EISDIR'          /* Is a directory */
   when err = 22 then return 'EINVAL'          /* Invalid argument */
   when err = 23 then return 'ENFILE'          /* File table overflow */
   when err = 34 then return 'EMFILE'          /* Too many open files */
   when err = 25 then return 'ENOTTY'          /* Not a typewriter */
   when err = 26 then return 'ETXTBSY'         /* Text file busy */
   when err = 27 then return 'EFBIG'           /* File too large */
   when err = 28 then return 'ENOSPC'          /* No space left on device */
   when err = 29 then return 'ESPIPE'          /* Illegal seek */
   when err = 30 then return 'EROFS'           /* Read-only file system */
   when err = 31 then return 'EMLINK'          /* Too many links */
   when err = 32 then return 'EPIPE'           /* Broken pipe */
   when err = 33 then return 'EDOM'            /* Math argument out of domain of func */
   when err = 34 then return 'ERANGE'          /* Math result not representable */
   when err = 35 then return 'EDEADLK'         /* Resource deadlock would occur */
   when err = 36 then return 'ENAMETOOLONG'    /* File name too long */
   when err = 37 then return 'ENOLCK'          /* No record locks available */
   when err = 38 then return 'ENOSYS'          /* Function not implemented */
   when err = 39 then return 'ENOTEMPTY'       /* Directory not empty */
   when err = 40 then return 'ELOOP'           /* Too many symbolic links encountered */
   when err = 42 then return 'ENOMSG'          /* No message of desired type */
   when err = 43 then return 'EIDRM'           /* Identifier removed */
   when err = 44 then return 'ECHRNG'          /* Channel number out of range */
   when err = 45 then return 'EL2NSYNC'        /* Level 2 not synchronized */
   when err = 46 then return 'EL3HLT'          /* Level 3 halted */
   when err = 47 then return 'EL3RST'          /* Level 3 reset */
   when err = 48 then return 'ELNRNG'          /* Link number out of range */
   when err = 49 then return 'EUNATCH'         /* Protocol driver not attached */
   when err = 50 then return 'ENOCSI'          /* No CSI structure available */
   when err = 51 then return 'EL2HLT'          /* Level 2 halted */
   when err = 52 then return 'EBADE'           /* Invalid exchange */
   when err = 53 then return 'EBADR'           /* Invalid request descriptor */
   when err = 54 then return 'EXFULL'          /* Exchange full */
   when err = 55 then return 'ENOANO'          /* No anode */
   when err = 56 then return 'EBADRQC'         /* Invalid request code */
   when err = 57 then return 'EBADSLT'         /* Invalid slot */
   when err = 59 then return 'EBFONT'          /* Bad font file format */
   when err = 60 then return 'ENOSTR'          /* Device not a stream */
   when err = 61 then return 'ENODATA'         /* No data available */
   when err = 62 then return 'ETIME'           /* Timer expired */
   when err = 63 then return 'ENOSR'           /* Out of streams resources */
   when err = 64 then return 'ENONET'          /* Machine is not on the network */
   when err = 65 then return 'ENOPKG'          /* Package not installed */
   when err = 66 then return 'EREMOTE'         /* Object is remote */
   when err = 67 then return 'ENOLINK'         /* Link has been severed */
   when err = 68 then return 'EADV'            /* Advertise error */
   when err = 69 then return 'ESRMNT'          /* Srmount error */
   when err = 70 then return 'ECOMM'           /* Communication error on send */
   when err = 71 then return 'EPROTO'          /* Protocol error */
   when err = 72 then return 'EMULTIHOP'       /* Multihop attempted */
   when err = 73 then return 'EDOTDOT'         /* RFS specific error */
   when err = 74 then return 'EBADMSG'         /* Not a data message */
   when err = 75 then return 'EOVERFLOW'       /* Value too large for defined data type */
   when err = 76 then return 'ENOTUNIQ'        /* Name not unique on network */
   when err = 77 then return 'EBADFD'          /* File descriptor in bad state */
   when err = 78 then return 'EREMCHG'         /* Remote address changed */
   when err = 79 then return 'ELIBACC'         /* Can not access a needed shared library */
   when err = 80 then return 'ELIBBAD'         /* Accessing a corrupted shared library */
   when err = 81 then return 'ELIBSCN'         /*.lib section in a.out corrupted */
   when err = 82 then return 'ELIBMAX'         /* Attempting to link in too many shared libraries */
   when err = 83 then return 'ELIBEXEC'        /* Cannot exec a shared library directly */
   when err = 84 then return 'EILSEQ'          /* Illegal byte sequence */
   when err = 85 then return 'ERESTART'        /* Interrupted system call should be restarted */
   when err = 86 then return 'ESTRPIPE'        /* Streams pipe error */
   when err = 87 then return 'EUSERS'          /* Too many users */
   when err = 88 then return 'ENOTSOCK'        /* Socket operation on non-socket */
   when err = 89 then return 'EDESTADDRREQ'    /* Destination address required */
   when err = 90 then return 'EMSGSIZE'        /* Message too long */
   when err = 91 then return 'EPROTOTYPE'      /* Protocol wrong type for socket */
   when err = 92 then return 'ENOPROTOOPT'     /* Protocol not available */
   when err = 93 then return 'EPROTONOSUPPORT' /* Protocol not supported */
   when err = 94 then return 'ESOCKTNOSUPPORT' /* Socket type not supported */
   when err = 95 then return 'EOPNOTSUPP'      /* Operation not supported on transport endpoint */
   when err = 96 then return 'EPFNOSUPPORT'    /* Protocol family not supported */
   when err = 97 then return 'EAFNOSUPPORT'    /* Address family not supported by protocol */
   when err = 98 then return 'EADDRINUSE'      /* Address already in use */
   when err = 99 then return 'EADDRNOTAVAIL'   /* Cannot assign requested address */
   when err = 100 then return 'ENETDOWN'       /* Network is down */
   when err = 101 then return 'ENETUNREACH'    /* Network is unreachable */
   when err = 102 then return 'ENETRESET'      /* Network dropped connection because of reset */
   when err = 103 then return 'ECONNABORTED'   /* Software caused connection abort */
   when err = 104 then return 'ECONNRESET'     /* Connection reset by peer */
   when err = 105 then return 'ENOBUFS'        /* No buffer space available */
   when err = 106 then return 'EISCONN'        /* Transport endpoint is already connected */
   when err = 107 then return 'ENOTCONN'       /* Transport endpoint is not connected */
   when err = 108 then return 'ESHUTDOWN'      /* Cannot send after transport endpoint shutdown */
   when err = 109 then return 'ETOOMANYREFS'   /* Too many references: cannot splice */
   when err = 110 then return 'ETIMEDOUT'      /* Connection timed out */
   when err = 111 then return 'ECONNREFUSED'   /* Connection refused */
   when err = 112 then return 'EHOSTDOWN'      /* Host is down */
   when err = 113 then return 'EHOSTUNREACH'   /* No route to host */
   when err = 114 then return 'EALREADY'       /* Operation already in progress */
   when err = 115 then return 'EINPROGRESS'    /* Operation now in progress */
   when err = 116 then return 'ESTALE'         /* Stale NFS file handle */
   when err = 117 then return 'EUCLEAN'        /* Structure needs cleaning */
   when err = 118 then return 'ENOTNAM'        /* Not a XENIX named type file */
   when err = 119 then return 'ENAVAIL'        /* No XENIX semaphores available */
   when err = 120 then return 'EISNAM'         /* Is a named type file */
   when err = 121 then return 'EREMOTEIO'      /* Remote I/O error */
   when err = 122 then return 'EDQUOT'         /* Quota exceeded */
   when err = 123 then return 'ENOMEDIUM'      /* No medium found */
   when err = 124 then return 'EMEDIUMTYPE'    /* Wrong medium type */
   when err = 125 then return 'ECANCELED'      /* Operation Canceled */
   when err = 126 then return 'ENOKEY'         /* Required key not available */
   when err = 127 then return 'EKEYEXPIRED'    /* Key has expired */
   when err = 128 then return 'EKEYREVOKED'    /* Key has been revoked */
   when err = 129 then return 'EKEYREJECTED'   /* Key was rejected by service */
   when err = 130 then return 'EOWNERDEAD'     /* Owner died */
   otherwise nop
   end
return err

/*----------------------------------------------------------------------------*/
/* Method: gai_strerror                                                       */
/* Description: get the error text associated with an error code from         */
/*              getaddrinfo method.                                           */
/* Arguments:                                                                 */
/*         errcode - error code                                               */
/*----------------------------------------------------------------------------*/

::method gai_strerror class external "LIBRARY orxsock6 orxGetStrerror6"

/*----------------------------------------------------------------------------*/
/* Method: getAddrInfo                                                        */
/* Description: get the address info for a host.                              */
/* Arguments:                                                                 */
/*         nodename - the host name or ip address                             */
/*         servname - the service name or number                              */
/*         hints    - an Inetaddr for the search hints                        */
/*         rea      - Rexx array variable (empty)                             */
/*----------------------------------------------------------------------------*/

::method getaddrinfo class external "LIBRARY orxsock6 orxGetAddrinfo6"

/*----------------------------------------------------------------------------*/
/* Method: getHostName                                                        */
/* Description: get the host name of the local machine.                       */
/* Arguments:                                                                 */
/*----------------------------------------------------------------------------*/

::method gethostname class external "LIBRARY orxsock6 orxGetHostName6"

/*----------------------------------------------------------------------------*/
/* Method: getPeerName                                                        */
/* Description: get the peer name connected to a socket                       */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

::method getpeername external "LIBRARY orxsock6 orxGetPeerName6"

/*----------------------------------------------------------------------------*/
/* Method: getProtoByName                                                     */
/* Description: get the protocol by its name.                                 */
/* Arguments:                                                                 */
/*         proto - protocol name                                              */
/*----------------------------------------------------------------------------*/

::method getprotocolbyname class external "LIBRARY orxsock6 orxGetProtocolByName6"

/*----------------------------------------------------------------------------*/
/* Method: getProtoByNumber                                                   */
/* Description: get the protocol by its number.                               */
/* Arguments:                                                                 */
/*         proto - protocol number                                            */
/*----------------------------------------------------------------------------*/

::method getprotocolbynumber class external "LIBRARY orxsock6 orxGetProtocolByNumber6"

/*----------------------------------------------------------------------------*/
/* Method: getSockName                                                        */
/* Description: get the socketpeer name of the socket.                       */
/* Arguments:                                                                 */
/*         inetaddr - Inetaddr instance                                       */
/*----------------------------------------------------------------------------*/

::method getsockname external "LIBRARY orxsock6 orxGetSockName6"

/*----------------------------------------------------------------------------*/
/* Method: getsockopt                                                         */
/* Description: get a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*----------------------------------------------------------------------------*/

::method getsockopt external "LIBRARY orxsock6 orxGetSockOpt6"

/*----------------------------------------------------------------------------*/
/* Method: listen                                                             */
/* Description: listen for connections on a socket                            */
/* Arguments:                                                                 */
/*         backlog - the backlog to use for pending connection requests       */
/*----------------------------------------------------------------------------*/

::method listen external "LIBRARY orxsock6 orxListen6"

/*----------------------------------------------------------------------------*/
/* Method: recv                                                               */
/* Description: recieve data on a socket                                      */
/* Arguments:                                                                 */
/*         len - the maximum amount of data to recieve in bytes               */
/*----------------------------------------------------------------------------*/

::method recv external "LIBRARY orxsock6 orxRecv6"

/*----------------------------------------------------------------------------*/
/* Method: recvFrom                                                           */
/* Description: recieve data on a socket from a specified address             */
/* Arguments:                                                                 */
/*         len         - the maximum amount of data to recieve in bytes       */
/*         inetaddress - initialized InetAddress instance                     */
/*----------------------------------------------------------------------------*/

::method recvfrom external "LIBRARY orxsock6 orxRecvFrom6"

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

::method select external "LIBRARY orxsock6 orxSelect6"

/*----------------------------------------------------------------------------*/
/* Method: send                                                               */
/* Description: write a block of text to the socket.                          */
/* Arguments:                                                                 */
/*         text - the bytes to be written to the socket                       */
/*         flags - (optional) flags, default is zero                          */
/*----------------------------------------------------------------------------*/

::method send external "LIBRARY orxsock6 orxSend6"

/*----------------------------------------------------------------------------*/
/* Method: sentTo                                                             */
/* Description: send data on a socket to specified address                    */
/* Arguments:                                                                 */
/*         data        - the data to be sent                                  */
/*         inetaddress - initialized InetAddress instance                     */
/*----------------------------------------------------------------------------*/

::method sendto external "LIBRARY orxsock6 orxSendTo6"

/*----------------------------------------------------------------------------*/
/* Method: setsockopt                                                         */
/* Description: set a socket option.                                          */
/* Arguments:                                                                 */
/*         option - socket option                                             */
/*         val   - value for the option                                       */
/*----------------------------------------------------------------------------*/

::method setsockopt external "LIBRARY orxsock6 orxSetSockOpt6"

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
return


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: INetAddress - internet address encapsulation.                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class InetAddress public

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

-- address family constants
::constant AF_UNSPEC             0
::constant AF_LOCAL              1
::constant AF_UNIX               1
::constant AF_FILE               1
::constant AF_INET               2
::constant AF_AX25               3
::constant AF_IPX                4
::constant AF_APPLETALK          5
::constant AF_NETROM             6
::constant AF_BRIDGE             7
::constant AF_ATMPVC             8
::constant AF_X25                9
::constant AF_INET6              10
::constant AF_ROSE               11
::constant AF_DECnet             12
::constant AF_NETBEUI            13
::constant AF_SECURITY           14
::constant AF_KEY                15
::constant AF_NETLINK            16
::constant AF_ROUTE              16
::constant AF_PACKET             17
::constant AF_ASH                18
::constant AF_ECONET             19
::constant AF_ATMSVC             20
::constant AF_RDS                21
::constant AF_SNA                22
::constant AF_IRDA               23
::constant AF_PPPOX              24
::constant AF_WANPIPE            25
::constant AF_LLC                26
::constant AF_CAN                29
::constant AF_TIPC               30
::constant AF_BLUETOOTH          31
::constant AF_IUCV               32
::constant AF_RXRPC              33
::constant AF_ISDN               34
::constant AF_PHONET             35
::constant AF_IEEE802154         36
::constant AF_CAIF               37
::constant AF_ALG                38
::constant AF_NFC                39
::constant AF_VSOCK              40
::constant AF_MAX                41

-- misc constants
::constant INADDR_ANY            0
::constant AI_PASSIVE            1
::constant AI_CANONNAME          2
::constant AI_NUMERICHOST        4
::constant AI_V4MAPPED           8
::constant AI_ALL                16
::constant AI_ADDRCONFIG         32

/*----------------------------------------------------------------------------*/
/* Attributes                                                                 */
/*----------------------------------------------------------------------------*/

::attribute address
::attribute family 
::attribute port   

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
if arg() = 3 then do
   use strict arg address, port, family
   end
else use strict arg
return

/*----------------------------------------------------------------------------*/
/* Method: fromstruct                                                         */
/* Description: return InetAddress from the supplied pointer                  */
/* Arguments:                                                                 */
/*         ptr - pointer to a sockaddr struct                                 */
/*----------------------------------------------------------------------------*/

::method fromstruct
use strict arg ptr
addr = .InetAddress~new(.InetAddress~AF_INET, 0, .InetAddress~INADDR_ANY)
call orxAddrFromStruct6 ptr, addr
return addr

/*----------------------------------------------------------------------------*/
/* Method: convert_address_family                                             */
/* Description: convert a number to an address family.                        */
/* Arguments:                                                                 */
/*         fam - family number                                                */
/*----------------------------------------------------------------------------*/

::method convert_address_family class
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
   when fam = 21 then return 'AF_RDS'
   when fam = 22 then return 'AF_SNA'
   when fam = 23 then return 'AF_IRDA'
   when fam = 24 then return 'AF_PPPOX'
   when fam = 25 then return 'AF_WANPIPE'
   when fam = 26 then return 'AF_LLC'
   when fam = 27 then return 'AF_CAN'
   when fam = 30 then return 'AF_TIPC'
   when fam = 31 then return 'AF_BLUETOOTH'
   when fam = 32 then return 'AF_IUCV'
   when fam = 33 then return 'AF_RXRPC'
   when fam = 34 then return 'AF_ISDN'
   when fam = 35 then return 'AF_PHONET'
   when fam = 36 then return 'AF_IEEE802154'
   when fam = 37 then return 'AF_CAIF'
   when fam = 38 then return 'AF_ALG'
   when fam = 39 then return 'AF_NFC'
   when fam = 40 then return 'AF_VSOCK'
   otherwise nop
   end
return fam



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

::attribute ai_flags
::attribute ai_family
::attribute ai_socktype
::attribute ai_protocol
::attribute ai_canonname
::attribute ai_addr
::attribute sa_family            -- part of the sockaddr struct
::attribute sa_addr              -- part of the sockaddr struct

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: initialization of the AddrInfo                                */
/* Arguments:                                                                 */
/*----------------------------------------------------------------------------*/

::method init
use strict arg
self~ai_flags = 0
self~ai_family = 0
self~ai_socktype = 0
self~ai_protocol = 0
self~ai_canonname = ''
self~sa_family = 0
self~sa_addr = ''
return



/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Simple classes to encalsulate stream sockets.                 */
/*                                                                            */
/* Copyright (c) 2007-2009 Rexx Language Association. All rights reserved.    */
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

::requires "rxsock" LIBRARY    -- need the rxsock library available to function

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
/*----------------------------------------------------------------------------*/
/* Class: Socket                                                              */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: convert_err_number                                                 */
/* Description: convert an error number to a short string.                    */
/*----------------------------------------------------------------------------*/

::method convert_err_number private
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
if newsocket = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
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
if \address~isA(.InetAddress) then ,
 raise syntax 93.914 array (1, 'an InetAddress class instance', address)
stem. = address~makeStem()
retc = SockBind(s, 'stem.!')
-- It should be noted here that the SockBind() function can give a false
-- indication of success when in fact it did not bind to anything.
if retc = -1 then errno = self~convert_err_number(SockSock_errno())
else errno = ''
return retc

/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: shutdown and close a socket                                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method close
expose s errno
use strict arg
retc = SockClose(s)
if retc = -1 then errno = self~convert_err_number(SockSock_errno())
else errno = ''
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
if \address~isA(.InetAddress) then ,
 raise syntax 93.914 array (1, 'an InetAddress class instance', address)
stem. = address~makeStem()
retc = SockConnect(s, 'stem.!')
if retc = -1 then errno = self~convert_err_number(SockSock_errno())
else errno = ''
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
optvals = .array~of('SO_BROADCAST', 'SO_DEBUG', 'SO_DONTROUTE', 'SO_ERROR',,
                    'SO_KEEPALIVE', 'SO_LINGER', 'SO_OOBINLINE', 'SO_RCVBUF',,
                    'SO_RCVLOWAT', 'SO_RCVTIMEO', 'SO_REUSEADDR', 'SO_SNDBUF',,
                    'SO_SNDLOWAT', 'SO_SNDTIMEO', 'SO_TYPE', 'SO_USELOOPBACK')
if optvals~hasItem(name) = .false then ,
 raise syntax 93.914 array (1, optvals~toString, name)
-- get the value
retc = SockGetSendOpt(s, 'SOL_SOCKET', name, 'xxx')
-- some values may not be gettable
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
return xxx

/*----------------------------------------------------------------------------*/
/* Method: getPeerName                                                        */
/* Description: get the peer name connected to a socket                       */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method getPeerName
expose s errno
use strict arg
retc = SockGetPeerName(s, 'stem.!')
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
return .InetAddress~new(stem.!addr, stem.!port, self~convert_address_family(stem.!family))

/*----------------------------------------------------------------------------*/
/* Method: getSockName                                                        */
/* Description: get the name of the socket                                    */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method getSockName
expose s errno
use strict arg
retc = SockGetSockName(s, cmd, 'stem.!')
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
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
   if s~datatype('W') = 0 then raise syntax 93.905 array (1, s)
   if s < 1 then raise syntax 93.907 array (1, s)
   return
   end
-- assume we want a new socket created
use strict arg domain = 'AF_INET', type = 'SOCK_STREAM', protocol = 0
domain = domain~upper()
type= type~upper()
protocol = protocol~upper()
-- check arguments
domains = .array~of('AF_INET')
if domains~hasItem(domain) = .false then ,
 raise syntax 93.914 array (1, domains~toString, domain)
types = .array~of('SOCK_STREAM', 'SOCK_DGRAM', 'SOCK_RAW')
if types~hasItem(type) = .false then ,
 raise syntax 93.914 array (2, types~toString, type)
protocols = .array~of(0, 'IPPROTO_UDP', 'IPPROTO_TCP')
if protocols~hasItem(protocol) = .false then ,
 raise syntax 93.914 array (3, protocols~toString, protocol)
-- create the socket
s = SockSocket(domain, type, protocol)
if s = -1 then errno = self~convert_err_number(SockSock_errno())
else errno = ''
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
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
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
if backlog~datatype('W') = 0 then raise syntax 93.905 array (1, backlog)
if backlog < 1 then raise syntax 93.907 array (1, backlog)
retc = SockListen(s, backlog)
if retc = -1 then errno = self~convert_err_number(SockSock_errno())
else errno = ''
return retc

/*----------------------------------------------------------------------------*/
/* Method: recv                                                               */
/* Description: recieve data on a socket                                      */
/* Arguments:                                                                 */
/*         len - the maximum amount of data to recieve in bytes               */
/*----------------------------------------------------------------------------*/

::method recv
expose s errno
use strict arg len
if len~datatype('W') = 0 then raise syntax 93.905 array (1, len)
if len < 1 then raise syntax 93.907 array (1, len)
retc = SockRecv(s, 'xxx', len)
if retc = -1 then do
   errno = socksock_errno()
   return .nil
   end
errno = ''
if retc = 0 then return ''
return xxx

/*----------------------------------------------------------------------------*/
/* Method: recvFrom                                                           */
/* Description: recieve data on a socket from a specified address             */
/* Arguments:                                                                 */
/*         len      - the maximum amount of data to recieve in bytes          */
/*         addressr - a host address of class InetAddress                     */
/*----------------------------------------------------------------------------*/

::method recvFrom
expose s errno
use strict arg len, address
if len~datatype('W') = 0 then raise syntax 93.905 array (1, len)
if len < 1 then raise syntax 93.907 array (1, len)
if \address~isA(.InetAddress) then ,
 raise syntax 93.914 array (1, 'an InetAddress', address)
stem. = address~makeStem()
retc = SockRecvFrom(s, 'xxx', len, 'stem.')
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
if retc = 0 & xxx~length > 0 then do
   errno = self~convert_err_number(SockSock_errno())
   if xxx~length > 0 then return xxx
   else return ''
   end
else errno = ''
return xxx

/*----------------------------------------------------------------------------*/
/* Method: select                                                             */
/* Description: monitor activity on a set of sockets                          */
/* Arguments:                                                                 */
/*         reads   - an array of sockets                                       */
/*         writes  - an array of sockets                                           */
/*         excepts - an array of sockets                                      */
/*         timeout - timeout in seconds                                       */
/*----------------------------------------------------------------------------*/

::method select
expose errno
use strict arg reads, writes, excepts, timeout
if \reads~isInstanceOf(.Array) then ,
 raise syntax 93.914 array (1, 'an Array', reads)
if \writes~isInstanceOf(.Array) then ,
 raise syntax 93.914 array (2, 'an Array', writes)
if \excepts~isInstanceOf(.Array) then ,
 raise syntax 93.914 array (3, 'an Array', excepts)
reads.0 = reads~items
do i = 1 to reads~items
   reads.i = reads[i]~s
   end
writes.0 = writes~items
do i = 1 to writes~items
   writes.i = writes[i]~s
   end
excepts.0 = excepts~items
do i = 1 to excepts~items
   excepts.i = excepts[i]~s
   end
retc = SockSelect('reads.', 'writes.', 'excepts.', timeout)
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
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
if retc = -1 then errno = self~convert_err_number(SockSock_errno())
else errno = ''
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
optvals = .array~of('SO_BROADCAST', 'SO_DEBUG', 'SO_DONTROUTE', 'SO_ERROR',,
                    'SO_KEEPALIVE', 'SO_LINGER', 'SO_OOBINLINE', 'SO_RCVBUF',,
                    'SO_RCVLOWAT', 'SO_RCVTIMEO', 'SO_REUSEADDR', 'SO_SNDBUF',,
                    'SO_SNDLOWAT', 'SO_SNDTIMEO', 'SO_TYPE', 'SO_USELOOPBACK')
if optvals~hasItem(name) = .false then ,
 raise syntax 93.914 array (1, optvals~toString, name)
if val~datatype('W') <> 1 & name <> 'SO_LINGER' then raise syntax 93.907 array(2, val)
retc = SockSetSockOpt(s, 'SOL_SOCKET', name, val)
if retc = -1 then do
   errno = self~convert_err_number(SockSock_errno())
   return .nil
   end
else errno = ''
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
/*        Class methods                                                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: InetAddress                                                         */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

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
families = .array~of('AF_INET')
if families~hasItem(family) = .false then ,
 raise syntax 93.914 array (1, families~toString, family)
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
if port~datatype('W') <> 1 then raise syntax 93.907 array(1, port)
if port < 1 then raise syntax 93.907 array(1, port)
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
/*        Class methods                                                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

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
/* Description: initialization of the InetAddress                             */
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


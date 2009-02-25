/*-- REXX --------------------------------------------------------------------*/
/*                                                                            */
/* Description: FTP client class (to replace rxftp.dll)                       */
/*                                                                            */
/* Notes:       The public methods in this class resemble the functions       */
/*              which were available in the old rxftp.dll function library.   */
/*              But there are differences! In any case, it should be easy     */
/*              to port your code to use this class.                          */
/*                                                                            */
/*              The main difference between this class and the old functions  */
/*              is that this class uses an Object Rexx array to return        */
/*              information to the caller as opposed to the use of stems in   */
/*              the old functions.                                            */
/*                                                                            */
/*              Another difference is that each instance of the class you     */
/*              instaniate has its own connection to an FTP server. Thus      */
/*              you can instaniate multiple instance of this class and        */
/*              connect to multiple FTP servers simultaneously. Also, each    */
/*              instance can be reused - the FtpLogoff method reinitializes   */
/*              the instance and a following FtpSetUser method can establish  */
/*              a new connection to an FTP server.                            */
/*                                                                            */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* Authors;                                                                   */
/*       W. David Ashley <dashley@us.ibm.com>                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/

::requires "rxsock" LIBRARY    -- need the rxsock library available to function

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: RXFTP                                                               */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class 'rxftp' subclass object public


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: RXFTP                                                               */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::method actv_port  attribute private -- local host data port
::method ascii      attribute private -- .true = ASCII mode, .false = BINARY
::method csock      attribute private -- command socket
::method data       attribute private -- unparsed received data
::method dsock      attribute private -- data socket
::method lhost      attribute private -- local host ip address
::method mode       attribute private -- .true = active mode, .false = passive
::method password   attribute private -- remote FTP password
::method pasv_port  attribute private -- remote FTP host data port
::method rcport     attribute private -- remote FTP host command port
::method rdone      attribute private -- send/receive done flag
::method resp       attribute private -- raw unparsed command response buffer
::method rhost      attribute private -- remote FTP host name/ip address
::method srvr_os    attribute private -- remote FTP host operating system
::method traceflg   attribute private -- .true = display ftp commands
::method tracelog   attribute private -- trace log file stream
::method user       attribute private -- remote FTP host user name
::method version    attribute private -- version of this class
::method thrdstatus attribute private -- finish status of a thread


/*----------------------------------------------------------------------------*/
/* Method: actvrecv                                                           */
/* Description: active receive on the data port                               */
/*----------------------------------------------------------------------------*/

::method actvrecv private unguarded

self~data = ''
if arg() = 1 then use arg flocal
else flocal = .nil

/* set the receive done flag to false */
self~rdone = .false
self~thrdstatus = ''

/* get a socket */
self~debugsay('Getting a socket.')
self~dsock = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
if dsock = -1 then do
   self~debugsay('Error getting socket.')
   self~debugsay('Errno =' errno)
   self~dsock = 0
   return -1
   end

/* bind it to our data port */
self~debugsay('Getting cmd socket info.')
retc = SockGetSockName(self~csock, 'addr.!')
addr.!port = self~actv_port
self~debugsay('Binding socket using port' addr.!port)
retc = SockBind(self~dsock, 'addr.!')
if retc <> 0 then do
   self~debugsay('Error: Unable to bind to port' addr.!port'.')
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end

/* listen and accept a connection */
retc = SockListen(self~dsock, 1)
if retc <> 0 then do
   self~debugsay('Error: Unable to listen on socket.')
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end

/* set up the local file if necessary */
if flocal <> .nil then do
   floc = .stream~new(flocal)
   retc = floc~open('write replace')
   if retc <> 'READY:' then do
      self~debugsay('Error opening local file.')
      call SockShutDown self~dsock, 2
      call SockClose self~dsock
      self~dsock = 0
      return -1
      end
   end
maxbuf = 16384

reply 0

self~debugsay('Accepting connection.')
ndsock = SockAccept(self~dsock)
if ndsock = -1 then do
   self~debugsay('Error: Unable to accept on socket.')
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   if flocal <> .nil then floc~close()
   self~rdone = .true
   return
   end

self~debugsay('Connected, recieving data.')
bytesrcvd = SockRecv(ndsock, 'resp', self~bufsize)
do while bytesrcvd > 0
   self~debugsay('SockRecv:' bytesrcvd 'bytes received.')
   self~data = self~data || resp
   if flocal <> .nil & self~data~length() >= maxbuf then do
      if self~ascii = .true then self~remove_cr()
      self~debugsay('Writing' self~data~length() 'bytes to file.')
      retc = floc~charout(self~data)
      if retc <> 0 then do
         self~thrdstatus = '550'
         bytesrcvd = 0
         iterate
         end
      self~data = ''
      end
   bytesrcvd = SockRecv(ndsock, 'resp', self~bufsize)
   end

call SockShutDown ndsock, 2
call SockClose ndsock
call SockShutDown self~dsock, 2
call SockClose self~dsock
self~dsock = 0

if flocal <> .nil then do
   if self~ascii = .true then self~remove_cr()
   retc = floc~charout(self~data)
   if retc <> 0 then do
      self~thrdstatus = '550'
      end
   floc~close()
   end

self~debugsay('All data received using active receive.')
self~rdone = .true
return


/*----------------------------------------------------------------------------*/
/* Method: actvsend                                                           */
/* Description: active send on the data port                                  */
/*----------------------------------------------------------------------------*/

::method actvsend private unguarded

self~data = ''
use arg flocal

/* set the receive done flag to false */
self~rdone = .false
self~thrdstatus = ''

/* get a socket */
self~debugsay('Getting a socket.')
self~dsock = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
if self~dsock = -1 then do
   self~debugsay('Error getting socket.')
   self~debugsay('Errno =' errno)
   return -1
   end

/* bind it to our data port */
self~debugsay('Getting cmd socket info.')
retc = SockGetSockName(self~csock, 'addr.!')
addr.!port = self~actv_port
self~debugsay('Binding socket using port' addr.!port)
retc = SockBind(self~dsock, 'addr.!')
if retc <> 0 then do
   self~debugsay('Error: Unable to bind to port' addr.!port'.')
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end

/* listen and accept a connection */
retc = SockListen(self~dsock, 1)
if retc <> 0 then do
   self~debugsay('Error: Unable to listen on socket.')
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end

/* set up the local file */
floc = .stream~new(flocal)
retc = floc~open('read')
if retc <> 'READY:' then do
   self~debugsay('Error opening local file.')
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end
maxbuf = 16384

reply 0

self~debugsay('Accepting connection.')
ndsock = SockAccept(self~dsock)
if ndsock = -1 then do
   self~debugsay('Error: Unable to accept on socket.')
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   if flocal <> .nil then floc~close()
   self~rdone = .true
   return
   end

/* send the file */
self~debugsay('Connected, sending data.')
if self~ascii = .true then do
   self~data = ''
   eof = .false
   call on notready name eof_floc
   asciiline = floc~linein()
   do while eof = .false & self~data~length() < maxbuf
      self~data = self~data || asciiline || '0D0A'x
      asciiline = floc~linein()
      end
   end
else self~data = floc~charin(, maxbuf)
do while self~data~length() > 0
   bytessent = SockSend(ndsock, self~data)
   if bytessent = -1 then do
      self~debugsay('Error during SockSend.')
      self~debugsay('Errno =' errno)
      call SockShutDown ndsock, 2
      call SockClose ndsock
      call SockShutDown self~dsock, 2
      call SockClose self~dsock
      self~dsock = 0
      floc~close()
      self~rdone = .true
      return
      end
   self~debugsay(bytessent 'bytes sent.')
   if eof = .true then leave
   if self~ascii = .true then do
      self~data = ''
      do while eof = .false & self~data~length() < maxbuf
         self~data = self~data || asciiline || '0D0A'x
         asciiline = floc~linein()
         end
      end
   else self~data = floc~charin(, maxbuf)
   end

call SockShutDown ndsock, 2
call SockClose ndsock
call SockShutDown self~dsock, 2
call SockClose self~dsock
self~dsock = 0

floc~charout(self~data)
floc~close()

self~debugsay('All data sent using active send.')
self~rdone = .true
return


eof_floc:
eof = .true
return


/*----------------------------------------------------------------------------*/
/* Method: chkipaddr                                                          */
/* Description: basic check for a valid ip dotted decimal address             */
/*----------------------------------------------------------------------------*/

::method chkipaddr private
use arg ipaddr
-- Note: this only works for ipv4 addresses, it needs to be updated for ipv6
parse var ipaddr p1 '.' p2 '.' p3 '.' p4
if p1~datatype('W') = 0 | p1 < 0 | p1 > 255 then return .true
if p2~datatype('W') = 0 | ps < 0 | p2 > 255 then return .true
if p3~datatype('W') = 0 | p3 < 0 | p3 > 255 then return .true
if p4~datatype('W') = 0 | p4 < 0 | p4 > 255 then return .true
return .false


/*----------------------------------------------------------------------------*/
/* Method: debugsay                                                           */
/* Description: say the message if debug = .true                              */
/*----------------------------------------------------------------------------*/

::method debugsay private unguarded
/* any value other than .false activates debug messages */
if self~debug <> .false then say arg(1)
return


/*----------------------------------------------------------------------------*/
/* Method: ftppasv                                                            */
/* Description: ask the server to provide a port for the data connecion       */
/*----------------------------------------------------------------------------*/

::method ftppasv private

self~response = .array~new()

/* perform transaction */
retc = self~transactsock('PASV', '227')
if retc = .true then do
   self~debugsay('Error: PASV command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end
self~mode = .false /* server now in passive mode */

/* get the host address and port */
temp = self~cmdresponse[self~cmdresponse~items]
self~debugsay(temp)
parse var temp . '(' h1 ',' h2 ',' h3 ',' h4 ',' p1 ',' p2 ')' .
if h1 = '' then
 parse value temp~subword(temp~words) with h1 ',' h2 ',' h3 ',' h4 ',' p1 ',' p2 '.'
self~rhost = h1'.'h2'.'h3'.'h4
self~pasv_port = (p1 * 256) + p2

self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: pasvrecv                                                           */
/* Description: passive receive on the data port                              */
/*----------------------------------------------------------------------------*/

::method pasvrecv private unguarded

self~data = ''
if arg() = 1 then use arg flocal
else flocal = .nil

/* set the receive done flag to false */
self~rdone = .false
self~thrdstatus = ''

/* get a socket */
self~debugsay('Getting a socket.')
self~dsock = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
if self~dsock = -1 then do
   self~debugsay('Error getting socket.')
   self~debugsay('Errno =' errno)
   return -1
   end

/* connect to the ftp server data port */
addr.!family = 'AF_INET'
addr.!port = self~pasv_port
addr.!addr = self~rhost
self~debugsay('Connecting to' self~rhost':'self~pasv_port'.')
retc = SockConnect(self~dsock, 'addr.!')
if retc <> 0 then do
   self~debugsay('Error: Unable to connect to' self~rhost':'self~pasv_port)
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end

/* set up the local file if necessary */
if flocal <> .nil then do
   floc = .stream~new(flocal)
   retc = floc~open('write replace')
   if retc <> 'READY:' then do
      self~debugsay('Error opening local file.')
      call SockShutDown self~dsock, 2
      call SockClose self~dsock
      self~dsock = 0
      return -1
      end
   self~debugsay('Opened local file.')
   end
maxbuf = 16384

reply 0

self~debugsay('Connected, recieving data.')
bytesrcvd = SockRecv(self~dsock, 'resp', self~bufsize)
do while bytesrcvd > 0
   self~debugsay('SockRecv:' bytesrcvd 'bytes received.')
   self~debugsay('Errno =' errno)
   self~data = self~data || resp
   if flocal <> .nil & self~data~length() >= maxbuf then do
      if self~ascii = .true then self~remove_cr()
      self~debugsay('Writing' self~data~length() 'bytes to file.')
      retc = floc~charout(self~data)
      if retc <> 0 then do
         self~thrdstatus = '550'
         bytesrcvd = 0
         iterate
         end
      self~data = ''
      end
   bytesrcvd = SockRecv(self~dsock, 'resp', self~bufsize)
   end

call SockShutDown self~dsock, 2
call SockClose self~dsock
self~dsock = 0

if flocal <> .nil then do
   self~debugsay('Writing' length(self~data) 'bytes to file.')
   if self~ascii = .true then self~remove_cr()
   retc = floc~charout(self~data)
   if retc <> 0 then do
      self~thrdstatus = '550'
      end
   self~debugsay('Closing file.')
   floc~close()
   end

self~debugsay('All data received using passive receive.')
self~rdone = .true
return


/*----------------------------------------------------------------------------*/
/* Method: pasvsend                                                           */
/* Description: passive send on the data port                                 */
/*----------------------------------------------------------------------------*/

::method pasvsend private unguarded

self~data = ''
use arg flocal

/* set the send done flag to false */
self~rdone = .false
self~thrdstatus = ''

/* get a socket */
self~debugsay('Getting a socket.')
self~dsock = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
if dsock = -1 then do
   self~debugsay('Error getting socket.')
   self~debugsay('Errno =' errno)
   self~dsock = 0
   return -1
   end

/* connect to the ftp server data port */
addr.!family = 'AF_INET'
addr.!port = self~pasv_port
addr.!addr = self~rhost
self~debugsay('Connecting to' self~rhost':'self~pasv_port'.')
retc = SockConnect(self~dsock, 'addr.!')
if retc <> 0 then do
   self~debugsay('Error: Unable to connect to' self~rhost':'self~pasv_port)
   self~debugsay('Errno =' errno)
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end

/* set up the local file */
floc = .stream~new(flocal)
retc = floc~open('read')
if retc <> 'READY:' then do
   self~debugsay('Error opening local file' flocal'.')
   call SockShutDown self~dsock, 2
   call SockClose self~dsock
   self~dsock = 0
   return -1
   end
self~debugsay('Opened file.')
maxbuf = 16384

reply 0

/* send the file */
self~debugsay('Connected, sending data.')
if self~ascii = .true then do
   self~data = ''
   eof = .false
   call on notready name eof_floc
   asciiline = floc~linein()
   do while eof = .false & self~data~length() < maxbuf
      self~data = self~data || asciiline || '0D0A'x
      asciiline = floc~linein()
      end
   end
else self~data = floc~charin(, maxbuf)
do while self~data~length() > 0
   bytessent = SockSend(self~dsock, self~data)
   if bytessent = -1 then do
      self~debugsay('Error during SockSend.')
      self~debugsay('Errno =' errno)
      call SockShutDown self~dsock, 2
      call SockClose self~dsock
      self~dsock = 0
      floc~close()
      self~rdone = .true
      return
      end
   self~debugsay(bytessent 'bytes sent.')
   if eof = .true then leave
   if self~ascii = .true then do
      self~data = ''
      do while eof = .false & self~data~length() < maxbuf
         self~data = self~data || asciiline || '0D0A'x
         asciiline = floc~linein()
         end
      end
   else self~data = floc~charin(, maxbuf)
   end

call SockShutDown self~dsock, 2
call SockClose self~dsock
self~dsock = 0

self~debugsay('Closing file.')
floc~close()

self~debugsay('All data sent using passive send.')
self~rdone = .true
return


eof_floc:
eof = .true
return


/*----------------------------------------------------------------------------*/
/* Method: putfile                                                            */
/* Description: put a single file to the server (append, put, putunique)      */
/*----------------------------------------------------------------------------*/

::method putfile private

/* check/get args */
if arg() = 3 then do
   use arg ftpcmd, flocal, fremote
   tmptype = self~ascii
   end
else do
   use arg ftpcmd, flocal, fremote, typestr
   if typestr~substr(1, 1) = 'A' then tmptype = .true
   else if typestr~substr(1, 1) = 'B' then tmptype = .false
   else raise syntax 93.914 array (3, '"ASCII", "BINARY"', typestr)
end

/* change transfer type if necessary */
if tmptype <> self~ascii then do
   if tmptype = .true then retc = self~FtpSetType('A')
   else retc = self~FtpSetType('B')
   if retc = -1 then do
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end

/* active or passive mode? */
if self~mode = .true then do
   /* active mode */
   retc = SockGetSockName(self~csock, 'addr.!')
   hostaddr = addr.!addr
   hostaddr = translate(hostaddr, ',', '.')
   /* TCP can hold a port after it has been closed so use the next available */
   if self~actv_port = 0 then self~actv_port = addr.!port + 1
   else self~actv_port = self~actv_port + 1
   p1 = self~actv_port
   p2 = p1 // 256
   p1 = trunc(p1 / 256, 0)
   url = hostaddr','p1','p2
   retc = self~transactsock('PORT' url, '200')
   self~debugsay(self~cmdresponse[self~cmdresponse~items])
   if retc = .true then do
      self~response = .array~new()
      self~debugsay('Error: port command to' self~rhost 'failed.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   if self~actvsend(flocal) = -1 then do
      self~response = .array~new()
      self~debugsay('Error creating listening socket.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end
else do
   /* passive mode */
   retc = self~ftppasv()
   if retc = -1 then do
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   if self~pasvsend(flocal) = -1 then do
      self~debugsay('Error creating connection socket.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end

/* perform transaction */
retc = self~transactsock(ftpcmd fremote, '150')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~debugsay('Error:' ftpcmd 'command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the transfer complete message */
self~debugsay('Waiting for file complete message.')
retc = self~recvresponse('226')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error:' mainftpcmd 'command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* wait for the send to be really done */
do while (self~rdone = .false)
   call SysSleep .1
   end

/* change transfer type back if necessary */
if tmptype <> self~ascii then do
   if self~ascii = .true then retc = self~FtpSetType('A')
   else retc = self~FtpSetType('B')
   /* we are just going to ignore any error from FtpSetType here */
   end

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: recvresponse                                                       */
/* Description: receive a cmd response from the ftp server                    */
/*----------------------------------------------------------------------------*/

::method recvresponse private
use arg exretc

/* socket open check */
if self~csock = 0 then do
   return .true
   end

/* fill the response buffer if necessary */
if self~resp = '' then do while self~resp~pos('0D0A'x) = 0
   bytesrcvd = SockRecv(self~csock, 'rcvdata', self~bufsize)
   self~resp = self~resp || rcvdata
   if bytesrcvd = -1 then do
      self~debugsay('SockRecv error: the error code was' Errno)
      return .true
      end
   self~debugsay('SockRecv:' bytesrcvd 'bytes received')
   end

/* get the first line of the response */
x = pos('0D0A'x, self~resp)
if x = 0 then do
   self~debugsay('SockRecv: invalid response')
   return .true
   end
self~cmdresponse[self~cmdresponse~items + 1] = self~resp~substr(1, x - 1)
self~resp = self~resp~substr(x + 2)
startresp = self~cmdresponse~items

/* if this is a multi-line response, get the rest of the lines */
if self~cmdresponse[self~cmdresponse~items]~substr(4, 1) = '-' then do
   lastline = self~cmdresponse[self~cmdresponse~items]~substr(1, 3) || ' '
   do forever
      /* split the buffer into lines */
      x = self~resp~pos('0D0A'x)
      do while x > 0
         self~cmdresponse[self~cmdresponse~items + 1] = self~resp~substr(1, x - 1)
         self~resp = self~resp~substr(x + 2)
         x = self~resp~pos('0D0A'x)
         end
      /* do we have the last line? */
      if self~cmdresponse[self~cmdresponse~items]~substr(1, 4) = lastline then leave
      /* get the next buffer of data */
      bytesrcvd = SockRecv(self~csock, 'rcvdata', self~bufsize)
      self~debugsay('SockRecv:' bytesrcvd 'bytes received')
      if bytesrcvd = 0 then leave
      if bytesrcvd < 0 then do
         self~debugsay('SockRecv error: the error code was' Errno)
         return .true
         end
      self~resp = self~resp || rcvdata
      end
   end

/* place response in trace file */
if self~tracelog <> .nil then do i = startresp to self~cmdresponse~items
   self~tracelog~lineout(self~cmdresponse[i])
   end

/* check the response & return .false if we find an expected return code */
self~ftperrno = self~cmdresponse[self~cmdresponse~items]~substr(1, 3)
if exretc = '*' then return .false
parse var exretc r1 exretc
do while r1 <> ''
   /* per the RFC 959 only check the first digit of the return code */
   if r1~substr(1, 1) = self~ftperrno~substr(1, 1) then return .false
   parse var exretc r1 exretc
   end

/* we found an unexpected return code */
do i = startresp to self~cmdresponse~items
   self~debugsay('SockRecv: unexpected response:' self~cmdresponse[i])
   end
return .true


/*----------------------------------------------------------------------------*/
/* Method: remove_cr                                                          */
/* Description: if necessary, remove CR bytes from the buffer                 */
/*----------------------------------------------------------------------------*/

::method remove_cr private unguarded

/* remove CR bytes? */
if self~cr_remove = .false then return

/* test to see if we need to remove the CR bytes */
parse source os .
if substr(os, 1, 7) = 'Windows' then os = 'Windows'
if os = 'Windows' | os = 'OS2' then return

/* remove CR bytes */
crpos = self~data~pos('0D'x)
do while crpos > 0
   self~data = self~data~delstr(crpos, 1)
   crpos = self~data~pos('0D'x, crpos)
   end
return


/*----------------------------------------------------------------------------*/
/* Method: sendcmd                                                            */
/* Description: send a command to the ftp server                              */
/*----------------------------------------------------------------------------*/

::method sendcmd private
use arg cmd

if substr(cmd, 1, 4) = 'PASS' then self~debugsay('sendcmd invoked, cmd = PASS XXXX')
else self~debugsay('sendcmd invoked, cmd =' cmd)

/* socket open check */
if self~csock = 0 then do
   return .true
   end

/* place cmd in trace file */
if self~tracelog <> .nil then do
   if cmd~substr(1, 4) = 'PASS' then self~tracelog~lineout('PASS XXXX')
   else self~tracelog~lineout(cmd)
   end

/* display cmd if tracing */
if self~traceflg = 1 then do
   if cmd~substr(1, 4) = 'PASS' then say 'PASS XXXX'
   else say cmd
   end

/* send the command */
self~cmdresponse[self~cmdresponse~items + 1] = cmd
bytessent = SockSend(self~csock, cmd || '0D0A'x)  /* add CRLF */
if bytessent > 0 then do
   self~debugsay('SockSend: cmd sent successfully')
   end
else do
   self~debugsay('SockSend error: the error code was' Errno)
   return .true
   end
return .false


/*----------------------------------------------------------------------------*/
/* Method: setdefaults                                                        */
/* Description: setup/reset all the defaults for the class                    */
/*----------------------------------------------------------------------------*/

::method setdefaults private
self~actv_port = 0               -- local active mode port
self~ascii = .true               -- .true = ascii transfer type, .false = binary
self~cr_remove = .true           -- .true = remove CR bytes from ASCII downloads
self~csock = 0                   -- command socket
self~dsock = 0                   -- data socket
self~cmdresponse = .array~new()  -- all command/status responses from the server
self~debug = .false              -- .false suppresses debugging information
self~ftperrno = ''               -- last ftp response code
self~lhost = ''                  -- local ip address
self~mode = .false               -- .true = active mode, .false = passive mode
self~password = ''               -- account password on the ftp server
self~pasv_port = ''              -- port on the passive host
self~rcport = 21                 -- default remote command port
self~rdone= .false               -- send/receive done flag
self~resp = ''                   -- raw unparsed command socket response buffer
self~response = .nil             -- parsed last command response from the server
self~rhost = ''                  -- host name or ip address of the ftp server
self~srvr_os = ''                -- remote host operating system
self~traceflg = 0                -- .true = display ftp commands
self~tracelog = .nil             -- trace log stream
self~user = ''                   -- user account on the ftp server
self~version = '4.0.0'           -- current version of this class
self~bufsize = 4096              -- default buffer size for send/receive
self~thrdstatus = ''
return


/*----------------------------------------------------------------------------*/
/* Method: transactsock                                                       */
/* Description: send a command and receive a response from the ftp server     */
/*----------------------------------------------------------------------------*/

::method transactsock private
use arg cmd, exretc

retc = self~sendcmd(cmd)
if retc = .true then return .true
return self~recvresponse(exretc)


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: RXFTP                                                               */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::method cmdresponse attribute -- all parsed commands and responses
::method cr_remove   attribute -- .true = remove CR bytes from ASCII downloads
::method debug       attribute -- .false suppresses debug messages
::method ftperrno    attribute -- last ftp response code
::method response    attribute -- parsed command response from last command
::attribute bufsize  get       -- bufsize for send/receive
::attribute bufsize  set
   expose bufsize
   use arg value
   if datatype(value, "Whole") = .false | value < 0 then
   raise syntax 93.906 array ("bufsize", value)
   bufsize=value


/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*----------------------------------------------------------------------------*/

::method init
self~setdefaults
return


/*----------------------------------------------------------------------------*/
/* Method: FtpAppend                                                          */
/* Description: append a single file to the server                            */
/*----------------------------------------------------------------------------*/

::method FtpAppend

/* check/get args */
if arg() > 3 then raise syntax 93.902 array (3)
if arg() < 2 then raise syntax 93.901 array (2)
if arg() = 2 then do
   use arg flocal, fremote
   retc = self~putfile('APPE', flocal, fremote)
   end
else do
   use arg flocal, fremote, typestr
   retc = self~putfile('APPE', flocal, fremote, typestr)
end
return retc


/*----------------------------------------------------------------------------*/
/* Method: FtpChDir                                                           */
/* Description: change to the specified directory on the ftp server           */
/*----------------------------------------------------------------------------*/

::method FtpChDir

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)

/* perform transaction */
retc = self~transactsock('CWD' arg(1), '2')
if retc = .true then do
   self~debugsay('Error: CWD command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpDelete                                                          */
/* Description: deletes the specified file on the ftp server.                 */
/*----------------------------------------------------------------------------*/

::method FtpDelete

self~response = .array~new()

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)
use arg pattern

/* perform transaction */
retc = self~transactsock('DELE' arg(1), '2')
if retc = .true then do
   self~debugsay('Error: DELE command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpDir                                                             */
/* Description: return a directory listing on the server                      */
/*----------------------------------------------------------------------------*/

::method FtpDir

/* what operating system is the remote server using? */
if self~srvr_os = '' then do
   retc = self~FtpSys()
   if retc = -1 then return -1
   end
select
   when self~srvr_os = 'WIN'  then cmd = 'LIST'
   when self~srvr_os = 'OS/2' then cmd = 'LIST'
   when self~srvr_os = 'MVS'  then cmd = 'LIST'
   when self~srvr_os = 'VM'   then cmd = 'LIST'
   when self~srvr_os = 'UNIX' then cmd = 'LIST -aL'
   otherwise cmd = 'LIST'
   end

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() = 1 then cmd = cmd arg(1)
else if wordpos(self~srvr_os, 'MVS VM') = 0 then cmd = cmd './*'

/* active or passive mode? */
if self~mode = .true then do
   /* active mode */
   retc = SockGetSockName(self~csock, 'addr.!')
   hostaddr = addr.!addr
   hostaddr = hostaddr~translate(',', '.')
   /* TCP can hold a port after it has been closed so use the next available */
   if self~actv_port = 0 then self~actv_port = addr.!port + 1
   else self~actv_port = self~actv_port + 1
   p1 = self~actv_port
   p2 = p1 // 256
   p1 = trunc(p1 / 256, 0)
   url = hostaddr','p1','p2
   retc = self~transactsock('PORT' url, '200')
   self~debugsay(self~cmdresponse[self~cmdresponse~items])
   if retc = .true then do
      self~response = .array~new()
      self~debugsay('Error: port command to' self~rhost 'failed.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   if self~actvrecv() = -1 then do
      self~response = .array~new()
      self~debugsay('Error creating listening socket.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end
else do
   /* passive mode */
   retc = self~ftppasv()
   if retc = .true then return retc
   if self~pasvrecv() = -1 then do
      self~response = .array~new()
      self~debugsay('Error connecting to the server data port.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end

/* perform transaction */
parse var cmd ftpcmd .
retc = self~transactsock(cmd, '150')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error:' ftpcmd 'command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the transfer complete message */
self~debugsay('Waiting for file complete message.')
retc = self~recvresponse('226')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error:' ftpcmd 'command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* wait for the receive to be really done */
do while (self~rdone = .false)
   call SysSleep .1
   end

/* split the response into lines */
self~response = .array~new()
self~debugsay('Parsing received data.')
if self~data~pos('0D0A'x) > 0 then sep = '0D0A'x
else sep = '0A'x
x = self~data~pos(sep)
do while x > 0
   self~response[self~response~items + 1] = self~data~substr(1, x - 1)
   self~data = self~data~substr(x + sep~length())
   x = self~data~pos(sep)
   end

self~ftperrno = ''
return .false


/*----------------------------------------------------------------------------*/
/* Method: FtpGet                                                             */
/* Description: get a single file from the server                             */
/*----------------------------------------------------------------------------*/

::method FtpGet

/* check/get args */
if arg() > 3 then raise syntax 93.902 array (3)
if arg() < 2 then raise syntax 93.901 array (2)
if arg() = 2 then do
   use arg flocal, fremote
   tmptype = self~ascii
   end
else do
   use arg flocal, fremote, typestr
   if typestr~substr(1, 1) = 'A' then tmptype = .true
   else if typestr~substr(1, 1) = 'B' then tmptype = .false
   else raise syntax 93.914 array (3, '"ASCII", "BINARY"', typestr)
end

self~thrdstatus = ''

/* change transfer type if necessary */
if tmptype <> self~ascii then do
   if tmptype = .true then retc = self~FtpSetType('A')
   else retc = self~FtpSetType('B')
   if retc = -1 then do
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end

/* socket open check */
if self~csock = 0 then do
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the data */
if self~mode = .true then do
   /* active mode */
   retc = SockGetSockName(self~csock, 'addr.!')
   hostaddr = addr.!addr
   hostaddr = hostaddr~translate(',', '.')
   /* TCP can hold a port after it has been closed so use the next available */
   if self~actv_port = 0 then self~actv_port = addr.!port + 1
   else self~actv_port = self~actv_port + 1
   p1 = self~actv_port
   p2 = p1 // 256
   p1 = trunc(p1 / 256, 0)
   url = hostaddr','p1','p2
   retc = self~transactsock('PORT' url, '200')
   self~debugsay(self~cmdresponse[self~cmdresponse~items])
   if retc = .true then do
      self~response = .array~new()
      self~debugsay('Error: port command to' self~rhost 'failed.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   if self~actvrecv(flocal) = -1 then do
      self~response = .array~new()
      self~debugsay('Error creating listening socket.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end
else do
   /* use passive mode */
   retc = self~ftppasv()
   if retc = -1 then do
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   if self~pasvrecv(flocal) = -1 then do
      self~debugsay('Error creating connection socket.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end

/* perform transaction */
retc = self~transactsock('RETR' fremote, '150')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error: RETR command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the transfer complete message */
self~debugsay('Waiting for file complete message.')
retc = self~recvresponse('226')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error: RETR command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* wait for the receive to be really done */
do while (self~rdone = .false)
   call SysSleep .1
   end

/* change transfer type back if necessary */
if tmptype <> self~ascii then do
   if self~ascii = .true then retc = self~FtpSetType('A')
   else retc = self~FtpSetType('B')
   /* we are just going to ignore any error from FtpSetType here */
   end

/* check to see if an error happened during the transfer */
if self~thrdstatus <> '' then do
   self~response = .array~new()
   self~debugsay('Error: RETR command to' self~rhost 'failed. Local disk may be full.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpGetMode                                                         */
/* Description: gets the current transfer mode for files (active or passive)  */
/*----------------------------------------------------------------------------*/

::method FtpGetMode

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

if self~mode = .true then return 'ACTIVE'
self~response = .array~new()
self~ftperrno = ''
return 'PASSIVE'


/*----------------------------------------------------------------------------*/
/* Method: FtpGetType                                                         */
/* Description: gets the current transfer type for files (ASCII or Binary)    */
/*----------------------------------------------------------------------------*/

::method FtpGetType

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

self~response = .array~new()
if self~ascii = .true then return 'ASCII'
return 'BINARY'


/*----------------------------------------------------------------------------*/
/* Method: FtpLogoff                                                          */
/* Description: logoff the ftp server                                         */
/*----------------------------------------------------------------------------*/

::method FtpLogoff

self~response = .array~new()

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

retc = self~transactsock('QUIT', '2')
if retc = .true then do
   self~debugsay('Error: QUIT command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   end
else retc = 0

/* be sure to stop tracing if it is active */
retc = self~FtpTraceLogOff

/* shutdown the socket */
call SockShutDown self~csock, 2
call SockClose self~csock

/* reset all the defaults */
self~setdefaults

return retc


/*----------------------------------------------------------------------------*/
/* Method: FtpLs                                                              */
/* Description: return a list of file names in a subdirectory on the server   */
/*----------------------------------------------------------------------------*/

::method FtpLs

/* what operating system is the remote server using? */
if self~srvr_os = '' then do
   retc = self~FtpSys()
   if retc = -1 then return -1
   end
cmd = 'NLST'

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() = 1 then cmd = cmd arg(1)
else select
   when self~srvr_os = 'MVS' then nop
   when self~srvr_os = 'VM'  then nop
   otherwise cmd = cmd './*'
   end

/* active or passive mode? */
if self~mode = .true then do
   /* active mode */
   retc = SockGetSockName(self~csock, 'addr.!')
   hostaddr = addr.!addr
   hostaddr = hostaddr~translate(',', '.')
   /* TCP can hold a port after it has been closed so use the next available */
   if self~actv_port = 0 then self~actv_port = addr.!port + 1
   else self~actv_port = self~actv_port + 1
   p1 = self~actv_port
   p2 = p1 // 256
   p1 = trunc(p1 / 256, 0)
   url = hostaddr','p1','p2
   retc = self~transactsock('PORT' url, '200')
   self~debugsay(self~cmdresponse[self~cmdresponse~items])
   if retc = .true then do
      self~response = .array~new()
      self~debugsay('Error: port command to' self~rhost 'failed.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   if self~actvrecv() = -1 then do
      self~response = .array~new()
      self~debugsay('Error creating listening socket.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end
else do
   /* passive mode */
   retc = self~ftppasv()
   if retc = .true then return retc
   if self~pasvrecv() = -1 then do
      self~response = .array~new()
      self~debugsay('Error connecting to the server data port.')
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   end

/* perform transaction */
parse var cmd ftpcmd .
retc = self~transactsock(cmd, '150')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error:' ftpcmd 'command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the transfer complete message */
self~debugsay('Waiting for file complete message.')
retc = self~recvresponse('226')
self~debugsay(self~cmdresponse[self~cmdresponse~items])
if retc = .true then do
   self~response = .array~new()
   self~debugsay('Error:' ftpcmd 'command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* wait for the receive to be really done */
do while (self~rdone = .false)
   call SysSleep .1
   end

/* split the response into lines */
self~response = .array~new()
self~debugsay('Parsing received data.')
if self~data~pos('0D0A'x) > 0 then sep = '0D0A'x
else sep = '0A'x
x = self~data~pos(sep)
do while x > 0
   self~response[self~response~items + 1] = self~data~substr(1, x - 1)
   self~data = self~data~substr(x + sep~length())
   x = self~data~pos(sep)
   end

self~ftperrno = ''
return .false


/*----------------------------------------------------------------------------*/
/* Method: FtpMkDir                                                           */
/* Description: make the specified directory on the ftp server                */
/*----------------------------------------------------------------------------*/

::method FtpMkDir

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)

/* perform transaction */
retc = self~transactsock('MKD' arg(1), '2')
if retc = .true then do
   self~debugsay('Error: MKD command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpPut                                                             */
/* Description: put a single file to the server                               */
/*----------------------------------------------------------------------------*/

::method FtpPut

/* check/get args */
if arg() > 3 then raise syntax 93.902 array (3)
if arg() < 2 then raise syntax 93.901 array (2)
if arg() = 2 then do
   use arg flocal, fremote
   retc = self~putfile('STOR', flocal, fremote)
   end
else do
   use arg flocal, fremote, typestr
   retc = self~putfile('STOR', flocal, fremote, typestr)
end
return retc


/*----------------------------------------------------------------------------*/
/* Method: FtpPutUnique                                                       */
/* Description: put a single file to the server with a unique name            */
/*----------------------------------------------------------------------------*/

::method FtpPutUnique

/* check/get args */
if arg() > 3 then raise syntax 93.902 array (3)
if arg() < 2 then raise syntax 93.901 array (2)
if arg() = 2 then do
   use arg flocal, fremote
   retc = self~putfile('STOU', flocal, fremote)
   end
else do
   use arg flocal, fremote, typestr
   retc = self~putfile('STOU', flocal, fremote, typestr)
end
return retc


/*----------------------------------------------------------------------------*/
/* Method: FtpPwd                                                             */
/* Description: return the current subdirectory on the ftp server             */
/*----------------------------------------------------------------------------*/

::method FtpPwd

self~response = .array~new()

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

/* perform transaction */
retc = self~transactsock('PWD', '2')
if retc = .true then do
   self~debugsay('Error: PWD command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the edited response */
temp = self~cmdresponse[self~cmdresponse~items]
if temp~pos('"') > 0 then parse var temp retc '"' temp '"'
else parse var temp retc temp
self~response[1] = temp

self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpQuote                                                           */
/* Description: send a simple command to the server                           */
/*----------------------------------------------------------------------------*/

::method FtpQuote

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)

/* perform transaction */
startresp = self~cmdresponse~items + 2
retc = self~transactsock(arg(1), '*')
if retc = .true then do
   self~debugsay('Error: Quote command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~response = .array~new()
i = 1
do j = startresp to self~cmdresponse~items
   self~response[i] = self~cmdresponse[j]
   i = i + 1
   end
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpRename                                                          */
/* Description: rename a file on the server.                                  */
/*----------------------------------------------------------------------------*/

::method FtpRename

self~response = .array~new()

/* check/get args */
if arg() > 2 then raise syntax 93.902 array (2)
if arg() < 2 then raise syntax 93.901 array (2)
use arg oldname, newname

/* perform transaction */
retc = self~transactsock('RNFR' oldname, '350')
if retc = .true then do
   self~debugsay('Error: RNFR command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end
retc = self~transactsock('RNTO' newname, '2')
if retc = .true then do
   self~debugsay('Error: RNTO command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpRmDir                                                           */
/* Description: remove the specified directory on the ftp server              */
/*----------------------------------------------------------------------------*/

::method FtpRmDir

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)

/* perform transaction */
retc = self~transactsock('RMD' arg(1), '2')
if retc = .true then do
   self~debugsay('Error: RMD command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpSetMode                                                         */
/* Description: change to the active/passive mode for file transfers          */
/*----------------------------------------------------------------------------*/

::method FtpSetMode

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)
use arg inmode

/* set the transfer mode */
mode = inmode~substr(1, 1)~translate()
if mode = 'A' then self~mode = .true
else if mode = 'P' then self~mode = .false
else raise syntax 93.914 array (1, '"ACTIVE", "PASSIVE"', inmode)

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpSetType                                                         */
/* Description: set the default transfer type for files (ASCII or Binary)     */
/*----------------------------------------------------------------------------*/

::method FtpSetType

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)
use arg intype

oldtype = self~ascii
type = intype~substr(1, 1)~translate()
if type = 'A' then self~ascii = .true
else if type = 'B' then self~ascii = .false
else raise syntax 93.914 array (1, '"ASCII", "BINARY"', intype)

/* perform transaction */
if self~ascii = .true then retc = self~transactsock('TYPE A', '2')
else retc = self~transactsock('TYPE I', '2')
if retc = .true then do
   self~debugsay('Error: TYPE command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   self~ascii = oldtype
   return -1
   end

self~response = .array~new()
self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpSetUser                                                         */
/* Description: set up the primary connection to the ftp server and logon     */
/*----------------------------------------------------------------------------*/

::method FtpSetUser

self~debugsay('FtpSetUser invoked')

self~response = .array~new()
self~ftperrno = ''

/* check the arguments */
if arg() > 4 then raise syntax 93.902 array (4)
if arg() < 1 then raise syntax 93.901 array (1)
select
   when arg() = 1 then do
      use arg thost
      tuser = 'anonymous'
      tpassword = 'xxx@nowhere.com'
      account = ''
      end
   when arg() = 2 then do
      use arg thost, tuser
      tpassword = 'xxx@nowhere.com'
      account = ''
      end
   when arg() = 3 then do
      use arg thost, tuser, tpassword
      account = ''
      end
   when arg() = 4 then do
      use arg thost, tuser, tpassword, account
      end
   otherwise nop
   end
if tuser = 'anonymous' then tpassword = 'xxx@nowhere.com'
self~rhost = thost
self~user = tuser
self~password = tpassword

/* socket open check */
if self~csock <> 0 then do
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* check to see if an alternate port was specified */
hostname = self~rhost
parse var hostname hostname ':' tport
if tport <> '' then do
   self~rhost = hostname
   if tport~datatype('W') = 0 then do
      self~ftperrno = 'FTPCOMMAND'
      return -1
      end
   self~rcport = tport
   end

/* convert the remote host name to an ip addr */
retc = SockGetHostByName(self~rhost, 'hostinfo.!')
if retc = 1 then do
   ipaddr = hostinfo.!addr
   end
else do
   ipaddr = self~rhost
   end
if self~chkipaddr(ipaddr) = .true then do
   self~debugsay('Error: Unable to resolve host name' ipaddr 'ro address.')
   self~ftperrno = 'FTPHOST'
   return -1
   end
if self~rhost <> ipaddr then self~debugsay(self~rhost '=' ipaddr)

/* now get a socket */
self~csock = SockSocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')
if self~csock = -1 then do
   self~ftperrno = 'FTPSOCKET'
   self~csock = 0
   return -1
   end

/* create the address stem vars for the remote host command port */
addr.!family = 'AF_INET'
addr.!port = self~rcport
addr.!addr = ipaddr

/* connect to the ftp server */
retc = SockConnect(self~csock, 'addr.!')
if retc <> 0 then do
   self~debugsay('Error: Unable to connect to' self~rhost'.')
   call SockShutDown self~csock, 2
   call SockClose self~csock
   self~csock = 0
   self~ftperrno = 'FTPCONNECT'
   return -1
   end

/* get the header message from the server */
self~debugsay('waiting on the header from' self~rhost)
retc = self~recvresponse('2')
if retc = .true then do
   self~debugsay('Error: header message from' self~rhost 'failed.')
   self~ftperrno = 'FTPCONNECT'
   return -1
   end

/* log the user in */
retc = self~transactsock('USER' self~user, '2 331')
if retc = .true then do
   self~debugsay('Error: USER command to' self~rhost 'failed.')
   self~ftperrno = 'FTPLOGIN'
   return -1
   end
retc = self~transactsock('PASS' self~password, '23 332')
if retc = .true then do
   self~debugsay('Error: PASS command to' self~rhost 'failed.')
   self~FtpLogoff()
   self~ftperrno = 'FTPLOGIN'
   return -1
   end
if account = '' & self~ftperrno = '332' then do
   self~debugsay('Error: ACCT information required by' self~rhost'.')
   self~FtpLogoff()
   self~ftperrno = 'FTPLOGIN'
   return -1
   end
if account <> '' then do
   retc = self~transactsock('ACCT' account, '2')
   if retc = .true then do
      self~debugsay('Error: ACCT command to' self~rhost 'failed.')
      self~ftperrno = 'FTPLOGIN'
      return 0
      end
   end

/* establish remote operating system type */
/* we need to do this before the first chdir so that it supports MVS correctly */
retc = self~FtpSys()

return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpSite                                                            */
/* Description: send a site command to the FTP server                         */
/*----------------------------------------------------------------------------*/

::method FtpSite

self~response = .array~new()

/* check/get args */
if arg() > 1 then raise syntax 93.902 array (1)
if arg() < 1 then raise syntax 93.901 array (1)

/* perform transaction */
retc = self~transactsock('SITE' arg(1), '2')
if retc = .true then do
   self~debugsay('Error: SITE command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the edited response */
temp = self~cmdresponse[self~cmdresponse~items]
parse var temp . temp
self~response[1] = temp

self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpSys                                                             */
/* Description: return the ftp server operating system description            */
/*----------------------------------------------------------------------------*/

::method FtpSys

self~response = .array~new()

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

/* perform transaction */
retc = self~transactsock('SYST', '2')
if retc = .true then do
   self~debugsay('Error: SYST command to' self~rhost 'failed.')
   self~ftperrno = 'FTPCOMMAND'
   return -1
   end

/* get the edited response */
temp = self~cmdresponse[self~cmdresponse~items]
if temp~pos('"') > 0 then parse var temp retc '"' temp '"'
else parse var temp retc temp
self~response[1] = temp

/* set the remote server operating system */
temp = self~response[1]~translate()
select
   when temp~pos('WIN')  > 0 then self~srvr_os = 'WIN'
   when temp~pos('OS/2') > 0 then self~srvr_os = 'OS/2'
   when temp~pos('VMS')  > 0 then self~srvr_os = 'UNIX'
   when temp~pos('VM')   > 0 then self~srvr_os = 'VM'
   when temp~pos('MVS')  > 0 then self~srvr_os = 'MVS'
   otherwise                      self~srvr_os = 'UNIX'
   end

self~ftperrno = ''
return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpTrace                                                           */
/* Description: toggle the trace of ftp commands display                      */
/*----------------------------------------------------------------------------*/

::method FtpTrace

self~response = .array~new()

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

/* toggle the flag */
if self~traceflg = 0 then self~traceflg = 1
else self~traceflg = 0

return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpTraceLog                                                        */
/* Description: begin an logging trace information to a file                  */
/*----------------------------------------------------------------------------*/

::method FtpTraceLog

self~response = .array~new()

/* check/get args */
if arg() > 2 then raise syntax 93.902 array (2)
if arg() < 1 then raise syntax 93.901 array (1)
if arg() = 1 then do
   use arg fname
   mode = 'R'
   end
else do
   use arg fname, mode
   mode = mode~substr(1, 1)~translate()
   if mode = 'R' then nop
   else raise syntax 93.914 array (2, 'R', mode)
   end

/* open the trace file */
if mode = '' then mode = 'R'
self~tracelog = .stream~new(fname)
if mode = 'R' then do
   retc = self~tracelog~open('write replace')
   end
else do
   retc = self~tracelog~open('write append')
   end
if retc <> 'READY:' then do
   self~debugsay('Error: cannot open trace file' fname'.')
   self~ftperrno = 'FTPCOMMAND'
   self~tracelog = .nil
   return -1
   end

return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpTraceLogOff                                                     */
/* Description: stop trace logging                                            */
/*----------------------------------------------------------------------------*/

::method FtpTraceLogOff

self~response = .array~new()

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

/* close the trace file */
if self~tracelog <> .nil then do
   self~tracelog~close()
   self~tracelog = .nil
   end

return 0


/*----------------------------------------------------------------------------*/
/* Method: FtpVersion                                                         */
/* Description: return the RxFtp class version string                         */
/*----------------------------------------------------------------------------*/

::method FtpVersion

self~response = .array~new()

/* check/get args */
if arg() > 0 then raise syntax 93.902 array (0)

self~ftperrno = ''
return self~version()


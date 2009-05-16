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


::requires 'socket.cls'


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: StreamSocket                                                        */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class 'StreamSocket' public subclass InputOutputStream

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: StreamSocket                                                        */
/*        Class methods                                                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: StreamSocket                                                        */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::method hostname attribute private -- the host name
::method port     attribute private -- the host port
::method s        attribute private -- the socket instance
::method opened   attribute private -- open flag            
::method rcvbuf   attribute private -- the recieve buffer
::method bufsize  attribute private -- the recieve buffer size

/*----------------------------------------------------------------------------*/
/* Method: Read                                                               */
/* Description: read a message or block of bytes from the socket (may return  */
/*              a msg smaller than the requested bufsize)                     */
/*----------------------------------------------------------------------------*/

::method read private
expose state description s bufsize
use strict arg
x = s~recv(bufsize)
if x = .nil then  x = ''
return x

/*----------------------------------------------------------------------------*/
/* Method: Write                                                              */
/* Description: write a message or block of bytes to the socket               */
/*----------------------------------------------------------------------------*/

::method write private
expose state description s
use strict arg msg
retc = s~send(msg)
return retc -- return the number of bytes written

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: StreamSocket                                                        */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Method: arrayIn                                                            */
/* Description: this is an invalid method for stream sockets                  */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method arrayIn
raise syntax 93.963 array ()  -- not supported
return

/*----------------------------------------------------------------------------*/
/* Method: arrayOut                                                           */
/* Description: this is an invalid method for stream sockets                  */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method arrayOut
raise syntax 93.963 array ()  -- not supported
return

/*----------------------------------------------------------------------------*/
/* Method: charIn                                                             */
/* Description: read one character from the stream                            */
/* Arguments: none                                                            */
/*         len - number of bytes to read                                      */
/*----------------------------------------------------------------------------*/

::method charIn
expose state description s rcvbuf opened
if opened = .false then do
   retc = self~open()
   if retc <> 'READY:' then return ''
   end
use strict arg start = 0, len = 1
-- we ignore start since this is not a persistent stream
if len < 0 then raise syntax 93.906 array (2, len)
if state = 'NOTREADY' then raise notready array('Socket') return ''
do while rcvbuf~length < len
   rcvbuf~append(self~read())
   if rcvbuf~length = 0 then do
      state = 'NOTREADY'
      description = s~errno
      raise notready array('Socket') return ''
      end
   end
state = 'READY'
description = ''
x = rcvbuf~substr(1, len)
rcvbuf~delete(1, len)
return x

/*----------------------------------------------------------------------------*/
/* Method: charOut                                                            */
/* Description: this is an invalid method for stream sockets                  */
/* Arguments:                                                                 */
/*         str   - the string to be written                                   */
/*         start - (optional) the start position (ignored)                    */
/*----------------------------------------------------------------------------*/

::method charOut
expose opened
if opened = .false then do
   retc = self~open()
   if retc <> 'READY:' then return 0
   end
use strict arg str, start = 0
-- we ignore start since this is not a persistent stream
retc = self~write(str)
if retc = -1 then do
   state = 'NOTREADY'
   description = s~errno
   raise notready array('Socket') return 0
   end
state = 'READY'
description = ''
return retc

/*----------------------------------------------------------------------------*/
/* Method: chars                                                              */
/* Description: return the state of the stream as a boolean                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method chars
expose opened
if opened = .true then return 1
return 0

/*----------------------------------------------------------------------------*/
/* Method: close                                                              */
/* Description: close the stream                                              */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method close
expose state description s opened
if opened = .true then do
   if s~close() = -1 then do
      state = 'ERROR'
      description = s~errno
      -- do not raise an error condition here
      return self~description
      end
   end
state = 'READY'
description = ''
return self~description

/*----------------------------------------------------------------------------*/
/* Method: description                                                        */
/* Description: returns the description of the stream state                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method description
expose state description
return (state || ':' description)~strip()

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*              If only one argument is supplied it is assumed to be a        */
/*              Socket class instance.                                        */
/* Arguments:                                                                 */
/*         host    - the hostname or ip address                               */
/*         port    - the port number                                          */
/*         bufsize - (optional) the recieve buffer size                       */
/*----------------------------------------------------------------------------*/

::method init
expose state description s hostname port opened bufsize rcvbuf
if arg() = 1 then do
   use strict arg s
   if \s~isA(.Socket) then ,
    raise syntax 93.914 array (1, 'a Socket class instance', s)
   inet = s~getPeerName()
   hostname = inet~address()
   port = inet~port()
   bufsize = 4096
   opened = .true
   end
else do
   use strict arg hostname, port, bufsize = 4096
   opened = .false
   s = .Socket~new('AF_INET', 'SOCK_STREAM', 0)
   if s~errno <> '' then do
      state = 'ERROR'
      description = s~errno
      raise error 5 array('Socket') return
      return
      end
   end
rcvbuf = .mutablebuffer~new(, bufsize)
state = 'READY'
description = ''
return

/*----------------------------------------------------------------------------*/
/* Method: lineIn                                                             */
/* Description: read one line from the stream                                 */
/* Arguments:                                                                 */
/*         line - line start position (ignored)                               */
/*         cnt  - the number of lines to read (either 0 or 1, default is 1)   */
/*----------------------------------------------------------------------------*/

::method lineIn
expose s state description rcvbuf opened
if opened = .false then do
   retc = self~open()
   if retc <> 'READY:' then return ''
   end
use strict arg line = 0, cnt = 1
-- we ignore line since this is not a persistent stream
if cnt = 0 then return ''
if cnt > 1 then raise syntax 93.908 array(2, 1)
if state = 'NOTREADY' then raise notready array('Socket') return ''
do while rcvbuf~pos('0D0A'x) = 0
   buf = self~read()
   if buf~length() = 0 then do
      if rcvbuf~length() > 0 then buf = '0D0A'x
      end
   rcvbuf~append(buf)
   if rcvbuf~length() = 0 then do
      state = 'NOTREADY'
      description = s~errno
      raise notready array('Socket') return ''
      end
   end
state = 'READY'
description = ''
x = rcvbuf~substr(1, rcvbuf~pos('0D0A'x) - 1) -- strip the CRLF
rcvbuf~delete(1, rcvbuf~pos('0D0A'x) + 1)
return x

/*----------------------------------------------------------------------------*/
/* Method: lineOut                                                            */
/* Description: write line to the socket                                      */
/* Arguments:                                                                 */
/*         str  - the string to be written                                    */
/*         line - (optional) the line start position (ignored)                */
/*----------------------------------------------------------------------------*/

::method lineOut
expose opened
if opened = .false then do
   retc = self~open()
   if retc <> 'READY:' then return 0
   end
use strict arg str = '', line = 0
-- we ignore line since this is not a persistent stream
retc = self~write(str || '0D0A'x)
if retc = -1 then do
   state = 'NOTREADY'
   description = s~errno
   raise notready array('Socket') return 0
   end
state = 'READY'
description = ''
return retc

/*----------------------------------------------------------------------------*/
/* Method: lines                                                              */
/* Description: return the state of the stream as a boolean                   */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method lines
expose opened
if opened = .true then return 1
return 0

/*----------------------------------------------------------------------------*/
/* Method: open                                                               */
/* Description: connect the socket to the host                                */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method open
expose state description s opened hostname port
if opened then do
   state = 'READY'
   description = ''
   return self~description
   end
use strict arg
if opened = .true then do
   state = 'READY'
   description = ''
   return self~description
   end
addr = .InetAddress~new(hostname, port)
retc = s~connect(addr)
if retc = -1 then do
   state = 'NOTREADY'
   description = s~errno
   -- do not raise a condition here
   return self~description
   end
opened = .true
state = 'READY'
description = ''
return self~description

/*----------------------------------------------------------------------------*/
/* Method: position                                                           */
/* Description: this is an invalid method for stream sockets                  */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method position
raise syntax 93.963 array ()  -- not supported
return

/*----------------------------------------------------------------------------*/
/* Method: say                                                                */
/* Description: alias to lineOut                                              */
/* Arguments:                                                                 */
/*         str  - the string to be written                                    */
/*----------------------------------------------------------------------------*/

::method say
use strict arg msg
retc = self~lineOut(msg)
return

/*----------------------------------------------------------------------------*/
/* Method: state                                                              */
/* Description: returns the state of the stream                               */
/* Arguments: none                                                            */
/*----------------------------------------------------------------------------*/

::method state   
expose state
return state

/*----------------------------------------------------------------------------*/
/* Method: string                                                             */
/* Description: returns a string representing the stream.                     */
/*----------------------------------------------------------------------------*/

::method string
expose hostname port
return hostname':'port


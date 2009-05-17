/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Classes to support SMTP.                                      */
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


::requires 'mime.cls'
::requires 'streamsocket.cls'


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: SMTPMSG                                                             */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class smtpmsg public


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: SMTPMSG                                                             */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: SMTPMSG                                                             */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute from           -- from header
::attribute subject        -- subject header


/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*----------------------------------------------------------------------------*/

::method init
expose content from recipients subject
use strict arg
content = ''
from = ''
recipients = .array~new()
subject = ''
return


/*----------------------------------------------------------------------------*/
/* Method: content                                                            */
/* Description: get the content of the message                                */
/*----------------------------------------------------------------------------*/

::attribute Content get
expose content
use strict arg
return content~string


/*----------------------------------------------------------------------------*/
/* Method: content=                                                           */
/* Description: add mime part or mimemultipart to the message                 */
/*----------------------------------------------------------------------------*/

::attribute Content set
expose content
use strict arg part
if part~isA(.mimepart) | part~isA(.mimemultipart) | part~isA(.string) then content = part
else raise syntax 93.948 array('MIMEPART', 'MIMEMULTIPART')
return


/*----------------------------------------------------------------------------*/
/* Method: addRecipient                                                       */
/* Description: add a recipient                                               */
/*----------------------------------------------------------------------------*/

::method addRecipient
expose recipients
use strict arg rec
recipients[recipients~items + 1] = rec
return


/*----------------------------------------------------------------------------*/
/* Method: Recipients                                                         */
/* Description: get the recipients                                            */
/*----------------------------------------------------------------------------*/

::method Recipients
expose recipients
use strict arg
return recipients


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: SMTP                                                                */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class smtp public


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: SMTP                                                                */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute authtypes  private -- authorization types, if any
::attribute strmsock   private -- stream socket class instance
::attribute data       private -- unparsed received data
::attribute lhost      private -- local host ip address
::attribute rcport     private -- remote SMTP port
::attribute resp       private -- raw unparsed command response buffer
::attribute rhost      private -- remote SMTP host name/ip address


/*----------------------------------------------------------------------------*/
/* Method: strmtransact                                                       */
/* Description: perform a single socket transaction & check return code       */
/*----------------------------------------------------------------------------*/

::method strmtransact private
expose strmsock cmdresponse response smtperrno
use strict arg cmd, respcode
cmdresponse[cmdresponse~items + 1] = cmd
retc = strmsock~lineout(cmd)
if retc = 1 then return .true
/* get the response, possibly multiple lines */
cmdresponse[cmdresponse~items + 1] = strmsock~linein()
startresp = cmdresponse~items
do while cmdresponse[cmdresponse~items]~substr(4, 1) = '-'
   cmdresponse[cmdresponse~items + 1] = strmsock~linein()
   end
/* check the response & return .false if we find an expected return code */
smtperrno = cmdresponse[self~cmdresponse~items]~substr(1, 3)
if respcode = '*' then return .false
parse var respcode r1 respcode
do while r1 <> ''
   /* per the RFC 959 only check the first digit of the return code */
   if r1~substr(1, 1) = smtperrno~substr(1, 1) then return .false
   parse var respcode r1 respcode
   end
/* we found an unexpected return code */
do i = startresp to cmdresponse~items
   self~debugsay('Error: unexpected response:' cmdresponse[i])
   end
return .true


/*----------------------------------------------------------------------------*/
/* Method: auth_login                                                         */
/* Description: login using plain text login authentication                   */
/*----------------------------------------------------------------------------*/

::method auth_login private
expose authid rhost password
use strict arg
retc = self~strmtransact('AUTH LOGIN', '3')
if retc = .true then do
   self~debugsay('Error: AUTH LOGIN command to' rhost 'failed.')
   return -1
   end
retc = self~strmtransact(authid~encodebase64, '3')
if retc = .true then do
   self~debugsay('Error: sending authid to' rhost 'failed.')
   return -1
   end
retc = self~strmtransact(password~encodebase64, '2')
if retc = .true then do
   self~debugsay('Error: sending password to' rhost 'failed.')
   return -1
   end
return 0


/*----------------------------------------------------------------------------*/
/* Method: debugsay                                                           */
/* Description: say the message if debug = .true                              */
/*----------------------------------------------------------------------------*/

::method debugsay private unguarded
/* any value other than .false activates debug messages */
if self~debug <> .false then say arg(1)
return


/*----------------------------------------------------------------------------*/
/* Method: setdefaults                                                        */
/* Description: setup/reset all the defaults for the class                    */
/*----------------------------------------------------------------------------*/

::method setdefaults private
expose authid authtypes cmdresponse debug password rcport resp response rhost ,
 smpterrno strmsock localhost
authid = ''                 -- authentication id
authtypes = ''              -- authorization types
cmdresponse = .array~new()  -- all command/status responses from the server
debug = .false              -- .false suppresses debugging information
password = ''               -- account password on the smtp server
rcport = 25                 -- default remote command port
resp = ''                   -- raw unparsed command socket response buffer
response = ''               -- parsed last command response from the server
rhost = ''                  -- host name or ip address of the smtp server
smtperrno = ''              -- last smtp response code
strmsock = .nil             -- get a stream socket instance
localhost = .socket~gethostbyaddr('127.0.0.l') -- get our host info
return


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: SMTP                                                                */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute authid          -- authentication id (username@xxx.com)
::attribute cmdresponse     -- all parsed commands and responses
::attribute debug           -- .false suppresses debug messages
::attribute localhost   get -- our local host name
::attribute password        -- remote SMTP password
::attribute response        -- parsed command response from last command
::attribute smtperrno       -- last smtp response code

/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*----------------------------------------------------------------------------*/

::method init
use strict arg
self~setdefaults
return


/*----------------------------------------------------------------------------*/
/* Method: Localhost=                                                         */
/* Description: set the local host name                                       */
/*----------------------------------------------------------------------------*/

::attribute LocalHost set
expose localhost
use strict arg localhost
if localhost~isA(.hostinfo) then localhost= localhost~name
return


/*----------------------------------------------------------------------------*/
/* Method: Connect                                                            */
/* Description: set up the primary connection to the smtp server and logon    */
/* Arguments:                                                                 */
/*      remote_host                                                           */
/*      authid      (optional)                                                */
/*      password    (optional)                                                */
/*----------------------------------------------------------------------------*/

::method Connect
expose rhost authid password rcport strmsock cmdresponse localhost authtypes
use strict arg rhost, authid = '', password = ''
/* check to see if an alternate port was specified */
hostname = rhost
parse var hostname hostname ':' tport
if tport <> '' then do
   rhost = hostname
   if tport~datatype('W') = 0 then do
      return -1
      end
   rcport = tport
   end
strmsock = .streamsocket~new(rhost, rcport)
retc = strmsock~open()
if retc = .true then return -1
/* get the header message from the server */
self~debugsay('waiting on the header from' rhost)
/* get the response, possibly multiple lines */
cmdresponse[cmdresponse~items + 1] = strmsock~linein()
startresp = cmdresponse~items
do while cmdresponse[cmdresponse~items]~substr(4, 1) = '-'
   cmdresponse[cmdresponse~items + 1] = strmsock~linein()
   end
/* get the capabilities of the server */
retc = self~strmtransact('EHLO' localhost~name, '2')
if retc = .true then return -1
/* get the authorization capability */
do i = 1 to cmdresponse~items
   self~debugsay(cmdresponse[i])
   if cmdresponse[i]~substr(5, 4) = 'AUTH' then do
      authtypes = cmdresponse[i]~substr(5)
      leave
      end
   end
/* if necessary, login */
select
   /* select the authorization method to use to login */
   when authtypes~pos('LOGIN') > 0 then retc = self~auth_login()
   otherwise retc = 0
   end
return retc


/*----------------------------------------------------------------------------*/
/* Method: Logoff                                                             */
/* Description: logoff the smtp server                                        */
/*----------------------------------------------------------------------------*/

::method Logoff
expose responce strmsock
use strict arg
/* log the user off */
retc = strmsock~lineout('QUIT')
/* shutdown the socket */
strmsock~close
/* reset all the defaults */
self~setdefaults
return 0


/*----------------------------------------------------------------------------*/
/* Method: Send                                                               */
/* Description: Send the mail message                                         */
/*----------------------------------------------------------------------------*/

::method Send
expose response rhost strmsock cmdresponse
use strict arg msg
use arg msg
if \msg~isA(.smtpmsg) then raise syntax 93.948 array (1, 'SMTPMsg')
response = .array~new()
/* send the 'To:' header */
retc = self~strmtransact('MAIL FROM:'msg~From, '2')
if retc = .true then do
   self~debugsay('Error: MAIL FROM command to' rhost 'failed.')
   return -1
   end
/* send the 'From:' header */
do rec over msg~recipients
   retc = self~strmtransact('RCPT TO:' rec, '2')
   if retc = .true then do
      self~debugsay('Error: RCPT TO command to' rhost 'failed.')
      return -1
      end
   end
/* send the message body */
retc = self~strmtransact('DATA', '3')
if retc = .true then do
   self~debugsay('Error: DATA command to' self~rhost 'failed.')
   return -1
   end
retc = self~strmsock~lineout('From:' msg~From)
do i = 1 to msg~Recipients~items
   if i < msg~Recipients~items then suffix = ','
   else suffix = ''
   if i = 1 then retc = self~strmsock~lineout('To:' msg~Recipients[i] || suffix)
   else retc = self~strmsock('   ' msg~Recipients[i] || suffix)
   end
retc = self~strmsock~lineout('Subject:' msg~Subject)
-- retc = self~strmsock~lineout('')
retc = self~strmsock~lineout(msg~content)
retc = self~strmsock~lineout('.') -- this is the message terminator for the smtp protocol
/* get and check the response */
cmdresponse[cmdresponse~items + 1] = strmsock~linein()
startresp = cmdresponse~items
do while cmdresponse[cmdresponse~items]~substr(4, 1) = '-'
   cmdresponse[cmdresponse~items + 1] = strmsock~linein()
   end
if cmdresponse[cmdresponse~items]~substr(1, 1) \= '2' then do
   self~debugsay('Error: DATA command to' rhost 'failed.')
   return -1
   end
return 0


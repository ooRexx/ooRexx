/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: MIME classes.                                                 */
/*                                                                            */
/* Copyright (c) 2006-2009 Rexx Language Association. All rights reserved.    */
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


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: MIMEPART                                                            */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class mimepart public


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: MIMEPART                                                            */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute mimever     private -- mime-version header


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: MIMEPART                                                            */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::constant  crlf '0D0A'x        -- carriage return/line feed
::attribute type                -- content-type header
::attribute encoding            -- content-transfer-encoding header
::attribute id                  -- content-id header
::attribute description         -- content-description header
::attribute disposition         -- content-disposition header
::attribute content     get     -- the actual content


/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*----------------------------------------------------------------------------*/

::method init
expose mimever type encoding id description disposition content
use strict arg type = 'text/plain'
mimever = '1.0'
encoding = ''
id = ''
description = ''
disposition = ''
content = ''
return


/*----------------------------------------------------------------------------*/
/* Method: addContent                                                         */
/* Description: add content to the mime part                                  */
/*----------------------------------------------------------------------------*/

::method addContent
expose content
use strict arg c
if c == '.' then c = '..'  -- we need this for smtp messages
content = content || c || self~crlf
return


/*----------------------------------------------------------------------------*/
/* Method: string                                                             */
/* Description: returns the entire formatted mime message                     */
/*----------------------------------------------------------------------------*/

::method string
expose mimever type encoding id description disposition content
use strict arg
headers = 'MIME-Version:' mimever || self~crlf
if type <> '' then do
   headers = headers || 'Content-Type:' type || self~crlf
   end
if encoding <> '' then do
   headers = headers || 'Content-Transfer-Encoding:' encoding || self~crlf
   end
if id <> '' then do
   headers = headers || 'Content-ID:' id || self~crlf
   end
if description <> '' then do
   headers = headers || 'Content-Description:' description || self~crlf
   end
if disposition <> '' then do
   headers = headers || 'Content-Disposition:' disposition || self~crlf
   end
mime = headers || self~crlf || self~content || self~crlf
return mime


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: MIMEMULTIPART                                                       */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::class mimemultipart public


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: MIMEMULTIPART                                                       */
/*        Private methods                                                     */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::attribute mimever     private -- mime-version header
::attribute multiparts  private -- each mime part


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* Class: MIMEMULTIPART                                                       */
/*        Public methods                                                      */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

::constant  crlf '0D0A'x        -- carriage return/line feed
::attribute type                -- content-type header
::attribute encoding            -- content-transfer-encoding header
::attribute id                  -- content-id header
::attribute description         -- content-description header
::attribute disposition         -- content-disposition header


/*----------------------------------------------------------------------------*/
/* Method: init                                                               */
/* Description: instance initialization                                       */
/*----------------------------------------------------------------------------*/

::method init
expose mimever type encoding id description disposition content multiparts
use strict arg
mimever = '1.0'
type = 'multipart/mixed'
encoding = ''
id = ''
description = ''
disposition = ''
multiparts = .array~new
return


/*----------------------------------------------------------------------------*/
/* Method: addPart                                                            */
/* Description: add a mime part                                               */
/*----------------------------------------------------------------------------*/

::method addPart
expose multiparts
use strict arg part
if part~isA(.mimepart) then do
   multiparts[multiparts~items + 1] = part
   end
else raise syntax 93.948 array('MIMEPART')
return


/*----------------------------------------------------------------------------*/
/* Method: string                                                             */
/* Description: get the content                                               */
/*----------------------------------------------------------------------------*/

::method string
expose mimever type encoding id description disposition content multiparts
use strict arg
headers = 'MIME-Version:' mimever || self~crlf
boundary = '===ooRexx=Mime=Part===' || c2x(date('b') || time('L')) || '=='
type = 'multipart/mixed; boundary="' || boundary || '"'
headers = headers || 'Content-Type:' type || self~crlf
if encoding <> '' then do
   headers = headers || 'Content-Transfer-Encoding:' encoding || self~crlf
   end
if id <> '' then do
   headers = headers || 'Content-ID:' id || self~crlf
   end
if description <> '' then do
   headers = headers || 'Content-Description:' description || self~crlf
   end
if disposition <> '' then do
   headers = headers || 'Content-Disposition:' disposition || self~crlf
   end
mime = headers || self~crlf
if multiparts~items > 0 then do
   do part over multiparts
      mime = mime || '--' || boundary || self~crlf || part~string || self~crlf
      end
   mime = mime || '--' || boundary || '--' || self~crlf
   end
return mime


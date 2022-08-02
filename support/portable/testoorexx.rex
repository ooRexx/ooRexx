#!/usr/bin/env rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2021 Rexx Language Association. All rights reserved.         */
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

/* Purpose: display information about this script and the Rexx version in use */
say "At       :" .DateTime~new
parse source s
say "source   :" s
parse version v
say "version  :" v
call dumpRexxInfo

/* Show information from .RexxInfo about this ooRexx installation */
::routine dumpRexxInfo
   say '.RexxInfo:'
   clz=.RexxInfo~class           -- get .RexxInfo's class object
   instMeths=clz~methods(.nil)   -- query all its instance methods
   i=0                           -- set counter to 0
   loop methName over instMeths~allIndexes~sort -- loop over sorted method names
      if methName="COPY" then iterate  -- skip COPY method (would cause a runtime error)
      i+=1                             -- increase counter
      value=.rexxInfo~send(methName)   -- send message
      if methName="ENDOFLINE" then value='"'value~c2x'"x'   -- make it displayable
      value=value~string               -- make sure we get the string value
      if value~pos("9999")>0 | value~pos("0000")>0 then  -- a large number?
         value=chunk(value)            -- format number
      say i~right(9)":" left(methName" ",18,'.')":" value
   end

/* Format large numbers in American style; could be used to format credit card numbers
   as well (sep=' ' and size=4). Edited to account for a leading sign.
   Author: Gil Barmwater, cf. his post on 2021-11-13 in the RexxLA mailing list
*/
::routine chunk public
    use arg str, sep=',', size=3

    sign=""                         -- if a sign, remove and remember it
    if pos(str~left(1),"+-")>0 then
    do
       sign=str~left(1)
       str =substr(str,2)
    end

    if str~length > size then do
        parse value str~length-size'|'str with p '|' +1 front +(p) back
        str = chunk(front, sep, size)||sep||back
    end
    return sign||str    -- add sign back if any

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/* make a .def file from an Oryx .exp file (used by Oryx make) */
parse arg iname oname opts
'@echo off'

if iname = '' then do
  say 'Syntax: makedef iname oname opts'
  exit 1
end
if left(oname,1) = '/' then do
  opts = oname opts
  oname = ''
end

parse var iname namepart '.' extn
if extn = '' then
  extn = 'exp'
input = namepart'.'extn

if oname \= '' then
  parse var oname namepart '.' extn
else
  extn = ''
if extn = '' then
  extn = 'def'
output = namepart'.'extn
if stream(output,'c','query exists') \= '' then
  'erase' output

emit = 'c'
exports = 0
imports = 0
do while lines(input) > 0
  addUnderbar = 0
  line = linein(input)
  select
    when line = '' then
      call lineout output, ';'

    when left(line,23) = '* OS/2 options (16-bit)' then
      if word(opts,1) = '/ibmc' then
        emit = 'c'
      else
        emit = 'o'

    when left(line,23) = '* OS/2 options (32-bit)' then
      if word(opts,1) = '/ibmc' then
        emit = 'o'
      else
        emit = 'c'

    when left(line,14) = '* OS/2 options' then
      emit = 'o'

    when left(line,23) = '* OS/2 exports (16-bit)' then
      if word(opts,1) = '/ibmc' then
        emit = 'c'
      else
        emit = 'e'

    when left(line,23) = '* OS/2 exports (32-bit)' then
      if word(opts,1) = '/ibmc' then
        emit = 'e'
      else
        emit = 'c'

    when left(line,14) = '* OS/2 exports' then
      emit = 'e'

    when left(line,23) = '* OS/2 imports (16-bit)' then
      if word(opts,1) = '/ibmc' then
        emit = 'c'
      else
        emit = 'i'

    when left(line,23) = '* OS/2 imports (32-bit)' then
      if word(opts,1) = '/ibmc' then
        emit = 'i'
      else
        emit = 'c'

    when left(line,14) = '* OS/2 imports' then
      emit = 'i'

    when left(line,13) = '* AIX exports' then do
      call lineout output, '; AIX exports...'
      emit = 'c'
      end

    when left(line,5) = '* end' then
      emit = 'c'

    when left(line,2) = '* ' then
      call lineout output, ';'line

    when emit = 'o' then /* emit option */
      call lineout output, substr(line,2)

    /*** following now obsolete - retained as memorabilia ****
    when emit = 'o' then do
      wc = pos('WINDOWCOMPAT',line)
      if wc > 0 & opts = '/ibmc' then
        call lineout output, substr(insert('NOT',line,wc-1),2)
      else
        call lineout output, substr(line,2)
      end
    **** end of memorabilia ***/

    when emit = 'c' then /* emit comment */
      call lineout output, ';'line

    when emit = 'e' then do /* emit export */
      if \exports then do
        call lineout output, 'EXPORTS'
        exports = 1
      end
      if left(line,1) = '*' then            /* we export all things w/ * (general   */
        line = substr(line,2)               /*  C runtime functions)                */
                                            /* All option will bebin w/ * and the   */
                                            /* 2nd character designates where this  */
                                            /* function is used/valid               */
      if left(line,1) = '-' then            /* Anything denoted w/ - is C Set/2 only*/
        if WORDPOS('/ibmcpp',opts) = 0 then /*  not included with C Set ++          */
         line = substr(line,2)
        else
         line = ';'substr(line,2)
      else if left(line,1) = '+' then       /* Anything denoted w/ + is C Set ++ Onl*/
        if WORDPOS('/ibmcpp',opts) \= 0 then
         line = substr(line,2)
        else
         line = ';'substr(line,2)
      else if left(line,1) = '%' then       /* Anything w/ % is C Set ++ and profiler*/
        if WORDPOS('/profiler',opts) \= 0 then
         line = substr(line,2)
        else
         line = ';'substr(line,2)
      else if left(line,1) = '$' then       /* Anything w/ $ is needed when not optimizing */
        if WORDPOS('/noopt',opts) \= 0 then
         line = substr(line,2)
        else
         line = ';'substr(line,2)
      else if left(line,1) = '=' then       /* Anything w/ = is needed for SOMV2 support   */
        if WORDPOS('/somv2',opts) \= 0 then
         line = substr(line,2)
        else
         line = ';'substr(line,2)
      else if left(line,1) = '&' then do    /* Anything w/ = is needed for SOMV2 support   */
         line = substr(line,2)
         addUnderbar = 1
       end

      if word(opts,1) = '/ibmc' | (line = translate(line) & \addUnderBar) then
        call lineout output, line
      else
        call lineout output, '_'line
    end

    when emit = 'i' then do /* emit import */
      if \imports then do
        call lineout output, 'IMPORTS'
        imports = 1
      end
      if word(opts,1) = '/ibmc' then
        call lineout output, substr(line,2)
      else do
        parse var line '*' module '.' entry
        if entry = translate(entry) then
          call lineout output, substr(line,2)
        else do
          parse var module alias '=' modname
          if modname \= '' then
            call lineout output, '_'alias'='modname'._'entry
          else
            call lineout output, module'._'entry
        end
      end
      end
  end
end

call lineout output
call stream input,'c','close'

exit 0

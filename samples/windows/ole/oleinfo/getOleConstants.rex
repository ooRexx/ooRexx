/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2002-2022 Rony G. Flatscher. All rights reserved.            */
/* Copyright (c) 2023      Rexx Language Association. All rights reserved.    */
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

/***********************************************************************
   name:    getOleConstants.rex
   purpose: create an Object Rexx file which saves the constants of the given
            OLE/ActiveX appId/clsId in a directory called "ole.const" in the
            environment .local (and hence can be retrieved by the environment
            symbol ".ole.const")
   usage:   getOleConstants appid/clsid [outfile]

            appid/clsid ... OLE/ActiveX application ID or CLSID
            outfile ....... optional name of the outputfile, otherwise STDOUT is used

                            use the above outfile in a "::requires" directive
                            which then allows you to use the defined constants,
                            e.g. ".ole.const~SWC_3RDPARTY"

            example: rexx getOleConstants InternetExplorer.Application msie_const.rex
                     ... then, whenever you need to access MSIE-constants, then merely use
                         "::requires msie_const.rex" and refer to them e.g. like
                         ".ole.const~csc_navigateBack"

***********************************************************************/

   if arg()=0 then -- show usage
   do
      say .resources~usage
      exit
   end


parse arg appId outFile

-- ole=.oleobject~new(appId)  -- get OLE proxy
ole=createOleObject(appId)    -- try to create OLE proxy
if ole=.nil then
do
   .error~say(appId": could not create the OLE proxy, aborting ...")
   exit -1
end

oleconst="ole.const"  -- define directory name to store OLE constants, change if needed

   -- get OLE constants, create sortable stem
constants.=ole~getConstant
stem. = .stem~new

stem.0=0
max=0
do counter i idx over constants.
   idx=substr(idx,2)  -- remove leading "!" character
   stem.i=idx || "09"x || ole~getConstant(idx)
   stem.0=i
   max=max(max, length(idx))  -- save longest constant name
end

   -- if no constants, abort
if stem.0=0 then
do
   .error~say("No constants found for:" pp(appId)", aborting ..." )
   exit -1
end

call SysStemSort "stem.", "A", "I"  -- sort stem

   -- dump infos
parse source . . thisPgm

   -- open file-stream
if outfile="" then of=.output -- appId"_constants.rex"
              else of=.stream~new(strip(outFile))~~open("write replace")

of~say("/*" pp(filespec("name", thisPgm)) "run on:" pp(.dateTime~new) "*/" )
of~say
tmpStr1="are"
tmpStr2="constants"
if stem.0=1 then  -- singular
do
   tmpStr1="is"
   tmpStr2="constant"
end

of~say("-- OLE/ActiveX-application/clsid:" pp(appId) "- there" tmpStr1 pp(stem.0) tmpStr2 )
of~say
of~say("-- create stringTable '"OLECONST"', if necessary; maybe shared with OLE constant definitions of other programs")
of~say("if .local~hasentry('"oleconst"')=.false then .local~"oleconst"=.stringTable~new -- create stringTable '"OLEconst"' in .local")
of~say

do i=1 to stem.0
  parse var stem.i const "09"x value
  of~say( '.'oleconst'~'left(const, max) '=' encode(value) )
end

of~close

::routine pp
  return "[" || arg(1)~string || "]"

::routine encode  -- if a number, leave unchanged, else escape quotes Rexx-style, enclose string into quotes

  if datatype(arg(1), "Number") & length(arg(1))<=digits() then return arg(1)  -- if a number, return value unchanged

  quote='"'
  tmp=changestr(quote, arg(1), quote||quote)
  return quote || tmp || quote

::resource usage  -- usage instructions
usage:   getOleConstants appid/clsid [outfile]

   appid/clsid ... OLE/ActiveX application ID or CLSID
   outfile ....... optional name of the outputfile, otherwise STDOUT is used

                   use the above outputfile in a "::requires" directive
                   which then allows you to use the defined constants,
                   e.g. ".ole.const~SWC_3RDPARTY"

   example: rexx getOleConstants InternetExplorer.Application msie_const.rex

            ... creates an ooRexx program named "msie_const.rex" which adds
                all found constants to the .ole.const stringTable (ooRexx 5.0,
                like a directory)

            ... then, whenever you need to access MSIE-constants, merely use

                   ::requires "msie_const.rex"

                in your program. This makes the InternetExplorer constants
                available via the environment symbol ".ole.const" such that
                you can refer to the constants by name in your programs like:

                   val=.ole.const~csc_navigateBack -- get the constant

                Note: you can require as many different ole constant files as
                      necessary in a single program; all constants will get
                      saved in ".ole.const"

::END

/** Makes sure that program does not get stopped if OLEObject cannot be created (e.g. SYNTAX 92.911),
    rather return .nil in that case. */
::routine createOleObject
   use strict arg appId
   signal on syntax
   return .oleobject~new(appId)  -- get OLE proxy
syntax:
   return .nil

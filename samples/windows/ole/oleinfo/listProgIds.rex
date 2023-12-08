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
   name:    listProgIds.rex
   purpose: list all ProgIDs to stdout, optionally with their CLSID

   usage:   listProgIds.rex
            listProgIds.rex needle
            listProgIds.rex 1
            listProgIds.rex 1 needle
            ... needle:   optional: a string, list ProgIdDs that contain this string (caseless comparison)
            ... 1         optional: supply CLSID

***********************************************************************/

parse arg indicator needle .
bCLSID=(indicator=1)
if \bCLSID then needle=indicator -- shift to needle

bTiming=.false -- .true -- determines whether timings are shown
time0=.DateTime~new

.clsid~analyze_and_build	   -- let the class method query and analyze the registry,
all_progid= .clsid~all_progid	-- get ProgID directory

time1=.DateTime~new
if bTiming then say "analyzing registry time spent:" pp(time1-time0)
   -- sort all ProgID indixes caselessly
allProgIds   = all_progid~allIndexes~sortWith(.CaselessComparator~new)

time2=.DateTime~new
items=allProgIds~items
if bTiming then
do
   say "sorting" items "took:" pp(time2-time1)
   say
end
prefix=.rexxinfo~architecture"-bit #"
len=items~length
bDots=.false
do counter i idx over allProgIds    -- idx is the ProgID
   if needle<>"", idx~caselessPos(needle)=0 then iterate

   o=all_progid~at(idx)    -- get ooRexx classid object
   hint=checkProgId(idx)   -- supply hint whether ProgID adheres to MS specs
   if bCLSID then
   do
      if bDots then
         say prefix i~right(len)"/"items":" "ProgId:" pp(idx)~left(70,".") || hint~left(5,".") "CLSID:" o~clsid
      else
         say prefix i~right(len)"/"items":" "ProgId:" pp(idx)~left(70) || hint~left(5) "CLSID:" o~clsid
      bDots=\bDots
   end
   else
      say prefix i~right(len)"/"items":" "ProgId:" pp(idx)~left(70) || hint~left(5)
end
time3=.DateTime~new
if bTiming then
do
   say "output took:                 " pp(time3-time2)
   say "total time:                  " pp(time3-time0)
end



::requires "reg_classids4ole.cls" -- get support for analyzing the registry

::routine pp
  return "["arg(1)"]"

::routine checkProgId         -- checks whether ProgID adheres to Microsoft's specifications
   use arg progid
   -- check validity of ProgId according to: <https://docs.microsoft.com/en-us/windows/win32/com/-progid--key>:
   --     length<=39, no punctuation (except dots), not start with a digit, ..
   -- also discussion at: <https://stackoverflow.com/questions/1754429/what-happens-if-i-violate-the-requirements-imposed-on-progids>
   hint=""
      -- too long or empty string?
   if progid~length >39 then hint="l"
   else if progid="" then hint="l"
      -- illegal character?
   if verify(progid,"ABCDEFGHIJKLMNOPQRSTUVWXYZ.abcdefghijklmnopqrstuvwxyz0123456789")>0 then hint||="c"
      -- must not start with a digit
   if datatype(progid[1],"W") then hint="d"
   if hint<>"" then return "*"hint"*"

   return hint


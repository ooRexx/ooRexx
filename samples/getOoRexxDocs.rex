#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2023 Rexx Language Association. All rights reserved.         */
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

/* purpose: utility to download the ooRexx documentation from SourceForge */
baseUrl="https://sourceforge.net/projects/oorexx/files/oorexx-docs/"
dirArr=getDirectories(baseUrl)   -- get array of available documentation directories

   -- check whether user wishes usage information
if .sysCargs[1]~isNil=.false,  "/h -h /? -? ?"~caselessPos(.sysCargs[1])>0 then  -- show usage?
do
   say .resources~usage
   say
   say pp(baseUrl) "currently contains the following directories:"
   say
   say dirArr        -- show documentation directories in descending order
   exit 0
end

needle=""
if arg()>0 then      -- documentation directory supplied?
do
   downloadDir=.sysCargs[1]   -- desired directory name
   needle     =.sysCargs[2]   -- text fragment file name must contain to be downloaded
   if needle~isNil then needle=""   -- any file name qualifies for download
end
else
   downloadDir=dirArr[1]      -- get first entry from directory array

if downloadDir~isNil | dirArr~hasItem(downloadDir)=.false then
do
   say pp(downloadDir) "does not exist, aborting ..."
   exit -1
end

localDir="docs."downloadDir   -- build local directory name
if sysFileExists(localDir)=.false then -- need to create it?
do
   rc=sysMkDir(localDir)   -- create local directory
   if rc<>0 then
   do
      say "RC="rc "could not create subdirectory" pp(localDir)",aborting..."
      exit rc
   end
end
call directory localDir    -- change into subdirectory

url=baseUrl || downloadDir"/"
cmd='curl --silent --list-only --noproxy "*" --insecure' url
say "cmd:" pp(cmd)         -- show command we are about to execute
outArr=.array~new          -- create array to receive the html text
   -- get the SourceForge download page html text with all the download urls
address system cmd with output using (outArr)
data=outArr~makeString     -- turn array into a string

urlArr=.array~new          -- array to store extracted urls
do counter c while data<>""
   parse var data '"files_name_h"><a href="' url '/download" title' data
   if url<>"" then urlArr~append(url)  -- download url
              else leave               -- no remaining download urls
   say "#" c~right(2)":" pp(url)       -- show download url
end
say

say "downloading ..."
downloaded=0
do counter c url over urlArr
   leadin="#" c~right(2)":"
   if needle<>"", url~caselessPos(needle)=0 then
   do
      say leadin "skipping" pp(url) "does not contain needle" pp(needle)
      iterate
   end
   cmd='curl --silent --noproxy "*" --insecure --remote-time -L -O' url
   say leadin pp(cmd) "..."
   address system cmd      -- downloads the file
   downloaded+=1
end
say pp(downloaded) "files downloaded to local directory named" pp(localDir)

/* ========================================================================== */
/* gets and extracts the SourceForge "oorexx/files/oorexx-docs" directories into
   an array which gets returned after being sorted caselessly and descendingly
   (directory starting with highest number first)
*/
::routine getDirectories      -- curl command: could be issued directly on the command line
 parse arg baseUrl
 cmd='curl --silent --noproxy "*" --insecure --list-only' baseUrl
 outArr =.array~new
   -- get the SourceForge oorexx-docs html page containing all documentation directories
 address system cmd with output using (outArr)
 data=outArr~makeString
 resArr=.array~new         -- array to receive the available directories
 do while data<>""         -- extract all available directories
    parse var data before '<tr title="' dir '" class="folder' data
    if data<>"" then
       resArr~append(dir)     -- save directory name
 end
 comp=.caselessDescendingComparator~new   -- create a comparator
 return resArr~sortWith(comp)    -- sort with comparator, return sorted array

/* ========================================================================== */
-- routine "Pretty-Print" ;) - encloses argument string in brackets
::routine pp
  return "["arg(1)"]"

/* ========================================================================== */
::resource usage  -- returns usage
getOoRexxDocs.rex [directory [needle]]

                  a utility to download the ooRexx documentation from SourceForge
                  and save them in a subdirectory "docs.V" of the current directory
                  where "V" is the specified version (e.g. "5.0.0")

      no argument ... download all files from the directory carrying the highest
                      version number

      ? or -h     ... show this usage information

      directory   ... the SourceForge documentation directory to download from,
                      usually named after the ooRexx version (e.g. "4.2.0",
                      "5.0.0", "5.1.0beta")

      needle      ... only downloads files from the SourceForge directory which
                      contain the (caseless) needle in their names (e.g. "-html.zip",
                      ".pdf", "ref")

N.B.: This program uses "curl" on all platforms. You can find more information
      on the Internet or <https://curl.se> and <https://en.wikipedia.org/wiki/CURL>.
::END             -- end of resource usage

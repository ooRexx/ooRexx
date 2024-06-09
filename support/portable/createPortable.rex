#!bin/rexx
-- note: the above hashbang is intentional: will run the archive's bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2021-2024 Rexx Language Association. All rights reserved.    */
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
/*    name:       createPortable.rex
      purpose:    - create zip-archives off an installation
                  - use the same directory layout for all platforms, use Linux structure, i.e.
                        bin      binaries, Windows includes dll
                        lib      Unix: shared objects/dynamic libraries, Windows: link libraries only (rexx.lib, rexxapi.lib)
                        include  include files (*.h)
                        samples  samples
                        doc      pdf-files, if present

      strategy:   - do DESTDIR installation into received temporary directory
                  - create a sibling directory with the name of the archive
                     - copy all files from the installation to the new flat layout
                     - do not expect any specific directory layout, use sysFileTree to
                       locate them
                  - create readme.txt and setupoorexx.{cmd|sh}
                  - copy setup.rex and test.rex

       changed:   - 20220130, rgf: ren setup.{*} to setupoorexx.{*}, and rhenv{*} to rxenv{*}
                  - 20220209, rgf: changed "test.rex" to "testoorexx.rex"
                  - 20231113, rgf: changed eCopy for Unix to "cp -RpPf" as suggested by P.O.
                  - 20240609, rgf: changed eCopy for Windows to "copy /y" (overwrite),
                                   copy support/portable/readme.txt
*/

local=.context~package~local     -- get package's local directory
local~leadin="+--->"       -- lead in to ease spotting in sea of output
local~lw    =17            -- width of debug labels
local~bDebug=.true  -- .false


parse arg "-portable_dirname " zipDir " -zip_name " zipName
if .bDebug=.true then say .leadin "zipDir"~left(.lw,".")":" pp(zipDir) "zipName:" pp(zipName)

parse source op_sys +1 1 . . thisPath -- op_sys: W...Windows, L...Linux, D...Darwin (Apple), ...
thisLocation=filespec("location",thisPath)
if .bDebug=.true then
do
   -- say .leadin "op_sys"~left(.lw,".")":" pp(op_sys) "directory():" pp(directory()) "thisLocation:" pp(thisLocation)
   say .leadin "op_sys"~left(.lw,".")":" pp(op_sys)
   say .leadin "scriptLocation"~left(.lw,".")":" pp(thisLocation) "script:" pp(filespec('name',thisPath))
   say .leadin "working directory"~left(.lw,".")":" pp(directory())
end

-- create DateTime instance
dt=.dateTime~new
strDT="ISO" .dateTime~new~utcIsoDate   -- will be used in the generated setup.{cmd|sh} script
strDT=strDT "== UTC "dt~toUtcTime

   -- do a DESTDIR installation into temporary directory
cmdMake="make"
if op_sys="W" then cmdMake="nmake"
cmd=cmdMake "DESTDIR="zipDir "install"
if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
address system cmd      -- do a local install
if .bDebug=.true then say .leadin "RC"~left(.lw,".")":" pp(rc) "(by" cmd")" "generated:" dt

-- define some operating system dependent values
if op_sys='W' then
   eCopy="copy /y"
else  -- Unix
-- v option not available on all platforms, removed 2023-02-15 POJ
-- changed "cp -af" with "cp -RpPf" as suggested by POJ
   eCopy="cp -RpPf"

targetRootDir=zipDir || .rexxinfo~directorySeparator || zipName  -- the archive's exploded root directory
if .bDebug=.true then say .leadin "targetRootDir"~left(.lw,".")":" pp(targetRootDir) "zipDir:" pp(zipDir) "zipName:" pp(zipName)

if op_sys="W" then
   call setupWindowsZipStructure zipDir, zipName, strDT        -- will create targetRootDir
else
   call setupUnixZipStructure zipDir, zipName, op_sys, strDT   -- will create targetRootDir

call make_readme targetRootDir, op_sys

do file over ("setupoorexx.rex", "testoorexx.rex", "readme.txt")    -- copy support files
   srcFile=thisLocation || file
   address system eCopy quote(srcFile) quote(targetRootdir)
   if .bDebug=.true then say .leadin "... copied"~left(.lw,".")":" pp(file) "to" pp(targetRootDir) "| RC="rc
   if op_sys<>"W" then
      address system "chmod 775" targetRootDir"/"file
end

/* "pretty-print" ;), enclose argument in square brackets */
::routine pp
  return "["arg(1)"]"

/* Windows version: create the portable zip archive's structure, copy the files from the
   installed sibling directory to the proper zip archive's directories.                   */
::routine setupWindowsZipStructure  -- does not expect any specific install layout
   use strict arg zipDir, zipName, strDT

   targetRootDir = zipDir'\'zipName
   call SysMkDir targetRootDir      -- create root directory for zip archive

      -- create setup.cmd script from resource
   .stream~new(targetRootDir"\setupoorexx.cmd")~~open("replace write")                                -
           ~~lineout(.resources~windows_setup_scripts~toString~changeStr("_HERE_DATE_TIME",strDT)) -
           ~~close

      -- create and copy to the "bin" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"\rexx.exe", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexx.exe" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "\"
   source        = sourceRootDir'*'
   targetDir     = targetRootDir'\bin'
   call SysMkDir targetDir          -- create bin directory for zip archive
   cmd = "copy" quote(source) targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "lib" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"\rexx.lib", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexx.lib" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "\"
   source        = sourceRootDir'*.lib'
   targetDir     = targetRootDir'\lib'
   call SysMkDir targetDir          -- create bin directory for zip archive
   cmd = "copy" quote(source) targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "include" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"\oorexxapi.h", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "oorexxapi.h" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "\"
   source        = sourceRootDir'*.h'
   targetDir     = targetRootDir'\include'
   call SysMkDir targetDir          -- create bin directory for zip archive
   cmd = "copy" quote(source) targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "doc" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"\rexxref.pdf", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexxref.pdf" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "\"
   source        = sourceRootDir'*'
   targetDir     = targetRootDir'\doc'
   call SysMkDir targetDir          -- create bin directory for zip archive
   cmd = "copy" quote(source) targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "samples" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"\arrayCallback.rex", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexxref.pdf" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "\"
   source        = sourceRootDir'*'
   targetDir     = targetRootDir'\samples'
   call SysMkDir targetDir          -- create bin directory for zip archive
   cmd = "xcopy /s" quote(source) targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd


/* Unix version: create the portable zip archive's structure, copy the files from the
   installed sibling directory to the proper zip archive's directories.                   */
::routine setupUnixZipStructure
   use strict arg zipDir, zipName, op_sys, strDT

   -- ad Unix masks: unfortunately SysMkDir uses DECIMAL values for the mask
   --                rather than the standard octal values; "dirMask" uses the familiar
   --                octal notation that will get used with a "chmod" command
   dirMask="775"  -- full rights for owner, group, read+execute for world

   targetRootDir = zipDir'/'zipName

   call SysMkDir targetRootDir            -- create root directory for zip archive
   address system "chmod" dirMask targetRootDir

      -- create setupoorexx.sh script from resource
   scriptFullPath=targetRootDir"/setupoorexx.sh"
   .stream~new(scriptFullPath)~~open("replace write")                                       -
          ~~lineout(.resources~unix_setup_scripts~toString~changeStr("_HERE_DATE_TIME",strDT)) -
          ~~close
   address system "chmod" dirMask scriptFullPath

-- v option not available on all platforms, removed 2023-02-15 POJ
-- changed "cp -af" with "cp -RpPf" as suggested by POJ
   eCopy="cp -RpPf"

      -- create and copy to the "bin" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"/rexx", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexx" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "/"
   source        = quote(sourceRootDir)'*'
   targetDir     = targetRootDir'/bin'
   call SysMkDir targetDir          -- create bin directory for zip archive
   address system "chmod" dirMask targetDir
   cmd = eCopy source targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "lib" directory, locating the installed directory with SysFileTree()
   if op_sys='D' then   -- Apple's Darwin
      call sysFileTree zipDir"/librexxapi.dylib", "files.", "FOS"
   else
      call sysFileTree zipDir"/librexxapi.so", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexxapi" shared object/dynamic library in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "/"
   source        = quote(sourceRootDir)'*'
   targetDir     = targetRootDir'/lib'
   call SysMkDir targetDir          -- create bin directory for zip archive
   address system "chmod" dirMask targetDir
   cmd = eCopy source targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "include" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"/oorexxapi.h", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "oorexxapi.h" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "/"
   source        = quote(sourceRootDir)'*.h'
   targetDir     = targetRootDir'/include'
   call SysMkDir targetDir          -- create bin directory for zip archive
   address system "chmod" dirMask targetDir
   cmd = eCopy source targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "doc" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"/rexxref.pdf", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexxref.pdf" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "/"
   source        = quote(sourceRootDir)'*'
   targetDir     = targetRootDir'/doc'
   call SysMkDir targetDir          -- create bin directory for zip archive
   address system "chmod" dirMask targetDir
   cmd = eCopy source targetDir
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

      -- create and copy to the "samples" directory, locating the installed directory with SysFileTree()
   call sysFileTree zipDir"/arrayCallback.rex", "files.", "FOS"
   if files.0 = 0 then raise syntax 40.900 array ('cannot find "rexxref.pdf" in any subdirectory of "'zipDir'"')
   sourceRootDir=filespec("location",files.1) -- note contains trailing "/"
   source        = quote(sourceRootDir)'*'
   targetDir     = targetRootDir'/samples'
   call SysMkDir targetDir          -- create bin directory for zip archive
   address system "chmod" dirMask targetDir
   cmd = eCopy "-R" source targetDir  -- recursively
   if .bDebug=.true then say .leadin "cmd"~left(.lw,".")":" pp(cmd)
   address system cmd

/* Quotes the argument in double quotes (meant for paths that may contain blanks).  */
::routine quote
   return '"' || arg(1)~changestr('"','""') || '"'

/* Create "readme.txt" file from resource, adjust it to the current operating system.  */
::routine make_readme
  use strict arg zipRexxHome, op_sys

   -- supply samples that can be executed right away from the archive's directory
  sample1="rexx testoorexx.rex"
  sample2pgm="runRexxProgram" -- this program is an executable linked to librexx.{so|dylib}/librexxapi.{so|dylib}
  sample2="samples\api\c++\callsample\"sample2pgm "testoorexx.rex"

  if op_sys='W' then
  do
     sample2needle=zipRexxHome"\"sample2pgm".exe"
     fullName=zipRexxHome"\readme.txt"
  end
  else
  do
     sample2needle=zipRexxHome"/"sample2pgm
     fullName=zipRexxHome"/readme.txt"
  end

  call sysFileTree sample2needle, "files.", "FOS"
  if files.0>0 then  -- found, create relative path to it
  do
     pos=files.1~caselessPos(zipRexxHome)
     pos+=zipRexxHome~length+1      -- position after slash
     relPath=files.1~substr(pos)    -- extract (relative) path
     if op_sys='W' then
     do
         -- remove program part (thereby we remove also the ".exe" part on Windows)
        lastPos=relPath~lastPos(.rexxinfo~directorySeparator)
        sample2=relPath~left(lastPos) || sample2pgm "testoorexx.rex"
     end
     else
        sample2=relPath "testoorexx.rex"
  end

  s=.stream~new(fullName)~~open("replace write")
  txt=.resources~readme.txt~toString


  if op_sys='W' then
  do
     txt=txt~changeStr("_HERE_SAMPLE1"            , sample1            )
     txt=txt~changeStr("_HERE_SAMPLE2"            , sample2            )
     txt=txt~changeStr("_HERE_SETUP_SCRIPT"       , "setupoorexx.cmd"  )
     txt=txt~changeStr("_HERE_SCRIPT_RXENV"       , "rxenv.cmd"        )
     txt=txt~changeStr("_HERE_SCRIPT_SETENV2RXENV", "setenv2rxenv.cmd" )
     txt=txt~changeStr("_HERE_CMD_RXENV"          , "rxenv"            )
     txt=txt~changeStr("_HERE_CMD_SETENV2RXENV"   , "setenv2rxenv"     )
     txt=txt~changeStr("_HERE_UNIX_SOURCE"        , ""                 )
     txt=txt~changeStr("_HERE_LOCAL_SH"           , ""                 )
     txt=txt~changeStr("_HERE_DARWIN_DYLIB"       , ""                 )
     txt=txt~changeStr("_HERE_SHOW_REXX_HOME"     , "echo %REXX_HOME%" )
  end
  else
  do
     txt=txt~changeStr("_HERE_SAMPLE1"            , sample1~changeStr("\","/") )
     txt=txt~changeStr("_HERE_SAMPLE2"            , sample2~changeStr("\","/") )
     txt=txt~changeStr("_HERE_SETUP_SCRIPT"       , "setupoorexx.sh"   )
     txt=txt~changeStr("_HERE_SCRIPT_RXENV"       , "rxenv.sh"         )
     txt=txt~changeStr("_HERE_SCRIPT_SETENV2RXENV", "setenv2rxenv.sh"  )
     txt=txt~changeStr("_HERE_CMD_RXENV"          , "rxenv.sh"         )
     txt=txt~changeStr("_HERE_CMD_SETENV2RXENV"   , "setenv2rxenv.sh"  )
     txt=txt~changeStr("_HERE_UNIX_SOURCE"        , "source "          )
     txt=txt~changeStr("_HERE_LOCAL_SH"           , "./"               )
     if op_sys="D" then
        txt=txt~changeStr("_HERE_DARWIN_DYLIB"    , "DYLD_LIBRARY_PATH=$REXX_HOME/lib ")
     else
        txt=txt~changeStr("_HERE_DARWIN_DYLIB"    , "")

     txt=txt~changeStr("_HERE_SHOW_REXX_HOME"     , "echo $REXX_HOME"  )
  end
  s~~lineout(txt) ~close

  if op_sys<>"W" then      -- Unix: change mode accoringly
     address system "chmod 664" fullName

  return txt

/* ============================================================================ */

/* This Windows script will be saved in the zipdir\zipname directory, such that
   it gets zipped up as well and allows the user to temporarily set REXX_HOME,
   PATH, INCLUDE and LIB to the respective subdirectories.
*/
::RESOURCE windows_setup_scripts
@echo off
rem ----------------------------------------------------------------------------
rem
rem Copyright (c) 2021-2023 Rexx Language Association. All rights reserved.
rem
rem This program and the accompanying materials are made available under
rem the terms of the Common Public License v1.0 which accompanies this
rem distribution. A copy is also available at the following address:
rem https://www.oorexx.org/license.html                         */
rem
rem Redistribution and use in source and binary forms, with or
rem without modification, are permitted provided that the following
rem conditions are met:
rem
rem Redistributions of source code must retain the above copyright
rem notice, this list of conditions and the following disclaimer.
rem Redistributions in binary form must reproduce the above copyright
rem notice, this list of conditions and the following disclaimer in
rem the documentation and/or other materials provided with the distribution.
rem
rem Neither the name of Rexx Language Association nor the names
rem of its contributors may be used to endorse or promote products
rem derived from this software without specific prior written permission.
rem
rem THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
rem "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
rem LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
rem FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
rem OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
rem SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
rem TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
rem OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
rem OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
rem NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
rem SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
rem
rem ----------------------------------------------------------------------------

rem generated: _HERE_DATE_TIME
echo creating rxenv.cmd and setenv2rxenv.cmd ...
rem use rexx.exe of this unzipped archive
bin\rexx.exe setupoorexx.rex
::END

/* ============================================================================ */

::RESOURCE unix_setup_scripts    -- hashbang and CPL license
#!/bin/sh
# ----------------------------------------------------------------------------
#
# Copyright (c) 2021-2023 Rexx Language Association. All rights reserved.
#
# This program and the accompanying materials are made available under
# the terms of the Common Public License v1.0 which accompanies this
# distribution. A copy is also available at the following address:
# https://www.oorexx.org/license.html                         */
#
# Redistribution and use in source and binary forms, with or
# without modification, are permitted provided that the following
# conditions are met:
#
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the distribution.
#
# Neither the name of Rexx Language Association nor the names
# of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ----------------------------------------------------------------------------

# generated: _HERE_DATE_TIME
echo creating rxenv.sh and setenv2rxenv.sh ...
bin/rexx setupoorexx.rex

::END

/* ============================================================================ */

::RESOURCE readme.txt
====================================================================
Portable Zip Archive ("Stick") Version for Open Object Rexx (ooRexx)
====================================================================

"Open object Rexx" (ooRexx) is an open-source project governed by the
non-profit special interest group "Rexx Language Association" (RexxLA). It
is an easy to learn, dynamically typed, caseless and powerful programming
language. ooRexx implements the message paradigm that makes it easy to
interact with any type of system.

This version allows you to test and use ooRexx without a need to install it
and to carry along with additional ooRexx versions for different operating
systems e.g. on a single (USB) "stick", hence "portable".

Links:

   RexxLA Homepage: http://www.rexxla.org
   ooRexx Homepage: http://www.oorexx.org
   ooRexx Project:  https://sourceforge.net/projects/oorexx/


-------------------
First Step (Setup)
-------------------

First run

       _HERE_LOCAL_SH_HERE_SETUP_SCRIPT

from this directory (usually the portable archive's unzipped directory) which will
run "setupoorexx.rex" using the Rexx interpreter from its "bin" subdirectory.

Running "setupoorexx.rex"  from the archive's unzipped directory will generate the scripts
"_HERE_SCRIPT_RXENV" (REXX_HOME Environment) and "_HERE_SCRIPT_SETENV2RXENV" (set process/shell environment
to REXX_HOME environment) which will contain absolute paths that point to Rexx in its
subdirectory named "bin" and therefore will work from any location as long as the
script's home directory does not get moved.

If the location of the script's home directory has changed simply rerun the
"_HERE_SETUP_SCRIPT" script from the new location.


------------------------------------------------------------------
Purpose and usage of "_HERE_SCRIPT_RXENV" (portable REXX_HOME Environment)
------------------------------------------------------------------

   This script will set up a local environment to this directory's Rexx interpreter
   in the current process and expects a program like "rexx" with arguments, e.g.

       _HERE_LOCAL_SH_HERE_CMD_RXENV _HERE_SAMPLE1
       _HERE_LOCAL_SH_HERE_CMD_RXENV _HERE_SAMPLE2

   Upon return from the process the shell's environment is unchanged.


--------------------------------------------------------------------------------------------
Purpose and usage of "_HERE_SCRIPT_SETENV2RXENV" (Set Environment to portable REXX_HOME Environment)
--------------------------------------------------------------------------------------------

   This script allows to set up the environment to this directory's Rexx interpreter
   in the current shell:

       _HERE_UNIX_SOURCE_HERE_LOCAL_SH_HERE_CMD_SETENV2RXENV


   Upon return from the process the shell's environment is changed. This will cause
   using that particular Rexx interpreter and expects a program like "rexx" with
   arguments, e.g.

       _HERE_SAMPLE1
       _HERE_DARWIN_DYLIB_HERE_SAMPLE2

   To locate the Rexx directory being used inspect the environment variable REXX_HOME, e.g.

       _HERE_SHOW_REXX_HOME
::END


#!bin/rexx
-- note: the above hashbang is intentional: will run the archive's bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2021-2022 Rexx Language Association. All rights reserved.    */
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
/* purpose:    create rxenv.{cmd|sh} and setenv2rxenv.{cmd|sh}
               - setup and usage should be the same on all operating systems

               - will contain absolute PATHs to the zip archive's directory/ies
                 - REXX_HOME ... absolute path to the archive's root directory
                 - PATH ... will get REXX_HOME/bin prepended
                 - INCLUDE (Windows), CPATH (Unix) ... will get REXX_HOME/include prepended
                 - LIB (Windows) ... will get REXX_HOME/lib prepended
                 - Unix: LD_LIBRARY_PATH=%REXX_HOME%/lib
                 - Apple: DYLD_LIBRARY_PATH=%REXX_HOME%/lib

               - running setupoorexx.exe needs to be done in the root of the zip
                 archive directory that contains the directories "bin", "lib", etc.

               - the created scripts rxenv.{cmd|sh} and rxenv2env.{cmd|sh}
                 can remain in place or may be copied to a directory that
                 is on the PATH such that they can be found from anywhere

               - rxenv.{cmd|sh} ("Rexx Home ENVironment")
                 - changes to the environment are local to the script while it
                   is running; upon termination the environment is unchanged

               - rxenv2env.{cmd|sh} (set environment to Rexx Home ENVironment)

                 - exports the environment variables REXX_HOME, PATH, INCLUDE
                   (on Unix CPATH), LIB (Windows) or LD_LIBRARY_PATH (Linux) or
                   DYLD_LIBRARY_PATH (Apple)

                   - note: on Apple DYLD_LIBRARY_PATH would get deleted before
                           running a command in a new process; hence prepend in
                           the command directly with DYLD_LIBRARY_PATH=$REXX_HOME
                           to remain in effect

   date:    - 2022-01-30, rgf: change rhenv to rxenv
*/

parse source op_sys +1 . . thisPgm
zipRexxHome=filespec('location',thisPgm)           -- get full path to location
zipRexxHome=zipRexxHome~left(zipRexxHome~length-1) -- remove trailing slash

dt=.dateTime~new~string
say "creating scripts ..."
do scriptName over ("rxenv", "setenv2rxenv")   -- note: scriptnames are also used for resource names!
   say "   creating" pp(scriptName) "..."
   call make_script scriptName, zipRexxHome, op_sys, dt
end
say "done."
say
   -- display readme.txt file
if op_sys='W' then "type readme.txt"
              else "cat readme.txt"

::routine make_script
  use strict arg scriptName, zipRexxHome, op_sys, dt

  if op_sys='W' then fullName=zipRexxHome"\"scriptName".cmd"
                else fullName=zipRexxHome"/"scriptName".sh"

  s=.stream~new(fullName)~~open("replace write")

  if op_sys='W' then
  do
     s~arrayout(.resources~windows_leadin)
     script_code=.resources~entry("windows_"scriptName)~toString
     script_code=script_code~changestr("_CMD_RXENV", scriptName)
  end
  else
  do
     s~arrayout(.resources~unix_leadin)
     script_code=.resources~entry("unix_"scriptName)~toString
     if op_sys="D" then       -- Apple's Darwin
     do
        script_code=script_code~changestr("_HERE_LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH")
        script_code=script_code~changestr("_HERE_FILLER"         , ".."               )
     end
     else
     do
        script_code=script_code~changestr("_HERE_LD_LIBRARY_PATH", "LD_LIBRARY_PATH"  )
        script_code=script_code~changestr("_HERE_FILLER"         , ""                 )
     end

     script_code=script_code~changestr("_HERE_SCRIPTNAME", fullName)
     script_code=script_code~changestr("_CMD_RXENV", scriptName".sh")

  end
  script_code=script_code~changestr("_HERE_DATETIME"     , dt) -
                         ~changestr("_HERE_REXX_HOME", zipRexxHome)

  s~~lineout(script_code)~~close
  if op_sys<>"W" then      -- Unix: change mode accoringly
     address system "chmod 775" fullName


::routine quote      -- quote argument
  return '"' || arg(1) || '"'

::routine pp         -- "pretty print": enclose in square brackets
  return '[' || arg(1) || ']'


::RESOURCE windows_rxenv
@echo off
@rem created: _HERE_DATETIME
@rem purpose: run programs and have them use ooRexx from %REXX_HOME%'s 'bin' folder
@rem example: _CMD_RXENV rexx somePgm.rex someargs
setlocal
set REXX_HOME=_HERE_REXX_HOME
set PATH=%REXX_HOME%\bin;%PATH%
set LIB=%REXX_HOME%\lib;%LIB%
set INCLUDE=%REXX_HOME%\include;%INCLUDE%

rem %0 is this script, %1 the program to run, %2 the first argument for it, ...
%1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
::END

::RESOURCE windows_setenv2rxenv
@echo off
@rem created: _HERE_DATETIME
@rem purpose: set environment to use ooRexx from %REXX_HOME%'s 'bin' directory

echo %~nx0: setting environment variables REXX_HOME, PATH, INCLUDE, LIB
set  REXX_HOME=_HERE_REXX_HOME
echo REXX_HOME=%REXX_HOME%

set  PATH=%REXX_HOME%\bin;%PATH%
echo PATH .. now points first to "%%REXX_HOME%%\bin"

set  INCLUDE=%REXX_HOME%\include;%INCLUDE%
echo INCLUDE now points first to "%%REXX_HOME%%\include"

set  LIB=%REXX_HOME%\lib;%LIB%
echo LIB ... now points first to "%%REXX_HOME%%\lib"

echo done.

::END

::RESOURCE unix_rxenv
# created: _HERE_DATETIME
# purpose: run programs and have them use ooRexx from %REXX_HOME%'s 'bin' directory
# example: _CMD_RXENV rexx somePgm.rex someargs
REXX_HOME=_HERE_REXX_HOME
PATH=$REXX_HOME/bin:$PATH
_HERE_LD_LIBRARY_PATH=$REXX_HOME/lib:$_HERE_LD_LIBRARY_PATH
CPATH=$REXX_HOME/include:$CPATH

# Unix allows to prefix environment variables for the command
# $0 is this script, $1 the program to run, $2 the first argument for it, ...
_HERE_LD_LIBRARY_PATH=$_HERE_LD_LIBRARY_PATH $1 $2 $3 $4 $5 $6 $7 $8 $9
::END

::RESOURCE unix_setenv2rxenv
# created: _HERE_DATETIME
# purpose: set and export environment to use ooRexx from %REXX_HOME%'s 'bin' directory
# example: source _HERE_SCRIPTNAME

echo $0: setting environment variables REXX_HOME, PATH, CPATH , _HERE_LD_LIBRARY_PATH
export REXX_HOME=_HERE_REXX_HOME
echo REXX_HOME=$REXX_HOME

export PATH=$REXX_HOME/bin:$PATH
echo PATH _HERE_FILLER.......... now points first to "\$REXX_HOME/bin"

export CPATH=$REXX_HOME/include:$CPATH
echo CPATH _HERE_FILLER......... now points first to "\$REXX_HOME/include"

export _HERE_LD_LIBRARY_PATH=$REXX_HOME/lib:$_HERE_LD_LIBRARY_PATH
echo _HERE_LD_LIBRARY_PATH now points first to "\$REXX_HOME/lib"

echo done.

::END


::RESOURCE windows_leadin     -- CPL license
@rem ----------------------------------------------------------------------------
@rem
@rem Copyright (c) 2021 Rexx Language Association. All rights reserved.
@rem
@rem This program and the accompanying materials are made available under
@rem the terms of the Common Public License v1.0 which accompanies this
@rem distribution. A copy is also available at the following address:
@rem https://www.oorexx.org/license.html                         */
@rem
@rem Redistribution and use in source and binary forms, with or
@rem without modification, are permitted provided that the following
@rem conditions are met:
@rem
@rem Redistributions of source code must retain the above copyright
@rem notice, this list of conditions and the following disclaimer.
@rem Redistributions in binary form must reproduce the above copyright
@rem notice, this list of conditions and the following disclaimer in
@rem the documentation and/or other materials provided with the distribution.
@rem
@rem Neither the name of Rexx Language Association nor the names
@rem of its contributors may be used to endorse or promote products
@rem derived from this software without specific prior written permission.
@rem
@rem THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
@rem "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
@rem LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
@rem FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
@rem OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
@rem SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
@rem TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
@rem OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
@rem OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
@rem NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
@rem SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@rem
@rem ----------------------------------------------------------------------------

::END

::RESOURCE unix_leadin        -- hashbang and CPL license
#!/bin/sh
# ----------------------------------------------------------------------------
#
# Copyright (c) 2021 Rexx Language Association. All rights reserved.
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

::END


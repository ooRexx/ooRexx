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
            - 2024-05-05, rgf: add packages to PATH to allow oorexxshell et.al. to be found
            - 2024-05-09, rgf: - honor packages/bsf4oorexx and packages/dbus4oorexx if present
                               - on Windows use "more" instead of "type", on Unix "less" instead
                                 of "cat" to display the created readme file
            - 2024-05-27, rgf: - set JAVA_HOME, if not set but java executable on PATH, give
                                 an informative error message if Java has wrong bitnes
                               - do not force readme.txt to be shown, just point at it for
                                 further information
            - 2024-05-28, rgf: - on Unix fix typo and use '=' to assign value to environment variable
                               - added some informative output to .error in getJavaProperties()
                               - changed the logic to allow to use this script also for installed
                                 ooRexx versions: unzip the packages zip archive and simply run
                                 "setupoorexx.rex"
            - 2024-05-29, rgf: - if an error in the redirected command, create a more informative
                                 error message
                               - remove debug output
            - 2024-05-30, rgf: - added 'export' to rxenv.sh for WLS to work (seems a subshell gets used there)
                               - added environment variable "OOREXX_HOME"
                                 - portable and Unix: the directory in which bin, lib exist
                                 - Windows: the directory the binaries are located in, unless
                                                           portable version
                               - added environment variable "PORTABLE_OOREXX" in case OOREXX_HOME
                                 has a subdirectory named "bin" and on Unix in addition the path
                                 does not start with "/usr"
                               - if using on genuine portable ooRexx, then no packages directory
                                 will be present, hence do not set PACKAGES_HOME (as it would be empty)
                               - tidy up code a little bit
            - 2024-06-01, rgf: - fixing two occasions of .error~errMsg(errMsag) (to .error~say(errMsg)
                               - reduce multiple empty lines
*/

parse source op_sys +1 . . thisPgm
zipHomeDir=filespec('location',thisPgm)           -- get full path to location
zipHomeDir=zipHomeDir~left(zipHomeDir~length-1) -- remove trailing slash
oldDir=directory(zipHomeDir) -- make that location the current directory (if invoked from a different location)

-- op_sys="D"     -- for debug only

.context~package~local~bDebug=.false      -- .true will cause some debug info to be shown

dt=.dateTime~new~string
packageConfigs=getPackagesConfigs(op_sys,zipHomeDir)

   -- try to set JAVA_HOME, if not yet set

java_home_value =value("JAVA_HOME",,"ENVIRONMENT")  -- get JAVA_HOME environment variable if any
if java_home_value="" then     -- no JAVA_HOME value, try to get it from Java's System properties
do
    st=getJavaProperties()
    if st<>.nil then     -- check for correct bitness and get its java.home property value
    do
       if st["sun.arch.data.model"]<>.rexxinfo~architecture then
       do
          errMsg="Java bitness ["st["sun.arch.data.model"]"] does not match ooRexx architecture [".rexxinfo~architecture"], cannot set JAVA_HOME"
          .error~say(errMsg)
       end
       else  -- o.k., we use JAVA_HOME from the System property
       do
          java_home_value=st["java.home"]
       end
    end
end
else  -- if JAVA_HOME exists, do not change it
do
   java_home_value=""
end

say "creating scripts ..."
do scriptName over ("rxenv", "setenv2rxenv")   -- note: scriptnames are also used for resource names!
   say "   creating" pp(scriptName) "..."
   call make_script scriptName, zipHomeDir, op_sys, dt, packageConfigs, java_home_value
end
say "   .... see" pp('readme.txt') "for more information"
say "done."

::routine make_script
  use strict arg scriptName, zipHomeDir, op_sys, dt, packageConfigs, java_home_value

  if op_sys='W' then fullName=zipHomeDir"\"scriptName".cmd"
                else fullName=zipHomeDir"/"scriptName".sh"

  s=.stream~new(fullName)~~open("replace write")


  if op_sys='W' then    -- Windows
  do
     s~arrayout(.resources~windows_leadin)
     script_code=.resources~entry("windows_"scriptName)~toString
     script_code=script_code~changestr("_CMD_RXENV", scriptName)
  end
  else                  -- Unix
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

  /* ------------------------------------------------------------------------- */
  /* 20240528: the default layout is that of the portable ooRexx zip archives from Sourceforge:
            - zipHomeDir    : location of the setup scripts and packages, if present
            - zipHomeDir/bin: location of the executable (.rexxInfo~executable~parentFile), hence
                               _REXX_BIN_="/bin" or "\bin"
            - zipHomeDir/lib: location of the libraries/dlls (.rexxInfo~libraryPath), hence
                               _REXX_LIBRARY_PATH_=REXX_HOME"/lib" or "\lib"

     If the layout got changed and ooRexx is used from a different location, then
            - use REXX_HOME as (.rexxInfo~executable~parentFile) and set _REXX_BIN_ to ""
            - use REXX_LIBRARY_PATH as (.rexxInfo~libraryPath) and set _REXX_BIN_ to ""
  */
  dirSep =.rexxInfo~directorySeparator    -- either / (Unix) or \ (Windows)

  tmpHomeOfRexxBin=dirSep || "bin"     -- replaces _REXX_BIN_
  tmpHomeOfRexxLib=dirSep || "lib"     -- replaces _REXX_LIB_
  tmpHomeOfRexxInc=dirSep || "include" -- replaces _REXX_INC_

  tmpHomeOfRexx=.rexxInfo~executable~parentFile -- get directory
  tmpRexx_Home=value("REXX_HOME",,"ENVIRONMENT")   -- get REXX_HOME (Windows) if any
  if tmpRexx_Home="" then        -- REXX_HOME does not exist, define it in the scripts
     tmpRexx_home=tmpHomeOfRexx

  tmpPortableOORexx = ""

  if tmpHomeOfRexx~absolutePath=zipHomeDir || dirSep || "bin" then   -- assume portable
  do
     if .bDebug=.true then say .line "in portable directory ..."
     tmpHomeOfRexx=zipHomeDir

     tmpPortableOORexx = (op_sys="W")~?("set","export") "PORTABLE_OOREXX=1" -- environment variable
     if op_sys<>"W", tmpHomeOfRexx~startsWith("/usr") then
         tmpPortableOORexx = ""  -- assuming a proper Unix installation, hence not portable
  end
  else   -- assume installed ooRexx
  do
     if .bDebug=.true then say .line "in INSTALLED directory ..."
     if op_sys="W" then
     do
        if .bDebug=.true then say .line "in INSTALLED directory on WINDOWS ..."
        tmpHomeOfRexxBin=""               -- replaces _REXX_BIN_
        tmpHomeOfRexxLib=""               -- replaces _REXX_LIB_
        tmpHomeOfRexxInc=dirSep || "api"  -- replaces _REXX_INC_
     end
     else -- Unix
     do
     end
  end
  /* ------------------------------------------------------------------------- */


  script_code=script_code~changestr("_HERE_DATETIME"     , dt)               -
                         ~changestr("_HERE_REXX_HOME"    , tmpRexx_home    ) -
                         ~changestr("_HERE_OOREXX_HOME"  , tmpHomeOfRexx   ) -
                         ~changestr("_REXX_BIN_"         , tmpHomeOfRexxBin) -
                         ~changestr("_REXX_LIB_"         , tmpHomeOfRexxLib) -
                         ~changestr("_REXX_INC_"         , tmpHomeOfRexxInc)


  script_code=script_code~changestr("_HERE_PORTABLE_OOREXX", tmpPortableOORexx)

  tmpPackagesHome=packageConfigs~packages_home
  if tmpPackagesHome<>"" then
     tmpPackagesHome=(op_sys="W")~?("set","export") "PACKAGES_HOME="packageConfigs~packages_home

  script_code=script_code~changeStr("_HERE_PACKAGES_HOME_"     , tmpPackagesHome)             -
                         ~changeStr("_HERE_PACKAGES_PATH_"     , packageConfigs~path)         -
                         ~changeStr("_HERE_PACKAGES_LIB_"      , packageConfigs~library_path)

  tmpCP=packageConfigs~classpath -- note this value has the trailing pathSeparator set
  if tmpCP<>"" then  -- bsf4oorexx present, need to set CLASSPATH
  do
     if op_sys="W" then
        tmpCP="set CLASSPATH="tmpCP
     else
        tmpCP="export CLASSPATH="tmpCP

     if value("CLASSPATH",,"ENVIRONMENT")<>"" then -- CLASSPATH set, we should honor it
     do
        if op_sys="W" then
           tmpCP=tmpCP"%CLASSPATH%"
        else
           tmpCP=tmpCP"$CLASSPATH"
     end
     if scriptName="setenv2rxenv" then
     do
        tmpCP=tmpCP .rexxInfo~endOfLine || "echo CLASSPATH="
        if op_sys="W" then tmpCP=tmpCP"%CLASSPATH%"
                      else tmpCP=tmpCP"$CLASSPATH"
     end

     -- oorexxshell defined explicitly UTF-8 (which since has become the default encoding of Java),
     -- set BSF4Rexx_JavaStartupOptions accordingly such that earlier Java versions behave the same
     needle        ="BSF4Rexx_JavaStartupOptions"
     needleValue   ="-Dsun.jnu.encoding=UTF-8 -Dfile.encoding=UTF-8"
     envNeedleValue=value(needle,,"ENVIRONMENT")  -- if exists, get it

     if envNeedle<>"" then -- honor existing BSF4Rexx_JavaStartupOptions
        needleValue=getJavaStartupOptions(envNeedleValue, needleValue)

         -- :: Set default encoding to UTF-8 for command line, environment variables and text file contents
         -- set BSF4Rexx_JavaStartupOptions=-Dsun.jnu.encoding=UTF-8 -Dfile.encoding=UTF-8
     -- on Unix value must be under double-quotes
     if op_sys="W" then tmpNeedle="set" needle"="needleValue
                   else tmpNeedle=needle'="'needleValue'"' -- on Unix value must be put under double-quotes

     if op_sys<>"W" then  -- Unix
        tmpNeedle="export" tmpNeedle

      -- add newline and BSF4Rexx_JavaStartupOptions
     tmpCP=tmpCP .rexxInfo~endOfLine || tmpNeedle

     if scriptName="setenv2rxenv" then
     do
        tmpCP=tmpCP .rexxInfo~endOfLine || "echo BSF4Rexx_JavaStartupOptions="
        if op_sys="W" then tmpCP=tmpCP"%BSF4Rexx_JavaStartupOptions%"
                      else tmpCP=tmpCP"$BSF4Rexx_JavaStartupOptions"
     end
  end

  script_code=script_code~changeStr("_HERE_PACKAGES_CLASSPATH_" , tmpCP)

   -- set JAVA_HOME
  if java_home_value<>"" then    -- Unix, we need to export
  do
     if op_sys="W" then java_home_value="set JAVA_HOME="java_home_value
                   else java_home_value="export JAVA_HOME="java_home_value
  end
     -- java_home_value will be the empty string, if JAVA_HOME is already defined in the environment
  script_code=script_code~changeStr("_HERE_JAVA_HOME_", java_home_value)

   -- reduce multiple empty lines
  eol=.rexxInfo~endOfLine
  eol2=eol~copies(2)
  eol3=eol~copies(3)
  script_code=script_code~changeStr(eol3,eol2)

  s~~lineout(script_code)~~close
  if op_sys<>"W" then      -- Unix: change mode accoringly
     address system "chmod 775" fullName
  exit

-- do not add Java startup options if already present
getJavaStartupOptions: Procedure
  use strict arg envNeedleValue, needleValue

  arr=needleValue~makeArray(" ")    -- split at blank
  do val over arr
     if envNeedleValue~wordPos(val)>0 then iterate -- skip, already defined
     envNeedleValue=val envNeedleValue -- prepend to existing value
  end
  return envNeedleValue~strip -- return value to use


::routine quote      -- quote argument
  return '"' || arg(1) || '"'

::routine pp         -- "pretty print": enclose in square brackets
  return '[' || arg(1) || ']'


/** Check for the existence of a "packages" subdirectory hinting at oorexxshell
*   and other packages (bsf4oorexx, dbus4oorexx) being present. If so create PATH,
*   LD_LIBRARY_PATH or DYLD_LIBRARY_PATH paths to be added to the respective
*   environment settings, returning a directory with the entries CLASSPATH, PATH and
*   LIBRARY_PATH.
*   If a subdirectory "packages" is not present, then returns a directory with
*   CLASSPATH, PATH and LIBRARY_PATH being set to an empty string.
*
*   @return a directory with the entries CLASSPATH, PATH and LIBRARY_PATH set accordingly
*
*/
::routine getPackagesConfigs
   use strict arg op_sys, zipHomeDir -- portable directory

   dirSep =.rexxInfo~directorySeparator   -- either / (Unix) or \ (Windows)
   pathSep=.rexxInfo~pathSeparator        -- either : (Unix) or ; (Windows)

      -- default to empty strings
   resDir=.directory~of( ("CLASSPATH",""), ("PATH",""), ("LIBRARY_PATH",""), ("PACKAGES_HOME","") )

   pkgDir=zipHomeDir || dirSep"packages"

   if \sysfileexists(pkgDir) then   -- no packages directory
      return resDir

      -- o.k. "packages" exists, check and add
   resDir~packages_home=pkgDir

   logPkgDir=(op_sys="W")~?("%PACKAGES_HOME%","$PACKAGES_HOME")
   tmpPath=logPkgDir

   tmpLib =""        -- populate on Unix
   tmpCP=""          -- CLASSPATH (bsf4oorexx)

   tmpBinSubdir=dirSep || "bin"
   tmpLibSubdir=dirSep || "lib"
   tmpAsterisk =dirSep || "*"

   if sysFileExists(pkgDir || dirSep || "dbus4oorexx") then   -- dbus4oorexx
   do
      dbusDir=logPkgDir || dirSep || "dbus4oorexx"
      tmpPath=tmpPath || pathSep || dbusDir || tmpBinSubdir    -- add to PATH
      if op_sys="W" then   -- Windows
         tmpPath=tmpPath || pathSep || dbusDir || tmpLibSubdir
      else  -- Unix
         tmpLib =dbusDir || tmpLibSubdir -- add to LIBRARY_PATH
   end

   if sysFileExists(pkgDir || dirSep || "bsf4oorexx") then   -- dbus4oorexx
   do
      b4rDir=logPkgDir || dirSep || "bsf4oorexx"
      tmpPath=tmpPath || pathSep || b4rDir  || tmpBinSubdir   -- add to PATH

      if op_sys="W" then   -- Windows
         tmpPath=tmpPath || pathSep || b4rDir || tmpLibSubdir
      else  -- Unix
      do
         tmpLib =dbusDir || tmpLibSubdir -- add to LIBRARY_PATH

         if tmpLib<>"" then tmpLib=tmpLib || pathSep
         tmpLib=tmpLib || b4rDir || tmpLibSubdir -- add to LIBRARY_PATH
      end

      -- we place the jar/zip files in the same subdirectory as the so/dylibs
      tmpCP=b4rDir || tmpLibSubdir || tmpAsterisk || pathSep || '.' || pathSep -- || dirSep
      if op_sys="W" then tmpCP=tmpCP || "%USERPROFILE%\BSF4ooRexx\lib"
                    else tmpCP=tmpCP || "$HOME/BSF4ooRexx/lib"
      tmpCP=tmpCP || tmpAsterisk
   end

   -- supply path separator if a value present
   if tmpPath<>"" then tmpPath=tmpPath || pathSep
   if tmpLib<>""  then tmpLib =tmpLib  || pathSep
   if tmpCP <>""  then tmpCP  =tmpCP   || pathSep

   if .bdebug=.true then
   do
      say
      thisName=.context~name
      say .line "("thisName"):" "tmpPath="pp(tmpPath)
      say .line "("thisName"):" "tmpLib ="pp(tmpLib)
      say .line "("thisName"):" "tmpCP  ="pp(tmpCP)
      say
   end

   resDir~path        =tmpPath
   resDir~library_path=tmpLib
   resDir~classpath   =tmpCP
   return resDir


/** Tries to use BSF4ooRexx' <code>JavaInfo4BSF4RexxInstallation.class</cdoe>
*   from packages/bsf4oorexx/bin/utilities
*   to get to Java's System properties in case JAVA_HOME is not set, but the Java
*   executable is available via PATH.
*
*  @return returns a StringTable with all Java System properties or <code>.nil</code>
*          if Java could not be queried
*/
::routine getJavaProperties
   signal on syntax
   call sysFileTree "JavaInfo4BSF4RexxInstallation.class", "file.", "FOS"
   if file.0=0 then
   do
      errMsg="line #" .line": getJavaProperties(): cannot find 'JavaInfo4BSF4RexxInstallation.class' to query for JAVA_HOME"
      .error~say(errMsg)
      return .nil
   end

   -- get location, remove trailing slash
   location=filespec('location',file.1)~strip('Trailing', .rexxInfo~directorySeparator)
   -- get all Java properties and additional (BSF4ooRexx related) properties
   cmd='java -cp "'location'" JavaInfo4BSF4RexxInstallation'
   arr=.array~new             -- array to receive stdout lines
   address system cmd with output using (arr)
   if rc<>0 then     -- not successful, report error
      signal syntax

   st=.stringTable~new        -- StringTable to receive all queried properties
   do line over arr           -- parse all properties, store them
      parse var line key '=[' value
      value=value~strip('trailing',"]")
      st[key]=value
   end
   return st

syntax:     -- whatever has happened, we cannot supply the desired information
   errMsg="line #" .line": getJavaProperties(): error while attempting to run 'java' occurred, cannot set JAVA_HOME"
   .error~say(errMsg)
   return .nil



/* --
-- remove duplicate entries from paths
::routine cleanPath
   use strict arg op_sys, path

if .bdebug=.true then say "***" .line "(".context~name"): received path=["path"]" "***"
   srcPath=path~makeArray((op_sys="W")~?(";",":")) -- turn path into array
   tgtPath=.array~new
   set=.set~new      -- used to see whether we saw that path already
   do counter c1 p over srcPath
      if set~hasIndex(p) | set~hasIndex(propCase(op_sys,p)) then
      do
         say "... skipping duplicate path #" c1~right(2)": ["p"]"
         iterate
      end
      tgtPath~append(p)
      set~~put(p)~~put(propCase(op_sys,p))
   end

   resStr=tgtPath~makeString('L', (op_sys="W")~?(";",":"))
if .bdebug=.true then say .line "(".context~name"): received path =["path"]"
if .bdebug=.true then say .line "(".context~name"): returning path=["resStr"]"
   return resStr

propCase: procedure
   use strict arg op_sys, name
   if op_sys="W" then return name~upper
   return name
-- */



::RESOURCE windows_rxenv
@echo off
@rem created: _HERE_DATETIME
@rem purpose: run programs and have them use ooRexx from %OOREXX_HOME%'s _REXX_BIN_ folder
@rem example: _CMD_RXENV rexx somePgm.rex someargs
setlocal
set REXX_HOME=_HERE_REXX_HOME
set OOREXX_HOME=_HERE_OOREXX_HOME
_HERE_PORTABLE_OOREXX
_HERE_PACKAGES_HOME_
set PATH=%OOREXX_HOME%_REXX_BIN_;_HERE_PACKAGES_PATH_%PATH%
set LIB=%OOREXX_HOME%_REXX_LIB_;_HERE_PACKAGES_LIB_%LIB%
set INCLUDE=%OOREXX_HOME%_REXX_INC_;%INCLUDE%
_HERE_JAVA_HOME_
_HERE_PACKAGES_CLASSPATH_

rem %0 is this script, %1 the program to run, %2 the first argument for it, ...
%1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
::END

::RESOURCE windows_setenv2rxenv
@echo off
@rem created: _HERE_DATETIME
@rem purpose: set environment to use ooRexx from %OOREXX_HOME%'s _REXX_BIN_ directory

echo %~nx0: setting environment variables REXX_HOME, OOREXX_HOME, PATH, INCLUDE, LIB
set  REXX_HOME=_HERE_REXX_HOME
echo REXX_HOME=%REXX_HOME%

set  OOREXX_HOME=_HERE_OOREXX_HOME
echo OOREXX_HOME=%OOREXX_HOME%

_HERE_PORTABLE_OOREXX
_HERE_PACKAGES_HOME_

_HERE_JAVA_HOME_
_HERE_PACKAGES_CLASSPATH_

set  PATH=%OOREXX_HOME%_REXX_BIN_;_HERE_PACKAGES_PATH_%PATH%
echo PATH .. now points first to "%%OOREXX_HOME%%_REXX_BIN_"

set  INCLUDE=%OOREXX_HOME%_REXX_INC_;%INCLUDE%
echo INCLUDE now points first to "%%OOREXX_HOME%%_REXX_INC_"

set  LIB=%OOREXX_HOME%_REXX_LIB_;_HERE_PACKAGES_LIB_%LIB%
echo LIB ... now points first to "%%OOREXX_HOME%%_REXX_LIB_"

echo done.

::END

::RESOURCE unix_rxenv
# created: _HERE_DATETIME
# purpose: run programs and have them use ooRexx from %OOREXX_HOME%'s _REXX_BIN_ directory
# example: _CMD_RXENV rexx somePgm.rex someargs
export REXX_HOME=_HERE_REXX_HOME
export OOREXX_HOME=_HERE_OOREXX_HOME
_HERE_PORTABLE_OOREXX
_HERE_PACKAGES_HOME_
_HERE_JAVA_HOME_
_HERE_PACKAGES_CLASSPATH_
export PATH=$OOREXX_HOME_REXX_BIN_:_HERE_PACKAGES_PATH_$PATH
export PREPEND_LIBRARY_PATH=$OOREXX_HOME_REXX_LIB_:_HERE_PACKAGES_LIB_$_HERE_LD_LIBRARY_PATH
export _HERE_LD_LIBRARY_PATH=$PREPEND_LIBRARY_PATH
export CPATH=$OOREXX_HOME_REXX_INC_:$CPATH

# Unix allows to prefix environment variables for the command
# $0 is this script, $1 the program to run, $2 the first argument for it, ...
_HERE_LD_LIBRARY_PATH="$_HERE_LD_LIBRARY_PATH" $1 $2 $3 $4 $5 $6 $7 $8 $9
::END

::RESOURCE unix_setenv2rxenv
# created: _HERE_DATETIME
# purpose: set and export environment to use ooRexx from %OOREXX_HOME%'s _REXX_BIN_ directory
# example: source _HERE_SCRIPTNAME

echo $0: setting environment variables REXX_HOME, OOREXX_HOME, PATH, CPATH , _HERE_LD_LIBRARY_PATH
export REXX_HOME=_HERE_REXX_HOME
echo REXX_HOME=$REXX_HOME

export OOREXX_HOME=_HERE_OOREXX_HOME
echo OOREXX_HOME=$OOREXX_HOME

_HERE_PORTABLE_OOREXX
_HERE_PACKAGES_HOME_
_HERE_JAVA_HOME_
_HERE_PACKAGES_CLASSPATH_

export PATH=$OOREXX_HOME_REXX_BIN_:_HERE_PACKAGES_PATH_$PATH
echo PATH _HERE_FILLER.......... now points first to "\$OOREXX_HOME_REXX_BIN_"

export CPATH=$OOREXX_HOME_REXX_INC_:$CPATH
echo CPATH _HERE_FILLER......... now points first to "\$OOREXX_HOME_REXX_INC_"

# on Apple important to allow assigning to DYLD_LIBRARY_PATH in a new shell
# (DYLD_LIBRARY_PATH may be deleted in a new shell due to Apple's SID)
export PREPEND_LIBRARY_PATH=$OOREXX_HOME_REXX_LIB_:_HERE_PACKAGES_LIB_$_HERE_LD_LIBRARY_PATH
export _HERE_LD_LIBRARY_PATH=$PREPEND_LIBRARY_PATH
echo _HERE_LD_LIBRARY_PATH now points first to "\$OOREXX_HOME_REXX_LIB_"

echo done.

::END


::RESOURCE windows_leadin     -- CPL license
@rem ----------------------------------------------------------------------------
@rem
@rem Copyright (c) 2021-2024 Rexx Language Association. All rights reserved.
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
# Copyright (c) 2021-2024 Rexx Language Association. All rights reserved.
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


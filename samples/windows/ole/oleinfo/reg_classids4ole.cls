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

	program: reg_classids4ole.rex

	purpose: analyzes the Windows registry for CLSID-->PROGIDs, defines a class and collections

	usage:   use the "::requires" directive to incorporate classes and routines
***********************************************************************/

.local~rgf.debug=0      -- set debug level

-- initialization
.local ~ rgf.registry = .WindowsRegistry~new            -- create a registry object save it in .local
.local ~ rgf.hKey_classes_root = .rgf.registry~classes_root  -- save handle to root of classes

   -- get handle to key "CLSID" located in top level key "HKEY_CLASSES_ROOT"
.local~rgf.hKey_CLSID_root = .rgf.registry~open(.rgf.hkey_classes_root, "CLSID", "READ")


::requires winsystm.cls  -- get access to the Windows registry


/** Returns .true if argument (a PROGID or CLSID) exists in HKEY_CLASSES_ROOT or HKEY_CLASSES_ROOT\CLSID, .false else.
*/
::routine reg_exists public
   parse arg reg_name

   -- query key (case insensitive lookup by Windows), if not available "0" will be returned as the handle
   hKey = .rgf.registry~open(.rgf.hkey_classes_root, reg_name, "READ")
   if hKey=0 then
   do
      hKey = .rgf.registry~open(.rgf.hkey_classes_root, "CLSID\" || reg_name, "READ")
      if .rgf.debug > 1 then say "reg_exists(): not found in root, looking in CLSID\ ..."
   end

   if .rgf.debug > 1 then say "reg_exists(): reg_name="pp(reg_name) "hKey="pp(hKey)
   if .rgf.debug > 2 then call dump_handle hKey

   .rgf.registry~close(hKey)     -- close (free) handle
   return hKey <> 0




/** Dumps the values and subkey names of the key referred to by the received handle */
::routine dump_handle
   use arg hKey

   say
   s. = .rgf.registry~query(hKey)   -- query key via its handle
   tmp="class name="pp(s.class) "subkeys="pp(s.subkeys) "values="pp(s.values) ,
       "last changed on:" pp(s.date) pp(s.time)
   say tmp

   ind="   "            -- leadin for text-output

   if s.values>0 then   -- show all values of the key, if any
   do
      say; say ind "listing all values for this key:"
      .rgf.registry~listvalues(hKey, stem.)  -- get a list of all values for this key

      do i=1 to s.values
         tmp=ind ind "name:" pp(stem.i.name) "type:" pp(stem.i.type) "data:" pp(stem.i.data)
         say tmp
      end
   end

   if s.subkeys>0 then  -- show names of all subkeys of the key, if any
   do
      say; say ind "listing all subkeys of this key:"
      .rgf.registry~list(hKey, stem.)        -- get a list of all subkeys of this key
      do i=1 to s.subkeys
         tmp=ind ind "subkey name:" pp(stem.i)
         say tmp
      end
   end
   say copies("-", 30)

   return


   /** Returns string value of given argument, enclosed in square brackets: &quot;<strong>P</strong>retty<strong>P</strong>rint&quot; ;) . */
::routine pp
  return "[" || arg(1)~string || "]"



/** Routine expands environment variables in the data. */
::routine expand public
  use arg data

  tmp=""
  do while data <> ""
     parse var data before "%" v "%" data
     tmp = tmp || before || value(v, , "ENVIRONMENT") -- replace environment variable with its value
  end
  return tmp


/** Routine to get the 'CLSID'-object from a given progid or clsid. */
::routine get_clsid_object public
   use arg prog_or_clsid

   clsid2progid=.clsid~clsid2progid -- get access to clsid2porgid[clsid]=progid

   o=clsid2progid[prog_or_clsid]    -- assume CLSID in hand
   if o=.nil then
   do
      o=clsid2progid~allat(prog_or_clsid)[1]  -- oops, maybe a PROGID ?

      if o=.nil then o=create_clsid_object(prog_or_clsid)   -- does not exist as of yet, create it !
   end
   else return .clsid~all_clsid[o]

   return o


/** Routine either gets a PROGID or a CLSID, analyzes it and returns an instance of class CLSID
    or .nil, if PROGID or CLSID not found. */
::routine create_clsid_object public
   use arg clsid, bCLSID   -- it's either a PROGID or a CLSID

   bClsid=(bClsid=.true)  -- determine if already CLSID given
   if \bClsid then         -- undetermined, could be PROGID or CLSID
   do
         -- a PROGID?
      hKey = .rgf.registry~open(.rgf.hkey_classes_root, clsid, "READ")
      if hKey\=0 then   -- a PROGID, get CLSID
      do
         hKey2=.rgf.registry~open(.rgf.hkey_classes_root, clsid"\CLSID")   -- get handle to subkey
         -- 2002-12-29, ---rgf
         if hkey2=0 then   -- subkey "CLSID" not found, maybe "CurVer" pointing to actual PROGID ?
         do
           hKey3=.rgf.registry~open(.rgf.hkey_classes_root, clsid"\CurVer") -- get handle to subkey
           if hKey3 <> 0 then  -- o.k. such a subkey was found, now use it to find CLSID
           do
              s. = .rgf.registry~getValue(hkey3, "")   -- get default value = PROGID
              curVerPROGID=s.data   -- get default value = PROGID
              .rgf.registry~close(hKey3)  -- close hKey3
              drop s.
              .rgf.registry~close(hkey2)  -- close hkey2
              hKey2=.rgf.registry~open(.rgf.hkey_classes_root, curVerPROGID"\CLSID") -- get CLSID of "CurVer"-PROGID
           end
         end

         s. = .rgf.registry~getValue(hKey2, "") -- get default value

         -- if .rgf.debug > 1 then say "create_c_o(): hkey="pp(hkey) "progid="pp(clsid) to "clssid="pp(s.data) "s.type="pp(s.type)
         clsid = s.data -- extract value from stem
         .rgf.registry~close(hKey2) -- close (free) handle
         .rgf.registry~close(hKey)  -- close (free) handle
      end
   end

   -- get a handle to subkey
   hClsidKey = .rgf.registry~open(.rgf.hkey_classes_root, "CLSID\"clsid, "READ")
   if hClsidKey=0 then
   do
      msg=clsid": not found!"
      if window <> "WINDOW" then nop -- call alert msg
                            else .error~say(msg)
      return .nil       -- do return nothing
   end

   o=.clsid~new(clsid)  -- create instance to store relevant information
   odir=o~keys          -- get access to directory to contain the keys and values

   s. = .rgf.registry~query(hClsidKey) -- query infos on key (number of subkeys, values; value of: date, time, class (name))
   o~datetime=changestr("/", s.date, "") s.time
   if s.values>0 then         -- show all values of the key, if any
   do
      .rgf.registry~listvalues(hClsidKey, stem.)  -- get a list of all values for this key
      -- show values of CLSID
      do i=1 to s.values
         if stem.i.name="" then  -- default value in hand ?
         do
            o~description=stem.i.data  -- save value with object
            leave i              -- leave loop
         end
      end
   end

   -- process subkeys
   .rgf.registry~list(hClsidKey, list2.) -- get all subkeys
   keysDir=.clsid~keysDir           -- get directory of interesting subkeys from class CLSID
   do idx2=1 to s.subkeys           -- iterate over subkeys and their values
      if \ keysDir~hasentry(list2.idx2) then iterate  -- if subkey-name is not of interest, iterate
         -- get a handle to the subkey in hand
      keyName=list2.idx2
      hSubKey2=.rgf.registry~open(hClsidKey, keyName, "READ")
      if hSubKey2 \= 0 then
      do
         s2. = .rgf.registry~query(hSubKey2) -- query various infos
         if s2.values>0 then           -- if values available, iterate over them
         do
            .rgf.registry~listvalues(hSubKey2, stem2.)  -- get a list of all values for this key
            do i=1 to s2.values
               if stem2.i.name="" then    -- default string value in hand, if so save
               do
                  tmp=""
                  if stem2.i.type="EXPAND" then tmp = expand(stem2.i.data) -- expand environment variable in string
                                           else tmp = stem2.i.data
                  odir~setentry(keyName, tmp)  -- save key with object
                  leave i
               end
            end
         end
         .rgf.registry~close(hSubKey2) -- close (return) handle
      end
   end   -- idx2

   if o=.nil then return .nil -- iterate     -- no entry found, iterate

   if \ odir~hasentry("PROGID") then return .nil -- iterate -- do not process a CLSID which has no value given for PROGID

      -- set up collections for administrating objects of this class
   .clsid~clsid_list~insert(clsid)      -- save CLSID in list
   .clsid~all_clsid~setentry(clsid, o)  -- save instance in collection

      -- set up relation between CLSID and PROGID
   .clsid~clsid2progid[clsid]=odir~progid

   -- all_progid~setentry(odir~progid, o)
   if odir~hasentry("VERSIONINDEPENDENTPROGID")  then
   do
      .clsid~clsid2progid[clsid]=odir~versionIndependentProgid

      -- all_progid~setentry(odir~versionIndependentProgid, o)
      .clsid~all_progid[odir~versionIndependentProgid] = o
-- say "versionIndependentProgId:" pp(odir~versionIndependentProgid) "-> o:" pp(o) "CLSID:" pp(o~clsid)
   end
   -- else
do
   .clsid~all_progid[odir~progid] = o -- o.k., then use PROGID
-- say "                  ProgId:" pp(odir~progId) "-> o:" pp(o)  "CLSID:" pp(o~clsid)
end

   if odir~hasentry("TREATAS")                   then
   do
      .clsid~clsid2progid[clsid]=odir~treatAs
      .clsid~treatAsSrc~put(clsid)         -- save CLSID, which needs to be treated as defined in another CLSID
      .clsid~treatAsTgt~put(odir~treatAs)  -- save target CLSID
   end

   return o

/* ================================================================================= */
/* class to represent all interesting information about CLSID's */
::class clsid public

   /* ------------------------- class methods ----------------------------------- */
::method init class
  expose all_clsid all_progid clsid_list clsid2progid keysList keysDir treatAsSrc treatAsTgt
  all_clsid = .directory~new     -- collects all instances, indexed by CLSID
  all_progid= .directory~new     -- collects all instances, indexed by PROGID+VERSIONINDEPENDENTPROGID
  clsid_list= .list~new
  clsid2progid = .relation~new   -- maps clsids to ProgID, VersionIndependentProgID and TreatAs
  treatAsSrc = .set~new
  treatAsTgt = .set~new
  keysList = .list~of( "VersionIndependentProgID", ,
                       "ProgID",                   ,
                       "Version",                  ,
                       "TreatAs",                  ,  -- points to CLSID which contains definitions
                       "LocalServer",              ,  -- binary program
                       "LocalServer32",            ,  -- binary program
                       "ScriptletURL",             ,  -- script-program (instead of LocalServer32)
                       "InProcHandler",            ,  -- handler being used
                       "InProcHandler32",          ,  -- handler being used
                       "InProcServer",             ,  -- server for handler being used
                       "InProcServer32" )             -- server for handler being used
   keysDir=.directory~new
   do item over keysList
      keysDir~setentry(item, item)        -- add name of key into directory
   end

::method all_clsid      attribute class   -- directory containing clsid as key to instance of this class
::method all_progid     attribute class   -- directory containing versionindependent/progid as key to instance of this class
::method clsid_list     attribute class   -- list: containing CLSIDs in order of appearence
::method clsid2progid   attribute class   -- relation: maps class-ids (idx) to progids, independent progids, treat-as progids
::method keysList       attribute class   -- list: denotes the key-names regarded to be interesting
::method keysDir        attribute class   -- directory: allow case-independent look-up of interesting keys
::method treatAsSrc     attribute class   -- set: contains CLSIDs which depend on other CLSID for operation
::method treatAsTgt     attribute class   -- set: contains CLSID pointed to


   -- analyze and build classes
::method analyze_and_build class
  expose all_clsid all_progid clsid_list clsid2progid keysList keysDir treatAsSrc treatAsTgt

  handle = .rgf.registry~open(.rgf.hKey_classes_root, "CLSID", "READ")
  clslist. = .rgf.registry~query(handle)    -- query number of subkeys, values, value of: class (name), date, time

  .rgf.registry~list(handle, subKeyList.)   -- get the subkeys (i.e. class-ids in UUID-format

  do idx =1 to clslist.subkeys     -- iterate over subkeys
      clsid=subKeyList.idx
      o=create_clsid_object( clsid, .true ) -- indicate that CLSID-value in hand

      if .rgf.debug>0 then if idx > 200 then leave  -- on debug, just process the first 50 entries
  end
--    .rgf.registry~close(hSubKey)
--  end
  .rgf.registry~close(handle)


   /* ------------------------- instance methods -------------------------------- */
::method init
  expose clsid description keys
  use arg clsid
  keys=.directory~new

::method clsid     attribute     -- stores the CLSID as found in registry
::method description attribute   -- stores the default value of CLSID (describing it)
::method keys      attribute     -- directory possessing the values of found keys
::method datetime  attribute     -- date and time of entry in registry

   -- this defines the default string value for this object
::method makestring
  expose clsid description datetime
  return clsid pp(description) pp(datetime)

   -- this dumps the keys
::method dumpkeys
   expose keys
   indent=copies(" ", 8)
   do item over self~class~keysList
      if keys~hasentry(item) then say indent item"="pp(keys~entry(item))
   end


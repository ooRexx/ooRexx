/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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


::requires 'rxwinsys' LIBRARY

::class 'WindowsRegistry' public

::attribute Local_Machine get external "LIBRARY rxwinsys getLocal_Machine"
::attribute Current_User get external "LIBRARY rxwinsys getCurrent_User"
::attribute Users get external "LIBRARY rxwinsys getUsers"
::attribute Classes_Root get external "LIBRARY rxwinsys getClasses_Root"
::attribute Performance_Data get external "LIBRARY rxwinsys getPerformance_Data"
::attribute Current_Config get external "LIBRARY rxwinsys getCurrent_Config"

::attribute Current_Key external "LIBRARY rxwinsys"

-- This is here to support some long time ago previous version.
::attribute CurrentKey external "LIBRARY rxwinsys Current_Key"

::method init external "LIBRARY rxwinsys WSRegistry_init"
::method open external "LIBRARY rxwinsys WSRegistry_open"
::method deleteKey external "LIBRARY rxwinsys WSRegistry_delete"
::method delete external "LIBRARY rxwinsys WSRegistry_delete"


::method Create
   use arg hkey, name
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryKey("CREATE", hkey, name)
   if (retc \= 0) then self~Current_Key = retc
   return retc

::method Close
   use arg hkey
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryKey("CLOSE", hkey)
   return retc

::method Query
   use arg hkey
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryKey("QUERY", hkey)
   if (retc \= 0) then do
      parse var retc classname", "subk", "vals", "adate", "atime
      retstem.Class = classname
      retstem.SubKeys = SubK
      retstem.Values = vals
      retstem.date = aDate
      retstem.time = aTime
      return retstem.
   end
   else return 0

::method List
   use arg hkey, astem.
   if arg(1,'o') = 1 then hkey = self~Current_Key
   drop InternKeyStem.
   retc = WSRegistryKey("LIST", hkey, "InternKeyStem")
   if retc = 0 then do i over InternKeyStem.
      astem.i = InternKeyStem.i
   end
   return retc

::method Flush
   use arg hkey
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryKey("FLUSH", hkey)
   return retc

::method SetValue
   use arg hkey, name, valu, atype
   if arg(1,'o') = 1 then hkey = self~Current_Key
   if arg(2,'o') = 1 then name = ""
   if arg(4,'o') = 1 then atype = "NORMAL"
   retc = WSRegistryValue("SET", hkey, name, valu, atype~translate)
   return retc

::method GetValue
   use arg hkey, name
   if arg(1,'o') = 1 then hkey = self~Current_Key
   if arg(2,'o') = 1 then name = ""
   retc = WSRegistryValue("QUERY", hkey, name)
   if (retc \= 0) then do
      parse var retc atype", "valu   /* swapped for feature 358 */
      retstem.Data = valu
      retstem.Type = atype
      return retstem.
   end
   else return 0

::method ListValues
   use arg hkey, astem.
   if arg(1,'o') = 1 then hkey = self~Current_Key
   drop InternValStem.
   retc = WSRegistryValue("LIST", hkey, "InternValStem")
   if retc = 0 then do i over InternValStem.
      astem.i = InternValStem.i
   end
   return retc

::method DeleteValue
   use arg hkey, name
   if arg(1,'o') = 1 then hkey = self~Current_Key
   if arg(2,'o') = 1 then name = ""
   retc = WSRegistryValue("DELETE", hkey, name)
   return retc

::method Connect
   use arg hkey, computername
   retc = WSRegistryFile("CONNECT", hkey, computername)
   return retc

::method Load
   use arg hkey, subkeyname, filename
   if arg(1,'o') = 1 then hkey = self~Local_Machine
   retc = WSRegistryFile("LOAD", hkey, subkeyname, filename)
   return retc

::method Save
   use arg hkey, filename
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryFile("SAVE", hkey, filename)
   return retc

::method Restore
   use arg hkey, filename, opt /* opt can be VOLATILE */
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryFile("RESTORE", hkey, filename, opt~translate)
   return retc

::method Replace
   use arg hkey, subkeyname, fnnew, fnold
   if arg(1,'o') = 1 then hkey = self~Current_Key
   if arg(2,'o') = 1 then subkeyname = "%NULL%"
   retc = WSRegistryFile("REPLACE", hkey, subkeyname, fnnew, fnold)
   return retc

::method UnLoad
   use arg hkey, subkeyname
   if arg(1,'o') = 1 then hkey = self~Current_Key
   retc = WSRegistryFile("UNLOAD", hkey, subkeyname)
   return retc

-- Deprecated.  Provided for backwards compatibility, does nothing.
::method deinstall

-- Deprecated.  Provided for backwards compatiblity, will always return 0
::attribute initCode get
  return 0


/*********************************** WindowsProgramManager ****************************************/

::class 'WindowsProgramManager' public inherit VirtualKeyCodes

::method AddGroup
   use arg name
   return WSProgManager("ADDGROUP", name, "")

::method DeleteGroup
   use arg name
   return WSProgManager("DELGROUP", name)

::method ShowGroup
   use arg name, cmd
   cmd = cmd~translate
   return WSProgManager("SHOWGROUP", name, cmd)

::method AddItem
   use arg name, prog, iconfile, iconnr, workdir, placelast, minimized, hotkey

   if arg(3,'o') = 1 then iconfile = ""
   if arg(4,'o') = 1 then iconnr = 0
   if arg(5,'o') = 1 then workdir = ""
   if arg(6,'o') = 1 then placelast = 0
   if arg(7,'o') = 1 then minimized = 0
   if arg(8,'o') = 1 then hotkey = ""

    scModifier = 0
    scKey = 0

    if hotkey \= "" then do
        hotkey = hotkey~translate
        /* if hotkey contains any modifier key (SHIFT,CONTROL,...) */
        if hotkey~pos('+') = 0 then do
            scKey = self~VCode(hotkey)
            /* only F1 to F24 are allowed without modification characters */
            /* for all others keys use Ctrl+ ALT                          */
            if scKey > self~VCode("F12") | scKey < self~VCode("F1") then do
                scModifier = 02~x2d
                scModifier = scModifier + 04~x2d
            end
        end
        else do
         /* When any modifier key is specified, use it ! Dont care if it gets any problem */
         /* EXT not supported, because it dos not function correctly */
            scKey = self~VCode(hotkey~substr(hotkey~lastpos('+') + 1))
            if hotkey~pos('SHIFT')   \= 0 then scModifier = scModifier + 01~x2d
            if hotkey~pos('CONTROL') \= 0 then scModifier = scModifier + 02~x2d
            if hotkey~pos('ALT')     \= 0 then scModifier = scModifier + 04~x2d
        end
    end

   return WSProgManager("ADDITEM", prog, name, iconfile, iconnr, workdir, placelast, scKey, scModifier, minimized)

::method DeleteItem
   use arg name
   return WSProgManager("DELITEM", name)

::method Close
   parse upper arg savegrp
   return WSProgManager("LEAVE", savegrp)

::method DeleteDesktopIcon
   use arg name, location
   if arg(2,'o') = 1 then location = "PERSONAL"
   return WSProgManager("DELDESKTOPICON", name, location )


::method AddDesktopIcon
   use arg name, prog, iconfile, iconnr, workdir, location, arguments, hotkey, run
   if arg(3,'o') = 1 then iconfile   = ""
   if arg(4,'o') = 1 then iconnr     =  0
   if arg(5,'o') = 1 then workdir    = ""
   if arg(6,'o') = 1 then location   = "PERSONAL"
   if arg(7,'o') = 1 then arguments  = ""
   if arg(8,'o') = 1 then hotkey     = ""
   if arg(9,'o') = 1 then run        = ""

    scModifier = 0
    scKey = 0

    if hotkey \= "" then do
        hotkey = hotkey~translate
        /* if hotkey contains any modifier key (SHIFT,CONTROL,...) */
        if hotkey~pos('+') = 0 then do
            scKey = self~VCode(hotkey)
            /* only F1 to F24 are allowed without modification characters */
            /* for all others keys use Ctrl+ ALT                          */
            if scKey > self~VCode("F12") | scKey < self~VCode("F1") then do
                scModifier = 02~x2d
                scModifier = scModifier + 04~x2d
            end
        end
        else do
         /* When any modifier key is specified, use it ! Dont care if it gets any problem */
         /* EXT not supported, because it dos not function correctly */
            scKey = self~VCode(hotkey~substr(hotkey~lastpos('+') + 1))
            if hotkey~pos('SHIFT')   \= 0 then scModifier = scModifier + 01~x2d
            if hotkey~pos('CONTROL') \= 0 then scModifier = scModifier + 02~x2d
            if hotkey~pos('ALT')     \= 0 then scModifier = scModifier + 04~x2d
        end
    end
   return WSProgManager("ADDDESKTOPICON", name, prog, iconfile, iconnr, workdir, location, arguments, scKey, scModifier, run)

::method AddShortCut
   arga = Arg(1,"A")
   newarg = .array~new(Arg()+1)
   do i = Arg() to 6 by -1; if arga~hasindex(i) = 1 then newarg[i+1] = arga[i]; end   /* shift arguments up by 1 */
   newarg[6] = ""  /* not COMMON nor PERSONAL */
   do i = 5 to 1 by -1; if arga~hasindex(i) = 1 then newarg[i] = arga[i]; end   /* copy first 5 */
   forward message "AddDesktopIcon" Arguments (newarg)

-- Deprecated.  Provided for backwards compatibility, does nothing.
::method deinstall

-- Deprecated.  Provided for backwards compatiblity, will always return 0
::attribute initCode get
  return 0



::class 'WindowsEventLog' public

::attribute currentHandle private
::attribute events get
::attribute minimumReadBuffer get
::attribute minimumReadMin get
::attribute minimumReadMax get

::method init external "LIBRARY rxwinsys WSEventLog_init"
::method uninit external "LIBRARY rxwinsys WSEventLog_uninit"
::method open external "LIBRARY rxwinsys WSEventLog_open"
::method close external "LIBRARY rxwinsys WSEventLog_close"
::method getNumber external "LIBRARY rxwinsys WSEventLog_getNumber"
::method getFirst external "LIBRARY rxwinsys WSEventLog_getFirst"
::method getLast external "LIBRARY rxwinsys WSEventLog_getLast"
::method getLogNames external "LIBRARY rxwinsys WSEventLog_getLogNames"
::method isFull external "LIBRARY rxwinsys WSEventLog_isFull"
::method clear external "LIBRARY rxwinsys WSEventLog_clear"
::method write external "LIBRARY rxwinsys WSEventLog_write"
::method readRecords external "LIBRARY rxwinsys WSEventLog_readRecords"
::method minimumRead external "LIBRARY rxwinsys WSEventLog_minimumReadGet"
::method "minimumRead=" external "LIBRARY rxwinsys WSEventLog_minimumReadSet"

-- Deprecated.  Provided for backwards compatibilty.  Use readRecords() instead.
::method Read
  forward message "readRecords" continue
  ret = result
  if ret == 0 then return self~events
  else return .nil

-- Deprecated.  Provided for backwards compatibility, does nothing.
::method deinstall

-- Deprecated.  Provided for backwards compatiblity, will always return 0
::attribute initCode get

-- Internal test method, do not use.
::method test external "LIBRARY rxwinsys WSEventLog_test"



/***********  VirtualKeyCodes **********************/

::class 'VirtualKeyCodes' mixinclass Object public

::method VCode
   parse arg k
   k = k~translate
   select
      when k = "LBUTTON"  then code = 01~x2d
      when k = "RBUTTON"  then code = 02~x2d
      when k = "CANCEL"  then code = 03~x2d
      when k = "MBUTTON"  then code = 04~x2d
      when k = "BACK"  then code = 08~x2d
      when k = "TAB" then code = 09~x2d
      when k = "CLEAR"  then code = 0C~x2d
      when k = "RETURN"  then code = 0D~x2d
      when k = "SHIFT"  then code = 10~x2d
      when k = "CONTROL"  then code = 11~x2d
      when k = "MENU"  then code = 12~x2d
      when k = "PAUSE"  then code = 13~x2d
      when k = "CAPITAL"  then code = 14~x2d
      when k = "ESCAPE"  then code = 1B~x2d
      when k = "SPACE" | k = " "  then code = 20~x2d
      when k = "PRIOR"  then code = 21~x2d
      when k = "NEXT"  then code = 22~x2d
      when k = "END"    then code = 23~x2d
      when k = "HOME"  then code = 24~x2d
      when k = "LEFT"  then code = 25~x2d
      when k = "UP"  then code = 26~x2d
      when k = "RIGHT"  then code = 27~x2d
      when k = "DOWN"  then code = 28~x2d
      when k = "SELECT"  then code = 29~x2d
      when k = "EXECUTE"  then code = 2B~x2d
      when k = "SNAPSHOT"  then code = 2C~x2d
      when k = "INSERT"  then code = 2D~x2d
      when k = "DELETE"  then code = 2E~x2d
      when k = "HELP"  then code = 2F~x2d
      when k = "0"  then code = 30~x2d
      when k = "1"  then code = 31~x2d
      when k = "2"  then code = 32~x2d
      when k = "3"  then code = 33~x2d
      when k = "4"  then code = 34~x2d
      when k = "5"  then code = 35~x2d
      when k = "6"  then code = 36~x2d
      when k = "7"  then code = 37~x2d
      when k = "8"  then code = 38~x2d
      when k = "9"  then code = 39~x2d
      when k = "A"  then code = 41~x2d
      when k = "B"  then code = 42~x2d
      when k = "C"  then code = 43~x2d
      when k = "D"  then code = 44~x2d
      when k = "E"  then code = 45~x2d
      when k = "F"  then code = 46~x2d
      when k = "G"  then code = 47~x2d
      when k = "H"  then code = 48~x2d
      when k = "I"  then code = 49~x2d
      when k = "J"  then code = 4A~x2d
      when k = "K"  then code = 4B~x2d
      when k = "L"  then code = 4C~x2d
      when k = "M"  then code = 4D~x2d
      when k = "N"  then code = 4E~x2d
      when k = "O"  then code = 4F~x2d
      when k = "P"  then code = 50~x2d
      when k = "Q"  then code = 51~x2d
      when k = "R"  then code = 52~x2d
      when k = "S"  then code = 53~x2d
      when k = "T"  then code = 54~x2d
      when k = "U"  then code = 55~x2d
      when k = "V"  then code = 56~x2d
      when k = "W"  then code = 57~x2d
      when k = "X"  then code = 58~x2d
      when k = "Y"  then code = 59~x2d
      when k = "Z"  then code = 5A~x2d
      when k = "NUMPAD0"  then code = 60~x2d
      when k = "NUMPAD1"  then code = 61~x2d
      when k = "NUMPAD2"  then code = 62~x2d
      when k = "NUMPAD3"  then code = 63~x2d
      when k = "NUMPAD4"  then code = 64~x2d
      when k = "NUMPAD5"  then code = 65~x2d
      when k = "NUMPAD6"  then code = 66~x2d
      when k = "NUMPAD7"  then code = 67~x2d
      when k = "NUMPAD8"  then code = 68~x2d
      when k = "NUMPAD9"  then code = 69~x2d
      when k = "MULTIPLY"  then code = 6A~x2d
      when k = "ADD" then code = 6B~x2d
      when k = "SEPARATOR"  then code = 6C~x2d
      when k = "SUBTRACT"  then code = 6D~x2d
      when k = "DECIMAL"  then code = 6E~x2d
      when k = "DIVIDE"  then code = 6F~x2d
      when k = "F1"  then code = 70~x2d
      when k = "F2"  then code = 71~x2d
      when k = "F3"  then code = 72~x2d
      when k = "F4"  then code = 73~x2d
      when k = "F5"  then code = 74~x2d
      when k = "F6"  then code = 75~x2d
      when k = "F7"  then code = 76~x2d
      when k = "F8"  then code = 77~x2d
      when k = "F9"  then code = 78~x2d
      when k = "F10"  then code = 79~x2d
      when k = "F11"  then code = 7A~x2d
      when k = "F12"  then code = 7B~x2d
      when k = "F13"  then code = 7C~x2d
      when k = "F14"  then code = 7D~x2d
      when k = "F15"  then code = 7E~x2d
      when k = "F16"  then code = 7F~x2d
      when k = "F17"  then code = 80~x2d
      when k = "F18"  then code = 81~x2d
      when k = "F19"  then code = 82~x2d
      when k = "F20"  then code = 83~x2d
      when k = "F21"  then code = 84~x2d
      when k = "F22"  then code = 85~x2d
      when k = "F23"  then code = 86~x2d
      when k = "F24"  then code = 87~x2d
      when k = "NUMLOCK"  then code = 90~x2d
      when k = "SCROLL"  then code = 91~x2d
      otherwise code = 255
   end
   return code


::method KeyName
   parse arg code
   k = k~translate
   select
      when code = 01~x2d   then k = "LBUTTON"
      when code = 02~x2d   then k = "RBUTTON"
      when code = 03~x2d   then k = "CANCEL"
      when code = 04~x2d   then k = "MBUTTON"
      when code = 08~x2d   then k = "BACK"
      when code = 09~x2d   then k = "TAB" then
      when code = 0C~x2d   then k = "CLEAR"
      when code = 0D~x2d   then k = "RETURN"
      when code = 10~x2d   then k = "SHIFT"
      when code = 11~x2d   then k = "CONTROL"
      when code = 12~x2d   then k = "MENU"
      when code = 13~x2d   then k = "PAUSE"
      when code = 14~x2d   then k = "CAPITAL"
      when code = 1B~x2d   then k = "ESCAPE"
      when code = 20~x2d   then k = "SPACE"
      when code = 21~x2d   then k = "PRIOR"
      when code = 22~x2d   then k = "NEXT"
      when code = 23~x2d   then k = "END"
      when code = 24~x2d   then k = "HOME"
      when code = 25~x2d   then k = "LEFT"
      when code = 26~x2d   then k = "UP"
      when code = 27~x2d   then k = "RIGHT"
      when code = 28~x2d   then k = "DOWN"
      when code = 29~x2d   then k = "SELECT"
      when code = 2B~x2d   then k = "EXECUTE"
      when code = 2C~x2d   then k = "SNAPSHOT"
      when code = 2D~x2d   then k = "INSERT"
      when code = 2E~x2d   then k = "DELETE"
      when code = 2F~x2d   then k = "HELP"
      when code = 30~x2d   then k = "0"
      when code = 31~x2d   then k = "1"
      when code = 32~x2d   then k = "2"
      when code = 33~x2d   then k = "3"
      when code = 34~x2d   then k = "4"
      when code = 35~x2d   then k = "5"
      when code = 36~x2d   then k = "6"
      when code = 37~x2d   then k = "7"
      when code = 38~x2d   then k = "8"
      when code = 39~x2d   then k = "9"
      when code = 41~x2d   then k = "A"
      when code = 42~x2d   then k = "B"
      when code = 43~x2d   then k = "C"
      when code = 44~x2d   then k = "D"
      when code = 45~x2d   then k = "E"
      when code = 46~x2d   then k = "F"
      when code = 47~x2d   then k = "G"
      when code = 48~x2d   then k = "H"
      when code = 49~x2d   then k = "I"
      when code = 4A~x2d   then k = "J"
      when code = 4B~x2d   then k = "K"
      when code = 4C~x2d   then k = "L"
      when code = 4D~x2d   then k = "M"
      when code = 4E~x2d   then k = "N"
      when code = 4F~x2d   then k = "O"
      when code = 50~x2d   then k = "P"
      when code = 51~x2d   then k = "Q"
      when code = 52~x2d   then k = "R"
      when code = 53~x2d   then k = "S"
      when code = 54~x2d   then k = "T"
      when code = 55~x2d   then k = "U"
      when code = 56~x2d   then k = "V"
      when code = 57~x2d   then k = "W"
      when code = 58~x2d   then k = "X"
      when code = 59~x2d   then k = "Y"
      when code = 5A~x2d   then k = "Z"
      when code = 60~x2d   then k = "NUMPAD0"
      when code = 61~x2d   then k = "NUMPAD1"
      when code = 62~x2d   then k = "NUMPAD2"
      when code = 63~x2d   then k = "NUMPAD3"
      when code = 64~x2d   then k = "NUMPAD4"
      when code = 65~x2d   then k = "NUMPAD5"
      when code = 66~x2d   then k = "NUMPAD6"
      when code = 67~x2d   then k = "NUMPAD7"
      when code = 68~x2d   then k = "NUMPAD8"
      when code = 69~x2d   then k = "NUMPAD9"
      when code = 6A~x2d   then k = "MULTIPLY"
      when code = 6B~x2d   then k = "ADD"
      when code = 6C~x2d   then k = "SEPARATOR"
      when code = 6D~x2d   then k = "SUBTRACT"
      when code = 6E~x2d   then k = "DECIMAL"
      when code = 6F~x2d   then k = "DIVIDE"
      when code = 70~x2d   then k = "F1"
      when code = 71~x2d   then k = "F2"
      when code = 72~x2d   then k = "F3"
      when code = 73~x2d   then k = "F4"
      when code = 74~x2d   then k = "F5"
      when code = 75~x2d   then k = "F6"
      when code = 76~x2d   then k = "F7"
      when code = 77~x2d   then k = "F8"
      when code = 78~x2d   then k = "F9"
      when code = 79~x2d   then k = "F10"
      when code = 7A~x2d   then k = "F11"
      when code = 7B~x2d   then k = "F12"
      when code = 7C~x2d   then k = "F13"
      when code = 7D~x2d   then k = "F14"
      when code = 7E~x2d   then k = "F15"
      when code = 7F~x2d   then k = "F16"
      when code = 80~x2d   then k = "F17"
      when code = 81~x2d   then k = "F18"
      when code = 82~x2d   then k = "F19"
      when code = 83~x2d   then k = "F20"
      when code = 84~x2d   then k = "F21"
      when code = 85~x2d   then k = "F22"
      when code = 86~x2d   then k = "F23"
      when code = 87~x2d   then k = "F24"
      when code = 90~x2d   then k = "NUMLOCK"
      when code = 91~x2d   then k = "SCROLL"
      otherwise k = ""
   end
   return k



/********************* WindowsManager **********************/

::class 'WindowsManager' public

::method DeskTopWindow
   hw = WSCtrlWindow("DESK")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method Find
   parse arg title
   hw = WSCtrlWindow("FIND", title)
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method ForegroundWindow
   hW = WSCtrlWindow("GETFG")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method WindowAtPosition
   use arg x, y
   hW = WSCtrlWindow("ATPOS", x, y)
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method ConsoleTitle
   return WSCtrlWindow("TITLE", 0)

::method "ConsoleTitle="
   use arg title
   return WSCtrlWindow("TITLE", 0, title)

::Method SendTextToWindow
   use arg title, text
   w = self~find(title)
   if w \= .nil then w~SendText(text)

::method PushButtonInWindow
   use arg title, text
   w = self~find(title)
   if w \= .nil then do
       cW = w~FindChild(text)
       if cW \= .nil then cW~SendKey("SPACE")
   end

::method ProcessMenuCommand
   if Arg() < 2 then return 0
   if Arg() = 2 then do
       wnd = self~Find(Arg(1))
       if wnd = .nil then return 0
       men = wnd~Menu
       if men = .nil then return 0
       men~ProcessItem(men~FindItem(Arg(1)))
   end
   else do
       wnd = self~Find(Arg(1))
       if wnd = .nil then return 0
       sub = wnd~Menu
       if sub = .nil then return 0
       do i = 2 to Arg()-1
           sub = sub~FindSubMenu(Arg(i))
       end
       if sub = .nil then return 0
       sub~ProcessItem(sub~FindItem(Arg(Arg())))
   end

/**
 * Send the WM_SETTINGSCHANGE message to all top-level windows.  The format is:
 *
 *   SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, "Environment")
 *
 * HWND_BROADCAST == 0xffff, WM_SETTINGCHANGE == 0x001A.
*/
::method BroadcastSettingChanged
   use arg milliseconds
   if Arg(1, 'O') then do
       ret = WSCtrlSend("MSG", "0xFFFF", "0x001A", "0x0", "0x0", 'E')
       if ret > 0 then ret = -ret
       return ret
   end

   if \ milliseconds~datatype('W') then milliseconds = 5000
   if milliseconds < 1 then milliseconds = 5000
   return WSCtrlSend("TO", "0xFFFF", "0x001A", "0x0", "0x0", milliseconds, 'E')

-- Deprecated.  Provided for backwards compatibility, does nothing.
::method deinstall

-- Deprecated.  Provided for backwards compatiblity, will always return 0
::attribute initCode get
  return 0


/********************* WindowsObject **********************/

::class 'WindowObject' public inherit VirtualKeyCodes

::method hwnd attribute

::method Init
   use arg hwnd
   self~hwnd = hwnd

::method AssocWindow
   use arg hwnd
   self~hwnd = hwnd

::method Handle
   return self~hwnd

::method Title
   return WSCtrlWindow("TITLE", self~hwnd)

::method "Title="
   use arg title
   return WSCtrlWindow("TITLE", self~hwnd, title)

::method WClass
   return WSCtrlWindow("CLASS", self~hwnd)

::method Id
   return WSCtrlWindow("ID", self~hwnd)

::method Coordinates
   return WSCtrlWindow("RECT", self~hwnd)

::method State
   return WSCtrlWindow("GETSTATE", self~hwnd)

::method getStyle
   return WSCtrlWindow("GETSTYLE", self~hwnd)

::method Restore
   return WSCtrlWindow("SHOW", self~hwnd, "RESTORE")

::method Hide
   return WSCtrlWindow("SHOW", self~hwnd, "HIDE")

::method Minimize
   return WSCtrlWindow("SHOW", self~hwnd, "MIN")

::method Maximize
   return WSCtrlWindow("SHOW", self~hwnd, "MAX")

::method Enable
   return WSCtrlWindow("ENABLE", self~hwnd, 1)

::method Disable
   return WSCtrlWindow("ENABLE", self~hwnd, 0)

::method MoveTo
   use arg x, y
   return WSCtrlWindow("MOVE", self~hwnd, x, y)

::method Resize
   use arg cx, cy
   return WSCtrlWindow("SIZE", self~hwnd, cx, cy)

::method ToForeground
   return WSCtrlWindow("SETFG", self~hwnd)

::method FocusNextItem
   self~SendMessage("0x0028", 0,0)

::method FocusPreviousItem
   self~SendMessage("0x0028", 1,0)

::method FocusItem
   use arg hItem
   self~SendMessage("0x0028", hItem~Handle, 1)

::method FindChild
   parse arg title
   hw = WSCtrlWindow("FINDCHILD", self~hwnd, title)
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method ChildAtPosition
   use arg x, y
   hw = WSCtrlWindow("ATPOS", x, y, self~hwnd)
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method Next
   hw = WSCtrlWindow("HNDL", self~hwnd, "NEXT")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method Previous
   hw = WSCtrlWindow("HNDL", self~hwnd, "PREVIOUS")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method First
   hw = WSCtrlWindow("HNDL", self~hwnd, "FIRST")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method FirstChild
   hw = WSCtrlWindow("HNDL", self~hwnd, "FIRSTCHILD")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method Last
   hw = WSCtrlWindow("HNDL", self~hwnd, "LAST")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method Owner
   hw = WSCtrlWindow("HNDL", self~hwnd, "OWNER")
   if hw = 0 then return .nil
   else return .WindowObject~new(hw)

::method EnumerateChildren
   fc = self~FirstChild
   cnt = 0
   wndlist.0 = 0
   if fc = .Nil then return wndlist.
   ac = fc
   do while ac \= .Nil
       cnt = cnt + 1
       wndlist.cnt.!Handle = ac~Handle
       wndlist.cnt.!Title = ac~Title
       wndlist.cnt.!Class = ac~WClass
       wndlist.cnt.!State = ac~State
       wndlist.cnt.!Coordinates = ac~Coordinates
       if ac~FirstChild \= .Nil then wndlist.cnt.!Children = 1
       else wndlist.cnt.!Children = 0
       wndlist.cnt.!Id = ac~Id
       ac = ac~Next
   end
   wndlist.0 = cnt
   return wndlist.

::method SendMessage
   use arg message, wParam, lParam
   return WSCtrlSend("MSG", self~hwnd, message, wParam, lParam)

::method SendCommand
   use arg cmd
   self~SendMessage("0x111", cmd~d2x, 0)

::method SendMenuCommand
   use arg cmd
   self~SendMessage("0x111", ("10000"~x2D + cmd)~d2x, 0)

::method SendSysCommand
   use arg command
   command = command~translate
   select
      when command = "SIZE" then cmd = "0xF000"
      when command = "MOVE" then cmd = "0xF010"
      when command = "MINIMIZE" then cmd = "0xF020"
      when command = "MAXIMIZE" then cmd = "0xF030"
      when command = "NEXTWINDOW" then cmd = "0xF040"
      when command = "PREVWINDOW" then cmd = "0xF050"
      when command = "CLOSE" then cmd = "0xF060"
      when command = "VSCROLL" then cmd = "0xF070"
      when command = "HSCROLL" then cmd = "0xF080"
      when command = "ARRANGE" then cmd = "0xF110"
      when command = "RESTORE"  then cmd = "0xF120"
      when command = "TASKLIST"  then cmd = "0xF130"
      when command = "SCREENSAVE" then cmd = "0xF140"
      when command = "MONITORPOWER" then cmd = "0xF170"
      when command = "CONTEXTHELP" then cmd = "0xF180"
      otherwise cmd = 0
   end
   if cmd \= 0 then self~SendMessage("0x112", cmd, 0)

::method SendMouseClick
    use arg which, kind, x, y, ext
    which = which~translate
    kind = kind~translate
    wp = 0
    select
        when kind = "DOWN" then cmd = 1
        when kind = "UP" then cmd = 2
        otherwise cmd = 3
    end
    select
        when which = "RIGHT" then do
            s = "203"~x2d
            if cmd = 1 then wp = 2
        end
        when which = "MIDDLE" then do
            s = "206"~x2d
            if cmd = 1 then wp = 10
        end
        otherwise do
            s = "200"~x2d
            if cmd = 1 then wp = 1
        end
    end
    if arg(5,'e') then do
        ext = ext~translate
        if ext~pos("LEFTDOWN") > 0 & which \= "LEFT" then wP = wP + 1
        if ext~pos("RIGHTDOWN") > 0 & which \= "RIGHT" then wP = wP + 2
        if ext~pos("MIDDLEDOWN") > 0 & which \= "MIDDLE" then wP = wP + 10
        if ext~pos("SHIFT") > 0 then wP = wP + 4
        if ext~pos("CONTROL") > 0 then wP = wp + 8
    end
    self~SendMessage((s + cmd)~d2x, wP~d2x, (y * "10000"~x2d + x)~d2x)

::method PushButton
   use arg text
   cW = self~FindChild(text)
   if cW \= .Nil then cW~SendKey("SPACE")

::method SendKey
   use arg char, alt, ext
   if Arg(3,'o') = 1 then ext = 0
   code = self~VCode(char)
   if ext = 1 then do
       if alt = 1 then
         alt = "0x21000000";
       else
         alt = "0x01000000"
   end
   else
      if alt = 1 then
        alt = "0x20000000";
      else
        alt = 0

   if (char~length = 1 &  (char >= "a" & char <= "z")) | code = 255 then
     cc = char~c2d;
   else
     cc = WSCtrlSend("MAP", code)
   if cc \= 0 then
     if char~length > 1 then
       do
         ret = self~SendKeyDown(char, ext)
         ret = self~SendKeyUp(char, ext)
       end
     else
      ret = WSCtrlSend("KEY", self~hwnd, cc, "C", alt)
   else
     ret = self~SendKeyDown(char, ext)
   return ret

::method SendChar
   use arg char, alt
   if Arg(2,'o') = 1 then return WSCtrlSend("KEY", self~hwnd, char~c2d, "C")
   if alt = 1 then alt = "0x20000000"
   return WSCtrlSend("KEY", self~hwnd, char~c2d, "C", alt)

::method SendKeyDown
   use arg code, ext
   if Arg(2,'o') = 1 then ext = 0
   if ext = 1 then ext = "0x01000000"
   return WSCtrlSend("KEY", self~hwnd, self~VCode(code), "D", ext)

::method SendKeyUp
   use arg code, ext
   if Arg(2,'o') = 1 then ext = 0
   if ext = 1 then ext = "0x01000000"
   return WSCtrlSend("KEY", self~hwnd, self~VCode(code), "U", ext)

::Method SendText
   use arg text
   if Arg(1,'o') = 1 then return -1
   do i = 1 to text~length
       c = text~substr(i,1)
       self~SendKey(c)
   end

::method Menu
   hw = WSCtrlMenu("GET", self~hwnd)
   if hw = 0 then do
       return .nil
   end
   else do
       obj = .MenuObject~new(hw, self)
       if obj~IsMenu = 0 then return .nil
       else return obj
   end

::method SystemMenu
   hw = WSCtrlMenu("SYS", self~hwnd)
   if hw = 0 then do
       return .nil
   end
   else do
       obj = .MenuObject~new(hw, self)
       if obj~IsMenu = 0 then return .nil
       else return obj
   end

::method IsMenu
   return WSCtrlMenu("STATE", self~hwnd)

::method ProcessMenuCommand
   if Arg() < 1 then return 0
   if Arg() = 1 then do
       men = self~Menu
       if men = .Nil then return 0
       men~ProcessItem(men~FindItem(Arg(1)))
   end
   else do
       sub = self~Menu
       if sub = .Nil then return 0
       do i = 1 to Arg()-1
           sub = sub~FindSubMenu(Arg(i))
       end
       if sub = .Nil then return 0
       sub~ProcessItem(sub~FindItem(Arg(Arg())))
   end


/********************* MenuObject **********************/

::class 'MenuObject' subclass WindowObject public

::method MainWnd attribute

::method Init
   use arg hwnd, main
   self~hwnd = hwnd
   self~MainWnd = main

::method IsMenu
   return WSCtrlMenu("STATE", self~hwnd)

::method IsSubMenu
   use strict arg pos
   return WSCtrlMenu("STATE", self~hwnd, "MENU", pos)

/* This method can not be 100% reliable.  Some applications do not set the check
 * mark for a menu item until the menu is displayed.  To be confident of the
 * result, the user should first test how a specific application behaves.
 */
::method IsChecked
   use strict arg pos
   return WSCtrlMenu("STATE", self~hwnd, "CHECK", pos)

::method IsSeparator
   use strict arg pos
   return WSCtrlMenu("STATE", self~hwnd, "SEPARTOR", pos)

::method IdOf
   use arg pos
   if Arg(1,'o') = 1 then return 0
   return WSCtrlMenu("ID", self~hwnd, pos)

::method Items
   return WSCtrlMenu("CNT", self~hwnd)

::method TextOf
   use arg pos
   if Arg(1,'o') = 1 then return ""
   return WSCtrlMenu("TEXT", self~hwnd, pos, "P")

::method TextOfId
   use arg id
   if Arg(1,'o') = 1 then return ""
   return WSCtrlMenu("TEXT", self~hwnd, id, "C")

::method SubMenu
   use arg pos
   if Arg(1,'o') = 1 then return .Nil
   hw = WSCtrlMenu("SUB", self~hwnd, pos)
   if hw = 0 then return .Nil
   else return .MenuObject~new(hw, self~MainWnd)

::method FindSubMenu
   use arg text
   if Arg(1,'o') = 1 then return .Nil
   cnt = self~Items
   do i = 0 to cnt-1
      mtext = self~TextOf(i)
      tabpos = mtext~pos("9"x)
      if text = mtext then return self~SubMenu(i)
      else if text~pos("9"x) = 0 & tabpos > 0 then if text = mtext~left(mtext~pos("9"x)-1) then return self~SubMenu(i)
   end
   return .Nil

::method FindItem
   use arg text
   if Arg(1,'o') = 1 then return 0
   cnt = self~Items
   do i = 0 to cnt-1
       mtext = self~TextOf(i)
       tabpos = mtext~pos("9"x)
       if text = mtext then return self~IdOf(i)
       else if text~pos("9"x) = 0 & tabpos > 0 then if text = mtext~left(mtext~pos("9"x)-1) then return self~IdOf(i)
   end
   return 0

::method ProcessItem
   use arg cmd
   if cmd \= 0 then self~MainWnd~SendMessage("0x111", ("10000"~x2D + cmd)~d2x, 0)




/********************* WindowsClipboard **********************/

::class 'WindowsClipboard' public

::method Copy
   use arg txt
   return WSClipboard("COPY", txt)

::method Paste
   return WSClipboard("PASTE")

::method Empty
   return WSClipboard("EMPTY")

::method IsDataAvailable
   return WSClipboard("AVAIL")

::method makeArray
   return WSClipboard("PASTE")~makeArray

-- Deprecated.  Provided for backwards compatibility, does nothing.
::method deinstall

-- Deprecated.  Provided for backwards compatiblity, will always return 0
::attribute initCode get
  return 0

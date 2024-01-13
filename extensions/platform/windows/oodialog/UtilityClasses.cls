/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * Utility Classes, "short-cut" dialogs and routines, public routines, and
 * utility dialogs.
 *
 * The mechanism for putting the .ApplicationManager object in the .local
 * environment relies on maintaining the first class definitions in this file as
 * being: .ResourceUtils, .ApplicationManager, and then .DlgUtils.
 *
 * Note:  This "class" file can not be used by itself, as for instance
 *        UserDialog.cls could.  It is intended to be merged into the main
 *        ooDialog framework file as the first file.  It contains classes that
 *        subclass other classes that will be present in the single framework
 *        file, but are not present in this file.
 */


::class 'ResourceUtils' public mixinclass object

-- Directory object containing symbolic constants
::attribute constDir get
::attribute constDir set private
::attribute processingLoad private    -- in loadItems ?

::method checkfile private
   use arg f
   if (f~lines = 0) then do
      f~close
      self~processingLoad = 0
      ret = errorDialog("Error reading resource file!" f)
      return 1
   end
   return 0

::method checkID external "LIBRARY oodialog rsrcUtils_checkID"
::method errorfile private
   use arg f, s
   f~close
   self~processingLoad = 0
   ret = errorDialog("Error reading resource file:" f "("s")")

-- This method is very specific looking only for an ICON resource statement.
::method findIconInRcFile private
    use strict arg resFile, iconID

    if \ SysIsFile(resFile) then do
        foundFile = SysSearchPath("PATH", resFile)
        if foundFile == "" then do
            msg = 'Unable to find resource script file "'resFile'"'
            j = MessageDialog(msg, 0, "File Not Found Error", "OK", "WARNING", "SYSTEMMODAL")
            return ""
        end
        resFile = foundFile
    end

    f = .stream~new(resFile)
    ret = f~open(read)
    if ret \= "READY:" then do
        msg = 'Resource script file "'resFile'" is not readable:' ret
        j = MessageDialog(msg, 0, "File Open Error", "OK", "WARNING", "SYSTEMMODAL")
        return ""
    end

    found = .false; iconFile = ''
    fl = f~lines
    do while \ found
        do while iconFile == '' & fl > 0
            s = f~linein; fl -= 1
            if s~wordpos("ICON") = 2 then do
                -- ICON RESOURCE statement as opposed to an ICON CONTROL statement.
                -- filename could have spaces and *should* be in quotes.
                parse var s nameID ic fileName
                if nameID~translate == iconID~translate then do
                    found = .true
                    iconfile = fileName~strip
                end
            end
        end
        if fl < 1 then leave
    end

    f~close

    -- The file name in the resource script file will be in a C string format.
    -- We need to change any escaped slashes.  In addition, the file name will
    -- most likely be used in LoadImage() and must not be enclosed in quotes.
    if iconFile~pos("\\") <> 0 then iconFile = iconFile~changestr("\\", '\')
    iconFile = iconFile~strip('B', '"')

    if iconFile == '' then do
        msg = 'Resource script file "'resFile'" does not contain an ICON resource statement for ID:' iconID
        j = MessageDialog(msg, 0, "Resource Script File Error", "OK", "WARNING", "SYSTEMMODAL")
    end
    return iconFile

::method getResourceID unguarded external "LIBRARY oodialog rsrcUtils_resolveResourceID"
::method idError private external "LIBRARY oodialog rsrcUtils_idError"
::method parseIncludeFile
  use strict arg hFile

  if \ SysIsFile(hFile) then do
    foundFile = SysSearchPath("PATH", hFile)
    if foundFile == "" then do
      msg = 'Unable to find header file "'hFile'"'
      j = MessageDialog(msg, 0, "File Not Found Error", "OK", "WARNING", "SYSTEMMODAL")
      return .false
    end
    hFile = foundFile
  end

  f = .stream~new(hFile)
  ret = f~open(read)
  if ret \= "READY:" then do
    msg = 'Header file "'hFile'" is not readable:' ret
    j = MessageDialog(msg, 0, "File Open Error", "OK", "WARNING", "SYSTEMMODAL")
    return .false
  end

  global = (self~isA(.ApplicationManager) | .constDirUsage == 'use only')

  do while f~state == "READY"
    line = f~linein~strip('L')

    if line~abbrev("#ifdef") then do
      self~skipThroughIfDefs(f, line)
      iterate
    end

    if line~abbrev("#define") & line~words == 3 then do
      parse var line def symbol numericID .
      if numericID~datatype('W') then do
        if global then .constDir[symbol] = numericID
        else self~constDir[symbol~translate] = numericID
      end
    end
  end
  f~close
  return .true

::method resolveIconID private external "LIBRARY oodialog rsrcUtils_resolveIconID_pvt"
::method resolveSymbolicId unguarded external "LIBRARY oodialog rsrcUtils_resolveResourceID"
::method skipThroughIfDefs private
  use arg fObj, line

  if \ line~abbrev("#ifdef") & \ line~abbrev("#ifndef") then return
  ifDefStack = .queue~new
  ifDefStack~push(line)
  do while fObj~state == "READY"
    l = fObj~linein~strip("L")
    select
      when l~abbrev("#endif") then ifDefStack~pull
      when l~abbrev("#ifdef") then ifDefStack~push(l)
      when l~abbrev("#ifndef") then ifDefStack~push(l)
      otherwise nop
    end
    if ifDefStack~items == 0 then leave
  end


/**
 * Return the symbolic ID from the ConstDir that matches the numeric ID.
 * The docs will advise users of ooDialog to use unique numbers for all
 * resources.
 */
::method resolveNumericID unguarded
  use strict arg numericID
  if \ numericID~datatype('W') then return -1

  if self~isA(.ApplicationManager) | .constDirUsage == "use only" then do
    symbol = .constDir~index(numericID)
  end
  else do
    if .constDirUsage == "use first" then do
      symbol = .constDir~index(numericID)
      if symbol == .nil then symbol = self~constDir~index(numericID)
    end
    else if .constDirUsage == "use last" then do
      symbol = self~constDir~index(numericID)
      if symbol == .nil then symbol = .constDir~index(numericID)
    end
    else do
      symbol = self~constDir~index(numericID)
    end
  end

  if symbol == .nil then return -1
  else return symbol


::class 'ApplicationManager' public inherit ResourceUtils

::constant pkg (self~package)
::attribute srcDir get external "LIBRARY oodialog app_srcDir_atr"

::method init external "LIBRARY oodialog app_init"

::method addToConstDir external "LIBRARY oodialog app_addToConstDir"
::method autoDetection external "LIBRARY oodialog app_autoDetection"
::method defaultFont external "LIBRARY oodialog app_defaultFont"
::method defaultIcon external "LIBRARY oodialog app_defaultIcon"
::method initAutoDetection external "LIBRARY oodialog app_initAutoDetection"  -- Internal use only.
::method requiredOS external "LIBRARY oodialog app_requiredOS"
::method setDefaults external "LIBRARY oodialog app_setDefaults"
::method useGlobalConstDir external "LIBRARY oodialog app_useGlobalConstDir"

::class 'DlgUtil' public
::method init class external "LIBRARY oodialog dlgutil_init_cls"
::method and class external "LIBRARY oodialog dlgutil_and_cls"
::method comctl32Version class external "LIBRARY oodialog dlgutil_comctl32Version_cls"
::method errMsg class external "LIBRARY oodialog dlgutil_errMsg_cls"
::method getGUID class external "LIBRARY oodialog dlgutil_getGuid_cls"
::method getSystemMetrics class external "LIBRARY oodialog dlgutil_getSystemMetrics_cls"
::method halt class external "LIBRARY oodialog dlgutil_halt_cls"
::method handleToPointer class external "LIBRARY oodialog dlgutil_handleToPointer_cls"
::method hiWord class external "LIBRARY oodialog dlgutil_hiWord_cls"
::method loWord class external "LIBRARY oodialog dlgutil_loWord_cls"
::method makeLParam class external "LIBRARY oodialog dlgutil_makeLPARAM_cls"
::method makeWParam class external "LIBRARY oodialog dlgutil_makeWPARAM_cls"
::method or class external "LIBRARY oodialog dlgutil_or_cls"
::method screenArea class external "LIBRARY oodialog dlgutil_screenArea_cls"
::method screenSize class external "LIBRARY oodialog dlgutil_screenSize_cls"
::method shiftLeft class external "LIBRARY oodialog dlgutil_shiftLeft_cls"
::method shiftRight class external "LIBRARY oodialog dlgutil_shiftRight_cls"
::method sHiWord class external "LIBRARY oodialog dlgutil_shiWord_cls"
::method signed class external "LIBRARY oodialog dlgutil_signed_cls"
::method signed32 class external "LIBRARY oodialog dlgutil_signed32_cls"
::method sLoWord class external "LIBRARY oodialog dlgutil_sloWord_cls"
::method sShiftLeft class external "LIBRARY oodialog dlgutil_sShiftLeft_cls"
::method sShiftRight class external "LIBRARY oodialog dlgutil_sShiftRight_cls"
::method terminate class external "LIBRARY oodialog dlgutil_terminate_cls"
::method threadID class external "LIBRARY oodialog dlgutil_threadID_cls"
::method unsigned class external "LIBRARY oodialog dlgutil_unsigned_cls"
::method unsigned32 class external "LIBRARY oodialog dlgutil_unsigned32_cls"
::method version class external "LIBRARY oodialog dlgutil_version_cls"
::method windowFromPoint class external "LIBRARY oodialog dlgutil_windowFromPoint_cls"

::method test class external "LIBRARY oodialog dlgutil_test_cls"

::class 'SPI' public
::constant WHEEL_PAGESCROLL   4294267295
::method init class external "LIBRARY oodialog spi_init_cls"

::attribute dragHeight get class external "LIBRARY oodialog spi_getDragHeight_cls"
::attribute dragHeight set class external "LIBRARY oodialog spi_setDragHeight_cls"
::attribute dragWidth get class external "LIBRARY oodialog spi_getDragWidth_cls"
::attribute dragWidth set class external "LIBRARY oodialog spi_setDragWidth_cls"
::attribute menuAnimation get class external "LIBRARY oodialog spi_getMenuAnimation_cls"
::attribute menuAnimation set class external "LIBRARY oodialog spi_setMenuAnimation_cls"
::attribute menuFade get class external "LIBRARY oodialog spi_getMenuFade_cls"
::attribute menuFade set class external "LIBRARY oodialog spi_setMenuFade_cls"
::attribute mouseHoverHeight get class external "LIBRARY oodialog spi_getMouseHoverHeight_cls"
::attribute mouseHoverHeight set class external "LIBRARY oodialog spi_getMouseHoverHeight_cls"
::attribute mouseHoverTime get class external "LIBRARY oodialog spi_getMouseHoverTime_cls"
::attribute mouseHoverTime set class external "LIBRARY oodialog spi_getMouseHoverTime_cls"
::attribute mouseHoverWidth get class external "LIBRARY oodialog spi_getMouseHoverWidth_cls"
::attribute mouseHoverWidth set class external "LIBRARY oodialog spi_getMouseHoverWidth_cls"
::attribute nonClientMetrics get class external "LIBRARY oodialog spi_getNonClientMetrics_cls"  -- For 4.2.2
::attribute nonClientMetrics set class external "LIBRARY oodialog spi_setNonClientMetrics_cls"  -- For 4.2.2

::attribute updateFlag get class external "LIBRARY oodialog spi_getUpdateFlag_cls"
::attribute updateFlag set class external "LIBRARY oodialog spi_setUpdateFlag_cls"

::attribute wheelScrollLines get class external "LIBRARY oodialog spi_getWheelScrollLines_cls"
::attribute wheelScrollLines set class external "LIBRARY oodialog spi_setWheelScrollLines_cls"
::attribute workArea get class external "LIBRARY oodialog spi_getWorkArea_cls"
::attribute workArea set class external "LIBRARY oodialog spi_setWorkArea_cls"

::class 'SM' public
::attribute cMouseButtons get class external "LIBRARY oodialog sm_cMouseButtons_cls"
::attribute cxCursor get class external      "LIBRARY oodialog sm_cxCursor_cls"
::attribute cxDrag get class external        "LIBRARY oodialog sm_cxDrag_cls"
::attribute cxFixedFrame get class external  "LIBRARY oodialog sm_cxFixedFrame_cls"
::attribute cxIcon get class external        "LIBRARY oodialog sm_cxIcon_cls"
::attribute cxScreen get class external      "LIBRARY oodialog sm_cxScreen_cls"
::attribute cxSize get class external        "LIBRARY oodialog sm_cxSize_cls"
::attribute cxSmIcon get class external      "LIBRARY oodialog sm_cxSmIcon_cls"
::attribute cxVscroll get class external     "LIBRARY oodialog sm_cxVScroll_cls"
::attribute cyCaption get class external     "LIBRARY oodialog sm_cyCaption_cls"
::attribute cyCursor get class external      "LIBRARY oodialog sm_cyCursor_cls"
::attribute cyDrag get class external        "LIBRARY oodialog sm_cyDrag_cls"
::attribute cyFixedFrame get class external  "LIBRARY oodialog sm_cyFixedFrame_cls"
::attribute cyHscroll get class external     "LIBRARY oodialog sm_cyHScroll_cls"
::attribute cyIcon get class external        "LIBRARY oodialog sm_cyIcon_cls"
::attribute cyMenu get class external        "LIBRARY oodialog sm_cyMenu_cls"
::attribute cyScreen get class external      "LIBRARY oodialog sm_cyScreen_cls"
::attribute cySize get class external        "LIBRARY oodialog sm_cySize_cls"
::attribute cySmIcon get class external      "LIBRARY oodialog sm_cySmIcon_cls"
::attribute menuDropAlignment get class external "LIBRARY oodialog sm_menuDropAlignment_cls"

::class 'OS' public
::method is64bit class external "LIBRARY oodialog os_is64bit"
::method is32on64bit class external "LIBRARY oodialog os_is32on64bit"
::method isWow64 class external "LIBRARY oodialog os_is32on64bit"
::method isW2K class external "LIBRARY oodialog os_isVersion"
::method isXP class external "LIBRARY oodialog os_isVersion"
::method isXP32 class external "LIBRARY oodialog os_isVersion"
::method isXP64 class external "LIBRARY oodialog os_isVersion"
::method isW2K3 class external "LIBRARY oodialog os_isVersion"
::method isVista class external "LIBRARY oodialog os_isVersion"
::method isServer2008 class external "LIBRARY oodialog os_isVersion"
::method isWindows7 class external "LIBRARY oodialog os_isVersion"
::method isServer2008R2 class external "LIBRARY oodialog os_isVersion"
::method isAtLeastW2K class external "LIBRARY oodialog os_isVersion"
::method isAtLeastXP class external "LIBRARY oodialog os_isVersion"
::method isAtLeastW2K3 class external "LIBRARY oodialog os_isVersion"
::method isAtLeastVista class external "LIBRARY oodialog os_isVersion"
::method isAtLeastWindows7 class external "LIBRARY oodialog os_isVersion"
::method settingChanged class external "LIBRARY oodialog os_settingChanged"
::method shellChangeNotify class external "LIBRARY oodialog os_shellChangeNotify"


::class 'Mouse' public
::constant HOVER_DEFAULT           4294967295

::method new class external "LIBRARY oodialog mouse_new_cls"
::method init external "LIBRARY oodialog mouse_init"

::method doubleClickTime class external "LIBRARY oodialog mouse_doubleClickTime_cls"
::method loadCursor class external "LIBRARY oodialog mouse_loadCursor_cls"
::method loadCursorFromFile class external "LIBRARY oodialog mouse_loadCursorFromFile_cls"
::method setDoubleClickTime class external "LIBRARY oodialog mouse_setDoubleClickTime_cls"
::method swapButton class external "LIBRARY oodialog mouse_swapButton_cls"

::method appStarting external "LIBRARY oodialog mouse_setCursor"
::method arrow external "LIBRARY oodialog mouse_setCursor"
::method capture external "LIBRARY oodialog mouse_capture"
::method clipCursor external "LIBRARY oodialog mouse_clipCursor"
::method connectEvent external "LIBRARY oodialog mouse_connectEvent"
::method cross external "LIBRARY oodialog mouse_setCursor"
::method dragDetect external "LIBRARY oodialog mouse_dragDetect"
::method getCapture external "LIBRARY oodialog mouse_get_release_capture"
::method getClipCursor external "LIBRARY oodialog mouse_getClipCursor"
::method getCursorPos external "LIBRARY oodialog mouse_getCursorPos"
::method isButtonDown external "LIBRARY oodialog mouse_isButtonDown"
::method no external "LIBRARY oodialog mouse_setCursor"
::method releaseCapture external "LIBRARY oodialog mouse_get_release_capture"
::method releaseClipCursor external "LIBRARY oodialog mouse_releaseClipCursor"
::method restoreCursor external "LIBRARY oodialog mouse_restoreCursor"
::method setCursor external "LIBRARY oodialog mouse_setCursor"
::method setCursorPos external "LIBRARY oodialog mouse_setCursorPos"
::method showCursor external "LIBRARY oodialog mouse_showCursor"
::method trackEvent external "LIBRARY oodialog mouse_trackEvent"
::method wait external "LIBRARY oodialog mouse_setCursor"

::method test external "LIBRARY oodialog mouse_test"

::class 'Keyboard' public
::method getAsyncKeyState class external "LIBRARY oodialog kb_getAsyncKeyState_cls"

::class 'DayState' public
::method init external "LIBRARY oodialog ds_init"
::method value external "LIBRARY oodialog ds_value"


::class 'DayStates' public

::method makeDayStateBuffer class external "LIBRARY oodialog dss_makeDayStateBuffer"
::method quickDayStateBuffer class external "LIBRARY oodialog dss_quickDayStateBuffer"

::attribute years private

::attribute startMonth get
::attribute startMonth set private

::attribute endMonth get
::attribute endMonth set private

::method init
    expose years startMonth endMonth
    use strict arg year = (.DateTime~today~year - 2), count = 3

    startMonth = .DateTime~fromStandardDate(year || '0101')
    endMonth = .DateTime~fromStandardDate((year + count - 1) || '1201')

    years = .table~new
    do i = year to year + count - 1
        years[i] = self~generateEmptyYear
    end

::method getDayState
    expose startMonth years
    use strict arg dateTime

    year = dateTime~year
    month = dateTime~month

    aYear = years[year]
    if aYear == .nil then return .nil
    else return aYear[month]

::method getDayStateBuffer
    expose startMonth years
    use strict arg dateTime, count

    year = dateTime~year
    month = dateTime~month

    resultArray = .array~new(count)

    aYear = years[year]
    if aYear == .nil then do
        do i = 1 to count
            resultArray[i] = .DayState~new
        end
        return .DayStates~makeDayStateBuffer(resultArray)
    end

    do i = 1 to count
        resultArray[i] = aYear[month]
        month += 1
        if month > 12 then do
            month = 1
            year += 1
            aYear = years[year]
            if aYear == .nil then leave
        end
    end

    if resultArray~items < count then do i = (resultArray~items + 1) to count
        resultArray[i] = .DayState~new
    end

    return .DayStates~makeDayStateBuffer(resultArray)

::method putYear
    expose startMonth endMonth years
    use strict arg dateTime, values

    year = dateTime~year

    newYear = .DateTime~fromStandardDate(year || '0101')
    months = .array~new(12)
    do i = 1 to 12
        if values[i] == .nil then months[i] = .DayState~new
        else months[i] = values[i]
    end

    years[year] = months

    needFill = .false
    if newYear < startMonth then do
        startMonth = newYear
        if years[year + 1] == .nil then needFill = .true
    end
    else if newYear > endMonth then do
        endMonth = .DateTime~fromStandardDate(year || '1201')
        if years[year - 1] == .nil then needFill = .true
    end

    if needFill then self~doFill

::method putMonth
    expose startMonth endMonth years
    use strict arg dateTime, dayState

    year = dateTime~year
    month = dateTime~month

    yearArray = years[year]
    if yearArray == .nil then do
        yearArray = .array~new(12)
        yearArray[month] = dayState
        self~putYear(dateTime, yearArray)
    end
    else do
        yearArray[month] = dayState
    end

::method generateEmptyYear private
    empty = .array~new(12)
    do i = 1 to 12
        empty[i] = .DayState~new
    end
    return empty

::method needFill private
    expose startMonth endMonth years

    do i = startMonth~year to endMonth~year
        if years[i] == .nil then years[i] = self~generateEmptyYear
    end


::class 'VK' mixinclass Object public

-- Virtual Keys, Standard Set.  Note that several of the integer values have
-- duplicate names.
::constant LBUTTON                1
::constant RBUTTON                2
::constant CANCEL                 3
::constant MBUTTON                4
::constant XBUTTON1               5
::constant XBUTTON2               6
                            --    7 : unassigned
::constant BACK                   8
::constant TAB                    9
                            --   10 : reserved
                            --   11 : reserved
::constant CLEAR                 12
::constant RETURN                13
                            --   14 : Microsoft does not list.
                            --   15 : Microsoft does not list.
::constant SHIFT                 16
::constant CONTROL               17
::constant MENU                  18
::constant PAUSE                 19
::constant CAPITAL               20
::constant KANA                  21
::constant HANGEUL               21
::constant HANGUL                21
                            --   22 : Microsoft does not list.
::constant JUNJA                 23
::constant FINAL                 24
::constant HANJA                 25
::constant KANJI                 25
                            --   26 : Microsoft does not list.
::constant ESCAPE                27
::constant CONVERT               28
::constant NONCONVERT            29
::constant ACCEPT                30
::constant MODECHANGE            31
::constant SPACE                 32
::constant PRIOR                 33
::constant NEXT                  34
::constant END                   35
::constant HOME                  36
::constant LEFT                  37
::constant UP                    38
::constant RIGHT                 39
::constant DOWN                  40
::constant SELECT                41
::constant PRINT                 42
::constant EXECUTE               43
::constant SNAPSHOT              44
::constant INSERT                45
::constant DELETE                46
::constant HELP                  47
::constant 0                     48
::constant 1                     49
::constant 2                     50
::constant 3                     51
::constant 4                     52
::constant 5                     53
::constant 6                     54
::constant 7                     55
::constant 8                     56
::constant 9                     57
                            -- 58 - 63 (0x3A -0x3F) Microsoft does not list.
                            -- 0x40 : unassigned
::constant A                     65
::constant B                     66
::constant C                     67
::constant D                     68
::constant E                     69
::constant F                     70
::constant G                     71
::constant H                     72
::constant I                     73
::constant J                     74
::constant K                     75
::constant L                     76
::constant M                     77
::constant N                     78
::constant O                     79
::constant P                     80
::constant Q                     81
::constant R                     82
::constant S                     83
::constant T                     84
::constant U                     85
::constant V                     86
::constant W                     87
::constant X                     88
::constant Y                     89
::constant Z                     90
::constant LWIN                  91
::constant RWIN                  92
::constant APPS                  93
                            -- 0x5E : reserved
::constant SLEEP                 95
::constant NUMPAD0               96
::constant NUMPAD1               97
::constant NUMPAD2               98
::constant NUMPAD3               99
::constant NUMPAD4              100
::constant NUMPAD5              101
::constant NUMPAD6              102
::constant NUMPAD7              103
::constant NUMPAD8              104
::constant NUMPAD9              105
::constant MULTIPLY             106
::constant ADD                  107
::constant SEPARATOR            108
::constant SUBTRACT             109
::constant DECIMAL              110
::constant DIVIDE               111
::constant F1                   112
::constant F2                   113
::constant F3                   114
::constant F4                   115
::constant F5                   116
::constant F6                   117
::constant F7                   118
::constant F8                   119
::constant F9                   120
::constant F10                  121
::constant F11                  122
::constant F12                  123
::constant F13                  124
::constant F14                  125
::constant F15                  126
::constant F16                  127
::constant F17                  128
::constant F18                  129
::constant F19                  130
::constant F20                  131
::constant F21                  132
::constant F22                  133
::constant F23                  134
::constant F24                  135
                            -- 0x88 - 0x8F : unassigned
::constant NUMLOCK              144
::constant SCROLL               145

-- NEC PC-9800 kbd definitions
::constant OEM_NEC_EQUAL        146   -- '=' key on numpad

-- Fujitsu/OASYS kbd definitions
::constant OEM_FJ_JISHO         146   -- 'Dictionary' key
::constant OEM_FJ_MASSHOU       147   -- 'Unregister word' key
::constant OEM_FJ_TOUROKU       148   -- 'Register word' key
::constant OEM_FJ_LOYA          149   -- 'Left OYAYUBI' key
::constant OEM_FJ_ROYA          150   -- 'Right OYAYUBI' key
                            -- 0x97 - 0x9F : unassigned

::constant LSHIFT               160
::constant RSHIFT               161
::constant LCONTROL             162
::constant RCONTROL             163
::constant LMENU                164
::constant RMENU                165
::constant BROWSER_BACK         166
::constant BROWSER_FORWARD      167
::constant BROWSER_REFRESH      168
::constant BROWSER_STOP         169
::constant BROWSER_SEARCH       170
::constant BROWSER_FAVORITES    171
::constant BROWSER_HOME         172
::constant VOLUME_MUTE          173
::constant VOLUME_DOWN          174
::constant VOLUME_UP            174
::constant MEDIA_NEXT_TRACK     176
::constant MEDIA_PREV_TRACK     177
::constant MEDIA_STOP           178
::constant MEDIA_PLAY_PAUSE     179
::constant LAUNCH_MAIL          180
::constant LAUNCH_MEDIA_SELECT  181
::constant LAUNCH_APP1          182
::constant LAUNCH_APP2          183
                            -- 0xB8 - 0xB9 : reserved

::constant OEM_1                186   -- ';:' for US
::constant OEM_PLUS             187   -- '+' any country
::constant OEM_COMMA            188   -- ',' any country
::constant OEM_MINUS            189   -- '-' any country
::constant OEM_PERIOD           190   -- '.' any country
::constant OEM_2                191   -- '/?' for US
::constant OEM_3                192   -- '`~' for US
                            -- 0xC1 - 0xD7 : reserved
                            -- 0xD8 - 0xDA : unassigned

::constant OEM_4                219  --  '[{' for US
::constant OEM_5                220  --  '\|' for US
::constant OEM_6                221  --  ']}' for US
::constant OEM_7                222  --  ''"' for US
::constant OEM_8                223
                            -- 0xE0 : reserved

-- Various ended or enhanced keyboards
::constant OEM_AX               225  --  'AX' key on Japanese AX kbd
::constant OEM_102              226  --  "<>" or "\|" on RT 102-key kbd.
::constant ICO_HELP             227  --  Help key on ICO
::constant ICO_00               228  --  00 key on ICO
::constant PROCESSKEY           229
::constant ICO_CLEAR            230
::constant PACKET               231
                            -- 0xE8 : unassigned

-- Nokia/Ericsson definitions
::constant OEM_RESET            233
::constant OEM_JUMP             234
::constant OEM_PA1              235
::constant OEM_PA2              236
::constant OEM_PA3              237
::constant OEM_WSCTRL           238
::constant OEM_CUSEL            239
::constant OEM_ATTN             240
::constant OEM_FINISH           241
::constant OEM_COPY             242
::constant OEM_AUTO             243
::constant OEM_ENLW             244
::constant OEM_BACKTAB          245
::constant ATTN                 246
::constant CRSEL                247
::constant EXSEL                248
::constant EREOF                249
::constant PLAY                 250
::constant ZOOM                 251
::constant NONAME               252
::constant PA1                  253
::constant OEM_CLEAR            254
                            -- 0xFF : reserved

::method key2name class external "LIBRARY oodialog vk_key2name"
::method key2name       external "LIBRARY oodialog vk_key2name"


::class 'OEM' mixinclass Object public                                                 -- Do not document for 4.2.0

::constant OBM_CLOSE           32754
::constant OBM_UPARROW         32753
::constant OBM_DNARROW         32752
::constant OBM_RGARROW         32751
::constant OBM_LFARROW         32750
::constant OBM_REDUCE          32749
::constant OBM_ZOOM            32748
::constant OBM_RESTORE         32747
::constant OBM_REDUCED         32746
::constant OBM_ZOOMD           32745
::constant OBM_RESTORED        32744
::constant OBM_UPARROWD        32743
::constant OBM_DNARROWD        32742
::constant OBM_RGARROWD        32741
::constant OBM_LFARROWD        32740
::constant OBM_MNARROW         32739
::constant OBM_COMBO           32738
::constant OBM_UPARROWI        32737
::constant OBM_DNARROWI        32736
::constant OBM_RGARROWI        32735
::constant OBM_LFARROWI        32734
::constant OBM_SIZE            32766
::constant OBM_BTSIZE          32761
::constant OBM_CHECK           32760
::constant OBM_CHECKBOXES      32759
::constant OBM_BTNCORNERS      32758

::constant OCR_NORMAL          32512
::constant OCR_IBEAM           32513
::constant OCR_WAIT            32514
::constant OCR_CROSS           32515
::constant OCR_UP              32516
::constant OCR_SIZENWSE        32642
::constant OCR_SIZENESW        32643
::constant OCR_SIZEWE          32644
::constant OCR_SIZENS          32645
::constant OCR_SIZEALL         32646
::constant OCR_NO              32648
::constant OCR_HAND            32649
::constant OCR_APPSTARTING     32650

::constant OIC_SAMPLE          32512
::constant OIC_HAND            32513
::constant OIC_QUES            32514
::constant OIC_BANG            32515
::constant OIC_NOTE            32516
::constant OIC_WINLOGO         32517
::constant OIC_WARNING         32515
::constant OIC_ERROR           32513
::constant OIC_INFORMATION     32516
::constant OIC_SHIELD          32518

::constant IDI_APPLICATION     32512
::constant IDI_HAND            32513
::constant IDI_QUESTION        32514
::constant IDI_EXCLAMATION     32515
::constant IDI_ASTERISK        32516
::constant IDI_WINLOGO         32517
::constant IDI_SHIELD          32518
::constant IDI_WARNING         32515
::constant IDI_ERROR           32513
::constant IDI_INFORMATION     32516


/**
 * Standard Dialog Control Classes.  Some simple short cut dialogs.
 */

-- Create a "default" dialog class and set it to the user dialog.
::class 'Dialog' subclass UserDialog public


-- This class shows a message window for a defined duration
::class 'TimedMessage' subclass UserDialog public

::method init public external "LIBRARY oodialog timedmsg_init"

::method defineDialog
   expose message sx sy
   self~createStaticText(-1, 10, 10, sx+2, sy, , message)

::method initAutoDetection
   self~noAutoDetection

::method execute unguarded
   expose message title sleeping earlyReply sx sy pos

   s = self~getTextSizeDu(message)
   sx = s~width
   sy = s~height
   self~createCenter(sx+20, sy + 20, title, "", , , , 1)
   self~executeAsync( , 'SHOWTOP')
   if sleeping >= 0 then do
      if earlyReply then reply
      ret = msSleep(sleeping)
      self~start("endAsyncExecution")
      self~ok:super
   end
   else do
      self~start("endAsyncExecution")
   end

::method initDialog
   expose pos

   if pos \== .nil then self~moveTo(pos)
   .SystemMenu~new(self)~disable(.SystemMenu~SC_CLOSE)


-- This class provides a simple dialog with a title, a message, one entry line,
-- an OK and a Cancel push button.
::class 'InputBox' subclass UserDialog public

::constant  IDC_EDIT   101
::attribute sx
::attribute sy
::attribute editText unguarded

::method init
   expose message title datas. size
   use arg message, title, default = "", size = 0
   self~init:super
   self~editText = default

::method defineDialog
   expose message size dlgy
   if size = 0 then size = self~sx
   self~createStaticText(-1, 10, 10, self~sx + 2, self~sy, , message)
   self~addLine(10, self~sy + 4 + 10, size)
   self~createPushButtonGroup(self~sx-80, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method addLine
   use arg x, y, l
   self~createEdit(self~IDC_EDIT, x, y, l)

::method execute
   expose message title dlgy size

   s = self~getTextSizeDu(message)
   self~sx = max(s~width, 100, size)
   self~sy = s~height

   dlgy = (self~sy + 4) * 2 + 36
   dlgx = self~sx + 20
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,,4)
   self~execute:super("SHOWTOP")
   if self~initCode = 1 then return self~editText
   else return ""

::method initDialog
   self~newEdit(self~IDC_EDIT)~setText(self~editText)

::method ok unguarded
   self~editText = self~newEdit(self~IDC_EDIT)~getText
   return self~ok:super

::method initAutoDetection
   self~noAutoDetection


-- A subclass of an InputBox dialog that masks the user input. Suitable for
-- asking the user for a password.
::class 'PasswordBox' subclass InputBox public

::method addLine
   use arg x, y, l
   self~createPasswordEdit(self~IDC_EDIT, x, y, l)


-- A subclass of an InputBox dialog that only accepts numerical data.
::class 'IntegerBox' subclass InputBox public

::method validate
   text = self~newEdit(self~IDC_EDIT)~getText
   if \ text~dataType('W') then do
      msg = "You did not enter numerical data"
      title = 'Data Input Error'
      j = MessageDialog(msg, self~hwnd, title, 'OK', 'ERROR')
      return .false
   end
   else return .true


::class 'MultiInputBox' subclass UserDialog public

-- This class is too hard to implement without automatic data detection and
-- still retain backwards compatibility.  So, we explicitly set it to on for
-- this dialog.  That way it will work if the user has globaly set it to off or
-- on.
::method initAutoDetection
   self~autoDetection

::method init
   expose message title datas. entries. len
   use arg message, title, entries., datas., len
   if arg(5,"o") = 1 then len = 0
   self~init:super(datas.)

::method defineDialog
   expose message sx sy len dlgx dlgy entries. maxlen
   self~createStaticText(-1, 10, 10, sx+2, sy, , message)
   self~createEditInputStem(101, 10, sy+4+10, maxlen+4, len, entries.)
   self~createPushButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method execute
   expose message title sx sy datas. dlgx dlgy len entries. maxlen

   s = self~getTextSizeDu(message)
   sx = s~width
   sy = s~height
   ll = 0; count = 0

   do while var("entries."count+1) = 1
      count = count + 1
      s = self~getTextSizeDu(entries.count)
      if s~width > ll then ll = s~width
   end

   if len = 0 then len = max(sx - ll, 50)
   maxlen = ll
   dlgy = count * (sy * 2) + trunc((sy*count)/4) + 56

   if sx >= len + ll + 4 then dlgx = (sx+24)
   else dlgx = (len + ll + 24)
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,, count*2+5)
   self~execute:super("SHOWTOP")
   return self~initCode


::class 'ListChoice' subclass UserDialog public

::constant  IDC_LB   101
::attribute sx
::attribute sy
::attribute dlgx
::attribute dlgy
::attribute lenx
::attribute leny
::attribute message
::attribute useVScroll
::attribute lbSelection unguarded

::method init
   expose message title input. lenx leny
   use arg message, title, input., lenx = 0, leny = 0, predev = ''

   self~init:super
   self~lbSelection = predev
   s = self~getTextSizeDu(message)
   self~sx = s~width
   self~sy = s~height
   self~message = message
   self~useVScroll = .false

::method defineDialog
   expose message sx sy lenx leny dlgx dlgy
   self~createStaticText(-1, 10, 10, sx + 2, sy, , message)
   if self~useVScroll then style = "VSCROLL"
   else style = "PARTIAL"
   self~createListBox(self~IDC_LB, 10, sy + 4 + 10, lenx, leny+4, style)
   self~createPushButtonGroup(dlgx - 100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method initDialog
   expose count input.
   do i = 1 to count
      self~addListEntry(self~IDC_LB, input.i)
   end
   self~setListBoxData(self~IDC_LB, self~lbSelection)

::method execute
   expose message title sx sy input. count lenx leny dlgx dlgy useVScroll

   s = self~getTextSizeDu(message)
   sx = s~width
   sy = s~height

   ll = 0; count = 0
   do while var("input."count+1) = 1
      count = count + 1
      s = self~getTextSizeDu(input.count)
      if s~width > ll then ll = s~width
   end

   goodHeight = (count * self~sy) + 1

   if self~lenx = 0 then self~lenx = max(self~sx,ll+5)
   if self~leny = 0 then do
      if count > 15 then do
         self~leny = self~sy * 15
         useVScroll = .true
         self~useVScroll = .true
         self~recalcListBoxWidth(ll + 5)
      end
      else do
         self~leny = goodHeight
      end
   end
   else do
      if self~leny < goodHeight then do
         self~leny = self~sy * 15
         useVScroll = .true
         self~recalcListBoxWidth(ll + 5)
      end
   end

   self~dlgy = self~leny+(self~sy+4)+34

   if self~sx >= self~lenx then self~dlgx = self~sx + 20; else self~dlgx = self~lenx + 20
   if self~dlgx < 110 then self~dlgx = 110

   self~createCenter(self~dlgx, self~dlgy, title,,,,, 4)
   self~execute:super("SHOWTOP")

   if self~initCode = 1 then return self~lbSelection
   else return ""

::method recalcListBoxWidth private
   use strict arg neededLBWidth
   sbWidth = self~getScrollBarWidth
   tmpLenX = neededLBWidth + sbWidth
   if tmpLenX > self~sx then self~lenx = tmpLenX

::method getScrollBarWidth private
   SM_CXVSCROLL = 20
   size = .Size~new
   size~width = .DlgUtil~getSystemMetrics(SM_CXVSCROLL)
   self~pixel2dlgUnit(size)
   if size~width > 15 then return size~width + 2
   else return size~width

::method ok unguarded
   self~lbSelection = self~newListBox(self~IDC_LB)~selected
   return self~ok:super

::method initAutoDetection
   self~noAutoDetection



::class 'MultiListChoice' subclass ListChoice public

::method defineDialog
   if self~useVScroll then style = "MULTI VSCROLL TABULATORS"
   else style = "MULTI PARTIAL TABULATORS"
   self~createStaticText(-1, 10, 10, self~sx+2, self~sy, , self~message)
   self~createListBox(101, 10, self~sy+4+10, self~lenx, self~leny+4, style, "DATA")
   self~createPushButtonGroup(self~dlgx-100, self~dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method ok unguarded
   self~lbSelection = self~newListBox(self~IDC_LB)~selectedIndexes
   return self~ok:.UserDialog



::class 'CheckList' subclass UserDialog public

  /* Entries is a stem variable with all the titles for the check boxes */
  /* Datas is a stem variable you can use to preselect the check boxes */
  /* 'Datas.i=1' means there will be a check mark on the i-th box */
::method init
   expose message title datas. entries. len max
   use arg message, title, entries., datas., len, max
   if arg(5,"o") = 1 then len = 0
   if arg(6,"o") = 1 then max = 0
   self~init:super(datas.)

-- Punt on this class and use automatic data detection to retain backwards
-- compatibility.  We explicitly set it to on for this dialog.  That way it will
-- work if the user has globaly set it to off or on.
::method initAutoDetection
   self~autoDetection

::method defineDialog
   expose message sx sy len dlgx dlgy entries. placex max
   self~createStaticText(-1, 10, 10, sx+2, sy, , message)
   self~createCheckBoxStem(101, placex, sy+4+10, len, entries., max)
   self~createPushButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method execute
   expose message title sx sy datas. dlgx dlgy len entries. placex max

   s = self~getTextSizeDu(message)
   sx = s~width
   sy = s~height
   ll = 0; count = 0
   do while var("entries."count+1) = 1
      count = count + 1
      s = self~getTextSizeDu(entries.count)
      if s~width > ll then ll = s~width
   end

   if max = 0 then max = count
   dlgy = max * trunc(sy * 1.5) + sy + 60
   placex = 10
   columns = (count % max)
   if  (count // max) > 0 then columns = columns +1

   if sx >= (ll + 20) * columns then
   do
      dlgx = (sx+24)
      if columns = 1 then placex = trunc((dlgx - ll - 20) /2)
   end
   else dlgx = (ll + 20) * columns + 24
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,, count+5)
   self~execute:super("SHOWTOP")
   return self~initCode



::class 'SingleSelection' subclass UserDialog public

::constant  MIN_DLGCX   110  -- minimum size to accomdate ok / cancel buttons
::constant  MIN_RB_ID   101  -- first radio button resource ID

::attribute selected unguarded

::method init
    expose message title entries. preSel len max
    use strict arg  message, title, entries., preSel = 1, len = 0, max = 0
    self~init:super
    self~selected = self~MIN_RB_ID - 1 + preSel

::method defineDialog
    expose entries. dlgCX dlgCY msgCX msgCY message groupBoxX staticX len max

    buttonTxt = "&Ok 1 OK &Cancel 2 CANCEL"

    self~createStaticText(-1, staticX, 10, msgCX + 2, msgCY, , message)
    self~createRadioButtonStem(self~MIN_RB_ID, groupBoxX, msgCY + 4 + 10, len, entries., max)
    self~createPushButtonGroup(dlgCX - 100, dlgCY - 18, 0, 0, buttonTxt, 1, "DEFAULT")

::method initDialog
    rb = self~newRadioButton(self~selected)
    if rb == .nil then return
    rb~check

::method execute
    expose dlgCX dlgCY title count

    self~calcSizes
    self~createCenter(dlgCX, dlgCY, title, , , , , count + 5)

    self~execute:super("SHOWTOP")
    if self~initCode = 1 then return (self~selected - self~MIN_RB_ID + 1)
    else return ""

::method ok unguarded
    expose count

    do i = self~MIN_RB_ID to (self~MIN_RB_ID + count - 1)
        if self~newRadioButton(i)~checked then do
            self~selected = i
            leave
       end
    end
    return self~ok:super

::method calcSizes private
    expose entries. max preSel message len msgCX msgCy count dlgCX dlgCY groupBoxX staticX

    s = self~getTextSizeDu(message)
    msgCX = s~width
    msgCY = s~height
    ll = 0; count = 0
    do while var("entries."count + 1) = 1
        count += 1
        if len == 0 then do
            s = self~getTextSizeDu(entries.count)
            if s~width > ll then ll = s~width
        end
    end
    if len <> 0 then ll = len
    else ll += 20

    if max = 0 then max = count
    dlgCY = max * trunc(msgCY * 1.5) + msgCY + 60
    dlgCX = self~getMinDlgCX
    groupBoxX = 10
    staticX = 10
    columns = (count % max)
    if  (count // max) > 0 then columns = columns + 1

    groupBoxCX = ll * columns
    if msgCX >= groupBoxCX then do
        if (msgCX + 24) > dlgCX then dlgCX = (msgCX + 24)
    end
    else do
        if groupBoxCX + 24 > dlgCX then dlgCX = groupBoxCX + 24
    end

    if groupBoxCX + 4 < dlgCX then do
        groupBoxX = trunc((dlgCX - groupBoxCX) / 2)
        if groupBoxCX >= msgCX then staticX = groupBoxX
        else staticX = trunc((dlgCX - msgCX) / 2)
    end

::method getMinDlgCX private
    expose title

    s = self~getTextSizeTitleBar(title)

    padding = 18
    s~width += (2 * .SM~cxFixedFrame) + .SM~cxSize + .SM~cxSmIcon + padding
    self~pixel2dlgUnit(s)

    if self~MIN_DLGCX < s~width then return s~width
    else return self~MIN_DLGCX

::method initAutoDetection
    self~noAutoDetection


::class 'ProgressDialog' public subclass UserDialog

::constant IDC_PB_MAIN    100
::constant IDC_ST_MSG     101
::constant IDC_ST_STATUS  102

::constant BUTTON_CX      35
::constant BUTTON_CY      14
::constant DLG_WIDTH      91  -- 2 x button width + margins + interbutton spacing
::constant CAPTION_TEXT   "Application Processing ..."
::constant INCR_STEP      1
::constant MARQUEE_PAUSE  100
::constant MSG_TEXT       "Background processing is taking place.  Please be patient."
::constant STATUS_TEXT    "Status: "
::constant PADDING_DU     7
::constant RANGE_MAX      100
::constant RANGE_MIN      0

::attribute absoluteWidth
::attribute canCancel
::attribute captionText
::attribute incrStep
::attribute initialStatusText
::attribute marqueeMode
::attribute marqueePause
::attribute msgHeight    -- Number of lines, default 1
::attribute msgText
::attribute noMsg
::attribute noStatus
::attribute rangeMax
::attribute rangeMin
::attribute width
::attribute worker unguarded private

::method init
    use strict arg capt = (self~CAPTION_TEXT), msg = (self~MSG_TEXT), min = (self~RANGE_MIN), max = (self~RANGE_MAX), incr = (self~INCR_STEP)
    self~init:super

    self~captionText = capt
    self~msgText     = msg
    self~rangeMax    = max
    self~rangeMin    = min
    self~incrStep    = incr
    self~setDefaults

::method begin
    expose width height
    use strict arg

    self~calcSizes
    if self~createCenter(width, height, self~captionText) then do
        self~popup('ShowTop')
        ret = 0
    end
    else do
        self~initCode = 1
        msg   = "Failed to create underyling dialog."
        title = 'ooDialog - Unexpected Error'
        r = MessageDialog(msg, , title, 'OK', 'ERROR')
        ret = 1
    end
    return ret

::method complete unguarded
    expose pb
    use strict arg
    if self~initCode <> 0 then return
    if \ pb~isA(.ProgressBar) then return
    if self~marqueeMode then pb~setMarquee(.false)
    else pb~setPos(self~rangeMax)

::method doStatus unguarded private
    expose statusBar
    use strict arg text
    if self~noStatus then return
    if self~initCode <> 0 then return
    guard on when statusBar~isA(.Static)
    statusBar~setText(text)

::method endNow unguarded
    return self~ok:super

::method increase unguarded
    expose pb
    use strict arg incr = (self~incrStep)
    if self~marqueeMode then return
    if self~initCode <> 0 then return
    guard on when pb~isA(.ProgressBar)
    pb~step(incr)

::method setInterruptible unguarded external "LIBRARY oodialog pd_setInterruptible"
::method updateStatus unguarded
    use strict arg text
    self~doStatus("Status:" text)

::method updateStatusText unguarded
    use strict arg text
    self~doStatus(text)

::method calcSizes private
    expose width height lineHeight
    use strict arg

    pad = self~PADDING_DU
    if self~noMsg, self~noStatus then do
        tempText = self~MSG_TEXT
    end
    else if \ self~noMsg, self~msgHeight < 2 then do
        tempText = self~msgText
    end
    else do
        tempText = self~MSG_TEXT
    end
    size = self~getTextSizeDU(tempText)
    lineHeight = size~height

    height = pad
    if \ self~noMsg then do
        height += lineHeight * self~msgHeight
    end
    if \ self~noStatus then do
        height += pad + lineHeight
    end

    if height > pad then height += pad
    height += lineHeight + pad

    if self~canCancel then do
        height += pad + self~BUTTON_CY
    end

    if self~absoluteWidth then do
        width = self~width
    end
    else do
      cx = size~width + (2 * pad)
      if cx >= self~DLG_WIDTH then width = cx
      else width = self~DLG_WIDTH
    end

::method setDefaults private

    self~msgHeight         = 1
    self~noMsg             = .false
    self~noStatus          = .false
    self~initialStatusText = self~STATUS_TEXT
    self~absoluteWidth     = .false
    self~width             = self~DLG_WIDTH
    self~marqueePause      = self~MARQUEE_PAUSE
    self~marqueeMode       = .false
    self~worker            = .nil
    self~canCancel         = .false

::method defineDialog
    expose width height lineHeight
    pad = self~PADDING_DU
    style = 'SMOOTH'
    if self~marqueeMode then style ||= ' MARQUEE'
    cx   = width - 2 * pad
    curY = pad
    if \ self~noMsg then do
      cy = lineheight * self~msgHeight
      self~createStaticText(self~IDC_ST_MSG, pad, curY, cx, cy, , self~msgText)
      curY += cy + pad
    end
    if \ self~noStatus then do
        self~createStaticText(self~IDC_ST_STATUS, pad, curY, cx, lineHeight, , "" )
        curY += lineHeight + pad
    end
    self~createProgressBar(self~IDC_PB_MAIN, pad, curY, cx, lineHeight, style)
    curY += lineHeight + pad
    if self~canCancel then do
        self~createPushButton(IDCANCEL, width - pad - self~BUTTON_CX, curY, self~BUTTON_CX, self~BUTTON_CY, , 'Cancel')
    end

::method initDialog
    expose pb statusBar

    pb = self~newProgressBar(self~IDC_PB_MAIN)
    if self~marqueeMode then do
        pb~setMarquee(.true, self~marqueePause)
    end
    else do
        pb~setRange(self~rangeMin, self~rangeMax)
        pb~setStep(self~incrStep)
        pb~step
    end
    if \ self~noMsg then do
        self~newStatic(self~IDC_ST_MSG)~setText(self~msgText)
    end
    if \ self~noStatus then do
        statusBar = self~newStatic(self~IDC_ST_STATUS)
        statusBar~setText(self~initialStatusText)
    end

-- Don't let the user close the dialog, unless the work can be stopped, or it
-- is allowed.
::method ok unguarded
::method cancel unguarded
    expose worker

    if worker <> .nil then do
        if worker~interruptible then do
            worker~interrupt
            return
        end
        self~giveStatus(.true)
        return
    end
    else if self~canCancel then do
        return self~cancel:super
    end
    self~giveStatus(.false)

::method giveStatus unguarded private
    expose pb worker
    use strict arg tryLater
    reply 0

    if self~marqueeMode then do
        msg = "Processing is not done yet, and can" || .endOfLine || -
              "not be canceled at this time."       || .endOfLine
    end
    else do
        r = pb~getFullRange
        percentDone = trunc((pb~getPos / (r~max - r~min)) * 100)
        msg = "Processing is only" percentDone"% done, and" || .endOfLine -
              "can not be canceled at this time."           || .endOfLine
    end
    if tryLater then do
        msg ||= "(Try canceling later.)" || .endOfLine
    end
    msg ||= .endOfLine || "Please be patient."

    ret = MessageDialog(msg, , self~captionText, 'OK', 'INFORMATION')

::method initAutoDetection
    self~noAutoDetection


::class 'Interruptible' public mixinclass Object
::method interrupt     unguarded abstract
::method interruptible unguarded abstract

::class 'Alerter' public inherit Interruptible
::attribute isCanceled get
::method init
    expose isCanceled
    isCanceled = .false
::method interruptible unguarded
    return .true
::method interrupt unguarded
    expose isCanceled
    isCanceled = .true


/* - - - - - - - - - - - - - Public routines - - - - - - - - - - - - - - - - -*/

::routine askDialog public
   use strict arg info, defaultButton = 'Y'
   miscStyles = "SETFOREGROUND TASKMODAL"
   if defaultButton~left(1)~translate == 'N' then miscStyles ||= ' DEFBUTTON2'
   return (messageDialog(info, , "Question", "YESNO", "QUESTION", miscStyles) == 6)

::routine errorDialog public
   use strict arg info
   return messageDialog(info, , "Error", "OK", "HAND", "SETFOREGROUND TASKMODAL")

::routine fileNameDialog public external "LIBRARY oodialog fileNameDlg_rtn"
::routine findWindow public external "LIBRARY oodialog findWindow_rtn"
::routine infoDialog public
   use strict arg info
   return messageDialog(info, , "Information", "OK", "INFORMATION", "SETFOREGROUND TASKMODAL")

::routine locate public external "LIBRARY oodialog locate_rtn"
::routine messageDialog public external "LIBRARY oodialog messageDialog_rtn"
::routine msSleep public external "LIBRARY oodialog msSleep_rtn"
::routine play public external "LIBRARY oodialog play_rtn"
::routine screenSize public
   return .DlgUtil~screenSize

::routine simpleFolderBrowse public external "LIBRARY oodialog simpleFolderBrowse_rtn"
::routine winTimer public external "LIBRARY oodialog winTimer_rtn"
::routine routineTest public external "LIBRARY oodialog routineTest_rtn"  -- internal use only


/** The following routines are, mostly, duplicates of the public routines above.
 *  They are needed to maintain backward compatibilty with  pre-4.0.0, because
 *  both names were documented.  They are all deprecated.
 */
/* ----- Start deprecated routines. ------------------------------------------*/
::routine binaryAnd public
  use strict arg n1, n2
  return .DlgUtil~and(n1, n2)

::routine errorMessage public
   use strict arg info
   return messageDialog(info, , "Error", "OK", "HAND", "SETFOREGROUND TASKMODAL")

::routine getFileNameWindow public external "LIBRARY oodialog fileNameDlg_rtn"
::routine getScreenSize public
  vals = .DlgUtil~screenSize
  return vals[1] vals[2] vals[3] vals[4]

::routine getSysMetrics public
  use strict arg index
  return .DlgUtil~getSystemMetrics(index)

::routine infoMessage public
   use strict arg info
   return messageDialog(info, , "Information", "OK", "INFORMATION", "SETFOREGROUND TASKMODAL")

::routine instMMFuncs public
  return 1

::routine playSoundFile public external "LIBRARY oodialog play_rtn"
::routine playSoundFileInLoop public external "LIBRARY oodialog play_rtn"
::routine sleepMS public external "LIBRARY oodialog msSleep_rtn"
::routine stopSoundFile public external "LIBRARY oodialog play_rtn"
::routine systemMetrics public
   use strict arg index
   return .DlgUtil~getSystemMetrics(index)

::routine yesNoMessage public
   use strict arg info, defaultButton = 'Y'
   miscStyles = "SETFOREGROUND TASKMODAL"
   if defaultButton~left(1)~translate == 'N' then miscStyles ||= ' DEFBUTTON2'
   return messageDialog(info, , "Question", "YESNO", "QUESTION", miscStyles) == 6

/* ----- End deprecated routines. --------------------------------------------*/

/*-------------------------- shortcut routines -------------------------------*/

::routine CheckList public
   use arg msg, title, labels, checks = (.array~new), lx = 0, max = 0
   num = labels~items
   do i=1 to num
      j = i+100
      lab.i = labels[i]
      if checks~hasIndex(i) then do
         if checks[i]~datatype('O') then dat.j = checks[i]
         else dat.j = 0
      end
      else dat.j = 0
   end
   dlg = .CheckList~new(msg, title, lab., dat., lx, max)
   if dlg~execute \= 1 then do; drop dlg; return .NIL; end
   ret = .array~new
   do j=101 to 100+num
      ret[j-100] = dat.j
   end
   drop dlg
   return ret

::routine InputBox public
   use arg msg, title, default = '', size = 0
   dlg = .InputBox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine IntegerBox public
   use arg msg, title, default = '', size = 0
   dlg = .IntegerBox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine ListChoice public
   use arg msg, title, list, lx = 0, ly = 0, predev = ""
   num = list~items
   do i=1 to num
      lst.i = list[i]
   end
   dlg = .ListChoice~new(msg, title, lst., lx, ly, predev)
   res = dlg~execute
   drop dlg
   if res = '' then return .nil
   else return res

::routine MultiInputBox public
   use arg msg, title, labels, datas, len = 0
   num = labels~items
   do i=1 to num
      j = i+100
      lab.i = labels[i]
      dat.j = datas[i]
   end
   dlg = .MultiInputBox~new(msg, title, lab., dat., len)
   if dlg~execute \= 1 then do; drop dlg; return .NIL; end
   ret = .array~new(num)
   do i=1 to num
      attr = labels[i]~changestr(' ','')~changestr('&','')
      Interpret 'ret[i] = dlg~'attr
   end
   drop dlg
   return ret

::routine MultiListChoice public
   use arg msg, title, list, lx = 0, ly = 0, predev = ""
   num = list~items
   do i=1 to num
      lst.i = list[i]
   end
   dlg = .MultiListChoice~new(msg, title, lst., lx, ly, predev)
   res = dlg~execute
   if res = '' then do; drop dlg; return .NIL; end
   ret = .array~new
   do i=1 to words(res)
      ret[i] = list[word(res,i)]
   end
   drop dlg
   return ret

::routine PasswordBox public
   use arg msg, title, default = "", size = 0
   dlg = .Passwordbox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine SingleSelection public
   use strict arg msg, title, labels, preSel = 1, rbWidth = 0, max = 0

   do i = 1 to labels~items
      lab.i = labels[i]
   end
   dlg = .SingleSelection~new(msg, title, lab., preSel, rbWidth, max)
   res = dlg~execute
   drop dlg
   return res

::routine TimedMessage public
   use arg msg, title, duration, earlyReply = .false, pos = .nil
   if \ duration~datatype('W') then return -1
   if \ earlyReply~datatype('O') then earlyReply = .false
   dlg = .TimedMessage~new(msg,title,duration, earlyReply, pos)
   dlg~execute
   if duration < 0 then return dlg
   if \ earlyReply then drop dlg
   return 0


/**
 * The DlgArea and DlgAreaU classes.
 */

/* ========================================================================= */
::class 'DlgArea' public
/* ========================================================================= */
/*  define an area of your dialog to place controls on                       */
/*                                                                           */
/*  parms x, y, width, height [,margin]                                      */
/*                                                                           */
/*  x & y take offset as an argument,                                        */
/*  a -ve argument is from the right/bottom                                  */
/*                                                                           */
/*  methods & attributes                                                     */
/*  x()        offset from left (right if -ve)                               */
/*  y()        offset from top  (bottom if -ve)                              */
/*  w()        width                                                         */
/*  h()        height                                                        */
/*  wr         width remaining                                               */
/*  hr         height remaining                                              */
/*  l          left    }                                                     */
/*  t          top     }  Dimensions                                         */
/*  b          bottom  }  within margins                                     */
/*  r          right   }                                                     */
/*  move()     Specify new x,y coordinates                                   */
/*  resize()   Specify new width and height                                  */
/*                                                                           */
/*                                                                           */
/*  methods                                                                  */
/*  x & y  can take an offset argument which may be absolute or a percentage */
/*  w & h  can take a percentage as argument                                 */
/*         a parm of r to w or h means the remainder of the width or height  */
/*                                                                           */
/*  so to add a button 80% across your margined dialog for the remainder     */
/*  a = .DlgArea~new(10,10,150,110,5)                                        */
/*  self~createPushButton(a~x(80%),10,a~w(20%),a~h(10%), ,'Text','Method')   */
/*    or not using percentages is equivalent to                              */
/*  self~createPushButton(a~x(112),10,a~w * 0.2,a~h(10), ,'Text','Method')   */
/*    or not using the object would be equivalent to                         */
/*  self~createPushButton(117, 10, 28, 10, , 'Text','Method')                */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/*               :                                  ^                        */
/*              top                                 :                        */
/*              <v.......width.............>     bottom                      */
/*              +--------------------------+   ^    :                        */
/* ......left..>|                       r  |   :    :                        */
/*              |                       v  |   :    :                        */
/*              |  +--------------------+  |   :    :                        */
/*              |  |+.x.>             ^ |  |   :    :                        */
/*              |  |:                 : |  |   :    :                        */
/*              |  |y                 : |  | height :                        */
/*              |  |:                 h |  |   :    :                        */
/*              |  |v                 : |  |   :    :                        */
/*              |  |<........w........+>|  |   :    :                        */
/*              |  |                  v |  |   :    :                        */
/*             ^|b>+--------------------+  |   :    :                        */
/*             m|                          |   :    :                        */
/*             v+--------------------------+   v    v                        */
/*              <m>                                                          */
/*.....right...............................>                                 */
/*                                                                           */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
::method init
/* ------------------------------------------------------------------------- */
arg left,top,width,height,margin

if margin~datatype('n')
then self~margin = margin~trunc
else self~margin = max(min(height % 7,5),1)

if top~datatype('n')
then self~top       = top~trunc
else self~top       = 10

if left~datatype('n')
then self~left      = left~trunc
else self~left      = 10

if width~datatype('n')
then self~width     = width~trunc
else self~width     = 100

if height~datatype('n')
then self~height    = height~trunc
else self~height    = 100


self~t = self~top  + self~margin
self~l = self~left + self~margin
self~b = self~top  + self~height - self~margin
self~r = self~left + self~width  - self~margin

self~lastX = 0
self~lastY = 0

/* ------------------------------------------------------------------------- */
::method x
/* ------------------------------------------------------------------------- */
arg offset
self~lastX='0'||Offset

if offset~pos('%') > 0
then do
   offset = offset~changestr('%','')
   if offset~datatype('n')
   then do
      if offset < 0 then offset = 100 + offset          /* from right margin */
      return trunc(self~left + self~margin + (self~w * (offset / 100)))
   end
   else return self~left + self~margin
end
else if offset~datatype('n')
     then if offset >= 0
          then return trunc(self~left + self~margin + offset) /* x from left */
          else return trunc(self~left + self~width - self~margin + offset) /* right */
else return self~left + self~margin

/* ------------------------------------------------------------------------- */
::method y
/* ------------------------------------------------------------------------- */
arg offset
self~lastY = '0'||offset

if offset~pos('%') > 0
then do
   offset = offset~changestr('%','')
   if offset~datatype('n')
   then do
      if offset < 0 then offset = 100 + offset          /* from right margin */
      return trunc(self~top + self~margin + (self~h * (offset / 100)))
   end
   else return self~top + self~margin
end
else if offset~datatype('n')
     then if offset >= 0
          then return trunc(self~top + self~margin + offset)
          else return trunc(self~top + self~height - self~margin + offset)
else return self~top + self~margin

/* ------------------------------------------------------------------------- */
::method h
/* ------------------------------------------------------------------------- */
arg pc

if pc = 'R' then return self~hr                       /* height is remainder */

h = self~height - (2 * self~margin)

if pc~pos('%') > 0
then return trunc(h * (pc~changestr('%','') / 100))
else return trunc(h)

/* ------------------------------------------------------------------------- */
::method HR
/* ------------------------------------------------------------------------- */

if self~lastY~pos('%') = 0                                 /* Y was absolute */
then return self~top + self~height - self~margin - self~lastY
else return self~h(100 - self~lastY~changestr('%','')||'%')       /* Y was % */

/* ------------------------------------------------------------------------- */
::method w
/* ------------------------------------------------------------------------- */
arg pc

if pc = 'R' then return self~wr                       /* width  is remainder */

w = self~width - (2 * self~margin)

if pc~pos('%') > 0
then return trunc(w * (pc~changestr('%','') / 100))
else return w

/* ------------------------------------------------------------------------- */
::method wr
/* ------------------------------------------------------------------------- */

if self~lastX~pos('%') = 0                                 /* X was absolute */
then return self~left + self~width - self~margin - self~lastX
else return self~w(100 - self~lastX~changestr('%','')||'%')       /* X was % */

/* ----------------------- compatibility methods --------------------------- */
::method cx    ; return self~w(arg(1))         -- Compatibility version of w
::method cy    ; return self~h(arg(1))         -- Compatibility version of h
::method sizeX ; return self~width
::method sizeY ; return self~height
/* ------------------------------------------------------------------------- */
::attribute t
::attribute l
::attribute b
::attribute r
::attribute top
::attribute left
::attribute width
::attribute height
::attribute margin
::attribute lastX           private
::attribute lastY           private

/* ========================================================================= */
::class 'DlgAreaU' subclass DlgArea public -- DlgAreaUniversal - Whole of Dialog
/* ========================================================================= */
/*  define the Area of the calling Dialog                                    */
/*                                                                           */
/*  parms callingDialog [,margin]                                            */
/*                                                                           */
/*  methods & attributes                                                     */
/*  resize     Aids in Dynamically Resizing Dialogs - See Documentation      */
/*  correctionFactor                                - See Documentation      */
/*                                                                           */
::method init
/* ------------------------------------------------------------------------- */
expose dlgObjList dlgObject
use arg dlgObj, margin, noResize, noMove

dlgObject = dlgObj

if \noResize~isA(.Set) then noResize = .Set~new
if \noMove~isA(.Set) then noMove = .Set~new
self~noResize = noResize
self~noMove   = noMove

self~resolveSetIDs

if \margin~datatype('n') then margin = 5

self~correctionFactor = 1.05

self~init:super(0,0,dlgObj~sizeX,dlgObj~sizeY,margin)

self~updateOnResize = .true
self~originalWidth  = dlgObj~sizeX
self~originalHeight = dlgObj~sizeY
self~lastError      = .nil

CreateControlTypes = .set~of('BITMAPBUTTON', 'CHECKBOX', 'COMBOBOX', 'TAB', -
  'PUSHBUTTON', 'RADIOBUTTON', 'DATETIMEPICKER', 'LISTVIEW', 'PASSWORDEDIT',    -
  'PROGRESSBAR', 'SCROLLBAR', 'TRACKBAR', 'BLACKFRAME', 'BLACKRECT', 'LISTBOX', -
  'ETCHEDHORIZONTAL', 'ETCHEDVERTICAL', 'GRAYRECT', 'GRAYFRAME', 'GROUPBOX',    -
  'STATIC', 'STATICFRAME', 'STATICIMAGE', 'STATICTEXT', 'TREEVIEW', 'UPDOWN',   -
  'WHITEFRAME', 'WHITERECT', 'MONTHCALENDAR', 'EDIT'                            -
)

-- Analyze the callers defineDialog Method
DDSourceArr=dlgObj~class~method('defineDialog')~source
DDSBuff=.mutableBuffer~new

-- Remove comments prefixed by --
do line over DDSourceArr
   parse var line line '--' .
   DDSBuff~Append(' '||line)
end
DDSource = DDSBuff~String ; Drop DDSBuff

-- remove comments within /*  */
do forever
   commEnd = DDsource~pos('*'||'/')
   if commEnd = 0 then leave
   commStart = DDSource~SubStr(1,CommEnd-1)~lastPos('/'||'*')
   if comStart=0
   then do
      self~lastError='Error in DlgAreaU Class: Unbalanced comments in defineDialog class of' Dlg
      leave
   end

   parse var DDSource front =(commStart) . =(commEnd) . +2 back
   DDSource = front back
end

-- copy of de-commented defineDialog source
DDSource1 = DDSource

-- re-create DlgArea objects within scope
do forever
   parse upper var DDSource1 front '.DLGAREA'DAMsg'('DAparms')'DDSource1
   do while DAParms~Countstr('(') > DAParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      DAParms = DAParms||')'||fragment
   end


   if DAParms = '' then leave    -- we have got them all!

   -- parse out the refferer variable name
   parse value front~reverse with . '='DAVar .
   DAVar = DAVar~reverse~strip
   DAObj = DAVar
   sourceline = DAVar'=.DLGAREA'||DAMsg||'('||DAParms||')'
   select
      when DAMsg~Space(0) = 'U~NEW'  -- This is the DlgAreaU (self) obj def
         then interpret DAVar'=.DlgArea~new(0,0,'||self~w||','||self~h||','||self~margin||')'
      when DAMsg~Space(0) = '~NEW'   -- This is a DlgArea Obj definition
         then interpret DAVar'=.DlgArea~new('daparms')'
      otherwise
         self~lastError='DlgAreaU Class: Unable to parse:' SourceLine
   end
end

-- Now we parse out resizable widgets.
dlgObjList=.List~new
delim = '00'x

signal on syntax name parseerror
Parseerroroccured = 1 /* provisionally */

-- Save another copy of DDSource, we'll need it later.
DDSourceCopy = DDSource

-- First look for the create<Control> methods.
do forever
   parse upper var DDSource front 'CREATE'ObjName'('ObjParms')'DDSource1
   do while ObjParms~Countstr('(') > ObjParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      ObjParms = ObjParms||')' fragment
   end

   if ObjParms = '' then leave
   if front~space(0)~right(5)='SELF~' then do
      sourcelinetxt = 'SELF~CREATE'||Objname||'('||Objparms||')'
      DDSource=DDSource1

      if CreateControlTypes~HasIndex(ObjName~strip) then
          parse var ObjParms ObjId','ObjX','ObjY','ObjW','ObjH','.
      else
          ObjId = ''       -- This createXXX method is ignored

      ObjId = ObjId~space(0)~strip( , "'")~strip( , '"')
      ObjId = dlgObject~resolveSymbolicID(ObjId)

      if ObjId > 0 then
         interpret 'dlgObjList~Insert(ObjId"@"' ObjX '"@"' ObjY '"@"' ObjW '"@"' ObjH ')'
   end
   else do
      -- skip past this create
      parse var DDSource 'CREATE'DDSource
   end
end

-- Now look for all the deprecated add<Control> methods.
DDSource = DDSourceCopy

do forever
   parse upper var DDSource front 'ADD'ObjName'('ObjParms')'DDSource1
   do while ObjParms~Countstr('(') > ObjParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      ObjParms = ObjParms||')' fragment
   end

   if ObjParms = '' then leave
   if front~space(0)~right(5)='SELF~' then do
      sourcelinetxt = 'SELF~ADD'||Objname||'('||Objparms||')'
      DDSource=DDSource1
      select
         when 'BITMAPBUTTON BUTTON PROGRESSBAR SCROLLBAR'~Wordpos(ObjName~strip) > 0 then
            parse var ObjParms ObjId','ObjX','ObjY','ObjW','ObjH','.
         when 'CHECKBOX COMBOBOX ENTRYLINE LISTBOX LISTCONTROL PASSWORDLINE RADIOBUTTON SLIDERCONTROL TABCONTROL'~Wordpos(ObjName~strip) > 0 then
            parse var ObjParms ObjId','.','ObjX','ObjY','ObjW','ObjH','.
         when 'BLACKFRAME BLACKRECT GRAYRECT GRAYFRAME TREECONTROL WHITEFRAME WHITERECT'~Wordpos(ObjName~strip) > 0 then
            parse var ObjParms ObjX','ObjY','ObjW','ObjH','.','ObjId','.
         when 'GROUPBOX TEXT'~Wordpos(ObjName~strip) > 0 then
            parse var ObjParms ObjX','ObjY','ObjW','ObjH','.','.','ObjId','.
         otherwise
            ObjId = ''     -- This addXXX method is ignored
      end

      ObjId = ObjId~space(0)~strip( , "'")~strip( , '"')
      ObjId = dlgObject~resolveSymbolicID(ObjId)

      if ObjId > 0 then
         interpret 'dlgObjList~Insert(ObjId"@"' ObjX '"@"' ObjY '"@"' ObjW '"@"' ObjH ')'
   end
   else do
     -- skip past this add
      parse var DDSource 'ADD'DDSource
   end
end

Parseerroroccured = 0

parseerror:
signal off syntax

if parseerroroccured then self~lastError = 'DlgAreaU class could not parse' sourcelinetxt


/* ------------------------------------------------------------------------- */
::method noResizePut
/* ------------------------------------------------------------------------- */
/* This method allows the user to place dialog controls in the noResize set  */
/* using symbolic IDs.                                                       */
/* ------------------------------------------------------------------------- */
expose dlgObject
   use strict arg id

   id = dlgObject~getResourceID(id)
   self~noResize~put(id)


/* ------------------------------------------------------------------------- */
::method noMovePut
/* ------------------------------------------------------------------------- */
/* This method allows the user to place dialog controls in the noMove set    */
/* using symbolic IDs.                                                       */
/* ------------------------------------------------------------------------- */
expose dlgObject
   use strict arg id

   id = dlgObject~getResourceID(id)
   self~noMove~put(id)


/* ------------------------------------------------------------------------- */
::method resolveSetIDs private
/* ------------------------------------------------------------------------- */
/* This method looks through the noMove and noResize sets and replaces any   */
/* symbolic resource IDs with the correct numeric ID.  If a symbolic ID can  */
/* not be resolved, it is removed from the set.                              */
/* ------------------------------------------------------------------------- */
expose dlgObject noMove noResize

   do id over noMove
     id = noMove~remove(id)
     id = dlgObject~getResourceID(id)
     noMove~put(id)
   end

   do id over noResize
     id = noResize~remove(id)
     id = dlgObject~getResourceID(id)
     noResize~put(id)
   end


/* ------------------------------------------------------------------------- */
::method resize
/* ------------------------------------------------------------------------- */
/* this method needs to be called as follows:                                */
/* in your dialog init method place this line after call to the superclass   */
/* self~connectResize('OnResize')                                            */
/* the method resize would contain the following                             */
/* expose u /* our topmost dlgAreaClass exposed in defineDialog */           */
/* use arg dummy, sizeinfo    /* sizeinfo contains information about the     */
/*                                                  new width and height */  */
/* u~resize(self,sizeinfo)                                                   */
/* ------------------------------------------------------------------------- */
expose dlgObjList
  use arg dlg,data
  dlg~sizeX = .DlgUtil~loWord(data) % dlg~factorX
  dlg~sizeY = .DlgUtil~hiWord(data) % dlg~factorY

  wFactor   = (dlg~sizeX / self~originalWidth ) * self~correctionFactor
  hFactor   = (dlg~sizeY / self~originalHeight) * self~correctionFactor

  do dlgObjDef over dlgObjList
     parse var dlgObjdef DOid'@'DOx'@'DOy'@'DOw'@'DOh
     if \self~noResize~hasIndex(DOid)
     then dlg~resizeItem(Doid,DOw * wFactor,DOh * hFactor,"NOREDRAW")
     if \self~noMove~hasIndex(DOid)
     then dlg~moveItem(DOid,DOx * wFactor,DOy * hFactor,"NOREDRAW")
  end /* DO */

  if self~updateOnResize then dlg~update
/* ------------------------------------------------------------------------- */

::attribute originalWidth     private
::attribute originalHeight    private
::attribute dlgObject         private get
::attribute noResize
::attribute noMove
::attribute correctionFactor
::attribute lastError
::attribute updateOnResize

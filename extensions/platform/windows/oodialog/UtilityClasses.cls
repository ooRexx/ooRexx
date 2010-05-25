/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
 * Includes the .DlgUtil class which must be defined as the the first class.
 * The class init() method of .DlgUtil controls the basic set up for the
 * ooDialog framework.
 *
 * Note:  This "class" file can not be used by itself, as for instance
 *        UserDialog.cls could.  It is intended to be merged into the main
 *        ooDialog framework file as the first file.  It contains classes that
 *        subclass other classes that will be present in the single framework
 *        file, but are not present in this file.
 */

::class 'DlgUtil' public
::method init class external "LIBRARY oodialog dlgutil_init_cls"
::method version class external "LIBRARY oodialog dlgutil_version_cls"
::method comctl32Version class external "LIBRARY oodialog dlgutil_comctl32Version_cls"
::method hiWord class external "LIBRARY oodialog dlgutil_hiWord_cls"
::method loWord class external "LIBRARY oodialog dlgutil_loWord_cls"
::method makeWParam class external "LIBRARY oodialog dlgutil_makeWPARAM_cls"
::method makeLParam class external "LIBRARY oodialog dlgutil_makeLPARAM_cls"
::method and class external "LIBRARY oodialog dlgutil_and_cls"
::method or class external "LIBRARY oodialog dlgutil_or_cls"
::method screenSize class external "LIBRARY oodialog dlgutil_screenSize_cls"
::method screenArea class external "LIBRARY oodialog dlgutil_screenArea_cls"
::method getSystemMetrics class external "LIBRARY oodialog dlgutil_getSystemMetrics_cls"
::method handleToPointer class external "LIBRARY oodialog dlgutil_handleToPointer_cls"
::method test class external "LIBRARY oodialog dlgutil_test_cls"


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


::class 'Rect' public
::method init external "LIBRARY oodialog rect_init"
::method left external "LIBRARY oodialog rect_left"
::method 'left=' external "LIBRARY oodialog rect_setLeft"
::method top external "LIBRARY oodialog rect_top"
::method 'top=' external "LIBRARY oodialog rect_setTop"
::method right external "LIBRARY oodialog rect_right"
::method 'right=' external "LIBRARY oodialog rect_setRight"
::method bottom external "LIBRARY oodialog rect_bottom"
::method 'bottom=' external "LIBRARY oodialog rect_setBottom"

::class 'Point' public
::method init external "LIBRARY oodialog point_init"
::method x external "LIBRARY oodialog point_x"
::method 'x=' external "LIBRARY oodialog point_setX"
::method y external "LIBRARY oodialog point_y"
::method 'y=' external "LIBRARY oodialog point_setY"

::class 'Size' public
::method init class external "LIBRARY oodialog size_init_cls"
::method init external "LIBRARY oodialog size_init"
::method width external "LIBRARY oodialog size_cx"
::method 'width=' external "LIBRARY oodialog size_setCX"
::method height external "LIBRARY oodialog point_y"
::method 'height=' external "LIBRARY oodialog point_setY"


::class 'DayState' public
::method init external "LIBRARY oodialog ds_init"
::method dayStateValue external "LIBRARY oodialog ds_dayStateValue"


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
    endMonth = .DateTime~fromStandardDate((year + count) || '0101')

    years = .table~new
    do i = year to year + count
        years[i] = self~generateEmptyYear
    end

::method getDayStateBuffer
    expose startMonth years
    use strict arg startDate, count

    year = startDate~year
    month = startDate~month

    resultArray = .array~new(count)

    aYear = years[year]
    if aYear == .nil then do
        do i = 1 to count
            resultArray[i] = .DayState~new
        end
        return .DayStates~makeDayStateBuffer(resultArray, count)
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

    return .DayStates~makeDayStateBuffer(resultArray, count)

::method addYear
    expose startMonth endMonth years
    use strict arg year, values

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
        endMonth = newYear
        if years[year - 1] == .nil then needFill = .true
    end

    if needFill then self~doFill

::method addMonth
    expose startMonth endMonth years
    use strict arg year, month, dayState

    yearArray = years[year]
    if yearArray == .nil then do
        yearArray = .array~new(12)
        yearArray[month] = dayState
        self~addYear(year, yearArray)
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



/**
 * Standard Dialog Control Classes.  Some simple short cut dialogs.
 */

-- Create a "default" dialog class and set it to the user dialog.
::class 'Dialog' subclass UserDialog public


-- This class shows a message window for a defined duration
::class 'TimedMessage' subclass UserDialog public

::method init
   expose message title sleeping earlyReply
   use arg message, title, sleeping, earlyReply = .false
   if \ earlyReply~datatype('O') then earlyReply = .false
   if \ sleeping~datatype('W') then sleeping = 0
   self~init:super("")

::method defineDialog
   expose message sx sy
   self~createStaticText(-1, 10, 10, sx+2, sy, , message)

::method initAutoDetection
   self~noAutoDetection

::method execute
   expose message title sleeping earlyReply sx sy

   s = self~getTextSizeDu(message)
   sx = s~width
   sy = s~height
   self~createCenter(sx+20, sy + 20, title,"NOMENU", , , , 1)
   self~startIt
   self~show("SHOWTOP")
   if sleeping >= 0 then do
      if earlyReply then reply
      ret = msSleep(sleeping)
      self~stopit
   end


-- This class provides a simple dialog with a title, a message, one entry line,
-- an OK and a Cancel push button.
::class 'InputBox' subclass UserDialog public

::method sx attribute
::method sy attribute

::method init
   expose message title datas. size
   use arg message, title, default, size
   if arg(3,"o") = 1 | default = "DEFAULT" then datas.101 = ""; else datas.101 = default
   if arg(4,"o") = 1 | size = "SIZE" then size = 0
   self~init:super(datas.)

::method defineDialog
   expose message size dlgy
   if size = 0 then size = self~sx
   self~createStaticText(-1, 10, 10, self~sx+2, self~sy, , message)
   self~AddLine(10, self~sy+4+10, size)
   self~createPushButtonGroup(self~sx-80, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method AddLine
   use arg x, y, l
   self~createEdit(101, x, y, l, , , "DATA")


::method execute
   expose message title datas. dlgy size

   s = self~getTextSizeDu(message)
   self~sx = max(s~width,100,size)
   self~sy = s~height

   dlgy = (self~sy+4)*2+36
   dlgx = self~sx + 20
   if dlgx < 110 then dlgx = 110

   self~createCenter(dlgx, dlgy, title,,,,,4)
   self~execute:super("SHOWTOP")
   if self~initCode = 1 then return datas.101
   else return ""


-- This class is an InputBox dialog which entry line have the not visible flag.
::class 'PasswordBox' subclass InputBox public

::method AddLine
   use arg x, y, l
   self~createPasswordEdit(101, x, y, l, , , "DATA")


-- This class is an InputBox dialog which entry line allows only numerical data.
::class 'IntegerBox' subclass InputBox public

::method validate
   self~GetDataStem(datas.)
   if datas.101~DataType('W') = 0 then
   do
      call infoDialog "You did not enter numerical data"
      return 0
   end
   else return 1


::class 'MultiInputBox' subclass UserDialog public

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

::attribute sx
::attribute sy
::attribute dlgx
::attribute dlgy
::attribute lenx
::attribute leny
::attribute message
::attribute useVScroll

::method init
   expose message title datas. input. lenx leny
   use arg message, title, input., lenx, leny, predev
   if arg(4,"o") = 1 then lenx = 0
   if arg(5,"o") = 1 then leny = 0
   if arg(6,"o") = 0 then datas.101 = predev; else datas.101 = ""
   self~init:super(datas.)
   s = self~getTextSizeDu(message)
   self~sx = s~width
   self~sy = s~height
   self~message = message
   self~useVScroll = .false

::method defineDialog
   expose message sx sy lenx leny dlgx dlgy
   self~createStaticText(-1, 10, 10, sx+2, sy, , message)
   if self~useVScroll then style = "VSCROLL"
   else style = "PARTIAL"
   self~createListBox(101, 10, sy+4+10, lenx, leny+4, style, "DATA")
   self~createPushButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method initDialog
   expose count input.
   do i = 1 to count
      self~AddListEntry(101, input.i)
   end
   self~setListBoxData(101, "")

::method execute
   expose message title sx sy datas. input. count lenx leny dlgx dlgy useVScroll

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

   if self~initCode = 1 then return datas.101
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


::class 'MultiListChoice' subclass ListChoice public

::method defineDialog
   if self~useVScroll then style = "MULTI VSCROLL TABULATORS"
   else style = "MULTI PARTIAL TABULATORS"
   self~createStaticText(-1, 10, 10, self~sx+2, self~sy, , self~message)
   self~createListBox(101, 10, self~sy+4+10, self~lenx, self~leny+4, style, "DATA")
   self~createPushButtonGroup(self~dlgx-100, self~dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")


::class 'CheckList' subclass UserDialog public

  /* Entries is a stem variable with all the titles for the check boxes */
  /* Datas is a stem variable you can use to preselect the check boxes */
  /* 'Datas.i=1' means there will be a check mark on the i-th box */
::method Init
   expose message title datas. entries. len max
   use arg message, title, entries., datas., len, max
   if arg(5,"o") = 1 then len = 0
   if arg(6,"o") = 1 then max = 0
   self~init:super(datas.)

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

::method Init
   expose message title entries. len datas. prev max
   use arg message, title, entries., prev, len, max
   if arg(5,"o") = 1 then len = 0
   if arg(6,"o") = 1 then max = 0
   if arg(4,"o") = 1 then prev = 1
   self~init:super(datas.)

::method defineDialog
   expose message sx sy len dlgx dlgy entries. placex max
   self~createStaticText(-1, 10, 10, sx+2, sy, , message)
   self~createRadioButtonStem(101, placex, sy+4+10, len, entries., max)
   self~createPushButtonGroup(dlgx-100, dlgy - 18, 0, 0, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method execute
   expose message title sx sy datas. dlgx dlgy len entries. placex prev max

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

   do i = 101 to 100+count
      datas.i = 0
   end
   pr = (100+prev)
   datas.pr = 1

   self~createCenter(dlgx, dlgy, title,,,,, count+5)
   self~execute:super("SHOWTOP")
   if self~initCode = 1 then
   do
     do i = 101 to 100+count
      if datas.i = 1 then return i-100
     end
     return 0
   end
   else
    return ""


::class 'StateIndicator' subclass UserDialog public

::method Xext attribute
::method TitleTxt attribute
::method Msg attribute
::method Dims attribute

::method init
   use arg message, title, fixedx
   if arg(3,'o') = 1 then self~XExt = 0; else self~XExt = fixedx
   self~Msg = message
   self~TitleTxt = title
   self~dims = .directory~new
   self~init:super("")
   if self~initCode = 0 then self~Popup

::method defineDialog
   self~createStaticText(100, 10, 10, self~dims["sx"]+2, self~dims["sy"], , self~msg)

::method initAutoDetection
   self~noAutoDetection

::method CalcSize
   size = self~getTextSizeDu(self~msg)
   scrSize = screenSize()
   self~dims["sx"] = size~width
   self~dims["sy"] = size~height
   if self~XExt = 0 then do
       self~dims["cx"] = self~dims["sx"]+20
       self~dims["cy"]= self~dims["sy"]*2.2+20
   end
   else do
       self~dims["cx"] = self~XExt
       lines = 1
       if self~dims["sx"] + 20 > self~dims["cx"] then do
           lines = Trunc(self~dims["sx"] / (self~dims["cx"] -20) + 0.98)
           self~dims["sx"] = self~dims["cx"] - 20
           self~dims["sy"] = Trunc(self~dims["sy"]*lines*1.1)
       end
       if lines >1 then self~dims["cy"] = self~dims["sy"]+20
       else self~dims["cy"] = self~dims["sy"]*2.2+20
   end
   self~dims["x"] = trunc((scrSize[1]-self~dims["cx"]) /2)
   self~dims["y"] = trunc((scrSize[2] - self~dims["cy"] - 20) /2)

::method Popup
   self~CalcSize
   self~create(self~dims["x"],self~dims["y"], self~dims["cx"], self~dims["cy"], self~TitleTxt,"NOMENU",,,1 )
   self~startIt
   self~show("SHOWTOP")

::method Destroy
   self~stopit

::method "Message="
   parse arg message
   self~msg = message
   self~CalcSize
   self~ResizeItem(100, self~dims["sx"]+2, self~dims["sy"])
   if self~XExt = 0 then do
       self~Resize(self~dims["cx"], self~dims["cy"], "HIDE")
   end
   else do
       parse value self~GetWindowRect(self~dlgHandle) with l t r b
       if self~dims["cy"] > (b-t) / self~factorY then self~Resize(self~dims["cx"], self~dims["cy"], "NORMAL")
   end
   self~setStaticData(100, message)
   if self~XExt = 0 then self~Move(self~dims["x"], self~dims["y"], "NORMAL")


::class 'ProgressIndicator' subclass StateIndicator public

::method lowValue attribute
::method highValue attribute
::method incStep attribute

::method init
   use arg message, title, low, high, step, fixedx
   if arg(6,'o') = 1 then fixedx = 0
   if arg(3,'o') = 1 then self~lowValue = 0; else self~lowValue = low
   if arg(4,'o') = 1 then self~highValue = 100; else self~highValue = high
   if arg(5,'o') = 1 then self~incStep = 10; else self~incStep = step
   self~init:super(message, title, fixedx)

::method initDialog
   pb = self~newProgressBar(101)
   if pb \= .nil then do
       pb~setRange(self~lowValue, self~highValue)
       pb~setPos(self~lowValue)
       pb~setStep(self~incStep)
   end

::method defineDialog
   self~createProgressBar(101, 5, 5, self~sizex-10, 14)
   self~createStaticText(100, 10, 25, self~dims["sx"]+2,self~dims["sy"], , self~msg)


::method calcSize
   self~calcSize:super
   self~dims["cy"] = self~dims["cy"]+10  /* 15 more for the progress bar */
   scrSize = screenSize()
   self~dims["x"] = trunc((scrSize[1]-self~dims["cx"]) /2)
   self~dims["y"] = trunc((scrSize[2] - self~dims["cy"] - 20) /2)

::method "progress="
   use arg newpos
   self~newProgressBar(101)~setPos(newpos)

::method increase
   if arg(1,'e') = 1 then self~newProgressBar(101)~step(arg(1))
   else self~newProgressBar(101)~step

::method complete
   self~newProgressBar(101)~setPos(self~highValue)

::method "message="
   forward class (super) continue
   self~resizeItem(101,  self~sizex-10, 14)


/* - - - - - - - - - - - - - Public routines - - - - - - - - - - - - - - - - -*/

::routine fileNameDialog public external "LIBRARY oodialog fileNameDlg_rtn"
::routine msSleep public external "LIBRARY oodialog msSleep_rtn"
::routine findWindow public external "LIBRARY oodialog findWindow_rtn"
::routine play public external "LIBRARY oodialog playSound_rtn"

::routine messageDialog public external "LIBRARY oodialog messageDialog_rtn"
::routine infoDialog public
   use strict arg info
   return messageDialog(info, , "Information", "OK", "INFORMATION", "SETFOREGROUND TASKMODAL")

::routine errorDialog public
   use strict arg info
   return messageDialog(info, , "Error", "OK", "HAND", "SETFOREGROUND TASKMODAL")

::routine askDialog public
   use strict arg info, defaultButton = 'Y'
   miscStyles = "SETFOREGROUND TASKMODAL"
   if defaultButton~left(1)~translate == 'N' then miscStyles ||= ' DEFBUTTON2'
   return (messageDialog(info, , "Question", "YESNO", "QUESTION", miscStyles) == 6)

::routine screenSize public
   return .DlgUtil~screenSize

::routine systemMetrics public
   use strict arg index
   return .DlgUtil~getSystemMetrics(index)

::routine routineTest public external "LIBRARY oodialog routineTest_rtn"


/** The following routines are, mostly, duplicates of the public routines above.
 *  They are needed to maintain backward compatibilty with  pre-4.0.0, because
 *  both names were documented.
 */
::routine sleepMS public external "LIBRARY oodialog msSleep_rtn"
::routine winTimer public external "LIBRARY oodialog winTimer_rtn"
::routine getFileNameWindow public external "LIBRARY oodialog fileNameDlg_rtn"
::routine playSoundFile public external "LIBRARY oodialog playSound_rtn"
::routine playSoundFileInLoop public external "LIBRARY oodialog playSound_rtn"
::routine stopSoundFile public external "LIBRARY oodialog playSound_rtn"

::routine infoMessage public
   use strict arg info
   return messageDialog(info, , "Information", "OK", "INFORMATION", "SETFOREGROUND TASKMODAL")

::routine errorMessage public
   use strict arg info
   return messageDialog(info, , "Error", "OK", "HAND", "SETFOREGROUND TASKMODAL")

::routine yesNoMessage public
   use strict arg info, defaultButton = 'Y'
   miscStyles = "SETFOREGROUND TASKMODAL"
   if defaultButton~left(1)~translate == 'N' then miscStyles ||= ' DEFBUTTON2'
   return messageDialog(info, , "Question", "YESNO", "QUESTION", miscStyles) == 6

::routine getSysMetrics public
  use strict arg index
  return .DlgUtil~getSystemMetrics(index)

::routine getScreenSize public
  vals = .DlgUtil~screenSize
  return vals[1] vals[2] vals[3] vals[4]

::routine instMMFuncs public
  return 1

::routine binaryAnd public
  use strict arg n1, n2
  return .DlgUtil~and(n1, n2)

/* ----- End compatibility routines. -----------------------------------------*/

/*-------------------------- shortcut routines -------------------------------*/

::routine TimedMessage public
   use arg msg, title, duration, earlyReply = .false
   if \ duration~datatype('W') then return -1
   if \ earlyReply~datatype('O') then earlyReply = .false
   dlg = .TimedMessage~new(msg,title,duration, earlyReply)
   dlg~execute
   if duration < 0 then return dlg
   if \ earlyReply then drop dlg
   return 0

::routine InputBox public
   use arg msg, title, default, size
   dlg = .InputBox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine IntegerBox public
   use arg msg, title, default, size
   dlg = .IntegerBox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine PasswordBox public
   use arg msg, title, default, size
   dlg = .Passwordbox~new(msg,title,default,size)
   ret = dlg~execute
   drop dlg
   return ret

::routine MultiInputBox public
   use arg msg, title, labels, datas, len
   if arg(5,"o") = 1 then len = 0
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

::routine ListChoice public
   use arg msg, title, list, lx, ly, predev
   if arg(4,"o") = 1 then lx = 0
   if arg(5,"o") = 1 then ly = 0
   if arg(6,"o") = 1 then predev = ""
   num = list~items
   do i=1 to num
      lst.i = list[i]
   end
   dlg = .ListChoice~new(msg, title, lst., lx, ly, predev)
   res = dlg~execute
   drop dlg
   return res

::routine MultiListChoice public
   use arg msg, title, list, lx, ly, predev
   if arg(4,"o") = 1 then lx = 0
   if arg(5,"o") = 1 then ly = 0
   if arg(6,"o") = 1 then predev = ""
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

::routine CheckList public
   use arg msg, title, labels, checks, lx = 0, max = 0
   if arg(4,"o") then checks = .array~new
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

::routine SingleSelection public
   use arg msg, title, labels, data, lx, max
   if arg(5,"o") = 1 then lx = 0
   if arg(6,"o") = 1 then max= 0
   num = labels~items
   do i=1 to num
      lab.i = labels[i]
   end
   dlg = .SingleSelection~new(msg, title, lab., data, lx, max)
   res = dlg~execute
   drop dlg
   return res


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
/*         a parm of R to W or H means the remainder of the width or height  */
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
::method Init
/* ------------------------------------------------------------------------- */
arg left,top,width,height,margin

if margin~datatype('n')
then self~Margin = margin~trunc
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


self~T = self~Top  + self~Margin
self~L = self~left + self~Margin
self~B = self~top  + self~height - self~margin
self~R = self~Left + self~Width  - self~Margin

self~LastX = 0
self~LastY = 0

/* ------------------------------------------------------------------------- */
::method X
/* ------------------------------------------------------------------------- */
arg offset
self~LastX='0'||Offset

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
::method Y
/* ------------------------------------------------------------------------- */
arg offset
self~LastY = '0'||offset

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
::method H
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

if self~LastY~pos('%') = 0                                 /* Y was absolute */
then return self~Top + self~Height - self~Margin - self~LastY
else return self~h(100 - self~LastY~changestr('%','')||'%')       /* Y was % */

/* ------------------------------------------------------------------------- */
::method W
/* ------------------------------------------------------------------------- */
arg pc

if pc = 'R' then return self~wr                       /* width  is remainder */

w = self~width - (2 * self~margin)

if pc~pos('%') > 0
then return trunc(w * (pc~changestr('%','') / 100))
else return w

/* ------------------------------------------------------------------------- */
::method WR
/* ------------------------------------------------------------------------- */

if self~LastX~pos('%') = 0                                 /* X was absolute */
then return self~Left + self~Width - self~Margin - self~LastX
else return self~w(100 - self~LastX~changestr('%','')||'%')       /* X was % */

/* ----------------------- compatibility methods --------------------------- */
::method cx    ; return self~w(arg(1))         -- Compatibility version of w
::method cy    ; return self~h(arg(1))         -- Compatibility version of h
::method sizeX ; return self~width
::method sizeY ; return self~height
/* ------------------------------------------------------------------------- */
::method T              Attribute
::method L              Attribute
::method B              Attribute
::method R              Attribute
::method Top            Attribute
::method Left           Attribute
::method Width          Attribute
::method Height         Attribute
::method Margin         Attribute
::method LastX          Attribute Private
::method LastY          Attribute Private

/* ========================================================================= */
::Class 'DlgAreaU' SubClass DlgArea Public -- DlgAreaUniversal - Whole of Dialog
/* ========================================================================= */
/*  define the Area of the calling Dialog                                    */
/*                                                                           */
/*  parms callingDialog [,margin]                                            */
/*                                                                           */
/*  methods & attributes                                                     */
/*  resize     Aids in Dynamically Resizing Dialogs - See Documentation      */
/*  CorrectionFactor                                - See Documentation      */
/*                                                                           */
::method Init
/* ------------------------------------------------------------------------- */
Expose DlgObjList

use arg DlgObj,Margin,NoResize,NoMove

if NoResize~String \= 'a Set' then NoResize = .Set~new
self~NoResize = NoResize
if NoMove~String   \= 'a Set' then NoMove   = .Set~new
self~NoMove   = NoMove

if \margin~datatype('n') then margin = 5

self~CorrectionFactor = 1.05

self~init:super(0,0,DlgObj~sizeX,DlgObj~sizeY,margin)

self~OriginalWidth  = DlgObj~sizeX
self~OriginalHeight = DlgObj~sizeY
self~LastError      = .nil

                                  /* Analyze the callers defineDialog Method */
                                       /* Remove comments prefixed by --     */
DDSourceArr=DlgObj~class~method('defineDialog')~source
DDSBuff=.mutableBuffer~new
do line over DDSourceArr
   parse var line line '--' .       -- strip out this style inline comments
   DDSBuff~Append(' '||line)
end /* DO */
DDSource = DDSBuff~String ; Drop DDSBuff
                                       /* remove comments within /*  */      */
do forever
   commEnd = DDsource~pos('*'||'/')
   if commEnd = 0 then leave
   commStart = DDSource~SubStr(1,CommEnd-1)~lastPos('/'||'*')
   if comStart=0
   then do
      self~LastError='Error in DlgAreaU Class: Unbalanced comments in defineDialog class of' Dlg
      leave
   end /* DO */

   parse var DDSource front =(commStart) . =(commEnd) . +2 back
   DDSource = front back
end /* DO */

DDSource1 = DDSource           /* copy of de-commented defineDialog source   */

                               /* re-create DlgArea objects within scope     */
do forever
   parse upper var DDSource1 front '.DLGAREA'DAMsg'('DAparms')'DDSource1
   do while DAParms~Countstr('(') > DAParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      DAParms = DAParms||')'||fragment
   end /* DO */
   if DAParms = '' then leave          /* we have got them all!              */

                                     /* parse out the refferer variable name */
   parse value front~reverse with . '='DAVar .
   DAVar = DAVar~reverse~strip
   DAObj = DAVar
   sourceline = DAVar'=.DLGAREA'||DAMsg||'('||DAParms||')'
   select
      when DAMsg~Space(0) = 'U~NEW'  /* This is the DlgAreaU (self) obj def  */
         then interpret DAVar'=.DlgArea~new(0,0,'||self~w||','||self~h||','||self~margin||')'
      when DAMsg~Space(0) = '~NEW'   /* This is a DlgArea Obj definition     */
         then interpret DAVar'=.DlgArea~new('daparms')'
      otherwise
         self~LastError='DlgAreaU Class: Unable to parse:' SourceLine
   end /* select */
end

                                    /* Now we parse out resizable widgets    */
DlgObjList=.List~new
delim = '00'x

signal on syntax name parseerror
Parseerroroccured = 1 /* provisionally */

do forever
   parse upper var DDSource front 'ADD'ObjName'('ObjParms')'DDSource1
   do while ObjParms~Countstr('(') > ObjParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      ObjParms = ObjParms||')' fragment
   end /* DO */

   if ObjParms = '' then leave
   if front~space(0)~right(5)='SELF~'
   then do
      sourcelinetxt = 'SELF~ADD'||Objname||'('||Objparms||')'
      DDSource=DDSource1
      select
         when 'BITMAPBUTTON BUTTON PROGRESSBAR SCROLLBAR'~Wordpos(ObjName~strip) >0
            then parse var ObjParms ObjId','ObjX','ObjY','ObjW','ObjH','.
         when 'CHECKBOX COMBOBOX ENTRYLINE LISTBOX LISTCONTROL PASSWORDLINE RADIOBUTTON SLIDERCONTROL TABCONTROL'~Wordpos(ObjName~strip) > 0
            then parse var ObjParms ObjId','.','ObjX','ObjY','ObjW','ObjH','.
         when 'BLACKFRAME BLACKRECT GRAYRECT GRAYFRAME TREECONTROL WHITEFRAME WHITERECT'~Wordpos(ObjName~strip) > 0
            then parse var ObjParms ObjX','ObjY','ObjW','ObjH','.','ObjId','.
         when 'GROUPBOX TEXT'~Wordpos(ObjName~strip) > 0
            then parse var ObjParms ObjX','ObjY','ObjW','ObjH','.','.','ObjId','.
         otherwise
            ObjId = ''                    /* This type is on the ignore list */
      end /* select */
      if ObjId \= ''
      then interpret 'DlgObjList~Insert(ObjId "@"' ObjX '"@"' ObjY '"@"' ObjW '"@"' ObjH ')'
   end /* DO */
   else parse var DDSource 'ADD'DDSource            /* skip past this add */
end /* DO */

Parseerroroccured = 0
parseerror:
signal off syntax

if parseerroroccured then self~lastError = 'DlgAreaU class could not parse' sourcelinetxt

/* ------------------------------- ignore list -----------------------------
ATTRIBUTE AUTOSTARTMETHOD BUTTONGROUP CATEGORYCOMBOENTRY CATEGORYLISTENTRY
CHECKBOXSTEM CHECKGROUP COMBOENTRY COMBOINPUT FULLSEQ INPUT INPUTGROUP
INPUTSTEM LISTENTRY MENUITEM MENUSEPERATOR OKCANCELLEFTBOTTOM OKCANCELLEFTTOP
OKCANCELRIGHTBOTTOM OKCANCELREGHTTOP POPUPMENU RADIOGROUP RADIOSTEM ROW
SEQUENCE STYLE USERMSG
   ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
::method Resize
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
expose DlgObjList
  use arg dlg,data
  dlg~sizeX = .DlgUtil~loWord(data) % dlg~factorX
  dlg~sizeY = .DlgUtil~hiWord(data) % dlg~factorY

  wFactor   = (dlg~sizeX / self~OriginalWidth ) * self~CorrectionFactor
  hFactor   = (dlg~sizeY / self~OriginalHeight) * self~CorrectionFactor


  do dlgObjDef over DlgObjList
     parse var dlgObjdef DOid'@'DOx'@'DOy'@'DOw'@'DOh
     if \self~NoResize~HasIndex(DOid~strip)
     then dlg~ResizeItem(Doid,DOw * wFactor,DOh * hFactor,"NOREDRAW")
     if \self~NoMove~HasIndex(DOid)
     then dlg~MoveItem(DOid,DOx * wFactor,DOy * hFactor,"NOREDRAW")
  end /* DO */
  dlg~update
/* ------------------------------------------------------------------------- */

::method OriginalWidth    Attribute Private
::method OriginalHeight   Attribute Private
::method NoResize         Attribute
::method NoMove           Attribute
::method CorrectionFactor Attribute
::method LastError        Attribute

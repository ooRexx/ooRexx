/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
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
 * A more advanced example of the date and time picker control.
 *
 * This is example is intended to show how to use call back fields in the format
 * string for the DTP control and how to respond to the FORMAT, FORMATQUERY, and
 * KEYDOWN notifications for the call back fields.
 *
 * What is not explicit in the comments in this program is that there must be
 * a custom format string, containing call back fields, set in the DTP control
 * in order for the 3 event notifications to occur.
 *
 * In the program, change the date and time in the DTP control to see how the
 * call back fields work.  Select a call back field by clicking on it or tabbing
 * to it and then use the up or down arrow keys, home, end, page up, or page
 * down keys and observe what happens.
 */

  .application~setDefaults('O', 'fiscalReports.h', .false)

  dlg = .FiscalReports~new('fiscalReports.rc', IDD_FISCAL_REPORTS)
  dlg~execute("SHOWTOP")

return 0

::requires "ooDialog.cls"

::class 'FiscalReports' subclass RcDialog

::method init
  expose haveSizes

  forward class (super) continue

  self~setUpArrays
  haveSizes = .false

  self~connectDateTimePickerEvent(IDC_DTP_REPORT, "FORMAT", onFormat)
  self~connectDateTimePickerEvent(IDC_DTP_REPORT, "FORMATQUERY", onFormatQuery)
  self~connectDateTimePickerEvent(IDC_DTP_REPORT, "KEYDOWN", onKeyDown)

  do i = .constDir[IDC_RB_PRE_FIRST] + 1 to .constDir[IDC_RB_ANTE_LAST] - 1
    self~connectButtonEvent(i, 'CLICKED', onRbClick, .true)
  end


::method initDialog
  expose dtp currentType

  dtp = self~newDateTimePicker(IDC_DTP_REPORT);
  self~newRadioButton(IDC_RB_BAL_SHEET)~check

  -- This is our format string with 3 call back fields in it.  Each call back
  -- field is designated by using a capital X. The number of X's allow you to
  -- uniquely indentify different call back fields.
  --
  -- As you see the format string starts with 3 call back fields.  The first is
  -- the number field, (First, Second, etc.,) the second is the period field,
  -- (quarter, half, etc.,) and the third is the report type, (balance, cash
  -- flow, etc..)  Within the format string, constant strings like:
  --   report starts from:
  -- need to be put in single quotes.  Punctuation and spaces do not need to be
  -- quoted, so for example ', ' does not need the single quotes.
  dtp~setFormat("XX XXX XXXX 'report starts from: 'hh':'mm tt dddd MMMM dd', 'yyyy")


/** onFormatQuery()
 *
 * This is the format query notification event handler. The DTP control sends
 * the notification to request the size of the largest item to be displayed in
 * the specified call back field
 *
 * @param  field  The identifier of the call back field.  In our case it will be
 *                XX, or XXX, or XXXX.
 *
 * @param  size   A .Size object.  The size of the largest item to display is
 *                returned to the DTP by setting this .Size object.
 *
 * The id and hwnd arguments are not needed here.
 *
 * Since the largest size for each field is not going to change, we just
 * calculate the size for each field 1 time and save a reference to it.  To
 * accurately calculate the size in pixels, we need to wait until the underlying
 * dialog is actually created.  We could have done this in initDialog(), but
 * I think it is better to wait until the DTP asks for the size.
 */
::method onFormatQuery unguarded
  expose dtp haveSizes xxSize xxxSize xxxxSize
  use arg field, size, id, hwnd

  if \ haveSizes then do
    xxSize  = self~calcSize('XX')
    xxxSize = self~calcSize('XXX')
    xxxxSize = self~calcSize('XXXX')
    haveSizes = .true
  end

  -- The equateTo() method sets the cx and cy attributes of the receiver .Size
  -- object to the cx and cy attributes of the argument .Size object.
  select
    when field == 'XX' then size~equateTo(xxSize)
    when field == 'XXX' then size~equateTo(xxxSize)
    otherwise size~equateTo(xxxxSize)
  end
  -- End select

  return 0

/** onFormat()
 *
 * This is the format notification event handler.  The DTP control sends this
 * notification to request the text to display in the call back field at this
 * point in time.  For this application, we base what should be displayed in the
 * numbers field on the currently selected date and time.  What the other 2
 * fields should display is tracked through the currentType and currentPeriod
 * variables.
 *
 * @param  field  The identifier of the call back field.  In our case it will be
 *                XX, or XXX, or XXXX.
 *
 * @param  dt     A .DateTime object.  This is the currently selected date and
 *                time in the DTP control.
 *
 * The id and hwnd arguments are not needed here.
 *
 * The return from this method is what the DTP control will dislpay in the
 * specified call back field.
 */
::method onFormat unguarded
  expose periods types currentType currentPeriod
  use arg field, dt, id, hwnd

  select
    when field == 'XX' then do
      ret = self~getPeriodNumber(dt)
    end

    when field == 'XXX' then do
      ret = periods[currentPeriod]
    end

    otherwise do
      ret = types[currentType]
    end
  end
  -- End select

  return ret

/** onKeyDown()
 *
 * This is the event handler for the KEYDOWN notification event handler.  The
 * DTP control sends this notification when the user types a key in a call back
 * field.  This allows the programmer to implement some custom behavior for the
 * call back fields.
 *
 * @param  field  The identifier of the call back field.  In our case it will be
 *                XX, or XXX, or XXXX.
 *
 * @param  dt     A .DateTime object.  This is the currently selected date and
 *                time in the DTP control.
 *
 * @param vKey    This is the virtual key the user pressed.
 *
 * The id and hwnd arguments are not needed here.
 *
 * The return is the modified date and time information based on the user's
 * keystroke.
 *
 * Each time the date and time in the DTP control is modified, the DTP control
 * sends the FORMAT notification.  This is what drives our program.
 */
::method onKeyDown
  expose periods numbers currentType currentPeriod
  use arg field, dt, vKey, idFrom, hwndFrom

  select
    when field == 'XX' then do
      newDT = self~updatePeriodNumber(dt, vKey)
    end

    when field == 'XXX' then do
      newDT = self~updatePeriod(dt, vKey)
    end

    otherwise do
      newDT = self~updateReport(dt, vKey)
    end
  end
  -- End select

  return newDT

::method onRbClick unguarded
  expose dtp currentType
  use arg info, hwnd

  id = .DlgUtil~loWord(info)
  dt = self~getFirstDayOfPeriod

  select
    when id == .constDir[IDC_RB_BAL_SHEET] then do
      currentType = 1
      dtp~setDateTime(dt)
    end

    when id == .constDir[IDC_RB_PAYROLL] then do
      currentType = 2
      dtp~setDateTime(dt)
    end

    when id == .constDir[IDC_RB_CASH_FLOW] then do
      currentType = 3
      dtp~setDateTime(dt)
    end

    when id == .constDir[IDC_RB_INCOME_STMT] then do
      currentType = 4
      dtp~setDateTime(dt)
    end

    when id == .constDir[IDC_RB_EARNINGS] then do
      currentType = 5
      dtp~setDateTime(dt)
    end

    otherwise do
      -- Should be impossible
      nop
    end
  end
  -- End select

  return 0


/** updatePeriodNumber()
 *
 * This method is invoked during the event handler for the key down event when
 * the call back field is the number, the period number (First, second, third,
 * etc..)  The user can use the up / down arrows, home, end, page up, and page
 * down keys to cycle through the periods.  All other keys are simply ignored.
 *
 * Note that when the number changes, the period (month, quarter, etc.,) stays
 * the same. The date changes to the same relative date in the new period.  The
 * numbers wrap, i.e., if it is the 12 month and the up arrow key is used, the
 * number wraps to 1.  To determine when to wrap, we have to check the period.
 * If the period is half, then we need to wrap at 3, etc..
 */
::method updatePeriodNumber unguarded private
  expose periods currentPeriod
  use strict arg dt, vkey

  mn = dt~month

  parse value dt~isoDate with pre 6 j 8 tail

  select
    when vKey == .VK~UP then do

      select
        when periods[currentPeriod] == 'month' then do
          mn += 1
          if mn == 13 then mn = 1
        end

        when periods[currentPeriod] == 'quarter' then do
          if mn <= 9 then mn += 3
          else mn -= 9
        end

        otherwise do
          if mn <= 6 then mn += 6
          else mn -= 6
        end
      end
      -- End select

      mn = mn~right(2, 0)
      newDT = .DateTime~fromIsoDate(pre || mn || tail)
    end

    when vKey == .VK~Down then do

      select
        when periods[currentPeriod] == 'month' then do
          mn -= 1
          if mn == 0 then mn = 12
        end

        when periods[currentPeriod] == 'quarter' then do
          if mn <= 3 then mn += 9
          else mn -= 3
        end

        otherwise do
          if mn <= 6 then mn += 6
          else mn -= 6
        end
      end
      -- End select

      mn = mn~right(2, 0)
      newDT = .DateTime~fromIsoDate(pre || mn || tail)
    end

    when vKey == .VK~HOME then do

      select
        when periods[currentPeriod] == 'month' then do
          mn = 1
        end

        when periods[currentPeriod] == 'quarter' then do
          if mn <= 3 then nop
          else if mn <= 6 then mn -= 3
          else if mn <= 9 then mn -= 6
          else mn -= 9
        end

        otherwise do
          if mn > 6 then mn -= 6
        end
      end
      -- End select

      mn = mn~right(2, 0)
      newDT = .DateTime~fromIsoDate(pre || mn || tail)
    end

    when vKey == .VK~END then do

      select
        when periods[currentPeriod] == 'month' then do
          mn = 12
        end

        when periods[currentPeriod] == 'quarter' then do
          if mn <= 3 then mn += 9
          else if mn <= 6 then mn += 6
          else if mn <= 9 then mn += 6
          else nop
        end

        otherwise do
          if mn <= 6 then mn += 6
        end
      end
      -- End select

      mn = mn~right(2, 0)
      newDT = .DateTime~fromIsoDate(pre || mn || tail)
    end

    when vKey == .VK~PRIOR then do
      -- We want to go half the year back, so it doesn't make any difference
      -- what the period is.
      if mn <= 6 then mn = 1
      else mn -= 6

      mn = mn~right(2, 0)
      newDT = .DateTime~fromIsoDate(pre || mn || tail)
    end

    when vKey == .VK~NEXT then do
      -- We want to go half the year ahead, so it doesn't make any difference
      -- what the period is.
      if mn <= 6 then mn += 6
      else mn = 12

      mn = mn~right(2, 0)
      newDT = .DateTime~fromIsoDate(pre || mn || tail)
    end

    otherwise do
      newDT = dt
    end
  end
  -- End select

  return newDT


/** updatePeriod()
 *
 * This method is invoked during the event handler for the key down event when
 * the call back field is the period (month, quarter, half.)  The user can use
 * the up / down arrows, home, end, page up, and page down keys to cycle through
 * the periods.  All other keys are simply ignored.
 *
 * Note that when the period changes, the date stays the same. But, if we simply
 * return the same date, the DTP will not update its display. So, we add 1
 * second to the date, which in turn causes the DTP control to update the
 * display.
 */
::method updatePeriod unguarded private
  expose periods currentPeriod
  use strict arg dt, vkey

  select
    when vKey == .VK~UP then do
      currentPeriod += 1
      if currentPeriod > periods~items then currentPeriod = 1
      newDT = dt~addSeconds(1)
    end

    when vKey == .VK~Down then do
      currentPeriod -= 1
      if currentPeriod < 1 then currentPeriod = periods~items
      newDT = dt~addseconds(1)
    end

    -- Since there are only 3 periods, Page Up and Home will act the same.
    when vKey == .VK~PRIOR | vKey == .VK~HOME then do
      currentPeriod = 1
      newDT = dt~addSeconds(1)
    end

    -- Since there are only 3 periods, Page Down and End will act the same.
    when vKey == .VK~NEXT | vKey == .VK~END then do
      currentPeriod = periods~items
      newDT = dt~addseconds(1)
    end

    otherwise do
      newDT = dt
    end
  end
  -- End select

  return newDT


::method updateReport unguarded private
  expose types currentType
  use strict arg dt, vkey

  select
    when vKey == .VK~UP then do
      currentType += 1
      if currentType > types~items then currentType = 1

      newDT = self~getFirstDayOfPeriod

      .RadioButton~checkInGroup(self, .constDir[IDC_RB_PRE_FIRST] + 1,    -
                                      .constDir[IDC_RB_ANTE_LAST] - 1,    -
                                      .constDir[IDC_RB_PRE_FIRST] + currentType)
    end

    when vKey == .VK~Down then do
      currentType -= 1
      if currentType < 1 then currentType = types~items

      newDT = self~getFirstDayOfPeriod

      .RadioButton~checkInGroup(self, .constDir[IDC_RB_PRE_FIRST] + 1,    -
                                      .constDir[IDC_RB_ANTE_LAST] - 1,    -
                                      .constDir[IDC_RB_PRE_FIRST] + currentType)
    end

    otherwise do
      newDT = dt
    end
  end
  -- End select

  return newDT


/** getFirstDay()
 *
 * Returns the first day of the current period.  I.e., if the current period is
 * second half 2010, the first day is 6/1/2010
 */
::method getFirstDay private
  expose periods currentPeriod
  use strict arg dtpSelected

  yr = dtpSelected~year
  mn = dtpSelected~month

  select
    when periods[currentPeriod] == 'month' then do
      mn = mn~right(2, 0)
    end

    when periods[currentPeriod] == 'quarter' then do
      if mn <= 3 then mn = 01
      else if mn <= 6 then mn = 04
      else if mn <= 9 then mn = 07
      else mn = 10
    end

    otherwise do
      if mn <= 6 then mn = 01
      else mn = 06
    end
  end
  -- End select

  return .DateTime~fromISODate(yr'-'mn'-01T08:00:00.000000')


/** getPeriod()
 *
 * Returns the number word (first, second, etc., for the specified date based on
 * the current period.  I.e., if the date is 7/4/1999 and current period is
 * month then the number word is 7
 */
::method getPeriodNumber private
  expose numbers periods currentPeriod
  use strict arg dt

  mn = dt~month

  select
    when periods[currentPeriod] == 'month' then do
      word = numbers[mn]
    end

    when periods[currentPeriod] == 'quarter' then do
      if mn <= 3 then word = numbers[1]
      else if mn <= 6 then word = numbers[2]
      else if mn <= 9 then word = numbers[3]
      else word = numbers[4]
    end

    otherwise do
      if mn <= 6 then word = numbers[1]
      else word = numbers[2]
    end
  end
  -- End select

  return word


::method getFirstDayOfPeriod private
  expose dtp
  use strict arg

  current = dtp~getDateTime
  dt = self~getFirstDay(current)

  if dt == current then do
    dt = dt~addSeconds(1)
  end

  return dt


::method calcSize unguarded private
  expose dtp periods numbers types
  use strict arg which

  biggest = .Size~new(0, 0)
  size   = .Size~new

  select
    when which == 'XX' then a = numbers
    when which == 'XXX' then a = periods
    otherwise a = types
  end
  -- End select

  do text over a
    dtp~textSize(text, size)
    if biggest < size then biggest~equateTo(size)
  end

  return biggest

::method setUpArrays private
  expose periods numbers types currentPeriod currentType

  periods = .Array~of('month', 'quarter', 'half')

  numbers = .Array~of('First',    'Second',  'Third',  'Fourth', 'Fifth', -
                      'Sixth',    'Seventh', 'Eighth', 'Ninth',  'Tenth', -
                      'Eleventh', 'Twelfth')

  types = .Array~of('balance', 'payroll', 'cash flow', 'income', 'earnings')

  currentPeriod = 2
  currentType = 1

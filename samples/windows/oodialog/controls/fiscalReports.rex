/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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

  sd = locate()
  .application~setDefaults('O', sd'fiscalReports.h', .false)

  dlg = .FiscalReports~new(sd'fiscalReports.rc', IDD_FISCAL_REPORTS)
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

  self~connectButtonEvent(IDC_PB_PRINT, 'CLICKED', onPrint)

::method initDialog
  expose dtp currentType

  dtp = self~newDateTimePicker(IDC_DTP_REPORT);
  self~newRadioButton(IDC_RB_BAL_SHEET)~check

  -- Set up the range of the DTP control.  We say we can't print a report in the
  -- future, so the maximum date is set to the current time.  We say the company
  -- first opened its doors on 3/2/1998 (which happens to be a Monday) at 8 AM,
  -- so a report can not start prior to that date.
  now = .DateTime~new
  start = .DateTime~fromIsoDate('1998-03-02T08:00:00.000000')
  dtp~setRange(.Array~of(start, now))

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
 * sends the FORMAT notification.  This is what drives our program.  When the
 * user presses the up or down arrow, home, end, page up, or page down keys in
 * one of the call back fields we send a modified date and time back to the DTP
 * control.  The DTP control generates a FORMAT event, and in our FORMAT event
 * handler we return the updated value for the call back field.
 */
::method onKeyDown
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


/** onRbClick()
 *
 * This is the event handler for the click event of a radio button.  We
 * connected the event for every radio button to this one method.  Each time the
 * user clicks on a radio button, it selects a new report type.  In our program,
 * when a new report type is selected, the initial date for the report is set to
 * the first day of the current period.  I.e., if the current period is 'month'
 * and the date is 7/10/2008, then the first day of the period is 7/1/2008.
 *
 * As explained in the comments for the onFormat() method above, when the date
 * and time is changed in the DTP control, the control generates a FORMAT event.
 * Here we update the index to the type of report, and set the new date and
 * time.  In the FORMAT event handler, the updated report type index causes the
 * event handler to return the text for the new report type.  And the display in
 * the DTP control is automatically updated to reflect the user's report
 * selection.
 */
::method onRbClick unguarded
  expose dtp currentType
  use arg info, hwnd, id

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
      -- Should be impossible to get here.
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


/** onPrint()
 *
 * The event handler for the CLICK event for the 'Print' push button.  A real
 * program would of course print the report using the details specified by the
 * user through the DTP control.  This is just an example program, we simply
 * put up a message box.
 */
::method onPrint unguarded
  expose types currentType dtp
  use arg info, hwndFrom

  select
    when currentType == 1 then report = 'Balance Sheet'
    when currentType == 2 then report = 'Payroll'
    when currentType == 3 then report = 'Cash Flow'
    when currentType == 4 then report = 'Income Statement'
    when currentType == 5 then report = 'Earnings'
    otherwise report = 'Error Report NOT Found'
  end
  -- End select

  dt = dtp~getDateTime

  if report~word(1) == 'Error' then do
    title = report
    msg   = 'An internal software error has occurred preventing'.endOfLine     || -
            'the printing of the report.' || .endOfLine~copies(2)              || -
            'Please contact IT at "internalsupport@acmewidgets.com"'.endOfLine || -
            'to resolve this issue.'

    icon = 'ERROR'
  end
  else do
    title = "Printing the" report "Report"
    msg   = 'The' report 'report is being sent to the default'.endOfLine    || -
            'printer' || .endOfLine~copies(3)                               || -
            'Report Start Time:' || '09'x || dt~civilTime  || .endOfLine    || -
            'Report Start Date:' || '09'x || dt~languageDate

    icon = 'INFORMATION'
  end

  j = MessageDialog(msg, self~hwnd, title, 'OK', icon)


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


/** updateReport()
 *
 * This method is invoked during the event handler for the key down event when
 * the call back field is the report type (balance, cash flow, etc...)  The user
 * can use the up / down arrows, home, end, page up, and page down keys to cycle
 * through the periods.  All other keys are simply ignored.
 *
 * Note that when the report type changes, the date gets set to the first day of
 * the period, and the selected radio button gets changed.  For the user, the
 * effect of using one of the arrow keys in the report type call back field is
 * exactly the same as selecting a report by using the radio buttons.
 */
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
 * month then the number word is Seventh.
 */
::method getPeriodNumber unguarded private
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


/** getFirstDayOfPeriod()
 *
 * Returns a .DateTime object that represents the first day of the period.  The
 * first day of the period is based on the currently selected date in the DTP
 * control and the setting of the period.  I.e., if the date is 5/13/2003 and
 * the period is quarter, then the first day of the period is 4/1/2003.
 *
 * Note that the .DateTime object returned here is used to update the DTP
 * control, where we need the DTP control to generate a FORMAT event.  If the
 * returned date is exactly the same as the currently selected date, the DTP
 * control does nothing.  So, we check for that possibility and add 1 second to
 * the return.  This is enough to force the FORMAT event notification we need.
 */
::method getFirstDayOfPeriod private
  expose dtp
  use strict arg

  current = dtp~getDateTime
  dt = self~getFirstDay(current)

  if dt == current then do
    dt = dt~addSeconds(1)
  end

  return dt


/** calcSize()
 *
 * Calculates the size needed to fully display the largest string in the
 * specified call back field.  The DTP control uses this size to ensure that
 * the display is formatted correctly.
 *
 * For the specified call back field, we iterate over the items for the field,
 * calculate the size in pixels needed for the item, and save the biggest size.
 * We could have just eye-balled the biggest item in the field and calculated
 * its size.  For instance, 'quarter' looks to be the biggest item in the period
 * call back.  However, doing it this way ensures we get the correct display
 * size and allows the program to be enhanced by adding items to any of the call
 * back fields without having to change this method.
 */
::method calcSize unguarded private
  expose dtp periods numbers types
  use strict arg which

  biggest = .Size~new(0, 0)
  size   = .Size~new

  select
    when which == 'XX' then array = numbers
    when which == 'XXX' then array = periods
    otherwise array = types
  end
  -- End select

  do text over array
    dtp~textSize(text, size)
    if biggest < size then biggest~equateTo(size)
  end

  return biggest

/** setUpArrays()
 *
 * Simple house keeping, initialize an array of items for each call back field.
 */
::method setUpArrays private
  expose periods numbers types currentPeriod currentType

  periods = .Array~of('month', 'quarter', 'half')

  numbers = .Array~of('First',    'Second',  'Third',  'Fourth', 'Fifth', -
                      'Sixth',    'Seventh', 'Eighth', 'Ninth',  'Tenth', -
                      'Eleventh', 'Twelfth')

  types = .Array~of('balance', 'payroll', 'cash flow', 'income', 'earnings')

  -- Set the starting period, 'quarter', and report, 'balance'.
  currentPeriod = 2
  currentType = 1

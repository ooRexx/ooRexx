/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2014 Rexx Language Association. All rights reserved.    */
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
 * Simple example of using the month calendar control.
 *
 * Although this example is relatively simple, it shows how to use a number of
 * features of the month calendar:
 *
 *   * How to size a month calendar to its optimal size.
 *   * How to restrict the range of months the calendar displays.
 *   * How to determine which month(s) are currently displayed in the control.
 *   * How to customize the calendar and display certain dates in bold.
 *   * How to connect and respond to the GETDAYSTATE event.
 *
 * The application is for a hypothetical company, used to show the paid holidays
 * for the current year. The dialog displays a month calendar for the current
 * year and as the user pages through the months, the paid holidays are high-
 * lighted in the calendar by bolding the dates.
 *
 * Note that to keep this simple, the "current" year is rather hard-coded.
 * However, it would not be too hard to enhance the program to calculate the
 * paid holidays dynamically.
 *
 * To fully understand this program, please read the ooDialog doc.
 */

  sd = locate()
  .application~useGlobalConstDir("O", sd"paidHolidays.h")

  dlg = .HolidayCalendarDlg~new(sd"paidHolidays.rc", IDD_HOLIDAY_DLG)

  if dlg~initCode == 0 then do
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
  end
  else do
    say
    say "Error creating dialog, aborting."
    return 99
  end

return 0

::requires "ooDialog.cls"

::class 'HolidayCalendarDlg' subclass RcDialog

::method initDialog
  expose calendar

  -- Connect the GETDAYSTATE event.
  self~connectMonthCalendarEvent(IDC_MC_HOLIDAYS, "GETDAYSTATE", onGetDayState)

  calendar = self~newMonthCalendar(IDC_MC_HOLIDAYS)

  -- Size the calendar to its optimal size
  self~sizeCalendar(calendar)

  -- Restrict the calendar so that it only displays the year 2011.
  start = .DateTime~fromStandardDate("20110101")
  end = .DateTime~fromStandardDate("20111231")

  calendar~setRange(.array~of(start, end))

  -- Determine which months are currently displayed in the calendar.  The first
  -- and last months showing are returned in the passed in array as .DateTime
  -- objects.  The return from getMonthRange() is the number of months
  -- displayed.  The 'P' argument means to include Partially shown months.
  monthsShowing = .array~new
  count = calendar~getMonthRange(monthsShowing, 'P')

  -- Bold the appropriate days in the month(s) currently showing.  To do that,
  -- we need an array of .DayState objects.
  dayStates = self~getDayStateArray(monthsShowing[1], count)
  calendar~setDayState(dayStates)


-- This is the GETDAYSTATE event handler.  The underlying month calendar sends
-- this notification whenever it needs to refresh the calendar display for a
-- new month.  The arguments are a .DateTime object that specifies the first
-- month to get the day state for and the second argument is the count of months
-- needed.  To reply to this notification we need to return a buffer containing
-- the required day states.  Creating the buffer correctly needs to be done
-- through the .DayStates class, where a class method is provided to create the
-- buffer.
::method onGetDayState unguarded
  expose calendar
  use arg startDate, count

  dayStates = self~getDayStateArray(startDate, count)

  buffer = .DayStates~makeDayStateBuffer(dayStates)
  return buffer


-- This function resizes the calendar to its optimal size.  The getMinRect()
-- method of the MonthCalendar returns the minimum required size to completely
-- display a full month.  The method fills in the .Rect object with the correct
-- size.  The top and left attributes in the returned .Rect are always set to 0,
-- so the right and bottom attributes are the width and height, in pixels,
-- needed for the calendar.  We then use the return to adjust the size of the
-- calendar, while leaving the postion of the top, left corner of the calendar
-- at the same spot.
--
-- The dialog was originally designed on XP using a resource editor, and looked
-- great.  But, on Vista and Windows 7, the calendar takes up more room and
-- overlaps the static text and Ok button controls.  So, if it is Vista or
-- later, we adjust the size of the dialog and the position of those controls.
-- The adjustment is really just a matter of getting some data, (sizes and
-- current positions,) and doing the math.
--
-- The same relative positioning of the controls is done.  I.e., in the original
-- dialog template the top of the Ok button is placed at the bottom edge of the
-- month calendar control.  The static text control is placed 1/2 the margin to
-- the right of calendar, etc..
::method sizeCalendar private
  use strict arg calendar

  r = .Rect~new
  if \ calendar~getMinRect(r) then return 0

  calendar~resizeTo(.Size~new(r~right, r~bottom))

  if .OS~isAtLeastVista then do
    static = self~newStatic(IDC_ST_MSG)
    button = self~newPushButton(IDOK)

    -- Get the current window rectangles of the controls and mapped them to the
    -- client area of the dialog.
    mcRect = calendar~windowRect
    self~screen2client(mcRect)

    stRect = static~windowRect
    self~screen2client(stRect)

    okRect = button~windowRect
    self~screen2client(okRect)

    -- The position of the top left corner of the month calendar control defines
    -- the margins
    marginLeft = mcRect~left
    marginTop  = mcRect~top

    -- Calculate what the new size of the dialog will need to be to contain the
    -- moved controls and resize the dialog to that.
    newWidth = (marginLeft * 2) + (marginLeft % 2) + (mcRect~right - mcRect~left) + (stRect~right - stRect~left)
    newHeight = (marginTop * 2) + (mcRect~bottom - mcRect~top) + (okRect~bottom - okRect~top)

    -- The new size of the dialog has to take into account the size of the
    -- window frame and the caption bar.
    newWidth  += .SM~cxFixedFrame * 2
    newHeight += (.SM~cyFixedFrame * 2) + .SM~cyCaption
    self~resizeTo(newWidth, newHeight)

    -- Move the static control to the right of the month calendar with a space
    -- 1/2 the width of the marging between them
    newX = mcRect~right + (marginLeft % 2)
    static~moveTo(newX, stRect~top)

    -- Move the Ok button so that the top edge of the button aligns with the
    -- bottom edge of the month calendar and the right edge of the button is
    -- even with the margin on the right.  (Left and right margins are equal.)
    clRect = self~clientRect
    newX = clRect~right - marginLeft - (okRect~right - okRect~left)
    button~moveTo(newX, mcRect~bottom)
  end


-- This function returns an array of .DayState objects for the specified months.
-- A day state object specifies which days in a month should be bolded.
::method getDayStateArray private unguarded
  use strict arg startMonth, count

  -- Create the array to hold the .DayState objects.
  d = .array~new(count)

  -- We know that the month calendar always asks for any partially displayed
  -- months.  We also know that our calendar only displays 1 month completely,
  -- and displays the partial month preceding and the partial month following.
  -- Therefore, if the complete month is January 2011, the preceding month will
  -- be December 2010.  Likewise we know that the range set for the calendar is
  -- January 2011 through December 2011, January 2012 can never be shown, so
  -- December 2011 can never be a preceding month.  Therefore, if the start
  -- month is 12, it must be December 2010.  We give that month a 0 number so
  -- that getDayState() will return an empty .DayState object.
  month = startMonth~month
  if month == 12 then month = 0

  do i = 1 to count
    d[i] = self~getDayState(month + i - 1)
  end

  return d

-- This function initalizes a .DayState object to the proper value depending on
-- the month specified.
::method getDayState private unguarded
  use strict arg month

  select
    when month ==  1 then ds = .DayState~new(17)
    when month ==  2 then ds = .DayState~new(21)
    when month ==  3 then ds = .DayState~new
    when month ==  4 then ds = .DayState~new
    when month ==  5 then ds = .DayState~new(30)
    when month ==  6 then ds = .DayState~new
    when month ==  7 then ds = .DayState~new(4)
    when month ==  8 then ds = .DayState~new
    when month ==  9 then ds = .DayState~new(5)
    when month == 10 then ds = .DayState~new
    when month == 11 then ds = .DayState~new(24, 25)
    when month == 12 then ds = .DayState~new(23, 30)
    otherwise ds = .DayState~new()
  end
  -- End select

  return ds

/*
 * Example for month calendar control.
 */

  dlg = .HolidayCalendarDlg~new("paidHolidays.rc", IDD_HOLIDAY_DLG, , "paidHolidays.h")
  if dlg~initCode == 0 then do
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
  end
  else do
    say
    say "Error creating dialog, aborting."
  end

return 0

::requires "ooDialog.cls"

::class 'HolidayCalendarDlg' subclass RcDialog

::method initDialog
  expose calendar

  self~connectMonthCalendarEvent(IDC_MC_HOLIDAYS, "GETDAYSTATE", onGetDayState)

  calendar = self~newMonthCalendar(IDC_MC_HOLIDAYS)

  self~sizeCalendar(calendar)

  start = .DateTime~fromStandardDate("20110101")
  end = .DateTime~fromStandardDate("20111231")

  calendar~setRange(.array~of(start, end))

  monthsShowing = .array~new
  count = calendar~getMonthRange(monthsShowing, 'P')

  dayStates = self~getDayStateArray(monthsShowing[1], count)
  calendar~setDayState(dayStates)


::method onGetDayState unguarded
  expose calendar
  use arg startDate, count

  dayStates = self~getDayStateArray(startDate, count)

  buffer = .DayStates~makeDayStateBuffer(dayStates)
  return buffer


::method sizeCalendar private
  use strict arg calendar

  r = .Rect~new
  if calendar~getMinRect(r) then calendar~resizeTo(.Size~new(r~right, r~bottom))


::method getDayStateArray private
  use strict arg startMonth, count

  d = .array~new(count)
  month = startMonth~month
  if month == 12 then month = 0

  do i = 1 to count
    d[i] = self~getDayState(month + i - 1)
  end

  return d


::method getDayState private
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

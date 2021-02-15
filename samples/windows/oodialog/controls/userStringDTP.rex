/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
 * An example of a date and time picker control and how to handle the USERSTRING
 * event notification.
 *
 * This program mimics an application that could be used to change the system
 * date and time.  It allows the user to enter a new system date and or time
 * through a DTP control.  Once a new date and / or new time are displayed in
 * the DTP control, if the user clicks the ok button the system date and time
 * is updated to the date and time displayed in the control.
 *
 * When a DTP control has the CANPARSE style, it allows the user to directly
 * enter text in the display area of the DTP.  When the user is done editing,
 * a USERSTRING event notification is sent.  The programmer connects this event
 * and then handles the notification by updating the DTP control based on the
 * text the user entered.
 *
 * This program allows the user to directly enter what are termed 'shortcut'
 * strings.
 *
 * When the program starts, the DTP control is set to the current system date
 * and time.  Of course as the dialog executes the system time is advancing.  If
 * the user has changed the DTP to a new date and / or time, but then wants to
 * undo those changes, it would be difficult to manually set the DTP control to
 * the current system date and time.  So that operation is one set of shortcuts.
 *
 * The other set of shortcuts allows the user to directly go to some time or
 * date by just typing it in.  For instance, if the user wanted to change the
 * date to May 12, 2050, rather than page through the DTP control to get there,
 * the user can just type 5/12/2050 directly in the DTP control.
 *
 * To directly type in the DTP control, the user can either use the F2 key, or
 * mouse click on the DTP control's display area when the DTP currently has the
 * focus.  This is a (little known?) feature of the DTP control itself.  It is
 * not something that the ooDialog framework provides.
 *
 * The valid shortcuts are these:
 *
 * Reset shortcut:
 *
 *   This shortcut resets any changes to the date and or time to none.  It also
 *   resets the date and time picker control to the current systeme date and
 *   time.
 *
 *   Type 'reset', 'cancel' 'r', or 'c'  Case is not significant.
 *
 * New date and or new time shortcut:
 *
 *   This shortcut allows directly entering a new date, or a new time, or both.
 *
 *   Formats for new date:  mm/dd, or mm/dd/yyyy
 *
 *   Formats for new time:  hh:mm, or hh:mm:ss
 *
 *   When both date and time are entered, the date must precede the time. The
 *   date and time must be separated by one or more spaces.  It is not required
 *   to enter a date and a time.  Only a new date can be entered or only a new
 *   time.
 *
 *   Months (mm), days (dd), hours (hh), minutes (mm), or seconds(ss) can
 *   contain a leading 0, but are not required to.  I.e., for June 5th, the new
 *   date string can be 06/05, 6/05, 06/5, or 6/5.  The year (yyyy) portion of
 *   the new date string is required to be the 4 digit year.  I.e. 6/5/11 is not
 *   valid for June 5th 2011.
 */

  sd = locate()
  .application~setDefaults('O', sd'userStringDTP.h', .false)

  dlg = .SystemTimeDlg~new(sd'userStringDTP.rc', IDD_SYSTEMTIME)
  dlg~execute("SHOWTOP")

return 0

::requires "ooDialog.cls"

::class 'SystemTimeDlg' subclass RcDialog

/** initDialog()
 *
 * initDialog() is automatically invoked by the ooDialog framework when the
 * underlying Windows dialog is created.  It is used to do initialization that
 * can only be done when the underlying dialog and dialog controls exist.
 *
 * This is a typical dialog initialization.
 */
::method initDialog
  expose curDate curTime dtp newDate newTime resetting colorIsSet stInvalid

  -- We need to be sure the Rexx stInvalid object exists before we connect the
  -- events to onFocus().  Otherwise, the events can fire with stInvalid being
  -- .nil.
  stInvalid = self~newStatic(IDC_ST_INVALID)~~setText("")

  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "SETFOCUS", onFocus)
  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "KILLFOCUS", onFocus)
  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "DROPDOWN", onFocus)

  -- Connect the help key event to our onHelp() method.
  self~connectHelp(onHelp)

  -- Connect the other DTP control event notifications and initialize the DTP.
  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "USERSTRING", onUserString)
  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "DATETIMECHANGE", onDateTime)

  dtp = self~newDateTimePicker(IDC_DTP_SYSTIME);
  dtp~setFormat("'Today is:' dddd MMMM dd, yyyy 'at' hh':'mm':'ss tt")

  -- Save references to the read only edit controls.
  curDate = self~newEdit(IDC_EDIT_CUR_DATE)
  curTime = self~newEdit(IDC_EDIT_CUR_TIME)
  newDate = self~newEdit(IDC_EDIT_NEW_DATE)
  newTime = self~newEdit(IDC_EDIT_NEW_TIME)

  -- Set the new date and new time fields to 'no change at present.'
  self~resetNewDateTime

  -- Begin updating the current date and time fields with the current system
  -- date and time.
  self~start('updateSysTime')


/** resetNewDateTime()
 *
 * The new date and new time fields display the date and time that the user has
 * selected to set the system date and time to.  The fields are updated every
 * time the user changes the date and time in the DTP control.
 *
 * However, once the user has changed the DTP time and date, one of the
 * shortcuts can be used to reset the DTP to the current system date and time.
 * At this point the new date and new time fields are set to blank and the color
 * of the fields is set to green, to indicate that no change will be made if the
 * user clicks the ok button a this time.
 */
::method resetNewDateTime unguarded private
  expose newDate newTime resetting colorIsSet

  newDate~setText('')
  newTime~setText('')

  self~setControlColor(IDC_EDIT_NEW_DATE, 3)
  self~setControlColor(IDC_EDIT_NEW_TIME, 3)
  newDate~redraw
  newTime~redraw

  resetting = .false
  colorIsSet = .false


/** onDateTime()
 *
 * This is the event handler for the DATETIMECHANGE event.  This method is
 * invoked each time there is a change in the DTP control.
 *
 * There are 2 cases here we need to check for.  Either, the user is changing
 * the date and / or time to set the system to a new time, or the user has used
 * the short cut to set the DTP control to the current system data and time.
 *
 * When the user is resetting the DTP control to the current system time, we
 * need to set new date and new time back to blank - in effect undo any changes
 * that may have selected a new data and time.
 *
 * Otherwise, the user has changed the DTP control to reflect the date and time
 * she wants to change the system date and time to.  In this case we update the
 * new date and time fields to show what the system time will be set to if the
 * ok button is clicked.
 */
::method onDateTime unguarded
  expose dtp newDate newTime resetting colorIsSet stInvalid

  stInvalid~setText('')

  if resetting then do
    self~resetNewDateTime
  end
  else do
    parse value dtp~getDateTime with yy '-' mm '-' dd 'T' time '.' junk

    if \ colorIsSet then do
      self~setControlColor(IDC_EDIT_NEW_DATE, 13)
      self~setControlColor(IDC_EDIT_NEW_TIME, 13)
      colorIsSet = .true
    end

    newDate~setText(mm'/'dd'/'yy)
    newTime~setText(time)
  end

  return 0

/** onUserString()
 *
 * This is the event handler for the USERSTRING event.  It is invoked when the
 * user has elected to type a shortcut in the DTP control and has finished
 * editing.
 *
 * This is the main point of this example, to show how to handle the user string
 * event.  We examine what the user typed and determine if it is a valid
 * 'shortcut' string.  When it is valid, we return a .DateTime object with the
 * date and time we want the DTP control to update its display to.  If it is not
 * valid we return the same .DateTime object we received.
 *
 * When the returned date and time is different from the DTP control's current
 * date an it, the control updates its display to the returned date and time.
 * When the returned date and time is the same as the DTP controls current date
 * and time, the DTP does nothing.
 *
 * When users type a shortcut string, they will expect the DTP control's display
 * to change.  However, typos are common.  Without a clue as to what they did
 * wrong, it is hard for users to know what happened.  So, it the user string is
 * not valid, we display a clue to the problem by setting the IDC_ST_INVALID
 * static control's text to an error message.
 *
 * Other than those basic principles, it is merely a matter of determining if
 * the user typed a valid shortcut, or not.
 */
::method onUserString unguarded
  expose resetting stInvalid
  use arg dt, userStr, id, hwnd

  stInvalid~setText('')

  -- Check for the shortcut to set the DTP to the current date and time.
  upStr = userStr~upper
  if upStr == 'C' | upStr == 'R' | upStr == 'CANCEL' | upStr == 'RESET' then do
    resetting = .true
    return .DateTime~new
  end

  -- The rest is just mechanics.  There is no attempt to be clever, this is just
  -- a rather brute force approach.  Check for things that can't possibly be
  -- valid and return invalid for them.  Continue doing that until we can parse
  -- out month, day, year, hour, minutes, seconds.  Check that the parsed values
  -- are valid numbers, the return a new .DateTime object based on those values.

  dateStr = ''
  timeStr = ''

  if userStr~words == 2 then do
    dateStr = userStr~word(1)
    timeStr = userStr~word(2)
  end
  else if userStr~words == 1 then do
    if userStr~pos('/') <> 0 then dateStr = userStr
    else if userStr~pos(':') <> 0 then  timeStr = userStr
  end
  else do
    return self~invalid(userStr, dt)
  end

  slashes = dateStr~countStr('/')
  colons  = timeStr~countStr(':')

  if slashes == 0, colons == 0 then return self~invalid(userStr, dt)
  if slashes > 2 | colons > 2 then return self~invalid(userStr, dt)

  parse value dt~isoDate with yy '-' mm '-' dd 'T' hh ':' min ':' ss '.' junk

  nYY = -1; nMM = -1; nDD = -1; nHH = -1; nMin = -1; nSS = -1

  if slashes == 1 then do
    parse var dateStr nMM '/' nDD .
    nMM = nMM~strip~strip( , '0')
    nDD = nDD~strip~strip( , '0')

    if \ nMM~dataType('W') | \ nDD~dataType('W') then return self~invalid(userStr, dt)
    if nMM < 1 | nMM > 12 then return self~invalid(userStr, dt)
    if nDD < 1 | nDD > 31 then return self~invalid(userStr, dt)
  end
  else if slashes == 2 then do
    parse var dateStr nMM '/' nDD '/' nYY .
    nMM = nMM~strip~strip( , '0')
    nDD = nDD~strip~strip( , '0')
    nYY = nYY~strip~strip( , '0')

    if \ nMM~dataType('W') | \ nDD~dataType('W') | \ nYY~dataType('W') then return self~invalid(userStr, dt)
    if nMM < 1 | nMM > 12 then return self~invalid(userStr, dt)
    if nDD < 1 | nDD > 31 then return self~invalid(userStr, dt)
    if nYY < 1601 | nYY > 9999 then return self~invalid(userStr, dt)
  end

  if colons == 1 then do
    parse var timeStr nHH ':' nMin .
    nHH = nHH~strip~strip( , '0')
    nMin = nMin~strip~strip( , '0')

    if \ nHH~dataType('W') | \ nMin~dataType('W') then return self~invalid(userStr, dt)
    if nHH < 0 | nHH > 23 then return self~invalid(userStr, dt)
    if nMin < 0 | nMin > 59 then return self~invalid(userStr, dt)
  end
  else if colons == 2 then do
    parse var timeStr nHH ':' nMin ':' nSS .
    nHH = nHH~strip~strip( , '0')
    nMin = nMin~strip~strip( , '0')
    nSS = nSS~strip~strip( , '0')

    if \ nHH~dataType('W') | \ nMin~dataType('W') | \ nSS~dataType('W') then return self~invalid(userStr, dt)
    if nHH < 0 | nHH > 23 then return self~invalid(userStr, dt)
    if nMin < 0 | nMin > 59 then return self~invalid(userStr, dt)
    if nSS < 0 | nSS > 59 then return self~invalid(userStr, dt)
  end

  if nYY <> -1 then yy = nYY
  if nMM <> -1 then mm = nMM~right(2, 0)
  if nDD <> -1 then dd = nDD~right(2, 0)
  if nHH <> -1 then hh = nHH~right(2, 0)
  if nMin <> -1 then min = nMin~right(2, 0)
  if nSS <> -1 then ss = nSS~right(2, 0)

  -- Okay, things should be good, but we haven't checked for something like
  -- February 30, or June 31.  Construct our ISO string and then check it is
  -- valid.
  isoStr = yy'-'mm'-'dd'T'hh':'min':'ss'.000000'

  if \ self~validIso(isoStr) then return self~invalid(userStr, dt)

  nDT = .DateTime~fromIsoDate(isoStr)
  return nDT


/** updateSysTime()
 *
 * This method is started and runs asynchronously to the rest of the dialog.  It
 * updates the current System date and time as the dialog is running.
 *
 * When the dialog is started the current date and the current time read only
 * edit controls are set to the system date and time.  Of course, the correct
 * system date and time is constantly incrementing.  This method updates the
 * edit with the correct current time approximately every second.  We do this by
 * setting an alarm to fire every second that re-invokes this method.
 *
 * To get the current date and time, we instantiate a new .DateTime object each
 * time this method is inovked.  This makes things very easy for us, and auto-
 * matically takes care of any 'drift' in the time.  For instance, if we just
 * added a second, to the last time, each time we are invoked, we would slowly
 * drift away from the correct time.  This is due to the fact that the timer
 * can not be 100% accurate.  Also there is some time taken up in processing
 * each time we are invoked.
 */
::method updateSysTime unguarded
  expose curDate curTime updateAlarm

  -- If the dialog is ended, quit.
  if \ self~isDialogActive then return

  parse value .DateTime~new with yy '-' mm '-' dd 'T' time '.' junk
  curDate~setText(mm'/'dd'/'yy)
  curTime~setText(time)

  timerMsg = .Message~new(self, updateSysTime)
  updateAlarm = .Alarm~new(1, timerMsg)


/** onFocus
 *
 * This event handler is connected to several of the DTP control's event
 * notifications.  Its only purpose is to erase the 'Invalid shortcut string'
 * text.
 */
::method onFocus unguarded
  expose  stInvalid

  stInvalid~setText("")
  return 0


/** invalid()
 *
 * Convenience method called from onUserString() when the user typed an invalid
 * shortcut.  It sets a static control to message showing the invalid shortcut.
 * This gives the user a visual clue as to why the time was not updated.
 */
::method invalid unguarded private
  expose stInvalid
  use strict arg userStr, dt

  stInvalid~setText('Invalid shortcut string:' userStr)
  return dt


/** validIso()
 *
 * This convenience method is called from onUserString() when the method gets to
 * the point where it thinks it has a valid shortcut to update to a new date and
 * time.
 *
 * But, it hasn't checked for things like June 31 or February 29.
 *
 * Here, rather than do an elaborate table lookup to check for invalid days, we
 * take a simple approach.  We trap a syntax condition and then just try to
 * instantiate a .DateTime object with the string.  If a condition is raised, we
 * say - no the string is not valid.  If no condition is raised we say - yes the
 * string is valid.
 */
::method validIso unguarded private
  use strict arg str

  signal on syntax

  .DateTime~fromIsoDate(str)
  return .true

  syntax:
  return .false


/** cancel()
 *
 * The event handler for the cancel event.  This is invoked when the user closes
 * the dialog, either through the cancel button or the escape key.
 *
 * We intercept the event to cancel the 'current' time updating, and then
 * forward the message on to let the superclass handle the ending of the dialog
 * prorperly.
 *
 * This is probably not really needed.  If the alarm was not canceled, then the
 * next time the alarm went off, updatesysTime() would see that the dialog was
 * ended and quit on its own.
 */
::method cancel unguarded
  expose updateAlarm

  updateAlarm~cancel
  return self~cancel:super

/** ok()
 *
 * The event handler for the OK event.  This is invoked when the user clicks the
 * ok button.
 *
 * In a real application, this is where the system date and time would be
 * reset, if the user actually had a new date and time selected.  Here we just
 * put up a message box 'saying' we reset the date and time.
 *
 * Note that the newDate and newTime will not contain any text, *unless*, the
 * user has actually changed the date or time in the DTP control.  This makes a
 * convenient check to see if we need to do anthing.
 */
::method ok unguarded
  expose updateAlarm dtp newDate newTime

  updateAlarm~cancel

  nDate = newDate~getText
  if nDate \== "" then do
    nTime = newTime~getText
    msg = "Set new system date and time to" nDate nTime
    title = "System Date Time Reset"
    j = MessageDialog(msg, self~hwnd, title)
  end

  return self~ok:super


/** help()
 *
 * This is the event handler for the IDHELP command.  Any button or menu item
 * with the resource ID of IDHELP will generate the HELP command event.  The
 * ooDialog framework automatically connects the IDOK, IDCANCEL, and IDHELP
 * commands to the ok(), cancel(), and help() methods in the dialog object.
 *
 * The default help() method does nothing.  To do something the programmer over-
 * rides the method in her subclass.  Here we simply invoke the onHelp() method
 * to do the work.
 *
 * Do not confuse the HELP command event with the help key (F1) event.
 */
::method help
  self~onHelp

/** onHelp()
 *
 * This is the event handler for the help key (F1) event.  The event is
 * generated when the user presses the F1 key.
 *
 * The programmer connects this event to a method in the Rexx dialog by using
 * the connectHelp() method.  Here in this event handler, we show a modal dialog
 * with some help text.
 *
 * Do not confuse the help key (F1) event with the HELP command event.
 */
::method onHelp
  .SystemTimeHelp~new(.application~srcDir'userStringDTP.rc', IDD_HELP)~execute("SHOWTOP")


::class 'SystemTimeHelp' subclass RcDialog

::method initDialog
  expose newFont e visibleLines

  e = self~newEdit(IDC_HELP_TEXT)

  -- Create a mono-spaced font for the edit control that displays the help text.
  newFont = self~createFontEx('Courier New', 9)

  -- Set the font of the edit control to our custom font, set the text of the
  -- edit control to our help text.
  e~setFont(newFont)
  e~setText(getHelpText())


/** leaving()
 *
 * The leaving method is invoked automatically by the ooDialog framework
 * immediately before the dialog is destroyed.
 *
 * At this point, the underlying Windows dialog still exists.  This makes the
 * leaving method the proper place to do clean up.  Especially if some of the
 * clean up, as does deleteFont(), requires the underlying dialog to exist.
 *
 * Here we delete the mono-spaced font created for the help text display.
 */
::method leaving
  expose newFont

  self~deleteFont(newFont)


/** routine::getHelpText()
 *
 * This is convenience routine that returns the help text for the examp program.
 *
 * The help text is constructed here as a single string.
 *
 * The text could be contained in a file and read in when needed.  To reduce the
 * nubmer of files needed for the program, the text was just typed into the
 * program file.
 */
::routine getHelpText

  txt = -
  ""                                                                              || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "                   Changing the System Date and Time"                          || .endOfLine || -
  "                   ================================="                          || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    This program allows you to change the system date and or time by"          || .endOfLine || -
  "    setting the displayed date and time in the date and time picker control"   || .endOfLine || -
  "    in the center of the dialog to the desired new date (and / or new"         || .endOfLine || -
  "    time.)"                                                                    || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    User Interface"                                                            || .endOfLine || -
  "    =============="                                                            || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    Two fields on the left of the dialog show the current system date and"     || .endOfLine || -
  "    time."                                                                     || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    Two fields on the right of the dialog show the date and time the system"   || .endOfLine || -
  "    date and time will be changed to, if the user clicks the okay button at"   || .endOfLine || -
  "    that point.  When there is no change to the system date and time, the"     || .endOfLine || -
  "    fields are empty and colored greenish as a visual clue that nothing"       || .endOfLine || -
  "    will change.  When there will be a change to the system date and time"     || .endOfLine || -
  "    the fields are colored reddish."                                           || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    The new system date and time are specified by changing the date and /"     || .endOfLine || -
  "    or time in the date and time picker control in the center of the"          || .endOfLine || -
  "    dialog.  This is done through the standard date and time picker user"      || .endOfLine || -
  "    interface."                                                                || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    This program allows directly entering what are termed 'shortcut'"          || .endOfLine || -
  "    strings."                                                                  || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    When the program starts, the DTP control is set to the current system"     || .endOfLine || -
  "    date and time.  Of course as the dialog executes the system time is"       || .endOfLine || -
  "    advancing.  If the DTP is changed to a new date and / or time, but then"   || .endOfLine || -
  "    those changes need to be undone, it would be difficult to manually set"    || .endOfLine || -
  "    the DTP control to the current system date and time.  So that operation"   || .endOfLine || -
  "    is one set of shortcuts."                                                  || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    The other set of shortcuts allows directly changing the DTP display to"    || .endOfLine || -
  "    some time or date by just typing it in.  For instance, to change the"      || .endOfLine || -
  "    date to May 12, 2050, rather than page through the DTP control to get"     || .endOfLine || -
  "    there, 5/12/2050 can be typed directly in the DTP control."                || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    To directly type in the DTP control, use either the F2 key, or mouse"      || .endOfLine || -
  "    click on the DTP control's display area when the DTP currently has the"    || .endOfLine || -
  "    focus.  This is a (little known?) feature of the DTP control itself."      || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    The valid shortcuts are these:"                                            || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    Reset shortcut:"                                                           || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      This shortcut resets any changes to the date and or time to none.  It"   || .endOfLine || -
  "      also resets the date and time picker control to the current system"      || .endOfLine || -
  "      date and time."                                                          || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      Type 'reset', 'cancel' 'r', or 'c'  Case is not significant."            || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "    New date and or new time shortcut:"                                        || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      This shortcut allows directly entering a new date, or a new time,"       || .endOfLine || -
  "      or both."                                                                || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      Formats for new date:  mm/dd, or mm/dd/yyyy"                             || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      Formats for new time:  hh:mm, or hh:mm:ss"                               || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      When both date and time are entered, the date must precede the time."    || .endOfLine || -
  "      The date and time must be separated by one or more spaces.  It is not"   || .endOfLine || -
  "      required to enter both a date and a time.  Only a new date can be"       || .endOfLine || -
  "      entered, or only a new time."                                            || .endOfLine || -
  ""                                                                              || .endOfLine || -
  "      Months (mm), days (dd), hours (hh), minutes (mm), or seconds(ss) can"    || .endOfLine || -
  "      contain a leading 0, but are not required to.  I.e., for June 5th,"      || .endOfLine || -
  "      the new date string can be 06/05, 6/05, 06/5, or 6/5.  The year"         || .endOfLine || -
  "      (yyyy) portion of the new date string is required to be the 4 digit"     || .endOfLine || -
  "      year. I.e.  6/5/11 is not valid for June 5th 2011."                      || .endOfLine || -
  ""                                                                              || .endOfLine
  return txt

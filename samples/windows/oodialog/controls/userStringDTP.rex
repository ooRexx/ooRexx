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


/* Test the DTN_USERSTRING notification messag for a date and time picker
 * control.
 */

  .application~setDefaults('O', 'userStringDTP.h', .false)

  dlg = .SystemTimeDlg~new('userStringDTP.rc', IDD_SYSTEMTIME)
  dlg~execute("SHOWTOP")

return 0

::requires "ooDialog.cls"

::class 'SystemTimeDlg' subclass RcDialog

::method init

  forward class (super) continue

  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "USERSTRING", onUserString)
  self~connectDateTimePickerEvent(IDC_DTP_SYSTIME, "DATETIMECHANGE", onDateTime)


::method initDialog
  expose curDate curTime dtp newDate newTime resetting colorIsSet

  dtp = self~newDateTimePicker(IDC_DTP_SYSTIME);
  dtp~setFormat("'Today is:' dddd MMMM dd, yyyy 'at' hh':'mm':'ss tt")

  curDate = self~newEdit(IDC_EDIT_CUR_DATE)
  curTime = self~newEdit(IDC_EDIT_CUR_TIME)
  newDate = self~newEdit(IDC_EDIT_NEW_DATE)
  newTime = self~newEdit(IDC_EDIT_NEW_TIME)

  self~setControlColor(IDC_EDIT_NEW_DATE, 3)
  self~setControlColor(IDC_EDIT_NEW_TIME, 3)
  newDate~redraw
  newTime~redraw
  resetting = .false
  colorIsSet = .false

  self~start('updateSysTime')

::method onDateTime unguarded
  expose dtp newDate newTime resetting colorIsSet

  if resetting then do
    newDate~setText('')
    newTime~setText('')
    self~setControlColor(IDC_EDIT_NEW_DATE, 3)
    self~setControlColor(IDC_EDIT_NEW_TIME, 3)
    colorIsSet = .false
    resetting = .false
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

::method onUserString unguarded
  expose resetting
  use arg dt, userStr, id, hwnd

  upStr = userStr~upper
  if upStr == 'C' | upStr == 'R' | upStr == 'CANCEL' | upStr == 'RESET' then do
    resetting = .true
    return .DateTime~new
  end

  say 'userStr:' userStr

  slashes = userStr~countStr('/')
  colons  = userStr~countStr(':'); say 'slashes:' slashes 'colons:' colons

  if slashes == 0, colons == 0 then return dt
  if slashes > 2 | colons > 2 then return dt

  parse value dt~isoDate with yy '-' mm '-' dd 'T' hh ':' min ':' ss '.' junk

  nYY = -1; nMM = -1; nDD = -1; nHH = -1; nMin = -1; nSS = -1

  if slashes == 1 then do
    parse var userStr nMM '/' nDD .
    nMM = nMM~strip~strip( , '0')
    nDD = nDD~strip~strip( , '0'); say 'nMM:' nMM 'nDD:' nDD

    if \ nMM~dataType('W') | \ nDD~dataType('W') then return dt
    if nMM < 1 | nMM > 12 then return dt
    if nDD < 1 | nDD > 31 then return dt
  end
  else if slashes == 2 then do
    parse var userStr nMM '/' nDD '/' nYY .
    nMM = nMM~strip~strip( , '0')
    nDD = nDD~strip~strip( , '0')
    nYY = nYY~strip~strip( , '0'); say 'nMM:' nMM 'nDD:' nDD 'nYY:' nYY

    if \ nMM~dataType('W') | \ nDD~dataType('W') | \ nYY~dataType('W') then return dt
    if nMM < 1 | nMM > 12 then return dt
    if nDD < 1 | nDD > 31 then return dt
    if nYY < 1601 | nYY > 9999 then return dt
  end

  say 'nMM:' nMM 'nDD:' nDD 'nYY:' nyy

  if colons == 1 then do
    parse var userStr nHH ':' nMin .
    nHH = nHH~strip~strip( , '0')
    nMin = nMin~strip~strip( , '0')

    if \ nHH~dataType('W') | \ nMin~dataType('W') then return dt
    if nHH < 0 | nHH > 23 then return dt
    if nMin < 0 | nMin > 59 then return dt
  end
  else if colons == 2 then do
    parse var userStr nHH ':' nMin ':' nSS .
    nHH = nHH~strip~strip( , '0')
    nMin = nMin~strip~strip( , '0')
    nSS = nSS~strip~strip( , '0'); say 'nHH:' nHH 'nMin:' nMin 'nSS:' nSS

    if \ nHH~dataType('W') | \ nMin~dataType('W') | \ nSS~dataType('W') then return dt
    if nHH < 0 | nHH > 23 then return dt
    if nMin < 0 | nMin > 59 then return dt
    if nSS < 0 | nSS > 59 then return dt
  end

  say 'nHH:' nHH 'nMin:' nMin 'nSS:' nSS

  if nYY <> -1 then yy = nYY
  if nMM <> -1 then mm = nMM~right(2, 0)
  if nDD <> -1 then dd = nDD~right(2, 0)
  if nHH <> -1 then hh = nHH~right(2, 0)
  if nMin <> -1 then min = nMin~right(2, 0)
  if nSS <> -1 then ss = nSS~right(2, 0)

  isoStr = yy'-'mm'-'dd'T'hh':'min':'ss'.000000'
  nDT = .DateTime~fromIsoDate(isoStr)

  say 'userStr' userStr
  say 'dt:    ' dt
  say 'nDT:   ' nDT

  return nDT


::method updateSysTime unguarded
  expose curDate curTime updateAlarm

  if self~finished then return

  parse value .DateTime~new with yy '-' mm '-' dd 'T' time '.' junk
  curDate~setText(mm'/'dd'/'yy)
  curTime~setText(time)

  timerMsg = .Message~new(self, updateSysTime)
  updateAlarm = .Alarm~new(1, timerMsg)


::method cancel unguarded
  expose updateAlarm

  updateAlarm~cancel
  return self~cancel:super

::method ok unguarded
  expose updateAlarm

  updateAlarm~cancel
  return self~cancel:super

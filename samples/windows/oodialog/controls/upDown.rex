/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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
 * UpDown control example.
 *
 * The dialog produced by this example contains 3 up down controls.  Two of the
 * up down controls are integer up down controls.  These are commonly referred
 * to as 'spinners.'
 *
 * The third up down control is used to simulate paging through database
 * records.  It demonstrates how the up down control is useful in non-integer
 * situations.
 *
 * The methods within the dialog class show the usage of mose of the methods of
 * the UpDown class.
 */

  dlg = .AnUpDownDlg~new("upDown.rc", IDD_UP_DOWN, , "resource.h" )
  dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
  dlg~deinstall

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::class 'AnUpDownDlg' subclass RcDialog

-- initDialog()
-- There are many different approaches that can be taken to doing the set up for
-- a dialog.  For myself, it seems simplest to just do all the initialization in
-- the initDialog() method, which is what is done here.
--
-- The initialization for this example falls naturally into 3 steps:
--  * Initialize the 2 integer up down controls and set up the data related to
--    maintaining the state for those controls.
--  * Initalize the 'client records' up down control and data structures related
--    to that up down.
--  * Connect all our button controls.
::method initDialog

  self~setupNumericUpDowns
  self~setupClientUpDown
  self~connectPushButtons


-- onClientChange()
-- Invoked when the client up down's position is changed.  I.e., the user clicks
-- on one of the arrows of the up down or uses the up or down arrow keys when
-- the focus is on the client edit control.
--
-- Three arguments are sent to the method: the current position of the up down,
-- the amount the position is to be changed (which can be negative depending on
-- the direction of the change,) and the window handle of the control.  We don't
-- need the window handle, so that arg is just ignored.
--
-- Each position of the up down maps to the index of a record in the client
-- database, so we just update the record fields using the new index.
::method onClientChange
  expose clientUPD clientDB
  use arg curPos, increment
  self~refreshClientDisplay(curPos + increment)


-- onChangeAcceleration()
-- Invoked when the user clicks on the "Change Acceleration" button.  We keep
-- track of the origianl acceleration values and then loop through 3 new values
-- each time the button is clicked.  Returning to the original values every 4th
-- click.
--
-- The acceleration values consist of an array of directory objects.  Each
-- directory object has a SECONDS index and an INCREMENT index.  This
-- essentially says after x seconds, set the increment to this value.  Typically
-- the first record is 0 seconds and an increment of 1.
--
-- To get the current acceleration of an up down control you use the
-- getAcceleration() method and an array as described above is returned.  To set
-- a new / different acceleration you construct an array as described above and
-- use the setAcceleration() method.  Note that the records (directory objects)
-- in the array have to be sorted by the seconds field, lowest to highest.
--
-- See the printAccelValues(), getFirstAccelIncrease(), and doubleAccel()
-- methods to see the acceleration array works.
::method onChangeAcceleration
  expose decUpDown originalAccel index

  currentAccel = decUpDown~getAcceleration
  reverting = .false

  if \ originalAccel~isA(.array) then do
    originalAccel = currentAccel
    index = 1
  end

  select
    when index == 1 then do
      newAccel = self~getFirstAccelIncrease(currentAccel)
      index = 2
    end
    when index == 2 then do
      newAccel = self~doubleAccel(currentAccel)
      index = 3
    end
    when index == 3 then do
      newAccel = self~doubleAccel(currentAccel)
      index = 4
    end
    otherwise do
      index = 1
      reverting = .true
      newAccel = originalAccel
    end
  end
  -- End select

  decUpDown~setAcceleration(newAccel)

  self~printAccelValues(currentAccel, newAccel, reverting)


-- onChangeRange()
-- Invoked when the "Change Range" button is clicked.  The range of the up down
-- control on the right is changed.  This up down starts out as base 16, but the
-- base may have been changed by the user.
--
-- The example program has a set of ranges and we just cycle though them.  We
-- keep a different set of ranges for base 10 and base 16.  The up down controls
-- seem to behave rather oddly if the base is 16 and the range includes negative
-- numbers.  So the decimal set has some ranges using negative numbers, but the
-- set for base 16 has only ranges with all positive positions.
::method onChangeRange
  expose hexUpDown decRanges hexRanges

  -- Get the index of the next range and keep track of the current position in
  -- the up down control.
  index = self~getNextRange
  currentPos = hexUpDown~getPosition

  -- Get the appropriate new range, and then set it.
  if hexUpDown~getBase == 10 then range = decRanges[index]
  else range = hexRanges[index]

  hexUpDown~setRange(range)

  -- Check if the current position is within the new range.  If not, change the
  -- current position.
  if currentPos < range~min | currentPos > range~max then hexUpDown~setPosition(range~max)

  -- Display to the user what we did.
  msg = 'Set new range for up down control on right to:' || .endOfLine || -
        '  minimum:' range~min                           || .endOfLine || -
        '  maximum:' range~max
  self~information(msg)


-- onChangeBase()
-- Invoked when the user pushes the "Change Base" putton.  Changes the base for
-- the integer up down control whose radio button is checked.
--
-- Integer up down controls can be either base 10 or base 16.  This method seems
-- to have a lot in it, but changing the base is actually simple.  The rest of
-- the code is to keep the user interface consistent and looking "good."
::method onChangeBase
  expose decUpDown hexUpDown leftUpDownDecimalRange leftUpDownHexadecimalRange

  if self~decimalRBSelected then do
    upd = decUpDown
    static = self~newStatic(IDC_ST_DECIMAL)
    side = 'left'
  end
  else do
    upd = hexUpDown
    static = self~newStatic(IDC_ST_HEXADECIMAL)
    side = 'right'
  end

  oldBase = upd~getBase
  if oldBase == 10 then do
    newBase = 16
    if side == 'right' then newText = ":Hexadecimal"
    else newText = "Hexadecimal:"
  end
  else do
    newBase = 10
    if side == 'right' then newText = ":Decimal"
    else newText = "Decimal:"
  end

  upd~setBase(newBase)
  static~setText(newText)

  -- Setting the position twice, is the only reliable way I've found to force
  -- the text in the buddy window (the edit control) to update.
  upd~setPosition(1)

  -- Read the comments for the setDecimalUpDown() method to see why we treat
  -- the range for the left-side up down, (the one that starts out as decimal,)
  -- special.
  if side == 'left' then do
    if base == 10 then upd~setRange(leftUpDownDecimalRange)
    else upd~setRange(leftUpDownHexadecimalRange)
  end

  upd~setPosition(0)

  msg = 'Changed the base of the up down control' || .endOfLine || -
        'on the' side 'from base' oldBase ' to'   || .endOfLine || -
        'base' newBase'.'
  self~information(msg)


-- onGetPostion()
-- Invoked when the user clicks the 'Get Position' push button.  We simply
-- display the value for the integer up down selected by a checked radio button.
::method onGetPosition
  expose decUpDown hexUpDown

  if self~decimalRBSelected then do
    pos = decUpDown~getPosition
    side = 'left'
  end
  else do
    pos = hexUpDown~getPosition
    side = 'right'
  end

  msg = "The position of the up down control on the" side "is" pos
  self~information(msg)


-- onGetBuddy()
-- Invoked when the "Get Buddy" push button is clicked.  The getBuddy() method
-- is used to get the 'buddy' control of the up down control.  The setBuddy()
-- method can be used to set the buddy control for an up down control, but that
-- method is not demonstrated in this example.  Here we just display the window
-- handle value.
::method onGetBuddy
  expose decUpDown hexUpDown

  if self~decimalRBSelected then do
    hwnd = decUpDown~getBuddy
    side = 'left'
  end
  else do
    hwnd = hexUpDown~getBuddy
    side = 'right'
  end

  msg = "The window handle of the up down control on the" side "is" hwnd
  self~information(msg)


-- connectPushButtons()
-- Convenience method, connects the clicked event of all the push buttons to
-- our corresponding event handling method.
::method connectPushButtons private

  self~connectButtonEvent(IDC_PB_ACCEL, "CLICKED", onChangeAcceleration)
  self~connectButtonEvent(IDC_PB_RANGE, "CLICKED", onChangeRange)
  self~connectButtonEvent(IDC_PB_BASE, "CLICKED", onChangeBase)
  self~connectButtonEvent(IDC_PB_POS, "CLICKED", onGetPosition)
  self~connectButtonEvent(IDC_PB_BUDDY, "CLICKED", onGetBuddy)

  -- At start up have the decimal radio button checked.
  self~newRadioButton(IDC_RB_DECIMAL)~check


-- decimalRBSelected()
-- Returns true if the decimal radio button is checked, otherwise false.
::method decimalRBSelected private
  return self~newRadioButton(IDC_RB_DECIMAL)~checked


-- setupNumericUpDowns()
-- Does all the initial set up for the two numeric up down controls.  The left
-- up down control starts out with base 10 (decimal) and the right up down
-- control starts out with base 16 (hexadecimal.)  However, these bases can be
-- changed after the dialog comes up.
::method setupNumericUpDowns private
  expose decUpDown hexUpDown

  decUpDown = self~newUpDown(IDC_UD_DECIMAL)
  hexUpDown = self~newUpDown(IDC_UD_HEXADECIMAL)

  -- By default an integer up down uses base 10 (deicmal.)  We set the other
  -- integer up down to base 16.
  hexUpDown~setBase(16)

  -- Set the minimum value in the range to 0 and the maximum range to 1024.  The
  -- range can also be set using a .directory object, see the setDecimalUpDonw()
  -- method.
  hexUpDown~setRange(0, 1024)

  -- Set the position at the top of the range.
  hexUpDown~setPosition(1024)

  -- Invoke some convenience methods to finish the set up.
  self~setDecimalUpDown
  self~setupRangeChanges


-- setDecimalUpDown()
-- Does the set up for the left-side up down.  This up down controls starts out
-- with a decimal base, but the user can change the base after the dialog has
-- come up.
--
-- The behavior of an up down control, when the base is set to 16, and the range
-- includes negative numbers, seems rather bizarre.  So, in this example, only
-- the left-hand side up down control uses ranges that include negative numbers.
-- When the left-hand up down has its base changed to base 16, the range is
-- changed to not include negative numbers.
::method setDecimalUpDown private
  expose decUpDown leftUpDownDecimalRange leftUpDownHexadecimalRange

  -- The setRange() method of the UpDown class accepts a Direcory object to set
  -- the range.  The MIN and MAX indexes of the Directory object set the range.
  leftUpDownDecimalRange = .directory~new
  leftUpDownDecimalRange~min = -200
  leftUpDownDecimalRange~max = 200

  leftUpDownHexadecimalRange = .directory~new
  leftUpDownHexadecimalRange~min = 0
  leftUpDownHexadecimalRange~max = 65536

  -- Set the range with the values used when the base is 10.
  decUpDown~setRange(leftUpDownDecimalRange)

  -- Set the position to -100.
  decUpDown~setPosition(-100)


-- setupRangeChanges()
-- Creates two arrays that contain different ranges for the integer up down
-- controls.  These ranges are used to change the range when the user clicks the
-- "Change Range" button.  The rangeIndex is used to keep track of where we are
-- and to select the new range.
::method setupRangeChanges private
  expose decRanges hexRanges rangeIndex

  rangeIndex = 1

  d1 = .directory~new~~setEntry("MIN", -400)~~setEntry("MAX", 400)
  d2 = .directory~new~~setEntry("MIN",   -5)~~setEntry("MAX", 5000)
  d3 = .directory~new~~setEntry("MIN",  600)~~setEntry("MAX", 700)
  decRanges = .array~of(d1, d2, d3)

  d1 = .directory~new~~setEntry("MIN", 1024)~~setEntry("MAX", 2048)
  d2 = .directory~new~~setEntry("MIN",    0)~~setEntry("MAX", 15)
  d3 = .directory~new~~setEntry("MIN",    0)~~setEntry("MAX", 700)
  hexRanges = .array~of(d1, d2, d3)


-- getNextRange()
-- Produce the next index to use when changing the range of an up down control
::method getNextRange private
  expose rangeIndex
  rangeIndex += 1
  if rangeIndex > 3 then rangeIndex = 1
  return rangeIndex


-- setupClientUpDown()
-- Sets up the third up down control.  This up down is meant to demonstrate how
-- the up down control can be used to scroll through things other than integers.
--
-- The example is for a set of data base records consisting of several fields.
-- The controlling field is the 'client.'  Each time the up down control has  a
-- position change, the matching data base record is displayed.
::method setupClientUpDown private
  expose clientUPD clientDB nameInfo ageInfo genderInfo paidInfo

  clientUPD = self~newUpDown(IDC_UD_CLIENT)

  -- We will use 20 records.
  clientDB = .array~new(20)
  self~fillClientDB(clientDB)

  -- 20 records, so use a range of 1 to 20.
  clientUPD~setRange(1, 20)

  -- Start at record 1.
  clientUPD~setPosition(1)

  -- Connect the position change event to our onClientChange() method
  self~connectUpDownEvent(IDC_UD_CLIENT, "DELTAPOS", onClientChange)

  nameInfo = self~newEdit(IDC_EDIT_CLIENT)
  ageInfo = self~newEdit(IDC_EDIT_AGE)
  genderInfo = self~newEdit(IDC_EDIT_GENDER)
  paidInfo = self~newEdit(IDC_EDIT_PAID)

  -- Set the initial values for the record.
  self~refreshClientDisplay(1)


-- getFirstAccelIncrease()
-- Generates new acceleration values using the default values of the up down
-- control.  Observation has shown that, on Windows XP, an up down control
-- starts out with an acceleration array of 3 values, the first being 0 seconds,
-- an increment of 1.  That is not documented and could not be a hard fast rule,
-- but works well enough for this example.
::method getFirstAccelIncrease private
  use strict arg accel

  newAccel = .array~new(4)
  newAccel[1] = accel[1]
  newAccel[2] = .directory~new~~setEntry("SECONDS", 1)~~setEntry("INCREMENT", 2)
  newAccel[3] = .directory~new~~setEntry("SECONDS", accel[2]~seconds)~~setEntry("INCREMENT", accel[2]~increment + 2)
  newAccel[4] = .directory~new~~setEntry("SECONDS", accel[3]~seconds)~~setEntry("INCREMENT", accel[3]~increment + 4)

  return newAccel


-- doubleAccel()
-- Generates new acceleration values by doubling the increment of the passed in
-- values.
::method doubleAccel private
  use strict arg accel

  newAccel = .array~new(accel~items)
  newAccel[1] = accel[1]

  do i = 2 to accel~items
    d = .directory~new
    d~seconds = accel[i]~seconds
    d~increment = accel[i]~increment * 2
    newAccel[i] = d
  end

  return newAccel


-- printAccelValues()
-- Convenience method to display acceleration values
::method printAccelValues private
  use strict arg currentAccel, newAccel, reverting

  tab = '09'x

  msg = "The current acceleration values are:"   || .endOfLine || -
        tab || "Items:  " currentAccel~items     || .endOfLine

  do a over currentAccel
    msg ||= tab || "Seconds:" a~seconds || tab || "Increment:" a~increment || .endOfLine
  end

  msg ||= .endOfLine

  if reverting then msg ||= "Reverting to original values of:" || .endOfLine
  else msg ||= "Changing acceleration values to:" || .endOfLine

  msg ||= tab || "Items:  " newAccel~items  || .endOfLine

  do a over newAccel
    msg ||= tab || "Seconds:" a~seconds || tab || "Increment:" a~increment || .endOfLine
  end

  self~information(msg)


-- refreshClientDisplay()
-- Updates the display to show the fields for the specified record.
::method refreshClientDisplay private
  expose nameInfo ageInfo genderInfo paidInfo clientDB
  use strict arg index

  -- There are numerous ways to handle reaching the end of the range for an up
  -- down control.  Here we put up a message box when either end of the range is
  -- reached.
  if index < 1 then do
    self~clientEnd("bottom")
    return
  end
  if index > 20 then do
    self~clientEnd("top")
    return
  end

  nameInfo~setText(clientDB[index][1])
  ageInfo~setText(clientDB[index][2])
  genderInfo~setText(clientDB[index][3])
  paidInfo~setText(clientDB[index][4])


-- clientEnd()
-- Put up a message box informing the user that they hit the end of the range of
-- records.
::method clientEnd private
  use strict arg whichEnd

  msg = "At the" whichEnd "of the client list."
  self~information(msg)


-- information()
-- A convenience method to display informational messages to the user.
::method information private
  use strict arg msg

  title = "UpDown Controls"
  button = "OK"
  icon = "INFORMATION"
  miscStyles = "APPLMODAL TOPMOST"
  j = messageDialog(msg, self~hwnd, title, button, icon, miscStyles)


-- fillClientDB()
-- Convenience method for this example.  Fills an array with the 'database'
-- records.
::method fillClientDB private
  use strict arg db

  db[1]  = .array~of("Cathy Smart", 44, "female", "yes")
  db[2]  = .array~of("Tom Jones", 34, "male", "no")
  db[3]  = .array~of("Bill Harris", 23, "male", "yes")
  db[4]  = .array~of("Larry Bonds", 41, "male", "yes")
  db[5]  = .array~of("Sue Evans", 55, "female", "no")
  db[6]  = .array~of("Ashley Wright", 17, "female", "yes")
  db[7]  = .array~of("Deb Newsome", 22, "female", "yes")
  db[8]  = .array~of("Frank Getts", 22, "male", "yes")
  db[9]  = .array~of("Betty Boop", 34, "female", "no")
  db[10] = .array~of("Fred Aston", 56, "male", "no")
  db[11] = .array~of("Cary Thule", 85, "female", "no")
  db[12] = .array~of("Brianna Medford", 24, "female", "yes")
  db[13] = .array~of("Sol Price", 26, "male", "no")
  db[14] = .array~of("Hugh Dentry", 41, "male", "no")
  db[15] = .array~of("Tina McGrath", 49, "female", "yes")
  db[16] = .array~of("Tom Denard", 26, "female", "yes")
  db[17] = .array~of("Crissy Albright", 21, "female", "yes")
  db[18] = .array~of("Phil Logan", 19, "male", "no")
  db[19] = .array~of("Walter Perkins", 50, "male", "yes")
  db[20] = .array~of("Zoe Sharpe", 28, "female", "no")


-- initAutoDetection()
-- We don't use auto detection, we initialize the controls to the state we want
-- in the initDialog() method.
::method initAutoDetection
  self~noAutoDetection

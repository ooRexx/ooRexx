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
 * A simple application that shows how to create an UserMenuBar menu.  The
 * dialog contains an Edit control and an UpDown control.  The menu is used to
 * change styles or behaviour of the two controls.
 *
 * About half of the menu items pop up a dialog to collect user input needed to
 * carry out the menu item action.
 *
 * This example uses a number of up-down controls in the different dialogs,
 * making it a good example of how to use up-down controls in addition, to being
 * a menu bar example.
 */

  sd = locate()
  .application~setDefaults("O", sd"UserMenuBar.h", .false)

  dlg = .SimpleDialog~new(sd"UserMenuBar.rc", IDD_MAIN_DIALOG)
  if dlg~initCode <> 0 then do
    return 99
  end

  dlg~execute("SHOWTOP")

return 0

::requires "ooDialog.cls"

-- We need 10 digits to work with the numbers in the full range of an up-down
-- control (-2147,483,648 to 2,147,483,647)
::options digits 10

::class 'SimpleDialog' subclass RcDialog

::constant DEFAULT_TEXT   "1, 2, 3, the Edit control Menu actions work better with text in the 1st edit control."
::constant WICKED_TEXT    "The wicked flee when none pursueth ..."
::constant LOTUS_TEXT     "Lotus 123 had its ups and downs."
::constant IDES_TEXT      "Ides of March - name of March 15 in Roman calendar."
::constant TITANIC_TEXT   "472 lifeboat seats not used when 1,503 people died on the Titanic."

::method init
  expose srcDir

  forward class (super) continue

  -- Grab the source dir value here and save it in an instance variable for
  -- convenience.
  srcDir = .application~srcDir

  if \ self~createMenuBar then do
    self~initCode = 1
    return
  end


::method initDialog
  expose menuBar edit upDown

  upDown = self~newUpDown(IDC_UPD)
  upDown~setRange(1, 20000)
  upDown~setPosition(1000)

  if \ menuBar~attachTo(self) then do
    msg = "Failed to attach menu bar System Error Code:" .SystemErrorCode
    z = MessageDialog(msg, self~hwnd, "Menu Error", "OK", "WARNING")
  end

  edit = self~newEdit(IDC_EDIT)
  edit~setText(.SimpleDialog~DEFAULT_TEXT)

  self~setRadioChecks(ID_EDITCONTROL_UNRESTRICTED)


-- Creates a UserMenuBar
::method createMenuBar private
  expose menuBar

  -- Create a menu bar that has a symbolic resource ID of IDM_MENUBAR, has no
  -- help ID, uses the default menu item count, and autoconnects all command
  -- menu items when it is attached to a dialog.
  menuBar = .UserMenuBar~new(IDM_MENUBAR, , , .true)

  -- Create the menu bar template.
  menuBar~addPopup(IDM_POP_FILES, "Files")
    menuBar~addItem(ID_FILES_HIDE_EDIT, "Hide Edit Control", "DEFAULT CHECK")
    menuBar~addItem(ID_FILES_HIDE_UPDOWN, "Hide UpDown Control", " CHECK")
    menuBar~addSeparator(IDM_SEP_FILES)
    menuBar~addItem(ID_FILES_EXIT, "Exit", "END")

  menuBar~addPopup(IDM_POP_EDITCONTROL, "Edit Control")
    menuBar~addItem(ID_EDITCONTROL_LOWER, "Lower Case Only", "CHECK RADIO")
    menuBar~addItem(ID_EDITCONTROL_NUMBER, "Numbers Only", "CHECK RADIO")
    menuBar~addItem(ID_EDITCONTROL_UPPER, "Upper Case Only", "CHECK RADIO")
    menuBar~addItem(ID_EDITCONTROL_UNRESTRICTED, "No Restriction", "CHECK RADIO DEFAULT")
    menuBar~addSeparator(IDM_SEP_EDITCONTROL)
    menuBar~addItem(ID_EDITCONTROL_INSERT, "Insert Text ...")
    menuBar~addItem(ID_EDITCONTROL_SELECT, "Select Text ...", "END")

  menuBar~addPopup(IDM_POP_UPDOWNCONTROL, "UpDown Control")
    menuBar~addItem(ID_UPDOWNCONTROL_HEXIDECIMAL, "Hexidecimal", "CHECK")
    menuBar~addSeparator(IDM_SEP_UPDOWNCONTROL)
    menuBar~addItem(ID_UPDOWNCONTROL_SET_ACCELERATION, "Set Acceleration ...")
    menuBar~addItem(ID_UPDOWNCONTROL_SET_RANGE, "Set Range ...")
    menuBar~addItem(ID_UPDOWNCONTROL_SET_POSITION, "Set Position ...", "END")

  menuBar~addPopup( IDM_POP_HELP, "Help", "END")
    menuBar~addItem(ID_HELP_ABOUT, "About User Menu Bar", "END")

  if \ menuBar~complete then do
    say 'User menu bar completion error:' .SystemErrorCode SysGetErrortext(.SystemErrorCode)
    return .false
  end

  return .true


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
--  The methods below, up to the next dividing lines, are the implementation
--  for each of the menu item command events.
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::method hideEditControl unguarded
  expose menuBar edit

  if menuBar~isChecked(ID_FILES_HIDE_EDIT) then do
    menuBar~uncheck(ID_FILES_HIDE_EDIT)
    self~newStatic(IDC_ST_EDIT)~show
    edit~show
  end
  else do
    menuBar~check(ID_FILES_HIDE_EDIT)
    self~newStatic(IDC_ST_EDIT)~hide
    edit~hide
  end

::method hideUpDownControl unguarded
  expose menuBar upDown

  if menuBar~isChecked(ID_FILES_HIDE_UPDOWN) then do
    menuBar~uncheck(ID_FILES_HIDE_UPDOWN)
    self~newStatic(IDC_ST_UPD)~show
    self~newEdit(IDC_EDIT_BUDDY)~show
    upDown~show
  end
  else do
    menuBar~check(ID_FILES_HIDE_UPDOWN)
    self~newStatic(IDC_ST_UPD)~hide
    self~newEdit(IDC_EDIT_BUDDY)~hide
    upDown~hide
  end

::method exit unguarded
  self~cancel


::method lowerCaseOnly unguarded
  expose menuBar edit

  alreadyChecked = menuBar~isChecked(ID_EDITCONTROL_LOWER)

  self~setRadioChecks(ID_EDITCONTROL_LOWER)
  edit~replaceStyle("UPPER NUMBER", "LOWER")

  text = edit~getText
  edit~setText(text~lower)

  if \ alreadyChecked then do
    edit~assignFocus
    edit~select(1, 1)

    msg = "You can only enter lower case letters in" || .endOfLine || -
          "the edit control now.  Try it."
    z = MessageDialog(msg, self~hwnd, 'Edit Control Style Change', "OK", "INFORMATION")
  end


::method numbersOnly unguarded
  expose menuBar edit

  alreadyChecked = menuBar~isChecked(ID_EDITCONTROL_NUMBER)

  self~setRadioChecks(ID_EDITCONTROL_NUMBER)
  edit~replaceStyle("LOWER UPPER", "NUMBER")

  text = edit~getText
  edit~setText(text~translate("", xrange("00"X, "/") || xrange(":", "FF"X))~space(0))

  if \ alreadyChecked then do
    edit~assignFocus
    edit~select(1, 1)

    msg = "You can only enter numbers in the" || .endOfLine || -
          "edit control now.  Try it."
    z = MessageDialog(msg, self~hwnd, 'Edit Control Style Change', "OK", "INFORMATION")
  end


::method upperCaseOnly unguarded
  expose menuBar edit

  alreadyChecked = menuBar~isChecked(ID_EDITCONTROL_UPPER)

  self~setRadioChecks(ID_EDITCONTROL_UPPER)
  edit~replaceStyle("LOWER NUMBER", "UPPER")

  text = edit~getText
  edit~setText(text~upper)

  if \ alreadyChecked then do
    edit~assignFocus
    edit~select(1, 1)

    msg = "You can only enter upper case letters in" || .endOfLine || -
          "the edit control now.  Try it."
    z = MessageDialog(msg, self~hwnd, 'Edit Control Style Change', "OK", "INFORMATION")
  end


::method noRestriction unguarded
  expose menuBar edit

  alreadyChecked = menuBar~isChecked(ID_EDITCONTROL_UNRESTRICTED)

  self~setRadioChecks(ID_EDITCONTROL_UNRESTRICTED)
  edit~removeStyle("LOWER NUMBER UPPER")

  text = edit~getText
  edit~setText(.SimpleDialog~DEFAULT_TEXT)

  if \ alreadyChecked then do
    edit~assignFocus
    edit~select(1, 1)

    msg = "You can now enter unrestricted text in" || .endOfLine || -
          "the edit control.  Try it."
    z = MessageDialog(msg, self~hwnd, 'Edit Control Style Change', "OK", "INFORMATION")
  end


::method insertText unguarded
  expose edit srcDir

  dlg = .InsertDialog~new(srcDir"UserMenuBar.rc", IDD_INSERT_DIALOG, , srcDir"UserMenuBar.h")

  if dlg~execute("SHOWTOP", IDI_DLG_OODIALOG) == .PlainBaseDialog~IDOK then do
    edit~setText(dlg~selectedText)
  end


::method selectText unguarded
  expose edit srcDir

  dlg = .SelectDialog~new(srcDir"UserMenuBar.rc", IDD_SELECT_DIALOG, , srcDir"UserMenuBar.h")
  dlg~currentText = edit~getText
  edit~select(1, 1)

  if dlg~execute("SHOWTOP", IDI_DLG_APPICON) == .PlainBaseDialog~IDOK then do
    s = dlg~selection
    edit~select(s~x, s~y)
  end


::method hexidecimal unguarded
  expose menuBar upDown

  if menuBar~isChecked(ID_UPDOWNCONTROL_HEXIDECIMAL) then do
    menuBar~uncheck(ID_UPDOWNCONTROL_HEXIDECIMAL)
    upDown~setBase(10)
  end
  else do
    menuBar~check(ID_UPDOWNCONTROL_HEXIDECIMAL)
    upDown~setBase(16)
  end


::method setAcceleration unguarded
  expose upDown srcDir

  dlg = .AccelDialog~new(srcDir"UserMenuBar.rc", IDD_ACCEL_DIALOG, , srcDir"UserMenuBar.h")

  if dlg~execute("SHOWTOP", IDI_DLG_APPICON2) == .PlainBaseDialog~IDOK then do
    accel = dlg~acceleration
    upDown~setAcceleration(accel)
  end


::method setRange unguarded
  expose upDown srcDir

  dlg = .RangeDialog~new(srcDir"UserMenuBar.rc", IDD_RANGE_DIALOG, , srcDir"UserMenuBar.h")

  if dlg~execute("SHOWTOP", IDI_DLG_OOREXX) == .PlainBaseDialog~IDOK then do
    r = dlg~range
    upDown~setRange(r~x, r~y)

    -- If the current position was no longer within the new range, the up-down
    -- control will have internally reset its position so that it is within the
    -- new range.  But, the value displayed will still be the old value.  This
    -- forces the value displayed to match the current position.
    upDown~setPosition(upDown~getPosition)
  end


::method setPosition unguarded
  expose upDown srcDir

  dlg = .PositionDialog~new(srcDir"UserMenuBar.rc", IDD_POSITION_DIALOG, , srcDir"UserMenuBar.h")
  dlg~upDown = upDown

  if dlg~execute("SHOWTOP", IDI_DLG_DEFAULT) == .PlainBaseDialog~IDOK then do
    p = dlg~position
    upDown~setPosition(p)
  end


::method aboutUserMenuBar unguarded
  expose srcDir

  dlg = .AboutDialog~new(srcDir"UserMenuBar.rc", IDD_ABOUT_DIALOG, , srcDir"UserMenuBar.h")

  dlg~execute("SHOWTOP", IDI_DLG_DEFAULT)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
--  End of the implementation methods for each of the menu item command events.
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/



-- Convenience method to set the radio button menu items.  The checkRadio()
-- method takes a start resource ID and an end resource, and the resource ID for
-- a single menu item within that range of IDs.  It removes the radio button
-- check mark from all the menu items in the range and adds the radio button
-- check mark to item specified by the third argument.
::method setRadioChecks private
  expose menuBar
  use strict arg item

  menuBar~checkRadio(ID_EDITCONTROL_LOWER, ID_EDITCONTROL_UNRESTRICTED, item)


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
--  The following classes all implement a single dialog that is used to collect
--  information, from the user, needed to carry out one of the menu item
--  commands.
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

::class 'InsertDialog' subclass RcDialog

::attribute selectedText

::method initDialog
  self~newRadioButton(IDC_RB_WICKED)~check

::method ok unguarded

  select
    when self~newRadioButton(IDC_RB_WICKED)~checked then self~selectedText = .SimpleDialog~WICKED_TEXT
    when self~newRadioButton(IDC_RB_LOTUS)~checked then self~selectedText = .SimpleDialog~LOTUS_TEXT
    when self~newRadioButton(IDC_RB_IDES)~checked then self~selectedText = .SimpleDialog~IDES_TEXT
    when self~newRadioButton(IDC_RB_TITANIC)~checked then self~selectedText = .SimpleDialog~TITANIC_TEXT
    otherwise self~selectedText = ""
  end
  -- End select

  self~ok:super


::class 'SelectDialog' subclass RcDialog

::attribute selection
::attribute currentText

::method initDialog
  expose updStart updEnd currentText

  self~newStatic(IDC_ST_CURRENT_TEXT)~setText(currentText)

  updStart = self~newUpDown(IDC_UPD_START)
  updStart~setRange(0, currentText~length)
  updStart~setPosition(1)

  updEnd = self~newUpDown(IDC_UPD_END)
  updEnd~setRange(0, currentText~length)
  updEnd~setPosition(1)

::method ok unguarded
  expose updStart updEnd

  self~selection = .Point~new(updStart~getPosition, updEnd~getPosition)
  self~ok:super



::class 'AccelDialog' subclass RcDialog

::attribute acceleration

::method initDialog

  upd = self~newUpDown(IDC_UPD_ACCEL_SECONDS0)
  upd~setPosition(0)
  upd~disable
  self~newEdit(IDC_EDIT_ACCEL_SECONDS0)~disable

  upd = self~newUpDown(IDC_UPD_ACCEL0)
  upd~setPosition(1)
  upd~disable
  self~newEdit(IDC_EDIT_ACCEL0)~disable

  do i = 1 to 3
    upd = self~newUpDown(IDC_UPD_ACCEL_SECONDS || i)
    upd~setRange(i, 32)
    upd~setPosition(i)

    upd = self~newUpDown(IDC_UPD_ACCEL || i)
    upd~setRange(2 ** i, 256)
    upd~setPosition(2 ** i)
  end

  self~newUpDown(IDC_UPD_ACCEL_SECONDS1)~assignFocus


::method ok unguarded

  a = .array~new(3)

  do i = 1 to 3
    d = .directory~new

    updS = self~newUpDown(IDC_UPD_ACCEL_SECONDS || i)
    updA = self~newUpDown(IDC_UPD_ACCEL || i)

    d~seconds = updS~getPosition
    d~increment = updA~getPosition
    a[i] = d
  end

  -- Check that the user has an unique value for each acceleration entry.
  check = .set~of(a[1]~seconds, a[2]~seconds, a[3]~seconds)
  if check~items <> 3 then do
    msg = "For each of the 3 acceleration input" || .endOfLine || -
          "lines, you must use a unique value for" || .endOfLine || -
          "seconds.  Found:" a[1]~seconds',' a[2]~seconds', and' a[3]~seconds'.'

    z = MessageDialog(msg, self~hwnd, "Acceleration Input Error", "OK", "STOP")
    return .false
  end

  -- Sort the entries by seconds, ascending, using brute force.
  max = 0; min = 4
  do i = 1 to 3
    if a[i]~seconds > max then max = i
    if a[i]~seconds < min then min = i
  end

  -- The user only has 3 up-down pairs she can set.  But, we also have the
  -- first disabled up-down pair for seconds == 0, increment == 1
  aa = .array~new(4)
  aa[1] = .directory~new~~setEntry("SECONDS", 0)~~setEntry("INCREMENT", 1)

  aa[2] = a[min]
  aa[4] = a[max]

  do i = 1 to 3
    if i \== min, i \== max then do
      aa[3] = a[i]
      leave
    end
  end

  self~acceleration = aa
  self~ok:super


::class 'RangeDialog' subclass RcDialog

::attribute range

::method initDialog
  expose updLow updHigh

  -- Set an acceleration that goes very fast if the user hold down the arrow
  -- keys, or holds down the mouse on the up / down arrows.
  accel = .array~new(4)
  accel[1] = .directory~new~~setEntry("SECONDS", 0)~~setEntry("INCREMENT", 1)
  accel[2] = .directory~new~~setEntry("SECONDS", 1)~~setEntry("INCREMENT", 32)
  accel[3] = .directory~new~~setEntry("SECONDS", 2)~~setEntry("INCREMENT", 64)
  accel[4] = .directory~new~~setEntry("SECONDS", 3)~~setEntry("INCREMENT", 256)

  updLow = self~newUpDown(IDC_UPD_LOW)
  updLow~setRange(-2147483648, 2147483647)
  updLow~setPosition(0)
  updLow~setAcceleration(accel)

  updHigh = self~newUpDown(IDC_UPD_HIGH)
  updHigh~setRange(-2147483648, 2147483647)
  updHigh~setPosition(0)
  updHigh~setAcceleration(accel)

::method ok unguarded
  expose updLow updHigh

  self~range = .Point~new(updLow~getPosition, updHigh~getPosition)
  self~ok:super


::class 'PositionDialog' subclass RcDialog

::attribute position
::attribute upDown

::method initDialog
  expose upDown upd

  upd = self~newUpDown(IDC_UPD_POSITION)

  upd~setRange(upDown~getRange)
  upd~setPosition(upDown~getPosition)

::method ok unguarded
  expose upd

  self~position = upd~getPosition
  self~ok:super


::class 'AboutDialog' subclass RcDialog

::method initDialog
   expose font

   bitmap = .Image~getImage(.application~srcDir"UserMenuBar.bmp")
   self~newStatic(IDC_ST_BITMAP)~setImage(bitmap)

   font = self~createFontEx("Ariel", 14)
   self~newStatic(IDC_ST_ABOUT)~setFont(font)

::method leaving
   expose font
   self~deleteFont(font)

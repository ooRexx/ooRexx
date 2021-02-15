/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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
 *  There are 3 types of combo boxes: simple, drop down, and drop down list.
 *  This example shows a dialog with 3 combo boxes, one of each kind.  It is
 *  mostly meant to show the difference in the 3 types to allow an ooDialog
 *  programmer to pick the type to use for a program.
 *
 *  One thing about combo boxes is that a combo box is not a single control.  It
 *  is a mix of several controls, an edit control and a listbox and a button.
 *  Because of this, the normal setColor() method does not work well for the
 *  programmer that wants to set the color of dialog controls.  For the combo
 *  box, ooDialog provides the setFullColor() method which sets the color for
 *  the entire combo box.  This example also demonstrates that method, along
 *  with the removeFullColor() method.
 */

    sd = locate()
    .application~setDefaults('O', sd'comboBoxTypes.h', .false)

    dlg = .ComboBoxTypes~new(sd"comboBoxTypes.rc", IDD_COMBOBOX_TYPES)

    if dlg~initCode = 0 then do
      dlg~execute("SHOWTOP")
    end
    else do
      say "Problem creating the dialog.  Init code:" dlg~initCode
      return 99
    end

    return 0
-- End of entry point.

::requires "ooDialog.cls"

::class 'ComboBoxTypes' subclass RcDialog

-- Normally an init() method for a RcDialog subclass is not used, it is really
-- not needed.  Here though it is used to do some set up stuff.  Everything done
-- here could just as well have been done in initDialog()
::method init
    expose mediumSpringGreen gold midnightBlue fireBrickRed colors animals fruits

    forward class (super) continue

    -- The setFullColor() method takes a COLORREF as input.  Be sure to use the
    -- colorRef() class method of the .Image class to properly construct a
    -- COLORREF from the RGB values.
    --
    -- Create the color values to be used by the setFullColor()
    mediumSpringGreen =  .Image~colorRef(0, 238, 118)
    gold              =  .Image~colorRef(255, 215, 0)
    midnightBlue      =  .Image~colorRef(47, 47, 79)
    fireBrickRed      =  .Image~colorRef(205, 38, 38)

    -- Connect the button clicks
    self~connectButtonEvent(IDC_PB_DROP, 'CLICKED', onDrop, sync)
    self~connectButtonEvent(IDC_PB_CLOSE, 'CLICKED', onClose, sync)

    self~connectButtonEvent(IDC_RB_NONE, 'CLICKED', onSchemeChange, .true)
    self~connectButtonEvent(IDC_RB_RAINBOW, 'CLICKED', onSchemeChange, .false)
    self~connectButtonEvent(IDC_RB_TEXTONLY, 'CLICKED', onSchemeChange, sync)
    self~connectButtonEvent(IDC_RB_FIREBRICK, 'CLICKED', onSchemeChange, .true)

    -- Create arrays of the items to be inserted into each combo box.
    colors  = .array~of("Green", "Blue", "Yellow", "Orange", "Red", "Pink")
    animals = .array~of("Horse", "Cow", "Dog", "Pig", "Cat", "Sheep")
    fruits  = .array~of("Apple", "Peach", "Pear", "Cherry", "Raspberry", "Banna")


-- Typical initDialog() method.  We just populate the combo boxes and check the
-- radio button that matches how we start up, no colors set for the combo boxes.
::method initDialog
    expose cbSimple cbDropDown cbDropDownList colors animals fruits

    cbSimple = self~getComboBox(IDC_CB_SIMPLE)
    do c over colors
      cbSimple~add(c)
    end
    cbSimple~selectIndex(1)

    cbDropDown = self~getComboBox(IDC_CB_DROPDOWN)
    do a over animals
      cbDropDown~add(a)
    end
    cbDropDown~selectIndex(1)

    cbDropDownList = self~getComboBox(IDC_CB_DROPDOWNLIST)
    do f over fruits
      cbDropDownList~add(f)
    end
    cbDropDownList~selectIndex(1)

    self~newRadioButton(IDC_RB_NONE)~check


-- This is the event handler for the radio buttons that select the color scheme.
-- We just check which radio button was clicked and do the appropriate thing.
-- Note that we can set the color for the combo boxes back to their default by
-- using the removeColor() method.
::method onSchemeChange unguarded
    expose mediumSpringGreen gold midnightBlue fireBrickRed cbSimple cbDropDown cbDropDownList
    use arg id

    select
      when id == .constDir[IDC_RB_NONE] then do
          cbSimple~removeFullColor
          cbDropDown~removeFullColor
          cbDropDownList~removeFullColor
      end

      when id == .constDir[IDC_RB_RAINBOW] then do
          cbSimple~setFullColor(midnightBlue, gold)
          cbDropDown~setFullColor(mediumSpringGreen, fireBrickRed)
          cbDropDownList~setFullColor(fireBrickRed, gold)
      end

      when id == .constDir[IDC_RB_TEXTONLY] then do
          cbSimple~setFullColor( , gold)
          cbDropDown~setFullColor( , mediumSpringGreen)
          cbDropDownList~setFullColor( , fireBrickRed)
      end

      when id == .constDir[IDC_RB_FIREBRICK] then do
          cbSimple~setFullColor(fireBrickRed, gold)
          cbDropDown~setFullColor(fireBrickRed, gold)
          cbDropDownList~setFullColor(fireBrickRed, gold)
      end

      otherwise nop
    end
    -- End select

    self~redraw
    return 0


-- This is the event handler for the Close Up push button.  It is meant to close
-- the opened combo boxes.  The reality is, as soon as a combo box loses the
-- focus it closes up.  The act of pushing the Close button will close the
-- opened combo box with out any further action.
::method onClose
    expose cbDropDown cbDropDownList

    cbDropDown~closeDropDown
    cbDropDownList~closedropDown
    return 0

-- This is the event handler for th Drop Down push button.  It is meant to drop
-- down both combo boxes.  The reality is, as soon as a combo box loses the
-- focus it closes up.  Since only 1 dialog at a time can have the focuse, you
-- can only have one combo box dropped down at any time.  Only the drop down
-- list combo box will remain open after the user pushes the button.
::method onDrop
    expose cbDropDown cbDropDownList

    cbDropDown~openDropDown
    cbDropDownList~openDropDown
    return 0

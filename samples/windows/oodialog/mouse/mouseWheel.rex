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
 *  An example program showing how to work with the mouse wheel.
 */


.application~setDefaults('O', 'mouseWheel.h', .false)

dlg = .MouseDemo~new("mouseWheel.rc", IDD_MOUSE_DLG)

if dlg~initCode <> 0 then do
  msg = "Error initializing the Mouse Wheel Demonstration dialog." || '0d0a0d0a'x || -
        "This example will have to quit."
  j = MessageDialog(msg, 0, "Application Error", "OK", "ERROR")
  return 99
end

dlg~execute("SHOWTOP")
return 0

::requires 'ooDialog.cls'

::class 'MouseDemo' subclass RcDialog

::method initDialog
  expose eData pbOk eCommand process

  eData = self~newEdit(IDC_EDIT_DATA)

  -- Disable the edit control's handling of the mouse wheel message, and invoke
  -- our onEditMouseWheel method.  Note that onEditMouseWheel will only be
  -- invoked when the IDC_EDIT_DATA edit control has the focus.
  eData~ignoreMouseWheel( , onEditMouseWheel)

  -- Connect all mouse wheel events to our method, (the default method name is
  -- is onMouseWheel.)  Note that this method will never be invoked if the
  -- IDC_EDIT_DATA edit control has the focus.
  self~connectMouseWheel

  -- Set up the radio buttons
  self~connectButtonEvent(IDC_RB_ANYWHERE        , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_DIALOG          , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_ON_EDIT_OK      , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_FOCUSED_ANYWHERE, 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_FOCUSED_DIALOG  , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_FOCUSED_EDIT    , 'CLICKED', onRbSelect)

  self~newRadioButton(IDC_RB_ANYWHERE)~check
  process = "ANYWHERE"

  -- Save a reference to the ok push button and the single-line edit control.
  pbOk = self~newPushButton(IDOK)
  eCommand = self~newEdit(IDC_EDIT_COMMAND)


  -- Fill the edit control with data so we can see the scrolling.
  text = ''
  do i = 1 to 300
    text ||= 'Line number' || '09'x || i || '09'x || 'Mouse Wheel Demonstation' || .endOfLine
  end
  eData~setText(text)

::method onMouseWheel unguarded
  use arg delta, state, pos

  return self~maybeScroll(delta, state, pos, .false)


::method onEditMouseWheel unguarded
  use arg delta, state, pos

  return self~maybeScroll(delta, state, pos, .true)


::method maybeScroll private
  expose process eData
  use strict arg delta, state, pos, focused

  -- Eliminate some easy stuff
  if focused, \ process~abbrev("FOCUSED") then return 0

  select
    when process~right(8) == "ANYWHERE" then return self~scroll(delta, state)
    when process~right(6) == "DIALOG" then return self~scrollOnDialog(delta, state, pos)
    when process == "ON_EDIT_OK" then return self~scrollOnControls(delta, state, pos)
    when process == "FOCUSED_EDIT", focused then return self~scrollOnEdit(delta, state, pos)
    otherwise do
      return 0
    end
  end
  -- End select


::method scrollOnDialog private
  use strict arg delta, state, pos

  if pos~inRect(self~windowRect) then return self~scroll(delta, state)

  return 0

::method scrollOnEdit private
  expose eData
  use strict arg delta, state, pos

  if pos~inRect(eData~windowRect) then return self~scroll(delta, state)

  return 0


::method scrollOnControls private
  expose eData pbOk eCommand
  use strict arg delta, state, pos

  if pos~inRect(eData~windowRect) then return self~scroll(delta, state)
  if pos~inRect(eCommand~windowRect) then return self~scroll(delta, state)
  if pos~inRect(pbOk~windowRect) then return self~scroll(delta, state)

  return 0



::method scroll private
  expose eData
  use strict arg delta, state

  if delta < 0 then do
    select
      when state == 'None' then eData~scrollCommand("DOWN")

      when state~wordPos('Control') <> 0, state~wordPos('Shift') <> 0 then do
        -- Scroll 3 pages down.
        eData~scrollCommand('PAGEDOWN', 3)
      end

      when state~wordPos('Control') <> 0 then do
        -- Scroll pages down.
        eData~scrollCommand('PAGEDOWN')
      end

      when state~wordPos('Shift') <> 0 then do
        -- Scroll 3 lines down.
        eData~scrollCommand('DOWN', 3)
      end

      otherwise do
        -- Ignore all others and scroll 1 line.
        eData~scrollCommand("DOWN")
      end
    end
    -- End select
  end
  else do
    -- Same as above, but scroll up.
    select
      when state == 'None' then eData~scrollCommand("UP")

      when state~wordPos('Control') <> 0, state~wordPos('Shift') <> 0 then do
        -- Scroll 3 pages down.
        eData~scrollCommand('PAGEUP', 3)
      end

      when state~wordPos('Control') <> 0 then do
        -- Scroll pages down.
        eData~scrollCommand('PAGEUP')
      end

      when state~wordPos('Shift') <> 0 then do
        -- Scroll 3 lines down.
        eData~scrollCommand('UP', 3)
      end

      otherwise do
        -- Ignore all others and scroll 1 line.
        eData~scrollCommand("UP")
      end
    end
    -- End select
  end

  return 0


::method onRbSelect
  expose process
  use arg info, handle

  id = .DlgUtil~loWord(info)

  select
    when id == .ConstDir[IDC_RB_ANYWHERE        ] then process = "ANYWHERE"
    when id == .ConstDir[IDC_RB_DIALOG          ] then process = "DIALOG"
    when id == .ConstDir[IDC_RB_ON_EDIT_OK      ] then process = "ON_EDIT_OK"
    when id == .ConstDir[IDC_RB_FOCUSED_ANYWHERE] then process = "FOCUSED_ANYWHERE"
    when id == .ConstDir[IDC_RB_FOCUSED_DIALOG  ] then process = "FOCUSED_DIALOG"
    when id == .ConstDir[IDC_RB_FOCUSED_EDIT    ] then process = "FOCUSED_EDIT"
    otherwise
      nop
  end
  -- End select


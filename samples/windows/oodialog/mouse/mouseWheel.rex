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
 *
 *  The ooDialog programmer can capture mouse wheel event by using the
 *  connectMouseWheel() method.  The event handler for the connection will be
 *  invoked each time the user turns the scroll wheel on the mouse.  The event
 *  handler can then use the arguments passed to it to do custom scrolling.
 *
 *  Read throught the code comments to understand how this works.
 *
 *  The dialog for this program contains a large multi-line edit control with a
 *  number of lines in it.  For each turn of the mouse scroll wheel, the text in
 *  the edit control is scrolled 1 line. If the shift key is held down while
 *  turning the mouse scroll wheel, the text is scrolled 3 lines instead of 1.
 *  If the control key is down, the text is scrolled 1 page and if both the
 *  shift and control keys are down, the text is scrolled 3 pages.
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

  self~connectHelp(onHelp)

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
  if process~abbrev("FOCUSED"), \ focused then return 0

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
  .MouseDemoHelp~new('mouseWheel.rc', IDD_HELP)~execute("SHOWTOP")


/** class::MouseDemoHelp
 *
 * This is a very simple dialog subclass whose purpose is to display the help
 * text for the Mouse Wheel Demonstration program.
 *
 * Note that we set up our own handlers for the mouse wheel event for the help
 * text simply to provide more example code.  There is really not much need to
 * do this.  The edit control by default handles the scrolling for the mouse
 * wheel itself.  There is not a compelling reason in this dialog to not simply
 * let the edit control handle the scrolling for us.  (Other than the reason
 * mentioned, to provide more code example.)
 */
::class 'MouseDemoHelp' subclass RcDialog

::method initDialog
  expose newFont e

  -- Connect the mouse wheel event to a method in this dialog.  We don't specify
  -- a method name, we'll just accept the default name of onMouseWheel.
  self~connectMouseWheel

  e = self~newEdit(IDC_HELP_TEXT)

  -- Prevent the edit control from handling the mouse wheel event, but do not
  -- pass in a method name.  This causes the edit control to ignore the mouse
  -- wheel event, to not invoke any method. The event notification will then
  -- be passed on up the parent / child window chain until it reaches our
  -- underlying Windows dialog, where we will intercept it.
  e~ignoreMouseWheel

  -- Create a mono-spaced font for the edit control that displays the help text.
  newFont = self~createFontEx('Courier New', 9)

  -- Set the font of the edit control to our custom font, set the text of the
  -- edit control to our help text.
  e~setFont(newFont)
  e~setText(getHelpText())


/** onMouseWheel
 *
 * This is the event handler for the mouse wheel.  We connected the event to
 * this method through the connectMouseWheel() method in initDialog().
 *
 *
 */
::method onMouseWheel unguarded
  expose e
  use arg delta, state, pos



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
 * This is convenience routine that returns the help text for the Mouse Wheel
 * demonstration program.  The help text is constructed here as a single string.
 *
 * The text could be contained in a file and read in when needed.  To reduce the
 * nubmer of files needed for the program, the text was just typed into the
 * program file.
 */
::routine getHelpText

  txt = -
''                                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'                      The Mouse Wheel Demonstration'                           || .endOfLine || -
'                      ============================='                           || .endOfLine || -
''                                                                              || .endOfLine || -
'   This demonstration program allows you to scroll the lines of text in the'   || .endOfLine || -
'   main edit control using the mouse wheel.'                                   || .endOfLine || -
''                                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   Scrolling'                                                                  || .endOfLine || -
'   ========='                                                                  || .endOfLine || -
''                                                                              || .endOfLine || -
'   When the mouse wheel is rotated one notch, the text is scrolled one line.'  || .endOfLine || -
'   Rotating the mouse wheel away from yourself scrolls the text up, while'     || .endOfLine || -
'   rotating the wheel towards yourself scrolls the text down.  When the text'  || .endOfLine || -
'   is scrolled to line 1, scrolling up stops.  Likewise when the lines of'     || .endOfLine || -
'   text are scrolled all the way to the bottom, scrolling down stops.'         || .endOfLine || -
''                                                                              || .endOfLine || -
'   Holding down the control and or the shift keys while rotating the mouse'    || .endOfLine || -
'   wheel changes the amount of text scrolled by rotating one notch of the'     || .endOfLine || -
'   mouse wheel.  This is summarized in the following table:'                   || .endOfLine || -
''                                                                              || .endOfLine || -
'   Notches       Key        Lines'                                             || .endOfLine || -
'   Rotated       Down       Scrolled'                                          || .endOfLine || -
'   ======================================='                                    || .endOfLine || -
'     1           None          1'                                              || .endOfLine || -
'   ---------------------------------------'                                    || .endOfLine || -
'     1          Shift          3'                                              || .endOfLine || -
'   ---------------------------------------'                                    || .endOfLine || -
'     1          Control      1 page'                                           || .endOfLine || -
'   ---------------------------------------'                                    || .endOfLine || -
'     1          Control'                                                       || .endOfLine || -
'                   &         3 pages'                                          || .endOfLine || -
'                Shift'                                                         || .endOfLine || -
'   ---------------------------------------'                                    || .endOfLine || -
''                                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   Radio Buttons'                                                              || .endOfLine || -
'   ============='                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   The radio buttons at the bottom of the dialog change how the position of'   || .endOfLine || -
'   the mouse pointer affects whether the text is scrolled, or not.'            || .endOfLine || -
''                                                                              || .endOfLine || -
'   Text is never scrolled unless the dialog has the input focus.  This'        || .endOfLine || -
'   essentially means when the dialog is the active window.'                    || .endOfLine || -
''                                                                              || .endOfLine || -
'   The following table assigns a number to each radio button according to'     || .endOfLine || -
'   the text of the radio button.  The next table uses the radio button'        || .endOfLine || -
'   number to describe its function.'                                           || .endOfLine || -
''                                                                              || .endOfLine || -
'   Radio     Radio'                                                            || .endOfLine || -
'   Button    Button'                                                           || .endOfLine || -
'     #        Text'                                                            || .endOfLine || -
'   ========================================================================='  || .endOfLine || -
'     1       Mouse is anywhere'                                                || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     2       Mouse is over dialog'                                             || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     3       Mouse over edit controls / Ok button'                             || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     4       Multi-line edit focused / mouse anywhere'                         || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     5       Multi-line edit focused / mouse over dialog'                      || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     6       Multi-line edit focused / mouse over edit control'                || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
''                                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   Radio     Text is'                                                          || .endOfLine || -
'   Button    scrolled'                                                         || .endOfLine || -
'     #       when:'                                                            || .endOfLine || -
'   ========================================================================='  || .endOfLine || -
'     1       The mouse pointer is anywhere on the screen.  It does not have'   || .endOfLine || -
'             to be over the dialog at all.'                                    || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     2       Only when the mouse pointer is over dialog.  If the pointer is'   || .endOfLine || -
'             moved outside of the dialog, the scrolling stops.'                || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     3       Only when the mouse pointer is directly over one of the two'      || .endOfLine || -
'             edit controls, or directly over the Ok button.'                   || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     4       First the multi-line edit control must have the focus.  If any'   || .endOfLine || -
'             other control has the focus, text is not scrolled.  The mouse'    || .endOfLine || -
'             pointer can be anywhere on the screen.'                           || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     5       The multi-line edit control must have the focus.  The mouse'      || .endOfLine || -
'             pointer must be directly over the dialog.'                        || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
'     6       The multi-line edit control must have the focus.  The mouse'      || .endOfLine || -
'             pointer must be directly over the multi-line edit control.'       || .endOfLine || -
'   -------------------------------------------------------------------------'  || .endOfLine || -
''                                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   Edit Controls'                                                              || .endOfLine || -
'   ============='                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   Large, top, edit control:'                                                  || .endOfLine || -
'   -------------------------'                                                  || .endOfLine || -
'   This contains the text that is scrolled.  You can type into this control'   || .endOfLine || -
'   to add or remove lines.'                                                    || .endOfLine || -
''                                                                              || .endOfLine || -
'   Small, bottom, edit control labeled Command:'                               || .endOfLine || -
'   --------------------------------------------'                               || .endOfLine || -
'   This control serves no purpose, other than to take up real estate in the'   || .endOfLine || -
'   dialog.  You can typ in this control, but it does nothing.'                 || .endOfLine || -
''                                                                              || .endOfLine || -
''                                                                              || .endOfLine || -
'   Push Buttons'                                                               || .endOfLine || -
'   ============'                                                               || .endOfLine || -
''                                                                              || .endOfLine || -
'   Ok button    ->  Closes the dialog'                                         || .endOfLine || -
''                                                                              || .endOfLine || -
'   Help button  ->  Shows this help.'                                          || .endOfLine

  return txt

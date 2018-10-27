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
 *  An example program showing how to work with the mouse wheel.
 *
 *  The ooDialog programmer can capture mouse wheel event by using the
 *  connectEvent() method of the .Mouse class.  The event handler for the
 *  connection will be invoked each time the user turns the scroll wheel on the
 *  mouse.  The event handler can then use the arguments passed to it to do
 *  custom scrolling.
 *
 *  Read throught the code comments to understand how this works.
 *
 *  The dialog for this program contains a large multi-line edit control with a
 *  number of lines in it.  For each turn of the mouse scroll wheel, the text in
 *  the edit control is scrolled 1 line. If the shift key is held down while
 *  turning the mouse scroll wheel, the text is scrolled 3 lines instead of 1.
 *  If the control key is down, the text is scrolled 1 page and if both the
 *  shift and control keys are down, the text is scrolled 3 pages.
 *
 *  The Help button in the dialog displays text explaining the dialog.  Use the
 *  help button to see the expected behavior of the dialog and compare the
 *  behavior with the implementing code in this file.
 */

  -- First we locate the direcotry our source code is in.  This allows us to
  -- build complete path names to our resource files, which in turn allows this
  -- program to be run from anywhwere on the computer.  For instance the program
  -- file can be dragged and dropped on ooDialog.exe and it will run correctly.
  srcDir = locate()

  -- When programmers wish to use symbolic resource IDs in their programs, as
  -- this program does, it is much easier to use the global constant directory,
  -- (.constDir,) rather than the constDir attribute of a dialog.  Using the
  -- .constDir Only is by far the most efficient way to use symbolic IDs.
  --
  -- The use of the global constant directory is controlled by the .Application
  -- object.  The .Application object is also used to set other application-wide
  -- defaults or values.  In this program, we also want to make the default for
  -- automatic data detection to be off.  Both of these requirements can be done
  -- with one method call to the .Application object.  As a convenience, the
  -- setDefaults() method will also populate the .constDir with symbols.
  --
  -- In this invokcation, the application is set to use the .constDir only, "O",
  -- symbols are loaded into the .constDir from the file: ContextMenu.h, and the
  -- default for automatic data detection is set to off (.false.)
.application~setDefaults('O', srcDir'mouseWheel.h', .false)

dlg = .MouseDemo~new(srcDir"mouseWheel.rc", IDD_MOUSE_DLG)

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

/** initDialog()
 *
 * The ooDialog framework automatically invokes the initDialog() method
 * immediately after the underlying Windows dialog is created.  This makes it
 * the correct place to do any initialization that requires the underlying
 * dialog and dialog controls to exist.
 *
 * This is a typical initDialog(), but take note of these points.  The mouse
 * wheel event notification is sent by the operating system to the window with
 * the current input focus.  If the window does not process the message, then
 * the OS sends the message to the parent window of that window.  This continues
 * on up the parent / child window chain until either a window does process the
 * notification or the top of the chain is reached.
 *
 * Multi-line edit controls *do* process the notification and normally handle
 * their own scrolling.  Because of this, we need to get two mouse objects.  One
 * for this dialog and one for the edit control.  We then connect the mouse
 * wheel event in both mouse objects to a method in this dialog.
 */
::method initDialog
  expose eData pbOk eCommand process

  -- Connect the help key event to our onHelp() method.
  self~connectHelp(onHelp)

  eData = self~newEdit(IDC_EDIT_DATA)

  -- Get a mouse object for the edit control and connect the mouse wheel event.
  -- There are several options of how to handle this.  We could, using options
  -- to connectEvent(), either have the edit control's mouse wheel events
  -- connect to the same method as we connect the the dialog's mouse wheel
  -- events to, or have them connect to a separate method.
  --
  -- In this case we choose the later, and specify the onEditMouseWheel()
  -- method.
  --
  -- Note that onEditMouseWheel() method will only be invoked when the
  -- IDC_EDIT_DATA edit control has the focus.
  mouse = .Mouse~new(eData)
  mouse~connectEvent('MOUSEWHEEL', onEditMouseWheel)

  -- Connect all mouse wheel events that reach the dialog to a method in this
  -- dialog.  The default method name is onMouseWheel, so we omit the method
  -- name arugment and name our method in this dialog: 'onMousWheel'.
  --
  -- Note that this onMouseWheel() method will never be invoked if the
  -- IDC_EDIT_DATA edit control has the focus.
  mouse = .Mouse~new(self)
  mouse~connectEvent('MOUSEWHEEL')

  -- Set up the radio buttons, connect a click event on any of them to our
  -- single onRbSelect() method.
  self~connectButtonEvent(IDC_RB_ANYWHERE        , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_DIALOG          , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_ON_EDIT_OK      , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_FOCUSED_ANYWHERE, 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_FOCUSED_DIALOG  , 'CLICKED', onRbSelect)
  self~connectButtonEvent(IDC_RB_FOCUSED_EDIT    , 'CLICKED', onRbSelect)

  -- Set the initial checked state to the 'Mouse is anywhere' radio button.
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


/** onMouseWheel()
 *
 * This is an event handler for the mouse wheel event.  Three arguments are
 * sent to the event handler by the ooDialog framework.
 *
 * @param  state  A list of keywords that indicate the keyboard and mouse button
 *                modifiers.  The list of possible words is exactly:
 *
 *                None Control lButton mButton rButton Shift xButton1 xButton2
 *
 *                The none keyword will always be by itself, otherwise, the list
 *                can consist of one or more of the keywords.
 *
 * @param  pos    A .Point object that represents the position of the mouse
 *                pointer on the screen.  This is in screen co-ordinates, in
 *                pixels.
 *
 * @param  mouse  The mouse object that connected this event.  For our purposes
 *                we don't need the mouse so we just ignore this argument.
 *
 * @param  delta  A whole number that represents the amount the scroll wheel was
 *                turned.  The number will always be a multiple of 120 because
 *                that is what the operating system sends.  If the number is
 *                positive, the user turned the wheel away from herself and
 *                indicates to scroll up.  When negative the user turned the
 *                wheel towards himself, and indicates to scroll down.
 *
 *                At this point in time (c. 2011) the OS sends the event
 *                notification for each notch of the scroll wheel and so delta
 *                will always be 120 or -120.
 *
 * Because of the 2 mouse~connectEvent() calls in initDialog() the method will
 * only be called when the dialog is the active window and the multi-line edit
 * control does *not* have the focus.  We simply delegate to the maybeScroll()
 * method, passing on 3 of the arguments and .false to indicate the edit control
 * does not have the focus.
 */
::method onMouseWheel unguarded
  use arg state, pos, mouse, delta

  return self~maybeScroll(delta, state, pos, .false)


/** onEditMouseWheel
 *
 * This is an event handler for the mouse wheel event.  The arguments are
 * explained in the comment header for onMouseWheel().
 *
 * Because of the 2 mouse~connectEvent() calls in initDialog() this method will
 * *only* be called when the edit control has the focus.
 *
 * We simply delegate to the maybeScroll() method, passing on the 3 arguments
 * and .true to indicate the edit control has the focus.
 */
::method onEditMouseWheel unguarded
  use arg state, pos, mouse, delta

  return self~maybeScroll(delta, state, pos, .true)


/** maybeScroll
 * Based on the arguments, we determine whether to scroll or not.
 *
 * In all cases .true is returned, which is eventually returned by the
 * event handler.  This indicates that the event message was processed.
 */
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
      return .true
    end
  end
  -- End select


/** scrollOnDialog()
 *
 * Determines if the position of the cursor (pos) is over the dialog window.  If
 * it is, we scroll, otherwise we do not scroll.
 *
 * In all cases .true is returned, which is eventually returned by the
 * event handler.  This indicates that the event message was processed.
 */
::method scrollOnDialog private
  use strict arg delta, state, pos

  if pos~inRect(self~windowRect) then return self~scroll(delta, state)

  return .true

/** scrollOnEdit()
 *
 * Determines if the position of the cursor (pos) is over the multi-line edit
 * control.  If it is, we scroll, otherwise we do not scroll.
 *
 * In all cases .true is returned, which is eventually returned by the
 * event handler.  This indicates that the event message was processed.
 */
::method scrollOnEdit private
  expose eData
  use strict arg delta, state, pos

  if pos~inRect(eData~windowRect) then return self~scroll(delta, state)

  return .true


/** scrollOnControls()
 *
 * Determines if the position of the cursor (pos) is over one of the edit
 * controls, or over the Ok button.  If it is, we scroll, otherwise we do not
 * scroll.
 *
 * In all cases .true is returned, which is eventually returned by the
 * event handler.  This indicates that the event message was processed.
 */
::method scrollOnControls private
  expose eData pbOk eCommand
  use strict arg delta, state, pos

  if pos~inRect(eData~windowRect) | pos~inRect(eCommand~windowRect) | -
     pos~inRect(pbOk~windowRect) then do
       return self~scroll(delta, state)
  end

  return .true


/** scroll()
 *
 * This private method is where the scrolling is actually done.  It is called
 * by other methods that have determined the mouse is in the correct place for
 * scrolling to happen.  delta and state are the orignal arguments sent to the
 * mouse wheel event handler.  See onMouseWheel() and onEditMouseWheel() for
 * the explanation of the arguments.
 *
 * Based on the arguments, we have the edit control scroll the appropriate
 * amount.  In all cases we return .true which is eventually returned by the
 * event handler.  This indicates that the event message was processed.
 */
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
        -- Scroll page down.
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
        -- Scroll page down.
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

  return .true


/** onRbSelect()
 *
 * This is the event handler for a button click on one of the radio buttons.
 *
 * With auto radio buttons, (which the radio buttons in this program are,) when
 * a radio button is clicked, it is automatically set to the selected state.
 *
 * Each time a radio button becomes the selected button, we update the process
 * variable.  That way 'process' always reflects which radio button is currently
 * selected.
 *
 * The event handler is sent five arguments.  However the first two are left
 * over from the old IBM ooDialog implementation and we ignore them.  We only
 * need the third 'id' argument, so we ignore the last two args also.
 */
::method onRbSelect
  expose process
  use arg , , id

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
  .MouseDemoHelp~new(.application~srcDir'mouseWheel.rc', IDD_HELP)~execute("SHOWTOP")


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
  expose newFont e visibleLines

  -- Connect the mouse wheel event to a method in this dialog.  We don't specify
  -- a method name, we'll just accept the default name of onMouseWheel.
  .Mouse~new(self)~connectEvent('MOUSEWHEEL')

  e = self~newEdit(IDC_HELP_TEXT)

  -- In this case we have the edit control not process the mouse wheel, but
  -- rather pass the message on to the dialog.  The fourth optional parameter of
  -- SENDTODLG is a keyword that causes the mouse wheel notfication to be
  -- passed on up the parent / child window chain and not sent to the edit
  -- control.
  --
  -- The effect of this is that the edit control never even sees the
  -- notification, allowing our onMouseWheel() method to handle all mouse wheel
  -- notifications, not matter where the mouse is over our dialog.
  .Mouse~new(e)~connectEvent('MOUSEWHEEL', 'NOOP', , "SENDTODLG")

  -- Create a mono-spaced font for the edit control that displays the help text.
  newFont = self~createFontEx('Courier New', 9)

  -- Set the font of the edit control to our custom font, set the text of the
  -- edit control to our help text.
  e~setFont(newFont)
  e~setText(getHelpText())

  -- visibleLines is the number of visible lines in the edit control.  It is
  -- only used if the user scrolls the help text using the mouse wheel.  So, we
  -- use lazy evaluation.  It is only calculated if needed.
  visibleLines = 0


/** onMouseWheel
 *
 * This is the event handler for the mouse wheel event.  We connected the event
 * to this method through the Mouse connectEvent() method in initDialog().
 *
 * The comment header for the onMouseWheel() event handler in the maid dialog
 * explains the arguments in detail.
 *
 * The system-wide wheelScrollLines parameter is used to indicate how much to
 * scroll for 1 notch of a turn of the mouse wheel.  The system default is 3,
 * but the user can set this to what they want.
 *
 * Microsoft says this about the wheel scroll lines parameter: it is the
 * suggested number of lines to scroll when there are no modifier keys; if the
 * value is 0, no scrolling should be done; if the value is greater than the
 * number of visible lines, or its value is WHEEL_PAGESCROLL, it should be
 * interpreted as clicking once in the page down or page up area of the scroll
 * bar.
 */
::method onMouseWheel unguarded
  expose e visibleLines
  use arg state, pos, mouse, delta

  if visibleLines == 0 then visibleLines = self~calcVisibleLines(e)

  if delta > 0 then direction = 'up'
  else direction = 'down'

  scrollLines = .SPI~wheelScrollLines

  -- scrollLines is only meant to indicate the number of lines to scroll when
  -- there are no modifier keys or mouse buttons.
  if state \== 'None' then do
    select
      when state~wordPos("Shift") <> 0, state~wordPos('Control') <> 0 then do
        if direction == 'up' then e~scrollCommand('PAGEUP', 3)
        else e~scrollCommand('PAGEDOWN', 3)
        return .true
      end

      when state~wordPos('Shift') then do
        if direction == 'up' then e~scrollCommand('UP', 3)
        else e~scrollCommand('DOWN', 3)
        return .true
      end

      when state~wordPos('Control') then do
        if direction == 'up' then e~scrollCommand('PAGEUP')
        else e~scrollCommand('PAGEDOWN')
        return .true
      end

      otherwise do
        -- Some other modifier, key or mouse, is active.  There are a lot of
        -- things we could do, but for simplicity we are going to just ignore
        -- this.
        return .false
      end
    end
    -- End select
  end

  -- No modifiers, we use the system scroll lines value to decide how much to
  -- scroll.
  select
    when scrollLines == 0 then do
      -- Indicates to not scroll
    end

    when scrollLines == .SPI~WHEEL_PAGESCROLL then do
      -- This setting indicates to scroll 1 page.
      if direction == 'up' then e~scrollCommand('PAGEUP')
      else e~scrollCommand('PAGEDOWN')
    end

    when scrollLines > visibleLines then do
      -- Scroll 1 page.
      if direction == 'up' then e~scrollCommand('PAGEUP')
      else e~scrollCommand('PAGEDOWN')
    end

    otherwise do
      -- Scroll the number of lines specified by  wheel scroll lines parameter.
      if direction == 'up' then e~scrollCommand('UP', scrollLines)
      else e~scrollCommand('DOWN', scrollLines)
    end
  end
  -- End select

  return .true


/** caclVisibleLines()
 *
 * Calculates the number of visible lines in the edit control.  This is done
 * by gettting the size of the edit control formatting rectangle in pixels and
 * the size of a string in the edit controls in pixels.  The height of the
 * formatting rectangle is then divided by the height of a line of text.
 *
 * When getting the height of a line in the edit control, any string can be used
 * because we only need the height of the string.  The length does not matter.
 *
 * On my first try at this, I used the client rect of the edit control for the
 * calculation.  But, it was off by 1.  The proper way to calculate the height
 * is to use the formatting rectangle.  The edit control does all its drawing
 * within that rectangle.
 */
::method calcVisibleLines private
  use strict arg editCtrl

  fRect = editCtrl~getRect

  height     = fRect~bottom - fRect~top
  lineHeight = editCtrl~getTextSizePX("Any string will do for this")~height

  -- Note that we return a whole number here, not a fraction.  The number is
  -- exactly the number of *whole* lines in the edit control.  Quite often in a
  -- dialog, a multi-line edit control will contain a partial line, (as does
  -- this dialog.)
  return height % lineHeight

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

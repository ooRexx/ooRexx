/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
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
 * ToolTip control example.
 *
 * This example demonstrates some fundemental use of the TooTip control.  It
 * creates a tool tip for each push button in the dialog and tool tips for some
 * rectangular areas in the dialog itself.
 *
 * There is a 'Test' push button in the dialog.  When this button is pushed,
 * several short demonstrations are executed that show some of the additional
 * ToolTip methods at work.  These demonstrations print to the screen using say
 * statements.  Therefore it is best to run this example program from a standard
 * console window so that the output can be seen.
 *
 */

    -- Set this application to use the global .constDir Only.  Then add our
    -- symbolic IDs to the .constDir.
    .application~setDefaults('O', , .false)

    .constDir[IDC_TT_MAIN ]     = 500
    .constDir[IDC_TT_OK]        = 501
    .constDir[IDC_TT_CANCEL]    = 502
    .constDir[IDC_TT_TEST]      = 503
    .constDir[IDTOOL_DLG_RECT1] = 774
    .constDir[IDTOOL_DLG_RECT2] = 775
    .constDir[IDTOOL_DLG_RECT3] = 776
    .constDir[IDTOOL_DLG_RECT4] = 777
    .constDir[IDC_PB_TEST]      = 600

    -- Instantiate our dialog and execute it.
    dlg = .ToolTipDialog~new
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

    return 0


::requires "ooDialog.cls"

::class 'ToolTipDialog' subclass UserDialog


/** init()
 *
 * Usual init() method for a UserDialog subclass.  We initialize the super class
 * and then create the dialog frame to get things started.
 *
 * We also use this method to create an icon image.  This image is used for the
 * balloon ToolTip.
 */
::method init
    expose icon
    forward class (super) continue

    -- Locate our icon file
    srcDir = locate()
    icon = .Image~getImage(srcDir"musicNote.ico", ICON, .size~new(16, 16))

    self~create(30, 30, 257, 123, "ToolTip Example Dialog", "CENTER")


/** defineDialog()
 *
 *  Usual defineDialog() method.  We add 3 push buttons to the dialog template.
 */
::method defineDialog

    self~createPushButton(IDC_PB_TEST, 10, 99, 50, 14, , "Test", onTest)
    self~createPushButton(IDOK, 142, 99, 50, 14, "DEFAULT", "Ok")
    self~createPushButton(IDCANCEL, 197, 99, 50, 14, , "Cancel")


/** initDialog()
 *
 * Usual initDialog() method.
 *
 * However, note that ToolTip controls can not be added to the dialog template.
 * They *have* to be created *after* the underlying dialog exists.  Typically,
 * ToolTips would always be created in the initDialog() method.
 */
::method initDialog unguarded
    expose icon pbTest count ttTest

    -- The main tool tip is set up in a private method, so that the code for it
    -- can be inspected without distraction.
    self~createMainToolTip

    -- Creat a somewhat fancy balloon tool tip for the Ok button.
    ttOk = self~createToolTip(IDC_TT_OK, 'BALLOON CLOSE')
    ttOk~setTitle("Important Message", icon)

    text = 'Push the'.endOfLine'Ok button'.endOfLine'to end this'.endOfLine'foolishness'
    ttOk~addTool(self~newPushButton(IDOK), text)

    -- Create a simple multiline tool tip for the Cancel push button.  The tool
    -- tip is turned into a multiline tool tip by seeting the max tool tip
    -- width.
    ttEsc  = self~createToolTip(IDC_TT_CANCEL)
    ttEsc~addTool(self~newPushButton(IDCANCEL), 'Press Cancel to end the dialog')

    margins = 2 * 4
    s = self~getTextSizePX('Press Cancel')
    ttEsc~setMaxTipWidth(s~width + margins)

    -- Create a tool tip for the Test push button.
    pbTest = self~newPushButton(IDC_PB_TEST)

    ttTest = self~createToolTip(IDC_TT_TEST)
    ttTest~addTool(pbTest)
    self~connectToolTipEvent(IDC_TT_TEST, 'NEEDTEXT')

    count = 0


/** createMainToolTip()
 *
 * This is a private method we use to set up the main ToolTip.  We do the set
 * up here so that the code for the set up can be studied without any other
 * distractions.
 */
::method createMainToolTip private
    expose tt clRect1 clRect2 clRect3 clRect4

    tt = self~createToolTip(IDC_TT_MAIN)

    -- We divide the client area of the dialog into 4 quadrants and then add a
    -- tool for each quandrant to the main tool tip.

    clRect = self~clientRect
    hMidpoint = trunc((clRect~right - clRect~left) / 2) + clRect~left
    vMidpoint = trunc((clRect~bottom - clRect~top) / 2) + clRect~top

    clRect1 = .Rect~new(clRect~left, clRect~top, hMidpoint, vMidpoint)
    clRect2 = .Rect~new(hMidpoint + 1, clRect~top, clRect~right, vMidpoint + 1)
    clRect3 = .Rect~new(clRect~left, vMidpoint + 1, hMidpoint + 1, clRect~bottom)
    clRect4 = .Rect~new(hMidpoint + 1, vMidpoint + 1, clRect~right, clRect~bottom)

    ret = tt~addToolRect(self, IDTOOL_DLG_RECT1, clRect1, 'Over main dialog, top left quadrant', 'TRANSPARENT')
    ret = tt~addToolRect(self, IDTOOL_DLG_RECT2, clRect2, 'Over main dialog, top right quadrant', 'TRANSPARENT')
    ret = tt~addToolRect(self, IDTOOL_DLG_RECT3, clRect3, 'Over main dialog, bottom left quadrant', 'TRANSPARENT')
    ret = tt~addToolRect(self, IDTOOL_DLG_RECT4, clRect4, 'Over main dialog, bottom right quadrant', 'TRANSPARENT')

    -- Connect the SHOW event so that we can position the tool tip text where
    -- we want it.  The SHOW notification is sent right before the tool tip is
    -- going to display its window.
    self~connectToolTipEvent(IDC_TT_MAIN, 'SHOW')


/** onShow()
 *
 * This is the event handler for the ToolTip SHOW event.  It is invoked right
 * before the ToolTip is about to show its display window.
 *
 * One of the uses for this event is to customize the position of the window.
 * To customize the window position, the programmer calculates where he wants
 * the window to be position and then uses the setWindowPos() method to place
 * the window.  When the position is set by the programmer, the event handler
 * needs to return .true to inform the ToolTip not to position the window in its
 * default position.  If .false is returned, then the ToolTip positions its
 * window where it normally would.
 *
 * We use this event handler to place the text for each tool in the main ToolTip
 * at the top left-hand corner of its rectangle.
 */
::method onShow unguarded
    expose clRect1 clRect2 clRect3 clRect4
    use arg toolID, toolTip

    -- Determine the position for the tool about to be displayed:
    select
      when toolID == .constDir[IDTOOL_DLG_RECT1] then pos = .Point~new(clRect1~left, clRect1~top)
      when toolID == .constDir[IDTOOL_DLG_RECT2] then pos = .Point~new(clRect2~left, clRect2~top)
      when toolID == .constDir[IDTOOL_DLG_RECT3] then pos = .Point~new(clRect3~left, clRect3~top)
      when toolID == .constDir[IDTOOL_DLG_RECT4] then pos = .Point~new(clRect4~left, clRect4~top)
      otherwise pos = .Point~new
    end
    -- End select

    -- Position the ToolTip window where we want it:
    self~client2screen(pos)
    toolTip~setWindowPos(TOPMOST, pos, .Size~new(0, 0), "NOACTIVATE NOSIZE NOZORDER")

    -- Return .true so that the ToolTip knows not to position the window itself.
    return .true


/** onNeedText()
 *
 * This is the event handler for the ToolTip NEEDTEXT event.  It is invoked when
 * a ToolTip control needs the text to display in its window.
 *
 * The notification is generated when there is no text assigned to a tool, of if
 * the special string: "TextCallBack" is assigned as the text of a tool.
 *
 * The event handler must return .true or .false.  It .true is returned, that
 * tells the ToolTip to store the returned text and not ask for it again.  If
 * .false is returned, then the ToolTip will ask for the text each time the
 * window is about to be displayed.
 *
 * The text itself is returned in the info object.  This is a .Directory object.
 * The returned text is placed at the TEXT index.
 */
::method onNeedText unguarded
    use arg id, toolTip, info

    info~text = 'Press Test to execute a ToolTip test'
    return .true


/** onTest()
 *
 * This is the event handler for the button CLICK event.  It is invoked when the
 * 'Test' push button is pressed or clicked.
 *
 * We use this to demonstrate some of the methods of the ToolTip class.
 */
::method onTest unguarded
    expose tt ttTest pbTest count

    reply 0
    count += 1

    select
        when count == 1 then do
            do i = 1 to tt~getToolCount
                toolInfo = tt~enumTools(i)
                say 'Tool info  hwnd:    ' toolInfo~rexxHwnd
                say 'Tool info  id:      ' toolInfo~rexxID
                say 'Tool info  text:    ' toolInfo~text
                say 'Tool info  flags:   ' toolInfo~flags
                say 'Tool info  rect:    ' toolInfo~rect
                say 'Tool info  userData:' toolInfo~userData
                say 'Tool info  resource:' toolInfo~resource
                say
            end
        end

        when count == 2 then do
            time = tt~getDelayTime
            say 'Time for autopop:' time
            say

            time = tt~getDelayTime('AUTOPOP')
            say 'Time for autopop:' time
            say

            time = tt~getDelayTime('INITIAL')
            say 'Time for initial:' time
            say

            time = tt~getDelayTime('RESHOW')
            say 'Time for reshow:' time
            say
        end

        when count == 3 then do
            mouse = .Mouse~new(self)
            pos = mouse~getCursorPos
            pbTest~screen2client(pos)

            hitTool = .ToolInfo~forHitTest(pbTest)

            if ttTest~hitTestInfo(hitTool, pos) then do
                say 'Got hit'
                say 'Tool info  hwnd:    ' hitTool~rexxHwnd
                say 'Tool info  id:      ' hitTool~rexxID
                say 'Tool info  text:    ' hitTool~text
                say 'Tool info  flags:   ' hitTool~flags
                say 'Tool info  rect:    ' hitTool~rect
                say 'Tool info  userData:' hitTool~userData
                say 'Tool info  resource:' hitTool~resource
                say
            end
            else do
                say 'NO hit'
                say 'Tool info  hwnd:      ' hitTool~rexxHwnd~hwnd
                say 'Test push button hwnd:' pbTest~hwnd
                say
            end
        end

        otherwise do
            count = 0
            say 'No test for this push.  Going to recycle.'
            say
        end
    end
    -- End select

    return


/* publicRoutines_demo.rex */

/*
Purpose.:   Demonstrate each of the ooDialog Public Routines
Who.....:   Lee Peedin
When....:   August 14, 2007
*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007 Rexx Language Association. All rights reserved.         */
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

/*
This demo incorporates the following "features of ooRexx 3.2
24 September 2007
    .endOfLine is used instead of '0d0a'x
    ScreenSize
    MSSleep
    Variations of TimedMessage
    time('t')
*/

-- Define a path most likely to be common to anyone using this demo - change as necessary
    ooRexxHome = value("REXX_HOME", , 'ENVIRONMENT' )
    if ooRexxHome~length == 0 then
        path = 'C:\Program Files\ooRexx\'
    else
        path = ooRexxHome || '\'

-- Define a couple of variables to use in the code, including a global used for
-- a helper dialog.
    delimiter = '0'x
    .local~helperDlgTitle = "The publicRoutines_Demo Helper Dialog"

-- Provide a menu of different examples - use the built in SingleSelection dialog
    preselect = 1
    do until op = ''
        option.1  = 'Play an audio file 1 time - (synchronously)'
        option.2  = 'Play an audio file 1 time - (asynchronously)'
        option.3  = 'Play an audio file multiple times - (loop)'
        option.4  = 'Stop playing the audio file'
        option.5  = 'Show an InfoDialog'
        option.6  = 'Show an ErrorDialog'
        option.7  = 'Show a multi-line InfoDialog'
        option.8  = 'Show an AskDialog - Yes button as the default'
        option.9  = 'Show an AskDialog - No button as the default'
        option.10 = 'Show a FileNameDialog'
        option.11 = 'Retrieve the handle to a window'
        option.12 = 'Use the .DlgUtil Class To Return Screen Resolution'
        option.13 = 'A Standard Timed Message (5 Seconds)'
        option.14 = 'An unTimed Message'
        option.15 = 'Stop The unTimed Message'
        option.16 = 'An Early Reply Timed Message'
        option.17 = 'Use the .DlgUtil Class To Query System Metrics'

        max = 17
        ssdlg = .SingleSelection~new('Select A Demonstration','Public Routines Demonstration',option.,preselect,,max%2+1)
        op = ssdlg~execute
        if op \= '' then
            do
                preselect = op + 1
                if preselect > max then preselect = 1
                call ('OPTION'op) ssdlg
            end
    end
exit

Option1:
    fileName = path||'samples\oodialog\wav\gotcha.wav'
    wstream = .stream~new(filename)
    if wstream~query('exists') = '' then
        call errorDialog'The expected audio file' filename 'does not exist'
    else
        do
            msg = "When you close this message box, a *.wav file will"||.endOfLine||-
                  "play synchronously.  You will return to the demo progam"||.endOfLine||-
                  "when the *.wav file is ended."
            call InfoDialog msg

            call Play fileName
        end
return
----------------------------------------------------------------------------------------------------------------
Option2:
    fileName = path||'samples\oodialog\wav\gotcha.wav'
    wstream = .stream~new(filename)
    if wstream~query('exists') = '' then
        call errorDialog 'The expected audio file' filename 'does not exist'
    else
        do
            msg = "When you close this message box, a *.wav file will"||.endOfLine||-
                  "play asynchronously.  You will return to the demo progam"||.endOfLine||-
                  "immediately, while the *.wav file continues to play."
            call InfoDialog msg

            call Play fileName,'YES'
        end
return
----------------------------------------------------------------------------------------------------------------
Option3:
    fileName = path||'samples\oodialog\wav\gotcha.wav'
    wstream = .stream~new(filename)
    if wstream~query('exists') = '' then
        call errorDialog 'The expected audio file' filename 'does not exist'
    else
        do
            msg = "When you close this message box, a *.wav file will"||.endOfLine||-
                  "play asynchronously in a continuous loop.  You will"||.endOfLine||-
                  "return to the demo progam immediately. The *.wav"||.endOfLine||-
                  "file will continue to play until you stop it using"||.endOfLine||-
                  "the 'Stop playing the audio file' option."
            call InfoDialog msg

            call Play fileName,'LOOP'
        end
return
----------------------------------------------------------------------------------------------------------------
Option4:
    call Play fileName
return
----------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------
Option5:
    call InfoDialog 'This is an InfoDialog - note the icon used & the tone played'
return
----------------------------------------------------------------------------------------------------------------
Option6:
    call ErrorDialog 'This is an ErrorDialog - note the icon used & the tone played'
return
----------------------------------------------------------------------------------------------------------------
Option7:
    msg = 'This is line1'||.endOfLine||-
          'This is line2'||.endOfLine||-
          'This is line3 - note that the dialog is "stretched" to accomodate the longest line'
    call InfoDialog msg
return
----------------------------------------------------------------------------------------------------------------
Option8:
    rv = AskDialog('Do you like this demonstration? (Yes button is pre-selected)')
    if rv = 1 then
        call InfoDialog 'You selected "Yes" - glad you like it'
    else
        call InfoDialog 'You selected "No" - mind sharing what you think would improve the demonstration?'

return
----------------------------------------------------------------------------------------------------------------
Option9:
    rv = AskDialog('Do you like this demonstration? (No button is pre-selected)','n')
    if rv = 1 then
        call InfoDialog 'You selected "Yes" - glad you like it'
    else
        call InfoDialog 'You selected "No" - you must be hard to please'

return
----------------------------------------------------------------------------------------------------------------
Option10:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'All Files (*.*)'delimiter'*.*'delimiter
    loadorsave    = ''                      -- Load is the default
    title         = ''                      -- See documentation for default
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder

    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call ErrorDialog 'You Did Not Select A File'
    else
        call InfoDialog 'You Selected' a_file

    call InfoDialog 'For additional FileNameDialog examples, see the sample titled "fileNameDialog_demo.rex"'
return
----------------------------------------------------------------------------------------------------------------
Option11:

    call infoDialog 'This program will open a dialog window in the'||.endOfLine||-
                    'upper left corner of your screen named:'||.endOfLine||.endOfLine||-
                    '"'.helperDlgTitle'"'||.endOfLine||.endOfLine||-
                    'and will return its handle.  After finding the window, the'||.endOfLine||-
                    'handle value will be added to the dialog window title.'

    -- The public routine FindWindow locates a window by its title.  Since this
    -- example should run for anyone, and there is no way of guaranteeing what
    -- window might be open on anyone's desktop, the example creates its own
    -- simple dialog window.  That way it ensures that there is a known window
    -- to find.

    aWindow = .SimpleDialog~new()
    if aWindow~initCode = 0 then
        do
            -- Start the dialog concurrently, so this example program can
            -- continue to run.  Note, it takes some finite amount of time for
            -- the OS to create and start up the underlying dialog.  The example
            -- sleeps a short time to give the OS time to get the dialog going.
            aWindow~start("Execute")
            call SysSleep .01

            hWnd = FindWindow(.helperDlgTitle)
            if hWnd = 0 then
                call ErrorDialog 'Window could not be found'
            else
                do
                    -- The window handle can now be used to change the title of
                    -- the helper dialog, which demonstrates that the handle is
                    -- correct.
                    aWindow~SetWindowTitle(hWnd,.helperDlgTitle '- My handle is.:' hWnd)
                    call InfoDialog 'The handle to the "'.helperDlgTitle'" window is:' hWnd
                end

            -- Now close the helper dialog.
            aWindow~cancel
        end
    else
        do
            call ErrorDialog 'Sorry, the dialog creation failed'
        end
return
----------------------------------------------------------------------------------------------------------------
Option12:
    use arg self

    -- Dialog units only have meaning in relation to a specfic dialog because the units are dependent on the
    -- font used by the dialog.  The screenSize() method of the .DlgUtil can accurately calculate the dialog
    -- unit values for any dialog, if passed a reference to the dialog.

    ss = .DlgUtil~screenSize('B', self)
    tab = '09'x
    msg = 'Dialog units are expressed as dialog units' || .endOfLine           ||-
          'of' self 'dialog'                           || .endOfLine~copies(3) || -
          'Width In Dialog Units: '  || tab || ss[1]   || .endOfLine ||-
          'Height In Dialog Units:'  || tab || ss[2]   || .endOfLine ||-
          'Width In Pixels: ' || tab || tab || ss[3]   || .endOfLine ||-
          'Height In Pixels:' || tab || tab || ss[4]
    call InfoDialog msg
return
----------------------------------------------------------------------------------------------------------------
Option13:
    msg = 'This Message Will Remain On The Screen For 5000 milliSeconds'
    ret = timedMessage(msg,'A Standard TimedMessage', 5000)
return
----------------------------------------------------------------------------------------------------------------
Option14:
    msg = "Hey, I'm Back Here - Frozen In Time! This Message Will Remain On The Screen Until The Program Calls For It To Be Stopped"
    tdlg = timedMessage(msg,'An unTimedMessage', -1)
return
----------------------------------------------------------------------------------------------------------------
Option15:
    if symbol('tdlg') = 'VAR' then
        do
            tdlg~stopit
            drop tdlg
        end
    else
        call errorDialog 'There Is Not unTimed Message To Stop'
return
----------------------------------------------------------------------------------------------------------------
Option16:
-- Total time will be the duration of the MSSleep - The TimeMessage will last only half the total duration
--    call time 'r'
    startTime = time()
    startTs = .TimeSpan~new(time('F'))
    msg = 'Processing Occurring - Please Wait - Processing Will Take Longer Than This Message'
    ret = timedMessage(msg,'A TimedMessage Early Reply', 5000, .true)
    ret = MSSleep(10000)
    endTime = time()
    endTS = .TimeSpan~new(time('F'))
    msg = 'Start Time:' startTime '- End Time:' endTime '- Duration:' (endTS - startTS)~string~right(9)~strip('T', '0') 'Seconds'
    call infoDialog msg
return
----------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------
Option17:

  tab = '09'x

  -- There are a large number of system metric values.  You query the value you
  -- are interested in by numeric index.  The GetSystemMetrics Windows API is
  -- documented in the MSDN Library, which Microsoft makes available online.
  -- The numeric values of the possible indexes and their meanings are all
  -- documented there.  Use a Google search of "GetSystemMetrics MSDN Library"
  -- to locate the documentation.

  SM_CMONITORS         = 80
  SM_CMOUSEBUTTONS     = 43
  SM_MOUSEWHEELPRESENT = 19

  countMonitors = .DlgUtil~getSystemMetrics(SM_CMONITORS)
  countMouseButtons = .DlgUtil~getSystemMetrics(SM_CMOUSEBUTTONS)
  haveMouseWheel = .DlgUtil~getSystemMetrics(SM_MOUSEWHEELPRESENT)

  if countMonitors <> -1 then
      l1 = "Attached Monitors:" tab || countMonitors"."      ||.endOfLine
  else
      l1 = "SystemMetrics error."                            ||.endOfLine

  select
    when countMouseButtons == 0 then
      l2 = "Mouse:" tab || tab || "no mouse attached."              ||.endOfLine
    when countMouseButtons > 0 then
      l2 = "Mouse:" tab || tab || countMouseButtons "button mouse." ||.endOfLine
    otherwise
      l2 = "SystemMetrics error."                                   ||.endOfLine
  end
  -- End select

  select
    when countMouseButtons == 0 then
      l3 = "Mouse wheel:" tab || "no mouse attached."        ||.endOfLine
    when haveMouseWheel then
      l3 = "Mouse wheel:" tab || "present."                  ||.endOfLine
    when \ haveMouseWheel then
      l3 = "Mouse wheel:" tab || "none."                     ||.endOfLine
    otherwise
      l3 = "SystemMetrics error."                            ||.endOfLine
  end
  -- End select

  l4 = .endOfLine||.endOfLine||"  ---------------------------  "||.endOfLine||.endOfLine

  SM_CXSIZE    = 30
  SM_CXVSCROLL = 20
  SM_CYHSCROLL =  3
  SM_CYMENU    = 15

  l5 = "Height of single line menu bar in pixels:"  tab .DlgUtil~getSystemMetrics(SM_CYMENU)    || ".   " ||.endOfLine
  l6 = "Width of title bar button in pixels:"       tab .DlgUtil~getSystemMetrics(SM_CXSIZE)    || ".   " ||.endOfLine
  l7 = "Width of vertical scroll bar in pixels:"    tab .DlgUtil~getSystemMetrics(SM_CXVSCROLL) || ".   " ||.endOfLine
  l8 = "Height of horizontal scroll bar in pixels:" tab .DlgUtil~getSystemMetrics(SM_CYHSCROLL) || ".   " ||.endOfLine

  msg = l1 || l2 || l3 || l4 || l5 || l6 || l7 || l8
  j = infoDialog(msg)

return
----------------------------------------------------------------------------------------------------------------

-- Requires directive to use the Public Routines
::requires "ooDialog.cls"

::class SimpleDialog subclass PlainUserDialog public
::method Execute
    self~Create(0,0,290,40, .helperDlgTitle)
    self~execute:super("SHOWTOP")

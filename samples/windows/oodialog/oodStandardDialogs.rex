/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
 * oDialog\Samples\oodStandardDialog.rex   Standard Dialog demonstration.
 *
 * This example pops up a main dialog from which the user can select to display
 * any of the standard dialogs provided by ooDialog.
 *
 * This example also demonstrates using the .application object to set a default
 * application icon.  This icon will then be used for every dialog that does not
 * over-ride the default by explicitly specifying some other application icon.
 * This gives all the dialogs displayed an uniform appearance.
 */

    sd = locate()
    .application~setDefaults('O', sd'rc\oodStandardDialogs.h')
    .application~defaultIcon(sd'bmp\oodStandardDialogs.ico')

    dlg = .StandardDialogs~new(sd"rc\oodStandardDialogs.rc", IDD_STDDLGS)
    dlg~execute("SHOWTOP")

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"
::requires "samplesSetup.rex"

::class 'StandardDialogs' subclass RcDialog

::method initDialog
    self~newRadioButton(IDC_RB_CHECKLIST)~check


::method ok unguarded
    reply 0

    select
        when self~newRadioButton(IDC_RB_CHECKLIST)~checked then self~checkList
        when self~newRadioButton(IDC_RB_INTEGERBOX)~checked then self~integerBox
        when self~newRadioButton(IDC_RB_INPUTBOX)~checked then self~inputBox
        when self~newRadioButton(IDC_RB_LISTCHOICE)~checked then self~listChoice
        when self~newRadioButton(IDC_RB_MULTIINPUTBOX)~checked then self~multiInputBox
        when self~newRadioButton(IDC_RB_MULTILISTCHOICE)~checked then self~multiListChoice
        when self~newRadioButton(IDC_RB_PASSWORDBOX)~checked then self~passwordBox
        when self~newRadioButton(IDC_RB_SINGLESELECTION)~checked then self~singleSelection
        when self~newRadioButton(IDC_RB_TIMEDMESSAGE)~checked then self~timedMessage
        otherwise nop
    end
    -- End select


/********************************************************************/
/* How to use "CheckList" class                                     */
/********************************************************************/
::method checkList private
    lst.1  = "Monday" ; lst.2  = "Tuesday" ; lst.3  = "Wednesday" ; lst.4  = "Thursday"
    lst.5  = "Friday" ; lst.6  = "Saturday"; lst.7  = "Sunday"

    do i = 101 to 107
       chk.i = 0
    end
    chk.102 = 1
    chk.104 = 1

    d = .CheckList~new("This is a checklist dialog","Checklist",lst., chk.)

    msg = "Your CheckList data:" || .endOfLine~copies(2)
    if d~execute = 1 then do i = 101 to 107
        a = i-100
        if chk.i = 1 then msg ||= "  ->" lst.a || .endOfLine
    end
    else do
        msg ||= 'The user canceled the dialog.'
    end
    self~displayResult(msg, 'Check List Dialog Results')


/********************************************************************/
/* How to use "InputBox" class                                      */
/********************************************************************/
::method inputBox private
    d = .InputBox~new("This is an input dialog, please enter some data","InputBox")
    msg = "Your InputBox data:" || .endOfLine~copies(2) || d~execute
    self~displayResult(msg, 'Input Box Dialog Results')


/********************************************************************/
/* How to use "IntegerBox" class                                    */
/********************************************************************/
::method integerBox private
    d = .IntegerBox~new("This is an integer dialog, please enter numerical data","IntegerBox")
    msg = "Your IntegerBox data:" || .endOfLine~copies(2) || d~execute
    self~displayResult(msg, 'Integer Box Dialog Results')


/********************************************************************/
/* How to use "ListChoice" class                                    */
/********************************************************************/
::method listChoice private
    lst.1  = "Monday" ; lst.2  = "Tuesday" ; lst.3  = "Wednesday" ; lst.4  = "Thursday"
    lst.5  = "Friday" ; lst.6  = "Saturday"; lst.7  = "Sunday"

    d = .ListChoice~new("This is a listchoice dialog, please select the day","ListChoice",lst., , , 'Thursday')
    s = d~execute
    msg = "Your ListChoice data:" || .endOfLine~copies(2) || s
    self~displayResult(msg, 'List Choice Dialog Results:')


/********************************************************************/
/* How to use "MultiInputBox" class                                 */
/********************************************************************/
::method multiInputBox private
    lab.1 = "First name: "
    lab.2 = "Last name: "
    lab.3 = "Street and City: "
    lab.4 = "Profession:"

    addr.101 = "John" ; addr.102 = "Smith" ; addr.103 = ""
    addr.104 = "Software Engineer"

    d = .MultiInputBox~new("This is a multi input dialog, please enter the address","Your Address",lab., addr.)
    msg = "Your MultiInputBox data:" || .endOfLine~copies(2)
    if d~execute = 1 then do
        msg || = "  ->" d~FirstName     || .endOfLine || "  ->" d~LastName || .endOfLine ||-
                 "  ->" d~StreetandCity || .endOfLine || "  ->" d~Profession
    end
    else do
        msg ||= 'The user canceled the dialog.'
    end
    self~displayResult(msg, 'MultiInput Box Dialog Results:')


/********************************************************************/
/* How to use "MultiListChoice" class                               */
/********************************************************************/
::method multiListChoice private
    lst.1  = "Monday" ; lst.2  = "Tuesday" ; lst.3  = "Wednesday" ; lst.4  = "Thursday"
    lst.5  = "Friday" ; lst.6  = "Saturday"; lst.7  = "Sunday"

    d = .MultiListChoice~new("This is a multiple list choice dialog, please select the days",,
                             "MultipleListChoice",lst.)
    s = d~execute
    msg = "Your MultiListChoice data:" || .endOfLine~copies(2) || "returned indexes:" s || .endOfLine~copies(2)
    if s <> 0 then do while s <> ""
        parse var s res s
        if res <> "" then msg ||= "  ->" lst.res || .endOfLine
    end
    self~displayResult(msg, 'MultiList Choice Dialog Results')


/********************************************************************/
/* How to use "PasswordBox" class                                   */
/********************************************************************/
::method passwordBox private
    d = .PasswordBox~new("Please enter your password","Security")
    msg = "Your PasswordBox data:" || .endOfLine~copies(2) || d~execute
    self~displayResult(msg, 'Password Box Dialog Results')


/********************************************************************/
/* How to use "SingleSelection" class                               */
/********************************************************************/
::method singleSelection private
    mon.1 = "January" ; mon.2 = "February" ; mon.3 = "March"   ; mon.4 = "April"
    mon.5 = "May"      ;mon.6 = "June"     ; mon.7 = "July"    ; mon.8 = "August"
    mon.9 = "September";mon.10= "October"  ; mon.11= "November"; mon.12= "December"

    d = .SingleSelection~new("This is a single selection dialog","Single Selection",mon.,6,,6)
    d~focusControl(106)
    s = d~execute
    msg =  "Your SingleSelection data:" || .endOfLine~copies(2)
    if s > 0 then do
        msg ||= mon.s
    end
    else do
        msg ||= 'The user canceled the dialog.'
    end
    self~displayResult(msg, 'Single Selection Dialog Results')


/********************************************************************/
/* How to use "TimedMessage" class                                  */
/********************************************************************/
::method timedMessage private
    d = .TimedMessage~new("This is a timed message, maybe you will need it sometime", "Hello !", 4000)
    d~execute


::method displayResult private
    use strict arg msg, title

    j = MessageDialog(msg, self~hwnd, title, "OK", "INFORMATION")


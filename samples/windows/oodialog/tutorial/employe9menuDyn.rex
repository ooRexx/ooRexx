/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
 * Name: employe9menuDyn.rex
 * Type: Open Object REXX Script
 *
 * Description:  Adds a menu, created dynamically, to the Employees application.
 */

sd = locate()
dlg = .MyDialogClass~new(sd"employe6.rc", 100)
if dlg~initCode <> 0 then exit
dlg~execute("SHOWTOP")

exit

::requires "ooDialog.cls"

::class MyDialogClass subclass RcDialog

::attribute employees
::attribute empCount
::attribute empCurrent

::method init

    forward class (super) continue
    if self~initCode <> 0 then return self~initCode

    self~employees = .array~new(10)
    self~empCount = 1
    self~empCurrent = 1
    self~connectButtonEvent(10, "CLICKED", "print")   /* connect button 10 with a method */
    self~connectButtonEvent(12, "CLICKED", "add")     /* connect button 12 with a method */
    self~connectUpDownEvent(11, "DELTAPOS", onEmpChange)
    self~connectButtonEvent(13, "CLICKED", "empList")
    self~defineMenu

    return self~initCode


::method defineMenu private
    expose menuBar

    empPopup = .PopupMenu~new(200)
    empPopup~insertItem(202, 202, "&Print")
    empPopup~insertItem(202, 204, "&List", "GRAYED")
    empPopup~insertSeparator(202, 203)
    empPopup~insertItem(202, 201, "&Add")

    ctrlPopup = .PopupMenu~new(210)
    ctrlPopup~insertItem(211, 211, "E&xit")
    ctrlPopup~insertItem(211, 214, "&About")
    ctrlPopup~insertSeparator(211, 213)
    ctrlPopup~insertItem(211, 212, "&Cancel")

    menuBar = .BinaryMenuBar~new(.nil, 300)
    menuBar~insertPopup(200, 200, empPopup, "&Employees")
    menuBar~insertPopup(200, 210, ctrlPopup, "&Control")

    menuBar~connectCommandEvent(201, add, self)
    menuBar~connectCommandEvent(202, print, self)
    menuBar~connectCommandEvent(204, empList, self)
    menuBar~connectCommandEvent(211, ok, self)
    menuBar~connectCommandEvent(212, cancel, self)
    menuBar~connectCommandEvent(214, about, self)


::method initDialog
    expose menuBar

    self~city = "New York"
    self~male = 1
    self~female = 0
    self~addComboEntry(22, "Munich")
    self~addComboEntry(22, "New York")
    self~addComboEntry(22, "San Francisco")
    self~addComboEntry(22, "Stuttgart")
    self~addListEntry(23, "Business Manager")
    self~addListEntry(23, "Software Developer")
    self~addListEntry(23, "Broker")
    self~addListEntry(23, "Police Man")
    self~addListEntry(23, "Lawyer")
    self~disableItem(11)
    self~disableItem(13)

    menuBar~attachTo(self, 1)


::method onEmpChange
    use arg curPos, increment

    if increment > 0 then self~empNext
    else self~empPrev

    return .UpDown~deltaPosReply


::method print
    self~getData

    if self~male = 1 then title = "Mr."
    else title = "Ms."
    if self~married = 1 then addition = " (married) "
    else addition = ""

    call infoDialog title self~name addition || "A"x || "City:" self~city || "A"x ||  -
                     "Profession:" self~name

::method add
    expose menuBar

    self~employees[self~empCount] = .directory~new
    self~employees[self~empCount]['NAME'] = self~getControlData(21)
    self~employees[self~empCount]['CITY'] = self~getControlData(22)
    self~employees[self~empCount]['PROFESSION'] = self~getControlData(23)

    if self~getControlData(31) = 1 then sex = 1
    else sex = 2
    self~employees[self~empCount]['SEX'] = sex

    self~employees[self~empCount]['MARRIED'] = self~getControlData(41)
    self~empCount = self~empCount + 1
    self~empCurrent = self~empCount
    self~setControlData(21, "");

    self~newUpDown(11)~setRange(1, self~empCount)
    self~newUpDown(11)~setPosition(self~empCount)

    self~enableItem(11)
    self~enableItem(13)

    menuBar~enable(204)


::method set
    self~setControlData(21, self~employees[self~empCurrent]['NAME'])
    self~setControlData(22, self~employees[self~empCurrent]['CITY'])
    self~setControlData(23, self~employees[self~empCurrent]['PROFESSION'])

    if self~employees[self~empCurrent]['SEX'] = 1 then do
       self~setControlData(31, 1)
       self~setControlData(32, 0)
    end
    else do
       self~setControlData(31, 0)
       self~setControlData(32, 1)
    end

    self~setControlData(41, self~employees[self~empCurrent]['MARRIED'])

::method empPrev
   if self~empCount = 1 then return
   if self~empCurrent > 1 then do
       self~empCurrent = self~empCurrent - 1
       self~newUpDown(11)~setPosition(self~empCount)
       self~set
   end
   else do
       call TimedMessage "You reached the bottom.", "Info", 1000
   end

::method empNext
   if self~empCount = 1 then return
   if self~empCurrent < self~empCount-1 then do
       self~empCurrent = self~empCurrent + 1
       self~newUpDown(11)~setPosition(self~empCount)
       self~set
   end
   else do
       call TimedMessage "You reached the top.", "Info", 1000
   end

::method empList
   lDlg = .EmployeeListClass~new(.application~srcDir"employe6.rc", 101)
   lDlg~parent = self
   lDlg~execute("SHOWTOP")


::method fillList
   use arg subdlg, id

   do i = 1 to self~empCount - 1
       if self~employees[i]['SEX'] = 1 then title = "Mr."
       else title = "Ms."
       addstring = title self~employees[i]['NAME']
       addstring = addstring || "9"x || self~employees[i]['PROFESSION']
       addstring = addstring || "9"x || self~employees[i]['CITY']
       subdlg~addListEntry(id, addstring)
   end


::method About
   call infoDialog "This sample demonstrates ooDialog menus."


::class EmployeeListClass subclass RcDialog

::method parent attribute

::method initDialog
   self~parent~fillList(self, 101)
   self~setListTabulators(101, 98, 198)

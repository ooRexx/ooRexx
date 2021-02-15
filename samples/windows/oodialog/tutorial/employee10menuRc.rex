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
 * Name: employe9menuRc.rex
 * Type: Open Object REXX Script
 *
 * Description:  Adds a menu, obtained from the .rc script, to the Employees
 *               application.  Continues the refinement / enhancement of the
 *               application.
 */

    sd = locate()
    .application~setDefaults('O', sd'employe7.h', .false)

    dlg = .MyDialogClass~new(sd"employe7.rc", IDD_EMPLOYEES7)
    if dlg~initCode <> 0 then return 99
    dlg~execute("SHOWTOP")

return 0


::requires "ooDialog.cls"

::class 'MyDialogClass' subclass RcDialog

::attribute employees
::attribute empCount

::method init
    expose menuBar sd

    forward class (super) continue
    if self~initCode <> 0 then return self~initCode

    self~employees = .array~new(10)
    self~empCount = 0

    self~connectButtonEvent(IDC_PB_PRINT, "CLICKED", "onPrint")
    self~connectButtonEvent(IDC_PB_ADD, "CLICKED", "onAdd")
    self~connectButtonEvent(IDC_PB_LIST, "CLICKED", "onList")
    self~connectButtonEvent(IDC_RB_ADD, "CLICKED", "onAdding")
    self~connectButtonEvent(IDC_RB_BROWSE, "CLICKED", "onBrowsing")

    self~connectEditEvent(IDC_EDIT_NAME, "CHANGE", onNameChange)
    self~connectUpDownEvent(IDC_UPD, "DELTAPOS", onEmpChange)

    sd = .application~srcDir
    menuBar = .ScriptMenuBar~new(sd"employe7.rc", IDM_MENUBAR, 0)
    menuBar~connectCommandEvent(IDM_ADD, "onAdd", self)
    menuBar~connectCommandEvent(IDM_PRINT, "onPrint", self)
    menuBar~connectCommandEvent(IDM_LIST, "onList", self)
    menuBar~connectCommandEvent(IDM_ABOUT, "about", self)

    return self~initCode

::method initDialog
    expose cbCity lbPosition menuBar

    cbCity = self~newComboBox(IDC_CB_CITY)
    cbCity~add("Munich")
    cbCity~add("New York")
    cbCity~add("San Francisco")
    cbCity~add("Stuttgart")
    cbCity~add("San Diego")
    cbCity~add("Tucson")
    cbCity~add("Houston")
    cbCity~add("Los Angeles")

    lbPosition = self~newListBox(IDC_LB_POSITION)
    lbPosition~add("Business Manager")
    lbPosition~add("Engineering Manager")
    lbPosition~add("Software Developer")
    lbPosition~add("Software QA")
    lbPosition~add("Accountant")
    lbPosition~add("Security")
    lbPosition~add("Secretary")
    lbPosition~add("Recptionist")
    lbPosition~add("Lab Manager")
    lbPosition~add("Lawyer")
    lbPosition~add("CEO")

    menuBar~attachTo(self, 1)

    self~setControls
    self~defaultForm


::method onPrint
    expose cbCity lbPosition rbMale chkMarried editName

    title = "Acme Software - Employee:"

    if rbMale~checked then msg = "Mr."
    else msg = "Ms."
    msg ||= editName~getText

    if chkMarried~checked then msg ||= " (married) "
    msg ||= "A"x || "City:" cbCity~selected || "A"x || "Profession:" lbPosition~selected

    j = MessageDialog(msg, self~hwnd, title, , INFORMATION)

::method onAdd
    expose cbCity lbPosition rbMale chkMarried editName pbList upDown rbBrowse

    self~empCount += 1
    self~employees[self~empCount] = .directory~new
    self~employees[self~empCount]['NAME'] = editName~getText
    self~employees[self~empCount]['CITY'] = cbCity~selected
    self~employees[self~empCount]['POSITION'] = lbPosition~selected

    if rbMale~checked then sex = 1
    else sex = 2

    self~employees[self~empCount]['SEX'] = sex
    self~employees[self~empCount]['MARRIED'] = chkMarried~checked

    upDown~setRange(1, self~empCount)
    if self~empCount == 1 then do
      rbBrowse~enable
      upDown~setPosition(1)
    end

    self~defaultForm

::method onNameChange
    expose menuBar editName rbAdd pbAdd pbPrint

    if rbAdd~checked then do
        if editName~getText~strip~length == 0 then do
            pbAdd~disable
            pbPrint~disable
            menuBar~disable(IDM_ADD)
            menuBar~disable(IDM_PRINT)
        end
        else do
            pbAdd~enable
            pbPrint~enable
            menuBar~enable(IDM_ADD)
            menuBar~enable(IDM_PRINT)
        end
    end

::method onEmpChange
    use arg curPos, increment

    self~setEmpRecord(curPos + increment)
    return .UpDown~deltaPosReply

::method onAdding
    self~defaultForm

::method onBrowsing
    expose upDown pbAdd menuBar

    pbAdd~disable
    menuBar~disable(IDM_ADD)
    upDown~enable
    self~setEmpRecord(upDown~getPosition)

::method setEmpRecord
    expose upDown editName cbCity lbPosition rbMale rbFemale chkMarried
    use strict arg emp

    if emp < 1 then return self~noRecord('bottom')
    else if emp > upDown~getRange~max then return self~noRecord('top')

    editName~setText(self~employees[emp]['NAME'])

    cbCity~select(self~employees[emp]['CITY'])
    lbPosition~select(self~employees[emp]['POSITION'])

    if self~employees[emp]['SEX'] = 1 then do
        rbMale~check
        rbFemale~uncheck
    end
    else do
        rbFemale~check
        rbMale~uncheck
    end

    if self~employees[emp]['MARRIED'] then chkMarried~check
    else chkMarried~uncheck


::method onList
   expose sd
   lDlg = .EmployeeListClass~new(sd"employe7.rc", IDD_EMPLOYEE_LIST)
   lDlg~parent = self
   lDlg~execute("SHOWTOP")


::method fillList
    use strict arg list
    do id = 1 to self~empCount
        if self~employees[id]['SEX'] = 1 then title = "Mr."; else title = "Ms."
        addstring = title self~employees[id]['NAME']
        addstring = addstring || "9"x || self~employees[id]['POSITION']
        addstring = addstring || "9"x || self~employees[id]['CITY']
        list~insert(id, addstring)
    end

::method about
    call infoDialog "Sample to demonstrate ooDialog menus."

::method setControls private
    expose rbMale rbFemale rbAdd rbBrowse chkMarried editName pbList pbAdd pbPrint upDown

    pbAdd      = self~newPushButton(IDC_PB_ADD)
    pbList     = self~newPushButton(IDC_PB_LIST)
    pbPrint    = self~newPushButton(IDC_PB_PRINT)
    rbMale     = self~newRadioButton(IDC_RB_MALE)
    rbFemale   = self~newRadioButton(IDC_RB_FEMALE)
    rbAdd      = self~newRadioButton(IDC_RB_ADD)
    rbBrowse   = self~newRadioButton(IDC_RB_BROWSE)
    chkMarried = self~newCheckBox(IDC_CHK_MARRIED)
    editName   = self~newEdit(IDC_EDIT_NAME)
    upDown     = self~newUpDown(IDC_UPD)

::method defaultForm private
    expose menuBar cbCity lbPosition rbMale rbFemale rbAdd rbBrowse chkMarried -
           editName pbAdd pbList pbPrint upDown

    editName~setText("")
    cbCity~select("New York")
    lbPosition~select("Software Developer")

    rbMale~check
    rbFemale~uncheck
    chkMarried~uncheck

    rbAdd~check
    pbAdd~disable
    pbPrint~disable
    menuBar~disable(IDM_ADD)
    menuBar~disable(IDM_PRINT)
    upDown~disable

    editName~assignFocus

    if self~empCount < 1 then do
        pbList~disable
        rbBrowse~disable
        upDown~setRange(0, 0)
        upDown~setPosition(0)
    end
    else do
        pbList~enable
        menuBar~enable(IDM_LIST)
    end

::method noRecord private
    use strict arg direction
    return MessageDialog('At the' direction 'of the records', self~hwnd, -
                         'Employee Records')


::class 'EmployeeListClass' subclass RcDialog

::attribute parent

::method initDialog
    lb = self~newListBox(IDC_LB_EMPLOYEES_LIST)
    self~parent~fillList(lb)
    lb~setTabulators(90, 190)

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2012 Rexx Language Association. All rights reserved.    */
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
 * Name: employee11tab.rex
 * Type: Open Object REXX Script
 *
 * Description:  Example to demonstrate the property sheet dialog class.
 */

  -- Note to self, we do not use data attributes in this app, need explanation.
  .application~setDefaults("O", "employee11tab.h", .false)

  -- Create the dialog pages.
  p1 = .EmployeeAdd~new("employee11tab.rc", IDD_EMPLOYEES_ADD)
  p2 = .EmployeeEdit~new("employee11tab.rc", IDD_EMPLOYEES_EDIT)
  p3 = .EmployeeBrowse~new("employee11tab.rc", IDD_EMPLOYEES_BROWSE)
  p4 = .EmployeeList~new("employee11tab.rc", IDD_EMPLOYEES_LIST)

  pages = .array~of(p1, p2, p3, p4)
  dlg = .AcmeEmployeesDlg~new(pages, "", "Acme Software - Employee Manager Version 10.00.0")

  dlg~execute

  return 0

::requires "ooDialog.cls"


::class 'AcmeEmployeesDlg' subclass PropertySheetDialog

::attribute employees
::attribute empCount
::attribute empIndex

::method init
    forward class (super) continue
    --say 'In' self 'init()'

    -- need to get employees from database
    self~employees = .array~new(10)
    self~empCount  = 0
    self~empIndex  = 0


    --say 'Pages:'
    do d over self~pages
      --say ' ' d
      d~configure(self~employees, self~empCount, self~empIndex)
    end

::method initDialog

    say 'In' self 'initDialog()'

::class 'EmployeeAdd' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex

::method configure
    use arg employees, count

    self~employees = employess
    self~empCount = count

    self~connectEditEvent(IDC_EDIT_NAME_A, "CHANGE", onNameChange)
    say 'In' self 'configure() propSheet?' self~propSheet

::method initDialog
    expose cbCity lbPosition

    say 'In' self 'init() self~propSheet?' self~propSheet
    self~setControls

    cbCity~add("Munich")
    cbCity~add("New York")
    cbCity~add("San Francisco")
    cbCity~add("Stuttgart")
    cbCity~add("San Diego")
    cbCity~add("Tucson")
    cbCity~add("Houston")
    cbCity~add("Las Angles")

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

    self~defaultForm


::method setActive unguarded

  self~defaultForm
  return 0

::method killActive unguarded
  use arg psDlg

  say 'leaving will abandon changes'
  return .true

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
    expose editName
    say 'NEED TO DO SOMETHING HERE'
    /*
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
    */

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
   lDlg = .EmployeeListClass~new("employe7.rc", IDD_EMPLOYEE_LIST)
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
    expose cbCity lbPosition rbMale rbFemale chkMarried chkFullTime editName

    cbCity      = self~newComboBox(IDC_CB_CITY_A)
    lbPosition  = self~newListBox(IDC_LB_POSITION_A)
    rbMale      = self~newRadioButton(IDC_RB_MALE_A)
    rbFemale    = self~newRadioButton(IDC_RB_FEMALE_A)
    chkMarried  = self~newCheckBox(IDC_CHK_MARRIED_A)
    chkFullTime = self~newCheckBox(IDC_CHK_FULLTIME_A)
    editName    = self~newEdit(IDC_EDIT_NAME_A)

::method defaultForm private
    expose cbCity lbPosition rbMale rbFemale chkMarried chkFullTime editName
    say 'In defaultForm()'
    editName~setText("")
    cbCity~select("New York")
    lbPosition~select("Software Developer")

    rbMale~check
    rbFemale~uncheck
    chkMarried~uncheck
    chkFullTime~check

    editName~assignFocus

    /*
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
    */

::method noRecord private
    use strict arg direction
    return MessageDialog('At the' direction 'of the records', self~hwnd, -
                         'Employee Records')



::class 'EmployeeEdit' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex

::method configure
    use arg employees, count

    self~employees = employess
    self~empCount = count

    self~connectEditEvent(IDC_EDIT_NAME_A, "CHANGE", onNameChange)
    say 'In' self 'configure() propSheet?' self~propSheet

::method initDialog
    expose cbCity lbPosition

    self~setControls

    cbCity~add("Munich")
    cbCity~add("New York")
    cbCity~add("San Francisco")
    cbCity~add("Stuttgart")
    cbCity~add("San Diego")
    cbCity~add("Tucson")
    cbCity~add("Houston")
    cbCity~add("Las Angles")

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
    expose cbCity lbPosition rbMale rbFemale chkMarried chkFullTime editName

    cbCity     = self~newComboBox(IDC_CB_CITY_E)
    lbPosition = self~newListBox(IDC_LB_POSITION_E)
    rbMale     = self~newRadioButton(IDC_RB_MALE_E)
    rbFemale   = self~newRadioButton(IDC_RB_FEMALE_E)
    chkMarried = self~newCheckBox(IDC_CHK_MARRIED_E)
    editName   = self~newEdit(IDC_EDIT_NAME_E)

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



::class 'EmployeeBrowse' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex

::method configure
    use arg employees, count, empIndex

    self~employees = employess
    self~empCount  = count
    self~empIndex  = empIndex

    say 'In' self 'configure() propSheet?' self~propSheet

::method initDialog
    expose cbCity lbPosition

    cbCity = self~newComboBox(IDC_CB_CITY_B)
    cbCity~add("Munich")
    cbCity~add("New York")
    cbCity~add("San Francisco")
    cbCity~add("Stuttgart")
    cbCity~add("San Diego")
    cbCity~add("Tucson")
    cbCity~add("Houston")
    cbCity~add("Las Angles")

    lbPosition = self~newListBox(IDC_LB_POSITION_B)
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

    self~setControls


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
   lDlg = .EmployeeListClass~new("employe7.rc", IDD_EMPLOYEE_LIST)
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

::method setControls private
    expose cbCity lbPosition rbMale rbFemale chkMarried chkFullTime editName

    cbCity     = self~newComboBox(IDC_CB_CITY_B)
    lbPosition = self~newListBox(IDC_LB_POSITION_B)
    rbMale     = self~newRadioButton(IDC_RB_MALE_B)
    rbFemale   = self~newRadioButton(IDC_RB_FEMALE_B)
    chkMarried = self~newCheckBox(IDC_CHK_MARRIED_B)
    editName   = self~newEdit(IDC_EDIT_NAME_B)

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



::class 'EmployeeList' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex

::method configure
    use arg employees, count

    self~employees = employess
    self~empCount = count

    say 'In' self 'configure() propSheet?' self~propSheet

::method initDialog
    say 'In' self 'initDialog() need to fill listview'

::method fillList
    use strict arg list

    do id = 1 to self~empCount
        if self~employees[id]['SEX'] = 1 then title = "Mr."; else title = "Ms."
        addstring = title self~employees[id]['NAME']
        addstring = addstring || "9"x || self~employees[id]['POSITION']
        addstring = addstring || "9"x || self~employees[id]['CITY']
        list~insert(id, addstring)
    end


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

  sd = locate()

  -- Note to self, we do not use data attributes in this app, need explanation.
  .application~setDefaults("O", sd"employee11tab.h", .false)

  -- Create the dialog pages.
  p1 = .EmployeeAdd~new(sd"employee11tab.rc", IDD_EMPLOYEES_ADD)
  p2 = .EmployeeEdit~new(sd"employee11tab.rc", IDD_EMPLOYEES_EDIT)
  p3 = .EmployeeBrowse~new(sd"employee11tab.rc", IDD_EMPLOYEES_BROWSE)
  p4 = .EmployeeList~new(sd"employee11tab.rc", IDD_EMPLOYEES_LIST)

  pages = .array~of(p1, p2, p3, p4)
  dlg = .AcmeEmployeesDlg~new(pages, "NOAPPLYNOW", "Acme Software - Employee Manager Version 10.00.0")

  dlg~execute

  return 0

::requires "ooDialog.cls"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  AcmeEmployeesDlg - The property sheet dialog for this application.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'AcmeEmployeesDlg' subclass PropertySheetDialog

::constant  DATABASE    'employee11tab.db'

::attribute employees
::attribute empCount
::attribute empIndex

::method init
    expose dbChanged

    forward class (super) continue

    self~readDataBase

    dbChanged = .false

    if self~employees~items == 0 then self~startPage = 1
    else self~startPage = 4

    do page over self~pages
      page~configure(self~employees, self~empCount, self~empIndex)
    end


::method newEmployeeNotify
    expose dbChanged
    use strict arg emp

    dbChanged = .true

    self~empCount += 1
    self~employees[self~empCount] = emp
    self~empIndex = self~empCount

    self~notifyPages

    listPage = self~pages[4]

    -- If the user has not yet visited the List Employees page, then there is
    -- no list view to update.  Instead, when the user first visits that page,
    -- the list view will be filled with all existing employees.
    if listPage~wasActivated then listPage~newEmployeeNotify


::method editedEmployeeNotify
    expose dbChanged
    use strict arg emp

    dbChanged = .true
    self~employees[self~empIndex] = emp

    listPage = self~pages[4]
    if listPage~wasActivated then listPage~editedEmployeeNotify

    self~setCurSel( , 4)


::method selectedEmployeeNotify
    use strict arg index

    self~empIndex = index
    self~notifyPages


::method maybeDeleteEmployee
    expose dbChanged

    emp = self~employees[self~empIndex]

    pronoun = 'him'
    if \ emp~male then pronoun = 'her'

    title = "Acme Software - Deleting Employee:" emp~name
    msg   = "Deleting" emp~name "will permanently remove" pronoun "from"        || .endOfLine ||           -
            'the database.  No further pay checks can be issued to' emp~name'.' || .endOfLine~copies(2) || -
            "Are you sure you want to continue this action?"

    answer = MessageDialog(msg, self~hwnd, title, 'YESNO', 'QUESTION', 'DEFBUTTON2')
    if answer == self~IDYES then do
        oldIndex = self~empIndex

        self~employees~delete(self~empIndex)
        self~empCount -= 1
        self~empIndex = 1

        listPage = self~pages[4]
        if listPage~wasActivated then listPage~deleteEmployeeNotify(oldIndex)

        self~notifyPages
        dbChanged = .true
    end

::method leaving
    expose dbChanged

    if dbChanged & self~getResult == 'CLOSEDOK' then do
        self~writeDataBase
    end


::method notifyPages private

    do page over self~pages
      page~empCount = self~empCount
      page~empIndex = self~empIndex
    end


::method readDataBase private

    db = self~DATABASE

    fsObj = .stream~new(db)
    if fsObj~query('EXISTS') == '' then do
        self~employees = .array~new(10)
        self~empCount  = 0
        self~empIndex  = 0
    end
    else do
        lines = fsObj~makeArray
        fsObj~close

        emps = .array~new(lines~items)
        do l over lines
            l = l~strip
            if l~length == 0 | l~abbrev("#") | l~abbrev("/*") then iterate

            parse var l name', 'city', 'pos', 'male', 'married', 'fullTime .

            if male == 'M' then male = .true
            else male = .false

            if married == 'M' then married = .true
            else married = .false

            if fullTime == 'F' then fullTime = .true
            else fullTime = .false

            emp = .Employee~new(name, city, pos, male, married, fullTime)
            emps~append(emp)
        end

        self~employees = emps
        self~empCount  = emps~items
        self~empIndex  = 1
    end


::method writeDataBase private

    db = self~DATABASE

    fsObj = .stream~new(db)

    fsObj~open('WRITE REPLACE')

    if fsObj~state == 'READY' then do e over self~employees
        line = e~toCSV
        fsObj~lineout(line)
    end
    else do
      title = "Acme Software - Internal Error Detected"
      msg   = "The database could not be opened to record current changes."
      j = MessageDialog(msg, 0, title, 'OK', 'ERROR')
    end

    fsObj~close



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  EmployeeAdd - A dialog class for a page in the PropertySheetDialog.  This page
                allows the user to add employee records.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'EmployeeAdd' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex
::attribute empView
::attribute empInitial

::method configure
    expose isChanged
    use arg employees, count, index

    self~employees = employess
    self~empCount  = count
    self~empIndex  = index
    self~empInitial = .Employee~new()

    self~connectEditEvent(IDC_EDIT_NAME_A,      "CHANGE",    onChange)
    self~connectComboBoxEvent(IDC_CB_CITY_A,    "SELCHANGE", onChange)
    self~connectListBoxEvent(IDC_LB_POSITION_A, "SELCHANGE", onChange)
    self~connectButtonEvent(IDC_RB_MALE_A,      "CLICKED",   onChange)
    self~connectButtonEvent(IDC_RB_FEMALE_A,    "CLICKED",   onChange)
    self~connectButtonEvent(IDC_CHK_MARRIED_A,  "CLICKED",   onChange)
    self~connectButtonEvent(IDC_CHK_FULLTIME_A, "CLICKED",   onChange)
    isChanged = .false

    self~connectButtonEvent(IDC_PB_PRINT_A, "CLICKED", onPrint)
    self~connectButtonEvent(IDC_PB_ADD_A, "CLICKED", onAdd)

::method initDialog
    expose cbCity lbPosition editName

    self~setControls
    self~empView~fillBoxes

    editName~connectCharEvent(onChar)

    -- If we are the start page, it means there are no employees.  So we put up
    -- a message box informing the user she needs to add employees through this
    -- page.
    if self~propSheet~startPage == 1 then do
        title = "Acme Software - Employee Manager"
        msg   = "The Employee Database is empty." || .endOfLine~copies(2) ||     -
                "Use the 'Add Employees' page to add employees."

        ret = MessageDialog(msg, self~hwnd, title)
    end


::method setActive unguarded

    self~defaultForm
    return 0

::method killActive unguarded
  expose isChanged
  use arg psDlg

  if isChanged then do
      title = "Acme Software - Add Employee"
      msg   = 'The addition of the new employee is not'  || .endOfLine ||           -
              'complete.  Leaving this page will result' || .endOfLine ||           -
              'in all changes being lost.'               || .endOfLine~copies(2) || -
              'To leave this page and abandon changes,'  || .endOfLine ||           -
              'press Ok.  To stay on the page and con-'  || .endOfLine ||           -
              'tinue adding a new employee, press Cancel'

      ret = MessageDialog(msg, self~hwnd, title, "OKCANCEL", "WARNING", 'DEFBUTTON2')
      if ret == self~IDCANCEL then return .false
  end

  return .true


::method onPrint unguarded

    currentEmployee = self~empView~createEmployee
    currentEmployee~print(self)
    return 0


::method onAdd unguarded

    newEmployee = self~empView~createEmployee
    self~propSheet~newEmployeeNotify(newEmployee)

    self~defaultForm
    return 0


::method onChar unguarded
    use arg char, isShift, isCtrl, isAlt, misc, editControl

    if char~d2c == ',' then do
      result = ' '~c2d
      reply result

      msg   = 'Employee names can not contain a comma. You can not type a comma here'
      title = "Unacceptable Character"
      editControl~showBalloon(title, msg, "ERROR")
    end
    else do
      return .true
    end


::method onChange unguarded
    expose isChanged pbPrint pbAdd editName

    if isChanged then do
        if self~empView~sameAs(self~empInitial) then isChanged = .false
    end
    else do
        if \ self~empView~sameAs(self~empInitial) then isChanged = .true
    end

    -- The Add and Print buttons are only enabled when there is some change and
    -- the employee name is not blank.  The status of the buttons *could* change
    -- on any change, but only *will* change under certain cicrcumstances.
    -- Rather than try to track and determine those exact circumstances, we just
    -- check if the buttons should be enabled or not and do that on each change.
    if isChanged, editName~getText~strip \== '' then do
        pbPrint~enable
        pbAdd~enable
    end
    else do
    		pbPrint~disable
        pbAdd~disable
    end

    return 0


::method setControls private
    expose editName cbCity lbPosition rbMale rbFemale chkMarried chkFullTime pbPrint pbAdd

    editName    = self~newEdit(IDC_EDIT_NAME_A)
    cbCity      = self~newComboBox(IDC_CB_CITY_A)
    lbPosition  = self~newListBox(IDC_LB_POSITION_A)
    rbMale      = self~newRadioButton(IDC_RB_MALE_A)
    rbFemale    = self~newRadioButton(IDC_RB_FEMALE_A)
    chkMarried  = self~newCheckBox(IDC_CHK_MARRIED_A)
    chkFullTime = self~newCheckBox(IDC_CHK_FULLTIME_A)

    self~empView = .EmployeeView~new(editName, cbCity, lbPosition, rbMale, rbFemale, chkMarried, chkFullTime)

    pbPrint = self~newPushButton(IDC_PB_PRINT_A)
    pbAdd   = self~newPushButton(IDC_PB_ADD_A)


::method defaultForm private
    expose pbPrint pbAdd

    pbPrint~disable
    pbAdd~disable

    self~empView~displayEmployee(self~empInitial)
    self~empView~editName~assignFocus



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  EmployeeEdit - A dialog class for a page in the PropertySheetDialog.  This
                 page allows the user to edit an existing employee's record
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'EmployeeEdit' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex
::attribute empInitial
::attribute empView

::method configure
    expose isChanged
    use arg employees, count, index

    self~employees = employees
    self~empCount  = count
    self~empIndex  = index

    self~empInitial = .nil

    self~connectEditEvent(IDC_EDIT_NAME_E,      "CHANGE",    onChange)
    self~connectComboBoxEvent(IDC_CB_CITY_E,    "SELCHANGE", onChange)
    self~connectListBoxEvent(IDC_LB_POSITION_E, "SELCHANGE", onChange)
    self~connectButtonEvent(IDC_RB_MALE_E,      "CLICKED",   onChange)
    self~connectButtonEvent(IDC_RB_FEMALE_E,    "CLICKED",   onChange)
    self~connectButtonEvent(IDC_CHK_MARRIED_E,  "CLICKED",   onChange)
    self~connectButtonEvent(IDC_CHK_FULLTIME_E, "CLICKED",   onChange)
    isChanged = .false

    self~connectButtonEvent(IDC_PB_PRINT_E, "CLICKED", onPrint)
    self~connectButtonEvent(IDC_PB_SAVE_E, "CLICKED", onSave)
    self~connectButtonEvent(IDC_PB_RESET_E, "CLICKED", onReset)


::method initDialog
    expose cbCity lbPosition

    self~setControls
    self~empView~fillBoxes


::method setActive unguarded

  self~setEmpRecord
  return 0


::method onPrint unguarded

    if self~empInitial \== .nil then do
        currentEmployee = self~empView~createEmployee
        currentEmployee~print(self)
    end
    return 0

::method onSave unguarded

    editedEmployee = self~empView~createEmployee
    self~propSheet~editedEmployeeNotify(editedEmployee)

    return 0


::method onReset unguarded
    self~setEmpRecord


::method onChange unguarded
    expose isChanged pbPrint pbSave pbReset editName

    if self~empInitial == .nil then return 0

    if isChanged then do
        if self~empView~sameAs(self~empInitial) then isChanged = .false
    end
    else do
        if \ self~empView~sameAs(self~empInitial) then isChanged = .true
    end

    txt = editName~getText~strip

    if txt == '' then do
    		pbPrint~disable
        pbSave~disable
        pbReset~enable
    end
    else if isChanged then do
        pbPrint~enable
        pbSave~enable
        pbReset~enable
    end
    else do
        pbPrint~enable
        pbSave~disable
        pbReset~disable
    end

    return 0


::method setControls private
    expose cbCity lbPosition rbMale rbFemale chkMarried chkFullTime editName pbPrint pbSave pbReset

    cbCity      = self~newComboBox(IDC_CB_CITY_E)
    lbPosition  = self~newListBox(IDC_LB_POSITION_E)
    rbMale      = self~newRadioButton(IDC_RB_MALE_E)
    rbFemale    = self~newRadioButton(IDC_RB_FEMALE_E)
    chkMarried  = self~newCheckBox(IDC_CHK_MARRIED_E)
    chkFullTime = self~newCheckBox(IDC_CHK_FULLTIME_E)
    editName    = self~newEdit(IDC_EDIT_NAME_E)

    self~empView = .EmployeeView~new(editName, cbCity, lbPosition, rbMale, rbFemale, chkMarried, chkFullTime)

    pbReset = self~newPushButton(IDC_PB_RESET_E)
    pbSave  = self~newPushButton(IDC_PB_SAVE_E)
    pbPrint = self~newPushButton(IDC_PB_PRINT_E)


::method setEmpRecord
    expose pbSave pbPrint pbReset isChanged

    if self~empCount < 1 then do
        self~empView~disable
        pbSave~disable
        pbPrint~disable
        pbReset~disable
        return 0
    end

    isChanged = .false
    pbSave~disable
    pbPrint~enable

    self~empInitial = self~employees[self~empIndex]
    self~empView~displayEmployee(self~empInitial)
    self~empView~editName~assignFocus



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  EmployeeBrowse - A dialog class for a page in the PropertySheetDialog.  This
                   page allows the user to scroll through employee records one
                   at a time.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'EmployeeBrowse' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex
::attribute empView

::method configure
    use arg employees, count, empIndex

    self~employees = employees
    self~empCount  = count
    self~empIndex  = empIndex

    self~connectButtonEvent(IDC_PB_PRINT_B, "CLICKED", onPrint)
    self~connectUpDownEvent(IDC_UPD, "DELTAPOS", onEmpChange)


::method initDialog
    expose upDown

    self~setControls
    self~empView~fillBoxes


::method setActive unguarded

    self~updateUpdown
    return 0


::method onPrint unguarded

    if self~empCount > 0 then do
        currentEmployee = self~empView~createEmployee
        currentEmployee~print(self)
    end
    return 0


::method onEmpChange unguarded
    use arg curPos, increment

    self~setEmpRecord(curPos + increment)
    return .UpDown~deltaPosReply


::method setEmpRecord private
    expose upDown
    use strict arg empIndex

    if empIndex < 1 then return self~noRecord('bottom')
    else if empIndex > upDown~getRange~max then return self~noRecord('top')

    emp = self~employees[empIndex]
    self~empView~displayEmployee(emp)

    self~propSheet~selectedEmployeeNotify(empIndex)


::method setControls private
    expose cbCity lbPosition rbMale rbFemale chkMarried chkFullTime editName upDown pbPrint

    cbCity      = self~newComboBox(IDC_CB_CITY_B)
    lbPosition  = self~newListBox(IDC_LB_POSITION_B)
    rbMale      = self~newRadioButton(IDC_RB_MALE_B)
    rbFemale    = self~newRadioButton(IDC_RB_FEMALE_B)
    chkMarried  = self~newCheckBox(IDC_CHK_MARRIED_B)
    chkFullTime = self~newCheckBox(IDC_CHK_FULLTIME_B)
    editName    = self~newEdit(IDC_EDIT_NAME_B)

    self~empView = .EmployeeView~new(editName, cbCity, lbPosition, rbMale, rbFemale, chkMarried, chkFullTime)

    upDown  = self~newUpDown(IDC_UPD)
    pbPrint = self~newPushButton(IDC_PB_PRINT_B)


::method noRecord private
    use strict arg direction
    return MessageDialog('At the' direction 'of the records', self~hwnd, -
                         'Employee Records')


::method updateUpdown private
    expose pbPrint upDown

    if self~empCount < 1 then do
        upDown~setRange(0, 0)
        upDown~setPosition(0)
        upDown~disable
        pbPrint~disable
        self~empView~blank
    end
    else do
        upDown~enable
        upDown~setRange(1, self~empCount)
        upDown~setPosition(self~empIndex)
        pbPrint~enable
        self~empView~displayEmployee(self~employees[self~empIndex])
    end



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  EmployeeList - A dialog class for a page in the PropertySheetDialog.  This
                 page allows the user to look at multiple employee records in
                 one view.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'EmployeeList' subclass RcPSPDialog

::attribute employees
::attribute empCount
::attribute empIndex

::method configure
    expose contextMenu
    use arg employees, count, index

    self~employees = employees
    self~empCount  = count
    self~empIndex  = index

    self~connectListViewEvent(IDC_LV_EMPLOYEES, "SELECTCHANGED", onSelectionChanged)

    mb = .ScriptMenuBar~new(.application~srcDir'employee11tab.rc', IDM_CONTEXT_MENUBAR)
    contextMenu = mb~getPopup(IDM_POP_CONTEXT)

::method newEmployeeNotify
    expose lv

    emp = self~employees[self~empIndex]
    self~addEmployeeRow(lv, emp)

    self~ensureSelection


::method editedEmployeeNotify
    expose lv

    emp   = self~employees[self~empIndex]
    index = self~empIndex - 1

    gender = 'Male'
    if \ emp~male then gender = 'Female'

    mStatus = 'Married'
    if \ emp~married then mStatus = 'Unmarried'

    eStatus = 'Full Time'
    if \ emp~fullTime then eStatus = 'Part Time'

    lv~setItemText(index, 0, emp~name)
    lv~setItemText(index, 1, emp~city)
    lv~setItemText(index, 2, emp~position)
    lv~setItemText(index, 3, gender)
    lv~setItemText(index, 4, mStatus)
    lv~setItemText(index, 5, eStatus)

    self~ensureSelection


::method initDialog
    expose lv contextMenu

    lv = self~newListView(IDC_LV_EMPLOYEES)

    lv~addExtendedStyle('CHECKBOXES GRIDLINES FULLROWSELECT')

    lv~insertColumn(0, "Name", 50)
    lv~insertColumn(1, "City", 40)
    lv~insertColumn(2, "Position", 40)
    lv~insertColumn(3, "Gender", 30)
    lv~insertColumn(4, "Marital Status", 30)
    lv~insertColumn(5, "Employment Classification", 40)

    do i = 1 to self~empCount
      self~addEmployeeRow(lv, self~employees[i])
    end

    self~ensureSelection

    contextMenu~assignTo(self, .true)
    contextMenu~connectContextMenu(onContext, lv~hwnd)


::method onSelectionChanged unguarded
    use arg id, itemIndex, state

    if state == 'SELECTED' then do
      self~propSheet~selectedEmployeeNotify(itemIndex + 1)
    end


::method setActive unguarded
    expose lv

    self~ensureSelection
    return 0


::method onContext
  expose contextMenu lv
  use arg hwnd, x, y

  if x == -1, y == -1 then do
      -- The keyboard was used, not the mouse.  Position the context menu
      -- towards the bottom right corner of the list view.
      rect = lv~windowRect
      x = rect~right - .SM~cxVScroll - 15
      y = rect~bottom - 15

      pos = .Point~new(x, y)
  end
  else do
      pos = .Point~new(x, y)

      p = pos~copy
      lv~screen2client(p)
      index = lv~hitTestInfo(p)

      if index == -1 then do
        contextMenu~disable(.array~of(IDM_EDIT, IDM_DELETE))
        self~ensureSelection
      end
  end

  ret = contextMenu~show(pos)

  return 0

::method browseEmployees unguarded
    self~propSheet~setCurSel( , 3)
    return 0

::method addEmployee unguarded
    self~propSheet~setCurSel( , 1)
    return 0

::method editEmployee unguarded
    self~propSheet~setCurSel( , 2)
    return 0

::method deleteEmployee unguarded
    self~propSheet~maybeDeleteEmployee
    return 0

::method deleteEmployeeNotify unguarded
    expose lv
    use strict arg index

    lv~delete(index - 1)
    self~ensureSelection

    return 0

::method ensureSelection private
    expose lv

    index = self~empIndex - 1
    if index < 0 then index = 0
    lv~select(index)
    lv~focus(index)

    return 0


::method addEmployeeRow private
    use strict arg list, employee

    gender = 'Male'
    if \ employee~male then gender = 'Female'

    mStatus = 'Married'
    if \ employee~married then mStatus = 'Unmarried'

    eStatus = 'Full Time'
    if \ employee~fullTime then eStatus = 'Part Time'

    e = employee
    list~addRow( , , e~name, e~city, e~position, gender, mStatus, eStatus)


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Employee - A class that reflects a single, specific, employee.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'Employee'

::attribute name
::attribute city
::attribute position
::attribute male
::attribute married
::attribute fullTime

::method init
    expose name city position male married fullTime
    use strict arg name = "", city = "New York", position = "Software Developer",     -
                   male = .true, married = .false, fullTime = .true

::method isValid
    expose name
    return name \== ''


::method print
    expose name city position male married fullTime
    use strict arg dlg

    eol = .endOfLine
    tab = '09'x

    if \ self~isValid then do
        title = "Acme Software - Internal Error Detected"
        msg = "Attempting to print invalid employee record." || eol~copies(2)  -
              "Employee record has no assigned name."
        icon = 'ERROR'
    end
    else do
        title = "Acme Software - Employee:"

        if male then msg = "Mr." name
        else msg = "Ms." name

        if married then msg ||= " (married) "

        msg ||= eol

        if fullTime then status = 'Full Time Employee'
        else status = 'Part Time Employee'

        msg ||= eol || "City:"       || tab~copies(2) || city     || eol || -
                       "Profession:" || tab           || position || eol || -
                       'Status:'     || tab~copies(2) || status

        icon = 'INFORMATION'
    end

    j = MessageDialog(msg, dlg~hwnd, title, , icon)


::method toCSV
    expose name city position male married fullTime
    use strict arg

    if male then mf = "M"
    else mf = "F"

    if married then mu = "M"
    else mu = 'U'

    if fullTime then fp = 'F'
    else fp = 'P'

    txt = name', 'city', 'position', 'mf', 'mu', 'fp

    return txt



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  EmployeeView - A class containing the dialog controls used to display an
                 employee in a dialog.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'EmployeeView' public

::attribute editName
::attribute cbCity
::attribute lbPosition
::attribute rbMale
::attribute rbFemale
::attribute chkMarried
::attribute chkFullTime

::method init
  expose editName cbCity lbPosition rbMale rbFemale chkMarried chkFullTime
  use strict arg editName, cbCity, lbPosition, rbMale, rbFemale, chkMarried, chkFullTime

/** fillBoxes()
 * Fills the city combo box and the position list box with the correct items.
 */
::method fillBoxes
    expose cbCity lbPosition
    use strict arg

    cbCity~add("Munich")
    cbCity~add("New York")
    cbCity~add("San Francisco")
    cbCity~add("Stuttgart")
    cbCity~add("San Diego")
    cbCity~add("Tucson")
    cbCity~add("Houston")
    cbCity~add("Los Angeles")

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


/** displayEmployee()
 * Sets the state of the controls to reflect the specified employee
 */
::method displayEmployee
    use strict arg emp

    self~editName~setText(emp~name)
    self~cbCity~select(emp~City)
    self~lbPosition~select(emp~Position)

    if emp~male then do
        self~rbMale~check
        self~rbFemale~uncheck
    end
    else do
        self~rbFemale~check
        self~rbMale~uncheck
    end

    if emp~married then self~chkMarried~check
    else self~chkMarried~uncheck

    if emp~fullTime then self~chkFullTime~check
    else self~chkFullTime~uncheck


/** disable()
 * Disables all the controls and sets the edit control text to the empty string.
 */
::method disable
    use strict arg

    self~editName~setText("")

    self~editName~disable
    self~cbCity~disable
    self~lbPosition~disable

    self~rbMale~disable
    self~rbFemale~disable
    self~chkMarried~disable
    self~chkFullTime~disable


/** blank()
 * Sets all the controls to a 'blank' state.
 */
::method blank
    use strict arg

    self~editName~setText("")
    --self~cbCity~
    self~lbPosition~deselectIndex

    self~rbMale~uncheck
    self~rbFemale~uncheck
    self~chkMarried~uncheck
    self~chkFullTime~uncheck


/** match()
 * Tests if the state of the controls match the specified employee.
 */
::method sameAs
    use strict arg emp

    if self~editName~getText      \== emp~name     then return .false
    if self~cbCity~selected       \== emp~City     then return .false
    if self~lbPosition~selected   \== emp~Position then return .false

    if self~rbMale~checked        & \ emp~male     then return .false
    if \ self~rbMale~checked      &   emp~male     then return .false

    if self~chkMarried~checked    & \ emp~married  then return .false
    if \ self~chkMarried~checked  &   emp~married  then return .false

    if self~chkFullTime~checked   & \ emp~fullTime then return .false
    if \ self~chkFullTime~checked &  emp~fullTime  then return .false

    return .true

/** createEmployee()
 * Instantiates a new Employee object using the current state of the controls,
 * and returns it if it is valid.  If it is not valid, returns .nil
 */
::method createEmployee
    use strict arg

    name     = self~editName~getText~strip
    city     = self~cbCity~selected
    position = self~lbPosition~selected
    male     = self~rbMale~checked
    married  =  self~chkMarried~checked
    fullTime = self~chkFullTime~checked

    emp = .Employee~new(name, city, position, male, married, fullTime)

    if emp~isValid then return emp
    else return .nil



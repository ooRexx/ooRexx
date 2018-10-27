/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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

/**  importList.rex
 *
 */

    sd = locate()
    .application~setDefaults("O", sd"importList.h", .false)

    files = getInitialFiles()
    if files == .nil then return 99

    if files~items == 0 then initDir = 'C:\'
    else initDir = filespec("location", files[1])

    dlg = .ImporterList~new(sd"importList.rc", IDD_IMPORTER)
    if dlg~initCode = 0 then do
        dlg~fileList = files
        dlg~fileListDirectory = initDir
        dlg~Execute("SHOWTOP")
    end

return 0

::requires "ooDialog.cls"

::class 'ImporterList' subclass RcDialog inherit ResizingAdmin

::constant NORMAL_HTEXT         'Review the import file Options, change if necessary, then click the "Process" button or use the "Process Current View" menu item to process the import files and continue working, click the "Ok" button to process the files and quit, or click the "Cancel" button to quit without processing.'
::constant EMPTY_LIST_HTEXT     'Click the "Import" button to poplulate the list view with the import files.'
::constant EMPTY_ONIMPORT_HTEXT 'The import file list is empty.  Reset the import directory and retry the import operation.'
::constant POST_PROCESS_HTEXT   'Double-click on the Import Directory text or use the Reset Dirctory menu item to set up a new import.'

::attribute fileList
::attribute fileListDirectory

::method defineSizing

    self~defaultLeft('STATIONARY', 'LEFT')
    self~defaultRight('STATIONARY', 'RIGHT')
    self~defaultTop('STATIONARY', 'TOP')
    self~defaultBottom('STATIONARY', 'BOTTOM')

    self~controlBottom(IDC_HEADER, 'MYTOP', 'TOP')

    self~controlLeft(IDCANCEL, 'STATIONARY', 'RIGHT')
    self~controlTop(IDCANCEL, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDCANCEL, 'MYLEFT', 'LEFT')
    self~controlBottom(IDCANCEL, 'MYTOP', 'TOP')

    self~controlLeft(IDOK, 'STATIONARY', 'LEFT', IDCANCEL)
    self~controlTop(IDOK, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDOK, 'MYLEFT', 'LEFT')
    self~controlBottom(IDOK, 'MYTOP', 'TOP')

    self~controlLeft(IDC_PB_IMPORT, 'STATIONARY', 'LEFT', IDOK)
    self~controlTop(IDC_PB_IMPORT, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDC_PB_IMPORT, 'MYLEFT', 'LEFT')
    self~controlBottom(IDC_PB_IMPORT, 'MYTOP', 'TOP')

    self~controlLeft(IDC_PB_PROCESS, 'STATIONARY', 'LEFT', IDC_PB_IMPORT)
    self~controlTop(IDC_PB_PROCESS, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDC_PB_PROCESS, 'MYLEFT', 'LEFT')
    self~controlBottom(IDC_PB_PROCESS, 'MYTOP', 'TOP')

    --self~controlLeft(IDC_ST_DIR_LABEL, 'STATIONARY', 'LEFT', IDC_PB_IMPORT)
    self~controlTop(IDC_ST_DIR_LABEL, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDC_ST_DIR_LABEL, 'MYLEFT', 'LEFT')
    self~controlBottom(IDC_ST_DIR_LABEL, 'MYTOP', 'TOP')

    self~controlLeft(IDC_ST_DIRECTORY, 'STATIONARY', 'RIGHT', IDC_ST_DIR_LABEL)
    self~controlTop(IDC_ST_DIRECTORY, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDC_ST_DIRECTORY, 'STATIONARY', 'LEFT', IDC_PB_PROCESS)
    self~controlBottom(IDC_ST_DIRECTORY, 'MYTOP', 'TOP')


    return 0

::method initDialog
    expose hText edit comboBox listView mouse pbProcess pbImport textDir fileListDirectory menuBar

    menuBar = .ScriptMenuBar~new(.application~srcDir"importList.rc", IDM_MENUBAR)
    menuBar~attachTo(self)

    hText    = self~newStatic(IDC_HEADER)
    comboBox = self~newComboBox(IDC_CB)
    listView = self~newListView(IDC_LV)
    edit     = self~newEdit(IDC_EDIT)

    pbProcess = self~newPushButton(IDC_PB_PROCESS)
    pbImport  = self~newPushButton(IDC_PB_IMPORT)
    textDir   = self~newStatic(IDC_ST_DIRECTORY)

    self~connectListViewEvent(IDC_LV, "DBLCLK", onLVDClick, .true)
    self~connectListViewEvent(IDC_LV, "CLICK", onLVSClick, .true)
    self~connectListViewEvent(IDC_LV, "BEGINSCROLL", onLVScroll)
    self~connectListViewEvent(IDC_LV, "ENDSCROLL", onLVScroll)

    self~connectButtonEvent(IDC_PB_PROCESS, 'CLICKED', onProcess)
    self~connectButtonEvent(IDC_PB_IMPORT, 'CLICKED', onImport)
    ret = self~connectStaticEvent(IDC_ST_DIRECTORY, 'DBLCLK', onDirectory)

    menuBar~connectCommandEvent(IDM_PROCESS_VIEW, onProcess)
    menuBar~connectCommandEvent(IDM_IMPORT_DIRECTORY, onImport)
    menuBar~connectCommandEvent(IDM_RESET_DIRECTORY, onDirectory)

    comboBox~setParent(listView)
    comboBox~isGrandchild(onEditGrandChildEvent)

    edit~setParent(listView)
    edit~isGrandchild(onEditGrandChildEvent)

    --comboBox~hide
    --edit~hide

    textDir~setText(fileListDirectory)

    mouse = .Mouse~new(listView)

    self~initListView
    self~onImport


::method onDirectory unguarded
    expose fileListDirectory fileList pbProcess pbImport textDir listView hText menuBar

    newDir = getNewDirectory(fileListDirectory, self)
    if newDir == .nil | newDir == '' then return 0

    fileListDirectory = newDir
    fileList          = .nil
    textDir~setText(fileListDirectory)
    listView~deleteAll

    pbProcess~disable
    menuBar~disable(IDM_PROCESS_VIEW)
    pbImport~enable
    menuBar~enable(IDM_IMPORT_DIRECTORY)

    hText~setText(self~EMPTY_LIST_HTEXT)

::method onImport unguarded
     expose hText listView pbProcess pbImport fileList fileListDirectory menuBar

     if fileList == .nil then do
         fileList = getFileList(fileListDirectory)
         if fileList == .nil then fileList = onErrorGetFiles(fileListDirectory)
         if fileList == .nil then return self~cancel:super
     end

     listView~deleteAll
     call mssleep(500)            -- delay to allow ListView to display before content added

     do i = 1 to fileList~items
         buName = 'Z:' || filespec('PATH', fileList[i]) || filespec('NAME', fileList[i])
         listView~addRow(i, , 'Import', buName, fileList[i])
     end

     if fileList~items == 0 then do
         hText~setText(self~EMPTY_ONIMPORT_HTEXT)
         pbProcess~disable
         menuBar~disable(IDM_PROCESS_VIEW)
     end
     else do
         hText~setText(self~getNormalHText)

         pbImport~disable
         menuBar~disable(IDM_IMPORT_DIRECTORY)
         pbProcess~enable
         menuBar~enable(IDM_PROCESS_VIEW)
     end


::method onProcess unguarded
     expose listView pbProcess pbImport hText menuBar

     buNamesImport  = .array~new
     buNamesReplace = .array~new
     importNames    = .array~new
     replaceNames   = .array~new
     count          = listView~items
     skipCount      = 0

     do i = 0 to count - 1
         verb = listView~itemText(i, 0)
         select
             when verb == 'Skip' then do
                 skipCount += 1
             end
             when verb == 'Import' then do
                 buNamesImport~append(listView~itemText(i, 1))
                 importNames~append(listView~itemText(i, 2))
             end
             when verb == 'Replace' then do
                 buNamesReplace~append(listView~itemText(i, 1))
                 replaceNames~append(listView~itemText(i, 2))
             end
         end
         -- End select
     end

     title = 'Import Manager'
     msg   = 'Projected Import Processing Statistics:'         || .endOfLine~copies(2)
     msg ||= 'Skipping files:' || '09'x  || skipCount          || .endOfLine
     msg ||= 'Importing files:' || '09'x || importNames~items  || .endOfLine
     msg ||= 'Replacing files:' || '09'x || replaceNames~items || .endOfLine~copies(2)
     msg ||= 'Process files?'

     if messageDialog(msg, self~hwnd, title, 'YESNO', 'INFORMATION') == self~IDNO then return 0

     col1width = 0
     col4width = 0
     do i = 1 to buNamesImport~items
         s = listView~getTextSizePx(buNamesImport[i])
         if s~width > col1width then col1width = s~width

         s = listView~getTextSizePx(importNames[i])
         if s~width > col4width then col4width = s~width
     end

     do i = 1 to buNamesReplace~items
         s = listView~getTextSizePx(buNamesReplace[i])
         if s~width > col1width then col1width = s~width

         s = listView~getTextSizePx(replaceNames[i])
         if s~width > col4width then col4width = s~width
     end

    dlg = .ResultList~new("importList.rc", IDD_RESULTS)
    if dlg~initCode = 0 then do
        dlg~buNamesImport  = buNamesImport
        dlg~buNamesReplace = buNamesReplace
        dlg~importNames    = importNames
        dlg~replaceNames   = replaceNames
        dlg~col1width      = col1width
        dlg~col4width      = col4width
        dlg~execute("SHOWTOP")
    end

    listView~deleteAll

    pbImport~disable
    pbProcess~disable
    menuBar~disable(IDM_IMPORT_DIRECTORY)
    menuBar~disable(IDM_PROCESS_VIEW)

    hText~setText(self~POST_PROCESS_HTEXT)

    return 0


::method initListView private
   expose hText listView comboBox cbVisible editVisible

   baksize = 25
   impsize = 25
   optsize = 10 * 8
   impsize = (impsize + 3) * 8
   baksize = (baksize + 3) * 8

   listView~addExtendedStyle("FULLROWSELECT GRIDLINES HEADERDRAGDROP")
   listView~insertColumnPX(0, "Option", optsize, "LEFT")
   listView~insertColumnPX(1, "Backup Filename to Import", impsize, "LEFT")
   listView~insertColumnPX(2, "Import To:", baksize, "LEFT")

   comboBox~add("Import")
   comboBox~add("Replace")
   comboBox~add("Skip")
   comboBox~selectIndex(1)
   cbVisible = .false
   editVisible = .false


::method ok unguarded
    expose pbProcess

    if pbProcess~isEnabled then self~onProcess
    return self~ok:super

-- Used to process double click, and modify the list view if the double click
-- was on column 2
::method onLVDClick unguarded
   expose currentIndex currentColumn comboBox edit cbVisible editVisible
   use arg id, itemIndex, columnIndex, keyState, , lv

   currentIndex  = itemIndex
   currentColumn = columnIndex

   if columnIndex == 2 then do
       text = lv~itemText(itemIndex, columnIndex)

       r = lv~getSubitemRect(itemIndex, columnIndex, "BOUNDS")
       edit~setWindowPos(lv~hwnd, r~left, r~top, r~right - r~left,      -
                         r~bottom - r~top, "SHOWWINDOW NOZORDERCHANGE")

       edit~setText(text)
       edit~assignFocus
       edit~show
       editVisible = .true
   end

   return 0

-- Used to process single click, and modify the list view if the single click
-- was on column 0
::method onLVSClick unguarded
   expose currentIndex currentColumn comboBox edit cbVisible editVisible
   use arg id, itemIndex, columnIndex, keyState, , lv

   currentIndex = itemIndex
   currentColumn = columnIndex

   if editVisible, columnIndex <> 2 then do
       -- This is not needed, lost focus event happens first.
       editVisible = .false
       edit~hide
   end

   if self~goodColumn0(itemIndex, columnIndex, lv) then do
       -- Set the combo box selection to match the current text of the list view
       -- item before it is shown.
       curText = lv~itemText(itemIndex)
       comboBox~select(curText)

       r = lv~getSubitemRect(itemIndex, columnIndex, "LABEL")
       comboBox~setWindowPos(lv~hwnd, r~left, r~top - 3, r~right - r~left, -
                             4 * (r~bottom - r~top), "SHOWWINDOW NOZORDERCHANGE")

       cbVisible = .true
       comboBox~assignFocus
       comboBox~show
   end
   return 0


-- Used to detect when focus is on/off scroll
::method onLVScroll unguarded
    expose comboBox edit cbVisible editVisible
    use arg ctrlID, dx, dy, listView, isBeginScroll

    return self~acceptUserInput('none')

-- The event handler for both the grandchild edit and comob box controls.
-- Invoked when the grandchild receives an enter, esc, or lost focus event.
::method onEditGrandChildEvent unguarded
    expose comboBox edit listView cbVisible editVisible currentIndex currentColumn
    use arg id, keyEvent, p3, p4, p5

    return self~acceptUserInput(keyEvent)

::method acceptUserInput private unguarded
    expose comboBox edit listView cbVisible editVisible currentIndex currentColumn
    use arg keyEvent

    if cbVisible then do
        text = comboBox~selected
        cbVisible = .false
        comboBox~hide
        if keyEvent <> 'escape' then listView~modify(currentIndex, currentColumn, text)
        if keyEvent == 'none' then do
            listView~redrawItems(currentIndex - 1, currentIndex + 1)
            listView~updateWindow
        end
    end
    else if editVisible = .true then do
        text = edit~getText~strip
        editVisible = .false
        edit~hide
        if keyEvent <> 'escape' then listView~modify(currentIndex, currentColumn, text)
    end

    return 0


-- Determines if a reported single click is a valid click for opening the combo
-- box.  Clicks that are below the last item in the view can have column == 0,
-- but the item index will be -1.  In addition, if the list view has more items
-- than fit on a single page, click to the right of the last column can return
-- 0 for the column index.  The only way to detect this situation is to check
-- the mouse postion and use hitTestInfo() to have the list view tell us whern
-- the mouse is.
::method goodColumn0 unguarded private
    expose mouse
    use strict arg index, colIndex, listView

    if colIndex == 0, index <> -1 then do
        pos = mouse~getCursorPos
        listView~screen2client(pos)
        info = .directory~new
        listView~hitTestInfo(pos, info)
        if info~info == 'OnLabel' then return .true
    end

    return .false

::method getNormalHText private

    return 'Review the import file Options, change if necessary, then click '     || -
           'the "Process" button or use the "Process Current View" menu item '    || -
           'to process the import files and continue working, click the "Ok" '    || -
           'button to process the files and quit, or click the "Cancel" button '  || -
           'to quit without processing.  The "Exit" menu item also quits without' || -
           'processing.'


::class 'ResultList' subclass RcDialog inherit ResizingAdmin

::attribute buNamesImport
::attribute buNamesReplace
::attribute importNames
::attribute replaceNames
::attribute col1width
::attribute col4width

::method defineSizing

    self~defaultLeft('STATIONARY', 'LEFT')
    self~defaultRight('STATIONARY', 'RIGHT')
    self~defaultTop('STATIONARY', 'TOP')
    self~defaultBottom('STATIONARY', 'BOTTOM')

    self~controlLeft(IDOK, 'STATIONARY', 'RIGHT')
    self~controlTop(IDOK, 'STATIONARY', 'BOTTOM')
    self~controlRight(IDOK, 'MYLEFT', 'LEFT')
    self~controlBottom(IDOK, 'MYTOP', 'TOP')

    return 0

::method initDialog

    listView = self~newListView(IDC_LV_RESULTS)

    s = listView~getTextSizePx("Importing")
    col0width = s~width

    s = listView~getTextSizePx("Replacing")
    if s~width > col0width then col0widt = s~width

    s = listView~getTextSizePx("-->")
    col2width = s~width

    -- Need some margins
    col0width += 12
    col2width += 12
    self~col1width += 12
    self~col4width += 12

    listView~addExtendedStyle("FULLROWSELECT GRIDLINES HEADERDRAGDROP")

    listView~insertColumnPX(0, "Action", col0width, "LEFT")
    listView~insertColumnPX(1, "Source File", self~col1width, "LEFT")
    listView~insertColumnPX(2, "To", col2width, "CENTER")
    listView~insertColumnPX(3, "Destination File", self~col4width, "LEFT")


    do i = 1 to self~buNamesImport~items
        listView~addRow(i, , 'Importing', self~buNamesImport[i], '-->', self~importNames[i])
    end

    k = i
    do i = 1 to self~buNamesReplace~items
        k += 1
        listView~addRow(k, , 'Replacing', self~buNamesReplace[i], '-->', self~replaceNames[i])
    end



-- Try to get a list of the files in the ooRexx installation to use for the list
::routine getInitialFiles

    initDir = value('REXX_HOME', , 'ENVIRONMENT')
    if initDir == '' then do
        title = 'Importer File Location Error'
        msg   = 'Could not locate the default importer file list'        || .endOfLine || -
                'directory.'                                             || .endOfLine~copies(2) || -
                'Do you wish to pick an alternative directory?'          || .endOfLine~copies(2) || -
                'Yes to select an alternative directory for the initial' || .endOfLine || -
                'file list. No to open the importer with an empty'       || .endOfLine || -
                'file list. Cancel to end the importer application.'

        ret = MessageDialog(msg, 0, title, 'YESNOCANCEL', 'WARNING')
        select
            when ret == .PlainBaseDialog~IDNO then return .array~new
            when ret == .PlainBaseDialog~IDCANCEL then return .nil
            otherwise nop
        end
        -- End select

        banner = 'Normally the importer opens with the default importer files, ' || -
                 'but the default files can not be located.  Select the folder ' || -
                 'to use for the initial import file list.'
        hint   = 'If this dialog is canceled, the importer will open with an empty file list.'

        bff = .BrowseForFolder~new('Locate Importer Files', banner, hint, 'C:\')
        bff~root = 'CSIDL_DRIVES'
        bff~options = bff~options 'NONEWFOLDERBUTTON'

        initDir = bff~getFolder
        if initDir == .nil then return .array~new
    end

    files = getFileList(initDir)
    if files == .nil then files = onErrorGetFiles()

    return files

::routine getFileList
    use strict arg directory

    fileSpec = directory'\*'
    ret = SysFileTree(fileSpec, f., 'FO')
    if ret <> 0 then do
        title = 'Operating System Error'
        msg   = 'Operating system error searching for files' || .endOfLine~copies(2) || -
                'Directory:'  || '09'x~copies(2) || directory   || .endOfLine || -
                'Error code:' || '09'x || ret         || .endOfLine || -
                'Error text:' || '09'x~copies(2) || SysGetErrorText(ret)

        ret = MessageDialog(msg, 0, title, 'OK', 'WARNING')
        return .nil
    end

    files = .array~new(f.0)
    do i = 1 to f.0
        files[i] = f.i
    end

    return files

::routine onErrorGetFiles
    use strict arg initialDir = 'C:\'

    title = 'Importer File Location Error'
    msg   = 'An operating system error occurred trying to get'       || .endOfLine || -
            'the file list.'                                         || .endOfLine~copies(2) || -
            'Do you wish to try again?'                              || .endOfLine~copies(2) || -
            'Abort closes the importer application.'

    ret = MessageDialog(msg, 0, title, 'ABORTRETRYIGNORE', 'ERROR')
    select
        when ret == .PlainBaseDialog~IDIGNORE then return .array~new
        when ret == .PlainBaseDialog~IDABORT then return .nil
        otherwise nop
    end
    -- End select

    banner = 'Select the directory containing the importer files.'
    hint   = 'If this dialog is canceled, the importer will use an empty file list.'

    bff = .BrowseForFolder~new('Locate Importer Files', banner, hint, initialDir)
    bff~root = 'CSIDL_DRIVES'
    bff~options = bff~options 'NONEWFOLDERBUTTON'

    initDir = bff~getFolder
    if initDir == .nil then return .array~new

    files = getFileList(initDir)
    if files == .nil then files = onErrorGetFiles()

    return files


-- Use the BrowseForFolder class to prompted the user for a directory.
::routine getNewDirectory
    use strict arg initialDir = 'C:\', owner = .nil

    banner = 'Select a new import file directory.  The Import File Manager ' || -
             'will display the import files, allowing the import operation ' || -
             'to be configured and then processed.'
    hint   = 'If this dialog is canceled, the import file list will remain unchanged.'

    bff = .BrowseForFolder~new('Locate Importer Files', banner, hint, initialDir)
    bff~root    = 'CSIDL_DRIVES'
    bff~options = bff~options 'NONEWFOLDERBUTTON'
    bff~owner   = owner

    return bff~getFolder




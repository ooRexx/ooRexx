/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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

/** customizeToolBar.rex
 *
 * The main purpose of this example is to show how to make the built-in
 * customization features of the toolbar available to the users of your
 * application.  It also shows how to connect many of the event notifications of
 * the toolbar.
 *
 *
 *
 */

    sd = locate()
    .application~useGlobalConstDir('O')

    dlg = .CustomizableToolBar~new(sd'rc\customizeToolBar.dll', IDD_TBAR, , sd'rc\customizeToolBar.h')
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::class 'CustomizableToolBar' subclass ResDialog

::method init
  expose buttons
  forward class (super) continue

  self~connectButtonEvent(IDB_ARROW_LEFT,      'CLICKED', onArrowLeft)
  self~connectButtonEvent(IDB_ARROW_RIGHT,     'CLICKED', onArrowRight)
  self~connectButtonEvent(IDB_COPY_CLIPBOARD,  'CLICKED', onCopyClipBoard)
  self~connectButtonEvent(IDB_COPY_TO_FOLDER , 'CLICKED', onCopyToFolder)
  self~connectButtonEvent(IDB_CUT_CLIPBOARD,   'CLICKED', onCutClipBoard)
  self~connectButtonEvent(IDB_DELETE         , 'CLICKED', onDelete)
  self~connectButtonEvent(IDB_FAVORITES      , 'CLICKED', onFavorites)
  self~connectButtonEvent(IDB_FOLDER_CLOSED  , 'CLICKED', onFolderClosed)
  self~connectButtonEvent(IDB_MOVE_TO_FOLDER , 'CLICKED', onMoveToFolder)
  self~connectButtonEvent(IDB_PASTE_CLIPBOARD, 'CLICKED', onPasteClipboard)
  self~connectButtonEvent(IDB_SEARCH         , 'CLICKED', onSearch)

  self~connectToolBarEvent(IDC_TOOLBAR, 'BEGINADJUST')
  self~connectToolBarEvent(IDC_TOOLBAR, 'ENDADJUST')
  self~connectToolBarEvent(IDC_TOOLBAR, 'CUSTHELP')
  self~connectToolBarEvent(IDC_TOOLBAR, 'DELETINGBUTTON')
  self~connectToolBarEvent(IDC_TOOLBAR, 'GETBUTTONINFO')
  self~connectToolBarEvent(IDC_TOOLBAR, 'INITCUSTOMIZE')
  self~connectToolBarEvent(IDC_TOOLBAR, 'QUERYDELETE')
  self~connectToolBarEvent(IDC_TOOLBAR, 'QUERYINSERT')
  self~connectToolBarEvent(IDC_TOOLBAR, 'RELEASEDCAPTURE')
  self~connectToolBarEvent(IDC_TOOLBAR, 'TOOLBARCHANGE')

  tbb1  = .TbButton~new(IDB_ARROW_LEFT     , "Previous"             , "BUTTON", "ENABLED", , 1)
  tbb2  = .TbButton~new(IDB_ARROW_RIGHT    , "Next"                 , "BUTTON", "ENABLED", , 2)
  tbb3  = .TbButton~new(IDB_COPY_CLIPBOARD , "Copy to clipboard"    , "BUTTON", "ENABLED", , 3)
  tbb5  = .TbButton~new(IDB_COPY_TO_FOLDER , "Copy to folder"       , "BUTTON", "ENABLED", , 4)
  tbb4  = .TbButton~new(IDB_CUT_CLIPBOARD  , "Cut to clipboard"     , "BUTTON", "ENABLED", , 5)
  tbb6  = .TbButton~new(IDB_DELETE         , "Delete"               , "BUTTON", "ENABLED", , 6)
  tbb7  = .TbButton~new(IDB_FAVORITES      , "Favorites"            , "BUTTON", "ENABLED", , 7)
  tbb8  = .TbButton~new(IDB_FOLDER_CLOSED  , "Close folder"         , "BUTTON", "ENABLED", , 8)
  tbb9  = .TbButton~new(IDB_MOVE_TO_FOLDER , "Move to folder"       , "BUTTON", "ENABLED", , 9)
  tbb10 = .TbButton~new(IDB_PASTE_CLIPBOARD, "Paste from clipboard" , "BUTTON", "ENABLED", , 10)
  tbb11 = .TbButton~new(IDB_SEARCH         , "Search, for something", "BUTTON", "ENABLED", , 11)

  self~storeButtonText
  buttons = .array~of(tbb1, tbb2, tbb3, tbb4, tbb5, tbb6, tbb7, tbb8, tbb9, tbb10, tbb11)


::method initDialog
  expose tb buttons listBox

  tb = self~newToolBar(IDC_TOOLBAR)

  ret = tb~setExtendedStyle('DOUBLEBUFFER MIXEDBUTTONS')
  ret = tb~setBitmapSize(.Size~new(24, 24))

  self~loadImageList(tb)
  self~setupListBox

  -- We only add the first 3 buttons here.
  btns = .array~of(buttons[1], buttons[2], buttons[3])
  ret = tb~addButtons(btns)

  ret = tb~setButtonText("Previous", IDB_ARROW_LEFT)
  ret = tb~setButtonText("Next", IDB_ARROW_RIGHT)
  ret = tb~setButtonText("Copy to clipboard", IDB_COPY_CLIPBOARD)

  -- Now that the tool bar has its buttons, tell it to recalculate its size.
  tb~autoSize

  -- And show the tool bar.
  tb~show
  tb~assignFocus


::method onArrowLeft unguarded
  expose tb listBox
  self~add('Current text:' tb~getButtonTextEx(IDB_ARROW_LEFT))

::method onArrowRight unguarded
  expose tb listBox
  self~add('Current text:' tb~getButtonTextEx(IDB_ARROW_RIGHT))

::method onCopyClipboard unguarded
  expose tb listBox
  self~add('Current text:' tb~getButtonTextEx(IDB_COPY_CLIPBOARD))

::method onCopyToFolder unguarded
  expose tb listBox
  self~add('Current text:' tb~getButtonTextEx(IDB_COPY_TO_FOLDER))


::method onGetButtonInfo unguarded
  expose buttons listBox
  use arg id, index, textLen, tbb, toobar

  self~add('TBN_GETBUTTONINFO  buttonindex:' index)
  if index > buttons~items then return .false

  self~mergeButtons(tbb, buttons[index])
  return .true

::method onQueryDelete unguarded
  expose listBox
  use arg id, index, tbb, toolbar

  self~add('TBN_QUERYDELETE    buttonindex:' index 'cmdID:' tbb~cmdID)
  return .true

::method onQueryInsert unguarded
  expose listBox
  use arg id, index, tbb, toolbar

  self~add('TBN_QUERYINSERT    buttonindex:' index 'cmdID:' tbb~cmdID)
  return .true


::method onReleasedCapture unguarded
  expose listBox
  use arg id, toolBar
  self~add('NM_RELEASEDCAPTURE')

  return 0

::method onBeginAdjust unguarded
  expose listBox

  self~add('TBN_BEGINADJUST')
  return .true

::method onCustHelp unguarded
  expose listBox

  self~add('TBN_CUSTHELP')
  return .true

::method onDeletingButton unguarded
  expose listBox
  use arg id, cmdID, toolbar

  self~add('TNB_DELETINGBUTTON cmdID:' cmdID)
  return 0

::method onInitCustomize unguarded
  expose listBox

  self~add('TBN_INITCUSTOMIZE')
  return .true

::method onEndAdjust unguarded
  expose listBox
  use arg id, tb
  self~add('TBN_ENDADJUST')

  count = tb~buttonCount
  do i = 1 to count
    cmdID = tb~indexToCommand(i)
    if cmdID <> 0, cmdID <> .nil then self~resetText(cmdID)
  end
  return 0

::method onToolBarChange unguarded
  expose buttonTextSet listBox
  use arg id, tb
  self~add('TBN_TOOLBARCHANGE')

  return 0

::method leaving
  expose font imageList
  imageList~release
  self~deleteFont(font)

::method mergeButtons unguarded private
  use strict arg button1, button2

  button1~bitmapID = button2~bitmapID
  button1~cmdID    = button2~cmdID
  button1~itemData = button2~itemData
  button1~state    = button2~state
  button1~style    = button2~style
  button1~text     = button2~text

::method printTbb unguarded private
  use arg tbb

  say 'bitmapID:' tbb~bitmapID
  say 'cmdID:   ' tbb~cmdID
  say 'itemData:' tbb~itemData
  say 'state:   ' tbb~state
  say 'style:   ' tbb~style
  say 'text:    ' tbb~text
  say

::method loadImageList private
  expose imageList
  use strict arg tb

  flags = 'COLOR32 MASK'
  cRef  = .Image~colorRef(255, 0, 255)

  ri = .ResourceImage~new(self)
  imageBMP = ri~getImage(IDB_BUTTONS_IL)

  imageList = .ImageList~create(.Size~new(24, 24), flags, 20, 0)

  index = imageList~addMasked(imageBMP, cRef)
  old   = tb~setImageList(imageList)

  imageBMP~release

::method setupListBox private
  expose listBox font maxItems

  font = self~createFontEx('Courier New')
  size = self~getTextSizePX('A'~copies(50))

  listBox = self~newListBox(IDC_LB_EVENTS)
  listBox~setFont(font)
  listBox~setWidthPX(size~width)

  maxItems = listBox~clientRect~bottom % listBox~itemHeightPX

::method add unguarded private
  expose listBox maxItems
  use strict arg text

  index = listBox~add(text)
  if index + listBox~getFirstVisible >= maxItems then listBox~makeFirstVisible(index - maxItems + 1)

::method storeButtonText private
  expose buttonTextTable

  table = .table~new
  table[.constDir[IDB_ARROW_LEFT]]      = "Previous"
  table[.constDir[IDB_ARROW_RIGHT]]     = "Next"
  table[.constDir[IDB_COPY_CLIPBOARD]]  = "Copy to clipboard"
  table[.constDir[IDB_COPY_TO_FOLDER]]  = "Copy to folder"
  table[.constDir[IDB_CUT_CLIPBOARD]]   = "Cut to clipboard"
  table[.constDir[IDB_DELETE]]          = "Delete"
  table[.constDir[IDB_FAVORITES]]       = "Favorites"
  table[.constDir[IDB_FOLDER_CLOSED]]   = "Close folder"
  table[.constDir[IDB_MOVE_TO_FOLDER]]  = "Move to folder"
  table[.constDir[IDB_PASTE_CLIPBOARD]] = "Paste from clipboard"
  table[.constDir[IDB_SEARCH]]          = "Search, for something"

  buttonTextTable = table

::method resetText unguarded private
  expose tb buttonTextTable
  use arg cmdID

  ret = tb~setButtonText(buttonTextTable[cmdID], cmdID)

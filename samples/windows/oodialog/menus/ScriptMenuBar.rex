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
 * A simple dialog to demonstrate a .ScriptMenu.
 */

  sd = locate()
  .application~useGlobalConstDir("O", sd"ScriptMenuBar.h")

  dlg = .SimpleDialog~new(sd"ScriptMenuBar.rc", IDD_SCRIPTMENUBAR_DLG)
  if dlg~initCode <> 0 then do
    return 99
  end

  dlg~execute("SHOWTOP")

return 0

::requires "ooDialog.cls"

::class 'SimpleDialog' subclass RcDialog

::method initDialog
  expose menuBar lv

  sd = .application~srcDir
  menuBar = .ScriptMenuBar~new(sd"ScriptMenuBar.rc", IDR_MENU_LV, , , .true, self)

  lv = self~newListView(IDC_LV)
  lv~insertColumnPX(0, "First Name", 70)
  lv~insertColumnPX(1, "Last Name", 90)
  lv~insertColumnPX(2, "Profession", 120)

  lv~addRow( , , "Bill", "Gates", "Philanthropist")
  lv~addRow( , , "Steve", "Jobs", "Salesman")
  lv~addRow( , , "Ron", "McCarthy", "Plumber")
  lv~addRow( , , "Greg", "Allen", "Truck Driver")

::method addItemsFromFile
  self~notImplemented("File -> Add Items")

::method 'openList-View'
  expose lv menuBar

  lv~show
  menuBar~check(IDM_OPEN_LIST_VIEW)
  menuBar~disable(IDM_OPEN_LIST_VIEW)

  menuBar~unCheck(IDM_CLOSE_LIST_VIEW)
  menuBar~enable(IDM_CLOSE_LIST_VIEW)

::method 'closeList-View'
  expose lv menuBar

  lv~hide
  menuBar~enable(IDM_OPEN_LIST_VIEW)
  menuBar~unCheck(IDM_OPEN_LIST_VIEW)

  menuBar~check(IDM_CLOSE_LIST_VIEW)
  menuBar~disable(IDM_CLOSE_LIST_VIEW)

::method checkBoxes
  expose lv menuBar

  if menuBar~isChecked(IDM_CHECKBOXES) then do
    lv~removeExtendedStyle("CHECKBOXES")
    menuBar~unCheck(IDM_CHECKBOXES)
  end
  else do
    lv~addExtendedStyle("CHECKBOXES")
    menuBar~check(IDM_CHECKBOXES)
  end


::method gridLines
  expose lv menuBar

  if menuBar~isChecked(IDM_GRID_LINES) then do
    lv~removeExtendedStyle("GRIDLINES")
    menuBar~unCheck(IDM_GRID_LINES)
  end
  else do
    lv~addExtendedStyle("GRIDLINES")
    menuBar~check(IDM_GRID_LINES)
  end


::method fullRowSelect
  expose lv menuBar

  if menuBar~isChecked(IDM_FULL_ROW_SELECT) then do
    lv~removeExtendedStyle("FULLROWSELECT")
    menuBar~unCheck(IDM_FULL_ROW_SELECT)
  end
  else do
    lv~addExtendedStyle("FULLROWSELECT")
    menuBar~check(IDM_FULL_ROW_SELECT)
  end


::method underLineWhenHot
  expose lv menuBar

  if menuBar~isChecked(IDM_UNDERLINE_HOT) then do
    lv~removeExtendedStyle("ONECLICKACTIVATE UNDERLINEHOT")
    menuBar~unCheck(IDM_UNDERLINE_HOT)
  end
  else do
    lv~addExtendedStyle("ONECLICKACTIVATE UNDERLINEHOT")
    menuBar~check(IDM_UNDERLINE_HOT)
  end

::method insertItem
  self~notImplemented("Operartions -> Insert Item")

::method deleteItem
  self~notImplemented("Operartions -> Delete Item")

::method EditItem
  self~notImplemented("Operartions -> Edit Item")

::method capitalizeItem
  self~notImplemented("Operartions -> Capitalize Item")

::method contents
  self~notImplemented("Help -> Contents")

::method index
  self~notImplemented("Help -> Index ...")

::method search
  self~notImplemented("Help -> Search ...")

::method about unguarded

  dlg = .AboutDialog~new(.application~srcDir"ScriptMenuBar.rc", IDD_ABOUT_DIALOG)

  dlg~execute("SHOWTOP", IDI_DLG_DEFAULT)


::method notImplemented private
  use strict arg title

  msg = title "has not been implemented yet."
  ret = MessageDialog(msg, self~hwnd, title, "OK", "INFORMATION")


::class 'AboutDialog' subclass RcDialog

::method initDialog
   expose font

   bitmap = .Image~getImage(.application~srcDir"UserMenuBar.bmp")
   self~newStatic(IDC_ST_BITMAP)~setImage(bitmap)

   font = self~createFontEx("Ariel", 14)
   self~newStatic(IDC_ST_ABOUT)~setFont(font)

::method leaving
   expose font
   self~deleteFont(font)

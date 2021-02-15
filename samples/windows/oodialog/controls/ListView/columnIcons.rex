/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
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

/**
 *  This example shows how to add icons to the invidual columns of a list-view
 *  when it is in report view.
 *
 */

    sd = locate()
    .application~useGlobalConstDir('O', sd"rc\columnIcons.h")

    dlg = .ListViews~new(sd"rc\columnIcons.dll", IDD_DIALOG)
    if dlg~initCode == 0 then do
        dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
    end
    else do
        say 'Dialog failed to initialize, aborting.'
        return 99
    end

    return 0
-- End of entry point.

::requires "ooDialog.cls"

::class 'ListViews' subclass ResDialog

::method initDialog
    expose list itemColumns

    list = self~newListView(IDC_LV_VIEWS)
    self~newRadioButton(IDC_RB_ICON)~check

    itemColumns = .array~of(-1, -1, -1, -1)

    self~connectEvents
    self~setImageLists
    self~populateList(list)


::method onReport
    expose list
    list~setView("REPORT")
    return 0

::method onList
    expose list
    list~setView("LIST")

::method onIcon
    expose list
    list~setView("ICON")

::method onSmallIcon
    expose list
    list~setView("SMALLICON")

::method onColClick unguarded
    expose itemColumns
    use arg id, colIndex, listView

    -- Adjust for 0-based indexes
    i = colIndex + 1

    d = .directory~new
    d~column = colIndex
    d~caseless = .false

    if itemColumns[i] == -1 then do
      d~ascending = .true
      itemColumns[i] = 0
    end
    else if itemColumns[i] == 0 then do
      d~ascending = .false
      itemColumns[i] = 1
    end
    else do
      d~ascending = .true
      itemColumns[i] = 0
    end

    listView~sortItems('InternalListViewSort', d)
    return 0


::method connectEvents private

    self~connectButtonEvent(IDC_RB_REPORT, "CLICKED", onReport, .true)
    self~connectButtonEvent(IDC_RB_LIST, "CLICKED", onList, sync)
    self~connectButtonEvent(IDC_RB_ICON, "CLICKED", onIcon)
    self~connectButtonEvent(IDC_RB_SMALL_ICON, "CLICKED", onSmallIcon, .false)
    self~connectListViewEvent(IDC_LV_VIEWS, "COLUMNCLICK", onColClick, .true)
    self~connectListViewEvent(IDC_LV_VIEWS, "BEGINDRAG", defListDragHandler)


::method setImageLists private
    expose list

    resourceImage = .ResourceImage~new(self)
    smIcons       = resourceImage~getImage(IDB_SMALL_ICONS)
    normalIcons   = resourceImage~getImage(IDB_NORMAL_ICONS)

    flags     = .Image~toId(ILC_COLOR24)
    imageList = .ImageList~create(.Size~new(16), flags, 28, 0)
    imageList~add(smIcons)

    list~setImageList(imageList, SMALL)

    imageList = .ImageList~create(.Size~new(32), flags, 9, 0)
    imageList~add(normalIcons)

    list~setImageList(imageList, NORMAL)


::method populateList private
    use strict arg list

    list~InsertColumnPx(0, "Title", 150)
    list~InsertColumnPx(1, "Name", 75)
    list~InsertColumnPx(2, "Last", 100)
    list~InsertColumnPx(3, "e-mail", 150)

    style = "FULLROWSELECT UNDERLINEHOT ONECLICKACTIVATE SUBITEMIMAGES HEADERDRAGDROP"
    list~addExtendedStyle(style)

    lvItem = .LvItem~new(0, "Business manager", 6)
    lvSub1 = .LvSubItem~new(0, 1, "Tom", 14)
    lvSub2  = .LvSubItem~new(0, 2, "Sawyer", 26)
    lvSub3  = .LvSubItem~new(0, 3, "ts@google.com", 11)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(1, "Software Developer", 1)
    lvSub1 = .LvSubItem~new(1, 1, "Sam", 14)
    lvSub2  = .LvSubItem~new(1, 2, "Frank", 15)
    lvSub3  = .LvSubItem~new(1, 3, "boo@gmail.com", 12)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(2, "Mechanical Engineer", 0)
    lvSub1 = .LvSubItem~new(2, 1, "Tamara", 13)
    lvSub2  = .LvSubItem~new(2, 2, "Ecclestone", 16)
    lvSub3  = .LvSubItem~new(2, 3, "tameccle@yahoo.com", 9)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(3, "Lawyer", 5)
    lvSub1 = .LvSubItem~new(3, 1, "Mary", 13)
    lvSub2  = .LvSubItem~new(3, 2, "Tyler", 17)
    lvSub3  = .LvSubItem~new(3, 3, "fkan@qualcom.com", 10)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(4, "Doctor", 2)
    lvSub1 = .LvSubItem~new(4, 1, "Cienna", 13)
    lvSub2  = .LvSubItem~new(4, 2, "Acer", 18)
    lvSub3  = .LvSubItem~new(4, 3, "ca@sharp.org", 11)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(5, "Clerk", 3)
    lvSub1 = .LvSubItem~new(5, 1, "Harry", 14)
    lvSub2  = .LvSubItem~new(5, 2, "Houdini", 19)
    lvSub3  = .LvSubItem~new(5, 3, "HH@magic.net", 12)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(6, "Nurse", 4)
    lvSub1 = .LvSubItem~new(6, 1, "Mike", 14)
    lvSub2  = .LvSubItem~new(6, 2, "Thompson", 18)
    lvSub3  = .LvSubItem~new(6, 3, "mike@microsoft.com", 10)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(7, "Drywall Finisher", 7)
    lvSub1 = .LvSubItem~new(7, 1, "Larry", 14)
    lvSub2  = .LvSubItem~new(7, 2, "Goodell", 20)
    lvSub3  = .LvSubItem~new(7, 3, "walls49@yahoo.com", 9)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)

    lvItem = .LvItem~new(8, "Biochemist", 8)
    lvSub1 = .LvSubItem~new(8, 1, "Kumar", 14)
    lvSub2  = .LvSubItem~new(8, 2, "Patel", 18)
    lvSub3  = .LvSubItem~new(8, 3, "kpatel@sequenom.com", 10)
    lvFullRow = .LvFullRow~new(lvItem, lvSub1, lvSub2, lvSub3, .true)
    list~addFullRow(lvFullRow)


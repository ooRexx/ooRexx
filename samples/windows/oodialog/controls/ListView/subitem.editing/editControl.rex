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

/**
 *  This example shows how to embedd an edit control in a list-view, to allow
 *  the user to edit the subitems in the list-view.
 *
 *  To activate the editing, the user clicks once on a subitem, then clicks one
 *  more time to activate the editing.  An edit control takes the place of the
 *  subitem in the list-view.
 *
 *  When the user is in the editing mode, hitting enter, escape, or clicking the
 *  mouse any where else on the scren, ends the editing.  If enter is hit, the
 *  changes are accepted, otherwise, the changes are abandoned.
 *
 *  The key to how this works is creating an invisible edit control, which is
 *  then made a child of the list-view.  When editing mode is entered, the edit
 *  control is positioned over the subitem, sized to the size of the subitem,
 *  and made visible.  When editing is over, the edit control is made invisible
 *  again.
 *
 *  The Rexx edit control, after it is made a child of the list-view, can be
 *  used as normal, with one caveat: Since it is a child of the list-view, the
 *  edit control no longer sends its event nofications to the dialog.  They are
 *  sent to the list-view.  This means that connecting the edit control events
 *  will have no effect.
 */

    -- Set the defaults for this application.  Use the global .constDir 'O'nly,
    -- turn automatic data detection off (.false.)  Then we add a few symbols
    -- to the global .constDir:
    .application~setDefaults('O', , .false)
    .constDir[IDC_LISTVIEW] = 200
    .constDir[IDC_EDIT]     = 201

    dlg = .SimpleLV~new
    if dlg~initCode = 0 then do
        dlg~create(30, 30, 325, 200, "In-place Editing List View", "VISIBLE")
        dlg~execute("SHOWTOP")
    end

return 0
-- End of entry point.

::requires "ooDialog.cls"

::class 'SimpleLV' subclass UserDialog

/** defineDialog()
 *
 * Standard defineDialog. We create an edit control, list-view, and ok button in
 * the dialog template.  Note the edit control is created invisible.
 *
 * We add a flag to keep track of whether the edit control is visisble or not.
 * We also connect the events we need to monitor.
 */
::method defineDialog
  expose editVisible

  editVisible = .false

  self~createListView(IDC_LISTVIEW, 10, 20, 305, 145, "REPORT SHOWSELALWAYS")
  self~createEdit(IDC_EDIT, 10, 10, 40, 11, 'AUTHSCROLLH NOTAB HIDDEN')
  self~createPushButton(IDOK, 280, 175, 35, 15, "DEFAULT", "Close")

  self~connectListViewEvent(IDC_LISTVIEW, "CLICK", onClick, sync)
  self~connectListViewEvent(IDC_LISTVIEW, "BEGINSCROLL", onBeginScroll, .true)
  self~connectListViewEvent(IDC_LISTVIEW, "ENDSCROLL", onBeginScroll)


/** initDialog()
 *
 *  Here we do 1 normal thing, populate the list-view.
 *
 *  The rest is what makes this work. The isGrandchild() method sets up a
 *  connection to some of the event notifications sent by the grandchild
 *  control, to a Rexx method in this dialog.  We need that event connection to
 *  monitor the Esc, Enter key events, and the lost focus event.
 *
 *  The other key thing we do is set the parent of the edit control to be the
 *  list view.  This parent / child relation is what keeps the edit control
 *  drawn correctly, it ensures that the edit control is drawn over the top of
 *  the list view.
 */
::method initDialog
    expose list edit

    list = self~newListView(IDC_LISTVIEW)

    edit = self~newEdit(IDC_EDIT)
    edit~setParent(list)
    edit~isGrandChild

    self~setUpListView(list)


/** onClick()
 *
 *  This is the event handler for a click on the list-view.  We track the clicks
 *  and when we see that the user has clicked twice in a row on the same subitem
 *  we enter editing mode.
 *
 *  When we enter editing mode, we get the rectangle of the subitem we are going
 *  to edit, size the edit control to that size, position the edit control over
 *  the subitem, and make the edit control visible.
 *
 *  We set our flag so that we know the edit control is now visible, and assign
 *  the focus to the edti control.  And that's it.
 */
::method onClick unguarded
  expose edit editVisible lastIdx lastCol
  use arg id, itemIndex, columnIndex, keyState, , lv

  if lastIdx == itemIndex & lastCol == columnIndex then do
    if columnIndex > 0 then do
        r = lv~getSubitemRect(itemIndex, columnIndex, 'LABEL')

        r~right  -= r~left
        r~bottom -= r~top
        flags = "SHOWWINDOW NOZORDERCHANGE"

        edit~setWindowPos(lv~hwnd, r, flags)

        editVisible = .true
        edit~assignFocus
    end
  end

  lastIdx = itemIndex; lastCol = columnIndex

  return 0


/** onBeginScroll()
 *
 *  This is the event handler for the begin and end scroll events.  When the
 *  user is in the editing mode and then moves away from the edit control, we
 *  interpret that as canceling the edit.
 *
 *  This works fine if the user tabs out of the edit control, used the mouse to
 *  click outside the edit control, brings some other application to the fore-
 *  ground.  But, for some reason, clicking on the scroll bars for the list-view
 *  does not trigger the onEditGrandChildEvent() handler.  This seriously messes
 *  up the logic.
 *
 *  The begin and / or end scroll event is sent as soon as the user clicks on
 *  the scroll bars.  So, we connect that event and use the event handler to
 *  hide the edit control if it is visible.
 */
::method onBeginScroll unguarded
    expose editVisible edit
    use arg ctrlID, dx, dy, lv, isBegin

    if editVisible then self~hideEdit(edit)
    return 0


/** onEditGrandChildEvent()
 *
 *  This is the event handler for events that happen in a grandchild control.
 *  There are 4 events that get forwarded on to the grandfathe dialog.  The Esc,
 *  Tab, and Enter key events, and the lost focus event.  Which event, is
 *  specified by the 2nd argument, which uses a keyword to denote the event.
 *
 *  The isGrandChild() method automatically sets up the connection to the 4
 *  events.  Since we did not request the tab key event be connected, we won't
 *  get that notification.  The other 3 notifications, all signal the end of the
 *  editing mode.  On enter, we need to update the subitem text with the new
 *  text.
 */
::method onEditGrandChildEvent unguarded
    expose list lastIdx lastCol
    use arg id, key, editCtrl

    if key == 'enter' then do
        text = editCtrl~getText~strip
        if text \== '' then list~setItemText(lastIdx, lastCol, text)
        self~hideEdit(editCtrl)
    end
    else if key == 'escape' then self~hideEdit(editCtrl)
    else if key == 'killfocus' then self~hideEdit(editCtrl, .false)

    return 0


/** hideEdit()
 *
 *  Makes the edit control invisible, removes its text, and, maybe, assigns the
 *  focus back to the list-view.  This is done each time the editing mode is
 *  ended.
 */
::method hideEdit private unguarded
    expose editVisible list
    use strict arg editCtrl, assignFocus = .true

    if assignFocus then list~assignFocus

    editCtrl~setText("")
    editCtrl~hide
    editVisible = .false


/** setUpListView()
 *
 * Sets up the list view by adding the columns and populating the list with
 * rows.
 */
::method setUpListView private
    use strict arg list

    list~addExtendedStyle("FULLROWSELECT GRIDLINES CHECKBOXES HEADERDRAGDROP")

    list~insertColumn(0, "Row (List-view item)", 75)
    list~insertColumn(1, "Column 2 (subitem 1)", 70)
    list~insertColumn(2, "Column 3 (subitem 2)", 70)

    do i = 1 to 200
        list~addRow(i, , "Row" i, "Row / Col ("i", 2)", "Row / Col ("i", 3)")
    end


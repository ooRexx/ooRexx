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
 *  This example shows how to work with a simple ListView control in report
 *  view.
 *
 *  The main points of the example are:
 *
 *  1.) It shows how to use the LvFullRow, LvItem, and LvSubItem objects to add
 *      items to list-views
 *
 *  2.) It shows how to use custom draw with a list-view change the text and
 *      background color of a single list-view item, and / or change the font
 *      for a single item.
 *
 *  Note that in the .rc file, the resource script file, for this example, the
 *  dialog is created *not* visible.  Thus, while the dialog is being
 *  initialized, it is not visible on the screen.  This eliminates flicker while
 *  the items are being inserted into the list-view.  Some people like the users
 *  of their applications to see the list-view being filled, some people dislike
 *  flicker.  So, it is mostly a matter of preference how you create the dialog,
 *  initially visible, or initially invisible.  But, it is good to be aware of
 *  the option.
 */

    -- Determine which directory our source code files are in:
    sd = locate()

    -- Set the defaults for this application.  Use the global .constDir 'O'nly,
    -- Read the 'customDrawListview.h' file for symbolic resource ID
    -- definitions.  And, turn automatica data detection off (.false.)
    .application~setDefaults('O', sd'rc\customDrawListview.h', .false)

    dlg = .CustomDrawDlg~new(sd'rc\customDrawListview.rc', IDD_CUSTOMDRAW)
    if dlg~initCode = 0 then do
        dlg~execute("SHOWTOP")
    end

return 0
-- End of entry point.

::requires "ooDialog.cls"

/** CustomDrawDlg
 *
 * Our dialog subclass.  To use custom draw, you need to inherit the CustomDraw
 * mixin class.
 */
::class 'CustomDrawDlg' subclass RcDialog inherit CustomDraw

/** init()
 *
 * To recieve the custom draw event notifications, you need to: 1.) Init the
 * custom draw interface, 2.) name the control you want the custom draw
 * notifications for.
 *
 * 1.) You must init the custom draw interface before invoking any of the custom
 * draw methods.  This is done by invoking the customDraw() method.
 *
 * 2.) You name the control you want the custom draw notifications for by
 * invoking the forControl() method with: the resource ID of the control, the
 * string name of the control, and the name of the method to be invoked.
 *
 * At this time, only ListView controls are supported.  Future versions of
 * ooDialog may support more dialog controls.
 *
 * Note that we created the colors and font one time here, rather than create
 * them each time the onCustomDraw() method is invoked.
 *
 * We also create our LvFullRows objects here rather than in initDialog().  This
 * helps allow the dialog to open with everything appearing at once.  The full
 * rows are inserted into the list-view in initDialog().  The rows can only be
 * inserted after the underlying Windows dialog exists.
 */
::method init
    expose textRed textBlack textBlue rowGreen rowLiteBlue rowYellow rows checkedFont

    forward class (super) continue

    self~customDraw
    self~customDrawControl(IDC_LV_CUSTOM, 'ListView', onCustomDraw)

    textBlack   = self~RGB(  0,   0,   0)
    textRed     = self~RGB(247,   7,  59)
    textBlue    = self~RGB( 17,   5, 250)
    rowYellow   = self~RGB(245, 245, 127)
    rowLiteBlue = self~RGB(115, 245, 186)
    rowGreen    = self~RGB(188, 237, 102)

    checkedFont = self~createFontEx("Courier New", 10)

    rows = self~createRows

/** initDialog()
 *
 * initDialog is the place to do any initialization that requires the underlying
 * dialog to exist.  Here we add some extended list-view styles and fill the
 * list-view with its items.
 */
::method initDialog
    expose list rows

    list = self~newListView(IDC_LV_CUSTOM)

    list~addExtendedStyle("FULLROWSELECT GRIDLINES CHECKBOXES HEADERDRAGDROP")

    list~InsertColumn(0, "Line text", 75)
    list~InsertColumn(1, "Line number", 55)
    list~InsertColumn(2, "Numbers", 50)
    list~InsertColumn(3, "Characters", 55)

    list~prepare4nItems(rows~items)
    do r over rows
        list~addFullRow(r)
    end


/** onCustomDraw()
 *
 * This is the event handler that is invoked whenever the list-view needs to
 * draw any item in the list.
 *
 * We are passed a LvCustomDrawSimple object to us.  This object's attributes
 * contain information concerning which item the list-view needs to draw, and
 * other information related to doing custom drawing.  Other attributes of the
 * object need to be set by us in order to control the drawing.
 *
 * You must return true or false from this event handler.  Failure to do so will
 * terminate the dialog and raise a condition.  Returning false basically says
 * you do not want to do any custom drawing for this item.
 *
 * You can change the text color, the background color, and the font by setting
 * those attributes of the LvCustomDrawSimple object.  You also need to set the
 * reply attribute.  This attribute is the reply sent back to the list-view.
 *
 * For this example, we return CDRF_NEWFONT to tell the list-view that either
 * colors, or fonts, were changed and that we do not want to recieve subitem
 * notifications.  Other possible replies are not discussed here as they are not
 * pertinent to this example.  The ooDialog documentation describes the other
 * possible replies.
 *
 * The logic of what we are doing is relatively simple.  We color every other
 * row with a different color, if the user has not checked the row.  All checked
 * rows use a differnt color and font than the unchecked rows.
 */
::method onCustomDraw unguarded
    expose list textRed textBlack textBlue rowGreen rowLiteBlue rowYellow checkedFont
    use arg lvcds

    item = lvcds~item

    if (item // 2) == 0 then do
        if list~isChecked(item) then do
            lvcds~clrText   = textBlue
            lvcds~clrTextBk = rowLiteBlue
            lvcds~font      = checkedFont
        end
        else do
            lvcds~clrText   = textRed
            lvcds~clrTextBk = rowYellow
        end
    end
    else do
        if list~isChecked(item) then do
            lvcds~clrText   = textBlue
            lvcds~clrTextBk = rowLiteBlue
            lvcds~font      = checkedFont
        end
        else do
            lvcds~clrText   = textBlack
            lvcds~clrTextBk = rowGreen
        end
    end

    lvcds~reply = self~CDRF_NEWFONT
    return .true


/** createRows()
 *
 * Here we create LvFullRow objects for every list-view item.  LvItem objects
 * represent the list-view item and LvSubItem objects represent each column in
 * the list-view item.
 *
 * All the full row objects are put into an array.  The array is later used to
 * insert all the items into the list-view.
 */
::method createRows private

    rows = .array~new(10)

    do i = 1 to 1000
        j = i - 1
        lvi = .LvItem~new(j, 'Line' i, , p)
        lvsi1 = .LvSubItem~new(j, 1, i)
        lvsi2 = .LvSubItem~new(j, 2, random(1, 200))
        lvsi3 = .LvSubItem~new(j, 3, self~randomChars)

        rows[i] = .LvFullRow~new(lvi, lvsi1, lvsi2, lvsi3, .false)
    end

    return rows


/** randomChars()
 *
 * Simple method to generate some random string of characters.
 */
::method randomChars private

    len   = random(1, 7)
    chars = ''

    do i = 1 to len
        chars || = random(97, 122)~d2c
    end

    return chars


/** leaving()
 *
 * This method is invoked automatically be the ooDialog framework when the
 * dialog is closed.  It should be used to do any needed clean up.  Here we
 * delete the font we created.
 *
 * Note that the colors we created are just numbers and do not represent any
 * system resource.
 */
::method leaving unguarded
  expose checkedFont
  self~deleteFont(checkedFont)

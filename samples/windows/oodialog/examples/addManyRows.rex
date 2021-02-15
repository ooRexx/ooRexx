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

/**
 *  This example shows how to use the standard dialog, the ProgressDialog
 *  dialog, to show the user how a lengthy operation is progressing.  It also
 *  demonstrates how much faster an internal sort can be than using a Rexx sort.
 *
 *  The main points of the example are:
 *
 *  1.) It shows how to use the ProgressDialog class.
 *
 *  2.) It shows how to use the LvFullRow, LvItem, and LvSubItem objects to add
 *      items to list-views
 *
 *  3.) It shows to sort on a column in the list-view
 *
 *  Note that in the .rc file, the resource script file, for this example, the
 *  dialog is created *not* visible.  Thus, while the dialog is being
 *  initialized, it is not visible on the screen.  This eliminates flicker while
 *  the items are being inserted into the list-view.  Some people like the users
 *  of their applications to see the list-view being filled, some people dislike
 *  flicker.  So, it is mostly a matter of preference how you create the dialog,
 *  initially visible, or initially invisible.  But, it is good to be aware of
 *  the option.
 *
 *  In addition, with a large number of list-view items, it takes some time to
 *  create the LvFullRow objects and insert them into the list-view.  With the
 *  dialog being invisible, the user is going to wonder what is going on.  This
 *  is the perfect time to use a ProgressDialog.
 *
 *  NOTE: During a Rexx sort of the items in a list-view, the list-view invokes
 *        the Rexx method in this dialog to do a comparison of 2 items.  We do
 *        not know what sorting algorithm the list-view is using, but we can
 *        speculate that it is a quick sort.  Each callback from the native code
 *        the list-view is running in, to the Rexx interpreter, is relatively
 *        expensive.  Expensive relative to the time to actually do a single
 *        comparison.  As the number of items to sort grows, the number of
 *        comparisons grows, worst case, exponentially. Even average case,
 *        O(n log n), the growth is far from linear.  As the number of
 *        comparisons grows, the expensive callbacks into the interpreter truely
 *        dominate.  Do not use the Rexx sort for a large number of items.
 *        Start by using a Rexx sort with a small number of items to get a feel
 *        for how long it is taking.  Using a Rexx sort with a large nubmer of
 *        items, will always finish, eventually.  It may literally take hours.
 *
 *  This example times the operations where a progress dialog is displayed and
 *  shows those times in the main dialog.  This is an interesting experiment.
 *
 *  The internal sort is very fast, in comparison.  Do not hesitate to sort
 *  even the maximum number of items using the internal sort.
 */

    -- Ensure we can be run from any directory.
    srcDir = locate()

    -- Set the defaults for this application.  Use the global .constDir 'O'nly,
    -- Read the 'addManyRows.h' file for symbolic resource ID  definitions.
    -- And, turn automatica data detection off (.false.)
    .application~setDefaults('O', srcDir'resources\addManyRows.h', .false)

    -- Allow the user to pick the number of items to be inserted into the
    -- list-view.
    dlgIntro = .ExampleSetUpDlg~new(srcDir'resources\addManyRows.rc', IDD_INTRO)
    if dlgIntro~execute('SHOWTOP') == dlgIntro~IDCANCEL then do
      ret = MessageDialog('You are missing out on an excellent example', ,'User Canceled', 'OK', 'WARNING')
      return 99
    end

    dlg = .AddManyRowsDlg~new(srcDir'resources\addManyRows.rc', IDD_ADD_ROWS)
    if dlg~initCode = 0 then do
        dlg~itemCount = dlgIntro~selectedCount
        dlg~execute("SHOWTOP")
    end

return 0
-- End of entry point.

::requires "ooDialog.cls"


/** AddManyRowsDlg
 *
 * Our main example dialog subclass.
 */
::class 'AddManyRowsDlg' subclass RcDialog

::attribute itemCount
::attribute createRowsTime
::attribute insertRowsTime
::attribute insertedRows
::attribute internalSortTime
::attribute rexxSortTime

/** initDialog()
 *
 * initDialog is the place to do any initialization that requires the underlying
 * dialog to exist.  Here we add some extended list-view styles and fill the
 * list-view with its items.
 */
::method initDialog
    expose list rows createCanceled insertCanceled

    list = self~newListView(IDC_LV)

    list~addExtendedStyle("FULLROWSELECT GRIDLINES CHECKBOXES HEADERDRAGDROP SUBITEMIMAGES")

    list~InsertColumn(0, "Line text", 75)
    list~InsertColumn(1, "Line number", 55)
    list~InsertColumn(2, "Numbers", 50)
    list~InsertColumn(3, "Characters", 55)
    list~InsertColumn(4, "Characters", 55)
    list~InsertColumn(5, "Characters", 55)
    list~InsertColumn(6, "Characters", 55)
    list~InsertColumn(7, "Characters", 55)
    list~InsertColumn(8, "Characters", 55)
    list~InsertColumn(9, "Characters", 55)
    list~InsertColumn(10, "Characters", 55)
    list~InsertColumn(11, "Characters", 55)

    self~connectButtonEvent(IDC_PB_SORT_INTERN, 'CLICKED', onSortInternally)
    self~connectButtonEvent(IDC_PB_SORT_REXX, 'CLICKED', onSortRexx)

    rows = self~createRows(list)

    self~newGroupBox(IDC_GB_TIMES)~setText('Time in seconds (' || self~itemCount || ' rows):')

    self~setStatics

/** setStatics
 *
 * Helper method to set up the static controls that are used to display the
 * timings.  How they are set is dependent on whether the user canceled the
 * LvFullRow creation or the insertion of the rows.
 */
::method setStatics private
    expose createCanceled insertCanceled staticInternal staticRexxSort

    staticCreate   = self~newStatic(IDC_ST_CREATE)
    staticInsert   = self~newStatic(IDC_ST_INSERT)
    staticInternal = self~newStatic(IDC_ST_INTERNAL)
    staticRexxSort = self~newStatic(IDC_ST_REXXSORT)

    if createCanceled then do
        staticCreate~setText('Create full rows:' self~createRowsTime '(canceled)')
        staticInsert~setText('Insert full rows: N/A')
        staticInternal~hide
        staticRexxSort~hide
        self~newPushButton(IDC_PB_SORT_INTERN)~disable
        self~newPushButton(IDC_PB_SORT_REXX)~disable

        rect = staticCreate~windowRect
        s = .Size~new(rect~right - rect~left, rect~bottom - rect~top)
        s~width *= 2
        staticCreate~resizeTo(s)
        return 0
    end

    staticCreate~setText('Create full rows:' self~createRowsTime)

    if insertCanceled then do
        staticInsert~setText('Insert full rows:' self~insertRowsTime '(canceled after inserting' self~insertedRows 'rows)')
        staticInternal~hide
        staticRexxSort~hide
        self~newPushButton(IDC_PB_SORT_INTERN)~disable
        self~newPushButton(IDC_PB_SORT_REXX)~disable

        rect = staticInsert~windowRect
        s = .Size~new(rect~right - rect~left, rect~bottom - rect~top)
        s~width *= 2
        staticInsert~resizeTo(s)
        return 0
    end

    staticInsert~setText('Insert full rows:' self~insertRowsTime)


/** onSortInternally()
 *
 * The event handler for the Internal Sort push button.  We put up a dialog to
 * let the user decide what and how to sort.
 */
::method onSortInternally unguarded
    expose list staticInternal

    dlg = .SortSetupDlg~new(.application~srcDir'resources\addManyRows.rc', IDD_SORT_PARAMS)
    if dlg~execute('SHOWTOP') == dlg~IDCANCEL then return 0

    d = .directory~new
    d~column    = dlg~column
    d~ascending = dlg~ascending
    d~caseless  = dlg~caseless

    j = time('E')

    list~sortItems('InternalListViewSort', d)

    self~internalSortTime = time('e')
    staticInternal~setText('Internal sort:' self~internalSortTime)


/** onSortRexx()
 *
 * The event handler for the Rexx Sort push button.  We put up a dialog to
 * let the user decide what and how to sort.  We also warn the user that the
 * sort can take a very long time depending on the number of items in the
 * list-view
 */
::method onSortRexx unguarded
    expose list staticRexxSort rexxColumn rexxAscending rexxCaseless

    count = self~itemCount

    if count > 1500 & count <= 2000 then do
        msg = "A Rexx sort with" count 'items can take' || .endOfLine || -
              'some time.  Please be patient.'                        || .endOfLine~copies(2)
    end
    else if count > 2000 then do
        msg = "A Rexx sort with" count 'items is not a' || .endOfLine || -
              'good idea, it can take too long.'                      || .endOfLine~copies(2) || -
              'If you want to perform this sort out of' || .endOfLine || -
              'curiousity, start the sort and then let' || .endOfLine || -
              'it alone. Windows will say the window'   || .endOfLine || -
              'is not responding.  But it will event-'  || .endOfLine || -
              'ually finish. 5 minutes for 5000 items.' || .endOfLine || -
              'Times are exponential for more items.'                 || .endOfLine~copies(2)
    end

    if msg~length > 3 then do
        msg || = 'Do you want to continue?'
        title = 'Cautionary Statement - Think Twice'
        if MessageDialog(msg, self~hwnd, title, 'YESNO', 'WARNING') == self~IDNO then return 0
    end

    dlg = .SortSetupDlg~new(.application~srcDir'resources\addManyRows.rc', IDD_SORT_PARAMS)
    if dlg~execute('SHOWTOP') == dlg~IDCANCEL then return 0

    rexxColumn    = dlg~column
    rexxAscending = dlg~ascending
    rexxCaseless  = dlg~caseless

    msg  = 'Performing a list-view item sort using a Rexx method of this dialog.  Please be patient...'
    capt = 'Rexx ooDialog Dialog Method Sort'
    pbDlg = .ProgressDialog~new(capt, msg)

    pbDlg~msgHeight = 2
    pbDlg~marqueeMode = .true
    pbDlg~marqueePause = 50
    pbDlg~noStatus = .true

    j = time('E')
    pbDlg~begin

    reply 0
    list~sortItems('DOREXXSORT')

    pbDlg~endNow

    self~rexxSortTime = time('e')
    staticRexxSort~setText('Rexx sort:' self~rexxSortTime)


/** doRexxSort()
 *
 * This is the call back method that actually does the comparison for 2 items in
 * the list view.
 */
::method doRexxSort unguarded
    expose rexxColumn rexxAscending rexxCaseless
    use arg lvRow1, lvRow2

    if rexxColumn == 0 then do
        text1 = lvRow1~item~text
        text2 = lvRow2~item~text
    end
    else do
        text1 = lvRow1~subitem(rexxColumn)~text
        text2 = lvRow2~subitem(rexxColumn)~text
    end

    if rexxAscending then do
        if rexxCaseless then return text1~caselessCompareTo(text2)
        else return text1~compareTo(text2)
    end
    else do
        if rexxCaseless then return text2~caselessCompareTo(text1)
        else return text2~compareTo(text1)
    end


/** createRows()
 *
 * Here we create LvFullRow objects for every list-view item.  LvItem objects
 * represent the list-view item and LvSubItem objects represent each column in
 * the list-view item.
 *
 * All the full row objects are put into an array.  The array is then used to
 * insert all the items into the list-view.
 */
::method createRows private
    expose createCanceled insertCanceled
    use arg list

    j = time('E')

    createCanceled = .false
    insertCanceled = .false

    count = self~itemCount
    rows = .array~new(count)

    step = (count * 2) / 100
    if \ step~datatype('W') then step = trunc(step)
    pbDlg = .ProgressDialog~new
    pbDlg~msgText = 'Creating and inserting full rows into the list-view. This will take some time.'

    if self~itemCount > 30000 then do
      extra = .endOfLine~copies(2) || 'It is possible that inserting' self~itemCount 'items will exhaust your system resources.'
      pbDlg~msgText || = extra
      pbDlg~msgHeight = 5
    end

    a = .Alerter~new
    pbDlg~setInterruptible(a)
    pbDlg~begin
    pbDlg~updateStatus('0 full rows created')

    do i = 1 to count
        j = i - 1
        lvi    = .LvItem~new(j, 'Line' i)
        lvsi1  = .LvSubItem~new(j,  1, i)
        lvsi2  = .LvSubItem~new(j,  2, random(1, 200))
        lvsi3  = .LvSubItem~new(j,  3, self~randomChars)
        lvsi4  = .LvSubItem~new(j,  4, self~randomChars)
        lvsi5  = .LvSubItem~new(j,  5, self~randomChars)
        lvsi6  = .LvSubItem~new(j,  6, self~randomChars)
        lvsi7  = .LvSubItem~new(j,  7, self~randomChars)
        lvsi8  = .LvSubItem~new(j,  8, self~randomChars)
        lvsi9  = .LvSubItem~new(j,  9, self~randomChars)
        lvsi10 = .LvSubItem~new(j, 10, self~randomChars)
        lvsi11 = .LvSubItem~new(j, 11, self~randomChars)

        rows[i] = .LvFullRow~new(lvi, lvsi1, lvsi2, lvsi3, lvsi4, lvsi5, lvsi6, lvsi7, lvsi8, lvsi9, lvsi10, lvsi11, .true)

        if i // step = 0 then do
              pbDlg~increase
              pbDlg~updateStatus(i 'full rows created')
        end

        if a~isCanceled then do
              pbDlg~updateStatus('canceled after creating' i 'full rows')
              r = SysSleep(1.5)
              pbDlg~endNow
              leave
        end
    end

    self~createRowsTime = time('e')
    self~itemCount = rows~items

    if a~isCanceled then do
        createCanceled = .true
        return rows
    end

    j = time('r')

    pbDlg~updateStatus('0 full rows inserted')

    list~prepare4nItems(rows~items)

    do i = 1 to rows~items
        list~addFullRow(rows[i])

        if i // step = 0 then do
                pbDlg~increase
                pbDlg~updateStatus(i 'full rows inserted')
        end
        if a~isCanceled then do
                pbDlg~updateStatus('canceled after inserting' i 'full rows')
                r = SysSleep(1.5)
                pbDlg~endNow
                leave
        end
    end

    self~insertRowsTime = time('e')

    if a~isCanceled then do
        self~insertedRows = i
        insertCanceled = .true
    end
    else do
        pbDlg~complete
        pbDlg~updateStatus('finished inserting' rows~items 'full rows')
        r = SysSleep(1.5)
        pbDlg~endNow
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
        upper = random(0, 1)
        if upper then chars || = random(65, 90)~d2c
        else chars || = random(97, 122)~d2c
    end

    return chars


/** ExampleSetUpDlg
 *
 * A simple helper dialog for our example.  We let the user choose the number of
 * list-view items to insert.
 *
 * There is not much comment for this class, it is really pretty straight
 * forward, initialize the dialog controls to the beginning state, what until
 * the user closes the dialog, record the number of items the user picked.
 */
::class 'ExampleSetUpDlg' subclass RcDialog

::attribute selectedCount

::method initDialog
    expose selectedCount updItems editItems chkFreeForm firstRB lastRB

    firstRB = .constDir[IDC_RB_1000]
    lastRb  = .constDir[IDC_RB_25000]

    selectedCount = 5000
    self~newRadioButton(IDC_RB_5000)~check

    updItems = self~newUpDown(IDC_SP_ITEMS)
    r = .directory~new
    r~min = 500
    r~max = 50000
    updItems~setRange(r)
    updItems~setPosition(10000)

    updItems~disable
    editItems = self~newEdit(IDC_ED_ITEMS)~~disable
    chkFreeForm = self~newCheckBox(IDC_CK_FREEFORM)

    self~connectButtonEvent(IDC_CK_FREEFORM, 'CLICKED', onClick)
    self~connectUpDownEVent(IDC_SP_ITEMS, 'DELTAPOS', onPosChange)


::method onClick unguarded
    expose chkFreeForm updItems editItems firstRB lastRB


    if chkFreeForm~checked then do
        updItems~enable
        editItems~enable

        do i = firstRB to lastRB
            self~newRadioButton(i)~disable
        end
    end
    else do
        updItems~disable
        editItems~disable

        do i = firstRB to lastRB
            self~newRadioButton(i)~enable
        end
    end

::method onPosChange unguarded
    use arg pos, delta, id, hwnd
    return .UpDown~deltaPosReply(.true, .false, delta * 100)


::method ok unguarded
    expose chkFreeForm updItems firstRB lastRB selectedCount

    if chkFreeForm~checked then do
        selectedCount = updItems~getPosition
    end
    else do
        j = 1
        do i = firstRB to lastRB
            if self~newRadioButton(i)~checked then leave
            j += 1
        end
        select
            when j = 1 then selectedCount = 1000
            when j = 2 then selectedCount = 2000
            when j = 3 then selectedCount = 5000
            when j = 4 then selectedCount = 10000
            when j = 5 then selectedCount = 15000
            when j = 6 then selectedCount = 25000
            otherwise       selectedCount = 0
        end
        -- End select
    end

    return self~ok:super


/** SortSetUpDlg
 *
 * A dialog to let the user choose what and how they want soreted.
 *
 * Similar to the ExampleSetUpDlg class there is not much comment here, the
 * dialog is straight forward, set up the controls, record what the user pickes.
 */
::class 'SortSetUpDlg' subclass RcDialog

::attribute column
::attribute ascending
::attribute caseless

::method initDialog
    expose column ascending caseless rbItem rbAscending rbCaseless upDown edit

    column    = 0
    ascending = .true
    caseless  = .true

    rbItem      = self~newRadioButton(IDC_RB_ITEM)~~check
    rbAscending = self~newRadioButton(IDC_RB_ASCENDING)~~check
    rbCaseless  = self~newRadioButton(IDC_RB_CASELESS)~~check

    upDown = self~newUpDown(IDC_SP_COL)~~disable
    edit   = self~newEdit(IDC_ED_COL)~~disable

    upDown~setRange(1, 11)
    upDown~setPosition(3)

    self~connectButtonEvent(IDC_RB_ITEM, 'CLICKED', onItemClick)
    self~connectButtonEvent(IDC_RB_SUBITEM, 'CLICKED', onSubItemClick)

    rbItem~assignFocus


::method onItemClick unguarded
    expose upDown edit

    upDown~disable
    edit~disable


::method onSubItemClick unguarded
    expose upDown edit

    upDown~enable
    edit~enable


::method ok unguarded
    expose column ascending caseless rbItem rbAscending rbCaseless upDown

    if rbItem~checked then column = 0
    else column = upDown~getPosition

    if rbAscending~checked then ascending = .true
    else ascending = .false

    if rbCaseless~checked then caseless = .true
    else caseless = .false

    return self~ok:super



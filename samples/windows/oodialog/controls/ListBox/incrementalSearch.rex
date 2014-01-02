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
 * An example of a custom implmentation of an incremental search in a ListBox.
 *
 * The list box has its own function that does an incremental search of the
 * items in the list box.  When a character is typed, the list box:
 *
 * "Moves the selection to the first item that begins with the character the
 * user typed.  Multiple characters typed within a short interval are treated as
 * a group, and the first item that begins with that series of characters is
 * selected."
 *
 * One common problem is that the "short interval" is too short.
 *
 * This example makes that short interval longer.  It also adds some custom
 * features.  The user can reset the search by typing the Del key.  The user
 * can also back up the search by one character by using the BackSpace key.
 *
 * We use a UserDialog here, although really an ResDialog or an RcDialog is
 * much better.
 */

    symbolMap = .table~new
    symbolMap[IDC_LB_FILEs]     = 200
    symbolMap[IDC_ST_SEARCHSTR] = 210

    .application~setDefaults('O', symbolMap, .false, 'Courier New', 10)

    dlg = .SimpleDialog~new
    if dlg~initCode = 0 then do
        if dlg~execute("SHOWTOP") == dlg~IDOK then do
            say 'User searched for and found:' dlg~searchAndFound
        end
        else do
            say 'User canceled'
        end
    end

return 0
-- End of entry point.

::requires "ooDialog.cls"

::class 'SimpleDialog' subclass UserDialog

::constant expiration  7  -- Time between keystrokes before the search string is
                          -- reset.  If the user does not type any keys within
                          -- this amount of time, in seconds, the search string
                          -- is set back to blank.

::attribute searchAndFound get
    expose selectedText
    return selectedText

::method init
    expose selectedText searchStr curIndex timer

    forward class (super) continue
    self~create(30, 30, 186, 124, "Directory Listing with Search", "CENTER")

    selectedText = ''
    searchStr    = ''
    curIndex     = 1
    timer        = .nil

::method defineDialog

    self~createListBox(IDC_LB_FILES, 10, 10, 166, 90, 'VSCROLL HSCROLL PARTIAL SORT NOTIFY')
    self~createStaticText(IDC_ST_SEARCHSTR, 10, 102, 64, 11, , '')
    self~createPushButton(IDCANCEL, 74, 105, 50, 14, ,"Cancel")
    self~createPushButton(IDOK, 126, 105, 50, 14, 'DEFAUT', "Ok")

::method initDialog
    expose lb stSearchStr

    lb = self~newListBox(IDC_LB_FILES)
    lb~addDirectory("C:\Windows\System32\*", "READWRITE READONLY HIDDEN SYSTEM DIRECTORY ARCHIVE")

    lb~connectCharEvent
    stSearchStr = self~newStatic(IDC_ST_SEARCHSTR)

    self~setHorizontalExtent(lb)

::method onChar unguarded
    expose searchStr curIndex
    use arg char, isShift, isCtrl, isAlt, misc, lBox

    if self~isDel(char, isCtrl, misc) then do
        self~resetSearch
        return .true
    end

    if self~isBkSpc(char, isCtrl, misc) then do
        len = searchStr~length
        if len > 0 then searchStr = searchStr~substr(1, searchStr~length - 1)
        curIndex = 1
    end
    else do
        if isShift | isCtrl | isAlt | (misc~pos('extended') <> 0) then return .true
        searchStr ||= char~d2c
    end

    self~incrementalSearch(lBox)
    return .false

::method incrementalSearch private unguarded
    expose searchStr curIndex
    use strict arg lBox

    self~setStatusBar
    self~resetTimer

    newIndex = lBox~find(searchStr, curIndex)
    if newIndex <> curIndex then do
        if newIndex == 0 then do
            -- Setting the selected index to 0 has the effect of removing
            -- the selection from all items.  This gives the user the
            -- hint that there is no next item with the current search
            -- string.  We leave the current index unchanged.
            lBox~selectIndex(0)
        end
        else do
            len = searchStr~length
            prefix    = lBox~getText(curIndex)~left(len)
            newPrefix = lBox~getText(newIndex)~left(len)

            -- Only move if the new item is a better match then the
            -- current item.  In other words if the prefixes match,
            -- don't do anything
            if prefix \== newPrefix then do
                lBox~selectIndex(newIndex)
                lBox~makeFirstVisible(newIndex)
                curIndex = newIndex
            end
        end
    end

-- When this method is invoked, the user has not typed a key for whatever
-- the timer period is, we abandon the search string.
::method expired unguarded
    expose searchStr curIndex timer stSearchStr
    searchStr = ''
    curIndex  = 1
    timer     = .nil
    self~setStatusBar

-- Set the timer to the expiration time.  Be sure to cancel the
-- existing timer if it exists or we will get a goofy thing going.
::method resetTimer private unguarded
    expose timer

    reply 0

    if timer \== .nil then timer~cancel

    waitPeriod = .TimeSpan~fromSeconds(self~expiration)
    msgObj     = .Message~new(self, 'expired')
    timer      = .Alarm~new(waitPeriod, msgObj)

-- Reset the incremental search
::method resetSearch private unguarded
    expose searchStr curIndex timer stSearchStr

    if timer \== .nil then timer~cancel

    searchStr = ''
    curIndex  = 1
    timer     = .nil
    self~setStatusBar

-- Test for a Backspace keypress
::method isBkSpc private unguarded
    use strict arg char, isCtrl, misc

    if char == .VK~back then return .true
    return .false

-- Test for a Del keypress
::method isDel private unguarded
    use strict arg char, isCtrl, misc

    if \ isCtrl & misc~pos('extended') <> 0 & char == .VK~delete then return .true
    return .false

-- Show the search string under the list box.  There is a StatuBar control
-- that is a possible future enhancement to ooDialog.  That is where I would
-- really like to display this.
::method setStatusBar private unguarded
    expose stSearchStr searchStr
    stSearchStr~setText(searchStr)

-- If the internal width of the list box is less than the width of an item,
-- not all of the item will be shown.  By default the internal width of the
-- list box is 0.  Here we measure the width of each string and set the internal
-- width of the list box to the width of the widest string, plus 4 for a margin.
::method setHorizontalExtent private
    use strict arg lb

    max = 0
    do i = 1 to lb~items
        s = lb~getTextSizePx(lb~getText(i))
        if s~width > max then max = s~width
    end
    lb~setWidthPx(max + 4)

::method ok unguarded
    expose lb selectedText timer

    -- Be sure to cancel the timer or the dialog won't close until it
    -- does expire.
    if timer \== .nil then timer~cancel
    selectedText = lb~selected
    return self~ok:super

::method cancel unguarded
    expose timer selectedText

    if timer \== .nil then timer~cancel
    selectedText = ''
    return self~cancel:super

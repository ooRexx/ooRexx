/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * treeViewCustomDraw.rex
 *
 * This example demonstrates many of the features of a tree-view control.
 * Including, but not limited to: label editing of items, custom draw, using
 * image lists to supply the icons for tree-view items, using a custom compare
 * function in the Rexx dialog to sort the tree-view items, displaying info
 * tips, etc..
 *
 * Note: this program uses the public routine, locate(), to get the full path
 * name to the directory this source code file is located. In places, the
 * variable holding this value has been callously abbreviated to 'sd' which
 * stands for source directory.
 *
 */

    -- Get our source code file location.
    srcDir = locate()

    -- Use the global .constDir for symbolic IDs and turn automatic data
    -- detection off.
    .application~setDefaults('O', srcDir'rc\treeViewCustomDraw.h', .false)

    dlg = .InventoryDlg~new(srcDir"rc\treeViewCustomDraw.rc", IDD_TREE_DLG)
    if dlg~initCode = 0 then do
        ret = dlg~execute("SHOWTOP")
    end

return 0

::requires "ooDialog.cls"          -- Require the ooDialog framework.


/*- TreeViewConstants - Class - - - - - - - - - - - - - - - - - - - - - - - - *\

   This mixin class defines some constant values and is inherited by the two
   dialog classes in this example.

\*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
::class 'TreeViewConstants' mixinclass Object
::attribute BMP_FILE get
    sd = locate()
    return sd"rc\treeViewCustomDraw.bmp"   -- Icons for selected/not-selected items.
::attribute TREE_FILE get
    sd = locate()
    return sd"treeViewCustomDraw.inp"      -- Input file with the items to build the tree.
::attribute ITEM_FILE get
    sd = locate()
    return sd"treeViewCustomDrawi.inp"     -- Input file with dynamically added items.

::constant APPLICATION_TITLE  "Crazy Sam's Emporium - Inventory"

::constant UNSELECTED_FOLDER  0         -- Index for the icon of an unselected folder item
::constant SELECTED_FOLDER    1         -- Index for the icon of a selected folder item
::constant UNSELECTED_LEAF    2         -- Index for the icon of an unselected leaf item
::constant SELECTED_LEAF      3         -- Index for the icon of a selected leaf item


/*- InventoryDlg - Class- - - - - - - - - - - - - - - - - - - - - - - - - - - *\

   This is the main dialog class for this example.  It contains the tree-view
   control that the example is all about.

\*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
::class 'InventoryDlg' subclass RcDialog inherit CustomDraw TreeViewConstants

/** isLeafItem()  [class method]
 *
 * The isLeafItem() method provides a convenient way to test if any tree-view
 * item is a leaf item rather than a folder item.  Leaf items and folder items
 * in this example use different icon images, so we just test which icon is set
 * for the item.
 */
::method isLeafItem class
  use strict arg treeView, item

  info. = treeView~itemInfo(item)

return info.!Image == self~UNSELECTED_LEAF


/** init()
 *
 * Initialization of the Rexx dialog object.  We do several things here.
 *
 * 1.) Initialize the super class.  Never invoke any method on a dialog class
 *     before the super class is initialized.
 *
 * 2.) So a sanity check to ensure the data files needed can be found.
 *
 * 3.) Connect the event handlers for the tree-view.
 *
 * 4.) Create custom colors to draw the individual items in the tree-view.
 *
 * 5.) Initialize the custom draw interface and register the tree-view control
 *     to use custom draw.
 */
::method init
    expose bkClr oddLevelClr evenLevelClr selectedClr leafClr
    use arg rcFile, idDlg

    self~init:super(rcFile, idDlg)
    if self~initCode <> 0 then return self~initCode

    if self~checkForRequiredFiles == .false then do
      self~initCode = 17
      return self~initCode
    end

    -- Connect dialog control events to methods in the Rexx dialog.
    self~connectTreeViewEvent(IDC_TREE, "EXPANDING", "onExpanding", .true)
    self~connectTreeViewEvent(IDC_TREE, "DEFAULTEDIT")
    self~connectTreeViewEvent(IDC_TREE, "KEYDOWN",   "onKeyDown")
    self~connectTreeViewEvent(IDC_TREE, "GETINFOTIP", "onGetInfoTip")

    self~connectButtonEvent(IDC_PB_NEW,     "CLICKED", "onNewItem")
    self~connectButtonEvent(IDC_PB_DELETE,  "CLICKED", "onDeleteItem")
    self~connectButtonEvent(IDC_PB_SORT,    "CLICKED", "onSortChildren")
    self~connectButtonEvent(IDC_PB_EXP_ALL, "CLICKED", "onExpandAll")
    self~connectButtonEvent(IDC_PB_COL_ALL, "CLICKED", "onCollapseAll")
    self~connectButtonEvent(IDC_PB_INFO,    "CLICKED", "onItemInfo")

    -- Create the custom colors use to draw the tree-view items
    bkClr        = self~RGB(250, 250, 250)
    oddLevelClr  = self~RGB( 82,  61,   0)
    evenLevelClr = self~RGB(  0,  20,  82)
    selectedClr  = self~RGB( 82,   0,  20)
    leafClr      = self~RGB(  0,  82,  20)

    -- Initialize the custom draw interface and register the tree-view control
    -- to use custom draw.
    self~customDraw
    self~customDrawControl(IDC_TREE, 'TreeView')

return 0

/** initDialog()
 *
 * Just as the init() method is for initializing the Rexx dialog object, the
 * initDialog() method is used to initialize the underlying Windows dialog.
 * Some things, like inserting the items into a tree-view, can only be done
 * after the Windows dialog exists.  Those types of things are done here.
 */
::method initDialog
    expose tv

    tv = self~newTreeView("IDC_TREE")

    -- Set the image list for the tree-vies
    image = .Image~getImage(self~BMP_FILE)
    imageList = .ImageList~create(.Size~new(16, 12), 'COLOR8', 5, 2)
    if \image~isNull,  \imageList~isNull then do
         imageList~add(image)
         tv~setImageList(imageList, 'NORMAL')
         image~release
    end

    -- Read the file containing the tree input data and build the tree.
    do while lines(self~TREE_FILE)
        args = self~makeArgs(linein(self~TREE_FILE))
        tv~sendWith('add', args)
    end

    -- Select the item with the text of Computers.
    hItem = tv~find('Computers')
    if hItem \== 0 then tv~select(hItem)

return 0


/*- - - - - - - - - - Event handler(s) - - - - - - - - - - - - - - - - - - - -*/

/** onCustomDraw()
 *
 * This is the event handler for the custom draw event.  Certain of the dialog
 * controls in Windows support custom draw.  Those controls send custom draw
 * event notifications through out the paint cycle when the control is drawing
 * or redrawing itself
 *
 * Please read the custom draw documentation in the ooDialog reference manual to
 * fully understand the details.
 *
 * We take advantage of custom draw here to paint each individual tree-view item
 * a custom color depending on exactly which item is about to be drawn.
 *
 * Our scheme is relatively simple.  Every other level of the tree is painted an
 * alternating color.  If an item is selected, it is painted a reddish color to
 * match the reddish selected icon.  If not selected and a leaf node, we paint
 * it a greenish color to match the non-folder, non-selected greenish icon.
 */
::method onCustomDraw unguarded
  expose tv bkClr oddLevelClr evenLevelClr selectedClr leafClr
  use arg tvcds

  if tvcds~drawStage == self~CDDS_ITEMPREPAINT then do
      tvcds~reply = self~CDRF_NEWFONT

      selected = tv~selected
      isLeaf   = .InventoryDlg~isLeafItem(tv, tvcds~item)

      if selected == tvcds~item then do
        tvcds~clrText   = selectedClr
        tvcds~clrTextBk = bkClr
      end
      else if isLeaf then do
        tvcds~clrText   = leafClr
        tvcds~clrTextBk = bkClr
      end
      else if tvcds~level // 2 == 1 then do
        tvcds~clrText   = oddLevelClr
        tvcds~clrTextBk = bkClr
      end
      else do
        tvcds~clrText   = evenLevelClr
        tvcds~clrTextBk = bkClr
      end

      return .true
  end

  return .false


/** onExpanding()
 *
 * This is the event handler for the EXPANDING event.  The method is invoked
 * when a tree-view item is about to be expanded or collapsed.  The method is
 * invoked before the item is expanded or collapsed.
 *
 * This method demonstrates dynamically adding items to the tree.  The node,
 * 'Special Offers' is initially added to the tree during initDialog with no
 * children.
 *
 * When the user expands the node, the child items for the node are dynamically
 * added here.  The items are loaded from a file in the same manner as the
 * original items are loaded from a file to build the tree.
 *
 * Note that the fourth argument to connectTreeViewEvent() was used for the
 * EXPANDING event and set to true. This causes the interpreter to wait for the
 * reply from this event handler, before it replies to the operating system. If
 * the interpreter were to not wait for the reply, the tree-view control would
 * immediately expand the item before the children were added.
 *
 * When the user collapses the node, all its children items are removed.  This
 * is done through the collapseAndReset method, which collapse the item and also
 * tells the tree-view control to deallocate the children.
 */
::method onExpanding unguarded
    expose tv
    use arg tree, item, what, extra

    itemInfo. = tv~itemInfo(item)

    if itemInfo.!TEXT = "Special Offers" then do
        if what == "EXPANDED", tv~child(item) == 0 then do
            do while lines(self~ITEM_FILE)
                args = self~makeArgs(item, , linein(self~ITEM_FILE))
                newItem = tv~sendWith('insert', args)
            end
        end
        else if what == "COLLAPSED", tv~child(item) \== 0 then do
            tv~collapseAndReset(item)
        end
    end

return .true


/** onKeyDown()
 *
 * This is the event handler for the key down event.  It is invoked each time
 * the user presses a key when the tree-view has the focus.
 *
 * We examime the key pressed to see if it is the insert or delete key.
 *
 * On the insert key we simulate pushing the 'New Item' push button by invoking
 * the event handler for that button.  This allows the user to add a new tree-
 * view item.
 *
 * On the delete key, we delete the selected item.
 */
::method onKeyDown unguarded
  expose tv
  use arg treeId, key

  if key == .VK~DELETE then
      tv~delete(tv~Selected)
  else if key == .VK~INSERT then
      self~onNewItem

return 0

/** onNewItem()
 *
 * This is the event handler for the 'New Item' push button.  It is invoked when
 * the user clicks that button.
 *
 * We put up a dialog that allows the user to fill out the details for a new
 * tree-view item and insert it into the tree-view.
 */
::method onNewItem unguarded
    expose tv

    sd = locate()
    dlg = .NewTreeItemDlg~new(sd"rc\treeViewCustomDraw.rc",  IDD_ADD_TREE_ITEM, tv)
    dlg~execute

return 0


/** onDeleteItem()
 *
 * This is the event handler for the 'Delete Item' push button.  This method is
 * invoked when the user clicks that button.
 *
 * We expand the selected item and all its children recursively.
 */
::method onDeleteItem unguarded
    expose tv

    selected = tv~selected
    if selected == 0 then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- Warning'
        msg   = "There is no tree-view item selected.  If you" || .endOfLine           ||-
                "continue, all items will be deleted."         || .endOfLine~copies(2) ||-
                "Do you want to continue and delete all items?"

        ret = MessageDialog(msg, self~hwnd, title, 'YESNO', 'WARNING', 'DEFBUTTON2')
        if ret == self~IDNO then return 0
    end

    tv~delete(selected)
return 0


/** onGetInfoTip()
 *
 * This is the event handler for the INFOTIP event.  It is invoked when the
 * tree-view wants the text to display in the info tip.
 *
 * If we return the empty string, then no info tip is displayed.  Otherwise, the
 * text returned is displayed.  Here, we only return text when the itemData is
 * the string: '...'  In that case we return some text.  In all other cases we
 * return the empty string and no info tip is displayed.
 */
::method onGetInfoTip unguarded
    expose tv
    use arg id, hItem, text, maxLen, itemData

    if itemData == '...' then return 'There are too many books to list'
    else return ''


/** onSortChildre()
 *
 * This is the event handler for the 'Reverse Sort' push button.  This method is
 * invoked when the user pushes that button.
 *
 * Note that the tree-view provides a function to sort the children of an item,
 * but it only sorts in ascending alphabetical order of the item text.  The
 * ooDialog TreeView class provides that function in the sortChildren() method.
 *
 * To sort in any other order requires using the sortChildrenCB() method.  With
 * this method, the programmer names a 'call back' method in the Rexx dialog.
 * This method is then invoked by the tree-view for each item it needs to
 * determine the order of.
 *
 * This is what we do here, we use the sortChildrenCB method and provide on own
 * comparsion method, rexxSort().
 *
 * We sort the children of the selected item.  If no item is selected we sort
 * the children of the root item.  Note that only the direct children of the
 * parent item are sorted.  To sort all items, the programmer would need to
 * implement a recursive function, similar to the expandAll() or collapseAll()
 * functions.
 */
::method onSortChildren unguarded
    expose tv

    selectedItem = tv~selected
    if selectedItem == 0 then selectedItem = tv~root

    ret = tv~sortChildrenCB(selectedItem, rexxSort)


/** rexxSort()
 *
 * This is our comparison callback function.  We are passed the user item data
 * for the first tree-view item and the user item data for the second tree-view
 * item.  The method needs to return a positive number if the first item is
 * greater than the second, a negative number if the first item is less than the
 * second, and 0 if the two items are equivalent.
 *
 * In this program, for every tree-view item inserted, we set the item data for
 * that item to be the text of the item.  Then, in our comparison function here,
 * we just do a reverse comparison of that text.  This orders the items in
 * descending order rather than ascending order.  The userParam argument is
 * ignored here.
 */
::method rexxSort unguarded
    use arg itemData1, itemData2, userParam

    -- Reverse sort:
    return itemData2~compareTo(itemData1)


/** onExpandAll()
 *
 * This is the event handler for the 'Expand All' push button.  This method is
 * invoked when the user clicks that button.
 *
 * We expand the selected item and all its children recursively.
 */
::method onExpandAll unguarded
    expose tv

    if tv~selected == 0 then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- Error'
        msg   = "There is no tree-view item selected.  Before" || .endOfLine ||-
                "you can expand all items you must first"      || .endOfLine ||-
                "select an item"

        ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING')
        return 0
    end

    if self~isFullyExpanded(tv~selected) then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- Information'
        msg   = "The selected tree-view item is already fully" || .endOfLine ||-
                "expanded.  Expanding will take place, but it" || .endOfLine ||-
                "have no visile effect."

        ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING')
    end

    self~expandAll(tv~selected)

return 0

/** onCollapseAll()
 *
 * This is the event handler for the 'Collapse All' push button.  This method is
 * invoked when the user clicks that button.
 *
 * We collapse the selected item and all its children recursively.
 */
::method onCollapseAll unguarded
    expose tv

    if tv~selected == 0 then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- Error'
        msg   = "There is no tree-view item selected.  Before"  || .endOfLine ||-
                "you can collapse all items you must first"     || .endOfLine ||-
                "select an item"

        ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING')
        return 0
    end

    itemInfo. = tv~itemInfo(tv~selected)
    if itemInfo.!State~wordPos('EXPANDED') == 0 then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- Information'
        msg   = "The selected tree-view item is already collapsed.  You"  || .endOfLine || -
                "may not see anything, but all children of the selected"  || .endOfLine || -
                "item will be collapsed."

        ret = MessageDialog(msg, self~hwnd, title, 'OK', 'INFORMATION')
    end

    self~collapseAll(tv~selected)

return 0

/** onItemInfo
 *
 * This is the event handler for the 'Item Info' push button.  It iw invoked
 * when the button is clicked.
 *
 * We get the selected tree-view item and display information about it.
 */
::method onItemInfo unguarded
    expose tv

    selected  = tv~selected

    if selected == 0 then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- Error'
        msg   = "There is no tree-view item selected.  Before"  || .endOfLine ||-
                "you can display information on an item you"    || .endOfLine ||-
                "must first select an item"

        ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING')
        return 0
    end

    itemInfo. = tv~itemInfo(selected)
    tab1      = '09'x
    tab2      = tab1~copies(2)

    hasChildren = self~logicalToString(itemInfo.!Children)

    title = self~APPLICATION_TITLE~changeStr('-', '') '- Item Information'
    msg   = 'The Selected Item is:'    || tab1 || '"'itemInfo.!Text'"'     || .endOfLine~copies(2) || -
            '  Contains children:'     || tab2 || hasChildren              || .endOfLine || -
            '  Unselected icon index:' || tab1 || itemInfo.!Image          || .endOfLine || -
            '  Selected icon index:'   || tab2 || itemInfo.!SelectedImage  || .endOfLine || -
            '  Item state:'            || tab2 || itemInfo.!State          || .endOfLine || -
            '  Is leaf node:'          || tab2 || self~logicalToString(.InventoryDlg~isLeafItem(tv, selected))

    ret = MessageDialog(msg, self~hwnd, title 'OK', 'INFORMATION')

return 0

/** help()
 *
 * The help command event handler is connected automatically by the ooDialog
 * framework to the command with ID of 9.  The default help event handler
 * provided by ooDialog does nothing.
 *
 * In this example, the Help push button is given the resource ID of 9, and
 * therefore the help event handler is invoked when the user clicks on the Help
 * button.  Rather than do noting when the button is clicked, we over-ride the
 * help method here.  We still do not do much, just inform the user that there
 * is no help.
 */
::method help unguarded
    title = self~APPLICATION_TITLE
    msg   = "There is no help available for this example program."
    ret =  MessageDialog(msg, self~hwnd, title, "OK", "INFORMATION")
return 0


/*- - - - - - - - - - Helper Methods - - - - - - - - - - - - - - - - - - - - -*/


/** expandAll()
 *
 * This helper method recursively expands all children nodes of the specified
 * tree-view item.
 */
::method expandAll private unguarded
    expose tv
    use strict arg item

    if item == 0 then return 0

    tv~expand(item)

    nextItem = tv~child(item)
    do while nextItem \== 0
        self~expandAll(nextItem)
        nextItem = tv~next(nextItem)
    end

return 0

/** collapseAll()
 *
 * This helper method recursively collapse the specified tree-view item and all
 * its children.
 */
::method collapseAll private unguarded
    expose tv
    use strict arg item

    if item == 0 then return 0

    nextItem = tv~child(item)
    do while nextItem \== 0
        self~collapseAll(nextItem)
        nextItem = tv~next(nextItem)
    end

    tv~collapse(item)

return 0


/** isFullyExpanded()
 *
 * This helper method checks if the specified item is fully expanded by
 * recursively descending though the children of the item and checking that each
 * item is expanded.
 *
 * Returns .true if the item is fully expanded, otherwise .false.
 */
::method isFullyExpanded private unguarded
    expose tv
    use strict arg item

    if item == 0 then return .true
    if .InventoryDlg~isLeafItem(tv, item) then return .true

    itemInfo. = tv~itemInfo(item)
    if itemInfo.!State~wordPos('EXPANDED') == 0 then return .false

    nextItem = tv~child(item)
    do while nextItem \== 0
        if \ self~isFullyExpanded(nextItem) then return .false
        nextItem = tv~next(nextItem)
    end

return .true

/** checkForRequiredFiles()
 *
 * This helper method is used to check that the bitmap and input files are
 * available.  It returns .true if they are found and .false if they are not
 * found.
 */
::method checkForRequiredFiles private

    haveError = .false
    file = ''

    if stream(self~BMP_FILE, "C", "QUERY EXISTS") = "" then do
      haveError = .true
      file = self~BMP_FILE
    end
    else if stream(self~TREE_FILE, "C", "QUERY EXISTS") = "" then do
      haveError = .true
      file = self~TREE_FILE
    end
    else if stream(self~ITEM_FILE, "C", "QUERY EXISTS") = "" then do
        haveError = .true
        file = self~ITEM_FILE
    end

    if haveError then do
        title = self~APPLICATION_TITLE~subword(1, 3) '- File Error'
        msg   = "The required data file" file                     || .endOfLine || -
                "does not exist.  Without that file, this progam" || .endOfLine || -
                "will not run."                                   || .endOfLine~copies(2) || -
                "The program will abort."

        ret = MessageDialog(msg, 0, title, 'OK', 'ERROR')
    end

return \haveError


/** makeArgs()
 *
 * This helper method turns a line of comma separted values into an argument
 * array with empty values turned into empty indexes in the array.
 */
::method makeArgs private unguarded
    if arg() == 1 then line = arg(1)
    else line = arg(3)

    args = line~makeArray(',')
    if arg() == 1 then do
        newArgs = .array~new(args~items)
        do i = 1 to args~items
            if args[i]~strip \== "" then newArgs[i] = args[i]~strip('B', '"')
        end
    end
    else do
        newArgs = .array~new(args~items + 2)
        newArgs[1] = arg(1)
        newArgs[2] = 'LAST'

        j = 2
        do i = 1 to args~items
            j += 1
            if args[i]~strip \== "" then newArgs[j] = args[i]~strip('B', '"')
        end
    end

return newArgs

/** logicalToString()
 *
 * This helper method converts and returns a value to true or false if value is
 * a logical, otherwise it returns the empty string.
 */
::method logicalToString private
    use strict arg logical
    if logical == .true then return 'True'
    else if logical == .false then return 'False'
    else return ''

/** print()
 *
 * This helper function is useful in debugging.  It prints the label of the
 * specified tree-view item to the screen.
 */
::method print private unguarded
    expose tv
    use strict arg item, indent

    itemInfo. = tv~itemInfo(item);
    say indent || itemInfo.!Text

return 0


/*- NewTreeItemDlg - Class- - - - - - - - - - - - - - - - - - - - - - - - - - *\

   This dialog class allows the user to add a new item to the tree-view

\*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
::class 'NewTreeItemDlg' subclass RcDialog inherit TreeViewConstants

/** init()
 *
 * The initialization of the Rexx dialog object.  We just use this to grab a
 * reference to the tree-view control in the parent dialog.
 */
::method init
  expose treeControl
  use arg scriptFile, dlgID, treeControl

  -- Initialize the super class.
  self~init:super(scriptFile, dlgID)

/** initDialog()
 *
 * The initialization of the Windows dialog.  We use this to set the state of
 * the various controls in the dialog.
 */
::method initDialog
    expose treeControl editControl childRB folderChk selected

    -- Save a reference to the current selected item
    selected = treeControl~selected

    editControl = self~newEdit(IDC_EDIT_NAME)
    childRB     = self~newRadioButton(IDC_RB_CHILD)
    siblingRB   = self~newRadioButton(IDC_RB_SIBLING)
    folderChk   = self~newCheckBox(IDC_CHK_FOLDER)

    -- If the selected is the root of the tree, a new item has to be inserted as
    -- a child.  So disable the radio buttons that allow the user to choose to
    -- insert as a child or sibling.  And, pre-check the add as a folder check
    -- box.
    if selected == treeControl~root then do
        childRB~~check~disable
        siblingRB~disable
        folderChk~check
    end
    else if .InventoryDlg~isLeafItem(treeControl, selected) then do
        siblingRB~~check~disable
        childRB~disable
    end
    else do
        siblingRB~check
    end

    -- Set a visual cue for the edit control.  This will only show when the edit
    -- control has no text in it, and does not have the focus.
    editControl~setCue("Enter name of new item")

return 0

/** ok()
 *
 * The is the event handler for the Ok button.  It is invoked when the user
 * clicks that button.
 *
 * The ooDialog framework provides a default implementation for this event.  It
 * is important to always close a dialog, once the underlying Windows dialog has
 * been created, through the Ok or Cancel event handlers.  This ensures that the
 * dialog is ended correctly and that the needed clean up.
 *
 * Here we over-ride the superclass ok method so that we can check that the user
 * entered the correct data for the new item and then insert it into the tree-
 * view.  If the user did not enter the data correctly, we giver her a chance to
 * correct it.  We give her a chance to cancel and if she does, we end the
 * dialog cancel instead of ok.
 *
 * Note that we end the dialog by either calling the superclass ok or cancel
 * method.  We prevent the dialog from closing by *not* invoking one of those
 * methods and simply returning.
 */
::method ok
    expose treeControl editControl childRB folderChk selected

    -- Make sure the user has given the item a name.
    text = editControl~getText~strip
    if text == "" then do
        msg   = "You must enter the name of the new item"
        title = self~APPLICATION_TITLE~changestr('-', '') '- Add Item'

        ret = MessageDialog(msg, self~hwnd, title, 'WARNING', 'OKCANCEL')
        if ret == self~IDCANCEL then return self~cancel:super

        editControl~assignFocus
        return 0
    end

    -- See if the user wants to add this as a folder item, or a regular item.
    -- This will determine the image IDs we use when we insert the item
    addAsFolder = folderChk~checked

    -- Now insert the item either as a child or a sibling depending on what the
    -- user requested. Note that we set the item data on each insert to the text
    -- of the inserted item.
    if childRB~checked then do
        if addAsFolder then newItem = treeControl~insert(selected, , text, self~UNSELECTED_FOLDER, self~SELECTED_FOLDER, , , text)
        else newItem = treeControl~insert(selected, , text, self~UNSELECTED_LEAF, self~SELECTED_LEAF, , , text)
        treeControl~expand(treeControl~parent(newItem))
    end
    else do
        if addAsFolder then treeControl~insert(treeControl~Parent(selected), , text, self~UNSELECTED_FOLDER, self~SELECTED_FOLDER, , , text)
        else treeControl~insert(treeControl~Parent(selected), , text, self~UNSELECTED_LEAF, self~SELECTED_LEAF, , , text)
    end

    -- Finally, quit by invoking the super class ok() method.
return self~ok:super



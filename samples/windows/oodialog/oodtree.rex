/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* Name: oodtree.rex                                                        */
/* Type: Object REXX Script using OODialog                                  */
/* Resource: oodtree.rc                                                     */
/*                                                                          */
/* Description:                                                             */
/*                                                                          */
/* Demonstration of a tree control using the Object REXX TreeControl class. */
/*                                                                          */
/* This file has been created by the Object REXX Workbench OODIALOG         */
/* template generator.                                                      */
/*                                                                          */
/****************************************************************************/




/* Install signal handler to catch error conditions and clean up */
signal on any name CleanUp

MyDialog = .MyDialogClass~new
if MyDialog~InitCode = 0 then do
  rc = MyDialog~Execute("SHOWTOP")
end

/* Add program code here */

exit   /* leave program */


/* ---- signal handler to destroy dialog if error condition was raised  ----*/
CleanUp:
   call errorDialog "Error" rc "occurred at line" sigl":" errortext(rc),
                     || "a"x || condition("o")~message
   if MyDialog~IsDialogActive then MyDialog~StopIt


::requires "ooDialog.cls"    /* This file contains the OODIALOG classes */
::requires "WINSYSTM.CLS"    /* This file contains the Windows classes */


/* ---------------------------- Directives ---------------------------------*/

::class MyDialogClass subclass UserDialog inherit AdvancedControls MessageExtensions VirtualKeyCodes

::method Init
  use arg InitStem.
  if Arg(1,"o") = 1 then
     InitRet = self~Init:super
  else
     InitRet = self~Init:super(InitStem.)   /* Initialization stem is used */

  if self~Load("rc\oodtree.rc", ) \= 0 then do
     self~InitCode = 1
     return
  end

  /* Connect dialog control items to class methods */
  self~ConnectTreeNotify("IDC_TREE","SelChanging","OnSelChanging_IDC_TREE")
  --self~ConnectTreeNotify("IDC_TREE","SelChanged","OnSelChanged_IDC_TREE")
  self~ConnectTreeNotify("IDC_TREE","Expanding","OnExpanding_IDC_TREE")
  --self~ConnectTreeNotify("IDC_TREE","Expanded","OnExpanded_IDC_TREE")
  self~ConnectTreeNotify("IDC_TREE","DefaultEdit")
  self~ConnectTreeNotify("IDC_TREE","BEGINDRAG", "DefTreeDragHandler")
  --self~ConnectTreeNotify("IDC_TREE","Delete","OnDelete_IDC_TREE")
  self~ConnectTreeNotify("IDC_TREE","KeyDown","OnKeyDown_IDC_TREE")
  self~ConnectButton("IDC_PB_NEW","IDC_PB_NEW")
  self~ConnectButton("IDC_PB_DELETE","IDC_PB_DELETE")
  self~ConnectButton(IDC_PB_EXP_ALL,"IDC_PB_EXP_ALL")
  self~ConnectButton(IDC_PB_COL_ALL,"IDC_PB_COL_ALL")
  self~ConnectButton(IDC_PB_INFO,"IDC_PB_INFO")
  self~ConnectButton(2,"Cancel")
  self~ConnectButton(9,"Help")
  self~ConnectButton(1,"OK")


  /* Initial values that are assigned to the object attributes */
  self~IDC_TREE= 'Products' /* Text of the item which schould be selected */

  /* Add your initialization code here */
  return InitRet


/* Initialization Code, fill tree with initialization data  */
::method InitDialog
  expose bmpFile treeFile itemFile

  InitDlgRet = self~InitDialog:super

  curTree = self~newTreeView("IDC_TREE")
  if curTree \= .Nil then
  do
    bmpFile  = "bmp\oodtree.bmp"  /* file which contains the icons for selected/not-selected items */
    treeFile = "oodtree.inp"  /* input file which contains the items to build the tree         */
    itemFile = "oodtreei.inp" /* input file which contains the items to build the special item */

    /* check the file existence and display an error messge */
    if stream(bmpFile, "C", "QUERY EXISTS") = "" then
    do
      call infoDialog "Data file " bmpFile " does not exist"
    end

    if stream(treeFile, "C", "QUERY EXISTS") = "" then
    do
      call infoDialog "Data file " treefile " does not exist !"
    end

    if stream(itemFile, "C", "QUERY EXISTS") = "" then
    do
      call infoDialog "Data file " itemFile " does not exist"
    end

    /* Set image list for Tree control IDC_TREE */
    image = .Image~getImage(bmpFile)
    imageList = .ImageList~create(.Size~new(16, 12), .Image~toID(ILC_COLOR8), 5, 2)
    if \image~isNull,  \imageList~isNull then do
       imageList~add(image)
       curTree~setImageList(imageList, .Image~toID(TVSIL_NORMAL))
       image~release
    end

    /* Read the file containig the tree input data and build the tree */
    do while lines(treeFile)
      line = linein(treeFile)
      command = "curTree~Add("||line||")"
      interpret command
    end
  end

  return InitDlgRet


::method DefineDialog
  result = self~DefineDialog:super
  if result = 0 then do
     /* Additional dialog items (e.g. AddInputGroup) */
  end


/* --------------------- message handler -----------------------------------*/

/* Method OnSelChanging_IDC_TREE handles notification 'SelChanging' for item IDC_TREE */
::method OnSelChanging_IDC_TREE
  curTree = self~newTreeView("IDC_TREE")

  /* diaplay items which are selected once as bolds */
  curTree~Modify( curtree~selected,,,,"BOLD")



/* Method OnExpanding_IDC_TREE handles notification 'Expanding' for item IDC_TREE */
::method OnExpanding_IDC_TREE
  expose itemFile
  use arg tree, item, what
  curTree = self~newTreeView(tree)
  itemInfo. = curTree~ItemInfo(item)

  /* if the special item is selected, load the child items dynamically from a file */
  /* and delete all children of the item if it will be collapsed                   */
  if itemInfo.!TEXT = "Special Offers" then
  do
    if what = "COLLAPSED" & curTree~Child(item) = 0 then
    do
      do while lines(itemFile)
        line = linein(itemFile)
        command = "curTree~Insert(item,,"||line||")"
        interpret command
      end
      curTree~Expand(item)
    end
    else
      if what = "EXPANDED" & curTree~Child(item) \= 0 then
      do
        curTree~CollapseAndReset(item)
      end
  end

/* track and display the notification messages */
/* uncomment this if you want to get the notification messages and their parameters displayed */
/*
::method HandleMessages
  msg = self~PeekDialogMessage
  if msg \= "" then say msg
  forward class(super)
*/


/* Method OnKeyDown_IDC_TREE handles notification 'KeyDown' for item IDC_TREE */
::method OnKeyDown_IDC_TREE
  use arg treeId, key
  curTree = self~newTreeView(treeId)
  /* if DELETE key is pressed, delete the selected item */
  if self~KeyName(key) = "DELETE" then
    curTree~Delete(curTree~Selected)
  else
    /* if INSERT key is pressed, simulate pressing the New button */
    if self~KeyName(key) = "INSERT" then
      self~IDC_PB_NEW


/* Method IDC_PB_NEW is connected to item IDC_PB_NEW */
::method IDC_PB_NEW
  -- When the new button is pressed, display a dialog that gets the name of the new item
  -- and inserts it into the tree.
  dlg = .NewTreeItemDlg~new("rc\oodtreeNewItem.rc",  IDD_ADD_TREE_ITEM, self~newTreeView("IDC_TREE"))
  dlg~execute


/* Method IDC_PB_DELETE is connected to item IDC_PB_DELETE */
::method IDC_PB_DELETE
  /*delete the selected item */
  curTree = self~newTreeView("IDC_TREE")
  curTree~Delete(curTree~Selected)


/* Method IDC_PB_EXP_ALL is connected to item IDC_PB_EXP_ALL */
::method IDC_PB_EXP_ALL
  /*expand the selected item and all its childs */
  curTree = self~newTreeView("IDC_TREE")
  if curTree~Selected = 0 then
    call infoDialog "No item selected !"
  else do
    curTree~Expand(curTree~Selected)
    nextItem = curTree~Child(curTree~Selected)
    do while nextItem \= 0
      curTree~Expand(nextItem)
      nextItem = curTree~Next(nextItem)
    end
  end


/* Method IDC_PB_COL_ALL is connected to item IDC_PB_COL_ALL */
::method IDC_PB_COL_ALL
  /*collapse the selected item and all its childs */
  curTree = self~newTreeView("IDC_TREE")
  if curTree~Selected = 0 then
    call infoDialog "No item selected !"
  else do
    nextItem = curTree~Child(curTree~Selected)
    do while nextItem \= 0
      curTree~Collapse(nextItem)
      nextItem = curTree~Next(nextItem)
    end
    curTree~Collapse(curTree~Selected)
  end

/* Method IDC_PB_INFO is connected to item IDC_PB_COL_ALL */
::method IDC_PB_INFO
  /* Display the attributes of the selected item */
  use arg tree
  curTree = self~newTreeView("IDC_TREE")
  itemInfo. = curTree~ItemInfo(CurTree~Selected)

  if itemInfo.!Children = 0 then
    children = "no"
  else
    children = ""

  call InfoDialog 'The selected item "'itemInfo.!Text'" has' children 'children. The index for the icon is "'itemInfo.!Image'"',
                   ', the index for the selected icon is "'itemInfo.!SelectedImage'". The states are "'itemInfo.!State'".'

/* Method Help is connected to item 9 */
::method Help
  call infoDialog "No help available."
  self~Help:super

::class 'NewTreeItemDlg' subclass RcDialog inherit AdvancedControls

::method init
  expose treeControl
  use arg scriptFile, dlgID, treeControl

  -- Initialize the super class.
  self~init:super(scriptFile, dlgID)

::method initDialog
  expose treeControl editControl childRB folderChk selected

  -- Save a reference to the current selected item
  selected = treeControl~selected

  -- Save a reference to some of the controls we will use repeatedly
  editControl = self~newEdit(IDC_EDIT_NAME)
  childRB = self~newRadioButton(IDC_RB_CHILD)
  folderChk = self~newCheckBox(IDC_CHK_FOLDER)

  -- If the selected is the root of the tree, a new item has to be inserted as
  -- a child.  So disable the radio buttons that allow the user to choose to
  -- insert as a child or sibling.  And, pre-check the add as a folder check
  -- box.
  if selected == treeControl~root then do
    childRB~~check~disable
    self~newRadioButton(IDC_RB_SIBLING)~disable
    folderChk~check
  end
  else do
    self~newRadioButton(IDC_RB_SIBLING)~check
  end

  -- Set a visual cue for the edit control.  This will only show when the edit
  -- control has no text in it, and does not have the focus.
  editControl~setCue("Enter name of new item")

::method ok
  expose treeControl editControl childRB folderChk selected

  -- Make sure the user has given the item a name.
  text = editControl~getText~strip
  if text == "" then do
    j = infoDialog("You must enter the name of the new item")
    editControl~assignFocus
    return 0
  end

  -- See if the user wants to add this as a folder item, or a regular item.
  -- This will determine the image IDs we use when we insert the item
  addAsFolder = folderChk~checked

  -- Now insert the item either as a child or a sibling depending on what the
  -- user requested.
  if childRB~checked then do
    if addAsFolder then newItem = treeControl~insert(selected, , text, 0, 1)
    else newItem = treeControl~insert(selected, , text, 2, 3)
    treeControl~expand(treeControl~parent(newItem))
  end
  else do
    if addAsFolder then treeControl~insert(treeControl~Parent(selected), , text, 0, 1)
    else treeControl~insert(treeControl~Parent(selected), , text, 2, 3)
  end

  -- Finally, quit by invoking the super class ok() method.
  return self~ok:super

::method initAutoDetection
  self~noAutoDetection

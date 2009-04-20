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
/* Name:                                                                    */
/* Type: Object REXX Script                                                 */
/* Resource: oodlist.rc, oodlist1.bmp, oodlist2.bmp                         */
/*                                                                          */
/* Description:                                                             */
/*                                                                          */
/* Demonstration of the various list controls using the                     */
/* Object REXX ListControl and PropertySheet class.                         */
/*                                                                          */
/* This file has been created by the Object REXX Workbench OODIALOG         */
/* template generator.                                                      */
/*                                                                          */
/****************************************************************************/

dlg = .ListsDialog~new(,,,,"WIZARD")

if dlg~InitCode \= 0 then do
  say "Dialog init did not work"
  exit
end


/* create a centered property sheet with 9pt Arial font */
dlg~createcenter(400, 200, "New List Controls",,,"Arial")
dlg~execute("SHOWTOP")
dlg~deinstall

return

::requires OODWIN32.CLS    /* property sheet is defined in OODialog extension */
::requires "WINSYSTM.CLS"           /* This file contains the Windows classes */




/* ---------------------------- Directives ---------------------------------*/

::class ListsDialog subclass PropertySheet inherit VirtualKeyCodes

/* track and display the notification messages */
/* uncomment this if you want to get the notification messages and their parameters displayed */
/*
::method HandleMessages
   if self~PeekDialogMessage \= "" then say self~PeekDialogMessage
   forward class (super)
*/

::method InitDialog
  forward class (super) continue /* call parent constructor */

  /* initialization data for all lists */
  Item.FirstName = "Mike"
  Item.Lastname  = "Miller"
  Item.Street    = "Street 1"
  Item.City      = "City 1"
  Item.Age       = "35"
  Item.Sex       = "M"
  self~UpdateLists(Item.)

  Item.FirstName = "Sue"
  Item.Lastname  = "Thaxton"
  Item.Street    = "Street 2"
  Item.City      = "City 2"
  Item.Age       = "30"
  Item.Sex       = "F"
  self~UpdateLists(Item.)

  Item.FirstName = "Dave"
  Item.Lastname  = "Hewitt"
  Item.Street    = "Street 3"
  Item.City      = "City 3"
  Item.Age       = "49"
  Item.Sex       = "M"
  self~UpdateLists(Item.)


/* set the categories: used as tab lable, definition method names and inititalization method names */
::method InitCategories
  expose ID_LIST ID_Rep ID_Ic ID_SIc

  self~catalog['names'] = .array~of("List", "Report", "Icon", "Small Icon") /* create the category pages */
  self~catalog['page']['font'] = "Arial"   /* use 9pt Arial for the pages as well */
  self~catalog['page']['btnwidth'] = 60
  self~catalog['page']['btnheight'] = 60
  self~ConnectButton(201,"OnAddButton")

  ID_List = 101 /* list view       */
  ID_Rep  = 102 /* report view     */
  ID_Ic   = 103 /* icon view       */
  ID_SIc  = 104 /* small icon view */

  self~createImageLists

::method CategoryPage
  forward class (super) continue /* call parent constructor */
  self~AddButton(201,160,185,60,12,"Add")

/* initialize first category page */
::method List
  expose ID_List
  self~AddListControl( ID_List, , 5, 5, 370, 170, "LIST") /* Add a simple list to the first category page */

/* initialize second category page */
::method Report
  expose ID_Rep
  self~AddListControl( ID_Rep, , 5, 5, 370, 180, "REPORT") /* Add a report list to the second category page */
  self~ConnectListNotify(ID_Rep,"ColumnClick")  /* connect click on a column header with OnColumnClick */
  self~ConnectListNotify(ID_Rep,"Activate")     /* connect double-click on a list item with OnActivate */

/* initialize third category page */
::method Icon
  expose ID_Ic
  self~AddListControl( ID_Ic, , 5, 5, 370, 140, "ICON") /* Add a icon list to the third category page */


/* initialize fourth category page */
::method SmallIcon
  expose ID_SIc
  self~AddListControl( ID_SIc, , 5, 5, 370, 140, "SMALLICON") /* Add a small icon list to the fourth category page */


/* initialize list control on first category page */
::method InitList
  expose ID_List
  curList = self~GetListControl(ID_List)

/* initialize report list control on second category page */
::method InitReport
  expose ID_Rep smallIcons

  curList = self~GetListControl(ID_Rep)
  if curList \= .Nil then
  do /* connect bitmap and insert colums */
    curList~setImageList(smallIcons, .Image~toID(LVSIL_SMALL))
    curList~InsertColumn(0,"Last Name",50)
    curList~InsertColumn(1,"First Name",50)
    curList~InsertColumn(2,"Street",50)
    curList~InsertColumn(3,"City",50)
    curList~InsertColumn(4,"Age",50)
  end
  else
    return

/* initialize icon list control on third category page */
::method InitIcon
  expose ID_Ic normalIcons
  curList = self~GetListControl(ID_Ic)
  if curList \= .Nil then
  do
    self~ConnectListNotify(ID_Ic,"BEGINDRAG","DefListDragHandler")
    curList~setImageList(normalIcons, .Image~toID(LVSIL_NORMAL))
  end
  else
    return

/* initialize small icon list control on second category page */
::method InitSmallIcon
  expose ID_SIc smallIcons
  curList = self~GetListControl(ID_SIc)
  curList~setImageList(smallIcons, .Image~toID(LVSIL_SMALL))
  self~ConnectListNotify(ID_SIc,"BEGINDRAG","DefListDragHandler") /* connect default drag handler */


/* a column was selected display info about the column */
::method OnColumnClick
  expose ID_Rep
  use arg id, column
  curList = self~GetListControl(ID_Rep)
  curList~SetColumnWidth(column,curList~ColumnWidth(column)+10)
  info. = curlist~ColumnInfo(column)
  call InfoDialog("Column Title : " info.!Text"d"x,
                  "Column Number : " info.!Column"d"x,
                  "Column Width : " info.!Width"d"x,
                  "Allignment : " info.!Align )

/* an item was double clicked (activated in Windows terms.) */
::method OnActivate
  use arg id
  curList = self~GetListControl(id)
  if curList == .nil then return

  -- Get the index of the item with the focus and the text associated with it
  index = curList~Focused
  firstName = curList~ItemText(index, 1)
  age = curList~ItemText(index, 4)

  -- The item info stem will contain the text for column 0 and the icon index
  info. = curList~ItemInfo(index)
  lastName = info.!Text
  iconIndex = info.!Image
  if iconIndex == 1 then pronoun = "her"; else pronoun = "his"

  msg = "You have doubled clicked on" firstName lastName || "0d0a0d0a"x ,
        "Should" pronoun "age be increased by 1?"
  ret = AskDialog(msg)
  if ret == 1 then do
    age = age + 1
    curList~SetItemText(index, 4, age)
  end

  -- Deselect the focused item and move the focus to the first item
  curList~Deselect(index)
  curList~Focus(0)

/* Add button selected, handle the address input dialog */
::method OnAddButton
  AdrDlg = .AdrDialogClass~new
  if AdrDlg~InitCode = 0 then
  do
    AdrDlg~Execute("SHOWTOP")

    if AdrDlg~ID_LNAME \= "" then
    do
      Item.FirstName = AdrDlg~ID_FNAME
      Item.Lastname  = AdrDlg~ID_LNAME
      Item.Street    = AdrDlg~ID_STREET
      Item.City      = AdrDlg~ID_CITY
      Item.Age       = AdrDlg~ID_AGE
      if AdrDlg~ID_MALE = 1 then
        Item.Sex = "M"
      else
        Item.Sex = "F"

      self~UpdateLists(Item.)
    end
  end

/* update all lists when a new item was added */
::method UpdateLists
  expose ID_Rep
  use arg Item.

  do j = 1 to 4
    ListId = j+100
    curList = self~GetListControl(ListId,j)
    if Item.Sex = "F" then
      iSex = 1
    else
      iSex = 0
    if ListID = ID_Rep then
      curlist~~AddRow(, iSex, Item.LastName, Item.FirstName, Item.Street, Item.City, Item.Age)
    else
      curlist~Insert(,,Item.LastName"," Item.FirstName, iSex)
  end

/* Create the small and large icon image lists.  This will be used in the
 * report, small icon, and icon views.  The small icon image list will be used
 * in both the report and the small icon views.
 */
::method createImageLists private
  expose smallIcons normalIcons

  small = .Image~getImage("bmp\oodlist1.bmp")
  tmpIL = .ImageList~create(.Size~new(16, 12), .Image~toID(ILC_COLOR4), 4, 0)
  if \small~isNull,  \tmpIL~isNull then do
      tmpIL~add(small)
      small~release
      smallIcons = tmpIL
  end
  else do
    smallIcons = .nil
  end

  normal = .Image~getImage("bmp\oodlist2.bmp")
  tmpIL = .ImageList~create(.Size~new(32, 32), .Image~toID(ILC_COLOR4), 4, 0)
  if \normal~isNull,  \tmpIL~isNull then do
      tmpIL~add(normal)
      normal~release
      normalIcons = tmpIL
  end
  else do
    normalIcons = .nil
  end

/* The Address input dialog, invoked when the Add button is selected */
::class AdrDialogClass subclass UserDialog inherit AdvancedControls

::method Init
  forward class (super) continue
  InitRet = Result

  if self~Load("rc\oodlist.rc" ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  /* connect Pk and Cancel button */
  self~ConnectButton(1,"Ok")
  self~ConnectButton(2,"Cancel")

  /* initialize input fields */
  self~ID_MALE=1
  self~ID_FEMALE=0
  self~ID_FNAME= ''
  self~ID_LNAME= ''
  self~ID_STREET= ''
  self~ID_CITY= ''
  self~ID_AGE= ''

  return InitRet


/* check that at least the last name was entered */
::method Validate
  if self~GetEntryLine(ID_LNAME) \= "" then
    return 1
  else
  do
    call InfoMessage("Last Name must be Specified !")
    return 0
  end


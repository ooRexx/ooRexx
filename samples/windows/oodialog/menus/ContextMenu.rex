/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2014 Rexx Language Association. All rights reserved.    */
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

/* A demonstration of some of the Menu features in ooDialog.
 *
 * This example focuses on context menus.  Two different context menus are used
 * in this example, showing 2 different ways to create the context menus.  One
 * of the context menus is shown when the user right-clicks on the list view
 * control.  The other context menu is shown when the user right-clicks on any
 * other place on the dialog.  The first menu is specific to the list view, the
 * other menu is specific to the dialog.
 *
 * The list view menu is created by loading a menu from a resource script file.
 * Menus created from a resouce script are always menu bars.  In this example,
 * the menu bar is *not* attached to the dialog, rather it is simply used as the
 * source of the list view context menu.
 *
 * The second context menu is created dynamically using methods of the PopupMenu
 * class.
 */

  -- Locate the directory our source code is in.  Then this program can be
  -- executed from anywhere.  It will work if the program is dragged and dropped
  -- on ooDialog.exe for instance.
  srcDir = locate()

  -- When using symbolic IDs with menus, as this program does, the programmer
  -- *must* use the global constant directory, (.constDir,) rather than the
  -- constDir attribute of a dialog.  Menus are independent of dialogs.
  --
  -- The use of the global constant directory is controlled by the .Application
  -- object.  The .Application object is also used to set other application-wide
  -- defaults or values.  In this program, we also want to make the default for
  -- automatic data detection to be off.  Both of these requirements can be done
  -- with one method call to the .Application object.  As a convenience, the
  -- setDefaults() method will also populate the .constDir with symbols.
  --
  -- In this invokcation, the application is set to use the .constDir only, "O",
  -- symbols are loaded into the .constDir from the file: ContextMenu.h, and the
  -- default for automatic data detection is set to off (.false.)  Note that we
  -- create a complete path name for ContextMenu.h
  .application~setDefaults("O", srcDir'ContextMenu.h', .false)

  -- The folloing demonstrates that menu objects are distinct from dialogs.
  --
  -- You can create a menu completely indepedent of a dialog.  Note that at this
  -- point in the program, there is no dialog object at all.  Once created, the
  -- menu object can then be used where ever it is convenient.
  --
  -- The code for creating the list view menu is put in a separate function so
  -- that people can focus on only one thing at a time.  The function is at the
  -- end of this file.  In createListViewMenu(), the context menu is created
  -- dynamically and passed back to us.
  contextMenu = createListViewMenu()

  -- Create a RcDialog as normally done.
  dlg = .ContextMenuDlg~new(srcDir"ContextMenu.rc", IDD_CONTEXT)

  -- putMenu() is a method added to our dialog class.  It is used to pass the
  -- context menu object to the dialog.  There are any number of ways to
  -- accomplish this.  For instance the menu object could have been passed in
  -- as an argument to new() above.
  dlg~putMenu(contextMenu)

  -- Excute the dialog as usual.
  if dlg~initCode = 0 then do
    dlg~execute("SHOWTOP")
  end
  else do
    say 'Error creating the context menu dialog initCode:' dlg~initCode
    return 99
  end

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::class 'ContextMenuDlg' subclass RcDialog

/** init()
 *
 * The init() method is invoked for every object instantiation.  This is part of
 * ooRexx and has nothing to do with ooDialog, except for this:
 *
 * ooDialog programming is primarily based on subclassing the dialog classes
 * provided by the ooDialog framework.  If you over-ride the init() method, you
 * *must* initialize the base class, with the proper arguments for the base
 * class, before you invoke any methods of the base class.
 *
 * Here with create the second context menu and create some arrays that contain
 * the list view items.  Both of these tasks do not need to be done in this
 * method, they could be done elsewhere.
 */
::method init
  expose dlgPopup
  use arg rcScript, dlgID
  forward class (super) continue

  -- A menu bar can be loaded from a resource script file using the
  -- .ScriptMenuBar class.
  --
  -- The first argument to init(), rcScript, is the .rc file that contains the
  -- dialog template.  That .rc file also contains the menu template for the
  -- dialog context menu.  In the new method of the .ScriptMenuBar the .rc file
  -- is parsed and the  menu with symbolic resource id of: IDM_RC_DLG is loaded
  -- into memory.
  menuBar = .ScriptMenuBar~new(rcScript, IDM_RC_DLG)

  -- If an error occurs in a menu object initialization, then a condition is
  -- always raised.  menuBar will never be 0
  if menuBar == 0 then do
    -- This code will never execute because if the menu bar was not created, a
    -- condition was raised.
    say 'ERROR: menubar shold be good, not 0.'
    say 'A condition should have been raised.'
    say 'SystemErrorCode:' .SystemErrorCode SysGetErrortext(.SystemErrorCode)
    self~initCode = 1
    return
  end

  -- A menu bar is made up of a number of pop up menus.  You can get one of
  -- those pop up menus by specifying either its resource ID or its position.
  --
  -- In this program, the menu bar itself is not used.  It contains only 1 pop
  -- up menu, which is used as a context menu in this program.  Which pop up
  -- menu to get is specified either by its resource ID or by its position in
  -- the menu.  Here, we get it by its position, which is 1.  Since both a
  -- resource ID and a position are numbers, we need to denote whether we are
  -- getting the menu by position or by resource ID.  The second argument is the
  -- 'byPosition' argument.  If true, the first arugment is a position.  If
  -- false, the first argument is a resource ID.  The default is .false.
  dlgPopup = menubar~getPopup(1, .true)

  -- Start with the menu item to enable the list view greyed out.
  dlgPopup~disable(IDM_RC_ENABLE_LV)

  -- Create an array of items for the list view control.  This could be done in
  -- initDialog(), or anywhere convenient.
  self~createArrays()

/** initDialog()
 *
 * initDialog() is invoked automatically by the dialog framework as soon as the
 * underlying Windows dialog is created.  This is the place to do any
 * initialization of the dialog controls, initialization that requires the
 * underlying dialog or dialog control to exist.
 *
 * We would not necessarily need to make the menu connections here.  However,
 * as you will see in makeMenuConnections() method, we are using the window
 * handles of the dialog and the list view to filter the context menu
 * notifications.  This does require that the underlying dialog and controls do
 * exist.
 */
::method initDialog

  self~makeMenuConnections
  self~initListView


/** makeMenuConnections()
 *
 * Connects the menu related events to event handling methods in this dialog.
 *
 * There are 2 types of connections made here.
 *
 * 1.) The context menu event is connected.  The context menu event is generated
 * when ever the user right clicks on the dialog, types the shift-F10 keyboard
 * combination, or the VK_APPS key (Natural keyboard.)  The context menu event
 * needs to be connected to a method that shows the context menu.
 *
 * 2.) The menu item command events are connected.  A menu item command event is
 * generated when the user selects a menu item.
 */
::method makeMenuConnections private
  expose lvPopup dlgPopup

  -- Connect a click on a menu item in the short cut menus to this dialog.
  --
  -- For the dialog popup menu, we connect all menu items to the same method,
  -- onMenuCommand().  onMenuCommand() will recieve the resource ID of the menu
  -- item selected, and we use that to decide what action to take.
  if \ dlgPopup~connectAllCommandEvents(onMenuCommand, self) then do
    say 'Error conecting menu items. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end

  -- To understand this explanation, note this point: although menu objects are
  -- independent of dialog objects, at the time a menu is shown, the operating
  -- system requires that the underlying menu be assigned to an underlying
  -- dialog.  For menu bars, the assigned dialog is the dialog the menu bar is
  -- attached to.  For pop up menus, the menu could be assigned to a dialog each
  -- time the context menu is shown.  But, it is usually more convenient to just
  -- assign the menu to a dialog one time.  That is what the assignTo() method
  -- does.
  --
  -- For the list view pop up menu, we connect each menu item to its own method.
  -- There are menu methods to connect individual menu items.  But there are
  -- also a number of convenience methods to connect a number of menu items at
  -- one time.
  --
  -- The second optional arguement in assignTo() turns on automatic menu item
  -- connection.  The default is off.  The third optional argument is the name
  -- of a method to connect all the items to.  So, you can either connect each
  -- menu item to its own method, (omit the third argument,) or you can connect
  -- all menu items to one method.  When each menu item is connected to its own
  -- method, the method name is constructed by the ooDialog framework using the
  -- text of the menu item.
  --
  -- For example, if the menu item text is 'Blow your own horn' then the method
  -- name will be blowYourOwnHorn().
  --
  -- This is what we do here, assign the list view menu to this dialog, and at
  -- the same time connect all the menu item comand events to an individual
  -- method.
  if \ lvPopup~assignTo(self, .true) then do
    say 'Error conecting menu items. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end
  say

  -- Next we connect the context menu message (right mouse click) with two
  -- methods.  If you supply a window handle as the second, optional, parameter,
  -- then the event notifications are filtered by that window handle.  In this
  -- way you can easily show a context menu specific to a certain control.  If
  -- you do not use a window handle to filter the event notifications, you can
  -- still do the same thing, you just need to determine the mouse position at
  -- the time of the event and relate that position to the context menu you
  -- want to show
  --
  -- Here we send all right-clicks on the list-view control to one method and
  -- all clicks on the dialog to a second method.
  if \ lvPopup~connectContextMenu(onListViewContext, self~newListView(IDC_LV)~hwnd) then do
    say 'Error conecting context menu. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end

  -- If we only use the first connectContextMenu(), then right clicks on the Ok
  -- and cancel buttons are ignored.  The shift-F10 and VK_APPS events are
  -- also ignored when either of the buttons has the focus.
  --
  -- That's perfectly fine, but it depends on what you want.  Here, we pretend
  -- that we don't want that behavior, and show how to prevent it.  Simply add
  -- filtered connections for the Ok and Cancel push buttons.
  --
  -- You can play with this to see it works.  Comment out the third and forth
  -- connections and then tab to the Ok or Cancel key and use Shift-F10.  There
  -- will be no context menu.  Or right click on one of those buttons, no
  -- context menu.  Uncomment the connections and try the same thing.
  if \ dlgPopup~connectContextMenu(onDialogContext, self~hwnd, self) then do
    say 'Error conecting context menu. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end
  else if \ dlgPopup~connectContextMenu(onDialogContext, self~newPushButton(IDOK)~hwnd, self) then do
    say 'Error conecting context menu. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end
  else if \ dlgPopup~connectContextMenu(onDialogContext, self~newPushButton(IDCANCEL)~hwnd, self) then do
    say 'Error conecting context menu. SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end


/** onListViewContext()
 *
 * This is an event handler for the context menu event.
 *
 * The context menu event is generated when the user right-clicks the mouse,
 * types the shift-F10 key combination, or types the VK_APPS key.  In this
 * program, onListViewContext() is only generated when the mouse is right
 * clicked on the list view control, or if the list view has the input focus and
 * shift-F10 or VK_APPS are used.
 *
 * Look at the makeMenuConnections() method to see how the connection is made.
 *
 * Three arguments are passed to the event handler for the context menu event:
 * the window handle of the window clicked, (or that has the focus if it is a
 * keyboard event,) and x / y co-ordinates.  For a mouse click, the x / y co-
 * ordinates are the position of the mouse, in pixels.  For the keyboard keys, x
 * and y are both -1.
 *
 * When the x and y arguments are -1, we need to decide where to place the
 * context menu when it is shown.  This is a design decision, there is no real
 * right answer.  A normal thing to do here would be to place it at the selected
 * list view item.  Instead, I decided to place at the lower right corner of the
 * list view.  However, the list view has a vertical scroll bar which is within
 * the area of the list view and I didn't like the look of that.
 *
 * So, I decided to move it to the left of the scroll bar.  I still didn't like
 * that because of the shadowing, so I take that point and shift it up and over
 * 15 pixels.
 */
::method onListViewContext
  expose lvPopup listView
  use arg hwnd, x, y

  if x == -1, y == -1 then do
    -- The keyboard was used, not the mouse.  Position the context menu as
    -- described in the comments above.
    rect = listView~windowRect
    x = rect~right - .SM~cxVScroll + 15
    y = rect~bottom - 15
  end

  -- pos is the point on the screen, in pixels, to place the context menu.
  pos = .Point~new(x, y)

  -- We show the menu.  The second optional argument is the dialog to assign the
  -- the menu to, but we've already done that in makeMenuConnections().
  ret = lvPopup~show(pos)
  if ret == -1 then do
    say 'lvPopup~show() failed SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end


/** onDialogContext()
 *
 * This is an event handler for the context menu event.
 *
 * The comments for onListViewContext() above apply here.  This method is
 * invoked when the user rigth clicks anywhere but the list-view, or for the
 * keyboard context menu event, whenever the list view does not have the focus.
 *
 * Here we also need to decide where to position the context menu for the
 * keyboard event.  I decided to place it in the rectangluar space between the
 * rigth edge of the list view and the right edge of the dialog, in the center.
 * I don't count the dialog caption bar as part of that space.
 */
::method onDialogContext
  expose dlgPopup listView
  use arg hwnd, x, y

  if x == -1, y == -1 then do
    -- The keyboard was used, not the mouse.  Position the context menu as
    -- described in the comments above.
    lvR = listView~windowRect
    dlgR = self~windowRect

    xOffset = (dlgR~right - lvR~right) % 2
    yOffset = (dlgR~bottom - (dlgR~top + .SM~cyCaption)) % 2

    x = lvR~right + xOffset
    y = dlgR~bottom - yOffset
  end

  -- This is the point where the context menu is positioned.
  pos = .Point~new(x, y)

  -- In contrast to the list view pop up menu, the dialog pop up menu has never
  -- been assigned to a dialog.  Therefore, each time we show it, we need to
  -- specify the dialog owner.  Which is done in the second arguemnt.
  ret = dlgPopup~show(pos, self)
  if ret == -1 then do
    say 'dlgPopup~show() failed SystemErrorCode:' || -
    .SystemErrorCode SysGetErrortext(.SystemErrorCode)
  end

/** onMenuCommand()
 *
 * This is the event handler for any menu item selected in the dialog pop up
 * menu.  We simply determine which menu item was selected through the id
 * argument and take the appropriate action.
 */
::method onMenuCommand unguarded
  use arg id

  select
    when id == .constDir[IDM_RC_SHOW      ] then self~showMessage
    when id == .constDir[IDM_RC_ENABLE_LV ] then self~enableListView(.true)
    when id == .constDir[IDM_RC_DISABLE_LV] then self~enableListView(.false)
    when id == .constDir[IDM_RC_BEEP      ] then beep(330, 250)
    when id == .constDir[IDM_RC_QUIT      ] then return self~ok
  end
  -- End select


/** sort()
 *
 * The event handler when the Sort menu item is selected in the list view pop up
 * menu.
 */
::method sort
  expose sArray
  self~rearrangeItems(sArray)

/** jumble()
 *
 * The event handler when the Jumble menu item is selected in the list view pop
 * up menu.
 *
 * Here we re-order the list view items in a somewhat random order.
 */
::method jumble
  expose nArray

  tempArray = .array~new(11)

  indexes = .set~new
  do while indexes~items < 11
    count = indexes~items
    do while indexes~items == count
      i = random(1, 11)
      if \ indexes~hasItem(i) then do
        tempArray~append(nArray[i])
        indexes~put(i)
      end
    end
  end

  self~rearrangeItems(tempArray)

/** reverseSort()
 *
 * The event handler when the Reverse Sort menu item is selected in the list
 * view pop up menu.
 */
::method reverseSort
  expose rsArray
  self~rearrangeItems(rsArray)

/** orderByFirstName()
 *
 * The event handler when the Order By First Name menu item is selected in the
 * list view pop up menu.
 */
::method orderByFirstName
  expose fnArray
  self~rearrangeItems(fnArray)

/** orderByLastName()
 *
 * The event handler when the Order By Last Name menu item is selected in the
 * list view pop up menu.
 */
::method orderByLastName
  expose lnArray
  self~rearrangeItems(lnArray)

/** orderByProfession()
 *
 * The event handler when the Order By Profession menu item is selected in the
 * list view pop up menu.
 */
::method orderByProfession
  expose profArray
  self~rearrangeItems(profArray)

/** restoreOriginalOrder()
 *
 * The event handler when the Restore Original Order menu item is selected in
 * the list view pop up menu.
 */
::method restoreOriginalOrder
  expose nArray
  self~rearrangeItems(nArray)

/** rearrangeItems()
 *
 * Places the list view items in the order specified.  As explained elsewhere,
 * the purpose of this example program is to show how context menus work, not
 * to show how list view controls work.
 *
 * Therefore we don't actually sort the list view items, we just use some pre-
 * ordered arrays of the items.  To change the order, we delete all the existing
 * items and replace them using the an array of the same items in a different
 * order.  The array is passed in here as the 'a' argument.
 *
 * The disable, delete, prepare ... enable redraw sequence seems, to me, to
 * reduce flicker in the list view.  However, there are not enough items in the
 * list view to tell for sure.  An interesting experiment would be to try it
 * with several hundreds of items.
 */
::method rearrangeItems private
  expose listView
  use strict arg a

  listView~disable
  listView~deleteAll
  listView~prepare4nItems(11)

  do r over a
    listView~addRow( , , r~fName, r~lName, r~profession)
  end

  listView~enable
  listView~redrawItems


/** showMessage()
 *
 * Helper method.  Shows a simple message to the user in response to the Show
 * Message menu item.
 */
::method showMessage private

  msg = "This is a good example of context menus."
  z = MessageDialog(msg, self~hwnd, "Context Menu Message", "OK", "INFORMATION")

/** enableListView()
 *
 * Helper method.  Enables or disables the list view control in response to the
 * Enable List View or Disable List View menu items.
 *
 * When the list view control is enabled, we disable the Enable List View menu
 * item and enable the Disable List View menu item.  When the list view is
 * disabled we do the reverse.  Note that when the list view is disabled, it is
 * no longer possible for the user to trigger the list view context menu.
 */
::method enableListView private
  expose listView dlgPopup
  use strict arg enable

  if enable then do
    dlgPopup~disable(IDM_RC_ENABLE_LV)
    dlgPopup~enable(IDM_RC_DISABLE_LV)
    listView~enable
  end
  else do
    dlgPopup~disable(IDM_RC_DISABLE_LV)
    dlgPopup~enable(IDM_RC_ENABLE_LV)
    listView~disable
  end

/** initListView()
 *
 * Initializes the list view by adding the columns, setting the extended list
 * view style, and adding the list view items.
 */
::method initListView private
  expose nArray listView

  listView = self~newListView(IDC_LV)

  listView~addExtendedStyle("FULLROWSELECT DOUBLEBUFFER GRIDLINES")

  listView~insertColumnPX(1, 'First Name', 68)
  listView~insertColumnPX(2, 'Last Name', 70)
  listView~insertColumnPX(3, 'Profession', 102)

  do r over nArray
    listView~addRow( , , r~fName, r~lName, r~profession)
  end

/** createArrays()
 *
 * The purpose of this example program is to demonstrate how context menus work,
 * not how list view controls work.  Rather than actually sort the list view
 * items, we just produce some pre-sorted arrays to use.
 *
 * The n, ln, fn, etc., prefixes stand for normal, first name, last name,
 * profession, sorted, and reverse sorted.
 */
::method createArrays private
  expose nArray lnArray fnArray profArray sArray rsArray

  nArray    = .array~new(11)
  lnArray   = .array~new(11)
  fnArray   = .array~new(11)
  profArray = .array~new(11)
  sArray    = .array~new(11)
  rsArray   = .array~new(11)

  nArray[1 ] = .directory~new~~put('Mark',     fName)~~put('Thompson',   lName)~~put('Mechanic', profession)
  nArray[2 ] = .directory~new~~put('Sue',      fName)~~put('Lamont',     lName)~~put('Nurse', profession)
  nArray[3 ] = .directory~new~~put('Mary',     fName)~~put('Greensmith', lName)~~put('CEO', profession)
  nArray[4 ] = .directory~new~~put('Susan',    fName)~~put('Dunn',       lName)~~put('Stock broker', profession)
  nArray[5 ] = .directory~new~~put('Hank',     fName)~~put('Miller',     lName)~~put('Librarian', profession)
  nArray[6 ] = .directory~new~~put('Larry',    fName)~~put('Williams',   lName)~~put('Doctor', profession)
  nArray[7 ] = .directory~new~~put('Alice',    fName)~~put('Toklas',     lName)~~put('Dentist', profession)
  nArray[8 ] = .directory~new~~put('Bill',     fName)~~put('Maher',      lName)~~put('Lawyer', profession)
  nArray[9 ] = .directory~new~~put('Vail',     fName)~~put('Miesfeld',   lName)~~put('Software Engineer', profession)
  nArray[10] = .directory~new~~put('Eric',     fName)~~put('Clapton',    lName)~~put('Cook', profession)
  nArray[11] = .directory~new~~put('Christin', fName)~~put('Henesy',     lName)~~put('Detective', profession)

  lnArray[1 ] = .directory~new~~put('Eric',     fName)~~put('Clapton',    lName)~~put('Cook', profession)
  lnArray[2 ] = .directory~new~~put('Susan',    fName)~~put('Dunn',       lName)~~put('Stock broker', profession)
  lnArray[3 ] = .directory~new~~put('Mary',     fName)~~put('Greensmith', lName)~~put('CEO', profession)
  lnArray[4 ] = .directory~new~~put('Christin', fName)~~put('Henesy',     lName)~~put('Detective', profession)
  lnArray[5 ] = .directory~new~~put('Sue',      fName)~~put('Lamont',     lName)~~put('Nurse', profession)
  lnArray[6 ] = .directory~new~~put('Bill',     fName)~~put('Maher',      lName)~~put('Lawyer', profession)
  lnArray[7 ] = .directory~new~~put('Vail',     fName)~~put('Miesfeld',   lName)~~put('Software Engineer', profession)
  lnArray[8 ] = .directory~new~~put('Hank',     fName)~~put('Miller',     lName)~~put('Librarian', profession)
  lnArray[9 ] = .directory~new~~put('Mark',     fName)~~put('Thompson',   lName)~~put('Mechanic', profession)
  lnArray[10] = .directory~new~~put('Alice',    fName)~~put('Toklas',     lName)~~put('Dentist', profession)
  lnArray[11] = .directory~new~~put('Larry',    fName)~~put('Williams',   lName)~~put('Doctor', profession)

  fnArray[1 ] = .directory~new~~put('Alice',    fName)~~put('Toklas',     lName)~~put('Dentist', profession)
  fnArray[2 ] = .directory~new~~put('Bill',     fName)~~put('Maher',      lName)~~put('Lawyer', profession)
  fnArray[3 ] = .directory~new~~put('Christin', fName)~~put('Henesy',     lName)~~put('Detective', profession)
  fnArray[4 ] = .directory~new~~put('Eric',     fName)~~put('Clapton',    lName)~~put('Cook', profession)
  fnArray[5 ] = .directory~new~~put('Hank',     fName)~~put('Miller',     lName)~~put('Librarian', profession)
  fnArray[6 ] = .directory~new~~put('Larry',    fName)~~put('Williams',   lName)~~put('Doctor', profession)
  fnArray[7 ] = .directory~new~~put('Mark',     fName)~~put('Thompson',   lName)~~put('Mechanic', profession)
  fnArray[8 ] = .directory~new~~put('Mary',     fName)~~put('Greensmith', lName)~~put('CEO', profession)
  fnArray[9 ] = .directory~new~~put('Sue',      fName)~~put('Lamont',     lName)~~put('Nurse', profession)
  fnArray[10] = .directory~new~~put('Susan',    fName)~~put('Dunn',       lName)~~put('Stock broker', profession)
  fnArray[11] = .directory~new~~put('Vail',     fName)~~put('Miesfeld',   lName)~~put('Software Engineer', profession)

  profArray[1 ] = .directory~new~~put('Mary',     fName)~~put('Greensmith', lName)~~put('CEO', profession)
  profArray[2 ] = .directory~new~~put('Eric',     fName)~~put('Clapton',    lName)~~put('Cook', profession)
  profArray[3 ] = .directory~new~~put('Alice',    fName)~~put('Toklas',     lName)~~put('Dentist', rprofession)
  profArray[4 ] = .directory~new~~put('Christin', fName)~~put('Henesy',     lName)~~put('Detective', profession)
  profArray[5 ] = .directory~new~~put('Larry',    fName)~~put('Williams',   lName)~~put('Doctor', profession)
  profArray[6 ] = .directory~new~~put('Bill',     fName)~~put('Maher',      lName)~~put('Lawyer', profession)
  profArray[7 ] = .directory~new~~put('Hank',     fName)~~put('Miller',     lName)~~put('Librarian', profession)
  profArray[8 ] = .directory~new~~put('Mark',     fName)~~put('Thompson',   lName)~~put('Mechanic', profession)
  profArray[9 ] = .directory~new~~put('Sue',      fName)~~put('Lamont',     lName)~~put('Nurse', profession)
  profArray[10] = .directory~new~~put('Vail',     fName)~~put('Miesfeld',   lName)~~put('Software Engineer', profession)
  profArray[11] = .directory~new~~put('Susan',    fName)~~put('Dunn',       lName)~~put('Stock broker', profession)

  -- Sorted array is same as fnArray
  sArray[1 ] = .directory~new~~put('Alice',    fName)~~put('Toklas',     lName)~~put('Dentist', profession)
  sArray[2 ] = .directory~new~~put('Bill',     fName)~~put('Maher',      lName)~~put('Lawyer', profession)
  sArray[3 ] = .directory~new~~put('Christin', fName)~~put('Henesy',     lName)~~put('Detective', profession)
  sArray[4 ] = .directory~new~~put('Eric',     fName)~~put('Clapton',    lName)~~put('Cook', profession)
  sArray[5 ] = .directory~new~~put('Hank',     fName)~~put('Miller',     lName)~~put('Librarian', profession)
  sArray[6 ] = .directory~new~~put('Larry',    fName)~~put('Williams',   lName)~~put('Doctor', profession)
  sArray[7 ] = .directory~new~~put('Mark',     fName)~~put('Thompson',   lName)~~put('Mechanic', profession)
  sArray[8 ] = .directory~new~~put('Mary',     fName)~~put('Greensmith', lName)~~put('CEO', profession)
  sArray[9 ] = .directory~new~~put('Sue',      fName)~~put('Lamont',     lName)~~put('Nurse', profession)
  sArray[10] = .directory~new~~put('Susan',    fName)~~put('Dunn',       lName)~~put('Stock broker', profession)
  sArray[11] = .directory~new~~put('Vail',     fName)~~put('Miesfeld',   lName)~~put('Software Engineer', profession)

  -- Just reverse the array indexes
  rsArray[11] = .directory~new~~put('Alice',    fName)~~put('Toklas',     lName)~~put('Dentist', profession)
  rsArray[10] = .directory~new~~put('Bill',     fName)~~put('Maher',      lName)~~put('Lawyer', profession)
  rsArray[9 ] = .directory~new~~put('Christin', fName)~~put('Henesy',     lName)~~put('Detective', profession)
  rsArray[8 ] = .directory~new~~put('Eric',     fName)~~put('Clapton',    lName)~~put('Cook', profession)
  rsArray[7 ] = .directory~new~~put('Hank',     fName)~~put('Miller',     lName)~~put('Librarian', profession)
  rsArray[6 ] = .directory~new~~put('Larry',    fName)~~put('Williams',   lName)~~put('Doctor', profession)
  rsArray[5 ] = .directory~new~~put('Mark',     fName)~~put('Thompson',   lName)~~put('Mechanic', profession)
  rsArray[4 ] = .directory~new~~put('Mary',     fName)~~put('Greensmith', lName)~~put('CEO', profession)
  rsArray[3 ] = .directory~new~~put('Sue',      fName)~~put('Lamont',     lName)~~put('Nurse', profession)
  rsArray[2 ] = .directory~new~~put('Susan',    fName)~~put('Dunn',       lName)~~put('Stock broker', profession)
  rsArray[1 ] = .directory~new~~put('Vail',     fName)~~put('Miesfeld',   lName)~~put('Software Engineer', profession)


-- The putMenu() method simply provides a convenient way to pass in the menu
-- object created in the program entry code to this dialog.
::method putMenu
  expose lvPopup
  use strict arg lvPopup


/** routine::createListViewMenu()
 *
 * This routine creates the list view context menu dynamically using methods of
 * the .PopupMenu.  It is very easy to use.  Menu items can be inserted or
 * removed from a menu at any time.
 *
 * The other point that is being made by creating this menu in a routine is that
 * menu objects are truely independent of a dialog.  A menu object can be
 * created without any existing dialogs.  The object can then be used with any
 * dialog.
 */
::routine createListViewMenu
  use strict arg

  -- Create a new empty pop up menu.
  m = .PopupMenu~new(IDM_LV_BAR)

  -- Insert the menu items.  The first argument is the resource ID of the item
  -- before which the inserted item goes.  The second argument is the resource
  -- ID of the item to insert.
  --
  -- For the first item, since there are no items in the menu, the insert before
  -- argument does not matter.  I ususally just use the same resource ID as the
  -- item being inserted.  You could use any positive number.
  --
  -- For the rest of the items, here they are just sort of in a random order.
  -- This is to demonstrate how the 'insert before' works.  I usually insert the
  -- items in a last comes first order because it makes it a little easy to
  -- visualize how the menu will look, if you picture the menu upside down.
  m~insertItem(IDM_LV_RESTORE, IDM_LV_RESTORE, "Restore Original Order")
  m~insertItem(IDM_LV_RESTORE, IDM_LV_PROFESSION, "Order by Profession")
  m~insertItem(IDM_LV_PROFESSION, IDM_LV_FNAME, "Order by First Name")

  m~insertItem(IDM_LV_FNAME, IDM_LV_REVERSE, "Reverse Sort")
  m~insertItem(IDM_LV_REVERSE, IDM_LV_SORT, "Sort")
  m~insertItem(IDM_LV_REVERSE, IDM_LV_JUMBLE, "Jumble")

  m~insertItem(IDM_LV_PROFESSION, IDM_LV_LNAME, "Order by Last Name")

  m~insertSeparator(IDM_LV_FNAME, IDM_LV_SEP1)
  m~insertSeparator(IDM_LV_RESTORE, IDM_LV_SEP2)

  return m


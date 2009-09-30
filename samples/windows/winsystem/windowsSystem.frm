/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2008 Rexx Language Association. All rights reserved.    */
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
 *   Name: windowsSystem.frm
 *   Type: Framework
 *
 *   Description:  A collection of public routines and classes to help work with
 *                 the winsystm.cls package.
 *
 *                 This is an example of how to extract common function into a
 *                 package, and the use the package to help in writting similar
 *                 programs.  A number of the sample programs that use
 *                 winsystm.cls make use of this framework.
 */

::requires 'winsystm.cls'
::requires "ooDialog.cls"

/** findTheWindow()
 * Uses the WindowsManager to create a WindowObject representing a top-level
 * window currently running on the system.
 *
 * When an application is started up, it takes some finite amount of time before
 * the window for the application is created by the operating system.  The
 * WindowsManager can not 'find' the aplication window until it has been
 * created.  This function loops a number of times trying to find the window,
 * then eventually gives up
 *
 * How long to wait for to find the window before giving up can be adjusted by
 * the caller.
 *
 * @param   title  The title of the window being sought.
 * @param   loops  Optional.  The number of loops to perform looking for the
 *                 window.  The default is 20.
 * @param   pause  Optional.  The time to sleep during each loop.  The default
 *                 is .2 of a second.
 *
 * @return         A WindowObject object representing the desired window on
 *                 success.  If the function fails, .nil is returned.
 *
 * @note           To find a window, you must use its exact title.
 */
::routine findTheWindow public
  use strict arg title, loops = 20, pause = .2

  windowMgr = .WindowsManager~new

  do loops
    j = SysSleep(pause)
    wnd = windowMgr~find(title)
    if wnd~class == .WindowObject then leave
  end

return wnd

/** sendTextWithPause()
 * Sends text to a window and pauses slightly before returning.
 *
 * @param  wnd   A window object that represents the window to send the text to.
 * @param  text  The text to send.
 *
 * @return  The return from SysSleep()
 *
 * @note  Not to go into too much detail about the underlying implmentation of
 *        winsystm.cls, but a brief explantion.  When sending text or a key
 *        press to a window, a Windows API function is used that returns
 *        immediately, before the text or key press actually makes it to
 *        the OS window.  In a fast dual-core / multi-core / multi-processor
 *        system this can result in the window receiving the key presses or text
 *        in a different order than they are sent by ooRexx.
 *
 *        This function and the sendKeyWithPause() function prevent that out of
 *        ordering by pausing very slightly after doing the send.  This actually
 *        mimics more closely a user entering the data on a keyboard.  Anyone
 *        familiar with Expect, probably knows that Expect provides a similiar
 *        function.
 */
::routine sendTextWithPause public
  use strict arg wnd, text
  wnd~sendText(text)
return SysSleep(.01)

/** sendKeyWithPause()
 * Sends a key press to a window and pauses slightly before returning.
 *
 * @param  wnd   A window object that represents the window to send the key
 *               press to.
 * @param  key   The key press to send.
 *
 * @return  The return from SysSleep()
 *
 * @see  sendTextWithPause()
 */
::routine sendKeyWithPause public
  use strict arg wnd, key
  wnd~sendKey(key)
return SysSleep(.01)

/** printChildren()
 * Given a window object, enumerates all descendent windows of that window and
 * prints the results to the screen.
 *
 * @param wnd     The parent window object.
 * @param indent  When printing to the console, the amount of indentation.
 *
 * @return  True if this function recursed, otherwise false.
 */
::routine printChildren public
  use strict arg wnd, indent

  line = indent || wnd~hwnd ":" wnd~id ":" wnd~wclass ":" wnd~title~left(15) ":" wnd~coordinates

  childs. = wnd~enumerateChildren

  select
    when childs.0 == 0 then do
      line = line "no children"
      say line
      return .false
    end

    when childs.0 == 1 then do
      line = line "1 child"
    end

    otherwise do
      line = line childs.0 "children"
    end
  end
  -- End select

  say line

  indent = indent || "  "

  do i = 1 to childs.0
    childWnd = .WindowObject~new(childs.i.!Handle)
    ret = printChildren(childWnd, indent)
  end

return .true

/** findDescendent()
 * Finds a descendent window with the window title specified, if any.
 *
 * @param  wnd   The parent window whose descendents are searched.
 * @param  title The title (label) of the window being searched for.
 *
 * @return  A .WindowObject representing the window if found.  If not found then
 *          .nil is returned.
 *
 * @note  This function is very similar to .WindowObject~findChild(), except
 *        that this function will search recursively through the window
 *        hierarchy for any descendent window with the specified label.  The
 *        WindowObject's findChild() on the other hand only looks at the
 *        immediate children.
 */
::routine findDescendent public
  use strict arg wnd, title

  descendent = wnd~findChild(title)
  if descendent \== .nil then return descendent

  children. = wnd~enumerateChildren

  do i = 1 to children.0
    childWnd = .WindowObject~new(children.i.!Handle)

    descendent = findDescendent(childWnd, title)
    if descendent \== .nil then return descendent
  end

return .nil

/** isVistaOrLater()
 * Simple convenience function to determine if the operating system is at least
 * Windows Vista.
 *
 * @return True if the current OS is Vista or a later version of Windows,
 *         otherwise false.
 */
::routine isVistaOrLater public
  parse value SysVersion() with name ver
  return (ver >= 6)


/** getPathToSystemExe()
 * Gets the complete path to an executable that is a standard application
 * shipped with a Windows system.  In a Windows distributions, these
 * applications are in the system directory.  Getting the complete path ensures
 * that the correct application is started.
 *
 * For instance, there are a surprisingly large number of applications named
 * 'calc.'  If a user has set up her system with a 'calc' program in a directory
 * ahead of the system directory in the path, then that calc program can end up
 * being started rather than the expected Windows calculator application.
 *
 * @param  prgName  The program whose path is being sought.  The .exe is
 *                  expected.
 *
 * @return  The fully qualified path name of the program, if it exists in the
 *          system directory.  If not found, then .nil is returned.
 */
::routine getPathToSystemExe public
  use strict arg prgName

  shell = .oleObject~new('Shell.Application')

  -- 0x25 is the CSIDL_SYSTEM constant.  The CSIDL_XXX constants are used to
  -- identify well known directories on Windows.  Like the My Documents
  -- directory, the All Users Start Menu, etc.  We use this constant to get the
  -- Windows System folder object, and from that the path to the directory.
  csidl_system = '25'

  folderObj = shell~nameSpace(csidl_system~x2d)
  sysFolderPath = folderObj~self~path
  if sysFolderPath~right(1) \== '\' then sysFolderPath = sysFolderPath'\'

  prgPathName = sysFolderPath || prgName
  if \ SysIsFile(prgPathName) then prgPathName = .nil

return prgPathName

/** getWindowTree()
 * Constructs a data structure that represents the entire window hierarchy of
 * a window.
 *
 * Each node in the structure represents a window.  The node is a .directory
 * object whose items contain the important attributes of the window.  This
 * directory object has an item: children, that is an array of nodes
 * representing the children windows of the node.
 *
 * Each node has these indexes:
 *
 * node~handle
 * node~title
 * node~windowClass
 * node~state
 * node~id
 * node~coordinates
 * node~children
 * node~hasChildren
 * node~childrenCount
 *
 * @param  wnd  The window object to construct a node for.
 *
 * @return  A tree of window nodes representing the window specified and all its
 *          children.
 *
 * @note  This is a recursive function.
 */
::routine getWindowTree public
  use strict arg wnd

  tree = buildWindowDetails(wnd)
  child = wnd~firstChild
  if child == .nil then return tree

  tree~children = .array~new
  tree~hasChildren = .true
  do while child \== .nil
    tree~children~append(getWindowTree(child))
    tree~childrenCount += 1
    child = child~next
  end

return tree

/** buildWindowDetails()
 * Private helper function for getWindowTree().
 *
 * @param  wnd  The .WindowObject whose details are desired.
 *
 * @return  A directory object whose items reflect a number of attributes of
 *          a window.
 */
::routine buildWindowDetails
  use strict arg wnd

  d = .directory~new
  d~handle = wnd~handle
  d~text = wnd~title
  d~windowClass = wnd~wClass
  d~state = wnd~state
  d~id = wnd~id
  d~coordinates = wnd~coordinates
  d~children = .nil
  d~hasChildren = .false
  d~childrenCount = 0

return d

/** printWindowTree
 * Given a tree of window nodes, prints out the tree to the console.
 *
 * @param  tree    The tree of window nodes to print.
 * @param  indent  The current indentation.  Note that this is a recursive
 *                 function and the indent is increased with each recursion.
 *
 * @return  True if the function has recursed, othewise false.
 *
 * @see getWindowTree()
 * @see printChildren()
 *
 * @note  The function produces the exact same output as printChildren().  Its
 *        main purpose was to debug getWindowTree().  However, it could serve
 *        as a useful template if one wanted to change how the information or
 *        what information is printed to the console.
 */
::routine printWindowTree public
  use strict arg tree, indent

  -- A little defensive programming.
  if \ tree~isA(.directory) then return .false

  line = indent || tree~handle ":" tree~id ":" tree~windowClass ":" tree~text~left(15) ":" tree~coordinates

  select
    when tree~childrenCount == 0 then do
      line = line "no children"
      say line
      return .false
    end

    when tree~childrenCount == 1 then do
      line = line "1 child"
    end

    otherwise do
      line = line tree~childrenCount "children"
    end
  end
  -- End select

  say line

  indent = indent || "  "

  do child over tree~children
    ret = printWindowTree(child, indent)
  end

return .true

/** showWindowTree()
 * Puts up an ooDialog dialog that displays the window hierarchy of a window in
 * a tree view control.
 *
 * @param  tree  A tree node data structure produced by the getWindowTree()
 *               function.
 *
 * @return True if the dialog was shown, false if it was not.
 *
 * @see getWindowTree()
 */
::routine showWindowTree public
  use strict arg tree

  dlg = .WindowTreeDlg~new("winSystemDlgs.rc", IDD_WINDOW_TREE, , "winSystemDlgs.h")
  if dlg~initCode == 0 then do
    dlg~useTree(tree)
    dlg~execute("SHOWTOP")
    dlg~deinstall
    return .true
  end

return .false

/** class: WindowTreeDlg
 *
 * A simple ooDialog dialog class to display the window hierarchy, parent and
 * descendent windows, of any window.
 *
 * The only caveat is that the class requires a window tree structure created by
 * the getWindowTree() public routine in this framework.
 *
 * This is a subclass of RcDialog, meaning the dialog template is defined in a
 * resource script file.  The dialog template was created by a GUI dialog
 * editor.  The dialog template is stored in the winSystemDlgs.rc file and has
 * a symbolic resource ID of IDD_WINDOW_TREE.  The symbolic resource IDs are
 * defined in the winSystemDlgs.h file.
 *
 * @note For an example of how to use this class see the showWindowTree()
 *       public routine in this framework.
 */
::class 'WindowTreeDlg' public subclass RcDialog inherit AdvancedControls MessageExtensions

/** useTree()
 * Sets the window tree structure for this dialog.  The structure must be set
 * prior to executing the dialog, otherwise the dialog will display an empty
 * tree view control.
 *
 * @param  windowTree  A tree node structure in the same format as that produced
 *                     by the getWindowTree() function.
 @
 @ @see getWindowTree()
 */
::method useTree
  expose windowTree
  use arg windowTree

/** initDialog()
 * Initializes the dialog controls for this dialog.  The only control is the
 * Tree control.
 *
 * The ooDialog framework automatically inovkes this method for every dialog
 * immediately after the underlying Windows dialog has been created.  Since most
 * of the initialization of dialog controls requires that the underlying control
 * has been created, this makes initDialog() the proper place to do all the
 * control initialization.
 */
::method initDialog
  expose windowTree

  if windowTree~isA(.directory), windowTree~hasIndex("WINDOWCLASS") then self~doInit(windowTree)

/** doInit()
 * A private method that does the actuall work of initializing the tree control.
 *
 * @param windowTree  The window tree node structure for this dialog.
 */
::method doInit private
  use strict arg windowTree

  -- Get the tree-view control object and then invoke the addNode() recursive
  -- method to add all the items to the control.
  tree = self~newTreeView(IDC_TREE_WINDOWS)
  rootNode = self~addNode(tree, "Root", windowTree)

  -- Set the title of this dialog, which will contain the window handle for the
  -- window we represent.
  title = "The" windowTree~text "(" || windowTree~handle || ") Window Hierarchy"
  self~setTitle(title)

  -- Expand the root item in the tree-view control.
  tree~expand(rootNode)

/** addNode()
 * A private recursive function that adds each node to the tree control.  Each
 * node represents a single window in the hiearchy.
 *
 * @param  tree  The tree-view control object.
 *
 * @param  root  A reference to the current item in the tree-view control we
 *               are working with.  Note that this can also be the keyword: root
 *               which will signal that this is to be the initial item in the
 *               tree-view control.  Otherwise, it is a reference to an already
 *               created item in the tree-view control.
 *
 * @param  node  The current node in the window tree structure we are working
 *               with.
 */
::method addNode private
  use strict arg tree, root, node

  -- The text displayed for this item.
  text = node~handle || ":" node~text

  -- Insert (create) a new item in the tree-view control.  Insert it after the
  -- 'LAST' sub-item under the item referenced by root in the tree-view control.
  -- This item is the window item.
  newRoot = tree~insert(root, "LAST", text, , , , node~childrenCount)

  -- Insert sub-items.  Each of these sub-items is an attribute of the window
  tree~insert(newRoot, , "Text:" node~text)
  tree~insert(newRoot, , "Handle:" node~handle)
  tree~insert(newRoot, , "Class:" node~windowClass)
  tree~insert(newRoot, , "ID:" node~id)
  tree~insert(newRoot, , "Coordinates:" node~coordinates)
  tree~insert(newRoot, , "State:" node~state)

  -- Now, if the window has children windows, recursively add them to the
  -- tree-view control.
  if node~hasChildren then do n over node~children
    self~addNode(tree, newRoot, n)
  end

return newRoot

/** class: MenuDetailer
 *
 * A class to parse and display the details of a menu bar.
 *
 * In Windows, a menu bar is similar to a top level window in that they both
 * have a hierarchy of contained objects.  A top-level window can contain other
 * windows, which themselves can contain other windows.  A menu bar can contain
 * submenus, which can contain submenus.
 *
 * This hierarchy is easily displayed in a tree-like structure, for both windows
 * and menus.  The function of displaying a menu hierachy is therefore very
 * similar to the set of functions used to display a window hierarchy provided
 * in this framework.
 *
 * However, the MenuDetailer is a more object-orientated approach than that used
 * for displaying a window hierarchy.  The data and the means to manipulate the
 * data are all contained within a single object, the MenuDetailer object.
 *
 * Note that this is a subclass of ooDialog's RcDialog, but the class can be
 * used / useful without ever creating an underlying Windows dialog.  The
 * ooDialog part is only used in the display() method which produces a graphical
 * dislpay of the menu tree.
 */
::class 'MenuDetailer' public subclass RcDialog inherit AdvancedControls MessageExtensions

::method init
  use strict arg wnd
  self~newWindow(wnd)


/** newWindow()
 * This method is called to create a menu tree node structure that represents
 * the menu of the specified window.  It is used when a new MenuDetailer object
 * is created and when / if the user of the class sets a new window.
 *
 * @param  wnd  A window object whose menu is to be 'detailed.;
 */
::method newWindow private
  expose menubar maxTextLength textFormat mainWindow
  use strict arg wnd

  -- Ensure wnd is a WindowObject.
  if \ wnd~isA(.WindowObject) then raise syntax 93.948 array ("1 'wnd'", "WindowObject")

  -- Set / reset some state variables.
  maxTextLength = 0
  textFormat = .nil
  menubar = .nil
  mainWindow = wnd

  -- Create the menu tree node structure.
  self~createMenubar

/** createMenubar()
 * Creates a menu tree node structure that represents the menu of the main
 * window.
 *
 * Conceptually, a menu consists of a container with some number of menu items,
 * where each menu item can, possibly, be a submenu.  Typically in Windows, the
 * top-level container is called a menubar.  A tree is a natural data
 * structure to represent this.
 *
 * The menu tree node structure consists of a directory that represents the
 * menubar and has this structure:
 *
 *   menubar~ownerHwnd
 *   menubar~ownerTitle
 *   menubar~itemCount
 *   menubar~menuItems  an array of menu items where each menu item is a
 *                      directory.
 *
 *     menuitem~pos
 *     menuitem~id
 *     menuitem~text
 *     menuitem~isSubmenu
 *     menuitem~isSeparator
 *     menuitem~isTextItem
 *     menuitem~isChecked
 *     menuitem~itemCount
 *     menuitem~menuItems  an array of menu items
 *
 */
::method createMenubar private
  expose menubar mainWindow

  menubar = .directory~new
  menubar~ownerTitle = mainWindow~title
  menubar~ownerHwnd = mainWindow~hwnd

  menu = mainWindow~menu
  if menu == .nil then do
    menubar~itemCount = -1
    menubar~menuItems = .nil
  end
  else do
    menubar~itemCount = menu~items
    menubar~menuItems = self~populate(menu)
  end

/** populate()
 * A recursive function to create and return an array of menu items.
 *
 * @param  A MenuObject object whose menuitems will be used to create and
 *         populate an array.
 *
 * @return The populated array.
 *
 * @see createMenubar()
 */
::method populate private
  expose maxTextLength
  use strict arg menu

  maxTextLength = 0
  count = menu~items

  a = .array~new(count)
  do i = 0 to count - 1
    d = .directory~new
    d~pos = i
    d~id = menu~idOf(i)
    d~text = menu~textOf(i)
    d~isSubmenu = menu~isSubmenu(i)
    d~isSeparator = menu~isSeparator(i)
    d~isTextItem = \(d~isSeparator | d~isSubmenu)
    d~isChecked = menu~isChecked(i)

    if d~text~length > maxTextLength then maxTextLength = d~text~length

    if d~isSubmenu then do
      submenu = menu~submenu(i)
      d~itemCount = submenu~items
      d~menuItems = self~populate(submenu)
    end
    else do
      d~itemCount = 0
      d~menuItems = .nil
    end

    a[i + 1] = d
  end

return a

/** print()
 * Outputs a text representation of the menu to the console.
 */
::method print
  expose textFormat

  -- If not already created, construct an array of text lines that represents
  -- the menu.  Then output the lines to the console.
  if textFormat == .nil then self~toArray
  do l over textFormat
    say l
  end

/** getOutline()
 * Return the array of text lines representing the menu to the caller.  The
 * caller can then use the array as they see fit.
 */
::method getOutline
  expose textFormat

  -- Return a copy of the array to the caller so that an outsider can not
  -- unintentionally or intentionally change our internal data.
  if textFormat == .nil then self~toArray
  return textFormat~copy

/** getMenubar()
 * Return the menu tree node structure to the caller.  The caller can then
 * format and / or use the data as desired.
 */
::method getMenubar
  expose menubar

  -- Recreate the menubar if needed.
  if menubar == .nil then self~createMenubar

  -- Since ooRexx does not have a deep copy, we save a reference to the menuBar
  -- object, set our internal reference to the menubar to .nil, and return the
  -- saved reference.  This allows the caller to change the menubar without
  -- changing our internal representation.  If we need the menubar structure
  -- again, we will re-create it.
  populatedMenubar = menubar
  menubar = .nil

return populatedMenubar

/** setWindow()
 * Changes the main window to a new one.
 */
::method setWindow
  use strict arg wnd
  self~newWindow(wnd)

/** toArray()
 * Transforms the menu tree node structure that represents our menu into an
 * array of text lines that represents the menu.
 */
::method toArray private
  expose menubar textFormat

  -- Check if we need to recreated the menubar.
  if menubar == .nil then self~createMenubar

  textFormat = .array~new

  -- The first line will show data about the main window.  It is possible that
  -- the window will were initialized with does not have a menu.  In addition,
  -- many applications now create their menus dynamically.  Those types of menus
  -- may not contain any menu items until the user actually selects a menu item.
  -- For these two cases we return immediately.

  line = menubar~ownerTitle "(" || menubar~ownerHwnd || ")"
  if menubar~itemCount == -1 then do
    textFormat[1] = line "does not have a menu"
    return textFormat
  end
  else if menubar~itemCount == 0 then do
    textFormat[1] = line "menu is not populated"
    return textFormat
  end

  -- Add the first line, then recursively add a line for every menu item.
  textFormat[1] = line
  do item over menubar~menuItems
    self~addArrayItem(item, "  ")
  end

/** addArrayItem()
 * A recursive method that adds a line of text for every menu item to an array
 * of lines.
 *
 * @param  item    A node in the menu tree node structure representing a menu
 *                 item.
 *
 * @param  indent  A string of spaces, the length of which shows the depth of
 *                 the menu item within the overall menu.
 */
::method addArrayItem private
  expose maxTextLength textFormat
  use strict arg item, indent

  -- Build up a line of text for the menu item
  line = indent
  sep = '-'~copies(maxTextLength)
  pos = item~pos~right(2) || ':'
  id = '[id:' item~id~right(3) || ']'

  select
    when item~isSubmenu then do
      line = line || pos id 'popup menu' item~text~left(maxTextLength)
    end
    when item~isSeparator then do
      line = line || pos id 'separator ' sep
    end
    otherwise do
      line = line || pos id 'menu item ' item~text~left(maxTextLength)
      if item~isChecked then line = line '[checked]'
    end
  end
  -- End select

  -- Add the line to our array of lines.
  textFormat~append(line)

  -- If item is itself a menu, recursively add its menu items to the array.
  if item~isSubmenu then do
    -- Increase our indent by 2 spaces.
    indent = indent || "  "
    do menuItem over item~menuItems
      self~addArrayItem(menuItem, indent)
    end
  end

/** display()
 * Displays our menu in a graphical format using a tree control in a dialog.
 */
::method display

  -- Initialize our ooDialog superclass.  The dialog template is stored in the
  -- resource script file: winSystemDlgs.rec with a symbolic ID of IDD_MENU_TREE
  -- and the symbolic IDs for the dialog are defined in winSystemDlgs.h

  self~init:super("winSystemDlgs.rc", IDD_MENU_TREE, , "winSystemDlgs.h")
  self~execute("SHOWTOP")
  self~deinstall

/** initDialog()
 * Initializes the dialog controls for this dialog.  The only control is the
 * Tree control.
 *
 * The ooDialog framework automatically inovkes this method for every dialog
 * immediately after the underlying Windows dialog has been created.  Since most
 * of the initialization of dialog controls requires that the underlying control
 * has been created, this makes initDialog() the proper place to do all the
 * control initialization.
 *
 * @see WindowTreeDlg::initDialog()
 */
::method initDialog
  expose menubar

  -- Check if we need to recreate the menubar.
  if menubar == .nil then self~createMenubar

  -- Set the dialog title to a subset of what will be the text for the first
  -- item in the tree-view control.
  rootText = menubar~ownerTitle "(" || menubar~ownerHwnd || ")"
  self~setTitle(rootText "Menu Hierarchy")

  -- The tree-view control is populated in a similar manner as was the tree-view
  -- control in the WindowTreeDlg class in this framework.  See that class for
  -- comment if needed.

  -- Get the tree-view object, set the first item, expand it.
  tree = self~newTreeView(IDC_TREE_MENUS)

  if menubar~itemCount == -1 then do
    rootText = rootText "[window does not have a menu]"
  end
  else if menubar~itemCount == 0 then do
    rootText = rootText "[menu is not populated]"
  end

  rootNode = tree~add(rootText, , , "EXPANDED")

  -- If the menubar has menu items, recursively add then to the tree-view
  -- control.
  if menubar~itemCount > 0 then do item over menubar~menuItems
    self~addNode(tree, rootNode, item)
  end

/** addNode()
 * Recursively add tree-view items for each menu item in our menu.
 *
 * @param  tree      The tree-view control object.
 *
 * @param  root      A reference to the current item in the tree-view control we
 *                   are working with.
 *
 * @param  menuItem  The current node in the menu tree structure we are working
 *                   with.
 *
 * @see WindowTreeDlg::initDialog()
 */
::method addNode private
  expose maxTextLength
  use strict arg tree, root, menuItem

  text = menuItem~pos~right(2) || ':'

  if menuItem~isSeparator then text = text '-'~copies(maxTextLength)
  else text = text menuItem~text

  if menuItem~isSubmenu then count = menuItem~itemCount
  else count = 0

  newRoot = tree~insert(root, "LAST", text, , , , count)

  tree~insert(newRoot, , "Text:" menuItem~text)
  tree~insert(newRoot, , "ID:" menuItem~id)
  tree~insert(newRoot, , "Submenu?" self~logicalToString(menuItem~isSubmenu))
  tree~insert(newRoot, , "Text Item?" self~logicalToString(menuItem~isTextItem))
  tree~insert(newRoot, , "Separator?" self~logicalToString(menuItem~isSeparator))
  tree~insert(newRoot, , "Checked?" self~logicalToString(menuItem~isChecked))

  if menuItem~isSubmenu then do mi over menuItem~menuItems
    self~addNode(tree, newRoot, mi)
  end

return newRoot

/** logicalToString()
 * Simple helper method to convert an ooRexx logical value to a string
 * representation.
 *
 * @param  logical  The value to convert.
 *
 * @return The string "true" if logical is strictly true, otherwise the string
 *         "false"
 */
::method logicalToString private
  use strict arg logical

  -- Work with anything sent to us.  This uses the short-cut AND operator.  Once
  -- a test fails, the rest of the tests are not evaluated.  If logical is a
  -- string object, and the data type of the string object is lOgical, and
  -- logical is true then return "true"  Othewise return "false"

  if logical~isA(.String), logical~datatype('O'), logical then return 'true'
  else return 'false'


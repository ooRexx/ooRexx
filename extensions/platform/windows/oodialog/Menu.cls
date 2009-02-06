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

/* Windows Dialog Interface for Open Object Rexx (ooRexx)
 *
 * Menu Classes
 *
 * Provides support for all types of Windows Menus.
 */


/** PLEASE NOTE:
 *
 * The classes and methods in this file are purposively NOT DOCUMENTED.  Do NOT
 * use these classes or methods because the classes, methods, and / or arugments
 * WILL CHANGE in the next release of ooRexx. If you do use these classes in
 * your own code, you do so at you own risk.
 *
 */

/* class: Menu - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A mix in class implementing functionality that is common to all menus.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'Menu' public mixinclass Object

::attribute hMenu set private
::attribute hMenu get

::attribute dlg private

::attribute initCode set private
::attribute initCode get

-- The resource ID of a menu.
::attribute wID set private
::attribute wID get

::attribute connectionRequested private
::attribute connectionQ private

::method menuInit private
  use strict arg symbolSrc

  self~hMenu = 0
  self~dlg = .nil
  self~initCode = 0
  self~wID = .nil

  self~connectionRequested = .false
  self~connectionQ = .queue~new

  select
    when symbolSrc~isA(.directory) then self~constDir = symbolSrc

    when symbolSrc~isA(.ResourceUtils) then self~constDir = symbolSrc~constDir

    when symbolSrc~isA(.string) then do
      self~ConstDir = .directory~new
      self~ParseIncludeFile(symbolSrc)
    end

    otherwise self~constDir = .directory~new
  end

  return 0

::method maybeResolveID private
  use strict arg id

  if id <> .nil then do
    id = self~resolveResourceID(id)
    if id = -1 then do
      self~initCode = id
      return .false
    end
    self~wID = id
  end

  return .true

/* getCount() returns the number of items in a menu.  The default is to get the
 * count of items in this menu, but passing in a raw menu handle will also work.
 */
::method getCount
  use arg obj

  select
    when arg(1, 'O') then return WinMenu("COUNT", self~hMenu)
    when self~isValidMenuHandle(obj) then return WinMenu("COUNT", obj)
    otherwise return -1
  end
  -- End select

/** isValidMenu()  Returns true if *this* Menu object represents a menu that the
 * operating system says is valid, otherwise false.
 */
::method isValidMenu
  return WinMenu("ISMENU", self~hMenu)

/** isValidMenuHandle()  Returns true if the specified handle represents a menu
 * that the operating system says is valid, otherwise false.
 */
::method isValidMenuHandle
  use strict arg handle
  return WinMenu("ISMENU", handle)

/** isSubMenu()  Returns true if the menu item specified is a menu item that
 * opens a submenu, othewise false.
 */
::method isSubMenu
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return .false

  return GetMII("ISSUB", self~hMenu, itemID, byPosition)

/** isCommandItem()  Returns true if the menu item specified is a command menu
 * item, i.e., not a menu item that opens a submenu or a separator, othewise
 * false.
 */
::method isCommandItem
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return .false

  return GetMII("ISITEM", self~hMenu, itemID, byPosition)

/** isSeparator()  Returns true if the menu item is a separator, otherwise false.
 * Note that with extended menus, it is possible to assign resource IDs even to
 * separators.  Therefore it is possible to query both by ID and position.
 */
::method isSeparator
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return .false

  return GetMII("ISSEP", self~hMenu, itemID, byPosition)

/** getHandle()  Returns the raw menu handle of this menu.
 */
::method getHandle
  return self~hMenu

/** getMenuHandle()  Get the raw menu handle of a submenu of this menu.
 */
::method getMenuHandle
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return -1

  return GetMII("SUB", self~hMenu, itemID, byPosition)

/** releaseMenuHandle()  Releases (frees) the operating system resources used by
 * a menu.  The menu is specified by its raw handle.  To release the operating
 * system resources for an ooDialog Menu object, use the destroy() method.
 */
::method releaseMenuHandle
  use strict arg handle
  return WinMenu("DEL", handle)

/** getPopup()  Returns a .PopupMenu that represents the specified submenu.
 * Check the PopupMenu initCode to determine success.
 */
::method getPopup unguarded
  use strict arg itemID, byPosition = .false, symbolSrc = .nil

  -- We want to *always* return a PopupMenu object, so even if there is an error
  -- with the itemID or with getMenuHandle(), we still pass the value on to
  -- PopupMenu~new.  .PopupMenu~init() can handle the incorrect values (if any)
  -- and will create the object with a non-zero initCode.

  itemID = self~resolveItemID(itemID, byPosition)

  -- This is the id to be passed on to PopupMenu~new.
  if byPosition then id = .nil
  else id = itemID

  hSubMenu = GetMII("SUB", self~hMenu, itemID, byPosition)

  return .PopupMenu~new(id, hSubMenu, symbolSrc)

::method insertPopup unguarded
  use strict arg before, popup, text, id, stateOpts = "", typeOpts = "", byPosition = .false

  id = self~resolveResourceID(id)
  if id == -1 then return -1

  before = self~resolveItemID(before, byPosition)
  if before == -1 then return -1

  if \ popup~isA(.PopupMenu) then return -2

  ret = InsertMII("POP", self~hMenu, before, byPosition, popup~getHandle, text, id, stateOpts~upper, typeOpts~upper)
  return self~maybeRedraw(ret)

::method insertItem unguarded
  use strict arg before, text, id, stateOpts = "", typeOpts = "", byPosition = .false, connect = .false

  id = self~resolveResourceID(id)
  if id == -1 then return -1

  before = self~resolveItemID(before, byPosition)
  if before == -1 then return -1

  ret = InsertMII("ITEM", self~hMenu, before, byPosition, text, id, stateOpts~upper, typeOpts~upper)

  if connect then do
    msgToRaise = text~changeStr('&', "")~space(0)
    if self~dlg <> .nil then do
      self~connectItem(id, msgToRaise, self~dlg)
    end
    else do
      self~connectionRequested = .true
      self~connectionQ~queue(.MapItem~new(id, msgToRaise))
    end
  end

  return self~maybeRedraw(ret)

::method insertSeparator unguarded
  use strict arg before, byPosition = .false

  before = self~resolveItemID(before, byPosition)
  if before == -1 then return -1

  ret = InsertMII("SEP", self~hMenu, before, byPosition)
  return self~maybeRedraw(ret)

::method maybeRedraw private
  use strict arg ret
  if ret <> 0 | self~dlg == .nil then return ret
  return WinMenu("DRAW", self~dlg~dlgHandle)

/** deleteSubMenu()
 * Removes the submenu from this menu and frees the hMenu.  To be able to reuse
 * the the submenu use the removeSubMenu() method.
 */
::method deleteSubMenu
  use strict arg id, byPosition = .false
  forward to (self~delete)

/** removeSubMenu()
 * Remove the submenu from this menu and return it, as a .PopupMenu to the
 * caller.
 */
::method removeSubMenu
  use strict arg id, byPosition = .false, symbolSrc = .nil

  subMenu = self~getPopup(id, byPosition, symbolSrc)
  if subMenu~initCode <> 0 then return subMenu

  self~delete(id, byPosition)
  return subMenu

/** removeItem() remove a menu item from this menu.
 */
::method removeItem
  use strict arg id, byPosition = .false
  forward to (self~delete)

::method removeSeparator
  use strict arg id, byPosition = .false
  forward to (self~delete)

::method delete private
  use strict arg id, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return .false

  ret = WinMenu("DELMENU", self~hMenu, itemID, byPosition)
  return self~maybeRedraw(ret)

::method isEnabled
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return .false

  return GetMII("ISTATE", self~hMenu, itemID, byPosition)~wordPos("ENABLED") <> 0

::method isGrayed
  use strict arg itemID, byPosition = .false
  forward to (self~isEnabled)

::method enable
  use strict arg itemIDs, byPosition = .false, enable = .true

  if itemIDs~isA(.Collection) then do
    status = 0
    do id over itemIDs
      ret = self~doEnable(id, byPosition, enable)
      if ret < 0, ret <> -1 then return ret
      if ret == -1 then status = -1
    end
  end
  else do
    status = self~doEnable(itemIDs, byPosition, enable)
  end

  return status

::method disable
  use strict arg itemID, byPosition = .false
  return self~enable(itemIDs, byPosition, .false)

::method gray
  use strict arg itemIDs, byPosition = .false
  return self~enable(itemIDs, byPosition, .false)

::method doEnable private
  use strict arg id, byPosition, enable

  id = self~resolveItemID(id, byPosition)
  if id == -1 then return -1

  keyword = "ENABLED"
  if \ enable then keyword = "DISABLED"

  return SetMII("ISTATE", self~hMenu, id, byPosition, keyword)

::method isChecked
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return .false

  return GetMII("ISTATE", self~hMenu, itemID, byPosition)~wordPos("CHECKED") <> 0

::method check
  use strict arg itemIDs, byPosition = .false, check = .true

  if itemIDs~isA(.Collection) then do
    status = 0
    do id over itemIDs
      ret = self~doCheck(id, byPosition, check)
      if ret < 0, ret <> -1 then return ret
      if ret == -1 then status = -1
    end
  end
  else do
    status = self~doCheck(itemIDs, byPosition, check)
  end

  return status

::method uncheck
  use strict arg itemIDs, byPosition = .false
  return self~check(itemIDs, byPosition, .false)

::method doCheck private
  use strict arg id, byPosition, check

  id = self~resolveItemID(id, byPosition)
  if id == -1 then return -1

  keyword = "CHECKED"
  if \ check then keyword = "UNCHECKED"

  return SetMII("ISTATE", self~hMenu, id, byPosition, keyword)

::method checkRadio
  use strict arg idStart, idEnd, idCheck, byPosition = .false
  idStart = self~resolveItemID(idStart, byPosition)
  idEnd   = self~resolveItemID(idEnd, byPosition)
  idCheck = self~resolveItemID(idCheck, byPosition)
  if (idStart == -1) | (idEnd == -1) | (idCheck == -1) then return -1

  return WinMenu("CHECKRADIO", self~hMenu, idStart, idEnd, idCheck, byPosition)

::method getState
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return -1

  return GetMII("ISTATE", self~hMenu, itemID, byPosition)

::method getType
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return -1

  return GetMII("TYPE", self~hMenu, itemID, byPosition)

/* Set or get a menu item's text */
::method getText
  use strict arg itemID, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return -1

  return GetMII("TEXT", self~hMenu, itemID, byPosition)

::method setText
  use strict arg itemID, text, byPosition = .false

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return -1

  return SetMII("TEXT", self~hMenu, itemID, byPosition, text)

/* getID()  Get an item ID, by definition this is by position.  If the
 * ID is already known, there is no point in querying for it.
 */
::method getID
  use strict arg posID
  return GetMII("ID", self~hMenu, posID, .true)

/* setID()  Set an item ID, by position is the defualt.  This function can
 * also be used to change an existing ID, so the by ID form is supported.  Note
 * that with extended menus all items can have IDs, even separators.
 */
::method setID
  use strict arg itemID, id, byPosition = .true

  itemID = self~resolveItemID(itemID, byPosition)
  if itemID == -1 then return -1

  id = self~resolveResourceID(id)
  if id == -1 then return -1

  return setMII("ID", self~hMenu, itemID, byPosition, id)

/* resolveItemID()  Convenience function to resolve an argument that can be
 * either a resource ID or a position ID.
 */
::method resolveItemID private
  use strict arg itemID, byPosition

  if byPosition then do
    if itemID~isA(.string), itemID~datatype('W'), itemID > -1 then return itemID
    else return -1
  end

  return self~resolveResourceID(itemID)


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Methods that affect an entire menu, rather than a menu item.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/* Set or get the menu help context ID */
::method setHelpID
  use strict arg id, recurse = .false  -- recurse means apply to submenus also.

  id = self~resolveResourceID(id)
  if id == -1 then return -1

  -- Be sure recurse is a boolean.
  if recurse \== .true, recurse \== .false then recurse = .true

  return SetMI("HELP", self~hMenu, id, recurse)

::method getHelpID
  return GetMI("HELP", self~hMenu)

::method destroy
  if self~isA(.MenuBar), self~dlg <> .nil, then self~detach
  ret = WinMenu("DEL", self~hMenu)
  self~hMenu = 0
  return ret

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Message connection methods.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::method checkConnectionRequest private
  use strict arg dlg

  if self~connectionRequested then do
    self~connectionRequested = .false
    do map over self~connectionQ
      self~connectItem(map~key, map~item, dlg)
    end
    self~connectionQ~empty
  end

/** connectItem()  Connects the menu command item with the specified ID to the
 * specified method.
 */
::method connectItem
  use strict arg id, msgToRaise, dlg = (self~dlg)

  id = self~resolveResourceID(id)
  if id == -1 then return -1

  if \ dlg~isA(.BaseDialog) then return -3

  if id < 3 | id = 9 then return 0

  ret = dlg~addUserMsg(msgToRaise, 0x00000111, 0xFFFFFFFF, id, 0x0000FFFF, 0, 0)
  if ret == 1 then return -4

  return 0

/** connectItemsToMsg()  Connects a collection of menu command items to the
 * single specified method.
 */
::method connectItemsToMsg
  use strict arg itemIDs, msgToRaise, dlg = (self~dlg)

  if \ itemIDs~isA(.Collection) then return -2

  status = 0
  do id over itemIDs
    ret = self~connectItem(id, msgToRaise, dlg)
    if ret < 0, ret <> -1 then return ret
    if ret == -1 then status = -1
  end

  return status

::method connectAllItems external "LIBRARY oodialog menu_connectAllItems"

-- Connect WM_CONTEXTMENU messages to a method.
::method connectContextMenu
  use strict arg msgToRaise, hwnd = .nil, dlg = (self~dlg)

  if \ dlg~isA(.BaseDialog) then return -3

  if hwnd \== .nil then
    return dlg~addUserMsg(msgToRaise, 0x0000007B, 0xFFFFFFFF, hwnd, 0xFFFFFFFF, 0, 0, 0x00000201)
  else
    return dlg~addUserMsg(msgToRaise, 0x0000007B, 0xFFFFFFFF, 0, 0, 0, 0, 0x00000201)

::method test external "LIBRARY oodialog menu_test"

/* class: MenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A mix in class implementing functionality that is common to all menu bars.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'MenuBar' public mixinclass Menu

/** attachTo()
 * Attaches this menu bar to the specified dialog window.  This method is not
 * intended to be used when this menu is already attached to a window.  Use the
 * replace method for that situation.
 *
 * However, a menu can only have one owner window.  So, if this menu is already
 * attached to a dialog, it is first detached.
 */
::method attachTo
  use strict arg dlg
  if \ dlg~isA(.BaseDialog) then return -3

  if self~dlg <> .nil then self~detach

  oldMenu = WinMenu("ASSOC", self~hMenu, dlg~hwnd)
  if oldMenu < 0 then return oldMenu

  dlg~linkMenu(self)
  self~dlg = dlg

  self~checkConnectionRequest(dlg)

  return oldMenu

::method replace
  use strict arg dlg
  if \ dlg~isA(.BaseDialog) then return -3

::method detach
  dlg = self~dlg
  if dlg == .nil then return -1

  ret = WinMenu("DET", dlg~hwnd)
  dlg~unlinkMenu
  self~dlg = .nil
  return ret

/** unInit()
 * A menu that is not assigned to a window must be destroyed by the application
 * when it closes.  On the other hand, menus assigned to a window are destroyed
 * by the system when the window is destroyed.
 */
::method unInit
  if self~dlg <> .nil then return
  self~destroy

::method failedInit private
  use strict arg ret
  self~destroy
  self~initCode = ret
  return ret


/* class: BinaryMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu created from a binary resource, or created as an empty
     menu.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'BinaryMenuBar' public subclass Object inherit MenuBar ResourceUtils

::method init
  use strict arg src = .nil, id = .nil, symbolSrc = .nil, helpID = 0, attach = .false, autoConnect = .false

  if self~menuInit(symbolSrc) <> 0 then return self~initCode
  if \ self~maybeResolveID(id) then return self~initCode

  select
    when src~isA(.ResDialog) then do
      if self~wID == .nil then do
        self~initCode = -1
        return -1
      end

      hMenu = WinMenu("LOAD", 0, src~adm, self~wID)
    end

    when src~isA(.string) then do
      if src~datatype('W') then do
        -- This is to create a BinaryMenuBar from a hMenu.  But, is this really
        -- of any use?  It could be used of creating a System menu, but a system
        -- menu might be better off as its own class.
        if WinMenu("ISMENU", src) then hMenu = src
        else hMenu = -1
      end
      else do
        if self~wID == .nil then do
          self~initCode = -1
          return -1
        end

        hMenu = WinMenu("LOAD", 1, src, self~wID)
      end
    end

    when src == .nil then hMenu = WinMenu("CREATE", 0)

    otherwise hMenu = -1
  end
  -- End select

  if hMenu < 0 then do
    self~initCode = hMenu
    return hMenu
  end

  self~hMenu = hMenu
  if helpID <> 0 then self~setHelpID(helpID)

  select
    when attach & src~isA(.ResDialog) then do
      ret = self~attachTo(src)
      if ret < 0 then return self~failedInit(ret)
    end

    when attach & symbolSrc~isA(.BaseDialog) then do
      ret = self~attachTo(symbolSrc)
      if ret < 0 then return self~failedInit(ret)
    end

    otherwise nop
  end
  -- End select

  return 0


/* class: PopupMenu- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A popup menu.  (Popup menus are also called, at various times, submenus,
     drop down menus, or context menus.)
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'PopupMenu' public subclass Object inherit Menu ResourceUtils

::method init
  use strict arg id = .nil, handle = .nil, symbolSrc = .nil, helpID = 0

  if self~menuInit(symbolSrc) <> 0 then return self~initCode
  if \ self~maybeResolveID(id) then return self~initCode

  select
    when handle == .nil then do
      handle = WinMenu("CREATE", 1)
    end

    when handle < 0 then nop

    otherwise do
      if \ WinMenu("ISMENU", handle) then handle = -2
    end
  end
  -- End select

  if handle < 0 then do
    self~initCode = handle
    return handle
  end

  self~hMenu = handle
  if helpID <> 0 then self~setHelpID(helpID)

  return 0

/** show() displays a context menu and returns after the user has selected an
 * item or canceled.  When the user selects an item, a WM_COMMAND message with
 * the id of the selection is generated.  If the user cancels, nothing happens.
 * 0 indicates success, less than 0 is the negated system error code.
 */
::method show unguarded
  use strict arg x, y, owner = (self~dlg), hAlign = "RIGHT", vAlign = "BOTTOM", bothButtons = .false, miscOpts = ""

  if \ owner~isA(.BaseDialog) then return -3
  if self~dlg \== owner then do
    -- TODO Revisit  Question: Do we want to set the assigned dialog to the owner ??
    self~dlg = owner
  end

  if bothButtons then mode = "RIGHT"
  else mode = ""

  return TrackPopup(mode, self~hMenu, owner~dlgHandle, x, y, hAlign~upper, vAlign~upper, miscOpts~upper)

/** track() displays a contex menu and returns after the user has selected an
 * item or canceled the menu.  No notification (no WM_COMMAND message) is sent
 * to the owner window.  The selected id is returned, or 0 if the menu is
 * canceled or an error happens.
 */
::method track unguarded
  use strict arg x, y, owner = (self~dlg), hAlign = "RIGHT", vAlign = "BOTTOM", bothButtons = .false, miscOpts = ""

  if \ owner~isA(.BaseDialog) then return -3
  if self~dlg \== owner then do
    -- TODO Revisit  Question: Do we want to set the assigned dialog to the owner ??
    self~dlg = owner
  end

  if bothButtons then mode = "NONOTIFY RIGHT"
  else mode = "NONOTIFY"

  return TrackPopup(mode, self~hMenu, owner~dlgHandle, x, y, hAlign~upper, vAlign~upper, miscOpts~upper)

::method assignTo unguarded
  use strict arg dlg
  if \ dlg~isA(.BaseDialog) then return -3

  oldDlg = self~dlg
  self~dlg = dlg

  self~checkConnectionRequest(dlg)
  return oldDlg

/* class: MenuTemplate - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
    A mix in class supplying the funtionality to create a menu template in
    memory.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'MenuTemplate' public mixinclass ResourceUtils

::method memInit private
  expose basePtr currentPtr memHandle isFinal
  use strict arg count, helpID

  isFinal = .false

  parse value MemMenu("INIT", count, helpID) with memHandle basePtr currentPtr

  if memHandle < 0 then do
    self~finalize
    return memHandle
  end

  return 0

::method isFinished
  expose isFinal
  return isFinal

::method finish
  expose basePtr currentPtr memHandle isFinal

  if isFinal | memHandle == 0 then return -2

  hMenu = WinMenu("LOAD", 2, memHandle, basePtr)
  self~finalize
  return hMenu

::method addPopup
  expose currentPtr isFinal
  use strict arg name, id = 0, opt = "", helpID = 0

  if isFinal then return -2

  id = self~checkId(id)
  if  id == -1 then return id

  -- Rather than fail on a bad help ID, just use 0.
  helpID = self~checkId(helpID)
  if  helpID == -1 then helpID = 0

  currentPtr = MemMenu("ADD", currentPtr, id, name, "POPUP " || opt~upper, helpID)

  return 0

::method addItem
  expose currentPtr isFinal
  use strict arg name, id, opt = "", msgToRaise = .nil

  if isFinal then return -2

  id = self~CheckId(id)
  if  id == -1 then return id

  currentPtr = MemMenu("ADD", currentPtr, id, name, opt~upper)

  if msgToRaise <> .nil then do
    self~connectionRequested = .true
    self~connectionQ~queue(.MapItem~new(id, msgToRaise))
  end

  return 0

/* With extended menus, separators can have IDs.  This is useful ...  */
::method addSeparator
  expose currentPtr isFinal
  use strict arg id = 0, opt = ""

  if isFinal then return -2

  -- Rather than fail on a bad ID, just use 0.
  id = self~checkId(id)
  if  id == -1 then id = 0

  currentPtr = MemMenu("ADD", currentPtr, id, "", "SEPARATOR")
  return 0

::method finalize private
  expose basePtr currentPtr memHandle isFinal

  basePtr = 0
  currentPtr = 0
  memHandle = 0
  isFinal = .true


/* class: UserMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu whose initial menu template is created in memory though
     user (programmer) defined statements.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'UserMenuBar' public subclass Object inherit MenuBar MenuTemplate

-- The context help ID for a menu.  Assigning the help ID in the
-- MENUEX_TEMPLATE_HEADER does not work as Microsoft documents it. This is used
-- as a work around.  It is not maintained to always reflect with the real id
-- is; use self~getHelpID() to determine the actual help ID.
::attribute helpID private

::method init
  use strict arg symbolSrc = .nil, count = 100, helpID = 0

  if self~menuInit(symbolSrc) <> 0 then return self~initCode

  -- Rather than fail on a bad help ID, just use 0.
  helpID = self~checkId(helpID)
  if  helpID == -1 then helpID = 0

  ret = self~memInit(count, helpID)
  if ret < 0 then do
    self~initCode = ret
    return ret
  end

  -- Work around.  Setting the help ID in the in-memory template does not work
  -- as documented.
  self~helpID = helpID

  return 0

::method complete
  hMenu = self~finish
  if hMenu < 0 then return hMenu

  self~hMenu = hMenu
  if self~helpID <> 0 then self~setHelpID(self~HelpID)

  return 0


/* class: ScriptMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu whose initial menu template is created in memory by parsing
     a resouce script file.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'ScriptMenuBar' public subclass Object inherit MenuBar MenuTemplate

::method init
  use strict arg rcFile, id = .nil, symbolSrc = .nil, helpID = 0, count = 50, connect = .false, attach = .false

  if self~menuInit(symbolSrc) <> 0 then return self~initCode
  if \ self~maybeResolveID(id) then return self~initCode

  helpID = self~checkId(helpID)
  if  helpID == -1 then helpID = 0

  ret = self~memInit(count, helpID)
  if ret < 0 then do
    self~initCode = ret
    return ret
  end

  ret = self~load(rcFile, self~wID, connect, count)
  if ret <> 0 then do
    self~initCode = ret
    return ret
  end

  hMenu = self~finish
  if hMenu < 0 then do
    self~initCode = hMenu
    return hMenu
  end

  self~hMenu = hMenu

  if helpID <> 0 then self~setHelpID(helpID)

  if attach & symbolSrc~isA(.BaseDialog) then do
    ret = self~attachTo(symbolSrc)
    if ret < 0 then return self~failedInit(ret)
  end

  return 0

::method load private
   use strict arg resfile, menuid, connect, count

   file = SysSearchPath("PATH",resfile)
   if file == "" then do
      say "Resource file not found -->" resfile
      return 1
   end

   f = .stream~new(file)
   op = f~open(read)
   if op \= "READY:" then do
      say "Resource file not readable -->" op
      return 1
   end

   found = 0; n = 0
   fl = f~lines
   do while found = 0 & fl > 0
      do while n = 0 & fl > 0
         s = f~linein; fl = fl - 1
         /* Add any sybmolic IDs defined in the file to the ConstDir.  Only
          * check for #define at the top of the file.  We check for symbolic IDS
          * here because menu resources could be defined in a separate file.
          */
         if s~wordpos("#define") > 0 then do
            s = s~translate(" ", "9"x)
            parse var s "#define " symb iid
            self~ConstDir[symb~space(0)~translate] = iid
         end
         n = s~wordpos("MENU")
      end

      if (self~checkfile(f) = 1) then return 1

      if menuid == .nil  then found = 1
      else do
         if s~word(n-1)~translate = menuid~translate then found = 1
         else if self~CheckId(s~word(n-1)) = menuid then found = 1
         else n = 0
      end
   end

   s = f~linein; fl = fl - 1
   do while s~wordpos("BEGIN") = 0 & s~pos("{") = 0 & fl > 0
      s = f~linein; fl = fl - 1
   end

   if (self~checkFile(f) = 1) then return 1

   rcarray = .array~new(50)
   bracket = 1
   cur = 0
   endcount = 0

   prevs = ""
   s = f~linein; fl = fl - 1
   do while bracket > 0 & fl >= 0
      if s~wordpos("END") > 0 | s~pos("}") > 0 then
      do
         bracket = bracket - 1;
         endcount = endcount + 1
         cur = cur + 1
         rcarray[cur] = s
      end
      else if s~wordpos("BEGIN") > 0 | s~pos("{") > 0 then do
         bracket = bracket + 1;
         cur = cur + 1
         rcarray[cur] = s
      end
      else if s~strip \= "" then do
         cur = cur + 1
         rcarray[cur] = s
      end
      s = f~linein; fl = fl - 1
   end

   f~close
   arrcount = cur;

   if (count < cur - endcount) then count = cur - endcount +5;

   self~processingLoad = 1

   do i = 1 to arrcount
      s = rcarray[i]

      select
         when s~wordpos("POPUP") > 0 then do
            parse var s type '"'name'"'    "," opt

            j = i + 1;
            bracket = 0
            do until bracket = 0
              if rcarray[j]~wordpos("BEGIN") > 0 | rcarray[j]~pos("{") > 0 then
                bracket = bracket + 1
              else if rcarray[j]~wordpos("END") > 0 | rcarray[j]~pos("}") > 0 then
                bracket = bracket - 1
              j = j +1
            end

            if rcarray[j]~wordpos("END") > 0 | rcarray[j]~pos("}") > 0 then opt = opt || " END"
            self~addPopup(name, , opt)
         end

         when s~wordpos("SEPARATOR") > 0 then do
            parse var s type sep opt
            if rcarray[i+1]~wordpos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
            self~addSeparator( , opt)
         end

         when s~wordpos("MENUITEM") > 0 then do
            parse var s type '"'name'"'    "," id "," opt
            if rcarray[i+1]~wordpos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
            if connect then
               self~addItem(name, id, opt, name~changeStr('&', "")~space(0))
            else
               self~addItem(name, id, opt)
         end

         when s~wordpos("END") > 0 | s~pos("}") > 0 | s~wordpos("BEGIN") > 0 | s~pos("{") > 0 then nop;

         otherwise do
            self~errorFile(f, s)
            return 1
         end
     end /*select 1*/
  end /* do while */
  self~processingLoad = 0

  return 0

::class 'MapItem' public

::attribute key set private
::attribute key get

::attribute item set private
::attribute item get

::method init
  expose key item
  use strict arg key, item


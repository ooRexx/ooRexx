/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2014 Rexx Language Association. All rights reserved.    */
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
 * Windows Dialog Interface for Open Object Rexx (ooRexx)
 *
 * Menu Classes
 *
 * Provides support for all types of Windows Menus.
 */


/* class: Menu - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A mix in class implementing functionality that is common to all menus.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'Menu' public mixinclass Object

::method connectCommandEvent class external "LIBRARY oodialog menu_connectCommandEvent_cls"

::method menuInit private external "LIBRARY oodialog menu_menuInit_pvt"
::method unInit external "LIBRARY oodialog menu_uninit"

::attribute wID get external "LIBRARY oodialog menu_wID"
::attribute hMenu get external "LIBRARY oodialog menu_getHMenu"

-- TODO NEED appendPopup() and appendItem()  (note you can not append a separator, or can you?)

-- TODO there are some methods where the C code was there but there are no
-- Rexx methods to match.  SetItemType, SetPopupType, SetItemState, SetPopupType
-- These methods would take a combination of keywords.  The parsing of the
-- keywords is already implmented.

::method check external "LIBRARY oodialog menu_check"
::method checkRadio external "LIBRARY oodialog menu_checkRadio"
::method connectAllCommandEvents external "LIBRARY oodialog menu_connectAllCommandEvents"
::method connectCommandEvent external "LIBRARY oodialog menu_connectCommandEvent"
::method connectMenuEvent external "LIBRARY oodialog menu_connectMenuEvent"
::method connectSomeCommandEvents external "LIBRARY oodialog menu_connectSomeCommandEvents"
::method deletePopup external "LIBRARY oodialog menu_deletePopup"
::method destroy external "LIBRARY oodialog menu_destroy"
::method disable external "LIBRARY oodialog menu_disable"
::method enable external "LIBRARY oodialog menu_enable"
::method getAutoConnectStatus external "LIBRARY oodialog menu_getAutoConnectStatus"
::method getCount external "LIBRARY oodialog menu_getCount"
::method getHandle external "LIBRARY oodialog menu_getHMenu"
::method getHelpID external "LIBRARY oodialog menu_getHelpID"
::method getID external "LIBRARY oodialog menu_getID"
::method getItemState external "LIBRARY oodialog menu_getItemState"
::method getMaxHeight external "LIBRARY oodialog menu_getMaxHeight"
::method getMenuHandle external "LIBRARY oodialog menu_getMenuHandle"
::method getPopup unguarded external "LIBRARY oodialog menu_getPopup"
::method getText external "LIBRARY oodialog menu_getText"
::method getType external "LIBRARY oodialog menu_getItemType"
::method gray external "LIBRARY oodialog menu_disable"
::method hilite external "LIBRARY oodialog menu_hilite"
::method insertItem unguarded external "LIBRARY oodialog menu_insertItem"
::method insertPopup unguarded external "LIBRARY oodialog menu_insertPopup"
::method insertSeparator unguarded external "LIBRARY oodialog menu_insertSeparator"
::method isChecked external "LIBRARY oodialog menu_isChecked"
::method isCommandItem external "LIBRARY oodialog menu_isCommandItem"
::method isDisabled external "LIBRARY oodialog menu_isDisabled"
::method isEnabled external "LIBRARY oodialog menu_isEnabled"
::method isGrayed external "LIBRARY oodialog menu_isDisabled"
::method isPopup external "LIBRARY oodialog menu_isPopup"
::method isSeparator external "LIBRARY oodialog menu_isSeparator"
::method isValidItemID external "LIBRARY oodialog menu_isValidItemID"
::method isValidMenu external "LIBRARY oodialog menu_isValidMenu"
::method isValidMenuHandle external "LIBRARY oodialog menu_isValidMenuHandle"
::method itemTextToMethodName external "LIBRARY oodialog menu_itemTextToMethodName"
::method releaseMenuHandle external "LIBRARY oodialog menu_releaseMenuHandle"
::method removeItem external "LIBRARY oodialog menu_removeItem"
::method removePopup external "LIBRARY oodialog menu_removePopup"
::method removeSeparator external "LIBRARY oodialog menu_removeSeparator"
::method setAutoConnection external "LIBRARY oodialog menu_setAutoConnection"
::method setHelpID external "LIBRARY oodialog menu_setHelpID"
::method setID external "LIBRARY oodialog menu_setID"
::method setMaxHeight external "LIBRARY oodialog menu_setMaxHeight"
::method setText external "LIBRARY oodialog menu_setText"
::method unCheck external "LIBRARY oodialog menu_unCheck"
::method unHilite external "LIBRARY oodialog menu_unHilite"

::method test external "LIBRARY oodialog menu_test"

/* class: MenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A mix in class implementing functionality that is common to all menu bars.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'MenuBar' public mixinclass Menu

::method attachTo external "LIBRARY oodialog menuBar_attachTo"
::method detach external "LIBRARY oodialog menuBar_detach"
::method isAttached external "LIBRARY oodialog menuBar_isAttached"
::method redraw external "LIBRARY oodialog menuBar_redraw"
::method replace external "LIBRARY oodialog menuBar_replace"


/** unInit()
 * A menu that is not assigned to a window must be destroyed by the application
 * when it closes.  On the other hand, menus assigned to a window are destroyed
 * by the system when the window is destroyed.
 */
::method unInit
  forward class (.Menu)

/* class: SystemMenu - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A system menu class.  System menus are also known as window or control
     menus.  It is the menu that appears when clicking on the icon in the upper
     left corner of a window.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'SystemMenu' public subclass Object inherit Menu

::constant WM_SYSCOMMAND     "0x0112"

-- Use decimal value to allow comparison in event handlers.
::constant SC_SIZE           61440
::constant SC_MOVE           61456
::constant SC_MINIMIZE       61472
::constant SC_MAXIMIZE       61488
::constant SC_NEXTWINDOW     61504
::constant SC_PREVWINDOW     61520
::constant SC_CLOSE          61536
::constant SC_VSCROLL        61552
::constant SC_HSCROLL        61568
::constant SC_MOUSEMENU      61584
::constant SC_KEYMENU        61696
::constant SC_ARRANGE        61712
::constant SC_RESTORE        61728
::constant SC_TASKLIST       61744
::constant SC_SCREENSAVE     61760
::constant SC_HOTKEY         61776
::constant SC_DEFAULT        61792
::constant SC_MONITORPOWER   61808
::constant SC_CONTEXTHELP    61824
::constant SC_SEPARATOR      61455

::method init external "LIBRARY oodialog sysMenu_init"
::method revert external "LIBRARY oodialog sysMenu_revert"
::method unInit
  forward class (.Menu)



/* class: BinaryMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu created from a binary resource, or created as an empty
     menu.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'BinaryMenuBar' public subclass Object inherit MenuBar

::method init external "LIBRARY oodialog binMenu_init"
::method unInit
  forward class (.Menu)


/* class: PopupMenu- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A popup menu.  (Popup menus are also called, at various times, submenus,
     drop down menus, or context menus.)
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'PopupMenu' public subclass Object inherit Menu

::method connectContextMenu class external "LIBRARY oodialog popMenu_connectContextMenu_cls"

::method init external "LIBRARY oodialog popMenu_init"

::method assignTo external "LIBRARY oodialog popMenu_assignTo"
::method connectContextMenu external "LIBRARY oodialog popMenu_connectContextMenu"
::method isAssigned external "LIBRARY oodialog popMenu_isAssigned"
::method show external "LIBRARY oodialog popMenu_show"
::method track external "LIBRARY oodialog popMenu_track"

::method unInit
  forward class (.Menu)

/* class: MenuTemplate - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
    A mix in class supplying the funtionality to create a menu template in
    memory.  This a private, internal use only, class.  The methods are doc-
    umented as part of the UserMenuBar, the class itself is not documented.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'MenuTemplate' private mixinclass Object

::method addPopup external "LIBRARY oodialog menuTemplate_addPopup"
::method addItem external "LIBRARY oodialog menuTemplate_addItem"
::method addSeparator external "LIBRARY oodialog menuTemplate_addSeparator"
::method isComplete external "LIBRARY oodialog menuTemplate_isComplete"


/* class: UserMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu whose initial menu template is created in memory though
     user (programmer) defined statements.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'UserMenuBar' public subclass Object inherit MenuBar MenuTemplate

::method init external "LIBRARY oodialog userMenu_init"
::method complete external "LIBRARY oodialog userMenu_complete"
::method unInit
  forward class (.Menu)


/* class: ScriptMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu whose initial menu template is created in memory by parsing
     a resouce script file.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'ScriptMenuBar' public subclass Object inherit MenuBar MenuTemplate

::method init external "LIBRARY oodialog scriptMenu_init"
::method resolveSymbolicID external "LIBRARY oodialog global_resolveSymbolicID"
::method unInit
  forward class (.Menu)

/** load() Parses a menu definition in a resource script file and loads it into
 *         memory (through a MenuTemplate.)  The MENU resource definition
 *         statement looks like:
 *
 *    menuID MENU  [[optional-statements]]
 *
 *    We ignore the optional statements. menuID can be a number, a symbolic ID
 *    that resolves to a number, or a string name.
 *
 *    If noMenuID is set to .true we simply take the first menu found.  If
 *    menuid is set to -1 then we are looking for a string name that matches
 *    menuName.  Otherwise we match menuid.  Syntax errors are raised for all
 *    problems, including not finding the menu.  If no error is raised, the
 *    menu is guaranteed to be loaded into memory.
 *
 *    Assumes that the caller has already parsed the file for symbolic ID
 *    defintions.
 */
::method load private
  use strict arg resfile, menuid, connect, count, noMenuID = .false, menuName = ""

  signal on syntax

  file = SysSearchPath("PATH", resfile)
  if file == "" then do
    msg = 'Unable to find resource script file "'resFile'"'
    raise syntax 98.900 array (msg)
  end

  f = .stream~new(file)
  op = f~open(read)
  if op \= "READY:" then do
    msg = 'Resource script file "'resFile'" is not readable:' op
    raise syntax 98.900 array (msg)
  end

  found = 0; n = 0
  menuName = menuName~translate
  isMenuEx = .false
  fl = f~lines
  do while found = 0, fl > 0
     do while n = 0, fl > 0
        s = f~linein; fl -= 1
        keyWord = s~word(2)
        if keyWord == "MENU" |  keyWord == "MENUEX" then n = 2
     end

     if f~lines == 0 then do
       -- We read the whole file and never found a MENU statement.
       f~close

       if noMenuID then subMsg = 'no MENU statement found'
       else if menuName \== "" then subMsg = 'no MENU statement for menuid "'menuName'"'
       else subMsg = 'no MENU statement for menuid "'menuid'"'

       msg = 'Error: script file "'resFile'"' subMsg
       raise syntax 98.900 array (msg)
     end

     if noMenuID then found = 1
     else if menuid == -1, s~word(n-1)~translate == menuName then found = 1
     else if menuid \== -1, self~resolveSymbolicID(s~word(n-1)) == menuid then found = 1
     else n = 0

     if n \= 0, keyWord == "MENUEX" then isMenuEx = .true
  end

  s = f~linein; fl -= 1
  do while s~wordpos("BEGIN") = 0, s~pos("{") = 0, fl > 0
     s = f~linein; fl -= 1
  end

  if f~lines == 0 then do
     -- We read the whole file and never found the begin or bracket.
     f~close
     msg = 'Error: script file "'resFile'" no BEGIN or "{" for MENU statement'
     raise syntax 98.900 array (msg)
  end

  rcarray = .array~new(50)
  bracket = 1
  cur = 0
  endcount = 0

  prevs = ""
  s = f~linein; fl -= 1
  do while bracket > 0, fl >= 0
     if s~wordpos("END") > 0 | s~pos("}") > 0 then do
       bracket -= 1;
       endcount += 1
       cur += 1
       rcarray[cur] = s
     end
     else if s~wordpos("BEGIN") > 0 | s~pos("{") > 0 then do
       bracket += 1;
       cur += 1
       rcarray[cur] = s
     end
     else if s~strip \= "" then do
       cur += 1
       rcarray[cur] = s
     end
     s = f~linein; fl -= 1
  end

  f~close
  arrcount = cur

  if (count < cur - endcount) then do
    msg = 'Menu in script file "'resFile'"contains more items ('cur - endcount') than allocated ('count')'
    raise syntax 98.900 array (msg)
  end

  if isMenuEx then return self~parseMenuEx(rcarray, arrcount, connect)
  else return self~parseMenu(rcarray, arrcount, connect)

syntax:
  c = condition('O')
  if c~traceBack \== .nil then c~traceBack~empty
  raise propagate

::method parseMenu private
  use strict arg rcarray, arrcount, connect

  do i = 1 to arrcount
     s = rcarray[i]

     select
        when s~wordPos("POPUP") > 0 then do
           parse var s type '"'name'"'    "," opt

           j = i + 1;
           bracket = 0
           do until bracket = 0
             if rcarray[j]~wordPos("BEGIN") > 0 | rcarray[j]~pos("{") > 0 then
               bracket += 1
             else if rcarray[j]~wordPos("END") > 0 | rcarray[j]~pos("}") > 0 then
               bracket -= 1
             j += 1
           end

           if rcarray[j]~wordPos("END") > 0 | rcarray[j]~pos("}") > 0 then opt = opt || " END"
           self~addPopup(0, name, opt, 0)
        end

        when s~wordPos("SEPARATOR") > 0 then do
           parse var s type sep opt
           if rcarray[i+1]~wordPos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
           self~addSeparator(0, opt)
        end

        when s~wordPos("MENUITEM") > 0 then do
           parse var s type '"'name'"'    "," id "," opt
           id = id~strip
           if rcarray[i+1]~wordPos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
           if connect then
              self~addItem(id, name, opt, self~itemTextToMethodName(name))
           else
              self~addItem(id, name, opt)
        end

        when s~wordPos("END") > 0 | s~pos("}") > 0 | s~wordPos("BEGIN") > 0 | s~pos("{") > 0 then nop;

        otherwise do
          msg = "Error parsing MENU definition." || '  ' || "Resource file:" f || '  ' || "Line:" s
          raise syntax 98.900 array (msg)
        end
    end /*select 1*/
  end /* do while */

  return 0

::method mergeTypeState private
  use strict arg type, state

  opt  = ""
  if type~wordPos("MFT_BITMAP") > 0 then opt = "BITMAP"
  else if type~wordPos("MFT_SEPARATOR") > 0 then opt = "SEPARATOR"
  else if type~wordPos("MFT_STRING") > 0 then opt = "STRING"

  if type~wordPos("MFT_MENUBARBREAK") > 0 then opt ||= " MENUBARBREAK"
  if type~wordPos("MFT_MENUBREAK") > 0 then opt    ||= " MENUBREAK"
  if type~wordPos("MFT_OWNERDRAW") > 0 then opt    ||= " OWNERDRAW"
  if type~wordPos("MFT_RADIOCHECK") > 0 then opt   ||= " RADIOCHECK"
  if type~wordPos("MFT_RIGHTJUSTIFY") > 0 then opt ||= " RIGHTJUSTIFY"
  if type~wordPos("MFT_RIGHTORDER") > 0 then opt   ||= " RIGHTORDER"

  if state~wordPos("MFS_CHECKED") > 0 then opt ||= " CHECKED"
  if state~wordPos("MFS_DEFAULT") > 0 then opt ||= " DEFAULT"
  if state~wordPos("MFS_DISABLED") > 0 then opt ||= " DISABLED"
  if state~wordPos("MFS_ENABLED") > 0 then opt ||= " ENABLED"
  if state~wordPos("MFS_GRAYED") > 0 then opt ||= " GRAYED"
  if state~wordPos("MFS_HILITE") > 0 then opt ||= " HILITE"
  if state~wordPos("MFS_UNCHECKED") > 0 then opt ||= " UNCHECKED"
  if state~wordPos("MFS_UNHILITE") > 0 then opt ||= " UNHILITE"

  return opt


::method parseMenuEx private
  use strict arg rcarray, arrcount, connect

  do i = 1 to arrcount
     s = rcarray[i]

     select
        when s~word(1) == "POPUP" then do
           parse var s type '"'name'"' "," id "," type "," state "," helpID
           id = id~strip; helpID = id~strip

           j = i + 1;
           bracket = 0
           do until bracket = 0
             if rcarray[j]~wordPos("BEGIN") > 0 | rcarray[j]~pos("{") > 0 then
               bracket += 1
             else if rcarray[j]~wordPos("END") > 0 | rcarray[j]~pos("}") > 0 then
               bracket -= 1
             j += 1
           end

           opt = self~mergeTypeState(type, state)
           if rcarray[j]~wordPos("END") > 0 | rcarray[j]~pos("}") > 0 then opt ||= " END"

           self~addPopup(id, name, opt, helpID)
        end

        when s~word(1) == "MENUITEM" then do
           parse var s type '"'name'"' "," id "," type "," state
           id = id~strip

           opt = self~mergeTypeState(type, state)
           if rcarray[i+1]~wordPos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt ||= " END"

           if opt~wordPos("SEPARATOR") > 0 then do
             self~addSeparator(id, opt)
           end
           else do
             if connect then
                self~addItem(id, name, opt, self~itemTextToMethodName(name))
             else
                self~addItem(id, name, opt)
           end
        end

        when s~wordPos("END") > 0 | s~pos("}") > 0 | s~wordPos("BEGIN") > 0 | s~pos("{") > 0 then nop;

        otherwise do
          msg = "Error parsing MENUEX definition." || '  ' || "Resource file:" f || '  ' || "Line:" s
          raise syntax 98.900 array (msg)
        end
    end /*select i = 1*/
  end /* do while */

  return 0



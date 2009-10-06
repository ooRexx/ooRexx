/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2009 Rexx Language Association. All rights reserved.    */
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

::method connectSelect class external "LIBRARY oodialog menu_connectSelect_cls"

::method menuInit private external "LIBRARY oodialog menu_menuInit_pvt"

::attribute wID get external "LIBRARY oodialog menu_wID"
::attribute hMenu get external "LIBRARY oodialog menu_getHMenu"

::method getCount external "LIBRARY oodialog menu_getCount"
::method isValidMenu external "LIBRARY oodialog menu_isValidMenu"
::method isValidMenuHandle external "LIBRARY oodialog menu_isValidMenuHandle"
::method isValidItemID external "LIBRARY oodialog menu_isValidItemID"

::method isPopup external "LIBRARY oodialog menu_isPopup"
::method isCommandItem external "LIBRARY oodialog menu_isCommandItem"
::method isSeparator external "LIBRARY oodialog menu_isSeparator"

/** getHandle()
 * Need to also doc this method which is really just an alias for self~hMenu.
 */
::method getHandle external "LIBRARY oodialog menu_getHMenu"
::method getMenuHandle external "LIBRARY oodialog menu_getMenuHandle"
::method releaseMenuHandle external "LIBRARY oodialog menu_releaseMenuHandle"
::method destroy external "LIBRARY oodialog menu_destroy"

::method getPopup unguarded external "LIBRARY oodialog menu_getPopup"

-- TODO NEED appendPopup() and appendItem()  (note you can not append a separator, or can you?)

::method insertPopup unguarded external "LIBRARY oodialog menu_insertPopup"
::method insertItem unguarded external "LIBRARY oodialog menu_insertItem"
::method insertSeparator unguarded external "LIBRARY oodialog menu_insertSeparator"
::method removePopup external "LIBRARY oodialog menu_removePopup"
::method removeItem external "LIBRARY oodialog menu_removeItem"
::method removeSeparator external "LIBRARY oodialog menu_removeSeparator"
::method deletePopup external "LIBRARY oodialog menu_deletePopup"

::method isEnabled external "LIBRARY oodialog menu_isEnabled"
::method isDisabled external "LIBRARY oodialog menu_isDisabled"
::method isGrayed external "LIBRARY oodialog menu_isDisabled"
::method isChecked external "LIBRARY oodialog menu_isDisabled"

::method enable external "LIBRARY oodialog menu_enable"
::method disable external "LIBRARY oodialog menu_disable"
::method gray external "LIBRARY oodialog menu_disable"
::method check external "LIBRARY oodialog menu_check"
::method unCheck external "LIBRARY oodialog menu_unCheck"
::method checkRadioItem external "LIBRARY oodialog menu_checkRadio"
::method hilite external "LIBRARY oodialog menu_hilite"
::method unHilite external "LIBRARY oodialog menu_unHilite"
::method getItemState external "LIBRARY oodialog menu_getItemState"
::method getType external "LIBRARY oodialog menu_getItemType"

-- TODO there are some methods where the C code was there but there are no
-- Rexx methods to match.  SetItemType, SetPopupType, SetItemState, SetPopupType
-- These methods would take a combination of keywords.  The parsing of the
-- keywords is already implmented.


::method getText external "LIBRARY oodialog menu_getText"
::method setText external "LIBRARY oodialog menu_setText"
::method getID external "LIBRARY oodialog menu_getID"
::method setID external "LIBRARY oodialog menu_setID"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Methods that affect an entire menu, rather than a menu item.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

::method setHelpID external "LIBRARY oodialog menu_setHelpID"
::method getHelpID external "LIBRARY oodialog menu_getHelpID"
::method setMaxHeight external "LIBRARY oodialog menu_setMaxHeight"
::method getMaxHeight external "LIBRARY oodialog menu_getMaxHeight"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Message connection methods.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::method itemTextToMethodName external "LIBRARY oodialog menu_itemTextToMethodName"
::method setAutoConnection external "LIBRARY oodialog menu_setAutoConnection"
::method getAutoConnectStatus external "LIBRARY oodialog menu_getAutoConnectStatus"
::method connectSelect external "LIBRARY oodialog menu_connectSelect"
::method connectAllSelects external "LIBRARY oodialog menu_connectAllSelects"
::method connectSomeSelects external "LIBRARY oodialog menu_connectSomeSelects"
::method connectMenuEvent external "LIBRARY oodialog menu_connectMenuEvent"

::method test external "LIBRARY oodialog menu_test"

/* class: MenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A mix in class implementing functionality that is common to all menu bars.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'MenuBar' public mixinclass Menu

::method replace
  use strict arg dlg
  if \ dlg~isA(.BaseDialog) then return -3

::method isAttached external "LIBRARY oodialog menuBar_isAttached"
::method redraw external "LIBRARY oodialog menuBar_redraw"
::method attachTo external "LIBRARY oodialog menuBar_attachTo"
::method detach external "LIBRARY oodialog menuBar_detach"

/** unInit()
 * A menu that is not assigned to a window must be destroyed by the application
 * when it closes.  On the other hand, menus assigned to a window are destroyed
 * by the system when the window is destroyed.
 */
--/*
::method unInit
  say 'In unInit() of MenuBar class'
  if self~dlg <> .nil then return
  self~destroy
--*/

/* class: SystemMenu - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A system menu class.  System menus are also known as window or control
     menus.  They are the menu that appears when clicking on the icon in the
     upper left corner of a window.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'SystemMenu' public subclass Object inherit Menu ResourceUtils

::method init external "LIBRARY oodialog sysMenu_init"
::method revert external "LIBRARY oodialog sysMenu_revert"
::method connectSelect external "LIBRARY oodialog sysMenu_connectSelect"
::method connectAllSelects external "LIBRARY oodialog sysMenu_connectAllSelects"
::method connectSomeSelects external "LIBRARY oodialog sysMenu_connectSomeSelects"


/* class: BinaryMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu created from a binary resource, or created as an empty
     menu.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'BinaryMenuBar' public subclass Object inherit MenuBar ResourceUtils

::method init external "LIBRARY oodialog binMenu_init"


/* class: PopupMenu- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A popup menu.  (Popup menus are also called, at various times, submenus,
     drop down menus, or context menus.)
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'PopupMenu' public subclass Object inherit Menu ResourceUtils

::method connectContextMenu class external "LIBRARY oodialog popMenu_connectContextMenu_cls"
::method init external "LIBRARY oodialog popMenu_init"
::method isAssigned external "LIBRARY oodialog popMenu_isAssigned"
::method show external "LIBRARY oodialog popMenu_show"
::method track external "LIBRARY oodialog popMenu_track"
::method assignTo external "LIBRARY oodialog popMenu_assignTo"
::method connectContextMenu external "LIBRARY oodialog popMenu_connectContextMenu"

::method unInit
  say 'In unInit() of PopupMenu class'
  --if self~dlg <> .nil then return
  --self~destroy

/* class: MenuTemplate - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
    A mix in class supplying the funtionality to create a menu template in
    memory.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'MenuTemplate' public mixinclass ResourceUtils

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


/* class: ScriptMenuBar- - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
     A menu bar menu whose initial menu template is created in memory by parsing
     a resouce script file.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::class 'ScriptMenuBar' public subclass Object inherit MenuBar MenuTemplate

::method init external "LIBRARY oodialog scriptMenu_init"

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
  fl = f~lines
  do while found = 0, fl > 0
     do while n = 0, fl > 0
        s = f~linein; fl -= 1
        n = s~wordpos("MENU")  -- TODO we could parse extended menus by looking for MENU *and* MENUEX
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
     else if menuid \== -1, self~resolveResourceID(s~word(n-1)) == menuid then found = 1
     else n = 0
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

  do i = 1 to arrcount
     s = rcarray[i]

     select
        when s~wordpos("POPUP") > 0 then do
           parse var s type '"'name'"'    "," opt

           j = i + 1;
           bracket = 0
           do until bracket = 0
             if rcarray[j]~wordpos("BEGIN") > 0 | rcarray[j]~pos("{") > 0 then
               bracket += 1
             else if rcarray[j]~wordpos("END") > 0 | rcarray[j]~pos("}") > 0 then
               bracket -= 1
             j += 1
           end

           if rcarray[j]~wordpos("END") > 0 | rcarray[j]~pos("}") > 0 then opt = opt || " END"
           self~addPopup(0, name, opt, 0)
        end

        when s~wordpos("SEPARATOR") > 0 then do
           parse var s type sep opt
           if rcarray[i+1]~wordpos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
           self~addSeparator(0, opt)
        end

        when s~wordpos("MENUITEM") > 0 then do
           parse var s type '"'name'"'    "," id "," opt
           if rcarray[i+1]~wordpos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
           if connect then
              self~addItem(id, name, opt, self~itemTextToMethodName(name))
           else
              self~addItem(id, name, opt)
        end

        when s~wordpos("END") > 0 | s~pos("}") > 0 | s~wordpos("BEGIN") > 0 | s~pos("{") > 0 then nop;

        otherwise do
          msg = "Error parsing MENU definition." || '  ' || "Resource file:" f || '  ' || "Line:" s
          raise syntax 98.900 array (msg)
        end
    end /*select 1*/
  end /* do while */

  return 0

syntax:
  c = condition('O')
  if c~traceBack \== .nil then c~traceBack~empty
  raise propagate


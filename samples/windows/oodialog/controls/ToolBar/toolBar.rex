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

/** toolBar.rex
 *
 * Very simple ToolBar example.  Just does the very minimum to show a tool bar.
 *
 * Note that most dialog editors do not have an option to add a tool bar.  This
 * can be overcome by editing the .rc file and manually adding the tool bar
 * control.  Which was done for this example.
 *
 * MSDN says that with some of the tool bar functionality there can be problems
 * painting the tool bar.  The suggestion is to create the tool bar not visible
 * and then show the tool bar after it has been set up.  That is what is done in
 * this example.
 */

  sd = locate()
  .application~setDefaults('O', sd'rc\toolBar.h', .false)

  dlg = .ToolBarDlg~new(sd"rc\toolBar.dll", IDD_TBAR)
  dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::class 'ToolBarDlg' subclass ResDialog

::method initDialog
  expose tb

  -- The buttons in a tool bar are just that, buttons.  We can connect the
  -- clicked event just as we would for a regular button.  Or we could use the
  -- connectToolBarEvent() method.
  self~connectButtonEvent(IDB_PRESS, 'CLICKED', onPress)
  self~connectButtonEvent(IDB_WHERE, 'CLICKED', onWhere)
  self~connectButtonEvent(IDB_PUSH, 'CLICKED', onPush)

  tb = self~newToolBar(IDC_TOOLBAR)

  tbb1 = .TbButton~new(IDB_PRESS, "Press Me Please", "BUTTON", "ENABLED")
  tbb2 = .TbButton~new(IDB_WHERE, "Where Am I?", "BUTTON", "ENABLED")
  tbb3 = .TbButton~new(IDB_PUSH, "Please Push Me", "BUTTON", "ENABLED")

  -- addButtons() takes an array of TbButton objects:
  buttons = .array~of(tbb1, tbb2)
  ret = tb~addButtons(buttons) -- Returns true on success, othewise false

  -- indsertButton() takes a single TbButton object and inserts the button to
  -- the left of the button index specified in argument 2.  In this case the
  -- button with the index of 2:
  ret = tb~insertButton(tbb3, 2) -- Returns true on success, othewise false

  -- Now that the tool bar has its buttons, tell it to recalculate its size.
  tb~autoSize

  -- And show the tool bar.
  tb~show
  tb~assignFocus


::method onPress unguarded
  title = 'ToolBar Example "Press Button"'
  msg   = 'Thanks for pressing me       '

  r = MessageDialog(msg, self~hwnd, title, 'OK', 'INFORMATION')
  return 0


::method onPush unguarded
  title = 'ToolBar Example "Push Button"'
  msg   = 'Glad you pushed me.           '

  r = MessageDialog(msg, self~hwnd, title, 'OK', 'INFORMATION')
  return 0


::method onWhere unguarded
  expose tb

  count = tb~buttonCount
  title = 'ToolBar Example "Where Button"'
  msg   = 'I am here, the button on the far right.' || .endOfLine~copies(2) || -
          'There are' count 'buttons.  I am button 3.'

  r = MessageDialog(msg, self~hwnd, title, 'OK', 'INFORMATION')
  return 0

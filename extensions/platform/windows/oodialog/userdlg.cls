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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/* User Dialog Class                                                          */
/*                                                                            */
/******************************************************************************/
/* include the base dialog class */
::requires "basedlg.cls"
::requires "dyndlg.cls"


/* This class extends the 'BaseDialog' class */
::class 'UserDialog' subclass BaseDialog public inherit DynamicDialog

::method init
   use arg DlgData., includeFile

   newArgs = .array~new(2)
   newArgs[1] = 0
   newArgs[2] = 0
   if arg(1, 'E') then newArgs[3] = dlgData.
   if Arg(2, 'E') then newArgs[4] = includeFile
   forward class (super) arguments (newArgs) continue

   if result = 0 then forward message "DynamicInit"
   else return result

::method StopIt
   forward class (super) continue
   forward message "StopDynamic" continue
   ret = result
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   if self~BkgBitmap \= 0 then self~RemoveBitmap(self~BkgBitmap)
   return ret

/******************************************************************************/
/* Menu/Action bar methods.                                                   */

::method SetMenu
    menuBar = self~menuBar

    if menuBar == .nil then return -2

    if menuBar~isA(.UserMenuBar) then do
      ret = menuBar~complete
      if ret < 0 then return ret
    end

    return menuBar~attachTo(self)

::method CreateMenu
    use strict arg count = 100

    menuBar = .UserMenuBar~new(self, count)
    if menuBar~initCode <> 0 then return menuBar~initCode

    self~menuBar = menuBar
    return 0;

::method AddPopupMenu
    use strict arg name, opt = ""
    if self~menuBar == .nil then return -2
    return self~menuBar~addPopup(name, , opt)

::method AddMenuItem
    use strict arg name, id, opt = "", msgToRaise = .nil
    if self~menuBar == .nil then return -2
    return self~menuBar~addItem(name, id, opt, msgToRaise)

::method AddMenuSeparator
    if self~menuBar == .nil then return -2
    return self~menuBar~addSeparator

::method LoadMenu protected
  use strict arg resfile, menuid = .nil, loadopts = "", count = 50

  connect = .false
  if loadopts~caselessWordPos("CONNECTITEMS") <> 0 then connect = .true

  menuBar = .ScriptMenuBar~new(resFile, menuid, self, 0, count, connect)
  if menuBar~initCode <> 0 then return menuBar~initCode

  self~menuBar = menuBar
  return 0

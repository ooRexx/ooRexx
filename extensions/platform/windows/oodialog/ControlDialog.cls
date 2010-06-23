/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2010 Rexx Language Association. All rights reserved.    */
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
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * Control Dialog Classes and interfaces.  A control dialog is a dialog with a
 * style that allows it work well as a dialog within a top-level dialog.  The
 * top-level dialog is both the owner and the parent of the control dialog.
 *
 * Control dialogs work well as the content for tab controls and as the pages of
 * a Windows PropertySheet dialog, not to be confused with the wrongly named
 * ooDialog PropertySheet class.
 *
 * A control dialog is also a child of the owner / parent dialog.  The
 * popupAsChild() method also uses the concept of parent child dialogs.  To
 * prevent confusion with that, the control dialog classes use the terms control
 * dialog and owner dialog.
 */

::class 'ControlDialog' mixinclass Object public

::attribute ownerDialog get  external "LIBRARY oodialog chld_getOwnerDialog"
::attribute ownerDialog set  external "LIBRARY oodialog chld_setOwnerDialog"


::class 'UserControlDialog' public subclass UserDialog inherit ControlDialog

-- Over-ride the superclass startIt().  The superclass startIt() creates a top-
-- level dialog and takes an 'icon' and a 'modeless' icon.  We need to accept
-- those arguments, but we don't use them.  So we just ignore all args
::method startIt

   if self~basePtr = 0 then return 0

   if self~startChildDialog(self~basePtr, 0) == 0 then return 0

   self~initDialog
   return self~dlgHandle


::class 'ResControlDialog' public subclass ResDialog inherit ControlDialog

::method startDialog private external "LIBRARY oodialog resCtrlDlg_startDialog_pvt"


::class 'RcControlDialog' subclass UserControlDialog public

::method init
  use arg rcFile, idDlg, initData., includeFile, options = "", expected = 200, ownerDlg

  newArgs = .array~new(5)
  if arg(3, 'E') then newArgs[1] = arg(3)
  if arg(4, 'E') then newArgs[2] = arg(4)
  if arg(7, 'E') then newArgs[3] = arg(7)
  newArgs[4] = rcFile
  newArgs[5] = idDlg

  forward class (super) arguments (newArgs) continue
  if result <> 0 then return result

  self~initCode = self~load(rcFile, idDlg, options, expected)

  return self~initCode


::class 'OwnerDialog' mixinclass Object public

::method init
  say 'In OwnerDialog::init()'
  --forward class (super) continue
  say 'Back in OwnerDialog::init()'

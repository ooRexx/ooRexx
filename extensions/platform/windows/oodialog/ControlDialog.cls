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
 * Control, PropertySheet, and PropertySheetPage dialog classes and interfaces.
 *
 * A control dialog is a dialog with a style that allows it work well as a
 * dialog within a top-level dialog.  The top-level dialog is both the owner and
 * the parent of the control dialog.  These dialogs work well as the content for
 * tab control pages.
 *
 * PropertySheet dialogs differ slightly from the normal ooDialog dialog because
 * the underlying dialog is created and maintained by the operating system.  This
 * introduces some subtle differences in the behavior of the dialog.
 *
 * PropertySheetPage dialogs serve as the pages of the PropertySheetDialog.  Do
 * not confuse a PropertySheetDialog with the wrongly named PropertySheet class.
 *
 * A control dialog is also a child of the owner / parent dialog.  The
 * popupAsChild() method also uses the concept of parent / child dialogs.  To
 * prevent confusion with that, the control dialog classes use the terms control
 * dialog and owner dialog rather than child and parent dialog .
 */

::class 'ControlDialog' mixinclass Object public

::attribute ownerDialog get  external "LIBRARY oodialog chld_getOwnerDialog"
::attribute ownerDialog set  external "LIBRARY oodialog chld_setOwnerDialog"


::class 'ResControlDialog' public subclass ResDialog inherit ControlDialog

::method startDialog private external "LIBRARY oodialog resCtrlDlg_startDialog_pvt"


::class 'UserControlDialog' public subclass UserDialog inherit ControlDialog

-- Over-ride the superclass startIt().  The superclass startIt() creates a top-
-- level dialog and takes an 'icon' and a 'modeless' icon.  We need to accept
-- those arguments, but we don't use them.  So we just ignore all args
::method startIt

   if self~basePtr = 0 then return 0

   if self~startChildDialog(self~basePtr, 0) == 0 then return 0

   self~initDialog
   return self~dlgHandle


::class 'RcControlDialog' subclass UserControlDialog public

::method init
  use strict arg rcFile, idDlg, initData. = .nil, includeFile = "", options = "", expected = 200, ownerDlg = .nil

  newArgs = .array~new(5)
  if arg(3, 'E') then newArgs[1] = arg(3)
  if arg(4, 'E') then newArgs[2] = arg(4)
  if arg(7, 'E') then newArgs[5] = arg(7)
  newArgs[3] = rcFile
  newArgs[4] = idDlg

  forward class (super) arguments (newArgs) continue
  if result <> 0 then return result

  self~initCode = self~load(rcFile, idDlg, options, expected)

  return self~initCode


::class 'OwnerDialog' mixinclass Object public

::method init
  say 'In OwnerDialog::init()'
  --forward class (super) continue
  say 'Back in OwnerDialog::init()'


/* Property sheets and pages */

::class 'PropertySheetDialog' public subclass PlainBaseDialog

-- WM_USER == 0x400
-- ::constant PSM_INDEXTOID             "0x0486"


::attribute pages get unguarded external "LIBRARY oodialog psdlg_getPages_atr"

::method init external "LIBRARY oodialog psdlg_init"

::method initDialog unguarded
  return 0

::method execute external "LIBRARY oodialog psdlg_execute"
::method popup external "LIBRARY oodialog psdlg_popup"

::method getPage unguarded external "LIBRARY oodialog psdlg_getPage"
::method changed unguarded external "LIBRARY oodialog psdlg_changed"
::method unchanged unguarded external "LIBRARY oodialog psdlg_unchanged"
::method setWizButtons unguarded external "LIBRARY oodialog psdlg_setWizButtons"
::method showWizButtons unguarded external "LIBRARY oodialog psdlg_showWizButtons"
::method indexToID external "LIBRARY oodialog psdlg_indexToID"


::method test unguarded external "LIBRARY oodialog psdlg_test"


::class 'PropertySheetPage' mixinclass Object public

::constant PSNRET_NOERROR                     0
::constant PSNRET_INVALID                     1
::constant PSNRET_INVALID_NOCHANGEPAGE        2
::constant PSNRET_MESSAGEHANDLED              3

::method init class external "LIBRARY oodialog psp_init_cls"

::attribute propSheet get external "LIBRARY oodialog psp_propSheet_atr"
::attribute wasActivated get external "LIBRARY oodialog psp_wasActivated_atr"

::attribute cx get external "LIBRARY oodialog psp_getcx"
::attribute cx set external "LIBRARY oodialog psp_setcx"

::attribute cy get external "LIBRARY oodialog psp_getcx"
::attribute cy set external "LIBRARY oodialog psp_setcx"

::attribute pageTitle get external "LIBRARY oodialog psp_getPageTitle"
::attribute pageTitle set external "LIBRARY oodialog psp_setPageTitle"

::attribute headerTitle get external "LIBRARY oodialog psp_getPageTitle"
::attribute headerTitle set external "LIBRARY oodialog psp_setPageTitle"

::attribute headerSubtitle get external "LIBRARY oodialog psp_getPageTitle"
::attribute headerSubtitle set external "LIBRARY oodialog psp_setPageTitle"

::attribute wantAccelerators get external "LIBRARY oodialog psp_getWantNotification"
::attribute wantAccelerators set external "LIBRARY oodialog psp_setWantNotification"

::method init_propertySheetPage private external "LIBRARY oodialog psp_init_propertySheetPage"  -- Not to be documented
::method initTemplate external "LIBRARY oodialog psp_initTemplate"                              -- Not to be documented

::method execute unguarded
    self~initDialog
    if self~useStem then self~setDataStem(self~dlgData)
    else self~setData

    do while self~automaticMethods~Items > 0
        p = self~automaticMethods~pull
        m = self~automaticMethods~pull
        c = self~automaticMethods~pull
        o = c~new(p, self)
        o~start(m)
    end

::method endExecute unguarded

    self~finished = .true
    self~stopIt

::method getObject unguarded
    return 0

::method setActive unguarded
    use strict arg propSheet
    return 0

::method killActive unguarded
    use strict arg propSheet
    return self~validatePage(propSheet)

::method queryCancel unguarded
    use strict arg propSheet
    self~initCode = 2
    return .true

::method queryInitialFocus unguarded
    use strict arg idDefFocus, propSheet
    return 0

::method reset unguarded
  use strict arg isCancelButton, propSheet
  self~initCode = 2
  return 0

::method help unguarded
    use strict arg propSheet
    return 0

::method translateAccelerator unguarded
    use strict arg msg, keyCode, d, propsheet
    return self~PSNRET_NOERROR

::method wizBack unguarded
    use strict arg propSheet
    return 0

::method wizNext unguarded
    use strict arg propSheet
    return 0

::method wizFinish unguarded
    use strict arg propSheet
    return 0

::method apply unguarded
    use strict arg isOkButton, propSheet

    ret = self~validate(isOkButton, propsheet)
    if ret == self~PSNRET_NOERROR then return self~applyNoError(propSheet)
    else return ret

::method validate private unguarded
    use strict arg isOkButton, propSheet
    return self~PSNRET_NOERROR

::method validatePage private unguarded
    use strict arg propSheet
    return .true

::method applyNoError private unguarded
    use strict arg propSheet

    self~initCode = 1
    if self~useStem then self~getDataStem(dlgData.)
    self~getData
    propSheet~unchanged(self)
    return self~PSNRET_NOERROR


::class 'ResPSPDialog' public subclass ResDialog inherit PropertySheetPage

::method init external "LIBRARY oodialog respspdlg_init"

::method execute unguarded
    -- if autodetection is on, create the data attributes
    if self~autoDetect then do
       ids = self~getDataTableIDs
       do id over ids
          text = self~getControlText(id)
          self~addAttribute(id, text)
       end
    end
    forward class (.PropertySheetPage)

::method validate unguarded
    forward class (.PropertySheetPage)

-- Over-ride these methods with a do nothing so that the user can not start the dialog
::method popup unguarded
::method popupAsChild unguarded
::method executeAsync unguarded
::method endAsyncExecution unguarded



::class 'UserPSPDialog' public subclass UserDialog inherit PropertySheetPage

::method init external "LIBRARY oodialog userpspdlg_init"

::method execute unguarded
    forward class (.PropertySheetPage)

::method validate unguarded
    forward class (.PropertySheetPage)

-- Over-ride these methods with a do nothing so that the user can not start the dialog
::method popup unguarded
::method popupAsChild unguarded
::method executeAsync unguarded
::method endAsyncExecution unguarded



::class 'RcPSPDialog' public subclass UserDialog inherit PropertySheetPage

::method init external "LIBRARY oodialog rcpspdlg_init"
::method startTemplate external "LIBRARY oodialog rcpspdlg_startTemplate"

::method loadFrame private
  use strict arg expected
  return self~loadFrame:super(self~library, self~resourceID, "", expected)

::method loadItems
  use strict arg opts
  return self~loadItems:super(self~library, self~resourceID, opts)

::method execute unguarded
    forward class (.PropertySheetPage)

::method validate unguarded
    forward class (.PropertySheetPage)

-- Over-ride these methods with a do nothing so that the user can not start the dialog
::method popup unguarded
::method popupAsChild unguarded
::method executeAsync unguarded
::method endAsyncExecution unguarded

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2014 Rexx Language Association. All rights reserved.    */
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
 * PropertySheetDialog, PropertySheetPage dialog, and ControlDialog  classes,
 * subclasses, and related interfaces.
 *
 * Do  not confuse a PropertySheetDialog with the wrongly named PropertySheet
 * class. The PropertySheet class is deprecated and not supported.
 *
 * PropertySheetDialog dialogs differ slightly from the normal ooDialog dialogs
 * because the underlying dialog is created and maintained by the operating
 * system.  ooDialog has no window procedure for a PropertySheetDialog, the
 * operating system is executing the window procedure.  This introduces some
 * subtle differences in the behavior of the dialog.  The ooDialog framework
 * can not define the dialog template, or receive the underly Property Sheet
 * window messages
 *
 * PropertySheetPage dialogs serve as the pages of the PropertySheetDialog.
 *
 * A control dialog is a dialog with a style that allows it work well as a
 * dialog within a top-level dialog.  The top-level dialog is both the owner and
 * the parent of the control dialog.  These dialogs work well as the content for
 * tab control pages.
 *
 * A control dialog is also a child of the owner / parent dialog.  The
 * popupAsChild() method also uses the concept of parent / child dialogs.  To
 * prevent confusion with that, the control dialog classes use the terms control
 * dialog and owner dialog rather than child and parent dialog .
 */



/*                    Property sheets and pages                               */

::class 'PropertySheetDialog' public subclass PlainBaseDialog

-- WM_USER == 0x400
::constant PSM_REBOOTSYSTEM             "0x046A"
::constant PSM_RESTARTWINDOWS           "0x0469"
::constant PSM_RECALCPAGESIZES          "0x0488"


::constant MAXPROPPAGES                 100
::constant ID_PSRESTARTWINDOWS            2
::constant ID_PSREBOOTSYSTEM              3  -- (ID_PSRESTARTWINDOWS | 0x1)

::constant IDC_TAB_IN_PROPSHEET            12320
::constant IDC_PB_APPLY_IN_PROPSHEET       12321


-- The application icon for the property sheet dialog
::attribute appIcon get unguarded
::attribute appIcon set unguarded external "LIBRARY oodialog psdlg_setAppIcon_atr"

-- The caption (title) for the property sheet dialog
::attribute caption get unguarded
::attribute caption set unguarded external "LIBRARY oodialog psdlg_setCaption_atr"

::attribute header get unguarded
::attribute header set unguarded external "LIBRARY oodialog psdlg_setHeader_atr"

-- Icons for each tab can come from an ImageList owned by the property dialog
::attribute imageList get unguarded
::attribute imageList set unguarded external "LIBRARY oodialog psdlg_setImageList_atr"

::attribute pages get unguarded external "LIBRARY oodialog psdlg_getPages_atr"

-- A resource image from which to get resources for the property sheet dialog
::attribute resources get unguarded
::attribute resources set unguarded external "LIBRARY oodialog psdlg_setResources_atr"

::attribute startPage get unguarded
::attribute startPage set unguarded external "LIBRARY oodialog psdlg_setStartPage_atr"

::attribute watermark get unguarded
::attribute watermark set unguarded external "LIBRARY oodialog psdlg_setWatermark_atr"

::method init external "LIBRARY oodialog psdlg_init"

::method addPage unguarded external "LIBRARY oodialog psdlg_addPage"
::method apply unguarded external "LIBRARY oodialog psdlg_apply"
::method cancelToClose unguarded external "LIBRARY oodialog psdlg_cancelToClose"
::method changed unguarded external "LIBRARY oodialog psdlg_changed"
::method enableWizButtons external "LIBRARY oodialog psdlg_enableWizButtons"
::method execute external "LIBRARY oodialog psdlg_execute"
::method getCurrentPageHwnd unguarded external "LIBRARY oodialog psdlg_getCurrentPageHwnd"
::method getPage unguarded external "LIBRARY oodialog psdlg_getPage"
::method getResult unguarded external "LIBRARY oodialog psdlg_getResult"
::method getTabControl unguarded external "LIBRARY oodialog psdlg_getTabControl"
::method hwndToIndex unguarded external "LIBRARY oodialog psdlg_hwndToIndex"
::method idToIndex unguarded external "LIBRARY oodialog psdlg_idToIndex"
::method indexToHwnd unguarded external "LIBRARY oodialog psdlg_indexToHandle"
::method indexToID unguarded external "LIBRARY oodialog psdlg_indexToID"
::method indexToPage unguarded external "LIBRARY oodialog psdlg_indexToHandle"
::method initDialog unguarded
    return 0
::method insertPage unguarded external "LIBRARY oodialog psdlg_insertPage"
::method pageToIndex unguarded external "LIBRARY oodialog psdlg_pageToIndex"
::method popup external "LIBRARY oodialog psdlg_popup"
::method popupAsChild external "LIBRARY oodialog psdlg_popupAsChild"
::method pressButton unguarded external "LIBRARY oodialog psdlg_pressButton"
::method querySiblings unguarded external "LIBRARY oodialog psdlg_querySiblings"
::method rebootSystem unguarded
    use strict arg
    self~sendWinIntMsg(self~PSM_REBOOTSYSTEM, 0, 0)
    return 0

::method recalcPageSizes unguarded
    use strict arg
    return self~sendWinIntMsg(self~PSM_RECALCPAGESIZES, 0, 0) != 0

::method restartWindows unguarded
    use strict arg
    self~sendWinIntMsg(self~PSM_RESTARTWINDOWS, 0, 0)
    return 0

::method removePage unguarded external "LIBRARY oodialog psdlg_removePage"
::method setCurSel unguarded external "LIBRARY oodialog psdlg_setCurSel"
::method setCurSelByID unguarded external "LIBRARY oodialog psdlg_setCurSelByID"
::method setButtonText unguarded external "LIBRARY oodialog psdlg_setButtonText"
::method setFinishText unguarded external "LIBRARY oodialog psdlg_setFinishText"
::method setHeaderSubTitle unguarded external "LIBRARY oodialog psdlg_resetPageText"
::method setHeaderTitle unguarded external "LIBRARY oodialog psdlg_resetPageText"
::method setNextText unguarded external "LIBRARY oodialog psdlg_setNextText"
::method setTitle unguarded external "LIBRARY oodialog psdlg_setTitle"
::method setWizButtons unguarded external "LIBRARY oodialog psdlg_setWizButtons"
::method showWizButtons unguarded external "LIBRARY oodialog psdlg_showWizButtons"
::method unchanged unguarded external "LIBRARY oodialog psdlg_unchanged"

::method test unguarded external "LIBRARY oodialog psdlg_test"


::class 'PropertySheetPage' mixinclass Object public

::constant PSNRET_NOERROR                     0
::constant PSNRET_INVALID                     1
::constant PSNRET_INVALID_NOCHANGEPAGE        2
::constant PSNRET_MESSAGEHANDLED              3
::constant PROP_SM_CXDLG                      212
::constant PROP_SM_CYDLG                      188
::constant PROP_MED_CXDLG                     227
::constant PROP_MED_CYDLG                     215
::constant PROP_LG_CXDLG                      252
::constant PROP_LG_CYDLG                      218

::method init class external "LIBRARY oodialog psp_init_cls"                                    -- Not to be documented

::attribute cx get external "LIBRARY oodialog psp_getcx"
::attribute cx set external "LIBRARY oodialog psp_setcx"

::attribute cy get external "LIBRARY oodialog psp_getcx"
::attribute cy set external "LIBRARY oodialog psp_setcx"

::attribute headerSubtitle get external "LIBRARY oodialog psp_getPageTitle"
::attribute headerSubtitle set external "LIBRARY oodialog psp_setPageTitle"

::attribute headerTitle get external "LIBRARY oodialog psp_getPageTitle"
::attribute headerTitle set external "LIBRARY oodialog psp_setPageTitle"

::attribute pageID get external "LIBRARY oodialog psp_pageID_atr"
::attribute pageNumber get external "LIBRARY oodialog psp_pageNumber_atr"

::attribute pageTitle get external "LIBRARY oodialog psp_getPageTitle"
::attribute pageTitle set external "LIBRARY oodialog psp_setPageTitle"

::attribute propSheet get unguarded external "LIBRARY oodialog psp_propSheet_atr"

::attribute resources get unguarded
::attribute resources set external "LIBRARY oodialog psp_setResources_atr"

::attribute tabIcon get unguarded
::attribute tabIcon set external "LIBRARY oodialog psp_setTabIcon_atr"

::attribute wantAccelerators get external "LIBRARY oodialog psp_getWantNotification"
::attribute wantAccelerators set external "LIBRARY oodialog psp_setWantNotification"

::attribute wantGetObject get external "LIBRARY oodialog psp_getWantNotification"
::attribute wantGetObject set external "LIBRARY oodialog psp_setWantNotification"

::attribute wasActivated get external "LIBRARY oodialog psp_wasActivated_atr"


::method endExecute unguarded                                                                   -- Not to be documented
    self~finished = .true
    self~stopIt
::method execute unguarded                                                                      -- Not to be documented
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
::method init_propertySheetPage private external "LIBRARY oodialog psp_init_propertySheetPage"  -- Not to be documented
::method initTemplate external "LIBRARY oodialog psp_initTemplate"                              -- Not to be documented


::method apply unguarded
    use strict arg isOkButton, pageIndex, pageCurrent, psDlg

    ret = self~validate(isOkButton, psDlg)
    if ret == self~PSNRET_NOERROR then return self~applyNoError(psDlg)
    else return ret

::method applyNoError private unguarded                                          -- Document ??
    use strict arg psDlg

    self~initCode = 1

    if self~useStem then self~getDataStem(self~dlgData)
    self~getData

    psDlg~unchanged(self)
    return self~PSNRET_NOERROR

::method getObject unguarded
    return 0

::method help unguarded
    use strict arg psDlg
    return 0

::method killActive unguarded
    use strict arg psDlg
    return self~validatePage(psDlg)

::method pageCreate unguarded
    use strict arg psDlg
    return .true

::method queryCancel unguarded
    use strict arg psDlg
    self~initCode = 2
    return .true

::method queryFromSibling unguarded
    use strict arg arg1, arg2, psDlg
    return 0

::method queryInitialFocus unguarded
    use strict arg idDefFocus, psDlg
    return 0

::method reset unguarded
  use strict arg isCancelButton, psDlg
  self~initCode = 2
  return 0

::method setActive unguarded
    use strict arg psDlg
    return 0

::method setSize external "LIBRARY oodialog psp_setSize"
::method translateAccelerator unguarded
    use strict arg msg, keyCode, d, psDlg
    return self~PSNRET_NOERROR

::method wizBack unguarded
    use strict arg psDlg
    return 0

::method wizFinish unguarded
    use strict arg psDlg
    return 0

::method wizNext unguarded
    use strict arg psDlg
    return 0

::method validate private unguarded
    use strict arg isOkButton, psDlg
    return self~PSNRET_NOERROR

::method validatePage private unguarded
    use strict arg psDlg
    return .true


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

::method loadItems private
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



/*            Owner and Control Dialogs                                       */

::class 'ResControlDialog' public subclass ResDialog inherit ControlDialog

::method startIt

   if self~startDialog(self~library, self~resourceID) then do
      -- if autodetection is on, create the data attributes
      if self~autoDetect then do
         ids = self~getDataTableIDs
         do id over ids
            text = self~getControlText(id)
            self~addAttribute(id, text)
         end
      end

      self~initDialog
      self~initializing = .false
      return .true
   end

   return .false

::method startDialog private external "LIBRARY oodialog resCtrlDlg_startDialog_pvt"


::method execute
    self~start("BEGINEXECUTION")
::method popup
    self~start("BEGINEXECUTION")
::method popupAsChild
    self~start("BEGINEXECUTION")
::method executeAsync abstract
::method endAsyncExecution abstract


::class 'UserControlDialog' public subclass UserDialog inherit ControlDialog

-- Over-ride the superclass startIt().  This is invoked from
-- ControlDialog::beginExecution.
::method startIt

    if self~basePtr = 0 then return .false

    if self~startChildDialog(self~basePtr, 0) == 0 then return .false

    self~initDialog
    self~initializing = .false;
    return .true


::method execute
    self~start("BEGINEXECUTION")
::method popup
    self~start("BEGINEXECUTION")
::method popupAsChild
    self~start("BEGINEXECUTION")
::method executeAsync abstract
::method endAsyncExecution abstract


::class 'RcControlDialog' subclass UserControlDialog public

::method init
    use strict arg rcFile, idDlg, initData. = .nil, includeFile = "", options = "", expected = 200, ownerData = .nil

    newArgs = .array~new(5)
    if arg(3, 'E') then newArgs[1] = arg(3)
    if arg(4, 'E') then newArgs[2] = arg(4)
    if arg(7, 'E') then newArgs[5] = arg(7)
    newArgs[3] = rcFile
    newArgs[4] = idDlg

    forward class (super) arguments (newArgs) continue
    if result <> 0 then return result

    self~extraOptions = options

    if \ self~isManaged then do
      self~initCode = self~load(rcFile, idDlg, options, expected)
    end

    return self~initCode

::method startTemplate external "LIBRARY oodialog rcCtrlDlg_startTemplate"

::method loadFrameRCCD
  use strict arg expected
  return self~loadFrame:super(self~library, self~resourceID, "", expected)

::method loadItemsRCCD
  use strict arg opts
  return self~loadItems:super(self~library, self~resourceID, opts)

::method execute
    self~start("BEGINEXECUTION")
::method popup
    self~start("BEGINEXECUTION")
::method popupAsChild
    self~start("BEGINEXECUTION")
::method executeAsync abstract
::method endAsyncExecution abstract


::class 'ControlDialog' mixinclass Object public

::attribute wasActivated get external "LIBRARY oodialog cd_get_wasActivated"

::method endExecution unguarded
    use strict arg isOkay = .false

    if isOkay, self~wasActivated then do
        self~initCode = 1
        if self~useStem then self~getDataStem(self~dlgData)
        self~getData
    end
    else do
        self~initCode = 2
    end
    self~finished = .true

    self~stopIt
    return self~initCode

::method initUpdateListView private
  use strict arg lvID, pause = .005

  self~addNewAttribute('updateListViewID', lvID, .true)

  src = .array~new
  src[1]  = "use strict arg hwnd"
  src[2]  = ""
  src[3]  = "lv = self~newListView(self~updateListViewID)"
  src[4]  = "if lv == .nil then return .false"
  src[5]  = ""
  src[6]  = "if hwnd == lv~hwnd then do"
  src[7]  = "  j = SysSleep("pause")"
  src[8]  = "  lv~redraw"
  src[9]  = "  return .true"
  src[10] = "end"
  src[11] = "return .false"
  self~setMethod('updateListView', src)

  owner = self~ownerDialog

  owner~addNewAttribute(updateListViewPageDialog, self, .true, .true)
  owner~addNewAttribute(updateListViewLastFocused, 0, .true, .true)

  src = .array~new
  src[1]  = "expose updateListViewPageDialog updateListViewLastFocused"
  src[2]  = "use arg flag, hwnd, hFocused, isMinimized"
  src[3]  = ""
  src[4]  = "reply .false"
  src[5]  = ""
  src[6]  = "if flag == INACTIVE then updateListViewLastFocused = hFocused"
  src[7]  = "else updateListViewPageDialog~updateListView(updateListViewLastFocused)"
  owner~addNewMethod('onUpdateListViewActivate', src, .true)

  owner~connectActivate('onUpdateListViewActivate')

/**   Do not document classes and methods from here to end of the file.
 *
 * Most of the classes will not be documented for 4.2.0 because they are not
 * ready.  They will be inlcuded in the release after 4.2.0.
 */


::method init class external "LIBRARY oodialog cd_init_cls"                                     -- Not to be documented

/*::attribute pageNumber get external "LIBRARY oodialog cd_get_pageNumber"
::attribute pageNumber set external "LIBRARY oodialog cd_set_pageNumber"*/

::attribute initializing get external "LIBRARY oodialog cd_get_initializing"
::attribute initializing set external "LIBRARY oodialog cd_set_initializing"

::attribute isManaged get external "LIBRARY oodialog cd_get_isManaged"

::attribute pageTitle get external "LIBRARY oodialog cd_get_pageTitle"
::attribute pageTitle set external "LIBRARY oodialog cd_set_pageTitle"

::attribute extraOptions get external "LIBRARY oodialog cd_get_extraOptions"
::attribute extraOptions set external "LIBRARY oodialog cd_set_extraOptions"

/*::attribute size get external "LIBRARY oodialog ctrlDlg_get_size"
::attribute size set external "LIBRARY oodialog ctrlDlg_set_size"

::attribute wantAllNotifications get external "LIBRARY oodialog ctrlDlg_get_wantAllNotifications"
::attribute wantAllNotifications set external "LIBRARY oodialog ctrlDlg_set_wantAllNotifications"

::attribute resources get unguarded
::attribute resources set external "LIBRARY oodialog psp_setResources_atr"

::attribute tabIcon get unguarded
::attribute tabIcon set external "LIBRARY oodialog psp_setTabIcon_atr"*/


::method controlDlgInit private external "LIBRARY oodialog cd_controlDlgInit"

::method beginExecution unguarded

    dlgData. = self~dlgData

    if \ self~isManaged then do
        if \ self~startIt then do
            self~finished = .true
            self~stopIt
            return 0
        end
    end

    if self~useStem then self~setDataStem(dlgData.)
    else self~setData

    do while self~automaticMethods~Items > 0
        p = self~automaticMethods~pull
        m = self~automaticMethods~pull
        c = self~automaticMethods~pull
        o = c~new(p, self)
        o~start(m)
    end

    self~waitForDialog

    return 0


-- Return .true, it's okay to leave the page, .false it is not okay, stay on the page
::method tabPageKillActive unguarded
  use strict arg owner, tab
  say "In ControlDialog::tabPageKillActive dlg:" self 'owner:' owner 'tab:' tab
  return .true

-- Return 0, accept the activation, otherwise page number of alternate page to set focus to.
::method tabPageSetActive unguarded
  use strict arg owner, tab
  say "In ControlDialog::tabPageSetActive dlg:" self 'owner:' owner 'tab:' tab
  return 0



::class 'TabOwnerDialog' mixinclass Object public

::method tabOwnerDlginit external "LIBRARY oodialog tod_tabOwnerDlgInit"
::method getTabPage external "LIBRARY oodialog tod_getTabPage"
::method tabOwnerOk external "LIBRARY oodialog tod_tabOwnerOk"
::method tabOwnerCancel external "LIBRARY oodialog tod_tabOwnerCancel"

-- Return .true, it's okay to leave the page, .false it is not okay, stay on the page
::method tabOwnerKillActive unguarded
  use strict arg pageDlg, tab
  say "In TabOwnerDialog::tabOwnerKillActive dlg:" self 'page:' pageDlg 'tab:' tab
  return .true

-- Return 0, accept the activation, otherwise page number of alternate page to set focus to.
::method tabOwnerSetActive unguarded
  use strict arg pageDlg, tab
  say "In TabOwnerDialog::tabOwnerSetActive dlg:" self 'page:' pageDlg 'tab:' tab
  return 0

::method tabOwnerSelChange unguarded
  use strict arg pageDlg, pageNumber
  return 0

::method tabOwnerSelChanging unguarded
  use strict arg pageDlg, pageNumber
  return 0


::class 'TabOwnerDlgInfo' subclass Object public

::method init external "LIBRARY oodialog todi_init"
::method add external "LIBRARY oodialog todi_add"


::class 'ManagedTab' subclass Object public

::method init external "LIBRARY oodialog mt_init"


::class 'ControlDlgInfo' subclass Object public

::attribute managed set external "LIBRARY oodialog cdi_set_managed"
::attribute title set external "LIBRARY oodialog cdi_set_title"
::attribute size set external "LIBRARY oodialog cdi_set_size"
::method init external "LIBRARY oodialog cdi_init"




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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * ooDialog classes that are more Shell orientated than the original ooDialog
 * implementation.
 *
 * BrowseForFolder     Main 'browse for folder' class, allows customization.
 *
 * SimpleFolderBrowse  Simplified version of BrowseForFolder with ???
 *
 */


::class 'BrowseForFolder' public

::attribute banner get external "LIBRARY oodialog bff_banner"
::attribute banner set external "LIBRARY oodialog bff_setBanner"
::attribute dlgTitle get external "LIBRARY oodialog bff_dlgTitle"
::attribute dlgTitle set external "LIBRARY oodialog bff_setDlgTitle"
::attribute hint get external "LIBRARY oodialog bff_hint"
::attribute hint set external "LIBRARY oodialog bff_setHint"
::attribute initialThread get external "LIBRARY oodialog bff_initialThread"
::attribute options get external "LIBRARY oodialog bff_options"
::attribute options set external "LIBRARY oodialog bff_setOptions"
::attribute owner get external "LIBRARY oodialog bff_Owner"
::attribute owner set external "LIBRARY oodialog bff_setOwner"
::attribute root get external "LIBRARY oodialog bff_root"
::attribute root set external "LIBRARY oodialog bff_setRoot"
::attribute startDir get external "LIBRARY oodialog bff_startDir"
::attribute startDir set external "LIBRARY oodialog bff_setStartDir"
::attribute usePathForHint get external "LIBRARY oodialog bff_usePathForHint"
::attribute usePathForHint set external "LIBRARY oodialog bff_setUsePathForHint"

::method init external "LIBRARY oodialog bff_init"
::method uninit external "LIBRARY oodialog bff_uninit"

::method getDisplayName external "LIBRARY oodialog bff_getDisplayName"
::method getFolder external "LIBRARY oodialog bff_getFolder"
::method getItemIDList external "LIBRARY oodialog bff_getItemIDList"
::method initCOM external "LIBRARY oodialog bff_initCOM"
::method releaseCOM external "LIBRARY oodialog bff_releaseCOM"
::method releaseItemIDList external "LIBRARY oodialog bff_releaseItemIDList"
::method test external "LIBRARY oodialog bff_test"


::class 'CommonItemDialog' public inherit ComConstants

::attribute options get external "LIBRARY oodialog cid_options"
::attribute options set external "LIBRARY oodialog cid_setOptions"

::method init external "LIBRARY oodialog cid_init"
::method uninit external "LIBRARY oodialog cid_uninit"

::method addPlace external "LIBRARY oodialog cid_addPlace"
::method advise external "LIBRARY oodialog cid_advise"
::method clearClientData external "LIBRARY oodialog cid_clearClientData"
::method close unguarded external "LIBRARY oodialog cid_close"
::method getCurrentSelection unguarded external "LIBRARY oodialog cid_getCurrentSelection"
::method getFileName unguarded external "LIBRARY oodialog cid_getFileName"
::method getFileTypeIndex unguarded external "LIBRARY oodialog cid_getFileTypeIndex"
::method getFolder external "LIBRARY oodialog cid_getFolder"
::method getResult external "LIBRARY oodialog cid_getResult"
::method initCOM unguarded external "LIBRARY oodialog cid_initCOM"
::method isReleased unguarded external "LIBRARY oodialog cid_isReleased"
::method release external "LIBRARY oodialog cid_release"
::method releaseCOM unguarded external "LIBRARY oodialog cid_releaseCOM"
::method setClientGuid external "LIBRARY oodialog cid_setClientGuid"
::method setDefaultExtension external "LIBRARY oodialog cid_setDefaultExtension"
::method setDefaultFolder external "LIBRARY oodialog cid_setDefaultFolder"
::method setFileName external "LIBRARY oodialog cid_setFileName"
::method setFileNameLabel external "LIBRARY oodialog cid_setFileNameLabel"
::method setFileTypeIndex external "LIBRARY oodialog cid_setFileTypeIndex"
::method setFileTypes external "LIBRARY oodialog cid_setFileTypes"
::method setFilter external "LIBRARY oodialog cid_setFilter"
::method setFolder external "LIBRARY oodialog cid_setFolder"
::method setCancelButtonLabel external "LIBRARY oodialog cid_setCancelButtonLabel"
::method setOkButtonLabel external "LIBRARY oodialog cid_setOkButtonLabel"
::method setTitle external "LIBRARY oodialog cid_setTitle"
::method show unguarded external "LIBRARY oodialog cid_show"
::method unadvise unguarded external "LIBRARY oodialog cid_unadvise"


::class 'OpenFileDialog' public subclass CommonItemDialog
::method init external "LIBRARY oodialog ofd_init"
::method getResults external "LIBRARY oodialog ofd_getResults"


::class 'SaveFileDialog' public subclass CommonItemDialog
::method init external "LIBRARY oodialog sfd_init"
::method setSaveAsItem external "LIBRARY oodialog sfd_setSaveAsItem"


::class 'CommonDialogCustomizations' public mixinclass Object
::method initCustomizations private external "LIBRARY oodialog cid_initCustomizations"  -- Internal use only
::method addCheckButton external "LIBRARY oodialog cid_addCheckButton"
::method addComboBox external "LIBRARY oodialog cid_addComboBox"
::method addControlItem external "LIBRARY oodialog cid_addControlItem"
::method addEditBox external "LIBRARY oodialog cid_addEditBox"
::method addMenu external "LIBRARY oodialog cid_addMenu"
::method addPushButton external "LIBRARY oodialog cid_addPushButton"
::method addRadioButtonList external "LIBRARY oodialog cid_addRadioButtonList"
::method addSeparator external "LIBRARY oodialog cid_addSeparator"
::method addText external "LIBRARY oodialog cid_addText"
::method check external "LIBRARY oodialog cid_setCheckButtonState"
::method enableOpenDropDown external "LIBRARY oodialog cid_enableOpenDropDown"
::method endVisualGroup external "LIBRARY oodialog cid_endVisualGroup"
::method getCheckButtonState external "LIBRARY oodialog cid_getCheckButtonState"
::method getControlItemState external "LIBRARY oodialog cid_getControlItemState"
::method getControlState external "LIBRARY oodialog cid_getControlState"
::method getEditBoxText external "LIBRARY oodialog cid_getEditBoxText"
::method getSelectedControlItem external "LIBRARY oodialog cid_getSelectedControlItem"
::method isChecked external "LIBRARY oodialog cid_getCheckButtonState"
::method makeProminent external "LIBRARY oodialog cid_makeProminent"
::method removeAllControlItems external "LIBRARY oodialog cid_removeAllControlItems"
::method removeControlItem external "LIBRARY oodialog cid_removeControlItem"
::method setCheckButtonState external "LIBRARY oodialog cid_setCheckButtonState"
::method setControlItemState external "LIBRARY oodialog cid_setControlItemState"
::method setControlItemText external "LIBRARY oodialog cid_setControlItemText"
::method setControlLabel external "LIBRARY oodialog cid_setControlLabel"
::method setControlState external "LIBRARY oodialog cid_setControlState"
::method setEditBoxText external "LIBRARY oodialog cid_setEditBoxText"
::method setSelectedControlItem external "LIBRARY oodialog cid_setSelectedControlItem"
::method startVisualGroup external "LIBRARY oodialog cid_startVisualGroup"
::method uncheck external "LIBRARY oodialog cid_setCheckButtonState"

::class 'ComConstants' mixinclass Object
::constant  CANCELED          2147943623
::constant  E_NOTIMPL         34360000513  -- _HRESULT_TYPEDEF_(0x80004001L)
::constant  S_FALSE           1
::constant  S_OK              0


::class 'ShellItemFilter' public inherit ComConstants
::method init external "LIBRARY oodialog sif_init"
::method includeItem unguarded
    return self~S_OK


::class 'CommonDialogEvents' public inherit ComConstants

-- OnOverwrite and OnShareViolation response constants
::constant  FDESVR_DEFAULT    0
::constant  FDESVR_ACCEPT     1
::constant  FDESVR_REFUSE     2


::method init external "LIBRARY oodialog cde_init"
::method onButtonClicked unguarded
    return self~S_OK

::method onCheckButtonToggled unguarded
    return self~S_OK

::method onControlActivating unguarded
    return self~S_OK

::method onFileOk unguarded
    return self~S_OK

::method onFolderChange unguarded
    return self~S_OK

::method onFolderChanging unguarded
    return self~S_OK

::method onHelp unguarded
    return self~S_OK

::method onItemSelected unguarded
    return self~S_OK

::method onOverwrite unguarded
    return self~FDESVR_DEFAULT

::method onSelectionChange unguarded
    return self~S_OK

::method onShareViolation unguarded
    return self~FDESVR_DEFAULT

::method onTypeChange unguarded
    return self~S_OK


::class 'SimpleFolderBrowse' public
::method getFolder class external "LIBRARY oodialog sfb_getFolder"


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
 * oodPackageEntry.cpp
 *
 * Contains the package entry point, routine and method declarations, and
 * routine and method tables for the native API.  Also contains the global
 * variables and DLLMain().
 */

#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

HINSTANCE MyInstance = NULL;
DIALOGADMIN * DialogTab[MAXDIALOGS] = {NULL};
DIALOGADMIN * topDlg = {NULL};
INT StoredDialogs = 0;
CRITICAL_SECTION crit_sec = {0};

// Initialized in dlgutil_init_cls
RexxObjectPtr TheTrueObj = NULLOBJECT;
RexxObjectPtr TheFalseObj = NULLOBJECT;
RexxObjectPtr TheNilObj = NULLOBJECT;
RexxPointerObject TheNullPtrObj = NULLOBJECT;
RexxDirectoryObject TheDotLocalObj = NULLOBJECT;
RexxObjectPtr TheZeroObj = NULLOBJECT;
RexxObjectPtr TheOneObj = NULLOBJECT;
RexxObjectPtr TheNegativeOneObj = NULLOBJECT;

#ifdef __cplusplus
extern "C" {
#endif

BOOL REXXENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        MyInstance = hinstDLL;
        InitializeCriticalSection(&crit_sec);
    }
    else if ( fdwReason == DLL_PROCESS_DETACH )
    {
        MyInstance = NULL;
        DeleteCriticalSection(&crit_sec);
    }
    return(TRUE);
}

#ifdef __cplusplus
}
#endif


REXX_CLASSIC_ROUTINE_PROTOTYPE(GetDlgMsg);
REXX_CLASSIC_ROUTINE_PROTOTYPE(SendWinMsg);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleDlg);
REXX_CLASSIC_ROUTINE_PROTOTYPE(AddUserMessage);
REXX_CLASSIC_ROUTINE_PROTOTYPE(GetFileNameWindow);
REXX_CLASSIC_ROUTINE_PROTOTYPE(DataTable);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleDialogAdmin);
REXX_CLASSIC_ROUTINE_PROTOTYPE(SetItemData);
REXX_CLASSIC_ROUTINE_PROTOTYPE(SetStemData);
REXX_CLASSIC_ROUTINE_PROTOTYPE(GetItemData);
REXX_CLASSIC_ROUTINE_PROTOTYPE(GetStemData);
REXX_CLASSIC_ROUTINE_PROTOTYPE(Wnd_Desktop);
REXX_CLASSIC_ROUTINE_PROTOTYPE(WndShow_Pos);
REXX_CLASSIC_ROUTINE_PROTOTYPE(WinAPI32Func);
REXX_CLASSIC_ROUTINE_PROTOTYPE(InfoMessage);
REXX_CLASSIC_ROUTINE_PROTOTYPE(ErrorMessage);
REXX_CLASSIC_ROUTINE_PROTOTYPE(YesNoMessage);
REXX_CLASSIC_ROUTINE_PROTOTYPE(FindTheWindow);
REXX_CLASSIC_ROUTINE_PROTOTYPE(StartDialog);
REXX_CLASSIC_ROUTINE_PROTOTYPE(WindowRect);
REXX_CLASSIC_ROUTINE_PROTOTYPE(GetScreenSize);
REXX_CLASSIC_ROUTINE_PROTOTYPE(GetSysMetrics);
REXX_CLASSIC_ROUTINE_PROTOTYPE(GetDialogFactor);
REXX_CLASSIC_ROUTINE_PROTOTYPE(SleepMS);
REXX_CLASSIC_ROUTINE_PROTOTYPE(PlaySoundFile);
REXX_CLASSIC_ROUTINE_PROTOTYPE(PlaySoundFileInLoop);
REXX_CLASSIC_ROUTINE_PROTOTYPE(StopSoundFile);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleScrollBar);
REXX_CLASSIC_ROUTINE_PROTOTYPE(BmpButton);
REXX_CLASSIC_ROUTINE_PROTOTYPE(DCDraw);
REXX_CLASSIC_ROUTINE_PROTOTYPE(DrawGetSet);
REXX_CLASSIC_ROUTINE_PROTOTYPE(ScrollText);
REXX_CLASSIC_ROUTINE_PROTOTYPE(ScrollTheWindow);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleDC_Obj);
REXX_CLASSIC_ROUTINE_PROTOTYPE(SetBackground);
REXX_CLASSIC_ROUTINE_PROTOTYPE(LoadRemoveBitmap);
REXX_CLASSIC_ROUTINE_PROTOTYPE(WriteText);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleTreeCtrl);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleListCtrl);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleListCtrlEx);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleControlEx);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleOtherNewCtrls);
REXX_CLASSIC_ROUTINE_PROTOTYPE(WinTimer);
REXX_CLASSIC_ROUTINE_PROTOTYPE(DumpAdmin);
REXX_CLASSIC_ROUTINE_PROTOTYPE(UsrAddControl);
REXX_CLASSIC_ROUTINE_PROTOTYPE(UsrCreateDialog);
REXX_CLASSIC_ROUTINE_PROTOTYPE(UsrDefineDialog);
REXX_CLASSIC_ROUTINE_PROTOTYPE(UsrAddNewCtrl);
REXX_CLASSIC_ROUTINE_PROTOTYPE(UsrAddResource);

// now build the actual entry list
RexxRoutineEntry oodialog_functions[] =
{
    REXX_CLASSIC_ROUTINE(GetDlgMsg,            GetDlgMsg),
    REXX_CLASSIC_ROUTINE(SendWinMsg,           SendWinMsg),
    REXX_CLASSIC_ROUTINE(HandleDlg,            HandleDlg),
    REXX_CLASSIC_ROUTINE(AddUserMessage,       AddUserMessage),
    REXX_CLASSIC_ROUTINE(GetFileNameWindow,    GetFileNameWindow),
    REXX_CLASSIC_ROUTINE(DataTable,            DataTable),
    REXX_CLASSIC_ROUTINE(HandleDialogAdmin,    HandleDialogAdmin),
    REXX_CLASSIC_ROUTINE(SetItemData,          SetItemData),
    REXX_CLASSIC_ROUTINE(SetStemData,          SetStemData),
    REXX_CLASSIC_ROUTINE(GetItemData,          GetItemData),
    REXX_CLASSIC_ROUTINE(GetStemData,          GetStemData),
    REXX_CLASSIC_ROUTINE(Wnd_Desktop,          Wnd_Desktop),
    REXX_CLASSIC_ROUTINE(WndShow_Pos,          WndShow_Pos),
    REXX_CLASSIC_ROUTINE(WinAPI32Func,         WinAPI32Func),
    REXX_CLASSIC_ROUTINE(InfoMessage,          InfoMessage),
    REXX_CLASSIC_ROUTINE(ErrorMessage,         ErrorMessage),
    REXX_CLASSIC_ROUTINE(YesNoMessage,         YesNoMessage),
    REXX_CLASSIC_ROUTINE(FindTheWindow,        FindTheWindow),
    REXX_CLASSIC_ROUTINE(StartDialog,          StartDialog),
    REXX_CLASSIC_ROUTINE(WindowRect,           WindowRect),
    REXX_CLASSIC_ROUTINE(GetScreenSize,        GetScreenSize),
    REXX_CLASSIC_ROUTINE(GetDialogFactor,      GetDialogFactor),
    REXX_CLASSIC_ROUTINE(SleepMS,              SleepMS),
    REXX_CLASSIC_ROUTINE(PlaySoundFile,        PlaySoundFile),
    REXX_CLASSIC_ROUTINE(PlaySoundFileInLoop,  PlaySoundFileInLoop),
    REXX_CLASSIC_ROUTINE(StopSoundFile,        StopSoundFile),
    REXX_CLASSIC_ROUTINE(HandleScrollBar,      HandleScrollBar),
    REXX_CLASSIC_ROUTINE(BmpButton,            BmpButton),
    REXX_CLASSIC_ROUTINE(DCDraw,               DCDraw),
    REXX_CLASSIC_ROUTINE(DrawGetSet,           DrawGetSet),
    REXX_CLASSIC_ROUTINE(ScrollText,           ScrollText),
    REXX_CLASSIC_ROUTINE(ScrollTheWindow,      ScrollTheWindow),
    REXX_CLASSIC_ROUTINE(HandleDC_Obj,         HandleDC_Obj),
    REXX_CLASSIC_ROUTINE(SetBackground,        SetBackground),
    REXX_CLASSIC_ROUTINE(LoadRemoveBitmap,     LoadRemoveBitmap),
    REXX_CLASSIC_ROUTINE(WriteText,            WriteText),
    REXX_CLASSIC_ROUTINE(HandleTreeCtrl,       HandleTreeCtrl),
    REXX_CLASSIC_ROUTINE(HandleListCtrl,       HandleListCtrl),
    REXX_CLASSIC_ROUTINE(HandleListCtrlEx,     HandleListCtrlEx),
    REXX_CLASSIC_ROUTINE(HandleControlEx,      HandleControlEx),
    REXX_CLASSIC_ROUTINE(HandleOtherNewCtrls,  HandleOtherNewCtrls),
    REXX_CLASSIC_ROUTINE(WinTimer,             WinTimer),
    REXX_CLASSIC_ROUTINE(DumpAdmin,            DumpAdmin),
    REXX_CLASSIC_ROUTINE(UsrAddControl,        UsrAddControl),
    REXX_CLASSIC_ROUTINE(UsrCreateDialog,      UsrCreateDialog),
    REXX_CLASSIC_ROUTINE(UsrDefineDialog,      UsrDefineDialog),
    REXX_CLASSIC_ROUTINE(UsrAddNewCtrl,        UsrAddNewCtrl),
    REXX_CLASSIC_ROUTINE(UsrAddResource,       UsrAddResource),

    REXX_LAST_ROUTINE()
};

REXX_METHOD_PROTOTYPE(dlgutil_init_cls);
REXX_METHOD_PROTOTYPE(dlgutil_comctl32Version_cls);
REXX_METHOD_PROTOTYPE(dlgutil_version_cls);
REXX_METHOD_PROTOTYPE(dlgutil_hiWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_loWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_and_cls);
REXX_METHOD_PROTOTYPE(dlgutil_or_cls);
REXX_METHOD_PROTOTYPE(dlgutil_getSystemMetrics_cls);
REXX_METHOD_PROTOTYPE(dlgutil_handleToPointer_cls);
REXX_METHOD_PROTOTYPE(dlgutil_test_cls);

REXX_METHOD_PROTOTYPE(generic_setListTabulators);

REXX_METHOD_PROTOTYPE(wb_getStyleRaw);
REXX_METHOD_PROTOTYPE(wb_getExStyleRaw);

REXX_METHOD_PROTOTYPE(pbdlg_init_cls);
REXX_METHOD_PROTOTYPE(pbdlg_setDefaultFont_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getFontName_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getFontSize_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getTextSizeDlg);

REXX_METHOD_PROTOTYPE(resdlg_setFontAttrib_pvt);

REXX_METHOD_PROTOTYPE(winex_getTextSizeScreen);
REXX_METHOD_PROTOTYPE(winex_getFont);
REXX_METHOD_PROTOTYPE(winex_setFont);
REXX_METHOD_PROTOTYPE(winex_createFontEx);
REXX_METHOD_PROTOTYPE(winex_createFont);

REXX_METHOD_PROTOTYPE(ri_init);
REXX_METHOD_PROTOTYPE(ri_release);
REXX_METHOD_PROTOTYPE(ri_handle);
REXX_METHOD_PROTOTYPE(ri_isNull);
REXX_METHOD_PROTOTYPE(ri_systemErrorCode);
REXX_METHOD_PROTOTYPE(ri_getImage);
REXX_METHOD_PROTOTYPE(ri_getImages);

REXX_METHOD_PROTOTYPE(image_toID_cls);
REXX_METHOD_PROTOTYPE(image_getImage_cls);
REXX_METHOD_PROTOTYPE(image_fromFiles_cls);
REXX_METHOD_PROTOTYPE(image_fromIDs_cls);
REXX_METHOD_PROTOTYPE(image_colorRef_cls);
REXX_METHOD_PROTOTYPE(image_getRValue_cls);
REXX_METHOD_PROTOTYPE(image_getGValue_cls);
REXX_METHOD_PROTOTYPE(image_getBValue_cls);
REXX_METHOD_PROTOTYPE(image_init);
REXX_METHOD_PROTOTYPE(image_release);
REXX_METHOD_PROTOTYPE(image_isNull);
REXX_METHOD_PROTOTYPE(image_handle);
REXX_METHOD_PROTOTYPE(image_systemErrorCode);

REXX_METHOD_PROTOTYPE(il_create_cls);
REXX_METHOD_PROTOTYPE(il_init);
REXX_METHOD_PROTOTYPE(il_release);
REXX_METHOD_PROTOTYPE(il_add);
REXX_METHOD_PROTOTYPE(il_addMasked);
REXX_METHOD_PROTOTYPE(il_addIcon);
REXX_METHOD_PROTOTYPE(il_addImages);
REXX_METHOD_PROTOTYPE(il_addImages);
REXX_METHOD_PROTOTYPE(il_getCount);
REXX_METHOD_PROTOTYPE(il_getImageSize);
REXX_METHOD_PROTOTYPE(il_duplicate);
REXX_METHOD_PROTOTYPE(il_remove);
REXX_METHOD_PROTOTYPE(il_removeAll);
REXX_METHOD_PROTOTYPE(il_isNull);
REXX_METHOD_PROTOTYPE(il_handle);

REXX_METHOD_PROTOTYPE(dlgctrl_getTextSizeDlg);

REXX_METHOD_PROTOTYPE(advCtrl_getStaticControl);
REXX_METHOD_PROTOTYPE(advCtrl_getButtonControl);
REXX_METHOD_PROTOTYPE(advCtrl_getListControl);
REXX_METHOD_PROTOTYPE(advCtrl_getTreeControl);
REXX_METHOD_PROTOTYPE(advCtrl_getTabControl);
REXX_METHOD_PROTOTYPE(advCtrl_putControl_pvt);

REXX_METHOD_PROTOTYPE(lv_setImageList);
REXX_METHOD_PROTOTYPE(lv_getImageList);
REXX_METHOD_PROTOTYPE(lv_getColumnCount);
REXX_METHOD_PROTOTYPE(lv_getColumnOrder);
REXX_METHOD_PROTOTYPE(lv_setColumnOrder);
REXX_METHOD_PROTOTYPE(lv_insertColumnEx);    // TODO review method name
REXX_METHOD_PROTOTYPE(lv_columnWidthEx);     // TODO review method name
REXX_METHOD_PROTOTYPE(lv_stringWidthEx);     // TODO review method name
REXX_METHOD_PROTOTYPE(lv_addRowEx);          // TODO review method name

REXX_METHOD_PROTOTYPE(tv_setImageList);
REXX_METHOD_PROTOTYPE(tv_getImageList);

REXX_METHOD_PROTOTYPE(tab_setImageList);
REXX_METHOD_PROTOTYPE(tab_getImageList);

REXX_METHOD_PROTOTYPE(get_dtp_dateTime);
REXX_METHOD_PROTOTYPE(set_dtp_dateTime);

REXX_METHOD_PROTOTYPE(get_mc_date);
REXX_METHOD_PROTOTYPE(set_mc_date);
REXX_METHOD_PROTOTYPE(get_mc_usesUnicode);
REXX_METHOD_PROTOTYPE(set_mc_usesUnicode);

REXX_METHOD_PROTOTYPE(pbc_stepIt);
REXX_METHOD_PROTOTYPE(pbc_getPos);
REXX_METHOD_PROTOTYPE(pbc_setPos);
REXX_METHOD_PROTOTYPE(pbc_getRange);
REXX_METHOD_PROTOTYPE(pbc_setRange);
REXX_METHOD_PROTOTYPE(pbc_setStep);
REXX_METHOD_PROTOTYPE(pbc_setMarquee);
REXX_METHOD_PROTOTYPE(pbc_setBkColor);
REXX_METHOD_PROTOTYPE(pbc_setBarColor);

REXX_METHOD_PROTOTYPE(stc_getText);
REXX_METHOD_PROTOTYPE(stc_setText);
REXX_METHOD_PROTOTYPE(stc_getIcon);
REXX_METHOD_PROTOTYPE(stc_setIcon);
REXX_METHOD_PROTOTYPE(stc_getImage);
REXX_METHOD_PROTOTYPE(stc_setImage);

REXX_METHOD_PROTOTYPE(gb_setStyle);
REXX_METHOD_PROTOTYPE(bc_getState);
REXX_METHOD_PROTOTYPE(bc_setState);
REXX_METHOD_PROTOTYPE(bc_setStyle);
REXX_METHOD_PROTOTYPE(bc_click);
REXX_METHOD_PROTOTYPE(bc_getIdealSize);
REXX_METHOD_PROTOTYPE(bc_getTextMargin);
REXX_METHOD_PROTOTYPE(bc_setTextMargin);
REXX_METHOD_PROTOTYPE(bc_getImage);
REXX_METHOD_PROTOTYPE(bc_setImage);
REXX_METHOD_PROTOTYPE(bc_setImageList);
REXX_METHOD_PROTOTYPE(bc_getImageList);
REXX_METHOD_PROTOTYPE(rb_checkInGroup_cls);
REXX_METHOD_PROTOTYPE(rb_getCheckState);
REXX_METHOD_PROTOTYPE(rb_checked);
REXX_METHOD_PROTOTYPE(rb_check);
REXX_METHOD_PROTOTYPE(rb_uncheck);
REXX_METHOD_PROTOTYPE(rb_isChecked);
REXX_METHOD_PROTOTYPE(rb_indeterminate);
REXX_METHOD_PROTOTYPE(ckbx_isIndeterminate);
REXX_METHOD_PROTOTYPE(ckbx_setIndeterminate);
REXX_METHOD_PROTOTYPE(bc_test);

REXX_METHOD_PROTOTYPE(rect_init);
REXX_METHOD_PROTOTYPE(rect_left);
REXX_METHOD_PROTOTYPE(rect_top);
REXX_METHOD_PROTOTYPE(rect_right);
REXX_METHOD_PROTOTYPE(rect_bottom);
REXX_METHOD_PROTOTYPE(rect_setLeft);
REXX_METHOD_PROTOTYPE(rect_setTop);
REXX_METHOD_PROTOTYPE(rect_setRight);
REXX_METHOD_PROTOTYPE(rect_setBottom);

REXX_METHOD_PROTOTYPE(point_init);
REXX_METHOD_PROTOTYPE(point_x);
REXX_METHOD_PROTOTYPE(point_setX);
REXX_METHOD_PROTOTYPE(point_y);
REXX_METHOD_PROTOTYPE(point_setY);

REXX_METHOD_PROTOTYPE(size_init);
REXX_METHOD_PROTOTYPE(size_cx);
REXX_METHOD_PROTOTYPE(size_setCX);
REXX_METHOD_PROTOTYPE(size_cy);
REXX_METHOD_PROTOTYPE(size_setCY);

// Menu classes methods
REXX_METHOD_PROTOTYPE(menu_getHMenu);
REXX_METHOD_PROTOTYPE(menu_wID);
REXX_METHOD_PROTOTYPE(menu_isValidItemID);
REXX_METHOD_PROTOTYPE(menu_isValidMenu);
REXX_METHOD_PROTOTYPE(menu_isValidMenuHandle);
REXX_METHOD_PROTOTYPE(menu_isSeparator);
REXX_METHOD_PROTOTYPE(menu_isCommandItem);
REXX_METHOD_PROTOTYPE(menu_isPopup);
REXX_METHOD_PROTOTYPE(menu_getMenuHandle);
REXX_METHOD_PROTOTYPE(menu_releaseMenuHandle);
REXX_METHOD_PROTOTYPE(menu_destroy);
REXX_METHOD_PROTOTYPE(menu_insertSeparator);
REXX_METHOD_PROTOTYPE(menu_removeSeparator);
REXX_METHOD_PROTOTYPE(menu_insertItem);
REXX_METHOD_PROTOTYPE(menu_removeItem);
REXX_METHOD_PROTOTYPE(menu_insertPopup);
REXX_METHOD_PROTOTYPE(menu_removePopup);
REXX_METHOD_PROTOTYPE(menu_deletePopup);
REXX_METHOD_PROTOTYPE(menu_getPopup);
REXX_METHOD_PROTOTYPE(menu_isEnabled);
REXX_METHOD_PROTOTYPE(menu_isDisabled);
REXX_METHOD_PROTOTYPE(menu_isChecked);
REXX_METHOD_PROTOTYPE(menu_check);
REXX_METHOD_PROTOTYPE(menu_unCheck);
REXX_METHOD_PROTOTYPE(menu_checkRadio);
REXX_METHOD_PROTOTYPE(menu_hilite);
REXX_METHOD_PROTOTYPE(menu_unHilite);
REXX_METHOD_PROTOTYPE(menu_getCount);
REXX_METHOD_PROTOTYPE(menu_enable);
REXX_METHOD_PROTOTYPE(menu_disable);
REXX_METHOD_PROTOTYPE(menu_getItemState);
REXX_METHOD_PROTOTYPE(menu_getItemType);
REXX_METHOD_PROTOTYPE(menu_setID);
REXX_METHOD_PROTOTYPE(menu_getID);
REXX_METHOD_PROTOTYPE(menu_getHelpID);
REXX_METHOD_PROTOTYPE(menu_setHelpID);
REXX_METHOD_PROTOTYPE(menu_getMaxHeight);
REXX_METHOD_PROTOTYPE(menu_setMaxHeight);
REXX_METHOD_PROTOTYPE(menu_setText);
REXX_METHOD_PROTOTYPE(menu_getText);
REXX_METHOD_PROTOTYPE(menu_setAutoConnection);
REXX_METHOD_PROTOTYPE(menu_getAutoConnectStatus);
REXX_METHOD_PROTOTYPE(menu_connectWM);
REXX_METHOD_PROTOTYPE(menu_connectItem);
REXX_METHOD_PROTOTYPE(menu_connectAllItems);
REXX_METHOD_PROTOTYPE(menu_connectSomeItems);
REXX_METHOD_PROTOTYPE(menu_itemTextToMethodName);
REXX_METHOD_PROTOTYPE(menu_test);

REXX_METHOD_PROTOTYPE(menuBar_isAttached);
REXX_METHOD_PROTOTYPE(menuBar_redraw);
REXX_METHOD_PROTOTYPE(menuBar_attachTo);
REXX_METHOD_PROTOTYPE(menuBar_detach);

REXX_METHOD_PROTOTYPE(binMenu_init);

REXX_METHOD_PROTOTYPE(sysMenu_init);
REXX_METHOD_PROTOTYPE(sysMenu_revert);
REXX_METHOD_PROTOTYPE(sysMenu_connectItem);
REXX_METHOD_PROTOTYPE(sysMenu_connectAllItems);
REXX_METHOD_PROTOTYPE(sysMenu_connectSomeItems);

REXX_METHOD_PROTOTYPE(popMenu_init);
REXX_METHOD_PROTOTYPE(popMenu_connectContextMenu);
REXX_METHOD_PROTOTYPE(popMenu_isAssigned);
REXX_METHOD_PROTOTYPE(popMenu_assignTo);
REXX_METHOD_PROTOTYPE(popMenu_track);
REXX_METHOD_PROTOTYPE(popMenu_show);

REXX_METHOD_PROTOTYPE(scriptMenu_init);

REXX_METHOD_PROTOTYPE(userMenu_init);
REXX_METHOD_PROTOTYPE(userMenu_complete);

REXX_METHOD_PROTOTYPE(menuTemplate_isComplete);
REXX_METHOD_PROTOTYPE(menuTemplate_addSeparator);
REXX_METHOD_PROTOTYPE(menuTemplate_addItem);
REXX_METHOD_PROTOTYPE(menuTemplate_addPopup);


RexxMethodEntry oodialog_methods[] = {
    REXX_METHOD(dlgutil_init_cls,             dlgutil_init_cls),
    REXX_METHOD(dlgutil_comctl32Version_cls,  dlgutil_comctl32Version_cls),
    REXX_METHOD(dlgutil_version_cls,          dlgutil_version_cls),
    REXX_METHOD(dlgutil_hiWord_cls,           dlgutil_hiWord_cls),
    REXX_METHOD(dlgutil_and_cls,              dlgutil_and_cls),
    REXX_METHOD(dlgutil_or_cls,               dlgutil_or_cls),
    REXX_METHOD(dlgutil_loWord_cls,           dlgutil_loWord_cls),
    REXX_METHOD(dlgutil_handleToPointer_cls,  dlgutil_handleToPointer_cls),
    REXX_METHOD(dlgutil_getSystemMetrics_cls, dlgutil_getSystemMetrics_cls),
    REXX_METHOD(dlgutil_test_cls,             dlgutil_test_cls),

    REXX_METHOD(generic_setListTabulators,    generic_setListTabulators),

    REXX_METHOD(wb_getStyleRaw,               wb_getStyleRaw),
    REXX_METHOD(wb_getExStyleRaw,             wb_getExStyleRaw),

    REXX_METHOD(pbdlg_init_cls,               pbdlg_init_cls),
    REXX_METHOD(pbdlg_setDefaultFont_cls,     pbdlg_setDefaultFont_cls),
    REXX_METHOD(pbdlg_getFontName_cls,        pbdlg_getFontName_cls),
    REXX_METHOD(pbdlg_getFontSize_cls,        pbdlg_getFontSize_cls),
    REXX_METHOD(pbdlg_getTextSizeDlg,         pbdlg_getTextSizeDlg),

    REXX_METHOD(resdlg_setFontAttrib_pvt,     resdlg_setFontAttrib_pvt),

    REXX_METHOD(winex_getTextSizeScreen,      winex_getTextSizeScreen),
    REXX_METHOD(winex_getFont,                winex_getFont),
    REXX_METHOD(winex_setFont,                winex_setFont),
    REXX_METHOD(winex_createFontEx,           winex_createFontEx),
    REXX_METHOD(winex_createFont,             winex_createFont),

    REXX_METHOD(ri_init,                     ri_init),
    REXX_METHOD(ri_release,                  ri_release),
    REXX_METHOD(ri_handle,                   ri_handle),
    REXX_METHOD(ri_isNull,                   ri_isNull),
    REXX_METHOD(ri_systemErrorCode,          ri_systemErrorCode),
    REXX_METHOD(ri_getImage,                 ri_getImage),
    REXX_METHOD(ri_getImages,                ri_getImages),

    REXX_METHOD(image_toID_cls,              image_toID_cls),
    REXX_METHOD(image_getImage_cls,          image_getImage_cls),
    REXX_METHOD(image_fromFiles_cls,         image_fromFiles_cls),
    REXX_METHOD(image_fromIDs_cls,           image_fromIDs_cls),
    REXX_METHOD(image_colorRef_cls,          image_colorRef_cls),
    REXX_METHOD(image_getRValue_cls,         image_getRValue_cls),
    REXX_METHOD(image_getGValue_cls,         image_getGValue_cls),
    REXX_METHOD(image_getBValue_cls,         image_getBValue_cls),
    REXX_METHOD(image_init,                  image_init),
    REXX_METHOD(image_release,               image_release),
    REXX_METHOD(image_isNull,                image_isNull),
    REXX_METHOD(image_systemErrorCode,       image_systemErrorCode),
    REXX_METHOD(image_handle,                image_handle),

    REXX_METHOD(il_create_cls,               il_create_cls),
    REXX_METHOD(il_init,                     il_init),
    REXX_METHOD(il_release,                  il_release),
    REXX_METHOD(il_add,                      il_add),
    REXX_METHOD(il_addMasked,                il_addMasked),
    REXX_METHOD(il_addIcon,                  il_addIcon),
    REXX_METHOD(il_addImages,                il_addImages),
    REXX_METHOD(il_getCount,                 il_getCount),
    REXX_METHOD(il_getImageSize,             il_getImageSize),
    REXX_METHOD(il_duplicate,                il_duplicate),
    REXX_METHOD(il_removeAll,                il_removeAll),
    REXX_METHOD(il_remove,                   il_remove),
    REXX_METHOD(il_isNull,                   il_isNull),
    REXX_METHOD(il_handle,                   il_handle),

    REXX_METHOD(dlgctrl_getTextSizeDlg,      dlgctrl_getTextSizeDlg),

    REXX_METHOD(advCtrl_getStaticControl, advCtrl_getStaticControl),
    REXX_METHOD(advCtrl_getButtonControl, advCtrl_getButtonControl),
    REXX_METHOD(advCtrl_getListControl,   advCtrl_getListControl),
    REXX_METHOD(advCtrl_getTreeControl,   advCtrl_getTreeControl),
    REXX_METHOD(advCtrl_getTabControl,    advCtrl_getTabControl),
    REXX_METHOD(advCtrl_putControl_pvt,   advCtrl_putControl_pvt),

    REXX_METHOD(lv_setImageList,          lv_setImageList),
    REXX_METHOD(lv_getImageList,          lv_getImageList),
    REXX_METHOD(lv_getColumnCount,        lv_getColumnCount),
    REXX_METHOD(lv_getColumnOrder,        lv_getColumnOrder),
    REXX_METHOD(lv_setColumnOrder,        lv_setColumnOrder),
    REXX_METHOD(lv_insertColumnEx,        lv_insertColumnEx),     // TODO review method name
    REXX_METHOD(lv_columnWidthEx,         lv_columnWidthEx),      // TODO review method name
    REXX_METHOD(lv_stringWidthEx,         lv_stringWidthEx),      // TODO review method name
    REXX_METHOD(lv_addRowEx,         	  lv_addRowEx),           // TODO review method name

    REXX_METHOD(tv_setImageList,          tv_setImageList),
    REXX_METHOD(tv_getImageList,          tv_getImageList),

    REXX_METHOD(tab_setImageList,         tab_setImageList),
    REXX_METHOD(tab_getImageList,         tab_getImageList),

    REXX_METHOD(get_dtp_dateTime,         get_dtp_dateTime),
    REXX_METHOD(set_dtp_dateTime,         set_dtp_dateTime),

    REXX_METHOD(get_mc_date,              get_mc_date),
    REXX_METHOD(set_mc_date,              set_mc_date),
    REXX_METHOD(get_mc_usesUnicode,       get_mc_usesUnicode),
    REXX_METHOD(set_mc_usesUnicode,       set_mc_usesUnicode),

    REXX_METHOD(pbc_stepIt,              pbc_stepIt),
    REXX_METHOD(pbc_getPos,              pbc_getPos),
    REXX_METHOD(pbc_setPos,              pbc_setPos),
    REXX_METHOD(pbc_getRange,            pbc_getRange),
    REXX_METHOD(pbc_setRange,            pbc_setRange),
    REXX_METHOD(pbc_setStep,             pbc_setStep),
    REXX_METHOD(pbc_setMarquee,          pbc_setMarquee),
    REXX_METHOD(pbc_setBkColor,          pbc_setBkColor),
    REXX_METHOD(pbc_setBarColor,         pbc_setBarColor),

    REXX_METHOD(stc_getText,             stc_getText),
    REXX_METHOD(stc_setText,             stc_setText),
    REXX_METHOD(stc_getIcon,             stc_getIcon),
    REXX_METHOD(stc_setIcon,             stc_setIcon),
    REXX_METHOD(stc_getImage,            stc_getImage),
    REXX_METHOD(stc_setImage,            stc_setImage),

    REXX_METHOD(bc_getState,             bc_getState),
    REXX_METHOD(bc_setState,             bc_setState),
    REXX_METHOD(bc_setStyle,             bc_setStyle),
    REXX_METHOD(bc_click,                bc_click),
    REXX_METHOD(bc_getIdealSize,         bc_getIdealSize),
    REXX_METHOD(bc_getTextMargin,        bc_getTextMargin),
    REXX_METHOD(bc_setTextMargin,        bc_setTextMargin),
    REXX_METHOD(bc_getImage,             bc_getImage),
    REXX_METHOD(bc_setImage,             bc_setImage),
    REXX_METHOD(bc_setImageList,         bc_setImageList),
    REXX_METHOD(bc_getImageList,         bc_getImageList),
    REXX_METHOD(rb_checkInGroup_cls,     rb_checkInGroup_cls),
    REXX_METHOD(rb_checked,              rb_checked),
    REXX_METHOD(rb_check,                rb_check),
    REXX_METHOD(rb_uncheck,              rb_uncheck),
    REXX_METHOD(rb_getCheckState,        rb_getCheckState),
    REXX_METHOD(rb_isChecked,            rb_isChecked),
    REXX_METHOD(rb_indeterminate,        rb_indeterminate),
    REXX_METHOD(ckbx_isIndeterminate,    ckbx_isIndeterminate),
    REXX_METHOD(ckbx_setIndeterminate,   ckbx_setIndeterminate),
    REXX_METHOD(gb_setStyle,             gb_setStyle),
    REXX_METHOD(bc_test,                 bc_test),

    REXX_METHOD(rect_init,               rect_init),
    REXX_METHOD(rect_left,               rect_left),
    REXX_METHOD(rect_top,                rect_top),
    REXX_METHOD(rect_right,              rect_right),
    REXX_METHOD(rect_bottom,             rect_bottom),
    REXX_METHOD(rect_setLeft,            rect_setLeft),
    REXX_METHOD(rect_setTop,             rect_setTop),
    REXX_METHOD(rect_setRight,           rect_setRight),
    REXX_METHOD(rect_setBottom,          rect_setBottom),
    REXX_METHOD(point_init,              point_init),
    REXX_METHOD(point_x,                 point_x),
    REXX_METHOD(point_setX,              point_setX),
    REXX_METHOD(point_y,                 point_y),
    REXX_METHOD(point_setY,              point_setY),
    REXX_METHOD(size_init,               size_init),
    REXX_METHOD(size_cx,                 size_cx),
    REXX_METHOD(size_setCX,              size_setCX),
    REXX_METHOD(size_cy,                 size_cy),
    REXX_METHOD(size_setCY,              size_setCY),

    // Menu classes methods
    REXX_METHOD(menu_getHMenu,                  menu_getHMenu),
    REXX_METHOD(menu_wID,                       menu_wID),
    REXX_METHOD(menu_isValidItemID,             menu_isValidItemID),
    REXX_METHOD(menu_isValidMenu,               menu_isValidMenu),
    REXX_METHOD(menu_isValidMenuHandle,         menu_isValidMenuHandle),
    REXX_METHOD(menu_isSeparator,               menu_isSeparator),
    REXX_METHOD(menu_isCommandItem,             menu_isCommandItem),
    REXX_METHOD(menu_isPopup,                   menu_isPopup),
    REXX_METHOD(menu_isEnabled,                 menu_isEnabled),
    REXX_METHOD(menu_isDisabled,                menu_isDisabled),
    REXX_METHOD(menu_isChecked,                 menu_isChecked),
    REXX_METHOD(menu_getMenuHandle,             menu_getMenuHandle),
    REXX_METHOD(menu_releaseMenuHandle,         menu_releaseMenuHandle),
    REXX_METHOD(menu_destroy,                   menu_destroy),
    REXX_METHOD(menu_enable,                    menu_enable),
    REXX_METHOD(menu_disable,                   menu_disable),
    REXX_METHOD(menu_check,                     menu_check),
    REXX_METHOD(menu_unCheck,                   menu_unCheck),
    REXX_METHOD(menu_checkRadio,                menu_checkRadio),
    REXX_METHOD(menu_hilite,                    menu_hilite),
    REXX_METHOD(menu_unHilite,                  menu_unHilite),
    REXX_METHOD(menu_insertSeparator,           menu_insertSeparator),
    REXX_METHOD(menu_removeSeparator,           menu_removeSeparator),
    REXX_METHOD(menu_insertItem,                menu_insertItem),
    REXX_METHOD(menu_removeItem,                menu_removeItem),
    REXX_METHOD(menu_insertPopup,               menu_insertPopup),
    REXX_METHOD(menu_getPopup,                  menu_getPopup),
    REXX_METHOD(menu_removePopup,               menu_removePopup),
    REXX_METHOD(menu_deletePopup,               menu_deletePopup),
    REXX_METHOD(menu_getCount,                  menu_getCount),
    REXX_METHOD(menu_getItemState,              menu_getItemState),
    REXX_METHOD(menu_getItemType,               menu_getItemType),
    REXX_METHOD(menu_getID,                     menu_getID),
    REXX_METHOD(menu_setID,                     menu_setID),
    REXX_METHOD(menu_getHelpID,                 menu_getHelpID),
    REXX_METHOD(menu_setHelpID,                 menu_setHelpID),
    REXX_METHOD(menu_getMaxHeight,              menu_getMaxHeight),
    REXX_METHOD(menu_setMaxHeight,              menu_setMaxHeight),
    REXX_METHOD(menu_getText,                   menu_getText),
    REXX_METHOD(menu_setText,                   menu_setText),
    REXX_METHOD(menu_getAutoConnectStatus,      menu_getAutoConnectStatus),
    REXX_METHOD(menu_setAutoConnection,         menu_setAutoConnection),
    REXX_METHOD(menu_connectWM,                 menu_connectWM),
    REXX_METHOD(menu_connectItem,               menu_connectItem),
    REXX_METHOD(menu_connectSomeItems,          menu_connectSomeItems),
    REXX_METHOD(menu_connectAllItems,           menu_connectAllItems),
    REXX_METHOD(menu_itemTextToMethodName,      menu_itemTextToMethodName),
    REXX_METHOD(menu_test,                      menu_test),

    REXX_METHOD(menuBar_isAttached,      menuBar_isAttached),
    REXX_METHOD(menuBar_redraw,          menuBar_redraw),
    REXX_METHOD(menuBar_attachTo,        menuBar_attachTo),
    REXX_METHOD(menuBar_detach,          menuBar_detach),

    REXX_METHOD(binMenu_init,            binMenu_init),

    REXX_METHOD(sysMenu_init,              sysMenu_init),
    REXX_METHOD(sysMenu_revert,            sysMenu_revert),
    REXX_METHOD(sysMenu_connectItem,       sysMenu_connectItem),
    REXX_METHOD(sysMenu_connectSomeItems,  sysMenu_connectSomeItems),
    REXX_METHOD(sysMenu_connectAllItems,   sysMenu_connectAllItems),

    REXX_METHOD(popMenu_init,               popMenu_init),
    REXX_METHOD(popMenu_isAssigned,         popMenu_isAssigned),
    REXX_METHOD(popMenu_connectContextMenu, popMenu_connectContextMenu),
    REXX_METHOD(popMenu_assignTo,           popMenu_assignTo),
    REXX_METHOD(popMenu_track,              popMenu_track),
    REXX_METHOD(popMenu_show,               popMenu_show),

    REXX_METHOD(scriptMenu_init,         scriptMenu_init),

    REXX_METHOD(userMenu_init,           userMenu_init),
    REXX_METHOD(userMenu_complete,       userMenu_complete),

    REXX_METHOD(menuTemplate_isComplete,   menuTemplate_isComplete),
    REXX_METHOD(menuTemplate_addSeparator, menuTemplate_addSeparator),
    REXX_METHOD(menuTemplate_addItem,      menuTemplate_addItem),
    REXX_METHOD(menuTemplate_addPopup,     menuTemplate_addPopup),

    REXX_LAST_METHOD()
};

RexxPackageEntry oodialog_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "OODIALOG",                          // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    oodialog_functions,                  // the exported functions
    oodialog_methods                     // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(oodialog);

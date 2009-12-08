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

#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

HINSTANCE            MyInstance = NULL;
DIALOGADMIN         *DialogTab[MAXDIALOGS] = {NULL};
DIALOGADMIN         *topDlg = {NULL};
INT                  StoredDialogs = 0;
CRITICAL_SECTION     crit_sec = {0};
DWORD                ComCtl32Version = 0;

// Initialized in dlgutil_init_cls
RexxObjectPtr       TheTrueObj = NULLOBJECT;
RexxObjectPtr       TheFalseObj = NULLOBJECT;
RexxObjectPtr       TheNilObj = NULLOBJECT;
RexxPointerObject   TheNullPtrObj = NULLOBJECT;
RexxDirectoryObject TheDotLocalObj = NULLOBJECT;
RexxObjectPtr       TheZeroObj = NULLOBJECT;
RexxObjectPtr       TheOneObj = NULLOBJECT;
RexxObjectPtr       TheTwoObj = NULLOBJECT;
RexxObjectPtr       TheNegativeOneObj = NULLOBJECT;

// Initialized in the PlainBaseDialog class init method (pbdlg_init_cls).
RexxClassObject     ThePlainBaseDialogClass = NULLOBJECT;

// Initialized in the DynamicDialog class init method (dyndlg_init_cls).
RexxClassObject     TheDynamicDialogClass = NULLOBJECT;

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


REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleTreeCtrl);
REXX_CLASSIC_ROUTINE_PROTOTYPE(HandleListCtrl);

REXX_TYPED_ROUTINE_PROTOTYPE(getDlgMsg_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(messageDialog_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(fileNameDlg_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(findWindow_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(msSleep_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(playSound_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(winTimer_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(routineTest_rtn);

// now build the actual entry list
RexxRoutineEntry oodialog_functions[] =
{
    REXX_CLASSIC_ROUTINE(HandleTreeCtrl,       HandleTreeCtrl),
    REXX_CLASSIC_ROUTINE(HandleListCtrl,       HandleListCtrl),

    REXX_TYPED_ROUTINE(getDlgMsg_rtn,          getDlgMsg_rtn),
    REXX_TYPED_ROUTINE(messageDialog_rtn,      messageDialog_rtn),
    REXX_TYPED_ROUTINE(findWindow_rtn,         findWindow_rtn),
    REXX_TYPED_ROUTINE(fileNameDlg_rtn,        fileNameDlg_rtn),
    REXX_TYPED_ROUTINE(msSleep_rtn,            msSleep_rtn),
    REXX_TYPED_ROUTINE(playSound_rtn,          playSound_rtn),
    REXX_TYPED_ROUTINE(winTimer_rtn,           winTimer_rtn),
    REXX_TYPED_ROUTINE(routineTest_rtn,        routineTest_rtn),

    REXX_LAST_ROUTINE()
};

REXX_METHOD_PROTOTYPE(dlgutil_init_cls);
REXX_METHOD_PROTOTYPE(dlgutil_comctl32Version_cls);
REXX_METHOD_PROTOTYPE(dlgutil_version_cls);
REXX_METHOD_PROTOTYPE(dlgutil_hiWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_loWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_makeLPARAM_cls);
REXX_METHOD_PROTOTYPE(dlgutil_makeWPARAM_cls);
REXX_METHOD_PROTOTYPE(dlgutil_and_cls);
REXX_METHOD_PROTOTYPE(dlgutil_or_cls);
REXX_METHOD_PROTOTYPE(dlgutil_getSystemMetrics_cls);
REXX_METHOD_PROTOTYPE(dlgutil_screenSize_cls);
REXX_METHOD_PROTOTYPE(dlgutil_screenArea_cls);
REXX_METHOD_PROTOTYPE(dlgutil_handleToPointer_cls);
REXX_METHOD_PROTOTYPE(dlgutil_test_cls);

REXX_METHOD_PROTOTYPE(rsrcUtils_resolveIconID_pvt);
REXX_METHOD_PROTOTYPE(rsrcUtils_resolveResourceID);
REXX_METHOD_PROTOTYPE(rsrcUtils_idError);
REXX_METHOD_PROTOTYPE(rsrcUtils_checkID);

REXX_METHOD_PROTOTYPE(wb_getHwnd);
REXX_METHOD_PROTOTYPE(wb_getFactorX);
REXX_METHOD_PROTOTYPE(wb_setFactorX);
REXX_METHOD_PROTOTYPE(wb_getFactorY);
REXX_METHOD_PROTOTYPE(wb_setFactorY);
REXX_METHOD_PROTOTYPE(wb_getSizeX);
REXX_METHOD_PROTOTYPE(wb_setSizeX);
REXX_METHOD_PROTOTYPE(wb_getSizeY);
REXX_METHOD_PROTOTYPE(wb_setSizeY);
REXX_METHOD_PROTOTYPE(wb_getPixelX);
REXX_METHOD_PROTOTYPE(wb_getPixelY);
REXX_METHOD_PROTOTYPE(wb_init_windowBase);
REXX_METHOD_PROTOTYPE(wb_sendMessage);
REXX_METHOD_PROTOTYPE(wb_sendWinIntMsg);
REXX_METHOD_PROTOTYPE(wb_sendWinHandleMsg);
REXX_METHOD_PROTOTYPE(wb_sendWinHandle2Msg);
REXX_METHOD_PROTOTYPE(wb_enable);
REXX_METHOD_PROTOTYPE(wb_isEnabled);
REXX_METHOD_PROTOTYPE(wb_isVisible);
REXX_METHOD_PROTOTYPE(wb_show);
REXX_METHOD_PROTOTYPE(wb_showFast);
REXX_METHOD_PROTOTYPE(wb_display);
REXX_METHOD_PROTOTYPE(wb_redrawClient);
REXX_METHOD_PROTOTYPE(wb_redraw);
REXX_METHOD_PROTOTYPE(wb_getText);
REXX_METHOD_PROTOTYPE(wb_setText);
REXX_METHOD_PROTOTYPE(wb_setRect);
REXX_METHOD_PROTOTYPE(wb_resizeMove);
REXX_METHOD_PROTOTYPE(wb_getSizePos);
REXX_METHOD_PROTOTYPE(wb_windowRect);
REXX_METHOD_PROTOTYPE(wb_clientRect);
REXX_METHOD_PROTOTYPE(wb_clear);
REXX_METHOD_PROTOTYPE(wb_foreGroundWindow);
REXX_METHOD_PROTOTYPE(wb_screenClient);
REXX_METHOD_PROTOTYPE(wb_getWindowLong_pvt);

REXX_METHOD_PROTOTYPE(en_init_eventNotification);
REXX_METHOD_PROTOTYPE(en_connectKeyPress);
REXX_METHOD_PROTOTYPE(en_connectFKeyPress);
REXX_METHOD_PROTOTYPE(en_disconnectKeyPress);
REXX_METHOD_PROTOTYPE(en_hasKeyPressConnection);
REXX_METHOD_PROTOTYPE(en_connectCommandEvents);
REXX_METHOD_PROTOTYPE(en_addUserMessage);

REXX_METHOD_PROTOTYPE(window_init);
REXX_METHOD_PROTOTYPE(window_unInit);

REXX_METHOD_PROTOTYPE(pbdlg_init_cls);
REXX_METHOD_PROTOTYPE(pbdlg_setDefaultFont_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getFontName_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getFontSize_cls);
REXX_METHOD_PROTOTYPE(pbdlg_init);
REXX_METHOD_PROTOTYPE(pbdlg_setDlgFont);
REXX_METHOD_PROTOTYPE(pbdlg_getFontName);
REXX_METHOD_PROTOTYPE(pbdlg_getFontSize);
REXX_METHOD_PROTOTYPE(pbdlg_getAutoDetect);
REXX_METHOD_PROTOTYPE(pbdlg_setAutoDetect);
REXX_METHOD_PROTOTYPE(pbdlg_sendMessageToControl);
REXX_METHOD_PROTOTYPE(pbdlg_sendMessageToWindow);
REXX_METHOD_PROTOTYPE(pbdlg_get);
REXX_METHOD_PROTOTYPE(pbdlg_getDlgHandle);
REXX_METHOD_PROTOTYPE(pbdlg_isDialogActive);
REXX_METHOD_PROTOTYPE(pbdlg_stopIt);
REXX_METHOD_PROTOTYPE(pbdlg_show);
REXX_METHOD_PROTOTYPE(pbdlg_toTheTop);
REXX_METHOD_PROTOTYPE(pbdlg_getFocus);
REXX_METHOD_PROTOTYPE(pbdlg_setFocus);
REXX_METHOD_PROTOTYPE(pbdlg_tabTo);
REXX_METHOD_PROTOTYPE(pbdlg_pixel2dlgUnit);
REXX_METHOD_PROTOTYPE(pbdlg_dlgUnit2pixel);
REXX_METHOD_PROTOTYPE(pbdlg_backgroundBitmap);
REXX_METHOD_PROTOTYPE(pbdlg_tiledBackgroundBitmap);
REXX_METHOD_PROTOTYPE(pbdlg_backgroundColor);
REXX_METHOD_PROTOTYPE(pbdlg_focusControl);
REXX_METHOD_PROTOTYPE(pbdlg_showControl);
REXX_METHOD_PROTOTYPE(pbdlg_showWindow);
REXX_METHOD_PROTOTYPE(pbdlg_getControlHandle);
REXX_METHOD_PROTOTYPE(pbdlg_getWindowText);
REXX_METHOD_PROTOTYPE(pbdlg_setWindowText);
REXX_METHOD_PROTOTYPE(pbdlg_getControlText);
REXX_METHOD_PROTOTYPE(pbdlg_setControlText);
REXX_METHOD_PROTOTYPE(pbdlg_enableDisableControl);
REXX_METHOD_PROTOTYPE(pbdlg_getControlID);
REXX_METHOD_PROTOTYPE(pbdlg_center);
REXX_METHOD_PROTOTYPE(pbdlg_doMinMax);
REXX_METHOD_PROTOTYPE(pbdlg_setTabGroup);
REXX_METHOD_PROTOTYPE(pbdlg_connect_ControName);
REXX_METHOD_PROTOTYPE(pbdlg_setDlgDataFromStem_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_putDlgDataInStem_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_getControlData);
REXX_METHOD_PROTOTYPE(pbdlg_setControlData);
REXX_METHOD_PROTOTYPE(pbdlg_getTextSizeDlg);
REXX_METHOD_PROTOTYPE(pbdlg_newControl);
REXX_METHOD_PROTOTYPE(pbdlg_putControl_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_unInit);

REXX_METHOD_PROTOTYPE(generic_setListTabulators);

REXX_METHOD_PROTOTYPE(dlgext_setWindowRect);
REXX_METHOD_PROTOTYPE(dlgext_getControlRect);
REXX_METHOD_PROTOTYPE(dlgext_clearWindowRect);
REXX_METHOD_PROTOTYPE(dlgext_clearControlRect);
REXX_METHOD_PROTOTYPE(dlgext_clearRect);
REXX_METHOD_PROTOTYPE(dlgext_redrawRect);
REXX_METHOD_PROTOTYPE(dlgext_redrawWindowRect);
REXX_METHOD_PROTOTYPE(dlgext_redrawControl);
REXX_METHOD_PROTOTYPE(dlgext_resizeMoveControl);
REXX_METHOD_PROTOTYPE(dlgext_setForgroundWindow);
REXX_METHOD_PROTOTYPE(dlgext_setControlColor);
REXX_METHOD_PROTOTYPE(dlgext_installBitmapButton);
REXX_METHOD_PROTOTYPE(dlgext_changeBitmapButton);
REXX_METHOD_PROTOTYPE(dlgext_drawBitmap);
REXX_METHOD_PROTOTYPE(dlgext_drawButton);
REXX_METHOD_PROTOTYPE(dlgext_getBitmapPosition);
REXX_METHOD_PROTOTYPE(dlgext_setBitmapPosition);
REXX_METHOD_PROTOTYPE(dlgext_getBitmapSize);
REXX_METHOD_PROTOTYPE(dlgext_getWindowDC);
REXX_METHOD_PROTOTYPE(dlgext_freeWindowDC);
REXX_METHOD_PROTOTYPE(dlgext_scrollText);
REXX_METHOD_PROTOTYPE(dlgext_writeToWindow);
REXX_METHOD_PROTOTYPE(dlgext_createBrush);
REXX_METHOD_PROTOTYPE(dlgext_mouseCapture);
REXX_METHOD_PROTOTYPE(dlgext_captureMouse);
REXX_METHOD_PROTOTYPE(dlgext_isMouseButtonDown);
REXX_METHOD_PROTOTYPE(dlgext_dumpAdmin_pvt);

REXX_METHOD_PROTOTYPE(baseDlg_init);
REXX_METHOD_PROTOTYPE(baseDlg_test);

REXX_METHOD_PROTOTYPE(userdlg_init);

REXX_METHOD_PROTOTYPE(catdlg_createCategoryDialog);
REXX_METHOD_PROTOTYPE(catdlg_getControlDataPage);
REXX_METHOD_PROTOTYPE(catdlg_setControlDataPage);
REXX_METHOD_PROTOTYPE(catdlg_sendMessageToCategoryControl);
REXX_METHOD_PROTOTYPE(catdlg_getCategoryComboEntry);

REXX_METHOD_PROTOTYPE(dyndlg_init_cls);
REXX_METHOD_PROTOTYPE(dyndlg_getBasePtr);
REXX_METHOD_PROTOTYPE(dyndlg_setBasePtr);
REXX_METHOD_PROTOTYPE(dyndlg_getActivePtr);
REXX_METHOD_PROTOTYPE(dyndlg_setActivePtr);
REXX_METHOD_PROTOTYPE(dyndlg_getDialogItemCount);
REXX_METHOD_PROTOTYPE(dyndlg_setDialogItemCount);
REXX_METHOD_PROTOTYPE(dyndlg_dynamicInit);
REXX_METHOD_PROTOTYPE(dyndlg_create);
REXX_METHOD_PROTOTYPE(dyndlg_startParentDialog);
REXX_METHOD_PROTOTYPE(dyndlg_startChildDialog);
REXX_METHOD_PROTOTYPE(dyndlg_createStatic);
REXX_METHOD_PROTOTYPE(dyndlg_createStaticText);
REXX_METHOD_PROTOTYPE(dyndlg_createStaticImage);
REXX_METHOD_PROTOTYPE(dyndlg_createStaticFrame);
REXX_METHOD_PROTOTYPE(dyndlg_createPushButton);
REXX_METHOD_PROTOTYPE(dyndlg_createRadioButton);
REXX_METHOD_PROTOTYPE(dyndlg_createGroupBox);
REXX_METHOD_PROTOTYPE(dyndlg_createEdit);
REXX_METHOD_PROTOTYPE(dyndlg_createScrollBar);
REXX_METHOD_PROTOTYPE(dyndlg_createListBox);
REXX_METHOD_PROTOTYPE(dyndlg_createComboBox);
REXX_METHOD_PROTOTYPE(dyndlg_createProgressBar);
REXX_METHOD_PROTOTYPE(dyndlg_createNamedControl);
REXX_METHOD_PROTOTYPE(dyndlg_addButton);
REXX_METHOD_PROTOTYPE(dyndlg_addRadioButton);
REXX_METHOD_PROTOTYPE(dyndlg_addGroupBox);
REXX_METHOD_PROTOTYPE(dyndlg_addEntryLine);
REXX_METHOD_PROTOTYPE(dyndlg_addMethod);
REXX_METHOD_PROTOTYPE(dyndlg_addIconResource);
REXX_METHOD_PROTOTYPE(dyndlg_stop);
REXX_METHOD_PROTOTYPE(dyndlg_stopDynamic_pvt);

REXX_METHOD_PROTOTYPE(resdlg_getDataTableIDs_pvt);
REXX_METHOD_PROTOTYPE(resdlg_startDialog_pvt);

REXX_METHOD_PROTOTYPE(winex_initWindowExtensions);
REXX_METHOD_PROTOTYPE(winex_getTextSizeScreen);
REXX_METHOD_PROTOTYPE(winex_getFont);
REXX_METHOD_PROTOTYPE(winex_setFont);
REXX_METHOD_PROTOTYPE(winex_createFontEx);
REXX_METHOD_PROTOTYPE(winex_createFont);
REXX_METHOD_PROTOTYPE(winex_getScrollPos);
REXX_METHOD_PROTOTYPE(winex_setScrollPos);
REXX_METHOD_PROTOTYPE(winex_scroll);
REXX_METHOD_PROTOTYPE(winex_setCursorShape);
REXX_METHOD_PROTOTYPE(winex_setCursorPos);
REXX_METHOD_PROTOTYPE(winex_getCursorPos);
REXX_METHOD_PROTOTYPE(winex_restoreCursorShape);
REXX_METHOD_PROTOTYPE(winex_writeDirect);
REXX_METHOD_PROTOTYPE(winex_loadBitmap);
REXX_METHOD_PROTOTYPE(winex_removeBitmap);
REXX_METHOD_PROTOTYPE(winex_write);
REXX_METHOD_PROTOTYPE(winex_createBrush);
REXX_METHOD_PROTOTYPE(winex_createPen);
REXX_METHOD_PROTOTYPE(winex_deleteObject);
REXX_METHOD_PROTOTYPE(winex_getDC);
REXX_METHOD_PROTOTYPE(winex_freeDC);
REXX_METHOD_PROTOTYPE(winex_rectangle);
REXX_METHOD_PROTOTYPE(winex_objectToDC);
REXX_METHOD_PROTOTYPE(winex_drawLine);
REXX_METHOD_PROTOTYPE(winex_drawPixel);
REXX_METHOD_PROTOTYPE(winex_getPixel);
REXX_METHOD_PROTOTYPE(winex_fillDrawing);
REXX_METHOD_PROTOTYPE(winex_drawArcOrPie);
REXX_METHOD_PROTOTYPE(winex_drawAngleArc);
REXX_METHOD_PROTOTYPE(winex_fontColor);
REXX_METHOD_PROTOTYPE(winex_textBkMode);
REXX_METHOD_PROTOTYPE(winex_getSetArcDirection);

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
REXX_METHOD_PROTOTYPE(image_userIcon_cls);
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

REXX_METHOD_PROTOTYPE(dlgctrl_new_cls);
REXX_METHOD_PROTOTYPE(dlgctrl_init);
REXX_METHOD_PROTOTYPE(dlgctrl_unInit);
REXX_METHOD_PROTOTYPE(dlgctrl_connectKeyPress);
REXX_METHOD_PROTOTYPE(dlgctrl_connectFKeyPress);
REXX_METHOD_PROTOTYPE(dlgctrl_disconnectKeyPress);
REXX_METHOD_PROTOTYPE(dlgctrl_hasKeyPressConnection);
REXX_METHOD_PROTOTYPE(dlgctrl_assignFocus);
REXX_METHOD_PROTOTYPE(dlgctrl_tabGroup);
REXX_METHOD_PROTOTYPE(dlgctrl_redrawRect);
REXX_METHOD_PROTOTYPE(dlgctrl_clearRect);
REXX_METHOD_PROTOTYPE(dlgctrl_getTextSizeDlg);
REXX_METHOD_PROTOTYPE(dlgctrl_captureMouse);
REXX_METHOD_PROTOTYPE(dlgctrl_setColor);

// Static
REXX_METHOD_PROTOTYPE(stc_getIcon);
REXX_METHOD_PROTOTYPE(stc_setIcon);
REXX_METHOD_PROTOTYPE(stc_getImage);
REXX_METHOD_PROTOTYPE(stc_setImage);

// Button
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
REXX_METHOD_PROTOTYPE(bc_scroll);
REXX_METHOD_PROTOTYPE(bc_dimBitmap);
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
REXX_METHOD_PROTOTYPE(bc_test_cls);

// Edit
REXX_METHOD_PROTOTYPE(e_isSingleLine);
REXX_METHOD_PROTOTYPE(e_selection);
REXX_METHOD_PROTOTYPE(e_replaceSelText);
REXX_METHOD_PROTOTYPE(e_getLine);
REXX_METHOD_PROTOTYPE(e_lineIndex);
REXX_METHOD_PROTOTYPE(e_setTabStops);
REXX_METHOD_PROTOTYPE(e_showBallon);
REXX_METHOD_PROTOTYPE(e_hideBallon);
REXX_METHOD_PROTOTYPE(e_setCue);
REXX_METHOD_PROTOTYPE(e_style);

// ComboBox
REXX_METHOD_PROTOTYPE(cb_getText);
REXX_METHOD_PROTOTYPE(cb_add);
REXX_METHOD_PROTOTYPE(cb_insert);
REXX_METHOD_PROTOTYPE(cb_select);
REXX_METHOD_PROTOTYPE(cb_find);
REXX_METHOD_PROTOTYPE(cb_addDirectory);

// ScrollBar
REXX_METHOD_PROTOTYPE(sb_getRange);
REXX_METHOD_PROTOTYPE(sb_setRange);
REXX_METHOD_PROTOTYPE(sb_getPosition);
REXX_METHOD_PROTOTYPE(sb_setPosition);

// ListBox
REXX_METHOD_PROTOTYPE(lb_isSingleSelection);
REXX_METHOD_PROTOTYPE(lb_getText);
REXX_METHOD_PROTOTYPE(lb_add);
REXX_METHOD_PROTOTYPE(lb_insert);
REXX_METHOD_PROTOTYPE(lb_select);
REXX_METHOD_PROTOTYPE(lb_selectIndex);
REXX_METHOD_PROTOTYPE(lb_deselectIndex);
REXX_METHOD_PROTOTYPE(lb_selectedIndex);
REXX_METHOD_PROTOTYPE(lb_find);
REXX_METHOD_PROTOTYPE(lb_addDirectory);

// ProgressBar
REXX_METHOD_PROTOTYPE(pbc_stepIt);
REXX_METHOD_PROTOTYPE(pbc_getPos);
REXX_METHOD_PROTOTYPE(pbc_setPos);
REXX_METHOD_PROTOTYPE(pbc_getRange);
REXX_METHOD_PROTOTYPE(pbc_setRange);
REXX_METHOD_PROTOTYPE(pbc_setStep);
REXX_METHOD_PROTOTYPE(pbc_setMarquee);
REXX_METHOD_PROTOTYPE(pbc_setBkColor);
REXX_METHOD_PROTOTYPE(pbc_setBarColor);

// TrackBar
REXX_METHOD_PROTOTYPE(tb_getRange);
REXX_METHOD_PROTOTYPE(tb_getSelRange);

// ListView
REXX_METHOD_PROTOTYPE(lv_setImageList);
REXX_METHOD_PROTOTYPE(lv_getImageList);
REXX_METHOD_PROTOTYPE(lv_getColumnOrder);
REXX_METHOD_PROTOTYPE(lv_setColumnOrder);
REXX_METHOD_PROTOTYPE(lv_stringWidthPx);
REXX_METHOD_PROTOTYPE(lv_insertColumnPx);    // TODO review method name
REXX_METHOD_PROTOTYPE(lv_addRowEx);          // TODO review method name
REXX_METHOD_PROTOTYPE(lv_getColumnCount);
REXX_METHOD_PROTOTYPE(lv_getExtendedStyle);
REXX_METHOD_PROTOTYPE(lv_replaceExtendStyle);
REXX_METHOD_PROTOTYPE(lv_addClearExtendStyle);
REXX_METHOD_PROTOTYPE(lv_hasCheckBoxes);
REXX_METHOD_PROTOTYPE(lv_isChecked);
REXX_METHOD_PROTOTYPE(lv_getCheck);
REXX_METHOD_PROTOTYPE(lv_checkUncheck);

// TreeView
REXX_METHOD_PROTOTYPE(tv_getSpecificItem);
REXX_METHOD_PROTOTYPE(tv_getNextItem);
REXX_METHOD_PROTOTYPE(tv_selectItem);
REXX_METHOD_PROTOTYPE(tv_expand);
REXX_METHOD_PROTOTYPE(tv_subclassEdit);
REXX_METHOD_PROTOTYPE(tv_insert);
REXX_METHOD_PROTOTYPE(tv_hitTestInfo);
REXX_METHOD_PROTOTYPE(tv_setImageList);
REXX_METHOD_PROTOTYPE(tv_getImageList);

// Tab
REXX_METHOD_PROTOTYPE(tab_select);
REXX_METHOD_PROTOTYPE(tab_selected);
REXX_METHOD_PROTOTYPE(tab_insert);
REXX_METHOD_PROTOTYPE(tab_addSequence);
REXX_METHOD_PROTOTYPE(tab_addFullSeq);
REXX_METHOD_PROTOTYPE(tab_modify);
REXX_METHOD_PROTOTYPE(tab_itemInfo);
REXX_METHOD_PROTOTYPE(tab_setItemSize);
REXX_METHOD_PROTOTYPE(tab_setPadding);
REXX_METHOD_PROTOTYPE(tab_getItemRect);
REXX_METHOD_PROTOTYPE(tab_calcRect);
REXX_METHOD_PROTOTYPE(tab_setImageList);
REXX_METHOD_PROTOTYPE(tab_getImageList);

// DateTimePicker
REXX_METHOD_PROTOTYPE(get_dtp_dateTime);
REXX_METHOD_PROTOTYPE(set_dtp_dateTime);

// MonthCalendar
REXX_METHOD_PROTOTYPE(get_mc_date);
REXX_METHOD_PROTOTYPE(set_mc_date);
REXX_METHOD_PROTOTYPE(get_mc_usesUnicode);
REXX_METHOD_PROTOTYPE(set_mc_usesUnicode);

// .Rect
REXX_METHOD_PROTOTYPE(rect_init);
REXX_METHOD_PROTOTYPE(rect_left);
REXX_METHOD_PROTOTYPE(rect_top);
REXX_METHOD_PROTOTYPE(rect_right);
REXX_METHOD_PROTOTYPE(rect_bottom);
REXX_METHOD_PROTOTYPE(rect_setLeft);
REXX_METHOD_PROTOTYPE(rect_setTop);
REXX_METHOD_PROTOTYPE(rect_setRight);
REXX_METHOD_PROTOTYPE(rect_setBottom);

// .Point
REXX_METHOD_PROTOTYPE(point_init);
REXX_METHOD_PROTOTYPE(point_x);
REXX_METHOD_PROTOTYPE(point_setX);
REXX_METHOD_PROTOTYPE(point_y);
REXX_METHOD_PROTOTYPE(point_setY);

// .Size
REXX_METHOD_PROTOTYPE(size_init);
REXX_METHOD_PROTOTYPE(size_cx);
REXX_METHOD_PROTOTYPE(size_setCX);
REXX_METHOD_PROTOTYPE(size_cy);
REXX_METHOD_PROTOTYPE(size_setCY);

// Menu classes methods
REXX_METHOD_PROTOTYPE(menu_menuInit_pvt);
REXX_METHOD_PROTOTYPE(menu_connectSelect_cls);
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
REXX_METHOD_PROTOTYPE(menu_connectMenuEvent);
REXX_METHOD_PROTOTYPE(menu_connectSelect);
REXX_METHOD_PROTOTYPE(menu_connectAllSelects);
REXX_METHOD_PROTOTYPE(menu_connectSomeSelects);
REXX_METHOD_PROTOTYPE(menu_itemTextToMethodName);
REXX_METHOD_PROTOTYPE(menu_test);

REXX_METHOD_PROTOTYPE(menuBar_isAttached);
REXX_METHOD_PROTOTYPE(menuBar_redraw);
REXX_METHOD_PROTOTYPE(menuBar_attachTo);
REXX_METHOD_PROTOTYPE(menuBar_detach);

REXX_METHOD_PROTOTYPE(binMenu_init);

REXX_METHOD_PROTOTYPE(sysMenu_init);
REXX_METHOD_PROTOTYPE(sysMenu_revert);
REXX_METHOD_PROTOTYPE(sysMenu_connectSelect);
REXX_METHOD_PROTOTYPE(sysMenu_connectAllSelects);
REXX_METHOD_PROTOTYPE(sysMenu_connectSomeSelects);

REXX_METHOD_PROTOTYPE(popMenu_connectContextMenu_cls);
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
    REXX_METHOD(dlgutil_init_cls,               dlgutil_init_cls),
    REXX_METHOD(dlgutil_comctl32Version_cls,    dlgutil_comctl32Version_cls),
    REXX_METHOD(dlgutil_version_cls,            dlgutil_version_cls),
    REXX_METHOD(dlgutil_hiWord_cls,             dlgutil_hiWord_cls),
    REXX_METHOD(dlgutil_loWord_cls,             dlgutil_loWord_cls),
    REXX_METHOD(dlgutil_makeLPARAM_cls,         dlgutil_makeLPARAM_cls),
    REXX_METHOD(dlgutil_makeWPARAM_cls,         dlgutil_makeWPARAM_cls),
    REXX_METHOD(dlgutil_and_cls,                dlgutil_and_cls),
    REXX_METHOD(dlgutil_or_cls,                 dlgutil_or_cls),
    REXX_METHOD(dlgutil_screenSize_cls,         dlgutil_screenSize_cls),
    REXX_METHOD(dlgutil_screenArea_cls,         dlgutil_screenArea_cls),
    REXX_METHOD(dlgutil_getSystemMetrics_cls,   dlgutil_getSystemMetrics_cls),
    REXX_METHOD(dlgutil_handleToPointer_cls,    dlgutil_handleToPointer_cls),
    REXX_METHOD(dlgutil_test_cls,               dlgutil_test_cls),

    REXX_METHOD(rsrcUtils_resolveIconID_pvt,    rsrcUtils_resolveIconID_pvt),
    REXX_METHOD(rsrcUtils_resolveResourceID,    rsrcUtils_resolveResourceID),
    REXX_METHOD(rsrcUtils_idError,              rsrcUtils_idError),
    REXX_METHOD(rsrcUtils_checkID,              rsrcUtils_checkID),

    REXX_METHOD(wb_init_windowBase,             wb_init_windowBase),
    REXX_METHOD(wb_getHwnd,                     wb_getHwnd),
    REXX_METHOD(wb_getFactorX,                  wb_getFactorX),
    REXX_METHOD(wb_setFactorX,                  wb_setFactorX),
    REXX_METHOD(wb_getFactorY,                  wb_getFactorY),
    REXX_METHOD(wb_setFactorY,                  wb_setFactorY),
    REXX_METHOD(wb_getSizeX,                    wb_getSizeX),
    REXX_METHOD(wb_setSizeX,                    wb_setSizeX),
    REXX_METHOD(wb_getSizeY,                    wb_getSizeY),
    REXX_METHOD(wb_setSizeY,                    wb_setSizeY),
    REXX_METHOD(wb_getPixelX,                   wb_getPixelX),
    REXX_METHOD(wb_getPixelY,                   wb_getPixelY),
    REXX_METHOD(wb_sendMessage,                 wb_sendMessage),
    REXX_METHOD(wb_sendWinIntMsg,               wb_sendWinIntMsg),
    REXX_METHOD(wb_sendWinHandleMsg,            wb_sendWinHandleMsg),
    REXX_METHOD(wb_sendWinHandle2Msg,           wb_sendWinHandle2Msg),
    REXX_METHOD(wb_enable,                      wb_enable),
    REXX_METHOD(wb_isEnabled,                   wb_isEnabled),
    REXX_METHOD(wb_isVisible,                   wb_isVisible),
    REXX_METHOD(wb_show,                        wb_show),
    REXX_METHOD(wb_showFast,                    wb_showFast),
    REXX_METHOD(wb_display,                     wb_display),
    REXX_METHOD(wb_redrawClient,                wb_redrawClient),
    REXX_METHOD(wb_redraw,                      wb_redraw),
    REXX_METHOD(wb_getText,                     wb_getText),
    REXX_METHOD(wb_setText,                     wb_setText),
    REXX_METHOD(wb_setRect,                     wb_setRect),
    REXX_METHOD(wb_resizeMove,                  wb_resizeMove),
    REXX_METHOD(wb_getSizePos,                  wb_getSizePos),
    REXX_METHOD(wb_windowRect,                  wb_windowRect),
    REXX_METHOD(wb_clientRect,                  wb_clientRect),
    REXX_METHOD(wb_clear,                       wb_clear),
    REXX_METHOD(wb_foreGroundWindow,            wb_foreGroundWindow),
    REXX_METHOD(wb_screenClient,                wb_screenClient),
    REXX_METHOD(wb_getWindowLong_pvt,           wb_getWindowLong_pvt),

    REXX_METHOD(en_init_eventNotification,      en_init_eventNotification),
    REXX_METHOD(en_connectKeyPress,             en_connectKeyPress),
    REXX_METHOD(en_connectFKeyPress,            en_connectFKeyPress),
    REXX_METHOD(en_disconnectKeyPress,          en_disconnectKeyPress),
    REXX_METHOD(en_hasKeyPressConnection,       en_hasKeyPressConnection),
    REXX_METHOD(en_connectCommandEvents,        en_connectCommandEvents),
    REXX_METHOD(en_addUserMessage,              en_addUserMessage),

    REXX_METHOD(pbdlg_init_cls,                 pbdlg_init_cls),
    REXX_METHOD(pbdlg_setDefaultFont_cls,       pbdlg_setDefaultFont_cls),
    REXX_METHOD(pbdlg_getFontName_cls,          pbdlg_getFontName_cls),
    REXX_METHOD(pbdlg_getFontSize_cls,          pbdlg_getFontSize_cls),
    REXX_METHOD(pbdlg_init,                     pbdlg_init),
    REXX_METHOD(pbdlg_getFontName,              pbdlg_getFontName),
    REXX_METHOD(pbdlg_getFontSize,              pbdlg_getFontSize),
    REXX_METHOD(pbdlg_setDlgFont,               pbdlg_setDlgFont),
    REXX_METHOD(pbdlg_getAutoDetect,            pbdlg_getAutoDetect),
    REXX_METHOD(pbdlg_setAutoDetect,            pbdlg_setAutoDetect),
    REXX_METHOD(pbdlg_sendMessageToControl,     pbdlg_sendMessageToControl),
    REXX_METHOD(pbdlg_sendMessageToWindow,      pbdlg_sendMessageToWindow),
    REXX_METHOD(pbdlg_get,                      pbdlg_get),
    REXX_METHOD(pbdlg_getDlgHandle,             pbdlg_getDlgHandle),
    REXX_METHOD(pbdlg_isDialogActive,           pbdlg_isDialogActive),
    REXX_METHOD(pbdlg_show,                     pbdlg_show),
    REXX_METHOD(pbdlg_showWindow,               pbdlg_showWindow),
    REXX_METHOD(pbdlg_toTheTop,                 pbdlg_toTheTop),
    REXX_METHOD(pbdlg_getFocus,                 pbdlg_getFocus),
    REXX_METHOD(pbdlg_setFocus,                 pbdlg_setFocus),
    REXX_METHOD(pbdlg_tabTo,                    pbdlg_tabTo),
    REXX_METHOD(pbdlg_backgroundBitmap,         pbdlg_backgroundBitmap),
    REXX_METHOD(pbdlg_tiledBackgroundBitmap,    pbdlg_tiledBackgroundBitmap),
    REXX_METHOD(pbdlg_backgroundColor,          pbdlg_backgroundColor),
    REXX_METHOD(pbdlg_pixel2dlgUnit,            pbdlg_pixel2dlgUnit),
    REXX_METHOD(pbdlg_dlgUnit2pixel,            pbdlg_dlgUnit2pixel),
    REXX_METHOD(pbdlg_focusControl,             pbdlg_focusControl),
    REXX_METHOD(pbdlg_showControl,              pbdlg_showControl),
    REXX_METHOD(pbdlg_connect_ControName,       pbdlg_connect_ControName),
    REXX_METHOD(pbdlg_setDlgDataFromStem_pvt,   pbdlg_setDlgDataFromStem_pvt),
    REXX_METHOD(pbdlg_putDlgDataInStem_pvt,     pbdlg_putDlgDataInStem_pvt),
    REXX_METHOD(pbdlg_getControlData,           pbdlg_getControlData),
    REXX_METHOD(pbdlg_setControlData,           pbdlg_setControlData),
    REXX_METHOD(pbdlg_getControlHandle,         pbdlg_getControlHandle),
    REXX_METHOD(pbdlg_getWindowText,            pbdlg_getWindowText),
    REXX_METHOD(pbdlg_setWindowText,            pbdlg_setWindowText),
    REXX_METHOD(pbdlg_getControlText,           pbdlg_getControlText),
    REXX_METHOD(pbdlg_setControlText,           pbdlg_setControlText),
    REXX_METHOD(pbdlg_enableDisableControl,     pbdlg_enableDisableControl),
    REXX_METHOD(pbdlg_getControlID,             pbdlg_getControlID),
    REXX_METHOD(pbdlg_center,                   pbdlg_center),
    REXX_METHOD(pbdlg_doMinMax,                 pbdlg_doMinMax),
    REXX_METHOD(pbdlg_setTabGroup,              pbdlg_setTabGroup),
    REXX_METHOD(pbdlg_stopIt,                   pbdlg_stopIt),
    REXX_METHOD(pbdlg_getTextSizeDlg,           pbdlg_getTextSizeDlg),
    REXX_METHOD(pbdlg_newControl,               pbdlg_newControl),
    REXX_METHOD(pbdlg_putControl_pvt,           pbdlg_putControl_pvt),
    REXX_METHOD(pbdlg_unInit,                   pbdlg_unInit),

    REXX_METHOD(generic_setListTabulators,      generic_setListTabulators),

    REXX_METHOD(dlgext_setWindowRect,           dlgext_setWindowRect),
    REXX_METHOD(dlgext_clearWindowRect,         dlgext_clearWindowRect),
    REXX_METHOD(dlgext_clearControlRect,        dlgext_clearControlRect),
    REXX_METHOD(dlgext_clearRect,               dlgext_clearRect),
    REXX_METHOD(dlgext_getControlRect,          dlgext_getControlRect),
    REXX_METHOD(dlgext_redrawWindowRect,        dlgext_redrawWindowRect),
    REXX_METHOD(dlgext_redrawControl,           dlgext_redrawControl),
    REXX_METHOD(dlgext_redrawRect,              dlgext_redrawRect),
    REXX_METHOD(dlgext_resizeMoveControl,       dlgext_resizeMoveControl),
    REXX_METHOD(dlgext_setForgroundWindow,      dlgext_setForgroundWindow),
    REXX_METHOD(dlgext_setControlColor,         dlgext_setControlColor),
    REXX_METHOD(dlgext_installBitmapButton,     dlgext_installBitmapButton),
    REXX_METHOD(dlgext_changeBitmapButton,      dlgext_changeBitmapButton),
    REXX_METHOD(dlgext_drawBitmap,              dlgext_drawBitmap),
    REXX_METHOD(dlgext_drawButton,              dlgext_drawButton),
    REXX_METHOD(dlgext_getBitmapPosition,       dlgext_getBitmapPosition),
    REXX_METHOD(dlgext_setBitmapPosition,       dlgext_setBitmapPosition),
    REXX_METHOD(dlgext_getBitmapSize,           dlgext_getBitmapSize),
    REXX_METHOD(dlgext_mouseCapture,            dlgext_mouseCapture),
    REXX_METHOD(dlgext_captureMouse,            dlgext_captureMouse),
    REXX_METHOD(dlgext_isMouseButtonDown,       dlgext_isMouseButtonDown),
    REXX_METHOD(dlgext_getWindowDC,             dlgext_getWindowDC),
    REXX_METHOD(dlgext_freeWindowDC,            dlgext_freeWindowDC),
    REXX_METHOD(dlgext_writeToWindow,           dlgext_writeToWindow),
    REXX_METHOD(dlgext_scrollText,              dlgext_scrollText),
    REXX_METHOD(dlgext_createBrush,             dlgext_createBrush),
    REXX_METHOD(dlgext_dumpAdmin_pvt,           dlgext_dumpAdmin_pvt),

    REXX_METHOD(baseDlg_init,                   baseDlg_init),
    REXX_METHOD(baseDlg_test,                   baseDlg_test),

    REXX_METHOD(userdlg_init,                   userdlg_init),

    REXX_METHOD(catdlg_createCategoryDialog,           catdlg_createCategoryDialog),
    REXX_METHOD(catdlg_getControlDataPage,             catdlg_getControlDataPage),
    REXX_METHOD(catdlg_setControlDataPage,             catdlg_setControlDataPage),
    REXX_METHOD(catdlg_getCategoryComboEntry,          catdlg_getCategoryComboEntry),
    REXX_METHOD(catdlg_sendMessageToCategoryControl,   catdlg_sendMessageToCategoryControl),

    REXX_METHOD(dyndlg_init_cls,                dyndlg_init_cls),
    REXX_METHOD(dyndlg_getBasePtr,              dyndlg_getBasePtr),
    REXX_METHOD(dyndlg_setBasePtr,              dyndlg_setBasePtr),
    REXX_METHOD(dyndlg_getActivePtr,            dyndlg_getActivePtr),
    REXX_METHOD(dyndlg_setActivePtr,            dyndlg_setActivePtr),
    REXX_METHOD(dyndlg_getDialogItemCount,      dyndlg_getDialogItemCount),
    REXX_METHOD(dyndlg_setDialogItemCount,      dyndlg_setDialogItemCount),
    REXX_METHOD(dyndlg_dynamicInit,             dyndlg_dynamicInit),
    REXX_METHOD(dyndlg_create,                  dyndlg_create),
    REXX_METHOD(dyndlg_createStatic,            dyndlg_createStatic),
    REXX_METHOD(dyndlg_createStaticText,        dyndlg_createStaticText),
    REXX_METHOD(dyndlg_createStaticImage,       dyndlg_createStaticImage),
    REXX_METHOD(dyndlg_createStaticFrame,       dyndlg_createStaticFrame),
    REXX_METHOD(dyndlg_createPushButton,        dyndlg_createPushButton),
    REXX_METHOD(dyndlg_createRadioButton,       dyndlg_createRadioButton),
    REXX_METHOD(dyndlg_createGroupBox,          dyndlg_createGroupBox),
    REXX_METHOD(dyndlg_createEdit,              dyndlg_createEdit),
    REXX_METHOD(dyndlg_createScrollBar,         dyndlg_createScrollBar),
    REXX_METHOD(dyndlg_createListBox,           dyndlg_createListBox),
    REXX_METHOD(dyndlg_createComboBox,          dyndlg_createComboBox),
    REXX_METHOD(dyndlg_createProgressBar,       dyndlg_createProgressBar),
    REXX_METHOD(dyndlg_createNamedControl,      dyndlg_createNamedControl),
    REXX_METHOD(dyndlg_addButton,               dyndlg_addButton),
    REXX_METHOD(dyndlg_addRadioButton,          dyndlg_addRadioButton),
    REXX_METHOD(dyndlg_addGroupBox,             dyndlg_addGroupBox),
    REXX_METHOD(dyndlg_addEntryLine,            dyndlg_addEntryLine),
    REXX_METHOD(dyndlg_addMethod,               dyndlg_addMethod),
    REXX_METHOD(dyndlg_addIconResource,         dyndlg_addIconResource),
    REXX_METHOD(dyndlg_startParentDialog,       dyndlg_startParentDialog),
    REXX_METHOD(dyndlg_startChildDialog,        dyndlg_startChildDialog),
    REXX_METHOD(dyndlg_stop,                    dyndlg_stop),
    REXX_METHOD(dyndlg_stopDynamic_pvt,         dyndlg_stopDynamic_pvt),

    REXX_METHOD(dlgctrl_new_cls,                dlgctrl_new_cls),
    REXX_METHOD(dlgctrl_init,                   dlgctrl_init),
    REXX_METHOD(dlgctrl_unInit,                 dlgctrl_unInit),
    REXX_METHOD(dlgctrl_connectKeyPress,        dlgctrl_connectKeyPress),
    REXX_METHOD(dlgctrl_connectFKeyPress,       dlgctrl_connectFKeyPress),
    REXX_METHOD(dlgctrl_disconnectKeyPress,     dlgctrl_disconnectKeyPress),
    REXX_METHOD(dlgctrl_hasKeyPressConnection,  dlgctrl_hasKeyPressConnection),
    REXX_METHOD(dlgctrl_assignFocus,            dlgctrl_assignFocus),
    REXX_METHOD(dlgctrl_tabGroup,               dlgctrl_tabGroup),
    REXX_METHOD(dlgctrl_clearRect,              dlgctrl_clearRect),
    REXX_METHOD(dlgctrl_redrawRect,             dlgctrl_redrawRect),
    REXX_METHOD(dlgctrl_getTextSizeDlg,         dlgctrl_getTextSizeDlg),
    REXX_METHOD(dlgctrl_captureMouse,           dlgctrl_captureMouse),
    REXX_METHOD(dlgctrl_setColor,               dlgctrl_setColor),

    REXX_METHOD(window_init,                    window_init),
    REXX_METHOD(window_unInit,                  window_unInit),

    REXX_METHOD(resdlg_getDataTableIDs_pvt,     resdlg_getDataTableIDs_pvt),
    REXX_METHOD(resdlg_startDialog_pvt,         resdlg_startDialog_pvt),

    REXX_METHOD(winex_initWindowExtensions,     winex_initWindowExtensions),
    REXX_METHOD(winex_getFont,                  winex_getFont),
    REXX_METHOD(winex_setFont,                  winex_setFont),
    REXX_METHOD(winex_getTextSizeScreen,        winex_getTextSizeScreen),
    REXX_METHOD(winex_createFontEx,             winex_createFontEx),
    REXX_METHOD(winex_createFont,               winex_createFont),
    REXX_METHOD(winex_getScrollPos,             winex_getScrollPos),
    REXX_METHOD(winex_setScrollPos,        	    winex_setScrollPos),
    REXX_METHOD(winex_scroll,        	        winex_scroll),
    REXX_METHOD(winex_setCursorShape,           winex_setCursorShape),
    REXX_METHOD(winex_setCursorPos,             winex_setCursorPos),
    REXX_METHOD(winex_getCursorPos,             winex_getCursorPos),
    REXX_METHOD(winex_restoreCursorShape,       winex_restoreCursorShape),
    REXX_METHOD(winex_writeDirect,              winex_writeDirect),
    REXX_METHOD(winex_loadBitmap,               winex_loadBitmap),
    REXX_METHOD(winex_removeBitmap,             winex_removeBitmap),
    REXX_METHOD(winex_write,                    winex_write),
    REXX_METHOD(winex_createBrush,              winex_createBrush),
    REXX_METHOD(winex_createPen,                winex_createPen),
    REXX_METHOD(winex_deleteObject,             winex_deleteObject),
    REXX_METHOD(winex_objectToDC,               winex_objectToDC),
    REXX_METHOD(winex_getDC,                    winex_getDC),
    REXX_METHOD(winex_freeDC,                   winex_freeDC),
    REXX_METHOD(winex_rectangle,                winex_rectangle),
    REXX_METHOD(winex_drawLine,                 winex_drawLine),
    REXX_METHOD(winex_drawPixel,                winex_drawPixel),
    REXX_METHOD(winex_getPixel,                 winex_getPixel),
    REXX_METHOD(winex_fillDrawing,              winex_fillDrawing),
    REXX_METHOD(winex_drawArcOrPie,             winex_drawArcOrPie),
    REXX_METHOD(winex_drawAngleArc,             winex_drawAngleArc),
    REXX_METHOD(winex_fontColor,                winex_fontColor),
    REXX_METHOD(winex_textBkMode,               winex_textBkMode),
    REXX_METHOD(winex_getSetArcDirection,       winex_getSetArcDirection),

    REXX_METHOD(ri_init,                        ri_init),
    REXX_METHOD(ri_release,                     ri_release),
    REXX_METHOD(ri_handle,                      ri_handle),
    REXX_METHOD(ri_isNull,                      ri_isNull),
    REXX_METHOD(ri_systemErrorCode,             ri_systemErrorCode),
    REXX_METHOD(ri_getImage,                    ri_getImage),
    REXX_METHOD(ri_getImages,                   ri_getImages),

    REXX_METHOD(image_toID_cls,                 image_toID_cls),
    REXX_METHOD(image_getImage_cls,             image_getImage_cls),
    REXX_METHOD(image_fromFiles_cls,            image_fromFiles_cls),
    REXX_METHOD(image_fromIDs_cls,              image_fromIDs_cls),
    REXX_METHOD(image_userIcon_cls,             image_userIcon_cls),
    REXX_METHOD(image_colorRef_cls,             image_colorRef_cls),
    REXX_METHOD(image_getRValue_cls,            image_getRValue_cls),
    REXX_METHOD(image_getGValue_cls,            image_getGValue_cls),
    REXX_METHOD(image_getBValue_cls,            image_getBValue_cls),
    REXX_METHOD(image_init,                     image_init),
    REXX_METHOD(image_release,                  image_release),
    REXX_METHOD(image_isNull,                   image_isNull),
    REXX_METHOD(image_systemErrorCode,          image_systemErrorCode),
    REXX_METHOD(image_handle,                   image_handle),

    REXX_METHOD(il_create_cls,                  il_create_cls),
    REXX_METHOD(il_init,                        il_init),
    REXX_METHOD(il_release,                     il_release),
    REXX_METHOD(il_add,                         il_add),
    REXX_METHOD(il_addMasked,                   il_addMasked),
    REXX_METHOD(il_addIcon,                     il_addIcon),
    REXX_METHOD(il_addImages,                   il_addImages),
    REXX_METHOD(il_getCount,                    il_getCount),
    REXX_METHOD(il_getImageSize,                il_getImageSize),
    REXX_METHOD(il_duplicate,                   il_duplicate),
    REXX_METHOD(il_removeAll,                   il_removeAll),
    REXX_METHOD(il_remove,                      il_remove),
    REXX_METHOD(il_isNull,                      il_isNull),
    REXX_METHOD(il_handle,                      il_handle),

    // ListView
    REXX_METHOD(lv_setImageList,                lv_setImageList),
    REXX_METHOD(lv_getImageList,                lv_getImageList),
    REXX_METHOD(lv_getColumnOrder,              lv_getColumnOrder),
    REXX_METHOD(lv_setColumnOrder,              lv_setColumnOrder),
    REXX_METHOD(lv_stringWidthPx,               lv_stringWidthPx),
    REXX_METHOD(lv_insertColumnPx,              lv_insertColumnPx),     // TODO review method name
    REXX_METHOD(lv_addRowEx,         	        lv_addRowEx),           // TODO review method name
    REXX_METHOD(lv_getColumnCount,              lv_getColumnCount),
    REXX_METHOD(lv_getExtendedStyle,            lv_getExtendedStyle),
    REXX_METHOD(lv_replaceExtendStyle,          lv_replaceExtendStyle),
    REXX_METHOD(lv_addClearExtendStyle,         lv_addClearExtendStyle),
    REXX_METHOD(lv_hasCheckBoxes,               lv_hasCheckBoxes),
    REXX_METHOD(lv_isChecked,                   lv_isChecked),
    REXX_METHOD(lv_getCheck,                    lv_getCheck),
    REXX_METHOD(lv_checkUncheck,                lv_checkUncheck),

    // Edit
    REXX_METHOD(e_isSingleLine,                 e_isSingleLine),
    REXX_METHOD(e_selection,                    e_selection),
    REXX_METHOD(e_replaceSelText,               e_replaceSelText),
    REXX_METHOD(e_getLine,                      e_getLine),
    REXX_METHOD(e_lineIndex,                    e_lineIndex),
    REXX_METHOD(e_setTabStops,                  e_setTabStops),
    REXX_METHOD(e_style,                        e_style),
    REXX_METHOD(e_showBallon,                   e_showBallon),
    REXX_METHOD(e_hideBallon,                   e_hideBallon),
    REXX_METHOD(e_setCue,                       e_setCue),

    // TreeView
    REXX_METHOD(tv_getSpecificItem,             tv_getSpecificItem),
    REXX_METHOD(tv_getNextItem,                 tv_getNextItem),
    REXX_METHOD(tv_selectItem,                  tv_selectItem),
    REXX_METHOD(tv_expand,                      tv_expand),
    REXX_METHOD(tv_subclassEdit,                tv_subclassEdit),
    REXX_METHOD(tv_insert,                      tv_insert),
    REXX_METHOD(tv_hitTestInfo,                 tv_hitTestInfo),
    REXX_METHOD(tv_setImageList,                tv_setImageList),
    REXX_METHOD(tv_getImageList,                tv_getImageList),

    // Tab
    REXX_METHOD(tab_select,                     tab_select),
    REXX_METHOD(tab_selected,                   tab_selected),
    REXX_METHOD(tab_insert,                     tab_insert),
    REXX_METHOD(tab_addSequence,                tab_addSequence),
    REXX_METHOD(tab_addFullSeq,                 tab_addFullSeq),
    REXX_METHOD(tab_itemInfo,                   tab_itemInfo),
    REXX_METHOD(tab_modify,                     tab_modify),
    REXX_METHOD(tab_setItemSize,                tab_setItemSize),
    REXX_METHOD(tab_setPadding,                 tab_setPadding),
    REXX_METHOD(tab_getItemRect,                tab_getItemRect),
    REXX_METHOD(tab_calcRect,                   tab_calcRect),
    REXX_METHOD(tab_setImageList,               tab_setImageList),
    REXX_METHOD(tab_getImageList,               tab_getImageList),

    REXX_METHOD(get_dtp_dateTime,               get_dtp_dateTime),
    REXX_METHOD(set_dtp_dateTime,               set_dtp_dateTime),

    REXX_METHOD(get_mc_date,                    get_mc_date),
    REXX_METHOD(set_mc_date,                    set_mc_date),
    REXX_METHOD(get_mc_usesUnicode,             get_mc_usesUnicode),
    REXX_METHOD(set_mc_usesUnicode,             set_mc_usesUnicode),

    REXX_METHOD(pbc_stepIt,                     pbc_stepIt),
    REXX_METHOD(pbc_getPos,                     pbc_getPos),
    REXX_METHOD(pbc_setPos,                     pbc_setPos),
    REXX_METHOD(pbc_getRange,                   pbc_getRange),
    REXX_METHOD(pbc_setRange,                   pbc_setRange),
    REXX_METHOD(pbc_setStep,                    pbc_setStep),
    REXX_METHOD(pbc_setMarquee,                 pbc_setMarquee),
    REXX_METHOD(pbc_setBkColor,                 pbc_setBkColor),
    REXX_METHOD(pbc_setBarColor,                pbc_setBarColor),

    // TrackBar
    REXX_METHOD(tb_getRange,                    tb_getRange),
    REXX_METHOD(tb_getSelRange,                 tb_getSelRange),

    // ProgressBar
    REXX_METHOD(sb_getRange,                    sb_getRange),
    REXX_METHOD(sb_setRange,                    sb_setRange),
    REXX_METHOD(sb_getPosition,                 sb_getPosition),
    REXX_METHOD(sb_setPosition,                 sb_setPosition),

    REXX_METHOD(stc_getIcon,                    stc_getIcon),
    REXX_METHOD(stc_setIcon,                    stc_setIcon),
    REXX_METHOD(stc_getImage,                   stc_getImage),
    REXX_METHOD(stc_setImage,                   stc_setImage),

    // Buttons
    REXX_METHOD(gb_setStyle,                    gb_setStyle),
    REXX_METHOD(bc_getState,                    bc_getState),
    REXX_METHOD(bc_setState,                    bc_setState),
    REXX_METHOD(bc_setStyle,                    bc_setStyle),
    REXX_METHOD(bc_click,                       bc_click),
    REXX_METHOD(bc_getIdealSize,                bc_getIdealSize),
    REXX_METHOD(bc_getTextMargin,               bc_getTextMargin),
    REXX_METHOD(bc_setTextMargin,               bc_setTextMargin),
    REXX_METHOD(bc_getImage,                    bc_getImage),
    REXX_METHOD(bc_setImage,                    bc_setImage),
    REXX_METHOD(bc_setImageList,                bc_setImageList),
    REXX_METHOD(bc_getImageList,                bc_getImageList),
    REXX_METHOD(bc_scroll,                      bc_scroll),
    REXX_METHOD(bc_dimBitmap,                   bc_dimBitmap),
    REXX_METHOD(rb_checkInGroup_cls,            rb_checkInGroup_cls),
    REXX_METHOD(rb_checked,                     rb_checked),
    REXX_METHOD(rb_check,                       rb_check),
    REXX_METHOD(rb_uncheck,                     rb_uncheck),
    REXX_METHOD(rb_getCheckState,               rb_getCheckState),
    REXX_METHOD(rb_isChecked,                   rb_isChecked),
    REXX_METHOD(rb_indeterminate,               rb_indeterminate),
    REXX_METHOD(ckbx_isIndeterminate,           ckbx_isIndeterminate),
    REXX_METHOD(ckbx_setIndeterminate,          ckbx_setIndeterminate),
    REXX_METHOD(bc_test,                        bc_test),
    REXX_METHOD(bc_test_cls,                    bc_test_cls),

    // ListBox
    REXX_METHOD(lb_isSingleSelection,           lb_isSingleSelection),
    REXX_METHOD(lb_getText,                     lb_getText),
    REXX_METHOD(lb_add,                         lb_add),
    REXX_METHOD(lb_insert,                      lb_insert),
    REXX_METHOD(lb_select,                      lb_select),
    REXX_METHOD(lb_selectIndex,               lb_selectIndex),
    REXX_METHOD(lb_deselectIndex,               lb_deselectIndex),
    REXX_METHOD(lb_selectedIndex,               lb_selectedIndex),
    REXX_METHOD(lb_find,                        lb_find),
    REXX_METHOD(lb_addDirectory,                lb_addDirectory),

    // ComboBox
    REXX_METHOD(cb_getText,                     cb_getText),
    REXX_METHOD(cb_add,                         cb_add),
    REXX_METHOD(cb_insert,                      cb_insert),
    REXX_METHOD(cb_select,                      cb_select),
    REXX_METHOD(cb_find,                        cb_find),
    REXX_METHOD(cb_addDirectory,                cb_addDirectory),

    REXX_METHOD(rect_init,                      rect_init),
    REXX_METHOD(rect_left,                      rect_left),
    REXX_METHOD(rect_top,                       rect_top),
    REXX_METHOD(rect_right,                     rect_right),
    REXX_METHOD(rect_bottom,                    rect_bottom),
    REXX_METHOD(rect_setLeft,                   rect_setLeft),
    REXX_METHOD(rect_setTop,                    rect_setTop),
    REXX_METHOD(rect_setRight,                  rect_setRight),
    REXX_METHOD(rect_setBottom,                 rect_setBottom),
    REXX_METHOD(point_init,                     point_init),
    REXX_METHOD(point_x,                        point_x),
    REXX_METHOD(point_setX,                     point_setX),
    REXX_METHOD(point_y,                        point_y),
    REXX_METHOD(point_setY,                     point_setY),
    REXX_METHOD(size_init,                      size_init),
    REXX_METHOD(size_cx,                        size_cx),
    REXX_METHOD(size_setCX,                     size_setCX),
    REXX_METHOD(size_cy,                        size_cy),
    REXX_METHOD(size_setCY,                     size_setCY),

    // Menu classes methods
    REXX_METHOD(menu_menuInit_pvt,              menu_menuInit_pvt),
    REXX_METHOD(menu_connectSelect_cls,         menu_connectSelect_cls),
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
    REXX_METHOD(menu_connectMenuEvent,          menu_connectMenuEvent),
    REXX_METHOD(menu_connectSelect,             menu_connectSelect),
    REXX_METHOD(menu_connectSomeSelects,        menu_connectSomeSelects),
    REXX_METHOD(menu_connectAllSelects,         menu_connectAllSelects),
    REXX_METHOD(menu_itemTextToMethodName,      menu_itemTextToMethodName),
    REXX_METHOD(menu_test,                      menu_test),

    REXX_METHOD(menuBar_isAttached,             menuBar_isAttached),
    REXX_METHOD(menuBar_redraw,                 menuBar_redraw),
    REXX_METHOD(menuBar_attachTo,               menuBar_attachTo),
    REXX_METHOD(menuBar_detach,                 menuBar_detach),

    REXX_METHOD(binMenu_init,                   binMenu_init),

    REXX_METHOD(sysMenu_init,                   sysMenu_init),
    REXX_METHOD(sysMenu_revert,                 sysMenu_revert),
    REXX_METHOD(sysMenu_connectSelect,          sysMenu_connectSelect),
    REXX_METHOD(sysMenu_connectSomeSelects,     sysMenu_connectSomeSelects),
    REXX_METHOD(sysMenu_connectAllSelects,      sysMenu_connectAllSelects),

    REXX_METHOD(popMenu_connectContextMenu_cls, popMenu_connectContextMenu_cls),
    REXX_METHOD(popMenu_init,                   popMenu_init),
    REXX_METHOD(popMenu_isAssigned,             popMenu_isAssigned),
    REXX_METHOD(popMenu_connectContextMenu,     popMenu_connectContextMenu),
    REXX_METHOD(popMenu_assignTo,               popMenu_assignTo),
    REXX_METHOD(popMenu_track,                  popMenu_track),
    REXX_METHOD(popMenu_show,                   popMenu_show),

    REXX_METHOD(scriptMenu_init,                scriptMenu_init),

    REXX_METHOD(userMenu_init,                  userMenu_init),
    REXX_METHOD(userMenu_complete,              userMenu_complete),

    REXX_METHOD(menuTemplate_isComplete,        menuTemplate_isComplete),
    REXX_METHOD(menuTemplate_addSeparator,      menuTemplate_addSeparator),
    REXX_METHOD(menuTemplate_addItem,           menuTemplate_addItem),
    REXX_METHOD(menuTemplate_addPopup,          menuTemplate_addPopup),

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

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
 * oodPropertySheetDialog.cpp
 *
 * Contains the classes used to implement a Property Sheet dialog, (as oppossed
 * to the PropertySheet.cls which is not a Windows property sheet at all.)
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodUser.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodData.hpp"
#include "oodResources.hpp"
#include "oodResourceIDs.hpp"
#include "oodControl.hpp"
#include "oodResizableDialog.hpp"
#include "oodShared.hpp"
#include "oodPropertySheetDialog.hpp"


PROPSHEETHOOKDATA *PropSheetHookData[MAX_PROPSHEET_DIALOGS];
size_t CountPropSheetHooks = 0;


class PropSheetThreadArgs
{
public:
    pCPropertySheetDialog  pcpsd;
    PROPSHEETHEADER       *psh;
    bool                  *release; // Used to signal thread initialization complete
};

#define VALID_PSNRET_LIST       "PSNRET_NOERROR, PSNRET_INVALID, or PSNRET_INVALID_NOCHANGEPAGE"
#define VALID_PSNRET_MSG_LIST   "PSNRET_NOERROR or PSNRET_MESSAGEHANDLED"
#define VALID_PROPSHEET_BUTTONS  "APPLYNOW, BACK, CANCEL, FINISH, HELP, NEXT, or OK"
#define VALID_AERO_BUTTONS       "BACK, CANCEL, FINISH, or NEXT"

/**
 * The following set of helper functions, some of which are named tcXXX (for
 * thread context), are used by both PropertySheetDialog and TabOwnerDialog.
 */

/**
 *
 * Checks for raised conditions during the sending of messages to a Rexx object
 * during the window message loop procedures.
 *
 * @param c
 * @param pcpbd
 *
 * @return bool
 */
bool tcCheckForCondition(RexxThreadContext *c, pCPlainBaseDialog pcpbd)
{
    if ( checkForCondition(c, true) )
    {
        endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
        return true;
    }
    return false;
}

/**
 * Checks that the reply from sending a message to a Rexx object is not null.
 *
 * @param c
 * @param pcpbd
 * @param r
 * @param method
 *
 * @return bool
 */
bool goodReply(RexxThreadContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr r, CSTRING method)
{
    if ( ! tcCheckForCondition(c, pcpbd) )
    {
        if ( r == NULLOBJECT )
        {
            c->RaiseException1(Rexx_Error_No_result_object_message, c->String(method));
            tcCheckForCondition(c, pcpbd);
            return false;
        }
        return true;
    }
    return false;
}

/**
 * Raises an out of memory exception, prints it, and ends the dialog.
 *
 * @param c
 * @param pcpbd
 */
inline void tcMemoryErr(RexxThreadContext *c, pCPlainBaseDialog pcpbd)
{
    outOfMemoryException(c);
    tcCheckForCondition(c, pcpbd);
}


/**
 *  98.900
 *  Error 98 - Execution error
 *        The language processor detected a specific error during execution.
 *
 *  The return from method "name"() must a whole number from "min" to "max";
 *  found "value"
 *
 *  The return from method setActive() must be a whole number from -1 to
 *  2,147,483,647; found an Array
 *
 *  The exception is raised, printed, and the dialog is ended.
 */
void tcWrongRangeException(RexxThreadContext *c, CSTRING method, int32_t min, int32_t max, RexxObjectPtr actual,
                            pCPlainBaseDialog pcpbd)
{
    TCHAR buf[256];
    _snprintf(buf, sizeof(buf), "The return from method %s() must be a whole number from %d to %d; found %s",
              method, min, max, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
    tcCheckForCondition(c, pcpbd);
}


/**
 *  98.900
 *  Error 98 - Execution error
 *        The language processor detected a specific error during execution.
 *
 *  The return from method "name"() must one of "a list"; found "value"
 *
 *  The return from method killActive() must be one of PSNRET_NOERROR,
 *  PSNRET_INVALID, or PSNRET_INVALID_NOCHANGEPAGE; found 15
 *
 *  The exception is raised, printed, and the dialog is ended.
 */
void tcInvalidReturnListException(RexxThreadContext *c, CSTRING method, CSTRING list, RexxObjectPtr actual,
                                   pCPlainBaseDialog pcpbd)
{
    TCHAR buf[256];
    _snprintf(buf, sizeof(buf), "The return from method %s() must be one of %s; found %s",
              method, list, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
    tcCheckForCondition(c, pcpbd);
}


RexxStringObject wm2string(RexxThreadContext *c, uint32_t wmMsg)
{
    char *str = "";
    switch ( wmMsg )
    {
        case WM_KEYDOWN :
            str = "KEYDOWN";
            break;
        case WM_KEYUP       :
            str = "KEYUP";
            break;
        case WM_CHAR        :
            str = "CHAR";
            break;
        case WM_DEADCHAR    :
            str = "DEADCHAR";
            break;
        case WM_SYSKEYDOWN  :
            str = "SYSKEYDOWN";
            break;
        case WM_SYSKEYUP    :
            str = "SYSKEYUP";
            break;
        case WM_SYSCHAR     :
            str = "SYSCHAR";
            break;
        case WM_SYSDEADCHAR :
            str = "SYSDEADCHAR";
            break;
    }
    return c->String(str);
}

/**
 * Creates a Rexx argument array for the property sheet PSN_TRANSLATEACCELERATOR
 * notification.
 *
 * @param c       Thread context we are operating in.
 * @param wmMsg   The windows message (WM_xxx).
 * @param wParam  The WPARAM for the message, the charater code for the key.
 * @param lParam  The LPARAM for the message, contains state info.
 *
 * @return An array of args.
 *
 * @remarks
 */
RexxArrayObject getTranslateAccelatorArgs(RexxThreadContext *c, uint32_t _wmMsg, WPARAM wParam, LPARAM lParam,
                                          RexxObjectPtr propSheet)
{
    RexxStringObject wmMsg = wm2string(c, _wmMsg);
    RexxObjectPtr    keyCode = c->WholeNumber(wParam);

    BOOL released   = (lParam & KEY_RELEASED)   ? 1 : 0;
    BOOL wasDown    = (lParam & KEY_WASDOWN)    ? 1 : 0;
    BOOL isExtended = (lParam & KEY_ISEXTENDED) ? 1 : 0;
    BOOL altHeld    = (lParam & KEY_ALTHELD)    ? 1 : 0;

    BOOL shiftHeld   = (GetAsyncKeyState(VK_SHIFT) & ISDOWN)   ? 1 : 0;
    BOOL controlHeld = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;

    RexxDirectoryObject d = (RexxDirectoryObject)rxNewBuiltinObject(c, "DIRECTORY");

    c->DirectoryPut(d, c->Logical(released),    "RELEASED");
    c->DirectoryPut(d, c->Logical(wasDown),     "WASDOWN");
    c->DirectoryPut(d, c->Logical(isExtended),  "ISEXTENDED");
    c->DirectoryPut(d, c->Logical(altHeld),     "ALTHELD");
    c->DirectoryPut(d, c->Logical(shiftHeld),   "SHIFTHELD");
    c->DirectoryPut(d, c->Logical(controlHeld), "CONTROLHELD");

    RexxArrayObject args = c->ArrayOfFour(wmMsg, keyCode, d, propSheet);

    return args;
}


/**
 * Translates the Rexx reply for a PSN_SETACTIVE notification to the Windows
 * equivalent.
 *
 * Note that the replies to PSN_SETACTIVE, PSN_WIZBACK, and PSN_WIZNEXT are all
 * handled by this function.
 *
 * The Rexx programmer returns 0 to allow the page to become active (or to go to
 * the previous page for WIZBACK, or to go to the next page for WIZNEXT,) -1 to
 * prevent a page change, and the page index, or page ID, to go to a specific
 * page.  The Windows property sheet actually uses the page ID.
 *
 * The Rexx programmer can get the proper page ID by using the indexToID()
 * method.  That method returns a .Pointer object.  Otherwise, it seems simplier
 * for the Rexx programmet to simply use the page index and we'll look it up
 * here.  The short of it is, the Rexx progammer can either reply using the page
 * ID, in which case result is a Pointer object.  Or they can simple use the
 * page index.
 *
 * @param c
 * @param pcpsd
 * @param result
 * @param name
 *
 * @return INT_PRT
 */
intptr_t getSetActiveValue(RexxThreadContext *c, pCPropertySheetDialog pcpsd, RexxObjectPtr result, CSTRING name)
{
    intptr_t ret = -2;

    if ( c->IsPointer(result) )
    {
        return (intptr_t)c->PointerValue((RexxPointerObject)result);
    }

    int max = (int)pcpsd->pageCount;

    int32_t index;
    if ( ! c->Int32(result, &index) || (index < -1 || index > max) )
    {
        tcWrongRangeException(c, name, -1, max, result, pcpsd->pcpbd);
        return ret;
    }

    if ( index < 1 )
    {
        ret = index;
    }
    else
    {
        pCPropertySheetPage pcpsp = pcpsd->cppPages[index - 1];
        ret = pcpsp->pageID;
    }
    return ret;
}


bool setPropSheetHook(pCPropertySheetDialog pcpsd)
{
    PROPSHEETHOOKDATA *pshd = NULL;
    bool               result = false;
    uint32_t           threadID = 0;

    pshd = (PROPSHEETHOOKDATA *)LocalAlloc(LPTR, sizeof(PROPSHEETHOOKDATA));
    if ( pshd != NULL )
    {
        pshd->pcpsd = pcpsd;
        SetLastError(0);

        EnterCriticalSection(&ps_crit_sec);

        if ( CountPropSheetHooks < MAX_PROPSHEET_DIALOGS )
        {
            threadID = GetCurrentThreadId();
            pshd->hHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)PropSheetCBTProc, (HINSTANCE)NULL, GetCurrentThreadId());

            if ( pshd->hHook != NULL )
            {
                pshd->threadID = threadID;
                PropSheetHookData[CountPropSheetHooks] = pshd;
                CountPropSheetHooks++;
                result = true;
            }
        }

        LeaveCriticalSection(&ps_crit_sec);

        if ( result == true )
        {
            goto done_out;
        }
    }

    // Only here if we had an error, figure out which one.
    if ( pshd == NULL )
    {
        outOfMemoryException(pcpsd->dlgProcContext);
    }
    else if ( threadID == 0 )
    {
        userDefinedMsgException(pcpsd->dlgProcContext, TOO_MANY_PROPSHEET_DIALOGS, MAX_PROPSHEET_DIALOGS);
    }
    else
    {
        systemServiceExceptionCode(pcpsd->dlgProcContext, API_FAILED_MSG, "SetWindowsHookEx", GetLastError());
    }

    safeLocalFree(pshd);

done_out:
   return result;
}

/**
 * Searches for a match of the property sheet hook data struct.  The match is
 * keyed on the current thread ID and hwnd.
 *
 * This function is called twice.  The first time from PropCBTProc() where the
 * hwnd has not yet been placed into the struct; the match is by thread ID only.
 * If found, the struct is returned and PropSheetCBTProc() adds the hwnd to the
 * struct and unhooks the hook.
 *
 * The second time this function is called, it is from the PropSheetCallback()
 * function and this invocation passes in the hwnd.  If found, when hwnd is not
 * null, the struct is removed from the table and the caller frees its memory.
 *
 * The use of the hwnd and second invocation is because of the AeroWizard style
 * property sheet.  When the CSelf struct is placed in the GWLP_USEDATA index fo
 * the property sheet, it ends up getting corrupted.  Probably because the Aero
 * Wizard uses that index for its own purpose.  But, maybe because the Aero
 * Wizard dialog classs does not seem to be a WC_DIALOG but a NativeHWNDHost.
 *
 * @param hwnd  If hwnd is not null, and a match is found, we are done with the
 *              struct, remove it from the table.
 *
 * @return PROPSHEETHOOKDATA*
 *
 * @remarks.  The caller is responsible for freeing the stuct when the caller
 *            passes in a non-null hwnd.
 */
PROPSHEETHOOKDATA *getPropSheetHookData(HWND hwnd)
{
    register size_t i;
    DWORD id = GetCurrentThreadId();
    PROPSHEETHOOKDATA *pshd = NULL;

    EnterCriticalSection(&ps_crit_sec);

    for ( i = 0; i < CountPropSheetHooks; i++ )
    {
        if ( PropSheetHookData[i]->threadID == id && PropSheetHookData[i]->hPropSheet == hwnd )
        {
            pshd = PropSheetHookData[i];
            break;
        }
    }

    // It seems almost impossible that pshd could be null, but there is nothing
    // to do about it if it is.
    if ( pshd != NULL && hwnd != NULL )
    {
        // If the slot found is the last, just set it to null.
        if ( i == CountPropSheetHooks - 1 )
        {
            PropSheetHookData[i] = NULL;
        }
        else
        {
            // Since it is not the last, i + 1 must be valid memory.
            memcpy(&PropSheetHookData[i], &PropSheetHookData[i + 1], (CountPropSheetHooks - i - 1) * sizeof(PROPSHEETHOOKDATA *));
            PropSheetHookData[CountPropSheetHooks - 1] = NULL;
        }
        CountPropSheetHooks--;
    }

    LeaveCriticalSection(&ps_crit_sec);

    return pshd;
}


/**
 * Common code for getting the value of a Rexx argument where the argument can
 * be an Image object or a resource ID.
 *
 * @param c
 * @param image
 * @param argPos
 * @param isImage
 * @param type
 *
 * @return The handle to the image or the resource ID.
 */
intptr_t getImageOrID(RexxMethodContext *c, RexxObjectPtr self, RexxObjectPtr image, size_t argPos,
                     bool *isImage, uint8_t *type)
{
    intptr_t result = 0;

    if ( c->IsOfType(image, "IMAGE") )
    {
        POODIMAGE oodImage = rxGetOodImage(c, image, argPos);
        if ( oodImage != NULL )
        {
            *isImage = true;
            *type = (uint8_t)oodImage->type;
            result = (intptr_t)oodImage->hImage;
        }
    }
    else
    {
        int32_t id = oodResolveSymbolicID(c->threadContext, self, image, -1, argPos, true);

        if ( id == OOD_ID_EXCEPTION )
        {
            if ( ! isOutOfMemoryException(c->threadContext) )
            {
                // We want our own wording for the exception.
                c->ClearCondition();
                wrongArgValueException(c->threadContext, 1, "a postive numeric ID, valid symbolic ID, or .Image object", image);
            }
        }
        else
        {
            *isImage = false;
            result = id;
        }
    }
    return result;
}


/**
 * A generic function used to set the title, or header title, or header subtitle
 * text for a property sheet page.
 *
 * These text strings can be set before the property sheet is created.  But they
 * can also be changed after the property sheet begins execution.  Because of
 * this, the function is called from both a property sheet method context and a
 * property sheet page context.
 *
 * When replacing already existing text, the old text needs to be freed.  During
 * delDialog for a property sheet page, the existing text is freed.
 *
 * This is further complicated by the fact that if the page is in an aero
 * dialog, the header text has to be in Unicode, but the other page parts do not
 * apply. What we do, is always set the ANSI text in the struct.  Then, if we
 * know now we are an Aero page, we also set the Unicode text in the struct.
 * But, we won't always know here. So, when the PROPSHEETPAGE struct is filled
 * in, it becomes necessary to, maybe, fix up the Unicode string.
 *
 * @param c
 * @param pcpsp
 * @param text
 * @param part
 *
 * @return bool
 */
bool setPageText(RexxMethodContext *c, pCPropertySheetPage pcpsp, CSTRING text, pagePart_t part)
{
    char *t = (char *)LocalAlloc(LPTR, strlen(text) + 1);
    if ( t == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(t, text);

    switch ( part )
    {
        case headerSubtext :
        {
            safeLocalFree(pcpsp->headerSubTitle);
            pcpsp->headerSubTitle = t;
            break;
        }
        case headerText :
        {
            safeLocalFree(pcpsp->headerTitle);
            pcpsp->headerTitle = t;

            if ( pcpsp->isAeroWizardPage )
            {
                LPWSTR newText = ansi2unicode(text);
                if ( newText != NULL )
                {
                    safeLocalFree(pcpsp->headerTitleAero);
                    pcpsp->headerTitleAero = (char *)newText;
                }
            }

            break;
        }
        case pageText :
        {
            safeLocalFree(pcpsp->pageTitle);
            pcpsp->pageTitle = t;
            break;
        }
    }

    return true;
}


/**
 * Called from the property sheet callback function when signaled that the
 * property sheet dialog is about to be created.  At this point, the in-memory
 * dialog template can be accessed.
 *
 * This allows us to alter the template.  One use of this, could be to change
 * the font of the property sheet.
 *
 * Note this is just temp code that shows it can actually be done.  Because the
 * dialog template items are variable in size, we need to calculate the
 * beginning and end of memory used by the dialog items, change the font type
 * name and then move the dialog items to the correct place.
 *
 * The code here is just saved for future reference.  For a more practical use,
 * see the adjustResizablePropSheetTemplate() function.
 *
 * @param tmplate
 */
void adjustPropSheetTemplate(LPARAM tmplate, bool quickReturn)
{
    if ( quickReturn )
    {
        return;
    }

#if 0
    DLGTEMPLATE *pDlgTemplate;
    DLGTEMPLATEEX *pDlgTemplateEx;
    bool hasFontInfo = false;

    pDlgTemplateEx = (DLGTEMPLATEEX *)tmplate;
    if (pDlgTemplateEx->signature == 0xFFFF)
    {
        hasFontInfo = (pDlgTemplateEx->style & DS_SHELLFONT) || (pDlgTemplateEx->style & DS_SETFONT);

        printf("PropertySheet, extended template. Has font info=%d\n", hasFontInfo);
    }
    else
    {
        pDlgTemplate = (DLGTEMPLATE *)tmplate;

        hasFontInfo = (pDlgTemplate->style & DS_SHELLFONT) || (pDlgTemplate->style & DS_SETFONT);

        printf("PropertySheet, regular template. Has font info=%d\n", hasFontInfo);

        WORD *pStruct = (WORD *)tmplate;
        printf(" count items=%d x=%d y=%d cx=%d cy=%d menu=%d class=%d titleWord=%d\n",
               pStruct[4], pStruct[5], pStruct[6], pStruct[7], pStruct[8],
               pStruct[9], pStruct[10], pStruct[11]);


        WCHAR *wstr = (WCHAR *)(pStruct + 13);
        printf("  pointSize=%d ", pStruct[12]);
        wprintf(L"Font=%s\n", wstr);

        // Temp, change the font size and font name.  Really temp.  Font name
        // has to be exactly same number of characters as MS Shell Dlg.
        //
        // Arial Italic
        // Book Antiqua
        // Cooper Black
        // Poor Richard
        *(pStruct + 12) = 12;
        putUnicodeText((LPWORD)wstr, "Arial Italic");
    }
#endif
}


/**
 * Called from the resizable property sheet callback function when signaled that
 * the property sheet dialog is about to be created.  At this point, the
 * in-memory dialog template can be accessed.
 *
 * This allows us to alter the template.  We use this to add the WS_THICKFRAME
 * to the property sheet.  Other adjustments could be made.
 *
 * @param tmplate
 */
void adjustResizablePropSheetTemplate(LPARAM tmplate)
{
    DLGTEMPLATE *pDlgTemplate;
    DLGTEMPLATEEX *pDlgTemplateEx;

    pDlgTemplateEx = (DLGTEMPLATEEX *)tmplate;
    if (pDlgTemplateEx->signature == 0xFFFF)
    {
        pDlgTemplateEx->style |= WS_THICKFRAME;
    }
    else
    {
        pDlgTemplate = (DLGTEMPLATE *)tmplate;
        pDlgTemplate->style |= WS_THICKFRAME;
    }
}


/**
 * This is the subclass procedure for resizable property sheet dialogs.
 *
 * @param hDlg
 * @param msg
 * @param wParam
 * @param lParam
 * @param id
 * @param dwData
 *
 * @return LRESULT CALLBACK
 *
 * @remarks  We need to call initializeResizableDialog() at the time of the
 *           first WM_SHOWWINDOW.  If we call it prior to this, the button
 *           controls have not been positioned and once a resize is done, they
 *           appear in the wrong place.
 */
LRESULT CALLBACK ResizableSubclassProc(HWND hDlg, uint32_t msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)dwData;

    switch ( msg )
    {
        case WM_ENTERSIZEMOVE :
        {
            pcpsd->pcpbd->resizeInfo->inSizeOrMove = true;
            break;
        }

        case WM_SHOWWINDOW :
        {
            if ( pcpsd->isFirstShow )
            {
                pCPlainBaseDialog pcpbd = pcpsd->pcpbd;

                initializeResizableDialog(hDlg, pcpbd->dlgProcContext, pcpbd);
                pcpsd->isFirstShow = false;
            }

            break;
        }

        case WM_SIZE :
        {
            pCPlainBaseDialog     pcpbd = pcpsd->pcpbd;
            pResizeInfoDlg        prid  = pcpbd->resizeInfo;

            if ( prid->inSizeOrMove || wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED )
            {
                prid->isSizing = true;
                BOOL ret = resizeAndPosition(pcpbd, hDlg, LOWORD(lParam), HIWORD(lParam));

                HWND hTab = PropSheet_GetTabControl(hDlg);

                POINT p = { 0 };
                SIZE  s = { 0 };

                calcDisplayRect(hTab, hDlg, &p, &s);

                for ( size_t i = 0; i < pcpsd->pageCount; i++ )
                {
                    pCPropertySheetPage pcpsp = pcpsd->cppPages[i];
                    HWND hPage = pcpsp->hPage;
                    if ( hPage == NULL )
                    {
                        continue;
                    }

                    if ( IsWindowVisible(hPage) )
                    {
                        SetWindowPos(hPage, hTab, p.x, p.y, s.cx, s.cy, SWP_NOCOPYBITS | SWP_NOOWNERZORDER);
                        prid->redrawThis = hPage;
                    }
                    else
                    {
                        MoveWindow(hPage, p.x, p.y, s.cx, s.cy, FALSE);
                    }
                }
            }

            break;
        }

        case WM_EXITSIZEMOVE :
        {
            pCPlainBaseDialog pcpbd = pcpsd->pcpbd;
            pResizeInfoDlg    prid  = pcpbd->resizeInfo;

            if ( prid->isSizing && prid->redrawThis )
            {
                RedrawWindow(prid->redrawThis, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                prid->redrawThis = NULL;
            }
            if ( prid->isSizing && prid->sizeEndedMeth != NULL)
            {
                notifyExitSizeMove(pcpbd);
            }

            prid->isSizing = false;
            prid->inSizeOrMove = false;

            break;
        }

        case WM_GETMINMAXINFO :
        {
            pResizeInfoDlg  prid = pcpsd->pcpbd->resizeInfo;
            MINMAXINFO     *pmmi = (MINMAXINFO *)lParam;

            if ( prid->haveMinSize )
            {
                pmmi->ptMinTrackSize.x = prid->minSize.cx;
                pmmi->ptMinTrackSize.y = prid->minSize.cy;
            }

            if ( prid->haveMaxSize )
            {
                pmmi->ptMaxTrackSize.x = prid->maxSize.cx;
                pmmi->ptMaxTrackSize.y = prid->maxSize.cy;
            }

            return TRUE;
        }

        case WM_NCDESTROY :
            /* The window is being destroyed, remove the subclass. The dwData
             * struct will be cleaned up normally since it is the CSelf struct
             * of the property sheet.
             */
            RemoveWindowSubclass(hDlg, ResizableSubclassProc, id);
            break;

        default :
            break;
    }

    return DefSubclassProc(hDlg, msg, wParam, lParam);
}

void subclassResizablePropertySheet(HWND hPropSheet, pCPropertySheetDialog pcpsd)
{
    SetWindowSubclass(hPropSheet, ResizableSubclassProc, 100, (DWORD_PTR)pcpsd);
}

/**
 * Called from the property sheet callback function when signaled that the
 * property sheet dialog is being initialized.  Performs the initialization
 * normally done in the window loop thread function and the execute() method for
 * normal ooDialog dialogs.
 *
 * In addition, if this is a resizable property sheet, we subclass the property
 * sheet so we can handle the resizing.
 *
 * @param hPropSheet  Window handle of the property sheet.
 */
static void initializePropSheet(HWND hPropSheet)
{
    PROPSHEETHOOKDATA *pshd = getPropSheetHookData(hPropSheet);

    if ( pshd != NULL )
    {
        pCPropertySheetDialog  pcpsd = pshd->pcpsd;
        RexxThreadContext     *c     = pcpsd->dlgProcContext;
        pCPlainBaseDialog      pcpbd = pcpsd->pcpbd;

        LocalFree(pshd);

        pcpbd->hDlg = hPropSheet;

        // Not sure about using the whole top dialog thing for property sheets.
        // There is no pcpbd->previous set.
        installNecessaryStuff(pcpbd, NULL);

        pcpbd->hDlg = hPropSheet;
        pcpbd->isActive = true;
        pcpbd->childDlg[0] = hPropSheet;

        setDlgHandle(pcpbd);
        setFontAttrib(c, pcpbd);

        pcpbd->onTheTop = true;
        pcpbd->dlgProcThreadID = GetCurrentThreadId();

        // Do we have a modal dialog?  TODO this check is worthless because
        // there is no pcpbd->previous set.
        checkModal((pCPlainBaseDialog)pcpbd->previous, pcpsd->modeless);

        if ( pcpbd->isResizableDlg )
        {
            SetWindowSubclass(hPropSheet, ResizableSubclassProc, pcpbd->dlgID, (DWORD_PTR)pcpsd);
        }

        c->SendMessage0(pcpsd->rexxSelf, "INITDIALOG");
    }
}


/**
 * Called from WM_INITDIALOG for a property sheet page.  Performs the
 * initialization for the dialog that is usually done in the window loop thread
 * furnction and the execute() method for regular ooDialog dialogs.
 *
 * @param hPage
 * @param pcpsp
 */
static void initializePropSheetPage(HWND hPage, pCPropertySheetPage pcpsp)
{
    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    RexxThreadContext *c = pcpsp->dlgProcContext;

    setWindowPtr(hPage, GWLP_USERDATA, (LONG_PTR)pcpsp);

    pcpsp->hPage = hPage;
    pcpsp->activated = true;

    pcpbd->hDlg = hPage;
    pcpbd->isActive = true;
    pcpbd->childDlg[0] = hPage;

    setDlgHandle(pcpbd);

    if ( pcpbd->isResizableDlg )
    {
        initializeResizableDialog(hPage, pcpbd->dlgProcContext, pcpbd);
    }

    if ( pcpsp->pageType == oodResPSPDialog )
    {
        setFontAttrib(c, pcpbd);

        if ( pcpbd->autoDetect )
        {
            if ( doDataAutoDetection(c, pcpbd) == OOD_MEMORY_ERR )
            {
                return;
            }
        }
    }

    c->SendMessage0(pcpsp->rexxSelf, "Execute");
}

/**
 * Attempts to match the window being created in a CBTProc hook function as a
 * property sheet window
 *
 * @param cs  Pointer to the window create struct.
 *
 * @return True for a match, otherwise false.
 *
 * @remarks  Tough to be sure we get the right window in the CBTProc function.
 *           But, for our use, I think it is impossible to create two windows in
 *           the same thread without the property sheet window being first.
 *
 *           Trying to match the Windows styles for all different types of
 *           property sheets, including AeroWizards, is not working well.  For
 *           now, just assume if it is a dialog or a NativeHWNDHost on Vista or
 *           later, that we are okay.
 */
inline bool isPropSheetMatch(LPCREATESTRUCT cs)
{
#if 0
    uint32_t styleMask = WS_POPUP | WS_CAPTION | WS_SYSMENU | 0x000000c4;
    uint32_t exStyleMask = WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME;

    printf("Matching styleMask=0x%08x exStyleMask=0x%08x\n", styleMask, exStyleMask);
    printf("Actual       style=0x%08x     exStyle=0x%08x\n", cs->style, cs->dwExStyle);
#endif

    if ( cs->lpszClass == WC_DIALOG || (_isAtLeastVista() && strcmp(cs->lpszClass, "NativeHWNDHost") == 0) )
    {
        return true;
    }
    return false;
}

/**
 * Forces a property sheet dialog to end.  Setting each page's abort flag
 * prevents the page from canceling the close.  We then programmatically press
 * the Cancel key.
 *
 * This was originally intended only for modal propert sheets.  Usually it is
 * not needed with modeless property sheets as DestroyWindow() works fine,
 * however there is no reason it can not be used with modeless property sheet
 * dialogs.
 *
 * @param pcpsd
 * @param hDlg
 * @param t
 */
void abortPropertySheet(pCPropertySheetDialog pcpsd, HWND hDlg, DlgProcErrType t)
{
    uint32_t count = pcpsd->pageCount;
    for ( uint32_t i = 0; i < count; i++ )
    {
        pcpsd->cppPages[i]->abort = true;
    }

    PropSheet_PressButton(hDlg, PSBTN_CANCEL);
}


void abortPropertySheetPage(pCPropertySheetPage page, HWND hDlg, DlgProcErrType t)
{
    RexxThreadContext     *c     = page->dlgProcContext;
    pCPropertySheetDialog  pcpsd = (pCPropertySheetDialog)page->cppPropSheet;
    uint32_t               count = pcpsd->pageCount;

    for ( uint32_t i = 0; i < count; i++ )
    {
        pCPropertySheetPage pcpsp = pcpsd->cppPages[i];
        pcpsp->abort = true;

        if ( pcpsp->activated )
        {
            ensureFinished(pcpsp->pcpbd, c, TheTrueObj);
        }
    }

    PropSheet_PressButton(pcpsd->hDlg, PSBTN_CANCEL);
}


/**
 * Filter for property sheet notification messages.
 *
 * @param uMsg    The windowm message id.
 * @param lParam  The LPARAM argument for the message.
 *
 * @return Return true if the message is a property sheet notification message,
 *         otherwise false.
 */
inline bool isPSMsg(uint32_t uMsg, LPARAM lParam)
{
    uint32_t code;
    return (uMsg == PSM_QUERYSIBLINGS) ||
           (uMsg == WM_NOTIFY && ((code = ((NMHDR *)lParam)->code) >= PSN_LAST  && code <= PSN_FIRST));
}


/**
 * The PSN_SETACTIVE, PSN_WIZBACK, and PSN_WIZNEXT notifications all use
 * identical code.
 *
 * The Rexx programmer returns 0 to allow the page to become active (or to go to
 * the previous page for WIZBACK, or to go to the next page for WIZNEXT,) -1 to
 * prevent a page change, and the page index, or page ID, to go to a specific
 * page.  The Windows property sheet actually uses the page ID.
 *
 * The Rexx programmer can get the proper page ID by using the indexToID()
 * method.  That method returns a .Pointer object.  Otherwise, it seems simplier
 * for the Rexx programmet to simply use the page index and we'll look it up
 * here.  The short of it is, the Rexx progammer can either reply using the page
 * ID, in which case result is a Pointer object.  Or they can simple use the
 * page index.
 *
 * @param c
 * @param pcpsd
 * @param hPage
 * @param name
 */
void doSetActiveCommon(RexxThreadContext *c, pCPropertySheetPage pcpsp, HWND hPage, CSTRING name)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;
    intptr_t reply = 0;

    RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, name, pcpsd->rexxSelf);

    if ( goodReply(c, pcpsd->pcpbd, result, name) )
    {
        reply = getSetActiveValue(c, pcpsd, result, name);
        tcCheckForCondition(c, pcpsd->pcpbd);
    }
    setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
}


/**
 * Handles the PSN_QUERYINITIALFOCUS notification.
 *
 * The Rexx programmer returns 0 to set the focus to the default control and
 * returns the dialog control resource ID to set that focus to that control.
 * The ID can be numeric or symbolic.
 *
 * The Windows return is 0 to set the focus to the default control and the
 * handle of the dialog control to set the focuse to some control other than the
 * default
 *
 * @param c
 * @param pcpsp
 * @param lParam
 *
 * @remarks  lParam of the PSHNOTIFY struct contains the HWND of the dialog
 *           control that will receive the focus by default.  We use that to get
 *           the resource ID of that control and use that ID as the first
 *           argument sent to Rexx.
 *
 */
void doQueryInitialFocus(RexxThreadContext *c, pCPropertySheetPage pcpsp, LPARAM lParam)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;

    int32_t id = GetDlgCtrlID((HWND)((LPPSHNOTIFY)lParam)->lParam);

    RexxObjectPtr result = c->SendMessage2(pcpsp->rexxSelf, QUERYINITIALFOCUS_MSG, c->Int32(id), pcpsd->rexxSelf);

    HWND   hPage = pcpsp->hPage;
    LPARAM reply = 0;

    if ( goodReply(c, pcpsd->pcpbd, result, QUERYINITIALFOCUS_MSG) )
    {
        // We need 0 to be okay, but not -1.
        id = oodResolveSymbolicID(c, pcpsp->rexxSelf, result, -1, 1, false);
        if ( id == OOD_ID_EXCEPTION || id == -1 )
        {
            if ( isOutOfMemoryException(c) )
            {
                tcMemoryErr(c, pcpsd->pcpbd);
            }
            else
            {
                tcInvalidReturnListException(c, QUERYINITIALFOCUS_MSG, "0, or a valid resource ID", result, pcpsd->pcpbd);
            }
        }
        else if ( id != 0 )
        {
            reply = (LPARAM)GetDlgItem(hPage, id);
        }
    }
    setWindowPtr(hPage, DWLP_MSGRESULT, reply);
}

/**
 * Handles the PSN_WIZFINISH notification.
 *
 * The Rexx programmer returns 0 to allow the wizard to finish, -1 to prevent
 * the wizard from finishing, or the resource ID of a control on the wizard
 * page.  Sending the resource ID prevents the wizard from finishing and sets
 * the focus on the page to that dialog control.
 *
 * The Windows return is 0 to allow the wizard to finish, true to prevent the
 * wizard from finishing, or the window handle of a dialog control on the page
 * to prevent the finish and set that page as the current page.
 *
 * @param c
 * @param pcpsd
 * @param result
 * @param hPage
 * @param name
 *
 * @remarks
 */
void doWizFinish(RexxThreadContext *c, pCPropertySheetPage pcpsp)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;

    RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, WIZFINISH_MSG, pcpsd->rexxSelf);

    HWND   hPage = pcpsp->hPage;
    LPARAM reply = 0;

    if ( goodReply(c, pcpsd->pcpbd, result, WIZFINISH_MSG) )
    {
        int32_t id = oodResolveSymbolicID(c, pcpsp->rexxSelf, result, -1, 1, false);
        if ( id == OOD_ID_EXCEPTION )
        {
            if ( isOutOfMemoryException(c) )
            {
                tcMemoryErr(c, pcpsd->pcpbd);
            }
            else
            {
                tcInvalidReturnListException(c, WIZFINISH_MSG, "-1, 0, or a valid resource ID", result, pcpsd->pcpbd);
            }
        }
        else if ( id != 0 )
        {
            reply = (LPARAM)GetDlgItem(hPage, id);
        }
    }
    setWindowPtr(hPage, DWLP_MSGRESULT, reply);
}


int queryFromSibling(RexxThreadContext *c, pCPropertySheetPage pcpsp, WPARAM wParam, LPARAM lParam)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;
    int                   reply = 0;

    RexxArrayObject args   = c->ArrayOfThree((RexxObjectPtr)wParam, (RexxObjectPtr)lParam, pcpsd->rexxSelf);
    RexxObjectPtr   result = c->SendMessage(pcpsp->rexxSelf, QUERYFROMSIBLING_MSG, args);

    if ( goodReply(c, pcpsd->pcpbd, result, QUERYFROMSIBLING_MSG) )
    {
        if ( ! c->Int32(result, &reply) )
        {
            tcWrongRangeException(c, QUERYFROMSIBLING_MSG, MININT, MAXINT, result, pcpsd->pcpbd);
        }
    }
    return reply;
}


/**
 * Handler for all the property sheet notification messages and the special case
 * PSM_QUERYSIBLINGS message.
 *
 * Rather than use a connectPropertySheetEvent() method, the property sheet
 * dialog handles every notification.  The dialog class supplies the correct
 * default implementation for each notification.  The Rexx programmer over-rides
 * the default implementation for any notification he wants to handle.
 *
 * The special PSM_QUERYSIBLINGS message is sent to a property sheet and the
 * property sheet then forwards it to each page.  So, the page recieving the
 * message is very similar to receiving a notification.  The sequence is
 * initiated through the PropertySheet::querySiblings() method and replied to
 * through the PropertySheetPage::queryFromSibling() method.
 *
 * @param pcpsp
 * @param pcpbd
 * @param wParam
 * @param lParam
 */
static int doPSMessage(pCPropertySheetPage pcpsp, pCPlainBaseDialog pcpbd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;
    HWND hPropSheet = pcpsd->hDlg;
    HWND hPage = pcpsp->hPage;
    RexxThreadContext *c = pcpbd->dlgProcContext;

    if ( msg == PSM_QUERYSIBLINGS )
    {
        return queryFromSibling(c, pcpsp, wParam, lParam);
    }

    switch ( ((NMHDR *)lParam)->code )
    {
        case PSN_APPLY :
        {
            LPPSHNOTIFY lppsn        = (LPPSHNOTIFY)lParam;
            uint32_t reply           = PSNRET_NOERROR;
            RexxObjectPtr isOkButton = lppsn->lParam ? TheTrueObj : TheFalseObj;

            // We want to get the Rexx property sheet page that had the focus when the Apply button was pressed.
            int32_t i = PropSheet_HwndToIndex(pcpsd->hDlg, PropSheet_GetCurrentPageHwnd(pcpsd->hDlg));
            pCPropertySheetPage current = pcpsd->cppPages[i];

            // Index to Rexx is one-based.
            i++;

            RexxArrayObject args = c->ArrayOfFour(isOkButton, c->Int32(i), current->rexxSelf, pcpsd->rexxSelf);

            RexxObjectPtr result = c->SendMessage(pcpsp->rexxSelf, APPLY_MSG, args);

            if ( goodReply(c, pcpsd->pcpbd, result, APPLY_MSG) )
            {
                if ( ! c->UnsignedInt32(result, &reply) || ! (reply >= PSNRET_NOERROR && reply <= PSNRET_INVALID_NOCHANGEPAGE) )
                {
                    tcInvalidReturnListException(c, APPLY_MSG, VALID_PSNRET_LIST, result, pcpsd->pcpbd);
                }
            }

            setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
            break;
        }

        case PSN_GETOBJECT :
        {
            // I have not been able to produce this notification, so this should essentially be a no-op
            if ( pcpsp->wantGetObject )
            {
                RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, GETOBJECT_MSG, pcpsd->rexxSelf);
            }

            break;
        }

        case PSN_HELP :
        {
            RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, HELP_MSG, pcpsd->rexxSelf);
            goodReply(c, pcpsd->pcpbd, result, HELP_MSG);
            break;
        }

        case PSN_KILLACTIVE :
        {
            // Reply TRUE to Windows to *cancel* the page change.  The Rexx
            // programmer should send .false to cancel the change.
            long reply = FALSE;

            RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, KILLACTIVE_MSG, pcpsd->rexxSelf);

            if ( goodReply(c, pcpsd->pcpbd, result, KILLACTIVE_MSG) )
            {
                if ( result != TheTrueObj && result != TheFalseObj )
                {
                    tcInvalidReturnListException(c, KILLACTIVE_MSG, "true or false", result, pcpsd->pcpbd);
                }
                else if ( result == TheFalseObj )
                {
                    reply = TRUE;
                }
            }

            setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
            break;
        }

        case PSN_QUERYCANCEL :
        {
            // Send TRUE to *prevent* the dialog from closing.  The Rexx
            // programmer should send .false to disallow the dialog from
            // closing.
            long reply = FALSE;

            if ( ! pcpsp->abort )
            {
                RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, QUERYCANCEL_MSG, pcpsd->rexxSelf);

                if ( goodReply(c, pcpsd->pcpbd, result, QUERYCANCEL_MSG) )
                {
                    if ( result != TheTrueObj && result != TheFalseObj )
                    {
                        tcInvalidReturnListException(c, QUERYCANCEL_MSG, "true or false", result, pcpsd->pcpbd);
                    }
                    else if ( result == TheFalseObj )
                    {
                        reply = TRUE;
                    }
                }
            }

            setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
            break;
        }

        case PSN_RESET :
        {
            LPPSHNOTIFY lppsn            = (LPPSHNOTIFY)lParam;
            RexxObjectPtr isCancelButton = lppsn->lParam ? TheFalseObj : TheTrueObj;

            RexxObjectPtr result = c->SendMessage2(pcpsp->rexxSelf, RESET_MSG, isCancelButton, pcpsd->rexxSelf);
            goodReply(c, pcpsd->pcpbd, result, RESET_MSG);
            break;
        }

        case PSN_TRANSLATEACCELERATOR :
        {
            if ( pcpsp->wantAccelerators )
            {
                LPMSG    pMsg  = (LPMSG)((LPPSHNOTIFY)lParam)->lParam;
                uint32_t reply = PSNRET_NOERROR;

                RexxArrayObject args = getTranslateAccelatorArgs(c, pMsg->message, pMsg->wParam, pMsg->lParam,
                                                                 pcpsd->rexxSelf);
                RexxObjectPtr result = c->SendMessage(pcpsp->rexxSelf, TRANSLATEACCELERATOR_MSG, args);

                if ( goodReply(c, pcpsd->pcpbd, result, TRANSLATEACCELERATOR_MSG) )
                {
                    if ( ! c->UnsignedInt32(result, &reply) ||
                         (reply != PSNRET_NOERROR && reply != PSNRET_MESSAGEHANDLED) )
                    {
                        tcInvalidReturnListException(c, TRANSLATEACCELERATOR_MSG, VALID_PSNRET_MSG_LIST, result,
                                                     pcpsd->pcpbd);
                    }
                }
                setWindowPtr(hPage, DWLP_MSGRESULT, reply);
            }
            break;
        }

        case PSN_SETACTIVE :
            doSetActiveCommon(c, pcpsp, hPage, SETACTIVE_MSG);
            break;

        case PSN_WIZBACK :
            doSetActiveCommon(c, pcpsp, hPage, WIZBACK_MSG);
            break;

        case PSN_WIZNEXT :
            doSetActiveCommon(c, pcpsp, hPage, WIZNEXT_MSG);
            break;

        case PSN_WIZFINISH :
            doWizFinish(c, pcpsp);
            break;

        case PSN_QUERYINITIALFOCUS :
            doQueryInitialFocus(c, pcpsp, lParam);
            break;

        default :
            break;
    }
    return TRUE;
}

/**
 * The thread and message pump function for a modeless property sheet.
 *
 *
 * @param arg
 *
 * @return DWORD WINAPI
 */
DWORD WINAPI PropSheetLoopThread(void *arg)
{
    ULONG ret;
    PropSheetThreadArgs *args = (PropSheetThreadArgs *)arg;

    pCPropertySheetDialog pcpsd = args->pcpsd;
    pCPlainBaseDialog pcpbd     = pcpsd->pcpbd;
    PROPSHEETHEADER *psh        = args->psh;
    bool *release               = args->release;

    // We need a thread context, so if we can not attach, we bail.
    RexxThreadContext *c = NULL;
    if ( ! pcpbd->interpreter->AttachThread(&c) )
    {
        *release = true;
        goto done_out;
    }

    RexxSetProcessMessages(FALSE);
    pcpbd->dlgProcContext = c;
    pcpbd->dlgProcThreadID = GetCurrentThreadId();
    assignPSDThreadContext(pcpsd, c, pcpbd->dlgProcThreadID);

    HWND hPropSheet = NULL;

    if ( setPropSheetHook(pcpsd) )
    {
        hPropSheet = (HWND)PropertySheet(psh);
    }
    *release = true;

    if ( hPropSheet == NULL )
    {
        goto done_out;
    }

    c->SendMessage1(pcpbd->rexxSelf, "START", c->String("WAITFORDIALOG"));

    MSG msg;
    BOOL result;

    while ( (result = GetMessage(&msg, NULL, 0, 0)) != 0 && pcpbd->dlgAllocated )
    {
        if ( result == -1 )
        {
            break;
        }
        if ( ! PropSheet_IsDialogMessage(hPropSheet, &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if ( PropSheet_GetCurrentPageHwnd(hPropSheet) == NULL )
        {
            pcpsd->getResultValue = PropSheet_GetResult(hPropSheet);
            break;
        }
    }

done_out:

    // Need to synchronize here, otherwise dlgAllocated may still be true, but
    // delDialog() is already running.
    EnterCriticalSection(&crit_sec);
    if ( pcpbd->dlgAllocated )
    {
        ret = delDialog(pcpbd, pcpbd->dlgProcContext);
        pcpbd->hDlgProcThread = NULL;

        c->SendMessage0(pcpbd->rexxSelf, "LEAVING");
    }
    LeaveCriticalSection(&crit_sec);

    if ( hPropSheet != NULL )
    {
        ensureFinished(pcpbd, c, TheFalseObj);
    }

    if ( c != NULL )
    {
        c->DetachThread();
        pcpbd->dlgProcContext = NULL;
        pcpsd->dlgProcContext = NULL;
    }

    safeLocalFree((void *)psh->ppsp);
    safeLocalFree(psh);
    return ret;
}


/**
 * The dialog procedure for control dialogs used by the PropertySheetDialog
 * property sheet page dislogs.
 *
 * These are 'nested' dialogs, or dialogs within a top-level dialog.  For the
 * most part, the procedure is exactly the same as for top-level dialogs.  See
 * the remarks for the differences
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT CALLBACK
 *
 * @remarks  PropertySheetPage dialogs are not created by the ooDialog
 *           framework, but rather by the OS.  So, in WM_INITDIALOG we do the
 *           Rexx dialog set up normally done in the window loop thread and the
 *           execute() method.
 *
 *           In WM_DESTROY we do the final steps / clean up normally done in the
 *           execute() method.
 */
LRESULT CALLBACK RexxPropertySheetPageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        PROPSHEETPAGE *p = (PROPSHEETPAGE *)lParam;
        pCPropertySheetPage pcpsp = (pCPropertySheetPage)p->lParam;

        if ( pcpsp == NULL )
        {
            // Theoretically impossible.  But ... if it should happen, abort.
            return endDialogPremature(NULL, hDlg, NoPCPSPpased);
        }

        pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
        pcpbd->hDlg = hDlg;

        initializePropSheetPage(hDlg, pcpsp);

        if ( pcpbd->isCustomDrawDlg && pcpbd->idsNotChecked )
        {
            // We don't care what the outcome of this is, customDrawCheckIDs
            // will take care of aborting this dialog if the IDs are bad.
            customDrawCheckIDs(pcpbd);
        }

        return TRUE;
    }

    pCPropertySheetPage pcpsp = (pCPropertySheetPage)getWindowPtr(hDlg, GWLP_USERDATA);
    if ( pcpsp == NULL )
    {
        // A number of messages arrive before WM_INITDIALOG, we just ignore them.
        return FALSE;
    }

    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    if ( pcpbd->dlgProcContext == NULL )
    {
        if ( ! pcpbd->isActive )
        {
            return FALSE;
        }

        // Once again, theoretically impossible ...
        return endDialogPremature(pcpbd, hDlg, NoThreadContext);
    }

    if ( uMsg == WM_DESTROY )
    {
        pcpsp->dlgProcContext->SendMessage0(pcpsp->rexxSelf, "ENDEXECUTE");
        return TRUE;
    }

    // We first deal with resizable stuff, then handle the rest with the normal
    // ooDialog process.
    if ( pcpbd->isResizableDlg )
    {
        MsgReplyType resizingReply = handleResizing(hDlg, uMsg, wParam, lParam, pcpbd);
        if ( resizingReply != ContinueProcessing )
        {
            return (resizingReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    bool msgEnabled = IsWindowEnabled(hDlg) ? true : false;

    // Do not search message table for WM_PAINT to improve redraw.
    if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
    {
        if ( isPSMsg(uMsg, lParam) )
        {
            return doPSMessage(pcpsp, pcpbd, uMsg, wParam, lParam);
        }

        MsgReplyType searchReply = searchMessageTables(uMsg, wParam, lParam, pcpbd);
        if ( searchReply != ContinueProcessing )
        {
            // Note pre 4.0.1, we always returned FALSE, (pass on to the system
            // to process.) But, post 4.0.1 we sometimes reply TRUE, the message
            // has been handled.
            return (searchReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    if ( uMsg >= WM_USER_REXX_FIRST && uMsg <= WM_USER_REXX_LAST )
    {
        return handleWmUser(pcpbd, hDlg, uMsg, wParam, lParam, true);
    }

    switch ( uMsg )
    {
        case WM_PAINT:
            return drawBackgroundBmp(pcpbd, hDlg);

        case WM_DRAWITEM:
            return drawBitmapButton(pcpbd, lParam, msgEnabled);

        case WM_CTLCOLORDLG:
            return handleDlgColor(pcpbd);

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
            return handleCtlColor(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            return paletteMessage(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_COMMAND:
            return handleWmCommand(pcpbd, hDlg, wParam, lParam, true);

        default:
            break;
    }

    return FALSE;
}


LRESULT CALLBACK PropSheetCBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PROPSHEETHOOKDATA *pshd = getPropSheetHookData(NULL);
    if ( pshd == NULL )
    {
        // Without the hook handle, there is nothing that can be done.
        return 0;
    }

    HHOOK hHook = pshd->hHook;

    if ( nCode == HCBT_CREATEWND )
    {
        LPCREATESTRUCT cs = (LPCREATESTRUCT)((CBT_CREATEWND *)lParam)->lpcs;


        if ( isPropSheetMatch(cs) )
        {
            pshd->pcpsd->hDlg = (HWND)wParam;
            pshd->hPropSheet = (HWND)wParam;
            UnhookWindowsHookEx(hHook);
        }
        else
        {
            printf("PropSheetCBTProc() DID NOT MATCH !! hwnd=%p className=%s name=%s style=0x%08x exStyle=0x%08x\n",
                    (void *)wParam, cs->lpszClass, cs->lpszName, cs->style, cs->dwExStyle);
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}


/**
 * An implementation of the PropSheetProc callback.  The system calls this
 * function when the property sheet is being created and initialized.
 *
 * There are only 3 uMsg messages sent.
 *
 * PSCB_PRECREATE Sent before the PropertySheet is created. lParam is a pointer
 * to the dialog template.  hwndPropSheet is null.  This callback could be used
 * to change something in the dialog template.  Don't see any good use for it at
 * this point.
 *
 * PSCB_INITIALIZED  Sent when the PropertySheet is initializing.  We use this
 * callback to do the Rexx dialog initialization that is usually done in the
 * thread creation function and the execute() method.
 *
 * PSCB_BUTTONPRESSED  Sent when a button in the property sheet is pressed.
 * Seems as though it would be useful.  But, it actually does not provide any
 * thing not already provided by the regular notification messages.  Not used at
 * this point.
 *
 * @param hwndPropSheet
 * @param uMsg
 * @param lParam
 *
 * @return void
 *
 * @remarks  Typical implementation would use a switch statement.  Since only 1
 *           uMsg is of interest, I've removed the switch.
 */
void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
    if ( uMsg == PSCB_INITIALIZED )
    {
        initializePropSheet(hwndPropSheet);
    }
    else if ( uMsg == PSCB_PRECREATE )
    {
        adjustPropSheetTemplate(lParam, true);
    }
}


/**
 * An implementation of the PropSheetProc callback.  The system calls this
 * function when the property sheet is being created and initialized.
 *
 * We use this callback function for resizable property sheet dialogs.  At the
 * PSCB_PRECREATE callback, we need to add the WS_THICKFRAME style to the dialog
 * template.  Adding the style flag anywhere other than in the dialog template
 * does not work.  However, the only information during this callback is the
 * template struct, there is no way to know if the dialog is meant to be
 * resizable or not.  However, the code that assigns callback function does know
 * if it should be resizable or not.  So, we simply use two nearly identical
 * callbacks.  One adds the right style to the template, the other does not.
 *
 * Other doc is in the PropSheetCallback header.
 *
 * @param hwndPropSheet
 * @param uMsg
 * @param lParam
 *
 * @return void
 */
void CALLBACK ResizablePropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
    if ( uMsg == PSCB_INITIALIZED )
    {
        initializePropSheet(hwndPropSheet);
    }
    else if ( uMsg == PSCB_PRECREATE )
    {
        adjustResizablePropSheetTemplate(lParam);
    }
}


/**
 * The property sheet page callback function.  It is called with 3 different
 * messages:
 *
 * PSPCB_ADDREF   Called when a page is being created.
 *
 * PSPCB_CREATE   Called when the dialog box for the page is being created.
 *
 * PSPCP_RELEASE  Called when the page is being destroyed
 *
 * We use PSPCB_CREATE to call back in to Rexx by invoking the pageCreate()
 * method of the PropertySheetPage.  The default implementation of pageCreate()
 * simply returns true.
 *
 * The programmer can over-ride pageCreate(), for whatever reason.  Returning 0
 * prevents the page dialog from being created.  Although, I'm not sure what use
 * the Rexx programmer would make of that.
 *
 * The other messages are ignored at this time.  (And I'm not sure if the one
 * callback is that useful for the Rexx programmer.)
 *
 *
 * @param hwnd  Reserved by MS, always NULL.
 * @param uMsg
 * @param ppsp
 *
 * @return UINT CALLBACK
 *
 * @remarks  The property sheet page thread context is assigned during the
 *           creation of the property sheet, on the thread where the property
 *           sheet is created.  Since the property sheet is created before any
 *           of the pages are created, the dlgProcContext for the page can not
 *           be null.
 */
uint32_t CALLBACK PropSheetPageCallBack(HWND hwnd, uint32_t msg, LPPROPSHEETPAGE ppsp)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)ppsp->lParam;

    if ( msg == PSPCB_CREATE )
    {
        RexxThreadContext *c = pcpsp->dlgProcContext;
        uint32_t reply = TRUE;

        if ( c != NULL )
        {
            pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;

            RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, PAGECREATE_MSG, pcpsd->rexxSelf);

            if ( goodReply(c, pcpsd->pcpbd, result, PAGECREATE_MSG) )
            {
                if ( result != TheTrueObj && result != TheFalseObj )
                {
                   tcInvalidReturnListException(c, PAGECREATE_MSG, "true or false", result, pcpsd->pcpbd);
                }
                else
                {
                    reply = (result == TheTrueObj ? 1 : 0);
                }
            }
        }
        return reply;
    }
    return 0;
}


/**
 *  Methods for the .PropertySheetDialog class.
 */
#define PROPERTYSHEETDIALOG_CLASS  "PropertySheetDialog"


bool setCaption(RexxMethodContext *c, pCPropertySheetDialog pcpsd, CSTRING text)
{
    pcpsd->caption = (char *)LocalAlloc(LPTR, strlen(text) + 1);
    if ( pcpsd->caption == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(pcpsd->caption, text);
    c->SetObjectVariable("CAPTION", c->String(text));
    return true;
}


static bool parsePropSheetOpts(RexxMethodContext *c, pCPropertySheetDialog pcpsd, CSTRING options)
{
    uint32_t opts = PSH_PROPSHEETPAGE | PSH_USECALLBACK;

    if ( options == NULL )
    {
        pcpsd->isNotWizard = true;
        goto done_out;
    }

    if ( StrStrI(options, "USEPAGELANG") != NULL ) opts |= PSH_USEPAGELANG;
    if ( StrStrI(options, "RTLREADING ") != NULL ) opts |= PSH_RTLREADING;

    if ( StrStrI(options, "AEROWIZARD") != NULL )
    {
        if ( ! requiredComCtl32Version(c, COMCTL32_6_0, "the AEROWIZARD style") ||
             ! requiredOS(c, Vista_OS, "the AEROWIZARD style", "Vista") )
        {
            goto err_out;
        }

        pcpsd->isAeroWiz = true;
        opts |= PSH_AEROWIZARD | PSH_WIZARD;

        if ( StrStrI(options, "RESIZABLE") != NULL ) opts |= PSH_RESIZABLE;
        if ( StrStrI(options, "NOMARGIN")  != NULL ) opts |= PSH_NOMARGIN;
    }
    else if ( StrStrI(options, "WIZARD97") != NULL )
    {
        pcpsd->isWiz97 = true;
        opts |= PSH_WIZARD97;
    }
    else if ( StrStrI(options, "WIZARDLITE") != NULL )
    {
        pcpsd->isWizLite = true;
        opts |= PSH_WIZARD_LITE;
    }
    else
    {
        pcpsd->isNotWizard = true;
        if ( StrStrI(options, "NOCONTEXTHELP") != NULL )
        {
            opts |= PSH_NOCONTEXTHELP;
        }
    }

    if ( ! pcpsd->isAeroWiz )
    {
        if ( StrStrI(options, "HASHELP")    != NULL ) opts |= PSH_HASHELP;
        if ( StrStrI(options, "NOAPPLYNOW") != NULL ) opts |= PSH_NOAPPLYNOW;
        if ( StrStrI(options, "PROPTITLE")  != NULL ) opts |= PSH_PROPTITLE;
    }

    if ( (pcpsd->isAeroWiz || pcpsd->isWiz97) && StrStrI(options, "WIZARDHASFINISH") != NULL )
    {
        opts |= PSH_WIZARDHASFINISH;
    }

    if ( (pcpsd->isWiz97 || pcpsd->isWizLite) && StrStrI(options, "WIZARDCONTEXTHELP") != NULL )
    {
        opts |= PSH_WIZARDCONTEXTHELP;
    }

done_out:
    pcpsd->propSheetFlags = opts;

    return true;

err_out:
    return false;
}

/**
 * Checks that the Rexx owner dialog passed in to execute() is valid.
 *
 * @param c       Method context we are operating in.
 * @param owner   Rexx object to check.
 * @param argPos  Argument position of the Rexx object.
 *
 * @return Windown handle of the dialog, if okay, otherise NULL.  If NULL is
 *         returned, an exceptions has been raised.
 */
static HWND checkPropSheetOwner(RexxMethodContext *c, RexxObjectPtr owner, size_t argPos)
{
    pCPlainBaseDialog pcpbdOwner = requiredDlgCSelf(c, owner, oodPlainBaseDialog, argPos, NULL);
    if ( pcpbdOwner == NULL )
    {
        return NULL;
    }

    if ( pcpbdOwner->isControlDlg || pcpbdOwner->isPageDlg )
    {
        userDefinedMsgException(c, argPos, "The owner dialog of a PropertySheetDialog can not be a control dialog.");
        return NULL;
    }

    if ( pcpbdOwner->hDlg == NULL )
    {
        methodCanNotBeInvokedException(c, "execute()", pcpbdOwner->rexxSelf,
                                       "using an owner dialog whose Windows dialog does not exist");
        return NULL;
    }
    return pcpbdOwner->hDlg;
}


void assignPSDThreadContext(pCPropertySheetDialog pcpsd, RexxThreadContext *c, uint32_t threadID)
{
    pcpsd->pcpbd->dlgProcThreadID = threadID;
    pcpsd->pcpbd->dlgProcContext  = c;
    pcpsd->dlgProcThreadID        = threadID;
    pcpsd->dlgProcContext         = c;

    uint32_t count = pcpsd->pageCount;
    for ( uint32_t i = 0; i < count; i++ )
    {
        pcpsd->cppPages[i]->pcpbd->dlgProcThreadID = threadID;
        pcpsd->cppPages[i]->pcpbd->dlgProcContext  = c;
        pcpsd->cppPages[i]->dlgProcThreadID        = threadID;
        pcpsd->cppPages[i]->dlgProcContext         = c;
    }
}


bool psdInitSuper(RexxMethodContext *context, RexxClassObject super, RexxStringObject hFile, RexxObjectPtr id)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, context->NullString(), 1);
    if ( id != NULLOBJECT )
    {
        context->ArrayPut(newArgs, id, 2);
    }
    else
    {
        context->ArrayPut(newArgs, context->UnsignedInt32(DEFAULT_PROPSHETT_ID), 2);
    }
    if ( hFile != NULLOBJECT )
    {
        context->ArrayPut(newArgs, hFile, 4);
    }
    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    return isInt(0, result, context->threadContext);
}

/**
 * Determines if an icon for the tab is going to be used and sets the property
 * sheet page struct accordingly.
 *
 * The user can specify to use an icon for the tab by setting the imageList
 * attribute of the property sheet dialog or by setting the icon attribute of
 * the propert sheet page dialog.  In addition, the icon attribute itself can
 * either be an icon handle or an icon resource ID.
 *
 * The precedence for a tab icon is: image list in property sheet, loaded icon,
 * icon ID.
 *
 * @param c
 * @param pcpsd
 * @param psp
 *
 * @return uint32_t
 */
uint32_t maybeSetTabIcon(RexxMethodContext *c, pCPropertySheetDialog pcpsd, PROPSHEETPAGE *psp, uint32_t index)
{
    uint32_t flag = 0;

    if ( pcpsd->imageList != NULL )
    {
        HICON hIcon = ImageList_GetIcon(pcpsd->imageList, index, INDEXTOOVERLAYMASK(index) | ILD_NORMAL);

        psp->hIcon = hIcon;
        flag = PSP_USEHICON;
    }
    else
    {
        pCPropertySheetPage pcpsp = pcpsd->cppPages[index];

        if ( pcpsp->hIcon != NULL )
        {
            psp->hIcon = pcpsp->hIcon;
            flag = PSP_USEHICON;
        }
        else if ( pcpsp->iconID != 0 && (pcpsp->hInstance != NULL || pcpsp->pageType == oodResPSPDialog) )
        {
            psp->pszIcon = MAKEINTRESOURCE(pcpsp->iconID);
            flag = PSP_USEICONID;
        }
    }
    return flag;
}


/**
 * Initializes a single PROPSHEETPAGE struct.
 *
 * @param c
 * @param pcpsd
 * @param psp
 * @param i
 *
 * @return bool
 *
 * @remarks  On error, it is the caller's repsonsibility to clean up psp memory.
 *
 * @remarks  If this is a page in an Aero wizard, then the header title, if
 *           used, has to be in Unicode. To manage this we create a second copy
 *           of the header title text as Unicode.  It is possible to get here,
 *           with that second Unicode copy not yet made.  So, if it is an Aero
 *           wizard page, if headerTitle is not null and headerTitleAero is
 *           null, then we make the copy here.
 *
 *           Also, if this page is for an Aero Wizard, we do not automatically
 *           set the PSP_HIDEHEADER flag like we do for other wizards when it is
 *           an exterior page.  The user will have to explicitly set the flag in
 *           the page opts.
 *
 * @remarks  For ResDialogs, the user has to include all other resources, header
 *           bitmap, etc., in the resource dll for the dialog.  But, for other
 *           types of dialog pages the user can use a resource image for the
 *           additional resources by using the resourceImage attribute.  When we
 *           fill in the struct for a page we ignore the resourceImage attribute
 *           if it is a ResDialog page.
 */
bool initPSP(RexxMethodContext *c, pCPropertySheetDialog pcpsd, PROPSHEETPAGE *psp, uint32_t i, bool isExteriorPage)
{
    RexxObjectPtr       dlg   = pcpsd->rexxPages[i];
    pCPropertySheetPage pcpsp = pcpsd->cppPages[i];
    pCDynamicDialog     pcdd  = pcpsp->pcdd;
    uint32_t            flags = pcpsp->pageFlags;
    bool                success = false;

    RexxObjectPtr result = c->SendMessage0(dlg, "INITTEMPLATE");
    if ( result != TheTrueObj )
    {
        if ( ! c->CheckCondition() )
        {
            noWindowsPageDlgException(c, i + 1);
        }
        goto done_out;
    }

    psp->dwSize      = sizeof(PROPSHEETPAGE);
    psp->pfnDlgProc  = (DLGPROC)RexxPropertySheetPageDlgProc;
    psp->lParam      = (LPARAM)pcpsp;
    psp->pfnCallback = PropSheetPageCallBack;

    if ( pcpsp->pageTitle != NULL )
    {
        flags |= PSP_USETITLE;
        psp->pszTitle = pcpsp->pageTitle;
    }

    if ( pcpsp->pageType == oodResPSPDialog )
    {
        psp->hInstance   = pcpsp->pcpbd->hInstance;
        psp->pszTemplate = MAKEINTRESOURCE(pcpsp->resID);
    }
    else
    {
        flags |= PSP_DLGINDIRECT;
        psp->pResource = (PROPSHEETPAGE_RESOURCE)pcdd->base;

        if ( pcpsp->hInstance != NULL )
        {
            psp->hInstance = pcpsp->hInstance;
        }
    }

    flags |= maybeSetTabIcon(c, pcpsd, psp, i);

    if ( pcpsd->isWiz97 || pcpsd->isWizLite || pcpsd->isAeroWiz )
    {
        if ( pcpsp->headerTitle != NULL )
        {
            if ( pcpsp->isAeroWizardPage )
            {
                if ( pcpsp->headerTitleAero == NULL )
                {
                    // This shouldn't fail, but if it does, we just won't have
                    // any header text.
                    pcpsp->headerTitleAero = (char *)ansi2unicode(pcpsp->headerTitle);
                }
                psp->pszHeaderTitle = pcpsp->headerTitleAero;
            }
            else
            {
                psp->pszHeaderTitle = pcpsp->headerTitle;
            }

            if ( psp->pszHeaderTitle != NULL )
            {
                flags |= PSP_USEHEADERTITLE;
            }
        }

        if ( pcpsp->headerSubTitle != NULL )
        {
            if ( ! pcpsd->isAeroWiz )
            {
                psp->pszHeaderSubTitle = pcpsp->headerSubTitle;
                flags |= PSP_USEHEADERSUBTITLE;
            }
        }

        if ( pcpsp->headerTitle == NULL && pcpsp->headerSubTitle == NULL && isExteriorPage )
        {
            if ( ! pcpsd->isAeroWiz )
            {
                flags |= PSP_HIDEHEADER;
            }
        }
    }

    psp->dwFlags = flags;

    success = true;

done_out:
    return success;
}


/**
 * Allocates the memory for the property sheet page structs and fills in the
 * struct for all the pages of the property sheet.
 *
 * @param c        Method context we are operating in.
 * @param pcpsd    Pointer to the CSelf struct for a PropertySheetDialog.
 * @param hParent  Possible handle of parent dialog, can be null.
 *
 * @return The pointer to the property sheet page struct(s) on success, null on
 *         failure.
 *
 * @remarks  When we call initPSP() we pass true for the isExteriorPage argument
 *           on the first and last page without checking if the property sheet
 *           is a wizard or not. initPSP() does that check.
 */
PROPSHEETPAGE *initPropSheetPages(RexxMethodContext *c, pCPropertySheetDialog pcpsd)
{
    PROPSHEETPAGE *psp = NULL;
    uint32_t count = pcpsd->pageCount;

    psp = (PROPSHEETPAGE *)LocalAlloc(LPTR, MAXPROPPAGES * sizeof(PROPSHEETPAGE));
    if ( psp == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    // Initialize each individual page.
    for ( uint32_t i = 0; i < count; i++ )
    {
        if ( ! initPSP(c, pcpsd, psp + i, i, (i == 0 || i == count - 1)) )
        {
            LocalFree(psp);
            psp = NULL;
            goto done_out;
        }
    }

done_out:
    return psp;
}

/**
 * Allocates the memory for the property sheet header struct and fills in the
 * struct.
 *
 * @param c        Method context we are operating in.
 * @param pcpsd    Pointer to the CSelf struct for a PropertySheetDialog.
 * @param psp      Pointer to the property sheet page struct(s) for the property
 *                 sheet.
 * @param hParent  Possible handle of parent dialog, can be null.
 *
 * @return The pointer to the property sheet header on success, null on failure.
 */
PROPSHEETHEADER *initPropSheetHeader(RexxMethodContext *c, pCPropertySheetDialog pcpsd, LPCPROPSHEETPAGE psp, HWND hParent)
{
    PROPSHEETHEADER *psh = (PROPSHEETHEADER *)LocalAlloc(LPTR, sizeof(PROPSHEETHEADER));
    if ( psh == NULL )
    {
        outOfMemoryException(c->threadContext);
        return NULL;
    }

    uint32_t flags = pcpsd->propSheetFlags;

    psh->dwSize           = sizeof(PROPSHEETHEADER);
    psh->hwndParent       = hParent;
    psh->nPages           = pcpsd->pageCount;
    psh->ppsp             = (LPCPROPSHEETPAGE)psp;


    if ( pcpsd->pcpbd->isResizableDlg )
    {
        psh->pfnCallback = (PFNPROPSHEETCALLBACK)ResizablePropSheetCallback;
    }
    else
    {
        psh->pfnCallback = (PFNPROPSHEETCALLBACK)PropSheetCallback;
    }

    if ( pcpsd->hInstance != NULL )
    {
        psh->hInstance = pcpsd->hInstance;
    }

    if ( pcpsd->caption != NULL )
    {
        if ( pcpsd->isAeroWiz )
        {
            LPWSTR newCaption = ansi2unicode(pcpsd->caption);
            if ( newCaption != NULL )
            {
                LocalFree(pcpsd->caption);
                pcpsd->caption = (char *)newCaption;
            }
        }

        psh->pszCaption = pcpsd->caption;
    }

    if ( pcpsd->startPage != 0 )
    {
        psh->nStartPage = pcpsd->startPage - 1;
    }

    if ( pcpsd->hIcon != NULL )
    {
        psh->hIcon = pcpsd->hIcon;
        flags |= PSH_USEHICON;
    }
    else if ( pcpsd->iconID != 0 && pcpsd->hInstance != NULL )
    {
        psh->pszIcon = MAKEINTRESOURCE(pcpsd->iconID);
        flags |= PSH_USEICONID;
    }

    if ( pcpsd->isWiz97 || pcpsd->isAeroWiz )
    {
        if ( pcpsd->hHeaderBitmap != NULL )
        {
            psh->hbmHeader = pcpsd->hHeaderBitmap;

            if ( pcpsd->isWiz97 )
            {
                flags |= PSH_USEHBMHEADER | PSH_HEADER;
            }
            else
            {
                flags |= PSH_USEHBMHEADER | PSH_HEADERBITMAP;
            }
        }
        else if ( pcpsd->headerBitmapID != 0 && pcpsd->hInstance != NULL )
        {
            psh->pszbmHeader = MAKEINTRESOURCE(pcpsd->headerBitmapID);
            flags |= pcpsd->isWiz97 ? PSH_HEADER : PSH_HEADERBITMAP;
        }

        if ( ! pcpsd->isAeroWiz )
        {
            if ( pcpsd->hWatermark != NULL )
            {
                psh->hbmWatermark = pcpsd->hWatermark;
                flags |= PSH_USEHBMWATERMARK | PSH_WATERMARK;
            }
            else if ( pcpsd->watermarkID != 0 && pcpsd->hInstance != NULL )
            {
                psh->pszbmWatermark = MAKEINTRESOURCE(pcpsd->watermarkID);
                flags |= PSH_WATERMARK;
            }
        }
    }

    if ( pcpsd->modeless )
    {
        flags |= PSH_MODELESS;
    }

    psh->dwFlags = flags;

    return psh;
}


HWND getValidPageHwnd(RexxMethodContext *c, pCPropertySheetDialog pcpsd, RexxObjectPtr page, size_t pos)
{
    RexxObjectPtr match = NULL;

    uint32_t count = pcpsd->pageCount;
    for ( uint32_t i = 0; i < count; i++ )
    {
        if ( page == pcpsd->rexxPages[i] )
        {
            match = pcpsd->rexxPages[i];
            break;
        }
    }

    if ( match == NULL )
    {
        noSuchPageException(c, page, pos);
        return NULL;
    }

    HWND hPage = dlgToPSPHDlg(c, match);
    if ( hPage == NULL )
    {
        noWindowsPageException(c, dlgToPSPCSelf(c, page)->pageNumber + 1, pos);
    }

    return hPage;
}


/**
 * Checks the requirements, and allocates the PROPSHEETPAGE struct, for adding a
 * single page to an existing property sheet page
 *
 * @param c
 * @param pcpsd
 * @param _page
 * @param index
 * @param abortDialog
 *
 * @return PROPSHEETPAGE*
 */
PROPSHEETPAGE *getPSPMemory(RexxMethodContext *c, pCPropertySheetDialog pcpsd, pCPropertySheetPage *ppcpsp,
                            RexxObjectPtr page, uint32_t index, bool *abortDialog)
{
    PROPSHEETPAGE         *psp   = NULL;

    *abortDialog = false;

    if ( pcpsd->pageCount > MAXPROPPAGES )
    {
        userDefinedMsgException(c->threadContext, TOO_MANY_PROPSHEET_PAGES, MAXPROPPAGES);
        goto done_out;
    }

    if ( index < 1 || index > pcpsd->pageCount )
    {
        wrongRangeException(c->threadContext, 1, 1, pcpsd->pageCount, index);
        goto done_out;
    }

    if ( ! requiredClass(c->threadContext, page, "PROPERTYSHEETPAGE", 1) )
    {
        goto done_out;
    }

    pCPropertySheetPage pcpsp = dlgToPSPCSelf(c, page);
    *ppcpsp = pcpsp;

    if ( pcpsp->activated )
    {
        userDefinedMsgException(c->threadContext, 1, PROPSHEET_PAGE_ALREADY_ACTIVATED);
        goto done_out;
    }

    psp = (PROPSHEETPAGE *)LocalAlloc(LPTR, sizeof(PROPSHEETPAGE));
    if ( psp == NULL )
    {
        *abortDialog = true;
        outOfMemoryException(c->threadContext);
    }

done_out:
    return psp;
}


/**
 * Updates some of the CSelf fields for a property sheet page that is to be
 * added to a property sheet.
 *
 *
 * @param pcpsd
 * @param ppcpsp
 * @param index   Zero-based index of the page.
 *
 * @remarks  The original property sheet pages have their dialog procedure
 *           thread context set when the property sheet is created.  When pages
 *           are added or inserted we need to remember to set the thread
 *           context.  This thread context is the same as the property sheet's
 *           context.  (The thread context could also be copied from one of the
 *           other pages.)
 */
void updatePageCSelf(pCPropertySheetDialog pcpsd, pCPropertySheetPage pcpsp, uint32_t index)
{
    pcpsp->pageNumber = index;
    pcpsp->rexxPropSheet = pcpsd->rexxSelf;
    pcpsp->cppPropSheet = pcpsd;
    pcpsp->isWizardPage = ! pcpsd->isNotWizard;

    pcpsp->dlgProcContext  = pcpsd->dlgProcContext;
    pcpsp->dlgProcThreadID = pcpsd->dlgProcThreadID;
    pcpsp->pcpbd->dlgProcContext = pcpsd->dlgProcContext;
}


/** PropertySheetDialog::appIcon()      [Attribute set]
 *
 *  Sets the icon for the appIcon attribute.  The user can specify the icon as
 *  either a resource ID (numeric or symbolic) or as an .Image object.
 *
 *  @remarks  If the user specifies the icon as an .Image object, then it has to
 *            be an icon image, not some other type of image, like a bitmap,
 *            etc.  The rxGetImageIcon() call will raise an exception if the
 *            image is not an icon.
 */
RexxMethod2(RexxObjectPtr, psdlg_setAppIcon_atr, RexxObjectPtr, icon, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    bool    isImage;
    uint8_t type;

    intptr_t result = getImageOrID(context, pcpsd->rexxSelf, icon, 1, &isImage, &type);
    if ( result != 0 )
    {
        if ( isImage )
        {
            if ( type != IMAGE_ICON )
            {
                invalidImageException(context->threadContext, 1, "Icon", getImageTypeName(type));
                return NULLOBJECT;
            }
            pcpsd->hIcon = (HICON)result;
        }
        else
        {
            pcpsd->iconID = (uint32_t)result;
        }

        context->SetObjectVariable("APPICON", icon);
    }

    return NULLOBJECT;
}

/** PropertySheetDialog::caption()      [Attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_setCaption_atr, CSTRING, text, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    setCaption(context, pcpsd, text);
    return NULLOBJECT;
}


/** PropertySheetDialog::header()       [Attribute set]
 *
 *  Sets the header bitmap used for a Wizard (Wizard97 or AeroWizard.)
 *
 *  For a Wizard97, the user can specify the bitmap as either a resource ID
 *  (numeric or symbolic) or as an .Image object.  However for an AeroWizard,
 *  the bitmap must be specified as an .Image object.
 *
 *  @remarks  If the user specifies the header as an .Image object, then it has
 *            to be a bitmap image, not some other type of image, like an icon,
 *            etc.
 *
 *            If this property sheet is an aero wizard we don't check that this
 *            is Vista or later because it is checked when the AeroWizard
 *            keyword is first used.
 */
RexxMethod2(RexxObjectPtr, psdlg_setHeader_atr, RexxObjectPtr, header, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( ! (pcpsd->isAeroWiz || pcpsd->isWiz97) )
    {
        invalidAttributeException(context, pcpsd->rexxSelf);
        goto done_out;
    }

    bool    isImage;
    uint8_t type;

    intptr_t result = getImageOrID(context, pcpsd->rexxSelf, header, 1, &isImage, &type);
    if ( result != 0 )
    {
        if ( isImage )
        {
            if ( type != IMAGE_BITMAP )
            {
                wrongArgValueException(context->threadContext, 1, "Bitmap", getImageTypeName(type));
                goto done_out;
            }
            pcpsd->hHeaderBitmap = (HBITMAP)result;
        }
        else
        {
            if ( pcpsd->isAeroWiz )
            {
                wrongClassException(context->threadContext, 1, "Image");
                goto done_out;
            }

            pcpsd->headerBitmapID = (uint32_t)result;
        }

        context->SetObjectVariable("HEADER", header);
    }

done_out:
    return NULLOBJECT;
}

/** PropertySheetDialog::imageList()    [Attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_setImageList_atr, RexxObjectPtr, imageList, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    pcpsd->imageList = rxGetImageList(context, imageList, 1);
    if ( pcpsd->imageList != NULL )
    {
        context->SetObjectVariable("IMAGELIST", imageList);
    }
    return NULLOBJECT;
}

/** PropertySheetDialog::pages          [Attrbiute Get]
 *
 *  Gets the array of page dialogs for this property sheet.
 *
 *  @return  An array of Rexx dialogs.  Each index in the array contains the
 *           Rexx dialog for the page matching the index.  Page indexes are
 *           one-based.
 *
 *  @remarks.  There is no set method for this attribute, it is set in the
 *             native code when the user instantiates the property sheet.
 *
 *             We return a copy of the actual array so that the user can not
 *             alter the actual array.
 */
RexxMethod1(RexxObjectPtr, psdlg_getPages_atr, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    uint32_t count = pcpsd->pageCount;
    RexxArrayObject pages = context->NewArray(count);

    for ( uint32_t i = 0; i < count; i++ )
    {
        context->ArrayPut(pages, pcpsd->rexxPages[i], i + 1);
    }
    return pages;
}


/** PropertySheetDialog::resources()    [Attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_setResources_atr, RexxObjectPtr, resourceImage, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    PRESOURCEIMAGE ri = rxGetResourceImage(context, resourceImage, 1);
    if ( ri != NULL )
    {
        pcpsd->hInstance = ri->hMod;
        context->SetObjectVariable("RESOURCES", resourceImage);
    }
    return NULLOBJECT;
}

/** PropertySheetDialog::startPage()    [Attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_setStartPage_atr, uint32_t, startPage, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( startPage < 1 || startPage > MAXPROPPAGES )
    {
        wrongRangeException(context->threadContext, 1, 1, MAXPROPPAGES, startPage);
    }
    else
    {
        pcpsd->startPage = startPage;
        context->SetObjectVariable("STARTPAGE", context->UnsignedInt32(startPage));
    }

    return NULLOBJECT;
}

/** PropertySheetDialog::watermark()    [Attribute set]
 *
 *  Sets the watermark bitmap used for a Wizard97 wizard.
 *
 *  The user can specify the bitmap as either a resource ID (numeric or
 *  symbolic) or as an .Image object.
 *
 *  @remarks  If the user specifies the watermark as an .Image object, then it
 *            has to be a bitmap image, not some other type of image, like an
 *            icon, etc.
 */
RexxMethod2(RexxObjectPtr, psdlg_setWatermark_atr, RexxObjectPtr, watermark, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( ! pcpsd->isWiz97 )
    {
        invalidAttributeException(context, pcpsd->rexxSelf);
        goto done_out;
    }

    bool    isImage;
    uint8_t type;

    intptr_t result = getImageOrID(context, pcpsd->rexxSelf, watermark, 1, &isImage, &type);
    if ( result != 0 )
    {
        if ( isImage )
        {
            if ( type != IMAGE_BITMAP )
            {
                wrongArgValueException(context->threadContext, 1, "Bitmap", getImageTypeName(type));
                goto done_out;
            }
            pcpsd->hWatermark = (HBITMAP)result;
        }
        else
        {
            pcpsd->watermarkID = (uint32_t)result;
        }

        context->SetObjectVariable("WATERMARK", watermark);
    }

done_out:
    return NULLOBJECT;
}


/** PropertySheetDialog::init()
 *
 *  The initialization of the property sheet dialog.
 *
 *
 *  @remarks Note that we need to make a copy of the pages array sent to us by
 *           the programmer so that the programmer can not inadvertently screw
 *           with the array.
 */
RexxMethod7(wholenumber_t, psdlg_init, RexxArrayObject, pages, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, caption,
            OPTIONAL_RexxStringObject, hFile, OPTIONAL_RexxObjectPtr, id, SUPER, super, OSELF, self)
{
    // This is an error return.
    wholenumber_t result = 1;

    if ( ! psdInitSuper(context, super, hFile, id) )
    {
        goto err_out;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();

    // Get a buffer for the PropertySheetDialog CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CPropertySheetDialog));
    if ( cselfBuffer == NULLOBJECT )
    {
        goto done_out;
    }

    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)context->BufferData(cselfBuffer);
    memset(pcpsd, 0, sizeof(CPropertySheetDialog));

    pcpbd->dlgPrivate  = pcpsd;
    pcpsd->pcpbd       = pcpbd;
    pcpsd->rexxSelf    = self;
    pcpsd->isFirstShow = true;
    context->SetObjectVariable("CSELF", cselfBuffer);

    // Now process the arguments and do the rest of the initialization.
    uint32_t count = (uint32_t)context->ArrayItems(pages);
    if ( count == 0 )
    {
        emptyArrayException(context->threadContext, 1);
        goto done_out;
    }
    pcpsd->pageCount = count;

    pCPropertySheetPage *cppPages = (pCPropertySheetPage *)LocalAlloc(LPTR, MAXPROPPAGES * sizeof(pCPropertySheetPage *));
    RexxObjectPtr *rexxPages = (RexxObjectPtr *)LocalAlloc(LPTR, MAXPROPPAGES * sizeof(RexxObjectPtr *));

    if ( cppPages == NULL || rexxPages == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    // We need to parse the options before we look at the pages so we know if we
    // are a wizard or not.
    if ( ! parsePropSheetOpts(context, pcpsd, opts) )
    {
        goto done_out;
    }

    RexxArrayObject      pagesCopy = context->NewArray(count);
    pCPropertySheetPage *pPage     = cppPages;
    RexxObjectPtr       *pRexxPage = rexxPages;

    for ( uint32_t i = 1; i <= count; i++, pPage++, pRexxPage++ )
    {
        RexxObjectPtr dlg = context->ArrayAt(pages, i);
        if ( dlg == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, i);
            goto done_out;
        }
        if ( ! context->IsOfType(dlg, "PROPERTYSHEETPAGE") )
        {
            wrongObjInArrayException(context->threadContext, 1, i, "a PropertySheetPage", dlg);
            goto done_out;
        }

        pCPropertySheetPage pcpsp = dlgToPSPCSelf(context, dlg);
        pcpsp->pageNumber         = i - 1; // Zero-based index in CSelf.
        pcpsp->rexxPropSheet      = pcpsd->rexxSelf;
        pcpsp->cppPropSheet       = pcpsd;
        pcpsp->isWizardPage       = ! pcpsd->isNotWizard;

        *pPage = pcpsp;
        *pRexxPage = dlg;
        context->ArrayPut(pagesCopy, dlg, i);
    }

    pcpsd->cppPages = cppPages;
    pcpsd->rexxPages = rexxPages;
    pcpsd->getResultValue = OOD_NO_VALUE;

    // Set values for all the attributes, APPICON first:
    RexxObjectPtr rxIcon;
    SIZE          s;
    s.cx = GetSystemMetrics(SM_CXSMICON);
    s.cy = GetSystemMetrics(SM_CYSMICON);

    if ( TheDefaultSmallIcon != NULL )
    {
        pcpsd->hIcon = TheDefaultSmallIcon;
    }
    else
    {
        pcpsd->hIcon = getOORexxIcon(IDI_DLG_OOREXX);
    }

    // We need a Rexx Image object because the user can retrieve the appIcon
    // attribute.  We use LR_SHARED for the flag because, even if we are using
    // the TheDefaultSmallIcon and it was loaded from a file, the only thing the
    // LR_SHARED flag does is prevent DestroyIcon() from being called if the
    // uninit() method runs for the .Image object.  Which is what we want for
    // the default application icon.
    rxIcon = rxNewValidImage(context, pcpsd->hIcon, IMAGE_ICON, &s, LR_SHARED, true);
    context->SetObjectVariable("APPICON", rxIcon);

    // Be sure we have a good dialog ID:
    if ( pcpbd->dlgID == -1 )
    {
        pcpbd->dlgID = DEFAULT_PROPSHETT_ID;
    }

    // CAPTION
    if ( argumentOmitted(3) )
    {
        caption = "ooRexx Property Sheet Dialog";
    }
    if ( ! setCaption(context, pcpsd, caption) )
    {
        goto done_out;
    }

    // and the rest:
    context->SetObjectVariable("HEADER", TheNilObj);
    context->SetObjectVariable("IMAGELIST", TheNilObj);
    context->SetObjectVariable("PAGES", pagesCopy);
    context->SetObjectVariable("RESOURCES", TheNilObj);

    pcpsd->startPage = 1;
    context->SetObjectVariable("STARTPAGE", TheOneObj);

    context->SetObjectVariable("WATERMARK", TheNilObj);

    result = 0;

done_out:
    pcpbd->wndBase->initCode = result;

err_out:
    return result;
}


/** PropertySheetDialog::addPage()
 *
 *  Adds a page to the property sheet at the end of the pages.
 *
 *  @param _page          The Rexx property sheet page dialog.
 *
 *  @param isExteriorPage [OPTIONAL]  If the added page is an exterior page in a
 *                        wizard.  The default is false.  If the property sheet
 *                        is not a wizard, this argument is ignored.  The
 *                        default is false, the page is NOT an exterior page.
 *
 *  @return  True on success, otherwise false.
 *
 *  @notes  Syntax conditions are raised if the _page argument is not correct.
 *          _page must be a PropertySheetPage dialog object.
 *
 *          The dialog object must not have already been "used" as a property
 *          sheet page.  In addition to adding or inserting pages in a prooerty
 *          sheet, pages can be removed from a property sheet.  When the page is
 *          removed, the operating system destroys the underlying Windows
 *          dialog.  Although the Rexx dialog object is still active, that
 *          object can not be reinserted into a property sheet as a page.
 *
 *          It is possible that the user never 'visited' an inserted page before
 *          it was removed. In this case the Rexx dialog object could be used to
 *          add or insert a new page at some later point, because the underlying
 *          Windows dialog would never have been created. The wasActivated
 *          attribute can be used to test for this condition.  However, it is
 *          probably simplier to always instantiate a new Rexx dialog object for
 *          each page added to a property sheet.
 *
 *          The Windows operating system restricts the maximum number of pages
 *          that a property sheet can hold.  The MAXPROPPAGES constant of the
 *          PropertySheetDialog reflects this number.  If the programer tries to
 *          insert or add a page past this number, a condition is raised.
 *
 */
RexxMethod3(RexxObjectPtr, psdlg_addPage, RexxObjectPtr, _page, OPTIONAL_logical_t, isExteriorPage, CSELF, pCSelf)
{
    pCPropertySheetDialog  pcpsd = (pCPropertySheetDialog)pCSelf;

    bool abortDialog;
    uint32_t count = pcpsd->pageCount;

    pCPropertySheetPage pcpsp = NULL;

    PROPSHEETPAGE *psp = getPSPMemory(context, pcpsd, &pcpsp, _page, count, &abortDialog);
    if ( psp == NULL )
    {
        goto err_out;
    }

    pcpsp->psp = psp;
    abortDialog = false;

    updatePageCSelf(pcpsd, pcpsp, count);

    pcpsd->cppPages[count] = pcpsp;
    pcpsd->rexxPages[count] = _page;
    pcpsd->pageCount++;

    if ( ! initPSP(context, pcpsd, psp, count, isExteriorPage ? true : false) )
    {
        goto err_out;
    }

    abortDialog = true;

    pcpsp->hPropSheetPage = CreatePropertySheetPage(psp);
    if ( pcpsp->hPropSheetPage == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "CreatePropertySheetPage");
        goto err_out;
    }

    if ( ! PropSheet_AddPage(pcpsd->hDlg, pcpsp->hPropSheetPage) )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "PropSheet_AddPage");
        goto err_out;
    }

    PropSheet_RecalcPageSizes(pcpsd->hDlg);

    return TheTrueObj;

err_out:
    safeLocalFree(psp);

    if ( pcpsp != NULL )
    {
        pcpsp->psp = NULL;

    if ( pcpsp->hPropSheetPage != NULL )
        {
            DestroyPropertySheetPage(pcpsp->hPropSheetPage);
            pcpsp->hPropSheetPage = NULL;
        }
    }

    if ( abortDialog )
    {
        endDialogPremature(pcpsd->pcpbd, pcpsd->hDlg, RexxConditionRaised);
    }
    return TheFalseObj;
}


/** PropertySheetDialog::apply()
 *
 *  Simulates the selection of the Apply button, indicating that one or more
 *  pages have changed and the changes need to be validated and recorded.
 *
 */
RexxMethod1(RexxObjectPtr, psdlg_apply, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    return PropSheet_Apply(pcpsd->hDlg) ? TheTrueObj : TheFalseObj;
}


/** PropertySheetDialog::cancelToClose()
 *
 *  Used when changes made since the most recent PSN_APPLY notification cannot
 *  be canceled.
 *
 */
RexxMethod1(RexxObjectPtr, psdlg_cancelToClose, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    PropSheet_CancelToClose(pcpsd->hDlg);
    return TheZeroObj;
}


/** PropertySheetDialog::changed()
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_changed, RexxObjectPtr, _page, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    HWND page = getValidPageHwnd(context, pcpsd, _page, 1);
    if ( page != NULL )
    {
        PropSheet_Changed(pcpsd->hDlg, page);
    }
    return TheZeroObj;
}


/** PropertySheetDialog::enableWizButtons()
 *
 *  Enables or disables buttons in an Aero wizard.
 *
 *  @param  opts       Zero or more of the keyword values that specify which
 *                     property sheet buttons are to be enabled. If a button
 *                     value is included in both this argument and the
 *                     optsButtons argument, then the button is enabled.
 *
 *  @param optsButtons Zero or more of the same keywords used in opts. Here,
 *                     they specify which property sheet buttons are to be
 *                     enablred or disabled. If a keyword appears in this
 *                     argument but not in opts, it indicates that the button
 *                     should be enabled.
 *
 *  @param  Returns true if this is an Aero Wizard property sheet on Vista on
 *          later, otherwise false.
 *
 *  @notes  This method only works for Aero Wizards.  This method requires Vista
 *          or later, a condition is raised if the OS is not Vista or later.
 *          This method has no effect if the property sheet is not an Aero
 *          Wizard.
 */
RexxMethod3(RexxObjectPtr, psdlg_enableWizButtons, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, optsButtons, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( requiredOS(context, "enableWizButtons", "Vista", Vista_OS) && pcpsd->isAeroWiz )
    {
        uint32_t flags = 0;
        uint32_t buttons = 0;

        if ( argumentExists(1) )
        {
            if ( StrStrI(opts, "BACK")   != NULL ) flags |= PSWIZB_BACK;
            if ( StrStrI(opts, "NEXT")   != NULL ) flags |= PSWIZB_NEXT;
            if ( StrStrI(opts, "FINISH") != NULL ) flags |= PSWIZB_FINISH;
            if ( StrStrI(opts, "CANCEL") != NULL ) flags |= PSWIZB_CANCEL;
        }

        if ( argumentExists(2) )
        {
            if ( StrStrI(optsButtons, "BACK")   != NULL ) buttons |= PSWIZB_BACK;
            if ( StrStrI(optsButtons, "NEXT")   != NULL ) buttons |= PSWIZB_NEXT;
            if ( StrStrI(optsButtons, "FINISH") != NULL ) buttons |= PSWIZB_FINISH;
            if ( StrStrI(optsButtons, "CANCEL") != NULL ) buttons |= PSWIZB_CANCEL;
        }

        PropSheet_EnableWizButtons(pcpsd->hDlg, flags, buttons);

        return TheTrueObj;
    }

    return TheFalseObj;
}


/** PropertySheetDialog::execute()
 *
 *  Creates a modal property sheet dialog.
 *
 *  @note  Sets the .systemErrorCode
 */
RexxMethod2(RexxObjectPtr, psdlg_execute, OPTIONAL_RexxObjectPtr, owner, CSELF, pCSelf)
{
    RexxObjectPtr   result = TheNegativeOneObj;
    HWND            hParent = NULL;

    oodSetSysErrCode(context->threadContext);

    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    pcpsd->getResultValue = -1;

    PROPSHEETPAGE *psp = NULL;
    PROPSHEETHEADER *psh = NULL;

    if ( argumentExists(1) )
    {
        hParent = checkPropSheetOwner(context, owner, 1);
        if ( hParent == NULL )
        {
            goto done_out;
        }
    }

    psp = initPropSheetPages(context, pcpsd);
    if ( psp == NULL )
    {
        goto done_out;
    }

    // It is not necessary to set pcpsd->modeless to false, it is false by default.

    psh = initPropSheetHeader(context, pcpsd, psp, hParent);
    if ( psh == NULL )
    {
        goto done_out;
    }

    intptr_t ret;
    if ( hParent == NULL )
    {
        assignPSDThreadContext(pcpsd, context->threadContext, GetCurrentThreadId());

        if ( setPropSheetHook(pcpsd) )
        {
            ret = PropertySheet(psh);
            oodSetSysErrCode(context->threadContext);
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = (intptr_t)SendMessage(hParent, WM_USER_CREATEPROPSHEET_DLG, (WPARAM)psh, (LPARAM)pcpsd);
    }

    pcpsd->getResultValue = ret;

    // Call leaving now, but note that the underlying Windows property sheet
    // dialog is now destroyed
    context->SendMessage0(pcpsd->rexxSelf, "LEAVING");

    result = context->WholeNumber(ret);

done_out:
    safeLocalFree(psp);
    safeLocalFree(psh);
    stopDialog(pcpsd->pcpbd, context->threadContext);
    return result;
}


/** PropertySheetDialog::getCurrentPageHwnd()
 *
 *  Retrieves a handle to the window of the current page of a property sheet.
 *
 */
RexxMethod1(RexxObjectPtr, psdlg_getCurrentPageHwnd, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    return pointer2string(context, PropSheet_GetCurrentPageHwnd(pcpsd->hDlg));
}


/** PropertySheetDialog::getPage()
 *
 *  Gets the page dialog specified by index.
 *
 *  @index  The one-based index of the page whose dialog is desired.
 *
 *  @return  The Rexx dialog object for the page specified.
 *
 *  @notes  Raises an exception if index is not correct.
 *
 *  @remarks  This method is not an interface to a PSM_x message, it is a helper
 *            function for ooDialog programmers.
 */
RexxMethod2(RexxObjectPtr, psdlg_getPage, uint32_t, index, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    uint32_t count = pcpsd->pageCount;
    if ( index < 1 || index > count )
    {
        wrongRangeException(context->threadContext, 1, 1, (int)count, index);
        return NULLOBJECT;
    }

    return pcpsd->rexxPages[index - 1];
}


/** PropertySheetDialog::getResult()
 *
 *  Returns the result of executing the PropertySheetDialog.
 *
 *  Originally this was intended to be  used by modeless property sheets to
 *  retrieve the same information returned to modal property sheets. Now howver,
 *  the getResultValue is set for modeless on modal dialogs.
 *
 */
RexxMethod1(RexxObjectPtr, psdlg_getResult, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    char *result;

    switch ( pcpsd->getResultValue )
    {
        case OOD_NO_VALUE :
            result = "NotFinished";
            break;
        case -1 :
            result = "ExecutionErr";
            break;
        case ID_PSRESTARTWINDOWS :
            result = "RestartWindows";
            break;
        case ID_PSREBOOTSYSTEM :
            result = "RebootSystem";
            break;
        case 0 :
            result = "ClosedCancel";
            break;
        case 1 :
            result = "ClosedOk";
            break;
        default :
            result = "Unknown";
            break;
    }
    return context->String(result);
}


/** PropertySheetDialog::getTabControl()
 *
 *  Retrieves the Rexx tab control object for the tab control of the property
 *  sheet.
 *
 *  Note that in this case, once we instantiate the Rexx tab object, we do not
 *  want it garbage collected so we use specify true in the
 *  createControlFromHwnd() call to have it put in the dialog's control bag.
 */
RexxMethod1(RexxObjectPtr, psdlg_getTabControl, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    return createControlFromHwnd(context, pcpsd->pcpbd, PropSheet_GetTabControl(pcpsd->hDlg), winTab, true);
}


/** PropertySheetDialog::hwndToIndex()
 *
 *  Takes a window handle of the property sheet page and returns its one-based
 *  index.
 *
 *  @param  hwnd  The window handle of a property sheet page.
 *
 *  @return  The one-based index of the page on success, or 0 on error.
 */
RexxMethod2(uint32_t, psdlg_hwndToIndex, POINTERSTRING, hwnd, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    int index = PropSheet_HwndToIndex(pcpsd->hDlg, hwnd);
    return ++index;
}


/** PropertySheetDialog::idToIndex()
 *
 *  Takes a property sheet page ID and returns its one-based index.
 *
 *  @param  id  The property sheet page ID.
 *
 *  @return  The one-based index of the page on success, or 0 on error.
 */
RexxMethod2(uint32_t, psdlg_idToIndex, POINTER, id, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    int index = PropSheet_IdToIndex(pcpsd->hDlg, id);
    return ++index;
}


/** PropertySheetDialog::indexToID()
 *
 *  Given the index of a page, returns its ID.
 *
 *  Page IDs are different depending on the type of the dialog page,
 *  UserPSPDialog, ResPSPDialog, etc..  In order to get the correct ID, the Rexx
 *  programmer must use this method.
 *
 *  There are also two special case values.  The page ID would most commonly be
 *  used in the setActive, wizBack, or wizNext event notification methods that
 *  signal a page is being changed.  To accept the page change, 0 is returned,
 *  and to cancel the page change -1 is returned.  So, 0 and -1 are acceptable
 *  here and return the proper value for those methods.
 *
 *  @param  index  The one-based index of the page.  The special values 0 and -1
 *                 are also acceptable.
 *
 *  @return  The proper page ID for the index.
 *
 *  @notes  A syntax condition is raised if the index is not -1, 0, or within
 *          the range of existing pages.
 *
 *  @remarks  In Windows the page id is either the resource ID of the dialog for
 *            a dialog template bound to an executable, or the pointer to the
 *            in-memory template for a dynamically constructed template.  We
 *            keep track of the proper id when the template is used and return
 *            it here to the programmer when requested.
 */
RexxMethod2(RexxObjectPtr, psdlg_indexToID, int32_t, index, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    intptr_t result = -2;

    int max = (int)pcpsd->pageCount;

    if ( index < -1 || index > max )
    {
        wrongRangeException(context->threadContext, 1, -1, max, index);
    }
    else if ( index < 1 )
    {
        result = index;
    }
    else
    {
        pCPropertySheetPage pcpsp = pcpsd->cppPages[index - 1];
        result = pcpsp->pageID;
    }

    return context->NewPointer((void *)result);
}


/** PropertySheetDialog::indexToHwnd()
 *
 *  Given the index to a page, returns the page dialog's window handle.
 *
 *  @param  index  The one-based page index.
 *
 *  @return  The window handle for the page.
 *
 ** PropertySheetDialog::indexToPage()
 *
 *  Given the index to a page, returns its HPROPSHEETPAGE handle
 *
 *  @param  index  The one-based page index.
 *
 *  @return  The HPROPSHEETPAGE handle for the page.
 *
 *  @notes  A syntax condition is raised if index is not the index of an
 *          existing page in the propertye sheet.
 *
 */
RexxMethod3(RexxObjectPtr, psdlg_indexToHandle, int, index, NAME, method, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    int max = (int)pcpsd->pageCount;

    if ( index < 1 || index > max )
    {
        wrongRangeException(context->threadContext, 1, 1, max, index);
        return NULL;
    }

    index--;
    if ( *(method + 7) == 'H' )
    {
        return pointer2string(context, PropSheet_IndexToHwnd(pcpsd->hDlg, index));
    }
    else
    {
        RexxMethodContext *c = context;
        return c->NewPointer(PropSheet_IndexToPage(pcpsd->hDlg, index));
    }
}


/** PropertySheetDialog::insertPage()
 *
 *
 */
RexxMethod4(RexxObjectPtr, psdlg_insertPage, RexxObjectPtr, _page, uint32_t, index, OPTIONAL_logical_t, isExteriorPage, CSELF, pCSelf)
{
    pCPropertySheetDialog  pcpsd = (pCPropertySheetDialog)pCSelf;

    bool     abortDialog;
    uint32_t max = pcpsd->pageCount;

    pCPropertySheetPage pcpsp = NULL;

    PROPSHEETPAGE *psp = getPSPMemory(context, pcpsd, &pcpsp, _page, max, &abortDialog);
    if ( psp == NULL )
    {
        goto err_out;
    }

    pcpsp->psp = psp;
    abortDialog = false;

    index--;
    updatePageCSelf(pcpsd, pcpsp, index);

    if ( index + 1 < max )
    {
        for ( uint32_t i = max - 1, j = max; i >= index; i--, j-- )
        {
            pcpsd->rexxPages[j] = pcpsd->rexxPages[i];
            pcpsd->cppPages[j] = pcpsd->cppPages[i];
            pcpsd->cppPages[j]->pageNumber = j;
        }
    }

    pcpsd->cppPages[index] = pcpsp;
    pcpsd->rexxPages[index] = _page;
    pcpsd->pageCount++;

    if ( ! initPSP(context, pcpsd, psp, index, isExteriorPage ? true : false) )
    {
        goto err_out;
    }

    abortDialog = true;

    pcpsp->hPropSheetPage = CreatePropertySheetPage(psp);
    if ( pcpsp->hPropSheetPage == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "CreatePropertySheetPage");
        goto err_out;
    }

    if ( PropSheet_InsertPage(pcpsd->hDlg, index, pcpsp->hPropSheetPage) == 0 )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "PropSheet_InsertPage");
        goto err_out;
    }

    PropSheet_RecalcPageSizes(pcpsd->hDlg);

    return TheTrueObj;

err_out:
    safeLocalFree(psp);

    if ( pcpsp != NULL )
    {
        pcpsp->psp = NULL;

        if ( pcpsp->hPropSheetPage != NULL )
        {
            DestroyPropertySheetPage(pcpsp->hPropSheetPage);
            pcpsp->hPropSheetPage = NULL;
        }
    }

    if ( abortDialog )
    {
        endDialogPremature(pcpsd->pcpbd, pcpsd->hDlg, RexxConditionRaised);
    }
    return TheFalseObj;
}


/** PropertySheetDialog::pageToIndex()
 *
 *  Takes the handle of a property sheet page and returns its one-based index.
 *
 *  @param  hPage  The handle of a property sheet page.
 *
 *  @return  The one-based index of the page on success, or 0 on error.
 */
RexxMethod2(uint32_t, psdlg_pageToIndex, POINTER, hPage, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    int index = PropSheet_PageToIndex(pcpsd->hDlg, hPage);
    return ++index;
}


/** PropertySheetDialog::popup()
 *
 *
 *  @notes  AeroWizard dialogs do not support modeless
 */
RexxMethod1(RexxObjectPtr, psdlg_popup, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcpsd->pcpbd;

    PROPSHEETPAGE *psp = NULL;
    PROPSHEETHEADER *psh = NULL;

    if ( pcpsd->isAeroWiz )
    {
        methodCanNotBeInvokedException(context, "popup", pcpsd->rexxSelf, "with the AeroWizard style");
        goto err_out;
    }

    psp = initPropSheetPages(context, pcpsd);
    if ( psp == NULL )
    {
        goto err_out;
    }

    pcpsd->modeless = true;

    psh = initPropSheetHeader(context, pcpsd, psp, NULL);
    if ( psh == NULL )
    {
        goto err_out;
    }

    DWORD threadID;
    bool Release = false;

    EnterCriticalSection(&crit_sec);

    PropSheetThreadArgs threadArgs;
    threadArgs.pcpsd = pcpsd;
    threadArgs.psh = psh;
    threadArgs.release = &Release;

    pcpbd->hDlgProcThread = CreateThread(NULL, 2000, PropSheetLoopThread, &threadArgs, 0, &threadID);

    // Wait for thread to signal us to continue, don't wait if the thread was not created.
    while ( ! Release && pcpbd->hDlgProcThread != NULL )
    {
        Sleep(1);
    }
    LeaveCriticalSection(&crit_sec);

    // Note we do not need to set pcpbd->dlgProcThreadID here.  It is set in
    // PropSheetLoopThread because that function also sets dlgProcContext and
    // dlgProcThreadID for all the property sheet page dialogs.
    if ( pcpbd->hDlgProcThread != NULL )
    {
        return TheTrueObj;
    }
    else
    {
        // Something failed in the the thread function.  In that case, things
        // are cleaned up in the thread function.
        return TheFalseObj;
    }

err_out:
    safeLocalFree(psp);
    safeLocalFree(psh);
    stopDialog(pcpsd->pcpbd, context->threadContext);
    return TheFalseObj;
}


/** PropertySheetDialog::popupAsChild()
 *
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_popupAsChild, RexxObjectPtr, parent, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcpsd->pcpbd;

    if ( ! requiredClass(context->threadContext, parent, "PlainBaseDialog", 1) )
    {
        goto err_out;
    }

    RexxObjectPtr childDialogs = context->SendMessage0(parent, "CHILDDIALOGS");
    if ( childDialogs != NULLOBJECT )
    {
        context->SendMessage1(childDialogs, "INSERT", pcpsd->rexxSelf);
        pcpbd->rexxParent = parent;

        return context->SendMessage0(pcpsd->rexxSelf, "POPUP");
    }

err_out:
    return TheFalseObj;
}


/** PropertySheetDialog::pressButton()
 *
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_pressButton, CSTRING, button, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    int flag = 0;

    if ( StrStrI(button, "APPLYNOW")    != NULL ) flag = PSBTN_APPLYNOW;
    else if ( StrStrI(button, "BACK")   != NULL ) flag = PSBTN_BACK;
    else if ( StrStrI(button, "CANCEL") != NULL ) flag = PSBTN_CANCEL;
    else if ( StrStrI(button, "FINISH") != NULL ) flag = PSBTN_FINISH;
    else if ( StrStrI(button, "HELP")   != NULL ) flag = PSBTN_HELP;
    else if ( StrStrI(button, "NEXT")   != NULL ) flag = PSBTN_NEXT;
    else if ( StrStrI(button, "OK")     != NULL ) flag = PSBTN_OK;
    else
    {
        wrongArgValueException(context->threadContext, 1, VALID_PROPSHEET_BUTTONS, button);
        return TheZeroObj;
    }

    PropSheet_PressButton(pcpsd->hDlg, flag);
    return TheZeroObj;
}


/** PropertySheetDialog::querySiblings()
 *
 *
 */
RexxMethod3(int32_t, psdlg_querySiblings, RexxObjectPtr, wParam, RexxObjectPtr, lParam, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    return (int32_t)PropSheet_QuerySiblings(pcpsd->hDlg, (WPARAM)wParam, (LPARAM)lParam);
}


/** PropertySheetDialog::removePage()
 *
 *  Removes a page from the property sheet.
 *
 *  @param  index  The one-based index of the page to be removed.
 *
 *  @return  Zero always.
 *
 *  @remarks  If the page being removed is not the last page we need to move the
 *            pointers in the arrays down 1.  However, we also need to adjust
 *            the page numbers in the Rexx page object.  So, rather than do a
 *            memmove for the pointers, we may as well just walk the arrays.
 */
RexxMethod2(RexxObjectPtr, psdlg_removePage, OPTIONAL_uint32_t, index, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    uint32_t max = pcpsd->pageCount;

    if ( argumentOmitted(1) )
    {
        index = max;
    }

    if ( index < 1 || index > max )
    {
        wrongRangeException(context->threadContext, 1, 1, max, index);
        return TheZeroObj;
    }

    index--;
    pCPropertySheetPage pcpspRemove = pcpsd->cppPages[index];

    if ( index + 1 < max )
    {
        for ( uint32_t i = index, j = index + 1; i > max - 1; i++, j++ )
        {
            pcpsd->rexxPages[i] = pcpsd->rexxPages[j];
            pcpsd->cppPages[i] = pcpsd->cppPages[j];
            pcpsd->cppPages[i]->pageNumber = i;
        }
    }

    pcpsd->rexxPages[max] = NULL;
    pcpsd->cppPages[max] = NULL;
    pcpsd->pageCount--;

    pcpspRemove->inRemovePage = true;

    PropSheet_RemovePage(pcpsd->hDlg, index, NULL);
    PropSheet_RecalcPageSizes(pcpsd->hDlg);

    if ( pcpspRemove->hPropSheetPage != NULL )
    {
        DestroyPropertySheetPage(pcpspRemove->hPropSheetPage);
        pcpspRemove->hPropSheetPage = NULL;

        safeLocalFree(pcpspRemove->psp);
        pcpspRemove->psp = NULL;
    }

    pcpspRemove->inRemovePage = false;

    return TheZeroObj;
}


/** PropertySheetDialog::setButtonText()
 *
 *  Sets the text of the specified button in an Aero wizard
 *
 *  @param  button  A keyword specifying which button to set the text for, Back,
 *                  Cancel, Finish, or Next.
 *
 *  @param  text    The text for the Next button.
 *
 *  @return  True on success, false otherwise.
 *
 *  @note  Raises syntax conditions if not Vista and if keyword is incorrect.
 *
 */
RexxMethod3(RexxObjectPtr, psdlg_setButtonText, CSTRING, button, CSTRING, text, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( requiredOS(context, "setButtonText", "Vista", Vista_OS) && pcpsd->isAeroWiz )
    {
        uint32_t flag = 0;

        if (      StrStrI(button, "BACK")   != NULL ) flag = PSWIZB_BACK;
        else if ( StrStrI(button, "CANCEL") != NULL ) flag = PSWIZB_CANCEL;
        else if ( StrStrI(button, "FINISH") != NULL ) flag = PSWIZB_FINISH;
        else if ( StrStrI(button, "NEXT")   != NULL ) flag = PSWIZB_NEXT;
        else
        {
            wrongArgValueException(context->threadContext, 1, VALID_AERO_BUTTONS, button);
            return TheFalseObj;
        }

        LPWSTR buttonText = ansi2unicode(text);
        if ( buttonText != NULL )
        {
            PropSheet_SetButtonText(pcpsd->hDlg, flag, buttonText);
            LocalFree(buttonText);
            return TheTrueObj;
        }
    }

    return TheFalseObj;
}


/** PropertySheetDialog::setCurSel()
 *
 *  Activates the specified page in a property sheet.
 *
 *  @param  index  [OPTIONAL] The one-based page index.
 *  @param  hPage  [OPTIONAL] The property sheet page handle.
 *
 *  @return  True on success, otherwise false.
 *
 *  @notes The property sheet page to active can be specified by either the
 *         property sheet page handle, or the page index, or both.  If both are
 *         specified, the property sheet page handle takes precedence.
 *
 *         Although both arguments are optional, they are optional individually.
 *         At least one of the arguments must be specified or a condition is
 *         raised.  In addition, if the index argument is used to specify the
 *         page is not a valid index, a condition is raised.
 */
RexxMethod3(RexxObjectPtr, psdlg_setCurSel, OPTIONAL_int32_t, index, OPTIONAL_POINTER, hPage, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    RexxObjectPtr result = TheFalseObj;

    if ( argumentExists(1) )
    {
        int max = (int)pcpsd->pageCount;

        if ( index < 1 || index > max )
        {
            wrongRangeException(context->threadContext, 1, 1, max, index);
            goto done_out;
        }

        index--;
        result = PropSheet_SetCurSel(pcpsd->hDlg, NULL, index) ? TheTrueObj : TheFalseObj;
    }
    else
    {
        if ( argumentOmitted(2) )
        {
            missingArgException(context->threadContext, 2);
            goto done_out;
        }

        result = PropSheet_SetCurSel(pcpsd->hDlg, (HPROPSHEETPAGE)hPage, NULL) ? TheTrueObj : TheFalseObj;
    }

done_out:
    return result;
}


/** PropertySheetDialog::setCurSelByID()
 *
 *  Activates the specified page in a property sheet.
 *
 *  @param  id  The property sheet page ID.
 *
 *  @return  True on success, otherwise false.
 *
 *  @notes The property sheet page ID can be obtained using the indexToID()
 *         method.  Do not confuse a page ID with a page handle, they are 2
 *         separate things.  The only way for the Rexx programmer to obtain the
 *         page ID is through indexToID().
 */
RexxMethod2(RexxObjectPtr, psdlg_setCurSelByID, POINTER, id, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    RexxObjectPtr result = PropSheet_SetCurSelByID(pcpsd->hDlg, id) ? TheTrueObj : TheFalseObj;
    return result;
}


/** PropertySheetDialog::setFinishText()
 *
 *  Sets the text of the Finish button in a wizard, shows and enables the
 *  button, and hides the Next and Back buttons.
 *
 *  @param  text  The text for the Finish button.
 *
 *  @return  0, always.
 *
 *  @note  setFinishText() does not work for Aero Wizards.  Use setButtonText()
 *         instead.
 */
RexxMethod2(RexxObjectPtr, psdlg_setFinishText, CSTRING, text, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    PropSheet_SetFinishText(pcpsd->hDlg, text);

    return TheZeroObj;
}

#if 0
/** PropertySheetDialog::setHeaderBitmap()
 *
 *  The property sheet PropSheet_SetHeaderBitmap and
 *  PropSheet_SetHeaderBitmapResource macros are apparently not implemented.  I
 *  wrote this code before discovering that.  The macros are not implemented
 *  because the underlying PSM messages are not implemented.  Going to save the
 *  code for future use.
 */
RexxMethod3(RexxObjectPtr, psdlg_setHeaderBitmap, uint32_t, index, RexxObjectPtr, bitmap, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( ! requiredOS(context, "setHeaderBitmap", "Vista", Vista_OS) || pcpsd->isNotWizard )
    {
        goto done_out;
    }

    uint32_t max = pcpsd->pageCount;

    if ( index < 1 || index > max )
    {
        wrongRangeException(context->threadContext, 1, 1, max, index);
        goto done_out;
    }

    index--;

    POODIMAGE oodImage = rxGetImageBitmap(context, bitmap, 2);
    if ( oodImage == NULL )
    {
        goto done_out;
    }

    PropSheet_SetHeaderBitmap(pcpsd->hDlg, index, oodImage->hImage);

done_out:
    return TheZeroObj;
}
#endif


/** PropertySheetDialog::setHeaderSubtitle()
 *  PropertySheetDialog::setHeaderTitle()
 *
 *  Resets the text for a page in a property sheet wizard dialog.
 *  setHeaderSubTitle() is not available for Aero wizards
 *
 *  The same native function handles the setHeaderTitle() and
 *  setHeaderSubtitle() methods.
 *
 *  @param  index    The one-based index of the page whose text is being
 *                   changed.
 *  @param  newText  New text for the page.
 *
 *  @return  Zero, always.
 *
 *  @note Neither function works for Aero wizards.  Tested many times.  Leaving
 *        Aero code for setHeaderTitle() in.
 */
RexxMethod4(RexxObjectPtr, psdlg_resetPageText, uint32_t, index, CSTRING, newText, NAME, method, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( pcpsd->isNotWizard )
    {
        goto done_out;
    }

    uint32_t max = pcpsd->pageCount;

    if ( index < 1 || index > max )
    {
        wrongRangeException(context->threadContext, 1, 1, max, index);
        goto done_out;
    }

    pCPropertySheetPage pcpsp = pcpsd->cppPages[--index];

    if ( *(method + 9) == 'S' )
    {
        if ( setPageText(context, pcpsp, newText, headerSubtext) )
        {
            PropSheet_SetHeaderSubTitle(pcpsd->hDlg, index, pcpsp->headerSubTitle);
        }
    }
    else
    {
        if ( setPageText(context, pcpsp, newText, headerText) )
        {
            if ( pcpsd->isAeroWiz )
            {
                PropSheet_SetHeaderTitle(pcpsd->hDlg, index, pcpsp->headerTitleAero);
            }
            else
            {
                PropSheet_SetHeaderTitle(pcpsd->hDlg, index, pcpsp->headerTitle);
            }
        }
    }

done_out:
    return TheZeroObj;
}


/** PropertySheetDialog::setNextText()
 *
 *  Sets the text of the Next button in an aero wizard.
 *
 *  @param  text  The text for the Next button.
 *
 *  @return  True if an aero wizard, otherwise false
 *
 *  @requires Vista or later.
 *
 *  @remarks  Testing has shown that all text in an aero wizard has to be
 *            Unicode.  Not sure about freeing the text after the call to
 *            PropSheet_SetNextText(), but it seems okay.
 */
RexxMethod3(RexxObjectPtr, psdlg_setNextText, CSTRING, text, NAME, method, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( requiredOS(context, "setNextText", "Vista", Vista_OS) && pcpsd->isAeroWiz )
    {
        LPWSTR buttonText = ansi2unicode(text);
        if ( buttonText != NULL )
        {
            PropSheet_SetNextText(pcpsd->hDlg, buttonText);
            LocalFree(buttonText);
        }
        return TheTrueObj;
    }

    return TheFalseObj;
}


/** PropertySheetDialog::setTitle()
 *
 *  Sets the title for a property sheet dialog.
 *
 *  @param  title             The title for the property sheet.
 *
 *  @param  addPropertiesFor  [OPTIONAL] If true the text "Properties for" is
 *                            prefixed to the title.  If omitted or false, there
 *                            is no text added.
 *
 *  @return  Zero, always.
 *
 *  @remarks  The MSDN documentation seems to indicate that this is valid for
 *            wizards, but experimentation shows it does not work for any
 *            wizard. In addition, newe MSD documentation explicitly states
 *            that: In an Aero Wizard, this macro can be used to change the
 *            title of an interior page dynamically for example, when handling
 *            the PSN_SETACTIVE notification.
 *
 *            However, it simply does not work.  The code for an aero wizard is
 *            left in, but it does not seem to work.
 */
RexxMethod3(RexxObjectPtr, psdlg_setTitle, CSTRING, title, OPTIONAL_logical_t, addPropertiesFor, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( pcpsd->isAeroWiz )
    {
        LPWSTR newText = ansi2unicode(title);
        if ( newText != NULL )
        {
            PropSheet_SetTitle(pcpsd->hDlg, addPropertiesFor, newText);
            LocalFree(newText);
        }
    }
    else
    {
        PropSheet_SetTitle(pcpsd->hDlg, addPropertiesFor, title);
    }

    return TheZeroObj;
}


/** PropertySheetDialog::setWizButtons()
 *
 *  Enables or disables the Back, Next, and Finish buttons in a wizard.
 *
 *  @param  opts  Keyword(s) that control which buttons are enabled or disabled.
 *
 *  @return  True this property sheet is a wizard, otherwise false.
 *
 *  @notes  Wizards display either three or four buttons below each page. This
 *          method is used to specify which buttons are enabled. Wizards
 *          normally display Back, Cancel, and either a Next or Finish button.
 *          Typically, enable only the Next button for the welcome page, Next
 *          and Back for interior pages, and Back and Finish for the completion
 *          page.  The Cancel button is always enabled.  Normally, setting
 *          FINISH or DISABLEDFINISH replaces the Next button with a Finish
 *          button.  To display Next and Finish buttons simultaneously, set the
 *          WIZARDHASFINISH keyword in the options when the PropertySheetDialog
 *          is instantiated.  Every page will then display all four buttons.
 *
 *          If this property sheet is not a Wizard, this method has no effect.
 *
 *  @remarks  We do not enforce that this is only called for a Wizard, although
 *            maybe we should.
 *
 *            The prop sheet marco does not return a value.
 */
RexxMethod2(RexxObjectPtr, psdlg_setWizButtons, OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    if ( pcpsd->isNotWizard )
    {
        return TheFalseObj;
    }

    uint32_t flags = 0;

    if ( argumentExists(1) )
    {
        if ( StrStrI(opts, "BACK")           != NULL ) flags |= PSWIZB_BACK;
        if ( StrStrI(opts, "NEXT")           != NULL ) flags |= PSWIZB_NEXT;
        if ( StrStrI(opts, "FINISH")         != NULL ) flags |= PSWIZB_FINISH;
        if ( StrStrI(opts, "DISABLEDFINISH") != NULL ) flags |= PSWIZB_DISABLEDFINISH;
    }

    PropSheet_SetWizButtons(pcpsd->hDlg, flags);
    return TheTrueObj;
}


/** PropertySheetDialog::showWizButtons()
 *
 *  Show or hide buttons in an Aero wizard.
 *
 *  @param  opts       Zero or more of the keyword values that specify which
 *                     property sheet buttons are to be shown. If a button value
 *                     is included in both this argument and the optsButtons
 *                     argument, then the button is shown.
 *
 *  @param optsButtons Zero or more of the same keywords used in opts. Here,
 *                     they specify which property sheet buttons are to be shown
 *                     or hidden. If a keyword appears in this argument but not
 *                     in opts, it indicates that the button should be hidden.
 *
 *  @param  Returns true if this is an Aero Wizard property sheet on Vista on
 *          later, otherwise false.
 *
 *  @notes  Thise method only works for Aero Wizards.  This method requires
 *          Vista or later, a condition is raised if the OS is not Vista or
 *          later. This method has no effect if the property sheet is not an
 *          Aero Wizard.
 *
 *          The order of showWizButtons() and setWizButtons() is important.
 *          This works:
 *
 *            propSheet~setWizButtons("NEXT")
 *            propSheet~showWizButtons("NEXT", "BACK NEXT")
 *
 *          This does not work:
 *
 *            propSheet~showWizButtons("NEXT", "BACK NEXT")
 *            propSheet~setWizButtons("NEXT")
 */
RexxMethod3(RexxObjectPtr, psdlg_showWizButtons, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, optsButtons, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( requiredOS(context, "showWizButtons", "Vista", Vista_OS) && pcpsd->isAeroWiz )
    {
        uint32_t flags = 0;
        uint32_t buttons = 0;

        if ( argumentExists(1) )
        {
            if ( StrStrI(opts, "BACK")   != NULL ) flags |= PSWIZB_BACK;
            if ( StrStrI(opts, "NEXT")   != NULL ) flags |= PSWIZB_NEXT;
            if ( StrStrI(opts, "FINISH") != NULL ) flags |= PSWIZB_FINISH;
            if ( StrStrI(opts, "CANCEL") != NULL ) flags |= PSWIZB_CANCEL;
        }

        if ( argumentExists(2) )
        {
            if ( StrStrI(optsButtons, "BACK")   != NULL ) buttons |= PSWIZB_BACK;
            if ( StrStrI(optsButtons, "NEXT")   != NULL ) buttons |= PSWIZB_NEXT;
            if ( StrStrI(optsButtons, "FINISH") != NULL ) buttons |= PSWIZB_FINISH;
            if ( StrStrI(optsButtons, "CANCEL") != NULL ) buttons |= PSWIZB_CANCEL;
        }

        PropSheet_ShowWizButtons(pcpsd->hDlg, flags, buttons);

        return TheTrueObj;
    }

    return TheFalseObj;
}


/** PropertySheetDialog::unchanged()
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_unchanged, RexxObjectPtr, _page, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    HWND page = getValidPageHwnd(context, pcpsd, _page, 1);
    if ( page != NULL )
    {
        PropSheet_UnChanged(pcpsd->hDlg, page);
    }
    return TheZeroObj;
}


/** PropertySheetDialog::test()
 *
 *  Testing method
 */
RexxMethod1(RexxObjectPtr, psdlg_test, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    printf("PropertySheetDialog pcpsd->hDlg=%p pcpsd->pcpbd->hDlg=%p\n", pcpsd->hDlg, pcpsd->pcpbd->hDlg);
    printf("No test set up at this time\n");
    printf("Make version for 6.1=%d\n", MAKEVERSION(6, 1));
    printf("Make version for 6.01=%d\n", MAKEVERSION(6, 01));
    return TheZeroObj;
}


/**
 *  Methods for the .PropertySheetPage mixin class.
 */
#define PROPERTYSHEETPAGE_CLASS  "PropertySheetPage"


/**
 * Ensures that a dialog CSelf pointer is not null.  This can happen if the user
 * does something wrong in their use of the PropertySheetPag class so that the
 * proper initialization is bypassed.
 *
 * Unfortunately users do this. ;-(  Before the conversion to the C++ API,
 * things whould not work as the user expected, but no "real bad" things
 * happened.  Now, this causes a null point derefernce unless we check first.
 *
 * @param c       Method context we are operating.
 * @param pCSelf  CSelf pointer for the property sheet page object.
 *
 * @return The CSelf pointer cast to a pCPropertySheetPage, or null if pCSelf is
 *         null.
 *
 * @note  The whole point of this is to raise the exception if pCSelf is null.
 */
static inline pCPropertySheetPage getPSPCSelf(RexxMethodContext *c, void * pCSelf)
{
    if ( pCSelf == NULL )
    {
        baseClassInitializationException(c, "PropertySheetPage");
    }
    return (pCPropertySheetPage)pCSelf;
}

/**
 * Performs the initialization of the PropertySheetPage mixin class.
 *
 * We create the CSelf struct for the class and then send the struct to the
 * init_propertySheetPage() method.  That method will raise an exception if its
 * arg is not a RexxBufferObject.  This implies that the PropertySheetPage mixin
 * class can only be initialized through the native API.
 *
 * The plain base dialog, dialog control, and window classes in ooDialog inherit
 * WindowBase.
 *
 * @param c        Method context we are operating in.
 *
 * @param
 *
 * @param self     The Rexx object that inherited WindowBase.
 *
 * @return True on success, otherwise false.  If false an exception has been
 *         raised.
 *
 * @remarks
 */
bool initPropertySheetPage(RexxMethodContext *c, pCPlainBaseDialog pcpbd, pCDynamicDialog pcdd,
                           RexxObjectPtr self)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CPropertySheetPage));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCPropertySheetPage pcpsp = (pCPropertySheetPage)c->BufferData(obj);
    memset(pcpsp, 0, sizeof(CPropertySheetPage));

    pcpsp->pcpbd         = pcpbd;
    pcpsp->pcdd          = pcdd;
    pcpsp->rexxPropSheet = TheNilObj;
    pcpsp->rexxSelf      = self;
    pcpsp->interpreter   = pcpbd->interpreter;
    pcpsp->extraOpts     = c->NullString();

    pcpbd->dlgPrivate = pcpsp;

    c->SendMessage1(self, "INIT_PROPERTYSHEETPAGE", obj);

    return true;
}


static void parsePageOpts(RexxMethodContext *c, pCPropertySheetPage pcpsp, CSTRING options)
{
    uint32_t opts = PSP_DEFAULT | PSP_USECALLBACK;

    if ( pcpsp->pageType != oodResPSPDialog )
    {
        pcpsp->cx = PROP_MED_CXDLG;
        pcpsp->cy = PROP_MED_CYDLG;
        if ( options != NULL )
        {
            if ( StrStrI(options, "LARGE") )
            {
                pcpsp->cx = PROP_LG_CXDLG;
                pcpsp->cy = PROP_LG_CYDLG;
            }
            else if ( StrStrI(options, "SMALL") )
            {
                pcpsp->cx = PROP_SM_CXDLG;
                pcpsp->cy = PROP_SM_CYDLG;
            }
        }
    }

    if ( options != NULL )
    {
        if ( StrStrI(options, "AEROPAGE")          != NULL ) pcpsp->isAeroWizardPage = true;

        if ( StrStrI(options, "USETITLE")          != NULL ) opts |= PSP_USETITLE;
        if ( StrStrI(options, "RTLREADING")        != NULL ) opts |= PSP_RTLREADING;
        if ( StrStrI(options, "HASHELP")           != NULL ) opts |= PSP_HASHELP;
        if ( StrStrI(options, "USEREFPARENT")      != NULL ) opts |= PSP_USEREFPARENT;
        if ( StrStrI(options, "PREMATURE")         != NULL ) opts |= PSP_PREMATURE;
        if ( StrStrI(options, "HIDEHEADER")        != NULL ) opts |= PSP_HIDEHEADER;
        if ( StrStrI(options, "USEHEADERTITLE")    != NULL ) opts |= PSP_USEHEADERTITLE;
        if ( StrStrI(options, "USEHEADERSUBTITLE") != NULL ) opts |= PSP_USEHEADERSUBTITLE;
        if ( StrStrI(options, "USEFUSIONCONTEXT")  != NULL ) opts |= PSP_USEFUSIONCONTEXT;
    }
    pcpsp->pageFlags = opts;
}

bool initPageDlgFrame(RexxThreadContext *c, pPageDialogInfo ppdi)
{
    pCPlainBaseDialog pcpbd = ppdi->pcpbd;
    pCDynamicDialog pcdd = ppdi->pcdd;

    if ( ppdi->pageTitle == NULL )
    {
        char *t = (char *)LocalAlloc(LPTR, 32);
        if ( t == NULL )
        {
            outOfMemoryException(c);
            return false;
        }

        _snprintf(t, 32, "Page %d", ppdi->pageNumber + 1);
        ppdi->newTitle = t;
    }

    pcpbd->wndBase->sizeX = ppdi->cx;
    pcpbd->wndBase->sizeY = ppdi->cy;

    // MSDN docs say that if a property sheet page dialog has the DS_SHELLFONT
    // style, the property sheet page manager will size the dialog according to
    // the font specified in the template.  Whereas, the doc also says,
    // concerning dialogs in general, that DS_SHELLFONT no effect if the
    // typeface is not MS Shell Dlg.  Implying that you need to use DS_SETFONT
    // if the font specified is other than MS Shell Dlg.  So here we always use
    // DS_SHELLFONT for property sheet page dialogs and DS_SETFONT for control
    // dialogs.  However, tests seem to show that DS_SHELLFONT would be fine for
    // all cases.

    uint32_t style = DS_3DLOOK | DS_CONTROL | WS_CHILD | WS_TABSTOP;
    if ( ppdi->pageType == oodUserPSPDialog || ppdi->pageType == oodRcPSPDialog )
    {
        style |= DS_SHELLFONT;
    }
    else
    {
        style |= DS_SETFONT;
    }

    DLGTEMPLATEEX *pBase;

    // TODO Need to allow / see what makes sense for extended window styles
    uint32_t exStyle = 0;

    return startDialogTemplate(c, &pBase, pcdd, 0, 0, ppdi->cx, ppdi->cy, NULL, "", pcpbd->fontName, pcpbd->fontSize,
                               style, exStyle);
}

/**
 * Creates the in-memory dialog template for a user property sheet page or a
 * managed tab user control dialog.
 *
 * @param c      Method context we are operating in.
 * @param pcpsp  Pointer to a page dialog information struct.  This struct is
 *               filled in by the caller.
 *
 * @return True on success false on error.
 *
 * @remarks  For a user defined dialog template we always use "" for the dialog
 *           title in the template.  If the user has not assigned a page title,
 *           we create one for him.  When the
 *           see that the pageTitle field is not null and add the PSP
 */
RexxObjectPtr initUserTemplate(RexxThreadContext *c, pPageDialogInfo ppdi)
{
    pCPlainBaseDialog pcpbd = ppdi->pcpbd;

    if ( ! initPageDlgFrame(c, ppdi) )
    {
        goto err_out;
    }

    pCDynamicDialog pcdd = ppdi->pcdd;

    RexxObjectPtr result = c->SendMessage0(ppdi->rexxSelf, "DEFINEDIALOG");

    if ( ! c->CheckCondition() && pcdd->active != NULL )
    {
        ppdi->pageID = (intptr_t)pcdd->base;

        // Set the number of dialog items field in the dialog template.
        ((DLGTEMPLATEEX *)pcdd->base)->cDlgItems = (WORD)pcdd->count;

        return TheTrueObj;
    }

err_out:
    // No underlying windows dialog can be created.  We still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  But I'm not sure it needs to be done here.
    stopDialog(pcpbd, c);
    return TheFalseObj;
}

RexxObjectPtr initRcTemplate(RexxThreadContext *c, pPageDialogInfo ppdi)
{
    pCPlainBaseDialog pcpbd = ppdi->pcpbd;
    pCDynamicDialog pcdd = ppdi->pcdd;

    CSTRING msg = (ppdi->pageType == oodRcPSPDialog) ? "LOADFRAME" : "LOADFRAMERCCD";
    RexxObjectPtr result = c->SendMessage1(ppdi->rexxSelf, msg, c->UnsignedInt32(pcdd->expected));

    // Checking that result is 1 is probably sufficient.
    if ( ! isInt(0, result, c) || c->CheckCondition() )
    {
        goto err_out;
    }

    // For a RcPSPDialog or a RcControlDialog the caller did not know the
    // correct cx, cy, and probably title because it is determined by parsing
    // the .rc file in loadFrame().  So, we set it now.
    if ( ppdi->pageType == oodRcPSPDialog )
    {
        pCPropertySheetPage pcpsp = (pCPropertySheetPage)ppdi->pPageCSelf;

        ppdi->cx = pcpsp->cx;
        ppdi->cy = pcpsp->cy;
        ppdi->pageTitle = pcpsp->pageTitle;
    }
    else
    {
        pCControlDialog pccd = (pCControlDialog)ppdi->pPageCSelf;

        ppdi->cx = pccd->size.cx;
        ppdi->cy = pccd->size.cy;
        ppdi->pageTitle = pccd->pageTitle;
    }

    if ( ! initPageDlgFrame(c, ppdi) )
    {
        goto err_out;
    }

    msg = (ppdi->pageType == oodRcPSPDialog) ? "LOADITEMS" : "LOADITEMSRCCD";
    result = c->SendMessage1(ppdi->rexxSelf, msg, ppdi->extraOpts);

    if ( ! isInt(0, result, c) || pcdd->active == NULL )
    {
        goto err_out;
    }

    result = c->SendMessage0(ppdi->rexxSelf, "DEFINEDIALOG");

    if ( ! c->CheckCondition() && pcdd->active != NULL )
    {
        ppdi->pageID = (intptr_t)pcdd->base;

        // Set the number of dialog items field in the dialog template.
        ((DLGTEMPLATEEX *)pcdd->base)->cDlgItems = (WORD)pcdd->count;

        return TheTrueObj;
    }

err_out:
    // No underlying windows dialog can be created.  We still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  But I'm not sure it needs to be done here.
    stopDialog(pcpbd, c);
    return TheFalseObj;
}

RexxObjectPtr initResTemplate(RexxThreadContext *c, pPageDialogInfo ppdi)
{
    pCPlainBaseDialog pcpbd = ppdi->pcpbd;

    if ( ! loadResourceDLL(pcpbd, pcpbd->library) )
    {
        goto err_out;
    }
    ppdi->pageID = (intptr_t)ppdi->resID;

    return TheTrueObj;

err_out:
    // No underlying windows dialog can be created.  We still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  But I'm not sure it needs to be done here.
    stopDialog(pcpbd, c);
    return TheFalseObj;
}

/** PropertySheetPage::init()               [Class method]
 *
 *  Used to capture the PropertySheetPage class object.  This is used for scoped
 *  look ups of the CSelf.
 */
RexxMethod1(RexxObjectPtr, psp_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, PROPERTYSHEETPAGE_CLASS) )
    {
        ThePropertySheetPageClass = (RexxClassObject)self;
    }
    return NULLOBJECT;
}


/** PropertySheetPage::cx                   [Attribute get]
 *  PropertySheetPage::cy                   [Attribute get]
 *
 */
RexxMethod2(uint32_t, psp_getcx, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    return *(name + 1) == 'X' ? pcpsp->cx : pcpsp->cy;
}


/** PropertySheetPage::cx                   [Attribute set]
 *  PropertySheetPage::cy                   [Attribute set]
 *
 */
RexxMethod3(RexxObjectPtr, psp_setcx, uint32_t, dlgUnit, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    *(name + 1) == 'X' ? pcpsp->cx = dlgUnit : pcpsp->cy = dlgUnit;
    return NULLOBJECT;
}


/** PropertySheetPage::pageID()             [Attribute get]
 *
 */
RexxMethod1(POINTER, psp_pageID_atr, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    return (POINTER)pcpsp->pageID;
}


/** PropertySheetPage::pageNumber()          [Attribute get]
 *
 */
RexxMethod1(uint32_t, psp_pageNumber_atr, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return 0;
    }

    return pcpsp->pageNumber + 1;
}


/** PropertySheetPage::pageTitle()          [Attribute get]
 *  PropertySheetPage::headerTitle()
 *  PropertySheetPage::headerSubTitle()
 */
RexxMethod2(RexxObjectPtr, psp_getPageTitle, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    switch ( *(name + 7) )
    {
        case 'L' :
            return pcpsp->pageTitle == NULL ? TheNilObj : context->String(pcpsp->pageTitle);
        case 'I' :
            return pcpsp->headerTitle == NULL ? TheNilObj : context->String(pcpsp->headerTitle);
    }
    return pcpsp->headerSubTitle == NULL ? TheNilObj : context->String(pcpsp->headerSubTitle);
}


/** PropertySheetPage::pageTitle()          [Attribute set]
 *  PropertySheetPage::headerTitle()        [Attribute set]
 *  PropertySheetPage::headerSubTitle()     [Attribute set]
 */
RexxMethod3(RexxObjectPtr, psp_setPageTitle, CSTRING, text, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    switch ( *(name + 7) )
    {
        case 'L' :
            setPageText(context, pcpsp, text, pageText);
            break;

        case 'I' :
            setPageText(context, pcpsp, text, headerText);
            break;

        case 'U' :
            setPageText(context, pcpsp, text, headerSubtext);
            break;
    }
    return NULLOBJECT;
}


/** PropertySheetPage::propSheet()          [Attribute get]
 *
 */
RexxMethod1(RexxObjectPtr, psp_propSheet_atr, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    return pcpsp->rexxPropSheet;
}


/** PropertySheetPage::resources()          [Attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, psp_setResources_atr, RexxObjectPtr, resourceImage, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    PRESOURCEIMAGE ri = rxGetResourceImage(context, resourceImage, 1);
    if ( ri != NULL )
    {
        pcpsp->hInstance = ri->hMod;
        context->SetObjectVariable("RESOURCES", resourceImage);
    }
    return NULLOBJECT;
}

/** PropertySheetPage::tabIcon()            [Attribute set]
 *
 *  Sets the icon used for the tab.  The user can specify the icon as either a
 *  resource ID (numeric or symbolic) or as an .Image object.
 *
 *  @remarks  If the user specifies the icon as an .Image object, then it has to
 *            be an icon image, not some other type of image, like a bitmap,
 *            etc.  The rxGetImageIcon() call will raise an exception if the
 *            image is not an icon.
 */
RexxMethod2(RexxObjectPtr, psp_setTabIcon_atr, RexxObjectPtr, icon, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    bool    isImage;
    uint8_t type;

    intptr_t result = getImageOrID(context, pcpsp->rexxSelf, icon, 1, &isImage, &type);
    if ( result != 0 )
    {
        if ( isImage )
        {
            if ( type != IMAGE_ICON )
            {
                wrongArgValueException(context->threadContext, 1, "Icon, Cursor", getImageTypeName(type));
                goto done_out;
            }
            pcpsp->hIcon = (HICON)result;
        }
        else
        {
            pcpsp->iconID = (uint32_t)result;
        }

        context->SetObjectVariable("TABICON", icon);
    }

done_out:
    return NULLOBJECT;
}


/** PropertySheetPage::wantAccelerators()   [Attribute get]
/** PropertySheetPage::wantGetObject()      [Attribute get]
 *
 */
RexxMethod2(RexxObjectPtr, psp_getWantNotification, NAME, methName, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr result = TheFalseObj;

    if ( *(methName + 4) == 'A' )
    {
        result = pcpsp->wantAccelerators ? TheTrueObj : TheFalseObj;
    }
    else
    {
        result = pcpsp->wantGetObject ? TheTrueObj : TheFalseObj;
    }
    return result;
}


/** PropertySheetPage::wantAccelerators()   [Attribute set]
/** PropertySheetPage::wantGetObject()      [Attribute set]
 *
 */
RexxMethod3(RexxObjectPtr, psp_setWantNotification, logical_t, want, NAME, methName, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    if ( *(methName + 4) == 'A' )
    {
        pcpsp->wantAccelerators = want ? true : false;
    }
    else
    {
        pcpsp->wantGetObject = want ? true : false;
    }
    return NULLOBJECT;
}


/** PropertySheetPage::wasActivated()       [Attribute get]
 *
 */
RexxMethod1(RexxObjectPtr, psp_wasActivated_atr, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    return pcpsp->activated ? TheTrueObj : TheFalseObj;
}


/** PropertySheetPage::init_propertySheetPage()
 *
 *  The property sheet page initialization.  This can only be done from native
 *  code.
 *
 *  @param cSelf  The Rexx Buffer object containing the CSelf for a property
 *                sheet page.
 */
RexxMethod1(logical_t, psp_init_propertySheetPage, RexxObjectPtr, cSelf)
{
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
}


/** PropertySheetPage::initTemplate()
 *
 *  @remarks  Users have been known to screw up the initialization of the
 *            property sheet page dialog and we could get here with pCSelf not
 *            really a valid pCPropertySheetPage struct.  Or at least not a
 *            properly filled in one.  switch ( pcpsp->pageType ) will detect
 *            this.
 */
RexxMethod1(RexxObjectPtr, psp_initTemplate, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    PageDialogInfo pdi = {0};
    pdi.pageTitle  = pcpsp->pageTitle;
    pdi.pageType   = pcpsp->pageType;
    pdi.pageNumber = pcpsp->pageNumber;
    pdi.pcdd       = pcpsp->pcdd;
    pdi.pcpbd      = pcpsp->pcpbd;
    pdi.rexxSelf   = pcpsp->rexxSelf;
    pdi.extraOpts  = pcpsp->extraOpts;
    pdi.resID      = pcpsp->resID;
    pdi.cx         = pcpsp->cx;
    pdi.cy         = pcpsp->cy;
    pdi.pPageCSelf = pcpsp;

    RexxObjectPtr result = TheFalseObj;

    switch ( pcpsp->pageType )
    {
        case oodUserPSPDialog :
            result = initUserTemplate(context->threadContext, &pdi);
            break;

        case oodRcPSPDialog :
            result = initRcTemplate(context->threadContext, &pdi);
            break;

        case oodResPSPDialog :
            result = initResTemplate(context->threadContext, &pdi);
            break;

        default :
            baseClassInitializationException(context, "PropertySheetPage");
            break;
    }

    if ( result == TheTrueObj )
    {
        pcpsp->pageID = pdi.pageID;

        if ( pdi.newTitle != NULL && ! (pcpsp->pageFlags & PSP_USETITLE) )
        {
            if ( ! setPageText(context, pcpsp, pdi.newTitle, pageText) )
            {
                result = TheFalseObj;
            }
        }
    }

    safeLocalFree(pdi.newTitle);

    return result;
}


/** PropertySheetPage::setSize()
 *
 *
 */
RexxMethod2(RexxObjectPtr, psp_setSize, ARGLIST, args, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = getPSPCSelf(context, pCSelf);
    if ( pcpsp == NULL )
    {
        return NULLOBJECT;
    }

    size_t arraySize;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 2, &arraySize, &argsUsed) )
    {
        return TheOneObj;
    }

    pcpsp->cx = point.x;
    pcpsp->cy = point.y;

    return TheZeroObj;
}



/**
 *  Methods for the .UserPSPDialog class.
 */
#define USERPSPDIALOG_CLASS  "UserPSPDialog"


/** UserPSPDialog::init()
 *
 *  The init() method of the super class, UserDialog, returns the zero object,
 *  so we check for that after forwarding to the super class init().
 *
 */
RexxMethod9(RexxObjectPtr, userpspdlg_init, OPTIONAL_RexxObjectPtr, dlgData, OPTIONAL_RexxObjectPtr, includeFile,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, title, OPTIONAL_CSTRING, fontName, OPTIONAL_uint32_t, fontSize,
            OPTIONAL_uint32_t, expected, SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(2);

    if ( argumentExists(1) )
    {
        context->ArrayPut(newArgs, dlgData, 1);
    }
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, includeFile, 2);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);
    if (result == TheZeroObj )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        pCDynamicDialog pcdd = (pCDynamicDialog)context->ObjectToCSelf(self, TheDynamicDialogClass);

        // We have a 0 result.  We set result back to an error code and then
        // reset it to 0 if we get through all the error checks
        result = TheOneObj;
        pcpbd->wndBase->initCode = 1;

        if ( ! initPropertySheetPage(context, pcpbd, pcdd, self) )
        {
            goto done_out;
        }

        pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;
        pcpsp->pageType = oodUserPSPDialog;

        parsePageOpts(context, pcpsp, opts);

        pcdd->expected = (expected == 0 ? DEFAULT_EXPECTED_DIALOG_ITEMS : expected);
        pcpbd->isPageDlg = true;

        if ( argumentExists(4) )
        {
            if ( ! setPageText(context, pcpsp, title, pageText) )
            {
                goto done_out;
            }
        }
        if ( argumentExists(5) )
        {
            if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
            {
                stringTooLongException(context->threadContext, 4, MAX_DEFAULT_FONTNAME, strlen(fontName));
                goto done_out;
            }
            strcpy(pcpbd->fontName, fontName);
        }
        if ( argumentExists(6) )
        {
            if ( fontSize == 0 )
            {
                context->RaiseException2(Rexx_Error_Invalid_argument_positive, context->WholeNumber(5), TheZeroObj);
                goto done_out;
            }
            pcpbd->fontSize = fontSize;
        }
        result = TheZeroObj;
        pcpbd->wndBase->initCode = 0;
    }
    else
    {
        // Let syntax conditions trickle through ...
        if ( ! context->CheckCondition() )
        {
            baseClassInitializationException(context, USERPSPDIALOG_CLASS);
        }
    }

done_out:
    return result;
}


/**
 *  Methods for the .RcPSPDialog class.
 */
#define RCPSPDIALOG_CLASS  "RcPSPDialog"


RexxMethod9(RexxObjectPtr, rcpspdlg_init, RexxStringObject, scriptFile, RexxObjectPtr, resource, OPTIONAL_RexxObjectPtr, dlgData,
            OPTIONAL_RexxObjectPtr, includeFile, OPTIONAL_CSTRING, pageOpts, OPTIONAL_RexxStringObject, connectOpts,
            OPTIONAL_uint32_t, expected, SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, scriptFile, 3);
    context->ArrayPut(newArgs, resource, 4);
    if ( argumentExists(3) )
    {
        context->ArrayPut(newArgs, dlgData, 1);
    }
    if ( argumentExists(4) )
    {
        context->ArrayPut(newArgs, includeFile, 2);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( result == TheZeroObj )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        pCDynamicDialog pcdd = (pCDynamicDialog)context->ObjectToCSelf(self, TheDynamicDialogClass);

        if ( ! initPropertySheetPage(context, pcpbd, pcdd, self) )
        {
            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }

        pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;
        pcpsp->pageType = oodRcPSPDialog;

        parsePageOpts(context, pcpsp, pageOpts);

        pcdd->expected = (expected == 0 ? DEFAULT_EXPECTED_DIALOG_ITEMS : expected);
        pcpbd->isPageDlg = true;

        if ( argumentExists(6) )
        {
            pcpsp->extraOpts = connectOpts;
        }
    }
    else
    {
        // Let syntax conditions trickle through ...
        if ( ! context->CheckCondition() )
        {
            baseClassInitializationException(context, RCPSPDIALOG_CLASS);
        }
    }

done_out:
    return result;
}


RexxMethod7(RexxObjectPtr, rcpspdlg_startTemplate, uint32_t, cx, uint32_t, cy, CSTRING, title, CSTRING, fontName,
            uint32_t, fontSize, uint32_t, expected, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;
    pCDynamicDialog pcdd = pcpsp->pcdd;

    pcpsp->cx = cx;
    pcpsp->cy = cy;

    // We only want to change the title if PSP_USETITLE is not set.
    if ( strlen(title) > 0 && ! (pcpsp->pageFlags & PSP_USETITLE) )
    {
        if ( ! setPageText(context, pcpsp, title, pageText) )
        {
           return TheOneObj;
        }
    }

    size_t len = strlen(fontName);

    if ( len > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 4, MAX_DEFAULT_FONTNAME, len);
        return TheOneObj;
    }
    if ( len > 0 )
    {
        strcpy(pcpbd->fontName, fontName);
    }

    if ( fontSize != 0 )
    {
        pcpbd->fontSize = fontSize;
    }
    pcdd->expected = expected;

    return TheZeroObj;
}


/**
 *  Methods for the .ResPSPDialog class.
 */
#define RESPSPDIALOG_CLASS  "ResPSPDialog"


RexxMethod7(RexxObjectPtr, respspdlg_init, RexxStringObject, dllFile, RexxObjectPtr, resourceID, OPTIONAL_RexxObjectPtr, dlgData,
            OPTIONAL_RexxObjectPtr, includeFile, OPTIONAL_CSTRING, pageOpts, SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, dllFile, 1);
    context->ArrayPut(newArgs, resourceID, 2);
    if ( argumentExists(3) )
    {
        context->ArrayPut(newArgs, dlgData, 3);
    }
    if ( argumentExists(4) )
    {
        context->ArrayPut(newArgs, includeFile, 4);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( result == TheZeroObj )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();

        int32_t resID = oodResolveSymbolicID(context->threadContext, self, resourceID, -1, 2, true);
        if ( resID == OOD_ID_EXCEPTION )
        {
            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }

        if ( ! initPropertySheetPage(context, pcpbd, NULL, self) )
        {
            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }

        pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;

        pcpsp->pageType = oodResPSPDialog;
        pcpsp->resID = resID;

        parsePageOpts(context, pcpsp, pageOpts);

        pcpbd->isPageDlg = true;
    }
    else
    {
        // Let syntax conditions trickle through ...
        if ( ! context->CheckCondition() )
        {
            baseClassInitializationException(context, RESPSPDIALOG_CLASS);
        }
    }

done_out:
    return result;
}


/**
 *  Methods for the .TabOwnerDialog class.
 */
#define TABOWNERDIALOG_CLASS          "TabOwnerDialog"
#define TOD_MANAGEDTABBAG_ATTRIBUTE   "TabOwnerDialogManagedTabBag"


static inline pCTabOwnerDialog validateTodCSelf(RexxMethodContext *c, void *pCSelf)
{
    pCTabOwnerDialog pctod = (pCTabOwnerDialog)pCSelf;
    if ( pctod == NULL )
    {
        baseClassInitializationException(c);
    }
    return pctod;
}

static void todAdjustMethods(RexxMethodContext *c, RexxObjectPtr self)
{
    char *mSrc = "self~tabOwnerOk\n";
    RexxMethodObject m = c->NewMethod("OK", mSrc, strlen(mSrc));
    c->SendMessage2(self, "SETMETHOD", c->String("OK"), m);

    mSrc = "self~tabOwnerCancel\n";
    m = c->NewMethod("CANCEL", mSrc, strlen(mSrc));
    c->SendMessage2(self, "SETMETHOD", c->String("CANCEL"), m);
}

static RexxObjectPtr findRexxPage(pCTabOwnerDialog pctod, int32_t id, uint32_t pageIndex)
{
    for ( uint32_t i = 0; i < pctod->countMTs; i++ )
    {
        if ( id == pctod->tabIDs[i] )
        {
            pCManagedTab pmt = pctod->mts[i];
            if ( pageIndex < pmt->count )
            {
                return pmt->rexxPages[pageIndex];
            }

            // We found the right managed tab, but the passed in pageIndex is no
            // good, No sense in looking any further.
            break;
        }
    }
    return NULLOBJECT;
}

/* Temp function. */
static inline char *tcn2str(LPARAM lParam)
{
    switch ( ((NMHDR *)lParam)->code )
    {
        case TCN_FOCUSCHANGE :    return "TCN_FOCUSCHANGE";
        case TCN_GETOBJECT :      return "TCN_GETOBJECT";
        case TCN_KEYDOWN :        return "TCN_KEYDOWN";
        case TCN_SELCHANGE :      return "TCN_SELCHANGE";
        case TCN_SELCHANGING :    return "TCN_SELCHANGING";
        case NM_CLICK :           return "NM_CLICK";
        case NM_DBLCLK :          return "NM_DBLCLK";
        case NM_RCLICK :          return "NM_RCLICK";
        case NM_RDBLCLK :         return "NM_RDBLCLK";
        case NM_RELEASEDCAPTURE : return "NM_RELEASEDCAPTURE";
        default : return "Not a tab control notification";
    }
}

/**
 * Filter for our tab control notification messages.
 *
 * @param pctod   Pointer to the tab owner CSelf.
 * @param uMsg    The windowm message id.
 * @param lParam  The LPARAM argument for the message.
 * @param index   Index to the managed tab CSelf if found.  Returned.
 *
 * @return Return true if the message is a tab control notification message for
 *         one of our managed tabs, otherwise false.
 */
inline bool isTCNMsg(pCTabOwnerDialog pctod, uint32_t uMsg, LPARAM lParam, uint32_t *index)
{
    if ( uMsg == WM_NOTIFY )
    {
        UINT_PTR id = ((NMHDR *)lParam)->idFrom;

        // First see if the control ID matches one of our managed tabs.
        uint32_t count = pctod->countMTs;
        register uint32_t i = 0;

        while ( i < count && pctod->mts[i]->tabID != id )
        {
            i++;
        }

        if ( i >= count )
        {
            return false;
        }
        *index = i;

        uint32_t code = ((NMHDR *)lParam)->code;
        if ( code >= TCN_LAST && code <= TCN_FIRST )
        {
            return true;
        }

        if ( pctod->mts[i]->wantNotifications )
        {
            if ( code == NM_CLICK || code == NM_DBLCLK || code == NM_RCLICK ||
                 code == NM_RDBLCLK || code == NM_RELEASEDCAPTURE )
            {
                return true;
            }
        }
    }
    return false;
}

void setRexxTab(pCTabOwnerDialog pctod, pCManagedTab pmt)
{
    RexxClassObject ctrlCls = oodClass4controlType(pctod->dlgProcContext, winTab);
    if ( ctrlCls != NULL )
    {
        pmt->rexxTab = createRexxControl(pctod->dlgProcContext, pmt->hTab, pctod->hDlg, pmt->tabID, winTab,
                                         pctod->rexxSelf, ctrlCls, false, true);
    }
    else
    {
        pmt->rexxTab = TheNilObj;
    }
}

/**
 * Sets up fields in the managed tab and page dialogs CSelfs that are not knonw
 * until the underlying tab owner dialog is created.  Adds a tab in the tab
 * control for each page dialog.
 *
 * For example, the Rexx owner is set when the page`dialogs are added to the tab
 * owner, but at that time, the dialog handle is not known.
 *
 * @param pctod
 */
void setMangedTabDetails(pCTabOwnerDialog pctod)
{
    HWND hOwner = pctod->hDlg;
    TCITEM tci;
    tci.mask = TCIF_TEXT;

    for ( uint32_t i = 0; i < pctod->countMTs; i++ )
    {
        pCManagedTab pmt = pctod->mts[i];

        HWND hTab = GetDlgItem(hOwner, pmt->tabID);
        pmt->hTab = hTab;

        setRexxTab(pctod, pmt);

        for ( uint32_t j = 0; j < pmt->count; j++ )
        {
            pCControlDialog pccd = pmt->cppPages[j];

            pccd->pcpbd->hOwnerDlg = hOwner;

            tci.pszText = pccd->pageTitle;
            TabCtrl_InsertItem(hTab, pccd->pageNumber, &tci);
        }
    }
}


/**
 * Returns a pointer to the dialog template in a resource only DLL (for a
 * ResDialog.)
 *
 * @param c
 * @param pccd
 *
 * @return DLGTEMPLATEEX*
 *
 * @remarks  This function follows an example from the MSDN library.  The doc
 *           specifically says pLocked does not need to be freed, but doesn't
 *           say whether it can be freed.  The doc does not say anything about
 *           freeing the other handles, and the example does not free them.
 */
DLGTEMPLATEEX *getDialogResource(RexxThreadContext *c, pCControlDialog pccd)
{
    DLGTEMPLATEEX *pDlg = NULL;
    HRSRC          hResource = NULL;   // Handle to dialog resource in DLL.
    HRSRC          hLoaded = NULL;     // Handle to resource loaded into memory.
    LPVOID         pLocked;            // Pointer to resource locked in memory.

    pCPlainBaseDialog pcpbd = pccd->pcpbd;

    // Find the dialog template resource.
    hResource = FindResource(pcpbd->hInstance, (LPCSTR)(UINT_PTR)pccd->resID, RT_DIALOG); // double cast avoids C4312
    if (hResource == NULL)
    {
        systemServiceExceptionCode(c, API_FAILED_MSG, "FindResource");
        goto done_out;
    }

    // Load the dialog template into global memory.
    hLoaded = (HRSRC)LoadResource(pcpbd->hInstance, hResource);
    if (hLoaded == NULL)
    {
        systemServiceExceptionCode(c, API_FAILED_MSG, "LoadResource");
        goto done_out;
    }

    // Lock the dialog template into global memory.
    pLocked = LockResource(hLoaded);
    if (pLocked == NULL)
    {
        systemServiceExceptionCode(c, API_FAILED_MSG, "LockResource");
        goto done_out;
    }

    pDlg = (DLGTEMPLATEEX *)pLocked;

done_out:
    return pDlg;
}


DLGTEMPLATEEX *getTemplate(RexxThreadContext *c, pCControlDialog pccd)
{
    DLGTEMPLATEEX *pDlg = NULL;

    PageDialogInfo pdi = {0};
    pdi.pageTitle  = pccd->pageTitle;
    pdi.pageType   = pccd->pageType;
    pdi.pageNumber = pccd->pageNumber;
    pdi.pcdd       = pccd->pcdd;
    pdi.pcpbd      = pccd->pcpbd;
    pdi.rexxSelf   = pccd->rexxSelf;
    pdi.extraOpts  = pccd->extraOpts;
    pdi.resID      = pccd->resID;
    pdi.cx         = pccd->size.cx;
    pdi.cy         = pccd->size.cy;
    pdi.pPageCSelf = pccd;

    RexxObjectPtr result = TheFalseObj;

    switch ( pdi.pageType )
    {
        case oodUserControlDialog :
            result = initUserTemplate(c, &pdi);
            break;

        case oodRcControlDialog :
            result = initRcTemplate(c, &pdi);
            break;

        case oodResControlDialog :
            result = initResTemplate(c, &pdi);
            break;

        default :
            break;
    }

    if ( result == TheTrueObj )
    {
        if ( pdi.newTitle != NULL )
        {
            safeLocalFree(pccd->pageTitle);
            pccd->pageTitle = pdi.newTitle;
            pdi.newTitle = NULL;
        }

        if ( pccd->pageType == oodResControlDialog )
        {
            pDlg = getDialogResource(c, pccd);
        }
        else
        {
            pDlg = pdi.pcdd->base;
        }
    }

    /*
     * We could have allocated newTitle, but then had an error.  In that case we
     * need to free newTitle.
     */
    safeLocalFree(pdi.newTitle);

    return pDlg;
}

/**
 *  Create Managed Tab Page dialog.  Creates the control dialog that serves as a
 *  page of tab control in a TabOwner dialog.  This function is executing in the
 *  same thread as the tab owner dialog's window message processing function.
 *
 *
 * @param c
 * @param pccd
 * @param pTemplate
 * @param pcpbdOwner
 *
 * @return HWND
 *
 * @remarks The childDlg thing was used for the CategoryDialog implementation.
 *          We now use it so that an owner dialog can keep track of its owned
 *          control dialogs.  For an owner dialog using control dialogs for
 *          whatever reason, the maximum number of child dialogs, 20, is
 *          probably sufficient.
 *
 *          For managed tab owner dialogs, that maximum is too small because the
 *          max page number is set at 100. Since the managed tab owner
 *          implementation is hazy right now, we just put the first 20 dialogs
 *          in the child dialog array. In the future we may need to rectify
 *          this. We probably should just ignore the childDlg array in this
 *          case.  We are keeping track of the page dialogs in the cppPages
 *          array.
 */
HWND createMTPageDlg(RexxThreadContext *c, pCControlDialog pccd, DLGTEMPLATEEX *pTemplate, pCPlainBaseDialog pcpbdOwner)
{
#ifdef _DEBUG
    printf("Enter createMTPageDlg() dlg=%s\n", c->ObjectToStringValue(pccd->rexxSelf));
#endif

    pCPlainBaseDialog pcpbd = pccd->pcpbd;

    /* Set the thread context and ID because it is not done in RexxChildDlgProc.
     */
    pcpbd->dlgProcContext = c;
    pcpbd->dlgProcThreadID = GetCurrentThreadId();

    HWND hPage = CreateDialogIndirectParam(MyInstance, (LPCDLGTEMPLATE)pTemplate, pcpbd->hOwnerDlg,
                                           (DLGPROC)RexxChildDlgProc, (LPARAM)pcpbd);

    if ( hPage )
    {
        pcpbd->hDlg = hPage;
        pcpbd->isActive = true;
        pccd->activated = true;

        pcpbd->childDlg[0] = hPage;
        if ( pccd->pageNumber < MAXCHILDDIALOGS )
        {
            pcpbdOwner->childDlg[pccd->pageNumber + 1] = hPage;
        }

        setDlgHandle(pcpbd);

        if ( pccd->pageType == oodResControlDialog )
        {
            setFontAttrib(c, pcpbd);

            if ( pcpbd->autoDetect )
            {
                // We ignore any error from doDataAutoDetection().  It could be
                // an out of memory error, in which case an exception is raised.
                // This should sort itself out the next time control is returned
                // to the interpreter (?).
                doDataAutoDetection(c, pcpbd);
            }
        }
        else
        {
            pCDynamicDialog pcdd = pccd->pcdd;
            cleanUpDialogTemplate(pTemplate, pcdd);
        }

        c->SendMessage0(pccd->rexxSelf, "INITDIALOG");
        pccd->isInitializing = false;

        c->SendMessage0(pccd->rexxSelf, "EXECUTE");
    }
    else
    {
        c->SendMessage1(pcpbd->rexxSelf, "FINISHED=", TheFalseObj);
        stopDialog(pcpbd, c);
    }

#ifdef _DEBUG
    printf("Leave createMTPageDlg() dlg=%s\n", c->ObjectToStringValue(pccd->rexxSelf));
#endif
    return hPage;
}


/**
 *
 *
 * @param c
 * @param pccd
 * @param pcmt
 * @param show  If true, size, position and show the dialog.  If false only size
 *              and position.
 *
 * @remarks  TODO need to check that GetWindowRect(), etc., succeed or not.
 *           Should probably return true or false.
 */
void sizeAndPositionPage(RexxThreadContext *c, pCControlDialog pccd, pCManagedTab pcmt, bool show)
{
    HWND hTab  = pcmt->hTab;
    HWND hPage = pccd->pcpbd->hDlg;
    HWND hDlg  = pccd->pcpbd->hOwnerDlg;

    if ( pcmt->needDisplayRect )
    {
        RECT r;

        GetWindowRect(hTab, &r);
        TabCtrl_AdjustRect(hTab, FALSE, &r);

        SIZE s;
        s.cx = r.right - r.left;
        s.cy = r.bottom - r.top;

        POINT p;
        p.x = r.left;
        p.y = r.top;

        ScreenToClient(hDlg, &p);
        pcmt->displayRect.left   = p.x;
        pcmt->displayRect.top    = p.y;
        pcmt->displayRect.right  = s.cx;
        pcmt->displayRect.bottom = s.cy;
        pcmt->needDisplayRect    = false;
    }

    uint32_t flags = show ? SWP_SHOWWINDOW | SWP_NOOWNERZORDER : SWP_HIDEWINDOW | SWP_NOOWNERZORDER;

    SetWindowPos(hPage, hTab, pcmt->displayRect.left, pcmt->displayRect.top, pcmt->displayRect.right,
                 pcmt->displayRect.bottom, flags);

    pccd->isPositioned = true;
    if ( show )
    {
        pcmt->showing = pccd->pageNumber;
    }
}

/**
 * For each managned tab in the tab owner dialog: creates the starting tab page
 * dialog, positions it, and starts the Rexx dialog executing.
 *
 * @param pctod
 *
 * @remarks  Although it does not make a lot of sense, the user is allowed to
 *           have a managed tab without any pages in it.  Pages can be added and
 *           deleted from the managed tab.
 */
void initStartPages(pCTabOwnerDialog pctod)
{
    RexxThreadContext *c = pctod->dlgProcContext;

    for ( uint32_t i = 0; i < pctod->countMTs; i++ )
    {
        pCManagedTab    pcmt = pctod->mts[i];
        pCControlDialog pccd = pcmt->cppPages[pcmt->startPage];

        DLGTEMPLATEEX *pTemplate = getTemplate(c, pccd);
        if ( pTemplate == NULL )
        {
            return;
        }

        HWND hPage = createMTPageDlg(c, pccd, pTemplate, pctod->pcpbd);
        if ( hPage == NULL )
        {
            return;
        }
        sizeAndPositionPage(c, pccd, pcmt, true);

        if ( pcmt->startPage != 0 )
        {
            pcmt->doingStartPage = true;
            TabCtrl_SetCurFocus(pcmt->hTab, pccd->pageNumber);
        }
    }
}

/**
 * Initializes, positions, and shows a tab page dialog that has not yet been
 * activated.
 *
 * @param pctod
 */
void initPage(pCTabOwnerDialog pctod, pCManagedTab pcmt, pCControlDialog pccd)
{
    RexxThreadContext *c = pctod->dlgProcContext;

    DLGTEMPLATEEX *pTemplate = getTemplate(c, pccd);
    if ( pTemplate == NULL )
    {
        return;
    }

    HWND hPage = createMTPageDlg(c, pccd, pTemplate, pctod->pcpbd);
    if ( hPage == NULL )
    {
        return;
    }

    sizeAndPositionPage(c, pccd, pcmt, false);
}

/**
 * Checks the reply to a set active notification.  The programmer can reply 0 to
 * accept the activation, or the page number of another page in the managed tab
 * to veto the activation and set the activation to the other page.
 *
 * @param c
 * @param pcmt
 * @param result
 * @param name
 *
 * @return -1 on error and an exception has been raised.  Or 0, the activation
 *         is accepted, or the one-based index of some other page to set the
 *         focus to.
 */
static int32_t getSetActiveReply(RexxThreadContext *c, pCManagedTab pcmt, RexxObjectPtr result, CSTRING name)
{
    int     max = (int)pcmt->count;
    int32_t index;

    if ( ! c->Int32(result, &index) || (index < 0 || index > max) )
    {
        tcWrongRangeException(c, name, 0, max, result, pcmt->pcpbd);
        return -1;
    }
    return index;
}


static int32_t tcnSetActive(RexxThreadContext *c, pCManagedTab pcmt, pCControlDialog pccdNew)
{
    RexxObjectPtr receiver;
    RexxObjectPtr arg1;
    CSTRING       msgName;
    int32_t       ret;
    RexxObjectPtr owner = pcmt->rexxOwner;

    if ( pcmt->ownerWantsSetActive )
    {
        receiver = owner;
        arg1     = pccdNew->rexxSelf;
        msgName  = TABOWNERSETACTIVE_MSG;
    }
    else
    {
        receiver = pccdNew->rexxSelf;
        arg1     = owner;
        msgName  = TABPAGESETACTIVE_MSG;
    }

    RexxObjectPtr result = c->SendMessage2(receiver, msgName, arg1, pcmt->rexxTab);

    if ( goodReply(c, pcmt->pcpbd, result, msgName) )
    {
        ret = getSetActiveReply(c, pcmt, result, msgName);
        if ( ret == -1 )
        {
            tcCheckForCondition(c, pcmt->pcpbd);
        }
    }
    return ret;
}

static LRESULT doTCNSelChange(pCTabOwnerDialog pctod, pCManagedTab pcmt)
{
    if ( pcmt->doingStartPage )
    {
        pcmt->doingStartPage = false;
        return TRUE;
    }

    int32_t cur  = TabCtrl_GetCurSel(pcmt->hTab);
    if ( cur == -1 )
    {
        // I don't know how this could happen, but it did happen to me during
        // testing.  We will crash if we use -1.  TODO we should try to
        // investigate this, see if it is the best solution, or even if it is
        // possible to get here.
        cur = TabCtrl_GetCurFocus(pcmt->hTab);
        if ( cur == -1 )
        {
            // Give up, although TabCtrl_GetCurFocus is not documented as returning -1.
            return FALSE;
        }
    }

    pCControlDialog    pccdNew = pcmt->cppPages[cur];
    pCControlDialog    pccdOld = pcmt->cppPages[pcmt->showing];
    RexxThreadContext *c       = pctod->dlgProcContext ;

    if ( ! pccdNew->activated )
    {
        // This will position but not show the dialog, because ... the about to
        // be actived dialog could veto it
        initPage(pctod, pcmt, pccdNew);
    }

    int32_t page = tcnSetActive(c, pcmt, pccdNew);
    switch (page)
    {
        case -1 :
            // Error - an exception has been raised.
            break;

        case 0 :
            ShowWindow(pccdOld->pcpbd->hDlg, SW_HIDE);
            ShowWindow(pccdNew->pcpbd->hDlg, SW_SHOW);
            pcmt->showing = cur;

            if ( pcmt->ownerWantsSelChange )
            {
                RexxObjectPtr   tabIndex = c->UnsignedInt32(cur + 1);
                RexxArrayObject args     = c->ArrayOfTwo(pccdNew->rexxSelf, tabIndex);

                invokeDispatch(c, pcmt->pcpbd, TABOWNERSELCHANGE_MSG, args);

                c->ReleaseLocalReference(tabIndex);
                c->ReleaseLocalReference(args);
            }
            break;

        default :
            TabCtrl_SetCurFocus(pcmt->hTab, page - 1);
            break;
    }
    return TRUE;
}

static LRESULT handleTCNMsg(pCTabOwnerDialog pctod, pCPlainBaseDialog pcpbd, uint32_t index, uint32_t uMsg,
                            WPARAM wParam, LPARAM lParam)
{
    pCManagedTab   pcmt   = pctod->mts[index];
    HWND           hTab   = pcmt->hTab;
    HWND           hOwner = pctod->hDlg;
    RexxObjectPtr  owner  = pctod->rexxSelf;

    RexxThreadContext *c = pcpbd->dlgProcContext;

    // TODO implement some type of TCN_QUERYSIBLINGS.

    switch ( ((NMHDR *)lParam)->code )
    {
        case TCN_FOCUSCHANGE :
        {
            printf("RexxTabOwnerDlgProc() got TCN message: %s\n", tcn2str(lParam));
            break;
        }

        case TCN_GETOBJECT :
        {
            printf("RexxTabOwnerDlgProc() got TCN message: %s\n", tcn2str(lParam));
            break;
        }

        case TCN_KEYDOWN :
        {
            printf("RexxTabOwnerDlgProc() got TCN message: %s\n", tcn2str(lParam));
            break;
        }

        case TCN_SELCHANGE :
        {
            printf("RexxTabOwnerDlgProc() got TCN message: %s doingStartPage=%d\n", tcn2str(lParam), pcmt->doingStartPage);
            return doTCNSelChange(pctod, pcmt);

            break;
        }

        case TCN_SELCHANGING :
        {
            printf("RexxTabOwnerDlgProc() got TCN message: %s doingStartPage=%d\n", tcn2str(lParam), pcmt->doingStartPage);

            if ( pcmt->doingStartPage )
            {
                // Nothing showing yet, user asked for a start page other than 1.
                return TRUE;
            }

            int cur = TabCtrl_GetCurSel(hTab);
            RexxObjectPtr dlg = pcmt->rexxPages[cur];

            RexxObjectPtr ret = c->SendMessage2(dlg, "tabPageKillActive", owner, pcmt->rexxSelf);

            if ( checkForCondition(c, true) )
            {
                endDialogPremature(pcpbd, hOwner, RexxConditionRaised);
            }

            break;
        }

        case NM_CLICK :
        case NM_DBLCLK :
        case NM_RCLICK :
        case NM_RDBLCLK :
        case NM_RELEASEDCAPTURE :
        {
            printf("RexxTabOwnerDlgProc() got TCN message: %s\n", tcn2str(lParam));
            break;
        }


        default :
            break;
    }
    return TRUE;
}


/**
 * The dialog procedure function for TabOwnerDialog ooDialog dialogs.  Handles
 * and processes all window messages for the dialog.
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT CALLBACK
 *
 * TODO rewrite comments.
 *
 * @remarks  The WM_INITDIALOG message.
 *
 *           In CreateDialogParam() / CreateDialogIndirectParam() we pass the
 *           pointer to the PlainBaseDialog CSelf as the param.  The OS then
 *           sends us this value as the LPARAM argument in the WM_INITDIALOG
 *           message. The pointer is stored in the user data field of the window
 *           words for this dialog.  We do the same thing for the child dialogs,
 *           see the WM_USER_CREATECHILD message.
 *
 *           Note that when the child dialogs of the category dialog get
 *           created, we recieve a WM_INITDIALOG for each of them.  These child
 *           dialogs are all running on the same thread as the parent category
 *           dialog.  We don't want to do a bunch of nested AttachThreads()
 *           because we only do 1 DetachThread() for each window message loop.
 *           So, we check to see if dlgProcContext is null before doing the
 *           AttachThread().
 *
 *           The WM_USER_CREATECHILD message.
 *
 *           This user message's purpose is to create a child dialog of this
 *           dialog and return its window handle. Child dialogs are only created
 *           to implement the CategoryDialog and at this time are always created
 *           dynamically (from an in-memory template.) The dialog template
 *           pointer is passed here as the LPARAM arg from
 *           DynamicDialog::startChildDialog().
 *
 *           These child dialogs do not have a backing Rexx dialog. There is no
 *           unique CPlainBaseDialog struct for them.  Instead, at this time,
 *           all interaction with the child dialogs is done through the
 *           CPlainBaseDialog struct of the parent.  For each child dialog, we
 *           set the CPlainBaseDialog struct of the parent in the window words
 *           of the child dialog.  Prior to the conversion of ooDialog to the
 *           C++ API, when a message came in for a child dialog, a search was
 *           made through the DialogTable to try and find the parent dialog.
 *           This has been disposed of and the CPlainBaseDialog struct is just
 *           pulled out of the window words.
 */
LRESULT CALLBACK RexxTabOwnerDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)lParam;

        if ( pcpbd == NULL )
        {
            // Theoretically impossible.  But ... if it should happen, abort.
            return endDialogPremature(pcpbd, hDlg, NoPCPBDpased);
        }
        pCTabOwnerDialog pctod = (pCTabOwnerDialog)pcpbd->dlgPrivate;

        if ( pcpbd->dlgProcContext == NULL )
        {
            RexxThreadContext *context;
            if ( ! pcpbd->interpreter->AttachThread(&context) )
            {
                // Again, this shouldn't happen ... but
                return endDialogPremature(pcpbd, hDlg, NoThreadAttach);
            }
            pcpbd->dlgProcContext = context;
            pctod->dlgProcContext = context;

            RexxSetProcessMessages(FALSE);
        }

        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pcpbd);

        pctod->hDlg = hDlg;

        // Since we will (probably) be starting a page dialog executing and its
        // initDialog will be run, to be on the safe side, set isActive and call
        // the setDlgHandle() function now.
        pcpbd->hDlg = hDlg;
        pcpbd->isActive = true;
        setDlgHandle(pcpbd);

        if ( pcpbd->isCustomDrawDlg && pcpbd->idsNotChecked )
        {
            // We don't care what the outcome of this is, customDrawCheckIDs
            // will take care of aborting this dialog if the IDs are bad.
            customDrawCheckIDs(pcpbd);
        }

        setMangedTabDetails(pctod);
        initStartPages(pctod);

        return TRUE;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)getWindowPtr(hDlg, GWLP_USERDATA);
    if ( pcpbd == NULL )
    {
        // A number of messages arrive before WM_INITDIALOG, we just ignore them.
        return FALSE;
    }

    if ( pcpbd->dlgProcContext == NULL )
    {
        if ( ! pcpbd->isActive )
        {
            return FALSE;
        }

        // Once again, theoretically impossible ...
        return endDialogPremature(pcpbd, hDlg, NoThreadContext);
    }

    if ( uMsg == WM_DESTROY )
    {
        // Under all normal circumstances, WM_DESTROY never gets here.  But if
        // it does, it is because of some unexplained / unanticpated error.
        // PostQuitMessage() will cause the window message loop to quit and
        // things should then (hopefully) unwind cleanly.
        PostQuitMessage(3);
        return TRUE;
    }

    bool msgEnabled = IsWindowEnabled(hDlg) ? true : false;

    // Do not search message table for WM_PAINT to improve redraw.
    if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
    {
        pCTabOwnerDialog pctod = (pCTabOwnerDialog)pcpbd->dlgPrivate;
        uint32_t         index;

        if ( isTCNMsg(pctod, uMsg, lParam, &index) )
        {
            return handleTCNMsg(pctod, pcpbd, index, uMsg, wParam, lParam);
        }

        MsgReplyType searchReply = searchMessageTables(uMsg, wParam, lParam, pcpbd);
        if ( searchReply != ContinueProcessing )
        {
            // Note pre 4.0.1, we always returned FALSE, (pass on to the system
            // to process.) But, post 4.0.1 we sometimes reply TRUE, the message
            // has been handled.
            return (searchReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    if ( uMsg >= WM_USER_REXX_FIRST && uMsg <= WM_USER_REXX_LAST )
    {
        return handleWmUser(pcpbd, hDlg, uMsg, wParam, lParam, false);
    }

    switch ( uMsg )
    {
        case WM_PAINT:
            return drawBackgroundBmp(pcpbd, hDlg);

        case WM_DRAWITEM:
            return drawBitmapButton(pcpbd, lParam, msgEnabled);

        case WM_CTLCOLORDLG:
            return handleDlgColor(pcpbd);

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
            return handleCtlColor(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            return paletteMessage(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_COMMAND:
            return handleWmCommand(pcpbd, hDlg, wParam, lParam, false);

        default:
            break;
    }

    return FALSE;
}

/** TabOwnerDialog::init()  [private]
 *
 *  @param cpbd       Pointer to the PlainBaseDialog CSelf.
 *  @param ownerData  Owner data.
 */
RexxMethod2(RexxObjectPtr, tod_tabOwnerDlgInit, POINTER, cpbd, OSELF, self)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)cpbd;


    RexxBufferObject pcdBuffer = context->NewBuffer(sizeof(CTabOwnerDialog));
    if ( pcdBuffer == NULLOBJECT )
    {
        return TheOneObj;
    }
    context->SetObjectVariable("CSELF", pcdBuffer);

    pCTabOwnerDialog pctod = (pCTabOwnerDialog)context->BufferData(pcdBuffer);
    memset(pctod, 0, sizeof(CTabOwnerDialog));

    pcpbd->dlgPrivate = pctod;

    pctod->pcpbd = pcpbd;
    pctod->rexxSelf = self;

    RexxObjectPtr bag = rxNewBag(context);
    context->SetObjectVariable(TOD_MANAGEDTABBAG_ATTRIBUTE, bag);

    if ( pcpbd->initPrivate != NULL && pcpbd->initPrivate != TheNilObj )
    {
        pCTabOwnerDlgInfo pctodi = (pCTabOwnerDlgInfo)context->ObjectToCSelf((RexxObjectPtr)pcpbd->initPrivate);

        pctod->countMTs = pctodi->count;

        for ( uint32_t i = 0; i < pctodi->count; i++)
        {
            pCManagedTab pmt = pctodi->mts[i];
            pmt->rexxOwner = self;
            pmt->pcpbd     = pcpbd;

            context->SendMessage2(bag, "PUT", pmt->rexxSelf, pmt->rexxSelf);

            pctod->tabIDs[i] = pmt->tabID;
            pctod->mts[i] = pmt;

            // For each control dialog, set ourself as the ownerDialog, and that
            // the dialog is both owned and managed.

            for ( uint32_t j = 0; j < pmt->count; j++)
            {
                pCControlDialog pccd = pmt->cppPages[j];
                pCPlainBaseDialog pcpbd = pccd->pcpbd;

                pcpbd->rexxOwner = self;
                pcpbd->isOwnedDlg = true;
                pcpbd->isManagedDlg = true;
            }
        }
    }

    todAdjustMethods(context, self);

    return TheZeroObj;
}


/** TabOwnerDialog::getTabPage()
 *
 *
 */
RexxMethod3(RexxObjectPtr, tod_getTabPage, RexxObjectPtr, tabID, uint32_t, pageIndex, CSELF, pCSelf)
{
    RexxObjectPtr page = NULLOBJECT;

    pCTabOwnerDialog pctod = validateTodCSelf(context, pCSelf);
    if ( pctod == NULL )
    {
        goto done_out;
    }

    int32_t id = oodGlobalID(context->threadContext, tabID, 1, true);
    if ( id == OOD_ID_EXCEPTION  )
    {
        goto done_out;
    }

    if ( pageIndex == 0 || pageIndex > MAXTABPAGES )
    {
        wrongRangeException(context->threadContext, 2, 1, MAXTABPAGES, pageIndex);
        goto done_out;
    }
    pageIndex--;

    page = findRexxPage(pctod, id, pageIndex);
    if ( page == NULLOBJECT )
    {
        noSuchPageException(context, id, pageIndex);
    }

done_out:
    return page;
}


/** TabOwner::tabOwnerOk
 *
 *  The ok and cancel methods of the tab owner dialog are removed and replaced
 *  by the tabOwnerOk() and tabOwnerCancel() methods.
 *
 *  This allows us to operate like a property sheet, asking the individual tab
 *  pages if ok or cancel should be allowed, and to shut down all the tab page
 *  dialogs properly.
 *
 */
RexxMethod1(RexxObjectPtr, tod_tabOwnerOk, CSELF, pCSelf)
{
    pCTabOwnerDialog pctod = validateTodCSelf(context, pCSelf);
    if ( pctod == NULL )
    {
        return NULLOBJECT;
    }

    for ( uint32_t i = 0; i < pctod->countMTs; i++ )
    {
        pCManagedTab pmt = pctod->mts[i];

        for ( uint32_t j = 0; j < pmt->count; j++ )
        {
            pCControlDialog pccd = pmt->cppPages[j];
            context->SendMessage1(pccd->rexxSelf, "ENDEXECUTION", TheTrueObj);
        }

        pctod->pcpbd->wndBase->initCode = 1;
        context->SendMessage1(pctod->rexxSelf, "ENSUREFINISHED", TheFalseObj);
    }
    return TheTrueObj;
}


/** TabOwner::tabOwnerCancel
 *
 *  The ok and cancel methods of the tab owner dialog are removed and replaced
 *  by the tabOwnerOk() and tabOwnerCancel() methods.
 *
 *  This allows us to operate like a property sheet, asking the individual tab
 *  pages if ok or cancel should be allowed, and to shut down all the tab page
 *  dialogs properly.
 *
 */
RexxMethod1(RexxObjectPtr, tod_tabOwnerCancel, CSELF, pCSelf)
{
    pCTabOwnerDialog pctod = validateTodCSelf(context, pCSelf);
    if ( pctod == NULL )
    {
        return NULLOBJECT;
    }

    for ( uint32_t i = 0; i < pctod->countMTs; i++ )
    {
        pCManagedTab pmt = pctod->mts[i];

        for ( uint32_t j = 0; j < pmt->count; j++ )
        {
            pCControlDialog pccd = pmt->cppPages[j];
            context->SendMessage1(pccd->rexxSelf, "ENDEXECUTION", TheFalseObj);
        }

        pctod->pcpbd->wndBase->initCode = 2;
        context->SendMessage1(pctod->rexxSelf, "ENSUREFINISHED", TheFalseObj);
    }
    return TheTrueObj;
}


/**
 *  Methods for the .TabOwnerDlgInfo class.
 */
#define TABOWNERDLGINFO_CLASS        "TabOwnerDlgInfo"


RexxObjectPtr todiAddMTs(RexxMethodContext *c, pCTabOwnerDlgInfo pctodi, RexxObjectPtr mts)
{
    if ( c->IsOfType(mts, "ManagedTab") )
    {
        if ( pctodi->count >= MAXMANAGEDTABS )
        {
            return tooManyPagedTabsException(c, 5, false);
        }

        pCManagedTab pcmt = (pCManagedTab)c->ObjectToCSelf(mts);
        pctodi->mts[pctodi->count] = pcmt;
        pctodi->count++;
    }
    else if ( c->IsOfType(mts, "Array"))
    {
        RexxArrayObject _mts = (RexxArrayObject)mts;

        uint32_t count = (uint32_t)c->ArrayItems(_mts);
        if( count + pctodi->count > MAXMANAGEDTABS )
        {
            return tooManyPagedTabsException(c, count + pctodi->count, false);
        }

        for ( uint32_t i = 1; i <= count; i++ )
        {
            RexxObjectPtr mTab = c->ArrayAt(_mts, i);
            if ( mTab == NULLOBJECT )
            {
                sparseArrayException(c->threadContext, 1, i);
                goto done_out;
            }
            if ( ! c->IsOfType(mTab, "ManagedTab") )
            {
                wrongObjInArrayException(c->threadContext, 1, i, "a ManagedTab", mTab);
                goto done_out;
            }

            pCManagedTab pcmt = (pCManagedTab)c->ObjectToCSelf(mTab);
            pctodi->mts[pctodi->count] = pcmt;
            pctodi->count++;
        }
    }
    else
    {
        wrongArgValueException(c->threadContext, 1, "an Array or a ManagedTab", mts);
        goto done_out;
    }

    return TheTrueObj;

done_out:
    return TheFalseObj;
}

/** TabOwnerDlgInfo::init()
 *
 * A tab owner dialog information object can be used to intialize a tab owner
 * dialog during its instantiation.
 *
 * Since the array of managed tabs is the only information at this time, it does
 * not make much sense to make the array optional, or empty.  However, if more
 * information is added to the object, it might make some sense for the array to
 * be optional or empty.  So we allow it here, even though there is not much use
 * for the object then.
 *
 * @param  mts  [optiona] an array of managed tab objects.
 *
 * @remarks  Originally, there was the optional useResourceImage.  But after
 *           consideration that really seems unneeded.  If the only information
 *           remains an array of managed tabs, this object could be done away
 *           with altogether and the PlainBaseDialog init could be changed to
 *           check for an array object rather than a tab owner dialog
 *           information object.
 */
RexxMethod2(RexxObjectPtr, todi_init, OPTIONAL_RexxObjectPtr, mts, OSELF, self)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(CTabOwnerDlgInfo));
    if ( obj == NULLOBJECT )
    {
        goto done_out;
    }
    context->SetObjectVariable("CSELF", obj);

    pCTabOwnerDlgInfo pctodi = (pCTabOwnerDlgInfo)context->BufferData(obj);
    memset(pctodi, 0, sizeof(CTabOwnerDlgInfo));

    if ( argumentExists(1) )
    {
        todiAddMTs(context, pctodi, mts);
    }

done_out:
    return NULLOBJECT;
}

/** TabOwnerDlgInfo::add()
 *
 *  Adds a managed tab or managed tabs to the tab owner dialog information
 *  object.
 *
 *  @param  mts  A single ManagedTab object, or an array of ManagedTab objects.
 *
 */
RexxMethod2(RexxObjectPtr, todi_add, RexxObjectPtr, mts, CSELF, pCSelf)
{
    pCTabOwnerDlgInfo pctodi = (pCTabOwnerDlgInfo)pCSelf;
    if ( pctodi == NULL )
    {
        return (RexxObjectPtr)baseClassInitializationException(context);
    }

    return todiAddMTs(context, pctodi, mts);
}

/**
 *  Methods for the .ManagedTab class.
 */
#define ManagedTab_CLASS           "ManagedTab"
#define MT_DIALOGBAG_ATTRIBUTE     "ManagedTabDialogBag"

/**
 * Keeps a reference to each Rexx page dialog of the managed tab by putting it
 * in a Rexx bag.  The bag is set as a context variable.
 *
 * @param c            The method context we are operating in.
 * @param controlDlg   The Rexx page dialog we are saving a reference to.
 * @param self         The managed tab Rexx self.
 *
 * @return True on success, false otherwise.
 *
 * @remarks  Each managed tab object saves a reference to each Rexx page dialog.
 *           Each tab owner dialog saves a reference to each of its Rexx managed
 *           tabs.  This protects each Rexx page dialog from garbage collection
 *           as long as the tab owner dialog exists.
 *
 *           When a managed tab is initialized, the dialog bag context variable
 *           is added and each Rexx page dialog is put directly in the bag.
 *           However, the managed tab can have pages added, so we need this
 *           function.
 */
bool mtPutDialog(RexxMethodContext *c, RexxObjectPtr controlDlg, RexxObjectPtr self)
{
    RexxObjectPtr bag = c->GetObjectVariable(MT_DIALOGBAG_ATTRIBUTE);
    if ( bag == NULLOBJECT )
    {
        bag = rxNewBag(c);
        c->SetObjectVariable(MT_DIALOGBAG_ATTRIBUTE, bag);
    }
    if ( bag != NULLOBJECT )
    {
        c->SendMessage2(bag, "PUT", controlDlg, controlDlg);
    }

    return bag != NULLOBJECT;
}

/** ManagedTab::init()
 *
 *  Initialize a managed tab object.
 *
 *  Managed tabs can have pages added, so we allow the user to instantiate a
 *  managed tab with no pages, although it seems unlikely that that would be of
 *  much use.
 *
 *  We never allow the start page to be set to any value that does not have a
 *  backing page.  I.e., if there are no pages, the start page can not be set to
 *  1.
 *
 *  @param tabID  [required] The resource ID of the tab to be managed. May be
 *                symbolic, but the user must use the global .constDir
 *
 *  @param pages  [optional] May be omitted to instantiate the managed tab with
 *                no pages.  If used, must be an array.  The array can not be
 *                sparse.
 *
 *  @param opts       [optional] Keywords specifying miscellaneous styles.
 *                               SELCHANGE    owner wants SELCHANGE notice
 *                               SELCHANGING  owner wants SELCHANGE notice
 *
 *  @param startPage  [opitonal] One-based index. Defaults to the first page.
 *                    Must be within range of the actual number of pages.
 *
 *  @param wantNotifications  [optional] Defaults to false.  If the user wants
 *                            the NM_xx notifications in addition to the TCN_xx
 *                            notifications
 *
 *
 * use strict arg tabID, pages, startPage = 1, wantNotifications = .false
 *
 */
RexxMethod6(RexxObjectPtr, mt_init, RexxObjectPtr, tabID, OPTIONAL_RexxArrayObject, pages, OPTIONAL_CSTRING, opts,
            OPTIONAL_uint32_t, startPage, OPTIONAL_logical_t, wantNotifications, OSELF, self)
{
    RexxMethodContext *c = context;

    int32_t id = oodGlobalID(context->threadContext, tabID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto done_out;
    }

    uint32_t count = 0;
    if ( argumentExists(2) )
    {
        count = (uint32_t)context->ArrayItems(pages);
        if( count > MAXTABPAGES )
        {
            arrayToLargeException(context->threadContext, count, MAXTABPAGES, 2);
            goto done_out;
        }
    }

    // If startPage is omitted, its value is 0, which is also the default.
    if ( argumentExists(4) )
    {
        if ( startPage < 1 || startPage > count )
        {
            wrongRangeException(context->threadContext, 3, 1, count, startPage);
            goto done_out;
        }
        startPage--;
    }

    RexxBufferObject obj = context->NewBuffer(sizeof(CManagedTab));
    if ( obj == NULLOBJECT )
    {
        goto done_out;
    }
    context->SetObjectVariable("CSELF", obj);

    pCManagedTab pcmt = (pCManagedTab)context->BufferData(obj);
    memset(pcmt, 0, sizeof(CManagedTab));

    pcmt->rxTabID           = tabID;
    pcmt->tabID             = (uint32_t)id;
    pcmt->startPage         = startPage;
    pcmt->wantNotifications = wantNotifications ? true : false;
    pcmt->needDisplayRect   = true;
    pcmt->count             = count;
    pcmt->showing           = (uint32_t)-1;
    pcmt->rexxSelf          = self;

    if ( argumentExists(3) )
    {
        if ( StrStrI(opts, "SELCHANGE")   != NULL ) pcmt->ownerWantsSelChange = true;
        if ( StrStrI(opts, "SELCHANGING") != NULL ) pcmt->ownerWantsSelChanging = true;
        if ( StrStrI(opts, "SETACTIVE")   != NULL ) pcmt->ownerWantsSetActive = true;
        if ( StrStrI(opts, "KILLACTIVE")  != NULL ) pcmt->ownerWantsKillActive = true;
    }

    RexxObjectPtr bag = rxNewBag(context);
    context->SetObjectVariable(MT_DIALOGBAG_ATTRIBUTE, bag);

    pCControlDialog *cppPages = NULL;
    RexxObjectPtr   *rexxPages = NULL;

    if ( count > 0 )
    {
        pCControlDialog *cppPages = (pCControlDialog *)LocalAlloc(LPTR, MAXPROPPAGES * sizeof(pCControlDialog *));
        RexxObjectPtr *rexxPages = (RexxObjectPtr *)LocalAlloc(LPTR, MAXPROPPAGES * sizeof(RexxObjectPtr *));

        if ( cppPages == NULL || rexxPages == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        pCControlDialog *pPage = cppPages;
        RexxObjectPtr *pRexxPage = rexxPages;
        for ( uint32_t i = 1; i <= count; i++, pPage++, pRexxPage++ )
        {
            RexxObjectPtr dlg = context->ArrayAt(pages, i);
            if ( dlg == NULLOBJECT )
            {
                sparseArrayException(context->threadContext, 1, i);
                goto err_out;
            }
            if ( ! context->IsOfType(dlg, "CONTROLDIALOG") )
            {
                wrongObjInArrayException(context->threadContext, 1, i, "a ControlDlg", dlg);
                goto err_out;
            }

            pCControlDialog pccd = dlgToCDCSelf(context, dlg);

            pccd->pageNumber = i - 1;  // Zero based index in C++ struct.
            pccd->isManaged = true;    // It is managed now.

            *pPage = pccd;
            *pRexxPage = dlg;

            context->SendMessage2(bag, "PUT", dlg, dlg);
        }

        pcmt->cppPages = cppPages;
        pcmt->rexxPages = rexxPages;
    }

    goto done_out;

err_out:
    safeLocalFree(cppPages);
    safeLocalFree(rexxPages);

done_out:
    return NULLOBJECT;
}


/**
 *  Methods for the .ControlDlgInfo class.
 */
#define CONTROLDLGINFO_CLASS        "ControlDlgInfo"


static bool setCdiTitle(RexxMethodContext *c, pCControlDialogInfo pccdi, CSTRING title)
{
    safeLocalFree(pccdi->title);

    pccdi->title = (char *)LocalAlloc(LPTR, strlen(title) + 1);
    if ( pccdi->title == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(pccdi->title, title);
    return true;
}

static bool setCdiSize(RexxMethodContext *c, pCControlDialogInfo pccdi, RexxObjectPtr _size, bool exists, size_t argPos)
{
    if ( exists )
    {
        SIZE *s = (PSIZE)rxGetSize(c, _size, argPos);
        if ( s == NULL )
        {
            return false;
        }
        pccdi->size.cx = s->cx;
        pccdi->size.cy = s->cy;
    }
    else
    {
        pccdi->size.cx = 200;
        pccdi->size.cy = 150;
    }
    return true;
}

/** ControlDlgInfo::init()
 *
 * use strict arg owner = .nil,
 *                managed = .false
 *                title = "",
 *                size = (.size~new(200, 150)),
 *                wantNotifications = .false,
 *                tabIcon = (-1),
 *                resources = .nil,
 *
 */
RexxMethod7(RexxObjectPtr, cdi_init, OPTIONAL_RexxObjectPtr, owner, OPTIONAL_logical_t, managed,
            OPTIONAL_CSTRING, title, OPTIONAL_RexxObjectPtr, _size, OPTIONAL_RexxObjectPtr, tabIcon,
            OPTIONAL_RexxObjectPtr, resources, OSELF, self)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(CControlDialogInfo));
    if ( obj == NULLOBJECT )
    {
        goto done_out;
    }
    context->SetObjectVariable("CSELF", obj);

    RexxMethodContext *c = context;

    pCControlDialogInfo pccdi = (pCControlDialogInfo)context->BufferData(obj);
    memset(pccdi, 0, sizeof(CControlDialogInfo));

    pccdi->managed = managed ? true : false;

    if ( argumentExists(1) )
    {
        if ( ! c->IsOfType(owner, "PLAINBASEDIALOG") )
        {
            wrongClassException(c->threadContext, 1, "PlainBaseDialog");
            goto done_out;
        }
        pccdi->owner = owner;
    }

    if ( argumentExists(2) )
    {
        if ( ! setCdiTitle(context, pccdi, title) )
        {
            goto done_out;
        }
    }

    if ( ! setCdiSize(context, pccdi, _size, argumentExists(3), 3) )
    {
        goto  done_out;
    }

done_out:
    return NULLOBJECT;
}

/** ControlDlgInfo::managed()     [Attribute set]
 *
 *
 */
RexxMethod2(RexxObjectPtr, cdi_set_managed, logical_t, managed, CSELF, pCSelf)
{
    pCControlDialogInfo pccdi = (pCControlDialogInfo)pCSelf;
    pccdi->managed = managed ? true : false;
    return NULLOBJECT;
}


/** ControlDlgInfo::title()       [Attribute set]
 *
 *
 */
RexxMethod2(RexxObjectPtr, cdi_set_title, CSTRING, title, CSELF, pCSelf)
{
    pCControlDialogInfo pccdi = (pCControlDialogInfo)pCSelf;
    setCdiTitle(context, pccdi, title);
    return NULLOBJECT;
}


/** ControlDlgInfo::size()        [Attribute set]
 *
 *  Allows the size attribute of a control dialog info object to be reset after
 *  the object is instantiated.
 *
 *  @param size  [optional]  The new size value for this object.  If omitted,
 *               the size is reset to the default 200 x 150.
 */
RexxMethod2(RexxObjectPtr, cdi_set_size, RexxObjectPtr, _size, CSELF, pCSelf)
{
    pCControlDialogInfo pccdi = (pCControlDialogInfo)pCSelf;
    setCdiSize(context, pccdi, _size, true, 1);
    return NULLOBJECT;
}



/**
 *  Methods for the .ControlDialog class.
 */
#define CONTROLDIALOG_CLASS        "ControlDialog"

static inline pCControlDialog validateCdCSelf(RexxMethodContext *c, void *pCSelf)
{
    pCControlDialog pccd = (pCControlDialog)pCSelf;
    if ( pccd == NULL )
    {
        baseClassInitializationException(c);
    }
    return pccd;
}


void abortOwnedDlg(pCPlainBaseDialog pcpbd, HWND hDlg, DlgProcErrType t)
{
    RexxThreadContext *c          = pcpbd->dlgProcContext;
    pCPlainBaseDialog  ownerCSelf = dlgToCSelf(c, pcpbd->rexxOwner);
    pCPlainBaseDialog  pcpbdChild;

    for ( size_t i = 1; i <= ownerCSelf->countChilds; i++ )
    {
        pcpbdChild = (pCPlainBaseDialog)ownerCSelf->childDlg[i];
        c->SendMessage1(pcpbdChild->rexxSelf, "ENSUREFINISHED", TheTrueObj);
    }

    c->SendMessage1(ownerCSelf->rexxSelf, "ENSUREFINISHED", TheTrueObj);

    DestroyWindow(ownerCSelf->hDlg);

}


/** ControlDialog::init()  [Class method]
 *
 *  Used to capture the ControlDialog class object.  This is used for scoped
 *  look ups of the CSelf.
 */
RexxMethod1(RexxObjectPtr, cd_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, CONTROLDIALOG_CLASS) )
    {
        TheControlDialogClass = (RexxClassObject)self;
    }
    return NULLOBJECT;
}


static bool setCdTitle(RexxMethodContext *c, pCControlDialog pccd, CSTRING title)
{
    safeLocalFree(pccd->pageTitle);

    pccd->pageTitle = (char *)LocalAlloc(LPTR, strlen(title) + 1);
    if ( pccd->pageTitle == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(pccd->pageTitle, title);
    return true;
}

/** ControlDialog::controlDlgInit()  [private]
 *
 *  @param cpbd       Pointer to the PlainBaseDialog CSelf.
 *  @param ownerData  Owner data.
 *
 *  @remarks  Note that the order of the IsOfType() checks are done is
 *            important.  A RcControlDialog is a UserControlDialog.
 */
RexxMethod2(RexxObjectPtr, cd_controlDlgInit, POINTER, cpbd, OSELF, self)
{
    pCPlainBaseDialog   pcpbd = (pCPlainBaseDialog)cpbd;
    pCControlDialogInfo pccdi = NULL;

    if ( pcpbd->initPrivate != NULL && pcpbd->initPrivate != TheNilObj )
    {
        pccdi = (pCControlDialogInfo)context->ObjectToCSelf((RexxObjectPtr)pcpbd->initPrivate);
    }

    RexxBufferObject pcdBuffer = context->NewBuffer(sizeof(CControlDialog));
    if ( pcdBuffer == NULLOBJECT )
    {
        return TheOneObj;
    }
    context->SetObjectVariable("CSELF", pcdBuffer);

    pCControlDialog pccd = (pCControlDialog)context->BufferData(pcdBuffer);
    memset(pccd, 0, sizeof(CControlDialog));

    pccd->pcpbd = pcpbd;
    pccd->rexxSelf = self;
    pccd->extraOpts = context->NullString();
    pccd->isInitializing = true;
    pccd->pcpbd->dlgPrivate = pccd;

    if( context->IsOfType(self, "RCCONTROLDIALOG") )
    {
        pccd->pageType = oodRcControlDialog;
    }
    else if ( context->IsOfType(self, "USERCONTROLDIALOG") )
    {
        pccd->pageType = oodUserControlDialog;
    }
    else
    {
        pccd->pageType = oodResControlDialog;
    }

    if ( pccd->pageType != oodResControlDialog )
    {
        // The dynamic dialog init is finished before control dialog init.
        pccd->pcdd = (pCDynamicDialog)context->ObjectToCSelf(self, TheDynamicDialogClass);
    }

    // For a UserControlDialog, the resource ID can be 0, but for the other
    // types, we want a valid resource ID.
    int32_t resID;
    if ( pccd->pageType == oodUserControlDialog )
    {
        resID = oodResolveSymbolicID(context->threadContext, self, pcpbd->resourceID, -1, 2, false);
    }
    else
    {
        resID = oodResolveSymbolicID(context->threadContext, self, pcpbd->resourceID, -1, 2, true);
    }

    if ( resID == OOD_ID_EXCEPTION )
    {
        pcpbd->wndBase->initCode = 1;
        goto err_out;
    }
    pccd->resID = resID;

    if ( pccdi != NULL )
    {
        pccd->isManaged = pccdi->managed;

        if ( pccdi->title != NULL )
        {
            pccd->pageTitle = pccdi->title;
            pccdi->title = NULL;
        }

        pccd->size.cx = pccdi->size.cx;
        pccd->size.cy = pccdi->size.cy;
    }

    return TheZeroObj;

err_out:
    if ( pccdi != NULL )
    {
        safeLocalFree(pccdi->title);
    }
    return TheOneObj;
}

/** ControlDialog::isManaged()    [Attribute get]
 */
RexxMethod1(RexxObjectPtr, cd_get_isManaged, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        return pccd->isManaged ? TheTrueObj : TheFalseObj;
    }
    return NULLOBJECT;
}

/** ControlDialog::wasActivated()    [Attribute get]
 */
RexxMethod1(RexxObjectPtr, cd_get_wasActivated, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        return pccd->activated ? TheTrueObj : TheFalseObj;
    }
    return NULLOBJECT;
}

/** ControlDialog::extraOptions()    [Attribute get]
 */
RexxMethod1(RexxObjectPtr, cd_get_extraOptions, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        return pccd->extraOpts;
    }
    return NULLOBJECT;
}

/** ControlDialog::extraOptions()    [Attribute set]
 */
RexxMethod2(RexxObjectPtr, cd_set_extraOptions, RexxStringObject, opts, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        pccd->extraOpts = opts;
    }
    return NULLOBJECT;
}

/** ControlDialog::initializing()    [Attribute get]
 */
RexxMethod1(RexxObjectPtr, cd_get_initializing, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        return pccd->isInitializing ? TheTrueObj : TheFalseObj;
    }
    return NULLOBJECT;
}

/** ControlDialog::initializing()    [Attribute set]
 */
RexxMethod2(RexxObjectPtr, cd_set_initializing, logical_t, initializing, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        pccd->isInitializing = initializing ? true : false;
    }
    return NULLOBJECT;
}

/** ControlDialog::pageTitle()       [Attribute get]
 */
RexxMethod1(RexxObjectPtr, cd_get_pageTitle, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        return pccd->pageTitle == NULL ? TheNilObj : context->String(pccd->pageTitle);
    }
    return NULLOBJECT;
}


/** ControlDialog::pageTitle()       [Attribute set]
 */
RexxMethod2(RexxObjectPtr, cd_set_pageTitle, CSTRING, text, CSELF, pCSelf)
{
    pCControlDialog pccd = validateCdCSelf(context, pCSelf);
    if ( pccd != NULL )
    {
        setCdTitle(context, pccd, text);
    }
    return NULLOBJECT;
}


/**
 *  Methods for the .ResControlDialog class.
 */
#define RESControlDialog_CLASS        "ResControlDialog"

/** ResControlDialog::startDialog()
 *
 *  This method over-rides the superclass (ResDialog) startDialog().
 *
 *  We only need library and id, the owner dialog we pull from the CSelf
 *  struct.
 */
RexxMethod3(RexxObjectPtr, resCtrlDlg_startDialog_pvt, CSTRING, library, RexxObjectPtr, _dlgID, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd == NULL )
    {
        goto err_out;
    }

    if ( ! validControlDlg(context, pcpbd) )
    {
        goto err_out;
    }

    int32_t dlgID = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, _dlgID, -1, 2, true);
    if ( dlgID == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }
    if ( dlgID == 0 )
    {
        wrongArgValueException(context->threadContext, 2, "a valid numeric resource ID or a valid symbolic ID", _dlgID);
        goto err_out;
    }

    if ( !loadResourceDLL(pcpbd, library) )
    {
        goto err_out;
    }

    HWND hChild = (HWND)SendMessage(pcpbd->hOwnerDlg, WM_USER_CREATECONTROL_RESDLG, (WPARAM)pcpbd, (LPARAM)dlgID);
    if ( hChild )
    {
        pcpbd->hDlg = hChild;
        pcpbd->isActive = true;
        ((pCControlDialog)pcpbd->dlgPrivate)->activated = true;
        pcpbd->childDlg[0] = hChild;

        setDlgHandle(pcpbd);
        setFontAttrib(context->threadContext, pcpbd);

        if ( pcpbd->autoDetect )
        {
            if ( doDataAutoDetection(context->threadContext, pcpbd) == OOD_MEMORY_ERR )
            {
                goto err_out;
            }
        }

        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}


/**
 *  Methods for the .RcControlDialog class.
 */
#define RcControlDialog_CLASS        "RcControlDialog"


RexxMethod7(RexxObjectPtr, rcCtrlDlg_startTemplate, uint32_t, cx, uint32_t, cy, CSTRING, title, CSTRING, fontName,
            uint32_t, fontSize, uint32_t, expected, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    pCControlDialog pccd = (pCControlDialog)pcpbd->dlgPrivate;
    pCDynamicDialog pcdd = pccd->pcdd;

    pccd->size.cx = cx;
    pccd->size.cy = cy;

    if ( strlen(title) > 0 )
    {
        if ( ! setCdTitle(context, pccd, title) )
        {
            goto err_out;
        }
    }

    size_t len = strlen(fontName);

    if ( len > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 4, MAX_DEFAULT_FONTNAME, len);
        goto err_out;
    }
    if ( len > 0 )
    {
        strcpy(pcpbd->fontName, fontName);
    }

    if ( fontSize != 0 )
    {
        pcpbd->fontSize = fontSize;
    }
    pcdd->expected = expected;

    return TheZeroObj;

err_out:
    return TheOneObj;
}



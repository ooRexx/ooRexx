/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
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
 * Open Object Rexx ooDialog - Resizable dialog funtionality.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodShared.hpp"
#include "oodControl.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodMessaging.hpp"
#include "oodResizableDialog.hpp"

/**
 * Validates that the CSelf pointer for a ResizingAdmin object is not null, and
 * that the resizeInfo pointer is not null.
 *
 * @note  Originally we were going to have a separate CResizingAdmin CSelf.
 *        But, if the user sets up a class that inherits ResizingAdmin before
 *        inheriting PlainBaseDialog, dialog methods expecting CSelf to be
 *        CPlainBaseDialog, will end up with CResizingAdmin instead.  That blows
 *        up.
 *
 *        Granted it is a not common class set up, but it is a reasonable class
 *        set up.  Since the ResizeInfoDlg struct is in the CPlainBaseDialog
 *        struct, it is simplier to not use CResizingAdmin.
 */
inline pCPlainBaseDialog validateRACSelf(RexxMethodContext *c, void *CSelf, pResizeInfoDlg *pprid)
{
    if ( CSelf == NULL || ((pCPlainBaseDialog)CSelf)->resizeInfo == NULL)
    {
        baseClassInitializationException(c, "ResizingAdmin");
        return NULL;
    }

    *pprid = ((pCPlainBaseDialog)CSelf)->resizeInfo;
    return (pCPlainBaseDialog)CSelf;
}

/**
 * Add the needed WS_CLIPCHILDREN flag.  This dramatically reduces flicker.
 *
 * If we also add WS_THICKFRAME here, it appears to work in most cases.  But,
 * there is some problem if the window does not have the WS_MAXIMIZEBOX style.
 * However, if WS_THICKFRAME is added to the original dialolg template, things
 * work without the WS_MAXIMIZEBOX style.  So, for resizable dialogs, we add
 * that flag to the dialog template instead.
 *
 * @param hDlg
 */
inline void setResizableDlgStyle(HWND hDlg)
{
    SetWindowLong(hDlg, GWL_STYLE, GetWindowLong(hDlg, GWL_STYLE) | WS_CLIPCHILDREN);
}

/**
 * Raises an exception for an invalid pin to control ID set by the user.
 *
 * IDC_DEFAULT_PINTO_WINDOW is always valid.  Otherwise there are 2 reasons why
 * a pin to ID is not valid. 1.) There must be a ResizeInfoCtrl record in the
 * table prior to ctrlID record.  2.) After the dialog is created, the pin to ID
 * must be a valid control window.
 *
 * For #1, the id is checked when / if the user adds a record for ctrlID.  For
 * #2, the existing pin to are checked after the dialog is created, but before
 * all dialog controls are enumerated.
 *
 * The text of the exception does not distinguish between case #1 and case #2.
 * The user will have to use the context of when the exception happens to
 * distiguish the two.
 *
 * @param c
 */
static void raiseInvalidPinToIDException(RexxThreadContext *c, uint32_t pinToID, uint32_t ctrlID)
{
    TCHAR buf[512];
    _snprintf(buf, sizeof(buf), "The resource ID (%d) for the pin to window edge of control ID (%d) is not valid",
              pinToID, ctrlID);
    executionErrorException(c, buf);
}

/**
 * Raises an exception for an invalid, default, pin to control ID set by the
 * user.
 *
 * IDC_DEFAULT_PINTO_WINDOW is always valid.
 *
 * Otherwise if the default sizing is changed, the pin to window must already
 * have a record in the tabel.  I.e., the pin to window has to precede any
 * default records that are added from this point on.
 *
 * If the user uses an invalid resource ID, for example 202, but there is no
 * dialog control with resource ID 202, this will be detected when the existing
 * pin to IDs are checked after the dialog is created, but before all dialog
 * controls are enumerated.  For this the raiseInvalidPinToIDException will be
 * raised.
 *
 * @param c
 */
static void raiseInvalidDefaultPinToIDException(RexxThreadContext *c, uint32_t pinToID)
{
    TCHAR buf[512];
    _snprintf(buf, sizeof(buf), "The resource ID (%d) for the default pin to window edge is not valid, it is not in the table",
              pinToID);
    executionErrorException(c, buf);
}

/**
 * This exception is for a control resize info record added by the user whose
 * control resource ID is not valid.  I.e., the user typed 109 for the resource
 * ID, but there is no dialog control with that ID in the dialog.
 *
 * This would be discovered after the underlying dialog is created.
 *
 * @param c
 * @param ctrlID
 */
static void raiseInvalidResizeInfoRecordException(RexxThreadContext *c, uint32_t ctrlID)
{
    TCHAR buf[512];
    _snprintf(buf, sizeof(buf), "Invalid resize info record; the control ID (%d) does not refer to a valid window", ctrlID);
    executionErrorException(c, buf);
}

/**
 * Checks that resize info record for each dialog control added by the user is
 * valid.
 *
 * When the user adds a resize info record, it is before the underlying dialog
 * is created.  At that time, a check is done that any pin to ID is valid.  To
 * be valid, it must either be IDC_DEFAULT_PINTO_WINDOW or be the control ID of
 * a previous record.
 *
 * However, there is no way to check that any resource ID actually is the
 * resource ID of a dialog control in the dialog.  All control window handles
 * are null for the user records.  After we enumerate all dialog controls, each
 * user added record should have a valid control window handle, *unless* the
 * user used an incorrect resource ID.
 *
 * @param c
 * @param hDlg
 * @param prid
 *
 * @return bool
 */
static bool verifyWindows(RexxThreadContext *c, HWND hDlg, pResizeInfoDlg prid, size_t countUserAdded)
{
    for ( register size_t i = 0; i < countUserAdded; i++)
    {
        if ( ! IsWindow(prid->riCtrls[i].hCtrl) )
        {
            raiseInvalidResizeInfoRecordException(c, prid->riCtrls[i].id);
            checkForCondition(c, false);
            return false;
        }
    }
    return true;
}

/**
 * Verifies that the pin to ID is either IDC_DEFAULT_PINTO_WINDOW, or that it is
 * the resource ID of a control present in the table *before* the control ID, or
 * that control ID is not currently in the table and pin to ID is present.
 *
 * @param c
 * @param ctrlID
 * @param pinToID
 * @param prid
 *
 * @return bool
 *
 * @notes  This function can be called prior to creating a record for the ctrlID
 *         control.  This is a normal usage.  In this case pinToID is valid if
 *         it exists in the table, even if it is the last entry.
 */
static bool validPinToID(RexxMethodContext *c, uint32_t ctrlID, uint32_t pinToID, pResizeInfoDlg prid)
{
    if ( pinToID == IDC_DEFAULT_PINTO_WINDOW )
    {
        return true;
    }

    bool validPinID  = false;

    for ( register size_t i = 0; i < prid->countCtrls; i++)
    {
        if ( prid->riCtrls[i].id == pinToID )
        {
            validPinID = true;
            break;
        }

        if ( prid->riCtrls[i].id == ctrlID )
        {
            break;
        }
    }

    if ( ! validPinID )
    {
        raiseInvalidPinToIDException(c->threadContext, pinToID, ctrlID);
        return false;
    }

    return true;
}

/**
 * Verifies that the pin to ID is either IDC_DEFAULT_PINTO_WINDOW, or that it is
 * the resource ID of a control that is already present in the table.
 *
 * @param c
 * @param pinToID
 * @param prid
 *
 * @return bool
 *
 * @notes  The default sizing for any edge can be changed, but it can not have a
 *         pin to ID that is not already in the table.
 */
static bool validDefaultPinToID(RexxMethodContext *c, uint32_t pinToID, pResizeInfoDlg prid)
{
    if ( pinToID == IDC_DEFAULT_PINTO_WINDOW )
    {
        return true;
    }

    for ( register size_t i = 0; i < prid->countCtrls; i++)
    {
        if ( prid->riCtrls[i].id == pinToID )
        {
            return true;
        }
    }

    raiseInvalidDefaultPinToIDException(c->threadContext, pinToID);
    return false;
}

/**
 * Verifies that for the control ID specified, all of the pin to IDs in the
 * edges of the control precede the control ID in the control resizing table.
 *
 * @param c
 * @param ctrlID
 * @param ric
 * @param prid
 *
 * @return True if the edges info is good, false otherwise.
 */
inline bool validPinToInfo(RexxMethodContext *c, uint32_t ctrlID, pResizeInfoCtrl ric, pResizeInfoDlg prid)
{
    return validPinToID(c, ctrlID, ric->edges.left.pinToID, prid)    &&
           validPinToID(c, ctrlID, ric->edges.top.pinToID, prid)     &&
           validPinToID(c, ctrlID, ric->edges.right.pinToID, prid)   &&
           validPinToID(c, ctrlID, ric->edges.bottom.pinToID, prid);
}

/**
 * Converts a pin to type keyword to the proper flag value. We only allow the
 * user to use MYTOP or MYLEFT where it makes sense.
 *
 * @param c
 * @param keyWord
 * @param pos
 * @param isLeftTop
 *
 * @return pinType_t
 */
static pinType_t keyword2pinType(RexxMethodContext *c, CSTRING keyWord, size_t pos, pinnedEdge_t edge)
{
    pinType_t pinType = notAPin;

    if (      StrCmpI(keyWord, "PROPORTIONAL") == 0 ) pinType = proportionalPin;
    else if ( StrCmpI(keyWord, "STATIONARY")   == 0 ) pinType = stationaryPin;

    switch ( edge )
    {
        case leftEdge :
        case topEdge :
            if ( pinType == notAPin )
            {
                wrongArgKeywordException(c, pos, PIN_TYPE_SHORT_LIST, keyWord);
            }
            break;

        case rightEdge :
            if ( pinType == notAPin )
            {
                if ( StrCmpI(keyWord, "MYLEFT") == 0 )
                {
                    pinType = myLeftPin;
                }
                else
                {
                    wrongArgKeywordException(c, pos, PIN_TYPE_RIGHT_LIST, keyWord);
                }
            }
            break;

        case bottomEdge :
            if ( pinType == notAPin )
            {
                if ( StrCmpI(keyWord, "MYTOP") == 0 )
                {
                    pinType = myTopPin;
                }
                else
                {
                    wrongArgKeywordException(c, pos, PIN_TYPE_BOTTOM_LIST, keyWord);
                }
            }
            break;

        default :
            break;

    }

    return pinType;
}

/**
 * Converts a pin to edge keyword to the proper flag value.
 *
 * @param c
 * @param keyWord
 * @param pos
 *
 * @return pinnedEdge_t
 */
static pinnedEdge_t keyword2pinnedEdge(RexxMethodContext *c, CSTRING keyWord, size_t pos)
{
    pinnedEdge_t edge = notAnEdge;

    if ( StrCmpI(keyWord,      "LEFT") == 0    ) edge = leftEdge;
    else if ( StrCmpI(keyWord, "TOP") == 0     ) edge = topEdge;
    else if ( StrCmpI(keyWord, "RIGHT") == 0   ) edge = rightEdge;
    else if ( StrCmpI(keyWord, "BOTTOM") == 0  ) edge = bottomEdge;
    else if ( StrCmpI(keyWord, "XCENTER") == 0 ) edge = xCenterEdge;
    else if ( StrCmpI(keyWord, "YCENTER") == 0 ) edge = yCenterEdge;
    else wrongArgKeywordException(c, pos, PINNED_EDGE_LIST, keyWord);

    return edge;
}

/**
 * Sets up the default left edge struct to the values specified.
 *
 * @param c
 * @param howPinned
 * @param whichEdge
 * @param prid
 *
 * @return Zero on success, one on error.  This Rexx object is passed on to the
 *         user as a reply from a method invocation.
 */
static RexxObjectPtr defaultLeft(RexxMethodContext *c, CSTRING howPinned, CSTRING whichEdge, uint32_t pinToID, pResizeInfoDlg prid)
{
    prid->defEdges.left.pinType   = keyword2pinType(c, howPinned, 1, leftEdge);
    prid->defEdges.left.pinToEdge = keyword2pinnedEdge(c, whichEdge, 2);
    prid->defEdges.left.pinToID   = pinToID;

    if ( prid->defEdges.left.pinType == notAPin || prid->defEdges.left.pinToEdge == notAnEdge )
    {
        return TheOneObj;
    }
    return TheZeroObj;
}

/**
 * Sets up the default top edge struct to the values specified.
 *
 * @param c
 * @param howPinned
 * @param whichEdge
 * @param prid
 *
 * @return Zero on success, one on error.  This Rexx object is passed on to the
 *         user as a reply from a method invocation.
 */
static RexxObjectPtr defaultTop(RexxMethodContext *c, CSTRING howPinned, CSTRING whichEdge, uint32_t pinToID, pResizeInfoDlg prid)
{
    prid->defEdges.top.pinType   = keyword2pinType(c, howPinned, 1, topEdge);
    prid->defEdges.top.pinToEdge = keyword2pinnedEdge(c, whichEdge, 2);
    prid->defEdges.top.pinToID   = pinToID;

    if ( prid->defEdges.top.pinType == notAPin || prid->defEdges.top.pinToEdge == notAnEdge )
    {
        return TheOneObj;
    }
    return TheZeroObj;
}

/**
 * Sets up the default right edge struct to the values specified.
 *
 * @param c
 * @param howPinned
 * @param whichEdge
 * @param prid
 *
 * @return Zero on success, one on error.  This Rexx object is passed on to the
 *         user as a reply from a method invocation.
 */
static RexxObjectPtr defaultRight(RexxMethodContext *c, CSTRING howPinned, CSTRING whichEdge, uint32_t pinToID, pResizeInfoDlg prid)
{
    prid->defEdges.right.pinType   = keyword2pinType(c, howPinned, 1, rightEdge);
    prid->defEdges.right.pinToEdge = keyword2pinnedEdge(c, whichEdge, 2);
    prid->defEdges.right.pinToID   = pinToID;

    if ( prid->defEdges.right.pinType == notAPin || prid->defEdges.right.pinToEdge == notAnEdge )
    {
        return TheOneObj;
    }
    return TheZeroObj;
}

/**
 * Sets up the default bottom edge struct to the values specified.
 *
 * @param c
 * @param howPinned
 * @param whichEdge
 * @param prid
 *
 * @return Zero on success, one on error.  This Rexx object is passed on to the
 *         user as a reply from a method invocation.
 */
static RexxObjectPtr defaultBottom(RexxMethodContext *c, CSTRING howPinned, CSTRING whichEdge, uint32_t pinToID, pResizeInfoDlg prid)
{
    prid->defEdges.bottom.pinType   = keyword2pinType(c, howPinned, 1, bottomEdge);
    prid->defEdges.bottom.pinToEdge = keyword2pinnedEdge(c, whichEdge, 2);
    prid->defEdges.bottom.pinToID   = pinToID;

    if ( prid->defEdges.bottom.pinType == notAPin || prid->defEdges.bottom.pinToEdge == notAnEdge )
    {
        return TheOneObj;
    }
    return TheZeroObj;
}

/**
 * Allocates a new dialog control resize info structure and assigns it to the
 * end of table.
 *
 * @param c
 * @param prid
 * @param ctrlID
 * @param hCtrl     May be null if before dialog is created
 * @param ctrlType  May be winUnknown if before dialog is created
 *
 * @return A pointer to the newly allocated structure on success, null on error.
 *
 * @assumes The table has already been checked and does not contain a struct for
 *          this control.
 */
static pResizeInfoCtrl allocCtrlInfo(RexxThreadContext *c, pResizeInfoDlg prid, int32_t ctrlID, HWND hCtrl, oodControl_t ctrlType)
{
    size_t index = prid->countCtrls;
    if ( index >= prid->tableSize )
    {
        HLOCAL temp = LocalReAlloc(prid->riCtrls, sizeof(ResizeInfoCtrl) * prid->tableSize * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(0, "Resizable control entrires have exceeded the maximum\n"
                          "number of allocated table entries, and the table could\n"
                          "not be expanded.\n\n"
                          "No more control entries can be added.\n",
                       "Error", MB_OK | MB_ICONHAND);

            return NULL;
        }

        prid->tableSize *= 2;
        prid->riCtrls = (pResizeInfoCtrl)temp;
    }

    pResizeInfoCtrl ric = &prid->riCtrls[index];
    prid->countCtrls++;

    memcpy(&ric->edges, &prid->defEdges, sizeof(ControlEdges));

    ric->hCtrl    = hCtrl;
    ric->id       = ctrlID;
    ric->ctrlType = ctrlType;

    return ric;
}

/**
 * Returns the dialog control resize info struct for the specified control ID.
 *
 * @param prid
 * @param ctrlID
 *
 * @return A pointer to the struct when found, otherwise null.
 *
 * @remarks  Resize info is added to the table before the underlying dialog is
 *           created.  At that time, the resource ID must be greater than 0.
 *           Then, resize info is also added during WM_INITDIALOG.  At that
 *           time, the ID could be -1 for a static control.
 *
 *           We *always* return NULL for -1.  We assume that if the caller uses
 *           -1 for the control ID, the caller knows that there is no existing
 *           resize info for the control.  This is true during the enum windows
 *           function, InitializeAllControlsProc().  The caller is responsible
 *           for any other use.
 */
static pResizeInfoCtrl getControlInfo(pResizeInfoDlg prid, uint32_t ctrlID)
{
    if ( ctrlID != (uint32_t)-1 )
    {
        for ( register size_t i = 0; i < prid->countCtrls; i++)
        {
            if ( prid->riCtrls[i].id == ctrlID )
            {
                return &prid->riCtrls[i];
            }
        }
    }

    return NULL;
}

/**
 * Searches the array of control resize information structs for a tab control
 * that matches the specified resource ID.
 *
 * Returns the window handle of the tab control when found.  Will return NULL if
 * not found, but really that should never happen.
 *
 * @param prid
 * @param ctrlID
 *
 * @return HWND
 */
static HWND getTabControlHwnd(pResizeInfoDlg prid, uint32_t ctrlID)
{
    if ( ctrlID != (uint32_t)-1 )
    {
        for ( register size_t i = 0; i < prid->countCtrls; i++)
        {
            if ( prid->riCtrls[i].id == ctrlID )
            {
                return prid->riCtrls[i].hCtrl;
            }
        }
    }

    return NULL;
}


/**
 * Searches the array of child dialogs for a child dialog with the specified
 * dialog ID.
 *
 * Returns the pCPlainBaseDialog pointer for the child when found, NULL when not
 * found.  The child will only be found when the underlying Windows dialog has
 * been created.  If the Rexx programmer waits until a page is visited before
 * creating the underlying Windows dialog, it is very likely that the child
 * dialog will not always be found.
 *
 * @param pcpbd
 * @param dlgID
 *
 * @return pCPlainBaseDialog
 */
static pCPlainBaseDialog getChildDlg(pCPlainBaseDialog pcpbd, uint32_t dlgID)
{
    for ( register size_t i = 1; i <= pcpbd->countChilds; i++ )
    {
        pCPlainBaseDialog chldPcpbd = (pCPlainBaseDialog)pcpbd->childDlg[i];
        if ( chldPcpbd->dlgID == dlgID )
        {
            return chldPcpbd;
        }
    }
    return NULL;
}


/**
 * When the page dialogs of a tab control are resized, they do not paint
 * correctly.  The only solution found so far is to use RedrawWindow() to redraw
 * the page dialog.  Doing this at every WM_SIZE increases the flickering and
 * doesn't even work.  Redrawing once at the WM_EXITSIZEMOVE message works well.
 *
 * During the WM_SIZE processing, we record which dialog, for each tab control
 * if there is more than one, needs to be redrawn.  Only the single visible
 * dialog in the tab control needs to be redrawn.
 *
 * @param prid
 */
static void maybeRedrawWindows(pResizeInfoDlg prid)
{
    for ( register size_t i = 0; i < prid->countPagedTabs; i++ )
    {
        if ( prid->pagedTabs[i]->redrawThis )
        {
            RedrawWindow(prid->pagedTabs[i]->redrawThis, NULL, NULL,
                         RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            prid->pagedTabs[i]->redrawThis = NULL;
        }
    }
}


/**
 * Handles the sizing and positioning of ControlDialog dialogs used as the pages
 * of a tab control.
 *
 * @param pcpbd
 * @param hDlg
 *
 * @remarks.  The pCPlainBaseDialog pointer for each ControlDialog created as a
 *            child of the 'main', or 'owner', dialog is stored in the
 *            childDlg[] array of the owner.
 */
static void handleChildDialogs(pCPlainBaseDialog pcpbd, HWND hDlg)
{
    pResizeInfoDlg prid = pcpbd->resizeInfo;

    for ( size_t i = 0; i < prid->countPagedTabs; i++ )
    {
        pPagedTab pt   = prid->pagedTabs[i];
        HWND      hTab = getTabControlHwnd(prid, pt->tabID);
        if ( hTab == NULL )
        {
            // Probably need error?
            continue;
        }

        POINT p = { 0 };
        SIZE  s = { 0 };

        calcDisplayRect(hTab, hDlg, &p, &s);

        for ( size_t j = 0; j < MAXCHILDDIALOGS; j++ )
        {
            uint32_t dlgID = pt->dlgIDs[j];
            if ( dlgID == 0 )
            {
                break;
            }

            pCPlainBaseDialog chldPcpbd = getChildDlg(pcpbd, dlgID);
            if ( chldPcpbd == NULL )
            {
                continue;
            }

            if ( IsWindowVisible(chldPcpbd->hDlg) )
            {
                SetWindowPos(chldPcpbd->hDlg, hTab, p.x, p.y, s.cx, s.cy, SWP_NOCOPYBITS | SWP_NOOWNERZORDER);
                pt->redrawThis = chldPcpbd->hDlg;
            }
            else
            {
                MoveWindow(chldPcpbd->hDlg, p.x, p.y, s.cx, s.cy, FALSE);
            }
        }
    }
}


/**
 * Returns a resize info record for the specified control ID after validating
 * that the pin to ID is valid for the control ID.
 *
 * @param c
 * @param ctrlID
 * @param pinToID
 * @param prid
 *
 * @return A pointer to the ResizeInfoCtrl struct for the control ID, or null on
 *         failure.
 *
 * @notes  Allocates a new record for the control if one does not already exist.
 *         The pin to ID must be valid, that is, it must either be
 *         IDC_DEFAULT_PINTO_WINDOW, or be the control ID of an already existing
 *         record.
 */
static pResizeInfoCtrl getValidatedControlInfo(RexxMethodContext *c, uint32_t ctrlID, uint32_t pinToID, pResizeInfoDlg prid)
{
    if ( ! validPinToID(c, ctrlID, pinToID, prid) )
    {
        return NULL;
    }

    pResizeInfoCtrl ric = getControlInfo(prid, ctrlID);
    if ( ric == NULL )
    {
        ric = allocCtrlInfo(c->threadContext, prid, ctrlID, NULL, winUnknown);
    }
    return ric;
}

/**
 * Returns a resize info record for the specified control ID, allocating the
 * record struct if needed.
 *
 * @param c
 * @param ctrlID
 * @param prid
 *
 * @return A pointer to the ResizeInfoCtrl struct for the control ID, or null on
 *         failure.
 *
 * @notes  Allocates a new record for the control if one does not already exist.
 */
static pResizeInfoCtrl getUnValidatedControlInfo(RexxMethodContext *c, uint32_t ctrlID, pResizeInfoDlg prid)
{
    pResizeInfoCtrl ric = getControlInfo(prid, ctrlID);
    if ( ric == NULL )
    {
        ric = allocCtrlInfo(c->threadContext, prid, ctrlID, NULL, winUnknown);
    }
    return ric;
}

/**
 * Returns the width or height of the specified rectangle.
 *
 * @param e     Specifies a pinned edge, this determines if the width or the
 *              height is desired.
 *
 * @param rect  The rectangle whose size is to be retrieved
 *
 * @return int
 */
static size_t calcRectSize(pinnedEdge_t e, RECT *rect)
{
    switch ( e )
    {
        case leftEdge :
        case rightEdge :
        case xCenterEdge :
            return rect->right - rect->left;

        case topEdge :
        case bottomEdge :
        case yCenterEdge :
        default :
            return rect->bottom - rect->top;
    }
}

/**
 * Gets either the original, or the current, window position rectangle for the
 * specified window.
 *
 * @param id      Window identifier, specifies the window whose position
 *                rectangle is desired. Use IDC_DEFAULT_PINTO_WINDOW for the
 *                dialog, otherwise the resource ID of a dialog control.
 *
 * @param prid    The resize info struct for this dialog.
 *
 * @param initial Specifies if the original or the current rectangle is desired.
 *                True for original, false for current.
 *
 * @return The window position rectangle specified.
 *
 * @note  It is theoretically impossible for the window position rectangle to
 *        not exist at this point.  The proper position of the pin to windows in
 *        the record table is checked during record generation, each dialog
 *        control is enumerated and a valid record placed in the table at dialog
 *        creation time, and finally each record is checked to be pointing at a
 *        valid window.
 *
 *        Nevertheless, the code is structured so that if id is not for the
 *        dialog, but the rectangle is not found in the table, the dialog
 *        rectangle is returned.  This is done on purpose.
 */
static RECT *getRect(uint32_t id, pResizeInfoDlg prid, bool initial)
{
    if ( id != 0 )
    {
        for ( register size_t i = 0; i < prid->countCtrls; i++)
        {
            if ( prid->riCtrls[i].id == id )
            {
                return initial ? &prid->riCtrls[i].originalRect : &prid->riCtrls[i].currentRect;
            }
        }
    }

    return initial ? &prid->originalRect : &prid->currentRect;
}

/**
 * Returns the co-ordinate in the specified rectangle that matches the pinned
 * edge type.
 *
 * @param edge
 * @param r
 *
 * @return int32_t
 */
static int32_t edgeCoord(pinnedEdge_t edge, RECT *r)
{
    switch ( edge )
    {
        case leftEdge :
            return r->left;

        case topEdge :
            return r->top;

        case rightEdge :
            return r->right;

        case bottomEdge :
            return r->bottom;

        case xCenterEdge :
            return (r->right + r->left) / 2;

        case yCenterEdge :
            return (r->top + r->bottom) / 2;

        default :
            break;
    }
    return 0;
}

/**
 * Returns the new left co-ordinate for dialog control specified through its
 * resize info, for the edge specified.
 *
 * @param prid
 * @param ric
 * @param edge
 *
 * @return int32_t
 */
static int32_t newLeft(pResizeInfoDlg prid, pResizeInfoCtrl ric, pEdge edge)
{
    RECT *pinToRectInitial = getRect(edge->pinToID, prid, true);
    RECT *pinToRectCurrent = getRect(edge->pinToID, prid, false);

    int32_t l = 0;

    switch ( edge->pinType )
    {
        case stationaryPin :
            l = ric->originalRect.left - edgeCoord(edge->pinToEdge, pinToRectInitial) + edgeCoord(edge->pinToEdge, pinToRectCurrent);
            break;

        case proportionalPin :
        {
            size_t oldSize = calcRectSize(leftEdge, pinToRectInitial);
            size_t newSize = calcRectSize(leftEdge, pinToRectCurrent);
            double factor = (double)newSize / oldSize;

#pragma warning(disable:4244)
            l = pinToRectCurrent->left + ((ric->originalRect.left - pinToRectInitial->left) * (factor));
#pragma warning(disable:4244)

            break;
        }

        default :
            break;
    }

    return l;
}

/**
 * Returns the new top co-ordinate for dialog control specified through its
 * resize info, for the edge specified.
 *
 * @param prid
 * @param ric
 * @param edge
 *
 * @return int32_t
 */
static int32_t newTop(pResizeInfoDlg prid, pResizeInfoCtrl ric, pEdge edge)
{
    RECT *pinToRectInitial = getRect(edge->pinToID, prid, true);
    RECT *pinToRectCurrent = getRect(edge->pinToID, prid, false);

    int32_t t = 0;

    switch ( edge->pinType )
    {
        case stationaryPin :
            t = ric->originalRect.top - edgeCoord(edge->pinToEdge, pinToRectInitial) + edgeCoord(edge->pinToEdge, pinToRectCurrent);
            break;

        case proportionalPin :
        {
            size_t oldSize = calcRectSize(topEdge, pinToRectInitial);
            size_t newSize = calcRectSize(topEdge, pinToRectCurrent);
            double factor  = (double)newSize / oldSize;

#pragma warning(disable:4244)
            t = pinToRectCurrent->top + ((ric->originalRect.top - pinToRectInitial->top) * (factor));
#pragma warning(default:4244)

            break;
        }

        default :
            break;
    }

    return t;
}


/**
 * Returns the new right co-ordinate for dialog control specified through its
 * resize info, for the edge specified.
 *
 * @param prid
 * @param ric
 * @param edge
 *
 * @return int32_t
 */
static int32_t newRight(pResizeInfoDlg prid, pResizeInfoCtrl ric, pEdge edge)
{
    RECT *pinToRectInitial = getRect(edge->pinToID, prid, true);
    RECT *pinToRectCurrent = getRect(edge->pinToID, prid, false);

    int32_t r = 0;

    switch ( edge->pinType )
    {
        case stationaryPin :
            r = ric->originalRect.right - edgeCoord(edge->pinToEdge, pinToRectInitial) + edgeCoord(edge->pinToEdge, pinToRectCurrent);
            break;

        case proportionalPin :
        {
            size_t oldSize = calcRectSize(rightEdge, pinToRectInitial);
            size_t newSize = calcRectSize(rightEdge, pinToRectCurrent);
            double factor  = (double)newSize / oldSize;

#pragma warning(disable:4244)
            r = pinToRectCurrent->right + ((ric->originalRect.right - pinToRectInitial->right) * (factor));
#pragma warning(default:4244)

            break;
        }

        case myLeftPin :
            r = ric->currentRect.left + (ric->originalRect.right - ric->originalRect.left);
            break;

        default :
            break;
    }

    return r;
}


/**
 * Returns the new bottom co-ordinate for dialog control specified through its
 * resize info, for the edge specified.
 *
 * @param prid
 * @param ric
 * @param edge
 *
 * @return int32_t
 */
static int32_t newBottom(pResizeInfoDlg prid, pResizeInfoCtrl ric, pEdge edge)
{
    RECT *pinToRectInitial = getRect(edge->pinToID, prid, true);
    RECT *pinToRectCurrent = getRect(edge->pinToID, prid, false);

    int32_t b = 0;

    switch ( edge->pinType )
    {
        case stationaryPin :
            b = ric->originalRect.bottom - edgeCoord(edge->pinToEdge, pinToRectInitial) + edgeCoord(edge->pinToEdge, pinToRectCurrent);
            break;

        case proportionalPin :
        {
            size_t oldSize = calcRectSize(bottomEdge, pinToRectInitial);
            size_t newSize = calcRectSize(bottomEdge, pinToRectCurrent);
            double factor  = (double)newSize / oldSize;

#pragma warning(disable:4244)
            b = pinToRectCurrent->bottom + ((ric->originalRect.bottom - pinToRectInitial->bottom) * (factor));
#pragma warning(default:4244)

            break;
        }

        case myTopPin :
            b = ric->currentRect.top + (ric->originalRect.bottom - ric->originalRect.top);
            break;

        default :
            break;
    }

    return b;
}


/**
 * Recalculate and set the current window position in the specified control
 * resize info struct.
 *
 * @param prid
 * @param ric
 */
static void recalcSizePosition(pResizeInfoDlg prid, pResizeInfoCtrl ric)
{
    ric->currentRect.left = newLeft(prid, ric, &ric->edges.left);

    ric->currentRect.top = newTop(prid, ric, &ric->edges.top);

    ric->currentRect.right = newRight(prid, ric, &ric->edges.right);

    ric->currentRect.bottom = newBottom(prid, ric, &ric->edges.bottom);
}

/**
 * Resizes and repositions the specified dialog control to its new size and
 * position, using DeferWindowPos(), if the control window is visible.
 *
 * If the window is not visible, the DeferWindowPos() documentation says it will
 * fail.  When not visible we just use MoveWindow() to immediately do the
 * resizing and repositioning.  Since we are only using DeferWindowPos() to
 * reduce flicker, and invisible windows will not produce flicker, this works
 * well.
 *
 * @param hdwp
 * @param ric
 *
 * @return HDWP
 */
static HDWP deferPosition(HDWP hdwp, pResizeInfoCtrl ric)
{
    if ( ! IsWindow(ric->hCtrl) )
    {
        return hdwp;
    }

    int32_t left   = ric->currentRect.left;
    int32_t top    = ric->currentRect.top;
    int32_t right  = ric->currentRect.right;
    int32_t bottom = ric->currentRect.bottom;

    int32_t width  = right - left;
    int32_t height = bottom - top;

    if ( IsWindowVisible(ric->hCtrl) )
    {
        return DeferWindowPos(hdwp, ric->hCtrl, NULL, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    // For not visible windows, just move them.
    MoveWindow(ric->hCtrl, left, top, width, height, FALSE);

    return hdwp;
}

/**
 * Resizes and repositions all dialog controls in the dialog using defer window
 * position to do them all at onc..
 *
 * @param prid
 *
 * @return bool
 */
static bool resizeAll(pResizeInfoDlg prid, pCPlainBaseDialog pcpbd, HWND hDlg)
{
    HDWP hdwp = BeginDeferWindowPos((int32_t)prid->countCtrls);
    if ( hdwp == NULL )
    {
        return false;
    }

    for ( size_t i = 0; i < prid->countCtrls; i++)
    {
        hdwp = deferPosition(hdwp, &prid->riCtrls[i]);
        if ( hdwp == NULL )
        {
            return false;
        }
    }

    return EndDeferWindowPos(hdwp) ? true : false;
}

/**
 * The handler for the WM_SIZE message.
 *
 * Calculates the new size for all the dialog controls in the dialog and then
 * resizes / repositions them all.
 *
 * @param pcpbd
 * @param hDlg
 * @param cx
 * @param cy
 *
 * @return BOOL
 */
BOOL resizeAndPosition(pCPlainBaseDialog pcpbd, HWND hDlg, long cx, long cy)
{
    pResizeInfoDlg prid = pcpbd->resizeInfo;

    prid->currentRect.right  = cx;
    prid->currentRect.bottom = cy;
    for ( size_t i = 0; i < prid->countCtrls; i++)
    {
        recalcSizePosition(prid, &prid->riCtrls[i]);
    }

    return resizeAll(prid, pcpbd, hDlg) ? TRUE : FALSE;
}

/**
 *  Called from WM_INITDIALOG to finish the initialization of a resizable
 *  dialog.
 *
 *  Gets the original window rectangles for the dialog and the controls.
 *  Enumerates all child windows of the dialog to either fix up their existing
 *  control info structs, or to add a record for any controls not specified by
 *  the user.  Verifies that all records added by the user refer to an actual
 *  window.  And does any other needed miscellaneous set up.
 *
 * @param hDlg
 * @param c
 * @param pcpbd
 *
 * @return LRESULT
 */
LRESULT initializeResizableDialog(HWND hDlg, RexxThreadContext *c, pCPlainBaseDialog pcpbd)
{
    pResizeInfoDlg prid = pcpbd->resizeInfo;

    setResizableDlgStyle(hDlg);

    GetClientRect(hDlg, &prid->originalRect);

    if ( prid->minSizeIsInitial )
    {
        prid->minSize.cx  = prid->originalRect.right;
        prid->minSize.cy  = prid->originalRect.bottom;
        prid->haveMinSize = true;
    }

    size_t originalCount = prid->countCtrls;

    EnumChildWindows(hDlg, InitializeAllControlsProc, (LPARAM)pcpbd);

    if ( ! verifyWindows(c, hDlg, prid, originalCount) )
    {
        endDialogPremature(pcpbd, hDlg, RexxConditionRaised);
        return FALSE;
    }

    return prid->haveError ? FALSE : TRUE;
}

/**
 * Given the window handle to a tab control and the dialog it is in, calculates
 * the size and position, in dialog client area coordinates of the tab control's
 * display rectangle.
 *
 * @param hTab
 * @param hDlg
 * @param p          Position returned in this point struct.
 * @param s          Size returned in this size struct.
 */
void calcDisplayRect(HWND hTab, HWND hDlg, POINT *p, SIZE *s)
{
    RECT r = { 0 };
    GetWindowRect(hTab, &r);

    TabCtrl_AdjustRect(hTab, false, &r);

    s->cx = r.right - r.left;
    s->cy = r.bottom - r.top;
    p->x  = r.left;
    p->y  = r.top;
    ScreenToClient(hDlg, p);
}


/**
 * Used to invoke a method in the Rexx dialog for the EXITSIZEMOVE event.
 *
 * @param pcpbd
 */
void notifyExitSizeMove(pCPlainBaseDialog pcpbd)
{
    RexxThreadContext *c    = pcpbd->dlgProcContext;
    RexxArrayObject    args = c->NewArray(0);
    pResizeInfoDlg     prid = pcpbd->resizeInfo;

    if ( prid->sizeEndedWillReply )
    {
        invokeDirect(c, pcpbd, prid->sizeEndedMeth, args);
    }
    else
    {
        invokeDispatch(c, pcpbd, prid->sizeEndedMeth, args);
    }

    c->ReleaseLocalReference(args);
}


/**
 * Called from one of the Rexx window message processing loops to handle the
 * messages related to resizing.
 *
 * @param hDlg
 * @param msg
 * @param wParam
 * @param lParam
 * @param pcpbd
 *
 * @return ReplyTrue or ReplyFalse to return immediately from the message
 *         processing loop.  Returns ContinueProcessing to indicate that the
 *         message processing loop should continue processing this message.
 *
 * @assumes  That the caller has already assured that the dialog is a
 *           ResizingAdmin dialog.
 */
MsgReplyType handleResizing(HWND hDlg, uint32_t msg, WPARAM wParam, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    pResizeInfoDlg prid = pcpbd->resizeInfo;

    switch ( msg )
    {
        case WM_ENTERSIZEMOVE :
        {
            prid->inSizeOrMove = true;
            return ReplyFalse;
        }

        case WM_NCCALCSIZE :
        {
            if ( wParam && prid->inSizeOrMove )
            {
                NCCALCSIZE_PARAMS * nccs_params = (NCCALCSIZE_PARAMS *)lParam;

                // Have the default window procedure calculate the client
                // co-ordinates.
                DefWindowProc(hDlg, msg, FALSE, (LPARAM)&nccs_params->rgrc[0]);

                // Set the source & target rectangles to be the same.
                nccs_params->rgrc[1] = nccs_params->rgrc[2];

                setWindowPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(WVR_ALIGNLEFT | WVR_ALIGNTOP));
                return ReplyTrue;
            }
            return ReplyFalse;
        }

        case WM_SIZE :
        {
            if ( prid->inSizeOrMove || wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED )
            {
                prid->isSizing = true;
                BOOL ret = resizeAndPosition(pcpbd, hDlg, LOWORD(lParam), HIWORD(lParam));

                if ( prid->countPagedTabs > 0 && pcpbd->countChilds > 0 )
                {
                    handleChildDialogs(pcpbd, hDlg);
                }

                return ret ? ReplyTrue : ReplyFalse;
            }
            return ReplyFalse;
        }

        case WM_SIZING :
        {
            return ReplyFalse;
        }

        case WM_EXITSIZEMOVE :
        {
            if ( prid->countPagedTabs > 0 && prid->isSizing )
            {
                maybeRedrawWindows(prid);
            }

            if ( prid->isSizing && prid->sizeEndedMeth != NULL)
            {
                notifyExitSizeMove(pcpbd);
            }

            prid->inSizeOrMove = false;
            prid->isSizing     = false;

            return ReplyFalse;
        }

        case WM_GETMINMAXINFO :
        {
            MINMAXINFO *pmmi = (MINMAXINFO *)lParam;

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

            return ReplyTrue;
        }

        default :
            break;
    }

    return ContinueProcessing;
}

/**
 * Allocates the ResizeInfoDlg struct and sets up the default values.  Also
 * passes on the CSelf buffer to be set as a context variable for the
 * ResizingAdmin object.
 *
 * @param c
 * @param pcpbd
 * @param cselfBuffer
 *
 * @return bool
 *
 * @note that cselfBuffer is the Rexx buffer object containing CPlainBaseDialog.
 */
bool allocateResizeInfo(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxBufferObject cselfBuffer)
{
    pResizeInfoDlg prid = (pResizeInfoDlg)LocalAlloc(LPTR, sizeof(ResizeInfoDlg));
    if ( prid == NULL )
    {
        goto err_out;
    }

    prid->riCtrls = (pResizeInfoCtrl)LocalAlloc(LPTR, DEFAULT_RESIZABLE_CONTROLS * sizeof(ResizeInfoCtrl));
    if ( prid->riCtrls == NULL )
    {
        goto err_out;
    }
    prid->tableSize = DEFAULT_RESIZABLE_CONTROLS;

    prid->defEdges.left.pinToEdge   = leftEdge;
    prid->defEdges.left.pinType     = proportionalPin;
    prid->defEdges.top.pinToEdge    = topEdge;
    prid->defEdges.top.pinType      = proportionalPin;
    prid->defEdges.right.pinToEdge  = rightEdge;
    prid->defEdges.right.pinType    = proportionalPin;
    prid->defEdges.bottom.pinToEdge = bottomEdge;
    prid->defEdges.bottom.pinType   = proportionalPin;

    prid->minSizeIsInitial = true;

    pcpbd->resizeInfo = prid;

    RexxObjectPtr reply = c->SendMessage1(pcpbd->rexxSelf, "INITRESIZING", cselfBuffer);
    if ( reply != TheZeroObj )
    {
        goto err_out;
    }

    if ( TheConstDirUsage == globalNever )
    {
        RexxDirectoryObject constDir = (RexxDirectoryObject)c->SendMessage0(pcpbd->rexxSelf, "CONSTDIR");
        if ( constDir != NULLOBJECT )
        {
            c->DirectoryPut(constDir, c->Int32(IDC_DEFAULT_PINTO_WINDOW), "IDC_DEFAULT_PINTO_WINDOW");
        }
    }
    else
    {
        c->DirectoryPut(TheConstDir, c->Int32(IDC_DEFAULT_PINTO_WINDOW), "IDC_DEFAULT_PINTO_WINDOW");
    }

    return true;

err_out:
    outOfMemoryException(c->threadContext);
    if ( prid != NULL )
    {
        safeLocalFree(prid->riCtrls);
    }
    safeLocalFree(prid);
    pcpbd->resizeInfo = NULL;

    return false;
}

/**
 * This is the callback function for our EnumChildWindows() call in
 * WM_INITDIALOG.  For each child window, we capture and save its original size,
 * save its window handle, determine and save its window type, and create a
 * resize info struct for the control if one does not exist already.
 *
 * @param hCtrl
 * @param lParam
 *
 * @return True to continue the enumeration, false to halt the enumeration.
 *
 * @note  We only want direct children of the dialog.  Some controls have their
 *        own child windows.  These are enumerated by EnumChildWindows, but we
 *        can not work with them.  For instance, a list-view with a column
 *        header has the header control as a child.  We can not work with the
 *        header control.  So we skip any child window whose parent is not the
 *        dialog.
 */
BOOL CALLBACK InitializeAllControlsProc(HWND hCtrl, LPARAM lParam)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)lParam;
    pResizeInfoDlg    prid  = pcpbd->resizeInfo;

    if ( GetParent(hCtrl) != pcpbd->hDlg )
    {
        return TRUE;
    }

    oodControl_t ctrlType = controlHwnd2controlType(hCtrl);
    int          ctrlID   = GetDlgCtrlID(hCtrl);
    if ( ctrlType == winUnknown )
    {
        return TRUE;
    }

    if ( ctrlType == winGroupBox || ctrlType == winStatic )
    {
        SetWindowLong(hCtrl, GWL_EXSTYLE, GetWindowLong(hCtrl, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
    }

    pResizeInfoCtrl ric = getControlInfo(prid, ctrlID);
    if ( ric == NULL )
    {
        ric = allocCtrlInfo(pcpbd->dlgProcContext, prid, ctrlID, hCtrl, ctrlType);
        if ( ric == NULL )
        {
            prid->haveError = true;
            return FALSE;
        }
    }
    else
    {
        ric->hCtrl    = hCtrl;
        ric->ctrlType = ctrlType;
    }

    GetWindowRect(hCtrl, &ric->originalRect);
    MapWindowPoints(NULL, pcpbd->hDlg, (POINT *)&ric->originalRect, 2);

#ifdef EXTRA_DEBUG
    printf("Control hwnd=%p type=%s id=%d rect=(%d, %d, %d, %d)\n", hCtrl, controlType2controlName(ctrlType), ric->id,
           ric->originalRect.left, ric->originalRect.top, ric->originalRect.right, ric->originalRect.bottom);
#endif

    return TRUE;
}

/**
 * The window message processing procedure for dialogs that have inherited
 * ResizingAdmin.
 *
 * This is based on the normal Rexx dialog message processing procedure,
 * RexxDlgProc() except that it intercepts WM_xxx messages related to resizing.
 * These messages are processed here, always, and they are never used to invoke
 * a Rexx dialog method.  This implies that if the user were to connect, say
 * WM_SIZE, no method would be invoked.
 *
 * The only exception to this is that the user can request a notification for
 * WM_EXITSIZEMOVE through ResizingAdmin::wantSizeEnded()
 *
 * PropertySheetDialog dialogs need to have the Windows PropertySheet control
 * subclassed to handle resizing.  That suclass procedure,
 * ResizableSubclassProc(), is similar to this and handles the resizng for
 * PropertySheetDialog and all PropertySheetPage dialogs.
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT CALLBACK
 */
LRESULT CALLBACK RexxResizableDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)lParam;
        if ( pcpbd == NULL )
        {
            // Theoretically impossible.  But ... if it should happen, abort.
            return endDialogPremature(pcpbd, hDlg, NoPCPBDpased);
        }

        if ( pcpbd->dlgProcContext == NULL )
        {
            RexxThreadContext *context;
            if ( ! pcpbd->interpreter->AttachThread(&context) )
            {
                // Again, this shouldn't happen ... but
                return endDialogPremature(pcpbd, hDlg, NoThreadAttach);
            }
            pcpbd->dlgProcContext = context;

            RexxSetProcessMessages(FALSE);
        }

        pcpbd->hDlg = hDlg;
        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pcpbd);

        if ( pcpbd->isCustomDrawDlg && pcpbd->idsNotChecked )
        {
            // We don't care what the outcome of this is, customDrawCheckIDs
            // will take care of aborting this dialog if the IDs are bad.
            customDrawCheckIDs(pcpbd);
        }

        return initializeResizableDialog(hDlg, pcpbd->dlgProcContext, pcpbd);
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

    // We first deal with resizable stuff, then handle the rest with the normal
    // ooDialog process.
    MsgReplyType resizingReply = handleResizing(hDlg, uMsg, wParam, lParam, pcpbd);
    if ( resizingReply != ContinueProcessing )
    {
        return (resizingReply == ReplyTrue ? TRUE : FALSE);
    }

    bool msgEnabled = IsWindowEnabled(hDlg) ? true : false;

    // Do not search message table for WM_PAINT to improve redraw.
    if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
    {
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

        default:
            break;
    }

    return FALSE;
}


/** Resizing::maxSize          [attribute]
 *
 *  By default resizable dialogs have no max size.  This can be changed through
 *  the maxSize attribute.  If no maxSize is set, the value of this attribute is
 *  the .nil object.
 */
RexxMethod1(RexxObjectPtr, ra_maxSize, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    if ( prid->haveMaxSize )
    {
        return rxNewSize(context, (PORXSIZE)&prid->maxSize);
    }

    return TheNilObj;
}
RexxMethod2(RexxObjectPtr, ra_setMaxSize, RexxObjectPtr, _size, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    PSIZE s = (PSIZE)rxGetSize(context, _size, 1);
    if ( s != NULL )
    {
        prid->maxSize.cx = s->cx;
        prid->maxSize.cy = s->cy;
        prid->haveMaxSize = true;
    }

    return NULLOBJECT;
}

/** Resizing::minSize          [attribute]
 *
 *  By default the minimum size for resizable dialogs is the size of the dialog
 *  when it is first created.  Basically this will be when it is shown on the
 *  screen.
 *
 *  Note that keeping a dialog hidden and increassing or decreasing its size in
 *  initDialog() will not change the intial size.
 *
 *  Use the noMinSize() method to change to no minimum size.  No minimum size
 *  can not be set by changing the attribute.  The minSize attribute can only be
 *  set to a .Size object.  Setting the attribute to a .Size with 0 width and
 *  height, *may*, work, but it also may have unknown consequences.
 */
RexxMethod1(RexxObjectPtr, ra_minSize, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    if ( prid->haveMinSize )
    {
        return rxNewSize(context, (PORXSIZE)&prid->minSize);
    }

    return TheNilObj;
}
RexxMethod2(RexxObjectPtr, ra_setMinSize, RexxObjectPtr, _size, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    PSIZE s = (PSIZE)rxGetSize(context, _size, 1);
    if ( s != NULL )
    {
        prid->minSize.cx  = s->cx;
        prid->minSize.cy  = s->cy;
        prid->haveMinSize = true;
        prid->minSizeIsInitial = false;
    }

    return NULLOBJECT;
}

/** ResizingAdmin::controlBottom()
 ** ResizingAdmin::controlLeft()
 ** ResizingAdmin::controlRight()
 ** ResizingAdmin::controlTop()
 *
 */
RexxMethod6(RexxObjectPtr, ra_controlSide, RexxObjectPtr, rxID, CSTRING, howPinned, CSTRING, whichEdge,
            OPTIONAL_RexxObjectPtr, _pinToID, NAME, method, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        goto err_out;
    }

    if ( ! prid->inDefineSizing )
    {
        methodCanOnlyBeInvokedException(context, method, "during the defineSizing method", pcpbd->rexxSelf);
        goto err_out;
    }

    uint32_t ctrlID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxID, -1, 1, true);
    if ( ctrlID == OOD_INVALID_ITEM_ID )
    {
        goto err_out;
    }

    uint32_t pinToID = IDC_DEFAULT_PINTO_WINDOW;
    if ( argumentExists(4) )
    {
        pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, _pinToID, -1, 4, false);
        if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
        {
            if ( pinToID == (uint32_t)-1 )
            {
                wrongArgValueException(context->threadContext, 4, "a valid numeric ID or a valid symbolic ID" , _pinToID);
            }
            goto err_out;
        }
    }

    pResizeInfoCtrl ric = getValidatedControlInfo(context, ctrlID, pinToID, prid);
    if ( ric == NULL )
    {
        goto err_out;
    }

    pinnedEdge_t edge;
    if (      method[7] == 'L' ) edge = leftEdge;
    else if ( method[7] == 'T' ) edge = topEdge;
    else if ( method[7] == 'R' ) edge = rightEdge;
    else if ( method[7] == 'B' ) edge = bottomEdge;

    pinType_t    pinType   = keyword2pinType(context, howPinned, 2, edge);
    pinnedEdge_t pinToEdge = keyword2pinnedEdge(context, whichEdge, 3);

    if ( pinType == notAPin || pinToEdge == notAnEdge )
    {
        goto err_out;
    }

    switch ( method[7] )
    {
        case 'L' :
            ric->edges.left.pinType   = pinType;
            ric->edges.left.pinToEdge = pinToEdge;
            ric->edges.left.pinToID   = pinToID;
            break;

        case 'T' :
            ric->edges.top.pinType   = pinType;
            ric->edges.top.pinToEdge = pinToEdge;
            ric->edges.top.pinToID   = pinToID;
            break;

        case 'R' :
            ric->edges.right.pinType   = pinType;
            ric->edges.right.pinToEdge = pinToEdge;
            ric->edges.right.pinToID   = pinToID;
            break;

        case 'B' :
            ric->edges.bottom.pinType   = pinType;
            ric->edges.bottom.pinToEdge = pinToEdge;
            ric->edges.bottom.pinToID   = pinToID;
            break;

        default :
            goto err_out;
    }

    return TheZeroObj;

err_out:
    return TheOneObj;
}


/** ResizingAdmin::controlSizing()
 *
 *
 */
RexxMethod6(RexxObjectPtr, ra_controlSizing, RexxObjectPtr, rxID, OPTIONAL_RexxArrayObject, left, OPTIONAL_RexxArrayObject, top,
            OPTIONAL_RexxArrayObject, right, OPTIONAL_RexxArrayObject, bottom, CSELF, pCSelf)
{
    pResizeInfoDlg    prid  = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        goto err_out;
    }

    if ( ! prid->inDefineSizing )
    {
        methodCanOnlyBeInvokedException(context, "controlSizing", "during the defineSizing method", pcpbd->rexxSelf);
        goto err_out;
    }

    uint32_t ctrlID  = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxID, -1, 1, true);
    if ( ctrlID == OOD_INVALID_ITEM_ID )
    {
        goto err_out;
    }

    pResizeInfoCtrl ric = getUnValidatedControlInfo(context, ctrlID, prid);
    if ( ric == NULL )
    {
        goto err_out;
    }

    RexxObjectPtr rxPinToID;
    RexxObjectPtr rxHowPinned;
    RexxObjectPtr rxWhichEdge;
    uint32_t      pinToID;

    if ( argumentExists(2) )
    {
        rxHowPinned = context->ArrayAt(left, 1);
        rxWhichEdge = context->ArrayAt(left, 2);
        rxPinToID   = context->ArrayAt(left, 3);
        if ( rxHowPinned == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 2, 1);
            goto err_out;
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 2, 2);
            goto err_out;
        }

        pinToID = IDC_DEFAULT_PINTO_WINDOW;
        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 2, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 2, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                goto err_out;
            }
        }

        if ( ! validPinToID(context, ctrlID, pinToID, prid) )
        {
            goto err_out;
        }

        pinType_t    pinType   = keyword2pinType(context, context->ObjectToStringValue(rxHowPinned), 2, leftEdge);
        pinnedEdge_t pinToEdge = keyword2pinnedEdge(context, context->ObjectToStringValue(rxWhichEdge), 2);

        if ( pinType == notAPin || pinToEdge == notAnEdge )
        {
            goto err_out;
        }

        ric->edges.left.pinType   = pinType;
        ric->edges.left.pinToEdge = pinToEdge;
        ric->edges.left.pinToID   = pinToID;
    }

    if ( argumentExists(3) )
    {
        rxHowPinned = context->ArrayAt(top, 1);
        rxWhichEdge = context->ArrayAt(top, 2);
        rxPinToID   = context->ArrayAt(top, 3);
        if ( rxHowPinned == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 3, 1);
            goto err_out;
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 3, 2);
            goto err_out;
        }

        pinToID = IDC_DEFAULT_PINTO_WINDOW;
        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 3, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 3, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                goto err_out;
            }
        }

        if ( ! validPinToID(context, ctrlID, pinToID, prid) )
        {
            goto err_out;
        }

        pinType_t    pinType   = keyword2pinType(context, context->ObjectToStringValue(rxHowPinned), 3, topEdge);
        pinnedEdge_t pinToEdge = keyword2pinnedEdge(context, context->ObjectToStringValue(rxWhichEdge), 3);

        if ( pinType == notAPin || pinToEdge == notAnEdge )
        {
            goto err_out;
        }

        ric->edges.top.pinType   = pinType;
        ric->edges.top.pinToEdge = pinToEdge;
        ric->edges.top.pinToID   = pinToID;
    }

    if ( argumentExists(4) )
    {
        rxHowPinned = context->ArrayAt(right, 1);
        rxWhichEdge = context->ArrayAt(right, 2);
        rxPinToID   = context->ArrayAt(right, 3);
        if ( rxHowPinned == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 4, 1);
            goto err_out;
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 4, 2);
            goto err_out;
        }

        pinToID = IDC_DEFAULT_PINTO_WINDOW;
        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 4, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 4, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                goto err_out;
            }
        }

        if ( ! validPinToID(context, ctrlID, pinToID, prid) )
        {
            goto err_out;
        }

        pinType_t    pinType   = keyword2pinType(context, context->ObjectToStringValue(rxHowPinned), 4, rightEdge);
        pinnedEdge_t pinToEdge = keyword2pinnedEdge(context, context->ObjectToStringValue(rxWhichEdge), 4);

        if ( pinType == notAPin || pinToEdge == notAnEdge )
        {
            goto err_out;
        }

        ric->edges.right.pinType   = pinType;
        ric->edges.right.pinToEdge = pinToEdge;
        ric->edges.right.pinToID   = pinToID;
    }

    if ( argumentExists(5) )
    {
        rxHowPinned = context->ArrayAt(bottom, 1);
        rxWhichEdge = context->ArrayAt(bottom, 2);
        rxPinToID   = context->ArrayAt(bottom, 3);
        if ( rxHowPinned == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 5, 1);
            goto err_out;
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 5, 2);
            goto err_out;
        }

        pinToID = IDC_DEFAULT_PINTO_WINDOW;
        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 5, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 5, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                goto err_out;
            }
        }

        if ( ! validPinToID(context, ctrlID, pinToID, prid) )
        {
            goto err_out;
        }

        pinType_t    pinType   = keyword2pinType(context, context->ObjectToStringValue(rxHowPinned), 5, bottomEdge);
        pinnedEdge_t pinToEdge = keyword2pinnedEdge(context, context->ObjectToStringValue(rxWhichEdge), 5);

        if ( pinType == notAPin || pinToEdge == notAnEdge )
        {
            goto err_out;
        }

        ric->edges.bottom.pinType   = pinType;
        ric->edges.bottom.pinToEdge = pinToEdge;
        ric->edges.bottom.pinToID   = pinToID;
    }

    return TheZeroObj;

err_out:
    return TheOneObj;
}


/** ResizingAdmin::defaultBottom()
 ** ResizingAdmin::defaultLeft()
 ** ResizingAdmin::defaultRight()
 ** ResizingAdmin::defaultTop()
 *
 */
RexxMethod5(RexxObjectPtr, ra_defaultSide, CSTRING, howPinned, CSTRING, whichEdge, OPTIONAL_RexxObjectPtr, _pinToID,
            NAME, method, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    if ( ! prid->inDefineSizing )
    {
        return methodCanOnlyBeInvokedException(context, method, "during the defineSizing method", pcpbd->rexxSelf);
    }

    uint32_t pinToID = IDC_DEFAULT_PINTO_WINDOW;
    if ( argumentExists(3) )
    {
        pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, _pinToID, -1, 3, false);
        if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
        {
            if ( pinToID == (uint32_t)-1 )
            {
                wrongArgValueException(context->threadContext, 3, "a valid numeric ID or a valid symbolic ID" , _pinToID);
            }
            return TheOneObj;
        }
    }

    if ( ! validDefaultPinToID(context, pinToID, prid) )
    {
        return TheOneObj;
    }

    switch ( method[7] )
    {
        case 'L' : return defaultLeft(context, howPinned, whichEdge, pinToID, prid);
        case 'T' : return defaultTop(context, howPinned, whichEdge, pinToID, prid);
        case 'R' : return defaultRight(context, howPinned, whichEdge, pinToID, prid);
        case 'B' : return defaultBottom(context, howPinned, whichEdge, pinToID, prid);
    }
    return TheOneObj;
}


/** ResizingAdmin::defaultSizing()
 *
 *
 */
RexxMethod5(RexxObjectPtr, ra_defaultSizing, OPTIONAL_RexxArrayObject, left, OPTIONAL_RexxArrayObject, top,
            OPTIONAL_RexxArrayObject, right, OPTIONAL_RexxArrayObject, bottom, CSELF, pCSelf)
{
    pResizeInfoDlg    prid  = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    if ( ! prid->inDefineSizing )
    {
        return methodCanOnlyBeInvokedException(context, "defaultSizing", "during the defineSizing method", pcpbd->rexxSelf);
    }

    RexxObjectPtr rxHowPinned;
    RexxObjectPtr rxWhichEdge;
    RexxObjectPtr rxPinToID;

    CSTRING  howPinned;
    CSTRING  whichEdge;
    uint32_t pinToID;

    if ( argumentExists(1) )
    {
        rxHowPinned = context->ArrayAt(left, 1);
        rxWhichEdge = context->ArrayAt(left, 2);
        rxPinToID   = context->ArrayAt(left, 3);

        if ( rxHowPinned == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 1, 1);
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 1, 2);
        }

        howPinned = context->ObjectToStringValue(rxHowPinned);
        whichEdge = context->ObjectToStringValue(rxWhichEdge);
        pinToID   = IDC_DEFAULT_PINTO_WINDOW;

        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 3, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 1, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                return NULLOBJECT;
            }

            if ( ! validDefaultPinToID(context, pinToID, prid) )
            {
                return NULLOBJECT;
            }
        }

        if ( defaultLeft(context, howPinned, whichEdge, pinToID, prid) == TheOneObj )
        {
            return NULLOBJECT;
        }
    }

    if ( argumentExists(2) )
    {
        rxHowPinned = context->ArrayAt(top, 1);
        rxWhichEdge = context->ArrayAt(top, 2);
        rxPinToID   = context->ArrayAt(top, 3);

        if ( rxHowPinned == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 2, 1);
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 2, 2);
        }

        howPinned = context->ObjectToStringValue(rxHowPinned);
        whichEdge = context->ObjectToStringValue(rxWhichEdge);
        pinToID   = IDC_DEFAULT_PINTO_WINDOW;

        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 3, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 2, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                return NULLOBJECT;
            }

            if ( ! validDefaultPinToID(context, pinToID, prid) )
            {
                return NULLOBJECT;
            }
        }

        if ( defaultTop(context, howPinned, whichEdge, pinToID, prid) == TheOneObj )
        {
            return NULLOBJECT;
        }
    }

    if ( argumentExists(3) )
    {
        rxHowPinned = context->ArrayAt(right, 1);
        rxWhichEdge = context->ArrayAt(right, 2);
        rxPinToID   = context->ArrayAt(right, 3);

        if ( rxHowPinned == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 3, 1);
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 3, 2);
        }

        howPinned = context->ObjectToStringValue(rxHowPinned);
        whichEdge = context->ObjectToStringValue(rxWhichEdge);
        pinToID   = IDC_DEFAULT_PINTO_WINDOW;

        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 3, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 3, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                return NULLOBJECT;
            }

            if ( ! validDefaultPinToID(context, pinToID, prid) )
            {
                return NULLOBJECT;
            }
        }

        if ( defaultRight(context, howPinned, whichEdge, pinToID, prid) == TheOneObj )
        {
            return NULLOBJECT;
        }
    }

    if ( argumentExists(4) )
    {
        rxHowPinned = context->ArrayAt(bottom, 1);
        rxWhichEdge = context->ArrayAt(bottom, 2);
        rxPinToID   = context->ArrayAt(bottom, 3);

        if ( rxHowPinned == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 4, 1);
        }
        if ( rxWhichEdge == NULLOBJECT )
        {
            return sparseArrayException(context->threadContext, 4, 2);
        }

        howPinned = context->ObjectToStringValue(rxHowPinned);
        whichEdge = context->ObjectToStringValue(rxWhichEdge);
        pinToID   = IDC_DEFAULT_PINTO_WINDOW;

        if ( rxPinToID != NULLOBJECT )
        {
            pinToID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxPinToID, -1, 3, false);
            if ( pinToID == OOD_INVALID_ITEM_ID || pinToID == (uint32_t)-1 )
            {
                if ( pinToID == (uint32_t)-1 )
                {
                    wrongObjInArrayException(context->threadContext, 4, 3, "a valid numeric ID or a valid symbolic ID", rxPinToID);
                }
                return NULLOBJECT;
            }

            if ( ! validDefaultPinToID(context, pinToID, prid) )
            {
                return NULLOBJECT;
            }
        }

        if ( defaultBottom(context, howPinned, whichEdge, pinToID, prid) == TheOneObj )
        {
            return NULLOBJECT;
        }
    }

    return TheZeroObj;
}


/** ResizingAdmin::initResizing()  [private]
 *
 *  Used to set the CSelf Rexx buffer object as a context variable of this
 *  object.
 *
 *  @note For internal use only.
 */
RexxMethod2(RexxObjectPtr, ra_initResizing, RexxObjectPtr, arg, OSELF, self)
{
    if ( ! context->IsBuffer(arg) )
    {
        baseClassInitializationException(context, "ResizingAdmin");
        return TheOneObj;
    }

    context->SetObjectVariable("CSELF", arg);

    return TheZeroObj;
}


/** ResizingAdmin::noMaxSize()
 *
 *  Sets the resizable dialog so that it has no maximum size.  By default, a
 *  resizable dialog has not maximum size.  This method only needs to be used if
 *  a maximum size was set for the dialog and then it is desired to remove that
 *  maximum size.
 *
 *  @return  Returns 0, always.
 */
RexxMethod1(RexxObjectPtr, ra_noMaxSize, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd != NULL )
    {
        prid->haveMaxSize = false;
    }
    return TheZeroObj;
}


/** ResizingAdmin::noMinSize()
 *
 *  Sets the resizable dialog so that it has no minimum size.  By default,
 *  resizable dialogs have a minimum size equal to their size when the
 *  underlying Windows dialog is first created.
 *
 *  @return  Returns 0, always.
 */
RexxMethod1(RexxObjectPtr, ra_noMinSize, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd != NULL )
    {
        prid->minSizeIsInitial = false;
        prid->haveMinSize      = false;
    }
    return TheZeroObj;
}


/** ResizingAdmin::pagedTab()
 *
 *  Adds the data necessary to properly resize the dialogs used as pages in a
 *  tab control.
 *
 *  @param rxTabID  The resource ID of the tab control that has control dialogs
 *                  for pages.  Mayb be numeric or symbolic.
 *
 *  @param dlgIDs   An array of the dialog IDs of each control dialog.  Each
 *                  item in the array is assumed to be the dlgID of one of the
 *                  dialog pages.  Each item in the array may be numeric or
 *                  symbolic.  The items must be valid dialog IDs, that is a
 *                  whole number greater than 0, or a sysmbolic ID that resolves
 *                  to a whole number gereater than 0.
 *
 *                  The array must not be sparse and the number of items can not
 *                  exceed the maximum number of child dialogs, which is 20.
 *                  The order of the IDs does not matter.
 *
 *  @return  0 on success.  On any detected error a syntax condition is raised.
 *
 *  @notes  A paged tab is a tab control that uses ControlDialog dialogs as the
 *          contents for its pages.  A resizable dialog can only define up to 4
 *          paged tabs.
 *
 *          In order for the resizing admin to properly resize ControlDialog
 *          dialogs embedded in a tab control, the pagedTab method must be used
 *          to define each paged tab.
 */
RexxMethod3(RexxObjectPtr, ra_pagedTab, RexxObjectPtr, rxTabID, RexxArrayObject, dlgIDs, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    if ( ! prid->inDefineSizing )
    {
        return methodCanOnlyBeInvokedException(context, "pagedTab", "during the defineSizing method", pcpbd->rexxSelf);
    }

    if ( prid->countPagedTabs >= MAXMANAGEDTABS )
    {
        return tooManyPagedTabsException(context, 5, true);
    }


    int32_t tabID = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, rxTabID, -1, 1, true);
    if ( tabID == OOD_ID_EXCEPTION )
    {
        return NULLOBJECT;
    }

    pPagedTab pt = (pPagedTab)LocalAlloc(LPTR, sizeof(PagedTab));
    if ( pt == NULL )
    {
        outOfMemoryException(context->threadContext);
        return NULLOBJECT;
    }

    pt->tabID = tabID;
    prid->pagedTabs[prid->countPagedTabs++] = pt;

    RexxMethodContext *c = context;
    size_t count = context->ArrayItems(dlgIDs);
    if ( count > MAXCHILDDIALOGS )
    {
        arrayToLargeException(context->threadContext, (uint32_t)count, MAXCHILDDIALOGS, 2);
        return NULLOBJECT;
    }

    for ( size_t i = 1; i <= count; i++ )
    {
        RexxObjectPtr rxID = context->ArrayAt(dlgIDs, i);
        if ( rxID == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 2, i);
            return NULLOBJECT;
        }

        int32_t dlgID = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, rxID, -1, 2, true);
        if ( dlgID == OOD_ID_EXCEPTION )
        {
            wrongObjInArrayException(context->threadContext, 2, i, "a valid numeric ID or a valid symbolic ID", rxID);
            return NULLOBJECT;
        }

        pt->dlgIDs[i - 1] = dlgID;
    }

    return TheZeroObj;
}


/** ResizingAdmin::useDefaultSizing()
 *
 * Sets the control resize info for the specified control to the current value
 * of the default sizing.
 *
 * @param  rxID  [required]  The resource ID of the control, may be numeric or
 *               symbolic.
 *
 * @return Zero on success, one on error.
 *
 * @notes  The values of the resize info for any control can be changed at any
 *         time, but the control's order in the resizing table can not be
 *         changed once allocated.
 *
 *         This method allows the programmer to explicity define a sizing record
 *         in the table, but still use the default sizing.  It is a convenience
 *         method so that a sizing definition can be adde to the table ahead of
 *         adding a record that pins to the control.
 */
RexxMethod2(RexxObjectPtr, ra_useDefaultSizing, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        goto err_out;
    }

    if ( ! prid->inDefineSizing )
    {
        methodCanOnlyBeInvokedException(context, "useDefaultSizing", "during the defineSizing method", pcpbd->rexxSelf);
        goto err_out;
    }

    uint32_t ctrlID = oodResolveSymbolicID(context, pcpbd->rexxSelf, rxID, -1, 1, true);
    if ( ctrlID == OOD_INVALID_ITEM_ID )
    {
        goto err_out;
    }

    // Unlikely, but no reason why the user couldn't be redefining this control
    // and we already have a resize info struct.
    pResizeInfoCtrl ric = getControlInfo(prid, ctrlID);
    if ( ric == NULL )
    {
        ric = allocCtrlInfo(context->threadContext, prid, ctrlID, NULL, winUnknown);
        if ( ric == NULL )
        {
            goto err_out;
        }
    }
    else
    {
        memcpy(&ric->edges, &prid->defEdges, sizeof(ControlEdges));
    }

    if ( ! validPinToInfo(context, ctrlID, ric, prid) )
    {
        goto err_out;
    }

    return TheZeroObj;

err_out:
    return TheOneObj;
}


/** ResizingAdmin::wantSizeEnded()
 *
 *  Allows the user to specify that a method in the Rexx dialog is invoked when
 *  the EXITSIZMORE event happens.
 *
 *  @param  mthName  The name of the method to invoke.  If omitted, the default
 *                   name is onSizeEnded.
 *
 *  @param  willReply  If true, the interpreter waits for the reply from the
 *                     method handler.  If false, the interpreter does not wait.
 *                     The operating system ignores the value of the reply, so
 *                     any value is acceptable to return from the method
 *                     handler.  0 always makes a good return value.
 *
 *  @return  This method always returns 0.
 */
RexxMethod3(RexxObjectPtr, ra_wantSizeEnded, OPTIONAL_CSTRING, mthName, OPTIONAL_logical_t, willReply, CSELF, pCSelf)
{
    pResizeInfoDlg    prid = NULL;
    pCPlainBaseDialog pcpbd = validateRACSelf(context, pCSelf, &prid);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    if ( argumentOmitted(1) )
    {
        mthName = "onSizeEnded";
    }

    prid->sizeEndedMeth = (char *)LocalAlloc(LPTR, strlen(mthName) + 1);
    if ( prid->sizeEndedMeth == NULL )
    {
        outOfMemoryException(context->threadContext);
        return TheOneObj;
    }

    strcpy(prid->sizeEndedMeth, mthName);
    prid->sizeEndedWillReply = willReply ? true : false;

    return TheZeroObj;
}


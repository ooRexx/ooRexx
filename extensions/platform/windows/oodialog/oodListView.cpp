/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
 * oodListView.cpp
 *
 * Contains methods for the list-view control and list-view helper classes.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodShared.hpp"
#include "oodMessaging.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"

/**
 *  Methods for the .ListView class.
 */
#define LISTVIEW_CLASS               "ListView"

#define LVSTATE_ATTRIBUTE            "LV!STATEIMAGELIST"
#define LVSMALL_ATTRIBUTE            "LV!SMALLIMAGELIST"
#define LVNORMAL_ATTRIBUTE           "LV!NORMALIMAGELIST"

#define LVITEM_ALL_MASK              LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_INDENT | LVIF_COLUMNS | LVIF_COLFMT | LVIF_GROUPID | LVIF_NORECOMPUTE
#define LVITEM_TEXT_MAX              260
#define LVITEM_TILECOLUMN_MAX        20   // According to MSDN
#define LPSTR_TEXTCALLBACK_STRING    "lpStrTextCallBack"

#define LVITEM_OBJ_MAGIC             "mv12fa2t@"
#define LVSUBITEM_OBJ_MAGIC          "L1sw2h2ww"

/**
 * Prototypes for local functions that are too hard to move around to eliminate
 * the need for the prototypes.
 *
 */
static bool          validColumnIndex(pCLvFullRow pclvfr, bool removed, uint32_t colIndex);
static RexxObjectPtr removeSubItemFromRow(RexxMethodContext *c, pCLvFullRow pclvfr, uint32_t index);
static RexxObjectPtr insertSubItemIntoRow(RexxMethodContext *c, RexxObjectPtr lvSubitem, pCLvFullRow pclvfr, uint32_t index);




inline bool hasCheckBoxes(HWND hList)
{
    return ((ListView_GetExtendedListViewStyle(hList) & LVS_EX_CHECKBOXES) != 0);
}

inline bool hasSubitemImages(HWND hList)
{
    return ((ListView_GetExtendedListViewStyle(hList) & LVS_EX_SUBITEMIMAGES) != 0);
}

/**
 * Checks that the list view is either in icon view, or small icon view.
 * Certain list view messages and functions are only applicable in those views.
 *
 * If we are not at least 6.0 version of the common controls library and using
 * GWL_STYLE, note that LVS_ICON == 0 so LVS_TYPEMASK must be used,
 *
 * @remarks  It seems as if ListView_SetView is used to set the view, which is
 *           possible from ooDialog 4.2.1 onwards, using the GWL_STYLE to
 *           determine the view doesn't work.
 *
 *           Under almost all circumstances we are at least using 6.0 common
 *           control library.  We only wouldn't be if we are running on Windows
 *           2000.  But, we check anyway.
 */
inline bool isInIconView(HWND hList)
{
    uint32_t style;

    if ( ComCtl32Version < COMCTL32_6_0 )
    {
        style = (uint32_t)GetWindowLong(hList, GWL_STYLE);
        return ((style & LVS_TYPEMASK) == LVS_ICON) || ((style & LVS_TYPEMASK) == LVS_SMALLICON);
    }
    else
    {
        style = ListView_GetView(hList);
        return (style == LV_VIEW_ICON) || (style == LV_VIEW_SMALLICON);
    }
}

inline bool isLvFullRowStruct(void *p)
{
    return p != NULL && *(((uint32_t *)p)) == LVFULLROW_MAGIC;
}

/**
 * Checks if the list view is in report view.
 *
 * See remarks for isInIconView() above.
 */
bool isInReportView(HWND hList)
{
    uint32_t style;
    if ( ComCtl32Version < COMCTL32_6_0 )
    {
        style = (uint32_t)GetWindowLong(hList, GWL_STYLE);
        return ((style & LVS_TYPEMASK) == LVS_REPORT);
    }
    else
    {
        style = ListView_GetView(hList);
        return style == LV_VIEW_DETAILS;
    }
}

/**
 * Returns the index of the first selected item in the list view, or -1 if no
 * items are selected.
 */
inline int32_t getSelected(HWND hList)
{
    return ListView_GetNextItem(hList, -1, LVNI_SELECTED);
}

inline int getColumnCount(HWND hList)
{
    return Header_GetItemCount(ListView_GetHeader(hList));
}

inline CSTRING getLVAttributeName(uint8_t type)
{
    switch ( type )
    {
        case LVSIL_STATE :
            return LVSTATE_ATTRIBUTE;
        case LVSIL_SMALL :
            return LVSMALL_ATTRIBUTE;
        case LVSIL_NORMAL :
        default :
            return LVNORMAL_ATTRIBUTE;
    }
}

/**
 * Change the window style of a list view to align left or align top.
 */
static void applyAlignStyle(HWND hList, bool doTop)
{
    uint32_t flag = (doTop ? LVS_ALIGNTOP : LVS_ALIGNLEFT);

    uint32_t style = (uint32_t)GetWindowLong(hList, GWL_STYLE);
    SetWindowLong(hList, GWL_STYLE, ((style & ~LVS_ALIGNMASK) | flag));

    int count = ListView_GetItemCount(hList);
    if ( count > 0 )
    {
        count--;
        ListView_RedrawItems(hList, 0, count);
        UpdateWindow(hList);
    }
}

/**
 * Parse a list-view control extended style string sent from ooDialog into the
 * corresponding style flags.
 *
 * The extended list-view styles are set (and retrieved) in a different manner
 * than other window styles.  This function is used only to parse those extended
 * styles.  The normal list-view styles are parsed using EvaluateListStyle.
 */
static uint32_t parseExtendedStyle(const char * style)
{
    uint32_t dwStyle = 0;

    if ( strstr(style, "BORDERSELECT"    ) ) dwStyle |= LVS_EX_BORDERSELECT;
    if ( strstr(style, "CHECKBOXES"      ) ) dwStyle |= LVS_EX_CHECKBOXES;
    if ( strstr(style, "FLATSB"          ) ) dwStyle |= LVS_EX_FLATSB;
    if ( strstr(style, "FULLROWSELECT"   ) ) dwStyle |= LVS_EX_FULLROWSELECT;
    if ( strstr(style, "GRIDLINES"       ) ) dwStyle |= LVS_EX_GRIDLINES;
    if ( strstr(style, "HEADERDRAGDROP"  ) ) dwStyle |= LVS_EX_HEADERDRAGDROP;
    if ( strstr(style, "INFOTIP"         ) ) dwStyle |= LVS_EX_INFOTIP;
    if ( strstr(style, "MULTIWORKAREAS"  ) ) dwStyle |= LVS_EX_MULTIWORKAREAS;
    if ( strstr(style, "ONECLICKACTIVATE") ) dwStyle |= LVS_EX_ONECLICKACTIVATE;
    if ( strstr(style, "REGIONAL"        ) ) dwStyle |= LVS_EX_REGIONAL;
    if ( strstr(style, "SUBITEMIMAGES"   ) ) dwStyle |= LVS_EX_SUBITEMIMAGES;
    if ( strstr(style, "TRACKSELECT"     ) ) dwStyle |= LVS_EX_TRACKSELECT;
    if ( strstr(style, "TWOCLICKACTIVATE") ) dwStyle |= LVS_EX_TWOCLICKACTIVATE;
    if ( strstr(style, "UNDERLINECOLD"   ) ) dwStyle |= LVS_EX_UNDERLINECOLD;
    if ( strstr(style, "UNDERLINEHOT"    ) ) dwStyle |= LVS_EX_UNDERLINEHOT;

    // Needs Comctl32.dll version 5.8 or higher
    if ( ComCtl32Version >= COMCTL32_5_8 )
    {
      if ( strstr(style, "LABELTIP") ) dwStyle |= LVS_EX_LABELTIP;
    }

    // Needs Comctl32 version 6.0 or higher
    if ( ComCtl32Version >= COMCTL32_6_0 )
    {
      if ( strstr(style, "DOUBLEBUFFER") ) dwStyle |= LVS_EX_DOUBLEBUFFER;
      if ( strstr(style, "SIMPLESELECT") ) dwStyle |= LVS_EX_SIMPLESELECT;
    }
    return dwStyle;
}


/**
 * Change a list-view's style.
 *
 * @param c
 * @param pCSelf
 * @param _style
 * @param _additionalStyle
 * @param remove
 *
 * @return uint32_t
 *
 *  @remarks  MSDN suggests setting last error to 0 before calling
 *            GetWindowLong() as the correct way to determine error.
 */
static uint32_t changeStyle(RexxMethodContext *c, pCDialogControl pCSelf, CSTRING _style, CSTRING _additionalStyle, bool remove)
{
    oodResetSysErrCode(c->threadContext);
    SetLastError(0);

    HWND     hList = getDChCtrl(pCSelf);
    uint32_t oldStyle = (uint32_t)GetWindowLong(hList, GWL_STYLE);

    if ( oldStyle == 0 && GetLastError() != 0 )
    {
        goto err_out;
    }

    uint32_t newStyle = 0;
    if ( remove )
    {
        newStyle = oldStyle & ~listViewStyle(_style, 0);
        if ( _additionalStyle != NULL )
        {
            newStyle = listViewStyle(_additionalStyle, newStyle);
        }
    }
    else
    {
        newStyle = listViewStyle(_style, oldStyle);
    }

    if ( SetWindowLong(hList, GWL_STYLE, newStyle) == 0 && GetLastError() != 0 )
    {
        goto err_out;
    }
    return oldStyle;

err_out:
    oodSetSysErrCode(c->threadContext);
    return 0;
}


static uint32_t keyword2lvir(RexxMethodContext *c, CSTRING part, bool isItemRect)
{
    uint32_t flag = (uint32_t)-1;

    if ( StrCmpI(part,      "BOUNDS") == 0 ) flag = LVIR_BOUNDS;
    else if ( StrCmpI(part, "ICON")   == 0 ) flag = LVIR_ICON;
    else if ( StrCmpI(part, "LABEL")  == 0 ) flag = LVIR_LABEL;
    else if ( isItemRect )
    {
        if ( StrCmpI(part,  "SELECTBOUNDS") == 0 ) flag = LVIR_SELECTBOUNDS;
        else
        {
            wrongArgValueException(c->threadContext, 2, "BOUNDS, ICON, LABEL, or SELECTBOUNDS", part);
        }
    }
    else
    {
        wrongArgValueException(c->threadContext, 3, "BOUNDS, ICON, or LABEL", part);
    }

    return flag;
}


static RexxStringObject view2keyword(RexxMethodContext *c, uint32_t view)
{
    if (      view == LV_VIEW_ICON      ) return c->String("ICON");
    else if ( view == LV_VIEW_SMALLICON ) return c->String("SMALLICON");
    else if ( view == LV_VIEW_LIST      ) return c->String("LIST");
    else if ( view == LV_VIEW_DETAILS   ) return c->String("REPORT");

    return c->NullString();
}


/**
 * Produce a string representation of a List-View's extended styles.
 */
static RexxStringObject extendedStyleToString(RexxMethodContext *c, HWND hList)
{
    char buf[256];
    DWORD dwStyle = ListView_GetExtendedListViewStyle(hList);
    buf[0] = '\0';

    if ( dwStyle & LVS_EX_BORDERSELECT )     strcat(buf, "BORDERSELECT ");
    if ( dwStyle & LVS_EX_CHECKBOXES )       strcat(buf, "CHECKBOXES ");
    if ( dwStyle & LVS_EX_FLATSB )           strcat(buf, "FLATSB ");
    if ( dwStyle & LVS_EX_FULLROWSELECT )    strcat(buf, "FULLROWSELECT ");
    if ( dwStyle & LVS_EX_GRIDLINES )        strcat(buf, "GRIDLINES ");
    if ( dwStyle & LVS_EX_HEADERDRAGDROP )   strcat(buf, "HEADERDRAGDROP ");
    if ( dwStyle & LVS_EX_INFOTIP )          strcat(buf, "INFOTIP ");
    if ( dwStyle & LVS_EX_MULTIWORKAREAS )   strcat(buf, "MULTIWORKAREAS ");
    if ( dwStyle & LVS_EX_ONECLICKACTIVATE ) strcat(buf, "ONECLICKACTIVATE ");
    if ( dwStyle & LVS_EX_REGIONAL )         strcat(buf, "REGIONAL ");
    if ( dwStyle & LVS_EX_SUBITEMIMAGES )    strcat(buf, "SUBITEMIMAGES ");
    if ( dwStyle & LVS_EX_TRACKSELECT )      strcat(buf, "TRACKSELECT ");
    if ( dwStyle & LVS_EX_TWOCLICKACTIVATE ) strcat(buf, "TWOCLICKACTIVATE ");
    if ( dwStyle & LVS_EX_UNDERLINECOLD )    strcat(buf, "UNDERLINECOLD ");
    if ( dwStyle & LVS_EX_UNDERLINEHOT )     strcat(buf, "UNDERLINEHOT ");
    if ( dwStyle & LVS_EX_LABELTIP )         strcat(buf, "LABELTIP ");
    if ( dwStyle & LVS_EX_DOUBLEBUFFER )     strcat(buf, "DOUBLEBUFFER ");
    if ( dwStyle & LVS_EX_SIMPLESELECT )     strcat(buf, "SIMPLESELECT ");

    return c->String(buf);
}

/**
 * Gets the image list type from the specified argument object, where the object
 * could be the numeric value, a string keyword, or omitted altogether.
 *
 * @param context
 * @param _type
 * @param argPos
 *
 * @return The list-view image list type or OOD_NO_VALUE on error.  An exception
 *         has been raised on error.
 */
static uint32_t getImageListTypeArg(RexxMethodContext *context, RexxObjectPtr _type, size_t argPos)
{
    uint32_t type = LVSIL_NORMAL;

    if ( argumentExists(argPos) )
    {
        if ( ! context->UnsignedInt32(_type, &type) )
        {
            CSTRING lvsil = context->ObjectToStringValue(_type);
            if (      StrCmpI("NORMAL", lvsil) == 0 ) type = LVSIL_NORMAL;
            else if ( StrCmpI("SMALL", lvsil)  == 0 ) type = LVSIL_SMALL;
            else if ( StrCmpI("STATE", lvsil)  == 0 ) type = LVSIL_STATE;
            else
            {
                wrongArgValueException(context->threadContext, argPos, "Normal, Small, or State", _type);
                type = OOD_NO_VALUE;
            }
        }

        if ( type != OOD_NO_VALUE && type > LVSIL_STATE )
        {
            wrongRangeException(context->threadContext, argPos, LVSIL_NORMAL, LVSIL_STATE, type);
            type = OOD_NO_VALUE;
        }
    }
    return type;
}

static int getColumnWidthArg(RexxMethodContext *context, RexxObjectPtr _width, size_t argPos)
{
    int width = OOD_BAD_WIDTH_EXCEPTION;

    if ( argumentOmitted(argPos) )
    {
        width = LVSCW_AUTOSIZE;
    }
    else
    {
        CSTRING tmpWidth = context->ObjectToStringValue(_width);

        if ( stricmp(tmpWidth, "AUTO") == 0 )
        {
            width = LVSCW_AUTOSIZE;
        }
        else if ( stricmp(tmpWidth, "AUTOHEADER") == 0 )
        {
            width = LVSCW_AUTOSIZE_USEHEADER;
        }
        else if ( ! context->Int32(_width, &width) || width < 1 )
        {
            wrongArgValueException(context->threadContext, argPos, "AUTO, AUTOHEADER, or a positive whole number", _width);
        }
    }

    return width;
}

/**
 * Checks for a LPSTR_TEXTCALLBACK in the pszText field of a LVITEM struct.
 *
 * @param lvi       Pointer to struct we are checking.
 * @param temp      Pointer to buffer originally set as pszText field
 * @param freeTemp  True if we should free temp, false is we should not
 *
 * @remarks  When doing a ListView_GetItem() MSDN says that the programmer
 *           should not rely on the pszText field pointing to the same buffer on
 *           return, that the list-view may change the pointer to point to the
 *           new text rather than place it in the buffer.
 *
 *           For certain, if the mask for the ListView_GetItem() contains
 *           LVIF_NORECOMPUTE, then pszText is changed to LPSTR_TEXTCALLBACK.
 *           This function checks for that.  I've never seen the pointer changed
 *           to 'new text' but we check for that also, and NULL.
 *
 * @assumes  That temp is pointing to a buffer at least LVITEM_TEXT_MAX + 1 in
 *           size.  All buffers being allocated for pszText in our code are
 *           that size.  Do not call this function if the size of temp is
 *           unknown, or less than LVITEM_TEXT_MAX + 1 in size.
 */
static void checkForCallBack(LPLVITEM lvi, char *temp, bool freeTemp)
{
    if ( lvi->pszText != temp )
    {
        if ( lvi->pszText == LPSTR_TEXTCALLBACK || lvi->pszText == NULL )
        {
            lvi->cchTextMax = 0;
            if ( freeTemp )
            {
                LocalFree(temp);
            }
        }
        else
        {
            StrCpyN(temp, lvi->pszText, LVITEM_TEXT_MAX);
            temp[LVITEM_TEXT_MAX] = '\0';

            lvi->cchTextMax = LVITEM_TEXT_MAX + 1;
            lvi->pszText    = temp;
        }
    }
}

/**
 * Allocates memory for the text buffer in a list-view item structure.
 *
 * This should only be used for Rexx objects such as a LvItem or LvSubItem,
 * etc., where the uninit method of the object will free the buffer.
 *
 * @param c
 * @param pLVI
 *
 * @return True on success, false on memory allocation failure.
 */
static bool allocLviTextBuf(RexxMethodContext *c, LPLVITEM pLVI)
{
    char *pszText = (char *)LocalAlloc(LPTR, LVITEM_TEXT_MAX + 1);
    if ( pszText == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    if ( pLVI->pszText != LPSTR_TEXTCALLBACK )
    {
        safeLocalFree(pLVI->pszText);
    }

    pLVI->cchTextMax = (LVITEM_TEXT_MAX + 1);
    pLVI->pszText    = pszText;

    return true;
}

/**
 * Allocates a buffer for the LISTITEM.puColumns field to recieve data.  The
 * buffer is set big enough for the maximum number of columns.
 *
 * @param c
 * @param pLVI
 *
 * @return bool
 */
static bool allocLviColumns(RexxMethodContext *c, LPLVITEM pLVI)
{
    uint32_t *puColumns = (uint32_t *)LocalAlloc(LPTR, LVITEM_TILECOLUMN_MAX * sizeof(uint32_t *));
    if ( puColumns == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    safeLocalFree(pLVI->puColumns);
    pLVI->puColumns = puColumns;
    pLVI->cColumns = LVITEM_TILECOLUMN_MAX;

    return true;
}

/**
 * Allocates a buffer for the LISTITEM.piColFmt field to recieve data.  The
 * buffer is set big enough for the maximum number of columns.
 *
 * @param c
 * @param pLVI
 *
 * @return bool
 */
static bool allocLviColFmt(RexxMethodContext *c, LPLVITEM pLVI)
{
    int32_t *piColFmt = (int32_t *)LocalAlloc(LPTR, LVITEM_TILECOLUMN_MAX * sizeof(uint32_t *));
    if ( piColFmt == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    safeLocalFree(pLVI->piColFmt);
    pLVI->piColFmt = piColFmt;
    pLVI->cColumns = LVITEM_TILECOLUMN_MAX;

    return true;
}

/**
 * Allocates memory for the tile column buffers in a list-view item structure.
 *
 * This should only be used for the LvItem Rexx objects, where the uninit method
 * of the object will free the buffers.
 *
 * @param c
 * @param pLVI
 *
 * @return True on success, false on memory allocation failure.
 *
 * @remarks  If possible, we don't want to change the existing LVITEM struct
 *           until we are sure things succeed.
 */
static bool allocLviTileCols(RexxMethodContext *c, LPLVITEM pLVI)
{
    uint32_t *puColumns = (uint32_t *)LocalAlloc(LPTR, LVITEM_TILECOLUMN_MAX * sizeof(uint32_t *));
    if ( puColumns == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    int32_t *piColFmt = (int32_t *)LocalAlloc(LPTR, LVITEM_TILECOLUMN_MAX * sizeof(uint32_t *));
    if ( piColFmt == NULL )
    {
        safeLocalFree(puColumns);
        outOfMemoryException(c->threadContext);
        return false;
    }

    safeLocalFree(pLVI->puColumns);
    pLVI->puColumns = puColumns;

    safeLocalFree(pLVI->piColFmt);
    pLVI->piColFmt = piColFmt;

    pLVI->cColumns = LVITEM_TILECOLUMN_MAX;

    return true;
}

/**
 * Ensures that the buffers in a LVITEM structure are allocated to recieve
 * information.
 *
 * @param c
 * @param pLVI
 *
 * @return bool
 *
 * @remarks  There is a vague assumption here that the LVITEM struct is only
 *           going to be used for a ListView_GetItem() call.  For a
 *           ListView_SetItem() call, the buffers would have been allocated as
 *           the Rexx object attributes were set.
 *
 *           However, this is also called when an LvItem in a LvFullRow is going
 *           to be updated.  It doesn't seem like having a too big buffer should
 *           be a problem ...
 *
 *           We don't want to change anything in the existing LVITEM struct, if
 *           at all possible.  So rather than call allocLviTextBuf() and
 *           allocLvitTileCols() individually, we duplicate their code here.
 */
static bool ensureLvItemBuffers(RexxMethodContext *c, LPLVITEM pLVI)
{
    char *pszText = (char *)LocalAlloc(LPTR, LVITEM_TEXT_MAX + 1);
    if ( pszText == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    uint32_t *puColumns = (uint32_t *)LocalAlloc(LPTR, LVITEM_TILECOLUMN_MAX * sizeof(uint32_t *));
    if ( puColumns == NULL )
    {
        safeLocalFree(pszText);
        outOfMemoryException(c->threadContext);
        return false;
    }

    int32_t *piColFmt = (int32_t *)LocalAlloc(LPTR, LVITEM_TILECOLUMN_MAX * sizeof(uint32_t *));
    if ( piColFmt == NULL )
    {
        safeLocalFree(pszText);
        safeLocalFree(puColumns);
        outOfMemoryException(c->threadContext);
        return false;
    }

    safeLocalFree(pLVI->pszText);
    pLVI->pszText = pszText;

    safeLocalFree(pLVI->puColumns);
    pLVI->puColumns = puColumns;

    safeLocalFree(pLVI->piColFmt);
    pLVI->piColFmt = piColFmt;

    pLVI->cchTextMax = LVITEM_TEXT_MAX + 1;
    pLVI->cColumns   = LVITEM_TILECOLUMN_MAX;

    return true;
}

/**
 * Uses MapIDToIndex and the unique ID stored with a LvFullRow object to correct
 * the item index in the LPLVITEM structs within the full row item and subitems
 *
 * @param pclvfr
 */
static void updateFullRowIndexes(pCLvFullRow pclvfr)
{
    if ( pclvfr->hList != NULL && pclvfr->id != LVFULLROW_NOID )
    {
        uint32_t realIndex = ListView_MapIDToIndex(pclvfr->hList, pclvfr->id);

        if ( realIndex != pclvfr->subItems[0]->iItem )
        {
            for ( uint32_t i = 0; i <= pclvfr->subItemCount; i++ )
            {
                pclvfr->subItems[i]->iItem = realIndex;
            }
        }
    }
}

/**
 * Updates the state field in the LvItem object within a LvFullRowObject.
 *
 * @param pclvfr
 *
 * @assumes updateFullRowIndexes() has already been invoked.
 *
 * @remarks  I wonder if we should set the state mask value used for the
 *           ListView_GetItem to the pLvi struct, and maybe |= the LVIF_STATE
 *           flag to the pLvi mask field in the struct?
 *
 *           If the Rexx user examines the LvItem object after a call to
 *           ListView::getFullRow(), he could see maybe SELECTED FOCUSED for the
 *           state, but no corrsponding values in the mask and stateMask fields
 *           that would indicate the state attribute is valid.
 */
static void updateFullRowItemState(pCLvFullRow pclvfr)
{
    if ( pclvfr->hList != NULL )
    {
        LPLVITEM pLvi = pclvfr->subItems[0];
        LVITEM   lvi  = {0};

        lvi.iItem     = pLvi->iItem;
        lvi.mask      = LVIF_STATE;
        lvi.stateMask = (uint32_t)-1;

        if ( ListView_GetItem(pclvfr->hList, &lvi) )
        {
            pLvi->state = lvi.state;
        }
    }
}

/**
 * Used to update the text for the item or subitem specified in the full rows
 * struct.
 *
 * The intent of this function is to keep a LvFullRow object that is assigned to
 * the lParam user data in sync with changes made to the list-view item.
 *
 * @param c
 * @param pclvfr
 * @param subitem
 * @param text
 *
 * @assumes  pclvfr has already been checked to be the assigned lParam user data
 *           of a lisr-view item.
 *
 * @remarks  If a memory allocation error happens, we raise an exception, but
 *           leave the current text alone.  No errors are reported.
 *
 *           If subitem is out of bounds then we just do nothing.  We need to
 *           account for LPSTR_TEXTCALLBACK.
 */
static void updateFullRowText(RexxThreadContext *c, pCLvFullRow pclvfr, uint32_t subitem, CSTRING text)
{
    if ( subitem <= pclvfr->subItemCount )
    {
        size_t  len = 0;
        char   *newText;

        if ( text == LPSTR_TEXTCALLBACK )
        {
            newText = LPSTR_TEXTCALLBACK;
        }
        else
        {
            len = strlen(text) + 1;

            newText = (char *)LocalAlloc(LPTR, len);
            if ( newText == NULL )
            {
                outOfMemoryException(c);
                return;
            }
            memcpy(newText, text, len);
        }

        LPLVITEM lvi = pclvfr->subItems[subitem];

        safeLocalFree(lvi->pszText);

        lvi->pszText    = newText;
        lvi->cchTextMax = (int)len;
    }
}

/**
 * Checks if the user data for a list-view item has bee set to a LvFullRow
 * object.
 *
 * @param hList
 * @param itemIndex
 *
 * @return The CSelf struct for a LvFullRow object if the user data of the
 *         list-view item has been set to a LvFullRow object, otherwise null.
 *
 * @note The item indexes in the full row struct are updated if the full row
 *       object is found.
 */
static pCLvFullRow maybeGetFullRow(HWND hList, uint32_t itemIndex)
{
    LVITEM      lvi    = {LVIF_PARAM, (int)itemIndex}; // cast avoids C4838
    pCLvFullRow pclvfr = NULL;

    if ( ListView_GetItem(hList, &lvi) != 0 )
    {
        if ( isLvFullRowStruct((void *)(lvi.lParam)) )
        {
            pclvfr = (pCLvFullRow)lvi.lParam;
            updateFullRowIndexes(pclvfr);
        }
    }
    return pclvfr;
}

static bool reasonableColumnFix(RexxMethodContext *c, HWND hList, uint32_t colIndex, bool removed)
{
    uint32_t cItems = ListView_GetItemCount(hList);

    if ( cItems == 0 )
    {
        return false;
    }

    for ( uint32_t i = 0; i < cItems, i < 3; i++ )
    {
        char buff[128];

        pCLvFullRow pclvfr = maybeGetFullRow(hList, i);
        if ( pclvfr == NULL )
        {
            _snprintf(buff, sizeof(buff), "the item data of list-view item %d is not a LvFullRow object", i);

            executionErrorException(c->threadContext, buff);
            return false;
        }

        if ( ! validColumnIndex(pclvfr, removed, colIndex) )
        {
            _snprintf(buff, sizeof(buff),
                     "LvFullRow object at list-view item %d; invalid column %s (subitem columns=%d column=%d)",
                     i, removed ? "delete" : "insert", pclvfr->subItemCount, colIndex);

            executionErrorException(c->threadContext, buff);
            return false;
        }
    }

    return true;
}

/**
 * Each item in a list-view allows the user to store a value at the lParam
 * member of the LVITEM struct.
 *
 * In ooDialog we allow the user to store any Rexx object there.  However, to
 * optimize the internal ooDialog sorting of list view items, if the Rexx object
 * is a LvFullRow object we store the CSelf pointer of the LvFullRow object,
 * rather than the LvFullRow object.
 *
 * This function gets the Rexx object stored by the user, translating it to the
 * LvFullRow object if needed.
 *
 * TODO It would be nice to allow the user to store an additional user data item
 * when the lParam is a LvFullRow ... ?
 *
 * @param lvi Pointer to a LVITEM struct.
 *
 * @return The Rexx object stored by the user for the specified list view item,
 *         or the .nil object if there is no stored object.
 */
static RexxObjectPtr getLviUserData(LVITEM *lvi)
{
    RexxObjectPtr result = TheNilObj;

    if ( lvi->lParam != 0 )
    {
        if ( isLvFullRowStruct((void *)(lvi->lParam)) )
        {
            updateFullRowIndexes((pCLvFullRow)lvi->lParam);
            result = ((pCLvFullRow)lvi->lParam)->rexxSelf;
        }
        else
        {
            result = (RexxObjectPtr)lvi->lParam;
        }
    }
    return result;
}


/**
 * Translates a list-view item lParam to a Rexx object.
 *
 * Some of the list-view related APIs pass the lParam value directly. If, this
 * value is not 0, it is a value stored by ooDialog.  Normally this is a Rexx
 * object. But, for a LvFullRow object we store the CSelf pointer instead.
 *
 * This function checks for that and returns the LvFullRow Rexx object, rather
 * than the CSelf pointer.
 *
 * @param lParam
 *
 * @return The Rexx object that is the lParam.
 */
static RexxObjectPtr lviLParam2UserData(LPARAM lParam)
{
    RexxObjectPtr result = TheNilObj;

    if ( lParam != 0 )
    {
        if ( isLvFullRowStruct((void *)(lParam)) )
        {
            updateFullRowIndexes((pCLvFullRow)lParam);
            result = ((pCLvFullRow)lParam)->rexxSelf;
        }
        else
        {
            result = (RexxObjectPtr)lParam;
        }
    }
    return result;
}

/**
 * Merges the state information being set in a LVITEM struct with an existing
 * LVITEM struct.
 *
 * @param c
 * @param pclvfr
 * @param existingLvi
 * @param lvi
 */
static void mergeLviState(RexxMethodContext *c, pCLvFullRow pclvfr, LPLVITEM existingLvi, LPLVITEM lvi)
{
    if ( LVIF_COLFMT & lvi->mask )
    {
        safeLocalFree(existingLvi->piColFmt);

        existingLvi->piColFmt = lvi->piColFmt;
        existingLvi->cColumns = lvi->cColumns;
    }
    if ( LVIF_COLUMNS & lvi->mask )
    {
        safeLocalFree(existingLvi->puColumns);

        existingLvi->puColumns = lvi->puColumns;
        existingLvi->cColumns  = lvi->cColumns;
    }
    if ( LVIF_GROUPID & lvi->mask )
    {
        existingLvi->iGroupId = lvi->iGroupId;
    }
    if ( LVIF_IMAGE & lvi->mask )
    {
        existingLvi->iImage = lvi->iImage;
    }
    if ( LVIF_INDENT & lvi->mask )
    {
        existingLvi->iIndent = lvi->iIndent;
    }
    if ( LVIF_TEXT & lvi->mask )
    {
        updateFullRowText(c->threadContext, pclvfr, 0, lvi->pszText);
    }
    if ( LVIF_STATE & lvi->mask )
    {
        uint32_t existingState = existingLvi->state;
        uint32_t newState      = 0;

        if ( LVIS_STATEIMAGEMASK & lvi->stateMask )
        {
            newState = INDEXTOSTATEIMAGEMASK(lvi->state & LVIS_STATEIMAGEMASK);
        }
        else
        {
            newState = existingState & LVIS_STATEIMAGEMASK;
        }

        if ( LVIS_OVERLAYMASK & lvi->stateMask )
        {
            newState |= INDEXTOOVERLAYMASK(lvi->state & LVIS_OVERLAYMASK);
        }
        else
        {
            newState |= existingState & LVIS_OVERLAYMASK;
        }

        if ( LVIS_CUT & lvi->stateMask )
        {
            newState |= lvi->state & LVIS_CUT;
        }
        else
        {
            newState |= existingState & LVIS_CUT;
        }

        if ( LVIS_DROPHILITED & lvi->stateMask )
        {
            newState |= lvi->state & LVIS_DROPHILITED;
        }
        else
        {
            newState |= existingState & LVIS_DROPHILITED;
        }

        if ( LVIS_FOCUSED & lvi->stateMask )
        {
            newState |= lvi->state & LVIS_FOCUSED;
        }
        else
        {
            newState |= existingState & LVIS_FOCUSED;
        }

        if ( LVIS_SELECTED & lvi->stateMask )
        {
            newState |= lvi->state & LVIS_SELECTED;
        }
        else
        {
            newState |= existingState & LVIS_SELECTED;
        }

        existingLvi->state      = newState;
        existingLvi->stateMask |= lvi->stateMask;
    }
}

/**
 *
 * Returns the lParam user data for the specified list-view item as a Rexx
 * object
 *
 * @param hList
 *
 * @return The Rexx object set as the user data, or the .nil object if no user
 *         data is set.
 */
static RexxObjectPtr getCurrentLviUserData(HWND hList, uint32_t index)
{
    LVITEM        lvi    = {LVIF_PARAM, (int)index}; // cast avoids C4838
    RexxObjectPtr result = TheNilObj;

    if ( ListView_GetItem(hList, &lvi) != 0 )
    {
        result = getLviUserData(&lvi);
    }
    return result;
}


static LPARAM getLParamUserData(RexxMethodContext *c, RexxObjectPtr data)
{
    if ( c->IsOfType(data, "LVFULLROW") )
    {
        return (LPARAM)c->ObjectToCSelf(data);
    }
    else
    {
        return (LPARAM)data;
    }
}

/**
 *  If the user stores a Rexx object in the user data storage of a list view
 *  item, the Rexx object could be garbage collected because no Rexx object has
 *  a reference to it.  To prevent that we put the Rexx object in a bag that is
 *  an attribute of the list view object.
 *
 * @param c
 * @param pcdc
 * @param lvi
 *
 * @notes  This function could have been called maybeProtectLvUserData() because
 *         it only stores a Rexx object if the lParam member of the list view
 *         item struct is not null.
 *
 *         With list-views, we have special handling of the lParam user data to
 *         optimize internal sorting.  Because of this we can not call
 *         protectControlUserData() directly, we first need to handle the
 *         possibility that the lParam is a C struct and not a Rexx object.
 */
static void protectLviUserData(RexxMethodContext *c, pCDialogControl pcdc, LVITEM *lvi)
{
    protectControlUserData(c, pcdc, getLviUserData(lvi));
}

/**
 * Instantiates a new LvItem object for the specified item in the list-view.
 *
 * @param c
 * @param hList
 * @param itemIndex
 *
 * @return RexxObjectPtr
 *
 * @remarks  Note that the iGroupId field of the LVITEM struct is a little
 *           hokey.  If we set the LVIF_GROUPID flag in the mask to retrieve
 *           the group ID and there is no group ID, the OS returns 0, instead of
 *           I_GROUPIDNONE (-2).  If we keep the LVIF_GROUPID flag set, and try
 *           to insert the item with the groupId set to 0, the insert fails.
 *
 *           I don't know if this is because groups are not available or what.
 *           For now, we check for a 0 value and change it to I_GROUPIDNONE.
 */
static RexxObjectPtr newLvItem(RexxMethodContext *c, HWND hList, uint32_t itemIndex)
{
    RexxObjectPtr result = TheNilObj;

    RexxBufferObject obj = c->NewBuffer(sizeof(LVITEM));
    if ( obj == NULLOBJECT )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    LPLVITEM lvi = (LPLVITEM)c->BufferData(obj);
    memset(lvi, 0, sizeof(LVITEM));

    if ( ! ensureLvItemBuffers(c, lvi) )
    {
        goto err_out;
    }

    lvi->iItem     = itemIndex;
    lvi->stateMask = (uint32_t)-1;
    lvi->mask      = LVITEM_ALL_MASK;

    char *temp = lvi->pszText;

    if ( ! ListView_GetItem(hList, lvi) )
    {
        goto err_out;
    }
    if ( lvi->iGroupId == 0 )
    {
        lvi->iGroupId = I_GROUPIDNONE;
    }
    checkForCallBack(lvi, temp, true);

    result = c->SendMessage2(TheLvItemClass, "NEW", obj, c->String(LVITEM_OBJ_MAGIC));
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
    }
    else
    {
        goto done_out;
    }

err_out:
    if ( lvi != NULL )
    {
        safeLocalFree(lvi->pszText);
        safeLocalFree(lvi->puColumns);
        safeLocalFree(lvi->piColFmt);
    }

done_out:
    return result;
}

/**
 * Constructs a new LvItem object with just the text of the subitem.  The item
 * is to be used for insertion only, it is not constructed to be suitable for a
 * get item.
 *
 * @param c
 * @param text  The text for the item, may be the special text call back value.
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr simpleNewLvItem(RexxMethodContext *c, CSTRING text)
{
    RexxObjectPtr result = TheNilObj;

    RexxBufferObject obj = c->NewBuffer(sizeof(LVITEM));
    if ( obj == NULLOBJECT )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    LPLVITEM lvi = (LPLVITEM)c->BufferData(obj);
    memset(lvi, 0, sizeof(LVITEM));

    if ( StrCmpI(text, "lpStrTextCallBack") == 0 )
    {
        lvi->pszText = LPSTR_TEXTCALLBACK;
    }
    else
    {
        size_t len = strlen(text);

        if ( len > LVITEM_TEXT_MAX )
        {
            stringTooLongException(c->threadContext, 1, LVITEM_TEXT_MAX, len);
            goto done_out;
        }

        lvi->pszText = (char *)LocalAlloc(LPTR, len + 1);
        if ( lvi->pszText == NULL )
        {
            outOfMemoryException(c->threadContext);
            goto done_out;
        }

        strcpy(lvi->pszText, text);
    }

    lvi->mask     = LVIF_TEXT | LVIF_GROUPID | LVIF_IMAGE;
    lvi->iGroupId = I_GROUPIDNONE;
    lvi->iImage   = I_IMAGENONE;

    result = c->SendMessage2(TheLvItemClass, "NEW", obj, c->String(LVITEM_OBJ_MAGIC));
    if ( result == NULLOBJECT )
    {
        safeLocalFree(lvi->pszText);
        result = TheNilObj;
    }
    else
    {
        goto done_out;
    }

done_out:
    return result;
}


/**
 * Returns a new LvSubItem object for the specified subitem in the list-view.
 *
 * @param c
 * @param hList
 * @param itemIndex
 * @param subitemIndex
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr newLvSubitem(RexxMethodContext *c, HWND hList, uint32_t itemIndex, uint32_t subitemIndex)
{
    RexxObjectPtr result = TheNilObj;

    RexxBufferObject obj = c->NewBuffer(sizeof(LVITEM));
    if ( obj == NULLOBJECT )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    LPLVITEM lvi = (LPLVITEM)c->BufferData(obj);
    memset(lvi, 0, sizeof(LVITEM));

    lvi->iItem    = itemIndex;
    lvi->iSubItem = subitemIndex;

    // setLviText() will allocate the buffer for us.
    if ( ! allocLviTextBuf(c, lvi) )
    {
        goto done_out;
    }

    char *temp = lvi->pszText;
    lvi->mask  = LVIF_TEXT | LVIF_IMAGE | LVIF_NORECOMPUTE;

    if ( ! ListView_GetItem(hList, lvi) )
    {
        safeLocalFree(lvi->pszText);
        goto done_out;
    }
    checkForCallBack(lvi, temp, true);

    // Argument 2 is required, but when argument 1 is a buffer object, argument
    // 2 will be ignored.
    result = c->SendMessage2(TheLvSubItemClass, "NEW", obj, TheOneObj);
    if ( result == NULLOBJECT )
    {
        safeLocalFree(lvi->pszText);
        result = TheNilObj;
    }

done_out:
    return result;
}

/**
 * Returns a new LvSubItem object for the specified subitem in the list-view.
 *
 * The returned object can only be used for insertion where the item index will
 * be fixed up at the time of insertion.
 *
 * @param c
 * @param subitemIndex
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr simpleNewLvSubitem(RexxMethodContext *c, CSTRING text, uint32_t subitemIndex)
{
    RexxObjectPtr result = TheNilObj;

    RexxBufferObject obj = c->NewBuffer(sizeof(LVITEM));
    if ( obj == NULLOBJECT )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    LPLVITEM lvi = (LPLVITEM)c->BufferData(obj);
    memset(lvi, 0, sizeof(LVITEM));

    if ( StrCmpI(text, "lpStrTextCallBack") == 0 )
    {
        lvi->pszText = LPSTR_TEXTCALLBACK;
    }
    else
    {
        size_t len = strlen(text);

        if ( len > LVITEM_TEXT_MAX )
        {
            stringTooLongException(c->threadContext, 1, LVITEM_TEXT_MAX, len);
            goto done_out;
        }

        lvi->pszText = (char *)LocalAlloc(LPTR, len + 1);
        if ( lvi->pszText == NULL )
        {
            outOfMemoryException(c->threadContext);
            goto done_out;
        }

        strcpy(lvi->pszText, text);
    }

    lvi->iSubItem = subitemIndex;
    lvi->mask     = LVIF_TEXT | LVIF_IMAGE;
    lvi->iImage   = I_IMAGENONE;

    // Argument 2 is required, but when argument 1 is a buffer object, argument
    // 2 will be ignored.
    result = c->SendMessage2(TheLvSubItemClass, "NEW", obj, TheOneObj);
    if ( result == NULLOBJECT )
    {
        safeLocalFree(lvi->pszText);
        result = TheNilObj;
    }

done_out:
    return result;
}


/**
 * Iterates through all LvFullRow objects assigned as the item data of a
 * list-view item and adds or removes the specified subitem column.
 *
 * @param c
 * @param hList
 * @param colIndex
 * @param removed
 *
 * @return The true or false object.
 *
 * @remarks  Theoretically this should only be invoked when all list-view items
 *           have a full row object assigned as the item data for the item, and
 *           the column index is valid for all full rows.  We check for this,
 *           but allow a few errors before we quit.
 *
 *           But, even one error indicates the user has something screwed up, so
 *           I'm not sure that we shouldn't be rasing an error condition ...
 */
static RexxObjectPtr fixColumns(RexxMethodContext *c, HWND hList, uint32_t colIndex, bool removed)
{
    uint32_t cItems = ListView_GetItemCount(hList);
    uint32_t cErrs  = 0;
    uint32_t i      = 0;

    for ( i = 0; i < cItems && cErrs < 3; i++ )
    {
        pCLvFullRow pclvfr = maybeGetFullRow(hList, i);
        if ( pclvfr == NULL )
        {
            cErrs++;
            continue;
        }

        if ( ! validColumnIndex(pclvfr, removed, colIndex) )
        {
            cErrs++;
            continue;
        }

        if ( removed )
        {
            removeSubItemFromRow(c, pclvfr, colIndex);
        }
        else
        {
            RexxObjectPtr lvSubItem = newLvSubitem(c, hList, i, colIndex);
            if ( lvSubItem == TheNilObj )
            {
                break;
            }

            if ( ! insertSubItemIntoRow(c, lvSubItem, pclvfr, colIndex) )
            {
                break;
            }
        }
    }

    return (i == cItems ? TheTrueObj : TheFalseObj);
}

/* This can be made static if we move the LVN processing from oodMessaging */
void maybeUpdateFullRowText(RexxThreadContext *c, NMLVDISPINFO *pdi)
{
    if ( pdi->item.lParam != 0 )
    {
        if ( isLvFullRowStruct((void *)pdi->item.lParam) )
        {
            pCLvFullRow pclvfr = (pCLvFullRow)pdi->item.lParam;

            updateFullRowIndexes(pclvfr);
            updateFullRowText(c, pclvfr, 0, pdi->item.pszText);
        }
    }
}


/**
 * Set the item's, and each subitem's, text to the corresponding value in the
 * pclvfr struct.
 *
 * @param pclvfr
 * @param hList
 * @param index
 *
 * @return The true object.
 *
 * @remarks  Originally this function was conceived to be used when the
 *           list-view item's user data was a full row object and the Rexx
 *           programmer wanted to sync the list-view item text with what he had
 *           set the full row object to.
 *
 *           However, it would work well in other circustances.
 *
 *           Contrast this function with the similare updateTextUsingFullRow()
 *           function.  This function unequivocally sets the item and every
 *           subitem with the text in the full row struct.  The other function
 *           checks for the LVIF_TEXT max and only updates the list-view if the
 *           mask is set.  In addition, it optionally updates the full row
 *           struct when the item has a full row struct assigned as the user
 *           data.
 */
static RexxObjectPtr syncFullRowText(pCLvFullRow pclvfr, HWND hList, uint32_t index)
{
    for ( uint32_t i = 0; i <= pclvfr->subItemCount; i++ )
    {
        ListView_SetItemText(hList, index, i, (LPSTR)pclvfr->subItems[i]->pszText);
    }
    return TheTrueObj;
}


/**
 * Sets the item's and subitem's text to the corresponding value in the pclvfr
 * struct.  Checks for a LvFullRow object assigned as the user data for the item
 * and updates that if needed
 *
 * @param c
 * @param pclvfr
 * @param hList
 *
 * @return The true object.
 */
static RexxObjectPtr updateTextUsingFullRow(RexxMethodContext *c, pCLvFullRow pclvfr, HWND hList)
{
    uint32_t index = pclvfr->subItems[0]->iItem;

    pCLvFullRow pclvfrExisting = maybeGetFullRow(hList, index);

    for ( uint32_t i = 0; i <= pclvfr->subItemCount; i++ )
    {
        if ( pclvfr->subItems[i]->mask & LVIF_TEXT )
        {
            ListView_SetItemText(hList, index, i, (LPSTR)pclvfr->subItems[i]->pszText);

            if ( pclvfrExisting != NULL )
            {
                updateFullRowText(c->threadContext, pclvfrExisting, i, pclvfr->subItems[i]->pszText);
            }
        }
    }
    return TheTrueObj;
}


/**
 * Set the item's, and each subitem's, values to the corresponding values in the
 * pclvfr struct.
 *
 * @param pclvfr
 * @param hList
 * @param index
 *
 * @return True on success, false if an error occurs with ListView_SetItem().
 *
 * @remarks  Originally this function was conceived to be used when the
 *           list-view item's user data was a full row object and the Rexx
 *           programmer wanted to sync the list-view item text with what he had
 *           set the full row object to.
 *
 *           However, it would work well in other circustances, *if*, there is
 *           no full row user data assigned to the list-view item that needs to
 *           be updated.
 */
static RexxObjectPtr syncFullRow(pCLvFullRow pclvfr, HWND hList, uint32_t index)
{
    for ( uint32_t i = 0; i <= pclvfr->subItemCount; i++ )
    {
        if ( ! ListView_SetItem(hList, pclvfr->subItems[i]) )
        {
            return TheFalseObj;
        }
    }
    return TheTrueObj;
}


/**
 * Sets the item's and subitem's values to the corresponding values in the
 * pclvfr struct.  Checks for a LvFullRow object assigned as the user data for
 * the item and updates that if needed
 *
 * @param c
 * @param pclvfr
 * @param hList
 *
 * @return The true object.
 *
 * @remarks  There are 3 scenarios we have to manage here.
 *
 *            1.) There is no current lParam user data set.  In this case the
 *            lvItem is just used as is to do a set item.  If a lParam user data
 *            value is set, we need to protect it.
 *
 *            2.) There is a current lParam user data set, but it is not a full
 *            row object.  In this case, if lvItem contains a lParam user data
 *            value, we need to replace the current value with the new value.
 *
 *            3.) There is a current lParam user data set and it is a full row
 *            object.  In this case, if lvItem contains a lParm user data value
 *            we need to update the current value with the new value.  If not,
 *            we need to merge the new lvItem into the full row item.
 */
static RexxObjectPtr modifyFullRow(RexxMethodContext *c, pCDialogControl pcdc, pCLvFullRow pclvfr, HWND hList)
{
    uint32_t index = pclvfr->subItems[0]->iItem;

    RexxObjectPtr oldUserData      = getCurrentLviUserData(hList, index);
    pCLvFullRow   pclvfrExisting   = maybeGetFullRow(hList, index);
    bool          lParamIsModified = (pclvfr->subItems[0]->mask & LVIF_PARAM) ? true : false;

    for ( uint32_t i = 0; i <= pclvfr->subItemCount; i++ )
    {
        LPLVITEM pLvi = pclvfr->subItems[i];

        if ( ! ListView_SetItem(hList, pLvi) )
        {
            return TheFalseObj;
        }

        if ( i == 0 )
        {
            if ( lParamIsModified )
            {
                protectLviUserData(c, pcdc, pLvi);
            }

            if ( oldUserData != TheNilObj )
            {
                if ( lParamIsModified )
                {
                    unProtectControlUserData(c, pcdc, oldUserData);
                }
                else if ( pclvfrExisting != NULL )
                {
                    mergeLviState(c, pclvfrExisting, pclvfrExisting->subItems[0], pLvi);
                }
            }
        }
        else
        {
            if ( pclvfrExisting != NULL && pclvfrExisting->subItemCount >= i )
            {
                if ( pLvi->mask & LVIF_TEXT )
                {
                    updateFullRowText(c->threadContext, pclvfrExisting, i, pLvi->pszText);
                }

                if ( pLvi->mask & LVIF_IMAGE )
                {
                    pclvfr->subItems[i]->iImage = pLvi->iImage;
                }
            }
        }
    }

    return TheTrueObj;
}


/**
 * Adds an item to the list view using a LvFullRow object.  The item is
 * inserted, appended, or prepended to the list, as specified by the FullRowOp
 * type.
 *
 * @param c
 * @param row
 * @param type
 * @param pCSelf
 *
 * @return The index of the newly added item.
 *
 * @remarks  The LvItem object in the row has its item index updated to what is
 *           actually assigned by the list view.
 *
 *           Likewise, the item index in the LvSubItem object(s) is updated to
 *           the the item index. This updating is done before the subitem is
 *           set, so that it is always set correctly.
 *
 *           If the operation is append or prepend, the item index in the LvItem
 *           object is ignored.  Instead it is set to an index that will ensure
 *           the item is inserted at the front of the list or at the end of the
 *           list.
 *
 *           If there are no columns in the list-view, the item insert will
 *           succeed. If column 0 is then inserted late, the item information
 *           shows up correctly.  But, ListView_SetItem() will fail for a
 *           subitem if there is no column for that subitem.  To prevent things
 *           getting hopelessly out of sync, if the user tries to add a full row
 *           with more subitems than columns, we raise a syntax condition.
 *
 *           On the other hand, if there is a column for the subitem, I've never
 *           seen ListView_SetItem() fail.  We check for a failure, and if one
 *           is detected, we deleted the aready inserted item and return -1.
 */
static int32_t fullRowOperation(RexxMethodContext *c, RexxObjectPtr row, FullRowOp type, void *pCSelf)
{
    pCDialogControl pcdc      = validateDCCSelf(c, pCSelf);
    int32_t         itemIndex = -1;

    if ( pcdc != NULL )
    {
        if ( ! c->IsOfType(row, "LVFULLROW") )
        {
            wrongClassException(c->threadContext, 1, "LvFullRow");
            goto done_out;
        }

        pCLvFullRow pclvfr = (pCLvFullRow)c->ObjectToCSelf(row);
        HWND        hwnd   = pcdc->hCtrl;

        if ( type == lvfrAdd )
        {
            pclvfr->subItems[0]->iItem = ListView_GetItemCount(hwnd);
        }
        else if ( type == lvfrPrepend )
        {
            pclvfr->subItems[0]->iItem = 0;
        }

        itemIndex = ListView_InsertItem(hwnd, pclvfr->subItems[0]);

        if ( itemIndex == -1 )
        {
            goto done_out;
        }

        pclvfr->hList = hwnd;
        pclvfr->id    = ListView_MapIndexToID(hwnd, itemIndex);

        pclvfr->subItems[0]->iItem = itemIndex;
        pcdc->lastItem             = itemIndex;
        protectLviUserData(c, pcdc, pclvfr->subItems[0]);

        uint32_t cColumns = (uint32_t)getColumnCount(pcdc->hCtrl);
        if ( cColumns == (uint32_t)-1 )
        {
            severeErrorException(c->threadContext, "the list-view control reports it has no columns; can not continue");
            goto done_out;
        }

        size_t count = pclvfr->subItemCount;

        if ( count >= cColumns )
        {
            char buffer[256];
            _snprintf(buffer, sizeof(buffer), "subitem count (%zd) is invalid for column count(%d)", count, cColumns);
            userDefinedMsgException(c, buffer);

            goto done_out;
        }

        for ( size_t i = 1; i <= count; i++)
        {
            LPLVITEM subItem = pclvfr->subItems[i];
            subItem->iItem = itemIndex;
            if ( ! ListView_SetItem(hwnd, subItem) )
            {
                ListView_DeleteItem(hwnd, itemIndex);
                itemIndex = -1;
            }
        }
    }

done_out:
    return itemIndex;
}

#define BEGIN_EVENT_NOTIFICATION_CODE

inline bool selectionDidChange(LPNMLISTVIEW p)
{
    return ((p->uNewState & LVIS_SELECTED) != (p->uOldState & LVIS_SELECTED));
}

inline bool focusDidChange(LPNMLISTVIEW p)
{
    return ((p->uNewState & LVIS_FOCUSED) != (p->uOldState & LVIS_FOCUSED));
}

/* matchSelectFocus
 * Check that: (a) tag is for select change and focuse change, and (b) that
 * either the selection or the focus actually changed.
 */
inline bool matchSelectFocus(uint32_t tag, LPNMLISTVIEW p)
{
    return ((tag & TAG_SELECTCHANGED) && (tag & TAG_FOCUSCHANGED)) && (selectionDidChange(p) || focusDidChange(p));
}

/* matchSelect
 * Check that: (a) tag is only for selection change and not focuse change, and (b)
 * that the selection actually changed.
 */
inline bool matchSelect(uint32_t tag, LPNMLISTVIEW p)
{
    return ((tag & TAG_SELECTCHANGED) && !(tag & TAG_FOCUSCHANGED)) && (selectionDidChange(p));
}

/* matchFocus
 * Check that: (a) tag is only for focus change and not selection change, and (b)
 * that the focus actually changed.
 */
inline bool matchFocus(uint32_t tag, LPNMLISTVIEW p)
{
    return ((tag & TAG_FOCUSCHANGED) && !(tag & TAG_SELECTCHANGED)) && (focusDidChange(p));
}

/**
 * Helper function to determine a list view item's index using a hit test.
 *
 * @param hwnd  Handle of the list view.
 * @param pIA   Pointer to an item activate structure.
 *
 * @remarks  This function should only be used when the list view is in report
 *           mode.  If the subitem hit test does not produce an item index, we
 *           only look for a y position that falls within the bounding rectangle
 *           of a visible item.
 *
 *           We start with the top visible index and look at each item on the
 *           page.  The count per page only includes fully visible items, so we
 *           also check for a last, partially visible item.
 */
static void getItemIndexFromHitPoint(LPNMITEMACTIVATE pIA, HWND hwnd)
{
    LVHITTESTINFO lvhti = {0};
    lvhti.pt.x = pIA->ptAction.x;
    lvhti.pt.y = pIA->ptAction.y;
    lvhti.flags = LVHT_ONITEM;

    ListView_SubItemHitTestEx(hwnd, &lvhti);
    pIA->iItem = lvhti.iItem;

    if ( pIA->iItem == -1 )
    {
        int topIndex = ListView_GetTopIndex(hwnd);
        int count = ListView_GetCountPerPage(hwnd);
        RECT r;

        for ( int i = topIndex; i <= count; i++)
        {
            if ( ListView_GetItemRect(hwnd, i, &r, LVIR_BOUNDS) )
            {
                if ( lvhti.pt.y >= r.top && lvhti.pt.y < r.bottom )
                {
                    pIA->iItem = i;
                    break;
                }
            }
        }
    }
}

/**
 * Processes the LVN_BEGINGDRAG and LVN_BEGINRDRAG notifications.
 *
 * @param pcpbd
 * @param lParam
 * @param methodName
 * @param tag
 * @param ifFrom
 * @param notifyCode  Sent to the event handler so that the user can distinguish
 *                    between LVN_BEGINDRAG and LVN_BEGINRDRAG.
 *
 * @param rxLV
 *
 * @return MsgReplyType
 *
 * @notes  For the begin drag notifications, we use the TAG_PRESERVE_OLD, even
 *         though we do not have a "new" behavior.  At some point we may add
 *         BEGINGDRAGEX and BEGINRDRAGEX keywords.
 *
 *         For the record, the arguments sent to the event handler under the old
 *         behavior are:
 *
 *           use arg id, item, pt
 *
 *         where id is the list-view control ID, item is the 0-based item ID,
 *         and point is a string "x y"
 *
 *         In 4.2.4 we add a fourth and fifth arg, which mouse button and the
 *         Rexx list view object.
 *
 *         use arg id, item, pt, isLMB, listView
 *
 *         For a "new" behavior the most likely arguments would be:
 *
 *           use arg id, item, pt, isLMB, lvObj
 *
 *         where pt would be a .Point object and we would send the Rexx
 *         list-view object in addition.
 */
MsgReplyType lvnBeginDrag(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag, uint32_t code)
{
    RexxThreadContext *c     = pcpbd->dlgProcContext;
    LPNMLISTVIEW       pnmlv = (LPNMLISTVIEW)lParam;

    if ( (tag & TAG_FLAGMASK) == TAG_PRESERVE_OLD )
    {
        char buf[256];
        sprintf(buf, "%d %d", pnmlv->ptAction.x, pnmlv->ptAction.y);

        RexxObjectPtr isLMB  = (code == LVN_BEGINDRAG) ? TheTrueObj : TheFalseObj;
        RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
        RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);
        RexxObjectPtr item   = c->Int32(pnmlv->iItem);
        RexxObjectPtr point  = c->String(buf);

        RexxArrayObject args = c->ArrayOfFour(idFrom, item, point, isLMB);
        c->ArrayPut(args, rxLV, 5);

        genericInvoke(pcpbd, methodName, args, tag);

        c->ReleaseLocalReference(idFrom);
        c->ReleaseLocalReference(item);
        c->ReleaseLocalReference(point);
        c->ReleaseLocalReference(rxLV);
        c->ReleaseLocalReference(args);
    }
    else
    {
        ;  // Purposively do nothing.
    }

    return ReplyTrue;
}


MsgReplyType lvnBeginEndScroll(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag, uint32_t code)
{
    RexxThreadContext *c  = pcpbd->dlgProcContext;
    NMLVSCROLL        *ps = (NMLVSCROLL  *)lParam;

    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);
    RexxObjectPtr rxDx   = c->Int32(ps->dx);
    RexxObjectPtr rxDy   = c->Int32(ps->dy);

    RexxArrayObject args = c->ArrayOfFour(idFrom, rxDx, rxDy, rxLV);
    c->ArrayPut(args, code == LVN_BEGINSCROLL ? TheTrueObj : TheFalseObj, 5);

    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxDx);
    c->ReleaseLocalReference(rxDy);
    c->ReleaseLocalReference(rxLV);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


MsgReplyType lvnBeginLabelEdit(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag)
{
    RexxThreadContext *c   = pcpbd->dlgProcContext;
    NMLVDISPINFO      *pdi = (NMLVDISPINFO *)lParam;

    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);

    if ( (tag & TAG_FLAGMASK) == TAG_PRESERVE_OLD )
    {
        // To preserve old behavior for DefListEditStater, we don't need
        // to do anything. Otherwise, we need to invoke the named method
        // with the old args, plus the list view object.
        if ( StrCmpI(methodName, "DefListEditStarter") != 0 )
        {
            RexxObjectPtr    useLess = c->Intptr(lParam);
            RexxArrayObject  args    = c->ArrayOfThree(idFrom, useLess, rxLV);

            invokeDispatch(c, pcpbd, methodName, args);

            c->ReleaseLocalReference(idFrom);
            c->ReleaseLocalReference(useLess);
            c->ReleaseLocalReference(rxLV);
            c->ReleaseLocalReference(args);
        }
        return ReplyTrue;
    }

    // Implement the new behavior
    HWND hEdit = ListView_GetEditControl(pdi->hdr.hwndFrom);

    RexxObjectPtr   itemID = c->UnsignedInt32(pdi->item.iItem);
    RexxObjectPtr   rxEdit = createControlFromHwnd(c, pcpbd, hEdit, winEdit, false);
    RexxArrayObject args   = c->ArrayOfFour(idFrom, itemID, rxEdit, rxLV);

    if ( tag & TAG_REPLYFROMREXX )
    {
        RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

        msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
        if ( msgReply == NULL )
        {
            // Dialog is ended, we don't care about releasing local references.
            setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, TRUE);
            return ReplyFalse;
        }

        // Return false to let the text be edited, true to disallow it.
        // The return from Rexx is true to allow, false to disallow.
        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, msgReply == TheTrueObj ? FALSE : TRUE);
    }
    else
    {
        // This is okay, we know the tag is not will reply.
        genericInvoke(pcpbd, methodName, args, tag);
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(itemID);
    c->ReleaseLocalReference(rxLV);
    if ( rxEdit != TheNilObj )
    {
        c->ReleaseLocalReference(rxEdit);
    }
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


MsgReplyType lvnColumnClick(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag)
{
    RexxThreadContext *c  = pcpbd->dlgProcContext;

    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxCol  = c->UnsignedInt32((ULONG)((NM_LISTVIEW *)lParam)->iSubItem);
    RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);

    RexxArrayObject args = c->ArrayOfThree(idFrom, rxCol, rxLV);

    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxCol);
    c->ReleaseLocalReference(rxLV);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


/**
 * Process the LVN_ENDLABELEDIT notification.
 *
 * @param pcpbd
 * @param lParam
 * @param methodName
 * @param tag
 * @param idFrom
 * @param rxLV
 *
 * @return MsgReplyType
 *
 * @remarks  The end label edit notification is sent when the user ends editing
 *           the label. If the user did not cancel the editing then
 *           pdi->item.pszText contains the text the user entered.  If there is
 *           text entered and the event handler returns true, the list view
 *           updates the item label with the new text.  If the event handler
 *           returns false, the item's lable is not changed.  If there is no
 *           text in pdi->item.pszText then the return is ignored.
 *
 *           What the old ooDialog did is always accept the text.  So to
 *           preserve old behavior we just need to always accept the text if
 *           there is any.  The old DefListEventHandler is removed from
 *           ooDialog, so we can not invoke it.  However if the user assigned
 *           there own event handler we need to inovke it, but we add the Rexx
 *           list-view object to the end of the argument list.
 *
 *           Note that SYNC is not allowed for willReply for this event.
 *           Either, the user omits willReply and gets the old behavior, or does
 *           not omit willReply and gets the new behavior.  The new behavior is
 *           that the user must reply true or false.
 */
MsgReplyType lvnEndLabelEdit(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag)
{
    RexxThreadContext *c   = pcpbd->dlgProcContext;
    NMLVDISPINFO      *pdi = (NMLVDISPINFO *)lParam;

    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);

    if ( (tag & TAG_FLAGMASK) == TAG_PRESERVE_OLD )
    {
        if ( StrCmpI(methodName, "DefListEditHandler") != 0 )
        {
            RexxArrayObject args;

            RexxObjectPtr itemID = c->UnsignedInt32(pdi->item.iItem);
            RexxObjectPtr text   = pdi->item.pszText ? c->String(pdi->item.pszText) : NULLOBJECT;

            args = c->ArrayOfFour(idFrom, itemID, text, rxLV);
            invokeDispatch(c, pcpbd, methodName, args);

            if ( text != NULLOBJECT )
            {
                c->ReleaseLocalReference(text);
            }
            c->ReleaseLocalReference(idFrom);
            c->ReleaseLocalReference(itemID);
            c->ReleaseLocalReference(rxLV);
            c->ReleaseLocalReference(args);
        }

        if ( pdi->item.pszText )
        {
            maybeUpdateFullRowText(c, pdi);
        }

        // If there is text we set reply to true, if not we set reply to false.
        // This is the old behavior, it doesn't matter if the user connected
        // their own event handler or not.
        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, pdi->item.pszText ? TRUE : FALSE);
        return ReplyTrue;
    }

    // Implement the new behavior.
    RexxObjectPtr   itemID = c->UnsignedInt32(pdi->item.iItem);
    RexxObjectPtr   text   = pdi->item.pszText ? c->String(pdi->item.pszText) : TheNilObj;
    RexxArrayObject args   = c->ArrayOfFour(idFrom, itemID, text, rxLV);

    if ( (tag & TAG_REPLYFROMREXX) == TAG_REPLYFROMREXX )
    {
        RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

        msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
        if ( msgReply == NULL )
        {
            // requiredBooleanReply() has ended the dialog, so we do not care
            // about releasing local references.
            return ReplyFalse;
        }

        // From Rexx, return true to accept the edited text, false to
        // cancel it.  The return to Windows is the same.  But, if the
        // user canceled the edit and the Rexx programmer returns true,
        // we ignore the Rexx programmer's reply and return FALSE to
        // Windows.
        BOOL windowsReply = (msgReply == TheTrueObj && pdi->item.pszText != NULL) ? TRUE : FALSE;

        if ( windowsReply )
        {
            maybeUpdateFullRowText(c, pdi);
        }

        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, windowsReply);
    }
    else
    {
        // This is okay, we know the tag is not will reply.
        genericInvoke(pcpbd, methodName, args, tag);
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(itemID);
    c->ReleaseLocalReference(rxLV);
    if ( text != TheNilObj )
    {
        c->ReleaseLocalReference(text);
    }
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}

MsgReplyType lvnGetInfoTip(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag)
{
    RexxThreadContext *c   = pcpbd->dlgProcContext;
    NMLVGETINFOTIP    *tip = (LPNMLVGETINFOTIP)lParam;

    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);
    RexxObjectPtr item   = c->Int32(tip->iItem);
    RexxObjectPtr text   = tip->dwFlags == 0 ? c->String(tip->pszText) : c->NullString();
    RexxObjectPtr len    = c->Int32(tip->cchTextMax - 1);

    RexxArrayObject args = c->ArrayOfFour(idFrom, item, text, len);
    c->ArrayPut(args, rxLV, 5);

    RexxObjectPtr rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
    {
        CSTRING newText = c->ObjectToStringValue(rexxReply);
        if ( strlen(newText) > 0 )
        {
            _snprintf(tip->pszText, tip->cchTextMax - 1, "%s", newText);
        }
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(item);
    c->ReleaseLocalReference(text);
    c->ReleaseLocalReference(len);
    c->ReleaseLocalReference(rxLV);
    c->ReleaseLocalReference(args);
    safeLocalRelease(c, rexxReply);

    return ReplyTrue;
}


MsgReplyType lvnItemChanged(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag)
{
    RexxThreadContext *c   = pcpbd->dlgProcContext;
    LPNMLISTVIEW       pLV = (LPNMLISTVIEW)lParam;

    MsgReplyType  msgReply = ReplyTrue;

    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr item   = c->Int32(pLV->iItem);
    RexxObjectPtr rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);

    RexxObjectPtr   flags = NULLOBJECT;
    RexxArrayObject args  = NULLOBJECT;

    char tmpBuffer[20];
    char *p;

    /* The use of the tag field allows a finer degree of control as to exactly
     * which event the user wants to be notified of, then does the initial
     * message match above.  Because of that, this specific LVN_ITEMCHANGED
     * notification may not match the tag.  So, if we do not match here, we
     * continue the search through the message table because this notification
     * may match some latter entry in the table.
     */

    if ( (tag & TAG_STATECHANGED) && (pLV->uChanged == LVIF_STATE) )
    {
        if ( (tag & TAG_CHECKBOXCHANGED) && (pLV->uNewState & LVIS_STATEIMAGEMASK) )
        {
            p = (pLV->uNewState == INDEXTOSTATEIMAGEMASK(2) ? "CHECKED" : "UNCHECKED");

            flags = c->String(p);
            args  = c->ArrayOfFour(idFrom, item, flags, rxLV);

            genericInvoke(pcpbd, methodName, args, tag);
        }
        else if ( matchSelectFocus(tag, pLV) )
        {
            tmpBuffer[0] = '\0';

            if ( selectionDidChange(pLV) )
            {
                if ( pLV->uNewState & LVIS_SELECTED )
                {
                    strcpy(tmpBuffer, "SELECTED");
                }
                else
                {
                    strcpy(tmpBuffer, "UNSELECTED");
                }
            }

            if ( focusDidChange(pLV) )
            {
                if ( pLV->uNewState & LVIS_FOCUSED )
                {
                    tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "FOCUSED") : strcat(tmpBuffer, " FOCUSED");
                }
                else
                {
                    tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "UNFOCUSED") : strcat(tmpBuffer, " UNFOCUSED");
                }
            }

            flags = c->String(tmpBuffer);
            args = c->ArrayOfFour(idFrom, item, flags, rxLV);

            genericInvoke(pcpbd, methodName, args, tag);
        }
        else if ( matchSelect(tag, pLV) )
        {
            p = (pLV->uNewState & LVIS_SELECTED) ? "SELECTED" : "UNSELECTED";

            flags = c->String(p);
            args = c->ArrayOfFour(idFrom, item, flags, rxLV);

            genericInvoke(pcpbd, methodName, args, tag);
            msgReply = ContinueSearching;  // Not sure if this is wise with the C++ API
        }
        else if ( matchFocus(tag, pLV) )
        {
            p = (pLV->uNewState & LVIS_FOCUSED) ? "FOCUSED" : "UNFOCUSED";

            flags = c->String(p);
            args = c->ArrayOfFour(idFrom, item, flags, rxLV);

            genericInvoke(pcpbd, methodName, args, tag);
            msgReply = ContinueSearching;  // Not sure if this is wise with the C++ API
        }
        else
        {
            // This message in the message table does not match, keep searching.
            msgReply = ContinueSearching;
        }
    }

    safeLocalRelease(c, flags);
    safeLocalRelease(c, args);

    return msgReply;
}


/**
 * Processes the LVN_KEYDOWN notification.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @notes  If the user uses the KEYDOWN keyword, then the tag TAG_PRESERVE_OLD
 *         is added, if the user used the KEYDOWNEX keyword then the preserve
 *         old tag is not added.
 *
 *         For both keywords the value of the return is ignored by the OS.  So,
 *         if the user sets will reply to true, we check that a value is
 *         returned from the event handler, but we ignore what value it is.
 *
 *         The args sent for KEYDOWN are, with rxLvObj added from original:
 *
 *         use arg id, vKey, rxLvObj
 *
 *         The args sent for KEYDOWNEX are:
 *
 *         use arg vKey, bShift, bControl, bAlt, info, rxLvObj
 */
MsgReplyType lvnKeyDown(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;
    uint16_t vKey        = ((NMLVKEYDOWN *)lParam)->wVKey;
    bool     preserveOld = (tag & TAG_FLAGMASK) == TAG_PRESERVE_OLD;

    RexxArrayObject args   = NULLOBJECT;
    RexxObjectPtr   rxVKey = NULLOBJECT;
    RexxObjectPtr   idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr   rxLV   = controlFrom2rexxArg(pcpbd, lParam, winListView);

    if ( preserveOld )
    {
        rxVKey = c->UnsignedInt32(vKey);
        args   = c->ArrayOfThree(idFrom, rxVKey, rxLV);
    }
    else
    {
        args = getKeyEventRexxArgs(c, (WPARAM)vKey, false, rxLV);
        c->ArrayPut(args, idFrom, 7);
    }

    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);

    if ( preserveOld )
    {
        c->ReleaseLocalReference(rxVKey);
        c->ReleaseLocalReference(rxLV);
        c->ReleaseLocalReference(args);
    }
    else
    {
        releaseKeyEventRexxArgs(c, args);
    }

    return ReplyTrue;
}


MsgReplyType lvnNmClick(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, uint32_t tag, uint32_t code)
{
    RexxThreadContext *c   = pcpbd->dlgProcContext;
    LPNMITEMACTIVATE   pIA = (LPNMITEMACTIVATE)lParam;

    RexxObjectPtr idFrom  = idFrom2rexxArg(c, lParam);
    RexxObjectPtr isClick = (code == NM_CLICK) ? TheTrueObj : TheFalseObj;
    RexxObjectPtr rxLV    = controlFrom2rexxArg(pcpbd, lParam, winListView);

    char tmpBuffer[20];
    if ( pIA->uKeyFlags == 0 )
    {
        strcpy(tmpBuffer, "NONE");
    }
    else
    {
        tmpBuffer[0] = '\0';

        if ( pIA->uKeyFlags & LVKF_SHIFT )
            strcpy(tmpBuffer, "SHIFT");
        if ( pIA->uKeyFlags & LVKF_CONTROL )
            tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "CONTROL") : strcat(tmpBuffer, " CONTROL");
        if ( pIA->uKeyFlags & LVKF_ALT )
            tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "ALT") : strcat(tmpBuffer, " ALT");
    }

    // The user can click on an item in a list view, or on the
    // background of the list view.  For report mode only, the user can
    // also click on a subitem of the item.  When the click is on the
    // background, the item index and column index will be sent to the
    // Rexx method as -1.
    //
    // In report mode, if the list view has the extended full row select
    // stylye, everything works as expected.  But, without that style,
    // if the user clicks anywhere on the row outside of the item icon
    // and item text, the OS does not report the item index.  This looks
    // odd to the user.  For this case we go to some extra trouble to
    // get the correct item index.
    if ( pIA->iItem == -1 && pIA->iSubItem != -1 )
    {
        HWND hwnd = pIA->hdr.hwndFrom;
        if ( isInReportView(hwnd)  )
        {
            getItemIndexFromHitPoint(pIA, hwnd);
        }
        else
        {
            // iSubItem is always 0 when not in report mode, but -1 is
            // more consistent.
            pIA->iSubItem = -1;
        }
    }

    RexxObjectPtr rxItem    = c->Int32(pIA->iItem);
    RexxObjectPtr rxSubitem = c->Int32(pIA->iSubItem);
    RexxObjectPtr keyState  = c->String(tmpBuffer);

    RexxArrayObject args = c->ArrayOfFour(idFrom, rxItem, rxSubitem, keyState);
    c->ArrayPut(args, isClick, 5);
    c->ArrayPut(args, rxLV, 6);

    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxItem);
    c->ReleaseLocalReference(rxSubitem);
    c->ReleaseLocalReference(keyState);
    c->ReleaseLocalReference(rxLV);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


#define END_EVENT_NOTIFICATION_CODE

/**
 * A list view item sort callback function that sorts the list-view here, using
 * string compares.
 *
 * @param lParam1
 * @param lParam2
 * @param lParamSort
 *
 * @return int32_t
 *
 * @remarks  The Rexx programmer will have had to set the lParam user data field
 *           to a LvFullRow item. If not, we simply return 0 and no sorting is
 *           done.
 *
 *           If lParam1 or lParam2 is null, indicating the user did not set a
 *           data value for the item, we simpley return 0.
 *
 *           It is also possible that the column number specified by the
 *           programmer is greater than the number of subitems in the full row.
 *           There is a check that the column number is not greater than the
 *           number of columns in the list view, but the list view could have a
 *           column and not have any matching subitem in the full row object.
 *           So, we need to check that there is actually a subitem in the row to
 *           match the column number.
 */
int32_t CALLBACK LvInternCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    if ( lParam1 == NULL || lParam2 == NULL )
    {
        return 0;
    }

    if ( ! (isLvFullRowStruct((void *)(lParam1)) && isLvFullRowStruct((void *)(lParam2)))  )
    {
        return 0;
    }

    uint16_t col = LOWORD(lParamSort);

    pCLvFullRow pclvfr1 = (pCLvFullRow)lParam1;
    pCLvFullRow pclvfr2 = (pCLvFullRow)lParam2;

    if ( col > pclvfr1->subItemCount || col > pclvfr2->subItemCount)
    {
        return 0;
    }

    switch ( HIWORD(lParamSort) )
    {
        case lvSortAscending :
            return strcmp(pclvfr1->subItems[col]->pszText, pclvfr2->subItems[col]->pszText);

        case lvSortAscendingI :
            return stricmp(pclvfr1->subItems[col]->pszText, pclvfr2->subItems[col]->pszText);

        case lvSortDescending :
            return strcmp(pclvfr2->subItems[col]->pszText, pclvfr1->subItems[col]->pszText);

        case lvSortDescendingI :
            return stricmp(pclvfr2->subItems[col]->pszText, pclvfr1->subItems[col]->pszText);

        default :
            break;
    }
    return 0;
}


/**
 * A list view item sort callback function that works by invoking a method in
 * the Rexx dialog that does the actual comparison.
 *
 * @param lParam1
 * @param lParam2
 * @param lParamSort
 *
 * @return int32_t
 *
 * @remarks  The Rexx programmer will have had to set the lParam user data field
 *           for each list view item of this to work.  If either lParam1 or
 *           lParam2 is null, indicating the user did not set a data value for
 *           the item, we simpley return 0.
 *
 *           Testing shows that this call back is always invoked on the dialog's
 *           window message processing thread, so we do no need to worry about
 *           doing an AttachThread(), with its subsequent problems of how to do
 *           a DetachThread().
 *
 *           Testing also shows that doing ReleaseLocalReference, significantly
 *           increases the time it takes to sort 1000 items.  Using the elapsed
 *           time count in the the Rexx shows shows an increase from 4.9 seconds
 *           to 6.9 seconds.
 */
int32_t CALLBACK LvRexxCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    if ( lParam1 == NULL || lParam2 == NULL )
    {
        return 0;
    }

    RexxObjectPtr lp1 = lviLParam2UserData(lParam1);
    RexxObjectPtr lp2 = lviLParam2UserData(lParam2);

    pCRexxSort pcrs = (pCRexxSort)lParamSort;
    RexxThreadContext *c = pcrs->threadContext;

    RexxArrayObject args = c->ArrayOfThree(lp1, lp2, pcrs->param);

    RexxObjectPtr reply = c->SendMessage(pcrs->rexxDlg, pcrs->method, args);
    c->ReleaseLocalReference(args);

    if ( msgReplyIsGood(c, pcrs->pcpbd, reply, pcrs->method, false) )
    {
        int32_t answer;
        if ( c->Int32(reply, &answer) )
        {
            c->ReleaseLocalReference(reply);
            return answer;
        }

        // We end the dialog on this error.
        wrongReplyRangeException(c, pcrs->method, INT32_MIN, INT32_MAX, reply);
        c->ReleaseLocalReference(reply);

        endDialogPremature(pcrs->pcpbd, pcrs->pcpbd->hDlg, RexxConditionRaised);
    }
    else
    {
        if ( reply != NULLOBJECT )
        {
            c->ReleaseLocalReference(reply);
        }
    }

    // There was some error if we are here ...
    return 0;
}

/**
 * Set up for ListView::sortItems() when the user indicates the sort should be
 * done internally.
 *
 * The user can specify which column to sort on, whether to sort ascending or
 * desxending, and whethere to sort ignoring case or not.
 *
 * The default is to sort on column 0, ascending, case sensitive.
 *
 * The user changes the default with a directory object whose indexes: COLUMN,
 * ASCENDING, and CASELESS are checked.  A missing index uses the default,
 *
 * Ascending and caseless must be boolean, column must be a non-negative whole
 * number.
 *
 * @param c
 * @param sortInfo
 * @param pcdc
 *
 * @return logical_t
 */
logical_t internalListViewSort(RexxMethodContext *c, RexxObjectPtr sortInfo, pCDialogControl pcdc)
{
    LPARAM sortValue = 0;

    if ( sortInfo == NULLOBJECT )
    {
        sortValue = MAKELPARAM(0, lvSortAscending);
    }
    else
    {
        if ( ! c->IsOfType(sortInfo, "DIRECTORY") )
        {
            wrongClassException(c->threadContext, 2, "Directory");
            goto err_out;
        }

        RexxDirectoryObject info = (RexxDirectoryObject)sortInfo;

        logical_t ascending = TRUE;
        logical_t caseless  = FALSE;
        uint32_t  col       = 0;

        RexxObjectPtr obj = c->DirectoryAt(info, "ASCENDING");
        if ( obj != NULLOBJECT )
        {
            if ( ! c->Logical(obj, &ascending) )
            {
                directoryIndexExceptionList(c->threadContext, 2, "Ascending", "true or false", obj);
                goto err_out;
            }
        }

        obj = c->DirectoryAt(info, "CASELESS");
        if ( obj != NULLOBJECT )
        {
            if ( ! c->Logical(obj, &caseless) )
            {
                directoryIndexExceptionList(c->threadContext, 2, "CASELESS", "true or false", obj);
                goto err_out;
            }
        }

        obj = c->DirectoryAt(info, "COLUMN");
        if ( obj != NULLOBJECT )
        {
            if ( ! c->UnsignedInt32(obj, &col) )
            {
                directoryIndexExceptionMsg(c->threadContext, 2, "COLUMN", "must be a non-negative whole number", obj);
                goto err_out;
            }

            uint32_t count = (uint32_t)getColumnCount(pcdc->hCtrl);
            if ( col >= count  )
            {
                wrongRangeException(c->threadContext, 2, 0, count - 1, col);
                goto err_out;
            }
        }

        if ( ascending )
        {
            sortValue = MAKELPARAM(col, caseless ? lvSortAscendingI : lvSortAscending);
        }
        else
        {
            sortValue = MAKELPARAM(col, caseless ? lvSortDescendingI : lvSortDescending);
        }
    }

    return ListView_SortItems(pcdc->hCtrl, LvInternCompareFunc, sortValue);

err_out:
    return FALSE;
}


/**
 * Sets things up for a list-view sort when the comparison function will invoke
 * a method in the Rexx dialog to do the actual sorting.
 *
 * @param context
 * @param sortInfo
 * @param pcdc
 *
 * @return logical_t
 *
 * @note Testing shows that using a method in the Rexx dialog to do the sort
 *       works well and seems fast with a smallish number of list-view items.
 *       However, with large lists it is slow.  This can be seen with lists
 *       containing a 1000 item.
 */
logical_t rexxListViewSort(RexxMethodContext *context, CSTRING method, RexxObjectPtr sortInfo, pCDialogControl pcdc)
{
    pCRexxSort pcrs = pcdc->pcrs;
    if ( pcrs == NULL )
    {
        pcrs = (pCRexxSort)LocalAlloc(LPTR, sizeof(CRexxSort));
        if ( pcrs == NULL )
        {
            outOfMemoryException(context->threadContext);
            return FALSE;
        }
        pcdc->pcrs = pcrs;
    }

    safeLocalFree(pcrs->method);
    memset(pcrs, 0, sizeof(CRexxSort));

    pcrs->method = (char *)LocalAlloc(LPTR, strlen(method) + 1);
    if ( pcrs->method == NULL )
    {
        outOfMemoryException(context->threadContext);
        return FALSE;
    }

    strcpy(pcrs->method, method);
    pcrs->pcpbd         = pcdc->pcpbd;
    pcrs->rexxDlg       = pcdc->pcpbd->rexxSelf;
    pcrs->rexxCtrl      = pcdc->rexxSelf;
    pcrs->threadContext = pcdc->pcpbd->dlgProcContext;
    pcrs->param         = (argumentExists(2) ? sortInfo : TheNilObj);

    return ListView_SortItems(pcdc->hCtrl, LvRexxCompareFunc, pcrs);
}

/** ListView::add()
 *
 *
 */
RexxMethod2(int32_t, lv_add, ARGLIST, args, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    uint32_t itemIndex = getDCinsertIndex(pCSelf);
    int32_t imageIndex = -1;
    int32_t result = -1;

    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;

    size_t argCount = context->ArraySize(args);
    for ( size_t i = 1; i <= argCount; i++ )
    {
        RexxObjectPtr _text = context->ArrayAt(args, i);
        if ( _text == NULLOBJECT )
        {
            continue;
        }

        lvi.pszText = (LPSTR)context->ObjectToStringValue(_text);

        if ( i < argCount )
        {
            RexxObjectPtr _imageIndex = context->ArrayAt(args, i + 1);
            if ( _imageIndex != NULLOBJECT )
            {
                if ( ! context->Int32(_imageIndex, &imageIndex) )
                {
                    wrongRangeException(context->threadContext, (int)(i + 1), INT32_MIN, INT32_MAX, _imageIndex);
                    result = -1;
                    goto done_out;
                }
            }
        }

        if ( imageIndex > -1 )
        {
            lvi.iImage = imageIndex;
            lvi.mask |= LVIF_IMAGE;
        }

        if ( i == 1 )
        {
            lvi.iItem = itemIndex;
            lvi.iSubItem = 0;

            result = ListView_InsertItem(hList, &lvi);

            if ( result != -1 )
            {
                ((pCDialogControl)pCSelf)->lastItem = result;
            }
        }
        else
        {
            lvi.iItem = itemIndex - 1;
            lvi.iSubItem = (int)(i - 1);

            if ( ListView_SetItem(hList, &lvi) )
            {
                result = lvi.iItem;
            }
        }

        // As soon as we find a non-omitted arg, we quit.  That is / was the
        // behaviour prior to the conversion to the C++ API.
        break;
    }

done_out:
    return result;
}

/** ListView::addExtendedStyle()
 *  ListView::removeExtendedStyle()
 *
 */
RexxMethod3(int32_t, lv_addClearExtendStyle, CSTRING, _style, NAME, method, CSELF, pCSelf)
{
    uint32_t style = parseExtendedStyle(_style);
    if ( style == 0  )
    {
        return -3;
    }

    HWND hList = getDChCtrl(pCSelf);

    if ( *method == 'R' )
    {
        ListView_SetExtendedListViewStyleEx(hList, style, 0);
    }
    else
    {
        ListView_SetExtendedListViewStyleEx(hList, style, style);
    }
    return 0;
}

/** ListView::addFullRow()
 *
 *  Adds an item to the list view at the end of the list using a LvFullRow
 *  object.
 *
 */
RexxMethod2(int32_t, lv_addFullRow, RexxObjectPtr, row, CSELF, pCSelf)
{
    return fullRowOperation(context, row, lvfrAdd, pCSelf);
}

/** ListView::addRow()
 *
 *
 */
RexxMethod5(int32_t, lv_addRow, OPTIONAL_uint32_t, index, OPTIONAL_int32_t, imageIndex, OPTIONAL_CSTRING, text,
            ARGLIST, args, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    index      = (argumentOmitted(1) ? getDCinsertIndex(pCSelf) : index);
    imageIndex = (argumentOmitted(2) ? -1 : imageIndex);
    text       = (argumentOmitted(3) ? "" : text);

    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = index;
    lvi.iSubItem = 0;
    lvi.pszText = (LPSTR)text;

    if ( imageIndex > -1 )
    {
        lvi.iImage = imageIndex;
        lvi.mask |= LVIF_IMAGE;
    }

    int32_t itemIndex = ListView_InsertItem(hList, &lvi);

    if ( itemIndex == -1 )
    {
        goto done_out;
    }
    ((pCDialogControl)pCSelf)->lastItem = itemIndex;

    size_t argCount = context->ArraySize(args);

    for ( size_t i = 4; i <= argCount; i++ )
    {
        RexxObjectPtr _columnText = context->ArrayAt(args, i);
        if ( _columnText == NULLOBJECT )
        {
            continue;
        }

        ListView_SetItemText(hList, itemIndex, (int)(i - 3), (LPSTR)context->ObjectToStringValue(_columnText));
    }

done_out:
    return itemIndex;
}


/** ListView::addRowFromArray()
 *
 *  Inserts an item into this list-view from an array of values.  The string
 *  value of each item in the array is used as the text for the item or
 *  subitem(s).
 *
 *  The string value of the first item in the array will be the text for the
 *  list-view item.  The string value of the second item in the array will be
 *  the text for the first subitem of the list-view item.  The string value of
 *  the third item in the array will be the text for the second subitem of the
 *  list-view item, etc..
 *
 *  The array can not be sparse.
 *
 *  @param data  [required]  The array of objects whose string values are to be
 *               used for the text of the item and subitems.
 *
 *  @param itemIndex  [optional]  The item index to use for the full row.  If
 *                    omitted, the item is added after the last inserted item in
 *                    the list-view
 *
 *  @param strIfNil   [opitonal]  String to use for the item or subitem text, if
 *                    the object in the array is the .nil object.
 *
 *  @return  The index of the inserted item on success, or -1 on error.
 *
 *  @notes
 */
RexxMethod4(int32_t, lv_addRowFromArray, RexxArrayObject, data, OPTIONAL_uint32_t, itemIndex,
            OPTIONAL_CSTRING, nullStr, CSELF, pCSelf)
{
    LVITEM  lvi    = { 0 };
    int32_t iIndex = -1;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    size_t cCols = context->ArrayItems(data);
    if ( cCols == 0 )
    {
        emptyArrayException(context->threadContext, 1);
        goto done_out;
    }

    CSTRING itemText      = NULL;
    bool    substitueText =  argumentOmitted(3) ? false : true;

    // Insert the item first
    RexxObjectPtr arrayItem = context->ArrayAt(data, 1);
    if ( arrayItem == NULLOBJECT )
    {
        sparseArrayException(context->threadContext, 1, 1);
        goto done_out;
    }

    if ( argumentOmitted(2) )
    {
        itemIndex = pcdc->lastItem + 1;
    }
    if ( substitueText )
    {
        itemText = arrayItem == TheNilObj ? nullStr : context->ObjectToStringValue(arrayItem);
    }
    else
    {
        itemText = context->ObjectToStringValue(arrayItem);
    }

    lvi.mask     = LVIF_TEXT | LVIF_GROUPID | LVIF_IMAGE;
    lvi.iItem    = itemIndex;
    lvi.pszText  = (LPSTR)itemText;
    lvi.iGroupId = I_GROUPIDNONE;
    lvi.iImage   = I_IMAGENONE;

    iIndex = ListView_InsertItem(pcdc->hCtrl, &lvi);

    if ( iIndex == -1 )
    {
        goto done_out;
    }
    pcdc->lastItem = iIndex;

    for ( size_t i = 2; i <= cCols; i++ )
    {
        arrayItem = context->ArrayAt(data, i);
        if ( arrayItem == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, i);
            goto done_out;
        }

        if ( substitueText )
        {
            itemText = arrayItem == TheNilObj ? nullStr : context->ObjectToStringValue(arrayItem);
        }
        else
        {
            itemText = context->ObjectToStringValue(arrayItem);
        }

        ListView_SetItemText(pcdc->hCtrl, iIndex, (int)(i - 1), (LPSTR)itemText);
    }

done_out:
    return iIndex;
}


/** ListView::addStyle()
 *  ListView::removeStyle()
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(uint32_t, lv_addRemoveStyle, CSTRING, style, NAME, method, CSELF, pCSelf)
{
    return changeStyle(context, (pCDialogControl)pCSelf, style, NULL, (*method == 'R'));
}

/** ListView::alignLeft()
 *  ListView::arrange()
 *  Listview::alignTop()
 *  ListView::snaptoGrid()
 *
 *  @remarks  MSDN says of ListView_Arrange():
 *
 *  LVA_ALIGNLEFT  Not implemented. Apply the LVS_ALIGNLEFT style instead.
 *  LVA_ALIGNTOP   Not implemented. Apply the LVS_ALIGNTOP style instead.
 *
 *  However, I don't see that changing the align style in these two cases really
 *  does anything.
 */
RexxMethod2(RexxObjectPtr, lv_arrange, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    int32_t flag = 0;
    switch ( method[5] )
    {
        case 'G' :
            flag = LVA_DEFAULT;
            break;
        case 'O' :
            flag = LVA_SNAPTOGRID;
            break;
        case 'L' :
            applyAlignStyle(hList, false);
            return TheZeroObj;
        case 'T' :
            applyAlignStyle(hList, true);
            return TheZeroObj;
    }
    return (ListView_Arrange(hList, flag) ? TheZeroObj : TheFalseObj);
}

/** ListView::BkColor
 *  ListView::TextColor
 *  ListView::TextBkColor
 *
 *
 *  @remarks.  This method is hopelessly outdated.  It should return a COLORREF
 *             so that the user has access to all available colors rather than
 *             be limited to 18 colors out of a 256 color display.
 */
RexxMethod2(int32_t, lv_getColor, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    COLORREF ref;

    if ( *method == 'B' )
    {
        ref = ListView_GetBkColor(hList);
    }
    else if ( method[4] == 'C' )
    {
        ref = ListView_GetTextColor(hList);
    }
    else
    {
        ref = ListView_GetTextBkColor(hList);
    }

    for ( int32_t i = 0; i < 256; i++ )
    {
        if ( ref == PALETTEINDEX(i) )
        {
            return i;
        }
    }
    return -1;
}

/** ListView::BkColor=
 *  ListView::TextColor=
 *  ListView::TextBkColor=
 *
 *
 *  @remarks.  This method is hopelessly outdated.  It should take a COLORREF so
 *             that the user has access to all available colors rather than be
 *             limited to 18 colors out of a 256 color display.
 */
RexxMethod3(RexxObjectPtr, lv_setColor, uint32_t, color, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    COLORREF ref = PALETTEINDEX(color);

    if ( *method == 'B' )
    {
        ListView_SetBkColor(hList, ref);
    }
    else if ( method[4] == 'C' )
    {
        ListView_SetTextColor(hList, ref);
    }
    else
    {
        ListView_SetTextBkColor(hList, ref);
    }
    return NULLOBJECT;
}

/** ListView::check()
 ** ListView::uncheck()
 *
 */
RexxMethod3(int32_t, lv_checkUncheck, int32_t, index, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    if ( ! hasCheckBoxes(hList) )
    {
        return -2;
    }

    ListView_SetCheckState(hList, index, (*method == 'C'));
    return 0;
}

/** ListView::delete()
 *
 *
 */
RexxMethod2(RexxObjectPtr, lv_delete, int32_t, index, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheOneObj;
    }

    unProtectControlUserData(context, pcdc, getCurrentLviUserData(pcdc->hCtrl, index));

    return ListView_DeleteItem(pcdc->hCtrl, index) ? TheZeroObj : TheOneObj;
}

/** ListView::deleteAll()
 *
 *
 */
RexxMethod1(RexxObjectPtr, lv_deleteAll, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheOneObj;
    }

    if ( pcdc->rexxBag != NULL )
    {
        context->SendMessage0(pcdc->rexxBag, "EMPTY");
    }

    return ListView_DeleteAllItems(pcdc->hCtrl) ? TheZeroObj : TheOneObj;
}

/** ListView::deleteColumn()
 *
 *  Deletes the specified column.
 *
 *  @param index  [required]  The zero-based index of the column to delete
 *
 *  @param adjustFullRows  [optional]  If true attempts to fix up the LvFullRow
 *                         objects assigned to the item data of each list-view
 *                         item.  Does nothing if false.  The default is false.
 *
 *  @return 0 on success, 1 on error.
 *
 *  @notes  MSDN says column 0 can not be deleted.  If column 0 must be deleted,
 *          MSDN suggests inserting a dummy column at index 0 and then deleting
 *          column 1.
 *
 *          However, this is not true, column 0 can be deleted.  What can not be
 *          deleted is the list-view item information for column 0.  When a
 *          column for a subitem is deleted, the subitem information is deleted.
 *          But, when column 0 is deleted the item inforation is not deleted.
 *
 *          Only adjust the LvFullRow objects if every list-view item has a
 *          LvFullRow item assigned as the item data, and the programmer has
 *          kept the LVFullRows consistent with changes to the column count of
 *          this list-view.  If these conditions are not met, this method will
 *          return an error, even if the column was deleted successfully.
 */
RexxMethod3(RexxObjectPtr, lv_deleteColumn, uint32_t, index, OPTIONAL_logical_t, adjustFullRows, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheOneObj;
    }

    RexxObjectPtr result = ListView_DeleteColumn(pcdc->hCtrl, index) ? TheZeroObj : TheOneObj;

    if ( result == TheZeroObj && index > 0 && adjustFullRows )
    {
        if ( ! reasonableColumnFix(context, pcdc->hCtrl, index, true) )
        {
            context->ClearCondition();
            result = TheOneObj;
        }
        else
        {
            if ( fixColumns(context, pcdc->hCtrl, index, true) == TheFalseObj )
            {
                result = TheOneObj;
            }
        }
    }

    return result;
}

/** ListView::deselectAll()
 *
 *
 */
RexxMethod1(uint32_t, lv_deselectAll, CSELF, pCSelf)
{
    int32_t  nextItem = -1;
    uint32_t flag     = LVNI_SELECTED;
    uint32_t count    = 0;
    HWND     hLV      = getDChCtrl(pCSelf);

    while ( TRUE )
    {
        nextItem = ListView_GetNextItem(hLV, nextItem, flag);
        if ( nextItem == -1 )
        {
            break;
        }
        ListView_SetItemState(hLV, nextItem, 0, LVIS_SELECTED);
        count++;
    }
    return count;
}

/** ListView::find()
 *  ListView::findPartial()
 *
 */
RexxMethod5(int32_t, lv_find, CSTRING, text, OPTIONAL_int32_t, startItem, OPTIONAL_logical_t, wrap, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    if ( argumentOmitted(2) )
    {
        startItem = -1;
    }

    LVFINDINFO finfo = {0};
    finfo.psz = text;
    finfo.flags = LVFI_STRING;
    if ( wrap )
    {
        finfo.flags = LVFI_STRING | LVFI_WRAP;
    }
    if ( method[4] == 'P' )
    {
        finfo.flags |= LVFI_PARTIAL;
    }

    return ListView_FindItem(hList, startItem, &finfo);
}

/** ListView::findNearestXY()
 *
 *  Finds the item nearest to the position specified by startPoint.  This method
 *  is only valid if the list view is in icon or small icon view.
 *
 *  @param  startPoint  The position, x and y co-ordinates of the starting point
 *                       for the search.  This can be specified in two forms.
 *
 *      Form 1:  arg 1 is a .Point object.
 *      Form 2:  arg 1 is the x co-ordinate and arg2 is the y co-ordinate.
 *
 *  @param  direction   [OPTIONAL] Keyword that controls the direction of the
 *                      search from the start position.  The default is DOWN,
 *                      the keywords are DOWN, UP, LEFT, and RIGHT.
 *
 *
 */
RexxMethod2(int32_t, lv_findNearestXY, ARGLIST, args, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);
    LVFINDINFO finfo = {0};

    if ( ! isInIconView(hList) )
    {
        goto err_out;
    }

    size_t arraySize;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 3, &arraySize, &argsUsed) )
    {
        goto err_out;
    }

    if ( arraySize > (argsUsed + 1) )
    {
        tooManyArgsException(context->threadContext, argsUsed + 1);
        goto err_out;
    }

    finfo.flags = LVFI_NEARESTXY;

    if ( argsUsed == arraySize )
    {
        finfo.vkDirection = VK_DOWN;
    }
    else
    {
        RexxObjectPtr _direction = context->ArrayAt(args, argsUsed + 1);
        CSTRING direction = context->ObjectToStringValue(_direction);

        if ( StrCmpI(direction,      "UP")    == 0 ) finfo.vkDirection = VK_UP;
        else if ( StrCmpI(direction, "LEFT")  == 0 ) finfo.vkDirection  = VK_LEFT;
        else if ( StrCmpI(direction, "RIGHT") == 0 ) finfo.vkDirection  = VK_RIGHT;
        else if ( StrCmpI(direction, "DOWN")  == 0 ) finfo.vkDirection  = VK_DOWN;
        else
        {
            wrongArgValueException(context->threadContext, argsUsed + 1, "DOWN, UP, LEFT, or RIGHT", _direction);
            goto err_out;
        }
    }

    finfo.pt.x = point.x;
    finfo.pt.y = point.y;
    return ListView_FindItem(hList, -1, &finfo);  // TODO what should startItem be????  old code used -1.

err_out:
    return -1;
}


/** ListView::fixFullRowColumns()
 *
 *
 */
RexxMethod3(RexxObjectPtr, lv_fixFullRowColumns, uint32_t, colIndex, OPTIONAL_logical_t, inserted, CSELF, pCSelf)
{
    RexxObjectPtr result = TheFalseObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    bool     isRemove = inserted ? false : true;
    uint32_t cColumns = (uint32_t)getColumnCount(pcdc->hCtrl);
    if ( cColumns == (uint32_t)-1 )
    {
        severeErrorException(context->threadContext, "the list-view control reports it has no columns; can not continue");
        goto done_out;
    }

    if ( colIndex == 0 )
    {
        userDefinedMsgException(context->threadContext, 2, "can not be column 0");
        goto done_out;
    }

    if ( isRemove )
    {
        if ( colIndex > cColumns )
        {
            wrongRangeException(context->threadContext, 2, 1, cColumns, colIndex);
            goto done_out;
        }
    }
    else
    {
        if ( colIndex >= cColumns )
        {
            wrongRangeException(context->threadContext, 2, 1, cColumns - 1, colIndex);
            goto done_out;
        }
    }

    if ( ! reasonableColumnFix(context, pcdc->hCtrl, colIndex, isRemove) )
    {
        goto done_out;
    }

    result = fixColumns(context, pcdc->hCtrl, colIndex, isRemove);

done_out:
    return result;
}

/** ListView::getCheck()
 *
 *
 */
RexxMethod2(int32_t, lv_getCheck, int32_t, index, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    if ( ! hasCheckBoxes(hList) )
    {
        return -2;
    }
    if ( index < 0 || index > ListView_GetItemCount(hList) - 1 )
    {
        return -3;
    }
    return (ListView_GetCheckState(hList, index) == 0 ? 0 : 1);
}

/** ListView::getColumnCount()
 *
 *
 */
RexxMethod1(int, lv_getColumnCount, CSELF, pCSelf)
{
    return getColumnCount(getDChCtrl(pCSelf));
}

/** ListView::getColumnInfo()
 *
 *
 */
RexxMethod3(RexxObjectPtr, lv_getColumnInfo, uint32_t, index, RexxObjectPtr, _d, CSELF, pCSelf)
{
    if ( ! context->IsDirectory(_d) )
    {
        wrongClassException(context->threadContext, 1, "Directory");
        return TheFalseObj;
    }
    RexxDirectoryObject d = (RexxDirectoryObject)_d;

    HWND hList = getDChCtrl(pCSelf);

    LVCOLUMN lvc;
    char buf[256];

    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
    lvc.pszText = buf;
    lvc.cchTextMax = 255;

    if ( ! ListView_GetColumn(hList, index, &lvc) )
    {
        return TheFalseObj;
    }

    char *align = "LEFT";
    if ( (LVCFMT_JUSTIFYMASK & lvc.fmt) == LVCFMT_CENTER )
    {
        align = "CENTER";
    }
    else if ( (LVCFMT_JUSTIFYMASK & lvc.fmt) == LVCFMT_RIGHT )
    {
        align = "RIGHT";
    }

    context->DirectoryPut(d, context->String(lvc.pszText), "TEXT");
    context->DirectoryPut(d, context->Int32(lvc.iSubItem), "SUBITEM");
    context->DirectoryPut(d, context->Int32(lvc.cx), "WIDTH");
    context->DirectoryPut(d, context->String(align), "FMT");

    return TheTrueObj;
}

/** Listview::getColumnOrder()
 *
 *
 */
RexxMethod1(RexxObjectPtr, lv_getColumnOrder, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    int count = getColumnCount(hwnd);
    if ( count == -1 )
    {
        return TheNilObj;
    }

    RexxArrayObject order = context->NewArray(count);
    RexxObjectPtr result = order;

    // the empty array covers the case when count == 0

    if ( count == 1 )
    {
        context->ArrayPut(order, context->Int32(0), 1);
    }
    else if ( count > 1 )
    {
        int *pOrder = (int *)malloc(count * sizeof(int));
        if ( pOrder == NULL )
        {
            outOfMemoryException(context->threadContext);
        }
        else
        {
            if ( ListView_GetColumnOrderArray(hwnd, count, pOrder) == 0 )
            {
                result = TheNilObj;
            }
            else
            {
                for ( int i = 0; i < count; i++)
                {
                    context->ArrayPut(order, context->Int32(pOrder[i]), i + 1);
                }
            }
            free(pOrder);
        }
    }
    return result;
}

/** ListView::getColumnText()
 *
 *
 */
RexxMethod2(RexxStringObject, lv_getColumnText, uint32_t, index, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    LVCOLUMN lvi;
    char buf[256];

    lvi.mask = LVCF_TEXT;
    lvi.pszText = buf;
    lvi.cchTextMax = 255;

    if ( ! ListView_GetColumn(hList, index, &lvi) )
    {
        buf[0] = '\0';
    }

    return context->String(buf);
}

/** ListView::getExtendedStyle()
 *  ListView::getExtendedStyleRaw()
 *
 */
RexxMethod2(RexxObjectPtr, lv_getExtendedStyle, NAME, method, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);
    if ( method[16] == 'R' )
    {
        return context->UnsignedInt32(ListView_GetExtendedListViewStyle(hList));
    }
    else
    {
        return extendedStyleToString(context, hList);
    }
}

/** ListView::getFullRow()
 *
 *  Returns a LvFullRow object for the specified item in this list view.
 *
 *  @param  itemIndex      [required] The index of the item to get the full row
 *                         for.
 *
 *  @param  useForSorting  [optional]  If the full row is going to be used for
 *                         internal sorting, this must be set to true.
 *
 *  @remarks  We first check to see if a LvFullRow object is the stored user
 *            data for the list-view item.  If it is, we just return that.
 *
 *            ------------------------------------------------------------
 *
 *            IMPORTANT:  The current approach we take for LvFullRow objects set
 *            as the item's lParam (user data) is this:  For each ListView
 *            method that alters (modifies) a list-view item's data, we check if
 *            we have a LvFullRow object as the item user data. If so, we update
 *            the LvFullRow object with the modified data. We need to be sure
 *            and maintain that.
 *
 *            Item indexes.  Inserting and deleting items will change the item
 *            index in the LvFullRow objects.  Rather than trying to fix up the
 *            indexes for all LvFullRow objects during an insert of delete, we
 *            just fix up the index at the time the user requests a full row, or
 *            a full row object.  The maybeGetFullRow() method updates the item
 *            index in the Rexx objects to the correct index, if needed.  (We
 *            are using ListView_MapIDToIndex() and ListView_MapIndexToID() to
 *            keep indexes correct.)
 *
 *            Transitory state.  State such as selected, focused, etc., can be
 *            changed and can not be managed by monitoring ListView methods. For
 *            this we the use the updateFullRowItemState() method to get the
 *            current transitory item state before we return a full row object
 *            to the user.
 *
 *            This same basic principle is used for the objects a full row
 *            contains, LvItem and LvSubItem objects.
 *
 *            The one remaining problem is if the user deletes or inserts new
 *            columns.  If this happens, the LvFullRow objects all need to be
 *            fixed up.  We could either monitor the deleteColumn() and
 *            insertColumn() methods and fix all LvFullRow objects during those
 *            methods, or add some method that allows the user to trigger a fix
 *            up.  ?  Undecided.  Fix ups during deleteColumn() / insertColumn()
 *            may be time consuming and for insertColumn() will involve creating
 *            a bunch of LvSubItem objects with no state ...
 *
 *            -----------------------------------------------------------
 *
 *            If there is no LvFullRow object as the user data, the most likely
 *            case, we create the object here.  We do that by creating Rexx
 *            objects for the item and for any subitems we can detect. Detecting
 *            the subitems is dependent on the coulumn count.  Testing has shown
 *            that once the column is inserted, getColumnCount() returns the
 *            correct number, even if the list-view is not in report view.
 *
 *            Note that if userForSorting is true, we always create a new item.
 */
RexxMethod3(RexxObjectPtr, lv_getFullRow, uint32_t, itemIndex, OPTIONAL_logical_t, useForSorting, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = NULL;
    HWND        hList  = getDChCtrl(pCSelf);

    if ( ! useForSorting )
    {
        // If we have a full row, update it and return it.
        pclvfr = maybeGetFullRow(hList, itemIndex);
        if ( pclvfr != NULL )
        {
            updateFullRowItemState(pclvfr);
            return pclvfr->rexxSelf;
        }
    }

    // Okay, no LvFullRow object as the user data (lParam), so create a
    // LvFullRow object from the current state of the item:

    // Create a LvItem
    RexxObjectPtr rxItem = newLvItem(context, hList, itemIndex);
    if ( rxItem == TheNilObj )
    {
        return TheNilObj;
    }

    // Allocate and set up the CLvFullRow struct using Rexx memory
    size_t cCols = getColumnCount(hList);

    RexxBufferObject buf = context->NewBuffer(sizeof(CLvFullRow));
    if ( buf == NULLOBJECT )
    {
        outOfMemoryException(context->threadContext);
        return TheNilObj;
    }

    pclvfr = (pCLvFullRow)context->BufferData(buf);
    memset(pclvfr, 0, sizeof(CLvFullRow));

    size_t size = LVFULLROW_DEF_SUBITEMS;
    if ( cCols >= size )
    {
        size = 2 * cCols;
    }

    pclvfr->subItems   = (LPLVITEM *)LocalAlloc(LPTR, size * sizeof(LPLVITEM *));
    pclvfr->rxSubItems = (RexxObjectPtr *)LocalAlloc(LPTR, size * sizeof(RexxObjectPtr *));

    if ( pclvfr->subItems == NULL || pclvfr->rxSubItems == NULL )
    {
        outOfMemoryException(context->threadContext);
        return TheNilObj;
    }

    LPLVITEM lvi = (LPLVITEM)context->ObjectToCSelf(rxItem);

    pclvfr->magic         = LVFULLROW_MAGIC;
    pclvfr->id            = ListView_MapIndexToID(hList, itemIndex);
    pclvfr->hList         = hList;
    pclvfr->size          = (uint32_t)size;
    pclvfr->subItems[0]   = lvi;
    pclvfr->rxSubItems[0] = rxItem;

    if ( useForSorting )
    {
        pclvfr->subItems[0]->lParam = (LPARAM)pclvfr;
        pclvfr->subItems[0]->mask  |= LVIF_PARAM;
    }

    // Create LvSubItem objects for each column and add them to the struct.
    for ( size_t i = 1; i < cCols; i++ )
    {
        RexxObjectPtr rxSubitem = newLvSubitem(context, hList, itemIndex, (uint32_t)i);

        if ( rxSubitem == TheNilObj )
        {
            goto err_out;
        }

        lvi = (LPLVITEM)context->ObjectToCSelf(rxSubitem);

        pclvfr->subItems[i]   = lvi;
        pclvfr->rxSubItems[i] = rxSubitem;

        pclvfr->subItemCount++;
    }

    // Now instantiate the LvFullRow Rexx object.
    RexxClassObject rxLvFR = rxGetContextClass(context, "LVFULLROW");
    if ( rxLvFR == NULLOBJECT )
    {
        goto err_out;
    }

    RexxObjectPtr result = context->SendMessage1(rxLvFR, "NEW", buf);
    if ( result != NULLOBJECT )
    {
        return result;
    }

err_out:
    safeLocalFree(pclvfr->subItems);
    safeLocalFree(pclvfr->rxSubItems);
    return TheNilObj;
}

/** ListView::getImageList()
 *
 *  Gets the list-view's specifed image list.
 *
 *  @param  type [optional] Identifies which image list to get.  Normal, small,
 *          or state. Normal is the default.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 *
 *  @remarks  Originally the image list type argument had to be the numeric
 *            value of the type.  This was a mistake.  For backward
 *            compatibility we still need to accept a number, but we allow a
 *            string keyword.
 */
RexxMethod2(RexxObjectPtr, lv_getImageList, OPTIONAL_RexxObjectPtr, _type, OSELF, self)
{
    RexxObjectPtr result = TheNilObj;
    uint32_t      type   = getImageListTypeArg(context, _type, 1);

    if ( type != OOD_NO_VALUE )
    {
        result = context->GetObjectVariable(getLVAttributeName(type));
        if ( result == NULLOBJECT )
        {
            result = TheNilObj;
        }
    }

    return result;
}

/** ListView::getItem()
 *
 *  Returns, or fills in, a LvItem object for the item specified.
 *
 *  @param _item  [required] Can be a LvItem object or the item index.
 *
 *                If it is a LvItem, we just do ListView_GetItem() to fill it in
 *                as specified.
 *
 *                If it is an index and we have no LvFullRow, we return a new,
 *                instantiated, LvItem object that reflects the item and the
 *                item's current state.
 *
 *                If we do have a LvFullRow, we ensure the LvFullRow objects
 *                have the item indexes in sync and update the LvItem with the
 *                current transitory state, and then return the Rexx LvItem from
 *                the full row.
 *
 *  @return  When _item is a LvItem object, returns true on success and false on
 *           error.
 *
 *           When the item to get is specified as an index, returns a LvItem
 *           object on success, or the .nil object on error.
 *
 *  @notes  If _item is a LvItem, some, or all, of the information the list-view
 *          maintains on the item is returned by the attributes of the LvItem.
 *          The mask attribute of the LvItem specifies which attributes are set
 *          to the current information of the item.
 *
 *          If _item is the item index, then the attributes of the returned
 *          LvItem object are all set to reflect the current state of the item.
 *
 *  @remarks  If we have a LvFullRow object assigned to the user data, we use
 *  the Rexx item in the full row.  maybeGetFullRow() will update the item
 *  index, and then updateFullRowItemState will sync the LVITEM struct by doing
 *  a fresh ListView_GetItem() which will ensure things are correct.
 *
 */
RexxMethod2(RexxObjectPtr, lv_getItem, RexxObjectPtr, _item, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    HWND          hList  = getDChCtrl(pCSelf);

    if ( context->IsOfType(_item, "LVITEM") )
    {
        LPLVITEM  plvi = (LPLVITEM)context->ObjectToCSelf(_item);
        char     *temp = plvi->pszText;

        BOOL success = ListView_GetItem(hList, plvi);
        if ( success )
        {
            if ( (plvi->mask & LVIF_GROUPID) && plvi->iGroupId == 0 )
            {
                plvi->iGroupId = I_GROUPIDNONE;
            }

            if ( (plvi->mask & LVIF_TEXT) && temp != NULL && plvi->cchTextMax == LVITEM_TEXT_MAX + 1 )
            {
                checkForCallBack(plvi, temp, true);
            }
        }

        return success ? TheTrueObj : TheFalseObj;
    }

    uint32_t itemIndex;
    if ( ! context->UnsignedInt32(_item, &itemIndex) )
    {
        wrongArgValueException(context->threadContext, 1, "a LvItem object or valid item index", _item);
        return TheNilObj;
    }

    pCLvFullRow pclvfr = maybeGetFullRow(hList, itemIndex);
    if ( pclvfr != NULL )
    {
        updateFullRowItemState(pclvfr);
        result = pclvfr->rxSubItems[0];
    }
    else
    {
        result = newLvItem(context, hList, itemIndex);
    }

    return result;
}

/** ListView::getItemData()
 *
 *  The getItemData() method is used to retrieve the stored Rexx object
 *  associated with the specified list view item.
 *
 *  Any list view item can have an associated value stored with it.  In ooDialog
 *  we allow the user to store any Rexx object with the list view item.
 *
 *  @param index  [required]  The item index whose user data is to be retrieved.
 *
 *  @return The Rexx object stored for the specified list view item, or the .nil
 *          object if there is no Rexx object stored with the item.
 */
RexxMethod2(RexxObjectPtr, lv_getItemData, uint32_t, index, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc != NULL )
    {
        result = getCurrentLviUserData(pcdc->hCtrl, index);
    }
    return result;
}

/** ListView::getItemInfo()
 *
 *
 *  @remarks  ListView_GetItem() returns some bogus information if a subitem
 *            index is used.  For example, if the list-view does not have the
 *            SUBITEMIMAGES extended style, it does not update the iImage field.
 *            Subitems can not have a state, but values for the item state are
 *            returned.
 *
 *            So, if the user specifies a subitem index we adjust the return to
 *            be correct.  For a subitem, state is always the empty string,
 *            image is always -1 if the SUBITEMIMAGES style is absent, itemData
 *            is always .nil
 */
RexxMethod4(RexxObjectPtr, lv_getItemInfo, uint32_t, index, RexxObjectPtr, _d, OPTIONAL_uint32_t, subItem, CSELF, pCSelf)
{
    if ( ! context->IsDirectory(_d) )
    {
        wrongClassException(context->threadContext, 2, "Directory");
        return TheFalseObj;
    }
    RexxDirectoryObject d = (RexxDirectoryObject)_d;

    HWND hList = getDChCtrl(pCSelf);

    LVITEM lvi = {0};
    char buf[LVITEM_TEXT_MAX + 1];

    bool subitemImages = hasSubitemImages(hList);

    lvi.iItem      = index;
    lvi.iSubItem   = subItem;
    lvi.mask       = LVIF_TEXT;
    lvi.pszText    = buf;
    lvi.cchTextMax = LVITEM_TEXT_MAX + 1;

    if ( subItem == 0 )
    {
        lvi.mask      |= LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
        lvi.stateMask  = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
    }
    else
    {
        if ( subitemImages )
        {
            lvi.mask |= LVIF_IMAGE;
        }
    }

    if ( ! ListView_GetItem(hList, &lvi) )
    {
        return TheFalseObj;
    }

    checkForCallBack(&lvi, buf, false);

    // Set the text index now because we are going to reuse the buffer for the
    // state text.
    if ( lvi.pszText == LPSTR_TEXTCALLBACK )
    {
        context->DirectoryPut(d, context->String(LPSTR_TEXTCALLBACK_STRING), "TEXT");
    }
    else
    {
        context->DirectoryPut(d, context->String(lvi.pszText), "TEXT");
    }

    *buf = '\0';
    RexxObjectPtr itemData = TheNilObj;
    if ( subItem == 0 )
    {
        if ( lvi.state & LVIS_CUT)         strcat(buf, "CUT ");
        if ( lvi.state & LVIS_DROPHILITED) strcat(buf, "DROP ");
        if ( lvi.state & LVIS_FOCUSED)     strcat(buf, "FOCUSED ");
        if ( lvi.state & LVIS_SELECTED)    strcat(buf, "SELECTED ");

        itemData = lviLParam2UserData(lvi.lParam);
    }
    else
    {
        if ( ! subitemImages )
        {
            lvi.iImage = I_IMAGENONE;
        }
    }

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    context->DirectoryPut(d, context->Int32(lvi.iImage), "IMAGE");
    context->DirectoryPut(d, context->String(buf), "STATE");
    context->DirectoryPut(d, itemData, "ITEMDATA");

    return TheTrueObj;
}

/** ListView::getItemPos()
 *
 */
RexxMethod2(RexxObjectPtr, lv_getItemPos, uint32_t, index, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    POINT p;
    if ( ! ListView_GetItemPosition(hList, index, &p) )
    {
        return TheZeroObj;
    }
    return rxNewPoint(context, p.x, p.y);
}

/** ListView::getIitemRect()
 *
 *  Gets the bounding rectangle for all or part of an item in the current view.
 *
 *  @param  index  [required]  The index of the item for which the rectangle is
 *                 being retrieve.
 *
 *  @param  part   [optional]  Keyword indicating which part of the item
 *                 rectangle is sought.
 *
 *                 BOUNDS
 *                 ICON
 *                 LABEL
 *                 SELECTBOUNDS
 *
 *                 The default is BOUNDS
 *
 *  @return  The specified bounding rectangle on success, the .nil object on
 *           error.
 */
RexxMethod3(RexxObjectPtr, lv_getItemRect, uint32_t, index, OPTIONAL_CSTRING, part, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    uint32_t flag = LVIR_BOUNDS;
    if ( argumentExists(2) )
    {
        flag = keyword2lvir(context, part, true);
        if ( flag == (uint32_t)-1 )
        {
            return NULLOBJECT;
        }
    }

    RECT r;
    if ( ! ListView_GetItemRect(hList, index, &r, flag) )
    {
        return TheNilObj;
    }
    return rxNewRect(context, (PORXRECT)&r);
}

/** ListView::getSubitem()
 *
 *  Returns, or fills in, a LvSubItem object for the subitem specified.
 *
 *  @param _item  [required] Can be a LvSubItem object or the item index.
 *
 *                If it is a LvSubItem, we just do ListView_GetItem() to fill it
 *                in as specified.
 *
 *                If it is an index and we have no LvFullRow, we return a new,
 *                instantiated, LvSubItem object that reflects the item and the
 *                item's current state.
 *
 *                If we do have a LvFullRow, we ensure the LvFullRow objects
 *                have the item indexes in sync, and then return the Rexx
 *                LvSubItem from the full row.
 *
 *  @return  When _item is a LvSubItem object, returns true on success and false
 *           on error.
 *
 *           When the subitem to get is specified as an index, returns a
 *           LvSubItem object on success, or the .nil object on error.
 *
 *  @notes  If _item is a LvSubItem, some, or all, of the information the
 *          list-view maintains on the item is returned by the attributes of the
 *          LvSubItem. The mask attribute of the LvSubItem specifies which
 *          attributes are set to the current information of the item.
 *
 *          If _item is the item index, then the attributes of the returned
 *          LvSubItem object are all set to reflect the current state of the
 *          subitem.
 *
 *          All list-view items have the same number of subitems. The number of
 *          subitems depends on the number of columns the list-view has.  If the
 *          index of the subitem is not valid, this method will fail.
 *
 *  @remarks  The ListView_GetItem macro will not fail, even if the subitem
 *            index is greater than any existing subitem.  So, we check the
 *            subitemIndex with getColumnCount() and fail the method if it is
 *            not valid.
 *
 *            If we have a LvFullRow object assigned to the user data, we use
 *            the Rexx subitem in the full row, if it exists.  maybeGetFullRow()
 *            will update the item index, which will ensure things are correct.
 *
 *  NOTE: remarks from getItemInfo()  we need to test what happens here
 *
 *  @remarks  ListView_GetItem() returns some bogus information if a subitem
 *            index is used.  For example, if the list-view does not have the
 *            SUBITEMIMAGES extended style, it does not update the iImage field.
 *            Subitems can not have a state, but values for the item state are
 *            returned.
 *
 *            So, if the user specifies a subitem index we adjust the return to
 *            be correct.  For a subitem, state is always the empty string,
 *            image is always -1 if the SUBITEMIMAGES style is absent, itemData
 *            is always .nil
 */
RexxMethod3(RexxObjectPtr, lv_getSubitem, RexxObjectPtr, _item, OPTIONAL_uint32_t, subitemIndex, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    HWND    hList = pcdc->hCtrl;
    int32_t count = getColumnCount(hList);

    if ( context->IsOfType(_item, "LVSUBITEM") )
    {
        LPLVITEM plvi = (LPLVITEM)context->ObjectToCSelf(_item);

        if ( count == -1 || plvi->iSubItem == 0 || plvi->iSubItem >= count )
        {
            result = TheFalseObj;
            goto done_out;
        }

        char *temp = plvi->pszText;

        result = ListView_GetItem(hList, plvi) ? TheTrueObj : TheFalseObj;

        if ( result == TheTrueObj && (plvi->mask & LVIF_TEXT) && temp != NULL && plvi->cchTextMax == LVITEM_TEXT_MAX + 1 )
        {
            checkForCallBack(plvi, temp, true);
        }

        goto done_out;
    }

    uint32_t itemIndex;
    if ( ! context->UnsignedInt32(_item, &itemIndex) )
    {
        wrongArgValueException(context->threadContext, 1, "a LvSubItem object or valid item index", _item);
        goto done_out;
    }
    if ( argumentOmitted(2) )
    {
        missingArgException(context->threadContext, 2);
        goto done_out;
    }

    if ( count < 1 || subitemIndex == 0 || subitemIndex >= (uint32_t)count )
    {
        goto done_out;
    }

    pCLvFullRow pclvfr = maybeGetFullRow(hList, itemIndex);
    if ( pclvfr != NULL )
    {
        if ( subitemIndex <= pclvfr->subItemCount )
        {
            result = pclvfr->rxSubItems[subitemIndex];
            goto done_out;
        }
    }

    result = newLvSubitem(context, hList, itemIndex, subitemIndex);

done_out:
    return result;
}

/** ListView::getSubitemRect()
 *
 *  Gets the bounding rectangle for all or part of a subitem in the current view
 *  of the list-view control.
 *
 *  @param  index     [required]  The index of the item for which the rectangle
 *                    is being retrieve.
 *
 *  @param  subIndex  [required]  The index of the subitem for which the
 *                    rectangle is being retrieved.
 *
 *  @param  part      [optional]  Keyword indicating which part of the subitem
 *                    rectangle is sought.
 *
 *                    BOUNDS
 *                    ICON
 *                    LABEL
 *
 *                    The default is BOUNDS
 *
 *  @return  The specified bounding rectangle on success, the .nil object on
 *           error.
 *
 *  @notes  On Vista, subIndex can be 0, in which case this method works exactly
 *          like getItemRect(), with the exception that SELECTBOUNDS is not
 *          accepted as a keyword.
 *
 *          If not on Vista, the list-view is required to be in report view.
 */
RexxMethod4(RexxObjectPtr, lv_getSubitemRect, uint32_t, index, uint32_t, subIndex, OPTIONAL_CSTRING, part, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return NULLOBJECT;
    }
    HWND hList = pcdc->hCtrl;

    uint32_t flag = LVIR_BOUNDS;
    if ( argumentExists(3) )
    {
        flag = keyword2lvir(context, part, false);
        if ( flag == (uint32_t)-1 )
        {
            return NULLOBJECT;
        }
    }

    RECT r = { 0 };

    if ( _isAtLeastVista() )
    {
        LVITEMINDEX lvii;
        lvii.iItem  = index;
        lvii.iGroup = I_GROUPIDNONE;

        if ( ! ListView_GetItemIndexRect(hList, &lvii, subIndex, flag, &r) )
        {
            return TheNilObj;
        }
    }
    else
    {
        if ( ! isInReportView(hList) )
        {
            requiredOS(context, "getSubitemRect", "Vista", Vista_OS);
            return TheNilObj;
        }

        if ( ! ListView_GetSubItemRect(hList, index, subIndex, flag, &r) )
        {
            return TheNilObj;
        }
    }

    return rxNewRect(context, (PORXRECT)&r);
}

/** ListView::getView()
 *
 *
 */
RexxMethod1(RexxObjectPtr, lv_getView, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return context->NullString();
    }
    if ( ! requiredComCtl32Version(context, "ListView::setView", COMCTL32_6_0) )
    {
        return context->NullString();
    }

    return view2keyword(context, ListView_GetView(pcdc->hCtrl));
}

/** ListView::hasCheckBoxes()
 */
RexxMethod1(RexxObjectPtr, lv_hasCheckBoxes, CSELF, pCSelf)
{
    return (hasCheckBoxes(getDChCtrl(pCSelf)) ? TheTrueObj : TheFalseObj);
}

/** ListView::hitTestInfo()
 *
 *  Determine the location of a point relative to the list-view control.
 *
 *  @param  pt  [required]  The position, x and y co-ordinates of the point to
 *              test. This can be specified in two forms.
 *
 *      Form 1:  arg 1 is a .Point object.
 *      Form 2:  arg 1 is the x co-ordinate and arg2 is the y co-ordinate.
 *
 *  @param  info  [optional in/out]  A directory object in which all hit info is
 *                returned.  If the directory is supplied, on return the
 *                directory will have these indexes:
 *
 *                info    Keywords concerning the location of the point.
 *
 *                infoEx  Extended keywords concerning the location of the
 *                        point.  Vista and later.
 *
 *                item    Same value as the return.  The item index if the point
 *                        hits an item, otherwise -1.  (Because list-views are 0
 *                        based indexes in ooDialog.)
 *
 *                subItem The subitem index if the point hits a subitem.
 *
 *                group   Vista and later.  Group index of the item hit (read
 *                        only). Valid only for owner data. If the point is
 *                        within an item that is displayed in multiple groups
 *                        then group will specify the group index of the item.
 *
 *  @return  The index of the item hit, or -1 if the point does not hit an item.
 *
 *  @note    Sometimes the returned index of the hit spot is all that is needed.
 *           In these cases, there is no need to supply the optional directory
 *           object.  However, other times the complete hit test information may
 *           be desired.
 *
 *           Any x, y coordinates will work.  I.e. -6000, -7000 will work. The
 *           item will be -1 and location will be "ABOVE TOLEFT"
 */
RexxMethod2(int32_t, lv_hitTestInfo, ARGLIST, args, CSELF, pCSelf)
{
    LVHITTESTINFO hti = { 0 };
    int32_t result    = -1;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    HWND hwnd = pcdc->hCtrl;

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 3, &sizeArray, &argsUsed) )
    {
        goto done_out;
    }

    bool haveDirectory = (sizeArray > argsUsed) ? true : false;
    RexxDirectoryObject info;

    // Check arg count against expected.
    if ( sizeArray > (haveDirectory ? argsUsed + 1 : argsUsed) )
    {
        tooManyArgsException(context->threadContext, (haveDirectory ? argsUsed + 1 : argsUsed));
        goto done_out;
    }

    if ( haveDirectory )
    {
        RexxObjectPtr _info = context->ArrayAt(args, argsUsed + 1);
        if ( _info == NULLOBJECT || ! context->IsDirectory(_info) )
        {
            wrongClassException(context->threadContext, argsUsed + 1, "Directory");
            goto done_out;
        }

        info = (RexxDirectoryObject)_info;
    }

    hti.pt = point;

    char buf[256];
    *buf = '\0';

    if ( _isAtLeastVista() )
    {
        result = ListView_SubItemHitTestEx(hwnd, &hti);

        if ( haveDirectory )
        {
            context->DirectoryPut(info, context->Int32(hti.iGroup), "GROUP");

            if ( (hti.flags & LVHT_EX_FOOTER          ) == LVHT_EX_FOOTER           ) strcat(buf, "Footer ");
            if ( (hti.flags & LVHT_EX_GROUP           ) == LVHT_EX_GROUP            ) strcat(buf, "Group ");
            if ( (hti.flags & LVHT_EX_GROUP_BACKGROUND) == LVHT_EX_GROUP_BACKGROUND ) strcat(buf, "GroupBackground ");
            if ( (hti.flags & LVHT_EX_GROUP_COLLAPSE  ) == LVHT_EX_GROUP_COLLAPSE   ) strcat(buf, "GroupCollapse ");
            if ( (hti.flags & LVHT_EX_GROUP_FOOTER    ) == LVHT_EX_GROUP_FOOTER     ) strcat(buf, "GroupFooter ");
            if ( (hti.flags & LVHT_EX_GROUP_HEADER    ) == LVHT_EX_GROUP_HEADER     ) strcat(buf, "GroupHeader ");
            if ( (hti.flags & LVHT_EX_GROUP_STATEICON ) == LVHT_EX_GROUP_STATEICON  ) strcat(buf, "GroupStateIcon ");
            if ( (hti.flags & LVHT_EX_GROUP_SUBSETLINK) == LVHT_EX_GROUP_SUBSETLINK ) strcat(buf, "GroupSubsetLink ");
            if ( (hti.flags & LVHT_EX_ONCONTENTS      ) == LVHT_EX_ONCONTENTS       ) strcat(buf, "OnContents ");

            if ( *buf != '\0' )
            {
                *(buf + strlen(buf) - 1) = '\0';
            }
            context->DirectoryPut(info, context->String(buf), "INFOEX");
        }
    }
    else
    {
        result = ListView_SubItemHitTest(hwnd, &hti);
    }

    if ( haveDirectory )
    {
        context->DirectoryPut(info, context->Int32(hti.iItem), "ITEM");
        context->DirectoryPut(info, context->Int32(hti.iSubItem), "SUBITEM");

        *buf = '\0';

        // LVHT_ABOVE and LVHT_ONITEMSTATEICON are the value ;-(
        if ( (hti.flags & LVHT_ABOVE          ) == LVHT_ABOVE && hti.iItem < 0 ) strcat(buf, "Above ");

        if ( (hti.flags & LVHT_BELOW          ) == LVHT_BELOW           ) strcat(buf, "Below ");
        if ( (hti.flags & LVHT_TORIGHT        ) == LVHT_TORIGHT         ) strcat(buf, "ToRight ");
        if ( (hti.flags & LVHT_TOLEFT         ) == LVHT_TOLEFT          ) strcat(buf, "ToLeft ");
        if ( (hti.flags & LVHT_NOWHERE        ) == LVHT_NOWHERE         ) strcat(buf, "NoWhere ");
        if ( (hti.flags & LVHT_ONITEMICON     ) == LVHT_ONITEMICON      ) strcat(buf, "OnIcon ");
        if ( (hti.flags & LVHT_ONITEMLABEL    ) == LVHT_ONITEMLABEL     ) strcat(buf, "OnLabel ");

        if ( (hti.flags & LVHT_ONITEMSTATEICON) == LVHT_ONITEMSTATEICON && hti.iItem > -1 ) strcat(buf, "OnStateIcon ");

        if ( (hti.flags & LVHT_ONITEM         ) == LVHT_ONITEM          ) strcat(buf, "OnItem ");

        if ( *buf != '\0' )
        {
            *(buf + strlen(buf) - 1) = '\0';
        }
        context->DirectoryPut(info, context->String(buf), "INFO");
    }

done_out:
    return result;
}

/** ListView::insert()
 *
 * Inserts a new list view item or a new subitem into in an existing list view
 * item.
 *
 * Note that as a byproduct of the way the underlying Windows API works, this
 * method would also modify an existing subitem.
 *
 * @param itemIndex
 * @param subitemIndex
 * @param text
 * @param imageIndex
 *
 * @return  -1 on error, othewise the inserted item index.
 *
 * @note  If a subitem is being inserted, the returned index will be the index
 *        of the item the subitem is inserted into.
 *
 */
RexxMethod5(int32_t, lv_insert, OPTIONAL_uint32_t, _itemIndex, OPTIONAL_uint32_t, subitemIndex, CSTRING, text,
            OPTIONAL_int32_t, imageIndex, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);
    int32_t newItem = -1;
    int32_t itemIndex = _itemIndex;
    LVITEM lvi = {0};

    if ( argumentOmitted(1) )
    {
        itemIndex = getDCinsertIndex(pCSelf);
        if ( subitemIndex > 0 )
        {
            itemIndex--;
            if ( itemIndex > (ListView_GetItemCount(hList) - 1) )
            {
                userDefinedMsgException(context->threadContext, 2, "A subitem can not be inserted prior to inserting the item");
                goto done_out;
            }
        }
    }

    imageIndex = (argumentOmitted(4) ? -1 : imageIndex);

    lvi.mask = LVIF_TEXT;
    lvi.iItem = itemIndex;
    lvi.iSubItem = subitemIndex;
    lvi.pszText = (LPSTR)text;

    if ( imageIndex > -1 )
    {
        lvi.iImage = imageIndex;
        lvi.mask |= LVIF_IMAGE;
    }

    if ( subitemIndex == 0 )
    {
        newItem = ListView_InsertItem(hList, &lvi);
        ((pCDialogControl)pCSelf)->lastItem = newItem;
    }
    else
    {
        if ( ListView_SetItem(hList, &lvi) )
        {
            newItem = itemIndex;
        }
    }

done_out:
    return newItem;
}

/** ListView::insertColumnPx()
 *
 *
 *  @param column
 *  @param text
 *  @param width   The width of the column in pixels
 *
 *  @param adjustFullRows  [optional]  If true attempts to fix up the LvFullRow
 *                         objects assigned to the item data of each list-view
 *                         item.  Does nothing if false.  The default is false.
 *
 *
 *  @note   Only adjust the LvFullRow objects if every list-view item has a
 *          LvFullRow item assigned as the item data, and the programmer has
 *          kept the LVFullRows consistent with changes to the column count of
 *          this list-view.  If these conditions are not met, this method will
 *          return an error, even if the column was deleted successfully.
 *
 *  @remarks  Even though the width argument in insertColumn() was documented as
 *            being in pixels, the code actually converted it to dialog units.
 *            This method is provided to really use pixels.
 *
 *  @remarks  Not sure why there is a restriction on the length of the column
 *            label, or why the passed text is copied to a buffer.  The
 *            ListView_InsertColumn() API does not impose a limit on the length,
 *            and just asks for a pointer to a string.  Both the length
 *            restriction and the copy are probably not needed.
 */
RexxMethod6(int, lv_insertColumnPx, OPTIONAL_uint16_t, column, CSTRING, text, uint16_t, width,
            OPTIONAL_CSTRING, fmt, OPTIONAL_logical_t, adjustFullRows, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    LVCOLUMN lvi = {0};
    int retVal = 0;
    char szText[256];

    lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;

    // If omitted, column is 0, which is also the default.
    lvi.iSubItem = column;

    lvi.cchTextMax = (int)strlen(text);
    if ( lvi.cchTextMax > (sizeof(szText) - 1) )
    {
        userDefinedMsgException(context->threadContext, 2, "the column title must be less than 256 characters");
        return 0;
    }
    strcpy(szText, text);
    lvi.pszText = szText;
    lvi.cx = width;

    lvi.fmt = LVCFMT_LEFT;
    if ( argumentExists(4) )
    {
        char f = toupper(*fmt);
        if ( f == 'C' )
        {
            lvi.fmt = LVCFMT_CENTER;
        }
        else if ( f == 'R' )
        {
            lvi.fmt = LVCFMT_RIGHT;
        }
    }

    retVal = ListView_InsertColumn(hwnd, lvi.iSubItem, &lvi);
    if ( retVal != -1 && lvi.fmt != LVCFMT_LEFT && lvi.iSubItem == 0 )
    {
        /* According to the MSDN docs: "If a column is added to a
         * list-view control with index 0 (the leftmost column) and with
         * LVCFMT_RIGHT or LVCFMT_CENTER specified, the text is not
         * right-aligned or centered." This is the suggested work around.
         */
        lvi.iSubItem = 1;
        ListView_InsertColumn(hwnd, lvi.iSubItem, &lvi);
        ListView_DeleteColumn(hwnd, 0);
    }

    if ( retVal > 0 && adjustFullRows )
    {
        // retVal is the inserted column indext
        if ( ! reasonableColumnFix(context, hwnd, retVal, false) )
        {
            context->ClearCondition();
            retVal = -1;
        }
        else
        {
            if ( fixColumns(context, hwnd, retVal, false) == TheFalseObj )
            {
                retVal = -1;
            }
        }
    }

    return retVal;
}

/** ListView::insertFullRow()
 *
 *  Inserts an item into the list view at the position specified using a
 *  LvFullRow object.
 *
 */
RexxMethod2(int32_t, lv_insertFullRow, RexxObjectPtr, row, CSELF, pCSelf)
{
    return fullRowOperation(context, row, lvfrInsert, pCSelf);
}

/** ListView::isChecked()
 *
 *
 */
RexxMethod2(RexxObjectPtr, lv_isChecked, int32_t, index, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    if ( hasCheckBoxes(hList) )
    {
        if ( index >= 0 && index <= ListView_GetItemCount(hList) - 1 )
        {
            if ( ListView_GetCheckState(hList, index) != 0 )
            {
                return TheTrueObj;
            }
        }
    }
    return TheFalseObj;
}

/** ListView::itemState()
 *
 *
 */
RexxMethod2(RexxStringObject, lv_itemState, uint32_t, index, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    uint32_t state = ListView_GetItemState(hList, index, LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED);

    char buf[64];
    *buf = '\0';

    if ( state & LVIS_CUT )         strcat(buf, "CUT ");
    if ( state & LVIS_DROPHILITED ) strcat(buf, "DROP ");
    if ( state & LVIS_FOCUSED )     strcat(buf, "FOCUSED ");
    if ( state & LVIS_SELECTED )    strcat(buf, "SELECTED ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return context->String(buf);
}

/** ListView::itemText()
 *
 *
 */
RexxMethod3(RexxStringObject, lv_itemText, uint32_t, index, OPTIONAL_uint32_t, subitem, CSELF, pCSelf)
{
    char buf[256];
    ListView_GetItemText(getDChCtrl(pCSelf), index, subitem, buf, sizeof(buf));
    return context->String(buf);
}

/** ListView::modify()
 *
 *  Modifies the text and or image index for an item.  Or the text only for a
 *  subitem.
 *
 *  @remarks  The docs for the imageIndex argument have read:  Use -1 or simply
 *            omit this argument to leave the icon index unchanged. If subitem
 *            is greater than 0, this argument is ignored.  So, we want to
 *            preserve that, which means this method can not be used to update
 *            the image index for subitems.
 *
 *  @remarks  If the user has a LvFullRox object as the user data, we update the
 *            values in that object after a successful modify().  The user can
 *            only change text and image index in the item here.  And only text
 *            in a subitem.
 *
 *            Testing has shown that if subItemIndex is greater than the column
 *            count, then ListView_SetItem() will fail.  MSDN says that all
 *            items in a list-view have the same number of subitems.  So, we do
 *            not test that the subitemIndex is within the number of columns.
 *            If it is not, the ListView_SetItem() call will fail.
 */
RexxMethod5(RexxObjectPtr, lv_modify, OPTIONAL_uint32_t, itemIndex, OPTIONAL_uint32_t, subitemIndex, CSTRING, text,
            OPTIONAL_int32_t, imageIndex, CSELF, pCSelf)
{
    RexxObjectPtr result = TheOneObj;  // Error result.

    HWND hList = getDChCtrl(pCSelf);

    if ( argumentOmitted(1) )
    {
        itemIndex = getDCinsertIndex(pCSelf);
        if ( subitemIndex > 0 )
        {
            itemIndex--;
        }
    }
    itemIndex  = (argumentOmitted(1) ? getSelected(hList) : itemIndex);
    imageIndex = (argumentExists(4) && subitemIndex == 0 ? imageIndex : -1);

    if ( itemIndex < 0 )
    {
        itemIndex = 0;
    }

    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = itemIndex;
    lvi.iSubItem = subitemIndex;
    lvi.pszText = (LPSTR)text;

    if ( imageIndex > -1 )
    {
        lvi.iImage = imageIndex;
        lvi.mask |= LVIF_IMAGE;
    }

    if ( ListView_SetItem(hList, &lvi) )
    {
        result = TheZeroObj;

        pCLvFullRow pclvfr = maybeGetFullRow(hList, itemIndex);
        if ( pclvfr != NULL )
        {
            updateFullRowText(context->threadContext, pclvfr, subitemIndex, text);

            if ( imageIndex > -1 && subitemIndex <= pclvfr->subItemCount )
            {
                pclvfr->subItems[subitemIndex]->iImage = imageIndex;
            }
        }
    }

    return result;
}

/** ListView::modifyColumnPX()
 *
 *
 * @remarks  LVSCW_AUTOSIZE_USEHEADER and LVSCW_AUTOSIZE are *only* accepted by
 *           ListView_SetColumnWidth()
 */
RexxMethod5(RexxObjectPtr, lv_modifyColumnPx, uint32_t, index, OPTIONAL_CSTRING, label, OPTIONAL_uint16_t, _width,
            OPTIONAL_CSTRING, align, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);
    LVCOLUMN lvi = {0};

    if ( argumentExists(2) && *label != '\0' )
    {
        lvi.pszText = (LPSTR)label;
        lvi.cchTextMax = (int)strlen(label);
        lvi.mask |= LVCF_TEXT;
    }
    if ( argumentExists(3) )
    {
        lvi.cx = _width;
        lvi.mask |= LVCF_WIDTH;
    }
    if ( argumentExists(4) && *align != '\0' )
    {
        if ( StrCmpI(align, "CENTER")     == 0 ) lvi.fmt = LVCFMT_CENTER;
        else if ( StrCmpI(align, "RIGHT") == 0 ) lvi.fmt = LVCFMT_RIGHT;
        else if ( StrCmpI(align, "LEFT")  == 0 ) lvi.fmt = LVCFMT_LEFT;
        else
        {
            wrongArgValueException(context->threadContext, 4, "LEFT, RIGHT, or CENTER", align);
            goto err_out;
        }
        lvi.mask |= LVCF_FMT;
    }

    return (ListView_SetColumn(hList, index, &lvi) ? TheZeroObj : TheOneObj);

err_out:
    return TheNegativeOneObj;
}

/** ListView::modifyFullRow()
 *
 *  Modifies a full row of a list-view item as specified.  That is, modifies the
 *  item and all subitems, if any, of the list-view item.
 *
 *  @param row  [required]  This argument can be either the index of the item to
 *              update, or a LvFullRow object to do the update with.
 *
 *              When row is an index, then the item data for the row *must* be a
 *              LvFullRow object.  Using an index signals that the row has
 *              already been updated and the programmer wants the list-view item
 *              "synched" with the full row.
 *
 *              When row is a LvFullRow object, then the underlying list-view
 *              item's values are set to the full row object's values.  Also,
 *              for this case, if the item being updated also has a full row
 *              object assigned the item data, that full row object's values are
 *              updated, or merged, with the sent full row object's values.
 *
 *  @return  True on success, false on error.
 */
RexxMethod2(RexxObjectPtr, lv_modifyFullRow, OPTIONAL_RexxObjectPtr, row, CSELF, pCSelf)
{
    RexxObjectPtr   result = TheFalseObj;
    pCDialogControl pcdc   = validateDCCSelf(context, pCSelf);

    if ( pcdc == NULL )
    {
        goto done_out;
    }

    uint32_t index;
    HWND     hList = pcdc->hCtrl;

    if ( context->IsOfType(row, "LVFULLROW") )
    {
        result = modifyFullRow(context, pcdc, (pCLvFullRow)context->ObjectToCSelf(row), hList);
    }
    else if ( context->UnsignedInt32(row, &index) )
    {
        pCLvFullRow pclvfr = maybeGetFullRow(hList, index);
        if ( pclvfr == NULL )
        {
            userDefinedMsgException(context, 1, "the item data for the list-view item is not a LvFullRow object");
            goto done_out;
        }
        result = syncFullRow(pclvfr, hList, index);
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, "a LvFullRow object or the list-view item index", row);
    }

done_out:
    return result;
}

/** ListView::modifyItem()
 *
 *  Modifies a list-view item using a LvItem object.
 *
 *  @remarks  There are 3 scenarios we have to manage here.
 *
 *            1.) There is no current lParam user data set.  In this case the
 *            lvItem is just used as is to do a set item.  If a lParam user data
 *            value is set, we need to protect it.
 *
 *            2.) There is a current lParam user data set, but it is not a full
 *            row object.  In this case, if lvItem contains a lParam user data
 *            value, we need to replace the current value with the new value.
 *
 *            3.) There is a current lParam user data set and it is a full row
 *            object.  In this case, if lvItem contains a lParm user data value
 *            we need to update the current value with the new value.  If not,
 *            we need to merge the new lvItem into the full row item.
 */
RexxMethod2(logical_t, lv_modifyItem, RexxObjectPtr, lvItem, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);

    if ( pcdc == NULL )
    {
        goto err_out;
    }

    if ( ! context->IsOfType(lvItem, "LVITEM") )
    {
        wrongClassException(context->threadContext, 1, "LvItem");
        goto err_out;
    }

    HWND     hList = pcdc->hCtrl;
    LPLVITEM lvi   = (LPLVITEM)context->ObjectToCSelf(lvItem);

    RexxObjectPtr oldUserData      = getCurrentLviUserData(hList, lvi->iItem);
    pCLvFullRow   pclvfr           = maybeGetFullRow(hList, lvi->iItem);
    bool          lParamIsModified = (lvi->mask & LVIF_PARAM) ? true : false;

    if ( ListView_SetItem(hList, lvi) == 0 )
    {
        goto err_out;
    }

    if ( lParamIsModified )
    {
        protectLviUserData(context, pcdc, lvi);
    }

    if ( oldUserData != TheNilObj )
    {
        if ( lParamIsModified )
        {
            unProtectControlUserData(context, pcdc, oldUserData);
        }
        else if ( pclvfr != NULL )
        {
            mergeLviState(context, pclvfr, pclvfr->subItems[0], lvi);
        }
    }

    return TRUE;

err_out:
    return FALSE;
}

/** ListView::modifySubitem()
 *
 *
 */
RexxMethod2(logical_t, lv_modifySubitem, RexxObjectPtr, lvSubItem, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);

    if ( pcdc == NULL )
    {
        goto err_out;
    }

    if ( ! context->IsOfType(lvSubItem, "LVSUBITEM") )
    {
        wrongClassException(context->threadContext, 1, "LvSubItem");
        goto err_out;
    }

    LPLVITEM lvi = (LPLVITEM)context->ObjectToCSelf(lvSubItem);

    if ( ListView_SetItem(pcdc->hCtrl, lvi) )
    {
        pCLvFullRow pclvfr = maybeGetFullRow(pcdc->hCtrl, lvi->iItem);

        if ( pclvfr != NULL && pclvfr->subItemCount >= (uint32_t)lvi->iSubItem )
        {
            if ( lvi->mask & LVIF_TEXT )
            {
                updateFullRowText(context->threadContext, pclvfr, lvi->iSubItem, lvi->pszText);
            }

            if ( lvi->mask & LVIF_IMAGE )
            {
                pclvfr->subItems[lvi->iSubItem]->iImage = lvi->iImage;
            }
        }
        return TRUE;
    }

err_out:
    return FALSE;
}

/** ListView::next()
 *  ListView::nextSelected()
 *  ListView::nextLeft()
 *  ListView::nextRight()
 *  ListView::previous()
 *  ListView::previousSelected()
 *
 *
 *  @remarks  For the next(), nextLeft(), nextRight(), and previous() methods,
 *            we had this comment:
 *
 *            The Windows API appears to have a bug when the list contains a
 *            single item, insisting on returning 0.  This, rather
 *            unfortunately, can cause some infinite loops because iterating
 *            code is looking for a -1 value to mark the iteration end.
 *
 *            And in the method did: if self~Items < 2 then return -1
 *
 *            In this code, that check is not added yet, and the whole premise
 *            needs to be tested.  I find no mention of this bug in any Google
 *            searches I have done, and it seems odd that we are the only people
 *            that know about the bug?
 */
RexxMethod3(int32_t, lv_getNextItem, OPTIONAL_int32_t, startItem, NAME, method, CSELF, pCSelf)
{
    uint32_t flag;

    if ( *method == 'N' )
    {
        switch ( method[4] )
        {
            case '\0' :
                flag = LVNI_BELOW | LVNI_TORIGHT;
                break;
            case 'S' :
                flag = LVNI_BELOW | LVNI_TORIGHT | LVNI_SELECTED;
                break;
            case 'L' :
                flag = LVNI_TOLEFT;
                break;
            default :
                flag = LVNI_TORIGHT;
                break;
        }
    }
    else
    {
        flag = (method[8] == 'S' ? LVNI_ABOVE | LVNI_TOLEFT | LVNI_SELECTED : LVNI_ABOVE | LVNI_TOLEFT);
    }

    if ( argumentOmitted(1) )
    {
        startItem = -1;
    }
    return ListView_GetNextItem(getDChCtrl(pCSelf), startItem, flag);
}

/** ListView::prependFullRow()
 *
 *  Adds an item to the list view at the beginning of the list using a LvFullRow
 *  object.
 *
 */
RexxMethod2(int32_t, lv_prependFullRow, RexxObjectPtr, row, CSELF, pCSelf)
{
    return fullRowOperation(context, row, lvfrInsert, pCSelf);
}

/** ListView::removeItemData()
 *
 * @remarks  Note that if the lParam user data is a LvFullRow object, there is
 *           no updating of that object that needs to be done, it is simply
 *           removed from the list-view.
 */
RexxMethod2(RexxObjectPtr, lv_removeItemData, uint32_t, index, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheNilObj;
    }

    RexxObjectPtr result = getCurrentLviUserData(pcdc->hCtrl, index);
    if ( result != TheNilObj )
    {
        LVITEM lvi = {LVIF_PARAM, (int)index}; // cast avoids C4838

        if ( ListView_SetItem(pcdc->hCtrl, &lvi) )
        {
            unProtectControlUserData(context, pcdc, result);
        }
        else
        {
            // Not removed, set result back to the .nil ojbect.
            result = TheNilObj;
        }
    }

    return result;
}

/** ListView::replaceExtendedStyle()
 *
 *
 */
RexxMethod3(int32_t, lv_replaceExtendStyle, CSTRING, remove, CSTRING, add, CSELF, pCSelf)
{
    uint32_t removeStyles = parseExtendedStyle(remove);
    uint32_t addStyles = parseExtendedStyle(add);
    if ( removeStyles == 0 || addStyles == 0  )
    {
        return -3;
    }

    HWND hList = getDChCtrl(pCSelf);
    ListView_SetExtendedListViewStyleEx(hList, removeStyles, 0);
    ListView_SetExtendedListViewStyleEx(hList, addStyles, addStyles);
    return 0;
}

/** ListView::replaceStyle()
 *
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(uint32_t, lv_replaceStyle, CSTRING, removeStyle, CSTRING, additionalStyle, CSELF, pCSelf)
{
    return changeStyle(context, (pCDialogControl)pCSelf, removeStyle, additionalStyle, true);
}

/** ListView::select()
 *  ListView::deselect()
 *  ListView::focus()
 *
 *
 */
RexxMethod3(RexxObjectPtr, lv_setSpecificState, uint32_t, index, NAME, method, CSELF, pCSelf)
{
    uint32_t state = 0;
    uint32_t mask = 0;

    if ( *method == 'S' )
    {
        mask  = LVIS_SELECTED;
        state = LVIS_SELECTED;
    }
    else if ( *method == 'D' )
    {
        mask = LVIS_SELECTED;
    }
    else
    {
        mask  = LVIS_FOCUSED;
        state = LVIS_FOCUSED;
    }
    ListView_SetItemState(getDChCtrl(pCSelf), index, state, mask);
    return TheZeroObj;
}

/** ListView::selected()
 *  ListView::focused()
 *  ListView::dropHighlighted()
 *
 *
 */
RexxMethod2(int32_t, lv_getNextItemWithState, NAME, method, CSELF, pCSelf)
{
    uint32_t flag;

    if ( *method == 'S' )
    {
        flag = LVNI_SELECTED;
    }
    else if ( *method == 'F' )
    {
        flag = LVNI_FOCUSED;
    }
    else
    {
        flag = LVNI_DROPHILITED;
    }
    return ListView_GetNextItem(getDChCtrl(pCSelf), -1, flag);
}

/** ListView::setColumnOrder()
 *
 *
 */
RexxMethod2(logical_t, lv_setColumnOrder, RexxArrayObject, order, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    size_t    items   = context->ArrayItems(order);
    int       count   = getColumnCount(hwnd);
    int      *pOrder  = NULL;
    logical_t success = FALSE;

    if ( count != -1 )
    {
        if ( count != items )
        {
            userDefinedMsgException(context->threadContext, "the number of items in the order array does not match the number of columns");
            goto done;
        }

        int *pOrder = (int *)malloc(items * sizeof(int));
        if ( pOrder != NULL )
        {
            RexxObjectPtr item;
            int column;

            for ( size_t i = 0; i < items; i++)
            {
                item = context->ArrayAt(order, i + 1);
                if ( item == NULLOBJECT || ! context->ObjectToInt32(item, &column) )
                {
                    wrongObjInArrayException(context->threadContext, 1, i + 1, "a valid column number");
                    goto done;
                }
                pOrder[i] = column;
            }

            if ( ListView_SetColumnOrderArray(hwnd, count, pOrder) )
            {
                // If we don't redraw the list view and it is already displayed
                // on the screen, it will look mangled.
                RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
                success = TRUE;
            }
        }
        else
        {
            outOfMemoryException(context->threadContext);
        }
    }

done:
    free(pOrder);
    return success;
}

/** ListView::setColumnWidthPX()
 *
 *
 */
RexxMethod3(RexxObjectPtr, lv_setColumnWidthPx, uint32_t, index, OPTIONAL_RexxObjectPtr, _width, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    int width = getColumnWidthArg(context, _width, 2);
    if ( width == OOD_BAD_WIDTH_EXCEPTION )
    {
        return TheOneObj;
    }

    if ( width == LVSCW_AUTOSIZE || width == LVSCW_AUTOSIZE_USEHEADER )
    {
        if ( !isInReportView(hList) )
        {
            userDefinedMsgException(context->threadContext, 2, "can not be AUTO or AUTOHEADER if not in report view");
            return TheOneObj;
        }
    }

    return (ListView_SetColumnWidth(hList, index, width) ? TheZeroObj : TheOneObj);
}

/** ListView::setFullRowText()
 *
 *  Sets the text for the specified item and all subitems.
 *
 *  @param row  [required]  This argument can be either the index of the item to
 *              update, or a LvFullRow object to do the update with.
 *
 *              When row is an index, then the item data for the row *must* be a
 *              LvFullRow object.  Using an index signals that the text in the
 *              full row has already been updated and the programmer wants the
 *              list-view item "synched" with the text in the full row.
 *
 *              When row is a LvFullRow object, then the underlying list-view
 *              item's text is set to the text in the full row object.  Only
 *              the item and subitems that have the TEXT mask set are updated.
 *              Also, for this case, if the item being updated also has a full
 *              row object assigned the item data, that full row object's text
 *              is updated
 *
 *  @return  True on success, false on error.
 */
RexxMethod2(RexxObjectPtr, lv_setFullRowText, OPTIONAL_RexxObjectPtr, row, CSELF, pCSelf)
{
    RexxObjectPtr   result = TheFalseObj;
    pCDialogControl pcdc   = validateDCCSelf(context, pCSelf);

    if ( pcdc == NULL )
    {
        goto done_out;
    }

    uint32_t index;
    HWND     hList = pcdc->hCtrl;

    if ( context->IsOfType(row, "LVFULLROW") )
    {
        result = updateTextUsingFullRow(context, (pCLvFullRow)context->ObjectToCSelf(row), hList);
    }
    else if ( context->UnsignedInt32(row, &index) )
    {
        pCLvFullRow pclvfr = maybeGetFullRow(hList, index);
        if ( pclvfr == NULL )
        {
            userDefinedMsgException(context, 1, "the item data for the list-view item is not a LvFullRow object");
            goto done_out;
        }
        result = syncFullRowText(pclvfr, hList, index);
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, "a LvFullRow object or the list-view item index", row);
    }

done_out:
    return result;
}

/** ListView::setImageList()
 *
 *  Sets or removes one of a list-view's image lists.
 *
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg serves two purposes.  If ilSrc is .nil or
 *                an .ImageList object, this arg indentifies which of the
 *                list-views image lists is being set, normal, small, or state.
 *                The default is LVSI_NORMAL.
 *
 *                If ilSrc is a bitmap, then this arg is the width of a single
 *                image.  The default is the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in
 *                which case it is the height of the bitmap.  The default is the
 *                height of the actual bitmap
 *
 *  @param ilType [optional]  Only used if ilSrc is a bitmap.  In that case it
 *                indentifies which of the list-views image lists is being set,
 *                normal, small, or state. The default is LVSI_NORMAL.
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 *
 *  @remarks  It is possible for this method to fail, without an exception
 *            raised.  Therefore returning NULLOBJECT on all errors is not
 *            viable.  The question is whether to return .nil on error, or 0.
 *            For now, 0 is returned for an error, *when* an exception has not
 *            been raised.
 *
 *            Originally the image list type argument had to be the numeric
 *            value of the type.  This was a mistake.  For backward
 *            compatibility we still need to accept a number, but we allow a
 *            string keyword.
 */
RexxMethod5(RexxObjectPtr, lv_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_RexxObjectPtr, width, OPTIONAL_int32_t, height, OPTIONAL_RexxObjectPtr, ilType, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    HWND hwnd = getDChCtrl(pCSelf);
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr imageList = NULL;
    HIMAGELIST    himl      = NULL;
    uint32_t      type      = LVSIL_NORMAL;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;

        type = getImageListTypeArg(context, width, 2);
        if ( type == OOD_NO_VALUE )
        {
            goto err_out;
        }
    }
    else if ( context->IsOfType(ilSrc, "IMAGELIST") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }

        type = getImageListTypeArg(context, width, 2);
        if ( type == OOD_NO_VALUE )
        {
            goto err_out;
        }
    }
    else
    {
        uint32_t _width;

        if ( ! context->ObjectToUnsignedInt32(width, &_width) )
        {
            wrongRangeException(context->threadContext, 2, (uint32_t)0, UINT32_MAX, width);
            goto err_out;
        }

        imageList = oodILFromBMP(context, &himl, ilSrc, _width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            result = TheZeroObj;
            goto err_out;
        }

        type = getImageListTypeArg(context, ilType, 4);
        if ( type == OOD_NO_VALUE )
        {
            goto err_out;
        }
    }

    ListView_SetImageList(hwnd, himl, type);
    return rxSetObjVar(context, getLVAttributeName(type), imageList);

err_out:
    return result;
}

/** ListView::setItemData()
 *
 *
 */
RexxMethod3(RexxObjectPtr, lv_setItemData, uint32_t, index, RexxObjectPtr, data, CSELF, pCSelf)
{
    LVITEM lvi = {LVIF_PARAM, (int)index}; // cast avoids C4838

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheFalseObj;
    }

    RexxObjectPtr oldUserData = getCurrentLviUserData(pcdc->hCtrl, index);

    lvi.lParam = getLParamUserData(context, data);

    if ( ListView_SetItem(pcdc->hCtrl, &lvi) != 0 )
    {
        protectLviUserData(context, pcdc, &lvi);
        unProtectControlUserData(context, pcdc, oldUserData);
        return TheTrueObj;
    }
    return TheFalseObj;
}

/** ListView::setItemPos()
 *
 *  Moves a list view item to the specified position, (when the list view is in
 *  icon or small icon view.)
 *
 *  @param  index  The index of the item to move.
 *
 *  The other argument(s) specify the new position, and are optional.  If
 *  omitted the position defaults to (0, 0).  The position can either be
 *  specified using a .Point object, or using an x and a y co-ordinate.
 *
 *  @return  -1 if the list view is not in icon or small icon view, otherwise 0.
 */
RexxMethod4(RexxObjectPtr, lv_setItemPos, uint32_t, index, OPTIONAL_RexxObjectPtr, _obj, OPTIONAL_int32_t, y, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    if ( ! isInIconView(hList) )
    {
        return TheNegativeOneObj;
    }

    POINT p = {0};
    if ( argumentOmitted(2) )
    {
        // Doesn't matter if arg 3 is omitted or not, we just use it.  The
        // default if omitted is 0.
        p.y = y;
    }
    else
    {
        if ( argumentExists(3) )
        {
            // Arg 2 & arg 3 exist, they must both be integers then.
            if ( ! context->Int32(_obj, (int32_t *)&(p.x)) )
            {
                return wrongRangeException(context->threadContext, 2, INT32_MIN, INT32_MAX, _obj);
            }
            p.y = y;
        }
        else
        {
            // Arg 2 exists and arg 3 doesn't.  Arg 2 can be a .Point or an
            // integer.
            if ( context->IsOfType(_obj, "POINT") )
            {
                PPOINT tmp = (PPOINT)context->ObjectToCSelf(_obj);
                p.x = tmp->x;
                p.y = tmp->y;
            }
            else
            {
                // Arg 2 has to be an integer, p.y is already set at its
                // default of 0
                if ( ! context->Int32(_obj, (int32_t *)&(p.x)) )
                {
                    return wrongRangeException(context->threadContext, 2, INT32_MIN, INT32_MAX, _obj);
                }
            }
        }
    }

    ListView_SetItemPosition32(hList, index, p.x, p.y);
    return TheZeroObj;
}

/** ListView::setItemState()
 *
 *
 */
RexxMethod3(RexxObjectPtr, lv_setItemState, uint32_t, index, CSTRING, _state, CSELF, pCSelf)
{
    uint32_t state = 0;
    uint32_t mask = 0;

    if ( StrStrI(_state, "NOTCUT") != NULL )
    {
        mask |= LVIS_CUT;
    }
    else if ( StrStrI(_state, "CUT") != NULL )
    {
        mask |= LVIS_CUT;
        state |= LVIS_CUT;
    }

    if ( StrStrI(_state, "NOTDROP") != NULL )
    {
        mask |= LVIS_DROPHILITED;
    }
    else if ( StrStrI(_state, "DROP") != NULL )
    {
        mask |= LVIS_DROPHILITED;
        state |= LVIS_DROPHILITED;
    }

    if ( StrStrI(_state, "NOTFOCUSED") != NULL )
    {
        mask |= LVIS_FOCUSED;
    }
    else if ( StrStrI(_state, "FOCUSED") != NULL )
    {
        mask |= LVIS_FOCUSED;
        state |= LVIS_FOCUSED;
    }

    if ( StrStrI(_state, "NOTSELECTED") != NULL )
    {
        mask |= LVIS_SELECTED;
    }
    else if ( StrStrI(_state, "SELECTED") != NULL )
    {
        mask |= LVIS_SELECTED;
        state |= LVIS_SELECTED;
    }

    ListView_SetItemState(getDChCtrl(pCSelf), index, state, mask);
    return TheZeroObj;
}

/** ListView::setItemText()
 *
 *  Sets the text for the specified item or subitem.
 *
 *  @remarks  We check for the possibility of a LvFullRow object being set as
 *            the user data, and, if so we update the text in the Rexx object.
 */
RexxMethod4(RexxObjectPtr, lv_setItemText, uint32_t, index, OPTIONAL_uint32_t, subitem, CSTRING, text, CSELF, pCSelf)
{
    HWND hList = getDChCtrl(pCSelf);

    pCLvFullRow pclvfr = maybeGetFullRow(hList, index);
    if ( pclvfr != NULL )
    {
        updateFullRowText(context->threadContext, pclvfr, subitem, text);
    }

    ListView_SetItemText(hList, index, subitem, (LPSTR)text);
    return TheZeroObj;
}

/** ListView::setView()
 *
 *
 */
RexxMethod2(RexxObjectPtr, lv_setView, CSTRING, view, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return context->NullString();
    }
    if ( ! requiredComCtl32Version(context, "ListView::setView", COMCTL32_6_0) )
    {
        return context->NullString();
    }

    uint32_t style = 0;
    uint32_t old   = ListView_GetView(pcdc->hCtrl);

    if ( StrCmpI(view, "ICON"          ) == 0 ) style = LV_VIEW_ICON;
    else if ( StrCmpI(view, "SMALLICON") == 0 ) style = LV_VIEW_SMALLICON;
    else if ( StrCmpI(view, "LIST"     ) == 0 ) style = LV_VIEW_LIST;
    else if ( StrCmpI(view, "REPORT"   ) == 0 ) style = LV_VIEW_DETAILS;
    else
    {
        wrongArgValueException(context->threadContext, 1, "Icon, SmallIcon, List, or Report", view);
        return context->NullString();
    }

    ListView_SetView(pcdc->hCtrl, style);

    return view2keyword(context, old);
}

/** ListView::sortItems()
 *
 *
 */
RexxMethod3(logical_t, lv_sortItems, CSTRING, method, OPTIONAL_RexxObjectPtr, param, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    if ( stricmp(method, "InternalListViewSort") == 0 )
    {
        return internalListViewSort(context, param, pcdc);
    }
    else
    {
        return rexxListViewSort(context, method, param, pcdc);
    }
}

/** ListView::stringWidthPX()
 *
 *
 */
RexxMethod2(int, lv_stringWidthPx, CSTRING, text, CSELF, pCSelf)
{
    return ListView_GetStringWidth(getDChCtrl(pCSelf), text);
}

/** ListView::zTest()
 *
 *  An undocumented method to make it easier to do development testing
 *
 */
RexxMethod1(RexxObjectPtr, lv_zTest, CSELF, pCSelf)
{
    printf("zTest() no test at this time\n");

    return TheTrueObj;
}


/**
 *  Methods for the .LvItem class.
 */
#define LVITEM_CLASS            "LvItem"


inline bool isLviInternalInit(RexxMethodContext *context, RexxObjectPtr index, CSTRING text)
{
    return argumentExists(1) && context->IsBuffer(index) && argumentExists(2) && strcmp(text, LVITEM_OBJ_MAGIC) == 0;
}

inline bool validExtendedTileFormat(int32_t format)
{
    return (uint32_t)format == LVCFMT_LINE_BREAK        ||
           (uint32_t)format == LVCFMT_FILL              ||
           (uint32_t)format == LVCFMT_WRAP              ||
           (uint32_t)format == LVCFMT_NO_TITLE          ||
           (uint32_t)format == LVCFMT_TILE_PLACEMENTMASK;
}

/**
 * Converts a keyword to the proper list view column format, LVCFMT_*, flag.
 *
 * @param c     Method context we are operating in.
 * @param flag  The keyword to convert.
 *
 * @return The flag or OOD_BAD_WIDTH_EXCEPTION on error.
 *
 * @note  Raises a syntax condition if flag is not a proper format flag.
 *
 * @TODO  This code is not actually used anywhere, at this time.  Saved because
 *        it may become usefull in the future.
 */
#if 0
uint32_t keyword2lvcfmt(RexxMethodContext *c, CSTRING flag)
{
    uint32_t val = 0;

    if ( StrCmpI(flag, "LEFT"                ) == 0 ) return LVCFMT_LEFT;
    else if ( StrCmpI(flag, "RIGHT"          ) == 0 ) return LVCFMT_RIGHT;
    else if ( StrCmpI(flag, "CENTER"         ) == 0 ) return LVCFMT_CENTER;
    else if ( StrCmpI(flag, "IMAGE"          ) == 0 ) return LVCFMT_IMAGE;
    else if ( StrCmpI(flag, "BITMAP_ON_RIGHT") == 0 ) return LVCFMT_BITMAP_ON_RIGHT;
    else if ( StrCmpI(flag, "COL_HAS_IMAGES" ) == 0 ) return LVCFMT_COL_HAS_IMAGES;
    else if ( StrCmpI(flag, "FIXED_WIDTH"    ) == 0 ) return LVCFMT_FIXED_WIDTH;
    else if ( StrCmpI(flag, "NO_DPI_SCALE"   ) == 0 ) return LVCFMT_NO_DPI_SCALE;
    else if ( StrCmpI(flag, "FIXED_RATIO"    ) == 0 ) return LVCFMT_FIXED_RATIO;
    else if ( StrCmpI(flag, "LINE_BREAK"     ) == 0 ) return LVCFMT_LINE_BREAK;
    else if ( StrCmpI(flag, "FILL"           ) == 0 ) return LVCFMT_FILL;
    else if ( StrCmpI(flag, "WRAP"           ) == 0 ) return LVCFMT_WRAP;
    else if ( StrCmpI(flag, "NO_TITLE"       ) == 0 ) return LVCFMT_NO_TITLE;
    else if ( StrCmpI(flag, "SPLITBUTTON"    ) == 0 ) return LVCFMT_SPLITBUTTON;

    return OOD_BAD_WIDTH_EXCEPTION;
}
#endif


/**
 * Converts a string of keywords to the proper LVIF_* flag.
 *
 * @param flags
 *
 * @return uint32_t
 *
 * @remarks  The special keyword ALL adds, almose, all flags. LVIF_DI_SETITEM is
 *           only valid in an event notification, so that should be skipped.  I
 *           am not sure about LVIF_NORECOMPUTE though?  Adding it for now.
 */
uint32_t keyword2lvif(CSTRING flags)
{
    uint32_t val = 0;

    if ( StrCmpI(flags, "ALL") == 0 )
    {
        val = LVITEM_ALL_MASK;
        return val;
    }

    if ( StrStrI(flags, "COLFMT")      != NULL ) val |= LVIF_COLFMT;
    if ( StrStrI(flags, "COLUMNS")     != NULL ) val |= LVIF_COLUMNS;
    if ( StrStrI(flags, "GROUPID")     != NULL ) val |= LVIF_GROUPID;
    if ( StrStrI(flags, "IMAGE")       != NULL ) val |= LVIF_IMAGE;
    if ( StrStrI(flags, "INDENT")      != NULL ) val |= LVIF_INDENT;
    if ( StrStrI(flags, "NORECOMPUTE") != NULL ) val |= LVIF_NORECOMPUTE;
    if ( StrStrI(flags, "PARAM")       != NULL ) val |= LVIF_PARAM;
    if ( StrStrI(flags, "STATE")       != NULL ) val |= LVIF_STATE;
    if ( StrStrI(flags, "TEXT")        != NULL ) val |= LVIF_TEXT;

    return val;
}


/**
 * Converts a set of LVIF_* flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject lvif2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & LVIF_COLFMT      ) strcat(buf, "COLFMT ");
    if ( flags & LVIF_COLUMNS     ) strcat(buf, "COLUMNS ");
    if ( flags & LVIF_DI_SETITEM  ) strcat(buf, "DI_SETITEM ");
    if ( flags & LVIF_GROUPID     ) strcat(buf, "GROUPID ");
    if ( flags & LVIF_IMAGE       ) strcat(buf, "IMAGE ");
    if ( flags & LVIF_INDENT      ) strcat(buf, "INDENT ");
    if ( flags & LVIF_NORECOMPUTE ) strcat(buf, "NORECOMPUTE ");
    if ( flags & LVIF_PARAM       ) strcat(buf, "PARAM ");
    if ( flags & LVIF_STATE       ) strcat(buf, "STATE ");
    if ( flags & LVIF_TEXT        ) strcat(buf, "TEXT ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}

/**
 * Converts a string of keywords to the proper LVIS_* flag.
 *
 * Note that activating and glow are included.  LVIS_ACTIVATING is documented as
 * not implemented and LVIS_GLOW is not documented period.
 *
 * @param flags
 * @param isStateMask
 *
 * @return uint32_t
 */
uint32_t keyword2lvis(CSTRING flags, bool isStateMask)
{
    uint32_t val = 0;

    if ( isStateMask )
    {
        if ( StrCmpI(flags, "ALL") == 0 )
        {
            return (uint32_t)-1;
        }

        if ( StrStrI(flags, "FOCUSED")        != NULL ) val |= LVIS_FOCUSED;
        if ( StrStrI(flags, "SELECTED")       != NULL ) val |= LVIS_SELECTED;
        if ( StrStrI(flags, "CUT")            != NULL ) val |= LVIS_CUT;
        if ( StrStrI(flags, "DROPHILITED")    != NULL ) val |= LVIS_DROPHILITED;
        if ( StrStrI(flags, "GLOW")           != NULL ) val |= LVIS_GLOW;
        if ( StrStrI(flags, "ACTIVATING")     != NULL ) val |= LVIS_ACTIVATING;
        if ( StrStrI(flags, "OVERLAYMASK")    != NULL ) val |= LVIS_OVERLAYMASK;
        if ( StrStrI(flags, "STATEIMAGEMASK") != NULL ) val |= LVIS_STATEIMAGEMASK;
    }
    else
    {
        if ( StrStrI(flags, "FOCUSED")        != NULL ) val |= LVIS_FOCUSED;
        if ( StrStrI(flags, "SELECTED")       != NULL ) val |= LVIS_SELECTED;
        if ( StrStrI(flags, "CUT")            != NULL ) val |= LVIS_CUT;
        if ( StrStrI(flags, "DROPHILITED")    != NULL ) val |= LVIS_DROPHILITED;
        if ( StrStrI(flags, "GLOW")           != NULL ) val |= LVIS_GLOW;
        if ( StrStrI(flags, "ACTIVATING")     != NULL ) val |= LVIS_ACTIVATING;
    }

    return val;
}


/**
 * Converts a set of LVIS_* flags to their keyword string.
 *
 * Note that activating and glow are included.  LVIS_ACTIVATING is documented as
 * not implemented and LVIS_GLOW is not documented period.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject lvis2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & LVIS_FOCUSED       ) strcat(buf, "FOCUSED ");
    if ( flags & LVIS_SELECTED      ) strcat(buf, "SELECTED ");
    if ( flags & LVIS_CUT           ) strcat(buf, "CUT ");
    if ( flags & LVIS_DROPHILITED   ) strcat(buf, "DROPHILITED ");
    if ( flags & LVIS_GLOW          ) strcat(buf, "GLOW ");
    if ( flags & LVIS_ACTIVATING    ) strcat(buf, "ACTIVATING ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}

RexxObjectPtr getLviText(RexxMethodContext *c, LPLVITEM pLVI)
{
    if ( pLVI->pszText == NULL )
    {
        return TheNilObj;
    }
    else if ( pLVI->pszText == LPSTR_TEXTCALLBACK )
    {
        return c->String(LPSTR_TEXTCALLBACK_STRING);
    }

    return c->String(pLVI->pszText);
}

/**
 * Sets the text attribute for the LvItem object.
 *
 * If the LvItem is used to receive information, the pszText member has to point
 * to a buffer to recieve the text.  We use the convention that if the text
 * argument is the empty string, we need to allocate a buffer.
 *
 * Also, the MSDN docs say that although the user can set the text to any
 * length, only the first 260 TCHARS are displayed.  So, to keep things a little
 * simplier, we only allow the Rexx programmer to use a string up to 260 TCHARS.
 * This is only enforced here, not currently enforced in the ListView class.
 *
 * We use lpStrTextCallBack as the string to indicate the call back value.
 *
 * @param c
 * @param pLVI
 * @param text
 *
 * @return bool
 */
bool setLviText(RexxMethodContext *c, LPLVITEM pLVI, CSTRING text, size_t argPos)
{
    if ( StrCmpI(text, "lpStrTextCallBack") == 0 )
    {
        pLVI->pszText    = LPSTR_TEXTCALLBACK;
        pLVI->cchTextMax = 0;
        return true;
    }

    size_t len = strlen(text);

    bool receiving = (len == 0 ? true : false);

    if ( len > LVITEM_TEXT_MAX )
    {
        stringTooLongException(c->threadContext, argPos, LVITEM_TEXT_MAX, len);
        return false;
    }

    safeLocalFree(pLVI->pszText);
    pLVI->pszText = NULL;

    if ( receiving )
    {
        len = LVITEM_TEXT_MAX;
    }

    pLVI->pszText = (char *)LocalAlloc(LPTR, len + 1);
    if ( pLVI->pszText == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(pLVI->pszText, text);
    pLVI->mask |= LVIF_TEXT;

    pLVI->cchTextMax = (int)(len + 1);

    return true;
}

RexxObjectPtr setLviUserData(RexxMethodContext *c, LPLVITEM lvi, RexxObjectPtr data)
{
    lvi->lParam  = getLParamUserData(c, data);
    lvi->mask   |= LVIF_PARAM;

    c->SetObjectVariable("USERDATA", data);
    return NULLOBJECT;
}

/**
 * Gets the group ID of this item.
 *
 * Note that group IDs are sort of like resource IDs, they are not the index of
 * a group.  A group at index 3, might have a group ID of 101.  All items with a
 * group ID of 101 belong to that group at index 3.
 *
 * @param c
 * @param pLVI
 *
 * @return int32_t
 */
int32_t getLviGroupID(RexxMethodContext *c, LPLVITEM pLVI)
{
    if ( ! requiredComCtl32Version(c, "LvItem::groupID", COMCTL32_6_0) )
    {
        return I_GROUPIDNONE;
    }
    return pLVI->iGroupId;
}

RexxObjectPtr setLviGroupID(RexxMethodContext *c, LPLVITEM pLVI, int32_t id)
{
    if ( ! requiredComCtl32Version(c, "LvItem::groupID", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }
    if ( id < I_GROUPIDNONE )
    {
        id = I_GROUPIDNONE;
    }

    pLVI->iGroupId = id;
    pLVI->mask |= LVIF_GROUPID;

    return NULLOBJECT;
}

RexxArrayObject getLviColumns(RexxMethodContext *c, LPLVITEM pLVI)
{
    if ( ! requiredComCtl32Version(c, "LvItem::columns", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    uint32_t  count    = pLVI->cColumns;
    uint32_t *pColumns = pLVI->puColumns;

    RexxArrayObject columns = c->NewArray(count);
    for ( uint32_t i = 0; i < count; i++)
    {
        c->ArrayPut(columns, c->UnsignedInt32(pColumns[i]), i + 1);
    }

    return columns;
}

RexxObjectPtr setLviColumns(RexxMethodContext *c, LPLVITEM pLVI, RexxArrayObject _columns, size_t argPos)
{
    if ( ! requiredComCtl32Version(c, "LvItem::columns", COMCTL32_6_0) )
    {
        goto done_out;
    }

    size_t    items   = c->ArrayItems(_columns);
    logical_t success = FALSE;

    if ( items < 1 || items > 20 )
    {
        userDefinedMsgException(c->threadContext, "the number of items in the columns array must be greater than 0 and less than 21");
        goto done_out;
    }

    uint32_t *pColumns = (uint32_t *)malloc(items * sizeof(uint32_t));
    if ( pColumns != NULL )
    {
        RexxObjectPtr item;
        uint32_t column;

        for ( size_t i = 0; i < items; i++)
        {
            item = c->ArrayAt(_columns, i + 1);
            if ( item == NULLOBJECT )
            {
                sparseArrayException(c->threadContext, argPos, i + 1);
                goto done_out;
            }
            if ( ! c->ObjectToUnsignedInt32(item, &column) || column < 1)
            {
                wrongObjInArrayException(c->threadContext, argPos, i + 1, "a valid column number", item);
                goto done_out;
            }

            pColumns[i] = column;
        }

        pLVI->cColumns   = (uint32_t)items;
        pLVI->puColumns  = pColumns;
        pLVI->mask      |= LVIF_COLUMNS;
    }
    else
    {
        outOfMemoryException(c->threadContext);
    }

done_out:
    return NULLOBJECT;
}


RexxArrayObject getLviColumnFormats(RexxMethodContext *c, LPLVITEM pLVI)
{
    if ( ! requiredOS(c, "LvItem::columnFormats", "Vista", Vista_OS) )
    {
        return 0;
    }

    uint32_t  count    = pLVI->cColumns;
    int32_t  *pFormats = pLVI->piColFmt;

    RexxArrayObject formats = c->NewArray(count);
    for ( uint32_t i = 0; i < count; i++)
    {
        c->ArrayPut(formats, c->Int32(pFormats[i]), i + 1);
    }

    return formats;
}

/**
 * Transforms an array of Rexx numbers into an array of LVCFMT_* flags.
 *
 * The Rexx program should use the provided LVCFMT_* ::constants to construct
 * the array.  Only these constants / flags are valid:
 *
 *   LVCFMT_LINE_BREAK
 *   LVCFMT_FILL
 *   LVCFMT_WRAP
 *   LVCFMT_NO_TITLE
 *   LVCFMT_TILE_PLACEMENTMASK
 *
 * @param c
 * @param pLVI
 * @param _formats
 * @param argPos
 *
 * @return RexxObjectPtr
 *
 * @notes  Since this code is newer than 4.2.0, and it seems that only a single
 *         flag can be set, not a combination of flags, the Rexx programmer is
 *         required to use the constant values rather than string keywords.
 */
RexxObjectPtr setLviColumnFormats(RexxMethodContext *c, LPLVITEM pLVI, RexxArrayObject _formats, size_t argPos)
{
    if ( ! requiredOS(c, "LvItem::columnFormats", "Vista", Vista_OS) )
    {
        return 0;
    }

    size_t    items   = c->ArrayItems(_formats);
    logical_t success = FALSE;

    if ( items < 1 || items > 20 )
    {
        userDefinedMsgException(c->threadContext, "the number of items in the column formats array must be greater than 0 and less than 21");
        goto done_out;
    }

    int32_t *pFormats = (int32_t *)malloc(items * sizeof(int32_t *));
    if ( pFormats != NULL )
    {
        RexxObjectPtr item;
        int32_t format = -1;

        for ( size_t i = 0; i < items; i++)
        {
            item = c->ArrayAt(_formats, i + 1);
            if ( item == NULLOBJECT )
            {
                sparseArrayException(c->threadContext, argPos, i + 1);
                goto done_out;
            }
            if ( ! c->ObjectToInt32(item, &format) || ! validExtendedTileFormat(format) )
            {
                wrongObjInArrayException(c->threadContext, argPos, i + 1, "a valid column format", item);
                goto done_out;
            }

            pFormats[i] = format;
        }

        pLVI->cColumns  = (uint32_t)items;
        pLVI->piColFmt  = pFormats;
        pLVI->mask     |= LVIF_COLFMT;
    }
    else
    {
        outOfMemoryException(c->threadContext);
    }

done_out:
    return NULLOBJECT;
}


/** LvItem::init()      [Class]
 *
 */
RexxMethod1(RexxObjectPtr, lvi_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, LVITEM_CLASS) )
    {
        TheLvItemClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheLvItemClass);
    }
    return NULLOBJECT;
}


/** LvItem::uninit()
 *
 */
RexxMethod1(RexxObjectPtr, lvi_unInit, CSELF, pCSelf)
{
#if 0
    printf("In lvi_unInit() pCSelf=%p\n", pCSelf);
#endif

    if ( pCSelf != NULLOBJECT )
    {
        LPLVITEM pLVI = (LPLVITEM)pCSelf;

#if 0
    printf("In lvi_unInit() pszText=%p\n", pLVI->pszText);
#endif
        safeLocalFree(pLVI->pszText);
        safeLocalFree(pLVI->puColumns);
        safeLocalFree(pLVI->piColFmt);
    }
    return NULLOBJECT;
}

/** LvItem::init()
 *
 *  Instantiates a LvItem object which can be used to specify or receive the
 *  attributes of a list-view item.
 *
 *  When the LvItem object is used to specify the attributes of a list-view item
 *  set the LvItem attributes to the desired values of the underlying list-view
 *  item.  When used to retrieve the attributes of the underlying list-view
 *  item, set the mask argument(s) to specify which attributes are to be
 *  received.
 *
 *  Each argument to new() is used to set the value of the attribute with the
 *  same name.  I.e., the 'text' argument sets the value of the 'text' attribute
 *  of this LvItem object.
 *
 *  @param  index   [optional] Zero-based index of the item.  Can not be less
 *                  than 0.  If omitted, the default is 0.  When used as part of
 *                  A LvFullRow object in either the addFullRow() or
 *                  appendFullRow() methods, this index is ignored.  See the
 *                  remarks section
 *
 *  @param  text    [optional]  The text for the item.
 *
 *  @param  imageIndex  [optional]  Index of the item's icon in the control's
 *                      image list. This applies to both the large and small
 *                      image list.
 *
 *                      This attribute can be set to the constant I_IMAGENONE to
 *                      indicate the item does not have an icon in the image
 *                      list. If this argument is omitted, it defaults to
 *                      I_IMAGENONE.
 *
 *  @param  itemData    [optional]  The list-view control can store a single
 *                      user value with each list-view item.  The Rexx
 *                      programmer can use this feature to store any single Rexx
 *                      object with each list-view item.
 *
 *  @param  itemState   [optional]
 *
 *  @param  itemStateMask  This value specifies which item state values will be
 *                         retrieved or modified.  The keyword ALL can be used
 *                         to specify all state values.
 *
 *  @notes    In general, if the LvItem object is going to be used to set an
 *            item, the user does not need to specify the mask value, the proper
 *            mask is created depending on what attributes the user assigned
 *            values to.  I.e., if the user assigns some text to the text
 *            attribute, the LVIF_TEXT flag is automatically added to the mask.
 *
 *            On the other hand, if the LvItem is going to be used to retrieve
 *            values, the user needs to set the mask to specify which values are
 *            to be retrieved.
 *
 *            Note that not all of the LvItem object's attributes can be set
 *            through the new() method.  The overlayImageIndex, stateImageIndex,
 *            columnFormats, and groupIndex attributes are set through their
 *            attribute methods.
 *
 *  @remarks  We allow a new LvItem to be instantiated internally by allocating
 *            the CSelf buffer for the object and passing it in as the first
 *            argument.  We check for this by checking if the first argument is
 *            a Rexx buffer object and the second argument is the "magic" string
 *            value.
 *
 *            The getItem() method will take a LvItem object as input.  The user
 *            can then set the mask to specify what information is to be gotten.
 *            Because of this, if the text argument is omitted, we need to check
 *            the mask and set up a buffer to recieve the text if LVIF_TEXT is
 *            specified.  The same thing applies to columns and colFormat
 *            attributes, we need to allocate the buffers to receive the
 *            inofmation
 *
 *            We have one problem.  If the user is constucting this LvItem to
 *            receive information we would like to not set any attriubtes, allow
 *            them to be filled in as specified by the mask.  But, if the user
 *            is constructing this LvItem to set the item and just omits the
 *            imageIndex arg, we would like to set imageIndex to I_IMAGENONE.
 *            Same thing applies to the group ID.
 *
 *            But, we don't know what the user's intent is ... so, for now we
 *            are setting the image index to I_IMAGENONE if the imageIndex arg
 *            is omitted and the group ID to I_GROUPIDNONE if the groupID arg is
 *            omitted..
 *
 *            The 'itemState' argument - Although the item state member in the
 *            LVITEM struct is a single value for the state, the overlay image
 *            index, and the state image index, we use this arg only for the
 *            state. 2 separate attributes are used for the overlay and state
 *            image indexes. To set either of the 2 indexes, the user must set
 *            the value of those attributes individually. They can not be set
 *            through arguments to new().
 *
 *            columnFormats attribute:  The value of the columnFormats attribute
 *            has no effect in standard tile views. For extended tile views,
 *            only the LVCFMT_LINE_BREAK, LVCFMT_FILL, LVCFMT_WRAP,
 *            LVCFMT_NO_TITLE, and LVCFMT_TILE_PLACEMENTMASK constants are
 *            valid.
 */
RexxMethod10(RexxObjectPtr, lvi_init, OPTIONAL_RexxObjectPtr, _index, OPTIONAL_CSTRING, text,
             OPTIONAL_int32_t, imageIndex, OPTIONAL_RexxObjectPtr, itemData, OPTIONAL_CSTRING, mask,
             OPTIONAL_CSTRING, itemState, OPTIONAL_CSTRING, itemStateMask, OPTIONAL_uint32_t, indent,
             OPTIONAL_int32_t, groupID, OPTIONAL_RexxArrayObject, columns)
{
    if ( isLviInternalInit(context, _index, text) )
    {
        context->SetObjectVariable("CSELF", _index);
        return NULLOBJECT;
    }

    RexxBufferObject obj = context->NewBuffer(sizeof(LVITEM));
    context->SetObjectVariable("CSELF", obj);

    LPLVITEM lvi = (LPLVITEM)context->BufferData(obj);
    memset(lvi, 0, sizeof(LVITEM));

    // We check if the user is setting the mask, and what values she sets,
    // first.  This allows use to allocate the needed buffers if the LvItem
    // object is going to be used to retrieve information.
    if ( argumentExists(5) )
    {
        lvi->mask = keyword2lvif(mask);
    }

    if ( argumentExists(1) )
    {
        int32_t index;
        if ( ! context->Int32(_index, &index) || index < 0 )
        {
            wrongRangeException(context->threadContext, 1, 0, INT_MAX, _index);
            return NULLOBJECT;
        }

        lvi->iItem = index;
    }
    else
    {
        lvi->iItem = 0;
    }

    if ( argumentExists(2) )
    {
        if ( ! setLviText(context, lvi, text, 2) )
        {
            return NULLOBJECT;
        }
    }
    else
    {
        // Check if the user has set the LVIF_TEXT flag.
        if ( lvi->mask & LVIF_TEXT )
        {
            // The empty string tells setLviText() to allocate a buffer.
            if ( ! setLviText(context, lvi, "", 2) )
            {
                return NULLOBJECT;
            }
        }
    }

    lvi->mask |= LVIF_IMAGE;
    if ( argumentExists(3) )
    {
        lvi->iImage = imageIndex < I_IMAGENONE ? I_IMAGENONE : imageIndex;
    }
    else
    {
        lvi->iImage = I_IMAGENONE;
    }

    if ( argumentExists(4) )
    {
        setLviUserData(context, lvi, itemData);
    }

    if ( argumentExists(6) )
    {
        lvi->state = keyword2lvis(itemState, false);
        lvi->mask |= LVIF_STATE;
    }

    if ( argumentExists(7) )
    {
        // The stateMask uses the exact same flags as the item state, and the
        // mask member does not need to be set for this.
        lvi->stateMask = keyword2lvis(itemStateMask, true);
    }

    if ( argumentExists(8) )
    {
        lvi->iIndent = indent;
        lvi->mask |= LVIF_INDENT;
    }

    if ( argumentExists(9) )
    {
        setLviGroupID(context, lvi, groupID);
    }
    else
    {
        if ( ComCtl32Version >= COMCTL32_6_0 )
        {
            lvi->iGroupId = I_GROUPIDNONE;
            lvi->mask |= LVIF_GROUPID;
        }
    }

    if ( argumentExists(10) )
    {
        setLviColumns(context, lvi, columns, 10);
    }
    else
    {
        // Check if the user has set the LVIF_COLUMNS flag.
        if ( lvi->mask & LVIF_COLUMNS )
        {
            if ( ! allocLviColumns(context, lvi) )
            {
                return NULLOBJECT;
            }
        }
    }

    // We want the user to be able to create the LvItem object to recieve data.
    // So we check if the user has set the LVIF_COLFMT flag.
    if ( lvi->mask & LVIF_COLFMT )
    {
        if ( ! allocLviColFmt(context, lvi) )
        {
            return NULLOBJECT;
        }
    }

    return NULLOBJECT;
}

/** LvItem::columns                [attribute]
 */
RexxMethod1(RexxArrayObject, lvi_columns, CSELF, pLVI)
{
    return getLviColumns(context, (LPLVITEM)pLVI);
}
RexxMethod2(RexxObjectPtr, lvi_setColumns, RexxArrayObject, _columns, CSELF, pLVI)
{
    return setLviColumns(context, (LPLVITEM)pLVI, _columns, 1);
}

/** LvItem::columnFormats          [attribute]
 */
RexxMethod1(RexxArrayObject, lvi_columnFormats, CSELF, pLVI)
{
    return getLviColumnFormats(context, (LPLVITEM)pLVI);
}
RexxMethod2(RexxObjectPtr, lvi_setColumnFormats, RexxArrayObject, _formats, CSELF, pLVI)
{
    return setLviColumnFormats(context, (LPLVITEM)pLVI, _formats, 1);
}

/** LvItem::groupID                [attribute]
 */
RexxMethod1(int32_t, lvi_groupID, CSELF, pLVI)
{
    return getLviGroupID(context, (LPLVITEM)pLVI);
}
RexxMethod2(RexxObjectPtr, lvi_setGroupID, int32_t, id, CSELF, pLVI)
{
    return setLviGroupID(context, (LPLVITEM)pLVI, id);
}

/** LvItem::groupIndex             [attribute]
 */
RexxMethod1(int32_t, lvi_groupIndex, CSELF, pLVI)
{
    if ( ! requiredOS(context, "LvItem::groupIndex", "Vista", Vista_OS) )
    {
        return 0;
    }
    return ((LPLVITEM)pLVI)->iGroup;
}
RexxMethod2(RexxObjectPtr, lvi_setGroupIndex, int32_t, id, CSELF, pLVI)
{
    if ( ! requiredOS(context, "LvItem::groupIndex", "Vista", Vista_OS) )
    {
        return 0;
    }
    ((LPLVITEM)pLVI)->iGroup  = id;
    return NULLOBJECT;
}

/** LvItem::imageIndex             [attribute]
 */
RexxMethod1(int32_t, lvi_imageIndex, CSELF, pLVI)
{
    return ((LPLVITEM)pLVI)->iImage;
}
RexxMethod2(RexxObjectPtr, lvi_setImageIndex, int32_t, imageIndex, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->iImage  = imageIndex < I_IMAGENONE ? I_IMAGENONE : imageIndex;
    ((LPLVITEM)pLVI)->mask   |= LVIF_IMAGE;
    return NULLOBJECT;
}

/** LvItem::indent                 [attribute]
 */
RexxMethod1(int32_t, lvi_indent, CSELF, pLVI)
{
    return ((LPLVITEM)pLVI)->iIndent;
}
RexxMethod2(RexxObjectPtr, lvi_setIndent, int32_t, indent, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->iIndent  = indent;
    ((LPLVITEM)pLVI)->mask    |= LVIF_INDENT;
    return NULLOBJECT;
}

/** LvItem::index                  [attribute]
 */
RexxMethod1(int32_t, lvi_index, CSELF, pLVI)
{
    return ((LPLVITEM)pLVI)->iItem;
}
RexxMethod2(RexxObjectPtr, lvi_setIndex, int32_t, index, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->iItem = index;
    return NULLOBJECT;
}

/** LvItem::itemData               [attribute]
 */
RexxMethod1(RexxObjectPtr, lvi_itemData, CSELF, pLVI)
{
    return getLviUserData((LPLVITEM)pLVI);
}
RexxMethod2(RexxObjectPtr, lvi_setItemData, RexxObjectPtr, userData, CSELF, pLVI)
{
    return setLviUserData(context, (LPLVITEM)pLVI, userData);
}

/** LvItem::itemState              [attribute]
 */
RexxMethod1(RexxStringObject, lvi_itemState, CSELF, pLVI)
{
    return lvis2keyword(context, ((LPLVITEM)pLVI)->state);
}
RexxMethod2(RexxObjectPtr, lvi_setItemState, CSTRING, itemState, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->state  = keyword2lvis(itemState, false);
    ((LPLVITEM)pLVI)->mask  |= LVIF_STATE;
    return NULLOBJECT;
}

/** LvItem::itemStateMask          [attribute]
 */
RexxMethod1(RexxStringObject, lvi_itemStateMask, CSELF, pLVI)
{
    return lvis2keyword(context, ((LPLVITEM)pLVI)->stateMask);
}
RexxMethod2(RexxObjectPtr, lvi_setItemStateMask, CSTRING, itemStateMask, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->stateMask = keyword2lvis(itemStateMask, true);
    return NULLOBJECT;
}

/** LvItem::mask                   [attribute]
 */
RexxMethod1(RexxStringObject, lvi_mask, CSELF, pLVI)
{
    return lvif2keyword(context, ((LPLVITEM)pLVI)->mask);
}
RexxMethod2(RexxObjectPtr, lvi_setMask, CSTRING, mask, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->mask = keyword2lvif(mask);
    return NULLOBJECT;
}

/** LvItem::overlayImageIndex      [attribute]
 */
RexxMethod1(uint32_t, lvi_overlayImageIndex, CSELF, pLVI)
{
    return (LVIS_OVERLAYMASK & ((LPLVITEM)pLVI)->state) >> 8;
}
RexxMethod2(RexxObjectPtr, lvi_setOverlayImageIndex, uint16_t, index, CSELF, pLVI)
{
    if ( index > 15 )
    {
        wrongRangeException(context, 1, 0, 15, index);
    }
    else
    {
        ((LPLVITEM)pLVI)->state     |= INDEXTOOVERLAYMASK(index);
        ((LPLVITEM)pLVI)->stateMask |= LVIS_OVERLAYMASK;
    }
    return NULLOBJECT;
}

/** LvItem::stateImageIndex        [attribute]
 */
RexxMethod1(uint16_t, lvi_stateImageIndex, CSELF, pLVI)
{
    return (LVIS_STATEIMAGEMASK & ((LPLVITEM)pLVI)->state) >> 12;
}
RexxMethod2(RexxObjectPtr, lvi_setStateImageIndex, uint16_t, index, CSELF, pLVI)
{
    if ( index > 15 )
    {
        wrongRangeException(context, 1, 0, 15, index);
    }
    else
    {
        ((LPLVITEM)pLVI)->state     |= INDEXTOSTATEIMAGEMASK(index);
        ((LPLVITEM)pLVI)->stateMask |= LVIS_STATEIMAGEMASK;
    }
    return NULLOBJECT;
}

/** LvItem::text                   [attribute]
 */
RexxMethod1(RexxObjectPtr, lvi_text, CSELF, pLVI)
{
    return getLviText(context, (LPLVITEM)pLVI);
}
RexxMethod2(RexxObjectPtr, lvi_setText, CSTRING, text, CSELF, pLVI)
{
    setLviText(context, (LPLVITEM)pLVI, text, 1);
    return NULLOBJECT;
}


/**
 *  Methods for the .LvSubItem class.
 */
#define LVSUBITEM_CLASS            "LvSubItem"


uint32_t keyword2lvifSub(CSTRING flags)
{
    uint32_t val = 0;

    if ( StrCmpI(flags, "ALL") == 0 )
    {
        val = LVIF_IMAGE | LVIF_NORECOMPUTE | LVIF_TEXT;
        return val;
    }

    if ( StrStrI(flags, "IMAGE")       != NULL ) val |= LVIF_IMAGE;
    if ( StrStrI(flags, "NORECOMPUTE") != NULL ) val |= LVIF_NORECOMPUTE;
    if ( StrStrI(flags, "TEXT")        != NULL ) val |= LVIF_TEXT;

    return val;
}

 RexxStringObject lvifSub2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & LVIF_IMAGE) strcat(buf, "IMAGE ");
    if ( flags & LVIF_TEXT ) strcat(buf, "TEXT ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}


/** LvSubItem::init()      [Class]
 *
 */
RexxMethod1(RexxObjectPtr, lvsi_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, LVSUBITEM_CLASS) )
    {
        TheLvSubItemClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheLvSubItemClass);
    }
    return NULLOBJECT;
}


/** LvSubItem::uninit()
 *
 */
RexxMethod1(RexxObjectPtr, lvsi_unInit, CSELF, pCSelf)
{
#if 0
    printf("In lvsi_unInit() pCSelf=%p\n", pCSelf);
#endif

    if ( pCSelf != NULLOBJECT )
    {
        LPLVITEM pLVI = (LPLVITEM)pCSelf;

#if 0
    printf("In lvsi_unInit() pszText=%p\n", pLVI->pszText);
#endif
        safeLocalFree(pLVI->pszText);
    }
    return NULLOBJECT;
}


/** LvSubItem::init()
 *
 *
 *
 *  @remarks  We check if the user is setting the mask, and what values he sets,
 *            first. This allows use to allocate the needed text buffer if the
 *            LvSubItem object is going to be used to retrieve information.
 *
 *            We have one problem.  If the user is constucting this LvSubItem to
 *            receive information we would like to not set any attriubtes, allow
 *            them to be filled in as specified by the mask.  But, if the user
 *            is constructing this LvSubItem to set the item and just omits the
 *            imageIndex arg, we would like to set imageIndex to I_IMAGENONE.
 *
 *            But, we don't know what the user's intent is ... so, for now we
 *            are setting the image index to I_IMAGENONE if the imageIndex arg
 *            is omitted.
 */
RexxMethod5(RexxObjectPtr, lvsi_init, RexxObjectPtr, _item, uint32_t, subItem, OPTIONAL_CSTRING, text,
             OPTIONAL_int32_t, imageIndex, OPTIONAL_CSTRING, mask)
{
    if ( context->IsBuffer(_item) )
    {
        context->SetObjectVariable("CSELF", _item);
        return NULLOBJECT;
    }

    RexxMethodContext *c = context;
    RexxBufferObject obj = context->NewBuffer(sizeof(LVITEM));
    context->SetObjectVariable("CSELF", obj);

    LPLVITEM lvi = (LPLVITEM)context->BufferData(obj);
    memset(lvi, 0, sizeof(LVITEM));

    uint32_t item;
    if ( ! c->UnsignedInt32(_item, &item) )
    {
        wrongRangeException(context->threadContext, 1, 0, INT_MAX, _item);
        return NULLOBJECT;
    }
    if ( subItem == 0 )
    {
        wrongRangeException(context->threadContext, 2, 1, INT_MAX, subItem);
        return NULLOBJECT;
    }

    lvi->iItem    = item;
    lvi->iSubItem = subItem;

    if ( argumentExists(5) )
    {
        lvi->mask = keyword2lvifSub(mask);
    }

    if ( argumentExists(3) )
    {
        if ( ! setLviText(context, lvi, text, 3) )
        {
            return NULLOBJECT;
        }
    }
    else
    {
        // Check if the user has set the LVIF_TEXT flag.
        if ( lvi->mask & LVIF_TEXT )
        {
            // The empty string tells setLviText() to allocate a buffer.
            if ( ! setLviText(context, lvi, "", 3) )
            {
                return NULLOBJECT;
            }
        }
    }

    lvi->mask |= LVIF_IMAGE;
    if ( argumentExists(4) )
    {
        lvi->iImage = imageIndex < I_IMAGENONE ? I_IMAGENONE : imageIndex;
    }
    else
    {
        lvi->iImage = I_IMAGENONE;
    }

    return NULLOBJECT;
}

/** LvSubItem::item                [attribute]
 */
RexxMethod1(int32_t, lvsi_item, CSELF, pLVI)
{
    return ((LPLVITEM)pLVI)->iItem;
}
RexxMethod2(RexxObjectPtr, lvsi_setItem, int32_t, item, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->iItem = item;
    return NULLOBJECT;
}

/** LvSubItem::subItem             [attribute]
 */
RexxMethod1(int32_t, lvsi_subItem, CSELF, pLVI)
{
    return ((LPLVITEM)pLVI)->iSubItem;
}
RexxMethod2(RexxObjectPtr, lvsi_setSubItem, int32_t, subItem, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->iSubItem = subItem;
    return NULLOBJECT;
}

/** LvSubItem::text                [attribute]
 */
RexxMethod1(RexxObjectPtr, lvsi_text, CSELF, pLVI)
{
    return getLviText(context, (LPLVITEM)pLVI);
}
RexxMethod2(RexxObjectPtr, lvsi_setText, CSTRING, text, CSELF, pLVI)
{
    setLviText(context, (LPLVITEM)pLVI, text, 1);
    return NULLOBJECT;
}

/** LvSubItem::imageIndex          [attribute]
 */
RexxMethod1(int32_t, lvsi_imageIndex, CSELF, pLVI)
{
    return ((LPLVITEM)pLVI)->iImage;
}
RexxMethod2(RexxObjectPtr, lvsi_setImageIndex, int32_t, imageIndex, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->iImage  = imageIndex < I_IMAGENONE ? I_IMAGENONE : imageIndex;
    ((LPLVITEM)pLVI)->mask   |= LVIF_IMAGE;
    return NULLOBJECT;
}

/** LvSubItem::mask                [attribute]
 */
RexxMethod1(RexxStringObject, lvsi_mask, CSELF, pLVI)
{
    return lvifSub2keyword(context, ((LPLVITEM)pLVI)->mask);
}
RexxMethod2(RexxObjectPtr, lvsi_setMask, CSTRING, mask, CSELF, pLVI)
{
    ((LPLVITEM)pLVI)->mask = keyword2lvifSub(mask);
    return NULLOBJECT;
}


/**
 *  Methods for the .LvFullRow class.
 */
#define LVFULLROW_CLASS                "LvFullRow"

#define LVFULLROW_BAGOFITEMS_ATTRIBUTE "LVFULLROW_BAGOFITEMS"

inline void lvfrStoreItem(RexxMethodContext *c, pCLvFullRow pclvfr, uint32_t index)
{
    c->SendMessage1(pclvfr->bagOfItems, "PUT", pclvfr->rxSubItems[index]);
}

inline RexxObjectPtr lvfrUnStoreItem(RexxMethodContext *c, pCLvFullRow pclvfr, RexxObjectPtr item)
{
    return c->SendMessage1(pclvfr->bagOfItems, "REMOVE", item);
}

inline void adjustSubItemIndexes(pCLvFullRow pclvfr)
{
    for ( uint32_t i = 1; i <= pclvfr->subItemCount; i++ )
    {
        pclvfr->subItems[i]->iSubItem = i;
    }
}

/**
 * Checks that a subitem column being added or removed from a full row stuct is
 * valid.
 *
 * A column that does not exist already in the struct can not be removed.  A
 * column being added or inserted can not result is a sparse array.
 *
 *
 * @param pclvfr
 * @param colIndex
 *
 * @return bool
 */
static bool validColumnIndex(pCLvFullRow pclvfr, bool removed, uint32_t colIndex)
{
    if ( removed )
    {
        if ( colIndex > pclvfr->subItemCount )
        {
            return false;
        }
    }
    else
    {
        if ( colIndex > pclvfr->subItemCount + 1 )
        {
            return false;
        }
    }

    return true;
}

/**
 *  For a LvFullRow object, we put the Rexx item and subitems in a Rexx bag and
 *  save the bag as an object variable to prevent GC of the items.
 *
 * @param c
 * @param pclvfr
 */
static void lvfrStoreItems(RexxMethodContext *c, pCLvFullRow pclvfr)
{
    RexxObjectPtr bag = rxNewBuiltinObject(c, "BAG");

    c->SetObjectVariable(LVFULLROW_BAGOFITEMS_ATTRIBUTE, bag);

    for ( size_t i = 0; i <= pclvfr->subItemCount; i++ )
    {
        c->SendMessage1(bag, "PUT", pclvfr->rxSubItems[i]);
    }
    pclvfr->bagOfItems = bag;
}


/**
 * Expands the rxItems and the subItems arrays in a CLvFullRow struct by
 * doubling the array size.
 *
 * @param c
 * @param pclvfr
 *
 * @return True on success, false on a memory allocation error.
 */
static bool expandSubItems(RexxMethodContext *c, pCLvFullRow pclvfr)
{
    uint32_t s = pclvfr->size * 2;

    RexxObjectPtr *rxItems = (RexxObjectPtr *)LocalAlloc(LPTR, s * sizeof(RexxObjectPtr *));
    LPLVITEM *subItems     = (LPLVITEM *)LocalAlloc(LPTR, s * sizeof(LPLVITEM *));

    if ( subItems == NULL || rxItems == NULL )
    {
        safeLocalFree(subItems);
        safeLocalFree(rxItems);

        outOfMemoryException(c->threadContext);
        return false;
    }

    memcpy(subItems, pclvfr->subItems, pclvfr->size * sizeof(LPLVITEM *));
    memcpy(rxItems, pclvfr->rxSubItems, pclvfr->size * sizeof(RexxObjectPtr *));

    LocalFree(pclvfr->subItems);
    LocalFree(pclvfr->rxSubItems);

    pclvfr->subItems   = subItems;
    pclvfr->rxSubItems = rxItems;
    pclvfr->size       = s;

    return true;
}

/**
 * Removes a subitem column from the full row struct.
 *
 * @param c
 * @param pclvfr
 * @param index
 *
 * @return RexxObjectPtr
 *
 * @assumes   That updateFullRowIndexes() has already been called to ensure that
 *            the item indexes in the current row are correct.
 *
 *            That index is valid for the full row, i.e., it must be an existing
 *            column in the full row.
 */
static RexxObjectPtr removeSubItemFromRow(RexxMethodContext *c, pCLvFullRow pclvfr, uint32_t index)
{
    RexxObjectPtr subItem = pclvfr->rxSubItems[index];

    if ( index < pclvfr->subItemCount )
    {
        size_t count = (pclvfr->subItemCount - index) * sizeof(void *);

        memmove(&pclvfr->rxSubItems[index], &pclvfr->rxSubItems[index + 1], count);
        memmove(&pclvfr->subItems[index],   &pclvfr->subItems[index + 1],   count);
    }

    pclvfr->subItemCount--;

    adjustSubItemIndexes(pclvfr);

    return lvfrUnStoreItem(c, pclvfr, subItem);
}


/**
 * Inserts the specified LvSubItem into a LvFullRow object.
 *
 * @param c
 * @param lvSubitem
 * @param pclvfr
 * @param index
 *
 * @return true on success, false on error.
 *
 * @assumes updateFullRowIndexes() has already been called to ensure that the
 *          item indexes in the current row are correct.
 *
 *          That the index is within the bounds of the current subitems array,
 *          plus one. I.e., the subitem can be appended to the current array,
 *          but it can not be added such that the array becomes sparse.
 *
 *          If the current subitem array has 4 subitems, the subitem can be
 *          'inserted' as the 5th subitem, but not as the 6th or greate subitem.
 *
 * @remarks  The subitem arrays are expanded if needed.  The subitem indexes of
 *           all the subitems are adjusted for the insertion.
 */
static RexxObjectPtr insertSubItemIntoRow(RexxMethodContext *c, RexxObjectPtr lvSubitem, pCLvFullRow pclvfr, uint32_t index)
{
    uint32_t i = pclvfr->subItemCount + 1;

    // Be sure we have room in the arrays for the new column.
    if ( i >= pclvfr->size )
    {
        if ( ! expandSubItems(c, pclvfr) )
        {
            return TheFalseObj;
        }
    }

    // If the new column is not being appended, then we need to shift the
    // exising columns up one slot.
    if ( index < i )
    {
        size_t count = (pclvfr->subItemCount - index + 1) * sizeof(void *);

        memmove(&pclvfr->rxSubItems[index + 1], &pclvfr->rxSubItems[index], count);
        memmove(&pclvfr->subItems[index + 1],   &pclvfr->subItems[index],   count);
    }

    // Add the new subitem at its column
    pclvfr->subItemCount           = i;
    pclvfr->rxSubItems[index]      = lvSubitem;
    pclvfr->subItems[index]        = (LPLVITEM)c->ObjectToCSelf(lvSubitem);
    pclvfr->subItems[index]->iItem = pclvfr->subItems[0]->iItem;

    // Adjust the subitem indexes from the new subitem to the end of columns
    for ( uint32_t j = index; j <= i; j++)
    {
        pclvfr->subItems[j]->iSubItem = j;
    }

    // Protect the new subitem from GC.
    lvfrStoreItem(c, pclvfr, index);

    return TheTrueObj;
}


/** LvFullRow::init()        [Class]
 *
 */
RexxMethod1(RexxObjectPtr, lvfr_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, LVFULLROW_CLASS) )
    {
        TheLvFullRowClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheLvFullRowClass);
    }
    return NULLOBJECT;
}


/** LvFullRow::fromArray()   [Class]
 *
 *  Constructs a full row object from an array of values.  The string value of
 *  each item in the array is used as the text for the item or subitem(s).
 *
 *  The string value of the first item in the array will be the text for the
 *  list-view item.  The string value of the second item in the array will be
 *  the text for the first subitem of the list-view item.  The string value of
 *  the third item in the array will be the text for the second subitem of the
 *  list-view item, etc..
 *
 *  The array can not be sparse.
 *
 *  @param data  [required]  The array of objects whose string values are to be
 *               used for the text of the item and subitems.
 *
 *  @param itemIndex  [optional]  The item index to use for the full row.  If
 *                    omitted, 0 is used for the item index.
 *
 *  @param strIfNil   [opitonal]  String to use for the item or subitem text, if
 *                    the object in the array is the .nil object.
 *
 *  @param useForSorting  [optional]  If the constructed full row object should
 *                        be assigned as the item data for the list-view item.
 *                        This allows the ooDialog framework to sort the
 *                        list-view item internally.
 *
 *  @return  A newly instantiated LvFullRow object on success, the .nil objec on
 *           error.
 *
 *  @notes  The returned full row object is intended to be used to insert an
 *          item into a list-view.  Other uses of the full row object may cause
 *          unpredictable results.  It is the responsibility of the programmer
 *          to determine if the full row object is suitable for some other use.
 *          The programmer should consider that only the text attribute of the
 *          full row item and its subitems is set.
 *
 *          Recall that if the full row object is used in either the addFullRow
 *          or prependFullRow methods, the item index of the full row object is
 *          ignored.  For the insertFullRow method, if the item index is 0, the
 *          method will work fine, the full row will be inserted as the first
 *          item in the list-view.  Therefore the optional <itemIndex> argument
 *          is never needed, and, depending on the use of the full row object,
 *          may be a waste of time.
 */
RexxMethod5(RexxObjectPtr, lvfr_fromArray_cls, RexxArrayObject, data, OPTIONAL_uint32_t, itemIndex,
            OPTIONAL_CSTRING, nullStr, OPTIONAL_logical_t, useForSorting, OSELF, self)
{
    RexxObjectPtr result = TheNilObj;

    size_t cCols = context->ArrayItems(data);
    if ( cCols == 0 )
    {
        emptyArrayException(context->threadContext, 1);
        return result;
    }

    // Allocate and set up the CLvFullRow struct using Rexx memory
    RexxBufferObject buf = context->NewBuffer(sizeof(CLvFullRow));
    if ( buf == NULLOBJECT )
    {
        outOfMemoryException(context->threadContext);
        return result;
    }

    pCLvFullRow pclvfr = (pCLvFullRow)context->BufferData(buf);
    memset(pclvfr, 0, sizeof(CLvFullRow));

    size_t size = LVFULLROW_DEF_SUBITEMS;
    if ( cCols >= size )
    {
        size = 2 * cCols;
    }

    pclvfr->subItems   = (LPLVITEM *)LocalAlloc(LPTR, size * sizeof(LPLVITEM *));
    pclvfr->rxSubItems = (RexxObjectPtr *)LocalAlloc(LPTR, size * sizeof(RexxObjectPtr *));

    if ( pclvfr->subItems == NULL || pclvfr->rxSubItems == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto err_out;
    }

    CSTRING itemText      = NULL;
    bool    substitueText =  argumentOmitted(3) ? false : true;

    // Create a LvItem
    RexxObjectPtr arrayItem = context->ArrayAt(data, 1);
    if ( arrayItem == NULLOBJECT )
    {
        sparseArrayException(context->threadContext, 1, 1);
        goto err_out;
    }

    if ( substitueText )
    {
        itemText = arrayItem == TheNilObj ? nullStr : context->ObjectToStringValue(arrayItem);
    }
    else
    {
        itemText = context->ObjectToStringValue(arrayItem);
    }

    RexxObjectPtr rxItem = simpleNewLvItem(context, itemText);
    if ( rxItem == TheNilObj )
    {
        goto err_out;
    }

    LPLVITEM lvi = (LPLVITEM)context->ObjectToCSelf(rxItem);

    lvi->iItem = itemIndex;

    pclvfr->magic         = LVFULLROW_MAGIC;
    pclvfr->size          = (uint32_t)size;
    pclvfr->subItems[0]   = lvi;
    pclvfr->rxSubItems[0] = rxItem;

    if ( useForSorting )
    {
        pclvfr->subItems[0]->lParam = (LPARAM)pclvfr;
        pclvfr->subItems[0]->mask  |= LVIF_PARAM;
    }

    // Create LvSubItem objects for each column and add them to the struct.
    for ( size_t i = 1; i < cCols; i++ )
    {
        // Create a LvSubItem
        arrayItem = context->ArrayAt(data, i + 1);
        if ( arrayItem == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, i + 1);
            goto err_out;
        }

        if ( substitueText )
        {
            itemText = arrayItem == TheNilObj ? nullStr : context->ObjectToStringValue(arrayItem);
        }
        else
        {
            itemText = context->ObjectToStringValue(arrayItem);
        }

        RexxObjectPtr rxSubItem = simpleNewLvSubitem(context, itemText, (uint32_t)i);
        if ( rxSubItem == TheNilObj )
        {
            goto err_out;
        }

        lvi = (LPLVITEM)context->ObjectToCSelf(rxSubItem);

        lvi->iItem = itemIndex;

        pclvfr->subItems[i]   = lvi;
        pclvfr->rxSubItems[i] = rxSubItem;

        pclvfr->subItemCount++;
    }

    // Now instantiate the LvFullRow Rexx object.
    result = context->SendMessage1(TheLvFullRowClass, "NEW", buf);
    if ( result != NULLOBJECT )
    {
        return result;
    }

err_out:
    safeLocalFree(pclvfr->subItems);
    safeLocalFree(pclvfr->rxSubItems);
    pclvfr->subItems = NULL;
    pclvfr->rxSubItems = NULL;
    return TheNilObj;
}


/** LvFullRow::uninit()
 *
 */
RexxMethod1(RexxObjectPtr, lvfr_unInit, CSELF, pCSelf)
{
#if 0
    printf("In lvfr_unInit() pCSelf=%p\n", pCSelf);
#endif

    if ( pCSelf != NULL )
    {
        pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;

#if 0
    printf("In lvfr_unInit() subItems=%p\n", pclvfr->subItems);
#endif
        safeLocalFree(pclvfr->subItems);
        safeLocalFree(pclvfr->rxSubItems);
    }
    return NULLOBJECT;
}


/** LvFullRow::init()
 *
 *  Instantiates a new LvFullRow object.
 *
 *  Full row objects can be used to specify, or receive, all the attributes of a
 *  list-view item, and all subitems of that list-view item.
 *
 *  @param   lvItem      [required] Must be a LvItem object.
 *
 *  @param  lvSubItem    [optional] A LvSubItem object representing the first
 *                                  column.
 *  @param  lvSubItem2   [optional] A LvSubItem object representing the second
 *                                  column.
 *  ...
 *
 *  @param  lvSubItemN   [optional] A LvSubItem object representing the nth
 *                                  column.  This is the last arg.
 *
 *                                  The last arg can also be true or false to
 *                                  indicate the user wants to use the ooDialog
 *                                  internal sorting of list view items.
 *
 *  @notes  We allow users to add or remove subitems.  So, we start out with a
 *          default number of items in CLvFullRow.subItems and
 *          CLvFullRow.rxSubItems, and then double the size of the arrays each
 *          time it is too small.  There seems to be no limit to the number of
 *          columns in a list view, so we don't set a limit.
 *
 *          We sometimes create a LvFullRow object within the C / C++ code.  We
 *          do this by allocating a Rexx buffer object for the CLvFullRow struct
 *          and filling it in.  We then send the buffer object to the new
 *          method.  When we detect this, we need to set the buffer as the
 *          CSelf, set the rexxSelf field in the CLvFullRow struct and store the
 *          Rexx item and subitems in the context bag of this object.
 */
RexxMethod2(RexxObjectPtr, lvfr_init, ARGLIST, args, OSELF, self)
{
    pCLvFullRow pclvfr;
    LPLVITEM    lvi;
    size_t      argCount = context->ArraySize(args);

    if ( argCount == 0 )
    {
        missingArgException(context, 1);
        goto err_out;
    }

    for ( size_t i = 1; i <= argCount; i++ )
    {
        RexxObjectPtr obj = context->ArrayAt(args, i);
        if ( obj == NULLOBJECT )
        {
            missingArgException(context, i);
            goto err_out;
        }

        if ( i == 1 )
        {
            if ( context->IsBuffer(obj) && isLvFullRowStruct(context->BufferData((RexxBufferObject)obj)) )
            {
                context->SetObjectVariable("CSELF", obj);

                pclvfr = (pCLvFullRow)context->BufferData((RexxBufferObject)obj);
                pclvfr->rexxSelf = self;

                lvfrStoreItems(context, pclvfr);

                goto done;
            }

            if ( ! context->IsOfType(obj, "LVITEM") )
            {
                wrongClassException(context->threadContext, 1, "LvItem");
                goto err_out;
            }

            RexxBufferObject buf = context->NewBuffer(sizeof(CLvFullRow));
            context->SetObjectVariable("CSELF", buf);

            pclvfr = (pCLvFullRow)context->BufferData(buf);
            memset(pclvfr, 0, sizeof(CLvFullRow));

            size_t size = LVFULLROW_DEF_SUBITEMS;
            if ( argCount >= size )
            {
                size = 2 * argCount;
            }

            pclvfr->subItems   = (LPLVITEM *)LocalAlloc(LPTR, size * sizeof(LPLVITEM *));
            pclvfr->rxSubItems = (RexxObjectPtr *)LocalAlloc(LPTR, size * sizeof(RexxObjectPtr *));

            if ( pclvfr->subItems == NULL || pclvfr->rxSubItems == NULL )
            {
                safeLocalFree(pclvfr->subItems);
                safeLocalFree(pclvfr->rxSubItems);

                outOfMemoryException(context->threadContext);
                goto err_out;
            }

            lvi = (LPLVITEM)context->ObjectToCSelf(obj);

            pclvfr->rexxSelf      = self;
            pclvfr->magic         = LVFULLROW_MAGIC;
            pclvfr->id            = LVFULLROW_NOID;
            pclvfr->size          = (uint32_t)size;
            pclvfr->subItems[0]   = lvi;
            pclvfr->rxSubItems[0] = obj;

            // No subitems yet, subItemCount is 0, which is correct.

            continue;
        }

        // All objects after the first one have to be a LvSubItem object, except
        // the last one can be true or false to enable internal sorting.
        if ( i == argCount )
        {
            int32_t logical = getLogical(context->threadContext, obj);
            if ( logical != -1 )
            {
                if ( logical == 1 )
                {
                    pclvfr->subItems[0]->lParam = (LPARAM)pclvfr;
                    pclvfr->subItems[0]->mask  |= LVIF_PARAM;
                }
                goto done;
            }
        }

        if ( ! context->IsOfType(obj, "LVSUBITEM") )
        {
            wrongClassException(context->threadContext, i, "LvSubItem");
            goto err_out;
        }

        lvi = (LPLVITEM)context->ObjectToCSelf(obj);

        pclvfr->subItems[i - 1]   = lvi;
        pclvfr->rxSubItems[i - 1] = obj;
        pclvfr->subItemCount++;
    }

done:
    lvfrStoreItems(context, pclvfr);

err_out:
    return NULLOBJECT;
}

/** LvFullRow::userData             [attribute]
 */
RexxMethod1(RexxObjectPtr, lvfr_userData, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;
    if ( pclvfr->userData != NULLOBJECT )
    {
        return pclvfr->userData;
    }
    return TheNilObj;
}
RexxMethod2(RexxObjectPtr, lvfr_setUserData, RexxObjectPtr, obj, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;

    if ( obj == TheNilObj )
    {
        pclvfr->userData = NULL;
        context->SetObjectVariable("USERDATA", NULLOBJECT);
    }
    else
    {
        pclvfr->userData = obj;
        context->SetObjectVariable("USERDATA", obj);
    }

    return NULLOBJECT;
}

/** LvFullRow::addSubitem()
 *
 *  Adds a subitem to this full row.  Subitems are always added as the last
 *  subitem in the row.
 *
 *  @param  subitem  The subitem to add.
 *
 *  @return  Returns the index of the added subitem, or 0 on error.
 */
RexxMethod2(uint32_t, lvfr_addSubitem, RexxObjectPtr, subitem, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;

    if ( ! context->IsOfType(subitem, "LVSUBITEM") )
    {
        wrongClassException(context->threadContext, 1, "LvSubItem");
        return 0;
    }

    updateFullRowIndexes(pclvfr);

    uint32_t newColumn = pclvfr->subItemCount + 1;
    if ( ! insertSubItemIntoRow(context, subitem, pclvfr, newColumn) )
    {
        return 0;
    }

    return newColumn;
}

/** LvFullRow::insertSubitem()
 *
 *  Inserts a new subitem into this full row and adjusts the subitem indexes for
 *  all existing subitems when needed.
 *
 *  @param  subitem   The subitem to insert.
 *
 *  @param  colIndex  The insertion index for the subitem.
 *
 *  @return  True on success, false on error.
 */
RexxMethod3(RexxObjectPtr, lvfr_insertSubitem, RexxObjectPtr, subitem, uint32_t, colIndex, CSELF, pCSelf)
{
    RexxObjectPtr result = TheFalseObj;

    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;

    if ( ! context->IsOfType(subitem, "LVSUBITEM") )
    {
        wrongClassException(context->threadContext, 1, "LvSubItem");
        goto done_out;
    }

    if ( colIndex == 0 )
    {
        userDefinedMsgException(context->threadContext, 2, "can not be column 0");
        goto done_out;
    }

    if ( colIndex > pclvfr->subItemCount + 1 )
    {
        wrongRangeException(context, 2, 1, pclvfr->subItemCount + 1, colIndex);
        goto done_out;
    }

    updateFullRowIndexes(pclvfr);

    if ( insertSubItemIntoRow(context, subitem, pclvfr, colIndex) )
    {
        result = TheTrueObj;
    }

done_out:
    return result;
}

/** LvFullRow::item()
 *
 *  Returns the item object of this full row.
 *
 */
RexxMethod1(RexxObjectPtr, lvfr_item, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;
    updateFullRowIndexes(pclvfr);
    return pclvfr->rxSubItems[0];
}

/** LvFullRow::removeSubItem()
 *
 *  Removes the specified subitem from this full row.
 *
 *  @param  index  The index of the subitem to remove.
 *
 *  @return  Returns the the removed subitem, or .nil on error.
 *
 *  @note If the index is the last subitem, we just need to decrement
 *        subItemCount.  Otherwise we need to decrement and shift the following
 *        subItems down by 1.
 */
RexxMethod2(RexxObjectPtr, lvfr_removeSubitem, uint32_t, index, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;

    if ( index < 1 || index > pclvfr->subItemCount )
    {
        wrongRangeException(context->threadContext, 1, 1, pclvfr->subItemCount, index);
        return TheNilObj;
    }

    updateFullRowIndexes(pclvfr);
    return removeSubItemFromRow(context, pclvfr, index);
}

/** LvFullRow::subItem()
 *
 *  Returns the subitem specified by <index> of this full row, or .nil if the
 *  subitem <index> is not valid.
 *
 */
RexxMethod2(RexxObjectPtr, lvfr_subitem, uint32_t, index, CSELF, pCSelf)
{
    pCLvFullRow pclvfr = (pCLvFullRow)pCSelf;

    if ( index == 0 || index > pclvfr->subItemCount )
    {
        return TheNilObj;
    }

    updateFullRowIndexes(pclvfr);
    return pclvfr->rxSubItems[index];
}

/** LvFullRow::subItems()
 *
 *  Returns the number of subitems in this full row.
 *
 */
RexxMethod1(uint32_t, lvfr_subitems, CSELF, pCSelf)
{
    return ((pCLvFullRow)pCSelf)->subItemCount;
}


/**
 *  Methods for the .LvCustomDrawSimple class.
 */
#define LVCUSTOMDRAWSIMPLE_CLASS                "LvCustomDrawSimple"


/**
 * Handles the processing for the list-view custom draw event for the basic
 * case.  That is, the user wants to change text color or font, for a list-view
 * item, and / or, in report mode, the item and subitems indivicually.
 *
 * @param c
 * @param methodName
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @notes  Typically for a list-view there are a flurry of custom draw events at
 *         one time.  Testing has shown that if there is a condition pending,
 *         things seem to hang.  So, we check for a condition on entry and just
 *         do a CDRF_DODEFAULT immediately if that is the case.
 *
 *         The simple case is to only respond to item prepaint or subitem
 *         prepaint. If the user returns .false from the event handler, this has
 *         the effect of returning CDFR_DODEFAULT to the list-view.
 *
 *         If the user returns .true, we not check the reply value in the
 *         CLvCustomDrawSimple struct, assuming the user has set the value to
 *         what they actually want. The user should use either CDRF_NEWFONT, or
 *         CDRF_NOTIFYSUBITEMDRAW, or (CDRF_NOTIFYSUBITEMDRAW | CDRF_NEFONT.)
 *         When .true is returned the colors are updated in the NMLVCUSTOMDRAW
 *         struct. If hFont in the LvCustomDrawSimple object is not null, the
 *         font is selected into the device context, which has the effect of
 *         changing the font.
 */
MsgReplyType lvSimpleCustomDraw(RexxThreadContext *c, CSTRING methodName, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    LPNMLVCUSTOMDRAW lvcd  = (LPNMLVCUSTOMDRAW)lParam;
    LPARAM           reply = CDRF_DODEFAULT;

    if ( lvcd->nmcd.dwDrawStage == CDDS_PREPAINT )
    {
        reply = CDRF_NOTIFYITEMDRAW;
    }
    else if ( lvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT || lvcd->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM) )
    {
        if ( c->CheckCondition() )
        {
            goto done_out;
        }

        RexxBufferObject lvcdsBuf = c->NewBuffer(sizeof(CLvCustomDrawSimple));
        if ( lvcdsBuf == NULLOBJECT )
        {
            outOfMemoryException(c);
            endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
            goto done_out;
        }

        pCLvCustomDrawSimple pclvcds = (pCLvCustomDrawSimple)c->BufferData(lvcdsBuf);
        memset(pclvcds, 0, sizeof(CLvCustomDrawSimple));

        pclvcds->drawStage = lvcd->nmcd.dwDrawStage;
        pclvcds->item      = lvcd->nmcd.dwItemSpec;
        pclvcds->subItem   = lvcd->iSubItem;
        pclvcds->id        = (uint32_t)((NMHDR *)lParam)->idFrom;
        pclvcds->userData  = lviLParam2UserData(lvcd->nmcd.lItemlParam);

        RexxObjectPtr custDrawSimple = c->SendMessage1(TheLvCustomDrawSimpleClass, "NEW", lvcdsBuf);
        if ( custDrawSimple != NULLOBJECT )
        {
            RexxObjectPtr msgReply = c->SendMessage1(pcpbd->rexxSelf, methodName, custDrawSimple);

            msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
            if ( msgReply == TheTrueObj )
            {
                lvcd->clrText   = pclvcds->clrText;
                lvcd->clrTextBk = pclvcds->clrTextBk;

                if ( pclvcds->hFont != NULL )
                {
                    // An example I've seen deletes the old font.  Doesn't seem
                    // appropriate for ooRexx.  The user would need to save the
                    // list-view font and then add it back in.  Not sure if
                    // there is a resource leak here.
                    HFONT hOldFont = (HFONT)SelectObject(lvcd->nmcd.hdc, pclvcds->hFont);
                }
                reply = pclvcds->reply;
            }
        }
    }

done_out:
    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, (LPARAM)reply);
    return ReplyTrue;
}


/** LvCustomDrawSimple::init()     [class]
 *
 *
 */
RexxMethod1(RexxObjectPtr, lvcds_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, LVCUSTOMDRAWSIMPLE_CLASS) )
    {
        TheLvCustomDrawSimpleClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheLvCustomDrawSimpleClass);
    }
    return NULLOBJECT;
}

/** LvCustomDrawSimple::init()
 *
 */
RexxMethod2(RexxObjectPtr, lvcds_init, RexxObjectPtr, cself, OSELF, self)
{
    if ( context->IsBuffer(cself) )
    {
        context->SetObjectVariable("CSELF", cself);
    }
    else
    {
        baseClassInitializationException(context, "LvCustomDrawSimple");
    }
    return NULLOBJECT;
}

/** LvCustomDrawSimple::clrText    [attribute]
 */
RexxMethod2(uint32_t, lvcds_setClrText, uint32_t, clrText, CSELF, pCSelf)
{
    ((pCLvCustomDrawSimple)pCSelf)->clrText = clrText;
    return NULLOBJECT;
}

/** LvCustomDrawSimple::clrTextBk  [attribute]
 */
RexxMethod2(RexxObjectPtr, lvcds_setClrTextBk, uint32_t, clrTextBk, CSELF, pCSelf)
{
    ((pCLvCustomDrawSimple)pCSelf)->clrTextBk = clrTextBk;
    return NULLOBJECT;
}

/** LvCustomDrawSimple::drawStage  [attribute]
 */
RexxMethod1(uint32_t, lvcds_getDrawStage, CSELF, pCSelf)
{
    return ((pCLvCustomDrawSimple)pCSelf)->drawStage;
}

/** LvCustomDrawSimple::font       [attribute]
 */
RexxMethod2(RexxObjectPtr, lvcds_setFont, POINTERSTRING, font, CSELF, pCSelf)
{
    ((pCLvCustomDrawSimple)pCSelf)->hFont = (HFONT)font;
    return NULLOBJECT;
}

/** LvCustomDrawSimple::id         [attribute]
 */
RexxMethod1(uintptr_t, lvcds_getID, CSELF, pCSelf)
{
    return ((pCLvCustomDrawSimple)pCSelf)->id;
}

/** LvCustomDrawSimple::item       [attribute]
 */
RexxMethod1(uintptr_t, lvcds_getItem, CSELF, pCSelf)
{
    return ((pCLvCustomDrawSimple)pCSelf)->item;
}

/** LvCustomDrawSimple::itemData   [attribute]
 */
RexxMethod1(RexxObjectPtr, lvcds_getItemData, CSELF, pCSelf)
{
    return ((pCLvCustomDrawSimple)pCSelf)->userData;
}

/** LvCustomDrawSimple::reply      [attribute]
 */
RexxMethod2(RexxObjectPtr, lvcds_setReply, uint32_t, reply, CSELF, pCSelf)
{
    ((pCLvCustomDrawSimple)pCSelf)->reply = reply;
    return NULLOBJECT;
}

/** LvCustomDrawSimple::subItem    [attribute]
 */
RexxMethod1(uint32_t, lvcds_getSubItem, CSELF, pCSelf)
{
    return ((pCLvCustomDrawSimple)pCSelf)->subItem;
}



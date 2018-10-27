/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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
 * oodStatusBar.cpp
 *
 * Implementation for the StatusBar dialog control.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <shlwapi.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"
#include "oodMessaging.hpp"
#include "oodShared.hpp"


/**
 * Methods for the StatusBar class.
 */
#define STATUSBAR_CLASS        "StatusBar"

#define STB_ICON_ATTRIBUTE     "STB!!ICONS!!ATTRIBUTE"
#define STB_DRAWTYPE_KEYWORDS "LOWERBOREDERS, NOBORDERS, OWNERDRAW, POPUT, RTLREADING, or NOTABPARSING"

#define STB_MAX_TOOLTIP_LENGTH  511



static uint32_t keyword2sbars(CSTRING flags)
{
    uint32_t val = 0;

    if ( flags != NULL )
    {
        if ( StrStrI(flags, "SIZEGRIP"    ) != NULL ) val |= SBARS_SIZEGRIP    ;
        if ( StrStrI(flags, "TOOLTIPS"    ) != NULL ) val |= SBARS_TOOLTIPS    ;
        if ( StrStrI(flags, "NONE"        ) != NULL ) val  = 0;
    }

    return val;
}

/**
 * Converts a set of SBARS_* StatusBar style flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
static RexxStringObject sbars2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & SBARS_SIZEGRIP     ) strcat(buf, "SIZEGRIP "     );
    if ( flags & SBARS_TOOLTIPS     ) strcat(buf, "TOOLTIPS "     );

    if ( *buf == '\0' )
    {
        strcpy(buf, "nil");
    }
    else
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}


static uint32_t keyword2drawType(RexxMethodContext *c, CSTRING keyword, size_t argPos)
{
    uint32_t val = 0;

    if ( keyword != NULL )
    {
        if (      StrCmpI(keyword, "LOWERBORDERS" ) == 0 ) val = 0                ;
        else if ( StrCmpI(keyword, "NOBORDERS"    ) == 0 ) val = SBT_NOBORDERS    ;
        else if ( StrCmpI(keyword, "OWNERDRAW"    ) == 0 ) val = SBT_OWNERDRAW    ;
        else if ( StrCmpI(keyword, "POPOUT"       ) == 0 ) val = SBT_POPOUT       ;
        else if ( StrCmpI(keyword, "RTLREADING"   ) == 0 ) val = SBT_RTLREADING   ;
        else if ( StrCmpI(keyword, "NOTABPARSING" ) == 0 ) val = SBT_NOTABPARSING ;
        else
        {
            wrongArgKeywordException(c, argPos, STB_DRAWTYPE_KEYWORDS, keyword);
            val = OOD_NO_VALUE;
        }
    }

    return val;
}

static RexxStringObject drawtype2keyword(RexxMethodContext *c, uint32_t flag)
{

    if (      flag == 0                ) return c->String("LOWERBORDERS");
    else if ( flag == SBT_NOBORDERS    ) return c->String("NOBORDERS"   );
    else if ( flag == SBT_OWNERDRAW    ) return c->String("OWNERDRAW"   );
    else if ( flag == SBT_POPOUT       ) return c->String("POPOUT"      );
    else if ( flag == SBT_RTLREADING   ) return c->String("RTLREADING"  );
    else if ( flag == SBT_NOTABPARSING ) return c->String("NOTABPARSING");

    return c->String("UNKNOWN");
}


RexxObjectPtr stbGetIcon(RexxMethodContext *c, uint32_t index)
{
    RexxArrayObject icons = (RexxArrayObject)c->GetObjectVariable(STB_ICON_ATTRIBUTE);
    if ( icons == NULLOBJECT )
    {
        return TheNilObj;
    }

    RexxObjectPtr icon = c->ArrayAt(icons, index);
    return icon == NULLOBJECT ? TheNilObj : icon;
}

RexxObjectPtr stbStoreIcon(RexxMethodContext *c, RexxObjectPtr icon, uint32_t index)
{
    RexxArrayObject icons = (RexxArrayObject)c->GetObjectVariable(STB_ICON_ATTRIBUTE);
    if ( icons == NULLOBJECT )
    {
        icons = c->NewArray(5);
        c->SetObjectVariable(STB_ICON_ATTRIBUTE, icons);
        c->ArrayPut(icons, icon, index);
        return TheNilObj;
    }

    RexxObjectPtr existing = stbGetIcon(c, index);

    c->ArrayPut(icons, icon, index);
    return existing;
}

bool isSimple(HWND hwnd)
{
    if ( SendMessage(hwnd, SB_ISSIMPLE, 0, 0) == 0 )
    {
        return false;
    }
    return true;
}

MsgReplyType sbnSimpleModeChange(pCPlainBaseDialog pcpbd, CSTRING methodName, uint32_t tag, LPARAM lParam)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;

    RexxObjectPtr idFrom     = idFrom2rexxArg(c, lParam);
    RexxObjectPtr notifyCode = notifyCode2rexxArg(c, lParam);
    RexxObjectPtr rxSb       = controlFrom2rexxArg(pcpbd, lParam, winStatusBar);

    RexxArrayObject args = c->ArrayOfThree(idFrom, notifyCode, rxSb);
    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(notifyCode);
    c->ReleaseLocalReference(rxSb);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}



/** StatusBar::getBorders()
 *
 *  Returns an array of integers containing the borders of the status bar.  The
 *  item at index 1 is the  width of the horizontal border, the item at index 2
 *  is the width of the vertical border, and the item at index 3 is the width of
 *  the border between rectangles.
 *
 *  Returns the .nil object on error.
 */
RexxMethod1(RexxObjectPtr, stb_getBorders, CSELF, pCSelf)
{
    int32_t borders[3];

    if (SendMessage(getDChCtrl(pCSelf), SB_GETBORDERS, 0, (LPARAM)&borders) )
    {
        RexxArrayObject rxBorders = context->ArrayOfThree(context->Int32(borders[0]),
                                                          context->Int32(borders[1]),
                                                          context->Int32(borders[2]));
        return rxBorders;
    }
    return TheNilObj;
}

/** StatusBar::getIcon()
 *
 *
 *  @remarks  Note that we are going to retrieve the icon Image object stored in
 *            our attribute array, so we don't want to decrement the index.
 */
RexxMethod2(RexxObjectPtr, stb_getIcon, uint32_t, index, CSELF, pCSelf)
{
    return stbGetIcon(context, index);
}

/** StatusBar::getParts()
 *
 *  Returns the number of parts in the status bar, and optionally, the
 *  co-ordinate of the right edge of each part
 *
 *  @param edges  [optional] [in / out] An array object in which the right edge
 *                of each part is returned.  If this argument is present, it
 *                will have indexes filled in with the right co-ordinate of each
 *                part.  The item at index 1 will have the right co-ordinate of
 *                the first part, the item at index 2 will have the right edge
 *                of the second part, etc.. Up to the numbe of parts.  Existing
 *                items at those indexes will be over-written.  It is suggested
 *                that the programmer use an empty array, but not required.
 *
 *  @return Returns the count of parts in this status bar, always.
 */
RexxMethod2(uint32_t, stb_getParts, OPTIONAL_RexxArrayObject, rxEdges, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    uint32_t count = (uint32_t)SendMessage(getDChCtrl(pCSelf), SB_GETPARTS, 0, 0);
    if ( argumentExists(1) )
    {
        int32_t *edges = (int32_t *)LocalAlloc(LPTR, count * sizeof(int32_t));
        if ( edges == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        SendMessage(getDChCtrl(pCSelf), SB_GETPARTS, count, (LPARAM)edges);
        for ( size_t i = 0; i < count; i++ )
        {
            c->ArrayPut(rxEdges, c->Int32(edges[i]), i + 1);
        }
    }

done_out:
    return count;
}

/** StatusBar::getRect()
 *
 *  Returns a .Rect object that contains the bounding rectangle of the specified
 *  part in this status bar.
 *
 *  @param index  [required] The one-based index of the part.
 *
 *  @return Returns a .Rect object with the bounding rectangle of the pare, or
 *          .nil on error.
 */
RexxMethod2(RexxObjectPtr, stb_getRect, uint32_t, index, CSELF, pCSelf)
{
    RECT r;

    index--;
    if ( SendMessage(getDChCtrl(pCSelf), SB_GETRECT, index, (LPARAM)&r) )
    {
        return rxNewRect(context, (PORXRECT)&r);
    }
    return TheNilObj;
}

/** StatusBar::getText()
 *
 *
 */
RexxMethod3(RexxObjectPtr, stb_getText, uint32_t, index, OPTIONAL_RexxObjectPtr, d, CSELF, pCSelf)
{
    RexxObjectPtr rxResult = TheNilObj;
    HWND          hSb      = getDChCtrl(pCSelf);

    index--;
    uint32_t result = (uint32_t)SendMessage(hSb, SB_GETTEXTLENGTH, index, 0);

    char *buf = (char *)LocalAlloc(LPTR, LOWORD(result) * sizeof(char));
    if ( buf == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    RexxDirectoryObject drawInfo = NULLOBJECT;
    if ( argumentExists(2) )
    {
        if ( ! context->IsDirectory(d) )
        {
            wrongClassException(context->threadContext, 2, "Directory", d);
            goto done_out;
        }
        drawInfo = (RexxDirectoryObject)d;
    }

    result = (uint32_t)SendMessage(hSb, SB_GETTEXT, index, (LPARAM)buf);

    rxResult = context->String(buf);
    if ( drawInfo != NULLOBJECT )
    {
        context->DirectoryPut(drawInfo, rxResult, "TEXT");
        context->DirectoryPut(drawInfo, drawtype2keyword(context, HIWORD(result)), "DRAWTYPE");
        context->DirectoryPut(drawInfo, context->UnsignedInt32(LOWORD(result)), "TEXTLENGTH");
    }

done_out:
    safeLocalFree(buf);
    return rxResult;
}

/** StatusBar::getTextLength()
 *
 *  Gets the length of the text for the specified pard of the status bar, and
 *  optionally the draw type of the text.
 *
 *  @param  [required]  One-based index of the part.
 *
 *  @param  [optional]  A directory object in which to return some extra
 *          information.  If present these indexes are filled in:  DRAWTYPE,
 *          TEXTLENGTH.
 *  @returns  The length of the text or -1 on error.
 *
 */
RexxMethod3(RexxObjectPtr, stb_getTextLength, uint32_t, index, OPTIONAL_RexxObjectPtr, d, CSELF, pCSelf)
{
    index--;
    uint32_t result = (uint32_t)SendMessage(getDChCtrl(pCSelf), SB_GETTEXTLENGTH, index, 0);

    RexxObjectPtr rxResult = context->UnsignedInt32(LOWORD(result));

    if ( argumentExists(2) )
    {
        if ( ! context->IsDirectory(d) )
        {
            wrongClassException(context->threadContext, 2, "Directory", d);
            return TheNegativeOneObj;
        }
        RexxDirectoryObject drawInfo = (RexxDirectoryObject)d;

        context->DirectoryPut(drawInfo, drawtype2keyword(context, HIWORD(result)), "DRAWTYPE");
        context->DirectoryPut(drawInfo, rxResult, "TEXTLENGTH");
    }

    return rxResult;
}

/** StatusBar::getTipText()
 *
 *
 */
RexxMethod2(RexxObjectPtr, stb_getTipText, uint32_t, index, CSELF, pCSelf)
{
    char *buf[STB_MAX_TOOLTIP_LENGTH + 1] = {'\0'};

    index--;

    SendMessage(getDChCtrl(pCSelf), SB_GETTIPTEXT, MAKEWPARAM(index, STB_MAX_TOOLTIP_LENGTH), (LPARAM)buf);

    return context->String((CSTRING)buf);
}

/** StatusBar::isSimple()
 *
 *
 */
RexxMethod1(logical_t, stb_isSimple, CSELF, pCSelf)
{
    return SendMessage(getDChCtrl(pCSelf), SB_ISSIMPLE, 0, 0);
}

/** StatusBar::setBkColor()
 *
 *
 */
RexxMethod2(uint32_t, stb_setBkColor, uint32_t, clr, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), SB_SETBKCOLOR, 0, clr);
}

/** StatusBar::setIcon()
 *
 *
 *  @returns  The existing icon, or .nil if there was no existing icon, on
 *            success.  Returns -1 if there was an error.
 */
RexxMethod3(RexxObjectPtr, stb_setIcon, RexxObjectPtr, icon, uint32_t, index, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    RexxObjectPtr existing = TheNegativeOneObj;
    index--;

    if ( c->IsOfType(icon, "IMAGE") )
    {
        POODIMAGE poi = rxGetImageIcon(context, icon, 1);
        if ( poi == NULL )
        {
            goto done_out;
        }
        if ( ! poi->isValid )
        {
            nullObjectException(context->threadContext, "Image", 1);
            goto done_out;
        }

        if ( SendMessage(getDChCtrl(pCSelf), SB_SETICON, index, (LPARAM)poi->hImage) )
        {
            existing = stbStoreIcon(context, icon, index + 1);
        }
        goto done_out;
    }
    else if ( icon == TheNilObj )
    {
        if ( SendMessage(getDChCtrl(pCSelf), SB_SETICON, index, NULL) )
        {
            // This will put .nil at the index, which is good.
            existing = stbStoreIcon(context, icon, index + 1);
        }
        goto done_out;
    }
    else
    {
        wrongArgValueException(c->threadContext, 1, "an icon image or .nil", icon);
    }

done_out:
    return existing;
}

/** StatusBar::setMinHeight()
 *
 *
 */
RexxMethod2(uint32_t, stb_setMinHeight, uint32_t, height, CSELF, pCSelf)
{
    SendMessage(getDChCtrl(pCSelf), SB_SETMINHEIGHT, height, 0);
    SendMessage(getDChCtrl(pCSelf), WM_SIZE, 0, 0);
    return 0;
}

/** StatusBar::setParts()
 *
 *  Sets the number of parts in a status window and the coordinate of the right
 *  edge of each part.
 *
 *  @param  rightEdge  [required]  An array of integers.  Each item specifies
 *                     the right edge, in client co-oridnates, in pixels, of a
 *                     part.
 *
 *                     -1 specifies the part extends to the right edge of client
 *                     area.
 *
 *  @return  True on success, otherwise false.
 *
 *  @note  The array must be non-sparse and contain only integers.
 */
RexxMethod2(logical_t, stb_setParts, RexxArrayObject, _edges, CSELF, pCSelf)
{
    logical_t result = FALSE;
    int32_t   *edges = NULL;

    size_t count = context->ArrayItems(_edges);
    if ( count == 0 )
    {
        emptyArrayException(context->threadContext, 1);
        goto done_out;
    }

    edges = (int32_t *)LocalAlloc(LPTR, count * sizeof(int32_t));
    if ( edges == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    for ( size_t i = 1; i <= count; i++ )
    {
        int32_t e;

        RexxObjectPtr rxE = context->ArrayAt(_edges, i);
        if ( rxE == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, i);
            goto done_out;
        }
        if ( ! context->Int32(rxE, &e) )
        {
            wrongObjInArrayException(context->threadContext, 1, i, "a whole number less than 2147483647", rxE);
            goto done_out;
        }

        edges[i - 1] = e;
    }

    if ( SendMessage(getDChCtrl(pCSelf), SB_SETPARTS, count, (LPARAM)edges) )
    {
        result = TRUE;
    }

done_out:
    return result;
}

/** StatusBar::setText()
 *
 *  Sets
 *
 *  @param  text       [required]  The text for the specified part.
 *
 *  @param  index      [required]  The 1-based index of the part.
 *
 *  @param  type       [optional]  Single keyword that specifies the draw type
 *                     of the text.  If this argument is omitted the draw typw
 *                     will be the default type.
 *
 *  @return  True on success, false otherwise.
 *
 *  @note  We use LOWERBORDERS to specify the default type.  There is no keywork
 *         for it, it is just 0.
 */
RexxMethod4(logical_t, stb_setText, CSTRING, text, uint32_t, index, OPTIONAL_CSTRING, _type, CSELF, pCSelf)
{
    uint32_t type = 0;

    if ( argumentExists(2) )
    {
        type = keyword2drawType(context, _type, 2);
        if ( type == OOD_NO_VALUE )
        {
            return FALSE;
        }
    }

    index--;
    index |= type;
    return SendMessage(getDChCtrl(pCSelf), SB_SETTEXT, (WPARAM)index, (LPARAM)text);
}

/** StatusBar::setTipText()
 *
 *
 */
RexxMethod3(RexxObjectPtr, stb_setTipText, CSTRING, text, uint32_t, index, CSELF, pCSelf)
{
    if ( strlen(text) > STB_MAX_TOOLTIP_LENGTH )
    {
        stringTooLongException(context->threadContext, 2, STB_MAX_TOOLTIP_LENGTH, strlen(text));
        return TheNegativeOneObj;
    }

    index--;

    SendMessage(getDChCtrl(pCSelf), SB_SETTIPTEXT, index, (LPARAM)text);

    return TheZeroObj;
}

/** StatusBar::simple()
 *
 *
 */
RexxMethod2(uint32_t, stb_simple, OPTIONAL_logical_t, simple, CSELF, pCSelf)
{
    if ( argumentOmitted(1) )
    {
        simple = TRUE;
    }
    SendMessage(getDChCtrl(pCSelf), SB_SIMPLE, simple, 0);
    return 0;
}


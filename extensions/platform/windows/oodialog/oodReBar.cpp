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
 * oodReBar.cpp
 *
 * Implementation for the ReBar dialog control.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodResources.hpp"
#include "oodShared.hpp"

/**
 *  Methods for the .ReBarBandInfo class.
 */
#define REBARBANDINFO_CLASS           "ReBarBandInfo"

#define REBARBANDINFO_OBJ_MAGIC       "Mv1vc2bn@l"
#define REBARBAND_TEXT_MAX            260
#define REBARBANDINFO_MASK_ALL        RBBIM_BACKGROUND | RBBIM_CHEVRONLOCATION | RBBIM_CHEVRONSTATE | RBBIM_COLORS | \
                                      RBBIM_HEADERSIZE | RBBIM_ID | RBBIM_IDEALSIZE | RBBIM_IMAGE | RBBIM_LPARAM | \
                                      RBBIM_SIZE | RBBIM_STYLE | RBBIM_TEXT | RBBIM_CHILDSIZE | RBBIM_CHILD


inline bool isRbbiInternalInit(RexxMethodContext *context, RexxObjectPtr child, CSTRING text)
{
    return argumentExists(1)
        && context->IsBuffer(child)
        && argumentExists(2)
        && strcmp(text, REBARBANDINFO_OBJ_MAGIC) == 0;
}

/**
 * Converts a string of keywords to the proper RBBIM_* mask flag.
 *
 * @param flags
 *
 * @return uint32_t
 */
uint32_t keyword2rbbim(CSTRING flags)
{
    uint32_t val = 0;

    char *words = StrDup(flags);
    if ( words != NULL )
    {
        char *token = strtok(words, " ");
        while ( token != NULL )
        {
            if (      StrCmpI(token, "BACKGROUND")      == 0 ) val |= RBBIM_BACKGROUND;
            else if ( StrCmpI(token, "CHEVRONLOCATION") == 0 ) val |= RBBIM_CHEVRONLOCATION;
            else if ( StrCmpI(token, "CHEVRONSTATE")    == 0 ) val |= RBBIM_CHEVRONSTATE;
            else if ( StrCmpI(token, "CHILD")           == 0 ) val |= RBBIM_CHILD;
            else if ( StrCmpI(token, "CHILDSIZE")       == 0 ) val |= RBBIM_CHILDSIZE;
            else if ( StrCmpI(token, "COLORS")          == 0 ) val |= RBBIM_COLORS;
            else if ( StrCmpI(token, "HEADERSIZE")      == 0 ) val |= RBBIM_HEADERSIZE;
            else if ( StrCmpI(token, "ID")              == 0 ) val |= RBBIM_ID;
            else if ( StrCmpI(token, "IDEALSIZE")       == 0 ) val |= RBBIM_IDEALSIZE;
            else if ( StrCmpI(token, "IMAGE")           == 0 ) val |= RBBIM_IMAGE;
            else if ( StrCmpI(token, "LPARAM")          == 0 ) val |= RBBIM_LPARAM;
            else if ( StrCmpI(token, "SIZE")            == 0 ) val |= RBBIM_SIZE;
            else if ( StrCmpI(token, "STYLE")           == 0 ) val |= RBBIM_STYLE;
            else if ( StrCmpI(token, "TEXT")            == 0 ) val |= RBBIM_TEXT;

            token = strtok(NULL, " ");
        }
        LocalFree(words);
    }

    return val;
}


/**
 * Converts a set of RBBIM_* mask flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject rbbim2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];  // This is big enough.
    *buf = '\0';

    if ( flags & RBBIM_BACKGROUND     ) strcat(buf, "BACKGROUND ");
    if ( flags & RBBIM_CHEVRONLOCATION) strcat(buf, "CHEVRONLOCATION ");
    if ( flags & RBBIM_CHEVRONSTATE   ) strcat(buf, "CHEVRONSTATE ");
    if ( flags & RBBIM_CHILD          ) strcat(buf, "CHILD ");
    if ( flags & RBBIM_CHILDSIZE      ) strcat(buf, "CHILDSIZE ");
    if ( flags & RBBIM_COLORS         ) strcat(buf, "COLORS ");
    if ( flags & RBBIM_HEADERSIZE     ) strcat(buf, "HEADERSIZE ");
    if ( flags & RBBIM_ID             ) strcat(buf, "ID ");
    if ( flags & RBBIM_IDEALSIZE      ) strcat(buf, "IDEALSIZE ");
    if ( flags & RBBIM_IMAGE          ) strcat(buf, "IMAGE ");
    if ( flags & RBBIM_LPARAM         ) strcat(buf, "LPARAM ");
    if ( flags & RBBIM_SIZE           ) strcat(buf, "SIZE ");
    if ( flags & RBBIM_STYLE          ) strcat(buf, "STYLE ");
    if ( flags & RBBIM_TEXT           ) strcat(buf, "TEXT ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}

/**
 * Converts a string of keywords to the proper RBB_* style flag.
 *
 * @param flags
 *
 * @return uint32_t
 */
uint32_t keyword2rbbs(CSTRING flags)
{
    uint32_t val = 0;

    if ( StrStrI(flags, "BREAK")          != NULL ) val |= RBBS_BREAK         ;
    if ( StrStrI(flags, "CHILDEDGE")      != NULL ) val |= RBBS_CHILDEDGE     ;
    if ( StrStrI(flags, "FIXEDBMP")       != NULL ) val |= RBBS_FIXEDBMP      ;
    if ( StrStrI(flags, "FIXEDSIZE")      != NULL ) val |= RBBS_FIXEDSIZE     ;
    if ( StrStrI(flags, "GRIPPERALWAYS")  != NULL ) val |= RBBS_GRIPPERALWAYS ;
    if ( StrStrI(flags, "HIDDEN")         != NULL ) val |= RBBS_HIDDEN        ;
    if ( StrStrI(flags, "HIDETITLE")      != NULL ) val |= RBBS_HIDETITLE     ;
    if ( StrStrI(flags, "NOGRIPPER")      != NULL ) val |= RBBS_NOGRIPPER     ;
    if ( StrStrI(flags, "NOVERT")         != NULL ) val |= RBBS_NOVERT        ;
    if ( StrStrI(flags, "TOPALIGN")       != NULL ) val |= RBBS_TOPALIGN      ;
    if ( StrStrI(flags, "USECHEVRON")     != NULL ) val |= RBBS_USECHEVRON    ;
    if ( StrStrI(flags, "VARIABLEHEIGHT") != NULL ) val |= RBBS_VARIABLEHEIGHT;

    return val;
}


/**
 * Converts a set of RBB_* style flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject rbbs2keyword(RexxThreadContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & RBBS_BREAK           ) strcat(buf, "BREAK ");
    if ( flags & RBBS_CHILDEDGE       ) strcat(buf, "CHILDEDGE ");
    if ( flags & RBBS_FIXEDBMP        ) strcat(buf, "FIXEDBMP ");
    if ( flags & RBBS_FIXEDSIZE       ) strcat(buf, "FIXEDSIZE ");
    if ( flags & RBBS_GRIPPERALWAYS   ) strcat(buf, "GRIPPERALWAYS ");
    if ( flags & RBBS_HIDDEN          ) strcat(buf, "HIDDEN ");
    if ( flags & RBBS_HIDETITLE       ) strcat(buf, "HIDETITLE ");
    if ( flags & RBBS_NOGRIPPER       ) strcat(buf, "NOGRIPPER ");
    if ( flags & RBBS_NOVERT          ) strcat(buf, "NOVERT ");
    if ( flags & RBBS_TOPALIGN        ) strcat(buf, "TOPALIGN ");
    if ( flags & RBBS_USECHEVRON      ) strcat(buf, "USECHEVRON ");
    if ( flags & RBBS_VARIABLEHEIGHT  ) strcat(buf, "VARIABLEHEIGHT ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}

RexxObjectPtr getRbbiText(RexxMethodContext *c, LPREBARBANDINFO prbbi)
{
    if ( prbbi->lpText == NULL )
    {
        return TheNilObj;
    }

    return c->String(prbbi->lpText);
}

/**
 * Sets the text attribute for the ReBarBandInfo object.
 *
 * If the ReBarBandInfo is used to receive information, the lpText member has to
 * point to a buffer to recieve the text.  When setting text, we probably do not
 * need to allocate a buffer, we could just assign the pointer.  However, to
 * keep things simpler, we just always allocate a buffer.
 *
 * Also, we set an arbitrary length of the text to 260 characters.  This is
 * similar to the maximum length of a string that will be displayed in other
 * controls.
 *
 * @param c
 * @param prbbi
 * @param text
 *
 * @return bool
 */
bool setRbbiText(RexxMethodContext *c, LPREBARBANDINFO prbbi, CSTRING text, size_t argPos)
{
    if ( text != NULL )
    {
        size_t len = strlen(text);
        if ( len > REBARBAND_TEXT_MAX )
        {
            stringTooLongException(c->threadContext, argPos, REBARBAND_TEXT_MAX, len);
            return false;
        }
    }

    if ( prbbi->lpText == NULL )
    {
        prbbi->lpText = (char *)LocalAlloc(LPTR, REBARBAND_TEXT_MAX + 1);
        if ( prbbi->lpText == NULL )
        {
            outOfMemoryException(c->threadContext);
            return false;
        }
        prbbi->cch = REBARBAND_TEXT_MAX + 1;
    }

    prbbi->fMask |= RBBIM_TEXT;

    if ( text != NULL )
    {
        strcpy(prbbi->lpText, text);
    }
    return true;
}

RexxObjectPtr getRbbiChild(RexxMethodContext *c, LPREBARBANDINFO prbbi)
{
    HWND hChild = prbbi->hwndChild;
    if ( hChild != NULL )
    {
        RexxObjectPtr rxControl = (RexxObjectPtr)getWindowPtr(hChild, GWLP_USERDATA);
        if ( rxControl != NULLOBJECT )
        {
            pCDialogControl pcdc = controlToCSelf(c->threadContext, rxControl);
            if ( pcdc != NULL )
            {
                return pcdc->rexxSelf;
            }
        }
    }
    return TheNilObj;
}

bool setRbbiChild(RexxMethodContext *c, LPREBARBANDINFO prbbi, RexxObjectPtr child)
{
    pCDialogControl pcdc = requiredDlgControlCSelf(c, child, 1);
    if ( pcdc != NULLOBJECT )
    {
        prbbi->hwndChild = pcdc->hCtrl;
        prbbi->fMask |= RBBIM_CHILD;
        return true;
    }
    return false;
}

bool setRbbiID(RexxMethodContext *c, LPREBARBANDINFO prbbi, RexxObjectPtr rxID, size_t argPos)
{
    prbbi->wID = oodGlobalID(c, rxID, argPos, true);
    if ( prbbi->wID == OOD_ID_EXCEPTION )
    {
        return false;
    }
    prbbi->fMask |= RBBIM_ID;
    return true;
}


/** ReBarBandInfo::init()      [Class]
 *
 */
RexxMethod1(RexxObjectPtr, rbbi_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, REBARBANDINFO_CLASS) )
    {
        TheReBarBandInfoClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheReBarBandInfoClass);
    }
    return NULLOBJECT;
}


/** ReBarBandInfo::uninit()
 *
 */
RexxMethod1(RexxObjectPtr, rbbi_unInit, CSELF, pCSelf)
{
#if 0
    printf("In rbbi_unInit() pCSelf=%p\n", pCSelf);
#endif

    if ( pCSelf != NULLOBJECT )
    {
        LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;

#if 0
    printf("In rbbi_unInit() pszText=%p\n", prbbi->lpText);
#endif
        safeLocalFree(prbbi->lpText);
    }
    return NULLOBJECT;
}

/** ReBarBandInfo::init()
 *
 *  Just temp for now.  We are not going to allow passing in every attribute,
 *  but we still need to figure out which attributes to accept / are the most
 *  useful.
 */
RexxMethod7(RexxObjectPtr , rbbi_init
           , OPTIONAL_RexxObjectPtr, _child
           , OPTIONAL_CSTRING      , text
           , OPTIONAL_CSTRING      , style
           , OPTIONAL_RexxObjectPtr, itemData
           , OPTIONAL_RexxObjectPtr, rxID
           , OPTIONAL_uint32_t     , cx
           , OPTIONAL_CSTRING      , mask)
{
    if ( isRbbiInternalInit(context, _child, text) )
    {
        context->SetObjectVariable("CSELF", _child);
        goto done_out;
    }

    RexxBufferObject obj = context->NewBuffer(sizeof(REBARBANDINFO));
    context->SetObjectVariable("CSELF", obj);

    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)context->BufferData(obj);
    memset(prbbi, 0, sizeof(REBARBANDINFO));
    prbbi->cbSize = sizeof(REBARBANDINFO);

    if ( argumentExists(1) )
    {
        if ( ! setRbbiChild(context, prbbi, _child) )
        {
            goto done_out;
        }
    }

    // We always create a buffer to recieve text.  If the user omits the text
    // argument, the buffer will be created and set to the empty string.
    if ( ! setRbbiText(context, prbbi, text, 2) )
    {
        goto done_out;
    }

    if ( argumentExists(3) )
    {
        prbbi->fStyle = keyword2rbbs(style);
        prbbi->fMask |= RBBIM_STYLE;
    }

    if ( argumentExists(4) )
    {
        prbbi->lParam = (LPARAM)itemData;
        prbbi->fMask |= RBBIM_LPARAM;
    }

    if ( argumentExists(5) )
    {
        if ( ! setRbbiID(context, prbbi, rxID, 5) )
        {
            goto done_out;
        }
    }

    if ( argumentExists(6) )
    {
        prbbi->cx     = cx;
        prbbi->fMask |= RBBIM_SIZE;
    }

    if ( argumentExists(7) )
    {
        prbbi->fMask = keyword2rbbim(mask);
    }

done_out:
    return NULLOBJECT;
}

/** ReBarBandInfo::bitmapBack                [attribute]
 */
RexxMethod1(RexxObjectPtr, rbbi_bitmapBack, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;
    if ( prbbi->hbmBack != NULL )
    {
        SIZE s = {0};
        return rxNewValidImage(context, prbbi->hbmBack, IMAGE_BITMAP, &s, LR_DEFAULTCOLOR, false);
    }
    return TheNilObj;
}
RexxMethod2(RexxObjectPtr, rbbi_setBitmapBack, RexxObjectPtr, _bitmap, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;

    if ( ! context->IsOfType(_bitmap, "IMAGE") )
    {
        wrongClassException(context->threadContext, 1, "Image", _bitmap);
    }
    else
    {
        POODIMAGE oodImage = rxGetImageBitmap(context, _bitmap, 1);
        if ( oodImage != NULL )
        {
            prbbi->hbmBack = (HBITMAP)oodImage->hImage;
            prbbi->fMask |= RBBIM_BACKGROUND;
        }
    }
    return NULLOBJECT;
}

/** ReBarBandInfo::chevronLocation          [attribute]
 */
RexxMethod1(RexxObjectPtr, rbbi_chevronLocation, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;

    if ( prbbi->rcChevronLocation.left < 1 && prbbi->rcChevronLocation.top < 1 &&
         prbbi->rcChevronLocation.right < 1 && prbbi->rcChevronLocation.bottom < 1 )
    {
        return TheNilObj;
    }
    return rxNewRect(context, prbbi->rcChevronLocation.left, prbbi->rcChevronLocation.top,
                     prbbi->rcChevronLocation.right, prbbi->rcChevronLocation.bottom);
}
RexxMethod2(RexxObjectPtr, rbbi_setChevronLocation, RexxObjectPtr, _rect, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;

    RECT *r = (PRECT)rxGetRect(context, _rect, 1);
    if ( r != NULL )
    {
        prbbi->rcChevronLocation.left   = r->left;
        prbbi->rcChevronLocation.top    = r->top;
        prbbi->rcChevronLocation.right  = r->right;
        prbbi->rcChevronLocation.bottom = r->bottom;
        prbbi->fMask |= RBBIM_CHEVRONLOCATION;
    }
    return NULLOBJECT;
}

/** ReBarBandInfo::chevronState                [attribute]
 */
RexxMethod1(RexxStringObject, rbbi_chevronState, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;
    return stateSystem2keyword(context, prbbi->uChevronState);
}
RexxMethod2(RexxObjectPtr, rbbi_setChevronState, CSTRING, state, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;
    prbbi->uChevronState = keyword2stateSystem(state);
    prbbi->fMask |= RBBIM_CHEVRONSTATE;
    return NULLOBJECT;
}

/** ReBarBandInfo::child             [attribute]
 *
 *  We return .nil if we fail to get the Rexx child control.  But, really that
 *  shouldn't happen if hChild is not NULL.
 */
RexxMethod1(RexxObjectPtr, rbbi_child, CSELF, pCSelf)
{
    return getRbbiChild(context, (LPREBARBANDINFO)pCSelf);

}
RexxMethod2(RexxObjectPtr, rbbi_setChild, RexxObjectPtr, _child, CSELF, pCSelf)
{
    setRbbiChild(context, (LPREBARBANDINFO)pCSelf, _child);
    return NULLOBJECT;
}

/** ReBarBandInfo::clrBack                 [attribute]
 */
RexxMethod1(uint32_t, rbbi_clrBack, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->clrBack;
}
RexxMethod2(RexxObjectPtr, rbbi_setClrBack, uint32_t, bg, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->clrBack = bg;
    ((LPREBARBANDINFO)prbbi)->fMask   |= RBBIM_COLORS;
    return NULLOBJECT;
}

/** ReBarBandInfo::clrFore                 [attribute]
 */
RexxMethod1(uint32_t, rbbi_clrFore, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->clrFore;
}
RexxMethod2(RexxObjectPtr, rbbi_setClrFore, uint32_t, fg, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->clrFore = fg;
    ((LPREBARBANDINFO)prbbi)->fMask   |= RBBIM_COLORS;
    return NULLOBJECT;
}

/** ReBarBandInfo::cx                  [attribute]
 */
RexxMethod1(uint32_t, rbbi_cx, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cx;
}
RexxMethod2(RexxObjectPtr, rbbi_setCx, uint32_t, cx, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cx    = cx;
    ((LPREBARBANDINFO)prbbi)->fMask |= RBBIM_SIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cxHeader              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cxHeader, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cxHeader;
}
RexxMethod2(RexxObjectPtr, rbbi_setCxHeader, uint32_t, cx, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cxHeader = cx;
    ((LPREBARBANDINFO)prbbi)->fMask    |= RBBIM_HEADERSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cxIdeal              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cxIdeal, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cxIdeal;
}
RexxMethod2(RexxObjectPtr, rbbi_setCxIdeal, uint32_t, cx, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cxIdeal = cx;
    ((LPREBARBANDINFO)prbbi)->fMask  |= RBBIM_IDEALSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cxMinChild              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cxMinChild, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cxMinChild;
}
RexxMethod2(RexxObjectPtr, rbbi_setCxMinChild, uint32_t, cx, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cxMinChild  = cx;
    ((LPREBARBANDINFO)prbbi)->fMask      |= RBBIM_CHILDSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cyChild              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cyChild, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cyChild;
}
RexxMethod2(RexxObjectPtr, rbbi_setCyChild, uint32_t, cy, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cyChild = cy;
    ((LPREBARBANDINFO)prbbi)->fMask  |= RBBIM_CHILDSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cyIntegral              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cyIntegral, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cyIntegral;
}
RexxMethod2(RexxObjectPtr, rbbi_setCyIntegral, uint32_t, cy, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cyIntegral = cy;
    ((LPREBARBANDINFO)prbbi)->fMask     |= RBBIM_CHILDSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cyMaxChild              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cyMaxChild, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cyMaxChild;
}
RexxMethod2(RexxObjectPtr, rbbi_setCyMaxChild, uint32_t, cy, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cyMaxChild = cy;
    ((LPREBARBANDINFO)prbbi)->fMask     |= RBBIM_CHILDSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::cyMinChild              [attribute]
 */
RexxMethod1(uint32_t, rbbi_cyMinChild, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->cyMinChild;
}
RexxMethod2(RexxObjectPtr, rbbi_setCyMinChild, uint32_t, cy, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->cyMinChild = cy;
    ((LPREBARBANDINFO)prbbi)->fMask      |= RBBIM_CHILDSIZE;
    return NULLOBJECT;
}

/** ReBarBandInfo::id          [attribute]
 */
RexxMethod1(uint32_t, rbbi_id, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->wID;
}
RexxMethod2(RexxObjectPtr, rbbi_setId, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    setRbbiID(context, (LPREBARBANDINFO)pCSelf, rxID, 1);
    return NULLOBJECT;
}

/** ReBarBandInfo::imageIndex             [attribute]
 */
RexxMethod1(int32_t, rbbi_imageIndex, CSELF, prbbi)
{
    return ((LPREBARBANDINFO)prbbi)->iImage;
}
RexxMethod2(RexxObjectPtr, rbbi_setImageIndex, int32_t, imageIndex, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->iImage  = imageIndex < 0 ? I_IMAGENONE : imageIndex;
    ((LPREBARBANDINFO)prbbi)->fMask  |= RBBIM_IMAGE;
    return NULLOBJECT;
}

/** ReBarBandInfo::itemData               [attribute]
 */
RexxMethod1(RexxObjectPtr, rbbi_itemData, CSELF, pCSelf)
{
    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)pCSelf;
    return ((prbbi->lParam == NULL) ? TheNilObj : (RexxObjectPtr)prbbi->lParam);
}
RexxMethod2(RexxObjectPtr, rbbi_setItemData, RexxObjectPtr, userData, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->lParam = (LPARAM)userData;
    ((LPREBARBANDINFO)prbbi)->fMask |= RBBIM_LPARAM;
    return NULLOBJECT;
}

/** ReBarBandInfo::mask                   [attribute]
 */
RexxMethod1(RexxStringObject, rbbi_mask, CSELF, prbbi)
{
    return rbbim2keyword(context, ((LPREBARBANDINFO)prbbi)->fMask);
}
RexxMethod2(RexxObjectPtr, rbbi_setMask, CSTRING, mask, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->fMask = keyword2rbbim(mask);
    return NULLOBJECT;
}

/** ReBarBandInfo::style                   [attribute]
 */
RexxMethod1(RexxStringObject, rbbi_style, CSELF, prbbi)
{
    return rbbs2keyword(context->threadContext, ((LPREBARBANDINFO)prbbi)->fStyle);
}
RexxMethod2(RexxObjectPtr, rbbi_setStyle, CSTRING, style, CSELF, prbbi)
{
    ((LPREBARBANDINFO)prbbi)->fStyle = keyword2rbbs(style);
    ((LPREBARBANDINFO)prbbi)->fMask |= RBBIM_STYLE;
    return NULLOBJECT;
}

/** ReBarBandInfo::text                   [attribute]
 */
RexxMethod1(RexxObjectPtr, rbbi_text, CSELF, prbbi)
{
    return getRbbiText(context, (LPREBARBANDINFO)prbbi);
}
RexxMethod2(RexxObjectPtr, rbbi_setText, CSTRING, text, CSELF, prbbi)
{
    setRbbiText(context, (LPREBARBANDINFO)prbbi, text, 1);
    return NULLOBJECT;
}


/**
 * Methods for the ReBar class.
 */
#define REBAR_CLASS   "ReBar"

#define RB_IMAGELIST_ATTRIB_NAME  "ReBarImageListAttribute"

// Windows SDK 7.1 does not define these extended styles.  We define them
// ourselves, assuming we can look up the correct values later.
#define RBS_EX_SPLITTER     0x0001
#define RBS_EX_TRANSPARENT  0x0002

/**
 * Converts a string of ReBar extended style keywords to the proper
 * RBS_EX_* style flag.
 *
 * @param flags
 *
 * @return uint32_t
 *
 * @note  We accept NULL for flags to accomdate an omitted argument.
 */
uint32_t keyword2rbsExt(CSTRING flags)
{
    uint32_t val = 0;

    if ( flags != NULL )
    {
        if ( StrStrI(flags, "SPLITTER")    != NULL ) val |= RBS_EX_SPLITTER;
        if ( StrStrI(flags, "TRANSPARENT") != NULL ) val |= RBS_EX_TRANSPARENT;
        if ( StrStrI(flags, "NONE")        != NULL ) val  = 0;
    }

    return val;
}

/**
 * Converts a set of RBS_EX_* ReBar extended style flags to their keyword
 * string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject rbsExt2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & RBS_EX_SPLITTER    ) strcat(buf, "SPLITTER ");
    if ( flags & RBS_EX_TRANSPARENT ) strcat(buf, "TRANSPARENT ");

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

static void genericRbnInvoke(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, RexxArrayObject args, uint32_t tag)
{
    switch ( tag & TAG_EXTRAMASK )
    {
        case TAG_REPLYFROMREXX :
            invokeDirect(c, pcpbd, methodName, args);
            break;

        case TAG_SYNC :
            invokeSync(c, pcpbd, methodName, args);
            break;

        default :
            invokeDispatch(c, pcpbd, methodName, args);
            break;
    }
}


/**
 * Handles the RBN_AUTOBREAK notification.  The user must reply true or false,
 * true to accept the break, false to reject it.
 *
 * @param pcpbd
 * @param methodName
 * @param tag
 * @param lParam
 *
 * @return MsgReplyType
 *
 * @note  Args to Rexx are:
 *
 *        use arg idFrom, bandIndex, wID, style, reBar, itemData, msgID
 */
MsgReplyType rbnAutobreak(pCPlainBaseDialog pcpbd, CSTRING methodName, uint32_t tag, LPARAM lParam)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;

    MsgReplyType  winReply = ReplyTrue;
    RexxObjectPtr idFrom   = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxRB     = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winReBar, true);

    LPNMREBARAUTOBREAK prab = (LPNMREBARAUTOBREAK)lParam;

    RexxObjectPtr bandIndex = c->UnsignedInt32(prab->uBand + 1); // could be -1, need to test
    RexxObjectPtr wID       = c->UnsignedInt32(prab->wID);       // application defined ID
    RexxObjectPtr style     = rbbs2keyword(c, prab->fStyleCurrent);
    RexxObjectPtr itemData  = prab->lParam == NULL ? TheNilObj : (RexxObjectPtr)prab->lParam;
    RexxObjectPtr rxMsgID   = c->UnsignedInt32(prab->uMsg);      // not sure what this is, or if we need it?

    RexxArrayObject args = c->ArrayOfFour(idFrom, bandIndex, wID, style);
    c->ArrayPut(args, rxRB, 5);
    c->ArrayPut(args, itemData, 6);
    c->ArrayPut(args, rxMsgID, 7);

    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);

    prab->fAutoBreak = (msgReply == TheFalseObj ? FALSE : TRUE);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(bandIndex);
    c->ReleaseLocalReference(wID);
    c->ReleaseLocalReference(style);
    c->ReleaseLocalReference(rxRB);
    c->ReleaseLocalReference(itemData);
    c->ReleaseLocalReference(rxMsgID);
    c->ReleaseLocalReference(args);

    return winReply;
}


/**
 * Sent by a rebar control when the control receives a WM_NCHITTEST message.
 *
 * Return zero to allow the rebar to perform default processing of the hit test
 * message, or return one of the HT* values documented under WM_NCHITTEST to
 * override the default hit test processing.
 *
 * The Rexx event handler is sent these args:
 *
 *   use arg id, bandIndex (1-based), point, ReBar object
 *
 * and returns 0 or one of the HT keywords.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks  The NMMOUSE struct has a dwItemData field that is sometimes not
 *           null.  But, it is definitely not the itemData of a band, and the
 *           doc for the ReBar NM_NCHITTEST does not say it is filled in.  Do
 *           not send it to Rexx.
 *
 *           In addition, the doc is not very clear on how to  return one of the
 *           HT* values documented under WM_NCHITTEST to override the default
 *           hit test processing. Originally we returned the value in
 *           DWLP_MSGRESULT. But maybe the dwHitInfo field is suppossed to be
 *           set to a new value?  Now we do both.
 */
MsgReplyType rbnNcHitTest(pCPlainBaseDialog pcpbd, CSTRING methodName, uint32_t tag, LPARAM lParam)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;

    MsgReplyType  winReply = ReplyFalse;
    RexxObjectPtr idFrom   = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxRB     = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winReBar, true);

    LPNMMOUSE pMouse = (LPNMMOUSE)lParam;

    RexxObjectPtr bandIndex = c->Uintptr(pMouse->dwItemSpec + 1);
    RexxObjectPtr pt        = rxNewPoint(c, (PORXPOINT)&pMouse->pt);
    RexxObjectPtr htWord    = ncHitTest2string(c, pMouse->dwHitInfo);
    RexxArrayObject args    = c->ArrayOfFour(idFrom, bandIndex, pt, htWord);

    c->ArrayPut(args, rxRB, 5);

    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    if ( msgReplyIsGood(c, pcpbd, msgReply, methodName, false) )
    {
        CSTRING htText = c->ObjectToStringValue(msgReply);
        if ( *htText != '0' )
        {
            uint32_t ht = keyword2ncHitTestt(htText);
            pMouse->dwHitInfo = ht;
            setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, ht);
            winReply = ReplyTrue;
        }
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(bandIndex);
    c->ReleaseLocalReference(pt);
    c->ReleaseLocalReference(htWord);
    c->ReleaseLocalReference(rxRB);
    c->ReleaseLocalReference(args);
    if ( msgReply != NULLOBJECT )
    {
        c->ReleaseLocalReference(msgReply);
    }

    return winReply;
}


/** ReBar::deleteBand()
 */
RexxMethod2(logical_t, rebar_deleteBand, uint32_t, index, CSELF, pCSelf)
{
    index--;
    return (logical_t)SendMessage(getDChCtrl(pCSelf), RB_DELETEBAND, index, 0);
}

/** ReBar::getBandBorders()
 */
RexxMethod2(RexxObjectPtr, rebar_getBandBorders, uint32_t, index, CSELF, pCSelf)
{
    RECT r = {0};

    index--;
    SendMessage(getDChCtrl(pCSelf), RB_GETBANDBORDERS, index, (LPARAM)&r);
    return rxNewRect(context, (PORXRECT)&r);
}

/** ReBar::getBandCount()
 */
RexxMethod1(uint32_t, rebar_getBandCount, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETBANDCOUNT, 0, 0);
}

/** ReBar::getBandInfo()
 */
RexxMethod3(RexxObjectPtr, rebar_getBandInfo, uint32_t, index, OPTIONAL_CSTRING, mask, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    RexxBufferObject obj = context->NewBuffer(sizeof(REBARBANDINFO));
    if ( obj == NULLOBJECT )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    LPREBARBANDINFO prbbi = (LPREBARBANDINFO)context->BufferData(obj);
    memset(prbbi, 0, sizeof(REBARBANDINFO));
    prbbi->cbSize = sizeof(REBARBANDINFO);

    if ( argumentOmitted(2) )
    {
        prbbi->fMask = REBARBANDINFO_MASK_ALL;
    }
    else
    {
        prbbi->fMask = keyword2rbbim(mask);
    }

    if ( prbbi->fMask & RBBIM_TEXT )
    {
        setRbbiText(context, prbbi, NULL, 1);
    }

    index--;
    if ( SendMessage(getDChCtrl(pCSelf), RB_GETBANDINFO, index, (LPARAM)prbbi) )
    {
        result = context->SendMessage2(TheReBarBandInfoClass, "NEW", obj, context->String(REBARBANDINFO_OBJ_MAGIC));
    }

done_out:
    return result;
}

/** ReBar::getBandMargins()
 */
RexxMethod1(RexxObjectPtr, rebar_getBandMargins, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "getBandMargins", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    MARGINS m = {0};

    SendMessage(getDChCtrl(pCSelf), RB_GETBANDBORDERS, 0, (LPARAM)&m);
    return rxNewRect(context, (ORXRECT *)&m);
}

/** ReBar::getBarHeight()
 */
RexxMethod1(uint32_t, rebar_getBarHeight, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETBARHEIGHT, 0, 0);
}

/** ReBar::getBarInfo()
 */
RexxMethod1(RexxObjectPtr, rebar_getBarInfo, CSELF, pCSelf)
{
    RexxObjectPtr ret = TheNilObj;
    REBARINFO     rbi = {sizeof(REBARINFO), RBIM_IMAGELIST, NULL};

    if ( SendMessage(getDChCtrl(pCSelf), RB_GETBARINFO, 0, (LPARAM)&rbi) && rbi.himl != NULL )
    {
        ret = context->GetObjectVariable(RB_IMAGELIST_ATTRIB_NAME);
        if ( ret == NULLOBJECT )
        {
            ret = TheNilObj;
        }
    }
    return ret;
}

/** ReBar::getBkColor()
 */
RexxMethod1(uint32_t, rebar_getBkColor, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETBKCOLOR, 0, 0);
}

/** ReBar::getColorScheme()
 */
RexxMethod1(RexxObjectPtr, rebar_getColorScheme, CSELF, pCSelf)
{
    COLORSCHEME cs = {sizeof(COLORSCHEME)};

    if ( SendMessage(getDChCtrl(pCSelf), RB_GETCOLORSCHEME, 0, (LPARAM)&cs) )
    {
        RexxDirectoryObject d = context->NewDirectory();

        context->DirectoryPut(d, context->UnsignedInt32(cs.clrBtnHighlight), "CLRBTNHIGHLIGHT");
        context->DirectoryPut(d, context->UnsignedInt32(cs.clrBtnShadow), "CLRBTNSHADOW");
        return d;
    }

    return TheNilObj;
}

/** ReBar::getExtendedStyle()
 */
RexxMethod1(RexxObjectPtr, rebar_getExtendedStyle, CSELF, pCSelf)
{
    if ( requiredOS(context, "getExtendedStyle", "Vista", Vista_OS) )
    {
        uint32_t style = (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETEXTENDEDSTYLE, 0, 0);
        return rbsExt2keyword(context, style);
    }
    return NULLOBJECT;
}

/** ReBar::getPalette()
 */
RexxMethod1(POINTER, rebar_getPalette, CSELF, pCSelf)
{
    HPALETTE hp = (HPALETTE)SendMessage(getDChCtrl(pCSelf), RB_GETPALETTE, 0, 0);
    return hp;
}

/** ReBar::getRect()
 */
RexxMethod2(RexxObjectPtr, rebar_getRect, uint32_t, index, CSELF, pCSelf)
{
    RECT r = {0};

    index--;
    if ( SendMessage(getDChCtrl(pCSelf), RB_GETRECT, index, (LPARAM)&r) )
    {
        return rxNewRect(context, (PORXRECT)&r);
    }
    return TheNilObj;
}

/** ReBar::getRowCount()
 */
RexxMethod1(uint32_t, rebar_getRowCount, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETROWCOUNT, 0, 0);
}

/** ReBar::getRowHeight()
 */
RexxMethod2(uint32_t, rebar_getRowHeight, uint32_t, index, CSELF, pCSelf)
{
    index--;
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETROWHEIGHT, index, 0);
}

/** ReBar::getTextColor()
 *
 *  @return  The current default text color, as a COLORREF.
 */
RexxMethod1(uint32_t, rebar_getTextColor, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_GETTEXTCOLOR, 0, 0);
}

/** ReBar::hitTestInfo()
 *
 *  Determine the location of a point relative to this ReBar control.
 *
 *  @param  pt  [required]  The position, x and y co-ordinates of the point to
 *              test. This can be specified in two forms.
 *
 *      Form 1:  arg 1 is a .Point object.
 *      Form 2:  arg 1 is the x co-ordinate and arg2 is the y co-ordinate.
 *
 *  @param  info  [optional in/out]  A directory object in which all hit test
 *                information is returned.  If the directory is supplied, on
 *                return the directory will have these indexes:
 *
 *                band         Same value as the return.  The one-based index of
 *                             the band if the hit test point hits a band,
 *                             otherwise 0 if the point does not hit a band.
 *
 *                component    A single keyword indicating the rebar band's
 *                             component located at the hit test point.
 *
 *                pt           A point object, the hit test point.
 *
 *  @return  The one-based index of the band hit, or o if the point does not
 *           hit a band.
 *
 *  @note    Sometimes the returned index of the band that is hit is all that is
 *           needed. In these cases, there is no need to supply the optional
 *           directory object.  However, other times the complete hit test
 *           information may be desired.
 *
 *           Any x, y coordinates will work.  I.e. -6000, -7000 will work. The
 *           item will be -1 and location will be "ABOVE TOLEFT"
 */
RexxMethod2(int32_t, rebar_hitTestInfo, ARGLIST, args, CSELF, pCSelf)
{
    RBHITTESTINFO hti = { 0 };
    int32_t result    = 0;

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

    result = (int32_t)SendMessage(getDChCtrl(pCSelf), RB_HITTEST, 0, (LPARAM)&hti);
    result++;

    if ( haveDirectory )
    {
        char buf[256] = {'\0'};

        if      ( hti.flags == RBHT_CAPTION  ) strcpy(buf, "Caption");
        else if ( hti.flags == RBHT_CHEVRON  ) strcpy(buf, "Chevron");
        else if ( hti.flags == RBHT_CLIENT   ) strcpy(buf, "Client");
        else if ( hti.flags == RBHT_GRABBER  ) strcpy(buf, "Grabber");
        else if ( hti.flags == RBHT_NOWHERE  ) strcpy(buf, "Nowhere");
        else if ( hti.flags == RBHT_SPLITTER ) strcpy(buf, "Splitter");

        context->DirectoryPut(info, context->String(buf), "COMPONENT");
        context->DirectoryPut(info, context->Int32(++hti.iBand), "BAND");
        context->DirectoryPut(info, rxNewPoint(context, hti.pt.x, hti.pt.y), "PT");
    }

done_out:
    return result;
}

/** ReBar::idToIndex()
 */
RexxMethod2(uint32_t, rebar_idToIndex, uint32_t, id, CSELF, pCSelf)
{
    return (uint32_t)(SendMessage(getDChCtrl(pCSelf), RB_IDTOINDEX, (WPARAM)id, 0) + 1);
}

/** ReBar::insertBand()
 */
RexxMethod3(logical_t, rebar_insertBand, RexxObjectPtr, bandInfo, OPTIONAL_uint32_t, index, CSELF, pCSelf)
{
    if ( requiredClass(context->threadContext, bandInfo, "REBARBANDINFO", 1) )
    {
        index--;
        return SendMessage(getDChCtrl(pCSelf), RB_INSERTBAND, index, (LPARAM)context->ObjectToCSelf(bandInfo));
    }
    return FALSE;
}

/** ReBar::maximizeBand()
 */
RexxMethod3(RexxObjectPtr, rebar_maximizeBand, uint32_t, index, OPTIONAL_logical_t, useIdeal, CSELF, pCSelf)
{
    index--;
    SendMessage(getDChCtrl(pCSelf), RB_MAXIMIZEBAND, index, useIdeal);
    return TheZeroObj;
}

/** ReBar::minimizeBand()
 */
RexxMethod2(RexxObjectPtr, rebar_minimizeBand, uint32_t, index, CSELF, pCSelf)
{
    index--;
    SendMessage(getDChCtrl(pCSelf), RB_MINIMIZEBAND, index, 0);
    return TheZeroObj;
}

/** ReBar::moveBand()
 *
 *  @return True on success, otherwise false.
 */
RexxMethod3(logical_t, rebar_moveBand, uint32_t, from, uint32_t, to, CSELF, pCSelf)
{
    from--;
    to--;
    return SendMessage(getDChCtrl(pCSelf), RB_MOVEBAND, from, to);
}

/** ReBar::pushChevron()
 *
 *  @return True on success, otherwise false.
 */
RexxMethod3(RexxObjectPtr, rebar_pushChevron, uint32_t, index, OPTIONAL_RexxObjectPtr, appValue, CSELF, pCSelf)
{
    index--;
    SendMessage(getDChCtrl(pCSelf), RB_PUSHCHEVRON, index, (LPARAM)appValue);
    return TheZeroObj;
}

/** ReBar::setBandInfo()
 */
RexxMethod3(logical_t, rebar_setBandInfo, RexxObjectPtr, bandInfo, uint32_t, index, CSELF, pCSelf)
{
    if ( requiredClass(context->threadContext, bandInfo, "REBARBANDINFO", 1) )
    {
        index--;
        return SendMessage(getDChCtrl(pCSelf), RB_SETBANDINFO, index, (LPARAM)context->ObjectToCSelf(bandInfo));
    }
    return FALSE;
}

/** ReBar::setBandWidth()
 */
RexxMethod3(logical_t, rebar_setBandWidth, uint32_t, index, uint32_t, width, CSELF, pCSelf)
{
    if ( requiredOS(context, "setBandWidth", "Vista", Vista_OS) )
    {
        index--;
        return SendMessage(getDChCtrl(pCSelf), RB_SETBANDWIDTH, index, width);
    }
    return FALSE;
}

/** ReBar::setBarInfo()
 *
 *  @note  rxGetImageList() will raise a wrong class exception if the imageList
 *         object is not an ImageList.
 */
RexxMethod2(RexxObjectPtr, rebar_setBarInfo, OPTIONAL_RexxObjectPtr, imageList, CSELF, pCSelf)
{
    RexxObjectPtr ret = TheNilObj;
    REBARINFO     rbi = {sizeof(REBARINFO), RBIM_IMAGELIST, NULL};

    if ( argumentExists(1) && imageList != TheNilObj )
    {
        rbi.himl = rxGetImageList(context, imageList, 1);
        if ( rbi.himl == NULL )
        {
            goto done_out;
        }
    }
    else
    {
        // Be sure imageList is .nil if arg 1 omitted.
        imageList = TheNilObj;
    }

    if ( SendMessage(getDChCtrl(pCSelf), RB_SETBARINFO, 0, (LPARAM)&rbi) )
    {
        ret = rxSetObjVar(context, RB_IMAGELIST_ATTRIB_NAME, imageList);
    }

done_out:
    return ret;
}

/** ReBar::setBkColor()
 *
 *  @param  clr  [optional] The new default background color.  If omitted, the
 *               back ground color is set to CLR_DEFAULT.
 *
 *  @return  The old default background color, as a COLORREF.
 */
RexxMethod2(uint32_t, rebar_setBkColor, OPTIONAL_uint32_t, clr, CSELF, pCSelf)
{
    if ( argumentOmitted(1) )
    {
        clr = CLR_DEFAULT;
    }
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_SETBKCOLOR, 0, clr);
}

/** ReBar::setColorScheme()
 */
RexxMethod3(RexxObjectPtr, rebar_setColorScheme, OPTIONAL_RexxObjectPtr, dirOrCLR, OPTIONAL_uint32_t, btnShadow, CSELF, pCSelf)
{
    COLORSCHEME cs = {sizeof(COLORSCHEME)};
    uint32_t    clr;   // really a COLORREF, but this way we do not need to cast

    RexxMethodContext *c = context;
    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        cs.clrBtnHighlight = CLR_DEFAULT;
        cs.clrBtnShadow    = CLR_DEFAULT;
    }
    else if ( argumentExists(1) && argumentExists(2) )
    {
        if ( ! c->UnsignedInt32(dirOrCLR, &clr) )
        {
            invalidTypeException(c->threadContext, 1, "COLORREF", dirOrCLR);
            goto done_out;
        }

        cs.clrBtnHighlight = clr;
        cs.clrBtnShadow    = btnShadow;
    }
    else if ( argumentExists(1) && argumentOmitted(2) )
    {
        if ( c->IsDirectory(dirOrCLR) )
        {
            RexxDirectoryObject d = (RexxDirectoryObject)dirOrCLR;

            if ( ! rxNumberFromDirectory(context, d, "CLRBTNHIGHLIGHT", &clr, 1, true) )
            {
                goto done_out;
            }
            cs.clrBtnHighlight = clr;

            if ( ! rxNumberFromDirectory(context, d, "CLRBTNSHADOW", &clr, 1, true) )
            {
                goto done_out;
            }
            cs.clrBtnShadow = clr;
        }
        else
        {
            if ( ! c->UnsignedInt32(dirOrCLR, &clr) )
            {
                invalidTypeException(c->threadContext, 1, "COLORREF or Directory object", dirOrCLR);
                goto done_out;
            }
            cs.clrBtnHighlight = clr;
            cs.clrBtnShadow    = CLR_DEFAULT;
        }
    }
    else
    {
        // argument 1 omitted and argument 2 exists.
        cs.clrBtnHighlight = CLR_DEFAULT;
        cs.clrBtnShadow    = btnShadow;
    }

    SendMessage(getDChCtrl(pCSelf), RB_SETCOLORSCHEME, 0, (LPARAM)&cs);

done_out:
    return TheZeroObj;
}

/** ReBar::setExtendedStyle()
 */
RexxMethod3(RexxObjectPtr, rebar_setExtendedStyle, OPTIONAL_CSTRING, _mask, OPTIONAL_CSTRING, _style, CSELF, pCSelf)
{
    if ( requiredOS(context, "setExtendedStyle", "Vista", Vista_OS) )
    {
        uint32_t mask     = keyword2rbsExt(_mask);
        uint32_t style    = keyword2rbsExt(_style);
        uint32_t oldStyle = (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_SETEXTENDEDSTYLE, style, mask);

        return rbsExt2keyword(context, oldStyle);
    }
    return NULLOBJECT;
}

/** ReBar::setPalette()
 */
RexxMethod2(POINTER, rebar_setPalette, RexxObjectPtr, _hPalette, CSELF, pCSelf)
{
    void *hp;
    if ( ! oodObj2handle(context, _hPalette, &hp, 1) )
    {
        return NULLOBJECT;
    }

    HPALETTE oldHp = (HPALETTE)SendMessage(getDChCtrl(pCSelf), RB_GETPALETTE, 0, (LPARAM)hp);
    return oldHp;
}

/** ReBar::setParent()
 */
RexxMethod2(RexxObjectPtr, rebar_setParent, RexxObjectPtr, dlg, CSELF, pCSelf)
{
    RexxObjectPtr     result = TheNilObj;
    pCPlainBaseDialog pcpbd  = requiredPlainBaseDlg(context, dlg, 1);
    if ( pcpbd != NULL )
    {
        HWND hwndOldParent = (HWND)SendMessage(getDChCtrl(pCSelf), RB_SETPARENT, (WPARAM)pcpbd->hDlg, 0);
        if ( hwndOldParent != NULL )
        {
            pCPlainBaseDialog oldPcpbd = (pCPlainBaseDialog)getWindowPtr(hwndOldParent, GWLP_USERDATA);
            if ( oldPcpbd != NULL )
            {
                result = oldPcpbd->rexxSelf;
            }
        }
    }

    return result;
}

/** ReBar::setTextColor()
 *
 *  @param  color  A COLORREF that is used to define the new default text color.
 *
 *  @return  The previous default text color, as a COLORREF.
 */
RexxMethod2(uint32_t, rebar_setTextColor, uint32_t, color, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), RB_SETTEXTCOLOR, 0, (LPARAM)color);
}

/** ReBar::setWindowTheme()
 */
RexxMethod2(RexxObjectPtr, rebar_setWindowTheme, CSTRING, _theme, CSELF, pCSelf)
{
    if ( requiredComCtl32Version(context, "setWindowTheme", COMCTL32_6_0) )
    {
        LPWSTR theme = ansi2unicode(_theme);
        if ( theme != NULL )
        {
            SendMessage(getDChCtrl(pCSelf), RB_SETWINDOWTHEME, 0, (LPARAM)theme);
            LocalFree(theme);
        }
    }

    return TheZeroObj;
}

/** ReBar::showBand()
 *
 *  Shows or hides the specified band.
 *
 *  @param   index [required]  The one-based index of the band to show or hide.
 *
 *  @param   show [optional] If show is true, the specified band is made
 *                visible, otherwise the band is hidden.  If show is omitted,
 *                the band is hidden.
 *
 *  @return  True on success, othewise false.
 */
RexxMethod3(logical_t, rebar_showBand, uint32_t, index, OPTIONAL_logical_t, show, CSELF, pCSelf)
{
    index--;
    return SendMessage(getDChCtrl(pCSelf), RB_SHOWBAND, index, show);
}

/** ReBar::sizeToRect()
 *
 *  Attempts to find the best layout of the bands for the given rectangle.
 *
 *  @param rect  [required]  A .Rect object that specifies the rectangle the
 *               ReBar control should be resized to.
 *
 *  @return  True if the layout of the ReBar was changed, otherwise false.
 *
 *  @note  The ReBar bands will be arranged and wrapped as necessary to fit the
 *         rectangle. Bands that have the VARIABLEHEIGHT style will be resized
 *         as evenly as possible to fit the rectangle. The height of a
 *         horizontal ReBar or the width of a vertical ReBar may change,
 *         depending on the new layout
 *
 */
RexxMethod2(logical_t, rebar_sizeToRect, RexxObjectPtr, _rect, CSELF, pCSelf)
{
    PRECT pRect = (PRECT)rxGetRect(context, _rect, 1);
    if ( pRect != NULL )
    {
        return SendMessage(getDChCtrl(pCSelf), RB_SIZETORECT, 0, (LPARAM)pRect);
    }

    return FALSE;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * ooShapes.cpp
 *
 * Contains the implmentation of the Point, Size, and XX classes.  Plus
 * convenience / helper functions used for working with those classess within
 * the ooRexx Native API.
 */

#ifdef _WIN32

    #define NTDDI_VERSION   NTDDI_LONGHORN
    #define _WIN32_WINNT    0x0600
    #define _WIN32_IE       0x0600
    #define WINVER          0x0501

    #define STRICT
    #define OEMRESOURCE

    #include <windows.h>

    #define strcasecmp                 _stricmp
    #define strncasecmp                _strnicmp

#else
    #include <ctype.h>

    #define TRUE   1
    #define FALSE  0
#endif

#include "oorexxapi.h"

#include <stdio.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"


static RexxStringObject genGetVersion(RexxMethodContext *c, CSTRING name, bool full, bool minimal)
{
    char   buf[512] = {'\0'};
    size_t bits = 32;

#ifdef __REXX64__
    bits = 64;
#endif

    if ( full )
    {
        size_t rx = c->InterpreterVersion();

#define QUOTE(arg) #arg
#define QUOTED(name) QUOTE(name)
        snprintf(buf, sizeof(buf), "%s\n"
                                   "      Version %d.%d.%d.%d (%zd bit)\n"
                                   "      Built %s %s\n"
                                   "      Copyright (c) %s Rexx Language Association.\n"
                                   "      All rights reserved.\n\n"
                                   "Rexx: Open Object Rexx Version %zd.%zd.%zd\n\n",
                 name,
                 OOSHAPES_VER_MAJOR, OOSHAPES_VER_MINOR, OOSHAPES_VER_LEVEL, OOSHAPES_VER_BUILD, bits,
                 __DATE__, __TIME__, QUOTED(OOSHAPES_COPYRIGHT_YEAR),
                 (rx >> 16) & 0xff, (rx >> 8) & 0xff, rx & 0x0000ff);
    }
    else
    {
        if ( minimal )
        {
            snprintf(buf, sizeof(buf), "%d.%d.%d.%d\n",
                     OOSHAPES_VER_MAJOR, OOSHAPES_VER_MINOR, OOSHAPES_VER_LEVEL, OOSHAPES_VER_BUILD);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%s Version %d.%d.%d.%d (%zd bit)\n",
                     name, OOSHAPES_VER_MAJOR, OOSHAPES_VER_MINOR, OOSHAPES_VER_LEVEL,
                     OOSHAPES_VER_BUILD, bits);
        }
    }

    return c->String(buf);
}

/** version  [class method]
 *
 *  @param type  [optional]  The style of the version output returned.  Keywords
 *               are, but only 1st letter is required:
 *
 *                 Compact
 *                 Full
 *                 OneLine
 *
 *               The defualt is OneLine
 *
 * @note This method is mapped, as a class method, to each of the .Point, .Rect,
 *       and .Size classes.  It reports the package version actually.
 *
 */
RexxMethod2(RexxObjectPtr, ooShapes_version_cls, OPTIONAL_CSTRING, type, OSELF, self)
{
    RexxObjectPtr rxName = context->SendMessage0(self, "DEFAULTNAME");
    CSTRING       name   = context->ObjectToStringValue(rxName);

    if ( argumentExists(1) )
    {
        switch ( toupper(*type) )
        {
            case 'O' :
                return genGetVersion(context, name, false, false);
                break;

            case 'F' :
                return genGetVersion(context, name, true, false);
                break;

            case 'C' :
                return genGetVersion(context, name, false, true);
                break;

            default :
                wrongArgKeywordException(context, 1, "Compact, Full, or Oneline", type);
                return TheZeroObj;
                break;
        }
    }

    return genGetVersion(context, name, false, false);
}



/**
 * Methods for the .Point class.
 */
#define POINT_CLASS  "Point"

RexxMethod2(RexxObjectPtr, point_init, OPTIONAL_int32_t,  x, OPTIONAL_int32_t, y)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(POINT));
    context->SetObjectVariable("CSELF", obj);

    ORXPOINT *p = (ORXPOINT *)context->BufferData(obj);
    p->x = argumentExists(1) ? x : 0;
    p->y = argumentExists(2) ? y : p->x;

    return NULLOBJECT;
}

RexxMethod1(int32_t, point_x, CSELF, p) { return ((ORXPOINT *)p)->x; }
RexxMethod1(int32_t, point_y, CSELF, p) { return ((ORXPOINT *)p)->y; }
RexxMethod2(RexxObjectPtr, point_setX, CSELF, p, int32_t, x) { ((ORXPOINT *)p)->x = x; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, point_setY, CSELF, p, int32_t, y) { ((ORXPOINT *)p)->y = y; return NULLOBJECT; }

/** Point::copy()
 *
 *  Returns a new point object that is a copy of this point.
 *
 */
RexxMethod1(RexxObjectPtr, point_copy, CSELF, p)
{
    ORXPOINT *_p = (ORXPOINT *)p;
    return rxNewPoint(context, _p->x, _p->y);
}

/** Point::+
 *
 *  Returns a new point object that is the result of "adding" two points.
 *
 */
RexxMethod2(RexxObjectPtr, point_add, RexxObjectPtr, other, CSELF, p)
{
    if ( ! context->IsOfType(other, "POINT") )
    {
        wrongClassException(context->threadContext, 1, "Point");
        return NULLOBJECT;
    }

    ORXPOINT *p1 = (ORXPOINT *)p;
    ORXPOINT *p2 = (ORXPOINT *)context->ObjectToCSelf(other);

    return rxNewPoint(context, p1->x + p2->x, p1->y + p2->y);
}

/** Point::-
 *
 *  Returns a new point object that is the result of "subtracting" two points.
 *
 */
RexxMethod2(RexxObjectPtr, point_subtract, RexxObjectPtr, other, CSELF, p)
{
    if ( ! context->IsOfType(other, "POINT") )
    {
        wrongClassException(context->threadContext, 1, "Point");
        return NULLOBJECT;
    }

    ORXPOINT *p1 = (ORXPOINT *)p;
    ORXPOINT *p2 = (ORXPOINT *)context->ObjectToCSelf(other);

    return rxNewPoint(context, p1->x - p2->x, p1->y - p2->y);
}

/** Point::incr
 *
 *  Increments this point's x and y attributes by the specified amount. If both
 *  optional args are ommitted, the x and y are incremented by 1.
 *
 *  @param  x  The amount to increment, (to add to,) this point's x attribute.
 *             If y is specified and this arg is omitted than 0 is used.
 *
 *  @param  y  The amount to increment, (to add to,) this point's y attribute.
 *             If x is specified and this arg is omitted than 0 is used.
 *
 *  @return  No return.
 *
 *  @remarks  If either x or y are omitted, then their value will be 0.  Since
 *            that is the default, once we check that both are not ommitted, we
 *            are safe to just add x and y to the current x and y.
 */
RexxMethod3(RexxObjectPtr, point_incr, OPTIONAL_int32_t, x, OPTIONAL_int32_t, y, CSELF, p)
{
    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        ((ORXPOINT *)p)->x++;
        ((ORXPOINT *)p)->y++;
    }
    else
    {
        ((ORXPOINT *)p)->x += x;
        ((ORXPOINT *)p)->y += y;
    }

    return NULLOBJECT;
}

/** Point::decr
 *
 *  Decrements this point's x and y attributes by the specified amount. See the
 *  comments above for Point::incr() for details.
 */
RexxMethod3(RexxObjectPtr, point_decr, OPTIONAL_int32_t, x, OPTIONAL_int32_t, y, CSELF, p)
{
    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        ((ORXPOINT *)p)->x--;
        ((ORXPOINT *)p)->y--;
    }
    else
    {
        ((ORXPOINT *)p)->x -= x;
        ((ORXPOINT *)p)->y -= y;
    }

    return NULLOBJECT;
}

/** Point::inRect
 *
 *  Determines if this point is in the specified rectangle.  The rectangle must
 *  be normalized, that is, rect.right must be greater than rect.left and
 *  rect.bottom must be greater than rect.top. If the rectangle is not
 *  normalized, a point is never considered inside of the rectangle.
 */
RexxMethod2(logical_t, point_inRect, RexxObjectPtr, rect, CSELF, p)
{
    PORXRECT r = rxGetRect(context, rect, 1);
    if ( r != NULL && rxIsNormalized(r) )
    {
        ORXPOINT pt = {((ORXPOINT *)p)->x, ((ORXPOINT *)p)->y};
        return rxPtInRect(r, &pt);
    }

    return FALSE;
}

RexxMethod1(RexxStringObject, point_string, CSELF, p)
{
    PORXPOINT pt = (PORXPOINT)p;

    TCHAR buf[128];
    snprintf(buf, sizeof(buf), "a Point (%d, %d)", pt->x, pt->y);

    return context->String(buf);
}

RexxMethod1(RexxStringObject, point_print, CSELF, p)
{
    PORXPOINT pt = (PORXPOINT)p;

    TCHAR buf[128];
    snprintf(buf, sizeof(buf), "(%d, %d)", pt->x, pt->y);

    return context->String(buf);
}


/**
 * Methods for the .Size class.
 */
#define SIZE_CLASS  "Size"

RexxMethod2(RexxObjectPtr, size_init, OPTIONAL_int32_t, cx, OPTIONAL_int32_t, cy)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(ORXSIZE));
    context->SetObjectVariable("CSELF", obj);

    ORXSIZE *s = (ORXSIZE *)context->BufferData(obj);

    s->cx = argumentExists(1) ? cx : 0;
    s->cy = argumentExists(2) ? cy : s->cx;

    return NULLOBJECT;
}

RexxMethod1(int32_t, size_cx, CSELF, s) { return ((ORXSIZE *)s)->cx; }
RexxMethod1(int32_t, size_cy, CSELF, s) { return ((ORXSIZE *)s)->cy; }
RexxMethod2(RexxObjectPtr, size_setCX, CSELF, s, int32_t, cx) { ((ORXSIZE *)s)->cx = cx; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, size_setCY, CSELF, s, int32_t, cy) { ((ORXSIZE *)s)->cy = cy; return NULLOBJECT; }

RexxMethod1(RexxStringObject, size_string, CSELF, s)
{
    PORXSIZE size = (PORXSIZE)s;

    TCHAR buf[128];
    snprintf(buf, sizeof(buf), "a Size (%d, %d)", size->cx, size->cy);

    return context->String(buf);
}

RexxMethod1(RexxStringObject, size_print, CSELF, s)
{
    PORXSIZE size = (PORXSIZE)s;

    TCHAR buf[128];
    snprintf(buf, sizeof(buf), "(%d, %d)", size->cx, size->cy);

    return context->String(buf);
}

RexxMethod2(logical_t, size_equateTo, RexxObjectPtr, other, CSELF, s)
{
    PORXSIZE size = (PORXSIZE)s;
    PORXSIZE pOther = rxGetSize(context, other, 1);
    if ( pOther == NULL )
    {
        return FALSE;
    }

    size->cx = pOther->cx;
    size->cy = pOther->cy;

    return TRUE;
}

RexxMethod3(logical_t, size_compare, RexxObjectPtr, other, NAME, method, CSELF, s)
{
    PORXSIZE size = (PORXSIZE)s;
    PORXSIZE pOther = rxGetSize(context, other, 1);
    if ( pOther == NULL )
    {
        return 0;
    }

    if ( strcmp(method, "=") == 0 )
    {
        return (size->cx * size->cy) == (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "==") == 0 )
    {
        return (size->cx == pOther->cx) && (size->cy == pOther->cy);
    }
    else if ( strcmp(method, "\\=") == 0 )
    {
        return (size->cx * size->cy) != (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "\\==") == 0 )
    {
        return (size->cx != pOther->cx) && (size->cy != pOther->cy);
    }
    else if ( strcmp(method, "<") == 0 )
    {
        return (size->cx * size->cy) < (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "<<") == 0 )
    {
        return (size->cx < pOther->cx) && (size->cy < pOther->cy);
    }
    else if ( strcmp(method, "<=") == 0 )
    {
        return (size->cx * size->cy) <= (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "<<=") == 0 )
    {
        return (size->cx <= pOther->cx) && (size->cy <= pOther->cy);
    }
    else if ( strcmp(method, ">") == 0 )
    {
        return (size->cx * size->cy) > (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, ">>") == 0 )
    {
        return (size->cx > pOther->cx) && (size->cy > pOther->cy);
    }
    else if ( strcmp(method, ">=") == 0 )
    {
        return (size->cx * size->cy) >= (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, ">>=") == 0 )
    {
        return (size->cx >= pOther->cx) && (size->cy >= pOther->cy);
    }

    return FALSE;
}

/**
 * Methods for the .Rect class.
 */
#define RECT_CLASS  "Rect"

RexxMethod4(RexxObjectPtr, rect_init, OPTIONAL_int32_t, left, OPTIONAL_int32_t, top,
            OPTIONAL_int32_t, right, OPTIONAL_int32_t, bottom)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(ORXRECT));
    context->SetObjectVariable("CSELF", obj);

    ORXRECT *r = (ORXRECT *)context->BufferData(obj);

    r->left = argumentExists(1) ? left : 0;
    r->top = argumentExists(2) ? top : r->left;
    r->right = argumentExists(3) ? right : r->left;
    r->bottom = argumentExists(4) ? bottom : r->left;

    return NULLOBJECT;
}

RexxMethod1(int32_t, rect_left, CSELF, pRect) { return ((ORXRECT *)pRect)->left; }
RexxMethod1(int32_t, rect_top, CSELF, pRect) { return ((ORXRECT *)pRect)->top; }
RexxMethod1(int32_t, rect_right, CSELF, pRect) { return ((ORXRECT *)pRect)->right; }
RexxMethod1(int32_t, rect_bottom, CSELF, pRect) { return ((ORXRECT *)pRect)->bottom; }
RexxMethod2(RexxObjectPtr, rect_setLeft, CSELF, pRect, int32_t, left) { ((ORXRECT *)pRect)->left = left; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setTop, CSELF, pRect, int32_t, top) { ((ORXRECT *)pRect)->top = top; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setRight, CSELF, pRect, int32_t, right) { ((ORXRECT *)pRect)->right = right; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setBottom, CSELF, pRect, int32_t, bottom) { ((ORXRECT *)pRect)->bottom = bottom; return NULLOBJECT; }

/** Rect::copy()
 *
 *  Returns a new Rect object that is a copy of this Rect.
 *
 */
RexxMethod1(RexxObjectPtr, rect_copy, CSELF, pRect)
{
    PORXRECT pR = (PORXRECT)pRect;
    return rxNewRect(context, pR->left, pR->top, pR->right, pR->bottom);
}

RexxMethod1(RexxStringObject, rect_string, CSELF, pRect)
{
    PORXRECT pR = (PORXRECT)pRect;

    TCHAR buf[128];
    snprintf(buf, sizeof(buf), "a Rect (%d, %d, %d, %d)", pR->left, pR->top, pR->right, pR->bottom);

    return context->String(buf);
}

RexxMethod1(RexxStringObject, rect_print, CSELF, pRect)
{
    PORXRECT pR = (PORXRECT)pRect;

    TCHAR buf[128];
    snprintf(buf, sizeof(buf), "(%d, %d, %d, %d)", pR->left, pR->top, pR->right, pR->bottom);

    return context->String(buf);
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * ooShapesAid.cpp
 *
 * Contains the convenience / helper functions used for working with the ooRexx
 * .Point, .Size. and .Rect classess within the ooRexx Native API.
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

// Initialized when / if needed.
RexxClassObject ThePointClass = NULLOBJECT;
RexxClassObject TheSizeClass  = NULLOBJECT;
RexxClassObject TheRectClass  = NULLOBJECT;

static void getClasses(RexxThreadContext *c)
{
    TheRectClass  = rxGetPackageClass(c, "ooShapes.cls", "RECT");
    TheSizeClass  = rxGetPackageClass(c, "ooShapes.cls", "SIZE");
    ThePointClass = rxGetPackageClass(c, "ooShapes.cls", "POINT");
    c->RequestGlobalReference(TheRectClass);
    c->RequestGlobalReference(TheSizeClass);
    c->RequestGlobalReference(ThePointClass);
}

static void getClasses(RexxMethodContext *c)
{
    TheRectClass  = rxGetContextClass(c, "RECT");
    TheSizeClass  = rxGetContextClass(c, "SIZE");
    ThePointClass = rxGetContextClass(c, "POINT");
    c->RequestGlobalReference(TheRectClass);
    c->RequestGlobalReference(TheSizeClass);
    c->RequestGlobalReference(ThePointClass);
}

__declspec(dllexport) bool rxCopyRect(PORXRECT rect, PORXRECT r)
{
    if ( rect == NULL || r == NULL )
    {
        return false;
    }
    rect->left   = r->left;
    rect->top    = r->top;
    rect->right  = r->right;
    rect->bottom = r->bottom;
    return true;
}

__declspec(dllexport) bool rxSetRect(PORXRECT rect, long x, long y, long x2, long y2)
{
    if ( rect == NULL )
    {
        return false;
    }
    rect->left   = x;
    rect->top    = y;
    rect->right  = x2;
    rect->bottom = y2;
    return true;
}

__declspec(dllexport) bool rxPtInRect(PORXRECT r, PORXPOINT pt)
{
    if ( pt->x > r->top && pt->x < r->bottom && pt->y > r->left && pt->y < r->top )
    {
        return true;
    }
    return false;
}

__declspec(dllexport) bool rxIsNormalized(PORXRECT r)
{
    if ( r->right > r->left && r->bottom > r->top )
    {
        return true;
    }
    return false;
}

__declspec(dllexport) PORXPOINT rxGetPoint(RexxMethodContext *context, RexxObjectPtr p, size_t argPos)
{
    if ( requiredClass(context->threadContext, p, "Point", argPos) )
    {
        return (PORXPOINT)context->ObjectToCSelf(p);
    }
    return NULL;
}


__declspec(dllexport) RexxObjectPtr rxNewPoint(RexxThreadContext *c, long x, long y)
{
    if ( ThePointClass == NULLOBJECT )
    {
        getClasses(c);
    }
    return c->SendMessage2(ThePointClass, "NEW", c->WholeNumber(x), c->WholeNumber(y));;
}

__declspec(dllexport) RexxObjectPtr rxNewPoint(RexxMethodContext *c, long x, long y)
{
    if ( ThePointClass == NULLOBJECT )
    {
        getClasses(c);
    }
    return c->SendMessage2(ThePointClass, "NEW", c->WholeNumber(x), c->WholeNumber(y));;
}

__declspec(dllexport) RexxObjectPtr rxNewPoint(RexxThreadContext *c, ORXPOINT *pt)
{
    return rxNewPoint(c, pt->x, pt->y);
}

__declspec(dllexport) RexxObjectPtr rxNewPoint(RexxMethodContext *c, ORXPOINT *pt)
{
    return rxNewPoint(c->threadContext, pt->x, pt->y);
}

__declspec(dllexport) PORXRECT rxGetRect(RexxMethodContext *context, RexxObjectPtr r, size_t argPos)
{
    if ( requiredClass(context->threadContext, r, "Rect", argPos) )
    {
        return (PORXRECT)context->ObjectToCSelf(r);
    }
    return NULL;
}


__declspec(dllexport) RexxObjectPtr rxNewRect(RexxThreadContext *c, PORXRECT r)
{
    if ( TheRectClass == NULLOBJECT )
    {
        getClasses(c);
    }
    RexxArrayObject args = c->ArrayOfFour(c->WholeNumber(r->left),
                                          c->WholeNumber(r->top),
                                          c->WholeNumber(r->right),
                                          c->WholeNumber(r->bottom));

    return c->SendMessage(TheRectClass, "NEW", args);
}


__declspec(dllexport) RexxObjectPtr rxNewRect(RexxMethodContext *c, long l, long t, long r, long b)
{
    if ( TheRectClass == NULLOBJECT )
    {
        getClasses(c);
    }
    RexxArrayObject args = c->ArrayOfFour(c->WholeNumber(l),
                                          c->WholeNumber(t),
                                          c->WholeNumber(r),
                                          c->WholeNumber(b));

    return c->SendMessage(TheRectClass, "NEW", args);
}


__declspec(dllexport) RexxObjectPtr rxNewRect(RexxMethodContext *context, PORXRECT r)
{
    return rxNewRect(context, r->left, r->top, r->right, r->bottom);
}

__declspec(dllexport) PORXSIZE rxGetSize(RexxMethodContext *context, RexxObjectPtr s, size_t argPos)
{
    if ( requiredClass(context->threadContext, s, "Size", argPos) )
    {
        return (PORXSIZE)context->ObjectToCSelf(s);
    }
    return NULL;
}

__declspec(dllexport) RexxObjectPtr rxNewSize(RexxThreadContext *c, long cx, long cy)
{
    if ( TheSizeClass == NULLOBJECT )
    {
        getClasses(c);
    }
    return c->SendMessage2(TheSizeClass, "NEW", c->WholeNumber(cx), c->WholeNumber(cy));
}

__declspec(dllexport) RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy)
{
    if ( TheSizeClass == NULLOBJECT )
    {
        getClasses(c);
    }
    return c->SendMessage2(TheSizeClass, "NEW", c->WholeNumber(cx), c->WholeNumber(cy));
}

__declspec(dllexport) RexxObjectPtr rxNewSize(RexxMethodContext *c, PORXSIZE s)
{
    return rxNewSize(c, s->cx, s->cx);
}

/**
 * Checks that an argument array contains the minimum, and not more than the
 * maximum, number of arguments.  Raises the appropriate exception if the check
 * fails.
 *
 * In addition the actual size of the argument array is returned.
 *
 * @param c         Method context we are operating in.
 * @param args      Argument array.
 * @param min       Minimum expected number of arguments.
 * @param max       Maximum number of arguments allowed.
 * @param arraySize [out] Size of argument array.
 *
 * @return True if the check succeeds, otherwise false.  If false, an exception
 *         has been raised.
 */
__declspec(dllexport) bool goodMinMaxArgs(RexxMethodContext *c, RexxArrayObject args, size_t min, size_t max, size_t *arraySize)
{
    *arraySize = c->ArraySize(args);
    if ( *arraySize > max )
    {
        tooManyArgsException(c->threadContext, max);
        return false;
    }
    if ( *arraySize < min )
    {
        missingArgException(c->threadContext, min);
        return false;
    }
    return true;
}

/**
 * Fills in a RECT structure using an argument array passed to a Rexx object
 * method.
 *
 * The purpose is to give the Rexx programmer some flexibility in how they pass
 * in "rectangle-like" coordinates to a method.
 *
 * The coordinates can be expressed as a .Rect, as a .Point and a .Size, as a
 * .Point and a .Point, or as 4 individual intergers.
 *
 * There are also two basic scenarios where this is used:
 *
 * 1.) A bounding rectangle: x1, y1, x2, y2  With a bounding rectangle x1 and y1
 * specify the upper-left corner of the rectangle. x2 and y2 specify the
 * lower-bottom corner of the rectangle.  In this scenario, the Rexx programmer
 * can use .Point .Point, but not .Point .Size.
 *
 * 2.) A point / size rectangle.  In this case the first two args specify the
 * upper-left corner of the rectangle, and the third and forth args specify the
 * width and height of the rectangle.  In this scenario, the Rexx programmer can
 * use .Point and .Size, but not .Point and .Point
 *
 * In either case, .Rect and 4 indvidual integers are taken at face value.
 *
 * @param c            Method context we are operating in.
 * @param args         The arg list array (ARGLIST) passed to the native API
 * @param rect         [IN/OUT] Pointer to a rect struct, this is filled in on
 *                     success.
 * @param boundingRect True if rect should be interpreted as a bounding
 *                     rectangle, false if rect should be interpreted as a
 *                     point/size rectangle.
 * @param startArg     The argument number in the arg array where the rectangle
 *                     specifications start.
 * @param maxArgs      The maximum number of args allowed.
 * @param arraySize    [IN/OUT] The size of the argument array, returned.
 * @param usedArgs     [IN/OUT] The number of arguments used in specifying the
 *                     rectangle. I.e., if startArg is a .Rect, then usedArgs
 *                     will be 1 on return.  If at startArg we have x, y, cx, cy
 *                     then useArgs will be 4 on return.
 *
 * @return True on success, false otherwise.  If the return is false, an
 *         exception has been raised.
 */
__declspec(dllexport) bool getRectFromArglist(RexxMethodContext *c, RexxArrayObject args, PORXRECT rect, bool boundingRect,
                        int startArg, int maxArgs, size_t *arraySize, size_t *usedArgs)
{
    if ( ! goodMinMaxArgs(c, args, startArg, maxArgs, arraySize) )
    {
        goto err_out;
    }

    RexxObjectPtr obj1 = c->ArrayAt(args, startArg);
    if ( obj1 == NULLOBJECT )
    {
        missingArgException(c->threadContext, startArg);
        goto err_out;
    }

    if ( c->IsOfType(obj1, "RECT") )
    {
        PORXRECT r = rxGetRect(c, obj1, startArg);
        if ( r == NULL )
        {
            goto err_out;
        }
        rxCopyRect(rect, r);
        *usedArgs = 1;
    }
    else if ( c->IsOfType(obj1, "POINT") )
    {
        PORXPOINT p = rxGetPoint(c, obj1, startArg);
        if ( p == NULL )
        {
            goto err_out;
        }

        RexxObjectPtr obj2 = c->ArrayAt(args, startArg + 1);
        if ( obj2 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 1);
            goto err_out;
        }

        // If it is a bounding rectangle, the second object has to be a .Point
        // object. Otherwise, the second object has to be a .Size object
        if ( boundingRect )
        {
            PORXPOINT p2 = rxGetPoint(c, obj2, startArg + 1);
            if ( p2 == NULL )
            {
                goto err_out;
            }
            rxSetRect(rect, p->x, p->y, p2->x, p2->y);
        }
        else
        {
            PORXSIZE s = rxGetSize(c, obj2, startArg + 1);
            if ( s == NULL )
            {
                goto err_out;
            }
            rxSetRect(rect, p->x, p->y, s->cx, s->cy);
        }
        *usedArgs = 2;
    }
    else
    {
        int x, y, cx, cy;

        if ( ! c->Int32(obj1, &x) )
        {
            wrongRangeException(c->threadContext, startArg, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 1);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 1);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &y) )
        {
            wrongRangeException(c->threadContext, startArg + 1, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 2);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 2);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &cx) )
        {
            wrongRangeException(c->threadContext, startArg + 2, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 3);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 3);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &cy) )
        {
            wrongRangeException(c->threadContext, startArg + 3, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }
        rxSetRect(rect, x, y, cx, cy);
        *usedArgs = 4;
    }
    return true;

err_out:
    return false;
}


/**
 * Fills in a ORXPOINT structure using an argument array passed to a Rexx object
 * method.
 *
 * The purpose is to give the Rexx programmer some flexibility in how they pass
 * in "point-like" coordinates to a method.
 *
 * The coordinates can be expressed as a .Point, a .Size, or as 2 individual
 * intergers.
 *
 * Since a point and a size are binary compatible, no effort is made to enforce
 * that only a point is used.  This makes things more flexible.
 *
 * @param c            Method context we are operating in.
 * @param args         The arg list array (ARGLIST) passed to the native API
 * @param point        [IN/OUT] Pointer to a point struct, this is filled in on
 *                     success.
 * @param startArg     The argument number in the arg array where the point
 *                     specifications start.
 * @param maxArgs      The maximum number of args allowed.
 * @param arraySize    [IN/OUT] The size of the argument array, returned.
 * @param usedArgs     [IN/OUT] The number of arguments used in specifying the
 *                     point. I.e., if startArg is a .point, then usedArgs
 *                     will be 1 on return.  If at startArg we have x, y, (or
 *                     cx, cy) then useArgs will be 2 on return.
 *
 * @return True on success, false otherwise.  If the return is false, an
 *         exception has been raised.
 */
__declspec(dllexport) bool getPointFromArglist(RexxMethodContext *c, RexxArrayObject args, PORXPOINT point, int startArg, int maxArgs,
                         size_t *arraySize, size_t *usedArgs)
{
    if ( ! goodMinMaxArgs(c, args, startArg, maxArgs, arraySize) )
    {
        goto err_out;
    }

    RexxObjectPtr obj1 = c->ArrayAt(args, startArg);
    if ( obj1 == NULLOBJECT )
    {
        missingArgException(c->threadContext, startArg);
        goto err_out;
    }

    if ( c->IsOfType(obj1, "POINT") )
    {
        PORXPOINT p = rxGetPoint(c, obj1, startArg);
        if ( p == NULL )
        {
            goto err_out;
        }
        point->x = p->x;
        point->y = p->y;
        *usedArgs = 1;
    }
    else if ( c->IsOfType(obj1, "SIZE") )
    {
        PORXSIZE s = rxGetSize(c, obj1, startArg);
        if ( s == NULL )
        {
            goto err_out;
        }
        point->x = s->cx;
        point->y = s->cy;
        *usedArgs = 1;
    }
    else
    {
        int x, y;
        if ( ! c->Int32(obj1, &x) )
        {
            wrongRangeException(c->threadContext, startArg, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 1);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 1);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &y) )
        {
            wrongRangeException(c->threadContext, startArg + 1, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }
        point->x = x;
        point->y = y;
        *usedArgs = 2;
    }
    return true;

err_out:
    return false;
}

/**
 * Fills in a SIZE structure using an argument array passed to a Rexx object
 * method.
 *
 * The purpose is to give the Rexx programmer some flexibility in how they pass
 * in "point-like" coordinates to a method.
 *
 * This function uses getPointFromArgList() to do the work.  It is a convenience
 * function for the ooDialog developer.
 *
 * @param c            Method context we are operating in.
 * @param args         The arg list array (ARGLIST) passed to the native API
 * @param point        [IN/OUT] Pointer to a point struct, this is filled in on
 *                     success.
 * @param startArg     The argument number in the arg array where the point
 *                     specifications start.
 * @param maxArgs      The maximum number of args allowed.
 * @param arraySize    [IN/OUT] The size of the argument array, returned.
 * @param usedArgs     [IN/OUT] The number of arguments used in specifying the
 *                     point. I.e., if startArg is a .point, then usedArgs
 *                     will be 1 on return.  If at startArg we have x, y, (or
 *                     cx, cy) then useArgs will be 2 on return.
 *
 * @return True on success, false otherwise.  If the return is false, an
 *         exception has been raised.
 */
__declspec(dllexport) bool getSizeFromArglist(RexxMethodContext *c, RexxArrayObject args, PORXPOINT point, int startArg, int maxArgs,
                        size_t *arraySize, size_t *usedArgs)
{
    return getPointFromArglist(c, args, point, startArg, maxArgs, arraySize, usedArgs);
}


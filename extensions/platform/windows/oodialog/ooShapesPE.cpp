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
 * ooShapesPE.cpp
 *
 * The package entry code for .SimpleFigures class / package.
 *
 * Also contains DLLMain(), a Windows only costruct.
 */

#ifdef _WIN32
    #include "stdio.h"
    #define NTDDI_VERSION   NTDDI_LONGHORN
    #define _WIN32_WINNT    0x0600
    #define _WIN32_IE       0x0600
    #define WINVER          0x0501

    #define STRICT
    #define OEMRESOURCE

    #include <windows.h>

    #ifdef __cplusplus
    #define BEGIN_EXTERN_C() extern "C" {
    #define END_EXTERN_C() }
    #else
    #define BEGIN_EXTERN_C()
    #define END_EXTERN_C()
    #endif


    BEGIN_EXTERN_C()

    BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
    {
        if ( fdwReason == DLL_PROCESS_ATTACH )
        {
            ; // Don't need to do anything
        }
        else if ( fdwReason == DLL_PROCESS_DETACH )
        {
            ; // Don't need to do anything
        }
        return(TRUE);
    }

    END_EXTERN_C()

#endif

#include "oorexxapi.h"
#include "APICommon.hpp"


/**
 * RexxPackageLoader function.
 *
 * The package loader function is called when the library package is first
 * loaded.  This makes it the ideal place for any initialization that must be
 * done prior to the Rexx program start up.  We call sqlite3_initialize() and
 * set up a few global values.
 *
 * @param c  Thread context pointer passed from the interpreter when this package
 *           is loaded.
 *
 * @return Nothing is returned
 */
void RexxEntry ooShapesLoad(RexxThreadContext *c)
{
    if ( packageLoadHelper(c) )
    {
        c->DirectoryPut(TheDotLocalObj, c->NullString(), "ROUTINEERRORMESSAGE");
    }
}

/**
 * RexxPackageUnloader function.
 *
 * The package unloader function is called when the library package is unloaded
 * by the interpreter. The unloading process happens when the last interpreter
 * instance is destroyed during the last cleanup stages.
 *
 * We just do nothing at this time.
 *
 * @param c  Thread context pointer passed from the intepreter when this package
 *           is unloaded.
 *
 * @return Nothing is returned
 */
void RexxEntry ooShapesUnLoad(RexxThreadContext *c)
{
    // Purposively do nothing
}


// Generic
REXX_METHOD_PROTOTYPE(ooShapes_version_cls);

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
REXX_METHOD_PROTOTYPE(rect_copy);
REXX_METHOD_PROTOTYPE(rect_string);
REXX_METHOD_PROTOTYPE(rect_print);

// .Point
REXX_METHOD_PROTOTYPE(point_init);
REXX_METHOD_PROTOTYPE(point_x);
REXX_METHOD_PROTOTYPE(point_setX);
REXX_METHOD_PROTOTYPE(point_y);
REXX_METHOD_PROTOTYPE(point_setY);
REXX_METHOD_PROTOTYPE(point_copy);
REXX_METHOD_PROTOTYPE(point_add);
REXX_METHOD_PROTOTYPE(point_subtract);
REXX_METHOD_PROTOTYPE(point_incr);
REXX_METHOD_PROTOTYPE(point_decr);
REXX_METHOD_PROTOTYPE(point_inRect);
REXX_METHOD_PROTOTYPE(point_string);
REXX_METHOD_PROTOTYPE(point_print);

// .Size
REXX_METHOD_PROTOTYPE(size_init);
REXX_METHOD_PROTOTYPE(size_cx);
REXX_METHOD_PROTOTYPE(size_setCX);
REXX_METHOD_PROTOTYPE(size_cy);
REXX_METHOD_PROTOTYPE(size_setCY);
REXX_METHOD_PROTOTYPE(size_compare);
REXX_METHOD_PROTOTYPE(size_equateTo);
REXX_METHOD_PROTOTYPE(size_string);
REXX_METHOD_PROTOTYPE(size_print);


RexxMethodEntry ooShapes_methods[] = {
    REXX_METHOD(ooShapes_version_cls,           ooShapes_version_cls),
    REXX_METHOD(rect_init,                      rect_init),
    REXX_METHOD(rect_left,                      rect_left),
    REXX_METHOD(rect_top,                       rect_top),
    REXX_METHOD(rect_right,                     rect_right),
    REXX_METHOD(rect_bottom,                    rect_bottom),
    REXX_METHOD(rect_setLeft,                   rect_setLeft),
    REXX_METHOD(rect_setTop,                    rect_setTop),
    REXX_METHOD(rect_setRight,                  rect_setRight),
    REXX_METHOD(rect_setBottom,                 rect_setBottom),
    REXX_METHOD(rect_copy,                      rect_copy),
    REXX_METHOD(rect_string,                    rect_string),
    REXX_METHOD(rect_print,                     rect_print),
    REXX_METHOD(point_init,                     point_init),
    REXX_METHOD(point_x,                        point_x),
    REXX_METHOD(point_setX,                     point_setX),
    REXX_METHOD(point_y,                        point_y),
    REXX_METHOD(point_setY,                     point_setY),
    REXX_METHOD(point_copy,                     point_copy),
    REXX_METHOD(point_add,                      point_add),
    REXX_METHOD(point_subtract,                 point_subtract),
    REXX_METHOD(point_incr,                     point_incr),
    REXX_METHOD(point_decr,                     point_decr),
    REXX_METHOD(point_inRect,                   point_inRect),
    REXX_METHOD(point_string,                   point_string),
    REXX_METHOD(point_print,                    point_print),
    REXX_METHOD(size_init,                      size_init),
    REXX_METHOD(size_cx,                        size_cx),
    REXX_METHOD(size_setCX,                     size_setCX),
    REXX_METHOD(size_cy,                        size_cy),
    REXX_METHOD(size_setCY,                     size_setCY),
    REXX_METHOD(size_compare,                   size_compare),
    REXX_METHOD(size_equateTo,                  size_equateTo),
    REXX_METHOD(size_string,                    size_string),
    REXX_METHOD(size_print,                     size_print),
    REXX_LAST_METHOD()
};

RexxPackageEntry ooShapes_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_1_0,              // needs at least the 4.1.0 interpreter
    "ooShapes",                          // name of the package
    "1.0.0",                             // package information
    ooShapesLoad,                        // package load function
    ooShapesUnLoad,                      // package unload function
    NULL,                                // no exported functions
    ooShapes_methods                     // the exported methods
};

OOREXX_GET_PACKAGE(ooShapes);

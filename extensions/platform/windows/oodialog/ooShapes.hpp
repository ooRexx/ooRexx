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

#ifndef ooShapes_Included
#define ooShapes_Included


typedef struct tagORXRECT
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} ORXRECT, *PORXRECT;

typedef struct tagORXPOINT
{
    long  x;
    long  y;
} ORXPOINT, *PORXPOINT;

typedef struct tagORXSIZE
{
    long        cx;
    long        cy;
} ORXSIZE, *PORXSIZE;


__declspec(dllexport)  bool rxIsNormalized(PORXRECT r);
__declspec(dllexport)  bool rxCopyRect(PORXRECT rect, PORXRECT r);
__declspec(dllexport)  bool rxSetRect(PORXRECT rect, long x, long y, long x2, long y2);
__declspec(dllexport)  bool rxPtInRect(PORXRECT r, PORXPOINT pt);

__declspec(dllexport)  PORXPOINT     rxGetPoint(RexxMethodContext *context, RexxObjectPtr p, size_t argPos);
__declspec(dllexport)  RexxObjectPtr rxNewPoint(RexxThreadContext *c, long x, long y);
__declspec(dllexport)  RexxObjectPtr rxNewPoint(RexxMethodContext *c, long x, long y);
__declspec(dllexport)  RexxObjectPtr rxNewPoint(RexxThreadContext *c, ORXPOINT *pt);
__declspec(dllexport)  RexxObjectPtr rxNewPoint(RexxMethodContext *c, ORXPOINT *pt);
__declspec(dllexport)  PORXRECT      rxGetRect(RexxMethodContext *context, RexxObjectPtr r, size_t argPos);
__declspec(dllexport)  RexxObjectPtr rxNewRect(RexxMethodContext *context, long l, long t, long r, long b);
__declspec(dllexport)  RexxObjectPtr rxNewRect(RexxThreadContext *context, PORXRECT r);
__declspec(dllexport)  RexxObjectPtr rxNewRect(RexxMethodContext *context, PORXRECT r);
__declspec(dllexport)  PORXSIZE      rxGetSize(RexxMethodContext *context, RexxObjectPtr s, size_t argPos);
__declspec(dllexport)  RexxObjectPtr rxNewSize(RexxThreadContext *c, long cx, long cy);
__declspec(dllexport)  RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy);
__declspec(dllexport)  RexxObjectPtr rxNewSize(RexxMethodContext *c, PORXSIZE s);

__declspec(dllexport)  bool          goodMinMaxArgs(RexxMethodContext *c, RexxArrayObject args, size_t min, size_t max, size_t *arraySize);
__declspec(dllexport)  bool          getRectFromArglist(RexxMethodContext *, RexxArrayObject, PORXRECT, bool, int, int, size_t *, size_t *);
__declspec(dllexport)  bool          getPointFromArglist(RexxMethodContext *, RexxArrayObject, PORXPOINT, int, int, size_t *, size_t *);
__declspec(dllexport)  bool          getSizeFromArglist(RexxMethodContext *, RexxArrayObject, PORXPOINT, int, int, size_t *, size_t *);

#endif

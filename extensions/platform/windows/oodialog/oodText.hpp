/*----------------------------------------------------------------------------*/;
/*                                                                            */;
/* Copyright (c) 2009-2009 Rexx Language Association. All rights reserved.    */;
/*                                                                            */;
/* This program and the accompanying materials are made available under       */;
/* the terms of the Common Public License v1.0 which accompanies this         */;
/* distribution. A copy is also available at the following address:           */;
/* http://www.oorexx.org/license.html                                         */;
/*                                                                            */;
/* Redistribution and use in source and binary forms, with or                 */;
/* without modification, are permitted provided that the following            */;
/* conditions are met:                                                        */;
/*                                                                            */;
/* Redistributions of source code must retain the above copyright             */;
/* notice, this list of conditions and the following disclaimer.              */;
/* Redistributions in binary form must reproduce the above copyright          */;
/* notice, this list of conditions and the following disclaimer in            */;
/* the documentation and/or other materials provided with the distribution.   */;
/*                                                                            */;
/* Neither the name of Rexx Language Association nor the names                */;
/* of its contributors may be used to endorse or promote products             */;
/* derived from this software without specific prior written permission.      */;
/*                                                                            */;
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */;
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */;
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */;
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */;
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */;
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */;
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */;
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */;
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */;
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */;
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */;
/*                                                                            */;
/*----------------------------------------------------------------------------*/;

#ifndef oodText_Included
#define oodText_Included

extern int   getWeight(CSTRING opts);
extern bool  getTextSize(RexxMethodContext *, CSTRING, CSTRING, uint32_t, HWND, RexxObjectPtr, PSIZE);
extern bool  textSizeIndirect(RexxMethodContext *, CSTRING, CSTRING, uint32_t, SIZE *, HWND);
extern bool  textSizeFromWindow(RexxMethodContext *, CSTRING, SIZE *, HWND);
extern bool  getTextExtent(HFONT, HDC, CSTRING, SIZE *);
extern bool  screenToDlgUnit(HWND hwnd, POINT *point);
extern void  screenToDlgUnit(HDC hdc, POINT *point);
extern HFONT createFontFromName(int logicalPixelsY, CSTRING name, uint32_t size);
extern HFONT createFontFromName(CSTRING name, uint32_t size);
extern bool  mapPixelToDu(RexxMethodContext *c, RexxObjectPtr dlg, PPOINT p);

inline void du2pixel(POINT *point, int baseUnitX, int baseUnitY)
{
    point->x = MulDiv(point->x, baseUnitX, 4);
    point->y = MulDiv(point->y, baseUnitY, 8);
}

inline void pixel2du(POINT *point, int baseUnitX, int baseUnitY)
{
    point->x = MulDiv(point->x, 4, baseUnitX);
    point->y = MulDiv(point->y, 8, baseUnitY);
}

inline HFONT createFontFromName(HDC hdc, CSTRING name, uint32_t size)
{
    return createFontFromName(GetDeviceCaps(hdc, LOGPIXELSY), name, size);
}


#endif

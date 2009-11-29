/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2009-2009 Rexx Language Association. All rights reserved.    */
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

#ifndef oodDeviceGraphics_Included
#define oodDeviceGraphics_Included

extern uint32_t      parseShowOptions(CSTRING options);
extern RexxObjectPtr oodGetClientRect(RexxMethodContext *, HWND hwnd, PRECT);
extern RexxObjectPtr oodGetWindowRect(RexxMethodContext *, HWND hwnd);
extern logical_t     oodColorTable(RexxMethodContext *, DIALOGADMIN *, uint32_t, int32_t, int32_t, bool);
extern logical_t     oodWriteToWindow(RexxMethodContext *, HWND, int32_t, int32_t, CSTRING, CSTRING, uint32_t, CSTRING, int32_t, int32_t);
extern HBRUSH        oodCreateBrush(RexxMethodContext *, uint32_t, CSTRING);
extern RexxObjectPtr clearRect(RexxMethodContext *, HWND, PRECT);
extern RexxObjectPtr redrawRect(RexxMethodContext *, HWND, PRECT, bool);
extern int           getHeightFromFontSize(int fontSize);
extern void          maybeSetColorPalette(RexxMethodContext *, HBITMAP, CSTRING, DIALOGADMIN *, RexxObjectPtr);
extern LPBITMAPINFO  loadDIB(const char *szFile, uint32_t *);
extern WORD          numDIBColorEntries(LPBITMAPINFO lpBmpInfo);
extern BOOL          drawBackgroundBmp(pCPlainBaseDialog, HWND);
extern BOOL          drawBitmapButton(DIALOGADMIN *, pCPlainBaseDialog, LPARAM, bool);


/* macros for searching and checking the bitmap table */
#define SEARCHBMP(addr, ndx, id) \
   {                     \
      ndx = 0;\
      if (addr && addr->BmpTab)              \
      while ((ndx < addr->BT_size) && (addr->BmpTab[ndx].buttonID != (ULONG)id))\
         ndx++;                                                  \
   }

#define VALIDBMP(addr, ndx, id) \
   (addr && &addr->BmpTab[ndx] && (ndx < addr->BT_size) && (addr->BmpTab[ndx].buttonID == (ULONG)id))

//
// macros to access the fields in a BITMAPINFO struct
// field_value = macro(pBitmapInfo)
//

#define BI_WIDTH(pBI)       (int)((pBI)->bmiHeader.biWidth)
#define BI_HEIGHT(pBI)      (int)((pBI)->bmiHeader.biHeight)
#define BI_PLANES(pBI)      ((pBI)->bmiHeader.biPlanes)
#define BI_BITCOUNT(pBI)    ((pBI)->bmiHeader.biBitCount)

//
// macros to access BITMAPINFO fields in a DIB
// field_value = macro(pDIB)
//

#define DIB_WIDTH(pDIB)     (BI_WIDTH((LPBITMAPINFO)(pDIB)))
#define DIB_HEIGHT(pDIB)    (BI_HEIGHT((LPBITMAPINFO)(pDIB)))
#define DIB_PLANES(pDIB)    (BI_PLANES((LPBITMAPINFO)(pDIB)))
#define DIB_BITCOUNT(pDIB)  (BI_BITCOUNT((LPBITMAPINFO)(pDIB)))
//#define DIB_COLORS(pDIB)    (((LPBITMAPINFO)pDIB)->bmiHeader.biClrUsed)
#define DIB_COLORS(pDIB)    (numDIBColorEntries((LPBITMAPINFO)(pDIB)))
#define DIB_BISIZE(pDIB)    (sizeof(BITMAPINFOHEADER) \
                            + DIB_COLORS(pDIB) * sizeof(RGBQUAD))
#define DIB_PBITS(pDIB)     (((LPSTR)((LPBITMAPINFO)(pDIB))) \
                            + DIB_BISIZE(pDIB))
#define DIB_PBI(pDIB)       ((LPBITMAPINFO)(pDIB))


#endif

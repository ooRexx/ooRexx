/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
 * oodDeviceGraphics.cpp
 *
 * This module contains functions and methods for classes that primarily need to
 * use the Windows GDI (graphic device interface.)
 *
 * In reality there is no class that is exclusively concentrated on GDI, so the
 * classes present in this module, like the DialogExtensions class, generally
 * are only partially concerned with GDI.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodDeviceGraphics.hpp"


extern LPBITMAPINFO LoadDIB(const char *szFile);


RexxObjectPtr drawButton(HWND hDlg, HWND hCtrl, uint32_t id)
{
    DRAWITEMSTRUCT dis;
    RECT r;
    RexxObjectPtr result = TheZeroObj;

    GetWindowRect(hCtrl, &r);
    r.bottom = r.bottom - r.top;
    r.right = r.right - r.left;
    r.top = 0;
    r.left = 0;

    dis.CtlType = ODT_BUTTON;
    dis.CtlID = id;
    dis.itemAction = ODA_DRAWENTIRE;
    dis.itemState = (uint32_t)SendDlgItemMessage(hDlg, dis.CtlID, BM_GETSTATE, 0, 0);
    dis.hwndItem = hCtrl;
    dis.hDC = GetWindowDC(hCtrl);
    dis.rcItem = r;

    dis.itemData = 0;
    if ( SendMessage(hDlg, WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis) == 0 )
    {
        result = TheOneObj;
    }

    ReleaseDC(hCtrl, dis.hDC);
    return result;
}


RexxObjectPtr oodGetClientRect(RexxMethodContext *c, HWND hwnd, PRECT rect)
{
    oodResetSysErrCode(c->threadContext);
    if ( GetClientRect(hwnd, rect) == 0 )
    {
        oodSetSysErrCode(c->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


RexxObjectPtr oodGetWindowRect(RexxMethodContext *c, HWND hwnd)
{
    oodResetSysErrCode(c->threadContext);

    RECT r = {0};
    if ( GetWindowRect(hwnd, &r) == 0 )
    {
        oodSetSysErrCode(c->threadContext);
    }
    return rxNewRect(c, &r);
}

/**
 * Invalidates a rectangle in a window and has the window update.  This should
 * cause the window to immediately repaint the rectangle.
 *
 * @param c                 Method context we are operating in.
 * @param hwnd              Handle to the window to be redrawn.
 * @param pr                Pointer to a rect structure specifying the area to
 *                          be redrawn.  If this arg is null, the entire client
 *                          area is redrawn.
 * @param eraseBackground   Should the background of the window be redrawn
 *                          during the repaint.
 *
 * @return The zero object on success, the one object on failure.
 *
 * @remarks  This is common code for several API methods.  Note that in the call
 *           to InvalidateRect() if pr is null, the entire client area
 *           invalidated.  Because of this there is no need to call
 *           GetClientRect() to fill in a RECT stucture.
 */
RexxObjectPtr redrawRect(RexxMethodContext *c, HWND hwnd, PRECT pr, bool eraseBackground)
{
    oodResetSysErrCode(c->threadContext);

    if ( InvalidateRect(hwnd, pr, eraseBackground) == 0 )
    {
        oodSetSysErrCode(c->threadContext);
        return TheOneObj;
    }

    if ( UpdateWindow(hwnd) == 0 )
    {
        oodSetSysErrCode(c->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


RexxObjectPtr clearRect(RexxMethodContext *c, HWND hwnd, PRECT rect)
{
    oodResetSysErrCode(c->threadContext);

    HDC hDC;
    HPEN hOldPen, hPen;
    HBRUSH hOldBrush, hBrush;

    // Note that pre 4.0.1 ooDialog used GetWindowDC() here.  The MS docs
    // suggest it should only in rare circumstances.  Switched to using GetDC().
    // GetDC() seems to be the correct API to use.
    hDC = GetDC(hwnd);
    if ( hDC == NULL )
    {
        goto err_out;
    }

    hBrush = GetSysColorBrush(COLOR_BTNFACE);
    if ( hBrush == NULL )
    {
        // This can only fail if the color index is incorrect.  The function
        // does not set last error.
        goto err_out;
    }

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
    if ( hPen == NULL )
    {
        goto err_out;
    }

    hOldPen = (HPEN)SelectObject(hDC, hPen);
    hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Rectangle(hDC, rect->left, rect->top, rect->right, rect->bottom);

    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, hOldPen);

    // Delete the pen, but not the brush - it is a system cached object.
    DeleteObject(hPen);
    ReleaseDC(hwnd, hDC);

    return TheZeroObj;

err_out:
    oodSetSysErrCode(c->threadContext);
    return TheOneObj;
}


/* Get and free a device, create pen and brush objects (no font), assign and delete graphic objects */

size_t RexxEntry HandleDC_Obj(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HDC hDC;
   HWND w;

   CHECKARGL(2);

   if (argv[0].strptr[0] == 'G')      /* Get a window dc */
   {
       w = GET_HWND(argv[1]);
       hDC = GetWindowDC(w);
       RETHANDLE(hDC)
   }
   else
   if (argv[0].strptr[0] == 'F')      /* Free a DC */
   {
       CHECKARG(3);

       w = GET_HWND(argv[1]);
       hDC = (HDC)GET_HANDLE(argv[2]);
       if (ReleaseDC(w, hDC))
          RETC(0)
       else
          RETC(1)
   }
   else
   if (argv[0].strptr[0] == 'S')      /* assign a graphic object to a DC */
   {

       CHECKARG(3);
       hDC = (HDC)GET_HANDLE(argv[1]);
       HGDIOBJ obj = (HGDIOBJ)GET_HANDLE(argv[2]);
       RETHANDLE(SelectObject(hDC, obj));
   }
   else
   if (argv[0].strptr[0] == 'D')      /* delete a graphic object (pen, brush, font) */
   {
       HGDIOBJ obj = (HGDIOBJ)GET_HANDLE(argv[1]);
       RETC(!DeleteObject(obj));
   }
   else
   if (argv[0].strptr[0] == 'P')      /* Create a pen */
   {
       HPEN hP;
       UINT style;

       CHECKARG(4);

       if (!strcmp(argv[2].strptr, "DASH")) style = PS_DASH; else
       if (!strcmp(argv[2].strptr, "DOT")) style = PS_DOT; else
       if (!strcmp(argv[2].strptr, "DASHDOT")) style = PS_DASHDOT; else
       if (!strcmp(argv[2].strptr, "DASHDOTDOT")) style = PS_DASHDOTDOT; else
       if (!strcmp(argv[2].strptr, "NULL")) style = PS_NULL; else
       style = PS_SOLID;

       hP = CreatePen(style, atoi(argv[1].strptr), PALETTEINDEX(atoi(argv[3].strptr)));
       RETHANDLE(hP);
   }
   else
   if (argv[0].strptr[0] == 'B')     /* create a brush */
   {
       HBRUSH hB;
       HBITMAP hBmp;
       LOGBRUSH lb;
       LONG lbmp;

       if (argc == 4)
       {
          if ((lbmp = atol(argv[3].strptr)) != 0 )         /* we have a resource id */
          {
             DEF_ADM;
             dlgAdm = (DIALOGADMIN *)GET_POINTER(argv[1]);
             if (!dlgAdm) RETC(0)

             hBmp = LoadBitmap(dlgAdm->TheInstance, MAKEINTRESOURCE(lbmp));
             hB = CreatePatternBrush(hBmp);
             DeleteObject(hBmp);
          }
          else
          {
             hBmp = (HBITMAP) LoadDIB(argv[3].strptr);     /* we have a file name */
             lb.lbStyle = BS_DIBPATTERNPT;
             lb.lbColor = DIB_RGB_COLORS;
             lb.lbHatch = (ULONG_PTR)hBmp;
             hB = CreateBrushIndirect(&lb);
             LocalFree((void *)hBmp);
          }
       }
       else if (argc == 3)                   /* color brush */
       {
           lb.lbStyle = BS_HATCHED;
           lb.lbColor = PALETTEINDEX(atoi(argv[1].strptr));
           if (!stricmp(argv[2].strptr,"UPDIAGONAL")) lb.lbHatch = HS_BDIAGONAL;
           else if (!stricmp(argv[2].strptr,"DOWNDIAGONAL")) lb.lbHatch = HS_FDIAGONAL;
           else if (!stricmp(argv[2].strptr,"CROSS")) lb.lbHatch = HS_CROSS;
           else if (!stricmp(argv[2].strptr,"DIAGCROSS")) lb.lbHatch = HS_DIAGCROSS;
           else if (!stricmp(argv[2].strptr,"HORIZONTAL")) lb.lbHatch = HS_HORIZONTAL;
           else if (!stricmp(argv[2].strptr,"VERTICAL")) lb.lbHatch = HS_VERTICAL;
           else lb.lbStyle = BS_SOLID;
           hB = CreateBrushIndirect(&lb);
       }
       else if (argc == 2)                   /* color brush */
          hB = CreateSolidBrush(PALETTEINDEX(atoi(argv[1].strptr)));
       else hB = (HBRUSH)GetStockObject(HOLLOW_BRUSH);

       RETHANDLE(hB)
   }
   RETERR
}



/* handle all the drawing like SetPixel, LineTo,.... */


size_t RexxEntry DCDraw(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   CHECKARGL(4);

   if (argv[0].strptr[0] == 'L')      /* Draw a line */
   {
       HDC hDC = (HDC)GET_HANDLE(argv[1]);

       RETC(!LineTo(hDC, atoi(argv[2].strptr), atoi(argv[3].strptr)))
   }
   else
   if ((argv[0].strptr[0] == 'P') && (argv[0].strptr[1] == 'T'))    /* Draw a pixel */
   {
       CHECKARG(5);
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       RETC(!SetPixel(hDC, atoi(argv[2].strptr), atoi(argv[3].strptr),
            PALETTEINDEX(atoi(argv[4].strptr))))
   }
   else
   if (!strcmp(argv[0].strptr,"REC"))          /* Draw a brushed rectangle */
   {
       INT rect[4];
       register INT i;
       CHECKARG(6);

       HDC hDC = (HDC)GET_HANDLE(argv[1]);

       for (i=0;i<4;i++) rect[i] = atoi(argv[i+2].strptr);

       RETC(!Rectangle(hDC, rect[0], rect[1], rect[2], rect[3]))
   }
   else
   if (!strcmp(argv[0].strptr,"ARC"))         /* Draw an arc */
   {
       INT rect[8];
       register INT i;
       CHECKARG(10);

       HDC hDC = (HDC)GET_HANDLE(argv[1]);

       for (i=0;i<8;i++) rect[i] = atoi(argv[i+2].strptr);
                                            /* left   top     right    bottom  */
       RETC(!Arc(hDC, rect[0], rect[1], rect[2], rect[3],
                                /* start x  start y  end x    end y  */
                                   rect[4], rect[5], rect[6], rect[7]))
   }
   else
   if (!strcmp(argv[0].strptr,"ANG"))         /* Draw an  angle arc */
   {
       CHECKARG(7);
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
                                                    /* x                 y                   radius         */
       RETC(!AngleArc(hDC, atoi(argv[2].strptr), atoi(argv[3].strptr), atoi(argv[4].strptr),
                              /* start angle                sweep angle*/
                        (float)atof(argv[5].strptr), (float)atof(argv[6].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr,"PIE"))         /* Draw a pie */
   {
       INT rect[8];
       register INT i;

       CHECKARG(10);

       HDC hDC = (HDC)GET_HANDLE(argv[1]);

       for (i=0;i<8;i++) rect[i] = atoi(argv[i+2].strptr);

       RETC(!Pie(hDC,
        rect[0],    // x-coord. of bounding rectangle's upper-left corner
        rect[1],    // y-coord. of bounding rectangle's upper-left corner
        rect[2],    // x-coord. of bounding rectangle's lower-right corner
        rect[3],    // y-coord. of bounding rectangle's lower-right corner
        rect[4],    // x-coord. of first radial's endpoint
        rect[5],    // y-coord. of first radial's endpoint
        rect[6],    // x-coord. of second radial's endpoint
        rect[7]     // y-coord. of second radial's endpoint
       ))
   }
   else
   if (!strcmp(argv[0].strptr,"FL"))      /* Fill a graphic */
   {
       CHECKARG(5);

       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       RETC(!FloodFill(hDC, atoi(argv[2].strptr), atoi(argv[3].strptr),
                        PALETTEINDEX(atoi(argv[4].strptr))));
   }
   else
   if (!strcmp(argv[0].strptr,"DIM"))      /* Dim a bitmap */
   {
       int a, i, j, x, y, diff, diffx, stepx, stepy, steps, sx, sy;

       CHECKARG(7);

       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       sx = atoi(argv[2].strptr);
       sy = atoi(argv[3].strptr);
       steps = atoi(argv[6].strptr);
       stepx = atoi(argv[4].strptr);
       stepy = atoi(argv[5].strptr);
       diff = steps * stepy;
       diffx = steps * stepx;

       for (a = 0; a < steps; a++)
          for (y = a*stepy, i = 0; i < sy / steps; y+=diff, i++)
             for (x = a*stepx, j = 0; j < sx / steps; x+=diffx, j++)
                 Rectangle(hDC, x-a*stepx, y-a*stepy, x+stepx+1, y+stepy+1);
       RETC(0)
   }
   RETERR
}


/* Prepares a DC for drawing (like MoveToEx) and gets info from a DC (like GetPixel) */

size_t RexxEntry DrawGetSet(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   CHECKARGL(2);

   if ((argv[0].strptr[0] == 'S') && (argv[0].strptr[1] == 'D'))     /* "SDP" Set the starting position of a draw operation */
   {
       POINT pnt;
       CHECKARG(4);

       HDC hDC = (HDC)GET_HANDLE(argv[1]);

       if (MoveToEx(hDC, atoi(argv[2].strptr), atoi(argv[3].strptr), &pnt))
       {
           sprintf(retstr->strptr, "%d %d", pnt.x, pnt.y);
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       else RETC(0)
   }
   else
   if ((argv[0].strptr[0] == 'G') && (argv[0].strptr[1] == 'P'))       /* "GPT" Get the pixel color */
   {
       CHECKARG(4);
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       RETVAL(GetPixel(hDC, atoi(argv[2].strptr), atoi(argv[3].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr, "COL"))
   {
       CHECKARG(3);
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       if (hDC)
       {
          SetTextColor(hDC, PALETTEINDEX(atoi(argv[2].strptr)));
          RETC(0)
       }
       RETC(1)
   }
   else
   if (!strcmp(argv[0].strptr,"SBK"))          /* Set background mode to transparent or opaque */
   {
       INT bkmode;
       CHECKARG(3);
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       if (!strcmp(argv[2].strptr, "TRANSPARENT")) bkmode = TRANSPARENT; else bkmode = OPAQUE;
       RETVAL(SetBkMode(hDC, bkmode))
   }
   else
   if (!strcmp(argv[0].strptr,"GAD"))          /* Get arc direction */
   {
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       RETVAL((GetArcDirection(hDC) == AD_CLOCKWISE))
   }
   else
   if (!strcmp(argv[0].strptr,"SAD"))          /* Set arc direction */
   {
       INT i;
       CHECKARG(3);
       HDC hDC = (HDC)GET_HANDLE(argv[1]);
       if (atoi(argv[2].strptr)) i = AD_CLOCKWISE; else i = AD_COUNTERCLOCKWISE;
       RETVAL(SetArcDirection(hDC, i))
   }
   RETERR
}



/* Set the background of the dialog or the color of dialog items */

size_t RexxEntry SetBackground(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   DEF_ADM;
   HBRUSH hbrush = NULL;
   ULONG id, i;

   CHECKARGL(3);

   GET_ADM;
   if (!dlgAdm) RETERR

   if (!strcmp(argv[1].strptr,"BMP"))              /* Set the background bitmap (drawn by the windows procedure) */
   {
       dlgAdm->BkgBitmap = (HBITMAP)GET_HANDLE(argv[2]);
   }
   else
   if (!strcmp(argv[1].strptr,"BRU"))              /* Set the background brush */
   {
       dlgAdm->BkgBrush = (HBRUSH)GET_HANDLE(argv[2]);
   }
   else
   if (!strcmp(argv[1].strptr,"COL"))             /* Set the color of dialog items (stored in a table) */
   {
       if (!dlgAdm->ColorTab)
       {
           dlgAdm->ColorTab = (COLORTABLEENTRY *)LocalAlloc(LMEM_FIXED, sizeof(COLORTABLEENTRY) * MAX_CT_ENTRIES);
           if (!dlgAdm->ColorTab)
           {
               MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
               RETVAL(-1);
           }
           dlgAdm->CT_size = 0;
       }

       if ( dlgAdm->CT_size < MAX_CT_ENTRIES )
       {
           int fgColor = -1;

           if ( argc >= 5 && argv[4].strlength > 0 )
           {
               fgColor = atoi(argv[4].strptr);
           }
           id = atol(argv[2].strptr);

           SEARCHBRUSH(dlgAdm, i, id, hbrush);
           if (hbrush)
           {
               if ( ! dlgAdm->ColorTab[i].isSysBrush )
               {
                   DeleteObject(hbrush);
               }
           }
           else
           {
               i = dlgAdm->CT_size;
               dlgAdm->ColorTab[i].itemID = id;
               dlgAdm->CT_size++;
           }

           dlgAdm->ColorTab[i].ColorBk = atoi(argv[3].strptr);
           dlgAdm->ColorTab[i].ColorFG = fgColor;

           if ( argc > 5 )
           {
               dlgAdm->ColorTab[i].ColorBrush = GetSysColorBrush(dlgAdm->ColorTab[i].ColorBk);
               dlgAdm->ColorTab[i].isSysBrush = true;
           }
           else
           {
               dlgAdm->ColorTab[i].ColorBrush = CreateSolidBrush(PALETTEINDEX(dlgAdm->ColorTab[i].ColorBk));
               dlgAdm->ColorTab[i].isSysBrush = false;
           }

           if ( hbrush )
           {
               RETC(1)
           }
       }
       else
       {
           MessageBox(0, "Dialog control elements have exceeded the maximum\n"
                         "number of allocated color table entries. The color\n"
                         "for the dialog control can not be added.",
                      "Error" ,MB_OK | MB_ICONHAND);
           RETERR
       }
   }
   RETC(0)
}


/**
 *  Methods for the .DialogExtensions mixin class.
 */
#define DIALOGEXTENSIONS_CLASS        "DialogExtensions"

pCPlainBaseDialog dlgExtSetup(RexxMethodContext *c, RexxObjectPtr dlg)
{
    oodResetSysErrCode(c->threadContext);
    pCPlainBaseDialog pcpbd = dlgToCSelf(c, dlg);

    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(c, dlg);
        return NULL;
    }
    return pcpbd;
}

/** DialogExtensions::clearWindowRect()
 *
 *  'Clears' the client area of the specified Window.
 *
 *  @param  hwnd  The handle of the window to clear.
 *
 *  @return 0 on success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @see WindowBase::clear()
 */
RexxMethod2(RexxObjectPtr, dlgext_clearWindowRect, POINTERSTRING, hwnd, OSELF, self)
{
    RECT r = {0};
    if ( oodGetClientRect(context, (HWND)hwnd, &r) == TheOneObj )
    {
        return TheOneObj;
    }
    return clearRect(context, (HWND)hwnd, &r);
}


/** DialogExtensions::clearWindowRect()
 *
 *  'Clears' the client area of the specified dialog control.
 *
 *  @param  rxID  The resource ID used to identify which control is cleared.
 *                May be numeric or symbolic.
 *
 *  @return 0 on success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @see WindowBase::clear()
 */
RexxMethod2(RexxObjectPtr, dlgext_clearControlRect, RexxObjectPtr, rxID, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }
    HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
    if ( hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }

    RECT r = {0};
    if ( oodGetClientRect(context, hCtrl, &r) == TheOneObj )
    {
        return TheOneObj;
    }
    return clearRect(context, hCtrl, &r);
}


/** DialogExtension::clearRect()
 *
 *  'Clears' a rectangle in the specified Window.
 *
 *  @param hwnd         The window of the dialog or dialog control to act on.
 *  @param coordinates  The coordinates of the rectangle, given in pixels.
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Point object.
 *    Form 3:  x1, y1, y1, y2
 *
 *  @return 0 on success or 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @see WindowBase::clear()
 *
 *  @remarks  Technically this method does not require the underlying dialog to
 *             have been created, so that check is skipped.  But, in reality I
 *             suspect this method should only be used with the dialog and its
 *             control windows.
 */
RexxMethod3(RexxObjectPtr, dlgext_clearRect, POINTERSTRING, hwnd, ARGLIST, args, OSELF, self)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = dlgToCSelf(context, self);

    RECT r = {0};
    size_t arraySize;
    int argsUsed;

    if ( ! getRectFromArglist(context, args, &r, true, 2, 5, &arraySize, &argsUsed) )
    {
        return TheOneObj;
    }
    if ( arraySize > (argsUsed + 1) )
    {
        return tooManyArgsException(context->threadContext, argsUsed + 1);
    }
    return clearRect(context, (HWND)hwnd, &r);
}


/** DialogExtensions::handle2Rect()
 *
 *  Retrieves the dimensions of the bounding rectangle of the specified window.
 *  The dimensions are in screen coordinates that are relative to the upper-left
 *  corner of the screen.
 *
 *  @param  hwnd  The window handle whose bounding rectangle is to be retrieved.
 *
 *  @return  The bounding rectangle of the specified window.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxObjectPtr, dlgext_handle2Rect, POINTERSTRING, hwnd)
{
    return oodGetWindowRect(context, (HWND)hwnd);
}

/** DialogExtensions::getControlRect()
 *
 *  Retrieves the dimensions of the bounding rectangle of the specified dialog
 *  control. The dimensions are in screen coordinates that are relative to the
 *  upper-left corner of the screen.
 *
 *  @param  rxID  The resource ID of the dialog control.  May be numeric or
 *                symbolic.
 *
 *  @return  The bounding rectangle of the specified dialog control.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The old getButtonRect() method forwards to this method, takes the
 *            returned .Rect object and manipulates it.  So, we must return a
 *            .Rect object.  Therefore, if rxID is not good, rather than return
 *            -1, we use a null HWND.  GetWindowRect() will fail and the
 *             .SystemErrorCode will get set.  getButtonRect() can then check if
 *             the system error code is not 0 and return 1 to mimic the old
 *             behavior.
 */
RexxMethod2(RexxObjectPtr, dlgext_getControlRect, RexxObjectPtr, rxID, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    HWND hCtrl = NULL;

    uint32_t id;
    if ( oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        hCtrl = GetDlgItem(pcpbd->hDlg, id);
    }
    return oodGetWindowRect(context, hCtrl);
}

/** DialogExtensions::redrawWindowRect()
 *
 *  Immediately redraws the entire client area of the specified window, (a
 *  dialog or a dialog control.)
 *
 *  @param hwnd   [OPITONAL]  The window of the dialog or dialog control to act
 *                on.  The default is this dialog.
 *  @param erase  [OPITONAL]  Whether the background should be erased first.
 *                The default is false.
 *
 *  @return 0 on success or 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, dlgext_redrawWindowRect, OPTIONAL_POINTERSTRING, _hwnd, OPTIONAL_logical_t, erase, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    HWND hwnd    = argumentExists(1) ? (HWND)_hwnd : pcpbd->hDlg;
    bool doErase = argumentExists(2) ? (erase ? true : false) : false;
    return redrawRect(context, hwnd, NULL, doErase);
}


/** DialogExtensions::redrawRect()
 *
 *  Immediately redraws the specified rectangle in the specified window, (a
 *  dialog or a dialog control.)
 *
 *  @param hwnd   [OPITONAL]  The window of the dialog or dialog control to act
 *                on.  The default is this dialog.
 *  @param  The coordinates of the rectangle.
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Point object.
 *    Form 3:  x1, y1, y1, y2
 *
 *  @param erase  [OPITONAL]  Whether the background should be erased first.
 *                The default is false.
 *
 *  @return 0 on success or 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, dlgext_redrawRect, POINTERSTRING, _hwnd, ARGLIST, args, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    HWND hwnd = argumentExists(1) ? (HWND)_hwnd : pcpbd->hDlg;
    bool doErase = false;

    RECT r = {0};
    size_t arraySize;
    int argsUsed;

    if ( ! getRectFromArglist(context, args, &r, true, 2, 6, &arraySize, &argsUsed) )
    {
        return TheOneObj;
    }

    // Even though arg 1 is optional, a required arg comes after it.  So, array
    // size will include it.
    if ( arraySize > (argsUsed + 2) )
    {
        return tooManyArgsException(context->threadContext, argsUsed + 2);
    }
    else if ( arraySize == (argsUsed + 2) )
    {
        // The object at argsUsed + 2 has to exist, otherwise arraySize would
        // equal argsUsed + 1.
        RexxObjectPtr obj = context->ArrayAt(args, argsUsed + 2);

        logical_t erase;
        if ( ! context->Logical(obj, &erase) )
        {
            return notBooleanException(context->threadContext, argsUsed + 2, obj);
        }
        doErase = erase ? true : false;
    }

    return redrawRect(context, hwnd, &r, doErase);
}


/** DialogExtensions::redrawControl()
 *
 *  Immediately redraws the entire client area of the specified control.
 *
 *  @param rxID   The resource ID of the control.  Can be numeric or symbolic.
 *  @param erase  [OPITONAL]  Whether the background should be erased first.
 *                The default is false.
 *
 *  @return 0 on success, -1 for an invalid resource ID, or 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, dlgext_redrawControl, RexxObjectPtr, rxID, OPTIONAL_logical_t, erase, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    bool doErase = argumentExists(2) ? (erase ? true : false) : false;
    HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);

    if ( hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return redrawRect(context, hCtrl, NULL, doErase);
}


RexxMethod2(RexxObjectPtr, dlgext_drawButton, RexxObjectPtr, rxID, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
    if ( hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return drawButton(pcpbd->hDlg, hCtrl, id);
}




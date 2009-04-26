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
#include "oovutil.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "oodCommon.h"


extern LPBITMAPINFO LoadDIB(const char *szFile);


BOOL DrawButton(DIALOGADMIN * aDlg, INT id)
{
   HWND hW = NULL;
   DRAWITEMSTRUCT dis;
   RECT r;
   BOOL rc;

   dis.CtlType = ODT_BUTTON;
   dis.CtlID = id;
   dis.itemAction = ODA_DRAWENTIRE;
   dis.itemState = (UINT)SendDlgItemMessage(aDlg->TheDlg, dis.CtlID, BM_GETSTATE, 0, 0);
   hW = GetDlgItem(aDlg->TheDlg, dis.CtlID);
   dis.hDC = GetWindowDC(hW);
   dis.hwndItem = hW;
   GetWindowRect(hW, &r);
   r.bottom = r.bottom - r.top;
   r.right = r.right - r.left;
   r.top = 0;
   r.left = 0;
   dis.rcItem = r;
   dis.itemData = 0;
   rc = SendMessage(aDlg->TheDlg, WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis) != 0;
   ReleaseDC(hW, dis.hDC);
   return rc;
}



/* Get information about window rectangle, redraw and clear a rectangle */

size_t RexxEntry WindowRect(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND w;
   RECT r;

   CHECKARGL(2);

   if (!strcmp(argv[0].strptr,"CLR"))            /* Clear a rectangle */
   {
       INT rect[4];
       HDC hDC;
       HPEN oP, hpen;
       HBRUSH oB, hbr;
       INT i;
       DEF_ADM;

       CHECKARGL(3);
       dlgAdm = (DIALOGADMIN *)GET_POINTER(argv[1]);
       if (!dlgAdm) RETERR

       w = GET_HWND(argv[2]);

       if ((hDC=GetWindowDC(w)))
       {
           if (argc == 7)                   /* coordinates are specified */
           {
                for (i=0;i<4;i++) rect[i] = atoi(argv[i+3].strptr);
           }
           else
           {
                GetClientRect(w, &r);      /* no coordinates, so get coordinates */
                rect[0] = r.left;
                rect[1] = r.top;
                rect[2] = r.right;
                rect[3] = r.bottom;
           }

           if (dlgAdm->Use3DControls)
           {
              hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
              hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
           }
           else
           {
              hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
              hpen = (HPEN)GetStockObject(WHITE_PEN);
           }
           oP = (HPEN)SelectObject(hDC, hpen);
           oB = (HBRUSH)SelectObject(hDC, hbr);
           Rectangle(hDC, rect[0], rect[1], rect[2], rect[3]);
           SelectObject(hDC, oB);
           SelectObject(hDC, oP);

           if (dlgAdm->Use3DControls)
           {
              DeleteObject(hpen);
              DeleteObject(hbr);
           }
           ReleaseDC(w, hDC);
           RETC(0)
       }
       else RETC(1)
   }
   else
   if (!strcmp(argv[0].strptr,"RDW"))          /* redraw a rectangle */
   {
       const char *erasebgr= "";

       w = GET_HWND(argv[1]);

       if (argc == 7)                          /* coordinates are specified */
       {
           r.left = atoi(argv[2].strptr);
           r.top = atoi(argv[3].strptr);
           r.right = atoi(argv[4].strptr);
           r.bottom = atoi(argv[5].strptr);
           erasebgr = argv[6].strptr;
       }
       else
       {
           GetClientRect(w, &r);               /* no coordinates, so get coordinates */
           if (argc == 3)
           {
               erasebgr = argv[2].strptr;
           }
       }

       if (InvalidateRect(w, &r, IsYes(erasebgr)))
       {
          UpdateWindow(w);
          RETC(0)
       }
       else RETC(1)
   }
   else
   if (!strcmp(argv[0].strptr,"GET"))         /* get the window/client rectangle */
   {
       w = GET_HWND(argv[1]);

       if (w)
       {
           if ((argc == 3) && !stricmp(argv[2].strptr, "CLIENT"))
           {
               if (GetClientRect(w, &r))
               {
                  sprintf(retstr->strptr, "%ld %ld %ld %ld", r.left, r.top, r.right, r.bottom);
                  retstr->strlength = strlen(retstr->strptr);
                  return 0;
               }
           }
           else
           if (GetWindowRect(w, &r))
           {
              sprintf(retstr->strptr, "%ld %ld %ld %ld", r.left, r.top, r.right, r.bottom);
              retstr->strlength = strlen(retstr->strptr);
              return 0;
           }
       }
       RETC(1)
   }
   else
   if (!strcmp(argv[0].strptr,"BTN"))          /* (re)draw a button */
   {
       DEF_ADM;

       CHECKARGL(3);
       dlgAdm = (DIALOGADMIN *)GET_POINTER(argv[1]);
       if (!dlgAdm) RETERR

       if (DrawButton(dlgAdm, atoi(argv[2].strptr)))
          RETC(0)
       else
          RETC(1)
   }
   RETERR
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

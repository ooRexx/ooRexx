/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
#include <windows.h>
#define INCL_REXXSAA
#include <rexx.h>
#include <stdio.h>
#include <dlgs.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "oovutil.h"


extern LPBITMAPINFO LoadDIB(LPSTR szFile);


BOOL DrawButton(DIALOGADMIN * aDlg, INT id)
{
   HWND hW = NULL;
   DRAWITEMSTRUCT dis;
   RECT r;
   BOOL rc;

   dis.CtlType = ODT_BUTTON;
   dis.CtlID = id;
   dis.itemAction = ODA_DRAWENTIRE;
   dis.itemState = SendDlgItemMessage(aDlg->TheDlg, dis.CtlID, BM_GETSTATE, 0, 0);
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
   rc = SendMessage(aDlg->TheDlg, WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis);
   ReleaseDC(hW, dis.hDC);
   return rc;
}



/* Get information about window rectangle, redraw and clear a rectangle */

ULONG APIENTRY WindowRect(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
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
       dlgAdm = (DIALOGADMIN *)atol(argv[1].strptr);
       if (!dlgAdm) RETERR

       w = (HWND)atol(argv[2].strptr);

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
              hbr = GetStockObject(WHITE_BRUSH);
              hpen = GetStockObject(WHITE_PEN);
           }
           oP = SelectObject(hDC, hpen);
           oB = SelectObject(hDC, hbr);
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
       PCHAR erasebgr= "";

       w = (HWND)atol(argv[1].strptr);

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
               erasebgr = argv[2].strptr;
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
       w = (HWND)atol(argv[1].strptr);

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
       dlgAdm = (DIALOGADMIN *)atol(argv[1].strptr);
       if (!dlgAdm) RETERR

       if (DrawButton(dlgAdm, atoi(argv[2].strptr)))
          RETC(0)
       else
          RETC(1)
   }
   RETERR
}




/* Get and free a device, create pen and brush objects (no font), assign and delete graphic objects */

ULONG APIENTRY HandleDC_Obj(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   HDC hDC;
   HWND w;

   CHECKARGL(2);

   if (argv[0].strptr[0] == 'G')      /* Get a window dc */
   {
       w = (HWND)atol(argv[1].strptr);
       hDC = GetWindowDC(w);
       RETVAL((ULONG)hDC)
   }
   else
   if (argv[0].strptr[0] == 'F')      /* Free a DC */
   {
       CHECKARG(3);

       w = (HWND)atol(argv[1].strptr);
       if (ReleaseDC(w, (HDC)atol(argv[2].strptr)))
          RETC(0)
       else
          RETC(1)
   }
   else
   if (argv[0].strptr[0] == 'S')      /* assign a graphic object to a DC */
   {

       CHECKARG(3);
       RETVAL((ULONG)SelectObject((HDC)atol(argv[1].strptr), (HGDIOBJ)atol(argv[2].strptr)))
   }
   else
   if (argv[0].strptr[0] == 'D')      /* delete a graphic object (pen, brush, font) */
   {
       RETC(!DeleteObject((HGDIOBJ)strtoul(argv[1].strptr,'\0',10)));
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
       RETVAL((ULONG)hP)
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
             dlgAdm = (DIALOGADMIN *)atol(argv[1].strptr);
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
             lb.lbHatch = (LONG)hBmp;
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

       RETVAL((ULONG)hB)
   }
   RETERR
}



/* handle all the drawing like SetPixel, LineTo,.... */


ULONG APIENTRY DCDraw(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   CHECKARGL(4);

   if (argv[0].strptr[0] == 'L')      /* Draw a line */
   {
       RETC(!LineTo((HDC)atol(argv[1].strptr), atoi(argv[2].strptr), atoi(argv[3].strptr)))
   }
   else
   if ((argv[0].strptr[0] == 'P') && (argv[0].strptr[1] == 'T'))    /* Draw a pixel */
   {
       CHECKARG(5);
       RETC(!SetPixel((HDC)atol(argv[1].strptr), atoi(argv[2].strptr), atoi(argv[3].strptr),
            PALETTEINDEX(atoi(argv[4].strptr))))
   }
   else
   if (!strcmp(argv[0].strptr,"REC"))          /* Draw a brushed rectangle */
   {
       INT rect[4];
       register INT i;
       CHECKARG(6);

       for (i=0;i<4;i++) rect[i] = atoi(argv[i+2].strptr);

       RETC(!Rectangle((HDC)atol(argv[1].strptr), rect[0], rect[1], rect[2], rect[3]))
   }
   else
   if (!strcmp(argv[0].strptr,"ARC"))         /* Draw an arc */
   {
       INT rect[8];
       register INT i;
       CHECKARG(10);

       for (i=0;i<8;i++) rect[i] = atoi(argv[i+2].strptr);
                                            /* left   top     right    bottom  */
       RETC(!Arc((HDC)atol(argv[1].strptr), rect[0], rect[1], rect[2], rect[3],
                                /* start x  start y  end x    end y  */
                                   rect[4], rect[5], rect[6], rect[7]))
   }
   else
   if (!strcmp(argv[0].strptr,"ANG"))         /* Draw an  angle arc */
   {
       CHECKARG(7);
                                                    /* x                 y                   radius         */
       RETC(!AngleArc((HDC)atol(argv[1].strptr), atoi(argv[2].strptr), atoi(argv[3].strptr), atoi(argv[4].strptr),
                              /* start angle                sweep angle*/
                        (float)atof(argv[5].strptr), (float)atof(argv[6].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr,"PIE"))         /* Draw a pie */
   {
       INT rect[8];
       register INT i;

       CHECKARG(10);

       for (i=0;i<8;i++) rect[i] = atoi(argv[i+2].strptr);

       RETC(!Pie((HDC)atol(argv[1].strptr),
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

       RETC(!FloodFill((HDC)atol(argv[1].strptr), atoi(argv[2].strptr), atoi(argv[3].strptr),
                        PALETTEINDEX(atoi(argv[4].strptr))));
   }
   else
   if (!strcmp(argv[0].strptr,"DIM"))      /* Dim a bitmap */
   {
       HDC hdc;
       int a, i, j, x, y, diff, diffx, stepx, stepy, steps, sx, sy;

       CHECKARG(7);

       hdc = (HDC)atol(argv[1].strptr);
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
                 Rectangle(hdc, x-a*stepx, y-a*stepy, x+stepx+1, y+stepy+1);
       RETC(0)
   }
   RETERR
}


/* Prepares a DC for drawing (like MoveToEx) and gets info from a DC (like GetPixel) */

ULONG APIENTRY DrawGetSet(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   CHECKARGL(2);

   if ((argv[0].strptr[0] == 'S') && (argv[0].strptr[1] == 'D'))     /* "SDP" Set the starting position of a draw operation */
   {
       POINT pnt;
       CHECKARG(4);

       if (MoveToEx((HDC)atol(argv[1].strptr), atoi(argv[2].strptr), atoi(argv[3].strptr), &pnt))
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
       RETVAL(GetPixel((HDC)atol(argv[1].strptr), atoi(argv[2].strptr), atoi(argv[3].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr, "COL"))
   {
       HDC hDC;

       CHECKARG(3);
       hDC = (HDC)atol(argv[1].strptr);
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
       if (!strcmp(argv[2].strptr, "TRANSPARENT")) bkmode = TRANSPARENT; else bkmode = OPAQUE;
       RETVAL(SetBkMode((HDC)atol(argv[1].strptr), bkmode))
   }
   else
   if (!strcmp(argv[0].strptr,"GAD"))          /* Get arc direction */
   {
       RETVAL((GetArcDirection((HDC)atol(argv[1].strptr)) == AD_CLOCKWISE))
   }
   else
   if (!strcmp(argv[0].strptr,"SAD"))          /* Set arc direction */
   {
       INT i;
       CHECKARG(3);
       if (atoi(argv[2].strptr)) i = AD_CLOCKWISE; else i = AD_COUNTERCLOCKWISE;
       RETVAL(SetArcDirection((HDC)atol(argv[1].strptr), i))
   }
   RETERR
}



/* Set the background of the dialog or the color of dialog items */

ULONG APIENTRY SetBackground(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   DEF_ADM;
   HBRUSH hbrush = NULL;
   ULONG id, i;

   CHECKARGL(3);

   GET_ADM;
   if (!dlgAdm) RETERR

   if (!strcmp(argv[1].strptr,"BMP"))              /* Set the background bitmap (drawn by the windows procedure) */
   {
       dlgAdm->BkgBitmap = (HBITMAP) atol(argv[2].strptr);
   }
   else
   if (!strcmp(argv[1].strptr,"BRU"))              /* Set the background brush */
   {
       dlgAdm->BkgBrush = (HBRUSH) atol(argv[2].strptr);
   }
   else
   if (!strcmp(argv[1].strptr,"COL"))             /* Set the color of dialog items (stored in a table) */
   {
       if (!dlgAdm->ColorTab)
       {
           dlgAdm->ColorTab = LocalAlloc(LMEM_FIXED, sizeof(COLORTABLEENTRY) * MAX_CT_ENTRIES);
           if (!dlgAdm->ColorTab)
           {
               MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
               RETVAL(-1);
           }
           dlgAdm->CT_size = 0;
       }

       if ( dlgAdm->CT_size < MAX_CT_ENTRIES )
       {
           id = atol(argv[2].strptr);
           SEARCHBRUSH(dlgAdm, i, id, hbrush);
           if (hbrush)
           {
               DeleteObject(hbrush);
               dlgAdm->ColorTab[i].ColorBk = atoi(argv[3].strptr);
               if (argc == 5) dlgAdm->ColorTab[i].ColorFG = atoi(argv[4].strptr); else dlgAdm->ColorTab[i].ColorFG = -1;
               dlgAdm->ColorTab[i].ColorBrush = (HBRUSH)CreateSolidBrush(PALETTEINDEX(dlgAdm->ColorTab[i].ColorBk));
               RETC(1)
           }
           else
           {
               dlgAdm->ColorTab[dlgAdm->CT_size].itemID = id;
               dlgAdm->ColorTab[dlgAdm->CT_size].ColorBk = atoi(argv[3].strptr);
               if (argc == 5) dlgAdm->ColorTab[i].ColorFG = atoi(argv[4].strptr); else dlgAdm->ColorTab[i].ColorFG = -1;
               dlgAdm->ColorTab[dlgAdm->CT_size].ColorBrush = (HBRUSH)CreateSolidBrush(PALETTEINDEX(dlgAdm->ColorTab[dlgAdm->CT_size].ColorBk));
               dlgAdm->CT_size++;
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

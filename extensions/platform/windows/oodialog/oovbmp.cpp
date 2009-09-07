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
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h
#include <stdio.h>
#include <dlgs.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "oodCommon.hpp"

extern LPBITMAPINFO LoadDIB(const char *szFile);
WORD NumDIBColorEntries(LPBITMAPINFO lpBmpInfo);
HPALETTE CreateDIBPalette(LPBITMAPINFO lpBmpInfo);
void SetSysPalColors(HPALETTE hPal);
HPALETTE CopyPalette(HPALETTE hSrcPal);
extern BOOL DrawButton(DIALOGADMIN *,INT id);
extern LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern HWND ScrollingButton;
extern HWND RedrawScrollingButton;


/* draw the part that will not be covered by the bitmap */
void DrawBmpBackground(DIALOGADMIN * addr, INT id, HDC hDC, RECT * itRect, RECT * bmpRect, WPARAM wParam, LPARAM lParam, LONG left, LONG top)
{
    HBRUSH hbr = NULL, oB;
    HPEN oP, hpen;
    INT topdiv, leftdiv, rightdiv, bottomdiv, i;

    leftdiv = bmpRect->left - itRect->left;
    topdiv = bmpRect->top - itRect->top;
    rightdiv = itRect->right - (bmpRect->right - left);
    bottomdiv = itRect->bottom - (bmpRect->bottom - top);

    /* the bitmap covers all */
    if ((!leftdiv) && (!topdiv) && (!rightdiv) && (!bottomdiv)) return;
    if (!RectVisible(hDC, itRect)) return;

    if (leftdiv < 0) leftdiv = 0;
    if (rightdiv < 0) rightdiv = 0;
    if (topdiv < 0) topdiv = 0;
    if (bottomdiv < 0) bottomdiv = 0;

    /* we dont have a bitmap button */
    /* draw rectangle with background color */
    hpen = CreatePen(PS_NULL, 0, GetSysColor(COLOR_BTNFACE));
    if (addr->CT_size)      /* Has the dialog item its own user color ?*/
        SEARCHBRUSH(addr, i, id, hbr);

    if (!hbr)
    {
#ifdef __CTL3D
        if (addr->Use3DControls)
           if (addr->BkgBrush) hbr = addr->BkgBrush; else hbr = Ctl3dCtlColorEx(WM_CTLCOLORDLG, wParam, lParam);
        else
#endif
           if (addr->BkgBrush) hbr = addr->BkgBrush; else hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
    }

    oP = (HPEN)SelectObject(hDC, hpen);
    oB = (HBRUSH)SelectObject(hDC, hbr);

    /* the bitmap covers nothing */
    if ((rightdiv == itRect->right - itRect->left) || (bottomdiv == itRect->bottom - itRect->top))
       Rectangle(hDC, itRect->left-1, itRect->top-1, itRect->right+1, itRect->bottom+1);
    else
    {
       if (leftdiv > 0)
             Rectangle(hDC, itRect->left-1, itRect->top + topdiv-1, itRect->left+leftdiv+1, itRect->bottom - bottomdiv+1);

       if (rightdiv > 0)
          Rectangle(hDC, itRect->right - rightdiv-1, itRect->top + topdiv-1, itRect->right+1, itRect->bottom - bottomdiv+1);

       if (topdiv > 0)
             Rectangle(hDC, itRect->left-1, itRect->top-1, itRect->right+1, itRect->top + topdiv+1);

       if (bottomdiv > 0)
             Rectangle(hDC, itRect->left-1, itRect->bottom - bottomdiv-1, itRect->right+1, itRect->bottom+1);
    }

    SelectObject(hDC, oB);
    SelectObject(hDC, oP);
    DeleteObject(hpen);
}




BOOL DrawBitmapButton(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL MsgEnabled)
{
    DRAWITEMSTRUCT * dis;
    HDC hDC;
    HBITMAP hBmp = NULL;
    LONG i;
    RECT r, ri;
    LONG rc;
    HPEN oP, nP;
    BITMAP bmpInfo;
    LONG left = 0, top = 0;
    POINT lp;
    HPALETTE hP;
    ULONG desth, destw;

    dis = (DRAWITEMSTRUCT *) lParam;

    RedrawScrollingButton = dis->hwndItem;
    if (ScrollingButton == dis->hwndItem) return FALSE;
    if (addr->ColorPalette)
    {
       hP = SelectPalette(dis->hDC, addr->ColorPalette, 0);
       RealizePalette(dis->hDC);
    }

    SEARCHBMP(addr, i, dis->CtlID);

    ri = dis->rcItem;
    r = ri;

    /* Don't enable focus or select for inactive dialog */
    if (!MsgEnabled) dis->itemState &= ~(ODS_SELECTED | ODS_FOCUS);

    if (VALIDBMP(addr, i, dis->CtlID))
    {
       if ((dis->itemState & ODS_DISABLED) == ODS_DISABLED)
       {
          if (addr->BmpTab[i].bmpDisableID)
             hBmp = (HBITMAP)addr->BmpTab[i].bmpDisableID;
          else
             hBmp = (HBITMAP)addr->BmpTab[i].bitmapID;
       }
       else
       if ((dis->itemState & ODS_SELECTED) == ODS_SELECTED)
       {
          if (addr->BmpTab[i].bmpSelectID)
             hBmp = (HBITMAP)addr->BmpTab[i].bmpSelectID;
          else
             hBmp = (HBITMAP)addr->BmpTab[i].bitmapID;
          /* for a 3D effect */
             if (addr->BmpTab[i].Frame)
          {
             r.top += 3;
              r.left += 3;
             r.right += 3;
             r.bottom += 3;
          }
       }
       else
       if ((dis->itemState & ODS_FOCUS) == ODS_FOCUS)
       {
          if (addr->BmpTab[i].bmpFocusID)
             hBmp = (HBITMAP)addr->BmpTab[i].bmpFocusID;
          else
             hBmp = (HBITMAP)addr->BmpTab[i].bitmapID;
       }
       else
          hBmp = (HBITMAP)addr->BmpTab[i].bitmapID;

       if ((addr->BmpTab[i].displacex) || (addr->BmpTab[i].displacey))
       {
          r.top = r.top + addr->BmpTab[i].displacey;
          r.bottom = r.bottom + addr->BmpTab[i].displacey;
          r.left = r.left + addr->BmpTab[i].displacex;
          r.right = r.right + addr->BmpTab[i].displacex;
          if (r.left<0)
          {
             left = abs(r.left);
             r.left = 0;
          }
          if (r.top<0)
          {
             top = abs(r.top);
             r.top = 0;
          }
       }

       if (hBmp)
       {
          if (!addr->BmpTab[i].Loaded)
          {
             hDC = CreateCompatibleDC(dis->hDC);
             SelectObject(hDC,hBmp);

             GetObject(hBmp, sizeof(BITMAP), &bmpInfo);
                r.right = r.left + bmpInfo.bmWidth;
             r.bottom = r.top + bmpInfo.bmHeight;

             BitBlt(dis->hDC, r.left, r.top, r.right, r.bottom, hDC, left, top, SRCCOPY);
          }
          /* this if has been added because of a violation error moving animated button dialogs a lot */
          else if ((ULONG_PTR)addr->BmpTab[i].bmpFocusID + (ULONG_PTR)addr->BmpTab[i].bmpSelectID +
                   (ULONG_PTR)addr->BmpTab[i].bitmapID + (ULONG_PTR)addr->BmpTab[i].bmpDisableID > 0)
          {
             /* is the stretching activated? */
             if ((addr->BmpTab[i].Loaded & 0x0100) == 0x0100)
             {
                destw = r.right - r.left;
                desth = r.bottom - r.top;
             }
             else
             {
                destw = DIB_WIDTH(hBmp);                      // dest width
                desth = DIB_HEIGHT(hBmp);                      // dest height
             }
             StretchDIBits(dis->hDC,
                r.left,                     // dest x
                r.top,                     // dest y
                destw,                      // dest width
                desth,                      // dest height
                left,                     // src x
                top,                     // src y
                DIB_WIDTH(hBmp),                      // src width
                DIB_HEIGHT(hBmp),                      // src height
                DIB_PBITS(hBmp),        // bits
                DIB_PBI(hBmp),          // BITMAPINFO
                DIB_RGB_COLORS,
                SRCCOPY);               // rop
                r.right = r.left + destw;
                r.bottom = r.top + desth;
          }

           DrawBmpBackground(addr, dis->CtlID, dis->hDC, &ri, &r, wParam, lParam, left, top);

             if (addr->BmpTab[i].Frame)
          {
            rc = FrameRect(dis->hDC, (LPRECT)&dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            /* draw 3D effect */
            if (((dis->itemState & ODS_SELECTED) == ODS_SELECTED) && !(dis->itemState & ODS_DISABLED))
            {
               nP = CreatePen(PS_SOLID, 2, RGB(120,120,120));
               oP = (HPEN)SelectObject(dis->hDC, nP);
               MoveToEx(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+2, &lp);
               LineTo(dis->hDC, dis->rcItem.right-2, dis->rcItem.top+2);
               MoveToEx(dis->hDC, dis->rcItem.left+2, dis->rcItem.bottom-2, &lp);
               LineTo(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+1);

               SelectObject(dis->hDC, oP);
               DeleteObject(nP);
            }
            else
            {
               /* white line */
               nP = CreatePen(PS_SOLID, 2, RGB(240,240,240));
               oP = (HPEN)SelectObject(dis->hDC, nP);
               MoveToEx(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+2, &lp);
               LineTo(dis->hDC, dis->rcItem.right-2, dis->rcItem.top+2);
               MoveToEx(dis->hDC, dis->rcItem.left+2, dis->rcItem.bottom-2, &lp);
               LineTo(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+1);

               SelectObject(dis->hDC, oP);
               DeleteObject(nP);

               /* grey line */
               nP = CreatePen(PS_SOLID, 2, RGB(120,120,120));
               oP = (HPEN)SelectObject(dis->hDC, nP);
               MoveToEx(dis->hDC, dis->rcItem.right-2, dis->rcItem.top+2, &lp);
               LineTo(dis->hDC, dis->rcItem.right-2, dis->rcItem.bottom-2);
               MoveToEx(dis->hDC, dis->rcItem.left+2, dis->rcItem.bottom-2, &lp);
               LineTo(dis->hDC, dis->rcItem.right-2, dis->rcItem.bottom-2);

               SelectObject(dis->hDC, oP);
               DeleteObject(nP);
            }

               if (((dis->itemState & ODS_FOCUS) == ODS_FOCUS)
               && !(dis->itemState & ODS_DISABLED)
               && !(dis->itemState & ODS_SELECTED))
            {
               nP = CreatePen(PS_DOT, 1, RGB(0,0,0));
               oP = (HPEN)SelectObject(dis->hDC, nP);
               MoveToEx(dis->hDC, dis->rcItem.left+4, dis->rcItem.top+4, &lp);
               LineTo(dis->hDC, dis->rcItem.right-4, dis->rcItem.top+4);
               LineTo(dis->hDC, dis->rcItem.right-4, dis->rcItem.bottom-4);
               LineTo(dis->hDC, dis->rcItem.left+4, dis->rcItem.bottom-4);
               LineTo(dis->hDC, dis->rcItem.left+4, dis->rcItem.top+4);
               SelectObject(dis->hDC, oP);
               DeleteObject(nP);
            }
          }
          if (!addr->BmpTab[i].Loaded)
             DeleteDC(hDC);

          if (addr->ColorPalette)
             SelectPalette(dis->hDC, hP, 0);

          return TRUE;
       }
    }
    r.left = 0;
    r.top = 0;
    r.right = ri.left;
    r.bottom = ri.top;

    DrawBmpBackground(addr, dis->CtlID, dis->hDC, &ri, &r, wParam, lParam, left, top);

    if (addr->ColorPalette)
        SelectPalette(dis->hDC, hP, 0);
    return FALSE;
}




BOOL DrawBackgroundBmp(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   HDC hDC;
   PAINTSTRUCT ps;
   HPALETTE hP;
   RECT r;
   LONG desth, destw;


   hDC = BeginPaint(hDlg, &ps);

   if (addr->ColorPalette)
   {
      hP = SelectPalette(hDC, addr->ColorPalette, 0);
      RealizePalette(hDC);
   }

   GetClientRect(hDlg, &r);


   destw = DIB_WIDTH(addr->BkgBitmap);                      // dest width
   desth = DIB_HEIGHT(addr->BkgBitmap);                      // dest height


   if (r.right - r.left > destw)
      destw = r.right - r.left;
   if (r.bottom - r.top > desth)
      desth = r.bottom - r.top;

   StretchDIBits(hDC,
                 0,                     // dest x
                 0,                     // dest y
                 destw,
                 desth,
                 0,                     // src x
                 0,                     // src y
                 DIB_WIDTH(addr->BkgBitmap),                      // src width
                 DIB_HEIGHT(addr->BkgBitmap),                      // src height
                 DIB_PBITS(addr->BkgBitmap),        // bits
                 DIB_PBI(addr->BkgBitmap),          // BITMAPINFO
                 DIB_RGB_COLORS,
                 SRCCOPY);               // rop

   return EndPaint(hDlg, &ps);
}




#define ASSIGNBMP(slot, field, bnr) \
      if (inmem) \
      { \
         dlgAdm->BmpTab[slot].field = (HBITMAP)string2pointer(buffer[bnr]); \
         dlgAdm->BmpTab[slot].Loaded = 2; \
      } \
      else \
      if ((atoi(buffer[bnr])) || (buffer[bnr][0] == '0') || (buffer[bnr][0] == '\0')) \
         dlgAdm->BmpTab[slot].field = LoadBitmap(dlgAdm->TheInstance, MAKEINTRESOURCE(atoi(buffer[bnr]))); \
      else \
      { \
         dlgAdm->BmpTab[slot].Loaded  = 1; \
         dlgAdm->BmpTab[slot].field  = (HBITMAP)LoadDIB(buffer[bnr]); \
      }



/* handle the bitmap buttons that are stored in the bitmap table */

size_t RexxEntry BmpButton(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   DEF_ADM;

   CHECKARGL(3);

   GET_ADM;
   if (!dlgAdm) RETERR

   if (argv[1].strptr[0] == 'C')     /* change a bitmap button */
   {
       register INT i, id;
       const char * buffer[5];
       const char *optb;
       BOOL inmem = FALSE;

       CHECKARGL(4);

       id = atoi(argv[2].strptr);   /* buffer[0] not used */
       buffer[1] = argv[3].strptr;
       if (argc > 4)
          buffer[2] = argv[4].strptr;
       else buffer[2] = "0";
       if (argc > 5)
          buffer[3]=argv[5].strptr;
       else buffer[3] = "0";
       if (argc > 6)
          buffer[4]= argv[6].strptr;
       else buffer[4] = "0";

       if (argc > 7) optb = argv[7].strptr; else optb = "";

       if (strstr(optb, "INMEMORY")) inmem = TRUE; else inmem = FALSE;

       SEARCHBMP(dlgAdm, i, id);
       if VALIDBMP(dlgAdm, i, id)
       {
          if ((dlgAdm->BmpTab[i].Loaded & 0x1011) == 1)
          {
             if (dlgAdm->BmpTab[i].bitmapID) LocalFree((void *)dlgAdm->BmpTab[i].bitmapID);
             if (dlgAdm->BmpTab[i].bmpFocusID) LocalFree((void *)dlgAdm->BmpTab[i].bmpFocusID);
             if (dlgAdm->BmpTab[i].bmpSelectID) LocalFree((void *)dlgAdm->BmpTab[i].bmpSelectID);
             if (dlgAdm->BmpTab[i].bmpDisableID) LocalFree((void *)dlgAdm->BmpTab[i].bmpDisableID);
          }
          else
          if (dlgAdm->BmpTab[i].Loaded == 0)
          {
             if (dlgAdm->BmpTab[i].bitmapID) DeleteObject((HBITMAP)dlgAdm->BmpTab[i].bitmapID);
             if (dlgAdm->BmpTab[i].bmpFocusID) DeleteObject((HBITMAP)dlgAdm->BmpTab[i].bmpFocusID);
             if (dlgAdm->BmpTab[i].bmpSelectID) DeleteObject((HBITMAP)dlgAdm->BmpTab[i].bmpSelectID);
             if (dlgAdm->BmpTab[i].bmpDisableID) DeleteObject((HBITMAP)dlgAdm->BmpTab[i].bmpDisableID);
          }

          dlgAdm->BmpTab[i].Loaded = 0;

          ASSIGNBMP(i, bitmapID, 1);
          ASSIGNBMP(i, bmpFocusID, 2);
          ASSIGNBMP(i, bmpSelectID, 3);
          ASSIGNBMP(i, bmpDisableID, 4);

          if (strstr(optb, "FRAME"))
          {
             if (isYes(buffer[4]))
                dlgAdm->BmpTab[i].Frame = TRUE;
             else
                dlgAdm->BmpTab[i].Frame = FALSE;
          }
          if (strstr(optb, "STRETCH") && (dlgAdm->BmpTab[i].Loaded)) dlgAdm->BmpTab[i].Loaded |= 0x0100;

          /* set the palette that fits with the bitmap */
          if (strstr(optb, "USEPAL"))
          {
             if (dlgAdm->ColorPalette) DeleteObject(dlgAdm->ColorPalette);
             dlgAdm->ColorPalette = CreateDIBPalette((LPBITMAPINFO)dlgAdm->BmpTab[i].bitmapID);
             SetSysPalColors(dlgAdm->ColorPalette);
          }

          if (!strstr(optb, "NODRAW"))
              DrawButton(dlgAdm, id);
          RETC(0)
       }
       RETC(1)
   }
   else
   if (argv[1].strptr[0] == 'D')     /* draw a bitmap button */
   {
        HDC hDC, hDC2;
        HBITMAP hBmp = NULL;
        LONG cid, x, y, xs, ys, xl, yl;
        register INT i;
        BITMAP bmpInfo;
        HPALETTE hP;
        HWND hW;

        CHECKARG(10);

        cid = atoi(argv[3].strptr);

        SEARCHBMP(dlgAdm, i, cid);
        if (VALIDBMP(dlgAdm, i, cid))
        {
           hBmp = (HBITMAP)dlgAdm->BmpTab[i].bitmapID;

           if (hBmp)
           {
              hW = (HWND)GET_HWND(argv[2]);
              x = atoi(argv[4].strptr);
              y = atoi(argv[5].strptr);
              xs = atoi(argv[6].strptr);
              ys = atoi(argv[7].strptr);
              xl = atoi(argv[8].strptr);
              yl = atoi(argv[9].strptr);

              hDC = GetDC(hW);
              if (dlgAdm->ColorPalette)
              {
                 hP = SelectPalette(hDC, dlgAdm->ColorPalette, 0);
                 RealizePalette(hDC);
              }
              if (!dlgAdm->BmpTab[i].Loaded)
              {
                 hDC2 = CreateCompatibleDC(hDC);
                 SelectObject(hDC2,hBmp);

                 GetObject(hBmp, sizeof(BITMAP), &bmpInfo);
                 if (!xl) xl = bmpInfo.bmWidth;
                 if (!yl) yl = bmpInfo.bmHeight;

                 BitBlt(hDC, x, y, xl, yl, hDC2, xs, ys, SRCCOPY);
                 DeleteDC(hDC2);
              }
              /* this if has been added because of a violation error moving animated button dialogs a lot */
              else if (dlgAdm->BmpTab[i].bitmapID || dlgAdm->BmpTab[i].bmpFocusID || dlgAdm->BmpTab[i].bmpSelectID)
              {
                 if (!xl) xl = DIB_WIDTH(hBmp);
                 if (!yl) yl = DIB_HEIGHT(hBmp);

                 StretchDIBits(hDC,x,y,xl,yl,xs,ys,xl,yl,
                    DIB_PBITS(hBmp),        // bits
                    DIB_PBI(hBmp),          // BITMAPINFO
                    DIB_RGB_COLORS,
                    SRCCOPY);               // rop
              }

              if (dlgAdm->ColorPalette)
                 SelectPalette(hDC, hP, 0);
              ReleaseDC(hW, hDC);
              RETC(0)
           }
        }
        RETC(1)
   }
   else
   if (argv[1].strptr[0] == 'S')     /* set displacement of a bitmap button */
   {
       register INT i, id;
       CHECKARG(5);

       id = atoi(argv[2].strptr);

       SEARCHBMP(dlgAdm, i, id);
       if VALIDBMP(dlgAdm, i, id)
       {
          dlgAdm->BmpTab[i].displacex = atoi(argv[3].strptr);
          dlgAdm->BmpTab[i].displacey = atoi(argv[4].strptr);
          RETC(0)
       }
       RETC(1)
   }
   else
   if (argv[1].strptr[0] == 'G')     /* get displacement of a bitmap button */
   {
       register INT i, id;

       id = atoi(argv[2].strptr);

       SEARCHBMP(dlgAdm, i, id);
       if VALIDBMP(dlgAdm, i, id)
       {
          sprintf(retstr->strptr, "%ld %ld", dlgAdm->BmpTab[i].displacex, dlgAdm->BmpTab[i].displacey);
          retstr->strlength = strlen(retstr->strptr);
          return 0;
       }
       RETC(0)
   }
   else
   if (argv[1].strptr[0] == 'A')     /* add a bitmap button */
   {
       const char *buffer[5];
       BOOL frame = FALSE;
       BOOL inmem = FALSE;
       BOOL strch = FALSE;

       CHECKARGL(4);
       if (!dlgAdm->BmpTab)
       {
          dlgAdm->BmpTab = (BITMAPTABLEENTRY *)LocalAlloc(LMEM_FIXED, sizeof(BITMAPTABLEENTRY) * MAX_BT_ENTRIES);
          if (!dlgAdm->BmpTab)
          {
             MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
             return 0;
          }
          dlgAdm->BT_size = 0;
       }

       buffer[0] = argv[3].strptr;
       buffer[1] = argv[4].strptr;
       if (argc > 5)
          buffer[2]=argv[5].strptr;
       else buffer[2]="0";
       if (argc > 6)
          buffer[3]=argv[6].strptr;
       else buffer[3]="0";
       if (argc > 7)
          buffer[4]=argv[7].strptr;
       else buffer[4]="0";

       if (argc > 8)
       {
          if (strstr(argv[8].strptr, "FRAME")) frame = TRUE; else frame = FALSE;
          if (strstr(argv[8].strptr, "INMEMORY")) inmem = TRUE; else inmem = FALSE;
          if (strstr(argv[8].strptr, "STRETCH")) strch = TRUE; else strch = FALSE;
       }

       if (dlgAdm->BT_size < MAX_BT_ENTRIES)
       {
          dlgAdm->BmpTab[dlgAdm->BT_size].Loaded  = 0;
          dlgAdm->BmpTab[dlgAdm->BT_size].buttonID  = atoi(buffer[0]);
          ASSIGNBMP(dlgAdm->BT_size, bitmapID, 1);
          ASSIGNBMP(dlgAdm->BT_size, bmpFocusID, 2);
          ASSIGNBMP(dlgAdm->BT_size, bmpSelectID, 3);
          ASSIGNBMP(dlgAdm->BT_size, bmpDisableID, 4);

          dlgAdm->BmpTab[dlgAdm->BT_size].Frame  = frame;
          dlgAdm->BmpTab[dlgAdm->BT_size].displacex = 0;
          dlgAdm->BmpTab[dlgAdm->BT_size].displacey = 0;
          if ((strch) && (dlgAdm->BmpTab[dlgAdm->BT_size].Loaded)) dlgAdm->BmpTab[dlgAdm->BT_size].Loaded |= 0x0100;

          /* set the palette that fits with the bitmap */
          if ((argc > 8) && (strstr(argv[8].strptr, "USEPAL")))
          {
             if (dlgAdm->ColorPalette) DeleteObject(dlgAdm->ColorPalette);
             dlgAdm->ColorPalette = CreateDIBPalette((LPBITMAPINFO)dlgAdm->BmpTab[dlgAdm->BT_size].bitmapID);
             SetSysPalColors(dlgAdm->ColorPalette);
          }

          if ( strlen(argv[2].strptr) == 0 )
          {
             dlgAdm->BT_size ++;
             RETC(0)
          }
          else if ( AddTheMessage(dlgAdm, WM_COMMAND, UINT32_MAX, atoi(buffer[0]), 0x0000FFFF, 0, 0, argv[2].strptr, 0) )
          {
             dlgAdm->BT_size ++;
             RETC(0)
          }
       }
       else
       {
          MessageBox(0, "Bitmap buttons have exceeded the maximum number of\n"
                        "allocated table entries. No bitmap button can be\n"
                        "added.",
                     "Error",MB_OK | MB_ICONHAND);
       }
   }
   else
   if (argv[1].strptr[0] == 'E')      /* get the size of a bitmap button */
   {
       BITMAP bmpInfo;
       register INT i, id;

       id = atoi(argv[2].strptr);

       SEARCHBMP(dlgAdm, i, id);
       if VALIDBMP(dlgAdm, i, id)
       {
          if (!dlgAdm->BmpTab[i].bitmapID) strcpy(retstr->strptr, "0 0");
          else {
              if (!dlgAdm->BmpTab[i].Loaded)
              {
                 GetObject(dlgAdm->BmpTab[i].bitmapID, sizeof(BITMAP), &bmpInfo);
                 sprintf(retstr->strptr, "%ld %ld", bmpInfo.bmWidth, bmpInfo.bmHeight);
              }
              else
              {
                 sprintf(retstr->strptr, "%ld %ld", DIB_WIDTH(dlgAdm->BmpTab[i].bitmapID), DIB_HEIGHT(dlgAdm->BmpTab[i].bitmapID));
              }
          }
          retstr->strlength = strlen(retstr->strptr);
          return 0;
       }
       RETC(0)
   }
   RETERR
}



size_t RexxEntry LoadRemoveBitmap(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HBITMAP hBmp;
   void * p;
   DEF_ADM;

   CHECKARG(3);

   GET_ADM;

   if (strstr(argv[2].strptr, "REMOVE"))
   {
      p = (void *)GET_POINTER(argv[1]);
      if (p)
      {
          LocalFree(p);
          RETC(0)
      } else RETC(1)
   }
   else
   {
      hBmp = (HBITMAP)LoadDIB(argv[1].strptr);
      if (strstr(argv[2].strptr, "USEPAL") && dlgAdm)
      {
         if (dlgAdm->ColorPalette) DeleteObject(dlgAdm->ColorPalette);
         dlgAdm->ColorPalette = CreateDIBPalette((LPBITMAPINFO)hBmp);
         SetSysPalColors(dlgAdm->ColorPalette);
      }
      RETHANDLE(hBmp);
   }
   RETC(0)
}



size_t RexxEntry ScrollTheWindow(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    HWND w;
    RECT r, rs;
    INT x, y;
    HDC hDC;
    HBRUSH hbr, oB;
    HPEN oP, hpen;
    size_t code = 1;
    BOOL err=FALSE;
    DEF_ADM;

    CHECKARG(9);

    GET_ADM;
    if ( !dlgAdm )
    {
        RETERR;
    }

    w = (HWND)GET_HWND(argv[1]);

    x=atoi(argv[2].strptr);
    y=atoi(argv[3].strptr);

    if ( GetWindowRect(w, &r) )
    {
        hDC = GetDC(w);
        rs.left = atoi(argv[4].strptr);
        rs.top = atoi(argv[5].strptr);
        rs.right = atoi(argv[6].strptr);
        rs.bottom = atoi(argv[7].strptr);
        r.right = r.right - r.left;
        r.bottom = r.bottom - r.top;
        r.left = 0;
        r.top = 0;

        if ( ScrollDC(hDC, x, y, &rs, &r, NULL, NULL) )
        {
            code = 0;
        }

        if ( isYes(argv[8].strptr) )
        {
           /* draw rectangle with background color */
           if (dlgAdm->BkgBrush)
           {
               hbr = dlgAdm->BkgBrush;
           }
           else
           {
               hbr = GetSysColorBrush(COLOR_BTNFACE);
           }

           hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
           oP = (HPEN)SelectObject(hDC, hpen);
           oB = (HBRUSH)SelectObject(hDC, hbr);

           if ( x > 0 )
           {
               Rectangle(hDC, rs.left, rs.top, rs.left + x, rs.bottom);
           }
           else if ( x < 0 )
           {
               Rectangle(hDC, rs.right+x, rs.top, rs.right, rs.bottom);
           }

           if ( y > 0 )
           {
               Rectangle(hDC, rs.left, rs.top, rs.right, rs.top+y);
           }
           else if ( y < 0 )
           {
               Rectangle(hDC, rs.left, rs.bottom+y, rs.right, rs.bottom);
           }

           SelectObject(hDC, oB);
           SelectObject(hDC, oP);
           DeleteObject(hpen);
        }
        ReleaseDC(w, hDC);
    }
    RETC(code)
}


WORD NumDIBColorEntries(LPBITMAPINFO lpBmpInfo)
{
    LPBITMAPINFOHEADER lpBIH;
    LPBITMAPCOREHEADER lpBCH;
    WORD wColors, wBitCount;

    if (!lpBmpInfo) return 0;

    lpBIH = &(lpBmpInfo->bmiHeader);
    lpBCH = (LPBITMAPCOREHEADER) lpBIH;

    //
    // start off by assuming the color table size from
    // the bit per pixel field
    //

    if (lpBIH->biSize == sizeof(BITMAPINFOHEADER)) {
        wBitCount = lpBIH->biBitCount;
    } else {
        wBitCount = lpBCH->bcBitCount;
    }

    switch (wBitCount) {
    case 1:
        wColors = 2;
        break;
    case 4:
        wColors = 16;
        break;
    case 8:
        wColors = 256;
        break;
    case 24:
    default:
        wColors = 0;
        break;
    }

    //
    // If this is a Windows DIB, then the color table length
    // is determined by the biClrUsed field
    //

    if (lpBIH->biSize == sizeof(BITMAPINFOHEADER)) {
        if (lpBIH->biClrUsed != 0) {
            wColors = (WORD)lpBIH->biClrUsed;
        }
    }

    return wColors;
}


LPBITMAPINFO LoadDIB(const char *szFile)
{
    int fd;
    OFSTRUCT os;
    BITMAPFILEHEADER BmpFileHdr;
    BITMAPINFOHEADER BmpInfoHdr;
    BITMAPCOREHEADER BmpCoreHdr;
    WORD wColors, wColorTableSize, wBytes, w;
    DWORD dwBISize, dwBitsSize, dwBytes, dwSize;
    LPBITMAPINFO pBmpInfo = NULL;
    HPSTR pBits = NULL;
    BOOL bIsPM = FALSE;
    RGBTRIPLE rgbt;
    LPRGBQUAD lpRGB;


    fd = OpenFile(szFile, &os, OF_READ);
    if (fd < 1)
    {
        char *msg;
        char *errBuff;
        DWORD err = GetLastError();

        msg = (char *)LocalAlloc(LPTR, 512);
        if ( msg )
        {
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errBuff, 0, NULL);

            _snprintf(msg, 512, "Failed to open the bitmap file: %s.\n\nSystem error (%u):\n%s",
                      szFile, err, errBuff);
            MessageBox(NULL, msg,  "ooDialog Error", MB_OK | MB_ICONASTERISK);

            LocalFree(msg);
            LocalFree(errBuff);
        }
        return NULL;
    }

    wBytes = _lread(fd, (LPSTR)&BmpFileHdr, sizeof(BmpFileHdr));
    if (wBytes != sizeof(BmpFileHdr)) goto $abort;

    if (BmpFileHdr.bfType != 0x4D42) goto $abort;

    //
    // make a wild guess that the file is in Windows DIB
    // format and read the BITMAPINFOHEADER.  If it turns
    // out to be a PM DIB file we'll convert it later.
    //

    wBytes = _lread(fd, (LPSTR)&BmpInfoHdr, sizeof(BmpInfoHdr));
    if (wBytes != sizeof(BmpInfoHdr)) goto $abort;

    //
    // check we got a real Windows DIB file
    //

    if (BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)) {
        if (BmpInfoHdr.biSize != sizeof(BITMAPCOREHEADER)) goto $abort;

        bIsPM = TRUE;

        //
        // back up the file pointer and read the BITMAPCOREHEADER
        // and create the BITMAPINFOHEADER from it
        //

        _llseek(fd, sizeof(BITMAPFILEHEADER), SEEK_SET);

        wBytes = _lread(fd, (LPSTR)&BmpCoreHdr, sizeof(BmpCoreHdr));
        if (wBytes != sizeof(BmpCoreHdr)) goto $abort;

        BmpInfoHdr.biSize = sizeof(BITMAPINFOHEADER);
        BmpInfoHdr.biWidth = (DWORD) BmpCoreHdr.bcWidth;
        BmpInfoHdr.biHeight = (DWORD) BmpCoreHdr.bcHeight;
        BmpInfoHdr.biPlanes = BmpCoreHdr.bcPlanes;
        BmpInfoHdr.biBitCount = BmpCoreHdr.bcBitCount;
        BmpInfoHdr.biCompression = BI_RGB;
        BmpInfoHdr.biSizeImage = 0;
        BmpInfoHdr.biXPelsPerMeter = 0;
        BmpInfoHdr.biYPelsPerMeter = 0;
        BmpInfoHdr.biClrUsed = 0;
        BmpInfoHdr.biClrImportant = 0;

    }

    //
    // ok so we got a real DIB file so work out
    // how much memory we need for the BITMAPINFO
    // structure, color table and bits.  Allocate it,
    // copy the BmpInfoHdr we have so far
    // and then read in the color table from the file
    //

    wColors = NumDIBColorEntries((LPBITMAPINFO) &BmpInfoHdr);
    wColorTableSize = wColors * sizeof(RGBQUAD);
    dwBitsSize = BmpFileHdr.bfSize - BmpFileHdr.bfOffBits;
    dwBISize = (DWORD) sizeof(BITMAPINFOHEADER)
           + (DWORD) wColorTableSize;
    dwSize = dwBISize + dwBitsSize;

    pBmpInfo = (LPBITMAPINFO) LocalAlloc(LMEM_FIXED, dwSize);
    if (!pBmpInfo) goto $abort;


    memcpy(pBmpInfo, &BmpInfoHdr, sizeof(BITMAPINFOHEADER));

    if (bIsPM == FALSE) {

        //
        // read the color table from the file
        //

        wBytes = _lread(fd,
                        ((LPSTR) pBmpInfo) + sizeof(BITMAPINFOHEADER),
                        wColorTableSize);

        if (wBytes != wColorTableSize) goto $abort;

    } else {

        //
        // read each color table entry in turn and convert it
        // to Win DIB format as we go
        //

        lpRGB = (LPRGBQUAD) ((LPSTR) pBmpInfo + sizeof(BITMAPINFOHEADER));
        for (w=0; w<wColors; w++) {
            wBytes = _lread(fd, (LPSTR) &rgbt, sizeof(RGBTRIPLE));
            if (wBytes != sizeof(RGBTRIPLE)) goto $abort;
            lpRGB->rgbBlue = rgbt.rgbtBlue;
            lpRGB->rgbGreen = rgbt.rgbtGreen;
            lpRGB->rgbRed = rgbt.rgbtRed;
            lpRGB->rgbReserved = 0;
            lpRGB++;
        }
    }


    pBits = (LPSTR) pBmpInfo
          + sizeof(BITMAPINFOHEADER)
          + wColors * sizeof(RGBQUAD);


    _llseek(fd, BmpFileHdr.bfOffBits, SEEK_SET);


    dwBytes = _hread(fd, pBits, dwBitsSize);
    if (dwBytes != dwBitsSize) goto $abort;

    _lclose(fd);


    //
    // make sure it's not RLE
    //

    if (pBmpInfo->bmiHeader.biCompression != BI_RGB) goto $abort;

    return (LPBITMAPINFO) pBmpInfo;

$abort: // crap out

    if (pBmpInfo) LocalFree(pBmpInfo);
    if (fd >= 1) _lclose(fd);

    return NULL;
}



HPALETTE CreateDIBPalette(LPBITMAPINFO lpBmpInfo)
{
    LPBITMAPINFOHEADER lpBmpInfoHdr;
    HANDLE hPalMem;
    LOGPALETTE *pPal;
    HPALETTE hPal;
    LPRGBQUAD lpRGB;
    int iColors, i;

    //
    // validate the header
    //

    lpBmpInfoHdr = (LPBITMAPINFOHEADER) lpBmpInfo;

    if (!lpBmpInfoHdr) return NULL;

    //
    // get a pointer to the RGB quads and the number of colors
    // in the color table (we don't do 24 bit stuff here)
    //

    lpRGB = (LPRGBQUAD)((LPSTR)lpBmpInfoHdr + (WORD)lpBmpInfoHdr->biSize);

    iColors = NumDIBColorEntries(lpBmpInfo);

    //
    // Check we got a color table
    //

    if (!iColors) return NULL;

    //
    // allocate a log pal and fill it with the color table info
    //

    hPalMem = LocalAlloc(LMEM_MOVEABLE, sizeof(LOGPALETTE) + iColors * sizeof(PALETTEENTRY));
    if (!hPalMem) return NULL;

    pPal = (LOGPALETTE *) LocalLock(hPalMem);
    pPal->palVersion = 0x300; // Windows 3.0
    pPal->palNumEntries = iColors; // table size
    for (i=0; i<iColors; i++) {
        pPal->palPalEntry[i].peRed = lpRGB[i].rgbRed;
        pPal->palPalEntry[i].peGreen = lpRGB[i].rgbGreen;
        pPal->palPalEntry[i].peBlue = lpRGB[i].rgbBlue;
        pPal->palPalEntry[i].peFlags = 0;
    }

    hPal = CreatePalette(pPal);
    LocalUnlock(hPalMem);
    LocalFree(hPalMem);

    return hPal;
}


HPALETTE CopyPalette(HPALETTE hSrcPal)
{
    HANDLE hPalMem;
    LOGPALETTE *pPal;
    HPALETTE hDstPal;
    int iEntries;


    GetObject(hSrcPal, sizeof(iEntries), (LPSTR)&iEntries); // get no. of pal colors
    if (!iEntries) return NULL;

    hPalMem = LocalAlloc(LMEM_MOVEABLE, sizeof(LOGPALETTE) + iEntries * sizeof(PALETTEENTRY));
    if (!hPalMem) return NULL;

    pPal = (LOGPALETTE *) LocalLock(hPalMem);
    pPal->palVersion = 0x300; // Windows 3.0
    pPal->palNumEntries = iEntries; // table size
    GetPaletteEntries(hSrcPal, 0, iEntries, pPal->palPalEntry);

    hDstPal = CreatePalette(pPal);
    LocalUnlock(hPalMem);
    LocalFree(hPalMem);

    return hDstPal;
}

//
// Modify a palette to have the system colors in the first
// and last 10 positions
//

void SetSysPalColors(HPALETTE hPal)
{
    HANDLE hPalMem;
    LOGPALETTE *pPal;
    int iEntries;
    HDC hdcScreen;

    //
    // Create a log palette with 256 entries
    //

    hPalMem = LocalAlloc(LMEM_MOVEABLE,
                         sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
    if (!hPalMem) return;

    //
    // Set the palette info from the supplied palette
    //

    pPal = (LOGPALETTE *) LocalLock(hPalMem);
    pPal->palVersion = 0x300; // Windows 3.0
    GetObject(hPal, sizeof(iEntries), (LPSTR)&iEntries);
    pPal->palNumEntries = iEntries; // table size
    GetPaletteEntries(hPal, 0, iEntries, pPal->palPalEntry);

    //
    // Copy the low 10 and high ten system palette entries
    //

    hdcScreen = GetDC(NULL);
    GetSystemPaletteEntries(hdcScreen, 0, 10, pPal->palPalEntry);
    GetSystemPaletteEntries(hdcScreen, 246, 10, &(pPal->palPalEntry[246]));
    ReleaseDC(NULL, hdcScreen);

    //
    // Write the modified entries back to the palette
    //

    SetPaletteEntries(hPal, 0, 256, pPal->palPalEntry);

    LocalUnlock(hPalMem);
    LocalFree(hPalMem);
}


//
// Process palette messages
//

LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HDC hDC;
    int i;
    HPALETTE hOldPal;


    switch (msg) {
    case WM_PALETTECHANGED:
        if ((HWND)wParam == hWnd) {
            return (LRESULT) 0; // nothing to do (it was us)
        }
    case WM_QUERYNEWPALETTE:
        if (addr->ColorPalette) {
            hDC = GetDC(hWnd);
            hOldPal = SelectPalette(hDC, addr->ColorPalette , 0);
            i = RealizePalette(hDC);
            ReleaseDC(hWnd, hDC);
            if (i > 0) {
                InvalidateRect(hWnd, NULL, TRUE); // repaint the lot
                return TRUE; // say we did something
            }
        }

        break;

    default:
        break;
    }

    return 0l; // say we did nothing
}

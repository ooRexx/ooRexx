/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodText.hpp"


/**
 * Modify a palette to have the system colors in the first and last 10
 * positions.
 *
 * @param hPal
 */
static void setSysPalColors(HPALETTE hPal)
{
    HANDLE hPalMem;
    LOGPALETTE *pPal;
    int iEntries;
    HDC hdcScreen;

    // Create a log palette with 256 entries.
    hPalMem = LocalAlloc(LMEM_MOVEABLE, sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
    if ( hPalMem == NULL )
    {
        return;
    }

    // Set the palette info from the supplied palette.
    pPal = (LOGPALETTE *)LocalLock(hPalMem);

    // Windows 3.0.
    pPal->palVersion = 0x300;
    GetObject(hPal, sizeof(iEntries), (LPSTR)&iEntries);

    // Table size
    pPal->palNumEntries = iEntries;
    GetPaletteEntries(hPal, 0, iEntries, pPal->palPalEntry);

    // Copy the low 10 and high ten system palette entries.
    hdcScreen = GetDC(NULL);
    GetSystemPaletteEntries(hdcScreen, 0, 10, pPal->palPalEntry);
    GetSystemPaletteEntries(hdcScreen, 246, 10, &(pPal->palPalEntry[246]));
    ReleaseDC(NULL, hdcScreen);

    // Write the modified entries back to the palette.
    SetPaletteEntries(hPal, 0, 256, pPal->palPalEntry);

    LocalUnlock(hPalMem);
    LocalFree(hPalMem);
}


static RexxObjectPtr drawButton(HWND hDlg, HWND hCtrl, uint32_t id)
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


WORD numDIBColorEntries(LPBITMAPINFO lpBmpInfo)
{
    LPBITMAPINFOHEADER lpBIH;
    LPBITMAPCOREHEADER lpBCH;
    WORD wColors, wBitCount;

    if ( lpBmpInfo == NULL )
    {
        return 0;
    }

    lpBIH = &(lpBmpInfo->bmiHeader);
    lpBCH = (LPBITMAPCOREHEADER)lpBIH;

    // Start off by assuming the color table size from the bit per pixel field.

    if ( lpBIH->biSize == sizeof(BITMAPINFOHEADER) )
    {
        wBitCount = lpBIH->biBitCount;
    }
    else
    {
        wBitCount = lpBCH->bcBitCount;
    }

    switch ( wBitCount )
    {
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

    // If this is a Windows DIB, then the color table length is determined by
    // the biClrUsed field
    if ( lpBIH->biSize == sizeof(BITMAPINFOHEADER) )
    {
        if ( lpBIH->biClrUsed != 0 )
        {
            wColors = (WORD)lpBIH->biClrUsed;
        }
    }

    return wColors;
}


/**
 * Parses the show options for a number of methods that have to do with the
 * moving, resizing, or both, of windows
 *
 * @param options  The keyword string.  This is a case-insensitive check. More
 *                 than one keyword, or no keyword, is acceptable.
 *
 * @return uint32_t
 */
uint32_t parseShowOptions(CSTRING options)
{
    uint32_t opts = SWP_NOZORDER;

    if ( options != NULL )
    {
       if ( StrStrI(options, "NOMOVE"    ) ) opts |= SWP_NOMOVE;
       if ( StrStrI(options, "NOSIZE"    ) ) opts |= SWP_NOSIZE;
       if ( StrStrI(options, "HIDEWINDOW") ) opts |= SWP_HIDEWINDOW;
       if ( StrStrI(options, "SHOWWINDOW") ) opts |= SWP_SHOWWINDOW;
       if ( StrStrI(options, "NOREDRAW"  ) ) opts |= SWP_NOREDRAW;
    }
    return opts;
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
 * Add, or replace, an entry in the color table for the specified control.
 *
 * @param c            Method context we are operating in.
 * @param dlgAdm       Pointer to the dialog admin block for the dialog the
 *                     control is within.
 * @param id           Resource ID of the control.
 * @param bkColor      Background color for the control.
 * @param fgColor      Optional foreground color for the control.  Use -1 to
 *                     indicate the foreground color should not be changed.
 * @param useSysColor  Whether the color index for the background color is a
 *                     system color index, or not.
 *
 * @return  0 on no error, otherwise 1.
 */
logical_t oodColorTable(RexxMethodContext *c, DIALOGADMIN *dlgAdm, uint32_t id,
                        int32_t bkColor, int32_t fgColor, bool useSysColor)
{
    if ( dlgAdm->ColorTab == NULL )
    {
        dlgAdm->ColorTab = (COLORTABLEENTRY *)LocalAlloc(LMEM_FIXED, sizeof(COLORTABLEENTRY) * MAX_CT_ENTRIES);
        if ( dlgAdm->ColorTab == NULL )
        {
            outOfMemoryException(c->threadContext);
            return 1;
        }
        dlgAdm->CT_size = 0;
    }

    if ( dlgAdm->CT_size < MAX_CT_ENTRIES )
    {
        uint32_t i = 0;

        // If brush is found, then an entry for this control already exists in
        // the color table.  The value of i is set to the entry index.  In this
        // case we are replacing the entry with a (presumably) new color.
        HBRUSH hbrush = searchForBrush(dlgAdm, &i, id);
        if ( hbrush != NULL )
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

        dlgAdm->ColorTab[i].ColorBk = bkColor;
        dlgAdm->ColorTab[i].ColorFG = fgColor;

        if ( useSysColor )
        {
            dlgAdm->ColorTab[i].ColorBrush = GetSysColorBrush(dlgAdm->ColorTab[i].ColorBk);
            dlgAdm->ColorTab[i].isSysBrush = true;
        }
        else
        {
            dlgAdm->ColorTab[i].ColorBrush = CreateSolidBrush(PALETTEINDEX(dlgAdm->ColorTab[i].ColorBk));
            dlgAdm->ColorTab[i].isSysBrush = false;
        }
    }
    else
    {
        MessageBox(NULL, "Dialog control elements have exceeded the maximum\n"
                   "number of allocated color table entries. The color\n"
                   "for the dialog control can not be added.",
                   "Error", MB_OK | MB_ICONHAND);
        return 1;
    }
    return 0;
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

/**
 * Given the point size of a font, calculate its height.  This is for the
 * display device, i.e., not for a printer device.
 *
 * @param fontSize  The point size of the font.
 *
 * @return A calculated height for the font.
 */
int getHeightFromFontSize(int fontSize)
{
    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    int height = -MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    DeleteDC(hdc);
    return height;
}

/**
 *  Sets the color palette of the bitmap as the system color palette, maybe.
 *
 *  A number of methods dealing with device independent bitmaps have an option
 *  that specifies that the patllete of the bitmap be used for the system color
 *  pallete.  This function implements that.
 *
 *  In general, it is safe to pass anything in, iff all the proper conditions
 *  are met, then the system color palette is set.  Otherwise nothing is done.
 *
 *  @param c       Method context we are operating in.
 *  @param hBmp    Handle to the bitmap.  Can be null, then nothing is done.
 *  @param opts    An option string, can be null.  If the string is null, or the
 *                 string does not contain the keyword "USEPAL" then nothing is
 *                 done.
 *  @param dlgAdm  Pointer to a dialog admin block.  If it is null, then an
 *                 attempt is made to get the admin block from the self object.
 *  @param self    Self could be either a dialog or a dialog control.  If it is
 *                 neither, and dlgAdm was null, then nothing is done.
 */
void maybeSetColorPalette(RexxMethodContext *c, HBITMAP hBmp, CSTRING opts, DIALOGADMIN *dlgAdm, RexxObjectPtr self)
{
    if ( hBmp != NULL && opts != NULL && StrStrI(opts, "USEPAL") != NULL )
    {
        if ( dlgAdm == NULL && self != NULLOBJECT )
        {
            dlgAdm = getDlgAdm(c, self);
        }

        if ( dlgAdm != NULL )
        {
            if ( dlgAdm->ColorPalette )
            {
                DeleteObject(dlgAdm->ColorPalette);
            }
            dlgAdm->ColorPalette = CreateDIBPalette((LPBITMAPINFO)hBmp);
            setSysPalColors(dlgAdm->ColorPalette);
        }
        else
        {
            c->ClearCondition();
        }
    }
}

LPBITMAPINFO loadDIB(const char *szFile)
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

    wColors = numDIBColorEntries((LPBITMAPINFO) &BmpInfoHdr);
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


/* draw the part that will not be covered by the bitmap */
void DrawBmpBackground(DIALOGADMIN * dlgAdm, pCPlainBaseDialog pcpbd, INT id, HDC hDC, RECT * itRect, RECT * bmpRect,
                       WPARAM wParam, LPARAM lParam, LONG left, LONG top)
{
    HBRUSH hbr = NULL, oB;
    HPEN oP, hpen;
    INT topdiv, leftdiv, rightdiv, bottomdiv;

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

    // Check to see if the user has set a color for dialog item background.
    if ( dlgAdm->CT_size != 0 )
    {
        uint32_t i;
        hbr = searchForBrush(dlgAdm, &i, id);
    }

    if ( hbr == NULL )
    {
        if ( pcpbd->bkgBrush )
        {
            hbr = pcpbd->bkgBrush;
        }
        else
        {
            // hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);  Old code, should be below, I think.  Revert if problems
            hbr = GetSysColorBrush(COLOR_3DFACE);
        }
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


BOOL DrawBitmapButton(DIALOGADMIN *dlgAdm, pCPlainBaseDialog pcpbd, WPARAM wParam, LPARAM lParam, bool msgEnabled)
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
    if (dlgAdm->ColorPalette)
    {
       hP = SelectPalette(dis->hDC, dlgAdm->ColorPalette, 0);
       RealizePalette(dis->hDC);
    }

    SEARCHBMP(dlgAdm, i, dis->CtlID);

    ri = dis->rcItem;
    r = ri;

    /* Don't enable focus or select for inactive dialog */
    if ( ! msgEnabled )
    {
       dis->itemState &= ~(ODS_SELECTED | ODS_FOCUS);
    }

    if (VALIDBMP(dlgAdm, i, dis->CtlID))
    {
       if ((dis->itemState & ODS_DISABLED) == ODS_DISABLED)
       {
          if (dlgAdm->BmpTab[i].bmpDisableID)
             hBmp = (HBITMAP)dlgAdm->BmpTab[i].bmpDisableID;
          else
             hBmp = (HBITMAP)dlgAdm->BmpTab[i].bitmapID;
       }
       else
       if ((dis->itemState & ODS_SELECTED) == ODS_SELECTED)
       {
          if (dlgAdm->BmpTab[i].bmpSelectID)
             hBmp = (HBITMAP)dlgAdm->BmpTab[i].bmpSelectID;
          else
             hBmp = (HBITMAP)dlgAdm->BmpTab[i].bitmapID;
          /* for a 3D effect */
             if (dlgAdm->BmpTab[i].Frame)
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
          if (dlgAdm->BmpTab[i].bmpFocusID)
             hBmp = (HBITMAP)dlgAdm->BmpTab[i].bmpFocusID;
          else
             hBmp = (HBITMAP)dlgAdm->BmpTab[i].bitmapID;
       }
       else
          hBmp = (HBITMAP)dlgAdm->BmpTab[i].bitmapID;

       if ((dlgAdm->BmpTab[i].displacex) || (dlgAdm->BmpTab[i].displacey))
       {
          r.top = r.top + dlgAdm->BmpTab[i].displacey;
          r.bottom = r.bottom + dlgAdm->BmpTab[i].displacey;
          r.left = r.left + dlgAdm->BmpTab[i].displacex;
          r.right = r.right + dlgAdm->BmpTab[i].displacex;
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
          if (!dlgAdm->BmpTab[i].Loaded)
          {
             hDC = CreateCompatibleDC(dis->hDC);
             SelectObject(hDC,hBmp);

             GetObject(hBmp, sizeof(BITMAP), &bmpInfo);
                r.right = r.left + bmpInfo.bmWidth;
             r.bottom = r.top + bmpInfo.bmHeight;

             BitBlt(dis->hDC, r.left, r.top, r.right, r.bottom, hDC, left, top, SRCCOPY);
          }
          /* this if has been added because of a violation error moving animated button dialogs a lot */
          else if ((ULONG_PTR)dlgAdm->BmpTab[i].bmpFocusID + (ULONG_PTR)dlgAdm->BmpTab[i].bmpSelectID +
                   (ULONG_PTR)dlgAdm->BmpTab[i].bitmapID + (ULONG_PTR)dlgAdm->BmpTab[i].bmpDisableID > 0)
          {
             /* is the stretching activated? */
             if ((dlgAdm->BmpTab[i].Loaded & 0x0100) == 0x0100)
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

           DrawBmpBackground(dlgAdm, pcpbd, dis->CtlID, dis->hDC, &ri, &r, wParam, lParam, left, top);

             if (dlgAdm->BmpTab[i].Frame)
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
          if (!dlgAdm->BmpTab[i].Loaded)
             DeleteDC(hDC);

          if (dlgAdm->ColorPalette)
             SelectPalette(dis->hDC, hP, 0);

          return TRUE;
       }
    }
    r.left = 0;
    r.top = 0;
    r.right = ri.left;
    r.bottom = ri.top;

    DrawBmpBackground(dlgAdm, pcpbd, dis->CtlID, dis->hDC, &ri, &r, wParam, lParam, left, top);

    if (dlgAdm->ColorPalette)
        SelectPalette(dis->hDC, hP, 0);
    return FALSE;
}


BOOL DrawBackgroundBmp(pCPlainBaseDialog pcpbd, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   HDC hDC;
   PAINTSTRUCT ps;
   HPALETTE hP;
   RECT r;
   LONG desth, destw;
   DIALOGADMIN *dlgAdm = pcpbd->dlgAdm;


   hDC = BeginPaint(hDlg, &ps);

   if (dlgAdm->ColorPalette)
   {
      hP = SelectPalette(hDC, dlgAdm->ColorPalette, 0);
      RealizePalette(hDC);
   }

   GetClientRect(hDlg, &r);

   destw = DIB_WIDTH(pcpbd->bkgBitmap);         // dest width
   desth = DIB_HEIGHT(pcpbd->bkgBitmap);        // dest height


   if (r.right - r.left > destw)
      destw = r.right - r.left;
   if (r.bottom - r.top > desth)
      desth = r.bottom - r.top;

   StretchDIBits(hDC,
                 0,                             // dest x
                 0,                             // dest y
                 destw,
                 desth,
                 0,                             // src x
                 0,                             // src y
                 DIB_WIDTH(pcpbd->bkgBitmap),   // src width
                 DIB_HEIGHT(pcpbd->bkgBitmap),  // src height
                 DIB_PBITS(pcpbd->bkgBitmap),   // bits
                 DIB_PBI(pcpbd->bkgBitmap),     // BITMAPINFO
                 DIB_RGB_COLORS,
                 SRCCOPY);                      // rop

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
         dlgAdm->BmpTab[slot].field  = (HBITMAP)loadDIB(buffer[bnr]); \
      }

/* handle the bitmap buttons that are stored in the bitmap table */
size_t RexxEntry BmpButton(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   DIALOGADMIN * dlgAdm = NULL;

   CHECKARGL(3);

   dlgAdm = (DIALOGADMIN *)string2pointer(&argv[0]);
   if ( dlgAdm == NULL )
   {
       RETERR
   }

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
             setSysPalColors(dlgAdm->ColorPalette);
          }

          if ( strstr(optb, "NODRAW") == 0 )
          {
              HWND hCtrl = GetDlgItem(dlgAdm->TheDlg, id);
              if ( hCtrl == NULL )
              {
                  RETC(1)
              }
              drawButton(dlgAdm->TheDlg, hCtrl, id);
          }
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
             setSysPalColors(dlgAdm->ColorPalette);
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

    iColors = numDIBColorEntries(lpBmpInfo);

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


void drawFontToDC(HDC hDC, int32_t x, int32_t y, const char * text, uint32_t fontSize, const char * opts, const char * fontName,
                  int32_t fgColor, int32_t bkColor)
{
   HFONT hFont, oldFont;
   COLORREF oldFg, oldBk;

   int weight = getWeight(opts);
   int height = getHeightFromFontSize(fontSize);

   hFont = CreateFont(height, 0, 0, 0, weight, StrStrI(opts, "ITALIC") != NULL, StrStrI(opts, "UNDERLINE") != NULL,
                      StrStrI(opts, "STRIKEOUT") != NULL, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      FF_DONTCARE, fontName);

   oldFont = (HFONT)SelectObject(hDC, hFont);

   int oldMode = 0;
   if ( StrStrI(opts, "TRANSPARENT") != NULL )
   {
       oldMode = SetBkMode(hDC, TRANSPARENT);
   }
   else if ( StrStrI(opts, "OPAQUE") != NULL )
   {
       oldMode = SetBkMode(hDC, OPAQUE);
   }

   if ( fgColor != -1 )
   {
       oldFg = SetTextColor(hDC, PALETTEINDEX(fgColor));
   }
   if  (bkColor != -1 )
   {
       oldBk = SetBkColor(hDC, PALETTEINDEX(bkColor));
   }

   TextOut(hDC, x, y, text, (int)strlen(text));

   SelectObject(hDC, oldFont);
   DeleteObject(hFont);
   if ( oldMode != 0 )
   {
       SetBkMode(hDC, oldMode);
   }
   if ( fgColor != -1 )
   {
       SetTextColor(hDC, oldFg);
   }
   if ( bkColor != -1 )
   {
       SetBkColor(hDC, oldBk);
   }
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
             hBmp = (HBITMAP) loadDIB(argv[3].strptr);     /* we have a file name */
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

RexxObjectPtr dlgExtControlSetup(RexxMethodContext *c, RexxObjectPtr self, RexxObjectPtr rxID,
                                 pCPlainBaseDialog *ppcpbd, uint32_t *pID, HWND *phCtrl)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(c, self);
    if ( pcpbd == NULL )
    {
        return TheOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
    if ( hCtrl == NULL )
    {
        oodSetSysErrCode(c->threadContext);
        return TheOneObj;
    }

    if ( ppcpbd != NULL )
    {
        *ppcpbd = pcpbd;
    }
    if ( pID != NULL )
    {
        *pID = id;
    }
    if ( phCtrl != NULL )
    {
        *phCtrl = hCtrl;
    }
    return TheZeroObj;
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


/** DialogExtension::clearRect()
 *
 *  'Clears' a rectangle in the specified Window.
 *
 *  @param hwnd         The window of the dialog or dialog control to act on.
 *  @param coordinates  The coordinates of the rectangle, given in pixels.
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Point object.
 *    Form 3:  x1, y1, x1, y2
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


/** DialogExtensions::setWindowRect()
 *
 *  Changes the size and position of the specified window.
 *
 *  By specifying either NOSIZE or NOMOVE options the programmer can only move
 *  or only resize the window.
 *
 *  @param hwnd         The window to be moved, resized, or both.
 *  @param coordinates  The coordinates of a point / size rectangle, given in
 *                      pixels
 *
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Size object.
 *    Form 3:  x1, y1, cx, cy
 *
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Microsoft says: If the SWP_SHOWWINDOW or SWP_HIDEWINDOW flag is
 *            set, the window cannot be moved or sized.  But, that does not
 *            appear to be true.
 *
 *            This method is essentially WindowBase::setRect() but works
 *            wit a supplied window handle rather then the window handle of the
 *            Rexx object.
 */
RexxMethod3(RexxObjectPtr, dlgext_setWindowRect, POINTERSTRING, hwnd, ARGLIST, args, OSELF, self)
{
    oodResetSysErrCode(context->threadContext);

    size_t countArgs;
    int    argsUsed;
    RECT   rect;
    if ( ! getRectFromArglist(context, args, &rect, false, 2, 6, &countArgs, &argsUsed) )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 3 )
        {
            return tooManyArgsException(context->threadContext, 3);
        }
        if ( countArgs == 3 )
        {
            obj = context->ArrayAt(args, 3);
            options = context->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs > 4 )
        {
            return tooManyArgsException(context->threadContext, 4);
        }
        if ( countArgs == 4 )
        {
            obj = context->ArrayAt(args, 4);
            options = context->ObjectToStringValue(obj);
        }
    }
    else
    {
        if ( countArgs == 6 )
        {
            obj = context->ArrayAt(args, 6);
            options = context->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    if ( SetWindowPos((HWND)hwnd, NULL, rect.left, rect.top, rect.right, rect.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
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
RexxMethod3(RexxObjectPtr, dlgext_redrawRect, OPTIONAL_POINTERSTRING, _hwnd, ARGLIST, args, OSELF, self)
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
 */
RexxMethod2(RexxObjectPtr, dlgext_getControlRect, RexxObjectPtr, rxID, OSELF, self)
{
    HWND hCtrl;
    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, NULL, NULL, &hCtrl);
    if ( result != TheZeroObj )
    {
        return result;
    }

    return oodGetWindowRect(context, hCtrl);
}


/** DialogExtensions::clearContolRect()
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
    HWND hCtrl;
    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, NULL, NULL, &hCtrl);
    if ( result != TheZeroObj )
    {
        return result;
    }

    RECT r = {0};
    if ( oodGetClientRect(context, hCtrl, &r) == TheOneObj )
    {
        return TheOneObj;
    }
    return clearRect(context, hCtrl, &r);
}


/** DialogExtensions::resizeControl()
 *  DialogExtensions::moveControl()
 *
 *  Resize control, changes the size of the specified dialog control.
 *
 *  Move control, changes the position of the specified dialog control.
 *
 *  @param  rxID         The resource id of the dialog control, may be symbolic
 *                       or numeric.
 *
 *  @param  coordinates  The new position (x, y) of the upper right corner of
 *                       the control, or the new size (cx, cy) for the control.
 *                       The units are pixels.
 *
 *    resizeControl()
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *    moveControl()
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that only a .Size object is used for
 *            resizeControl() and only a .Point object for moveControl().
 */
RexxMethod4(RexxObjectPtr, dlgext_resizeMoveControl, RexxObjectPtr, rxID, ARGLIST, args, NAME, method, OSELF, self)
{
    HWND hCtrl;
    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, NULL, NULL, &hCtrl);
    if ( result != TheZeroObj )
    {
        return result;
    }

    size_t arraySize;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 2, 4, &arraySize, &argsUsed) )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( arraySize > 3 )
        {
            return tooManyArgsException(context->threadContext, 3);
        }
        if ( arraySize == 3 )
        {
            // The object at index 3 has to exist, otherwise arraySize would
            // equal 2.
            obj = context->ArrayAt(args, 3);
            options = context->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( arraySize == 4 )
        {
            // The object at index 4 has to exist, otherwise arraySize would
            // equal 3.
            obj = context->ArrayAt(args, 4);
            options = context->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    RECT r = {0};

    if ( *method == 'R' )
    {
        opts |= SWP_NOMOVE;
        r.right = point.x;
        r.bottom = point.y;
    }
    else
    {
        opts |= SWP_NOSIZE;
        r.left = point.x;
        r.top = point.y;
    }

    if ( SetWindowPos(hCtrl, NULL, r.left, r.top, r.right, r.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
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
    HWND hCtrl;
    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, NULL, NULL, &hCtrl);
    if ( result != TheZeroObj )
    {
        return result;
    }

    bool doErase = argumentExists(2) ? (erase ? true : false) : false;
    return redrawRect(context, hCtrl, NULL, doErase);
}


RexxMethod2(RexxObjectPtr, dlgext_drawButton, RexxObjectPtr, rxID, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    uint32_t id;
    HWND hCtrl;
    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, &pcpbd, &id, &hCtrl);
    if ( result != TheZeroObj )
    {
        return result;
    }

    return drawButton(pcpbd->hDlg, hCtrl, id);
}


/** DialogExtensions::getMouseCapture()
 *
 *  Retrieves a handle to the window (if any) that has captured the mouse.
 *
 *  Only one window at a time can capture the mouse; this window receives mouse
 *  input whether or not the cursor is within its borders.
 *
 *  @return  A NULL return value means the current thread has not captured the
 *           mouse. However, it is possible that another thread or process has
 *           captured the mouse.
 *
 *  DialogExtensions::releaseMouseCapture()
 *
 *  Releases the mouse capture from a window in the current thread and restores
 *  normal mouse input processing.
 *
 *  @return  0 on success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode, but that only has meaning for
 *         releaseMouseCapture().
 *
 *  @remarks  GetCapture() and ReleaseCapture() need to run on the same thread
 *            as the dialog's message loop.  So we use SendMessage with one of
 *            the custom window messages.
 *
 */
RexxMethod2(RexxObjectPtr, dlgext_mouseCapture, NAME, method, OSELF, self)
{
    RexxObjectPtr result = NULLOBJECT;

    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd != NULL )
    {
        if ( *method == 'G' )
        {
            HWND hwnd = (HWND)SendMessage(pcpbd->hDlg, WM_USER_GETSETCAPTURE, 0, 0);
            result = pointer2string(context, hwnd);
        }
        else
        {
            RexxMethodContext *c = context;
            uint32_t rc = (uint32_t)SendMessage(pcpbd->hDlg, WM_USER_GETSETCAPTURE, 2,0);
            result = (rc == 0 ? TheZeroObj : c->UnsignedInt32(rc));
        }
    }
    return result;
}

/** DialogExtensions::captureMouse
 *
 *  Sets the mouse capture to this dialog window.  captureMouse() captures mouse
 *  input either when the mouse is over the dialog, or when the mouse button was
 *  pressed while the mouse was over the dialog and the button is still down.
 *  Only one window at a time can capture the mouse.
 *
 *  If the mouse cursor is over a window created by another thread, the system
 *  will direct mouse input to the specified window only if a mouse button is
 *  down.
 *
 *  @return  The window handle of the window that previously had captured the
 *           mouse, or 0 if there was no such window.
 *
 *  @note  Sets the .SystemErrorCode,
 */
RexxMethod1(RexxObjectPtr, dlgext_captureMouse, OSELF, self)
{
    RexxObjectPtr result = TheZeroObj;

    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd != NULL )
    {
        HWND oldCapture = (HWND)SendMessage(pcpbd->hDlg, WM_USER_GETSETCAPTURE, 1, (LPARAM)pcpbd->hDlg);
        result = pointer2string(context, oldCapture);
    }
    return result;
}


/** DialogExtensions::isMouseButtonDown()
 *
 *  Determines if one of the mouse buttons is down.
 *
 *  @param  whichButton  [OPTIONAL]  Keyword indicating which mouse button
 *                       should be queried. By default it is the left button
 *                       that is queried.
 *
 *  @return  True if the specified mouse button was down, otherwise false
 *
 *  @note  Sets the .SystemErrorCode, but there is nothing that would change it
 *         to not zero.
 *
 *  @remarks  The key state must be handled in the window thread, so
 *            SendMessage() has to be used.
 */
RexxMethod2(RexxObjectPtr, dlgext_isMouseButtonDown, OPTIONAL_CSTRING, whichButton, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    int mb = VK_LBUTTON;
    if ( argumentExists(1) )
    {
        if ( StrStrI(whichButton, "LEFT"  ) ) mb = VK_LBUTTON;
        else if ( StrStrI(whichButton, "RIGHT" ) ) mb = VK_RBUTTON;
        else if ( StrStrI(whichButton, "MIDDLE") ) mb = VK_MBUTTON;
        else
        {
            return wrongArgValueException(context->threadContext, 1, "LEFT, RIGHT, MIDDLE", whichButton);
        }
    }

    if ( GetSystemMetrics(SM_SWAPBUTTON) )
    {
        if ( mb == VK_LBUTTON )
        {
            mb = VK_RBUTTON;
        }
        else if ( mb == VK_RBUTTON )
        {
            mb = VK_LBUTTON;
        }
    }

    return ((short)SendMessage(pcpbd->hDlg, WM_USER_GETKEYSTATE, mb, 0) & ISDOWN) ? TheTrueObj : TheFalseObj;
}


/** DialogExtensions::setForegroundWindow()
 *
 *  Brings the specified wind to the foreground.
 *
 *  @param  hwnd  The window handle of the window to bring to the foreground.
 *
 *  @return  The handle of the window that previously was the foreground on
 *           success, 0 on failure.
 *
 *  @note  Sets the .SystemErrorCode.  In very rare cases, there might not be a
 *         previous foreground window and 0 would be returned.  In this case,
 *         the .SystemErrorCode will be 0, otherwise the .SystemErrorCode will
 *         not be 0.
 *
 *  @note  Windows no longer allows a program to arbitrarily change the
 *         foreground window.
 *
 *         The system restricts which processes can set the foreground window. A
 *         process can set the foreground window only if one of the following
 *         conditions is true:
 *
 *         The process is the foreground process.
 *         The process was started by the foreground process.
 *         The process received the last input event.
 *         There is no foreground process.
 *         No menus are active.
 *
 *         With this change, an application cannot force a window to the
 *         foreground while the user is working with another window.
 *
 * @remarks  SetForegroundWindow() is not documented as setting last error.  So,
 *           if it fails, last error is checked.  If it is set, it is used.  If
 *           it is not set, we arbitrarily use ERROR_NOTSUPPORTED.  On XP at
 *           least, last error is set to 5, access denied.
 */
RexxMethod1(RexxObjectPtr, dlgext_setForgroundWindow, RexxStringObject, hwnd)
{
    return oodSetForegroundWindow(context, (HWND)string2pointer(context, hwnd));
}


/** DialogExtensions::setControlColor()
 *  DialogExtensions::setControlSysColor
 */
RexxMethod5(int32_t, dlgext_setControlColor, RexxObjectPtr, rxID, int32_t, bkColor, OPTIONAL_int32_t, fgColor,
            NAME, method, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(context, self);

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return -1;
    }

    return (int32_t) oodColorTable(context, pcpbd->dlgAdm, id, bkColor, (argumentOmitted(3) ? -1 : fgColor),
                                   (method[10] == 'S'));
}


RexxMethod9(logical_t, dlgext_writeToWindow, POINTERSTRING, hwnd, int32_t, xPos, int32_t, yPos, CSTRING, text,
            OPTIONAL_CSTRING, fontName, OPTIONAL_uint32_t, fontSize, OPTIONAL_CSTRING, fontStyle,
            OPTIONAL_int32_t, fgColor, OPTIONAL_int32_t, bkColor)
{
    fontName  = (argumentOmitted(5) ? "System" : fontName);
    fontSize  = (argumentOmitted(6) ? 10       : fontSize);
    fontStyle = (argumentOmitted(7) ? ""       : fontStyle);
    fgColor   = (argumentOmitted(8) ? -1       : fgColor);
    bkColor   = (argumentOmitted(9) ? -1       : bkColor);

    HDC hDC = NULL;
    if ( StrStrI(fontStyle, "CLIENT") != NULL )
    {
        hDC = GetDC((HWND)hwnd);
    }
    else
    {
        hDC = GetWindowDC((HWND)hwnd);
    }
    if ( hDC != NULL )
    {
        drawFontToDC(hDC, xPos, yPos, text, fontSize, fontStyle, fontName, fgColor, bkColor);
        ReleaseDC((HWND)hwnd, hDC);
        return 0;
    }
    return 1;
}



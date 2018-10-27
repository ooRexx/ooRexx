/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>

/** Note:
 *  When building for Vista or later, tmschema.h is obsolete and vssym32.h
 *  should be used.  However, vssym32.h is not present in Windows SDKs prior to
 *  the Vista one.  Since, tmschema.h is only used for the PUSBHBUTTONSTATES
 *  enum, that enum has been replaced by a PUSHBUTTON_STATES enum defined in
 *  oodDeviceGraphics.hpp.  This is fine, for now.  In the future if more work
 *  is done using the updated features of Vista, it may be better to require a
 *  later version of the SDK.
 */
//#include <vssym32.h>
//#include <tmschema.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodDeviceGraphics.hpp"


static HWND      ScrollingButton = NULL;
static HWND      RedrawScrollingButton = NULL;

static HANDLE    TimerEvent = NULL;
static uint32_t  TimerCount = 0;
static ULONG_PTR Timer = 0;


inline BitmapButtonBMPType getBMPType(bool inMemory, bool isIntResource)
{
    BitmapButtonBMPType t = FromFileBmp;
    if ( inMemory )
    {
        t = InMemoryBmp;
    }
    else if ( isIntResource )
    {
        t = IntResourceBmp;
    }
    return t;
}

inline char *bmpType2String(BitmapButtonBMPType type)
{
    if (      type == InMemoryBmp    ) return "in memory";
    else if ( type == IntResourceBmp ) return "from resource ID";
    else if ( type == FromFileBmp    ) return "from file";
    return "unknown";
}

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

/**
 * This function is (currently) only called from oodWriteToWindow(). Previously,
 * none of the return codes were checked for error. Now we just check them in
 * order to set the .SystemErrorCode.
 *
 * SelectObject() does not set the system error code (according to the docs.)
 * It is unlikely that it fails, but just for completedness, we set it to
 * ERROR_SIGNAL_REFUSED.
 *
 * @param hDC
 * @param x
 * @param y
 * @param text
 * @param fontSize
 * @param opts
 * @param fontName
 * @param fgColor
 * @param bkColor
 *
 * @return logical_t
 *
 * Even through we return logical_t, the ooDialog methods that use this base
 * function return 0 for success, 1 for failure.  Need to keep that in mind.
 */
static logical_t drawFontToDC(RexxThreadContext *c, HDC hDC, int32_t x, int32_t y, const char * text, uint32_t fontSize,
                              const char * opts, const char * fontName, int32_t fgColor, int32_t bkColor)
{
    logical_t ret = 0;
    int       oldMode = 0;
    COLORREF  oldFg = CLR_INVALID;
    COLORREF  oldBk = CLR_INVALID;

    HFONT hFont = oodGenericFont(fontName, fontSize, opts);
    if ( hFont == NULL )
    {
        oodSetSysErrCode(c);
        return 1;
    }

    HFONT oldFont = (HFONT)SelectObject(hDC, hFont);
    if ( oldFont == NULL )
    {
        oodSetSysErrCode(c, ERROR_SIGNAL_REFUSED);
        ret = 1;
        goto clean_up;
    }

    if ( StrStrI(opts, "TRANSPARENT") != NULL )
    {
        oldMode = SetBkMode(hDC, TRANSPARENT);
        if ( oldMode == 0 )
        {
            oodSetSysErrCode(c);
            ret = 1;
            goto clean_up;
        }
    }
    else if ( StrStrI(opts, "OPAQUE") != NULL )
    {
        oldMode = SetBkMode(hDC, OPAQUE);
        if ( oldMode == 0 )
        {
            oodSetSysErrCode(c);
            ret = 1;
            goto clean_up;
        }
    }

    if ( fgColor != -1 )
    {
        oldFg = SetTextColor(hDC, PALETTEINDEX(fgColor));
        if ( oldFg == CLR_INVALID )
        {
            oodSetSysErrCode(c);
            ret = 1;
            goto clean_up;
        }
    }
    if ( bkColor != -1 )
    {
        oldBk = SetBkColor(hDC, PALETTEINDEX(bkColor));
        if ( oldFg == CLR_INVALID )
        {
            oodSetSysErrCode(c);
            ret = 1;
            goto clean_up;
        }
    }

    if ( ! TextOut(hDC, x, y, text, (int)strlen(text)) )
    {
        oodSetSysErrCode(c);
        ret = 1;
    }

clean_up:

    // We don't check for errors here.  Presumably, if something fails, it would
    // be because of an error above.  In which case the correct system error
    // code for the failure should be set and we do not want to change it.

    if ( oldFont != NULL )
    {
        SelectObject(hDC, oldFont);
    }

    // hFont is not null, or we would have already returned.
    DeleteObject(hFont);

    if ( oldMode != 0 )
    {
        SetBkMode(hDC, oldMode);
    }
    if ( fgColor != CLR_INVALID )
    {
        SetTextColor(hDC, oldFg);
    }
    if ( bkColor != CLR_INVALID )
    {
        SetBkColor(hDC, oldBk);
    }

    return ret;
}


static HPALETTE createDIBPalette(LPBITMAPINFO lpBmpInfo)
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


static bool findBmpForID(pCPlainBaseDialog pcpbd, uint32_t id, size_t *index)
{
    if ( pcpbd->BmpTab != NULL )
    {
        for ( size_t i = 0; i < pcpbd->BT_nextIndex; i++ )
        {
            if ( pcpbd->BmpTab[i].buttonID == id )
            {
                *index = i;
                return true;
            }
        }
    }
    return false;
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
 * @return The show window postion flag corresponding to the keywords found, or
 *         SWP_NOZORDER if no keywords are found.
 */
uint32_t parseShowOptions(CSTRING options)
{
    uint32_t opts = SWP_NOZORDER;

    if ( options != NULL )
    {
       if ( StrStrI(options, "NOMOVE"    ) != NULL ) opts |= SWP_NOMOVE;
       if ( StrStrI(options, "NOSIZE"    ) != NULL ) opts |= SWP_NOSIZE;
       if ( StrStrI(options, "HIDEWINDOW") != NULL ) opts |= SWP_HIDEWINDOW;
       if ( StrStrI(options, "SHOWWINDOW") != NULL ) opts |= SWP_SHOWWINDOW;
       if ( StrStrI(options, "NOREDRAW"  ) != NULL ) opts |= SWP_NOREDRAW;
    }
    return opts;
}


/**
 * Parses a common font style keyword argument.  The common font styles are set
 * to their defaults with the keywords over-riding the defaults, if found.
 *
 * Note that the fontStyle string can be null.
 *
 * @param fontStyle  The string to parse.
 * @param weight     [IN/OUT]  Font weight, returned.
 * @param italic     [IN/OUT]  Use italics, returned.
 * @param underline  [IN/OUT]  Use underline, returned.
 * @param strikeout  [IN/OUT]  Use strike out, returned.
 *
 * @return True if the fontStyle string contains at least one of the keywords.
 *         Return false if the string does not contain any of the keywords.
 */
bool parseFontStyleArg(CSTRING fontStyle, int *weight, BOOL *italic, BOOL *underline, BOOL *strikeout)
{
    bool found = false;

    *weight = FW_NORMAL;
    *italic = FALSE;
    *underline = FALSE;
    *strikeout = FALSE;

    if ( fontStyle != NULL )
    {
        if ( StrStrI(fontStyle, "ITALIC"   ) != NULL )
        {
            *italic = TRUE;
            found = true;
        }
        if ( StrStrI(fontStyle, "UNDERLINE") != NULL )
        {
            *underline = TRUE;
            found = true;
        }
        if ( StrStrI(fontStyle, "STRIKEOUT") != NULL )
        {
            *strikeout = TRUE;
            found = true;
        }

        if ( StrStrI(fontStyle, "THIN") != NULL )
        {
            *weight = FW_THIN;
            found = true;
        }
        else if ( StrStrI(fontStyle, "EXTRALIGHT") != NULL )
        {
            *weight = FW_EXTRALIGHT;
            found = true;
        }
        else if ( StrStrI(fontStyle, "LIGHT") != NULL )
        {
            *weight = FW_LIGHT;
            found = true;
        }
        else if ( StrStrI(fontStyle, "MEDIUM") != NULL )
        {
            *weight = FW_MEDIUM;
            found = true;
        }
        else if ( StrStrI(fontStyle, "SEMIBOLD") != NULL )
        {
            *weight = FW_SEMIBOLD;
            found = true;
        }
        else if ( StrStrI(fontStyle, "EXTRABOLD") != NULL )
        {
            *weight = FW_EXTRABOLD;
            found = true;
        }
        else if ( StrStrI(fontStyle, "BOLD") != NULL )
        {
            *weight = FW_BOLD;
            found = true;
        }
        else if ( StrStrI(fontStyle, "HEAVY") != NULL )
        {
            *weight = FW_HEAVY;
            found = true;
        }
    }
    return found;
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
    return rxNewRect(c, (PORXRECT)&r);
}

/**
 * Add, or replace, an entry in the color table for the specified control.
 *
 * @param c            Method context we are operating in.
 * @param pcpbd        Pointer to the CSelf struct for the dialog the control is
 *                     within.
 * @param id           Resource ID of the control.
 * @param bkColor      Background color for the control.
 * @param fgColor      Optional foreground color for the control.  Use -1 to
 *                     indicate the foreground color should not be changed.
 * @param useSysColor  Whether the color index for the background color is a
 *                     system color index, or not.
 *
 * @return  0 on no error, otherwise 1.
 */
logical_t oodColorTable(RexxMethodContext *c, pCPlainBaseDialog pcpbd, uint32_t id,
                        uint32_t bkColor, uint32_t fgColor, bool useSysColor)
{
    if ( pcpbd->ColorTab == NULL )
    {
        pcpbd->ColorTab = (COLORTABLEENTRY *)LocalAlloc(LPTR, sizeof(COLORTABLEENTRY) * DEF_MAX_CT_ENTRIES);
        if ( pcpbd->ColorTab == NULL )
        {
            outOfMemoryException(c->threadContext);
            return 1;
        }
        pcpbd->CT_nextIndex = 0;
        pcpbd->CT_size = DEF_MAX_CT_ENTRIES;
    }

    if ( pcpbd->CT_nextIndex >= pcpbd->CT_size )
    {
        HLOCAL temp = LocalReAlloc(pcpbd->ColorTab, sizeof(COLORTABLEENTRY) * pcpbd->CT_size * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(NULL, "Dialog control elements have exceeded the maximum\n"
                       "number of allocated color table entries, and the table\n"
                       "could not be expanded.\n\n"
                       "The color for the dialog control can not be added.",
                       "Error", MB_OK | MB_ICONHAND);
            return 1;
        }

        pcpbd->CT_size *= 2;
        pcpbd->ColorTab = (COLORTABLEENTRY *)temp;
    }

    size_t i = 0;

    // If brush is found, then an entry for this control already exists in
    // the color table.  The value of i is set to the entry index.  In this
    // case we are replacing the entry with a (presumably) new color.
    HBRUSH hbrush = searchForBrush(pcpbd, &i, id);
    if ( hbrush != NULL )
    {
        if ( ! pcpbd->ColorTab[i].isSysBrush )
        {
            DeleteObject(hbrush);
        }
    }
    else
    {
        i = pcpbd->CT_nextIndex;
        pcpbd->ColorTab[i].itemID = id;
        pcpbd->CT_nextIndex++;
    }

    pcpbd->ColorTab[i].ColorBk = bkColor;
    pcpbd->ColorTab[i].ColorFG = fgColor;

    if ( useSysColor )
    {
        pcpbd->ColorTab[i].ColorBrush   = GetSysColorBrush(bkColor);
        pcpbd->ColorTab[i].isSysBrush   = true;
        pcpbd->ColorTab[i].useSysColors = true;
    }
    else
    {
        if ( bkColor == CLR_DEFAULT )
        {
            pcpbd->ColorTab[i].ColorBrush = GetSysColorBrush(COLOR_3DFACE);
            pcpbd->ColorTab[i].isSysBrush = true;
        }
        else
        {
            pcpbd->ColorTab[i].ColorBrush = CreateSolidBrush(bkColor);
            pcpbd->ColorTab[i].isSysBrush = false;
        }
    }
    return 0;
}


HFONT oodGenericFont(const char *fontName, uint32_t fontSize, const char *opts)
{
    int weight;
    BOOL italic;
    BOOL underline;
    BOOL strikeout;
    parseFontStyleArg(opts, &weight, &italic, &underline, &strikeout);

    int height = getHeightFromFontSize(fontSize);

    return CreateFont(height, 0, 0, 0, weight, italic, underline, strikeout, DEFAULT_CHARSET,
                      OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, fontName);
}


/**
 * We set the .SystemErrorCode from here on out.
 *
 * Note that in drawFontToDC(), the SelectObject() function does not set the
 * system error code (according to the docs.)  It is unlikely that it fails, but
 * just for completedness, we set it to ERROR_SIGNAL_REFUSED if it does fail.
 *
 * @param context
 * @param hwnd
 * @param xPos
 * @param yPos
 * @param text
 * @param fontName
 * @param fontSize
 * @param fontStyle
 * @param fgColor
 * @param bkColor
 *
 * @return logical_t
 *
 * Even through we return logical_t, the ooDialog methods that use this base
 * function return 0 for success, 1 for failure.  Need to keep that in mind.
 */
logical_t oodWriteToWindow(RexxMethodContext *context, HWND hwnd, int32_t xPos, int32_t yPos, CSTRING text,
                           CSTRING fontName, uint32_t fontSize, CSTRING fontStyle, int32_t fgColor, int32_t bkColor)
{
    oodResetSysErrCode(context->threadContext);

    logical_t ret = 0;
    fontName  = (argumentOmitted(5) ? "System" : fontName);
    fontSize  = (argumentOmitted(6) ? 10       : fontSize);
    fontStyle = (argumentOmitted(7) ? ""       : fontStyle);
    fgColor   = (argumentOmitted(8) ? -1       : fgColor);
    bkColor   = (argumentOmitted(9) ? -1       : bkColor);

    HDC hDC = NULL;
    if ( StrStrI(fontStyle, "CLIENT") != NULL )
    {
        hDC = GetDC(hwnd);
    }
    else
    {
        hDC = GetWindowDC(hwnd);
    }
    if ( hDC != NULL )
    {
        ret = drawFontToDC(context->threadContext, hDC, xPos, yPos, text, fontSize, fontStyle, fontName, fgColor, bkColor);
        ReleaseDC(hwnd, hDC);
    }
    else
    {
        oodSetSysErrCode(context->threadContext);
        ret = 1;
    }
    return ret;
}

/**
 * Returns a handle to brush.  The type of brush is dependent on the arguments.
 *
 * If both args were omitted,then a stock hollow brush is returned.  When only
 * the color arg is specified, then a solid color brush of the color specified
 * is returned.
 *
 * @param context         Method context we are operating in.
 * @param color           [OPTIONAL]  The color of the brush.  If omitted, the
 *                        default is 1.
 * @param brushSpecifier  [OPTIONAL]  If specified, can be either a keyword for
 *                        the hatch pattern of a brush, or the name of a bitmap
 *                        file to use for the brush.
 *
 * @return The handle to the brush on success, or null on failure.  Sets the
 *         .SystemErrorCode on failure.
 */
HBRUSH oodCreateBrush(RexxMethodContext *context, uint32_t color, CSTRING brushSpecifier)
{
    oodResetSysErrCode(context->threadContext);

    HBRUSH hBrush = NULL;

    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        goto checkForErr_out;
    }

    color = (argumentOmitted(1) ? 1 : color);

    if ( argumentOmitted(2) )
    {
        hBrush = CreateSolidBrush(PALETTEINDEX(color));
        goto checkForErr_out;
    }

    LOGBRUSH logicalBrush;

    logicalBrush.lbStyle = BS_HATCHED;
    logicalBrush.lbColor = PALETTEINDEX(color);
    logicalBrush.lbHatch = (ULONG_PTR)-1;

    if (      stricmp(brushSpecifier, "UPDIAGONAL")   == 0 ) logicalBrush.lbHatch = HS_BDIAGONAL;
    else if ( stricmp(brushSpecifier, "DOWNDIAGONAL") == 0 ) logicalBrush.lbHatch = HS_FDIAGONAL;
    else if ( stricmp(brushSpecifier, "CROSS")        == 0 ) logicalBrush.lbHatch = HS_CROSS;
    else if ( stricmp(brushSpecifier, "DIAGCROSS")    == 0 ) logicalBrush.lbHatch = HS_DIAGCROSS;
    else if ( stricmp(brushSpecifier, "HORIZONTAL")   == 0 ) logicalBrush.lbHatch = HS_HORIZONTAL;
    else if ( stricmp(brushSpecifier, "VERTICAL")     == 0 ) logicalBrush.lbHatch = HS_VERTICAL;

    if ( logicalBrush.lbHatch == (ULONG_PTR)-1 )
    {
        // No keyword was matched, so the brushSpecifier has to be a bitmap file
        // name.

        uint32_t errCode = 0;
        HBITMAP hBmp = (HBITMAP)loadDIB(brushSpecifier, &errCode);
        if ( hBmp == NULL )
        {
            oodSetSysErrCode(context->threadContext, errCode);
            goto done_out;
        }

        logicalBrush.lbStyle = BS_DIBPATTERNPT;
        logicalBrush.lbColor = DIB_RGB_COLORS;
        logicalBrush.lbHatch = (ULONG_PTR)hBmp;
        hBrush = CreateBrushIndirect(&logicalBrush);
        if ( hBrush == NULL )
        {
            oodSetSysErrCode(context->threadContext);
        }
        LocalFree((void *)hBmp);
        goto done_out;
    }

    hBrush = CreateBrushIndirect(&logicalBrush);

checkForErr_out:
    if ( hBrush == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }

done_out:
    return hBrush;
}

/**
 * Invalidates a rectangle in a window and, usually, has the window update.
 * Update should cause the window to immediately repaint the rectangle.
 *
 * @param c                 Method context we are operating in.
 * @param hwnd              Handle to the window to be redrawn.
 * @param pr                Pointer to a rect structure specifying the area to
 *                          be redrawn.  If this arg is null, the entire client
 *                          area is redrawn.
 * @param eraseBackground   Should the background of the window be redrawn
 *                          during the repaint.
 * @param udate             Should UpdateWindow() be called.
 *
 * @return The zero object on success, the one object on failure.
 *
 * @remarks  This is common code for several API methods.  Note that in the call
 *           to InvalidateRect() if pr is null, the entire client area
 *           invalidated.  Because of this there is no need to call
 *           GetClientRect() to fill in a RECT stucture.
 */
RexxObjectPtr redrawRect(RexxMethodContext *c, HWND hwnd, PRECT pr, bool eraseBackground, bool update)
{
    oodResetSysErrCode(c->threadContext);

    if ( InvalidateRect(hwnd, pr, eraseBackground) == 0 )
    {
        oodSetSysErrCode(c->threadContext);
        return TheOneObj;
    }

    if ( update )
    {
        if ( UpdateWindow(hwnd) == 0 )
        {
            oodSetSysErrCode(c->threadContext);
            return TheOneObj;
        }
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
 *  (USEPAL) that specifies that the palette of the bitmap be used for the
 *  system color pallette.  This function implements that.
 *
 *  In general, it is safe to pass anything in, iff all the proper conditions
 *  are met, then the system color palette is set.  Otherwise nothing is done.
 *
 *  @param c       Method context we are operating in.
 *  @param hBmp    Handle to the bitmap.  Can be null, then nothing is done.
 *  @param opts    An option string, can be null.  If the string is null, or the
 *                 string does not contain the keyword "USEPAL" then nothing is
 *                 done.
 *  @param         Pointer to a dialog CSelf struct.  The caller should not pass
 *                 in null, but a check is done that it is not null.
 */
void maybeSetColorPalette(RexxMethodContext *c, HBITMAP hBmp, CSTRING opts, pCPlainBaseDialog pcpbd)
{
    if ( hBmp != NULL && opts != NULL && StrStrI(opts, "USEPAL") != NULL && pcpbd != NULL)
    {
        if ( pcpbd->colorPalette )
        {
            DeleteObject(pcpbd->colorPalette);
        }
        pcpbd->colorPalette = createDIBPalette((LPBITMAPINFO)hBmp);
        setSysPalColors(pcpbd->colorPalette);
    }
}

// TODO this function needs to be rewritten, it is based on Win16 obsolete functions
LPBITMAPINFO loadDIB(const char *szFile, uint32_t *lastError)
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
        uint32_t err = GetLastError();

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

        if ( lastError != NULL )
        {
            *lastError = err;
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

    if ( lastError != NULL )
    {
        *lastError = GetLastError();
        if ( *lastError == 0 )
        {
            *lastError = ERROR_INVALID_FUNCTION;  // Temp catch-all error code.
        }
    }
    return NULL;
}


/**
 * Used to draw the part of a bitmap button that will not be covered by the
 * bitmap.
 */
static void drawBmpBackground(pCPlainBaseDialog pcpbd, INT id, HDC hDC, RECT * itRect, RECT * bmpRect,
                              LPARAM lParam, LONG left, LONG top)
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
    if ( pcpbd->CT_nextIndex != 0 )
    {
        size_t i;
        hbr = searchForBrush(pcpbd, &i, id);
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


inline bool forSureHaveBitmap(pCPlainBaseDialog pcpbd, size_t i)
{
    return ( (ULONG_PTR)pcpbd->BmpTab[i].bmpFocusID  +
             (ULONG_PTR)pcpbd->BmpTab[i].bmpSelectID +
             (ULONG_PTR)pcpbd->BmpTab[i].bitmapID    +
             (ULONG_PTR)pcpbd->BmpTab[i].bmpDisableID
             > 0 );
}
inline bool focusedNotDisabled(uint32_t itemState)
{
    return ( (itemState & ODS_FOCUS)    == ODS_FOCUS &&
             (itemState & ODS_DISABLED) == 0         &&
             (itemState & ODS_SELECTED) == 0 );
}

/**
 * Handles the WM_DRAWITEM message in the dialog window procedure.  Called from
 * any of the Rexx dialog procedures in ooDialog.
 *
 *
 * @param pcpbd
 * @param lParam
 * @param msgEnabled
 *
 * @return TRUE of FALSE.  The return value has been preserved so that it
 *         matches what the original ooDialog code would have returned.
 */
LRESULT drawBitmapButton(pCPlainBaseDialog pcpbd, LPARAM lParam, bool msgEnabled)
{
    if ( lParam == 0 )
    {
        return FALSE;
    }

    DRAWITEMSTRUCT * dis;
    HDC hDC;
    HBITMAP hBmp = NULL;
    RECT r, ri;
    LONG rc;
    HPEN oP, nP;
    BITMAP bmpInfo;
    LONG left = 0, top = 0;
    POINT lp;
    HPALETTE hP;
    ULONG desth, destw;

    dis = (DRAWITEMSTRUCT *)lParam;

    RedrawScrollingButton = dis->hwndItem;
    if ( ScrollingButton == dis->hwndItem )
    {
        return FALSE;
    }

    if ( pcpbd->colorPalette != NULL )
    {
        hP = SelectPalette(dis->hDC, pcpbd->colorPalette, 0);
        RealizePalette(dis->hDC);
    }

    ri = dis->rcItem;
    r = ri;

    // Don't enable focus or selected for inactive dialog.
    if ( ! msgEnabled )
    {
        dis->itemState &= ~(ODS_SELECTED | ODS_FOCUS);
    }

    size_t i;

    if ( findBmpForID(pcpbd, dis->CtlID, &i) )
    {
        if ( (dis->itemState & ODS_DISABLED) == ODS_DISABLED )
        {
            hBmp = (HBITMAP)(pcpbd->BmpTab[i].bmpDisableID == NULL ? pcpbd->BmpTab[i].bitmapID : pcpbd->BmpTab[i].bmpDisableID);
        }
        else if ( (dis->itemState & ODS_SELECTED) == ODS_SELECTED )
        {
            hBmp = (HBITMAP)(pcpbd->BmpTab[i].bmpSelectID == NULL ? pcpbd->BmpTab[i].bitmapID : pcpbd->BmpTab[i].bmpSelectID);

            // For a 3D effect.
            if ( pcpbd->BmpTab[i].frame )
            {
                r.top    += 3;
                r.left   += 3;
                r.right  += 3;
                r.bottom += 3;
            }
        }
        else if ( (dis->itemState & ODS_FOCUS) == ODS_FOCUS )
        {
            hBmp = (HBITMAP)(pcpbd->BmpTab[i].bmpFocusID == NULL ? pcpbd->BmpTab[i].bitmapID : pcpbd->BmpTab[i].bmpFocusID);
        }
        else
        {
            hBmp = (HBITMAP)pcpbd->BmpTab[i].bitmapID;
        }

        if ( pcpbd->BmpTab[i].displaceX != 0 || pcpbd->BmpTab[i].displaceY != 0 )
        {
            r.top    = r.top    + pcpbd->BmpTab[i].displaceY;
            r.bottom = r.bottom + pcpbd->BmpTab[i].displaceY;
            r.left   = r.left   + pcpbd->BmpTab[i].displaceX;
            r.right  = r.right  + pcpbd->BmpTab[i].displaceX;
            if ( r.left < 0 )
            {
                left = abs(r.left);
                r.left = 0;
            }
            if ( r.top < 0 )
            {
                top = abs(r.top);
                r.top = 0;
            }
        }

        if ( hBmp != NULL )
        {
            if ( pcpbd->BmpTab[i].loaded == 0 )
            {
                hDC = CreateCompatibleDC(dis->hDC);
                SelectObject(hDC, hBmp);

                GetObject(hBmp, sizeof(BITMAP), &bmpInfo);
                r.right  = r.left + bmpInfo.bmWidth;
                r.bottom = r.top  + bmpInfo.bmHeight;

                BitBlt(dis->hDC, r.left, r.top, r.right, r.bottom, hDC, left, top, SRCCOPY);
            }
            else if ( forSureHaveBitmap(pcpbd, i) )
            {
                // Original comment: This if has been added because of a
                // violation error moving animated button dialogs a lot
                //
                // I think what was meant was the 'if' part of the else.

                // Is stretching activated?
                if ( (pcpbd->BmpTab[i].loaded & 0x0100) == 0x0100 )
                {
                    destw = r.right  - r.left;
                    desth = r.bottom - r.top;
                }
                else
                {
                    destw = dibWidth(hBmp);
                    desth = dibHeight(hBmp);
                }

                StretchDIBits(dis->hDC, r.left, r.top, destw, desth, left, top, dibWidth(hBmp), dibHeight(hBmp),                      // src height
                              dibPBits(hBmp), dibPBI(hBmp), DIB_RGB_COLORS, SRCCOPY);

                r.right  = r.left + destw;
                r.bottom = r.top  + desth;
            }

            drawBmpBackground(pcpbd, dis->CtlID, dis->hDC, &ri, &r, lParam, left, top);

            if ( pcpbd->BmpTab[i].frame )
            {
                rc = FrameRect(dis->hDC, (LPRECT)&dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));

                if ( (dis->itemState & ODS_SELECTED) == ODS_SELECTED && ! (dis->itemState & ODS_DISABLED) )
                {
                    // Draw a 3D effect.
                    nP = CreatePen(PS_SOLID, 2, RGB(120,120,120));
                    oP = (HPEN)SelectObject(dis->hDC, nP);

                    MoveToEx(dis->hDC, dis->rcItem.left  + 2, dis->rcItem.top    + 2, &lp);
                    LineTo(dis->hDC,   dis->rcItem.right - 2, dis->rcItem.top    + 2);
                    MoveToEx(dis->hDC, dis->rcItem.left  + 2, dis->rcItem.bottom - 2, &lp);
                    LineTo(dis->hDC,   dis->rcItem.left  + 2, dis->rcItem.top    + 1);

                    SelectObject(dis->hDC, oP);
                    DeleteObject(nP);
                }
                else
                {
                    // Draw a white line.
                    nP = CreatePen(PS_SOLID, 2, RGB(240,240,240));
                    oP = (HPEN)SelectObject(dis->hDC, nP);

                    MoveToEx(dis->hDC, dis->rcItem.left  + 2, dis->rcItem.top    + 2, &lp);
                    LineTo(dis->hDC,   dis->rcItem.right - 2, dis->rcItem.top    + 2);
                    MoveToEx(dis->hDC, dis->rcItem.left  + 2, dis->rcItem.bottom - 2, &lp);
                    LineTo(dis->hDC,   dis->rcItem.left  + 2, dis->rcItem.top    + 1);

                    SelectObject(dis->hDC, oP);
                    DeleteObject(nP);

                    // Draw a grey line.
                    nP = CreatePen(PS_SOLID, 2, RGB(120,120,120));
                    oP = (HPEN)SelectObject(dis->hDC, nP);

                    MoveToEx(dis->hDC, dis->rcItem.right - 2, dis->rcItem.top    + 2, &lp);
                    LineTo(dis->hDC,   dis->rcItem.right - 2, dis->rcItem.bottom - 2);
                    MoveToEx(dis->hDC, dis->rcItem.left  + 2, dis->rcItem.bottom - 2, &lp);
                    LineTo(dis->hDC,   dis->rcItem.right - 2, dis->rcItem.bottom - 2);

                    SelectObject(dis->hDC, oP);
                    DeleteObject(nP);
                }

                if ( focusedNotDisabled(dis->itemState) )
                {
                    nP = CreatePen(PS_DOT, 1, RGB(0,0,0));
                    oP = (HPEN)SelectObject(dis->hDC, nP);

                    MoveToEx(dis->hDC, dis->rcItem.left  + 4, dis->rcItem.top    + 4, &lp);
                    LineTo(dis->hDC,   dis->rcItem.right - 4, dis->rcItem.top    + 4);
                    LineTo(dis->hDC,   dis->rcItem.right - 4, dis->rcItem.bottom - 4);
                    LineTo(dis->hDC,   dis->rcItem.left  + 4, dis->rcItem.bottom - 4);
                    LineTo(dis->hDC,   dis->rcItem.left  + 4, dis->rcItem.top    + 4);

                    SelectObject(dis->hDC, oP);
                    DeleteObject(nP);
                }
            }

            if ( pcpbd->BmpTab[i].loaded == 0 )
            {
                DeleteDC(hDC);
            }
            if ( pcpbd->colorPalette != NULL )
            {
                SelectPalette(dis->hDC, hP, 0);
            }

            return TRUE;
        }
    }

    r.left   = 0;
    r.top    = 0;
    r.right  = ri.left;
    r.bottom = ri.top;
    drawBmpBackground(pcpbd, dis->CtlID, dis->hDC, &ri, &r, lParam, left, top);

    if ( pcpbd->colorPalette != NULL )
    {
        SelectPalette(dis->hDC, hP, 0);
    }
    return FALSE;
}


/**
 * Handles the WM_CTLCOLORDLG message in the dialog window procedure.  Called
 * from any of the various Rexx dialog procedures of ooDialog.
 *
 * @param pcpbd
 *
 * @return The handle of the background brush if one has been set, otherwise
 *         FALSE;
 */
LRESULT handleDlgColor(pCPlainBaseDialog pcpbd)
{
    if ( pcpbd->bkgBrush )
    {
        return(LRESULT)pcpbd->bkgBrush;
    }
    return FALSE;
}

/**
 * Handles the
 *
 *   WM_CTLCOLORSTATIC:
 *   WM_CTLCOLORBTN:
 *   WM_CTLCOLOREDIT:
 *   WM_CTLCOLORLISTBOX:
 *   WM_CTLCOLORMSGBOX:
 *   WM_CTLCOLORSCROLLBAR:
 *
 * messages in the dialog window procedure.  Called from any of the various Rexx
 * dialog procedures of ooDialog.  Handles the case where the user has set a
 * custom color for a dialog control.
 *
 * @param pcpbd
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT
 */
LRESULT handleCtlColor(pCPlainBaseDialog pcpbd, HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hbrush = NULL;

    // If we have entries in the color table, see if the user has set a color
    // for this control.
    if ( pcpbd->CT_nextIndex > 0 )
    {
        long id = GetWindowLong((HWND)lParam, GWL_ID);
        if ( id > 0 )
        {
            register size_t i = 0;
            while ( i < pcpbd->CT_nextIndex && pcpbd->ColorTab[i].itemID != id )
            {
                i++;
            }
            if ( i < pcpbd->CT_nextIndex )
            {
                hbrush = pcpbd->ColorTab[i].ColorBrush;
            }

            if ( hbrush )
            {
                if ( pcpbd->ColorTab[i].useSysColors )
                {
                    if ( pcpbd->ColorTab[i].ColorBk != CLR_DEFAULT )
                    {
                        SetBkColor((HDC)wParam, GetSysColor(pcpbd->ColorTab[i].ColorBk));
                        if ( pcpbd->ColorTab[i].ColorFG != CLR_DEFAULT )
                        {
                            SetTextColor((HDC)wParam, GetSysColor(pcpbd->ColorTab[i].ColorFG));
                        }
                    }
                    else if ( pcpbd->ColorTab[i].ColorFG != CLR_DEFAULT )
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        SetTextColor((HDC)wParam, GetSysColor(pcpbd->ColorTab[i].ColorFG));

                        if ( pcpbd->bkgBrush )
                        {
                            hbrush = pcpbd->bkgBrush;
                        }
                    }
                }
                else
                {
                    if ( pcpbd->ColorTab[i].ColorBk != CLR_DEFAULT )
                    {
                        SetBkColor((HDC)wParam, pcpbd->ColorTab[i].ColorBk);
                        if ( pcpbd->ColorTab[i].ColorFG != CLR_DEFAULT )
                        {
                            SetTextColor((HDC)wParam, pcpbd->ColorTab[i].ColorFG);
                        }
                    }
                    else if ( pcpbd->ColorTab[i].ColorFG != CLR_DEFAULT )
                    {
                        SetBkMode((HDC)wParam, TRANSPARENT);
                        SetTextColor((HDC)wParam, pcpbd->ColorTab[i].ColorFG);

                        if ( pcpbd->bkgBrush )
                        {
                            hbrush = pcpbd->bkgBrush;
                        }
                    }
                }
            }
        }
    }

    if ( hbrush )
    {
        return(LRESULT)hbrush;
    }

    return DefWindowProc(hDlg, uMsg, wParam, lParam);
}


/**
 * Handles the WM_PAINT message in the dialog window procedure.  Called from any
 * of the Rexx dialog procedures in ooDialog.
 *
 * @param pcpbd
 * @param hDlg
 *
 * @return FALSE always.  Note that this is what the original ooDialog code did
 *         and has not been changed through all the revisions of the code.
 */
LRESULT drawBackgroundBmp(pCPlainBaseDialog pcpbd, HWND hDlg)
{
    if ( pcpbd->bkgBitmap == NULL )
    {
        return FALSE;
    }

    HDC hDC;
    PAINTSTRUCT ps;
    HPALETTE hP;
    RECT r;
    LONG desth, destw;

    hDC = BeginPaint(hDlg, &ps);

    if (pcpbd->colorPalette)
    {
       hP = SelectPalette(hDC, pcpbd->colorPalette, 0);
       RealizePalette(hDC);
    }

    GetClientRect(hDlg, &r);

    destw = dibWidth(pcpbd->bkgBitmap);         // dest width
    desth = dibHeight(pcpbd->bkgBitmap);        // dest height

    if ( r.right - r.left > destw )
    {
       destw = r.right - r.left;
    }
    if ( r.bottom - r.top > desth )
    {
       desth = r.bottom - r.top;
    }

    StretchDIBits(hDC,
                  0,                             // dest x
                  0,                             // dest y
                  destw,
                  desth,
                  0,                             // src x
                  0,                             // src y
                  dibWidth(pcpbd->bkgBitmap),   // src width
                  dibHeight(pcpbd->bkgBitmap),  // src height
                  dibPBits(pcpbd->bkgBitmap),   // bits
                  dibPBI(pcpbd->bkgBitmap),     // BITMAPINFO
                  DIB_RGB_COLORS,
                  SRCCOPY);                      // rop

    EndPaint(hDlg, &ps);

    return FALSE;
}

/**
 * Retrieves the system color index number from a Rexx object that may be the
 * actual number or the color keyword.
 *
 * @param c       Method context we are operating in.
 * @param clr     Rexx object, presumably the system color number or keyword.
 * @param color   The color index is returned here on success
 * @param argPos  The argument position of the Rexx object.
 *
 * @return True on success, othewise false.
 *
 * @remarks  Currently this is only called when clr is an argument to a method.
 *           If called under other circumstances, the logic may need to be
 *           adjusted.
 *
 *           Raises a syntax condition on failure to convert clr.
 */
bool getSystemColor(RexxMethodContext *c, RexxObjectPtr clr, uint32_t *color, size_t argPos)
{
    if ( c->UnsignedInt32(clr, color) )
    {
        return true;
    }

    CSTRING keyword = c->ObjectToStringValue(clr);

    if (      StrCmpI(keyword, "3DDKSHADOW")              == 0) *color = 21;
    else if ( StrCmpI(keyword, "3DFACE")                  == 0) *color = 15;
    else if ( StrCmpI(keyword, "3DHIGHLIGHT")             == 0) *color = 20;
    else if ( StrCmpI(keyword, "3DHILIGHT")               == 0) *color = 20;
    else if ( StrCmpI(keyword, "3DLIGHT")                 == 0) *color = 22;
    else if ( StrCmpI(keyword, "3DSHADOW")                == 0) *color = 16;
    else if ( StrCmpI(keyword, "ACTIVEBORDER")            == 0) *color = 10;
    else if ( StrCmpI(keyword, "ACTIVECAPTION")           == 0) *color =  2;
    else if ( StrCmpI(keyword, "APPWORKSPACE")            == 0) *color = 12;
    else if ( StrCmpI(keyword, "BACKGROUND")              == 0) *color =  1;
    else if ( StrCmpI(keyword, "BTNFACE")                 == 0) *color = 15;
    else if ( StrCmpI(keyword, "BTNHIGHLIGHT")            == 0) *color = 20;
    else if ( StrCmpI(keyword, "BTNHILIGHT")              == 0) *color = 20;
    else if ( StrCmpI(keyword, "BTNSHADOW")               == 0) *color = 16;
    else if ( StrCmpI(keyword, "BTNTEXT")                 == 0) *color = 18;
    else if ( StrCmpI(keyword, "CAPTIONTEXT")             == 0) *color =  9;
    else if ( StrCmpI(keyword, "DESKTOP")                 == 0) *color =  1;
    else if ( StrCmpI(keyword, "GRADIENTACTIVECAPTION")   == 0) *color = 27;
    else if ( StrCmpI(keyword, "GRADIENTINACTIVECAPTION") == 0) *color = 28;
    else if ( StrCmpI(keyword, "GRAYTEXT")                == 0) *color = 17;
    else if ( StrCmpI(keyword, "HIGHLIGHT")               == 0) *color = 13;
    else if ( StrCmpI(keyword, "HIGHLIGHTTEXT")           == 0) *color = 14;
    else if ( StrCmpI(keyword, "HOTLIGHT")                == 0) *color = 26;
    else if ( StrCmpI(keyword, "INACTIVEBORDER")          == 0) *color = 11;
    else if ( StrCmpI(keyword, "INACTIVECAPTION")         == 0) *color =  3;
    else if ( StrCmpI(keyword, "INACTIVECAPTIONTEXT")     == 0) *color = 19;
    else if ( StrCmpI(keyword, "INFOBK")                  == 0) *color = 24;
    else if ( StrCmpI(keyword, "INFOTEXT")                == 0) *color = 23;
    else if ( StrCmpI(keyword, "MENU")                    == 0) *color =  4;
    else if ( StrCmpI(keyword, "MENUHILIGHT")             == 0) *color = 29;
    else if ( StrCmpI(keyword, "MENUBAR")                 == 0) *color = 30;
    else if ( StrCmpI(keyword, "MENUTEXT")                == 0) *color =  7;
    else if ( StrCmpI(keyword, "SCROLLBAR")               == 0) *color =  0;
    else if ( StrCmpI(keyword, "WINDOW")                  == 0) *color =  5;
    else if ( StrCmpI(keyword, "WINDOWFRAME")             == 0) *color =  6;
    else if ( StrCmpI(keyword, "WINDOWTEXT")              == 0) *color =  8;
    else
    {
        TCHAR buffer[512];
        _snprintf(buffer, sizeof(buffer), "Argument %zd is not a valid system color keyword; found %s", argPos, keyword);
        userDefinedMsgException(c, buffer);

        return false;
    }
    return true;
}

static bool getIdOrName(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr rxBmp, int32_t *id, CSTRING *bmp)
{
    if ( oodSafeResolveID(id, c, pcpbd->rexxSelf, rxBmp, -1, 1, true) )
    {
        return true;
    }
    else
    {
        // If the global .constDir is *only* being used, there could be a
        // condition raised even when oodSaveResolveID() is used.
        if ( c->CheckCondition() )
        {
            c->ClearCondition();
        }

        *bmp = c->ObjectToStringValue(rxBmp);
    }

    return false;
}

void assignBitmap(pCPlainBaseDialog pcpbd, size_t index, CSTRING bmp, int32_t bmpID, PUSHBUTTON_STATES type,
                  bool isInMemory, bool isIntResource)
{
    HBITMAP hBmp = NULL;
    BITMAPTABLEENTRY *bitmapEntry = pcpbd->BmpTab + index;

    if ( isInMemory )
    {
        hBmp = (HBITMAP)string2pointer(bmp);
        if ( hBmp != NULL )
        {
            bitmapEntry->loaded = 2;
        }
    }
    else if ( isIntResource )
    {
        hBmp = LoadBitmap(pcpbd->hInstance, MAKEINTRESOURCE(bmpID));
    }
    else
    {
        if ( strlen(bmp) < 2 && (bmp[0] == '0' || bmp[0] == '\0') )
        {
            // Purposefully do nothing.  IBM Object Rexx whould check for a
            // bitmap name of 0 (zero) or the empty string and then use loadDIB
            // on it. Which in turn set the bitmapID field to NULL.  This
            // ultimately had the effect of no bitmap being drawn for that
            // button state. At least one of the example programs depended on
            // that.
            //
            // Since hBmp is already NULL, we don't do anything.
            ;
        }
        else
        {
            hBmp = (HBITMAP)loadDIB(bmp, NULL);
        }

        if ( hBmp != NULL )
        {
            bitmapEntry->loaded = 1;
        }
    }

    switch ( type )
    {
        case PBSS_NORMAL :
            bitmapEntry->bitmapID = hBmp;
            break;
        case PBSS_PRESSED :
            bitmapEntry->bmpSelectID = hBmp;
            break;
        case PBSS_DISABLED :
            bitmapEntry->bmpDisableID = hBmp;
            break;
        case PBSS_DEFAULTED :
            bitmapEntry->bmpFocusID = hBmp;
            break;
    }
}


/**
 *  Methods for the .DialogExtensions mixin class.
 */
#define DIALOGEXTENSIONS_CLASS        "DialogExtensions"

pCPlainBaseDialog dlgExtSetup(RexxMethodContext *c, RexxObjectPtr dlg)
{
    oodResetSysErrCode(c->threadContext);

    pCPlainBaseDialog pcpbd = requiredDlgCSelf(c, dlg, oodPlainBaseDialog, 0, NULL);
    if ( pcpbd == NULL )
    {
        return (pCPlainBaseDialog)baseClassInitializationException(c);
    }

    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(c, dlg);
        return NULL;
    }
    return pcpbd;
}

/**
 * Do common set up for a DialogExtensions method involving a resource ID for a
 * control.
 *
 * Note that we want to be able to call this, some times, before the underlying
 * dialog has been created, so we bypass dlgExtSetup().
 *
 * Passing in null for phCtrl, signals that the underlying dialog does not need
 * to be created yet.  If phCtrl is not null, then that signals that the method
 * requires the underlying dialog to exist.
 *
 * @param c
 * @param self
 * @param rxID
 * @param ppcpbd
 * @param pID
 * @param phCtrl
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr dlgExtControlSetup(RexxMethodContext *c, RexxObjectPtr self, RexxObjectPtr rxID,
                                 pCPlainBaseDialog *ppcpbd, uint32_t *pID, HWND *phCtrl)
{
    oodResetSysErrCode(c->threadContext);

    pCPlainBaseDialog pcpbd = dlgToCSelf(c, self);
    if ( pcpbd == NULL )
    {
        baseClassInitializationException(c);
        return TheOneObj;
    }

    if ( pcpbd->hDlg == NULL && phCtrl != NULL )
    {
        noWindowsDialogException(c, self);
        return TheOneObj;
    }

    int32_t id;
    if ( ! oodSafeResolveID(&id, c, pcpbd->rexxSelf, rxID, -1, 1, true) )
    {
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheNegativeOneObj;
    }

    if ( phCtrl != NULL )
    {
        HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
        if ( hCtrl == NULL )
        {
            oodSetSysErrCode(c->threadContext);
            return TheOneObj;
        }
        *phCtrl = hCtrl;
    }
    if ( ppcpbd != NULL )
    {
        *ppcpbd = pcpbd;
    }
    if ( pID != NULL )
    {
        *pID = id;
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


/** DialogExtensions::clearRect()
 *
 *  'Clears' a bounding rectangle in the specified Window.
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
 *            have been created, so that check is skipped.  But, in reality I
 *            suspect this method should only be used with the dialog and its
 *            control windows.
 */
RexxMethod3(RexxObjectPtr, dlgext_clearRect, POINTERSTRING, hwnd, ARGLIST, args, OSELF, self)
{
    oodResetSysErrCode(context->threadContext);

    pCPlainBaseDialog pcpbd = dlgToCSelf(context, self);
    if ( pcpbd == NULL )
    {
        baseClassInitializationException(context);
        return TheOneObj;
    }

    RECT r = {0};
    size_t arraySize;
    size_t argsUsed;

    if ( ! getRectFromArglist(context, args, (PORXRECT)&r, true, 2, 5, &arraySize, &argsUsed) )
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
 *
 *            There is no syntax exception if the underlying Windows dialog does
 *            not exist.  There is no reason why the dialog would need to exist,
 *            since we are using the window handle to position a window.  If the
 *            hwnd arg is not a window, the system error code will get set.
 */
RexxMethod3(RexxObjectPtr, dlgext_setWindowRect, POINTERSTRING, hwnd, ARGLIST, args, OSELF, self)
{
    oodResetSysErrCode(context->threadContext);

    size_t countArgs;
    size_t    argsUsed;
    RECT   rect;
    if ( ! getRectFromArglist(context, args, (PORXRECT)&rect, false, 2, 6, &countArgs, &argsUsed) )
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
    return redrawRect(context, hwnd, NULL, doErase, true);
}


/** DialogExtensions::redrawRect()
 *
 *  Immediately redraws the specified rectangle in the client area of the
 *  specified window.
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
    size_t argsUsed;

    if ( ! getRectFromArglist(context, args, (PORXRECT)&r, true, 2, 6, &arraySize, &argsUsed) )
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

    return redrawRect(context, hwnd, &r, doErase, true);
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
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 2, 4, &arraySize, &argsUsed) )
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
    return redrawRect(context, hCtrl, NULL, doErase, true);
}


/** DialogExtensions::installBitmapButton()
 *
 * Installs (sets up) a bitmap button.  In ooDialog a bitmap button is an
 * owner-drawn button that uses the supplied bitmap(s) for the button.  ooDialog
 * internally handles the drawing of the button.
 *
 * Up to 4 bitmaps can be assigned to the button.  These bitmaps are drawn for
 * the different button states: normal, focused, selected and disabled.  The
 * normal bitmap must be supllied and that bitmap is used for any of the other
 * states if no bitmap for the state is supplied.
 *
 * The bitmap arg(s) must all be the same type.  They can be a bitmap file name,
 * a bitmap handle (the style argument *must* contain the INMEMORY keyword in
 * this case,) or the resource number of a bitmap compiled into the resource DLL
 * of a ResDialog.
 *
 * @param  id          The resource ID of the button.  Can be numeric or
 *                     symbolic.
 * @param  msgToRaise  [OPTIONAL]  A method name to connect the button click
 *                     event to.
 * @param  bmpNormal   Bitmap to draw for the normal button state.
 * @param  bmpFocused  [OPTIONAL]  Bitmap to draw when the button has the focus.
 * @param  bmpSelected [OPTIONAL]  Bitmap to draw when the button is selected
 *                     (pushed.)
 * @param  bmpDisabled [OPTIONAL]  Bitmap to draw when the button is disabled.
 * @param  style       [OPTIONAL]  Keyword signalling several options.
 *
 * @return  0 on success, -1 if the resource ID could not be resolved, and 1 for
 *          other errors.
 *
 * @remarks  The only way to get the bitmaps into the bitmap table is through
 *           this method. Therefore, we have to allow an omitted msgToRise.  If
 *           msgToRise == "", or is ommitted, then  adding a method to the
 *           message table is skipped.
 *
 * @remarks  Note that the dialog does not need to yet be created for some
 *           variations of this method.  But if any of the bitmap source
 *           arguments are a number, so that the bitmap is to be loaded from a
 *           resource DLL, then the dialog does need to be created.  We want to
 *           raise the no windows dialog exception, *only* for that one
 *           condition.
 *
 *           The bitmap sources must all be from the same sources, all from a
 *           file name, or all from from a bitmap handle already loaded in
 *           memory, or all loaded from the resource DLL of a ResDialog. In
 *           particular, if one is loaded from a resource DLL and others loaded
 *           from a file, delDialog() will crash trying to free the bitmaps.
 *
 *           These old bitmap methods are too error-prone.
 */
RexxMethod8(RexxObjectPtr, dlgext_installBitmapButton, RexxObjectPtr, rxID, OPTIONAL_CSTRING, msgToRaise,
            RexxObjectPtr, bmpNormal, OPTIONAL_RexxObjectPtr, bmpFocused, OPTIONAL_RexxObjectPtr, bmpSelected,
            OPTIONAL_RexxObjectPtr, bmpDisabled, OPTIONAL_CSTRING, style, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    uint32_t id;

    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, &pcpbd, &id, NULL);
    if ( result != TheZeroObj )
    {
        return result;
    }

    bool noUnderlyingDlg = pcpbd->hDlg == NULL ? true : false;

    int32_t bmpID = -1;
    CSTRING bmp   = "";

    bool isIntResource = getIdOrName(context, pcpbd, bmpNormal, &bmpID, &bmp);
    if ( isIntResource && noUnderlyingDlg )
    {
        noWindowsDialogException(context, self);
        return TheOneObj;
    }

    if ( pcpbd->BmpTab == NULL )
    {
       pcpbd->BmpTab = (BITMAPTABLEENTRY *)LocalAlloc(LPTR, sizeof(BITMAPTABLEENTRY) * DEF_MAX_BT_ENTRIES);
       if ( pcpbd->BmpTab == NULL )
       {
          MessageBox(0, "No memory available","Error", MB_OK | MB_ICONHAND);
          return TheOneObj;
       }
       pcpbd->BT_nextIndex = 0;
       pcpbd->BT_size = DEF_MAX_BT_ENTRIES;
    }

    bool frame      = false;
    bool inMemory   = false;
    bool stretch    = false;
    bool usePalette = false;
    if ( argumentExists(7) )
    {
        if ( StrStrI(style, "FRAME")    != NULL ) frame = true;
        if ( StrStrI(style, "INMEMORY") != NULL ) inMemory = true;
        if ( StrStrI(style, "STRETCH")  != NULL ) stretch = true;
        if ( StrStrI(style, "USEPAL")   != NULL ) usePalette = true;
    }

    if ( pcpbd->BT_nextIndex >= pcpbd->BT_size )
    {
        HLOCAL temp = LocalReAlloc(pcpbd->BmpTab, sizeof(BITMAPTABLEENTRY) * pcpbd->BT_size * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(NULL, "Bitmap buttons have exceeded the maximum\n"
                       "number of allocated table entries, and the table\n"
                       "could not be expanded.\n\n"
                       "No bitmap button can be added.",
                       "Error", MB_OK | MB_ICONHAND);
            return TheOneObj;
        }

        pcpbd->BT_size *= 2;
        pcpbd->BmpTab = (BITMAPTABLEENTRY *)temp;
    }

    size_t index = pcpbd->BT_nextIndex;

    pcpbd->BmpTab[index].buttonID = id;
    pcpbd->BmpTab[index].frame  = frame;

    BitmapButtonBMPType normal = getBMPType(inMemory, isIntResource);

    assignBitmap(pcpbd, index, bmp, bmpID, PBSS_NORMAL, inMemory, isIntResource);

    if ( argumentExists(4) )
    {
        isIntResource = getIdOrName(context, pcpbd, bmpFocused, &bmpID, &bmp);
        if ( isIntResource && noUnderlyingDlg )
        {
            noWindowsDialogException(context, self);
            return TheOneObj;
        }

        BitmapButtonBMPType focused = getBMPType(inMemory, isIntResource);
        if ( focused != normal )
        {
            bitmapTypeMismatchException(context, bmpType2String(normal), bmpType2String(focused), 4);
            return TheOneObj;
        }

        assignBitmap(pcpbd, index, bmp, bmpID, PBSS_DEFAULTED, inMemory, isIntResource);
    }
    if ( argumentExists(5) )
    {
        isIntResource = getIdOrName(context, pcpbd, bmpSelected, &bmpID, &bmp);
        if ( isIntResource && noUnderlyingDlg )
        {
            noWindowsDialogException(context, self);
            return TheOneObj;
        }

        BitmapButtonBMPType selected = getBMPType(inMemory, isIntResource);
        if ( selected != normal )
        {
            bitmapTypeMismatchException(context, bmpType2String(normal), bmpType2String(selected), 5);
            return TheOneObj;
        }

        assignBitmap(pcpbd, index, bmp, bmpID, PBSS_PRESSED, inMemory, isIntResource);
    }
    if ( argumentExists(6) )
    {
        isIntResource = getIdOrName(context, pcpbd, bmpDisabled, &bmpID, &bmp);
        if ( isIntResource && noUnderlyingDlg )
        {
            noWindowsDialogException(context, self);
            return TheOneObj;
        }

        BitmapButtonBMPType disabled = getBMPType(inMemory, isIntResource);
        if ( disabled != normal )
        {
            bitmapTypeMismatchException(context, bmpType2String(normal), bmpType2String(disabled), 6);
            return TheOneObj;
        }

        assignBitmap(pcpbd, index, bmp, bmpID, PBSS_DISABLED, inMemory, isIntResource);
    }

    if ( stretch && pcpbd->BmpTab[index].loaded )
    {
        pcpbd->BmpTab[index].loaded |= 0x0100;
    }

    // Maybe create a palette that conforms to the bitmap colors.
    if ( usePalette )
    {
       if ( pcpbd->colorPalette != NULL )
       {
           DeleteObject(pcpbd->colorPalette);
       }

       pcpbd->colorPalette = createDIBPalette((LPBITMAPINFO)pcpbd->BmpTab[index].bitmapID);
       setSysPalColors(pcpbd->colorPalette);
    }

    if ( argumentOmitted(2) || msgToRaise[0] == '\0' )
    {
       pcpbd->BT_nextIndex++;
       return TheZeroObj;
    }
    else if ( addCommandMessage(pcpbd->enCSelf, context, id, 0x0000FFFF, 0, 0, msgToRaise, TAG_NOTHING) )
    {
       pcpbd->BT_nextIndex++;
       return TheZeroObj;
    }

    return TheOneObj;
}


/** DialogExtensions::changeBitmapButton()
 *
 * Changes the bitmap(s) for an already installed bitmap button and immediately
 * redraw the button.
 *
 * In ooDialog a bitmap button is an owner-drawn button that uses the supplied
 * bitmap(s) for the button. ooDialog internally handles the drawing of the
 * button.
 *
 * Up to 4 bitmaps can be assigned to the button.  These bitmaps are drawn for
 * the different button states: normal, focused, selected and disabled.  The
 * normal bitmap must be supllied and that bitmap is used for any of the other
 * states if no bitmap for the state is supplied.
 *
 * The bitmap arg(s) must all be the same type.  They can be a bitmap file name,
 * a bitmap handle (the style argument *must* contain the INMEMORY keyword in
 * this case,) or the resource number of a bitmap compiled into the resource DLL
 * of a ResDialog.
 *
 * @param  id          The resource ID of the button.  Can be numeric or
 *                     symbolic.
 * @param  bmpNormal   Bitmap to draw for the normal button state.
 * @param  bmpFocused  [OPTIONAL]  Bitmap to draw when the button has the focus.
 * @param  bmpSelected [OPTIONAL]  Bitmap to draw when the button is selected
 *                     (pushed.)
 * @param  bmpDisabled [OPTIONAL]  Bitmap to draw when the button is disabled.
 * @param  style       [OPTIONAL]  Keyword signalling several options.
 *
 * @return  0 on success, -1 if the resource ID could not be resolved, and 1 for
 *          other errors.
 */
RexxMethod7(RexxObjectPtr, dlgext_changeBitmapButton, RexxObjectPtr, rxID, RexxObjectPtr, bmpNormal,
            OPTIONAL_RexxObjectPtr, bmpFocused, OPTIONAL_RexxObjectPtr, bmpSelected,
            OPTIONAL_RexxObjectPtr, bmpDisabled, OPTIONAL_CSTRING, style, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    uint32_t id;
    HWND hCtrl = NULL;

    bool draw = (argumentExists(6) && StrStrI(style, "NODRAW") == NULL);

    // Note that the dialog does not need to yet be created for this method,
    // unless we are going to draw the button, or the BMP args are resource IDs
    // in the resource DLL of the dialog.
    RexxObjectPtr result;
    if ( draw )
    {
        result = dlgExtControlSetup(context, self, rxID, &pcpbd, &id, &hCtrl);
    }
    else
    {
        result = dlgExtControlSetup(context, self, rxID, &pcpbd, &id, NULL);
    }

    if ( result != TheZeroObj )
    {
        return result;
    }

    bool noUnderlyingDlg = pcpbd->hDlg == NULL ? true : false;

    size_t i;
    if ( findBmpForID(pcpbd, id, &i) )
    {
        bool frame      = false;
        bool inMemory   = false;
        bool stretch    = false;
        bool usePalette = false;
        if ( argumentExists(6) )
        {
            if ( StrStrI(style, "FRAME")    != NULL ) frame = true;
            if ( StrStrI(style, "INMEMORY") != NULL ) inMemory = true;
            if ( StrStrI(style, "STRETCH")  != NULL ) stretch = true;
            if ( StrStrI(style, "USEPAL")   != NULL ) usePalette = true;
        }

        if ( (pcpbd->BmpTab[i].loaded & 0x1011) == 1 )
        {
           safeLocalFree(pcpbd->BmpTab[i].bitmapID);
           safeLocalFree(pcpbd->BmpTab[i].bmpFocusID);
           safeLocalFree(pcpbd->BmpTab[i].bmpSelectID);
           safeLocalFree(pcpbd->BmpTab[i].bmpDisableID);
        }
        else if ( pcpbd->BmpTab[i].loaded == 0 )
        {
           safeDeleteObject(pcpbd->BmpTab[i].bitmapID);
           safeDeleteObject(pcpbd->BmpTab[i].bmpFocusID);
           safeDeleteObject(pcpbd->BmpTab[i].bmpSelectID);
           safeDeleteObject(pcpbd->BmpTab[i].bmpDisableID);
        }

        pcpbd->BmpTab[i].bitmapID     = NULL;
        pcpbd->BmpTab[i].bmpFocusID   = NULL;
        pcpbd->BmpTab[i].bmpSelectID  = NULL;
        pcpbd->BmpTab[i].bmpDisableID = NULL;

        pcpbd->BmpTab[i].frame  = frame;
        pcpbd->BmpTab[i].loaded = 0;

        int32_t bmpID = -1;
        CSTRING bmp   = "";

        bool isIntResource = getIdOrName(context, pcpbd, bmpNormal, &bmpID, &bmp);
        if ( isIntResource && noUnderlyingDlg )
        {
            noWindowsDialogException(context, self);
            return TheOneObj;
        }

        BitmapButtonBMPType normal = getBMPType(inMemory, isIntResource);

        assignBitmap(pcpbd, i, bmp, bmpID, PBSS_NORMAL, inMemory, isIntResource);

        if ( argumentExists(3) )
        {
            isIntResource = getIdOrName(context, pcpbd, bmpFocused, &bmpID, &bmp);
            if ( isIntResource && noUnderlyingDlg )
            {
                noWindowsDialogException(context, self);
                return TheOneObj;
            }

            BitmapButtonBMPType focused = getBMPType(inMemory, isIntResource);
            if ( focused != normal )
            {
                bitmapTypeMismatchException(context, bmpType2String(normal), bmpType2String(focused), 3);
                return TheOneObj;
            }

            assignBitmap(pcpbd, i, bmp, bmpID, PBSS_DEFAULTED, inMemory, isIntResource);
        }
        if ( argumentExists(4) )
        {
            isIntResource = getIdOrName(context, pcpbd, bmpSelected, &bmpID, &bmp);
            if ( isIntResource && noUnderlyingDlg )
            {
                noWindowsDialogException(context, self);
                return TheOneObj;
            }

            BitmapButtonBMPType selected = getBMPType(inMemory, isIntResource);
            if ( selected != normal )
            {
                bitmapTypeMismatchException(context, bmpType2String(normal), bmpType2String(selected), 4);
                return TheOneObj;
            }

            assignBitmap(pcpbd, i, bmp, bmpID, PBSS_PRESSED, inMemory, isIntResource);
        }
        if ( argumentExists(5) )
        {
            isIntResource = getIdOrName(context, pcpbd, bmpDisabled, &bmpID, &bmp);
            if ( isIntResource && noUnderlyingDlg )
            {
                noWindowsDialogException(context, self);
                return TheOneObj;
            }

            BitmapButtonBMPType disabled = getBMPType(inMemory, isIntResource);
            if ( disabled != normal )
            {
                bitmapTypeMismatchException(context, bmpType2String(normal), bmpType2String(disabled), 5);
                return TheOneObj;
            }

            assignBitmap(pcpbd, i, bmp, bmpID, PBSS_DISABLED, inMemory, isIntResource);
        }

        if ( stretch && pcpbd->BmpTab[i].loaded )
        {
            pcpbd->BmpTab[i].loaded |= 0x0100;
        }

        // Maybe create a palette that conforms to the bitmap colors.
        if ( usePalette )
        {
           if ( pcpbd->colorPalette != NULL )
           {
               DeleteObject(pcpbd->colorPalette);
           }

           pcpbd->colorPalette = createDIBPalette((LPBITMAPINFO)pcpbd->BmpTab[i].bitmapID);
           setSysPalColors(pcpbd->colorPalette);
        }

        if ( draw )
        {
            drawButton(pcpbd->hDlg, hCtrl, id);
        }
        return TheZeroObj;
    }
    return TheOneObj;
}


RexxMethod9(RexxObjectPtr, dlgext_drawBitmap, OPTIONAL_RexxObjectPtr, ignored, RexxObjectPtr, rxID,
            OPTIONAL_int32_t, x,  OPTIONAL_int32_t, y, OPTIONAL_int32_t, xS, OPTIONAL_int32_t, yS,
            OPTIONAL_int32_t, xL, OPTIONAL_int32_t, yL, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    uint32_t id;
    HWND hCtrl;

    RexxObjectPtr result = dlgExtControlSetup(context, self, rxID, &pcpbd, &id, &hCtrl);
    if ( result != TheZeroObj )
    {
        return result;
    }

    size_t index;
    if ( findBmpForID(pcpbd, id, &index) )
    {
        HBITMAP hBmp = (HBITMAP)pcpbd->BmpTab[index].bitmapID;

        if ( hBmp != NULL )
        {
            HPALETTE hP;

            HDC hDC = GetDC(hCtrl);
            if ( pcpbd->colorPalette != NULL )
            {
               hP = SelectPalette(hDC, pcpbd->colorPalette, 0);
               RealizePalette(hDC);
            }
            if ( pcpbd->BmpTab[index].loaded == 0 )
            {
                HDC hDC2 = CreateCompatibleDC(hDC);

                SelectObject(hDC2, hBmp);

                if ( xL == 0 || yL == 0 )
                {
                    BITMAP bmpInfo;
                    GetObject(hBmp, sizeof(BITMAP), &bmpInfo);

                    xL = (xL == 0 ? bmpInfo.bmWidth  : xL);
                    yL = (yL == 0 ? bmpInfo.bmHeight : yL);
                }

                BitBlt(hDC, x, y, xL, yL, hDC2, xS, yS, SRCCOPY);
                DeleteDC(hDC2);
            }
            else if ( forSureHaveBitmap(pcpbd, index) )
            {
                // Original comment: This if has been added because of a
                // violation error moving animated button dialogs a lot
                //
                // I think what was meant was the 'if' part of the else.

                xL = (xL == 0 ? dibWidth(hBmp)  : xL);
                yL = (yL == 0 ? dibHeight(hBmp) : yL);

                StretchDIBits(hDC, x, y, xL, yL, xS, yS, xL, yL, dibPBits(hBmp), dibPBI(hBmp), DIB_RGB_COLORS, SRCCOPY);
            }

            if ( pcpbd->colorPalette != NULL )
            {
               SelectPalette(hDC, hP, 0);
            }
            ReleaseDC(hCtrl, hDC);
            return TheZeroObj;
        }
    }
    return TheOneObj;
}


/** DialogExtensions::getBitmapPostion()
 *
 *  Retrieves the postion, (X,Y) co-ordinates of the upper left corner of a
 *  bitmap within a bitmap button.
 *
 *  @param  id   The resource ID of the bitmap button.  Can be numeric or
 *               symbolic.
 *  @param  pos  [IN / OUT]  A .Point object.  The position is returned here on
 *               success.  On failure, the point object is left unchanged.
 *
 *  @return  True on success, otherwise false.
 */
RexxMethod3(RexxObjectPtr, dlgext_getBitmapPosition, RexxObjectPtr, rxID, RexxObjectPtr, pos, OSELF, self)
{
    PORXPOINT p = rxGetPoint(context, pos, 2);
    if ( p == NULL )
    {
        return TheFalseObj;
    }

    pCPlainBaseDialog pcpbd;
    uint32_t id;
    if ( dlgExtControlSetup(context, self, rxID, &pcpbd, &id, NULL) != TheZeroObj )
    {
        return TheFalseObj;
    }

    size_t index;

    if ( findBmpForID(pcpbd, id, &index) )
    {
        p->x = pcpbd->BmpTab[index].displaceX;
        p->y = pcpbd->BmpTab[index].displaceY;
        return TheTrueObj;
    }
    return TheFalseObj;
}

RexxMethod3(RexxObjectPtr, dlgext_setBitmapPosition, RexxObjectPtr, rxID, ARGLIST, args, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    uint32_t id;
    if ( dlgExtControlSetup(context, self, rxID, &pcpbd, &id, NULL) != TheZeroObj )
    {
        return TheNegativeOneObj;
    }

    size_t arraySize;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 2, 3, &arraySize, &argsUsed) )
    {
        return TheOneObj;
    }

    size_t index;
    if ( findBmpForID(pcpbd, id, &index) )
    {
        pcpbd->BmpTab[index].displaceX = point.x;
        pcpbd->BmpTab[index].displaceY = point.y;
        return TheZeroObj;
    }
    return TheOneObj;
}

/** DialogExtensions::getBitmapSize()
 *  DialogExtensions::getBitmapSizeX()
 *  DialogExtensions::getBitmapSizeY()
 *
 *
 *
 */
RexxMethod3(RexxObjectPtr, dlgext_getBitmapSize, RexxObjectPtr, rxID, NAME, method, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    uint32_t id;

    if ( dlgExtControlSetup(context, self, rxID, &pcpbd, &id, NULL) != TheZeroObj )
    {
        return TheNegativeOneObj;
    }

    size_t index;
    if ( findBmpForID(pcpbd, id, &index) )
    {
        int x, y;

        if ( pcpbd->BmpTab[index].loaded == 0 )
        {
            BITMAP bmpInfo;
            GetObject(pcpbd->BmpTab[index].bitmapID, sizeof(BITMAP), &bmpInfo);
            x = bmpInfo.bmWidth;
            y = bmpInfo.bmHeight;
        }
        else
        {
            x = dibWidth(pcpbd->BmpTab[index].bitmapID);
            y = dibHeight(pcpbd->BmpTab[index].bitmapID);
        }

        switch ( method[13] )
        {
            case '\0' :
                return rxNewSize(context, x, y);
            case 'X' :
                return context->Int32(x);
            case 'Y' :
                return context->Int32(y);
            default :
                break;
        }
    }
    return TheNegativeOneObj;
}


/** DialogExtensions:scrollButton()
 *
 *  Moves the specified rectangle within the button and redraws the uncovered
 *  area with the button background color.  This method is used to move bitmaps
 *  within bitmap buttons.
 *
 *  @note  Sets .SystemErrorCode.
 *
 *  @remarks  TODO convert to using an options .Rect arg.
 *
 *            The original ooDialog external function had an option whether or
 *            not to redraw the uncovered portion of the button.  The option was
 *            not documented and internally the function was always called with
 *            true.  That option was therefore eliminated
 *
 *            This method was originally a Button method, the scroll() method.
 *            But it belongs in the rather silly bitmap button category.  Since
 *            the majority of these methods were implemented in the dialog
 *            extensions class, this method was moved here.
 *
 *            The DialogExtensions class already had the scrollButton() method,
 *            which simply forwarded to the Button::scroll() method.  So we just
 *            reversed the logic and have the Button::scroll() method forward to
 *            this method.
 *
 *            For backwards compatibility, the Button::scroll() method has to be
 *            maintained.  It now simply forwards to this method.  The button
 *            method is deprecated, and therefore no longer documented.
 *
 *            Although we use logical_t for the return, the actual return is 0
 *            for success and 1 for failure.
 */
RexxMethod8(logical_t, dlgext_scrollButton, RexxObjectPtr, rxID, int32_t, xPos, int32_t, yPos, int32_t, left, int32_t, top,
            int32_t, right, int32_t, bottom, OSELF, self)
{
    pCPlainBaseDialog pcpbd;
    HWND hwnd;

    if ( dlgExtControlSetup(context, self, rxID, &pcpbd, NULL, &hwnd) != TheZeroObj )
    {
        goto err_out;
    }

    // Enforce that this is a button control.
    oodControl_t controlType = oodName2controlType("PUSHBUTTON");
    if ( ! isControlMatch(hwnd, controlType) )
    {
        goto err_out;
    }

    RECT r;
    if ( GetWindowRect(hwnd, &r) )
    {
        RECT rs;
        HDC hDC = GetDC(hwnd);

        rs.left = left;
        rs.top = top;
        rs.right = right;
        rs.bottom = bottom;

        r.right = r.right - r.left;
        r.bottom = r.bottom - r.top;
        r.left = 0;
        r.top = 0;

        if ( ScrollDC(hDC, xPos, yPos, &rs, &r, NULL, NULL) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            goto err_out;
        }

        // Draw uncovered rectangle with background color.
        HBRUSH hBrush, hOldBrush;
        HPEN hOldPen, hPen;

        if ( pcpbd->bkgBrush )
        {
            hBrush = pcpbd->bkgBrush;
        }
        else
        {
            hBrush = GetSysColorBrush(COLOR_BTNFACE);
        }

        hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
        hOldPen = (HPEN)SelectObject(hDC, hPen);
        hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

        if ( xPos > 0 )
        {
            Rectangle(hDC, rs.left, rs.top, rs.left + xPos, rs.bottom);
        }
        else if ( xPos < 0 )
        {
            Rectangle(hDC, rs.right + xPos, rs.top, rs.right, rs.bottom);
        }

        if ( yPos > 0 )
        {
            Rectangle(hDC, rs.left, rs.top, rs.right, rs.top + yPos);
        }
        else if ( yPos < 0 )
        {
            Rectangle(hDC, rs.left, rs.bottom + yPos, rs.right, rs.bottom);
        }

        SelectObject(hDC, hOldBrush);
        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);

        ReleaseDC(hwnd, hDC);
        return 0;
    }

err_out:
    return 1;
}


/** Button::dimBitmap()
 *
 *  Draws a bitmap on the client area of the button control step by step.
 *
 *  @param  id    The resource ID of the bitmap button.  Can be numeric or
 *                symbolic.
 *  @param  hBmp  A handle to the bitmap loaded with loadBitmap.
 *
 *  @param  cx, cy [required] The width and height of the bitmap.
 *
 *  @param  stepx, stepy [optional] The number of pixels to increment the x and
 *                       y position of the bitmap at each step. The default is 2
 *                       pixels for both cx and cy.
 *
 *  @param  steps [optional]  The number of iterations used to draw the bitmap.
 *                            The bitmap is redrawn at each step. The default is
 *                            10.
 *
 *  @notes  We enforce that the control must be a button.
 *
 *  @remarks  This method was originally a Button method, but it belongs in the
 *            rather silly bitmap button category.  Since the majority of these
 *            methods were implemented in the dialog extensions class, this
 *            method was moved here.
 *
 *            For backwards compatibility, the Button::dimBitmap() method has to
 *            be maintained.  It now simply forwards to this method.  The button
 *            method is deprecated, and therefore no longer documented.
 */
RexxMethod8(RexxObjectPtr, dlgext_dimBitmap, RexxObjectPtr, rxID, POINTERSTRING, hBmp, uint32_t, width, uint32_t, height,
            OPTIONAL_uint32_t, stepX, OPTIONAL_uint32_t, stepY, OPTIONAL_uint32_t, steps, OSELF, self)
{
    HWND hwnd;

    if ( dlgExtControlSetup(context, self, rxID, NULL, NULL, &hwnd) != TheZeroObj )
    {
        return TheNegativeOneObj;
    }

    // Enforce that this is a button control.
    oodControl_t controlType = oodName2controlType("PUSHBUTTON");
    if ( ! isControlMatch(hwnd, controlType) )
    {
        return TheNegativeOneObj;
    }

    stepX = (argumentOmitted(4) ? 2  : stepX);
    stepY = (argumentOmitted(5) ? 2  : stepY);
    steps = (argumentOmitted(6) ? 10 : steps);

    HDC hDC = GetWindowDC(hwnd);

    LOGBRUSH logicalBrush;
    logicalBrush.lbStyle = BS_DIBPATTERNPT;
    logicalBrush.lbColor = DIB_RGB_COLORS;
    logicalBrush.lbHatch = (ULONG_PTR)hBmp;

    HBRUSH hBrush = CreateBrushIndirect(&logicalBrush);
    HPEN hPen = CreatePen(PS_NULL, 0, PALETTEINDEX(0));

    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
    HPEN oldPen = (HPEN)SelectObject(hDC, hPen);

    uint32_t diffY = steps * stepY;
    uint32_t diffX = steps * stepX;

    uint32_t a, i, j, x, y;

    for ( a = 0; a < steps; a++ )
    {
       for ( y = a * stepY, i = 0; i < height / steps; y += diffY, i++ )
       {
          for ( x = a * stepX, j = 0; j < width / steps; x += diffX, j++ )
          {
              Rectangle(hDC, x - a * stepX, y - a * stepY, x + stepX + 1, y + stepY + 1);
          }
       }
    }

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(oldBrush);
    DeleteObject(oldPen);
    ReleaseDC(hwnd, hDC);

    return TheZeroObj;
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


/** DialogExtensions::getWindowDC()
 *
 *  Retrieves the device context (DC) for the entire window.  For dialog windows
 *  this includes the title bar, menus, and scroll bars.
 *
 *  A window device context permits painting anywhere in a window, because the
 *  origin of the device context is the upper-left corner of the window instead
 *  of the client area.
 *
 *  The operating system assigns default attributes to the window device context
 *  each time it retrieves the device context.  Previous attributes are lost.
 *
 *  @param  hwnd  The handle of the window whose device context is to be
 *                retrieved.
 *
 *  @return  The handle of the device context on success, a null handle on
 *           failure.
 *
 *  @note  Sets the .SystemErrorCode on failure.
 *
 *         The Microsoft documentation says of the underlying API used in this
 *         method: [The API] is intended for special painting effects within a
 *         window's nonclient area.  Painting in nonclient areas of any window
 *         is not recommended.
 */
RexxMethod1(POINTERSTRING, dlgext_getWindowDC, POINTERSTRING, hwnd)
{
    oodResetSysErrCode(context->threadContext);

    if ( hwnd == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return NULL;
    }

    HDC hDC = GetWindowDC((HWND)hwnd);
    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return hDC;
}


/** DialogExtensions::freeWindowDC()
 *
 *
 * @param hDC
 *
 * @remarks  The MSDN docs make no mention of ReleaseDC() setting last error.
 */
RexxMethod2(logical_t, dlgext_freeWindowDC, POINTERSTRING, hwnd, POINTERSTRING, hDC)
{
    oodResetSysErrCode(context->threadContext);

    if ( hwnd == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }
    return (ReleaseDC((HWND)hwnd, (HDC)hDC) == 1 ? TRUE : FALSE);
}


/** DialogExtensions::createBrush()
 *
 *  Retrieves a handle to a graphics brush.  The type of brush is dependent on
 *  the supplied arguments.
 *
 *  This method is exactly the same as the WindowsExtensions createBrush()
 *  method except that it also allows the brush specifier to be a resource ID of
 *  a bitmap compiled into the resource DLL of a ResDialog.
 *
 * If both args were omitted,then a stock hollow brush is returned.  When only
 * the color arg is specified, then a solid color brush of the color specified
 * is returned.
 *
 * The second argument can either be a keyword to specify a brush pattern, the
 * resource ID of a bitmap, or the file name of a bitmap to use as the brush.
 *
 * @param color           [OPTIONAL]  The color of the brush.  If omitted, the
 *                        default is 1.
 * @param brushSpecifier  [OPTIONAL]  If specified, can be either a keyword for
 *                        the hatch pattern of a brush, the resource ID of a
 *                        bitmap compiled into the resource DLL of a ResDialog,
 *                        or the name of a bitmap file to use for the brush.
 *
 * @return The handle to the brush on success, or a null handle on failure.
 *
 * @note  Sets the .SystemErrorCode on failure.
 *
 * @remarks  In essence, this createBrush() method overrides the
 *           WindowExtensions::createBrush in a dialog object.
 */
RexxMethod3(POINTERSTRING, dlgext_createBrush, OPTIONAL_uint32_t, color, OPTIONAL_RexxObjectPtr, specifier, OSELF, self)
{
    HBRUSH hBrush = NULL;
    int32_t resID;

    if ( argumentExists(2) && context->Int32(specifier, &resID) )
    {
        pCPlainBaseDialog pcpbd = dlgExtSetup(context, self);
        if ( pcpbd != NULL )
        {
            HBITMAP hBmp = LoadBitmap(pcpbd->hInstance, MAKEINTRESOURCE(resID));
            if ( hBmp == NULL )
            {
                oodSetSysErrCode(context->threadContext);
                goto done_out;
            }

            hBrush = CreatePatternBrush(hBmp);
            if ( hBrush == NULL )
            {
                oodSetSysErrCode(context->threadContext);
            }

            DeleteObject(hBmp);
            goto done_out;
        }
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
    }
    else
    {
        return oodCreateBrush(context, color, (argumentExists(2) ? context->ObjectToStringValue(specifier) : ""));
    }

done_out:
    return hBrush;
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
 *  DialogExtensions::setControlSysColor()
 *
 *  @remarks  For sys color we accept keyword IDs, but not for regular colors.
 *            At some point we might convert regular colors to accept keywords
 *            also.
 *
 *            Since accepting keywords is a 4.2.0 or later feature, we raise a
 *            syntax error if an unsupported keyword is used.
 *
 *            The setControlColor method is enhanced from the original to make
 *            the background color optional and to allow a third argument.  The
 *            third argument allows the user to specify a COLORREF instead of a
 *            pallete index.
 *
 *            For setControlSysColor, the background argument is now also
 *            optional.
 */
RexxMethod6(int32_t, dlgext_setControlColor, RexxObjectPtr, rxID, OPTIONAL_RexxObjectPtr, rxBG, OPTIONAL_RexxObjectPtr, rxFG,
            OPTIONAL_logical_t, isClrRef, NAME, method, OSELF, self)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(context, self);
    if ( pcpbd == NULL )
    {
        baseClassInitializationException(context);
        return 0;
    }

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1, true) )
    {
        return -1;
    }

    uint32_t bkColor     = CLR_DEFAULT;
    uint32_t fgColor     = CLR_DEFAULT;
    bool     useSysColor = (method[10] == 'S');

    RexxMethodContext *c = context;
    if ( useSysColor )
    {
        if ( argumentExists(2) && ! getSystemColor(context, rxBG, &bkColor, 2) )
        {
            return -1;
        }
        if ( argumentExists(3) && ! getSystemColor(context, rxFG, &fgColor, 3) )
        {
            return -1;
        }
    }
    else
    {
        RexxMethodContext *c = context;
        if ( argumentExists(2) && ! c->UnsignedInt32(rxBG, &bkColor) )
        {
            return -1;
        }
        if ( argumentExists(3) && ! context->UnsignedInt32(rxFG, &fgColor) )
        {
            return -1;
        }
    }

    if ( ! (useSysColor || isClrRef) )
    {
        if ( bkColor != CLR_DEFAULT )
        {
            bkColor = PALETTEINDEX(bkColor);
        }
        if ( fgColor != CLR_DEFAULT )
        {
            fgColor = PALETTEINDEX(fgColor);
        }
    }

    return (int32_t)oodColorTable(context, pcpbd, id, bkColor, fgColor, useSysColor);
}


/** DialogExtensions::writeToWindow()
 *
 *
 *  @notes  Sets the .SystemErrorCode.
 *
 *          Note that in one of the lower called functions, drawFontToDC(), the
 *          Windows SelectObject() function does not set the system error code
 *          (according to the docs.)  It is unlikely that it fails, but just for
 *          completedness, we set it to ERROR_SIGNAL_REFUSED if it does fail.
 *
 *  @remarks  This method uses the correct process to create the font.
 *
 *            The return from writeToWindow() was never documented in ooRexx
 *            3.2.0, but it returned 0 for success and 1 for failure.  Even
 *            though we use logical_t for the return, we maintain the 0 for
 *            success.
 */
RexxMethod9(logical_t, dlgext_writeToWindow, POINTERSTRING, hwnd, int32_t, xPos, int32_t, yPos, CSTRING, text,
            OPTIONAL_CSTRING, fontName, OPTIONAL_uint32_t, fontSize, OPTIONAL_CSTRING, fontStyle,
            OPTIONAL_int32_t, fgColor, OPTIONAL_int32_t, bkColor)
{
    return oodWriteToWindow(context, (HWND)hwnd, xPos, yPos, text, fontName, fontSize, fontStyle, fgColor, bkColor);
}


VOID CALLBACK scrollTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    SetEvent(TimerEvent);
}

/** DialogExtensions::scrollText()
 *  DialogExtensions::scrollInButton()
 *  DialogExtensions::scrollInControl()
 *
 *  @param  rxObj  Either a window handle, (for scrollText,) or a dialog control
 *                 resource ID, (for scrollInButton and scrollInControl.)
 *
 *  @note  Sets the .SystemErrorCode, under some circumstances.
 *
 *  @remarks  This method uses the correct process to create the font.
 */
RexxMethod10(RexxObjectPtr, dlgext_scrollText, RexxObjectPtr, rxObj, OPTIONAL_CSTRING, text, OPTIONAL_CSTRING, fontName,
            OPTIONAL_uint32_t, fontSize, OPTIONAL_CSTRING, fontStyle, OPTIONAL_uint32_t, displaceY, OPTIONAL_int32_t, step,
            OPTIONAL_uint32_t, sleep, OPTIONAL_int32_t, color, OSELF, self)
{
    pCPlainBaseDialog pcpbd = NULL;
    HWND hCtrl = NULL;
    RexxObjectPtr result = TheOneObj;

    if ( *(context->GetMessageName() + 6) == 'I' )
    {
        dlgExtControlSetup(context, self, rxObj, &pcpbd, NULL, &hCtrl);
    }
    else
    {
        pcpbd = dlgExtSetup(context, self);
        hCtrl = (HWND)string2pointer(context->ObjectToStringValue(rxObj));
    }

    if ( pcpbd == NULL || hCtrl == NULL )
    {
        goto quick_out;
    }

    if ( pcpbd->scrollNow || rxArgCount(context) == 1 )
    {
        SendMessage(pcpbd->hDlg, WM_USER_INTERRUPTSCROLL, (WPARAM)hCtrl, 0);
        result = TheZeroObj;
        goto quick_out;
    }

    pcpbd->scrollNow = true;

    text      = (argumentOmitted(2) ? ""       : text);
    fontName  = (argumentOmitted(3) ? "System" : fontName);
    fontSize  = (argumentOmitted(4) ? 10       : fontSize);
    fontStyle = (argumentOmitted(5) ? ""       : fontStyle);
    displaceY = (argumentOmitted(6) ? 0        : displaceY);
    step      = (argumentOmitted(7) ? 4        : step);
    sleep     = (argumentOmitted(8) ? 10       : sleep);
    color     = (argumentOmitted(9) ? 0        : color);

    HDC hDC = GetWindowDC(hCtrl);
    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto quick_out;
    }

    RECT r;

    GetWindowRect(hCtrl, &r);

    // The assumption was that all of the following succeed.  Really, the only
    // thing that might fail is creating the font.

    HFONT hFont = oodGenericFont(fontName, fontSize, fontStyle);
    if ( hFont == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto quick_out;
    }

    HPEN hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
    if ( hPen == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto quick_out;
    }

    HBRUSH hBrush = GetSysColorBrush(COLOR_BTNFACE);

    SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
    HFONT oldFont = (HFONT)SelectObject(hDC, hFont);
    HPEN oldPen = (HPEN)SelectObject(hDC, hPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    if ( color > 0 )
    {
        SetTextColor(hDC, PALETTEINDEX(color));
    }

    SIZE s;
    size_t textLength = strlen(text);
    BOOL rc = GetTextExtentPoint32(hDC, text, (int)textLength, &s);
    if ( rc == FALSE )
    {
        oodSetSysErrCode(context->threadContext);
        goto clean_up_out;
    }

    r.right  = r.right - r.left;
    r.bottom = r.bottom - r.top + displaceY;
    r.top    = displaceY;
    r.left   = 0;

    CSTRING textPointer = text;
    size_t j = 0;
    int32_t displace = 0;
    SIZE s1;

    rc = GetTextExtentPoint32(hDC, textPointer, 1, &s1);
    if ( rc == FALSE )
    {
        oodSetSysErrCode(context->threadContext);
        goto clean_up_out;
    }

    RECT rs, rclip;
    rclip = r;
    rs.top    = r.top;
    rs.bottom = r.top + s.cy + 2;
    rs.right  = r.right;
    rs.left   = r.right;

    if ( sleep > 0 )
    {
        if ( TimerEvent == NULL )
        {
            TimerEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
            Timer = SetTimer(NULL, GetCurrentThreadId(), sleep, (TIMERPROC)scrollTimerProc);
            TimerCount++;
        }
        else
        {
            TimerCount++;
        }
    }
    else
    {
        Timer = 0;
    }

    ScrollingButton = hCtrl;
    int i;

    for ( i = step; i <= r.right + s.cx; i += step )
    {
        if ( i >= s.cx + step)
        {
            Rectangle(hDC, r.right - i + s.cx, r.top, r.right - i + s.cx + step + step, r.top + s.cy + 2);
        }

        if ( j < textLength)
        {
            if ( RedrawScrollingButton == hCtrl )
            {
                rc = TextOut(hDC, r.right - i, r.top, text, (int)textLength);
            }
            else
            {
                rc = TextOut(hDC, r.right - i+displace, r.top, textPointer, 1);
            }
        }

        if ( j < textLength && rc == FALSE )
        {
            oodSetSysErrCode(context->threadContext);
            break;
        }

        RedrawScrollingButton = NULL;

        if ( i - displace > s1.cx )
        {
            textPointer++;
            j++;
            displace += s1.cx;
            rc = GetTextExtentPoint32(hDC, textPointer, 1, &s1);
        }

        rs.left -= step;
        rc = FALSE;

        if ( pcpbd->stopScroll == (WPARAM)hCtrl )
        {
            pcpbd->stopScroll = 0;
            break;
        }

        if ( ScrollDC(hDC, -(int)step, 0, &rs, &rclip, NULL, NULL) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            break;
        }

        if ( Timer != 0 )
        {
            WaitForSingleObject(TimerEvent, (DWORD)((double)sleep*1.5));
            ResetEvent(TimerEvent);
        }
    }

    if ( Timer != 0 )
    {
        if ( TimerCount == 1 )
        {
            KillTimer(NULL, Timer);
            if ( TimerEvent != NULL )
            {
                CloseHandle(TimerEvent);
            }
            TimerEvent = NULL;
            TimerCount = 0;
            Timer = 0;
        }
        else
        {
            TimerCount--;
        }
    }

    ScrollingButton = NULL;

    // Dialog could have been closed while we were scrolling.
    if ( ! pcpbd->isActive || ! IsWindow(pcpbd->hDlg) )
    {
        goto quick_out;
    }

    Rectangle(hDC, r.left, r.top, r.right, r.bottom);
    result = TheZeroObj;

clean_up_out:

    SelectObject(hDC, oldFont);
    SelectObject(hDC, oldPen);
    SelectObject(hDC, oldBrush);

    // Don't delete hBrush, its a system cached brush
    safeDeleteObject(hPen);
    safeDeleteObject(hFont);
    ReleaseDC(hCtrl, hDC);

quick_out:
    if ( pcpbd != NULL )
    {
        pcpbd->scrollNow = false;
    }
    return result;
}


HFONT createFontFromName(CSTRING name, uint32_t size)
{
    HDC hdc = GetDC(NULL);
    HFONT font = createFontFromName(GetDeviceCaps(hdc, LOGPIXELSY), name, size);
    ReleaseDC(NULL, hdc);
    return font;
}

HFONT createFontFromName(int logicalPixelsY, CSTRING name, uint32_t size)
{
    LOGFONT lf={0};

    strcpy(lf.lfFaceName, name);
    lf.lfHeight = -MulDiv(size, logicalPixelsY, 72);
    return CreateFontIndirect(&lf);
}


/**
 * Correctly converts from a device coordinate (pixel) to a dialog unit
 * coordinate, for any dialog.
 *
 * MapDialogRect() correctly converts from dialog units to pixels for any
 * dialog.  But, there is no conversion the other way, from pixels to dialog
 * units.
 *
 * MSDN gives these formulas to convert from pixel to dialog unit:
 *
 * templateunitX = MulDiv(pixelX, 4, baseUnitX);
 * templateunitY = MulDiv(pixelY, 8, baseUnitY);
 *
 * Now, you just need to get the correct dialog base unit.
 *
 * GetDialogBaseUnits() always assumes the font is the system font.  If the
 * dialog uses any other font, the base units returned will be incorrect.
 *
 * MSDN, again, has two methods for calculating the correct base units for any
 * font.  This way is the simplest, but it requires the window handle to the
 * dialog.
 *
 * Rect rect{0, 0, 4, 8};
 * MapDialogRect(hDlg, &rc);
 * int baseUnitY = rc.bottom;
 * int baseUnitX = rc.right;
 *
 * @param hwnd   Window handle of the dialog.  If this is not a dialog window
 *               handle, this method will fail.
 *
 * @param point  Pointer to an array of POINT structs.  Not that a SIZE struct
 *               and a POINT struct are binary equivalents.  They both have two
 *               fields, each of which is a long.  Only the field names differ,
 *               cx and cy for a SIZE and x and y for a POINT.  Therefore you
 *               can cast a SIZE pointer to a POINT pointer.
 *
 * @param count  The number of point structs in the array.
 *
 * @return true on success, false otherwise.
 *
 * Dialog class: #32770
 */
bool screenToDlgUnit(HWND hwnd, POINT *point, size_t count)
{
    RECT r = {0, 0, 4, 8};

    if ( MapDialogRect(hwnd, &r) )
    {
        for ( size_t i = 0; i < count; i++ )
        {
            pixel2du(point + i, r.right, r.bottom);
        }
        return true;
    }
    return false;
}

/**
 * Given a device context with the correct font already selected into it,
 * correctly converts from a device coordinate (pixel) to a dialog unit
 * coordinate.  The correct font means, the font actually used by the dialog.
 *
 * See screenToDlgUnit(HWND, POINT *) for a discussion of this
 * conversion.
 *
 * @param hdc    Handle to a device context with the dialog's font selected into
 *               it.
 *
 * @param point  Pointer to an array of POINT structs.  Not that a SIZE struct
 *               and a POINT struct are binary equivalents.  They both have two
 *               fields, each of which is a long.  Only the field names differ,
 *               cx and cy for a SIZE and x and y for a POINT.  Therefore you
 *               can cast a SIZE pointer to a POINT pointer.
 *
 * @param count  The number of point structs in the array.
 *
 * @return true on success, false otherwise.
 *
 */
void screenToDlgUnit(HDC hdc, POINT *point, size_t count)
{
    TEXTMETRIC tm;
    SIZE size;
    GetTextMetrics(hdc, &tm);
    int baseUnitY = tm.tmHeight;

    GetTextExtentPoint32(hdc, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
    int baseUnitX = (size.cx / 26 + 1) / 2;

    for ( size_t i = 0; i < count; i++ )
    {
        pixel2du(point + i, baseUnitX, baseUnitY);
    }
}


/**
 * Calculates the dialog base units using the window handle of the dialog.
 *
 * @param hDlg       The window handle of the dialog whose base units are to be
 *                   calculated.
 * @param baseUnitX  The X base unit is returned here.
 * @param baseUnitY  The Y base unit is returned here.
 */
void calcDlgBaseUnits(HWND hDlg, int *baseUnitX, int *baseUnitY)
{
    RECT r = {0, 0, 4, 8};

    MapDialogRect(hDlg, &r);
    *baseUnitX = r.right;
    *baseUnitY = r.bottom;
}

/**
 * Calculates the dialog base units for a Rexx dialog object.  The underlying
 * Windows dialog does not need to have been created.  The font name and size
 * are gotten from the Rexx dialog object.
 *
 * The base units are calculated using the font of the dialog.  If the
 * underlying Windows dialog is then created later using a different font, the
 * base units will be incorrect.
 *
 * Typically, the correct way to get the right base units would be something
 * like this:
 *
 * dlg = .MyDialog~new(...)
 * dlg~setDlgFont("Tahoma", 14)
 *
 * @param c          Method context we are operating in.
 * @param fontName   The font name in use for the Rexx dialog object.
 * @param fontSize   The font size in use for the Rexx dialog object.
 * @param baseUnitX  The X base unit is returned here.
 * @param baseUnitY  The Y base unit is returned here.
 *
 * @return True on success, false on failure.  On failure an exception has been
 *         raised.
 *
 * @note  It is presumed that the font name and size come from a Rexx dialog
 *        object.
 *
 * @remarks  Once the dialog has been created use:
 *
 *           calcDlgBaseUnits(HWND, baseUnitX, baseUnitY)
 */
bool calcDlgBaseUnits(RexxMethodContext *c, CSTRING fontName, uint32_t fontSize, int *baseUnitX, int *baseUnitY)
{
    HDC hdc = NULL;
    HFONT font = NULL;
    bool result = false;

    hdc = GetDC(NULL);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "GetDC");
        goto done_out;
    }

    font = createFontFromName(hdc, fontName, fontSize);
    if ( font == NULL )
    {
        systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "CreateFontIndirect");
        goto done_out;
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, font);

    TEXTMETRIC tm;
    SIZE size;
    GetTextMetrics(hdc, &tm);
    GetTextExtentPoint32(hdc, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);

    *baseUnitX = (size.cx / 26 + 1) / 2;
    *baseUnitY = tm.tmHeight;

    result = true;

    SelectObject(hdc, hOldFont);

done_out:
    if ( hdc != NULL )
    {
        ReleaseDC(NULL, hdc);
    }
    if ( font != NULL )
    {
        DeleteObject(font);
    }

    return result;
}

/**
 * Given a Rexx dialog object and an array of points in pixels, maps the points
 * to their dialog unit equivalent.
 *
 * @param  c      Method context we are operating in.
 * @param  dlg    The Rexx dialog object.
 * @param  p      Pointer to the array of points
 * @param  count  Count of points in the array.
 *
 * @return  True on success, false on failure.
 *
 * @assumes  The caller has ensured that dlg is indeed a Rexx dialog object.
 */
bool mapPixelToDu(RexxMethodContext *c, RexxObjectPtr dlg, PPOINT p, size_t count)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(c, dlg);

    if ( pcpbd->hDlg != NULL )
    {
        return screenToDlgUnit(pcpbd->hDlg, p, count);
    }

    int buX, buY;
    if ( ! calcDlgBaseUnits(c, pcpbd->fontName, pcpbd->fontSize, &buX, &buY) )
    {
        return false;
    }

    for ( size_t i = 0; i < count; i++ )
    {
        pixel2du(p + i, buX, buY);
    }
    return true;
}

/**
 * Given a Rexx dialog object and a rectangle in dialog units, maps the
 * rectangle to pixels
 *
 * @param  c      Method context we are operating in.
 * @param  dlg    The Rexx dialog object.
 * @param  r      Pointer to the rectangle.
 *
 * @return  True on success, false on failure.
 *
 * @assumes  The caller has ensured that dlg is indeed a Rexx dialog object.
 */
bool mapDuToPixel(RexxMethodContext *c, RexxObjectPtr dlg, PRECT r)
{
    oodResetSysErrCode(c->threadContext);

    pCPlainBaseDialog pcpbd = dlgToCSelf(c, dlg);

    if ( pcpbd->hDlg != NULL )
    {
        if ( MapDialogRect(pcpbd->hDlg, r) == 0 )
        {
            oodSetSysErrCode(c->threadContext);
            return false;
        }
        return true;
    }

    int buX, buY;
    if ( ! calcDlgBaseUnits(c, pcpbd->fontName, pcpbd->fontSize, &buX, &buY) )
    {
        return false;
    }

    r->left   = MulDiv(r->left,   buX, 4);
    r->right  = MulDiv(r->right,  buX, 4);
    r->top    = MulDiv(r->top,    buY, 8);
    r->bottom = MulDiv(r->bottom, buY, 8);
    return true;
}

/**
 * Uses GetTextExtentPoint32() to get the size needed for a string using the
 * specified font and device context.
 *
 * @param font   The font being used for the string.
 * @param hdc    The device context to use.
 * @param text   The string.
 * @param size   Pointer to a SIZE struct used to return the size.
 *
 * @return True if  GetTextExtentPoint32() succeeds, otherwise false.
 *
 * @note   GetTextExtentPoint32() sets last error and SelectObject() does not.
 *         Therefore if this function fails, GetLastError() will return the
 *         correct error code for the failed GetTextExtentPoint32().
 */
bool getTextExtent(HFONT font, HDC hdc, CSTRING text, SIZE *size)
{
    bool success = true;
    HFONT hOldFont = (HFONT)SelectObject(hdc, font);

    if ( GetTextExtentPoint32(hdc, text, (int)strlen(text), size) == 0 )
    {
        success = false;
    }
    SelectObject(hdc, hOldFont);
    return success;
}

bool textSizeIndirect(RexxMethodContext *context, CSTRING text, CSTRING fontName, uint32_t fontSize,
                      SIZE *size, HWND hwnd)
{
    bool success = true;

    // If hwnd is null, GetDC() returns a device context for the whole screen,
    // and that suites our purpose here.
    HDC hdc = GetDC(hwnd);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetDC");
        return false;
    }

    HFONT font = createFontFromName(hdc, fontName, fontSize);
    if ( font == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "CreateFontIndirect");
        ReleaseDC(hwnd, hdc);
        return false;
    }

    if ( ! getTextExtent(font, hdc, text, size) )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetTextExtentPoint32");
        success = false;
    }

    DeleteObject(font);
    ReleaseDC(hwnd, hdc);

    return success;
}

bool textSizeFromWindow(RexxMethodContext *context, CSTRING text, SIZE *size, HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetDC");
        return false;
    }

    // Dialogs and controls need to have been issued a WM_SETFONT or else they
    // return null here.  If null, they are using the stock system font.
    HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if ( font == NULL )
    {
        font = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    bool success = true;
    if ( ! getTextExtent(font, hdc, text, size) )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetTextExtentPoint32");
        success = false;
    }

    ReleaseDC(hwnd, hdc);
    return success;
}


/**
 * Retrieves the size needed, in dialog units, to display a given text string in
 * a dialog.
 *
 * This function first retrieves the size needed for the text in pixels, then
 * accurately converts the pixel size to the dialog unit size for the specified
 * dialog.
 *
 * @param context
 * @param text
 * @param fontName
 * @param fontSize
 * @param hwndFontSrc
 * @param dlgObj
 *
 * @return RexxObjectPtr
 */
bool getTextSize(RexxMethodContext *context, CSTRING text, CSTRING fontName, uint32_t fontSize,
                 HWND hwndFontSrc, RexxObjectPtr dlgObj, PSIZE textSize)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(context, dlgObj);

    // hwndDlg can be null if this is happening before the real dialog is created.
    HWND hwndDlg = pcpbd->hDlg;

    // See if we have a real window handle to use for the call to GetDC().  If
    // both hwndFontSrc and hwndDlg are null, that's okay, we can use null.
    HWND hwndForDC = (hwndFontSrc != NULL ? hwndFontSrc : hwndDlg);

    // If either the font name or the font source window handle were specified,
    // we calculate the text size in pixels now.  The normal case is that the
    // font is coming from the dialog object.
    if ( fontName != NULL )
    {
        if ( ! textSizeIndirect(context, text, fontName, fontSize, textSize, hwndForDC) )
        {
            goto error_out;
        }
    }
    else if ( hwndFontSrc != NULL )
    {
        if ( ! textSizeFromWindow(context, text, textSize, hwndFontSrc) )
        {
            goto error_out;
        }
    }

    // Even if we have already caclulated the text size above, we always have to
    // get the dialog font and select it into a HDC to correctly convert the
    // pixel size to the dialog unit size.
    HDC hdc = GetDC(hwndForDC);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetDC");
        goto error_out;
    }

    HFONT dlgFont = NULL;
    bool createdFont = false;

    if ( hwndDlg == NULL )
    {
        dlgFont = createFontFromName(hdc, pcpbd->fontName, pcpbd->fontSize);
        if ( dlgFont != NULL )
        {
            createdFont = true;
        }
    }
    else
    {
        dlgFont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
    }

    // If dlgFont is null, then, (almost for sure,) the dialog will be using the
    // default system font.  We use that font for the rest of the calculations.
    // This may be inacurrate, but we have to use some font.
    //
    // If the user has called getTextSizeDlg() method before the create()
    // method, and then defines a custom font in create(), then this will be
    // inaccurate for sure.  Need to explain in the docs how to correctly use
    // this functionality.
    if ( dlgFont == NULL )
    {
        dlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, dlgFont);

    // Check if the pixel text size has been determined above.  The normal case
    // will be that it has not.  The normal case is that the size is determined
    // here using the DC with the dialog font selected into it.
    if ( textSize->cx == 0 )
    {
        GetTextExtentPoint32(hdc, text, (int)strlen(text), textSize);
    }

    // Now, convert the pixel size to dialog unit size, and clean up.
    screenToDlgUnit(hdc, (POINT *)textSize, 1);

    SelectObject(hdc, hOldFont);
    ReleaseDC(hwndForDC, hdc);

    if ( createdFont )
    {
        DeleteObject(dlgFont);
    }

    return true;

error_out:
    return false;
}


bool getTextSizeDuActiveDlg(RexxMethodContext *c, pCPlainBaseDialog pcpbd, CSTRING text, SIZE *textSize)
{
    HWND hDlg = pcpbd->hDlg;

    HDC hdc = GetDC(hDlg);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "GetDC");
        return false;
    }

    HFONT dlgFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

    // Even if we have the dialog handle, the font could be null if WM_SETFONT
    // was not used.  In this case the stock system font will be correct.
    if ( dlgFont == NULL )
    {
        dlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, dlgFont);

    GetTextExtentPoint32(hdc, text, (int)strlen(text), textSize);
    screenToDlgUnit(hdc, (POINT *)&textSize, 1);

    SelectObject(hdc, hOldFont);
    ReleaseDC(hDlg, hdc);

    return true;
}
/**
 * Gets the text size of the specified string when we have the CSELF of a
 * dialog, but the underlying dialog has not been created.  This calculates the
 * size using the current assigned font in the CSELF.
 *
 * @param c
 * @param pcpbd
 * @param text
 * @param textSize
 *
 * @return bool
 *
 * @assumes  The caller has already checked that pcpbd->hDlg is null.
 */
bool getTextSizeDuInactiveDlg(RexxMethodContext *c, pCPlainBaseDialog pcpbd, CSTRING text, SIZE *textSize)
{
    // With a null hwnd, this will get a DC for the entire screen and that
    // should be ok.
    HDC hdc = GetDC(NULL);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "GetDC");
        return false;
    }

    bool  createdFont = false;
    HFONT dlgFont     = createFontFromName(hdc, pcpbd->fontName, pcpbd->fontSize);
    if ( dlgFont != NULL )
    {
        createdFont = true;
    }

    // If the createFontFromName() failed, well that is unlikely.  Just use the
    // stock system font in that case.
    if ( dlgFont == NULL )
    {
        dlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, dlgFont);

    GetTextExtentPoint32(hdc, text, (int)strlen(text), textSize);
    screenToDlgUnit(hdc, (POINT *)textSize, 1);

    SelectObject(hdc, hOldFont);
    ReleaseDC(NULL, hdc);

    if ( createdFont )
    {
        DeleteObject(dlgFont);
    }
    return true;
}



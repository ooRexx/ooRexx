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
// OrxWin message handler declarations

/* function prototypes ---------------------------------------------*/
extern int StartRexx(int argc, char **argv);
int ParseWinArgs(char *cInParms);

BOOL OrxWin_Register(HINSTANCE hInstance);

LRESULT CALLBACK OrxWin_WndProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

VOID OrxWin_OnCommand(HWND hwnd, int id, HWND hwndCtl, WORD codeNotify);
VOID OrxWin_OnPaint(HWND hwnd);
VOID OrxWin_OnClose(HWND hwnd);
BOOL OrxWin_OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct);
VOID OrxWin_OnDestroy(HWND hwnd);
VOID OrxWin_OnSize(HWND hwnd, WORD state, int cx, int cy);
//VOID OrxWin_OnGetMinMaxInfo(HWND hwnd, MINMAXINFO FAR* lpMinMaxInfo);
//VOID OrxWin_OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, WORD flags);
//DWORD OrxWin_OnMenuChar(HWND hwnd, WORD ch, WORD flags, HMENU hmenu);
//VOID OrxWin_OnEnterIdle(HWND hwnd, WORD source, HWND hwndSource);
VOID OrxWin_OnDestroy(HWND hwnd);
//VOID OrxWin_OnKeyUp(HWND hwnd, WORD vk, BOOL fDown, int cRepeat, WORD flags);
VOID OrxWin_OnKeyDown(HWND hwnd, WORD vk, BOOL fDown, int cRepeat, WORD flags);
VOID OrxWin_OnSetFocus(HWND hwnd, HWND hwndOldFocus);
VOID OrxWin_OnVscroll(HWND hWnd, HWND hWndCtl, UINT code, int pos );


VOID PaintOrxWindow(HWND hWnd, HDC hdc, PAINTSTRUCT ps);
/* The following structure contains state about a window, and a
        pointer to it is stored as instance data within Windows. It can be
        retrieved using GetWindowLong(4) */

#include "winlist.h"           // list methods
typedef struct {
        int     xChar,         // horizontal scrolling unit
                yChar,         // vertical scrolling unit

                xPos,          // current horiz. scrolling position
                yPos,          // current vertical scrolling position

                CaretPosX,     // current horiz. caret position

                xMax,          // Maximum horiz. scrolling position
                yMax,          // Maximum vertical scrolling position

                xClient,       // client area width
                yClient;       // client aread height

        long    cLines;        // #lines of output
        short   iHitEnter;     // Enter key hit to end input text

        LIST    liDisplay;     // pointer to Display Linked List
        } ORXWDATA,  * LPORXWDATA;

/*----------------------------------------------------------------------------*/;
/*                                                                            */;
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */;
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

#ifndef oodMouse_Included
#define oodMouse_Included

#define TRACK_MOUSE_KEYWORDS    "CANCEL, HOVER, LEAVE, NONCLIENT, or QUERY"
#define WM_MOUSE_KEYWORDS       "MouseMove, MouseWheel, MouseLeave, MouseHover, NcMouseLeave, NcMouseHover, lButtonUp, lButtonDown, or CaptureChanged"
#define MOUSE_BUTTON_KEYWORDS   "LEFT, RIGHT, MIDDLE, XBUTTON1, or XBUTTON2"
#define SYSTEM_CURSOR_KEYWORDS  "APPSTARTING, ARROW, CROSS, HAND, HELP, IBEAM, NO, SIZEALL, SIZENESW, SIZENS, " \
                                "SIZENWSE, SIZEWE, UPARROW, or WAIT"

#define DLG_HAS_ENDED_MSG       "windows dialog has executed and been closed"


// Struct for instantiating a new Rexx Mouse object.
typedef struct newMouseParams
{
    pCPlainBaseDialog  dlgCSelf;      // Pointer to dialog owner CSelf struct, if owner is a dialog window
    pCDialogControl    controlCSelf;  // Pointer to dialog control owner CSelf struct, if owner is a dialog control window
    bool               isDlgWindow;   // True if owner window is a dialog, false if owner window is a dialog control
} NEWMOUSEPARAMS;
typedef NEWMOUSEPARAMS *PNEWMOUSEPARAMS;

// Struct for mouse wheel notify processing.
typedef struct {
    pCPlainBaseDialog    pcpbd;           // The owner dialog CSelf.
    RexxObjectPtr        mouse;           // The Rexx mouse object
    HWND                 hwnd;            // Window handle of window receiving WM_MOUSEWHEEL.
    char                *method;          // Name of method to invoke.
    uint32_t             tag;             // The internal ooDialog event message tag.
    bool                 isControlMouse;  // True if a dialog control mouse processing, false if a dialog mouse processing.
    bool                 willReply;       // User wants event handler invoked directly, or not.
} MOUSEWHEELDATA;
typedef MOUSEWHEELDATA *PMOUSEWHEELDATA;


extern MsgReplyType    processMouseMsg(RexxThreadContext *c, char *methodName, uint32_t tag, uint32_t msg, WPARAM wParam,
                                       LPARAM lParam, pCPlainBaseDialog pcpbd);
extern LRESULT         processMouseMsg(RexxThreadContext *c, char *methodName, uint32_t tag, uint32_t msg, HWND hwnd,
                                       WPARAM wParam, LPARAM lParam, pCDialogControl pcdc);

#endif

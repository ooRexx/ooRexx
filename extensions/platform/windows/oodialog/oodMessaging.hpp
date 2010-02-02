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

#ifndef oodMessaging_Included
#define oodMessaging_Included


// Enum for errors during key press processing
typedef enum
{
    noErr         = 0,
    nameErr       = 1,  // The method name was too long, or the empty string.
    winAPIErr     = 2,  // A Windows API failure.
    memoryErr     = 5,  // Memory allocation error.
    maxMethodsErr = 6,  // No room left in the method table.
    dupMethodErr  = 7,  // The method name provided already exists in the table, so nothing was done.
    badFilterErr  = 8,  // The filter provided was not a filter, so it was ignored.
    keyMapErr     = 9   // Some or all of the keys did not get mapped.
} keyPressErr_t;

extern LRESULT CALLBACK RexxDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

extern LRESULT       paletteMessage(DIALOGADMIN *, HWND, UINT, WPARAM, LPARAM);
extern MsgReplyType  searchMessageTables(ULONG message, WPARAM param, LPARAM lparam, pCPlainBaseDialog);
extern bool          initCommandMessagesTable(RexxMethodContext *c, pCEventNotification pcen);
extern bool          initEventNotification(RexxMethodContext *, DIALOGADMIN *, RexxObjectPtr, pCEventNotification *);
extern char         *getDlgMessage(DIALOGADMIN *dlgAdm, char *buffer, bool peek);
extern BOOL          addDialogMessage(CHAR * msg, CHAR * Qptr);
extern bool          addCommandMessage(pCEventNotification, WPARAM, ULONG_PTR, LPARAM, ULONG_PTR, CSTRING, uint32_t);
extern bool          addNotifyMessage(pCEventNotification, WPARAM, ULONG_PTR, LPARAM, ULONG_PTR, CSTRING, uint32_t);
extern bool          addMiscMessage(pCEventNotification, uint32_t, uint32_t, WPARAM, ULONG_PTR, LPARAM, ULONG_PTR, CSTRING, uint32_t);

// Shared functions for keyboard hooks and key press subclassing.
extern void          removeKBHook(DIALOGADMIN *dlgAdm);
extern keyPressErr_t setKeyPressData(KEYPRESSDATA *, CSTRING, CSTRING, CSTRING);
extern void          processKeyPress(KEYPRESSDATA *, WPARAM, LPARAM, PCHAR);
extern void          freeKeyPressData(KEYPRESSDATA *);
extern uint32_t      seekKeyPressMethod(KEYPRESSDATA *, CSTRING);
extern void          removeKeyPressMethod(KEYPRESSDATA *, uint32_t);


#endif

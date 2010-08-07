/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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

#ifndef oodPropertySheetDialog_Included
#define oodPropertySheetDialog_Included


// PSN_* notification method names
#define TRANSLATEACCELERATOR_MSG    "translateAccelerator"
#define SETACTIVE_MSG               "setActive"
#define WIZBACK_MSG                 "wizBack"
#define WIZNEXT_MSG                 "wizNext"
#define WIZFINISH_MSG               "wizFinish"
#define KILLACTIVE_MSG              "killActive"
#define RESET_MSG                   "reset"
#define QUERYCANCEL_MSG             "queryCancel"
#define QUERYINITIALFOCUS_MSG       "queryInitialFocus"
#define APPLY_MSG                   "apply"
#define GETOBJECT_MSG               "getObject"
#define HELP_MSG                    "help"

// PSM_* message methods names.  Really, it is likely there will only be the one.
#define QUERYFROMSIBLING_MSG        "queryFromSibling"

// Property Sheet Page callback method names
#define PAGECREATE_MSG              "pageCreate"


#define MAX_PROPSHEET_DIALOGS        5

#define TOO_MANY_PROPSHEET_DIALOGS \
        "the concurrent creation of property sheet dialogs has reached the maximum (%d)"

#define TOO_MANY_PROPSHEET_PAGES   \
        "the number of property sheet pages has reached the operating system maximum (%d)"


#define PROPSHEET_PAGE_ALREADY_ACTIVATED \
        "can not be a property sheet page dialog that has already been activated and destroyed"

// Enum for parts of a property sheet page.
typedef enum
{
    pageText, headerText, headerSubtext
} pagePart_t;

// Struct used to pass information to a CBTHook set up for a property sheet.
typedef struct {
    pCPropertySheetDialog   pcpsd;           // CSelf struct for the property sheet dialog.
    HHOOK                   hHook;           // Hook handle.
    HWND                    hPropSheet;      // Window handle of property sheet when discovered
    uint32_t                threadID;        // Thread ID used to match the struct.
} PROPSHEETHOOKDATA;

extern PROPSHEETHOOKDATA *PropSheetHookData[];
extern size_t             CountPropSheetHooks;

extern bool setPropSheetHook(pCPropertySheetDialog);

extern void assignPSDThreadContext(pCPropertySheetDialog pcpsd, RexxThreadContext *c);

extern LRESULT CALLBACK RexxPropertySheetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK PropSheetCBTProc(int nCode, WPARAM wParam, LPARAM lParam);

#endif

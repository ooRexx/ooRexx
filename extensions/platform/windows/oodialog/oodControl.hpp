/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2012 Rexx Language Association. All rights reserved.    */
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

#ifndef oodControl_Included
#define oodControl_Included

/**
 * Many of the Windows controls allow the user to store a user defined value
 * either with the control, or with each item of a control.  A good example is
 * the list view control where the programmer can store / associate a value with
 * each item in the list view.
 *
 * In ooDialog, where a dialog control object allows the Rexx programmer to take
 * advantage of the Windows function each user value is put in the dialog
 * control object's bag to prevent garbage collection until the dialog control
 * object itself is garbage collected.
 */
#define DIALOGCONTROL_BAG_ATTRIBUTE  "DialogControlBagAttribute"

/**
 *  A 'tag' is used in processing the mapping of Windows messages to user
 *  defined methods.  It allows the user mapping to dictate different processing
 *  of a Windows message based on the tag.
 *
 *  These tags are based on the dialog tags, but are used solely by the dialog
 *  control subclassing functionality.  The tags are a work in progress, it is
 *  possible that they may end up not being needed.
 *
 *  The least significant byte is used to segrate groups of messages.  This byte
 *  can be isolated using CTRLTAG_MASK.
 */
#define CTRLTAG_MASK              0x000000FF

#define CTRLTAG_NOTHING           0x00000000
#define CTRLTAG_CONTROL           0x00000001
#define CTRLTAG_MOUSE             0x00000002
#define CTRLTAG_DIALOG            0x00000003
#define CTRLTAG_EDIT              0x00000004

/**
 * The next 2 bytes are generic 'flags' that can be isolated using TAG_FLAGMASK.
 * The individual flags are not necessarily unique, but rather are unique when
 * combined with a specific CTRLTAG least significant byte.  For instance, if
 * the least significant byte is CTRLTAG_DIALOG, it could have a flag value that
 * is the same as a flag value that is used for CTRLTAG_MOUSE and have a
 * completely different meaning.
 */
#define CTRLTAG_FLAGMASK          0x00FFFF00

#define CTRLTAG_ISOLATE           0x00000100

/**
 * The last byte is for, well 'extra' information.  Use TAG_EXTRAMASK to
 * isolate the byte.
 */
#define CTRLTAG_EXTRAMASK                0xFF000000

// Return TRUE in the subclass procedure.  Do not pass on to DefSubclassProc(),
// do not send message to the dialog.
#define CTRLTAG_REPLYTRUE                0x01000000

// Return FALSE in the subclass procedure.  Do not pass on to DefSubclassProc(),
// do not send message to the dialog.  This actually should have the same effect
// as CTRLTAG_REPLYZERO, but in some Rexx methods, a keyword of ReplyFalse makes
// more sense than ReplyZero.
#define CTRLTAG_REPLYFALSE               0x02000000

// Return 0 in the subclass procedure.  Do not pass on to DefSubclassProc(), do
// not send message to the dialog.
#define CTRLTAG_REPLYZERO                0x04000000

// Send the message to the dialog.  I.e.:
//   return SendMessage((hDlg, msg, wParam, lParam);
#define CTRLTAG_SENDTODLG                0x08000000

// Pass the message on to DefSubclassProc().  I.e.:
//   return DefSubclassProc(hwnd, msg, wParam, lParam);
#define CTRLTAG_SENDTOCONTROL            0x10000000

// Send the message to the default window procedure.  I.e.:
//   return DefWindowProc(hwnd, msg, wParam, lParam);
#define CTRLTAG_SENDTODEFWINDOWPROC      0x20000000

// Wait in the subclass procedure for the return from invoking the Rexx method.
// I.e., use invokeDirect() rather than invokeDispatch.
#define CTRLTAG_REPLYFROMREXX            0x40000000

// The defualt size for the control's subclass message table.
#define DEF_CONTROL_MSGS          10

#define SUBCLASS_TAG_KEYWORDS      "SendToDlg, SendToControl, ReplyTrue, ReplyZero, or NoWait"
#define USERSUBCLASS_TAG_KEYWORDS  "A string in conventional hexidecimal, SendToDlg, SendToControl, ReplyTrue, ReplyZero, or NoWait"

typedef struct newControlParams
{
    pCPlainBaseDialog   pcpbd;           // Rexx parent dialog CSelf.
    HWND                hwnd;            // Window handle of the control
    oodControl_t        controlType;
    uint32_t            id;
    bool                isCatDlg;
} NEWCONTROLPARAMS;
typedef NEWCONTROLPARAMS *PNEWCONTROLPARAMS;

/* Struct for the LvFullRow object CSelf. */
typedef struct _lvFullRow
{
    uint32_t          magic;         // Indentifies this struct
    LPLVITEM         *subItems;      // Subitem[0] is actually the item the rest are the subitems
    RexxObjectPtr    *rxSubItems;    // The Rexx subitems rxSubItems[0] is a LvItem, the rest LvSubItems
    RexxObjectPtr     rexxSelf;      // The LvFullRow Rexx object
    uint32_t          subItemCount;  // The number of subItems
    uint32_t          size;          // The allocated size of the subItem array.
} CLvFullRow;
typedef CLvFullRow *pCLvFullRow;

#define LVFULLROW_MAGIC              0xCafeDeaf  // Magic number to identify a CLvFullRow struct
#define LVFULLROW_DEF_SUBITEMS       10          // Initial size of the subItems array

enum DateTimePart {dtFull, dtTime, dtDate, dtNow};

typedef void (*pfnFreeSubclassData)(pSubClassData pSCData);

// Defined in oodUser.cpp
extern uint32_t      listViewStyle(CSTRING opts, uint32_t style);
extern uint32_t      monthCalendarStyle(CSTRING opts, uint32_t style);
extern bool          parseTagOpts(RexxThreadContext *c, CSTRING opts, uint32_t *pTag, size_t argPos);

extern RexxClassObject    oodClass4controlType(RexxMethodContext *c, oodControl_t controlType);
extern RexxClassObject    oodClass4controlType(oodControl_t controlType, RexxMethodContext *c);
extern RexxClassObject    oodClass4controlType(RexxThreadContext *c, oodControl_t controlType);
extern oodControl_t       control2controlType(HWND hControl);
extern oodControl_t       winName2controlType(const char *className);
extern const char        *controlType2winName(oodControl_t control);
extern RexxStringObject   controlWindow2rexxString(RexxMethodContext *c, HWND hControl);
extern oodControl_t       oodName2controlType(CSTRING name);
extern bool               isControlMatch(HWND, oodControl_t);

extern RexxStringObject   cbLbGetText(RexxMethodContext *c, HWND hCtrl, uint32_t index, oodControl_t ctrl);
extern void               sysTime2dt(RexxThreadContext *c, SYSTEMTIME *sysTime, RexxObjectPtr *dateTime, DateTimePart part);
extern bool               dt2sysTime(RexxThreadContext *c, RexxObjectPtr dateTime, SYSTEMTIME *sysTime, DateTimePart part);
extern RexxStringObject   objectStateToString(RexxMethodContext *c, uint32_t state);
extern RexxObjectPtr      createRexxControl(RexxThreadContext *, HWND, HWND, uint32_t, oodControl_t, RexxObjectPtr, RexxClassObject, bool, bool);
extern RexxObjectPtr      createControlFromHwnd(RexxMethodContext *, pCDialogControl, HWND, oodControl_t, bool);
extern RexxObjectPtr      createControlFromHwnd(RexxMethodContext *, pCPlainBaseDialog, HWND, oodControl_t, bool);
extern RexxObjectPtr      createControlFromHwnd(RexxThreadContext *, pCPlainBaseDialog, HWND, oodControl_t, bool);
extern bool               addSubclassMessage(RexxMethodContext *c, pCDialogControl pcdc, pWinMessageFilter pwmf);

#define ButtonAtom           0x0080
#define EditAtom             0x0081
#define StaticAtom           0x0082
#define ListBoxAtom          0x0083
#define ScrollBarAtom        0x0084
#define ComboBoxAtom         0x0085

#define BS_IMAGEMASK         0x000000c0

/* Determine if an edit control is a single line edit control.  */
inline bool isSingleLineEdit(HWND hEdit)
{
    return ((GetWindowLong(hEdit, GWL_STYLE) & ES_MULTILINE) == 0);
}

/* Determine if a combo box is a drop down list combo box.  */
inline bool isDropDownList(HWND hDlg, uint32_t id)
{
    return ((GetWindowLong(GetDlgItem(hDlg, id), GWL_STYLE) & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST);
}

/* Determine if a list box is a single selection list box.  */
inline bool isSingleSelectionListBox(HWND hListBox)
{
    return ((GetWindowLong(hListBox, GWL_STYLE) & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) == 0);
}
inline bool isSingleSelectionListBox(HWND hDlg, uint32_t id)
{
    return isSingleSelectionListBox(GetDlgItem(hDlg, id));
}

/* Determine if a date time picker (DTP) control is a show none control.  */
inline bool isShowNoneDTP(HWND hDTP)
{
    return ((GetWindowLong(hDTP, GWL_STYLE) & DTS_SHOWNONE) == DTS_SHOWNONE);
}

/* Retrieve specific fields / information from the CDialogControl struct. */
inline HWND getDChCtrl(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->hCtrl);
}
inline RexxObjectPtr getDCrexxSelf(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->rexxSelf);
}
inline HWND getDChDlg(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->hDlg);
}
inline RexxObjectPtr getDCownerDlg(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->oDlg);
}
inline uint32_t getDCinsertIndex(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->lastItem + 1);
}

/**
 * Validates that the CSelf pointer for a DialogControl object is not null.
 */
inline pCDialogControl validateDCCSelf(RexxMethodContext *c, void *pcdc)
{
    oodResetSysErrCode(c->threadContext);
    if ( pcdc == NULL )
    {
        baseClassIntializationException(c);
    }
    return (pCDialogControl)pcdc;
}



#endif

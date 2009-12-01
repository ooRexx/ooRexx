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

#ifndef oodControl_Included
#define oodControl_Included


typedef struct newControlParams {
   HWND           hwnd;
   uint32_t       id;
   HWND           hwndDlg;
   RexxObjectPtr  parentDlg;
} NEWCONTROLPARAMS;
typedef NEWCONTROLPARAMS *PNEWCONTROLPARAMS;

// Defined in oodUser.cpp
extern uint32_t      listViewStyle(CSTRING opts, uint32_t style);

extern RexxClassObject oodClass4controlType(RexxMethodContext *c, oodControl_t controlType);
extern oodControl_t    control2controlType(HWND hControl);
extern oodControl_t    winName2controlType(const char *className);
extern const char     *controlType2winName(oodControl_t control);
extern oodControl_t    oodName2controlType(CSTRING name);
extern bool            isControlMatch(HWND, oodControl_t);

extern RexxStringObject cbLbGetText(RexxMethodContext *c, HWND hCtrl, uint32_t index, oodControl_t ctrl);
extern DIALOGADMIN *getDCDlgAdm(RexxMethodContext *c, pCDialogControl pcdc);

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

/* Retrieve specific fields / information from the CDialogControl struct. */
inline HWND getDCHCtrl(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->hCtrl);
}
inline HWND getDCHDlg(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->hDlg);
}
inline RexxObjectPtr getDCOwnerDlg(void *pCSelf)
{
    return (((pCDialogControl)pCSelf)->oDlg);
}



#endif

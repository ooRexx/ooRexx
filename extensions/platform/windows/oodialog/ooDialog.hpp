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

#ifndef oodDialog_Included
#define oodDialog_Included

#define NTDDI_VERSION   NTDDI_WINXPSP2
#define _WIN32_WINNT    0x0501
#define WINVER          0x0501

#define STRICT
#define OEMRESOURCE

#include <windows.h>
#include "oorexxapi.h"

#define MAXREXXNAME        128
#define MAXLENQUEUE       2056
#define NR_BUFFER           15
#define STR_BUFFER         256
#define LONGSTR_BUFFER    1024
#define DATA_BUFFER       8192
#define MAX_P               64
#define MAX_MT_ENTRIES     500
#define MAX_BT_ENTRIES     300
#define MAX_DT_ENTRIES     750
#define MAX_CT_ENTRIES    1000
#define MAX_IT_ENTRIES      20
#define MAXCHILDDIALOGS     20
#define MAXDIALOGS          20

/* User defined window messages used for RexxDlgProc() */
#define WM_USER_CREATECHILD         WM_USER + 0x0601
#define WM_USER_INTERRUPTSCROLL     WM_USER + 0x0602
#define WM_USER_GETFOCUS            WM_USER + 0x0603
#define WM_USER_GETSETCAPTURE       WM_USER + 0x0604
#define WM_USER_GETKEYSTATE         WM_USER + 0x0605
#define WM_USER_SUBCLASS            WM_USER + 0x0606
#define WM_USER_SUBCLASS_REMOVE     WM_USER + 0x0607
#define WM_USER_HOOK                WM_USER + 0x0608
#define WM_USER_CONTEXT_MENU        WM_USER + 0x0609

#define OODDLL                      "oodialog.dll"
#define DLLVER                      2130

#define MSG_TERMINATE               "1DLGDELETED1"

/* Flags for the get icon functions.  Indicates the source of the icon. */
#define ICON_FILE                 0x00000001
#define ICON_OODIALOG             0x00000002
#define ICON_DLL                  0x00000004

/* Defines for the different possible versions of comctl32.dll up to Windows
 * XP SP2. These DWORD "packed version" numbers are calculated using the
 * following macro:
 */
#define MAKEVERSION(major,minor) MAKELONG(minor,major)

#define COMCTL32_4_0         262144
#define COMCTL32_4_7         262151
#define COMCTL32_4_71        262215
#define COMCTL32_4_72        262216
#define COMCTL32_5_8         327688
#define COMCTL32_5_81        327761
#define COMCTL32_6_0         393216

/* The version of comctl32.dll in use when oodialog.dll is loaded. */
extern DWORD ComCtl32Version;

/**
 *  A 'tag' is used in processing the mapping of Windows messages to user
 *  defined methods.  It allows the user mapping to dictate different processing
 *  of a Windows message based on the tag.
 *
 *  The least significant 2 bytes are used to define the type of control.  These
 *  bytes can be isolated using TAG_CTRLMASK.
 */
#define TAG_DIALOG                0x00000001
#define TAG_HELP                  0x00000100
#define TAG_CONTEXTMENU           0x00000200
#define TAG_MENUCOMMAND           0x00000400
#define TAG_SYSMENUCOMMAND        0x00000800
#define TAG_MENUMESSAGE           0x00001000

// Reply TRUE in dialog procedure, not FALSE.  Reply FALSE passes message on to
// the system for processing.  TRUE indicates the message was handled.
#define TAG_MSGHANDLED            0x01000000

#define TAG_BUTTON                0x00000004
#define TAG_TREEVIEW              0x00000006
#define TAG_LISTVIEW              0x00000007
#define TAG_TRACKBAR              0x00000008
#define TAG_TAB                   0x00000009

#define TAG_CTRLMASK              0x000000FF
#define TAG_FLAGMASK              0x00FFFF00
#define TAG_EXTRAMASK             0xFF000000

#define TAG_STATECHANGED          0x00000100
#define TAG_CHECKBOXCHANGED       0x00000200
#define TAG_SELECTCHANGED         0x00000400
#define TAG_FOCUSCHANGED          0x00000800

// Describes how a message searched for in the message table should be handled
// by RexxDlgProc() after the search.
typedef enum
{
    NotMatched    = 0,    // Message not matched.
    ReplyFalse    = 1,    // Message matched and handled return FALSE to the system
    ReplyTrue     = 2,    // Message matched and handled return TRUE to the system
} MsgReplyType;


// Enum for the type of Windows dialog control.
typedef enum
{
    winStatic              =  1,
    winPushButton          =  2,
    winCheckBox            =  3,
    winRadioButton         =  4,
    winGroupBox            =  5,
    winEdit                =  6,
    winComboBox            =  7,
    winListBox             =  8,
    winScrollBar           =  9,
    winTreeView            = 10,
    winListView            = 11,
    winTrackBar            = 12,
    winProgressBar         = 13,
    winTab                 = 14,
    winDateTimePicker      = 15,
    winMonthCalendar       = 16,

    // A special value used by the data table / data table connection functions.
    winNotAControl         = 42,

    winUnknown             = 55
} oodControl_t;


/* macros to check the number of arguments */
#define CHECKARG(argexpct) { \
   if (argc != argexpct) \
      return HandleError(retstr, "Wrong number of arguments"); }


#define CHECKARGL(argexpct) { \
   if (argc < argexpct) \
      return HandleError(retstr, "Too few arguments"); }


#define CHECKARGLH(argexpctl, argexpcth) { \
   if (argc < argexpctl) return HandleError(retstr, "Too few arguments"); \
   if (argc > argexpcth) return HandleError(retstr, "Too many arguments"); }


/* macros for a easier return code */
#define RETC(retcode) { \
                   retstr->strlength = 1;\
                   if (retcode) retstr->strptr[0] = '1'; else retstr->strptr[0] = '0'; \
                   retstr->strptr[1] = '\0'; \
                   return 0; \
                }

#define RETERR  { \
                   retstr->strlength = 1;\
                   retstr->strptr[0] = '1'; \
                   retstr->strptr[1] = '\0'; \
                   return 40; \
                }


#define RETVAL(retvalue)  { \
                   ltoa(retvalue, retstr->strptr, 10); \
                   retstr->strlength = strlen(retstr->strptr);\
                   return 0; \
                }


#define RETPTR(retvalue)  { \
                   pointer2string(retstr, (void *)retvalue); \
                   return 0; \
                }

#define RETHANDLE(retvalue)  { \
                   pointer2string(retstr, (void *)retvalue); \
                   return 0; \
                }

inline LONG_PTR setWindowPtr(HWND hwnd, int index, LONG_PTR newPtr)
{
#ifndef __REXX64__
#pragma warning(disable:4244)
#endif
    return SetWindowLongPtr(hwnd, index, newPtr);
#ifndef __REXX64__
#pragma warning(default:4244)
#endif
}

inline LONG_PTR getWindowPtr(HWND hwnd, int index)
{
    return GetWindowLongPtr(hwnd, index);
}

inline LONG_PTR setClassPtr(HWND hwnd, int index, LONG_PTR newPtr)
{
#ifndef __REXX64__
#pragma warning(disable:4244)
#endif
    return SetClassLongPtr(hwnd, index, newPtr);
#ifndef __REXX64__
#pragma warning(default:4244)
#endif
}

inline LONG_PTR getClassPtr(HWND hwnd, int index)
{
    return GetClassLongPtr(hwnd, index);
}

/* structures to manage the dialogs */
typedef struct {
   WPARAM    wParam;
   LPARAM    lParam;
   uint32_t  msg;
   uint32_t  filterM;
   ULONG_PTR filterP;
   ULONG_PTR filterL;
   uint32_t  tag;
   PCHAR     rexxProgram;
} MESSAGETABLEENTRY;

typedef struct {
    oodControl_t  type;
    uint32_t      id;
    uint32_t      category;
} DATATABLEENTRY;

typedef struct {
   uint32_t buttonID;
   HBITMAP  bitmapID;
   HBITMAP  bmpFocusID;
   HBITMAP  bmpSelectID;
   HBITMAP  bmpDisableID;
   int32_t  displaceX;
   int32_t  displaceY;
   uint32_t loaded;
   bool     frame;
} BITMAPTABLEENTRY;

typedef struct {
   bool isSysBrush;
   ULONG itemID;
   INT ColorBk;
   INT ColorFG;
   HBRUSH ColorBrush;
} COLORTABLEENTRY;

typedef struct {
   ULONG iconID;
   PCHAR fileName;
} ICONTABLEENTRY;

typedef struct {
    HMENU       hMenu;
    HWND        hWnd;
    UINT        flags;
    POINT       point;
    LPTPMPARAMS lptpm;
    DWORD       dwErr;
} TRACKPOP, *PTRACKPOP;

/* Stuff for key press subclassing and keyboard hooks */
#define MAX_KEYPRESS_METHODS  63
#define COUNT_KEYPRESS_KEYS   256
#define CCH_METHOD_NAME       197

#define KEY_REALEASE          0x80000000
#define KEY_WASDOWN           0x40000000
#define KEY_TOGGLED           0x00000001
#define ISDOWN                    0x8000

/* Microsoft does not define these, just has this note:
 *
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */
#define VK_0   0x30
#define VK_1   0x31
#define VK_2   0x32
#define VK_3   0x33
#define VK_4   0x34
#define VK_5   0x35
#define VK_6   0x36
#define VK_7   0x37
#define VK_8   0x38
#define VK_9   0x39

#define VK_A   0x41
#define VK_B   0x42
#define VK_C   0x43
#define VK_D   0x44
#define VK_E   0x45
#define VK_F   0x46
#define VK_G   0x47
#define VK_H   0x48
#define VK_I   0x49
#define VK_J   0x4A
#define VK_K   0x4B
#define VK_L   0x4C
#define VK_M   0x4D
#define VK_N   0x4E
#define VK_O   0x4F
#define VK_P   0x50
#define VK_Q   0x51
#define VK_R   0x52
#define VK_S   0x53
#define VK_T   0x54
#define VK_U   0x55
#define VK_V   0x56
#define VK_W   0x57
#define VK_X   0x58
#define VK_Y   0x59
#define VK_Z   0x5A

typedef struct {
    BOOL none;          /* If none, neither of shift, control, or alt can be pressed */
    BOOL shift;
    BOOL alt;
    BOOL control;
    BOOL and;           /* If 'and' is false, filter is 'or' */
} KEYFILTER, *PKEYFILTER;

typedef struct {
    BYTE       key[COUNT_KEYPRESS_KEYS];            /* Value of key[x] is index to pMethods[]   */
    UINT       usedMethods;                         /* Count of used slots in  pMethods[]       */
    UINT       topOfQ;                              /* Top of next free queue, 0 if empty       */
    PCHAR      pMethods[MAX_KEYPRESS_METHODS + 1];  /* Index 0 intentionally left empty         */
    KEYFILTER *pFilters[MAX_KEYPRESS_METHODS + 1];  /* If null, no filter                       */
    UINT       nextFreeQ[MAX_KEYPRESS_METHODS];     /* Used only if existing connection removed */
} KEYPRESSDATA;

typedef struct {
    UINT          uID;
    HWND          hCtrl;
    PCHAR         pMessageQueue;
    KEYPRESSDATA *pKeyPressData;
} SUBCLASSDATA;

typedef struct
{
   void              *previous;
   size_t             TableEntry;
   MESSAGETABLEENTRY *MsgTab;
   DATATABLEENTRY    *DataTab;
   BITMAPTABLEENTRY  *BmpTab;
   COLORTABLEENTRY   *ColorTab;
   ICONTABLEENTRY    *IconTab;
   size_t             MT_size;
   size_t             DT_size;
   size_t             BT_size;
   size_t             CT_size;
   size_t             IT_size;
   HWND               TheDlg;
   HWND               ChildDlg[MAXCHILDDIALOGS+1];
   HWND               AktChild;
   HINSTANCE          TheInstance;
   HANDLE             TheThread;
   BOOL               OnTheTop;
   ULONG              LeaveDialog;
   BOOL               Use3DControls;
   HPALETTE           ColorPalette;
   HICON              SysMenuIcon;
   HICON              TitleBarIcon;
   BOOL               SharedIcon;
   BOOL               DidChangeIcon;
   HHOOK              hHook;
   KEYPRESSDATA      *pKeyPressData;
   DWORD              threadID;
   WPARAM             StopScroll;
   CHAR              *pMessageQueue;
} DIALOGADMIN;


// All global variables are defined in oodPackageEntry.cpp
extern HINSTANCE           MyInstance;
extern DIALOGADMIN        *DialogTab[];
extern DIALOGADMIN        *topDlg;
extern INT                 StoredDialogs;
extern CRITICAL_SECTION    crit_sec;
extern DWORD               ComCtl32Version;
extern RexxObjectPtr       TheTrueObj;
extern RexxObjectPtr       TheFalseObj;
extern RexxObjectPtr       TheNilObj;
extern RexxObjectPtr       TheZeroObj;
extern RexxObjectPtr       TheTwoObj;
extern RexxObjectPtr       TheOneObj;
extern RexxObjectPtr       TheNegativeOneObj;
extern RexxDirectoryObject TheDotLocalObj;
extern RexxPointerObject   TheNullPtrObj;

extern RexxClassObject ThePlainBaseDialogClass;
extern RexxClassObject TheDynamicDialogClass;

extern HBRUSH searchForBrush(DIALOGADMIN *dlgAdm, size_t *index, uint32_t id);

inline DIALOGADMIN *seekDlgAdm(HWND hDlg)
{
    if (topDlg && ((topDlg->TheDlg == hDlg) || (topDlg->AktChild == hDlg)))
    {
        return topDlg;
    }

    register INT i = 0;
    while ( (i < StoredDialogs) && (DialogTab[i]->TheDlg != hDlg) && (DialogTab[i]->AktChild != hDlg) )
    {
        i++;
    }
    return i < StoredDialogs ? DialogTab[i] : NULL;
}

#endif

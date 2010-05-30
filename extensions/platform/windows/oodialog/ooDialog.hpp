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

#define NTDDI_VERSION   NTDDI_LONGHORN
#define _WIN32_WINNT    0x0600
#define WINVER          0x0501

#define STRICT
#define OEMRESOURCE

#include <windows.h>
#include "oorexxapi.h"

#define NR_BUFFER           15
#define DATA_BUFFER       8192
#define MAX_BT_ENTRIES     300
#define MAX_DT_ENTRIES     750
#define MAX_CT_ENTRIES    1000
#define MAX_IT_ENTRIES      20
#define MAXCHILDDIALOGS     20
#define MAXDIALOGS          20

#define DEFAULT_FONTNAME            "MS Shell Dlg"
#define DEFAULT_FONTSIZE            8
#define MAX_DEFAULT_FONTNAME        256

#define MAX_MT_ENTRIES     500
#define MAX_NOTIFY_MSGS    200
#define MAX_COMMAND_MSGS   200
#define MAX_MISC_MSGS      100

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
 *  The least significant byte is used to define the type of control.  This byte
 *  can be isolated using TAG_CTRLMASK.
 */
#define TAG_CTRLMASK              0x000000FF

#define TAG_NOTHING               0x00000000
#define TAG_DIALOG                0x00000001
#define TAG_BUTTON                0x00000004
#define TAG_TREEVIEW              0x00000006
#define TAG_LISTVIEW              0x00000007
#define TAG_TRACKBAR              0x00000008
#define TAG_TAB                   0x00000009
#define TAG_UPDOWN                0x0000000A
#define TAG_DATETIMEPICKER        0x0000000B
#define TAG_MONTHCALENDAR         0x0000000C

/**
 * The next 2 bytes are generic 'flags' that can be isolated using TAG_FLAGMASK.
 * The individual flags are not necessarily unique, but rather are unique when
 * combined with a specific CTRL byte.  For instance, the help and menu related
 * flags are only used with TAG_DIALOG.  So, it doesn't matter that TAG_HELP has
 * the same value as TAG_STATECHANGED.
 */
#define TAG_FLAGMASK              0x00FFFF00

#define TAG_HELP                  0x00000100
#define TAG_CONTEXTMENU           0x00000200
#define TAG_MENUCOMMAND           0x00000400
#define TAG_SYSMENUCOMMAND        0x00000800
#define TAG_MENUMESSAGE           0x00001000

#define TAG_STATECHANGED          0x00000100
#define TAG_CHECKBOXCHANGED       0x00000200
#define TAG_SELECTCHANGED         0x00000400
#define TAG_FOCUSCHANGED          0x00000800

/**
 * The last byte is for, well 'extra' information.  Use TAG_EXTRAMASK to
 * isolate the byte.
 */
#define TAG_EXTRAMASK             0xFF000000

// Reply TRUE in dialog procedure, not FALSE.  Reply FALSE passes message on to
// the system for processing.  TRUE indicates the message was handled.
#define TAG_MSGHANDLED            0x01000000

// The message reply comes from Rexx.  I.e., from the programmer.  The return
// will be a .Pointer, unwrap it and use it as the message reply.  (This is a
// first cut at this, may change.)
#define TAG_REPLYFROMREXX         0x02000000

// Describes how a message searched for in the message table should be handled.
typedef enum
{
    ContinueProcessing   = 0,    // Message not matched, continue in RexxDlgProc()
    ReplyFalse           = 1,    // Message matched and handled return FALSE to the system from RexxDlgProc()
    ReplyTrue            = 2,    // Message matched and handled return TRUE to the system from RexxDlgProc()
    ContinueSearching    = 3     // Continue searching message table before returning to RexxDlgProc()
} MsgReplyType;


// Identifies an error, that should never happen, discovered in RexxDlgProc().
// Used in endDialogPremature() to determine what message to display.
typedef enum
{
    NoPCPBDpased        = 0,    // pCPlainBaseDialog not passed in the WM_INITDIALOG message
    NoThreadAttach      = 1,    // Failed to attach the thread context.
    NoThreadContext     = 2,    // The thread context pointer is null.
    RexxConditionRaised = 3,    // The invocation of a Rexx event handler method raised a condition.
} DlgProcErrType;

#define NO_PCPBD_PASSED_MSG    "RexxDlgProc() ERROR in WM_INITDIALOG.  PlainBaseDialog\nCSELF is null.\n\n\tpcpdb=%p\n\thDlg=%p\n"
#define NO_THREAD_ATTACH_MSG   "RexxDlgProc() ERROR in WM_INITDIALOG.  Failed to attach\nthread context.\n\n\tpcpdb=%p\n\thDlg=%p\n"
#define NO_THREAD_CONTEXT_MSG  "RexxDlgProc() ERROR.  Thread context is null.\n\n\\tdlgProcContext=%p\n\thDlg=%pn"


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
    winUpDown              = 17,

    // A special value used by the data table / data table connection functions.
    winNotAControl         = 42,

    winUnknown             = 55
} oodControl_t;


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
   PCHAR      rexxMethod;
   WPARAM     wParam;
   ULONG_PTR  wpFilter;
   LPARAM     lParam;
   ULONG_PTR  lpfilter;
   uint32_t   msg;
   uint32_t   msgFilter;
   uint32_t   tag;
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

// Structure used for context menus
typedef struct {
    HMENU       hMenu;
    HWND        hWnd;
    UINT        flags;
    POINT       point;
    LPTPMPARAMS lptpm;
    DWORD       dwErr;
} TRACKPOP, *PTRACKPOP;

// A generic structure used for subclassing controls with the Windows
// subclassing helper functions and for the keyboard hook function.
typedef struct {
    RexxThreadContext *dlgProcContext;  /* Attached thread context of dialog.     */
    RexxObjectPtr      rexxDialog;      /* Rexx dialog matching thread context.   */
    void              *pData;           /* Pointer to subclass specific data.     */
    UINT               uID;             /* Resource ID of subclassed control.     */
    HWND               hCtrl;           /* Window handle of subclassed control.   */
} SUBCLASSDATA;

/* Stuff for key press subclassing and keyboard hooks */

#define MAX_KEYPRESS_METHODS  63
#define COUNT_KEYPRESS_KEYS   256
#define CCH_METHOD_NAME       197

typedef struct {
    BOOL none;          /* If none, neither of shift, control, or alt can be pressed */
    BOOL shift;
    BOOL alt;
    BOOL control;
    BOOL and;           /* If 'and' is false, filter is 'or' */
} KEYFILTER, *PKEYFILTER;

typedef struct {
    BYTE               key[COUNT_KEYPRESS_KEYS];            /* Value of key[x] is index to pMethods[]   */
    UINT               usedMethods;                         /* Count of used slots in  pMethods[]       */
    UINT               topOfQ;                              /* Top of next free queue, 0 if empty       */
    PCHAR              pMethods[MAX_KEYPRESS_METHODS + 1];  /* Index 0 intentionally left empty         */
    KEYFILTER         *pFilters[MAX_KEYPRESS_METHODS + 1];  /* If null, no filter                       */
    UINT               nextFreeQ[MAX_KEYPRESS_METHODS];     /* Used only if existing connection removed */
} KEYPRESSDATA;

// It is anticpated that the connectKeyEvent() method will be extended some time
// soon, so we have a KEYEVENTDATA struct even though it is not technically
// needed at this point.
typedef struct {
    char              *method;          /* Name of method to invoke. */
} KEYEVENTDATA;

#define KEY_RELEASE          0x80000000
#define KEY_WASDOWN           0x40000000
#define EXTENDED_KEY          0x01000000
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

/* Struct for the WindowBase object CSelf. */
typedef struct _wbCSelf {
    HWND              hwnd;
    RexxObjectPtr     rexxHwnd;
    RexxObjectPtr     rexxSelf;
    wholenumber_t     initCode;
    uint32_t          sizeX;
    uint32_t          sizeY;
    double            factorX;
    double            factorY;
} CWindowBase;
typedef CWindowBase *pCWindowBase;

/* Struct for the EventNotification object CSelf. */
typedef struct _enCSelf {
    MESSAGETABLEENTRY  *notifyMsgs;
    MESSAGETABLEENTRY  *commandMsgs;
    MESSAGETABLEENTRY  *miscMsgs;
    size_t              nmSize;
    size_t              cmSize;
    size_t              mmSize;
    HWND                hDlg;
    RexxObjectPtr       rexxSelf;
    HHOOK               hHook;
    SUBCLASSDATA       *pHookData;
} CEventNotification;
typedef CEventNotification *pCEventNotification;

// Struct for the PlainBaseDialog class CSelf.
typedef struct _pbdcCSelf {
    char         fontName[MAX_DEFAULT_FONTNAME];
    uint32_t     fontSize;

} CPlainBaseDialogClass;
typedef CPlainBaseDialogClass *pCPlainBaseDialogClass;

/* Struct for the WindowExtensions object CSelf. */
typedef struct _weCSelf {
    pCWindowBase   wndBase;
    HWND           hwnd;
    RexxObjectPtr  rexxSelf;
} CWindowExtensions;
typedef CWindowExtensions *pCWindowExtensions;

/* Struct for the PlainBaseDialog object CSelf.  The struct itself is
 * allocated using interpreter memory and therefore garbage collected by the
 * interpreter.  But, there are still things like the table allocated externally
 * to the interpreter and require normal C/C++ memory management.  Also things
 * like brushes, bitmaps, etc., still need to be released.
 */
typedef struct _pbdCSelf {
    char                 fontName[MAX_DEFAULT_FONTNAME];
    void                *previous;      // Previous pCPlainBaseDialog used for stored dialogs
    size_t               tableIndex;    // Index of this dialog in the stored dialog table
    HWND                 activeChild;   // The active child dialog, used for CategoryDialogs
    HWND                 childDlg[MAXCHILDDIALOGS+1];
    HINSTANCE            hInstance;     // Handle to loaded DLL instance, ooDialog.dll or a resource DLL for a ResDialog
    HANDLE               hDlgProcThread;
    bool                 onTheTop;
    RexxInstance        *interpreter;
    RexxThreadContext   *dlgProcContext;
    HICON                sysMenuIcon;
    HICON                titleBarIcon;
    DWORD                threadID;
    pCWindowBase         wndBase;
    pCEventNotification  enCSelf;
    pCWindowExtensions   weCSelf;
    RexxObjectPtr        rexxSelf;
    HWND                 hDlg;
    DATATABLEENTRY      *DataTab;
    ICONTABLEENTRY      *IconTab;
    COLORTABLEENTRY     *ColorTab;
    BITMAPTABLEENTRY    *BmpTab;
    size_t               DT_size;
    size_t               IT_size;
    size_t               CT_size;
    size_t               BT_size;
    HBRUSH               bkgBrush;
    HBITMAP              bkgBitmap;
    WPARAM               stopScroll;
    HPALETTE             colorPalette;
    logical_t            autoDetect;
    uint32_t             fontSize;
    bool                 sharedIcon;
    bool                 didChangeIcon;
    bool                 isActive;
    bool                 dlgAllocated;
    bool                 abnormalHalt;
    bool                 scrollNow;   // For scrolling text in windows.
} CPlainBaseDialog;
typedef CPlainBaseDialog *pCPlainBaseDialog;

// Struct for the DialogControl object CSelf.
//
// Note that for a control in a category dialog page, the hDlg is the handle of
// the actual dialog the control resides in.  This is differnent than the dialog
// handle of the Rexx owner dialog.
typedef struct _dcCSelf {
    bool           isInCategoryDlg;
    uint32_t       id;
    oodControl_t   controlType;
    int            lastItem;
    pCWindowBase   wndBase;
    RexxObjectPtr  rexxSelf; // The Rexx dialog control object
    HWND           hCtrl;    // Handle of the dialog control
    RexxObjectPtr  oDlg;     // The Rexx owner dialog object
    HWND           hDlg;     // Handle of the dialog the control is in.
} CDialogControl;
typedef CDialogControl *pCDialogControl;

/* Struct for the DynamicDialog object CSelf. */
typedef struct _ddCSelf {
    pCPlainBaseDialog  pcpbd;
    RexxObjectPtr      rexxSelf;
    DLGTEMPLATE       *base;          // Base pointer to dialog template (basePtr)
    void              *active;        // Pointer to current location in dialog template (activePtr)
    void              *endOfTemplate; // Pointer to end of allocated memory for the template
    uint32_t           count;         // Dialog item count (dialogItemCount)
} CDynamicDialog;
typedef CDynamicDialog *pCDynamicDialog;


// All global variables are defined in oodPackageEntry.cpp
extern HINSTANCE           MyInstance;
extern pCPlainBaseDialog   DialogTable[];
extern pCPlainBaseDialog   TopDlg;
extern size_t              CountDialogs;
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
extern RexxClassObject TheDialogControlClass;
extern RexxClassObject TheSizeClass;

extern HBRUSH searchForBrush(pCPlainBaseDialog pcpbd, size_t *index, uint32_t id);

extern bool _isVersion(DWORD, DWORD, unsigned int, unsigned int, unsigned int);
extern bool _is32on64Bit(void);

inline bool _is64Bit(void)
{
#if defined(_WIN64)
    return true;
#else
    return false;
#endif
}

inline bool _isW2K(void)
{
    return _isVersion(5, 0, 0, 0, VER_EQUAL);
}

inline bool _isAtLeastW2K(void)
{
    return _isVersion(5, 0, 4, 0, VER_GREATER_EQUAL);
}

inline bool _isXP(void)
{
    return (_isVersion(5, 1, 0, 0, VER_EQUAL) || _isVersion(5, 2, 0, VER_NT_WORKSTATION, VER_EQUAL));
}

inline bool _isXP32(void)
{
    return _isVersion(5, 1, 0, 0, VER_EQUAL);
}

inline bool _isXP64(void)
{
    return _isVersion(5, 2, 0, VER_NT_WORKSTATION, VER_EQUAL);
}

inline bool _isAtLeastXP(void)
{
    return _isVersion(5, 1, 2, 0, VER_GREATER_EQUAL);
}

inline bool _isW2K3(void)
{
    return (_isVersion(5, 2, 0, VER_NT_DOMAIN_CONTROLLER, VER_EQUAL) ||
            _isVersion(5, 2, 0, VER_NT_SERVER, VER_EQUAL));
}

inline bool _isAtLeastW2K3(void)
{
    return (_isVersion(5, 2, 1, VER_NT_DOMAIN_CONTROLLER, VER_EQUAL) ||
            _isVersion(5, 2, 1, VER_NT_SERVER, VER_EQUAL));
}

inline bool _isVista(void)
{
    return _isVersion(6, 0, 0, VER_NT_WORKSTATION, VER_EQUAL);
}

inline bool _isServer2008(void)
{
    return (_isVersion(6, 0, 0, VER_NT_DOMAIN_CONTROLLER, VER_EQUAL) ||
            _isVersion(6, 0, 0, VER_NT_SERVER, VER_EQUAL));
}

inline bool _isAtLeastVista(void)
{
    return _isVersion(6, 0, 0, 0, VER_GREATER_EQUAL);
}

inline bool _isWindows7(void)
{
    return _isVersion(6, 1, 0, VER_NT_WORKSTATION, VER_EQUAL);
}

inline bool _isServer2008R2(void)
{
    return (_isVersion(6, 1, 0, VER_NT_DOMAIN_CONTROLLER, VER_EQUAL) ||
            _isVersion(6, 1, 0, VER_NT_SERVER, VER_EQUAL));
}

inline bool _isAtLeastWindows7(void)
{
    return _isVersion(6, 1, 0, 0, VER_GREATER_EQUAL);
}

#endif

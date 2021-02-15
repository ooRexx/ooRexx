/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
// #define _WIN32_IE       0x0600  <- old IE70 may be too much ?
#define _WIN32_IE       _WIN32_IE_IE70
#define WINVER          0x0600

#define STRICT
#define OEMRESOURCE

#include <windows.h>
#include <CommCtrl.h>
#include "oorexxapi.h"

#define NR_BUFFER               20
#define DATA_BUFFER           8192
#define MAXCHILDDIALOGS         20
#define MAXDIALOGS              20

#define MAXTABPAGES           MAXPROPPAGES
#define MAXMANAGEDTABS           4

#define MAXCUSTOMDRAWCONTROLS   20

#define DEFAULT_FONTNAME            "MS Shell Dlg"
#define DEFAULT_FONTSIZE              8
#define MAX_DEFAULT_FONTNAME        256
#define MAX_LIBRARYNAME             256

/*
 * The number of table entries is per dialog.  100 controls per dialog seems
 * like more than plenty.
 */
#define DEF_MAX_BT_ENTRIES           50
#define DEF_MAX_DT_ENTRIES          100
#define DEF_MAX_CT_ENTRIES          100
#define DEF_MAX_IT_ENTRIES           20
#define DEF_MAX_TTT_ENTRIES          20
#define DEF_MAX_NOTIFY_MSGS         100
#define DEF_MAX_COMMAND_MSGS        100
#define DEF_MAX_MISC_MSGS            50

/* User defined window messages used for RexxDlgProc() */
#define WM_USER_REXX_FIRST             WM_USER + 0x0601
#define WM_USER_CREATECHILD            WM_USER + 0x0601
#define WM_USER_INTERRUPTSCROLL        WM_USER + 0x0602
#define WM_USER_GETFOCUS               WM_USER + 0x0603
#define WM_USER_MOUSE_MISC             WM_USER + 0x0604
#define WM_USER_SUBCLASS               WM_USER + 0x0605
#define WM_USER_SUBCLASS_REMOVE        WM_USER + 0x0606
#define WM_USER_HOOK                   WM_USER + 0x0607
#define WM_USER_CONTEXT_MENU           WM_USER + 0x0608
#define WM_USER_CREATECONTROL_DLG      WM_USER + 0x0609
#define WM_USER_CREATECONTROL_RESDLG   WM_USER + 0x060A
#define WM_USER_CREATEPROPSHEET_DLG    WM_USER + 0x060B
#define WM_USER_CREATETOOLTIP          WM_USER + 0x060C
#define WM_USER_REXX_LAST              WM_USER + 0x060C

// Flags for WM_USER_MOUSE_MISC
#define MF_GETCAPTURE       0
#define MF_SETCAPTURE       1
#define MF_RELEASECAPTURE   2
#define MF_BUTTONDOWN       3
#define MF_SHOWCURSOR       4

#define OODDLL                      "oodialog.dll"

/* Flags for the get icon functions.  Indicates the source of the icon. */
#define ICON_FILE                 0x00000001
#define ICON_OODIALOG             0x00000002
#define ICON_DLL                  0x00000004
#define ICON_RC                   0x00000008

/* Defines for the different possible versions of comctl32.dll up to Windows 7.
 * These DWORD "packed version" numbers are calculated using the following
 * macro:
 */
#define MAKEVERSION(major,minor) MAKELONG(minor,major)

#define COMCTL32_4_0         262144
#define COMCTL32_4_7         262151
#define COMCTL32_4_71        262215
#define COMCTL32_4_72        262216
#define COMCTL32_5_8         327688
#define COMCTL32_5_81        327761
#define COMCTL32_6_0         393216
#define COMCTL32_6_10        393226    // Probably should use this as the latest version
#define COMCTL32_6_16        393232    // Latest version I've seen on an updated Windows 7


/**
 *  A 'tag' is used in processing the mapping of Windows messages to user
 *  defined methods.  It allows the user mapping to dictate different processing
 *  of a Windows message based on the tag.
 *
 *  The least significant byte is used to define the type of control.  This byte
 *  can be isolated using TAG_CTRLMASK. The value of the byte is not a 'flag' in
 *  the usual sense, but a constant value.  Thus, the control byte has 256
 *  unique values. The lowest values are used for dialog controls.  Over time,
 *  the higher values have beend added for uses unrelated to a dialog control.
 */
#define TAG_CTRLMASK              0x000000FF

#define TAG_NOTHING               0x00000000
#define TAG_DIALOG                0x00000001
#define TAG_MOUSE                 0x00000002
#define TAG_BUTTON                0x00000004
#define TAG_TREEVIEW              0x00000006
#define TAG_LISTVIEW              0x00000007
#define TAG_TRACKBAR              0x00000008
#define TAG_TAB                   0x00000009
#define TAG_UPDOWN                0x0000000A
#define TAG_DATETIMEPICKER        0x0000000B
#define TAG_MONTHCALENDAR         0x0000000C
#define TAG_TOOLTIP               0x0000000D
#define TAG_REBAR                 0x0000000E
#define TAG_MENU                  0x0000000F
#define TAG_TOOLBAR               0x00000010
#define TAG_STATUSBAR             0x00000011
#define TAG_USERADDED             0x000000FE
#define TAG_CUSTOMDRAW            0x000000FF

/**
 * The next 2 bytes are generic 'flags' that can be isolated using TAG_FLAGMASK.
 * The individual flags are not necessarily unique, but rather are unique when
 * combined with a specific CTRL byte.  For instance, the help and menu related
 * flags are only used with TAG_DIALOG.  So, it doesn't matter that TAG_HELP has
 * the same value as TAG_STATECHANGED.
 */
#define TAG_FLAGMASK              0x00FFFF00

#define TAG_HELP                  0x00000100

#define TAG_STATECHANGED          0x00000100
#define TAG_CHECKBOXCHANGED       0x00000200
#define TAG_SELECTCHANGED         0x00000400
#define TAG_FOCUSCHANGED          0x00000800

// When combined with CTRL flag of custom draw, specifies which type of control.
#define TAG_CD_HEADER             0x00000100
#define TAG_CD_LISTVIEW           0x00000200
#define TAG_CD_REBAR              0x00000400
#define TAG_CD_TOOLBAR            0x00000800
#define TAG_CD_TOOLTIP            0x00001000
#define TAG_CD_TRACKBAR           0x00002000
#define TAG_CD_TREEVIEW           0x00004000

// When combined with the CTRL flag of a dialog control, (nothing else,)
// indicates the event handler should be invoked exactly the same as in old
// ooDialog (pre 4.2.0.)
#define TAG_PRESERVE_OLD          0x00100000

// Indicates that, in genericInvoke(), if we have TAG_WILLREPLY we should use
// the return value from the event handler as a reply to the notification
// message.  This must be tested for as:
//
// if ( (TAG & TAG_USE_RETURN) == TAG_USE_RETURN )
#define TAG_USE_RETURN            0x00200000

/**
 * The last byte is for, well 'extra' information.  Use TAG_EXTRAMASK to
 * isolate the byte.
 */
#define TAG_EXTRAMASK             0xFF000000

// When compbined with TAG_CUSTOMDRAW and one of the TAG_CD_* flags means to use
// the simple form of custom draw.  What is 'simple' is defined by the control.
// For instance, with a list-view, ooDialog lets the user change the foreground,
// background colors, and the font on an item and sub-item basis, but nothing
// more.
#define TAG_CD_SIMPLE             0x01000000

// Reply TRUE in dialog procedure, not FALSE.  Reply FALSE passes message on to
// the system for processing.  TRUE indicates the message was handled.
#define TAG_MSGHANDLED            0x01000000

// The message reply comes from Rexx.  I.e., from the programmer.  The return
// is specific to the message, and may simply be ignored.  Many of the event
// connection methods now let the programmer specify if he wants the window
// message handling loop to wait for the reply, even when Windows ignores the
// reply.  When the programmer specifies to not wait, the event handler method
// is invoked through startWith(), which of course returns immediately.
#define TAG_REPLYFROMREXX         0x02000000

// Reply FALSE in the dialog procedure, not TRUE.  Reply FALSE passes message on
// to the system for processing.  TRUE indicates the message was handled.
// Previously, the default was always to reply FALSE, so only TAG_MSGHANDLED was
// needed.  However, now some event connections have the default to be reply
// TRUE and we need some way for the user to change that default.
#define TAG_REPLYFALSE            0x04000000

// Envoke the event handler directly, which means that the interprete will wait
// for the event handler to return, but do not enforce that a value is returned
// from the event handler. This is for backwards compatibility where it is
// expected that existing programs have event handlers but don't return values.
#define TAG_SYNC                  0x08000000

#define TAG_INVALID               0xFFFFFFFF

// Describes how a message searched for in the message table should be handled.
typedef enum
{
    ContinueProcessing   = 0,    // Message not matched, continue in RexxDlgProc()
    ReplyFalse           = 1,    // Message matched and handled return FALSE to the system from RexxDlgProc()
    ReplyTrue            = 2,    // Message matched and handled return TRUE to the system from RexxDlgProc()
    ContinueSearching    = 3     // Continue searching message table before returning to RexxDlgProc()
} MsgReplyType;


// Enum for the type of bitmap used for bitmap buttons
typedef enum
{
    InMemoryBmp    = 1,
    IntResourceBmp = 2,
    FromFileBmp    = 3,
} BitmapButtonBMPType;


// Identifies an error, that should never happen, discovered in RexxDlgProc(),
// or some other place trying to do a therad attach. Used in
// endDialogPremature() to determine what message to display.
typedef enum
{
    NoPCPBDpased         = 0,    // pCPlainBaseDialog not passed in the WM_INITDIALOG message
    NoPCPSPpased         = 1,    // pCPropertySheet not passed in the WM_INITDIALOG message
    NoThreadAttach       = 2,    // Failed to attach the thread context in RexxDlgProc().
    NoThreadAttachOther  = 3,    // Failed to attach the thread context some other place.
    NoThreadContext      = 4,    // The thread context pointer is null.
    RexxConditionRaised  = 5,    // The invocation of a Rexx event handler method raised a condition.
} DlgProcErrType;

#define NO_PCPBD_PASSED_MSG         "RexxDlgProc() ERROR in WM_INITDIALOG.  PlainBaseDialog\nCSELF is null.\n\n\tpcpdb=%p\n\thDlg=%p\n"
#define NO_PCPSP_PASSED_MSG         "RexxDlgProc() ERROR in WM_INITDIALOG.  PropertySheetPage\nCSELF is null.\n\n\tpcpsp=%p\n\thDlg=%p\n"
#define NO_THREAD_ATTACH_MSG        "RexxDlgProc() ERROR in WM_INITDIALOG.  Failed to attach\nthread context.\n\n\tpcpdb=%p\n\thDlg=%p\n"
#define NO_THREAD_ATTACH_OTHER_MSG  "Internal Windows ERROR.  Failed to attach\nthread context.\n\n\tpcpdb=%p\n\thDlg=%p\n"
#define NO_THREAD_CONTEXT_MSG       "RexxDlgProc() ERROR.  Thread context is null.\n\n\tdlgProcContext=%p\n\thDlg=%pn"


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
    winToolTip             = 18,
    winComboLBox           = 19,
    winReBar               = 20,
    winToolBar             = 21,
    winStatusBar           = 22,

    // A special value used by the data table / data table connection functions.
    winNotAControl         = 42,

    // A special value used to indicate a dialog instead of a dialog control
    winDialog              = 51,

    winUnknown             = 55
} oodControl_t;


// Enum for the type of an ooDialog class.  Types to be added as needed.
typedef enum
{
    oodPlainBaseDialog, oodCategoryDialog,    oodUserDialog,      oodRcDialog,         oodResDialog,
    oodControlDialog,   oodUserControlDialog, oodRcControlDialog, oodResControlDialog, oodUserPSPDialog,
    oodRcPSPDialog,     oodResPSPDialog,      oodDialogControl,   oodStaticControl,    oodButtonControl,
    oodEditControl,     oodListBox,           oodProgressBar,     oodToolBar,          oodToolTip,
    oodReBar,           oodUnknown
} oodClass_t;

// How the Global constDir is to be used
typedef enum {globalOnly, globalFirst, globalLast, globalNever} oodConstDir_t;

#define APPLICATION_MANAGER_MAGIC  0xdeadfeed

/* structures to manage the dialogs */
typedef struct {
   PCHAR      rexxMethod;
   WPARAM     wParam;
   ULONG_PTR  wpFilter;
   LPARAM     lParam;
   ULONG_PTR  lpFilter;
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
    HBRUSH   ColorBrush;
    COLORREF ColorBk;
    COLORREF ColorFG;
    uint32_t itemID;
    bool     isSysBrush;
    bool     useSysColors;
} COLORTABLEENTRY;

typedef struct {
    uint32_t  iconID;
    char     *fileName;
} ICONTABLEENTRY;


// We need to define this here instead of in oodCommon.hpp with the other tool
// tip stuff.
#define MAX_TOOLINFO_TEXT_LENGTH    1023

typedef struct {
    char           textBuf[MAX_TOOLINFO_TEXT_LENGTH + 1];
    LPWSTR         wcharBuf;
    RexxObjectPtr  rexxSelf;
    HWND           hToolTip;
    uint32_t       id;
} TOOLTIPTABLEENTRY, *PTOOLTIPTABLEENTRY;

typedef struct _createToolTip {
    HINSTANCE   hInstance;
    uint32_t    style;
    uint32_t    errRC;
} CREATETOOLTIP, *PCREATETOOLTIP;

// Structure used for context menus
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

typedef struct {
    BOOL none;          /* If none, neither of shift, control, or alt can be pressed */
    BOOL shift;
    BOOL alt;
    BOOL control;
    BOOL and;           /* If 'and' is false, filter is 'or' */
    BOOL virt;          /* If virtual use GetKeyState to see if control, alt, or shift is down. */
} KEYFILTER, *PKEYFILTER;

typedef struct {
    BYTE               key[COUNT_KEYPRESS_KEYS];            /* Value of key[x] is index to pMethods[]   */
    UINT               usedMethods;                         /* Count of used slots in  pMethods[]       */
    UINT               topOfQ;                              /* Top of next free queue, 0 if empty       */
    PCHAR              pMethods[MAX_KEYPRESS_METHODS + 1];  /* Index 0 intentionally left empty         */
    KEYFILTER         *pFilters[MAX_KEYPRESS_METHODS + 1];  /* If null, no filter                       */
    UINT               nextFreeQ[MAX_KEYPRESS_METHODS];     /* Used only if existing connection removed */
} KEYPRESSDATA;

// Masks for lParam of key messages.  WM_KEYDOWN, WM_CHAR, etc..
#define KEY_RELEASED          0x80000000  // Transition state: 1 key is being released / 0 key is being pressed.
#define KEY_WASDOWN           0x40000000  // Previous state: 1 key was down / 0 key was up.
#define KEY_ALTHELD           0x20000000  // Context code: 1 ALT key was held when key pressed / 0 otherwise.
#define KEY_ISEXTENDED        0x01000000  // Key is an extended key: 1 if an extended key / 0 otherwise

// Masks for GetAsyncKeyState() and GetKeyState() returns.
#define TOGGLED               0x00000001  // GetKeyState()
#define ISDOWN                    0x8000  // GetAsyncKeyState()
#define WASPRESSED                0x0001  // GetAsyncKeyState()

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

/* The extended dialog template struct.  Microsoft does not supply this in any
 * header, just provided the definition in the MSDN docs.
 */
typedef struct {
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
    WCHAR menu;         // Actually variable length array of 16-bit values
    WCHAR windowClass;  // Actually variable length array of 16-bit values
    WCHAR title;        // Actually variable length array of 16-bit values
    WORD pointsize;
    WORD weight;
    BYTE italic;
    BYTE charset;
    WCHAR typeface;     // Actually variable length array of 16-bit values
} DLGTEMPLATEEX;

typedef struct {
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    short x;
    short y;
    short cx;
    short cy;
    DWORD id;
    WORD  windowClass;
    WORD  title;
    WORD extraCount;
} DLGITEMTEMPLATEEX;


/* Struct for window message filtering. */
typedef struct _wmf {
    RexxObjectPtr  _wp;
    CSTRING        _wpFilter;
    RexxObjectPtr  _lp;
    CSTRING        _lpFilter;
    CSTRING        _wm;
    CSTRING        _wmFilter;
    WPARAM         wp;
    ULONG_PTR      wpFilter;
    LPARAM         lp;
    ULONG_PTR      lpFilter;
    CSTRING        method;
    void          *pData;
    void          *pfn;
    uint32_t       wm;
    uint32_t       wmFilter;
    uint32_t       tag;
} WinMessageFilter;
typedef WinMessageFilter *pWinMessageFilter;

/* Stuff for the ResizingAdmin class (resizable dialogs.) */

// How an edge of a control is pinned to the edge of another window.
typedef enum
{
    myLeftPin = 1,       // control width does not change
    myTopPin,            // control height does not change
    proportionalPin,     // control position is proportional to pinned edge
    stationaryPin,       // control position is unchanged from pinned edge
    notAPin              // invalid
} pinType_t;

// Which edge of another window the control's edge is pinned to.
typedef enum
{
    leftEdge = 1,  // pinned to the left edge
    topEdge,       // pinned to the top edge
    rightEdge,     // pinned to the right edge
    bottomEdge,    // pinned to the bottom edge
    xCenterEdge,   // centered horizontally ...
    yCenterEdge,   // centered vertically ...
    notAnEdge      // invalid
} pinnedEdge_t;

// Defines how one edge of a dialog control is pinned to some other window.
typedef struct _edge {
    pinnedEdge_t  pinToEdge;  // which edge of the pinned to window is pinned
    pinType_t     pinType;    // how this edge is pinned
    uint32_t      pinToID;    // the window this edge is pinned to
} Edge;
typedef Edge *pEdge;

// Defines how all 4 edges of a dialog control are pinned to other windows
typedef struct _controlEdges {
    Edge      left;
    Edge      top;
    Edge      right;
    Edge      bottom;
} ControlEdges;
typedef ControlEdges *pControlEdges;

/* Struct for the resizing information for a single control */
typedef struct _resizeInfoCtrl {
    ControlEdges       edges;
    RECT               originalRect;
    RECT               currentRect;
    HWND               hCtrl;
    oodControl_t       ctrlType;
    uint32_t           id;
} ResizeInfoCtrl;
typedef ResizeInfoCtrl *pResizeInfoCtrl;

// For control dialogs, a struct for information for each tab control in the
// dialog.
typedef struct _pagedTab {
    HWND      redrawThis;                // For control dialogs, we need a redrawThis for each tab control.
    uint32_t  tabID;
    uint32_t  dlgIDs[MAXCHILDDIALOGS];
} PagedTab;
typedef PagedTab *pPagedTab;

/* Struct for the resizable dialog information (ResizingAdmin.) */
typedef struct _resizeInfoDlg {
    ControlEdges       defEdges;
    pPagedTab          pagedTabs[MAXMANAGEDTABS];
    RECT               originalRect;
    RECT               currentRect;
    SIZE               minSize;
    SIZE               maxSize;
    pResizeInfoCtrl    riCtrls;
    HWND               redrawThis;     // Used for PropertySheetDialogs only
    char              *sizeEndedMeth;
    size_t             tableSize;      // should be ctrlTableSize
    size_t             countCtrls;
    size_t             countPagedTabs;
    bool               sizeEndedWillReply;
    bool               inDefineSizing;
    bool               inSizeOrMove;
    bool               isSizing;
    bool               haveError;
    bool               minSizeIsInitial;
    bool               haveMaxSize;
    bool               haveMinSize;
} ResizeInfoDlg;
typedef ResizeInfoDlg *pResizeInfoDlg;


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
    uint32_t            magic;
    MESSAGETABLEENTRY  *notifyMsgs;
    MESSAGETABLEENTRY  *commandMsgs;
    MESSAGETABLEENTRY  *miscMsgs;
    size_t              nmSize;        // Size of  table.
    size_t              nmNextIndex;   // Next free index in table.
    size_t              cmSize;
    size_t              cmNextIndex;
    size_t              mmSize;
    size_t              mmNextIndex;
    RexxObjectPtr       rexxSelf;      // This is the dialog Rexx self, NOT the EventNotification::self
    HWND                hDlg;
    HHOOK               hHook;
    void               *pHookData;
    void               *pCustomDraw;
    void               *pDlgCSelf;
} CEventNotification;
typedef CEventNotification *pCEventNotification;

#define EVENTNOTIFICATION_MAGIC    0x1ee1e11e

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

/* Struct for the CreateWindows object CSelf. */
typedef struct _cwCSelf {
    HINSTANCE      hinst;
    pCWindowBase   wndBase;
    HWND           hDlg;
    RexxObjectPtr  rexxDlg;
} CCreateWindows;
typedef CCreateWindows *pCCreateWindows;

/* Struct for the PlainBaseDialog object CSelf.  The struct itself is
 * allocated using interpreter memory and therefore garbage collected by the
 * interpreter.  But, there are still things like the table allocated externally
 * to the interpreter and require normal C/C++ memory management.  Also things
 * like brushes, bitmaps, etc., still need to be released.
 */
typedef struct _pbdCSelf {
    char                 fontName[MAX_DEFAULT_FONTNAME];
    char                 library[MAX_LIBRARYNAME];
    void                *previous;      // Previous pCPlainBaseDialog used for stored dialogs
    size_t               tableIndex;    // Index of this dialog in the stored dialog table
    HWND                 activeChild;   // The active child dialog, used for CategoryDialogs
    HWND                 childDlg[MAXCHILDDIALOGS + 1];
    HINSTANCE            hInstance;     // Handle to loaded DLL instance, ooDialog.dll or a resource DLL for a ResDialog
    HANDLE               hDlgProcThread;
    RexxInstance        *interpreter;
    RexxThreadContext   *dlgProcContext;
    RexxObjectPtr        resourceID;
    HICON                sysMenuIcon;
    HICON                titleBarIcon;
    pCWindowBase         wndBase;
    pCEventNotification  enCSelf;
    pCWindowExtensions   weCSelf;
    pResizeInfoDlg       resizeInfo;
    RexxObjectPtr        rexxSelf;      // This dialog's Rexx dialog object
    RexxObjectPtr        rexxParent;    // This dialog's Rexx parent dialog object
    HWND                 hDlg;          // The handle to this dialog's underlying Windows dialog
    RexxObjectPtr        rexxOwner;     // This dialog's Rexx owner dialog object
    HWND                 hOwnerDlg;     // Owner dialog window handle
    void                *ownerCSelf;    // Owner dialog CSelf.
    void                *dlgPrivate;    // Subclasses can store data unique to the subclass
    void                *initPrivate;   // Subclasses can store init data unique to the subclass
    void                *mouseCSelf;
    RexxObjectPtr        rexxMouse;
    HBRUSH               bkgBrush;
    HBITMAP              bkgBitmap;
    WPARAM               stopScroll;
    HPALETTE             colorPalette;
    DATATABLEENTRY      *DataTab;
    ICONTABLEENTRY      *IconTab;
    COLORTABLEENTRY     *ColorTab;
    BITMAPTABLEENTRY    *BmpTab;
    TOOLTIPTABLEENTRY   *ToolTipTab;
    size_t               DT_nextIndex;
    size_t               DT_size;
    size_t               IT_nextIndex;
    size_t               IT_size;
    size_t               TTT_nextIndex;
    size_t               TTT_size;
    size_t               CT_nextIndex;
    size_t               CT_size;
    size_t               BT_nextIndex;
    size_t               BT_size;
    size_t               countChilds;
    logical_t            autoDetect;
    uint32_t             dlgProcThreadID;
    uint32_t             fontSize;
    int32_t              dlgID;            // ID, usually set to resourceID if ResDlg or RcDlg, set to -1 if not resoloved during init()
    bool                 onTheTop;
    bool                 isCategoryDlg;    // Need to use IsNestedDialogMessage()
    bool                 isControlDlg;     // Dialog was created as DS_CONTROL | WS_CHILD
    bool                 isOwnedDlg;       // Dialog has an owner dialog
    bool                 isOwnerDlg;       // Dialog is an owner dialog
    bool                 isManagedDlg;     // Dialog has an owner dialog, which is a tab owner dialog
    bool                 isPageDlg;        // Dialog is a property sheet page dialog
    bool                 isPropSheetDlg;   // Dialog is a property sheet dialog
    bool                 isTabOwnerDlg;    // Dialog is a tab owner dialog
    bool                 isCustomDrawDlg;  // Dialog inherited CustomDraw
    bool                 isResizableDlg;   // Dialog inherited ResizingAdmin
    bool                 idsNotChecked;
    bool                 badIDs;
    bool                 isDlgHwndSet;     // Has setDlgHandle() been executed
    bool                 sharedIcon;
    bool                 didChangeIcon;
    bool                 isActive;
    bool                 isFinished;       // The value of the finished attribute
    bool                 dlgAllocated;
    bool                 abnormalHalt;
    bool                 scrollNow;        // For scrolling text in windows.
    bool                 bkgBrushIsSystem; // Do not delete brush if true.
} CPlainBaseDialog;
typedef CPlainBaseDialog *pCPlainBaseDialog;


// It is anticpated that the connectCharEvent() method will be extended some time
// soon, so we have a CHAREVENTDATA struct even though it is not technically
// needed at this point.
typedef struct {
    char              *method;          /* Name of method to invoke. */
} CHAREVENTDATA;

// Struct for sorting list-view or tree-view items when the sorting is done by
// invoking a method in the Rexx dialog.
typedef struct _lvRexxSort{
    pCPlainBaseDialog    pcpbd;            // The Rexx owner dialog CSelf
    RexxThreadContext   *threadContext;    // Thread context of the sort function
    RexxObjectPtr        rexxDlg;          // The Rexx dialog object
    RexxObjectPtr        rexxCtrl;         // The Rexx control object
    RexxObjectPtr        param;            // An optional parameter the Rexx programmer can have passed to his method.
    HTREEITEM            hItem;
    char                *method;           // Name of the comparsion method to invoke.
} CRexxSort;
typedef CRexxSort *pCRexxSort;

/**
 * Struct for the CustomDraw object CSelf.  This is in anticipation of future
 * enhancements to CustoDraw.  It is not really needed as yet.
 */
typedef struct _customdrawCSelf {
    uint32_t             magic;
    uint32_t             ids[MAXCUSTOMDRAWCONTROLS];   // List of all control IDs the user has set.
    oodControl_t         types[MAXCUSTOMDRAWCONTROLS]; // Matching type of control for each ID.
    pCEventNotification  enCSelf;
    RexxObjectPtr        rexxSelf;
    uint32_t             count;                        // Number of IDs in the ids array.
} CCustomDraw;
typedef CCustomDraw *pCCustomDraw;

#define CUSTOMDRAW_MAGIC    0x7cd7c77d


// Struct for the DialogControl object CSelf.
//
// Note that for a control in a category dialog page, the hDlg is the handle of
// the actual dialog the control resides in.  This is differnent than the dialog
// handle of the Rexx owner dialog.
typedef struct _dcCSelf {
    RexxObjectPtr       rexxSelf;        // The Rexx dialog control object
    HWND                hCtrl;           // Handle of the dialog control
    RexxObjectPtr       oDlg;            // The Rexx owner dialog object
    pCPlainBaseDialog   pcpbd;           // The Rexx owner dialog CSelf
    HWND                hDlg;            // Handle of the dialog the control is in.
    pCWindowBase        wndBase;
    void               *pscd;            // Pointer to general subclass data struct, usually null.
    void               *pKeyPress;       // Pointer to KeyPress subclass data struct, usually null.
    void               *pRelayEvent;     // Pointer to relay event (tool tips) subclass data struct, usually null.
    void               *mouseCSelf;      // Mouse CSelf struct
    pCRexxSort          pcrs;            // Pointer to Rexx sort struct used for sorting list view items, usually null.
    RexxObjectPtr       rexxMouse;       // Rexx mouse object if there is one.
    PTOOLTIPTABLEENTRY  toolTipEntry;    // The tool tip table entry for this dialog control, if this control is a tool tip.
    // A Rexx bag to put Rexx objects in, used to prevent gc.
    RexxObjectPtr       rexxBag;         // A bag is used, meaning the same item can be put in multiple times.
    int32_t             lastItem;        // Index of the last item added to the control
    uint32_t            id;              // Resouce ID of the control
    oodControl_t        controlType;     // Enum value for control type
    bool                isInCategoryDlg;
} CDialogControl;
typedef CDialogControl *pCDialogControl;

// A generic structure used for subclassing controls with the Windows
// subclassing helper functions and for the keyboard hook function.
typedef struct _subClassData {
    pCPlainBaseDialog   pcpbd;           // The Rexx owner dialog CSelf
    pCDialogControl     pcdc;            // The Rexx control dialog CSelf
    HWND                hCtrl;           // Window handle of subclassed control.
    void               *pData;           // Pointer to subclass specific data.
    void               *pfn;             // Pointer to subclass specific free data function.
    MESSAGETABLEENTRY  *msgs;            // Message table for generiec subclass
    size_t              mSize;           // Size of  message table.
    size_t              mNextIndex;      // Next free index in message table.
    uint32_t            id;              // Resource ID of subclassed control.
} SubClassData;
typedef SubClassData *pSubClassData;

// ToolTip relay event stuff:
#define RE_COUNT_RELAYEVENTS        5
#define RE_RELAYEVENT_IDX           0
#define RE_NEEDTEXT_IDX             1
#define RE_SHOW_IDX                 2
#define RE_POP_IDX                  3
#define RE_LINKCLICK_IDX            4
#define RE_NORELAY_IDX              5

// A specific structure used for subclassing controls to use with the tool tip
// relay event.  This structure is allocate and then set as the pData member of
// the generic SubClassData structure.  The generic SubClassData struct is set
// as the pRelayEvent in the CDialgControl struct.
// A function ... TODO finish comment
typedef struct _relayEventData {
    HWND             hToolTip;        // Window handle of tool tip
    RexxObjectPtr    rxToolTip;       // Rexx tool tip object.
    // Rexx methods to invoke, 1 for each possible event.
    char            *methods[RE_COUNT_RELAYEVENTS];
    bool             doEvent[RE_COUNT_RELAYEVENTS];
    bool             skipRelay;
} RelayEventData;
typedef RelayEventData *pRelayEventData;

extern void freeRelayData(pSubClassData pSCData);

/* Struct for the DynamicDialog object CSelf. */
typedef struct _ddCSelf {
    pCPlainBaseDialog  pcpbd;
    RexxObjectPtr      rexxSelf;
    DLGTEMPLATEEX     *base;          // Base pointer to dialog template (basePtr)
    void              *active;        // Pointer to current location in dialog template (activePtr)
    void              *endOfTemplate; // Pointer to end of allocated memory for the template
    uint32_t           expected;      // Expected dialog item count
    uint32_t           count;         // Dialog item count (dialogItemCount)
} CDynamicDialog;
typedef CDynamicDialog *pCDynamicDialog;

/* Struct for the Mouse object CSelf. Note that the owner window can be a dialog
 * window, or a dialog control window.  If the owner window is a dialog control,
 * then there is, still, a dialog CSelf and a dialog window handle.  These are
 * the dialog control's owner dialog, which are present for all dialog controls.
 */
typedef struct _mCSelf {
    RexxObjectPtr      rexxSelf;        // Rexx Mouse object.
    HWND               hWindow;         // Window handle of owner window
    RexxObjectPtr      rexxWindow;      // Rexx owner window object
    RexxObjectPtr      rexxDlg;         // Dialog Rexx self
    HWND               hDlg;            // Dialog window handle
    pCPlainBaseDialog  dlgCSelf;        // Dialog CSelf struct pointer
    pCDialogControl    controlCSelf;    // Pointer to dialog control owner CSelf struct, if owner is a dialog control window
    uint32_t           dlgProcThreadID; // Dialog window message processing function thread.
    bool               isDlgWindow;     // True if owner window is a dialog, false if owner window is a dialog control
} CMouse;
typedef CMouse *pCMouse;


/* Struct for the ControlDialogInfo object CSelf. */
typedef struct _cdiCSelf {
    SIZE              size;
    RexxObjectPtr     owner;
    char             *title;
    HINSTANCE         hInstance;        // resources attribute, C++ part of .ResourceImage
    HICON             hIcon;            // tabIcon attribute, C++ part if using .Image
    uint32_t          iconID;           // tabIcon attribute, C++ part if using resource ID
    bool              managed;
} CControlDialogInfo;
typedef CControlDialogInfo *pCControlDialogInfo;

/* Struct for the ControlDialog object CSelf. */
typedef struct _cdCSelf {
    SIZE                size;
    pCPlainBaseDialog   pcpbd;
    pCDynamicDialog     pcdd;             // DynmicDialog CSelf, may be null.
    RexxObjectPtr       rexxSelf;
    RexxStringObject    extraOpts;        // Storage for extra options, used by RcControlDialog, available for other uses.
    char               *pageTitle;
    oodClass_t          pageType;
    HICON               hIcon;            // tabIcon attribute, C++ part if using .Image
    uint32_t            iconID;           // tabIcon attribute, C++ part if using resource ID
    uint32_t            pageNumber;       // Page number, zero-based index
    uint32_t            resID;            // Resource ID of dlg template for a ResPSPDialog (converted from symbolic if needed.)
    bool                activated;        // Was the page visited by the user
    bool                isInitializing;
    bool                isManaged;
    bool                isPositioned;
} CControlDialog;
typedef CControlDialog *pCControlDialog;


/* Struct for the ManagedTab object CSelf. */
typedef struct _mtCSelf {
    RECT                 displayRect;            // The area of the tab used to display the page content.
    RexxObjectPtr        rexxOwner;              // The tab owner dialog Rexx self;
    pCPlainBaseDialog    pcpbd;                  // The tab owner dialog CSelf
    RexxObjectPtr        rexxSelf;               // Rexx MangageTab object
    RexxObjectPtr       *rexxPages;              // Array of Rexx page dialogs (control dialogs.)
    pCControlDialog     *cppPages;               // Array of page dialog CSelfs
    RexxObjectPtr        rexxTab;                // tab control Rexx object
    HWND                 hTab;                   // tab control hwnd.
    RexxObjectPtr        rxTabID;                // tab control resource ID as a Rexx object
    uint32_t             tabID;                  // tab control resource ID
    uint32_t             count;                  // count of pages
    uint32_t             startPage;              // initial page displayed in tab. zero-based index.
    uint32_t             showing;                // The current visible page.
    bool                 wantNotifications;      // Send the page dialog all tab notifications, includes NM_xx.
    bool                 needDisplayRect;        // Is the display rectangle of the tab calculated yet.
    bool                 ownerWantsSelChange;    // Also send owner TCN_SELCHANGE
	bool                 ownerWantsSelChanging;  // Also send owner TCN_SELCHANGING
    bool                 ownerWantsSetActive;    // Send setActive to owner, not ot page dialog
	bool                 ownerWantsKillActive;   // Send killActive to owner, not ot page dialog
    bool                 doingStartPage;         // Initializing start page which is not page 1.
} CManagedTab;
typedef CManagedTab *pCManagedTab;

/* Struct for the TabOwnerDlgInfo object CSelf. */
typedef struct _todiCSelf {
    pCManagedTab      mts[MAXMANAGEDTABS];
    uint32_t          count;
} CTabOwnerDlgInfo;
typedef CTabOwnerDlgInfo *pCTabOwnerDlgInfo;

/* Struct for the TabOwnerDialog object CSelf. */
typedef struct _todCSelf {
    pCManagedTab         mts[MAXMANAGEDTABS];
    uint32_t             tabIDs[MAXMANAGEDTABS];
    RexxThreadContext   *dlgProcContext;
    RexxObjectPtr        rexxSelf;
    HWND                 hDlg;
    pCPlainBaseDialog    pcpbd;
    uint32_t             countMTs;
} CTabOwnerDialog;
typedef CTabOwnerDialog *pCTabOwnerDialog;


/* Struct for the PropertySheetPage object CSelf. */
typedef struct _pspCSelf {
    RexxInstance           *interpreter;
    RexxThreadContext      *dlgProcContext;
    pCPlainBaseDialog       pcpbd;
    pCDynamicDialog         pcdd;             // DynmicDialog CSelf, may be null.
    RexxObjectPtr           rexxSelf;
    HPROPSHEETPAGE          hPropSheetPage;   // If added to existing property sheet.
    PROPSHEETPAGE          *psp;              // Allocated prop sheet page struct, needs to be freed.
    HWND                    hPage;            // Dialog handle of page.
    void                   *cppPropSheet;     // PropertySheetDialog CSelf.
    RexxObjectPtr           rexxPropSheet;    // Rexx PropertySheetDialog object.
    HINSTANCE               hInstance;        // resources attribute, C++ part of .ResourceImage
    RexxStringObject        extraOpts;        // Storage for extra options, used by RcPSPDialog, available for other uses.
    HICON                   hIcon;            // tabIcon attribute, C++ part if using .Image
    uint32_t                iconID;           // tabIcon attribute, C++ part if using resource ID
    intptr_t                 pageID;           // Identifies the page to the Windows property sheet, resource ID or pointer
    char                   *pageTitle;
    char                   *headerTitle;
    char                   *headerTitleAero;  // Must be Unicode for Aero.
    char                   *headerSubTitle;
    oodClass_t              pageType;
    uint32_t                dlgProcThreadID;
    uint32_t                cx;               // Width and height of the dialog.
    uint32_t                cy;
    uint32_t                pageFlags;
    uint32_t                resID;            // Resource ID of dlg template for a ResPSPDialog (converted from symbolic if needed.)
    uint32_t                pageNumber;       // Page number, zero-based index
    bool                    activated;        // Was the page visited by the user
    bool                    abort;            // Used to force a modal property sheet to close
    bool                    wantAccelerators; // User wants PSN_TRANSLATEACCELERATOR notifications
    bool                    wantGetObject;    // User wants PSN_GETOBJECT notifications
    bool                    isWizardPage;
    bool                    isAeroWizardPage;
    bool                    inRemovePage;     // Signals running in PropertySheetDialg::removePage()
} CPropertySheetPage;
typedef CPropertySheetPage *pCPropertySheetPage;

/* Struct for the PropertySheetDialog object CSelf. */
typedef struct _psdCSelf {
    RexxThreadContext   *dlgProcContext;
    RexxObjectPtr        rexxSelf;
    HWND                 hDlg;
    RexxObjectPtr       *rexxPages;
    pCPropertySheetPage *cppPages;
    pCPlainBaseDialog    pcpbd;
    HINSTANCE            hInstance;        // resources attribute, C++ part of .ResourceImage
    HICON                hIcon;            // icon attribute, C++ part if using .Image
    uint32_t             iconID;           // icon attribute, C++ part if using resource ID
    HBITMAP              hWatermark;       // waterMark attribute, C++ part if using .Image
    uint32_t             watermarkID;      // watermark attribute, C++ part if useing resource ID
    HBITMAP              hHeaderBitmap;    // headerBitmap attribute, C++ part if using .Image
    uint32_t             headerBitmapID;   // headerBitmap attribute, C++ part if useing resource ID
    HPALETTE             hplWatermark;     // Palette to use when drawing watermark and / or header bitmap
    HIMAGELIST           imageList;        // imageList attribute, C++ part of .ImageList
    uint32_t             startPage;        // Index of start page, 1-based.  If 0 not set;
    uint32_t             dlgProcThreadID;
    char                *caption;
    uint32_t             pageCount;
    uint32_t             propSheetFlags;
    uint32_t             id;               // Used to ID the property sheet if resizable
    intptr_t             getResultValue;   // Storage for the return from PSM_GETRESULT
    bool                 modeless;
    bool                 isNotWizard;
    bool                 isWiz97;
    bool                 isWizLite;
    bool                 isAeroWiz;
    bool                 isFirstShow;      // For resizable dialogs, in subclass proc, indicates is 1st WM_SHOWWINDOW
} CPropertySheetDialog;
typedef CPropertySheetDialog *pCPropertySheetDialog;

/*
 * Struct for the page dialog information.  This is used for initializing the
 * dialog template pointer for either a property sheet page dialog or a managed
 * tab page dialog.  (A managed tab page dialog is a ControlDialog in a
 * ManagedTab.)
 */
typedef struct _pdi {
    pCPlainBaseDialog   pcpbd;            // PlainBaseDialog CSelf
    pCDynamicDialog     pcdd;             // DynamicDialog CSelf
    void               *pPageCSelf;
    RexxObjectPtr       rexxSelf;
    RexxStringObject    extraOpts;
    char               *pageTitle;        // Page title (could be null.)
    char               *newTitle;         // If pagetitle is null, this is the generated page title
    intptr_t             pageID;           // Returned, but only used by property sheet pages.
    oodClass_t          pageType;
    uint32_t            resID;
    uint32_t            pageNumber;       // Page number, zero-based index
    uint32_t            cx;               // Width and height of the dialog.
    uint32_t            cy;
} PageDialogInfo;
typedef PageDialogInfo *pPageDialogInfo;


/* Struct for the AppliationManager object CSelf. */
typedef struct _amCSelf {
    RexxObjectPtr     rexxSelf;
    RexxObjectPtr     rxProgramDir;  // The directory the main program resides in, or .nil
    bool              autoDetect;
} CApplicationManager;
typedef CApplicationManager *pCApplicationManager;


/* Struct for the SPI class object CSelf. */
typedef struct _spiCSelf {
    // Flag used when setting a system parameter to have the setting written to
    // the user profile.  And possibly broadcasts the WM_SETTINGSCHANGE message
    // after updating the user profile.
    uint32_t  fWinIni;
} CSpi;
typedef CSpi *pCSpi;


#define COMCTL32_VERSION_STRING_LEN  31

// All global variables are defined in oodPackageEntry.cpp
extern HINSTANCE           MyInstance;
extern pCPlainBaseDialog   DialogTable[];
extern pCPlainBaseDialog   TopDlg;
extern size_t              CountDialogs;
extern CRITICAL_SECTION    crit_sec;
extern CRITICAL_SECTION    ps_crit_sec;
extern DWORD               ComCtl32Version;
extern char                ComCtl32VersionStr[];

extern RexxObjectPtr       TheApplicationObj;
extern RexxDirectoryObject TheConstDir;
extern oodConstDir_t       TheConstDirUsage;
extern HICON               TheDefaultSmallIcon;
extern HICON               TheDefaultBigIcon;
extern bool                DefaultIconIsShared;

extern RexxClassObject ThePlainBaseDialogClass;
extern RexxClassObject TheDynamicDialogClass;
extern RexxClassObject TheDialogControlClass;
extern RexxClassObject ThePropertySheetPageClass;
extern RexxClassObject TheControlDialogClass;
extern RexxClassObject ThePointClass;
extern RexxClassObject TheSizeClass;
extern RexxClassObject TheRectClass;
extern RexxClassObject TheLvCustomDrawSimpleClass;
extern RexxClassObject TheTvCustomDrawSimpleClass;
extern RexxClassObject TheLvFullRowClass;
extern RexxClassObject TheLvItemClass;
extern RexxClassObject TheLvSubItemClass;
extern RexxClassObject TheReBarBandInfoClass;
extern RexxClassObject TheTbButtonClass;

extern HBRUSH searchForBrush(pCPlainBaseDialog pcpbd, size_t *index, uint32_t id);

extern bool _isVersion(DWORD, DWORD, unsigned int, unsigned int, unsigned int);
extern bool _is32on64Bit(void);

// Enum for a Windows OS, don't need many right now.
typedef enum
{
    XP_OS, Vista_OS, Windows7_OS
} os_name_t;

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

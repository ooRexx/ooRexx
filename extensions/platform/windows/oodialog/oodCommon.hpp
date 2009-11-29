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

#ifndef oodCommon_Included
#define oodCommon_Included

// Map string keywords representing constant defines to their int values.  For
// translating things like "IDI_APPLICATION" from the user to the proper API
// value.
#include <string>
#include <map>
using namespace std;
typedef map<string, int, less<string> > String2Int;


#define COMCTL_ERR_TITLE             "ooDialog - Windows Common Controls Error"
#define DLLGETVERSION_FUNCTION       "DllGetVersion"
#define COMMON_CONTROL_DLL           "comctl32.dll"

#define NO_COMMCTRL_MSG              "failed to initialize %s; OS error code %d"
#define COMCTL32_FULL_PART           0
#define COMCTL32_NUMBER_PART         1
#define COMCTL32_OS_PART             2

#define OOD_RESOURCE_ERR_TITLE      "ooDialog - Resource Definition Error"

#define OOD_ADDICONFILE_ERR_MSG     "Icon resource elements have exceeded the maximum\n" \
                                    "number of allocated icon table entries. The icon\n" \
                                    "resource will not be added."

#define DEFAULT_FONTNAME            "MS Shell Dlg"
#define DEFAULT_FONTSIZE            8
#define MAX_DEFAULT_FONTNAME        256

#define OOD_ID_EXCEPTION            0xFFFFFFF7   // -9

// Enum for the type of an ooDialog class.  Types to be added as needed.
typedef enum
{
    oodPlainBaseDialog, oodCategoryDialog, oodStaticControl, oodButtonControl, oodEditControl,
    oodListBox,         oodProgressBar,    oodUnknown
} oodClass_t;

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
    RexxObjectPtr  rexxSelf;
    HWND           hDlg;
    DIALOGADMIN    *dlgAdm;
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

/* Struct for the PlainBaseDialog object CSelf. */
typedef struct _pbdCSelf {
    char                 fontName[MAX_DEFAULT_FONTNAME];
    pCWindowBase         wndBase;
    pCEventNotification  enCSelf;
    pCWindowExtensions   weCSelf;
    RexxObjectPtr        rexxSelf;
    HWND                 hDlg;
    DIALOGADMIN          *dlgAdm;
    HBRUSH               bkgBrush;
    HBITMAP              bkgBitmap;
    logical_t            autoDetect;
    uint32_t             fontSize;
} CPlainBaseDialog;
typedef CPlainBaseDialog *pCPlainBaseDialog;

/* Struct for the DialogControl object CSelf. */
typedef struct _dcCSelf {
    pCWindowBase   wndBase;
    RexxObjectPtr  rexxSelf;
    uint32_t       id;
    HWND           hCtrl;    // Handle of the dialog control
    HWND           hDlg;     // Handle of the owner dialog
    RexxObjectPtr  oDlg;     // The Rexx owner dialog object
} CDialogControl;
typedef CDialogControl *pCDialogControl;

/* Struct for the DynamicDialog object CSelf. */
typedef struct _ddCSelf {
    pCPlainBaseDialog  pcpbd;
    RexxObjectPtr      rexxSelf;
    DLGTEMPLATE       *base;    // Base pointer to dialog template (basePtr)
    void              *active;  // Pointer to current location in dialog template (activePtr)
    uint32_t           count;   // Dialog item count (dialogItemCount)
} CDynamicDialog;
typedef CDynamicDialog *pCDynamicDialog;

extern LRESULT CALLBACK RexxDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
extern bool             dialogInAdminTable(DIALOGADMIN * Dlg);
extern bool             InstallNecessaryStuff(DIALOGADMIN* dlgAdm, CSTRING library);
extern int32_t          stopDialog(HWND hDlg);
extern int32_t          DelDialog(DIALOGADMIN * aDlg);
extern BOOL             GetDialogIcons(DIALOGADMIN *, INT, UINT, PHANDLE, PHANDLE);
extern void             rxstrlcpy(CHAR * tar, CONSTRXSTRING &src);
extern void             rxdatacpy(CHAR * tar, RXSTRING &src);
extern bool             isYes(const char *s);
extern void *           string2pointer(const char *string);
extern void *           string2pointer(RexxMethodContext *c, RexxStringObject string);
extern void             pointer2string(char *, void *pointer);
extern RexxStringObject pointer2string(RexxMethodContext *, void *);
extern RexxStringObject pointer2string(RexxThreadContext *c, void *pointer);
extern LONG             HandleError(PRXSTRING r, CHAR * text);
extern char *           strdupupr(const char *str);
extern char *           strdupupr_nospace(const char *str);
extern char *           strdup_nospace(const char *str);
extern char *           strdup_2methodName(const char *str);
extern DIALOGADMIN *    getDlgAdm(RexxMethodContext *c, RexxObjectPtr dlg);
extern DIALOGADMIN *    rxGetDlgAdm(RexxMethodContext *, RexxObjectPtr);

extern BOOL addTheMessage(DIALOGADMIN *, UINT, UINT, WPARAM, ULONG_PTR, LPARAM, ULONG_PTR, CSTRING, ULONG);

extern void          ooDialogInternalException(RexxMethodContext *, char *, int, char *, char *);
extern RexxObjectPtr noWindowsDialogException(RexxMethodContext *c, RexxObjectPtr rxDlg);
extern RexxObjectPtr invalidCategoryPageException(RexxMethodContext *c, int, int);
extern inline void   failedToRetrieveDlgAdmException(RexxThreadContext *c, RexxObjectPtr source);
extern void          controlFailedException(RexxThreadContext *, CSTRING, CSTRING, CSTRING);
extern void          wrongWindowStyleException(RexxMethodContext *c, CSTRING, CSTRING);

extern oodClass_t    oodClass(RexxMethodContext *, RexxObjectPtr, oodClass_t *, size_t);
extern uint32_t      oodResolveSymbolicID(RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, int);
extern bool          oodSafeResolveID(uint32_t *, RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, int);
extern DWORD         oodGetSysErrCode(RexxThreadContext *);
extern void          oodSetSysErrCode(RexxThreadContext *, DWORD);
extern void          oodResetSysErrCode(RexxThreadContext *context);
extern bool          oodGetWParam(RexxMethodContext *, RexxObjectPtr, WPARAM *, int);
extern bool          oodGetLParam(RexxMethodContext *, RexxObjectPtr, LPARAM *, int);

extern int32_t    checkID(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr self);
extern int32_t    idError(RexxMethodContext *c, RexxObjectPtr rxID);
extern int32_t    resolveResourceID(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr self);
extern int32_t    resolveIconID(RexxMethodContext *c, RexxObjectPtr rxIconID, RexxObjectPtr self);
extern bool       requiredComCtl32Version(RexxMethodContext *context, const char *methodName, DWORD minimum);

extern PPOINT        rxGetPoint(RexxMethodContext *context, RexxObjectPtr p, int argPos);
extern RexxObjectPtr rxNewPoint(RexxMethodContext *c, long x, long y);
extern PRECT         rxGetRect(RexxMethodContext *context, RexxObjectPtr r, int argPos);
extern RexxObjectPtr rxNewRect(RexxMethodContext *context, long l, long t, long r, long b);
extern RexxObjectPtr rxNewRect(RexxMethodContext *context, PRECT r);
extern PSIZE         rxGetSize(RexxMethodContext *context, RexxObjectPtr s, int argPos);
extern RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy);

extern bool rxGetWindowText(RexxMethodContext *c, HWND hwnd, RexxStringObject *pStringObj);
extern bool rxLogicalFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, BOOL *, int);
extern bool rxNumberFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, DWORD *, int);
extern bool rxIntFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, int *, int);

extern RexxObjectPtr setWindowStyle(RexxMethodContext *c, HWND hwnd, uint32_t style);
extern int           putUnicodeText(LPWORD dest, const char *text);
extern int           getKeywordValue(String2Int *cMap, const char * str);
extern bool          goodMinMaxArgs(RexxMethodContext *c, RexxArrayObject args, int min, int max, size_t *arraySize);
extern bool          getRectFromArglist(RexxMethodContext *, RexxArrayObject, PRECT, bool, int, int, size_t *, int *);
extern bool          getPointFromArglist(RexxMethodContext *, RexxArrayObject, PPOINT, int, int, size_t *, int *);

// TODO move to APICommon when ooDialog is converted to use .Pointer instead of
// pointer strings.
extern POINTER rxGetPointerAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name);

// These functions are defined in oodUser.cpp.
extern bool getCategoryHDlg(RexxMethodContext *, RexxObjectPtr, uint32_t *, HWND *, int);
extern uint32_t getCategoryNumber(RexxMethodContext *, RexxObjectPtr);

// These functions are defined in oodUtilities.cpp
extern const char *comctl32VersionPart(DWORD id, DWORD type);

// These functions are defined in ooDialog.cpp
extern bool          initWindowBase(RexxMethodContext *c, HWND hwndObj, RexxObjectPtr self, pCWindowBase *ppCWB);
extern RexxObjectPtr setDlgHandle(RexxMethodContext *c, pCPlainBaseDialog pcpbd, HWND hDlg);
extern RexxObjectPtr oodSetForegroundWindow(RexxMethodContext *c, HWND hwnd);
extern RexxObjectPtr oodGetFocus(RexxMethodContext *c, HWND hDlg);
extern RexxObjectPtr sendWinMsgGeneric(RexxMethodContext *, HWND, CSTRING, RexxObjectPtr, RexxObjectPtr, int, bool);

// These functions are defined in oodBaseDialog.cpp
extern bool initWindowExtensions(RexxMethodContext *, RexxObjectPtr, HWND, pCWindowBase, pCPlainBaseDialog);

// Shared button stuff.
typedef enum {push, check, radio, group, owner, notButton} BUTTONTYPE, *PBUTTONTYPE;
typedef enum {def, autoCheck, threeState, autoThreeState, noSubtype } BUTTONSUBTYPE, *PBUTTONSUBTYPE;

extern BUTTONTYPE getButtonInfo(HWND, PBUTTONSUBTYPE, DWORD *);

#define GET_HANDLE(p) string2pointer(p)
#define GET_HWND(p)   ((HWND)string2pointer(p))
#define GET_POINTER(p) string2pointer(p)

inline void *string2pointer(CONSTRXSTRING *string) { return string2pointer(string->strptr); }
inline void *string2pointer(CONSTRXSTRING &string) { return string2pointer(string.strptr); }

// TODO check whether these functions are really inlined.

inline void pointer2string(PRXSTRING result, void *pointer)
{
    pointer2string(result->strptr, pointer);
    result->strlength = strlen(result->strptr);
}

inline void safeLocalFree(void *p)
{
    if (p != NULL)
    {
        LocalFree(p);
    }
}

inline void safeFree(void *p)
{
    if (p != NULL)
    {
        free(p);
    }
}

inline void safeDeleteObject(HANDLE h)
{
    if (h != NULL)
    {
        DeleteObject(h);
    }
}

inline void oodSetSysErrCode(RexxThreadContext *context)
{
    oodSetSysErrCode(context, GetLastError());
}

inline HWND rxGetWindowHandle(RexxMethodContext * context, RexxObjectPtr windowObject)
{
    return (HWND)rxGetPointerAttribute(context, windowObject, "HWND");
}


inline LPWORD lpwAlign(LPWORD lpIn)
{
  ULONG_PTR ul = (ULONG_PTR)lpIn;
  ul +=3;
  ul >>=2;
  ul <<=2;
  return (LPWORD)ul;
}


/**
 * Returns the first character of the message name that invoked the current
 * method.
 *
 * @param context  The method context.
 *
 * @return The first charactere of the message name.
 */
inline char msgAbbrev(RexxMethodContext *context)
{
    return *(context->GetMessageName());
}

/**
 * Checks that the argument could be construed as 'true'.  This would be 1 or
 * yes, but for historical reasons the German ja must also be included.
 *
 * This will also work for an optional arg to an API method.  I.e., if s is
 * null, false is returned.
 *
 * @param s  The string to check.
 *
 * @return bool
 */
inline bool isYes(const char * s)
{
   if ( s == NULL || strlen(s) == 0 )
   {
       return FALSE;
   }

   char c = toupper(s[0]);
   return ( c == 'J' || c =='Y' || c == '1' );
}

inline const char *comctl32VersionName(DWORD id)
{
    return comctl32VersionPart(id, COMCTL32_FULL_PART);
}

inline bool hasStyle(HWND hwnd, LONG style)
{
    if ( (GetWindowLong(hwnd, GWL_STYLE) & style) || (GetWindowLong(hwnd, GWL_EXSTYLE) & style) )
    {
        return true;
    }
    return false;
}

/**
 * Retrieves the PlainBaseDialog class CSelf pointer.
 *
 * @param c  Method contex we are operating in.
 *
 * @return The pointer to the CPlainBaseDialogClass struct.
 */
inline pCPlainBaseDialogClass getPBDClass_CSelf(RexxMethodContext *c)
{
    return (pCPlainBaseDialogClass)c->ObjectToCSelf(ThePlainBaseDialogClass);
}

/**
 * Retrieves the CSelf pointer for a dialog object when the dialog object is not
 * the direct object the method was invoked on.  This performs a scoped CSelf
 * lookup.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose CSelf pointer is needed.
 *
 * @return A pointer to the CSelf of the dlg object.
 */
inline pCPlainBaseDialog dlgToCSelf(RexxMethodContext *c, RexxObjectPtr dlg)
{
    return (pCPlainBaseDialog)c->ObjectToCSelf(dlg, ThePlainBaseDialogClass);
}

/**
 * Convenience function to put up an error message box.
 *
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
inline void internalErrorMsgBox(CSTRING pszMsg, CSTRING pszTitle)
{
    MessageBox(0, pszMsg, pszTitle, MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
}


#endif

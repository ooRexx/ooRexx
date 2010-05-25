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

// Some int32_t codes where -1 and greater is valid.
#define OOD_ID_EXCEPTION            0xFFFFFFF7   // -9
#define OOD_BAD_WIDTH_EXCEPTION     0xFFFFFFF8   // -8
#define OOD_INVALID_ITEM_ID         0xFFFFFFF7   // Rewording of OOD_ID_EXCEPTION

// Some uint32_t result codes  TODO set up an enum for above and these.
#define OOD_NO_ERROR                0
#define OOD_DATATABLE_FULL          1
#define OOD_MEMORY_ERR              2

// Enum for the type of an ooDialog class.  Types to be added as needed.
typedef enum
{
    oodPlainBaseDialog, oodCategoryDialog, oodStaticControl, oodButtonControl, oodEditControl,
    oodListBox,         oodProgressBar,    oodUnknown
} oodClass_t;

/* Struct for a reply to the UDN_DELTAPOS notification message. (Up-down control.) */
typedef struct _DELTAPOS_REPLY {
    bool      change;
    bool      cancel;
    int32_t   newDelta;
} DELTAPOSREPLY;
typedef DELTAPOSREPLY *PDELTAPOSREPLY;


extern bool              installNecessaryStuff(pCPlainBaseDialog pcpbd, CSTRING library);
extern int32_t           stopDialog(pCPlainBaseDialog);
extern int32_t           delDialog(pCPlainBaseDialog);
extern BOOL              getDialogIcons(pCPlainBaseDialog, INT, UINT, PHANDLE, PHANDLE);
extern bool              isYes(const char *s);
extern void *            string2pointer(const char *string);
extern void *            string2pointer(RexxMethodContext *c, RexxStringObject string);
extern void              pointer2string(char *, void *pointer);
extern RexxStringObject  pointer2string(RexxMethodContext *, void *);
extern RexxStringObject  pointer2string(RexxThreadContext *c, void *pointer);
extern RexxStringObject  dword2string(RexxMethodContext *, uint32_t);
extern char *            strdupupr(const char *str);
extern char *            strdupupr_nospace(const char *str);
extern char *            strdup_nospace(const char *str);
extern char *            strdup_2methodName(const char *str);
extern void              checkModal(pCPlainBaseDialog previous, BOOL modeless);
extern void              enablePrevious(pCPlainBaseDialog previous);

extern DIALOGADMIN *     getDlgAdm(RexxMethodContext *c, RexxObjectPtr dlg);
extern pCPlainBaseDialog getDlgCSelf(RexxMethodContext *c, RexxObjectPtr self);

extern oodClass_t    oodClass(RexxMethodContext *, RexxObjectPtr, oodClass_t *, size_t);
extern uint32_t      oodResolveSymbolicID(RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, size_t);
extern bool          oodSafeResolveID(uint32_t *, RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, size_t);
extern DWORD         oodGetSysErrCode(RexxThreadContext *);
extern void          oodSetSysErrCode(RexxThreadContext *, DWORD);
extern void          oodResetSysErrCode(RexxThreadContext *context);
extern bool          oodGetWParam(RexxMethodContext *, RexxObjectPtr, WPARAM *, size_t, bool);
extern bool          oodGetLParam(RexxMethodContext *, RexxObjectPtr, LPARAM *, size_t, bool);
extern bool          oodObj2handle(RexxMethodContext *c, RexxObjectPtr obj, void **result, size_t argPos);
extern void         *oodObj2pointer(RexxMethodContext *c, RexxObjectPtr obj);

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
extern RexxObjectPtr rxNewSize(RexxThreadContext *c, long cx, long cy);
extern RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy);

extern bool rxGetWindowText(RexxMethodContext *c, HWND hwnd, RexxStringObject *pStringObj);
extern bool rxDirectoryFromArray(RexxMethodContext *c, RexxArrayObject a, size_t index, RexxDirectoryObject *d, size_t argPos);
extern bool rxLogicalFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, BOOL *, int, bool);
extern bool rxNumberFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, uint32_t *, int, bool);
extern bool rxIntFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, int *, int, bool);

extern RexxObjectPtr     setWindowStyle(RexxMethodContext *c, HWND hwnd, uint32_t style);
extern int               putUnicodeText(LPWORD dest, const char *text);
extern RexxStringObject  unicode2String(RexxMethodContext *c, PWSTR wstr, int32_t len);
extern char *            unicode2Ansi(PWSTR wstr, int32_t len);
extern int               getKeywordValue(String2Int *cMap, const char * str);
extern bool              goodMinMaxArgs(RexxMethodContext *c, RexxArrayObject args, size_t min, size_t max, size_t *arraySize);
extern bool              getRectFromArglist(RexxMethodContext *, RexxArrayObject, PRECT, bool, int, int, size_t *, size_t *);
extern bool              getPointFromArglist(RexxMethodContext *, RexxArrayObject, PPOINT, int, int, size_t *, size_t *);

// These functions are defined in oodUser.cpp.
extern bool getCategoryHDlg(RexxMethodContext *, RexxObjectPtr, uint32_t *, HWND *, int);
extern uint32_t getCategoryNumber(RexxMethodContext *, RexxObjectPtr);

// These functions are defined in oodUtilities.cpp
extern const char *  comctl32VersionPart(DWORD id, DWORD type);
extern RexxObjectPtr makeDayStateBuffer(RexxMethodContext *c, RexxArrayObject list, size_t count, LPMONTHDAYSTATE *ppmds);
extern RexxObjectPtr quickDayStateBuffer(RexxMethodContext *c, uint32_t ds1, uint32_t ds2, uint32_t ds3, LPMONTHDAYSTATE *ppmds);

// These functions are defined in ooDialog.cpp
extern bool          initWindowBase(RexxMethodContext *c, HWND hwndObj, RexxObjectPtr self, pCWindowBase *ppCWB);
extern void          setDlgHandle(RexxMethodContext *c, pCPlainBaseDialog pcpbd);
extern RexxObjectPtr oodSetForegroundWindow(RexxMethodContext *c, HWND hwnd);
extern RexxObjectPtr oodGetFocus(RexxMethodContext *c, HWND hDlg);
extern RexxObjectPtr sendWinMsgGeneric(RexxMethodContext *, HWND, CSTRING, RexxObjectPtr, RexxObjectPtr, size_t, bool);

// These functions are defined in oodBaseDialog.cpp
extern bool initWindowExtensions(RexxMethodContext *, RexxObjectPtr, HWND, pCWindowBase, pCPlainBaseDialog);

// Shared button stuff.
typedef enum {push, check, radio, group, owner, notButton} BUTTONTYPE, *PBUTTONTYPE;
typedef enum {def, autoCheck, threeState, autoThreeState, noSubtype } BUTTONSUBTYPE, *PBUTTONSUBTYPE;

extern BUTTONTYPE getButtonInfo(HWND, PBUTTONSUBTYPE, DWORD *);

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
       return false;
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

extern void          ooDialogInternalException(RexxMethodContext *, char *, int, char *, char *);
extern RexxObjectPtr noWindowsDialogException(RexxMethodContext *c, RexxObjectPtr rxDlg);
extern RexxObjectPtr invalidWindowException(RexxMethodContext *c, RexxObjectPtr rxObj);
extern RexxObjectPtr invalidCategoryPageException(RexxMethodContext *c, int, int);
extern RexxObjectPtr wrongClassReplyException(RexxThreadContext *c, const char *n);
extern void          controlFailedException(RexxThreadContext *, CSTRING, CSTRING, CSTRING);
extern void          wrongWindowStyleException(RexxMethodContext *c, CSTRING, CSTRING);
extern RexxObjectPtr wrongWindowsVersionException(RexxMethodContext *, const char *, const char *);

inline void failedToRetrieveDlgAdmException(RexxThreadContext *c, RexxObjectPtr source)
{
    failedToRetrieveException(c, "dialog administration block", source);
}

inline void failedToRetrieveDlgAdmException(RexxThreadContext *c)
{
    userDefinedMsgException(c, "Could not retrieve the dialog administration information block.");
}

inline void failedToRetrieveDlgCSelfException(RexxThreadContext *c, RexxObjectPtr source)
{
    failedToRetrieveException(c, "dialog CSelf information", source);
}

inline void failedToRetrieveDlgCSelfException(RexxThreadContext *c)
{
    userDefinedMsgException(c, "Could not retrieve the dialog CSelf information.");
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
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx dialog
 *           object.
 */
inline pCPlainBaseDialog dlgToCSelf(RexxMethodContext *c, RexxObjectPtr dlg)
{
    return (pCPlainBaseDialog)c->ObjectToCSelf(dlg, ThePlainBaseDialogClass);
}

/**
 * Retrieves the dialog admin block from an ooDialog dialog object.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose dialog admin block is needed.
 *
 * @return A pointer to dialog admin block for the dialog.
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx dialog
 *           object.
 */
inline DIALOGADMIN *dlgToDlgAdm(RexxMethodContext *c, RexxObjectPtr dlg)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(c, dlg);
    return pcpbd->dlgAdm;
}

/**
 * Retrieves the EventNotification CSelf from an ooDialog dialog object.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose EventNotification CSelf is needed.
 *
 * @return A pointer to CEventNotification struct for the dialog.
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx dialog
 *           object.
 */
inline pCEventNotification dlgToEventNotificationCSelf(RexxMethodContext *c, RexxObjectPtr dlg)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(c, dlg);
    return pcpbd->enCSelf;
}

/**
 * Retrieves the dialog window handle from an ooDialog dialog object.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose window handle is needed.
 *
 * @return The window handle of the dialog.  Note that this will be null if the
 *         underlying Windows dialog has not yet been created.
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx dialog
 *           object.
 */
inline HWND dlgToHDlg(RexxMethodContext *c, RexxObjectPtr dlg)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(c, dlg);
    return pcpbd->hDlg;
}

/**
 * Retrieves the CSelf pointer for a dialog control object when the control
 * object is not the direct object the method was invoked on.  This performs a
 * scoped CSelf lookup.
 *
 * @param c     The method context we are operating in.
 * @param ctrl  The control object whose CSelf pointer is needed.
 *
 * @return A pointer to the CSelf of the control object.
 *
 * @assumes  The caller has ensured ctrl is in fact a ooDialog Rexx dialog
 *           control object.
 */
inline pCDialogControl controlToCSelf(RexxMethodContext *c, RexxObjectPtr ctrl)
{
    return (pCDialogControl)c->ObjectToCSelf(ctrl, TheDialogControlClass);
}

/**
 * Retrieves the window handle for a dialog control object when the control
 * object is not the direct object the method was invoked on.  This performs a
 * scoped CSelf lookup.
 *
 * @param c     The method context we are operating in.
 * @param ctrl  The control object whose window handle is needed.
 *
 * @return The window handle of the dialog control.
 *
 * @assumes  The caller has ensured ctrl is in fact a ooDialog Rexx dialog
 *           control object.
 */
inline HWND controlToHCtrl(RexxMethodContext *c, RexxObjectPtr ctrl)
{
    pCDialogControl pcdc = (pCDialogControl)c->ObjectToCSelf(ctrl, TheDialogControlClass);
    return pcdc->hCtrl;
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

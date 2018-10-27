/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

#define NO_COMMCTRL_MSG           "failed to initialize %s; OS error code %d"
#define NO_HMODULE_MSG            "failed to obtain %s module handle; OS error code %d"
#define NO_PROC_MSG               "failed to get procedeure adddress for %s(); OS error code %d"
#define API_FAILED_MSG            "system API %s() failed; OS error code %d"
#define COM_API_FAILED_MSG        "system API %s() failed; COM code 0x%08x"
#define FUNC_WINCTRL_FAILED_MSG   "the '%s'() function of the Windows '%s' control failed"
#define MSG_WINCTRL_FAILED_MSG    "the '%s' message of the Windows '%s' control failed"
#define NO_LOCAL_ENVIRONMENT_MSG  "the .local environment was not found"
#define NO_SIZE_CLASS_MSG         "the .Size class was not found"
#define BAD_APPLICATION_MSG       "the .Application object already exists"


#define COMCTL32_FULL_PART           0
#define COMCTL32_NUMBER_PART         1
#define COMCTL32_OS_PART             2

#define OOD_RESOURCE_ERR_TITLE      "ooDialog - Resource Definition Error"

#define OOD_ADDICONFILE_ERR_MSG     "Icon resource elements have exceeded the maximum\n" \
                                    "number of allocated icon table entries, and the\n" \
                                    "table could not be expanded.\n\n" \
                                    "The icon resource will not be added."

// ooDialog return codes for special purposes, including some int32_t codes
// where -1 and greater is valid.
#define OOD_ID_EXCEPTION            0xFFFFFFF7   // -9
#define OOD_INVALID_ITEM_ID         0xFFFFFFF7   // Rewording of OOD_ID_EXCEPTION
#define OOD_BAD_WIDTH_EXCEPTION     0xFFFFFFF8   // -8
#define OOD_MEMORY_ERR              0xFFFFFFF9   // -7
#define OOD_NO_VALUE                0xFFFFFFFA   // -6
#define OOD_NO_ERROR                0
#define OOD_DATATABLE_FULL          1

// Tool tip stuff
#define MAX_TOOLTITLE_TEXT_LENGTH   99
#define TT_CCH_TOOLTITLE_BUF        MAX_TOOLTITLE_TEXT_LENGTH + 1
#define TT_MAX_ICON_KEYWORD         TTI_ERROR_LARGE
#define TT_VALID_ICON_VALUES        "an icon Image object, or keyword: None, Error, ErrorLarge, Info, InfoLarge, Warning, or WarningLarge"

// The context variable names used by the ToolInfo object to keep track of
// things.
#define TOOLINFO_MEMALLOCATED_VAR     "TEXT_MEMORY_IS_ALLOCATED"
#define TOOLINFO_HWND_OBJECT_VAR      "HWND_SUPPLYING_OBJECT"
#define TOOLINFO_UID_OBJECT_VAR       "UID_SUPPLYING_OBJECT"

extern uint32_t         keyword2ttdiFlags(CSTRING flags);
extern RexxStringObject ttdiFlags2keyword(RexxThreadContext *c, uint32_t flags);


/* Struct for a reply to the UDN_DELTAPOS notification message. (Up-down control.) */
typedef struct _DELTAPOS_REPLY {
    bool      change;
    bool      cancel;
    int32_t   newDelta;
} DELTAPOSREPLY;
typedef DELTAPOSREPLY *PDELTAPOSREPLY;

extern const char *      comctl32VersionPart(DWORD id, DWORD type);
extern bool              installNecessaryStuff(pCPlainBaseDialog pcpbd, CSTRING library);
extern int32_t           stopDialog(pCPlainBaseDialog, RexxThreadContext *c);
extern int32_t           delDialog(pCPlainBaseDialog, RexxThreadContext *c);
extern BOOL              getDialogIcons(pCPlainBaseDialog, INT, UINT, PHANDLE, PHANDLE);
extern bool              isYes(const char *s);
extern RexxStringObject  dword2string(RexxMethodContext *, uint32_t);
extern RexxObjectPtr     convertToTrueOrFalse(RexxThreadContext *c, RexxObjectPtr obj);
extern char *            strdupupr(const char *str);
extern char *            strdupupr_nospace(const char *str);
extern char *            strdup_nospace(const char *str);
extern char *            strdup_2methodName(const char *str);
extern void              checkModal(pCPlainBaseDialog previous, logical_t modeless);

extern oodClass_t    oodClass(RexxMethodContext *, RexxObjectPtr, oodClass_t *, size_t);
extern uint32_t      oodGetSysErrCode(RexxThreadContext *);
extern void          oodSetSysErrCode(RexxThreadContext *, DWORD);
extern void          oodResetSysErrCode(RexxThreadContext *context);
extern bool          oodGetWParam(RexxMethodContext *, RexxObjectPtr, WPARAM *, size_t, bool);
extern bool          oodGetLParam(RexxMethodContext *, RexxObjectPtr, LPARAM *, size_t, bool);
extern bool          oodObj2handle(RexxMethodContext *c, RexxObjectPtr obj, void **result, size_t argPos);
extern void         *oodObj2pointer(RexxMethodContext *c, RexxObjectPtr obj);

extern int32_t    checkID(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr self);
extern int32_t    idError(RexxMethodContext *c, RexxObjectPtr rxID);
extern int32_t    oodGlobalID(RexxThreadContext *c, RexxObjectPtr id, size_t argPosID, bool strict);
extern int32_t    oodResolveSymbolicID(RexxThreadContext *, RexxObjectPtr, RexxObjectPtr, int, size_t, bool);
extern bool       oodSafeResolveID(int32_t *, RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, size_t, bool);
extern int32_t    resolveResourceID(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr self);
extern int32_t    resolveIconID(RexxMethodContext *c, RexxObjectPtr rxIconID, RexxObjectPtr self);
extern HICON      getOORexxIcon(uint32_t id);

extern bool       requiredOS(RexxMethodContext *context, const char *method, const char *osName, os_name_t os);
extern bool       requiredOS(RexxMethodContext *context, os_name_t os, const char *msg, const char *osName);
extern bool       requiredComCtl32Version(RexxMethodContext *context, const char *methodName, DWORD minimum);
extern bool       requiredComCtl32Version(RexxMethodContext *context, DWORD minimum, const char *msg);

extern bool rxGetWindowText(RexxMethodContext *c, HWND hwnd, RexxStringObject *pStringObj);
extern bool rxDirectoryFromArray(RexxMethodContext *c, RexxArrayObject a, size_t index, RexxDirectoryObject *d, size_t argPos);
extern bool rxLogicalFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, BOOL *, int, bool);
extern bool rxNumberFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, uint32_t *, int, bool);
extern bool rxIntFromDirectory(RexxMethodContext *, RexxDirectoryObject, CSTRING, int *, int, bool);

extern bool              printHResultErr(CSTRING api, HRESULT hr);
extern bool              getFormattedErrMsg(char **errBuff, uint32_t errCode, uint32_t *thisErr);

extern RexxObjectPtr     setWindowStyle(RexxMethodContext *c, HWND hwnd, uint32_t style);
extern int               getKeywordValue(String2Int *cMap, const char * str);

// These functions are defined in oodUser.cpp.
extern bool getCategoryHDlg(RexxMethodContext *, RexxObjectPtr, uint32_t *, HWND *, int);
extern uint32_t getCategoryNumber(RexxMethodContext *, RexxObjectPtr);


// These functions are defined in oodUtilities.cpp
extern RexxObjectPtr makeDayStateBuffer(RexxMethodContext *c, RexxArrayObject list, size_t count, LPMONTHDAYSTATE *ppmds);
extern RexxObjectPtr makeQuickDayStateBuffer(RexxMethodContext *c, RexxObjectPtr _ds1, RexxObjectPtr _ds2, RexxObjectPtr _ds3, LPMONTHDAYSTATE *ppmds);
extern RexxObjectPtr quickDayStateBuffer(RexxMethodContext *c, uint32_t ds1, uint32_t ds2, uint32_t ds3, LPMONTHDAYSTATE *ppmds);
extern void          putDefaultSymbols(RexxMethodContext *c, RexxDirectoryObject constDir);

// These functions are defined in ooDialog.cpp
extern bool          initWindowBase(RexxMethodContext *c, HWND hwndObj, RexxObjectPtr self, pCWindowBase *ppCWB);
extern void          setDlgHandle(pCPlainBaseDialog pcpbd);
extern RexxObjectPtr oodSetForegroundWindow(RexxMethodContext *c, HWND hwnd);
extern RexxObjectPtr oodGetFocus(RexxMethodContext *c, HWND hDlg);
extern RexxObjectPtr sendWinMsgGeneric(RexxMethodContext *, HWND, CSTRING, RexxObjectPtr, RexxObjectPtr, size_t, bool);
extern bool          loadResourceDLL(pCPlainBaseDialog pcpbd, CSTRING library);
extern void          ensureFinished(pCPlainBaseDialog pcpbd, RexxThreadContext *c, RexxObjectPtr abnormal);

// These functions are defined in oodBaseDialog.cpp
extern bool initWindowExtensions(RexxMethodContext *, RexxObjectPtr, HWND, pCWindowBase, pCPlainBaseDialog);
extern bool initCreateWindows(RexxMethodContext *c, RexxObjectPtr self, pCPlainBaseDialog pcpbd);
extern bool validControlDlg(RexxMethodContext *c, pCPlainBaseDialog pcpbd);
extern bool processOwnedDialog(RexxMethodContext *c, pCPlainBaseDialog pcpbd);
extern void setFontAttrib(RexxThreadContext *c, pCPlainBaseDialog pcpbd);
extern void customDrawCheckIDs(pCPlainBaseDialog pcpbd);

// These functions are defined in oodPropertySheet.cpp
extern void abortPropertySheet(pCPropertySheetDialog pcpsd, HWND hDlg, DlgProcErrType t);
extern void abortPropertySheetPage(pCPropertySheetPage page, HWND hDlg, DlgProcErrType t);
extern void abortOwnedDlg(pCPlainBaseDialog pcpbd, HWND hDlg, DlgProcErrType t);

// These functions are defined in oodControl.cpp.  TODO should be in
// oodControl.hpp, but the order of includes misses them.  Need to straighten
// out all these extern declarations.
extern const char     *controlType2controlName(oodControl_t control);
extern RexxObjectPtr   createControlFromHwnd(RexxMethodContext *, pCDialogControl, HWND, oodControl_t, bool);
extern RexxObjectPtr   createControlFromHwnd(RexxMethodContext *, pCPlainBaseDialog, HWND, oodControl_t, bool);
extern RexxObjectPtr   createControlFromHwnd(RexxThreadContext *, pCPlainBaseDialog, HWND, oodControl_t, bool);

// These functions are defined in oodViewControls.cpp
extern bool isInReportView(HWND hList);

// These functions are defined in oodToolBar.cpp
extern void putToolBarSymbols(RexxMethodContext *c, RexxDirectoryObject constDir);

// Shared button stuff.
typedef enum {push, check, radio, group, owner, notButton} BUTTONTYPE, *PBUTTONTYPE;
typedef enum {def, autoCheck, threeState, autoThreeState, noSubtype } BUTTONSUBTYPE, *PBUTTONSUBTYPE;

extern BUTTONTYPE getButtonInfo(HWND, PBUTTONSUBTYPE, DWORD *);

inline int32_t oodGlobalID(RexxMethodContext *c, RexxObjectPtr id, size_t argPosID, bool strict)
{
    return oodGlobalID(c->threadContext, id, argPosID, strict);
}

inline int32_t oodResolveSymbolicID(RexxMethodContext *c, RexxObjectPtr oodObj, RexxObjectPtr id,
                                    int posObj, size_t posID, bool strict)
{
    return oodResolveSymbolicID(c->threadContext, oodObj, id, posObj, posID, strict);
}

inline void safeLocalFree(void *p)
{
    if ( p != NULL && p != LPSTR_TEXTCALLBACK )
    {
        LocalFree(p);
    }
}

inline void safeFree(void *p)
{
    if ( p != NULL )
    {
        free(p);
    }
}

inline void safeDeleteObject(HANDLE h)
{
    if ( h != NULL )
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

inline bool isEmptyString(const char * s)
{
    return s != NULL && *s == '\0';
}

inline RexxObjectPtr convertToTrueOrFalse(RexxMethodContext *c, RexxObjectPtr obj)
{
    return convertToTrueOrFalse(c->threadContext, obj);
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
 * Determines if the currently executing thread is the same thread as the
 * dialog's message processing loop thread.
 *
 * @param pcpbd
 * @return bool
 */
inline bool isDlgThread(pCPlainBaseDialog pcpbd)
{
    return pcpbd->dlgProcThreadID == GetCurrentThreadId();
}

/**
 * Searches the tool tip table for the tool tip matching id.
 *
 * @param pcpbd
 * @param id
 *
 * @return  a pointer to the tool tip entry on success, null if there is no such
 *          tool tip.
 */
inline PTOOLTIPTABLEENTRY findToolTipForID(pCPlainBaseDialog pcpbd, uint32_t id)
{
    for ( register size_t i = 0; i < pcpbd->TTT_nextIndex; i++ )
    {
        if ( pcpbd->ToolTipTab[i].id == id )
        {
            return &pcpbd->ToolTipTab[i];
        }
    }
    return NULL;
}


extern void           ooDialogInternalException(RexxMethodContext *, char *, int, char *, char *);
extern void           systemServiceExceptionCode(RexxThreadContext *context, const char *msg, const char *arg1);
extern void           systemServiceExceptionComCode(RexxThreadContext *context, const char *msg, const char *arg1, HRESULT hr);
extern RexxObjectPtr  invalidCategoryPageException(RexxMethodContext *c, int, int);
extern RexxObjectPtr  noSuchPageException(RexxMethodContext *c, RexxObjectPtr page, size_t pos);
extern RexxObjectPtr  noWindowsPageException(RexxMethodContext *c, size_t pageID, size_t pos);
extern RexxObjectPtr  noSuchPageException(RexxMethodContext *c, int32_t id, uint32_t index);
extern void          *noWindowsPageDlgException(RexxMethodContext *c, size_t pos);
extern RexxObjectPtr  noSuchControlException(RexxMethodContext *c, int32_t id, RexxObjectPtr rxDlg, size_t pos);
extern RexxObjectPtr  controlNotSupportedException(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr rxDlg, size_t pos, RexxStringObject controlName);
extern void          *wrongClassReplyException(RexxThreadContext *c, const char *mName, const char *n);
extern void          *wrongReplyListException(RexxThreadContext *c, const char *mName, const char *list, RexxObjectPtr actual);
extern void          *wrongReplyMsgException(RexxThreadContext *c, const char *mName, const char *msg);
extern void          *wrongReplyRangeException(RexxThreadContext *c, const char *mName, int32_t min, int32_t max, RexxObjectPtr actual);
extern void          *wrongReplyNotBooleanException(RexxThreadContext *c, const char *mName, RexxObjectPtr actual);
extern void           stringTooLongReplyException(RexxThreadContext *c, CSTRING method, size_t len, size_t realLen);
extern void           controlFailedException(RexxThreadContext *, CSTRING, CSTRING, CSTRING);
extern void           wrongWindowStyleException(RexxMethodContext *c, CSTRING, CSTRING);
extern void           bitmapTypeMismatchException(RexxMethodContext *c, CSTRING orig, CSTRING found, size_t pos);
extern void           customDrawMismatchException(RexxThreadContext *c, uint32_t id, oodControl_t type);
extern RexxObjectPtr  tooManyPagedTabsException(RexxMethodContext *c, uint32_t count, bool isPagedTab);
extern RexxObjectPtr  methodCanOnlyBeInvokedException(RexxMethodContext *c, CSTRING methodName, CSTRING msg, RexxObjectPtr rxObj);
extern RexxObjectPtr  methodCanNotBeInvokedException(RexxMethodContext *c, CSTRING methodName, RexxObjectPtr rxDlg, CSTRING msg);
extern RexxObjectPtr  methodCanNotBeInvokedException(RexxMethodContext *c, CSTRING methodName, CSTRING msg, RexxObjectPtr rxDlg);
extern RexxObjectPtr  invalidAttributeException(RexxMethodContext *c, RexxObjectPtr rxDlg);

extern pCPlainBaseDialog requiredDlgCSelf(RexxMethodContext *c, RexxObjectPtr self, oodClass_t type, size_t argPos, pCDialogControl *ppcdc);
extern pCDialogControl   requiredDlgControlCSelf(RexxMethodContext *c, RexxObjectPtr control, size_t argPos);
extern pCPlainBaseDialog requiredPlainBaseDlg(RexxMethodContext *c, RexxObjectPtr dlg, size_t argPos);

extern uint32_t          keyword2stateSystem(CSTRING flags);
extern RexxStringObject  stateSystem2keyword(RexxMethodContext *c, uint32_t flags);

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the "msg"
 *
 *  The connectEdit method can not be invoked on a StyleDlg when the Windows
 *  dialog does not exist.
 *
 * @param c
 * @param rxDlg
 * @param msg
 */
inline RexxObjectPtr methodCanNotBeInvokedException(RexxMethodContext *c, RexxObjectPtr rxDlg, CSTRING msg)
{
    return methodCanNotBeInvokedException(c, c->GetMessageName(), msg, rxDlg);
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the Windows
 *  dialog does not exist.
 *
 *  The connectEdit method can not be invoked on a StyleDlg when the Windows
 *  dialog does not exist.
 *
 * @param c
 * @param rxDlg
 */
inline RexxObjectPtr noWindowsDialogException(RexxMethodContext *c, RexxObjectPtr rxDlg)
{
    return methodCanNotBeInvokedException(c, rxDlg, "Windows dialog does not exist");
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the Windows
 *  dialog does not exist.
 *
 *  The createToolTip method can not be invoked on a StyleDlg when the Windows
 *  dialog does not exist.
 *
 * @param c
 * @param rxDlg
 */
inline RexxObjectPtr noWindowsDialogException(RexxMethodContext *c, CSTRING methName, RexxObjectPtr rxDlg)
{
    return methodCanNotBeInvokedException(c, methName, rxDlg, "Windows dialog does not exist");
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the parent
 *  Windows dialog does not exist.
 *
 *  The STARTDIALOG method can not be invoked on an AddressDlg when the parent
 *  Windows dialog does not exist.
 *
 * @param c
 * @param rxDlg
 */
inline RexxObjectPtr noParentWindowsDialogException(RexxMethodContext *c, RexxObjectPtr rxDlg)
{
    return methodCanNotBeInvokedException(c, rxDlg, "parent Windows dialog does not exist");
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the parent
 *  Rexx dialog has not been assigned.
 *
 *  The connectEdit method can not be invoked on a StyleDlg when the parent
 *  Rexx dialog has not been assigned.
 *
 * @param c
 * @param rxDlg
 */
inline RexxObjectPtr noOwnerRexxDialogException(RexxMethodContext *c, RexxObjectPtr rxDlg)
{
    return methodCanNotBeInvokedException(c, rxDlg, "parent Rexx dialog has not been assigned");
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the window
 *  handle is not valid.
 *
 *  The getMaxSelection method can not be invoked on a MonthCalendar when the
 *  window handle is not valid.
 *
 * @param c
 * @param rxObj
 */
inline RexxObjectPtr invalidWindowException(RexxMethodContext *c, RexxObjectPtr rxObj)
{
    return methodCanNotBeInvokedException(c, rxObj, "window handle is not valid");
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the tool tip
 *  control does not exist.
 *
 *  The connectToolTipEvent method can not be invoked on a SimpleDialog when
 *  the tool tip control does not exist.
 *
 * @param c
 * @param rxObj
 */
inline RexxObjectPtr noToolTipException(RexxMethodContext *c, RexxObjectPtr rxObj)
{
    return methodCanNotBeInvokedException(c, rxObj, "tool tip control does not exist");
}

/**
 * Ensures that a dialog CSelf pointer is not null.  This can happen if the user
 * invokes a method in a dialog subclass before invoking the superclass init()
 * method.
 *
 * Unfortunately users do this. ;-(  Before the conversion to the C++ API,
 * things were not working as the user expected, but no "real bad" things
 * happened.  Now, this causes a null point derefernce unless we check first.
 *
 * @param c       Method context we are operating.
 * @param pCSelf  CSelf pointer for the dialog object.
 *
 * @return The CSelf pointer cast to a pCPlainBaseDialog, or null if pCSelf is
 *         null.
 *
 * @note  The whole point of this is to raise the exception if pCSelf is null.
 */
static inline pCPlainBaseDialog getPBDCSelf(RexxMethodContext *c, void * pCSelf)
{
    if ( pCSelf == NULL )
    {
        baseClassInitializationException(c);
    }
    return (pCPlainBaseDialog)pCSelf;
}

/**
 * Retrieves the PlainBaseDialog class CSelf pointer.
 *
 * @param c  Method contex we are operating in.
 *
 * @return The pointer to the CPlainBaseDialogClass struct.
 */
inline pCPlainBaseDialogClass dlgToClassCSelf(RexxMethodContext *c)
{
    return (pCPlainBaseDialogClass)c->ObjectToCSelf(ThePlainBaseDialogClass);
}

/**
 * Retrieves the CSelf pointer for a dialog object when the dialog object is not
 * the direct object the method was invoked on.  This performs a scoped CSelf
 * lookup.
 *
 * @param c    The method (or thread) context we are operating in.
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
inline pCPlainBaseDialog dlgToCSelf(RexxThreadContext *c, RexxObjectPtr dlg)
{
    return (pCPlainBaseDialog)c->ObjectToCSelf(dlg, ThePlainBaseDialogClass);
}

/**
 * Retrieves the PropertySheetPage CSelf pointer for a dialog object when the
 * dialog object is not the direct object the method was invoked on.  This
 * performs a scoped CSelf lookup.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose PropertySheetPage CSelf pointer is
 *             needed.
 *
 * @return A pointer to the PropertySheetPage CSelf of the dlg object.
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx
 *           PropertySheetPage dialog object.
 */
inline pCPropertySheetPage dlgToPSPCSelf(RexxMethodContext *c, RexxObjectPtr dlg)
{
    return (pCPropertySheetPage)c->ObjectToCSelf(dlg, ThePropertySheetPageClass);
}

/**
 * Retrieves the ControlDialog CSelf pointer for a dialog object when the
 * dialog object is not the direct object the method was invoked on.  This
 * performs a scoped CSelf lookup.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose ControlDialog CSelf pointer is needed.
 *
 * @return A pointer to the ControlDialog CSelf of the dlg object.
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx ControlDialog
 *           dialog object.
 */
inline pCControlDialog dlgToCDCSelf(RexxMethodContext *c, RexxObjectPtr dlg)
{
    return (pCControlDialog)c->ObjectToCSelf(dlg, TheControlDialogClass);
}

/**
 * Retrieves the window handle for a property sheet page from an ooDialog dialog
 * object which was not the direct object the method was invoked on.  This
 * performs a scoped CSelf lookup.
 *
 * @param c    The method context we are operating in.
 * @param dlg  The dialog object whose window handle is needed.
 *
 * @return The window handle of the dialog.  Note that this will be null if the
 *         underlying Windows dialog has not yet been created.
 *
 * @assumes  The caller has ensured dlg is in fact a ooDialog Rexx property
 *           page object.
 */
inline HWND dlgToPSPHDlg(RexxMethodContext *c, RexxObjectPtr dlg)
{
    pCPropertySheetPage pcpsp = dlgToPSPCSelf(c, dlg);
    return pcpsp->hPage;
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
inline pCDialogControl controlToCSelf(RexxThreadContext *c, RexxObjectPtr ctrl)
{
    return (pCDialogControl)c->ObjectToCSelf(ctrl, TheDialogControlClass);
}
inline pCDialogControl controlToCSelf(RexxMethodContext *c, RexxObjectPtr ctrl)
{
    return controlToCSelf(c->threadContext, ctrl);
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


#endif

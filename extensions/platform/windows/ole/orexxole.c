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
//******************************************************************************
// Rexx OLE Automation Support                                    orexxole.c
//
// Methods for the OLEObject class (see OREXXOLE.CMD)
//******************************************************************************

#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <dispex.h>

#include "oorexxapi.h"
#include "events.h"


//******************************************************************************
// global data
//******************************************************************************
BOOL            fInitialized = FALSE;
CHAR            szDbg[255];
PPOLECLASSINFO  ppClsInfo = NULL;
PTYPELIBLIST    pTypeLibList = NULL;
int             iClsInfoSize = 0;
int             iClsInfoUsed = 0;

long iInstanceCount = 0;    // count number of created OLE objects

typedef enum {FailureReturn, SuccessReturn, ExceptionReturn} ThreeStateReturn;

//******************************************************************************
// function prototypes for local functions
//******************************************************************************
void *ORexxOleAlloc(size_t iSize);
void *ORexxOleReAlloc(void * ptr, size_t iSize);
void ORexxOleFree(void *ptr);

LPOLESTR lpAnsiToUnicode(LPCSTR pszA, size_t length);
LPOLESTR lpAnsiToUnicodeLength(LPCSTR pszA, size_t length, size_t *outLength);
PSZ pszUnicodeToAnsi(LPOLESTR pszU);

PSZ pszStringDupe(const char *pszOrig);

bool psFindClassInfo(RexxThreadContext *, const char *, ITypeInfo *, POLECLASSINFO *);
VOID ClearClassInfoBlock(POLECLASSINFO pClsInfo);
POLEFUNCINFO AddFuncInfoBlock(POLECLASSINFO, MEMBERID, INVOKEKIND, VARTYPE, int, int, PSZ);
POLECONSTINFO AddConstInfoBlock(POLECLASSINFO, MEMBERID, PSZ, VARIANT *);

BOOL fFindFunction(const char *, IDispatch *, IDispatchEx *, ITypeInfo *, POLECLASSINFO, unsigned short,PPOLEFUNCINFO, MEMBERID *, size_t);
BOOL fFindConstant(const char *pszConstName, POLECLASSINFO pClsInfo, PPOLECONSTINFO ppConstInfo );

RexxObjectPtr Variant2Rexx(RexxThreadContext *context, VARIANT *pVariant);
bool Rexx2Variant(RexxThreadContext *context, RexxObjectPtr RxObject, VARIANT *pVariant, VARTYPE DestVt, size_t iArgPos);
bool createEmptySafeArray(RexxThreadContext *, VARIANT *);
bool RexxArray2SafeArray(RexxThreadContext *context, RexxObjectPtr RxArray, VARIANT *VarArray, size_t iArgPos);
BOOL fExploreTypeAttr( ITypeInfo *pTypeInfo, TYPEATTR *pTypeAttr, POLECLASSINFO pClsInfo );
VARTYPE getUserDefinedVT( ITypeInfo *pTypeInfo, HREFTYPE hrt );
BOOL fExploreTypeInfo( ITypeInfo *pTypeInfo, POLECLASSINFO pClsInfo );
ThreeStateReturn checkForOverride(RexxThreadContext *, VARIANT *, RexxObjectPtr , VARTYPE, RexxObjectPtr *, VARTYPE * );
BOOL isOutParam(RexxThreadContext *, RexxObjectPtr , POLEFUNCINFO, size_t);
VOID handleVariantClear(RexxMethodContext *, VARIANT *, RexxObjectPtr  );
__inline BOOL okayToClear(RexxMethodContext *, RexxObjectPtr  );
static void formatDispatchException(EXCEPINFO *, char *);
void sendOutArgBackToRexx(RexxObjectPtr, VARIANT *);
bool getClassInfo(IDispatch *, ITypeInfo **);
bool getConnectionPointContainer(IDispatch *, IConnectionPointContainer **);
bool eventTypeInfoFromCoClass(ITypeInfo *, ITypeInfo **);
bool getClassInfoFromCLSID(ITypeInfo *, CLSID *, ITypeInfo **);
bool addEventHandler(RexxMethodContext *, RexxObjectPtr, bool, IDispatch *, POLECLASSINFO, ITypeInfo *, CLSID *);
inline bool haveEventHandler(RexxMethodContext *);
inline bool connectedToEvents(RexxMethodContext *);
void GUIDFromTypeInfo(ITypeInfo *pTypeInfo, GUID *guid);
bool getClassInfoFromTypeInfo(ITypeInfo *p, ITypeInfo **);
bool isImplementedInterface(ITypeInfo *, GUID *);
bool getEventTypeInfo(IDispatch *, ITypeInfo *, CLSID *, ITypeInfo **);
bool createEventHandler(RexxMethodContext *, ITypeInfo *, RexxObjectPtr, POLECLASSINFO, OLEObjectEvent **);
bool connectEventHandler(RexxMethodContext *, IConnectionPointContainer *, OLEObjectEvent *);
void releaseEventHandler(RexxMethodContext *);
void disconnectEventHandler(RexxMethodContext *);
void getClsIDFromString(const char *, CLSID *);
bool maybeCreateEventHandler(RexxMethodContext *, OLEObjectEvent **, IConnectionPointContainer **, RexxObjectPtr);
bool isConnectableObject(IDispatch *);
static BOOL getIDByName(const char *, IDispatch *, IDispatchEx *, MEMBERID *);

int (__stdcall *creationCallback)(CLSID, IUnknown*) = NULL;
void setCreationCallback(int (__stdcall *f)(CLSID, IUnknown*))
{
  creationCallback = f;
}


//******************************************************************************
// debugging functions implementation
//******************************************************************************

// Function to print out the string representation of a GUID.
void dbgPrintGUID( IID *pGUID )
{
  OLECHAR     oleBuffer[100];

  StringFromGUID2( *pGUID, oleBuffer, sizeof(oleBuffer) );

  wprintf( L"%s\n", oleBuffer );
}


PSZ pszDbgInvkind(INVOKEKIND invkind)
{
    szDbg[0] = 0;

    if (invkind & INVOKE_FUNC)
    {
        strcat(szDbg, "INVOKE_FUNC ");
    }
    if (invkind & INVOKE_PROPERTYGET)
    {
        strcat(szDbg, "INVOKE_PROPERTYGET ");
    }
    if (invkind & INVOKE_PROPERTYPUT)
    {
        strcat(szDbg, "INVOKE_PROPERTYPUT ");
    }
    if (invkind & INVOKE_PROPERTYPUTREF)
    {
        strcat(szDbg, "INVOKE_PROPERTYPUTREF");
    }

    if (szDbg[strlen(szDbg)-1] == ' ')
    {
        szDbg[strlen(szDbg)-1] = 0;
    }
    return szDbg;
}


PSZ pszDbgTypekind(TYPEKIND typeKind)
{
    szDbg[0] = 0;

    switch ( typeKind )
    {
        case TKIND_ENUM:
            strcpy(szDbg, "TKIND_ENUM");
            break;
        case TKIND_RECORD:
            strcpy(szDbg, "TKIND_RECORD");
            break;
        case TKIND_MODULE:
            strcpy(szDbg, "TKIND_MODULE");
            break;
        case TKIND_INTERFACE:
            strcpy(szDbg, "TKIND_INTERFACE");
            break;
        case TKIND_DISPATCH:
            strcpy(szDbg, "TKIND_DISPATCH");
            break;
        case TKIND_COCLASS:
            strcpy(szDbg, "TKIND_COCLASS");
            break;
        case TKIND_ALIAS:
            strcpy(szDbg, "TKIND_ALIAS");
            break;
        case TKIND_UNION:
            strcpy(szDbg, "TKIND_UNION");
            break;
    }

    return szDbg;
}


PSZ pszDbgParmFlags(unsigned short pf)
{
    szDbg[0] = 0;

    if (pf == PARAMFLAG_NONE)
    {
        strcat(szDbg, "PARAMFLAG_NONE");
    }
    if (pf & PARAMFLAG_FIN)
    {
        strcat(szDbg, "PARAMFLAG_FIN ");
    }
    if (pf & PARAMFLAG_FOUT)
    {
        strcat(szDbg, "PARAMFLAG_FOUT ");
    }
    if (pf & PARAMFLAG_FLCID)
    {
        strcat(szDbg, "PARAMFLAG_FLCID ");
    }
    if (pf & PARAMFLAG_FRETVAL)
    {
        strcat(szDbg, "PARAMFLAG_FRETVAL ");
    }
    if (pf & PARAMFLAG_FOPT)
    {
        strcat(szDbg, "PARAMFLAG_FOPT ");
    }
    if (pf & PARAMFLAG_FHASDEFAULT)
    {
        strcat(szDbg, "PARAMFLAG_FHASDEFAULT ");
    }

    if (szDbg[strlen(szDbg)-1] == ' ')
    {
        szDbg[strlen(szDbg)-1] = 0;
    }
    return szDbg;
}


PSZ pszDbgVarType(VARTYPE vt)
{
    szDbg[0] = 0;

    if (vt == VT_ILLEGAL)
    {
        strcat(szDbg, "VT_ILLEGAL");
        return szDbg;
    }

    if (vt & VT_VECTOR)
        strcat(szDbg, "VT_VECTOR ");
    if (vt & VT_ARRAY)
        strcat(szDbg, "VT_ARRAY ");
    if (vt & VT_BYREF)
        strcat(szDbg, "VT_BYREF ");
    if (vt & VT_RESERVED)
        strcat(szDbg, "VT_RESERVED ");

    vt &= VT_TYPEMASK;
    switch (vt)
    {
        case VT_EMPTY:
            strcat(szDbg, "VT_EMPTY");
            break;
        case VT_NULL:
            strcat(szDbg, "VT_NULL");
            break;
        case VT_I2:
            strcat(szDbg, "VT_I2");
            break;
        case VT_I4:
            strcat(szDbg, "VT_I4");
            break;
        case VT_R4:
            strcat(szDbg, "VT_R4");
            break;
        case VT_R8:
            strcat(szDbg, "VT_R8");
            break;
        case VT_CY:
            strcat(szDbg, "VT_CY");
            break;
        case VT_DATE:
            strcat(szDbg, "VT_DATE");
            break;
        case VT_BSTR:
            strcat(szDbg, "VT_BSTR");
            break;
        case VT_DISPATCH:
            strcat(szDbg, "VT_DISPATCH");
            break;
        case VT_ERROR:
            strcat(szDbg, "VT_ERROR");
            break;
        case VT_BOOL:
            strcat(szDbg, "VT_BOOL");
            break;
        case VT_VARIANT:
            strcat(szDbg, "VT_VARIANT");
            break;
        case VT_UNKNOWN:
            strcat(szDbg, "VT_UNKNOWN");
            break;
        case VT_DECIMAL:
            strcat(szDbg, "VT_DECIMAL");
            break;
        case VT_I1:
            strcat(szDbg, "VT_I1");
            break;
        case VT_UI1:
            strcat(szDbg, "VT_UI1");
            break;
        case VT_UI2:
            strcat(szDbg, "VT_UI2");
            break;
        case VT_UI4:
            strcat(szDbg, "VT_UI4");
            break;
        case VT_I8:
            strcat(szDbg, "VT_I8");
            break;
        case VT_UI8:
            strcat(szDbg, "VT_UI8");
            break;
        case VT_INT:
            strcat(szDbg, "VT_INT");
            break;
        case VT_UINT:
            strcat(szDbg, "VT_UINT");
            break;
        case VT_VOID:
            strcat(szDbg, "VT_VOID");
            break;
        case VT_HRESULT:
            strcat(szDbg, "VT_HRESULT");
            break;
        case VT_PTR:
            strcat(szDbg, "VT_PTR");
            break;
        case VT_SAFEARRAY:
            strcat(szDbg, "VT_SAFEARRAY");
            break;
        case VT_CARRAY:
            strcat(szDbg, "VT_CARRAY");
            break;
        case VT_USERDEFINED:
            strcat(szDbg, "VT_USERDEFINED");
            break;
        case VT_LPSTR:
            strcat(szDbg, "VT_LPSTR");
            break;
        case VT_LPWSTR:
            strcat(szDbg, "VT_LPWSTR");
            break;
        case VT_FILETIME:
            strcat(szDbg, "VT_FILETIME");
            break;
        case VT_BLOB:
            strcat(szDbg, "VT_BLOB");
            break;
        case VT_STREAM:
            strcat(szDbg, "VT_STREAM");
            break;
        case VT_STORAGE:
            strcat(szDbg, "VT_STORAGE");
            break;
        case VT_STREAMED_OBJECT:
            strcat(szDbg, "VT_STREAMED_OBJECT");
            break;
        case VT_STORED_OBJECT:
            strcat(szDbg, "VT_STORED_OBJECT");
            break;
        case VT_BLOB_OBJECT:
            strcat(szDbg, "VT_BLOB_OBJECT");
            break;
        case VT_CF:
            strcat(szDbg, "VT_CF");
            break;
        case VT_CLSID:
            strcat(szDbg, "VT_CLSID");
            break;
        default:
            strcat(szDbg, "unknown vt");
    }

    return szDbg;
}


PSZ pszDbgVariant(VARIANT *pVar)
{
    CHAR      szValue[2000];
    VARIANT   sStrArg;

    pszDbgVarType(V_VT(pVar));
    strcat(szDbg, " -> ");
    szValue[0] = 0;

    if (V_VT(pVar) & VT_ARRAY)
    {
        /* this is an array of variants */
        SAFEARRAY  *pSafeArray;
        LONG       lDimensions;
        LONG       lCurrDim;
        LONG       lLowBound;
        LONG       lUpperBound;
        CHAR       szDim[80];

        pSafeArray = V_ARRAY(pVar);
        lDimensions = SafeArrayGetDim(pSafeArray);
        if (lDimensions == 1)
        {
            SafeArrayGetLBound(pSafeArray, 1, &lLowBound);
            SafeArrayGetUBound(pSafeArray, 1, &lUpperBound);
            sprintf(szValue, "Singledimensional array[%d...%d]", lLowBound, lUpperBound);
        }
        else
        {
            sprintf(szValue, "%d-dimensional array[", lDimensions);
            for (lCurrDim = 1; lCurrDim <= lDimensions; lCurrDim++)
            {
                SafeArrayGetLBound(pSafeArray, lCurrDim, &lLowBound);
                SafeArrayGetUBound(pSafeArray, lCurrDim, &lUpperBound);
                sprintf(szDim, "%d...%d", lLowBound, lUpperBound);

                if (lCurrDim < lDimensions)
                    strcat(szDim, ",");
                else
                    strcat(szDim, "]");

                strcat(szValue, szDim);
            }
        }
    }
    else
    {
        /* a single type variant */
        VariantInit(&sStrArg);
        if (! (V_VT(pVar) & VT_BYREF) )
        {
            if (VariantChangeType(&sStrArg, pVar, 0, VT_BSTR) == S_OK)
                sprintf(szValue, "%.32S", V_BSTR(&sStrArg));  // cut off after 32 characters
            else
                strcpy(szValue, "<impossible conversion>");
        }
        VariantClear(&sStrArg);
    }

    strcat(szDbg, szValue);
    return szDbg;
}

//******************************************************************************
// type library list
//******************************************************************************
void addTypeListElement(GUID *pGUID, POLECONSTINFO infoPtr)
{
    if (pTypeLibList)
    {
        PTYPELIBLIST newEntry = (PTYPELIBLIST) ORexxOleAlloc(sizeof(TYPELIBLIST));
        newEntry->next = pTypeLibList;
        newEntry->info = infoPtr;
        memcpy(&(newEntry->guid),pGUID,sizeof(GUID));
        pTypeLibList = newEntry;
    }
    else
    {
        pTypeLibList = (PTYPELIBLIST) ORexxOleAlloc(sizeof(TYPELIBLIST));
        pTypeLibList->next = NULL;
        pTypeLibList->info = infoPtr;
        memcpy(&(pTypeLibList->guid),pGUID,sizeof(GUID));
    }
}

POLECONSTINFO fFindConstantInTypeLib(GUID *pGUID)
{
    POLECONSTINFO fFound = NULL;
    PTYPELIBLIST entry = pTypeLibList;

    while (!fFound && entry)
    {
        if (memcmp(pGUID,&(entry->guid),sizeof(GUID)) == 0)
        {
            fFound = entry->info;
            break;
        }
        else
        {
            entry = entry->next;
        }
    }

    return fFound;
}


void destroyTypeLibList()
{
    while (pTypeLibList)
    {
        PTYPELIBLIST  pNext = pTypeLibList->next;
        POLECONSTINFO pCurrConst, pNextConst;

        pCurrConst = pTypeLibList->info;
        while ( pCurrConst )
        {
            pNextConst = pCurrConst->pNext;

            ORexxOleFree( pCurrConst->pszConstName );
            VariantClear( &(pCurrConst->sValue) );
            ORexxOleFree( pCurrConst );
            pCurrConst = pNextConst;
        }

        ORexxOleFree(pTypeLibList);

        pTypeLibList = pNext;
    }
}

//******************************************************************************
// Memory allocation functions implementation
//******************************************************************************
void *ORexxOleAlloc(size_t iSize)
{
    void  *ptr = NULL;   // default value

    if (iSize)
    {         // only allocate when needed!
        ptr = calloc(iSize,1);
    }
    return ptr;
}


void *ORexxOleReAlloc(void *ptr, size_t iSize)
{
    return realloc( ptr, iSize );
}


void ORexxOleFree(void *ptr)
{
    if ( ptr )
    {
        free( ptr );
    }
}


//******************************************************************************
// OLE function implementation
//******************************************************************************

// ENG: Made a change in architecture, because the use of ~GetObject(...) caused
//      Access Violations (at the end of the REXX script) with
//      "winmgmts:{impersonationLevel=impersonate}".
//      I believe the cause to be the way OleUninitialize() was called (in
//      DllMain(...)). I found this in the MSDN documentation:
//
// "Warning: On attach, the body of your DLL entry-point function should perform
//  only simple initialization tasks, such as setting up thread local storage (TLS),
//  creating synchronization objects, and opening files. You must not call
//  LoadLibrary in the entry-point function, because you may create dependency
//  loops in the DLL load order. This can result in a DLL being used before the
//  system has executed its initialization code. Similarly, you must not call the
//  FreeLibrary function in the entry-point function on detach, because this can
//  result in a DLL being used after the system has executed its termination code.
//
//  Calling Win32 functions other than TLS, synchronization, and file functions may
//  result in problems that are difficult to diagnose. For example, calling User,
//  Shell, COM, RPC, and Windows Sockets functions (or any functions that call these
//         ^^^ (! aka: OLE)
//  functions) can cause access violation errors, because their DLLs call LoadLibrary
//  to load other system components."
//
// I have therefore introduced an instance count for the OLE objects and removed
// the OleInitialize() from DllMain; OleInitialize() will be called in the INIT
// method if there are no instances yet. OleUninitialize() will be called in the
// UNINIT method, if this is the last one.
// Problem: As objects might "die" and their UNINIT won't be called then, (as a sort
// of "desperate measure") OleUninitialize() will be called when a PROCESS_DETACH is
// sent to DllMain and the instance count is NOT zero. This might still cause trouble.
//
// August 2008.  And in fact it does cause trouble.  The consequences of calling
// OleUninitialize() in the DLL entry-point function (a crash or a process hang)
// are worse than the consequences of not calling it (a possible memory leak.)

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch ( ul_reason_for_call )
    {
        case DLL_PROCESS_DETACH:
            if ( iInstanceCount > 0 )
            {
                printf("Warning: orexxole.dll DLL process detach, instance count (%d) not 0\n", iInstanceCount);
                // destroyTypeLibList();
                // OleUninitialize();
            }
            // free class info memory [not a real leak]  not really needed,
            // since the process will go away and release that memory, but it is
            // cleaner this way
            if ( ppClsInfo )
            {
                for ( int q = 0; q < iClsInfoUsed; q++ )
                {
                    if ( ppClsInfo[q] )
                    {
                        ORexxOleFree(ppClsInfo[q]);
                    }
                }
                ORexxOleFree(ppClsInfo);
                ppClsInfo = NULL;
                iClsInfoSize = iClsInfoUsed = 0;
            }
            break;
    }

    return TRUE;
}

/**
 * Retrieve the stored class information and / or the ITypeInfo pointer for this
 * object.
 *
 * @param context   The ooRexx method context.
 * @param pClsInfo  The returned class information.
 * @param pTypeInfo The returned type information.
 *
 * @note  When psFindClassInfo() fails to find the object it is searching for,
 *        it will allocate a new class info structure.  We don't want that here,
 *        but we also don't have to worry about it.  If there is an entry for
 *        !CLSID or for !TYPEINFO, then there is always a class info block
 *        allocated for that entry.
 *
 *        For that reason, the return from psFindClassInfo() does not need to be
 *        checked.  False is only returned on a memory allocation error and
 *        there will be no attempt to allocate memory when called from this
 *        function.
 */
void getCachedClassInfo(RexxMethodContext *context, POLECLASSINFO *pClsInfo, ITypeInfo **pTypeInfo)
{
    // first try the lookup using the CLSID
    RexxObjectPtr value = context->GetObjectVariable("!CLSID");
    if ( value != NULLOBJECT )
    {
        psFindClassInfo(context->threadContext, context->ObjectToStringValue(value), NULL, pClsInfo);
        // if we have class info, we can retrieve the type info
        if ( *pClsInfo != NULL )
        {
            *pTypeInfo = (*pClsInfo)->pTypeInfo;
        }
    }
    else
    {
        // No CLSID, try using the type info pointer for the lookup
        value = context->GetObjectVariable("!ITYPEINFO");
        if ( value != NULLOBJECT )
        {
            *pTypeInfo = (ITypeInfo *)context->PointerValue((RexxPointerObject)value);
            // and hopefully from the type info, we can get the class info
            if ( *pTypeInfo != NULL )
            {
                psFindClassInfo(context->threadContext, NULL, *pTypeInfo, pClsInfo);
            }
        }
    }
}

/**
 * Retrieve the dispatch pointer for an object.
 *
 * @param   pDispatch The returned IDispatch pointer
 * @retunr  True on success, false if an exception is raised.
 */
bool getDispatchPtr(RexxMethodContext *context, IDispatch **pDispatch)
{
    /* Get the IDispatch pointer for the OLE object we represent. */
    RexxObjectPtr value = context->GetObjectVariable("!IDISPATCH");
    if ( value != NULLOBJECT )
    {
        *pDispatch = (IDispatch *)context->PointerValue((RexxPointerObject)value);
    }
    // we must have this
    if (*pDispatch == NULL)
    {
        context->RaiseException0(Rexx_Error_Interpretation_initialization);
        return false;
    }
    return true;
}

/**
 * Retrieve the event hanlder pointer for this object, if there is one.
 *
 * @param ppHandler The returned OLEObjectEvent pointer.  This is set to null if
 *                  there is no event handler, or some other unanticipated error
 *                  occurs.
 */
void getEventHandlerPtr(RexxMethodContext *context, OLEObjectEvent **ppHandler)
{
    RexxObjectPtr ptr = context->GetObjectVariable("!EVENTHANDLER");
    if ( ptr != NULLOBJECT )
    {
        *ppHandler = (OLEObjectEvent *)context->PointerValue((RexxPointerObject)ptr);
    }
    else
    {
        *ppHandler = NULL;
    }
}

void OLEInit()
{
    fInitialized = TRUE;
}


LPOLESTR lpAnsiToUnicode(LPCSTR pszA, size_t iInputLength)
{
    return lpAnsiToUnicodeLength(pszA, iInputLength, NULL);
}


LPOLESTR lpAnsiToUnicodeLength(LPCSTR pszA, size_t iInputLength, size_t *outLength)
{
    size_t        iNewLength;
    LPOLESTR      lpOleStr = NULL;

    /* Special case the empty string, MultiByteToWideChar does not handle */
    /* an input length of 0.  Create a true Unicode empty string. */
    if ( iInputLength == 0 && pszA && *pszA == 0 )
    {
        lpOleStr = (LPOLESTR) ORexxOleAlloc(2);
        if ( outLength )
        {
            *outLength = 0;
        }
    }
    else
    {
        iNewLength = MultiByteToWideChar( CP_ACP, 0, pszA, (int)iInputLength, NULL, 0 );

        if ( iNewLength )
        {
            lpOleStr = (LPOLESTR) ORexxOleAlloc(iNewLength * 2 + 2);
            if (lpOleStr)
            {
                if ( MultiByteToWideChar( CP_ACP, 0, pszA, (int)iInputLength, lpOleStr, (int)iNewLength ) == 0)
                {
                    /* conversion failed */
                    ORexxOleFree(lpOleStr);
                    lpOleStr = NULL;
                }

                if ( outLength )
                {
                    *outLength = iNewLength;
                }
            }
        }
    }

    return lpOleStr;
}


PSZ pszUnicodeToAnsi(LPOLESTR pszU)
{
    size_t iNewLength;
    size_t iInputLength;
    PSZ   pszAnsiStr = NULL;

    if (pszU == NULL)
    {
        return NULL;
    }

    iInputLength = wcslen(pszU) + 1;
    iNewLength = WideCharToMultiByte(CP_ACP, 0, pszU, (int)iInputLength, NULL, 0, NULL, NULL);
    if ( iNewLength )
    {
        pszAnsiStr = (PSZ) ORexxOleAlloc(iNewLength + 1);
        if (pszAnsiStr)
        {
            if ( WideCharToMultiByte(CP_ACP, 0, pszU, (int)iInputLength, pszAnsiStr, (int)iNewLength, NULL, NULL) == 0)
            {
                /* conversion failed */
                ORexxOleFree(pszAnsiStr);
                pszAnsiStr = NULL;
            }
        }
    }

    return pszAnsiStr;
}


PSZ pszStringDupe(const char *pszOrig)
{
    PSZ   pszCopy = NULL;

    if ( pszOrig )
    {
        pszCopy = (PSZ) ORexxOleAlloc(strlen(pszOrig) + 1);
        if ( pszCopy )
        {
            strcpy(pszCopy, pszOrig);
        }
    }

    return pszCopy;
}


char *prxStringDupe(const char * pszOrig, size_t length)
{
    PSZ   pszCopy = NULL;

    if ( pszOrig )
    {
        pszCopy = (PSZ) ORexxOleAlloc(length + 1);
        if ( pszCopy )
        {
            strcpy(pszCopy, pszOrig);
        }
    }

    return pszCopy;
}

/**
 * Returns a class information block for a COM class.  The COM class is uniquely
 * identified by either its CLSID or by its type info pointer.  If a class
 * information block for the specified COM class does not currently exist, a new
 * block is allocated.
 *
 * Note that this function only fails on a memory allocation error, in which
 * case an exception is raised.
 *
 * @param context    Used to raise an exception if needed.
 * @param pszCLSId   The COM class CLSID, may be null if pTypeInfo is not null.
 * @param pTypeInfo  The COM class type info pointer, may be null.
 * @param pClsInfo   The returned pointer to a class information block.
 *
 * @return False if an exception is raised, otherwise true.
 */
bool psFindClassInfo(RexxThreadContext *context, const char *pszCLSId, ITypeInfo *pTypeInfo,
                     POLECLASSINFO *pClsInfo)
{
    int iIdx;
    int iFound = -1;

    if ( iClsInfoSize == 0 )
    {
        // First time through, need to allocate the table.
        ppClsInfo = (PPOLECLASSINFO)ORexxOleAlloc(sizeof(POLECLASSINFO) * 10);
        if ( ppClsInfo == NULL )
        {
            context->RaiseException0(Rexx_Error_System_resources);
            return false;
        }
        iClsInfoSize = 10;
    }
    else
    {
        // See if an existing block matches the specified CLSID or pTypeInfo.
        for ( iIdx = 0; iIdx < iClsInfoUsed; iIdx++ )
        {
            if ( (pszCLSId && ppClsInfo[iIdx]->pszCLSId &&
                  (strcmp(pszCLSId, ppClsInfo[iIdx]->pszCLSId) == 0)) ||
                 (pTypeInfo && (pTypeInfo == ppClsInfo[iIdx]->pTypeInfo)) )
            {
                iFound = iIdx;
                break;
            }
        }
    }

    if ( iFound == -1 )
    {
        // Not found, look for an empty slot.
        for ( iIdx = 0; iIdx < iClsInfoUsed; iIdx++)
        {
            if ( ppClsInfo[iIdx]->fUsed == FALSE )
            {
                iFound = iIdx;
                ppClsInfo[iFound]->fUsed = TRUE;
                break;
            }
        }
    }

    if ( iFound == -1 )
    {
        // Still no slot found, allocate a new one.  First see if array needs to
        // be enlarged.
        if ( iClsInfoUsed >= iClsInfoSize )
        {
            PPOLECLASSINFO  ppNewBlock;
            ppNewBlock = (PPOLECLASSINFO)ORexxOleReAlloc(ppClsInfo, sizeof(POLECLASSINFO) * iClsInfoSize * 2);
            if ( ppNewBlock == NULL)
            {
                context->RaiseException0(Rexx_Error_System_resources);
                return false;
            }

            ppClsInfo = ppNewBlock;
            iClsInfoSize *= 2;
        }

        iFound = iClsInfoUsed;
        ppClsInfo[iFound] = (POLECLASSINFO)ORexxOleAlloc(sizeof(OLECLASSINFO));
        if ( ppClsInfo[iFound] == NULL)
        {
            context->RaiseException0(Rexx_Error_System_resources);
            return false;
        }

        ppClsInfo[iFound]->fUsed = TRUE;
        iClsInfoUsed++;
    }

    *pClsInfo = ppClsInfo[iFound];
    return true;
}


VOID ClearClassInfoBlock( POLECLASSINFO pClsInfo )
{
    POLEFUNCINFO    pCurrent, pNext;
/* only cleanup to avoid compiler warnings */
#if 0
    POLECONSTINFO   pCurrConst, pNextConst;
#endif

    /* free allocated strings */
    ORexxOleFree( pClsInfo->pszProgId );
    ORexxOleFree( pClsInfo->pszCLSId );

    /* free all function info blocks and the included names & parameters */
    pCurrent = pClsInfo->pFuncInfo;
    while ( pCurrent )
    {
        pNext = pCurrent->pNext;

        ORexxOleFree( pCurrent->pszFuncName );
        ORexxOleFree( pCurrent->pOptVt );
        ORexxOleFree( pCurrent->pusOptFlags );
        ORexxOleFree( pCurrent );
        pCurrent = pNext;
    }
/* will now be cleaned up in destroyTypeLibList() */
#if 0
    /* free all constant info blocks and the included constant names */
    pCurrConst = pClsInfo->pConstInfo;
    while ( pCurrConst )
    {
        pNextConst = pCurrConst->pNext;


        ORexxOleFree( pCurrConst->pszConstName );
        VariantClear( &(pCurrConst->sValue) );
        ORexxOleFree( pCurrConst );
        pCurrConst = pNextConst;
    }
#endif
    /* clear block */
    memset(pClsInfo, 0, sizeof(OLECLASSINFO));
    pClsInfo->fUsed = FALSE;
}


POLEFUNCINFO AddFuncInfoBlock( POLECLASSINFO pClsInfo, MEMBERID memId, INVOKEKIND invKind, VARTYPE funcVT,
                               int iParmCount, int iOptParms, PSZ pszFuncName )
{
    POLEFUNCINFO    pNewBlock = NULL;
    POLEFUNCINFO    pCurrBlock = NULL;

    pNewBlock = (POLEFUNCINFO) ORexxOleAlloc( sizeof(OLEFUNCINFO) );
    if ( pNewBlock )
    {
        pNewBlock->pNext = NULL;
        /* insert new block into class info structure, new block */
        /* will be added to the end of the linked list           */
        if ( pClsInfo->pFuncInfo )
        {
            /* list exists, add to end of list */
            pCurrBlock = pClsInfo->pFuncInfo;
            if ( (pCurrBlock->memId == memId) && (pCurrBlock->invkind == invKind) &&
                 (pCurrBlock->FuncVt == funcVT) && (pCurrBlock->iParmCount == iParmCount) &&
                 (pCurrBlock->iOptParms == iOptParms) && (pszFuncName != NULL) &&
                 (stricmp(pCurrBlock->pszFuncName, pszFuncName) == 0) )
            {
                /* same memberid found, don't store data */
                ORexxOleFree( pNewBlock );
                pNewBlock = NULL;
                return pNewBlock;
            }

            while ( pCurrBlock->pNext )
            {
                pCurrBlock = pCurrBlock->pNext;

                if ( (pCurrBlock->memId == memId) && (pCurrBlock->invkind == invKind) &&
                     (pCurrBlock->FuncVt == funcVT) && (pCurrBlock->iParmCount == iParmCount) &&
                     (pCurrBlock->iOptParms == iOptParms) && (pszFuncName != NULL) &&
                     (stricmp(pCurrBlock->pszFuncName, pszFuncName) == 0) )
                {
                    /* same memberid found, don't store data */
                    ORexxOleFree( pNewBlock );
                    pNewBlock = NULL;
                    return pNewBlock;
                }
            }

            /* add new block to end of list */
            pCurrBlock->pNext = pNewBlock;
        }
        else
        {
            /* list does not exist, this is the first element */
            pClsInfo->pFuncInfo = pNewBlock;
        }
    }

    return pNewBlock;
}


POLECONSTINFO AddConstInfoBlock( POLECLASSINFO pClsInfo, MEMBERID memId, PSZ pszConstName, VARIANT *pValue)
{
    POLECONSTINFO   pNewBlock = NULL;
    POLECONSTINFO   pCurrBlock = NULL;

    pNewBlock = (POLECONSTINFO) ORexxOleAlloc( sizeof(OLECONSTINFO) );
    if ( pNewBlock )
    {
        pNewBlock->pNext = NULL;

        /* insert new block into class info structure, new block */
        /* will be added to the end of the linked list           */
        if ( pClsInfo->pConstInfo )
        {
            /* list exists, add to end of list */
            pCurrBlock = pClsInfo->pConstInfo;
            if ( (pCurrBlock->memId == memId) && (pszConstName != NULL) &&
                 (stricmp(pCurrBlock->pszConstName, pszConstName) == 0) )
            {
                /* same memberid and name found, don't store data */
                ORexxOleFree( pNewBlock );
                pNewBlock = NULL;
                return pNewBlock;
            }

            while ( pCurrBlock->pNext )
            {
                pCurrBlock = pCurrBlock->pNext;

                if ( (pCurrBlock->memId == memId) && (pszConstName != NULL) &&
                     (stricmp(pCurrBlock->pszConstName, pszConstName) == 0) )
                {
                    /* same memberid and name found, don't store data */
                    ORexxOleFree( pNewBlock );
                    pNewBlock = NULL;
                    return pNewBlock;
                }
            }

            /* add new block to end of list */
            pCurrBlock->pNext = pNewBlock;
        }
        else
        {
            /* list does not exist, this is the first element */
            pClsInfo->pConstInfo = pNewBlock;
        }
    }

    return pNewBlock;
}


BOOL fFindFunction(const char *pszFunction, IDispatch *pDispatch, IDispatchEx *pDispatchEx,
                   ITypeInfo *pTypeInfo, POLECLASSINFO pClsInfo, unsigned short wFlags,
                   PPOLEFUNCINFO ppFuncInfo, MEMBERID *pMemId, size_t expectedArgCount )
{
    BOOL            fFound = FALSE;
    POLEFUNCINFO    pFuncInfo = NULL;
    POLEFUNCINFO    pFuncCache = NULL;

    /* initialize to not found entry */
    if (ppFuncInfo)
    {
        *ppFuncInfo = NULL;
    }

    if (pTypeInfo && !pDispatchEx)
    {
        /* find function in class info structure */
        pFuncInfo = pClsInfo->pFuncInfo;
        while ( pFuncInfo )
        {
            if (stricmp(pFuncInfo->pszFuncName, pszFunction) == 0)
            {
                /* ensure the right function description for property puts / gets */
                if ( wFlags == DISPATCH_PROPERTYPUT )
                {
                    if ( !((pFuncInfo->invkind & INVOKE_PROPERTYPUT) || (pFuncInfo->invkind & INVOKE_PROPERTYPUTREF)) )
                    {
                        pFuncInfo = pFuncInfo->pNext;
                        continue;
                    }

                    if (pFuncInfo->iParmCount + pFuncInfo->iOptParms >= (int)expectedArgCount)
                    {
                        fFound = TRUE;
                        *pMemId = pFuncInfo->memId;
                        if (ppFuncInfo)
                        {
                            *ppFuncInfo = pFuncInfo;
                        }
                        break; /* found correct function */
                    }
                }
                else
                {
                    if ( (pFuncInfo->invkind & INVOKE_PROPERTYPUT) || (pFuncInfo->invkind & INVOKE_PROPERTYPUTREF) )
                    {
                        pFuncInfo = pFuncInfo->pNext;
                        continue;
                    }

                    if (pFuncInfo->iParmCount + pFuncInfo->iOptParms >= (int)expectedArgCount)
                    {
                        fFound = TRUE;
                        *pMemId = pFuncInfo->memId;
                        if (ppFuncInfo)
                        {
                            *ppFuncInfo = pFuncInfo;
                        }
                        break; /* found correct function */
                    }
                }

                // arg count doesn't match. cache "a" fitting method
                // in case we don't find anything better
                pFuncCache = pFuncInfo;
            }

            /* not found so far, go to next element in list */
            pFuncInfo = pFuncInfo->pNext;
        } /* endwhile */
    }

    if (fFound == FALSE)
    {
        if (pFuncCache)
        {
            fFound = TRUE;
            *pMemId = pFuncCache->memId;
            // found "something" with that name, but the
            // function description is not reliable (fFindFunction
            // will be called again with *any* number of arguments -
            // this way we can turn on the DISPATCH_METHOD bit for
            // invocation)
        }
        else
        {
            // Try to get the dispatch ID by name.
            fFound = getIDByName(pszFunction, pDispatch, pDispatchEx, pMemId);
        }
    }

    return fFound;
}

/**
 * Try to get the dispatch id for a function / methd by name.
 *
 * @param pszFunction  Function / method name
 * @param pDispatch    Pointer to IDispatch, may not be null.
 * @param pDispatchEx  Pointer to IDispatchEx, can be null, often is null.
 * @param pMemId       [in/out] returned dispatch id, if found.
 *
 * @return True if the dispatch ID was obtained, otherwise false.
 */
static BOOL getIDByName(const char *pszFunction, IDispatch *pDispatch, IDispatchEx *pDispatchEx, MEMBERID *pMemId)
{
    LPOLESTR unicodeName;
    HRESULT  hResult = E_FAIL;
    BOOL     gotID = FALSE;

    unicodeName = lpAnsiToUnicode(pszFunction, strlen(pszFunction) + 1);
    if ( unicodeName )
    {
        if ( pDispatchEx )
        {
            BSTR bstrName = SysAllocString(unicodeName);
            if ( bstrName )
            {
                hResult = pDispatchEx->GetDispID(bstrName, fdexNameCaseInsensitive, pMemId);
                SysFreeString(bstrName);
            }
        }
        if ( FAILED(hResult) ) // If IDispatchEx call fails, try pDispatch
        {
            if ( pDispatch )
            {
                hResult = pDispatch->GetIDsOfNames(IID_NULL, &unicodeName, 1, LOCALE_USER_DEFAULT, pMemId);
            }
        }

        if ( hResult == S_OK )
        {
            gotID = TRUE;
        }

        ORexxOleFree(unicodeName);
    }
    return gotID;
}


BOOL fFindConstant(const char * pszConstName, POLECLASSINFO pClsInfo, PPOLECONSTINFO ppConstInfo )
{
    BOOL            fFound = FALSE;
    POLECONSTINFO   pConstInfo = NULL;

    /* initialize to not found entry */
    if (ppConstInfo)
    {
        *ppConstInfo = NULL;
    }


    /* find function in class info structure */
    pConstInfo = pClsInfo->pConstInfo;
    while ( pConstInfo )
    {
        if (stricmp(pConstInfo->pszConstName, pszConstName) == 0)
        {
            fFound = TRUE;
            if (ppConstInfo)
            {
                *ppConstInfo = pConstInfo;
            }
            break; /* found correct constant */
        }

        /* not found so far, go to next element in list */
        pConstInfo = pConstInfo->pNext;
    } /* endwhile */

    return fFound;
}


RexxObjectPtr SafeArray2RexxArray(RexxThreadContext *context, VARIANT *pVariant)
{
    SAFEARRAY  *pSafeArray;
    VARTYPE     EmbeddedVT;
    VARIANT     sVariant;
    LONG        lDimensions;
    LONG        lIdx;
    LONG        lDIdx;               // dimension index
    PVOID       pTarget;
    RexxObjectPtr RxItem;
    RexxObjectPtr ResultObj = context->Nil();
    RexxArrayObject argArray = NULL;     // argument array for "new" of multidimensional array
    HRESULT     hResult;
    PLONG       lpIndices;
    PLONG       lpLowBound;
    PLONG       lpUpperBound;
    LONG        lNumOfElements;
    BOOL        fCarryBit;
    INT         i;

    /* V_ARRAY can be passed by reference. */
    if ( V_VT(pVariant) & VT_BYREF )
    {
        pSafeArray = *(V_ARRAYREF(pVariant));
    }
    else
    {
        pSafeArray = V_ARRAY(pVariant);
    }

    EmbeddedVT = V_VT(pVariant) & VT_TYPEMASK;
    lDimensions = SafeArrayGetDim(pSafeArray);

    /* alloc an array of lDimensions LONGs to hold the indices */
    lpIndices=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);
    /* alloc an array of lDimensions LONGs to hold the upper bound */
    lpUpperBound=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);
    /* alloc an array of lDimensions LONGs to hold the indices */
    lpLowBound=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);

    if ( lpIndices == NULL || lpUpperBound == NULL || lpLowBound == NULL )
    {
        ORexxOleFree(lpLowBound);
        ORexxOleFree(lpUpperBound);
        ORexxOleFree(lpIndices);

        context->RaiseException0(Rexx_Error_System_resources);
        return NULLOBJECT;
    }

    /* build argument array for construction of multidimensional array */
    argArray = context->NewArray(lDimensions);

    lNumOfElements=1;  // total number of elements
    for (lDIdx=1;lDIdx<=lDimensions;lDIdx++)
    {
        hResult = SafeArrayGetLBound(pSafeArray, lDIdx, lpLowBound+lDIdx-1);
        hResult = SafeArrayGetUBound(pSafeArray, lDIdx, lpUpperBound+lDIdx-1);
        lNumOfElements*=(lpUpperBound[lDIdx-1]-lpLowBound[lDIdx-1]+1);
        // put number of elements for this dimension into argument array
        context->ArrayPut(argArray, context->WholeNumberToObject(lpUpperBound[lDIdx-1]-lpLowBound[lDIdx-1]+1), lDIdx);
        // initial value of indices vector = [LowBound[1],...,LowBound[n]]
        lpIndices[lDIdx-1]=lpLowBound[lDIdx-1];
    }

    RexxClassObject ArrayObjectClass = context->FindClass("ARRAY");
    // create an array with lDimensions dimensions
    ResultObj = context->SendMessage(ArrayObjectClass, "NEW", argArray);

    /* process all elements */
    for (lIdx=0;lIdx<lNumOfElements;lIdx++)
    {
        /* now build message array to put element at its place in rexx array */
        /* the indices for each dimension get place at 2,...,n+1, because    */
        /* 1 is reserved for the object itself (see PUT of array method)     */
        argArray = context->NewArray(lDimensions);
        for (i=0; i<lDimensions; i++)
        {
            context->ArrayPut(argArray, context->WholeNumberToObject(1-lpLowBound[i]+lpIndices[i]), i + 1);
        }

        /* get the element at current indices, transform it into a rexx object and */
        /* set it into the rexx array                                              */

        VariantInit(&sVariant);
        V_VT(&sVariant) = EmbeddedVT;

        // if (EmbeddedVT == VT_VARIANT || EmbeddedVT == VT_BSTR)  // treat VT_BSTR like VT_VARIANT
        if (EmbeddedVT == VT_VARIANT)
        {
            pTarget = (PVOID) &sVariant;
        }
        else
        {
            pTarget = (PVOID) &(V_NONE(&sVariant));
        }

        hResult = SafeArrayGetElement(pSafeArray, lpIndices, pTarget);
        if (hResult == S_OK)
        {
            /* create a new REXX object from the result */
            RxItem = Variant2Rexx(context, &sVariant);
            if ( RxItem == NULLOBJECT )
            {
                // Variant2Rexx() has raised an exception.
                ORexxOleFree(lpLowBound);
                ORexxOleFree(lpUpperBound);
                ORexxOleFree(lpIndices);
                return NULLOBJECT;
            }
        }

        VariantClear(&sVariant);
        // as of 3.2.0, Array PUT messages allow a multi-dimensional index to be specified
        // as an array of indices.
        context->SendMessage2(ResultObj, "PUT", RxItem, argArray);

        /* increment indices vector (to access safearray elements) */
        fCarryBit=TRUE;
        i=0;
        while (fCarryBit && i<lDimensions)
        {
            if (lpIndices[i] == lpUpperBound[i])
            {
                lpIndices[i] = lpLowBound[i];
            }
            else
            {
                lpIndices[i]++;
                fCarryBit=FALSE;
            }
            i++;
        }
    }

    ORexxOleFree(lpLowBound);
    ORexxOleFree(lpUpperBound);
    ORexxOleFree(lpIndices);

    return ResultObj;
}

/**
 * Attempts to convert a variant into a corresponding ooRexx object.
 *
 * @param context   The thread context we are operating under.
 * @param pVariant  The variant to convert.
 *
 * @return          The converted ooRexx object, or NULLOBJECT if an exception
 *                  has been raised.
 *
 * @note  In some cases, the correct conversion is the .nil object. However, in
 *        some cases where a conversion fails the .nil object is also returned
 *        and is not considered an error. For instance, an IUnknown interface
 *        that does not have an IDispatch interface can not be converted to
 *        anything meaningful in ooRexx.
 *
 *        In all cases where NULLOBJECT is returned, an exception has been
 *        raised.
 */
RexxObjectPtr Variant2Rexx(RexxThreadContext *context, VARIANT *pVariant)
{
    RexxObjectPtr  ResultObj = NULLOBJECT;
    VARIANT        sTempVariant;
    CHAR           szBuffer[2048];
    IUnknown      *pUnknown;
    RexxObjectPtr  OLEObjectClass = NULL;
    HRESULT        hResult;
    BOOL           fByRef = FALSE;
    IDispatch     *pOleObject = NULL;

    if ( V_VT(pVariant) & VT_BYREF )
    {
        fByRef = TRUE;
    }

    if (V_VT(pVariant) & VT_ARRAY)
    {
        ResultObj = SafeArray2RexxArray(context, pVariant);
        if ( ResultObj == NULLOBJECT )
        {
            // SafeArray2RexxArray() has raised an exception.
            return NULLOBJECT;
        }
    }
    else
    {
        switch (V_VT(pVariant) & VT_TYPEMASK)
        {
            case VT_I2:
            case VT_I4:
            case VT_R4:
            case VT_R8:
            case VT_CY:
            case VT_DATE:
            case VT_BSTR:
            case VT_VARIANT:
            case VT_DECIMAL:
            case VT_I1:
            case VT_UI1:
            case VT_UI2:
            case VT_UI4:
            case VT_I8:
            case VT_UI8:
            case VT_INT:
            case VT_UINT:
            case VT_HRESULT:
            case VT_LPSTR:
            case VT_LPWSTR:
            case VT_FILETIME:
                /* transform result into a string object */
                VariantInit(&sTempVariant);
                // special treatment for VT_VARIANT|VT_BYREF:
                // normally it is not allowed to again reference a VARIANT if it is already
                // referenced by VT_BYREF. unfortunately, the internet explorer does this,
                // so we try to catch this further (illegal) level of indirection here.
                if (V_VT(pVariant) == (VT_VARIANT|VT_BYREF))
                {
                    pVariant = (VARIANT*) pVariant->pvarVal;
                    fByRef = FALSE;
                    if (V_VT(pVariant) & VT_BYREF) pVariant = pVariant->pvarVal;
                }

                if (fByRef)
                {
                    VARIANT temp;
                    VariantInit(&temp);
                    // take care of the indirection
                    VariantCopyInd(&temp,pVariant);
                    hResult = VariantChangeType(&sTempVariant, &temp, 0, VT_BSTR);
                    VariantClear(&temp);
                }
                else
                {
                    hResult = VariantChangeType(&sTempVariant, pVariant, 0, VT_BSTR);
                }
                if (hResult == S_OK)
                {
                    /* convert BSTR to an ANSI string */
                    PSZ pszAnsiStr;

                    pszAnsiStr = pszUnicodeToAnsi(V_BSTR(&sTempVariant));
                    if (pszAnsiStr)
                    {
                        // if this was a float/double, ignore(!) the users LOCALE settings
                        if (V_VT(pVariant) == VT_R8 || V_VT(pVariant) == VT_R4)
                        {
                            char  pBuffer[4];
                            GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,pBuffer,4);
                            for (int j=0;j<(int) strlen(pszAnsiStr);j++)
                            {
                                if (pszAnsiStr[j] == pBuffer[0]) pszAnsiStr[j]='.';
                            }
                        }

                        ResultObj = context->NewStringFromAsciiz(pszAnsiStr);
                        ORexxOleFree(pszAnsiStr);
                    }
                }
                else
                {
                    sprintf(szBuffer, "%s", pszDbgVarType(V_VT(pVariant)));
                    context->RaiseException1(Rexx_Error_Variant2Rexx, context->NewStringFromAsciiz(szBuffer));
                    return NULLOBJECT;
                }
                // if (fByRef) V_VT(pVariant)^=VT_BYREF; // VariantChangeType does not like VT_BYREF
                VariantClear(&sTempVariant);
                break;

            case VT_BOOL:
                /* some special handling for VT_BOOL */
                if (V_BOOL(pVariant) == 0)
                {
                    ResultObj = context->False();
                }
                else
                {
                    ResultObj = context->True();
                }
                break;

            case VT_UNKNOWN:
                // Try to get a dispatch pointer and then create an OLE Object.
                if ( fByRef )
                {
                    pUnknown = *V_UNKNOWNREF(pVariant);
                }
                else
                {
                    pUnknown = V_UNKNOWN(pVariant);
                }

                if ( pUnknown )
                {
                    IDispatch  *pDispatch;

                    hResult = pUnknown->QueryInterface(IID_IDispatch, (LPVOID *)&pDispatch);
                    if ((hResult == S_OK) && pDispatch)
                    {
                        sprintf(szBuffer, "IDISPATCH=%p", pDispatch);
                        OLEObjectClass = context->FindClass("OLEOBJECT");
                        ResultObj = context->SendMessage1(OLEObjectClass, "NEW", context->String(szBuffer));
                        pDispatch->Release();
                    }
                }

                if (ResultObj == NULLOBJECT)
                {
                    ResultObj = context->Nil();
                }
                break;

            case VT_DISPATCH:
                // Create a new OLE object with this dispatch pointer.
                if (fByRef)
                {
                    pOleObject = *V_DISPATCHREF(pVariant);
                }
                else
                {
                    pOleObject = V_DISPATCH(pVariant);
                }
                if (pOleObject)
                {
                    sprintf(szBuffer, "IDISPATCH=%p", pOleObject);
                    OLEObjectClass = context->FindClass("OLEOBJECT");
                    ResultObj = context->SendMessage1(OLEObjectClass, "NEW", context->String(szBuffer));
                }
                else
                {
                    ResultObj = context->Nil();
                }
                break;

            case VT_EMPTY:
            case VT_NULL:
            case VT_VOID:
                ResultObj = context->Nil();
                break;

            case VT_ERROR:
                if ( fByRef )
                {
                    hResult = *V_ERRORREF(pVariant);
                }
                else
                {
                    hResult = V_ERROR(pVariant);
                }

                void *tmpBuf;

                if ( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                   NULL, hResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&tmpBuf, 0, NULL) )
                {
                    sprintf(szBuffer, "VT_ERROR (0x%08x) <%s>", hResult, tmpBuf);
                    LocalFree(tmpBuf);
                }
                else
                {
                    sprintf(szBuffer, "VT_ERROR (0x%08x) <%s>", hResult, "no description");
                }
                ResultObj = context->String(szBuffer);

                break;


            case VT_BLOB:
            case VT_STREAM:
            case VT_STORAGE:
            case VT_STREAMED_OBJECT:
            case VT_STORED_OBJECT:
            case VT_BLOB_OBJECT:
            case VT_CF:
            case VT_CLSID:
            case VT_PTR:
            case VT_SAFEARRAY:
            case VT_CARRAY:
            case VT_USERDEFINED:
            default:
                context->RaiseException1(Rexx_Error_Variant2Rexx, context->NewStringFromAsciiz(pszDbgVarType(V_VT(pVariant))));
                return NULLOBJECT;
        } /* end switch */
    }

    if ( ResultObj == NULLOBJECT )
    {
        ResultObj = context->NewStringFromAsciiz("");
    }
    return ResultObj;
}

/**
 *  Converts the specified Rexx object to VT_BOOL, or raises an exception if the
 *  Rexx object is not strictly .true or .false.  This function should only be
 *  called when it is known that VT_BOOL is the only correct type for the
 *  variant.
 *
 *  When we know the COM method requires a bool, (DestVt == VT_BOOL,) but the
 *  user did not specify true or false for the argument, an exception is raised.
 *  In the past, converting to VARIANT_FALSE or VARIANT_TRUE when the usere did
 *  not specify .true or .false, has lead to hard to diagnose failures. Better
 *  to raise an exception and get the user to fix her code.
 *
 * @param context   The thread context we are operating under.
 * @param RxObject  The ooRexx object to be converted.
 * @param pVariant  The returned variant, VT_TRUE or VT_FALSE.
 * @param byRef     Whether the variant is by reference or not.
 *
 * @return  True, conversion successful, or false an exception was raised.
 */
bool rexx2vt_bool(RexxThreadContext *context, RexxObjectPtr RxObject, VARIANT *pVariant, bool byRef)
{
    VARIANT_BOOL targetValue;

    if ( RxObject == context->True() )
    {
        targetValue = VARIANT_TRUE;
    }
    else if ( RxObject == context->False() )
    {
        targetValue = VARIANT_FALSE;
    }
    else
    {
        wholenumber_t intval;

        if ( context->ObjectToWholeNumber(RxObject, &intval) )
        {
            if (intval == 0)
            {
                targetValue = VARIANT_FALSE;
            }
            else if (intval == 1)
            {
                    targetValue = VARIANT_TRUE;
            }
            else
            {
                goto notTrueOrFalse;
            }
        }
        else
        {
            goto notTrueOrFalse;
        }
    }

    if ( byRef )
    {
        V_VT(pVariant) = VT_BOOL|VT_BYREF;
        *V_BOOLREF(pVariant) = targetValue;
    }
    else
    {
        V_VT(pVariant) = VT_BOOL;
        V_BOOL(pVariant) = targetValue;
    }
    return true;

notTrueOrFalse:
    char buf[128];
    _snprintf(buf, sizeof(buf),
              "Cannot convert REXX object \"%s\" to VT_BOOL: The REXX object is not .true or .false.",
              context->ObjectToStringValue(RxObject));

    V_VT(pVariant) = VT_ERROR;
    V_ERROR(pVariant) = DISP_E_PARAMNOTFOUND;
    context->RaiseException1(Rexx_Error_OLE_Error_user_defined, context->String(buf));

    return false;
}

/**
  * Convert an ooRexx object to a Variant.
  *
  * Notes:
  *   VT_BOOL
  *     Microsoft uses a non-intuitive value for this variant, and documents it
  *     this way: A value of 0xFFFF (all bits 1) indicates True; a value of 0
  *     (all bits 0) indicates False. No other values are valid.
  *
  *     Setting a VT_BOOL's value to 1 for true does not produce the correct
  *     result.
  */
bool Rexx2Variant(RexxThreadContext *context, RexxObjectPtr _RxObject, VARIANT *pVariant, VARTYPE _DestVt, size_t iArgPos)
{
    bool          fByRef = false;
    VARIANT       sVariant;
    HRESULT       hResult;
    RexxObjectPtr RxObject;
    VARTYPE       DestVt;

    switch ( checkForOverride(context, pVariant, _RxObject, _DestVt, &RxObject, &DestVt) )
    {
        case SuccessReturn :
            return true;
        case ExceptionReturn :
            return false;
        default:
            ; // Drop through
    }

    if (DestVt & VT_BYREF)
    {
        DestVt ^= VT_BYREF;
        fByRef = true;
    }

    /* arguments are filled in from the end of the array */
    VariantInit(pVariant);

    if ((RxObject == NULL) || (RxObject == context->Nil()))
    {
        /* omitted argument */
        V_VT(pVariant) = VT_ERROR;
        V_ERROR(pVariant) = DISP_E_PARAMNOTFOUND;
        return true;
    }
    RexxClassObject oleClass = context->FindClass("OLEOBJECT");

    /* is this an OLEObject providing an !IDISPATCH property? */
    if (context->IsInstanceOf(RxObject, oleClass))
    {
        RexxPointerObject RxPointer = (RexxPointerObject)context->SendMessage1(RxObject, "!GETVAR", context->NewStringFromAsciiz("!IDISPATCH"));
        if (RxPointer != context->Nil())
        {
            if (fByRef)
            {
                *V_DISPATCHREF(pVariant) = (IDispatch *)context->PointerValue(RxPointer);
                V_VT(pVariant) = VT_DISPATCH|VT_BYREF;
                V_DISPATCH(pVariant)->AddRef();
            }
            else
            {
                V_DISPATCH(pVariant) = (IDispatch *)context->PointerValue(RxPointer);
                V_VT(pVariant) = VT_DISPATCH;
                V_DISPATCH(pVariant)->AddRef();
            }
            return true;
        }
    }

    /* or maybe this is an array? */
    if (context->IsArray(RxObject))
    {
        return RexxArray2SafeArray(context, RxObject, pVariant, iArgPos); // byRefCheck!!!!
    }

    /* if no target type is specified try original REXX types */
    if ((DestVt == VT_EMPTY) || (DestVt == VT_PTR) || (DestVt == VT_VARIANT))
    {
        if (RxObject == context->False() || RxObject == context->True())
        {
            if (fByRef)
            {
                V_VT(pVariant) = VT_BOOL|VT_BYREF;
                *V_BOOLREF(pVariant) = (RxObject==context->True()) ? VARIANT_TRUE : VARIANT_FALSE;
            }
            else
            {
                V_VT(pVariant) = VT_BOOL;
                V_BOOL(pVariant) = (RxObject==context->True()) ? VARIANT_TRUE : VARIANT_FALSE;
            }
            return true;
        }

        wholenumber_t intval;

        if (context->ObjectToWholeNumber(RxObject, &intval))
        {
            if (fByRef)
            {
                V_VT(pVariant) = VT_I4|VT_BYREF;
                *V_I4REF(pVariant) = (LONG)intval;
            }
            else
            {
                V_VT(pVariant) = VT_I4;
                V_I4(pVariant) = (LONG)intval;
            }
            return true;
        }

        double val;
        if (context->ObjectToDouble(RxObject, &val))
        {
            if (fByRef)
            {
                V_VT(pVariant) = VT_R8|VT_BYREF;
                *V_R8REF(pVariant) = val;
            }
            else
            {
                V_VT(pVariant) = VT_R8;
                V_R8(pVariant) = val;
            }
            return true;
        }
    }

    if ( DestVt == VT_BOOL )
    {
        return rexx2vt_bool(context, RxObject, pVariant, fByRef);
    }

    LPOLESTR  lpUniBuffer = NULL;

    if (DestVt == VT_R8 || DestVt == VT_R4)
    {
        double val;

        // try to convert this to a double value.  If it works, then we set this
        // directly.  Otherwise, fall through and pass it in as a string with the
        // VariantChangeType call.
        if (context->ObjectToDouble(RxObject, &val))
        {
            if (DestVt == VT_R8)
            {
                if (fByRef)
                {
                    V_VT(pVariant) = VT_R8|VT_BYREF;
                    *V_R8REF(pVariant) = val;
                }
                else
                {
                    V_VT(pVariant) = VT_R8;
                    V_R8(pVariant) = val;
                }
            }
            else
            {
                if (fByRef)
                {
                    V_VT(pVariant) = VT_R4|VT_BYREF;
                    *V_R4REF(pVariant) = (float)val;
                }
                else
                {
                    V_VT(pVariant) = VT_R4;
                    V_R4(pVariant) = (float)val;
                }
            }
            return true;
        }
    }

    // everything else gets returned as a string value
    RexxStringObject RxString = context->ObjectToString(RxObject);
    size_t uniBufferLength;
    lpUniBuffer = lpAnsiToUnicodeLength(context->StringData(RxString), context->StringLength(RxString), &uniBufferLength);

    if (lpUniBuffer)
    {
        if (DestVt != VT_EMPTY)
        {
            VariantInit(&sVariant);
            V_VT(&sVariant) = VT_BSTR;
            V_BSTR(&sVariant) = SysAllocStringLen(lpUniBuffer, (UINT)uniBufferLength);
            if (VariantChangeType(pVariant, &sVariant, 0, DestVt) != S_OK)
            {
                /* could not convert, give it the string value */
                hResult = VariantCopy(pVariant, &sVariant);
            }

            /* clear temporary string */
            VariantClear(&sVariant);
        }
        else
        {
            V_VT(pVariant) = VT_BSTR;
            V_BSTR(pVariant) = SysAllocStringLen(lpUniBuffer, (UINT)uniBufferLength);
        }

        ORexxOleFree( lpUniBuffer);
        return true;
    }

    // Nothing worked, raise an exception.
    V_VT(pVariant) = VT_ERROR;
    V_ERROR(pVariant) = DISP_E_PARAMNOTFOUND;
    context->RaiseException1(Rexx_Error_Rexx2Variant, RxObject);
    return false;
}


/**
 * Creates an empty safe array and configures the variant, VarArray, to contain
 * it.
 *
 * @param VarArray  The variant to contain the empty safe array.
 *
 * @return          True if the array was created, false if an exception was
 *                  raised.
 */
bool createEmptySafeArray(RexxThreadContext *context, VARIANT *VarArray)
{
    SAFEARRAY      *pSafeArray;
    SAFEARRAYBOUND *pArrayBound;

    pArrayBound = (SAFEARRAYBOUND*)ORexxOleAlloc(sizeof(SAFEARRAYBOUND));
    pArrayBound->cElements = 0;
    pArrayBound->lLbound = 0;

    pSafeArray = SafeArrayCreate(VT_VARIANT, 1, pArrayBound);
    if ( ! pSafeArray )
    {
        context->RaiseException0(Rexx_Error_System_resources);
        false;
    }

    V_VT(VarArray) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(VarArray) = pSafeArray;
    return true;
}


bool RexxArray2SafeArray(RexxThreadContext *context, RexxObjectPtr RxArray, VARIANT *VarArray, size_t iArgPos)
{
    wholenumber_t   lDimensions;
    PLONG           lpIndices;              // vector of indices
    wholenumber_t   lSize = 1;              // number of elements that need to be considered
    wholenumber_t   lCount;
    LONG            i, j;                   // counter variables
    RexxObjectPtr      RexxItem;
    SAFEARRAY      *pSafeArray;             // the safearray
    SAFEARRAYBOUND *pArrayBounds;           // bounds for each dimension
    VARIANT         sVariant;
    HRESULT         hResult;
    BOOL            fCarryBit;

    context->ObjectToWholeNumber(context->SendMessage0(RxArray,"DIMENSION"), &lDimensions);

    /* An empty array is valid, and necessary for some OLE Automation objects. */
    if ( lDimensions == 0 )
    {
        return createEmptySafeArray(context, VarArray);
    }

    /* alloc an array of lDimensions LONGs to hold the indices */
    lpIndices=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);
    /* alloc an array of SAFEARRAYBOUNDs */
    pArrayBounds=(SAFEARRAYBOUND*) ORexxOleAlloc(sizeof(SAFEARRAYBOUND)*lDimensions);

    if ( ! lpIndices || ! pArrayBounds )
    {
        ORexxOleFree(pArrayBounds);
        ORexxOleFree(lpIndices);
        context->RaiseException0(Rexx_Error_System_resources);
        return false;
    }

    /* get necessary information on array and set indices vector to initial state */
    for (i=0;i<lDimensions;i++)
    {
        context->ObjectToWholeNumber(context->SendMessage1(RxArray,"DIMENSION", context->WholeNumberToObject(i + 1)), &lCount);
        // calculate the number of overall elements
        lSize *= lCount;
        /* initialize the SAFEARRAYBOUNDs */
        pArrayBounds[i].cElements = (ULONG)lCount;  // size of dimension
        pArrayBounds[i].lLbound=0;                  // lower bound
        // set indices vector to initial state
        lpIndices[i]=0;
    }

    /* create the SafeArray */
    pSafeArray = SafeArrayCreate(VT_VARIANT,(UINT) lDimensions, pArrayBounds);
    if ( ! pSafeArray )
    {
        ORexxOleFree(pArrayBounds);
        ORexxOleFree(lpIndices);
        context->RaiseException0(Rexx_Error_System_resources);
        return false;
    }

    V_VT(VarArray) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(VarArray) = pSafeArray;

    /* get each element and transform it into a VARIANT */
    for (i=0; i<lSize; i++)
    {
        RexxArrayObject argArray = context->NewArray(lDimensions);
        for (j=0; j < lDimensions; j++)
        {
            // put j-th index in msg array
            context->ArrayPut(argArray, context->WholeNumberToObject(lpIndices[j]+1), j+1);
        }
        /* get item from RexxArray */
        RexxItem = context->SendMessage1(RxArray, "AT", argArray);

        /* convert it into a VARIANT */
        VariantInit(&sVariant);

        if (RexxItem == context->Nil())
        {
            // special handling of .nil (avoid VT_ERROR)
            V_VT(&sVariant)=VT_EMPTY;
        }
        else
        {
            if ( ! Rexx2Variant(context, RexxItem, &sVariant, VT_EMPTY, 0) )
            {
                ORexxOleFree(pArrayBounds);
                ORexxOleFree(lpIndices);
                return false;
            }
        }

        /* set into the SafeArray */
        hResult = SafeArrayPutElement(pSafeArray, lpIndices, &sVariant);
        if (FAILED(hResult))
        {
            // safearrayputelement failed - action required?
        }
        /* clear the local copy */
        VariantClear(&sVariant);

        /* increment indices vector */
        fCarryBit=TRUE;
        j=0;
        while (fCarryBit && j<lDimensions)
        {
            if (lpIndices[j] == (long) pArrayBounds[j].cElements - 1)
            {
                lpIndices[j] = 0;
            }
            else
            {
                lpIndices[j]++;
                fCarryBit=FALSE;
            }
            j++;
        }
    }

    ORexxOleFree(pArrayBounds);
    ORexxOleFree(lpIndices);

    return true;
}

BOOL fExploreTypeAttr( ITypeInfo *pTypeInfo, TYPEATTR *pTypeAttr, POLECLASSINFO pClsInfo )
{
    BOOL            fOk = TRUE;
    int             iFuncVarIdx;
    int             iParmIdx;
    HRESULT         hResult;
    FUNCDESC        *pFuncDesc = NULL;
    VARDESC         *pVarDesc = NULL;
    POLEFUNCINFO    pFuncInfo = NULL;
    POLECONSTINFO   pConstInfo = NULL;
    BSTR            bstrName;
    PSZ             pszName = NULL;

    /* get information for all functions */
    for (iFuncVarIdx = 0; iFuncVarIdx < pTypeAttr->cFuncs; ++iFuncVarIdx)
    {
        hResult = pTypeInfo->GetFuncDesc(iFuncVarIdx, &pFuncDesc);
        if ( hResult == S_OK )
        {
            hResult = pTypeInfo->GetDocumentation(pFuncDesc->memid, &bstrName, NULL, NULL, NULL);
            if ( hResult == S_OK )
            {
                pszName = pszUnicodeToAnsi(bstrName);
                SysFreeString(bstrName);
            }
            else
                pszName = NULL;

            pFuncInfo = AddFuncInfoBlock( pClsInfo, pFuncDesc->memid, pFuncDesc->invkind,
                                          pFuncDesc->elemdescFunc.tdesc.vt, pFuncDesc->cParams,
                                          pFuncDesc->cParamsOpt, pszName );
            if ( pFuncInfo )
            {
                pFuncInfo->memId = pFuncDesc->memid;
                pFuncInfo->invkind = pFuncDesc->invkind;
                pFuncInfo->FuncVt = pFuncDesc->elemdescFunc.tdesc.vt;
                pFuncInfo->iParmCount = pFuncDesc->cParams;
                pFuncInfo->iOptParms = pFuncDesc->cParamsOpt;
                pFuncInfo->pOptVt = (VARTYPE *) ORexxOleAlloc(pFuncDesc->cParams * sizeof(VARTYPE));
                pFuncInfo->pusOptFlags = (PUSHORT) ORexxOleAlloc(pFuncDesc->cParams * sizeof(USHORT));
                pFuncInfo->pszFuncName = pszName;

                /* get parameter info for this function */
                if ( pFuncDesc->lprgelemdescParam )
                {
                    for (iParmIdx = 0; iParmIdx < pFuncDesc->cParams; iParmIdx++)
                    {
                        pFuncInfo->pOptVt[iParmIdx] = pFuncDesc->lprgelemdescParam[iParmIdx].tdesc.vt;
                        pFuncInfo->pusOptFlags[iParmIdx] = pFuncDesc->lprgelemdescParam[iParmIdx].paramdesc.wParamFlags;

                        if ( pFuncInfo->pOptVt[iParmIdx] == VT_USERDEFINED )
                        {
                            pFuncInfo->pOptVt[iParmIdx] =
                            getUserDefinedVT(pTypeInfo, pFuncDesc->lprgelemdescParam[iParmIdx].tdesc.hreftype);
                        }
                    } /* endfor */
                }
            }
            else
            {
                // memory exception or duplicate entry!
                ORexxOleFree( pszName );
            }

            pTypeInfo->ReleaseFuncDesc(pFuncDesc);
        }
    } /* endfor */

    /* get information for all variables */
    for (iFuncVarIdx = 0; iFuncVarIdx < pTypeAttr->cVars; ++iFuncVarIdx)
    {
        hResult = pTypeInfo->GetVarDesc(iFuncVarIdx, &pVarDesc);

        hResult = pTypeInfo->GetDocumentation(pVarDesc->memid, &bstrName, NULL, NULL, NULL);
        if ( hResult == S_OK )
        {
            pszName = pszUnicodeToAnsi(bstrName);
            SysFreeString(bstrName);

            if (pVarDesc->varkind == VAR_CONST)
                pConstInfo = AddConstInfoBlock( pClsInfo, pVarDesc->memid, pszName, pVarDesc->lpvarValue );
            else
                pConstInfo = NULL;

            if ( pConstInfo )
            {
                pConstInfo->memId = pVarDesc->memid;
                pConstInfo->pszConstName = pszName;
                VariantCopy( &(pConstInfo->sValue), pVarDesc->lpvarValue );
            }
            else
            {
                // duplicate entry or memory allocation error!
                ORexxOleFree( pszName );
            }
        }
        else
            pszName = NULL;

        pTypeInfo->ReleaseVarDesc(pVarDesc);
    }

    return fOk;
}

/* Determines the Automation Variant Type to use for VT_USERDEFINED based on the
 * TYPEKIND of the referenced type.  For instance, if the referenced type is
 * TKIND_ENUM, the variant type to use is VT_I4.  (Reference KB237771 in MSDN,
 * i.e Knowledge Base article 237771.)
 */
VARTYPE getUserDefinedVT( ITypeInfo *pTypeInfo, HREFTYPE hrt )
{
    VARTYPE    vt = VT_USERDEFINED;
    TYPEATTR  *pTypeAttr = NULL;
    ITypeInfo *pTypeInfo2 = NULL;
    HRESULT    hResult;

    hResult = pTypeInfo->GetRefTypeInfo(hrt, &pTypeInfo2);
    if ( SUCCEEDED(hResult) && pTypeInfo2 )
    {
        hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);
        if ( SUCCEEDED(hResult) && pTypeAttr )
        {
            switch ( pTypeAttr->typekind )
            {
                case TKIND_ENUM :
                    vt = VT_I4;
                    break;
                case TKIND_INTERFACE :
                    vt = VT_UNKNOWN;
                    break;
                case TKIND_DISPATCH :
                    vt = VT_DISPATCH;
                    break;
                case TKIND_ALIAS:
                    if ( pTypeAttr->tdescAlias.vt == VT_USERDEFINED )
                        vt = getUserDefinedVT(pTypeInfo2, pTypeAttr->tdescAlias.hreftype);
                    else
                        vt = pTypeAttr->tdescAlias.vt;
                default :
                    break;
            }
            pTypeInfo2->ReleaseTypeAttr(pTypeAttr);
        }
        pTypeInfo2->Release();
    }

    return vt;
}

BOOL fExploreTypeInfo( ITypeInfo *pTypeInfo, POLECLASSINFO pClsInfo )
{
    BOOL         fOk = FALSE;
    HRESULT      hResult;
    ITypeLib     *pTypeLib = NULL;
    unsigned int iTypeInfoCount;
    TYPEATTR     *pTypeAttr;
    INT          j;
    unsigned int iTypeIndex = 0;
    GUID         typeGUID;
    POLECONSTINFO cachedInfo;

    /* get containing type library / index of type description for this type. */
    /* at first get associated type info */
    hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
    if ( hResult == S_OK )
    {

        fExploreTypeAttr( pTypeInfo, pTypeAttr, pClsInfo );

        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    }

    /* now get info from all type info blocks in type library */
    hResult = pTypeInfo->GetContainingTypeLib(&pTypeLib, &iTypeIndex);
    if ( hResult == S_OK && pTypeLib )
    {
        ITypeInfo *pTypeInfo2 = NULL;
        TYPEKIND  iTypeInfoType;

        hResult = pTypeLib->GetTypeInfo(iTypeIndex, &pTypeInfo2);
        if (pTypeInfo2)
        {
            hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);
            if ( hResult == S_OK )
            {
                fExploreTypeAttr(pTypeInfo2, pTypeAttr, pClsInfo);
                pTypeInfo2->ReleaseTypeAttr(pTypeAttr);
            }

            pTypeInfo2->Release();
        }

        iTypeInfoCount = (INT) pTypeLib->GetTypeInfoCount();
        // find the GUID for the type library
        {
            TLIBATTR *pTLibAttr = NULL;
            hResult = pTypeLib->GetLibAttr(&pTLibAttr);
            if (hResult == S_OK)
            {
                memcpy(&typeGUID,&(pTLibAttr->guid),sizeof(GUID));
                pTypeLib->ReleaseTLibAttr(pTLibAttr);
            }
            else // this is illegal, what do we do about it? [shouldn't happen]
            {
                memset(&typeGUID,0,sizeof(GUID));
            }
        }
#if 0
        j=uTypeLibIndex;   // this block should contain all info on the object
        // for (j = 0; j < (INT) iTypeInfoCount; j++)
        {
            hResult = pTypeLib->GetTypeInfoType(j, &iTypeInfoType);

            hResult = pTypeLib->GetTypeInfo(j, &pTypeInfo2);

            if (pTypeInfo2)
            {
                hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);
                if ( hResult == S_OK )
                {

                    fExploreTypeAttr( pTypeInfo2, pTypeAttr, pClsInfo );

                    pTypeInfo2->ReleaseTypeAttr(pTypeAttr);
                }

                pTypeInfo2->Release();
            }
        }
#endif

        // retrieve constants. do this only once for each type library
        // if this was already done, use the cached information (from an extra list)
        if ( (cachedInfo = fFindConstantInTypeLib(&typeGUID)))
            pClsInfo->pConstInfo = cachedInfo;
        else
        {
            // now iterate through whole type library and find all constants
            for (j = 0; j < (INT) iTypeInfoCount; j++)
            {
                hResult = pTypeLib->GetTypeInfoType(j, &iTypeInfoType);
                if (iTypeInfoType == TKIND_ENUM || iTypeInfoType == TKIND_MODULE)
                {
                    hResult = pTypeLib->GetTypeInfo(j, &pTypeInfo2);
                    if (hResult == S_OK)
                    {
                        hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);
                        if (hResult == S_OK)
                        {
                            fExploreTypeAttr( pTypeInfo2, pTypeAttr, pClsInfo );
                            pTypeInfo2->ReleaseTypeAttr(pTypeAttr);
                        }
                        pTypeInfo2->Release();
                    }
                }
            }
            if (pClsInfo->pConstInfo)
            {
                addTypeListElement(&typeGUID, pClsInfo->pConstInfo);
            }
        }

        pTypeLib->Release();
    }

    return fOk;
}

/* Retrieve all information to invoke an event in case it occurs */
POLEFUNCINFO2 GetEventInfo(ITypeInfo *pTypeInfo, RexxObjectPtr self, POLECLASSINFO pClsInfo)
{
    TYPEATTR *pTypeAttr = NULL;
    FUNCDESC *pFuncDesc = NULL;
    HRESULT   hResult;
    POLEFUNCINFO2 pEventList = NULL;
    POLEFUNCINFO2 pTemp;
    POLEFUNCINFO  pFuncInfo = NULL;
    BSTR     *pbStrings;
    int j;
    unsigned short i;
    unsigned int uFlags;

    hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);

    if (hResult == S_OK)
    {
        for (i=0;i<pTypeAttr->cFuncs;i++)
        {
            hResult = pTypeInfo->GetFuncDesc(i,&pFuncDesc);

            if (hResult == S_OK && pFuncDesc)
            {
                // see if this function can be used "normally" (not hidden etc.)  Only
                // add user-callable functions to the list.
                if (!(pFuncDesc->wFuncFlags & FUNCFLAG_FRESTRICTED) &&
                    !(pFuncDesc->wFuncFlags & FUNCFLAG_FHIDDEN))
                {
                    BSTR bString;
                    char szBuffer[2048];

                    pbStrings=new BSTR[pFuncDesc->cParams + 1];  // one more for method name
                    hResult = pTypeInfo->GetNames(pFuncDesc->memid,pbStrings,pFuncDesc->cParams+1,&uFlags);

                    //if ((hResult != S_OK) || uFlags != pFuncDesc->cParams+1) fprintf(stderr,"autsch\n");

                    sprintf(szBuffer,"%S",pbStrings[0]);

                    if (pClsInfo)
                    {
                        pFuncInfo = pClsInfo->pFuncInfo;
                        if (pFuncInfo)
                        {
                            while (pFuncInfo)
                            {
                                if (!stricmp(pFuncInfo->pszFuncName,szBuffer))
                                {
                                    sprintf(szBuffer,"OLEEvent_%S",pbStrings[0]);
                                    pFuncInfo = NULL;
                                }
                                else
                                {
                                    pFuncInfo=pFuncInfo->pNext;
                                }
                            }
                        }
                    }

                    SysFreeString(pbStrings[0]);

                    // insert new entry at the beginning
                    pTemp = pEventList;
                    pEventList = (POLEFUNCINFO2) ORexxOleAlloc( sizeof(OLEFUNCINFO2) );
                    pEventList->pNext = pTemp;
                    // fill in necessary info to let OLEObjectEvent::Invoke do its job...
                    pEventList->pszFuncName = (PSZ) ORexxOleAlloc(1+strlen(szBuffer));
                    strcpy(pEventList->pszFuncName, szBuffer);

                    hResult = pTypeInfo->GetDocumentation(pFuncDesc->memid,NULL,&bString,NULL,NULL);
                    if (hResult == S_OK)
                    {
                        sprintf(szBuffer,"%S",bString);
                        SysFreeString(bString);
                        pEventList->pszDocString = (PSZ) ORexxOleAlloc(1+strlen(szBuffer));
                        strcpy(pEventList->pszDocString, szBuffer);
                    }
                    else
                    {
                        pEventList->pszDocString = NULL;
                    }

                    pEventList->memId = pFuncDesc->memid;
                    pEventList->invkind = pFuncDesc->invkind;
                    pEventList->iParmCount = pFuncDesc->cParams;
                    pEventList->iOptParms = pFuncDesc->cParamsOpt;
                    pEventList->pOptVt = (VARTYPE *) ORexxOleAlloc(pFuncDesc->cParams * sizeof(VARTYPE));
                    pEventList->pusOptFlags = (PUSHORT) ORexxOleAlloc(pFuncDesc->cParams * sizeof(USHORT));
                    pEventList->pszName = (char**) ORexxOleAlloc(pFuncDesc->cParams * sizeof(char*));

                    for (j=0;j<pEventList->iParmCount;j++)
                    {
                        pEventList->pOptVt[j] = pFuncDesc->lprgelemdescParam[j].tdesc.vt;
                        pEventList->pusOptFlags[j] = pFuncDesc->lprgelemdescParam[j].paramdesc.wParamFlags;

                        if (pbStrings[j+1])
                        {
                            sprintf(szBuffer,"%S",pbStrings[j+1]);
                            SysFreeString(pbStrings[j+1]);
                        }
                        else
                        {
                            sprintf(szBuffer,"<unnamed>");
                        }

                        pEventList->pszName[j] = (PSZ) ORexxOleAlloc(1+strlen(szBuffer));
                        strcpy(pEventList->pszName[j], szBuffer);
                    }
                }
                pTypeInfo->ReleaseFuncDesc(pFuncDesc);
            }
        }
        pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    }
    return pEventList;
}

/* this retrieves the event type information from the coclass entry */
BOOL GetEventTypeInfo(ITypeInfo *pTypeInfo, CLSID *clsID, ITypeInfo **ppTypeInfoEvents, IID *pIID)
{
    ITypeLib *pTypeLib = NULL;
    TYPEATTR *pTypeAttr = NULL;
    ITypeInfo *pCoClass = NULL;
    TYPEKIND kind;
    unsigned int index;
    unsigned int icnt, icnt2;
    int implFlags=0;
    BOOL fFound = false;
    HRESULT hResult;

    /* first, get the type library from the type information of this OLE object */
    hResult = pTypeInfo->GetContainingTypeLib(&pTypeLib,&index);
    if (hResult == S_OK)
    {
        index = pTypeLib->GetTypeInfoCount();

        icnt=0;
        /* look at each entry... */
        while (icnt<index && !fFound)
        {
            hResult = pTypeLib->GetTypeInfoType(icnt,&kind);
            /* ...and check if it is a coclass */
            if (hResult == S_OK && kind == TKIND_COCLASS)
            {
                hResult = pTypeLib->GetTypeInfo(icnt,&pCoClass);
                if (hResult == S_OK)
                {
                    hResult = pCoClass->GetTypeAttr(&pTypeAttr);
                    /* yes... does it describe this object? */
                    if (hResult == S_OK && IsEqualCLSID(*clsID,pTypeAttr->guid) )
                    {
                        /* yes... look for the [default, source] entry */
                        for (icnt2=0;icnt2<pTypeAttr->cImplTypes;icnt2++)
                        {
                            hResult = pCoClass->GetImplTypeFlags(icnt2,&implFlags);
                            if (FAILED(hResult)) continue;
                            if ((implFlags & IMPLTYPEFLAG_FDEFAULT) &&
                                (implFlags & IMPLTYPEFLAG_FSOURCE))
                            {
                                HREFTYPE hRefType = NULL;
                                /* found it, get the type information & the IID */
                                pCoClass->GetRefTypeOfImplType(icnt2,&hRefType);
                                hResult = pCoClass->GetRefTypeInfo(hRefType,ppTypeInfoEvents);
                                if (hResult == S_OK)
                                {
                                    TYPEATTR *pTempTypeAttr = NULL;
                                    if ((*ppTypeInfoEvents)->GetTypeAttr(&pTempTypeAttr) == S_OK)
                                    {
                                        memcpy(pIID,&(pTempTypeAttr->guid),sizeof(CLSID));
                                        (*ppTypeInfoEvents)->ReleaseTypeAttr(pTempTypeAttr);
                                    }

                                    fFound = true;
                                }
                                break;
                            }
                        }
                        pCoClass->ReleaseTypeAttr(pTypeAttr);
                    }
                    pCoClass->Release();
                }
            }
            icnt++;
        }
        pTypeLib->Release();
    }
    return fFound;
}

//******************************************************************************
// Method:  OLEObject_Init
//
//   Arguments:
//     self - A pointer to self
//     objectClass (RexxObjectPtr) - The OLE class to create an object of
//     events (RexxObjectPtr)      - String to indicate whether to use events or not
//     getObjectFlag (RexxObjectPtr) - Flag to indicate whether we try a GetObject
//                                  (undocumented, unused!)
//
//   Returned:
//     returnObject (RexxObjectPtr) - The created proxy object.
//
//   Notes:
//     This method will create a new proxy object for an OLE object. The OLE
//     object will either be created from its CLSID or ProgId as specified
//     in the objectClass parameter. Additionally a proxy object can be
//     created for an already existing OLE object when the IDispatch pointer
//     is specified in the objectClass parameter in the format:
//          IDISPATCH=1234ABCD
//     If a new proxy object is created using the IDISPATCH pointer the method
//     will increase the reference count to the OLE object by calling AddRef().
//
//******************************************************************************
RexxMethod4(int,                             // Return type
            OLEObject_Init,                  // Object_method name
            OSELF, self,                     // Pointer to self
            CSTRING, pszArg,                 // Class specifier for new object
            OPTIONAL_CSTRING, eventString,   // keyword to active event handling
            OPTIONAL_RexxObjectPtr, getObjectFlag) // Try a GetActiveObject
{
    CLSID       clsID;
    HRESULT     hResult;
    LPOLESTR    lpUniBuffer = NULL;
    OLECHAR     OleBuffer[100];
    LPOLESTR    lpOleStrBuffer = OleBuffer;
    IUnknown   *pUnknown = NULL;
    IDispatch  *pDispatch = NULL;
    ITypeInfo  *pTypeInfo = NULL;
    unsigned int  iTypeInfoCount;
    POLECLASSINFO pClsInfo = NULL;
#ifdef DEBUG_TESTING
    BOOL          gotIDispatch = false;
#endif

    if ( !fInitialized )
    {
        OLEInit();
    }

    if (iInstanceCount == 0)
    {
        OleInitialize(NULL);
    }
    iInstanceCount++;

    /* get pointer to string data and convert to Unicode */
    lpUniBuffer = lpAnsiToUnicode(pszArg, strlen(pszArg) + 1);

    if ( lpUniBuffer )
    {
        if ( *pszArg == '{' )
        {
            /* argument is a CLSID */
            hResult = CLSIDFromString(lpUniBuffer, &clsID);
        }
        else
        {
            /* argument is probably a ProgID */
            hResult = CLSIDFromProgID(lpUniBuffer, &clsID);
        }

        ORexxOleFree( lpUniBuffer );
        lpUniBuffer = NULL;
    }
    else
    {
        context->RaiseException0(Rexx_Error_Interpretation_initialization);
        return 0;
    }

    if (hResult == S_OK)
    {
        /* now store the CLSID and ProgID with the object attributes */
        PSZ         pszAnsiStr = NULL;

        hResult = StringFromCLSID(clsID, &lpOleStrBuffer);
        pszAnsiStr = pszUnicodeToAnsi(lpOleStrBuffer);
        if (SUCCEEDED(hResult))
        {
            CoTaskMemFree(lpOleStrBuffer); // memory was not freed
        }
        if (pszAnsiStr)
        {
            context->SetObjectVariable("!CLSID", context->NewStringFromAsciiz(pszAnsiStr));
            if ( ! psFindClassInfo(context->threadContext, pszAnsiStr, NULL, &pClsInfo) )
            {
                // An exception has been raised.
                return 0;
            }

            if ( pClsInfo )
            {
                pClsInfo->iInstances++; /* add to instance counter */
                if (!pClsInfo->pszCLSId)
                {
                    pClsInfo->pszCLSId = pszAnsiStr;
                }
                else
                {
                    ORexxOleFree(pszAnsiStr);
                }
            }
            else
            {
                ORexxOleFree(pszAnsiStr); // free this memory!
            }
        }

        hResult = ProgIDFromCLSID(clsID, &lpOleStrBuffer);
        pszAnsiStr = pszUnicodeToAnsi(lpOleStrBuffer);
        if (SUCCEEDED(hResult))
        {
            CoTaskMemFree(lpOleStrBuffer); // memory was not freed
        }
        if (pszAnsiStr)
        {
            context->SetObjectVariable("!PROGID", context->NewStringFromAsciiz(pszAnsiStr));
            if (pClsInfo)
            {
                if (!pClsInfo->pszProgId)
                {
                    pClsInfo->pszProgId = pszAnsiStr;
                }
                else
                {
                    ORexxOleFree(pszAnsiStr);
                }
            }
            else
            {
                ORexxOleFree(pszAnsiStr);
            }
        }

        /* check if CLSID is okay for security manager */
        hResult = S_OK;
        if (creationCallback)
        {
            hResult = creationCallback(clsID, NULL)?S_OK:E_FAIL;
        }
        if (FAILED(hResult))
        {
            char errmsg[256];
            sprintf(errmsg, "an external security manager denies creation of %s",pszArg);
            context->RaiseException1(Rexx_Error_System_service_service, context->NewStringFromAsciiz(errmsg));
            return 0;
        }

        if (getObjectFlag == context->True())
        {
            hResult = GetActiveObject(clsID, NULL, &pUnknown);
        }
        else
        {
            hResult = DISP_E_MEMBERNOTFOUND; /* simulate failed GetActiveObject if it was not requested */
        }

        if (hResult != S_OK)
        {
            /* now create the OLE object and get the IDispatch interface pointer */
            /*  hResult = CoCreateInstance(clsID, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, */
            hResult = CoCreateInstance(clsID, NULL, CLSCTX_SERVER, IID_IUnknown,
                                       (LPVOID*)&pUnknown);
            if (hResult != S_OK)
            {
                hResult = CoInitialize(NULL);
                hResult = CoCreateInstance(clsID, NULL, CLSCTX_SERVER, IID_IUnknown,
                                           (LPVOID*)&pUnknown);
            }
        }

        if (hResult == S_OK)
        {
            hResult = pUnknown->QueryInterface(IID_IDispatch, (LPVOID*)&pDispatch);
        }

        if (pUnknown)
        {
            pUnknown->Release(); /* this is no longer needed */
            pUnknown = NULL;
        }
    }
    else
    {
        /* the argument is probably an IDispatch pointer or an unknown CLSID/ProgID */
        if (sscanf(pszArg, "IDISPATCH=%p", &pDispatch) == 1)
        {
            /* seems to be a valid IDispatch pointer */
            if (pDispatch)
            {
                pDispatch->AddRef();
                hResult = S_OK;
#ifdef DEBUG_TESTING
                gotIDispatch = true;
                {
                    IPersist *pPersist = NULL;
                    int hr = pDispatch->QueryInterface(IID_IPersist, (LPVOID*) &pPersist);
                    if (SUCCEEDED(hr))
                    {
                        CLSID ClsId;
                        pPersist->GetClassID(&ClsId);
                        pPersist->Release();
                    }
                }
#endif
            }
            else
            {
                context->RaiseException0(Rexx_Error_Interpretation_initialization);
                return 0;
            }
        }
        else
        {
            context->RaiseException1(Rexx_Error_Execution_noclass, context->NewStringFromAsciiz(pszArg));
            return 0;
        }
    }

    context->SetObjectVariable("!IDISPATCH", context->NewPointer(pDispatch));

    if ( (hResult != S_OK) || (pDispatch == NULL) )
    {
        context->RaiseException0(Rexx_Error_No_OLE_instance);
        return 0;
    }

    // changed logic so it can never, not even theoretically, crash here...
    hResult = E_FAIL;
    if ( !pClsInfo )
    {
        hResult = S_OK;
    }
    else if ( !(pClsInfo->pTypeInfo) )
    {
        hResult = S_OK;
    }
    if ( hResult == S_OK)
    {
        /* see if we can get some information about the object */
        hResult = pDispatch->GetTypeInfoCount(&iTypeInfoCount);

        if ((hResult == S_OK) && iTypeInfoCount)
        {
            hResult = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
            /* store type info with object */
            if (SUCCEEDED(hResult))
            { // when successful
                context->SetObjectVariable("!ITYPEINFO", context->NewPointer(pTypeInfo));
            }

            if (!pClsInfo && (hResult == S_OK) && pTypeInfo)
            {
                /* search/allocate class info block for this typeinfo */
                if ( ! psFindClassInfo(context->threadContext, NULL, pTypeInfo, &pClsInfo) )
                {
                    // An exception has been raised.
                    return 0;
                }

                if ( pClsInfo )
                {
                    pClsInfo->iInstances++; /* add to instance counter */
                }
            }
        }
    }

    context->SetObjectVariable("!OUTARRAY", context->Nil());    // set array of out parameters to nonexistent

    if (pClsInfo && !(pClsInfo->pTypeInfo))
    {
        /* store typeinfo pointer in object data */
        pClsInfo->pTypeInfo = pTypeInfo;

        if ( pTypeInfo )
        {
            fExploreTypeInfo( pTypeInfo, pClsInfo );
        }
    }

#ifdef DEBUG_TESTING
    if (pClsInfo->pTypeInfo && gotIDispatch)
    {
        TYPEATTR     *pTypeAttr;
        hResult = pClsInfo->pTypeInfo->GetTypeAttr(&pTypeAttr);
        if (SUCCEEDED(hResult))
        {
            memcpy((void*) &clsID, (void*) &(pTypeAttr->guid), sizeof(CLSID));
            pClsInfo->pTypeInfo->ReleaseTypeAttr(pTypeAttr);
        }
    }
#endif
#if 0  // this tests even if IDISPATCH=... was given (otherwise first
    // check works). we don't do this, we assume IDISPATCH=... is safe!
    /* check if CLSID is okay for security manager */
    hResult = S_OK;
    if (creationCallback)
        hResult = creationCallback(clsID, pDispatch)?S_OK:E_FAIL;
    if (FAILED(hResult))
    {
        char errmsg[256];
        sprintf(errmsg, "an external security manager denies usage of %s",pszArg);
        rexx_exception1(Error_System_service_service, ooRexxString(errmsg));
    }
#endif

    /* pTypeInfo may not be set.  It happens when this object is created from a
     * CLSID or PROGID and an OLEObject instance has already been created for
     * this COM class.  It needs to be set for the event handling check.
     */
    if ( !pTypeInfo && pClsInfo )
    {
        pTypeInfo = pClsInfo->pTypeInfo;
    }

    /* Event handling:  Only create the event object if the user asks for it by
     * explicitly specifying the second 'events' arg.  Without a type library,
     * there is no way to know what events the object supports.  So, we have to
     * have pTypeInfo.
     */
    if ( eventString != NULL && pTypeInfo != NULL )
    {
        bool connect = (strcmpi(eventString,"WITHEVENTS") == 0) ? true : false;
        addEventHandler(context, self, connect, pDispatch, pClsInfo, pTypeInfo, &clsID);
    }

    return 0;
}

//******************************************************************************
// Method:  OLEObject_Uninit
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     This method returns 0
//
//   Notes:
//     Handle the uninit of the OLE proxy object. This includes calling
//     Release() for the OLE object which will eventually free the OLE
//     object in the server process as well as decreasing the usage counter
//     in the internal type info list. If the usage counter in the internal
//     list has reached zero all the type information will be freed in order
//     to allow a clean shutdown of the OLE interface (the internal type info
//     keeps an ITypeInfo pointer which needs to be fully released before the
//     REXX program terminates in order to run OleUninitialize successfully).
//
//******************************************************************************
RexxMethod1(int, OLEObject_Uninit, OSELF, self)
{
    IDispatch        *pDispatch = NULL;
    ITypeInfo        *pTypeInfo = NULL;
    POLECLASSINFO     pClsInfo = NULL;

    if ( !fInitialized )
        OLEInit();

    /* Check for event handler & release if needed. */
    if ( haveEventHandler(context) )
    {
        releaseEventHandler(context);
    }

    /** Get the IDispatch pointer for the OLE object.  If the OLE / COM object
     *  could not be created in init() there will be no dispatch pointer, but
     *  uninit() will still run for the ooRexx object. The instance counter
     *  still needs to be reduced by 1 for this case.
     */
    RexxObjectPtr value = context->GetObjectVariable("!IDISPATCH");
    if ( value != NULLOBJECT )
    {
        pDispatch = (IDispatch *)context->PointerValue((RexxPointerObject)value);
    }
    if ( pDispatch != NULL )
    {
        /** There can be a number of OLEObjects created for the same COM class.
         *  If type info is acquired for the COM class, the type info is stored.
         *  There is only 1 AddRef() done for the type info pointer.  When
         *  successive OLEObjects are created for the same COM class, the type
         *  info is looked up, no new type info pointer is acquired.  When the
         *  AddRef is done, the type info pointer is put into the !TYPEINFO
         *  variable. It is important that the Release be called on that
         *  pointer, and only on that pointer.
         */

        // This call is only to get pClsInfo, pTypeInfo is ignored here.
        getCachedClassInfo(context, &pClsInfo, &pTypeInfo);

        // Now see if there is a pTypeInfo pointer stored in the object variable
        value = context->GetObjectVariable("!TYPEINFO");
        if ( value != NULLOBJECT )
        {
            // Yes we have the pointer.  This is the interface pointer that
            // needs to be released.
            pTypeInfo = (ITypeInfo *)context->PointerValue((RexxPointerObject)value);
            if ( pTypeInfo != NULL )
            {
                pTypeInfo->Release();
            }
        }

        if ( pClsInfo )
        {
            /* reduce instance counter, clear item if 0 reached */
            pClsInfo->iInstances--;
            if ( pClsInfo->iInstances == 0 )
                ClearClassInfoBlock( pClsInfo );
        }

        pDispatch->Release();
    }

    if ( iInstanceCount > 0 )
    {
        iInstanceCount--;
        if ( iInstanceCount == 0 )
        {
            // Free the type lib constant list and uninitialize OLE.
            destroyTypeLibList();
            OleUninitialize();
        }
    }
    return 0;
}


/**
 * Handle referencing for out parameters in IDispatch::Invoke.  Memory is
 * allocated for and set with the variant value, the variant value is set as a
 * pointer to this memory, and the VT_BYREF flag is added to the vartype.
 *
 * @param obj  Pointer to the variant to be changed to a by reference variant.
 */
void referenceVariant(VARIANT *obj)
{
    void *pNew;

    /**
     * If the variant is already by reference, then nothing should be done, so
     * just return.
     */
    {
        if ( V_VT(obj) & VT_BYREF )
            return;
    }

    switch ( V_VT(obj) & VT_TYPEMASK )
    {
        case VT_VARIANT:
            /**
             * If the VARTYPE is VT_VARIANT, it has to be orr'd with VT_BYREF,
             * VT_ARRAY, or VT_VECTOR.  OLE Automation does not use VT_VECTOR,
             * VT_BYREF has already been checked, so it must be VT_ARRAY.
             * Nevertheless, check that it is.
             */
            if ( V_VT(obj) & VT_ARRAY )
            {
                SAFEARRAY **ppsa = (SAFEARRAY **)ORexxOleAlloc(sizeof(SAFEARRAY *));
                *ppsa = V_ARRAY(obj);
                V_ARRAYREF(obj) = ppsa;
            }
            else
            {
                /**
                 * It should be impossible to be here, but, do not add VT_BYREF
                 * to a variant that is not changed by this function.
                 */
                return;
            }
            break;

        case VT_DISPATCH:
        {
            IDispatch **ppdisp = (IDispatch **)ORexxOleAlloc(sizeof(IDispatch *));
            *ppdisp = V_DISPATCH(obj);
            V_DISPATCHREF(obj) = ppdisp;
            break;
        }

        case VT_UNKNOWN:
        {
            IUnknown **ppunk = (IUnknown **)ORexxOleAlloc(sizeof(IUnknown *));
            *ppunk = V_UNKNOWN(obj);
            V_UNKNOWNREF(obj) = ppunk;
            break;
        }

        case VT_I1:
        case VT_UI1:
            pNew = (void*) ORexxOleAlloc(sizeof(char));
            * (char*) pNew = obj->cVal;
            obj->pcVal = (char*) pNew;
            break;
        case VT_I2:
        case VT_UI2:
            pNew = (void*) ORexxOleAlloc(sizeof(short));
            * (short*) pNew = obj->iVal;
            obj->piVal = (short*) pNew;
            break;
        case VT_I4:
        case VT_UI4:
            pNew = (void*) ORexxOleAlloc(sizeof(long));
            * (long*) pNew = obj->lVal;
            obj->plVal = (long*) pNew;
            break;
        case VT_INT:
        case VT_UINT:
            pNew = (void*) ORexxOleAlloc(sizeof(int));
            * (int*) pNew = obj->intVal;
            obj->pintVal = (int*) pNew;
            break;
        case VT_R4:
            pNew = (void*) ORexxOleAlloc(sizeof(float));
            * (float*) pNew = obj->fltVal;
            obj->pfltVal = (float*) pNew;
            break;
        case VT_R8:
            pNew = (void*) ORexxOleAlloc(sizeof(double));
            * (double*) pNew = obj->dblVal;
            obj->pdblVal = (double*) pNew;
            break;
        case VT_BOOL:
            pNew = (void*) ORexxOleAlloc(sizeof(VARIANT_BOOL));
            * (VARIANT_BOOL*) pNew = obj->boolVal;
            obj->pboolVal = (VARIANT_BOOL*) pNew;
            break;
        case VT_ERROR:
            pNew = (void*) ORexxOleAlloc(sizeof(SCODE));
            * (SCODE*) pNew = obj->scode;
            obj->pscode = (SCODE*) pNew;
            break;
        case VT_CY:
            pNew = (void*) ORexxOleAlloc(sizeof(CY));
            * (CY*) pNew = obj->cyVal;
            obj->pcyVal = (CY*) pNew;
            break;
        case VT_DATE:
            pNew = (void*) ORexxOleAlloc(sizeof(DATE));
            * (DATE*) pNew = obj->date;
            obj->pdate = (DATE*) pNew;
            break;
        case VT_BSTR:
            pNew = (void*) ORexxOleAlloc(sizeof(BSTR));
            *((BSTR*) pNew) = obj->bstrVal;
            obj->pbstrVal = (BSTR*) pNew;
            break;
        default:
            /**
             * Do not add VT_BYREF to a variant that has not been changed by
             * this function.
             */
            return;
    }
    obj->vt |= VT_BYREF;
}


/**
 * Changes a by-reference variant, that was set up using referenceVariant(),
 * back to its original form.  Note that this function is symmetrical with
 * referenceVariant() and should only be called with by-reference variants that
 * have been altered by that function.
 *
 * @param obj  Pointer to the variant to be dereferenced.
 */
void dereferenceVariant(VARIANT *obj)
{
    void *temp = NULL;

    /* If the variant is not by reference, do not touch it. */
    if ( ! (V_VT(obj) & VT_BYREF) )
    {
        return;
    }

    switch ( V_VT(obj) & VT_TYPEMASK )
    {
        case VT_VARIANT:
            if ( V_VT(obj) & VT_ARRAY )
            {
                temp = (void*)V_ARRAYREF(obj);
                V_ARRAY(obj) = *(V_ARRAYREF(obj));
                if ( temp )
                {
                    ORexxOleFree(temp);
                }
            }
            else
            {
                /** VT_VARIANT | VT_BYREF is a valid variant.  But
                 *  referenceVariant() does not set up that type.  So, it should
                 *  not be possible to be here.  Nevertheless, the code to
                 *  correctly dereference this type of variant, at this point of
                 *  execution, is not, and has never been, present in OLEObject.
                 *
                 *  It is a mistake to remove the VT_BYREF flag, if the variant
                 *  is not changed.  So, just return.
                 *
                 *  DFX TODO revisit this logic, especially if Variant2Rexx() is
                 *  fixed to properly handle VT_VARIANT | VT_BYREF.
                 */
                return;
            }
            break;

        case VT_DISPATCH:
            temp = (void*)V_DISPATCHREF(obj);
            V_DISPATCH(obj) = *(V_DISPATCHREF(obj));
            if ( temp )
            {
                ORexxOleFree(temp);
            }
            break;

        case VT_UNKNOWN:
            temp = (void*)V_UNKNOWNREF(obj);
            V_UNKNOWN(obj) = *(V_UNKNOWNREF(obj));
            if ( temp )
            {
                ORexxOleFree(temp);
            }
            break;

        case VT_I1:
        case VT_UI1:
            temp = (void*) obj->pcVal;
            obj->cVal = *(obj->pcVal);
            ORexxOleFree(temp);
            break;
        case VT_I2:
        case VT_UI2:
            temp = (void*) obj->piVal;
            obj->iVal = *(obj->piVal);
            ORexxOleFree(temp);
            break;
        case VT_I4:
        case VT_UI4:
            temp = (void*) obj->plVal;
            obj->lVal = *(obj->plVal);
            ORexxOleFree(temp);
            break;
        case VT_INT:
        case VT_UINT:
            temp = (void*) obj->pintVal;
            obj->intVal = *(obj->pintVal);
            ORexxOleFree(temp);
            break;
        case VT_R4:
            temp = (void*) obj->pfltVal;
            obj->fltVal = *(obj->pfltVal);
            ORexxOleFree(temp);
            break;
        case VT_R8:
            temp = (void*) obj->pdblVal;
            obj->dblVal = *(obj->pdblVal);
            ORexxOleFree(temp);
            break;
        case VT_BOOL:
            temp = (void*) obj->pboolVal;
            obj->boolVal = *(obj->pboolVal);
            ORexxOleFree(temp);
            break;
        case VT_ERROR:
            temp = (void*) obj->pscode;
            obj->scode = *(obj->pscode);
            ORexxOleFree(temp);
            break;
        case VT_CY:
            temp = (void*) obj->pcyVal;
            obj->cyVal = *(obj->pcyVal);
            ORexxOleFree(temp);
            break;
        case VT_DATE:
            temp = (void*) obj->pdate;
            obj->date = *(obj->pdate);
            ORexxOleFree(temp);
            break;
        case VT_BSTR:
            temp = (void*) obj->pbstrVal;
            obj->bstrVal = *(obj->pbstrVal);
            ORexxOleFree(temp);
            break;
        default:
            /* Do not remove the VT_BYREF flag if the variant is not changed. */
            return;
    }
    obj->vt ^= VT_BYREF;
}

/** OLEObject::addRef()  [private]
 *
 *  Increases the reference count on the IDispatch pointer by 1.  This method is
 *  used by the class internally, if and only if, an object copy of the
 *  oleObject has been made through the .Object~copy() method.
 *
 */
RexxMethod0(RexxObjectPtr, OLEObject_addRef_pvt)
{
    IDispatch *pDispatch = NULL;

    if ( getDispatchPtr(context, &pDispatch) )
    {
        pDispatch->AddRef();
    }
    return NULLOBJECT;
}

/** OLEObject::hasOLEMethod()  [private]
 *
 *  Does a quick check to see if the COM object has a method with the specified
 *  name.  This is used internally by the class to check if one of the .Object
 *  method names, such as "send," is a method name of the COM object.
 *
 *  If the COM object does have the method name, the messsage if forwarded
 *  directly to the unknown() method.  If not, the message is forwarded to the
 *  super class.
 *
 *  @param  methodName  The name to check.
 *
 *  @return  True if the COM object does have the method name, false if it does
 *           not appear to have the method.
 *
 *  @note  Currently this is a private method, but it might be useful as a
 *         public method.  Many COM objects do not have a TYPELIB, making
 *         getKnownMethods() useless.  For those COM objects, this method could
 *         provide a way for the Rexx programmer to check if a method invocation
 *         had a chance of succeeding or not.
 *
 */
RexxMethod1(logical_t, OLEObject_hasOLEMethod_pvt, CSTRING, methodName)
{
    IDispatch    *pDispatch = NULL;
    IDispatchEx  *pDispatchEx = NULL;
    MEMBERID      MemId;

    if ( ! fInitialized )
    {
        OLEInit();
    }

    if ( ! getDispatchPtr(context, &pDispatch) )
    {
        return NULLOBJECT;
    }

    pDispatch->QueryInterface(IID_IDispatchEx, (LPVOID*)&pDispatchEx);

    logical_t hasMethod = getIDByName(methodName, pDispatch, pDispatchEx, &MemId);
    if ( pDispatchEx )
    {
        pDispatchEx->Release();
    }
    return hasMethod;
}

//******************************************************************************
// Method:  OLEObject_Unknown
//
//   Arguments:
//     self - A pointer to self
//     msgName (RexxObjectPtr) - The name of OLE method being called.
//     msgArgs (RexxObjectPtr) - An array containing the parameters for the OLE
//                            method.
//
//   Returned:
//     returnObject (RexxObjectPtr) - The result from the OLE method invocation.
//
//   Notes:
//     This method will forward any call made to the Object REXX proxy object
//     to the real OLE object. It will try to convert the supplied parameters
//     to the formats as expected by the method (if the method can be found in
//     the typeinfo for this object). It then invokes the method through the
//     IDispatch interface and returns the result to the caller. The result
//     object will be converted from the OLE result object. Conversion for
//     arguments and results will try to match original data types, so new OLE
//     objects being returned will be converted to new REXX OLEObject proxy
//     objects accordingly. Also this method will distinguish between
//     property get/put methods and method invocations.
//
//     Besides the argument conversion based either on the type info we will
//     translate a few Object REXX specific methods into OLE specific methods.
//     This currently only affects the Object REXX "AT" and "[]" methods which
//     are translated to the OLE method "Item" used on OLE collections in order
//     to handle OLE collections transparently in Object REXX. This method
//     translation will only be applied when no method with the original name
//     can be found in the object's type info.
//
//     If the OLE object supports the IDispatchEx interface it will be used
//     to facilitate future usage of the OLE interface in scripting engines.
//
//******************************************************************************
RexxMethod3(RexxObjectPtr, OLEObject_Unknown, OSELF, self, CSTRING, msgName, RexxArrayObject, msgArgs)
{
    HRESULT         hResult;
    CHAR            szBuffer[2048];
    CHAR           *pszFunction;
    RexxObjectPtr   arrItem;
    IDispatch      *pDispatch = NULL;
    IDispatchEx    *pDispatchEx = NULL;
    ITypeInfo      *pTypeInfo = NULL;
    MEMBERID        MemId;
    POLECLASSINFO   pClsInfo = NULL;
    RexxClassObject variantClass = NULL;
    size_t          iArgCount;
    unsigned short  wFlags = 0;
    DISPPARAMS      dp;
    VARIANTARG     *pVarArgs = NULL;
    VARIANTARG     *pInputParameters = NULL;
    DISPID          PropPutDispId = DISPID_PROPERTYPUT;
    VARIANT         sResult;
    VARIANT        *pResult;
    EXCEPINFO       sExc;
    unsigned int    uArgErr;
    POLEFUNCINFO    pFuncInfo = NULL;
    BOOL            fFound = FALSE;
    VARTYPE         DestVt;
    RexxObjectPtr   ResultObj = context->Nil();

    if ( !fInitialized )
    {
        OLEInit();
    }

    context->SetObjectVariable("!OUTARRAY", context->Nil());

    iArgCount = context->ArraySize(msgArgs);

    if ( ! getDispatchPtr(context, &pDispatch) )
    {
        return NULLOBJECT;
    }
    getCachedClassInfo(context, &pClsInfo, &pTypeInfo);

    pszFunction = pszStringDupe(msgName);
    if (!pszFunction)
    {
        context->RaiseException0(Rexx_Error_System_resources);
        return NULLOBJECT;
    }

    if ( pszFunction[strlen(pszFunction)-1] == '=' )
    {
        /* property put operation */
        pszFunction[strlen(pszFunction)-1] = 0;
        wFlags = DISPATCH_PROPERTYPUT;
    }

    hResult = pDispatch->QueryInterface(IID_IDispatchEx, (LPVOID*)&pDispatchEx);

    fFound = fFindFunction(pszFunction, pDispatch, pDispatchEx, pTypeInfo,
                           pClsInfo, wFlags, &pFuncInfo, &MemId, iArgCount);

    /* This change honors the function description better; before, the wFlag
     * (invocation kind) was determined by use, regardless of the description.
     * Now, if this is no direct PROPERTYPUT (format: "obj~function=") the
     * invkind of the function description is used.  If no function description
     * is available, we use our "best guess"...
     */
    if (wFlags == 0)
    {
        if (fFound && pFuncInfo)
        {
            // Use the invkind the function description contains.  Note that the
            // DISPATCH_XXX & INVOKE_XXX flags have the same values.
            wFlags = pFuncInfo->invkind;
        }
        else
        {
            wFlags = DISPATCH_METHOD;

            if (iArgCount == 0)
            {
                /* this could be a property get or a dispatch method */
                wFlags |= DISPATCH_PROPERTYGET;
            }
            else
            {
                // we found something the 1st time - had arguments -
                // but we did not get a function description. this time,
                // we call fFindFunction to return function description
                // regardless of the number of args - it will work
                // if there is type info for this method at all
                fFound = fFindFunction(pszFunction, pDispatch, pDispatchEx, pTypeInfo,
                                       pClsInfo, wFlags, &pFuncInfo, &MemId, -1);
            }
        }
    }

    /* replace methods '[]' and 'at' with the method 'Item' if they */
    /* could not be found */
    if (!fFound && (wFlags & DISPATCH_METHOD) &&
        ((stricmp(pszFunction, "AT") == 0) ||
         (stricmp(pszFunction, "[]") == 0)) )
    {
        fFound = fFindFunction("Item", pDispatch, pDispatchEx, pTypeInfo,
                               pClsInfo, wFlags, &pFuncInfo, &MemId, iArgCount);
    }

    /* The below was used to allow setting a property by calling it like a
     * method. This creates problems, because it might "shadow" a method with
     * the same signature. Therefore it is disabled...
     */
    /*
    // call of PROPERTYPUT with braces: e.g. ~visible(.true)
    if (fFound && pFuncInfo)
    {
        // are we sure this is a PROPERTYGET?
        if (pFuncInfo->invkind == DISPATCH_PROPERTYGET)
        {
            // check if parameter list is valid. but how?
            if (iArgCount > (pFuncInfo->iParmCount - pFuncInfo->iOptParms) )
            {
                // oh, we must rethink our call: this must be a property put, and not a get!!!
                wFlags = DISPATCH_PROPERTYPUT;
                fFound = fFindFunction(pszFunction, pDispatch, pDispatchEx, pTypeInfo, pClsInfo,
                                       wFlags, &pFuncInfo, &MemId);
            }
        }
    }
    */

    // This memory is no longer needed.
    ORexxOleFree(pszFunction);

    if ( ! fFound )
    {
        context->RaiseException2(Rexx_Error_No_method_name, self, context->NewStringFromAsciiz(msgName));
        goto clean_up_exit;
    }

    /* now assemble the parameters for the function call */
    if ( iArgCount > 0 )
    {
        // allocate twice the size to store original variants in 2nd half
        pVarArgs = (VARIANTARG *) ORexxOleAlloc(sizeof(VARIANTARG) * iArgCount * 2);
        if ( pVarArgs == NULL )
        {
            context->RaiseException0(Rexx_Error_System_resources);
            goto clean_up_exit;
        }
        // point to second half
        pInputParameters = pVarArgs + iArgCount;
    }

    size_t i;
    for (i = 0; i < iArgCount; i++)
    {
        /* arguments are filled in from the end of the array */
        VariantInit(&(pVarArgs[iArgCount - i - 1]));

        arrItem = context->ArrayAt(msgArgs, i + 1);

        if (pTypeInfo && pFuncInfo)
        {
            DestVt = (i < (size_t)pFuncInfo->iParmCount) ? pFuncInfo->pOptVt[i] : VT_EMPTY;
        }
        else
        {
            DestVt = VT_EMPTY;
        }

        if ( ! Rexx2Variant(context->threadContext, arrItem, &(pVarArgs[iArgCount - i - 1]), DestVt, i + 1) )
        {
            // An exception was raised.
            goto clean_up_exit;
        }

        /* If an out parameter, the variant must be passed as a reference. The
         * original input variant is saved so that, if a new value is returned,
         * the original can be freed.
         */
        if ( isOutParam(context->threadContext, arrItem, pFuncInfo, i) )
        {
            referenceVariant(&pVarArgs[iArgCount - i - 1]);
            memcpy(&pInputParameters[iArgCount - i - 1],&pVarArgs[iArgCount - i- 1],
                   sizeof(VARIANT));
        }
    }

    /* if we have a property put then the new property value needs to be a named
     * argument
     */
    if (wFlags == DISPATCH_PROPERTYPUT)
    {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &PropPutDispId;
        // without this check, property put is hardcoded to PROPERTYPUT...  ...bad.
        if (fFound && pFuncInfo)
        {
            /* use the invkind the function description contains */
            wFlags = pFuncInfo->invkind; // DISPATCH_... & INVOKE_... are the same (values 2,4,8)
        }
    }
    else
    {
        dp.cNamedArgs = 0;
        dp.rgdispidNamedArgs = NULL;
    }

    dp.cArgs = (UINT)iArgCount;
    dp.rgvarg = pVarArgs;
    VariantInit(&sResult);

    /* Zero out the exception structure, some OLE Automation objects are not well
     * behaved.
     */
    ZeroMemory(&sExc, sizeof(EXCEPINFO));

    pResult = &sResult;
    if (pTypeInfo && pFuncInfo)
    {
        if (pFuncInfo->FuncVt == VT_VOID)
        {
            pResult = NULL; /* function has no return code! */
        }

        if (pFuncInfo->invkind & INVOKE_PROPERTYGET)
        {
            wFlags |= DISPATCH_PROPERTYGET; /* this might be a property get */
        }
    }

    if (pDispatchEx)
    {
        hResult = pDispatchEx->InvokeEx(MemId, LOCALE_USER_DEFAULT,
                                        wFlags, &dp, pResult, &sExc, NULL);
    }
    else
    {
        hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                    wFlags, &dp, pResult, &sExc, &uArgErr);
    }

    /* maybe this is a property get with arguments */
    if ((hResult == DISP_E_MEMBERNOTFOUND) && iArgCount)
    {
        hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                    wFlags | DISPATCH_PROPERTYGET, &dp,
                                    pResult, &sExc, &uArgErr);
    }

    /* if function has no return value, try again */
    if (hResult == DISP_E_MEMBERNOTFOUND)
    {
        hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                    wFlags, &dp, NULL, &sExc, &uArgErr);
    }
    // needed for instance of tests
    variantClass = context->FindClass("OLEVARIANT");

    for (i = 0; i < dp.cArgs; i++)
    {
        arrItem = context->ArrayAt(msgArgs, i + 1);

        /* was this an out parameter? */
        if ( isOutParam(context->threadContext, arrItem, pFuncInfo, i) )
        {
            /* yes, then change the REXX object to a new state */
            RexxObjectPtr   outObject;
            RexxObjectPtr   outArray = context->GetObjectVariable("!OUTARRAY");

            if (outArray == context->Nil())
            {
                outArray = context->NewArray(1);
                context->SetObjectVariable("!OUTARRAY", outArray);
            }

            outObject = Variant2Rexx(context->threadContext, &(dp.rgvarg[dp.cArgs-i-1]));
            if ( outObject == NULLOBJECT )
            {
                // Variant2Rexx() has raised an exception.
                goto clean_up_exit;
            }

            context->SendMessage1(outArray,"APPEND", outObject);

            // If the arg was ommitted (arrItem), it can not be an out
            // parameter, but to be safe, check for NULLOBJECT.
            if (arrItem != NULLOBJECT && context->IsInstanceOf(arrItem, variantClass))
            {
                context->SendMessage1(arrItem, "!VARVALUE_=", outObject);
            }

            // if the call changed an out parameter, we have to clear the original variant that
            // was overwritten
            if (memcmp(&pInputParameters[iArgCount - i - 1],&pVarArgs[iArgCount - i - 1],sizeof(VARIANT)))
            {
                dereferenceVariant(&pInputParameters[iArgCount - i - 1]);
                handleVariantClear(context, &pInputParameters[iArgCount - i - 1], arrItem);
            }
            else
            {
                dereferenceVariant(&dp.rgvarg[iArgCount - i - 1]);
            }
        }

        /* Clear the argument and, if an OLEVariant, reset the clear variant flag.
         */
        handleVariantClear(context, &(dp.rgvarg[dp.cArgs-i-1]), arrItem);

        if (arrItem != NULLOBJECT && context->IsInstanceOf(arrItem, variantClass))
        {
            context->SendMessage1(arrItem, "!CLEARVARIANT_=", context->True());
            context->SendMessage1(arrItem, "!VARIANTPOINTER_=", context->NewPointer(NULL));
        }
    }

    if (hResult == S_OK)
    {
        // If Variant2Rexx() raises an exception we drop through and catch the
        // clean up code and are okay here.
        ResultObj = Variant2Rexx(context->threadContext, &sResult);
    }
    else
    {
        // An error occured.  We raise an exception and then drop through to
        // catch the clean up code.  For some errors, uArgErr contains the index
        // of the wrong parameter.
        switch ( hResult )
        {
            case DISP_E_BADPARAMCOUNT:
                context->RaiseException0(Rexx_Error_Argument_Count_Mismatch);
                break;
            case DISP_E_BADVARTYPE:
                context->RaiseException0(Rexx_Error_Invalid_Variant);
                break;
            case DISP_E_EXCEPTION:
                formatDispatchException(&sExc, szBuffer);
                context->RaiseException1(Rexx_Error_OLE_Exception, context->NewStringFromAsciiz(szBuffer));
                break;
            case DISP_E_MEMBERNOTFOUND:
                context->RaiseException0(Rexx_Error_Unknown_OLE_Method);
                break;
            case DISP_E_OVERFLOW:
                context->RaiseException0(Rexx_Error_Coercion_Failed_Overflow);
                break;
            case DISP_E_TYPEMISMATCH:
                context->RaiseException1(Rexx_Error_Coercion_Failed_Type_Mismatch, context->WholeNumberToObject(uArgErr + 1));
                break;
            case DISP_E_PARAMNOTOPTIONAL:
                context->RaiseException0(Rexx_Error_Parameter_Omitted);
                break;
            case RPC_E_DISCONNECTED:
                context->RaiseException0(Rexx_Error_Client_Disconnected_From_Server);
                break;
            case DISP_E_NONAMEDARGS:
            case DISP_E_UNKNOWNLCID:
            case DISP_E_UNKNOWNINTERFACE:
            case DISP_E_PARAMNOTFOUND:
            default:
                sprintf(szBuffer, "%8.8X", hResult);
                context->RaiseException1(Rexx_Error_Unknown_OLE_Error, context->NewStringFromAsciiz(szBuffer));
                break;
        }
    }

clean_up_exit:

    // Free the argument array.
    ORexxOleFree(pVarArgs);

    // Remove reference count for result object.
    VariantClear(&sResult);

    if (pDispatchEx)
    {
        pDispatchEx->Release();
    }
    return ResultObj;
}

/**
 * Determine if the user wants to override OLEObject's automatic conversion of
 * ooRexx objects to variants.
 *
 * The use of an OLEVariant object for a parameter signals that the ooRexx
 * programmer may want to override the default type conversion.  Often simply
 * specifying what the ooRexx object is to be converted to is sufficient.  Cases
 * where the default conversion is known to be wrong are handled here and then
 * the caller is informed that the conversion has already taken place.
 *
 * @param pVariant   The variant receiving the converted ooRexx object.
 *
 * @param RxObject   The ooRexx object to be converted.  If this is an
 *                   OLEVariant object, the actual object to convert is
 *                   contained within the OLEVariant object.
 *
 * @param DestVt     The VT type that the automatic conversion believes the
 *                   ooRexx object should be coerced to.
 *
 * @param pRxObject  [Returned] The real ooRexx object to convert.
 *
 * @param pDestVt    [Returned] The real VT type to coerce the ooRexx object to.
 *
 * @return           SuccessReturn: the conversion is complete.
 *
 *                   FailureReturn: the conversion is not complete - continue
 *                   with the automatic conversion.
 *
 *                   ExceptionReturn: an exception is raised.
 */
ThreeStateReturn checkForOverride(RexxThreadContext *context, VARIANT *pVariant, RexxObjectPtr RxObject,
                                  VARTYPE DestVt, RexxObjectPtr *pRxObject, VARTYPE *pDestVt )
{
    ThreeStateReturn converted = FailureReturn;

    // needed for instance of tests
    RexxClassObject variantClass = context->FindClass("OLEVARIANT");

    if ( RxObject == NULLOBJECT || ! context->IsInstanceOf(RxObject, variantClass) )
    {
        *pRxObject = RxObject;
        *pDestVt   = DestVt;
    }
    else
    {
        RexxObjectPtr tmpRxObj = context->SendMessage0(RxObject, "!_VT_");

        *pRxObject = context->SendMessage0(RxObject, "!VARVALUE_");
        if ( tmpRxObj == context->Nil() )
        {
            /* Do not override default conversion. */
            *pDestVt = DestVt;
        }
        else
        {
            wholenumber_t intval;
            context->ObjectToWholeNumber(tmpRxObj, &intval);

            *pDestVt = (VARTYPE)intval;

            switch ( *pDestVt & VT_TYPEMASK )
            {
                case VT_NULL :
                    V_VT(pVariant) = VT_NULL;
                    converted = SuccessReturn;
                    break;

                case VT_EMPTY :
                    V_VT(pVariant) = VT_EMPTY;
                    converted = SuccessReturn;
                    break;

                case VT_DISPATCH :
                    if ( *pRxObject == context->Nil() || *pRxObject == NULLOBJECT )
                    {
                        IDispatch **ppDisp = NULL;

                        if ( *pDestVt & VT_BYREF )
                        {
                            ppDisp = (IDispatch **)ORexxOleAlloc(sizeof(IDispatch **));
                            if ( ! ppDisp )
                            {
                                context->RaiseException0(Rexx_Error_System_resources);
                                return ExceptionReturn;
                            }
                            *ppDisp = (IDispatch *)NULL;

                            V_VT(pVariant) = VT_DISPATCH | VT_BYREF;
                            V_DISPATCHREF(pVariant) = ppDisp;
                        }
                        else
                        {
                            V_VT(pVariant) = VT_DISPATCH;
                            V_DISPATCH(pVariant) = NULL;
                        }
                        /* ooRexx, not VariantClear, must clear this variant. */
                        context->SendMessage1(RxObject, "!CLEARVARIANT_=", context->False());
                        context->SendMessage1(RxObject, "!VARIANTPOINTER_=", context->NewPointer(ppDisp));
                        converted = SuccessReturn;
                        break;
                    }
                    /* Let default conversion handle non-nil. */
                    break;

                case VT_UNKNOWN :
                    if ( *pRxObject == context->Nil() || *pRxObject == NULLOBJECT )
                    {
                        IUnknown **ppU = NULL;

                        if ( *pDestVt & VT_BYREF )
                        {
                            ppU = (IUnknown **)ORexxOleAlloc(sizeof(IUnknown **));
                            if ( ! ppU )
                            {
                                context->RaiseException0(Rexx_Error_System_resources);
                                return ExceptionReturn;
                            }

                            *ppU = (IUnknown *)NULL;
                            V_VT(pVariant) = VT_UNKNOWN | VT_BYREF;
                            V_UNKNOWNREF(pVariant) = ppU;
                        }
                        else
                        {
                            V_VT(pVariant) = VT_UNKNOWN;
                            V_UNKNOWN(pVariant) = NULL;
                        }
                        /* ooRexx, not VariantClear, must clear this variant. */
                        context->SendMessage1(RxObject, "!CLEARVARIANT_=", context->False());
                        context->SendMessage1(RxObject, "!VARIANTPOINTER_=", context->NewPointer(ppU));
                        converted = SuccessReturn;
                        break;
                    }
                    /* Let default conversion handle non-nil. */
                    break;

                default :
                    /* Let default conversion handle all other cases. */
                    break;
            }
        }
    }
    return converted;
}

/**
 * Determine, based on the information OLEObject has, if the ooRexx object used
 * as a parameter in an IDispatch invocation should be an out parameter.
 *
 * @param param      The parameter to be inspected.  If this is an OLEVariant
 *                   object, the ooRexx programmer may be overriding what
 *                   OLEObject thinks it knows.
 *
 * @param pFuncInfo  A, possible, pointer to a function information block with
 *                   details concerning this parameter.
 *
 * @param i          The position (index) of the parameter in the method
 *                   invocation.  This will also be the index of the parameter
 *                   in the function information block.  Note howerver that it
 *                   is possible that the index is not valid for the function
 *                   information block.
 *
 * @return           True if there is enough information to be certain the
 *                   paramter is an out parameter, otherwise false.
 */
BOOL isOutParam(RexxThreadContext *context, RexxObjectPtr param, POLEFUNCINFO pFuncInfo, size_t i )
{
    USHORT  paramFlags = PARAMFLAG_NONE;
    BOOL    overridden = FALSE;

    // needed for instance of tests
    RexxClassObject variantClass = context->FindClass("OLEVARIANT");

    if ( param != NULLOBJECT && context->IsInstanceOf(param, variantClass) )
    {
        RexxObjectPtr tmpRxObj = context->SendMessage0(param, "!_PFLAGS_");
        if ( tmpRxObj != context->Nil() )
        {
            wholenumber_t intval;
            context->ObjectToWholeNumber(tmpRxObj, &intval);

            paramFlags = (USHORT)intval;
            overridden = TRUE;
        }
    }

    if ( !overridden && pFuncInfo && i < (size_t)pFuncInfo->iParmCount )
    {
        paramFlags = pFuncInfo->pusOptFlags[i];
    }

    return((paramFlags & PARAMFLAG_FOUT) && !(paramFlags & PARAMFLAG_FRETVAL));
}

/**
 * Helper function to clear a variant when it may not be safe to pass the
 * variant to VariantClear.
 *
 * Assumes: Pass by reference variants have already been dereferenced and
 * therefore V_ISBYREF(pVariant) == FALSE.
 *
 * @param pVariant  The variant to clear.
 *
 * @param RxObject  If this is an ooRexx OLEVariant object, it contains the flag
 *                  signaling whether it is okay to pass the variant to
 *                  VariantClear.
 *
 * @return          VOID
 */
VOID handleVariantClear(RexxMethodContext *context, VARIANT *pVariant, RexxObjectPtr RxObject )
{
    bool useVariantClear = true;

    if ( ! okayToClear(context, RxObject) )
    {
        /* This reverses work done in Rexx2Variant when the parameter is an
         * OLEVariant object.  Memory allocated in checkForOverride(), that needs to
         * be freed, is freed here.
         */
        void *variantPointer = context->PointerValue((RexxPointerObject)context->SendMessage0(RxObject, "!VARIANTPOINTER_"));

        switch ( V_VT(pVariant) & VT_TYPEMASK )
        {
            case VT_DISPATCH :
                if ( V_DISPATCH(pVariant) == NULL)
                {
                    useVariantClear = false;
                }
                else
                {
                    IDispatch  *pDispatch = (IDispatch *)variantPointer;
                    if (pDispatch == V_DISPATCH(pVariant) )
                    {
                        ORexxOleFree(V_DISPATCH(pVariant));
                        useVariantClear = false;
                    }
                }
                break;

            case VT_UNKNOWN :
                if ( V_UNKNOWN(pVariant) == NULL)
                {
                    useVariantClear = false;
                }
                else
                {
                    IUnknown *pUnknown = (IUnknown *)variantPointer;
                    if (pUnknown == V_UNKNOWN(pVariant) )
                    {
                        ORexxOleFree(V_UNKNOWN(pVariant));
                        useVariantClear = false;
                    }
                }
                break;

            default :
                break;
        }
    }

    if ( useVariantClear )
    {
        VariantClear(pVariant);
    }
    else
    {
        V_VT(pVariant) = VT_EMPTY;
    }
}

/**
 * Check if it is okay to use VariantClear.
 */
__inline BOOL okayToClear(RexxMethodContext *context, RexxObjectPtr RxObject )
{
    // needed for instance of tests
    RexxClassObject variantClass = context->FindClass("OLEVARIANT");

    if ( RxObject != NULLOBJECT && context->IsInstanceOf(RxObject, variantClass) )
    {
        return (context->SendMessage0(RxObject, "!CLEARVARIANT_") == context->True());
    }
    return TRUE;
}

/**
 * Convenience function to format the information returned from
 * IDispatch::Invoke for DISP_E_EXCEPTION.
 *
 * @param pInfo   The exception information structure.
 * @param buffer  Buffer in which to return the formatted information.
 */
static void formatDispatchException(EXCEPINFO *pInfo, char *buffer)
{
    const char *fmt = "Code: %08x Source: %S Description: %S";
    BSTR bstrUnavail = lpAnsiToUnicode("unavailable", sizeof("unavailable"));

    /* If there is a deferred fill-in routine, call it */
    if ( pInfo->pfnDeferredFillIn )
    {
        pInfo->pfnDeferredFillIn(pInfo);
    }

    if ( pInfo->wCode )
    {
        fmt = "Code: %04x Source: %S Description: %S";
    }

    sprintf(buffer, fmt,
            pInfo->wCode ? pInfo->wCode : pInfo->scode,
            pInfo->bstrSource == NULL ? bstrUnavail : pInfo->bstrSource,
            pInfo->bstrDescription == NULL ? bstrUnavail : pInfo->bstrDescription);

    SysFreeString(pInfo->bstrSource);
    SysFreeString(pInfo->bstrDescription);
    SysFreeString(pInfo->bstrHelpFile);
    ORexxOleFree(bstrUnavail);
}

//******************************************************************************
// Method:  OLEObject_Request
//
//   Arguments:
//     self - A pointer to self
//     classID (RexxObjectPtr) - The string identifier of the requested class.
//                            (currently only ARRAY is supported)
//
//   Returned:
//     returnObject (RexxObjectPtr) - The result of the conversion.
//
//   Notes:
//     This method will try to convert the OLE object into the requested class.
//     Currently the only supported target class is the ARRAY class for OLE
//     collection objects. An enumerator object that should be provided by any
//     OLE collection will be used to retrieve the single elements. If this does
//     not succeed, the following strategy is employed:
//     OLE collection objects all provide the methods "Count" specifying the
//     number of items in the collection and the method "Item" to retrieve an
//     individual item. Zero-based or one-based indices are expected.
//
//******************************************************************************
RexxMethod2(RexxObjectPtr,                // Return type
            OLEObject_Request,         // Object_method name
            OSELF, self,               // Pointer to self
            CSTRING, classID)          // Name of OSA event to be sent
{
    HRESULT         hResult;
    RexxObjectPtr   ResultObj = context->Nil();
    RexxObjectPtr   RxItem;
    IDispatch      *pDispatch = NULL;
    IDispatchEx    *pDispatchEx = NULL;
    ITypeInfo      *pTypeInfo = NULL;
    MEMBERID        MemId;
    POLECLASSINFO   pClsInfo = NULL;
    BOOL            fFound = FALSE;
    DISPPARAMS      dp;
    VARIANT         sResult;
    VARIANT         sTempVariant;
    EXCEPINFO       sExc;
    unsigned int    uArgErr;
    INT             iItemCount;
    INT             iIdx;
    INT             iIdxBaseShift = 1;

    if ( !fInitialized )
    {
        OLEInit();
    }

    if ( ! getDispatchPtr(context, &pDispatch) )
    {
        return NULLOBJECT;
    }
    getCachedClassInfo(context, &pClsInfo, &pTypeInfo);

    if (stricmp(classID, "ARRAY") == 0)
    {
        /* first check if there is a _NewEnum method and use this */
        fFound = fFindFunction("_NewEnum", pDispatch, pDispatchEx, pTypeInfo,
                               pClsInfo, 0, NULL, &MemId, -1);

        if (fFound)
        {
            /* get the count of items in the collection */
            dp.cNamedArgs = 0;
            dp.rgdispidNamedArgs = NULL;
            dp.cArgs = 0;
            dp.rgvarg = NULL;
            VariantInit(&sResult);

            // some objects behave very badly, therefore this code (ugly!):
            if (MemId == -1)
            {
                MemId = -4;
            }

            hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                        DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                                        &dp, &sResult, &sExc, &uArgErr);
            if (SUCCEEDED(hResult))
            {
                IUnknown     *pUnknown=V_UNKNOWN(&sResult);
                IEnumVARIANT *pEnum = NULL;

                pUnknown->AddRef();  // VariantClear will remove one reference!
                // clear result from last invocation
                VariantClear(&sResult);

                // get IEnumVARIANT interface
                hResult = pUnknown->QueryInterface(IID_IEnumVARIANT, (LPVOID*) &pEnum);
                if (hResult == S_OK)
                {
                    ULONG lFetched = 0;
                    iItemCount = 1;

                    hResult = pEnum->Reset();     // set enumerator to first item
                    RexxArrayObject ResultArr = context->NewArray(0); // create REXX array

                    VariantInit(&sResult);

                    while ( pEnum->Next(1, &sResult, &lFetched) == S_OK )
                    {
                        RxItem = Variant2Rexx(context->threadContext, &sResult);
                        VariantClear(&sResult);

                        if ( RxItem == NULLOBJECT )
                        {
                            // Variant2Rexx() has rasised an exception.
                            pEnum->Release();
                            pUnknown->Release();
                            return NULLOBJECT;
                        }
                        context->ArrayPut(ResultArr, RxItem, iItemCount++);
                    }

                    pEnum->Release();   // enumerator release returns 1, is that ok?
                    pUnknown->Release();
                    return ResultArr;
                }
            }
            else
            {
                // ADSI has it's own special way of error reporting (...)
                // these will all get swallowed and an empty Array will be returned
                if (hResult == 0x80020009)
                {
                    switch (sExc.scode)
                    {
                        case 0x800704b8:        // IADsContainer does not contain items
                        case 0x800704c6:        // Network not present
                            return context->NewArray(0);
                            break;
                            // more error codes are expected...
                        default:
                            break;
                    }
                }
            }
        }

        /* no enumerator supplied, just assume integer indices and go...     */
        /* see if the OLEObject supports Count/Item                          */
        if (!fFound)
        {
            fFound = fFindFunction("Count", pDispatch, pDispatchEx, pTypeInfo,
                                   pClsInfo, 0, NULL, &MemId, -1);

            if (fFound)
            {
                /* get the count of items in the collection */
                dp.cNamedArgs = 0;
                dp.rgdispidNamedArgs = NULL;
                dp.cArgs = 0;
                dp.rgvarg = NULL;
                VariantInit(&sResult);

                hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                            DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                                            &dp, &sResult, &sExc, &uArgErr);

                if (hResult == S_OK)
                {
                    VariantInit(&sTempVariant);

                    if ( VariantChangeType(&sTempVariant, &sResult, 0, VT_I4) == S_OK )
                    {
                        iItemCount = sTempVariant.lVal;
                    }
                    else
                    {
                        // VARIANT change to integer failed (should not happen, really)
                        context->RaiseException1(Rexx_Error_Execution_noarray, self);
                        VariantClear(&sResult);
                        VariantClear(&sTempVariant);
                        return NULLOBJECT;
                    }

                    VariantClear(&sResult);
                    VariantClear(&sTempVariant);

                    fFound = fFindFunction("Item", pDispatch, pDispatchEx, pTypeInfo,
                                           pClsInfo, 0, NULL, &MemId, -1);
                    if (fFound)
                    {
                        /* Count & Item are understood -> return an array in any case */
                        ResultObj = context->NewArray(iItemCount);
                        /* fill the array with the items */
                        for (iIdx = 0; iIdx < iItemCount; iIdx++)
                        {
                            /* get the pointer to the nth item in the collection */
                            VariantInit(&sTempVariant);
                            V_VT(&sTempVariant) = VT_I4;
                            V_I4(&sTempVariant) = iIdx;
                            dp.cNamedArgs = 0;
                            dp.rgdispidNamedArgs = NULL;
                            dp.cArgs = 1;
                            dp.rgvarg = &sTempVariant;
                            VariantInit(&sResult);

                            hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                                        DISPATCH_METHOD, &dp, &sResult,
                                                        &sExc, &uArgErr);

                            // if Object~Item(0) failed, assume a one-based index
                            if ( FAILED(hResult) && iIdx == 0 )
                            {
                                iIdxBaseShift=0;
                                iItemCount++;
                                continue;
                            }
                            if (hResult == S_OK)
                            {
                                /* create a new REXX object from the result */
                                RxItem = Variant2Rexx(context->threadContext, &sResult);
                                if ( RxItem == NULLOBJECT )
                                {
                                    // Variant2Rexx() raised an exception.
                                    VariantClear(&sResult);
                                    VariantClear(&sTempVariant);
                                    return NULLOBJECT;
                                }
                                context->ArrayPut((RexxArrayObject)ResultObj, RxItem, iIdx + iIdxBaseShift);
                            }

                            VariantClear(&sResult);
                            VariantClear(&sTempVariant);
                        }
                    }
                }
            }
        }
    }
    return ResultObj;
}


//******************************************************************************
// Method:  OLEObject_GetVar
//
//   Arguments:
//     self - A pointer to self
//     varName (CSTRING) - The name of the object variable to retrieve.
//
//   Returned:
//     returnObject (RexxObjectPtr) - The result of the GETVAR operation
//
//   Notes:
//     This method will retrieve the object variable stored with the current
//     OLE proxy object under the specified name. This allows other methods
//     to get hold of internal data stored with other REXX OLE objects they
//     are handling.
//
//******************************************************************************
RexxMethod2(RexxObjectPtr,                // Return type
            OLEObject_GetVar,          // Object_method name
            OSELF, self,               // Pointer to self
            CSTRING, varName)          // string defining variable to query
{
    RexxObjectPtr RxString = context->GetObjectVariable(varName);
    if ( RxString != NULLOBJECT)
    {
        return RxString;
    }
    else
    {
        return context->Nil();
    }
}


//******************************************************************************
// Method:  OLEObject_GetConst
//
//   Arguments:
//     self - A pointer to self
//     constName (CSTRING) - The name of the object constant to retrieve.
//
//   Returned:
//     returnObject (RexxObjectPtr) - The converted Object REXX object referring
//                                 to the object constant or .Nil.
//                             OR: a stem with all names and values of known
//                                 constants if constname was not specified
//
//   Notes:
//     This method will retrieve the object constant defined in the type
//     library of the object. If the constant with this name can not be
//     found .NIL will be returned indicating failure of the search.
//
//     It is *only* possible to retrieve this information through the Type
//     Library of the OLE object.  It is not required that an OLE object have a
//     type library.  If we have no stored internal class info, then it is not
//     possible to determine the value of the constant.
//
//******************************************************************************
RexxMethod1(RexxObjectPtr, OLEObject_GetConst, OPTIONAL_CSTRING, constName)
{
    ITypeInfo       *pTypeInfo = NULL;
    POLECLASSINFO   pClsInfo = NULL;
    POLECONSTINFO   pConstInfo = NULL;
    RexxObjectPtr   rxDefault = context->Nil();
    RexxObjectPtr   rxTemp = NULLOBJECT;

    if ( !fInitialized )
    {
        OLEInit();
    }

    /** Try to retrieve the internal class info through either the !CLSID  or
     *  !TYPEINFO variables.
     */
    getCachedClassInfo(context, &pClsInfo, &pTypeInfo);

    if (pClsInfo == NULL)
    {
        return rxDefault;
    }

    if (constName != NULL)
    {
        if ( fFindConstant(constName, pClsInfo, &pConstInfo) )
        {
            // If Variant2Rexx() raises an exception we are just returning
            // anyway, so we are good.
            return Variant2Rexx(context->threadContext, &(pConstInfo->sValue));
        }
        else
        {
            return rxDefault;
        }
    }
    else
    {
        char upperBuffer[256]="!";
        pConstInfo = pClsInfo->pConstInfo;
        RexxStemObject rxResult = context->NewStem(NULL);

        while ( pConstInfo && rxResult != context->Nil() )
        {
            /* hide constants that start with _ (MS convention) */
            if (pConstInfo->pszConstName[0] != '_')
            {
                strcpy(upperBuffer+1,pConstInfo->pszConstName);
                strupr(upperBuffer);

                rxTemp = Variant2Rexx(context->threadContext, &pConstInfo->sValue);
                if ( rxTemp == NULLOBJECT )
                {
                    return NULLOBJECT;
                }
                context->SetStemElement(rxResult, upperBuffer, rxTemp);
            }
            pConstInfo = pConstInfo->pNext;
        }
        return rxResult;
    }
}

/********************************************************/
/* following are functions that support GetKnownMethods */
/********************************************************/

/* Insert type information into a REXX array */
void InsertTypeInfo(RexxMethodContext *context, ITypeInfo *pTypeInfo, TYPEATTR *pTypeAttr, RexxStemObject RxResult, int *pIndex)
{
    HRESULT      hResult;
    BSTR         bName;
    BSTR         bDocString;
    BSTR        *pbStrings;
    CHAR         szBuffer[2048];
    CHAR         szSmallBuffer[256];
    FUNCDESC    *pFuncDesc;
    INT          iIndex;
    INT          i;
    unsigned int uFlags, wFlags;

    // run through all functions and get their description
    for (iIndex=0;iIndex<pTypeAttr->cFuncs;iIndex++)
    {
        pFuncDesc = NULL;
        hResult = pTypeInfo->GetFuncDesc(iIndex,&pFuncDesc);
        if (hResult == S_OK)
        {

            hResult = pTypeInfo->GetDocumentation(pFuncDesc->memid,&bName,&bDocString,0,0);
            // display only if this is not a restricted or hidden function and if it does not begin
            // with a _ (according to "Inside OLE" p. 664: "The leading underscore, according to OLE
            // Automation, means that the method or property is hidden and should not be shown in
            // any user interface that a controller might present.")
            if (!(pFuncDesc->wFuncFlags & FUNCFLAG_FRESTRICTED) &&
                !(pFuncDesc->wFuncFlags & FUNCFLAG_FHIDDEN) &&
                bName[0] != '_')
            {
                szBuffer[0] = szSmallBuffer[0] = 0x00;
                (*pIndex)++;

                pbStrings=new BSTR[pFuncDesc->cParams + 1];  // one more for method name
                // get names of parameters
                // if it fails, then output will place "<unnamed>" as the name(s)
                hResult = pTypeInfo->GetNames(pFuncDesc->memid,pbStrings,pFuncDesc->cParams+1,&uFlags);

                // store member id
                sprintf(szBuffer,"%08x",pFuncDesc->memid);
                sprintf(szSmallBuffer,"%d.!MEMID",*pIndex);
                context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(szBuffer));

                // store return type
                sprintf(szSmallBuffer,"%d.!RETTYPE",*pIndex);
                context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(pszDbgVarType(pFuncDesc->elemdescFunc.tdesc.vt)));

                // store invoke kind
                sprintf(szSmallBuffer,"%d.!INVKIND",*pIndex);
                context->SetStemElement(RxResult, szSmallBuffer, context->WholeNumberToObject(pFuncDesc->invkind));

                if (bName)
                {
                    // store name
                    sprintf(szBuffer,"%S",bName);
                    sprintf(szSmallBuffer,"%d.!NAME",*pIndex);
                    context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(szBuffer));

                }
                else
                { // could not retrieve name of method (should never happen, really)
                    sprintf(szSmallBuffer,"%d.!NAME",*pIndex);
                    context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz("???"));
                }

                // store doc string
                if (bDocString)
                {
                    sprintf(szBuffer,"%S",bDocString);
                    sprintf(szSmallBuffer,"%d.!DOC",*pIndex);
                    context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(szBuffer));
                }

                sprintf(szSmallBuffer,"%d.!PARAMS.0",*pIndex);
                context->SetStemElement(RxResult, szSmallBuffer, context->WholeNumberToObject(pFuncDesc->cParams));
                for (i=0;i<pFuncDesc->cParams;i++)
                {

                    szBuffer[0]=0x00;
                    // display in/out parameters
                    wFlags=pFuncDesc->lprgelemdescParam[i].paramdesc.wParamFlags;
                    if (wFlags || i >= pFuncDesc->cParams - pFuncDesc->cParamsOpt)
                    {
                        strcat(szBuffer,"[");
                        if (wFlags & PARAMFLAG_FIN)
                            strcat(szBuffer,"in,");
                        if (wFlags & PARAMFLAG_FOUT)
                            strcat(szBuffer,"out,");
                        if (i >= pFuncDesc->cParams - pFuncDesc->cParamsOpt)
                            strcat(szBuffer,"opt,");
                        szBuffer[strlen(szBuffer)-1]=0x00; // remove last comma
                        strcat(szBuffer,"]");
                    }
                    sprintf(szSmallBuffer,"%d.!PARAMS.%d.!FLAGS",*pIndex,i+1);
                    context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(szBuffer));

                    // display variant type
                    sprintf(szSmallBuffer,"%d.!PARAMS.%d.!TYPE",*pIndex,i+1);
                    context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(pszDbgVarType(pFuncDesc->lprgelemdescParam[i].tdesc.vt)));

                    // display name
                    if (i+1 < (int) uFlags)
                    {
                        sprintf(szBuffer,"%S",pbStrings[i+1]);
                    }
                    else
                    {
                        sprintf(szBuffer,"<unnamed>");
                    }

                    sprintf(szSmallBuffer,"%d.!PARAMS.%d.!NAME",*pIndex,i+1);
                    context->SetStemElement(RxResult, szSmallBuffer, context->NewStringFromAsciiz(szBuffer));
                }

                SysFreeString(bName);
                SysFreeString(bDocString);
                // free the BSTRs
                for (i=0;i < (int) uFlags;i++)
                {
                    SysFreeString(pbStrings[i]);
                }
                delete []pbStrings;

            }
        }

        if (pFuncDesc)
        {
            pTypeInfo->ReleaseFuncDesc(pFuncDesc);
        }
    }
}





//******************************************************************************
// Method:  OLEObject_GetKnownMethods
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     returnObject (RexxObjectPtr) - A Rexx stem with the names of all known
//                                 methods or .Nil.
//
//   Notes:
//     This method will retrieve the needed information from the object's
//     type information
//
//******************************************************************************
RexxMethod1(RexxObjectPtr,                // Return type
            OLEObject_GetKnownMethods, // Object_method name
            OSELF, self)               // Pointer to self
{
    IDispatch       *pDispatch = NULL;
    ITypeInfo       *pTypeInfo = NULL;
    ITypeLib        *pTypeLib = NULL;
    TYPEATTR        *pTypeAttr = NULL;
    UINT             iTypeInfoCount;
    HRESULT          hResult;
    INT              iCount = 0;
    UINT             iTypeIndex;
    CHAR             pszInfoBuffer[2048];

    if ( !fInitialized )
    {
        OLEInit();
    }

    if ( ! getDispatchPtr(context, &pDispatch) )
    {
        return context->Nil();
    }

    hResult = pDispatch->GetTypeInfoCount(&iTypeInfoCount);
    // check if type information is available
    if (iTypeInfoCount && SUCCEEDED(hResult))
    {
        hResult = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);  // AddRef type info pointer
        // did we get a ITypeInfo interface pointer?
        if (pTypeInfo)
        {
            // create a Stem that will contain all info
            RexxStemObject RxResult = context->NewStem(NULL);

            // get type library
            hResult = pTypeInfo->GetContainingTypeLib(&pTypeLib,&iTypeIndex); // AddRef type lib pointer
            if (hResult == S_OK && pTypeLib)
            {
                BSTR         bName, bDoc;
                ITypeInfo   *pTypeInfo2 = NULL;

                // Get the library name and documentation
                hResult = pTypeLib->GetDocumentation(-1,&bName,&bDoc,NULL,NULL);
                if (bName)
                {
                    sprintf(pszInfoBuffer,"%S",bName);
                    context->SetStemElement(RxResult, "!LIBNAME", context->NewStringFromAsciiz(pszInfoBuffer));
                }
                if (bDoc)
                {
                    sprintf(pszInfoBuffer,"%S",bDoc);
                    context->SetStemElement(RxResult, "!LIBDOC", context->NewStringFromAsciiz(pszInfoBuffer));
                }
                SysFreeString(bName);
                SysFreeString(bDoc);

                // Now get the COM class name and documentation
                hResult = pTypeLib->GetDocumentation(iTypeIndex,&bName,&bDoc,NULL,NULL);
                if (bName)
                {
                    sprintf(pszInfoBuffer,"%S",bName);
                    context->SetStemElement(RxResult, "!COCLASSNAME", context->NewStringFromAsciiz(pszInfoBuffer));
                }
                if (bDoc)
                {
                    sprintf(pszInfoBuffer,"%S",bDoc);
                    context->SetStemElement(RxResult, "!COCLASSDOC", context->NewStringFromAsciiz(pszInfoBuffer));
                }

                hResult = pTypeLib->GetTypeInfo(iTypeIndex,&pTypeInfo2);   // AddRef type info pointer2
                if (pTypeInfo2)
                {
                    hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);           // AddRef type attr pointer
                    if (hResult == S_OK)
                    {
                        InsertTypeInfo(context, pTypeInfo2,pTypeAttr,RxResult,&iCount);
                        pTypeInfo2->ReleaseTypeAttr(pTypeAttr);                // Release type attr pointer
                    }
                }

                pTypeInfo2->Release();                                     // Release type info pointer2
                SysFreeString(bName);
                SysFreeString(bDoc);

                context->SetStemElement(RxResult, "0", context->WholeNumberToObject(iCount));
            }

            if (pTypeLib)
            {
                pTypeLib->Release();                           // Release type lib pointer
            }
            pTypeInfo->Release();                                        // Release type info pointer
            return RxResult;
        }
    }

    return context->Nil();
}

/**** Moniker to Object binding ****/

//******************************************************************************
// Method:  OLEObject_GetObject_Class
//
//   Arguments:
//     self - A pointer to self
//     monikerName (RexxObjectPtr) - The moniker name to get an object of
//     optClass (RexxObjectPtr)    - The class from which to create the object
//                                   (optional, must be derived from OLEObject)
//
//   Returned:
//     returnObject (RexxObjectPtr) - an REXX OLE instance or .Nil.
////
//******************************************************************************
RexxMethod3(RexxObjectPtr,                // Return type
            OLEObject_GetObject_Class, // Object_method name
            OSELF, self,               // Pointer to self
            CSTRING,    monikerName,   // Class specifier for new object
            OPTIONAL_RexxObjectPtr, optClass)  // an optional class that is to be used when created
{
    HRESULT      hResult;
    LPOLESTR     lpUniBuffer = NULL;
    IBindCtx    *pBC = NULL;
    IDispatch   *pDispatch = NULL;
    CHAR         szBuffer[2048];
    int          rc = 0;
    long         iLast = iInstanceCount;

    RexxClassObject OLEObjectClass = context->FindClass("OLEOBJECT");

    // if a class argument has been supplied, make sure that it is a class derived
    // from OLEObject. if not, return a nil object!
    if (optClass != NULLOBJECT)
    {
        // verify this is an OLEObject instance
        if (!context->IsInstanceOf(optClass, OLEObjectClass))
        {
            return context->Nil();
        }
    }

    // if no OLE objects exist now, we must call OleInitialize()
    if (iInstanceCount == 0)
    {
        hResult = OleInitialize(NULL);
    }
    iInstanceCount++;

    lpUniBuffer = lpAnsiToUnicode( monikerName, strlen(monikerName) + 1);

    RexxObjectPtr ResultObj = context->Nil();

    if (lpUniBuffer)
    {
        hResult = CreateBindCtx(NULL, &pBC);
        if (SUCCEEDED(hResult))
        {
            DWORD     dwEaten;
            IMoniker *pMoniker = NULL;
            /* create moniker object */
            hResult = MkParseDisplayName(pBC, lpUniBuffer, &dwEaten, &pMoniker);
            if (SUCCEEDED(hResult))
            {
                // on success, BindToObject calls AddRef of target object
                hResult = pMoniker->BindToObject(pBC, NULL, IID_IDispatch, (LPVOID*) &pDispatch);
                if (SUCCEEDED(hResult))
                {
                    sprintf(szBuffer, "IDISPATCH=%p", pDispatch);
                    if (optClass != NULLOBJECT)
                    {
                        ResultObj = context->SendMessage2(optClass, "NEW", context->NewStringFromAsciiz(szBuffer), context->NewStringFromAsciiz("WITHEVENTS"));
                    }
                    else
                    {
                        ResultObj = context->SendMessage1(OLEObjectClass, "NEW", context->NewStringFromAsciiz(szBuffer));
                    }

                    // ~new has called AddRef for the object, so we must Release it here once
                    rc = pDispatch->Release();
                }
                rc = pMoniker->Release();
            }
            rc = pBC->Release();
        }
        ORexxOleFree(lpUniBuffer);
    }

    // if creation failed, we must very likely shut down OLE
    // by calling OleUninitialize()
    if (iLast < iInstanceCount)
    {
        iInstanceCount--;
        if (iInstanceCount == 0)
        {
            OleUninitialize();
        }
    }

    return ResultObj;
}

//******************************************************************************
// Method:  OLEObject_GetKnownMethods_Class
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     returnObject (RexxObjectPtr) - A Rexx stem with the names of all known
//                                 methods or .Nil.
//
//   Notes:
//     This method will retrieve the needed information from type information
//     of the supplied ProgID/ClsID (does not instantiate any OLE objects!)
//   Attention:
//     This is currently DISABLED, see below!!!
//
//******************************************************************************

//******************************************************************************
// There are currently two problems with this:
// a. the major code of the type library has to be extracted from the registry:
//    is the way how it's done now correct? (probably not)
// b. we do not know which type index to use. request this from the user? how
//    will he know how many there are? or return all? this would possibly give a
//    user information on more than one object...
//******************************************************************************
RexxMethod2(RexxObjectPtr,                      // Return type
            OLEObject_GetKnownMethods_Class, // Object_method name
            OSELF, self,                     // Pointer to self
            CSTRING, className)              // string defining class name
{
    RexxObjectPtr RxResult = context->Nil();
    LPOLESTR    lpUniBuffer = NULL;
    PSZ         pszAnsiStr = NULL;
    CHAR        pszInfoBuffer[2048];
    CLSID       clsID;
    HRESULT     hResult;
    TYPEATTR   *pTypeAttr = NULL;
    ITypeLib   *pTypeLib = NULL;
    char        pBuffer[100];
    LPOLESTR    lpOleStrBuffer = NULL;
    LONG        lSize = 100;
    SHORT       sMajor;                        //  Major code of type library
    int         i;
    UINT        iTypeIndex, iTypeCount;
    INT         iCount = 0;

    lpUniBuffer = lpAnsiToUnicode(className, strlen(className) + 1);

    if (lpUniBuffer)
    {
        if ( *className == '{' )
        {
            /* argument is a CLSID */
            hResult = CLSIDFromString(lpUniBuffer, &clsID);
        }
        else
        {
            /* argument is a ProgID */
            hResult = CLSIDFromProgID(lpUniBuffer, &clsID);
        }

        ORexxOleFree( lpUniBuffer );
        lpUniBuffer = NULL;

        /* successfully retrieved CLSID? */
        if (hResult == S_OK)
        {
            // create a unicode representation of CLSID
            hResult = StringFromCLSID(clsID, &lpOleStrBuffer);

            if (hResult == S_OK)
            {
                // create ansi representation of CLSID
                pszAnsiStr = pszUnicodeToAnsi(lpOleStrBuffer);

                if (pszAnsiStr)
                {
                    // get the ProgID from registry
                    sprintf(pBuffer,"CLSID\\%s\\ProgID",pszAnsiStr);
                    if ( RegQueryValue(HKEY_CLASSES_ROOT,pBuffer,pBuffer,&lSize) == ERROR_SUCCESS)
                    {

                        for (i=lSize-1;i>=0;i--)
                        {
                            if (pBuffer[i]=='.')
                            {
                                pBuffer[i]=0x00;
                                break;
                            }
                        }
                        if (i>0)
                        {
                            if (sscanf(pBuffer+i+1,"%d",&sMajor) != 1) sMajor = 1; // assume one if read in fails
                        }

                    }
                    else
                    {
                        sMajor = 1;  // assume 1 if getting progid fails (should never happen)
                    }

                    // get the TypeLib from registry
                    sprintf(pBuffer,"CLSID\\%s\\TypeLib",pszAnsiStr);
                    lSize=100;
                    if ( RegQueryValue(HKEY_CLASSES_ROOT,pBuffer,pBuffer,&lSize) == ERROR_SUCCESS)
                    {
                        lpUniBuffer = lpAnsiToUnicode(pBuffer, strlen(pBuffer) + 1);
                        hResult = CLSIDFromString(lpUniBuffer, &clsID);   // get CLSID of type lib
                        ORexxOleFree( lpUniBuffer );
                    }

                    ORexxOleFree(pszAnsiStr);
                }

                CoTaskMemFree(lpOleStrBuffer);
            }

            hResult = LoadRegTypeLib(clsID,sMajor,0,LOCALE_USER_DEFAULT,&pTypeLib);   // AddRef type lib pointer
            if (hResult == S_OK)
            {
                BSTR         bName, bDoc;
                ITypeInfo   *pTypeInfo2 = NULL;

                // create a Stem that will contain all info
                RexxStemObject RxStem = context->NewStem(NULL);

                hResult = pTypeLib->GetDocumentation(/*iTypeIndex*/-1,&bName,&bDoc,NULL,NULL);
                sprintf(pszInfoBuffer,"%S",bName);
                context->SetStemElement(RxStem, "!LIBNAME", context->NewStringFromAsciiz(pszInfoBuffer));
                sprintf(pszInfoBuffer,"%S",bDoc);
                context->SetStemElement(RxStem, "!LIBDOC", context->NewStringFromAsciiz(pszInfoBuffer));

                SysFreeString(bName);
                SysFreeString(bDoc);

                iTypeCount=pTypeLib->GetTypeInfoCount();

                for (iTypeIndex=0;iTypeIndex<iTypeCount;iTypeIndex++)
                {


                    hResult = pTypeLib->GetDocumentation(iTypeIndex,&bName,&bDoc,NULL,NULL);
                    sprintf(pszInfoBuffer,"%S",bName);
                    sprintf(pszInfoBuffer+1024,"!LIBNAME.%d",iTypeIndex);
                    context->SetStemElement(RxStem, pszInfoBuffer+1024, context->NewStringFromAsciiz(pszInfoBuffer));
                    sprintf(pszInfoBuffer,"%S",bDoc);
                    sprintf(pszInfoBuffer+1024,"!LIBDOC.%d",iTypeIndex);
                    context->SetStemElement(RxStem, pszInfoBuffer+1024, context->NewStringFromAsciiz(pszInfoBuffer));

                    SysFreeString(bName);
                    SysFreeString(bDoc);


                    hResult = pTypeLib->GetTypeInfo(iTypeIndex,&pTypeInfo2);   // AddRef type info pointer2
                    if (pTypeInfo2)
                    {
                        hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);           // AddRef type attr pointer
                        if (hResult == S_OK)
                        {
                            InsertTypeInfo(context, pTypeInfo2,pTypeAttr,RxStem,&iCount);
                            pTypeInfo2->ReleaseTypeAttr(pTypeAttr);                // Release type attr pointer
                        }
                    }
                    pTypeInfo2->Release();                                     // Release type info pointer2
                }

                context->SetStemElement(RxStem, "0", context->WholeNumberToObject(iCount));
                RxResult = RxStem;
            }
            if (pTypeLib)
            {
                pTypeLib->Release();                           // Release type lib pointer
            }
        }
    }

    return RxResult;
}

//******************************************************************************
// Method:  OLEObject_GetKnownEvents
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     returnObject (RexxObjectPtr) - A Rexx stem with the names of all known
//                                 events or .Nil.
//
//   Notes:
//     This method will retrieve the needed information from the object's
//     event list which was created in Init, if the user requested events and if
//     the OLE object supports connnection points.
//
//******************************************************************************
RexxMethod1(RexxObjectPtr,                // Return type
            OLEObject_GetKnownEvents,  // Object_method name
            OSELF, self)               // Pointer to self
{
    RexxObjectPtr   RxResult = context->Nil();
    INT             iCount = 0;
    INT             j;
    CHAR            pszInfoBuffer[2048];
    CHAR            pszSmall[128];
    OLEObjectEvent *pEventHandler = NULL;
    POLEFUNCINFO2   pEventList = NULL;
    unsigned short  wFlags = 0;

    if ( !fInitialized )
    {
        OLEInit();
    }

    if ( haveEventHandler(context) )
    {
        getEventHandlerPtr(context, &pEventHandler);
    }
    else
    {
        IConnectionPointContainer *pContainer = NULL;

        if ( maybeCreateEventHandler(context, &pEventHandler, &pContainer, self) )
        {
            /* This is not needed. */
            pContainer->Release();
        }
    }

    if ( pEventHandler != NULL )
    {
        pEventList = pEventHandler->getEventList();
        if ( pEventList )
        {
            // create a Stem that will contain all info
            RexxStemObject RxStem = context->NewStem(NULL);

            while ( pEventList )
            {
                iCount++;

                sprintf(pszSmall,"%d.!NAME",iCount);
                context->SetStemElement(RxStem, pszSmall, context->NewStringFromAsciiz(pEventList->pszFuncName));

                sprintf(pszSmall,"%d.!DOC",iCount);
                context->SetStemElement(RxStem, pszSmall, context->NewStringFromAsciiz(pEventList->pszDocString));

                sprintf(pszSmall,"%d.!PARAMS.0",iCount);
                context->SetStemElement(RxStem, pszSmall, context->WholeNumberToObject(pEventList->iParmCount));

                for ( j=0;j<pEventList->iParmCount;j++ )
                {
                    sprintf(pszSmall,"%d.!PARAMS.%d.!NAME",iCount,j+1);
                    context->SetStemElement(RxStem, pszSmall, context->NewStringFromAsciiz(pEventList->pszName[j]));

                    sprintf(pszSmall,"%d.!PARAMS.%d.!TYPE",iCount,j+1);
                    context->SetStemElement(RxStem, pszSmall, context->NewStringFromAsciiz(pszDbgVarType(pEventList->pOptVt[j])));

                    wFlags=pEventList->pusOptFlags[j];
                    pszInfoBuffer[0] = 0x00;
                    if ( wFlags || j >= pEventList->iParmCount - pEventList->iOptParms )
                    {
                        strcat(pszInfoBuffer,"[");
                        if ( wFlags & PARAMFLAG_FIN )
                            strcat(pszInfoBuffer,"in,");
                        if ( wFlags & PARAMFLAG_FOUT )
                            strcat(pszInfoBuffer,"out,");
                        if ( j >= pEventList->iParmCount - pEventList->iOptParms )
                            strcat(pszInfoBuffer,"opt,");
                    }
                    pszInfoBuffer[strlen(pszInfoBuffer)-1]=0x00; // remove last comma
                    strcat(pszInfoBuffer,"]");

                    sprintf(pszSmall,"%d.!PARAMS.%d.!FLAGS",iCount,j+1);
                    context->SetStemElement(RxStem, pszSmall, context->NewStringFromAsciiz(pszInfoBuffer));
                }

                pEventList = pEventList->pNext;
            }
            context->SetStemElement(RxStem, "0", context->WholeNumberToObject(iCount));
            RxResult = RxStem;
        }
    }
    return RxResult;
}


RexxMethod0(logical_t, OLEObject_isConnected)
{

    if ( connectedToEvents(context) )
    {
        return true;
    }
    return false;
}

RexxMethod0(logical_t, OLEObject_isConnectable)
{
    IDispatch *pDispatch = NULL;

    /* See if we already have an event handler object.  If we do then this
     * object is connectable for sure.
     */
    if ( haveEventHandler(context) )
    {
        return true;
    }

    /* Get the IDispatch pointer for this object and see if the COM object we
     *  are proxying for is a connectable object.
     */
    if ( ! getDispatchPtr(context, &pDispatch) )
    {
        return false;
    }

    if ( isConnectableObject(pDispatch) )
    {
        return true;
    }
    return false;
}

RexxMethod1(logical_t, OLEObject_connectEvents, OSELF, self)
{
    IConnectionPointContainer *pContainer = NULL;
    IDispatch                 *pDispatch = NULL;
    OLEObjectEvent            *pEventHandler = NULL;
    bool                       connected = false;

    if ( connectedToEvents(context) )
    {
        return true;
    }

    if ( haveEventHandler(context) )
    {
        getEventHandlerPtr(context, &pEventHandler);
        if ( ! getDispatchPtr(context, &pDispatch) )
        {
            return false;
        }
        getConnectionPointContainer(pDispatch, &pContainer);
    }
    else
    {
        maybeCreateEventHandler(context, &pEventHandler, &pContainer, self);
    }

    if ( pEventHandler != NULL && pContainer != NULL )
    {
        connected = connectEventHandler(context, pContainer, pEventHandler);
        pContainer->Release();
    }

    if ( connected )
    {
        return true;
    }
    return false;
}


RexxMethod0(logical_t, OLEObject_disconnectEvents)
{
    if ( connectedToEvents(context) )
    {
        disconnectEventHandler(context);
        return true;
    }
    return false;
}


RexxMethod0(logical_t, OLEObject_removeEventHandler)
{
    if ( haveEventHandler(context) )
    {
        releaseEventHandler(context);
        return true;
    }
    return false;
}


/**
 * If the COM object this object represents is a connectable object, then create
 * an OLEObjectEvent handler and save it in this object's instance variables.
 *
 * @param self        Pointer to this ooRexx object.
 * @param connect     Connect or not connect the handler after creation.
 * @param pDispatch   IDispatch pointer for this COM object.
 * @param pClsInfo    Possible cached class info for this COM object's CoClass.
 * @param pTypeInfo   ITypeInfo pointer for this COM object, must not be null.
 * @param pClsID      Possible CLSID for this COM object's CoClass.
 *
 * @return True if the event handler object is instantiated, otherwise false.
 */
bool addEventHandler(RexxMethodContext *context, RexxObjectPtr self, bool connect, IDispatch *pDispatch,
                     POLECLASSINFO pClsInfo, ITypeInfo *pTypeInfo, CLSID *pClsID)
{
    IConnectionPointContainer *pConnectionPointContainer = NULL;
    bool created = false;

    /* If there is a connection point container interface, then this object is a
     * connectable object, otherwise it is not connectable and we just skip it.
     */
    if ( getConnectionPointContainer(pDispatch, &pConnectionPointContainer) )
    {
        OLEObjectEvent   *pEventHandler = NULL;
        ITypeInfo        *pEventTypeInfo = NULL;

        getEventTypeInfo(pDispatch, pTypeInfo, pClsID, &pEventTypeInfo);

        if ( pEventTypeInfo != NULL )
        {
            created = createEventHandler(context, pEventTypeInfo, self, pClsInfo, &pEventHandler);
            if ( created && connect )
            {
                connectEventHandler(context, pConnectionPointContainer, pEventHandler);
            }

            pEventTypeInfo->Release();
        }
        pConnectionPointContainer->Release();
    }
    return created;
}

/**
 * Given the type information for an outgoing (event) interface, instantiate an
 * OLEObjectEvent object that will handle invocations on that interface.
 *
 * @param pEventTypeInfo   [in]  Type information for the event interface.
 * @param self             [in]  Pointer to this ooRexx object.
 * @param pClsInfo         [in]  Cached class information for a CoClass.
 * @param ppEventHandler   [out] Returned instantiated OLEObjectEvent.
 *
 * @return True if the OLEObject is instantiated, otherwise false.  Provided
 *         that pEventTypeInfo is correct, failure is very unlikely.
 */
bool createEventHandler(RexxMethodContext *context, ITypeInfo *pEventTypeInfo, RexxObjectPtr self,
                        POLECLASSINFO pClsInfo, OLEObjectEvent **ppEventHandler)
{
    POLEFUNCINFO2 pEventList = NULL;
    bool          success = false;
    IID           theIID;

    GUIDFromTypeInfo(pEventTypeInfo, &theIID);

    /* Get the names, dispids, and parameters of the event interface's
     * functions.
     */
    pEventList = GetEventInfo(pEventTypeInfo, self, pClsInfo);

    if ( pEventList )
    {
        *ppEventHandler = new OLEObjectEvent(pEventList, self, context->threadContext->instance, theIID);
        success = true;

        context->SetObjectVariable("!EVENTHANDLER", context->NewPointer(*ppEventHandler));
    }
    return success;
}

/**
 * Connect an OLEObjectEvent (the sink) to the connectable COM object.
 *
 * @param pContainer     [in]  The connection point container interface of the
 *                       connectable COM object.
 * @param pEventHandler  [in]  The OLEObjectEvent to connect.
 *
 * @return Return true if connected, otherwise false.
 *
 * Note:  The caller is responsible for releasing the IConnectionPointContainer
 * pointer.
 */
bool connectEventHandler(RexxMethodContext *context, IConnectionPointContainer *pContainer,
                         OLEObjectEvent *pEventHandler)
{
    IConnectionPoint *pConnectionPoint = NULL;
    HRESULT           hResult;
    DWORD             dwCookie = 0;

    hResult = pContainer->FindConnectionPoint(pEventHandler->getIntefaceID(), &pConnectionPoint);
    if ( hResult == S_OK )
    {
        hResult = pConnectionPoint->Advise((IUnknown*) pEventHandler, &dwCookie);
        if ( hResult == S_OK )
        {
            context->SetObjectVariable("!EVENTHANDLERCOOKIE", context->UnsignedInt32ToObject(dwCookie));
            context->SetObjectVariable("!CONNECTIONPOINT", context->NewPointer(pConnectionPoint));
        }
        else
        {
            pConnectionPoint->Release();
        }
    }
    return (hResult == S_OK);
}

/**
 * Get the IConnectionPointContainer interace for a connectable object, if the
 * object is connectable.
 *
 * @param pDispatch    [in]  The IDispatch interface of the object.
 * @param ppContainer  [out] The connection point container interface is
 *                     returned here.
 *
 * @return True on success, the interface was obtained, otherwise false.
 */
bool getConnectionPointContainer(IDispatch *pDispatch, IConnectionPointContainer **ppContainer)
{
    HRESULT hResult;

    hResult = pDispatch->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)ppContainer);
    if ( SUCCEEDED(hResult) )
    {
        return true;
    }
    return false;
}

/**
 * Return true if the object is a connectable object, i.e. if it has outgoing
 * interfaces.
 *
 * @param pDispatch  [in] The IDispatch interface of the object.
 *
 * @return True if the object is connectable, otherwise false.
 */
bool isConnectableObject(IDispatch *pDispatch)
{
    HRESULT hResult;
    IConnectionPointContainer *pContainer = NULL;

    hResult = pDispatch->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pContainer);
    if ( SUCCEEDED(hResult) && pContainer )
    {
        pContainer->Release();
        return true;
    }
    return false;
}

/**
 * Get the type information for an event interface, i.e. an outgoing or source
 * interface.
 *
 * @param pDispatch        [in]  Dispatch interface of an object.
 * @param pTypeInfo        [in]  Type information of an interface or CoClass.
 * @param pClsID           [in]  Possible CLSID of a CoClass.
 * @param ppEventTypeInfo  [out] On success the event type information is
 *                         returned here.
 *
 * @return True on success, otherwise false.  *ppEventTypeInfo is set to null on
 *         failure.
 *
 * Three methods are used to try and find the event type information.  The are
 * tried in order from easiest to hardest.
 *
 * 1.) A connectable object should implement one of the IProvideClassInfo
 * interfaces. The interface is queried for the CoClass that implements the
 * event interface.
 *
 * 2.) If the connectable object does not implement the IProvideClassInfo
 * interface, but the CLSID of the CoClass that implements the event interface
 * is known, the the CLSID can be used to get the CoClass from the containing
 * type library.
 *
 * 3.) Finally, if none of the above methods are successful, a brute force
 * search of the type libarary is used to locate the CoClass that implements the
 * event interface.  Once the CoClass is found, each of the interfaces it
 * implements is looked at to find the [default source] interface.  This should
 * always succeed, provided the input is valid.
 *
 * Note:  Some objects that implement IProvideClassInfo implement it
 * incorrectly, for example Microsoft Outlook.  Outlook returns the type
 * information of the dispatch interface rather than that of the CoClass.
 * Because of this, an extra step is taken to verify that the event type
 * information can actually be obtained when the object implements
 * IProvideClassInfo.
 */
bool getEventTypeInfo(IDispatch *pDispatch, ITypeInfo *pTypeInfo, CLSID *pClsID,
                      ITypeInfo **ppEventTypeInfo)
{
    ITypeInfo *pCoClassTypeInfo = NULL;
    ITypeInfo *pSourceTypeInfo = NULL;

    if ( getClassInfo(pDispatch, &pCoClassTypeInfo) )
    {
        eventTypeInfoFromCoClass(pCoClassTypeInfo, &pSourceTypeInfo);
    }

    if ( pSourceTypeInfo == NULL )
    {
        /* The easy way did not work.  Use the CLSID if we have it, otherwise
         * try searching the entire containing type library.
         */
        if ( ! IsEqualCLSID(*pClsID, CLSID_NULL) )
        {
            getClassInfoFromCLSID(pTypeInfo, pClsID, &pCoClassTypeInfo);
        }
        else
        {
            getClassInfoFromTypeInfo(pTypeInfo, &pCoClassTypeInfo);
        }

        if ( pCoClassTypeInfo != NULL )
        {
            eventTypeInfoFromCoClass(pCoClassTypeInfo, &pSourceTypeInfo);
        }
    }

    if ( pCoClassTypeInfo != NULL )
    {
        pCoClassTypeInfo->Release();
    }

    *ppEventTypeInfo = pSourceTypeInfo;
    return (pSourceTypeInfo != NULL);
}

/**
 * Retrieve the type information for the connectable CoClass from one of the
 * IProvideClassInfo interfaces if possible.
 *
 * @param pDispatch          [in]  Dispatch interface to query for the
 *                           IProvideClassInfo interface.
 * @param ppCoClassTypeInfo  [out] If the connectable object implements
 *                           IProvideClassInfo, the CoClass type iformation is
 *                           returned here.
 *
 * @return Returns true if the CoClass type information is obtained, otherwise
 *         false.  On failure *ppCoClassTypeInfo is set to null.
 */
bool getClassInfo(IDispatch *pDispatch, ITypeInfo **ppCoClassTypeInfo)
{
    HRESULT             hResult;
    IProvideClassInfo  *pProvideClassInfo  = NULL;
    IProvideClassInfo2 *pProvideClassInfo2 = NULL;
    ITypeInfo          *pTypeInfo = NULL;
    bool                success = false;

    hResult = pDispatch->QueryInterface(IID_IProvideClassInfo2, (LPVOID*)&pProvideClassInfo2);
    if ( SUCCEEDED(hResult) && pProvideClassInfo2 )
    {
        hResult = pProvideClassInfo2->GetClassInfo(&pTypeInfo);
        if ( SUCCEEDED(hResult) && pTypeInfo )
        {
            success = true;
            *ppCoClassTypeInfo = pTypeInfo;
        }
        pProvideClassInfo2->Release();
    }

    if ( ! success )
    {
        hResult = pDispatch->QueryInterface(IID_IProvideClassInfo, (LPVOID*)&pProvideClassInfo);
        if (hResult == S_OK && pProvideClassInfo )
        {
            hResult = pProvideClassInfo->GetClassInfo(&pTypeInfo);
            if ( SUCCEEDED(hResult) && pTypeInfo )
            {
                success = TRUE;
                *ppCoClassTypeInfo = pTypeInfo;
            }
            pProvideClassInfo->Release();
        }
    }

    if ( ! success )
    {
        *ppCoClassTypeInfo = NULL;
    }
    return success;
}

/**
 * Retrieve the type information for a CoClass using its CLSID.
 *
 * @param pTypeInfo           [in]  Type information of an interface or other
 *                            object in a type library.
 * @param pClsID              [in]  The CLSID of the CoClass
 * @param ppCoClassTypeInfo   [out] Returned CoClass type information.
 *
 * @return True on success, otherwise false.  On failure *ppCoClassTypeInfo is
 *         set to null.
 *
 * The specified type information (pTypeInfo) is used to obtain the containing
 * type library of the object.  The type library is then queried directly for
 * the type information of the object that matches the CLSID.
 *
 * Care should be used to ensure that pClsID points to a CLSID of a CoClass.
 * This function will return the type information for other objects in the type
 * library using the object's GUID. CLSIDs and GUIDs are interchangeable.  A
 * CLSID is the GUID of a CoClass.
 */
bool getClassInfoFromCLSID(ITypeInfo *pTypeInfo, CLSID *pClsID, ITypeInfo **ppCoClassTypeInfo)
{
    ITypeLib     *pTypeLib = NULL;
    ITypeInfo    *pCoClass = NULL;
    unsigned int  index;
    bool          found = false;
    HRESULT       hResult;

    hResult = pTypeInfo->GetContainingTypeLib(&pTypeLib, &index);
    if ( hResult == S_OK )
    {
        hResult = pTypeLib->GetTypeInfoOfGuid(*pClsID, &pCoClass);
        if ( hResult == S_OK )
        {
            found = true;
            *ppCoClassTypeInfo = pCoClass;
        }
        pTypeLib->Release();
    }

    if ( ! found )
    {
        *ppCoClassTypeInfo = NULL;
    }
    return found;
}


/**
 * Search for the CoClass that implements the interface with the specified type
 * info, and return that CoClasse's type info.
 *
 * @param pInterfaceTypeInfo [in]  Type info of an implemented interface.
 * @param ppCoClassTypeInfo  [out] Return the type info of the CoClass that
 *                           implements the input interface.
 *
 * @return True on success, otherwise false.  *pCoClassTypeInfo is set to null
 *         on failure.
 *
 * The algorithm works this way:  Find the containing type library of the
 * specified interface.  Look through the entire library, and, for each CoClass
 * in the library determine if the CoClass implements the specified interface.
 * If it does, return the type info of that CoClass.
 */
bool getClassInfoFromTypeInfo(ITypeInfo *pInterfaceTypeInfo, ITypeInfo **ppCoClassTypeInfo)
{
    ITypeLib   *pTypeLib = NULL;
    ITypeInfo  *pTypeInfo = NULL;
    TYPEKIND    kind;
    HRESULT     hResult;
    GUID        guid;
    UINT        count, i = 0;
    bool        found = false;

    /* Get the GUID for the interface we know. */
    GUIDFromTypeInfo(pInterfaceTypeInfo, &guid);

    hResult = pInterfaceTypeInfo->GetContainingTypeLib(&pTypeLib, &count);
    if ( hResult == S_OK )
    {
        count = pTypeLib->GetTypeInfoCount();
        while ( i++ < count && ! found )
        {
            hResult = pTypeLib->GetTypeInfoType(i, &kind);
            if ( hResult == S_OK && kind == TKIND_COCLASS )
            {
                /* This is a CoClass type info.  See if this class has
                 * implemented an interface whose GUID matches the one of the
                 * type info passed to us.
                 */
                hResult = pTypeLib->GetTypeInfo(i, &pTypeInfo);
                if ( hResult == S_OK )
                {
                    if ( isImplementedInterface(pTypeInfo, &guid) )
                    {
                        /* A match, this is the CoClass type info we want. */
                        found = true;
                        *ppCoClassTypeInfo = pTypeInfo;
                    }
                    else
                    {
                        pTypeInfo->Release();
                    }
                }
            }
        }
        pTypeLib->Release();
    }

    if ( ! found )
    {
        *ppCoClassTypeInfo = NULL;
    }
    return found;
}

/**
 * Determines if a CoClass has implemented an interface with the specified GUID.
 *
 * @param pCoClass  [in] The CoClass to search.
 * @param guid      [in] The GUID to search for.
 *
 * @return True if the CoClass has an implemented interface with the specified
 *         GUID, otherwise false.
 */
bool isImplementedInterface(ITypeInfo *pCoClass, GUID *guid)
{
    TYPEATTR  *pTypeAttr = NULL;
    ITypeInfo *pTypeInfo = NULL;
    HREFTYPE   hRefType = NULL;
    unsigned int i;
    bool match = false;
    HRESULT hResult;

    hResult = pCoClass->GetTypeAttr(&pTypeAttr);
    if ( hResult == S_OK )
    {
        for ( i = 0; i < pTypeAttr->cImplTypes && (! match); i++ )
        {
            pCoClass->GetRefTypeOfImplType(i, &hRefType);
            hResult = pCoClass->GetRefTypeInfo(hRefType, &pTypeInfo);
            if ( SUCCEEDED(hResult) )
            {
                TYPEATTR *pTempTypeAttr = NULL;
                hResult = pTypeInfo->GetTypeAttr(&pTempTypeAttr);
                if ( hResult == S_OK )
                {
                    if ( InlineIsEqualGUID(*guid, pTempTypeAttr->guid) )
                    {
                        match = true;
                    }
                    pTypeInfo->ReleaseTypeAttr(pTempTypeAttr);
                }
                pTypeInfo->Release();
            }
        }
        pCoClass->ReleaseTypeAttr(pTypeAttr);
    }
    return match;
}

/**
 * Search for and return the event type information from the implemented
 * interfaces of the specified CoClass.
 *
 * @param pCoClass          [in]  The CoClass to search.
 * @param ppTypeInfoEvents  [out] If found, the event type information is
 *                          returned here.
 *
 * @return True on success, otherwise false. *ppTypeInfoEvetns is set to null on
 *         failure.
 */
bool eventTypeInfoFromCoClass(ITypeInfo *pCoClass, ITypeInfo **ppTypeInfoEvents)
{
    TYPEATTR  *pTypeAttr = NULL;
    ITypeInfo *pTypeInfo = NULL;
    unsigned int i;
    int flags = 0;
    bool found = false;
    HRESULT hResult;

    hResult = pCoClass->GetTypeAttr(&pTypeAttr);

    if ( hResult == S_OK )
    {
        /* Look for the [default, source] entry */
        for ( i = 0; i < pTypeAttr->cImplTypes; i++ )
        {
            hResult = pCoClass->GetImplTypeFlags(i, &flags);
            if ( SUCCEEDED(hResult) && (flags & IMPLTYPEFLAG_FDEFAULT) && (flags & IMPLTYPEFLAG_FSOURCE) )
            {
                HREFTYPE hRefType = NULL;

                /* Found the default source (the outgoing) interface.  Get the
                 * type information.
                 */
                pCoClass->GetRefTypeOfImplType(i, &hRefType);
                hResult = pCoClass->GetRefTypeInfo(hRefType, &pTypeInfo);
                if ( SUCCEEDED(hResult) )
                {
                    found = true;
                    *ppTypeInfoEvents = pTypeInfo;
                }
                break;
            }
        }
        pCoClass->ReleaseTypeAttr(pTypeAttr);
    }

    if ( ! found )
    {
        *ppTypeInfoEvents = NULL;
    }
    return found;
}

/**
 * Converts a string representation of a CLSID into a CLSID.
 *
 * @param str    [in]  The string to convert.
 * @param clsID  [out] The CLSID is returned here.
 *
 * On error, clsID is set to the CLSID_NULL.
 */
void getClsIDFromString(const char * str, CLSID *clsID)
{
    LPOLESTR lpUniBuffer = NULL;

    lpUniBuffer = lpAnsiToUnicode(str, strlen(str) + 1);

    if (lpUniBuffer)
    {
        CLSIDFromString(lpUniBuffer, clsID);
        ORexxOleFree( lpUniBuffer );
    }
    else
    {
        memcpy((void*)clsID, (void*)&CLSID_NULL, sizeof(CLSID));
    }
}

/**
 * Gets the GUID of an object from its type information.
 *
 * @param pTypeInfo  [in]  Type information of the object.
 * @param guid       [out] The GUID is returned here.
 *
 * On error, guid is set to the GUID_NULL.
 */
void GUIDFromTypeInfo(ITypeInfo *pTypeInfo, GUID *guid)
{
    TYPEATTR     *pTypeAttr;
    HRESULT      hResult = E_UNEXPECTED;

    if ( pTypeInfo != NULL )
    {
        hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
    }

    if ( SUCCEEDED(hResult) )
    {
      memcpy((void*)guid, (void*)&(pTypeAttr->guid), sizeof(GUID));
      pTypeInfo->ReleaseTypeAttr(pTypeAttr);
    }
    else
    {
        memcpy((void*)guid, (void*)&GUID_NULL, sizeof(GUID));
    }
}

/**
 * Will try to instantiate an OLEObjectEvent handler object, if the COM object
 * this ooRexx object represents is a connectable object.
 *
 * @param ppHandler    [out] If instantiated, the OLEObjectEvent is returned
 *                     here.
 * @param ppContainer  [out] If the OLEObjectEvent is instantiated, the
 *                     IConnectionPointContainer for this COM object is returned
 *                     here.
 * @param self         [in]  Pointer to this ooRexx object.
 *
 * @return True if the OLEObjectEvent handler is instantiated, otherwise false.
 *         On failure, *ppHandler and *ppContainer are set to null.
 *
 * Note: It is possible for an event method to have the same name as an
 * IDispatch invocation method.  This gives rise to this scenario:  The user
 * adds an ooRexx event method to the OLEObject with that name. The user tries
 * to invoke the IDispatch method.  But, it will no longer get forwarded to the
 * unknown() method.  Instead the event method is invoked.  To prevent this,
 * when the event method names are discovered, they are compared against the
 * known IDispatch method names.  If there is a match, then the event method has
 * OLEEvent_ prepended to the event method name.
 *
 * If it weren't for this, we would not necessarily need to get the pTypeInfo
 * and pClsInfo pointers.  It is possible that this COM object implements the
 * IProvideClassInfo interface and a connection could be made without the type
 * info for the COM object.
 *
 */
bool maybeCreateEventHandler(RexxMethodContext * context, OLEObjectEvent **ppHandler,
                             IConnectionPointContainer **ppContainer, RexxObjectPtr self)
{
    RexxObjectPtr   value;
    IDispatch      *pDispatch = NULL;
    ITypeInfo      *pTypeInfo = NULL;
    ITypeInfo      *pEventTypeInfo = NULL;
    POLECLASSINFO   pClsInfo = NULL;
    CLSID           clsID = {0};
    bool            created = false;

    *ppContainer = NULL;  /* Insurance. */

    if ( ! getDispatchPtr(context, &pDispatch) )
    {
        return false;
    }

    if ( getConnectionPointContainer(pDispatch, ppContainer) )
    {
        getCachedClassInfo(context, &pClsInfo, &pTypeInfo);

        value = context->GetObjectVariable("!CLSID");
        if (value != NULLOBJECT)
        {
            getClsIDFromString(context->ObjectToStringValue(value), &clsID);
        }

        if ( pClsInfo != NULL )
        {
            getEventTypeInfo(pDispatch, pTypeInfo, &clsID, &pEventTypeInfo);

            if ( pEventTypeInfo != NULL )
            {
                createEventHandler(context, pEventTypeInfo, self, pClsInfo, ppHandler);
                pEventTypeInfo->Release();
                created = true;
            }
        }
    }

    if ( ! created )
    {
        if ( *ppContainer != NULL )
        {
            (*ppContainer)->Release();
            *ppContainer = NULL;
        }
        *ppHandler = NULL;
    }
    return created;
}

/**
 * Checks for an instantiated OLEObjectEvent that is attached to this instance
 * of an OLEObject.
 *
 * @return True if an OLEObjectEvent instance exists, otherwise false.
 */
inline bool haveEventHandler(RexxMethodContext *context)
{
    return (context->GetObjectVariable("!EVENTHANDLER") != NULLOBJECT);
}

/**
 * By definition, this object is connected to events if it has: the event
 * handler, the connection point, and the cookie.
 *
 * @return True if connected as an event sink to an event source, otherwise
 *         false.
 */
inline bool connectedToEvents(RexxMethodContext *context)
{
    return (haveEventHandler(context) &&
            (context->GetObjectVariable("!EVENTHANDLERCOOKIE") != NULLOBJECT) &&
            (context->GetObjectVariable("!CONNECTIONPOINT") != NULLOBJECT));
}

/**
 * If present, the event handler is released and removed from this object's
 * instance variables.  If the event handler is currently connected, the
 * connection is closed and the connection instance variables are also removed.
 */
void releaseEventHandler(RexxMethodContext *context)
{
    OLEObjectEvent *pEventHandler = NULL;

    if ( connectedToEvents(context) )
    {
        disconnectEventHandler(context);
    }

    getEventHandlerPtr(context, &pEventHandler);
    if (pEventHandler != NULL)
    {
        pEventHandler->Release();
        context->SetObjectVariable("!EVENTHANDLER", NULLOBJECT);
    }
}

/**
 * If the event handler is currently connected to the connectable COM object,
 * the connection is closed, the connection point in released, and the
 * connection variables are removed from this object's instance variables.
 */
void disconnectEventHandler(RexxMethodContext *context)
{
    IConnectionPoint *pConnectionPoint = NULL;
    DWORD             dwCookie = 0;

    RexxObjectPtr ptr = context->GetObjectVariable("!CONNECTIONPOINT");
    if ( ptr != NULLOBJECT )
    {
        pConnectionPoint = (IConnectionPoint *)context->PointerValue((RexxPointerObject)ptr);

        RexxObjectPtr cookie = context->GetObjectVariable("!EVENTHANDLERCOOKIE");
        if ( cookie != NULLOBJECT )
        {

            context->ObjectToUnsignedInt32(cookie, (uint32_t *)&dwCookie);
        }

        if (pConnectionPoint != NULL) {
          pConnectionPoint->Unadvise(dwCookie);   // remove connection
          pConnectionPoint->Release();            // free cp
        }

        context->DropObjectVariable("!CONNECTIONPOINT");
        context->DropObjectVariable("!EVENTHANDLERCOOKIE");
    }
}


REXX_METHOD_PROTOTYPE(OLEVariant_ParamFlagsEquals)
REXX_METHOD_PROTOTYPE(OLEVariant_VarValueEquals)
REXX_METHOD_PROTOTYPE(OLEVariant_VarTypeEquals)
REXX_METHOD_PROTOTYPE(OLEVariant_Init)


// now build the actual entry list
RexxMethodEntry oleobject_methods[] = {
    REXX_METHOD(OLEObject_Init,                  OLEObject_Init),
    REXX_METHOD(OLEObject_Uninit,                OLEObject_Uninit),
    REXX_METHOD(OLEObject_addRef_pvt,            OLEObject_addRef_pvt),
    REXX_METHOD(OLEObject_hasOLEMethod_pvt,      OLEObject_hasOLEMethod_pvt),
    REXX_METHOD(OLEObject_Unknown,               OLEObject_Unknown),
    REXX_METHOD(OLEObject_Request,               OLEObject_Request),
    REXX_METHOD(OLEObject_GetVar,                OLEObject_GetVar),
    REXX_METHOD(OLEObject_GetConst,              OLEObject_GetConst),
    REXX_METHOD(OLEObject_GetKnownMethods,       OLEObject_GetKnownMethods),
    REXX_METHOD(OLEObject_GetKnownMethods_Class, OLEObject_GetKnownMethods_Class),
    REXX_METHOD(OLEObject_GetKnownEvents,        OLEObject_GetKnownEvents),
    REXX_METHOD(OLEObject_GetObject_Class,       OLEObject_GetObject_Class),
    REXX_METHOD(OLEObject_isConnected,           OLEObject_isConnected),
    REXX_METHOD(OLEObject_isConnectable,         OLEObject_isConnectable),
    REXX_METHOD(OLEObject_connectEvents,         OLEObject_connectEvents),
    REXX_METHOD(OLEObject_disconnectEvents,      OLEObject_disconnectEvents),
    REXX_METHOD(OLEObject_removeEventHandler,    OLEObject_removeEventHandler),
    REXX_METHOD(OLEVariant_ParamFlagsEquals,     OLEVariant_ParamFlagsEquals),
    REXX_METHOD(OLEVariant_VarTypeEquals,        OLEVariant_VarTypeEquals),
    REXX_METHOD(OLEVariant_VarValueEquals,       OLEVariant_VarValueEquals),
    REXX_METHOD(OLEVariant_Init,                 OLEVariant_Init),
    REXX_LAST_METHOD()
};

RexxPackageEntry oleobject_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "OLEObject",                         // name of the package
    "1.4",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    NULL,                                // no functions in this package
    oleobject_methods                    // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(oleobject);

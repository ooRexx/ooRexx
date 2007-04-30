/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
#include "RexxCore.h"
#include "DebugOutput.h"
#include "events.h"

#define INCL_REXXSAA
#include "rexx.h"
#include "RexxNativeAPI.h"                        // REXX native interface
#include "ObjectClass.hpp"
#include "StringClass.hpp"


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

//******************************************************************************
// function prototypes for local functions
//******************************************************************************
PVOID ORexxOleAlloc(INT iSize);
PVOID ORexxOleReAlloc(PVOID ptr, INT iSize);
VOID ORexxOleFree(PVOID ptr);

LPOLESTR lpAnsiToUnicode(LPCSTR pszA, int length);
LPOLESTR lpAnsiToUnicodeLength(LPCSTR pszA, int length, int *outLength);
PSZ pszUnicodeToAnsi(LPOLESTR pszU);

PSZ pszStringDupe(PSZ pszOrig);

POLECLASSINFO psFindClassInfo(PSZ pszCLSId, ITypeInfo *pTypeInfo);
VOID ClearClassInfoBlock( POLECLASSINFO pClsInfo );
POLEFUNCINFO AddFuncInfoBlock( POLECLASSINFO pClsInfo, MEMBERID memId,
                               INVOKEKIND invKind, VARTYPE funcVT,
                               int iParmCount, int iOptParms, PSZ pszName );
POLECONSTINFO AddConstInfoBlock( POLECLASSINFO pClsInfo, MEMBERID memId,
                                 PSZ pszConstName, VARIANT *pValue);

BOOL fFindFunction(PSZ pszFunction, IDispatch *pDispatch, IDispatchEx *pDispatchEx,
                   ITypeInfo *pTypeInfo, POLECLASSINFO pClsInfo, unsigned short wFlags,
                   PPOLEFUNCINFO ppFuncInfo, MEMBERID *pMemId, int expectedArgCount );
BOOL fFindConstant(PSZ pszConstName, POLECLASSINFO pClsInfo, PPOLECONSTINFO ppConstInfo );

RexxObject *Variant2Rexx(VARIANT *pVariant);
VOID Rexx2Variant(RexxObject *RxObject, VARIANT *pVariant, VARTYPE DestVt, INT iArgPos);
BOOL fIsRexxArray(RexxObject *TestObject);
BOOL fIsOLEObject(RexxObject *TestObject);
BOOL fIsOleVariant(RexxObject *TestObject);
BOOL fRexxArray2SafeArray(RexxObject *RxArray, VARIANT FAR *VarArray, INT iArgPos);
BOOL fExploreTypeAttr( ITypeInfo *pTypeInfo, TYPEATTR *pTypeAttr, POLECLASSINFO pClsInfo );
VARTYPE getUserDefinedVT( ITypeInfo *pTypeInfo, HREFTYPE hrt );
BOOL fExploreTypeInfo( ITypeInfo *pTypeInfo, POLECLASSINFO pClsInfo );
BOOL checkForOverride( VARIANT *, RexxObject *, VARTYPE, RexxObject **, VARTYPE * );
BOOL isOutParam( RexxObject *, POLEFUNCINFO, INT );
VOID handleVariantClear( VARIANT *, RexxObject * );
__inline BOOL okayToClear( RexxObject * );

int (__stdcall *creationCallback)(CLSID, IUnknown*) = NULL;
void setCreationCallback(int (__stdcall *f)(CLSID, IUnknown*))
{
  creationCallback = f;
}

//******************************************************************************
// debugging functions implementation
//******************************************************************************

PSZ pszDbgInvkind(INVOKEKIND invkind)
{
  szDbg[0] = 0;

  if (invkind & INVOKE_FUNC)
    strcat(szDbg, "INVOKE_FUNC ");
  if (invkind & INVOKE_PROPERTYGET)
    strcat(szDbg, "INVOKE_PROPERTYGET ");
  if (invkind & INVOKE_PROPERTYPUT)
    strcat(szDbg, "INVOKE_PROPERTYPUT ");
  if (invkind & INVOKE_PROPERTYPUTREF)
    strcat(szDbg, "INVOKE_PROPERTYPUTREF");

  if (szDbg[strlen(szDbg)-1] == ' ')
    szDbg[strlen(szDbg)-1] = 0;
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
  } /* endswitch */

  return szDbg;
}


PSZ pszDbgParmFlags(unsigned short pf)
{
  szDbg[0] = 0;

  if (pf == PARAMFLAG_NONE)
    strcat(szDbg, "PARAMFLAG_NONE");

  if (pf & PARAMFLAG_FIN)
    strcat(szDbg, "PARAMFLAG_FIN ");
  if (pf & PARAMFLAG_FOUT)
    strcat(szDbg, "PARAMFLAG_FOUT ");
  if (pf & PARAMFLAG_FLCID)
    strcat(szDbg, "PARAMFLAG_FLCID ");
  if (pf & PARAMFLAG_FRETVAL)
    strcat(szDbg, "PARAMFLAG_FRETVAL ");
  if (pf & PARAMFLAG_FOPT)
    strcat(szDbg, "PARAMFLAG_FOPT ");
  if (pf & PARAMFLAG_FHASDEFAULT)
    strcat(szDbg, "PARAMFLAG_FHASDEFAULT ");

  if (szDbg[strlen(szDbg)-1] == ' ')
    szDbg[strlen(szDbg)-1] = 0;
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
      } /* endfor */
    } /* endif */
  }
  else
  {
    /* a single type variant */
    VariantInit(&sStrArg);
    if (! (V_VT(pVar) & VT_BYREF) ) {
      if (VariantChangeType(&sStrArg, pVar, 0, VT_BSTR) == S_OK)
        sprintf(szValue, "%.32S", V_BSTR(&sStrArg));  // cut off after 32 characters
      else
        strcpy(szValue, "<impossible conversion>");
    }
    VariantClear(&sStrArg);
  } /* endif */

  strcat(szDbg, szValue);
  return szDbg;
}

//******************************************************************************
// type library list
//******************************************************************************
void addTypeListElement(GUID *pGUID, POLECONSTINFO infoPtr)
{
  if (pTypeLibList) {
    PTYPELIBLIST newEntry = (PTYPELIBLIST) ORexxOleAlloc(sizeof(TYPELIBLIST));
    newEntry->next = pTypeLibList;
    newEntry->info = infoPtr;
    memcpy(&(newEntry->guid),pGUID,sizeof(GUID));
    pTypeLibList = newEntry;
  } else {
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

  while (!fFound && entry) {
    if (memcmp(pGUID,&(entry->guid),sizeof(GUID)) == 0) {
      fFound = entry->info;
      break;
    } else
      entry = entry->next;
  }

  return fFound;
}


void destroyTypeLibList()
{
  while (pTypeLibList) {
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
PVOID ORexxOleAlloc(INT iSize)
{
  PVOID  ptr = NULL;   // default value

  if (iSize) {         // only allocate when needed!
/*    ptr = malloc( iSize );
    if ( ptr ) {
      memset(ptr, 0, iSize);
    } */
    ptr = calloc(iSize,1);
//  printf("Allocation: %p of size %d\n", ptr, iSize);
  }
  return ptr;
}


PVOID ORexxOleReAlloc(PVOID ptr, INT iSize)
{
  PVOID rptr = realloc( ptr, iSize );
//printf("Reallocation: %p to %p to size %d\n", ptr, rptr, iSize);
  return rptr;
}


VOID ORexxOleFree(PVOID ptr)
{
  if ( ptr ) {
      free( ptr );
//    printf("Free: %p\n", ptr);
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

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{

  switch( ul_reason_for_call ) {
    case DLL_PROCESS_DETACH:
      if (iInstanceCount > 0) {
        destroyTypeLibList();
        OleUninitialize();
      }
      // free class info memory [not a real leak]
      // not really needed, since the process will go away
      // and release that memory, but it is cleaner this way
      if (ppClsInfo) {
        for (int q=0;q<iClsInfoUsed;q++)
          if (ppClsInfo[q])
            ORexxOleFree(ppClsInfo[q]);
        ORexxOleFree(ppClsInfo);
        ppClsInfo = NULL;
        iClsInfoSize = iClsInfoUsed = 0;
      }
      break;
  } /* endswitch */

  return TRUE;
}


void OLEInit()
{
  fInitialized = TRUE;
}


LPOLESTR lpAnsiToUnicode(LPCSTR pszA, int iInputLength)
{
  return lpAnsiToUnicodeLength(pszA, iInputLength, NULL);
}


LPOLESTR lpAnsiToUnicodeLength(LPCSTR pszA, int iInputLength, int *outLength)
{
  int           iNewLength;
  LPOLESTR      lpOleStr = NULL;

  /* Special case the empty string, MultiByteToWideChar does not handle */
  /* an input length of 0.  Create a true Unicode empty string. */
  if ( iInputLength == 0 && pszA && *pszA == 0 )
  {
    lpOleStr = (LPOLESTR) ORexxOleAlloc(2);
    if ( outLength )
      *outLength = 0;
  }
  else
  {
    iNewLength = MultiByteToWideChar( CP_ACP, 0, pszA, iInputLength, NULL, 0 );

    if ( iNewLength )
    {
      lpOleStr = (LPOLESTR) ORexxOleAlloc(iNewLength * 2 + 2);
      if (lpOleStr)
      {
        if ( MultiByteToWideChar( CP_ACP, 0, pszA, iInputLength, lpOleStr, iNewLength ) == 0)
        {
          /* conversion failed */
          ORexxOleFree(lpOleStr);
          lpOleStr = NULL;
        } /* endif */

        if ( outLength )
          *outLength = iNewLength;
      } /* endif */
    } /* endif */
  } /* endelse */

  return lpOleStr;
}


PSZ pszUnicodeToAnsi(LPOLESTR pszU)
{
  int   iNewLength;
  int   iInputLength;
  PSZ   pszAnsiStr = NULL;

  if (pszU == NULL) return NULL;

  iInputLength = wcslen(pszU) + 1;
  iNewLength = WideCharToMultiByte(CP_ACP, 0, pszU, iInputLength, NULL, 0, NULL, NULL);
  if ( iNewLength )
  {
    pszAnsiStr = (PSZ) ORexxOleAlloc(iNewLength + 1);
    if (pszAnsiStr)
    {
      if ( WideCharToMultiByte(CP_ACP, 0, pszU, iInputLength, pszAnsiStr, iNewLength, NULL, NULL) == 0)
      {
        /* conversion failed */
        ORexxOleFree(pszAnsiStr);
        pszAnsiStr = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  return pszAnsiStr;
}


PSZ pszStringDupe(PSZ pszOrig)
{
  PSZ   pszCopy = NULL;

  if ( pszOrig )
  {
    pszCopy = (PSZ) ORexxOleAlloc(strlen(pszOrig) + 1);
    if ( pszCopy )
    {
      strcpy(pszCopy, pszOrig);
    } /* endif */
  } /* endif */

  return pszCopy;
}


PSZ prxStringDupe(PSZ pszOrig, int length)
{
  PSZ   pszCopy = NULL;

  if ( pszOrig )
  {
    pszCopy = (PSZ) ORexxOleAlloc(length + 1);
    if ( pszCopy )
    {
      strcpy(pszCopy, pszOrig);
    } /* endif */
  } /* endif */

  return pszCopy;
}

POLECLASSINFO psFindClassInfo(PSZ pszCLSId, ITypeInfo *pTypeInfo)
{
  int iIdx;
  int iFound = -1;


  /* special case: no class info at all yet */
  if ( iClsInfoSize == 0 )
  {
    ppClsInfo = (PPOLECLASSINFO) ORexxOleAlloc(sizeof(POLECLASSINFO) * 10);
    if (ppClsInfo)
      iClsInfoSize = 10;
    else
    {
      send_exception(Error_System_resources);
    } /* endif */
  } /* endif */

  /* first try to locate the specified CLSID or pTypeInfo */
  for (iIdx = 0; iIdx < iClsInfoUsed; iIdx++)
  {
    if ( (pszCLSId && ppClsInfo[iIdx]->pszCLSId &&
         (strcmp(pszCLSId, ppClsInfo[iIdx]->pszCLSId) == 0)) ||
         (pTypeInfo && (pTypeInfo == ppClsInfo[iIdx]->pTypeInfo)) )
    {
      iFound = iIdx;
      break;
    } /* endif */
  } /* endfor */

  if (iFound == -1)
  {
    /* not found, see if there is an unused slot */
    for (iIdx = 0; iIdx < iClsInfoUsed; iIdx++)
    {
      if ( ppClsInfo[iIdx]->fUsed == FALSE )
      {
        iFound = iIdx;
        ppClsInfo[iFound]->fUsed = TRUE;
        break;
      } /* endif */
    } /* endfor */
  } /* endif */

  if (iFound == -1)
  {
    /* if still no slot found then allocate a new one */
    /* see if array needs to be enlarged */
    if (iClsInfoUsed >= iClsInfoSize)
    {
      PPOLECLASSINFO    ppNewBlock;
      ppNewBlock = (PPOLECLASSINFO) ORexxOleReAlloc(ppClsInfo,
                                    sizeof(POLECLASSINFO) * iClsInfoSize * 2);
      if (ppNewBlock)
      {
        ppClsInfo = ppNewBlock;
        iClsInfoSize *= 2;
      }
      else
      {
        send_exception(Error_System_resources);
      } /* endif */
    } /* endif */

    if (iClsInfoUsed < iClsInfoSize)
    {
      /* we have room in the array */
      iFound = iClsInfoUsed;
      ppClsInfo[iFound] = (POLECLASSINFO) ORexxOleAlloc(sizeof(OLECLASSINFO));
      if (ppClsInfo[iFound])
      {
        ppClsInfo[iFound]->fUsed = TRUE;
        iClsInfoUsed++;
      }
      else
      {
        iFound = -1;
      } /* endif */
    } /* endif */
  } /* endif */

  if (iFound == -1)
    return NULL;
  else
  {
    return ppClsInfo[iFound];
  } /* endif */
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
  } /* endwhile */
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
  } /* endwhile */
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
      } /* endif */

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
        } /* endif */
      } /* endwhile */

      /* add new block to end of list */
      pCurrBlock->pNext = pNewBlock;
    }
    else
    {
      /* list does not exist, this is the first element */
      pClsInfo->pFuncInfo = pNewBlock;
    } /* endif */
  } /* endif */

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
      } /* endif */

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
        } /* endif */
      } /* endwhile */

      /* add new block to end of list */
      pCurrBlock->pNext = pNewBlock;
    }
    else
    {
      /* list does not exist, this is the first element */
      pClsInfo->pConstInfo = pNewBlock;
    } /* endif */
  } /* endif */

  return pNewBlock;
}


BOOL fFindFunction(PSZ pszFunction, IDispatch *pDispatch, IDispatchEx *pDispatchEx,
                   ITypeInfo *pTypeInfo, POLECLASSINFO pClsInfo, unsigned short wFlags,
                   PPOLEFUNCINFO ppFuncInfo, MEMBERID *pMemId, int expectedArgCount )
{
  HRESULT         hResult;
  BOOL            fFound = FALSE;
  POLEFUNCINFO    pFuncInfo = NULL;
  POLEFUNCINFO    pFuncCache = NULL;

  /* initialize to not found entry */
  if (ppFuncInfo)
    *ppFuncInfo = NULL;

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

          if (pFuncInfo->iParmCount + pFuncInfo->iOptParms >= expectedArgCount)
          {
            fFound = TRUE;
            *pMemId = pFuncInfo->memId;
            if (ppFuncInfo)
              *ppFuncInfo = pFuncInfo;
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

          if (pFuncInfo->iParmCount + pFuncInfo->iOptParms >= expectedArgCount)
          {
            fFound = TRUE;
            *pMemId = pFuncInfo->memId;
            if (ppFuncInfo)
              *ppFuncInfo = pFuncInfo;
            break; /* found correct function */
          }
        }

        // arg count doesn't match. cache "a" fitting method
        // in case we don't find anything better
        pFuncCache = pFuncInfo;
      } /* endif */

      /* not found so far, go to next element in list */
      pFuncInfo = pFuncInfo->pNext;
    } /* endwhile */
  } /* endif */

  if (fFound == FALSE) {
    if (pFuncCache) {
      fFound = TRUE;
      *pMemId = pFuncCache->memId;
      // found "something" with that name, but the
      // function description is not reliable (fFindFunction
      // will be called again with *any* number of arguments -
      // this way we can turn on the DISPATCH_METHOD bit for
      // invocation)
    } else {
      /* try to find function description with GetIDsOfNames */
      LPOLESTR   lpUniBuffer;
      hResult = E_FAIL;

      lpUniBuffer = lpAnsiToUnicode(pszFunction, strlen(pszFunction) + 1);
      if (lpUniBuffer) {
        if (pDispatchEx) {
          //                 IDispatchEx::GetDispID expects a "real" BSTR
          //                 not the one MS documented as a simple OLECHAR* !
          int len = wcslen(lpUniBuffer)+1;
          LPOLESTR realBSTR = (LPOLESTR) ORexxOleAlloc(4+2*len);
          memcpy(realBSTR+2,lpUniBuffer,2*len);
          *((int*) realBSTR) = len-1;
//          hResult = pDispatchEx->GetDispID(lpUniBuffer, fdexNameCaseInsensitive, pMemId);
          hResult = pDispatchEx->GetDispID(realBSTR+2, fdexNameCaseInsensitive, pMemId);
          ORexxOleFree(realBSTR);
        }
        if (FAILED(hResult)) // If IDispatchEx call fails, try pDispatch
//        else
          if (pDispatch)
          {
            hResult = pDispatch->GetIDsOfNames(IID_NULL, &lpUniBuffer, 1,
                                               LOCALE_USER_DEFAULT, pMemId);
          }  /* endif */

        if (hResult == S_OK)
          fFound = TRUE;

        ORexxOleFree(lpUniBuffer);
      } /* endif */
    } /* endif */
  } /* endif */

  return fFound;
}


BOOL fFindConstant(PSZ pszConstName, POLECLASSINFO pClsInfo, PPOLECONSTINFO ppConstInfo )
{
  BOOL            fFound = FALSE;
  POLECONSTINFO   pConstInfo = NULL;

  /* initialize to not found entry */
  if (ppConstInfo)
    *ppConstInfo = NULL;


  /* find function in class info structure */
  pConstInfo = pClsInfo->pConstInfo;
  while ( pConstInfo )
  {
    if (stricmp(pConstInfo->pszConstName, pszConstName) == 0)
    {
      fFound = TRUE;
      if (ppConstInfo)
        *ppConstInfo = pConstInfo;
      break; /* found correct constant */
    } /* endif */

    /* not found so far, go to next element in list */
    pConstInfo = pConstInfo->pNext;
  } /* endwhile */

#if defined(REXX_DEBUG)
  if (fFound)
    DbgPrintf("Located const info ptr=%p, memid=%8.8X, sValue=%s\n",
              pConstInfo, pConstInfo->memId,
              pszDbgVariant(&(pConstInfo->sValue)));
#endif

  return fFound;
}


RexxObject *SafeArray2RexxArray(VARIANT *pVariant)
{
  SAFEARRAY  *pSafeArray;
  VARTYPE     EmbeddedVT;
  VARIANT     sVariant;
  LONG        lDimensions;
  LONG        lIdx;
  LONG        lDIdx;               // dimension index
  PVOID       pTarget;
  RexxObject *RxItem;
  RexxObject *ResultObj = RexxNil;
  RexxObject *ArrayObjectClass = NULL;
  RexxObject *argArray = NULL;     // argument array for "new" of multidimensional array
  HRESULT     hResult;
  char        szBuffer1[32];
  char        szBuffer2[32];
  PLONG       lpIndices;
  PLONG       lpLowBound;
  PLONG       lpUpperBound;
  LONG        lNumOfElements;
  BOOL        fCarryBit;
  INT         i;


  pSafeArray = V_ARRAY(pVariant);
  EmbeddedVT = V_VT(pVariant) & VT_TYPEMASK;
  lDimensions = SafeArrayGetDim(pSafeArray);

  /* alloc an array of lDimensions LONGs to hold the indices */
  lpIndices=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);
  /* alloc an array of lDimensions LONGs to hold the upper bound */
  lpUpperBound=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);
  /* alloc an array of lDimensions LONGs to hold the indices */
  lpLowBound=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);

  /* build argument array for construction of multidimensional array */
  argArray=RexxArray(lDimensions);

  lNumOfElements=1;  // total number of elements
  for (lDIdx=1;lDIdx<=lDimensions;lDIdx++) {
    hResult = SafeArrayGetLBound(pSafeArray, lDIdx, lpLowBound+lDIdx-1);
    hResult = SafeArrayGetUBound(pSafeArray, lDIdx, lpUpperBound+lDIdx-1);
    sprintf(szBuffer1,"%d",lpUpperBound[lDIdx-1]-lpLowBound[lDIdx-1]+1);
    sprintf(szBuffer2,"%d",lDIdx);
    lNumOfElements*=(lpUpperBound[lDIdx-1]-lpLowBound[lDIdx-1]+1);
    // put number of elements for this dimension into argument array
    RexxSend2(argArray,"PUT",RexxString(szBuffer1),RexxString(szBuffer2));
    // initial value of indices vector = [LowBound[1],...,LowBound[n]]
    lpIndices[lDIdx-1]=lpLowBound[lDIdx-1];
  }

  ArrayObjectClass = RexxSend0(RexxEnvironment,"ARRAY");
  // create an array with lDimensions dimensions
  ResultObj=RexxSend(ArrayObjectClass,"NEW",argArray);

  /* process all elements */
  for (lIdx=0;lIdx<lNumOfElements;lIdx++) {
    /* now build message array to put element at its place in rexx array */
    /* the indices for each dimension get place at 2,...,n+1, because    */
    /* 1 is reserved for the object itself (see PUT of array method)     */
    argArray=RexxArray(lDimensions+1);
    for (i=0;i<lDimensions;i++) {
      sprintf(szBuffer1,"%d",1-lpLowBound[i]+lpIndices[i]);  // rexx array always start at one
      array_put(argArray, RexxString(szBuffer1), i+2);       // index of array index (starts at 2, see above)
    }

    /* get the element at current indices, transform it into a rexx object and */
    /* set it into the rexx array                                              */

    VariantInit(&sVariant);
    V_VT(&sVariant) = EmbeddedVT;

    // if (EmbeddedVT == VT_VARIANT || EmbeddedVT == VT_BSTR)  // treat VT_BSTR like VT_VARIANT
    if (EmbeddedVT == VT_VARIANT)
      pTarget = (PVOID) &sVariant;
    else
      pTarget = (PVOID) &(V_NONE(&sVariant));

    hResult = SafeArrayGetElement(pSafeArray, lpIndices, pTarget);
    if (hResult == S_OK)
    {
      /* create a new REXX object from the result */
      RxItem = Variant2Rexx(&sVariant);
    }
    else
    {
    } /* endif */

    VariantClear(&sVariant);

    array_put(argArray,RxItem, 1);         // put object at index 1, indices follow thereafter
    RexxSend(ResultObj,"PUT", argArray);   // put object into rexx array

    /* increment indices vector (to access safearray elements) */
    fCarryBit=TRUE;
    i=0;
    while (fCarryBit && i<lDimensions) {
      if (lpIndices[i] == lpUpperBound[i])
        lpIndices[i] = lpLowBound[i];
      else {
        lpIndices[i]++;
        fCarryBit=FALSE;
      }
      i++;
    } /* end while */
  } /* end for */

  ORexxOleFree(lpLowBound);
  ORexxOleFree(lpUpperBound);
  ORexxOleFree(lpIndices);

  return ResultObj;
}

RexxObject *Variant2Rexx(VARIANT *pVariant)
{
  RexxObject  *ResultObj = NULL; /* use string as result */
  VARIANT      sTempVariant;
  CHAR         szBuffer[2048];
  IUnknown    *pUnknown;
  RexxObject  *OLEObjectClass = NULL;
  HRESULT      hResult;
  BOOL         fByRef = FALSE;
  VARIANT     *pVariant2 = NULL;
  IDispatch   *pOleObject = NULL;

  if ( V_VT(pVariant) & VT_BYREF ) fByRef = TRUE;

  if (V_VT(pVariant) & VT_ARRAY)
  {
    ResultObj = SafeArray2RexxArray(pVariant);
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
        if (V_VT(pVariant) == (VT_VARIANT|VT_BYREF)) {
          pVariant = (VARIANT*) pVariant->pvarVal;
          fByRef = FALSE;
          if (V_VT(pVariant) & VT_BYREF) pVariant = pVariant->pvarVal;
        }

        if (fByRef) {
          VARIANT temp;
          VariantInit(&temp);
          // take care of the indirection
          VariantCopyInd(&temp,pVariant);
          hResult = VariantChangeType( &sTempVariant, &temp, 0, VT_BSTR);
          VariantClear(&temp);
        } else
          hResult = VariantChangeType( &sTempVariant, pVariant, 0, VT_BSTR);
        if (hResult == S_OK)
        {
          /* convert BSTR to an ANSI string */
          PSZ pszAnsiStr;

          pszAnsiStr = pszUnicodeToAnsi(V_BSTR(&sTempVariant));
          if (pszAnsiStr)
          {
            // if this was a float/double, ignore(!) the users LOCALE settings
            if (V_VT(pVariant) == VT_R8 || V_VT(pVariant) == VT_R4) {
              char  pBuffer[4];
              GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,pBuffer,4);
              for (int j=0;j<(int) strlen(pszAnsiStr);j++)
                if (pszAnsiStr[j] == pBuffer[0]) pszAnsiStr[j]='.';
            }

            ResultObj = RexxString(pszAnsiStr);
            ORexxOleFree(pszAnsiStr);
          }
        }
        else
        {
          sprintf(szBuffer, "%s", pszDbgVarType(V_VT(pVariant)));
          send_exception1(Error_Variant2Rexx,RexxArray1(RexxString(szBuffer)));
        } /* endif */
        // if (fByRef) V_VT(pVariant)^=VT_BYREF; // VariantChangeType does not like VT_BYREF
        VariantClear(&sTempVariant);
        break;

      case VT_BOOL:
        /* some special handling for VT_BOOL */
        if (V_BOOL(pVariant) == 0)
          ResultObj = RexxFalse;
        else
          ResultObj = RexxTrue;
        break;

      case VT_UNKNOWN:
        /* try to get a dispatch pointer and then create an OLE Object */
        /* DFX TODO: IUnknown can also be passed by reference.  This should be
         * handled in the same manner as VT_DISPATCH.
         */
        pUnknown = V_UNKNOWN(pVariant);
        if (pUnknown)
        {
          IDispatch  *pDispatch;

          hResult = pUnknown->QueryInterface(IID_IDispatch, (LPVOID *)&pDispatch);
          if ((hResult == S_OK) && pDispatch)
          {
            sprintf(szBuffer, "IDISPATCH=%p", pDispatch);
            OLEObjectClass = RexxSend0(RexxEnvironment, "OLEOBJECT");
            ResultObj = RexxSend1(OLEObjectClass, "NEW", RexxString(szBuffer));
            pDispatch->Release();
          } /* endif */
        } /* endif */

        if (ResultObj == NULL)
        {
          ResultObj = RexxNil;
        } /* endif */
        break;

      case VT_DISPATCH:
        /* create a new OLE object with this dispatch pointer */
        if (fByRef) pOleObject = *V_DISPATCHREF(pVariant);
        else pOleObject = V_DISPATCH(pVariant);
        if (pOleObject)
        {
          sprintf(szBuffer, "IDISPATCH=%p", pOleObject);
          OLEObjectClass = RexxSend0(RexxEnvironment, "OLEOBJECT");
          ResultObj = RexxSend1(OLEObjectClass, "NEW", RexxString(szBuffer));
        }
        else
        {
          ResultObj = RexxNil;
        } /* endif */
        break;

      case VT_EMPTY:
      case VT_NULL:
      case VT_VOID:
        ResultObj = RexxNil;
        break;

      case VT_ERROR:
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
          sprintf(szBuffer, "%s", pszDbgVarType(V_VT(pVariant)));
          send_exception1(Error_Variant2Rexx,RexxArray1(RexxString(szBuffer)));
        break;
    } /* end switch */
  } /* endif */

  if (ResultObj == NULL)
    ResultObj = (RexxObject*) RexxString(szBuffer);

  return ResultObj;
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
VOID Rexx2Variant(RexxObject *_RxObject, VARIANT *pVariant, VARTYPE _DestVt, INT iArgPos)
{
  BOOL         fDone = FALSE;
  BOOL         fByRef = FALSE;
  PSZ          pszRxString;
  RexxString  *RxString;
  VARIANT      sVariant;
  HRESULT      hResult;
  RexxObject  *RxObject;
  VARTYPE      DestVt;

  if ( checkForOverride(pVariant, _RxObject, _DestVt, &RxObject, &DestVt) )
    return;

  if (DestVt & VT_BYREF) {
    DestVt ^= VT_BYREF;
    fByRef = TRUE;
  }

  /* arguments are filled in from the end of the array */
  VariantInit(pVariant);

  if ((RxObject == NULL) || (RxObject == RexxNil))
  {
    /* omitted argument */
    V_VT(pVariant) = VT_ERROR;
    V_ERROR(pVariant) = DISP_E_PARAMNOTFOUND;
  }
  else
  {
    /* is this an OLEObject providing an !IDISPATCH property? */
    if (fIsOLEObject(RxObject))
    {
      RxString = (RexxString *)RexxSend1(RxObject, "!GETVAR", RexxString("!IDISPATCH"));
      pszRxString = string_data(RxString);
      if (*pszRxString != '!')
      {
        if (fByRef) {
          if (sscanf(pszRxString, "%p", &(V_DISPATCHREF(pVariant))) == 1)
          {
            V_VT(pVariant) = VT_DISPATCH|VT_BYREF;
            V_DISPATCH(pVariant)->AddRef();
            fDone = TRUE;
          } /* endif */
        } else {
          if (sscanf(pszRxString, "%p", &(V_DISPATCH(pVariant))) == 1)
          {
            V_VT(pVariant) = VT_DISPATCH;
            V_DISPATCH(pVariant)->AddRef();
            fDone = TRUE;
          } /* endif */
        } /* end if */
      } /* endif */
    } /* endif */

    if (!fDone)
    {
      /* or maybe this is an array? */
      if (fIsRexxArray(RxObject))
      {
        fDone = fRexxArray2SafeArray(RxObject, pVariant, iArgPos); // byRefCheck!!!!
      } /* endif */
    } /* endif */

    /* if no target type is specified try original REXX types */
    if ((DestVt == VT_EMPTY) || (DestVt == VT_PTR) || (DestVt == VT_VARIANT))
    {
      if (!fDone)
      {
        if (RxObject == RexxFalse || RxObject == RexxTrue) {
          if (fByRef) {
            V_VT(pVariant) = VT_BOOL|VT_BYREF;
            *V_BOOLREF(pVariant) = (RxObject==RexxTrue) ? VARIANT_TRUE : VARIANT_FALSE;
          } else {
            V_VT(pVariant) = VT_BOOL;
            V_BOOL(pVariant) = (RxObject==RexxTrue) ? VARIANT_TRUE : VARIANT_FALSE;
          }
          fDone = TRUE;
        }
        else if (REXX_ISINTEGER(RxObject))
        {
          if (fByRef) {
            V_VT(pVariant) = VT_I4|VT_BYREF;
            *V_I4REF(pVariant) = REXX_INTEGER(RxObject);
          } else {
            V_VT(pVariant) = VT_I4;
            V_I4(pVariant) = REXX_INTEGER(RxObject);
          }
          fDone = TRUE;
        } /* endif */
      } /* endif */

      if (!fDone)
      {
        if (REXX_ISDOUBLE(RxObject))
        {
          if (fByRef) {
            V_VT(pVariant) = VT_R8|VT_BYREF;
            *V_R8REF(pVariant) = REXX_DOUBLE(RxObject);
          } else {
            V_VT(pVariant) = VT_R8;
            V_R8(pVariant) = REXX_DOUBLE(RxObject);
          }
          fDone = TRUE;
        } /* endif */
      } /* endif */
    } /* endif */

    /* handle VT_BOOL request */
    if (!fDone) {
      if ( DestVt == VT_BOOL ) {
        VARIANT_BOOL targetValue;

        if (RxObject == RexxTrue)
          targetValue = VARIANT_TRUE;
        else  if (RxObject == RexxFalse)
          targetValue = VARIANT_FALSE;
        else {
          LPOLESTR  lpUniBuffer = NULL;
          int       d=0;

          RxString = (RexxString *) RexxSend0(RxObject, "STRING");
          sscanf(string_data(RxString),"%d",&d);
          if (d == 0)
            targetValue = VARIANT_FALSE;
          else
            targetValue = VARIANT_TRUE;
        }

        if (fByRef) {
          V_VT(pVariant) = VT_BOOL|VT_BYREF;
          *V_BOOLREF(pVariant) = targetValue;
        } else {
          V_VT(pVariant) = VT_BOOL;
          V_BOOL(pVariant) = targetValue;
        }
        fDone = TRUE;
      }
    }

    if (!fDone)
    {
      LPOLESTR  lpUniBuffer = NULL;

      RxString = (RexxString *) RexxSend0(RxObject, "STRING");
      if (RxString == OREF_NULL) {
        send_exception1(Error_Rexx2Variant,RexxArray1(RexxString("given object")));
      }

      int uniBufferLength;

      // if the target is float or double, consider LOCALE information to force rexxlike representation
      if (DestVt == VT_R8 || DestVt == VT_R4) {
        // warning, this assumes non-unicode(!) and only single character symbols
        char  pBuffer[4];
        char *pNew;

        GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,pBuffer,4);
        pNew = prxStringDupe(string_data(RxString), string_length(RxString));
        // replace the "." decimal seperator with the one the user is using
        for (int j=0;j<(int) strlen(pNew);j++)
          if (pNew[j] == '.') pNew[j]=pBuffer[0];
        lpUniBuffer = lpAnsiToUnicodeLength(pNew, string_length(RxString), &uniBufferLength);
        ORexxOleFree(pNew);
      } else
        lpUniBuffer = lpAnsiToUnicodeLength(string_data(RxString), string_length(RxString), &uniBufferLength);

      if (lpUniBuffer)
      {
        if (DestVt != VT_EMPTY)
        {
          VariantInit(&sVariant);
          V_VT(&sVariant) = VT_BSTR;
          V_BSTR(&sVariant) = SysAllocStringLen(lpUniBuffer, uniBufferLength);
          if (VariantChangeType(pVariant, &sVariant, 0, DestVt) == S_OK)
          {
          }
          else
          {
            /* could not convert, give it the string value */
            hResult = VariantCopy(pVariant, &sVariant);

          } /* endif */

          /* clear temporary string */
          VariantClear(&sVariant);
        }
        else
        {
          V_VT(pVariant) = VT_BSTR;
          V_BSTR(pVariant) = SysAllocStringLen(lpUniBuffer, uniBufferLength);
        } /* endif */

        ORexxOleFree( lpUniBuffer);
      }
      else
      {
        V_VT(pVariant) = VT_ERROR;
        V_ERROR(pVariant) = DISP_E_PARAMNOTFOUND;
        send_exception1(Error_Rexx2Variant,RexxArray1((RexxString *) RexxSend0(RxObject, "STRING")));
      } /* endif */
    } /* endif */
  } /* endif */
} /* endif */


BOOL fIsRexxArray(RexxObject *TestObject)
{
  if ( TestObject &&
       (RexxSend1(TestObject, "HASMETHOD", RexxString("DIMENSION")) == RexxTrue) &&
       (RexxSend1(TestObject, "HASMETHOD", RexxString("HASINDEX")) == RexxTrue))
    return TRUE;
  else
    return FALSE;
}


BOOL fIsOLEObject(RexxObject *TestObject)
{
  if (TestObject &&
      RexxSend1(TestObject, "HASMETHOD", RexxString("!OLEOBJECT")) == RexxTrue)
    return TRUE;
  else
    return FALSE;
}

BOOL fIsOleVariant(RexxObject *TestObject)
{
  if ( TestObject &&
       RexxSend1(TestObject, "HASMETHOD", RexxString("!OLEVARIANT_")) == RexxTrue)
    return TRUE;
  else
    return FALSE;
}

BOOL fRexxArray2SafeArray(RexxObject *RxArray, VARIANT FAR *VarArray, INT iArgPos)
{
  BOOL            fDone = FALSE;
  LONG            lDimensions;
  PLONG           lpIndices;              // vector of indices
  LONG            lSize = 1;              // number of elements that need to be considered
  LONG            lCount;
  LONG            i, j;                   // counter variables
  RexxObject     *RexxStr;
  RexxObject     *RexxItem;
  RexxObject     *argArray = NULL;        // argument array for access to multidimensional array
  PSZ             pString;
  char            szBuffer[32];
  SAFEARRAY      *pSafeArray;             // the safearray
  SAFEARRAYBOUND *pArrayBounds;           // bounds for each dimension
  VARIANT         sVariant;
  HRESULT         hResult;
  BOOL            fCarryBit;

  RexxStr=RexxSend0(RxArray,"DIMENSION");
  pString=string_data((RexxString*) RexxSend0(RexxStr,"STRING"));

  // if dimension cannot be read => error!
  if (sscanf(pString,"%ld",&lDimensions) != 1)
  {
    send_exception(Error_Interpretation_initialization);
  }

  /* alloc an array of lDimensions LONGs to hold the indices */
  lpIndices=(PLONG) ORexxOleAlloc(sizeof(LONG)*lDimensions);
  /* alloc an array of SAFEARRAYBOUNDs */
  pArrayBounds=(SAFEARRAYBOUND*) ORexxOleAlloc(sizeof(SAFEARRAYBOUND)*lDimensions);

  /* get necessary information on array and set indices vector to initial state */
  for (i=0;i<lDimensions;i++) {
    sprintf(szBuffer,"%d",i+1);
    // get approx. number of elements in this dimension
    RexxStr=RexxSend1(RxArray,"DIMENSION",RexxString(szBuffer));
    pString=string_data((RexxString*) RexxSend0(RexxStr,"STRING"));
    sscanf(pString,"%ld",&lCount);
    // calculate the number of overall elements
    lSize*=lCount;
    /* initialize the SAFEARRAYBOUNDs */
    pArrayBounds[i].cElements=lCount;        // size of dimension
    pArrayBounds[i].lLbound=0;               // lower bound
    // set indices vector to initial state
    lpIndices[i]=0;
  }

  /* create the SafeArray */
  pSafeArray = SafeArrayCreate(VT_VARIANT,(UINT) lDimensions, pArrayBounds);

  if (pSafeArray) {
    V_VT(VarArray) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(VarArray) = pSafeArray;

    /* get each element and transform it into a VARIANT */
    for (i=0;i<lSize;i++) {
      argArray=RexxArray(lDimensions);
      for (j=0;j<lDimensions;j++) {
        sprintf(szBuffer,"%d",lpIndices[j]+1);          // rexx indices start at 1!
        array_put(argArray,RexxString(szBuffer),j+1);   // put j-th index in msg array
      }
      /* get item from RexxArray */
      RexxItem=RexxSend(RxArray,"AT",argArray);

      /* convert it into a VARIANT */
      VariantInit(&sVariant);

      if (RexxItem == RexxNil)                          // special handling of .nil (avoid VT_ERROR)
        V_VT(&sVariant)=VT_EMPTY;
      else
        Rexx2Variant(RexxItem,&sVariant,VT_EMPTY,0);

      /* set into the SafeArray */
      hResult = SafeArrayPutElement(pSafeArray, lpIndices, &sVariant);
      if (FAILED(hResult)) {
        // safearrayputelement failed - action required?
      }
      /* clear the local copy */
      VariantClear(&sVariant);

      /* increment indices vector */
      fCarryBit=TRUE;
      j=0;
      while (fCarryBit && j<lDimensions) {
        if (lpIndices[j] == (long) pArrayBounds[j].cElements - 1)
          lpIndices[j] = 0;
        else {
          lpIndices[j]++;
          fCarryBit=FALSE;
        }
        j++;
      } /* end while */
    } /* end for */
    fDone = TRUE;
  } /* end if */

  ORexxOleFree(pArrayBounds);
  ORexxOleFree(lpIndices);

  return fDone;
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
  // cleanup only to avoid compiler warning OLECHAR         OleBuffer[100];
  LPOLESTR        lpOleStrBuffer = NULL; // = OleBuffer;
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
        } /* endif */
      }
      else
      {
        // memory exception or duplicate entry!
        ORexxOleFree( pszName );
      } /* endif */

      pTypeInfo->ReleaseFuncDesc(pFuncDesc);
    } /* endif */
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
      } /* endif */
    }
    else
      pszName = NULL;

    pTypeInfo->ReleaseVarDesc(pVarDesc);
  } /* endfor */

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
  } /* endif */

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
      } /* endif */

      pTypeInfo2->Release();
    } /* endif */

    iTypeInfoCount = (INT) pTypeLib->GetTypeInfoCount();
    // find the GUID for the type library
    {
      TLIBATTR *pTLibAttr = NULL;
      hResult = pTypeLib->GetLibAttr(&pTLibAttr);
      if (hResult == S_OK) {
        memcpy(&typeGUID,&(pTLibAttr->guid),sizeof(GUID));
        pTypeLib->ReleaseTLibAttr(pTLibAttr);
      } else // this is illegal, what do we do about it? [shouldn't happen]
        memset(&typeGUID,0,sizeof(GUID));
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
        } /* endif */

        pTypeInfo2->Release();
      } /* endif */
    } /* endfor */
#endif

    // retrieve constants. do this only once for each type library
    // if this was already done, use the cached information (from an extra list)
    if ( (cachedInfo = fFindConstantInTypeLib(&typeGUID)))
      pClsInfo->pConstInfo = cachedInfo;
    else {
      // now iterate through whole type library and find all constants
      for (j = 0; j < (INT) iTypeInfoCount; j++) {
        hResult = pTypeLib->GetTypeInfoType(j, &iTypeInfoType);
        if (iTypeInfoType == TKIND_ENUM || iTypeInfoType == TKIND_MODULE) {
          hResult = pTypeLib->GetTypeInfo(j, &pTypeInfo2);
          if (hResult == S_OK) {
            hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);
            if (hResult == S_OK) {
              fExploreTypeAttr( pTypeInfo2, pTypeAttr, pClsInfo );
              pTypeInfo2->ReleaseTypeAttr(pTypeAttr);
            }
           pTypeInfo2->Release();
          }
        }
      }
      if (pClsInfo->pConstInfo)
        addTypeListElement(&typeGUID, pClsInfo->pConstInfo);
    }

    pTypeLib->Release();
  } /* endif */

  return fOk;
}

/* Retrieve all information to invoke an event in case it occurs */
POLEFUNCINFO2 GetEventInfo(ITypeInfo *pTypeInfo, RexxObject *self, POLECLASSINFO pClsInfo)
{
  TYPEATTR *pTypeAttr = NULL;
  FUNCDESC *pFuncDesc = NULL;
  HRESULT   hResult;
  POLEFUNCINFO2 pEventList = NULL;
  POLEFUNCINFO2 pTemp;
  POLEFUNCINFO  pFuncInfo = NULL;
  BSTR     *pbStrings;
  BOOL      fNonHidden = TRUE;
  int j;
  unsigned short i;
  unsigned int uFlags;

  hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);

  if (hResult == S_OK) {
    for (i=0;i<pTypeAttr->cFuncs;i++) {
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

          if (pClsInfo) {
            pFuncInfo = pClsInfo->pFuncInfo;
            if (pFuncInfo) {
              while (pFuncInfo) {
                if (!stricmp(pFuncInfo->pszFuncName,szBuffer)) {
                  sprintf(szBuffer,"OLEEvent_%S",pbStrings[0]);
                  pFuncInfo = NULL;
                } else pFuncInfo=pFuncInfo->pNext;
              } // end while
            } // end if
          } // end if

          SysFreeString(pbStrings[0]);

          // insert new entry at the beginning
          pTemp = pEventList;
          pEventList = (POLEFUNCINFO2) ORexxOleAlloc( sizeof(OLEFUNCINFO2) );
          pEventList->pNext = pTemp;
          // fill in necessary info to let OLEObjectEvent::Invoke do its job...
          pEventList->pszFuncName = (PSZ) ORexxOleAlloc(1+strlen(szBuffer));
          strcpy(pEventList->pszFuncName, szBuffer);

          hResult = pTypeInfo->GetDocumentation(pFuncDesc->memid,NULL,&bString,NULL,NULL);
          if (hResult == S_OK) {
            sprintf(szBuffer,"%S",bString);
            SysFreeString(bString);
            pEventList->pszDocString = (PSZ) ORexxOleAlloc(1+strlen(szBuffer));
            strcpy(pEventList->pszDocString, szBuffer);
          } else pEventList->pszDocString = NULL;

          pEventList->memId = pFuncDesc->memid;
          pEventList->invkind = pFuncDesc->invkind;
          pEventList->iParmCount = pFuncDesc->cParams;
          pEventList->iOptParms = pFuncDesc->cParamsOpt;
          pEventList->pOptVt = (VARTYPE *) ORexxOleAlloc(pFuncDesc->cParams * sizeof(VARTYPE));
          pEventList->pusOptFlags = (PUSHORT) ORexxOleAlloc(pFuncDesc->cParams * sizeof(USHORT));
          pEventList->pszName = (char**) ORexxOleAlloc(pFuncDesc->cParams * sizeof(char*));

          for (j=0;j<pEventList->iParmCount;j++) {
            pEventList->pOptVt[j] = pFuncDesc->lprgelemdescParam[j].tdesc.vt;
            pEventList->pusOptFlags[j] = pFuncDesc->lprgelemdescParam[j].paramdesc.wParamFlags;

            if (pbStrings[j+1]) {
              sprintf(szBuffer,"%S",pbStrings[j+1]);
              SysFreeString(pbStrings[j+1]);
            } else sprintf(szBuffer,"<unnamed>");

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
  OLEObjectEvent *pEventHandler = NULL;
  unsigned int index;
  unsigned int icnt, icnt2;
  int implFlags=0;
  BOOL fFound = false;
  HRESULT hResult;

  /* first, get the type library from the type information of this OLE object */
  hResult = pTypeInfo->GetContainingTypeLib(&pTypeLib,&index);
  if (hResult == S_OK) {
    index = pTypeLib->GetTypeInfoCount();

    icnt=0;
    /* look at each entry... */
    while (icnt<index && !fFound) {
      hResult = pTypeLib->GetTypeInfoType(icnt,&kind);
      /* ...and check if it is a coclass */
      if (hResult == S_OK && kind == TKIND_COCLASS) {
        hResult = pTypeLib->GetTypeInfo(icnt,&pCoClass);
        if (hResult == S_OK) {
          hResult = pCoClass->GetTypeAttr(&pTypeAttr);
          /* yes... does it describe this object? */
          if (hResult == S_OK && IsEqualCLSID(*clsID,pTypeAttr->guid) ) {
            /* yes... look for the [default, source] entry */
            for (icnt2=0;icnt2<pTypeAttr->cImplTypes;icnt2++) {
              hResult = pCoClass->GetImplTypeFlags(icnt2,&implFlags);
              if (FAILED(hResult)) continue;
              if ((implFlags & IMPLTYPEFLAG_FDEFAULT) &&
                  (implFlags & IMPLTYPEFLAG_FSOURCE)) {
                HREFTYPE hRefType = NULL;
                /* found it, get the type information & the IID */
                pCoClass->GetRefTypeOfImplType(icnt2,&hRefType);
                hResult = pCoClass->GetRefTypeInfo(hRefType,ppTypeInfoEvents);
                if (hResult == S_OK) {
                  TYPEATTR *pTempTypeAttr = NULL;
                  if ((*ppTypeInfoEvents)->GetTypeAttr(&pTempTypeAttr) == S_OK) {
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
//     objectClass (REXXOBJECT) - The OLE class to create an object of
//     events (REXXOBJECT)      - String to indicate whether to use events or not
//     getObjectFlag (REXXOBJECT) - Flag to indicate whether we try a GetObject
//                                  (undocumented, unused!)
//
//   Returned:
//     returnObject (REXXOBJECT) - The created proxy object.
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
RexxMethod4(REXXOBJECT,                // Return type
            OLEObject_Init,            // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, objectClass,   // Class specifier for new object
            REXXOBJECT, events,        // keyword to active event handling
            REXXOBJECT, getObjectFlag) // Try a GetActiveObject
{
  RexxString *argString;
  RexxString *eventString = NULL;
  CLSID       clsID;
  HRESULT     hResult;
  CHAR        *pszArg;
  CHAR        szBuffer[200];
  LPOLESTR    lpUniBuffer = NULL;
  OLECHAR     OleBuffer[100];
  LPOLESTR    lpOleStrBuffer = OleBuffer;
  IUnknown   *pUnknown = NULL;
  IDispatch  *pDispatch = NULL;
  ITypeInfo  *pTypeInfo = NULL;
  IProvideClassInfo *pProvideClassInfo = NULL;
  IConnectionPointContainer *pConnectionPointContainer = NULL;  // for event handling
  DWORD       dwCookie = 0;                                     // for event handling
  POLEFUNCINFO2 pEventList = NULL;                              // list with all "handleable" events
  unsigned int  iTypeInfoCount;
  POLECLASSINFO pClsInfo = NULL;
#ifdef DEBUG_TESTING
  BOOL          gotIDispatch = false;
#endif

  if ( !fInitialized )
    OLEInit();

  if (iInstanceCount == 0) OleInitialize(NULL);
  iInstanceCount++;

  argString = (RexxString *) RexxSend1(objectClass, "REQUEST", RexxString("STRING"));
  if ( !_isstring(argString) )
    send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("1")));
  if (events) {
    eventString = (RexxString *) RexxSend1(events, "REQUEST", RexxString("STRING"));
    if ( !_isstring(eventString) )
      send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("2")));
  }

  /* get pointer to string data and convert to Unicode */
  pszArg = string_data(argString);
  lpUniBuffer = lpAnsiToUnicode(pszArg, string_length(argString) + 1);

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
    } /* endif */

    ORexxOleFree( lpUniBuffer );
    lpUniBuffer = NULL;
  }
  else
  {
    send_exception(Error_Interpretation_initialization);
  } /* endif */

  if (hResult == S_OK)
  {
    /* now store the CLSID and ProgID with the object attributes */
    PSZ         pszAnsiStr = NULL;

    hResult = StringFromCLSID(clsID, &lpOleStrBuffer);
    pszAnsiStr = pszUnicodeToAnsi(lpOleStrBuffer);
    if (SUCCEEDED(hResult)) CoTaskMemFree(lpOleStrBuffer); // memory was not freed
    if (pszAnsiStr)
    {
      REXX_SETVAR("!CLSID", RexxString(pszAnsiStr));
      pClsInfo = psFindClassInfo(pszAnsiStr, NULL);
      if ( pClsInfo )
      {
        pClsInfo->iInstances++; /* add to instance counter */
        if (!pClsInfo->pszCLSId)
          pClsInfo->pszCLSId = pszAnsiStr;
        else
          ORexxOleFree(pszAnsiStr);
      } /* endif */
      else ORexxOleFree(pszAnsiStr); // free this memory!
    } /* endif */

    hResult = ProgIDFromCLSID(clsID, &lpOleStrBuffer);
    pszAnsiStr = pszUnicodeToAnsi(lpOleStrBuffer);
    if (SUCCEEDED(hResult)) CoTaskMemFree(lpOleStrBuffer); // memory was not freed
    if (pszAnsiStr)
    {
      REXX_SETVAR("!PROGID", RexxString(pszAnsiStr));
      if (pClsInfo) {
        if (!pClsInfo->pszProgId)
          pClsInfo->pszProgId = pszAnsiStr;
        else
          ORexxOleFree(pszAnsiStr);
      } else ORexxOleFree(pszAnsiStr);
    } /* endif */

    /* check if CLSID is okay for security manager */
    hResult = S_OK;
    if (creationCallback)
      hResult = creationCallback(clsID, NULL)?S_OK:E_FAIL;
    if (FAILED(hResult)) {
      char errmsg[256];
      sprintf(errmsg, "an external security manager denies creation of %s",pszArg);
      send_exception1(Error_System_service_service, RexxArray1(RexxString(errmsg)));
    }

    if (getObjectFlag == RexxTrue)
    {
      hResult = GetActiveObject(clsID, NULL, &pUnknown);
    }
    else
      hResult = DISP_E_MEMBERNOTFOUND; /* simulate failed GetActiveObject if it was not requested */

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
    } /* endif */

    if (hResult == S_OK)
    {
      hResult = pUnknown->QueryInterface(IID_IDispatch, (LPVOID*)&pDispatch);
    } /* endif */

    if (pUnknown)
    {
      pUnknown->Release(); /* this is no longer needed */
      pUnknown = NULL;
    } /* endif */
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
          if (SUCCEEDED(hr)) {
            CLSID ClsId;
            pPersist->GetClassID(&ClsId);
            pPersist->Release();
          }
        }
#endif
      }
      else
        send_exception(Error_Interpretation_initialization);
    }
    else
    {
      send_exception1(Error_Execution_noclass, RexxArray1(RexxString(pszArg)));
    } /* endif */
  } /* endif */

  sprintf(szBuffer, "%p", pDispatch);
  REXX_SETVAR("!IDISPATCH", RexxString(szBuffer));

  if ( (hResult != S_OK) || (pDispatch == NULL) )
    send_exception(Error_No_OLE_instance);

  // changed logic so it can never, not even theoretically, crash here...
  hResult = E_FAIL;
  if ( !pClsInfo ) hResult = S_OK;
  else if ( !(pClsInfo->pTypeInfo) ) hResult = S_OK;
//  if ( !pClsInfo || !(pClsInfo->pTypeInfo) )
  if ( hResult == S_OK)
  {
    /* see if we can get some information about the object */
    hResult = pDispatch->GetTypeInfoCount(&iTypeInfoCount);

    if ((hResult == S_OK) && iTypeInfoCount)
    {
      hResult = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);
      /* store type info with object */
      if (SUCCEEDED(hResult)) { // when successful
        sprintf(szBuffer, "%p", pTypeInfo);
        REXX_SETVAR("!ITYPEINFO", RexxString(szBuffer));
      }

      if (!pClsInfo && (hResult == S_OK) && pTypeInfo)
      {
        /* store type info with object */
//M        sprintf(szBuffer, "%p", pTypeInfo);
//M        REXX_SETVAR("!ITYPEINFO", RexxString(szBuffer));

        /* search/allocate class info block for this typeinfo */
        pClsInfo = psFindClassInfo(NULL, pTypeInfo);
        if ( pClsInfo )
        {
          pClsInfo->iInstances++; /* add to instance counter */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  REXX_SETVAR("!OUTARRAY", RexxNil);    // set array of out parameters to nonexistent

  if (pClsInfo && !(pClsInfo->pTypeInfo)) {
    /* store typeinfo pointer in object data }*/
    pClsInfo->pTypeInfo = pTypeInfo;

    if ( pTypeInfo )
      fExploreTypeInfo( pTypeInfo, pClsInfo );
  }

#ifdef DEBUG_TESTING
  if (pClsInfo->pTypeInfo && gotIDispatch) {
    TYPEATTR     *pTypeAttr;
    hResult = pClsInfo->pTypeInfo->GetTypeAttr(&pTypeAttr);
    if (SUCCEEDED(hResult)) {
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
  if (FAILED(hResult)) {
    char errmsg[256];
    sprintf(errmsg, "an external security manager denies usage of %s",pszArg);
    send_exception1(Error_System_service_service, RexxArray1(RexxString(errmsg)));
  }
#endif
  // if pTypeInfo is not set (happens when an instance of the class was already created),
  // get it: needed for event handling!
  if (!pTypeInfo && pClsInfo) pTypeInfo = pClsInfo->pTypeInfo;

  // event handling:  only connect the events if this is explicitly desired.
  // Note: ooRexx does not, at this time, support events if the ITypeInfo
  // interface pointer could not be obtained.  However, it should be able to
  // support events with a redesign.
  if (eventString && pTypeInfo) {
    bool connect = false;

    if (!strcmpi(string_data(eventString),"WITHEVENTS")) connect = true;
    /* if (!strcmpi(string_data(eventString),"WITHEVENTS")) */ {
      // is there a connection point container? yes: events are supported
      hResult = pDispatch->QueryInterface(IID_IConnectionPointContainer, (LPVOID*)&pConnectionPointContainer);
      if (hResult == S_OK) {
        OLEObjectEvent *pEventHandler = NULL;
        IConnectionPoint *pConnectionPoint = NULL;
        ITypeInfo *pTempTypeInfo = NULL;
        IID theIID;

        if (GetEventTypeInfo(pTypeInfo,&clsID,&pTempTypeInfo,&theIID) == (BOOL) true)
          hResult = pConnectionPointContainer->FindConnectionPoint(theIID,&pConnectionPoint);
        else hResult = -1;  // simulate failure

        if (hResult == S_OK) {
          /* get names, dispids & parameters of the interfaces functions */
          pEventList = GetEventInfo(pTempTypeInfo,self,pClsInfo);
          pTempTypeInfo->Release();

          if (pEventList) {
            pEventHandler = new OLEObjectEvent(pEventList,self,theIID);

            if (connect) {
              hResult = pConnectionPoint->Advise((IUnknown*) pEventHandler, &dwCookie);
              if (hResult == S_OK) {
                sprintf(szBuffer, "%p", pEventHandler);
                REXX_SETVAR("!EVENTHANDLER", RexxString(szBuffer));
                sprintf(szBuffer, "%ld", dwCookie);
                REXX_SETVAR("!EVENTHANDLERCOOKIE", RexxString(szBuffer));
                sprintf(szBuffer, "%p", pConnectionPoint);
                REXX_SETVAR("!CONNECTIONPOINT", RexxString(szBuffer));
              } else {
                pConnectionPoint->Release();
                pEventHandler->Release();
              }
            } else {
              // just "fake" a connection
              sprintf(szBuffer, "%p", pEventHandler);
              REXX_SETVAR("!EVENTHANDLER", RexxString(szBuffer));
              pConnectionPoint->Release();
            }
          } /* end if event list */
        } /* end if connectionpoint */

        pConnectionPointContainer->Release();
      } /* end if cp container exists */
    } /* end if event string == "WITHEVENTS" */
  } /* end if event string != null */

  return RexxNil;
}

//******************************************************************************
// Method:  OLEObject_Uninit
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     returnObject (REXXOBJECT) - This method only returns .NIL
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
RexxMethod1(REXXOBJECT,                // Return type
            OLEObject_Uninit,          // Object_method name
            OSELF, self)               // Pointer to self
{
  RexxString       *RxString;
  CHAR             *pszRxString;
  IDispatch        *pDispatch = NULL;
  ITypeInfo        *pTypeInfo = NULL;
  POLECLASSINFO     pClsInfo = NULL;
  OLEObjectEvent   *pEventHandler = NULL;  // event handling
  IConnectionPoint *pConnectionPoint = NULL; // event handling
  DWORD             dwCookie = 0;  // event handling

  if ( !fInitialized )
    OLEInit();

  /* get event handler & release */
  RxString = (RexxString *)REXX_GETVAR("!EVENTHANDLER");
  pszRxString = string_data(RxString);
  if (*pszRxString != '!') {
    sscanf(pszRxString, "%p", &pEventHandler);

    RxString = (RexxString *)REXX_GETVAR("!CONNECTIONPOINT");
    pszRxString = string_data(RxString);
    if (*pszRxString != '!')
      sscanf(pszRxString, "%p", &pConnectionPoint);

    RxString = (RexxString *)REXX_GETVAR("!EVENTHANDLERCOOKIE");
    pszRxString = string_data(RxString);
    sscanf(pszRxString, "%ld", &dwCookie);

    if (pConnectionPoint != NULL) {
      pConnectionPoint->Unadvise(dwCookie);   // remove connection
      pConnectionPoint->Release();            // free cp
    }
    if (pEventHandler != NULL) {
      pEventHandler->Release();        // terminate event handler
    }

  }

  /* get the IDispatch pointer for that object */
  RxString = (RexxString *)REXX_GETVAR("!IDISPATCH");
  pszRxString = string_data(RxString);
  sscanf(pszRxString, "%p", &pDispatch);
  //fprintf(stdout,"DBG: %p _Uninit\n",pDispatch);

  /* get the CLSID for that object */
  RxString = (RexxString *)REXX_GETVAR("!CLSID");
  pszRxString = string_data(RxString);
  if (*pszRxString != '!')
  {
    pClsInfo = psFindClassInfo(pszRxString, NULL);
    if (pClsInfo)
      pTypeInfo = pClsInfo->pTypeInfo;
  }
  else
  {
    /* we don't have a CLSID, so get type info pointer */
    RxString = (RexxString *)REXX_GETVAR("!ITYPEINFO");
    pszRxString = string_data(RxString);
    sscanf(pszRxString, "%p", &pTypeInfo);
    if (pTypeInfo)
      pClsInfo = psFindClassInfo(NULL, pTypeInfo);
  } /* endif */

  /* if typeinfo was NOT retrieved from !ITYPEINFO */
  /* of the object it must NOT be released!!       */
  RxString = (RexxString *)REXX_GETVAR("!ITYPEINFO");
  pszRxString = string_data(RxString);
  if (*pszRxString != '!') {
//  if ( pTypeInfo ) /* we have a type info - release instance */
//  {
    pTypeInfo->Release();
    pTypeInfo = NULL;
  } /* endif */

  if ( pClsInfo )
  {
    /* reduce instance counter, clear item if 0 reached */
    pClsInfo->iInstances--;
    if ( pClsInfo->iInstances == 0 )
      ClearClassInfoBlock( pClsInfo );
  } /* endif */

  if ( pDispatch )
  {
    pDispatch->Release();
  } /* endif */

  if (iInstanceCount > 0) {
    iInstanceCount--;
    if (iInstanceCount == 0) {
      destroyTypeLibList();  // remove info list on constants
      OleUninitialize();
    }
  }
  return RexxNil;
}


/* handle rereferencing/dereferencing for out-parameters
   => sets VT_BYREF and changes variant correctly */
// make the VARIANT a VT_BYREF VARIANT
// this sets the VT_BYREF flag in the vt member,
// creates a pointer to the content of the variant
// and changes the entry in the variant from
// xxxVal to pxxxVal (ptr)
void referenceVariant(VARIANT *obj)
{
  void *pNew;

  switch (obj->vt) {
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
    break;
  }
  obj->vt |= VT_BYREF;
}

// change a by-ref VARIANT that was created with
// referenceVariant() back to an unreferenced one.
// this free the pointer and moves the content
// "into" this variant by changing the entry from
// pxxxVal (ptr) to xxxVal ...
void dereferenceVariant(VARIANT *obj)
{
  void *temp;

  obj->vt ^= VT_BYREF;
  switch (obj->vt) {
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
    break;
  }
}

//******************************************************************************
// Method:  OLEObject_Unknown
//
//   Arguments:
//     self - A pointer to self
//     msgName (REXXOBJECT) - The name of OLE method being called.
//     msgArgs (REXXOBJECT) - An array containing the parameters for the OLE
//                            method.
//
//   Returned:
//     returnObject (REXXOBJECT) - The result from the OLE method invocation.
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
RexxMethod3(REXXOBJECT,                // Return type
            OLEObject_Unknown,         // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, msgName,       // Name of OLE method being called
            REXXOBJECT, msgArgs)       // Array containing OLE method parameters
{
  INT             i;
  HRESULT         hResult;
  CHAR            szBuffer[2048];
  CHAR           *pszFunction;
  CHAR           *pszRxString;
  RexxString     *RxString;
  RexxObject     *arrItem;
  IDispatch      *pDispatch = NULL;
  IDispatchEx    *pDispatchEx = NULL;
  ITypeInfo      *pTypeInfo = NULL;
  MEMBERID        MemId;
  POLECLASSINFO   pClsInfo = NULL;

  int             iArgCount;
  unsigned short  wFlags = 0;
  DISPPARAMS      dp;
  VARIANTARG     *pVarArgs;
  VARIANTARG     *pInputParameters;
  DISPID          PropPutDispId = DISPID_PROPERTYPUT;
  VARIANT         sResult;
  VARIANT        *pResult;
  EXCEPINFO       sExc;
  unsigned int    uArgErr;
  POLEFUNCINFO    pFuncInfo = NULL;
  BOOL            fFound = FALSE;
  VARTYPE         DestVt;
  RexxObject     *ResultObj = RexxNil;

  if ( !fInitialized )
    OLEInit();

  if (msgName == NULL || msgName == RexxNil)
    send_exception1(Error_Incorrect_method_noarg,RexxArray1(RexxString("1")));

  REXX_SETVAR("!OUTARRAY",RexxNil);

  if (msgArgs != NULL && fIsRexxArray(msgArgs))
    iArgCount = array_size(msgArgs);
  else
    iArgCount = 0;

  /* get the IDispatch pointer for that object */
  RxString = (RexxString *)REXX_GETVAR("!IDISPATCH");
  pszRxString = string_data(RxString);
  sscanf(pszRxString, "%p", &pDispatch);
  if (!pDispatch)
    send_exception(Error_Interpretation_initialization);

  /* get the CLSID for that object */
  RxString = (RexxString *)REXX_GETVAR("!CLSID");
  pszRxString = string_data(RxString);
  if (*pszRxString != '!')
  {
    pClsInfo = psFindClassInfo(pszRxString, NULL);
    if (pClsInfo)
      pTypeInfo = pClsInfo->pTypeInfo;
  }
  else
  {
    /* we don't have a CLSID, so get type info pointer */
    RxString = (RexxString *)REXX_GETVAR("!ITYPEINFO");
    pszRxString = string_data(RxString);
    sscanf(pszRxString, "%p", &pTypeInfo);
    if (pTypeInfo)
      pClsInfo = psFindClassInfo(NULL, pTypeInfo);
  } /* endif */

  pszFunction = pszStringDupe(string_data(msgName));
  if (!pszFunction)
    send_exception(Error_System_resources);

  if ( pszFunction[strlen(pszFunction)-1] == '=' )
  {
    /* property put operation */
    pszFunction[strlen(pszFunction)-1] = 0;
    wFlags = DISPATCH_PROPERTYPUT;
  }

  hResult = pDispatch->QueryInterface(IID_IDispatchEx, (LPVOID*)&pDispatchEx);

  fFound = fFindFunction(pszFunction, pDispatch, pDispatchEx, pTypeInfo,
                         pClsInfo, wFlags, &pFuncInfo, &MemId, iArgCount);
  /* this change honors the function description better; before, the wFlag (invocation
   * kind) was determined by use, regardless of the description.
   * now, if this is no direct PROPERTYPUT (format: "obj~function=") the invkind
   * of the function description is used.
   * if no function description is available, we use are "best guess"...
   */
  if (wFlags == 0) {
    if (fFound && pFuncInfo) {
      /* use the invkind the function description contains */
      wFlags = pFuncInfo->invkind; // DISPATCH_... & INVOKE_... are the same (values 2,4,8)
    }
    else
    {
      wFlags = DISPATCH_METHOD;

      if (iArgCount == 0)
      {
        /* this could be a property get or a dispatch method */
        wFlags |= DISPATCH_PROPERTYGET;
      } else {
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
  } /* endif */

/* this was used to allow setting a property by calling it like a method
   this creates problems, because it might "shadow" a method with the same
   signature. thus disabled...
  // call of PROPERTYPUT with braces: e.g. ~visible(.true)
  if (fFound && pFuncInfo) {
    // are we sure this is a PROPERTYGET?
    if (pFuncInfo->invkind == DISPATCH_PROPERTYGET)
      // check if parameter list is valid. but how?
      if (iArgCount > (pFuncInfo->iParmCount - pFuncInfo->iOptParms) ) {
        // oh, we must rethink our call: this must be a property put, and not a get!!!
        wFlags = DISPATCH_PROPERTYPUT;
        fFound = fFindFunction(pszFunction, pDispatch, pDispatchEx, pTypeInfo,
                             pClsInfo, wFlags, &pFuncInfo, &MemId);
      } // endif

  } // endif
*/

  /* free now no longer needed memory */
  ORexxOleFree(pszFunction);

  if (!fFound)
    send_exception1(Error_No_method_name, RexxArray2(self, msgName));

  /* now assemble the parameters for the function call */
//  pVarArgs = (VARIANTARG *) ORexxOleAlloc(sizeof(VARIANTARG) * iArgCount);
  // allocate twice the size to store original variants in 2nd half
  pVarArgs = (VARIANTARG *) ORexxOleAlloc(sizeof(VARIANTARG) * iArgCount * 2);
  // point to second half
  pInputParameters = pVarArgs + iArgCount;
  if (!pVarArgs && iArgCount) // only real error if memory was needed
  {
    send_exception(Error_System_resources);
  } /* endif */

  for (i=0; i < iArgCount; i++)
  {
    /* arguments are filled in from the end of the array */
    VariantInit(&(pVarArgs[iArgCount - i - 1]));

    arrItem = array_at(msgArgs, i + 1);

    if (pTypeInfo && pFuncInfo)
      DestVt = (i<pFuncInfo->iParmCount)?pFuncInfo->pOptVt[i]:VT_EMPTY;
    else
      DestVt = VT_EMPTY;

    Rexx2Variant(arrItem, &(pVarArgs[iArgCount - i - 1]), DestVt, i + 1);

    /* If an out parameter, the variant must be passed as a reference.  The
     * original input variant is saved so that, if a new value is returned, the
     * original can be freed.
     */
    if ( isOutParam(arrItem, pFuncInfo, i) )
    {
      referenceVariant(&pVarArgs[iArgCount - i - 1]);
      memcpy(&pInputParameters[iArgCount - i - 1],&pVarArgs[iArgCount - i- 1],
             sizeof(VARIANT));
    }
  } /* endfor */

  /* if we have a property put then the new property value needs to be a named
   * argument
   */
  if (wFlags == DISPATCH_PROPERTYPUT)
  {
    dp.cNamedArgs = 1;
    dp.rgdispidNamedArgs = &PropPutDispId;
    // without this check, property put is hardcoded to PROPERTYPUT...  ...bad.
    if (fFound && pFuncInfo) {
      /* use the invkind the function description contains */
      wFlags = pFuncInfo->invkind; // DISPATCH_... & INVOKE_... are the same (values 2,4,8)
    }
  }
  else
  {
    dp.cNamedArgs = 0;
    dp.rgdispidNamedArgs = NULL;
  } /* endif */

  dp.cArgs = iArgCount;
  dp.rgvarg = pVarArgs;
  VariantInit(&sResult);

  pResult = &sResult;
  if (pTypeInfo && pFuncInfo)
  {
    if (pFuncInfo->FuncVt == VT_VOID)
      pResult = NULL; /* function has no return code! */

    if (pFuncInfo->invkind & INVOKE_PROPERTYGET)
      wFlags |= DISPATCH_PROPERTYGET; /* this might be a property get */
  } /* endif */

  if (pDispatchEx)
  {
    hResult = pDispatchEx->InvokeEx(MemId, LOCALE_USER_DEFAULT,
                                    wFlags, &dp, pResult, &sExc, NULL);
  }
  else
  {
    hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                wFlags, &dp, pResult, &sExc, &uArgErr);
  } /* endif */

  /* maybe this is a property get with arguments */
  if ((hResult == DISP_E_MEMBERNOTFOUND) && iArgCount)
  {
    hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                wFlags | DISPATCH_PROPERTYGET, &dp,
                                pResult, &sExc, &uArgErr);
  } /* endif */

  /* if function has no return value, try again */
  if (hResult == DISP_E_MEMBERNOTFOUND)
  {
    hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                wFlags, &dp, NULL, &sExc, &uArgErr);
  } /* endif */

  for (i=0; i < (INT) dp.cArgs; i++)
  {
    arrItem = array_at(msgArgs, i + 1);

    /* was this an out parameter? */
    if ( isOutParam(arrItem, pFuncInfo, i) )
    {
      /* yes, then change the REXX object to a new state */
      RexxObject  *outObject;
      RexxObject  *outArray=(RexxObject*) REXX_GETVAR("!OUTARRAY");
      int         index=1;
      char        indexBuffer[32];

      if (outArray == RexxNil) {
        outArray = RexxArray(1);
        REXX_SETVAR("!OUTARRAY",outArray);
      }
      else {
        arrItem = RexxSend0(outArray, "LAST");
        pszRxString = string_data((RexxString*) RexxSend0(arrItem,"STRING"));
        sscanf(pszRxString, "%d", &index);
        index++; // next entry
      }
      outObject = Variant2Rexx(&(dp.rgvarg[dp.cArgs-i-1]));
      sprintf(indexBuffer,"%d",index);
      RexxSend2(outArray,"PUT",outObject,RexxString(indexBuffer));

      if ( fIsOleVariant(arrItem) )
        RexxSend1(arrItem, "!VARVALUE_=", outObject);

      // if the call changed an out parameter, we have to clear the original variant that
      // was overwritten
      if (memcmp(&pInputParameters[iArgCount - i - 1],&pVarArgs[iArgCount - i - 1],sizeof(VARIANT))) {
        dereferenceVariant(&pInputParameters[iArgCount - i - 1]);
        handleVariantClear(&pInputParameters[iArgCount - i - 1], arrItem);
      } else
        dereferenceVariant(&dp.rgvarg[iArgCount - i - 1]);
    }

    /* Clear the argument and, if an OLEVariant, reset the clear variant flag.
     */
    handleVariantClear(&(dp.rgvarg[dp.cArgs-i-1]), arrItem);
    if ( fIsOleVariant(arrItem) )
      RexxSend1(arrItem, "!CLEARVARIANT_=", RexxTrue);
  } /* endfor */

  /* free the argument array */
  ORexxOleFree(pVarArgs);

  if (hResult == S_OK)
  {
    ResultObj = Variant2Rexx(&sResult);
  }
  else
  {
    BSTR bstrUnavail;
    /* error occured */
    switch (hResult)
    {
      case DISP_E_BADPARAMCOUNT:
        send_exception(Error_Argument_Count_Mismatch);
        break;
      case DISP_E_BADVARTYPE:
        send_exception(Error_Invalid_Variant);
        break;
      case DISP_E_EXCEPTION:
        bstrUnavail= lpAnsiToUnicode("unavailable", sizeof("unavailable"));
        /* if there is a routine for deferred fill-in, call it */
        if (sExc.pfnDeferredFillIn) {
          (*sExc.pfnDeferredFillIn)(&sExc);
          sprintf(szBuffer, "Code: %04x Source: %S Description: %S",
                  sExc.wCode, sExc.bstrSource , sExc.bstrDescription);
          SysFreeString(sExc.bstrSource);
          SysFreeString(sExc.bstrDescription);
          SysFreeString(sExc.bstrHelpFile);
        } else {
          sprintf(szBuffer, "Code: %08x Source: %S Description: %S",
            sExc.wCode==0?sExc.scode:sExc.wCode,
            sExc.bstrSource==NULL?bstrUnavail:sExc.bstrSource,
            sExc.bstrDescription==NULL?bstrUnavail:sExc.bstrDescription);
        }
        ORexxOleFree(bstrUnavail);
        send_exception1(Error_OLE_Exception,RexxArray1(RexxString(szBuffer)));
        break;
      case DISP_E_MEMBERNOTFOUND:
        send_exception(Error_Unknown_OLE_Method);
        break;
      case DISP_E_OVERFLOW:
        send_exception(Error_Coercion_Failed_Overflow);
        break;
      case DISP_E_TYPEMISMATCH:
        sprintf(szBuffer, "%d", uArgErr + 1);
        send_exception1(Error_Coercion_Failed_Type_Mismatch,RexxArray1(RexxString(szBuffer)));
        break;
      case DISP_E_PARAMNOTOPTIONAL:
        send_exception(Error_Parameter_Omitted);
        break;
      case RPC_E_DISCONNECTED:
        send_exception(Error_Client_Disconnected_From_Server);
        break;
      case DISP_E_NONAMEDARGS:
      case DISP_E_UNKNOWNLCID:
      case DISP_E_UNKNOWNINTERFACE:
      case DISP_E_PARAMNOTFOUND:
        // uArgErr contains the index of the wrong parameter
      default:
        sprintf(szBuffer, "%8.8X", hResult);
        send_exception1(Error_Unknown_OLE_Error,RexxArray1(RexxString(szBuffer)));
        break;
    } /* endswitch */

    ResultObj = (RexxObject*) RexxString(szBuffer);
  } /* endif */

  /* remove reference count for result object */
  VariantClear(&sResult);

  if (pDispatchEx)
  {
    pDispatchEx->Release();
  } /* endif */

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
 *                   contained withing the OLEVariant object.
 *
 * @param DestVt     The VT type that the automatic conversion believes the
 *                   ooRexx object should be coerced to.
 *
 * @param pRxObject  [Returned] The real ooRexx object to convert.
 *
 * @param pDestVt    [Returned] The real VT type to coerce the ooRexx object to.
 *
 * @return           True, the conversion is complete, or false, the conversion
 *                   is not complete - continue with the automatic conversion.
 */
BOOL checkForOverride( VARIANT *pVariant, RexxObject *RxObject, VARTYPE DestVt,
                       RexxObject **pRxObject, VARTYPE *pDestVt )
{
  BOOL converted = FALSE;

  if ( ! fIsOleVariant(RxObject) )
  {
    *pRxObject = RxObject;
    *pDestVt   = DestVt;
  }
  else
  {
    RexxObject *tmpRxObj = RexxSend0(RxObject, "!_VT_");

    *pRxObject = RexxSend0(RxObject, "!VARVALUE_");
    if ( tmpRxObj == RexxNil )
    {
      /* Do not override default conversion. */
      *pDestVt = DestVt;
    }
    else
    {
      *pDestVt = (VARTYPE)_integer(tmpRxObj);

      switch ( *pDestVt & VT_TYPEMASK )
      {
        case VT_NULL :
          V_VT(pVariant) = VT_NULL;
          converted = TRUE;
          break;

        case VT_EMPTY :
          V_VT(pVariant) = VT_EMPTY;
          converted = TRUE;
          break;

        case VT_DISPATCH :
          if ( *pRxObject == RexxNil || *pRxObject == NULL )
          {
            if ( *pDestVt & VT_BYREF )
            {
              IDispatch **ppDisp;

              ppDisp = (IDispatch **)ORexxOleAlloc(sizeof(IDispatch **));
              if ( ! ppDisp )
                send_exception(Error_System_resources);

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
            RexxSend1(RxObject, "!CLEARVARIANT_=", RexxFalse);
            converted = TRUE;
            break;
          }
          /* Let default conversion handle non-nil. */
          break;

        case VT_UNKNOWN :
          if ( *pRxObject == RexxNil || *pRxObject == NULL )
          {
            if ( *pDestVt & VT_BYREF )
            {
              IUnknown **ppU;

              ppU = (IUnknown **)ORexxOleAlloc(sizeof(IUnknown **));
              if ( ! ppU )
                send_exception(Error_System_resources);

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
            RexxSend1(RxObject, "!CLEARVARIANT_=", RexxFalse);

            converted = TRUE;
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
BOOL isOutParam( RexxObject *param, POLEFUNCINFO pFuncInfo, INT i )
{
  USHORT  paramFlags = PARAMFLAG_NONE;
  BOOL    overridden = FALSE;

  if ( fIsOleVariant(param) )
  {
    RexxObject *tmpRxObj = RexxSend0(param, "!_PFLAGS_");
    if ( tmpRxObj != RexxNil )
    {
      paramFlags = (USHORT)_integer(tmpRxObj);
      overridden = TRUE;
    }
  }

  if ( !overridden && pFuncInfo && i < pFuncInfo->iParmCount )
    paramFlags = pFuncInfo->pusOptFlags[i];

  return ((paramFlags & PARAMFLAG_FOUT) && !(paramFlags & PARAMFLAG_FRETVAL));
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
VOID handleVariantClear( VARIANT *pVariant, RexxObject *RxObject )
{
  if ( ! okayToClear(RxObject) )
  {
    /* This reverses work done in Rexx2Variant when the parameter is an
     * OLEVariant object.
     */

    switch ( V_VT(pVariant) & VT_TYPEMASK )
    {
      case VT_DISPATCH :
        if ( V_DISPATCH(pVariant) != NULL)
          ORexxOleFree(V_DISPATCH(pVariant));
        break;

      case VT_UNKNOWN :
        if ( V_UNKNOWN(pVariant) != NULL)
          ORexxOleFree(V_UNKNOWN(pVariant));
        break;

      default :
        break;
    }
  }
  else
    VariantClear(pVariant);
}

/**
 * Check if it is okay to use VariantClear.
 */
__inline BOOL okayToClear( RexxObject *RxObject )
{
  if ( fIsOleVariant(RxObject) )
  {
    return (RexxSend0(RxObject, "!CLEARVARIANT_") == RexxTrue);
  }
  return TRUE;
}

//******************************************************************************
// Method:  OLEObject_Request
//
//   Arguments:
//     self - A pointer to self
//     classID (REXXOBJECT) - The string identifier of the requested class.
//                            (currently only ARRAY is supported)
//
//   Returned:
//     returnObject (REXXOBJECT) - The result of the conversion.
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
RexxMethod2(REXXOBJECT,                // Return type
            OLEObject_Request,         // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, classID)       // Name of OSA event to be sent
{
  HRESULT         hResult;
  CHAR           *pszRxString;
  RexxString     *RxString;
  RexxObject     *ResultObj = RexxNil;
  RexxObject     *RxItem;
  IDispatch      *pDispatch = NULL;
  IDispatchEx    *pDispatchEx = NULL;
  ITypeInfo      *pTypeInfo = NULL;
  MEMBERID        MemId;
  POLECLASSINFO   pClsInfo = NULL;
  POLEFUNCINFO    pFuncInfo = NULL;
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
    OLEInit();

  /* get the IDispatch pointer for that object */
  RxString = (RexxString *)REXX_GETVAR("!IDISPATCH");
  pszRxString = string_data(RxString);
  sscanf(pszRxString, "%p", &pDispatch);
  if (!pDispatch)
    send_exception(Error_Interpretation_initialization);

  /* get the CLSID for that object */
  RxString = (RexxString *)REXX_GETVAR("!CLSID");
  pszRxString = string_data(RxString);
  if (*pszRxString != '!')
  {
    pClsInfo = psFindClassInfo(pszRxString, NULL);

    if (pClsInfo)
      pTypeInfo = pClsInfo->pTypeInfo;
  }
  else
  {
    /* we don't have a CLSID, so get type info pointer */
    RxString = (RexxString *)REXX_GETVAR("!ITYPEINFO");
    pszRxString = string_data(RxString);
    sscanf(pszRxString, "%p", &pTypeInfo);
    if (pTypeInfo)
      pClsInfo = psFindClassInfo(NULL, pTypeInfo);
  } /* endif */

  if (stricmp(string_data(classID), "ARRAY") == 0)
  {
    /* first check if there is a _NewEnum method and use this */
    fFound = fFindFunction("_NewEnum", pDispatch, pDispatchEx, pTypeInfo,
                           pClsInfo, 0, NULL, &MemId, -1);

    if (fFound) {
      /* get the count of items in the collection */
      dp.cNamedArgs = 0;
      dp.rgdispidNamedArgs = NULL;
      dp.cArgs = 0;
      dp.rgvarg = NULL;
      VariantInit(&sResult);

      // some objects behave very badly, therefore this code (ugly!):
      if (MemId == -1) {
        //fprintf(stderr,"Warning: Suspicious MemId for _NewEnum! Using DISP_NEWENUM (-4) instead.\n");
        MemId = -4;
      } //else fprintf(stderr,"MemId seems to be good\n");


      hResult = pDispatch->Invoke(MemId, IID_NULL, LOCALE_USER_DEFAULT,
                                  DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                                  &dp, &sResult, &sExc, &uArgErr);
      if (SUCCEEDED(hResult)) {
        IUnknown     *pUnknown=V_UNKNOWN(&sResult);
        IEnumVARIANT *pEnum = NULL;

        pUnknown->AddRef();  // VariantClear will remove one reference!
        // clear result from last invocation
        VariantClear(&sResult);

        // get IEnumVARIANT interface
        hResult = pUnknown->QueryInterface(IID_IEnumVARIANT, (LPVOID*) &pEnum);
        if (hResult == S_OK) {
          ULONG lFetched = 0;
          char pszBuff[32];

          iItemCount = 1;

          hResult = pEnum->Reset();     // set enumerator to first item
          ResultObj = RexxArray(0);     // create REXX array

          VariantInit(&sResult);

          while(pEnum->Next(1,&sResult,&lFetched) == S_OK) {
            RxItem = Variant2Rexx(&sResult);
            sprintf(pszBuff,"%d",iItemCount++);
            //array_put(ResultObj, RxItem, iItemCount);
            RexxSend2(ResultObj,"PUT",RxItem,RexxString(pszBuff));
            VariantClear(&sResult);
            //iItemCount++;
          }

          pEnum->Release();   // enumerator release returns 1, is that ok?
        } else fFound = 0;

        pUnknown->Release();
      } else {
        // ADSI has it's own special way of error reporting (...)
        // these will all get swallowed and an empty Array will be returned
        if (hResult == 0x80020009) {
          switch (sExc.scode) {
          case 0x800704b8:        // IADsContainer does not contain items
          case 0x800704c6:        // Network not present
            ResultObj = RexxArray(0);
            fFound = 1;
            break;
          // more error codes are expected...
          default:
            fFound = 0;
            break;
          }
        }
      }
    }

    /* no enumerator supplied, just assume integer indices and go...     */
    /* see if the OLEObject supports Count/Item                          */
    if (!fFound) {
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

          if (VariantChangeType( &sTempVariant, &sResult, 0, VT_I4) == S_OK)
          {
            iItemCount = sTempVariant.lVal;
          }
          else
          {
            // VARIANT change to integer failed (should not happen, really)
            send_exception1(Error_Execution_noarray,RexxArray1((RexxString*) RexxSend0(self,"STRING")));
          } /* endif */

          VariantClear(&sResult);
          VariantClear(&sTempVariant);

          fFound = fFindFunction("Item", pDispatch, pDispatchEx, pTypeInfo,
                                 pClsInfo, 0, NULL, &MemId, -1);
          if (fFound)
          {
            /* Count & Item are understood -> return an array in any case */
            ResultObj = RexxArray(iItemCount);
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
              if (FAILED(hResult) && iIdx==0) {
                iIdxBaseShift=0;
                iItemCount++;
                continue;
              }
              if (hResult == S_OK)
              {
                /* create a new REXX object from the result */
                RxItem = Variant2Rexx(&sResult);
                array_put(ResultObj, RxItem, iIdx + iIdxBaseShift);
              } /* endif */

              VariantClear(&sResult);
              VariantClear(&sTempVariant);
            } /* endfor */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
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
//     returnObject (REXXOBJECT) - The result of the GETVAR operation
//
//   Notes:
//     This method will retrieve the object variable stored with the current
//     OLE proxy object under the specified name. This allows other methods
//     to get hold of internal data stored with other REXX OLE objects they
//     are handling.
//
//******************************************************************************
RexxMethod2(REXXOBJECT,                // Return type
            OLEObject_GetVar,          // Object_method name
            OSELF, self,               // Pointer to self
            CSTRING, varName)          // string defining variable to query
{
  /* get the desired variable */
  return REXX_GETVAR(varName);
}


//******************************************************************************
// Method:  OLEObject_GetConst
//
//   Arguments:
//     self - A pointer to self
//     constName (CSTRING) - The name of the object constant to retrieve.
//
//   Returned:
//     returnObject (REXXOBJECT) - The converted Object REXX object referring
//                                 to the object constant or .Nil.
//                             OR: a stem with all names and values of known
//                                 constants if constname was not specified
//
//   Notes:
//     This method will retrieve the object constant defined in the type
//     library of the object. If the constant with this name can not be
//     found .NIL will be returned indicating failure of the search.
//
//******************************************************************************
RexxMethod2(REXXOBJECT,                // Return type
            OLEObject_GetConst,        // Object_method name
            OSELF, self,               // Pointer to self
            CSTRING, constName)        // string defining constant to query
{
  RexxString      *RxString;
  CHAR            *pszRxString;
  IDispatch       *pDispatch = NULL;     // IDispatch is not NEEDED !!! remove!
  ITypeInfo       *pTypeInfo = NULL;
  POLECLASSINFO   pClsInfo = NULL;
  POLECONSTINFO   pConstInfo = NULL;
  RexxObject      *RxResult = RexxNil;
  BOOL            fFound = FALSE;

  if ( !fInitialized )
    OLEInit();

  /* get the IDispatch pointer for that object
  RxString = (RexxString *)REXX_GETVAR("!IDISPATCH");
  pszRxString = string_data(RxString);
  sscanf(pszRxString, "%p", &pDispatch);
*/

  /* get the CLSID for that object */
  RxString = (RexxString *)REXX_GETVAR("!CLSID");
  pszRxString = string_data(RxString);
  if (*pszRxString != '!')
  {
    pClsInfo = psFindClassInfo(pszRxString, NULL);
    if (pClsInfo)
      pTypeInfo = pClsInfo->pTypeInfo;
  }
  else
  {
    /* we don't have a CLSID, so get type info pointer */
    RxString = (RexxString *)REXX_GETVAR("!ITYPEINFO");
    pszRxString = string_data(RxString);
    sscanf(pszRxString, "%p", &pTypeInfo);
    if (pTypeInfo)
      pClsInfo = psFindClassInfo(NULL, pTypeInfo);
  } /* endif */

  if ( pClsInfo && constName)
  {
    fFound = fFindConstant(constName, pClsInfo, &pConstInfo);
    if ( fFound )
      RxResult = Variant2Rexx( &(pConstInfo->sValue) );
  } /* endif */
  else if ( pClsInfo && !constName) {
    char upperBuffer[256]="!";
    //int iCount = 0;
    pConstInfo = pClsInfo->pConstInfo;
    RxResult = RexxSend0(RexxSend0(RexxEnvironment,"STEM"),"NEW");

    while ( pConstInfo && RxResult != RexxNil ) {
      /* hide constants that start with _ (MS convention) */
      if (pConstInfo->pszConstName[0] != '_') {
        strcpy(upperBuffer+1,pConstInfo->pszConstName);
        strupr(upperBuffer);
        RexxSend2(RxResult,"[]=",Variant2Rexx(&pConstInfo->sValue),RexxString(upperBuffer));
      }
      pConstInfo = pConstInfo->pNext;
      //iCount++;
    }
    //sprintf(upperBuffer,"%d",iCount);
    //RexxSend2(RxResult,"[]=",RexxString(upperBuffer),RexxString("0"));
  }

  return RxResult;
}

/********************************************************/
/* following are functions that support GetKnownMethods */
/********************************************************/

/* Insert type information into a REXX array */
void InsertTypeInfo(ITypeInfo *pTypeInfo, TYPEATTR *pTypeAttr, RexxObject *RxResult, int *pIndex)
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
  for (iIndex=0;iIndex<pTypeAttr->cFuncs;iIndex++) {
    pFuncDesc = NULL;
    hResult = pTypeInfo->GetFuncDesc(iIndex,&pFuncDesc);
    if (hResult == S_OK) {

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
        RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));

        // store return type
        sprintf(szBuffer,"%s",pszDbgVarType(pFuncDesc->elemdescFunc.tdesc.vt));
        sprintf(szSmallBuffer,"%d.!RETTYPE",*pIndex);
        RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));

        // store invoke kind
        sprintf(szBuffer,"%d",pFuncDesc->invkind);
        sprintf(szSmallBuffer,"%d.!INVKIND",*pIndex);
        RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));

        if (bName)
        {
          // store name
          sprintf(szBuffer,"%S",bName);
          sprintf(szSmallBuffer,"%d.!NAME",*pIndex);
          RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));

        }
        else { // could not retrieve name of method (should never happen, really)
          sprintf(szBuffer,"???");
          sprintf(szSmallBuffer,"%d.!NAME",*pIndex);
          RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));
        }

        // store doc string
        if (bDocString) {
          sprintf(szBuffer,"%S",bDocString);
          sprintf(szSmallBuffer,"%d.!DOC",*pIndex);
          RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));
        }

        sprintf(szSmallBuffer,"%d.!PARAMS.0",*pIndex);
        sprintf(szBuffer,"%d",pFuncDesc->cParams);
        RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));
        for (i=0;i<pFuncDesc->cParams;i++) {

          szBuffer[0]=0x00;
          // display in/out parameters
          wFlags=pFuncDesc->lprgelemdescParam[i].paramdesc.wParamFlags;
          if (wFlags || i >= pFuncDesc->cParams - pFuncDesc->cParamsOpt) {
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
          RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));

          // display variant type
          sprintf(szBuffer,"%s",pszDbgVarType(pFuncDesc->lprgelemdescParam[i].tdesc.vt));
          sprintf(szSmallBuffer,"%d.!PARAMS.%d.!TYPE",*pIndex,i+1);
          RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));

          // display name
          if (i+1 < (int) uFlags) sprintf(szBuffer,"%S",pbStrings[i+1]);
          else sprintf(szBuffer,"<unnamed>");

          sprintf(szSmallBuffer,"%d.!PARAMS.%d.!NAME",*pIndex,i+1);
          RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));
        }

        SysFreeString(bName);
        SysFreeString(bDocString);
        // free the BSTRs
        for (i=0;i < (int) uFlags;i++)
          SysFreeString(pbStrings[i]);
        delete []pbStrings;

      }
    }

    if (pFuncDesc) pTypeInfo->ReleaseFuncDesc(pFuncDesc);
  }

/* this includes known variables
   since the method is called "getknownmethods" this does not really make sense
   will be left out for the time being...

  // run through all variables and get their description
  for (iIndex=0;iIndex<pTypeAttr->cVars;iIndex++) {
    pVarDesc = NULL;
    hResult = pTypeInfo->GetVarDesc(iIndex,&pVarDesc);

    if (hResult == S_OK) {
      // display only if this is not a restricted or hidden variable
      if (!(pVarDesc->wVarFlags & VARFLAG_FRESTRICTED) && !(pVarDesc->wVarFlags & VARFLAG_FHIDDEN)) {
        szBuffer[0] = szSmallBuffer[0] = 0x00;

        hResult = pTypeInfo->GetDocumentation(pVarDesc->memid,&bName,&bDocString,0,0);
        if (hResult == S_OK)
          sprintf(szBuffer,"%08x [variable] %s %S",pVarDesc->memid,pszDbgVarType(pVarDesc->elemdescVar.tdesc.vt),bName);
        else // could not retrieve name of variable (should never happen, really)
          sprintf(szBuffer,"???????? [variable] ???");

        // add doc string
        if (bDocString) {
          sprintf(szSmallBuffer," (%S)",bDocString);
          strcat(szBuffer,szSmallBuffer);
        }

        SysFreeString(bName);
        SysFreeString(bDocString);

        sprintf(szSmallBuffer,"%d",(*pIndex)++);
        RexxSend2(RxResult,"[]=",RexxString(szBuffer),RexxString(szSmallBuffer));
      }
    }

    if (pVarDesc) pTypeInfo->ReleaseVarDesc(pVarDesc);
  }
*/
}





//******************************************************************************
// Method:  OLEObject_GetKnownMethods
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     returnObject (REXXOBJECT) - A Rexx stem with the names of all known
//                                 methods or .Nil.
//
//   Notes:
//     This method will retrieve the needed information from the object's
//     type information
//
//******************************************************************************
RexxMethod1(REXXOBJECT,                // Return type
            OLEObject_GetKnownMethods, // Object_method name
            OSELF, self)               // Pointer to self
{
  RexxString      *RxString;
  RexxObject      *RxResult = RexxNil;
  IDispatch       *pDispatch = NULL;
  ITypeInfo       *pTypeInfo = NULL;
  ITypeLib        *pTypeLib = NULL;
  TYPEATTR        *pTypeAttr = NULL;
  UINT             iTypeInfoCount;
  CHAR            *pszRxString;
  HRESULT          hResult;
  INT              iCount = 0;
  UINT             iTypeIndex;
  CHAR             pszInfoBuffer[2048];

  if ( !fInitialized )
    OLEInit();

  /* get the IDispatch pointer for that object */
  RxString = (RexxString *)REXX_GETVAR("!IDISPATCH");
  pszRxString = string_data(RxString);
  if (sscanf(pszRxString, "%p", &pDispatch) != 1) send_exception(Error_Interpretation_initialization);


  if (pDispatch) {
    hResult = pDispatch->GetTypeInfoCount(&iTypeInfoCount);
    // check if type information is available
    if (iTypeInfoCount && SUCCEEDED(hResult)) {
      hResult = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);  // AddRef type info pointer
      // did we get a ITypeInfo interface pointer?
      if (pTypeInfo) {
        // create a Stem that will contain all info
        RxResult = RexxSend0(RexxSend0(RexxEnvironment,"STEM"),"NEW");

        // get type library
        hResult = pTypeInfo->GetContainingTypeLib(&pTypeLib,&iTypeIndex); // AddRef type lib pointer
        if (hResult == S_OK && pTypeLib) {
          BSTR         bName, bDoc;
          ITypeInfo   *pTypeInfo2 = NULL;

          // Get the library name and documentation
          hResult = pTypeLib->GetDocumentation(-1,&bName,&bDoc,NULL,NULL);
          if (bName) {
            sprintf(pszInfoBuffer,"%S",bName);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("!LIBNAME"));
          }
          if (bDoc) {
            sprintf(pszInfoBuffer,"%S",bDoc);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("!LIBDOC"));
          }
          SysFreeString(bName);
          SysFreeString(bDoc);

          // Now get the COM class name and documentation
          hResult = pTypeLib->GetDocumentation(iTypeIndex,&bName,&bDoc,NULL,NULL);
          if (bName) {
            sprintf(pszInfoBuffer,"%S",bName);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("!COCLASSNAME"));
          }
          if (bDoc) {
            sprintf(pszInfoBuffer,"%S",bDoc);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("!COCLASSDOC"));
          }

          hResult = pTypeLib->GetTypeInfo(iTypeIndex,&pTypeInfo2);   // AddRef type info pointer2
          if (pTypeInfo2) {
            hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);           // AddRef type attr pointer
            if (hResult == S_OK) {
              InsertTypeInfo(pTypeInfo2,pTypeAttr,RxResult,&iCount);
              pTypeInfo2->ReleaseTypeAttr(pTypeAttr);                // Release type attr pointer
            }
          }

          pTypeInfo2->Release();                                     // Release type info pointer2
          SysFreeString(bName);
          SysFreeString(bDoc);

          sprintf(pszInfoBuffer,"%d",iCount);
          RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("0"));

        }

        if (pTypeLib) pTypeLib->Release();                           // Release type lib pointer
        pTypeInfo->Release();                                        // Release type info pointer

      } /* end if (type info pointer) */
    } /* end if (type info available) */
  } /* endif (dispatch pointer) */

  return RxResult;
}

//******************************************************************************
// Method:  OLEObject_GetKnownMethods_Class
//
//   Arguments:
//     self - A pointer to self
//
//   Returned:
//     returnObject (REXXOBJECT) - A Rexx stem with the names of all known
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
RexxMethod2(REXXOBJECT,                      // Return type
            OLEObject_GetKnownMethods_Class, // Object_method name
            OSELF, self,                     // Pointer to self
            CSTRING, className)              // string defining class name
{
  RexxObject *RxResult = RexxNil;
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

  if (lpUniBuffer) {
    if ( *className == '{' )
    {
      /* argument is a CLSID */
      hResult = CLSIDFromString(lpUniBuffer, &clsID);
    }
    else
    {
      /* argument is a ProgID */
      hResult = CLSIDFromProgID(lpUniBuffer, &clsID);
    } /* endif */

    ORexxOleFree( lpUniBuffer );
    lpUniBuffer = NULL;

    /* successfully retrieved CLSID? */
    if (hResult == S_OK) {
      // create a unicode representation of CLSID
      hResult = StringFromCLSID(clsID, &lpOleStrBuffer);

      if (hResult == S_OK) {
        // create ansi representation of CLSID
        pszAnsiStr = pszUnicodeToAnsi(lpOleStrBuffer);

        if (pszAnsiStr)
        {
          // get the ProgID from registry
          sprintf(pBuffer,"CLSID\\%s\\ProgID",pszAnsiStr);
          if ( RegQueryValue(HKEY_CLASSES_ROOT,pBuffer,pBuffer,&lSize) == ERROR_SUCCESS) {

            for (i=lSize-1;i>=0;i--)
              if (pBuffer[i]=='.')
              {
                pBuffer[i]=0x00;
                break;
              }
            if (i>0)
              if (sscanf(pBuffer+i+1,"%d",&sMajor) != 1) sMajor = 1; // assume one if read in fails

          } else sMajor = 1;  // assume 1 if getting progid fails (should never happen)

          // get the TypeLib from registry
          sprintf(pBuffer,"CLSID\\%s\\TypeLib",pszAnsiStr);
          lSize=100;
          if ( RegQueryValue(HKEY_CLASSES_ROOT,pBuffer,pBuffer,&lSize) == ERROR_SUCCESS) {
            lpUniBuffer = lpAnsiToUnicode(pBuffer, strlen(pBuffer) + 1);
            hResult = CLSIDFromString(lpUniBuffer, &clsID);   // get CLSID of type lib
            ORexxOleFree( lpUniBuffer );
          }

          ORexxOleFree(pszAnsiStr);
        } /* endif */

        CoTaskMemFree(lpOleStrBuffer);
      }

      hResult = LoadRegTypeLib(clsID,sMajor,0,LOCALE_USER_DEFAULT,&pTypeLib);   // AddRef type lib pointer
      if (hResult == S_OK) {
        BSTR         bName, bDoc;
        ITypeInfo   *pTypeInfo2 = NULL;

        // create a Stem that will contain all info
        RxResult = RexxSend0(RexxSend0(RexxEnvironment,"STEM"),"NEW");

        hResult = pTypeLib->GetDocumentation(/*iTypeIndex*/-1,&bName,&bDoc,NULL,NULL);
        sprintf(pszInfoBuffer,"%S",bName);
        RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("!LIBNAME"));
        sprintf(pszInfoBuffer,"%S",bDoc);
        RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("!LIBDOC"));

        SysFreeString(bName);
        SysFreeString(bDoc);

        iTypeCount=pTypeLib->GetTypeInfoCount();

        for (iTypeIndex=0;iTypeIndex<iTypeCount;iTypeIndex++) {


          hResult = pTypeLib->GetDocumentation(iTypeIndex,&bName,&bDoc,NULL,NULL);
          sprintf(pszInfoBuffer,"%S",bName);
          sprintf(pszInfoBuffer+1024,"!LIBNAME.%d",iTypeIndex);
          RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszInfoBuffer+1024));
          sprintf(pszInfoBuffer,"%S",bDoc);
          sprintf(pszInfoBuffer+1024,"!LIBDOC.%d",iTypeIndex);
          RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszInfoBuffer+1024));

          SysFreeString(bName);
          SysFreeString(bDoc);


          hResult = pTypeLib->GetTypeInfo(iTypeIndex,&pTypeInfo2);   // AddRef type info pointer2
          if (pTypeInfo2) {
            hResult = pTypeInfo2->GetTypeAttr(&pTypeAttr);           // AddRef type attr pointer
            if (hResult == S_OK) {
              InsertTypeInfo(pTypeInfo2,pTypeAttr,RxResult,&iCount);
              pTypeInfo2->ReleaseTypeAttr(pTypeAttr);                // Release type attr pointer
            }
          }
          pTypeInfo2->Release();                                     // Release type info pointer2
        }

        sprintf(pszInfoBuffer,"%d",iCount);
        RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("0"));

      }
      if (pTypeLib) pTypeLib->Release();                           // Release type lib pointer
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
//     returnObject (REXXOBJECT) - A Rexx stem with the names of all known
//                                 events or .Nil.
//
//   Notes:
//     This method will retrieve the needed information from the object's
//     event list which was created in Init.
//
//******************************************************************************
RexxMethod1(REXXOBJECT,                // Return type
            OLEObject_GetKnownEvents,  // Object_method name
            OSELF, self)               // Pointer to self
{
  RexxString     *RxString;
  RexxObject     *RxResult = RexxNil;
  CHAR           *pszRxString;
//  HRESULT         hResult;
  INT             iCount = 0;
  INT             j;
  CHAR            pszInfoBuffer[2048];
  CHAR            pszSmall[128];
  OLEObjectEvent *pEventHandler = NULL;
  POLEFUNCINFO2   pEventList = NULL;
  unsigned short  wFlags = 0;

  if ( !fInitialized )
    OLEInit();

  /* get the event object that contains the list */
  RxString = (RexxString *)REXX_GETVAR("!EVENTHANDLER");
  pszRxString = string_data(RxString);

  // got a pointer?
  if (*pszRxString != '!') {
    if (sscanf(pszRxString, "%p", &pEventHandler) != 1) send_exception(Error_Interpretation_initialization);

    if (pEventHandler) {
      pEventList = pEventHandler->getEventList();
      if (pEventList) {
        // create a Stem that will contain all info
        RxResult = RexxSend0(RexxSend0(RexxEnvironment,"STEM"),"NEW");

        while (pEventList) {
          iCount++;

          sprintf(pszInfoBuffer,"%s",pEventList->pszFuncName);
          sprintf(pszSmall,"%d.!NAME",iCount);
          RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszSmall));

          sprintf(pszInfoBuffer,"%s",pEventList->pszDocString);
          sprintf(pszSmall,"%d.!DOC",iCount);
          RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszSmall));

          sprintf(pszInfoBuffer,"%d",pEventList->iParmCount);
          sprintf(pszSmall,"%d.!PARAMS.0",iCount);
          RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszSmall));

          for (j=0;j<pEventList->iParmCount;j++) {
            sprintf(pszInfoBuffer,"%s",pEventList->pszName[j]);
            sprintf(pszSmall,"%d.!PARAMS.%d.!NAME",iCount,j+1);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszSmall));

            sprintf(pszInfoBuffer,"%s",pszDbgVarType(pEventList->pOptVt[j]));
            sprintf(pszSmall,"%d.!PARAMS.%d.!TYPE",iCount,j+1);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszSmall));

            wFlags=pEventList->pusOptFlags[j];
            pszInfoBuffer[0] = 0x00;
            if (wFlags || j >= pEventList->iParmCount - pEventList->iOptParms ) {
              strcat(pszInfoBuffer,"[");
              if (wFlags & PARAMFLAG_FIN)
                strcat(pszInfoBuffer,"in,");
              if (wFlags & PARAMFLAG_FOUT)
                strcat(pszInfoBuffer,"out,");
              if (j >= pEventList->iParmCount - pEventList->iOptParms)
                strcat(pszInfoBuffer,"opt,");
            }
            pszInfoBuffer[strlen(pszInfoBuffer)-1]=0x00; // remove last comma
            strcat(pszInfoBuffer,"]");

            sprintf(pszSmall,"%d.!PARAMS.%d.!FLAGS",iCount,j+1);
            RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString(pszSmall));
          }

          pEventList = pEventList->pNext;
        }

        sprintf(pszInfoBuffer,"%d",iCount);
        RexxSend2(RxResult,"[]=",RexxString(pszInfoBuffer),RexxString("0"));
      }
    }
  }

  return RxResult;
}


/**** Moniker to Object binding ****/

//******************************************************************************
// Method:  OLEObject_GetObject_Class
//
//   Arguments:
//     self - A pointer to self
//     monikerName (REXXOBJECT) - The moniker name to get an object of
//     optClass (REXXOBJECT)    - The class from which to create the object (optional, must be derived from OLEObject)
//
//   Returned:
//     returnObject (REXXOBJECT) - an REXX OLE instance or .Nil.
////
//******************************************************************************
RexxMethod3(REXXOBJECT,                // Return type
            OLEObject_GetObject_Class, // Object_method name
            OSELF, self,               // Pointer to self
            REXXOBJECT, monikerName,   // Class specifier for new object
            REXXOBJECT, optClass)      // an optional class that is to be used when created
{
  RexxObject  *ResultObj = RexxNil;
  RexxString  *argString;
  HRESULT      hResult;
  LPOLESTR     lpUniBuffer = NULL;
  IBindCtx    *pBC = NULL;
  IDispatch   *pDispatch = NULL;
  RexxObject  *OLEObjectClass = NULL;
  CHAR         szBuffer[2048];
  int          rc = 0;
  long         iLast = iInstanceCount;

  if (monikerName == NULL)
    send_exception1(Error_Incorrect_method_noarg, RexxArray1(RexxString("1")));

  argString = (RexxString *) RexxSend1(monikerName, "REQUEST", RexxString("STRING"));

  if ( !_isstring(argString) )
    send_exception1(Error_Incorrect_method_string, RexxArray1(RexxString("1")));

  OLEObjectClass = RexxSend0(RexxEnvironment, "OLEOBJECT");

  // if a class argument has been supplied, make sure that it is a class derived
  // from OLEObject. if not, return a nil object!
  if (optClass) {
    RexxArray  *superClasses = NULL;
    RexxString *arSize;
    int i;
    BOOL fFound = false;

    superClasses = (RexxArray*) RexxSend0(optClass, "SUPERCLASSES");
    if (superClasses != RexxNil) {
      arSize = (RexxString*) RexxSend0(RexxSend0(superClasses,"SIZE"),"STRING");
      sscanf(string_data(arSize),"%d",&i);
      while (i>=1)
        // one of the superclasses is OLEObjectClass?
        if (array_at(superClasses,i) == OLEObjectClass) {
          // ok to create an object of the derived class...
          OLEObjectClass = optClass;
          fFound = true;
          break;
        }
        else i--;

      if (!fFound) return ResultObj;
    }
  }

  // if no OLE objects exist now, we must call OleInitialize()
  if (iInstanceCount == 0) {
    OleInitialize(NULL);
  }
  iInstanceCount++;

  lpUniBuffer = lpAnsiToUnicode( string_data(argString), string_length(argString) + 1);

  if (lpUniBuffer) {
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
          if (OLEObjectClass == optClass)
            ResultObj = RexxSend2(optClass, "NEW", RexxString(szBuffer), RexxString("WITHEVENTS"));
          else
            ResultObj = RexxSend1(OLEObjectClass, "NEW", RexxString(szBuffer));

          // ~new has called AddRef for the object, so we must Release it here once
          rc = pDispatch->Release();
        }
        rc = pMoniker->Release();
      }
//      else
//        if (hResult == MK_E_SYNTAX) fprintf(stderr,"MK_E_SYNTAX parsing moniker\n");

      rc = pBC->Release();
    }
    ORexxOleFree(lpUniBuffer);
  }

  // if creation failed, we must very likely shut down OLE
  // by calling OleUninitialize()
  if (iLast < iInstanceCount) {
    iInstanceCount--;
    if (iInstanceCount == 0) {
      OleUninitialize();
    }
  }

  return ResultObj;
}

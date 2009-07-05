/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

#include <windows.h>
#include "oorexxapi.h"
#include <stdio.h>
#include <string.h>
#include <ddeml.h>
#include <time.h>
#include <shlobj.h>
#include <shlwapi.h>

#define STR_BUFFER    256
#define MAX_TIME_DATE 128

// The OS specifies a maximum size for a registry key name as 255.
#define MAX_REGISTRY_KEY_SIZE 256

#define MSG_TIMEOUT  5000 // 5000ms
#if (WINVER >= 0x0500)
#define MSG_TIMEOUT_OPTS (SMTO_ABORTIFHUNG|SMTO_NORMAL|SMTO_NOTIMEOUTIFNOTHUNG)
#else
#define MSG_TIMEOUT_OPTS (SMTO_ABORTIFHUNG|SMTO_NORMAL)
#endif


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   memupper                                     */
/*                                                                   */
/*   Descriptive Name:  uppercase a memory location                  */
/*                                                                   */
/*   Entry Point:       memupper                                     */
/*                                                                   */
/*   Input:             memory to upper case                         */
/*                      length of memory location                    */
/*                                                                   */
/*********************************************************************/

void  memupper(
  char    *location,                   /* location to uppercase      */
  size_t   length)                     /* length to uppercase        */
{
  for (; length--; location++)         /* loop for entire string     */
                                       /* uppercase in place         */
    *location = toupper(*location);
}


/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

VOID Little2BigEndian(BYTE *pbInt, INT iSize);

size_t dwordPtrToRexx(DWORD_PTR val, PRXSTRING r)
{
    _snprintf(r->strptr, RXAUTOBUFLEN, "%Iu", val);
    r->strlength = strlen(r->strptr);
    return 0;
}

LONG HandleArgError(PRXSTRING r, BOOL ToMuch)
{
      r->strlength = 2;
      r->strptr[0] = '4';
      r->strptr[1] = '0';
      r->strptr[2] = '\0';
      return 40;
}

#define CHECKARG(argexpctl, argexpcth) \
   if ((argc < argexpctl) || (argc > argexpcth)) return HandleArgError(retstr, (argc > argexpcth))


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
                   itoa(retvalue, retstr->strptr, 10); \
                   retstr->strlength = strlen(retstr->strptr);\
                   return 0; \
                }


#define RET_HANDLE(retvalue)  { \
                   pointer2string(retstr, retvalue); \
                   return 0; \
                }


#define ISHEX(value) \
   ((strlen(value) > 2) && (value[0] == '0') && (toupper(value[1]) == 'X'))


/* Note many existing programs abbreviate HKEY_LOCAL_MACHINE to "LOCAL_MACHINE",
 * or "MACHINE", and many do not.  Many existing programs use the full
 * HKEY_LOCAL_MACHINE.  So the comparison needs to remain strstr.
 */
#define GET_HKEY(argum, ghk) { \
     ghk = NULL; \
     if (strstr(argum,"MACHINE")) ghk = HKEY_LOCAL_MACHINE; else \
     if (strstr(argum,"CLASSES")) ghk = HKEY_CLASSES_ROOT; else \
     if (strstr(argum,"CURRENT_USER")) ghk = HKEY_CURRENT_USER; else \
     if (strstr(argum,"USERS")) ghk = HKEY_USERS; else \
     if (strstr(argum,"PERFORMANCE")) ghk = HKEY_PERFORMANCE_DATA; else \
     if (strstr(argum,"CURRENT_CONFIG")) ghk = HKEY_CURRENT_CONFIG; else \
     if (strstr(argum,"DYN_DATA")) ghk = HKEY_DYN_DATA; else \
     string2pointer(argum, (void **)&ghk); \
}


#define GET_HANDLE(argum, ghk) string2pointer(argum, (void **)&(ghk))


#define SET_VARIABLE(varname, data, retc) {\
             shvb.shvnext = NULL; \
             shvb.shvname.strptr = varname; \
             shvb.shvname.strlength = strlen(varname); \
             shvb.shvnamelen = shvb.shvname.strlength; \
             shvb.shvvalue.strptr = data; \
             shvb.shvvalue.strlength = strlen(data); \
             shvb.shvvaluelen = strlen(data); \
             shvb.shvcode = RXSHV_SYSET; \
             shvb.shvret = 0; \
             if (RexxVariablePool(&shvb) == RXSHV_BADN) RETC(retc); \
        }


#define GET_ACCESS(argu, acc) \
{ \
      if (!stricmp(argu,"READ"))    acc = 'R'; else \
      if (!stricmp(argu,"OPEN"))    acc = 'O'; else \
      if (!stricmp(argu,"CLOSE"))   acc = 'C'; else  \
      if (!stricmp(argu,"NUM"))     acc = 'N'; else  \
      if (!stricmp(argu,"CLEAR"))   acc = 'L'; else  \
      if (!stricmp(argu,"WRITE"))   acc = 'W'; else  \
         RETERR; \
}

#define GET_TYPE_INDEX(type, index)   \
{                                     \
    switch (type)                     \
    {                                 \
      case EVENTLOG_ERROR_TYPE:       \
         index=0;                     \
          break;                      \
      case EVENTLOG_WARNING_TYPE:     \
         index=1;                     \
         break;                       \
      case EVENTLOG_INFORMATION_TYPE: \
         index=2;                     \
         break;                       \
      case EVENTLOG_SUCCESS:          \
         index=2;                     \
         break;                       \
      case EVENTLOG_AUDIT_SUCCESS:    \
         index=3;                     \
         break;                       \
      case EVENTLOG_AUDIT_FAILURE:    \
         index=4;                     \
         break;                       \
      default:                        \
        index=5;                      \
    }                                 \
}


/********************************************************************
* Function:  string2pointer(string)                                 *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to a pointer value.  Returns false if the number  *
*            is not valid, true if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        true - Good number converted                           *
*            false - Invalid number supplied.                       *
*********************************************************************/

BOOL string2pointer(
  const char *string,                  /* string to convert          */
  void **pointer)                      /* converted number           */
{
    if ( strlen(string) == 0 )
    {
        *pointer = NULL;
        return FALSE;
    }

    if ( ISHEX(string) )
    {
        return (string[1] == 'x' ?
                sscanf(string, "0x%p", pointer) == 1 : sscanf(string, "0X%p", pointer) == 1);
    }

    return sscanf(string, "%p", pointer) == 1;
}


void pointer2string(PRXSTRING result, void *pointer)
{
    if ( pointer == NULL )
    {
        result->strlength = 1;
        result->strptr[0] = '0';
        result->strptr[1] = '\0';
    }
    else
    {
        sprintf(result->strptr, "0x%p", pointer);
        result->strlength = strlen(result->strptr);
    }
}


BOOL IsRunningNT()
{
    OSVERSIONINFO version_info={0};

    version_info.dwOSVersionInfoSize = sizeof(version_info);
    GetVersionEx(&version_info);
    if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT) return TRUE; // Windows NT
    else return FALSE;                                              // Windows 95
}


// TODO This is a function from oodCommon.cpp, need to put all this stuff together.
POINTER rxGetPointerAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name)
{
    CSTRING value = "";
    if ( obj != NULLOBJECT )
    {
        RexxObjectPtr rxString = context->SendMessage0(obj, name);
        if ( rxString != NULLOBJECT )
        {
            value = context->ObjectToStringValue(rxString);
        }
    }
    POINTER p = NULL;
    string2pointer(value, &p);
    return p;
}


/** WindowsRegistry::delete() | WindowsRegistry::deleteKey()
 *
 *  Deletes a registry key.  Maps to both the delete() and the deleteKey()
 *  methods.
 *
 *  delete() deletes a subkey and all its descendents (subkeys.)  deleteKey()
 *  will only delete the subkey if it is empty, i.e. it contains no subkeys.
 *
 *  @param hkHandle    [optional] A handle to an open registry key. The key must
 *                     have been opened with the DELETE access right.  If this
 *                     argument is omitted then the CURRENT_KEY attribute is
 *                     used.
 *
 *  @param subkeyName  The name of the subkey to be deleted.  The name is case
 *                     insensitive.
 *
 *  @return O on success, otherwise the Windows system error code.
 */
RexxMethod3(uint32_t, WSRegistry_delete, OPTIONAL_POINTERSTRING, hkHandle, CSTRING, subKeyName, OSELF, self)
{
    HKEY hk = (HKEY)(argumentExists(1) ? hkHandle : rxGetPointerAttribute(context, self, "CURRENT_KEY"));

    if ( strcmp(context->GetMessageName(), "DELETEKEY") == 0 )
    {
        return RegDeleteKey(hk, subKeyName);
    }
    else
    {
        return SHDeleteKey(hk, subKeyName);
    }
}

size_t RexxEntry WSRegistryKey(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HKEY hk;

    CHECKARG(2,5);

    if ( strcmp(argv[0].strptr, "CREATE") == 0 )
    {
        HKEY hkResult;

        GET_HANDLE(argv[1].strptr, hk);
        if (RegCreateKey(hk, argv[2].strptr, &hkResult ) == ERROR_SUCCESS)
        {
            RET_HANDLE(hkResult);
        }
        else
        {
            RETC(0);
        }
    }
    else if ( strcmp(argv[0].strptr, "OPEN") == 0 )
    {
        HKEY hkResult;
        DWORD access=0;

        GET_HKEY(argv[1].strptr, hk);

        if (argc == 2)
        {
            RET_HANDLE(hk);      // return the predefined handle
        }

        // Docs say, have always said, that the access arg can be more than one
        // keyword. So, even if "ALL" makes the other keywords unnecessary, we
        // can't rely on it being the only word in the string.
        if ((argc < 4) || strstr(argv[3].strptr,"ALL") != 0)
        {
            access = KEY_ALL_ACCESS;
        }
        else
        {
            if (strstr(argv[3].strptr,"WRITE")) access |= KEY_WRITE;
            if (strstr(argv[3].strptr,"READ")) access |= KEY_READ;
            if (strstr(argv[3].strptr,"QUERY")) access |= KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE;
            if (strstr(argv[3].strptr,"EXECUTE")) access |= KEY_EXECUTE;
            if (strstr(argv[3].strptr,"NOTIFY")) access |= KEY_NOTIFY;
            if (strstr(argv[3].strptr,"LINK")) access |= KEY_CREATE_LINK;
        }

        if (RegOpenKeyEx(hk, argv[2].strptr, 0, access, &hkResult ) == ERROR_SUCCESS)
        {
            RET_HANDLE(hkResult);
        }
        else
        {
            RETC(0);
        }
    }
    else if ( strcmp(argv[0].strptr, "CLOSE") == 0 )
    {
        GET_HANDLE(argv[1].strptr, hk);
        if (RegCloseKey(hk) == ERROR_SUCCESS)
        {
            RETC(0);
        }
        else
        {
            RETC(1);
        }
    }
    else if ( strcmp(argv[0].strptr, "QUERY") == 0 )
    {
        char Class[256];
        DWORD retcode, cbClass, cSubKeys, cbMaxSubKeyLen,
        cbMaxClassLen, cValues, cbMaxValueNameLen, cbMaxValueLen, cbSecurityDescriptor;
        FILETIME ftLastWriteTime;
        SYSTEMTIME stTime;

        cbClass = 256;

        GET_HANDLE(argv[1].strptr, hk);

        if ((retcode=RegQueryInfoKey(hk,                             // handle of key to query
             Class,    // address of buffer for class string
             &cbClass,    // address of size of class string buffer
             NULL,    // reserved
             &cSubKeys,    // address of buffer for number of subkeys
             &cbMaxSubKeyLen,    // address of buffer for longest subkey name length
             &cbMaxClassLen,    // address of buffer for longest class string length
             &cValues,    // address of buffer for number of value entries
             &cbMaxValueNameLen,    // address of buffer for longest value name length
             &cbMaxValueLen,    // address of buffer for longest value data length
             &cbSecurityDescriptor,    // address of buffer for security descriptor length
             &ftLastWriteTime     // address of buffer for last write time
            )) == ERROR_SUCCESS)
            {
                if (FileTimeToSystemTime(&ftLastWriteTime, &stTime))
                {

                    sprintf(retstr->strptr,"%s, %ld, %ld, %04d/%02d/%02d, %02d:%02d:%02d",
                            Class, cSubKeys, cValues, stTime.wYear, stTime.wMonth, stTime.wDay,
                            stTime.wHour, stTime.wMinute, stTime.wSecond);
                }
                else
                {
                    sprintf(retstr->strptr,"%s, %ld, %ld",Class, cSubKeys, cValues);
                }

                retstr->strlength = strlen(retstr->strptr);
                return 0;
            }
        else
        {
            RETC(0);
        }
    }
    else if ( strcmp(argv[0].strptr, "LIST") == 0 )
    {
        DWORD retcode, ndx=0;
        char Name[256];
        char sname[64];
        SHVBLOCK shvb;

        GET_HKEY(argv[1].strptr, hk);
        do
        {
            retcode = RegEnumKey(hk,    // handle of key to query
                                 ndx++,    // index of subkey to query
                                 Name,    // address of buffer for subkey name
                                 sizeof(Name));     // size of subkey buffer
            if (retcode == ERROR_SUCCESS)
            {
                strcpy(sname, argv[2].strptr);
                // make sure there is a period on the stem name
                if (sname[argv[2].strlength - 1] != '.')
                {
                    strcat(sname, ".");
                }
                sprintf(sname + strlen(sname),"%d", ndx);
                SET_VARIABLE(sname, Name, 2);
            }
            else if (retcode != ERROR_NO_MORE_ITEMS)
            {
                RETC(1);
            }
        } while (retcode == ERROR_SUCCESS);
        RETC(0);
    }
    else if ( strcmp(argv[0].strptr, "FLUSH") == 0 )
    {
        GET_HKEY(argv[1].strptr, hk);

        if (RegFlushKey(hk) == ERROR_SUCCESS)
        {
            RETC(0);
        }
        else
        {
            RETC(1);
        }
    }
    else
    {
        RETC(1);
    }
}


size_t RexxEntry WSRegistryValue(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HKEY hk;
    LONG rc;

    CHECKARG(2,5);

    if ( strcmp(argv[0].strptr, "SET") == 0 )
    {
        DWORD valType;
        DWORD dwNumber;
        DWORD dataLen;
        const BYTE * data;

        GET_HKEY(argv[1].strptr, hk);

        if (!strcmp(argv[4].strptr,"EXPAND")) valType = REG_EXPAND_SZ;
        else
            if (!strcmp(argv[4].strptr,"MULTI")) valType = REG_MULTI_SZ;
        else
            if (!strcmp(argv[4].strptr,"NUMBER")) valType = REG_DWORD;
        else
            if (!strcmp(argv[4].strptr,"BINARY")) valType = REG_BINARY;
        else
            if (!strcmp(argv[4].strptr,"LINK")) valType = REG_LINK;
        else
            if (!strcmp(argv[4].strptr,"RESOURCELIST")) valType = REG_RESOURCE_LIST;
        else
            if (!strcmp(argv[4].strptr,"RESOURCEDESC")) valType = REG_FULL_RESOURCE_DESCRIPTOR;
        else
            if (!strcmp(argv[4].strptr,"RESOURCEREQS")) valType = REG_RESOURCE_REQUIREMENTS_LIST;
        else
            if (!strcmp(argv[4].strptr,"BIGENDIAN")) valType = REG_DWORD_BIG_ENDIAN;
        else
            if (!strcmp(argv[4].strptr,"NONE")) valType = REG_NONE;
        else
            valType = REG_SZ;

        if ((valType == REG_DWORD) || (valType == REG_DWORD_BIG_ENDIAN))
        {
            dwNumber = atoi(argv[3].strptr);

            if (valType == REG_DWORD_BIG_ENDIAN)
            {
                Little2BigEndian((BYTE *) &dwNumber, sizeof(dwNumber));
            }

            data = (const BYTE *) &dwNumber;
            dataLen = sizeof(dwNumber);
        }
        else
        {
            data = (const BYTE *) argv[3].strptr;
            switch (valType)
            {
                case REG_BINARY:
                case REG_NONE:
                case REG_LINK:
                case REG_RESOURCE_LIST:
                case REG_FULL_RESOURCE_DESCRIPTOR:
                case REG_RESOURCE_REQUIREMENTS_LIST:
                    dataLen = (DWORD)argv[3].strlength;
                    break;

                case REG_EXPAND_SZ:
                case REG_MULTI_SZ:
                case REG_SZ:
                    dataLen = (DWORD)argv[3].strlength+1;
                    break;
            }
        }

        if (RegSetValueEx(hk, argv[2].strptr, 0, valType, data, dataLen) == ERROR_SUCCESS)
        {
            RETC(0);
        }
        else
        {
            RETC(1);
        }

    }
    else if ( strcmp(argv[0].strptr, "QUERY") == 0 )
    {
        DWORD valType, cbData;
        char * valData, *vType;

        cbData = sizeof(valData);

        GET_HKEY(argv[1].strptr, hk);

        if (RegQueryValueEx(hk,        // handle of key to query
                            argv[2].strptr,    // address of name of value to query
                            NULL,        // reserved
                            &valType,    // address of buffer for value type
                            NULL,        // NULL to get the size
                            &cbData) == ERROR_SUCCESS) // address of data buffer size
        {
            valData = (char *)GlobalAlloc(GPTR, cbData);

            if (!valData)
            {
                RETERR;
            }

            if (RegQueryValueEx(hk,    // handle of key to query
                                argv[2].strptr,    // address of name of value to query
                                NULL,    // reserved
                                &valType,    // address of buffer for value type
                                (LPBYTE)valData,    // address of data buffer
                                &cbData) == ERROR_SUCCESS) // address of data buffer size
            {
                // If the size of the value data is larger than the default
                // return string buffer, we need to allocate a bigger buffer.
                if ( cbData + sizeof("RESOURCEDESC, ") > STR_BUFFER )
                {
                    retstr->strptr = (char *)RexxAllocateMemory(cbData + sizeof("RESOURCEDESC, "));
                    if ( retstr->strptr == NULL )
                    {
                        RETERR;
                    }
                }

                switch (valType)
                {
                    case REG_MULTI_SZ:
                        vType = "MULTI";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_DWORD:
                        vType = "NUMBER";
                        sprintf(retstr->strptr,"%s, %ld",vType, *(DWORD *)valData);
                        break;
                    case REG_BINARY:
                        vType = "BINARY";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_NONE:
                        vType = "NONE";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_SZ:
                        vType = "NORMAL";
                        sprintf(retstr->strptr,"%s, %s",vType, valData);
                        break;
                    case REG_EXPAND_SZ:
                        vType = "EXPAND";
                        sprintf(retstr->strptr,"%s, %s",vType, valData);
                        break;
                    case REG_RESOURCE_LIST:
                        vType = "RESOURCELIST";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_FULL_RESOURCE_DESCRIPTOR:
                        vType = "RESOURCEDESC";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_RESOURCE_REQUIREMENTS_LIST:
                        vType = "RESOURCEREQS";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_LINK:
                        vType = "LINK";
                        sprintf(retstr->strptr,"%s, ",vType);
                        memcpy(&retstr->strptr[strlen(retstr->strptr)], valData, cbData);
                        break;
                    case REG_DWORD_BIG_ENDIAN:
                        {
                            DWORD dwNumber;
                            vType = "BIGENDIAN";
                            dwNumber = * (DWORD *)valData;
                            Little2BigEndian((BYTE *) &dwNumber, sizeof(dwNumber));
                            sprintf(retstr->strptr,"%s, %ld",vType, dwNumber);
                        }
                        break;

                    default:
                        vType = "OTHER";
                        sprintf(retstr->strptr,"%s,",vType);
                }
                if ((valType == REG_MULTI_SZ) ||
                    (valType == REG_BINARY) ||
                    (valType == REG_RESOURCE_LIST) ||
                    (valType == REG_FULL_RESOURCE_DESCRIPTOR) ||
                    (valType == REG_RESOURCE_REQUIREMENTS_LIST) ||
                    (valType == REG_NONE))
                {
                    retstr->strlength = strlen(vType) + 2 + cbData;
                }
                else
                {
                    retstr->strlength = strlen(retstr->strptr);
                }

                GlobalFree(valData);
                return 0;
            }

            GlobalFree(valData);
            RETC(0);
        }
        else
        {
            RETC(0);
        }
    }
    else if ( strcmp(argv[0].strptr, "LIST") == 0 )
    {
        DWORD retcode, ndx=0, valType, cbValue, cbData, initData = 1024;
        char * valData, Name[256];
        char sname[300];
        SHVBLOCK shvb;

        GET_HKEY(argv[1].strptr, hk);
        valData = (char *)GlobalAlloc(GPTR, initData);
        if (!valData) RETERR

            do
            {
                cbData = initData;
                cbValue = sizeof(Name);
                retcode = RegEnumValue(hk,    // handle of key to query
                                       ndx++,    // index of subkey to query
                                       Name,    // address of buffer for subkey name
                                       &cbValue,
                                       NULL,    // reserved
                                       &valType,    // address of buffer for type code
                                       (LPBYTE)valData,    // address of buffer for value data
                                       &cbData);     // address for size of data buffer

                if ((retcode == ERROR_MORE_DATA) && (cbData > initData))   /* we need more memory */
                {
                    GlobalFree(valData);
                    initData = cbData;
                    valData = (char *)GlobalAlloc(GPTR, cbData);
                    if (!valData) RETERR
                        ndx--;                      /* try to get the previous one again */
                    cbValue = sizeof(Name);
                    retcode = RegEnumValue(hk,    // handle of key to query
                                           ndx++,    // index of subkey to query
                                           Name,    // address of buffer for subkey name
                                           &cbValue,
                                           NULL,    // reserved
                                           &valType,    // address of buffer for type code
                                           (LPBYTE)valData,    // address of buffer for value data
                                           &cbData);     // address for size of data buffer
                }

                if (retcode == ERROR_SUCCESS)
                {
                    strcpy(sname, argv[2].strptr);
                    // make sure there is a period on the stem name
                    if (sname[argv[2].strlength - 1] != '.')
                    {
                        strcat(sname, ".");
                    }
                    sprintf(sname + strlen(sname),"%d", ndx);
                    SET_VARIABLE(sname, Name, 2);
                    strcpy(sname, argv[2].strptr);
                    // make sure there is a period on the stem name
                    if (sname[argv[2].strlength - 1] != '.')
                    {
                        strcat(sname, ".");
                    }
                    sprintf(sname + strlen(sname),"%d.Type", ndx);
                    switch (valType)
                    {
                        case REG_EXPAND_SZ:
                            SET_VARIABLE(sname, "EXPAND", 2);
                            break;
                        case REG_NONE:
                            SET_VARIABLE(sname, "NONE", 2);
                            break;
                        case REG_DWORD:
                            SET_VARIABLE(sname, "NUMBER", 2);
                            break;
                        case REG_MULTI_SZ:
                            SET_VARIABLE(sname, "MULTI", 2);
                            break;
                        case REG_BINARY:
                            SET_VARIABLE(sname, "BINARY", 2);
                            break;
                        case REG_SZ:
                            SET_VARIABLE(sname, "NORMAL", 2);
                            break;
                        case REG_RESOURCE_LIST:
                            SET_VARIABLE(sname, "RESOURCELIST", 2);
                            break;
                        case REG_FULL_RESOURCE_DESCRIPTOR:
                            SET_VARIABLE(sname, "RESOURCEDESC", 2);
                            break;
                        case REG_RESOURCE_REQUIREMENTS_LIST:
                            SET_VARIABLE(sname, "RESOURCEREQS", 2);
                            break;
                        case REG_LINK:
                            SET_VARIABLE(sname, "LINK", 2);
                            break;
                        case REG_DWORD_BIG_ENDIAN:
                            SET_VARIABLE(sname, "BIGENDIAN", 2);
                            break;
                        default:
                            SET_VARIABLE(sname, "OTHER", 2);
                    }
                    strcpy(sname, argv[2].strptr);
                    // make sure there is a period on the stem name
                    if (sname[argv[2].strlength - 1] != '.')
                    {
                        strcat(sname, ".");
                    }
                    sprintf(sname + strlen(sname),"%d.Data", ndx);
                    if ((valType == REG_MULTI_SZ) ||
                        (valType == REG_BINARY) ||
                        (valType == REG_LINK) ||
                        (valType == REG_RESOURCE_LIST) ||
                        (valType == REG_FULL_RESOURCE_DESCRIPTOR) ||
                        (valType == REG_RESOURCE_REQUIREMENTS_LIST) ||
                        (valType == REG_NONE))
                    {
                        shvb.shvnext = NULL;
                        shvb.shvname.strptr = sname;
                        shvb.shvname.strlength = strlen(sname);
                        shvb.shvnamelen = shvb.shvname.strlength;
                        shvb.shvvalue.strptr = valData;
                        shvb.shvvalue.strlength = cbData;
                        shvb.shvvaluelen = cbData;
                        shvb.shvcode = RXSHV_SYSET;
                        shvb.shvret = 0;
                        if (RexxVariablePool(&shvb) == RXSHV_BADN)
                        {
                            GlobalFree(valData);
                            RETC(2);
                        }
                    }
                    else if ((valType == REG_EXPAND_SZ) || (valType == REG_SZ))
                    {
                        SET_VARIABLE(sname, valData, 2);
                    }
                    else if ((valType == REG_DWORD) || (valType == REG_DWORD_BIG_ENDIAN))
                    {
                        char tmp[30];
                        DWORD dwNumber;

                        dwNumber = *(DWORD *) valData;
                        if (valType == REG_DWORD_BIG_ENDIAN)
                        {
                            Little2BigEndian((BYTE *)&dwNumber, sizeof(dwNumber));
                        }
                        ltoa(dwNumber, tmp, 10);
                        SET_VARIABLE(sname, tmp, 2);
                    }
                    else
                    {
                        SET_VARIABLE(sname, "", 2);
                    }
                }
                else if (retcode != ERROR_NO_MORE_ITEMS)
                {
                    GlobalFree(valData);
                    RETC(1);
                }
            } while (retcode == ERROR_SUCCESS);
        GlobalFree(valData);
        RETC(0);
    }
    else if ( strcmp(argv[0].strptr, "DELETE") == 0 )
    {
        GET_HKEY(argv[1].strptr, hk);

        if ((rc = RegDeleteValue(hk, argv[2].strptr)) == ERROR_SUCCESS)
        {
            RETC(0);
        }
        else
        {
            RETVAL(rc);
        }
    }
    else
    {
        RETC(1);
    }
}


size_t RexxEntry WSRegistryFile(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    DWORD retc, rc;
    HKEY hk;
    HANDLE hToken;              /* handle to process token */
    TOKEN_PRIVILEGES tkp;        /* ptr. to token structure */

    CHECKARG(2,5);

    if ( strcmp(argv[0].strptr, "CONNECT") == 0 )
    {
        HKEY hkResult;
        GET_HKEY(argv[1].strptr, hk);

        if (RegConnectRegistry(argv[2].strptr, hk, &hkResult ) == ERROR_SUCCESS)
        {
            RET_HANDLE(hkResult);
        }
        else
        {
            RETC(0);
        }
    }
    else if ( strcmp(argv[0].strptr, "SAVE") == 0 )
    {
        /* set SE_BACKUP_NAME privilege.  */

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            RETVAL(GetLastError());
        }

        LookupPrivilegeValue(NULL, SE_BACKUP_NAME,&tkp.Privileges[0].Luid);

        tkp.PrivilegeCount = 1;  /* one privilege to set    */
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        /* Set SE_BACKUP_NAME privilege for this process. */

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                              (PTOKEN_PRIVILEGES) NULL, 0);

        if ((rc = GetLastError()) != ERROR_SUCCESS)
        {
            RETVAL(rc);
        }

        GET_HKEY(argv[1].strptr, hk);
        if ((retc = RegSaveKey(hk, argv[2].strptr, NULL)) == ERROR_SUCCESS)
        {
            RETC(0);
        }
        else
        {
            RETVAL(retc);
        }
    }
    else if ( strcmp(argv[0].strptr, "LOAD") == 0 || strcmp(argv[0].strptr, "RESTORE") == 0 ||
              strcmp(argv[0].strptr, "REPLACE") == 0 || strcmp(argv[0].strptr, "UNLOAD") == 0 )
    {
        /* set SE_RESTORE_NAME privilege.  */

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            RETVAL(GetLastError())
        }

        LookupPrivilegeValue(NULL, SE_RESTORE_NAME,&tkp.Privileges[0].Luid);

        tkp.PrivilegeCount = 1;  /* one privilege to set    */
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        /* Set SE_BACKUP_NAME privilege for this process. */

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

        if ((rc = GetLastError()) != ERROR_SUCCESS)
        {
            RETVAL(rc);
        }

        if ( strcmp(argv[0].strptr, "UNLOAD") == 0 )
        {
            GET_HKEY(argv[1].strptr, hk);
            if ((retc = RegUnLoadKey(hk, argv[2].strptr)) == ERROR_SUCCESS)
            {
                RETC(0);
            }
            else
            {
                RETVAL(retc);
            }
        }
        else if ( strcmp(argv[0].strptr, "LOAD") == 0 )
        {
            GET_HKEY(argv[1].strptr, hk);
            if ((retc = RegLoadKey(hk, argv[2].strptr, argv[3].strptr)) == ERROR_SUCCESS)
            {
                RETC(0);
            }
            else
            {
                RETVAL(retc);
            }
        }
        else if ( strcmp(argv[0].strptr, "RESTORE") == 0 )
        {
            DWORD vola;

            GET_HKEY(argv[1].strptr, hk);
            if (!strcmp(argv[3].strptr, "VOLATILE")) vola = REG_WHOLE_HIVE_VOLATILE;
            else vola = 0;

            if ((retc = RegRestoreKey(hk, argv[2].strptr, vola)) == ERROR_SUCCESS)
            {
                RETC(0);
            }
            else
            {
                RETVAL(retc);
            }
        }
        else if ( strcmp(argv[0].strptr, "REPLACE") == 0 )
        {
            const char * p;
            GET_HKEY(argv[1].strptr, hk);
            if (!strcmp(argv[2].strptr, "%NULL%"))
            {
                p = NULL;
            }
            else
            {
                p = argv[2].strptr;
            }

            if ((retc = RegReplaceKey(hk, p, argv[3].strptr, argv[4].strptr)) == ERROR_SUCCESS)
            {
                RETC(0);
            }
            else
            {
                RETVAL(retc);
            }
        }
        RETC(1);
    }
    else
    {
        // MessageBox(0,"Illegal registry file command!","Error",MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
        RETC(1);
    }
}


HDDEDATA CALLBACK DDECallback(UINT wType,
                              UINT wFmt,
                              HCONV hConv,
                              HSZ hsz1,
                              HSZ hsz2,
                              HDDEDATA hDDEData,
                              DWORD dwData1,
                              DWORD dwData2)
{
    return NULL;
}

BOOL ProgmanCmd(LPSTR lpszCmd)
{
    DWORD dwDDEInst = 0;
    UINT ui;
    HSZ hszProgman;
    HCONV hConv;
    HDDEDATA hExecData;
    HDDEDATA exRes;



    ui = DdeInitialize(&dwDDEInst,
                       (PFNCALLBACK)DDECallback,
                       CBF_FAIL_ALLSVRXACTIONS,
                       0l);

    if (ui != DMLERR_NO_ERROR) return FALSE;



    hszProgman = DdeCreateStringHandle(dwDDEInst,
                                       "PROGMAN",
                                       CP_WINANSI);

    hConv = DdeConnect(dwDDEInst,
                       hszProgman,
                       hszProgman,
                       NULL);


    DdeFreeStringHandle(dwDDEInst, hszProgman);

    if (!hConv) return FALSE;


    hExecData = DdeCreateDataHandle(dwDDEInst,
                                    (LPBYTE)lpszCmd,
                                    lstrlen(lpszCmd)+1,
                                    0,
                                    NULL,
                                    0,
                                    0);


    exRes = DdeClientTransaction((LPBYTE)hExecData,
                         (DWORD)-1,
                         hConv,
                         NULL,
                         0,
                         XTYP_EXECUTE,
                         2000, // ms timeout
                         NULL);


    DdeDisconnect(hConv);
    DdeUninitialize(dwDDEInst);

    return exRes != 0;
}



BOOL AddPMGroup(const char *lpszGroup, const char *lpszPath)
{
    char buf[1024];

    if (lpszPath && lstrlen(lpszPath))
    {
        wsprintf(buf,
                 "[CreateGroup(%s,%s)]",
                 lpszGroup,
                 lpszPath);
    }
    else
    {
        wsprintf(buf,
                 "[CreateGroup(%s)]",
                 lpszGroup);
    }

    return ProgmanCmd(buf);
}

BOOL DeletePMGroup(const char *lpszGroup)
{
    char buf[512];

    if (lpszGroup && lstrlen(lpszGroup))
    {
        wsprintf(buf,
                 "[DeleteGroup(%s)]",
                 lpszGroup);
    }

    return ProgmanCmd(buf);
}

BOOL ShowPMGroup(const char *lpszGroup, WORD wCmd)
{
    char buf[512];

    if (lpszGroup && lstrlen(lpszGroup))
    {
        wsprintf(buf,
                 "[ShowGroup(%s,%u)]",
                 lpszGroup,
                 wCmd);
    }

    return ProgmanCmd(buf);
}


BOOL AddPMItem(const char *lpszCmdLine,
               const char *lpszCaption,
               const char *lpszIconPath,
               WORD  wIconIndex,
               const char *lpszDir,
               BOOL  bLast,
               const char *lpszHotKey,
               const char *lpszModifier,
               BOOL  bMin)
{
    char buf[2048];
    int Pos;

    if (bLast) Pos = -1; else Pos = 0;

    if (lpszIconPath && lstrlen(lpszIconPath))
    {
        wsprintf(buf,
                 "[AddItem(%s,%s,%s,%u,%d,%d,%s,%d,%d)]",
                 lpszCmdLine,
                 lpszCaption,
                 lpszIconPath,
                 wIconIndex,
                 Pos,Pos,
                 lpszDir,
                 MAKEWORD( atoi(lpszHotKey), atoi(lpszModifier)),
                 bMin);
    }
    else
    {
        wsprintf(buf,
                 "[AddItem(%s,%s,"","",%d,%d,%s,%d,%d)]",
                 lpszCmdLine,
                 lpszCaption,
                 Pos, Pos,
                 lpszDir,
                 MAKEWORD( atoi(lpszHotKey), atoi(lpszModifier)),
                 bMin);
    }

    return ProgmanCmd(buf);
}


BOOL DeletePMItem(const char *lpszItem)
{
    char buf[512];

    if (lpszItem && lstrlen(lpszItem))
    {
        wsprintf(buf,
                 "[DeleteItem(%s)]",
                 lpszItem);
    }

    return ProgmanCmd(buf);
}


BOOL LeavePM(BOOL bSaveGroups)
{
    char buf[256];

    wsprintf(buf,
             "[ExitProgman(%u)]",
             bSaveGroups ? 1 : 0);

    return ProgmanCmd(buf);
}


//-----------------------------------------------------------------------------
//
// Function: BOOL GetCurrentUserDesktopLocation
//
// This function reads the  CurrentUser Desktop Path from the registry.
//
// Syntax:   call GetCurrentUserDesktopLocation( LPBYTE szDesktopDir, LPDWORD  lpcbData )
//
// Params:
//
//  szDesktopDir   - Drive and Path of the Desktop to be returned
//  lpcbData       _ Max Size of szDesktopDir on entry, Size of szDesktopDir on exit
//
//   return :  TRUE  - No error
//             FALSE - Error
//-----------------------------------------------------------------------------
#define IDS_REGISTRY_KEY_CURRENT_SHELLFOLDER "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
#define IDS_CURRENT_DESKTOP                  "Desktop"

BOOL GetCurrentUserDesktopLocation ( LPBYTE szDesktopDir, LPDWORD  lpcbData )
{


   HKEY hKey;                      //handle of key
   long rc;
   DWORD Type ;

   szDesktopDir[0] ='\0';         //initialize return

   if ( (rc = RegOpenKeyEx(HKEY_CURRENT_USER,
                           IDS_REGISTRY_KEY_CURRENT_SHELLFOLDER,
                           0,
                           KEY_QUERY_VALUE,
                           &hKey)) == ERROR_SUCCESS )
   {
      if ( (rc = RegQueryValueEx(hKey,                      // handle of key to query
                           IDS_CURRENT_DESKTOP ,            // address of name of value to query
                           NULL,                            // reserved
                           &Type,                         // address of buffer for value type
                           szDesktopDir ,                   // address of returned data
                           lpcbData)) == ERROR_SUCCESS )    // .. returned here
      {
        RegCloseKey ( hKey ) ;
        return TRUE ;
      }
      RegCloseKey ( hKey ) ;
   }

   // Error occured
   return FALSE ;

}
//-----------------------------------------------------------------------------
//
// Function: BOOL GetAllUserDesktopLocation
//
// This function reads All UsersDesktop Path from the registry.
//
// Syntax:   call GetAllUserDesktopLocation( LPBYTE szDesktopDir, LPDWORD  lpcbData )
//
// Params:
//
//  szDesktopDir   - Drive and Path of the Desktop to be returned
//  lpcbData       _ Max Size of szDesktopDir on entry, Size of szDesktopDir on exit
//
//   return :  TRUE  - No error
//             FALSE - Error
//-----------------------------------------------------------------------------
#define IDS_REGISTRY_KEY_ALL_NT_SHELLFOLDER "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
#define IDS_ALL_NT_DESKTOP                  "Common Desktop"

#define IDS_REGISTRY_KEY_ALL_9x_SHELLFOLDER ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
#define IDS_ALL_9x_DESKTOP                  "Desktop"


BOOL GetAllUserDesktopLocation ( LPBYTE szDesktopDir, LPDWORD  lpcbData )
{


   HKEY hKey;                      //handle of key
   long rc;
   DWORD lpType ;
   LPTSTR lpValueName ;

   szDesktopDir[0] ='\0';         //initialize return

   // Test, if 95/98/Millenium or NT/Win2000
   if ( IsRunningNT() )
   {
     rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       IDS_REGISTRY_KEY_ALL_NT_SHELLFOLDER,
                       0,
                       KEY_QUERY_VALUE,
                       &hKey) ;
     lpValueName = IDS_ALL_NT_DESKTOP ;
   }
   else
   {
     rc = RegOpenKeyEx(HKEY_USERS,
                       IDS_REGISTRY_KEY_ALL_9x_SHELLFOLDER,
                       0,
                       KEY_QUERY_VALUE,
                       &hKey) ;
     lpValueName = IDS_ALL_9x_DESKTOP ;
   }
   if ( rc == ERROR_SUCCESS )
   {
      if ( (rc = RegQueryValueEx(hKey,                      // handle of key to query
                                 lpValueName ,                    // address of name of value to query
                                 NULL,                            // reserved
                                 &lpType,                         // address of buffer for value type
                                 szDesktopDir ,                   // address of returned data
                                 lpcbData)) == ERROR_SUCCESS )    // .. returned here
      {
        RegCloseKey ( hKey ) ;
        return TRUE ;
      }
      RegCloseKey ( hKey ) ;
   }

   // Error occured
   return FALSE ;

}

//-----------------------------------------------------------------------------
//
// Function: BOOL AddPMDesktopIcon
//
// This function creates a shortcut to a file on the desktop.
//
// Syntax:   call AddPmDesktopIcon( lpszName, lpszProgram, lpszIcon, iIconIndex, lpszWorkDir,
//                                  lpszLocation, lpszArguments, iscKey, iscModifier, run )
//
// Params:
//
//  lpszName       - Name of the short cut, displayed on the desktop below the icon
//  lpszProgram    - full name of the file of the shortcut referes to
//  lpszIcon       - full name of the icon file
//  iIconIndex     - index of the icon within the icon file
//  lpszWorkDir    - working directory of the shortcut
//  lpszLocation   - "PERSONAL"  : icon is only on desktop for current user
//                 - "COMMON"    : icon is displayed on desktop of all users
//                 - ""          : it's a link and can be placed in any folder
//  lpszArguments  - arguments passed to the shortcut
//  iScKey         - shortcut key
//  iScModifier    - modifier of shortcut key
//  run            - run application in  a : NORMAL
//                                           MINIMIZED
//                                           MAXIMIZED    window
//
//   return :  TRUE  - No error
//             FALSE - Error
//-----------------------------------------------------------------------------

BOOL AddPMDesktopIcon(const char *lpszName,
                      const char *lpszProgram,
                      const char *lpszIcon,
                      int   iIconIndex,
                      const char *lpszWorkDir,
                      const char *lpszLocation,
                      const char *lpszArguments,
                      int   iScKey,
                      int   iScModifier,
                      const char *lpszRun )
{
    HRESULT      hres;
    IShellLink*  psl;
    BOOL         bRc = TRUE;
    int          iRun = SW_NORMAL;

    CoInitialize(NULL);

    // Get a pointer to the IShellLink interface.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);

    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target
        psl->SetPath(lpszProgram );

        // icon location. default of iIconIndex is 0, set in WINSYSTM.CLS
        psl->SetIconLocation(lpszIcon, iIconIndex);

        // command-line arguments
        psl->SetArguments(lpszArguments );

        //shortcut key, the conversion to hex is done in WINSYSTM.CLS
        // modificationflag:
        // The modifier flags can be a combination of the following values:
        // HOTKEYF_SHIFT   = SHIFT key      0x01
        // HOTKEYF_CONTROL = CTRL key       0x02
        // HOTKEYF_ALT     = ALT key        0x04
        // HOTKEYF_EXT     = Extended key   0x08
        psl->SetHotkey(MAKEWORD( iScKey, iScModifier) );

        // working directory
        psl->SetWorkingDirectory(lpszWorkDir );

        // run in normal, maximized , minimized window, default is NORMAL, set in WINSYSTM.CLS
        if ( !stricmp(lpszRun,"MAXIMIZED") )
        {
            iRun = SW_SHOWMAXIMIZED;
        }
        else if ( !stricmp(lpszRun,"MINIMIZED") )
        {
            iRun = SW_SHOWMINNOACTIVE;
        }

        psl->SetShowCmd(iRun );

        // Query IShellLink for the IPersistFile interface for saving the
        // shortcut in persistent storage.
        hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];
            CHAR  szShortCutName[MAX_PATH];
            CHAR  szDesktopDir[MAX_PATH];
            DWORD dwSize = MAX_PATH;

            // If strlen(lpszLocation is < 6, then lpszName contains a full qualified filename
            if (strlen(lpszLocation)>5)
            {
                // if icon should only be created on the desktop of current user
                // get current user
                if (!stricmp(lpszLocation,"PERSONAL"))
                {
                    bRc = GetCurrentUserDesktopLocation ( (LPBYTE)szDesktopDir , &dwSize ) ;
                }
                else
                {  // Location is COMMON
                    bRc = GetAllUserDesktopLocation ( (LPBYTE)szDesktopDir , &dwSize ) ;
                }

                if ( bRc)
                {
                    //.lnk must be added to identify the file as a shortcut
                    sprintf( szShortCutName, "%s\\%s.lnk", szDesktopDir, lpszName);
                }
            }
            else   /* empty specifier so it's a link */
            {
                sprintf( szShortCutName, "%s.lnk", lpszName); /* lpszName contains a full qualified filename */
            }

            // Continueonly, if bRC is TRUE
            if ( bRc )
            {

                // Ensure that the string is Unicode.
                MultiByteToWideChar(CP_ACP, 0, szShortCutName, -1, wsz, MAX_PATH);

                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(wsz, TRUE);
                if (!SUCCEEDED(hres))
                {
                    bRc = FALSE;
                }
                ppf->Release();
            }
            else
            {
                bRc = FALSE;
            }
        }
        else
        {
            bRc = FALSE;
        }

        psl->Release();

    }
    else
    {
        bRc = FALSE;
    }

    return bRc;
}

//-----------------------------------------------------------------------------
//
// Function: INT DelPMDesktopIcon
//
// This function deletes a shortcut created with AddPMDektopIcon
//
// Syntax:   call DelPMDesktopIcon( lpszName,lpszLocation )
//
// Params:
//
//  lpszName       - Name of the short cut to be deleted
//
//   return :  0      - No error
//             others - Error codes from DeleteFile
//   Note: The returncode of GetCurrentXXXDesktopLocation is not checked because
//         this hould be handeled by DeleteFile. So if an error occured during this
//         function, file nit found is returned
//-----------------------------------------------------------------------------
INT DelPMDesktopIcon( const char *lpszName, const char *lpszLocation)
{
    CHAR  szDesktopDir[MAX_PATH];
    CHAR  szShortCutName[MAX_PATH];
    DWORD dwSize = MAX_PATH;

    // get the location (directory) of the shortcut file in dependency of
    // the specified location
    if (!stricmp(lpszLocation,"PERSONAL"))
    {
        GetCurrentUserDesktopLocation ( (LPBYTE)szDesktopDir , &dwSize );
    }
    else
    {  // Location is COMMON
        GetAllUserDesktopLocation ( (LPBYTE)szDesktopDir , &dwSize );
    }

    //.lnk must be added to identify the file as a shortcut
    sprintf( szShortCutName, "%s\\%s.lnk", szDesktopDir, lpszName);

    if (!DeleteFile(szShortCutName))
    {
        return GetLastError();
    }
    else
    {
        return 0;
    }
}

size_t RexxEntry WSProgManager(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    CHECKARG(2,11);

    if (strcmp( argv[0].strptr, "ADDGROUP") == 0 )
    {
        RETC(!AddPMGroup(argv[1].strptr, argv[2].strptr));
    }
    else if ( strcmp(argv[0].strptr, "DELGROUP") == 0 )
    {
        RETC(!DeletePMGroup(argv[1].strptr));
    }
    else if ( strcmp(argv[0].strptr, "SHOWGROUP") == 0 )
    {
        INT stype;
        if ( strcmp(argv[2].strptr, "MAX") == 0 )
        {
            stype = 3;
        }
        else if ( strcmp(argv[2].strptr, "MIN") == 0 )
        {
            stype = 2;
        }
        else
        {
            stype = 1;
        }

        RETC(!ShowPMGroup(argv[1].strptr, (WORD)stype));
    }
    else if ( strcmp(argv[0].strptr, "ADDITEM") == 0 )
    {
        if (argc > 7)
        {

            RETC(!AddPMItem(argv[1].strptr, argv[2].strptr, argv[3].strptr, (WORD)atoi(argv[4].strptr), argv[5].strptr,\
                            (BOOL)atoi(argv[6].strptr), argv[7].strptr, argv[8].strptr, (BOOL)atoi(argv[9].strptr)));
        }
        else
        {
            RETC(!AddPMItem(argv[1].strptr, argv[2].strptr, argv[3].strptr, (WORD)atoi(argv[4].strptr), argv[5].strptr,\
                            (BOOL)atoi(argv[6].strptr), "", "", 0));
        }

    }
    else if ( strcmp(argv[0].strptr, "DELITEM") == 0 )
    {
        RETC(!DeletePMItem(argv[1].strptr));
    }
    else if (strcmp(argv[0].strptr, "LEAVE") == 0 )
    {
        if ( strcmp(argv[1].strptr, "SAVE") == 0 )
        {
            RETC(!LeavePM(TRUE));
        }
        else
        {
            RETC(!LeavePM(FALSE));
        }
    }
    else if ( strcmp(argv[0].strptr, "ADDDESKTOPICON") == 0 )
    {
        RETC(!AddPMDesktopIcon( argv[1].strptr, argv[2].strptr, argv[3].strptr, atoi(argv[4].strptr),
                                argv[5].strptr, argv[6].strptr, argv[7].strptr, atoi(argv[8].strptr),
                                atoi(argv[9].strptr), argv[10].strptr ));
    }
    else if ( strcmp(argv[0].strptr, "DELDESKTOPICON") == 0 )
    {
        CHECKARG(3,3);
        RETVAL(DelPMDesktopIcon( argv[1].strptr, argv[2].strptr));
    }
    else
    {
        RETC(0);
    }
}



//-----------------------------------------------------------------------------
//
//  Section for the WindowsEventLog class
//
//-----------------------------------------------------------------------------

#define WSEL_DEFAULT_EVENTS_ARRAY_SIZE      512
#define WSEL_DEFAULT_SOURCE                 "Application"
#define HANDLE_ATTRIBUTE                    "CURRENTHANDLE"
#define RECORDS_ATTRIBUTE                   "EVENTS"
#define INITCODE_ATTRIBUTE                  "INITCODE"
#define MIN_READ_BUFFER_ATTRIBUTE           "MINIMUMREADBUFFER"
#define MIN_READ_MIN_ATTRIBUTE              "MINIMUMREADMIN"
#define MIN_READ_MAX_ATTRIBUTE              "MINIMUMREADMAX"

// This is the OS defined maximum size for any single record.
#define MAX_RECORD_SIZE                     256 * 1024

// The default max and min read buffer sizes.
#define MAX_READ_KB_COUNT                   256
#define MAX_READ_BUFFER                     MAX_READ_KB_COUNT * 1024
#define MIN_READ_KB_COUNT                   16
#define MIN_READ_BUFFER                     MIN_READ_KB_COUNT  * 1024

#define BAD_RECORD_ARRAY_MSG   "The events attribute has been altered so it is no longer an array object"
#define BAD_RECORD_START_MSG   "Requested start exceeds number of records"
#define TOO_SMALLBUFFER_MSG    "An event log record is too large (%u) for the read buffer (%u.)"
#define START_RECORD_OUT_OF_RANGE_MSG "The start record (%u) is out of range; (%u - %u)"
#define END_RECORD_OUT_OF_RANGE_MSG "start and count produce an end record (%u) out of range; (%u - %u)"

typedef enum {record_count, oldest_record, youngest_record} LogNumberOp;

/**
 * Query the registery for the name of an event record's message file. The file
 * name of the message file is stored as a value of a subkey under an
 * application log. The subkey name is the source name of the event.  The value
 * name corresponds to the type of message file, event, parameter, or category.
 * (Although the category message file is not currently used by the
 * WindowsEventLog object.)
 *
 * @param pValueName  The name of the value whose data is being sought. The data
 *                    is the message file name.
 * @param logName     The name of the event log.
 * @param source      The source of the event in the specified event log.
 * @param fileBuffer  [out] The message file path name is returned in this
 *                    buffer, which must be at least MAX_PATH in size.
 */
void lookupMessageFile(char *pValueName, char *logName, char *source, char *fileBuffer)
{
    char eventLogKey[] = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\";

    DWORD  dataSize;                   // size of the message file name returned from RegQueryValueEx
    DWORD  valueType = REG_EXPAND_SZ;  // type for call to RegQueryValueEx
    char  *valueData;                  // value returned from RegQueryValueEx
    char  *pKey;                       // name of complete key
    HKEY   hKey;                       // handle of key

    // If there is no value in the registry for the message file, or an error
    // happens, return the empty string
    fileBuffer[0] ='\0';

    // Build the complete key name
    pKey = (char *)LocalAlloc(LPTR, strlen(eventLogKey) + strlen(logName) + 1 + strlen(source) + 1);
    sprintf(pKey, "%s%s\\%s", eventLogKey, logName, source);

    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, pKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS )
    {
        // Use null for the data value to query for the size of the data.
        if ( RegQueryValueEx(hKey, pValueName, NULL, &valueType, NULL, &dataSize) == ERROR_SUCCESS )
        {
            // No error getting the size of the message file name, allocate a
            // buffer and get the value
            valueData = (char *)LocalAlloc(LPTR, dataSize);

            if ( RegQueryValueEx(hKey, pValueName, NULL, &valueType, (LPBYTE)valueData, &dataSize) == ERROR_SUCCESS )
            {
                // Place the message file path name in the return buffer,
                // expanding any environment variables in the process.
                ExpandEnvironmentStrings(valueData, fileBuffer, MAX_PATH);
            }
            LocalFree(valueData);
        }
    }
    LocalFree(pKey);
}

/**
 * Search a list of messages files for a specified message ID and, if found,
 * format and return the message.
 *
 * @param files      List of message files separated by a semi-colon.
 * @param msgID      The message ID to search for.
 * @param ppInserts  Possible array of insertion strings to use when formatting
 *                   the message.
 * @param lpMsgBuf   On success, a returned buffer containing the formatted
 *                   message.
 *
 * @return True if the message was found and formatted, false if not found or
 *         any other error.
 *
 * @note On success, lpMsgBuf will have been allocated by FormatMessage().  This
 *       buffer must be freed by the caller with LocalFree().
 */
BOOL findAndFormatDescription(char *files, DWORD msgID, char **ppInserts, LPVOID *lpMsgBuf )
{
    HINSTANCE h = NULL;
    char     *pBuffer = NULL;
    char     *pTemp = NULL;
    char     *pNext = NULL;
    DWORD     count = 0;

    if ( *files != '\0' )
    {
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY;
        DWORD langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

        pBuffer = (char *)LocalAlloc(LPTR, strlen(files) + 1);
        if ( pBuffer != NULL )
        {
            strcpy(pBuffer, files);
            pTemp = pBuffer;

            while ( pTemp != NULL && count == 0 )
            {
                pNext = strchr(pTemp, ';');
                if ( pNext != NULL )
                {
                    *pNext = '\0';
                    pNext++;
                }

                h = LoadLibrary(pTemp);
                if ( h != NULL )
                {
                    count = FormatMessage(flags, h, msgID, langID, (LPTSTR)lpMsgBuf, 0, ppInserts);
                    FreeLibrary(h);
                    h = NULL;
                }
                pTemp = pNext;
            }
            LocalFree(pBuffer);
        }
    }
    return(count != 0);
}

/**
 * Builds up the descriptive message that goes with an event record.
 *
 * @param pEvLogRecord  Event record of interest.
 * @param pchSource     Source of event with an event log.
 * @param ppMessage     Formatted description is returned here.
 */
void getEventDescription(PEVENTLOGRECORD pEvLogRecord , const char *pchSource, char **ppMessage)
{
    char  *pchString = NULL;           // pointer to substitution string within event log record
    int    iStringLen = 0;             // accumulation of the string length

    HINSTANCE hInstance = NULL;        // handle for message files
    LPVOID    lpMsgBuf = NULL;         // buffer to format the string
    char      *pSubstitutions[100];    // array of substitution strings
    char      chMessageFile[MAX_PATH]; // name of message file
    char      *pchPercent;             // pointer to "%%" within a substitution string

    // Initialize the array of substitution strings.
    memset(pSubstitutions, 0, sizeof(pSubstitutions));

    // Point to first substitution string in the event log record.
    pchString = (char*)pEvLogRecord + pEvLogRecord->StringOffset;

    // It is possible that the substitution strings themselves contain
    // substitutions in the form of %%nn.  The replacement for these comes from
    // the 'ParameterMessageFile'
    //
    // Loop over all the substitution strings, replacing each "%%" with a value
    // from the ParameterMessageFile. This builds an array of formatted
    // substitution strings.
    for ( int i = 0; i < pEvLogRecord->NumStrings; i++ )
    {
        // If a substitution string contains "%%", the name of the parameter
        // message file is read from the registry log. Each "%%" is followed by
        // an id, which is the id of the parameter string to be loaded from the
        // parameter message file.  The %% and id are then replaced by the
        // parameter string in the substitution string by FormatMessage().
        if ( (pchPercent = strstr(pchString, "%%")) )
        {
            // This substitution string contains a placeholder for parameters.
            // Get the name of parameter message file from the registry.
            lookupMessageFile("ParameterMessageFile", const_cast<char *>(pchSource),
                              (char *)pEvLogRecord + sizeof(EVENTLOGRECORD), chMessageFile);

            DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS;
            DWORD langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
            int   id = atoi(pchPercent + 2);   // ID of message to be loaded

            // Load the parameter message file and format the substitution
            // string.
            if ( (hInstance = LoadLibrary(chMessageFile)) != NULL )
            {
                if ( FormatMessage(flags, hInstance, id, langID, (LPTSTR)&pSubstitutions[i], 0, 0) == 0 )
                {
                    // An error occurred formatting the string, use the empty
                    // string.
                    pSubstitutions[i] = (char *)LocalAlloc(LPTR, 1);
                }
                FreeLibrary(hInstance);
                hInstance = NULL;
            }
            else
            {
                // The parameter message file could not be loaded, use the empty
                // string.
                pSubstitutions[i] = (char *)LocalAlloc(LPTR, 1);
            }

            // Accumulate the total string length.
            iStringLen += (int)strlen(pSubstitutions[i]) + 1;
        }
        else
        {
            // The substitution string does not have any replaceable parameters,
            // use them as is.
            iStringLen += (int)strlen(pchString) + 1;
            pSubstitutions[i] = (char *)LocalAlloc(LMEM_FIXED, strlen(pchString) + 1);
            strcpy(pSubstitutions[i], pchString);
        }
        // Point to next substitution string within the log record.
        pchString += strlen(pchString) + 1;
    }

    // Get the name of event log  message file from the registry
    lookupMessageFile("EventMessageFile", const_cast<char *>(pchSource),
                      (char *)pEvLogRecord + sizeof(EVENTLOGRECORD), chMessageFile);

    // Search for the message ID in the all event message files and format the
    // descriptive message if it is found.
    if ( findAndFormatDescription(chMessageFile, pEvLogRecord->EventID, pSubstitutions, &lpMsgBuf) )
    {
        // The message ID was found and the descriptive message was formatted.
        // So return that.
        *ppMessage = (char *)LocalAlloc(LPTR, strlen((char*)lpMsgBuf) + 1);
        strcpy(*ppMessage, (const char *)lpMsgBuf);

        // lpMsgBuf will have been allocated by FormatMessage(), it is no longer
        // needed so free it here.
        LocalFree(lpMsgBuf);
    }
    else
    {
        // The  message ID was not found, or some other error. Use the
        // substitution strings for the descriptive message, if there are any.
        if ( pEvLogRecord->NumStrings )
        {
            *ppMessage = (char *)LocalAlloc(LPTR, iStringLen + 1);

            for ( int i = 0; i < pEvLogRecord->NumStrings; i++ )
            {
                strcat(*ppMessage, pSubstitutions[i]);
                strcat(*ppMessage, " ");
            }
        }
        else
        {
            // No substitution strings, just return the empty string for the
            // descriptive message.
            *ppMessage = (char *)LocalAlloc(LPTR, 1);
        }
    }

    // Free any substitution strings.
    for ( int i = 0; i < pEvLogRecord->NumStrings; i++ )
    {
        LocalFree(pSubstitutions[i]);
    }
}

/**
 * Translates the security ID value in the event log record to a user name, or
 * "N/A" if that is not possible.
 *
 * @param pEvLogRecord  Pointer to event log record.
 *
 * @return  Allocated string naming the user.
 *
 * @note The string is allocated using LocalAlloc and must be freed using
 *       LocalFree.
 */
char * getEventUserName(PEVENTLOGRECORD pEvLogRecord)
{
    SID   *psid;
    char  *pUserID = NULL;
    DWORD  sizeID = 0;
    char   defUserID[] = "N/A";

    // Needed for LookupAccountSid(), but not used.
    SID_NAME_USE strDummy;
    char  *pDomain = NULL;
    DWORD sizeDomain = 0;

    if ( pEvLogRecord->UserSidLength == 0 )
    {
        // No SID record avaialable return default user ID
        pUserID = (char *)LocalAlloc(LPTR, strlen(defUserID) + 1);
        strcpy(pUserID, defUserID);
    }
    else
    {
        // Get the SID record within the event log record
        psid = (SID *)((char*)pEvLogRecord + pEvLogRecord->UserSidOffset);

        // Get the size required for the return buffers
        LookupAccountSid(NULL, psid, pUserID, &sizeID, pDomain, &sizeDomain, &strDummy);

        pUserID = (char *)LocalAlloc(LPTR, max(sizeID, (DWORD)strlen(defUserID) + 1));
        pDomain = (char *)LocalAlloc(LPTR, sizeDomain);

        if ( LookupAccountSid(NULL, psid, pUserID, &sizeID, pDomain, &sizeDomain, &strDummy) == 0 )
        {
            // Some type of error, just use the default.
            strcpy(pUserID, defUserID);
        }

        // Historically, the domain name has not been returned to the ooRexx
        // user.  Seems as though some one might want it.
        LocalFree(pDomain);
    }
    return pUserID;
}

/**
 * Allocates a buffer and fills it with the event log record's binary data field
 * converted to a character representation.  I.e., 4115 would be converted to a
 * series of chars: 01000103 (0x1013 == 4115) and returned in the buffer.
 *
 * @param pEvLogRecord  [in]  Pointer to an event log record.
 * @param ppBinData     [out] Allocated buffer address is returned here.
 *
 * @return  Number of characters used to represent the binary data bytes, with 0
 *          having the special meaning that there was no binary data in the
 *          event record. When 0 is returned, the actual size of the buffer is
 *          1.
 *
 * @note The buffer is allocated using LocalAlloc and must be freed using
 *       LocalFree.
 */
size_t getEventBinaryData(PEVENTLOGRECORD pEvLogRecord, char **ppBinData )
{
    char  *pRecData;
    char   pTemp[3];
    char  *p;

    if ( pEvLogRecord->DataLength != 0 )
    {
        *ppBinData = (char *)LocalAlloc(LPTR, (pEvLogRecord->DataLength + 1) * 2);
        p = *ppBinData;

        pRecData = (char*)pEvLogRecord + pEvLogRecord->DataOffset;

        for ( DWORD i = 0; i < pEvLogRecord->DataLength; i++ )
        {
            _snprintf(pTemp, sizeof(pTemp), "%02x", *pRecData);
            memcpy(p, pTemp, 2);
            p += 2;
            pRecData++;
        }
    }
    else
    {
        *ppBinData = (char *)LocalAlloc(LPTR, 1);
    }

    return (pEvLogRecord->DataLength * 2);
}

void systemServiceException(RexxMethodContext *context, char *msg)
{
    context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz(msg));
}

inline void outOfMemoryException(RexxMethodContext *c)
{
    systemServiceException(c, "Failed to allocate memory");
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Incorrect_method_list,
                      c->ArrayOfThree(c->WholeNumberToObject(pos), c->NewStringFromAsciiz(list), actual));
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, const char *actual)
{
    wrongArgValueException(c, pos, list, c->NewStringFromAsciiz(actual));
}

inline unsigned int wrongClassException(RexxMethodContext *c, int pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Incorrect_method_noclass, c->WholeNumberToObject(pos), c->NewStringFromAsciiz(n));
    return 0;
}


inline void setCurrentHandle(RexxMethodContext *context, HANDLE h)
{
    context->SetObjectVariable(HANDLE_ATTRIBUTE, context->NewPointer(h));
}

/**
 * Gets the handle value at the CURRENTHANDLE attribute. If the handle is not
 * null, it is an open handle to an event log.
 *
 * @param context  The method context we are operating under.
 *
 * @return An event handle.  It is not unexpected that this handle is NULL.
 */
HANDLE getCurrentHandle(RexxMethodContext *context)
{
    HANDLE h = NULL;

    RexxObjectPtr ptr = context->GetObjectVariable(HANDLE_ATTRIBUTE);
    if ( ptr != NULLOBJECT )
    {
        h = (HANDLE)context->PointerValue((RexxPointerObject)ptr);
    }
    return h;
}

/**
 * Gets the minimum read buffer size for reading event records from the object
 * attribute.  This minimum can be changed by the user.  If some error happens,
 * then the original default value is returned.
 *
 * @param c  The method context we are operating under.
 *
 * @return  The current minimum buffer size.
 */
DWORD getMinimumReadBufferSize(RexxMethodContext *c)
{
    uint32_t val = 0;

    RexxObjectPtr ptr = c->GetObjectVariable(MIN_READ_BUFFER_ATTRIBUTE);
    if ( ptr != NULLOBJECT )
    {
        if ( ! c->ObjectToUnsignedInt32(ptr, &val) )
        {
            val = MIN_READ_BUFFER;
        }
    }
    return (DWORD)val;
}

/**
 * Gets the records array (the array at the EVENTS attribute.)  The array is
 * emptied before it is returnd.
 *
 * @param context  The method context we are operating under.
 *
 * @return The empty records array on success, a null object on error.
 *
 * @note  If NULLOBJECT is returned an exception has been raised.
 */
RexxArrayObject getRecordArray(RexxMethodContext *context)
{
    RexxArrayObject records = NULLOBJECT;

    RexxObjectPtr ptr = context->GetObjectVariable(RECORDS_ATTRIBUTE);
    if ( ptr != NULLOBJECT )
    {
        if ( context->IsArray(ptr) )
        {
            records = (RexxArrayObject)ptr;
        }
    }

    if ( records == NULLOBJECT )
    {
        context->RaiseException1(Rexx_Error_Execution_user_defined,
                                 context->NewStringFromAsciiz(BAD_RECORD_ARRAY_MSG));
    }
    else
    {
        context->SendMessage0(records, "EMPTY");
    }

    return records;
}

/**  getOpenEventLog()
 *
 * Gets the opened handle to the specified event log.
 *
 * For the WindowsEventLog class, a currently opened event log handle always
 * take precedence over anything specified by the user.  If currentHandle is not
 * null, then server and source are always ignored.
 *
 * If there is not a currently opened handle, then the user specified server and
 * source are used.  Both server and source have defaults, so the user does not
 * need to specify either.
 *
 * Note that in the OpenEventLog() call, null is used to indicate the default
 * server (the local machine.)  If the user omitted the server arg, then sever
 * will be null, which gives us the default automatically.  Also note that
 * historically the empty string has also been used to indicate the default, so
 * that is maintained here.
 *
 * @param context  The method context pointer
 * @param server   The server to open the event log on.  If specified this must
 *                 be in UNC format, but we do not check for that, merely let
 *                 the function fail.
 * @param source   The source event log to open, may be omitted.
 * @param pHandle  The opened handle is returned here.
 * @param pRC      On an OS error, the value of GetLastError() is returned here,
 *                 otherwise 0 is returned here.
 *
 * @return         True indicates that the event log handle was opened by this
 *                 function and should be closed by the caller.  False
 *                 indicates that the handle comes from the currentHandle
 *                 attribute and should not be closed by the caller.
 */
bool getOpenEventLog(RexxMethodContext *context, const char *server, const char *source, HANDLE *pHandle, DWORD *pRC)
{
    bool didOpen = false;

    *pRC = 0;
    HANDLE hEventLog = getCurrentHandle(context);

    if ( hEventLog == NULL )
    {
        if ( server != NULL && strlen(server) == 0 )
        {
            server = NULL;
        }
        if ( source == NULL || strlen(source) == 0 )
        {
            source = WSEL_DEFAULT_SOURCE;
        }

        hEventLog = OpenEventLog(server, source);
        if ( hEventLog == NULL )
        {
            *pRC = GetLastError();
        }
        else
        {
            didOpen = true;
        }
    }

    *pHandle = hEventLog;
    return didOpen;
}

/**
 * If the current handle attribute has an opened handle, then it is closed.
 *
 * @param context  The method context we are operating under.
 *
 * @return  0 if there is no open handle, or on success.  If there is an error
 *          closing an open handle, then the system error code is returned.
 */
DWORD closeEventLog(RexxMethodContext *context)
{
    DWORD rc = 0;
    HANDLE hEventLog = getCurrentHandle(context);

    if ( hEventLog != NULL )
    {
        rc = (CloseEventLog(hEventLog) == 0) ? GetLastError() : 0;
        setCurrentHandle(context, NULL);
    }
    return rc;
}

/**
 * Gets either the total count of records in the event log, or the record number
 * of the oldest record, or the record number of the youngest record in the
 * event log.
 *
 * The first record written to the log is record number 1, so 1 is often the
 * oldest record.  However, if the user elects to have the log records
 * over-written when the log is full, the oldest record will no longer be 1 as
 * soon as the log starts to overwrite exsiting records.
 *
 * @param context     The method context we are operating under.
 * @param numberType  The number operation, i.e. what to do.
 * @param server      The server arg, see getOpenEventLog() for details.
 * @param source      The source arg, see getOpenEventLog() for details.
 *
 * @return The number requested on success, or the negated system error code on
 *         failure.
 */
int32_t getEventLogNumber(RexxMethodContext *context, LogNumberOp numberType, CSTRING server, CSTRING source)
{
    int32_t retNum = 0;
    HANDLE hEventLog;
    DWORD number;
    BOOL success;

    // We use number to hold a returned error code, if there is one.
    bool didOpen = getOpenEventLog(context, server, source, &hEventLog, &number);

    if ( hEventLog == NULL )
    {
        retNum = -(int32_t)number;
        goto finished;
    }

    // Unfortunately, on Vista, GetOldestEventLogRecord() returns an error if
    // the log is empty.  So, we work around that.
    DWORD count;
    success = GetNumberOfEventLogRecords(hEventLog, &count);
    if ( ! success )
    {
        retNum = -(int32_t)GetLastError();
        goto finished;
    }

    if ( count == 0 )
    {
        retNum = 0;
        goto finished;
    }

    DWORD oldest;
    success = GetOldestEventLogRecord(hEventLog, &oldest);
    if ( ! success )
    {
        retNum = -(int32_t)GetLastError();
        goto finished;
    }

    switch ( numberType )
    {
        case record_count :
            retNum = count;
            break;

        case oldest_record :
            retNum = oldest;
            break;

        case youngest_record :
            retNum = oldest + count - 1;
            break;

        default :
            // This really should be impossible.
            break;
    }

finished:

    if ( didOpen )
    {
        CloseEventLog(hEventLog);
    }
    return retNum;
}

/**
 * Convenience function to do most of the arg checking for the
 * WindowsEventLog::readRecords() method.
 *
 * @param context    The method context we're operating under.
 * @param hLog       The opened handle to the event log we are dealing with.
 * @param direction  The direction arg (arg 1) passed to the method.
 * @param start      The start arg (arg 4) passed to the method.
 * @param count      Pointer to the count arg (arg 5) passed to the method.  The
 *                   true count of records to be read is returned here.
 * @param flags      Pointer to the flags to use in the ReadEventLog() API.  The
 *                   actual flags to use are based on the args passed to the
 *                   method and returned here.
 * @param rc         A returned error code, 0 or a system error code.
 *
 * @return           True if things should continue, or false if they should not
 *                   continue.
 *
 * @note             If false is returned, it is likely that an exception has
 *                   been raised.  But, it may just be an OS system error.  In
 *                   all cases the value of rc should be returned to the
 *                   interpreter. If an exception has not been raised, it is the
 *                   system error code that needs to be returned to the ooRexx
 *                   programmer.
 */
bool checkReadRecordsArgs(RexxMethodContext *context, HANDLE hLog, CSTRING direction,
                          uint32_t start, uint32_t *count, DWORD *flags, DWORD *rc)
{
    *flags = EVENTLOG_FORWARDS_READ;
    if ( argumentExists(1) )
    {
        if ( stricmp("FORWARDS", direction) == 0 )
        {
            ; // Do nothing, flags are correct.
        }
        else if ( stricmp("BACKWARDS", direction) == 0 )
        {
            *flags = EVENTLOG_BACKWARDS_READ;
        }
        else
        {
            wrongArgValueException(context, 1, "FORWARDS, BACKWARDS", direction);
            return false;
        }
    }

    if ( argumentExists(4) || argumentExists(5) )
    {
        if ( argumentOmitted(4) )
        {
            context->RaiseException1(Rexx_Error_Incorrect_method_noarg, context->WholeNumberToObject(4));
            return false;
        }
        if ( argumentOmitted(5) )
        {

            context->RaiseException1(Rexx_Error_Incorrect_method_noarg, context->WholeNumberToObject(5));
            return false;
        }
        *flags |= EVENTLOG_SEEK_READ;
    }
    else
    {
        *flags |= EVENTLOG_SEQUENTIAL_READ;
    }

    DWORD totalRecords;
    if ( GetNumberOfEventLogRecords(hLog, &totalRecords) == 0 )
    {
        *rc = GetLastError();
        return false;
    }

    // We only need good start and count values if we are doing a seek read.
    // The start value is ignored on sequential reads.  The oldest record has
    // the smallest record number, always.
    if ( *flags & EVENTLOG_SEEK_READ )
    {
        DWORD oldestRecord;
        if ( GetOldestEventLogRecord(hLog, &oldestRecord) == 0 )
        {
            *rc = GetLastError();
            return false;
        }

        DWORD youngestRecord = oldestRecord + totalRecords - 1;
        char  tmp[256];

        if ( youngestRecord < start || start < oldestRecord )
        {
            _snprintf(tmp, sizeof(tmp), START_RECORD_OUT_OF_RANGE_MSG, start, oldestRecord, youngestRecord);

            context->RaiseException1(Rexx_Error_Incorrect_method_user_defined,
                                     context->NewStringFromAsciiz(tmp));
            return false;
        }

        DWORD endRecord;
        if ( *flags & EVENTLOG_FORWARDS_READ )
        {
            endRecord = start + *count - 1;
        }
        else
        {
            endRecord = start - *count + 1;
        }

        if ( youngestRecord < endRecord || endRecord < oldestRecord )
        {
            _snprintf(tmp, sizeof(tmp), END_RECORD_OUT_OF_RANGE_MSG, endRecord, oldestRecord, youngestRecord);

            context->RaiseException1(Rexx_Error_Incorrect_method_user_defined,
                                     context->NewStringFromAsciiz(tmp));
            return false;
        }
    }
    else
    {
        // For a sequential read, start is ignored.  But having a valid count
        // value makes looping through the log easier.
        *count = totalRecords;
    }
    return true;
}

inline bool isGoodEventType(uint16_t type)
{
    return (type == EVENTLOG_SUCCESS          ||
            type == EVENTLOG_AUDIT_FAILURE    ||
            type == EVENTLOG_AUDIT_SUCCESS    ||
            type == EVENTLOG_ERROR_TYPE       ||
            type == EVENTLOG_INFORMATION_TYPE ||
            type == EVENTLOG_WARNING_TYPE);
}

/** WindowsEventLog::init()
 *
 * The init() method for the WindowsEventLog object.  Sets the object attributes
 * to their default values.
 */
RexxMethod0(int, WSEventLog_init)
{
    RexxArrayObject obj = context->NewArray(WSEL_DEFAULT_EVENTS_ARRAY_SIZE);
    context->SetObjectVariable(RECORDS_ATTRIBUTE, obj);

    context->SetObjectVariable(MIN_READ_BUFFER_ATTRIBUTE, context->WholeNumberToObject(MIN_READ_BUFFER));
    context->SetObjectVariable(MIN_READ_MIN_ATTRIBUTE, context->WholeNumberToObject(MIN_READ_KB_COUNT));
    context->SetObjectVariable(MIN_READ_MAX_ATTRIBUTE, context->WholeNumberToObject(MAX_READ_KB_COUNT));

    context->SetObjectVariable(INITCODE_ATTRIBUTE, context->WholeNumberToObject(0));

    setCurrentHandle(context, NULL);

    return 0;
}

/** WindowsEventLog::open()
 *
 *  Opens a handle to the specified event log.
 *
 * @param server   [optional] The server to open the event log on.  If specified
 *                 this should be in UNC format.  The default is the local
 *                 machine.  The empty string can also be used to specify the
 *                 local macnine.
 *
 * @param source   [optional] The event log to open, the default is the
 *                 Application log.  The empty string can also be used to
 *                 specify the local machine.
 *
 * @note  The event logging service will open the Application log if the
 *        specified log (the source arg) can not be found.
 */
RexxMethod2(uint32_t, WSEventLog_open, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source)
{
    // Close an already opened handle if there is one.
    closeEventLog(context);

    // See the comment header for getOpenEventLog() for detail on the server and
    // source arguments.
    if ( server != NULL && strlen(server) == 0 )
    {
        server = NULL;
    }
    if ( source == NULL || strlen(source) == 0 )
    {
        source = WSEL_DEFAULT_SOURCE;
    }

    HANDLE hEventLog = OpenEventLog(server, source);
    if ( hEventLog == NULL )
    {
        return GetLastError();
    }

    setCurrentHandle(context, hEventLog);
    return 0;
}

/** WindowsEventLog::getNumber()
 *
 * Returns the number of event records in the specified log.
 *
 * @param server     The server to open the event log on.  If specified this
 *                   should be in UNC format.  The default is the local machine.
 *                   The empty string can also be used to specify the local
 *                   macnine.
 *
 * @param source     The event log to open, the default is the Application log.
 *                   The empty string can also be used to specify the local
 *                   machine.
 *
 * @return  The count of records in the log.
 */
RexxMethod2(int32_t, WSEventLog_getNumber, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source)
{
    return getEventLogNumber(context, record_count, server, source);
}

/** WindowsEventLog::getFirst()
 *
 * Returns the record number of the first recorded, or oldest, event.  Event
 * log messages are numbered sequentially, with the oldest record having the
 * lowest record number.
 *
 * @param server     The server to open the event log on.  If specified this
 *                   should be in UNC format.  The default is the local machine.
 *                   The empty string can also be used to specify the local
 *                   macnine.
 *
 * @param source     The event log to open, the default is the Application log.
 *                   The empty string can also be used to specify the local
 *                   machine.
 *
 * @return  The record number of the first recorded event.
 */
RexxMethod2(int32_t, WSEventLog_getFirst, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source)
{
    return getEventLogNumber(context, oldest_record, server, source);
}

/** WindowsEventLog::getLast()
 *
 * Returns the record number of the last recorded, or youngest, event.  Event
 * log messages are numbered sequentially, with the oldest record have the
 * lowest record number.
 *
 * @param server     The server to open the event log on.  If specified this
 *                   should be in UNC format.  The default is the local machine.
 *                   The empty string can also be used to specify the local
 *                   macnine.
 *
 * @param source     The event log to open, the default is the Application log.
 *                   The empty string can also be used to specify the local
 *                   machine.
 *
 * @return  The record number of the last recorded event.
 */
RexxMethod2(int32_t, WSEventLog_getLast, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source)
{
    return getEventLogNumber(context, youngest_record, server, source);
}

/** WindowsEventLog::isFull()
 *
 * Queries the event log to see if it is full.
 *
 * @return  True if event log is full, othewise false.  False is also returned
 *          if any system error happens.
 */
RexxMethod2(logical_t, WSEventLog_isFull, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source)
{
    HANDLE hLog;
    DWORD rc = 0;
    BOOL isFull = FALSE;

    // An error is just ignored and false returned.
    bool didOpen = getOpenEventLog(context, server, source, &hLog, &rc);

    if ( hLog != NULL )
    {
        EVENTLOG_FULL_INFORMATION efi;
        DWORD needed;
        if ( GetEventLogInformation(hLog, EVENTLOG_FULL_INFO, (void *)&efi, sizeof(efi), &needed) != 0 )
        {
            isFull = efi.dwFull ? TRUE : FALSE;
        }
    }

    if ( didOpen )
    {
        CloseEventLog(hLog);
    }
    return isFull;
}

/** WindowsEventLog::clear()
 *
 * Clears, empties, the specified event log.  Optionally backs up the log before
 * clearing it.
 *
 * @param server     The server to open the event log on.  If specified this
 *                   should be in UNC format.  The default is the local machine.
 *                   The empty string can also be used to specify the local
 *                   macnine.
 *
 * @param source     The event log to open, the default is the Application log.
 *                   The empty string can also be used to specify the local
 *                   machine.
 *
 * @param backupFile  If specified, the log will be backed up to this file.
 *                    There is no default value.  If backupFile is omitted, the
 *                    log is not backed up.  If a file name is supplied with no
 *                    extension, the the default event log extension of ".evt"
 *                    is added.
 *
 * @return            0 on success, the system error code otherwise.
 *
 * @note Note for the ooRexx docs:  The backup file can not exist on a remote
 *       machine. In particular, you can not use a backup file that is on a
 *       network mapped drive.  The clear operation will fail with rc 3 'The
 *       system cannot find the path specified.'
 */
RexxMethod3(uint32_t, WSEventLog_clear, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source,
            OPTIONAL_CSTRING, backupFile)
{
    HANDLE hEventLog;
    DWORD rc = 0;

    bool didOpen = getOpenEventLog(context, server, source, &hEventLog, &rc);

    if ( hEventLog == NULL )
    {
        return rc;
    }

    if ( backupFile != NULL && strlen(backupFile) == 0 )
    {
        backupFile = NULL;
    }

    // If the user supplied a backup file name, and it does not have an
    // extension, then add the default extension for event log files.  Note that
    // PathAddExtension() only adds the extension if the file does not already
    // have any extension.

    char *pDupeName = NULL;
    if ( backupFile != NULL )
    {
        pDupeName = (char *)LocalAlloc(LPTR, MAX_PATH);
        if ( ! pDupeName )
        {
            outOfMemoryException(context);
            return 0;
        }
        strcpy(pDupeName, backupFile);
        PathAddExtension(pDupeName, ".evt");
    }

    if ( ClearEventLog(hEventLog, pDupeName) == 0 )
    {
        rc = GetLastError();
    }

    // If the arg to LocalFree() is null, LocalFree() does nothing.
    LocalFree(pDupeName);

    if ( didOpen )
    {
        CloseEventLog(hEventLog);
    }

    return rc;
}

/**  WindowsEventLog::write()
 *
 * Write an event to an event log.  In theory all args are optional, but that
 * would not be much of an event.
 *
 * @param server     The server to open the event log on.  If specified this
 *                   should be in UNC format.  The default is the local machine.
 *                   The empty string can also be used to specify the local
 *                   macnine.
 *
 * @param source     The event log to open, the default is the Application log.
 *                   The empty string can also be used to specify the local
 *                   machine.
 *
 * @param type       The event type (EVENTLOG_SUCCESS EVENTLOG_AUDIT_FAILURE,
 *                   etc.,) the default is 1.
 * @param category   The event category, default is 0.
 * @param id         The event ID, default is 0.
 * @param data       Raw binary data, default is none.
 * @param strings    Args 7 to any number.  Any number of strings.  These are
 *                   the insertion strings used to contruct the event
 *                   descripiton.
 *
 * @return 0 on success, system error code on failure.
 */
RexxMethod7(uint32_t, WSEventLog_write, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source,
            OPTIONAL_uint16_t, t, OPTIONAL_uint16_t, category, OPTIONAL_uint32_t, id,
            OPTIONAL_RexxStringObject, rawData, ARGLIST, args)
{
    DWORD rc = 0;

    WORD         type         = 1;
    void        *data         = NULL;
    DWORD        dataSize     = 0;
    WORD         countStrings = 0;
    const char **strings      = NULL;

    // See the comment header for getOpenEventLog() for detail on the server and
    // source arguments.
    if ( server != NULL && strlen(server) == 0 )
    {
        server = NULL;
    }
    if ( source == NULL || strlen(source) == 0 )
    {
        source = WSEL_DEFAULT_SOURCE;
    }

    if ( argumentExists(3) && isGoodEventType(t) )
    {
        type = t;
    }

    if ( argumentExists(6) )
    {
        dataSize = (DWORD)context->StringLength(rawData);
        data = malloc(dataSize);
        if ( data != NULL )
        {
            memcpy(data, (void *)context->StringData(rawData), dataSize);
        }
        else
        {
            dataSize = 0;
        }
    }

    size_t argc = context->ArraySize(args);
    if ( argc > 6 )
    {
        strings = (const char **)malloc((argc - 6) * sizeof(const char **));
        if ( strings != NULL )
        {
            const char **p = strings;
            for ( size_t i = 7; i <= argc; i++, countStrings++)
            {
                RexxObjectPtr obj = context->ArrayAt(args, i);
                if ( obj == NULLOBJECT )
                {
                    break;
                }
                else
                {
                    *p++ = context->ObjectToStringValue(obj);
                }
            }
        }
    }

    HANDLE hSource = RegisterEventSource(server, source);
    if ( hSource != NULL )
    {
        if ( ReportEvent(hSource, type, category, id, NULL, countStrings, dataSize, strings, data) == 0 )
        {
            rc = GetLastError();
        }

        DeregisterEventSource(hSource);
    }
    else
    {
        rc = GetLastError();
    }

    if ( data != NULL )
    {
        free(data);
    }
    if ( strings != NULL )
    {
        free(strings);
    }

    return rc;
}

/** WindowsEventLog::minimumRead=
 *
 * Sets the minimumReadBuffer attribute.  This attribute controls the minimum
 * allocation size of the buffer used to read in event log records.  This value
 * is specified in multiples of 1 KB and must be wtihin the range of
 * MIN_READ_KB_COUNT and MAX_READ_KB_COUNT.
 *
 * @param  countKB  The number of kilobytes for the minimum buffer allocation.
 */
RexxMethod1(int, WSEventLog_minimumReadSet, uint32_t, countKB)
{
    if ( countKB < MIN_READ_KB_COUNT || MAX_READ_KB_COUNT < countKB )
    {
        context->RaiseException(
            Rexx_Error_Invalid_argument_range,
            context->ArrayOfFour(context->Int32ToObject(1),
                                 context->Int32ToObject(MIN_READ_KB_COUNT),
                                 context->Int32ToObject(MAX_READ_KB_COUNT),
                                 context->Int32ToObject(countKB)));
    }
    else
    {
        context->SetObjectVariable(MIN_READ_BUFFER_ATTRIBUTE, context->WholeNumberToObject(countKB * 1024));
    }
    return 0;
}

/** WindowsEventLog::minimumRead
 *
 * Returns the current setting for the minimum read buffer.  The value returned
 * is the size of the buffer in KiloBytes.
 *
 * @return  The minimum size allocation size of the event log read buffer
 *          buffer, in KiloBytes.  E.g., if the minimum is set at a 4096 byte
 *          buffer, then the return will be 4.
 */
RexxMethod0(uint32_t, WSEventLog_minimumReadGet)
{
    return getMinimumReadBufferSize(context) / 1024;
}

/**  WindowsEventLog::readRecords()
 *
 * Reads records from an event log.  If no args are given then all records from
 * the default system (the local machine) and the default source (the
 * Application log) are read.
 *
 * Each record is read from the event log, formatted into a string
 * representation and placed in an ooRexx array object. The array object is the
 * 'events' attribute of the WindowsEventLog.  Before the read is started, the
 * array is emptied.  After each read, the array will contain the records for
 * the previous read.
 *
 * @param direction  FORWARDS or BACKWARDS default is forwards.  The oldest
 *                   record is considered record 1.
 * @param server     The server to open the event log on.  If specified this
 *                   should be in UNC format.  The default is the local machine.
 *                   The empty string can also be used to specify the local
 *                   macnine.
 * @param source     The event log to open, the default is the Application log.
 *                   The empty string can also be used to specify the local
 *                   machine.
 * @param start      First record to read.  The default is all records.  If a
 *                   start record is specified then count is mandatory.
 * @param count      Count of records to read.  Only used if start is specified,
 *                   in which case count is not optional.
 *
 * @return           0 on success, the system error code on failure.
 */
RexxMethod5(uint32_t, WSEventLog_readRecords, OPTIONAL_CSTRING, direction, OPTIONAL_CSTRING, server,
            OPTIONAL_CSTRING, source, OPTIONAL_uint32_t, start, OPTIONAL_uint32_t, count)
{
    DWORD rc = 0;
    HANDLE hLog;

    bool didOpen = getOpenEventLog(context, server, source, &hLog, &rc);

    if ( hLog == NULL )
    {
        return rc;
    }

    // source is optional, but if it was omitted we need to set the variable to
    // the default.  It is used later.
    if ( source == NULL )
    {
        source = WSEL_DEFAULT_SOURCE;
    }

    DWORD flags;
    if ( ! checkReadRecordsArgs(context, hLog, direction, start, &count, &flags, &rc) )
    {
        if ( didOpen )
        {
            CloseEventLog(hLog);
        }
        return rc;
    }

    // Array to convert the event type into its string representation.
    char   evType[6][12]={"Error","Warning","Information","Success","Failure","Unknown"};
    int    evTypeIndex;

    PEVENTLOGRECORD pEvLogRecord;        // pointer to one event record
    PVOID  pBuffer = NULL;               // buffer for event records
    DWORD  bufferPos = 0;                // position within the eventlog buffer
    DWORD  bytesRead;                    // count of bytes read into buffer by ReadEventlog
    DWORD  needed;                       // returned from ReadEventlog if buffer to small
    char * pchEventSource  = NULL;       // source from event log record
    char * pchComputerName = NULL;       // computer name of event
    char * pchMessage = NULL;            // text of description string
    char * pBinaryData = NULL;           // raw data of event log record
    char   time[MAX_TIME_DATE];          // converted time of event
    char   date[MAX_TIME_DATE];          // converted date of event
    struct tm * DateTime;                // converted from elapsed seconds
    char * pchStrBuf = NULL;             // temp buffer for event string
    char * pchUser = NULL;               // user name for event
    DWORD  countRecords = 0;             // number of event log records processed

    // We'll try to allocate a buffer big enough to hold all the records, but
    // not bigger than our max.  Same idea for a minimum, we'll make sure the
    // buffer is at least a minimum size.  The minimum size can be adjusted by
    // the user, up to the maximum size of one record.
    DWORD bufSize = count * 1024;
    DWORD minSize = getMinimumReadBufferSize(context);
    bufSize = bufSize > MAX_READ_BUFFER ? MAX_READ_BUFFER : bufSize;
    bufSize = bufSize < minSize ? minSize : bufSize;

    pBuffer = LocalAlloc(LPTR, bufSize);
    if ( pBuffer == NULL )
    {
        outOfMemoryException(context);
        return 0;
    }

    // Get the ooRexx array object that will hold the event records.
    RexxArrayObject rxRecords = getRecordArray(context);
    if ( rxRecords == NULLOBJECT )
    {
        return 0;
    }

    BOOL success = TRUE;

    // Loop through the event log reading the number of records specified by the
    // ooRexx programmer.
    while ( (countRecords < count) && (success = ReadEventLog(hLog, flags, start, pBuffer, bufSize, &bytesRead, &needed)) )
    {
        pEvLogRecord = (PEVENTLOGRECORD)pBuffer;
        bufferPos = 0;

        while ( (bufferPos < bytesRead) && (countRecords < count) )
        {
            if (flags & EVENTLOG_FORWARDS_READ)
            {
                start = pEvLogRecord->RecordNumber + 1;
            }
            else
            {
                start = pEvLogRecord->RecordNumber - 1;
            }

            // Count each event record
            countRecords++;

            // Gather together all the pieces that will make up our final string
            // representation of this record.

            // Get index to event type string
            GET_TYPE_INDEX(pEvLogRecord->EventType, evTypeIndex);

            // Get time and date converted to local time.  The TimeWritten field
            // in the event log record struct is 32 bits but in VC++ 8.0, and
            // on, time_t is inlined to be 64-bits, and on 64-bit Windows time_t
            // is 64-bit to begin with.  So, _localtime32() needs to be used.
            // However, VC++ 7.0 doesn't appear to have __time32_t.
#if _MSC_VER < 1400
            DateTime = localtime((const time_t *)&pEvLogRecord->TimeWritten);
#else
            DateTime = _localtime32((const __time32_t *)&pEvLogRecord->TimeWritten);
#endif
            strftime(date, MAX_TIME_DATE,"%x", DateTime);
            strftime(time, MAX_TIME_DATE,"%X", DateTime);

            // Get the event source, computer name, and user name strings.
            pchEventSource  = (char*)pEvLogRecord + sizeof(EVENTLOGRECORD);
            pchComputerName = pchEventSource + strlen(pchEventSource)+1;
            pchUser = getEventUserName(pEvLogRecord);

            // Build the description string, which involves a look up from the
            // message files and possibly some string substitution.
            getEventDescription(pEvLogRecord, source, &pchMessage);

            // Get any binary data associated with the record.
            size_t cch = getEventBinaryData(pEvLogRecord, &pBinaryData);

            // Calculate how big a buffer we need for the final string, and
            // allocate it.
            size_t len = strlen(evType[evTypeIndex]) + 1 +
                         strlen(date) + 1 +
                         strlen(time) + 1 +
                         strlen(pchEventSource) + 3 +
                         12 +
                         strlen(pchUser) + 1 +
                         strlen(pchComputerName) + 1 +
                         strlen(pchMessage) + 3 +
                         cch + 3;

            pchStrBuf = (char *)LocalAlloc(LPTR, len);

            // Put it all together.  The binary data is surrounded by single
            // quotes so that the ooRexx user can parse it out.
            //
            // Type Date Time Source Id User Computer Details rawData
            sprintf(pchStrBuf,
                    "%s %s %s '%s' %u %s %s '%s' '",
                    evType[evTypeIndex],
                    date,
                    time,
                    pchEventSource,
                    LOWORD(pEvLogRecord->EventID),
                    pchUser,
                    pchComputerName,
                    pchMessage);

            // Now we need to tack on the binary data, the last apostrophe,
            // and the trailing null.
            len = strlen(pchStrBuf);
            memcpy((pchStrBuf + len), pBinaryData, cch);
            len += cch;

            *(pchStrBuf + len++) = '\'';
            *(pchStrBuf + len) = '\0';

            LocalFree(pchMessage);
            pchMessage = NULL;
            LocalFree(pBinaryData);
            LocalFree(pchUser);
            pchMessage = NULL;

            // Add the record to the array.
            context->ArrayPut(rxRecords, context->NewString(pchStrBuf, len), countRecords);

            LocalFree(pchStrBuf);

            // Update postion and point to next event record
            bufferPos += pEvLogRecord->Length;
            pEvLogRecord = (PEVENTLOGRECORD)(((char*)pEvLogRecord) + pEvLogRecord->Length);
        }
        memset(pBuffer, 0, bufSize);
    }

    if ( ! success )
    {
        rc = GetLastError();

        if ( rc == ERROR_INSUFFICIENT_BUFFER )
        {
            // Seems unlikely the buffer could be too small, but if it is we're
            // not going to fool with trying to reallocate a bigger buffer.
            char tmp[256];
            _snprintf(tmp, sizeof(tmp), TOO_SMALLBUFFER_MSG, needed, bufSize);
            context->RaiseException1(Rexx_Error_Execution_user_defined, context->NewStringFromAsciiz(tmp));
        }
        else if ( rc == ERROR_HANDLE_EOF )
        {
            // If we read to the end of log, we'll count that as okay.
            rc = 0;
        }
    }

    // Clean up and return.
    LocalFree(pBuffer);

    if ( didOpen )
    {
        CloseEventLog(hLog);
    }
    return rc;
}

/** WindowsEventLog::getLogNames()
 *
 * Obtains a list of all the event log names on the current system.
 *
 * @param   logNames [in/out]
 *            An .array object in which the event log names are returned.
 *
 * @return  An OS return code, 0 on success, the system error code on failure.
 *
 * @note    The passed in array is emptied before the log names are added.  On
 *          error, the array will also be empty.
 */
RexxMethod1(uint32_t, WSEventLog_getLogNames, RexxObjectPtr, obj)
{
    DWORD rc = 0;
    const char key[] = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\";
    HKEY hKey;

    if ( ! context->IsOfType(obj, "ARRAY") )
    {
        return wrongClassException(context, 1, "Array");
    }
    RexxArrayObject logNames = (RexxArrayObject)obj;

    context->SendMessage0(logNames, "EMPTY");

    // Open the ..\Services\EventLog key and then enumerate each of its subkeys.
    // The name of each subkey is the name of an event log.

    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_ENUMERATE_SUB_KEYS, &hKey);
    if ( rc == ERROR_SUCCESS )
    {
        DWORD index = 0;
        TCHAR name[MAX_REGISTRY_KEY_SIZE];
        DWORD cName = MAX_REGISTRY_KEY_SIZE;

        while ( (rc = RegEnumKeyEx(hKey, index, name, &cName, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS )
        {
            context->ArrayAppendString(logNames, name, strlen(name));
            index++;
            cName = MAX_REGISTRY_KEY_SIZE;
        }

        if ( rc == ERROR_NO_MORE_ITEMS )
        {
            // Not an error, this is the expected.
            rc = 0;
        }
        else
        {
            // An actual error, empty the array.
            context->SendMessage0(logNames, "EMPTY");
        }

        RegCloseKey(hKey);
    }

    return rc;
}

/** WindowsEventLog::close()
 *
 *  If we have an open open event log handle, closes it.
 *
 *  @return   Returns 0 on success and if there is no open handle to begin with.
 *            If there is an error attempting to close an open handle, the OS
 *            system error code is returned.
 */
RexxMethod0(uint32_t, WSEventLog_close)
{
    return closeEventLog(context);
}

/** WindowsEventLog::uninit()
 *
 *  Uninit method for the class.  Simply makes sure, if we have an open handle,
 *  it gets closed.
 */
RexxMethod0(uint32_t, WSEventLog_uninit)
{
    closeEventLog(context);
    return 0;
}

/* This method is used as a convenient way to test code. */
RexxMethod4(int, WSEventLog_test, RexxStringObject, data, OPTIONAL_CSTRING, server, OPTIONAL_CSTRING, source, ARGLIST, args)
{
    return 0;
}



size_t RexxEntry WSCtrlWindow(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HWND       hW;
    HWND       thW;
    char      *tmp_ptr;
    DWORD_PTR  dwResult;
    LRESULT    lResult;

    CHECKARG(1,10);

    if (!strcmp(argv[0].strptr,"DESK"))
    {
        RET_HANDLE(GetDesktopWindow());
    }
    else if (!strcmp(argv[0].strptr,"FIND"))
    {
        CHECKARG(2,2);
        hW = FindWindow(NULL, argv[1].strptr);
        RET_HANDLE(hW);
    }
    else if (!strcmp(argv[0].strptr,"FINDCHILD"))
    {
        CHECKARG(3,3);
        GET_HANDLE(argv[1].strptr, thW);

        hW = FindWindowEx(thW, NULL, NULL, argv[2].strptr);
        RET_HANDLE(hW);
    }
    else if (!strcmp(argv[0].strptr,"SETFG"))
    {
        CHECKARG(2,2);
        hW =  GetForegroundWindow();
        GET_HANDLE(argv[1].strptr, thW);

        if (hW != thW)
        {
            SetForegroundWindow(thW);
        }
        RET_HANDLE(hW);
    }
    else if (!strcmp(argv[0].strptr,"GETFG"))
    {
        RET_HANDLE(GetForegroundWindow())
    }
    else if (!strcmp(argv[0].strptr,"TITLE"))
    {
        CHECKARG(2,3);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW != NULL)
        {
            if (argc ==3)
            {
                RETC(!SetWindowText(hW, argv[2].strptr));
            }
            else
            {
                // GetWindowText do not work for controls from other processes and for listboxes
                // Now using WM_GETTEXT which works for all controls.
                // The return string was limitted to 255 characters, now it's unlimited.
                // retstr->strlength = GetWindowText(hW, retstr->strptr, 255);

                // at first get the text length to determine if the default lenght (255)of strptr would be exceeded.
                lResult = SendMessageTimeout(hW, WM_GETTEXTLENGTH ,0, 0, MSG_TIMEOUT_OPTS, MSG_TIMEOUT, &dwResult);
                // time out or error?
                if (lResult == 0)
                {
                    // an error, but we return an empty string
                    retstr->strlength = 0;
                    return 0;
                }
                else
                {
                    retstr->strlength = dwResult;
                }
                if ( retstr->strlength > (RXAUTOBUFLEN - 1)  )
                {
                    // text larger than 255, allocate a new one
                    // the original REXX retstr->strptr must not be released! REXX keeps track of this
                    tmp_ptr = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, retstr->strlength + 1);
                    if (!tmp_ptr)
                    {
                        RETERR
                    }
                    retstr->strptr = tmp_ptr;
                }
                // now get the text
                lResult = SendMessageTimeout(hW, WM_GETTEXT, retstr->strlength + 1, (LPARAM)retstr->strptr,
                                             MSG_TIMEOUT_OPTS, MSG_TIMEOUT, &dwResult);
                // time out or error?
                if (lResult == 0)
                {
                    // an error, but we return an empty string
                    retstr->strlength = 0;
                    return 0;
                }
                else
                {
                    retstr->strlength = dwResult;
                }

                // if still no text found, the control may be a listbox, for which LB_GETTEXT must be used
                if ( retstr->strlength == 0 )
                {
                    // at first get the selected item
                    int curSel;
                    lResult = SendMessageTimeout(hW, LB_GETCURSEL, 0, 0, MSG_TIMEOUT_OPTS, MSG_TIMEOUT, &dwResult);
                    // time out or error?
                    if (lResult == 0)
                    {
                        // an error, but we return an empty string
                        retstr->strlength = 0;
                        return 0;
                    }
                    else
                    {
                        curSel = (int) dwResult;
                    }
                    if (curSel != LB_ERR)
                    {
                        // get the text length and allocate a new strptr if needed
                        lResult = SendMessageTimeout(hW, LB_GETTEXTLEN, curSel, 0, MSG_TIMEOUT_OPTS, MSG_TIMEOUT, &dwResult);
                        // time out or error?
                        if (lResult == 0)
                        {
                            // an error, but we return an empty string
                            retstr->strlength = 0;
                            return 0;
                        }
                        else
                        {
                            retstr->strlength = dwResult;
                        }

                        if ( retstr->strlength > (RXAUTOBUFLEN - 1)  )
                        {
                            tmp_ptr = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, retstr->strlength + 1);
                            if (!tmp_ptr)
                            {
                                RETERR
                            }
                            retstr->strptr = tmp_ptr;
                        }
                        lResult = SendMessageTimeout(hW, LB_GETTEXT, curSel, (LPARAM)retstr->strptr, MSG_TIMEOUT_OPTS, MSG_TIMEOUT, &dwResult);
                        // time out or error?
                        if (lResult == 0)
                        {
                            // an error, but we return an empty string
                            retstr->strlength = 0;
                            return 0;
                        }
                        else
                        {
                            retstr->strlength = dwResult;
                        }
                    }
                }
            }
            return 0;
        }
        else
        {
            if (argc ==3)
            {
                RETC(!SetConsoleTitle(argv[2].strptr))
            }
            else
            {
                retstr->strlength = GetConsoleTitle(retstr->strptr, 255);
            }
            return 0;
        }
    }
    else if (!strcmp(argv[0].strptr,"CLASS"))
    {
        CHECKARG(2,2);
        GET_HANDLE(argv[1].strptr, hW);
        {
            retstr->strlength = GetClassName(hW, retstr->strptr, 255);
            return 0;
        }
    }

    else if (!strcmp(argv[0].strptr,"SHOW"))
    {
        CHECKARG(3,3);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            INT sw;
            if (!strcmp(argv[2].strptr,"HIDE")) sw = SW_HIDE;
            else
                if (!strcmp(argv[2].strptr,"MAX")) sw = SW_SHOWMAXIMIZED;
            else
                if (!strcmp(argv[2].strptr,"MIN")) sw = SW_MINIMIZE;
            else
                if (!strcmp(argv[2].strptr,"RESTORE")) sw = SW_RESTORE;
            else sw = SW_SHOW;
            RETC(ShowWindow(hW, sw))
        }
    }
    else if (!strcmp(argv[0].strptr,"HNDL"))
    {
        CHECKARG(3,3);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            INT gw;

            if (!strcmp(argv[2].strptr,"NEXT")) gw = GW_HWNDNEXT;
            else if (!strcmp(argv[2].strptr,"PREV")) gw = GW_HWNDPREV;
            else if (!strcmp(argv[2].strptr,"FIRSTCHILD")) gw = GW_CHILD;
            else if (!strcmp(argv[2].strptr,"FIRST")) gw = GW_HWNDFIRST;
            else if (!strcmp(argv[2].strptr,"LAST")) gw = GW_HWNDLAST;
            else if (!strcmp(argv[2].strptr,"OWNER")) gw = GW_OWNER;
            else RETC(1);
            RET_HANDLE(GetWindow(hW, gw));
        }
        else
        {
            RETC(0)
        }
    }
    else if (!strcmp(argv[0].strptr,"ID"))
    {
        CHECKARG(2,2);

        GET_HANDLE(argv[1].strptr, hW);
        RETVAL(GetDlgCtrlID(hW))
    }
    else if (!strcmp(argv[0].strptr,"ATPOS"))
    {
        POINT pt;
        CHECKARG(3,4);
        pt.x = atol(argv[1].strptr);
        pt.y = atol(argv[2].strptr);

        if (argc == 4)
        {
            GET_HANDLE(argv[3].strptr, hW);
        }
        else
        {
            hW = 0;
        }
        if (hW)
        {
            RET_HANDLE(ChildWindowFromPointEx(hW, pt, CWP_ALL));
        }
        else
        {
            RET_HANDLE(WindowFromPoint(pt));
        }
    }
    else if (!strcmp(argv[0].strptr,"RECT"))
    {
        RECT r;
        CHECKARG(2,2);
        retstr->strlength = 0;
        GET_HANDLE(argv[1].strptr, hW);
        if (hW && GetWindowRect(hW, &r))
        {
            sprintf(retstr->strptr,"%d,%d,%d,%d",r.left, r.top, r.right, r.bottom);
            retstr->strlength = strlen(retstr->strptr);
        }
        return 0;
    }
    else if (!strcmp(argv[0].strptr,"GETSTATE"))
    {
        CHECKARG(2,2);
        retstr->strlength = 0;
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            retstr->strptr[0] = '\0';
            if (!IsWindow(hW)) strcpy(retstr->strptr, "Illegal Handle");
            else
            {
                if (IsWindowEnabled(hW)) strcat(retstr->strptr, "Enabled ");
                else strcat(retstr->strptr, "Disabled ");
                if (IsWindowVisible(hW)) strcat(retstr->strptr, "Visible ");
                else strcat(retstr->strptr, "Invisible ");
                if (IsZoomed(hW)) strcat(retstr->strptr, "Zoomed ");
                else if (IsIconic(hW)) strcat(retstr->strptr, "Minimized ");
                if (GetForegroundWindow() == hW) strcat(retstr->strptr, "Foreground ");
            }
            retstr->strlength = strlen(retstr->strptr);
        }
        return 0;
    }
    else if (!strcmp(argv[0].strptr,"GETSTYLE"))
    {
        CHECKARG(2,2);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            WINDOWINFO wi;

            wi.cbSize = sizeof(WINDOWINFO);
            if ( GetWindowInfo(hW, &wi) )
            {
                _snprintf(retstr->strptr, RXAUTOBUFLEN, "0x%08x 0x%08x", wi.dwStyle, wi.dwExStyle);
                retstr->strlength = strlen(retstr->strptr);
                return 0;
            }
            else
            {
                _snprintf(retstr->strptr, RXAUTOBUFLEN, "%d", GetLastError());
                retstr->strlength = strlen(retstr->strptr);
                return 0;
            }
        }
    }
    else if (!strcmp(argv[0].strptr,"ENABLE"))
    {
        CHECKARG(3,3);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            RETC(EnableWindow(hW, atoi(argv[2].strptr)));
        }
    }
    else if (!strcmp(argv[0].strptr,"MOVE"))
    {
        CHECKARG(4,4);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            RECT r;
            if (!GetWindowRect(hW, &r))
            {
                RETC(1);
            }
            RETC(!MoveWindow(hW, atol(argv[2].strptr), atol(argv[3].strptr), r.right - r.left, r.bottom-r.top, TRUE))
        }
    }
    else if (!strcmp(argv[0].strptr,"SIZE"))
    {
        CHECKARG(4,4);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            RECT r;
            if (!GetWindowRect(hW, &r))
            {
                RETC(1);
            }
            RETC(!SetWindowPos(hW, NULL, r.left, r.top,  atol(argv[2].strptr), atol(argv[3].strptr), SWP_NOMOVE | SWP_NOZORDER))
        }
    }
    RETC(1); /* in this case 0 is an error */
}



size_t RexxEntry WSCtrlSend(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HWND hW,HwndFgWin;

    CHECKARG(1,10);

    if ( strcmp(argv[0].strptr, "KEY") == 0 )
    {
        CHECKARG(4,5);
        GET_HANDLE(argv[1].strptr, hW);
        if (hW)
        {
            WPARAM k;
            LPARAM l;

            k = (WPARAM)atoi(argv[2].strptr);
            if (argc == 5)
            {
                l = strtol(argv[4].strptr,'\0',16);
            }
            else
            {
                l = 0;
            }

            /* Combination of keys (e.g. SHIFT END) and function keys (e.g. F1) dont worked                 */
            /* The reson is, that the WM_KEWUP/Doen message cannot handle this !                            */
            /* To get the function keys working, WM_KEYDOWN/UP can be used, but the message MUST be posted! */
            /* To get key combination working the keybd_event function must be use. But .....               */
            /* The window must be in the Foreground and The handle must be associated:                      */
            /* To get these working, the REXX code must be lok like:                                        */
            /* EditHandle = npe~Handle                                                                      */
            /* npe~ToForeground                                                                             */
            /* npe~AssocWindow(np~Handle)                                                                   */
            /* npe~SendKeyDown("SHIFT")                                                                     */
            /* npe~SendKeyDown("HOME")                                                                      */
            /* npe~SendKeyUp("SHIFT")                                                                       */
            /* npe~SendKeyUp("HOME")                                                                        */

            if (argv[3].strptr[0] == 'D')
            {
                HwndFgWin = GetForegroundWindow();
                if ( hW == HwndFgWin)
                {
                    if (l != 0)
                    {
                        keybd_event((BYTE)k,0,KEYEVENTF_EXTENDEDKEY,0);
                    }
                    else
                    {
                        keybd_event((BYTE)k,0,0,0);
                    }

                    retstr->strptr[0] = '1';
                    retstr->strptr[1] = '\0';
                }
                else
                {
                    itoa(PostMessage(hW,WM_KEYDOWN, k,l), retstr->strptr, 10);
                }
            }
            else if (argv[3].strptr[0] == 'U')
            {
                HwndFgWin = GetForegroundWindow();
                if ( hW == HwndFgWin )
                {
                    if (l == 1)
                    {
                        keybd_event((BYTE)k,0,KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY,0);
                    }
                    else
                    {
                        keybd_event((BYTE)k,0,KEYEVENTF_KEYUP,0);
                    }

                    retstr->strptr[0] = '1';
                    retstr->strptr[1] = '\0';
                }
                else
                {
                    itoa(PostMessage(hW,WM_KEYUP, k, l | 0xC0000000), retstr->strptr, 10);
                    retstr->strptr[0] = '1';
                    retstr->strptr[1] = '\0';
                }
            }
            /* WM_CHAR don't works for ALT key combinations                             */
            /* WM_SYSKEYDOWN and hW,WM_SYSKEYUP must used                               */
            else if ( l != 0 )
            {
                itoa(PostMessage(hW,WM_SYSKEYDOWN, k,l), retstr->strptr, 10);
                itoa(PostMessage(hW,WM_SYSKEYUP, k,l), retstr->strptr, 10);
            }
            else
            {
                itoa(SendNotifyMessage(hW,WM_CHAR, k,l), retstr->strptr, 10);
            }
            retstr->strlength = strlen(retstr->strptr);
            return 0;
        }
    }
    else if ( strcmp(argv[0].strptr, "MSG") == 0 )
    {
        UINT   msg;
        WPARAM wp;
        LPARAM lp;

        CHECKARG(5,6);

        GET_HANDLE(argv[1].strptr, hW);
        GET_HANDLE(argv[2].strptr, msg);
        GET_HANDLE(argv[3].strptr, wp);
        GET_HANDLE(argv[4].strptr, lp);

        /* The 6th arg can, optionally, be used as an extra qualifier.  Currently
         * only used in one case.
         */
        if ( argc > 5 && argv[5].strptr[0] == 'E' )
        {
            lp = (LPARAM)"Environment";
        }
        if ( SendNotifyMessage(hW, msg, wp, lp) )
        {
            RETVAL(0)
        }
        else
        {
            RETVAL(GetLastError())
        }
    }
    else if ( strcmp(argv[0].strptr,"TO") == 0 )  /* Send message Time Out */
    {
        UINT      msg, timeOut;
        WPARAM    wp;
        LPARAM    lp;
        DWORD_PTR dwResult;
        LRESULT   lResult;

        CHECKARG(6,7);

        GET_HANDLE(argv[1].strptr, hW);
        GET_HANDLE(argv[2].strptr, msg);
        GET_HANDLE(argv[3].strptr, wp);
        GET_HANDLE(argv[4].strptr, lp);
        GET_HANDLE(argv[5].strptr, timeOut);

        /* The 7th arg can, optionally, be used as an extra qualifier.  Currently
         * only used in one case.
         */
        if ( argc > 6 && argv[6].strptr[0] == 'E' )
        {
            lp = (LPARAM)"Environment";
        }

        lResult = SendMessageTimeout(hW, msg, wp, lp, MSG_TIMEOUT_OPTS, timeOut, &dwResult);
        if ( lResult == 0 )
        {
            /* Some error occurred, if last error is 0 it is a time out,
             * otherwise some other system error.
             */
            DWORD err = GetLastError();
            if ( err == 0 )
            {
                err = 1;  // We are going to negate this.
            }
            RETVAL(-(INT)err);
        }
        else
        {
            return dwordPtrToRexx(dwResult, retstr);
        }
    }
    else if ( strcmp(argv[0].strptr, "MAP") == 0 )
    {
        CHECKARG(2,2);
        RETVAL(MapVirtualKey((UINT)atoi(argv[1].strptr), 2));
    }

    RETC(1);  /* in this case 0 is an error */
}



size_t RexxEntry WSCtrlMenu(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    CHECKARG(1,10);

    if (!strcmp(argv[0].strptr,"GET"))
    {
        CHECKARG(2,2);
        HWND hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);

        RET_HANDLE(GetMenu(hMenu));
    }
    else if (!strcmp(argv[0].strptr,"SYS"))
    {
        CHECKARG(2,2);
        HWND hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);

        RET_HANDLE(GetSystemMenu(hMenu, FALSE));
    }
    else if (!strcmp(argv[0].strptr,"SUB"))
    {
        CHECKARG(3,3);
        HMENU hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);

        RET_HANDLE(GetSubMenu(hMenu, atoi(argv[2].strptr)));
    }
    else if (!strcmp(argv[0].strptr,"ID"))
    {
        CHECKARG(3,3);
        HMENU hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);

        RETVAL(GetMenuItemID(hMenu, atoi(argv[2].strptr)));
    }
    else if (!strcmp(argv[0].strptr,"CNT"))
    {
        CHECKARG(2,2);
        HMENU hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);

        RETVAL(GetMenuItemCount(hMenu));
    }
    else if (!strcmp(argv[0].strptr,"TEXT"))
    {
        HMENU hM;
        UINT flag;
        CHECKARG(4,4);

        GET_HANDLE(argv[1].strptr, hM);
        if (!hM)
        {
            RETC(0);
        }
        if (argv[3].strptr[0] == 'C')
        {
            flag = MF_BYCOMMAND;
        }
        else
        {
            flag = MF_BYPOSITION;
        }

        retstr->strlength = GetMenuString(hM, atol(argv[2].strptr), retstr->strptr, 255, flag);
        return 0;
    }
    else if (!strcmp(argv[0].strptr,"STATE"))
    {
        CHECKARG(2,4);

        HMENU hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);

        if ( argc == 2 )
        {
            RETVAL(IsMenu(hMenu));
        }

        CHECKARG(4,4);

        UINT flags;
        UINT pos = atoi(argv[3].strptr);

        flags = GetMenuState(hMenu, pos, MF_BYPOSITION);

        if ( flags == 0xffffffff )
        {
            // Error, most likely no such position.  Return false.
            RETVAL(0);
        }

        if ( argv[2].strptr[0] == 'M' )
        {
            RETVAL((flags & MF_POPUP) ? 1 : 0);
        }
        else if ( argv[2].strptr[0] == 'C' )
        {
            RETVAL((flags & MF_CHECKED) ? 1 : 0);
        }
        else
        {
            RETVAL(((flags & MF_POPUP) == 0 && (flags & MF_SEPARATOR) != 0) ? 1 : 0);
        }
    }

    RETC(1)  /* in this case 0 is an error */
}



size_t RexxEntry WSClipboard(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    BOOL ret = FALSE;

    CHECKARG(1,2);

    if (!strcmp(argv[0].strptr, "COPY"))
    {
        HGLOBAL hmem;
        char * membase;

        CHECKARG(2,2);
        hmem = (char *)GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, argv[1].strlength + 1);
        membase = (char *) GlobalLock(hmem);

        if (membase)
        {
            memcpy(membase, argv[1].strptr, argv[1].strlength + 1);
            if (OpenClipboard(NULL) && EmptyClipboard())
            {
                ret = SetClipboardData(CF_TEXT, hmem) != NULL;
            }
            GlobalUnlock(membase);
            CloseClipboard();
        }
        RETC(!ret)
    }
    else if (!strcmp(argv[0].strptr, "PASTE"))
    {
        HGLOBAL hmem;
        char * membase;
        if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(NULL))
        {
            hmem = GetClipboardData(CF_TEXT);
            membase = (char *) GlobalLock(hmem);
            if (membase)
            {
                size_t s = GlobalSize(hmem);
                if (s>255)
                {
                    retstr->strptr = (char *)GlobalAlloc(GMEM_FIXED, s);
                    if (retstr->strptr == NULL)
                    {
                        CloseClipboard();
                        return -1;
                    }
                }
                s = strlen(membase);
                memcpy(retstr->strptr, membase, s+1);
                retstr->strlength = s;
                GlobalUnlock(membase);
                CloseClipboard();
                return 0;
            }
            else
            {
                CloseClipboard();
            }
        }
        retstr->strlength = 0;
        return 0;
    }
    else if (!strcmp(argv[0].strptr, "EMPTY"))
    {
        if (IsClipboardFormatAvailable(CF_TEXT))
        {
            BOOL ret;
            if (!OpenClipboard(NULL))
            {
                RETC(1)
            }
            ret = !EmptyClipboard();
            CloseClipboard();
            RETC(ret)
        }
        RETC(0)
    }
    else if (!strcmp(argv[0].strptr, "AVAIL"))
    {
        RETC(IsClipboardFormatAvailable(CF_TEXT))
    }
    else
    {
        RETVAL(-1)
    }
}


VOID Little2BigEndian(BYTE *pbInt, INT iSize)
{
    /* convert any integer number from little endian to big endian or vice versa */
    BYTE  bTemp[32];
    INT   i;

    if (iSize <= 32)
    {
        for (i = 0; i < iSize; i++)
        {
            bTemp[i] = pbInt[iSize - i - 1];
        }

        memcpy(pbInt, bTemp, iSize);
    }
}

/**
 * Prior to 4.0.0, this function was documented as a work around to use the
 * WindowObject class when no WindowManager object had been instantiated.  So
 * for now it needs to stay. Does nothing.
 */
size_t RexxEntry InstWinSysFuncs(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   RETC(0)
}


// now build the actual entry lists
RexxRoutineEntry rxwinsys_functions[] =
{
    REXX_CLASSIC_ROUTINE(WSRegistryKey,    WSRegistryKey),
    REXX_CLASSIC_ROUTINE(WSRegistryValue,  WSRegistryValue),
    REXX_CLASSIC_ROUTINE(WSRegistryFile,   WSRegistryFile),
    REXX_CLASSIC_ROUTINE(WSProgManager,    WSProgManager),
    REXX_CLASSIC_ROUTINE(WSCtrlWindow,     WSCtrlWindow),
    REXX_CLASSIC_ROUTINE(WSCtrlSend,       WSCtrlSend),
    REXX_CLASSIC_ROUTINE(WSCtrlMenu,       WSCtrlMenu),
    REXX_CLASSIC_ROUTINE(WSClipboard,      WSClipboard),
    REXX_CLASSIC_ROUTINE(InstWinFuncs,     InstWinSysFuncs),
    REXX_LAST_ROUTINE()
};


RexxMethodEntry rxwinsys_methods[] = {
    REXX_METHOD(WSRegistry_delete,          WSRegistry_delete),

    REXX_METHOD(WSEventLog_test,            WSEventLog_test),
    REXX_METHOD(WSEventLog_init,            WSEventLog_init),
    REXX_METHOD(WSEventLog_uninit,          WSEventLog_uninit),
    REXX_METHOD(WSEventLog_open,            WSEventLog_open),
    REXX_METHOD(WSEventLog_close,           WSEventLog_close),
    REXX_METHOD(WSEventLog_write,           WSEventLog_write),
    REXX_METHOD(WSEventLog_readRecords,     WSEventLog_readRecords),
    REXX_METHOD(WSEventLog_minimumReadSet,  WSEventLog_minimumReadSet),
    REXX_METHOD(WSEventLog_minimumReadGet,  WSEventLog_minimumReadGet),
    REXX_METHOD(WSEventLog_getNumber,       WSEventLog_getNumber),
    REXX_METHOD(WSEventLog_getFirst,        WSEventLog_getFirst),
    REXX_METHOD(WSEventLog_getLast,         WSEventLog_getLast),
    REXX_METHOD(WSEventLog_isFull,          WSEventLog_isFull),
    REXX_METHOD(WSEventLog_getLogNames,     WSEventLog_getLogNames),
    REXX_METHOD(WSEventLog_clear,           WSEventLog_clear),
    REXX_LAST_METHOD()
};

RexxPackageEntry rxwinsys_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "RXWINSYS",                          // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    rxwinsys_functions,                  // the exported functions
    rxwinsys_methods                     // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(rxwinsys);

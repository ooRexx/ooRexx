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

#include <windows.h>
#include "oorexxapi.h"
#include <stdio.h>
#include <string.h>
#include <ddeml.h>
#include <time.h>
#include <shlobj.h>

#define STR_BUFFER    256
#define MAX_TIME_DATE 128
#define MAX_VARNAME   256

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
   ((value[0] == '0') && (toupper(value[1]) == 'X'))


/* Note many existing programs abbreviate HKEY_LOCAL_MACHINE to "LOCAL_MACHINE",
 * or "MACHINE", and many do not.  Many existing programs use the full
 * HKEY_LOCAL_MACHINE.  So the comparison needs to remain strstr.
 *
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
      case EVENTLOG_AUDIT_SUCCESS:    \
         index=3;                     \
         break;                       \
      case EVENTLOG_AUDIT_FAILURE:    \
         index=4;                     \
         break;                       \
      default:                        \
        index=4;                      \
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
    else if ( strlen(string) < 2 )
    {
        return sscanf(string, "%p", pointer) == 1;
    }
    else if ( string[1] == 'x' && string[0] == '0' )
    {
        return sscanf(string, "0x%p", pointer) == 1;
    }
    else if ( string[1] == 'X' && string[0] == '0' )
    {
        return sscanf(string, "0X%p", pointer) == 1;
    }
    else
    {
        return sscanf(string, "%p", pointer) == 1;
    }
}


void pointer2string(PRXSTRING result, void *pointer)
{
    sprintf(result->strptr, "0x%p", pointer);
    result->strlength = strlen(result->strptr);
}


BOOL IsRunningNT()
{
    OSVERSIONINFO version_info={0};

    version_info.dwOSVersionInfoSize = sizeof(version_info);
    GetVersionEx(&version_info);
    if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT) return TRUE; // Windows NT
    else return FALSE;                                              // Windows 95
}


size_t RexxEntry WSRegistryKey(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HKEY hk;
    LONG rc;

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

        if ((argc < 4) || strcmp(argv[3].strptr,"ALL"))
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
    else if ( strcmp(argv[0].strptr, "DELETE") == 0 )
    {
        GET_HKEY(argv[1].strptr, hk);

        if ((rc = RegDeleteKey(hk, argv[2].strptr)) == ERROR_SUCCESS)
        {
            RETC(0);
        }
        else
        {
            RETVAL(rc);
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
        ULONG intsize;

        cbData = sizeof(valData);

        GET_HKEY(argv[1].strptr, hk);

        if (RegQueryValueEx(hk,        // handle of key to query
                            argv[2].strptr,    // address of name of value to query
                            NULL,        // reserved
                            &valType,    // address of buffer for value type
                            NULL,        // NULL to get the size
                            &cbData) == ERROR_SUCCESS)
        {        // address of data buffer size
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
                                &cbData) == ERROR_SUCCESS)
            {        // address of data buffer size

                if ((GlobalFlags(retstr->strptr) != GMEM_INVALID_HANDLE) && !GetLastError())
                {
                    intsize = (ULONG)GlobalSize(retstr->strptr);
                }
                else
                {
                    intsize = STR_BUFFER;
                }
                if (intsize > retstr->strlength+1)
                {
                    intsize = (ULONG)retstr->strlength+1;
                }
                if (cbData+10 > intsize)
                {
                    if ((GlobalFlags(retstr->strptr) != GMEM_INVALID_HANDLE) && !GetLastError())
                    {
                        GlobalFree(retstr->strptr);
                    }
                    retstr->strptr = (char *)GlobalAlloc(GMEM_FIXED, cbData + 10);
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
            else
            {
                RETC(0);
            }
            GlobalFree(valData);
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
    else if (strcmp(argv[0].strptr, "LEAVE"))
    {
        if ( strcmp(argv[1].strptr, "SAVE") == 0)
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


//Functions for reading Event Log

//-----------------------------------------------------------------------------
//
//  Function GetEvPaMessageFile
//
//  Gets the name of the message file from the registry
//
//  char * GetEvPaMessageFile(char * pMessageFile, char * pchSource,char * pchSourceName, char * chMessageFile)
//
//       pMessaegFile  - (in) What to get
//                            "EventMessageFile"     get event message file name
//                            "ParameterMessageFile" get parameter message file
//
//       pchSource     - (in) second part of the key to build
//
//       pchSourceName - (in) third part of the key to build
//
//       chMessageFile - (in/out) name of the message file
//                                string buffer with length MAX_PATH
//
//-----------------------------------------------------------------------------
void GetEvPaMessageFile(char * pMessageFile, char * pchSource,char * pchSourceName, char * chMessageFile)
{
   DWORD dataSize;                 //size of the message file name returned from RegQueryValueEx
   DWORD valType = REG_EXPAND_SZ;  //type for call to RegQueryValueEx
   char chKey[] = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\";  //first part of key for message files
   char *pchKey;                   //contains name of complete key
   char *valData;                  //value returned from RegQueryValueEx
   HKEY hKey;                      //handle of key
   int rc;

   chMessageFile[0] ='\0';         //initialize return

   //build the complete key --> first part + second part from input + third part from input
   pchKey = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, strlen(chKey) + strlen(pchSource) + 1 + strlen(pchSourceName) +1);
   sprintf(pchKey, "%s%s\\%s", chKey, pchSource, pchSourceName);

   if ( (rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     pchKey,
                     0,
                     KEY_QUERY_VALUE,
                     &hKey)) == ERROR_SUCCESS )
   {
      if ( (rc = RegQueryValueEx(hKey,                      // handle of key to query
                           pMessageFile,                  // address of name of value to query
                           NULL,                            // reserved
                           &valType,                      // address of buffer for value type
                           NULL,                            // NULL to get the size ...
                           &dataSize)) == ERROR_SUCCESS )// .. returned here
      {
         //no error getting the size of message file name, allocate buffer and get the value
         valData = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, dataSize);

         if ( (rc = RegQueryValueEx(hKey,                   // handle of key to query
                              pMessageFile,                 // address of name of value to query
                              NULL,                         // reserved
                              &valType,                     // address of buffer for value type
                              (LPBYTE)valData,              // address of buffer for value data
                              &dataSize)) == ERROR_SUCCESS )
         {
            ExpandEnvironmentStrings(valData,chMessageFile,MAX_PATH);
         }
         GlobalFree(valData);
      }
   }
   GlobalFree(pchKey);
}

//-----------------------------------------------------------------------------
//
//  Function SearchMessage
//
//  Load message files.
//  Search the message in message files, format and return the message
//
//  BOOL SearchMessage(char * chFileNames, DWORD dwMessageID, char ** pchInsertArray, LPVOID *lpMsgBuf )
//
//       chFileNames    - (in) filenames, separated by semicolons
//       dwMessageID    - (in) message id to be searched
//       pchInsertArray - (in) array with replacement strings for message formatting
//       lpMsgBuf       - (out) buffer containing the formatted message
//
//  return : 1 - message found and returned in lpMsgBuf
//           0 -  message not found or other error
//-----------------------------------------------------------------------------

BOOL SearchMessage(char * chFileNames, DWORD dwMessageID, char ** pchInsertArray, LPVOID *lpMsgBuf )
{
   HINSTANCE hInstance = 0;   //return value
   char *    pBuffer=0;       //buffer where chFileNames is copied to
   char *    pTemp=0;         //pointers for parsing ...
   char *    pNext=0;         //the file names
   int       rc;
   BOOL      fRc=0;

   if (chFileNames[0] != '\0')
   {
      pBuffer = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, strlen(chFileNames)+1);
      strcpy(pBuffer,chFileNames);
      pTemp = pBuffer;

      while (pTemp != 0 && fRc != 1)
      {
         pNext  = strchr(pTemp,';');
         if (pNext != NULL )
         {
            *pNext = '\0';
            pNext++;
         }

         if ( (hInstance = LoadLibrary(pTemp)) != 0 )
         {
            rc = FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  hInstance,                                 //message file handle
                  dwMessageID,                               //message id
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  (LPTSTR)lpMsgBuf,                          //output for formatted message
                  0,                                         //not needed for FORMAT_MESSAGE_ALLOCATE_BUFFER
                  pchInsertArray );                          //replacement strings

            if ( rc != 0 ) //no error
               fRc = 1;

            FreeLibrary(hInstance);
            hInstance = 0;
         }
         pTemp = pNext;
      }
      if (pBuffer)
         GlobalFree(pBuffer);
   }
   return fRc;
}

//-----------------------------------------------------------------------------
//
//  Function BuildMessage
//
//  Extracts the user name from an event log record
//
//  void BuildMessager(PEVENTLOGRECORD pEvLogRecord, char * pchSource, char ** pMessage)
//
//       pEvLogRecord - (in)event log record
//       pchSource    - (in)source within event log record
//       pMessage     - (out)formatted message
//
//-----------------------------------------------------------------------------
void BuildMessage(PEVENTLOGRECORD pEvLogRecord , const char * pchSource, char ** pMessage )
{
   char * pchString = (char*)0;      //pointer to replacement string within event log record
   int    iStringLen = 0;            //accumulation of the string length
   int    i, rc;

   HINSTANCE hInstance=0;             //handle for message files
   LPVOID    lpMsgBuf=0;              //buffer to format the string
   char      *pchInsertArray[100];    //pointer array to replacement strings
   char      chMessageFile[MAX_PATH]; //name of message file
   char      *pchPercent;             //pointer to "%%" within a replacement string

   //initialize insert array
   memset(pchInsertArray,0,sizeof(pchInsertArray));

   //point to first replacement string
   pchString = (char*)pEvLogRecord + pEvLogRecord->StringOffset;

   //loop over all replacement strings, substitute "%%" and build an array of replacement strings
   for (i=0; i < pEvLogRecord->NumStrings ; i++)
   {
      //If a replacement string contains "%%", the name of the parameter message file must be
      //read from the registry log. "%%" is followed by an id which is the id of parameter strings
      //to be loaded from the parameter message file
      if ( (pchPercent = strstr(pchString,"%%")) )
      {
         //the replacement strings contains placeholder for parameters
         //get name of parameter message file from registry
            GetEvPaMessageFile("ParameterMessageFile", const_cast<char *>(pchSource), (char *)pEvLogRecord + sizeof(EVENTLOGRECORD),chMessageFile);

         //load strings from the parameter message file
         if ( (hInstance = LoadLibrary(chMessageFile)) != 0 )
         {
            //load the parameters to be inserted into the replacement string
            rc = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                                hInstance,                                 //handle to message file
                                atoi(pchPercent+2),                        //id of message to be loaded
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                (LPTSTR)&pchInsertArray[i],
                                0,
                                0 );

            if (rc == 0) //error getting strings
            {
               //use empty string
               pchInsertArray[i] = (char *)LocalAlloc(LMEM_FIXED,1);
               *pchInsertArray[i] ='\0';
            }
            FreeLibrary(hInstance);
            hInstance =0;
         }
         else
         {
            //parameter message file could not be loaded, use empty string
            pchInsertArray[i] = (char *)LocalAlloc(LMEM_FIXED,1);
            *pchInsertArray[i] ='\0';
         }
         //accumulate sum of string length
         iStringLen += (int)strlen(pchInsertArray[i])+1;
      }
      else
      {
         //the replacement strings contains NO placeholder for parameters, use them as is
         iStringLen += (int)strlen(pchString)+1;
         pchInsertArray[i] = (char *)LocalAlloc(LMEM_FIXED,strlen(pchString)+1);
         strcpy(pchInsertArray[i],pchString);
      }
      //point to next replacement string within log record
      pchString += strlen(pchString)+1;
   }

   //get name of event log  message file from registry
    GetEvPaMessageFile("EventMessageFile",const_cast<char *>(pchSource),(char *)pEvLogRecord + sizeof(EVENTLOGRECORD),chMessageFile);

   //Search the message in the all event message files
   rc = SearchMessage(chMessageFile,pEvLogRecord->EventID,pchInsertArray,&lpMsgBuf);

   if (rc == 0) //no message found or any other error
   {
      //use replacement strings for the message if available
      if (pEvLogRecord->NumStrings)
      {
         *pMessage = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, iStringLen+1);

         for (i=0;i < pEvLogRecord->NumStrings;i++)
         {
            strcat(*pMessage,pchInsertArray[i]);
            strcat(*pMessage," ");
         }
      }
      else //return empty string
      {
         *pMessage    = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, 1);
         *pMessage[0] = '\0';
      }
   }
   else
   {
      //message formatted return it
      *pMessage = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, strlen((char*)lpMsgBuf) + 1);
      strcpy(*pMessage, (const char *)lpMsgBuf);
      //lpMsgBuf must be given free with LocalFree
      LocalFree(lpMsgBuf);
   }

   //free replacement strings
   for (i=0; i < pEvLogRecord->NumStrings ; i++)
   {
      LocalFree(pchInsertArray[i]);
   }
}

//-----------------------------------------------------------------------------
//
//  Function GetUser
//
//  Extracts the user name from an event log record
//
//  char * GetUser(PEVENTLOGRECORD pEvLogRecord)
//
//       pEvLogRecord - (in)pointer to event log record
//
//  return  char * - pointer to user name (account) from event log record or
//                   pointer to "N/A" when no user available or in error case
//                   (pointer is allocated from heap)
//                   must be freed vy caller
//
//-----------------------------------------------------------------------------
char * GetUser(PEVENTLOGRECORD pEvLogRecord)
{
   SID   *psid;                           //points to security structure withi event log record
   char * pchUserId=0;                    //returned userid, from heap, chDefUserId in any error case
                                          //or when no userid available
   DWORD  sizeId=0;                        //0 to get the size of the uder id
   char   chDefUserId[] = "N/A";          //default userid
   int    rc;
   SID_NAME_USE strDummy;                 //dummies needed for call to ...
   char  *pchDummy=0;                     //LookupAccontSid ...
   DWORD sizeDummy=0;                     //but not subject of interest


   if (pEvLogRecord->UserSidLength != 0) //no SID record avaialable return default userid
   {
      //get the SID record within event log record
      psid = (SID *)((char*)pEvLogRecord + pEvLogRecord->UserSidOffset);

      //get the size of the pchUserIid and pchDummy
      rc = LookupAccountSid(NULL,        // address of string for system name
                            psid,        // address of security identifier
                            pchUserId,   // address of string for account name
                            &sizeId,     // address of size account string
                            pchDummy,    // address of string for referenced domain (not needed)
                            &sizeDummy,  // address of size domain string
                            &strDummy ); // address of structure for SID type
      if (rc == 0) //error
      {
         if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) //expected because sizes are set to 0
         {
            pchUserId = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, max(sizeId,(DWORD)strlen(chDefUserId)+1));
            pchDummy = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeDummy);
            //now get it ...
            rc = LookupAccountSid(NULL,                    // address of string for system name
                                  psid,                    // address of security identifier
                                  pchUserId,               // address of string for account name
                                  &sizeId,                 // address of size account string
                                  pchDummy,                // address of string for referenced domain
                                  &sizeDummy,              // address of size domain string
                                  &strDummy );                 // address of structure for SID type
            GlobalFree(pchDummy); //don't want this
            //error: return default
            if (rc == 0) strcpy(pchUserId,chDefUserId);
         }
      }
   }

   //if not already set return default
   if (pchUserId == (char*)0)
   {
      pchUserId = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, strlen(chDefUserId)+1);
      strcpy(pchUserId,chDefUserId);
   }
   return pchUserId;
}

//-----------------------------------------------------------------------------
//
//  Function GetEvData
//
//  Extracts the data  name from an event log record
//
//  char * GetEvData(PEVENTLOGRECORD pEvLogRecord, char ** pchData)
//
//       pEvLogRecord - (in)pointer to event log record
//
//       pchData      - (in/out) data, allocated from heap
//                               empty string when no data
//
//-----------------------------------------------------------------------------
void GetEvData(PEVENTLOGRECORD pEvLogRecord, char ** pchData)
{
    char * puchAct;
    char            pTemp[3];
    DWORD           i;

    if (pEvLogRecord->DataLength != 0)
    {
       *pchData = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,(pEvLogRecord->DataLength + 1) * 2);

       puchAct =  (char*)pEvLogRecord + pEvLogRecord->DataOffset;

       for ( i=0; i<pEvLogRecord->DataLength; i++)
       {
         sprintf(pTemp, "%02x", *puchAct);
         strcat(*pchData, pTemp);
         puchAct++;
       }
    }
    else
    {
        *pchData = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, 1);
    }
}

//-----------------------------------------------------------------------------
//
// Function: WSEventLog
//
// This function reads the contents of the event log file. Each event log record is
// returned in a stem.# variable.
//
//                            0        1          2         3       4         5         6       7     8
// Syntax:   call WSEventLog( access, [options], [hEvent], [stem], [server], [source], [start],[num],[backupFileName]
//                            9     10        11  12    13
//                            type, category, id, data, strings
//                          )
//
// Params:
//
//   access - What to do with the event log
//            'OPEN'    returns event log handle
//                      0 in error case
//            'CLOSE'
//            'READ'    returns a stem containing the records
//            'NUM'     returns number of record or 0 if no records available or error
//            'CLEAR"   clear an backup the event log ........................
//
//   options - 'FORWARDS'       - The log is read in forward chronological order. (default)
//                              (oldest first)
//             'BACKWARDS'      - The log is read in backward chronological order.
//                              (youngest first)
//
//   hEvent -  eventlog handle, returned when eventlog is opened before
//
//   stem      Name of stem variable to store results in.
//             stem.0 contains the number of returned records
//
//   server -  Universal Naming Convention (UNC) name of the server on which the
//             event log is to be opened. If this parameter is not spcified,
//             the log is opened on the local computer.
//
//   source -  Name of the source. The source name must be a subkey of a logfile
//             entry under the EventLog key in the registry.
//             'System'       - system log
//             'Security'     - security log
//             'Application'  - application log (default if source is empty or when source not found
//
//   start  -  Record number of event log record to start. The oldest record is always the first record !!!
//             (independent if BACKEWRDS or FORWARDS is specified)
//
//   num  -    Number of event log record to read
//
//   backupFileName - if specified togetherr with 'CLEAR' the event log is backed up
//                    before it is cleared
//
//-----------------------------------------------------------------------------

size_t RexxEntry WSEventLog(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HANDLE hEventLog=0;                  //handle of event log
    const char * pchServer = NULL;       //server name, must be NULL if local
    const char * pchSource = NULL;       //source from input, or default
    const char * pchBackupFileName = NULL;     //name of the backup file name for CLEAR
    char   chDefSource[] = {"Application"}; //default source if no input
    char   varName[MAX_VARNAME];         //name of stem variable
    size_t stemLen;                      //length of stem vaiable
    char   access;                       //what to do with the log
    DWORD  dwReadFlags;                  //read flags
    LONG   rc = 0;                       //return code
    LONG   rcGetLastError = 0;           //rc from GetLastError
    DWORD  bufSize = 1024;               //initial size for event log buffer
    ULONG  start,num;                    //start Record number and number of of event log record to read
    DWORD numEvRecords;                  //number of all record within a event log
    SHVBLOCK shvb;                       //shared variable block



    //check parameter access(required); GET_ACESSS returns when invcalid access
    if (argc >= 1 && argv[0].strlength != 0)
    {
        GET_ACCESS(argv[0].strptr,access);
        // in case of write, a variable number of arguments (strings at the end) are possible
        if ( access != 'W' )
        {
            CHECKARG(1,14); //minimum of one and maximum of 14 parameters allowed
        }
    }
    else
    {
        RETERR;
    }

    if (access == 'R') //read access
    {
        dwReadFlags = EVENTLOG_FORWARDS_READ; //default
        //check parameter options
        if (argv[1].strlength != 0)
        {
            if (stricmp(argv[1].strptr,"FORWARDS"))
            {
                if (!stricmp(argv[1].strptr,"BACKWARDS"))
                {
                    dwReadFlags = EVENTLOG_BACKWARDS_READ;
                }
                else
                {
                    RETERR;
                }
            }
        }

        //preset defaults for start record and number of records to read
        start = 1;
        num   = 0xffffffff;
        bufSize = 64 * bufSize; //assume that when complete log should be read (64K)

        //check parameter start and num
        if (argc >= 7 )
        {
            if (argc >= 8 )
            {
                //start and num are available
                if ( (argv[6].strlength != 0) && (argv[7].strlength != 0) )
                {
                    //start and num contains values
                    dwReadFlags |= EVENTLOG_SEEK_READ;
                    start = atoi(argv[6].strptr);
                    num   = atoi(argv[7].strptr);
                    bufSize = num * bufSize;
                }
                else
                {
                    dwReadFlags |= EVENTLOG_SEQUENTIAL_READ;
                }
            }
            else //only start available, num is then required
            {
                RETERR;
            }
        }
        else
        {
            dwReadFlags |= EVENTLOG_SEQUENTIAL_READ;
        }

        //check parameter hEventLog (initialized with 0 means that event log will be opened)
        if ( argc >= 3 )
        {
            if (argv[2].strlength != 0)
            {
                GET_HANDLE(argv[2].strptr, hEventLog);
            }
        }

        //check parameter stem, add dot if necesssary
        if (argc >= 4 )
        {
            if ( RXVALIDSTRING(argv[3]) )
            {
                strcpy(varName, argv[3].strptr);
                memupper(varName, strlen(varName));
                stemLen = argv[3].strlength;
                if (varName[stemLen-1] != '.')
                {
                    varName[stemLen++] = '.';
                }
            }
            else
            {
                RETERR;
            }
        }
    }
    //check parameter server
    //if no server, use local machine, 0 pointer required !
    if ( argc >= 5 )
    {
        if (argv[4].strlength != 0)
        {
            pchServer = argv[4].strptr;
        }
    }

    //check parameter source, if empty use default source "Application"
    if ( (argc >= 6) &&  (argv[5].strlength != 0) )
    {
        pchSource = argv[5].strptr;
    }
    else
    {
        pchSource = chDefSource;
    }

    //open
    if (access == 'O')
    {
        if ( (hEventLog = OpenEventLog(pchServer,pchSource) ) != 0 )
        {
            RET_HANDLE(hEventLog);
        }
        else
        {
            RETC(0);
        }
    }
    //close
    if (access == 'C')
    {
        if (argc >= 3 )
        {
            if (argv[2].strlength != 0)
            {
                GET_HANDLE(argv[2].strptr, hEventLog);
            }
            else
            {
                RETERR;
            }
            if (CloseEventLog(hEventLog))
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
            RETERR;
        }
    }
    //NUM or CLEAR (and backup)
    if (access == 'N' || access == 'L')
    {
        //check parameter hEventLog (initialized with 0 means that event log must be opened)
        if ( argc >= 3 )
        {
            if (argv[2].strlength != 0)
            {
                GET_HANDLE(argv[2].strptr, hEventLog);
            }
        }

        if (access == 'L')
        {
            //check parameter BackupFileName
            //if no BackupFileName, no backup, 0 pointer required !, pchBackupFileName is initialized with NULL)
            if ( argc >= 9 )
            {
                if (argv[8].strlength != 0)
                {
                    pchBackupFileName = argv[8].strptr;
                }
            }
        }

        //no handle as input, so the event log must be opened
        if (hEventLog == 0)
        {
            if ( (hEventLog = OpenEventLog(pchServer,pchSource)) == 0 )
            {
                RETVAL(GetLastError());
            }
            else
            {
                if (access == 'N')
                {
                    rc = GetNumberOfEventLogRecords(hEventLog,&numEvRecords);
                }
                else
                {
                    rc = ClearEventLog(hEventLog,pchBackupFileName);
                }

                CloseEventLog(hEventLog);

                if (access == 'N')
                {
                    if (rc)
                    {
                        RETVAL((INT)numEvRecords);
                    }
                    else
                    {
                        RETC(0)
                    }
                }
                else
                {
                    if (rc)
                    {
                        RETC(0);
                    }
                    else
                    {
                        RETVAL(GetLastError());
                    }
                }
            }
        }
        else
        {
            if (access == 'N')
            {
                if ( GetNumberOfEventLogRecords(hEventLog,&numEvRecords) )
                {
                    RETVAL((INT)numEvRecords);
                }
                else
                {
                    RETC(0);
                }
            }
            else
            {
                rc = ClearEventLog(hEventLog,pchBackupFileName);
                if (rc)
                {
                    RETC(0);
                }
                else
                {
                    RETVAL(GetLastError());
                }
            }
        }
    }

    if (access == 'W')
    {
        WORD     wEventType       = 1;
        WORD     wEventCategory   = 0;
        DWORD    dwEventId        = 0;
        char    *lpData           = NULL;
        DWORD    dwDataSize       = 0;
        WORD     wNumStrings      = 0;
        const char *lpStrings[100];

        //check and prepare event log data

        // check the event type (initialized to 1 = default)
        // EVENTLOG_SUCCESS                   0X0000
        // EVENTLOG_ERROR_TYPE                0x0001
        // EVENTLOG_WARNING_TYPE              0x0002
        // EVENTLOG_INFORMATION_TYPE         0x0004
        // EVENTLOG_AUDIT_SUCCESS            0x0008
        // EVENTLOG_AUDIT_FAILURE           0x0010
        if ( argc >= 10 && argv[9].strlength != 0 )
        {
            wEventType = atoi(argv[9].strptr);
        }
        // check the event category (initialized to 0 = default)
        if ( argc >= 11 && argv[10].strlength != 0 )
        {
            wEventCategory = atoi(argv[10].strptr);
        }
        // check the event id (initialized to 0 = default)
        if ( argc >= 12 && argv[11].strlength != 0 )
        {
            dwEventId = atoi(argv[11].strptr);
        }
        // check the event data (initialized to NULL = default)
        if ( argc >= 13 && argv[12].strlength != 0 )
        {
            lpData = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,argv[12].strlength+1);
            sprintf(lpData, "%s", argv[12].strptr);
            dwDataSize = (DWORD)argv[12].strlength;
        }
        // check the event strings (initialized to NULL = default)
        if ( argc >= 14 && argv[13].strlength != 0 )
        {
            int   j=0;
            ULONG i;
            for ( i = 13; i < argc; i++ )
            {
                //count the number of strings and create the string array
                wNumStrings++;
                lpStrings[j] = argv[i].strptr;
                j++;
            }
        }
        //register the event source
        if ( (hEventLog = RegisterEventSource( pchServer, pchSource)) == NULL )
        {
            rcGetLastError = GetLastError();
        }

        if ( !rcGetLastError )
        {
            //write the log
            if ( ReportEvent( hEventLog, wEventType, wEventCategory, dwEventId, NULL, wNumStrings, dwDataSize, lpStrings, lpData) == NULL )
            {
                rcGetLastError = GetLastError();
            }

            //deregister the event source
            DeregisterEventSource( hEventLog);

            //free buffer for data
            if ( lpData != NULL )
            {
                GlobalFree(lpData);
            }
        }

        if ( rcGetLastError )
        {
            RETVAL(rcGetLastError);
        }
        else
        {
            RETC(0);
        }
    }

    //read
    if (access == 'R')
    {
        PEVENTLOGRECORD pEvLogRecord;        //pointer to one event record
        PVOID  pBuffer = 0;                  //buffer for event records
        DWORD  pnBytesRead;                  //returned from ReadEventlog
        DWORD  pnMinNumberOfBytesNeeded;     //returned from ReadEventlog if not all read
        char * pchEventSource  = (char *)0;  //source from event log record
        char * pchComputerName = (char *)0;  //computer name of event
        char * pchMessage=0;                 //text of detail string
        char * pchData=0;                    //data of event log record
        char   time[MAX_TIME_DATE];          //converted time of event
        char   date[MAX_TIME_DATE];          //converted date of event
        struct tm * DateTime;                //converted from elapsed seconds
        char * pchStrBuf = (char *)0;        //temp buffer for event string
        DWORD  bufferPos=0;                  //position within the eventlog buffer
        BOOL   cont = 1;                     //continue flag for while loop
        char   evType[5][12]={"Error","Warning","Information","Success","Failure"}; //event type strings
        int    evTypeIndex;
        char * pchUser;
        ULONG  count;                        //number of event log records retuned in stem

        //no handle as input, so the event log must be opened
        if (hEventLog == 0)
        {
            if ( (hEventLog = OpenEventLog(pchServer,pchSource)) == 0 )
            {
                RETVAL(GetLastError());
            }
        }

        GetNumberOfEventLogRecords(hEventLog,&numEvRecords);
        if (start > numEvRecords)
        {
            RETC(1);
        }

        pBuffer = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, bufSize);
        count = 0;

        while (cont)
        {
            while ( (rc = ReadEventLog(hEventLog, dwReadFlags, start, pBuffer, bufSize, &pnBytesRead, &pnMinNumberOfBytesNeeded) ) && (count < num) )
            {
                pEvLogRecord = (PEVENTLOGRECORD)pBuffer;
                bufferPos = 0;

                while ( (bufferPos < pnBytesRead) && (count < num) )
                {
                    if (dwReadFlags & EVENTLOG_FORWARDS_READ)
                    {
                        start = pEvLogRecord->RecordNumber+1;
                    }
                    else
                    {
                        start = pEvLogRecord->RecordNumber-1;
                    }
                    count++; //count number of events

                    //get index to event type string
                    GET_TYPE_INDEX(pEvLogRecord->EventType, evTypeIndex);

                    //get time and date
                    //DateTime = localtime(&pEvLogRecord->TimeGenerated);   // convert to local time
                    DateTime = localtime((const time_t *)&pEvLogRecord->TimeWritten);   // convert to local time
                    strftime(date, MAX_TIME_DATE,"%x", DateTime);
                    strftime(time, MAX_TIME_DATE,"%X", DateTime);

                    pchEventSource  = (char*)pEvLogRecord + sizeof(EVENTLOGRECORD);
                    pchComputerName = pchEventSource + strlen(pchEventSource)+1;

                    pchUser = GetUser(pEvLogRecord);

                    BuildMessage(pEvLogRecord,pchSource,&pchMessage);

                    GetEvData(pEvLogRecord,&pchData);

                    pchStrBuf = (char *)GlobalAlloc(GMEM_FIXED,
                                            strlen(evType[evTypeIndex])+1+
                                            strlen(date)+1+
                                            strlen(time)+1+
                                            strlen(pchEventSource)+3+
                                            12+
                                            strlen(pchUser)+1+
                                            strlen(pchComputerName)+1+
                                            strlen(pchMessage)+3+
                                            strlen(pchData)+3+10);

                    //Type Date Time Source Id User Computer Details
                    sprintf( pchStrBuf,
                             "%s %s %s '%s' %u %s %s '%s' '%s'",
                             evType[evTypeIndex],
                             date,
                             time,
                             pchEventSource,
                             LOWORD(pEvLogRecord->EventID),
                             pchUser,
                             pchComputerName,
                             pchMessage,
                             pchData);

                    GlobalFree(pchMessage);
                    GlobalFree(pchData);
                    pchMessage = 0;
                    GlobalFree(pchUser);

                    //write record to to stem
                    ltoa(count, varName+stemLen, 10);

                    shvb.shvnext            = NULL;
                    shvb.shvname.strptr     = varName;
                    shvb.shvname.strlength  = strlen(varName);
                    shvb.shvvalue.strptr    = pchStrBuf;
                    shvb.shvvalue.strlength = strlen(pchStrBuf);
                    shvb.shvnamelen         = shvb.shvname.strlength;
                    shvb.shvvaluelen        = shvb.shvvalue.strlength;
                    shvb.shvcode            = RXSHV_SET;
                    shvb.shvret             = 0;
                    rc = RexxVariablePool(&shvb);
                    GlobalFree(pchStrBuf);
                    if (rc == RXSHV_BADN)
                    {
                        if (pBuffer) GlobalFree(pBuffer);
                        return 2;
                    }
                    bufferPos += pEvLogRecord->Length;

                    //position to next event record
                    pEvLogRecord = (PEVENTLOGRECORD)(((char*)pEvLogRecord) + pEvLogRecord->Length);
                }
                memset(pBuffer,0,bufSize);
            }
            rc = GetLastError();
            // if buffer is to small, reallocate it, pnMinNumberOfBytesNeeded is returned from ReadEventLog

            if ( (count == num) || (start > numEvRecords) || (start < 1) )
            {
                cont = 0;
            }
            else
            {
                if (rc == ERROR_INSUFFICIENT_BUFFER)
                {
                    bufSize += pnMinNumberOfBytesNeeded;
                    pBuffer = GlobalReAlloc(pBuffer,bufSize,GMEM_FIXED|GMEM_ZEROINIT);
                }
                else
                {
                    cont = 0; //stop reading, reached end or error (except ERROR_INSUFFICIENT_BUFFER)
                }
            }
        }//while (cont)

        //close event log only if it was not opened before (imnplicit open during read)
        if ( atoi(argv[2].strptr) == 0 )
        {
            CloseEventLog(hEventLog);
        }

        if (pBuffer) GlobalFree(pBuffer);

        if ( rc == ERROR_HANDLE_EOF || count == num || (start > numEvRecords) || (start < 1) ) //all OK complete log read, also true for an empty log
        {
            char   strBuffer[8];                 //string for number of records read

            itoa(0, varName+stemLen, 10);
            itoa(count, strBuffer, 10);
            SET_VARIABLE(varName, strBuffer, 2);
            RETC(0)
        }
        else
        {
            RETVAL(rc);
        }
    }
    RETERR;
}

size_t RexxEntry WSCtrlWindow(const char *funcname, size_t argc, CONSTRXSTRING argv[], const char *qname, PRXSTRING retstr)
{
    HWND       hW;
    HWND       thW;
    char      *tmp_ptr;
    DWORD_PTR  dwResult;
    LRESULT    lResult;

    CHECKARG(1,10);

    if (!strcmp(argv[0].strptr,"FIND"))
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
        CHECKARG(2,2);

        HMENU hMenu;
        GET_HANDLE(argv[1].strptr, hMenu);
        RETVAL(IsMenu(hMenu));
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


// now build the actual entry list
RexxRoutineEntry rxwinsys_functions[] =
{
    REXX_CLASSIC_ROUTINE(WSRegistryKey,    WSRegistryKey),
    REXX_CLASSIC_ROUTINE(WSRegistryValue,  WSRegistryValue),
    REXX_CLASSIC_ROUTINE(WSRegistryFile,   WSRegistryFile),
    REXX_CLASSIC_ROUTINE(WSProgManager,    WSProgManager),
    REXX_CLASSIC_ROUTINE(WSEventLog,       WSEventLog),
    REXX_CLASSIC_ROUTINE(WSCtrlWindow,     WSCtrlWindow),
    REXX_CLASSIC_ROUTINE(WSCtrlSend,       WSCtrlSend),
    REXX_CLASSIC_ROUTINE(WSCtrlMenu,       WSCtrlMenu),
    REXX_CLASSIC_ROUTINE(WSClipboard,      WSClipboard),
    REXX_LAST_ROUTINE()
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
    NULL                                 // no methods in this package
};

// package loading stub.
OOREXX_GET_PACKAGE(rxwinsys);

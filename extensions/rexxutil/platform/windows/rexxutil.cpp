/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Windows Support                                          rexxutil.c   */
/*                                                                            */
/* Windows system utility functions                                           */
/*                                                                            */
/******************************************************************************/
/**********************************************************************
*                                                                     *
*   This program extends the REXX language by providing many          *
*   REXX external functions.                                          *
*   These functions are:                                              *
*       SysCls              -- Clear the screen in an OS/2 fullscreen *
*                              or windowed command prompt session.    *
*       SysCurPos           -- Set and/or Query the cursor position   *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysCurState         -- Make the cursor visible or invisible   *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysDriveInfo        -- Returns information about a specific   *
*                              drive.                                 *
*       SysDriveMap         -- Returns a list of the drives on the    *
*                              machine                                *
*       SysDropFuncs        -- Makes all functions in this package    *
*                              unknown to REXX.                       *
*       SysFileDelete       -- Deletes a file                         *
*       SysFileSearch       -- Searches for a file matching a given   *
*                              filespec.                              *
*       SysFileTree         -- Searches for files matching a given    *
*                              filespec, including files in           *
*                              subdirectories.                        *
*       SysGetKey           -- Returns one by of data indicating the  *
*                              key which was pressed,                 *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysIni              -- Reads and/or updates entries in .INI   *
*                              files.                                 *
*       SysLoadFuncs        -- Makes all functions in this package    *
*                              known to REXX so REXX programs may     *
*                              call them.                             *
*       SysMkDir            -- Creates a directory                    *
*       SysWinVer           -- Returns the Win OS and Version number  *
*       SysVersion          -- Returns the OS and Version number      *
*       SysRmDir            -- Removes a directory                    *
*       SysSearchPath       -- Searches throught a specified path     *
*                              for a file.                            *
*       SysSleep            -- Suspends execution for a number of     *
*                              seconds and milliseconds.              *
*       SysTempFilename     -- Creates a unique filename              *
*       SysTextScreenRead   -- Reads characters from the screen,      *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysTextScreenSize   -- Returns the size of the window in      *
*                              rows and columns,                      *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysWaitNamedPipe    -- Wait on a named pipe.                  *
*       SysRegisterObjectClass -- Register a new object class         *
*       SysDeregisterObjectClass -- Remove class registration         *
*       SysQueryClassList   -- Get list of registered classes         *
*       SysCreateObject     -- Create an object instance              *
*       SysDestroyObject    -- Delete an object instance              *
*       SysSetObjectData    -- Change object settings data            *
*       SysBootDrive        -- Return the windows boot drive          *
*       SysSystemDirectory  -- Return the Windows system directory    *
*       SysQueryEAList      -- Return list of file EA names           *
*       SysWildCard         -- Perform file wild card editting        *
*       SysFileSystemType   -- Return drive file system type          *
*       SysVolumeLabel      -- Return the drive label                 *
*       SysAddFileHandle    -- Add file handles to a process          *
*       SysSetFileHandle    -- Set file handles for a process         *
*       SysCreateMutexSem   -- Create a Mutex semaphore               *
*       SysOpenMutexSem     -- Open a Mutex semaphore                 *
*       SysCloseMutexSem    -- Close a Mutex semaphore                *
*       SysRequestMutexSem  -- Request a Mutex semaphore              *
*       SysReleaseMutexSem  -- Release a Mutex semaphore              *
*       SysCreateEventSem   -- Create an Event semaphore              *
*       SysOpenEventSem     -- Open an Event semaphore                *
*       SysCloseEventSem    -- Close an Event semaphore               *
*       SysPostEventSem     -- Post an Event semaphore                *
*       SysPulseEventSem    -- Post and reset an Event semaphore      *
*       SysResetEventSem    -- Reset an Event semaphore               *
*       SysWaitEventSem     -- Wait on an Event semaphore             *
*       SysProcessType      -- Return type of process                 *
*       SysSetPriority      -- Set current thread priority            *
*       SysGetCollate       -- Get country/codepage collating sequence*
*       SysNationalLanguageCompare -- NLS strict compare              *
*       SysMapCase          -- NLS uppercasing                        *
*       SysSetProcessCodePage -- Set current code page                *
*       SysQueryProcessCodePage -- Get current code page              *
*       SysAddRexxMacro     -- Load program into macro space          *
*       SysDropRexxMacro    -- Drop program from macro space          *
*       SysReorderRexxMacro -- Reorder program in macro space         *
*       SysQueryRexxMacro   -- Query ordering of macro space program  *
*       SysClearRexxMacroSpace -- Remove all programs from macro space*
*       SysLoadRexxMacroSpace  -- Load a Rexx macro space             *
*       SysSaveRexxMacroSpace  -- Save a Rexx macro space             *
*       SysShutDownSystem   -- Shutdown the system                    *
*       SysSwitchSession    -- Switch to a named session              *
*       SysDropLibrary      -- Drop a function package                *
*       SysQueryProcess     -- Get information on current proc/thread *
*       SysDumpVariables    -- Dump current variables to a file       *
*       SysSetFileDateTime  -- Set the last modified date of a file   *
*       SysGetFileDateTime  -- Get the last modified date of a file   *
*       SysStemSort         -- sort a stem array                      *
*       SysStemDelete       -- delete items in a stem array           *
*       SysStemInsert       -- insert items into a stem array         *
*       SysStemCopy         -- copy items from one stem array to other*
*       SysUtilVersion      -- query version of REXXUTIL.DLL          *
*       SysWinFileEncrypt   -- Encrypt file on a W2K-NTFS             *
*       SysWinFileDecrypt   -- Decrypt file on a W2K-NTFS             *
*       SysGetErrortext     -- Retrieve textual desc. of error number *
*       SysWinGetDefaultPrinter -- retrieve default printer           *
*       SysWinGetPrinters   -- Obtain list of local printers          *
*       SysWinSetDefaultPrinter -- set the local default printer      *
*       SysFileCopy         -- Copy files on the file system          *
*       SysFileMove         -- Move / Rename files or directories     *
*       SysIsFile           -- Check for the existance of a file      *
*       SysIsFileDirectory  -- Check for the existance of a directory *
*       SysIsFileLink       -- Check for the existance of a link      *
*       SysIsFileCompressed   -- Check for a file to be compressed    *
*       SysIsFileEncrypted    -- Check for a file to be encrypted     *
*       SysIsFileNotContentIndexed -- Check if a file should be indexed *
*       SysIsFileOffline    -- Check if a file is offline             *
*       SysIsFileSparse     -- Check if a file is sparse              *
*       SysIsFileTemporary  -- Check if a file is temporary           *
*                                                                     *
**********************************************************************/

/* Include files */

#include "oorexxapi.h"
#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <limits.h>
#include <shlwapi.h>
#include <math.h>                      // isnan(), HUGE_VAL

#define OM_WAKEUP (WM_USER+10)
VOID CALLBACK SleepTimerProc( HWND, UINT, UINT, DWORD);

/*********************************************************************/
/*  Various definitions used by various functions.                   */
/*********************************************************************/

#define MAX_LABEL      13              /* max label length (sdrvinfo)*/
#define MAX_DIGITS     9               /* max digits in numeric arg  */
#define MAX            264             /* temporary buffer length    */
#define IBUF_LEN       4096            /* Input buffer length        */
#define MAX_READ       0x10000         /* full segment of buffer     */
#define CH_EOF         0x1A            /* end of file marker         */
#define CH_CR          '\r'            /* carriage return character  */
#define CH_NL          '\n'            /* new line character         */
#define AllocFlag      PAG_COMMIT | PAG_WRITE  /* for DosAllocMem    */
#define RNDFACTOR      1664525L
#define MAX_ENVVAR     1024
#define MAX_LINE_LEN   4096            /* max line length            */
#define MAX_CREATEPROCESS_CMDLINE (32 * 1024)

/*********************************************************************/
/*  Defines used by SysDriveMap                                      */
/*********************************************************************/

#define  USED           0
#define  FREE           1
#define  CDROM          2
#define  REMOTE         3
#define  LOCAL          4
#define  RAMDISK        5
#define  REMOVABLE      6

/*********************************************************************/
/* Defines uses by SysTree                                           */
/*********************************************************************/

#define  RECURSE        0x0002
#define  DO_DIRS        0x0004
#define  DO_FILES       0x0008
#define  NAME_ONLY      0x0010
#define  EDITABLE_TIME  0x0020
#define  LONG_TIME      0x0040   /* long time format for SysFileTree */
#define  CASELESS       0x0080
#define  RXIGNORE       2              /* Ignore attributes entirely */
#define  AllAtts        FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | \
FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED
#define  AllFiles       FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | \
FILE_SYSTEM | FILE_ARCHIVED
#define  AllDirs        FILE_READONLY | FILE_HIDDEN | \
FILE_SYSTEM | FILE_ARCHIVED | MUST_HAVE_DIRECTORY | FILE_DIRECTORY

/*********************************************************************/
/* Defines used by SysStemSort -- must match values in okstem.hpp    */
/*********************************************************************/
#define SORT_CASESENSITIVE 0
#define SORT_CASEIGNORE    1

#define SORT_ASCENDING 0
#define SORT_DECENDING 1

/*********************************************************************/
/* Define used for Unicode translation. Not present in early Windows */
/* SDKs.                                                             */
/*********************************************************************/
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS      0x00000080
#endif

// Defines for various SysFileTree buffer.
#define FNAMESPEC_BUF_EXTRA    8
#define FNAMESPEC_BUF_LEN      MAX_PATH + FNAMESPEC_BUF_EXTRA
#define FOUNDFILE_BUF_LEN      MAX_PATH
#define FILETIME_BUF_LEN       64
#define FILEATTR_BUF_LEN       16
#define FOUNDFILELINE_BUF_LEN  FOUNDFILE_BUF_LEN + FILETIME_BUF_LEN + FILEATTR_BUF_LEN


/*********************************************************************/
/* Structures used throughout REXXUTIL.C                             */
/*********************************************************************/

/*********************************************************************/
/* RxTree Structure used by GetLine, OpenFile and CloseFile          */
/*********************************************************************/
typedef struct _GetFileData {
  char *       buffer;                 /* file read buffer           */
  size_t       size;                   /* file size                  */
  size_t       data;                   /* data left in buffer        */
  size_t       residual;               /* size left to read          */
  char *       scan;                   /* current scan position      */
  HANDLE       handle;                 /* file handle                */
} GetFileData;


/*
 *  Data structure for SysFileTree.
 *
 *  Note that in Windows the MAX_PATH define includes the terminating null.
 */
typedef struct RxTreeData {
    size_t         count;                         // Number of found file lines
    RexxStemObject files;                         // Stem that holds results.
    char           fNameSpec[FNAMESPEC_BUF_LEN];  // File name portion of the search for file spec, may contain glob characters.
    char           foundFile[FOUNDFILE_BUF_LEN];  // Full path name of found file
    char           fileTime[FILETIME_BUF_LEN];    // Time and size of found file
    char           fileAttr[FILEATTR_BUF_LEN];    // File attribute string of found file
    char           foundFileLine[FOUNDFILELINE_BUF_LEN]; // Buffer for found file line, includes foundFile, fileTime, and fileAttr
    char          *dFNameSpec;                    // Starts out pointing at fNameSpec
    size_t         nFNameSpec;                    // CouNt of bytes in dFNameSpec buffer
} RXTREEDATA;

/*********************************************************************/
/* RxStemData                                                        */
/*   Structure which describes as generic                            */
/*   stem variable.                                                  */
/*********************************************************************/

typedef struct RxStemData {
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    char ibuf[IBUF_LEN];               /* Input buffer               */
    char varname[MAX];                 /* Buffer for the variable    */
                                       /* name                       */
    char stemname[MAX];                /* Buffer for the variable    */
                                       /* name                       */
    size_t stemlen;                    /* Length of stem.            */
    size_t vlen;                       /* Length of variable value   */
    size_t j;                          /* Temp counter               */
    size_t tlong;                      /* Temp counter               */
    size_t count;                      /* Number of elements         */
                                       /* processed                  */
} RXSTEMDATA;


/*********************************************************************/
/* Saved character status                                            */
/*********************************************************************/
static   int   ExtendedFlag = 0;       /* extended character saved   */
static   char  ExtendedChar;           /* saved extended character   */

/*********************************************************************/
/* function pointer for GetDiskFreespaceEx for SysDriveInfo          */
/*********************************************************************/
typedef  BOOL (WINAPI *P_GDFSE)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER,
                               PULARGE_INTEGER);
static   P_GDFSE pGetDiskFreeSpaceEx = NULL;

/*********************************************************************/
/* Numeric Error Return Strings                                      */
/*********************************************************************/

#define  NO_UTIL_ERROR    "0"          /* No error whatsoever        */
#define  ERROR_NOMEM      "2"          /* Insufficient memory        */
#define  ERROR_FILEOPEN   "3"          /* Error opening text file    */

/*********************************************************************/
/* Alpha Numeric Return Strings                                      */
/*********************************************************************/

#define  ERROR_RETSTR   "ERROR:"

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

/*********************************************************************/
/* Some useful macros                                                */
/*********************************************************************/

#define BUILDRXSTRING(t, s) { \
  strcpy((t)->strptr,(s));\
  (t)->strlength = strlen((s)); \
}

#define RETVAL(retc) { \
  retstr->strlength = strlen(itoa(retc, retstr->strptr,10)); \
  return VALID_ROUTINE; \
}

/*********************************************************************/
/****************  REXXUTIL Supporting Functions  ********************/
/****************  REXXUTIL Supporting Functions  ********************/
/****************  REXXUTIL Supporting Functions  ********************/
/*********************************************************************/

void inline outOfMemoryException(RexxThreadContext *c)
{
    c->RaiseException1(Rexx_Error_System_service_user_defined, c->String("failed to allocate memory"));
}

/**
 * <routineName> argument <argPos> must not be a null string
 *
 * SysFileTree argument 2 must not be a null string
 *
 * @param c      Threade context we are operating in.
 * @param fName  Routine name.
 * @param pos    Argument position.
 */
void inline nullStringException(RexxThreadContext *c, CSTRING fName, size_t pos)
{
    c->RaiseException2(Rexx_Error_Incorrect_call_null, c->String(fName), c->StringSize(pos));
}

inline void safeLocalFree(void *p)
{
    if (p != NULL)
    {
        LocalFree(p);
    }
}

/**
 * Raises an exception for an unrecoverable system API failure.
 *
 * @param c    Call context we are operating in.
 * @param api  System API name.
 * @param rc   Return code from calling the API.
 */
static void systemServiceExceptionCode(RexxThreadContext *c, CSTRING api, uint32_t rc)
{
    char buf[256] = {0};
    _snprintf(buf, sizeof(buf),
             "system API %s() failed; rc: %d last error code: %d", api, rc, GetLastError());

    c->RaiseException1(Rexx_Error_System_service_user_defined, c->String(buf));
}

/**
 * Tests if the the current operating system version meets the specified
 * requirements. Really a front end to VerifyVersionInfo().  See MSDN docs for
 * type and condition flags.
 *
 * @param major       OS major number.
 * @param minor       OS minor number.
 * @param sp          Service pack level.
 * @param type        Further refines the test.  See MSDN for all the flags, but
 *                    for example there is VER_NT_WORKSTATION to differentiate
 *                    between NT desktop and NT server.
 * @param condition   The test condition.  Typical flags would be VER_EQUAL or
 *                    VER_GREATER_EQUAL.
 *
 * @return True if the condition is met by the current operating system, or
 *         false if not.
 */
static bool isWindowsVersion(DWORD major, DWORD minor, unsigned int sp, unsigned int type, unsigned int condition)
{
    OSVERSIONINFOEX ver;
    DWORDLONG       mask = 0;
    DWORD           testForMask = VER_MAJORVERSION | VER_MINORVERSION;

    ZeroMemory(&ver, sizeof(OSVERSIONINFOEX));

    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    ver.dwMajorVersion = major;
    ver.dwMinorVersion = minor;

    VER_SET_CONDITION(mask, VER_MAJORVERSION, condition);
    VER_SET_CONDITION(mask, VER_MINORVERSION, condition);

    if ( condition != VER_EQUAL )
    {
        ver.wServicePackMajor = sp;
        testForMask |= VER_SERVICEPACKMAJOR;
        VER_SET_CONDITION(mask, VER_SERVICEPACKMAJOR, condition);
    }

    if ( type != 0 )
    {
        ver.wProductType = type;
        testForMask |= VER_PRODUCT_TYPE;
        VER_SET_CONDITION(mask, VER_PRODUCT_TYPE, condition);
    }

    if ( VerifyVersionInfo(&ver, testForMask, mask) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/********************************************************************
* Function:  string2size_t(string, number)                          *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns false if the number *
*            is not valid, true if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        true - Good number converted                           *
*            false - Invalid number supplied.                       *
*********************************************************************/
bool string2size_t(
    const char *string,                  /* string to convert          */
    size_t *number)                      /* converted number           */
{
    size_t   accumulator;                /* converted number           */
    size_t   length;                     /* length of number           */

    length = strlen(string);             /* get length of string       */
    if (length == 0 ||                   /* if null string             */
        length > MAX_DIGITS + 1)         /* or too long                */
    {
        return false;                    /* not valid                  */
    }

    accumulator = 0;                     /* start with zero            */

    while (length)                       /* while more digits          */
    {
        if (!isdigit(*string))             /* not a digit?               */
        {
            return false;                    /* tell caller                */
        }
                                             /* add to accumulator         */
        accumulator = accumulator * 10 + (*string - '0');
        length--;                          /* reduce length              */
        string++;                          /* step pointer               */
    }
    *number = accumulator;               /* return the value           */
    return true;                         /* good number                */
}

inline bool isAtLeastVista(void)
{
    return isWindowsVersion(6, 0, 0, 0, VER_GREATER_EQUAL);
}

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

bool ReadNextBuffer( GetFileData *filedata );

/********************************************************************
* Function:  OpenFile(file, filedata)                               *
*                                                                   *
* Purpose:   Prepares a file for reading.                           *
*                                                                   *
* RC:        0     - file was opened successfully                   *
*            1     - file open error occurred                       *
*********************************************************************/

bool MyOpenFile(
   const char  *file,                  /* file name                  */
   GetFileData *filedata )             /* global file information    */
{
   DWORD       dwSize;                 /* file status information    */

                                       /* try to open the file       */
  if ((filedata->handle = CreateFile(file, GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                            FILE_FLAG_WRITE_THROUGH, 0))
                            == INVALID_HANDLE_VALUE)
    return true;                          /* return failure             */

                                       /* retrieve the file size     */
  dwSize = GetFileSize(filedata->handle, NULL);
                                       /* if GetFileSize failed or   */
                                       /* size=0                     */
  if (dwSize == 0xffffffff || !dwSize) {
    CloseHandle(filedata->handle);     /* close the file             */
    return true;                       /* and quit                   */
  }
  if (dwSize <= MAX_READ) {            /* less than a single buffer  */
    DWORD bytesRead;
                                       /* allocate buffer for file   */
    if (!(filedata->buffer = (char *)GlobalAlloc(GMEM_ZEROINIT |
                                         GMEM_FIXED, dwSize))) {
      CloseHandle(filedata->handle);   /* close the file             */
      return true;
    }
    filedata->size = dwSize;           /* save file size             */
    filedata->residual = 0;            /* no left over information   */
                                       /* read the file in           */
    if (!ReadFile(filedata->handle, filedata->buffer, dwSize, &bytesRead, NULL)) {
      GlobalFree(filedata->buffer);    /* free the buffer            */
      CloseHandle(filedata->handle);   /* close the file             */
      return true;
    }

    // Set scan to beginning of buffer, and data to number of bytes we have.
    filedata->scan = filedata->buffer;
    filedata->data = bytesRead;
  }
  else {                               /* need to read partial       */
                                       /* allocate buffer for read   */
    if (!(filedata->buffer = (char *)GlobalAlloc(GMEM_ZEROINIT |
                                         GMEM_FIXED, MAX_READ))) {
      CloseHandle(filedata->handle);   /* close the file             */
      return true;
    }

    filedata->size = dwSize;           /* save file size             */
                                       /* and set remainder          */
    filedata->residual = filedata->size;
                                       /* read the file in           */
    if (ReadNextBuffer(filedata)) {
      GlobalFree(filedata->buffer);    /* free the buffer            */
      CloseHandle(filedata->handle);   /* close the file             */
      return true;
    }
  }
  return false;                        /* file is opened             */
}

/********************************************************************
* Function:  CloseFile(filedata)                                    *
*                                                                   *
* Purpose:   Close a file                                           *
*********************************************************************/
void CloseFile(
   GetFileData *filedata )             /* global file information    */
{
  CloseHandle(filedata->handle);       /* close the file             */
  GlobalFree(filedata->buffer);        /* free the buffer            */
}

/**
 * Reads the next buffer of data.
 *
 * @param filedata  Global file information.
 *
 * @return  0, buffer was read.  1, an error occurred reading buffer.
 */
bool ReadNextBuffer(GetFileData *filedata)
{
    size_t size;
    DWORD  bytesRead;

    /* get size of this read      */
    size = min(MAX_READ, filedata->residual);

    /* read the file in           */
    if ( !ReadFile(filedata->handle, filedata->buffer, (DWORD)size, &bytesRead, NULL) )
    {
        return 1;
    }
    filedata->data = bytesRead;

    if ( filedata->data != size )
    {
        // Read less than requested, no residual.
        filedata->residual = 0;            /* no residual                */
    }
    else
    {
        // Residual is remainder.
        filedata->residual = filedata->residual - size;
    }

    /* don't check for EOF but read to real end of file     */
    //                                     /* look for a EOF mark        */
    //endptr = memchr(filedata->buffer, CH_EOF, filedata->data);
    //
    //if (endptr) {                        /* found an EOF mark          */
    //                                     /* set new length             */
    //  filedata->data = (ULONG)(endptr - filedata->buffer);
    //  filedata->residual = 0;            /* no residual                */
    //}

    // Set position to beginning.
    filedata->scan = filedata->buffer;
    return 0;
}

/********************************************************************
* Function:  GetLine(line, size, filedata)                          *
*                                                                   *
* Purpose:   Reads a line of data using buffered reads.  At end of  *
*            file, zero is returned to indicate nothing left.       *
*                                                                   *
* RC:        true -  line was read successfully                     *
*            false - end of file was reached                        *
*********************************************************************/

bool GetLine(
   char        *line,                  /* returned line              */
   size_t       size,                  /* size of line buffer        */
   GetFileData *filedata )             /* file handle                */
{
   char        *scan;                  /* current scan pointer       */
   size_t       length;                /* line length                */
   size_t       copylength;            /* copied length              */


  if (!(filedata->data)) {             /* if out of current buffer   */
    if (filedata->residual) {          /* may be another buffer      */
      ReadNextBuffer(filedata);        /* try to read one            */
      if (!filedata->data)             /* nothing more?              */
        return true;                   /* all done                   */
    }
    else
      return true;                     /* return EOF condition       */
  }
                                       /* look for a carriage return */
  scan = (char *)memchr(filedata->scan, CH_NL, filedata->data);
  if (scan) {                          /* found one                  */
                                       /* calculate the length       */
    length = scan - filedata->scan;
    copylength = min(length, size);    /* get length to copy         */
                                       /* copy over the data         */
    memcpy(line, filedata->scan, copylength);
    line[copylength] = '\0';           /* make into ASCIIZ string    */

    /* we don't want the CR character in the result string*/
    if ( line[copylength - 1] == CH_CR ) {
      line[copylength - 1] = '\0';
    } /* endif */

    filedata->data -= length + 1;      /* reduce the length          */
    filedata->scan = scan + 1;         /* set new scan point         */

    if (!filedata->data) {             /* all used up                */
      if (filedata->residual)          /* more to read               */
        ReadNextBuffer(filedata);      /* read the next buffer       */
    }
    return false;                        /* this worked ok           */
  }
  else                                   /* ran off the end          */
  {
    /* now we have scanned the whole buffer, but didn't find LF.         */
    /* we have two situation that can appear:                            */
    /* 1.) size > filedata->data ==> there is still room in the working  */
    /*     buffer, we can see whether we have scanned the whole file     */
    /*     --> ReadNextBuffer, or this was it, and we return             */
    /* 2.) size < filedata->buffer ==> we have scanned to the end of the */
    /*     buffer, more than what would fit into it, but still we        */
    /*     haven't had a hit. So copy all elements into the buffer       */
    /*     read the next buffer, GetLine to get the next LF              */
    /*     and return what was put into buffer. Be ALWAYS AWARE that     */
    /*     that buffer limits to 2047 bytes, and that we only return up  */
    /*     to 2047 bytes of a line. The rest of the line is not returned */
    /*     and not checked for search argument. Nevertheless, this       */
    /*     garantees, that the line counter (argument 'N') corresponds   */
    /*     with the input file                                           */

                                       /* get length to copy         */
    if (size > filedata->data)
    {
       copylength = filedata->data;    /* copy the rest into linebuffer */
                                       /* copy over the data         */
       memcpy(line, filedata->scan, copylength);
       line[copylength] = '\0';          /* make into ASCIIZ string  */

     /* all data should be read, filedata->data must be zero         */
       filedata->data -= copylength;
     /* scan should be at the end                                    */
       filedata->scan += copylength;     /* set new scan point       */

    /* if no more data to read in the file, return OK     */
       if (!filedata->residual)
          return false;
       else
          return GetLine(line + copylength, size - copylength, filedata);
    }
    else        /* the line is full, scan until LF found but no copy */
    {
       copylength = min(size, filedata->data);
                                         /* copy over the data       */
       memcpy(line, filedata->scan, copylength);
       line[copylength] = '\0';          /* make into ASCIIZ string  */

       filedata->data  = 0;            /* no data in buffer          */
       filedata->scan += filedata->data; /* set scan point to end    */

       if (filedata->residual)         /* more to read               */
       {
           ReadNextBuffer(filedata);   /* read the next buffer       */
           return GetLine(line + copylength, 0, filedata);
       }
       else
          return false;
    }
  }
}

/********************************************************************
* Function:  SetFileMode(file, attributes)                          *
*                                                                   *
* Purpose:   Change file attribute bits                             *
*            without PM.                                            *
*                                                                   *
* RC:        0    -  File attributes successfully changed           *
*            1    -  Unable to change attributes                    *
*********************************************************************/
bool SetFileMode(
  const char *file,                    /* file name                  */
  size_t   attr )                      /* new file attributes        */
{

  DWORD         dwfileattrib;          /* file attributes            */

                                       /* get the file status        */
  if ((dwfileattrib = GetFileAttributes(file)) != 0xffffffff) {
                                       /* if worked                  */
                                       /* set the attributes         */
    if ((dwfileattrib = SetFileAttributes(file, (DWORD)attr)) != 0)
      return false;   /* give back success flag     */
    else
      return true;
  } else
    return true;
}

/********************************************************************
* Function:  mystrstr(haystack, needle, hlen, nlen, sensitive)      *
*                                                                   *
* Purpose:   Determines if the string 'needle' is in the            *
*            string 'haystack' by returning it's position or        *
*            a NULL if not found.  The length of haystack and       *
*            needle are given by 'hlen' and 'nlen' respectively.    *
*                                                                   *
*            If 'sensitive' is true, then the search is case        *
*            sensitive, else it is case insensitive.                *
*                                                                   *
* RC:        num  - The pos where needle was found.                 *
*            NULL - needle not found.                               *
*                                                                   *
* Used By:   SysFileSearch()                                        *
*********************************************************************/

char *mystrstr(
  const char *haystack,
  const char *needle,
  size_t  hlen,
  size_t  nlen,
  bool    sensitive)
{
// TODO:  This can be made a LOT more efficient

  char line[MAX_LINE_LEN];
  char target[MAX_LINE_LEN];
  size_t p;
 /* Copy line  - Change nulls to spaces and uppercase if needed      */

  for (p = 0; p < hlen; p++)
  {
    if (haystack[p] == '\0')
      line[p] = ' ';
    else if (sensitive)
      line[p] = haystack[p];
    else line[p] = (char)toupper(haystack[p]);
  }
  line[p] = '\0';

 /* Copy target  - Change nulls to spaces and uppercase if needed    */

  for (p = 0; p < nlen; p++) {

    if (needle[p] == '\0')
      target[p] = ' ';
    else if (sensitive)
      target[p] = needle[p];
    else target[p] = (char)toupper(needle[p]);
  }
  target[p] = '\0';

  return strstr(line, target);
}

const char *mystrstr(const char *haystack, const char *needle)
{
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(haystack);

    char *haystackCopy = strdup(haystack);
    for (size_t i = 0; i < hlen; i++)
    {
        haystackCopy[i] = toupper(haystackCopy[i]);
    }

    const char *result = strstr(haystackCopy, needle);
    free(haystackCopy);
    return result;
}

/****************************************************************
* Function: GetUniqueFileName(Template, Filler, file)           *
*                                                               *
* Purpose:  This function returns a unique temporary file name  *
*           given a template and a filler character.            *
*                                                               *
* Params:   CHAR* Template - The template.  Must contain at     *
*                            least one or more filler chars.    *
*                                                               *
*                            Example:  "C:\TEMP\FILE????.DAT    *
*                                                               *
*           CHAR Filler    - The character in the Template to   *
*                            be replaced with numbers.  For     *
*                            the above example, the filler char *
*                            would be '?'.                      *
*           CHAR* file     - file name produced (output)        *
*                                                               *
* Used By:  RxTempFileName()                                    *
****************************************************************/

VOID GetUniqueFileName(
  CHAR  *Template,
  CHAR   Filler,
  CHAR  *file)
{

  CHAR numstr[6];
  bool Unique = false;

  ULONG x,                             /* loop index                 */
        i,                             /*                            */
        j = 0,                         /* number of filler chars     */
                                       /* found                      */
        num,                           /* temporary random number    */
        start,                         /* first random number        */
        max = 1;                       /* maximum random number      */

  INT  seed;                           /* to get current time        */
  WIN32_FIND_DATA wfdFinfo;            /* Windows Find data struct   */
                                       /* Structure                  */
  SYSTEMTIME DT;                       /* The date and time structure*/
  UINT            fuErrorMode;         /* holds current file err mode*/
  HANDLE hSearch;                      /* handle of file if found    */

 /** Determine number of filler characters *                         */

  for (x = 0; Template[x] != 0; x++)

    if (Template[x] == Filler) {
      max = max *10;
      j++;
    }

 /** Return NULL string if less than 1 or greater than 4 *           */

  if (j == 0 || j > 5) {
    Unique = true;
    strcpy(file, "");
    return;
  }

 /** Get a random number in the appropriate range                    */

                                       /* Get the time               */
  GetSystemTime(&DT);                  /* via Windows                */

  seed = DT.wHour*60 + DT.wMinute;     /* convert to hundreths       */
  seed = seed*60 + DT.wSecond;
  seed = seed*100 + ( DT.wMilliseconds / (UINT)10 );
  seed = seed * RNDFACTOR + 1;
  num = (ULONG)seed % max;
  start = num;

 /** Do until a unique name is found                                 */

  while (!Unique) {

    /** Generate string which represents the number                  */

    switch (j) {
      case 1 :
        wsprintf(numstr, "%01u", num);
        break;
      case 2 :
        wsprintf(numstr, "%02u", num);
        break;
      case 3 :
        wsprintf(numstr, "%03u", num);
        break;
      case 4 :
        wsprintf(numstr, "%04u", num);
        break;
      case 5 :
        wsprintf(numstr, "%05u", num);
        break;
    }

    /** Subsitute filler characters with numeric string              */

    i = 0;

    for (x = 0; Template[x] != 0; x++)

      if (Template[x] == Filler)
        file[x] = numstr[i++];

      else
        file[x] = Template[x];
    file[x] = '\0';

    /** See if the file exists                                       */
                                       /* Disable Hard-Error popups  */
    fuErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hSearch = FindFirstFile(file, &wfdFinfo);

    if (hSearch == INVALID_HANDLE_VALUE)/* file not found?           */
      Unique = true;                   /* got one                    */

    FindClose(hSearch);
    SetErrorMode(fuErrorMode);         /* Enable previous setting    */

    /** Make sure we are not wasting our time                        */

    num = (num+1)%max;

    if (num == start && !Unique) {
      Unique = true;
      strcpy(file, "");
    }
  }
}

/**********************************************************************
***             <<<<<< REXXUTIL Functions Follow >>>>>>>            ***
***             <<<<<< REXXUTIL Functions Follow >>>>>>>            ***
***             <<<<<< REXXUTIL Functions Follow >>>>>>>            ***
***             <<<<<< REXXUTIL Functions Follow >>>>>>>            ***
**********************************************************************/
/**********************************************************************
* Function:  SysCls                                                   *
*                                                                     *
* Syntax:    call SysCls                                              *
*                                                                     *
* Return:    NO_UTIL_ERROR - Successful.                              *
**********************************************************************/

size_t RexxEntry SysCls(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  HANDLE hStdout;                      /* Handle to Standard Out     */
  DWORD dummy;
  COORD Home = {0, 0};                 /* Home coordinates on console*/
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */

  if (numargs)                         /* arguments specified?       */
      return INVALID_ROUTINE;          /* raise the error            */

  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                                       /* if in character mode       */
  if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) {
    FillConsoleOutputCharacter( hStdout, ' ',
                                csbiInfo.dwSize.X * csbiInfo.dwSize.Y,
                                Home, &dummy );
    SetConsoleCursorPosition(hStdout, Home);

  }
  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysCurPos - positions cursor in DOS window                  *
*                                                                        *
* Syntax:    call SysCurPos [row, col]                                   *
*                                                                        *
* Params:    row   - row to place cursor on                              *
*            col   - column to place cursor on                           *
*                                                                        *
* Return:    row, col                                                    *
*************************************************************************/

RexxRoutine2(RexxStringObject, SysCurPos, OPTIONAL_stringsize_t, inrow, OPTIONAL_stringsize_t, incol)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */


    if ((argumentExists(1) && argumentOmitted(2)) || (argumentExists(2) && argumentOmitted(1)))
    {
        context->InvalidRoutine();
        return NULL;
    }
    /* get handle to stdout       */
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    /* get current position, and  */
    /* continue only if in        */
    /* character mode             */
    if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
    {
        char buffer[256];

        sprintf(buffer, "%d %d", csbiInfo.dwCursorPosition.Y, csbiInfo.dwCursorPosition.X);

        if (argumentExists(2))
        {
            COORD newHome;                       /* Position to move cursor    */
            newHome.Y = (short)inrow;      /* convert to short form      */
            newHome.X = (short)incol;      /* convert to short form      */
                                           /* Set the cursor position    */
            SetConsoleCursorPosition(hStdout, newHome);
        }

        return context->NewStringFromAsciiz(buffer);
    }

    return context->NullString();
}

/*************************************************************************
* Function:  SysCurState                                                 *
*                                                                        *
* Syntax:    call SysCurState state                                      *
*                                                                        *
* Params:    state - Either 'ON' or 'OFF'.                               *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

RexxRoutine1(int, SysCurState, CSTRING, option)
{
    CONSOLE_CURSOR_INFO CursorInfo;      /* info about cursor          */
                                         /* get handle to stdout       */
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    /* Get the cursor info        */
    GetConsoleCursorInfo(hStdout,&CursorInfo);

    /* Get state and validate     */
    if (stricmp(option, "ON") == 0)
    {
        CursorInfo.bVisible = true;
    }
    else if (stricmp(option, "OFF") == 0)
    {
        CursorInfo.bVisible = false;
    }
    else
    {
        // this is an error, raise the condition and return
        context->InvalidRoutine();
        return 0;
    }
    /* Set the cursor info        */
    SetConsoleCursorInfo(hStdout,&CursorInfo);
    return 0;                            /* no error on call           */
}


/*************************************************************************
* Function:  SysDriveInfo                                                *
*                                                                        *
* Syntax:    call SysDriveInfo drive                                     *
*                                                                        *
* Params:    drive - 'C', 'D', 'E', etc.                                 *
*                                                                        *
* Return:    disk free total label                                       *
*************************************************************************/

size_t RexxEntry SysDriveInfo(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  CHAR   chFileSysType[MAX_PATH],      /*  File system name          */
         chVolumeName[MAX_PATH],       /*  volume label              */
         chDriveLetter[4];             /*  drive_letter + : + \ + \0 */
  BOOL   bGVIrc;                       /* rc from GVI                */

                                       /* GetDiskFreeSpace calculations */
  DWORD  dwSectorsPerCluster, dwBytesPerSector;
  DWORD  dwFreeClusters, dwClusters;
  BOOL   bGDFSrc;                      /* GDFS rc                    */
  UINT   errorMode;

  DWORD dwgle;
  unsigned __int64 i64FreeBytesToCaller,
                   i64TotalBytes,
                   i64FreeBytes;

                                       /* validate arguments         */
  if (numargs != 1 ||
      args[0].strlength > 2 ||         /* no more than 2 chars       */
      args[0].strlength == 0)          /* at least 1                 */
    return INVALID_ROUTINE;

  const char *arg = args[0].strptr;    /* get argument pointer       */
                                       /* drive letter?              */
  if (strlen(arg) == 2 &&              /* if second letter isn't : bye */
      arg[1] != ':')
    return INVALID_ROUTINE;

  if (arg[0] < 'A' ||                  /* is it in range?            */
      arg[0] > 'z')
    return INVALID_ROUTINE;

   if (strlen(arg) == 1){              /* need to add a : if only the*/
     chDriveLetter[0]=arg[0];          /* letter was passed in       */
     chDriveLetter[1]=':';
     chDriveLetter[2]='\\';            /* need to add \ because of   */
     chDriveLetter[3]='\0';            /* bug in getvolumeinfo       */
   }
   else                           /* have <letter>: , just copy over */
   {
     strcpy(chDriveLetter, args[0].strptr);
     chDriveLetter[2]='\\';            /* need to add \ because of   */
     chDriveLetter[3]='\0';            /* bug in getvolumeinfo       */
   }

  /* try to load GetDiskFreeSpaceEx function pointer */
  if ( !pGetDiskFreeSpaceEx )
  {
    pGetDiskFreeSpaceEx = (P_GDFSE) GetProcAddress(GetModuleHandle("kernel32.dll"),
                                                   "GetDiskFreeSpaceExA");
  } /* endif */

  errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
                         /* get the volume name and file system type */
  bGVIrc=GetVolumeInformation(chDriveLetter,
                           chVolumeName,
                           (DWORD)MAX_PATH,
                           NULL,
                           NULL,
                           NULL,
                           chFileSysType,
                           (DWORD)MAX_PATH);

  dwgle=GetLastError();

  /* use appropriate function */
  if ( pGetDiskFreeSpaceEx )
  {
    bGDFSrc = pGetDiskFreeSpaceEx(chDriveLetter,
                                  (PULARGE_INTEGER) &i64FreeBytesToCaller,
                                  (PULARGE_INTEGER) &i64TotalBytes,
                                  (PULARGE_INTEGER) &i64FreeBytes);
  }
  else
  {
                              /* get the disk free space information */
    bGDFSrc=GetDiskFreeSpace( chDriveLetter,
                              &dwSectorsPerCluster,
                              &dwBytesPerSector,
                              &dwFreeClusters,
                              &dwClusters);

    /* force 64 bit maths */
    i64TotalBytes = (__int64)dwClusters * dwSectorsPerCluster * dwBytesPerSector;
    i64FreeBytes = (__int64)dwFreeClusters * dwSectorsPerCluster * dwBytesPerSector;
  } /* endif */

  dwgle=GetLastError();
  SetErrorMode(errorMode);

  if (bGVIrc && bGDFSrc) {

    /* use simplified display routine with 64 bit types */
    sprintf(retstr->strptr,            // drive free total label
            "%c%c  %-12I64u %-12I64u %-13s",
            chDriveLetter[0], chDriveLetter[1],
            i64FreeBytes, i64TotalBytes, chVolumeName);
                                       /* create return string       */
    retstr->strlength = strlen(retstr->strptr);
  }
  else
    retstr->strlength = 0;             /* return null string         */

  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysDriveMap                                                 *
*                                                                        *
* Syntax:    call SysDriveMap [drive] [,mode]                            *
*                                                                        *
* Params:    drive - 'C', 'D', 'E', etc.  The drive to start the search  *
*                     with.                                              *
*            mode  - Any of the following:  'FREE', 'USED', 'DETACHED',  *
*                                           'LOCAL', 'REMOTE'            *
*                                                                        *
* Return:    'A: B: C: D: ...'                                           *
*************************************************************************/

size_t RexxEntry SysDriveMap(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  CHAR     temp[MAX];                  /* Entire drive map built here*/

  CHAR     tmpstr[MAX];                /* Single drive entries built */
                                       /* here                       */
  CHAR     DeviceName[4];              /* Device name or drive letter*/
                                       /* string                     */
  DWORD    DriveMap;                   /* Drive map                  */
  ULONG    Ordinal;                    /* Ordinal of entry in name   */
                                       /* list                       */
                                       /* required                   */
  ULONG    dnum;                       /* Disk num variable          */
  ULONG    start = 3;                  /* Initial disk num           */
  ULONG    Mode = USED;                /* Query mode USED, FREE,     */
                                       /* LOCAL, etc                 */
  LONG     rc;                         /* OS/2 return codes          */
  UINT     errorMode;

  Ordinal = (ULONG )0;

  temp[0] = '\0';

  if (numargs > 2)                     /* validate arguments         */
    return INVALID_ROUTINE;
                                       /* check starting drive letter*/
  if (numargs >= 1 && args[0].strptr) {

    if ((strlen(args[0].strptr) == 2 &&
        args[0].strptr[1] != ':') ||
        strlen(args[0].strptr) > 2 ||
        strlen(args[0].strptr) == 0)
      return INVALID_ROUTINE;
    start = toupper(args[0].strptr[0])-'A'+1;
  }
  if (start < 1 ||                     /* is it in range?            */
      start > 26)
    return INVALID_ROUTINE;
                                       /* check the mode             */
  if (numargs == 2 && args[1].strlength != 0) {

    if (!stricmp(args[1].strptr, "FREE"))
      Mode = FREE;
    else if (!stricmp(args[1].strptr, "USED"))
      Mode = USED;
    else if (!stricmp(args[1].strptr, "RAMDISK"))
      Mode = RAMDISK;
    else if (!stricmp(args[1].strptr, "REMOTE"))
      Mode = REMOTE;
    else if (!stricmp(args[1].strptr, "LOCAL"))
      Mode = LOCAL;
    else if (!stricmp(args[1].strptr, "REMOVABLE"))
      Mode = REMOVABLE;
    else if (!stricmp(args[1].strptr, "CDROM"))
      Mode = CDROM;
    else
      return INVALID_ROUTINE;
  }
                                       /* perform the query          */
  DriveMap = GetLogicalDrives();
  DriveMap>>=start-1;                  /* Shift to the first drive   */
  temp[0] = '\0';                      /* Clear temporary buffer     */

  for (dnum = start; dnum <= 26; dnum++) {

                                       /* Hey, we have a free drive  */
    if (!(DriveMap&(DWORD)1) && Mode == FREE) {
      sprintf(tmpstr, "%c: ", dnum+'A'-1);
      strcat(temp, tmpstr);
    }
                                       /* Hey, we have a used drive  */
    else if ((DriveMap&(DWORD)1) && Mode == USED) {
      sprintf(tmpstr, "%c: ", dnum+'A'-1);
      strcat(temp, tmpstr);
    }

    else if (DriveMap&(DWORD)1) {      /* Check specific drive info  */
      sprintf(DeviceName, "%c:\\", dnum+'A'-1);

      errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
      rc = (LONG)GetDriveType(DeviceName);
      SetErrorMode(errorMode);
      #ifdef UNDELETE
      DataBufferLen = sizeof DataBuffer;
      DosQueryFSAttach(DeviceName, Ordinal, FSAInfoLevel,
          &DataBuffer, &DataBufferLen);
      rc = DosQueryFSInfo(dnum, 2, buf, sizeof(buf));
      #endif

      if (rc == DRIVE_REMOVABLE && Mode == REMOVABLE) {
                                       /* Hey, we have a removable   */
                                       /* drive                      */
        sprintf(tmpstr, "%c: ", dnum+'A'-1);
        strcat(temp, tmpstr);
      }

      else if (rc == DRIVE_CDROM && Mode == CDROM) {
        sprintf(tmpstr, "%c: ", dnum+'A'-1);
        strcat(temp, tmpstr);
      }

      else if (rc == DRIVE_FIXED && Mode == LOCAL) {
        sprintf(tmpstr, "%c: ", dnum+'A'-1);
        strcat(temp, tmpstr);
      }

      else if (rc == DRIVE_REMOTE && Mode == REMOTE) {
        sprintf(tmpstr, "%c: ", dnum+'A'-1);
        strcat(temp, tmpstr);
      }

      else if (rc == DRIVE_RAMDISK && Mode == RAMDISK) {
        sprintf(tmpstr, "%c: ", dnum+'A'-1);
        strcat(temp, tmpstr);
      }
    }
    DriveMap>>=1;                      /* Shift to the next drive    */
  }

  BUILDRXSTRING(retstr, temp);         /* pass back result           */
  if (retstr->strlength)               /* if not a null string       */
    retstr->strlength--;               /* Get rid of last space      */
  return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysDropFuncs                                                *
*                                                                        *
* Syntax:    call SysDropFuncs                                           *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

size_t RexxEntry SysDropFuncs(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    // this is a NOP now
    retstr->strlength = 0;               /* set return value           */
    return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SysFileDelete                                               *
*                                                                        *
* Syntax:    call SysFileDelete file                                     *
*                                                                        *
* Params:    file - file to be deleted.                                  *
*                                                                        *
* Return:    Return code from DosDelete() function.                      *
*************************************************************************/

RexxRoutine1(int, SysFileDelete, CSTRING, name)
{
    return !DeleteFile(name) ? GetLastError() : 0;
}


/*************************************************************************
* Function:  SysFileSearch                                               *
*                                                                        *
* Syntax:    call SysFileSearch target, file, stem [, options]           *
*                                                                        *
* Params:    target  - String to search for.                             *
*            file    - Filespec to search.                               *
*            stem    - Stem variable name to place results in.           *
*            options - Any combo of the following:                       *
*                       'C' - Case sensitive search (non-default).       *
*                       'N' - Preceed each found string in result stem   *
*                              with it line number in file (non-default).*
*                                                                        *
* Return:    NO_UTIL_ERROR   - Successful.                               *
*            ERROR_NOMEM     - Out of memory.                            *
*************************************************************************/

size_t RexxEntry SysFileSearch(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  CHAR        line[MAX_LINE_LEN];      /* Line read from file        */
  char       *ptr;                     /* Pointer to char str found  */
  ULONG       num = 0;                 /* Line number                */
  size_t      len;                     /* Length of string           */
  size_t      len2;                    /* Length of string           */
  ULONG       rc = 0;                  /* Return code of this func   */
  bool        linenums = false;        /* Set true for linenums in   */
                                       /* output                     */
  bool        sensitive = false;       /* Set true for case-sens     */
                                       /* search                     */
  RXSTEMDATA  ldp;                     /* stem data                  */
  char       *buffer_pointer;          /* current buffer pointer     */
  GetFileData filedata;                /* file read information      */

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
                                       /* validate arguments         */
  if (numargs < 3 || numargs > 4 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;            /* raise an error             */

  buffer_pointer = NULL;               /* nothing in buffer          */

  const char *target = args[0].strptr;             /* get target pointer         */
  const char *file = args[1].strptr;               /* get file name              */

  if (numargs == 4) {                  /* process options            */
    const char *opts = args[3].strptr;     /* point to the options       */
    if (strstr(opts, "N") || strstr(opts, "n"))
      linenums = true;

    if (strstr(opts, "C") || strstr(opts, "c"))
      sensitive = true;
  }

                                       /* Initialize data area       */
  ldp.count = 0;
  strcpy(ldp.varname, args[2].strptr);
  ldp.stemlen = args[2].strlength;
                                       /* uppercase the name         */
  memupper(ldp.varname, strlen(ldp.varname));

  if (ldp.varname[ldp.stemlen-1] != '.')
    ldp.varname[ldp.stemlen++] = '.';

  if (MyOpenFile(file, &filedata)) {   /* open the file              */
    BUILDRXSTRING(retstr, ERROR_FILEOPEN);
    return VALID_ROUTINE;              /* finished                   */
  }
                                       /* do the search...found lines*/
                                       /* are saved in stem vars     */
  while (!GetLine(line, MAX_LINE_LEN - 1, &filedata)) {
    len = strlen(line);
    num++;
    ptr = mystrstr(line, target, len, args[0].strlength, sensitive);

    if (ptr != NULL) {

      if (linenums) {
        wsprintf(ldp.ibuf, "%d ", num);
        len2 = strlen(ldp.ibuf);
        memcpy(ldp.ibuf+len2, line, min(len, IBUF_LEN-len2));
        ldp.vlen = min(IBUF_LEN, len+len2);
      }
      else {
        memcpy(ldp.ibuf, line, len);
        ldp.vlen = len;
      }
      ldp.count++;
      ltoa((long)ldp.count, ldp.varname+ldp.stemlen, 10);

      if (ldp.ibuf[ldp.vlen-1] == '\n')
        ldp.vlen--;
      ldp.shvb.shvnext = NULL;
      ldp.shvb.shvname.strptr = ldp.varname;
      ldp.shvb.shvname.strlength = strlen(ldp.varname);
      ldp.shvb.shvnamelen = ldp.shvb.shvname.strlength;
      ldp.shvb.shvvalue.strptr = ldp.ibuf;
      ldp.shvb.shvvalue.strlength = ldp.vlen;
      ldp.shvb.shvvaluelen = ldp.vlen;
      ldp.shvb.shvcode = RXSHV_SET;
      ldp.shvb.shvret = 0;
      if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) {
        CloseFile(&filedata);          /* close the file             */
        return INVALID_ROUTINE;        /* error on non-zero          */
      }
    }
  }

  CloseFile(&filedata);                /* Close that file            */
                                       /* set stem.0 to lines read   */
  ltoa((long)ldp.count, ldp.ibuf, 10);
  ldp.varname[ldp.stemlen] = '0';
  ldp.varname[ldp.stemlen+1] = 0;
  ldp.shvb.shvnext = NULL;
  ldp.shvb.shvname.strptr = ldp.varname;
  ldp.shvb.shvname.strlength = ldp.stemlen+1;
  ldp.shvb.shvnamelen = ldp.stemlen+1;
  ldp.shvb.shvvalue.strptr = ldp.ibuf;
  ldp.shvb.shvvalue.strlength = strlen(ldp.ibuf);
  ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
  ldp.shvb.shvcode = RXSHV_SET;
  ldp.shvb.shvret = 0;
  if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;                /* no error on call           */
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *                                                                            *
 *   SysFileTree() implmentation and helper functions.                        *
 *                                                                            *
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/**
 * This is a SysFileTree specific function.
 *
 * @param c
 * @param pos
 * @param actual
 */
static void badSFTOptsException(RexxThreadContext *c, size_t pos, CSTRING actual)
{
    char buf[256] = {0};
    _snprintf(buf, sizeof(buf),
             "SysFileTree argument %zd must be a combination of F, D, B, S, T, L, I, or O; found \"%s\"",
             pos, actual);

    c->RaiseException1(Rexx_Error_Incorrect_call_user_defined, c->String(buf));
}

/**
 * This is a SysFile specific function.
 *
 * @param c
 * @param pos
 * @param actual
 */
static void badMaskException(RexxThreadContext *c, size_t pos, CSTRING actual)
{
    char buf[256] = {0};
    _snprintf(buf, sizeof(buf),
             "SysFileTree argument %zd must be 5 characters or less in length containing only '+', '-', or '*'; found \"%s\"",
             pos, actual);

    c->RaiseException1(Rexx_Error_Incorrect_call_user_defined, c->String(buf));
}

/**
 * Returns a value that is greater than 'need' by doubling 'have' until that
 * value is reached.
 */
inline size_t neededSize(size_t need, size_t have)
{
    while ( have < need )
    {
        have *= 2;
    }
    return have;
}

/**
 * Allocates a buffer that is at least twice as big as the buffer passed in.
 *
 * @param c             Call context we are operating in.
 * @param dBuf          Pointer to the buffer to reallocate
 * @param nBuf          Size of current dBuf buffer. Will be updated on return
 *                      to size of newly allocated buffer.
 * @param needed        Minimum size needed.
 * @param nStaticBuffer Size of original static buffer.
 *
 * @return True on success, false on memory allocation failure.
 *
 * @remarks  NOTE: that the pointer to the buffer to reallocate, may, or may
 *           not, be a pointer to a static buffer.  We must NOT try to free a
 *           static buffer and we MUST free a non-static buffer.
 */
static bool getBiggerBuffer(RexxCallContext *c, char **dBuf, size_t *nBuf, size_t needed, size_t nStaticBuffer)
{
    if ( *nBuf != nStaticBuffer )
    {
        LocalFree(*dBuf);
    }

    *nBuf = neededSize(needed, *nBuf);
    *dBuf = (char *)LocalAlloc(LPTR, *nBuf * sizeof(char));

    if ( *dBuf == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    return true;
}

/**
 * Checks that attr is the same as that specified by the mask.
 *
 * @param mask
 * @param attr
 * @param options
 *
 * @return True for a match, otherwise false.
 */
static bool sameAttr(int32_t *mask, uint32_t attr, uint32_t options)
{
    if ( (options & DO_DIRS) && ! (options & DO_FILES) && ! (attr & FILE_ATTRIBUTE_DIRECTORY) )
    {
        return false;
    }
    if ( ! (options & DO_DIRS) && (options & DO_FILES) && (attr & FILE_ATTRIBUTE_DIRECTORY) )
    {
        return false;
    }
    if ( mask[0] == RXIGNORE )
    {
        return  true;
    }

    if ( mask[0] < 0 && attr & FILE_ATTRIBUTE_ARCHIVE )
    {
        return  false;
    }
    if ( mask[0] > 0 && ! (attr & FILE_ATTRIBUTE_ARCHIVE) )
    {
        return  false;
    }
    if ( mask[1] < 0 && attr & FILE_ATTRIBUTE_DIRECTORY )
    {
        return  false;
    }
    if ( mask[1] > 0 && ! (attr & FILE_ATTRIBUTE_DIRECTORY) )
    {
        return  false;
    }
    if ( mask[2] < 0 && attr & FILE_ATTRIBUTE_HIDDEN )
    {
        return  false;
    }
    if (mask[2] > 0 && ! (attr & FILE_ATTRIBUTE_HIDDEN) )
    {
        return  false;
    }
    if (mask[3] < 0 && attr & FILE_ATTRIBUTE_READONLY )
    {
        return  false;
    }
    if (mask[3] > 0 && ! (attr & FILE_ATTRIBUTE_READONLY) )
    {
        return  false;
    }
    if (mask[4] < 0 && attr & FILE_ATTRIBUTE_SYSTEM )
    {
        return  false;
    }
    if (mask[4] > 0 && ! (attr & FILE_ATTRIBUTE_SYSTEM) )
    {
        return  false;
    }

    return  true;
}

/**
 * Returns a new file attribute value given a mask of attributes to be changed
 * and the current attribute value.
 *
 * @param mask
 * @param attr
 *
 * @return New attribute value.
 */
static uint32_t newAttr(int32_t *mask, uint32_t attr)
{
    if ( mask[0] == RXIGNORE )
    {
      return  attr;
    }

    if ( mask[0] < 0 )
    {
        attr &= ~FILE_ATTRIBUTE_ARCHIVE;   // Clear
    }
    if ( mask[0] > 0 )
    {
        attr |= FILE_ATTRIBUTE_ARCHIVE;    // Set
    }
    if ( mask[1] < 0 )
    {
        attr &= ~FILE_ATTRIBUTE_DIRECTORY; // Clear
    }
    if ( mask[1] > 0 )
    {
        attr |= FILE_ATTRIBUTE_DIRECTORY;  // Set
    }
    if ( mask[2] < 0 )
    {
        attr &= ~FILE_ATTRIBUTE_HIDDEN;    // Clear
    }
    if ( mask[2] > 0 )
    {
        attr |= FILE_ATTRIBUTE_HIDDEN;     // Set
    }
    if ( mask[3] < 0 )
    {
        attr &= ~FILE_ATTRIBUTE_READONLY;  // Clear
    }
    if ( mask[3] > 0 )
    {
        attr |= FILE_ATTRIBUTE_READONLY;   // Set
    }
    if ( mask[4] < 0 )
    {
        attr &= ~FILE_ATTRIBUTE_SYSTEM;    // Clear
    }
    if ( mask[4] > 0 )
    {
        attr |= FILE_ATTRIBUTE_SYSTEM;     // Set
    }

    return  attr;
}

/**
 * Changes the file attributes of the specified file to those specified by attr.
 *
 * @param file  File to change the attributes of.
 *
 * @param attr  New file attributes.
 *
 * @return True on success, false on error.
 *
 * @remarks  Note that this function was named SetFileMode() in the old IBM
 * code.
 */
static bool setAttr(const char *file, uint32_t attr)
{
    if ( SetFileAttributes(file, attr) == 0 )
    {
        return false;
    }
    return true;
}


/**
 * This function is used by SysFileTree only.
 *
 * Formats the line for a found file and adds it to the stem containing all the
 * found files.
 *
 * @param c
 * @parm  path
 * @param treeData
 * @param newMask
 * @param options
 * @param wfd
 *
 * @return True on success, false on error.
 *
 * @remarks  We try to use the static buffers in treeData, but if they are not
 *  big enough, we allocate memory.  If we do allocate memory, we have to free
 *  it of course.  We can determine if the memory needs to be freed by checking
 *  that either nFoundFile, or nFoundFileLine, are the same size as they are
 *  originally set to, or not.
 *
 *  If the file search is a very deep recursion in the host file system, a very
 *  large number of String objects may be created in the single Call context of
 *  SysFileTree.  A reference to each created object is saved in a hash table to
 *  protect it from garbage collection, which can lead to a very large hash
 *  table.  To prevent the creation of a very large hash table, we create a temp
 *  object, pass that object to the interpreter, and then tell the interpreter
 *  the object no longer needs to be protected in this call context.
 */
static bool formatFile(RexxCallContext *c, char *path, RXTREEDATA *treeData, int32_t *newMask,
                       uint32_t options, WIN32_FIND_DATA *wfd)
{
    SYSTEMTIME systime;
    FILETIME   ftLocal;

    char   *dFoundFile = treeData->foundFile;
    size_t  nFoundFile = sizeof(treeData->foundFile);

    int len = _snprintf(dFoundFile, nFoundFile, "%s%s", path, wfd->cFileName);
    if ( len < 0 || len == nFoundFile )
    {
        nFoundFile = strlen(path) + strlen(wfd->cFileName) + 1;
        dFoundFile = (char *)LocalAlloc(LPTR, nFoundFile);
        if ( dFoundFile == NULL )
        {
            outOfMemoryException(c->threadContext);
            return false;
        }

        // Buffer is sure to be big enough now, we we don't check the return.
        _snprintf(dFoundFile, nFoundFile, "%s%s", path, wfd->cFileName);
    }

    if ( options & NAME_ONLY )
    {
        RexxStringObject t = c->String(dFoundFile);

        // Add the file name to the stem and be done with it.
        treeData->count++;
        c->SetStemArrayElement(treeData->files, treeData->count, t);
        c->ReleaseLocalReference(t);

        if ( nFoundFile != sizeof(treeData->foundFile) )
        {
            LocalFree(dFoundFile);
        }
        return true;
    }

    // The file attributes need to be changed before we format the found file
    // line.

    uint32_t changedAttr = newAttr(newMask, wfd->dwFileAttributes);
    if ( changedAttr != wfd->dwFileAttributes )
    {
        // try to set the attributes, but if it fails, just use the exsiting.
        if ( ! setAttr(treeData->foundFile, changedAttr & ~FILE_ATTRIBUTE_DIRECTORY) )
        {
            changedAttr = wfd->dwFileAttributes;
        }
    }

    // Convert UTC to local file time, and then to system format.
    FileTimeToLocalFileTime(&wfd->ftLastWriteTime, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &systime);

    // The fileTime buffer is 64 bytes, and the fileAtt buffer is 16 bytes.
    // Since we can count the characters put into the buffer here, there is
    // no need to check for buffer overflow.

    if ( options & LONG_TIME )
    {
        sprintf(treeData->fileTime, "%4d-%02d-%02d %02d:%02d:%02d  %10lu  ",
                systime.wYear,
                systime.wMonth,
                systime.wDay,
                systime.wHour,
                systime.wMinute,
                systime.wSecond,
                wfd->nFileSizeLow);
    }
    else
    {
        if ( options & EDITABLE_TIME )
        {
            sprintf(treeData->fileTime, "%02d/%02d/%02d/%02d/%02d  %10lu  ",
                    (systime.wYear + 100) % 100,
                    systime.wMonth,
                    systime.wDay,
                    systime.wHour,
                    systime.wMinute,
                    wfd->nFileSizeLow);
        }
        else
        {
            sprintf(treeData->fileTime, "%2d/%02d/%02d  %2d:%02d%c  %10lu  ",
                    systime.wMonth,
                    systime.wDay,
                    (systime.wYear + 100) % 100,
                    (systime.wHour < 13 && systime.wHour != 0 ?
                     systime.wHour : (abs(systime.wHour - (SHORT)12))),
                    systime.wMinute,
                    (systime.wHour < 12 || systime.wHour == 24) ? 'a' : 'p',
                    wfd->nFileSizeLow);
        }
    }

    sprintf(treeData->fileAttr, "%c%c%c%c%c  ",
           (changedAttr & FILE_ATTRIBUTE_ARCHIVE)   ? 'A' : '-',
           (changedAttr & FILE_ATTRIBUTE_DIRECTORY) ? 'D' : '-',
           (changedAttr & FILE_ATTRIBUTE_HIDDEN)    ? 'H' : '-',
           (changedAttr & FILE_ATTRIBUTE_READONLY)  ? 'R' : '-',
           (changedAttr & FILE_ATTRIBUTE_SYSTEM)    ? 'S' : '-');

    // Now format the complete line, allocating memory if we have to.

    char   *dFoundFileLine = treeData->foundFileLine;
    size_t  nFoundFileLine = sizeof(treeData->foundFileLine);

    len = _snprintf(dFoundFileLine, nFoundFileLine, "%s%s%s",
                    treeData->fileTime, treeData->fileAttr, dFoundFile);
    if ( len < 0 || len == nFoundFileLine )
    {
        nFoundFileLine = strlen(treeData->fileTime) + strlen(treeData->fileAttr) + nFoundFile + 1;
        dFoundFileLine = (char *)LocalAlloc(LPTR, nFoundFileLine);

        if ( dFoundFileLine == NULL )
        {
            outOfMemoryException(c->threadContext);
            if ( nFoundFile != sizeof(treeData->foundFile) )
            {
                LocalFree(dFoundFile);
            }
            return false;
        }
        // Buffer is sure to be big enough now so we don't check return.
        _snprintf(dFoundFileLine, nFoundFileLine, "%s%s%s", treeData->fileTime, treeData->fileAttr, dFoundFile);
    }

    // Place found file line in the stem.
    RexxStringObject t = c->String(dFoundFileLine);

    treeData->count++;
    c->SetStemArrayElement(treeData->files, treeData->count, t);
    c->ReleaseLocalReference(t);

    if ( nFoundFile != sizeof(treeData->foundFile) )
    {
        LocalFree(dFoundFile);
    }
    if ( nFoundFileLine != sizeof(treeData->foundFileLine) )
    {
        LocalFree(dFoundFileLine);
    }

    return true;
}

/**
 * Finds all files matching a file specification, formats a file name line and
 * adds the formatted line to a stem.  Much of the data to complete this
 * operation is contained in the treeData struct.
 *
 * This is a recursive function that may search through subdirectories if the
 * recurse option is used.
 *
 * @param c           Call context we are operating in.
 *
 * @param path        Current directory we are searching.
 *
 * @param treeData    Struct containing data pertaining to the search, such as
 *                    the file specification we are searching for, the stem to
 *                    put the results in, etc..
 *
 * @param targetMask  An array of integers which describe the source attribute
 *                    mask.  Only files with attributes matching this mask will
 *                    be found.
 *
 * @param newMask     An array of integers which describe the target attribute
 *                    mask.  Attributes of all found files will be changed / set
 *                    to the values specified by this mask.
 * @param options
 *
 * @return uint32_t
 *
 * @remarks  For both targetMask and newMask, each index of the mask corresponds
 *           to a different file attribute.  Each index and its associated
 *           attribute are as follows:
 *
 *                        mask[0] = FILE_ARCHIVED
 *                        mask[1] = FILE_DIRECTORY
 *                        mask[2] = FILE_HIDDEN
 *                        mask[3] = FILE_READONLY
 *                        mask[4] = FILE_SYSTEM
 *
 *           A negative value at a given index indicates that the attribute bit
 *           of the file is not set.  A positive number indicates that the
 *           attribute should be set. A value of 0 indicates a "Don't Care"
 *           setting.
 *
 *           A close reading of MSDN seems to indicate that as long as we are
 *           compiled for ANSI, which we are, that MAX_PATH is sufficiently
 *           large.  But, we will code for the possibility that it is not large
 *           enough, by mallocing dynamic memory if _snprintf indicates a
 *           failure.
 *
 *           We point dTmpFileName at the static buffer and nTmpFileName is set
 *           to the size of the buffer.  If we have to allocate memory,
 *           nTmpFileName will be set to the size we allocate and if
 *           nTmpFileName does not equal what it is originally set to, we know
 *           we have to free the allocated memory.
 */
static bool recursiveFindFile(RexxCallContext *c, char *path, RXTREEDATA *treeData,
                              int32_t *targetMask, int32_t *newMask, uint32_t options)
{
  WIN32_FIND_DATA  wfd;
  HANDLE           fHandle;
  char             tmpFileName[FNAMESPEC_BUF_LEN];
  char            *dTmpFileName = tmpFileName;       // Dynamic memory for tmpFileName, static memory to begin with.
  size_t           nTmpFileName = FNAMESPEC_BUF_LEN; // CouNt of bytes in dTmpFileName.
  int32_t          len;
  bool             result = true;

  len = _snprintf(dTmpFileName, nTmpFileName, "%s%s", path, treeData->dFNameSpec);
  if ( len < 0 || len == nTmpFileName )
  {
      nTmpFileName = strlen(path) + strlen(treeData->dFNameSpec) + 1;
      dTmpFileName = (char *)LocalAlloc(LPTR, nTmpFileName);
      if ( dTmpFileName == NULL )
      {
          outOfMemoryException(c->threadContext);
          result = false;
          goto done_out;
      }
      // buffer is sure to be big enough now, so we don't check the return.
      _snprintf(dTmpFileName, nTmpFileName, "%s%s", path, treeData->dFNameSpec);
  }

  fHandle = FindFirstFile(dTmpFileName, &wfd);
  if ( fHandle != INVALID_HANDLE_VALUE )
  {
      do
      {
          // Skip dot directories
          if ( strcmp(wfd.cFileName, ".") == 0 || strcmp(wfd.cFileName, "..") == 0 )
          {
              continue;
          }

          if ( sameAttr(targetMask, wfd.dwFileAttributes, options) )
          {
              if ( ! formatFile(c, path, treeData, newMask, options, &wfd) )
              {
                  FindClose(fHandle);
                  result = false;
                  goto done_out;
              }
          }
      } while ( FindNextFile(fHandle, &wfd) );

      FindClose(fHandle);
  }

  if ( options & RECURSE )
  {
      // Build new target spec.  Above, path + fileSpec fit into tmpFileName,
      // fileSpec is always longer than 1 character, so we are okay here.
      sprintf(dTmpFileName, "%s*", path);

      fHandle = FindFirstFile(dTmpFileName, &wfd);
      if ( fHandle != INVALID_HANDLE_VALUE )
      {
          do
          {
              // Skip non-directories and dot directories.
              if ( ! (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                   strcmp(wfd.cFileName, ".") == 0 || strcmp(wfd.cFileName, "..") == 0 )
              {
                  continue;
              }

              // Build the new directory file name.
              len = _snprintf(dTmpFileName, nTmpFileName, "%s%s\\", path, wfd.cFileName);
              if ( len < 0 || len == nTmpFileName )
              {
                  // We may need to free dTmpFileName if it is now allocated
                  // memory.
                  if ( nTmpFileName != FNAMESPEC_BUF_LEN )
                  {
                      LocalFree(dTmpFileName);
                  }

                  nTmpFileName = strlen(path) + strlen(wfd.cFileName) + 2;
                  dTmpFileName = (char *)LocalAlloc(LPTR, nTmpFileName);
                  if ( dTmpFileName == NULL )
                  {
                      outOfMemoryException(c->threadContext);
                      FindClose(fHandle);
                      result = false;
                      goto done_out;
                  }
                  // buffer is sure to be big enough now, so we don't check the
                  // return.
                  _snprintf(dTmpFileName, nTmpFileName, "%s%s\\", path, wfd.cFileName);
              }

              // Search the next level.
              if ( ! recursiveFindFile(c, tmpFileName, treeData, targetMask, newMask, options) )
              {
                  FindClose(fHandle);
                  result = false;
                  goto done_out;
              }
          }
          while (FindNextFile(fHandle, &wfd));

          FindClose(fHandle);
      }
  }

done_out:

    if ( nTmpFileName != FNAMESPEC_BUF_LEN )
    {
        safeLocalFree(dTmpFileName);
    }
    return result;
}

/**
 * This is a SysFileTree() specific function.  It is only called, indirectly
 * through getPath(), from SysFileTree().
 *
 * This function mimics the old IBM code.
 *
 * Leading spaces are stripped, in some cases. A file specification of "." is
 * changed to "*.*" and a file specification of ".." is changed to "..\*.*"
 *
 * Leading spaces in fSpec are stripped IFF the first character(s) after the
 * leading spaces:
 *
 *       is '\' or '/'
 *     or
 *       is '.\' or './'
 *     or
 *       is '..\' or '../'
 *     or
 *        is z:  (i.e., a drive letter)
 *
 * @param fSpec  The SysFileTree search specification
 *
 * @return A pointer to fSpec, possibly adjust to point to the first non-space
 *         character in the string.
 *
 * @side effects:  fSpec may be changed from "." to "*.*" or may be changed from
 *                 ".." to "..\*.*"
 *
 * @assumes:  The buffer for fSpec is large enough for the possible changes.
 */
static char *adjustFSpec(char *fSpec)
{
    size_t i = 0;

    // Skip leading blanks.
    while ( fSpec[i] == ' ' )
    {
        i++;
    }

    if ( i > 0 )
    {
        size_t len = strlen(fSpec);

        // This series of if statements could be combined in to on huge if, but
        // this is easier to comprehend:
        if ( fSpec[i] == '\\' || fSpec[i] == '/' )                         // The "\" case
        {
            fSpec = &fSpec[i];
        }
        else if ( fSpec[i] == '.' )
        {
            if ( i + 1 < len )
            {
                if ( fSpec[i + 1] == '\\' || fSpec[i + 1] == '/' )         // The ".\" case
                {
                    fSpec = &fSpec[i];
                }
                else if ( i + 2 < len )
                {
                    if ( fSpec[i + 1] == '.' &&
                         (fSpec[i + 2] == '\\' || fSpec[i + 2] == '/') )   // The "..\" case
                    {
                        fSpec = &fSpec[i];
                    }
                }
            }
        }
        else if ( i + 1 < len && fSpec[i + 1] == ':' )                     // The "z:' case
        {
            fSpec = &fSpec[i];
        }
    }

    if ( strcmp(fSpec, ".") == 0 )
    {
        // If fSpec is exactly "." then change it to "*.*"
        strcpy(fSpec, "*.*");
    }
    else if ( strcmp(fSpec, "..") == 0 )
    {
        // Else if fSpec is exactly ".." then change it to "..\*.*"
        strcpy(fSpec, "..\\*.*");
    }

    return fSpec;
}

static bool safeGetCurrentDirectory(RexxCallContext *c, char **pPath, size_t *pPathLen)
{
    size_t  pathLen = *pPathLen;
    char   *path    = *pPath;

    // Get the current directory.  First check that the path buffer is large
    // enough.
    uint32_t ret = GetCurrentDirectory(0, 0);
    if ( ret == 0 )
    {
        systemServiceExceptionCode(c->threadContext, "GetCurrentDirectory", GetLastError());
        return false;
    }

    // we might need to add a trailing backslash here, so make sure we leave enough room
    ret += FNAMESPEC_BUF_EXTRA;

    path = (char *)LocalAlloc(LPTR, ret);
    if ( path == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    // Fix up our input path / pathLen variables now.  The input path buffer
    // is allocated memory, so we need to free it.
    LocalFree(*pPath);
    *pPath    = path;
    *pPathLen = ret;

    ret = GetCurrentDirectory(ret, path);
    if ( ret == 0 )
    {
        systemServiceExceptionCode(c->threadContext, "GetCurrentDirectory", GetLastError());
        return false;
    }

    return true;
}

static bool expandNonPath2fullPath(RexxCallContext *c, char *fSpec, char **pPath, size_t *pPathLen, int *lastSlashPos)
{
    char     *buf    = NULL;  // used to save current dir
    char      drv[3] = {0};   // used to change current drive
    uint32_t  ret    = 0;
    bool      result = false;

    // fSpec could be a drive designator.
    if ( fSpec[1] == ':' )
    {
        // Save the current drive and path, first get needed buffer size.
        ret = GetCurrentDirectory(0, 0);
        if ( ret == 0 )
        {
            systemServiceExceptionCode(c->threadContext, "GetCurrentDirectory", ret);
            goto done_out;
        }

        buf = (char *)LocalAlloc(LPTR, ret);
        if ( buf == NULL )
        {
            outOfMemoryException(c->threadContext);
            goto done_out;
        }

        ret = GetCurrentDirectory(ret, buf);
        if ( ret == 0 )
        {
            systemServiceExceptionCode(c->threadContext, "GetCurrentDirectory", ret);
            goto done_out;
        }

        // Just copy the drive letter and the colon, omit the rest.  This is
        // necessary e.g. for something like "I:*"
        memcpy(drv, fSpec, 2);

        // Change to the specified drive, get the current directory, then go
        // back to where we came from.
        SetCurrentDirectory(drv);
        bool success = safeGetCurrentDirectory(c, pPath, pPathLen);

        SetCurrentDirectory(buf);
        LocalFree(buf);
        buf = NULL;

        if ( ! success )
        {
            systemServiceExceptionCode(c->threadContext, "GetCurrentDirectory", ret);
            goto done_out;
        }

        // make drive the path
        *lastSlashPos = 1;
    }
    else
    {
        // No drive designator, just get the current directory.
        if ( ! safeGetCurrentDirectory(c, pPath, pPathLen) )
        {
            goto done_out;
        }
    }

    // If we need a trailing slash, add one.
    char *path = *pPath;
    if ( path[strlen(path) - 1] != '\\' )
    {
        strcat(path, "\\");
    }

    result = true;

done_out:
    safeLocalFree(buf);
    return result;
}


/**
 * Splits the path portion off from fSpec and returns it in the path buffer.
 *
 * When this function is called, there is always at least one slash in fSpec.
 *
 * @param c
 * @param fSpec
 * @param lastSlashPos
 * @param pPath
 * @param pPathLen
 *
 * @return bool
 *
 * @remarks  The size of the path buffer is guarenteed to be at least the string
 *           length of fSpec + FNAMESPEC_BUF_EXTRA (8) in size.  Or MAX (264)
 *           bytes in size.  Whichever is larger.  So path is big enough to
 *           contain all of fSpec + 7 characters.
 *
 *           We may have to enlarge the passed in path buffer.  If we do, we
 *           need to be sure and update the path buffer pointer and the path
 *           length. As long as we keep pPath and pPathLen correct, the caller
 *           will take care of freeing any memory.
 *
 *           But if we do change pPath, we need to free the buffer it was
 *           pointing to.
 */
static bool expandPath2fullPath(RexxCallContext *c, char *fSpec, size_t lastSlashPos, char **pPath, size_t *pPathLen)
{
    size_t l = 0;    // Used to calculate lengths of strings.

    char   *path    = *pPath;
    size_t  pathLen = *pPathLen;

    // If fSpec starts with a drive designator, then we have a full path. Copy
    // over the path portion, including the last slash, and null terminate it.
    if (fSpec[1] == ':')
    {
        l = lastSlashPos + 1;
        memcpy(path, fSpec, l);
        path[l] = '\0';
    }
    else
    {
        char fpath[MAX_PATH];
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        char lastChar;

        // fpath is the only buffer we need to worry about being too small.
        // Although, when compiled for ANSI, which we are, I really think it is
        // impossible for MAX_PATH to be too small.
        char   *dFPath = fpath;
        size_t  nFPath = MAX_PATH;

        if ( lastSlashPos == 0 )
        {
            // Only 1 slash at the beginning, get the full path.
            _fullpath(dFPath, "\\", nFPath);

            l = strlen(dFPath) + strlen(&fSpec[1]) + 1;
            if ( l > nFPath )
            {
                if ( ! getBiggerBuffer(c, &dFPath, &nFPath, l, nFPath) )
                {
                    return false;
                }
            }

            strcat(dFPath, &fSpec[1]);
        }
        else
        {
            // Chop off the path part by putting a null at the last slash, get
            // the full path, and then put the slash back.
            fSpec[lastSlashPos] = '\0';
            if ( _fullpath(dFPath, fSpec, nFPath) == NULL )
            {
                // This will double the buffer until either _fullpath()
                // succeeds, or we run out of memory.  If we go through the loop
                // more than once, and fail, we need to free memory allocated
                // for dFPath.  We fix fSpec on failure, but that is not really
                // needed, the caller(s) will just quit on failure of this
                // function.
                do
                {
                    if ( ! getBiggerBuffer(c, &dFPath, &nFPath, l, MAX_PATH) )
                    {
                        if ( nFPath != MAX_PATH )
                        {
                            LocalFree(dFPath);
                        }

                        fSpec[lastSlashPos] = '\\';
                        return false;
                    }
                } while ( _fullpath(dFPath, fSpec, nFPath) == NULL );
            }

            fSpec[lastSlashPos] = '\\';

            lastChar = dFPath[strlen(dFPath) - 1];
            if (lastChar != '\\' && lastChar != '/')
            {
                l = strlen(dFPath) + strlen(&fSpec[lastSlashPos]) + 1;
                if ( l > nFPath )
                {
                    if ( ! getBiggerBuffer(c, &dFPath, &nFPath, l, MAX_PATH) )
                    {
                        // If dFPath was allocated, free it.
                        if ( nFPath != MAX_PATH )
                        {
                            LocalFree(dFPath);
                        }
                        return false;
                    }
                }

                strcat(dFPath, &fSpec[lastSlashPos]);
            }
        }

        _splitpath(dFPath, drive, dir, fname, ext);

        l = strlen(drive) + strlen(dir) + 1;
        if ( l > pathLen )
        {
            if ( ! getBiggerBuffer(c, &path, &pathLen, l, pathLen) )
            {
                return false;
            }

            LocalFree(*pPath);
            *pPath    = path;
            *pPathLen = pathLen;
        }

        strcpy(path, drive);
        strcat(path, dir);

        // If path is invalid, (the empty string,) for some reason, copy the
        // path from fSpec.  That is from the start of the string up through the
        // last slash.  Then zero terminate it.  The path buffer is guaranteed
        // big enough for this, see the remarks.
        if ( strlen(path) == 0 )
        {
            memcpy(path, fSpec, lastSlashPos + 1);
            path[lastSlashPos + 1] = '\0';
        }

        // If we need a trailing slash, add it.  Again, the path buffer is
        // guaranteed to be big enough.
        if (path[strlen(path) - 1] != '\\')
        {
            strcat(path, "\\");
        }
    }

    return true;
}

/**
 * This is a SysFileTree() specific function..
 *
 * This function expands the file spec passed in to the funcition into its full
 * path name.  The full path name is then split into the path portion and the
 * file name portion.  The path portion is then returned in path and the file
 * name portion is returned in fileName.
 *
 * The path portion will end with the '\' char if fSpec contains a path.
 *
 * @param fSpec
 * @param path       Pointer to path buffer.  Path buffer is allocated memory,
 *                   not a static buffer.
 * @param filename
 * @param pathLen    Pointer to size of the path buffer.
 *
 * @remarks  On entry, the buffer pointed to by fSpec is guaranteed to be at
 *           least strlen(fSpec) + FNAMESPEC_BUF_EXTRA (8).  So, we can strcat
 *           to it at least 7 characters and still have it null terminated.
 *
 *           In addition, the path buffer is guarenteed to be at least that size
 *           also.
 */
static bool getPath(RexxCallContext *c, char *fSpec, char **path, char *filename, size_t *pathLen)
{
    size_t len;                     // length of filespec
    int    lastSlashPos;            // position of last slash

    fSpec = adjustFSpec(fSpec);

    // Find the position of the last slash in fSpec
    len = strlen(fSpec);

    // Get the maximum position of the last '\'
    lastSlashPos = (int)len;

    // Step back through fSpec until at its beginning or at a '\' or '/' character
    while ( fSpec[lastSlashPos] != '\\' && fSpec[lastSlashPos] != '/' && lastSlashPos >= 0 )
    {
        --lastSlashPos;
    }

    // If lastSlashPos is less than 0, then there is no backslash present in
    // fSpec.
    if ( lastSlashPos < 0 )
    {
        if ( ! expandNonPath2fullPath(c, fSpec, path, pathLen, &lastSlashPos) )
        {
            return false;
        }
    }
    else
    {
        if ( ! expandPath2fullPath(c, fSpec, lastSlashPos, path, pathLen) )
        {
            return false;
        }
    }

    // Get the file name from fSpec, the portion just after the last '\'
    if ( fSpec[lastSlashPos + 1] != '\0' )
    {
        // The position after the last slash is not the null terminator so there
        // is something to copy over to the file name segment.
        strcpy(filename, &fSpec[lastSlashPos + 1]);
    }
    else
    {
        // The last slash is the last character in fSpec, just use wildcards for
        // the file name segment.
        strcpy(filename, "*.*");
    }

    return true;
}

/**
 * This is a SysFileTree specific function.
 *
 * Determines the options by converting the character based argument to the
 * correct set of flags.
 *
 * @param c
 * @param opts
 * @param pOpts
 *
 * @return bool
 */
static bool goodOpts(RexxCallContext *c, char *opts, uint32_t *pOpts)
{
    uint32_t options = *pOpts;

    while ( *opts )
    {
        switch( toupper(*opts) )
        {
          case 'S':                      // recurse into subdirectories
              options |= RECURSE;
              break;

          case 'O':                      // only return names
              options |= NAME_ONLY;
              break;

          case 'T':                      // use short time format, ignored if L is used
            options |= EDITABLE_TIME;
            break;

          case 'L':                      // use long time format
              options |= LONG_TIME;
              break;

          case 'F':                      // include only files
              options &= ~DO_DIRS;
              options |= DO_FILES;
              break;

          case 'D':                      // include only directories
              options |= DO_DIRS;
              options &= ~DO_FILES;
              break;

          case 'B':                      // include both files and directories
              options |= DO_DIRS;
              options |= DO_FILES;
              break;

          case 'I':                      // case insensitive? no op on Windows
              break;

          default:                       // error, unknown option
            return false;
        }
        opts++;
    }

    *pOpts = options;
    return true;
}

/**
 * This is a SysFileTree() specific helper function.
 *
 * Set a mask of unsigned ints to what is specified by a mask of chars.
 *
 * @param c
 * @param msk
 * @param mask
 *
 * @return True on success, false on error.
 *
 * @remarks  If a character in position N is a '+' then the unsigned int at
 *           position N is set to 1.  This is turning it on.
 *
 *           If a character in position N is a '-' then the unsigned int at
 *           position N is set to -1.  This is turning it off.
 *
 *           If a character in position N is a '*' then the unsigned int at
 *           position N is set to 0.  This is saying ignore it, it doesn't
 *           matter what the attribute is.
 */
static bool goodMask(RexxCallContext *c, char *msk, int32_t *mask)
{
    uint32_t y = 0;

    while (*msk)
    {
        if ( *msk == '+' )
        {
            mask[y] = 1;
        }
        else if ( *msk == '-' )
        {
            mask[y] = -1;
        }
        else if (*msk == '*')
        {
            mask[y] = 0;
        }
        else
        {
            return false;
        }

        y++;
        msk++;
    }

    return true;
}

/**
 * This is a SysFileTree specific helper function.
 *
 * Checks the validity of an attribute mask argument and converts the character
 * based mask into an integer based mask.
 *
 * @param context
 * @param msk
 * @param mask
 * @param argPos
 *
 * @return bool
 */
static bool getMaskFromArg(RexxCallContext *context, char *msk, int32_t *mask, size_t argPos)
{
    if ( argumentExists(argPos) && strlen(msk) > 0 )
    {
        if ( strlen(msk) > 5 )
        {
            badMaskException(context->threadContext, argPos, msk);
            return false;
        }

        if ( ! goodMask(context, msk, mask) )
        {
            badMaskException(context->threadContext, argPos, msk);
            return false;
        }
    }
    else
    {
        mask[0] = RXIGNORE;
    }

    return true;
}

/**
 * This is a SysFileTree specific helper function.
 *
 * Checks the validity of the options argument to SysFileTree and converts the
 * character based argument to the proper set of flags.
 *
 * @param context
 * @param opts
 * @param options
 * @param argPos
 *
 * @return bool
 */
static bool getOptionsFromArg(RexxCallContext *context, char *opts, uint32_t *options, size_t argPos)
{
    *options = DO_FILES | DO_DIRS;

    if ( argumentExists(argPos) )
    {
        if ( strlen(opts) == 0 )
        {
            nullStringException(context->threadContext, "SysFileTree", argPos);
            return false;
        }

        if ( ! goodOpts(context, opts, options) )
        {
            badSFTOptsException(context->threadContext, argPos, opts);
            return false;
        }
    }

    return true;
}

/**
 * This is a SysFileTree specific helper function.
 *
 * Allocates and returns a buffer containing the file specification to search
 * for.
 *
 * The file specification consists of the search string as sent by the Rexx
 * user, with possibly some glob characters added.  The returned buffer is
 * bigger than the original string to accommodate these, possible, added
 * characters.  The number of bytes added to the buffer is 8, which is what the
 * original IBM code used.  8 is probably 1 byte more than needed, but there is
 * no reason that this needs to be exact, better too long than too short.
 *
 * If the file speicfication ends in a slash ('\') or a period ('.') or two
 * periods ('..'), then a wild card specification ('*.*') is appended.
 *
 * However, note that there is also the case where a single '.' at the end of
 * the file specification is not used as a directory specifier, but rather is
 * tacked on to the end of a file name.
 *
 * Windows has a sometimes used convention that a '.' at the end of a file name
 * can be used to indicate the file has no extension. For example, given a file
 * named: MyFile a command of "dir MyFile." will produce a listing of "MyFile".
 *
 * In this case we want to leave the file specification alone. that is, do not
 * append a "*.*". A command of "dir *." will produce a directory listing of all
 * files that do not have an extension.
 *
 * @param context
 * @param fSpec
 * @param fSpecLen     [returned]  The length of the original fSpec argument,
 *                     not the length of the allocated buffer.
 * @param fSpecBufLen  [returned]  The length of the length of the allocated
 *                     fSpec buffer.
 * @param argPos
 *
 * @return A string specifying the file pattern to search for.  The buffer
 *         holding the string is larger than the original input specify.
 *
 * @remarks  Caller is responsible for freeing memory.  Memory is allocated
 *           using LocalAlloc(), not malloc().
 *
 *           If the returned buffer is null, a condition has been raised.
 *
 *           FNAMESPEC_BUF_EXTRA (8) is sized to contain the terminating NULL.
 *           So the allocated buffer has room to concatenate 7 characters.
 */
static char *getFileSpecFromArg(RexxCallContext *context, CSTRING fSpec, size_t *fSpecLen,
                                size_t *fSpecBufLen, size_t argPos)
{
    size_t len = strlen(fSpec);
    if ( len == 0 )
    {
        nullStringException(context->threadContext, "SysFileTree", argPos);
        return NULL;
    }

    char *fileSpec = (char *)LocalAlloc(LPTR, len + FNAMESPEC_BUF_EXTRA);
    if ( fileSpec == NULL )
    {
        outOfMemoryException(context->threadContext);
        return NULL;
    }

    // Allocated buffer is zero filled (LPTR flag) already, no need to zero
    // terminate.
    memcpy(fileSpec, fSpec, len);

    if ( fileSpec[len - 1] == '\\' )
    {
        strcat(fileSpec, "*.*");
    }
    else if ( fileSpec[len - 1] == '.')
    {
        if ( len == 1 ||
             (len > 1  && (fileSpec[len - 2] == '\\' || fileSpec[len - 2] == '.')) )
        {
            strcat(fileSpec, "\\*.*");
        }
    }

    *fSpecLen    = len;
    *fSpecBufLen = len + FNAMESPEC_BUF_EXTRA;
    return fileSpec;
}

/**
 * This is a SysFileTree specific helper function.
 *
 * Allocates and returns a buffer large enough to contain the path to search
 * along.
 *
 *  We need a minimum size for the path buffer of at least MAX (264).  But the
 *  old code seemed to think fileSpecLen + FNAMESPEC_BUF_EXTRA could be longer
 *  than that.  I guess it could if the user put in a non-existent long file
 *  path.
 *
 *  The old code of checking fSpecLen is still used, but I'm unsure of its exact
 *  purpose.
 *
 * @param context
 * @param fSpecLen
 * @param pathLen
 *
 * @return A buffer the larger of MAX or fSpecLen + FNAMESPEC_BUF_EXTRA bytes in
 *         size.  Returns NULL on failure.
 *
 * @remarks  The caller is resposible for freeing the allocated memory.
 *
 *           LocalAlloc(), not malloc() is used for memory allocation.
 *
 *           Note that the path buffer is guarenteed to be FNAMESPEC_BUF_EXTRA
 *           (8) bytes larger than the fNameSpec buffer in the caller.  This is
 *           important in later checks for buffer overflow.
 */
static char *getPathBuffer(RexxCallContext *context, size_t fSpecLen, size_t *pathLen)
{
    size_t bufLen = MAX;

    if ( fSpecLen + FNAMESPEC_BUF_EXTRA > MAX )
    {
        bufLen = fSpecLen + FNAMESPEC_BUF_EXTRA;
    }

    *pathLen = bufLen;

    char *path = (char *)LocalAlloc(LPTR, bufLen);
    if ( path == NULL )
    {
        outOfMemoryException(context->threadContext);
    }

    return path;
}

/**
 * Tests for illegal file name characters in fSpec.
 *
 * @param fSpec
 *
 * @return bool
 *
 * @note  Double quotes in the file spec is not valid, spaces in file names do
 *        not need to be within quotes in the string passed to the Windows API.
 *
 *        A ':' is only valid if it is the second character. Technically a '*'
 *        and a '?' are not valid characters for a file name, but they are okay
 *        for fSpec. Same thing for '\' and '/', they are not valid in a file
 *        name, but they are valid in fSpec. A '/' is iffy. The Windows API
 *        accepts '/' as a directory separator, but most Rexx programmers
 *        probably don't know that.  Not sure if we should flag that as illegal
 *        or not.
 */
static bool illegalFileNameChars(char * fSpec)
{
    static char illegal[] = "<>|\"";

    for ( size_t i = 0; i < 4; i++ )
    {
        if ( strchr(fSpec, illegal[i]) != NULL )
        {
            return true;
        }
    }

    char *pos = strchr(fSpec, ':');
    if ( pos != NULL )
    {
        if ( ((int32_t)(pos - fSpec + 1)) != 2 )
        {
            return true;
        }
        if ( strchr(pos + 1, ':') != NULL )
        {
            return true;
        }
    }

    return false;
}
/**
 * SysFileTree() implementation.  Searches for files in a directory tree
 * matching the specified search pattern.
 *
 * @param  fSpec  [required] The search pattern, may contain glob characters
 *                 and full or partial path informattion. E.g., *.bat, or
 *                 ../../*.txt, or C:\temp.  May not contain illegal file name
 *                 characters which are: ", <, >, |, and :  The semicolon is
 *                 only legal if it is exactly the second character.  Do not use
 *                 a double quote in fSpec, it is not needed and is taken as a
 *                 character in a file name, which is an illegal character.
 *
 * @param  files  [required] A stem to contain the returned results.  On return,
 *                files.0 contains the count N of found files and files.1
 *                through files.N will contain the found files.
 *
 * @param  opts   [optional] Any combination of the following letters that
 *                specify how the search takes place, or how the returned found
 *                file line is formatted.  Case is not significant:
 *
 *                  'B' - Search for files and directories.
 *                  'D' - Search for directories only.
 *                  'F' - Search for files only.
 *                  'O' - Only output file names.
 *                  'S' - Recursively scan subdirectories.
 *                  'T' - Combine time & date fields into one.
 *                  'L' - Long time format
 *                  'I' - Case Insensitive search.
 *
 *                The defualt is 'B' using normal time (neither 'T' nor 'L'.)
 *                The 'I'option is meaningless on Windows.
 *
 * @param targetAttr  [optional] Target attribute mask.  Only files with these
 *                    attributes will be searched for.  The default is to ignore
 *                    the attributes of the files found, so all files found are
 *                    returned.
 *
 * @param newAttr     [optional] New attribute mask.  Each found file will have
 *                    its attributes set (changed) to match this mask.  The
 *                    default is to not change any attributes.
 *
 * @return  0 on success, non-zero on error.  For all errors, a condition is
 *          raised.
 *
 * @remarks  The original IBM code passed in fileSpec to recursiveFindFile(),
 *           but then never used it in recursiveFineFile.  So, that has been
 *           eliminated.
 *
 */
RexxRoutine5(uint32_t, SysFileTree, CSTRING, fSpec, RexxStemObject, files, OPTIONAL_CSTRING, opts,
             OPTIONAL_CSTRING, targetAttr, OPTIONAL_CSTRING, newAttr)
{
     uint32_t     result   = 1;        // Return value, 1 is an error.
     char        *fileSpec = NULL;     // File spec to search for.
     size_t       fSpecLen = 0;        // Length of the original fSpec string.
     size_t       fSpecBufLen = 0;     // Length of the allocated fSpec buffer.
     char        *path     = NULL;     // Path to search along.
     size_t       pathLen  = 0;        // Size of buffer holding path.
     RXTREEDATA   treeData = {0};      // Struct for data.

     context->SetStemArrayElement(files, 0, context->WholeNumber(0));

     treeData.files      = files;
     treeData.dFNameSpec = treeData.fNameSpec;
     treeData.nFNameSpec = FNAMESPEC_BUF_LEN;

     fileSpec = getFileSpecFromArg(context, fSpec, &fSpecLen, &fSpecBufLen, 1);
     if ( fileSpec == NULL )
     {
         goto done_out;
     }

     if ( illegalFileNameChars((char *)fSpec) )
     {
         result = ERROR_INVALID_NAME;
         goto done_out;
     }

     // Some, or all, of fileSpec will eventually be copied into
     // treeData.dFNameSpec. So, if we ensure that treeData.dFNameSpec is big
     // enough to hold fileSpec we do not need to worry about it any more.
     if ( fSpecBufLen >= FNAMESPEC_BUF_LEN )
     {
         if ( ! getBiggerBuffer(context, &treeData.dFNameSpec, &treeData.nFNameSpec, fSpecBufLen + 1, FNAMESPEC_BUF_LEN) )
         {
             goto done_out;
         }
     }

     path = getPathBuffer(context, fSpecLen, &pathLen);
     if ( path == NULL )
     {
         goto done_out;
     }

     uint32_t options = 0;
     if ( ! getOptionsFromArg(context, (char *)opts, &options, 3) )
     {
         goto done_out;
     }

     int32_t targetMask[5] = {0};    // Attribute mask of files to search for.
     int32_t newMask[5]    = {0};    // Attribute mask to set found files to.

     if ( ! getMaskFromArg(context, (char *)targetAttr, targetMask, 4) )
     {
         goto done_out;
     }

     if ( ! getMaskFromArg(context, (char *)newAttr, newMask, 5) )
     {
         goto done_out;
     }

     // Get the full path segment and the file name segment by expanding the
     // file specification string.  It seems highly unlikely, but possible, that
     // this could fail.
     if ( ! getPath(context, fileSpec, &path, treeData.dFNameSpec, &pathLen) )
     {
         goto done_out;
     }

     if ( recursiveFindFile(context, path, &treeData, targetMask, newMask, options) )
     {
         context->SetStemArrayElement(treeData.files, 0, context->WholeNumber(treeData.count));
         result = 0;
     }

done_out:
    safeLocalFree(fileSpec);
    safeLocalFree(path);
    if ( treeData.nFNameSpec != FNAMESPEC_BUF_LEN )
    {
        LocalFree(treeData.dFNameSpec);
    }
    return result;
}


/*************************************************************************
* Function:  SysGetKey                                                   *
*                                                                        *
* Syntax:    call SysGetKey [echo]                                       *
*                                                                        *
* Params:    echo - Either of the following:                             *
*                    'ECHO'   - Echo the inputted key (default).         *
*                    'NOECHO' - Do not echo the inputted key.            *
*                                                                        *
* Return:    The key striked.                                            *
*************************************************************************/

size_t RexxEntry SysGetKey(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  INT       tmp;                       /* Temp var used to hold      */
                                       /* keystroke value            */
  bool      echo = true;               /* Set to false if we         */
                                       /* shouldn't echo             */

  if (numargs > 1)                     /* too many arguments         */
    return INVALID_ROUTINE;            /* raise an error             */

  if (numargs == 1) {                  /* validate arguments         */
    if (!_stricmp(args[0].strptr, "NOECHO"))
      echo = false;
    else if (_stricmp(args[0].strptr, "ECHO"))
      return INVALID_ROUTINE;          /* Invalid option             */
  }
  if (ExtendedFlag) {                  /* if have an extended        */
    tmp = ExtendedChar;                /* get the second char        */
    ExtendedFlag = false;              /* do a real read next time   */
  }
  else {
    tmp = _getch();                    /* read a character           */

                                       /* If a function key or arrow */
    if ((tmp == 0x00) || (tmp == 0xe0)) {
      ExtendedChar = _getch();         /* Read another character     */
      ExtendedFlag = true;
    }
    else
      ExtendedFlag = false;
  }
  if (echo)                            /* echoing?                   */
    _putch(tmp);                       /* write the character back   */

  wsprintf(retstr->strptr, "%c", tmp);
  retstr->strlength = 1;

  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysIni                                                      *
*                                                                        *
* Syntax:    call SysIni [inifile], app [,key/stem] [,val/stem]          *
*                                                                        *
* Params:    inifile - INI file from which to query or write info.  The  *
*                       default is the current user INI file.            *
*            app     - Application title to work with.  May be either    *
*                       of the following:                                *
*                        'ALL:' - All app titles will be returned in the *
*                                  stem variable specified by the next   *
*                                  parameter.                            *
*                        other  - Specific app to work with.             *
*            key     - Key to work with.  May be any of the following:   *
*                        'ALL:'    - All key titles will be returned in  *
*                                     the stem variable specified by the *
*                                     next parameter.                    *
*                        'DELETE:' - All keys associated with the app    *
*                                     will be deleted.                   *
*                        other     - Specific key to work with.          *
*            val     - Key to work with. May be either of the following: *
*                        'DELETE:' - Delete app/key pair.                *
*                        other     - Set app/key pair info to data spec- *
*                                     ified.                             *
*            stem    - Name of stem variable in which to store results.  *
*                      Stem.0 = Number found (n).                        *
*                      Stem.1 - Stem.n = Entries found.                  *
*                                                                        *
* Return:    other          - Info queried from specific app/key pair.   *
*            ''             - Info queried and placed in stem or data    *
*                              deleted successfully.                     *
*            ERROR_NOMEM    - Out of memory.                             *
*            ERROR_RETSTR   - Error opening INI or querying/writing info.*
*************************************************************************/

size_t RexxEntry SysIni(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    size_t      x;                       /* Temp counter               */
    size_t      len;                     /* Len var used when creating */
                                         /* stem                       */
    size_t      lSize;                   /* Size of queried info buffer*/
                                         /* area                       */
    LONG        Error = FALSE;           /* Set to true if error       */
                                         /* encountered                */
    bool        WildCard = false;        /* Set to true if a wildcard  */
                                         /* operation                  */
    bool        QueryApps;               /* Set to true if a query     */
                                         /* operation                  */
    bool        terminate = true;        /* perform WinTerminate call  */
    RXSTEMDATA  ldp;                     /* local data                 */
    size_t      buffersize;              /* return buffer size         */


    buffersize = retstr->strlength;      /* save default buffer size   */
    retstr->strlength = 0;               /* set return value           */
    const char *Key = "";
    /* validate arguments         */
    if (numargs < 2 || numargs > 4 || !RXVALIDSTRING(args[1]))
    {
        return INVALID_ROUTINE;
    }
    /* get pointers to args       */
    const char *IniFile = args[0].strptr;
    if (!RXVALIDSTRING(args[0]))         /* not specified?             */
        IniFile = "WIN.INI";               /* default to WIN.INI         */
    const char *App = args[1].strptr;

    if (numargs >= 3 && args[2].strptr)
        Key = args[2].strptr;

    const char *Val = NULL;
    if (numargs == 4)
        Val = args[3].strptr;
    /* Check KEY and APP values   */
    /* for "WildCard"             */
    if (!_stricmp(App, "ALL:"))
    {
        App = "";
        QueryApps = true;
        WildCard = true;

        if (numargs != 3)
            return INVALID_ROUTINE;          /* Error - Not enough args    */
        else
            x = 2;                           /* Arg number of STEM variable*/
    }

    else if (!_stricmp(Key, "ALL:"))
    {
        Key = "";
        Val = "";
        QueryApps = false;
        WildCard = true;

        if (numargs != 4)
            return INVALID_ROUTINE;          /* Error - Not enough args    */

        else
            x = 3;                           /* Arg number of STEM variable*/
    }
    /* If this is a "WildCard     */
    /* search, then allocate mem  */
    /* for stem struct and get the*/
    /* stem name                  */
    if (WildCard == true)
    {

        ldp.count = 0;                     /* get the stem variable name */
        strcpy(ldp.varname, args[x].strptr);
        ldp.stemlen = args[x].strlength;
        /* uppercase the name         */
        memupper(ldp.varname, strlen(ldp.varname));

        if (ldp.varname[ldp.stemlen-1] != '.')
            ldp.varname[ldp.stemlen++] = '.';
    }

    char *returnVal = NULL;
    /* get value if is a query    */
    if ((numargs == 3 && _stricmp(Key, "DELETE:")) ||
        WildCard == true)
    {
        lSize = 0x0000ffffL;
        /* Allocate a large buffer    */
        returnVal = (char *)GlobalAlloc(GPTR, lSize);
        if (returnVal == NULL)
        {
            BUILDRXSTRING(retstr, ERROR_NOMEM);
            return VALID_ROUTINE;
        }

        if (WildCard && QueryApps)
            /* Retrieve the names of all  */
            /* applications.              */
            lSize = GetPrivateProfileString(NULL, NULL, "", returnVal, (DWORD)lSize, IniFile);
        else if (WildCard && !QueryApps)
            /* Retrieve all keys for an   */
            /* application                */
            lSize = GetPrivateProfileString(App, NULL, "", returnVal, (DWORD)lSize, IniFile);
        else
            /* Retrieve a single key value*/
            lSize = GetPrivateProfileString(App, Key, "", returnVal, (DWORD)lSize, IniFile);

        if (lSize <= 0)
        {
            Error = true;
            BUILDRXSTRING(retstr, ERROR_RETSTR);
        }
        else if (WildCard == false)
        {
            if (lSize > buffersize)
                retstr->strptr = (PCH)GlobalAlloc(GMEM_FIXED, lSize);
                if (retstr->strptr == NULL)
                {
                    GlobalFree(returnVal);  /* release buffer */
                    BUILDRXSTRING(retstr, ERROR_NOMEM);
                    return VALID_ROUTINE;
                }
            memcpy(retstr->strptr, returnVal, lSize);
            retstr->strlength = lSize;
        }
    }
    else
    {                               /* Set or delete Key          */

        if (!_stricmp(Key, "DELETE:") || (numargs == 2) || !RXVALIDSTRING(args[2]))
            /* Delete application and all */
            /* associated keys            */
            Error = !WritePrivateProfileString(App, NULL, NULL, IniFile);
        else if (!_stricmp(Val, "DELETE:") ||
                 !RXVALIDSTRING(args[3]))
            /* Delete a single key        */
            Error = !WritePrivateProfileString(App, Key, NULL, IniFile);
        else
        {
            lSize = args[3].strlength;
            /* Set a single key value     */
            Error = !WritePrivateProfileString(App, Key, Val, IniFile);
        }

        if (Error)
        {
            BUILDRXSTRING(retstr, ERROR_RETSTR);
        }
        else
            retstr->strlength = 0;           /* just return a null string  */
    }

    /******************************************
    * If this was a wildcard search, change   *
    * the Val variable from one long string   *
    * of values to a REXX stem variable.      *
    ******************************************/

    if (WildCard == true)
    {              /* fill stem variable         */

        if (Error == false)
        {
            x = 0;
            ldp.count = 0;

            do
            {
                /* Copy string terminated by \0 to Temp.  Last string will end     */
                /* in \0\0 and thus have a length of 0.                            */
                len = 0;

                const char *next = &returnVal[x]; /* point to string            */
                len = strlen(next);            /* get string length          */
                                               /* if non-zero length, then   */
                                               /* set the stem element       */
                if (len != 0)
                {
                    x += (len+1);                /* Increment pointer past the */
                                                 /* new string                 */
                    strcpy(ldp.ibuf, next);
                    ldp.vlen = len;
                    ldp.count++;
                    ltoa((long)ldp.count, ldp.varname+ldp.stemlen, 10);

                    if (ldp.ibuf[ldp.vlen-1] == '\n')
                        ldp.vlen--;
                    ldp.shvb.shvnext = NULL;
                    ldp.shvb.shvname.strptr = ldp.varname;
                    ldp.shvb.shvname.strlength = strlen(ldp.varname);
                    ldp.shvb.shvvalue.strptr = ldp.ibuf;
                    ldp.shvb.shvvalue.strlength = ldp.vlen;
                    ldp.shvb.shvnamelen = ldp.shvb.shvname.strlength;
                    ldp.shvb.shvvaluelen = ldp.vlen;
                    ldp.shvb.shvcode = RXSHV_SET;
                    ldp.shvb.shvret = 0;
                    if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
                    {
                        if (returnVal != NULL)
                        {
                            GlobalFree(returnVal);  /* release buffer */
                        }
                        return INVALID_ROUTINE;    /* error on non-zero          */
                    }
                }
            }

            while (returnVal[x] != '\0');
        }

        else
            ldp.count = 0;

        if (returnVal != NULL)
        {
            GlobalFree(returnVal);
            returnVal = NULL;
        }

        /* set number returned        */
        ltoa((long)ldp.count, ldp.ibuf, 10);
        ldp.varname[ldp.stemlen] = '0';
        ldp.varname[ldp.stemlen+1] = 0;
        ldp.shvb.shvnext = NULL;
        ldp.shvb.shvname.strptr = ldp.varname;
        ldp.shvb.shvname.strlength = ldp.stemlen+1;
        ldp.shvb.shvnamelen = ldp.stemlen+1;
        ldp.shvb.shvvalue.strptr = ldp.ibuf;
        ldp.shvb.shvvalue.strlength = strlen(ldp.ibuf);
        ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
        ldp.shvb.shvcode = RXSHV_SET;
        ldp.shvb.shvret = 0;
        if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
            return INVALID_ROUTINE;          /* error on non-zero          */

    }                                    /* * End - IF (Wildcard ... * */
    if (returnVal != NULL)
    {
        GlobalFree(returnVal);  /* release buffer                              */
    }

    return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysLoadFuncs                                                *
*                                                                        *
* Syntax:    call SysLoadFuncs [option]                                  *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

size_t RexxEntry SysLoadFuncs(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    // this is a NOP now
    retstr->strlength = 0;               /* set return value           */
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysMkDir                                                    *
*                                                                        *
* Syntax:    call SysMkDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be created.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from CreateDirectory()                          *
*************************************************************************/

RexxRoutine1(int, SysMkDir, CSTRING, dir)
{
    return CreateDirectory(dir, NULL) != 0 ? 0 : GetLastError();
}


/*************************************************************************
* Function:  SysGetErrortext                                             *
*                                                                        *
* Syntax:    call SysGetErrortext errnumber                              *
*                                                                        *
* Params:    errnumber - error number to be described                    *
*                                                                        *
* Return:    Description or empty string                                 *
*************************************************************************/

size_t RexxEntry SysGetErrortext(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    DWORD  errnum;
    char  *errmsg;
    size_t length;

    if (numargs != 1)
    {
        /* If no args, then its an    */
        /* incorrect call             */
        return INVALID_ROUTINE;
    }

    errnum = atoi(args[0].strptr);
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,errnum,0,(LPSTR)&errmsg,64,NULL) == 0)
    {
        retstr->strptr[0] = 0x00;
    }
    else
    {                               /* succeeded                  */
        length = strlen(errmsg);

        // FormatMessage returns strings with trailing CrLf, which we want removed
        if (length >= 1 && errmsg[length - 1] == 0x0a)
        {
          errmsg[length - 1] = 0x00;
          length--;
        }
        if (length >= 1 && errmsg[length - 1] == 0x0d)
        {
          errmsg[length - 1] = 0x00;
          length--;
        }

        if (length >= retstr->strlength)
        {
            retstr->strptr = (PCH)GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, strlen(errmsg+1));
        }
        strcpy(retstr->strptr,errmsg);
        LocalFree(errmsg);
    }
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysWinEncryptFile (W2K only)                                *
*                                                                        *
* Syntax:    call SysWinEncryptFile filename                             *
*                                                                        *
* Params:    filename - file to be encrypted                             *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from EncryptFile()                              *
*************************************************************************/

size_t RexxEntry SysWinEncryptFile(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    ULONG  rc;                           /* Ret code of func           */
    OSVERSIONINFO vi;

    if (numargs != 1)
    {
        /* If no args, then its an    */
        /* incorrect call             */
        return INVALID_ROUTINE;
    }

    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (rc = GetVersionEx(&vi))
    {
        /* allow this only on W2K or newer */
        if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT && vi.dwMajorVersion > 4)
        {
            rc = EncryptFile(args[0].strptr);
        }
        else
        {
            rc = 0;
            SetLastError(ERROR_CANNOT_MAKE);
        }
    }
    if (rc)
    {
        sprintf(retstr->strptr, "%d", 0);
    }
    else
    {
        sprintf(retstr->strptr, "%d", GetLastError());
    }
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SysWinDecryptFile (W2K only)                                *
*                                                                        *
* Syntax:    call SysWinDecryptFile filename                             *
*                                                                        *
* Params:    filename - file to be decrypted                             *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from DecryptFile()                              *
*************************************************************************/

size_t RexxEntry SysWinDecryptFile(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  ULONG  rc;                           /* Ret code of func           */
  OSVERSIONINFO vi;

  if (numargs != 1)
                                       /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;

    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  if (rc = GetVersionEx(&vi)) {
    /* allow this only on W2K or newer */
    if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT && vi.dwMajorVersion > 4)
      rc = DecryptFile(args[0].strptr,0);
    else {
      rc = 0;
      SetLastError(ERROR_CANNOT_MAKE);
    }
  }

  if (rc)
    sprintf(retstr->strptr, "%d", 0);
  else
    sprintf(retstr->strptr, "%d", GetLastError());
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysWinVer                                                   *
*                                                                        *
* Syntax:    call SysWinVer                                              *
*                                                                        *
* Return:    Windows Version                                             *
*************************************************************************/

size_t RexxEntry SysWinVer(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  OSVERSIONINFO vi;                    /* Return RXSTRING            */
    char chVerBuf[12];

  vi.dwOSVersionInfoSize = sizeof(vi); /* if not set --> violation error */

  if (numargs != 0)                    /* validate arg count         */
    return INVALID_ROUTINE;

  GetVersionEx(&vi);                /* get version with extended api */
  if (vi.dwPlatformId == VER_PLATFORM_WIN32s)
    strcpy(chVerBuf, "Windows");       /* Windows 3.1                */
  else
    if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
      strcpy(chVerBuf, "WindowsNT");   /* Windows NT               */
  else strcpy(chVerBuf, "Windows95");  /* Windows 95               */

                                       /* format into the buffer     */
  wsprintf(retstr->strptr,"%s %lu.%02lu",
             chVerBuf,
             vi.dwMajorVersion,
             vi.dwMinorVersion);

  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SysVersion                                                  *
*                                                                        *
* Syntax:    Say  SysVersion                                             *
*                                                                        *
* Return:    Operating System and Version                                *
*************************************************************************/

size_t RexxEntry SysVersion(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  /* this is only an alias for SysWinVer */
  return SysWinVer(name, numargs, args, queuename, retstr);
}


/*************************************************************************
* Function:  SysRmDir                                                    *
*                                                                        *
* Syntax:    call SysRmDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be removed.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from RemoveDirectory()                          *
*************************************************************************/

RexxRoutine1(int, SysRmDir, CSTRING, dir)
{
    return RemoveDirectory(dir) != 0 ? 0 : GetLastError();
}


/*************************************************************************
* Function:  SysSearchPath                                               *
*                                                                        *
* Syntax:    call SysSearchPath path, file [, options]                   *
*                                                                        *
* Params:    path - Environment variable name which specifies a path     *
*                    to be searched (ie 'PATH', 'DPATH', etc).           *
*            file - The file to search for.                              *
*            options -  'C' - Current directory search first (default).  *
*                       'N' - No Current directory search. Only searches *
*                             the path as specified.                     *
*                                                                        *
* Return:    other  - Full path and filespec of found file.              *
*            ''     - Specified file not found along path.               *
*************************************************************************/

size_t RexxEntry SysSearchPath(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    char     szFullPath[_MAX_PATH];      /* returned file name         */
    char     szCurDir[_MAX_PATH];        /* current directory          */
    char     *szEnvStr = NULL;

    LPTSTR pszOnlyFileName;              /* parm for searchpath        */
    LPTSTR lpPath = NULL;                /* ptr to search path+        */
    UINT   errorMode;

    /* validate arguments         */
    if (numargs < 2 || numargs > 3 ||
        !RXVALIDSTRING(args[0]) ||
        !RXVALIDSTRING(args[1]))
    {
        return INVALID_ROUTINE;
    }

    char opt = 'C'; // this is the default
    if (numargs == 3)
    {                  /* process options            */
        opt = toupper(args[2].strptr[0]);
        if (opt != 'C' && opt != 'N')
        {
            return INVALID_ROUTINE;          /* Invalid option             */
        }
    }

    szEnvStr = (LPTSTR) malloc(sizeof(char) * MAX_ENVVAR);
    if (szEnvStr != NULL)
    {
        DWORD charCount = GetEnvironmentVariable(args[0].strptr, szEnvStr, MAX_ENVVAR);
        if (charCount == 0)
        {
            *szEnvStr = '\0';
        }
        else if (charCount > MAX_ENVVAR)
        {
            szEnvStr = (LPTSTR) realloc(szEnvStr, sizeof(char) * charCount);
            if (szEnvStr != NULL)
            {
                DWORD charCount2 = GetEnvironmentVariable(args[0].strptr, szEnvStr, charCount);
                if (charCount2 == 0 || charCount2 > charCount)
                {
                    *szEnvStr = '\0';
                }
            }
        }
    }

    if (opt == 'N')
    {
        lpPath = (szEnvStr == NULL) ? NULL : strdup(szEnvStr);
    }
    else if (opt == 'C')
    {
        /* search current directory   */
        DWORD charCount = GetCurrentDirectory(_MAX_PATH, szCurDir);
        if (charCount == 0 || charCount > _MAX_PATH)
        {
            szCurDir[0] = '\0';
        }

        if (szEnvStr != NULL)
        {
            lpPath = (LPTSTR) malloc(sizeof(char) * (strlen(szCurDir) + 1 + strlen(szEnvStr) + 1));
            if (lpPath != NULL)
            {
                strcpy(lpPath, szCurDir);
                strcat(lpPath, ";");
                strcat(lpPath, szEnvStr);
            }
        }
        else
        {
            lpPath = strdup(szCurDir);
        }
    }

    /* use DosSearchPath          */

    DWORD charCount = 0;
    if (lpPath != NULL)
    {
        errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        charCount = SearchPath(
                           (LPCTSTR)lpPath,              /* path srch, NULL will+      */
                           (LPCTSTR)args[1].strptr,      /* address if filename        */
                           NULL,                         /* filename contains .ext     */
                           _MAX_PATH,                    /* size of fullname buffer    */
                           szFullPath,                   /* where to put results       */
                           &pszOnlyFileName);
        SetErrorMode(errorMode);
    }
    if (charCount == 0 || charCount > _MAX_PATH)
    {
        szFullPath[0]='\0';              /* set to NULL if failure     */
    }

    BUILDRXSTRING(retstr, szFullPath);   /* pass back result           */
    free(szEnvStr);
    free(lpPath);
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysSleep                                                    *
*                                                                        *
* Syntax:    call SysSleep secs                                          *
*                                                                        *
* Params:    secs - Number of seconds to sleep.                          *
*                   must be in the range 0 .. 2147483                    *
*                                                                        *
* Return:    0                                                           *
*************************************************************************/
RexxRoutine1(int, SysSleep, RexxStringObject, delay)
{
  double seconds;
  // try to convert the provided delay to a valid floating point number
  if (context->ObjectToDouble(delay, &seconds) == 0 ||
      isnan(seconds) || seconds == HUGE_VAL || seconds == -HUGE_VAL)
  {
      // 88.902 The &1 argument must be a number; found "&2"
      context->RaiseException2(Rexx_Error_Invalid_argument_number, context->String("delay"), delay);
      return 1;
  }

  // according to MSDN the maximum is USER_TIMER_MAXIMUM (0x7FFFFFFF) milliseconds,
  // which translates to 2147483.647 seconds
  if (seconds < 0.0 || seconds > 2147483.0)
  {
      // 88.907 The &1 argument must be in the range &2 to &3; found "&4"
      context->RaiseException(Rexx_Error_Invalid_argument_range,
          context->ArrayOfFour(context->String("delay"),
          context->String("0"), context->String("2147483"), delay));
      return 1;
  }

  // convert to milliseconds, no overflow possible
  LONG milliseconds = (LONG) (seconds * 1000);

  /** Using Sleep with a long timeout risks sleeping on a thread with a message
   *  queue, which can make the system sluggish, or possibly deadlocked.  If the
   *  sleep is longer than 333 milliseconds use a window timer to avoid this
   *  risk.
   */
  if ( milliseconds > 333 )
  {
      if ( !(SetTimer(NULL, 0, milliseconds, (TIMERPROC) SleepTimerProc)) )
      {
          // no timer available, need to abort
          context->RaiseException1(Rexx_Error_System_resources_user_defined,
              context->String("System resources exhausted: cannot start timer"));
          return 1;
      }

      MSG msg;
      while ( GetMessage (&msg, NULL, 0, 0) )
      {
          if ( msg.message == OM_WAKEUP )  /* If our message, exit loop       */
              break;
          TranslateMessage( &msg );
          DispatchMessage ( &msg );
      }
  }
  else
  {
      Sleep(milliseconds);
  }

  return 0;
}

/*********************************************************************
 *                                                                   *
 *  Routine   : SleepTimerProc                                       *
 *                                                                   *
 *  Purpose   : callback routine for SetTimer set in SysSleep        *
 *  Notes     :                                                      *
 *  Arguments : hwnd - window handle                                 *
 *              uMsg - WM_TIMER message                              *
 *              idEvent - timer identifier                           *
 *              dwtime - current system time                         *
 *  Returns   :                                                      *
 *                                                                   *
 *********************************************************************/
 VOID CALLBACK SleepTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
   DWORD ThreadId;
   KillTimer(NULL, idEvent);       /* kill the timer that just ended */
   ThreadId = GetCurrentThreadId();
   PostThreadMessage(ThreadId, OM_WAKEUP, 0 , 0L); /* send ourself the wakeup message*/
 }

/*************************************************************************
* Function:  SysTempFileName                                             *
*                                                                        *
* Syntax:    call SysTempFileName template [,filler]                     *
*                                                                        *
* Params:    template - Description of filespec desired.  For example:   *
*                        C:\TEMP\FILE.???                                *
*            filler   - A character which when found in template will be *
*                        replaced with random digits until a unique file *
*                        or directory is found.  The default character   *
*                        is '?'.                                         *
*                                                                        *
* Return:    other - Unique file/directory name.                         *
*            ''    - No more files exist given specified template.       *
*************************************************************************/

size_t RexxEntry SysTempFileName(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    CHAR   filler;                       /* filler character           */

    if (numargs < 1 ||                   /* validate arguments         */
        numargs > 2 ||
        !RXVALIDSTRING(args[0]) ||
        args[0].strlength > 512)
        return INVALID_ROUTINE;

    if (numargs == 2 &&                  /* get filler character       */
        !RXNULLSTRING(args[1]))
    {
        if (args[1].strlength != 1)        /* must be one character      */
            return INVALID_ROUTINE;
        filler = args[1].strptr[0];
    }
    else
    {
        filler = '?';
    }
    /* get the file id            */
    GetUniqueFileName(const_cast<char *>(args[0].strptr), filler, retstr->strptr);
    retstr->strlength = strlen(retstr->strptr);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysTextScreenRead                                           *
*                                                                        *
* Syntax:    call SysTextScreenRead row, col [,len]                      *
*                                                                        *
* Params:    row - Horizontal row on the screen to start reading from.   *
*                   The row at the top of the screen is 0.               *
*            col - Vertical column on the screen to start reading from.  *
*                   The column at the left of the screen is 0.           *
*            len - The number of characters to read.  The default is the *
*                   rest of the screen.                                  *
*                                                                        *
* Return:    Characters read from text screen.                           *
*************************************************************************/
RexxRoutine3(RexxStringObject, SysTextScreenRead, int, row, int, col, OPTIONAL_int, len)
{
    int    lPos,lPosOffSet;              /* positioning                */
                                         /* (132x50)                   */
    int    lBufferLen = 16000;           /* default: 200x80 characters */

    COORD coordLine;                     /* coordinates of where to    */
                                         /* read characters from       */
    DWORD dwCharsRead,dwSumCharsRead;    /* Handle to Standard Out     */
    HANDLE hStdout;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
    {
        context->InvalidRoutine();
        return NULLOBJECT;
    }

    if (argumentOmitted(3))               /* check the length           */
    {
        len = csbiInfo.dwSize.Y * csbiInfo.dwSize.X;
    }

    coordLine.X = (short)col;
    coordLine.Y = (short)row;

    char buffer[256];
    char *ptr = buffer;

    if (len > sizeof(buffer))
    {
        // allocate a new buffer
        ptr = (char *)malloc(len);
        if (ptr == NULL)
        {
            context->InvalidRoutine();
            return NULL;
        }
    }

    if (len < lBufferLen)
    {
        lBufferLen = len;
    }

    lPos = 0;                                     /* current position */
    lPosOffSet = row * csbiInfo.dwSize.X + col;   /* add offset if not started at beginning */
    dwSumCharsRead = 0;

    while (lPos < len )
    {

        if (!ReadConsoleOutputCharacter(hStdout, &ptr[lPos], lBufferLen, coordLine, &dwCharsRead))
        {
            if (ptr != buffer) {
                free(ptr);
            }
            context->InvalidRoutine();
            return NULL;
        }


        lPos = lPos + lBufferLen;
        coordLine.Y = (short)((lPos + lPosOffSet) / csbiInfo.dwSize.X);
        coordLine.X = (short)((lPos + lPosOffSet) % csbiInfo.dwSize.X);
        dwSumCharsRead = dwSumCharsRead + dwCharsRead;
    }

    RexxStringObject result = context->NewString(ptr, dwSumCharsRead);
    if (ptr != buffer) {
        free(ptr);
    }
    return result;
}

/*************************************************************************
* Function:  SysTextScreenSize                                           *
*                                                                        *
* Syntax:    call SysTextScreenSize [option], [rows, colummns]          *
*            call SysTextScreenSize [option], [top, left, bottom, right] *
*                                                                        *
* Params:    option - "BUFFERSIZE", "WINDOWRECT", "MAXWINDOWSIZE"        *
*               "BUFFERSIZE" (default) return or set console buffer size *
*               "WINDOWRECT" return or set windows position              *
*               "MAXWINDOWSIZE" return maximum window size               *
*            lines, columns - set buffer size to lines by columns        *
*            top, left, bottom, right - set window size and position     *
*                                                                        *
* Return:    "BUFFERSIZE" or "MAXWINDOWSIZE": rows columns               *
*            "WINDOWRECT": top left bottom right                         *
*************************************************************************/

RexxRoutine5(RexxStringObject, SysTextScreenSize,
    OPTIONAL_CSTRING, optionString,
    OPTIONAL_stringsize_t, rows, OPTIONAL_stringsize_t, columns,  
    OPTIONAL_stringsize_t, rows2, OPTIONAL_stringsize_t, columns2)  
{
    // check for valid option
    typedef enum { BUFFERSIZE, WINDOWRECT, MAXWINDOWSIZE } console_option;
    console_option option;
    if (optionString == NULL || stricmp(optionString, "BUFFERSIZE") == 0)
    {
        option = BUFFERSIZE;
    }
    else if (stricmp(optionString, "WINDOWRECT") == 0)
    {
        option = WINDOWRECT;
    }
    else if (stricmp(optionString, "MAXWINDOWSIZE") == 0)
    {
        option = MAXWINDOWSIZE;
    }
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    // check for valid SET arguments: either none, or two more, or four more
    size_t setArgs;
    bool omitted45 = argumentOmitted(4) && argumentOmitted(5);
    bool exists23 = argumentExists(2) && argumentExists(3);
    if (argumentOmitted(2) && argumentOmitted(3) && omitted45)
    {
        setArgs = 0;
    }
    else if (exists23 && omitted45)
    {
        setArgs = 2;
    }
    else if (exists23 && argumentExists(4) && argumentExists(5))
    {
        setArgs = 4;
    }
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    // check that all SET arguments fit a SHORT
    if (!(setArgs == 0 ||
         (setArgs == 2 && rows <= SHRT_MAX && columns <= SHRT_MAX) ||
         (setArgs == 4 && rows <= SHRT_MAX && columns <= SHRT_MAX && rows2 <= SHRT_MAX && columns2 <= SHRT_MAX)))
    {
        context->InvalidRoutine();
        return 0;
    }

    // real work starts here
    CONSOLE_SCREEN_BUFFER_INFO csbi; // console screen buffer information
    char buffer[100];

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (setArgs == 0)
    {
        // this is a GET requset, retrieve console information
        if (GetConsoleScreenBufferInfo(hStdout, &csbi) == NULL)
        {
            // console not in character mode, return two or four zeroes
            return context->NewStringFromAsciiz(option == WINDOWRECT ? "0 0 0 0" : "0 0");
        }
    }

    if (option == BUFFERSIZE && setArgs == 0)
    {
        // this is a BUFFERSIZE GET, returns two values
        sprintf(buffer, "%d %d", csbi.dwSize.Y, csbi.dwSize.X);
    }
    else if (option == WINDOWRECT && setArgs == 0)
    {
        // this is a WINDOWRECT GET, returns four values
        sprintf(buffer, "%d %d %d %d", csbi.srWindow.Top, csbi.srWindow.Left, csbi.srWindow.Bottom, csbi.srWindow.Right);
    }
    else if (option == MAXWINDOWSIZE && setArgs == 0)
    {
        // this is a MAXWINDOWSIZE GET, returns two values
        sprintf(buffer, "%d %d", csbi.dwMaximumWindowSize.Y, csbi.dwMaximumWindowSize.X);
    }
    else if (option == BUFFERSIZE && setArgs == 2)
    {
        // this is a BUFFERSIZE SET, requires two more arguments
        COORD consoleBuffer;
        consoleBuffer.Y = (SHORT)rows;
        consoleBuffer.X = (SHORT)columns;
        BOOL code = SetConsoleScreenBufferSize(hStdout, consoleBuffer);
        sprintf(buffer, "%d", code == 0 ? GetLastError() : 0);
    }
    else if (option == WINDOWRECT  && setArgs == 4)
    {
        // this is a WINDOWRECT  SET, requires four more arguments
        SMALL_RECT consoleWindow;
        consoleWindow.Top =    (SHORT)rows;
        consoleWindow.Left =   (SHORT)columns;
        consoleWindow.Bottom = (SHORT)rows2;
        consoleWindow.Right =  (SHORT)columns2;
        BOOL code = SetConsoleWindowInfo(hStdout, 1, &consoleWindow);
        sprintf(buffer, "%d", code == 0 ? GetLastError() : 0);
    }
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    // return the buffer as result
    return context->NewStringFromAsciiz(buffer);
}

/*************************************************************************
* Function:  RxWinExec                                                   *
*                                                                        *
* Syntax:    call RxWinExec command,CmdShow                              *
*                                                                        *
*                                                                        *
* Parms:     command    - program to execute                             *
*            CmdShow    - Any of the SW_ type values in winuser.h        *
*                         SW_SHOW                 5                      *
*                         SW_HIDE                 0                      *
*                         SW_MINIMIZE etc...      6                      *
*                         numeric values...                              *
*                                                                        *
* Return:    Process ID or Error code                                    *
*************************************************************************/

size_t RexxEntry RxWinExec(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

    int         CmdShow;                 /* show window style flags    */
    int         index;                   /* table index                */
    ULONG       pid;                     /* PID or error return code   */
    size_t      length;                  /* length of option           */
    STARTUPINFO si;
    PROCESS_INFORMATION procInfo;

    // Show window types.
    PSZ    show_styles[] =
    {
        "SHOWNORMAL",
        "SHOWNOACTIVATE",
        "SHOWMINNOACTIVE",
        "SHOWMINIMIZED",
        "SHOWMAXIMIZED",
        "HIDE",
        "MINIMIZE"
    };

    // Show window styles.
    ULONG  show_flags[] =
    {
        SW_SHOWNORMAL,
        SW_SHOWNOACTIVATE,
        SW_SHOWMINNOACTIVE,
        SW_SHOWMINIMIZED,
        SW_SHOWMAXIMIZED,
        SW_HIDE,
        SW_MINIMIZE
    };

    BUILDRXSTRING(retstr, NO_UTIL_ERROR);  /* pass back result           */

    // Should be 1 or 2 args.
    if ( numargs < 1 || numargs > 2 || !RXVALIDSTRING(args[0]) ||
         (numargs == 2 && !RXVALIDSTRING(args[1])) || args[0].strlength > MAX_CREATEPROCESS_CMDLINE )
    {
        return INVALID_ROUTINE;
    }

    // Show type can be one and only one of the SW_XXX constants.
    if ( numargs < 2 || args[1].strptr == NULL )
    {
        CmdShow = SW_SHOWNORMAL;
    }
    else
    {
        // Get length of option and search the style table.
        length = args[1].strlength;
        for ( index = 0; index < sizeof(show_styles)/sizeof(PSZ); index++ )
        {
            if ( length == strlen(show_styles[index]) && memicmp(args[1].strptr, show_styles[index], length) == 0 )
            {
                CmdShow = show_flags[index];
                break;
            }
        }

        if ( index == sizeof(show_styles)/sizeof(PSZ) )
        {
            // User sent an argument, but not the right one.
            return INVALID_ROUTINE;
        }
    }

    ZeroMemory(&procInfo, sizeof(procInfo));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = (WORD)CmdShow;

    if ( CreateProcess(NULL, (LPSTR)args[0].strptr, NULL, NULL, FALSE, 0, NULL,
                       NULL, &si, &procInfo ) )
    {
        pid = procInfo.dwProcessId;
    }
    else
    {
        pid = GetLastError();
        if ( pid > 31 )
        {
            // Maintain compatibility to versions < ooRexx 3.1.2
            pid = (ULONG)-((int)pid);
        }
    }

    // Return value as string.
    sprintf(retstr->strptr, "%d", pid);
    retstr->strlength = strlen(retstr->strptr);

    // Close process / thread handles as they are not used / needed.
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysAddRexxMacro                                             *
*                                                                        *
* Syntax:    result = SysAddRexxMacro(name, file, <order>)               *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            file   - file containing the loaded macro                   *
*            order  - Either 'B'efore or 'A'fter.  The default is 'B'    *
*                                                                        *
* Return:    return code from RexxAddMacro                               *
*************************************************************************/

RexxRoutine3(int, SysAddRexxMacro, CSTRING, name, CSTRING, file, OPTIONAL_CSTRING, option)
{
    size_t position;         /* added position             */

    position = RXMACRO_SEARCH_BEFORE;    /* set default search position*/
    if (option != NULL)                  /* have an option?            */
    {
        switch (*option)
        {
            case 'B':     // 'B'efore
            case 'b':
                position = RXMACRO_SEARCH_BEFORE;
                break;

            case 'A':     // 'A'fter
            case 'a':
                position = RXMACRO_SEARCH_AFTER;
                break;

            default:
                context->InvalidRoutine();
                return 0;
        }
    }
    /* try to add the macro       */
    return(int)RexxAddMacro(name, file, position);
}

/*************************************************************************
* Function:  SysReorderRexxMacro                                         *
*                                                                        *
* Syntax:    result = SysReorderRexxMacro(name, order)                   *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            order  - Either 'B'efore or 'A'fter.                        *
*                                                                        *
* Return:    return code from RexxReorderMacro                           *
*************************************************************************/

RexxRoutine2(int, SysReorderRexxMacro, CSTRING, name, CSTRING, option)
{
    size_t position;        /* added position             */

    switch (*option)
    {
        case 'B':     // 'B'efore
        case 'b':
            position = RXMACRO_SEARCH_BEFORE;
            break;

        case 'A':     // 'A'fter
        case 'a':
            position = RXMACRO_SEARCH_AFTER;
            break;

        default:
            context->InvalidRoutine();
            return 0;
    }
    return(int)RexxReorderMacro(name, position);
}

/*************************************************************************
* Function:  SysDropRexxMacro                                            *
*                                                                        *
* Syntax:    result = SysDropRexxMacro(name)                             *
*                                                                        *
* Params:    name   - name of the macro space function                   *
*                                                                        *
* Return:    return code from RexxDropMacro                              *
*************************************************************************/

RexxRoutine1(int, SysDropRexxMacro, CSTRING, name)
{
   return (int)RexxDropMacro(name);
}

/*************************************************************************
* Function:  SysQueryRexxMacro                                           *
*                                                                        *
* Syntax:    result = SysQueryRexxMacro(name)                            *
*                                                                        *
* Params:    name   - name of the macro space function                   *
*                                                                        *
* Return:    position of the macro ('B' or 'A'), returns null for errors.*
*************************************************************************/

RexxRoutine1(CSTRING, SysQueryRexxMacro, CSTRING, name)
{
    unsigned short position;         /* returned position          */

    if (RexxQueryMacro(name, &position) != 0)
    {
        return "";
    }
    // before?
    if (position == RXMACRO_SEARCH_BEFORE)
    {
        return "B";
    }
    else
    {
        return "A";                    /* must be 'A'fter            */
    }
}

/*************************************************************************
* Function:  SysClearRexxMacroSpace                                      *
*                                                                        *
* Syntax:    result = SysClearRexxMacroSpace()                           *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    return code from RexxClearMacroSpace()                      *
*************************************************************************/

RexxRoutine0(int, SysClearRexxMacroSpace)
{
    return (int)RexxClearMacroSpace();          /* clear the macro space      */
}

/*************************************************************************
* Function:  SysSaveRexxMacroSpace                                       *
*                                                                        *
* Syntax:    result = SysSaveRexxMacroSpace(file)                        *
*                                                                        *
* Params:    file   - name of the saved macro space file                 *
*                                                                        *
* Return:    return code from RexxSaveMacroSpace()                       *
*************************************************************************/

RexxRoutine1(int, SysSaveRexxMacroSpace, CSTRING, file)
{
    return (int)RexxSaveMacroSpace(0, NULL, file);
}

/*************************************************************************
* Function:  SysLoadRexxMacroSpace                                       *
*                                                                        *
* Syntax:    result = SysLoadRexxMacroSpace(file)                        *
*                                                                        *
* Params:    file   - name of the saved macro space file                 *
*                                                                        *
* Return:    return code from RexxLoadMacroSpace()                       *
*************************************************************************/

RexxRoutine1(int, SysLoadRexxMacroSpace, CSTRING, file)
{
    return (int)RexxLoadMacroSpace(0, NULL, file);
}


/*************************************************************************
* Function:  SysBootDrive                                                *
*                                                                        *
* Syntax:    drive = SysBootDrive()                                      *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    'A: B: C: D: ...'                                           *
*************************************************************************/

size_t RexxEntry SysBootDrive(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  if (numargs)                         /* validate arguments         */
    return INVALID_ROUTINE;

  if (GetSystemDirectory(retstr->strptr, 255) > 0)
  {
     retstr->strptr[2] = '\0';
     retstr->strlength = 2;
  }
  else
     retstr->strlength = 0;
  return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysSystemDirectory                                          *
*                                                                        *
* Syntax:    drive = SysSystemDirectory()                                *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    'C:\WINDOWS ...'                                            *
*************************************************************************/

size_t RexxEntry SysSystemDirectory(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  if (numargs)                         /* validate arguments         */
    return INVALID_ROUTINE;

  if (GetSystemDirectory(retstr->strptr, 255) > 0)
  {
     retstr->strlength = strlen(retstr->strptr);
  }
  else
     retstr->strlength = 0;
  return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  SysFileSystemType                                           *
*                                                                        *
* Syntax:    result = SysFileSystemType("drive")                         *
*                                                                        *
* Params:    drive - drive letter (in form of 'D:')                      *
*        or  none - current drive                                        *
*                                                                        *
* Return:    result - File System Name attached to the specified drive   *
*                     (FAT, HPFS ....)                                   *
*            '' - Empty string in case of any error                      *
*************************************************************************/

size_t RexxEntry SysFileSystemType(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  char *      drive;
  CHAR chDriveLetter[4];
  UINT errorMode;

                                       /* validate arguments         */
  if (numargs > 1 || ((numargs == 1) &&
      (args[0].strlength > 2 ||
      args[0].strlength == 0)))
    return INVALID_ROUTINE;
                                       /* validate the arg           */
                                       /* drive letter?              */
  if ((numargs == 1) && (strlen(args[0].strptr) == 2 && args[0].strptr[1] != ':'))
    return INVALID_ROUTINE;

  if (numargs == 1)
  {
     if (args[0].strlength == 1)       /* need to add a : if only the*/
     {
        chDriveLetter[0]=args[0].strptr[0]; /* letter was passed in  */
        chDriveLetter[1]=':';
        chDriveLetter[2]='\\';         /* need to add \ because of   */
        chDriveLetter[3]='\0';         /* bug in getvolumeinfo       */
      }
      else                        /* have <letter>: , just copy over */
      {
         strcpy(chDriveLetter, args[0].strptr);
         chDriveLetter[2]='\\';        /* need to add \ because of   */
         chDriveLetter[3]='\0';        /* bug in getvolumeinfo       */
      }
      drive = chDriveLetter;
  } else drive = NULL;

  errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

  if (GetVolumeInformation(
    drive,    // address of root directory of the file system
    NULL,    // address of name of the volume
    0,    // length of lpVolumeNameBuffer
    NULL,    // address of volume serial number
    NULL,    // address of system's maximum filename length
    NULL,    // address of file system flags
    retstr->strptr,    // address of name of file system
    255     // length of lpFileSystemNameBuffer
  ))
     retstr->strlength = strlen(retstr->strptr);
  else
     retstr->strlength = 0;            /* return a null string       */

  SetErrorMode(errorMode);
  return VALID_ROUTINE;                /* good completion            */
}


/*************************************************************************
* Function:  SysVolumeLabel                                              *
*                                                                        *
* Syntax:    result = SysVolumeLabel("drive")                            *
*                                                                        *
* Params:    drive - drive letter (in form of 'D:')                      *
*        or  none - current drive                                        *
*                                                                        *
* Return     '' - Empty string in case of any error                      *
*************************************************************************/

size_t RexxEntry SysVolumeLabel(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  char  *      drive;
  CHAR chDriveLetter[4];

                                       /* validate arguments         */
  if (numargs > 1 || ((numargs == 1) &&
      (args[0].strlength > 2 ||
      args[0].strlength == 0)))
    return INVALID_ROUTINE;
                                       /* validate the arg           */
                                       /* drive letter?              */
  if ((numargs == 1) && (strlen(args[0].strptr) == 2 && args[0].strptr[1] != ':'))
    return INVALID_ROUTINE;

  if (numargs == 1)
  {
     if (args[0].strlength == 1)       /* need to add a : if only the*/
     {
        chDriveLetter[0]=args[0].strptr[0];  /* letter was passed in */
        chDriveLetter[1]=':';
        chDriveLetter[2]='\\';         /* need to add \ because of   */
        chDriveLetter[3]='\0';         /* bug in getvolumeinfo       */
      }
      else                        /* have <letter>: , just copy over */
      {
         strcpy(chDriveLetter, args[0].strptr);
         chDriveLetter[2]='\\';        /* need to add \ because of   */
         chDriveLetter[3]='\0';        /* bug in getvolumeinfo       */
      }
      drive = chDriveLetter;
  } else drive = NULL;

  if (GetVolumeInformation(
    drive,           /* address of root directory of the file system */
    retstr->strptr,                  /*address of name of the volume */
    255,                             /* length of lpVolumeNameBuffer */
    NULL,                         /* address of volume serial number */
    NULL,             /* address of system's maximum filename length */
    NULL,                            /* address of file system flags */
    NULL,                          /* address of name of file system */
    0                            /* length of lpFileSystemNameBuffer */
  ))
     retstr->strlength = strlen(retstr->strptr);
  else
     retstr->strlength = 0;            /* return a null string       */
  return VALID_ROUTINE;                /* good completion            */
}


/*************************************************************************
* Function:  SysCreateMutexSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateMutexSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a mutex semaphore                 *
*                                                                        *
* Return:    handle - token used as a mutex handle for                   *
*                     SysRequestMutexSem, SysReleaseMutexSem,            *
*                     SysCloseMutexSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

RexxRoutine1(RexxObjectPtr, SysCreateMutexSem, OPTIONAL_CSTRING, name)
{
    HANDLE    handle;                    /* mutex handle               */
    SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, true};

    handle = 0;                          /* zero the handle            */
    if (name != NULL)                  /* request for named sem      */
    {
        /* create it by name          */
        handle = CreateMutex(&sa, false, name);
        if (!handle)                            /* may already be created     */
        {
            /* try to open it             */
            handle = OpenMutex(MUTEX_ALL_ACCESS, true, name);
        }
    }
    else                                 /* unnamed semaphore          */
    {
        handle = CreateMutex(&sa, false, NULL);
    }

    if (handle == NULL) {
        return context->NullString();
    }

    return context->Uintptr((uintptr_t)handle);
}


/*************************************************************************
* Function:  SysOpenMutexSem                                             *
*                                                                        *
* Syntax:    result = SysOpenMutexSem(name)                              *
*                                                                        *
* Params:    name - name of the mutex semaphore                          *
*                                                                        *
* Return:    result - handle to the mutex                                *
*************************************************************************/

RexxRoutine1(uintptr_t, SysOpenMutexSem, CSTRING, name)
{
    SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, true};

                                       /* get a binary handle        */
    return (uintptr_t) OpenMutex(MUTEX_ALL_ACCESS, true, name); /* try to open it             */
}

/*************************************************************************
* Function:  SysReleaseMutexSem                                          *
*                                                                        *
* Syntax:    result = SysReleaseMutexSem(handle)                         *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from ReleaseMutex                      *
*************************************************************************/

RexxRoutine1(int, SysReleaseMutexSem, uintptr_t, h)
{
    return !ReleaseMutex((HANDLE)h) ? GetLastError() : 0;
}

/*************************************************************************
* Function:  SysCloseMutexSem                                            *
*                                                                        *
* Syntax:    result = SysCloseMutexSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from CloseHandle                       *
*************************************************************************/

RexxRoutine1(int, SysCloseMutexSem, uintptr_t, h)
{
    return !ReleaseMutex((HANDLE)h) ? GetLastError() : 0;
}

/*************************************************************************
* Function:  SysRequestMutexSem                                          *
*                                                                        *
* Syntax:    result = SysRequestMutexSem(handle, <timeout>)              *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from WaitForSingleObject               *
*************************************************************************/

RexxRoutine2(int, SysRequestMutexSem, uintptr_t, h, OPTIONAL_int, timeout)
{
    if (argumentOmitted(2))
    {
        timeout = INFINITE;       /* default is no timeout      */
    }
    int rc = WaitForSingleObject((HANDLE)h, timeout);
    if (rc == WAIT_FAILED)
    {
        return GetLastError();
    }
    else
    {
        return rc;                         /* format the return code     */
    }
}

/*************************************************************************
* Function:  SysCreateEventSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateEventSem(<name>,<manual>)                 *
*                                                                        *
* Params:    name  - optional name for a event semaphore                 *
*            any second argument means manual reset event                *
* Return:    handle - token used as a event sem handle for               *
*                     SysPostEventSem, SysClearEventSem,                 *
*                     SysCloseEventSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

RexxRoutine2(RexxObjectPtr, SysCreateEventSem, OPTIONAL_CSTRING, name, OPTIONAL_CSTRING, reset)
{
    HANDLE    handle;                    /* mutex handle               */
    SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, true};
    bool      manual;

    handle = 0;                          /* zero the handle            */
    if (reset != NULL)
    {
        manual = true;
    }
    else
    {
        manual = false;
    }

    if (name != NULL)
    {                                    /* request for named sem      */
                                         /* create it by name          */
        handle = CreateEvent(&sa, manual, false, name);
        if (!handle)                       /* may already be created     */
        {
                                           /* try to open it             */
            handle = OpenEvent(EVENT_ALL_ACCESS, true, name);
        }
    }
    else                                 /* unnamed semaphore          */
    {
        handle = CreateEvent(&sa, manual, false, NULL);
    }

    if (handle == NULL) {
        return context->NullString();
    }

    return context->Uintptr((uintptr_t)handle);
}

/*************************************************************************
* Function:  SysOpenEventSem                                             *
*                                                                        *
* Syntax:    result = SysOpenEventSem(name)                              *
*                                                                        *
* Params:    name - name of the event semaphore                          *
*                                                                        *
* Return:    result - return code from OpenEvent                         *
*************************************************************************/

RexxRoutine1(uintptr_t, SysOpenEventSem, CSTRING, name)
{
    SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, true};

                                       /* get a binary handle        */
    return (uintptr_t)OpenEvent(EVENT_ALL_ACCESS, true, name); /* try to open it             */
}

/*************************************************************************
* Function:  SysPostEventSem                                             *
*                                                                        *
* Syntax:    result = SysPostEventSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from SetEvent                          *
*************************************************************************/

RexxRoutine1(int, SysPostEventSem, uintptr_t, h)
{
    return !SetEvent((HANDLE)h) ? GetLastError() : 0;
}

/*************************************************************************
* Function:  SysResetEventSem                                            *
*                                                                        *
* Syntax:    result = SysResetEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from ResetEvent                        *
*************************************************************************/

RexxRoutine1(int, SysResetEventSem, uintptr_t, h)
{
    return !ResetEvent((HANDLE)h) ? GetLastError() : 0;
}


/*************************************************************************
* Function:  SysPulseEventSem                                            *
*                                                                        *
* Syntax:    result = SysPulseEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from PulseEvent                        *
*************************************************************************/

RexxRoutine1(int, SysPulseEventSem, uintptr_t, h)
{
    return !PulseEvent((HANDLE)h) ? GetLastError() : 0;
}


/*************************************************************************
* Function:  SysCloseEventSem                                            *
*                                                                        *
* Syntax:    result = SysCloseEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from CloseHandle                       *
*************************************************************************/

RexxRoutine1(int, SysCloseEventSem, uintptr_t, h)
{
    return !CloseHandle((HANDLE)h) ? GetLastError() : 0;
}

/*************************************************************************
* Function:  SysWaitEventSem                                             *
*                                                                        *
* Syntax:    result = SysWaitEventSem(handle, <timeout>)                 *
*                                                                        *
* Params:    handle - token returned from SysWaitEventSem                *
*                                                                        *
* Return:    result - return code from WaitForSingleObject               *
*************************************************************************/

RexxRoutine2(int, SysWaitEventSem, uintptr_t, h, OPTIONAL_int, timeout)
{
    if (!argumentExists(2))
    {
        timeout = INFINITE;       /* default is no timeout      */
    }
    /* request the semaphore      */
    int rc = WaitForSingleObject((HANDLE)h, timeout);
    if (rc == WAIT_FAILED)
    {
        return GetLastError();
    }
    else
    {
        return rc;                         /* format the return code     */
    }
}


/*************************************************************************
* Function:  SysSetPriority                                              *
*                                                                        *
* Syntax:    result = SysSetPriority(Class, Level)                       *
*                                                                        *
* Params: Class - The priority class (0-3 or HIGH,REALTIME,NORMAL,IDLE)  *
*         Level  - Amount to change (-15 to +15 or IDLE, LOWEST,...)     *
*                                                                        *
*************************************************************************/

RexxRoutine2(int, SysSetPriority, RexxObjectPtr, classArg, RexxObjectPtr, levelArg)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    DWORD     iclass=-1;
    wholenumber_t classLevel;               /* priority class             */
    if (context->WholeNumber(classArg, &classLevel))
    {
        switch (classLevel)
        {
            case 0: iclass = IDLE_PRIORITY_CLASS;
                break;
            case 1: iclass = NORMAL_PRIORITY_CLASS;
                break;
            case 2: iclass = HIGH_PRIORITY_CLASS;
                break;
            case 3: iclass = REALTIME_PRIORITY_CLASS;
            default:
                context->InvalidRoutine();
                return 0;
        }
    }
    else
    {
        const char *classStr = context->ObjectToStringValue(classArg);

        if (stricmp(classStr, "REALTIME") == 0)
        {
            iclass = REALTIME_PRIORITY_CLASS;
        }
        else if (stricmp(classStr, "HIGH") == 0)
        {
            iclass = HIGH_PRIORITY_CLASS;
        }
        else if (!stricmp(classStr, "NORMAL") == 0)
        {
            iclass = NORMAL_PRIORITY_CLASS;
        }
        else if (stricmp(classStr, "IDLE") == 0)
        {
            iclass = IDLE_PRIORITY_CLASS;
        }
        else
        {
            context->InvalidRoutine();
            return 0;
        }
    }


    wholenumber_t level;                    /* priority level             */
    if (context->WholeNumber(levelArg, &level))
    {
        if (level < -15 || level > 15)
        {
            context->InvalidRoutine();
            return 0;
        }
    }
    else
    {
        const char *levelStr = context->ObjectToStringValue(levelArg);

        if (stricmp(levelStr, "ABOVE_NORMAL") == 0)
        {
            level = THREAD_PRIORITY_ABOVE_NORMAL;
        }
        else if (stricmp(levelStr, "BELOW_NORMAL") == 0)
        {
            level = THREAD_PRIORITY_BELOW_NORMAL;
        }
        else if (stricmp(levelStr, "HIGHEST") == 0)
        {
            level = THREAD_PRIORITY_HIGHEST;
        }
        else if (stricmp(levelStr, "LOWEST") == 0)
        {
            level = THREAD_PRIORITY_LOWEST;
        }
        else if (stricmp(levelStr, "NORMAL") == 0)
        {
            level = THREAD_PRIORITY_NORMAL;
        }
        else if (stricmp(levelStr, "IDLE") == 0)
        {
            level = THREAD_PRIORITY_IDLE;
        }
        else if (stricmp(levelStr, "TIME_CRITICAL") == 0)
        {
            level = THREAD_PRIORITY_TIME_CRITICAL;
        }
        else
        {
            context->InvalidRoutine();
            return 0;
        }
    }

    int rc = SetPriorityClass(process, iclass);
    if (rc)
    {
        rc = SetThreadPriority(thread, (int)level);
    }

    return rc != 0 ? 0 : GetLastError();
}


/*************************************************************************
* Function:  SysQueryProcess                                             *
*                                                                        *
* Params:    "PID" - (default) returns current process ID                *
*            "TID" - (default) returns current thread ID                 *
*            "PPRIO" - (default) returns current process priority        *
*            "TPRIO" - (default) returns current thread priority         *
*            "PTIME" - (default) returns current process times           *
*            "TTIME" - (default) returns current thread times            *
*************************************************************************/

RexxRoutine1(RexxObjectPtr, SysQueryProcess, OPTIONAL_CSTRING, option)
{
    if (option == NULL || stricmp(option, "PID") == 0)
    {
        return context->WholeNumber(GetCurrentProcessId());
    }
    if (stricmp(option, "TID") == 0)
    {
        return context->WholeNumber(GetCurrentThreadId());
    }
    if (stricmp(option, "PPRIO") == 0)
    {
        LONG p;
        p = GetPriorityClass(GetCurrentProcess());

        switch (p)
        {
            case HIGH_PRIORITY_CLASS:
                return context->String("HIGH");
            case IDLE_PRIORITY_CLASS:
                return context->String("IDLE");
            case NORMAL_PRIORITY_CLASS:
                return context->String("NORMAL");
            case REALTIME_PRIORITY_CLASS:
                return context->String("REALTIME");
            default:
                return context->String("UNKNOWN");
        }
    }
    if (stricmp(option, "TPRIO") == 0)
    {
        LONG p;
        p = GetThreadPriority(GetCurrentThread());

        switch (p)
        {
            case THREAD_PRIORITY_ABOVE_NORMAL:
                return context->String("ABOVE_NORMAL");
            case THREAD_PRIORITY_BELOW_NORMAL:
                return context->String("BELOW_NORMAL");
            case THREAD_PRIORITY_HIGHEST:
                return context->String("HIGHEST");
            case THREAD_PRIORITY_IDLE:
                return context->String("IDLE");
            case THREAD_PRIORITY_LOWEST:
                return context->String("LOWEST");
                break;
            case THREAD_PRIORITY_NORMAL:
                return context->String("NORMAL");
                break;
            case THREAD_PRIORITY_TIME_CRITICAL:
                return context->String("TIME_CRITICAL");
            default:
                return context->String("UNKNOWN");
        }
    }
    if (stricmp(option, "PTIME") == 0 || stricmp(option, "TTIME") == 0)
    {
        FILETIME createT, kernelT, userT, dummy;
        SYSTEMTIME createST, kernelST, userST;
        BOOL ok;

        if (*option == 'T' || *option == 't')
        {
            ok = GetThreadTimes(GetCurrentThread(), &createT,&dummy,&kernelT, &userT);
        }
        else
        {
            ok = GetProcessTimes(GetCurrentProcess(), &createT,&dummy,&kernelT, &userT);
        }

        if (ok)
        {
            FileTimeToLocalFileTime(&createT, &createT);
            FileTimeToSystemTime(&createT, &createST);
            FileTimeToSystemTime(&kernelT, &kernelST);
            FileTimeToSystemTime(&userT, &userST);

            char buffer[256];

            wsprintf(buffer, "Create: %4.4d/%2.2d/%2.2d %d:%2.2d:%2.2d:%3.3d  "\
                     "Kernel: %d:%2.2d:%2.2d:%3.3d  User: %d:%2.2d:%2.2d:%3.3d",
                     createST.wYear,createST.wMonth,createST.wDay,createST.wHour,createST.wMinute,
                     createST.wSecond,createST.wMilliseconds,
                     kernelST.wHour,kernelST.wMinute,kernelST.wSecond,kernelST.wMilliseconds,
                     userST.wHour,userST.wMinute,userST.wSecond,userST.wMilliseconds);

            return context->String(buffer);
        }
    }
    context->InvalidRoutine();
    return NULLOBJECT;
}


/** SysShutDownSystem()
 *
 *  Interface to the InitiateSystemShutdown() API on Windows.
 *
 *  @param  computer         The name of the computer to shut down.  If omitted
 *                           or the empty string, the local machine is shut
 *                           down.  Otherwise this is the name of a remote
 *                           machine to shut down.
 *  @param  message          If timout is not 0, a shut down dialog is displayed
 *                           on the machine being shut down, naming the user who
 *                           initiated the shut down, a timer counting down the
 *                           seconds until the machine is shut down, and
 *                           prompting the local user to log off. This parametr
 *                           can be an additional message to add to the dialog
 *                           box.  It can be ommitted if no additional message
 *                           is desired.
 *  @param  timeout          Number of seconds to display the shut down dialog.
 *                           If this is 0 no dialog is displayed.  The default
 *                           is 30 seconds, see the remarks below.  The user can
 *                           force a 0 timeout by explicitly specifying 0.
 *  @param  forceAppsClosed  If true applications with unsaved data are forcibly
 *                           closed.  If false, the user is presented with a
 *                           dialog telling the user to close the applcation(s).
 *  @param  reboot           If true, the system is rebooted, if false the
 *                           system is shut down.
 *
 *  @remarks Note that prior to 4.0.0, the defaults for all arguments were
 *           exactly what the value of each parameter is if omitted.
 *
 *           machine         == NULL
 *           message         == NULL
 *           timeout         == 0
 *           forceAppsClosed == false
 *           reboot          == false
 *
 *           Because of this, there would be no need to check if any argument is
 *           ommitted or not.  However, the consequences of having a 0 timeout
 *           value are severe if the system has an application open with unsaved
 *           data.  Therefore for 4.0.0 and on the default time out value is
 *           changed to 30 (seconds.)
 */
RexxRoutine5(uint32_t, SysShutDownSystem, OPTIONAL_CSTRING, computer, OPTIONAL_CSTRING, message, OPTIONAL_uint32_t, timeout,
             OPTIONAL_logical_t, forceAppsClosed, OPTIONAL_logical_t, reboot)
{
    uint32_t result = 0;

    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;

    // First we get the token for the current process so we can add the proper
    // shutdown privilege.
    if ( OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == 0 )
    {
        result = GetLastError();
        goto done_out;
    }

    // Get the locally unique identifier for the shutdown privilege we need,
    // local or remote, depending on what the user specified.
    LPCTSTR privilegeName = (computer == NULL || *computer == '\0') ? SE_SHUTDOWN_NAME : SE_REMOTE_SHUTDOWN_NAME;
    if ( LookupPrivilegeValue(NULL, privilegeName, &tkp.Privileges[0].Luid) == 0 )
    {
        result = GetLastError();
        goto done_out;
    }

    // We are only going to adjust 1 privilege and we are going to enable it.
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // The return from this function can not accurately be used to determine if
    // it failed or not.  Instead we need to use GetLastError to determine
    // success.
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
    result = GetLastError();
    if ( result != ERROR_SUCCESS )
    {
        goto done_out;
    }

    // Do not shut down in 0 seconds by default.
    if ( argumentOmitted(3) )
    {
        timeout = 30;
    }

    // Now just call the API with the parameters specified by the user.
    if ( InitiateSystemShutdown((LPSTR)computer, (LPSTR)message, timeout, (BOOL)forceAppsClosed, (BOOL)reboot) == 0 )
    {
        result = GetLastError();
    }

    // Finally, restore the shutdown privilege for this process to disabled.
    tkp.Privileges[0].Attributes = 0;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

done_out:
    if ( hToken != NULL )
    {
        CloseHandle(hToken);
    }
    return result;
}

/*************************************************************************
* Function:  SysSwitchSession                                            *
*                                                                        *
* Syntax:    result = SysSwitchSession(name)                             *
*                                                                        *
* Params:    name   - name of target session                             *
*                                                                        *
* Return:    OS/2 error return code                                      *
*************************************************************************/

RexxRoutine1(int, SysSwitchSession, CSTRING, name)
{
    HWND hwnd = FindWindow(NULL, name);

    if (hwnd)
    {
        if (!SetForegroundWindow(hwnd))
        {
            return GetLastError();
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 1;
    }
}

/*************************************************************************
* Function:  SysWaitNamedPipe                                            *
*                                                                        *
* Syntax:    result = SysWaitNamedPipe(name, timeout)                    *
*                                                                        *
* Params:    name - name of the pipe                                     *
*            timeout - amount of time to wait.                           *
*                                                                        *
* Return:    Return code from WaitNamedPipe                              *
*************************************************************************/

RexxRoutine2(int, SysWaitNamedPipe, CSTRING, name, OPTIONAL_int, timeout)
{
    if (argumentOmitted(2))
    {
        timeout = NMPWAIT_USE_DEFAULT_WAIT;
    }
    else
    {
        if (timeout < -1)
        {
            context->InvalidRoutine();
            return 0;
        }
        if (timeout == 0)
        {
            timeout = NMPWAIT_USE_DEFAULT_WAIT;
        }
        else if (timeout == -1)
        {
            timeout = NMPWAIT_WAIT_FOREVER;
        }
    }

    if (WaitNamedPipe(name, timeout))
    {
        return 0;
    }
    else
    {
        return GetLastError();
    }
}


/*************************************************************************
* Function:  SysDumpVariables                                            *
*                                                                        *
* Syntax:    result = SysDumpVariables([filename])                       *
*                                                                        *
* Params:    filename - name of the file where variables are appended to *
*                       (dump is written to stdout if omitted)           *
*                                                                        *
* Return:    0 - dump completed OK                                       *
*            -1 - failure during dump                                     *
*************************************************************************/

size_t RexxEntry SysDumpVariables(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  LONG      rc;                        /* Ret code                   */
  SHVBLOCK  shvb;
  HANDLE    outFile = NULL;
  bool      fCloseFile = false;
  DWORD     dwBytesWritten = 0;
  char     *buffer = NULL;             /* ENG: write result file to  */
  char     *current, *end;             /* memory first, much faster! */
  size_t    buffer_size = 10240;       /* buffer, realloc'd if needed*/
  size_t    new_size;                  /* real new size              */

  if ( (numargs > 1) ||                /* wrong number of arguments? */
       ((numargs > 0) && !RXVALIDSTRING(args[0])) )
    return INVALID_ROUTINE;            /* raise error condition      */

  if (numargs > 0)
  {
    /* open output file for append */
    outFile = CreateFile(args[0].strptr, GENERIC_WRITE | GENERIC_READ,
                         0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                         FILE_FLAG_WRITE_THROUGH, NULL);
    if (outFile)
    {
      fCloseFile = true;

      /* seek to end of file */
      SetFilePointer(outFile, 0, 0, FILE_END);
    }
  }
  else
    outFile = GetStdHandle(STD_OUTPUT_HANDLE);

                                       /* write results to memory    */
                                       /* first and then in one step */
                                       /* to disk                    */
  buffer = (char*) calloc(buffer_size,1);
  if (buffer == NULL)
    return INVALID_ROUTINE;            /* raise error condition      */
  current = buffer;
  end = current + buffer_size;

  do
  {
    /* prepare request block */
    shvb.shvnext = NULL;
    shvb.shvname.strptr = NULL;      /* let REXX allocate the memory */
    shvb.shvname.strlength = 0;
    shvb.shvnamelen = 0;
    shvb.shvvalue.strptr = NULL;     /* let REXX allocate the memory */
    shvb.shvvalue.strlength = 0;
    shvb.shvvaluelen = 0;
    shvb.shvcode = RXSHV_NEXTV;
    shvb.shvret = 0;

    rc = RexxVariablePool(&shvb);

    if (rc == RXSHV_OK)
    {
      new_size = 5 + 9 + 3 + shvb.shvname.strlength + shvb.shvvalue.strlength;
                                     /* if buffer is not big enough, */
                                     /* reallocate                   */
      if (current + new_size >= end) {
        size_t offset = current - buffer;
        buffer_size *= 2;
        /* if new buffer too small, use the minimal fitting size */
        if (buffer_size - offset < new_size) {
          buffer_size = new_size + offset;
        }
        buffer = (char *)realloc(buffer,buffer_size);
        current = buffer + offset;
        end = buffer + buffer_size;
      }
      sprintf(current, "Name=");
      current += 5;
      memcpy(current, shvb.shvname.strptr, shvb.shvname.strlength);
      current += shvb.shvname.strlength;
      sprintf(current, ", Value='");
      current += 9;
      memcpy(current, shvb.shvvalue.strptr, shvb.shvvalue.strlength);
      current += shvb.shvvalue.strlength;
      sprintf(current, "'\r\n");
      current += 3;

      /* free memory allocated by REXX */
      RexxFreeMemory((void *)shvb.shvname.strptr);
      RexxFreeMemory((void *)shvb.shvvalue.strptr);

      /* leave loop if this was the last var */
      if (shvb.shvret & RXSHV_LVAR)
        break;
    }
  } while (rc == RXSHV_OK);

  WriteFile(outFile, buffer, (DWORD)(current - buffer), &dwBytesWritten, NULL);
  free(buffer);

  if (fCloseFile)
    CloseHandle(outFile);

  if (rc != RXSHV_LVAR)
    RETVAL(-1)
  else
    RETVAL(0)
}


/*************************************************************************
* Function:  SysSetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysSetFileDateTime(filename [,newdate] [,newtime]) *
*                                                                        *
* Params:    filename - name of the file to update                       *
*            newdate  - new date to set in format YYYY-MM-DD (YYYY>1800) *
*            newtime  - new time to set in format HH:MM:SS               *
*                                                                        *
* Return:    0 - file date/time was updated correctly                    *
*            -1 - failure attribute update                               *
*************************************************************************/

RexxRoutine3(int, SysSetFileDateTime, CSTRING, name, OPTIONAL_CSTRING, newdate, OPTIONAL_CSTRING, newtime)
{
    BOOL      fOk = FALSE;
    FILETIME  sFileTime;
    FILETIME  sLocalFileTime;
    SYSTEMTIME sLocalSysTime;

    /* open output file for read/write for update */
    HANDLE setFile = CreateFile(name, GENERIC_WRITE | GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH |
                                FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (setFile && (setFile != INVALID_HANDLE_VALUE))
    {
        fOk = GetFileTime(setFile, NULL, NULL, &sFileTime);
        fOk &= FileTimeToLocalFileTime(&sFileTime, &sLocalFileTime);
        fOk &= FileTimeToSystemTime(&sLocalFileTime, &sLocalSysTime);
        if (fOk)
        {
            /* file date/time could be read, now parse the new date/time */
            if (newdate != NULL)
            {
                /* parse new date */
                if (sscanf(newdate, "%4hu-%2hu-%2hu", &sLocalSysTime.wYear,
                           &sLocalSysTime.wMonth, &sLocalSysTime.wDay) != 3)
                {
                    fOk = false;
                }

                if (sLocalSysTime.wYear < 1800)
                {
                    fOk = false;
                }
            }

            if (newtime != NULL)
            {
                /* parse new time */
                if (sscanf(newtime, "%2hu:%2hu:%2hu", &sLocalSysTime.wHour,
                           &sLocalSysTime.wMinute, &sLocalSysTime.wSecond) != 3)
                {
                    fOk = false;
                }
            }

            if (newdate == NULL && newtime == NULL)
            {
                /* we set the timestamp to the current time and date */
                GetLocalTime(&sLocalSysTime);
            }

            if (fOk)
            {
                fOk &= SystemTimeToFileTime(&sLocalSysTime, &sLocalFileTime);
                fOk &= LocalFileTimeToFileTime(&sLocalFileTime, &sFileTime);
                fOk &= SetFileTime(setFile, NULL, NULL, &sFileTime);
            }
        }

        CloseHandle(setFile);
    }

    return fOk ? 0 : 1;
}

/*************************************************************************
* Function:  SysGetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysGetFileDateTime(filename [,timesel])            *
* Params:    filename - name of the file to query                        *
*            timesel  - What filetime to query: Created/Access/Write     *
*                                                                        *
* Return:    -1 - file date/time query failed                            *
*            other - date and time as YYYY-MM-DD HH:MM:SS                *
*************************************************************************/

RexxRoutine2(RexxObjectPtr, SysGetFileDateTime, CSTRING, name, OPTIONAL_CSTRING, selector)
{
    FILETIME  sFileTime;
    FILETIME  sLocalFileTime;
    FILETIME  *psFileCreated = NULL;
    FILETIME  *psFileAccessed = NULL;
    FILETIME  *psFileWritten = NULL;
    SYSTEMTIME sLocalSysTime;

    if (selector != NULL)
    {
        switch (selector[0])
        {
            case 'c':
            case 'C':
                psFileCreated = &sFileTime;
                break;
            case 'a':
            case 'A':
                psFileAccessed = &sFileTime;
                break;
            case 'w':
            case 'W':
                psFileWritten = &sFileTime;
                break;
            default:
                context->InvalidRoutine();
                return NULLOBJECT;
        }
    }
    else
    {
        psFileWritten = &sFileTime;
    }

    /* open file for read to query time */
    HANDLE setFile = CreateFile(name, FILE_READ_ATTRIBUTES,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH |
                                FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (setFile != INVALID_HANDLE_VALUE)
    {
        BOOL fOk = GetFileTime(setFile, psFileCreated, psFileAccessed, psFileWritten);
        CloseHandle(setFile);
        fOk &= FileTimeToLocalFileTime(&sFileTime, &sLocalFileTime);
        fOk &= FileTimeToSystemTime(&sLocalFileTime, &sLocalSysTime);

        if (fOk)
        {
            char buffer[256];
            sprintf(buffer, "%4d-%02d-%02d %02d:%02d:%02d",
                    sLocalSysTime.wYear,
                    sLocalSysTime.wMonth,
                    sLocalSysTime.wDay,
                    sLocalSysTime.wHour,
                    sLocalSysTime.wMinute,
                    sLocalSysTime.wSecond);
            return context->String(buffer);
        }
    }
    return context->WholeNumber(-1);

}


RexxReturnCode REXXENTRY RexxStemSort(const char *stemname, int order, int type,
    size_t start, size_t end, size_t firstcol, size_t lastcol);

/*************************************************************************
* Function:  SysStemSort                                                 *
*                                                                        *
* Syntax:    result = SysStemSort(stem, order, type, start, end,         *
*                                 firstcol, lastcol)                     *
*                                                                        *
* Params:    stem - name of stem to sort                                 *
*            order - 'A' or 'D' for sort order                           *
*            type - 'C', 'I', 'N' for comparision type                   *
*            start - first index to sort                                 *
*            end - last index to sort                                    *
*            firstcol - first column to use as sort key                  *
*            lastcol - last column to use as sort key                    *
*                                                                        *
* Return:    0 - sort was successful                                     *
*            -1 - sort failed                                            *
*************************************************************************/

size_t RexxEntry SysStemSort(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    CHAR          stemName[255];
    size_t        first = 1;
    size_t        last = SIZE_MAX;
    size_t        firstCol = 0;
    size_t        lastCol = SIZE_MAX;
    INT           sortType = SORT_CASESENSITIVE;
    INT           sortOrder = SORT_ASCENDING;

    // validate arguments
    if ( (numargs < 1) || (numargs > 7) || !RXVALIDSTRING(args[0]) )
    {
        return INVALID_ROUTINE;
    }

    // remember stem name
    memset(stemName, 0, sizeof(stemName));
    strcpy(stemName, args[0].strptr);
    if ( stemName[args[0].strlength-1] != '.' )
    {
        stemName[args[0].strlength] = '.';
    }

    // check other parameters.  sort order
    if ( (numargs >= 2) && RXVALIDSTRING(args[1]) )
    {
        switch ( args[1].strptr[0] )
        {
            case 'A':
            case 'a':
                sortOrder = SORT_ASCENDING;
                break;
            case 'D':
            case 'd':
                sortOrder = SORT_DECENDING;
                break;
            default:
                return INVALID_ROUTINE;
        }
    }
    // sort type
    if ( (numargs >= 3) && RXVALIDSTRING(args[2]) )
    {
        switch ( args[2].strptr[0] )
        {
            case 'C':
            case 'c':
                sortType = SORT_CASESENSITIVE;
                break;
            case 'I':
            case 'i':
                sortType = SORT_CASEIGNORE;
                break;
            default:
                return INVALID_ROUTINE;
        }
    }
    // first element to sort
    if ( (numargs >= 4) && RXVALIDSTRING(args[3]) )
    {
        if (!string2size_t(args[3].strptr, &first))
        {
            return INVALID_ROUTINE;
        }
        if ( first == 0 )
        {
            return INVALID_ROUTINE;
        }
    }
    // last element to sort
    if ( (numargs >= 5) && RXVALIDSTRING(args[4]) )
    {
        if (!string2size_t(args[4].strptr, &last))
            return INVALID_ROUTINE;
        if ( last < first )
            return INVALID_ROUTINE;
    }
    // first column to sort
    if ( (numargs >= 6) && RXVALIDSTRING(args[5]) )
    {
        if (!string2size_t(args[5].strptr, &firstCol))
        {
            return INVALID_ROUTINE;
        }
        firstCol--;
    }
    // last column to sort
    if ( (numargs == 7) && RXVALIDSTRING(args[6]) )
    {
        if (!string2size_t(args[6].strptr, &lastCol))
        {
            return INVALID_ROUTINE;
        }
        lastCol--;
        if ( lastCol < firstCol )
        {
            return INVALID_ROUTINE;
        }

    }

    // the sorting is done in the interpreter
    if ( !RexxStemSort(stemName, sortOrder, sortType, first, last, firstCol, lastCol) )
    {
        sprintf(retstr->strptr, "-1");
        retstr->strlength = 2;
        return INVALID_ROUTINE;
    }

    sprintf(retstr->strptr, "0");
    retstr->strlength = 1;
    return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysStemDelete                                               *
*                                                                        *
* Syntax:    result = SysStemDelete(stem, startitem [,itemcount])        *
*                                                                        *
* Params:    stem - name of stem where item will be deleted              *
*            startitem - index of item to delete                         *
*            itemcount - number of items to delete if more than 1        *
*                                                                        *
* Return:    0 - delete was successful                                   *
*            -1 - delete failed                                          *
*************************************************************************/

RexxRoutine3(int, SysStemDelete, RexxStemObject, toStem, stringsize_t, start, OPTIONAL_stringsize_t, count)

{
    if (argumentOmitted(3))
    {
        count = 1;
    }

    stringsize_t items;

    RexxObjectPtr temp = context->GetStemArrayElement(toStem, 0);
    if (temp == NULLOBJECT || !context->StringSize(temp, &items))
    {
        context->InvalidRoutine();
        return 0;
    }

    // make sure the deletion site is within the bounds
    if (start + count - 1 > items)
    {
        context->InvalidRoutine();
        return 0;
    }

    stringsize_t index;
    /* now copy the remaining indices up front */
    for ( index = start;  index + count <= items; index++)
    {
        // copy from the old index to the new index
        RexxObjectPtr value = context->GetStemArrayElement(toStem, index + count);
        // is this a sparse array?
        if (value == NULLOBJECT)
        {
            // return this as a failure
            return -1;
        }
        context->SetStemArrayElement(toStem, index, value);
    }

    /* now delete the items at the end */
    for (index = items - count + 1; index <= items; index++)
    {
        context->DropStemArrayElement(toStem, index);
    }

    context->SetStemArrayElement(toStem, 0, context->StringSize(items - count));
    return 0;
}


/*************************************************************************
* Function:  SysStemInsert                                               *
*                                                                        *
* Syntax:    result = SysStemInsert(stem, position, value)               *
*                                                                        *
* Params:    stem - name of stem where item will be inserted             *
*            position - index where new item will be inserted            *
*            value - new item value                                      *
*                                                                        *
* Return:    0 - insert was successful                                   *
*            -1 - insert failed                                          *
*************************************************************************/

RexxRoutine3(int, SysStemInsert, RexxStemObject, toStem, stringsize_t, position, RexxObjectPtr, newValue)
{
    stringsize_t count;

    RexxObjectPtr temp = context->GetStemArrayElement(toStem, 0);
    if (temp == NULLOBJECT || !context->StringSize(temp, &count))
    {
        context->InvalidRoutine();
        return 0;
    }

    /* check wether new position is within limits */
    if (position == 0 || (position > count + 1))
    {
        context->InvalidRoutine();
        return 0;
    }

    for (size_t index = count; index >= position; index--)
    {
        // copy from the old index to the new index
        RexxObjectPtr value = context->GetStemArrayElement(toStem, index);
        // is this a sparse array?
        if (value == NULLOBJECT)
        {
            // return this as a failure
            return -1;
        }
        context->SetStemArrayElement(toStem, index + 1, value);
    }

    // now set the new value and increase the count at stem.0
    context->SetStemArrayElement(toStem, position, newValue);
    context->SetStemArrayElement(toStem, 0, context->WholeNumber(count + 1));
    return 0;
}


/*************************************************************************
* Function:  SysStemCopy                                                 *
*                                                                        *
* Syntax:    result = SysStemCopy(fromstem, tostem, from, to, count      *
*                                 [,insert])                             *
*                                                                        *
* Params:    fromstem - name of source stem                              *
*            tostem - - name of target stem                              *
*            from  - first index in source stem to copy                  *
*            to - position where items are copied/inserted in target stem*
*            count - number of items to copy/insert                      *
*            insert - 'I' to indicate insert instead of 'O' overwrite    *
*                                                                        *
* Return:    0 - stem copy was successful                                *
*            -1 - stem copy failed                                       *
*************************************************************************/

RexxRoutine6(int, SysStemCopy, RexxStemObject, fromStem, RexxStemObject, toStem,
    OPTIONAL_stringsize_t, from, OPTIONAL_stringsize_t, to, OPTIONAL_stringsize_t, count,
    OPTIONAL_CSTRING, option)
{
    bool inserting = false;

    /* get copy type */
    if (option != NULL)
    {
        switch (*option)
        {
            case 'I':
            case 'i':
                inserting = true;
                break;
            case 'O':
            case 'o':
                inserting = false;
                break;
            default:
            {
                context->InvalidRoutine();
                return 0;
            }
        }
    }

    stringsize_t fromCount;

    RexxObjectPtr temp = context->GetStemArrayElement(fromStem, 0);
    if (temp == NULLOBJECT || !context->StringSize(temp, &fromCount))
    {
        context->InvalidRoutine();
        return 0;
    }

    // default from location is the first element
    if (argumentOmitted(3))
    {
        from = 1;
    }

    if (argumentOmitted(4))
    {
        to = 1;
    }

    // was a count explicitly specified?
    if (argumentExists(5))
    {
        // this must be in range
        if ((count > (fromCount - from + 1)) || (fromCount == 0))
        {
            context->InvalidRoutine();
            return 0;
        }
    }
    else
    {
        // default is to copy everything from the starting position.
        count = fromCount - from + 1;
    }

    stringsize_t toCount = 0;
    // but if it is set, then use that value
    temp = context->GetStemArrayElement(toStem, 0);
    if (temp != NULLOBJECT && !context->StringSize(temp, &toCount))
    {
        context->InvalidRoutine();
        return 0;
    }

    // copying out of range?  Error
    if (to > toCount + 1)
    {
        context->InvalidRoutine();
        return 0;
    }

    if (inserting)
    {
        /* if we are about to insert the items we have to make room */
        for (size_t index = toCount; index >= to; index--)
        {
            // copy from the old index to the new index
            RexxObjectPtr value = context->GetStemArrayElement(toStem, index);
            // is this a sparse array?
            if (value == NULLOBJECT)
            {
                // return this as a failure
                return -1;
            }
            context->SetStemArrayElement(toStem, index + count, value);
        }


        // set the new count value in the target
        toCount += count;
        context->SetStemArrayElement(toStem, 0, context->StringSize(toCount));
    }
    /* now do the actual copying from the source to target */
    for (size_t index = 0; index < count; index++)
    {
        // just retrieve and copy
        RexxObjectPtr value = context->GetStemArrayElement(fromStem, from + index);
        // is this a sparse array?
        if (value == NULLOBJECT)
        {
            // return this as a failure
            return -1;
        }
        context->SetStemArrayElement(toStem, to + index, value);
    }

    // do we need to update the size?
    if (to + count - 1 > toCount)
    {
        context->SetStemArrayElement(toStem, 0, context->StringSize(to + count - 1));
    }
    return 0;
}


/*************************************************************************
* Function:  SysUtilVersion                                              *
*                                                                        *
* Syntax:    Say SysUtilVersion                                          *
*                                                                        *
* Return:    REXXUTIL.DLL Version                                        *
*************************************************************************/

RexxRoutine0(RexxStringObject, SysUtilVersion)
{
    char buffer[256];
                                       /* format into the buffer     */
    sprintf(buffer, "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
    return context->String(buffer);
}

/**
 * Check if the dwFlags arguement to WideCharToMultiByte() can be used by the
 * specified code page.  See MSDN documentation for WideCharToMultiByte() for
 * clarification.  This is used by SysFromUnicode()
 *
 * @param cp  Code page to check.
 *
 * @return Return true if dwFlags can be non-zero, return false if dwFlags must
 *         be zero.
 */
static bool canUseWideCharFlags(UINT cp)
{
    if ( cp == CP_SYMBOL || cp == CP_UTF7 || cp == CP_UTF8 )
    {
        return false;
    }
    if ( 50220 <= cp && cp <= 50222  )
    {
        return false;
    }
    if ( cp == 50225 || cp == 50227 || cp == 50229 || cp == 52936 || cp == 54936 )
    {
        return false;
    }
    if ( 57002 <= cp && cp <= 57011  )
    {
        return false;
    }
    return true;
}

/*************************************************************************
* Function:  SysFromUnicode                                              *
*            Converts a UNICODE string to an ASCII string                *
*                                                                        *
* Syntax:    result = SysFromUniCode(string,CodePage,MappingFlags,       *
*                                    DefaultChar, outstem.)              *
*                                                                        *
* Params:    string       - unicode string to be converted               *
*            Codepage     - target codepage                              *
*            MappingFlags - Mapping flags                                *
*            DefaultChar  - default for unmappable chars                 *
*             outstem.    - stem containg the result                     *
*              .!USEDDEFAULTCHAR - 1: character used as default          *
*              .!TEXT     - converted text                               *
*                                                                        *
*                                                                        *
* Return:    0 - successfull completetion                                *
*            error code from WideCharToMultiByte                         *

  The following are the OEM code-page identifiers.

    437  MS-DOS United States
    708  Arabic (ASMO 708)
    709  Arabic (ASMO 449+, BCON V4)
    710  Arabic (Transparent Arabic)
    720  Arabic (Transparent ASMO)
    737  Greek (formerly 437G)
    775  Baltic
    850  MS-DOS Multilingual (Latin I)
    852  MS-DOS Slavic (Latin II)
    855  IBM Cyrillic (primarily Russian)
    857  IBM Turkish
    860  MS-DOS Portuguese
    861  MS-DOS Icelandic
    862  Hebrew
    863  MS-DOS Canadian-French
    864  Arabic
    865  MS-DOS Nordic
    866  MS-DOS Russian (former USSR)
    869  IBM Modern Greek
    874  Thai
    932  Japan
    936  Chinese (PRC, Singapore)
    949  Korean
    950  Chinese (Taiwan; Hong Kong SAR, PRC)
    1361 Korean (Johab)

  The following are the ANSI code-page identifiers.

    874  Thai
    932  Japan
    936  Chinese (PRC, Singapore)
    949  Korean
    950  Chinese (Taiwan; Hong Kong SAR, PRC)
    1200 Unicode (BMP of ISO 10646)
    1250 Windows 3.1 Eastern European
    1251 Windows 3.1 Cyrillic
    1252 Windows 3.1 Latin 1 (US, Western Europe)
    1253 Windows 3.1 Greek
    1254 Windows 3.1 Turkish
    1255 Hebrew
    1256 Arabic
    1257 Baltic

  COMPOSITECHECK :
    Convert composite characters to precomposed characters.

  DISCARDNS :
    Discard nonspacing characters during conversion.

  SEPCHARS :
    Generate separate characters during conversion. This is the default conversion behavior.

  DEFAULTCHAR :
    Replace exceptions with the default character during conversion.

*************************************************************************/

RexxRoutine5(int, SysFromUniCode, RexxStringObject, sourceString, OPTIONAL_CSTRING, codePageOpt,
    OPTIONAL_CSTRING, mappingFlags, OPTIONAL_CSTRING, defaultChar, RexxStemObject, stem)
{
    const char *source = context->StringData(sourceString);
    size_t sourceLength = context->StringLength(sourceString);

    UINT  codePage;
    /* evaluate codepage          */
    if (codePageOpt == NULL)
    {
        codePage = GetOEMCP();
    }
    else
    {
        if (_stricmp(codePageOpt, "THREAD_ACP") == 0)
        {
            codePage = CP_THREAD_ACP;
        }
        else if (_stricmp(codePageOpt,"ACP") == 0)
        {
            codePage = CP_ACP;
        }
        else if (_stricmp(codePageOpt,"MACCP") == 0)
        {
            codePage = CP_MACCP;
        }
        else if (_stricmp(codePageOpt,"OEMCP") == 0)
        {
            codePage = CP_OEMCP;
        }
        else if (_stricmp(codePageOpt,"SYMBOL") == 0)
        {
            codePage = CP_SYMBOL;
        }
        else if (_stricmp(codePageOpt,"UTF7") == 0)
        {
            codePage = CP_UTF7;
        }
        else if (_stricmp(codePageOpt,"UTF8") == 0)
        {
            codePage = CP_UTF8;
        }
        else
        {
            codePage = atoi(codePageOpt);
        }
    }

    DWORD dwFlags = 0;
    /* evaluate the mapping flags */
    if (mappingFlags != NULL && *mappingFlags != '\0' )
    {
        /* The WC_SEPCHARS, WC_DISCARDNS, and WC_DEFAULTCHAR flags must also
         * specify the WC_COMPOSITECHECK flag.  So, we add that for the user if
         * they skipped it. Those 4 flags are only available for code pages <
         * 50000, excluding 42 (CP_SYMBOL).  See the remarks section in the MSDN
         * docs for clarification.
         */
        if ( codePage < 50000 && codePage != CP_SYMBOL )
        {
            if ( StrStrI(mappingFlags, "COMPOSITECHECK") != NULL )
            {
                dwFlags |= WC_COMPOSITECHECK;
            }
            if ( StrStrI(mappingFlags, "SEPCHARS") != NULL )
            {
                dwFlags |= WC_SEPCHARS | WC_COMPOSITECHECK;
            }
            if ( StrStrI(mappingFlags, "DISCARDNS") != NULL )
            {
                dwFlags |= WC_DISCARDNS| WC_COMPOSITECHECK;
            }
            if ( StrStrI(mappingFlags, "DEFAULTCHAR") != NULL )
            {
                dwFlags |= WC_DEFAULTCHAR | WC_COMPOSITECHECK;
            }
        }

        if ( StrStrI(mappingFlags, "NO_BEST_FIT") != NULL )
        {
            dwFlags |= WC_NO_BEST_FIT_CHARS;
        }

        if ( StrStrI(mappingFlags, "ERR_INVALID") != NULL )
        {
            if ( codePage == CP_UTF8 && isAtLeastVista() )
            {
                dwFlags |= WC_ERR_INVALID_CHARS;
            }
        }
        else if ( dwFlags == 0 && ! (codePage < 50000 && codePage != CP_SYMBOL) )
        {
            context->InvalidRoutine();
            return 0;
        }
    }

    /* evaluate default charcter  */
    const char  *strDefaultChar = NULL;
    BOOL  bUsedDefaultChar = FALSE;
    BOOL  *pUsedDefaultChar = &bUsedDefaultChar;

    if (defaultChar != NULL && (dwFlags & WC_DEFAULTCHAR) == WC_DEFAULTCHAR)
    {
        strDefaultChar = defaultChar;
    }
    else
    {
        /* use our own default character rather than relying on the windows default */
        strDefaultChar = "?";
    }

    /* There are a number of invalid combinations of arguments to
     *  WideCharToMultiByte(), see the MSDN docs. Eliminate them here.
     */
    if ( codePage == CP_UTF8 && dwFlags == WC_ERR_INVALID_CHARS)
    {
        strDefaultChar = NULL;
        pUsedDefaultChar = NULL;
    }
    else if ( ! canUseWideCharFlags(codePage) )
    {
        dwFlags = 0;
        strDefaultChar = NULL;
        pUsedDefaultChar = NULL;
    }

    /* Allocate space for the string, to allow double zero byte termination */
    char *strptr = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sourceLength + 4);
    if (strptr == NULL)
    {
        context->InvalidRoutine();
        return 0;
    }
    memcpy(strptr, source, sourceLength);

    /* Query the number of bytes required to store the Dest string */
    int iBytesNeeded = WideCharToMultiByte( codePage,
                                        dwFlags,
                                        (LPWSTR) strptr,
                                        (int)(sourceLength/2),
                                        NULL,
                                        0,
                                        NULL,
                                        NULL);

    if (iBytesNeeded == 0)
    {
        GlobalFree(strptr);
        return GetLastError();
    }

        // hard error, stop
    char *str = (char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, iBytesNeeded + 4);
    if (str == NULL)
    {
        context->InvalidRoutine();
        return 0;
    }

        /* Do the conversion */
    int iBytesDestination = WideCharToMultiByte(codePage,           // codepage
                                            dwFlags,                // conversion flags
                                            (LPWSTR) strptr,        // source string
                                            (int)(sourceLength/2),  // source string length
                                            str,                    // target string
                                            (int)iBytesNeeded,      // size of target buffer
                                            strDefaultChar,
                                            pUsedDefaultChar);

    if (iBytesDestination == 0) // call to function fails
    {
        GlobalFree(str);          //  free allocated string
        GlobalFree(strptr);       // free allocated string
        return GetLastError();    // return error from function call
    }

    // set whether the default character was used in the output stem
    if (bUsedDefaultChar)
    {
        context->SetStemElement(stem, "!USEDDEFAULTCHAR", context->True());
    }
    else
    {
        context->SetStemElement(stem, "!USEDDEFAULTCHAR", context->False());
    }

    context->SetStemElement(stem, "!TEXT", context->String(str, iBytesNeeded));
    GlobalFree(strptr);          // free allocated string
    GlobalFree(str);             // free allocated string
    return 0;
}

/*
* Syntax:    result = SysToUniCode(string,CodePage,MappingFlags,outstem.)
*/
/*************************************************************************
* Function:  SysToUnicode                                                *
*            Converts an ASCII to UNICODE                                *
*                                                                        *
* Syntax:    result = SysToUniCode(string,CodePage,MappingFlags,outstem.)*
*                                                                        *
* Params:    string       - ascii string to be converted                 *
*            Codepage     - target codepage                              *
*            MappingFlags - Mapping flags                                *
*             outstem.    - stem containg the result                     *
*              .!TEXT     - converted text                               *
*                                                                        *
* Return:    0 - successfull completetion                                *
*            error code from WideCharToMultiByteToWideChars              *

  For available codepages see function SysFromUniCode.

  Additional parameters for codepages:

    ACP        ANSI code page
    MACCP      Macintosh code page
    OEMCP      OEM code page
    SYMBOL     Windows 2000: Symbol code page (42)
    THREAD_ACP Windows 2000: The current thread's ANSI code page
    UTF7       Windows NT 4.0 and Windows 2000: Translate using UTF-7
    UTF8       Windows NT 4.0 and Windows 2000: Translate using UTF-8.
               When this is set, dwFlags must be zero.

    PRECOMPOSED       Always use precomposed characters-that is, characters
                      in which a base character and a nonspacing character
                      have a single character value.
                      This is the default translation option.
                      Cannot be used with MB_COMPOSITE.
    COMPOSITE         Always use composite characters that is,
                      characters in which a base character and a nonspacing
                      character have different character values.
                      Cannot be used with MB_PRECOMPOSED.
    ERR_INVALID_CHARS If the function encounters an invalid input character,
                      it fails and GetLastError returns
                      ERROR_NO_UNICODE_TRANSLATION.
    USEGLYPHCHARS     Use glyph characters instead of control characters.



*************************************************************************/
RexxRoutine4(int, SysToUniCode, RexxStringObject, source, OPTIONAL_CSTRING, codePageOpt,
    OPTIONAL_CSTRING, mappingFlags, RexxStemObject, stem)
{
    // evaluate codepage
    UINT   codePage;
    if (codePageOpt == NULL)
    {
        codePage = GetOEMCP();
    }
    else
    {
        if (_stricmp(codePageOpt,"THREAD_ACP") == 0)
        {
            codePage = CP_THREAD_ACP;
        }
        else if (_stricmp(codePageOpt,"ACP") == 0)
        {
            codePage = CP_ACP;
        }
        else if (_stricmp(codePageOpt,"MACCP") == 0)
        {
            codePage = CP_MACCP;
        }
        else if (_stricmp(codePageOpt,"OEMCP") == 0)
        {
            codePage = CP_OEMCP;
        }
        else if (_stricmp(codePageOpt,"SYMBOL") == 0)
        {
            codePage = CP_SYMBOL;
        }
        else if (_stricmp(codePageOpt,"UTF7") == 0)
        {
            codePage = CP_UTF7;
        }
        else if (_stricmp(codePageOpt,"UTF8") == 0)
        {
            codePage = CP_UTF8;
        }
        else
        {
            codePage = atoi(codePageOpt);
        }
    }

    DWORD  dwFlags = 0;
    // evaluate the mapping flags
    if (mappingFlags != NULL)
    {
        if (mystrstr(mappingFlags, "PRECOMPOSED"))
        {
            dwFlags |= MB_PRECOMPOSED;
        }
        if (mystrstr(mappingFlags, "COMPOSITE"))
        {
            dwFlags  |= MB_COMPOSITE;
        }
        if (mystrstr(mappingFlags, "ERR_INVALID"))
        {
            dwFlags |= MB_ERR_INVALID_CHARS;
        }
        if (mystrstr(mappingFlags, "USEGLYPHCHARS"))
        {
            dwFlags |= MB_USEGLYPHCHARS;
        }
        if (dwFlags == 0)
        {
            context->InvalidRoutine();
            return 0;
        }
    }

    /* Query the number of bytes required to store the Dest string */
    ULONG ulWCharsNeeded = MultiByteToWideChar( codePage, dwFlags,
        context->StringData(source), (int)context->StringLength(source), NULL, NULL);

    if (ulWCharsNeeded == 0)
    {
        return GetLastError();
    }

    ULONG ulDataLen = (ulWCharsNeeded)*2;

    LPWSTR lpwstr = (LPWSTR)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, ulDataLen+4);

    // hard error, stop
    if (lpwstr == NULL)
    {
        context->InvalidRoutine();
        return 0;
    }


    /* Do the conversion */
    ulWCharsNeeded = MultiByteToWideChar(codePage,  dwFlags,
        context->StringData(source), (int)context->StringLength(source),
        lpwstr, ulWCharsNeeded);

    if (ulWCharsNeeded == 0) // call to function fails
    {
        GlobalFree(lpwstr);       // free allocated string
        return GetLastError();
    }

    context->SetStemElement(stem, "!TEXT", context->String((const char *)lpwstr, ulDataLen));
    GlobalFree(lpwstr);        // free allocated string
    return 0;
}

/*************************************************************************
* Function:  SysWinGetPrinters                                           *
*                                                                        *
* Syntax:    call SysWinGetPrinters stem.                                *
*                                                                        *
* Params:    stem. - stem to store infos in                              *
*                                                                        *
* Return:    error number                                                *
*************************************************************************/

RexxRoutine1(uint32_t, SysWinGetPrinters, RexxStemObject, stem)
{
    DWORD realSize = 0;
    DWORD entries = 0;
    DWORD currentSize = 10*sizeof(PRINTER_INFO_2)*sizeof(char);
    char *pArray = (char*) malloc(sizeof(char)*currentSize);

    while (true)
    {
        if (EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS, NULL, 2, (LPBYTE)pArray,
                         currentSize, &realSize, &entries) == 0)
        {
            // this is not a failure if we get ERROR_INSUFFICIENT_BUFFER
            DWORD rc = GetLastError();
            if ( rc != ERROR_INSUFFICIENT_BUFFER )
            {
                free(pArray);
                return rc;
            }
        }
        if ( currentSize >= realSize )
        {
            break;
        }
        currentSize = realSize;
        realSize = 0;
        pArray = (char*) realloc(pArray, sizeof(char)*currentSize);
    }

    PRINTER_INFO_2 *pResult = (PRINTER_INFO_2*) pArray;

    // set stem.0 to the number of entries then add all the found printers
    context->SetStemArrayElement(stem, 0, context->WholeNumber(entries));
    while ( entries-- )
    {
        char  szBuffer[256];
        sprintf(szBuffer,"%s,%s,%s", pResult[entries].pPrinterName, pResult[entries].pDriverName,
                pResult[entries].pPortName);
        context->SetStemArrayElement(stem, entries + 1, context->String(szBuffer));
    }
    free(pArray);
    return 0;          // a little reversed...success is false, failure is true
}

/*************************************************************************
* Function:  SysWinGetDefaultPrinter                                     *
*                                                                        *
* Syntax:    call SysWinGetDefaultPrinter                                *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    string describing default printer                           *
*************************************************************************/

RexxRoutine0(RexxStringObject, SysWinGetDefaultPrinter)
{
    char buffer[256];
    buffer[0] = '\0';

    GetProfileString("Windows", "DEVICE", ",,,", buffer, sizeof(buffer));
    return context->String(buffer);
}


/*************************************************************************
* Function:  SysWinSetDefaultPrinter                                     *
*                                                                        *
* Syntax:    call SysWinSetDefaultPrinter printer                        *
*                                                                        *
* Params:    string describing default printer                           *
*                                                                        *
* Return:    0 on success, otherwise the OS system error number.         *
*************************************************************************/
RexxRoutine1(int, SysWinSetDefaultPrinter, CSTRING, printer)
{
    int count = 0;

    // Two forms of input are allowed.  The old form of
    // "Printername,Drivername,Portname" and for W2K or later just the printer
    // name.  Count the commas to determine which form this might be.
    for ( size_t i = 0; printer[i] != '\0'; i++ )
    {
        if (printer[i] == ',' )
        {
            count++;
        }
    }

    if (count != 0 && count != 2)
    {
        context->InvalidRoutine();
        return 0;
    }
    SetLastError(0);

    if (count == 0 )
    {
        // This is W2K or later and the user specified just the printer name.
        // This code will work on W2K through Vista.
        if (SetDefaultPrinter(printer) == 0)
        {
            return 0;
        }
        else
        {
            return GetLastError();
        }
    }
    else
    {
        // The user still specified the old format. Microssoft
        // only provides WriteProfileString() for backward compatibility to
        // 16-bit Windows, and at some point this may no longer be supported.
        // But it still appears to work on XP.
        if (WriteProfileString("Windows", "DEVICE", printer) == 0)
        {
            return 0;
        }
        else
        {
            if ( SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0L, 0L, SMTO_NORMAL, 1000, NULL) == 0 )
            {
                // If a window just timed out, then GetLastError() will return 0
                // and the user will get the succes code.  If GetLastError()
                // does not return 0, then chances are something really is
                // wrong.
                return 0;
            }
            else
            {
                return GetLastError();
            }
        }
    }
}


/*************************************************************************
* Function:  SysFileCopy                                                 *
*                                                                        *
* Syntax:    call SysFileCopy FROMfile TOfile                            *
*                                                                        *
* Params:    FROMfile - file to be copied.                               *
*            TOfile - target file of copy operation.                     *
*                                                                        *
* Return:    Return code from CopyFile() function.                       *
*************************************************************************/

RexxRoutine2(int, SysFileCopy, CSTRING, fromFile, CSTRING, toFile)
{
    return CopyFile(fromFile, toFile, 0) ? 0 : GetLastError();
}

/*************************************************************************
* Function:  SysFileMove                                                 *
*                                                                        *
* Syntax:    call SysFileMove FROMfile TOfile                            *
*                                                                        *
* Params:    FROMfile - file to be moved.                                *
*            TOfile - target file of move operation.                     *
*                                                                        *
* Return:    Return code from MoveFile() function.                       *
*************************************************************************/

RexxRoutine2(int, SysFileMove, CSTRING, fromFile, CSTRING, toFile)
{
    return MoveFile(fromFile, toFile) ? 0 : GetLastError();
}

/*************************************************************************
* Function:  SysFileExist                                                *
*                                                                        *
* Syntax:    call SysFileExist file                                      *
*                                                                        *
* Params:    file - file to check existance of.                          *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFile, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    // not a file if either of these is one
    return (dwAttrs != 0xffffffff) && ((dwAttrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0);
}

/*************************************************************************
* Function:  SysDirExist                                                 *
*                                                                        *
* Syntax:    call SysDirExist dir                                        *
*                                                                        *
* Params:    dir - dir to check existance of.                            *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileDirectory, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}

/*************************************************************************
* Function:  SysIsFileLink                                               *
*                                                                        *
* Syntax:    call SysIsFileLink file                                     *
*                                                                        *
* Params:    file - file to check if it is a Link (Alias).               *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileLink, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_REPARSE_POINT);
}

/*************************************************************************
* Function:  SysIsFileCompressed                                         *
*                                                                        *
* Syntax:    call SysIsFileCompressed file                               *
*                                                                        *
* Params:    file - file to check if it is compressed.                   *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileCompressed, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_COMPRESSED);
}

/*************************************************************************
* Function:  SysIsFileEncrypted                                          *
*                                                                        *
* Syntax:    call SysIsFileEncrypted file                                *
*                                                                        *
* Params:    file - file to check if it is Encrypted.                    *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileEncrypted, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_ENCRYPTED);
}

/*************************************************************************
* Function:  SysIsFileNotContentIndexed                                  *
*                                                                        *
* Syntax:    call SysIsFileNotContentIndexed file                        *
*                                                                        *
* Params:    file - file to check if it is to be indexed by the indexing *
*             service.                                                   *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileNotContentIndexed, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
}

/*************************************************************************
* Function:  SysIsFileOffline                                            *
*                                                                        *
* Syntax:    call SysIsFileOffline file                                  *
*                                                                        *
* Params:    file - file to check if it is offline                       *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileOffline, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_OFFLINE);
}

/*************************************************************************
* Function:  SysIsFileSparse                                             *
*                                                                        *
* Syntax:    call SysIsFileSparse file                                   *
*                                                                        *
* Params:    file - file to check if it is sparse                        *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileSparse, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_SPARSE_FILE);
}


/*************************************************************************
* Function:  SysIsFileTemporary                                          *
*                                                                        *
* Syntax:    call SysIsFileTemporary file                                *
*                                                                        *
* Params:    file - file to check if it is temporary                     *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysIsFileTemporary, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_TEMPORARY);
}


/*************************************************************************
* Function:  SysFileExists                                               *
*                                                                        *
* Syntax:    call SysFileExists  file                                    *
*                                                                        *
* Params:    file - file to check existence                              *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

RexxRoutine1(logical_t, SysFileExists, CSTRING, file)
{
    DWORD dwAttrs = GetFileAttributes(file);
    return (dwAttrs != 0xffffffff);
}


/*************************************************************************
* Function:  SysGetLongPathName                                          *
*            Converts the specified path to its long form                *
*                                                                        *
* Syntax:    longPath = SysGetLongPathName(path)                         *
*                                                                        *
* Params:    path - a path to an existing file                           *
*                                                                        *
* Return:    longPath - path converted to its long form                  *
*                       NULL string if path doesn't exist or call fails  *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysGetLongPathName, CSTRING, path)
{
  CHAR  longPath[MAX];                 // long version of path
  DWORD code = GetLongPathName(path, longPath, MAX);
  if ((code == 0) || (code >= MAX))    // call failed of buffer too small
  {
    return context->NullString();
  }
  else
  {
    return context->NewStringFromAsciiz(longPath);
  }
}


/*************************************************************************
* Function:  SysGetShortPathName                                         *
*            Converts the specified path to its short form               *
*                                                                        *
* Syntax:    shortPath = SysGetShortPathName(path)                       *
*                                                                        *
* Params:    path - a path to an existing file                           *
*                                                                        *
* Return:    shortPath - path converted to its short form                *
*                        NULL string if path doesn't exist or call fails *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysGetShortPathName, CSTRING, path)
{
  CHAR  shortPath[MAX];                // short version of path
  DWORD code = GetShortPathName(path, shortPath, MAX);
  if ((code == 0) || (code >= MAX))    // call failed of buffer too small
  {
    return context->NullString();
  }
  else
  {
    return context->NewStringFromAsciiz(shortPath);
  }
}


// now build the actual entry list
RexxRoutineEntry rexxutil_routines[] =
{
    REXX_CLASSIC_ROUTINE(SysCls,                      SysCls),
    REXX_TYPED_ROUTINE(SysCurPos,                     SysCurPos),
    REXX_TYPED_ROUTINE(SysCurState,                   SysCurState),
    REXX_CLASSIC_ROUTINE(SysDriveInfo,                SysDriveInfo),
    REXX_CLASSIC_ROUTINE(SysDriveMap,                 SysDriveMap),
    REXX_CLASSIC_ROUTINE(SysDropFuncs,                SysDropFuncs),
    REXX_TYPED_ROUTINE(SysFileDelete,                 SysFileDelete),
    REXX_CLASSIC_ROUTINE(SysFileSearch,               SysFileSearch),
    REXX_TYPED_ROUTINE(SysFileTree,                   SysFileTree),
    REXX_CLASSIC_ROUTINE(SysGetKey,                   SysGetKey),
    REXX_CLASSIC_ROUTINE(SysIni,                      SysIni),
    REXX_CLASSIC_ROUTINE(SysLoadFuncs,                SysLoadFuncs),
    REXX_TYPED_ROUTINE(SysMkDir,                      SysMkDir),
    REXX_CLASSIC_ROUTINE(SysWinVer,                   SysWinVer),
    REXX_CLASSIC_ROUTINE(SysVersion,                  SysVersion),
    REXX_TYPED_ROUTINE(SysRmDir,                      SysRmDir),
    REXX_CLASSIC_ROUTINE(SysSearchPath,               SysSearchPath),
    REXX_TYPED_ROUTINE(SysSleep,                      SysSleep),
    REXX_CLASSIC_ROUTINE(SysTempFileName,             SysTempFileName),
    REXX_TYPED_ROUTINE(SysTextScreenRead,             SysTextScreenRead),
    REXX_TYPED_ROUTINE(SysTextScreenSize,             SysTextScreenSize),
    REXX_TYPED_ROUTINE(SysAddRexxMacro,               SysAddRexxMacro),
    REXX_TYPED_ROUTINE(SysDropRexxMacro,              SysDropRexxMacro),
    REXX_TYPED_ROUTINE(SysReorderRexxMacro,           SysReorderRexxMacro),
    REXX_TYPED_ROUTINE(SysQueryRexxMacro,             SysQueryRexxMacro),
    REXX_TYPED_ROUTINE(SysClearRexxMacroSpace,        SysClearRexxMacroSpace),
    REXX_TYPED_ROUTINE(SysLoadRexxMacroSpace,         SysLoadRexxMacroSpace),
    REXX_TYPED_ROUTINE(SysSaveRexxMacroSpace,         SysSaveRexxMacroSpace),
    REXX_CLASSIC_ROUTINE(SysBootDrive,                SysBootDrive),
    REXX_CLASSIC_ROUTINE(SysSystemDirectory,          SysSystemDirectory),
    REXX_CLASSIC_ROUTINE(SysFileSystemType,           SysFileSystemType),
    REXX_CLASSIC_ROUTINE(SysVolumeLabel,              SysVolumeLabel),
    REXX_TYPED_ROUTINE(SysCreateMutexSem,             SysCreateMutexSem),
    REXX_TYPED_ROUTINE(SysOpenMutexSem,               SysOpenMutexSem),
    REXX_TYPED_ROUTINE(SysCloseMutexSem,              SysCloseMutexSem),
    REXX_TYPED_ROUTINE(SysRequestMutexSem,            SysRequestMutexSem),
    REXX_TYPED_ROUTINE(SysReleaseMutexSem,            SysReleaseMutexSem),
    REXX_TYPED_ROUTINE(SysCreateEventSem,             SysCreateEventSem),
    REXX_TYPED_ROUTINE(SysOpenEventSem,               SysOpenEventSem),
    REXX_TYPED_ROUTINE(SysCloseEventSem,              SysCloseEventSem),
    REXX_TYPED_ROUTINE(SysResetEventSem,              SysResetEventSem),
    REXX_TYPED_ROUTINE(SysPostEventSem,               SysPostEventSem),
    REXX_TYPED_ROUTINE(SysPulseEventSem,              SysPulseEventSem),
    REXX_TYPED_ROUTINE(SysWaitEventSem,               SysWaitEventSem),
    REXX_TYPED_ROUTINE(SysSetPriority,                SysSetPriority),
    REXX_TYPED_ROUTINE(SysSwitchSession,              SysSwitchSession),
    REXX_TYPED_ROUTINE(SysWaitNamedPipe,              SysWaitNamedPipe),
    REXX_TYPED_ROUTINE(SysQueryProcess,               SysQueryProcess),
    REXX_CLASSIC_ROUTINE(SysDumpVariables,            SysDumpVariables),
    REXX_TYPED_ROUTINE(SysSetFileDateTime,            SysSetFileDateTime),
    REXX_TYPED_ROUTINE(SysGetFileDateTime,            SysGetFileDateTime),
    REXX_CLASSIC_ROUTINE(SysStemSort,                 SysStemSort),
    REXX_TYPED_ROUTINE(SysStemDelete,                 SysStemDelete),
    REXX_TYPED_ROUTINE(SysStemInsert,                 SysStemInsert),
    REXX_TYPED_ROUTINE(SysStemCopy,                   SysStemCopy),
    REXX_TYPED_ROUTINE(SysUtilVersion,                SysUtilVersion),
    REXX_CLASSIC_ROUTINE(RxWinExec,                   RxWinExec),
    REXX_CLASSIC_ROUTINE(SysWinEncryptFile,           SysWinEncryptFile),
    REXX_CLASSIC_ROUTINE(SysWinDecryptFile,           SysWinDecryptFile),
    REXX_CLASSIC_ROUTINE(SysGetErrortext,             SysGetErrortext),
    REXX_TYPED_ROUTINE(SysFromUniCode,                SysFromUniCode),
    REXX_TYPED_ROUTINE(SysToUniCode,                  SysToUniCode),
    REXX_TYPED_ROUTINE(SysWinGetPrinters,             SysWinGetPrinters),
    REXX_TYPED_ROUTINE(SysWinGetDefaultPrinter,       SysWinGetDefaultPrinter),
    REXX_TYPED_ROUTINE(SysWinSetDefaultPrinter,       SysWinSetDefaultPrinter),

    REXX_TYPED_ROUTINE(SysShutDownSystem,             SysShutDownSystem),
    REXX_TYPED_ROUTINE(SysFileCopy,                   SysFileCopy),
    REXX_TYPED_ROUTINE(SysFileMove,                   SysFileMove),
    REXX_TYPED_ROUTINE(SysIsFile,                     SysIsFile),
    REXX_TYPED_ROUTINE(SysIsFileDirectory,            SysIsFileDirectory),
    REXX_TYPED_ROUTINE(SysIsFileLink,                 SysIsFileLink),
    REXX_TYPED_ROUTINE(SysIsFileCompressed,           SysIsFileCompressed),
    REXX_TYPED_ROUTINE(SysIsFileEncrypted,            SysIsFileEncrypted),
    REXX_TYPED_ROUTINE(SysIsFileNotContentIndexed,    SysIsFileNotContentIndexed),
    REXX_TYPED_ROUTINE(SysIsFileOffline,              SysIsFileOffline),
    REXX_TYPED_ROUTINE(SysIsFileSparse,               SysIsFileSparse),
    REXX_TYPED_ROUTINE(SysIsFileTemporary,            SysIsFileTemporary),
    REXX_TYPED_ROUTINE(SysFileExists,                 SysFileExists),
    REXX_TYPED_ROUTINE(SysGetLongPathName,            SysGetLongPathName),
    REXX_TYPED_ROUTINE(SysGetShortPathName,           SysGetShortPathName),
    REXX_LAST_ROUTINE()
};

RexxPackageEntry rexxutil_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "REXXUTIL",                          // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    rexxutil_routines,                   // the exported functions
    NULL                                 // no methods in this package
};

// package loading stub.
OOREXX_GET_PACKAGE(rexxutil);

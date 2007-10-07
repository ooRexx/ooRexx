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
*       SysLoadLibrary      -- Load a function package                *
*       SysDropLibrary      -- Drop a function package                *
*       SysPi               -- Return Pi to given precision           *
*       SysSqrt             -- Calculate a square root                *
*       SysExp              -- Calculate an exponent                  *
*       SysLog              -- Return natural log of a number         *
*       SysLog10            -- Return log base 10 of a number         *
*       SysSinh             -- Hyperbolic sine function               *
*       SysCosh             -- Hyperbolic cosine function             *
*       SysTanh             -- Hyperbolic tangent function            *
*       SysPower            -- raise number to non-integer power      *
*       SysSin              -- Sine function                          *
*       SysCos              -- Cosine function                        *
*       SysTan              -- Tangent function                       *
*       SysCotan            -- Cotangent function                     *
*       SysArcSin           -- ArcSine function                       *
*       SysArcCos           -- ArcCosine function                     *
*       SysArcTan           -- ArcTangent function                    *
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

#define  INCL_REXXSAA
#define  INCL_RXMACRO

#include <rexx.h>
#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "APIUtil.h"
#include <math.h>
#include <limits.h>
#include "wintypes.h"
#include "SystemVersion.h"


#define OM_WAKEUP (WM_USER+10)
VOID CALLBACK SleepTimerProc( HWND, UINT, UINT, DWORD);

/*********************************************************************/
/*  Various definitions used by various functions.                   */
/*********************************************************************/

#define CCHMAXPATH MAX_PATH            /* used in os2file            */
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
#define rxstricmp(a,b) _stricmp(a,b)
#define MAX_ENVVAR     1024
#define MAX_LINE_LEN   4096            /* max line length            */

/*********************************************************************/
/*  Various definitions used by the math functions                   */
/*********************************************************************/
#define SINE        0                  /* trig function defines...   */
#define COSINE      3                  /* the ordering is important, */
#define TANGENT     1                  /* as these get transformed   */
#define COTANGENT   2                  /* depending on the angle     */
#define MAXTRIG     3                  /* value                      */
#define ARCSINE     0                  /* defines for arc trig       */
#define ARCCOSINE   1                  /* functions.  Ordering is    */
#define ARCTANGENT  2                  /* not as important here      */

#define pi  3.14159265358979323846l    /* pi value                   */

#define DEGREES    'D'                 /* degrees option             */
#define RADIANS    'R'                 /* radians option             */
#define GRADES     'G'                 /* grades option              */

#define DEFAULT_PRECISION  9           /* default precision to use   */
#define MAX_PRECISION     16           /* maximum available precision*/

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
/* Structures used throughout REXXUTIL.C                             */
/*********************************************************************/

/*********************************************************************/
/* RxTree Structure used by GetLine, OpenFile and CloseFile          */
/*********************************************************************/
typedef struct _GetFileData {
  PUCHAR       buffer;                 /* file read buffer           */
  ULONG        size;                   /* file size                  */
  ULONG        data;                   /* data left in buffer        */
  ULONG        residual;               /* size left to read          */
  PUCHAR       scan;                   /* current scan position      */
  HANDLE       handle;                 /* file handle                */
} GetFileData;

/*********************************************************************/
/* RxTree Structure used by SysTree.                                 */
/*********************************************************************/

typedef struct RxTreeData {
    ULONG count;                       /* Number of lines processed  */
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    ULONG stemlen;                     /* Length of stem             */
    ULONG vlen;                        /* Length of variable value   */
    CHAR TargetSpec[CCHMAXPATH+1];     /* Target filespec            */
    CHAR truefile[CCHMAXPATH+1];       /* expanded file name         */
    CHAR Temp[CCHMAXPATH+80];          /* buffer for returned values */
    CHAR varname[MAX];                 /* Buffer for variable name   */
    ULONG nattrib;                     /* New attrib, diff for each  */
} RXTREEDATA;

/*********************************************************************/
/* RxStemData                                                        */
/*   Structure which describes as generic                            */
/*   stem variable.                                                  */
/*********************************************************************/

typedef struct RxStemData {
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    CHAR ibuf[IBUF_LEN];               /* Input buffer               */
    CHAR varname[MAX];                 /* Buffer for the variable    */
                                       /* name                       */
    CHAR stemname[MAX];                /* Buffer for the variable    */
                                       /* name                       */
    ULONG stemlen;                     /* Length of stem.            */
    ULONG vlen;                        /* Length of variable value   */
    ULONG j;                           /* Temp counter               */
    ULONG tlong;                       /* Temp counter               */
    ULONG count;                       /* Number of elements         */
                                       /* processed                  */
} RXSTEMDATA;

/*********************************************************************/
/* RxFncTable                                                        */
/*   Array of names of the REXXUTIL functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/

static PSZ  RxFncTable[] =
   {
      "SysCls",
      "SysCurpos",
      "SysCurState",
      "SysDriveInfo",
      "SysDriveMap",
      "SysDropFuncs",
      "SysFileDelete",
      "SysFileSearch",
      "SysFileTree",
      "SysGetKey",
      "SysIni",
      "SysLoadFuncs",
      "SysMkDir",
      "SysWinVer",
      "SysVersion",
      "SysRmDir",
      "SysSearchPath",
      "SysSleep",
      "SysTempFileName",
      "SysTextScreenRead",
      "SysTextScreenSize",
      "SysPi",
      "SysSqrt",
      "SysExp",
      "SysLog",
      "SysLog10",
      "SysSinh",
      "SysCosh",
      "SysTanh",
      "SysPower",
      "SysSin",
      "SysCos",
      "SysTan",
      "SysCotan",
      "SysArcSin",
      "SysArcCos",
      "SysArcTan",
      "SysAddRexxMacro",
      "SysDropRexxMacro",
      "SysReorderRexxMacro",
      "SysQueryRexxMacro",
      "SysClearRexxMacroSpace",
      "SysLoadRexxMacroSpace",
      "SysSaveRexxMacroSpace",
      "SysBootDrive",
      "SysSystemDirectory",
      "SysFileSystemType",
      "SysVolumeLabel",
      "SysCreateMutexSem",
      "SysOpenMutexSem",
      "SysCloseMutexSem",
      "SysRequestMutexSem",
      "SysReleaseMutexSem",
      "SysCreateEventSem",
      "SysOpenEventSem",
      "SysCloseEventSem",
      "SysResetEventSem",
      "SysPostEventSem",
      "SysPulseEventSem",
      "SysWaitEventSem",
      "SysSetPriority",
      "SysShutDownSystem",
      "SysSwitchSession",
      "SysWaitNamedPipe",
      "SysQueryProcess",
      "SysDumpVariables",
      "SysSetFileDateTime",
      "SysGetFileDateTime",
      "SysStemSort",
      "SysStemDelete",
      "SysStemInsert",
      "SysStemCopy",
      "SysUtilVersion",
      "RxWinExec",
      "SysWinEncryptFile",
      "SysWinDecryptFile",
      "SysGetErrortext",
      "SysFromUniCode",
      "SysToUniCode",
      "SysWinGetPrinters",
      "SysWinGetDefaultPrinter",
      "SysWinSetDefaultPrinter",
      "SysFileCopy",
      "SysFileMove",
      "SysIsFile",
      "SysIsFileDirectory",
      "SysIsFileLink",
      "SysIsFileCompressed",
      "SysIsFileEncrypted",
      "SysIsFileNotContentIndexed",
      "SysIsFileOffline",
      "SysIsFileSparse",
      "SysIsFileTemporary"
   };

/*********************************************************************/
/* Saved character status                                            */
/*********************************************************************/
static   INT   ExtendedFlag = 0;       /* extended character saved   */
static   UCHAR ExtendedChar;           /* saved extended character   */

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

INT ReadNextBuffer( GetFileData *filedata );

/********************************************************************
* Function:  OpenFile(file, filedata)                               *
*                                                                   *
* Purpose:   Prepares a file for reading.                           *
*                                                                   *
* RC:        0     - file was opened successfully                   *
*            1     - file open error occurred                       *
*********************************************************************/

INT MyOpenFile(
   PSZ          file,                  /* file name                  */
   GetFileData *filedata )             /* global file information    */
{
   DWORD       dwSize;                 /* file status information    */

                                       /* try to open the file       */
  if ((filedata->handle = CreateFile(file, GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                            FILE_FLAG_WRITE_THROUGH, 0))
                            == INVALID_HANDLE_VALUE)
    return 1;                          /* return failure             */

                                       /* retrieve the file size     */
  dwSize = GetFileSize(filedata->handle, NULL);
                                       /* if GetFileSize failed or   */
                                       /* size=0                     */
  if (dwSize == 0xffffffff || !dwSize) {
    CloseHandle(filedata->handle);     /* close the file             */
    return 1;                          /* and quit                   */
  }
  if (dwSize <= MAX_READ) {            /* less than a single buffer  */
                                       /* allocate buffer for file   */
    if (!(filedata->buffer = GlobalAlloc(GMEM_ZEROINIT |
                                         GMEM_FIXED, dwSize))) {
      CloseHandle(filedata->handle);   /* close the file             */
      return 1;
    }
    filedata->size = dwSize;           /* save file size             */
    filedata->residual = 0;            /* no left over information   */
                                       /* read the file in           */
    if (!ReadFile(filedata->handle, filedata->buffer, dwSize,
                &filedata->data, NULL)) {
      GlobalFree(filedata->buffer);    /* free the buffer            */
      CloseHandle(filedata->handle);   /* close the file             */
      return 1;
    }

    filedata->scan = filedata->buffer; /* set position to beginning  */
  }
  else {                               /* need to read partial       */
                                       /* allocate buffer for read   */
    if (!(filedata->buffer = GlobalAlloc(GMEM_ZEROINIT |
                                         GMEM_FIXED, MAX_READ))) {
      CloseHandle(filedata->handle);   /* close the file             */
      return 1;
    }

    filedata->size = dwSize;           /* save file size             */
                                       /* and set remainder          */
    filedata->residual = filedata->size;
                                       /* read the file in           */
    if (ReadNextBuffer(filedata)) {
      GlobalFree(filedata->buffer);    /* free the buffer            */
      CloseHandle(filedata->handle);   /* close the file             */
      return 1;
    }
  }
  return 0;                            /* file is opened             */
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

/********************************************************************
* Function:  ReadNextBuffer(filedata)                               *
*                                                                   *
* Purpose:   Reads the next buffer of data.                         *
*                                                                   *
* RC:        0       buffer was read                                *
*            1     - error occurred reading buffer                  *
*********************************************************************/
INT ReadNextBuffer(
   GetFileData  *filedata )            /* global file information    */
{
  ULONG     size;                      /* size to read               */

                                       /* get size of this read      */
  size = min(MAX_READ, filedata->residual);

                                       /* read the file in           */
  if (!ReadFile(filedata->handle, filedata->buffer, size,
                &filedata->data, NULL))
      return 1;

  if (filedata->data != size)          /* not get all of it?         */
    filedata->residual = 0;            /* no residual                */
  else                                 /* residual is remainder      */
    filedata->residual = filedata->residual - size;

  /* don't check for EOF but read to real end of file     */
  //                                     /* look for a EOF mark        */
  //endptr = memchr(filedata->buffer, CH_EOF, filedata->data);
  //
  //if (endptr) {                        /* found an EOF mark          */
  //                                     /* set new length             */
  //  filedata->data = (ULONG)(endptr - filedata->buffer);
  //  filedata->residual = 0;            /* no residual                */
  //}

  filedata->scan = filedata->buffer;   /* set position to beginning  */
  return 0;
}

/********************************************************************
* Function:  GetLine(line, size, filedata)                          *
*                                                                   *
* Purpose:   Reads a line of data using buffered reads.  At end of  *
*            file, zero is returned to indicate nothing left.       *
*                                                                   *
* RC:        TRUE -  line was read successfully                     *
*            FALSE - end of file was reached                        *
*********************************************************************/

INT GetLine(
   PSZ          line,                  /* returned line              */
   ULONG        size,                  /* size of line buffer        */
   GetFileData *filedata )             /* file handle                */
{
   PUCHAR       scan;                  /* current scan pointer       */
   ULONG        length;                /* line length                */
   ULONG        copylength;            /* copied length              */


  if (!(filedata->data)) {             /* if out of current buffer   */
    if (filedata->residual) {          /* may be another buffer      */
      ReadNextBuffer(filedata);        /* try to read one            */
      if (!filedata->data)             /* nothing more?              */
        return 1;                      /* all done                   */
    }
    else
      return (1);                      /* return EOF condition       */
  }
                                       /* look for a carriage return */
  scan = memchr(filedata->scan, CH_NL, filedata->data);
  if (scan) {                          /* found one                  */
                                       /* calculate the length       */
    length = (ULONG)(scan - filedata->scan);
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

//      if (filedata->data &&            /* if more to read            */
//        *filedata->scan == CH_NL) {    /* may need to skip a char    */
//        filedata->scan++;              /* step past new line         */
//        filedata->data--;              /* reduce size by one         */
//      }
    }
                                       /* may need to skip a char    */
//    else if (*filedata->scan == CH_NL) {
//      filedata->scan++;                /* step past new line         */
//      filedata->data--;                /* reduce size by one         */
//    }
    return 0;                            /* this worked ok           */
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

    /* we don't want the CR character in the result string*/
    /* we have not found LF, so why look for CR                      */
//     if ( line[copylength - 1] == CH_CR )
//     {
//       line[copylength - 1] = '\0';
//     } /* endif */

     /* all data should be read, filedata->data must be zero         */
       filedata->data -= copylength;
     /* scan should be at the end                                    */
       filedata->scan += copylength;     /* set new scan point       */

    /* if no more data to read in the file, return OK     */
//       if (!filedata->residual && !filedata->data)
       if (!filedata->residual)
          return 0;
       else
          return GetLine(line + copylength, size - copylength, filedata);
    }
    else        /* the line is full, scan until LF found but no copy */
    {
       copylength = min(size, filedata->data);
                                         /* copy over the data       */
       memcpy(line, filedata->scan, copylength);
       line[copylength] = '\0';          /* make into ASCIIZ string  */

    /* we don't want the CR character in the result string*/
//     if ( line[copylength - 1] == CH_CR )
//     {
//       line[copylength - 1] = '\0';
//     } /* endif */

       filedata->data  = 0;            /* no data in buffer          */
       filedata->scan += filedata->data; /* set scan point to end    */

       if (filedata->residual)         /* more to read               */
       {
           ReadNextBuffer(filedata);   /* read the next buffer       */
           return GetLine(line + copylength, 0, filedata);
       }
       else
          return 0;
    }
  }

//    /* if line didn't fit into buffer, then we look at it */
//    /* as multiple lines                                             */
//    if ( size > copylength) {
//      /* buffer was not full, read additional data from file */
//    } else {
//      filedata->scan += copylength;    /* set new scan point         */
//
//      if (!filedata->data) {           /* all used up                */
//        if (filedata->residual)        /* more to read               */
//          ReadNextBuffer(filedata);    /* read the next buffer       */
//      }
//
//      return 0;
//    } /* endif */
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
INT  SetFileMode(
  PSZ      file,                       /* file name                  */
  ULONG    attr )                      /* new file attributes        */
{

  DWORD         dwfileattrib;          /* file attributes            */

                                       /* get the file status        */
  if ((dwfileattrib = GetFileAttributes(file)) != 0xffffffff) {
                                       /* if worked                  */
                                       /* set the attributes         */
    if ((dwfileattrib = SetFileAttributes(file,attr)) != 0)
      return 0;   /* give back success flag     */
    else
      return 1;
  } else
    return 1;
}

/********************************************************************
* Function:  string2long(string, number)                            *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2long(
  PSZ string,
  LONG *number)
{
  ULONG    accumulator;                /* converted number           */
  INT      length;                     /* length of number           */
  INT      sign;                       /* sign of number             */

  sign = 1;                            /* set default sign           */
  if (*string == '-') {                /* negative?                  */
    sign = -1;                         /* change sign                */
    string++;                          /* step past sign             */
  }

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS)             /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator * 10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator * sign;        /* return the value           */
  return TRUE;                         /* good number                */
}

/********************************************************************
* Function:  string2ulong(string, number)                           *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2ulong(
  PSZ    string,                       /* string to convert          */
  PULONG number)                       /* converted number           */
{
  ULONG    accumulator;                /* converted number           */
  INT      length;                     /* length of number           */

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS + 1)         /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator * 10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator;               /* return the value           */
  return TRUE;                         /* good number                */
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

ULONG mystrstr(
  CHAR   *haystack,
  CHAR   *needle,
  ULONG   hlen,
  ULONG   nlen,
  BOOL    sensitive)

{
  CHAR line[MAX_LINE_LEN];
  CHAR target[MAX_LINE_LEN];
  ULONG p;
 /* Copy line  - Change nulls to spaces and uppercase if needed      */

  for (p = 0; p < hlen; p++) {

    if (haystack[p] == '\0')
      line[p] = ' ';
    else if (sensitive)
      line[p] = haystack[p];
    else line[p] = (CHAR)toupper(haystack[p]);
  }
  line[p] = '\0';

 /* Copy target  - Change nulls to spaces and uppercase if needed    */

  for (p = 0; p < nlen; p++) {

    if (needle[p] == '\0')
      target[p] = ' ';
    else if (sensitive)
      target[p] = needle[p];
    else target[p] = (CHAR)toupper(needle[p]);
  }
  target[p] = '\0';

  return ((ULONG)strstr(line, target));
}

/*****************************************************************
* Function:  getpath(string, path, filename)                     *
*                                                                *
* Purpose:  This function gets the PATH and FILENAME of the file *
*           target contained in STRING.  The path will end with  *
*           the '\' char if a path is supplied.                  *
*                                                                *
*****************************************************************/

VOID getpath(
  CHAR *string,
  CHAR *path,
  CHAR *filename)
{
  INT    len;                          /* length of filespec         */
  INT    LastSlashPos;                 /* position of last slash     */
  char   szBuff[MAX_PATH];             /* used to save current dir   */
  char   drv[3];                       /* used to change dir         */
  INT    i=0;

  while (string[i] == ' ') i++;        /* skip leading blanks        */
  if (i) {
      len = strlen(string);            /* Get length of full file    */
      if ((string[i] == '\\' || string[i] == '/') ||  /* if first after blank is \ */
          (string[i] == '.' &&
            ((i<len && (string[i+1] == '\\' || string[i+1] == '/')) ||  /* or .\ */
            (i+1<len && string[i+1] == '.' && (string[i+2] == '\\' || string[i+2] == '/')))) ||  /* or ..\ */
            (i<len && string[i+1] == ':'))  /* z: */
                string = &string[i];
  }

  if (!strcmp(string, "."))            /* period case?               */
    strcpy(string, "*.*");             /* make it a *.* request      */
  else if (!strcmp(string, ".."))      /* double period case?        */
    strcpy(string, "..\\*.*");         /* make it a ..\*.* request   */
  len = strlen(string);                /* Get length of full file    */
                                       /* spec                       */
  LastSlashPos = len;                  /* Get max pos of last '\'    */

    /* Step back through string until at begin or at '\' char        */

  while (string[LastSlashPos] != '\\' && string[LastSlashPos] != '/' && LastSlashPos >= 0)
    --LastSlashPos;
  if (LastSlashPos < 0) {              /* no backslash, may be drive */
    if (string[1] == ':') {
      len = MAX_PATH;                  /* set max length             */
                                       /* Save the current drive     */
                                       /* and path                   */
      GetCurrentDirectory(sizeof(szBuff), szBuff);
      /* just copy the drive letter and the colon, omit the rest */
      /* (necessary i.g. if "I:*" is used */
      memcpy(drv, string, 2);
      drv[2] = '\0';

      SetCurrentDirectory(drv);        /* change to specified drive  */
                                       /* Get current directory      */
      GetCurrentDirectory(len, path);
      SetCurrentDirectory(szBuff);     /* go back to where we were   */
                                       /* need a trailing slash?     */
      if (path[strlen(path) - 1] != '\\')
        strcat(path, "\\");            /* add a trailing slash       */
      LastSlashPos = 1;                /* make drive the path        */
    }
    else {
                                       /* Get current directory      */
      GetCurrentDirectory(MAX_PATH, path);
                                       /* need a trailing slash?     */
      if (path[strlen(path) - 1] != '\\')
        strcat(path, "\\");            /* add a trailing slash       */
    }
  }
  else {                               /* have a path                */
    if (string[1] == ':') {            /* have a drive?              */
                                       /* copy over the path         */
      memcpy(path, string, LastSlashPos+1);
      path[LastSlashPos+1] = '\0';     /* make into an ASCII-Z string*/
    }
    else {
      CHAR fpath[MAX_PATH];
      char drive[_MAX_DRIVE];
      char dir[_MAX_DIR];
      char fname[_MAX_FNAME];
      char ext[_MAX_EXT];
      char lastc;


      if (LastSlashPos == 0)  /* only one backslash at the beginning */
      {
          _fullpath(fpath, "\\", MAX_PATH);  /* Get full path        */
          strcat(fpath, &string[1]);
      }
      else
      {

          string[LastSlashPos] = '\0'; /* chop off the path          */
          _fullpath(fpath, string, MAX_PATH); /* Get full path       */
          string[LastSlashPos] = '\\'; /* put the slash back         */
          lastc = fpath[strlen(fpath)-1];
          if (lastc != '\\' && lastc != '/')
              strcat(fpath, &string[LastSlashPos]);
      }
      _splitpath( fpath, drive, dir, fname, ext );

      strcpy(path, drive);
      strcat(path, dir);

      if (!strlen(path)) {             /* invalid path?              */
                                       /* copy over the path         */
         memcpy(path, string, LastSlashPos+1);
         path[LastSlashPos+1] = '\0';  /* make into an ASCII-Z string*/
      }
                                       /* need a trailing slash?     */
      if (path[strlen(path) - 1] != '\\')
        strcat(path, "\\");            /* add a trailing slash       */
    }
  }

    /* Get file name from filespec (just after last '\')             */
  if (string[LastSlashPos+1])          /* have a real name?          */
                                       /* copy it over               */
    strcpy(filename, &string[LastSlashPos+1]);
  else
    strcpy(filename, "*.*");           /* just use wildcards         */
}


/*********************************************************************/
/* Function: ULONG SameAttr(mask, attr)                              */
/*                                                                   */
/* Purpose:  Returns the value TRUE if the attribute is identical to */
/*           that specified by the mask.  If not the same, then      */
/*           returns the value FALSE.                                */
/*                                                                   */
/*********************************************************************/

ULONG SameAttr(
  INT   *mask,
  ULONG  attr,
  ULONG  options)
{

                                       /* if only want directories   */
                                       /* and is not a directory     */
  if ((options&DO_DIRS) && !(options&DO_FILES) && !(attr&FILE_ATTRIBUTE_DIRECTORY))
     return FALSE;
                                       /* if only want files and     */
                                       /* is a directory             */
  if (!(options&DO_DIRS) && (options&DO_FILES) && (attr&FILE_ATTRIBUTE_DIRECTORY))
     return FALSE;

  if (mask[0] == RXIGNORE)
     return  TRUE;


  if (mask[0] < 0 && attr&FILE_ATTRIBUTE_ARCHIVE)
    return  FALSE;

  if (mask[0] > 0 && !(attr&FILE_ATTRIBUTE_ARCHIVE))
    return  FALSE;

  if (mask[1] < 0 && attr&FILE_ATTRIBUTE_DIRECTORY)
    return  FALSE;

  if (mask[1] > 0 && !(attr&FILE_ATTRIBUTE_DIRECTORY))
    return  FALSE;

  if (mask[2] < 0 && attr&FILE_ATTRIBUTE_HIDDEN)
    return  FALSE;

  if (mask[2] > 0 && !(attr&FILE_ATTRIBUTE_HIDDEN))
    return  FALSE;

  if (mask[3] < 0 && attr&FILE_ATTRIBUTE_READONLY)
    return  FALSE;

  if (mask[3] > 0 && !(attr&FILE_ATTRIBUTE_READONLY))
    return  FALSE;

  if (mask[4] < 0 && attr&FILE_ATTRIBUTE_SYSTEM)
    return  FALSE;

  if (mask[4] > 0 && !(attr&FILE_ATTRIBUTE_SYSTEM))
    return  FALSE;

  return  TRUE;
}


/*********************************************************************/
/* Function: ULONG NewAttr(mask, attr)                               */
/*                                                                   */
/* Purpose:  Returns the new file attribute, given the mask of       */
/*           attributes to be cleared/set and the current attribute  */
/*           settings.                                               */
/*                                                                   */
/*********************************************************************/

ULONG NewAttr(
  INT   *mask,
  ULONG  attr)
{


  if (mask[0] == RXIGNORE)
    return  attr;

  if (mask[0] < 0)
    attr &= ~FILE_ATTRIBUTE_ARCHIVE;   /* Clear                      */

  if (mask[0] > 0)
    attr |= FILE_ATTRIBUTE_ARCHIVE;    /* Set                        */

  if (mask[1] < 0)
    attr &= ~FILE_ATTRIBUTE_DIRECTORY; /* Clear                      */

  if (mask[1] > 0)
    attr |= FILE_ATTRIBUTE_DIRECTORY;  /* Set                        */

  if (mask[2] < 0)
    attr &= ~FILE_ATTRIBUTE_HIDDEN;    /* Clear                      */

  if (mask[2] > 0)
    attr |= FILE_ATTRIBUTE_HIDDEN;     /* Set                        */

  if (mask[3] < 0)
    attr &= ~FILE_ATTRIBUTE_READONLY;  /* Clear                      */

  if (mask[3] > 0)
    attr |= FILE_ATTRIBUTE_READONLY;   /* Set                        */

  if (mask[4] < 0)
    attr &= ~FILE_ATTRIBUTE_SYSTEM;    /* Clear                      */

  if (mask[4] > 0)
    attr |= FILE_ATTRIBUTE_SYSTEM;     /* Set                        */
  return  attr;
}

/*********************************************************************/
/* Function: ULONG FormatFile(                                       */
/*                                                                   */
/* Purpose:  Returns the new file attribute, given the mask of       */
/*           attributes to be cleared/set and the current attribute  */
/*           settings.                                               */
/*                                                                   */
/*********************************************************************/

ULONG FormatFile(
  RXTREEDATA   *ldp,                   /* Pointer to local data      */
  INT          *smask,                 /* Mask of attributes to      */
                                       /* search for                 */
  INT          *dmask,                 /* Mask of attributes to set  */
  ULONG         options,               /* Search and output format   */
  WIN32_FIND_DATA *wfd )               /* Find File data struct      */

{
  SYSTEMTIME systime;
  FILETIME ftLocal;
  ULONG nattrib;                       /* New file attributes        */
  ULONG rc;


/* File-attributes need to be changed independent of the             */
/* output format                                                     */

  nattrib = NewAttr((INT *)dmask,  wfd->dwFileAttributes);
                                       /* need to change?            */
  if (nattrib != wfd->dwFileAttributes)
                                       /* try to set attributes      */
     if (SetFileMode(ldp->truefile, nattrib&~FILE_ATTRIBUTE_DIRECTORY))
        nattrib = wfd->dwFileAttributes;/* use old ones if it failed */

  if (options&NAME_ONLY)               /* name only?                 */
    strcpy(ldp->Temp, ldp->truefile);  /* just copy it over          */

  else {
                                       /* Convert UTC to Local File  */
                                       /* Time,  and then to system  */
                                       /* format.                    */
    FileTimeToLocalFileTime(&wfd->ftLastWriteTime,&ftLocal);
    FileTimeToSystemTime(&ftLocal, &systime);

    if (options&LONG_TIME)             /* need the long time format? */
                                       /* format as such             */
      sprintf(ldp->Temp, "%4d-%02d-%02d %02d:%02d:%02d  %10lu  ",
        systime.wYear,
        systime.wMonth,
        systime.wDay,
        systime.wHour,
        systime.wMinute,
        systime.wSecond,
        wfd->nFileSizeLow);
    else
    {
      if (options&EDITABLE_TIME)       /* need the "smushed" form?   */
                                       /* format as such             */
      wsprintf(ldp->Temp, "%02d/%02d/%02d/%02d/%02d  %10lu  ",
        (systime.wYear+100)%100,
        systime.wMonth,
        systime.wDay,
        systime.wHour,
        systime.wMinute,
        wfd->nFileSizeLow);
      else                             /* give the pretty form       */
        wsprintf(ldp->Temp, "%2d/%02d/%02d  %2d:%02d%c  %10lu  ",
          systime.wMonth,
          systime.wDay,
          (systime.wYear+100)%100,
          (systime.wHour < 13 && systime.wHour != 0 ?
          systime.wHour:
          (abs(systime.wHour-(SHORT)12))),
          systime.wMinute,
          ((systime.wHour < 12 ||
          systime.wHour == 24)?'a':'p'),
          wfd->nFileSizeLow);


    }
                                       /* get the attributes            */
/*   nattrib = NewAttr((INT *)dmask,  wfd->dwFileAttributes);           */
                                       /* need to change?               */
/*    if (nattrib != wfd->dwFileAttributes)                             */
                                       /* try to set attributes         */
/*       if (SetFileMode(ldp->truefile, nattrib&~FILE_ATTRIBUTE_DIRECTORY)) */
/*          nattrib = wfd->dwFileAttributes; use old ones if it failed  */

                                       /* format the attributes now     */
    wsprintf(ldp->Temp, "%s%c%c%c%c%c  %s", ldp->Temp,
      ((nattrib&FILE_ATTRIBUTE_ARCHIVE)?'A':'-'),
      ((nattrib&FILE_ATTRIBUTE_DIRECTORY)?'D':'-'),
      ((nattrib&FILE_ATTRIBUTE_HIDDEN)?'H':'-'),
      ((nattrib&FILE_ATTRIBUTE_READONLY)?'R':'-'),
      ((nattrib&FILE_ATTRIBUTE_SYSTEM)?'S':'-'),
      ldp->truefile);
  }
                                       /* Place new string in Stem      */
  ldp->vlen = strlen(ldp->Temp);
  ldp->count++;
  ltoa(ldp->count, ldp->varname+ldp->stemlen, 10);
  ldp->shvb.shvnext = NULL;
  ldp->shvb.shvname.strptr = ldp->varname;
  ldp->shvb.shvname.strlength = strlen(ldp->varname);
  ldp->shvb.shvvalue.strptr = ldp->Temp;
  ldp->shvb.shvvalue.strlength = ldp->vlen;
  ldp->shvb.shvnamelen = ldp->shvb.shvname.strlength;
  ldp->shvb.shvvaluelen = ldp->vlen;
  ldp->shvb.shvcode = RXSHV_SET;
  ldp->shvb.shvret = 0;

  rc = RexxVariablePool(&ldp->shvb);
  if (rc & (RXSHV_BADN | RXSHV_MEMFL))
  {
    return INVALID_ROUTINE;
  }
  return 0;                            /* good return                   */
}

/*****************************************************************************
* Function: RecursiveFindFile( FileSpec, path, lpd, smask, dmask, options )  *
*                                                                            *
* Purpose:  Finds all files starting with FileSpec, and will look down the   *
*           directory tree if required.                                      *
*                                                                            *
* Params:   FileSpec - ASCIIZ string which designates filespec to search     *
*                       for.                                                 *
*           path     - ASCIIZ string for current path                        *
*                                                                            *
*           ldp      - Pointer to local data structure.                      *
*                                                                            *
*           smask    - Array of integers which describe the source attribute *
*                       mask.  Only files with attributes matching this mask *
*                       will be found.                                       *
*                                                                            *
*           dmask    - Array of integers which describe the target attribute *
*                       mask.  Attributes of all found files will be set     *
*                       using this mask.                                     *
*                                                                            *
*             Note:  Both source and targets mask are really arrays of       *
*                    integers.  Each index of the mask corresponds           *
*                    to a different file attribute.  Each indexe and         *
*                    its associated attribute follows:                       *
*                                                                            *
*                         mask[0] = FILE_ARCHIVED                            *
*                         mask[1] = FILE_DIRECTORY                           *
*                         mask[2] = FILE_HIDDEN                              *
*                         mask[3] = FILE_READONLY                            *
*                         mask[4] = FILE_SYSTEM                              *
*                                                                            *
*                    A negative value at a given index indicates that        *
*                    the attribute bit of the file is not set.  A positive   *
*                    number indicates that the attribute should be set.      *
*                    A value of 0 indicates a "Don't Care" setting.          *
*                                                                            *
*           options  - The search/output options.  The following options     *
*                       may be ORed together when calling this function:     *
*                                                                            *
*                    RECURSE     - Indicates that function should search     *
*                                   all child subdirectories recursively.    *
*                    DO_DIRS     - Indicates that directories should be      *
*                                   included in the search.                  *
*                    DO_FILES    - Indicates that files should be included   *
*                                   in the search.                           *
*                    NAME_ONLY   - Indicates that the output should be       *
*                                   restricted to filespecs only.            *
*                    EDITABLE_TIME - Indicates time and date fields should   *
*                                   be output as one timestamp.              *
*                    LONG_TIME   - Indicates time and date fields should     *
*                                   be output as one long formatted timestamp*
*                                                                            *
*****************************************************************************/

LONG RecursiveFindFile(
  PSZ         FileSpec,                /* Filespecs to search for    */
  PSZ         path,                    /* current directory          */
  RXTREEDATA *ldp,                     /* Pointer to local data      */
  INT        *smask,                   /* Mask of attributes to      */
                                       /* search for                 */
  INT        *dmask,                   /* Mask of attributes to set  */
  ULONG       options )                /* Search and output format   */
                                       /* options                    */
{


  WIN32_FIND_DATA wfd;                 /* Find File data struct      */

  CHAR  staticBuffer[CCHMAXPATH+1];    /* dynamic memory             */
  CHAR  *tempfile = staticBuffer;      /* Used to hold temp file name*/
  HANDLE fHandle;                      /* search handle used by      */
                                       /* FindFirstFile()            */

  /* other changes not flagged (see all tempfile checks) */
                                       /* if > than static mem, use  */
                                       /* dynamic mem. dynamic mem   */
                                       /* must be FREED before func  */
                                       /* returns...                 */
  size_t maxsize = strlen(path) + strlen(ldp->TargetSpec);
                                       /* build spec name            */
  if (maxsize >= CCHMAXPATH) {
    tempfile = (CHAR*) malloc(sizeof(CHAR)*(maxsize+1));
  }
  wsprintf(tempfile, "%s%s", path, ldp->TargetSpec);
  if ((fHandle = FindFirstFile(tempfile,&wfd)) != INVALID_HANDLE_VALUE) {

                                       /* Get the rest of the files  */
    do {
                                       /* dot directory?             */
      if (!strcmp(wfd.cFileName, ".") ||
          !strcmp(wfd.cFileName, ".."))
          continue;                    /* skip this one              */
                                       /* got requested attributes?  */
      if (SameAttr(smask, wfd.dwFileAttributes, options)) {
                                       /* build the full name        */
        wsprintf(ldp->truefile, "%s%s", path,wfd.cFileName);
                                       /* passed back ok?            */
        if (FormatFile(ldp, smask, dmask, options, &wfd)) {
          if (tempfile != staticBuffer) free(tempfile);
          return INVALID_ROUTINE;      /* error on non-zero          */
        }
      }
    } while (FindNextFile(fHandle,&wfd));
    FindClose(fHandle);
  }

  if (options&RECURSE) {               /* need to recurse?           */
    wsprintf(tempfile, "%s*", path);   /* build new target spec      */
                                       /* and have some              */
    if ((fHandle = FindFirstFile(tempfile,&wfd)) != INVALID_HANDLE_VALUE) {
      do {
                                       /* dot directory?             */
        if (!strcmp(wfd.cFileName, ".") ||
            !strcmp(wfd.cFileName, ".."))
          continue;                    /* skip this one              */
        maxsize = strlen(path) + strlen(wfd.cFileName) + 1;
        if (maxsize >= CCHMAXPATH) {
          if (tempfile != staticBuffer) {
            free(tempfile);
          }
          tempfile = (CHAR*) malloc(sizeof(CHAR)*(maxsize+1));
        }
                                       /* build the new directory    */
        wsprintf(tempfile, "%s%s\\", path, wfd.cFileName);
                                       /* search the next level      */
        if (RecursiveFindFile(ldp->TargetSpec, tempfile, ldp,
            smask, dmask, options)) {
          if (tempfile != staticBuffer) free(tempfile);
          return INVALID_ROUTINE;      /* error on non-zero          */
        }
      } while (FindNextFile(fHandle,&wfd));
      FindClose(fHandle);
    }
  }
  if (tempfile != staticBuffer) free(tempfile);
  return VALID_ROUTINE;                /* finished                   */
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
  BOOL Unique = FALSE;

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
    Unique = TRUE;
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
      Unique = TRUE;                   /* got one                    */

    FindClose(hSearch);
    SetErrorMode(fuErrorMode);         /* Enable previous setting    */

    /** Make sure we are not wasting our time                        */

    num = (num+1)%max;

    if (num == start && !Unique) {
      Unique = TRUE;
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

LONG APIENTRY SysCls(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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
* Function:  SysCurPos - positions cursor in OS/2 session                *
*                                                                        *
* Syntax:    call SysCurPos [row, col]                                   *
*                                                                        *
* Params:    row   - row to place cursor on                              *
*            col   - column to place cursor on                           *
*                                                                        *
* Return:    row, col                                                    *
*************************************************************************/

LONG APIENTRY SysCurPos(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  LONG   inrow;                        /* Row to change to           */
  LONG   incol;                        /* Col to change to           */
  COORD NewHome;                       /* Position to move cursor    */
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */
  HANDLE hStdout;                      /* Handle to Standard Out     */

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* set default result         */
                                       /* check arguments            */
  if ((numargs != 0 && numargs != 2))  /* wrong number?              */
    return INVALID_ROUTINE;            /* raise an error             */

                                       /* get handle to stdout       */
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

                                       /* get current position, and  */
                                       /* continue only if in        */
                                       /* character mode             */
  if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) {

    sprintf(retstr->strptr, "%d %d", csbiInfo.dwCursorPosition.Y,
              csbiInfo.dwCursorPosition.X);
    retstr->strlength = strlen(retstr->strptr);

    if (numargs != 0) {                /* reset position to given    */
      if (!RXVALIDSTRING(args[0]) ||   /* not real arguments give    */
          !RXVALIDSTRING(args[1]))
        return INVALID_ROUTINE;        /* raise an error             */
                                       /* convert row to binary      */
      if (!string2long(args[0].strptr, &inrow) || inrow < 0)
        return INVALID_ROUTINE;        /* return error               */
                                       /* convert row to binary      */
      if (!string2long(args[1].strptr, &incol) || incol < 0)
        return INVALID_ROUTINE;        /* return error               */

      NewHome.Y = (SHORT)inrow;        /* convert to short form      */
      NewHome.X = (SHORT)incol;        /* convert to short form      */
                                       /* Set the cursor position    */
      SetConsoleCursorPosition(hStdout, NewHome);
    }
  }

  return VALID_ROUTINE;                /* no error on call           */
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

LONG APIENTRY SysCurState(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  CONSOLE_CURSOR_INFO CursorInfo;      /* info about cursor          */
  HANDLE hStdout;                      /* Handle to Standard Out     */

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);
                                       /* validate the arguments     */
  if (numargs != 1)
    return INVALID_ROUTINE;
                                       /* get handle to stdout       */
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                                       /* Get the cursor info        */
  GetConsoleCursorInfo(hStdout,&CursorInfo);
                                       /* Get state and validate     */
  if (rxstricmp(args[0].strptr, "ON") == 0)
    CursorInfo.bVisible = TRUE;
  else if (rxstricmp(args[0].strptr, "OFF") == 0)
    CursorInfo.bVisible = FALSE;
  else
    return INVALID_ROUTINE;            /* Invalid state              */
                                       /* Set the cursor info        */
  SetConsoleCursorInfo(hStdout,&CursorInfo);
  return VALID_ROUTINE;                /* no error on call           */
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

LONG APIENTRY SysDriveInfo(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  PSZ    arg;                          /* Temp var for holding args  */

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

  arg = args[0].strptr;                /* get argument pointer       */
                                       /* drive letter?              */
  if (strlen(arg) == 2 &&            /* if second letter isn't : bye */
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

LONG APIENTRY SysDriveMap(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  CHAR     temp[MAX];                  /* Entire drive map built here*/

  CHAR     tmpstr[MAX];                /* Single drive entries built */
                                       /* here                       */
  CHAR     DeviceName[3];              /* Device name or drive letter*/
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

LONG APIENTRY SysDropFuncs(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  INT     entries;                     /* Num of entries             */
  INT     j;                           /* Counter                    */

  if (numargs != 0)                    /* no arguments for this      */
    return INVALID_ROUTINE;            /* raise an error             */

  retstr->strlength = 0;               /* return a null string result*/

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++)
    RexxDeregisterFunction(RxFncTable[j]);

  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysFileDelete                                               *
*                                                                        *
* Syntax:    call SysFileDelete file                                     *
*                                                                        *
* Params:    file - file to be deleted.                                  *
*                                                                        *
* Return:    Return code from DeleteFile() function.                     *
*************************************************************************/

LONG APIENTRY SysFileDelete(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  if (!DeleteFile(args[0].strptr))     /* delete the file            */
     RETVAL(GetLastError())            /* pass back return code      */
  else
     RETVAL(0)
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

LONG APIENTRY SysFileSearch(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  PSZ         target;                  /* search string              */
  PSZ         file;                    /* search file                */
  PSZ         opts;                    /* option string              */
  CHAR        line[MAX_LINE_LEN];      /* Line read from file        */
  ULONG       ptr;                     /* Pointer to char str found  */
  ULONG       num = 0;                 /* Line number                */
  ULONG       len;                     /* Length of string           */
  ULONG       len2;                    /* Length of string           */
  ULONG       rc = 0;                  /* Return code of this func   */
  BOOL        linenums = FALSE;        /* Set TRUE for linenums in   */
                                       /* output                     */
  BOOL        sensitive = FALSE;       /* Set TRUE for case-sens     */
                                       /* search                     */
  RXSTEMDATA  ldp;                     /* stem data                  */
  PUCHAR      buffer_pointer;          /* current buffer pointer     */
  GetFileData filedata;                /* file read information      */

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
                                       /* validate arguments         */
  if (numargs < 3 || numargs > 4 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;            /* raise an error             */

  buffer_pointer = NULL;               /* nothing in buffer          */

  target = args[0].strptr;             /* get target pointer         */
  file = args[1].strptr;               /* get file name              */

  if (numargs == 4) {                  /* process options            */
    opts = args[3].strptr;             /* point to the options       */
    if (strstr(opts, "N") || strstr(opts, "n"))
      linenums = TRUE;

    if (strstr(opts, "C") || strstr(opts, "c"))
      sensitive = TRUE;
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

    if (ptr != '\0') {

      if (linenums) {
        wsprintf(ldp.ibuf, "%d ", num);
        len2 = strlen(ldp.ibuf);
//      memcpy(ldp.ibuf+len2, line, len);
        memcpy(ldp.ibuf+len2, line, min(len, IBUF_LEN-len2));
//      ldp.vlen = len+len2;
        ldp.vlen = min(IBUF_LEN, len+len2);
      }
      else {
        memcpy(ldp.ibuf, line, len);
        ldp.vlen = len;
      }
      ldp.count++;
      ltoa(ldp.count, ldp.varname+ldp.stemlen, 10);

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
  ltoa(ldp.count, ldp.ibuf, 10);
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

/*************************************************************************
* Function:  SysFileTree                                                 *
*                                                                        *
* Syntax:    call SysFileTree filespec, stem [, options]                 *
*                                                                        *
* Params:    filespec - Filespec to search for (may include * and ?).    *
*            stem     - Name of stem var to store results in.            *
*            options  - Any combo of the following:                      *
*                        'B' - Search for files and directories.         *
*                        'D' - Search for directories only.              *
*                        'F' - Search for files only.                    *
*                        'O' - Only output file names.                   *
*                        'S' - Recursively scan subdirectories.          *
*                        'T' - Combine time & date fields into one.      *
*                        'L' - Long time format                          *
*                        'I' - Case Insensitive search.                  *
*                                                                        *
* Return:    NO_UTIL_ERROR   - Successful.                               *
*            ERROR_NOMEM     - Out of memory.                            *
*************************************************************************/

LONG APIENTRY SysFileTree(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  CHAR        buff1[MAX];              /* buffer1 ...we may need to  */
  CHAR        buff2[MAX];              /* buffer2 ...alloc new mem...*/
  CHAR       *FileSpec = buff1;        /* File spec to look for      */
  CHAR       *path = buff2;            /* path to search along       */
  PUCHAR      optptr;                  /* option scan pointer        */
  ULONG       options;                 /* Mask of options            */
  ULONG       y;                       /* Temp counter (II)          */
  INT         smask[5];                /* Source attribute mask      */
  INT         dmask[5];                /* Target attribute mask      */
  RXTREEDATA  ldp;                     /* local data                 */

  options = DO_FILES|DO_DIRS;          /* Clear if we should not     */
                                       /* display files              */
  smask[0] = RXIGNORE;                 /* No mask unless specified   */
  dmask[0] = RXIGNORE;                 /* No mask unless specified   */
  path[0] = '\0';                      /* no path yet                */
  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */

                                       /* validate arguments         */
  if (numargs < 2 || numargs > 5 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;            /* Invalid call to routine    */

  if (args[0].strlength > 255) {
    FileSpec = (char*) malloc(sizeof(char)*(args[0].strlength+8));
    if (FileSpec == NULL)
      return INVALID_ROUTINE;          /* Invalid call to routine    */
    path     = (char*) malloc(sizeof(char)*(args[0].strlength+8));
    if (path == NULL) {
      free(FileSpec);
      return INVALID_ROUTINE;          /* Invalid call to routine    */
    }
  }
                                       /* initialize data area       */
  ldp.count = 0;
  strcpy(ldp.varname, args[1].strptr);
  ldp.stemlen = args[1].strlength;
                                       /* uppercase the name         */
  memupper(ldp.varname, strlen(ldp.varname));

  if (ldp.varname[ldp.stemlen-1] != '.')
    ldp.varname[ldp.stemlen++] = '.';

                                       /* get file spec              */
  memcpy(FileSpec, args[0].strptr, args[0].strlength);
                                       /* zero terminate             */
  if (FileSpec[args[0].strlength])
    FileSpec[args[0].strlength] = 0x00;

   /** If FileSpec ends in \ then append *.*  *                      */

  if (FileSpec[args[0].strlength-1] == '\\')
    strcat(FileSpec, "*.*");
                                       /* in case of '.' or '..'     */
                                       /* append wildcard '\*.*'     */
  else if (FileSpec[args[0].strlength-1] == '.')
    strcat(FileSpec, "\\*.*");

  if (numargs >= 3 &&                  /* check third option         */
      !RXNULLSTRING(args[2])) {
    if (!args[2].strlength) {          /* a zero length string isn't */
      if (FileSpec != buff1) {         /* valid                      */
        free(FileSpec);
        free(path);
      }
      return INVALID_ROUTINE;
    }
    optptr = args[2].strptr;           /* copy the pointer           */
    while (*optptr) {                  /* while more characters      */
      switch(toupper(*optptr)) {       /* process each option        */
        case 'S':                      /* recurse on subdirectories  */
          options |= RECURSE;          /* Should we recurse          */
          break;

        case 'O':                      /* only return names          */
          options |= NAME_ONLY;        /* Should include names only  */
          break;

        case 'T':                      /* editable time?             */
          options |= EDITABLE_TIME;    /* create editable timestamp  */
          break;

        case 'L':                      /* long time format?          */
          options |= LONG_TIME;        /* create timestamp           */
          break;

        case 'F':                      /* include only files?        */
          options &= ~DO_DIRS;         /* Should not include dirs !  */
          options |= DO_FILES;         /* Should include files !     */
          break;

        case 'D':                      /* include only directories?  */
          options |= DO_DIRS;          /* Should include dirs !      */
          options &= ~DO_FILES;        /* Should not include files ! */
          break;

        case 'B':                      /* include both files and dirs*/
          options |= DO_DIRS;          /* Should include dirs !      */
          options |= DO_FILES;         /* Should include files !     */
          break;

        case 'I':                      /* case insensitive?          */
          break;                       /* nop on Windows             */

        default:                       /* unknown option             */
          if (FileSpec != buff1) {
            free(FileSpec);
            free(path);
          }
          return INVALID_ROUTINE;      /* raise an error             */
      }
      optptr++;                        /* step the option pointer    */
    }
  }

  if (numargs >= 4 &&                  /* check fourth option        */
      !RXNULLSTRING(args[3])) {
    optptr = args[3].strptr;           /* copy the pointer           */

    smask[0] = smask[1] = smask[2] = smask[3] = smask[4] = 0;
    if (strlen(optptr) > 5) {          /* too long to be good?       */
      if (FileSpec != buff1) {         /* raise an error             */
        free(FileSpec);
        free(path);
      }
      return INVALID_ROUTINE;
    }
    y = 0;                             /* starting at the first      */
    while (*optptr) {                  /* while still in the string  */

      if (*optptr == '+')              /* turn it on?                */
        smask[y] = 1;                  /* set mask appropriately     */

      else if (*optptr == '-')         /* turning it off?            */
        smask[y] = -1;                 /* use a negative             */
      else if (*optptr == '*')         /* don't care?                */
        smask[y] = 0;                  /* that stays zero            */
      else {                           /* invalid setting            */
        if (FileSpec != buff1) {
          free(FileSpec);
          free(path);
        }
        return INVALID_ROUTINE;
      }
      y++;                             /* step to the next attribute */
      optptr++;                        /* step the pointer           */
    }
  }

  if (numargs == 5) {                  /* check fifth argument       */
    dmask[0] = dmask[1] = dmask[2] = dmask[3] = dmask[4] = 0;
    optptr = args[4].strptr;           /* copy the pointer           */
    if (strlen(optptr) > 5) {          /* too long to be good?       */
      if (FileSpec != buff1) {         /* raise an error             */
        free(FileSpec);
        free(path);
      }
      return INVALID_ROUTINE;
    }
    y = 0;                             /* starting at the first      */
    while (*optptr) {                  /* while still in the string  */

      if (*optptr == '+')              /* turn it on?                */
        dmask[y] = 1;                  /* set mask appropriately     */

      else if (*optptr == '-')         /* turning it off?            */
        dmask[y] = -1;                 /* use a negative             */
      else if (*optptr == '*')         /* don't care?                */
        dmask[y] = 0;                  /* that stays zero            */
      else {                           /* invalid setting            */
        if (FileSpec != buff1) {
          free(FileSpec);
          free(path);
        }
        return INVALID_ROUTINE;
      }
      y++;                             /* step to the next attribute */
      optptr++;                        /* step the pointer           */
    }
    dmask[1] = 0;                      /* Ignore directory bit of    */
                                       /* destination mask           */
  }
                                       /* get path and name          */
  getpath(FileSpec, path, ldp.TargetSpec);
                                       /* recursively search         */
  if (RecursiveFindFile(FileSpec, path, &ldp, smask, dmask, options)) {
    if (FileSpec != buff1) {
      free(FileSpec);
      free(path);
    }
    return INVALID_ROUTINE;
  }
                                       /* return lines read          */
  ltoa(ldp.count, ldp.Temp, 10);
  ldp.varname[ldp.stemlen] = '0';
  ldp.varname[ldp.stemlen+1] = 0;
  ldp.shvb.shvnext = NULL;
  ldp.shvb.shvname.strptr = ldp.varname;
  ldp.shvb.shvname.strlength = ldp.stemlen+1;
  ldp.shvb.shvnamelen = ldp.stemlen+1;
  ldp.shvb.shvvalue.strptr = ldp.Temp;
  ldp.shvb.shvvalue.strlength = strlen(ldp.Temp);
  ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
  ldp.shvb.shvcode = RXSHV_SET;
  ldp.shvb.shvret = 0;

                                       /* did we allocate memory?    */
  if (FileSpec != buff1) {
    free(FileSpec);                    /* yes, so free it            */
    free(path);
  }

  if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;                /* no error on call           */
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

LONG APIENTRY SysGetKey(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  INT       tmp;                       /* Temp var used to hold      */
                                       /* keystroke value            */
  BOOL      echo = TRUE;               /* Set to FALSE if we         */
                                       /* shouldn't echo             */

  if (numargs > 1)                     /* too many arguments         */
    return INVALID_ROUTINE;            /* raise an error             */

  if (numargs == 1) {                  /* validate arguments         */
    if (!rxstricmp(args[0].strptr, "NOECHO"))
      echo = FALSE;
    else if (rxstricmp(args[0].strptr, "ECHO"))
      return INVALID_ROUTINE;          /* Invalid option             */
  }
  if (ExtendedFlag) {                  /* if have an extended        */
    tmp = ExtendedChar;                /* get the second char        */
    ExtendedFlag = FALSE;              /* do a real read next time   */
  }
  else {
    tmp = _getch();                    /* read a character           */

    if (RUNNING_95)
    {
       /* The _getch() function and therefore the underlying OS function      */
       /* ReadConsoleInput does not works correctly for W´95 and 98.          */
       /* The function should read the console and remove the read character  */
       /* from the console input buffer. But it does not really remove it !   */
       /* For this a second ReadConsoleInput is executed.                     */
       /* Under NT 4.0 it works correctly                                     */

       HANDLE       hStdin = 0;
       INPUT_RECORD ConInpRec;
       DWORD        dNum;
                                       /* if not an extended char    */
       if (!(tmp == 0x00) || (tmp == 0xe0))
       {
                                     /* get the console input handle */
         hStdin = GetStdHandle(STD_INPUT_HANDLE);

         ReadConsoleInput( hStdin,  /* read again one character from */
                            &ConInpRec,/* the console input buffer   */
                            1L,
                            &dNum );
       }
    }

                                       /* If a function key or arrow */
    if ((tmp == 0x00) || (tmp == 0xe0)) {
      ExtendedChar = _getch();         /* Read another character     */
      ExtendedFlag = TRUE;
    }
    else
      ExtendedFlag = FALSE;
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

LONG APIENTRY SysIni(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  ULONG       x;                       /* Temp counter               */
  ULONG       len;                     /* Len var used when creating */
                                       /* stem                       */
  ULONG       lSize;                   /* Size of queried info buffer*/
                                       /* area                       */
  PSZ         IniFile;                 /* Ini file (USER, SYSTEM,    */
                                       /* BOTH, file)                */
  PSZ         App;                     /* Application field          */
  PSZ         Key;                     /* Key field                  */
  CHAR       *Val=NULL;                /* Ptr to data associated w/  */
                                       /* App->Key                   */
  LONG        Error = FALSE;           /* Set to true if error       */
                                       /* encountered                */
  BOOL        WildCard = FALSE;        /* Set to true if a wildcard  */
                                       /* operation                  */
  BOOL        QueryApps;               /* Set to true if a query     */
                                       /* operation                  */
  BOOL        terminate = TRUE;        /* perform WinTerminate call  */
  RXSTEMDATA  ldp;                     /* local data                 */
  PSZ         next;                    /* next returned string       */
  ULONG       buffersize;              /* return buffer size         */


  buffersize = retstr->strlength;      /* save default buffer size   */
  retstr->strlength = 0;               /* set return value           */
  Key = "";
                                       /* validate arguments         */
  if (numargs < 2 ||
      numargs > 4 ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;
                                       /* get pointers to args       */
  IniFile = args[0].strptr;
  if (!RXVALIDSTRING(args[0]))         /* not specified?             */
    IniFile = "WIN.INI";               /* default to WIN.INI         */
  App = args[1].strptr;

  if (numargs >= 3 && args[2].strptr)
    Key = args[2].strptr;

  if (numargs == 4)
    Val = args[3].strptr;
                                       /* Check KEY and APP values   */
                                       /* for "WildCard"             */
  if (!rxstricmp(App, "ALL:")) {
    App = "";
    QueryApps = TRUE;
    WildCard = TRUE;

    if (numargs != 3)
      return INVALID_ROUTINE;          /* Error - Not enough args    */
    else
      x = 2;                           /* Arg number of STEM variable*/
  }

  else if (!rxstricmp(Key, "ALL:")) {
    Key = "";
    Val = "";
    QueryApps = FALSE;
    WildCard = TRUE;

    if (numargs != 4)
      return INVALID_ROUTINE;          /* Error - Not enough args    */

    else
      x = 3;                           /* Arg number of STEM variable*/
  }
                                       /* If this is a "WildCard     */
                                       /* search, then allocate mem  */
                                       /* for stem struct and get the*/
                                       /* stem name                  */
  if (WildCard == TRUE) {

    ldp.count = 0;                     /* get the stem variable name */
    strcpy(ldp.varname, args[x].strptr);
    ldp.stemlen = args[x].strlength;
                                       /* uppercase the name         */
    memupper(ldp.varname, strlen(ldp.varname));

    if (ldp.varname[ldp.stemlen-1] != '.')
      ldp.varname[ldp.stemlen++] = '.';
  }

                                         /* get value if is a query    */
  if ((numargs == 3 && rxstricmp(Key, "DELETE:")) ||
      WildCard == TRUE) {
    lSize = 0x0000ffffL;
                                       /* Allocate a large buffer    */
    if (!(Val = GlobalAlloc(GPTR, lSize))) {
      BUILDRXSTRING(retstr, ERROR_NOMEM);
      return VALID_ROUTINE;
    }

    if (WildCard && QueryApps)
                                       /* Retrieve the names of all  */
                                       /* applications.              */
      lSize = GetPrivateProfileString(NULL, NULL, "", Val, lSize, IniFile);
    else if (WildCard && !QueryApps)
                                       /* Retrieve all keys for an   */
                                       /* application                */
      lSize = GetPrivateProfileString(App, NULL, "", Val, lSize, IniFile);
    else
                                       /* Retrieve a single key value*/
      lSize = GetPrivateProfileString(App, Key, "", Val, lSize, IniFile);

    if (lSize <= 0) {
      Error = TRUE;
      BUILDRXSTRING(retstr, ERROR_RETSTR);
//      GlobalFree(Val);                  /* release buffer             */
    }
    else if (WildCard == FALSE) {
      if (lSize > buffersize)
        if (!(retstr->strptr = GlobalAlloc(GMEM_FIXED, lSize))) { /* use GlobalAlloc */
          if (GlobalFlags(Val) != GMEM_INVALID_HANDLE) GlobalFree(Val);  /* release buffer */
          BUILDRXSTRING(retstr, ERROR_NOMEM);
          return VALID_ROUTINE;
        }
      memcpy(retstr->strptr, Val, lSize);
      retstr->strlength = lSize;
//      GlobalFree(Val);                 /* release buffer             */
    }
  }
  else {                               /* Set or delete Key          */

    if (!rxstricmp(Key, "DELETE:") || (numargs == 2) ||
        !RXVALIDSTRING(args[2]))
                                       /* Delete application and all */
                                       /* associated keys            */
      Error = !WritePrivateProfileString(App, NULL, NULL, IniFile);
    else if (!rxstricmp(Val, "DELETE:") ||
        !RXVALIDSTRING(args[3]))
                                       /* Delete a single key        */
      Error = !WritePrivateProfileString(App, Key, NULL, IniFile);
    else {
      lSize = args[3].strlength;
                                       /* Set a single key value     */
      Error = !WritePrivateProfileString(App, Key, Val, IniFile);
    }

    if (Error) {
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

  if (WildCard == TRUE) {              /* fill stem variable         */

    if (Error == FALSE) {
      x = 0;
      ldp.count = 0;

      do {
  /* Copy string terminated by \0 to Temp.  Last string will end     */
  /* in \0\0 and thus have a length of 0.                            */
        len = 0;

        next = &Val[x];                /* point to string            */
        len = strlen(next);            /* get string length          */
                                       /* if non-zero length, then   */
                                       /* set the stem element       */
        if (len != 0) {
          x += (len+1);                /* Increment pointer past the */
                                       /* new string                 */
          strcpy(ldp.ibuf, next);
          ldp.vlen = len;
          ldp.count++;
          ltoa(ldp.count, ldp.varname+ldp.stemlen, 10);

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
          if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) {
            if (GlobalFlags(Val) != GMEM_INVALID_HANDLE) GlobalFree(Val);  /* release buffer */
            return INVALID_ROUTINE;    /* error on non-zero          */
          }
        }
      }

      while (Val[x] != '\0');
    }

    else
      ldp.count = 0;

    if (GlobalFlags(Val) != GMEM_INVALID_HANDLE)
    {
      GlobalFree(Val);
      Val = NULL;
    }

                                       /* set number returned        */
    ltoa(ldp.count, ldp.ibuf, 10);
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
  if (Val != 0 && ((GlobalFlags(Val) != GMEM_INVALID_HANDLE)))
    GlobalFree(Val);  /* release buffer                              */

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

LONG APIENTRY SysLoadFuncs(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs > 0)
    return INVALID_ROUTINE;

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++) {
    RexxRegisterFunctionDll(RxFncTable[j],
          "REXXUTIL", RxFncTable[j]);
  }
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

LONG APIENTRY SysMkDir(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  if (!CreateDirectory(args[0].strptr, NULL)) /* make the directory  */
      RETVAL(GetLastError())            /* pass back return code     */
  else
      RETVAL(0)
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

LONG APIENTRY SysGetErrortext(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  DWORD  errnum;
  char  *errmsg;

  if (numargs != 1)
                                       /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;

  errnum = atoi(args[0].strptr);
  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,errnum,0,(LPSTR)&errmsg,64,NULL) == 0)
    retstr->strptr[0] = 0x00;
  else {                               /* succeeded                  */
    if (strlen(errmsg)>=retstr->strlength)
      retstr->strptr = GlobalAlloc(GMEM_ZEROINIT | GMEM_FIXED, strlen(errmsg+1));
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

LONG APIENTRY SysWinEncryptFile(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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
      rc = EncryptFile(args[0].strptr);
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
* Function:  SysWinDecryptFile (W2K only)                                *
*                                                                        *
* Syntax:    call SysWinDecryptFile filename                             *
*                                                                        *
* Params:    filename - file to be decrypted                             *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*            Return code from DecryptFile()                              *
*************************************************************************/

LONG APIENTRY SysWinDecryptFile(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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

LONG APIENTRY SysWinVer(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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

LONG APIENTRY SysVersion(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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

LONG APIENTRY SysRmDir(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  if (!RemoveDirectory(args[0].strptr)) /* remove the directory      */
      RETVAL(GetLastError())           /* pass back return code      */
  else
      RETVAL(0)
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

LONG APIENTRY SysSearchPath(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  UCHAR    szFullPath[_MAX_PATH];      /* returned file name         */
  UCHAR    szCurDir[MAX_ENVVAR + _MAX_PATH]; /* current directory    */
  UCHAR    szEnvStr[MAX_ENVVAR];
  PSZ      opts;                       /* option string              */

  LPTSTR pszOnlyFileName;              /* parm for searchpath        */
  LPTSTR lpPath;                       /* ptr to search path+        */
//  LPTSTR lpEnv;                        // ptr to env
  UINT   errorMode;

//  SearchFlag = SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT |
//               SEARCH_CUR_DIRECTORY ;
                                       /* validate arguments         */
  if (numargs < 2 || numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;


                                       /* search current directory   */
  GetCurrentDirectory(_MAX_PATH, szCurDir);
  lpPath=strcat(szCurDir,";");         /*  and specified path        */

//  lpEnv=getenv(args[0].strptr);   /* do not search current dir  */

  if (GetEnvironmentVariable(args[0].strptr, szEnvStr, MAX_ENVVAR))
/*  if (lpEnv)  */
     lpPath=strcat(szCurDir,szEnvStr); /* szEnvStr instead of lpEnv  */

  if (numargs == 3) {                  /* process options            */

    opts = args[2].strptr;             /* point to the options       */
    if ((*opts == 'N') || (*opts == 'n'))
    {
//      lpPath=getenv(args[0].strptr);   /* do not search current dir  */
      GetEnvironmentVariable(args[0].strptr, szEnvStr, MAX_ENVVAR);
      lpPath = szEnvStr;
    }
    else if ((*opts == 'C') || (*opts == 'c'));
                                       /* search current 1st(default)*/
    else
      return INVALID_ROUTINE;          /* Invalid option             */
  }
                                       /* use DosSearchPath          */
//  DosSearchPath(SearchFlag, args[0].strptr, args[1].strptr,
//                buf, sizeof(buf));

  errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
  if (0 == SearchPath(
         (LPCTSTR)lpPath,              /* path srch, NULL will+      */
         (LPCTSTR)args[1].strptr,      /* address if filename        */
         NULL,                         /* filename contains .ext     */
         _MAX_PATH,                    /* size of fullname buffer    */
         szFullPath,                   /* where to put results       */
         &pszOnlyFileName))
      szFullPath[0]='\0';              /* set to NULL if failure     */

  BUILDRXSTRING(retstr, szFullPath);   /* pass back result           */
  SetErrorMode(errorMode);
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysSleep                                                    *
*                                                                        *
* Syntax:    call SysSleep secs                                          *
*                                                                        *
* Params:    secs - Number of seconds to sleep.                          *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*************************************************************************/

LONG APIENTRY SysSleep(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  LONG secs;                           /* Time to sleep in secs      */
  MSG msg;
  BOOL UseMsgLoop;                     /* for VAC++                  */

  LONG milliseconds;
  LONG secs_buf;
  LONG length;
  LONG digits;
  PCHAR string;

  if (numargs != 1)                    /* Must have one argument     */
    return INVALID_ROUTINE;

  /* code fragment taken from lrxutil.c: */
  string = args[0].strptr;             /* point to the string        */
  length = args[0].strlength;          /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS)             /* or too long                */
    return INVALID_ROUTINE;            /* not valid                  */

  secs = 0;                            /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      break;                           /* get out of this loop       */
    secs = secs * 10 + (*string - '0');/* add to accumulator         */
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  secs_buf = secs;                     /* remember the seconds       */
  secs = secs * 1000;                  /* convert to milliseconds    */
  if (*string == '.') {                /* have a decimal number?     */
    string++;                          /* step over the decimal      */
    length--;                          /* reduce the length          */
    milliseconds = 0;                  /* no milliseconds yet        */
    digits = 0;                        /* and no digits              */
    while (length) {                   /* while more digits          */
      if (!isdigit(*string))           /* not a digit?               */
        return INVALID_ROUTINE;        /* not a valid number         */
      if (++digits <= 3)               /* still within precision?    */
                                       /* add to accumulator         */
        milliseconds = milliseconds * 10 + (*string - '0');
      length--;                        /* reduce length              */
      string++;                        /* step pointer               */
    }
    while (digits < 3) {               /* now adjust up              */
      milliseconds = milliseconds * 10;/* by powers of 10            */
      digits++;                        /* count the digit            */
    }
    secs += milliseconds;              /* now add in the milliseconds*/
  }
  else if (length != 0)                /* invalid character found?   */
    return INVALID_ROUTINE;            /* this is invalid            */


//  /* get number of seconds      */
//  if (!string2long(args[0].strptr, &secs) || secs < 0)
//    return INVALID_ROUTINE;            /* raise error if bad         */

  /* for VAC++ begin*/
  UseMsgLoop = RexxSetProcessMessages(TRUE); /* retrieve current setting */
  RexxSetProcessMessages(UseMsgLoop);  /* set back settings          */

  if (UseMsgLoop)
  {
  /* for VAC++ end */
      if ( !(SetTimer(NULL, 0, (secs), (TIMERPROC) SleepTimerProc)) )
        return INVALID_ROUTINE;   /* no timer available, logic error */
      while (GetMessage (&msg, NULL, 0, 0) ) {
        if (msg.message == OM_WAKEUP)  /* If our message exit loop   */
          break;
        TranslateMessage( &msg );
        DispatchMessage ( &msg );
      }
  } else Sleep(secs); /* for VAC++ */
  BUILDRXSTRING(retstr, NO_UTIL_ERROR);
  return VALID_ROUTINE;
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

LONG APIENTRY SysTempFileName(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  CHAR   filler;                       /* filler character           */

  if (numargs < 1 ||                   /* validate arguments         */
      numargs > 2 ||
      !RXVALIDSTRING(args[0]) ||
      args[0].strlength > 512)
    return INVALID_ROUTINE;

  if (numargs == 2 &&                  /* get filler character       */
      !RXNULLSTRING(args[1])) {
    if (args[1].strlength != 1)        /* must be one character      */
      return INVALID_ROUTINE;
    filler = args[1].strptr[0];
  }
  else
    filler = '?';
                                       /* get the file id            */
  GetUniqueFileName(args[0].strptr, filler, retstr->strptr);
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
LONG APIENTRY SysTextScreenRead(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  LONG  row;                           /* Row from which to start    */
  LONG  col;                           /* Column from which to start */
  LONG  len;                           /* nunber of chars to be read */
  LONG  lPos,lPosOffSet;               /* positioning                */
                                       /* (132x50)                   */
  LONG lBufferLen = 16000;             /* default: 200x80 characters */

  COORD coordLine;                     /* coordinates of where to    */
                                       /* read characters from       */
  DWORD dwCharsRead,dwSumCharsRead;    /* Handle to Standard Out     */
  HANDLE hStdout;
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */
  PCH temp_strptr;

  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

  if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
    len = csbiInfo.dwSize.Y * csbiInfo.dwSize.X;
  else
    RETVAL(GetLastError())

  if (numargs < 2 ||                   /* validate arguments         */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !string2long(args[0].strptr, &row) || row < 0 ||
      !string2long(args[1].strptr, &col) || col < 0)
    return INVALID_ROUTINE;

  if (numargs == 3) {                  /* check the length           */
    if (!RXVALIDSTRING(args[2]) ||     /* bad string?                */
        !string2long(args[2].strptr, &len) || len < 0)
      return INVALID_ROUTINE;          /* error                      */
  }
  coordLine.X = (SHORT)col;
  coordLine.Y = (SHORT)row;

  if (len > (LONG)retstr->strlength) {
                                       /* allocate a new one         */
    if (!(temp_strptr = GlobalAlloc(GMEM_FIXED , len))) { /* use GlobalAlloc */
      BUILDRXSTRING(retstr, ERROR_NOMEM);
      return VALID_ROUTINE        ;
    }
    else
      retstr->strptr = temp_strptr;
  }

  if (len < lBufferLen)
    lBufferLen = len;

  lPos = 0;                            /* current position           */
  lPosOffSet = row * csbiInfo.dwSize.X + col;   /* add offset if not started at beginning */
  dwSumCharsRead = 0;

  while (lPos < len ) {

    if (!ReadConsoleOutputCharacter(hStdout, &retstr->strptr[lPos], lBufferLen, coordLine, &dwCharsRead))
    {
      RETVAL(GetLastError())
    }


    lPos = lPos + lBufferLen;
    coordLine.Y = (SHORT)((lPos + lPosOffSet) / csbiInfo.dwSize.X);
    coordLine.X = (SHORT)((lPos + lPosOffSet) % csbiInfo.dwSize.X);
    dwSumCharsRead = dwSumCharsRead + dwCharsRead;
  }

  retstr->strlength = dwSumCharsRead;
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SysTextScreenSize                                           *
*                                                                        *
* Syntax:    call SysTextScreenSize                                      *
*                                                                        *
* Return:    Size of screen in row and columns returned as:  row, col    *
*************************************************************************/

LONG APIENTRY SysTextScreenSize(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  HANDLE    hStdout;                   /* Handle to Standard Out     */

  CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */

  if (numargs != 0)                    /* no arguments on this       */
    return INVALID_ROUTINE;

  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                                       /* if in character mode       */
  if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) {

    wsprintf(retstr->strptr, "%d %d", csbiInfo.dwSize.Y, csbiInfo.dwSize.X);
    retstr->strlength = strlen(retstr->strptr);
  }
  else
  {
    strcpy(retstr->strptr, "0 0");
    retstr->strlength = strlen(retstr->strptr);
  }
  return VALID_ROUTINE;
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

LONG APIENTRY RxWinExec(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  ULONG       CmdShow;                 /* show window style flags    */
  INT         index;                   /* table index                */
  ULONG       pid;                     /* PID or error return code   */
  ULONG       length;                  /* length of option           */
  STARTUPINFO si;
  PROCESS_INFORMATION procInfo;


PSZ    show_styles[] =                 /* show window types          */
    {"SHOWNORMAL",
     "SHOWNOACTIVATE",
     "SHOWMINNOACTIVE",
     "SHOWMINIMIZED",
     "SHOWMAXIMIZED",
     "HIDE",
     "MINIMIZE"
     };

ULONG  show_flags[] =                  /* show window styles        */
    {SW_SHOWNORMAL,
     SW_SHOWNOACTIVATE,
     SW_SHOWMINNOACTIVE,
     SW_SHOWMINIMIZED,
     SW_SHOWMAXIMIZED,
     SW_HIDE,
     SW_MINIMIZE
    };

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */


  if (numargs < 1 ||
      numargs > 2 ||                   /* should be 1 or two args    */
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      args[0].strlength > MAX_PATH)
    return INVALID_ROUTINE;            /* Invalid call to routine    */

  CmdShow=0;                           /* initialize show flags      */
                                       /* validate arguments         */
  if (numargs < 2 ||                   /* no show window style?      */
      args[1].strptr == NULL)
    CmdShow += SW_SHOWNORMAL;          /* set default show style     */
  else {                               /* check various show styles  */
    length = args[1].strlength;        /* get length of option       */
                                       /* search style table         */
    for (index = 0;
         index < sizeof(show_styles)/sizeof(PSZ);
         index++) {
                                       /* find a match?              */
      if (length == strlen(show_styles[index]) &&
          !memicmp(args[1].strptr, show_styles[index], length)) {
        CmdShow += show_flags[index];  /* add to the style           */
        break;
      }
    }/* for */
                                       /* not found?                 */
    if (index == sizeof(show_styles)/sizeof(PSZ))
      return INVALID_ROUTINE;          /* raise an error             */
  }/* else */

  ZeroMemory(&procInfo, sizeof(procInfo));
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = CmdShow;

  if ( CreateProcess(NULL, (LPSTR)args[0].strptr, NULL, NULL, FALSE, 0, NULL,
                     NULL, &si, &procInfo ) ) {
    pid = procInfo.dwProcessId;
  }
  else {
    pid = GetLastError();
    if ( pid > 31 )                    /* maintain compatibility to  */
      pid = -pid;                      /* versions < ooRexx 3.1.2    */
  }
                                       /* return value as string     */
  sprintf(retstr->strptr, "%d", pid);
  retstr->strlength = strlen(retstr->strptr);

  /* Close process / thread handles as they are not used / needed.   */
  CloseHandle(procInfo.hProcess);
  CloseHandle(procInfo.hThread);

  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysAddRexxMacro(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  ULONG       position;                /* added position             */

  if (numargs < 2 || numargs > 3 ||    /* wrong number?              */
      !RXVALIDSTRING(args[0]) ||       /* first is omitted           */
      !RXVALIDSTRING(args[1]))         /* second is omitted          */
    return INVALID_ROUTINE;            /* raise error condition      */

  position = RXMACRO_SEARCH_BEFORE;    /* set default search position*/
  if (numargs == 3) {                  /* have an option?            */
    if (RXZEROLENSTRING(args[2]))      /* null string?               */
      return INVALID_ROUTINE;          /* this is an error           */
                                       /* 'B'efore?                  */
    else if (toupper(args[2].strptr[0]) == 'B')
      position = RXMACRO_SEARCH_BEFORE;/* place before               */
                                       /* 'A'fter?                   */
    else if (toupper(args[2].strptr[0]) == 'A')
      position = RXMACRO_SEARCH_AFTER; /* place after                */
    else                               /* parm given was bad         */
      return INVALID_ROUTINE;          /* raise an error             */
  }
                                       /* try to add the macro       */
  RETVAL(RexxAddMacro(args[0].strptr, args[1].strptr, position))
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

LONG APIENTRY SysReorderRexxMacro(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  ULONG       position;                /* added position             */

  if (numargs != 2 ||                  /* wrong number?              */
      !RXVALIDSTRING(args[0]) ||       /* first is omitted           */
      RXZEROLENSTRING(args[1]))        /* null string?               */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* 'B'efore?                  */
  if (toupper(args[1].strptr[0]) == 'B')
    position = RXMACRO_SEARCH_BEFORE;  /* place before               */
                                       /* 'A'fter?                   */
  else if (toupper(args[1].strptr[0]) == 'A')
    position = RXMACRO_SEARCH_AFTER;   /* place after                */
  else                                 /* parm given was bad         */
    return INVALID_ROUTINE;            /* raise an error             */
                                       /* try to add the macro       */
  RETVAL(RexxReorderMacro(args[0].strptr, position));
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

LONG APIENTRY SysDropRexxMacro(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */

  RETVAL(RexxDropMacro(args[0].strptr));  /* try to drop the macro   */
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

LONG APIENTRY SysQueryRexxMacro(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  USHORT      position;                /* returned position          */

  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* query the macro position   */
  if (RexxQueryMacro(args[0].strptr, &position))
    retstr->strlength = 0;             /* return a null string       */
  else {
                                       /* before?                    */
    if (position == RXMACRO_SEARCH_BEFORE)
      retstr->strptr[0] = 'B';         /* return a 'B'               */
    else
      retstr->strptr[0] = 'A';         /* must be 'A'fter            */
    retstr->strlength = 1;             /* returning one character    */
  }
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysClearRexxMacroSpace(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs)                         /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
  RETVAL(RexxClearMacroSpace());       /* clear the macro space      */
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

LONG APIENTRY SysSaveRexxMacroSpace(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* clear the macro space      */
  RETVAL(RexxSaveMacroSpace(0, NULL, args[0].strptr));
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

LONG APIENTRY SysLoadRexxMacroSpace(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* clear the macro space      */
  RETVAL(RexxLoadMacroSpace(0, NULL, args[0].strptr));
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

LONG APIENTRY SysBootDrive(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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

LONG APIENTRY SysSystemDirectory(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
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

LONG APIENTRY SysFileSystemType(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  UCHAR *      drive;
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

LONG APIENTRY SysVolumeLabel(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  UCHAR *      drive;
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

LONG APIENTRY SysCreateMutexSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */
  SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  handle = 0;                          /* zero the handle            */
  if (numargs == 1) {                  /* request for named sem      */
                                       /* create it by name          */
    handle = CreateMutex(&sa, FALSE, args[0].strptr);
    if (!handle)                       /* may already be created     */
                                       /* try to open it             */
      handle = OpenMutex(MUTEX_ALL_ACCESS, TRUE, args[0].strptr);
  }
  else                                 /* unnamed semaphore          */
    handle = CreateMutex(&sa, FALSE, NULL);
  if (!handle)                         /* failed?                    */
    retstr->strlength = 0;             /* return null string         */
  else {
                                       /* format the result          */
    sprintf(retstr->strptr, "%lu", handle);
                                       /* set the length             */
    retstr->strlength = strlen(retstr->strptr);
  }
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysOpenMutexSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */
  SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */

                                       /* get a binary handle        */
                                       /* try to open it             */
  handle = OpenMutex(MUTEX_ALL_ACCESS, TRUE, args[0].strptr);
  wsprintf(retstr->strptr, "%lu", handle); /* format the return code */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysReleaseMutexSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (!ReleaseMutex(handle))
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysCloseMutexSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (!CloseHandle(handle))
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysRequestMutexSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */
  APIRET    rc;                        /* creation return code       */
  LONG      timeout;                   /* timeout value              */

  if (numargs < 1 ||                   /* too few, or                */
      numargs > 2 ||                   /* too many, or               */
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    return INVALID_ROUTINE;            /* raise error condition      */
  timeout = INFINITE;                  /* default is no timeout      */
  if (numargs == 2) {                  /* have a timeout value?      */
                                       /* get number of seconds      */
    if (!string2long(args[1].strptr, &timeout))
      return INVALID_ROUTINE;          /* raise error if bad         */
  }
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
                                       /* request the semaphore      */
  rc = WaitForSingleObject(handle, timeout);
  if (rc == WAIT_FAILED)
    wsprintf(retstr->strptr, "%d", GetLastError());
  else
    wsprintf(retstr->strptr, "%d", rc);/* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysCreateEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */
  SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
  BOOL      manual;

  handle = 0;                          /* zero the handle            */
  if (numargs > 2)
    return INVALID_ROUTINE;            /* raise error condition      */
  else if (numargs == 2)
     manual = TRUE;
  else manual = FALSE;

  if ((numargs >= 1) && args[0].strptr != 0 && (strlen(args[0].strptr) > 0))
  {                                    /* request for named sem      */
                                       /* create it by name          */
    handle = CreateEvent(&sa, manual, FALSE, args[0].strptr);
    if (!handle)                       /* may already be created     */
                                       /* try to open it             */
      handle = OpenEvent(EVENT_ALL_ACCESS, TRUE, args[0].strptr);
  }
  else                                 /* unnamed semaphore          */
    handle = CreateEvent(&sa, manual, FALSE, NULL);
  if (!handle)                         /* failed?                    */
    retstr->strlength = 0;             /* return null string         */
  else
  {
                                       /* format the result          */
    sprintf(retstr->strptr, "%lu", handle);
                                       /* set the length             */
    retstr->strlength = strlen(retstr->strptr);
  }
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysOpenEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */
  SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  handle = OpenEvent(EVENT_ALL_ACCESS, TRUE, args[0].strptr); /* try to open it */
  wsprintf(retstr->strptr, "%lu", handle); /* format the return code */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysPostEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (!SetEvent(handle))
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysResetEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (!ResetEvent(handle))
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysPulseEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (!PulseEvent(handle))
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysCloseEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */

  if (numargs != 1)                    /* Only one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (!CloseHandle(handle))
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysWaitEventSem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HANDLE    handle;                    /* mutex handle               */
  APIRET    rc;                        /* creation return code       */
  LONG      timeout;                   /* timeout value              */

  if (numargs < 1 ||                   /* too few, or                */
      numargs > 2 ||                   /* too many, or               */
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    return INVALID_ROUTINE;            /* raise error condition      */
  timeout = INFINITE;       /* default is no timeout      */
  if (numargs == 2) {                  /* have a timeout value?      */
                                       /* get number of seconds      */
    if (!string2long(args[1].strptr, &timeout))
      return INVALID_ROUTINE;          /* raise error if bad         */
  }
                                       /* get a binary handle        */
  if (!string2ulong(args[0].strptr, (PULONG)&handle))
    return INVALID_ROUTINE;            /* raise error if bad         */
                                       /* request the semaphore      */
  rc = WaitForSingleObject(handle, timeout);
  if (rc == WAIT_FAILED)
    wsprintf(retstr->strptr, "%d", GetLastError());
  else
    wsprintf(retstr->strptr, "%d", rc);/* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysSetPriority(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  LONG      class;                     /* priority class             */
  LONG      level;                     /* priority level             */
  APIRET    rc;                        /* creation return code       */
  HANDLE    process;
  HANDLE    thread;
  DWORD     iclass=-1;

  process = GetCurrentProcess();
  thread = GetCurrentThread();

  if (numargs != 2 ||                  /* must have two              */
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    return INVALID_ROUTINE;            /* raise error condition      */

  if (string2long(args[0].strptr, &class))
  {
    if (class < 0 || class > 3)
        return INVALID_ROUTINE;        /* raise error condition      */
    switch (class) {
       case 0: iclass = IDLE_PRIORITY_CLASS;
               break;
       case 1: iclass = NORMAL_PRIORITY_CLASS;
               break;
       case 2: iclass = HIGH_PRIORITY_CLASS;
               break;
       case 3: iclass = REALTIME_PRIORITY_CLASS;
    };
  }
  else
  if (!stricmp(args[0].strptr, "REALTIME")) iclass = REALTIME_PRIORITY_CLASS; else
  if (!stricmp(args[0].strptr, "HIGH")) iclass = HIGH_PRIORITY_CLASS; else
  if (!stricmp(args[0].strptr, "NORMAL")) iclass = NORMAL_PRIORITY_CLASS; else
  if (!stricmp(args[0].strptr, "IDLE")) iclass = IDLE_PRIORITY_CLASS;
  if (iclass == -1)  return INVALID_ROUTINE; /* raise error condition*/


  if (string2long(args[1].strptr, &level))
  {
      if (level < -15 || level > 15)
          return INVALID_ROUTINE;      /* raise error condition      */
  }
  else
  {
      if (!stricmp(args[1].strptr, "ABOVE_NORMAL")) level = THREAD_PRIORITY_ABOVE_NORMAL; else
      if (!stricmp(args[1].strptr, "BELOW_NORMAL")) level = THREAD_PRIORITY_BELOW_NORMAL; else
      if (!stricmp(args[1].strptr, "HIGHEST")) level = THREAD_PRIORITY_HIGHEST; else
      if (!stricmp(args[1].strptr, "LOWEST")) level = THREAD_PRIORITY_LOWEST; else
      if (!stricmp(args[1].strptr, "NORMAL")) level = THREAD_PRIORITY_NORMAL; else
      if (!stricmp(args[1].strptr, "IDLE")) level = THREAD_PRIORITY_IDLE; else
      if (!stricmp(args[1].strptr, "TIME_CRITICAL")) level = THREAD_PRIORITY_TIME_CRITICAL;
      else return INVALID_ROUTINE;     /* raise error condition      */
  }

  rc = SetPriorityClass(process, iclass);
  if (rc)
     rc = SetThreadPriority(thread, level);

  if (!rc)
     RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysQueryProcess(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs > 1)                  /* none or one argument accepted */
    return INVALID_ROUTINE;            /* raise error condition      */

  if ((numargs == 0) || (!stricmp(args[0].strptr, "PID")))
  {
      ltoa(GetCurrentProcessId(), retstr->strptr, 10);
      retstr->strlength = strlen(retstr->strptr);
      return VALID_ROUTINE;            /* good completion            */
  } else
  if (!stricmp(args[0].strptr, "TID"))
  {
      ltoa(GetCurrentThreadId(), retstr->strptr, 10);
      retstr->strlength = strlen(retstr->strptr);
      return VALID_ROUTINE;            /* good completion            */
  } else
  if (!stricmp(args[0].strptr, "PPRIO"))
  {
      LONG p;
      p = GetPriorityClass(GetCurrentProcess());

      switch(p) {
        case HIGH_PRIORITY_CLASS: strcpy(retstr->strptr, "HIGH");
            break;
        case IDLE_PRIORITY_CLASS: strcpy(retstr->strptr, "IDLE");
            break;
        case NORMAL_PRIORITY_CLASS: strcpy(retstr->strptr, "NORMAL");
            break;
        case REALTIME_PRIORITY_CLASS: strcpy(retstr->strptr, "REALTIME");
            break;
        default: strcpy(retstr->strptr, "UNKNOWN");
      }
      retstr->strlength = strlen(retstr->strptr);
      return VALID_ROUTINE;            /* good completion            */
  } else
  if (!stricmp(args[0].strptr, "TPRIO"))
  {
      LONG p;
      p = GetThreadPriority(GetCurrentThread());

      switch(p) {
        case THREAD_PRIORITY_ABOVE_NORMAL: strcpy(retstr->strptr, "ABOVE_NORMAL");
            break;
        case THREAD_PRIORITY_BELOW_NORMAL: strcpy(retstr->strptr, "BELOW_NORMAL");
            break;
        case THREAD_PRIORITY_HIGHEST: strcpy(retstr->strptr, "HIGHEST");
            break;
        case THREAD_PRIORITY_IDLE: strcpy(retstr->strptr, "IDLE");
            break;
        case THREAD_PRIORITY_LOWEST: strcpy(retstr->strptr, "LOWEST");
            break;
        case THREAD_PRIORITY_NORMAL: strcpy(retstr->strptr, "NORMAL");
            break;
        case THREAD_PRIORITY_TIME_CRITICAL: strcpy(retstr->strptr, "TIME_CRITICAL");
            break;
        default: strcpy(retstr->strptr, "UNKNOWN");
      }
      retstr->strlength = strlen(retstr->strptr);
      return VALID_ROUTINE;            /* good completion            */
  } else
  if ((!stricmp(args[0].strptr, "PTIME")) || (!stricmp(args[0].strptr, "TTIME")))
  {
      FILETIME createT, kernelT, userT, dummy;
      SYSTEMTIME createST, kernelST, userST;
      BOOL ok;

      if ((args[0].strptr[0] == 'T') || (args[0].strptr[0] == 't'))
          ok = GetThreadTimes(GetCurrentThread(), &createT,&dummy,&kernelT, &userT);
      else
          ok = GetProcessTimes(GetCurrentProcess(), &createT,&dummy,&kernelT, &userT);

      if (ok)
      {
          FileTimeToLocalFileTime(&createT, &createT);
          FileTimeToSystemTime(&createT, &createST);
          FileTimeToSystemTime(&kernelT, &kernelST);
          FileTimeToSystemTime(&userT, &userST);

          wsprintf(retstr->strptr,"Create: %4.4d/%2.2d/%2.2d %d:%2.2d:%2.2d:%3.3d  "\
              "Kernel: %d:%2.2d:%2.2d:%3.3d  User: %d:%2.2d:%2.2d:%3.3d",
              createST.wYear,createST.wMonth,createST.wDay,createST.wHour,createST.wMinute,
              createST.wSecond,createST.wMilliseconds,
              kernelST.wHour,kernelST.wMinute,kernelST.wSecond,kernelST.wMilliseconds,
              userST.wHour,userST.wMinute,userST.wSecond,userST.wMilliseconds);

          retstr->strlength = strlen(retstr->strptr);
          return VALID_ROUTINE;        /* good completion            */
      }
  }
  return INVALID_ROUTINE;              /* good completion            */
}

/**********************************************************************
* Function:  SysShutDownSystem                                        *
*                                                                     *
* Syntax:    call SysShutDownSystem(<computer>,<message>,<timeout>,<appclose>,<reboot> *
*                                                                     *
* Params:    <computer> - name of the remote machine ('' = local)     *
*            <message>  - message for dialog                          *
*            <timeout>  - time to display message                     *
*            <appclose> - no dialog "save unsaved data"               *
*            <reboot>   - reboot the system                           *
*                                                                     *
* Return:    success (1) or failure (0) string                        *
**********************************************************************/

LONG APIENTRY SysShutDownSystem(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  UCHAR * machine = NULL;
  UCHAR * message = NULL;
  ULONG  timeout= 0;
  LONG  rc = 0;
  BOOL forceClose = FALSE;
  BOOL reboot = FALSE;

  if (numargs>5)                       /* arguments specified?       */
    return INVALID_ROUTINE;            /* raise the error            */

  if ((numargs>=1) && (strlen(args[0].strptr) > 0)) machine = args[0].strptr;
  if ((numargs>=2) && (strlen(args[1].strptr) > 0)) message = args[1].strptr;
  if (numargs>=3) if (!string2ulong(args[2].strptr, &timeout))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (numargs>=4) if (!string2ulong(args[3].strptr, &forceClose))
    return INVALID_ROUTINE;            /* raise error if bad         */
  if (numargs>=5) if (!string2ulong(args[4].strptr, &reboot))
    return INVALID_ROUTINE;            /* raise error if bad         */

/* Display the shutdown dialog box and start the time-out countdown. */

  if (!InitiateSystemShutdown(
     machine,            /* address of name of computer to shut down */
     message,         /* address of message to display in dialog box */
     timeout,                          /* time to display dialog box */
     forceClose,     /* force applications with unsaved changes flag */
     reboot                            /*                reboot flag */
  )) RETVAL(GetLastError())
  else
     RETVAL(0)
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

LONG APIENTRY SysSwitchSession(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  HWND hwnd;

  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */

  hwnd = FindWindow(NULL, args[0].strptr);

  if (hwnd)
  {
     if (!SetForegroundWindow(hwnd))
        RETVAL(GetLastError())
     else
        RETVAL(0)
  }
  else
     RETVAL(1)
  return VALID_ROUTINE;                /* good completion            */
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

LONG APIENTRY SysWaitNamedPipe(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  LONG        timeout;                 /* timeout value              */

  if (numargs < 1 ||                   /* wrong number of arguments? */
      numargs > 2 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;            /* raise error condition      */
  timeout = NMPWAIT_USE_DEFAULT_WAIT;

  if (numargs == 2) {                  /* have a timeout value?      */
    if (!string2long(args[1].strptr, &timeout) ||
        (timeout < 0 && timeout != -1))
      return INVALID_ROUTINE;          /* raise error condition      */
  }
  if (timeout == 0) timeout = NMPWAIT_USE_DEFAULT_WAIT;
  else if (timeout == -1) timeout = NMPWAIT_WAIT_FOREVER;

  if (WaitNamedPipe(args[0].strptr, timeout))
      RETVAL(0)
  else
      RETVAL(GetLastError())
  return VALID_ROUTINE;
}

/*********************************************************************/
/* Function FormatFloat:  Common routine to format a floating point  */
/* result for the math functions                                     */
/*********************************************************************/
void FormatResult(
  double    result,                    /* formatted result           */
  ULONG     precision,                 /* required precision         */
  PRXSTRING retstr )                   /* return string              */
{
  if (result == 0)                     /* zero result?               */
    strcpy(retstr->strptr, "0");       /* make exactly 0             */
  else
                                       /* format the result          */
    _gcvt(result, precision, retstr->strptr);
                                       /* set the length             */
  retstr->strlength = strlen(retstr->strptr);
                                       /* end in a period?           */
  if (retstr->strptr[retstr->strlength - 1] == '.')
    retstr->strlength--;               /* remove the period          */
}

/*********************************************************************/
/* Function ValidateMath: Common validation routine for math         */
/* that are of the form fn(number, <precision>)                      */
/*********************************************************************/
LONG  ValidateMath(
  LONG      numargs,                   /* Number of arguments.       */
  RXSTRING  args[],                    /* Function arguments.        */
  double   *x,                         /* input number               */
  PULONG    precision )                /* returned precision         */
{
  LONG      rc;                        /* validation code            */

  rc = VALID_ROUTINE;                  /* set default completion     */
  *precision = DEFAULT_PRECISION;      /* set max digits count       */

  if (numargs < 1 ||                   /* no arguments               */
      numargs > 2 ||
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 2 &&             /* have a precision           */
      !string2ulong(args[1].strptr, precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", x) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* truncate to maximum        */
  *precision = min(*precision, MAX_PRECISION);
  return rc;                           /* return success code        */
}

/*********************************************************************/
/* Function ValidateTrig: Common validation routine for math         */
/* that are of the form fn(number, <precision>, <unit>)              */
/*********************************************************************/
LONG  ValidateTrig(
  LONG      numargs,                   /* Number of arguments.       */
  RXSTRING  args[],                    /* Function arguments.        */
  PRXSTRING retstr,                    /* return string              */
  INT       function )                 /* function to perform        */
{
  LONG      rc;                        /* validation code            */
  INT       units;                     /* angle type                 */
  double    angle;                     /* working angle              */
  double    nsi;                       /* convertion factor          */
  double    nco;                       /* convertion factor          */
  ULONG     precision;                 /* returned precision         */
  double    result;                    /* result                     */

  rc = VALID_ROUTINE;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */
  units = DEGREES;                     /* default angle is degrees   */
  nsi = 1.;                            /* set default conversion     */
  nco = 1.;                            /* set default conversion     */

  if (numargs < 1 ||                   /* no arguments               */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs >= 2 &&             /* have a precision           */
      RXVALIDSTRING(args[1]) &&        /* and it is real string      */
      !string2ulong(args[1].strptr, &precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", &angle) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 3) {             /* have an option             */
    if (RXZEROLENSTRING(args[2]))      /* null string?               */
      rc = INVALID_ROUTINE;            /* this is invalid            */
    else {                             /* process the options        */
                                       /* get the option character   */
      units = toupper(args[2].strptr[0]);
                                       /* was it a good option?      */
      if (units != DEGREES && units != RADIANS && units != GRADES)
        rc = INVALID_ROUTINE;          /* bad option is error        */
    }
  }
  if (!rc) {                           /* everything went well?      */
                                       /* truncate to maximum        */
    precision = min(precision, MAX_PRECISION);
    if (units == DEGREES) {            /* need to convert degrees    */
      nsi = (angle < 0.) ? -1. : 1.;   /* get the direction          */
      angle = fmod(fabs(angle), 360.); /* make modulo 360            */
      if (angle <= 45.)                /* less than 45?              */
        angle = angle * pi / 180.;
      else if (angle < 135.) {         /* over on the other side?    */
        angle = (90. - angle) * pi / 180.;
        function = MAXTRIG - function; /* change the function        */
        nco = nsi;                     /* swap around the conversions*/
        nsi = 1.;
      }
      else if (angle <= 225.) {        /* around the other way?      */
        angle = (180. - angle) * pi / 180.;
        nco = -1.;
      }
      else if (angle < 315.) {         /* close to the origin?       */
        angle = (angle - 270.) * pi / 180.;
        function = MAXTRIG - function; /* change the function        */
        nco = -nsi;
        nsi = 1.;
      }
      else
        angle = (angle - 360.) * pi / 180.;
    }
    else if (units == GRADES) {        /* need to convert degrees    */
      nsi = (angle < 0.) ? -1. : 1.;   /* get the direction          */
      angle = fmod(fabs(angle), 400.); /* make modulo 400            */
      if (angle <= 50.)
        angle = angle * pi / 200.;
      else if (angle < 150.) {
        angle = (100. - angle) * pi / 200.;
        function = MAXTRIG - function; /* change the function        */
        nco = nsi;                     /* swap the conversions       */
        nsi = 1.;
      }
      else if (angle <= 250.) {
        angle = (200. - angle) * pi / 200.;
        nco = -1.;
      }
      else if (angle < 350.) {
        angle = (angle - 300.) * pi / 200.;
        function = MAXTRIG - function; /* change the function        */
        nco = -nsi;
        nsi = 1.;
      }
      else
        angle = (angle - 400.) * pi / 200.;
    }
    switch (function) {                /* process the function       */
      case SINE:                       /* Sine function              */
        result = nsi * sin(angle);
        break;
      case COSINE:                     /* Cosine function            */
        result = nco * cos(angle);
        break;
      case TANGENT:                    /* Tangent function           */
        result = nsi * nco * tan(angle);
        break;
      case COTANGENT:                  /* cotangent function         */
                                       /* overflow?                  */
        if ((result = tan(angle)) == 0.0)
          rc = 40;                     /* this is an error           */
        else
          result = nsi * nco / result; /* real result                */
        break;
    }
    if (!rc)                           /* good result?               */
                                       /* format the result          */
      FormatResult(result, precision, retstr);
  }
  return rc;                           /* return success code        */
}

/*********************************************************************/
/* Function ValidateATrig: Common validation routine for math        */
/* that are of the form fn(number, <precision>, <units>)             */
/*********************************************************************/
LONG  ValidateArcTrig(
  LONG       numargs,                  /* Number of arguments.       */
  RXSTRING   args[],                   /* Function arguments.        */
  PRXSTRING  retstr,                   /* return string              */
  INT        function )                /* function to perform        */
{
  LONG      rc;                        /* validation code            */
  INT       units;                     /* angle type                 */
  double    angle;                     /* working angle              */
  double    nsi;                       /* convertion factor          */
  double    nco;                       /* convertion factor          */
  ULONG     precision;                 /* returned precision         */
  double    x;                         /* input number               */

  rc = VALID_ROUTINE;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */
  units = DEGREES;                     /* default angle is degrees   */
  nsi = 1.;                            /* set default conversion     */
  nco = 1.;                            /* set default conversion     */

  if (numargs < 1 ||                   /* no arguments               */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]))         /* first is omitted           */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs >= 2 &&             /* have a precision           */
      RXVALIDSTRING(args[1]) &&        /* and it is real string      */
      !string2ulong(args[1].strptr, &precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", &x) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 3) {             /* have an option             */
    if (RXZEROLENSTRING(args[2]))      /* null string?               */
      rc = INVALID_ROUTINE;            /* this is invalid            */
    else {                             /* process the options        */
                                       /* get the option character   */
      units = toupper(args[2].strptr[0]);
                                       /* was it a good option?      */
      if (units != DEGREES && units != RADIANS && units != GRADES)
        rc = INVALID_ROUTINE;          /* bad option is error        */
    }
  }
  if (!rc) {                           /* everything went well?      */
                                       /* truncate to maximum        */
    precision = min(precision, MAX_PRECISION);
    switch (function) {                /* process the function       */
      case ARCSINE:                    /* ArcSine function           */
        angle = asin(x);
        break;
      case ARCCOSINE:                  /* ArcCosine function         */
        angle = acos(x);
        break;
      case ARCTANGENT:                 /* ArcTangent function        */
        angle = atan(x);
        break;
    }
    if (units == DEGREES)              /* have to convert the result?*/
      angle = angle * 180. / pi;       /* make into degrees          */
    else if (units == GRADES)          /* need it in grades?         */
      angle = angle * 200. / pi;       /* convert to base 400        */
                                       /* format the result          */
    FormatResult(angle, precision, retstr);
  }
  return rc;                           /* return success code        */
}

/********************************************************************/
/* Functions:           SysSqrt(), SysExp(), SysLog(), SysLog10,    */
/* Functions:           SysSinH(), SysCosH(), SysTanH()             */
/* Description:         Returns function value of argument.         */
/* Input:               One number.                                 */
/* Output:              Value of the function requested for arg.    */
/*                      Returns 0 if the function executed OK,      */
/*                      40 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to two parameters.                     */
/*   The form of the call is:                                       */
/*   result = func_name(x <, prec> <,angle>)                        */
/*                                                                  */
/********************************************************************/
LONG  APIENTRY SysSqrt(                /* Square root function.      */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* function return code       */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(sqrt(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/
LONG  APIENTRY SysExp(                 /* Exponential function.      */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(exp(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/
LONG  APIENTRY SysLog(                 /* Logarithm function.        */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(log(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/
LONG  APIENTRY SysLog10(               /* Log base 10 function.      */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(log10(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/
LONG  APIENTRY SysSinH(                /* Hyperbolic sine function.  */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(sinh(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/
LONG  APIENTRY SysCosH(                /* Hyperbolic cosine funct.   */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(cosh(x), precision, retstr);
  return rc;                           /* return error code          */
}

/*==================================================================*/
LONG  APIENTRY SysTanH(                /* Hyperbolic tangent funct.  */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation return code     */

                                       /* validate the inputs        */
  rc = ValidateMath(numargs, args, &x, &precision);
  if (!rc)                             /* good function call         */
                                       /* format the result          */
    FormatResult(tanh(x), precision, retstr);
  return rc;                           /* return error code          */
}

/********************************************************************/
/* Functions:           SysPower()                                  */
/* Description:         Returns function value of arguments.        */
/* Input:               Two numbers.                                */
/* Output:              Value of the x to the power y.              */
/*                      Returns 0 if the function executed OK,      */
/*                      -1 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   This routine takes two to three parameters.                    */
/*   The form of the call is:                                       */
/*   result = func_name(x, y <, prec>)                              */
/*                                                                  */
/********************************************************************/
LONG  APIENTRY SysPower(               /* Power function.           */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  double    x;                         /* input number               */
  double    y;                         /* second input number        */
  ULONG     precision;                 /* precision used             */
  LONG      rc;                        /* validation code            */

  rc = VALID_ROUTINE;                  /* set default completion     */
  precision = DEFAULT_PRECISION;       /* set max digits count       */

  if (numargs < 2 ||                   /* no arguments               */
      numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||       /* first is omitted           */
      !RXVALIDSTRING(args[1]))         /* second is omitted          */
    rc = INVALID_ROUTINE;              /* this is invalid            */
  else if (numargs == 3 &&             /* have a precision           */
      !string2ulong(args[2].strptr, &precision))
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert input number       */
  else if (sscanf(args[0].strptr, " %lf", &x) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
                                       /* convert second input       */
  else if (sscanf(args[1].strptr, " %lf", &y) != 1)
    rc = INVALID_ROUTINE;              /* this is invalid            */
  if (!rc) {                           /* good function call         */
                                       /* keep to maximum            */
    precision = min(precision, MAX_PRECISION);
                                       /* format the result          */
    FormatResult(pow(x,y), precision, retstr);
  }
  return rc;                           /* return error code          */
}

/********************************************************************/
/* Functions:           RxSin(), RxCos(), RxTan(), RxCotan()        */
/* Description:         Returns trigonometric angle value.          */
/* Input:               Angle in radian or degree or grade          */
/* Output:              Trigonometric function value for Angle.     */
/*                      Returns 0 if the function executed OK,      */
/*                      -1 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to three parameters.                   */
/*   The form of the call is:                                       */
/*   x = func_name(angle <, prec> <, [R | D | G]>)                  */
/*                                                                  */
/********************************************************************/
LONG  APIENTRY SysSin(                 /* Sine function.             */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, SINE);
}

/*==================================================================*/
LONG  APIENTRY SysCos(                 /* Cosine function.           */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, COSINE);
}

/*==================================================================*/
LONG  APIENTRY SysTan(                 /* Tangent function.          */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, TANGENT);
}

/*==================================================================*/
LONG  APIENTRY SysCotan(               /* Cotangent function.        */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateTrig(numargs, args, retstr, COTANGENT);
}

/********************************************************************/
/* Functions:           SysPi()                                     */
/* Description:         Returns value of pi for given precision     */
/* Input:               Precision.   Default is 9                   */
/* Output:              Value of the pi to given precision          */
/* Notes:                                                           */
/*   This routine takes one parameters.                             */
/*   The form of the call is:                                       */
/*   result = syspi(<precision>)                                    */
/*                                                                  */
/********************************************************************/
LONG  APIENTRY SysPi(                  /* Pi function                */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  ULONG     precision;                 /* required precision         */

  precision = DEFAULT_PRECISION;       /* set default precision      */
  if (numargs > 1 ||                   /* too many arguments?        */
      (numargs == 1 &&                 /* bad precision?             */
      !string2ulong(args[0].strptr, &precision)))
    return INVALID_ROUTINE;            /* bad routine                */
                                       /* keep to maximum            */
  precision = min(precision, MAX_PRECISION);
                                       /* format the result          */
  FormatResult(pi, precision, retstr); /* format the result          */
  return VALID_ROUTINE;                /* good result                */
}

/********************************************************************/
/* Functions:           SysArcSin(), SysArcCos(), SysArcTan()       */
/* Description:         Returns angle from trigonometric value.     */
/* Input:               Angle in radian or degree or grade          */
/* Output:              Angle for matching trigonometric value.     */
/*                      Returns 0 if the function executed OK,      */
/*                      -1 otherwise.  The interpreter will fail    */
/*                      if the function returns a negative result.  */
/* Notes:                                                           */
/*   These routines take one to three parameters.                   */
/*   The form of the call is:                                       */
/*   a = func_name(arg <, prec> <, [R | D | G]>)                    */
/*                                                                  */
/********************************************************************/
LONG  APIENTRY SysArcSin(              /* Arc Sine function.         */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateArcTrig(numargs, args, retstr, ARCSINE);
}

/*==================================================================*/
LONG  APIENTRY SysArcCos(              /* Arc Cosine function.       */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateArcTrig(numargs, args, retstr, ARCCOSINE);
}

/*==================================================================*/
LONG  APIENTRY SysArcTan(              /* Arc Tangent function.      */
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* call common routine        */
  return ValidateArcTrig(numargs, args, retstr, ARCTANGENT);
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

LONG APIENTRY SysDumpVariables(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  LONG      rc;                        /* Ret code                   */
  SHVBLOCK  shvb;
  HANDLE    outFile = NULL;
  BOOL      fCloseFile = FALSE;
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
      fCloseFile = TRUE;

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
        int offset = current - buffer;
        buffer_size *= 2;
        /* if new buffer too small, use the minimal fitting size */
        if (buffer_size - offset < new_size) {
          buffer_size = new_size + offset;
        }
        buffer = realloc(buffer,buffer_size);
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
      GlobalFree(shvb.shvname.strptr);
      GlobalFree(shvb.shvvalue.strptr);

      /* leave loop if this was the last var */
      if (shvb.shvret & RXSHV_LVAR)
        break;
    }
  } while (rc == RXSHV_OK);

  WriteFile(outFile, buffer, current - buffer, &dwBytesWritten, NULL);
  free(buffer);

  if (fCloseFile)
    CloseHandle(outFile);

/*  if (rc != RXSHV_OK)  */
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

LONG APIENTRY SysSetFileDateTime(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  BOOL      fOk = TRUE;
  HANDLE    setFile = NULL;
  FILETIME  sFileTime;
  FILETIME  sLocalFileTime;
  SYSTEMTIME sLocalSysTime;

  /* we expect one to three parameters, if three parameters are      */
  /* specified then the second may be omitted to set only a new time,*/
  /* if only one is specified then the file is set to current time   */
  if ( (numargs < 1) || (numargs > 3) ||
       ((numargs == 2) && !RXVALIDSTRING(args[1])) ||
       ((numargs == 3) && !RXVALIDSTRING(args[2])) )
    return INVALID_ROUTINE;            /* raise error condition      */

  /* open output file for read/write for update */
  setFile = CreateFile(args[0].strptr, GENERIC_WRITE | GENERIC_READ,
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
      if ((numargs >= 2) && RXVALIDSTRING(args[1]))
      {
        /* parse new date */
        if (sscanf(args[1].strptr, "%4hu-%2hu-%2hu", &sLocalSysTime.wYear,
                   &sLocalSysTime.wMonth, &sLocalSysTime.wDay) != 3)
          fOk = FALSE;

        if (sLocalSysTime.wYear < 1800)
          fOk = FALSE;
      }

      if ((numargs == 3) && RXVALIDSTRING(args[2]))
      {
        /* parse new time */
        if (sscanf(args[2].strptr, "%2hu:%2hu:%2hu", &sLocalSysTime.wHour,
                   &sLocalSysTime.wMinute, &sLocalSysTime.wSecond) != 3)
          fOk = FALSE;
      }

      if (numargs == 1)
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
  else
    fOk = FALSE;

  if (fOk)
    RETVAL(0)
  else
    RETVAL(-1)
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

LONG APIENTRY SysGetFileDateTime(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  BOOL      fOk = TRUE;
  HANDLE    setFile = NULL;
  FILETIME  sFileTime;
  FILETIME  sLocalFileTime;
  FILETIME  *psFileCreated = NULL;
  FILETIME  *psFileAccessed = NULL;
  FILETIME  *psFileWritten = NULL;
  SYSTEMTIME sLocalSysTime;

  /* we expect one to three parameters, if three parameters are      */
  /* specified then the second may be omitted to set only a new time,*/
  /* if only one is specified then the file is set to current time   */
  if ( (numargs < 1) || (numargs > 2) ||
       ((numargs == 2) && !RXVALIDSTRING(args[1])) )
    return INVALID_ROUTINE;            /* raise error condition      */

  if (numargs > 1)
  {
    switch (args[1].strptr[0])
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
        return INVALID_ROUTINE;
    }
  }
  else
    psFileWritten = &sFileTime;

  /* open file for read to query time */
  setFile = CreateFile(args[0].strptr, GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH |
                       FILE_FLAG_BACKUP_SEMANTICS, NULL);
  if (setFile && (setFile != INVALID_HANDLE_VALUE))
  {
    fOk = GetFileTime(setFile, psFileCreated, psFileAccessed, psFileWritten);
    fOk &= FileTimeToLocalFileTime(&sFileTime, &sLocalFileTime);
    fOk &= FileTimeToSystemTime(&sLocalFileTime, &sLocalSysTime);
    if (fOk)
    {
      sprintf(retstr->strptr, "%4d-%02d-%02d %02d:%02d:%02d",
              sLocalSysTime.wYear,
              sLocalSysTime.wMonth,
              sLocalSysTime.wDay,
              sLocalSysTime.wHour,
              sLocalSysTime.wMinute,
              sLocalSysTime.wSecond);
      retstr->strlength = strlen(retstr->strptr);
    }

    CloseHandle(setFile);
  }
  else
    fOk = FALSE;

  if (!fOk)
    RETVAL(-1)
  else
    return VALID_ROUTINE;
}

APIRET APIENTRY RexxStemSort(PCHAR stemname, INT order, INT type,
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

LONG APIENTRY SysStemSort(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */

{
    CHAR          stemName[255];
    size_t        first = 1;
    size_t        last = ULONG_MAX;
    size_t        firstCol = 0;
    size_t        lastCol = ULONG_MAX;
    INT           sortType = SORT_CASESENSITIVE;
    INT           sortOrder = SORT_ASCENDING;

    if ( (numargs < 1) || (numargs > 7) || /* validate arguments     */
        !RXVALIDSTRING(args[0]))
      return INVALID_ROUTINE;

    /* remember stem name */
    memset(stemName, 0, sizeof(stemName));
    strcpy(stemName, args[0].strptr);
    if (stemName[args[0].strlength-1] != '.')
      stemName[args[0].strlength] = '.';

    /* check other parameters */
    if ((numargs >= 2) && RXVALIDSTRING(args[1])) /* sort order      */
    {
      switch (args[1].strptr[0])
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
      } /* endswitch */
    } /* endif */

    if ((numargs >= 3) && RXVALIDSTRING(args[2])) /* sort type */
    {
      switch (args[2].strptr[0])
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
      } /* endswitch */
    } /* endif */

    if ((numargs >= 4) && RXVALIDSTRING(args[3])) /* first element to sort */
    {
      if (sscanf(args[3].strptr, "%ld", &first) != 1)
        return INVALID_ROUTINE;
      if (first == 0)
        return INVALID_ROUTINE;
    } /* endif */

    if ((numargs >= 5) && RXVALIDSTRING(args[4])) /* last element to sort */
    {
      if (sscanf(args[4].strptr, "%ld", &last) != 1)
        return INVALID_ROUTINE;
      if (last < first)
        return INVALID_ROUTINE;
    } /* endif */

    if ((numargs >= 6) && RXVALIDSTRING(args[5])) /* first column to sort */
    {
      if (sscanf(args[5].strptr, "%ld", &firstCol) != 1)
        return INVALID_ROUTINE;
      firstCol--;
    } /* endif */

    if ((numargs == 7) && RXVALIDSTRING(args[6])) /* last column to sort */
    {
      if (sscanf(args[6].strptr, "%ld", &lastCol) != 1)
        return INVALID_ROUTINE;
      lastCol--;
      if (lastCol < firstCol)
        return INVALID_ROUTINE;

    } /* endif */

    /* the sorting is done in the interpreter */
    if (!RexxStemSort(stemName, sortOrder, sortType, first, last, firstCol, lastCol)) {
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

LONG APIENTRY SysStemDelete(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */

{
  APIRET        rc;
  CHAR          szStemName[255];
  PSZ           pszStemIdx;
  CHAR          szValue[255];
  SHVBLOCK      shvb;
  ULONG         ulIdx;
  ULONG         ulFirst;
  ULONG         ulItems = 1;
  ULONG         ulCount;
  BOOL          fOk = TRUE;

  if ( (numargs < 2) || (numargs > 3) || /* validate arguments       */
      !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]) ||
      ((numargs == 3) && !RXVALIDSTRING(args[2])) )
    return INVALID_ROUTINE;

  /* remember stem name */
  memset(szStemName, 0, sizeof(szStemName));
  strcpy(szStemName, args[0].strptr);
  if (szStemName[args[0].strlength-1] != '.')
    szStemName[args[0].strlength] = '.';
  pszStemIdx = &(szStemName[strlen(szStemName)]);

  /* get item index to be deleted */
  if (sscanf(args[1].strptr, "%ld", &ulFirst) != 1)
    return INVALID_ROUTINE;

  /* get number of items to delete */
  if (numargs == 3)
  {
    if (sscanf(args[2].strptr, "%ld", &ulItems) != 1)
      return INVALID_ROUTINE;
    if (ulItems == 0)
      return INVALID_ROUTINE;
  } /* endif */

  /* retrieve the number of elements in stem */
  strcpy(pszStemIdx, "0");
  shvb.shvnext = NULL;
  shvb.shvname.strptr = szStemName;
  shvb.shvname.strlength = strlen((const char *)szStemName);
  shvb.shvvalue.strptr = szValue;
  shvb.shvvalue.strlength = sizeof(szValue);
  shvb.shvnamelen = shvb.shvname.strlength;
  shvb.shvvaluelen = shvb.shvvalue.strlength;
  shvb.shvcode = RXSHV_SYFET;
  shvb.shvret = 0;
  if (RexxVariablePool(&shvb) == RXSHV_OK)
  {
    /* index retrieved fine */
    if (sscanf(shvb.shvvalue.strptr, "%ld", &ulCount) != 1)
      return INVALID_ROUTINE;

    /* check wether supplied index and count is within limits */
    if (ulFirst + ulItems - 1 > ulCount)
      return INVALID_ROUTINE;

    /* now copy the remaining indices up front */
    for (ulIdx = ulFirst; ulIdx + ulItems <= ulCount; ulIdx++)
    {
      /* get element to relocate */
      sprintf(pszStemIdx, "%ld", ulIdx + ulItems);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szStemName;
      shvb.shvname.strlength = strlen((const char *)szStemName);
      shvb.shvvalue.strptr = NULL;
      shvb.shvvalue.strlength = 0;
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYFET;
      shvb.shvret = 0;

      if (RexxVariablePool(&shvb) == RXSHV_OK)
      {
        sprintf(pszStemIdx, "%ld", ulIdx);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szStemName;
        shvb.shvname.strlength = strlen((const char *)szStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = FALSE;

        /* free memory allocated by REXX */
        GlobalFree(shvb.shvvalue.strptr);
      }
      else
        fOk = FALSE;

      if (!fOk)
        break;
    } /* endfor */

    if (fOk)
    {
      /* now delete the items at the end */
      for (ulIdx = ulCount - ulItems + 1; ulIdx <= ulCount; ulIdx++)
      {
        sprintf(pszStemIdx, "%ld", ulIdx);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szStemName;
        shvb.shvname.strlength = strlen((const char *)szStemName);
        shvb.shvvalue.strptr = NULL;
        shvb.shvvalue.strlength = 0;
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYDRO;
        shvb.shvret = 0;
        if (RexxVariablePool(&shvb) != RXSHV_OK)
        {
          fOk = FALSE;
          break;
        } /* endif */
      } /* endfor */
    } /* endif */

    if (fOk)
    {
      /* set the new number of items in the stem array */
      strcpy(pszStemIdx, "0");
      sprintf(szValue, "%ld", ulCount - ulItems);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szStemName;
      shvb.shvname.strlength = strlen((const char *)szStemName);
      shvb.shvvalue.strptr = szValue;
      shvb.shvvalue.strlength = strlen(szValue);
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYSET;
      shvb.shvret = 0;
      rc = RexxVariablePool(&shvb);
      if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
        fOk = FALSE;
    } /* endif */
  }
  else
  {
    fOk = FALSE;
  } /* endif */

  if (fOk)
    RETVAL(0)
  else
    RETVAL(-1)
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

LONG APIENTRY SysStemInsert(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */

{
  APIRET        rc;
  CHAR          szStemName[255];
  PSZ           pszStemIdx;
  CHAR          szValue[255];
  SHVBLOCK      shvb;
  ULONG         ulIdx;
  ULONG         ulPosition;
  ULONG         ulCount;
  BOOL          fOk = TRUE;

  if ( (numargs != 3) ||  /* validate arguments       */
      !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]) ||
      RXNULLSTRING(args[2]) )
    return INVALID_ROUTINE;

  /* remember stem name */
  memset(szStemName, 0, sizeof(szStemName));
  strcpy(szStemName, args[0].strptr);
  if (szStemName[args[0].strlength-1] != '.')
    szStemName[args[0].strlength] = '.';
  pszStemIdx = &(szStemName[strlen(szStemName)]);

  /* get new item index */
  if (sscanf(args[1].strptr, "%ld", &ulPosition) != 1)
     return INVALID_ROUTINE;

  /* retrieve the number of elements in stem */
  strcpy(pszStemIdx, "0");
  shvb.shvnext = NULL;
  shvb.shvname.strptr = szStemName;
  shvb.shvname.strlength = strlen((const char *)szStemName);
  shvb.shvvalue.strptr = szValue;
  shvb.shvvalue.strlength = sizeof(szValue);
  shvb.shvnamelen = shvb.shvname.strlength;
  shvb.shvvaluelen = shvb.shvvalue.strlength;
  shvb.shvcode = RXSHV_SYFET;
  shvb.shvret = 0;
  if (RexxVariablePool(&shvb) == RXSHV_OK)
  {
    /* index retrieved fine */
    if (sscanf(shvb.shvvalue.strptr, "%ld", &ulCount) != 1)
      return INVALID_ROUTINE;

    /* check wether new position is within limits */
    if ((ulPosition == 0) || (ulPosition > ulCount + 1))
      return INVALID_ROUTINE;

    /* make room for new item by moving all items to the end */
    for (ulIdx = ulCount; ulIdx >= ulPosition; ulIdx--)
    {
      /* get element to relocate */
      sprintf(pszStemIdx, "%ld", ulIdx);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szStemName;
      shvb.shvname.strlength = strlen((const char *)szStemName);
      shvb.shvvalue.strptr = NULL;
      shvb.shvvalue.strlength = 0;
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYFET;
      shvb.shvret = 0;

      if (RexxVariablePool(&shvb) == RXSHV_OK)
      {
        sprintf(pszStemIdx, "%ld", ulIdx + 1);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szStemName;
        shvb.shvname.strlength = strlen((const char *)szStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = FALSE;

        /* free memory allocated by REXX */
        GlobalFree(shvb.shvvalue.strptr);
      }
      else
        fOk = FALSE;

      if (!fOk)
        break;
    } /* endfor */

    if (fOk)
    {
      /* set the new item value */
      sprintf(pszStemIdx, "%ld", ulPosition);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szStemName;
      shvb.shvname.strlength = strlen((const char *)szStemName);
      shvb.shvvalue.strptr = args[2].strptr;
      shvb.shvvalue.strlength = args[2].strlength;
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYSET;
      shvb.shvret = 0;
      rc = RexxVariablePool(&shvb);
      if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
        fOk = FALSE;
    } /* endif */

    if (fOk)
    {
      /* set the new number of items in the stem array */
      strcpy(pszStemIdx, "0");
      sprintf(szValue, "%ld", ulCount + 1);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szStemName;
      shvb.shvname.strlength = strlen((const char *)szStemName);
      shvb.shvvalue.strptr = szValue;
      shvb.shvvalue.strlength = strlen(szValue);
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYSET;
      shvb.shvret = 0;
      rc = RexxVariablePool(&shvb);
      if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
        fOk = FALSE;
    } /* endif */
  }
  else
  {
    fOk = FALSE;
  } /* endif */

  if (fOk)
    RETVAL(0)
  else
    RETVAL(-1)
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

LONG APIENTRY SysStemCopy(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */

{
  APIRET        rc;
  CHAR          szFromStemName[255];
  CHAR          szToStemName[255];
  PSZ           pszFromStemIdx;
  PSZ           pszToStemIdx;
  CHAR          szValue[255];
  SHVBLOCK      shvb;
  ULONG         ulIdx;
  ULONG         ulToCount;
  ULONG         ulFromCount;
  ULONG         ulFrom = 1;
  ULONG         ulTo = 1;
  ULONG         ulCopyCount = 0;
  BOOL          fInsert = FALSE;
  BOOL          fOk = TRUE;

  if ( (numargs < 2) || (numargs > 6) ||  /* validate arguments      */
      !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[1]) ||
      ((numargs == 6) && !RXVALIDSTRING(args[5])) )
    return INVALID_ROUTINE;

  /* remember stem names */
  memset(szFromStemName, 0, sizeof(szFromStemName));
  strcpy(szFromStemName, args[0].strptr);
  if (szFromStemName[args[0].strlength-1] != '.')
    szFromStemName[args[0].strlength] = '.';
  pszFromStemIdx = &(szFromStemName[strlen(szFromStemName)]);

  memset(szToStemName, 0, sizeof(szToStemName));
  strcpy(szToStemName, args[1].strptr);
  if (szToStemName[args[1].strlength-1] != '.')
    szToStemName[args[1].strlength] = '.';
  pszToStemIdx = &(szToStemName[strlen(szToStemName)]);

  /* get from item index */
  if ((numargs >= 3) && RXVALIDSTRING(args[2]))
    if (sscanf(args[2].strptr, "%ld", &ulFrom) != 1)
      return INVALID_ROUTINE;

  /* get to item index */
  if ((numargs >= 4) && RXVALIDSTRING(args[3]))
    if (sscanf(args[3].strptr, "%ld", &ulTo) != 1)
      return INVALID_ROUTINE;

  /* get copy count */
  if ((numargs >= 5) && RXVALIDSTRING(args[4]))
    if (sscanf(args[4].strptr, "%ld", &ulCopyCount) != 1)
      return INVALID_ROUTINE;

  /* get copy type */
  if (numargs >= 6)
  {
    switch (args[5].strptr[0])
    {
      case 'I':
      case 'i':
        fInsert = TRUE;
        break;
      case 'O':
      case 'o':
        fInsert = FALSE;
        break;
      default:
        return INVALID_ROUTINE;
    } /* endswitch */
  } /* endif */

  /* retrieve the number of elements in stems */
  strcpy(pszFromStemIdx, "0");
  shvb.shvnext = NULL;
  shvb.shvname.strptr = szFromStemName;
  shvb.shvname.strlength = strlen((const char *)szFromStemName);
  shvb.shvvalue.strptr = szValue;
  shvb.shvvalue.strlength = sizeof(szValue);
  shvb.shvnamelen = shvb.shvname.strlength;
  shvb.shvvaluelen = shvb.shvvalue.strlength;
  shvb.shvcode = RXSHV_SYFET;
  shvb.shvret = 0;
  if (RexxVariablePool(&shvb) == RXSHV_OK)
  {
    /* index retrieved fine */
    if (sscanf(shvb.shvvalue.strptr, "%ld", &ulFromCount) != 1)
      return INVALID_ROUTINE;

    if ((ulCopyCount > (ulFromCount - ulFrom + 1)) || (ulFromCount == 0))
      return INVALID_ROUTINE;
  }
  else
    fOk = FALSE;

  if (fOk)
  {
    strcpy(pszToStemIdx, "0");
    shvb.shvnext = NULL;
    shvb.shvname.strptr = szToStemName;
    shvb.shvname.strlength = strlen((const char *)szToStemName);
    shvb.shvvalue.strptr = szValue;
    shvb.shvvalue.strlength = sizeof(szValue);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_SYFET;
    shvb.shvret = 0;
    rc = RexxVariablePool(&shvb);
    if (rc == RXSHV_OK)
    {
      /* index retrieved fine */
      if (sscanf(shvb.shvvalue.strptr, "%ld", &ulToCount) != 1)
        return INVALID_ROUTINE;
    }
    else
    {
      if (rc == RXSHV_NEWV)
      {
        /* tostem.0 is not set, we assume empty target stem */
        ulToCount = 0;
      }
      else
        fOk = FALSE;
    } /* endif */

    if (fOk)
    {
      if (ulTo > ulToCount + 1)
        return INVALID_ROUTINE;
    } /* endif */
  } /* endif */

  /* set copy count to number of items in source stem if not already set */
  if (ulCopyCount == 0)
    ulCopyCount = ulFromCount - ulFrom + 1;

  if (fOk && fInsert)
  {
    /* if we are about to insert the items we have to make room */
    for (ulIdx = ulToCount; ulIdx >= ulTo; ulIdx--)
    {
      /* get element to relocate */
      sprintf(pszToStemIdx, "%ld", ulIdx);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szToStemName;
      shvb.shvname.strlength = strlen((const char *)szToStemName);
      shvb.shvvalue.strptr = NULL;
      shvb.shvvalue.strlength = 0;
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYFET;
      shvb.shvret = 0;

      if (RexxVariablePool(&shvb) == RXSHV_OK)
      {
        sprintf(pszToStemIdx, "%ld", ulIdx + ulCopyCount);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szToStemName;
        shvb.shvname.strlength = strlen((const char *)szToStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = FALSE;

        /* free memory allocated by REXX */
        GlobalFree(shvb.shvvalue.strptr);
      }
      else
        fOk = FALSE;

      if (!fOk)
        break;
    } /* endfor */

    if (fOk)
    {
      /* set the new count for the target stem */
      strcpy(pszToStemIdx, "0");
      ulToCount += ulCopyCount;
      sprintf(szValue, "%ld", ulToCount);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szToStemName;
      shvb.shvname.strlength = strlen((const char *)szToStemName);
      shvb.shvvalue.strptr = szValue;
      shvb.shvvalue.strlength = strlen(szValue);
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYSET;
      shvb.shvret = 0;
      rc = RexxVariablePool(&shvb);
      if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
        fOk = FALSE;
    } /* endif */
  } /* endif */

  if (fOk)
  {
    /* now do the actual copying from the source to target */
    for (ulIdx = 0; ulIdx < ulCopyCount; ulIdx++)
    {
      /* get element to copy */
      sprintf(pszFromStemIdx, "%ld", ulFrom + ulIdx);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szFromStemName;
      shvb.shvname.strlength = strlen((const char *)szFromStemName);
      shvb.shvvalue.strptr = NULL;
      shvb.shvvalue.strlength = 0;
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYFET;
      shvb.shvret = 0;

      if (RexxVariablePool(&shvb) == RXSHV_OK)
      {
        sprintf(pszToStemIdx, "%ld", ulTo + ulIdx);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szToStemName;
        shvb.shvname.strlength = strlen((const char *)szToStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = FALSE;

        /* free memory allocated by REXX */
        GlobalFree(shvb.shvvalue.strptr);
      }
      else
        fOk = FALSE;

      if (!fOk)
        break;
    } /* endfor */
  } /* endif */

  if (fOk && (ulTo + ulCopyCount - 1 > ulToCount))
  {
    /* set the new count for the target stem */
    strcpy(pszToStemIdx, "0");
    ulToCount = ulTo + ulCopyCount - 1;
    sprintf(szValue, "%ld", ulToCount);
    shvb.shvnext = NULL;
    shvb.shvname.strptr = szToStemName;
    shvb.shvname.strlength = strlen((const char *)szToStemName);
    shvb.shvvalue.strptr = szValue;
    shvb.shvvalue.strlength = strlen(szValue);
    shvb.shvnamelen = shvb.shvname.strlength;
    shvb.shvvaluelen = shvb.shvvalue.strlength;
    shvb.shvcode = RXSHV_SYSET;
    shvb.shvret = 0;
    rc = RexxVariablePool(&shvb);
    if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
      fOk = FALSE;
  } /* endif */

  if (fOk)
    RETVAL(0)
  else
    RETVAL(-1)
}


/*************************************************************************
* Function:  SysUtilVersion                                              *
*                                                                        *
* Syntax:    Say SysUtilVersion                                          *
*                                                                        *
* Return:    REXXUTIL.DLL Version                                        *
*************************************************************************/

LONG APIENTRY SysUtilVersion(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs != 0)                    /* validate arg count         */
    return INVALID_ROUTINE;
                                       /* format into the buffer     */
  sprintf(retstr->strptr, "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SetRexxStem                                                 *
*                                                                        *
*            internal function to set a stem value in the varpool        *
*            for not UNICODE character strings                           *
*                                                                        *
* Return:    FALSE - failed                                              *
*            TRUE  - no error                                            *
*************************************************************************/
LONG SetRexxStem(CHAR * name, char * tailname, CHAR * data)
{
   SHVBLOCK shvb;
   CHAR buffer[MAX];

   sprintf(buffer,"%s%s",name,tailname);

   shvb.shvnext = NULL;
   shvb.shvname.strptr = buffer;
   shvb.shvname.strlength = strlen(buffer);
   shvb.shvnamelen = shvb.shvname.strlength;
   shvb.shvvalue.strptr = data;
   shvb.shvvalue.strlength = strlen(data);
   shvb.shvvaluelen = strlen(data);
   shvb.shvcode = RXSHV_SYSET;
   shvb.shvret = 0;
   if (RexxVariablePool(&shvb) == RXSHV_BADN)
     return FALSE;
   return TRUE;
}

/*************************************************************************
* Function:  SetRexxStemLength                                           *
*                                                                        *
*            internal function to set a stem value in the varpool        *
*            for not UNICODE character strings                           *
*                                                                        *
* Return:    FALSE - failed                                              *
*            TRUE  - no error                                            *
*************************************************************************/
LONG SetRexxStemLength(CHAR * name, char * tailname, CHAR * data, LONG datalen)
{
   SHVBLOCK shvb;
   CHAR buffer[MAX];

   sprintf(buffer,"%s%s",name,tailname);

   shvb.shvnext = NULL;
   shvb.shvname.strptr = buffer;
   shvb.shvname.strlength = strlen(buffer);
   shvb.shvnamelen = shvb.shvname.strlength;
   shvb.shvvalue.strptr = data;
   shvb.shvvalue.strlength = datalen;
   shvb.shvvaluelen = datalen;
   shvb.shvcode = RXSHV_SYSET;
   shvb.shvret = 0;
   if (RexxVariablePool(&shvb) == RXSHV_BADN)
     return FALSE;
   return TRUE;
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
LONG APIENTRY SysFromUniCode(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */

{
  int   iBytesDestination;
  ULONG iBytesNeeded;
  DWORD dwFlags = 0;
  char  *strDefaultChar = NULL;
  BOOL  bUsedDefaultChar = FALSE;
  UINT  len, codePage;
  char* str = NULL;
  char* strptr = NULL;
  CHAR  stemName[MAX];
  CHAR  szUsedDefChar[2];

  /* correct number of arguments ? */
  /* arguments must always be 5. Args 1 and 5 must be valid strings */
  if ( numargs != 5 || !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[4]) )
//  if ( numargs < 2 || numargs > 5 ||
//       !RXVALIDSTRING(args[4]) )
    return INVALID_ROUTINE;

  /* calculate the length of the input string */
  len = wcslen((wchar_t*)(args[0].strptr)) + 1;

                                       /* evaluate codepage          */
  if (args[1].strlength == 0)
    codePage = GetOEMCP();
  else
  {
    if (rxstricmp(args[1].strptr, "THREAD_ACP") == 0)
      codePage = CP_THREAD_ACP;
    else if (rxstricmp(args[1].strptr,"ACP") == 0)
      codePage = CP_ACP;
    else if (rxstricmp(args[1].strptr,"MACCP") == 0)
      codePage = CP_MACCP;
    else if (rxstricmp(args[1].strptr,"OEMCP") == 0)
      codePage = CP_OEMCP;
    else if (rxstricmp(args[1].strptr,"SYMBOL") == 0)
      codePage = CP_SYMBOL;
    else if (rxstricmp(args[1].strptr,"UTF7") == 0)
      codePage = CP_UTF7;
    else if (rxstricmp(args[1].strptr,"UTF8") == 0)
      codePage = CP_UTF8;
    else
      codePage = atoi(args[1].strptr);
  }

                                       /* evaluate the mapping flags */
  if (args[2].strlength != 0)
  {
    /* all flags MUST also specify WC_COMPOSITECHECK */
    if (mystrstr(args[2].strptr,"COMPOSITECHECK",args[2].strlength,14,FALSE)) dwFlags |= WC_COMPOSITECHECK;
    if (mystrstr(args[2].strptr,"SEPCHARS",args[2].strlength,8,FALSE))        dwFlags |= WC_SEPCHARS | WC_COMPOSITECHECK;
    if (mystrstr(args[2].strptr,"DISCARDNS",args[2].strlength,9,FALSE))       dwFlags |= WC_DISCARDNS| WC_COMPOSITECHECK;
    if (mystrstr(args[2].strptr,"DEFAULTCHAR",args[2].strlength,11,FALSE))    dwFlags |= WC_DEFAULTCHAR | WC_COMPOSITECHECK;
    if (dwFlags == 0)
      return INVALID_ROUTINE;
  }

                                       /* evaluate default charcter  */
  if (args[3].strlength != 0 && (dwFlags & WC_DEFAULTCHAR) == WC_DEFAULTCHAR)
  {
    strDefaultChar = args[3].strptr;
  }
  else
  {
    /* use our own default character rather than relying on the windows default */
    strDefaultChar = "?";
  }

                                       /* evaluate output stem       */
  strcpy(stemName, args[4].strptr);

                                       /* uppercase the name         */
  memupper(stemName, args[4].strlength);

                              /* get and correct the output stem name*/
  if (stemName[args[4].strlength-1] != '.')
  {
    stemName[args[4].strlength] = '.';
    stemName[args[4].strlength+1] = '\0';
  }

  /* Allocate space for the string, to allow double zero byte termination */
  if (!(strptr = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, args[0].strlength + 4)))
    return INVALID_ROUTINE;
  memcpy ( (void*)strptr, (void*)args[0].strptr, (size_t)args[0].strlength ) ;

  /* Query the number of bytes required to store the Dest string */
  iBytesNeeded = WideCharToMultiByte( codePage,
                                      dwFlags,
                                      (LPWSTR) strptr,     // (LPWSTR)args[0].strptr,
                                      args[0].strlength/2, // len,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL);

  if (iBytesNeeded == 0) RETVAL(GetLastError())  // call to function fails

  // hard error, stop
  if (!(str = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, iBytesNeeded + 4)))
  {
    GlobalFree(strptr);          // free allocated string
    return INVALID_ROUTINE;
  }

  /* Do the conversion */
  /* in case of UTF8, the documentation says: When CP_UTF8 is set, dwFlags must be zero  */
  /* and both lpDefaultChar and lpUsedDefaultChar must be NULL.                          */
  /* MHES 1 Mar 2006 - I find no documentation that says this */
#if 0
  if ( !bUsedDefaultChar )
  {
     iBytesDestination = WideCharToMultiByte(codePage,               //codepage
                                          dwFlags,                //conversion flags
                                          (LPWSTR) strptr,        //source string
                                          args[0].strlength/2,    //source string length
                                          str,                    //target string
                                          iBytesNeeded,           //size of target buffer
                                          NULL,
                                          NULL);
  }
  else
#endif
  {
	/* Do the conversion */
  iBytesDestination = WideCharToMultiByte(codePage,               //codepage
                                          dwFlags,                //conversion flags
                                          (LPWSTR) strptr,        // (LPWSTR)args[0].strptr,  //source string
                                          args[0].strlength/2,    // len,                     //source string length
                                          str,                    //target string
                                          iBytesNeeded,           //size of target buffer
                                          strDefaultChar,
                                          &bUsedDefaultChar);

  }
  if (iBytesDestination == 0) // call to function fails
  {
     GlobalFree(str);          //  free allocated string
     GlobalFree(strptr);          // free allocated string
     RETVAL(GetLastError())    // return error from function call
  }

  // convert the default character flag to an character
//  itoa(chDefaultChar,szUsedDefChar,10);
//  szUsedDefChar[1] ='\0';

  // set the default character flag in the output stem
  if (bUsedDefaultChar && (dwFlags & WC_DEFAULTCHAR) == WC_DEFAULTCHAR)
  {
     if (!SetRexxStem(stemName, "!USEDDEFAULTCHAR", strDefaultChar))
        return INVALID_ROUTINE;
  }
  else
  {
     if (!SetRexxStem(stemName, "!USEDDEFAULTCHAR", ""))
        return INVALID_ROUTINE;
  }

  // set the output data character flag in the output stem
  if (!SetRexxStemLength(stemName, "!TEXT", str, iBytesNeeded))
  {
     GlobalFree(strptr);          // free allocated string
    return INVALID_ROUTINE;
  }
  GlobalFree(strptr);          // free allocated string

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* set default result         */
  return VALID_ROUTINE;                /* no error on call           */

}
/*************************************************************************
* Function:  SetRexxUIStem                                               *
*                                                                        *
*            internal function to set a stem value in the varpool        *
*            for UNICODE character strings                               *
*                                                                        *
* Return:    FALSE - failed                                              *
*            TRUE  - no error                                            *
*************************************************************************/

LONG SetRexxUIStem(CHAR * name, char * tailname, LPWSTR data, int datalen)
{
   SHVBLOCK shvb;
   CHAR buffer[MAX];

   sprintf(buffer,"%s%s",name,tailname);

   shvb.shvnext = NULL;
   shvb.shvname.strptr = buffer;
   shvb.shvname.strlength = strlen(buffer);
   shvb.shvnamelen = shvb.shvname.strlength;
   shvb.shvvalue.strptr = (char*)data;
   shvb.shvvalue.strlength = datalen;
   shvb.shvvaluelen = datalen;
   shvb.shvcode = RXSHV_SYSET;
   shvb.shvret = 0;
   if (RexxVariablePool(&shvb) == RXSHV_BADN)
     return FALSE;
   return TRUE;
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

    PRECOMPOSED       Always use precomposed characters—that is, characters
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
LONG APIENTRY SysToUniCode(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */

{
  ULONG  ulDataLen, ulWCharsNeeded;
  DWORD  dwFlags = 0;
  UINT   codePage;
  LPWSTR lpwstr = NULL;
  CHAR   stemName[MAX];

  // check number of arguments
  // arguments must always be 4. Args 1 and 4 must be valid strings
  if ( numargs != 4 || !RXVALIDSTRING(args[0]) || !RXVALIDSTRING(args[3]) )
//  if ( numargs < 2 || numargs > 4 ||
//       !RXVALIDSTRING(args[3]) )
    return INVALID_ROUTINE;            /* Invalid call to routine    */

  // evaluate codepage
  if (args[1].strlength == 0)
    codePage = GetOEMCP();
  else
  {
    if (rxstricmp(args[1].strptr,"THREAD_ACP") == 0)
      codePage = CP_THREAD_ACP;
    else if (rxstricmp(args[1].strptr,"ACP") == 0)
      codePage = CP_ACP;
    else if (rxstricmp(args[1].strptr,"MACCP") == 0)
      codePage = CP_MACCP;
    else if (rxstricmp(args[1].strptr,"OEMCP") == 0)
      codePage = CP_OEMCP;
    else if (rxstricmp(args[1].strptr,"SYMBOL") == 0)
      codePage = CP_SYMBOL;
    else if (rxstricmp(args[1].strptr,"UTF7") == 0)
      codePage = CP_UTF7;
    else if (rxstricmp(args[1].strptr,"UTF8") == 0)
      codePage = CP_UTF8;
    else
      codePage = atoi(args[1].strptr);
  }

  // evaluate the mapping flags
  if (args[2].strlength != 0)
  {
    if (mystrstr(args[2].strptr,"PRECOMPOSED",args[2].strlength,11,FALSE))   dwFlags |= MB_PRECOMPOSED;
    if (mystrstr(args[2].strptr,"COMPOSITE",args[2].strlength,9,FALSE))     dwFlags  |= MB_COMPOSITE;
    if (mystrstr(args[2].strptr,"ERR_INVALID",args[2].strlength,11,FALSE))   dwFlags |= MB_ERR_INVALID_CHARS;
    if (mystrstr(args[2].strptr,"USEGLYPHCHARS",args[2].strlength,13,FALSE)) dwFlags |= MB_USEGLYPHCHARS;
    if (dwFlags == 0)
      return INVALID_ROUTINE;
  }

  // evaluate output stem
  strcpy(stemName, args[3].strptr);

  /* uppercase the name         */
  memupper(stemName, args[3].strlength);

  if (stemName[args[3].strlength-1] != '.')
  {
    stemName[args[3].strlength] = '.';
    stemName[args[3].strlength+1] = '\0';
  }

  /* Query the number of bytes required to store the Dest string */
  ulWCharsNeeded = MultiByteToWideChar( codePage,
                                        dwFlags,
                                        args[0].strptr,
                                        args[0].strlength,
                                        NULL,
                                        NULL);

  if (ulWCharsNeeded == 0) RETVAL(GetLastError())  // call to function fails

  ulDataLen = (ulWCharsNeeded)*2;

  // hard error, stop
  if (!(lpwstr = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, ulDataLen+4)))
    return INVALID_ROUTINE;


  /* Do the conversion */
  ulWCharsNeeded = MultiByteToWideChar( codePage,
                                        dwFlags,
                                        args[0].strptr,
                                        args[0].strlength,
                                        lpwstr,
                                        ulWCharsNeeded);

  if (ulWCharsNeeded == 0) // call to function fails
  {
     GlobalFree(lpwstr);       // free allocated string
     RETVAL(GetLastError())    // return error from function call
  }

  if (!SetRexxUIStem(stemName, "!TEXT", lpwstr, ulDataLen))
    return INVALID_ROUTINE;

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* set default result         */
  return VALID_ROUTINE;                /* no error on call           */

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

LONG APIENTRY SysWinGetPrinters(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD realSize = 0;
  DWORD entries = 0;
  BOOL  fSuccess = FALSE;
  char  szBuffer[256];
  PRINTER_INFO_2 *pResult;
  DWORD currentSize = 10*sizeof(PRINTER_INFO_2)*sizeof(char);
  char *pArray = (char*) malloc(sizeof(char)*currentSize);
  SHVBLOCK shvb;
  APIRET rc;

  if (numargs != 1)                    /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;

  // must be a stem!
  if (args[0].strptr[args[0].strlength-1] != '.')
    return INVALID_ROUTINE;

  while (fSuccess == FALSE) {
    fSuccess = EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS, NULL, 2, pArray, currentSize, &realSize, &entries);
    if (currentSize < realSize) {
      currentSize = realSize;
      realSize = 0;
      pArray = (char*) realloc(pArray, sizeof(char)*currentSize);
      fSuccess = FALSE;
    } else
      fSuccess = TRUE;
  }
  pResult = (PRINTER_INFO_2*) pArray;

  // set number of entries to stem.0
  sprintf(szBuffer,"%d",entries);
  sprintf(args[0].strptr+args[0].strlength,"0");
  shvb.shvnext = NULL;
  shvb.shvname.strptr = args[0].strptr;
  shvb.shvname.strlength = strlen(args[0].strptr);
  shvb.shvvalue.strptr = szBuffer;
  shvb.shvvalue.strlength = strlen(szBuffer);
  shvb.shvnamelen = shvb.shvname.strlength;
  shvb.shvvaluelen = shvb.shvvalue.strlength;
  shvb.shvcode = RXSHV_SYSET;
  shvb.shvret = 0;

  fSuccess = FALSE;

  rc = RexxVariablePool(&shvb);
  if (rc == RXSHV_OK || rc == RXSHV_NEWV) {
    fSuccess = TRUE;
    while (entries--) {
      sprintf(szBuffer,"%s,%s,%s",pResult[entries].pPrinterName,pResult[entries].pDriverName,pResult[entries].pPortName);
      sprintf(args[0].strptr+args[0].strlength,"%d",entries+1);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = args[0].strptr;
      shvb.shvname.strlength = strlen(args[0].strptr);
      shvb.shvvalue.strptr = szBuffer;
      shvb.shvvalue.strlength = strlen(szBuffer);
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYSET;
      shvb.shvret = 0;
      rc = RexxVariablePool(&shvb);
      if (rc != RXSHV_OK && rc != RXSHV_NEWV) {
        fSuccess = FALSE;
        break;
      }
    }
  }
  free(pArray);

  sprintf(retstr->strptr,"%s",fSuccess==TRUE?"0":"1");
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
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

LONG APIENTRY SysWinGetDefaultPrinter(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  DWORD  errnum = 0;
  OSVERSIONINFO osv;

  if (numargs != 0)                    /* If args, then its an       */
                                       /* incorrect call             */
    return INVALID_ROUTINE;

  // What version of Windows are you running?
  osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osv);

  // If Windows 95 or 98, use EnumPrinters...
  if (osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
    DWORD dwNeeded = 0;
    DWORD dwReturned;
    PRINTER_INFO_2 *printerInfo = NULL;

    // find out how much memory is needed
    EnumPrinters(PRINTER_ENUM_DEFAULT, NULL, 2, NULL, 0, &dwNeeded, &dwReturned);
    if (dwNeeded == 0)
      return INVALID_ROUTINE;

    printerInfo = (PRINTER_INFO_2*) malloc(sizeof(char)*dwNeeded);
    if (!printerInfo)
      return INVALID_ROUTINE;

    // fill in info
    if (! EnumPrinters(PRINTER_ENUM_DEFAULT, NULL, 2, (LPBYTE) printerInfo, dwNeeded, &dwNeeded, &dwReturned) ) {
      free(printerInfo);
      return INVALID_ROUTINE;
    }

    lstrcpy(retstr->strptr,printerInfo->pPrinterName);

    free(printerInfo);

  } else
    // NT / W2K:
    GetProfileString("Windows", "DEVICE", ",,,", retstr->strptr, retstr->strlength);

  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SysWinSetDefaultPrinter                                     *
*                                                                        *
* Syntax:    call SysWinSetDefaultPrinter printer                        *
*                                                                        *
* Params:    string describing default printer                           *
*                                                                        *
* Return:    error number                                                *
*************************************************************************/

LONG APIENTRY SysWinSetDefaultPrinter(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  DWORD  errnum = 0;
  UINT   count = 0;
  OSVERSIONINFO osv;

  if (numargs != 1)                    /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;

  /* just make sure the input string has valid format:
     it has to contain at least two commas */
  for (count = 0; count < args[0].strlength; count++) {
    if (args[0].strptr[count] == ',')
      errnum++;
  }

  if (errnum < 2)
    return INVALID_ROUTINE;

  // What version of Windows are you running?
  osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osv);

  // If Windows 95 or 98, use EnumPrinters...
  if (osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
    BOOL   bFlag;
    HANDLE hPrinter = NULL;
    char  *pPrinterName = args[0].strptr;
    PRINTER_INFO_2 *printerInfo = NULL;
    DWORD  dwNeeded = 0;

    for (count = 0; count < args[0].strlength; count++)
      if (pPrinterName[count] == ',') {
        pPrinterName[count] = 0x00; // we only need the name
        break;
      }

    // Open this printer so you can get information about it...
    bFlag = OpenPrinter(pPrinterName, &hPrinter, NULL);
    if (!bFlag || !hPrinter)
      return INVALID_ROUTINE;

    // The first GetPrinter() tells you how big our buffer should
    // be in order to hold ALL of PRINTER_INFO_2. Note that this will
    // usually return FALSE. This only means that the buffer (the 3rd
    // parameter) was not filled in. You don't want it filled in here...
    GetPrinter(hPrinter, 2, 0, 0, &dwNeeded);
    if (dwNeeded == 0)
    {
      ClosePrinter(hPrinter);
      return INVALID_ROUTINE;
    }

    // Allocate enough space for PRINTER_INFO_2...
    printerInfo = (PRINTER_INFO_2*) malloc(sizeof(char)*dwNeeded);
    if (!printerInfo)
    {
      ClosePrinter(hPrinter);
      return INVALID_ROUTINE;
    }

    // The second GetPrinter() will fill in all the current information
    // so that all you need to do is modify what you're interested in...
    bFlag = GetPrinter(hPrinter, 2, (LPBYTE) printerInfo, dwNeeded, &dwNeeded);
    if (!bFlag)
    {
      ClosePrinter(hPrinter);
      free(printerInfo);
      return INVALID_ROUTINE;
    }

    // Set default printer attribute for this printer...
    printerInfo->Attributes |= PRINTER_ATTRIBUTE_DEFAULT;
    bFlag = SetPrinter(hPrinter, 2, (LPBYTE) printerInfo, 0);
    if (bFlag)
      SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "windows");

    ClosePrinter(hPrinter);
    free(printerInfo);
  }
  else {
    // NT / W2K
    WriteProfileString("Windows", "DEVICE", args[0].strptr);
    SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "windows");
  }

  errnum = GetLastError();
  sprintf(retstr->strptr,"%d",errnum);
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
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

LONG APIENTRY SysFileCopy(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

                                       /* we need two valid arguments */
  if ( numargs != 2 || !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;            /* raise an error             */

                                       /* copy the file              */
  if (!CopyFile(args[0].strptr, args[1].strptr, 0))
      RETVAL(GetLastError())           /* pass back return code      */
  else
      RETVAL(0)
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

LONG APIENTRY SysFileMove(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
                                       /* we need two valid arguments */
  if ( numargs != 2 || !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;            /* raise an error             */

                                       /* move the file              */
  if (!MoveFile(args[0].strptr, args[1].strptr))
      RETVAL(GetLastError())           /* pass back return code      */
  else
      RETVAL(0)
}

/*************************************************************************
* Function:  SysIsFile                                                *
*                                                                        *
* Syntax:    call SysIsFile file                                      *
*                                                                        *
* Params:    file - file to check existance of.                          *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

LONG APIENTRY SysIsFile(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) ||
      (dwAttrs & FILE_ATTRIBUTE_REPARSE_POINT))
      RETVAL(0)                        /* False - Is something else  */
  else
      RETVAL(1)                        /* True - Is a File           */
}

/*************************************************************************
* Function:  SysIsFileDirectory                                                 *
*                                                                        *
* Syntax:    call SysIsFileDirectory dir                                        *
*                                                                        *
* Params:    dir - dir to check existance of.                            *
*                                                                        *
* Return:    Logical.                                                    *
*************************************************************************/

LONG APIENTRY SysIsFileDirectory(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
      RETVAL(1)                        /* True - Is a Directory      */
  else
      RETVAL(0)                        /* False - Is a File          */
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

LONG APIENTRY SysIsFileLink(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_REPARSE_POINT)
      RETVAL(1)                        /* True - Is a link           */
  else
      RETVAL(0)                        /* False - Is not a link      */
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

LONG APIENTRY SysIsFileCompressed(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_COMPRESSED)
      RETVAL(1)                        /* True - Is a compressed     */
  else
      RETVAL(0)                        /* False - Is not a compressed*/
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

LONG APIENTRY SysIsFileEncrypted(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_ENCRYPTED)
      RETVAL(1)                        /* True - Is a encrypted      */
  else
      RETVAL(0)                        /* False - Is not a encrypted */
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

LONG APIENTRY SysIsFileNotContentIndexed(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
      RETVAL(1)                        /* True - Is not to be indexed*/
  else
      RETVAL(0)                        /* False - Is to be indexed   */
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

LONG APIENTRY SysIsFileOffline(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_OFFLINE)
      RETVAL(1)                        /* True - Is offline          */
  else
      RETVAL(0)                        /* False - Is a File          */
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

LONG APIENTRY SysIsFileSparse(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_SPARSE_FILE)
      RETVAL(1)                        /* True - Is a sprase file    */
  else
      RETVAL(0)                        /* False - Is not sparse file */
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

LONG APIENTRY SysIsFileTemporary(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  DWORD dwAttrs;

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  dwAttrs = GetFileAttributes(args[0].strptr);
// INVALID_FILE_ATTRIBUTES is not defined in Visual Studio 6 SP5
//  if (dwAttrs==INVALID_FILE_ATTRIBUTES)
  if (dwAttrs==0xffffffff)
      RETVAL(0)                        /* Unable to check attributes */

  if (dwAttrs & FILE_ATTRIBUTE_TEMPORARY)
      RETVAL(1)                        /* True - Is temporary        */
  else
      RETVAL(0)                        /* False - Is not temporary   */
}

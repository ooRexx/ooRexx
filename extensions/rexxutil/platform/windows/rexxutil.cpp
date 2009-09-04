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

#include "oorexxapi.h"
#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <limits.h>

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
  char *       buffer;                 /* file read buffer           */
  size_t       size;                   /* file size                  */
  size_t       data;                   /* data left in buffer        */
  size_t       residual;               /* size left to read          */
  char *       scan;                   /* current scan position      */
  HANDLE       handle;                 /* file handle                */
} GetFileData;

/*********************************************************************/
/* RxTree Structure used by SysTree.                                 */
/*********************************************************************/

typedef struct RxTreeData {
    size_t count;                      /* Number of lines processed  */
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    size_t stemlen;                    /* Length of stem             */
    size_t vlen;                       /* Length of variable value   */
    char TargetSpec[MAX_PATH+1];     /* Target filespec            */
    char truefile[MAX_PATH+1];       /* expanded file name         */
    char Temp[MAX_PATH+80];          /* buffer for returned values */
    char varname[MAX];                 /* Buffer for variable name   */
    size_t nattrib;                    /* New attrib, diff for each  */
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

    const char *result = (haystackCopy, needle);
    free(haystackCopy);
    return result;
}

/*****************************************************************
* Function:  getpath(string, path, filename)                     *
*                                                                *
* Purpose:  This function gets the PATH and FILENAME of the file *
*           target contained in STRING.  The path will end with  *
*           the '\' char if a path is supplied.                  *
*                                                                *
*****************************************************************/

void getpath(
  char *string,
  char *path,
  char *filename)
{
  size_t len;                          /* length of filespec         */
  int    LastSlashPos;                 /* position of last slash     */
  char   szBuff[MAX_PATH];             /* used to save current dir   */
  char   drv[3];                       /* used to change dir         */
  size_t i=0;

  while (string[i] == ' ')
  {
      i++;        /* skip leading blanks        */
  }
  if (i > 0)
  {
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
  LastSlashPos = (int)len;             /* Get max pos of last '\'    */

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
      GetCurrentDirectory((DWORD)len, path);
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
      char fpath[MAX_PATH];
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
/* Purpose:  Returns the value true if the attribute is identical to */
/*           that specified by the mask.  If not the same, then      */
/*           returns the value false.                                */
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
     return false;
                                       /* if only want files and     */
                                       /* is a directory             */
  if (!(options&DO_DIRS) && (options&DO_FILES) && (attr&FILE_ATTRIBUTE_DIRECTORY))
     return false;

  if (mask[0] == RXIGNORE)
     return  true;


  if (mask[0] < 0 && attr&FILE_ATTRIBUTE_ARCHIVE)
    return  false;

  if (mask[0] > 0 && !(attr&FILE_ATTRIBUTE_ARCHIVE))
    return  false;

  if (mask[1] < 0 && attr&FILE_ATTRIBUTE_DIRECTORY)
    return  false;

  if (mask[1] > 0 && !(attr&FILE_ATTRIBUTE_DIRECTORY))
    return  false;

  if (mask[2] < 0 && attr&FILE_ATTRIBUTE_HIDDEN)
    return  false;

  if (mask[2] > 0 && !(attr&FILE_ATTRIBUTE_HIDDEN))
    return  false;

  if (mask[3] < 0 && attr&FILE_ATTRIBUTE_READONLY)
    return  false;

  if (mask[3] > 0 && !(attr&FILE_ATTRIBUTE_READONLY))
    return  false;

  if (mask[4] < 0 && attr&FILE_ATTRIBUTE_SYSTEM)
    return  false;

  if (mask[4] > 0 && !(attr&FILE_ATTRIBUTE_SYSTEM))
    return  false;

  return  true;
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
  ltoa((long)ldp->count, ldp->varname+ldp->stemlen, 10);
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

  CHAR  staticBuffer[MAX_PATH+1];    /* dynamic memory             */
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
  if (maxsize >= MAX_PATH) {
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
        if (maxsize >= MAX_PATH) {
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

size_t RexxEntry SysFileTree(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  CHAR        buff1[MAX];              /* buffer1 ...we may need to  */
  CHAR        buff2[MAX];              /* buffer2 ...alloc new mem...*/
  CHAR       *FileSpec = buff1;        /* File spec to look for      */
  CHAR       *path = buff2;            /* path to search along       */
  const char *optptr;                  /* option scan pointer        */
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

  /* zero terminate, RXVALIDSTRING() guarentees strlength is not 0.  */
  FileSpec[args[0].strlength] = 0x00;

  // If FileSpec ends in \ then append *.*  *
  if (FileSpec[args[0].strlength-1] == '\\')
  {
    strcat(FileSpec, "*.*");
  }
  else if (FileSpec[args[0].strlength-1] == '.')
  {
    // when '.' or '..' are used as directory specifiers append wildcard '\*.*'
    if ( (args[0].strlength > 1) && FileSpec[args[0].strlength-2] != '\\' && FileSpec[args[0].strlength-2] != '.' )
    {
      // The '.' is not used as a directory specifier, but rather is tacked on
      // to the end of a file name. Windows has a sometimes used convention that
      // a '.' at the end of a file name can be used to indicate the file has
      // no extension. For example, given a file named: MyFile a command of
      // dir MyFile. will produce a listing of MyFile  Just remove the '.'
      FileSpec[args[0].strlength-1] = '\0';
    }
    else
    {
      strcat(FileSpec, "\\*.*");
    }
  }

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
  ltoa((long)ldp.count, ldp.Temp, 10);
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
    return CreateDirectory(dir, NULL) == 0 ? 0 : GetLastError();
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
        if (strlen(errmsg)>=retstr->strlength)
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
    return RemoveDirectory(dir) == 0 ? 0 : GetLastError();
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
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*************************************************************************/

size_t RexxEntry SysSleep(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  LONG secs;                           /* Time to sleep in secs      */
  MSG msg;

  LONG milliseconds;
  LONG secs_buf;
  size_t length;
  LONG digits;

  if (numargs != 1)                    /* Must have one argument     */
    return INVALID_ROUTINE;

  /* code fragment taken from lrxutil.c: */
  const char *string = args[0].strptr; /* point to the string        */
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


  /** Using Sleep with a long timeout risks sleeping on a thread with a message
   *  queue, which can make the system sluggish, or possibly deadlocked.  If the
   *  sleep is longer than 333 milliseconds use a window timer to avoid this
   *  risk.
   */
  if ( secs > 333 )
  {
      if ( !(SetTimer(NULL, 0, (secs), (TIMERPROC) SleepTimerProc)) )
          return INVALID_ROUTINE;        /* no timer available, need to abort */
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
      Sleep(secs);
  }

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
* Syntax:    call SysTextScreenSize                                      *
*                                                                        *
* Return:    Size of screen in row and columns returned as:  row, col    *
*************************************************************************/

RexxRoutine0(RexxStringObject, SysTextScreenSize)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    /* if in character mode       */
    if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
    {
        char buffer[100];

        wsprintf(buffer, "%d %d", csbiInfo.dwSize.Y, csbiInfo.dwSize.X);
        return context->NewStringFromAsciiz(buffer);
    }
    else
    {
        return context->NewStringFromAsciiz("0 0");
    }
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
         (numargs == 2 && !RXVALIDSTRING(args[1])) ||args[0].strlength > MAX_PATH )
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

    return rc == 0 ? 0 : GetLastError();
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
 *                           If this is ommitted or 0 no dialog is displayed.
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


// simple class for handling numeric values
class NumericFormatter
{
public:
    NumericFormatter(RexxCallContext *c, uint32_t p)
    {
        if (p == 0)
        {
            precision = DEFAULT_PRECISION;
        }
        else
        {
            precision = min(p, MAX_PRECISION);
        }
        context = c;
    }

    RexxObjectPtr format(double x)
    {
        return context->DoubleToObjectWithPrecision(x, precision);
    }

protected:
    uint32_t precision;
    RexxCallContext *context;
};

class TrigFormatter : public NumericFormatter
{
public:

    TrigFormatter(RexxCallContext *c, int p) : NumericFormatter(c, p)
    {
        units = TRIG_DEGREES;
    }

    bool setUnits(const char *u)
    {
        if (u != NULL)
        {
            switch (*u)
            {
                case 'D':
                case 'd':
                    units = TRIG_DEGREES;
                    break;

                case 'R':
                case 'r':
                    units = TRIG_RADIANS;
                    break;

                case 'G':
                case 'g':
                    units = TRIG_GRADES;
                    break;

                default:
                    context->InvalidRoutine();
                    return false;
            }
        }
        return true;
    }

    RexxObjectPtr evaluate(double angle, int function)
    {
        double    nsi;                       /* convertion factor          */
        double    nco;                       /* convertion factor          */
        double    result;                    /* result                     */

        nsi = 1.;                            /* set default conversion     */
        nco = 1.;                            /* set default conversion     */

        switch (units)
        {
            case TRIG_DEGREES:         {            /* need to convert degrees    */
                    nsi = (angle < 0.) ? -1. : 1.;   /* get the direction          */
                    angle = fmod(fabs(angle), 360.); /* make modulo 360            */
                    if (angle <= 45.)                /* less than 45?              */
                    {
                        angle = angle * pi / 180.;
                    }
                    else if (angle < 135.)
                    {         /* over on the other side?    */
                        angle = (90. - angle) * pi / 180.;
                        function = MAXTRIG - function; /* change the function        */
                        nco = nsi;                     /* swap around the conversions*/
                        nsi = 1.;
                    }
                    else if (angle <= 225.)
                    {        /* around the other way?      */
                        angle = (180. - angle) * pi / 180.;
                        nco = -1.;
                    }
                    else if (angle < 315.)
                    {         /* close to the origin?       */
                        angle = (angle - 270.) * pi / 180.;
                        function = MAXTRIG - function; /* change the function        */
                        nco = -nsi;
                        nsi = 1.;
                    }
                    else
                    {
                        angle = (angle - 360.) * pi / 180.;
                    }
                    break;
                }

            case TRIG_GRADES:              {        /* need to convert degrees    */
                    nsi = (angle < 0.) ? -1. : 1.;   /* get the direction          */
                    angle = fmod(fabs(angle), 400.); /* make modulo 400            */
                    if (angle <= 50.)
                    {
                        angle = angle * pi / 200.;
                    }
                    else if (angle < 150.)
                    {
                        angle = (100. - angle) * pi / 200.;
                        function = MAXTRIG - function; /* change the function        */
                        nco = nsi;                     /* swap the conversions       */
                        nsi = 1.;
                    }
                    else if (angle <= 250.)
                    {
                        angle = (200. - angle) * pi / 200.;
                        nco = -1.;
                    }
                    else if (angle < 350.)
                    {
                        angle = (angle - 300.) * pi / 200.;
                        function = MAXTRIG - function; /* change the function        */
                        nco = -nsi;
                        nsi = 1.;
                    }
                    else
                    {
                        angle = (angle - 400.) * pi / 200.;
                    }
                    break;
                }

            // radians are already ok
            case TRIG_RADIANS:
                break;
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
            {
                context->InvalidRoutine();
                return NULLOBJECT;
            }
            result = nsi * nco / result; /* real result                */
            break;
        }

        // now format based on precision setting
        return format(result);
    }

    RexxObjectPtr evaluateArc(double x, int function)
    {
        double    angle;                     /* working angle              */
        double    nsi;                       /* convertion factor          */
        double    nco;                       /* convertion factor          */

        nsi = 1.;                            /* set default conversion     */
        nco = 1.;                            /* set default conversion     */

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
        if (units == TRIG_DEGREES)         /* have to convert the result?*/
        {
            angle = angle * 180. / pi;     /* make into degrees          */
        }
        else if (units == TRIG_GRADES)     /* need it in grades?         */
        {
            angle = angle * 200. / pi;     /* convert to base 400        */
        }
        // now format based on precision setting
        return format(angle);
    }

protected:

    typedef enum
    {
        TRIG_DEGREES,
        TRIG_RADIANS,
        TRIG_GRADES
    } Units;


    Units units;              // the type of units to process
};


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
RexxRoutine2(RexxObjectPtr, SysSqrt, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(sqrt(x));
}

/*==================================================================*/
RexxRoutine2(RexxObjectPtr, SysExp, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(exp(x));
}

/*==================================================================*/
RexxRoutine2(RexxObjectPtr, SysLog, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(log(x));
}

/*==================================================================*/
RexxRoutine2(RexxObjectPtr, SysLog10, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(log10(x));
}

/*==================================================================*/
RexxRoutine2(RexxObjectPtr, SysSinH, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(sinh(x));
}

/*==================================================================*/
RexxRoutine2(RexxObjectPtr, SysCosH, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(cosh(x));
}


/*==================================================================*/
RexxRoutine2(RexxObjectPtr, SysTanH, double, x, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(tanh(x));
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
RexxRoutine3(RexxObjectPtr, SysPower, double, x, double, y, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    // calculate and return
    return formatter.format(pow(x, y));
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
RexxRoutine3(RexxObjectPtr, SysSin, double, angle, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluate(angle, SINE);
}

RexxRoutine3(RexxObjectPtr, SysCos, double, angle, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluate(angle, COSINE);
}

RexxRoutine3(RexxObjectPtr, SysTan, double, angle, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluate(angle, TANGENT);
}

RexxRoutine3(RexxObjectPtr, SysCotan, double, angle, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluate(angle, COTANGENT);
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
RexxRoutine1(RexxObjectPtr, SysPi, OPTIONAL_uint32_t, precision)
{
    NumericFormatter formatter(context, precision);

    return formatter.format(pi);
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
RexxRoutine3(RexxObjectPtr, SysArcSin, double, x, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluateArc(x, ARCSINE);
}

/*==================================================================*/
RexxRoutine3(RexxObjectPtr, SysArcCos, double, x, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluateArc(x, ARCCOSINE);
}

/*==================================================================*/
RexxRoutine3(RexxObjectPtr, SysArcTan, double, x, OPTIONAL_uint32_t, precision, OPTIONAL_CSTRING, units)
{
    TrigFormatter formatter(context, precision);
    if (!formatter.setUnits(units))
    {
        return NULLOBJECT;
    }

    // calculate and return
    return formatter.evaluateArc(x, ARCTANGENT);
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

RexxRoutine2(RexxObjectPtr, SysGetFileDateTime, CSTRING, name, CSTRING, selector)
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
    HANDLE setFile = CreateFile(name, GENERIC_READ,
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
        if ( sscanf(args[3].strptr, "%Iu", &first) != 1 )
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
        if ( sscanf(args[4].strptr, "%Iu", &last) != 1 )
            return INVALID_ROUTINE;
        if ( last < first )
            return INVALID_ROUTINE;
    }
    // first column to sort
    if ( (numargs >= 6) && RXVALIDSTRING(args[5]) )
    {
        if ( sscanf(args[5].strptr, "%Iu", &firstCol) != 1 )
        {
            return INVALID_ROUTINE;
        }
        firstCol--;
    }
    // last column to sort
    if ( (numargs == 7) && RXVALIDSTRING(args[6]) )
    {
        if ( sscanf(args[6].strptr, "%Iu", &lastCol) != 1 )
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

    // now set the new value
    context->SetStemArrayElement(toStem, position, newValue);
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
            context->SetStemArrayElement(toStem, index, value);
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
    const char  *strDefaultChar = NULL;
    BOOL  bUsedDefaultChar = FALSE;

    const char *source = context->StringData(sourceString);
    size_t sourceLength = context->StringLength(sourceString);

    /* calculate the length of the input string */
    size_t len = wcslen((wchar_t*)(source)) + 1;

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
    if (mappingFlags != NULL)
    {
        /* all flags MUST also specify WC_COMPOSITECHECK */
        if (mystrstr(mappingFlags, "COMPOSITECHECK"))
        {
            dwFlags |= WC_COMPOSITECHECK;
        }
        if (mystrstr(mappingFlags, "SEPCHARS"))
        {
            dwFlags |= WC_SEPCHARS | WC_COMPOSITECHECK;
        }
        if (mystrstr(mappingFlags, "DISCARDNS"))
        {
            dwFlags |= WC_DISCARDNS| WC_COMPOSITECHECK;
        }
        if (mystrstr(mappingFlags, "DEFAULTCHAR"))
        {
            dwFlags |= WC_DEFAULTCHAR | WC_COMPOSITECHECK;
        }
        if (dwFlags == 0)
        {
            context->InvalidRoutine();
            return 0;
        }
    }

    /* evaluate default charcter  */
    if (defaultChar != NULL && (dwFlags & WC_DEFAULTCHAR) == WC_DEFAULTCHAR)
    {
        strDefaultChar = defaultChar;
    }
    else
    {
        /* use our own default character rather than relying on the windows default */
        strDefaultChar = "?";
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
                                        (LPWSTR) strptr,     // (LPWSTR)args[0].strptr,
                                        (int)(sourceLength/2), // len,
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
    int iBytesDestination = WideCharToMultiByte(codePage,               //codepage
                                            dwFlags,                //conversion flags
                                            (LPWSTR) strptr,        // (LPWSTR)args[0].strptr,  //source string
                                            (int)(sourceLength/2),    // len,                     //source string length
                                            str,                    //target string
                                            (int)iBytesNeeded,      //size of target buffer
                                            strDefaultChar,
                                            &bUsedDefaultChar);

    if (iBytesDestination == 0) // call to function fails
    {
        GlobalFree(str);          //  free allocated string
        GlobalFree(strptr);          // free allocated string
        return GetLastError();    // return error from function call
    }

    // set the default character flag in the output stem
    if (bUsedDefaultChar && (dwFlags & WC_DEFAULTCHAR) == WC_DEFAULTCHAR)
    {
        context->SetStemElement(stem, "!USEDDEFAULTCHAR", context->String(strDefaultChar));
    }
    else
    {
        context->SetStemElement(stem, "!USEDDEFAULTCHAR", context->String(strDefaultChar));
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
    REXX_CLASSIC_ROUTINE(SysFileTree,                 SysFileTree),
    REXX_CLASSIC_ROUTINE(SysGetKey,                   SysGetKey),
    REXX_CLASSIC_ROUTINE(SysIni,                      SysIni),
    REXX_CLASSIC_ROUTINE(SysLoadFuncs,                SysLoadFuncs),
    REXX_TYPED_ROUTINE(SysMkDir,                      SysMkDir),
    REXX_CLASSIC_ROUTINE(SysWinVer,                   SysWinVer),
    REXX_CLASSIC_ROUTINE(SysVersion,                  SysVersion),
    REXX_TYPED_ROUTINE(SysRmDir,                      SysRmDir),
    REXX_CLASSIC_ROUTINE(SysSearchPath,               SysSearchPath),
    REXX_CLASSIC_ROUTINE(SysSleep,                    SysSleep),
    REXX_CLASSIC_ROUTINE(SysTempFileName,             SysTempFileName),
    REXX_TYPED_ROUTINE(SysTextScreenRead,             SysTextScreenRead),
    REXX_TYPED_ROUTINE(SysTextScreenSize,             SysTextScreenSize),
    REXX_TYPED_ROUTINE(SysPi,                         SysPi),
    REXX_TYPED_ROUTINE(SysSqrt,                       SysSqrt),
    REXX_TYPED_ROUTINE(SysExp,                        SysExp),
    REXX_TYPED_ROUTINE(SysLog,                        SysLog),
    REXX_TYPED_ROUTINE(SysLog10,                      SysLog10),
    REXX_TYPED_ROUTINE(SysSinH,                       SysSinH),
    REXX_TYPED_ROUTINE(SysCosH,                       SysCosH),
    REXX_TYPED_ROUTINE(SysTanH,                       SysTanH),
    REXX_TYPED_ROUTINE(SysPower,                      SysPower),
    REXX_TYPED_ROUTINE(SysSin,                        SysSin),
    REXX_TYPED_ROUTINE(SysCos,                        SysCos),
    REXX_TYPED_ROUTINE(SysTan,                        SysTan),
    REXX_TYPED_ROUTINE(SysCotan,                      SysCotan),
    REXX_TYPED_ROUTINE(SysArcSin,                     SysArcSin),
    REXX_TYPED_ROUTINE(SysArcCos,                     SysArcCos),
    REXX_TYPED_ROUTINE(SysArcTan,                     SysArcTan),
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

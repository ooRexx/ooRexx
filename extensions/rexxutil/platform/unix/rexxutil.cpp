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
/* REXX AIX/LINUX Support                                                     */
/*                                                                            */
/* AIX  system utility functions                                              */
/*                                                                            */
/******************************************************************************/
/**********************************************************************
*   LRXUTIL.C                                                         *
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
*       SysGetMessage       -- Retrieves a message text from an OS/2  *
*                              message file, substituting values      *
*                              provided.                              *
*       SysIni              -- Reads and/or updates entries in .INI   *
*                              files.                                 *
*       SysLoadFuncs        -- Makes all functions in this package    *
*                              known to REXX so REXX programs may     *
*                              call them.                             *
*       SysMkDir            -- Creates a directory                    *
*       SysVersion          -- Returns the AIX  Version number        *
*       SysLinVer           -- Returns the OS/2 Version number        *
*       SysRmDir            -- Removes a directory                    *
*       SysSearchPath       -- Searches throught a specified path     *
*                              for a file.                            *
*       SysSleep            -- Suspends execution for a number of     *
*                              seconds.                               *
*       SysTempFilename     -- Creates a unique filename              *
*       SysTextScreenRead   -- Reads characters from the screen,      *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*       SysTextScreenSize   -- Returns the size of the window in      *
*                              rows and columns,                      *
*                              in an OS/2 fullscreen or windowed      *
*                              command prompt session.                *
*EX     SysGetEA            -- Reads an extended attribute            *
*                              for a file.                            *
*EX     SysPutEA            -- Writes an extended attribute           *
*                              for a file.                            *
*EX     SysWaitNamedPipe    -- Wait on a named pipe.                  *
*EX     SysSetIcon          -- Set a file icon                        *
*EX     SysRegisterObjectClass -- Register a new object class         *
*EX     SysDeregisterObjectClass -- Remove class registration         *
*EX     SysQueryClassList   -- Get list of registered classes         *
*EX     SysCreateObject     -- Create an object instance              *
*EX     SysDestroyObject    -- Delete an object instance              *
*EX     SysSetObjectData    -- Change object settings data            *
*EX     SysElapsedTime      -- Return the OS/2 elapsed time count     *
*EX     SysBootDrive        -- Return the OS/2 boot drive             *
*EX     SysQueryEAList      -- Return list of file EA names           *
*EX     SysWildCard         -- Perform file wild card editting        *
*EX     SysFileSystemType   -- Return drive file system type          *
*EX     SysAddFileHandle    -- Add file handles to a process          *
*EX     SysSetFileHandle    -- Set file handles for a process         *
*       SysCreateMutexSem   -- Create a Mutex semaphore               *
*       SysOpenMutexSem     -- Open a Mutex semaphore                 *
*       SysCloseMutexSem    -- Close a Mutex semaphore                *
*       SysRequestMutexSem  -- Request a Mutex semaphore              *
*       SysReleaseMutexSem  -- Release a Mutex semaphore              *
*       SysCreateEventSem   -- Create an Event semaphore              *
*       SysOpenEventSem     -- Open an Event semaphore                *
*       SysCloseEventSem    -- Close an Event semaphore               *
*       SysPostEventSem     -- Post an Event semaphore                *
*       SysResetEventSem    -- Reset an Event semaphore               *
*       SysWaitEventSem     -- Wait on an Event semaphore             *
*EX     SysProcessType      -- Return type of process                 *
*EX     SysSetPriority      -- Set current thread priority            *
*EX     SysGetCollate       -- Get country/codepage collating sequence*
*EX     SysNationalLanguageCompare -- NLS strict compare              *
*EX     SysMapCase          -- NLS uppercasing                        *
*EX     SysSetProcessCodePage -- Set current code page                *
*EX     SysQueryProcessCodePage -- Get current code page              *
*       SysAddRexxMacro     -- Load program into macro space          *
*       SysDropRexxMacro    -- Drop program from macro space          *
*       SysReorderRexxMacro -- Reorder program in macro space         *
*       SysQueryRexxMacro   -- Query ordering of macro space program  *
*       SysClearRexxMacroSpace -- Remove all programs from macro space*
*       SysLoadRexxMacroSpace  -- Load a Rexx macro space             *
*       SysSaveRexxMacroSpace  -- Save a Rexx macro space             *
*EX     SysShutDownSystem   -- Shutdown the system                    *
*EX     SysSwitchSession    -- Switch to a named session              *
*EX     SysDropLibrary      -- Drop a function package                *
*  EX   SysPi               -- Return Pi to given precision           *
*  EX   SysSqrt             -- Calculate a square root                *
*  EX   SysExp              -- Calculate an exponent                  *
*  EX   SysLog              -- Return natural log of a number         *
*  EX   SysLog10            -- Return log base 10 of a number         *
*  EX   SysSinh             -- Hyperbolic sine function               *
*  EX   SysCosh             -- Hyperbolic cosine function             *
*  EX   SysTanh             -- Hyperbolic tangent function            *
*  EX   SysPower            -- raise number to non-integer power      *
*  EX   SysSin              -- Sine function                          *
*  EX   SysCos              -- Cosine function                        *
*  EX   SysTan              -- Tangent function                       *
*  EX   SysCotan            -- Cotangent function                     *
*  EX   SysArcSin           -- ArcSine function                       *
*  EX   SysArcCos           -- ArcCosine function                     *
*  EX   SysArcTan           -- ArcTangent function                    *
*EX     SysSaveObject       -- Save an object either Async or Sync    *
*EX     SysOpenObject       -- Open a view of an object               *
*EX     SysMoveObject       -- Move an object from the existing folder*
*                              to the destination folder              *
*EX     SysCopyObject       -- Copy an object from the existing folder*
*                              to the destination folder              *
*EX     SysCreateShadow     -- Create a shadow of an object           *
*EX     SysWaitForShell     -- Wait for WPS initialization            *
*                              (Merlin only).                         *
*EX     SysQuerySwitchList  -- query the entries of the switch list   *
*                              (for SysSwitchSession)                 *
*       SysDumpVariables    -- Dump current variables to a file       *
*       SysSetFileDateTime  -- Set the last modified date of a file   *
*       SysGetFileDateTime  -- Get the last modified date of a file   *
*       SysStemSort         -- sort a stem array                      *
*       SysStemDelete       -- delete items in a stem array           *
*       SysStemInsert       -- insert items into a stem array         *
*       SysStemCopy         -- copy items from one stem array to other*
*       SysGetErrortext     -- Retrieve textual desc. of error number *
*       SysQueryProcess     -- Get information on current proc/thread *
*       SysUtilVersion      -- query version of REXXUTIL.DLL          *
*                                                                     *
*       SysAddFuncPkg       -- CREXX for AIX function support         *
*       SysAddCmdPkg        -- CREXX for AIX function support         *
*       SysDropFuncPkg      -- CREXX for AIX function support         *
*       SysDropCmdPkg       -- CREXX for AIX function support         *
*       SysGetpid           -- CREXX for AIX function support         *
*       SysFork             -- CREXX for AIX function support         *
*       SysWait             -- CREXX for AIX function support         *
*       SysCreatePipe       -- CREXX for AIX function support         *
*                                                                     *
*       SysIsFile           -- does file exist?                       *
*       SysIsFileDirectory  -- is file a subdirectory?                *
*       SysIsFileLink       -- is file a link?                        *
*                                                                     *
**********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined( HAVE_LOCALE_H )
# include <locale.h>
#endif

#include "oorexxapi.h"

#if defined( HAVE_SYS_WAIT_H )
# include <sys/wait.h>
#endif

#include <sys/ipc.h>
#include <memory.h>

#if defined( HAVE_MALLOC_H )
# include <malloc.h>
#endif

#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <limits.h>
#include <sys/stat.h>                  /* mkdir() function           */
#include <errno.h>                     /* get the errno variable     */
#include <stddef.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <semaphore.h>


#if defined( HAVE_SYS_SEM_H )
# include <sys/sem.h>
#endif

#include <dirent.h>                    /* directory functions        */
#include <sys/time.h>                  /* needed for the select func */

#include <time.h>                 /* needed for the select func          */

#if defined( HAVE_SYS_SELECT_H )
# include <sys/select.h>           /* needed for the select func          */
#endif

#if defined( HAVE_SYS_LDR_H )
# include <sys/ldr.h>              /* needed for the load   func          */
#endif

#if defined( HAVE_STRINGS_H )
# include <strings.h>
#endif

#include <utime.h>                /* moved, used by AIX & Linux          */

#if defined( HAVE_SYS_UTSNAME_H )
# include <sys/utsname.h>               /* get the uname() function   */
#endif

#include <signal.h>

#if defined( HAVE_SYS_RESOURCE_H )
# include <sys/resource.h>              /* get the getpriority() func */
#endif

#if defined( HAVE_FEATURES_H )
# include <features.h>                 /* catopen()                  */
#endif

#if defined( HAVE_NL_TYPES_H )
# include <nl_types.h>                  /* catopen()                  */
#endif

#include <termios.h>                   /* needed for SysGetKey       */
#include <fnmatch.h>                   /* fnmatch()                  */

#if !defined( HAVE_UNION_SEMUN )
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
#endif

extern char *resolve_tilde(const char *);

#define INVALID_ROUTINE 40
#define  MAX_DIGITS     9
#define  NO_UTIL_ERROR    "0"          /* No error whatsoever        */
#define  VALID_ROUTINE    0            /* Successful completion      */
//#define  MAX_LINE_LEN   2048         /* max line length */
#define  MAX_LINE_LEN   4096           /* max line length */
#define  MAX_READ       0x10000        /* full segment of buffer     */
#define  CH_EOF         0x1A           /* end of file marker         */
#define  CH_CR          '\r'           /* carriage return character  */
#define  CH_NL          '\n'           /* new line character         */
#define  MAX            256            /* temporary buffer length    */
#define  IBUF_LEN       4096           /* Input buffer length        */
#define CURRENT_DIR_FIRST 0            /* search flag 'C'            */
#define ENVIRONMENT_ONLY  1            /* search flag 'N'            */
#define OFFSET          1000           /* needed to prevent collision*/
                                       /* with the return codes      */
#define MAXUSECOUNT     65535          /* max semaphore usecount     */
#define REXXMESSAGEFILE    "rexx.cat"


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
/* Defines used by SysStemSort                                       */
/*********************************************************************/
#define SORT_CASESENSITIVE 0
#define SORT_CASEIGNORE    1


#define SORT_ASCENDING 0
#define SORT_DECENDING 1

#define SORT_NUMERIC       3

#define SORT_DEF_AVG_SIZE  20

/*********************************************************************/
/* Some useful macros                                                */
/*********************************************************************/

#define BUILDRXSTRING(t, s) { \
  strcpy((t)->strptr,(s));\
  (t)->strlength = strlen((s)); \
}


#define RETVAL(retc) { \
  sprintf(retstr->strptr, "%d", retc); \
  retstr->strlength = strlen(retstr->strptr); \
  return VALID_ROUTINE; \
}

/*********************************************************************/
/* Defines uses by SysTree                                           */
/*********************************************************************/

#define  RECURSE        0x0002
#define  DO_DIRS        0x0004
#define  DO_FILES       0x0008
#define  NAME_ONLY      0x0010
#define  EDITABLE_TIME  0x0020
#define  LONG_TIME      0x0040
#define  CASELESS       0x0080
#define  IGNORE         2              /* Ignore attributes entirely */


/******************************************************************************/
/* Defines used by SysGetKey                                                  */
/******************************************************************************/

#define stty(a,b)         (void)tcsetattr(a,TCSANOW,b) /* simple set attr.    */
#define gtty(a,b)         (void)tcgetattr(a,b)         /* simple get attr.    */
#define discard_input(a)  tcflush(a,TCIFLUSH)          /* simple flush        */
#define restore_tty(a)    stty(ttyfd,a)                /* new settings STDIN  */


/* original terminal settings                                                 */
struct termios in_orig;                /* original settings (important!!)     */


/*********************************************************************/
/* RxTree Structure used by SysTree.                                 */
/*********************************************************************/

typedef struct RxTreeData {
    size_t count;                      /* Number of lines processed  */
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    size_t stemlen;                    /* Length of stem             */
    size_t vlen;                       /* Length of variable value   */
    char TargetSpec[MAX+1];            /* Target filespec            */
    char truefile[MAX+1];              /* expanded file name         */
    char Temp[MAX+80];                 /* buffer for returned values */
    char varname[MAX];                 /* Buffer for variable name   */
    size_t nattrib;                    /* New attrib, diff for each  */
} RXTREEDATA;


/*********************************************************************/
/* RxTree Structure used by GetLine, OpenFile and CloseFile          */
/*********************************************************************/

typedef struct _GetFileData {
  char *       buffer;                 /* file read buffer           */
  size_t       size;                   /* file size                  */
  size_t       data;                   /* data left in buffer        */
  size_t       residual;               /* size left to read          */
  const char  *scan;                   /* current scan position      */
  FILE         *handle;                /* file handle                */
} GetFileData;

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
/* SORTMEM used by SysStemSort                                       */
/*********************************************************************/
typedef struct _SORT_MEM {
  size_t                ulSize;
  size_t                ulRemaining;
  size_t                ulItems;
  char *                pNextBlock;
  struct _SORT_MEM      *pNext;
  char                  pData;
} SORTMEM, *PSORTMEM;


#ifdef XX__cplusplus
extern "C" {
#endif

/********************************************************************
* Function:  string2ulong(string, number)                           *
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
    return false;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return false;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator * 10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator;               /* return the value           */
  return true;                         /* good number                */
}

/*********************************************************************/
/****************  REXXUTIL Supporting Functions  ********************/
/****************  REXXUTIL Supporting Functions  ********************/
/****************  REXXUTIL Supporting Functions  ********************/
/*********************************************************************/

/********************************************************************
* Function:  ReadNextBuffer(filedata)                               *
*                                                                   *
* Purpose:   Reads the next buffer of data.                         *
*                                                                   *
* RC:        0       buffer was read                                *
*            1     - error occurred reading buffer                  *
*********************************************************************/
int ReadNextBuffer(
   GetFileData  *filedata )            /* global file information    */
{
  size_t    size;                      /* size to read               */
  char     *endptr;                    /* end of file pointer        */
                                       /* get size of this read      */
  if(filedata->residual >= MAX_READ)
    size = MAX_READ;                   /* read as much as possible   */
  else
    size = filedata->residual;         /* take the rest              */
                                       /* read the file in           */
  filedata->data = fread(filedata->buffer, 1, size, filedata->handle);
  if(!filedata->data)                  /* problems ?                 */
    return (1);                        /* get out                    */
  if (filedata->data != size)          /* not get all of it?         */
    filedata->residual = 0;            /* no residual                */
  else                                 /* residual is remainder      */
    filedata->residual = filedata->residual - size;
                                       /* look for a EOF mark        */
  endptr = (char *)memchr(filedata->buffer, CH_EOF, filedata->data);

  if (endptr) {                        /* found an EOF mark          */
                                       /* set new length             */
    filedata->data = (size_t)(endptr - filedata->buffer);
    filedata->residual = 0;            /* no residual                */
  }
  filedata->scan = filedata->buffer;   /* set position to beginning  */
  return 0;
}

/***********************************************************************/
/* Function: strupr(string)                                            */
/* Purpose:  Uppercas the given string                                 */
/***********************************************************************/
void strupr(char * string){

  for(;*string != '\0';string++){        /* while not at the end       */
    *string = toupper(*string);
  }
}

/********************************************************************
* Function:  OpenFile(file, filedata)                               *
*                                                                   *
* Purpose:   Prepares a file for reading.                           *
*                                                                   *
* RC:        0     - file was opened successfully                   *
*            1     - file open error occurred                       *
*********************************************************************/

int OpenFile(
   const char  *file,                  /* file name                  */
   GetFileData *filedata )             /* global file information    */
{
   struct stat finfo;                  /* file information           */
   char *      endptr = NULL;          /* end of buffer pointer      */

                                       /* try to open the file       */
  if((filedata->handle = fopen(file,"r")) == NULL)
    return (1);                        /* return failure             */
                                       /* retrieve the file size     */
  if((stat(file,&finfo) == -1)||(!finfo.st_size)){
    fclose(filedata->handle);          /* close the file             */
    return (1);                        /* and quit                   */
  }
  if (finfo.st_size <= MAX_READ) {     /* less than a single buffer  */
                                       /* allocate buffer for file   */
    if((filedata->buffer = (char *)malloc(finfo.st_size)) == NULL ){
      fclose(filedata->handle);        /* close the file             */
      return (1);                      /* and quit                   */
    }
    filedata->size = finfo.st_size;    /* save file size             */
    filedata->residual = 0;            /* no left over information   */
                                       /* read the file in           */
    filedata->data = fread(filedata->buffer, 1,
                                      finfo.st_size, filedata->handle);
    if(filedata->data != (size_t)finfo.st_size){/*  problems ?               */
      free(filedata->buffer);          /* free the buffer            */
      fclose(filedata->handle);        /* close the file             */
      return (1);                      /* and quit                   */
    }                                  /* look for a EOF mark        */
    endptr = (char *)memchr(filedata->buffer, CH_EOF, filedata->data);
    if (endptr)                        /* found an EOF mark          */
                                       /* set new length             */
      filedata->data = (size_t)(endptr - filedata->buffer);
    filedata->scan = filedata->buffer; /* set position to beginning  */
  }
  else {                               /* need to read partial       */
                                       /* allocate buffer for read   */
    if((filedata->buffer = (char *)malloc(MAX_READ)) == NULL ){
      fclose(filedata->handle);        /* close the file             */
      return (1);                      /* and quit                   */
    }
    filedata->size = finfo.st_size;    /* save file size             */
                                       /* and set remainder          */
    filedata->residual = filedata->size;
                                       /* read the file in           */
    if (ReadNextBuffer(filedata)) {
      free(filedata->buffer);          /* free the buffer            */
      fclose(filedata->handle);        /* close the file             */
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
  fclose(filedata->handle);            /* close the file             */
  free(filedata->buffer);              /* release the file buffer    */
}

/*********************************************************************
* Function:  GetLine(line, size, filedata)                          *
*                                                                   *
* Purpose:   Reads a line of data using buffered reads.  At end of  *
*            file, zero is returned to indicate nothing left.       *
*                                                                   *
* RC:        true -  line was read successfully                     *
*            false - end of file was reached                        *
*********************************************************************/

int GetLine(
   char *line,                        /* returned line              */
   size_t      size,                  /* size of line buffer        */
   GetFileData *filedata )            /* file handle                */
{
   const char *scan;                    /* current scan pointer       */
   size_t        length;                /* line length                */
   size_t        copylength;            /* copied length              */


  if (!(filedata->data)) {             /* if out of current buffer   */
    if (filedata->residual) {          /* may be another buffer      */
      ReadNextBuffer(filedata);        /* try to read one            */
      if (!filedata->data)             /* nothing more?              */
        return 1;                      /* all done                   */
    }
    else
      return (1);                      /* return EOF condition       */
  }
                                       /* look for a line feed       */
  scan = (const char *)memchr(filedata->scan, CH_NL, filedata->data);
  if (scan) {                          /* found one                  */
                                       /* calculate the length       */
    length = scan - filedata->scan;
    copylength = length;
    if (copylength > size)
    {
        copylength = size;
    }
                                       /* copy over the data         */
    memcpy(line, filedata->scan, copylength);
    line[copylength] = '\0';           /* make into ASCIIZ string    */

    /* we don't want the CR character in the result string*/
    if ( line[copylength - 1] == CH_CR ) {
      line[copylength - 1] = '\0';
    }

    filedata->data -= length + 1;      /* reduce the length          */
    filedata->scan = scan + 1;         /* set new scan point         */

    if (!filedata->data) {             /* all used up                */
      if (filedata->residual)          /* more to read               */
        ReadNextBuffer(filedata);      /* read the next buffer       */
    }
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
       line[copylength] = '\0';           /* make into ASCIIZ string    */

     /* all data should be read, filedata->data must be zero          */
       filedata->data -= copylength;
     /* scan should be at the end                                     */
       filedata->scan += copylength;    /* set new scan point         */

    /* if no more data to read in the file, return OK     */
       if (!filedata->residual)
          return 0;
       else
          return GetLine(line + copylength, size - copylength, filedata);
    }
    else        /* the line is full, scan until LF found but no copy */
    {
       copylength = filedata->data;
       if (size < copylength)
       {
           copylength = size;
       }
                                      /* copy over the data         */
       memcpy(line, filedata->scan, copylength);
       line[copylength] = '\0';           /* make into ASCIIZ string    */

    /* we don't want the CR character in the result string*/
       filedata->data  = 0;             /* no data in buffer          */
       filedata->scan += filedata->data;     /* set scan point to end */

       if (filedata->residual)          /* more to read               */
       {
           ReadNextBuffer(filedata);    /* read the next buffer       */
           return GetLine(line + copylength, 0, filedata);
       }
       else
          return 0;
    }
  }
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

const char *mystrstr(
  const char   *haystack,
  const char   *needle,
  size_t   hlen,
  size_t   nlen,
  bool    sensitive)

{
  char line[MAX_LINE_LEN];
  char target[MAX_LINE_LEN];
  size_t p;

 /* Copy line  - Change nulls to spaces and uppercase if needed      */

  for (p = 0; p < hlen; p++) {

    if (haystack[p] == '\0')
      line[p] = ' ';
    else if (sensitive)
      line[p] = haystack[p];
    else line[p] = toupper(haystack[p]);
  }
  line[p] = '\0';
 /* Copy target  - Change nulls to spaces and uppercase if needed    */

  for (p = 0; p < nlen; p++) {

    if (needle[p] == '\0')
      target[p] = ' ';
    else if (sensitive)
      target[p] = needle[p];
    else target[p] = toupper(needle[p]);
  }
  target[p] = '\0';
  const char *hit = strstr(line, target);
  if (hit == NULL)
  {
      return NULL;
  }
  // adjust original pointer to offset
  return haystack +  (hit - line);
}

/*************************************************************************
* Function:  get_next_path                                               *
*                                                                        *
* Purpose:   Read one path out of an environment value pointed to by     *
*            ppenv.                                                      *
*            Used by the 'SearchPath' func.                              *
* Return:    0 - read successful                                         *
*            1 - end of environment entry                                *
*************************************************************************/
int get_next_path(
  char * *ppenv,                      /* environment pointer              */
  char * path_buf)                    /* path buffer                      */
{
  int i;                             /* counter                          */

  if(*ppenv == NULL)                 /* environment entry available ?    */
    return (1);                      /* return end of envrionment        */
  if(**ppenv == ':')                 /* if we point to the seperator     */
    (*ppenv)++;                      /* jump over                        */
  if(**ppenv == '\0')                /* if end of environment entry      */
    return (1);                      /* return so                        */
                                     /* copy the path out of the         */
                                     /* environment entry                */
  for(i=0;(**ppenv != ':')&&(**ppenv != '\0');(*ppenv)++){
    if(i>MAX_LINE_LEN)               /* if buffer to short               */
      return (1);                    /* get out                          */
    path_buf[i++] = **ppenv;         /* copy this character              */
  }
  path_buf[i] = '\0';                /* terminate the string             */
  return (0);                        /* found another path               */
}

/*************************************************************************
* Function:  SearchPath                                                  *
*                                                                        *
* Purpose:   Search a file along the given environment entry and return  *
*            the full filespec if found.                                 *
*                                                                        *
* Return:    0 - found the file(=>buf is modified)                       *
*            1 - not found                                               *
*************************************************************************/
int SearchPath(
  int   SearchFlag,                    /* search curr dir first ?    */
  const char * path,                   /* environment variable name  */
  const char * filename,               /* searched file              */
  char * buf,                          /* returned filespec if found */
  size_t   buf_size)                   /* size of the return buffer  */
{

  int   rc = 1;                        /* init rc to not found       */
  DIR   *dp;                           /* directory pointer          */
  struct dirent *ep;                   /* directory entry pointer    */
  int   length;                        /* path length                */
  char  path_buf[IBUF_LEN];            /* current search path        */
  char * penv;                          /* ptr to the environment     */

  if(!SearchFlag){                     /* search curr dir first ?    */
    dp = opendir("./");                /* open current directory     */
    if(dp != NULL){                    /* worked well ?              */
      while((ep = readdir(dp))){       /* while entries in the dir   */
                                       /* if we have a match         */
        if(!strcmp(ep->d_name,filename)){
          if(!getcwd(buf,buf_size))    /* copy the cwd to return buf */
            return rc;                 /* failed, get out            */
          length = strlen(buf);        /* get the length of the path */
          if((length+2+strlen(filename))>buf_size)/* check buf space */
            return rc;                 /* not enough, get out        */
          buf[length] = '/';           /* add a slash                */
          buf[length+1] = '\0';        /* and update the terminater  */
          strcat(buf,filename);        /* now complete the filespec  */
          rc = 0;                      /* Yep,found !                */
        }
      }
      (void)closedir(dp);              /* close the directory        */
    }
  }
  if(!rc)                              /* if we already found it     */
   return rc;                          /* return to caller           */

  /* Now search along the environment entry                          */
  penv = getenv(path);                 /* get the environment entry  */
  if(!penv)                            /* if couldn't get the env    */
    return rc;                         /* get out                    */
                                       /* while we have another path */
                                       /* to search for the file     */
  while(!get_next_path(&penv,path_buf)){
    dp = opendir(path_buf);            /* open the directory         */
    if(dp != NULL){                    /* worked well ?              */
      while((ep = readdir(dp))){       /* while entries in the dir   */
                                       /* if we have a match         */
        if(!strcmp(ep->d_name,filename)){
          if(strlen(path_buf)>buf_size)/* check the size             */
            return rc;                 /* get out                    */
          strcpy(buf,path_buf);        /* copy path to the return buf*/
          length = strlen(buf);        /* get the length of the path */
          if((length+2+strlen(filename))>buf_size)/* check buf space */
            return rc;                 /* not enough, get out        */
          buf[length] = '/';           /* add a slash                */
          buf[length+1] = '\0';        /* and update the terminater  */
          strcat(buf,filename);        /* now complete the filespec  */
          (void)closedir(dp);          /* close the directory        */
          return 0;                    /* Yep,found !                */
        }
      }
      (void)closedir(dp);              /* close the directory        */
    }
  }
  return rc;                           /* return not found           */
}


/*****************************************************************
* Function:  getpath(string, path, filename)                     *
*                                                                *
* Purpose:  This function gets the PATH and FILENAME of the file *
*           target contained in STRING.  The path will end with  *
*           the '/' char if a path is supplied.                  *
*                                                                *
*****************************************************************/

void getpath(
    char *string,
    char *path,
    char *filename )
{
  int    iLen;                         /* length of filespec         */
  int    LastSlashPos;                 /* position of last slash     */
  char   szSavePath[IBUF_LEN];         /* Keep path to get back to   */

  if (!strcmp(string, "."))            /* period case?               */
    strcpy(string, "./*");             /* make it a * request        */
  else if (!strcmp(string, ".."))      /* double period case?        */
    strcpy(string, "../*");            /* make it a ../ * request    */
  iLen = strlen(string);               /* Get length of full file    */
                                       /* spec                       */
  LastSlashPos = iLen-1;               /* Get max pos of last '/'    */
  /* Step back through string until it begins or at last '/' char    */
  do
    LastSlashPos--;
  while((string[LastSlashPos] != '/') && (LastSlashPos>=0));

  if (string[LastSlashPos] == '/'){    /* if we have a slash         */
    /* Get file name from filespec (just after last '/')             */
    if (string[LastSlashPos+1])        /* have a real name?          */
                                       /* copy it over               */
      strcpy(filename, &string[LastSlashPos+1]);
    else
      strcpy(filename, "*");           /* just use wildcards         */
  }
  else {                               /* no '/' just filename       */
      strcpy(filename, &string[LastSlashPos+1]);
  }

  /* Now resolve to fully qualified path ---------------------------- */
  iLen = strlen(filename);             /* Get file name length        */
  if (string[LastSlashPos] != '/')     /* if we have no slash         */
  {                                    /* resolve current dir         */
    if (!getcwd(path, (IBUF_LEN - iLen - 2)))
       strcpy(path, "./");             /* if no cwd set current dir  */
    else
       strcat(path, "/");
  }
  else {                               /* there is path info         */
    strncpy(path, string, LastSlashPos+1);/* copy the path out       */
    *(path+LastSlashPos+1) = '\0';     /* make ASCII-Z string        */
    if (getcwd(szSavePath, (IBUF_LEN - 1 )))
    {
      if (!chdir(path))                /* If successful get           */
      {                                /*   reolved path name         */
        if ((getcwd(path, (IBUF_LEN - iLen - 2 ))) &&
            ( LastSlashPos > 0 ))
           strcat(path, "/");          /* Add if not root dir         */
        chdir(szSavePath);             /* Back to current dir         */
      }
    }
  }
}


/*****************************************************************************
* Function:  LinFindNextFile(path, dir_handle, finfo, d_name, caseless)      *
*                                                                            *
* Purpose:  This function finds the next file in the directory PATH pointed  *
*           by DIR_HANDLE which matchs the filespec.  All needed info is     *
*           returned via the FINFO struct and the D_NAME pointer.            *
*                                                                            *
* Note:  '?' is currently not supported. Add the impletmentation here !      *
******************************************************************************/
int LinFindNextFile(
  const char * filespec,               /* filespec to search for     */
  const char * path,                   /* current path               */
  DIR *dir_handle,                     /* directory handle           */
  struct stat *finfo,                  /* return buf for the finfo   */
  char * *d_name,                      /* name of the file found     */
  size_t caseless)                     /* case insensitive matching  */
{
  struct dirent *dir_entry;            /* Directory entry            */
  char full_path[IBUF_LEN+1];
                                       /* strtok routine             */

 if(!(dir_entry = readdir(dir_handle)))/* get first entry            */
   return 0;                           /* no entry or EOF            */

 do{

                                       /* make full spec             */
    sprintf(full_path, "%s%s", path, dir_entry->d_name);
    lstat(full_path, finfo);           /* read the info about it     */

  if(S_ISREG(finfo->st_mode) ||        /* if it is a file            */
     S_ISCHR(finfo->st_mode) ||        /* or a device special        */
     S_ISBLK(finfo->st_mode) ||        /* file                       */
     S_ISSOCK(finfo->st_mode) ||       /* or a socket                */
     S_ISLNK(finfo->st_mode) ||        /* or a symbolic link         */
     S_ISFIFO(finfo->st_mode)){        /* or a FIFO                  */

    if (caseless) {                    /* if caseless search         */
      char dup_d_name[IBUF_LEN+1];     /* compare upper cased copy   */
      char * pDest = dup_d_name;        /* of the entry name          */
      char * pSrc  = dir_entry->d_name;

      for ( ; *pSrc; pDest++, pSrc++ )
        *pDest = toupper(*pSrc);
      *pDest = '\x0';

      if(fnmatch(filespec,dup_d_name,FNM_NOESCAPE|FNM_PATHNAME|FNM_PERIOD)==0){
        *d_name = dir_entry->d_name;    /* retptr to the name location*/
        return 1;                       /* return success             */
      }
    }
    else {                              /* else compare normally      */
      if(fnmatch(filespec,dir_entry->d_name,FNM_NOESCAPE|FNM_PATHNAME|FNM_PERIOD)==0){
        *d_name = dir_entry->d_name;    /* retptr to the name location*/
        return 1;                       /* return success             */
      }
    }
  }
   }
 while((dir_entry = readdir(dir_handle)));/* while have entries       */
 return 0;                            /* no file found or EOF       */
}

/*****************************************************************
* Function:  LinFindNextDir(path, dir_handle, finfo, d_name)     *
*                                                                *
* Purpose:  This function finds the next dir in the directory    *
*           PATH pointed by DIR_HANDLE. All needed info is       *
*           returned via the FINFO struct and the D_NAME pointer.*
*                                                                *
* Note:  '?' is currently not supported. Add the impletmentation *
*        here !                                                  *
*****************************************************************/
int LinFindNextDir(
  const char * filespec,               /* filespec to search for     */
  const char * path,                   /* current path               */
  DIR *dir_handle,                     /* directory handle           */
  struct stat *finfo,                  /* return buf for the finfo   */
  char * *d_name,                      /* name of the file found     */
  size_t caseless)                     /* case insensitive matching  */
{
  struct dirent *dir_entry;            /* Directory entry            */
  char full_path[IBUF_LEN+1];
                                       /* strtok routine             */

 if(!(dir_entry = readdir(dir_handle)))/* get first entry           */
   return 0;                           /* no entry or EOF            */

 do{

                                       /* make full spec             */
    sprintf(full_path, "%s%s", path, dir_entry->d_name);
    lstat(full_path, finfo);           /* read the info about it     */

    if(S_ISDIR(finfo->st_mode)){       /* if it is a directory       */

      if (caseless) {                    /* if caseless search         */
        char dup_d_name[IBUF_LEN+1];     /* compare upper cased copy   */
        char * pDest = dup_d_name;        /* of the entry name          */
        char * pSrc  = dir_entry->d_name;

        for ( ; *pSrc; pDest++, pSrc++ )
          *pDest = toupper(*pSrc);
        *pDest = '\x0';

        if(fnmatch(filespec,dup_d_name,FNM_NOESCAPE|FNM_PATHNAME|FNM_PERIOD)==0){
          *d_name = dir_entry->d_name;    /* retptr to the name location*/
          return 1;                       /* return success             */
        }
      }
      else {                              /* else compare normally      */
        if(fnmatch(filespec,dir_entry->d_name,FNM_NOESCAPE|FNM_PATHNAME|FNM_PERIOD)==0){
          *d_name = dir_entry->d_name;    /* retptr to the name location*/
          return 1;                       /* return success             */
        }
      }
    }
   }
 while((dir_entry = readdir(dir_handle)));/* while have entries       */
 return 0;                            /* no file found or EOF       */
}

/*********************************************************************/
/* Function: size_t FormatFile(                                       */
/*                                                                   */
/* Purpose:  Returns the new file attribute, given the mask of       */
/*           attributes to be cleared/set and the current attribute  */
/*           settings.                                               */
/*                                                                   */
/*********************************************************************/

size_t FormatFile(
  RXTREEDATA   *ldp,                   /* Pointer to local data      */
  int          *smask,                 /* Mask of attributes to      */
                                       /* search for                 */
  int          *dmask,                 /* Mask of attributes to set  */
  size_t        options,               /* Search and output format   */
  struct stat  *finfo )                /* File info sturcture        */
{
  struct tm *timestamp;                /* Time info about the file   */
  char tp;                             /* type of the entry          */

  if (options&NAME_ONLY)               /* name only?                 */
    strcpy(ldp->Temp, ldp->truefile);  /* just copy it over          */
  else {
#ifdef AIX
    struct tm stTimestamp;               /* Time info about the file   */
    timestamp = localtime_r(&(finfo->st_mtime),&stTimestamp); /* get the time info */
#else
    timestamp = localtime(&(finfo->st_mtime));/* get the time info   */
#endif
    /* add long time format */
    if (options&LONG_TIME)             /* need the long time format? */
                                       /* format as such             */
      sprintf(ldp->Temp, "%4d-%02d-%02d %02d:%02d:%02d  %10lu  ",
        timestamp->tm_year+1900,
        timestamp->tm_mon+1,
        timestamp->tm_mday,
        timestamp->tm_hour,
        timestamp->tm_min,
        timestamp->tm_sec,
        finfo->st_size);
    else
    {
      if (options&EDITABLE_TIME)       /* need the "smushed" form?   */
                                       /* format as such             */
        sprintf(ldp->Temp, "%02d/%02d/%02d/%02d/%02d  %10lu  ",
          (timestamp->tm_year)%100,
          timestamp->tm_mon+1,
          timestamp->tm_mday,
          timestamp->tm_hour,
          timestamp->tm_min,
          finfo->st_size);
      else                             /* give the pretty form       */
        sprintf(ldp->Temp, "%2d/%02d/%02d  %2d:%02d%c  %10lu  ",
          timestamp->tm_mon+1,
          timestamp->tm_mday,
          (timestamp->tm_year)%100,
          (timestamp->tm_hour < 13?
          timestamp->tm_hour:
          (timestamp->tm_hour-12)),
          timestamp->tm_min,
          ((timestamp->tm_hour < 12 ||
          timestamp->tm_hour == 24)?'a':'p'),
          finfo->st_size);
    }

    /* find the type of the entry                                    */
    if(S_ISLNK(finfo->st_mode))
      tp = 'l';                        /* symbolic link              */
    else if(S_ISBLK(finfo->st_mode))
      tp = 'b';                        /* block device               */
    else if(S_ISCHR(finfo->st_mode))
      tp = 'c';                        /* character device           */
    else if(S_ISDIR(finfo->st_mode))
      tp = 'd';                        /* directory                  */
    else if(S_ISFIFO(finfo->st_mode))
      tp = 'p';                        /* FIFO                       */
    else if(S_ISSOCK(finfo->st_mode))
      tp = 's';                        /* socket                     */
    else
      tp = '-';                        /* regular file               */

                                       /* format the attributes now  */
    sprintf(ldp->Temp, "%s%c%c%c%c%c%c%c%c%c%c  %s", ldp->Temp,
      tp,
      (((finfo->st_mode)&S_IREAD)?'r':'-'),
      (((finfo->st_mode)&S_IWRITE)?'w':'-'),
      (((finfo->st_mode)&S_IEXEC)?'x':'-'),
      (((finfo->st_mode)&S_IRGRP)?'r':'-'),
      (((finfo->st_mode)&S_IWGRP)?'w':'-'),
      (((finfo->st_mode)&S_IXGRP)?'x':'-'),
      (((finfo->st_mode)&S_IROTH)?'r':'-'),
      (((finfo->st_mode)&S_IWOTH)?'w':'-'),
      (((finfo->st_mode)&S_IXOTH)?'x':'-'),
      ldp->truefile);
  }
                                       /* Place new string in Stem   */
  ldp->vlen = strlen(ldp->Temp);
  ldp->count++;
  sprintf(ldp->varname+ldp->stemlen, "%d", ldp->count);
  ldp->shvb.shvnext = NULL;
  ldp->shvb.shvname.strptr = ldp->varname;
  ldp->shvb.shvname.strlength = strlen(ldp->varname);
  ldp->shvb.shvvalue.strptr = ldp->Temp;
  ldp->shvb.shvvalue.strlength = ldp->vlen;
  ldp->shvb.shvnamelen = ldp->shvb.shvname.strlength;
  ldp->shvb.shvvaluelen = ldp->vlen;
  ldp->shvb.shvcode = RXSHV_SET;
  ldp->shvb.shvret = 0;

  int rc = RexxVariablePool(&ldp->shvb);
  if (rc & (RXSHV_BADN | RXSHV_MEMFL))
  {
    return INVALID_ROUTINE;
  }
  return 0;                            /* good return                */
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
*                    CASELESS    - Indicates do a case insensitive check for *
*                                   file names.                              *
*                                                                            *
*****************************************************************************/

int  RecursiveFindFile(
  const char *FileSpec,                /* Filespecs to search for    */
  const char *path,                    /* current directory          */
  RXTREEDATA *ldp,                     /* Pointer to local data      */
  int        *smask,                   /* Mask of attributes to      */
                                       /* search for                 */
  int        *dmask,                   /* Mask of attributes to set  */
  size_t       options )                /* Search and output format   */
                                       /* options                    */
{
  char  tempfile[MAX+1];               /* Used to hold temp file name*/
  DIR *dir_handle;                     /* Directory handle           */
  struct stat finfo;                   /* file information           */
  char * filename;
  size_t caseless = options&CASELESS;


  /********************************************************************
  * First, process all of the normal files, saving directories for    *
  * last.                                                             *
  ********************************************************************/

  if(!(dir_handle = opendir(path))){   /* get the directory handle   */
    return VALID_ROUTINE;              /* return  on non-zero        */
  }

  if (options&DO_FILES &&              /* if processing files        */
                                       /* and have some              */
      (LinFindNextFile(ldp->TargetSpec,path ,dir_handle,&finfo, &filename, caseless))) {
                                       /* Get the rest of the files  */
    do {
                                       /* build the full name        */
      sprintf(ldp->truefile, "%s%s", path, filename);
                                       /* passed back ok?            */
      if (FormatFile(ldp, smask, dmask, options, &finfo)) {
        closedir(dir_handle);          /* close the search           */
        return INVALID_ROUTINE;        /* error on non-zero          */
      }
    } while (LinFindNextFile(ldp->TargetSpec,path, dir_handle, &finfo, &filename, caseless));
  }
  closedir(dir_handle);                /* reset the                  */
  dir_handle = opendir(path);          /* directory handle (rewinddir*/
                                       /* doesn't work!)             */
  if (options&DO_DIRS  &&              /* need directories?          */
                                       /* and have some              */
      (LinFindNextDir(ldp->TargetSpec,path,dir_handle,&finfo,&filename, caseless))) {

    do {
                                       /* dot directory?             */
      if (!strcmp(filename, ".") || !strcmp(filename, ".."))
        continue;                      /* skip this one              */
                                       /* build the full name        */
        sprintf(ldp->truefile, "%s%s", path, filename);
                                       /* passed back ok?            */
        if (FormatFile(ldp, smask, dmask, options, &finfo)) {
          closedir(dir_handle);        /* close the search           */
          return INVALID_ROUTINE;      /* error on non-zero          */
      }
   } while (LinFindNextDir(ldp->TargetSpec,path, dir_handle, &finfo, &filename, caseless));
  }
  closedir(dir_handle);                /* reset the directory handle */
  dir_handle = opendir(path);          /* (rewinddir doesn't work!)  */
                                       /* need to recurse? and have  */
  if (options&RECURSE) {               /* some? no need for caseless */
                                       /* matching star              */
    if (LinFindNextDir("*",path,dir_handle,&finfo,&filename, 0)) {
      do {
                                       /* dot directory?             */
        if (!strcmp(filename, ".") ||
            !strcmp(filename, ".."))
          continue;                    /* skip this one              */
                                       /* build the new directory    */
        sprintf(tempfile, "%s%s/", path, filename);
                                       /* search the next level      */
        if (RecursiveFindFile(ldp->TargetSpec, tempfile, ldp,
            smask, dmask, options)) {
          closedir(dir_handle);        /* close the search           */
          return INVALID_ROUTINE;      /* error on non-zero          */
        }
      } while (LinFindNextDir("*",path,dir_handle,&finfo,&filename, 0));
    }
  }
  closedir(dir_handle);
  return VALID_ROUTINE;                /* finished                   */
}



/********************************************************************
* Function:  string2long(string, number)                            *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns false if the number *
*            is not valid, true if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        true - Good number converted                           *
*            false - Invalid number supplied.                       *
*********************************************************************/

bool string2int(
  const char * string,
  int  *number)
{
  int       accumulator;                /* converted number           */
  int      length;                     /* length of number           */
  int      sign;                       /* sign of number             */

  sign = 1;                            /* set default sign           */
  if (*string == '-') {                /* negative?                  */
    sign = -1;                         /* change sign                */
    string++;                          /* step past sign             */
  }

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS)             /* or too long                */
    return false;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return false;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator * 10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator * sign;        /* return the value           */
  return true;                         /* good number                */
}



void restore_terminal(int signal){

  stty(STDIN_FILENO,&in_orig);          /* restore the terminal settings        */
  raise(signal);                        /* propagate signal                     */
}

/******************************************************************************/
/* getkey                                                                     */
/******************************************************************************/
/* echo == false => no echo                                                   */
/* echo == true  => echo                                                      */
/******************************************************************************/
int getkey(char *ret, bool echo){
  /* restore original TTY settings on exit */

int ttyfd = STDIN_FILENO;              /* standard tty filedescriptor        */
/* Set the cleanup handler for unconditional process termination              */
struct sigaction new_action;


/* Set up the structure to specify the new action                             */
new_action.sa_handler = restore_terminal;
sigfillset(&new_action.sa_mask);
new_action.sa_flags = SA_RESTART;

/* Termination signals                                                        */
sigaction(SIGINT, &new_action, NULL);  /* exitClear on SIGINT signal          */
sigaction(SIGTERM, &new_action, NULL); /* exitClear on SIGTERM signal         */
sigaction(SIGQUIT, &new_action, NULL); /* exitClear on SIGQUIT signal         */
sigaction(SIGHUP, &new_action, NULL);  /* exitClear on SIGHUP signal          */
sigaction(SIGTSTP, &new_action, NULL); /* exitClear on SIGTSTP signal         */
sigaction(SIGTTIN, &new_action, NULL); /* exitClear on SIGTTIN signal         */
sigaction(SIGTTOU, &new_action, NULL); /* exitClear on SIGTTOU signal         */
/* Error signals                                                              */
sigaction(SIGSEGV, &new_action, NULL); /* exitClear on SIGSEGV signal         */
sigaction(SIGFPE, &new_action, NULL);  /* exitClear on SIGFPE signal          */
sigaction(SIGILL, &new_action, NULL);  /* exitClear on SIGILL signal          */
sigaction(SIGBUS, &new_action, NULL);  /* exitClear on SIGBUS signal          */
sigaction(SIGPIPE, &new_action, NULL); /* exitClear on broken pipe            */


  if( !isatty(ttyfd) ){                 /* connected to a terminal ?          */
    ret[0] = '\0';
    return 0;
  }
  ttyfd = STDIN_FILENO;                 /* STDIN_FILENO is out default fd     */

                                        /* open tty                           */
  ttyfd = open("/dev/tty", O_RDONLY);   /* get filedescriptor (fd) for tty    */

  struct termios in_raw;                /* global for save reset after SIGNAL */

  gtty(ttyfd, &in_orig);                /* determine existing tty settings */

  /* restore original TTY settings on exit */

  /* change STDIN settings to raw */
  gtty(ttyfd, &in_raw);                 /* save old settings                  */

  in_raw.c_lflag &= ~ICANON;            /* no canonical mode                  */
  if (!echo)                            /* no echo flag set                   */
    in_raw.c_lflag &= ~ECHO;            /* no echo                            */
  in_raw.c_cc[VMIN] = 1;                /* read 1 byte before returning       */
  in_raw.c_cc[VTIME] = 0;               /* return immediatly (no timeout)     */
  stty(ttyfd, &in_raw);                 /* execute settings now!              */


  ret[0] = getchar();                   /* read the char                      */

  ret[1] = '\0';                        /* terminate string                   */

  restore_tty(&in_orig);                /* for standard I/O behavior          */
  close(ttyfd);                         /* close the terminal                 */
  return 0;                             /* everything is fine                 */
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
  int   secs;                          /* Time to sleep in secs      */
  size_t length;                       /* length of the count        */
  const char * string;                 /* input sleep time           */
  int   nanoseconds = 0;               /* decimals value             */
  int   digits;                        /* number of decimal digits   */
#if defined( HAVE_NANOSLEEP )
  struct timespec    Rqtp, Rmtp;
#elif defined( HAVE_NSLEEP )
  struct timestruc_t Rqtp, Rmtp;
#endif
  int  nano;
  if (numargs != 1)                    /* Must have one argument     */
    return INVALID_ROUTINE;

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
  if (*string == '.') {                /* have a decimal number?     */
    string++;                          /* step over the decimal      */
    length--;                          /* reduce the length          */
    nanoseconds = 0;                   /* no nanoseconds yet         */
    digits = 0;                        /* and no digits              */
    nano = 9;

    while(nano)
    {
      while (length)
      {                   /* while more digits          */
        if (!isdigit(*string))           /* not a digit?               */
          return INVALID_ROUTINE;        /* not a valid number         */
                                       /* add to accumulator         */
        nanoseconds = nanoseconds * 10 + (*string - '0');
        length--;                        /* reduce length              */
        string++;                        /* step pointer               */
        nano--;
      }
      while(nano)
      {
        nanoseconds = nanoseconds * 10;
        nano--;
      }
    }
  }
  else if (length != 0)                /* invalid character found?   */
    return INVALID_ROUTINE;            /* this is invalid            */
#if defined( HAVE_NANOSLEEP )
  Rqtp.tv_sec = secs;
  Rqtp.tv_nsec = nanoseconds;
  nanosleep(&Rqtp, &Rmtp);
#elif defined( HAVE_NSLEEP )
  Rqtp.tv_sec = secs;
  Rqtp.tv_nsec = nanoseconds;
  nsleep(&Rqtp, &Rmtp);
#else
  sleep( secs );
#endif
  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* return no error            */
  return VALID_ROUTINE;                /* this worked ok             */
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


/**********************************************************************
* Function:  SysCls                                                   *
*                                                                     *
* Syntax:    call SysCls                                              *
*                                                                     *
* Return:    NO_UTIL_ERROR - Successful.                              *
**********************************************************************/

size_t RexxEntry SysCls(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  if (numargs)                         /* arguments specified?       */
    return INVALID_ROUTINE;            /* raise the error            */
  system("clear");                     /* do the clear               */             // think about the use of 'execve', Weigold
  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
  return VALID_ROUTINE;                /* no error on call           */
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

size_t RexxEntry SysAddRexxMacro(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode      rc;                      /* creation return code       */
  size_t       position;                /* added position             */

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
  rc = RexxAddMacro(args[0].strptr, args[1].strptr, position);
  sprintf(retstr->strptr, "%d", rc);   /* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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

size_t RexxEntry SysDropRexxMacro(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode      rc;                      /* creation return code       */

  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */

  rc = RexxDropMacro(args[0].strptr);  /* try to drop the macro      */
  sprintf(retstr->strptr, "%d", rc);   /* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
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

size_t RexxEntry SysClearRexxMacroSpace(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode      rc;                      /* creation return code       */

  if (numargs)                         /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
  rc = RexxClearMacroSpace();          /* clear the macro space      */
  sprintf(retstr->strptr, "%d", rc);   /* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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
size_t RexxEntry SysSaveRexxMacroSpace(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode      rc;                      /* creation return code       */

  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* clear the macro space      */
  rc = RexxSaveMacroSpace(0, NULL, args[0].strptr);
  sprintf(retstr->strptr, "%d", rc);   /* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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

size_t RexxEntry SysLoadRexxMacroSpace(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode      rc;                      /* creation return code       */

  if (numargs != 1)                    /* wrong number?              */
    return INVALID_ROUTINE;            /* raise error condition      */
                                       /* clear the macro space      */
  rc = RexxLoadMacroSpace(0, NULL, args[0].strptr);
  sprintf(retstr->strptr, "%d", rc);   /* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
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
size_t RexxEntry SysQueryRexxMacro(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  unsigned short position;             /* returned position          */

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
* Function:  SysReorderRexxMacro                                         *
*                                                                        *
* Syntax:    result = SysReorderRexxMacro(name, order)                   *
*                                                                        *
* Params:    name   - loaded name of the macro file                      *
*            order  - Either 'B'efore or 'A'fter.                        *
*                                                                        *
* Return:    return code from RexxReorderMacro                           *
*************************************************************************/
size_t RexxEntry SysReorderRexxMacro(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode      rc;                      /* creation return code       */
  size_t       position;                /* added position             */

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
  rc = RexxReorderMacro(args[0].strptr, position);
  sprintf(retstr->strptr, "%d", rc);   /* format the return code     */
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;                /* good completion            */
}


/*************************************************************************
* Function:  SysMkDir                                                    *
*                                                                        *
* Syntax:    call SysMkDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be created.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*                                                                        *
*************************************************************************/

size_t RexxEntry SysMkDir(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  size_t  rc;                           /* Ret code of func           */
  const char *  path;                   /* given path                 */
  char *  dir_buf = NULL;               /* full directory path        */

  if (numargs != 1)
                                       /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;
  path = args[0].strptr;               /* directory to make          */
                                       /* check for using '~/'       */
  if(*(path) == '~'){
    dir_buf = resolve_tilde(path);     /* get absolut path           */
    path = dir_buf;                    /* directory to make          */
  }
  /* Make the dir; standard permissions are rwxr-xr-x                */
  /* we do not restrict permission, this is done by root in the file */
  /* /etc/security/user. We allow anything. System restricts         */
  /* according to the user settings --> smitty/user                  */
//rc = mkdir(path, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
  rc = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
  if(!rc){                             /* if worked well             */
    sprintf(retstr->strptr, "%d", rc); /* result is return code      */
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }
  else{                                /* if there was an error      */
    switch (errno) {

      case EACCES: {
        sprintf(retstr->strptr, "%d", 5); /* result: Access denied   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case EEXIST: {
        sprintf(retstr->strptr, "%d", 87);/*result: already exists   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case EMLINK: {
        sprintf(retstr->strptr, "%d", 206);/* result: exceeds range  */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case ENOSPC: {
        sprintf(retstr->strptr, "%d", 206);/* result: exceeds range  */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case EROFS: {
        sprintf(retstr->strptr, "%d", 108);/* result:read only system*/
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      default:
        sprintf(retstr->strptr, "%d", 2);  /* result: file not found */
        retstr->strlength = strlen(retstr->strptr);
        break;
    }
  }
  if(dir_buf)                              /* did we allocate ?       */
    free(dir_buf);                         /* free the buffer memory  */
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysRmDir                                                    *
*                                                                        *
* Syntax:    call SysRmDir dir                                           *
*                                                                        *
* Params:    dir - Directory to be removed.                              *
*                                                                        *
* Return:    NO_UTIL_ERROR                                               *
*                                                                        *
*************************************************************************/

size_t RexxEntry SysRmDir(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  size_t  rc;                           /* Ret code of func           */
  const char *  path;                   /* given path                 */
  char *  dir_buf = NULL;               /* full directory path        */

  if (numargs != 1)
                                       /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;
  path = args[0].strptr;               /* directory to remove        */
                                       /* check for using '~/'       */
  if(*(path) == '~'){
    dir_buf = resolve_tilde(path);     /* get absolut path           */
    path = dir_buf;                    /* directory to remove        */
  }

  rc = rmdir(path);                    /* Remove the directory       */
  if(!rc){                             /* if worked well             */
    sprintf(retstr->strptr, "%d", rc); /* result is return code      */
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }
  else{                                /* if there was an error      */
    switch (errno) {

      case EACCES: {
        sprintf(retstr->strptr, "%d", 5); /* result: Access denied   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
#if defined( ENOTEMPTY ) && defined( EEXIST )
      case EEXIST: {
        sprintf(retstr->strptr, "%d", 87);/*result: already exists   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
#elif defined( ENOTEMPTY )
      case ENOTEMPTY: {                 /* sometimes used for EEXITST */
        sprintf(retstr->strptr, "%d", 87);/*result: already exists   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
#elif defined( EEXIST )
      case EEXIST: {
        sprintf(retstr->strptr, "%d", 87);/*result: already exists   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
#endif
      case EBUSY: {
        sprintf(retstr->strptr, "%d", 5);/* result: currently in use */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case ENOENT: {
        sprintf(retstr->strptr, "%d", 87);/* result: doesn't exitst  */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case EROFS: {
        sprintf(retstr->strptr, "%d", 108);/* result:read only system*/
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
   default:
        sprintf(retstr->strptr, "%d", 2);  /* result: dir not found  */
        retstr->strlength = strlen(retstr->strptr);
        break;
    }
  }
  if(dir_buf)                              /* did we allocate ?       */
    free(dir_buf);                         /* free the buffer memory  */
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

size_t RexxEntry SysFileDelete(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  size_t  rc;                           /* Ret code of func           */
  const char *  path;                   /* given path                 */
  char *  dir_buf = NULL;               /* full directory path        */

  if (numargs != 1)
                                       /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;
  path = args[0].strptr;               /* file to remove             */
                                       /* check for using '~/'       */
  if(*(path) == '~'){
    dir_buf = resolve_tilde(path);     /* get absolut path           */
    path = dir_buf;                    /* file to remove             */
  }

  rc = remove(path);                   /* Remove the file            */
  if(!rc){                             /* if worked well             */
    sprintf(retstr->strptr, "%d", rc); /* result is return code      */
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }
  else{                                /* if there was an error      */
    switch (errno) {

      case EACCES: {
        sprintf(retstr->strptr, "%d", 5); /* result: Access denied   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case EBUSY: {
        sprintf(retstr->strptr, "%d", 5);/* result: currently in use */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case ENOENT: {
        sprintf(retstr->strptr, "%d", 87);/* result: doesn't exist   */
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      case EROFS: {
        sprintf(retstr->strptr, "%d", 108);/* result:read only system*/
        retstr->strlength = strlen(retstr->strptr);
        break;
      }
      default:
        sprintf(retstr->strptr, "%d", 2);  /* result: file not found */
        retstr->strlength = strlen(retstr->strptr);
        break;
    }
  }
  if(dir_buf)                              /* did we allocate ?       */
    free(dir_buf);                         /* free the buffer memory  */
  return VALID_ROUTINE;
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
  const char *target;                  /* search string              */
  const char *file;                    /* search file                */
  const char *opts;                    /* option string              */
  const char *ptr;                     /* Pointer to char str found  */
  size_t      num = 0;                 /* Line number                */
  size_t      len;                     /* Length of string           */
  size_t      len2;                    /* Length of string           */
  bool        linenums = false;        /* Set true for linenums in   */
                                       /* output                     */
  bool        sensitive = false;       /* Set true for case-sens     */
                                       /* search                     */
  RXSTEMDATA  ldp;                     /* stem data                  */
  GetFileData filedata;                /* file read information      */
  char *       dir_buf = NULL;          /* directory buffer           */
  char *       line = NULL;             /* Line read from file        */

  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
                                       /* validate arguments         */
  if (numargs < 3 || numargs > 4 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;            /* raise an error             */

  target = args[0].strptr;             /* get target pointer         */
  file = args[1].strptr;               /* get file name              */

  if(*(file) == '~'){                  /* check for using '~/'       */
    dir_buf = resolve_tilde(file);     /* get absolut path           */
    file = dir_buf;                    /* full path to the file      */
  }
  if (numargs == 4) {                  /* process options            */
    opts = args[3].strptr;             /* point to the options       */
    if (strstr(opts, "N") || strstr(opts, "n"))
      linenums = true;

    if (strstr(opts, "C") || strstr(opts, "c"))
      sensitive = true;
  }
                                       /* Initialize data area       */
  ldp.count = 0;
  strcpy(ldp.varname, args[2].strptr);
  ldp.stemlen = args[2].strlength;
  strupr(ldp.varname);                 /* uppercase the name         */
  if (ldp.varname[ldp.stemlen-1] != '.')
    ldp.varname[ldp.stemlen++] = '.';
  if (OpenFile(file, &filedata)) {     /* open the file              */
    BUILDRXSTRING(retstr, ERROR_FILEOPEN);
    if(dir_buf)                        /* did we allocate ?          */
      free(dir_buf);                   /* free it                    */
    return VALID_ROUTINE;              /* finished                   */
  }

  line = (char *) malloc(4096 * sizeof(char));
                                       /* do the search...found lines*/
                                       /* are saved in stem vars     */
  while (!GetLine(line, MAX_LINE_LEN - 1, &filedata)) {
    len = strlen(line);
    num++;

    ptr = mystrstr(line, target, len, args[0].strlength, sensitive);
    if (ptr != '\0') {
      if (linenums) {
        sprintf(ldp.ibuf, "%d ", num);
        len2 = strlen(ldp.ibuf);
        memcpy(ldp.ibuf+len2, line, len < IBUF_LEN-len2 ? len : IBUF_LEN-len2);
        ldp.vlen = IBUF_LEN < len+len2 ? IBUF_LEN : len + len2;
      }
      else {
        memcpy(ldp.ibuf, line, len);
        ldp.vlen = len;
      }
      ldp.count++;
      sprintf(ldp.varname+ldp.stemlen, "%d", ldp.count);
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
        if(dir_buf)                    /* did we allocate ?          */
          free(dir_buf);               /* free it                    */
        free(line);
        return INVALID_ROUTINE;        /* error on non-zero          */
      }
    }
  }
  free(line);
  CloseFile(&filedata);                /* Close that file            */
                                       /* set stem.0 to lines read   */
  sprintf(ldp.ibuf, "%d", ldp.count);
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
  if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN){
    if(dir_buf)                        /* did we allocate ?          */
      free(dir_buf);                   /* free it                    */
    return INVALID_ROUTINE;            /* error on non-zero          */
  }
  if(dir_buf)                          /* did we allocate ?          */
    free(dir_buf);                     /* free it                    */
  return VALID_ROUTINE;                /* no error on call           */
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
  char     buf[IBUF_LEN]={0};          /* returned file name         */
  const char *      opts;                       /* option string              */
  int      SearchFlag;                 /* Search control variable    */
                                       /* default=search current 1st */
  int      ulRc;                       /* return value of SearchPath */


  SearchFlag = CURRENT_DIR_FIRST;      /* set default search flag    */
                                       /* validate arguments         */
  if (numargs < 2 || numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;

  if (numargs == 3) {                  /* process options            */

    opts = args[2].strptr;             /* point to the options       */
    if ((*opts == 'N') || (*opts == 'n'))
      SearchFlag = ENVIRONMENT_ONLY ;
                                       /* do not search current dir  */

    else if ((*opts == 'C') || (*opts == 'c'))
      SearchFlag = CURRENT_DIR_FIRST;
                                       /* search current 1st(default)*/
    else
      return INVALID_ROUTINE;          /* Invalid option             */
  }

  ulRc = SearchPath(SearchFlag, args[0].strptr, args[1].strptr,
                       (char *)buf, sizeof(buf));

  /* if environment variable could not be found, try again with      */
  /* uppercase name.                                                 */
  if (ulRc)
  {
    char *temp = strdup(args[0].strptr);
    strupr(temp);
    ulRc = SearchPath(SearchFlag, temp, args[1].strptr,
                         (char *)buf, sizeof(buf));
    free(temp);
  }

  BUILDRXSTRING(retstr, (const char *)buf);          /* pass back result           */
  return VALID_ROUTINE;
}
#ifdef LINUX
/*************************************************************************
* Function:  SysLinVer                                                   *
*                                                                        *
* Syntax:    call SysLinVer                                              *
*                                                                        *
* Return:    Linux Version                                               *
*************************************************************************/

size_t RexxEntry SysLinVer(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  struct utsname info;                 /* info structur              */

  if (numargs != 0)                    /* validate arg count         */
    return INVALID_ROUTINE;

  if(uname(&info) < 0)                   /* if no info stored          */
    return INVALID_ROUTINE;            /* get out                    */

  sprintf(retstr->strptr, "%s %s",info.sysname,info.release);
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}
#endif

/*************************************************************************
* Function:  SysVersion                                                  *
*                                                                        *
* Syntax:    call SysVersion                                             *
*                                                                        *
* Return:    Operating System name (LINUX/AIX/WINDOWS) and Version       *
*************************************************************************/

size_t RexxEntry SysVersion(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  struct utsname info;                 /* info structur              */

  if (numargs != 0)                    /* validate arg count         */
    return INVALID_ROUTINE;

  if(uname(&info) < 0)                 /* if no info stored          */
    return INVALID_ROUTINE;            /* get out                    */

  sprintf(retstr->strptr, "%s %s.%s",info.sysname, info.version, info.release);

  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}


/*************************************************************************
* Semaphore data struct                                                  *
*************************************************************************/

typedef struct RxSemData {
    bool          named;               /* Named semaphore?           */
    sem_t *       handle;              /* Semaphore pointer          */
} RXSEMDATA;


/*************************************************************************
* Function:  SysCreateEventSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateEventSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a event semaphore                 *
*                                                                        *
* Return:    handle - token used as a event sem handle for               *
*                     SysPostEventSem, SysClearEventSem,                 *
*                     SysCloseEventSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

RexxRoutine2(uintptr_t, SysCreateEventSem, OPTIONAL_CSTRING, name, OPTIONAL_CSTRING, reset)
{
    RXSEMDATA *semdata;
    int rc;

    // Note that the reset arg has no meaning on Unix/Linux and is unused.
    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL) {
        return 0;
    }
    if (name == NULL) {
        /* this is an unnamed semaphore */
        semdata->handle = (sem_t *)malloc(sizeof(sem_t));
        rc = sem_init(semdata->handle, 0, 0);
        if (rc == -1) {
            free(semdata);
            return 0;
        }
        semdata->named = false;
    }
    else {
        /* this is a named semaphore */
        sem_unlink(name);
        semdata->handle = sem_open(name, (O_CREAT | O_EXCL), (S_IRWXU | S_IRWXG), 0);
        if (semdata->handle == SEM_FAILED ) {
            free(semdata);
            return 0;
        }
        semdata->named = true;
    }
    return (uintptr_t)semdata;
}


/*************************************************************************
* Function:  SysOpenEventSem                                             *
*                                                                        *
* Syntax:    result = SysOpenEventSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosOpenEventSem                   *
*************************************************************************/

RexxMethod1(uintptr_t, SysOpenEventSem, CSTRING, name)
{
    RXSEMDATA *semdata;

    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL) {
        return 0;
    }
    semdata->handle = sem_open(name, 0);
    if (semdata->handle == SEM_FAILED ) {
        return 0;
    }
    semdata->named = true;
    return (uintptr_t)semdata->handle;
}


/*************************************************************************
* Function:  SysResetEventSem                                            *
*                                                                        *
* Syntax:    result = SysResetEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosResetEventSem                  *
*************************************************************************/

RexxMethod1(int, SysResetEventSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;

    sem_init(semdata->handle, 1, 0);
    return 0;
}


/*************************************************************************
* Function:  SysPostEventSem                                             *
*                                                                        *
* Syntax:    result = SysPostEventSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosPostEventSem                   *
*************************************************************************/

RexxMethod1(int, SysPostEventSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc;

    rc = sem_post(semdata->handle);
    if (rc) {
        return 6;
    }
    return 0;
}


/*************************************************************************
* Function:  SysCloseEventSem                                            *
*                                                                        *
* Syntax:    result = SysCloseEventSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateEventSem              *
*                                                                        *
* Return:    result - return code from DosCloseEventSem                  *
*************************************************************************/

RexxMethod1(int, SysCloseEventSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;

    if (semdata->named == false) {
        /* this is an unnamed semaphore so we must free the target */
        if (sem_destroy(semdata->handle)) {
            if (errno == EINVAL) {
                return 6;
            }
            else if (errno) {
                return 102;
            }
        }
    }
    else {
        /* this is a named semaphore */
        if (sem_close(semdata->handle)) {
            if (errno == EINVAL) {
                return 6;
            }
            else if (errno) {
                return 102;
            }
        }
    }
    free(semdata);
    return 0;
}


#define SEM_WAIT_PERIOD 100 /* POSIX says this should be 10ms */


/*************************************************************************
* Function:  SysWaitEventSem                                             *
*                                                                        *
* Syntax:    result = SysWaitEventSem(handle, <timeout>)                 *
*                                                                        *
* Params:    handle - token returned from SysWaitEventSem                *
*                                                                        *
* Return:    result - return code from DosWaitEventSem                   *
*************************************************************************/

RexxRoutine2(int, SysWaitEventSem, uintptr_t, vhandle, OPTIONAL_int, timeout)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc = 0;

    if (timeout != 0) {
        /* this looping construct will cause us to wait longer than the */
        /* specified timeout due to the latency involved in the loop,   */
        /* but that cannot be helped                                    */
        while (timeout > 0) {
            rc = sem_trywait(semdata->handle);
            if (rc == 0) {
                break;
            }
            if (usleep(SEM_WAIT_PERIOD * 1000)) {
                timeout -= SEM_WAIT_PERIOD;
            }
        }
    }
    else {
        rc = sem_wait(semdata->handle);
    }
    if (rc) {
        if (errno == EAGAIN) {
            return 121;
        }
        else if (errno == EINVAL) {
            return 6;
        }
    }
    return 0;
}


/*************************************************************************
* Function:  SysCreateMutexSem                                           *
*                                                                        *
* Syntax:    handle = SysCreateMutexSem(<name>)                          *
*                                                                        *
* Params:    name  - optional name for a event semaphore                 *
*                                                                        *
* Return:    handle - token used as a event sem handle for               *
*                     SysPostEventSem, SysClearEventSem,                 *
*                     SysCloseEventSem, and SysOpenEventSem              *
*            '' - Empty string in case of any error                      *
*************************************************************************/

RexxRoutine1(uintptr_t, SysCreateMutexSem, OPTIONAL_CSTRING, name)
{
    RXSEMDATA *semdata;
    int rc;

    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL) {
        return 0;
    }
    if (strlen(name) == 0) {
        /* this is an unnamed semaphore */
        semdata->handle = (sem_t *)malloc(sizeof(sem_t));
        rc = sem_init(semdata->handle, 0, 0);
        if (rc == -1) {
            free(semdata);
            return 0;
        }
        semdata->named = false;
    }
    else {
        /* this is a named semaphore */
        sem_unlink(name);
        semdata->handle = sem_open(name, (O_CREAT | O_EXCL), (S_IRWXU | S_IRWXG), 0);
        if (semdata->handle == SEM_FAILED ) {
            free(semdata);
            return 0;
        }
        semdata->named = true;
    }
    rc = sem_post(semdata->handle);
    return (uintptr_t)semdata;
}


/*************************************************************************
* Function:  SysOpenMutexSem                                             *
*                                                                        *
* Syntax:    result = SysOpenMutexSem(handle)                            *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from DosOpenEventSem                   *
*************************************************************************/

RexxRoutine1(uintptr_t, SysOpenMutexSem, CSTRING, name)
{
    RXSEMDATA *semdata;

    semdata = (RXSEMDATA *)malloc(sizeof(RXSEMDATA));
    if (semdata == NULL) {
        return 0;
    }
    semdata->handle = sem_open(name, 0);
    if (semdata->handle == SEM_FAILED ) {
        return 0;
    }
    semdata->named = true;
    return (uintptr_t)semdata->handle;
}


/*************************************************************************
* Function:  SysRequestMutexSem                                          *
*                                                                        *
* Syntax:    result = SysRequestMutexSem(handle, <timeout>)              *
*                                                                        *
* Params:    handle - token returned from SysRequestMutexSem             *
*                                                                        *
* Return:    result - return code from DosWaitEventSem                   *
*************************************************************************/

RexxRoutine2(int, SysRequestMutexSem, uintptr_t, vhandle, OPTIONAL_int, timeout)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc = 0;

    if (timeout != 0) {
        /* this looping construct will cause us to wait longer than the */
        /* specified timeout due to the latency involved in the loop,   */
        /* but that cannot be helped                                    */
        while (timeout > 0) {
            rc = sem_trywait(semdata->handle);
            if (rc == 0) {
                break;
            }
            if (usleep(SEM_WAIT_PERIOD * 1000)) {
                timeout -= SEM_WAIT_PERIOD;
            }
        }
    }
    else {
        rc = sem_wait(semdata->handle);
    }
    if (rc) {
        if (errno == EAGAIN) {
            return 121;
        }
        else if (errno == EINVAL) {
            return 6;
        }
    }
    return 0;
}


/*************************************************************************
* Function:  SysReleaseMutexSem                                          *
*                                                                        *
* Syntax:    result = SysReleaseMutexSem(handle)                         *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from DosCloseEventSem                  *
*************************************************************************/

RexxRoutine1(int, SysReleaseMutexSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;
    int rc;
    int val;

    rc = sem_getvalue(semdata->handle, &val);
    if (rc) {
        if (errno == EINVAL) {
            return 6;
        }
        else {
            return 288;
        }
    }
    if (val == 0) {
        rc = sem_post(semdata->handle);
        if (rc) {
            return 6;
        }
    }
    return 0;

}


/*************************************************************************
* Function:  SysCloseMutexSem                                            *
*                                                                        *
* Syntax:    result = SysCloseMutexSem(handle)                           *
*                                                                        *
* Params:    handle - token returned from SysCreateMutexSem              *
*                                                                        *
* Return:    result - return code from DosCloseEventSem                  *
*************************************************************************/

RexxRoutine1(int, SysCloseMutexSem, uintptr_t, vhandle)
{
    RXSEMDATA *semdata = (RXSEMDATA *)vhandle;

    if (semdata->named == false) {
        /* this is an unnamed semaphore so we must free the target */
        if (sem_destroy(semdata->handle)) {
            if (errno == EINVAL) {
                return 6;
            }
            else if (errno) {
                return 102;
            }
        }
    }
    else {
        /* this is a named semaphore */
        if (sem_close(semdata->handle)) {
            if (errno == EINVAL) {
                return 6;
            }
            else if (errno) {
                return 102;
            }
        }
    }
    free(semdata);
    return 0;
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
  char        FileSpec[IBUF_LEN];      /* File spec to look for      */
  char        path[IBUF_LEN];          /* path to search along       */
  const char *optptr;                  /* option scan pointer        */
  size_t       options;                /* Mask of options            */
  int         smask[5];                /* Source attribute mask      */
  int         dmask[5];                /* Target attribute mask      */
  RXTREEDATA  ldp;                     /* local data                 */
  char *       temp;

  options = DO_FILES|DO_DIRS;          /* Clear if we should not     */
                                       /* display files              */
  smask[0] = IGNORE;                   /* No mask unless specified   */
  dmask[0] = IGNORE;                   /* No mask unless specified   */
  path[0] = '\0';                      /* no path yet                */
  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */

                                       /* validate arguments         */
  if (numargs < 2 || numargs > 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      args[0].strlength > 255)
    return INVALID_ROUTINE;            /* Invalid call to routine    */
                                       /* initialize data area       */
  ldp.count = 0;
  strcpy(ldp.varname, args[1].strptr);
  ldp.stemlen = args[1].strlength;
  /* uppercase the name  */
  strupr(ldp.varname);

  if (ldp.varname[ldp.stemlen-1] != '.')
    ldp.varname[ldp.stemlen++] = '.';

  strcpy(FileSpec, args[0].strptr);    /* get file spec              */

  /* if filespec is '*' then use './ *'                               */
  if((strlen(FileSpec)==1)&&(FileSpec[0] == '*'))
    strcpy(FileSpec, "./*");

   /** If FileSpec ends in / then append  *                          */

  if (FileSpec[strlen(FileSpec)-1] == '/')
    strcat(FileSpec, "*");

  /* if filespec begins with '~' generate absolute path              */
  if(FileSpec[0] == '~'){
    if(!(temp = resolve_tilde(FileSpec)))/* get the absolute path    */
      return INVALID_ROUTINE;
    strcpy(FileSpec,temp);             /* work with the new one      */
    free(temp);                        /* free temp space            */
  }

  if (numargs >= 3 &&                  /* check third option         */
      !RXNULLSTRING(args[2])) {
    if (!args[2].strlength)            /* a zero length string isn't */
      return INVALID_ROUTINE;          /* valid                      */
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
          options |= CASELESS;         /* do caseless check          */
          break;

        default:                       /* unknown option             */
          return INVALID_ROUTINE;      /* raise an error             */
      }
      optptr++;                        /* step the option pointer    */
    }
  }
                                       /* get path and name          */
  getpath(FileSpec, path, ldp.TargetSpec);

  if ( options & CASELESS ) {          /* if caseless upper case     */
    char * p = ldp.TargetSpec;          /* file name portion now      */
    for ( ; *p; ++p ) {
      *p = toupper(*p);
    }
  }
                                       /* recursively search         */
  if (RecursiveFindFile(FileSpec, path, &ldp, smask, dmask, options))
    return INVALID_ROUTINE;
                                       /* return lines read          */
  sprintf(ldp.Temp, "%d", ldp.count);
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
  if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
    return INVALID_ROUTINE;            /* error on non-zero          */

  return VALID_ROUTINE;                /* no error on call           */
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
  char   filler;                       /* filler character           */
  char numstr[6];
  size_t  num, max;
  char   *array;
  char   *dir;                         /* the directory              */
  char   *file;                        /* the file prefix            */
  char *tmp;                           /* temporary                  */
  int x, j, i;
  j = 0;                                     /* get a copy of the argument */
  i = 0;
  max = 1;

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
  dir = (char*) malloc(args[0].strlength+1);
  if (dir == NULL){                    /* if something went wrong    */
    BUILDRXSTRING(retstr, ERROR_NOMEM);
    return VALID_ROUTINE;
  }

  strcpy(dir, args[0].strptr);        /* copy the string             */

  tmp = dir;                          /* set temporary               */

/* search for filter        *********/

  for (x = 0; tmp[x] != 0; x++)
  {
    if (tmp[x] == filler)
    {
      max = max *10;
      j++;
    }
  }

  if (j)
  {
    srand(time(0));
    num = rand();
    num = num % max;

    switch (j)
    {
       case 1:
              sprintf(numstr, "%01u", num);
              break;
       case 2:
              sprintf(numstr, "%02u", num);
              break;
       case 3:
              sprintf(numstr, "%03u", num);
              break;
       case 4:
              sprintf(numstr, "%04u", num);
              break;
       case 5:
              sprintf(numstr, "%05u", num);
              break;
       default:
              return INVALID_ROUTINE;            /* raise error condition */
    }                                            /* for compatibility     */

    for (x = 0; tmp[x] !=0; x++)
    {
      if (tmp[x] == filler)
      {
        tmp[x] = numstr[i++];
      }
    }
  }                                         /* if we need the filler */

  while(*tmp != 0 )                         /* lets start at the end */
  {
    tmp++;
  }

  while((*tmp != '/') && (*tmp != '\\') && (tmp > dir))
  {
    --tmp;
  }

  if(tmp == dir)
  {                                 /* directory string is '' or '/' */
    if(*dir == '\\'){
      file = dir+1;
      array = tempnam(NULL,file); /* call system routine    */
    }
    else if(*dir == '/'){
      file = dir+1;
      array = tempnam("/",file); /* call system routine     */
    }
    else{
      file = dir;
      array = tempnam(NULL,file);/* call system routine     */
    }
  }
  else
  {                                /* directory string exists    */
    file = tmp+1;                      /* set filename prefix        */
    *tmp = '\0';                       /* terminate directory string */
    array = tempnam(dir,file);/* call system routine        */
  }

  if(strlen(array) > 255)
  {
     free((char *)retstr->strptr);
     retstr->strptr = (char *) malloc(strlen(array) + 1);
  }

  strcpy(retstr->strptr, array);
                                       /* correct string length      */
  retstr->strlength = strlen(retstr->strptr);

  free (array);
  free(dir);                           /* free local string          */

  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysSetPriority                                              *
*                                                                        *
* Syntax:    result = SysSetPriority(Class, Level)                       *
*                                                                        *
* Params:    Class  - The priority class (0-4)                           *
*            Level  - Amount to change (-31 to +31)                      *
*                     (lower to higher priority)                         *
* Return:    0    for correct execution                                  *
*            304  for ERROR_INVALID_PDELTA                               *
*            307  for ERROR_INVALID_PCLASS                               *
*            derived from:                                               *
*            result - return code from DosSetPriority                    *
*                                                                        *
*************************************************************************/

size_t RexxEntry SysSetPriority(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  int       pclass;                    /* priority class                      */
  int       level;                     /* priority level                      */
  RexxReturnCode    rc;                        /* creation return code                */

  if (numargs != 2 ||                  /* must have two                       */
      !RXVALIDSTRING(args[0]))         /* first is omitted                    */
    return INVALID_ROUTINE;            /* raise error condition               */
                                       /* get class of change                 */

  if(!string2int(args[0].strptr,&pclass) || /* set the value for pclass      */
     !string2int(args[1].strptr,&level))    /* set the value for level       */
    return INVALID_ROUTINE;

  if (pclass == 0){                     /* class 0 -> no change               */
    rc = 0;                             /* no error                           */
  }
                                        /* change the priority                */
                                        /* change according to delta          */
  else if (((size_t)pclass > 0) && ((size_t)pclass <= 4)){
    int pid;                            /* PID                                */
    pid = getpid();                     /* current PID                        */

    int priority;                       /* Priority                           */
                                        /* current priority                   */
    priority = getpriority(PRIO_PROCESS, getpid());

                                        /* Set new priority                   */
    setpriority(PRIO_PROCESS, getpid(),-level);
    rc = 0;
  }

  else{
    rc = 307;
    return INVALID_ROUTINE;             /* raise error condition              */
  }

  sprintf(retstr->strptr, "%d", rc);   /* format the return code              */
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;                /* good completion                     */
}



/*************************************************************************
* Function:  SysGetMessage                                               *
*                                                                        *
* Syntax:    call SysGetMessage msgnum [,file] [,str1]...[,str9]         *
*                                                                        *
* Params:    file           - Name of message file to get message from.  *
*                              Default is OSO001.MSG.                    *
*            msgnum         - Number of message being queried.           *
*            str1 ... str9  - Insertion strings.  For messages which     *
*                              contain %1, %2, etc, the str1, str2, etc  *
*                              strings will be inserted (if given).      *
*                                                                        *
* Return:    The message with the inserted strings (if given).           *
* Note:      The set number ist always 1. Therefore the interface        *
*            remains the same as in OS/2 and Win.                        *
*            Reason: keep portability                                    *
*************************************************************************/

size_t RexxEntry SysGetMessage(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  int  msgnum;                         /* Message number to get      */
  int setnum = 1;                      /* Set number (const 1)       */
#if defined( HAVE_CATOPEN )
  nl_catd catalog;                     /* catalog handle             */
#endif
                                       /* default error msg          */
  const char default_message[] = {"Error: Message catalog not open !\0"};
                                       /* msg not found  msg         */
  const char not_found_message[] = {"Error: Message not found !\0"};
                                       /* insertion error  msg       */
  const char error_insertions[] = {"Error: Unable to generate message \
                              (wrong insertions)\0"};
                                       /* cat not found  msg         */
  const char cat_not_found_message[] = {"Error: Message catalog not found !\0"};

  const char * msg;                    /* msg retrieved from catalog */
  int icount;                          /* number of insertions       */
  int msg_length = 0;                  /* length of the return msg   */
  const char * msgfile;                /* name of the message file   */
  char * temp;
  int count = 0;                       /* number of '%s' in the msg  */


  if (numargs < 1 || numargs > 11 ||   /* validate arguments         */
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;            /* exit with error            */

                                       /* get message number         */
  if (!string2int(args[0].strptr, &msgnum) || msgnum < 0)
    return INVALID_ROUTINE;            /* exit with error            */

                                       /* Get message file name.     */
                                       /* Use "rexx.cat if not        */
                                       /* given                      */
  if (numargs >= 2 && RXVALIDSTRING(args[1]))
    msgfile = args[1].strptr;          /* use provided message file  */
  else
    msgfile = REXXMESSAGEFILE;

#if defined( HAVE_SETLOCALE )
  setlocale(LC_ALL, "en_US");
#endif

#if defined( HAVE_CATOPEN )
                                       /* open the catalog           */
  if((catalog = catopen(msgfile, NL_CAT_LOCALE)) == (nl_catd)-1){
    retstr->strptr = (char *)malloc(strlen(cat_not_found_message)+1);
    strcpy(retstr->strptr, cat_not_found_message);
    retstr->strlength = strlen(cat_not_found_message);
    return VALID_ROUTINE;
  }

                                       /* retrieve msg from catalog  */
  msg = catgets(catalog, setnum, (int)msgnum, default_message);

  if(*msg == '\0')                     /* if empty string returned   */
    msg = not_found_message;           /* it means msg not found     */

                                       /* set number of insertions   */
  if (numargs >= 2)
    icount = numargs-2;
  else
    icount = 0;

  /* calculate length of the return message */
  for(int j=2; j < icount+2; j++)
    msg_length += args[j].strlength;
  msg_length += strlen(msg);
  msg_length -= icount*2;

  /* alloc needed space for the return message (add 100 for default msgs)  */
  if(!(retstr->strptr = (char *)malloc(msg_length+100))){
     BUILDRXSTRING(retstr, ERROR_NOMEM);
      catclose(catalog);
      return VALID_ROUTINE;
  }

  /* check for too much '%s' in the message                          */
  temp = const_cast<char *>(msg);
  /* replace all &1..&9 with %s                                         */
  while((temp = strstr(temp, "&"))){
    if(isdigit(*(temp+1))){             /* replace &1..&9 ?             */
      *(temp++) = '%';
      *(temp++) = 's';                  /* %s expected                  */
      //      count++;
    }
    else
      temp++;
  }
  /* now look for number of replacement variables                       */
  temp = const_cast<char *>(msg);        /* reset temp pointer          */
  while((temp = strstr(temp,"%s"))){     /* search for the %s           */
    count ++;                            /* increment counter           */
    temp += 2;                           /* jump over %s                */
  }
  if(count > icount)
    icount = 10;                         /* go to error case         */

  /* generate full message with insertions                           */
  switch(icount){
    case(1):{
      if(sprintf(retstr->strptr, msg,args[2].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(2):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(3):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(4):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(5):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(6):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(7):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(8):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr,
                                     args[9].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(9):{
      if(sprintf(retstr->strptr, msg,args[2].strptr,
                                     args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr,
                                     args[9].strptr,
                                     args[10].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(10):{
      strcpy(retstr->strptr, error_insertions); /* error case        */
      break;
    }
    default:{
      strcpy(retstr->strptr, msg);
      break;
    }
  }
  retstr->strlength = strlen(retstr->strptr);/* insert the length    */

  catclose(catalog);                   /* close the catalog          */
  return VALID_ROUTINE;                /* no error on call           */
#else
                                       /* catopen not supported      */
  const char cat_not_supported_message[] = {"Error: Message catalog (catopen) not supported !\0"};

  retstr->strptr = (char *)malloc(strlen(cat_not_supported_message)+1);
  strcpy(retstr->strptr, cat_not_supported_message);
  retstr->strlength = strlen(cat_not_supported_message);
  return VALID_ROUTINE;
#endif
}



/*************************************************************************
* Function:  SysGetMessageX                                              *
*                                                                        *
* Syntax:    call SysGetMessageX setnum, msgnum [,file] [,str1]...[,str9]*
*                                                                        *
* Params:    file           - Name of message file to get message from.  *
*                              Default is OSO001.MSG.                    *
*            msgnum         - Number of message being queried.           *
*            str1 ... str9  - Insertion strings.  For messages which     *
*                              contain %1, %2, etc, the str1, str2, etc  *
*                              strings will be inserted (if given).      *
*            setnum         - set number in the catalog                  *
*                                                                        *
* Return:    The message with the inserted strings (if given).           *
* Note:      This is a special Unix only version of SysGetMessage which  *
*            supports the selection of a set in the msg catalog.         *
*************************************************************************/

size_t RexxEntry SysGetMessageX(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  int  msgnum;                         /* Message number to get      */
  int  setnum;                         /* Set number                 */
#if defined( HAVE_CATOPEN )
  nl_catd catalog;                     /* catalog handle             */
#endif
                                       /* default error msg          */
  const char default_message[] = {"Error: Message catalog not open !\0"};
                                       /* msg not found  msg         */
  const char not_found_message[] = {"Error: Message not found !\0"};
                                       /* insertion error  msg       */
  const char error_insertions[] = {"Error: Unable to generate message \
                              (wrong insertions)\0"};
                                       /* cat not found  msg         */
  const char cat_not_found_message[] = {"Error: Message catalog not found !\0"};

  char * msg;                           /* msg retrieved from catalog */
  int icount;                          /* number of insertions       */
  int msg_length = 0;                  /* length of the return msg   */
  const char * msgfile;                /* name of the message file   */
  char * temp;
  int count = 0;                       /* number of '%s' in the msg  */


  if (numargs < 1 || numargs > 12 ||   /* validate arguments         */
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;            /* exit with error            */

                                       /* get set number             */
  if (!string2int(args[0].strptr, &setnum) || setnum < 0)
    return INVALID_ROUTINE;            /* exit with error            */

                                       /* get message number         */
  if (!string2int(args[1].strptr, &msgnum) || msgnum < 0)
    return INVALID_ROUTINE;            /* exit with error            */

                                       /* Get message file name.     */
                                       /* Use "rexx.cat if not        */
                                       /* given                      */
  if (numargs >= 3 && RXVALIDSTRING(args[1]))
    msgfile = args[2].strptr;          /* use provided message file  */
  else
    msgfile = REXXMESSAGEFILE;

#if defined( HAVE_CATOPEN )
                                       /* open the catalog           */
  if((catalog = catopen(msgfile, NL_CAT_LOCALE)) == (nl_catd)-1){
    retstr->strptr = (char *)malloc(strlen(cat_not_found_message)+1);
    strcpy(retstr->strptr, cat_not_found_message);
    retstr->strlength = strlen(cat_not_found_message);
    return VALID_ROUTINE;
  }

                                       /* retrieve msg from catalog  */
  msg = catgets(catalog, setnum, msgnum, default_message);

  if(*msg == '\0')                     /* if empty string returned   */
    msg = const_cast<char *>(not_found_message);  /* it means msg not found     */

                                       /* set number of insertions   */
  if (numargs >= 3)
    icount = numargs-3;
  else
    icount = 0;

  /* calculate length of the return message */
  for(int j=3; j < icount+3; j++)
    msg_length += args[j].strlength;
  msg_length += strlen(msg);
  msg_length -= icount*2;

  /* alloc needed space for the return message (add 100 for default msgs)  */
  if(!(retstr->strptr = (char *)malloc(msg_length+100))){
     BUILDRXSTRING(retstr, ERROR_NOMEM);
      catclose(catalog);
      return VALID_ROUTINE;
  }

  /* check for to much '%s' in the message                           */
  temp = msg;
  /* replace all &1..&9 with %s                                         */
  while((temp = strstr(temp, "&"))){
    if(isdigit(*(temp+1))){             /* replace &1..&9 ?             */
      *(temp++) = '%';
      *(temp++) = 's';                  /* %s expected                  */
    }
    else
      temp++;
  }
  /* now look for number of replacement variables                       */
  temp = msg;                            /* reset temp pointer          */
  while((temp = strstr(temp,"%s"))){     /* search for the %s           */
    count++;                             /* increment counter           */
    temp += 2;                           /* jump over %s                */
  }

  if(count > icount)
    icount = 10;                         /* go to error case         */

  /* generate full message with insertions                           */
  switch(icount){
    case(1):{
      if(sprintf(retstr->strptr, msg,args[3].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(2):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(3):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(4):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(5):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(6):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(7):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr,
                                     args[9].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(8):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr,
                                     args[9].strptr,
                                     args[10].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(9):{
      if(sprintf(retstr->strptr, msg,args[3].strptr,
                                     args[4].strptr,
                                     args[5].strptr,
                                     args[6].strptr,
                                     args[7].strptr,
                                     args[8].strptr,
                                     args[9].strptr,
                                     args[10].strptr,
                                     args[11].strptr) != msg_length)
        strcpy(retstr->strptr, error_insertions);
      break;
    }
    case(10):{
      strcpy(retstr->strptr, error_insertions); /* error case        */
      break;
    }
    default:{
      strcpy(retstr->strptr, msg);
      break;
    }
  }
  retstr->strlength = strlen(retstr->strptr);/* insert the length    */

  catclose(catalog);                   /* close the catalog          */
  return VALID_ROUTINE;                /* no error on call           */
#else
                                       /* catopen not supported      */
  const char cat_not_supported_message[] = {"Error: Message catalog (catopen) not supported !\0"};

  retstr->strptr = (char *)malloc(strlen(cat_not_supported_message)+1);
  strcpy(retstr->strptr, cat_not_supported_message);
  retstr->strlength = strlen(cat_not_supported_message);
  return VALID_ROUTINE;
#endif
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
  bool      echo = true;               /* Set to false if we         */
                                       /* shouldn't echo             */
  if (numargs > 1)                     /* too many arguments         */
    return INVALID_ROUTINE;            /* raise an error             */

  if (numargs == 1) {                  /* validate arguments         */
    if (!strcasecmp(args[0].strptr, "NOECHO"))
      echo = false;
    else if (strcasecmp(args[0].strptr, "ECHO"))
      return INVALID_ROUTINE;          /* Invalid option             */
  }

  getkey(retstr->strptr,echo);         /* call the complicated part  */
  retstr->strlength = strlen(retstr->strptr); /* format string       */

  return VALID_ROUTINE;                /* no error on call           */
}


#ifdef AIX_DISABLED
/*************************************************************************
* Function:  SysAddFuncPkg                                               *
*                                                                        *
* Description:   Function to enable load of old CREXX libraries for AIX. *
*                The Path must be fully qualified in REXX procedure,     *
*                it is used to select the old >load< function call.      *
*                                                                        *
* Syntax:    call SysAddFuncPkg ( /Fully_qualified_path/Package_name )   *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

size_t RexxEntry SysAddFuncPkg(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode rc = NULL;
  int    j;                            /* Counter                    */
  size_t arglength;                    /* length of the count        */
  char *  argstring;                    /* input sleep time           */
  RXFUNCBLOCK *funcblock;              /* Base for function blocks   */
  PRXINITFUNCPKG InitFunc;             /* Pointer returned from load */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs != 1)                    /* Must have one argument     */
    return INVALID_ROUTINE;

  argstring = args[0].strptr;             /* point to the string        */
  arglength = args[0].strlength;          /* get length of string       */
  if ( (arglength == 0 ) ||               /* if null string             */
       (arglength >= MAXNAME) ||          /* or too long                */
       (!strchr( argstring, '/' )) )
    return INVALID_ROUTINE;               /* not valid                  */

  if (!(InitFunc = (PRXINITFUNCPKG)load(argstring,0,NULL)))
  {                                          /*  loadAndInit? for load  */
    if ( InitFunc == NULL ) {
      fprintf(stderr, " *** Unable to load library %s !\nError message: errno = %d;",\
               argstring, errno);
      perror(" REXXUTIL");
      rc = 1;
    }
  }
  if ( rc == NULL ) {
    /* Call the initialization routine for the library (which should */
    /* be the function pointer returned to us by load).              */
    rc = (*InitFunc)(&funcblock);
    if (rc) {                          /* If routine indicates error,*/
                                       /* tell the user.             */
      fprintf(stderr,"*** Library load routine gave error %d.\n",rc);
      return(rc);                      /* don't load anything        */
    } /* endif */

    /* Now run through the array of func blocks, adding them to the  */
    /* list of external functions or subcommand handlers.  Note that */
    /* we use the external function types in all cases, but since    */
    /* the only thing affected is the function pointers, there's no  */
    /* problem - the RXFUNCBLOCK and RXSUBCOMBLOCK types are         */
    /* otherwise identical.                                          */
    for (j=0; funcblock[j].name != NULL; j++) {
#ifdef ORXLD_DEBUG
        fprintf(stderr,"REXXUTIL: PKGFunction %s \n", funcblock[j].name);
#endif
        if ( funcblock[j].function && (rc == NULL) ) {
          RexxRegisterFunctionDll( funcblock[j].name,
                                   argstring, funcblock[j].name);
        }
    }
  }
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  SysAddCmdPkg    like SysAddFuncPkg                          *
*                                                                        *
*************************************************************************/
size_t RexxEntry SysAddCmdPkg(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    return SysAddFuncPkg(
                        name,          /* Function name              */
                        numargs,       /* Number of arguments        */
                        args,          /* Argument array             */
                        queuename,     /* Current queue              */
                        retstr );      /* Return RXSTRING            */

}

/*************************************************************************
* Function:  SysDropFuncPkg                                              *
*                                                                        *
* Description:   Function to enable load of old CREXX libraries for AIX. *
*                It is used for the old >load< function call for the     *
*                list of function to be dropped from REXX.               *
*                The Path must not be fully qualified in REXX procedure. *
*                                                                        *
* Syntax:    call SysDropFuncPkg                                         *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

size_t RexxEntry SysDropFuncPkg(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode rc = NULL;
  int    j;                            /* Counter                    */
  size_t arglength;                    /* length of the count        */
  char *  argstring;                    /* input sleep time           */
  RXFUNCBLOCK *funcblock;              /* Base for function blocks   */
  PRXINITFUNCPKG InitFunc;             /* Pointer returned from load */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs != 1)                    /* Must have one argument     */
    return INVALID_ROUTINE;

  argstring = args[0].strptr;             /* point to the string        */
  arglength = args[0].strlength;          /* get length of string       */
  if ( (arglength == 0 ) ||               /* if null string             */
       (arglength >= MAXNAME) )           /* or too long                */
    return INVALID_ROUTINE;               /* not valid                  */

  if (!(InitFunc = (PRXINITFUNCPKG)load(argstring,0,NULL)))
  {                                       /*  loadAndInit? for load     */
    if ( InitFunc == NULL ) {
      fprintf(stderr, " *** Unable to drop library %s !\nError message: errno = %d;",\
               argstring, errno);
      perror(" REXXUTIL");
      rc = 1;
    }
  }
  if ( rc == NULL ) {
    /* Call the initialization routine for the library (which should */
    /* be the function pointer returned to us by load).              */
    rc = (*InitFunc)(&funcblock);
    if (rc) {                          /* If routine indicates error,*/
                                       /* tell the user.             */
      fprintf(stderr," *** Library drop routine gave error %d.\n",rc);
      return(rc);                      /* don't load anything        */
    } /* endif */

    /* Now run through the array of func blocks, adding them to the  */
    /* list of external functions or subcommand handlers.  Note that */
    /* we use the external function types in all cases, but since    */
    /* the only thing affected is the function pointers, there's no  */
    /* problem - the RXFUNCBLOCK and RXSUBCOMBLOCK types are         */
    /* otherwise identical.                                          */
    for (j=0; funcblock[j].name != NULL; j++) {
#ifdef ORXLD_DEBUG
        fprintf(stderr,"REXXUTIL: PKGFunction %s \n", funcblock[j].name);
#endif
        if ( funcblock[j].function && (rc == NULL) ) {
          RexxDeregisterFunction( funcblock[j].name );
        }
    } /* endfor */
  }
  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysDropCmdPkg    like SysDropFuncPkg                        *
*                                                                        *
*************************************************************************/
size_t RexxEntry SysDropCmdPkg(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    return SysDropFuncPkg(
                        name,          /* Function name              */
                        numargs,       /* Number of arguments        */
                        args,          /* Argument array             */
                        queuename,     /* Current queue              */
                        retstr );      /* Return RXSTRING            */
}
#endif                                 /* CREXX funcs                */

#ifdef AIX
/*************************************************************************
* Function:  SysGetpid                                                   *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                                                                        *
* Syntax:    call SysGetpid()                                            *
*                                                                        *
* Return:    Process_ID                                                  *
*************************************************************************/

size_t RexxEntry SysGetpid(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs != 0)                    /* Must have no  argument     */
    return INVALID_ROUTINE;
  else
  {
    sprintf(retstr->strptr, "%d", getpid());
    retstr->strlength = strlen(retstr->strptr);
  }
  return VALID_ROUTINE;                /* no error on call           */
}
#endif                                 /* CREXX funcs                */

/*************************************************************************
* Function:  SysFork                                                     *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                                                                        *
* Syntax:    call SysFork()                                              *
*                                                                        *
* Return:    Process_ID   ( to parent child''s ID / to child the ID 0 )  *
*************************************************************************/

size_t RexxEntry SysFork(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs != 0)                    /* Must have no  argument     */
    return INVALID_ROUTINE;
  else
  {
    sprintf(retstr->strptr, "%d", fork());
    retstr->strlength = strlen(retstr->strptr);
  }
  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysWait                                                     *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                                                                        *
* Syntax:    call SysWait()                                              *
*                                                                        *
* Return:    exit code of child                                          *
*************************************************************************/

size_t RexxEntry SysWait(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  int iStatus;
  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs != 0)                    /* Must have no  argument     */
    return INVALID_ROUTINE;
  else
  {
    wait( &iStatus );
    sprintf(retstr->strptr, "%d", iStatus);
    retstr->strlength = strlen(retstr->strptr);
  }
  return VALID_ROUTINE;                /* no error on call           */
}

/*************************************************************************
* Function:  SysCreatePipe                                               *
*                                                                        *
* Description:   Function to migrate CREXX for AIX procedures.           *
*                Function creates an unnamed pipe                        *
*                                                                        *
* Syntax:    call SysCreatePipe( Blocking | Nonblocking )                *
*                                                                        *
* Return:    'handle handle'     ( handle for read and handle for write )*
*************************************************************************/

size_t RexxEntry SysCreatePipe(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  int  iStatus;
  int  iaH[2];
  char cBlocking = 0;
  retstr->strlength = 0;                     /* set return value           */
                                             /* check arguments ---------- */
  if (numargs > 1)                           /* More than one arg?         */
    return INVALID_ROUTINE;
  if (numargs == 0 )                         /* No arg?                    */
    cBlocking = 1;                           /* Default is blocking        */
  /* One arg, first char is 'b'?    */
  else if (args[0].strptr[0] == 'b' || args[0].strptr[0] == 'B')
    cBlocking = 1;                            /* Wants blocking             */
  /* One arg, first char is 'n'?    */
  else if (args[0].strptr[0] == 'n' || args[0].strptr[0] == 'N')
    cBlocking = 0;                            /* Wants non-blocking         */

  if (pipe(iaH))                             /* Create the pipe            */
  {
    perror("*** ERROR: Creating pipe");      /* pipe creation failed       */
    return VALID_ROUTINE;                    /* no error on call           */
  }
  if (!cBlocking)                            /* Non-blocking?              */
  {
    /* Get file status flags ---------- */
    iStatus = fcntl(iaH[0], F_GETFL, NULL);
    iStatus |= O_NONBLOCK;                   /* Turn on NONBLOCK flag      */
    /* Does set work? ----------------- */
    if (fcntl(iaH[0], F_SETFL, iStatus) == -1) {
      perror("*** ERROR: Setting NONBLOCK flag");  /* No, tell user        */
      close(iaH[0]); close(iaH[1]);                /* Close pipes          */
      return VALID_ROUTINE;                  /* no error on call           */
    }
  }
  sprintf(retstr->strptr, "%d %d", iaH[0], iaH[1]);/* Create return string */
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;                      /* no error on call           */
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
  RexxReturnCode    rc;                        /* Ret code                   */
  SHVBLOCK  shvb;
  int       handle;
  bool      fCloseFile = false;

  if ( (numargs > 1) ||                /* wrong number of arguments? */
       ((numargs > 0) && !RXVALIDSTRING(args[0])) )
    return INVALID_ROUTINE;            /* raise error condition      */

  if (numargs > 0)
  {
    /* open output file for append */
    fCloseFile = true;

    handle = open(args[0].strptr, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
    if(handle < 0)
        return INVALID_ROUTINE;            /* raise error condition      */
  }
  else
      handle = STDOUT_FILENO;

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


      write(handle, "Name=", strlen("Name="));
      write(handle, shvb.shvname.strptr, shvb.shvname.strlength);
      write(handle, ", Value='", 9);
      write(handle, shvb.shvvalue.strptr,shvb.shvvalue.strlength);
      write(handle, "'\n", 2);

      /* free memory allocated by REXX */
      RexxFreeMemory((void *)shvb.shvname.strptr);
      RexxFreeMemory((void *)shvb.shvvalue.strptr);

      /* leave loop if this was the last var */
      if (shvb.shvret & RXSHV_LVAR)
        break;
    }
  } while (rc == RXSHV_OK);

  if (fCloseFile)
     close(handle);          /* close the file             */

  if (rc != RXSHV_LVAR)
    RETVAL(-1)
  else
    RETVAL(0)
}

/*************************************************************************
* Function:  SysGetFileDateTime                                          *
*                                                                        *
* Syntax:    result = SysGetFileDateTime(filename [,timesel])            *
* Params:    filename - name of the file to query                        *
*            timesel  - What filetime to query: Access/Write             *
*                                                                        *
* Return:    -1 - file date/time query failed                            *
*            other - date and time as YYYY-MM-DD HH:MM:SS                *
*************************************************************************/

size_t RexxEntry SysGetFileDateTime(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  struct    stat buf;
  struct    tm *newtime;
  const char *dir_buf = NULL;            /* full directory path        */
  bool      fOk = true;
  bool      alloc_Flag = false;

  if ( (numargs < 1) || (numargs > 2) ||
       ((numargs == 2) && !RXVALIDSTRING(args[1])) )
    return INVALID_ROUTINE;            /* raise error condition      */

  if(*(args[0].strptr) == '~')
  {
    dir_buf = resolve_tilde(args[0].strptr);
    alloc_Flag = true;
  }
  else
  {
    dir_buf = args[0].strptr;
  }

  if (stat(dir_buf, &buf) < 0)
  {
     fOk = false;
  }

  if(fOk)
  {
     if (numargs > 1)
     {
       switch (args[1].strptr[0])
       {
         case 'a':
         case 'A':
           newtime = localtime(&(buf.st_atime));
           break;
         case 'w':
         case 'W':
           newtime = localtime(&(buf.st_mtime));

           break;
         default:
           return INVALID_ROUTINE;
       }
     }
     else
         newtime = localtime(&(buf.st_mtime));

     newtime->tm_year += 1900;
     newtime->tm_mon += 1;

     sprintf(retstr->strptr, "%4d-%02d-%02d %02d:%02d:%02d",
             newtime->tm_year,
             newtime->tm_mon,
             newtime->tm_mday,
             newtime->tm_hour,
             newtime->tm_min,
             newtime->tm_sec);
             retstr->strlength = strlen(retstr->strptr);
  }
  if( (dir_buf) && (alloc_Flag == true) )
    free((void *)dir_buf);  /* free the buffer memory  */
  if (!fOk)
    RETVAL(-1)
  else
    return VALID_ROUTINE;
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

size_t RexxEntry SysSetFileDateTime(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  bool      fOk = true;
  struct utimbuf timebuf;
  struct tm *newtime;
  time_t ltime;
  struct stat buf;
  const char *dir_buf = NULL;
  bool      alloc_Flag = false;

  /* we expect one to three parameters, if three parameters are      */
  /* specified then the second may be omitted to set only a new time,*/
  /* if only one is specified then the file is set to current time   */

  if ( (numargs < 1) || (numargs > 3) ||
       ((numargs == 2) && !RXVALIDSTRING(args[1])) ||
       ((numargs == 3) && !RXVALIDSTRING(args[2])) )
    return INVALID_ROUTINE;            /* raise error condition      */

  if(*(args[0].strptr) == '~')
  {
    dir_buf = resolve_tilde(args[0].strptr);
    alloc_Flag = true;
  }
  else
  {
    dir_buf = args[0].strptr;
  }

  if (stat(dir_buf, &buf) < 0)
  {
     fOk =  false;
  }

  if (numargs == 1)
  {
    time(&ltime);
    timebuf.modtime = ltime;
    if (utime(dir_buf, &timebuf) < 0)
    {
       fOk = false;
    }
  }
  else
  {
    newtime = localtime(&(buf.st_mtime));
    if ((numargs >= 2) && RXVALIDSTRING(args[1]))
    {

       /* parse new date */
       if (sscanf(args[1].strptr, "%4d-%2d-%2d", &newtime->tm_year,
                  &newtime->tm_mon, &newtime->tm_mday) != 3)
       fOk = false;
       newtime->tm_year -= 1900;
       newtime->tm_mon -= 1;
    }
    if ((numargs == 3) && RXVALIDSTRING(args[2]))
    {
       /* parse new time */
       if (sscanf(args[2].strptr, "%2d:%2d:%2d", &newtime->tm_hour,
                  &newtime->tm_min, &newtime->tm_sec) != 3)
            fOk = false;
    }
    ltime = mktime(newtime);
    timebuf.modtime = ltime;
    if (utime(dir_buf, &timebuf) < 0)
    {
       fOk = false;
    }
  }

  if( (dir_buf) && (alloc_Flag == true) )
    free((void *)dir_buf);               /* free the buffer memory  */
  if (fOk)
    RETVAL(0)
  else
    RETVAL(-1)
}


size_t RexxEntry RexxStemSort(const char *stemname, int order, int type,
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
    char          stemName[255];
    size_t        first = 1;
    size_t        last = SIZE_MAX;
    size_t        firstCol = 0;
    size_t        lastCol = SIZE_MAX;
    int           sortType = SORT_CASESENSITIVE;
    int           sortOrder = SORT_ASCENDING;

    if ( (numargs < 1) || (numargs > 7) || /* validate arguments       */
        !RXVALIDSTRING(args[0]))
      return INVALID_ROUTINE;

    /* remember stem name */
    memset(stemName, 0, sizeof(stemName));
    strcpy(stemName, args[0].strptr);
    if (stemName[args[0].strlength-1] != '.')
      stemName[args[0].strlength] = '.';

    /* check other parameters */
    if ((numargs >= 2) && RXVALIDSTRING(args[1])) /* sort order */
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
      if (sscanf(args[3].strptr, "%u", &first) != 1)
        return INVALID_ROUTINE;
      if (first == 0)
        return INVALID_ROUTINE;
    } /* endif */

    if ((numargs >= 5) && RXVALIDSTRING(args[4])) /* last element to sort */
    {
      if (sscanf(args[4].strptr, "%u", &last) != 1)
        return INVALID_ROUTINE;
      if (last < first)
        return INVALID_ROUTINE;
    } /* endif */

    if ((numargs >= 6) && RXVALIDSTRING(args[5])) /* first column to sort */
    {
      if (sscanf(args[5].strptr, "%u", &firstCol) != 1)
        return INVALID_ROUTINE;
      firstCol--;
    } /* endif */

    if ((numargs == 7) && RXVALIDSTRING(args[6])) /* last column to sort */
    {
      if (sscanf(args[6].strptr, "%u", &lastCol) != 1)
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

size_t RexxEntry SysStemDelete(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode        rc;
  char          szStemName[255];
  char *        pszStemIdx;
  char          szValue[255];
  SHVBLOCK      shvb;
  size_t         ulIdx;
  size_t         ulFirst;
  size_t         ulItems = 1;
  size_t         ulCount;
  bool          fOk = true;

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
  if (sscanf(args[1].strptr, "%u", &ulFirst) != 1)
    return INVALID_ROUTINE;

  /* get number of items to delete */
  if (numargs == 3)
  {
    if (sscanf(args[2].strptr, "%u", &ulItems) != 1)
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
    if (sscanf(shvb.shvvalue.strptr, "%u", &ulCount) != 1)
      return INVALID_ROUTINE;

    /* check wether supplied index and count is within limits */
    if (ulFirst + ulItems - 1 > ulCount)
      return INVALID_ROUTINE;

    /* now copy the remaining indices up front */
    for (ulIdx = ulFirst; ulIdx + ulItems <= ulCount; ulIdx++)
    {
      /* get element to relocate */
      sprintf(pszStemIdx, "%u", ulIdx + ulItems);
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
        sprintf(pszStemIdx, "%u", ulIdx);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szStemName;
        shvb.shvname.strlength = strlen((const char *)szStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = false;

        /* free memory allocated by REXX */
        free(shvb.shvvalue.strptr);
      }
      else
        fOk = false;

      if (!fOk)
        break;
    } /* endfor */

    if (fOk)
    {
      /* now delete the items at the end */
      for (ulIdx = ulCount - ulItems + 1; ulIdx <= ulCount; ulIdx++)
      {
        sprintf(pszStemIdx, "%u", ulIdx);
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
          fOk = false;
          break;
        } /* endif */
      } /* endfor */
    } /* endif */

    if (fOk)
    {
      /* set the new number of items in the stem array */
      strcpy(pszStemIdx, "0");
      sprintf(szValue, "%u", ulCount - ulItems);
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
        fOk = false;
    } /* endif */
  }
  else
  {
    fOk = false;
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

size_t RexxEntry SysStemInsert(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode        rc;
  char          szStemName[255];
  char *        pszStemIdx;
  char          szValue[255];
  SHVBLOCK      shvb;
  size_t         ulIdx;
  size_t         ulPosition;
  size_t         ulCount;
  bool          fOk = true;

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
  if (sscanf(args[1].strptr, "%u", &ulPosition) != 1)
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
    if (sscanf(shvb.shvvalue.strptr, "%u", &ulCount) != 1)
      return INVALID_ROUTINE;

    /* check wether new position is within limits */
    if ((ulPosition == 0) || (ulPosition > ulCount + 1))
      return INVALID_ROUTINE;

    /* make room for new item by moving all items to the end */
    for (ulIdx = ulCount; ulIdx >= ulPosition; ulIdx--)
    {
      /* get element to relocate */
      sprintf(pszStemIdx, "%u", ulIdx);
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
        sprintf(pszStemIdx, "%u", ulIdx + 1);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szStemName;
        shvb.shvname.strlength = strlen((const char *)szStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = false;

        /* free memory allocated by REXX */
        free(shvb.shvvalue.strptr);
      }
      else
        fOk = false;

      if (!fOk)
        break;
    } /* endfor */

    if (fOk)
    {
      /* set the new item value */
      sprintf(pszStemIdx, "%u", ulPosition);
      shvb.shvnext = NULL;
      shvb.shvname.strptr = szStemName;
      shvb.shvname.strlength = strlen((const char *)szStemName);
      shvb.shvvalue.strptr = const_cast<char *>(args[2].strptr);
      shvb.shvvalue.strlength = args[2].strlength;
      shvb.shvnamelen = shvb.shvname.strlength;
      shvb.shvvaluelen = shvb.shvvalue.strlength;
      shvb.shvcode = RXSHV_SYSET;
      shvb.shvret = 0;
      rc = RexxVariablePool(&shvb);
      if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
        fOk = false;
    } /* endif */

    if (fOk)
    {
      /* set the new number of items in the stem array */
      strcpy(pszStemIdx, "0");
      sprintf(szValue, "%u", ulCount + 1);
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
        fOk = false;
    } /* endif */
  }
  else
  {
    fOk = false;
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

size_t RexxEntry SysStemCopy(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  RexxReturnCode        rc;
  char          szFromStemName[255];
  char          szToStemName[255];
  char *        pszFromStemIdx;
  char *        pszToStemIdx;
  char          szValue[255];
  SHVBLOCK      shvb;
  size_t         ulIdx;
  size_t         ulToCount;
  size_t         ulFromCount;
  size_t         ulFrom = 1;
  size_t         ulTo = 1;
  size_t         ulCopyCount = 0;
  bool          fInsert = false;
  bool          fOk = true;

  if ( (numargs < 2) || (numargs > 6) ||  /* validate arguments       */
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
    if (sscanf(args[2].strptr, "%u", &ulFrom) != 1)
      return INVALID_ROUTINE;

  /* get to item index */
  if ((numargs >= 4) && RXVALIDSTRING(args[3]))
    if (sscanf(args[3].strptr, "%u", &ulTo) != 1)
      return INVALID_ROUTINE;

  /* get copy count */
  if ((numargs >= 5) && RXVALIDSTRING(args[4]))
    if (sscanf(args[4].strptr, "%u", &ulCopyCount) != 1)
      return INVALID_ROUTINE;

  /* get copy type */
  if (numargs >= 6)
  {
    switch (args[5].strptr[0])
    {
      case 'I':
      case 'i':
        fInsert = true;
        break;
      case 'O':
      case 'o':
        fInsert = false;
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
    if (sscanf(shvb.shvvalue.strptr, "%u", &ulFromCount) != 1)
      return INVALID_ROUTINE;

    if ((ulCopyCount > ulFromCount) || (ulFromCount == 0))
      return INVALID_ROUTINE;
  }
  else
    fOk = false;

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
      if (sscanf(shvb.shvvalue.strptr, "%u", &ulToCount) != 1)
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
        fOk = false;
    } /* endif */

    if (fOk)
    {
      if (ulTo > ulToCount + 1)
        return INVALID_ROUTINE;
    } /* endif */
  } /* endif */

  /* set copy count to number of items in source stem if not already set */
  if (ulCopyCount == 0)
    ulCopyCount = ulFromCount;

  if (fOk && fInsert)
  {
    /* if we are about to insert the items we have to make room */
    for (ulIdx = ulToCount; ulIdx >= ulTo; ulIdx--)
    {
      /* get element to relocate */
      sprintf(pszToStemIdx, "%u", ulIdx);
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
        sprintf(pszToStemIdx, "%u", ulIdx + ulCopyCount);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szToStemName;
        shvb.shvname.strlength = strlen((const char *)szToStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = false;

        /* free memory allocated by REXX */
        free(shvb.shvvalue.strptr);
      }
      else
        fOk = false;

      if (!fOk)
        break;
    } /* endfor */

    if (fOk)
    {
      /* set the new count for the target stem */
      strcpy(pszToStemIdx, "0");
      ulToCount += ulCopyCount;
      sprintf(szValue, "%u", ulToCount);
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
        fOk = false;
    } /* endif */
  } /* endif */

  if (fOk)
  {
    /* now do the actual copying from the source to target */
    for (ulIdx = 0; ulIdx < ulCopyCount; ulIdx++)
    {
      /* get element to copy */
      sprintf(pszFromStemIdx, "%u", ulFrom + ulIdx);
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
        sprintf(pszToStemIdx, "%u", ulTo + ulIdx);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = szToStemName;
        shvb.shvname.strlength = strlen((const char *)szToStemName);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvaluelen = shvb.shvvalue.strlength;
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        rc = RexxVariablePool(&shvb);
        if ((rc != RXSHV_OK) && (rc != RXSHV_NEWV))
          fOk = false;

        /* free memory allocated by REXX */
        free(shvb.shvvalue.strptr);
      }
      else
        fOk = false;

      if (!fOk)
        break;
    } /* endfor */
  } /* endif */

  if (fOk && (ulTo + ulCopyCount - 1 > ulToCount))
  {
    /* set the new count for the target stem */
    strcpy(pszToStemIdx, "0");
    ulToCount = ulTo + ulCopyCount - 1;
    sprintf(szValue, "%u", ulToCount);
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
      fOk = false;
  } /* endif */

  if (fOk)
    RETVAL(0)
  else
    RETVAL(-1)
}

//#endif


/**************************************************************************
* Function:  SysQueryProcess                                              *
*                                                                         *
* Params:    "PID"       - (default) returns current process ID           *
*    NEW:    "PPID"      -  returns parent of current process ID          *
*    NEW:    "PGID"      -  returns group ID of current process           *
*    NO      "TID"       -  returns current thread ID                     *
*    YES     "PPRIO"     -  returns current process priority              *
*    NO      "TPRIO"     -  returns current thread priority               *
*    YES     "PTIME"     -  returns current process times                 *
*    NO      "TTIME"     -  returns current thread times                  *
*    NEW:    "PMEM"      -  returns current process max memory size RSS   *
*    NEW:    "PSWAPS"    -  returns current process number of swaps out   *
*    NEW:    "PRCVDSIG"  -  returns current process received signals      *
***************************************************************************/

size_t RexxEntry SysQueryProcess(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{

  unsigned int uiUsedCPUTime  = 0;
  unsigned int uiUsedCPUmsec  = 0;
  unsigned int uiUsedHours    = 0;
  unsigned int uiUsedMinutes  = 0;
  unsigned int uiUsedSeconds  = 0;
  char timebuf[40];
  int iRc = 0;
  struct rusage struResUse;
  retstr->strlength = 0;                 /* set return value           */

  if (numargs > 1)                    /* none or one argument accepted */
    return INVALID_ROUTINE;              /* raise error condition      */

  if ((numargs == 0) || (!strcasecmp(args[0].strptr, "PID")))
  {
    sprintf(retstr->strptr, "%d", getpid());
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }
  else
  if (!strcasecmp(args[0].strptr, "PPID"))
  {
    sprintf(retstr->strptr, "%d", getppid());
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }
  else
  if (!strcasecmp(args[0].strptr, "PGID"))
  {
    sprintf(retstr->strptr, "%d", getpgid(getpid()));
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }
  else
  if (!strcasecmp(args[0].strptr, "PPRIO"))
  {
    sprintf(retstr->strptr, "%d", getpriority(PRIO_PROCESS, 0));
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }

  /* ----------------------------------------------------------------- */
  /* Get process usage data and keep calls together at the end of      */
  /* the function SysQueryProcess.                                     */
  iRc = getrusage ( RUSAGE_SELF, &struResUse);
  if ( iRc )
  {
     sprintf(retstr->strptr, " System error; errno = %d", errno);
     retstr->strlength = strlen(retstr->strptr);
     return VALID_ROUTINE;               /* no error on call           */
  }
  if (!strcasecmp(args[0].strptr, "PTIME")) /* Calculate the used CPU time*/
  {
    uiUsedCPUmsec  = (unsigned int) struResUse.ru_utime.tv_usec/1000;
    uiUsedCPUmsec += (unsigned int) struResUse.ru_stime.tv_usec/1000;
    if (uiUsedCPUmsec >= 1000 )
    {
      uiUsedCPUTime = uiUsedCPUmsec / 1000;
      uiUsedCPUmsec = uiUsedCPUmsec % 1000;
    }
    uiUsedCPUTime += (unsigned int) struResUse.ru_utime.tv_sec;
    uiUsedCPUTime += (unsigned int) struResUse.ru_stime.tv_sec;
    uiUsedHours   = uiUsedCPUTime / 3600;
    uiUsedMinutes = uiUsedCPUTime / 60;
    if (uiUsedMinutes >= 60 ) uiUsedMinutes = uiUsedMinutes % 60;
    if (uiUsedCPUTime >= 60 ) uiUsedSeconds = uiUsedCPUTime % 60;
    else uiUsedSeconds = uiUsedCPUTime;

    sprintf(retstr->strptr, "CPU_Time Summary: %2d:%.2d:%.2d:%.3d  Kernel:",
                    uiUsedHours, uiUsedMinutes, uiUsedSeconds, uiUsedCPUmsec );

    uiUsedCPUmsec = (unsigned int) struResUse.ru_stime.tv_usec/1000;
    uiUsedCPUTime = (unsigned int) struResUse.ru_stime.tv_sec;
    uiUsedHours   = uiUsedCPUTime / 3600;
    uiUsedMinutes = uiUsedCPUTime / 60;
    if (uiUsedMinutes >= 60 ) uiUsedMinutes = uiUsedMinutes % 60;
    if (uiUsedCPUTime >= 60 ) uiUsedSeconds = uiUsedCPUTime % 60;
    else uiUsedSeconds = uiUsedCPUTime;

    sprintf(timebuf, " %2d:%.2d:%.2d:%.3d  User:", uiUsedHours,
                                 uiUsedMinutes, uiUsedSeconds, uiUsedCPUmsec );
    strcat(retstr->strptr, timebuf);

    uiUsedCPUmsec = (unsigned int) struResUse.ru_utime.tv_usec/1000;
    uiUsedCPUTime = (unsigned int) struResUse.ru_utime.tv_sec;
    uiUsedHours   = uiUsedCPUTime / 3600;
    uiUsedMinutes = uiUsedCPUTime / 60;
    if (uiUsedMinutes >= 60 ) uiUsedMinutes = uiUsedMinutes % 60;
    if (uiUsedCPUTime >= 60 ) uiUsedSeconds = uiUsedCPUTime % 60;
    else uiUsedSeconds = uiUsedCPUTime;

    sprintf(timebuf, " %2d:%.2d:%.2d:%.3d", uiUsedHours,
                                 uiUsedMinutes, uiUsedSeconds, uiUsedCPUmsec );
    strcat(retstr->strptr, timebuf);

    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }
  else
  if (!strcasecmp(args[0].strptr, "PMEM"))  /* Show max memory RSS used   */
  {
    sprintf(retstr->strptr, "Max_Memory_RSS: %ld", struResUse.ru_maxrss);
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }
  else
  if (!strcasecmp(args[0].strptr, "PSWAPS")) /* Memory has been swapped   */
  {
    sprintf(retstr->strptr, "Memory_swaps: %ld", struResUse.ru_nswap);
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }
  else
  if (!strcasecmp(args[0].strptr, "PRCVDSIG")) /* Process received signals*/
  {
    sprintf(retstr->strptr, "Received_signals: %ld", struResUse.ru_nsignals);
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;                /* no error on call           */
  }

  return INVALID_ROUTINE;                /* good completion            */
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

  int   errnum  = 0;
  char  *errmsg = NULL;

  if (numargs != 1)
                                       /* If no args, then its an    */
                                       /* incorrect call             */
    return INVALID_ROUTINE;

  errnum = atoi(args[0].strptr);
  errmsg = strerror( errnum );
  if (errmsg == NULL )
    retstr->strptr[0] = '\0';
  else
  {
    if (strlen(errmsg) >= retstr->strlength)
       retstr->strptr = (char *) malloc(strlen(errmsg + 1));
    strcpy(retstr->strptr,errmsg);
  }
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  SysUtilVersion                                              *
*                                                                        *
* Syntax:    Say SysUtilVersion                                          *
*                                                                        *
* Return:    REXXUTIL.DLL Version                                        *
*************************************************************************/

size_t RexxEntry SysUtilVersion(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  if (numargs != 0)                    /* validate arg count         */
    return INVALID_ROUTINE;
                                       /* format into the buffer            */
  sprintf(retstr->strptr, "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
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

size_t RexxEntry SysIsFile(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
  struct stat finfo;                   /* return buf for the finfo   */

  if (numargs != 1)                    /* we need one argument       */
    return INVALID_ROUTINE;            /* raise an error             */

  stat(args[0].strptr, &finfo);        /* read the info about it     */

  if(S_ISREG(finfo.st_mode) ||         /* if it is a file            */
     S_ISBLK(finfo.st_mode)) {         /* file                       */
    RETVAL(1)                          /* True - Is a File           */
  }
  else {
    RETVAL(0)                          /* False - Is something else  */
  }
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

size_t RexxEntry SysIsFileDirectory(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    struct stat finfo;                 /* return buf for the finfo   */

    if (numargs != 1)                  /* we need one argument       */
      return INVALID_ROUTINE;          /* raise an error             */

    stat(args[0].strptr, &finfo);      /* read the info about it     */

    if(S_ISDIR(finfo.st_mode)) {       /* is a subdir?               */
      RETVAL(1)                        /* True - Is a File           */
    }
    else {
      RETVAL(0)                        /* False - Is something else  */
    }
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

size_t RexxEntry SysIsFileLink(const char *name, size_t numargs, CONSTRXSTRING args[], const char *queuename, PRXSTRING retstr)
{
    struct stat finfo;                 /* return buf for the finfo   */

    if (numargs != 1)                  /* we need one argument       */
      return INVALID_ROUTINE;          /* raise an error             */

    lstat(args[0].strptr, &finfo);     /* read the info about it     */

    if(S_ISLNK(finfo.st_mode)) {       /* or a symbolic link         */
      RETVAL(1)                        /* True - Is a File           */
    }
    else {
      RETVAL(0)                        /* False - Is something else  */
    }
}


#ifdef XX__cplusplus
}
#endif


// now build the actual entry list
RexxRoutineEntry rexxutil_routines[] =
{
    REXX_TYPED_ROUTINE(SysCreateMutexSem,      SysCreateMutexSem),
    REXX_TYPED_ROUTINE(SysOpenMutexSem,        SysOpenMutexSem),
    REXX_TYPED_ROUTINE(SysCloseMutexSem,       SysCloseMutexSem),
    REXX_TYPED_ROUTINE(SysRequestMutexSem,     SysRequestMutexSem),
    REXX_TYPED_ROUTINE(SysReleaseMutexSem,     SysReleaseMutexSem),
    REXX_TYPED_ROUTINE(SysCreateEventSem,      SysCreateEventSem),
    REXX_TYPED_ROUTINE(SysOpenEventSem,        SysOpenEventSem),
    REXX_TYPED_ROUTINE(SysCloseEventSem,       SysCloseEventSem),
    REXX_TYPED_ROUTINE(SysResetEventSem,       SysResetEventSem),
    REXX_TYPED_ROUTINE(SysPostEventSem,        SysPostEventSem),
    REXX_TYPED_ROUTINE(SysWaitEventSem,        SysWaitEventSem),
    REXX_CLASSIC_ROUTINE(SysSetPriority,         SysSetPriority),
    REXX_CLASSIC_ROUTINE(SysAddRexxMacro,        SysAddRexxMacro),
    REXX_CLASSIC_ROUTINE(SysDropRexxMacro,       SysDropRexxMacro),
    REXX_CLASSIC_ROUTINE(SysReorderRexxMacro,    SysReorderRexxMacro),
    REXX_CLASSIC_ROUTINE(SysQueryRexxMacro,      SysQueryRexxMacro),
    REXX_CLASSIC_ROUTINE(SysClearRexxMacroSpace, SysClearRexxMacroSpace),
    REXX_CLASSIC_ROUTINE(SysLoadRexxMacroSpace,  SysLoadRexxMacroSpace),
    REXX_CLASSIC_ROUTINE(SysSaveRexxMacroSpace,  SysSaveRexxMacroSpace),
#if defined(AIX_DISABLED)
    REXX_CLASSIC_ROUTINE(SysAddFuncPkg,          SysAddFuncPkg),
    REXX_CLASSIC_ROUTINE(SysAddCmdPkg,           SysAddCmdPkg),
    REXX_CLASSIC_ROUTINE(SysDropFuncPkg,         SysDropFuncPkg),
    REXX_CLASSIC_ROUTINE(SysDropCmdPkg,          SysDropCmdPkg),
#endif
#if defined(AIX)
    REXX_CLASSIC_ROUTINE(SysGetpid,              SysGetpid),
#endif
    REXX_CLASSIC_ROUTINE(SysFork,                SysFork),
    REXX_CLASSIC_ROUTINE(SysWait,                SysWait),
    REXX_CLASSIC_ROUTINE(SysCreatePipe,          SysCreatePipe),
    REXX_CLASSIC_ROUTINE(SysCls,                 SysCls),
    REXX_CLASSIC_ROUTINE(SysDropFuncs,           SysDropFuncs),
    REXX_CLASSIC_ROUTINE(SysFileDelete,          SysFileDelete),
    REXX_CLASSIC_ROUTINE(SysFileSearch,          SysFileSearch),
    REXX_CLASSIC_ROUTINE(SysFileTree,            SysFileTree),
    REXX_CLASSIC_ROUTINE(SysGetKey,              SysGetKey),
    REXX_CLASSIC_ROUTINE(SysGetMessage,          SysGetMessage),
    REXX_CLASSIC_ROUTINE(SysGetMessageX,         SysGetMessageX),
    REXX_CLASSIC_ROUTINE(SysLoadFuncs,           SysLoadFuncs),
    REXX_CLASSIC_ROUTINE(SysMkDir,               SysMkDir),
#ifdef LINUX
    REXX_CLASSIC_ROUTINE(SysLinVer,              SysLinVer),
#endif
    REXX_CLASSIC_ROUTINE(SysVersion,             SysVersion),
    REXX_CLASSIC_ROUTINE(SysRmDir,               SysRmDir),
    REXX_CLASSIC_ROUTINE(SysSearchPath,          SysSearchPath),
    REXX_CLASSIC_ROUTINE(SysSleep,               SysSleep),
    REXX_CLASSIC_ROUTINE(SysTempFileName,        SysTempFileName),
    REXX_CLASSIC_ROUTINE(SysDumpVariables,       SysDumpVariables),
    REXX_CLASSIC_ROUTINE(SysSetFileDateTime,     SysSetFileDateTime),
    REXX_CLASSIC_ROUTINE(SysGetFileDateTime,     SysGetFileDateTime),
    REXX_CLASSIC_ROUTINE(SysStemSort,            SysStemSort),
    REXX_CLASSIC_ROUTINE(SysStemDelete,          SysStemDelete),
    REXX_CLASSIC_ROUTINE(SysStemInsert,          SysStemInsert),
    REXX_CLASSIC_ROUTINE(SysStemCopy,            SysStemCopy),
    REXX_CLASSIC_ROUTINE(SysQueryProcess,        SysQueryProcess),
    REXX_CLASSIC_ROUTINE(SysGetErrortext,        SysGetErrortext),
    REXX_CLASSIC_ROUTINE(SysUtilVersion,         SysUtilVersion),
    REXX_CLASSIC_ROUTINE(SysIsFile,              SysIsFile),
    REXX_CLASSIC_ROUTINE(SysIsFileDirectory,     SysIsFileDirectory),
    REXX_CLASSIC_ROUTINE(SysIsFileLink,          SysIsFileLink),
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

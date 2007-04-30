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
/* REXX Kernel                                                  winfile.c     */
/*                                                                            */
/* Windows specific file related routines.                                    */
/*                                                                            */
/******************************************************************************/

#define  INCL_DOSMISC
#define  INCL_REXX_STREAM
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxNativeAPI.h"
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#define DEFEXT ".REX"
#define TEMPEXT ".CMD"                 /* Alternate extension   */
#define MAX_STDOUT_LENGTH     32767    /* max. amount of data to push to STDOUT @THU007A */ /* @HOL007M */
#include "StreamNative.h"              /* include the stream information    */

#define COMPILE_NEWAPIS_STUBS          /* Allows GetLongPathName to run on  */
#define WANT_GETLONGPATHNAME_WRAPPER   /* NT and Windows 95                 */
#include <NewAPIs.h>

PCHAR SysFileExtension(PCHAR);
RexxString * LocateProgram(RexxString *, PCHAR[], LONG);
BOOL  SearchFileName(PCHAR, PCHAR);
void GetLongName(PCHAR, DWORD);
BOOL FindFirstFile(PCHAR Name);
FILE * SysBinaryFilemode(FILE *, BOOL);
INT SysFFlush(FILE *);
BOOL SysFileIsDevice(int fhandle);
int  SysPeekKeyboard(void);
INT SysStat(char * path, struct stat *buffer);
BOOL SysFileIsPipe(STREAM_INFO * stream_info);


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysResolveProgramName                        */
/*                                                                   */
/*   Function:          Expand a filename to a fully resolved REXX   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
RexxString * SysResolveProgramName(
   RexxString * Name,                  /* starting filename                 */
   RexxString * Parent )               /* parent program                    */
{
  PCHAR     Extension;                 /* parent file extensions            */
  PCHAR     ExtensionArray[3];         /* array of extensions to check      */
  LONG      ExtensionCount;            /* count of extensions               */

  ExtensionCount = 0;                  /* Count of extensions               */
  if (Parent != OREF_NULL) {           /* have one from a parent activation?*/
                       /* check for a file extension        */
    Extension = SysFileExtension(Parent->stringData);
    if (Extension != NULL)             /* have an extension?                */
                       /* record this                       */
      ExtensionArray[ExtensionCount++] = Extension;
  }
                       /* next check for .REX               */
  ExtensionArray[ExtensionCount++] = DEFEXT;
                       /* then check for .CMD               */
  ExtensionArray[ExtensionCount++] = TEMPEXT;
                       /* go do the search                  */
  return LocateProgram(Name, ExtensionArray, ExtensionCount);
}

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : SysFileExtension                                    */
/*                                                                   */
/* DESCRIPTION : Looks for a file extension in given string. Returns */
/*               the ext in PSZ form. If no file ext returns an      */
/*               empty pointer.                                      */
/*                                                                   */
/*********************************************************************/

PCHAR SysFileExtension(
  PCHAR     Name )                     /* file name                         */
{
  PCHAR     Scan;                      /* scanning pointer                  */
  INT       Length;                    /* extension length                  */

  Scan = strrchr(Name, '\\');          /* have a path?                      */
  if (Scan)                            /* find one?                         */
    Scan++;                            /* step over last backspace          */
  else
    Scan = Name;                       /* no path, use name                 */

    /* Look for the last occurence of period in the name. If not            */
    /* found OR if found and the chars after the last period are all        */
    /* periods or spaces, then we do not have an extension.                 */

  if ((!(Scan = strrchr(Scan, '.'))) || strspn(Scan, ". ") == strlen(Scan))
    return NULL;                       /* just return a null                */

  Scan++;                              /* step over the period              */
  Length = strlen(Scan);               /* calculate residual length         */
  if (!Length)                         /* if no residual length             */
    return  NULL;                      /* so return null extension          */
  return --Scan;                       /* return extension position         */
}

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : LocateProgram                                       */
/*                                                                   */
/* DESCRIPTION : Finds out if file name is minimally correct. Finds  */
/*               out if file exists. If it exists, then produce      */
/*               fullpath name.                                      */
/*                                                                   */
/*********************************************************************/
RexxString * LocateProgram(
  RexxString * InName,                 /* name of rexx proc to check        */
  PCHAR        Extensions[],           /* array of extensions to check      */
  LONG         ExtensionCount )        /* count of extensions               */
{
  UCHAR      TempName[CCHMAXPATH + 2]; /* temporary name buffer             */
  UCHAR      FullName[CCHMAXPATH + 2]; /* temporary name buffer             */
  PCHAR        Name;                   /* ASCII-Z version of the name       */
  PCHAR        Extension;              /* start of file extension           */
  RexxString * Result;                 /* returned name                     */
  LONG         i;                      /* loop counter                      */
  LONG         ExtensionSpace;         /* room for an extension             */

  // retrofit by IH
  BOOL         Found;                  /* found the file                    */
  RexxActivity*activity;               /* the current activity              */

  activity = CurrentActivity;          /* save the activity                 */
  ReleaseKernelAccess(activity);       /* release the kernel access         */

  Name = InName->stringData;           /* point to the string data          */
  Found = FALSE;                       /* no name found yet                 */
  Extension = SysFileExtension(Name);  /* locate the file extension start   */

  if (!Extension) {                    /* have an extension?                */
                       /* get space left for an extension   */
    ExtensionSpace = sizeof(TempName) - strlen(Name);
                       /* loop through the extensions list  */
    for (i = 0; !Found && i < ExtensionCount; i++) {
                       /* copy over the name                */
      strncpy((PCHAR)TempName, Name, sizeof(TempName));
                       /* copy over the extension           */
      strncat((PCHAR)TempName, (PCHAR)Extensions[i], ExtensionSpace);
                       /* check this version of the name    */
      Found = SearchFileName((PCHAR)TempName, (PCHAR)FullName);
    }
  }
  if (!Found)                          /* not found?  try without extensions*/
                       /* check on the "raw" name last      */
    Found = SearchFileName(Name, (PCHAR)FullName);
  RequestKernelAccess(activity);       /* get the semaphore back            */
  if (Found)                           /* got one?                          */
                       /* get as a string object            */
    Result = new_cstring((PCHAR)FullName);
  else
    Result = OREF_NULL;                /* this wasn't found                 */
  return Result;                       /* return the name                   */
}

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : SearchFileName                                      */
/*                                                                   */
/* DESCRIPTION : Search for a given filename, returning the fully    */
/*               resolved name if it is found.                       */
/*                                                                   */
/*********************************************************************/

BOOL SearchFileName(
  PCHAR      Name,                     /* name of rexx proc to check        */
  PCHAR      FullName )                /* fully resolved name               */
{
  LONG       NameLength;               /* length of name                    */

  DWORD dwFileAttrib;             // file attributes
  LPTSTR ppszFilePart=NULL;            // file name only in buffer
  UINT   errorMode;

  NameLength = strlen(Name);           /* get length of incoming name       */

                       /* if name is too small or big       */
  if (NameLength < 1 || NameLength > CCHMAXPATH)
    return FALSE;                  /* then Not a rexx proc name         */
                       /* now try for original name         */
  errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
  if (GetFullPathName(Name, CCHMAXPATH, (LPTSTR)FullName, &ppszFilePart)) {
                       /* make sure it really exists        */
                       // make sure it's not a directory
     if (-1 != (dwFileAttrib=GetFileAttributes((LPTSTR)FullName))
    && (dwFileAttrib != FILE_ATTRIBUTE_DIRECTORY))
     {
                       /* got it! get its case-preserved long file name */
       GetLongName(FullName, CCHMAXPATH);
       SetErrorMode(errorMode);
       return TRUE;
     }
  }
                       /* try searching the path            */
  if ( SearchPath(NULL,                // search default order
          (LPCTSTR)Name,       // @ of filename
          NULL,                // @ of extension, no default
          CCHMAXPATH,          // len of buffer
          (LPTSTR)FullName,    // buffer for found
          &ppszFilePart) )
                       // make sure it's not a directory
     if (-1 != (dwFileAttrib=GetFileAttributes((LPTSTR)FullName))
    && (dwFileAttrib != FILE_ATTRIBUTE_DIRECTORY))
     {
                       /* got it! get its case-preserved long file name */
       GetLongName(FullName, CCHMAXPATH);
       SetErrorMode(errorMode);
       return TRUE;
     }

  SetErrorMode(errorMode);
  return FALSE;                    /* not found                         */
}

/****************************************************************************/
/* Function:  GetLongName()                                                 */
/*                                                                          */
/* Convert FullName to the original, case-preserved, long file name stored  */
/* by the file system.                                                      */
/*                                                                          */
/* Note:  The converted name is returned in the FullName buffer.  If any    */
/*        error occurs, the buffer remains unchanged.                       */
/****************************************************************************/
void GetLongName(
  PCHAR      FullName,    /* buffer containing a fully resolved path name   */
  DWORD      nSize )      /* size of FullName buffer must be >= CCHMAXPATH  */
{
  DWORD           length;
  WIN32_FIND_DATA findData;
  HANDLE          hFind;
  PCHAR           p;

  if ( nSize >= CCHMAXPATH ) {
    length = GetLongPathName(FullName, FullName, nSize);

    if ( 0 < length && length <= nSize )  {
      hFind = FindFirstFile(FullName, &findData);
      if ( hFind != INVALID_HANDLE_VALUE )  {
        p = strrchr(FullName, '\\');

        if ( p )
          strcpy(++p, findData.cFileName);
        FindClose(hFind);
      }
    }
  }
  return;
}

// retrofit by IH
void SysLoadImage(
  char **imageBuffer,                  /* returned start of the image       */
  long  *imageSize )                   /* size of the image                 */
/*******************************************************************/
/* Function:  Load the image into storage                          */
/*******************************************************************/
{
  CHAR      FullName[CCHMAXPATH + 2];  /* temporary name buffer             */
  HANDLE    fileHandle;                /* open file access handle           */
  ULONG     bytesRead;                 /* number of bytes read              */

  LPTSTR ppszFilePart=NULL;            // file name only in buffer

  if ( !SearchPath(NULL,                // search default order
          (LPCTSTR)BASEIMAGE,  // @ of filename
          NULL,                // @ of extension, no default
          CCHMAXPATH,          // len of buffer
          (LPTSTR)FullName,    // buffer for found
          &ppszFilePart) )
    logic_error("no startup image");   /* can't find it                     */

                       /* try to open the file              */
  fileHandle = CreateFile(FullName, GENERIC_READ, FILE_SHARE_READ,
                          NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);

  if (fileHandle == INVALID_HANDLE_VALUE)
    logic_error("no startup image");   /* can't find it                     */
                       /* Read in the size of the image     */
  ReadFile(fileHandle, imageSize, sizeof(long), &bytesRead, NULL);
  *imageBuffer = memoryObject.allocateImageBuffer(*imageSize);
                       /* read in the image                 */
  ReadFile(fileHandle, *imageBuffer, (ULONG)*imageSize, (ULONG *)imageSize, NULL);
  CloseHandle(fileHandle);                /* and close the file                */
}

// retrofit by IH
RexxBuffer *SysReadProgram(
  PCHAR file_name)                     /* program file name                 */
/*******************************************************************/
/* Function:  Read a program into a buffer                         */
/*******************************************************************/
{
  HANDLE        fileHandle;             /* open file access handle           */
  INT      buffersize;                 /* size of read buffer               */
  RexxBuffer * buffer;                 /* buffer object to read file into   */
  RexxActivity*activity;               /* the current activity              */
  BY_HANDLE_FILE_INFORMATION   status; /* file status information           */
  ULONG        bytesRead;              /* number of bytes read              */

  activity = CurrentActivity;          /* save the activity                 */
  ReleaseKernelAccess(activity);       /* release the kernel access         */
                       /* try to open the file              */
  fileHandle = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ,
                          NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    RequestKernelAccess(activity);     /* get the access back               */
    return OREF_NULL;                  /* return nothing                    */
  }
                       /* retrieve the file size            */
  GetFileInformationByHandle(fileHandle, &status);
  RequestKernelAccess(activity);       /* get the access back               */
  buffersize = status.nFileSizeLow;    /* get the file size                 */
  buffer = new_buffer(buffersize);     /* get a buffer object               */
  save(buffer);                        /* and protect this                  */
  ReleaseKernelAccess(activity);       /* release the kernel access         */
                       /* read in a buffer of data   */
  if (ReadFile(fileHandle, buffer->data, buffersize, &bytesRead, NULL) == 0) {
    RequestKernelAccess(activity);     /* get the access back               */
    discard_hold(buffer);              /* and release the protection        */
    return OREF_NULL;                  /* return nothing                    */
  }
  CloseHandle(fileHandle);                /* close the file now         */
  RequestKernelAccess(activity);       /* get the access back               */
  discard_hold(buffer);                /* and release the protection        */

  return buffer;                       /* return the program buffer         */
}

void SysQualifyStreamName(
  STREAM_INFO *stream_info )           /* stream information block          */
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
  LPTSTR  lpszLastNamePart;
  UINT errorMode;
                       /* already expanded?                 */
  if (stream_info->full_name_parameter[0] != '\0')
    return;                            /* nothing more to do                */
                       /* copy the name to full area        */
  strcpy(stream_info->full_name_parameter, stream_info->name_parameter);

  size_t namelen = strlen(stream_info->full_name_parameter);
                       /* name end in a colon?              */
  if (stream_info->full_name_parameter[namelen - 1] == ':') {
      // this could be the drive letter.  If so, make it the root of the current drive.
      if (namelen == 2)
      {
          strcat(stream_info->full_name_parameter, "\\");
      }
      else
      {
          // potentially a device, we need to remove the colon.
          stream_info->full_name_parameter[namelen - 1] = '\0';
          return;                            /* all finished                      */

      }
  }

  /* GetFullPathName doesn't work for COM? names without colon */
  if (RUNNING_95)
  {
      if (!strnicmp(stream_info->full_name_parameter, "com",3)
          && (stream_info->full_name_parameter[3] > '0') && (stream_info->full_name_parameter[3] <= '9'))
          return;
  }
                       /* get the fully expanded name       */
  errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
  GetFullPathName(stream_info->full_name_parameter, sizeof(stream_info->full_name_parameter), stream_info->full_name_parameter, &lpszLastNamePart);
  SetErrorMode(errorMode);
}

RexxString *SysQualifyFileSystemName(
  RexxString * name)                   /* stream information block          */
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
   STREAM_INFO stream_info;            /* stream information                */

                       /* clear out the block               */
   memset(&stream_info, 0, sizeof(STREAM_INFO));
                       /* initialize stream info structure  */
   strncpy(stream_info.name_parameter, name->stringData, path_length+10);
   strcpy(&stream_info.name_parameter[path_length+11], "\0");
   SysQualifyStreamName(&stream_info); /* expand the full name              */
                       /* uppercase this                    */
   strupr(stream_info.full_name_parameter);
                       /* get the qualified file name       */
   return new_cstring(stream_info.full_name_parameter);
}


BOOL SearchFirstFile(
  PCHAR      Name)                     /* name of file with wildcards       */
{
   HANDLE FindHandle;
   WIN32_FIND_DATA FindData;
   UINT errorMode;

   errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
   FindHandle = FindFirstFile(Name, &FindData);
   SetErrorMode(errorMode);

   if (FindHandle != INVALID_HANDLE_VALUE)
   {
      FindClose(FindHandle);
      if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
      || (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
      || (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
     return FALSE;
      else return TRUE;
   }
   else return FALSE;
}


FILE * SysBinaryFilemode(FILE * sfh, BOOL fRead)
{
     _setmode( _fileno( sfh ), _O_BINARY );
     return sfh;
}


BOOL SysFileIsDevice(int fhandle)
{
  if( _isatty( fhandle ) )
     return TRUE;
   else
     return FALSE;
}

int SysPeekKeyboard(void)
{
   return (_kbhit() != 0) ? 1 : 0;
}


INT SysStat(char * path, struct stat *buffer)
{
   UINT   errorMode;
   INT    retstat;

   errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
   retstat = stat(path, buffer);
   SetErrorMode(errorMode);
   return retstat;
}


BOOL SysFileIsPipe(STREAM_INFO * stream_info)
{
   struct _stat buf;

   if (_fstat( stream_info->fh, &buf )) return FALSE;
   else
       return (buf.st_mode & _S_IFIFO);
}


LONG SysTellPosition(STREAM_INFO * stream_info)
{
    if (SysFileIsDevice(stream_info->fh) || SysFileIsPipe(stream_info)) return -1;
    return ftell(stream_info->stream_file);
}

/* strem_info->stream_file -> sfile, tesul != length */
LONG line_write_check(char * buffer, LONG length, FILE * sfile)
{
   LONG result;
   result = fwrite(buffer,1,length,sfile);
   if ((result != length) && (ferror(sfile)) && (errno == ENOMEM))
   {
     ULONG ulMod;
     LONG ulTempValue;
     char *pTemp = buffer;
     clearerr(sfile);  /* clear memory err, give a new chance */
     ulTempValue  = length / MAX_STDOUT_LENGTH;
     ulMod        = length % MAX_STDOUT_LENGTH;
     while ((ulTempValue > 0) && (!ferror(sfile)))
     {
        result += fwrite(pTemp,1,MAX_STDOUT_LENGTH, sfile);
        pTemp += MAX_STDOUT_LENGTH;
        ulTempValue--;
     }
     if (!ferror(sfile))
       result += fwrite(pTemp,1,ulMod, sfile);
   }
   return result;
}

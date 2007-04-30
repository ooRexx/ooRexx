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
/* REXX AIX Support                                            aixfile.c      */
/*                                                                            */
/* AIX specific file related routines.                                        */
/*                                                                            */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxNativeAPI.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <limits.h>

#if defined( HAVE_SYS_FILIO_H )
# include <sys/filio.h>
#endif

#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_STROPTS_H
# include <stropts.h>
#endif

#define CCHMAXPATH PATH_MAX+1
#define DEFEXT  ".CMD"           /* leave default for AIX REXX programs too */
#define DEFEXT1  ".cmd"
//#define TEMPEXT ".ORX"         /* Temporary development extension   */

extern char achRexxCurDir[ CCHMAXPATH+2 ];    /* Save current working direct */

PCHAR SysFileExtension(PCHAR);
RexxString * LocateProgram(RexxString *, PCHAR[], LONG);
char * SearchFileName(PCHAR, char);
FILE * SysBinaryFilemode(FILE *);

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysResolveProgramName                        */
/*                                                                   */
/*   Function:          Expand a filename to a fully resolved REXX   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
RexxString *  SysResolveProgramName(
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
                                       /* then check for .CMD               */
  ExtensionArray[ExtensionCount++] = DEFEXT;
                                       /* and check for .cmd                */
  ExtensionArray[ExtensionCount++] = DEFEXT1;
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

  Scan = strrchr(Name, '/');           /* have a path?                      */
  if (Scan)                            /* find one?                         */
    Scan++;                            /* step over last slash in path      */
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

/**
 * Portable implementation of an ascii-z string to uppercase (in place).
 *
 * @param str    String argument
 *
 * @return The address of the str unput argument.
 */
void strlower(char *str)
{
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }

    return;
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

RexxString *  LocateProgram(
  RexxString * InName,                 /* name of rexx proc to check        */
  PCHAR      Extensions[],             /* array of extensions to check      */
  LONG       ExtensionCount )          /* count of extensions               */
{
  UCHAR      TempName[CCHMAXPATH + 2]; /* temporary name buffer             */
  PCHAR      Name;                     /* ASCII-Z version of the name       */
  PCHAR      Extension;                /* start of file extension           */
  PCHAR      Result;                   /* returned name                     */
  LONG       i;                        /* loop counter                      */
  LONG       ExtensionSpace;           /* room for an extension             */

  Name = InName->stringData;           /* point to the string data          */

                                       /* extract extension from name       */
  Extension = SysFileExtension(Name);  /* locate the file extension start   */

  if (Extension)                       /* have an extension?                */
    ExtensionCount = 0;                /* no further extension processing   */
  Result = SearchFileName(Name, 'P');  /* check on the "raw" name first     */
  if (Result != NULL)                  /* not found?  try adding extensions */
  {
      return new_cstring(Result);
  }
                                     /* get space left for an extension   */
  ExtensionSpace = sizeof(TempName) - strlen(Name);
                                     /* loop through the extensions list  */
  for (i = 0; i < ExtensionCount; i++) {
                                       /* copy over the name                */
      strncpy((PCHAR)TempName, Name, sizeof(TempName));
                                       /* copy over the extension           */
      strncat((PCHAR)TempName, (PCHAR)Extensions[i], ExtensionSpace);
                                       /* check on the "raw" name first     */
      Result = SearchFileName((PCHAR)TempName, 'P'); /* PATH search         */
      if (Result != NULL)                      /* not found?  try adding extensions */
      {
          return new_cstring(Result);
      }
      // try again in lower case
      strlower((char *)TempName);
                                       /* check on the "raw" name first     */
      Result = SearchFileName((PCHAR)TempName, 'P'); /* PATH search         */
      if (Result != NULL)                      /* not found?  try adding extensions */
      {
          return new_cstring(Result);
      }
  }
  return OREF_NULL;
}

/****************************************************************************/
/*                                                                          */
/* FUNCTION    : SearchFileName                                             */
/*                                                                          */
/* DESCRIPTION : Search for a given filename, returning the fully           */
/*               resolved name if it is found.                              */
/*               Control char A <=> Always return PATH+NAME w/o PATH search */
/*               Control char P <=> PATH search for existing file name      */
/*                                                                          */
/****************************************************************************/

char *SearchFileName(
  PCHAR      Name,                     /* name of rexx proc to check         */
  char       chCont )                  /* Control char for search and output */
{
  static CHAR       achFullName[CCHMAXPATH + 2]; /* temporary name buffer           */
  CHAR       achTempName[CCHMAXPATH + 2]; /* temporary name buffer           */
  PSZ        p;
  PSZ        q;
  PSZ        enddir;
  PSZ        pszPath;

  LONG       NameLength;               /* length of name                    */
  struct stat dummy;                   /* structure for stat system calls   */
  BOOL       found=0;

  NameLength = strlen(Name);           /* get length of incoming name       */
                                       /* if name is too small or big       */
  if (NameLength < 1 || NameLength > CCHMAXPATH)
    return OREF_NULL;                  /* then Not a rexx proc name         */

  /* If the name contains a '/', then there is directory info in the name.  */
  /* Get the absolute directory name not via chdir and getcwd, because of   */
  /* performance problems (lines, linein and lineout directory resolution). */

  if ((enddir=strrchr(Name,'/')))      /* If there's directory info, enddir */
  {                                    /* points to end of directory name   */
    p = &achTempName[0];
    memcpy(p, Name, enddir-Name);      /* Copy path from name to p          */
    *(p+(enddir-Name))   = '\0';       /* Null-terminate it                 */
    *(p+(enddir-Name)+1) = '\0';       /* Null-terminate the search of >.<  */
    achFullName[0] = '\0';             /* Reset                             */

    switch(*p)
    {
      case '~':
           if ( *(p+1) == '\0' )
           {
              strcpy( achFullName, getenv("HOME"));
              strncat( achFullName, Name+1,
                                         ( CCHMAXPATH - strlen(achFullName)) );
              break;
           }
           if ( *(p+1) == '/' )
           {
              strcpy( achFullName, getenv("HOME"));
              p = p + 2;
           }
      case '.':
           if ( *(p+1) == '\0' )
           {
              strcpy(achFullName, achRexxCurDir);
              strncat( achFullName, (Name+1),
                                         ( CCHMAXPATH - strlen(achFullName)) );
              break;
           }
           if ( *(p+1) == '/' )
           {
              strcpy(achFullName, achRexxCurDir);
              p = p + 2;
           }
           if ( ( *(p+1) == '.' ) && ( *(p+2) == '\0' ) )
           {
              p = p + 2;
              if ( achFullName[0] == '\0' )
              {
                enddir=strrchr(achRexxCurDir,'/');              /* Copy path */
                memcpy(achFullName, achRexxCurDir, enddir-(&achRexxCurDir[0]));
                achFullName[enddir-(&achRexxCurDir[0])] = '\0'; /* Terminate */
              } else {
                enddir=strrchr(achFullName,'/');              /* Copy path */
                achFullName[enddir-(&achFullName[0])] = '\0'; /* Terminate */
              }
              strncat( achFullName, (Name+(p-(&achTempName[0]))),
                                         ( CCHMAXPATH - strlen(achFullName)) );
              break;
           }
           if ( ( *(p+1) == '.' ) && ( *(p+2) == '/' ) )
           {
              p = p + 3;
              if ( achFullName[0] == '\0' )
              {
                enddir=strrchr(achRexxCurDir,'/');              /* Copy path */
                memcpy(achFullName, achRexxCurDir, enddir-(&achRexxCurDir[0]));
                achFullName[enddir-(&achRexxCurDir[0])] = '\0'; /* Terminate */
              } else {
                enddir=strrchr(achFullName,'/');              /* Copy path */
                achFullName[enddir-(&achFullName[0])] = '\0'; /* Terminate */
              }
              for ( ;
                   ( ( *p == '.' ) && ( *(p+1) == '.' ) );
                   p = p + 3)
              {
                 if (enddir=strrchr(achFullName,'/'))            /* Copy path */
                   achFullName[enddir-(&achFullName[0])] = '\0'; /* Terminate */
              }
           }
           strncat( achFullName, (Name+(p-(&achTempName[0]))-1),
                                         ( CCHMAXPATH - strlen(achFullName)) );
        break;
      default:
              strcpy(achFullName, Name);
    }  /* endswitch */

    if (stat(achFullName,&dummy))             /* look for file              */
      found = 0;                              /* Give up - we can't find it */
    else                                      /*                            */
      found = 1;                              /* Tell user we found it      */

    if (found || (chCont == 'A'))
      return (PCHAR)achFullName;
    else
      return OREF_NULL;

  }                                       /* End if dir info present    */

  /* Otherwise, there's no directory info, so we must use the PATH      */
  /*             environment variables to find the file.                */

  if ((!stat(Name, &dummy)) || (chCont == 'A'))  /* First try current dir      */
  {
    strcpy(achFullName, achRexxCurDir);   /* Copy current directory in  */
    strcat(achFullName,"/");              /* Put in a final slash       */
    strcat(achFullName, Name);            /* Now add name to end        */
    found = 1;                            /* Tell caller we found it    */
  }
  if (!found && (chCont == 'P' ))         /* Not in current dir         */
  {
    pszPath = getenv("PATH");             /* Get PATH                   */
    if (!pszPath)                         /* No PATH or REXXPATH?       */
    {
      return OREF_NULL;                   /* Didn't find the file       */
    }
    /* Now we have one contiguous string with all the directories       */
    /* that must be searched listed in order.                           */
    NameLength = strlen(pszPath);
    found = 0;
                                          /* For every dir in searchpath*/
    for (p=pszPath, q = strchr(p,':');
         p < pszPath+NameLength;
         p = q+1, q = strchr(p,':')) {
      if (!q)                             /* Is there a terminating ':'?*/
        q = p + strlen(p);                /* Make q point to \0         */
      memcpy(achTempName, p, q-p);        /* Copy this dir to tempname  */
      achTempName[q-p] = '/';             /* End dir with slash         */
      strcpy(&achTempName[q-p+1], Name);  /* Append name                */

      /* If we do find this file, it's possible that the directory      */
      /* that we used may not have been a complete directory            */
      /* specification from the root.  If so, as above, we must         */
      /* get the full directory.                                        */
      if ( achTempName[0] == '~' )
      {
          strcpy( achFullName, getenv("HOME"));
          strcat( achFullName, &achTempName[1]);
      }
      else
         strcpy(achFullName, achTempName);

      if (!stat(achFullName, &dummy))     /* If file is found,          */
      {
        found = 1;                        /* Tell user we found it      */
        break;                            /* Break out of loop          */
      }  /* endif */
    }  /* endfor */
  } /* endif */

  if (found)
    return (PCHAR)achFullName;
  else
    return OREF_NULL;

}

/************ Commented out by Weigold, since moved to the 'SysLoadImage' - function **************/
//FILE *SysOpenImage(void)
/*********************************************************************/
/*                                                                   */
/* FUNCTION    : SysOpenImage                                        */
/*                                                                   */
/* DESCRIPTION : Search for and locate the image file for a restore  */
/*               image process.                                      */
/*                                                                   */
/*********************************************************************/
//{
//  CHAR      FullName[CCHMAXPATH + 2];  /* temporary name buffer             */
//  RexxString * imgpath;
//  PCHAR      fullname;
//
//  imgpath = SearchFileName((PCHAR)BASEIMAGE);
//  fullname = (PSZ)string_data(imgpath);
//
//  if (fullname)
//       return fopen((PSZ)fullname, "rb");    /* try to open the file        */
//    else
//       return NULL;                          /* return an open failure      */
//}


void SysLoadImage(char **imageBuffer, long *imageSize)
/*******************************************************************/
/* Function : Load the image into storage                          */
/*******************************************************************/
{
  FILE *image = NULL;
  PCHAR      fullname;
//RexxString * imgpath;

  fullname = SearchFileName((PCHAR)BASEIMAGE, 'P');  /* PATH search         */

#ifdef ORX_CATDIR
  if( fullname == OREF_NULL ) {
      fullname = ORX_CATDIR"/rexx.img";
  }
#endif

//  fullname = (PSZ)string_data(imgpath);

//if ( imgpath && fullname )                         /* seg faultn          */
  if ( fullname != OREF_NULL )
    image = fopen((PSZ)fullname, "rb");/* try to open the file              */
  else
    logic_error("no startup image");   /* open failure                      */

  if( image == NULL )
      logic_error("unable to open image file");


                                       /* Read in the size of the image     */
  if(!fread(imageSize, 1, sizeof(long), image))
    logic_error("could not check the size of the image");
                                       /* Create new segment for image      */
//memoryObject.newOldSegment(*imageSize);
  *imageBuffer = (char *)memoryObject.allocateImageBuffer(*imageSize);
                                       /* Create an object the size of the  */
                                       /* image. We will be overwriting the */
                                       /* object header.                    */
//*imageBuffer = (char *)memoryObject.oldObject(*imageSize);
                                       /* read in the image, store the      */
                                       /* the size read                     */
  if(!(*imageSize = fread(*imageBuffer, 1, *imageSize, image)))
    logic_error("could not read in the image");
  fclose(image);                       /* and close the file                */
}


RexxBuffer *SysReadProgram(
  PCHAR file_name)                     /* program file name                 */
/*******************************************************************/
/* Function:  Read a program into a buffer                         */
/*******************************************************************/
{
  FILE    *handle;                     /* open file access handle           */
  INT      buffersize;                 /* size of read buffer               */
  RexxBuffer * buffer;                 /* buffer object to read file into   */
  RexxActivity*activity;               /* the current activity              */

  activity = CurrentActivity;          /* save the activity                 */
  ReleaseKernelAccess(activity);       /* release the kernel access         */

  handle = fopen(file_name, "rb");     /* open as a binary file             */
  if (handle == NULL){                 /* open error?                       */
    RequestKernelAccess(activity);     /* get the access back               */
    return OREF_NULL;                  /* return nothing                    */
  }

  if (fileno(handle) == (FOPEN_MAX - 2)){      /* open error?                       */
    RequestKernelAccess(activity);     /* get the access back               */
    return OREF_NULL;                  /* return nothing                    */
  }

  fseek(handle, 0, SEEK_END);          /* seek to the file end              */
  RequestKernelAccess(activity);       /* get the access back               */
  buffersize = ftell(handle);          /* get the file size                 */
  fseek(handle, 0, SEEK_SET);          /* seek back to the file beginning   */
  buffer = new_buffer(buffersize);     /* get a buffer object               */
  save(buffer);                        /* and protect this                  */
  ReleaseKernelAccess(activity);       /* release the kernel access         */
                                       /* read the entire file in one shot  */
  fread(buffer->data, 1, buffersize, handle);
  fclose(handle);                      /* close the file                    */
  RequestKernelAccess(activity);       /* get the access back               */
  discard_hold(buffer);                /* and release the protection        */
  return buffer;                       /* return the program buffer         */
}

#include "StreamNative.h"              /* include the stream information    */

void SysQualifyStreamName(
  STREAM_INFO *stream_info )           /* stream information block          */
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
//  RexxString *something;               /* place to save RexxString from the */
  char *something;               /* place to save RexxString from the */
                                       /* call to SearchFileName()          */

                                       /* already expanded?                 */
  if (stream_info->name_parameter[0] == '/')
  {                                    /* nothing more to do                */
                                       /* copy the name to full area        */
     strcpy(stream_info->full_name_parameter, stream_info->name_parameter);
                                       /* name end in a colon?              */
     if (stream_info->full_name_parameter[strlen(stream_info->full_name_parameter)-1] == ':')
                                       /* this is a special name...lop off  */
                                       /* the colon...                      */
       stream_info->full_name_parameter[strlen(stream_info->full_name_parameter)-1] = '\0';
     return;                           /* all finished                      */
  }
                                       /* get Always the fully expanded name */
//  something = SearchFileName(stream_info->name_parameter, 'A'); /* returns RexxString * */
  something = SearchFileName(stream_info->name_parameter, 'A'); /* returns RexxString * */
  if (something != OREF_NULL)
  {                           // copy expanded name to stream_info structure
    strncpy(stream_info->full_name_parameter, something, strlen(something));
                              //  insert a null at end
    stream_info->full_name_parameter[strlen(something)] = '\0';
  }
  return;
}

RexxString *SysQualifyFileSystemName(
                        RexxString * name )  /* stream information block   */
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

                                       /* get the qualified file name       */
   return new_cstring(stream_info.full_name_parameter);
}

BOOL SearchFirstFile(
  PCHAR Name)                     /* name of file with wildcards       */
{


  /**************** Dummy *********************/

/*  HDIR          hdirFindHandle = HDIR_SYSTEM;
 *  FILEFINDBUF3  FindBuffer     = {0};
 *  ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);
 *  ULONG         ulFindCount    = 1;
 *
 *  return (DosFindFirst( Name,
 *                        &hdirFindHandle,
 *                        FILE_ARCHIVED | FILE_READONLY,
 *                        &FindBuffer,
 *                        ulResultBufLen,
 *                        &ulFindCount,
 *  ************          FIL_STANDARD) == NO_ERROR); */
    return(0);
}



/* In POSIX systems(including the GNU system) there is no difference between */
/* opening a file for binary or for text writing                             */

FILE * SysBinaryFilemode(FILE * fh,BOOL fRead)
{
  return fh;                        /* dummy funcion !    */
}

//BOOL SysFileIsDevice(int fhandle)
//{
//  struct stat info;                 /* file info buffer */
//
//  fstat(fhandle,&info);             /* get the info     */
//  if(S_ISREG(info.st_mode))         /* if it's a file   */
//    return FALSE;
//  else                              /* it's the keyboard*/
//    return TRUE;
//}

BOOL SysFileIsDevice(int fhandle)
{
  if( isatty( fhandle ) )
    return TRUE;
  else
    return FALSE;
}

int SysPeekKeyboard(void)
{
  struct stat info;                 /* file info buffer */
  int rc;

//fstat(0,&info);                   /* get the info              */
  rc = fstat(STDIN_FILENO,&info);   /* get the info              */
  if(info.st_size)
    return (1);
  else
    return (0);
}

#if defined( FIONREAD )
int SysPeekSTD(STREAM_INFO *stream_info)
{
  int c;
  int rc;

  /* ioctl returns number of fully received bytes from keyboard, after        */
  /* the Enter key has been hit. After the first byte has been read with      */
  /* charin, ioctl returns '0'. stream_file->_cnt returns '0' until a first   */
  /* charin has buffered input from STDIN. After that _cnt returns the num-   */
  /* of still buffered characters. If new input arrives through STDIN, the    */
  /* already buffered input is worked off, _cnt gets '0' and with that,       */
  /* ioctl returns a non zero value. With the first charin, the logic repeats */

  ioctl(stream_info->fh, FIONREAD, &c);
#if defined( HAVE_FILE__IO_READ_PTR )
  if ( (!c) && (!(stream_info->stream_file->_IO_read_ptr <
      stream_info->stream_file->_IO_read_end)) )
#elif defined( HAVE_FILE__CNT )
  if( (!c) && (!stream_info->stream_file->_cnt) )
#else
  if ( !c ) /* not sure what to do here ? */
#endif
   return(0);
  else
   return(1);
}
#endif

#if 0
// this function no longer used - MHES 20050124
/* function for SysPeekKeyboard() to read from STDIN buffer */
int SysPeekSTDIN(STREAM_INFO *stream_info)
{
  struct stat info;                 /* File info buffer */
  long lRc = 0;

  lRc = fstat(0,&info);             /* Get the info     */
  if ( (lRc == 0) && info.st_size)  /* Stream is open   */
     return (1);
                                    /* Buffer is empty  */
#if defined(AIX) || defined(OPSYS_SUN)
  if ( stream_info->stream_file->_cnt != 0 )
#endif
#if defined(LINUX) && !defined(OPSYS_SUN)
  if (stream_info->stream_file->_IO_read_ptr <
      stream_info->stream_file->_IO_read_end )
#endif
     return (1);
  else
     return (0);
}
#endif

int SysStat(char * PathName, struct stat * Buffer)
{
  int rc;

  rc = stat(PathName, Buffer);
  return rc;
}

BOOL SysFileIsPipe(STREAM_INFO * stream_info)
{
   struct stat buf;

   if (fstat(stream_info->fh, &buf ))
     return FALSE;
   else
     return (S_ISFIFO(buf.st_mode));
}


LONG SysTellPosition(STREAM_INFO * stream_info)
{                                                 /* flags.std has been changed */
   if (stream_info->flags.bstd || stream_info->fh == stdin_handle) return -1;
    return ftell(stream_info->stream_file);
}




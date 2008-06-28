/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Unix implementation of the SysFileSystem class.                            */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "SysFileSystem.hpp"


// This is all the static stuff
const int SysFileSystem::stdinHandle = 0;
const int SysFileSystem::stdoutHandle = 1;
const int SysFileSystem::stderrHandle = 2;

const char SysFileSystem::EOF_Marker = 0x1A;
const char *SysFileSystem::EOL_Marker = "\n";
const char SysFileSystem::PathDelimiter = '/';




/*********************************************************************/
/*                                                                   */
/* FUNCTION    : SearchFileName                                      */
/*                                                                   */
/* DESCRIPTION : Search for a given filename, returning the fully    */
/*               resolved name if it is found.                       */
/*                                                                   */
/*********************************************************************/

bool SysFileSystem::searchFileName(
  char *     name,                     /* name of rexx proc to check        */
  char *     fullName )                /* fully resolved name               */
{
  size_t nameLength;                   /* length of name                    */
  char tempPath[MaximumFileNameBuffer];// temporary place to store the path/name
  char * currentpath;                  // current path
  char * sep;                          // next colon in the path

  nameLength = strlen(name);           /* get length of incoming name       */

  /* if name is too small or big */
  if (nameLength < 1 || nameLength > MaximumFileNameBuffer)
    return false;

  /* does the filename already have a path? */
  if (strstr(name, "/") != NULL) {
      switch (*name) {
      case '~':
          strcpy(tempPath, getenv("HOME"));
          strcat(tempPath, name + 1);
          break;
      case '.':
          getcwd(tempPath, MaximumFileNameBuffer);
          strcat(tempPath, name + 1);
          break;
      case '/':
          strcpy(tempPath, name);
          break;
      default:
          getcwd(tempPath, MaximumFileNameBuffer);
          strcat(tempPath, "/");
          strcat(tempPath, name);
          break;
      }
      if (fileExists(tempPath) == false) {
          strcpy(fullName, tempPath);
          return false;
      }
      return true;
  }

  /* there was no leading path so try the current directory */
  getcwd(tempPath, MaximumFileNameBuffer);
  strcat(tempPath, "/");
  strcat(tempPath, name);
  if (fileExists(name) == false) {
      strcpy(fullName, name);
      return false;
  }

  /* it was not in the current directory so search the PATH */
  currentpath = getenv("PATH");
  if (currentpath == NULL) {
      return true;
  }
  sep = strchr(currentpath, ':');
  while (sep != NULL) {
      /* try each entry in the PATH */
      int i = sep - currentpath;
      strncpy(tempPath, currentpath, i);
      tempPath[i] = '\0';
      strcat(tempPath, "/");
      strcat(tempPath, name);
      if (fileExists(tempPath) == false) {
          strcpy(fullName, tempPath);
          return false;
      }
      currentpath = sep + 1;
      sep = strchr(currentpath, ':');
  }
  /* the last entry in the PATH may not be terminated by a colon */
  if (*currentpath != '\0') {
      strcpy(tempPath, currentpath);
      strcat(tempPath, "/");
      strcat(tempPath, name);
      if (fileExists(tempPath) == false) {
          strcpy(fullName, tempPath);
          return false;
      }
  }

  /* file not found */
  return false;
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

char *SysFileSystem::extractFileExtension(
  char     *name )                     /* file name                         */
{
  char     *scan;                      /* scanning pointer                  */
  size_t    length;                    /* extension length                  */

  scan = strrchr(name, '/');          /* have a path?                      */
  if (scan)                            /* find one?                         */
    scan++;                            /* step over last backspace          */
  else
    scan = name;                       /* no path, use name                 */

    /* Look for the last occurence of period in the name. If not            */
    /* found OR if found and the chars after the last period are all        */
    /* periods or spaces, then we do not have an extension.                 */

  if ((!(scan = strrchr(scan, '.'))) || strspn(scan, ". ") == strlen(scan))
    return NULL;                       /* just return a null                */

  scan++;                              /* step over the period              */
  length = strlen(scan);               /* calculate residual length         */
  if (!length)                         /* if no residual length             */
    return  NULL;                      /* so return null extension          */
  return --scan;                       /* return extension position         */
}

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : getTempFileName                                     */
/*                                                                   */
/* DESCRIPTION : Returns a temp file name.                           */
/*                                                                   */
/*********************************************************************/

char *SysFileSystem::getTempFileName()
{
    char tempName[11];

    strcpy(tempName, "tempXXXXXX");
    return mktemp(tempName);
}

void SysFileSystem::qualifyStreamName(
  char *name,                         // input name
  char *fullName,                     // the output name
  size_t bufferSize)                  // size of the output buffer
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
    char tempPath[MaximumFileNameBuffer];// temporary place to store the path/name

  /* already expanded? */
  if (*fullName != '\0')
  {
      return;                            /* nothing more to do                */
  }

  /* does the filename already have a path? */
  if (strstr(name, "/") != NULL) {
      switch (*name) {
      case '~':
          strcpy(tempPath, getenv("HOME"));
          strcat(tempPath, name + 1);
          break;
      case '.':
          getcwd(tempPath, MaximumFileNameBuffer);
          strcat(tempPath, name + 1);
          break;
      case '/':
          strcpy(tempPath, name);
          break;
      default:
          getcwd(tempPath, MaximumFileNameBuffer);
          strcat(tempPath, "/");
          strcat(tempPath, name);
          break;
      }
      if (strlen(tempPath) < bufferSize) {
          strcpy(fullName, tempPath);
      }
      else {
          *fullName = '\0';
      }
      return;
  }

  /* there was no leading path so try the current directory */
  getcwd(tempPath, MaximumFileNameBuffer);
  strcat(tempPath, "/");
  strcat(tempPath, name);
  if (strlen(tempPath) < bufferSize) {
      strcpy(fullName, tempPath);
  }
  else {
      *fullName = '\0';
  }
  return;
}


bool SysFileSystem::findFirstFile(
    char *name)                     /* name of file with wildcards       */
{
    return false;
}


bool SysFileSystem::fileExists(char * fname)
{
    struct stat filestat;                // file attributes
    int rc;                              // stat function return code

    rc = stat(fname, &filestat);
    if (rc == 0) {
        if (S_ISREG(filestat.st_mode)) {
           return false;
        }
    }
    return true;
}







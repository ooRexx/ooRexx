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
/* REXX Kernel                                              SysFileSystem.cpp */
/*                                                                            */
/* Windows implementation of the SysFileSystem class.                         */
/*                                                                            */
/******************************************************************************/

#include "windows.h"
#include "SysFileSystem.hpp"

int SysFileSystem::stdinHandle = 0;
int SysFileSystem::stdoutHandle = 1;
int SysFileSystem::stderrHandle = 2;

const char SysFileSystem::EOF_Marker = 0x1a;    // the end-of-file marker
const char *SysFileSystem::EOL_Marker = "\r\n"; // the end-of-line marker
const char SysFileSystem::PathDelimiter = '\\'; // directory path delimiter

/*********************************************************************/
/*                                                                   */
/* FUNCTION    : SearchFileName                                      */
/*                                                                   */
/* DESCRIPTION : Search for a given filename, returning the fully    */
/*               resolved name if it is found.                       */
/*                                                                   */
/*********************************************************************/

bool SysFileSystem::searchFileName(
  const char *     name,               /* name of rexx proc to check        */
  char *     fullName )                /* fully resolved name               */
{
    size_t nameLength;                   /* length of name                    */

    DWORD dwFileAttrib;                  // file attributes
    LPTSTR ppszFilePart=NULL;            // file name only in buffer
    UINT   errorMode;

    nameLength = strlen(name);           /* get length of incoming name       */

    /* if name is too small or big       */
    if (nameLength < 1 || nameLength > MAX_PATH)
    {
        return false;                  /* then Not a rexx proc name         */
    }
    /* now try for original name         */
    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (GetFullPathName(name, MAX_PATH, (LPTSTR)fullName, &ppszFilePart))
    {
        /* make sure it really exists        */
        // make sure it's not a directory
        if (-1 != (dwFileAttrib=GetFileAttributes((LPTSTR)fullName)) && (dwFileAttrib != FILE_ATTRIBUTE_DIRECTORY))
        {
            /* got it!                           */
            SetErrorMode(errorMode);
            return true;
        }
    }
    /* try searching the path            */
    if ( SearchPath(NULL, (LPCTSTR)name, NULL, MAX_PATH, (LPTSTR)fullName, &ppszFilePart) )
    {
        // make sure it's not a directory
        if (-1 != (dwFileAttrib=GetFileAttributes((LPTSTR)fullName)) && (dwFileAttrib != FILE_ATTRIBUTE_DIRECTORY))
        {
            /* got it!                           */
            SetErrorMode(errorMode);
            return true;
        }
    }

    SetErrorMode(errorMode);
    return false;                    /* not found                         */
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
    return tmpnam(NULL);
}

void SysFileSystem::qualifyStreamName(
  const char *unqualifiedName,              // input name
  char *qualifiedName,                // the output name
  size_t bufferSize)                  // size of the output buffer
/*******************************************************************/
/* Function:  Qualify a stream name for this system                */
/*******************************************************************/
{
    LPTSTR  lpszLastNamePart;
    UINT errorMode;
    /* already expanded?                 */
    if (qualifiedName[0] != '\0')
    {
        return;                            /* nothing more to do                */
    }
    /* copy the name to full area        */
    strcpy(qualifiedName, unqualifiedName);

    size_t namelen = strlen(qualifiedName);
    /* name end in a colon?              */
    if (qualifiedName[namelen - 1] == ':')
    {
        // this could be the drive letter.  If so, make it the root of the current drive.
        if (namelen == 2)
        {
            strcat(qualifiedName, "\\");
        }
        else
        {
            // potentially a device, we need to remove the colon.
            qualifiedName[namelen - 1] = '\0';
            return;                            /* all finished                      */

        }
    }
    /* get the fully expanded name       */
    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    GetFullPathName(qualifiedName, (DWORD)bufferSize, qualifiedName, &lpszLastNamePart);
    SetErrorMode(errorMode);
}


bool SysFileSystem::findFirstFile(const char *name)
{
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    UINT errorMode;

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    FindHandle = FindFirstFile(name, &FindData);
    SetErrorMode(errorMode);

    if (FindHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(FindHandle);
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            || (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            || (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

/**
 * Check to see if a file with a given name exists.
 *
 * @param name   The name to check.
 *
 * @return True if the file exists, false otherwise.
 */
bool SysFileSystem::fileExists(const char *name)
{
    return findFirstFile(name);
}


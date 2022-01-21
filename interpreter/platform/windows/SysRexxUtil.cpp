/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/*                                                                            */
/* Windows-specific RexxUtil functions                                        */
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
*       SysGetKey           -- Returns one by of data indicating the  *
*                              key which was pressed,                 *
*                              in a command prompt session            *
*       SysIni              -- Reads and/or updates entries in .INI   *
*                              files.                                 *
*       SysMkDir            -- Creates a directory                    *
*       SysWinVer           -- Returns the Win OS and Version number  *
*       SysVersion          -- Returns the OS and Version number      *
*       SysTextScreenRead   -- Reads characters from the screen,      *
*                              in a command prompt session            *
*                              command prompt session.                *
*       SysTextScreenSize   -- Returns the size of the window in      *
*                              rows and columns, in a                 *
*                              command prompt session.                *
*       SysWaitNamedPipe    -- Wait on a named pipe.                  *
*       SysBootDrive        -- Return the windows boot drive          *
*       SysSystemDirectory  -- Return the Windows system directory    *
*       SysFileSystemType   -- Return drive file system type          *
*       SysVolumeLabel      -- Return the drive label                 *
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
*       SysShutDownSystem   -- Shutdown the system                    *
*       SysSwitchSession    -- Switch to a named session              *
*       SysQueryProcess     -- Get information on current proc/thread *
*       SysSetFileDateTime  -- Set the last modified date of a file   *
*       SysGetFileDateTime  -- Get the last modified date of a file   *
*       SysUtilVersion      -- query version of REXXUTIL.DLL          *
*       SysWinFileEncrypt   -- Encrypt file on a W2K-NTFS             *
*       SysWinFileDecrypt   -- Decrypt file on a W2K-NTFS             *
*       SysGetErrorText     -- Retrieve textual desc. of error number *
*       SysWinGetDefaultPrinter -- retrieve default printer           *
*       SysWinGetPrinters   -- Obtain list of local printers          *
*       SysWinSetDefaultPrinter -- set the local default printer      *
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
#include <versionhelpers.h>
#include "SysFileSystem.hpp"
#include "RexxUtilCommon.hpp"
#include "FileNameBuffer.hpp"
#include "Utilities.hpp"



/*********************************************************************/
/* Saved character status                                            */
/*********************************************************************/
static   int   ExtendedFlag = 0;       /* extended character saved   */
static   char  ExtendedChar;           /* saved extended character   */

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
RexxRoutine0(int, SysCls)
{
    DWORD dummy;
    COORD Home = { 0, 0 };                 /* Home coordinates on console*/
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; /* Console information        */

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    /* if in character mode       */
    if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
    {
        FillConsoleOutputCharacter(hStdout, ' ',
                                   csbiInfo.dwSize.X * csbiInfo.dwSize.Y,
                                   Home, &dummy);
        SetConsoleCursorPosition(hStdout, Home);
    }

    return 0;
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

        snprintf(buffer, sizeof(buffer), "%d %d", csbiInfo.dwCursorPosition.Y, csbiInfo.dwCursorPosition.X);

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
    GetConsoleCursorInfo(hStdout, &CursorInfo);

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
        invalidOptionException(context, "SysCurState", "option", "'ON' or 'OFF'", option);
    }
    /* Set the cursor info        */
    SetConsoleCursorInfo(hStdout, &CursorInfo);
    return 0;                            /* no error on call           */
}


/*************************************************************************
* Function:  SysDriveInfo - returns total number of free bytes,          *
*                           total number of bytes, and                   *
*                           volume label                                 *
*                                                                        *
* Syntax:    result = SysDriveInfo([drive])                              *
*                                                                        *
* Params:    drive - d, d:, d:\, \\share\path                            *
*                    if omitted, defaults to current drive               *
*                                                                        *
* Return:    result - drive free total label                             *
*                     null string for any error                          *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysDriveInfo, OPTIONAL_CSTRING, drive)
{
    FileNameBuffer d;

    if (drive == NULL)
    {
        // Although the Windows APIs interpret NULL as the current drive,
        // we still need to figure out the actual current drive as we are
        // expected to return it as the first word.
        GetCurrentDirectory((DWORD)d.capacity(), (LPTSTR)d);
        if (d.length() >= 3 && d.at(1) == ':' && d.at(2) == '\\')
        {
            // current directory is a standard d:\path, just keep the d:\ part
            d.truncate(3);
        }
        else if (d.startsWith("\\\\"))
        {
            // current directory is a UNC path, just keep the \\share\path\ part
            d.truncate(d.locatePathDelimiter(d.locatePathDelimiter(2) + 1));
        }
    }
    else
    {
        d = drive;
        // We just let the Windows APIs handle all the different ways to
        // specify a volume, like d:\, \\?\d:, \\localhost\path, etc.  But
        // on top of that we support a single-character drive d.
        if (d.length() == 1 && Utilities::isAlpha(d.at(0)))
        {
            // make this a valid drive specification
            d += ":\\";
        }
    }

    // GetVolumeInformation() is picky, it requires a final backslash
    if (d.length() > 0)
    {
        d.addFinalPathDelimiter();
    }

    // get the volume label, then the total bytes and total free bytes
    char volume[MAX_PATH];
    uint64_t total, free;
    if (GetVolumeInformation((char * )d, volume, (DWORD)sizeof(volume), NULL, NULL, NULL, NULL, 0) &&
        GetDiskFreeSpaceEx(d, NULL, (PULARGE_INTEGER)&total, (PULARGE_INTEGER)&free))
    {
        // we remove any trailing backslash for our return string
        if (d.length() > 1 && d.endsWith("\\"))
        {
            d.truncate(d.length() - 1);
        }

        // return drive free total label
        char retstr[256];
        snprintf(retstr, sizeof(retstr), "%s %I64u %I64u %s", (char *)d, free, total, volume);
        return context->String(retstr);
    }
    return context->NullString();
}


/*********************************************************************/
/*  Defines used by SysDriveMap                                      */
/*********************************************************************/

#define  USED           0
#define  FREE           1
#define  SPECIFIC       3


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

RexxRoutine2(RexxStringObject, SysDriveMap, OPTIONAL_CSTRING, drive, OPTIONAL_CSTRING, mode)
{
    ULONG selectMode = USED;             // Query mode USED, FREE,
    ULONG driveType = DRIVE_UNKNOWN;     // checking for a particular type of drive
    ULONG ordinal = 0;                   // Ordinal of entry in name list required

    char     temp[MAX_PATH];             // Entire drive map built here
    temp[0] = '\0';
    /* check starting drive letter*/
    ULONG start = 3;
    if (drive != NULL)
    {
        size_t driveLength = strlen(drive);

        if (driveLength == 0 || driveLength > 2 || (driveLength == 2 && drive[1] != ':'))
        {
            context->ThrowException1(Rexx_Error_Incorrect_call_user_defined, context->String("Invalid drive specification"));
        }

        // make sure this is in range
        start = Utilities::toUpper(drive[0]) - 'A' + 1;
        if (start < 1 || start > 26)
        {
            context->ThrowException1(Rexx_Error_Incorrect_call_user_defined, context->String("Invalid drive specification"));
        }
    }
    /* check the mode             */
    if (mode != NULL)
    {
        if (!stricmp(mode, "FREE"))
        {
            selectMode = FREE;
        }
        else if (!stricmp(mode, "USED"))
        {
            selectMode = USED;
        }
        else if (!stricmp(mode, "RAMDISK"))
        {
            selectMode = SPECIFIC;
            driveType = DRIVE_RAMDISK;
        }
        else if (!stricmp(mode, "REMOTE"))
        {
            selectMode = SPECIFIC;
            driveType = DRIVE_REMOTE;
        }
        // local drives are really fixed ones
        else if (!stricmp(mode, "LOCAL"))
        {
            selectMode = SPECIFIC;
            driveType = DRIVE_FIXED;
        }
        else if (!stricmp(mode, "REMOVABLE"))
        {
            selectMode = SPECIFIC;
            driveType = DRIVE_REMOVABLE;
        }
        else if (!stricmp(mode, "CDROM"))
        {
            selectMode = SPECIFIC;
            driveType = DRIVE_CDROM;
        }
        else
        {
            context->ThrowException1(Rexx_Error_Incorrect_call_user_defined, context->String("Invalid drive type"));
        }
    }

    /* perform the query          */
    DWORD driveMap = GetLogicalDrives();
    driveMap >>= start - 1;              // Shift to the first drive
    FileNameBuffer driveMapBuffer;       // the map is built here

    char driveStr[8];                    // just enough space to format a drive specifier

    for (ULONG dnum = start; dnum <= 26; dnum++)
    {
        // Hey, we have a free drive
        if (!(driveMap & (DWORD)1))
        {
            // is this what we're looking for?
            if (selectMode == FREE)
            {
                snprintf(driveStr, sizeof(driveStr), "%c: ", dnum + 'A' - 1);
                driveMapBuffer += driveStr;
            }
        }
        /* Hey, we have a used drive  */
        else if ((driveMap & (DWORD)1))
        {
            // format in advance in case we need to add this
            snprintf(driveStr, sizeof(driveStr), "%c: ", dnum + 'A' - 1);
            // if we only want to know the used ones, we have all the
            // information we need
            if (selectMode == USED)
            {
                driveMapBuffer += driveStr;
            }
            // need to check specific drive information
            else
            {
                char deviceName[8];
                snprintf(deviceName, sizeof(deviceName), "%c:\\", dnum + 'A' - 1);

                // if this matches what we're looking for, add it to the list
                if (driveType == GetDriveType(deviceName))
                {
                    driveMapBuffer += driveStr;
                }
            }
        }
        // shift to the next drive
        driveMap >>= 1;
    }

    size_t driveLength = driveMapBuffer.length();
    // if we are returning anything, then remove the last blank
    if (driveLength > 0)
    {
        driveLength--;
    }

    return context->NewString(driveMapBuffer, driveLength);
}


// below are Windows-specific implementations of TreeFinder methods.

/**
 *
 * if this ends in a directory separator, add a * wildcard to the end
 */
void TreeFinder::adjustDirectory()
{
    // if this ends in a directory separator, add a * wildcard to the end
    if (fileSpec.endsWith('\\') || fileSpec.endsWith('/'))
    {
        fileSpec += "*";
    }
    // just a . or .. is wildcarded also
    else if (fileSpec == "." || fileSpec == "..")
    {
        fileSpec += "\\*";
    }
    // if the end section is either \. or \.., we also add the wildcard
    else if (fileSpec.endsWith("\\.") || fileSpec.endsWith("\\..")  || fileSpec.endsWith("/.") || fileSpec.endsWith("/.."))
    {
        fileSpec += "\\*";
    }
}



/**
 * Perform any platform-specific adjustments to the platform specification.
 */
void TreeFinder::adjustFileSpec()
{
    size_t i = 0;

    // Skip leading blanks.
    while (fileSpec.at(i) == ' ')
    {
        i++;
    }

    // did we have leading spaces? There's some adjustment required.
    // we only remove the spaces if this starts with a drive or directory
    // specification, otherwise the blanks are considered part of the name.
    if (i > 0)
    {
        size_t len = fileSpec.length();

        // now check the special cases. First, we start with
        // a directory marker
        if (fileSpec.at(i) == '\\' || fileSpec.at(i) == '/')                         // The "\" case
        {
            // perform a left shift on the buffer
            fileSpec.shiftLeft(i);
        }
        // starts with "." (or potentially "..")
        else if (fileSpec.at(i) == '.')
        {
            // if this is not at the end, look for ".\" or "..\"
            if (i + 1 < len)
            {
                if (fileSpec.at(i + 1) == '\\' || fileSpec.at(i + 1) == '/')         // The ".\" case
                {
                    // perform a left shift on the buffer
                    fileSpec.shiftLeft(i);
                }
                else if (i + 2 < len)
                {
                    if (fileSpec.at(i + 1) == '.' &&
                        (fileSpec.at(i + 2) == '\\' || fileSpec.at(i + 2) == '/'))   // The "..\" case
                    {
                        // perform a left shift on the buffer
                        fileSpec.shiftLeft(i);
                    }
                }
            }
        }
        else if (i + 1 < len && fileSpec.at(i + 1) == ':')                     // The "z:' case
        {
            // perform a left shift on the buffer
            fileSpec.shiftLeft(i);
        }
    }
}


/**
 * Tests for illegal file name characters in fileSpec
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
void TreeFinder::validateFileSpecName()
{
    const char illegal[] = "<>|\"";

    for (size_t i = 0; i < strlen(illegal); i++)
    {
        if (strchr((const char *)fileSpec, illegal[i]) != NULL)
        {
            throw InvalidFileName;
        }
    }

    const char *pos = strchr((const char *)fileSpec, ':');
    if (pos != NULL)
    {
        // c:\temp, \\?\c:\temp, \\.\c:\temp are all valid
        int32_t p = (int32_t)(pos - fileSpec + 1);
        if (p != 2 && p != 6)
        {
            throw InvalidFileName;
        }

        if (strchr(pos + 1, ':') != NULL)
        {
            throw InvalidFileName;
        }
    }
}


/**
 * Returns a new file attribute value given a mask of attributes to be changed
 * and the current attribute value.
 *
 * @return New attribute value.
 */
uint32_t mergeAttrs(TreeFinder::AttributeMask &mask, uint32_t attr)
{
    // if no settings to process, return the old set
    if (mask.noNewSettings())
    {
        return attr;
    }

    if (mask.isOff(TreeFinder::AttributeMask::Archive))
    {
        attr &= ~FILE_ATTRIBUTE_ARCHIVE;   // Clear
    }
    else if (mask.isOn(TreeFinder::AttributeMask::Archive))
    {
        attr |= FILE_ATTRIBUTE_ARCHIVE;    // Set
    }

    // we can't really turn the directory bit on or off, so leave
    // this unchanged.
    if (mask.isOff(TreeFinder::AttributeMask::Hidden))
    {
        attr &= ~FILE_ATTRIBUTE_HIDDEN; // Clear
    }
    else if (mask.isOn(TreeFinder::AttributeMask::Hidden))
    {
        attr |= FILE_ATTRIBUTE_HIDDEN;  // Set
    }
    if (mask.isOff(TreeFinder::AttributeMask::ReadOnly))
    {
        attr &= ~FILE_ATTRIBUTE_READONLY; // Clear
    }
    else if (mask.isOn(TreeFinder::AttributeMask::ReadOnly))
    {
        attr |= FILE_ATTRIBUTE_READONLY;  // Set
    }
    if (mask.isOff(TreeFinder::AttributeMask::System))
    {
        attr &= ~FILE_ATTRIBUTE_SYSTEM; // Clear
    }
    else if (mask.isOn(TreeFinder::AttributeMask::System))
    {
        attr |= FILE_ATTRIBUTE_SYSTEM;  // Set
    }

    return  attr;
}


/**
 * Set the file attributes using the new mask and then return the
 * new current attribute set for the file.
 *
 * @param fileName The target file name
 *
 * @return A new attribute mask for the file.
 */
void setFileAttributes(const char *fileName, TreeFinder::AttributeMask &newAttributes)
{
    // The file attributes need to be changed before we format the found file
    // line.

    uint32_t oldAttrs = GetFileAttributes(fileName);

    // create a merged set of attributes
    uint32_t changedAttrs = mergeAttrs(newAttributes, oldAttrs);

    // if they are not the same, then we need to try to set them
    if (changedAttrs != oldAttrs)
    {
        // try to set the attributes, but if it fails, just use the exsiting.
        // the directory flag is not settable, so make sure this flag is off
        SetFileAttributes(fileName, changedAttrs & ~FILE_ATTRIBUTE_DIRECTORY);
    }
}


/**
 * Format the system-specific file time, attribute mask, and size for
 * the given file.
 *
 * @param foundFile The buffer the result is formatted into
 * @param finfo     The system information about the file.
 */
void formatFileAttributes(TreeFinder *finder, FileNameBuffer &foundFile, SysFileIterator::FileAttributes &attributes)
{
    char fileAttr[256];                 // File attribute string of found file

    FILETIME lastWriteTime;
    FileTimeToLocalFileTime(&attributes.findFileData.ftLastWriteTime, &lastWriteTime);

    // Convert UTC to local file time, and then to system format.
    SYSTEMTIME systime;
    FileTimeToSystemTime(&lastWriteTime, &systime);

    // The fileTime buffer is 64 bytes.
    // Since we can count the characters put into the buffer here, there is
    // no need to check for buffer overflow.

    // wYear range is 1601 through 30827
    if (finder->longTime())
    {
        snprintf(fileAttr, sizeof(fileAttr), "%4d-%02d-%02d %02d:%02d:%02d  ",
                 systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute,
                 systime.wSecond);
    }
    else if (finder->editableTime())
    {
        snprintf(fileAttr, sizeof(fileAttr), "%02d/%02d/%02d/%02d/%02d  ",
                 systime.wYear % 100, systime.wMonth, systime.wDay,
                 systime.wHour, systime.wMinute);

    }
    else
    {
        snprintf(fileAttr, sizeof(fileAttr), "%2d/%02d/%02d  %2d:%02d%c  ",
                 systime.wMonth, systime.wDay, systime.wYear % 100,
                 (systime.wHour < 13 && systime.wHour != 0 ?
                  systime.wHour : (abs(systime.wHour - (SHORT)12))),
                 systime.wMinute, (systime.wHour < 12 || systime.wHour == 24) ? 'a' : 'p');
    }

    // this is the first part of the return value. Copy to the result buffer so we can
    // reuse the buffer
    foundFile = fileAttr;

    uint64_t longFileSize = attributes.findFileData.nFileSizeHigh;
    longFileSize <<= 32;
    longFileSize |= attributes.findFileData.nFileSizeLow;

    // now the size information
    if (finder->longSize())
    {
        snprintf(fileAttr, sizeof(fileAttr), "%20llu  ", longFileSize);
    }
    else
    {
        if (longFileSize > 9999999999)
        {
            longFileSize = 9999999999;
        }
        snprintf(fileAttr, sizeof(fileAttr), "%10llu  ", longFileSize);
    }

    // the order is time, size, attributes
    foundFile += fileAttr;

    snprintf(fileAttr, sizeof(fileAttr), "%c%c%c%c%c  ",
             (attributes.findFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ? 'A' : '-',
             (attributes.findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 'D' : '-',
             (attributes.findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? 'H' : '-',
             (attributes.findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? 'R' : '-',
             (attributes.findFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ? 'S' : '-');

    // add on this section
    foundFile += fileAttr;
}


/**
 * SysFileTree helper routine to see if this is a file we need to include
 * in the result set.
 *
 * @param attr   The file attributes
 *
 * @return true if the file should be included, false otherwise.
 */
bool checkInclusion(TreeFinder *finder, uint32_t attr)
{
    // if this is a directory and we're not looking for dirs, this is a pass
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) && !finder->includeDirs())
    {
        return false;
    }

    // file is not a DIR and we're only looking for directories
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY) && !finder->includeFiles())
    {
        return false;
    }

    // we've passed the file type checks, now see if we have attribute checks to make

    // if no mask is set, then everything is good
    if (finder->acceptAll())
    {
        return true;
    }

    if (!finder->archiveSelected((attr & FILE_ATTRIBUTE_ARCHIVE) != 0))
    {
        return  false;
    }
    // a little silly since this overlaps with the options
    if (!finder->directorySelected((attr & FILE_ATTRIBUTE_DIRECTORY) != 0))
    {
        return  false;
    }
    if (!finder->hiddenSelected((attr & FILE_ATTRIBUTE_HIDDEN) != 0))
    {
        return  false;
    }
    if (!finder->readOnlySelected((attr & FILE_ATTRIBUTE_READONLY) != 0))
    {
        return  false;
    }
    if (!finder->systemSelected((attr & FILE_ATTRIBUTE_SYSTEM) != 0))
    {
        return  false;
    }

    return  true;
}


/**
 * Checks if this file should be part of the included result and adds it to the result set
 * if it should be returned.
 */
void TreeFinder::checkFile(SysFileIterator::FileAttributes &attributes)
{
    // check to see if this one should be included. If not, we just return without doing anythign
    if (!checkInclusion(this, attributes.findFileData.dwFileAttributes))
    {
        return;
    }

    // handle setting of the new file attributes, if supported
    setFileAttributes(foundFile, newAttributes);

    // if only the name is requested, create a string object and set the
    // stem varaiable
    if (nameOnly())
    {
        addResult(foundFile);
        return;
    }

    // format all of the attributes and add them to the foundFile result
    formatFileAttributes(this, foundFileLine, attributes);

    // and finally add on the file name
    foundFileLine += foundFile;

    // add this to the stem
    addResult(foundFileLine);
}


/**
 * Platform-specific TreeFinder method for locating the end of the
 * fileSpec directory
 *
 * @return The position of the end of the directory. -1 indicates the file spec
 *         does not contain a directory.
 */
int TreeFinder::findDirectoryEnd()
{
    // Get the maximum position of the last '\'
    int lastSlashPos = (int)fileSpec.length();

    // Step back through fileSpec until at its beginning or at a '\' or '/' character
    while (fileSpec.at(lastSlashPos) != '\\' && fileSpec.at(lastSlashPos) != '/' && lastSlashPos >= 0)
    {
        --lastSlashPos;
    }

    return lastSlashPos;
}


/**
 * Perform platform-specific drive checks on the file spec. This only
 * applies to Windows.
 *
 * @return true if this was processed, false if additional work is required.
 */
bool TreeFinder::checkNonPathDrive()
{
// fileSpec could be a drive designator.
    if (fileSpec.at(1) == ':')
    {
        RoutineFileNameBuffer currentDirectory(context);

        SysFileSystem::getCurrentDirectory(currentDirectory);

        char drive[4] = { 0 };

        // Just copy the drive letter and the colon, omit the rest.  This is
        // necessary e.g. for something like "I:*"
        memcpy(drive, fileSpec, 2);

        // Change to the specified drive, get the current directory, then go
        // back to where we came from.
        SysFileSystem::setCurrentDirectory(drive);
        SysFileSystem::getCurrentDirectory(filePath);
        SysFileSystem::setCurrentDirectory(currentDirectory);

        // everything after the drive delimiter is the search name.
        nameSpec = ((const char *)fileSpec) + 2;
        return true;
    }
    // not a drive, this will need to be prepended by the current directory
    return false;
}


/**
 * Perform any platform-specific fixup that might be required with
 * the supplied path.
 */
void TreeFinder::fixupFilePath()
{
    // resolve any relative postions using _fullpath(), which really saves us
    // a lot of work;

    const char *p = _fullpath(NULL, filePath, 0);
    // this replaces what we currently have
    // but if we get a zero back, this is somehow invalid, keep the
    // existing path and try to use that
    if (strlen(p) > 0)
    {
        filePath = p;
    }
    free((void *)p);

    // make sure this is terminated with a path delimiter
    filePath.addFinalPathDelimiter();
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

RexxRoutine1(RexxStringObject, SysGetKey, OPTIONAL_CSTRING, echoOpt)
{
    bool echo = true;      // indicates if we're expected to echo the character.

    // if we have an option specified, validate it
    if (echoOpt != NULL)
    {
        if (!_stricmp(echoOpt, "NOECHO"))
        {
            echo = false;
        }
        else if (_stricmp(echoOpt, "ECHO"))
        {
            invalidOptionException(context, "SysGetKey", "echo", "'ECHO' or 'NOECHO'", echoOpt);
        }
    }

    int       tmp;                       /* Temp var used to hold      */
    // if we have the second part of an extended value, return that without
    // doing a real read.
    if (ExtendedFlag)
    {
        tmp = ExtendedChar;                /* get the second char        */
        ExtendedFlag = false;
    }
    else
    {
        tmp = _getch();                    /* read a character           */

        // either of these characters indicate this is an extended character.
        if ((tmp == 0x00) || (tmp == 0xe0))
        {
            // this character is saved for the next call. We return the marker
            // character first.
            ExtendedChar = _getch();         /* Read another character     */
            ExtendedFlag = true;
        }
        else
        {
            ExtendedFlag = false;
        }
    }
    if (echo)                            /* echoing?                   */
    {
        _putch(tmp);                       /* write the character back   */
    }

    char character = (char)tmp;
    return context->NewString(&character, 1);
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

RexxRoutine4(RexxStringObject, SysIni, OPTIONAL_CSTRING, iniFile, CSTRING, app, RexxObjectPtr, key, OPTIONAL_RexxObjectPtr, val)
{
    // the ini file is optional and defaults to WIN.INI
    if (iniFile == NULL)
    {
        iniFile = "WIN.INI";
    }

    // Process first off of the app key. This could be a keyword command, which changes
    // the meaning of the following arguments.

    // all is a special app name, this wants everything
    if (_stricmp(app, "ALL:") == 0)
    {
        StemHandler stemVariable(context);            // used to manipulate the stem variable for return values.

        // the third arg is required and is the stem variable name.
        // the 4th argument cannot be specified
        if (argumentExists(4))
        {
            maxArgException(context, "SysIni ALL:", 3);
        }

        // Retrieve the stem variable using the key name
        stemVariable.setStem(key, 3);

        size_t lSize = 0x0000ffffL;
        // Allocate a large buffer for retrieving the information
        AutoFree returnVal = (char *)malloc(lSize);
        if (returnVal == NULL)
        {
            return context->NewStringFromAsciiz(ERROR_NOMEM);
        }
        // now retrieve the application names.
        lSize = GetPrivateProfileString(NULL, NULL, "", returnVal, (DWORD)lSize, iniFile);
        // zero indicates there was an error
        if (lSize == 0)
        {
            return context->NewStringFromAsciiz(ERROR_RETSTR);
        }

        // parse this up into a list added to the stem variable
        stemVariable.addList(returnVal);
        return context->NullString();
    }

    // ok, this is targetted at a particular app. Now decode what sort of operation this is.
    // the key needs
    const char *keyName = context->ObjectToStringValue(key);

    // this could be a request for all keys of a given app
    // the val argument must be a stem variable name.
    if (!_stricmp(keyName, "ALL:"))
    {
        StemHandler stemVariable(context);            // used to manipulate the stem variable for return values.

        // val is the stem variable for this case
        if (argumentOmitted(4))
        {
            // Missing argument; argument 4 is required
            context->ThrowException1(Rexx_Error_Invalid_argument_noarg, context->WholeNumberToObject(4));
        }
        stemVariable.setStem(val, 4);

        size_t lSize = 0x0000ffffL;
        // Allocate a large buffer for retrieving the information
        AutoFree returnVal = (char *)malloc(lSize);
        if (returnVal == NULL)
        {
            return context->NewStringFromAsciiz(ERROR_NOMEM);
        }

        // Retrieve all keys for a specific application
        lSize = GetPrivateProfileString(app, NULL, "", returnVal, (DWORD)lSize, iniFile);

        // zero indicates there was an error
        if (lSize == 0)
        {
            return context->NewStringFromAsciiz(ERROR_RETSTR);
        }

        // parse this up into a list added to the stem variable
        stemVariable.addList(returnVal);
        return context->NullString();
    }

    // this could be a DELETE: operation for a particular application
    if (stricmp(keyName, "DELETE:") == 0)
    {
        // the 4th argument cannot be specified
        if (argumentExists(4))
        {
            maxArgException(context, "SysIni DELETE:", 3);
        }

        // A request to delete all keys for a given application
        if (!WritePrivateProfileString(app, NULL, NULL, iniFile))
        {
            return context->NewStringFromAsciiz(ERROR_RETSTR);
        }
        else
        {
            return context->NullString();
        }
    }

    // OK, if we got this far, then we're working with a specific key for a specific app.
    // This is either a retrieval operation, a delete request, or a set request.

    // a NULL value argument is a retrieval request. This is the return value.
    if (val == NULL)
    {
        size_t lSize = 0x0000ffffL;
        // Allocate a large buffer for retrieving the information
        AutoFree returnVal = (char *)malloc(lSize);
        if (returnVal == NULL)
        {
            return context->NewStringFromAsciiz(ERROR_NOMEM);
        }

        // Retrieve just the given key value
        lSize = GetPrivateProfileString(app, keyName, "", returnVal, (DWORD)lSize, iniFile);

        // zero indicates there was an error
        if (lSize == 0)
        {
            return context->NewStringFromAsciiz(ERROR_RETSTR);
        }

        return context->NewString(returnVal, lSize);
    }

    // this argument must be a string from this point
    const char *valueString = context->ObjectToStringValue(val);

    // this could be a key deletion request
    if (stricmp(valueString, "DELETE:") == 0)
    {
        // A request to delete a key for a given application
        if (!WritePrivateProfileString(app, keyName, NULL, iniFile))
        {
            return context->NewStringFromAsciiz(ERROR_RETSTR);
        }
        else
        {
            return context->NullString();
        }
    }

    // we have a value specified and it is not the special key marker, this is a set operation
    if (!WritePrivateProfileString(app, keyName, valueString, iniFile))
    {
        return context->NewStringFromAsciiz(ERROR_RETSTR);
    }
    else
    {
        return context->NullString();
    }
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
    RoutineQualifiedName directory(context, dir);

    // this could easily made a common function but the Linux version
    // takes an extra argument.
    return CreateDirectory(directory, NULL) != 0 ? 0 : GetLastError();
}


/*************************************************************************
* Function:  SysGetErrorText                                             *
*                                                                        *
* Syntax:    call SysGetErrortext errnumber                              *
*                                                                        *
* Params:    errnumber - error number to be described                    *
*                                                                        *
* Return:    Description or empty string                                 *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysGetErrorText, int32_t, errnum)
{
    char *errmsg;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errnum, 0, (LPSTR)&errmsg, 64, NULL) == 0)
    {
        return context->NullString();
    }
    else
    {                               /* succeeded                  */
        size_t length = strlen(errmsg);

        // FormatMessage returns strings with trailing CrLf, which we want removed
        if (length >= 1 && errmsg[length - 1] == 0x0a)
        {
            length--;
        }
        if (length >= 1 && errmsg[length - 1] == 0x0d)
        {
            length--;
        }

        RexxStringObject ret = context->NewString(errmsg, length);
        LocalFree(errmsg);

        return ret;
    }
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

RexxRoutine1(uint32_t, SysWinEncryptFile, CSTRING, fileName)
{
    RoutineQualifiedName qualifiedName(context, fileName);

    ULONG rc = EncryptFile(qualifiedName);

    return rc != 0 ? rc : GetLastError();
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

RexxRoutine1(uint32_t, SysWinDecryptFile, CSTRING, fileName)
{
    RoutineQualifiedName qualifiedName(context, fileName);

    ULONG rc = DecryptFile(qualifiedName, 0);

    return rc != 0 ? rc : GetLastError();
}


/*************************************************************************
* Function:  SysWinVer                                                   *
*                                                                        *
* Syntax:    call SysWinVer                                              *
*                                                                        *
* Return:    Windows Version                                             *
*************************************************************************/

RexxRoutine0(RexxStringObject, SysWinVer)
{
    // MS has deprecated GetVersionEx().  Also, since 10.0.18363, querying
    // the version information from one of the system DLLs won't give the
    // correct version number any longer.  The only remaining option seems
    // to be this:
    // https://stackoverflow.com/questions/36543301/detecting-windows-10-version

    #define STATUS_SUCCESS (0x00000000)
    typedef LONG NTSTATUS;
    typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod != NULL)
    {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != NULL)
        {
            RTL_OSVERSIONINFOW rovi;
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fxPtr(&rovi) == STATUS_SUCCESS)
            {
                char retstr[256];

                snprintf(retstr, sizeof(retstr), "Windows %d.%d.%d",
                 rovi.dwMajorVersion, rovi.dwMinorVersion, rovi.dwBuildNumber);
                return context->String(retstr);
            }
        }
    }
    // just return the nullstring if we fail
    return context->NullString();
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
*                                                                        *
* Note: The ReadConsoleOutputCharacter API is no longer recommended      *
*************************************************************************/
RexxRoutine3(RexxStringObject, SysTextScreenRead, int, row, int, col, OPTIONAL_int, len)
{
    HANDLE h;                            // stdout handle
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; // screen buffer size
    AutoFree buffer;                     // return buffer
    COORD start;                         // start coordinates
    DWORD chars;                         // actual chars read

    if (row < 0 || col < 0 || argumentExists(3) && len < 0 ||
        (h = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
    {
        context->InvalidRoutine();
        return NULLOBJECT;
    }

    // get screen buffer size if len argument is not given
    if (argumentOmitted(3))
    {
        if (GetConsoleScreenBufferInfo(h, &csbiInfo) == 0)
        {
            context->InvalidRoutine();
            return NULLOBJECT;
        }
        len = csbiInfo.dwSize.Y * csbiInfo.dwSize.X;
    }

    if (len >= 0 && (buffer = (char *)malloc(len)) == NULL)
    {
        outOfMemoryException(context);
    }

    start.X = (short)col;
    start.Y = (short)row;
    if (ReadConsoleOutputCharacter(h, buffer, len, start, &chars) == 0)
    {
        context->InvalidRoutine();
        return NULLOBJECT;
    }

    return context->NewString(buffer, chars);
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
    typedef enum {BUFFERSIZE, WINDOWRECT, MAXWINDOWSIZE } console_option;
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
        invalidOptionException(context, "SysTextScreenSize", "option", "BUFFERSIZE, WINDOWRECT, or MAXWINDOWSIZE", optionString);
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
        snprintf(buffer, sizeof(buffer), "%d %d", csbi.dwSize.Y, csbi.dwSize.X);
    }
    else if (option == WINDOWRECT && setArgs == 0)
    {
        // this is a WINDOWRECT GET, returns four values
        snprintf(buffer, sizeof(buffer), "%d %d %d %d", csbi.srWindow.Top, csbi.srWindow.Left, csbi.srWindow.Bottom, csbi.srWindow.Right);
    }
    else if (option == MAXWINDOWSIZE && setArgs == 0)
    {
        // this is a MAXWINDOWSIZE GET, returns two values
        snprintf(buffer, sizeof(buffer), "%d %d", csbi.dwMaximumWindowSize.Y, csbi.dwMaximumWindowSize.X);
    }
    else if (option == BUFFERSIZE && setArgs == 2)
    {
        // this is a BUFFERSIZE SET, requires two more arguments
        COORD consoleBuffer;
        consoleBuffer.Y = (SHORT)rows;
        consoleBuffer.X = (SHORT)columns;
        BOOL code = SetConsoleScreenBufferSize(hStdout, consoleBuffer);
        snprintf(buffer, sizeof(buffer), "%d", code == 0 ? GetLastError() : 0);
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
        snprintf(buffer, sizeof(buffer), "%d", code == 0 ? GetLastError() : 0);
    }
    else
    {
        context->InvalidRoutine();
        return 0;
    }

    // return the buffer as result
    return context->NewStringFromAsciiz(buffer);
}

#define MAX_CREATEPROCESS_CMDLINE (32 * 1024)

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

RexxRoutine2(uint32_t, RxWinExec, CSTRING, command, OPTIONAL_CSTRING, show)
{
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

    if (strlen(command) > MAX_CREATEPROCESS_CMDLINE)
    {
        context->InvalidRoutine();
        return 0;
    }

    int cmdShow = SW_SHOWNORMAL;    // The style is optional

    // Show type can be one and only one of the SW_XXX constants.
    if (show != NULL)
    {
        // Get length of option and search the style table.

        int         index;                   /* table index                */
        for (index = 0; index < sizeof(show_styles) / sizeof(PSZ); index++)
        {
            if (_stricmp(show, show_styles[index]) == 0)
            {
                cmdShow = show_flags[index];
                break;
            }
        }

        if (index == sizeof(show_styles) / sizeof(PSZ))
        {
            context->InvalidRoutine();
            return 0;
        }
    }

    ULONG       pid;                     /* PID or error return code   */
    STARTUPINFO si;
    PROCESS_INFORMATION procInfo;

    ZeroMemory(&procInfo, sizeof(procInfo));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = (WORD)cmdShow;

    if (CreateProcess(NULL, (LPSTR)command, NULL, NULL, FALSE, 0, NULL,
                      NULL, &si, &procInfo))
    {
        pid = procInfo.dwProcessId;
    }
    else
    {
        pid = GetLastError();
        if (pid > 31)
        {
            // Maintain compatibility to versions < ooRexx 3.1.2
            pid = (ULONG)-((int)pid);
        }
    }

    // Close process / thread handles as they are not used / needed.
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return pid;
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
RexxRoutine0(RexxStringObject, SysBootDrive)
{
    char retstr[MAX_PATH];

    if (GetSystemDirectory(retstr, sizeof(retstr)) > 0)
    {
        // the drive is the first two characters of the system directory
        return context->NewString(retstr, 2);
    }
    else
    {
        return context->NullString();
    }
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

RexxRoutine0(RexxStringObject, SysSystemDirectory)
{
    char retstr[MAX_PATH];

    if (GetSystemDirectory(retstr, sizeof(retstr)) > 0)
    {
        return context->NewStringFromAsciiz(retstr);
    }
    else
    {
        return context->NullString();
    }
}


/*************************************************************************
* Function:  SysFileSystemType                                           *
*                                                                        *
* Syntax:    result = SysFileSystemType([drive])                         *
*                                                                        *
* Params:    drive - d, d:, d:\, \\share\path                            *
*                    if omitted, defaults to current drive               *
*                                                                        *
* Return:    result - the drive's file system (e. g. NTFS, FAT)          *
*                     null string for any error                          *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysFileSystemType, OPTIONAL_CSTRING, drive)
{
    FileNameBuffer d;

    if (drive != NULL)
    {
        d = drive;
        // We just let the Windows APIs handle all the different ways to
        // specify a volume, like d:\, \\?\d:, \\localhost\path, etc.  But
        // on top of that we support a single-character drive d.
        if (d.length() == 1 && Utilities::isAlpha(d.at(0)))
        {
            // make this a valid drive specification
            d += ":\\";
        }

        // GetVolumeInformation() is picky, it requires a final backslash
        if (d.length() > 0)
        {
            d.addFinalPathDelimiter();
        }
    }

    // get the file system type
    char fileSystem[MAX_PATH];
    if (GetVolumeInformation((drive == NULL) ? NULL : (char * )d, NULL, 0, NULL, NULL, NULL, fileSystem, sizeof(fileSystem)))
    {
        return context->NewStringFromAsciiz(fileSystem);
    }
    return context->NullString();
}


/*************************************************************************
* Function:  SysVolumeLabel                                              *
*                                                                        *
* Syntax:    result = SysVolumeLabel([drive])                            *
*                                                                        *
* Params:    drive - d, d:, d:\, \\share\path                            *
*                    if omitted, defaults to current drive               *
*                                                                        *
* Return:    result - the volume label                                   *
*                     null string for any error                          *
*************************************************************************/

RexxRoutine1(RexxStringObject, SysVolumeLabel, OPTIONAL_CSTRING, drive)
{
    FileNameBuffer d;

    if (drive != NULL)
    {
        d = drive;
        // We just let the Windows APIs handle all the different ways to
        // specify a volume, like d:\, \\?\d:, \\localhost\path, etc.  But
        // on top of that we support a single-character drive d.
        if (d.length() == 1 && Utilities::isAlpha(d.at(0)))
        {
            // make this a valid drive specification
            d += ":\\";
        }

        // GetVolumeInformation() is picky, it requires a final backslash
        if (d.length() > 0)
        {
            d.addFinalPathDelimiter();
        }
    }

    // get the volume label
    char volumeName[MAX_PATH];
    if (GetVolumeInformation((drive == NULL) ? NULL : (char * )d, volumeName, sizeof(volumeName), NULL, NULL, NULL, NULL, 0))
    {
        return context->NewStringFromAsciiz(volumeName);
    }
    return context->NullString();
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
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };

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

    if (handle == NULL)
    {
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
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };

    /* get a binary handle        */
    return (uintptr_t)OpenMutex(MUTEX_ALL_ACCESS, true, name); /* try to open it             */
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
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };
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

    if (handle == NULL)
    {
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
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };

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

    DWORD     iclass = -1;
    wholenumber_t classLevel;               /* priority class             */
    if (context->WholeNumber(classArg, &classLevel))
    {
        switch (classLevel)
        {
            case 0:
                iclass = IDLE_PRIORITY_CLASS;
                break;
            case 1:
                iclass = NORMAL_PRIORITY_CLASS;
                break;
            case 2:
                iclass = HIGH_PRIORITY_CLASS;
                break;
            case 3:
                iclass = REALTIME_PRIORITY_CLASS;
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
            ok = GetThreadTimes(GetCurrentThread(), &createT, &dummy, &kernelT, &userT);
        }
        else
        {
            ok = GetProcessTimes(GetCurrentProcess(), &createT, &dummy, &kernelT, &userT);
        }

        if (ok)
        {
            FileTimeToLocalFileTime(&createT, &createT);
            FileTimeToSystemTime(&createT, &createST);
            FileTimeToSystemTime(&kernelT, &kernelST);
            FileTimeToSystemTime(&userT, &userST);

            char buffer[256];

            snprintf(buffer, sizeof(buffer), "Create: %4.4d/%2.2d/%2.2d %d:%2.2d:%2.2d:%3.3d  "\
                         "Kernel: %d:%2.2d:%2.2d:%3.3d  User: %d:%2.2d:%2.2d:%3.3d",
                     createST.wYear, createST.wMonth, createST.wDay, createST.wHour, createST.wMinute,
                     createST.wSecond, createST.wMilliseconds,
                     kernelST.wHour, kernelST.wMinute, kernelST.wSecond, kernelST.wMilliseconds,
                     userST.wHour, userST.wMinute, userST.wSecond, userST.wMilliseconds);

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
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == 0)
    {
        result = GetLastError();
        goto done_out;
    }

    // Get the locally unique identifier for the shutdown privilege we need,
    // local or remote, depending on what the user specified.
    LPCTSTR privilegeName = (computer == NULL || *computer == '\0') ? SE_SHUTDOWN_NAME : SE_REMOTE_SHUTDOWN_NAME;
    if (LookupPrivilegeValue(NULL, privilegeName, &tkp.Privileges[0].Luid) == 0)
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
    if (result != ERROR_SUCCESS)
    {
        goto done_out;
    }

    // Do not shut down in 0 seconds by default.
    if (argumentOmitted(3))
    {
        timeout = 30;
    }

    // Now just call the API with the parameters specified by the user.
    if (InitiateSystemShutdown((LPSTR)computer, (LPSTR)message, timeout, (BOOL)forceAppsClosed, (BOOL)reboot) == 0)
    {
        result = GetLastError();
    }

    // Finally, restore the shutdown privilege for this process to disabled.
    tkp.Privileges[0].Attributes = 0;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

done_out:
    if (hToken != NULL)
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
                // We never receive fractions of seconds.  Set them to zero.
                sLocalSysTime.wMilliseconds = 0;
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

    return fOk ? 0 : -1;
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
    SysFileSystem::FiletimeType type;
    FILETIME  sFileTime;
    FILETIME  sLocalFileTime;
    SYSTEMTIME sLocalSysTime;

    if (selector != NULL)
    {
        switch (selector[0])
        {
            case 'c':
            case 'C':
                type = SysFileSystem::FiletimeCreation;
                break;
            case 'a':
            case 'A':
                type = SysFileSystem::FiletimeAccess;
                break;
            case 'w':
            case 'W':
                type = SysFileSystem::FiletimeWrite;
                break;
            default:
                invalidOptionException(context, "SysGetFileDateTime", "time selector", "'A', 'C', or 'W'", selector);
        }
    }
    else
    {
        type = SysFileSystem::FiletimeWrite;
    }

    if (SysFileSystem::getFiletime(name, type, &sFileTime) &&
        FileTimeToLocalFileTime(&sFileTime, &sLocalFileTime) &&
        FileTimeToSystemTime(&sLocalFileTime, &sLocalSysTime))
    {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d %02d:%02d:%02d",
                sLocalSysTime.wYear,
                sLocalSysTime.wMonth,
                sLocalSysTime.wDay,
                sLocalSysTime.wHour,
                sLocalSysTime.wMinute,
                sLocalSysTime.wSecond);
        return context->String(buffer);
    }
    return context->WholeNumber(-1);
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
    if (cp == CP_SYMBOL || cp == CP_UTF7 || cp == CP_UTF8)
    {
        return false;
    }
    if (50220 <= cp && cp <= 50222)
    {
        return false;
    }
    if (cp == 50225 || cp == 50227 || cp == 50229 || cp == 52936 || cp == 54936)
    {
        return false;
    }
    if (57002 <= cp && cp <= 57011)
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
        else if (_stricmp(codePageOpt, "ACP") == 0)
        {
            codePage = CP_ACP;
        }
        else if (_stricmp(codePageOpt, "MACCP") == 0)
        {
            codePage = CP_MACCP;
        }
        else if (_stricmp(codePageOpt, "OEMCP") == 0)
        {
            codePage = CP_OEMCP;
        }
        else if (_stricmp(codePageOpt, "SYMBOL") == 0)
        {
            codePage = CP_SYMBOL;
        }
        else if (_stricmp(codePageOpt, "UTF7") == 0)
        {
            codePage = CP_UTF7;
        }
        else if (_stricmp(codePageOpt, "UTF8") == 0)
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
    if (mappingFlags != NULL && *mappingFlags != '\0')
    {
        /* The WC_SEPCHARS, WC_DISCARDNS, and WC_DEFAULTCHAR flags must also
         * specify the WC_COMPOSITECHECK flag.  So, we add that for the user if
         * they skipped it. Those 4 flags are only available for code pages <
         * 50000, excluding 42 (CP_SYMBOL).  See the remarks section in the MSDN
         * docs for clarification.
         */
        if (codePage < 50000 && codePage != CP_SYMBOL)
        {
            if (StrStrI(mappingFlags, "COMPOSITECHECK") != NULL)
            {
                dwFlags |= WC_COMPOSITECHECK;
            }
            if (StrStrI(mappingFlags, "SEPCHARS") != NULL)
            {
                dwFlags |= WC_SEPCHARS | WC_COMPOSITECHECK;
            }
            if (StrStrI(mappingFlags, "DISCARDNS") != NULL)
            {
                dwFlags |= WC_DISCARDNS | WC_COMPOSITECHECK;
            }
            if (StrStrI(mappingFlags, "DEFAULTCHAR") != NULL)
            {
                dwFlags |= WC_DEFAULTCHAR | WC_COMPOSITECHECK;
            }
        }

        if (StrStrI(mappingFlags, "NO_BEST_FIT") != NULL)
        {
            dwFlags |= WC_NO_BEST_FIT_CHARS;
        }

        if (StrStrI(mappingFlags, "ERR_INVALID") != NULL)
        {
            if (codePage == CP_UTF8)
            {
                dwFlags |= WC_ERR_INVALID_CHARS;
            }
        }
        else if (dwFlags == 0 && !(codePage < 50000 && codePage != CP_SYMBOL))
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
    if (codePage == CP_UTF8 && dwFlags == WC_ERR_INVALID_CHARS)
    {
        strDefaultChar = NULL;
        pUsedDefaultChar = NULL;
    }
    else if (!canUseWideCharFlags(codePage))
    {
        dwFlags = 0;
        strDefaultChar = NULL;
        pUsedDefaultChar = NULL;
    }

    /* Allocate space for the string, to allow double zero byte termination */
    AutoFree strptr = (char *)malloc(sourceLength + 4);
    if (strptr == NULL)
    {
        outOfMemoryException(context);
    }

    memcpy(strptr, source, sourceLength);

    /* Query the number of bytes required to store the Dest string */
    int iBytesNeeded = WideCharToMultiByte(codePage,
                                           dwFlags,
                                           (LPWSTR)(char *)strptr,
                                           (int)(sourceLength / 2),
                                           NULL,
                                           0,
                                           NULL,
                                           NULL);

    if (iBytesNeeded == 0)
    {
        return GetLastError();
    }

    // hard error, stop
    AutoFree str = (char *)malloc(iBytesNeeded + 4);
    if (str == NULL)
    {
        outOfMemoryException(context);
    }

    /* Do the conversion */
    int iBytesDestination = WideCharToMultiByte(codePage,           // codepage
                                                dwFlags,                // conversion flags
                                                (LPWSTR)(char *)strptr, // source string
                                                (int)(sourceLength / 2),  // source string length
                                                str,                    // target string
                                                (int)iBytesNeeded,      // size of target buffer
                                                strDefaultChar,
                                                pUsedDefaultChar);

    if (iBytesDestination == 0) // call to function fails
    {
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
        if (_stricmp(codePageOpt, "THREAD_ACP") == 0)
        {
            codePage = CP_THREAD_ACP;
        }
        else if (_stricmp(codePageOpt, "ACP") == 0)
        {
            codePage = CP_ACP;
        }
        else if (_stricmp(codePageOpt, "MACCP") == 0)
        {
            codePage = CP_MACCP;
        }
        else if (_stricmp(codePageOpt, "OEMCP") == 0)
        {
            codePage = CP_OEMCP;
        }
        else if (_stricmp(codePageOpt, "SYMBOL") == 0)
        {
            codePage = CP_SYMBOL;
        }
        else if (_stricmp(codePageOpt, "UTF7") == 0)
        {
            codePage = CP_UTF7;
        }
        else if (_stricmp(codePageOpt, "UTF8") == 0)
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
    ULONG ulWCharsNeeded = MultiByteToWideChar(codePage, dwFlags,
                                               context->StringData(source), (int)context->StringLength(source), NULL, NULL);

    if (ulWCharsNeeded == 0)
    {
        return GetLastError();
    }

    ULONG ulDataLen = (ulWCharsNeeded)*2;

    AutoFree lpwstr = (char *)malloc(ulDataLen + 4);

    // hard error, stop
    if (lpwstr == NULL)
    {
        outOfMemoryException(context);
    }


    /* Do the conversion */
    ulWCharsNeeded = MultiByteToWideChar(codePage,  dwFlags,
                                         context->StringData(source), (int)context->StringLength(source),
                                         (LPWSTR)(char *)lpwstr, ulWCharsNeeded);

    if (ulWCharsNeeded == 0) // call to function fails
    {
        return GetLastError();
    }

    context->SetStemElement(stem, "!TEXT", context->String((const char *)lpwstr, ulDataLen));
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

RexxRoutine1(uint32_t, SysWinGetPrinters, RexxObjectPtr, stem)
{
    DWORD realSize = 0;
    DWORD entries = 0;
    DWORD currentSize = 10 * sizeof(PRINTER_INFO_2) * sizeof(char);
    AutoFree pArray = (char *)malloc(sizeof(char) * currentSize);

    while (true)
    {
        if (pArray == NULL)
        {
            outOfMemoryException(context);
        }

        if (EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, (LPBYTE)(char *)pArray,
                         currentSize, &realSize, &entries) == 0)
        {
            // this is not a failure if we get ERROR_INSUFFICIENT_BUFFER
            DWORD rc = GetLastError();
            if (rc != ERROR_INSUFFICIENT_BUFFER)
            {
                return rc;
            }
        }
        if (currentSize >= realSize)
        {
            break;
        }
        currentSize = realSize;
        realSize = 0;
        // adjust to the new size
        pArray.realloc(currentSize);
    }

    StemHandler stemVariable(context, stem, 1);

    PRINTER_INFO_2 *pResult = (PRINTER_INFO_2 *)(char *)pArray;

    while (entries--)
    {
        char  buffer[256];
        snprintf(buffer, sizeof(buffer), "%s,%s,%s", pResult[entries].pPrinterName, pResult[entries].pDriverName,
                pResult[entries].pPortName);
        stemVariable.addValue(buffer);
    }
    stemVariable.complete();   // set the final value
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
    for (size_t i = 0; printer[i] != '\0'; i++)
    {
        if (printer[i] == ',')
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

    if (count == 0)
    {
        // This is W2K or later and the user specified just the printer name.
        // This code will work on W2K through Vista.
        if (SetDefaultPrinter(printer) != 0)
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
            if (SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0L, 0L, SMTO_NORMAL, 1000, NULL) == 0)
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
    DWORD dwAttrs = SysFileSystem::getFileAttributes(file);
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
    DWORD dwAttrs = SysFileSystem::getFileAttributes(file);
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
    DWORD dwAttrs = SysFileSystem::getFileAttributes(file);
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
    DWORD dwAttrs = SysFileSystem::getFileAttributes(file);
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
    DWORD dwAttrs = SysFileSystem::getFileAttributes(file);
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
    DWORD dwAttrs = SysFileSystem::getFileAttributes(file);
    return (dwAttrs != 0xffffffff) && (dwAttrs & FILE_ATTRIBUTE_TEMPORARY);
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
    RoutineFileNameBuffer longPath(context);
    DWORD requiredSize = GetLongPathName(path, longPath, (DWORD)longPath.capacity());
    if (requiredSize == 0)    // call failed
    {
        return context->NullString();
    }
    // was the buffer too small?
    else if (requiredSize > longPath.capacity())
    {
        // expand and retry (which should succeed this time)
        longPath.ensureCapacity(requiredSize);
        if (GetLongPathName(path, longPath, (DWORD)longPath.capacity()))
        {
            return context->NullString();
        }
    }

    return context->NewStringFromAsciiz(longPath);
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
    RoutineFileNameBuffer shortPath(context);
    DWORD requiredSize = GetShortPathName(path, shortPath, (DWORD)shortPath.capacity());
    if (requiredSize == 0)    // call failed
    {
        return context->NullString();
    }
    // was the buffer too small?
    else if (requiredSize > shortPath.capacity())
    {
        // expand and retry (which should succeed this time)
        shortPath.ensureCapacity(requiredSize);
        if (GetShortPathName(path, shortPath, (DWORD)shortPath.capacity()))
        {
            return context->NullString();
        }
    }

    return context->NewStringFromAsciiz(shortPath);
}

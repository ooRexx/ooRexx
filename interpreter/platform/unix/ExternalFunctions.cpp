/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
/* REXX unix Support                                                          */
/*                                                                            */
/* unix specific external function lookup and unix built-in functions.        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*  aixextf.c - Methods to resolve external function calls.                   */
/*                                                                            */
/*  C methods:                                                                */
/*    sysDirectory- Method for the DIRECTORY BIF                              */
/*    SysExternalFunction- Method for searching/invoking an external function */
/*                                                                            */
/*  Internal routines:                                                        */
/*    ExecExternalSearch - Search for and execute a REXX program from disk.   */
/*    MacroSpaceSearch - Search for and execute a function in REXX macrospace.*/
/*    RegExternalFunction - Search for and execute a registered external      */
/*                          function.                                         */
/******************************************************************************/
#include <stdio.h>                          /* Get printf, FILE type, etc.    */
#include <string.h>                         /* Get strcpy, strcat, etc.       */
#include <ctype.h>                          /* Get toupper                    */
#include <unistd.h>                         /* get getcwd routine/environ     */
#include <limits.h>                         /* Get PATH_MAX                   */
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>

#include "RexxCore.h"                         /* global REXX definitions        */
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"
#include "SystemInterpreter.hpp"
#include "PackageManager.hpp"
#include "BufferClass.hpp"
#include "FileNameBuffer.hpp"


typedef struct _ENVENTRY {                  /* setlocal/endlocal structure    */
     size_t   size;                            /* size of the saved memory       */
} ENVENTRY;

int putflag = 0;                            /* static or dynamic env memory   */


/******************************************************************************/
/* Name:       SysExternalFunction                                            */
/*                                                                            */
/* Notes:      Handles searching for and executing an external function.  The */
/*             search order is:                                               */
/*               1) Macro-space pre-order functions                           */
/*               2) Registered external functions                             */
/*               3) REXX programs with same extension (if applicable)         */
/*               4) REXX programs with default extension                      */
/*               5) Macro-space post-order functions                          */
/******************************************************************************/
bool SystemInterpreter::invokeExternalFunction(
    RexxActivation *activation,         /* Current Activation                */
    Activity   *activity,           /* activity in use                   */
    RexxString     *target,             /* Name of external function         */
    RexxObject    **arguments,          /* Argument array                    */
    size_t           argcount,           /* count of arguments                */
    RexxString     *calltype,           /* Type of call                      */
    ProtectedObject &result)
{
    if (activation->callMacroSpaceFunction(target, arguments, argcount, calltype, RXMACRO_SEARCH_BEFORE, result))
    {
        return true;
    }
    /* no luck try for a registered func */
    if (PackageManager::callNativeRoutine(activity, target, arguments, argcount, result))
    {
        return true;
    }
    /* have activation do the call       */
    if (activation->callExternalRexx(target, arguments, argcount, calltype, result))
    {
        return true;
    }
    /* function.  If still not found,    */
    /* then raise an error               */
    if (activation->callMacroSpaceFunction(target, arguments, argcount, calltype, RXMACRO_SEARCH_AFTER, result))
    {
        return true;
    }

    return false;
}


/**
 * Push a new environment for the SysSetLocal() BIF.
 *
 * @param context The current activation context.
 *
 * @return Returns TRUE if the environment was successfully pushed.
 */
RexxObject* SystemInterpreter::pushEnvironment(RexxActivation *context)
{
    RexxObject *Current = buildEnvlist(); /* build the new save block          */
    if (Current == OREF_NULL)           /* if unsuccessful return zero       */
    {
        return TheFalseObject;
    }
    else
    {
        /* Have Native Actiovation           */
        context->pushEnvironment(Current);          /*  update environemnt list          */
        return TheTrueObject;              /* this returns one                  */
    }
}

/**
 * Pop an environment for the SysEndLocal() BIF.
 *
 * @param context The current activation context.
 *
 * @return Always returns FALSE.  This is a NOP on Windows.
 */
RexxObject* SystemInterpreter::popEnvironment(RexxActivation *context)
{
    BufferClass *Current =  (BufferClass *)context->popEnvironment(); /*  block, if ixisted.               */
    if (TheNilObject == Current)         /* nothing saved?                    */
    {
        return TheFalseObject;             /* return failure value              */
    }
    else
    {
        /* restore everything                */
        restoreEnvironment(Current->getData());
        return TheTrueObject;              /* this worked ok                    */
    }
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   BuildEnvlist                                 */
/*                                                                   */
/*   Descriptive Name:  Build saved environment block                */
/*                                                                   */
/*   Function:          Builds a block containing all of the         */
/*                      environment variables, the current drive     */
/*                      and the current directory.                   */
/*                                                                   */
/*********************************************************************/

RexxObject* SystemInterpreter::buildEnvlist()
{
    BufferClass *newBuffer;               /* Buffer object to hold env  */
    char      **Environment;             /* environment pointer        */
    size_t      size = 0;                /* size of the new buffer     */
    char       *New;                     /* starting address of buffer */
    Environment = getEnvironment();      /* get the ptr to the environ */

    for (; *Environment != NULL; Environment++)
    {
        size += strlen(*Environment);      /* calculate the size for all */
        size++;                            /* environment variables+'\0' */
    }                                    /* now get current dir        */
    if (!size)
    {
        return OREF_NULL;                  /* no envrionment !           */
    }

    FileNameBuffer curr_dir;

    // start with a copy of the current working directory
    SysFileSystem::getCurrentDirectory(curr_dir);
    size_t dirLen = curr_dir.length();

    size += dirLen;                      /* add the space for curr dir */
    size++;                              /* and its terminating '\0'   */
    size += sizeof(size_t);              /* this is for the size itself*/
    /* Now we have the size for   */
    /* allocating the new buffer  */
    newBuffer = new_buffer(size);        /* let's do it                */
    /* Get starting address of buf*/
    New = newBuffer->getData();
    ((ENVENTRY *)New)->size = size;       /* first write the size       */
    New += sizeof(size_t);               /* update the pointer         */
    strcpy(New, (const char *)curr_dir); // add the current dir to the buffer
    New += dirLen + 1;                   /* update the pointer         */
    Environment = getEnvironment();      /* reset to begin of environ  */
    /* Loop through environment   */
    /* and copy all entries to the*/
    /* buffer, each terminating   */
    /* with '\0'                  */
    for (; *Environment != NULL; Environment++)
    {
        // copy the entry and step over it
        size_t len = strlen(*Environment);
        strcpy(New, *Environment);
        New += len + 1;
    }
    return newBuffer;                    /* return the pointer         */
}


/**
 * Tell whether a block of saved environment entries has a variable
 * with the given name.
 *
 * @param entries The first saved "name=value" entry.
 * @param end     The end of the saved entries.
 * @param name    The name (not terminated).
 * @param length  The length of the name.
 *
 * @return true if one of the saved entries is for that name.
 */
static bool isSavedName(const char *entries, const char *end, const char *name, size_t length)
{
    for (const char *entry = entries; entry < end; entry += strlen(entry) + 1)
    {
        if (strncmp(entry, name, length) == 0 && entry[length] == '=')
        {
            return true;
        }
    }
    return false;
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   RestoreEnvironment                           */
/*                                                                   */
/*   Descriptive Name:  restores environment saved by Setlocal()     */
/*                                                                   */
/*   Function:          restores the environment variables, current  */
/*                      directory and drive.                         */
/*                                                                   */
/*********************************************************************/

void SystemInterpreter::restoreEnvironment(
    void *CurrentEnv)                    /* saved environment          */
{
    char  *current;                      /* ptr to saved environment   */
    size_t size;                         /* size of the saved space    */
    char  *begin;                        /* begin of saved space       */

    begin = current = (char *)CurrentEnv; /* get the saved space        */
    size = ((ENVENTRY *)current)->size;   /* first read out the size    */
    current += sizeof(size_t);           /* update the pointer         */
    if (chdir(current) == -1)             /* restore the curr dir       */
    {
        char msg[1024];
        snprintf(msg, sizeof(msg), "Error restoring current directory: %s", current);
        reportException(Error_System_service_service, msg);
    }
    current += strlen(current);          /* update the pointer         */
    current++;                           /* jump over '\0'             */

    // Remove the variables that were not there when the environment was
    // saved.  unsetenv() changes environ, so start the scan again after each.
    char *saved = current;
    bool removed = true;
    while (removed)
    {
        removed = false;
        for (char **entry = getEnvironment(); *entry != NULL; entry++)
        {
            // an entry with no name, or no '=', is nothing unsetenv() can remove
            const char *equal = strchr(*entry, '=');
            if (equal == NULL || equal == *entry)
            {
                continue;
            }
            size_t nameLength = (size_t)(equal - *entry);
            if (isSavedName(saved, begin + size, *entry, nameLength))
            {
                continue;
            }
            char *name = (char *)malloc(nameLength + 1);
            if (name == NULL)
            {
                reportException(Error_System_service_service, "Error restoring environment variable");
            }
            memcpy(name, *entry, nameLength);
            name[nameLength] = '\0';
            int result = unsetenv(name);
            free(name);
            if (result == -1)
            {
                reportException(Error_System_service_service, "Error restoring environment variable");
            }
            removed = true;
            break;
        }
    }

    // Restore the saved "name=value" entries with setenv(), which makes its
    // own copies.  The entries must not be given to putenv(): putenv() keeps
    // the pointer, and these point into the saved buffer, a Rexx object that
    // the garbage collector can move or reclaim.  Nor may the entries they
    // replace be freed: one set with VALUE() belongs to setenv(), which keeps
    // it and may hand it out again.
    while ((size_t)(current - begin) < size)
    {
        size_t length = strlen(current);
        char *equal = strchr(current, '=');
        // an entry with no name, or no '=', is not something we can set
        if (equal != NULL && equal != current)
        {
            // split the entry in place for the call, then put it back
            *equal = '\0';
            int result = setenv(current, equal + 1, 1);
            *equal = '=';
            if (result == -1)
            {
                reportException(Error_System_service_service, "Error restoring environment variable");
            }
        }
        current += length + 1;
    }
}

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
/* REXX AIX Support                                            aixextf.c      */
/*                                                                            */
/* AIX specific external function lookup and AIX built-in functions.          */
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

#include "RexxCore.h"                         /* global REXX definitions        */
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"
#include "SystemInterpreter.hpp"
#include "PackageManager.hpp"
#include "BufferClass.hpp"


#define CMDBUFSIZE      1024                 /* Max size of executable cmd     */
#if defined(AIX)
#define CMDDEFNAME      "/bin/ksh"           /* Default unix sys cmd handler   */
#define COMSPEC         "ksh"                /* unix cmd handler env name      */
#define SYSENV          "ksh"                /* Default AIX  cmd environment   */
#elif defined(OPSYS_SUN)
#define CMDDEFNAME      "/bin/sh"            /* Default unix sys cmd handler   */
#define COMSPEC         "sh"                 /* unix cmd handler env name      */
#define SYSENV          "sh"                 /* Default LINUX cmd environment  */
#else
#define CMDDEFNAME      "/bin/bash"          /* Default unix sys cmd handler   */
#define COMSPEC         "bash"               /* unix cmd handler env name      */
#define SYSENV          "bash"               /* Default AIX  cmd environment   */
#endif

#define DRVNUM          0x40                /* drive number subtractor        */
#define DIRLEN          256                 /* length of a directory          */
#define FULLSEG         65536L              /* ^4K constant                   */
                                            /* FILESPEC function options      */
#define FILESPEC_PATH         'P'
#define FILESPEC_NAME         'N'
#define FILESPEC_LOCATION     'L'
#define FILESPEC_EXTENSION    'E'

#define KIOCSOUND   0x4B2F              /* start sound generation (0 for off) */

typedef struct _ENVENTRY {                  /* setlocal/endlocal structure    */
  size_t   size;                            /* size of the saved memory       */
} ENVENTRY;

int putflag = 0;                            /* static or dynamic env memory   */

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   sysBeep                                      */
/*                                                                   */
/*   Descriptive Name:  BEEP function                                */
/*                                                                   */
/*   Function:          sounds the speaker if possible, else flash   */
/*                      the screen                                   */
/*********************************************************************/

RexxRoutine2(CSTRING, sysBeep, OPTIONAL_wholenumber_t, Frequency, OPTIONAL_wholenumber_t, Duration)
{
                                        /* console beep for Unix     */
  printf("\a");
  return "";                            /* always returns a null     */
}


/********************************************************************
* Function:  resolve_tilde(path)                                    *
*                                                                   *
* Purpose:   Resolves path names including '~'.                     *
*                                                                   *
* RC:        Returns the absolute path in new allocated space.      *
*                                                                   *
*********************************************************************/
char *resolve_tilde(const char *path)
{
    const char * st;
    const char *home_dir = NULL;            /* home directory path        */
    char  *dir_buf = NULL;             /* full directory path        */
    const char * slash;
    char   username[100];
    struct passwd *ppwd;

    st = path;
    /* if no user name            */
    if (*(st) == '~' && (*(st+1) == '\0' || *(st+1) == '/'|| *(st+1) == ' ' ))
    {
        if (*(st+1) == '/')
        {              /* if there is a path         */
            st +=2;                        /* jump over '~/'             */
                                           /* get home directory path    */
            home_dir = getenv("HOME");     /* from the environment       */
            if (!home_dir)                  /* if no home dir info        */
                return(0);
            /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(home_dir)+strlen(st)+2);
            if (!dir_buf)
                return(0);
            /* merge the strings          */
            sprintf(dir_buf, "%s/%s", home_dir, st);
            return dir_buf;
        }
        else
        {
            /* get home directory path    */
            home_dir = getenv("HOME");     /* from the environment       */
                                           /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(home_dir)+2);
            if (!dir_buf)
                return(0);
            sprintf(dir_buf, "%s/", home_dir);
            return dir_buf;
        }
    }
    else if (*(st) == '~')
    {             /* cmd is '~username...'      */
        st++;                            /* jump over '~'              */
        slash = strchr(st,'/');          /* search for '/'             */
        if (!slash)
        {                      /* if no '/'                  */
                               /* rest of string is username */
            ppwd = getpwnam(st);           /* get info about the user    */
                                           /* get space for the buf      */
            if (ppwd == NULL)
            {                  /* no user                    */
                return NULL;                     /* nothing happend            */
            }
            dir_buf = (char *)malloc(strlen(ppwd->pw_dir)+2);
            if (!dir_buf)
                return NULL;
            /* merge the strings          */
            sprintf(dir_buf, "%s/", ppwd->pw_dir);
        }
        else
        {                            /* there is a slash           */
                                     /* copy the username into a   */
                                     /* local buffer; 100 bytes    */
                                     /* should be big enough       */
                                     /* fixes bug 1695834          */
            memcpy(username, st, slash-st);
            username[slash-st] = '\0';
            ppwd = getpwnam(username);     /* get info about the user    */
            slash++;                       /* step over the slash        */
                                           /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(ppwd->pw_dir)+strlen(slash)+2);
            if (!dir_buf)
                return NULL;
            /* merge the strings          */
            sprintf(dir_buf, "%s/%s", ppwd->pw_dir, slash);
        }
        return dir_buf;                  /* directory change to        */
    }
    return NULL;
}

/****************************************************************************/
/* sysDirectory                                                             */
/****************************************************************************/
RexxRoutine1(RexxStringObject, sysDirectory, OPTIONAL_CSTRING, dir)
{
  RexxReturnCode rc;
  char  *rdir;                         /* resolved path */

  rc = 0;
  if (dir != NO_CSTRING)               /* if new directory is not null,     */
  {
    if(*dir == '~')
    {
      rdir = resolve_tilde(dir);
      rc = chdir(rdir);
      free(rdir);
    }
    else
      rc = chdir(dir);                   /* change to the new directory     */
  }

  // get the current working directory and return it
  char temp[PATH_MAX + 3];
  SystemInterpreter::getCurrentWorkingDirectory(temp);
  return context->NewStringFromAsciiz(temp);
}


/*****************************************************************************/
/* sysFilespec                                                               */
/*****************************************************************************/
RexxRoutine2(RexxStringObject, sysFilespec, CSTRING, option, CSTRING, name)
{
    const char *endPtr = name + strlen(name);        // point to last character
    const char *pathEnd = strrchr(name, '/');       // find the last slash in name
    // get the end of the path portion (if any)
    const char *pathStart = name;
    // note that pathend is one character past the end of the path.
    // this means the length is easily calculated as pathEnd - pathStart,
    // even in the cases where there is no patch portion
    pathEnd = pathEnd == NULL ? pathStart : pathEnd + 1;
    // this one needs a little adjustment for the case where this is all name
    const char *nameStart = pathEnd == name ? name : pathEnd;

    switch (toupper(*option))              /* process each option               */
    {
        case FILESPEC_PATH:                /* extract the path                  */
            {
                return context->String(pathStart, pathEnd - pathStart);
            }

        case FILESPEC_NAME:                  /* extract the file name               */
            {                                /* everything to right of slash        */
                return context->String(nameStart, endPtr - nameStart);
            }

        case FILESPEC_LOCATION:          /* extract the file name               */
            {                                /* everything to left of slash        */
                return context->String(name, pathEnd - name);
            }

        case FILESPEC_EXTENSION:           // extract the file extension
            {
                // find the position of the last dot
                const char *lastDot = strrchr(name, '.');

                if (lastDot >= nameStart)
                {
                    // we don't extract the period
                    lastDot++;
                    return context->String(lastDot, endPtr - lastDot);
                }
                else
                {
                    return context->NullString();        // nothing found, return the empty string
                }

            }
        default:                           /* unknown option                    */
        {
            char optionChar[2];
            optionChar[0] = *option;
            optionChar[1] = '\0';

            RexxArrayObject subs = context->Array(context->String("FILESPEC"), context->WholeNumberToObject(1),
                context->String("ELNP"), context->String(optionChar));
            /* raise an error                    */
            context->RaiseException(Rexx_Error_Incorrect_call_list, subs);
            return NULLOBJECT;
        }
    }
}


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
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxObject    ** arguments,          /* Argument array                    */
  size_t           argcount,           /* count of arguments                */
  RexxString     * calltype,           /* Type of call                      */
  ProtectedObject &result)
{
    if (activation->callMacroSpaceFunction(target, arguments, argcount, calltype, MS_PREORDER, result))
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
    if (activation->callMacroSpaceFunction(target, arguments, argcount, calltype, MS_POSTORDER, result))
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
RexxObject *SystemInterpreter::pushEnvironment(RexxActivation *context)
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
RexxObject *SystemInterpreter::popEnvironment(RexxActivation *context)
{
    RexxBuffer *Current =  (RexxBuffer *)context->popEnvironment();/*  block, if ixisted.               */
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

RexxObject *SystemInterpreter::buildEnvlist()
{
    RexxBuffer *newBuffer;               /* Buffer object to hold env  */
    char      **Environment;             /* environment pointer        */
    size_t      size = 0;                /* size of the new buffer     */
    char       *curr_dir;                /* current directory          */
    char       *New;                     /* starting address of buffer */
    Environment = getEnvironment();      /* get the ptr to the environ */

    for (;*Environment != NULL;Environment++)
    {
        size += strlen(*Environment);      /* calculate the size for all */
        size++;                            /* environment variables+'\0' */
    }                                    /* now get current dir        */
    if (!size)
    {
        return OREF_NULL;                  /* no envrionment !           */
    }
    if (!(curr_dir=(char *)malloc(PATH_MAX + 3)))/* malloc storage for cwd*/
    {
        reportException(Error_System_service);
    }

    // start with a copy of the current working directory
    SystemInterpreter::getCurrentWorkingDirectory(curr_dir);
    size += strlen(curr_dir);            /* add the space for curr dir */
    size++;                              /* and its terminating '\0'   */
    size += sizeof(size_t);              /* this is for the size itself*/
                                         /* Now we have the size for   */
                                         /* allocating the new buffer  */
    newBuffer = new_buffer(size);        /* let's do it                */
                                         /* Get starting address of buf*/
    New = newBuffer->getData();
    ((ENVENTRY*)New)->size = size;       /* first write the size       */
    New += sizeof(size_t);               /* update the pointer         */
                                         /* now write the curr dir     */
    memcpy(New,curr_dir,strlen(curr_dir));
    New += strlen(curr_dir);             /* update the pointer         */
    memcpy(New,"\0",1);                  /* write the terminator       */
    New++;                               /* update the pointer         */
    Environment = getEnvironment();      /* reset to begin of environ  */
                                         /* Loop through environment   */
                                         /* and copy all entries to the*/
                                         /* buffer, each terminating   */
                                         /* with '\0'                  */
    for (;*Environment != NULL;Environment++)
    {
        /* copy the entry             */
        memcpy(New,*Environment,strlen(*Environment));
        New += strlen(*Environment);       /* update the pointer         */
        memcpy(New,"\0",1);                /* write the terminator       */
        New++;                             /* update the pointer         */
    }
    free(curr_dir);                      /* free curr dir buffer       */
    return newBuffer;                    /* return the pointer         */
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
    size_t length;                       /* string length              */
    char  *begin;                        /* begin of saved space       */
    char  **Environment;                 /* environment pointer        */

    char  *del = NULL;                   /* ptr to old unused memory   */
    char  *Env_Var_String;               /* enviornment entry          */
    char   namebufsave[256],namebufcurr[256];
    char  *np;
    int i;

    Environment = getEnvironment();    /* get the current environment*/

    begin = current = (char *)CurrentEnv;/* get the saved space        */
    size = ((ENVENTRY*)current)->size;   /* first read out the size    */
    current += sizeof(size_t);           /* update the pointer         */
    if (chdir(current) == -1)             /* restore the curr dir       */
    {
        char msg[1024];
        sprintf(msg, "Error restoring current directory: %s", current);
        reportException(Error_System_service_service, msg);
    }
    current += strlen(current);          /* update the pointer         */
    current++;                           /* jump over '\0'             */
    if (!putflag)
    {                        /* first change in the        */
                             /* environment ?              */
        /* copy all entries to dynamic memory                            */
        /*for all entries in the env  */
        for (;*Environment != NULL;Environment++)
        {
            length = strlen(*Environment)+1; /* get the size of the string */
                                             /* and alloc space for it     */
            Env_Var_String = (char *)malloc(length);
            memcpy(Env_Var_String,*Environment,length);/* copy the string  */
            putenv(Env_Var_String);          /* and make it part of env    */
        }
        putflag = 1;                       /* prevent do it again        */
    }
    /* Loop through the saved env */
    /* entries and restore them   */
    for (;(size_t)(current-begin)<size;current+=(strlen(current)+1))
    {
        Environment = getEnvironment();    /* get the environment        */
        del = NULL;
        np = current;
        /* extract the the name       */
        /* from the saved enviroment  */
        for (i=0;(*np!='=')&&(i<255);np++,i++)
        {
            memcpy(&(namebufsave[i]),np,1);  /* copy the character         */
        }
        memcpy(&(namebufsave[i]),"\0",1);  /* copy the terminator        */
                                           /* find the entry in the env  */
        for (;*Environment != NULL;Environment++)
        {
            np = *Environment;
            /* extract the the name       */
            /* from the current env       */
            for (i=0;(*np!='=')&&(i<255);np++,i++)
            {
                memcpy(&(namebufcurr[i]),np,1);/* copy the character         */
            }
            memcpy(&(namebufcurr[i]),"\0",1);/* copy the terminator        */

            if (!strcmp(namebufsave,namebufcurr))
            {/* have a match ?         */
                del = *Environment;            /* remember it for deletion   */
                break;                         /* found, so get out of here  */
            }
        }
        if (putenv(current) == -1)
        {
            reportException(Error_System_service_service, "Error restoring environment variable");
        }
        if (del)                            /* if there was an old entry  */
        {
            free(del);                       /* free it                    */
        }
    }
}

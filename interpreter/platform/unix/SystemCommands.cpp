/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX UNIX  Support                                            aixcmd.c     */
/*                                                                            */
/* AIX specific command processing routines                                   */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*  aixcmd.c - C methods for handling calls to system exits and subcommand    */
/*             handlers.                                                      */
/*                                                                            */
/*  C methods:                                                                */
/*    SysCommand     - Method to invoke a subcommand handler                  */
/*                                                                            */
/*  Internal routines:                                                        */
/*    sys_command - Run a command through system command processor.           */
/******************************************************************************/

#include <string.h>                         /* Get strcpy, strcat, etc.       */
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#include "RexxCore.h"                         /* global REXX declarations       */
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "SysInterpreterInstance.hpp"

#include "RexxInternalApis.h"
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

#define CMDBUFSIZE 1024                     /* Max size of executable cmd     */
#define MAX_COMMAND_ARGS 400

#if defined(AIX)
#define CMDDEFNAME "/bin/ksh"               /* Korn shell is default for AIX */
#elif defined(OPSYS_SUN)                        /*  path for AIX        */
#define CMDDEFNAME "/bin/sh"                /* Bourne Again Shell is default */
#else                                       /* shell for Linux               */
#define CMDDEFNAME "/bin/bash"              /* Bourne Again Shell is default */
#endif

#define UNKNOWN_COMMAND 127                 /* unknown command return code    */

#define SYSENV "command"                    /* Default cmd environment        */
#define SHELL  "SHELL"                      /* UNIX cmd handler env. var. name*/
#define EXPORT_FLAG 1
#define SET_FLAG    2
#define UNSET_FLAG  3
#define MAX_VALUE   1280

extern int putflag;


/**
 * Retrieve the globally default initial address.
 *
 * @return The string name of the default address.
 */
RexxString *SystemInterpreter::getDefaultAddressName()
{
    return OREF_INITIALADDRESS;
}

/* Handle "export" command in same process */
bool sys_process_export(RexxExitContext *context, const char * cmd, RexxObjectPtr &rc, int flag)
{
    char *Env_Var_String = NULL;         /* Environment variable string for   */
    size_t size, allocsize;              /* size of the string                */
    char      **Environment;             /* environment pointer               */
    char  *np;
    size_t i,j,k,l,iLength, copyval;
    char   namebufcurr[1281];             /* buf for extracted name            */
    char   cmd_name[1281];                /* name of the envvariable setting   */
    char   *array, *runarray, *runptr, *endptr, *maxptr;
    char   temparray[1281];
    const char *st;
    char  *tmpptr;
    char   name[1281];                    /* is the name + value + =           */
    char   value[1281];                   /* is the part behind =              */
    char  *del = NULL;                    /* ptr to old unused memory          */
    char  *hit = NULL;
    bool   HitFlag = false;
    l = 0;
    j = 0;
    allocsize = 1281 * 2;

    memset(temparray, '\0', sizeof(temparray));

    Environment = getEnvironment();       /* get the environment               */
    if (flag == EXPORT_FLAG)
    {
        st = &cmd[6];
    }
    else if (flag == UNSET_FLAG)
    {
        st = &cmd[5];
    }
    else
    {
        st = &cmd[3];
    }
    while ((*st) && (*st == ' '))
    {
        st++;
    }
    strcpy(name, st);
    iLength = strlen(st) + 1;

/* if string == EXPORT_FLAG or string == SET and only blanks are delivered */

    if ( ((flag == EXPORT_FLAG) || (flag == SET_FLAG)) &&  (iLength == 1) )
    {
        return false;
    }

    if (!putflag)
    {                        /* first change in the environment ? */
        /* copy all entries to dynamic memory                                   */
        for (;*Environment != NULL; Environment++)
        {                                  /*for all entries in the env         */
            size = strlen(*Environment)+1;   /* get the size of the string        */
            Env_Var_String = (char *)malloc(size); /* and alloc space for it      */
            memcpy(Env_Var_String,*Environment,size);/* copy the string           */
            putenv(Env_Var_String);          /* and chain it in                   */
        }
    }
    putflag = 1;                         /* prevent do it again               */
    Environment = getEnvironment();      /* reset the environment pointer     */

/* do we have a assignment operator? If not return true           */
/* The operating system treads this like no command, and so do we */

    if ( !(strchr(name, '=')) && (flag != UNSET_FLAG) ) /*only set and export */
    {
/* we do not have a assignment operator, but maybe a '|' for a    */
/* controlled output                                              */
        if ( (strchr(name, '|'))  || (strchr(name, '>')) || (strstr(name, ">>")) )
        {
            return false;
        }
        else
        {
            // this worked ok (well, sort of)
            rc = context->False();
            return true;
        }
    }

/* no '=' for unset, so force a shell error message               */

    if ( (strchr(name, '=')) && (flag == UNSET_FLAG) )
    {
        return false;
    }

    for (i=0;(name[i]!='=')&&(i<iLength);name[i++])
    {
        cmd_name[i] = name[i];
    }

/* lets try handling variables in the assignment string */

    cmd_name[i]  = '\0';                /* copy the terminator           */

    i++;                                /* the place after'=' */

/* lets search the value for variable part(s)  */

    strcpy(value, &(name[i]));         /* value contains the part behind '=' */
    array = (char *) malloc(1281);
    strcpy(array, cmd_name);
    array[strlen(cmd_name)] = '=';
    array[i] = '\0';                   /* copy the terminator           */
    runarray = array + strlen(array);
    runptr = value;
    endptr = runptr + strlen(value);   /*this is the end of the input*/
    maxptr = array + MAX_VALUE -1;     /* this is the end of our new string */

    while ((tmpptr = (strchr(runptr, '$'))) != 0)
    {
        Environment = getEnvironment();  /* get the beginning of the environment*/
        HitFlag = true;          /* if not true inputvalue= outputvalue*/
        copyval = tmpptr - runptr;
        if (copyval)   /* runarray should keep the 'real' environment  */
        {
            while ((runarray + copyval) >  maxptr)
            {
                array = (char *) realloc(array, allocsize);
                runarray = array + strlen(array);
                maxptr = array + allocsize - 1;
                allocsize = allocsize * 2;
            }
            memcpy(runarray,runptr, copyval);
            runarray= runarray + copyval; /* a new place to copy */
            *runarray = '\0';
            runptr = tmpptr;              /* now runptr is at the place of $ */
        }
        runptr++;
        for (j = 0;(*runptr != '/') && (*runptr != ':') && (*runptr != '$') &&
            (*runptr); j++)
        {
            memcpy(&(temparray[j]), runptr,1); /*temparray is the env var to search*/
            runptr++;
        }

        temparray[j] = '\0';                /* lets see what we can do    */
        np = NULL;

        for (;(*Environment != NULL) && (hit == NULL) ;Environment++)
        {
            np = *Environment;

            for (k=0;(*np!='=')&&(k<255);np++,k++)
            {
                memcpy(&(namebufcurr[k]),np,1);  /* copy the character                */
            }

            namebufcurr[k] = '\0';                 /* copy the terminator           */

            if (!strcmp(temparray,namebufcurr))     /* have a match ?         */
            {
                hit = *Environment;
                /* copy value to new string*/
            }
        }
        if (hit) /* if we have found an entry of the env var in the env list */
        {
            np ++;                  /* don't copy equal                     */
            while ((runarray + strlen(np)) >  maxptr)
            {
                array = (char *) realloc(array, allocsize);
                runarray = array + strlen(array);
                maxptr = array + allocsize - 1;
                allocsize = allocsize * 2;
            }
            strcpy(runarray, np);
            runarray = runarray + strlen(np);
            *runarray = '\0';
            hit = NULL;
        }
    }

    if (HitFlag == true)
    {
        if (runptr < endptr)
        {
            while ((runarray + strlen(runptr)) >  maxptr)
            {
                array = (char *) realloc(array, allocsize);
                runarray = array + strlen(array);
                maxptr = array + allocsize - 1;
                allocsize = allocsize * 2;
            }
            strcpy(runarray, runptr);      /* if there is a part after a var */
            runarray = runarray + strlen(runptr);
            *runarray = '\0';
        }
    }
    else   /* no hit so lets copy the value as it is                     */
    {
        while ((runarray + strlen(value)) >  maxptr)
        {
            array = (char *) realloc(array, allocsize);
            runarray = array + strlen(array);
            maxptr = array + allocsize - 1;
            allocsize = allocsize * 2;
        }
        strcpy(runarray,value);
        runarray = runarray + strlen(runptr);
        *runarray = '\0';
    }

    Environment = getEnvironment();     /* get the beginning of the environment*/

    for (;*Environment != NULL;Environment++)
    {
        np = *Environment;

        for (i=0;(*np!='=')&&(i<255);np++,i++)
        {
            memcpy(&(namebufcurr[i]),np,1);  /* copy the character                */
        }

        namebufcurr[i] = '\0';                 /* copy the terminator          */

        if (!strcmp(cmd_name,namebufcurr))/* have a match ?         */
        {
            del = *Environment;              /* remember it for deletion          */
        }
    }
    /* find the entry in the environ     */
    if (flag != UNSET_FLAG)
    {
        size = strlen(array)+1;
        Env_Var_String = (char *)malloc(size);/* get the memory                    */
        memcpy(Env_Var_String, array, size);
        int errCode = putenv(Env_Var_String);
        if (errCode != 0)
        {
            // non-zero is an error condition
            context->RaiseCondition("ERROR", context->String(cmd), NULL, context->WholeNumberToObject(errCode));
        }
        else
        {
            rc = context->False();
        }
    }

    if (del)                              /* if there was a old one            */
    {
        free(del);                         /* free it                           */
    }
    rc = context->False();
    return true;
}


/* Returns a copy of s without quotes. Escaped characters are kept unchanged */
char *unquote(const char *s)
{
    if ( s == NULL )
    {
        return NULL;
    }
    size_t size = strlen(s) + 1;
    char *unquoted = (char*)malloc(sizeof(char)*size);
    if ( unquoted == NULL )
    {
        return NULL;
    }
    char *u = unquoted;
    char c;
    bool escape = false;
    do
    {
        c = *s;
        if ( escape )
        {
            *u++ = *s;
            escape = false;
        }
        else if ( c == '\\' )
        {
            escape = true;
        }
        else if ( c != '"' )
        {
            *u++ = *s;
        }
        s++;
    }
    while ( c != '\0' );
    return unquoted;
}


/* Handle "cd XXX" command in same process */
bool sys_process_cd(RexxExitContext *context, const char * cmd, RexxObjectPtr rc)
{
    const char * st;
    const char *home_dir = NULL;            /* home directory path        */
    char *dir_buf = NULL;             /* full directory path        */
    const char *slash;                      /* ptr to '/'                 */
    struct passwd *ppwd;

    st = &cmd[2];
    while ((*st) && (*st == ' '))
    {
        st++;
    }
    if ((!*st) || (strlen(cmd) == 2))
    {
        home_dir = getenv("HOME");
        if (!home_dir)
        {
            return false;
        }
        dir_buf = (char *)malloc(strlen(home_dir)+1);
        strcpy(dir_buf, home_dir);
    }                                  /* if no user name            */
    else if (*(st) == '~' && (*(st+1) == '\0' || *(st+1) == '/'|| *(st+1) == ' ' ))
    {
        if (*(st+1) == '/')              /* if there is a path         */
        {
            st +=2;                        /* jump over '~/'             */
                                           /* get home directory path    */
            home_dir = getenv("HOME");     /* from the environment       */
            if (!home_dir)                  /* if no home dir info        */
            {
                return false;
            }
            /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(home_dir)+strlen(st)+2);
            if (!dir_buf)
            {
                return false;
            }
            /* merge the strings          */
            sprintf(dir_buf, "%s/%s", home_dir, st);
        }
        else
        {
            /* get home directory path    */
            home_dir = getenv("HOME");     /* from the environment       */
                                           /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(home_dir)+2);
            if (!dir_buf)
            {
                return false;
            }
            sprintf(dir_buf, "%s/", home_dir);
        }
    }
    else if (*(st) == '~')             /* cmd is '~username...'      */
    {
        st++;                            /* jump over '~'              */
        slash = strchr(st,'/');          /* search for '/'             */
        if (!slash)                      /* if no '/'                  */
        {
            /* rest of string is username */
            ppwd = getpwnam(st);           /* get info about the user    */
            if (ppwd == NULL || ppwd->pw_dir == NULL)
            {
                return false;
            }
                                           /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(ppwd->pw_dir)+2);
            if (!dir_buf)
            {
                return false;
            }
            /* merge the strings          */
            sprintf(dir_buf, "%s/", ppwd->pw_dir);
        }
        else                            /* there is a slash           */
        {
            char username[256];            // need to copy the user name
            memcpy(username, st, slash - st);
            username[slash - st] = '\0';

            ppwd = getpwnam(username);     /* get info about the user    */
            if (ppwd == NULL || ppwd->pw_dir == NULL)
            {
                return false;
            }
            slash++;                       /* step over the slash        */
                                           /* get space for the buf      */
            dir_buf = (char *)malloc(strlen(ppwd->pw_dir)+strlen(slash)+2);
            if (!dir_buf)
            {
                return false;
            }
            /* merge the strings          */
            sprintf(dir_buf, "%s/%s", ppwd->pw_dir, slash);
        }
    }
    else
    {
        dir_buf = strdup(st);
    }

    char *unquoted = unquote(dir_buf);
    if (unquoted == NULL)
    {
        return false;
    }
    int errCode = chdir(unquoted);
    free(unquoted);

    free(dir_buf);
    if (errCode != 0)
    {
        // non-zero is an error condition
        context->RaiseCondition("ERROR", context->String(cmd), NULL, context->WholeNumberToObject(errCode));
    }
    else
    {
        rc = context->False();
    }
    return true;
}


/*********************************************************************/
/* This function breaks a command up into whitespace-delimited pieces*/
/* to create the pointer array for the execvp call.  It is only used */
/* to support the "COMMAND" command environment, which does not use  */
/* a shell to invoke its commands.                                   */
/*********************************************************************/

bool scan_cmd(const char *parm_cmd, char **argPtr)
{
    char *cmd = strdup(parm_cmd);        /* Allocate for copy          */

    char *end = cmd + strlen(cmd);       /* Find the end of the command*/

    /* This loop scans our copy of the command, setting pointers in    */
    /* the args[] array to point to each of the arguments, and null-   */
    /* terminating each one of them.                                   */

    /* LOOP INVARIANT:                                                 */
    /* pos points to the next character of the command to be examined. */
    /* i indicates the next element of the args[] array to be loaded.  */
    size_t i = 0;                               /* Start with args[0]         */
    for (char *pos = cmd; pos < end; pos++)
    {
        while (*pos==' ' || *pos=='\t')
        {
            pos++;                           /* Skip to first non-white    */
        }

        if (*pos == '\0')                  /* If we're at the end,       */
        {
            break;                           /* get out.                   */
        }

        /* If at this point, we've used up all but one of the available  */
        /* elements of our args[] array, let the user know, and we must  */
        /* terminate.                                                    */
        if (i == MAX_COMMAND_ARGS)
        {
            return false;
        }

        argPtr[i++] = pos;                 /* Point to current argument  */
                                           /* and advance i to next      */
                                           /* element of args[]          */
        while (*pos!=' ' && *pos!='\t' && *pos!='\0')
        {
            pos++;                           /* Look for next whitespace   */
        }                                  /* or end of command          */
        *pos = '\0';                       /* Null-terminate this arg    */

    }

    /* Finally, put a null pointer in args[] to indicate the end.      */
    argPtr[i] = NULL;
    return true;
}

/******************************************************************************/
/* Name:       sys_command                                                    */
/*                                                                            */
/* Arguments:  cmd - Command to be executed                                   */
/*             local_env_type - integer indicating which shell                */
/*                                                                            */
/* Returned:   rc - Return Code                                               */
/*                                                                            */
/* Notes:      Handles processing of a system command.                        */
/*             Uses the 'fork' and 'exec' system calls to create a new process*/
/*             and invoke the shell indicated by the local_env_type argument. */
/*             This is modeled after command handling done in Classic REXX.   */
/******************************************************************************/
RexxObjectPtr RexxEntry systemCommandHandler(RexxExitContext *context, RexxStringObject address, RexxStringObject command)
{
    const char *cmd = context->StringData(command);
    const char *envName = context->StringData(address);

    RexxObjectPtr rc = NULLOBJECT;

    /* check for redirection symbols, ignore them when enclosed in double quotes.
       escaped quotes are ignored. */
    bool noDirectInvoc = false;
    bool inQuotes = false;
    bool escape = false;
    size_t i;
    for (i = 0; i<strlen(cmd); i++)
    {
        if (escape)
        {
            escape = false;
        }
        else if (cmd[i] == '\\')
        {
            escape = true;
        }
        else if (cmd[i] == '"')
        {
            inQuotes = !inQuotes;
        }
        else
        {
            /* if we're in the unquoted part and the current character is one of */
            /* the redirection characters or the & for multiple commands then we */
            /* will no longer try to invoke the command directly                 */
            if (!inQuotes && (strchr("<>|&", cmd[i]) != NULL))
            {
                noDirectInvoc = true;
                break;
            }
        }
    }

    if (!noDirectInvoc)
    {
        /* execute 'cd' in the same process */
        size_t commandLen = strlen(cmd);

        if (strcmp(cmd, "cd") == 0)
        {
            if (sys_process_cd(context, cmd, rc))
            {
                return rc;
            }
        }
        else if (commandLen >= 3)
        {
            char tmp[16];
            strncpy(tmp, cmd, 3);
            tmp[3] = '\0';
            if (strcmp("cd ",tmp) == 0)
            {
                if (sys_process_cd(context, cmd, rc))
                {
                    return rc;
                }
            }
            strncpy(tmp, cmd, 4);
            tmp[4] = '\0';
            if (strcmp("set ",tmp) == 0)
            {
                if (sys_process_export(context, cmd, rc, SET_FLAG)) /*unset works fine for me*/
                {
                    return rc;
                }
            }
            strncpy(tmp, cmd, 6);
            tmp[6] = '\0';
            if (Utilities::strCaselessCompare("unset ", tmp) == 0)
            {
                if (sys_process_export(context, cmd, rc, UNSET_FLAG))
                {
                    return rc;
                }
            }
            strncpy(tmp, cmd, 7);
            tmp[7] = '\0';
            if (Utilities::strCaselessCompare("export ", tmp) == 0)
            {
                if (sys_process_export(context, cmd, rc, EXPORT_FLAG))
                {
                    return rc;
                }
            }
        }
    }


    /****************************************************************************/
    /* Invoke the system command handler to execute the command                 */
    /****************************************************************************/
    // if this is the null string, then use the default address environment
    // for the platform
    if (strlen(envName) == 0)
    {
        envName = SYSINITIALADDRESS;
    }

    int errCode = 0;
#ifdef LINUX

    if (Utilities::strCaselessCompare("bash", envName) == 0)
    {
        errCode = system( cmd );
        if ( errCode >= 256 )
        {
            errCode = errCode / 256;
        }
    }
    else
#endif
    {
        int pid = fork();
        int status;

        if (pid != 0)                         /* spawn a child process to run the  */
        {
            waitpid ( pid, &status, 0);          /* command and wait for it to finish */
            if (WIFEXITED(status))               /* If cmd process ended normal       */
            {
                                                 /* Give 'em the exit code            */
                errCode = WEXITSTATUS(status);
            }
            else                              /* Else process died ugly, so        */
            {
                errCode = -(WTERMSIG(status));
                if (errCode == 1)                    /* If process was stopped            */
                {
                    errCode = -1;                   /* Give 'em a -1.                    */
                }
            }
        }
        else
        {                                /* run the command in the child      */
            if (Utilities::strCaselessCompare("sh", envName) == 0)
            {
                execl("/bin/sh", "sh", "-c", cmd, NULL);
            }
            else if (Utilities::strCaselessCompare("ksh", envName) == 0)
            {
                execl("/bin/ksh", "ksh", "-c", cmd, NULL);
            }
            else if (Utilities::strCaselessCompare("bsh", envName) == 0)
            {
                execl("/bin/bsh", "bsh", "-c", cmd, NULL);
            }
            else if (Utilities::strCaselessCompare("csh", envName) == 0)
            {
                execl("/bin/csh", "csh", "-c", cmd, NULL);
            }
            else if (Utilities::strCaselessCompare("bash", envName) == 0)
            {
                execl("/bin/bash", "bash", "-c", cmd, NULL);
            }
            else if (Utilities::strCaselessCompare("cmd", envName) == 0)
            {
                char * args[MAX_COMMAND_ARGS+1];      /* Array for argument parsing */
                if (!scan_cmd(cmd, args))             /* Parse cmd into arguments  */
                {
                    exit(1);
                }
                execvp(args[0], args);           /* Invoke command directly   */
                perror(" *E* Address COMMAND");  /* If we get to this point,  */
                exit(1);                         /* we couldn't run the       */
            }
            else
            {
                execl("/bin/sh", "sh", "-c", cmd, NULL);
            }
        }
    }
    // unknown command code?
    if (errCode == UNKNOWN_COMMAND)
    {
        // failure condition
        context->RaiseCondition("FAILURE", context->String(cmd), NULL, context->WholeNumberToObject(errCode));
    }
    else if (errCode != 0)
    {
        // non-zero is an error condition
        context->RaiseCondition("ERROR", context->String(cmd), NULL, context->WholeNumberToObject(errCode));
    }
    return context->False();      // zero return code
}


/**
 * Register the standard system command handlers.
 *
 * @param instance The created instance.
 */
void SysInterpreterInstance::registerCommandHandlers(InterpreterInstance *_instance)
{
    // Unix has a whole collection of similar environments, services by a single handler
    _instance->addCommandHandler("COMMAND", (REXXPFN)systemCommandHandler);
    _instance->addCommandHandler("", (REXXPFN)systemCommandHandler);
    _instance->addCommandHandler("SH", (REXXPFN)systemCommandHandler);
    _instance->addCommandHandler("KSH", (REXXPFN)systemCommandHandler);
    _instance->addCommandHandler("CSH", (REXXPFN)systemCommandHandler);
    _instance->addCommandHandler("BSH", (REXXPFN)systemCommandHandler);
    _instance->addCommandHandler("BASH", (REXXPFN)systemCommandHandler);
}






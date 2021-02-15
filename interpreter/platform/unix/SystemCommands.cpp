/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX UNIX  Support                                                         */
/*                                                                            */
/* unix specific command processing routines                                  */
/*                                                                            */
/******************************************************************************/

#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <spawn.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "SysInterpreterInstance.hpp"
#include "SysThread.hpp"

#include "RexxInternalApis.h"
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <errno.h>

// "sh" should be our initial ADDRESS() environment across all Unix platforms
#define SYSINITIALADDRESS "sh"

#define MAX_COMMAND_ARGS 400
#define UNKNOWN_COMMAND 127                 /* unknown command return code    */

#define EXPORT_FLAG 1
#define SET_FLAG    2
#define UNSET_FLAG  3
#define MAX_VALUE   1280

extern int putflag;

// a thread that writes INPUT data to the command pipe
class InputWriterThread : public SysThread
{

public:
    inline InputWriterThread() : SysThread(),
        pipe(0), inputBuffer(NULL), bufferLength(0), error(0) { }
    inline ~InputWriterThread() { terminate(); }

    void start(int _pipe)
    {
        pipe = _pipe;
        SysThread::createThread();
    }

    virtual void dispatch()
    {

        if (inputBuffer != NULL && bufferLength > 0)
        {
            if (write(pipe, inputBuffer, bufferLength) < 0)
            {
                // We may receive EPIPE, which we only expect if a spawned
                // command finishes while we're still busy trying to pipe
                // our input.  The command may simply not need (i. e. want
                // to read) our input.  We dont' consider this an error.
                if (errno != EPIPE)
                {
                    error = errno;
                }
            }
            close(pipe);
        }
    }

    int         pipe;             // the pipe we read the data from
    const char *inputBuffer;      // the buffer of data to write
    size_t      bufferLength;     // the length of the read data
    int         error;            // and error that resulted.
};


// a thread that reads ERROR data from the command pipe
class ErrorReaderThread : public SysThread
{

public:
    inline ErrorReaderThread() : SysThread(),
        pipe(0), errorBuffer(NULL), bufferLength(0), error(0) { }
    inline ~ErrorReaderThread()
    {
        if (errorBuffer != NULL)
        {
            free(errorBuffer);
        }
        terminate();
    }

    void start(int _pipe)
    {
        pipe = _pipe;
        SysThread::createThread();
    }

    virtual void dispatch()
    {
        bufferLength = 0;
        size_t bufferAllocation = PIPE_BUF;

        // allocate the initial buffer
        if ((errorBuffer = (char *)malloc(PIPE_BUF)) == NULL)
        {
            error = errno;
            return;
        }
        ssize_t length;

        // Read until we hit EOF or an error.  Whenever the buffer gets too
        // small, we reallocate it with twice the size.
        while ((length = read(pipe, &errorBuffer[bufferLength], bufferAllocation - bufferLength)) > 0)
        {
            bufferLength += length;
            // do we need to increase our buffer allocation?
            if (bufferLength >= bufferAllocation)
            {
                bufferAllocation += PIPE_BUF;  // add another increment
                char *largerBuffer = (char *)realloc(errorBuffer, bufferAllocation);
                if (largerBuffer == NULL)
                {
                    error = errno;
                    return;
                }
                errorBuffer = largerBuffer;
            }
        }
        if (length < 0) // error reading pipe
        {
            error = errno;
            return;
        }
        close(pipe);
    }

    int    pipe;                  // the pipe we read the data from
    char  *errorBuffer;           // dynamically allocated once we start reading
    size_t bufferLength;          // the length of the read data
    int    error;                 // and error that resulted.
};


/**
 * Retrieve the globally default initial address.
 *
 * @return The string name of the default address.
 */
RexxString *SystemInterpreter::getDefaultAddressName()
{
    return GlobalNames::INITIALADDRESS;
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

    for (i=0; (i<iLength) && (name[i]!='='); i++)
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
            context->RaiseCondition("ERROR", context->String(cmd), NULL, context->WholeNumberToObject(errno));
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
    if (errCode < 0)
    {
        errCode = errno;
    }
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


/**
 * Breaks up a command string into whitespace-delimited pieces. Whitespace-
 * delimited double-quoted strings are treated as a single argument.
 *
 * @param parm_cmd   The command string.
 * @param argPtr     The "argv" array.
 *
 * @return           True if command parsed successfully, else false.
 */
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
    logical_t quoted = false;
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

        if (*pos == '"')               // start of a quoted part?
        {
            quoted = true;             // we're now within quotes
            *pos++ = '\0';             // remove and skip the quote character
        }

        argPtr[i++] = pos;                 /* Point to current argument  */
                                           /* and advance i to next      */
                                           /* element of args[]          */
        if (quoted)
        {   // look for next quote followed by whitespace or end of command
            while (!(*pos == '\0' ||
                    (*(pos - 1) == '"' && (*pos == ' ' || *pos == '\t' ))))
            {
                pos++;
            }
            if (*(pos - 1) == '"')
            {
                quoted = false;        // we're again outside of any quotes
                *(pos - 1) = '\0';     // remove the quote character
            }
        }
        else
        {   // look for next whitespace or end of command
            while (*pos != ' ' && *pos != '\t' && *pos != '\0')
            {
                pos++;
            }
        }
        *pos = '\0';                       /* Null-terminate this arg    */

    }

    /* Finally, put a null pointer in args[] to indicate the end.      */
    argPtr[i] = NULL;
    return true;
}

/**
 * Try to handle special commands like "cd" or "export" internally.  This will
 * work if we have a simple commend with no redirection, piping, or such.
 *
 * @param context    The Exit oontext.
 * @param cmd        The command string.
 * @param rc         The operating system return code.
 *
 * @return           True if command was handled, false otherwise.
 */
logical_t handleCommandInternally(RexxExitContext *context, char *cmd, RexxObjectPtr rc)
{
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
            if (!inQuotes && (strchr("<>|&;", cmd[i]) != NULL))
            {
                noDirectInvoc = true;
                break;
            }
        }
    }

    if (!noDirectInvoc)
    {
        // execute special commands in the same process
        // we currently don't handle formats like " cd" or "'cd'" internally
        if (strcmp("cd", cmd) == 0 || strncmp("cd ", cmd, 3) == 0)
        {
            return sys_process_cd(context, cmd, rc);
        }
        if (strncmp("set ", cmd, 4) == 0)
        {
            return sys_process_export(context, cmd, rc, SET_FLAG);
        }
        if (strncmp("unset ", cmd, 6) == 0)
        {
            return sys_process_export(context, cmd, rc, UNSET_FLAG);
        }
        if (strncmp("export ", cmd, 7) == 0)
        {
            return sys_process_export(context, cmd, rc, EXPORT_FLAG);
        }
    }
    return false;
}


/**
 * Raises syntax error 98.923 Address command redirection failed.
 *
 * @param context    The Exit context.
 * @param errCode    The operating system error code.
 */
RexxObjectPtr ErrorRedirection(RexxExitContext *context, int errCode)
{
    // raise 98.923 Address command redirection failed
    context->RaiseException1(Error_Execution_address_redirection_failed,
      context->CString(strerror(errno)));
    return NULLOBJECT;
}

/**
 * Raises a FAILURE condition.
 *
 * @param context    The Exit context.
 * @param command    The command name and arguments.
 */
RexxObjectPtr ErrorFailure(RexxExitContext *context, CSTRING commandString)
{
    context->RaiseCondition("FAILURE", context->String(commandString),
      NULLOBJECT, context->WholeNumberToObject(UNKNOWN_COMMAND));
    return NULLOBJECT;
}


/**
 * A redirecting test command handler.
 *
 * @param context    The Exit context.
 * @param address    The environment name.
 * @param command    The command name and arguments.
 * @param ioContext  The IO Redirector context.
 */
RexxObjectPtr RexxEntry ioCommandHandler(RexxExitContext *context, RexxStringObject address, RexxStringObject command, RexxIORedirectorContext *ioContext)
{
    pid_t pid;
    int status;
    char* argv[MAX_COMMAND_ARGS + 1];
    // If SYSSHELLPATH could ever grow longer than 128 chars
    // then this array size will need to be increased.
    char shell[128 + 1 + 256 + 1]; // SYSSHELLPATH plus "/" plus environment name

    CSTRING environment = context->CString(address);
    CSTRING commandString = context->CString(command);

    if (Utilities::strCaselessCompare("path", environment) == 0)
    {   // For the no-shell "direct" environment we need to break the command
        // string into a command name and separate arguments each (we have no
        // shell to do this for us).
        if (!scan_cmd(commandString, argv))
        {   // too many arguments
            return ErrorFailure(context, commandString);
        }
        // posix_spawnp() will fault if argv[0] is NULL.  Fix this.
        if (argv[0] == NULL)
        {
            argv[0] = (char*)"";
            argv[1] = NULL;
        }
    }
    else
    {   // Set up the full path to the requested shell.
        strcpy(shell, SYSSHELLPATH);
        if (shell[strlen(shell) - 1] != '/')
        {   // append slash if we don't have one yet
            strcat(shell, "/");
        }
        if (strlen(environment) == 0 ||
         Utilities::strCaselessCompare("command", environment) == 0 ||
         Utilities::strCaselessCompare("system", environment) == 0)
        {   // for environments "", "command" and "system" we use "sh" as shell
            strcat(shell, "sh");
        }
        else
        {   // for all other environment names we use this name in lower case
            strcat(shell, environment);
            Utilities::strlower(shell);
        }

        argv[0] = (char*)shell;
        argv[1] = (char*)"-c";
        argv[2] = (char*)commandString;
        argv[3] = NULL;
    }

    if (ioContext->IsRedirectionRequested())
    {
        InputWriterThread inputThread; // separate thread if we need to write INPUT
        ErrorReaderThread errorThread; // separate thread if we need to read ERROR
        bool need_to_write_input = false;
        bool need_to_read_output = false;
        bool need_to_read_error = false;
        int input[2], output[2], error[2];

        posix_spawn_file_actions_t action;
        posix_spawn_file_actions_init(&action);

        // Create stdin, stdout, and stderr pipes as requested.  pipe() returns
        // two file descriptors: [0] is the pipe read end, [1] is the pipe
        // write end.  The child process (i. e. the command to be run) will
        // inherit both the read and write end of all created pipes.  Of each
        // end *we* will only ever use one end, and the child the other end.
        // Therefore we and the child will close any unused ends.  In addition,
        // we also prepare any file actions that are to be be done by the child.
        // These actions are collected with posix_spawn_file_actions()

        // is stdin redirection requested?
        if (ioContext->IsInputRedirected())
        {
            if (pipe(input) != 0) // create stdin pipe
            {
                return ErrorRedirection(context, errno);
            }
            posix_spawn_file_actions_adddup2(&action, input[0], 0); // stdin reads from pipe
            posix_spawn_file_actions_addclose(&action, input[1]); // close unused write end in child
            need_to_write_input = true;
        }

        // is stdout redirection requested?
        if (ioContext->IsOutputRedirected())
        {
            if (pipe(output) != 0) // create stdout pipe
            {
                return ErrorRedirection(context, errno);
            }
            // do we want interleaved stdout and stderr redirection?
            // this is a special case .. we just redirect stderr to stdout upfront
            if (ioContext->AreOutputAndErrorSameTarget())
            {
                posix_spawn_file_actions_adddup2(&action, output[1], 2); //stderr writes to pipe
            }
            posix_spawn_file_actions_adddup2(&action, output[1], 1); //stdout writes to pipe
            posix_spawn_file_actions_addclose(&action, output[0]); // close unused read end in child
            need_to_read_output = true;
        }

        // now stderr redirection
        // if both stdout and stderr are to be redirected to the same object, then
        // everything was already set up in the previous stdout redirection step
        if (ioContext->IsErrorRedirected() && !ioContext->AreOutputAndErrorSameTarget())
        {
            if (pipe(error) != 0) // create stderr pipe
            {
                return ErrorRedirection(context, errno);
            }
            posix_spawn_file_actions_adddup2(&action, error[1], 2); // stderr writes to pipe
            posix_spawn_file_actions_addclose(&action, error[0]); // close unused read end in child
            need_to_read_error = true;
        }

        // ok, everything is set up accordingly, let's fork the redirected command
        if (posix_spawnp(&pid, argv[0], &action, NULL, argv, getEnvironment()) != 0)
            {
                return ErrorFailure(context, commandString);
            }

        // Close all unneeded read- and write ends
        if (need_to_write_input)
        {
            close(input[0]); // we close our unused stdout pipe write end
        }
        if (need_to_read_output)
        {
            close(output[1]); // we close our unused stdout pipe write end
        }
        if (need_to_read_error)
        {
            close(error[1]); // we close our unused stderr pipe write end
        }

        if (need_to_write_input)
        {
            ioContext->ReadInputBuffer(&inputThread.inputBuffer, &inputThread.bufferLength);
            // we start a separate thread to write INPUT data to the input pipe
            inputThread.start(input[1]);
        }
        if (need_to_read_error)
        {
            // we start a separate thread to read ERROR data from the error pipe
            errorThread.start(error[0]);
        }

        // do we need to read OUTPUT from the stdout pipe?
        // we run this in the main thread instead of starting another new thread
        // if we have stdout and sterr interleaved, we'll read both of them here
        if (need_to_read_output)
        {
            // According to POSIX.1 PIPE_BUF is at least 512 bytes; on Linux
            // it's 4096 bytes.
            char outputBuffer[PIPE_BUF];
            ssize_t outputLength;

            while ((outputLength = read(output[0], &outputBuffer, PIPE_BUF)) > 0)
            {
                ioContext->WriteOutputBuffer(outputBuffer, outputLength);
            }
            if (outputLength != 0) // 0 is EOF, success
            {
                return ErrorRedirection(context, errno);
            }
            close(output[0]);
        }

        // did we start an ERROR thrad?
        if (need_to_read_error)
        {
            // wait for the ERROR thread to finish
            errorThread.waitForTermination();
            if (errorThread.bufferLength > 0)
            {   // return what the ERROR thread read from its pipe
                ioContext->WriteErrorBuffer(errorThread.errorBuffer, errorThread.bufferLength);
            }
            // the ERROR thread may have encountered an error .. raise it now
            if (errorThread.error != 0)
            {
                return ErrorRedirection(context, errorThread.error);
            }
        }

        // if we started an INPUT thread, wait until it finishes
        if (need_to_write_input)
        {
            inputThread.waitForTermination();
            // the INPUT thread may have encountered an error .. raise it now
            if (inputThread.error != 0)
            {
                return ErrorRedirection(context, inputThread.error);
            }
        }

        posix_spawn_file_actions_destroy(&action);
    }
    else // no redirection requested
    {
        // try to handle special commands like 'cd' or 'export' internally
        RexxObjectPtr rc = NULLOBJECT;

        if (handleCommandInternally(context, (char*)commandString, rc))
        {   // the command was handled in this thread
            return rc;
        }
        else
        {   // to run the command we spawn another thread
            if (posix_spawnp(&pid, argv[0], NULL, NULL, argv, getEnvironment()) != 0)
            {
                return ErrorFailure(context, commandString);
            }
        }
    }

    // wait for our child, the forked command
    waitpid(pid, &status, 0);
    int rc;
    if (WIFEXITED(status)) // child process ended normal
    {
        rc = WEXITSTATUS(status); // set return code
    }
    else
    {   // child process died
        rc = -(WTERMSIG(status));
        if (rc == 1) // process was stopped
        {
            rc = -1;
        }
    }

    // unknown command code?
    // a FAILURE will be raised for any command returning 127
    if (rc == UNKNOWN_COMMAND)
    {
        return ErrorFailure(context, commandString);
    }
    else if (rc != 0)
    {
        // for any non-zero return code we raise an ERROR condition
        context->RaiseCondition("ERROR", context->String(commandString),
          NULL, context->WholeNumberToObject(rc));
        return NULLOBJECT;
    }
    return context->False(); // zero return code
}


/**
 * Register the standard system command handlers.
 *
 * @param instance The created instance.
 */
void SysInterpreterInstance::registerCommandHandlers(InterpreterInstance *_instance)
{
    // The default command handler on Unix is "SH"
    // It comes with three aliases named "", "COMMAND", and "SYSTEM"
    // "SYSTEM" is compatible with Regina
    _instance->addCommandHandler("SH",      (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("",        (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("COMMAND", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("SYSTEM",  (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);

    // The command handlers for shells other than "sh" will only work
    // if a shell with this named is installed on the system
    _instance->addCommandHandler("KSH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("CSH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("BSH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("BASH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("TCSH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
    _instance->addCommandHandler("ZSH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);

    // This is a no-shell environment that searches PATH.  It is named "PATH"
    // which happens to be compatible with Regina.
    _instance->addCommandHandler("PATH", (REXXPFN)ioCommandHandler, HandlerType::REDIRECTING);
}


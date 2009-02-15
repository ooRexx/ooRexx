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
/*  wincmd.c - C methods for handling calls to system exits and subcommand    */
/*             handlers.                                                      */
/*                                                                            */
/*  C methods:                                                                */
/*    SysCommand     - Method to invoke a subcommand handler                  */
/*                                                                            */
/*  Internal routines:                                                        */
/*    sys_command - Run a command through system command processor.           */
/******************************************************************************/
#include <string.h>                    /* Get strcpy, strcat, etc.       */

#include <process.h>
#include <stdlib.h>

#include "RexxCore.h"                    /* global REXX declarations     */
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "SysInterpreterInstance.hpp"

#define CMDBUFSIZE32S 260              /* Max size of executable cmd     */
#define CMDBUFSIZENT 8092              /* Max size of executable cmd     */
#define CMDDEFNAME32S "COMMAND.COM"    /* Default Win 95   cmd handler   */
#define CMDDEFNAMENT "CMD.EXE"         /* Default Win NT   cmd handler   */
#define UNKNOWN_COMMAND 1              /* unknown command return code    */
#include "direct.h"

#define SYSENV "CMD"                   /* Default windows cmd environment*/
#define COMSPEC "COMSPEC"              /*      cmd handler env name      */

#define SHOWWINDOWFLAGS SW_HIDE        // determines visibility of cmd
                                       // window SHOW, HIDE etc...
                                       // function prototypes
/**
 * Retrieve the globally default initial address.
 *
 * @return The string name of the default address.
 */
RexxString *SystemInterpreter::getDefaultAddressName()
{
    return OREF_INITIALADDRESS;
}


/* Handle "SET XX=YYY" command in same process */
bool sys_process_set(RexxExitContext *context, const char *command, const char * cmd, RexxObjectPtr &rc)
{
    rc = NULLOBJECT;
    const char * eqsign;
    const char * st;
    char name[256];
    char value[4096];
    eqsign = strchr(cmd, '=');
    if (!eqsign)
    {
        return false;
    }

    st = &cmd[4];
    while ((*st) && (*st == ' '))
    {
        st++;
    }
    if (st == eqsign)
    {
        return false;
    }
    strncpy(name, st, eqsign-st);
    name[eqsign-st]='\0';

    if (ExpandEnvironmentStrings(eqsign+1, value, 4095) && SetEnvironmentVariable(name,value))
    {
        rc = context->False();     // just return a zero
    }
    else
    {
        context->RaiseCondition("ERROR", context->String(command), NULLOBJECT, context->WholeNumberToObject(GetLastError()));
    }
    return true;
}


/* Returns a copy of s without quotes */
char *unquote(const char *s)
{
    char *unquoted = (char*)malloc(sizeof(char) * strlen(s) + 1);

    if (unquoted != NULL)
    {
        char *p = unquoted;
        while ( (*p = *s++) != 0 )
        {
            if (*p != '"')
            {
                p++;
            }
        }
        *p = '\0';
    }
    return unquoted;
}


/* Handle "CD XXX" command in same process */
bool sys_process_cd(RexxExitContext *context, const char *command, const char * cmd, RexxObjectPtr &res)
{
    const char * st;
    int rc;
    res = NULLOBJECT;

    st = &cmd[3];
    while ((*st) && (*st == ' '))
    {
        st++;
    }
    if (!*st)
    {
        return false;
    }

    if ((strlen(st) == 2) && (st[1] == ':'))
    {
        rc = _chdrive(toupper( *st ) - 'A' + 1);
    }
    else
    {
        char *unquoted = unquote(st);
        if (unquoted == NULL)
        {
            return false;
        }
        rc = _chdir(unquoted);
        free(unquoted);
    }
    if (rc != 0)
    {
        context->RaiseCondition("ERROR", context->String(command), NULLOBJECT, context->WholeNumberToObject(GetLastError()));
    }
    else
    {
        res = context->False();
    }

    return true;
}


/*-----------------------------------------------------------------------------
 | Name:       sysCommandNT                                                   |
 |                                                                            |
 | Arguments:  cmd - Command to be executed                                   |
 |                                                                            |
 | Returned:   rc - Return Code                                               |
 |                  Note: if CreateProcess fails return GetLastError code     |
 |                  else return rc from executed command                      |
 |                                                                            |
 | Notes:      Handles processing of a system command on a Windows NT system  |
 |                                                                      |
  ----------------------------------------------------------------------------*/
bool sysCommandNT(RexxExitContext *context, const char *command, const char *cmdstring_ptr, bool direct, RexxObjectPtr &result)
{
    DWORD rc;
    STARTUPINFO siStartInfo;                  // process startup info
    PROCESS_INFORMATION piProcInfo;           // returned process info
    char ctitle[256];
    DWORD creationFlags;
    bool titleChanged;

    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    ZeroMemory(&piProcInfo, sizeof(piProcInfo));
    /****************************************************************************/
    /* Invoke the system command handler to execute the command                 */
    /****************************************************************************/
    siStartInfo.cb = sizeof(siStartInfo);

    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    titleChanged = GetConsoleTitle(ctitle, 255) != 0;
    siStartInfo.lpTitle = (LPSTR)cmdstring_ptr;
    creationFlags = GetPriorityClass(GetCurrentProcess()) | CREATE_NEW_PROCESS_GROUP;
    if (!siStartInfo.hStdInput && !siStartInfo.hStdOutput && !titleChanged)  /* is REXXHIDE running without console */
    {
        if (!direct)
        {
            siStartInfo.wShowWindow = SHOWWINDOWFLAGS;
            siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
        }
        else if (SystemInterpreter::explicitConsole)
        {
            creationFlags |= CREATE_NEW_CONSOLE; /* new console if CMD ord COMMAND was specified */
        }
    }
    else              /* just use standard handles if we are running in a console */
    {
        if (direct)
        {
            siStartInfo.dwFlags = STARTF_USESTDHANDLES; /* no SW_HIDE for direct commands */
        }
        else
        {
            siStartInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        }
        siStartInfo.wShowWindow = SHOWWINDOWFLAGS;
    }

    if ( CreateProcess ( NULL,           // address of module name
                         (LPSTR)cmdstring_ptr,// address of command line
                         NULL,                // address of process security attrs
                         NULL,                // address of thread security attrs
                         true,                // new process inherits handles?
                         // creation flags
                         creationFlags,
                         NULL,                // address of new environment block
                         NULL,                // address of current directory name
                         &siStartInfo,        // address of STARTUPINFO
                         &piProcInfo))        // address of PROCESS_INFORMATION
    // good rc from create now
    // Wait for process to end and
    {
        if (titleChanged)
        {
            SetConsoleTitle(siStartInfo.lpTitle);
        }
        SystemInterpreter::exceptionHostProcess = piProcInfo.hProcess;
        SystemInterpreter::exceptionHostProcessId = piProcInfo.dwProcessId;

        if (WAIT_FAILED != WaitForSingleObject ( piProcInfo.hProcess, INFINITE ) )
        {
            // complete,ok?, get terminate rc
            GetExitCodeProcess ( piProcInfo.hProcess, &rc );
        }
        else
        {
            rc = GetLastError ();         // bad termination? get error code
            context->RaiseCondition("FAILURE", context->String(command), NULLOBJECT, context->WholeNumberToObject(rc));
            result = NULLOBJECT;
            return true;
        }
        /* the new process must be detached so it will be discarded automatically after execution */
        /* The thread must be closed first */
        if (titleChanged)
        {
            SetConsoleTitle(ctitle);
        }
        CloseHandle(piProcInfo.hThread);
        CloseHandle(piProcInfo.hProcess);
    }
    else
    {
        // return this as a failure for now...we might try this again later
        return false;
    }

    SystemInterpreter::exceptionHostProcess = NULL;
    SystemInterpreter::exceptionHostProcessId = 0;

    if (rc != 0)
    {
        context->RaiseCondition("ERROR", context->String(command), NULLOBJECT, context->Int32ToObject(rc));
        result = NULLOBJECT;
        return true;
    }
    // this is a zero return
    result = context->False();
    return true;
}



/******************************************************************************/
/* Name:       sys_command                                                    */
/*                                                                            */
/* Arguments:  cmd - Command to be executed                                   */
/*                                                                            */
/* Returned:   rc - Return Code                                               */
/*                  Note: if non-zero rc from DosExecPgm return DosExecPgm rc */
/*                  else if non-zero termination code from system return code */
/*                  else return rc from executed command                      */
/*                                                                            */
/* Notes:      Handles processing of a system command.  Finds location of     */
/*             system command handler using the COMSPEC environment variable  */
/*             and invokes the system specific routine which invokes the      */
/*             command handler with the command to be executed                */
/*                                                                            */
/******************************************************************************/
RexxObjectPtr RexxEntry systemCommandHandler(RexxExitContext *context, RexxStringObject address, RexxStringObject command)
{
    // address the command information
    const char *cmd = context->StringData(command);
    const char *cl_opt = " /c ";         /* "/c" opt for sys cmd handler      */
    const char *interncmd;
    /* remove quiet sign              */
    if (cmd[0] == '@')
    {
        interncmd = cmd + 1;
    }
    else
    {
        interncmd = cmd;
    }

    /* check for redirection symbols, ignore them when enclosed in double quotes */
    bool noDirectInvoc = false;
    bool inQuotes = false;
    size_t i;
    for (i = 0; i<strlen(interncmd); i++)
    {
        if (interncmd[i] == '"')
        {
            inQuotes = !inQuotes;
        }
        else
        {
            /* if we're in the unquoted part and the current character is one of */
            /* the redirection characters or the & for multiple commands then we */
            /* will no longer try to invoke the command directly                 */
            if (!inQuotes && (strchr("<>|&", interncmd[i]) != NULL))
            {
                noDirectInvoc = true;
                break;
            }
        }
    }

    i = 0; /* reset to zero for next usage ! */

    // scan for the first whitespace character
    size_t j = 0;
    while (interncmd[j] == ' ')
    {
        j++;
    }

    if (!noDirectInvoc)
    {
        char tmp[8];
        strncpy(tmp, &interncmd[j], 4);
        tmp[4] = '\0';
        RexxObjectPtr  rc;                      /* Return code                       */
        if (!stricmp("set ",tmp))
        {
            if (sys_process_set(context, cmd, &interncmd[j], rc))
            {
                return rc;
            }
        }
        else
        {
            strncpy(tmp, &interncmd[j], 3);
            tmp[3] = '\0';
            if (!stricmp("cd ",tmp))
            {
                if (sys_process_cd(context, cmd, &interncmd[j], rc))
                {
                    return rc;
                }
            }
            else
            {          /* check for drive letter change */
                if ((tmp[1] == ':') && ((tmp[2] == ' ') || (!tmp[2])))
                {
                    int code = _chdrive(toupper( tmp[0] ) - 'A' + 1);
                    if (code != 0)
                    {
                        context->RaiseCondition("ERROR", command, NULLOBJECT, context->WholeNumberToObject(code));
                        return NULLOBJECT;
                    }
                    else
                    {
                        return context->False();
                    }
                }
                else
                {
                    /* check if a START command is specified, if so don not start it directly */
                    strncpy(tmp, &interncmd[j], 6);
                    tmp[6] = '\0';
                    noDirectInvoc = stricmp("start ",tmp) == 0;
                }
            }
        }
    }

    const char *sys_cmd_handler;         /* Pointer to system cmd handler     */
    // location of command interpreter
    if (NULL == (sys_cmd_handler = getenv(COMSPEC)))
    {
        // CMD.EXE on NT Command.com on 32s
        sys_cmd_handler = CMDDEFNAMENT;
    }
    /* Get len of handler                  */
    size_t length_cmd_handler = strlen(sys_cmd_handler);

    char  cmdstring[CMDBUFSIZENT];             /* Largest cmd we can give system    */
    char *cmdstring_ptr = cmdstring;           /* Set pointer to cmd buffer           */
    /****************************************************************************/
    /* Compute maximum size of command we can fit into string being passed to   */
    /* CreateProcess                                                            */
    /****************************************************************************/
    // string length is system specific
    size_t maxStringLength = CMDBUFSIZENT;
    size_t max_cmd_length = maxStringLength -     /* Maximum size of string         */
                     (length_cmd_handler + 1) -   /* Minus length of sys cmd handler*/
                     strlen(cl_opt) -             /* Minus length of cmd line option*/
                     1;                           /* Minus length of terminating \0 */

    SystemInterpreter::exceptionConsole = false;
    SystemInterpreter::explicitConsole = false;

    /* check whether or not program to invoke is cmd or command */
    _strupr(strcpy(cmdstring_ptr, &interncmd[j]));
    bool searchFile = strstr(cmdstring_ptr, "CMD") != NULL;

    if (searchFile)
    {
        if (cmdstring_ptr[0] == '\"')
        {
            cmdstring_ptr++;
            while (cmdstring_ptr[i] && (cmdstring_ptr[i] != '\"'))
            {
                i++;
            }
            cmdstring_ptr[i]='\0';
        }
        else if (cmdstring_ptr[0] == '\'')
        {
            cmdstring_ptr++;
            while (cmdstring_ptr[i] && (cmdstring_ptr[i] != '\''))
            {
                i++;
            }
            cmdstring_ptr[i]='\0';
        }
        else
        {
            while (cmdstring_ptr[i] && (cmdstring_ptr[i] != ' '))
            {
                i++;
            }
            cmdstring_ptr[i]='\0';
        }

        LPSTR filepart;
        bool fileFound = SearchPath(NULL, cmdstring_ptr, ".EXE", CMDBUFSIZENT-1, cmdstring, &filepart) != 0;
        cmdstring_ptr = cmdstring;           /* Set pointer again to cmd buffer (might have been increased) */

        if (fileFound && !stricmp(sys_cmd_handler, cmdstring_ptr))
        {
            SystemInterpreter::exceptionConsole = true;
            SystemInterpreter::explicitConsole = true;
        }
    }

    /* first check whether we can run command directly as a program (no file redirection when not cmd or command) */
    if (SystemInterpreter::explicitConsole || !noDirectInvoc)
    {
        // try to invoke this directly.  A true failure allows us to fall through (for now)
        RexxObjectPtr result = NULLOBJECT;
        if (sysCommandNT(context, cmd, &interncmd[j], true, result))
        {
            return result;
        }
    }
    /* no we couldn't, so pass command to cmd.exe or command.com */

    strcpy(cmdstring_ptr,sys_cmd_handler);    /* Put in system cmd handler      */

    /* the following lines checks whether or not a /k option is specified */
    /* if so the /c option must not be concatenated */
    /* /k can only be specified as the first argument to keep the command handler active */
    if (!( (strlen(interncmd) > j+1) && (interncmd[j] == '/')
           && ((interncmd[j+1] == 'k') || (interncmd[j+1] == 'K'))
           && ((interncmd[j+2] == ' ') || (interncmd[j+2] == '\0')) ))
    {
        strcat(cmdstring_ptr,cl_opt);
    }
    else
    {
        SystemInterpreter::exceptionConsole = true;
        strcat(cmdstring_ptr," ");
    }

    strcat(cmdstring_ptr,interncmd);        /* And cmd to be executed         */

    /****************************************************************************/
    /* Invoke the system command handler to execute the command                 */
    // On NT, this will be the actual rc from the executing program
    // on Win32s this will be 0 if Winexec spawned the program, 1-31
    // reflect errors from WinExec; where WinExec rc=0 is mapped to 1
    /****************************************************************************/
    // Call system specific routine
    RexxObjectPtr rc = NULLOBJECT;

    if (!sysCommandNT(context, cmd, cmdstring_ptr, false, rc))
    {
        // bad termination? get error code
        context->RaiseCondition("FAILURE", context->String(cmd), NULLOBJECT, context->WholeNumberToObject(GetLastError()));
        return NULLOBJECT;
    }
    SystemInterpreter::exceptionConsole = false;
    return rc;
}                                      // SystemCommand


/**
 * Register the standard system command handlers.
 *
 * @param instance The created instance.
 */
void SysInterpreterInstance::registerCommandHandlers(InterpreterInstance *instance)
{
    // Windows only has the single command environment, we also register this
    // under "" for the default handler
    instance->addCommandHandler("CMD", (REXXPFN)systemCommandHandler);
    instance->addCommandHandler("COMMAND", (REXXPFN)systemCommandHandler);
    instance->addCommandHandler("", (REXXPFN)systemCommandHandler);
}

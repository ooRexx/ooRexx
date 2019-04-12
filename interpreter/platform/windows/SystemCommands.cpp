/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/*  wincmd.c - C methods for handling calls to system exits and subcommand    */
/*             handlers.                                                      */
/*                                                                            */
/*  C methods:                                                                */
/*    SysCommand     - Method to invoke a subcommand handler                  */
/*                                                                            */
/*  Internal routines:                                                        */
/*    sys_command - Run a command through system command processor.           */
/******************************************************************************/
#include <process.h>

#include "RexxCore.h"                    /* global REXX declarations     */
#include "StringClass.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "SysInterpreterInstance.hpp"
#include "CommandHandler.hpp"
#include "SysThread.hpp"

#include <ctype.h>

#define CMDBUFSIZE 8092                // maximum commandline length
#define CMDDEFNAME "CMD.EXE"           // default Windows cmomand handler
#define UNKNOWN_COMMAND 1              /* unknown command return code    */
#include "direct.h"

#define COMSPEC "COMSPEC"              /*      cmd handler env name      */

#define SHOWWINDOWFLAGS SW_HIDE        // determines visibility of cmd
                                       // window SHOW, HIDE etc...
                                       // function prototypes

const int READ_SIZE = 4096;         // size of chunks read from pipes

class InputWriterThread : public SysThread
{

public:
    inline InputWriterThread() : SysThread(),
        pipe(0), inputBuffer(NULL), bufferLength(0), error(0), wasShutdown(false) { }
    inline ~InputWriterThread() { terminate(); }

    void start(HANDLE _pipe)
    {
        pipe = _pipe;
        SysThread::createThread();
    }
    virtual void dispatch()
    {
        DWORD bytesWritten = 0;
        if (!WriteFile(pipe, inputBuffer, (DWORD)bufferLength, &bytesWritten, NULL))
        {
            // if we've had a shutdown request after the process completes, no longer
            // record errors
            if (!wasShutdown)
            {
                error = GetLastError();
            }
        }
        CloseHandle(pipe);
    }

    void shutdown()
    {
        wasShutdown = true;
    }



    bool        wasShutdown;      // this is a shutdown request
    const char *inputBuffer;      // the buffer of data to write
    size_t      bufferLength;     // the length of the buffer
    HANDLE      pipe;             // the pipe we write the data to
    int         error;            // and error that resulted.
};


// a thread that reads ERROR or OUTPUT data from the command pipe
class ReaderThread : public SysThread
{

public:
    inline ReaderThread() : SysThread(),
        pipe(0), pipeBuffer(NULL), dataLength(0), error(0) { }
    inline ~ReaderThread()
    {
        if (pipeBuffer != NULL)
        {
            free(pipeBuffer);
            pipeBuffer = NULL;
        }
        terminate();
    }

    void start(HANDLE _pipe)
    {
        pipe = _pipe;
        SysThread::createThread();
    }

    virtual void dispatch()
    {
        DWORD cntRead;
        size_t bufferSize = READ_SIZE;       // current size of our buffer
        pipeBuffer = (char *)malloc(READ_SIZE);  // allocate an initial buffer
        if (pipeBuffer == NULL)
        {
            // use the Windows error code
            error = ERROR_NOT_ENOUGH_MEMORY;
            return;
        }
        for (;;)
        {
            if (!ReadFile(pipe, pipeBuffer + dataLength, (DWORD)(bufferSize - dataLength), &cntRead, NULL) || cntRead == 0)
            {
                break;
            }

            dataLength += cntRead;
            // have we hit the end of the buffer?
            if (dataLength >= bufferSize)
            {
                // increase the buffer by another increment
                bufferSize += READ_SIZE;
                char *largerBuffer = (char *)realloc(pipeBuffer, bufferSize);
                if (largerBuffer == NULL)
                {
                    // use the Windows error code
                    error = ERROR_NOT_ENOUGH_MEMORY;
                    return;
                }
                pipeBuffer = largerBuffer;
            }
        }
        CloseHandle(pipe);
    }

    HANDLE pipe;                  // the pipe we read the data from
    char  *pipeBuffer;            // initally errorBuffer = firstBuffer
    size_t dataLength;            // the length of data we've read
    int    error;                 // and error that resulted.
};


/**
 * Raises syntax error 98.923 Address command redirection failed.
 *
 * @param context    The Exit context.
 * @param errCode    The operating system error code.
 */
void ErrorRedirection(RexxExitContext *context, int errCode)
{
    // raise 98.923 Address command redirection failed
    context->RaiseException1(Error_Execution_address_redirection_failed,
      context->Int32ToObject(errCode));
}



/**
 * Retrieve the globally default initial address.
 *
 * @return The string name of the default address.
 */
RexxString *SystemInterpreter::getDefaultAddressName()
{
    return GlobalNames::INITIALADDRESS;
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
    int rc, error;
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
        int drive = toupper( *st ) - 'A' + 1;
        if (drive < 1 || drive > 26)
        {
          // an out-of-range drive will make _chdrive() MSVC run-time library fail
          rc = -1;
          error = 15; // The system cannot find the drive specified.
        }
        else
        {
          rc = _chdrive(drive);
          error = GetLastError();
        }

    }
    else
    {
        AutoFree unquoted = unquote(st);
        if (unquoted == NULL)
        {
            return false;
        }
        rc = _chdir(unquoted);
        error = GetLastError();
    }
    if (rc != 0)
    {
        context->RaiseCondition("ERROR", context->String(command), NULLOBJECT, context->WholeNumberToObject(error));
    }
    else
    {
        res = context->False();
    }

    return true;
}

void makePipe(PHANDLE readH, PHANDLE writeH,    // pointers to the handles
              SECURITY_ATTRIBUTES* sAttr,       // pointer to the security attr.
              CSTRING name,                     // name of the pipe
              RexxExitContext* context)         // context for errors
{
    BOOL pipeOK;
    HANDLE noInheritH;

    pipeOK = CreatePipe(readH, writeH, sAttr, 0);
    if (!pipeOK)
    {
        ErrorRedirection(context, GetLastError());
        return;
    }

    // insure the "unconnected" end of the pipe is not inherited
    if (strcmp(name, "Input") == 0)
    {
        noInheritH = *writeH;
    }
    else
    {
        noInheritH = *readH;
    }

    pipeOK = SetHandleInformation(noInheritH, HANDLE_FLAG_INHERIT, 0);
    if (!pipeOK)
    {
        ErrorRedirection(context, GetLastError());
        return;
    }
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
bool sysCommandNT(RexxExitContext *context,
                  const char *command,
                  const char *cmdstring_ptr,
                  bool direct,
                  RexxObjectPtr &result,
                  RexxIORedirectorContext *ioContext)
{
    DWORD rc;
    STARTUPINFO siStartInfo;                  // process startup info
    PROCESS_INFORMATION piProcInfo;           // returned process info
    char ctitle[256];
    DWORD creationFlags;
    BOOL titleChanged;

    logical_t redirIn = ioContext->IsInputRedirected();      // Input redirected?
    logical_t redirOut = ioContext->IsOutputRedirected();    // Output redirected?
    logical_t redirErr = ioContext->IsErrorRedirected();     // Error redirected?
    logical_t combo = ioContext->AreOutputAndErrorSameTarget();  // Err/Out combined?

    InputWriterThread inputThread;                           // used if need to write input
    ReaderThread outputThread;                               // separate thread if we need to read OUTPUT
    ReaderThread errorThread;                                // separate thread if we need to read ERROR

    BOOL readOK = false;
    // handles for pipes if needed
    HANDLE inPipeW, outPipeR, errPipeR;

    // security attributes for pipes
    SECURITY_ATTRIBUTES secAttr;
    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    // Set the bInheritHandle flag so pipe handles are inherited.
    secAttr.bInheritHandle = TRUE;
    secAttr.lpSecurityDescriptor = NULL;

    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    ZeroMemory(&piProcInfo, sizeof(piProcInfo));
    /****************************************************************************/
    /* Invoke the system command handler to execute the command                 */
    /****************************************************************************/
    siStartInfo.cb = sizeof(siStartInfo);
    // change the following if redirecting I/O
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    titleChanged = GetConsoleTitle(ctitle, 255) != 0;
    siStartInfo.lpTitle = (LPSTR)cmdstring_ptr;
    // if creation flag CREATE_NEW_PROCESS_GROUP is specified, CreateProcess() will
    // implicitly call SetConsoleCtrlHandler(NULL,TRUE) which causes the created
    // process to ignore Ctrl+C signals, as reported in [bugs:#1328]
    // (for details see MSDN CreateProcess() and SetConsoleCtrlHandler())
    creationFlags = GetPriorityClass(GetCurrentProcess()); // | CREATE_NEW_PROCESS_GROUP;
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

    if (ioContext->IsRedirectionRequested()) {
        // create the pipes needed to implement the redirection
        if (redirIn)
        {
            makePipe(&siStartInfo.hStdInput, &inPipeW, &secAttr, "Input", context);
        }

        if (redirOut)
        {
            makePipe(&outPipeR, &siStartInfo.hStdOutput, &secAttr, "Output", context);
        }

        if (redirErr)
        {
            if (combo)
            {                // send output and error to the same stream
                siStartInfo.hStdError = siStartInfo.hStdOutput;
                redirErr = false;
            }
            else
            {
                makePipe(&errPipeR, &siStartInfo.hStdError, &secAttr, "Error", context);
            }
        }
    }

    if (CreateProcess(NULL,           // address of module name
                      (LPSTR)cmdstring_ptr,// address of command line
                      NULL,                // address of process security attrs
                      NULL,                // address of thread security attrs
                      true,                // new process inherits handles?
                      creationFlags,       // creation flags
                      NULL,                // address of new environment block
                      NULL,                // address of current directory name
                      &siStartInfo,        // address of STARTUPINFO
                      &piProcInfo))        // address of PROCESS_INFORMATION
    {
        // CreateProcess succeeded, change the title if needed
        if (titleChanged)
        {
            SetConsoleTitle(siStartInfo.lpTitle);
        }

        // if Input is redirected, write the input data to the input pipe
        if (redirIn)
        {
            ioContext->ReadInputBuffer(&inputThread.inputBuffer, &inputThread.bufferLength);
            // only start the thread if we have real data
            if (inputThread.inputBuffer != NULL)
            {
                // we need the pipe handle to do this
                inputThread.start(inPipeW);
            }
        }

        if (redirErr)
        {
            // we start a separate thread to read ERROR data from the error pipe
            errorThread.start(errPipeR);
        }


        // if Output is redirected, write the data from the output pipe
        if (redirOut)
        {
            // we start a separate thread to read OUTPUT data from the stdout pipe
            outputThread.start(outPipeR);
        }


        SystemInterpreter::exceptionHostProcess = piProcInfo.hProcess;
        SystemInterpreter::exceptionHostProcessId = piProcInfo.dwProcessId;

        if (WAIT_FAILED != WaitForSingleObject ( piProcInfo.hProcess, INFINITE ) )
        {
            // Completed ok, get termination rc
            GetExitCodeProcess(piProcInfo.hProcess, &rc);
            // do we have input cleanup to perform?
            if (redirIn)
            {
                // the process has returned, but the input thread may be stuck
                // on a write for data that was never used. We need to close the
                // pipe to force it to complete
                inputThread.shutdown();
                // close the process end of the pipe
                CloseHandle(siStartInfo.hStdInput);
                // wait for everything to complete
                inputThread.waitForTermination();
                // the INPUT thread may have encountered an error .. raise it now
                if (inputThread.error != 0)
                {
                    ErrorRedirection(context, inputThread.error);
                    return false;
                }
            }

            // did we start an ERROR thread?
            if (redirOut)
            {
                CloseHandle(siStartInfo.hStdOutput);  // close the handle so readFile will stop
                // wait for the ERROR thread to finish
                outputThread.waitForTermination();
                if (outputThread.dataLength > 0)
                {   // return what the ERROR thread read from its pipe
                    ioContext->WriteOutputBuffer(outputThread.pipeBuffer, outputThread.dataLength);
                }
                // the OUTPUT thread may have encountered an error .. raise it now
                if (outputThread.error != 0)
                {
                    ErrorRedirection(context, errorThread.error);
                    return false;
                }
            }

            // did we start an ERROR thread?
            if (redirErr)
            {
                CloseHandle(siStartInfo.hStdError);  // close the handle so readFile will stop
                // wait for the ERROR thread to finish
                errorThread.waitForTermination();
                if (errorThread.dataLength > 0)
                {   // return what the ERROR thread read from its pipe
                    ioContext->WriteErrorBuffer(errorThread.pipeBuffer, errorThread.dataLength);
                }
                // the ERROR thread may have encountered an error .. raise it now
                if (errorThread.error != 0)
                {
                    ErrorRedirection(context, errorThread.error);
                    return false;
                }
            }
        }
        else
        {
            rc = GetLastError();    // Bad termination, get error code
            context->RaiseCondition("FAILURE", context->String(command), NULLOBJECT, context->WholeNumberToObject(rc));
            result = NULLOBJECT;
            return true;
        }

        // The new process must be detached so it will be discarded
        // automatically after execution.  The thread must be closed first
        if (titleChanged)
        {
            SetConsoleTitle(ctitle);
        }
        CloseHandle(piProcInfo.hThread);
        CloseHandle(piProcInfo.hProcess);
    }
    else
    {
        // return this as a failure for now ... the caller might try this again
        // later
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
RexxObjectPtr RexxEntry systemCommandHandler(RexxExitContext *context,
                                             RexxStringObject address,
                                             RexxStringObject command,
                                             RexxIORedirectorContext *ioContext)
{
    // address the command information
    const char *cmd = context->StringData(command);
    const char *cl_opt = " /c "; // The "/c" opt for system commandd handler
    const char *interncmd;
    RexxObjectPtr result = NULLOBJECT;

    // is this directed to the no-shell "path" environment?
    if (stricmp(context->StringData(address), "path") == 0)
    {
        if (!sysCommandNT(context, cmd, cmd, true, result, ioContext))
        {
            // rc from GetLastError() would be 2, ERROR_FILE_NOT_FOUND
            // but for consistency with cmd.exe rc for an unknown command
            // we always set this to 1
            context->RaiseCondition("FAILURE", context->String(cmd), NULLOBJECT, context->WholeNumberToObject(1));
            return NULLOBJECT;
        }
        return result;
    }

    // Remove the "quiet sign" if present
    if (cmd[0] == '@')
    {
        interncmd = cmd + 1;
    }
    else
    {
        interncmd = cmd;
    }

    /* Check for redirection symbols, ignore them when enclosed in double
     * quotes.  If there are more than 2 double qoutes, cmd.exe strips off the
     * first and last.  To preserve what the user sent to us, we count the
     * double quotes, and, if more than two, add a double quote to the front and
     * end of the string.
     */
    size_t quoteCount    = 0;
    bool   noDirectInvoc = false;
    bool   inQuotes      = false;
    size_t i;

    for (i = 0; i < strlen(interncmd); i++)
    {
        if (interncmd[i] == '"')
        {
            inQuotes = !inQuotes;
            quoteCount++;
        }
        else
        {
            /* if we're in the unquoted part and the current character is one of */
            /* the redirection characters or the & for multiple commands then we */
            /* will no longer try to invoke the command directly                 */
            if (!inQuotes && (strchr("<>|&", interncmd[i]) != NULL))
            {
                noDirectInvoc = true;
                if ( quoteCount > 2 )
                {
                    break;
                }
            }
        }
    }

    i = 0; // reset to zero for next usage

    // scan for the first non-whitespace character
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

        if (!stricmp("set ",tmp))
        {
            if (sys_process_set(context, cmd, &interncmd[j], result))
            {
                return result;
            }
        }
        else
        {
            strncpy(tmp, &interncmd[j], 3);
            tmp[3] = '\0';
            if (!stricmp("cd ",tmp))
            {
                if (sys_process_cd(context, cmd, &interncmd[j], result))
                {
                    return result;
                }
            }
            else
            {   // Check if the command is to change drive
                if ((tmp[1] == ':') && ((tmp[2] == ' ') || (!tmp[2])))
                {
                    int rc, error;

                    int drive = toupper( tmp[0] ) - 'A' + 1;
                    if (drive < 1 || drive > 26)
                    {
                      // an out-of-range drive will make _chdrive() MSVC run-time library fail
                      rc = -1;
                      error = 15; // The system cannot find the drive specified.
                    }
                    else
                    {
                      rc = _chdrive(drive);
                      error = GetLastError();
                    }
                    if (rc != 0)
                    {
                        context->RaiseCondition("ERROR", command, NULLOBJECT, context->WholeNumberToObject(error));
                        return NULLOBJECT;
                    }
                    else
                    {
                        // 0 result.
                        return context->False();
                    }
                }
                else
                {
                    // Check if a START command is specified, if so do not
                    // invoke the command directly.
                    strncpy(tmp, &interncmd[j], 6);
                    tmp[6] = '\0';
                    noDirectInvoc = stricmp("start ",tmp) == 0;
                }
            }
        }
    }

    const char *sys_cmd_handler; // Pointer to system cmd handler

    // Determine the system command interpreter.  This could be the full path
    // name if COMSPEC is set, or a default name if it isn't.  We no longer
    // suport Windows 95, so the default will be cmd.exe. But, it is still
    // possible for people to set COMSPEC to command.com, so we could end up
    // with command.com
    if ( (sys_cmd_handler = getenv(COMSPEC)) == NULL )
    {
        sys_cmd_handler = CMDDEFNAME;
    }

    // Determine the maximum possible buffer size needed to pass the final
    // command to sysCommandNT().
    size_t maxBufferSize = strlen(sys_cmd_handler) + 1
                           + strlen(cl_opt)
                           + strlen(&interncmd[j])
                           + 2   // Two possible extra quotes
                           + 1;  // Terminating null

    char  cmdstring[CMDBUFSIZE];    // Default static buffer.
    char *cmdstring_ptr = cmdstring;  // Will point to static buffer.

    if ( maxBufferSize > CMDBUFSIZE )
    {
        // Allocate dynamic memory and set cmdstring_ptr to point to it.
        cmdstring_ptr = (char *)LocalAlloc(LPTR, maxBufferSize);
        if ( cmdstring_ptr == NULL )
        {
            context->RaiseException1(Rexx_Error_System_resources_user_defined,
                                     context->String("Failed to allocate memory"));
            return NULLOBJECT;
        }
    }
    else
    {
        // We want maxBufferSize to relect the actual size of the buffer we are
        // using so that we can test the return from SearchPath()
        maxBufferSize = CMDBUFSIZE;
    }

    SystemInterpreter::exceptionConsole = false;
    SystemInterpreter::explicitConsole = false;

    // Check whether or not the command to invoke is cmd.exe or command.com
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
        uint32_t count = SearchPath(NULL, cmdstring_ptr, ".EXE", (uint32_t)(maxBufferSize - 1), cmdstring, &filepart);
        bool fileFound = count != 0 && count <= maxBufferSize;

        // Set pointer back again to cmd buffer (might have been increased)
        cmdstring_ptr = cmdstring;

        if (fileFound && !stricmp(sys_cmd_handler, cmdstring_ptr))
        {
            SystemInterpreter::exceptionConsole = true;
            SystemInterpreter::explicitConsole = true;
        }
    }

    // First check whether we can run the command directly as a program. (There
    // can be no file redirection when not using cmd.exe or command.com)
    //  N.B. ADDRESS ... WITH also implies redirection
    if (SystemInterpreter::explicitConsole || (!noDirectInvoc && !ioContext->IsRedirectionRequested()))
    {
        // Invoke this directly.  If we fail, we fall through and try again.
        if (sysCommandNT(context, cmd, &interncmd[j], true, result, ioContext))
        {
            if ( cmdstring_ptr != cmdstring )
            {
                // Not pointing to static buffer so we need to free it.
                LocalFree(cmdstring_ptr);
            }
            return result;
        }
    }

    // We couldn't invoke the command directly, or we tried and failed.  So,
    // pass the command to cmd.exe or command.com.


    // Start the command buffer with the system cmd handler
    strcpy(cmdstring_ptr,sys_cmd_handler);

    // Check whether or not the user specified the /k option.  If so do not use
    // the /c option.  The /k option can only be specified as the first
    // argument, and if used, keeps the command handler process open after the
    // command has finished.  Normally the /c option would be usee to close the
    // command handler process when the command is finished..
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

    // Add cmd to be executed, possibly quoting it to preserve embedded quotes.
    if ( quoteCount> 2 )
    {
        strcat(cmdstring_ptr,"\"");
        strcat(cmdstring_ptr,interncmd);
        strcat(cmdstring_ptr,"\"");
    }
    else
    {
        strcat(cmdstring_ptr,interncmd);
    }

    // Invoke the command
    if (!sysCommandNT(context, cmd, cmdstring_ptr, false, result, ioContext))
    {
        // Failed, get error code and return
        context->RaiseCondition("FAILURE", context->String(cmd), NULLOBJECT, context->WholeNumberToObject(GetLastError()));

        if ( cmdstring_ptr != cmdstring )
        {
            // Not pointing to static buffer so we need to free it.
            LocalFree(cmdstring_ptr);
        }
        return NULLOBJECT;
    }

    if ( cmdstring_ptr != cmdstring )
    {
        // Not pointing to static buffer so we need to free it.
        LocalFree(cmdstring_ptr);
    }
    SystemInterpreter::exceptionConsole = false;
    return result;
}


/**
 * Register the standard system command handlers.
 *
 * @param instance The created instance.
 */
void SysInterpreterInstance::registerCommandHandlers(InterpreterInstance *instance)
{
    // The default command handler on Windows is "CMD"
    // It comes with three aliases named "", "COMMAND", and "SYSTEM"
    // "SYSTEM" is compatible with Regina
    instance->addCommandHandler("CMD",     (REXXPFN)systemCommandHandler, HandlerType::REDIRECTING);
    instance->addCommandHandler("",        (REXXPFN)systemCommandHandler, HandlerType::REDIRECTING);
    instance->addCommandHandler("COMMAND", (REXXPFN)systemCommandHandler, HandlerType::REDIRECTING);
    instance->addCommandHandler("SYSTEM",  (REXXPFN)systemCommandHandler, HandlerType::REDIRECTING);

    // This is a no-shell environment that searches PATH.  It is named "PATH"
    // which happens to be compatible with Regina.
    instance->addCommandHandler("PATH",    (REXXPFN)systemCommandHandler, HandlerType::REDIRECTING);
}

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

#include "SysLocalAPIManager.hpp"
#include "LocalAPIManager.hpp"
#include <stdio.h>
#include "SysCSNamedPipeStream.hpp"
#include "SysProcess.hpp"


/* - - - - Temporary stuff for debugging help, will be removed  - - - - - - - */
#if 0
#include <shlobj.h>
#include <shlwapi.h>
TCHAR fileBuf[MAX_PATH];
static const char *fileName = NULL;

void __cdecl DebugMsg(const char* pszFormat, ...)
{
  FILE *stream;
  va_list arglist;
  char buf[1024];
  sprintf(buf, "[%s](%lu): ", SERVICENAME, GetCurrentThreadId());
  va_start(arglist, pszFormat);
  vsprintf(&buf[strlen(buf)], pszFormat, arglist);
  va_end(arglist);
  strcat(buf, "\n");

  if ( fileName == NULL )
  {
      SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS | CSIDL_FLAG_CREATE, NULL, 0, fileBuf);
      PathAppend(fileBuf, "clientApi.log");
      fileName = fileBuf;
  }

  stream = fopen(fileName, "a+");
  if ( stream )
  {
    fwrite(buf, 1, strlen(buf), stream);
    fclose(stream);
  }
}
#endif

/**
 * Check if the rxapi daemon process is running and start it up if not.
 */
void SysLocalAPIManager::startServerProcess()
{
    char apiExeName[] = "RXAPI.EXE";
    LPCTSTR lpszImageName = NULL;       // address of module name
    LPTSTR lpszCommandLine = NULL;      // address of command line
    LPSECURITY_ATTRIBUTES
        lpsaProcess = NULL;                 // address of process security attrs
    LPSECURITY_ATTRIBUTES
        lpsaThread = NULL;                  // address of thread security attrs */
    /* don't inherit handles, because otherwise files are inherited
       and inaccessible until RXAPI.EXE is stopped again */
    BOOL fInheritHandles = FALSE;       //  new process doesn't inherit handles
    DWORD fdwCreate = DETACHED_PROCESS; // creation flags
    LPVOID lpvEnvironment = NULL;       // address of new environment block
    char szSysDir[256];                 // retrieve system directory
    LPCTSTR lpszCurDir = szSysDir;      // address of command line
    LPSTARTUPINFO lpsiStartInfo;        // address of STARTUPINFO
    STARTUPINFO siStartInfo;
    LPPROCESS_INFORMATION lppiProcInfo; // address of PROCESS_INFORMATION
    PROCESS_INFORMATION MemMgrProcessInfo;
    lppiProcInfo = &MemMgrProcessInfo;

    char *fullExeName = NULL;
    // determine the location from location of rexxapi.dll.
    const char *installLocation =  SysProcess::getLibraryLocation();

    // we should get this, if not, we'll just work from the path
    if (installLocation == NULL)
    {
        fullExeName = strdup(apiExeName);
    }
    else
    {
        size_t commandSize = strlen(installLocation) + strlen(apiExeName) + 3;

        fullExeName = (char *)malloc(commandSize);
        // NB: the executable location includes the trailing "\" character
        // however, the path might contain blanks, so we'll need to enclose the
        // command name in quotes
        snprintf(fullExeName, commandSize, "\"%s%s\"", installLocation, apiExeName);
    }


    siStartInfo.cb = sizeof(siStartInfo);
    siStartInfo.lpReserved = NULL;
    siStartInfo.lpDesktop = NULL;
    siStartInfo.lpTitle = NULL;
    siStartInfo.dwX = 0;
    siStartInfo.dwY = 0;
    siStartInfo.dwXSize = 0;
    siStartInfo.dwYSize = 0;
    siStartInfo.dwXCountChars = 0;
    siStartInfo.dwYCountChars = 0;
    siStartInfo.dwFillAttribute = 0;
    siStartInfo.dwFlags = 0;
    siStartInfo.wShowWindow = 0;
    siStartInfo.cbReserved2 = 0;
    siStartInfo.lpReserved2 = NULL;
    siStartInfo.hStdInput = NULL;
    siStartInfo.hStdOutput = NULL;
    siStartInfo.hStdError = NULL;
    lpsiStartInfo = &siStartInfo;
    lpszCommandLine = fullExeName;

    /* start RXAPI process out of system directory */
    if (!GetSystemDirectory(szSysDir, 255))
    {
        lpszCurDir = NULL;
    }

    if (!CreateProcess(lpszImageName, lpszCommandLine, lpsaProcess,
                       lpsaThread, fInheritHandles, fdwCreate, lpvEnvironment,
                       lpszCurDir, lpsiStartInfo, lppiProcInfo))
    {
        free(fullExeName);
        // if no install location, we did this already
        if (installLocation == NULL)
        {
            // if we failed using the install location, we'll try again just using the path.
            throw new ServiceException(API_FAILURE, "Unable to start API server");
        }

        // use the fall back command line that will search the path.
        lpszCommandLine = strdup(apiExeName);
        if (!CreateProcess(lpszImageName, lpszCommandLine, lpsaProcess,
                           lpsaThread, fInheritHandles, fdwCreate, lpvEnvironment,
                           lpszCurDir, lpsiStartInfo, lppiProcInfo))
        {
            free(fullExeName);
            // if we failed using the install location, we'll try again just using the path.
            throw new ServiceException(API_FAILURE, "Unable to start API server");
        }
    }
    free(fullExeName);
}


/**
 * Check to see if we've inherited a session queue from a calling process.  This shows
 * up as an environment variable value.
 *
 * @param sessionQueue
 *               The returned session queue handle, if it exists.
 *
 * @return true if the session queue is inherited, false if a new once needs to be created.
 */
bool SysLocalAPIManager::getActiveSessionQueue(QueueHandle &sessionQueue)
{
    // check to see if we have an env variable set...if we do we
    // inherit from our parent session
    char   envbuffer[MAX_QUEUE_NAME_LENGTH+1];
    DWORD envchars = GetEnvironmentVariable("RXQUEUESESSION", (LPTSTR) envbuffer, MAX_QUEUE_NAME_LENGTH);
    if (envchars != 0)
    {
        sscanf(envbuffer, "%p", (void **)&sessionQueue);
        return true;
    }
    return false;
}

/**
 * Set the active session queue as an environment variable.
 *
 * @param sessionQueue
 *               The session queue handle.
 */
void SysLocalAPIManager::setActiveSessionQueue(QueueHandle sessionQueue)
{
    char   envbuffer[MAX_QUEUE_NAME_LENGTH+1];
    // and set this as an environment variable for programs we call
    sprintf(envbuffer, "%p", (void *)sessionQueue);
    SetEnvironmentVariable("RXQUEUESESSION", (LPTSTR) envbuffer);
}


/**
 * Create a new connection instance of the appropiate type for
 * connection to the daemon process.
 *
 * @return A connection instance.
 */
ApiConnection *SysLocalAPIManager::newClientConnection()
{
    SysNamedPipeConnection *connection = new SysNamedPipeConnection();

    // open the pipe to the server
    if (!connection->connect(SysServerNamedPipeConnectionManager::generatePipeName()))
    {
        // don't leak memory!
        delete connection;
        throw new ServiceException(CONNECTION_FAILURE, "Failure connecting to rxapi server");
    }
    return connection;
}

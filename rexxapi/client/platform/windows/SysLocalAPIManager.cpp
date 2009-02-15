/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

#include "SysLocalAPIManager.hpp"
#include "LocalAPIManager.hpp"
#include <stdio.h>


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
/* - - End Temporary stuff for debugging help - - - - - - - - - - - - - - - - */

#define SERVICENAME "RXAPI"

/**
 * If rxapi is installed as a service, and the service is not disabled, and the
 * user (owner of this running process) has the authority to start the rxapi
 * service, then the service is started.
 *
 * This function will wait to see that the service actually starts.  If the
 * above conditions are met, then the service will start.
 *
 * @return True if the service is started and running, otherwise false.
 *
 * @note   Both the service control manager and the service itself are opened
 *         with the minimum access rights needed to accomplish the task.  On
 *         Vista this is especially important because it prevents unnecessary
 *         failures.
 */
bool startAsService(void)
{
    LPQUERY_SERVICE_CONFIG serviceCfg = NULL;
    DWORD needed;
    SC_HANDLE hService = NULL;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if ( hSCM )
    {
        // Open the service with the query and start access rights.
        hService = OpenService(hSCM, SERVICENAME, SERVICE_QUERY_CONFIG | SERVICE_START | SERVICE_QUERY_STATUS);
        if ( hService )
        {
            // The service is installed, make sure it is not currently disabled.
            serviceCfg = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4096);
            if ( serviceCfg )
            {
                if ( QueryServiceConfig(hService, serviceCfg, 4096, &needed) )
                {
                    if ( serviceCfg->dwStartType == SERVICE_DISABLED )
                    {
                        // The service is disabled set hService to null to
                        // indicate rxapi can not be started as a service.
                        CloseServiceHandle(hService);
                        hService = NULL;
                    }
                }
                LocalFree(serviceCfg);
            }
        }
    }

    if ( hService == NULL )
    {
        // Either rxapi is not installed as a service, or it is disabled.
        // Return false so that rxapi will be started through CreateProcess().
        if ( hSCM != NULL )
        {
            CloseServiceHandle(hSCM);
        }
        return false;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD oldCheck;
    DWORD startTicks;
    DWORD waitTime;

    if ( ! StartService(hService, 0, NULL) )
    {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return false;
    }

    // Check the status until the service is no longer start pending.
    if ( QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed) == 0 )
    {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return false;
    }

    // Save the tick count and initial checkpoint.
    startTicks = GetTickCount();
    oldCheck = ssp.dwCheckPoint;

    while ( ssp.dwCurrentState == SERVICE_START_PENDING )
    {
        // Do not wait longer than the wait hint, which for rxapi will be 2000
        // milliseconds.
        //
        // Microsoft suggests that a good interval is one tenth the wait hint,
        // but no less than 1 second and no more than 10 seconds. However, rxapi
        // starts in less than 200 milliseconds, so there is no sense in waiting
        // more than that.
        waitTime = ssp.dwWaitHint / 10;

        if( waitTime < 200 )
        {
            waitTime = 200;
        }
        else if ( waitTime > 10000 )
        {
            waitTime = 10000;
        }

        Sleep(waitTime);

        if ( QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed) == 0 )
        {
            break;
        }

        if ( ssp.dwCheckPoint > oldCheck )
        {
            // The service is making progress, so continue.
            startTicks = GetTickCount();
            oldCheck = ssp.dwCheckPoint;
        }
        else
        {
            if( (GetTickCount() - startTicks) > ssp.dwWaitHint )
            {
                // The wait hint interval has expired and we are still not
                // started, so quit.
                break;
            }
        }
    }

    CloseServiceHandle(hSCM);
    CloseServiceHandle(hService);

    return ssp.dwCurrentState == SERVICE_RUNNING ? true : false;
}

void SysLocalAPIManager::startServerProcess()
{
    // First try to start rxapi as a Windows service.  If that fails, then start
    // it as a normal process.
    if ( startAsService() )
    {
        return;
    }

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
    lpszCommandLine = apiExeName;

    /* start RXAPI process out of system directory */
    if (!GetSystemDirectory(szSysDir, 255))
    {
        lpszCurDir = NULL;
    }

    if(!CreateProcess(lpszImageName, lpszCommandLine, lpsaProcess,
                      lpsaThread, fInheritHandles, fdwCreate, lpvEnvironment,
                      lpszCurDir, lpsiStartInfo, lppiProcInfo))
    {
        throw new ServiceException(API_FAILURE, "Unable to start API server");
    }
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
        sscanf(envbuffer, "%p", &sessionQueue);
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
    sprintf(envbuffer, "%p", sessionQueue);
    SetEnvironmentVariable("RXQUEUESESSION", (LPTSTR) envbuffer);
}

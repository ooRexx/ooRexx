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

// Windows Service status codes
typedef enum
{
    WS_IS_RUNNING = 0,
    WS_NOT_INSTALLED,
    WS_IS_DISABLED,
    WS_IS_STOPPED,
    WS_NOT_PRIVILEGED,
    WS_UNKNOWN
} WinServiceStatusT;


/**
 * Reports if the rxapi service is in the running state at the time this
 * function returns.  If the service is in the START PENDING state the function
 * waits untils the service is running or has timed out.
 *
 * @param hService  Opened service handle, must have the SERVICE_QUERY_STATUS
 *                  privilege.
 *
 * @return True the service is not running, otherwise false.
 */
static bool hasServiceStarted(SC_HANDLE hService)
{
    SERVICE_STATUS_PROCESS ssp;
    DWORD                  needed;

    if ( QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed) == 0 )
    {
        return false;
    }

    if ( ssp.dwCurrentState == SERVICE_RUNNING || ssp.dwCurrentState == SERVICE_STOPPED ||
         ssp.dwCurrentState == SERVICE_STOP_PENDING )
    {
        return ssp.dwCurrentState == SERVICE_RUNNING;
    }

    // Save the tick count and initial checkpoint.
    uint32_t startTicks = GetTickCount();
    uint32_t oldCheck = ssp.dwCheckPoint;
    uint32_t waitTime;

    // Check the status until the service is no longer start pending.  rxapi is
    // not pausable, so PAUSED or PAUSED_PENDING should not be possible.
    while ( ssp.dwCurrentState == SERVICE_START_PENDING )
    {
        // Do not wait longer than the wait hint, which for rxapi will be 2000
        // milliseconds.
        //
        // Microsoft suggests that a good interval is one tenth the wait hint,
        // but not less than 1 second and no more than 10 seconds. rxapi usually
        // starts in less than 200 milliseconds.
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

        BOOL success = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed);
        if ( ! success || ssp.dwCurrentState == SERVICE_RUNNING )
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

    return ssp.dwCurrentState == SERVICE_RUNNING ? true : false;
}


/**
 * Get the status of the rxapi service, which could be not installed as a
 * service.
 *
 * @param phSCM   Pointer to a handle for the Service Control Manager.  If rxapi
 *                is installed as a service, and not disabled, an open handle is
 *                returned here.  Otherwise the handle is set to NULL.
 *
 * @return  A WinServiceStatusT enum indicating the status of rxapi as a Windows
 *          service.
 */
static WinServiceStatusT getServiceStatus(SC_HANDLE *phSCM)
{
    WinServiceStatusT      status = WS_UNKNOWN;
    SC_HANDLE              hService = NULL;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if ( hSCM )
    {
        // Open the service with the query access rights.
        hService = OpenService(hSCM, SERVICENAME, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
        if ( hService )
        {
            // The service is installed, make sure it is not currently disabled.
            LPQUERY_SERVICE_CONFIG serviceCfg = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4096);
            if ( serviceCfg )
            {
                DWORD needed;

                if ( QueryServiceConfig(hService, serviceCfg, 4096, &needed) )
                {
                    if ( serviceCfg->dwStartType == SERVICE_DISABLED )
                    {
                        status = WS_IS_DISABLED;
                    }
                }
                LocalFree(serviceCfg);
            }
            else
            {
                status = WS_NOT_PRIVILEGED;
            }

            if ( status != WS_UNKNOWN )
            {
                CloseServiceHandle(hService);
                hService = NULL;
            }
        }
    }
    else
    {
        status = WS_NOT_PRIVILEGED;
    }

    if ( status != WS_UNKNOWN )
    {
        if ( hSCM != NULL )
        {
            CloseServiceHandle(hSCM);
        }
        return status;
    }

    *phSCM = hSCM;

    if ( hasServiceStarted(hService) )
    {
        status = WS_IS_RUNNING;
    }
    else
    {
        status = WS_IS_STOPPED;
    }

    CloseServiceHandle(hService);
    return status;
}

/**
 * Determines if rxapi is installed and running as a service.
 *
 * If rxapi is installed as a service, but not running an attempt is made to
 * start the service.  When the service is not disabled, and the user (owner of
 * this running process) has the authority to start the rxapi service, then the
 * attempt should succeed.
 *
 * If the function attempts to start the service, it will wait to see that the
 * service actually starts. If the above conditions are met, then the service
 * will start.
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
    SC_HANDLE hSCM = NULL;

    // First see if rxapi is already running as a service, or if not running, if
    // there is at least a possibility of starting it.
    WinServiceStatusT status = getServiceStatus(&hSCM);
    if ( status == WS_NOT_INSTALLED || status == WS_IS_DISABLED || status == WS_NOT_PRIVILEGED )
    {
        return false;
    }
    else if ( status == WS_IS_RUNNING )
    {
        CloseServiceHandle(hSCM);
        return true;
    }

    // Okay, the service is not running, but it is installed and should be
    // startable.
    SC_HANDLE hService = OpenService(hSCM, SERVICENAME, SERVICE_QUERY_CONFIG | SERVICE_START | SERVICE_QUERY_STATUS);
    if ( hService == NULL )
    {
        // Seems the user has sufficient privileges to query the service, but
        // not to start it.
        if ( hSCM != NULL )
        {
            CloseServiceHandle(hSCM);
        }
        return false;
    }

    bool hasStarted = false;

    if ( ! StartService(hService, 0, NULL) )
    {
        goto done_out;
    }

    hasStarted = hasServiceStarted(hService);

done_out:
    CloseServiceHandle(hSCM);
    CloseServiceHandle(hService);
    return hasStarted;
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

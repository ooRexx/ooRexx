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

#include <aclapi.h>
#include "APIServer.hpp"
#include "stdio.h"

#define SERVICENAME "RXAPI"
#define SERVICEDESCRIPTION "%s Service for Open Object Rexx version %d.%d.%d"

#define SYNTAX_HELP  "Syntax: rxapi opt [/s]\n\n"     \
                     "Where opt is exactly one of:\n" \
                     "/i :\tinstall as Service.\n"    \
                     "/u :\tuninstall as Service.\n"  \
                     "/v :\tshow Version number\n\n"  \
                     "The /s (silent) is optional and prevents any messages from displaying."

#define APP_LOG_KEYNAME  "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"

typedef enum {installed_state, disabled_state, uninstalled_state} ServiceStateType;
typedef enum {Windows_NT, Windows_2K} WindowsVersionType;

// Prototypes
void setServiceDACL(SC_HANDLE hService);

// Globals for Service
static SERVICE_STATUS_HANDLE m_hServiceStatus;
static SERVICE_STATUS m_Status;
static SERVICE_DESCRIPTION Info;


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
  sprintf(buf, "[%s](%lu)(%lu){%d}: ", SERVICENAME, GetCurrentThreadId(),GetCurrentProcessId(),
          m_Status.dwCurrentState);
  va_start(arglist, pszFormat);
  vsprintf(&buf[strlen(buf)], pszFormat, arglist);
  va_end(arglist);
  strcat(buf, "\n");

  if ( fileName == NULL )
  {
      SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS | CSIDL_FLAG_CREATE, NULL, 0, fileBuf);
      PathAppend(fileBuf, "apiService.log");
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

APIServer apiServer;             // the real server instance

// A couple of helper functions.
inline void showMessage(const char *msg, unsigned int icon)
{
    MessageBox(NULL, msg, SERVICENAME, MB_OK | icon);
}

#define WINDOWS_95_98_ME  VER_PLATFORM_WIN32_WINDOWS

bool isAtLeastVersion(WindowsVersionType type)
{
    OSVERSIONINFO version_info;
    bool answer = false;

    version_info.dwOSVersionInfoSize = sizeof(version_info);
    GetVersionEx(&version_info);

    switch ( type )
    {
        case Windows_NT :
            answer = version_info.dwPlatformId != WINDOWS_95_98_ME;
            break;

        case Windows_2K :
            answer = version_info.dwMajorVersion >= 5;
            break;

        default :
            break;
    }
    return answer;
}

/**
 * Starts up the API server and has it listen for messages.
 *
 * Run() is either called from ServiceMain() in which case the API server will
 * be running in a Service process.  If rxapi is not installed as a service, or
 * for some reason the service can not be started, then Run() is called from
 * WinMain() and the API server runs in a standard process.
 *
 * @param asService  If true, Run() was called from ServiceMain(), otherwise
 *                   Run() was called from WinMain().
 */
void Run (bool asService)
{
    try
    {
        apiServer.initServer();               // start up the server
        apiServer.listenForConnections();     // go into the message loop
    }
    catch (ServiceException *)
    {
    }
    apiServer.terminateServer();     // shut everything down
}

/**
 * The following functions handle the Service control requests.
 */

// Called when the service is first initialized
bool OnInit()
{
    return true;
}

// Called when the service control manager wants to stop the service
void OnStop()
{
    apiServer.terminateServer();    // terminate the server.
}

// called when the service is interrogated
void OnInterrogate()
{
}

// called when the service is paused
void OnPause()
{
}

// called when the service is continued
void OnContinue()
{
}

// called when the service is shut down
void OnShutdown()
{
}

// called when the service gets a user control message
BOOL OnUserControl(DWORD dwOpcode)
{
    return FALSE; // say not handled
}

/**
 * Updates the Service Control Manager with our current state.
 *
 * @param dwState  The current state.
 */
void SetStatus(DWORD dwState)
{
    m_Status.dwCurrentState = dwState;
    SetServiceStatus(m_hServiceStatus, &m_Status);
}

/**
 * The handler function registered with the Service Control dispatcher.  The
 * dispatcher uses this function to send service control messages to the service
 * process.
 *
 * @param dwOpcode  The control message.
 */
static void __cdecl Handler(DWORD dwOpcode)
{
    switch (dwOpcode) {
    case SERVICE_CONTROL_STOP: // 1
        SetStatus(SERVICE_STOP_PENDING);
        OnStop();
        break;

    case SERVICE_CONTROL_PAUSE: // 2
        OnPause();
        break;

    case SERVICE_CONTROL_CONTINUE: // 3
        OnContinue();
        break;

    case SERVICE_CONTROL_INTERROGATE: // 4
        OnInterrogate();
        break;

    case SERVICE_CONTROL_SHUTDOWN: // 5
        OnShutdown();
        break;

    default:
        break;
    }

    // Report current status
    SetServiceStatus(m_hServiceStatus, &m_Status);
}

bool Initialize()
{
    bool result;

    // Inform the Service Control Manager that we are about to start.
    SetStatus(SERVICE_START_PENDING);

    // Perform the actual initialization
    result = OnInit();

    // Inform the Service Control Manager of our final state.
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;

    if ( ! result )
    {
        SetStatus(SERVICE_STOPPED);
        return false;
    }
    SetStatus(SERVICE_RUNNING);
    return true;
}

/**
 * After the service control dislpatcher receives a start request, (see
 * startTheService(),) it creates a new thread and invokes this function on that
 * thread to do the actual work of the service.
 *
 * @param dwArgc    Count of args  (not used by rxapi)
 * @param lpszArgv  Args           (not used by rxapi)
 */
static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler.  The Service Control Manager will call back to this
    // function to issue control requests.
    m_Status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(SERVICENAME, (LPHANDLER_FUNCTION)Handler);
    if ( m_hServiceStatus ==  (SERVICE_STATUS_HANDLE)NULL )
    {
        return;
    }

    // Start the initialisation
    if ( Initialize() )
    {
        // Do the actual work.  When the Run function returns, the service has
        // stopped.
        m_Status.dwWin32ExitCode = 0;
        m_Status.dwCheckPoint = 0;
        m_Status.dwWaitHint = 0;

        Run(true);
    }

    // Tell the service control manager we have stopped
    SetStatus(SERVICE_STOPPED);
}

/**
 * Connects the main thread of this service process to the Service Control
 * Manager.  When this function succeeds, it will not return until the rxapi
 * Service is stopped.
 *
 * @return A return of true indicates that rxapi was successfully started as a
 *         service process and has run to completion.  A return of false
 *         indicates the rxapi service process was not started.
 *
 * @note   When rxapi is installed as a service, it can not run as a service
 *         process unless it is started through the Service Control Manager.  If
 *         rxapi is started through the command line or by CreateProcess(),
 *         StartServiceCtrlDispatcher() will return
 *         ERROR_FAILED_SERVICE_CONTROLLER_CONNECT and the service process will
 *         not have run.
 */
bool startTheService(void)
{
    SERVICE_TABLE_ENTRY st[] =
    {
      {
        (LPTSTR)SERVICENAME,
        ServiceMain
      },
      {
        NULL, NULL
      }
    };

    m_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_Status.dwCurrentState = SERVICE_STOPPED;
    m_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_Status.dwWin32ExitCode = 0;
    m_Status.dwServiceSpecificExitCode = 0;
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;

    return StartServiceCtrlDispatcher(st) != 0;
}

/**
 * If rxapi is currently running as a Service process, then stop it.
 *
 * @param hService  Opened handle to the service.  The handle must have been
 *                  opened with SERVICE_QUERY_STATUS and SERVICE_STOP access
 *                  rights.
 *
 * @param timeOut   Time to wait, in miliseconds for a pending stop to clear.
 *
 * @return  True if the service is stopped, false if not sure that the service
 *          is stopped.
 */
bool stopTheService(SC_HANDLE hService, DWORD timeOut)
{
    SERVICE_STATUS_PROCESS ssp;
    DWORD startTicks = GetTickCount();
    DWORD waitTime = timeOut / 20;
    DWORD needed;
    bool success = false;

    // See if the service is already stopped.
    if ( ! QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed) )
    {
        goto finished;
    }

    if ( ssp.dwCurrentState == SERVICE_STOPPED )
    {
        success = true;
        goto finished;
    }

    // When a service has a stop pending, we'll wait for it.
    while ( ssp.dwCurrentState == SERVICE_STOP_PENDING )
    {
        Sleep(waitTime);
        if ( ! QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed) )
        {
            goto finished;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED )
        {
            success = true;
            goto finished;
        }

        if ( GetTickCount() - startTicks > timeOut )
        {
            goto finished;
        }
    }

    // Send a stop code through the Sevice Control Manager to the service
    SERVICE_STATUS ss;
    if ( ! ControlService(hService, SERVICE_CONTROL_STOP, &ss) )
    {
        goto finished;
    }

    // Wait for the service to stop
    while ( ssp.dwCurrentState != SERVICE_STOPPED )
    {
        Sleep(waitTime);
        if ( ! QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &needed) )
        {
            break;
        }

        if ( ss.dwCurrentState == SERVICE_STOPPED )
        {
            success = true;
            break;
        }

        if ( GetTickCount() - startTicks > timeOut )
        {
            break;
        }
    }

finished:
    return success;
}

void setServiceDACL(SC_HANDLE hService)
{
    EXPLICIT_ACCESS      ea;
    SECURITY_DESCRIPTOR  sd;
    PSECURITY_DESCRIPTOR psd            = NULL;
    PACL                 pacl           = NULL;
    PACL                 pNewAcl        = NULL;
    BOOL                 bDaclPresent   = FALSE;
    BOOL                 bDaclDefaulted = FALSE;
    DWORD                dwError        = 0;
    DWORD                needed         = 0;

    // Get the current security descriptor for the service.  First query with a
    // size of 0 to get the needed size.
    if ( ! QueryServiceObjectSecurity(hService, DACL_SECURITY_INFORMATION, &psd, 0, &needed) )
    {
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            DWORD size = needed;
            psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, size);
            if (psd == NULL)
            {
                goto cleanup;
            }

            if ( ! QueryServiceObjectSecurity( hService, DACL_SECURITY_INFORMATION, psd, size, &needed) )
            {
                goto cleanup;
            }
        }
        else
        {
            goto cleanup;
        }
    }

    // Get the current DACL from the security descriptor.
    if ( ! GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted) )
    {
        goto cleanup;
    }

    // Build the ACE.
    BuildExplicitAccessWithName(&ea, TEXT("Users"), SERVICE_START | SERVICE_STOP | GENERIC_READ, SET_ACCESS, NO_INHERITANCE);

    dwError = SetEntriesInAcl(1, (PEXPLICIT_ACCESS)&ea, pacl, &pNewAcl);
    if ( dwError != ERROR_SUCCESS )
    {
        goto cleanup;
    }

    // Initialize a new security descriptor.
    if ( ! InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) )
    {
        goto cleanup;
    }

    // Set the new DACL in the security descriptor.
    if ( ! SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE) )
    {
        goto cleanup;
    }

    // Set the new security descriptor for the service object.
    SetServiceObjectSecurity(hService, DACL_SECURITY_INFORMATION, &sd);

cleanup:
    if( pNewAcl != NULL )
    {
        LocalFree((HLOCAL)pNewAcl);
    }
    if( psd != NULL )
    {
        LocalFree((LPVOID)psd);
    }
}

/**
 * Install rxapi as a Windows service and add the registry entries to allow
 * logging through the Event Log service.
 *
 * @return True if the service was installed, otherwise false.
 */
bool Install()
{
    char szFilePath[_MAX_PATH];
    SC_HANDLE hService = NULL;
    char szKey[_MAX_PATH];
    HKEY hKey = NULL;
    DWORD dwData;
    bool success = false;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE | SC_MANAGER_CONNECT);
    if ( hSCM == NULL )
    {
      goto cleanup;
    }

    // Get the executable file path.  The docs for Services say that if the path
    // contains spaces it must be quoted.

    // Use szKey as a temp buffer.
    GetModuleFileName(NULL, szKey, sizeof(szKey));
    if ( strstr(szKey, " ") == 0 )
    {
        strcpy(szFilePath, szKey);
    }
    else
    {
        _snprintf(szFilePath, sizeof(szFilePath), "\"%s\"", szKey);
    }

    // Create the service
    hService = CreateService(hSCM, SERVICENAME, SERVICENAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                             SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szFilePath, NULL, NULL, NULL, NULL,
                             NULL);

    if ( hService == NULL )
    {
        goto cleanup;
    }

    // At this point, the service is installed, so success is set to true.  The
    // rest of the stuff is icing on the cake so to speak.  If something fails,
    // it might be nice to notify someone, but we shouldn't say the service
    // install failed, because it hasn't.
    success = true;

    // ChangeServiceConfig2() needs W2K or later.
    if ( isAtLeastVersion(Windows_2K) )
    {
        // Use szKey as a temporary buffer
        sprintf(szKey, SERVICEDESCRIPTION, SERVICENAME, ORX_VER, ORX_REL, ORX_MOD);
        Info.lpDescription = szKey;

        ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &Info);
    }

    // Set the discretionary ACL for the service.
    setServiceDACL(hService);

    // Add the registry entries to support logging messages.  The source name is
    // added as a subkey under the Application key in the EventLog service
    // portion of the registry.
    strcpy(szKey, APP_LOG_KEYNAME);
    strcat(szKey, SERVICENAME);

    if ( RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) == ERROR_SUCCESS )
    {
        // Set the value of 'EventMessageFile' to the Event ID message-file name.
        RegSetValueEx(hKey, "EventMessageFile", 0, REG_EXPAND_SZ, (CONST BYTE*)szFilePath,
                      (DWORD)strlen(szFilePath) + 1);

        // Set the value of 'TypesSupported' to the supported types.
        dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD, (CONST BYTE*)&dwData, sizeof(DWORD));

        RegCloseKey(hKey);
    }

cleanup:
    if ( hService != NULL )
    {
        CloseServiceHandle(hService);
    }
    if ( hSCM != NULL )
    {
        CloseServiceHandle(hSCM);
    }
    return success;
}

/**
 * Deletes rxapi as a service and cleans up the registry entries that were made
 * when rxapi was installed as a service.
 *
 * @return true on success, othewise false.
 */
bool Uninstall()
{
    bool success = false;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if ( hSCM != NULL )
    {
        SC_HANDLE hService = OpenService(hSCM, SERVICENAME, DELETE | SERVICE_QUERY_STATUS | SERVICE_STOP);
        if ( hService != NULL )
        {
            // First stop the service if it is running, which allows the Service
            // Control Manger to clean up the database immediately.  If we fail
            // to stop the service we just ignore it.  The database will be
            // cleaned up at the next reboot. (Or sooner if the rxapi process is
            // killed.)
            stopTheService(hService, 1000);

            if ( DeleteService(hService) )
            {
                success = true;
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }

    // Remove the Event Log service entries in the registry for rxapi.  These
    // entries are added during the Service installation function.  Any errors
    // are just ignored.
    if ( success )
    {
        HKEY hKey = NULL;
        if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, APP_LOG_KEYNAME, 0, DELETE, &hKey) == ERROR_SUCCESS )
        {
            if ( RegDeleteKey(hKey, SERVICENAME) == ERROR_SUCCESS )
            {
                RegCloseKey(hKey);
            }
        }
    }
    return success;
}

/**
 * Determines if: rxapi is installed as a service, installed but the service is
 * currently disabled, or not installed as a service.
 *
 * @return  One of the 3 service state enums.
 */
ServiceStateType getServiceState(void)
{
    ServiceStateType state = uninstalled_state;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if ( hSCM )
    {
        // Try to open the service, if this fails, rxapi is not installed as a
        // service.
        SC_HANDLE hService = OpenService(hSCM, SERVICENAME, SERVICE_QUERY_CONFIG);
        if ( hService )
        {
            // The service is definitely installed, we'll mark it as disabled
            // until we know for sure it is not disabled.
            state = disabled_state;

            LPQUERY_SERVICE_CONFIG pqsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4096);
            if ( pqsc )
            {
                DWORD needed;
                if ( QueryServiceConfig(hService, pqsc, 4096, &needed) )
                {
                    if ( pqsc->dwStartType != SERVICE_DISABLED )
                    {
                        state = installed_state;
                    }
                }
                LocalFree(pqsc);
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }
    return state;
}

/**
 * Start rxapi as a Windows Service, if possible.  This function determines if
 * rxapi is installed as a Service, and, if so, starts up the Service.
 *
 * @return bool
 */
bool startAsWindowsService(void)
{
    bool started;
    DWORD errRC;
    int iRet;

    if ( getServiceState() == installed_state )
    {
        // When the service is started successfully here, we do not return until
        // the service has been stopped.
        started = startTheService();

        if ( ! started )
        {
            // The service did not start, get the error code.
            errRC = GetLastError();
            if ( errRC == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT )
            {
                // This specific error happens when rxapi is not running and is
                // started from the command line or by CreateProcess().  Service
                // processes can not run when started as console programs.
                // (Really we should educate the user not to do this.)  We use
                // NET START RXAPI, which will invoke the service controller to
                // start rxapi correctly. When we return from system(), rxapi
                // will be running as a service in another process, so it is
                // important to exit this process.

                iRet = system("NET start rxapi");

                // If the return is not 0, rxapi was not started as a service.
                return iRet == 0;
            }
            else
            {
                // The service could not be started for some un-anticipated
                // reason.  Return false so that rxapi will continue by running
                // as a standard process.
                return false;
            }
        }

        // Do not continue by running rxapi as a standard process.
        return true;
    }

    // Either rxapi is not installed as a Service, or the Service has been set
    // to disabled. Continue by running rxapi as a standard process.
    return false;
}

/**
 * We have command line args, process them.  Most likely the args are to install
 * or uninstall rxapi as a service.
 *
 * @param cmdLine The command line, must not be null.
 */
void processCmdLine(const char *cmdLine)
{
    char msg[256];
    bool verbose = true;

    size_t len = strlen(cmdLine);

    if ( cmdLine[0] != '/' || len < 2 )
    {
        showMessage(SYNTAX_HELP, MB_ICONERROR);
        return;
    }

    if ( len >= 5 )
    {
        char opt = cmdLine[4];
        if ( opt == 's' || opt == 'S' || opt == 'q' || opt == 'Q' )
        {
            verbose = false;
        }
    }

    bool isInstalled = (getServiceState() != uninstalled_state);

    switch ( cmdLine[1] )
    {
        case 'v' :
        case 'V' :
            sprintf(msg ,"%s Version %d.%d.%d\n\nThe service is %s installed", SERVICENAME,
                    ORX_VER, ORX_REL, ORX_MOD, isInstalled ? "currently" : "not");
            showMessage(msg, MB_ICONINFORMATION);
            break;

        case 'i' :
        case 'I' :
            if ( isInstalled )
            {
                if ( verbose )
                {
                    sprintf(msg, "%s is already installed as a service.", SERVICENAME);
                    showMessage(msg, MB_ICONWARNING);
                }
            }
            else
            {
                if ( Install() )
                {
                    if ( verbose )
                    {
                        sprintf(msg, "The %s service was installed\n", SERVICENAME);
                        showMessage(msg, MB_ICONINFORMATION);
                    }
                }
                else
                {
                    if ( verbose )
                    {
                        sprintf(msg, "The %s service failed to install.\n"
                                     "Error %d\n", SERVICENAME, GetLastError());
                        showMessage(msg, MB_ICONERROR);
                    }
                }
            }
            break;

        case 'u' :
        case 'U' :
            if ( ! isInstalled )
            {
                if ( verbose )
                {
                    sprintf(msg, "%s is not installed as a service\n", SERVICENAME);
                    showMessage(msg, MB_ICONWARNING);
                }
            }
            else
            {
                if ( Uninstall() )
                {
                    if ( verbose )
                    {
                        sprintf(msg, "%s was uninstalled as a service.\n\n"
                                     "(Open Object REXX is not uninstalled.)", SERVICENAME);
                        showMessage(msg, MB_ICONINFORMATION);
                    }
                }
                else
                {
                    if ( verbose )
                    {
                        sprintf(msg, "The %s service could not be uninstalled.\n"
                                     "Error %d\n", SERVICENAME, GetLastError());
                        showMessage(msg, MB_ICONERROR);
                    }
                }
            }
            break;

        default :
            showMessage(SYNTAX_HELP, MB_ICONERROR);
            break;
    }
    return;
}

/**
 * The main entry point for rxapi.exe.
 *
 * When rxapi is installed as a service, the invoker might be the service
 * control manager, or the interpreter could be starting rxapi as a service
 * because rxapi was not running.
 *
 * When rxapi is not installed as a service, then the invoker is most likely the
 * interpreter starting up rxapi.  Although, the user could also be invoking
 * rxapi from the command line.
 *
 * The third possibility is that rxapi is being invoked from the command line
 * with arguments to either install, uninstall, or query rxapi as a service.
 *
 *
 * @param hInstance
 * @param hPrevInstance
 * @param lpCmdLine       The only arg we are interested in, the command line
 *                        arguments.
 * @param nCmdShow
 *
 * @return 0
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if ( isAtLeastVersion(Windows_NT) )
    {
        // If there are args, we always process them and exit.
        if ( strlen(lpCmdLine) != 0 )
        {
            processCmdLine(lpCmdLine);
            exit(0);
        }

        // When rxapi is successfully started as a service, we do not return
        // from startAsWindowsService() until the service is stopped.
        if ( startAsWindowsService() )
        {
            // rxapi ran as a service, and is now ended / stopped.
            exit(0);
        }
    }

    // For some reason we did not run as a service, (either not installed as
    // service, the user does not have the authority to start a stopped service,
    // or some other reason.) In this case, run rxapi as a normal process.
    Run(false);

    return 0;
}

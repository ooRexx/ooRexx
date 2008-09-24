/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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

#include "APIServer.hpp"
#include "stdio.h"

#define SERVICENAME "RXAPI"
#define MAJORVERSION 3
#define MINORVERSION 0
#define SUBVERSION   0
#define SERVICEDESCRIPTION "RXAPI Service for Open Object Rexx version 3.0.0.0" /* @DOE002C @DOE003C @RIN005 */

// Globals for Service

static SERVICE_STATUS_HANDLE m_hServiceStatus;
static SERVICE_STATUS m_Status ;
static SERVICE_DESCRIPTION Info;

// End Globals for Service


/* module static data -------------------------------------------------*/

APIServer apiServer;             // the real server instance

/*==========================================================================*
 *  Function: Run
 *
 *  Purpose:
 *
 *  handles the original RXAPI functions.
 *    Register window classes, create windows,
 *    perform a message loop
 *
 *  RIN006 : is either called from WINMAIN, if no arguments and
 *           RXAPI is NOT installed as an service,
 *           or, from ServiceMain, if RXAPI is installed as an service.
 *
 *
 *==========================================================================*/
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

//////////////////////////////////////////////////////////////////////////////////////
// Control request handlers


// Called when the service is first initialized
BOOL OnInit()
{
    return TRUE;
}

// Called when the service control manager wants to stop the service
void OnStop()
{
    apiServer.stop();    // tell the server to stop processing messages.
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


///////////////////////////////////////////////////////////////////////////////////////////
// status functions

void SetStatus(DWORD dwState)
{
    m_Status.dwCurrentState = dwState;
    SetServiceStatus(m_hServiceStatus, &m_Status);
}


// static member function (callback) to handle commands from the
// service control manager
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


///////////////////////////////////////////////////////////////////////////////////////////
// Service initialization

BOOL Initialize()
{

  BOOL bResult ;
    // Start the initialization
    SetStatus(SERVICE_START_PENDING);

    // Perform the actual initialization
    bResult = OnInit();

    // Set final state
    m_Status.dwWin32ExitCode = GetLastError();
    m_Status.dwCheckPoint = 0;
    m_Status.dwWaitHint = 0;
    if (!bResult)
    {
        SetStatus(SERVICE_STOPPED);
        return FALSE;
    }
    SetStatus(SERVICE_RUNNING);
    return TRUE;
}


/*==========================================================================*
 *  Function: ServiceMain
 *
 *  Purpose:
 *
 *  static member function (callback)
 *
 *==========================================================================*/
static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
  // Register the control request handler
    m_Status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(SERVICENAME,
        (LPHANDLER_FUNCTION )Handler);
    if (m_hServiceStatus ==  (SERVICE_STATUS_HANDLE )NULL)
    {
        return;
    }

    // Start the initialisation
    if (Initialize()) {

        // Do the real work.
        // When the Run function returns, the service has stopped.
        m_Status.dwWin32ExitCode = 0;
        m_Status.dwCheckPoint = 0;
        m_Status.dwWaitHint = 0;
        Run(true);
    }

    // Tell the service manager we have stopped
    SetStatus(SERVICE_STOPPED);
}


/*==========================================================================*
 *  Function: StartTheService
 *
 *  Purpose:
 *
 *  Start the Service and do all necessaries functions and tests
 *
 *==========================================================================*/
BOOL StartTheService ( void )
{
    SERVICE_TABLE_ENTRY st[] =
    {
      {
        (LPTSTR )SERVICENAME,
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

    return StartServiceCtrlDispatcher(st);
}
/*==========================================================================*
 *  Function: Install
 *
 *  Purpose:
 *
 *  Install the service in the Service Control Manager.
 *
 * Returns TRUE if OK, FALSE if not
 *
 *==========================================================================*/

BOOL Install()
{
    char szFilePath[_MAX_PATH];
    SC_HANDLE hService ;
    SC_HANDLE hSCM ;
    char szKey[256];
    HKEY hKey = NULL;
    DWORD dwData;
    OSVERSIONINFO vi;

    // Open the Service Control Manager
    hSCM = OpenSCManager(NULL, // local machine
                         NULL, // ServicesActive database
                         SC_MANAGER_ALL_ACCESS); // full access
    if (!hSCM)
      return FALSE;

    // Get the executable file path
    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

    // Create the service
    hService = CreateService(hSCM,
                             SERVICENAME ,
                             SERVICENAME ,
                             SERVICE_ALL_ACCESS,
                             SERVICE_WIN32_OWN_PROCESS,
                             SERVICE_AUTO_START,        // start condition
                             SERVICE_ERROR_NORMAL,
                             szFilePath,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);

    Info.lpDescription = SERVICEDESCRIPTION ;


    // Info is only setable from NT 5 up (Win2K)
    vi.dwOSVersionInfoSize = sizeof(vi);
    GetVersionEx(&vi);              // get version with extended api
    if ( vi.dwMajorVersion >= 5 )
    {
      // NT version 5 ( Win2000) Set Description
      ChangeServiceConfig2(hService,
                           SERVICE_CONFIG_DESCRIPTION,
                           &Info );
    }




    if (!hService)
    {
        CloseServiceHandle(hSCM);
        return FALSE;
    }

    // make registry entries to support logging messages
    // Add the source name as a subkey under the Application
    // key in the EventLog service portion of the registry.
    strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
    strcat(szKey, SERVICENAME);
    if (RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return FALSE;
    }

    // Add the Event ID message-file name to the 'EventMessageFile' subkey.
    RegSetValueEx(hKey,
                  "EventMessageFile",
                  0,
                  REG_EXPAND_SZ,
                  (CONST BYTE*)szFilePath,
                  (DWORD)strlen(szFilePath) + 1);

    // Set the supported types flags.
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    RegSetValueEx(hKey,
                  "TypesSupported",
                  0,
                  REG_DWORD,
                  (CONST BYTE*)&dwData,
                  sizeof(DWORD));
    RegCloseKey(hKey);

    // tidy up
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);
    return TRUE;
}

/*==========================================================================*
 *  Function: Uninstall
 *
 *  Purpose:
 *
 *  Removes the Service in the Service Control Manager.
 *
 * Returns TRUE if OK, FALSE if not
 *
 *==========================================================================*/
BOOL Uninstall()
{
  BOOL bResult = FALSE ;
  SC_HANDLE hService ;
  SC_HANDLE hSCM ;

  // Open the Service Control Manager
  hSCM = OpenSCManager(NULL, // local machine
                       NULL, // ServicesActive database
                       SC_MANAGER_ALL_ACCESS); // full access
  if (!hSCM)
    return FALSE;

  hService = OpenService(hSCM,
                         SERVICENAME,
                         DELETE);
  if (hService)
  {
    if (DeleteService(hService))
    {
      bResult = TRUE;
    }
     CloseServiceHandle(hService);
  }

    CloseServiceHandle(hSCM);
    return bResult;
}

/*==========================================================================*
 *  Function: IsInstalled
 *
 *  Purpose:
 *
 *  Test if the service is currently installed and not disabled.
 *
 * Returns TRUE if installed, FALSE if not installed or installed but
 * disabled.
 *
 *==========================================================================*/
BOOL IsInstalled(void)
{
    LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;
    DWORD BytesNeeded ;  // address of variable for bytes needed
    BOOL bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
    if ( hSCM )
    {
        // Try to open the service
        SC_HANDLE hService = OpenService(hSCM, SERVICENAME, SERVICE_QUERY_CONFIG);
        if ( hService )
        {
            // The service is installed, make sure it is not currently disabled.
            lpServiceConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, 4096);
            if ( lpServiceConfig )
            {
                if ( QueryServiceConfig(hService, lpServiceConfig, 4096, &BytesNeeded) )
                {
                    if ( lpServiceConfig->dwStartType != SERVICE_DISABLED )
                    {
                        bResult = TRUE;
                    }
                }
                LocalFree(lpServiceConfig);
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCM);
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// Routines to run RXAPI as an Service BEGIN




/*==========================================================================*
 *  Function: ParseStandardArgs
 *
 *  Purpose:
 *
 *  Parse the cmdlinearguments, if any, and handle.
 *
 * Returns TRUE if it found an arg it recognised, FALSE if not
 * Note: processing some arguments causes output to stdout to be generated.
 *
 *==========================================================================*/
BOOL ParseStandardArgs( LPSTR lpCmdLine )
{
    char chString[255] ;
    BOOL bRet ;
    DWORD dwRet ;
    int iRet ;
    BOOL bVersionInfo  = FALSE ;
    BOOL bInstall  = FALSE ;
    BOOL bUninstall  = FALSE ;
    BOOL bSilent  = FALSE ;


   // See if we have any command line args we recognise
    if (strlen ( lpCmdLine ) == 0 )
    {
      if ( IsInstalled() )  // As service installed ??
      {
        bRet = StartTheService ( ) ; // we return control, if Service has ended.
        if ( !bRet )
        {
          // Error, see the reason
          dwRet = GetLastError ( );
          if (dwRet == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
          {
            // Not correct started, use NET START RXAPI,
            // to allow correct connection to service controller
            iRet = system("NET start rxapi") ;
          }
        return FALSE ;   // Do NOT Continue the program
        }
      }
      // either NOT installed as Service, or Service is set to DISABLED
      // Continue as standard RXAPI.EXE
      return TRUE;       // Continue as standard RXAPI.EXE
    }


    // ADD SILENT PARMS
    if ( lpCmdLine[0] == '/' )
    {
      // must be parameters, set BOOLeans for them
      if ( strchr( lpCmdLine , (int) 'v' ) != NULL )    // show Version
        bVersionInfo = TRUE ;
      if ( strchr( lpCmdLine , (int) 'V' ) != NULL )    // show Version
        bVersionInfo = TRUE ;
      if ( strchr( lpCmdLine , (int) 'i' ) != NULL )    // Install as service
        bInstall = TRUE ;
      if ( strchr( lpCmdLine , (int) 'I' ) != NULL )    // Install as service
        bInstall = TRUE ;
      if ( strchr( lpCmdLine , (int) 'u' ) != NULL )    // Uninstall as service
        bUninstall = TRUE ;
      if ( strchr( lpCmdLine , (int) 'U' ) != NULL )    // Uninstall as service
        bUninstall = TRUE ;
      if ( strchr( lpCmdLine , (int) 's' ) != NULL )    // Silent install as service, no messages
        bSilent = TRUE ;
      if ( strchr( lpCmdLine , (int) 'S' ) != NULL )    // Silent install as service, no messages
        bSilent = TRUE ;
      if ( strchr( lpCmdLine , (int) 'q' ) != NULL )    // Silent install as service, no messages
        bSilent = TRUE ;
      if ( strchr( lpCmdLine , (int) 'Q' ) != NULL )    // Silent install as service, no messages
        bSilent = TRUE ;



      if ( bVersionInfo )
      {
        // Spit out version info
        sprintf(chString ,"%s Version %d.%d.%d\nThe service is %s installed",
               SERVICENAME, MAJORVERSION, MINORVERSION, SUBVERSION, IsInstalled() ? "currently" : "not");
        MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONINFORMATION ) ;
        return FALSE ; // say we processed the argument
      } // If CmdLine includes /v

      if ( bInstall )
      {
        // Request to install.
        if (IsInstalled())
        {
          if (!bSilent )
          {
            sprintf( chString, "%s is already installed.", SERVICENAME);
            MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONINFORMATION ) ;
          }
        }
        else
        {
          // Try and install the copy that's running
          if (Install())
          {
            if (!bSilent )
            {
              sprintf( chString, "%s installed\n", SERVICENAME);
             MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONINFORMATION ) ;
            }
          }
          else
          {
            if (!bSilent )
            {
              printf( chString, "%s failed to install. Error %d\n", SERVICENAME, GetLastError());
              MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONERROR ) ;
            }
          }
        }
        return FALSE ; // say we processed the argument
      } // If CmdLine includes /i

      if ( bUninstall )
      {
        // Request to uninstall.
        if (!IsInstalled())
        {
          if (!bSilent )
          {
            sprintf( chString, "%s is not installed\n", SERVICENAME);
            MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONINFORMATION ) ;
          }
        }
        else
        {
          // Try and remove the copy that's installed
          if (Uninstall())
          {
            // Get the executable file path
            char szFilePath[_MAX_PATH];
            GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
            if (!bSilent )
            {
              sprintf( chString, "%s uninstalled as service.\n(Object REXX is NOT uninstalled.)",
                     SERVICENAME);
              MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONINFORMATION ) ;
            }
          }
          else
          {
            if (!bSilent )
            {
              sprintf( chString, "Could not remove %s. Error %d\n", SERVICENAME, GetLastError());
              MessageBox ( NULL , chString, SERVICENAME , MB_OK | MB_ICONERROR ) ;
            }
          }
        }
        return FALSE; // say we processed the argument
      } // If CmdLine includes /u
    }

    // Don't recognise the args
    // Spit out the help info
    if (!bSilent )
    {
      MessageBox ( NULL , "Available parms :\n/i : install as Service.\n\
/u : uninstall as Service.\n/v : show Version number\n\
an additional s installs silent.",
                 SERVICENAME , MB_OK | MB_ICONINFORMATION ) ;
    }

    return FALSE;
}

// Routines to run RXAPI as an Service END
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


/*==========================================================================*
 *  Function: WinMain
 *
 *  Purpose:
 *
 *  Main entry point. Register window classes, create windows,
 *  parse command line arguments and then perform a message loop
 *
 *==========================================================================*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
{
    OSVERSIONINFO version_info;
    // GetVersionEx called for the first time */
    version_info.dwOSVersionInfoSize = sizeof(version_info);
    GetVersionEx(&version_info);

    if (version_info.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS)
    {
        // Test Commandline for parameters
        if ( !ParseStandardArgs( lpCmdLine ) )
        {
            exit (0) ;                 // Stop RXAPI
        }
    }

    // Main portion is moved to RUN() function, since it is global
    // for standard and service invocation.
    Run(false);

    return 0;
}

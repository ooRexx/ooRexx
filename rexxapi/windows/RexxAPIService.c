/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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

#include "rexx.h"
#include "orxgui.h"
#include "string.h"
#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "RexxAPIService.h"
#include "APIServiceMessages.h"
#include "RexxAPIManager.h"
#include "SystemVersion.h"
#include "APIServiceSystem.h"

#include "RexxService.h"
#define  REGSUBCOMM 0
// Globals

char g_szAppName[] = REXXAPI_WINDOW_CLASS;     // The name of this application
char g_szAppTitle[80] = REXXAPI_WINDOW_TITLE;  // title bar text
char g_OrxParmBuf[MAX_PATH];                   // Parameters (from cmd ln, dbox...

// Globals for Service

static SERVICE_STATUS_HANDLE m_hServiceStatus;
static SERVICE_STATUS m_Status ;
static BOOL m_bIsRunning ;
static SERVICE_DESCRIPTION Info;

// End Globals for Service RIN0006


/* module static data -------------------------------------------------*/

#undef WINDOWED_APP /* define WINDOWED_APP if you want RXAPI to create a window */
#ifdef WINDOWED_APP

CHAR szNotepadBuf[MAX_PATH];   // Holds notepad command and fn
LPORXWDATA lpWdata;            // Orexx data structure in window extra bytes
HMENU hMenu;                   // handle to the menu
HWND hEditChild;               // edit window handle

HINSTANCE   g_hinst = NULL;
HINSTANCE   g_hinstPrev = NULL;
LPCSTR      g_lpCmdLine = NULL;
int         g_nCmdShow  = 0;
HMODULE     g_hmod  = NULL;
HANDLE      g_htask = NULL;
HWND        RxAPI_hwndFrame = NULL;
#endif

HANDLE      APIMutex[6] = {NULL};
HANDLE      APIExistenceMutex = NULL;    /* must be a mutex that is not shared but only owned by RXAPI.EXE */

extern __declspec(dllimport) REXXAPIDATA * RexxinitExports;      /* Global state data          */
HANDLE Rx_Map = NULL;
extern __declspec(dllimport) CRITICAL_SECTION nest;

extern __declspec(dllimport) LONG APIexecheck(PSZ, LONG, DWORD );
extern __declspec(dllimport) LONG APIdllcheck(PAPIBLOCK, LONG);
extern __declspec(dllimport) LONG APIregdrop(PSZ, PSZ, LONG, DWORD);
extern __declspec(dllimport) PAPIBLOCK APIsearch(PSZ, PSZ, LONG, DWORD);

extern _declspec(dllimport) RexxReturnCode APIAddQueue(void);
extern _declspec(dllimport) RexxReturnCode APIPullQueue(void);
extern _declspec(dllimport) PQUEUEHEADER APICreateQueue(process_id_t Pid, BOOL newProcess);
extern _declspec(dllimport) size_t APISessionQueue(process_id_t Pid, BOOL newProcess);
extern _declspec(dllimport) size_t APIDeleteQueue(process_id_t Pid, BOOL SessionQ);
extern _declspec(dllimport) size_t APIQueryQueue();
extern _declspec(dllimport) LONG addPID(PAPIBLOCK cblock, process_id_t processID);
extern _declspec(dllimport) LONG removePID(PAPIBLOCK cblock, process_id_t processID);
extern _declspec(dllimport) char** APInamedObjects;

/* the name_size had to be changed because of win95 using high session IDs */
#define INTERNAL_NAME_SIZE 20

extern _declspec(dllimport) SECURITY_DESCRIPTOR SD_NullAcl;      /* let RXAPI.EXE share its null ACL */

BOOL AllocComBlock(int chain, size_t size, size_t modifer, SECURITY_ATTRIBUTES * sa);
void FreeComBlock(int chain);

extern _declspec(dllimport) RexxReturnCode APIAddMacro(BOOL updateIfExists);
extern _declspec(dllimport) RexxReturnCode APIDropMacro(void);
extern _declspec(dllimport) RexxReturnCode APIClearMacroSpace(void);
extern _declspec(dllimport) RexxReturnCode APIQueryMacro(void);
extern _declspec(dllimport) RexxReturnCode APIReorderMacro(void);
extern _declspec(dllimport) RexxReturnCode APIExecuteMacroFunction(void);
extern _declspec(dllimport) RexxReturnCode APIList(int kind);

static BOOL API_Stopped = FALSE;

#define REUSE_SYSTEM_OBJECTS


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// Routines to run RXAPI as an Service BEGIN

////////////////////////////////////////////////////////////////////////////////////////////
// Debugging support

void __cdecl DebugMsg(const char* pszFormat, ...)
{
  FILE *stream;
  va_list arglist;
  char buf[1024];
  sprintf(buf, "[%s](%lu){%d}: ", SERVICENAME, GetCurrentThreadId(),m_Status.dwCurrentState);
  va_start(arglist, pszFormat);
  vsprintf(&buf[strlen(buf)], pszFormat, arglist);
  va_end(arglist);
  strcat(buf, "\n");

  OutputDebugString(buf);
  stream = fopen("C:\\apilog.txt","a+");
  if ( stream )
  {
    fwrite ( buf, 1, strlen(buf), stream ) ;
    fclose ( stream ) ;
  }

}



//////////////////////////////////////////////////////////////////////////////////////
// Control request handlers


// Called when the service is first initialized
BOOL OnInit()
{
//  DebugMsg( "OnInit" ) ;
        return TRUE;
}

// Called when the service control manager wants to stop the service
void OnStop()
{
//  DebugMsg( "OnStop" ) ;
   m_bIsRunning = FALSE;
   API_Stopped = TRUE;   /* stop message handling */
}

// called when the service is interrogated
void OnInterrogate()
{
//  DebugMsg( "OnInterrogate" ) ;
}

// called when the service is paused
void OnPause()
{
//  DebugMsg( "OnPause" ) ;
}

// called when the service is continued
void OnContinue()
{
//  DebugMsg( "OnContinue" ) ;
}

// called when the service is shut down
void OnShutdown()
{
//  DebugMsg( "OnShutdown" ) ;
}

// called when the service gets a user control message
BOOL OnUserControl(DWORD dwOpcode)
{
//    DebugMsg("OnUserControl(%8.8lXH)", dwOpcode);
    return FALSE; // say not handled
}


///////////////////////////////////////////////////////////////////////////////////////////
// status functions

void SetStatus(DWORD dwState)
{
//    DebugMsg( "SetStatus. Status is : %d",dwState ) ;
    m_Status.dwCurrentState = dwState;
    SetServiceStatus(m_hServiceStatus, &m_Status);
}


// static member function (callback) to handle commands from the
// service control manager
static void __cdecl Handler(DWORD dwOpcode)
{

//static int count = 0 ;

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
        if (dwOpcode >= SERVICE_CONTROL_USER)
        {
          OnUserControl(dwOpcode) ;
        }
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
//  DebugMsg( "Initialize" ) ;
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
//        DebugMsg ( "Service is stopped" ) ;
        return FALSE;
    }
//    DebugMsg ( "Service is running" ) ;

    SetStatus(SERVICE_RUNNING);

    return TRUE;
}



int Run ( void ) ; // prototype main function
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


//DebugBreak();


//  DebugMsg( "ServiceMain" ) ;

  // Register the control request handler
    m_Status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(SERVICENAME,
                                                  (LPHANDLER_FUNCTION )Handler);
    if (m_hServiceStatus ==  (SERVICE_STATUS_HANDLE )NULL)
    {
//      DebugMsg ( "m_hServiceStatus is NULL");
        return;
    }

    // Start the initialisation
    if (Initialize()) {

        // Do the real work.
        // When the Run function returns, the service has stopped.
        m_bIsRunning = TRUE;
        m_Status.dwWin32ExitCode = 0;
        m_Status.dwCheckPoint = 0;
        m_Status.dwWaitHint = 0;
        Run( );
//DebugMsg("Run ended");
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
    m_bIsRunning = FALSE;

//    DebugMsg( "StartTheService" ) ;

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
    char szDescription[256];
    HKEY hKey = NULL;
    DWORD dwData;
    OSVERSIONINFO vi;

//  DebugMsg( "Install" ) ;
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
    sprintf( szDescription, SERVICEDESCRIPTION, SERVICENAME,  MAJORVERSION, MINORVERSION, SUBVERSION, BUILDVERSION);
    Info.lpDescription = szDescription ;


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
                  (const BYTE*)szFilePath,
                  (DWORD)strlen(szFilePath) + 1);

    // Set the supported types flags.
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    RegSetValueEx(hKey,
                  "TypesSupported",
                  0,
                  REG_DWORD,
                  (const BYTE*)&dwData,
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
  //char chString[255] ;
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
 *  Test if the service is currently installed
 *
 * Returns TRUE if installed, FALSE if not
 * Note: processing some arguments causes output to stdout to be generated.
 *
 *==========================================================================*/
BOOL IsInstalled(void)
{
  LPQUERY_SERVICE_CONFIG lpServiceConfig;
  DWORD BytesNeeded ;  // address of variable for bytes needed
  BOOL bResult = FALSE;

    // Open the Service Control Manager
    SC_HANDLE hSCM = OpenSCManager(NULL, // local machine
                                   NULL, // ServicesActive database
                                   SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM)
    {
        // Try to open the service
        SC_HANDLE hService = OpenService(hSCM,
                                         SERVICENAME,
                                         SERVICE_QUERY_CONFIG);
        if (hService)
        {
          // Query Service Configuration
          lpServiceConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc( LPTR, 4096);
          if ( QueryServiceConfig( hService,       // handle of service
                                   lpServiceConfig, // address of service config. structure
                                   (DWORD)4096,     // size of service configuration buffer
                                   &BytesNeeded  )  )  // address of variable for bytes needed
          {
            // See, if Service is NOT DISABLED ??
            if (lpServiceConfig->dwStartType != SERVICE_DISABLED)
            {
              bResult = TRUE;
            }
          }
          CloseServiceHandle(hService);
        }

        CloseServiceHandle(hSCM);
    }

    return bResult;
}

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
        sprintf(chString ,"%s Version %d.%d.%d.%d\nThe service is %s installed",
               SERVICENAME, MAJORVERSION, MINORVERSION, SUBVERSION, BUILDVERSION, IsInstalled() ? "currently" : "not");
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
              sprintf( chString, "%s failed to install. Error %d\n", SERVICENAME, GetLastError());
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
              sprintf( chString, "%s uninstalled as service.\n(Open Object Rexx is NOT uninstalled.)",
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


BOOL WaitForAPIMessage()
{
    ULONG rc;
#ifdef WINDOWED_APP
    MSG msg;
#endif


    /* wait in intervals so shutdown request can be processed */
    if (!API_Stopped || m_bIsRunning)  // m_bIsRunning flag signals, that the rxapi is running as an SERVICE
    do {

#ifdef WINDOWED_APP
    /* it is recommended to use MsgWaitForMultipleObjects in windowed apps but using this
       with QS_ALLINPUT performes worse (5000 function registrations require about 60 secs
       instead of 45-50 secs) */
        if ((rc = WaitForSingleObject(APIMutex[API_MSGEVENT], 30)) == WAIT_ABANDONED)
            return FALSE;      /* should never happen */
        if (rc == WAIT_TIMEOUT)
            /* if no message loop is provided, other apps like InstallShield may hang */
            // m_bIsRunning flag signals, that the rxapi is running as an SERVICE
            while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) && (!API_Stopped || m_bIsRunning))
            {
                if (msg.message == WM_QUIT) &&
                   ( !m_bIsRunning)               // m_bIsRunning flag signals, that the rxapi is running as an SERVICE
                {
                    PostQuitMessage(msg.wParam);
                    API_Stopped = TRUE;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
#else
        if ((rc = WaitForSingleObject(APIMutex[API_MSGEVENT], 1000)) == WAIT_ABANDONED)
            return FALSE;
#endif
    }
    while ((rc != WAIT_OBJECT_0) && (!API_Stopped || m_bIsRunning) );
    ResetEvent(APIMutex[API_MSGEVENT]);
    if (API_Stopped && !m_bIsRunning) return FALSE;
    else return TRUE;
}

#define APIRETURN(ares) {\
    RX.msg.result = ares;\
    RX.msg.done = TRUE;\
    SetEvent(APIMutex[API_RESULTEVENT]);}


#ifdef WINDOWED_APP
VOID APIWin_OnDestroy(HWND hWnd)
{
    API_Stopped = TRUE;   /* stop message handling */
    RX.msg.message = RXAPI_TERMINATE;
    SetEvent(APIMutex[API_MSGEVENT]);  /* do this to exit semaphore wait */
}



/*==========================================================================*
 *
 *       FUNCTION: APIWin_WndProc(.........)
 *
 *       PURPOSE: Initializes window data and registers window class
 *
 *==========================================================================*/
LRESULT CALLBACK APIWin_WndProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_QUIT, APIWin_OnDestroy);
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
#endif


/*==========================================================================*
 *
 *       FUNCTION: APIMessageHandler()
 *
 *       PURPOSE: Own message handler for interprocess communication
 *
 *==========================================================================*/
BOOL APIMessageHandler()
{
  LRESULT result = 0;
  UINT size = 0;
  LPVOID tmemPtr;
  LONG   type = 0;
  PAPIBLOCK cblock;

  while (WaitForAPIMessage())
  {
    result = 0;     /* important! otherwise result has old value */
    switch (RX.msg.message)
    {
        case RXAPI_REGREGISTER:
           type    = (LONG)RX.msg.lParam;

           tmemPtr = GlobalAlloc(GMEM_FIXED, ((PAPIBLOCK)RX.comblock[API_API])->apisize);
           if (!tmemPtr) result = RXSUBCOM_NOEMEM;
           else {
               memcpy(tmemPtr, RX.comblock[API_API], ((PAPIBLOCK)RX.comblock[API_API])->apisize);

               cblock = (PAPIBLOCK)tmemPtr;

               EnterCriticalSection(&nest);
               cblock->next =                 /* Next pointer set to old top*/
                      RX.baseblock[type];
               RX.baseblock[type] = cblock;   /* Make this block the top    */
               if (type != REGSUBCOMM) {
                 addPID(cblock, cblock->apipid);/* list with 1 entry*/
               }
               LeaveCriticalSection(&nest);
           }
           APIRETURN(result);
           break;
#ifdef UPDATE_ADDRESS
        case RXAPI_REGUPDATE:
           type    = (LONG)RX.msg.lParam;

           /* get address of current pointer from the end of communication page */
           cblock = (PAPIBLOCK)(((RXREG_TALK *)RX.comblock[API_API])->curAPI);
           if (!cblock) result = RXSUBCOM_NOEMEM;
           else memcpy(cblock, RX.comblock[API_API], cblock->apisize);
           APIRETURN(result);
           break;
#endif
        case RXAPI_REGQUERY:
           type    = (LONG)RX.msg.lParam;
           cblock = (PAPIBLOCK)RX.comblock[API_API];

           cblock = APIsearch(APIBLOCKNAME(cblock),
                           APIBLOCKDLLNAME(cblock),
                           type,
                           (DWORD)(cblock->apipid));
           result = (LRESULT)cblock;
           APIRETURN(result);
           break;
        case RXAPI_REGDEREGISTER:
           type    = (LONG)RX.msg.lParam;

           cblock = (PAPIBLOCK)RX.comblock[API_API];

           result = (LRESULT)APIregdrop(APIBLOCKNAME(cblock),
                                     APIBLOCKDLLNAME(cblock),
                                     type,
                                     (DWORD)(cblock->apipid));

           APIRETURN(result);
           break;
        case RXAPI_PROCESSGONE:
          // remove the given PID from all APIBLOCK PID lists
          // this is called when a REXX process ends (via DllMain - DLL_DETACH)
          type   = (LONG)RX.msg.wParam;
          cblock = (PAPIBLOCK) RX.baseblock[type];
          if (type == REGSUBCOMM) {
            cblock = NULL;
          }
          while (cblock) {
            // remove this PID as using any block that
            // is registered
            removePID(cblock, (process_id_t) RX.msg.lParam);
            cblock = cblock->next;
          }
          APIRETURN(0);
          break;
        case RXAPI_REGCHECKDLL:
           type    = (LONG)RX.msg.lParam;

           cblock = (PAPIBLOCK)RX.comblock[API_API];
           result = (LRESULT)APIdllcheck(cblock, type);
           APIRETURN(result);
           break;
        case RXAPI_REGCHECKEXE:
           type    = (LONG)RX.msg.lParam;

           cblock = (PAPIBLOCK)RX.comblock[API_API];

           result = (LRESULT)APIexecheck(APIBLOCKNAME(cblock),
                                      type,
                                      (DWORD)(cblock->apipid));
           APIRETURN(result);
           break;
        case RXAPI_QUEUECREATE:
           // a non-null pointer is success, so we need to return zero for that.
           result = APICreateQueue(0, FALSE) == NULL;  /* create a named queue */
           APIRETURN(result);
           break;
        case RXAPI_QUEUEDELETE:
           result = APIDeleteQueue((process_id_t)RX.msg.wParam, FALSE);  /* delete a named queue */
           APIRETURN(result);
           break;
        case RXAPI_QUEUEQUERY:
           result = APIQueryQueue();
           APIRETURN(result);
           break;
        case RXAPI_QUEUECOMEXTEND:
           {
               SECURITY_ATTRIBUTES sa;
               size_t size = RX.msg.wParam / PAGE_SIZE + 1;
               FreeComBlock(API_QUEUE);
               result = !AllocComBlock(API_QUEUE, size * PAGE_SIZE, size, SetSecurityDesc(&sa));
               RX.comblockQueue_ExtensionLevel = size;
           }
           APIRETURN(result);
           break;
        case RXAPI_QUEUEADD:
           result = APIAddQueue();
           APIRETURN(result);
           break;
        case RXAPI_QUEUEPULL:
           result =  APIPullQueue();
           APIRETURN(result);
           break;
        case RXAPI_QUEUESESSION:
           result =  APISessionQueue((process_id_t)RX.msg.wParam, (BOOL)RX.msg.lParam);  /* search and evtl. create a session queue */
           APIRETURN(result);
           break;
        case RXAPI_QUEUESESSIONDEL:
           result = APIDeleteQueue((process_id_t)RX.msg.wParam, TRUE);  /* delete a session queue */
           APIRETURN(result);
           break;
        case RXAPI_MACROCOMEXTEND:
           {
               SECURITY_ATTRIBUTES sa;
               size_t size = RX.msg.wParam / PAGE_SIZE + 1;
               FreeComBlock(API_MACRO);
               result = !AllocComBlock(API_MACRO, size * PAGE_SIZE, size, SetSecurityDesc(&sa));
               RX.comblockMacro_ExtensionLevel = size;
           }
           APIRETURN(result);
           break;
        case RXAPI_MACRO:
           switch (RX.msg.wParam)
           {
               case MACRO_ADD:result = APIAddMacro((BOOL)RX.msg.lParam);
                    break;
               case MACRO_DROP:result = APIDropMacro();
                    break;
               case MACRO_CLEAR:result = APIClearMacroSpace();
                    break;
               case MACRO_QUERY:result = APIQueryMacro();
                    break;
               case MACRO_REORDER:result = APIReorderMacro();
                    break;
               case MACRO_EXECUTE:result = APIExecuteMacroFunction();
                    break;
               case MACRO_LIST:result = APIList((int)RX.msg.lParam);
                    break;
               default: result = 2;  /* not found */
           }
           APIRETURN(result);
           break;
        case RXAPI_SHUTDOWN:
            {
              HANDLE   orexx_active_sem;

              result = FALSE;
              if (!m_bIsRunning) // RIN006 m_bIsRunning flag signals, that the rxapi is running as an SERVICE
              {

               if ((APIDeleteQueue((process_id_t)RX.msg.wParam, TRUE)<2) && (!RX.base))    /* delete a session queue */
                  RxFreeProcessSubcomList((process_id_t)RX.msg.wParam);

               /* check if there's still another rexx program running that needs RXAPI */
               orexx_active_sem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "OBJECTREXX_RUNNING");
               if (orexx_active_sem) CloseHandle(orexx_active_sem);
               else
               {

                  if ((!RX.base)
                   && (!RX.session_base)
                  && (!RX.baseblock[0])
                     && (!RX.baseblock[1])
                     && (!RX.baseblock[2])
                     && (!RX.macrobase))       /* otherwise macrospace is ignored */
                  {
//DebugMsg( "2 API_Stopped : %d; mbIsRunning : %d", API_Stopped, m_bIsRunning) ;
                      result = TRUE;
                     API_Stopped = TRUE;
                  }
#if 0
                  else
                  {
                        CHAR shtxt[256];
                         sprintf(shtxt,"Base: %x, SessBase: %x, BB1: %x, BB2: %x, BB3: %x",
                         RX.base, RX.session_base, RX.baseblock[0], RX.baseblock[1], RX.baseblock[2]);

                      MessageBox(NULL, shtxt, "ShutDown", MB_OK);
                   }
#endif
               }
              }
            }
            APIRETURN(result);
             break;
        case RXAPI_PROCESSCLEANUP:
           RxFreeProcessSubcomList((process_id_t)RX.msg.lParam);
           APIRETURN(0);
           break;

        case RXAPI_TERMINATE: return TRUE;  /* stop immediately */

    default:APIRETURN(0);
    }
  }
//DebugMsg( "3 API_Stopped : %d; mbIsRunning : %d", API_Stopped, m_bIsRunning) ;

  if (API_Stopped && !m_bIsRunning) return TRUE; // m_bIsRunning flag signals, that the rxapi is running as an SERVICE
  else return FALSE;   /* something wnt wrong */
}



#ifdef WINDOWED_APP
/*==========================================================================*
 *
 *       FUNCTION: APIWin_Register(HINSTANCE)
 *
 *       PURPOSE: Initializes window data and registers window class
 *
 *==========================================================================*/
BOOL APIWin_Register(HINSTANCE hInstance)
{

    WNDCLASS  wc;

    // Fill in window class structure with parameters that describe the
    // main window.
    wc.style         = CS_HREDRAW | CS_VREDRAW;    // Class style(s).
    wc.lpfnWndProc   = (WNDPROC)APIWin_WndProc;    // Window Procedure
    wc.cbClsExtra    = 0;                          // No per-class extra data.
    wc.cbWndExtra    = CBWNDEXTRA;                 // Per-window extra data!
    wc.hInstance     = g_hinst;                    // Owner of this class
    wc.hIcon         = LoadIcon (g_hinst,"RXAPICO");// Icon name from .RC
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);// Cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);   // Default color
    wc.lpszMenuName  = NULL;                       // Menu name from .RC
    wc.lpszClassName = g_szAppName;                // Name to register as

    // Register the window class and return success/failure code.
     return (RegisterClass(&wc));

}
#endif


BOOL AllocComBlock(int chain, size_t size, size_t modifier, SECURITY_ATTRIBUTES * sa)
{
    // The com blocks used for the QUEUE and MACROSPACE apis need to
    // be reallocatable.  Because we're using named memory, this requires
    // that the server process AND all of the client processes close
    // the named memory segment before we can allocate a new segment
    // with the same name.  This is generally difficult (if not impossible)
    // to implement, so we get around the problem by using named memory
    // segments that incorporate the extension size in the name so that
    // we avoid conflicts between the new and old segments.
    char mapName[256];

    if (chain == API_QUEUE || chain == API_MACRO)
    {
        sprintf(mapName, "%s%u", FMAPNAME_COMBLOCK(chain), modifier);
    }
    else
    {
        strcpy(mapName, FMAPNAME_COMBLOCK(chain));
    }

    RX.comhandle[chain] = CreateFileMapping(INVALID_HANDLE_VALUE, sa,
                                     PAGE_READWRITE, 0, (DWORD)size, mapName);
    if (!RX.comhandle[chain])
        return FALSE;

    RX.comblock[chain] = MapViewOfFile(RX.comhandle[chain],FILE_MAP_WRITE,0,0,0);
    if (!RX.comblock[chain])
    {
        CloseHandle(RX.comhandle[chain]);
        return FALSE;
    }
    return TRUE;
}

void FreeComBlock(int chain)
{
    if (RX.comblock[chain]) UnmapViewOfFile(RX.comblock[chain]);
    if (RX.comhandle[chain]) CloseHandle(RX.comhandle[chain]);
}


/*==========================================================================*
 *
 *       FUNCTION: InitApplication(HINSTANCE)
 *
 *       PURPOSE: Initializes window data and registers window class
 *
 *==========================================================================*/
BOOL App_Initialize(void)
{
    SECURITY_ATTRIBUTES sa;
    int i;
#ifdef REUSE_SYSTEM_OBJECTS
    BOOL fMapReused = FALSE;
#else
    HANDLE mtx;
#endif

    SetSecurityDesc(&sa);

    /* The existence mutex is not owned by any other process and therefore will be
    closed when RXAPI.EXE is killed */
    APIExistenceMutex = CreateMutex(&sa, TRUE, MUTEXNAME_EXISTENCE);
    if (!APIExistenceMutex || (GetLastError() == ERROR_ALREADY_EXISTS))
        return FALSE;

#ifndef REUSE_SYSTEM_OBJECTS
    /* already initialized but not existing any more (detached process hanging around) */
    if ((mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEXNAME_API)) != NULL)
    {
        CloseHandle(mtx);
        MessageBox(NULL, "Could not start Open Object Rexx API Manager due to a Rexx process "\
            "that runs in an incorrect state!\nTerminate this process before starting "\
            "any other Open Object Rexx program.", "Startup Error!", MB_OK | MB_SYSTEMMODAL);
        return FALSE;
    }
#endif

    /* need this on NT to make OpenProcess and DuplicateHandle work
       when RXAPI is started from a different user */
    if (!RUNNING_95)
        SetKernelObjectSecurity(GetCurrentProcess(),    // handle of object
                        DACL_SECURITY_INFORMATION,    // type of information to set
                        &SD_NullAcl);

#ifdef WINDOWED_APP     /* we don't use a window anymore but have our own MySendMessage (due to Services) */
    if (!APIWin_Register(g_hinst))
        return FALSE;
#endif


    /* Create a memory mapped file to share RexxinitExports */
    Rx_Map = CreateFileMapping(INVALID_HANDLE_VALUE, &sa,
                                    PAGE_READWRITE, 0, sizeof(REXXAPIDATA), FMAPNAME_INITEXPORTS);
    if (!Rx_Map)
        return FALSE;

#ifdef REUSE_SYSTEM_OBJECTS
    fMapReused = (GetLastError() == ERROR_ALREADY_EXISTS);
#endif

    RexxinitExports = MapViewOfFile(Rx_Map,FILE_MAP_WRITE,0,0,0);
    if (!RexxinitExports)
    {
        CloseHandle(Rx_Map);
        return FALSE;
    }

#ifdef REUSE_SYSTEM_OBJECTS
    if (fMapReused)
        ZeroMemory(RexxinitExports, sizeof(REXXAPIDATA));
#endif
    RX.UID = 0;

    /* open the 3 required mutex semaphores for serialization */
    APIMutex[API_API] = CreateMutex(&sa, FALSE, MUTEXNAME_API);
#ifndef REUSE_SYSTEM_OBJECTS
    if (!APIMutex[API_API] || (GetLastError() == ERROR_ALREADY_EXISTS))
#else
    if (!APIMutex[API_API])
#endif
        return FALSE;

    APIMutex[API_QUEUE] = CreateMutex(&sa, FALSE, MUTEXNAME_QUEUE);
    if (!APIMutex[API_QUEUE])
    {
        CloseHandle(APIMutex[API_API]);
        return FALSE;
    }

    APIMutex[API_MACRO] = CreateMutex(&sa, FALSE, MUTEXNAME_MACRO);
    if (!APIMutex[API_MACRO])
    {
        for (i = 0; i<2; i++) CloseHandle(APIMutex[i]);
        return FALSE;
    }

    /* open the 2 message semaphores */
    APIMutex[API_MESSAGE] = CreateMutex(&sa, FALSE, MUTEXNAME_MESSAGE);
    if (!APIMutex[API_MESSAGE])
    {
        for (i = 0; i<3; i++) CloseHandle(APIMutex[i]);
        return FALSE;
    }

    APIMutex[API_MSGEVENT] = CreateEvent(&sa, FALSE, FALSE, MUTEXNAME_MSGEVENT);  /* no auto reset event */
    if (!APIMutex[API_MSGEVENT])
    {
        for (i = 0; i<4; i++) CloseHandle(APIMutex[i]);
        return FALSE;
    }

    APIMutex[API_RESULTEVENT] = CreateEvent(&sa, FALSE, FALSE, MUTEXNAME_RESULTEVENT);  /* no auto reset event */
    if (!APIMutex[API_RESULTEVENT])
    {
        for (i = 0; i<5; i++) CloseHandle(APIMutex[i]);
        return FALSE;
    }


    if (!AllocComBlock(API_QUEUE, API_QUEUE_INITIAL_EXTLEVEL * PAGE_SIZE, API_QUEUE_INITIAL_EXTLEVEL, &sa))
    {
        for (i = 0; i<MUTEXCOUNT; i++) CloseHandle(APIMutex[i]);
        return FALSE;
    }

    RX.comblockQueue_ExtensionLevel = API_QUEUE_INITIAL_EXTLEVEL;


    if (!AllocComBlock(API_API, sizeof(RXREG_TALK), 0, &sa))
    {
        for (i = 0; i<MUTEXCOUNT; i++) CloseHandle(APIMutex[i]);
        FreeComBlock(API_QUEUE);
        return FALSE;
    }


    if (!AllocComBlock(API_MACRO, API_MACRO_INITIAL_EXTLEVEL * PAGE_SIZE, API_MACRO_INITIAL_EXTLEVEL, &sa))
    {
        for (i = 0; i<MUTEXCOUNT; i++) CloseHandle(APIMutex[i]);
        FreeComBlock(API_API);
        FreeComBlock(API_QUEUE);
        return FALSE;
    }

    RX.comblockMacro_ExtensionLevel = API_MACRO_INITIAL_EXTLEVEL;

#ifdef WINDOWED_APP     /* we don't use a window anymore */
    // Create a main window for this application instance.
    RxAPI_hwndFrame = CreateWindowEx(
        0L,                                // extendedStyle
        (LPCTSTR)g_szAppName,              // class name
        (LPCTSTR)g_szAppTitle,             // text
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,      // x, y
        CW_USEDEFAULT, CW_USEDEFAULT,      // cx, cy
        NULL,                              // hwndParent
        NULL,                              // hmenu
        g_hinst,                           // hInstance
        NULL);                             // lpParam

    // If window could not be created, return "failure"
    if (RxAPI_hwndFrame == NULL)
    {
        for (i = 0; i<MUTEXCOUNT; i++)
            CloseHandle(APIMutex[i]);
        for (i = 0; i<3; i++)
             FreeComBlock(i);
        return FALSE;
    }
#endif

    //InitStatBar();

    RX.MemMgrVersion = RXAPI_VERSION;
    RX.MemMgrPid = GetCurrentProcessId();
    RX.init = 1;
    RX.UID = GetTickCount();

#ifdef WINDOWED_APP
    ShowWindow(RxAPI_hwndFrame, SW_HIDE);
#endif

    return TRUE;
}



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
int Run ( void )
{
    BOOL code;
#if 0
    CHAR shtxt[256];
#endif
    int i;

//    DebugMsg ( "Run" ) ;
    /* Perform initializations that apply to a specific instance */
    if (!App_Initialize())
    {
      if (APIExistenceMutex)
      {
          ReleaseMutex(APIExistenceMutex);   /* first release mutex  */
          CloseHandle(APIExistenceMutex);
      }

      if (RexxinitExports)
      {
          RX.init = -1;
          UnmapViewOfFile(RexxinitExports);
          if (Rx_Map) CloseHandle(Rx_Map);
      }
      return -1;
    }

    if (APIExistenceMutex) ReleaseMutex(APIExistenceMutex);   /* now release mutex so that waiting REXX process can continue */

    if (!nest.DebugInfo) InitializeCriticalSection(&nest);
#if 0
    sprintf(shtxt,"CritSec: debug: %x lockc: %d recc: %d locksem: %x  owner: %x res: %d", nest.DebugInfo,
                   nest.LockCount,nest.RecursionCount, nest.OwningThread,nest.LockSemaphore,nest.Reserved);
    MessageBox(NULL, shtxt, "Critical Section", MB_OK);
#endif


    code = APIMessageHandler();

    if (nest.DebugInfo) DeleteCriticalSection(&nest);
    if (RexxinitExports) {
        for (i = 0; i<MUTEXCOUNT; i++)
        {
            if (APIMutex[i]) CloseHandle(APIMutex[i]);
        }
        for (i = 0; i<NUMBEROFCOMBLOCKS; i++)
            FreeComBlock(i);

        RX.init = -1;
        RX.MemMgrPid = 0;
        UnmapViewOfFile(RexxinitExports);
        if (Rx_Map) CloseHandle(Rx_Map);
        RexxinitExports = NULL;
    }
    if (APIExistenceMutex) CloseHandle(APIExistenceMutex);

    if (code) return 0; else return -1;

}


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

#ifdef WINDOWED_APP
    g_hinst = hInstance;
    g_hinstPrev = hPrevInstance;
    g_lpCmdLine = lpCmdLine;
    g_nCmdShow  = nCmdShow;

    g_hmod  = GetInstanceModule(g_hinst);
    g_htask = GetCurrentThread();
#endif


    // Install as service for WINNT
    if (!RUNNING_95)
    {
      // Test Commandline for parameters
      if ( !ParseStandardArgs( lpCmdLine ) )
      {
        API_Stopped = TRUE;   /* stop message handling */
        exit (0) ;                 // Stop RXAPI
      }
    } // if (!RUNNING_95)


    // Main portion is moved to RUN() function, since it is global
    // for standard and service invocation.
    return ( Run ( ) ) ;
}




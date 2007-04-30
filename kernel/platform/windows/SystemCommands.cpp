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
/******************************************************************************/
/*  wincmd.c - C methods for handling calls to system exits and subcommand    */
/*             handlers.                                                      */
/*                                                                            */
/*  C methods:                                                                */
/*    SysExitHandler - Native method to invoke a system exit                  */
/*    SysCommand     - Method to invoke a subcommand handler                  */
/*                                                                            */
/*  Internal routines:                                                        */
/*    sys_command - Run a command through system command processor.           */
/******************************************************************************/
#include <string.h>                    /* Get strcpy, strcat, etc.       */

#include <process.h>
#include <stdlib.h>

#define INCL_RXSUBCOM                  /* Include subcom declares        */
#define INCL_RXFUNC                    /* and external function...       */
#define INCL_RXSYSEXIT                 /* and system exits               */
#define INCL_INTEGER
#define INCL_ARRAY

#include "RexxCore.h"                    /* global REXX declarations     */
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "RexxNativeAPI.h"             /* Lot's of useful REXX macros    */

#include SYSREXXSAA                    /* Include REXX header            */

#define CMDBUFSIZE32S 260              /* Max size of executable cmd     */
#define CMDBUFSIZENT 8092              /* Max size of executable cmd     */
#define CMDDEFNAME32S "COMMAND.COM"    /* Default Win 95   cmd handler   */
#define CMDDEFNAMENT "CMD.EXE"         /* Default Win NT   cmd handler   */
#define DEFEXT "REX"                   /* Default Win  REXX program ext  */
#include "SubcommandAPI.h"             /* Get private REXXAPI API's      */
#define UNKNOWN_COMMAND 1              /* unknown command return code    */
#include "direct.h"

#define SYSENV "CMD"                   /* Default windows cmd environment*/
#define COMSPEC "COMSPEC"              /*      cmd handler env name      */

extern ULONG ExceptionHostProcessId = 0;
extern HANDLE ExceptionHostProcess = NULL;
extern BOOL ExceptionConsole = FALSE;
static BOOL ExplicitConsole;

#define SHOWWINDOWFLAGS SW_HIDE        // determines visibility of cmd
                                       // window SHOW, HIDE etc...
                                       // function prototypes
LONG sys_command(char *cmd, RexxString ** error_failure);
LONG sysCommandNT(char *cmd, RexxString ** error_failure, BOOL direct);
// LONG sysCommand32s(char *cmd, RexxString ** error_failure);

/******************************************************************************/
/* Arguments:  System Exit name                                               */
/*             System Exit function code (see REXXSAA.H)                      */
/*             System Exit subfunction code (see REXXSAA.H)                   */
/*             Exit specific parm buffer                                      */
/*                                                                            */
/* Returned:   Return code (one of) -                                         */
/*               RXEXIT_HANDLED (exit ran and handled task)                   */
/*               RXEXIT_NOT_HANDLED (exit ran and declined task)              */
/*               RXEXIT_RAISE_ERROR (exit didn't run or raised an error)      */
/*                                                                            */
/* Notes:      Inovkes a system exit routine.                                 */
/******************************************************************************/
BOOL SysExitHandler(
  RexxActivity     * activity,         /* activity working under              */
  RexxActivation   * activation,       /* activation working under            */
  RexxString       * exitname,         /* name of the exit handler            */
  long  function,                      /* major function                      */
  long  subfunction,                   /* minor exit function                 */
  PVOID exitbuffer,                    /* exit specific arguments             */
  BOOL  enable )                       /* enable variable pool                */
{
  int   rc;                            /* exit return code                    */
  PCHAR handler_name;                  /* ASCII-Z handler name                */

  handler_name = exitname->stringData; /* point to the handler name           */
  activity->setCurrentExit(exitname);  /* save the exitname                   */
/* CRITICAL window here -->>  ABSOLUTELY NO KERNEL CALLS ALLOWED              */

                                       /* get ready to call the function      */
  activity->exitKernel(activation, OREF_SYSEXITHANDLER, enable);

                                       /* go call the handler                 */
  rc = RexxCallExit((PSZ)handler_name, NULL, function, subfunction, (PEXIT)exitbuffer);
                                       /* now re-enter the kernel             */
  activity->enterKernel();

/* END CRITICAL window here -->>  kernel calls now allowed again              */
  activity->setCurrentExit(OREF_NULL); /* clear the exitname                  */
                                       /* got an error case?                  */

  if (rc == RXEXIT_RAISE_ERROR || rc < 0) {
    if (function == RXSIO) {           /* this the I/O function?              */
                                       /* disable the I/O exit from here to   */
                                       /* prevent recursive error conditions  */
      activity->setSysExit(RXSIO, OREF_NULL);
    }
    if (function != RXTER)             /* not the termination exit?           */ // retrofir by IH
                                       /* go raise an error                   */
      report_exception1(Error_System_service_service, exitname);
  }
  if (rc == RXEXIT_HANDLED)            /* Did exit handle task?               */
    return FALSE;                      /* Yep                                 */
  else                                 /* rc = RXEXIT_NOT_HANDLED             */
    return TRUE;                       /* tell caller to handle               */
}



/******************************************************************************/
/* Name:       SysCommand                                                     */
/*                                                                            */
/* Arguments:  cmdenv - Command environment (string OREF)                     */
/*             cmd - Command to be executed (string OREF)                     */
/*                                                                            */
/* Returned:   result - Array object containing:                              */
/*                      1) rc from command (integer OREF)                     */
/*                      2) error condition (integer OREF)                     */
/*                                                                            */
/* Notes:      Handles processing of a command.  First passes cmd on to the   */
/*             current subcommand handler.  If there isn't one registered     */
/*             and the current command environment matches the default system */
/*             environment, we'll try passing the command to the system for   */
/*             execution.                                                     */
/******************************************************************************/
RexxObject * SysCommand(
  RexxActivation    * activation,      /* activation working on behalf of     */
  RexxActivity      * activity,        /* activity running on                 */
  RexxString        * environment,     /* target address                      */
  RexxString        * command,         /* command to issue                    */
  RexxString       ** error_failure )  /* error or failure flags              */
{
  INT      rc    = 0;                  /* Return code from call               */
  PCHAR    current_address;            /* Subcom handler that gets cmd        */
  RXSTRING rxstrcmd;                   /* Command to be executed              */
  USHORT   flags = 0;                  /* Subcom error flags                  */
  SHORT    sbrc  = 0;                  /* Subcom return code                  */
  RXSTRING retstr;                     /* Subcom result string                */
  RexxObject * result;                 /* Result array                        */
                                       /* default return code buffer          */
  CHAR     default_return_buffer[DEFRXSTRING];

  *error_failure = OREF_NULL;          /* default to clean call               */
                                       /* set up the RC buffer                */
  MAKERXSTRING(retstr, default_return_buffer, DEFRXSTRING);
                                       /* set up the command RXSTRING         */
  MAKERXSTRING(rxstrcmd, command->stringData, command->length);
                                       /* get the current environment         */
  current_address = environment->stringData;
  if (environment->length == 0)        /* null string command?                */
    current_address = SYSENV;          /* null string is same as CMD          */
  sbrc = 0;                            /* set initial return code             */

/* CRITICAL window here -->>  ABSOLUTELY NO KERNEL CALLS ALLOWED              */

                                       /* get ready to call the function      */
  activity->exitKernel(activation, OREF_COMMAND, TRUE);
  rc=RexxCallSubcom( current_address, NULL, &rxstrcmd, &flags, (PUSHORT)&sbrc, (PRXSTRING)&retstr);
  activity->enterKernel();             /* now re-enter the kernel           */

/* END CRITICAL window here -->>  kernel calls now allowed again              */

  /****************************************************************************/
  /* If subcom isn't registered and it happens to be the current system cmd   */
  /* handler, try passing it on to the system to handle.                      */
  /****************************************************************************/
  if (rc == RXSUBCOM_NOTREG) {
    if ((stricmp((PCHAR)current_address,SYSENV))==0) {
      ReleaseKernelAccess(activity);   /* unlock the kernel                   */
                                       /* issue the command                   */
      rc = sys_command(command->stringData, error_failure);
      RequestKernelAccess(activity);   /* reacquire the kernel lock           */
      result = new_integer(rc);        /* get the command return code         */

      /* CMD.EXE does not have a special 'not found' error as on OS/2 */
//      if (rc == UNKNOWN_COMMAND)       /* is this unknown command?            */
//                                       /*   send failure condition back       */
//        *error_failure = OREF_FAILURENAME;
//     else
     if (rc != 0)                      /* command error?                      */
                                       /*   send error condition back         */
        *error_failure = OREF_ERRORNAME;
    }
    else {                             /* real command failure                */
      *error_failure = OREF_FAILURENAME;
      result = new_integer(rc);        /* just give not registered rc         */
    }
  }
// retrofit by IH
  else if (rc == RXSUBCOM_OK) {        /* Call to subcom worked               */
  /*************************************************************************  */
  /* Put rc from subcom handler into result array                             */
  /*************************************************************************  */
    if (sbrc != 0)                     /* have a numeric return code?         */
    {
      result = new_integer(sbrc);      /* just use the binary return code     */
      hold(result);
    }
    else if (!RXNULLSTRING(retstr)) {  /* we have something in retstr?        */
                                       /* make into a return string           */
      result = new_string(retstr.strptr, retstr.strlength);
      hold(result);
                                       /* try to get the numeric value also */
      sbrc = (SHORT)result->longValue(NO_LONG);
                                       /* user give us a new buffer?          */
      if (retstr.strptr != default_return_buffer)
                                       /* free it                             */
        SysReleaseResultMemory(retstr.strptr);
    }
    else
      result = IntegerZero;            /* got a zero return code              */
                                       /* OS/2 system call?                 */
    if (stricmp((PCHAR)current_address,SYSENV)==0) {

      if (sbrc == UNKNOWN_COMMAND)     /* is this unknown command?          */
                                       /*   send failure condition back     */
        flags |= (USHORT)RXSUBCOM_FAILURE;
      else if (sbrc != 0)              /* command error?                    */
                                       /*   send error condition back       */
        flags |= (USHORT)RXSUBCOM_ERROR;
    }

  /****************************************************************************/
  /* Check error flags from subcom handler and if needed, stick condition     */
  /* into result array.                                                       */
  /****************************************************************************/

    if (flags&(USHORT)RXSUBCOM_FAILURE)/* If failure flag set               */
                                       /*   send failure condition back     */
      *error_failure = OREF_FAILURENAME;
                                       /* If error flag set                 */
    else if (flags&(USHORT)RXSUBCOM_ERROR)
      *error_failure = OREF_ERRORNAME; /*   send error condition back       */
  }
  else {                               /* Call to subcom didn't work        */
    *error_failure = OREF_FAILURENAME; /*   send failure condition back     */
    result = new_integer(rc);          /* just use the binary return code   */
  }
  return result;                       /* Return result value               */
}


/* Handle "SET XX=YYY" command in same process */
BOOL sys_process_set(char * cmd, LONG * rc)
{
    char * eqsign, * st;
    char name[256];
    char value[4096];
    eqsign = strchr(cmd, '=');
    if (!eqsign) return FALSE;

    st = &cmd[4];
    while ((*st) && (*st == ' ')) st++;
    if (st == eqsign) return FALSE;
    strncpy(name, st, eqsign-st);
    name[eqsign-st]='\0';

    if (ExpandEnvironmentStrings(eqsign+1, value, 4095)
        && SetEnvironmentVariable(name,value)) *rc = 0; else *rc = GetLastError();
    return TRUE;
}


/* Handle "CD XXX" command in same process */
BOOL sys_process_cd(char * cmd, LONG * rc)
{
    char * st;

    st = &cmd[3];
    while ((*st) && (*st == ' ')) st++;
    if (!*st) return FALSE;

    if ((strlen(st) == 2) && (st[1] == ':'))
         *rc = _chdrive(toupper( *st ) - 'A' + 1);
    else
         *rc = _chdir(st);
    return TRUE;
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
LONG sys_command(char *cmd, RexxString **error_failure)
{
  PCHAR       cl_opt = " /c ";         /* "/c" opt for sys cmd handler      */
  PCHAR       cmdstring_ptr;           /* Command string pointer            */
  CHAR        cmdstring[CMDBUFSIZENT]; /* Largest cmd we can give system    */
  PCHAR       sys_cmd_handler;         /* Pointer to system cmd handler     */
  ULONG       length_cmd_handler;      /* Length of cmd handler path/name   */
  ULONG       max_cmd_length;          /* Maximum size of command           */
  LONG        rc;                      /* Return code                       */
  CHAR       *interncmd;

  ULONG       uMaxStringLength;        // max length of string (system
                                       // specific)
  CHAR        tmp[8];
  size_t      j = 0, i = 0;
  PCHAR       filepart;
  BOOL        fileFound, searchFile;
  BOOL        NoDirectInvoc;
  BOOL        fInQuotes = FALSE;

                                              /* remove quiet sign              */
  if (cmd[0] == '@') interncmd = cmd + 1; else interncmd = cmd;

  /* check for redirection symbols, ignore them when enclosed in double quotes */
  NoDirectInvoc = FALSE;
  for (i=0; i<strlen(interncmd); i++)
  {
    if (interncmd[i] == '"')
      fInQuotes = !fInQuotes;
    else
    {
      /* if we're in the unquoted part and the current character is one of */
      /* the redirection characters or the & for multiple commands then we */
      /* will no longer try to invoke the command directly                 */
      if (!fInQuotes && (strchr("<>|&", interncmd[i]) != NULL))
      {
        NoDirectInvoc = TRUE;
        break;
      } /* endif */
    } /* endif */
  } /* endfor */

  i = 0; /* reset to zero for next usage ! */

  while (interncmd[j] == ' ') j++;

  if (!NoDirectInvoc)
  {
      strncpy(tmp, &interncmd[j], 4);
      tmp[4] = '\0';
      if (!stricmp("set ",tmp))
      {
          if (sys_process_set(&interncmd[j], &rc))
              return rc;
      }
      else
      {
         strncpy(tmp, &interncmd[j], 3);
         tmp[3] = '\0';
         if (!stricmp("cd ",tmp))
         {
             if (sys_process_cd(&interncmd[j], &rc))
                 return rc;
         }
         else
         {          /* check for drive letter change */
             if ((tmp[1] == ':') && ((tmp[2] == ' ') || (!tmp[2])))
                return _chdrive(toupper( tmp[0] ) - 'A' + 1);
             else
             {
                 /* check if a START command is specified, if so don not start it directly */
                 strncpy(tmp, &interncmd[j], 6);
                 tmp[6] = '\0';
                 NoDirectInvoc = (BOOL) !stricmp("start ",tmp);
             }
         }
      }
  }
                                       // location of command interpreter
  if (NULL == (sys_cmd_handler = getenv(COMSPEC)))
                                       // CMD.EXE on NT Command.com on 32s
    sys_cmd_handler = (RUNNING_NT) ? CMDDEFNAMENT : CMDDEFNAME32S;
                                       /* Get len of handler                  */
  length_cmd_handler = strlen((PCHAR)sys_cmd_handler);

  cmdstring_ptr = cmdstring;           /* Set pointer to cmd buffer           */
  /****************************************************************************/
  /* Compute maximum size of command we can fit into string being passed to   */
  /* CreateProcess                                                            */
  /****************************************************************************/
                                      // string length is system specific
  uMaxStringLength = (RUNNING_NT) ? CMDBUFSIZENT : CMDBUFSIZE32S;
  max_cmd_length = uMaxStringLength -       /* Maximum size of string         */
                   (length_cmd_handler+1) - /* Minus length of sys cmd handler*/
                   strlen(cl_opt) -         /* Minus length of cmd line option*/
                   1;                       /* Minus length of terminating \0 */

  if (max_cmd_length < strlen(interncmd))   /* If command is too long...      */
    *(interncmd+max_cmd_length) = '\0';     /*  truncate it                   */
  ExceptionConsole = FALSE;
  ExplicitConsole = FALSE;

  /* check whether or not program to invoke is cmd or command */
  _strupr(strcpy(cmdstring_ptr,&interncmd[j]));
  if (!RUNNING_95)
      searchFile = (BOOL)strstr(cmdstring_ptr, "CMD");
  else
      searchFile = (BOOL)strstr(cmdstring_ptr, "COMMAND");

  if (searchFile)
  {
      if (cmdstring_ptr[0] == '\"') {
          cmdstring_ptr++;
          while (cmdstring_ptr[i] && (cmdstring_ptr[i] != '\"')) i++;
          cmdstring_ptr[i]='\0';
      } else
      if (cmdstring_ptr[0] == '\'') {
          cmdstring_ptr++;
          while (cmdstring_ptr[i] && (cmdstring_ptr[i] != '\'')) i++;
          cmdstring_ptr[i]='\0';
      } else {
          while (cmdstring_ptr[i] && (cmdstring_ptr[i] != ' ')) i++;
          cmdstring_ptr[i]='\0';
      }

      if (!RUNNING_95)
          fileFound = SearchPath(NULL, cmdstring_ptr, ".EXE", CMDBUFSIZENT-1, cmdstring, &filepart);
      else
          fileFound = SearchPath(NULL, cmdstring_ptr, ".COM", CMDBUFSIZENT-1, cmdstring, &filepart);
      cmdstring_ptr = cmdstring;           /* Set pointer again to cmd buffer (might have been increased) */

      if (fileFound && !stricmp(sys_cmd_handler, cmdstring_ptr))
      {
          ExceptionConsole = TRUE;
          ExplicitConsole = TRUE;
      }
  }

  /* first check whether we can run command directly as a program (no file redirection when not cmd or command) */
  if (ExplicitConsole || !NoDirectInvoc)
  {
      rc = sysCommandNT(&interncmd[j], error_failure, TRUE);
      if (*error_failure != OREF_FAILURENAME) return rc;   /* command processed, exit */
      else *error_failure = OREF_NULL;
  }
  /* no we couldn't, so pass command to cmd.exe or command.com */

  strcpy(cmdstring_ptr,(PCHAR)sys_cmd_handler);    /* Put in system cmd handler      */

  /* the following lines checks whether or not a /k option is specified */
  /* if so the /c option must not be concatenated */
  /* /k can only be specified as the first argument to keep the command handler active */
  if (!( (strlen(interncmd) > j+1) && (interncmd[j] == '/')
      && ((interncmd[j+1] == 'k') || (interncmd[j+1] == 'K'))
      && ((interncmd[j+2] == ' ') || (interncmd[j+2] == '\0')) ))
      strcat(cmdstring_ptr,cl_opt);
  else
  {
      ExceptionConsole = TRUE;
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
  rc = sysCommandNT(cmdstring_ptr, error_failure, FALSE);
//  else rc = sysCommand32s(cmdstring_ptr, error_failure);

  ExceptionConsole = FALSE;
  return rc;
}                                      // SystemCommand


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
LONG sysCommandNT(char *cmdstring_ptr, RexxString  **error_failure, BOOL direct)
{
  DWORD rc;
  STARTUPINFO siStartInfo;                  // process startup info
  PROCESS_INFORMATION piProcInfo;           // returned process info
  CHAR ctitle[256];
  DWORD creationFlags;
  BOOL titleChanged;

  ZeroMemory(&siStartInfo, sizeof(siStartInfo));
  ZeroMemory(&piProcInfo, sizeof(piProcInfo));
  /****************************************************************************/
  /* Invoke the system command handler to execute the command                 */
  /****************************************************************************/
  siStartInfo.cb = sizeof(siStartInfo);
  //siStartInfo.lpReserved = siStartInfo.lpDesktop = NULL;
  //siStartInfo.lpReserved2 = siStartInfo.hStdInput = NULL;
  //siStartInfo.hStdOutput = siStartInfo.hStdError = NULL;

  siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  titleChanged = GetConsoleTitle(ctitle, 255);
  siStartInfo.lpTitle = cmdstring_ptr;
  creationFlags = GetPriorityClass(GetCurrentProcess()) | CREATE_NEW_PROCESS_GROUP;
  if (!siStartInfo.hStdInput && !siStartInfo.hStdOutput && !titleChanged)  /* is REXXHIDE running without console */
  {
      if (!direct) {
          siStartInfo.wShowWindow = SHOWWINDOWFLAGS;
             siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;
      } else if (ExplicitConsole)
          creationFlags |= CREATE_NEW_CONSOLE; /* new console if CMD ord COMMAND was specified */
  }
  else              /* just use standard handles if we are running in a console */
  {
     if (direct) siStartInfo.dwFlags = STARTF_USESTDHANDLES; /* no SW_HIDE for direct commands */
     else siStartInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
     siStartInfo.wShowWindow = SHOWWINDOWFLAGS;
  }

  if ( CreateProcess ( NULL,           // address of module name
                  cmdstring_ptr,       // address of command line
                  NULL,                // address of process security attrs
                  NULL,                // address of thread security attrs
                  TRUE,                // new process inherits handles?
                                       // creation flags
                  creationFlags,
                  NULL,                // address of new environment block
                  NULL,                // address of current directory name
                  &siStartInfo,        // address of STARTUPINFO
                  &piProcInfo))        // address of PROCESS_INFORMATION
                                       // good rc from create now
                                       // Wait for process to end and
  {
    if (titleChanged) SetConsoleTitle(siStartInfo.lpTitle);
    ExceptionHostProcess = piProcInfo.hProcess;
    ExceptionHostProcessId = piProcInfo.dwProcessId;

    if (WAIT_FAILED != WaitForSingleObject ( piProcInfo.hProcess, INFINITE ) )
                                       // complete,ok?, get terminate rc
      GetExitCodeProcess ( piProcInfo.hProcess, &rc );
    else {
       rc = GetLastError ();         // bad termination? get error code
       *error_failure = OREF_FAILURENAME;
    } // WaitForSingleObject
    /* the new process must be detached so it will be discarded automatically after execution */
    /* The thread must be closed first */
    if (titleChanged) SetConsoleTitle(ctitle);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(piProcInfo.hProcess);
  }
  else {
     rc = GetLastError ();           // Couldn't create process
     *error_failure = OREF_FAILURENAME;
  } // CreateProcess

  ExceptionHostProcess = NULL;
  ExceptionHostProcessId = 0;
  return rc;
}                                      // SysCommandNT



#if 0
/*-----------------------------------------------------------------------------
 | Name:       sysCommand32s                                                  |
 |                                                                            |
 | Arguments:  cmd - Command to be executed                                   |
 |                                                                            |
 | Returned:   rc - Return Code                                               |
 |                  Note: if non-zero rc from DosExecPgm return DosExecPgm rc |
 |                  else if non-zero termination code from system return code |
 |                  else return rc from executed command                      |
 |                                                                            |
 | Notes:      Handles processing of a system command on a Windows 3.1 system |
 |             Calls 16-bit SystemCommand function via 32-16-bit Universal    |
 |             thunk                                                         |
 |                                                           |
  ----------------------------------------------------------------------------*/
LONG sysCommand32s(char *cmdstring_ptr, RexxString **error_failure)
{
  typedef int (FAR WINAPI * PSYSCMD)(PTSTR, UINT);
  #define UT32DLL "RXCMD32.DLL"        // name of 32-bit UT dll
  DWORD rc;                            // SystemCommand RC
  PSYSCMD fpSystemCommand = NULL;      // ptr to SystemCommand in UT
  HINSTANCE hInstUT32 = NULL;          // handle of 32-bit UT DLL

  /* On Windows 3.1 must use 16-bit WinExec
   | Spawn command.com with the system command as command line
   | Get ahold of the Universal Thunk 32-bit stub DLL, and the
   | address of SystemCommand service routine
   */
                                       // retrieve library handle
  if (hInstUT32 = LoadLibrary (UT32DLL))
                                       // retrieve address of 32-bit UT stub
    if (fpSystemCommand = (PSYSCMD)GetProcAddress (hInstUT32,"SystemCommand"))
                                       // run baby run
      rc = (fpSystemCommand)(cmdstring_ptr, SHOWWINDOWFLAGS);
    else {                             // can't find proc address set failure
      *error_failure = OREF_FAILURENAME;
      rc = GetLastError();             // set rc for error
     }//else
  else {                               // dll load failure, set failure
    *error_failure = OREF_FAILURENAME; // condition and rc for error
    rc = GetLastError();
  } //else


  return rc;
}                                      // sysCmd32s
#endif

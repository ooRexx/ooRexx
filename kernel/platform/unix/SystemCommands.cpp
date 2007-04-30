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
/* REXX UNIX  Support                                            aixcmd.c     */
/*                                                                            */
/* AIX specific command processing routines                                   */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*  aixcmd.c - C methods for handling calls to system exits and subcommand    */
/*             handlers.                                                      */
/*                                                                            */
/*  C methods:                                                                */
/*    SysExitHandler - Native method to invoke a system exit                  */
/*    SysCommand     - Method to invoke a subcommand handler                  */
/*                                                                            */
/*  Internal routines:                                                        */
/*    sys_command - Run a command through system command processor.           */
/******************************************************************************/

#include <string.h>                         /* Get strcpy, strcat, etc.       */
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#define INCL_RXSUBCOM                       /* Include subcom declares        */
#define INCL_RXFUNC                         /* and external function...       */
#define INCL_RXSYSEXIT                      /* and system exits               */
#define ALL_RETURN_CODES

#include "RexxCore.h"                         /* global REXX declarations       */
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "RexxNativeAPI.h"                           /* Lot's of useful REXX macros    */

#include SYSREXXSAA                         /* Include REXX header            */
#include "SystemCommands.h"
#include "SubcommandAPI.h"                  /* Get private REXX API's         */
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

#define CCHMAXPATH PATH_MAX+1
#define CMDBUFSIZE 1024                     /* Max size of executable cmd     */
#define MAX_COMMAND_ARGS 400

#if defined(AIX)
#define CMDDEFNAME "/bin/ksh"               /* Korn shell is default for AIX */
#elif defined(OPSYS_SUN)                        /*  path for AIX        */
#define CMDDEFNAME "/bin/sh"                /* Bourne Again Shell is default */
#else                                       /* shell for Linux               */
#define CMDDEFNAME "/bin/bash"              /* Bourne Again Shell is default */
#endif

#define DEFEXT "CMD"                        /* Default REXX program ext       */
#define UNKNOWN_COMMAND 127                 /* unknown command return code    */

#define SYSENV "command"                    /* Default cmd environment        */
#define SHELL  "SHELL"                      /* UNIX cmd handler env. var. name*/
#define EXPORT_FLAG 1
#define SET_FLAG    2
#define UNSET_FLAG  3
#define MAX_VALUE   1280

extern INT putflag;

extern char **environ;

extern char achRexxCurDir[ CCHMAXPATH+2 ];  /* Save current working direct    */

char * args[MAX_COMMAND_ARGS+1];            /* Array for argument parsing */

LONG sys_command(char *cmd, CMD_TYPE local_env_type);
void scan_cmd(char *parm_cmd, char **args);

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
  RexxActivity     * activity,         /* activity working under            */
  RexxActivation   * activation,       /* activation working under          */
  RexxString       * exitname,         /* name of the exit handler          */
  long             function,           /* major function                    */
  long             subfunction,        /* minor exit function               */
  PVOID            exitbuffer,         /* exit specific arguments           */
  BOOL             enable )            /* enable variable pool              */
{
  int   rc;                            /* exit return code                  */
  PCHAR handler_name;                  /* ASCII-Z handler name              */

  handler_name = exitname->stringData; /* point to the handler name         */
  activity->setCurrentExit(exitname);  /* save the exitname                 */
/* CRITICAL window here -->>  ABSOLUTELY NO KERNEL CALLS ALLOWED            */

                                       /* get ready to call the function    */
  activity->exitKernel(activation, OREF_SYSEXITHANDLER, enable);

                                       /* go call the handler               */
rc = RexxCallExit((PSZ)handler_name, NULL, function, subfunction, (PEXIT)exitbuffer);


  activity->enterKernel();             /* now re-enter the kernel           */

/* END CRITICAL window here -->>  kernel calls now allowed again            */
  activity->setCurrentExit(OREF_NULL); /* clear the exitname                */
                                       /* got an error case?                */
  if (rc == RXEXIT_RAISE_ERROR || rc < 0) {
    if (function == RXSIO) {           /* this the I/O function?            */
                                       /* disable the I/O exit from here to */
                                       /* prevent recursive error conditions*/
      activity->setSysExit(RXSIO, OREF_NULL);
    }
                                       /* go raise an error                 */
    report_exception1(Error_System_service_service, exitname);
  }
  if (rc == RXEXIT_HANDLED)            /* Did exit handle task?             */
    return FALSE;                      /* Yep                               */
  else                                 /* rc = RXEXIT_NOT_HANDLED           */
    return TRUE;                       /* tell caller to handle             */
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
  RexxActivation    * activation,      /* activation working on behalf of   */
  RexxActivity      * activity,        /* activity running on               */
  RexxString        * environment,     /* target address                    */
  RexxString        * command,         /* command to issue                  */
  RexxString       ** error_failure )  /* error or failure flags            */
{
  INT          rc    = 0;              /* Return code from call             */
  PCHAR        current_address;        /* Subcom handler that gets cmd      */
  RXSTRING     rxstrcmd;               /* Command to be executed            */
  USHORT       flags = 0;              /* Subcom error flags                */
  SHORT        sbrc  = 0;              /* Subcom return code                */
  RXSTRING     retstr;                 /* Subcom result string              */
  CMD_TYPE     local_env_type;
  char *       shell_cmd;
  int          i;
  long         length;
  RexxObject * result;

  CHAR     default_return_buffer[DEFRXSTRING];

  *error_failure = OREF_NULL;          /* default to clean call             */

                                       /* set up the RC buffer              */
  MAKERXSTRING(retstr, default_return_buffer, DEFRXSTRING);

                                       /* set up the command RXSTRING       */
  MAKERXSTRING(rxstrcmd, command->stringData, command->length);

                                       /* get the current environment       */
  current_address = environment->stringData;

                                   /* convert current_address to a CMD_TYPE */
  local_env_type = cmd_pgm;               /* default to user defined subcom */
  for (i = 0; i < REG_ENVTABLE_SIZE; i++)     /* scan the table for a match */
    {
    length = strlen(reg_envtable[i].envname);
       if (environment->length == length &&
           !memcmp(current_address,               /* and if names are equal */
           reg_envtable[i].envname,           /* this is the new environment*/
           length))  {                             /* otherwise use default */
              local_env_type = reg_envtable[i].envtype;
              break;
             } /* endif */
     } /* endfor */

  sbrc = 0;                               /* set initial subcom return code */
                                       /* get ready to call the function    */
  activity->exitKernel(activation, OREF_COMMAND, TRUE);
  rc=RexxCallSubcom( current_address, NULL, &rxstrcmd, &flags, (PUSHORT)&sbrc, (PRXSTRING)&retstr);
  activity->enterKernel();             /* now re-enter the kernel           */

/* END CRITICAL window here -->>  kernel calls now allowed again            */

  /****************************************************************************/
  /* If subcom isn't registered and it happens to be the current system cmd   */
  /* handler, try passing it on to the system to handle.                      */
  /****************************************************************************/
  if (rc == RXSUBCOM_NOTREG) {
    if (!(local_env_type == cmd_pgm)) {

      shell_cmd = command->stringData;

      ReleaseKernelAccess(activity);                   /* unlock the kernel */

      rc = sys_command(shell_cmd, local_env_type);     /* issue the command */

      RequestKernelAccess(activity);           /* reacquire the kernel lock */

      result = new_integer(rc);              /* get the command return code */

      if (rc == UNKNOWN_COMMAND) {            /* is this unknown command?    */
                                             /* send failure condition back */
        *error_failure = OREF_FAILURENAME;
      }
      else if (rc != 0)                             /* command error?       */
                                               /* send error condition back */
        *error_failure = OREF_ERRORNAME;
    }
    else {                                          /* real command failure */
      *error_failure = OREF_FAILURENAME;
      result = new_integer(rc);              /* just give not registered rc */
    } // end of RXSUBCOM_NOTREG
  }
  else if (rc == RXSUBCOM_OK) {                    /* Call to subcom worked */
                                                   /* system call?          */
    if (strcmp((PCHAR)current_address,SYSENV)==0) {

      if (sbrc == UNKNOWN_COMMAND)              /* is this unknown command? */
                                             /* send failure condition back */
        flags |= (USHORT)RXSUBCOM_FAILURE;
      else if (sbrc != 0)                                 /* command error? */
                                               /* send error condition back */
        flags |= (USHORT)RXSUBCOM_ERROR;
    }
  /****************************************************************************/
  /* Put rc from subcom handler into result array                             */
  /****************************************************************************/
    if (sbrc != 0)                             /* have a numeric return code? */
      result = new_integer(sbrc);          /* just use the binary return code */
    else if (!RXNULLSTRING(retstr)) {         /* we have something in retstr? */
                                                 /* make into a return string */
      result = new_string(retstr.strptr, retstr.strlength);
                                                /* user give us a new buffer? */
      if (retstr.strptr != default_return_buffer)

        SysReleaseResultMemory(retstr.strptr);                  /* free it... */
    }
    else
      result = IntegerZero;                         /* got a zero return code */

  /****************************************************************************/
  /* Check error flags from subcom handler and if needed, stick condition     */
  /* into result array.                                                       */
  /****************************************************************************/
    if (flags&(USHORT)RXSUBCOM_ERROR)           /* If error flag set          */
      *error_failure = OREF_ERRORNAME;          /* send error condition back  */
                                                /* If failure flag set        */
    else if (flags&(USHORT)RXSUBCOM_FAILURE)
                                                /* send failure condition back*/
      *error_failure = OREF_FAILURENAME;
  }
  else {                                        /* Call to subcom didn't work */
    *error_failure = OREF_FAILURENAME;          /* send failure condition back*/
    result = new_integer(rc);                   /* just use the binary return code*/
  }
  return result;                                /* Return result value        */
}


/* Handle "export" command in same process */
BOOL sys_process_export(char * cmd, LONG * rc, int flag)
{
  PCHAR  Env_Var_String = NULL;        /* Environment variable string for   */
  ULONG size, allocsize;               /* size of the string                */
  PCHAR      * Environment;            /* environment pointer               */
  PCHAR  np;
  INT    i,j,k,l,iLength, copyval;
  CHAR   namebufcurr[1281];             /* buf for extracted name            */
  CHAR   cmd_name[1281];                /* name of the envvariable setting   */
  CHAR   *array, *runarray, *runptr, *endptr, *maxptr;
  CHAR   temparray[1281];
  CHAR   *st;
  CHAR   *tmpptr;
  CHAR   name[1281];                    /* is the name + value + =           */
  CHAR   value[1281];                   /* is the part behind =              */
  PCHAR  del = NULL;                   /* ptr to old unused memory          */
  PCHAR  hit = NULL;
  BOOL   HitFlag = FALSE;
  l = 0;
  j = 0;
  allocsize = 1281 * 2;

  memset(temparray, '\0', sizeof(temparray));

  Environment = environ;               /* get the environment               */
  if(flag == EXPORT_FLAG)
  {
     st = &cmd[6];
  }
  else if(flag == UNSET_FLAG)
  {
     st = &cmd[5];
  }
  else
  {
     st = &cmd[3];
  }
  while ((*st) && (*st == ' ')) st++;
  strcpy(name, st);
  iLength=strlen(st) +1;

/* if string == EXPORT_FLAG or string == SET and only blanks are delivered */

  if ( ((flag == EXPORT_FLAG) || (flag == SET_FLAG)) &&  (iLength == 1) )
  {
     return FALSE;
  }

  if(!putflag)
  {                        /* first change in the environment ? */
    /* copy all entries to dynamic memory                                   */
    for(;*Environment != NULL;Environment++)
    {                                  /*for all entries in the env         */
      size = strlen(*Environment)+1;   /* get the size of the string        */
      Env_Var_String = (PCHAR)malloc(size);/* and alloc space for it        */
      memcpy(Env_Var_String,*Environment,size);/* copy the string           */
      putenv(Env_Var_String);          /* and chain it in                   */
    }
  }
  putflag = 1;                         /* prevent do it again               */
  Environment = environ;               /* reset the environment pointer     */

/* do we have a assignment operator? If not return TRUE           */
/* The operating system treads this like no command, and so do we */

  if ( !(strchr(name, '=')) && (flag != UNSET_FLAG) ) /*only set and export */
  {
/* we do not have a assignment operator, but maybe a '|' for a    */
/* controlled output                                              */
     if ( (strchr(name, '|'))  || (strchr(name, '>')) || (strstr(name, ">>")) )
     {
       return FALSE;
     }
     else
     {
       *rc = 0;
       return TRUE;
     }
  }

/* no '=' for unset, so force a shell error message               */

  if ( (strchr(name, '=')) && (flag == UNSET_FLAG) )
  {
     return FALSE;
  }

  for(i=0;(name[i]!='=')&&(i<iLength);name[i++])
  {
//   memcpy(&(cmd_name[i]), &(name[i]), 1);
     cmd_name[i] = name[i];
  }

/* lets try handling variables in the assignment string */

//memcpy(&(cmd_name[i]),"\0",1);      /* copy the terminator           */
  cmd_name[i]  = '\0';                /* copy the terminator           */

  i++;                                /* the place after'=' */

/* lets search the value for variable part(s)  */

  strcpy(value, &(name[i]));         /* value contains the part behind '=' */
  array = (char *) malloc(1281);
  strcpy(array, cmd_name);
  array[strlen(cmd_name)] = '=';
//memcpy(&(array[i+1]),"\0",1);      /* copy the terminator           */
//memcpy(&(array[i]),"\0",1);        /* copy the terminator           */
  array[i] = '\0';                   /* copy the terminator           */
  runarray = array + strlen(array);
  runptr = value;
  endptr = runptr + strlen(value);   /*this is the end of the input*/
  maxptr = array + MAX_VALUE -1;     /* this is the end of our new string */

  while(tmpptr = (strchr(runptr, '$')))
  {
    Environment = environ;   /* get the beginning of the environment*/
    HitFlag = TRUE;          /* if not true inputvalue= outputvalue*/
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
       *runarray = NULL;
       runptr = tmpptr;              /* now runptr is at the place of $ */
    }
    runptr++;
    for (j = 0;(*runptr != '/') && (*runptr != ':') && (*runptr != '$') &&
               (*runptr); j++)
    {
      memcpy(&(temparray[j]), runptr,1); /*temparray is the env var to search*/
      runptr++;
    }

//  memcpy(&(temparray[j]), '\0',1);    /* lets see what we can do    */
    temparray[j] = '\0';                /* lets see what we can do    */

    for(;(*Environment != NULL) && (hit == NULL) ;Environment++)
    {
      np = *Environment;

      for(k=0;(*np!='=')&&(k<255);np++,k++)
      {
        memcpy(&(namebufcurr[k]),np,1);  /* copy the character                */
      }

//    memcpy(&(namebufcurr[k]),"\0",1);      /* copy the terminator           */
      namebufcurr[k] = '\0';                 /* copy the terminator           */

      if(!strcmp(temparray,namebufcurr))     /* have a match ?         */
      {
        hit = *Environment;
                /* copy value to new string*/
      }
    } /* end of for loop                                                */
    if(hit) /* if we have found an entry of the env var in the env list */
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
       *runarray = NULL;
       hit = NULL;
    }
  }   /* end while loop */

  if (HitFlag == TRUE)
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
       *runarray = NULL;
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
     *runarray = NULL;
  }

  Environment = environ;            /* get the beginning of the environment*/

  for(;*Environment != NULL;Environment++)
  {
    np = *Environment;

    for(i=0;(*np!='=')&&(i<255);np++,i++)
    {
      memcpy(&(namebufcurr[i]),np,1);  /* copy the character                */
    }

//  memcpy(&(namebufcurr[i]),"\0",1);      /* copy the terminator          */
    namebufcurr[i] = '\0';                 /* copy the terminator          */

    if(!strcmp(cmd_name,namebufcurr))/* have a match ?         */
    {
      del = *Environment;              /* remember it for deletion          */
    }
  }
                                       /* find the entry in the environ     */
  if(flag != UNSET_FLAG)
  {
    size = strlen(array)+1;
    Env_Var_String = (PCHAR)malloc(size);/* get the memory                    */
    memcpy(Env_Var_String, array, size);
    *rc = putenv(Env_Var_String);
  }

  if(del)                              /* if there was a old one            */
  {
    free(del);                         /* free it                           */
    *rc = 0;
  }
  else
  {
    *rc = 0;
  }
  return TRUE;
}


/* Handle "cd XXX" command in same process */
BOOL sys_process_cd(char * cmd, LONG * rc)
{
    char * st;
    PCHAR  home_dir = NULL;            /* home directory path        */
    PCHAR  dir_buf = NULL;             /* full directory path        */
    PCHAR  slash;                      /* ptr to '/'                 */
    struct passwd *ppwd;
    INT alloc_flag = 0;

    st = &cmd[3];
    while ((*st) && (*st == ' ')) st++;
    if ((!*st) || (strlen(cmd) == 2))
    {
      home_dir = getenv("HOME");
      if(!home_dir)
          return FALSE;
      dir_buf = (PCHAR)malloc(strlen(home_dir)+1);
      strcpy(dir_buf, home_dir);
      st = dir_buf;
      alloc_flag = 1;
    }                                  /* if no user name            */
    else if(*(st) == '~' && (*(st+1) == '\0' || *(st+1) == '/'|| *(st+1) == ' ' )){
      if(*(st+1) == '/'){              /* if there is a path         */
        st +=2;                        /* jump over '~/'             */
                                       /* get home directory path    */
        home_dir = getenv("HOME");     /* from the environment       */
        if(!home_dir)                  /* if no home dir info        */
          return FALSE;
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(home_dir)+strlen(st)+1);
        if(!dir_buf)
          return FALSE;
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/%s", home_dir, st);
        st = dir_buf;                  /* directory change to        */
        alloc_flag = 1;
      }
      else{
                                       /* get home directory path    */
        home_dir = getenv("HOME");     /* from the environment       */
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(home_dir)+1);
        if(!dir_buf)
          return FALSE;
        sprintf(dir_buf, "%s/", home_dir);
        st = dir_buf;                  /* directory change to        */
        alloc_flag = 1;
      }
    }
    else if(*(st) == '~'){             /* cmd is '~username...'      */
      st++;                            /* jump over '~'              */
      slash = strchr(st,'/');          /* search for '/'             */
      if(!slash){                      /* if no '/'                  */
                                       /* rest of string is username */
        ppwd = getpwnam(st);           /* get info about the user    */
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(ppwd->pw_dir)+1);
        if(!dir_buf)
          return FALSE;
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/", ppwd->pw_dir);
        alloc_flag = 1;
      }
      else{                            /* there is a slash           */
        *slash = '\0';                 /* teminate to get username   */
        ppwd = getpwnam(st);           /* get info about the user    */
        slash++;                       /* step over the slash        */
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(ppwd->pw_dir)+strlen(slash)+1);
        if(!dir_buf)
          return FALSE;
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/%s", ppwd->pw_dir, slash);
        alloc_flag = 1;
      }
      st = dir_buf;                    /* directory change to        */
    }

    *rc = chdir(st);

    if (!getcwd(achRexxCurDir, CCHMAXPATH))    /* Save current working direct */
    {
      strncpy( achRexxCurDir, getenv("PWD"), CCHMAXPATH);
      achRexxCurDir[CCHMAXPATH - 1] = '\0';
      if (achRexxCurDir[0] != '/' )
        report_exception(Error_System_service);  /* Complain if it fails        */
    }
    if (alloc_flag)
      free(st);
    return TRUE;
}

/******************************************************************************/
/* Name:       sys_command                                                    */
/*                                                                            */
/* Arguments:  cmd - Command to be executed                                   */
/*             local_env_type - integer indicating which shell                */
/*                                                                            */
/* Returned:   rc - Return Code                                               */
/*                                                                            */
/* Notes:      Handles processing of a system command.                        */
/*             Uses the 'fork' and 'exec' system calls to create a new process*/
/*             and invoke the shell indicated by the local_env_type argument. */
/*             This is modeled after command handling done in Classic REXX.   */
/******************************************************************************/
LONG sys_command(char *cmd, CMD_TYPE local_env_type)
{
  LONG        rc;                      /* Return code                       */
  int         pid;                     /* process id of child from fork     */
  int         status;
  int         cmdlength;
#ifdef LINUX
  int         iErrCode = 0;
#endif

  /* execute 'cd' in the same process */
  CHAR        tmp[8];


  if(strlen(cmd) == 2)
  {
    strncpy(tmp, cmd, 2);
    tmp[2] = '\0';
    if (!strcmp("cd", tmp))
    {
       if (sys_process_cd(cmd, &rc))
           return rc;
    }
  }
  else if(strlen(cmd) >= 3)
  {
     strncpy(tmp, cmd, 3);
     tmp[3] = '\0';
     if (!strcmp("cd ",tmp))
     {
         if (sys_process_cd(cmd, &rc))
             return rc;
     }
     strncpy(tmp, cmd, 4);
     tmp[4] = '\0';
     if (!strcmp("set ",tmp))
     {
         if (sys_process_export(cmd, &rc, SET_FLAG)) /*unset works fine for me*/
             return rc;
     }
     strncpy(tmp, cmd, 6);
     tmp[6] = '\0';
     if (!stricmp("unset ", tmp))
     {
        if (sys_process_export(cmd, &rc, UNSET_FLAG))
             return rc;
     }
     strncpy(tmp, cmd, 7);
     tmp[7] = '\0';
     if (!stricmp("export ", tmp))
     {
        if (sys_process_export(cmd, &rc, EXPORT_FLAG))
             return rc;
     }
  }


  /****************************************************************************/
  /* Invoke the system command handler to execute the command                 */
  /****************************************************************************/

#ifdef LINUX
  if ( local_env_type == cmd_bash )
  {
    iErrCode = system( cmd );
    if ( iErrCode >= 256 )
       rc = iErrCode / 256;
    else
       rc = iErrCode;
  }
  else
  {
#endif
    if  ( pid = fork())                    /* spawn a child process to run the  */
    {
      waitpid ( pid, &status, 0);          /* command and wait for it to finish */
      if (WIFEXITED(status))               /* If cmd process ended normal       */
                                           /* Give 'em the exit code            */
        rc = WEXITSTATUS(status);

       else {                              /* Else process died ugly, so        */
         rc = -(WTERMSIG(status));
        if (rc == 1)                       /* If process was stopped            */
          rc = -1;                         /* Give 'em a -1.                    */
      } /* endif */
      return rc;
    }
    else
    {                                /* run the command in the child      */
      switch (local_env_type) {
      case cmd_sh:
        execl("/bin/sh", "sh", "-c", cmd, 0);
        break;
      case cmd_ksh:
        execl("/bin/ksh", "ksh", "-c", cmd, 0);
        break;
      case cmd_bsh:
        execl("/bin/bsh", "bsh", "-c", cmd, 0);
        break;
      case cmd_csh:
        execl("/bin/csh", "csh", "-c", cmd, 0);
        break;
      case cmd_bash:
        execl("/bin/bash", "bash", "-c", cmd, 0);
        break;
      case cmd_cmd:
        scan_cmd(cmd,args);              /* Parse cmd into arguments  */
        execvp(args[0], args);           /* Invoke command directly   */
        perror(" *E* Address COMMAND");  /* If we get to this point,  */
        exit(1);                         /* we couldn't run the       */
                                         /* command, so say so and    */
                                         /* get out.                  */
        break;
      default:
        execl("/bin/sh", "sh", "-c", cmd, 0);
        break;
      } /* endswitch */
    }
#ifdef LINUX
  }
#endif
   return rc;
}    // end sys_command()


/*********************************************************************/
/* This function breaks a command up into whitespace-delimited pieces*/
/* to create the pointer array for the execvp call.  It is only used */
/* to support the "COMMAND" command environment, which does not use  */
/* a shell to invoke its commands.                                   */
/*********************************************************************/

void scan_cmd(char *parm_cmd, char **args)
{
  char * pos;                          /* Current position in command*/
  char * end;                          /* End of command             */
  char * cmd;                          /* Local copy of parm_cmd     */
  short int i;                         /* Local variable             */

  i = strlen(parm_cmd);                /* Get length of parm_cmd     */
  cmd = (char *)malloc(i+1);           /* Allocate for copy          */
  memcpy(cmd,parm_cmd,i+1);            /* Copy the command           */

  end = cmd + i;                       /* Find the end of the command*/

  /* This loop scans our copy of the command, setting pointers in    */
  /* the args[] array to point to each of the arguments, and null-   */
  /* terminating each one of them.                                   */

  /* LOOP INVARIANT:                                                 */
  /* pos points to the next character of the command to be examined. */
  /* i indicates the next element of the args[] array to be loaded.  */
  i = 0;                               /* Start with args[0]         */
  for (pos=cmd; pos<end; pos++) {

    while (*pos==' ' || *pos=='\t') {
      pos++;                           /* Skip to first non-white    */
    } /* endwhile */

    if (*pos == '\0')                  /* If we're at the end,       */
      break;                           /* get out.                   */

    /* If at this point, we've used up all but one of the available  */
    /* elements of our args[] array, let the user know, and we must  */
    /* terminate.                                                    */
    if (i==MAX_COMMAND_ARGS)
      report_exception(MSG_TOO_MANY_CMD_ARGS);

    args[i++] = pos;                   /* Point to current argument  */
                                       /* and advance i to next      */
                                       /* element of args[]          */

    while (*pos!=' ' && *pos!='\t' && *pos!='\0') {
      pos++;                           /* Look for next whitespace   */
    } /* endwhile */                   /* or end of command          */

    *pos = '\0';                       /* Null-terminate this arg    */

  } /* endfor */

  /* Finally, put a null pointer in args[] to indicate the end.      */
  args[i] = NULL;
}






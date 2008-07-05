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
/*    SysCommand     - Method to invoke a subcommand handler                  */
/*                                                                            */
/*  Internal routines:                                                        */
/*    sys_command - Run a command through system command processor.           */
/******************************************************************************/

#include <string.h>                         /* Get strcpy, strcat, etc.       */
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#include "RexxCore.h"                         /* global REXX declarations       */
#include "StringClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"

#include "SystemCommands.h"
#include "RexxInternalApis.h"
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

#define UNKNOWN_COMMAND 127                 /* unknown command return code    */

#define SYSENV "command"                    /* Default cmd environment        */
#define SHELL  "SHELL"                      /* UNIX cmd handler env. var. name*/
#define EXPORT_FLAG 1
#define SET_FLAG    2
#define UNSET_FLAG  3
#define MAX_VALUE   1280

extern int putflag;

char * args[MAX_COMMAND_ARGS+1];            /* Array for argument parsing */

int sys_command(const char *cmd, CMD_TYPE local_env_type);
void scan_cmd(const char *parm_cmd, char **args);


/**
 * Retrieve the globally default initial address.
 *
 * @return The string name of the default address.
 */
RexxString *SystemInterpreter::getDefaultAddressName()
{
    return OREF_INITIALADDRESS;
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
RexxObject *SystemInterpreter::invokeHostCommand(
  RexxActivation    * activation,      /* activation working on behalf of   */
  RexxActivity      * activity,        /* activity running on               */
  RexxString        * environment,     /* target address                    */
  RexxString        * command,         /* command to issue                  */
  RexxString       ** error_failure )  /* error or failure flags            */
{
  int          rc    = 0;              /* Return code from call             */
  const char  *current_address;        /* Subcom handler that gets cmd      */
  CONSTRXSTRING     rxstrcmd;          /* Command to be executed            */
  unsigned short flags = 0;            /* Subcom error flags                */
  wholenumber_t sbrc  = 0;             /* Subcom return code                */
  RXSTRING     retstr;                 /* Subcom result string              */
  CMD_TYPE     local_env_type;
  const char * shell_cmd;
  size_t       i;
  size_t       length;
  RexxObject * result;

  char     default_return_buffer[DEFRXSTRING];

  *error_failure = OREF_NULL;          /* default to clean call             */

                                       /* set up the RC buffer              */
  MAKERXSTRING(retstr, default_return_buffer, DEFRXSTRING);

                                       /* set up the command RXSTRING       */
  MAKERXSTRING(rxstrcmd, command->getStringData(), command->getLength());

                                       /* get the current environment       */
  current_address = environment->getStringData();

                                   /* convert current_address to a CMD_TYPE */
  local_env_type = cmd_pgm;               /* default to user defined subcom */
  for (i = 0; i < REG_ENVTABLE_SIZE; i++)     /* scan the table for a match */
    {
    length = strlen(reg_envtable[i].envname);
       if (environment->getLength() == length &&
           !memcmp(current_address,               /* and if names are equal */
           reg_envtable[i].envname,           /* this is the new environment*/
           length))  {                             /* otherwise use default */
              local_env_type = reg_envtable[i].envtype;
              break;
             } /* endif */
     } /* endfor */

  sbrc = 0;                               /* set initial subcom return code */
                                       /* get ready to call the function    */

// BEGIN CRITICAL window here -->>  absolutely no kernel calls inside this window
  {
      CalloutBlock releaser;
      rc=RexxCallSubcom(current_address, NULL, &rxstrcmd, &flags, &sbrc, &retstr);
  }
// END CRITICAL window here -->>  kernel calls now allowed again

  /****************************************************************************/
  /* If subcom isn't registered and it happens to be the current system cmd   */
  /* handler, try passing it on to the system to handle.                      */
  /****************************************************************************/
  if (rc == RXSUBCOM_NOTREG) {
    if (!(local_env_type == cmd_pgm)) {

      shell_cmd = command->getStringData();

      {
          UnsafeBlock releaser;

          rc = sys_command(shell_cmd, local_env_type);     /* issue the command */
      }
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
    if (strcmp(current_address,SYSENV)==0) {

      if (sbrc == UNKNOWN_COMMAND)              /* is this unknown command? */
                                             /* send failure condition back */
        flags |= (unsigned short)RXSUBCOM_FAILURE;
      else if (sbrc != 0)                                 /* command error? */
                                               /* send error condition back */
        flags |= (unsigned short)RXSUBCOM_ERROR;
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

        SystemInterpreter::releaseResultMemory(retstr.strptr);                  /* free it... */
    }
    else
      result = IntegerZero;                         /* got a zero return code */

  /****************************************************************************/
  /* Check error flags from subcom handler and if needed, stick condition     */
  /* into result array.                                                       */
  /****************************************************************************/
    if (flags&(unsigned short)RXSUBCOM_ERROR)   /* If error flag set          */
      *error_failure = OREF_ERRORNAME;          /* send error condition back  */
                                                /* If failure flag set        */
    else if (flags&(unsigned short)RXSUBCOM_FAILURE)
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
bool sys_process_export(const char * cmd, int *rc, int flag)
{
  char *Env_Var_String = NULL;         /* Environment variable string for   */
  size_t size, allocsize;              /* size of the string                */
  char      **Environment;             /* environment pointer               */
  char  *np;
  size_t i,j,k,l,iLength, copyval;
  char   namebufcurr[1281];             /* buf for extracted name            */
  char   cmd_name[1281];                /* name of the envvariable setting   */
  char   *array, *runarray, *runptr, *endptr, *maxptr;
  char   temparray[1281];
  const char *st;
  char  *tmpptr;
  char   name[1281];                    /* is the name + value + =           */
  char   value[1281];                   /* is the part behind =              */
  char  *del = NULL;                    /* ptr to old unused memory          */
  char  *hit = NULL;
  bool   HitFlag = false;
  l = 0;
  j = 0;
  allocsize = 1281 * 2;

  memset(temparray, '\0', sizeof(temparray));

  Environment = getEnvironment();       /* get the environment               */
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
     return false;
  }

  if(!putflag)
  {                        /* first change in the environment ? */
    /* copy all entries to dynamic memory                                   */
    for(;*Environment != NULL;Environment++)
    {                                  /*for all entries in the env         */
      size = strlen(*Environment)+1;   /* get the size of the string        */
      Env_Var_String = (char *)malloc(size); /* and alloc space for it      */
      memcpy(Env_Var_String,*Environment,size);/* copy the string           */
      putenv(Env_Var_String);          /* and chain it in                   */
    }
  }
  putflag = 1;                         /* prevent do it again               */
  Environment = getEnvironment();      /* reset the environment pointer     */

/* do we have a assignment operator? If not return true           */
/* The operating system treads this like no command, and so do we */

  if ( !(strchr(name, '=')) && (flag != UNSET_FLAG) ) /*only set and export */
  {
/* we do not have a assignment operator, but maybe a '|' for a    */
/* controlled output                                              */
     if ( (strchr(name, '|'))  || (strchr(name, '>')) || (strstr(name, ">>")) )
     {
       return false;
     }
     else
     {
       *rc = 0;
       return true;
     }
  }

/* no '=' for unset, so force a shell error message               */

  if ( (strchr(name, '=')) && (flag == UNSET_FLAG) )
  {
     return false;
  }

  for(i=0;(name[i]!='=')&&(i<iLength);name[i++])
  {
     cmd_name[i] = name[i];
  }

/* lets try handling variables in the assignment string */

  cmd_name[i]  = '\0';                /* copy the terminator           */

  i++;                                /* the place after'=' */

/* lets search the value for variable part(s)  */

  strcpy(value, &(name[i]));         /* value contains the part behind '=' */
  array = (char *) malloc(1281);
  strcpy(array, cmd_name);
  array[strlen(cmd_name)] = '=';
  array[i] = '\0';                   /* copy the terminator           */
  runarray = array + strlen(array);
  runptr = value;
  endptr = runptr + strlen(value);   /*this is the end of the input*/
  maxptr = array + MAX_VALUE -1;     /* this is the end of our new string */

  while((tmpptr = (strchr(runptr, '$'))) != 0)
  {
    Environment = getEnvironment();  /* get the beginning of the environment*/
    HitFlag = true;          /* if not true inputvalue= outputvalue*/
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
       *runarray = '\0';
       runptr = tmpptr;              /* now runptr is at the place of $ */
    }
    runptr++;
    for (j = 0;(*runptr != '/') && (*runptr != ':') && (*runptr != '$') &&
               (*runptr); j++)
    {
      memcpy(&(temparray[j]), runptr,1); /*temparray is the env var to search*/
      runptr++;
    }

    temparray[j] = '\0';                /* lets see what we can do    */
    np = NULL;

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
       *runarray = '\0';
       hit = NULL;
    }
  }   /* end while loop */

  if (HitFlag == true)
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
       *runarray = '\0';
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
     *runarray = '\0';
  }

  Environment = getEnvironment();     /* get the beginning of the environment*/

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
    Env_Var_String = (char *)malloc(size);/* get the memory                    */
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
  return true;
}


/* Handle "cd XXX" command in same process */
bool sys_process_cd(const char * cmd, int * rc)
{
    const char * st;
    const char *home_dir = NULL;            /* home directory path        */
          char *dir_buf = NULL;             /* full directory path        */
    const char *slash;                      /* ptr to '/'                 */
    struct passwd *ppwd;

    st = &cmd[3];
    while ((*st) && (*st == ' ')) st++;
    if ((!*st) || (strlen(cmd) == 2))
    {
      home_dir = getenv("HOME");
      if(!home_dir)
          return false;
      dir_buf = (char *)malloc(strlen(home_dir)+1);
      strcpy(dir_buf, home_dir);
    }                                  /* if no user name            */
    else if(*(st) == '~' && (*(st+1) == '\0' || *(st+1) == '/'|| *(st+1) == ' ' )){
      if(*(st+1) == '/'){              /* if there is a path         */
        st +=2;                        /* jump over '~/'             */
                                       /* get home directory path    */
        home_dir = getenv("HOME");     /* from the environment       */
        if(!home_dir)                  /* if no home dir info        */
          return false;
                                       /* get space for the buf      */
        dir_buf = (char *)malloc(strlen(home_dir)+strlen(st)+1);
        if(!dir_buf)
          return false;
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/%s", home_dir, st);
      }
      else{
                                       /* get home directory path    */
        home_dir = getenv("HOME");     /* from the environment       */
                                       /* get space for the buf      */
        dir_buf = (char *)malloc(strlen(home_dir)+1);
        if(!dir_buf)
          return false;
        sprintf(dir_buf, "%s/", home_dir);
      }
    }
    else if(*(st) == '~'){             /* cmd is '~username...'      */
      st++;                            /* jump over '~'              */
      slash = strchr(st,'/');          /* search for '/'             */
      if(!slash){                      /* if no '/'                  */
                                       /* rest of string is username */
        ppwd = getpwnam(st);           /* get info about the user    */
                                       /* get space for the buf      */
        dir_buf = (char *)malloc(strlen(ppwd->pw_dir)+1);
        if(!dir_buf)
          return false;
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/", ppwd->pw_dir);
      }
      else{                            /* there is a slash           */
        char username[256];            // need to copy the user name
        memcpy(username, st, slash - st);
        username[slash - st] = '\0';

        ppwd = getpwnam(username);     /* get info about the user    */
        slash++;                       /* step over the slash        */
                                       /* get space for the buf      */
        dir_buf = (char *)malloc(strlen(ppwd->pw_dir)+strlen(slash)+1);
        if(!dir_buf)
          return false;
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/%s", ppwd->pw_dir, slash);
      }
    }

    *rc = chdir(dir_buf);

    free(dir_buf);
    return true;
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
int  sys_command(const char *cmd, CMD_TYPE local_env_type)
{
  int         rc;                      /* Return code                       */
  int         pid;                     /* process id of child from fork     */
  int         status;
#ifdef LINUX
  int         iErrCode = 0;
#endif

  /* execute 'cd' in the same process */
  char        tmp[8];


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
     if (!Utilities::strCaselessCompare("unset ", tmp))
     {
        if (sys_process_export(cmd, &rc, UNSET_FLAG))
             return rc;
     }
     strncpy(tmp, cmd, 7);
     tmp[7] = '\0';
     if (!Utilities::strCaselessCompare("export ", tmp))
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
    pid = fork();

    if  (pid != 0)                         /* spawn a child process to run the  */
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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        break;
      case cmd_ksh:
        execl("/bin/ksh", "ksh", "-c", cmd, NULL);
        break;
      case cmd_bsh:
        execl("/bin/bsh", "bsh", "-c", cmd, NULL);
        break;
      case cmd_csh:
        execl("/bin/csh", "csh", "-c", cmd, NULL);
        break;
      case cmd_bash:
        execl("/bin/bash", "bash", "-c", cmd, NULL);
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
        execl("/bin/sh", "sh", "-c", cmd, NULL);
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

void scan_cmd(const char *parm_cmd, char **argPtr)
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
      reportException(MSG_TOO_MANY_CMD_ARGS);

    argPtr[i++] = pos;                 /* Point to current argument  */
                                       /* and advance i to next      */
                                       /* element of args[]          */

    while (*pos!=' ' && *pos!='\t' && *pos!='\0') {
      pos++;                           /* Look for next whitespace   */
    } /* endwhile */                   /* or end of command          */

    *pos = '\0';                       /* Null-terminate this arg    */

  } /* endfor */

  /* Finally, put a null pointer in args[] to indicate the end.      */
  argPtr[i] = NULL;
}






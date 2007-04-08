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
/* REXX AIX Support                                            aixextf.c      */
/*                                                                            */
/* AIX specific external function lookup and AIX built-in functions.          */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*  aixextf.c - Methods to resolve external function calls.                   */
/*                                                                            */
/*  C methods:                                                                */
/*    sysDirectory- Method for the DIRECTORY BIF                              */
/*    activation_rxfuncadd - Method to support the RXFUNCADD function         */
/*    activation_rxfuncdrop - Method to support the RXFUNCDROP function       */
/*    activation_rxfuncquery - Method to support the RXFUNCQUERY function     */
/*    SysExternalFunction- Method for searching/invoking an external function */
/*                                                                            */
/*  Internal routines:                                                        */
/*    ExecExternalSearch - Search for and execute a REXX program from disk.   */
/*    MacroSpaceSearch - Search for and execute a function in REXX macrospace.*/
/*    RegExternalFunction - Search for and execute a registered external      */
/*                          function.                                         */
/******************************************************************************/
#include <stdio.h>                          /* Get printf, FILE type, etc.    */
#include <string.h>                         /* Get strcpy, strcat, etc.       */
#include <ctype.h>                          /* Get toupper                    */
#include <unistd.h>                         /* get getcwd routine/environ     */
#include <limits.h>                         /* Get PATH_MAX                   */
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#define INCL_RXSUBCOM                       /* Include subcom declares        */
#define INCL_RXFUNC                         /* and external function...       */
#define INCL_RXSYSEXIT                      /* and system exits               */
#define INCL_RXMACRO                        /* and system exits               */

#include "RexxCore.h"                         /* global REXX definitions        */
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "RexxNativeAPI.h"                           /* Lot's of useful REXX macros    */
#include SYSREXXSAA                         /* Include REXX header            */
#include "SubcommandAPI.h"
#include "RexxAPIManager.h"
#include "APIUtilities.h"


#define CMDBUFSIZE      1024                 /* Max size of executable cmd     */
#if defined(AIX)
#define CMDDEFNAME      "/bin/ksh"           /* Default unix sys cmd handler   */
#define COMSPEC         "ksh"                /* unix cmd handler env name      */
#define SYSENV          "ksh"                /* Default AIX  cmd environment   */
#elif defined(OPSYS_SUN)
#define CMDDEFNAME      "/bin/sh"            /* Default unix sys cmd handler   */
#define COMSPEC         "sh"                 /* unix cmd handler env name      */
#define SYSENV          "sh"                 /* Default LINUX cmd environment  */
#else
#define CMDDEFNAME      "/bin/bash"          /* Default unix sys cmd handler   */
#define COMSPEC         "bash"               /* unix cmd handler env name      */
#define SYSENV          "bash"               /* Default AIX  cmd environment   */
#endif


#define DEFEXT          ".CMD"              /* Default OS/2 REXX program ext  */
#define DRVNUM          0x40                /* drive number subtractor        */
#define DIRLEN          256                 /* length of a directory          */
#define FULLSEG         65536L              /* ^4K constant                   */
                                            /* FILESPEC function options      */
#define FILESPEC_PATH         'P'
#define FILESPEC_NAME         'N'
#define CCHMAXPATH PATH_MAX+1

#define MS_PREORDER   0x01                  /* Macro Space Pre-Search         */
#define MS_POSTORDER  0x02                  /* Macro Space Post-Search        */

#define KIOCSOUND   0x4B2F              /* start sound generation (0 for off) */

extern char **environ;

extern char achRexxCurDir[ CCHMAXPATH+2 ];  /* Save current working direct    */

typedef struct _ENVENTRY {                  /* setlocal/endlocal structure    */
  ULONG    size;                            /* size of the saved memory       */
} ENVENTRY;

INT putflag = NULL;                         /* static or dynamic env memory   */

REXXOBJECT BuildEnvlist(void);
RexxMethod *SysRestoreProgramBuffer(PRXSTRING, RexxString *);
void RestoreEnvironment( void * );

APIRET APIENTRY RexxExecuteMacroFunction ( PSZ, PRXSTRING );

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   sysBeep                                      */
/*                                                                   */
/*   Descriptive Name:  BEEP function                                */
/*                                                                   */
/*   Function:          sounds the speaker if possible, else flash   */
/*                      the screen                                   */
/*********************************************************************/

RexxMethod2(REXXOBJECT, sysBeep, long, Frequency, long, Duration)
{
                                        /* console beep for Unix     */
  printf("\a");
  return RexxString("");                /* always returns a null     */
}

/*********************************************************************/
/*                                                                   */
/*   Method Name : sysSetLocal                                       */
/*                                                                   */
/*   Descriptive Name:  SETLOCAL                                     */
/*                                                                   */
/*   Function:          Save all environment variables, drive and    */
/*                      directory of current drive.                  */
/*                                                                   */
/*********************************************************************/

RexxMethod1(REXXOBJECT, sysSetLocal, OSELF, self)
{
  REXXOBJECT     Current;              /* new save block                    */
  REXXOBJECT     Retval;               /* return result                     */

  Current = BuildEnvlist();            /* build the new save block          */
  if (NULLOBJECT == Current)           /* if unsuccessful return zero       */
    Retval = TheFalseObject;
  else {
                                       /* Have Native Actiovation           */
    REXX_PUSH_ENVIRONMENT(Current);    /*  update environemnt list          */
    Retval = TheTrueObject;            /* this returns one                  */
  }
  return Retval;                       /* return success/failure            */
}


/*********************************************************************/
/*                                                                   */
/*   method Name:  sysEndLocal                                       */
/*                                                                   */
/*   Descriptive Name:  ENDLOCAL                                     */
/*                                                                   */
/*   Function:          restore all previous environment variables   */
/*                      drive and current directory.                 */
/*                                                                   */
/*********************************************************************/

RexxMethod0(REXXOBJECT, sysEndLocal)
{
  REXXOBJECT     Current;              /* new save block                    */
  REXXOBJECT     Retval;               /* return result                     */

                                       /* retrieve top environment          */
  Current =  REXX_POP_ENVIRONMENT();   /*  block, if ixisted.               */
  if (TheNilObject == Current)         /* nothing saved?                    */
    Retval = TheFalseObject;           /* return failure value              */
  else {
                                       /* restore everything                */
    RestoreEnvironment(buffer_address(Current));
    Retval = TheTrueObject;            /* this worked ok                    */
  }
  return Retval;                       /* return function result            */
}


/********************************************************************
* Function:  resolve_tilde(path)                                    *
*                                                                   *
* Purpose:   Resolves path names including '~'.                     *
*                                                                   *
* RC:        Returns the absolute path in new allocated space.      *
*                                                                   *
*********************************************************************/
PCHAR resolve_tilde(PCHAR path)
{
    char * st;
    PCHAR  home_dir = NULL;            /* home directory path        */
    PCHAR  dir_buf = NULL;             /* full directory path        */
    PCHAR  slash;
    CHAR   username[100];
    struct passwd *ppwd;
    INT alloc_flag = 0;

    st = path;
                                       /* if no user name            */
    if(*(st) == '~' && (*(st+1) == '\0' || *(st+1) == '/'|| *(st+1) == ' ' )){
      if(*(st+1) == '/'){              /* if there is a path         */
        st +=2;                        /* jump over '~/'             */
                                       /* get home directory path    */
        home_dir = getenv("HOME");     /* from the environment       */
        if(!home_dir)                  /* if no home dir info        */
          return (0);
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(home_dir)+strlen(st)+2);
        if(!dir_buf)
          return (0);
                               /* merge the strings          */
        sprintf(dir_buf, "%s/%s", home_dir, st);
        st = dir_buf;                  /* directory change to        */
        alloc_flag = 1;
      }
      else{
                                       /* get home directory path    */
        home_dir = getenv("HOME");     /* from the environment       */
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(home_dir)+2);
        if(!dir_buf)
          return (0);
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
    if(ppwd == NULL){                  /* no user                    */
      return 0;                        /* nothing happend            */
    }
        dir_buf = (PCHAR)malloc(strlen(ppwd->pw_dir)+2);
        if(!dir_buf)
          return (0);
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/", ppwd->pw_dir);
        alloc_flag = 1;
      }
      else{                            /* there is a slash           */
                                       /* copy the username into a   */
                                       /* local buffer; 100 bytes    */
                                       /* should be big enough       */
                                       /* fixes bug 1695834          */
        memcpy(username, st, slash-st);
        username[slash-st] = '\0';
        ppwd = getpwnam(username);     /* get info about the user    */
        slash++;                       /* step over the slash        */
                                       /* get space for the buf      */
        dir_buf = (PCHAR)malloc(strlen(ppwd->pw_dir)+strlen(slash)+2);
        if(!dir_buf)
          return (0);
                                       /* merge the strings          */
        sprintf(dir_buf, "%s/%s", ppwd->pw_dir, slash);
        alloc_flag = 1;
      }
      st = dir_buf;                    /* directory change to        */
    }
    if(!alloc_flag)
      return 0;                        /* nothing happend            */
    return st;
}

/****************************************************************************/
/* sysDirectory                                                             */
/****************************************************************************/
RexxMethod1(REXXOBJECT, sysDirectory, CSTRING, dir)
{
//char buffer[CCHMAXPATH+2];
  APIRET rc;
  PCHAR rdir;                           /* resolved path */

  rc = 0;
  if (dir != NO_CSTRING){              /* if new directory is not null,     */
    if(*dir == '~'){
      rdir = resolve_tilde(dir);
      rc = chdir(rdir);
      free(rdir);
    }
    else
      rc = chdir(dir);                   /* change to the new directory     */
  }

  if (!getcwd(achRexxCurDir, CCHMAXPATH) || (rc != 0)) /* Get current working direct */
  {
     strncpy( achRexxCurDir, getenv("PWD"), CCHMAXPATH);
     achRexxCurDir[CCHMAXPATH - 1] = '\0';
     if ((achRexxCurDir[0] != '/' ) || (rc != 0))
       return RexxString("");                /* No directory returned       */
  }
//strcpy(achRexxCurDir, buffer);             /* Save current working direct */

  return RexxString(achRexxCurDir);          /* Return the current directory*/
}


/*****************************************************************************/
/* sysFilespec                                                               */
/*****************************************************************************/
RexxMethod2 (REXXOBJECT, sysFilespec, CSTRING, Option, CSTRING, Name)
{
  LONG       NameLength;               /* file name length                  */
  PCHAR      ScanPtr;                  /* scanning pointer                  */
  PCHAR      EndPtr;                   /* end of string                     */
  PCHAR      PathPtr;                  /* path pointer                      */
  PCHAR      PathEnd;                  /* path end pointer                  */
  REXXOBJECT Retval;                   /* return value                      */

                                       /* required arguments missing?       */
  if (Option == NO_CSTRING || strlen(Option) == 0 || Name == NO_CSTRING)
                                       /* raise an error                    */
    send_exception(Error_Incorrect_call);

  NameLength = strlen(Name);           /* get filename length               */

  EndPtr = Name + (NameLength - 1);    /* point to last character           */
  PathEnd = strrchr(Name, '/');        /* find the last slash in Name       */


  Retval = OREF_NULLSTRING;            /* set the default return value      */


  switch (toupper(*Option)) {            /* process each option               */
      case FILESPEC_PATH:                /* extract the path                  */
      {
         if (PathEnd)                    /* if there is a path spec. , return */
                                         /* up to and including last slash.   */
                                         /* else return OREF_NULLSTRING       */
           Retval = RexxStringL(Name, PathEnd - Name + 1);
      }
      break;                           /* finished                            */

      case FILESPEC_NAME:              /* extract the file name               */
      {                                /* everything to right of slash        */
         if ((PathEnd) && (PathEnd != EndPtr))
            Retval = RexxStringL(PathEnd + 1, EndPtr - PathEnd);

         if (!PathEnd)                 /* there was no path spec.             */
            Retval = RexxString(Name);
      }
      break;                           /* finished                          */

    default:                           /* unknown option                    */
                                       /* raise an error                    */
      send_exception(Error_Incorrect_call);
  }
  return Retval;                       /* return extracted part             */
}

/******************************************************************************/
/* activation_rxfuncadd - Method to support RXFUNCADD function                */
/******************************************************************************/
RexxMethod3(REXXOBJECT,sysRxfuncadd,CSTRING,name,CSTRING,module,CSTRING,proc)
{
                                       /* must have two arguments           */
  if (name == NO_CSTRING || module == NO_CSTRING)
                                       /* raise an error                    */
    send_exception(Error_Incorrect_call);
  if (proc == NO_CSTRING)              /* no procedure given?               */
    proc = name;                       /* use the defined name              */
                                       /* try to register the function      */

  if ((RexxRegisterFunctionDll((PSZ)name, (PSZ)module, (PSZ)proc)) == RXFUNC_NOTREG){
    return TheTrueObject;              /* this failed                       */
  } else {
    return TheFalseObject;             /* this worked ok                    */
  }
}


/******************************************************************************/
/* activation_rxfuncdrop - Method to support RXFUNCDROP function              */
/******************************************************************************/
RexxMethod1(REXXOBJECT,sysRxfuncdrop,CSTRING,name)
{
  if (name == NO_CSTRING)              /* must have a name                  */
                                       /* raise an error                    */
    send_exception(Error_Incorrect_call);
                                       /* try to drop the function          */
  if (!RexxDeregisterFunction((PSZ)name))
    return TheFalseObject;
  else
    return TheTrueObject;

}


/******************************************************************************/
/* activation_rxfuncquery - Method to support RXFUNCQUERY function            */
/******************************************************************************/
RexxMethod1(REXXOBJECT,sysRxfuncquery,CSTRING,name)
{
  if (name == NO_CSTRING)              /* must have a name                  */
                                       /* raise an error                    */
    send_exception(Error_Incorrect_call);
    if (!RexxQueryFunction((PSZ)name)) /* is it not there?                  */
    return TheFalseObject;             /* this failed  (function found!)    */
    else
      return TheTrueObject;            /* this worked ok  (no function!)    */
}

/******************************************************************************/
/* Name:       ExecExternalSearch                                             */
/*                                                                            */
/* Arguments:  target - Name of external function (string REXXOBJECT)         */
/*             parent    - Full name of calling program                       */
/*             argarray - Argument array (array REXXOBJECT)                   */
/*             calltype - Type of call (string REXXOBJECT)                    */
/*                                                                            */
/* Returned:   rc - Boolean return code:                                      */
/*                    0 means we didn't find the REXX program                 */
/*                    1 means we found and executed the REXX program          */
/*             result - Result returned from REXX program                     */
/*                                                                            */
/* Notes:      Searches for a REXX program with the target name and extension */
/*             and if it finds one, runs the exec passing back the result.    */
/******************************************************************************/
BOOL ExecExternalSearch(
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxString     * parent,             /* Parent program                    */
  RexxObject    ** arguments,          /* Argument array                    */
//RexxArray      * argarray,           /* Argument array                    */
  size_t           argcount,           /* the count of arguments            */
  RexxString     * calltype,           /* Type of call                      */
  RexxObject    ** result )            /* Result of function call           */
{
                                       /* have activation do the call       */
//return activation->callExternalRexx(target, parent, argarray, calltype, result);
  return activation->callExternalRexx(target, parent, arguments, argcount, calltype, result);
}

/******************************************************************************/
/* Name:       MacroSpaceSearch                                               */
/*                                                                            */
/* Function:   Searches for a function within the REXX macrospace.  If the    */
/*             target function is found, it executes the function and passes  */
/*             back the rc in result.                                         */
/******************************************************************************/
BOOL MacroSpaceSearch(
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxObject    ** arguments,          /* Argument array                    */
  size_t           argcount,           /* the count of arguments            */
//RexxArray      * argarray,           /* Argument array                    */
  RexxString     * calltype,           /* Type of call                      */
  BOOL             order,              /* Pre/Post order search flag        */
  RexxObject    ** result )            /* Result of function call           */
{
  USHORT       Position;               /* located macro search position     */
  PCHAR        MacroName;              /* ASCII-Z name version              */
  RXSTRING     MacroImage;             /* target macro image                */
  RexxMethod * Routine;                /* method to execute                 */

  MacroName = target->stringData;      /* point to the string data          */
                                       /* did we find this one?             */
  if (RexxQueryMacro(MacroName, &Position) == 0) {
                                       /* but not at the right time?        */
    if (order == MS_PREORDER && Position == RXMACRO_SEARCH_AFTER)
      return FALSE;                    /* didn't really find this           */
                                       /* get image of function             */
      /* The ExecMacro func returns a ptr to the shared memory. So we must  */
      /* call APISTARTUP to be sure that the ptr remains valid.             */
      APISTARTUP(MACROCHAIN);

      if (RexxExecuteMacroFunction(MacroName, &MacroImage) != 0)
      {
         APICLEANUP(MACROCHAIN);
         return FALSE;
      }

                                       /* unflatten the method now          */
      Routine = SysRestoreProgramBuffer(&MacroImage, target);
                                       /* run as a call                     */
      APICLEANUP(MACROCHAIN);          /* now we have a copy of the routine */
      if (Routine == OREF_NULL) return FALSE;
//  *result = Routine->call(activity, (RexxObject *)activation, target, argarray, calltype, OREF_NULL, EXTERNALCALL);
    *result = Routine->call(activity, (RexxObject *)activation, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL);
    /* @CHM014A - merge (class) definitions from macro with current settings */
    activation->settings.parent_source->mergeRequired(Routine->code->u_source);
    return TRUE;                       /* return success we found it flag   */
  }
  return FALSE;                        /* nope, nothing to find here        */
}


/******************************************************************************/
/* Name:       RegExternalFunction                                            */
/*                                                                            */
/* Arguments:  target - Name of external function (string REXXOBJECT)         */
/*             argarray - Argument array (array REXXOBJECT)                   */
/*             calltype - Type of call (string REXXOBJECT)                    */
/*                                                                            */
/* Returned:   rc - Boolean return code:                                      */
/*                    0 means the function wasn't registered                  */
/*                    1 means we found and executed the registered function   */
/*             result - Result from running registered function               */
/*                                                                            */
/* Notes:      Queries the REXX External Function API to see if the target    */
/*             function is registered.  If it is, it asks the API to invoke   */
/*             the function, passing the rc back in result.                   */
/******************************************************************************/
BOOL RegExternalFunction(
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxObject    ** arguments,          /* Argument array                    */
  size_t           argcount,           /* the count of arguments            */
//RexxArray      * argarray,           /* Argument array                    */
  RexxString     * calltype,           /* Type of call                      */
  RexxObject    ** result )            /* Result of function call           */
{
  PCHAR         funcname;              /* Pointer to function name          */
  PCHAR         queuename;             /* Pointer to active queue name      */
  long          rc;                    /* RexxCallFunction return code      */
//long          argcount;              /* Number of args in arg array       */
//long          argindex;              /* Index into arg array              */
  size_t    argindex;                  /* Index into arg array              */
  PRXSTRING     argrxarray;            /* Array of args in PRXSTRING form   */
  RXSTRING      funcresult;            /* Function result                   */
  RexxString  * argument;              /* current argument                  */
  USHORT        functionrc;            /* Return code from function         */

                                       /* default return code buffer        */
  CHAR      default_return_buffer[DEFRXSTRING];

  funcname = target->stringData;       /* point to the function name        */
                                       /* Do we have the function?          */
  if (RexxQueryFunction(funcname) != 0) {
                                       /* this a system routine?            */
    if (memicmp(funcname, "SYS", 3) == 0) {
                                       /* try to register SysLoadFuncs      */
      if (RexxRegisterFunctionDll((PSZ)"SYSLOADFUNCS", (PSZ)"rexxutil", (PSZ)"SysLoadFuncs") == 0) {
                                       /* first registration?               */
                                       /* set up an result RXSTRING         */
        MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));
                                       /* call the function loader          */
        RexxCallFunction("SYSLOADFUNCS", 0, (PRXSTRING)NULL, &functionrc, &funcresult, "");

      }
    }
                                       /* Do we have the function?          */
    if (RexxQueryFunction(funcname) != 0)
      return FALSE;                    /* truely not found                  */
  }

//argcount = argarray->size();         /* Get number of args                */

  /* allocate enough memory for all arguments */
  /* at least one item needs to be allocated to prevent error reporting */
  argrxarray = (PRXSTRING) SysAllocateResultMemory(sizeof(RXSTRING)*max(argcount,1));
  if (argrxarray == OREF_NULL)    /* memory error?                   */
      report_exception(Error_System_resources);
                                       /* create RXSTRING arguments         */
  for (argindex=0; argindex<argcount; argindex++) {
                                       /* get the next argument             */
//  argument = (RexxString *)argarray->get(argindex + 1);
    argument = (RexxString *)arguments[argindex];
    if (argument != OREF_NULL) {       /* have an argument?                 */
                                       /* force to string form              */
      argument = argument->stringValue();
      /* replace in argArray to help protect from being GC'd. */
      /* We're replacing this references in place, since this is */
      /* the last search in the order, and we won't be wiping out */
      /* something that will still be required */
//    argarray->put((RexxObject *)argument, argindex+1);
      arguments[argindex] = argument;
                                       /* set the RXSTRING length           */
      argrxarray[argindex].strlength = argument->length;
                                       /* and pointer                       */
      argrxarray[argindex].strptr = argument->stringData;
    }
    else {                             /* have an omitted argument          */
                                       /* give it zero length               */
      argrxarray[argindex].strlength = 0;
                                       /* and a zero pointer                */
      argrxarray[argindex].strptr = NULL;
    }
  }
                                       /* get the current queue name        */
  queuename = SysGetCurrentQueue()->stringData;
                                       /* make the RXSTRING result          */
  MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));

/* CRITICAL window here -->>  ABSOLUTELY NO KERNEL CALLS ALLOWED            */

                                       /* get ready to call the function    */
  activity->exitKernel(activation, OREF_SYSEXTERNALFUNCTION, TRUE);
                                       /* now call the external function    */
  rc = RexxCallFunction(funcname, argcount, argrxarray, &functionrc, &funcresult, queuename);
  activity->enterKernel();             /* now re-enter the kernel           */

/* END CRITICAL window here -->>  kernel calls now allowed again            */

  SysReleaseResultMemory(argrxarray);

  if (rc == 0) {                       /* If good rc from RexxCallFunc      */
    if (functionrc == 0) {             /* If good rc from function          */
      if (funcresult.strptr) {         /* If we have a result, return it    */
                                       /* make a string result              */
        *result = new_string(funcresult.strptr, funcresult.strlength);
        save(*result);
                                       /* user give us a new buffer?        */
        if (funcresult.strptr != default_return_buffer )
                                       /* free it                           */
            SysReleaseResultMemory(funcresult.strptr);
      }
      else
        *result = OREF_NULL;           /* nothing returned                  */
    }
    else                               /* Bad rc from function, signal      */
                                       /* error                             */
      report_exception1(Error_Incorrect_call_external, target);
  }
  else                                 /* Bad rc from RexxCallFunction,     */
    report_exception1(Error_Routine_not_found_name, target);
  return TRUE;                         /* We found this                     */
}
/******************************************************************************/
/* Name:       SysExternalFunction                                            */
/*                                                                            */
/* Notes:      Handles searching for and executing an external function.  The */
/*             search order is:                                               */
/*               1) Macro-space pre-order functions                           */
/*               2) Registered external functions                             */
/*               3) SOM methods                                               */
/*               4) REXX programs with same extension (if applicable)         */
/*               5) REXX programs with default extension                      */
/*               6) Macro-space post-order functions                          */
/******************************************************************************/
RexxObject * SysExternalFunction(
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxString     * parent,             /* Parent program                    */
  RexxObject    ** arguments,          /* Argument array                    */
  size_t           argcount,           /* count of arguments                */
//RexxArray      * argarray,           /* Argument array                    */
  RexxString     * calltype,           /* Type of call                      */
  BOOL           * foundFnc)
{
  RexxObject * result;                 /* Init function result to null      */
  RXSTRING     funcresult;             /* Function result                   */
  USHORT       functionrc;             /* Return code from function         */
  CHAR      default_return_buffer[10]; /* default return code buffer        */

  *foundFnc = TRUE;
                                       /* check for macrospace first        */
//if (!MacroSpaceSearch(activation, activity, target, argarray, calltype, MS_PREORDER, &result)) { MICMICD
  if (!MacroSpaceSearch(activation, activity, target, arguments, argcount, calltype, MS_PREORDER, &result)) {
                                       /* no luck try for a registered func */
//  if (!RegExternalFunction(activation, activity, target, argarray, calltype, &result)) {
    if (!RegExternalFunction(activation, activity, target, arguments, argcount, calltype, &result)) {
                                       /* no go for an external file        */
//    if (!ExecExternalSearch(activation, activity, target, parent, argarray, calltype, &result)) {
      if (!ExecExternalSearch(activation, activity, target, parent, arguments, argcount, calltype, &result)) {
                                       /* last shot, post-order macro space */
                                       /* function.  If still not found,    */
                                       /* then raise an error               */
//      if (!MacroSpaceSearch(activation, activity, target, argarray, calltype, MS_POSTORDER, &result)) {
//          report_exception1(Error_Routine_not_found_name, target);
        if (!MacroSpaceSearch(activation, activity, target, arguments, argcount, calltype, MS_POSTORDER, &result)) {
          *foundFnc = FALSE;
        }
      }
    }
  }
  return result;                       /* return result                     */
}

/******************************************************************************/
/* Name:       SysGetMacroCode                                                */
/*                                                                            */
/* Notes:      Retrieves the RexxMethod from a named macro in macrospace.     */
/*             Search order is specified as second parameter.                 */
/******************************************************************************/
RexxMethod * SysGetMacroCode(
  RexxString     * MacroName)
{
  RXSTRING       MacroImage;
  RexxMethod   * method = OREF_NULL;

  MacroImage.strptr = NULL;

  /* The ExecMacro func returns a ptr to the shared memory. So we must  */
  /* call APISTARTUP to be sure that the ptr remains valid.             */
  APISTARTUP(MACROCHAIN);

  if (RexxExecuteMacroFunction(MacroName->stringData, &MacroImage) == 0)
    method = SysRestoreProgramBuffer(&MacroImage, MacroName);

  APICLEANUP(MACROCHAIN);          /* now we have a copy of the routine */
  return method;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   BuildEnvlist                                 */
/*                                                                   */
/*   Descriptive Name:  Build saved environment block                */
/*                                                                   */
/*   Function:          Builds a block containing all of the         */
/*                      environment variables, the current drive     */
/*                      and the current directory.                   */
/*                                                                   */
/*********************************************************************/

REXXOBJECT BuildEnvlist()
{
  REXXOBJECT  newBuffer;               /* Buffer object to hold env  */
  PCHAR      * Environment;            /* environment pointer        */
  ULONG       size = 0;                /* size of the new buffer     */
  PSZ         curr_dir;                /* current directory          */
  PSZ         New;                     /* starting address of buffer */
  Environment = environ;               /* get the ptr to the environ */

  for(;*Environment != NULL;Environment++){
    size += strlen(*Environment);      /* calculate the size for all */
    size++;                            /* environment variables+'\0' */
  }                                    /* now get current dir        */
  if(!size)
    return OREF_NULL;                  /* no envrionment !           */
  if (!(curr_dir=(PSZ)malloc(CCHMAXPATH+2)))/* malloc storage for cwd*/
    report_exception(Error_System_service);

  if (!getcwd(curr_dir,CCHMAXPATH))    /* get current directory      */
  {
     strncpy( achRexxCurDir, getenv("PWD"), CCHMAXPATH);
     achRexxCurDir[CCHMAXPATH - 1] = '\0';
     if (achRexxCurDir[0] != '/' )
       report_exception(Error_System_service);/* Complain if it fails*/
  }
  size += strlen(curr_dir);            /* add the space for curr dir */
  size++;                              /* and its terminating '\0'   */
  size += 4;                           /* this is for the size itself*/
                                       /* Now we have the size for   */
                                       /* allocating the new buffer  */
  newBuffer = RexxBuffer(size);        /* let's do it                */
                                       /* Get starting address of buf*/
  New = (char*)buffer_address(newBuffer);
  ((ENVENTRY*)New)->size = size;       /* first write the size       */
  New +=4;                             /* update the pointer         */
                                       /* now write the curr dir     */
  memcpy(New,curr_dir,strlen(curr_dir));
  New += strlen(curr_dir);             /* update the pointer         */
  memcpy(New,"\0",1);                  /* write the terminator       */
  New++;                               /* update the pointer         */
  Environment = environ;               /* reset to begin of environ  */
                                       /* Loop through environment   */
                                       /* and copy all entries to the*/
                                       /* buffer, each terminating   */
                                       /* with '\0'                  */
  for(;*Environment != NULL;Environment++){
                                       /* copy the entry             */
    memcpy(New,*Environment,strlen(*Environment));
    New += strlen(*Environment);       /* update the pointer         */
    memcpy(New,"\0",1);                /* write the terminator       */
    New++;                             /* update the pointer         */
  }
  free(curr_dir);                      /* free curr dir buffer       */
  return newBuffer;                    /* return the pointer         */
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   RestoreEnvironment                           */
/*                                                                   */
/*   Descriptive Name:  restores environment saved by Setlocal()     */
/*                                                                   */
/*   Function:          restores the environment variables, current  */
/*                      directory and drive.                         */
/*                                                                   */
/*********************************************************************/

void RestoreEnvironment(
  void *CurrentEnv)                    /* saved environment          */
{
  PSZ  current;                        /* ptr to saved environment   */
  ULONG size;                          /* size of the saved space    */
  ULONG length;                        /* string length              */
  PSZ begin;                           /* begin of saved space       */
  PCHAR      * Environment;            /* environment pointer        */

  PCHAR  del = NULL;                   /* ptr to old unused memory   */
  PCHAR Env_Var_String;                /* enviornment entry          */
  CHAR   namebufsave[256],namebufcurr[256];
  PCHAR  np;
  INT i;

    Environment = environ;             /* get the current environment*/

  begin = current = (PCHAR)CurrentEnv; /* get the saved space        */
  size = ((ENVENTRY*)current)->size;   /* first read out the size    */
  current += 4;                        /* update the pointer         */
  if(chdir(current) == -1)             /* restore the curr dir       */
      send_exception1(Error_System_service_service,
                   RexxArray1(RexxString("ERROR CHANGING DIRECTORY")));
  current += strlen(current);          /* update the pointer         */
  current++;                           /* jump over '\0'             */
  if(!putflag){                        /* first change in the        */
                                       /* environment ?              */
    /* copy all entries to dynamic memory                            */
                                       /*for all entries in the env  */
    for(;*Environment != NULL;Environment++){
      length = strlen(*Environment)+1; /* get the size of the string */
                                       /* and alloc space for it     */
      Env_Var_String = (PCHAR)malloc(length);
      memcpy(Env_Var_String,*Environment,length);/* copy the string  */
      putenv(Env_Var_String);          /* and make it part of env    */
    }
    putflag = 1;                       /* prevent do it again        */
  }
                                       /* Loop through the saved env */
                                       /* entries and restore them   */
  for(;(current-begin)<size;current+=(strlen(current)+1)){
    Environment = environ;             /* get the environment        */
    del = NULL;
    np = current;
                                       /* extract the the name       */
                                       /* from the saved enviroment  */
    for(i=0;(*np!='=')&&(i<255);np++,i++){
      memcpy(&(namebufsave[i]),np,1);  /* copy the character         */
    }
    memcpy(&(namebufsave[i]),"\0",1);  /* copy the terminator        */
                                       /* find the entry in the env  */
    for(;*Environment != NULL;Environment++){
      np = *Environment;
                                       /* extract the the name       */
                                       /* from the current env       */
      for(i=0;(*np!='=')&&(i<255);np++,i++){
        memcpy(&(namebufcurr[i]),np,1);/* copy the character         */
      }
      memcpy(&(namebufcurr[i]),"\0",1);/* copy the terminator        */

      if(!strcmp(namebufsave,namebufcurr)){/* have a match ?         */
        del = *Environment;            /* remember it for deletion   */
        break;                         /* found, so get out of here  */
      }
    }
    if(putenv(current) == -1)
      send_exception1(Error_System_service_service,
        RexxArray1(RexxString("ERROR RESTORING ENVIRONMENT VARIABLE")));
    if(del)                            /* if there was an old entry  */
      free(del);                       /* free it                    */
  }

}

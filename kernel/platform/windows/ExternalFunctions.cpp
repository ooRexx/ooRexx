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
/*  winextf.c - Methods to resolve external function calls.                   */
/*                                                                            */
/*  C methods:                                                                */
/*    sysBeep     - Method for the BEEP BIF                                   */
/*    sysSetLocal - Method for the SETLOCAL BIF                               */
/*    sysEndLocal - Method for the ENDLOCAL BIF                               */
/*    sysDirectory- Method for the DIRECTORY BIF                              */
/*    activation_rxfuncadd - Method to support the RXFUNCADD function         */
/*    activation_rxfuncdrop - Method to support the RXFUNCDROP function       */
/*    activation_rxfuncquery - Method to support the RXFUNCQUERY function     */
/*    SysExternalFunction- Method for searching/invoking an external function */
/*    sysMessageBox - Method to pop up message box                            */
/*                                                                            */
/*  Internal routines:                                                        */
/*    ExecExternalSearch - Search for and execute a REXX program from disk.   */
/*    MacroSpaceSearch - Search for and execute a function in REXX macrospace.*/
/*    RegExternalFunction - Search for and execute a registered external      */
/*                          function.                                         */
/******************************************************************************/
#include <stdio.h>                          /* Get printf, FILE type, etc.    */
#include <string.h>                         /* Get strcpy, strcat, etc.       */
#include <stdlib.h>                         /* Get system, max_path etc...    */
#include <process.h>
#include <direct.h>

#define INCL_RXSUBCOM                       /* Include subcom declares        */
#define INCL_RXFUNC                         /* and external function...       */
#define INCL_RXSYSEXIT                      /* and system exits               */
#define INCL_RXMACRO


#include "RexxCore.h"                         /* global REXX definitions        */
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "RexxNativeAPI.h"                  /* Lot's of useful REXX macros    */
#include SYSREXXSAA                         /* Include REXX header            */
#include "SubcommandAPI.h"                  /* Get private REXXAPI API's      */
#include "RexxAPIManager.h"
#define DEFEXT "REX"                        /* Default OS/2 REXX program ext  */
#define DIRLEN        256                   /* length of a directory          */

#define  MAX_FREQUENCY 32767
#define  MIN_FREQUENCY    37
#define  MAX_DURATION  60000
#define  MIN_DURATION      0

                                            /* FILESPEC function options      */
#define FILESPEC_DRIVE        'D'
#define FILESPEC_PATH         'P'
#define FILESPEC_NAME         'N'

#define MS_PREORDER   0x01                  /* Macro Space Pre-Search         */
#define MS_POSTORDER  0x02                  /* Macro Space Post-Search        */

extern HANDLE apiProtect;

typedef struct _ENVENTRY {                  /* setlocal/endlocal structure    */
  ULONG    DriveNumber;                     /* saved drive                    */
  CHAR     Directory[DIRLEN];               /* saved current directory        */
  PCHAR    Environment;                     /* saved environment segment      */
  CHAR     Variables[1];                    /* start of variable values       */
} ENVENTRY;

RexxMethod *SysRestoreProgramBuffer(PRXSTRING, RexxString *);
void ReplaceEnvironment( PSZ );
REXXOBJECT BuildEnvlist(void);
void RestoreEnvironment( void * );

extern "C" {
APIRET APIENTRY RexxExecuteMacroFunction ( PSZ, PRXSTRING );
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   sysBeep                                      */
/*                                                                   */
/*   Descriptive Name:  BEEP function                                */
/*                                                                   */
/*   Function:          sounds the speaker at frequency Hertz for    */
/*                      specified duration (in milliseconds)         */
/*********************************************************************/

RexxMethod2(REXXOBJECT, sysBeep, long, Frequency, long, Duration)
{
                                       /* out of range?              */
  if (Frequency > MAX_FREQUENCY || Frequency < MIN_FREQUENCY || Duration > MAX_DURATION || Duration < MIN_DURATION)
                                       /* raise an error             */
    send_exception(Error_Incorrect_call);

  Beep(Frequency, Duration);           /* sound beep                 */
  return RexxString("");               /* always returns a null      */
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
  return TheFalseObject;               /* return failure             */
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
  return TheFalseObject;               /* return failure             */
}


/********************************************************************************************/
/* sysDirectory                                                                             */
/********************************************************************************************/
RexxMethod1(REXXOBJECT, sysDirectory, CSTRING, dir)
{
  char buffer[CCHMAXPATH+1];  // retrofit by IH
  int rc = 0;

  if (dir != NO_CSTRING)
//     rc = SetCurrentDirectory(dir);
  {
      if ((strlen(dir) == 2) && (dir[1] == ':'))
         rc = _chdrive(toupper( dir[0] ) - 'A' + 1);
      else
         rc = _chdir(dir);
  }
                                       /* Return the current directory    */
//  return (rc == 0) || (GetCurrentDirectory(CCHMAXPATH,buffer) == NULL) ? RexxString("") : RexxString(buffer);
  return (rc != 0) || (_getcwd(buffer,CCHMAXPATH) == NULL) ? RexxString("") : RexxString(buffer);
}


/********************************************************************************************/
/* sysFilespec                                                                              */
/********************************************************************************************/
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

  EndPtr = Name + NameLength;          /* point to last character           */
  Retval = OREF_NULLSTRING;            /* set the default return value      */

  switch (toupper(*Option)) {          /* process each option               */

    case FILESPEC_DRIVE:               /* extract the drive                 */
      if (NameLength) {                /* have a real string?               */
                                       /* scan for the character            */
        ScanPtr = (char *)memchr(Name, ':', NameLength);
        if (ScanPtr)                   /* found one?                        */
                                       /* create result string              */
          Retval = RexxStringL(Name, ScanPtr - Name + 1);
      }
      break;

    case FILESPEC_PATH:                /* extract the path                  */
      if (NameLength) {
                                       /* find colon or backslash           */
        ScanPtr = mempbrk(Name, ":\\/", NameLength);
        if (ScanPtr) {
          if (*ScanPtr == ':') {       /* found a colon?                    */
            ScanPtr++;                 /* step past the colon               */
            if (ScanPtr < EndPtr) {    /* not last character?               */
              PathEnd = NULL;          /* no end here                       */
                                       /* search for backslashes            */
              PathPtr = mempbrk(ScanPtr, "\\/", EndPtr - ScanPtr);
              while (PathPtr) {        /* while more backslashes            */
                PathEnd = PathPtr;     /* save the position                 */
                                       /* search for more backslashes       */
                PathPtr++;             /* step past the last match          */
                PathPtr = mempbrk(PathPtr, "\\/", EndPtr-PathPtr);
              }
              if (PathEnd)             /* have backslashes?                 */
                                       /* extract the path                  */
                Retval = RexxStringL(ScanPtr, PathEnd - ScanPtr + 1);
            }
          }
          else {
            PathPtr = ScanPtr;         /* save start position               */
            PathEnd = PathPtr;         /* CHM - defect 85: save end pos.    */
            PathPtr++;                 /* step past first one               */
                                       /* search for backslashes            */
            PathPtr = mempbrk(PathPtr, "\\/", EndPtr - PathPtr);
            while (PathPtr) {          /* while more backslashes            */
              PathEnd = PathPtr;       /* save the position                 */
              PathPtr++;               /* step past the last match          */
                                       /* search for more backslashes       */
              PathPtr = mempbrk(PathPtr, "\\/", EndPtr-PathPtr);
            }
                                       /* extract the path                  */
            Retval = RexxStringL(Name, PathEnd - Name + 1); //retrofit by IH
          }
        }
      }
      break;                           /* finished                          */

    case FILESPEC_NAME:                /* extract the file name             */
      if (NameLength) {                /* filename null string?             */
                                       /* find colon or backslash           */
        ScanPtr = mempbrk(Name, ":\\/", NameLength);
        if (ScanPtr) {
          if (*ScanPtr == ':') {       /* found a colon?                    */
            ScanPtr++;                 /* step past the colon               */
            PathEnd = ScanPtr;         /* save current position             */
            PathPtr = mempbrk(ScanPtr, "\\/", EndPtr - ScanPtr);
//          if (PathPtr == NULL)       /* nothing at all?                   */
//            return RexxString("");   /* just return null string           */
            while (PathPtr) {          /* while more backslashes            */
              PathPtr++;               /* step past the last match          */
              PathEnd = PathPtr;       /* save the position                 */
                                       /* search for more backslashes       */
              PathPtr = mempbrk(PathPtr, "\\/", EndPtr-PathPtr);
            }
            if (PathEnd < EndPtr)      /* stuff to return?                  */
                                       /* extract the name                  */
              Retval = RexxStringL(PathEnd, EndPtr - PathEnd);
          }
          else {
            PathPtr = ScanPtr + 1;     /* save start position               */
            PathEnd = PathPtr;         /* step past first one               */
                                       /* search for backslashes            */
            PathPtr = mempbrk(PathPtr, "\\/", EndPtr - PathPtr);
            while (PathPtr) {          /* while more backslashes            */
              PathPtr++;               /* step past the last match          */
              PathEnd = PathPtr;       /* save the position                 */
                                       /* search for more backslashes       */
              PathPtr = mempbrk(PathPtr, "\\/", EndPtr-PathPtr);
            }
            if (PathEnd < EndPtr)      /* stuff to return?                  */
                                       /* extract the name                  */
              Retval = RexxStringL(PathEnd, EndPtr - PathEnd);
          }
        }
        else
          Retval = RexxString(Name);   /* entire string is a name           */
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
  if (!RexxRegisterFunctionDll((PSZ)name,(PSZ)module,(PSZ)proc))
    return TheFalseObject;             /* this failed                       */
  else
    return TheTrueObject;              /* this worked ok                    */
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
    return TheFalseObject;             /* this failed                       */
  else
    return TheTrueObject;              /* this worked ok                    */
}


/******************************************************************************/
/* activation_rxfuncquery - Method to support RXFUNCQUERY function            */
/******************************************************************************/
RexxMethod1(REXXOBJECT,sysRxfuncquery,CSTRING,name)
{
  if (name == NO_CSTRING)              /* must have a name                  */
                                       /* raise an error                    */
    send_exception(Error_Incorrect_call);
  if (!RexxQueryFunction((PSZ)name))   /* is it not there?                  */
    return TheFalseObject;             /* this failed                       */
  else
    return TheTrueObject;              /* this worked ok                    */
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
  size_t           argcount,           /* the count of arguments            */
  RexxString     * calltype,           /* Type of call                      */
  RexxObject    ** result )            /* Result of function call           */
{
                                       /* have activation do the call       */
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
  RexxString     * calltype,           /* Type of call                      */
  BOOL             order,              /* Pre/Post order search flag        */
  RexxObject    ** result )            /* Result of function call           */
{

  USHORT     Position;                 /* located macro search position     */
  PCHAR      MacroName;                /* ASCII-Z name version              */
  RXSTRING   MacroImage;               /* target macro image                */
  RexxMethod * Routine;                /* method to execute                 */

  MacroName = target->stringData;      /* point to the string data          */
                                       /* did we find this one?             */
  if (RexxQueryMacro(MacroName, &Position) == 0) {
                                       /* but not at the right time?        */
    if (order == MS_PREORDER && Position == RXMACRO_SEARCH_AFTER)
      return FALSE;                    /* didn't really find this           */
                                       /* get image of function             */
    if (RexxExecuteMacroFunction(MacroName, &MacroImage) != 0)
        return FALSE;
                                       /* unflatten the method now          */
    Routine = SysRestoreProgramBuffer(&MacroImage, target);

    /* RexxExecuteMacroFunction on Windows allocates a local buffer and copies
       the macro image in this buffer. SysRestoreProgramBuffer allocates its
       own buffer (a MemoryObject). Therefore we have to free the local
       buffer somewhere and do it here. */
    if (MacroImage.strptr) GlobalFree(MacroImage.strptr);

    if (Routine == OREF_NULL) return FALSE;
                                       /* run as a call                     */
    *result = Routine->call(activity, (RexxObject *)activation, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL);
    /* merge (class) definitions from macro with current settings */
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
  RexxString     * calltype,           /* Type of call                      */
  RexxObject    ** result )            /* Result of function call           */
{
  PCHAR     funcname;                  /* Pointer to function name          */
  PCHAR     queuename;                 /* Pointer to active queue name      */
  long      rc;                        /* RexxCallFunction return code      */
  size_t    argindex;                  /* Index into arg array              */
  PRXSTRING argrxarray;                /* Array of args in PRXSTRING form   */
  RXSTRING  funcresult;                /* Function result                   */
  RexxString * argument;               /* current argument                  */
  USHORT    functionrc;                /* Return code from function         */
                                       /* default return code buffer        */
  CHAR      default_return_buffer[DEFRXSTRING];

// retrofit by IH

  funcname = target->stringData;       /* point to the function name        */
  if (RexxQueryFunction(funcname) != 0) {  /* is the function registered ?  */

                                       /* this a system routine?            */
    if (memicmp(funcname, "SYS", 3) == 0) {
      MTXRQ(apiProtect);               /* protect these instructions        */
                                       /* from concurrent access of multiple*/
                                       /* processes                         */
                                       /* try to register SysLoadFuncs      */


      if (RexxQueryFunction("SYSLOADFUNCS") == RXSUBCOM_NOTREG)
      {
        /* SysLoadFunc is not registered */
        /* register and call SysLoadFunc */

        if (RexxRegisterFunctionDll((PSZ)"SYSLOADFUNCS", (PSZ)"REXXUTIL", (PSZ)"SysLoadFuncs") == 0)
        {
                                       /* first registration?               */
                                       /* set up an result RXSTRING         */
          MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));
                                       /* call the function loader          */
          RexxCallFunction("SYSLOADFUNCS", 0, (PRXSTRING)NULL, &functionrc, &funcresult, "");

        }
      }
      else
      {
        /* SysloadFunc is registered, call it */
        MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));
        RexxCallFunction("SYSLOADFUNCS", 0, (PRXSTRING)NULL, &functionrc, &funcresult, "");
      }
      MTXRL(apiProtect);
    }
                                       /* Do we have the function?          */
    if (RexxQueryFunction(funcname) != 0)
      return FALSE;                    /* truely not found                  */
  }

  /* allocate enough memory for all arguments */
  /* at least one item needs to be allocated to prevent error reporting */
  argrxarray = (PRXSTRING) SysAllocateResultMemory(sizeof(RXSTRING)*max(argcount,1));
  if (argrxarray == OREF_NULL)    /* memory error?                   */
      report_exception(Error_System_resources);
                                       /* create RXSTRING arguments         */
  for (argindex=0; argindex<argcount; argindex++) {
                                       /* get the next argument             */
    argument = (RexxString *)arguments[argindex];
    if (argument != OREF_NULL) {     /* have an argument?                 */
                                       /* force to string form              */
      argument = argument->stringValue();
      /* replace in argArray to help protect from being GC'd. */
      /* We're replacing this references in place, since this is */
      /* the last search in the order, and we won't be wiping out */
      /* something that will still be required */
      arguments[argindex] = argument;
                                       /* set the RXSTRING length           */
      argrxarray[argindex].strlength = argument->length;
                                       /* and pointer                       */
      argrxarray[argindex].strptr = argument->stringData;
    }
    else {                           /* have an omitted argument          */
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
  activity->enterKernel();           /* now re-enter the kernel           */

/* END CRITICAL window here -->>  kernel calls now allowed again            */

  SysReleaseResultMemory(argrxarray);

  if (rc == 0) {                     /* If good rc from RexxCallFunc      */
    if (functionrc == 0) {           /* If good rc from function          */
      if (funcresult.strptr) {       /* If we have a result, return it    */
                                       /* make a string result              */
        *result = new_string(funcresult.strptr, funcresult.strlength);
        save(*result);
                                       /* user give us a new buffer?        */
        if (funcresult.strptr != default_return_buffer )
                                       /* free it                           */
            SysReleaseResultMemory(funcresult.strptr);
      }
      else
        *result = OREF_NULL;         /* nothing returned                  */
    }
    else                             /* Bad rc from function, signal      */
                                       /* error                             */
      report_exception1(Error_Incorrect_call_external, target);
  }
  else                               /* Bad rc from RexxCallFunction,     */
    report_exception1(Error_Routine_not_found_name, target);

  return TRUE;                      /* Show what happened                */
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
  RexxString     * calltype,           /* Type of call                      */
  BOOL           * foundFnc)
{
  RexxObject * result;                 /* Init function result to null      */
  //RXSTRING  funcresult;                /* Function result                 */
  //USHORT    functionrc;                /* Return code from function       */
  //CHAR      default_return_buffer[10]; /* default return code buffer      */

  *foundFnc = TRUE;

  if (!MacroSpaceSearch(activation, activity, target, arguments, argcount, calltype, MS_PREORDER, &result)) {
                                       /* no luck try for a registered func */
    if (!RegExternalFunction(activation, activity, target, arguments, argcount, calltype, &result)) {
                                       /* no go for an external file        */
      if (!ExecExternalSearch(activation, activity, target, parent, arguments, argcount, calltype, &result)) {
                                       /* last shot, post-order macro space */
                                       /* function.  If still not found,    */
                                       /* then raise an error               */
        if (!MacroSpaceSearch(activation, activity, target, arguments, argcount, calltype, MS_POSTORDER, &result)) {
//          report_exception1(Error_Routine_not_found_name, target);
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
  if (RexxExecuteMacroFunction(MacroName->stringData, &MacroImage) == 0)
    method = SysRestoreProgramBuffer(&MacroImage, MacroName);

  /* On Windows we need to free the allocated buffer for the macro */
  if (MacroImage.strptr)
    GlobalFree(MacroImage.strptr);

  return method;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   ReplaceEnvironment                           */
/*                                                                   */
/*   Descriptive Name:  Replace environment variable set             */
/*                                                                   */
/*   Function:          resets the entire set of environment         */
/*                      variables, resizing the segment if needed.   */
/*                                                                   */
/*********************************************************************/

void ReplaceEnvironment(
  PSZ      VariableSet )               /* new set of variables       */
{
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
  void *CurrentEnv)                   /* saved environment          */
{
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   RxMessageBox                                 */
/*                                                                   */
/*   Descriptive Name:  RxMessageBox function                        */
/*                                                                   */
/*   Function:          pops up a PM message box when called from    */
/*                      a PM session.                                */
/*   Parameters:                                                     */
/*        Input:                                                     */
/*                      Text   = The message box text.               */
/*                      Title  = The message box title.              */
/*                      Button = The message box button style.       */
/*                      Icon   = The message box icon style.         */
/*                                                                   */
/*********************************************************************/
RexxMethod4(REXXOBJECT,sysMessageBox,CSTRING,Text,CSTRING,Title, CSTRING,Button,CSTRING,Icon)
{
  ULONG       Style;                   /* window style flags         */
  INT         MaxCnt;                  /* Max loop count             */
  INT         Index;                   /* table index                */
  ULONG       rc;                      /* WinMessageBox return code  */

  PSZ    Button_Styles[] =             /* message box button styles  */
    {"OK",
     "OKCANCEL",
     "RETRYCANCEL",
     "ABORTRETRYIGNORE",
     "YESNO",
     "YESNOCANCEL"};

ULONG  Button_Flags[] =                /* message box button styles  */
    {MB_OK,
     MB_OKCANCEL,
     MB_RETRYCANCEL,
     MB_ABORTRETRYIGNORE,
     MB_YESNO,
     MB_YESNOCANCEL};

PSZ    Icon_Styles[] =                 /* message box icon styles    */
    {"HAND",
     "QUESTION",
     "EXCLAMATION",
     "ASTERISK",
     "INFORMATION",
     "WARNING",
     "ERROR",
     "QUERY",
     "NONE",
     "STOP"};

ULONG  Icon_Flags[] =                  /* message box icon styles    */
    {MB_ICONHAND,
     MB_ICONQUESTION,
     MB_ICONEXCLAMATION,
     MB_ICONASTERISK,
     MB_ICONINFORMATION,
     MB_ICONWARNING,
     MB_ICONERROR,
     MB_ICONQUESTION,
     0,
     MB_ICONSTOP};

  if (Text == NULL)                    /* raise an error if empty    */
    send_exception(Error_Incorrect_call);


                                       /* set initial style flags    */
  Style = MB_SETFOREGROUND;            /* make this foreground       */

  if (Button == NULL) Style += MB_OK;  /* set default button style?  */
  else {                               /* check various button styles*/
                                       /* get the number of styles   */
                                       /* search style table         */
    MaxCnt = sizeof(Button_Styles)/sizeof(PSZ);

    for (Index = 0; Index < MaxCnt; Index++) {
                                       /* find a match?               */
      if (!stricmp(Button, Button_Styles[Index])) {
        Style += Button_Flags[Index];  /* add to the style            */
        break;
      }
    }
    if (Index == MaxCnt)               /* if not found raise error          */
      send_exception(Error_Incorrect_call);

  }//end else

  if (Icon == NULL) Style += MB_OK;    /* set default icon style?           */
  else {                               /* check various button styles*/
    MaxCnt = sizeof(Icon_Styles)/sizeof(PSZ);
                                       /* search style table                */
    for (Index = 0; Index < MaxCnt; Index++) {
                                       /* find a match?                     */
      if (!stricmp(Icon,Icon_Styles[Index])) {
        Style += Icon_Flags[Index];    /* add to the style                  */
        break;
      }
    }
    if (Index == MaxCnt)               /* if not found raise error          */
      send_exception(Error_Incorrect_call);

  }// end else

  rc = MessageBox(NULL,                //hWndOwner
                  Text,                // Text
                  Title,               // Title
                  Style);              // Styles

  return RexxInteger( rc );}           // convert to return code


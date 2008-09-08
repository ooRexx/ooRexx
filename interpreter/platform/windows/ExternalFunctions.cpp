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

#include "RexxCore.h"                         /* global REXX definitions        */
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"
#include "PackageManager.hpp"
#include "SystemInterpreter.hpp"

#define DIRLEN        256                   /* length of a directory          */

#define  MAX_FREQUENCY 32767
#define  MIN_FREQUENCY    37
#define  MAX_DURATION  60000
#define  MIN_DURATION      0

                                            /* FILESPEC function options      */
#define FILESPEC_DRIVE        'D'
#define FILESPEC_PATH         'P'
#define FILESPEC_NAME         'N'

typedef struct _ENVENTRY {                  /* setlocal/endlocal structure    */
  size_t   DriveNumber;                     /* saved drive                    */
  char     Directory[DIRLEN];               /* saved current directory        */
  char    *Environment;                     /* saved environment segment      */
  char     Variables[1];                    /* start of variable values       */
} ENVENTRY;

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   sysBeep                                      */
/*                                                                   */
/*   Descriptive Name:  BEEP function                                */
/*                                                                   */
/*   Function:          sounds the speaker at frequency Hertz for    */
/*                      specified duration (in milliseconds)         */
/*********************************************************************/

RexxRoutine2(CSTRING, sysBeep, wholenumber_t, Frequency, wholenumber_t, Duration)
{
                                         /* out of range?              */
    if (Frequency > MAX_FREQUENCY || Frequency < MIN_FREQUENCY)
    {
        RexxArrayObject subs = context->NewArray(4);
        context->ArrayAppend(subs, context->NewStringFromAsciiz("frequency"));
        context->ArrayAppend(subs, context->NumberToObject(MIN_FREQUENCY));
        context->ArrayAppend(subs, context->NumberToObject(MAX_FREQUENCY));
        context->ArrayAppend(subs, context->NumberToObject(Frequency));
        context->RaiseExceptionArray(Rexx_Error_Invalid_argument_range, subs);
        return NULL;
    }
                                         /* out of range?              */
    if (Duration > MAX_DURATION || Duration < MIN_DURATION)
    {
        RexxArrayObject subs = context->NewArray(4);
        context->ArrayAppend(subs, context->NewStringFromAsciiz("duration"));
        context->ArrayAppend(subs, context->NumberToObject(MIN_DURATION));
        context->ArrayAppend(subs, context->NumberToObject(MAX_DURATION));
        context->ArrayAppend(subs, context->NumberToObject(Duration));
        context->RaiseExceptionArray(Rexx_Error_Invalid_argument_range, subs);
        return NULL;
    }

    Beep((DWORD)Frequency, (DWORD)Duration);  /* sound beep                 */
    return "";                           /* always returns a null      */
}


/********************************************************************************************/
/* sysDirectory                                                                             */
/********************************************************************************************/
RexxRoutine1(RexxStringObject, sysDirectory, OPTIONAL_CSTRING, dir)
{
    char buffer[MAX_PATH+1];
    int rc = 0;

    if (dir != NO_CSTRING)
    {
        if ((strlen(dir) == 2) && (dir[1] == ':'))
        {
            rc = _chdrive(toupper( dir[0] ) - 'A' + 1);
        }
        else
        {
            rc = _chdir(dir);
        }
    }
    /* Return the current directory    */
    if (rc != 0 || _getcwd(buffer, MAX_PATH) == NULL)
    {
        return context->NullString();
    }
    else
    {
        return context->NewStringFromAsciiz(buffer);
    }
}


/********************************************************************************************/
/* sysFilespec                                                                              */
/********************************************************************************************/
RexxRoutine2(RexxStringObject, sysFilespec, CSTRING, option, CSTRING, name)
{
  size_t nameLength;                   /* file name length                  */
  const char *scanPtr;                 /* scanning pointer                  */
  const char *endPtr;                  /* end of string                     */
  const char *pathPtr;                 /* path pointer                      */
  const char *pathEnd;                 /* path end pointer                  */

  nameLength = strlen(name);           /* get filename length               */

  endPtr = name + nameLength;          /* point to last character           */

  switch (toupper(*option)) {          /* process each option               */

    case FILESPEC_DRIVE:               /* extract the drive                 */
      if (nameLength > 0) {            /* have a real string?               */
                                       /* scan for the character            */
        scanPtr = (char *)memchr(name, ':', nameLength);
        if (scanPtr != NULL)           /* found one?                        */
        {
            return context->NewString(name, scanPtr - name + 1);
        }
      }
      break;

    case FILESPEC_PATH:                /* extract the path                  */
      if (nameLength > 0) {            /* have a real string?               */
                                       /* find colon or backslash           */
        scanPtr = Utilities::locateCharacter(name, ":\\/", nameLength);
        if (scanPtr != NULL) {
          if (*scanPtr == ':') {       /* found a colon?                    */
            scanPtr++;                 /* step past the colon               */
            if (scanPtr < endPtr) {    /* not last character?               */
              pathEnd = NULL;          /* no end here                       */
                                       /* search for backslashes            */
              pathPtr = Utilities::locateCharacter(scanPtr, "\\/", endPtr - scanPtr);
              while (pathPtr != NULL) {  /* while more backslashes            */
                pathEnd = pathPtr;     /* save the position                 */
                                       /* search for more backslashes       */
                pathPtr++;             /* step past the last match          */
                pathPtr = Utilities::locateCharacter(pathPtr, "\\/", endPtr - pathPtr);
              }
              if (pathEnd != NULL)     /* have backslashes?                 */
              {
                  return context->NewString(scanPtr, pathEnd - scanPtr + 1);
              }
            }
          }
          else {
            pathPtr = scanPtr;         /* save start position               */
            pathEnd = pathPtr;         /* CHM - defect 85: save end pos.    */
            pathPtr++;                 /* step past first one               */
                                       /* search for backslashes            */
            pathPtr = Utilities::locateCharacter(pathPtr, "\\/", endPtr - pathPtr);
            while (pathPtr) {          /* while more backslashes            */
              pathEnd = pathPtr;       /* save the position                 */
              pathPtr++;               /* step past the last match          */
                                       /* search for more backslashes       */
              pathPtr = Utilities::locateCharacter(pathPtr, "\\/", endPtr - pathPtr);
            }
                                       /* extract the path                  */
            return context->NewString(name, pathEnd - name + 1);
          }
        }
      }
      break;                           /* finished                          */

    case FILESPEC_NAME:                /* extract the file name             */
      if (nameLength > 0) {            /* filename null string?             */
                                       /* find colon or backslash           */
        scanPtr = Utilities::locateCharacter(name, ":\\/", nameLength);
        if (scanPtr != NULL) {
          if (*scanPtr == ':') {       /* found a colon?                    */
            scanPtr++;                 /* step past the colon               */
            pathEnd = scanPtr;         /* save current position             */
            pathPtr = Utilities::locateCharacter(scanPtr, "\\/", endPtr - scanPtr);
            while (pathPtr) {          /* while more backslashes            */
              pathPtr++;               /* step past the last match          */
              pathEnd = pathPtr;       /* save the position                 */
                                       /* search for more backslashes       */
              pathPtr = Utilities::locateCharacter(pathPtr, "\\/", endPtr - pathPtr);
            }
            if (pathEnd < endPtr)      /* stuff to return?                  */
            {
                return context->NewString(pathEnd, endPtr - pathEnd);
            }
          }
          else {
            pathPtr = scanPtr + 1;     /* save start position               */
            pathEnd = pathPtr;         /* step past first one               */
                                       /* search for backslashes            */
            pathPtr = Utilities::locateCharacter(pathPtr, "\\/", endPtr - pathPtr);
            while (pathPtr != NULL) {  /* while more backslashes            */
              pathPtr++;               /* step past the last match          */
              pathEnd = pathPtr;       /* save the position                 */
                                       /* search for more backslashes       */
              pathPtr = Utilities::locateCharacter(pathPtr, "\\/", endPtr - pathPtr);
            }
            if (pathEnd < endPtr)      /* stuff to return?                  */
            {
                return context->NewString(pathEnd, endPtr - pathEnd);
            }
          }
        }
        else
        {
            // entire string is the result
            return context->NewStringFromAsciiz(name);
        }
      }
      break;                           /* finished                          */

    default:                           /* unknown option                    */
      context->InvalidRoutine();       // this is an error
      return NULLOBJECT;
  }
  return context->NullString();        // nothing found, return the empty string
}


/******************************************************************************/
/* Name:       SysExternalFunction                                            */
/*                                                                            */
/* Notes:      Handles searching for and executing an external function.  The */
/*             search order is:                                               */
/*               1) Macro-space pre-order functions                           */
/*               2) Registered external functions                             */
/*               3) REXX programs with same extension (if applicable)         */
/*               4) REXX programs with default extension                      */
/*               5) Macro-space post-order functions                          */
/******************************************************************************/
bool SystemInterpreter::invokeExternalFunction(
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxObject    ** arguments,          /* Argument array                    */
  size_t           argcount,           /* count of arguments                */
  RexxString     * calltype,           /* Type of call                      */
  ProtectedObject &result)
{
  if (activation->callMacroSpaceFunction(target, arguments, argcount, calltype, MS_PREORDER, result))
  {
      return true;
  }
                                       /* no luck try for a registered func */
  if (PackageManager::callNativeRoutine(activity, target, arguments, argcount, result))
  {
      return true;
  }
                                       /* have activation do the call       */
  if (activation->callExternalRexx(target, arguments, argcount, calltype, result))
  {
      return true;
  }
                                       /* function.  If still not found,    */
                                       /* then raise an error               */
  if (activation->callMacroSpaceFunction(target, arguments, argcount, calltype, MS_POSTORDER, result))
  {
      return true;
  }

  return false;
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

void SystemInterpreter::restoreEnvironment(void *CurrentEnv)
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
RexxRoutine4(int, sysMessageBox, CSTRING, text, OPTIONAL_CSTRING, title, OPTIONAL_CSTRING, button, OPTIONAL_CSTRING, icon)
{
  ULONG       style;                   /* window style flags         */
  int         maxCnt;                  /* Max loop count             */
  int         index;                   /* table index                */

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

                                       /* set initial style flags    */
  style = MB_SETFOREGROUND;            /* make this foreground       */

  if (button == NULL)
  {
      style |= MB_OK;                  /* set default button style?  */
  }
  else {                               /* check various button styles*/
                                       /* get the number of styles   */
                                       /* search style table         */
    maxCnt = sizeof(Button_Styles) / sizeof(PSZ);

    for (index = 0; index < maxCnt; index++) {
                                       /* find a match?               */
      if (!stricmp(button, Button_Styles[index])) {
        style += Button_Flags[index];  /* add to the style            */
        break;
      }
    }
    if (index == maxCnt)               /* if not found raise error          */
    {
        context->InvalidRoutine();
        return 0;
    }
  }

  if (icon == NULL)
  {
      style |= MB_OK;    /* set default icon style?           */
  }
  else {                               /* check various button styles*/
    maxCnt = sizeof(Icon_Styles)/sizeof(PSZ);
                                       /* search style table                */
    for (index = 0; index < maxCnt; index++) {
                                       /* find a match?                     */
      if (!stricmp(icon,Icon_Styles[index])) {
        style |= Icon_Flags[index];    /* add to the style                  */
        break;
      }
    }
    if (index == maxCnt)               /* if not found raise error          */
    {
        context->InvalidRoutine();
        return 0;
    }
  }

  return MessageBox(NULL,                //hWndOwner
                  text,                // Text
                  title,               // Title
                  style);              // Styles
}



/**
 * Push a new environment for the SysSetLocal() BIF.
 *
 * @param context The current activation context.
 *
 * @return Returns TRUE if the environment was successfully pushed.
 */
RexxObject *SystemInterpreter::pushEnvironment(RexxActivation *context)
{
    return TheFalseObject;
}

/**
 * Pop an environment for the SysEndLocal() BIF.
 *
 * @param context The current activation context.
 *
 * @return Always returns FALSE.  This is a NOP on Windows.
 */
RexxObject *SystemInterpreter::popEnvironment(RexxActivation *context)
{
    return TheFalseObject;             /* return failure value              */
}

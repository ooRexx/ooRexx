/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/*    SysExternalFunction- Method for searching/invoking an external function */
/*    sysMessageBox - Method to pop up message box                            */
/*                                                                            */
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
#define FILESPEC_LOCATION     'L'
#define FILESPEC_EXTENSION    'E'

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
        context->ArrayAppend(subs, context->WholeNumberToObject(MIN_FREQUENCY));
        context->ArrayAppend(subs, context->WholeNumberToObject(MAX_FREQUENCY));
        context->ArrayAppend(subs, context->WholeNumberToObject(Frequency));
        context->RaiseException(Rexx_Error_Invalid_argument_range, subs);
        return NULL;
    }
                                         /* out of range?              */
    if (Duration > MAX_DURATION || Duration < MIN_DURATION)
    {
        RexxArrayObject subs = context->NewArray(4);
        context->ArrayAppend(subs, context->NewStringFromAsciiz("duration"));
        context->ArrayAppend(subs, context->WholeNumberToObject(MIN_DURATION));
        context->ArrayAppend(subs, context->WholeNumberToObject(MAX_DURATION));
        context->ArrayAppend(subs, context->WholeNumberToObject(Duration));
        context->RaiseException(Rexx_Error_Invalid_argument_range, subs);
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
    const char *endPtr = name + strlen(name);        // point to last character
    const char *pathEnd = strrchr(name, '\\');       // find the last backslash in name
    const char *driveEnd = strchr(name, ':');        // and first colon
    // get the end of the path portion (if any)
    const char *pathStart = driveEnd == NULL ? name : driveEnd + 1;
    // note that pathend is one character past the end of the path.
    // this means the length is easily calculated as pathEnd - pathStart,
    // even in the cases where there is no patch portion
    pathEnd = pathEnd == NULL ? pathStart : pathEnd + 1;
    // this one needs a little adjustment for the case where this is all name
    const char *nameStart = pathEnd == name ? name : pathEnd;

    switch (toupper(*option))              /* process each option               */
    {
        case FILESPEC_PATH:                /* extract the path                  */
            {
                return context->String(pathStart, pathEnd - pathStart);
            }

        case FILESPEC_NAME:                  /* extract the file name               */
            {                                /* everything to right of slash        */
                return context->String(nameStart, endPtr - nameStart);
            }

        case FILESPEC_LOCATION:          /* extract the file name               */
            {                                /* everything to left of slash        */
                return context->String(name, pathEnd - name);
            }

        case FILESPEC_DRIVE:               /* extract the drive                 */
            {
                if (driveEnd != NULL)          /* have a real string?               */
                {
                    return context->String(name, driveEnd + 1 - name);
                }
                else
                {
                    return context->NullString();        // nothing found, return the empty string
                }
            }

        case FILESPEC_EXTENSION:           // extract the file extension
            {
                // find the position of the last dot
                const char *lastDot = strrchr(name, '.');

                if (lastDot >= nameStart)
                {
                    // we don't extract the period
                    lastDot++;
                    return context->String(lastDot, endPtr - lastDot);
                }
                else
                {
                    return context->NullString();        // nothing found, return the empty string
                }

            }
        default:                           /* unknown option                    */
        {
            char optionChar[2];
            optionChar[0] = *option;
            optionChar[1] = '\0';

            RexxArrayObject subs = context->Array(context->String("FILESPEC"), context->WholeNumberToObject(1),
                context->String("DELNP"), context->String(optionChar));
            /* raise an error                    */
            context->RaiseException(Rexx_Error_Incorrect_call_list, subs);
            return NULLOBJECT;
        }
    }
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

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
#include "RexxNativeAPI.h"                  /* Lot's of useful REXX macros    */
#include "SubcommandAPI.h"                  /* Get private REXXAPI API's      */
#include "RexxAPIManager.h"
#include "ProtectedObject.hpp"
#include "StringUtil.hpp"

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

typedef struct _ENVENTRY {                  /* setlocal/endlocal structure    */
  size_t   DriveNumber;                     /* saved drive                    */
  char     Directory[DIRLEN];               /* saved current directory        */
  char    *Environment;                     /* saved environment segment      */
  char     Variables[1];                    /* start of variable values       */
} ENVENTRY;

RexxMethod *SysRestoreProgramBuffer(PRXSTRING, RexxString *);
void ReplaceEnvironment( char * );
REXXOBJECT BuildEnvlist(void);
void RestoreEnvironment( void * );

extern "C" {
APIRET APIENTRY RexxExecuteMacroFunction (const char *, PRXSTRING );
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

RexxMethod2(REXXOBJECT, sysBeep, wholenumber_t, Frequency, wholenumber_t, Duration)
{
                                       /* out of range?              */
  if (Frequency > MAX_FREQUENCY || Frequency < MIN_FREQUENCY || Duration > MAX_DURATION || Duration < MIN_DURATION)
                                       /* raise an error             */
    rexx_exception(Error_Incorrect_call);

  Beep((DWORD)Frequency, (DWORD)Duration);  /* sound beep                 */
  return ooRexxString("");             /* always returns a null      */
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
  {
      if ((strlen(dir) == 2) && (dir[1] == ':'))
         rc = _chdrive(toupper( dir[0] ) - 'A' + 1);
      else
         rc = _chdir(dir);
  }
                                       /* Return the current directory    */
  return (rc != 0) || (_getcwd(buffer,CCHMAXPATH) == NULL) ? ooRexxString("") : ooRexxString(buffer);
}


/********************************************************************************************/
/* sysFilespec                                                                              */
/********************************************************************************************/
RexxMethod2 (REXXOBJECT, sysFilespec, CSTRING, Option, CSTRING, Name)
{
  size_t     NameLength;               /* file name length                  */
  const char *ScanPtr;                 /* scanning pointer                  */
  const char *EndPtr;                  /* end of string                     */
  const char *PathPtr;                 /* path pointer                      */
  const char *PathEnd;                 /* path end pointer                  */
  REXXOBJECT Retval;                   /* return value                      */

                                       /* required arguments missing?       */
  if (Option == NO_CSTRING || strlen(Option) == 0 || Name == NO_CSTRING)
                                       /* raise an error                    */
    rexx_exception(Error_Incorrect_call);

  NameLength = strlen(Name);           /* get filename length               */

  EndPtr = Name + NameLength;          /* point to last character           */
  Retval = OREF_NULLSTRING;            /* set the default return value      */

  switch (toupper(*Option)) {          /* process each option               */

    case FILESPEC_DRIVE:               /* extract the drive                 */
      if (NameLength) {                /* have a real string?               */
                                       /* scan for the character            */
        ScanPtr = (const char *)memchr(Name, ':', NameLength);
        if (ScanPtr)                   /* found one?                        */
                                       /* create result string              */
          Retval = ooRexxStringL(Name, ScanPtr - Name + 1);
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
                Retval = ooRexxStringL(ScanPtr, PathEnd - ScanPtr + 1);
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
            Retval = ooRexxStringL(Name, PathEnd - Name + 1); //retrofit by IH
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
            while (PathPtr) {          /* while more backslashes            */
              PathPtr++;               /* step past the last match          */
              PathEnd = PathPtr;       /* save the position                 */
                                       /* search for more backslashes       */
              PathPtr = mempbrk(PathPtr, "\\/", EndPtr-PathPtr);
            }
            if (PathEnd < EndPtr)      /* stuff to return?                  */
                                       /* extract the name                  */
              Retval = ooRexxStringL(PathEnd, EndPtr - PathEnd);
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
              Retval = ooRexxStringL(PathEnd, EndPtr - PathEnd);
          }
        }
        else
          Retval = ooRexxString(Name); /* entire string is a name           */
      }
      break;                           /* finished                          */

    default:                           /* unknown option                    */
                                       /* raise an error                    */
      rexx_exception(Error_Incorrect_call);
  }
  return Retval;                       /* return extracted part             */
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
bool SysExternalFunction(
  RexxActivation * activation,         /* Current Activation                */
  RexxActivity   * activity,           /* activity in use                   */
  RexxString     * target,             /* Name of external function         */
  RexxString     * parent,             /* Parent program                    */
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
  if (activation->callRegisteredExternalFunction(target, arguments, argcount, calltype, result))
  {
      return true;
  }
                                       /* have activation do the call       */
  if (activation->callExternalRexx(target, parent, arguments, argcount, calltype, result))
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
  if (RexxExecuteMacroFunction(MacroName->getStringData(), &MacroImage) == 0)
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
  char *VariableSet )                 /* new set of variables       */
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
  size_t      MaxCnt;                  /* Max loop count             */
  size_t      Index;                   /* table index                */
  ULONG       rc;                      /* WinMessageBox return code  */

  char * Button_Styles[] =             /* message box button styles  */
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

char * Icon_Styles[] =                 /* message box icon styles    */
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
    rexx_exception(Error_Incorrect_call);


                                       /* set initial style flags    */
  Style = MB_SETFOREGROUND;            /* make this foreground       */

  if (Button == NULL) Style += MB_OK;  /* set default button style?  */
  else {                               /* check various button styles*/
                                       /* get the number of styles   */
                                       /* search style table         */
    MaxCnt = sizeof(Button_Styles)/sizeof(char *);

    for (Index = 0; Index < MaxCnt; Index++) {
                                       /* find a match?               */
      if (!stricmp(Button, Button_Styles[Index])) {
        Style += Button_Flags[Index];  /* add to the style            */
        break;
      }
    }
    if (Index == MaxCnt)               /* if not found raise error          */
      rexx_exception(Error_Incorrect_call);

  }//end else

  if (Icon == NULL) Style += MB_OK;    /* set default icon style?           */
  else {                               /* check various button styles*/
    MaxCnt = sizeof(Icon_Styles)/sizeof(char *);
                                       /* search style table                */
    for (Index = 0; Index < MaxCnt; Index++) {
                                       /* find a match?                     */
      if (!stricmp(Icon,Icon_Styles[Index])) {
        Style += Icon_Flags[Index];    /* add to the style                  */
        break;
      }
    }
    if (Index == MaxCnt)               /* if not found raise error          */
      rexx_exception(Error_Incorrect_call);

  }// end else

  rc = MessageBox(NULL,                //hWndOwner
                  Text,                // Text
                  Title,               // Title
                  Style);              // Styles

  return RexxInteger( rc );}           // convert to return code


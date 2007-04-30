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
/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysValue                                     */
/*                                                                   */
/*   Function:          process the VALUE function selector          */
/*                      function                                     */
/*                                                                   */
/*********************************************************************/

#include <windows.h>
#include <stdlib.h>

#include "RexxCore.h"
#include "StringClass.hpp"

#define  SELECTOR  "ENVIRONMENT"    /* environment selector               */
#define  AXENGINE1 "WSHPROPERTY"    /* scripting engine selector          */
#define  AXENGINE2 "WSHTYPELIB"     /* scripting engine selector          */
#define  AXENGINE3 "WSHENGINE"      /* scripting engine selector          */

 // external scripting support
 extern RexxObject* (__stdcall *WSHPropertyChange)(RexxString*,RexxObject*,int,int*);

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysValue                                     */
/*                                                                   */
/*   Descriptive Name:  set value of environment variable            */
/*                                                                   */
/*********************************************************************/

RexxObject * SysValue(
    RexxString * Name,                 /* variable name                     */
    RexxObject * NewValue,             /* new assigned value                */
    RexxString * Selector )            /* variable selector                 */
{
  /* GetEnvironmentVariable will not alloc memory for OldValue ! */
  CHAR        *OldValue = NULL;        /* old environment value             */
  RexxObject * Retval;                 /* returned old name                 */
  DWORD        dwSize;                 /* size of env. variable             */
  int          SelectorType;           /* Scripting Engine Selector         */
  int          RetCode;                /* Success/Fail                      */

  Selector = Selector->upper();        /* upper case the selector           */
  Name = Name->upper();                /* and the name too                  */

//  if (!Selector->strCompare(SELECTOR)) /* correct selector?                 */
//                                       /* flag this error                   */
//    report_exception1(Error_Incorrect_call_selector, Selector);

  if (Selector->strCompare(SELECTOR)) {/* selector ENVIRONMENT?             */

    /* get the size of the environment variable and allocate buffer         */
    dwSize = GetEnvironmentVariable(Name->stringData, NULL, 0);
    if (dwSize)
    {
      OldValue = (CHAR *) SysAllocateResultMemory(dwSize);
                                         /* scan for the variable           */
      if (OldValue && GetEnvironmentVariable(Name->stringData,OldValue,dwSize) )
      {
                                         /* have a value already?           */
        Retval = (RexxObject*) new_cstring(OldValue);
        SysReleaseResultMemory(OldValue);
      }
      else
        Retval = OREF_NULLSTRING;        /* otherwise, return null            */
    }
    else
      Retval = OREF_NULLSTRING;

    if (NewValue != OREF_NULL)           /* have a new value?                 */
    {
       if (NewValue == (RexxString *) TheNilObject)
            SetEnvironmentVariable((LPCTSTR)Name->stringData, NULL);
       else
            SetEnvironmentVariable((LPCTSTR)Name->stringData,
                             (LPCTSTR)REQUIRED_STRING(NewValue,ARG_TWO)->stringData);
    }
  } else if (WSHPropertyChange != NULL) {     /* in engine environment?       */
    SelectorType = 0;
    RetCode = 0;
    if (Selector->strCompare(AXENGINE1)) SelectorType = 1;
    else if (Selector->strCompare(AXENGINE2)) SelectorType = 2;
    else if (Selector->strCompare(AXENGINE3)) SelectorType = 3;
    if (SelectorType) Retval = WSHPropertyChange(Name,NewValue,SelectorType,&RetCode);  // call into engine
    else report_exception1(Error_Incorrect_call_selector, Selector);

    if (RetCode == 5) report_exception(Error_Incorrect_call_read_from_writeonly);
    else if (RetCode == 4) report_exception(Error_Incorrect_call_write_to_readonly);

  } else
    report_exception1(Error_Incorrect_call_selector, Selector);

  return Retval;                       /* return old value                  */
}


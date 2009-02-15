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
#include <windows.h>
#include <stdlib.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"

#define  SELECTOR  "ENVIRONMENT"    /* environment selector               */

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysValue                                     */
/*                                                                   */
/*   Descriptive Name:  set value of environment variable            */
/*                                                                   */
/*********************************************************************/

bool SystemInterpreter::valueFunction(
    RexxString * Name,                 /* variable name                     */
    RexxObject * NewValue,             /* new assigned value                */
    RexxString * Selector,             /* variable selector                 */
    RexxObject *&result)
{
  /* GetEnvironmentVariable will not alloc memory for OldValue ! */
  char        *OldValue = NULL;        /* old environment value             */
  DWORD        dwSize;                 /* size of env. variable             */

  Selector = Selector->upper();        /* upper case the selector           */
  Name = Name->upper();                /* and the name too                  */

  if (Selector->strCompare(SELECTOR)) {/* selector ENVIRONMENT?             */

    /* get the size of the environment variable and allocate buffer         */
    dwSize = GetEnvironmentVariable(Name->getStringData(), NULL, 0);
    if (dwSize)
    {
      OldValue = (char *) SystemInterpreter::allocateResultMemory(dwSize);
                                         /* scan for the variable           */
      if (OldValue && GetEnvironmentVariable(Name->getStringData(),OldValue,dwSize) )
      {
                                         /* have a value already?           */
        result = (RexxObject*) new_string(OldValue);
        SystemInterpreter::releaseResultMemory(OldValue);
      }
      else
        result = OREF_NULLSTRING;        /* otherwise, return null            */
    }
    else
      result = OREF_NULLSTRING;

    if (NewValue != OREF_NULL)           /* have a new value?                 */
    {
       if (NewValue == (RexxString *) TheNilObject)
            SetEnvironmentVariable((LPCTSTR)Name->getStringData(), NULL);
       else
            SetEnvironmentVariable((LPCTSTR)Name->getStringData(),
                             (LPCTSTR)stringArgument(NewValue,ARG_TWO)->getStringData());
    }
    return true;
  }
  return false;                        // we could not handle this
}


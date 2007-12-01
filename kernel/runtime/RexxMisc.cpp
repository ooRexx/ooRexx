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
/* REXX Kernel                                                  RexxMisc.c    */
/*                                                                            */
/* Primitive Corral  Class  - This is a general purpose class, we never expect*/
/*  to create instances of this class. Its  only purpose is to have a place   */
/*  to hang/define kernel methods to be use by various ORX OBJECT classes,    */
/*  needed by normal OREXX kernel/core stuff.                                 */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "StringClass.hpp"
#include "RexxMisc.hpp"
#include "ActivityManager.hpp"
                                       /* Since self will ALWAYS be OBJECT  */

enum { STOP, START };

void SysRunProgram(PVOID arguments);   /* system dependent program startup  */


RexxDirectory *RexxLocal::local()
/******************************************************************************/
/* Function:  Return the current activation's local environment pointer       */
/******************************************************************************/
{
                                       /* just return the current local     */
                                       /* environment                       */
  return ActivityManager::localEnvironment;
}

RexxObject *RexxLocal::runProgram(
  RexxInteger *arguments)              /* system specific arguments         */
/******************************************************************************/
/* Function:  Bootstrap the process of running a REXX program                 */
/******************************************************************************/
{
  void *   argument_block;             /* system dependent argument block   */

                                       /* get the argument pointer          */
  argument_block = (void *)arguments->getValue();
  SysRunProgram(argument_block);       /* go run the program                */
  return OREF_NULL;                    /* always returns null               */
}

RexxObject *RexxLocal::callProgram(
  RexxObject **arguments,              /* call program arguments            */
  size_t       argCount)               /* number of arguments               */
/******************************************************************************/
/* Function:  Call a program through the RexxCallProgram interface            */
/******************************************************************************/
{
  RexxMethod *routine;                 /* method to call                    */
  RexxString *filename;                /* file to call                      */
  RexxObject *result;                  /* call result                       */

  result = OREF_NULL;
                                       /* go resolve the name               */
  filename = SysResolveProgramName((RexxString *)arguments[0], OREF_NULL);
  if (filename != OREF_NULL) {         /* found something?                  */
                                       /* try to restore saved image        */
    routine = (RexxMethod *)SysRestoreProgram(filename);
    if (routine == OREF_NULL) {        /* unable to restore?                */
                                       /* go translate the image            */
      routine = TheMethodClass->newFile(filename);
                                       /* go save this method               */
      SysSaveProgram(filename, routine);
    }
    if (routine != OREF_NULL)          /* Do we have a method???            */
                                       /* run as a call                     */
      result = ((RexxObject *)ActivityManager::currentActivity)->shriekRun(routine, OREF_COMMAND, OREF_INITIALADDRESS, arguments + 1, argCount - 1);
  }
  else
    reportException(Error_Routine_not_found_name, arguments[0]);
                                       /* run and get the result            */
  return result;
}

RexxObject *RexxLocal::callString(
    RexxObject **arguments,              /* call program arguments            */
    size_t       argCount)               /* number of arguments               */
/******************************************************************************/
/* Function:  Call a program through the RexxCallString  interface            */
/******************************************************************************/
{
  RexxMethod *method;                  /* method to call                    */
  RexxObject *result;                  /* call result                       */

                                       /* go translate the image            */
  method = TheMethodClass->newRexxCode(OREF_COMMAND, arguments[0], IntegerOne);
                                       /* run and get the result            */
  result = ((RexxObject *)ActivityManager::currentActivity)->shriekRun(method, OREF_COMMAND, OREF_INITIALADDRESS, arguments + 1, argCount - 1);
  return result;
}

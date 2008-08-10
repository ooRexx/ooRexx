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
/* REXX Win  Support                                            winmain.c     */
/*                                                                            */
/* Main interface to the REXX interpreter                                     */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

#include "RexxCore.h"                    /* bring in global defines           */
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "MethodClass.hpp"
#include "RexxCode.hpp"
#include "ArrayClass.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "SourceFile.hpp"
#include "DirectoryClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "IntegerClass.hpp"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */
#include "Interpreter.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "InterpreterInstance.hpp"
#include "SupplierClass.hpp"

#include <fcntl.h>
#include <io.h>

#ifdef TIMESLICE                       /* System Yielding function prototype*/
RexxReturnCode REXXENTRY RexxSetYield(process_id_t procid, thread_id_t threadid);
#endif /*timeslice*/


extern "C" {
}


#ifdef TIMESLICE
/******************************************************************************/
/* Name:       RexxSetYield                                                   */
/*                                                                            */
/* Arguments:  procid - Process id of target REXX procedure                   */
/*             threadid - Thread id of target REXX procedure                  */
/*                                                                            */
/* Returned:   rc - RXARI_OK (halt condition set)                             */
/*                  RXARI_NOT_FOUND (couldn't find threadid)                  */
/*                                                                            */
/* Notes:  activity_setyield -> activation_yield ->..->activity_relinquish    */
/*         Causes bits in top activation to be flipped which will cause       */
/*         a system yield via activity_relinquish.                            */
/*                                                                            */
/******************************************************************************/
RexxReturnCode REXXENTRY RexxSetYield(process_id_t procid, thread_id_t threadid)
{
  if (RexxQuery()) {                        /* Are we up?                     */
    if (ActivityManager::yieldActivity(threadid))    /* Set yield condition?           */
      return (RXARI_OK);                    /* Yes, return okay               */
    else
      return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
    }
  else
    return (RXARI_NOT_FOUND);               /* REXX not running, error...     */
}

#endif /* TIMESLICE*/


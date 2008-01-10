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
/* REXX Kernel                                                  okstart.c     */
/*                                                                            */
/* Startup                                                                    */
/*                                                                            */
/******************************************************************************/
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "MethodClass.hpp"
#include "RexxNativeAPI.h"
#include "StackClass.hpp"
#include "Interpreter.hpp"

extern bool  ProcessDoneInit;          /* initialization is done            */
extern bool  ProcessDoneTerm;          /* termination is done               */
extern bool  ProcessFirstThread;       /* first (and primary thread)        */

void kernelShutdown (void)
/******************************************************************************/
/* Shutdown OREXX System for this process                                     */
/******************************************************************************/
{
  SysTermination();                    /* cleanup                           */

  Interpreter::signalTermination();    /* let anyone who cares know we're done*/
  if (!ProcessDoneTerm) {              /* if first time through             */
    ProcessDoneTerm = true;            /* don't allow a "reterm"            */
    ProcessDoneInit = false;           /* no longer initialized.            */
    ProcessFirstThread = true;         /* first thread needs to be created  */
    memoryObject.freePools();          /* release access to memoryPools     */
  }
}


extern bool ProcessSaveImage;


int REXXENTRY RexxTerminate()
/******************************************************************************/
/* Function:  Terminate the REXX interpreter...will only terminate if the     */
/*            call nesting level has reached zero.                            */
/******************************************************************************/
{
    // terminate one instance and shutdown, if necessary
    ActivityManager::terminateInterpreter();
    return 0;
}

bool REXXENTRY RexxInitialize (void)
/******************************************************************************/
/* Function:  Perform main kernel initializations                             */
/******************************************************************************/
{
  bool result;                         /* initialization result             */

  setbuf(stdout,NULL);                 /* No buffering                      */
  setbuf(stderr,NULL);

  SysThreadInit();                     /* do thread initialization          */
  result = ProcessFirstThread;         /* check on the first thread         */

  // perform activity manager startup for another instance
  ActivityManager::createInterpreter();
  if (ProcessFirstThread) {            /* if the first time                 */
    ProcessFirstThread = false;        /* this is the first thread          */
    Interpreter::createLocks();
    ActivityManager::createKernelLock();
    ProcessDoneInit = false;           /* allow for restart :               */
    ProcessDoneTerm = false;           /* allow for restart :               */
    memoryObject.accessPools();        /* Gain access to memory Pools       */
    SysInitialize();                   /* perform other system init work    */

    if (ProcessSaveImage)              /* need to create the image?         */
    {
        RexxMemory::createImage();     /* go create the image               */
    }
    else {
      RexxMemory::restore();           // go restore the state of the memory object
      ActivityManager::startup();      // go create the local enviroment.
    }
    ProcessDoneInit = true;            /* we're now initialized             */
  }                                    /* end of serialized block of code   */
  return result;                       /* all done                          */
}

bool REXXENTRY RexxQuery (void)
/******************************************************************************/
/* Function:  Determine if the REXX interpreter is initialized and active     */
/******************************************************************************/
{
  return ProcessDoneInit;              /* just check the doneinit flag      */
}

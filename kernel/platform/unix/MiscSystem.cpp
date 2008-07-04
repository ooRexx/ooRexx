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
/* REXX AIX Support                                             aixmisc.c     */
/*                                                                            */
/* Miscellaneous AIX specific routines.                                       */
/*                                                                            */
/******************************************************************************/

/*********************************************************************/
/*                                                                   */
/*   Function:  Miscellaneous system specific routines               */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "RexxCore.h"
#include "StringClass.hpp"
// #include "RexxNativeAPI.h"                      /*  THUTHUREXX native interface*/
#include "DirectoryClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "ThreadSupport.hpp"
#include "ActivityManager.hpp"
#include "PointerClass.hpp"
#include "SystemInterpreter.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#if defined( HAVE_SIGNAL_H )
# include <signal.h>
#endif

#if defined( HAVE_SYS_SIGNAL_H )
# include <sys/signal.h>
#endif

#if defined( HAVE_SYS_LDR_H )
# include <sys/ldr.h>
#endif

#if defined( HAVE_FILEHDR_H )
# include <filehdr.h>
#endif

#include <dlfcn.h>

#if defined( HAVE_SYS_UTSNAME_H )
# include <sys/utsname.h>               /* get the uname() function   */
#endif

void RxExitClear(int);
void RxExitClearNormal();


unsigned int iClauseCounter=0;         // count of clauses
#define LOADED_OBJECTS 100

RexxString *SystemInterpreter::getInternalSystemName()
{
    return getSystemName();     // this is the same
}

RexxString *SystemInterpreter::getSystemName()
/******************************************************************************/
/* Function: Get System Name                                                  */
/******************************************************************************/
{
#if defined(AIX)
    return new_string("AIX")
#elif defined(OPSYS_SUN)
    return new_string("SUNOS")
#else
    return new_string("LINUX")
#endif
}


RexxString *SystemInterpreter::getSystemVersion()
/******************************************************************************/
/* Function:   Return the system specific version identifier that is stored   */
/*             in the image.                                                  */
/******************************************************************************/
{
    struct utsname info;                 /* info structur              */

    uname(&info);                        /* get the info               */

    return new_string(info.release);    /* return as a string                */
}


void *SysLoadProcedure(
  RexxPointer * LibraryHandle,         /* library load handle               */
  RexxString  * Procedure)             /* required procedure name           */
/******************************************************************************/
/* Function:  Resolve a named procedure in a library                          */
/******************************************************************************/
{
   void *load_address;
   load_address = dlsym((void *)LibraryHandle->pointer(), Procedure->getStringData());
   if (load_address == NULL)
   {
      reportException(Error_External_name_not_found_method, Procedure);
   }
   return load_address;
}

RexxPointer * SysLoadLibrary(
     RexxString * Library)             /* required library name             */
/******************************************************************************/
/* Function:  Load a named library, returning the library handle              */
/******************************************************************************/
{
  RexxString *result;
  RexxString *tempresult;
  void *plib;

  result = (RexxString*) new_string("lib");
  result = result->concatWithCstring(Library->getStringData());
  result = result->concatWithCstring(ORX_SHARED_LIBRARY_EXT);
  tempresult = (RexxString *)result->copy();

  if (!(plib = dlopen(result->getStringData(), RTLD_LAZY )))
  {
     if (!(plib = dlopen(result->getStringData(), RTLD_LAZY )))
     {
        fprintf(stderr, " *** Error dlopen: %s\n", dlerror());
        reportException(Error_Execution_library, tempresult);
     }
  }
  return new_pointer(plib);
}

#define MAX_ADDRESS_NAME_LENGTH  250   /* maximum command environment name  */

void SysValidateAddressName(
  RexxString *Name )                   /* name to validate                  */
/******************************************************************************/
/* Function:  Validate an external address name                               */
/******************************************************************************/
{
                                       /* name too long?                    */
  if (Name->getLength() > MAX_ADDRESS_NAME_LENGTH)
                                       /* go report an error                */
    reportException(Error_Environment_name_name, MAX_ADDRESS_NAME_LENGTH, Name);
}

void SysSetupProgram(
  RexxActivation *activation)          /* current running program           */
/******************************************************************************/
/* Function:  Do system specific program setup                                */
/******************************************************************************/
{
#ifdef RXTRACE_SUPPORT
  char     *RxTraceBuf = NULL;

                                       /* scan current environment,         */
/*if (GetEnvironmentVariable("RXTRACE", RxTraceBuf, 8)) {                   */
  RxTraceBuf = getenv("RXTRACE");
  if (RxTraceBuf)
  {
    if (!stricmp(RxTraceBuf, "ON"))    /* request to turn on?               */
                                       /* turn on tracing                   */
      activation->setTrace(TRACE_RESULTS, DEBUG_ON);
  }
#endif
}

RexxString * SystemInerpreter::getSourceString(
  RexxString * callType,               /* type of call token                */
  RexxString * programName )           /* program name token                */
/******************************************************************************/
/* Function:  Produce a system specific source string                         */
/******************************************************************************/
{
    RexxString * source_string;          /* final source string               */
    char       * outPtr;
    source_string = raw_string(1+sizeof(ORX_SYS_STR)+callType->getLength()+programName->getLength());
    outPtr = source_string->getWritableData();  /* point to result Data.             */

    strcpy(outPtr, ORX_SYS_STR);          /* copy the system name              */
    outPtr +=sizeof(ORX_SYS_STR) - 1;     /* step past the name                */
    *outPtr++ = ' ';                     /* put a blank between               */
                                         /* copy the call type                */
    memcpy(outPtr, callType->getStringData(), callType->getLength());
    outPtr += callType->getLength();     /* step over the call type           */
    *outPtr++ = ' ';                     /* put a blank between               */
                                         /* copy the system name              */
    memcpy(outPtr, programName->getStringData(), programName->getLength());
    return source_string;                /* return the source string          */
}

// these routines are NOPs
void SysRegisterExceptions(SYSEXCEPTIONBLOCK *exception_info) { ; }
void SysDeregisterExceptions(SYSEXCEPTIONBLOCK *exception_info) { ; }


void SysRegisterSignals(
  SYSEXCEPTIONBLOCK *exception_info)   /* system specific exception info    */
/******************************************************************************/
/* Function:   Establish exception handlers                                   */
/******************************************************************************/
{
}

void SysDeregisterSignals(
  SYSEXCEPTIONBLOCK *exception_info)   /* system specific exception info    */
/******************************************************************************/
/* Function:   Clear out registered exception handlers                        */
/******************************************************************************/
{
}

void SysClauseBoundary(RexxActivation *stub)
{
}

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
/* REXX Kernel                                                  RexxNativeMethod.c     */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxNativeMethod.hpp"
#include <ctype.h>

extern "C" internalMethodEntry internalMethodTable[];

     RexxNativeCode::RexxNativeCode(
     RexxString *procedure,            /* procedure to load                 */
     RexxString *library,              /* library to load from              */
     PFN         entry,                /* Entry point address for method    */
     LONG        index )               /* internal method index             */
/****************************************************************************/
/* Function:  Initialize a REXX native code object                          */
/****************************************************************************/
{
  this->entry = entry;                 /* no resolved entry point yet       */
  this->index = index;                 /* save the index                    */
                                       /* save the library name             */
  OrefSet(this, this->library, library);
                                       /* save the procedure name           */
  OrefSet(this, this->procedure, procedure);
}

void RexxNativeCode::reinit(           /* reinitialize the nmethod entry    */
     RexxInteger *Handle )             /* library handle information        */
/****************************************************************************/
/* Function:  Reinitialize a REXX native method                             */
/****************************************************************************/
{
  if (this->procedure != OREF_NULL)    /* in another library?               */
                                       /* and resolve the function address  */
    this->entry = (PFN)SysLoadProcedure(Handle, this->procedure);
}

void RexxNativeCode::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->library);
  memory_mark(this->procedure);
  cleanUpMemoryMark
}

void RexxNativeCode::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->library);
  memory_mark_general(this->procedure);
  if (memoryObject.restoringImage()) { /* restoring the image?              */
    if (this->procedure == OREF_NULL)  /* this an internal method?          */
                                       /* reresolve the internal method     */
      this->entry = internalMethodTable[this->index].entryPoint;
  }
  cleanUpMemoryMarkGeneral
}

void RexxNativeCode::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNativeCode)

   flatten_reference(newThis->library, envelope);
   flatten_reference(newThis->procedure, envelope);
                                       /* Set entry to NUll for 2 reasons   */
                                       /* 1st force branch to 0 is not      */
                                       /*restored, 2 used to indicated if   */
                                       /* the method has bee unflattened    */
   newThis->entry = NULL;
  cleanUpFlatten
}

RexxObject * RexxNativeCode::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
  RexxNativeCode *newCode;

                                       /* Does the entry have a value?      */
                                       /* if not then we haven't unflattened*/
   if (this->entry == NULL) {
                                       /* Resolve new address procedure     */
     newCode = new_nmethod(this->procedure, this->library);
     this->entry = newCode->entry;     /* Store new entry in existing obj   */
   }
   return (RexxObject *)this;          /* return ourself.                   */
}



void * RexxNativeCode::operator new(
     size_t      size)                 /* object size                       */
/****************************************************************************/
/* Function:  Create a new Native method                                    */
/****************************************************************************/
{
  RexxObject *newMethod;               /* new object                        */

  newMethod = new_object(size);        /* Get new object                    */
                                       /* Give new object its behaviour     */
  BehaviourSet(newMethod, TheNativeCodeBehaviour);
  return newMethod;                    /* and return the new method         */
}

RexxNativeCodeClass::RexxNativeCodeClass()
/****************************************************************************/
/* Function:  Set up initial nmethod class state data                       */
/****************************************************************************/
{
                                       /* just create the library table     */
  OrefSet(this, this->libraries, new_directory());
}

void RexxNativeCodeClass::restore()
/******************************************************************************/
/* Function:  Do nmethod class restore image processing                       */
/******************************************************************************/
{
  long        i;                       /* table index                       */
                                       /* go reload all the libraries       */
  for (i = this->libraries->first(); this->libraries->available(i); i = this->libraries->next(i)) {
                                       /* first reload the library          */
    this->load((RexxString *)this->libraries->index(i));
                                       /* then re-resolve all addresses     */
    this->reload((RexxDirectory *)this->libraries->value(i));
  }
}

void RexxNativeCodeClass::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  this->RexxClass::live();
  setUpMemoryMark
  memory_mark(this->libraries);
  cleanUpMemoryMark
}

void RexxNativeCodeClass::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  this->RexxClass::liveGeneral();
  setUpMemoryMarkGeneral
  memory_mark_general(this->libraries);
  cleanUpMemoryMarkGeneral
}

RexxNativeCode *RexxNativeCodeClass::newClass(
     RexxString *procedure,            /* procedure to load                 */
     RexxString *library )             /* library to load from              */
/****************************************************************************/
/* Function:  Create a new native method                                    */
/****************************************************************************/
{
  RexxNativeCode *newMethod;           /* new code to return                */
  RexxDirectory  *libinfo;             /* Library info table for library    */
  PFN             entry;               /* routine entry point address       */

  libinfo = this->load(library);       /* Load the library.                 */
  if (libinfo != OREF_NULL) {          /* library loaded ok?                */
                                       /* See if we already know about this */
                                       /* method.                           */
    newMethod = (RexxNativeCode *)libinfo->entry(procedure);
    if (newMethod == OREF_NULL) {      /* not there yet?                    */
                                       /* resolve the function address      */
      entry = (PFN)SysLoadProcedure((RexxInteger *)libinfo->at(OREF_NULLSTRING), procedure);
                                       /* unknown, create a new one.        */
                                       /* Get new object                    */
      newMethod = new RexxNativeCode (procedure, library, entry, 0);
                                       /* add this to the libraries table   */
      libinfo->setEntry(procedure, (RexxObject *)newMethod);
    }
  }
  else {
    newMethod = new RexxNativeCode (procedure, library, NULL, 0);
  }
  return (RexxNativeCode *)newMethod;  /* return the new method             */
}

RexxNativeCode *RexxNativeCodeClass::newInternal(
     LONG index )                     /* index of the internal REXX method */
/******************************************************************************/
/* Function:  Create a new native method from the internal REXX table         */
/******************************************************************************/
{
                                       /* create a native method object     */
  return (RexxNativeCode *)new RexxNativeCode (OREF_NULL, OREF_NULL, internalMethodTable[index].entryPoint, index);
}

void RexxNativeCodeClass::reload(
     RexxDirectory *LibraryInfo )      /* library information table         */
/****************************************************************************/
/* Function:  Reload a library of routines                                  */
/****************************************************************************/
{
  RexxString     *Procedure;           /* table procedure name              */
  long            i;                   /* table traversal index             */
  RexxNativeCode *Method;              /* resolved method object            */
  RexxInteger    *Handle;              /* library handle                    */

                                       /* get the library handle            */
  Handle = (RexxInteger *)LibraryInfo->at(OREF_NULLSTRING);
                                       /* now traverse the entire table     */
  for (i = LibraryInfo->first(); LibraryInfo->available(i); i = LibraryInfo->next(i)) {
                                       /* get the next procedure            */
    Procedure = (RexxString *)LibraryInfo->index(i);
    if (Procedure != OREF_NULLSTRING) {/* not the handle entry?             */
                                       /* get the matching method           */
      Method = (RexxNativeCode *)LibraryInfo->value(i);
      Method->reinit(Handle);          /* re-resolve the function address   */
    }
  }
}

RexxDirectory *RexxNativeCodeClass::load(
     RexxString *Library)
/****************************************************************************/
/* Function:  Resolve and load a native method library name                 */
/****************************************************************************/
{
  RexxDirectory *LibraryInfo;          /* table of library information      */
  RexxString    *LibraryName;          /* uppercase library name            */

  if (Library == OREF_NULL)            /* no library given?                 */
    return OREF_NULL;                  /* don't load this                   */
  LibraryName = Library->upper();      /* get name in upper case            */
                                       /* and get the library info          */
  LibraryInfo = (RexxDirectory *)this->libraries->get(LibraryName);
  if (LibraryInfo == OREF_NULL) {      /* first request for this one?       */
    LibraryInfo = new_directory();     /* create a new directory            */
                                       /* add to the library table          */
    this->libraries->setEntry(LibraryName, (RexxObject *)LibraryInfo);
  }
                                       /* now get the "handle" information  */
  LibraryInfo->setEntry(OREF_NULLSTRING, (RexxObject *)SysLoadLibrary(Library));
  return LibraryInfo;                  /* return the library information    */
}

void nmethod_create (void)
/******************************************************************************/
/* Function:  Create the nmethod class during save image processing           */
/******************************************************************************/
{
                                       /* create the class object           */
  create_udsubClass(NativeCode, RexxNativeCodeClass);
  TheNativeCodeClass->init();          /* and do class-specific init        */
                                       /* hook up the class and behaviour   */
  TheNativeCodeBehaviour->setClass(TheNativeCodeClass);
}

void nmethod_restore (void)
/******************************************************************************/
/* Function:  Restore the nmethod class during start up                       */
/******************************************************************************/
{
  TheNativeCodeClass->restore();       /* go reinitialize                   */
}

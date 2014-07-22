/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX Kernel                                                 RexxCode.hpp   */
/*                                                                            */
/* Primitive REXX Code Class Definitions                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxCode
#define Included_RexxCode

#include "SourceFile.hpp"
#include "BaseCode.hpp"
#include "RexxLocalVariables.hpp"

/**
 * The fundamental unit of Rexx code execution.  This
 * anchors the pieces that allow Rexx code to run.
 */
class RexxCode : public BaseCode
{
  public:
   void *operator new(size_t);
   inline void *operator new(size_t size, void *objectPtr) { return objectPtr; };
   inline void  operator delete(void *) { ; }
   inline void  operator delete(void *, void *) {;}

   // an extra added to the stack frame needed because the count
   // is generally off by one or two.
   const size_t MINIMUM_STACK_FRAME = 10;

   RexxCode(RexxSource *s, RexxInstruction *i, DirectoryClass *l = OREF_NULL, size_t f = 0, size_t v = RexxLocalVariables::FIRST_VARIABLE_INDEX);
   inline RexxCode(RESTORETYPE restoreType) { ; };

   virtual void live(size_t);
   virtual void liveGeneral(MarkReason reason);
   virtual void flatten(Envelope *);

   ArrayClass      * getSource();
   RexxObject     * setSecurityManager(RexxObject *);
   RexxString     * getProgramName();

   inline RexxSource *getSourceObject() { return source; }
   inline RexxInstruction *getFirstInstruction() { return start; }
   inline DirectoryClass   *getLabels() { return labels; }
   inline size_t getMaxStackSize() { return maxStack; }
   inline size_t getLocalVariableSize() { return vdictSize; }
   inline DirectoryClass *getLocalRoutines() { return source->getLocalRoutines(); }
   inline DirectoryClass *getPublicRoutines() { return source->getPublicRoutines(); }
   inline void setLocalRoutines(DirectoryClass *r) { source->setLocalRoutines(r); }
   inline void setPublicRoutines(DirectoryClass *r) { source->setPublicRoutines(r); }
   inline bool isTraceable() { return source->isTraceable(); }
   inline RexxString *extract(SourceLocation &l) { return source->extract(l); }
   inline SecurityManager *getSecurityManager() { return source->getSecurityManager(); }
   inline void        install(RexxActivation *activation) { source->install(activation); }
   inline DirectoryClass *getMethods() { return source->getMethods(); };
   inline DirectoryClass *getRoutines() { return source->getRoutines(); };
   inline RoutineClass *findRoutine(RexxString *n) { return source->findRoutine(n); }
   inline RexxString *resolveProgramName(Activity *activity, RexxString *name) { return source->resolveProgramName(activity, name); }
   inline void        mergeRequired(RexxSource *s) { source->mergeRequired(s); }
          RexxCode *interpret(RexxString *source, size_t lineNumber);

   // overrides for BaseCode classes
   virtual void run(Activity *, MethodClass *, RexxObject *, RexxString *, RexxObject **,  size_t, ProtectedObject &);
   virtual void call(Activity *, RoutineClass *, RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, int, ProtectedObject &);
   virtual void call(Activity *, RoutineClass *, RexxString *,  RexxObject **, size_t, ProtectedObject &);

protected:

    RexxSource      * source;            // the source this code belongs to.
    RexxInstruction * start;             // root of instruction tree
    SourceLocation    location;          // the full location of the code.
    DirectoryClass   * labels;            // list of labels in this code block
    size_t            maxStack;          // maximum stack depth
    size_t            vdictSize;         // size of variable dictionary
};
#endif

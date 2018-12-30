/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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

#include "BaseCode.hpp"
#include "RexxLocalVariables.hpp"
#include "PackageClass.hpp"

class StringTable;
class RexxInstruction;

/**
 * The fundamental unit of Rexx code execution.  This
 * anchors the pieces that allow Rexx code to run.
 */
class RexxCode : public BaseCode
{
  public:
   void *operator new(size_t);
   inline void  operator delete(void *) { ; }

   // an extra added to the stack frame needed because the count
   // is generally off by one or two.
   static const size_t MINIMUM_STACK_FRAME = 10;

   RexxCode(PackageClass *s, SourceLocation &loc, RexxInstruction *i, StringTable *l = OREF_NULL, size_t f = 0, size_t v = RexxLocalVariables::FIRST_VARIABLE_INDEX);
   inline RexxCode(RESTORETYPE restoreType) { ; };

   void live(size_t) override;
   void liveGeneral(MarkReason reason) override;
   void flatten(Envelope *) override;

   // overrides for BaseCode methods
   void run(Activity *, MethodClass *, RexxObject *, RexxString *, RexxObject **,  size_t, ProtectedObject &) override;
   void call(Activity *, RoutineClass *, RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, ActivationContext, ProtectedObject &) override;
   void call(Activity *, RoutineClass *, RexxString *,  RexxObject **, size_t, ProtectedObject &) override;
   ArrayClass *getSource() override;
   RexxObject *setSecurityManager(RexxObject *) override;

   RexxString      *getProgramName();

   inline RexxInstruction *getFirstInstruction() { return start; }
   inline StringTable *getLabels() { return labels; }
   inline size_t getMaxStackSize() { return maxStack; }
   inline size_t getLocalVariableSize() { return vdictSize; }
   inline StringTable *getLocalRoutines() { return package->getLocalRoutines(); }
   inline StringTable *getPublicRoutines() { return package->getPublicRoutines(); }
   inline bool isTraceable() { return package->isTraceable(); }
   inline RexxString *extract(SourceLocation &l) { return package->extract(l); }
   inline SecurityManager *getSecurityManager() { return package->getSecurityManager(); }
   inline void        install(RexxActivation *activation) { package->install(activation); }
   inline StringTable *getResources() { return package->getResources(); };
   inline StringTable *getMethods() { return package->getMethods(); };
   inline StringTable *getRoutines() { return package->getRoutines(); };
   inline RoutineClass *findRoutine(RexxString *n) { return package->findRoutine(n); }
   inline RexxString *resolveProgramName(Activity *activity, RexxString *name) { return package->resolveProgramName(activity, name); }
   inline void        mergeRequired(PackageClass *s) { package->mergeRequired(s); }
          RexxCode *interpret(RexxString *source, size_t lineNumber);
          void addInstruction(RexxInstruction *i, size_t m, size_t v);


protected:

    RexxInstruction *start;             // root of instruction tree
    SourceLocation   location;          // the full location of the code.
    StringTable     *labels;            // list of labels in this code block
    size_t           maxStack;          // maximum stack depth
    size_t           vdictSize;         // size of variable dictionary
};
#endif

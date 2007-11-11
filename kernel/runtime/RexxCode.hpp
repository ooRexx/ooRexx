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
/* REXX Kernel                                                  RexxCode.hpp   */
/*                                                                            */
/* Primitive REXX Code Class Definitions                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxCode
#define Included_RexxCode

#include "SourceFile.hpp"

                                       /* various types of call or function */
                                       /* calls                             */
#define INTERNAL_ROUTINE 1
#define BUILTIN_ROUTINE  2
#define EXTERNAL_ROUTINE 3
#define DYNAMIC_ROUTINE  4

class RexxCode : public RexxInternalObject {
  public:
   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   RexxCode(RexxSource *, RexxInstruction *, RexxDirectory *, size_t, size_t);
   inline RexxCode(RESTORETYPE restoreType) { ; };
   void live();
   void liveGeneral();
   void flatten(RexxEnvelope *);
   RexxArray      * sourceRexx();
   RexxString     * getProgramName();
   inline RexxSource *getSource() { return source; }
   inline RexxInstruction *getFirstInstruction() { return start; }
   inline RexxDirectory   *getLabels() { return labels; }
   inline size_t getMaxStackSize() { return maxStack; }
   inline size_t getLocalVariableSize() { return vdictSize; }
   inline RexxDirectory *getLocalRoutines() { return source->getLocalRoutines(); }
   inline RexxDirectory *getPublicRoutines() { return source->getPublicRoutines(); }
   inline void setLocalRoutines(RexxDirectory *r) { source->setLocalRoutines(r); }
   inline void setPublicRoutines(RexxDirectory *r) { source->setPublicRoutines(r); }
   inline bool isTraceable() { return source->isTraceable(); }
   inline bool isInterpret() { return source->isInterpret(); }
   inline RexxString *extract(SourceLocation &l) { return source->extract(l); }
   inline RexxObject *getSecurityManager() { return source->getSecurityManager(); }
   inline void        install(RexxActivation *activation) { source->install(activation); }
   inline RexxMethod *interpret(RexxString *s, size_t n) { return source->interpret(s, labels, n); }
   inline RexxDirectory *getMethods() { return source->getMethods(); };
   inline RexxMethod *resolveRoutine(RexxString *n) { return source->resolveRoutine(n); }
   inline void        mergeRequired(RexxSource *s) { return source->mergeRequired(s); }

protected:

  RexxSource      * source;            // the source this code belongs to.
  RexxInstruction * start;             /* root of parse tree                */
  RexxDirectory   * labels;            /* root of label list                */
  size_t            maxStack;          /* maximum stack depth               */
  size_t            vdictSize;         /* size of variable dictionary       */

};
#endif

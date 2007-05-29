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
/* REXX Kernel                                                  RexxBehaviour.hpp   */
/*                                                                            */
/* Primitive Behaviour Class Definitions                                      */
/*                                                                            */
/*******************************************************************************/
#ifndef Included_RexxBehaviour
#define Included_RexxBehaviour

void behaviour_setup (void);
                                       /* This flags is TRUE once a         */
                                       /* Behaviour is copied               */
#define NON_PRIMITIVE_BEHAVIOUR 0x0001
#define ENHANCED_OBJECT         0x0002 /* this is an enhanced object        */
#define BEHAVIOUR_NOT_RESOLVED  0x0010
#define RESET_BEHAVIOUR_RESOLVED  0xFFEF

#define resolveNonPrimitiveBehaviour(o)                                            \
       if (o->isBehaviourNotResolved() ){                                          \
                                       /* Nope, turn off not resolved       */     \
         o->setBehaviourResolved();                                                \
                                       /* Resolve address of                */     \
                                       /*  all kernel methods for behav     */     \
                                       /*  all oper   methods for behav     */     \
         o->operatorMethods = pbehav[o->typenum()].operatorMethods;                \
       }

 class RexxBehaviour : public RexxInternalObject {
  public:
  void *operator new(size_t, short);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  RexxBehaviour(HEADINFO, short, PCPPM *);
  inline RexxBehaviour() {;};
  inline RexxBehaviour(RESTORETYPE restoreType) { ; };
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);
  RexxObject *copy();
  RexxObject *define(RexxString *, RexxMethod *);
  void        addMethod(RexxString *, RexxMethod *);
  void        removeMethod(RexxString *);
  RexxMethod *methodObject(RexxString *);
  RexxMethod *methodLookup( RexxString *);
  RexxMethod *getMethod( RexxString *);
  RexxObject *deleteMethod(RexxString *);
  void restore(short, RexxBehaviour *);
  RexxClass  *restoreClass();
  RexxObject *superScope( RexxObject *);
  RexxMethod *superMethod(RexxString *, RexxObject *);
  void        setMethodDictionaryScope( RexxObject *);
  RexxObject *setScopes( RexxObjectTable *);
  RexxObject *addScope( RexxObject *);
  RexxObject *mergeScope( RexxObject *);
  BOOL        checkScope( RexxObject *);
  void        subclass(RexxBehaviour *);
  RexxSupplier *getMethods(RexxObject *scope);

  void merge( RexxBehaviour *);
  void methodDictionaryMerge( RexxTable *);


   inline RexxObjectTable  *getScopes()       { return this->scopes; };
   inline RexxTable  *getMethodDictionary()   { return this->methodDictionary; };
   inline void        setMethodDictionary(RexxTable * m) { OrefSet(this, this->methodDictionary, m); };
   inline void        setInstanceMethodDictionary(RexxTable * m) { OrefSet(this, this->instanceMethodDictionary, m); };
   inline RexxTable  *getInstanceMethodDictionary()   { return this->instanceMethodDictionary; };
   inline RexxClass  *getCreateClass()        { return this->createClass;};
   inline void        setClass(RexxClass *c)  { OrefSet(this, this->createClass,  c); };
   inline SHORT typenum()                 { return this->behaviourInfo.typeNum; };
   inline SHORT flags()                   { return this->behaviourInfo.behaviourFlags; };
   inline void  setFlags(SHORT v)         { this->behaviourInfo.behaviourFlags = v; };
   inline void  setTypenum(SHORT typenumber) { this->behaviourInfo.typeNum = typenumber; };

   inline BOOL  isPrimitiveBehaviour()    {  return ! this->isNonPrimitiveBehaviour(); };
   inline BOOL  isNonPrimitiveBehaviour() {  return this->behaviourInfo.behaviourFlags & NON_PRIMITIVE_BEHAVIOUR; };
   inline BOOL  isBehaviourResolved()     {  return ! this->isBehaviourNotResolved(); };
   inline BOOL  isBehaviourNotResolved()  {  return this->behaviourInfo.behaviourFlags & BEHAVIOUR_NOT_RESOLVED; };
   inline BOOL  isEnhanced()              {  return this->behaviourInfo.behaviourFlags & ENHANCED_OBJECT; };
   inline void  setBehaviourResolved()    {  this->behaviourInfo.behaviourFlags &= RESET_BEHAVIOUR_RESOLVED; };
   inline void  setBehaviourNotResolved() {  this->behaviourInfo.behaviourFlags |= BEHAVIOUR_NOT_RESOLVED; };
   inline void  setEnhanced()             {  this->behaviourInfo.behaviourFlags |= ENHANCED_OBJECT; };
   inline void  setNonPrimitiveBehaviour(){  this->behaviourInfo.behaviourFlags |= NON_PRIMITIVE_BEHAVIOUR; };

   RexxObjectTable  *scopes;           /* scopes table                      */
   RexxTable  *methodDictionary;       /* method dictionary                 */
   PCPPM      *operatorMethods;        /* operator look-a-side table        */
   RexxClass  *createClass;            /* class that created this object    */
                                       /* methods added via SETMETHOD       */
   RexxTable  *instanceMethodDictionary;
 };

#endif

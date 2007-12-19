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
/* REXX Kernel                                      RexxNativeActivation.hpp  */
/*                                                                            */
/* Primitive Native Activation Class Definitions                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxNativeActivation
#define Included_RexxNativeActivation

#include <setjmp.h>
#include "RexxActivity.hpp"

class RexxNativeActivation : public RexxActivationBase {
 public:
         void *operator new(size_t, RexxObject *, RexxMethod *, RexxActivity *, RexxString *, RexxActivationBase *);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  inline void  operator delete(void *, RexxObject *, RexxMethod *, RexxActivity *, RexxString *, RexxActivationBase *) { }
  inline void  operator delete(void *, void *) { ; }
  inline void  operator delete(void *) { ; }

  inline RexxNativeActivation(RESTORETYPE restoreType) { ; };
  inline RexxNativeActivation() {;};
  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope *);
  RexxObject *run(size_t, RexxObject **);
  RexxObject *saveObject(RexxObject *);
  RexxVariableDictionary *methodVariables();
  bool   isInteger(RexxObject *);
  const char *cstring(RexxObject *);
  double getDoubleValue(RexxObject *);
  bool   isDouble(RexxObject *);
  void  *cself();
  void  *buffer();
  void  *pointer(RexxObject *);
  RexxObject *dispatch();
  RexxObject *getReceiver() {return  this->receiver;}
  void   traceBack(RexxList *);
  size_t digits();
  size_t fuzz();
  bool   form();
  void   setDigits(size_t);
  void   setFuzz(size_t);
  void   setForm(bool);
  void   guardOff();
  void   guardOn();
  void   enableVariablepool();
  void   disableVariablepool();
  bool   trap (RexxString *, RexxDirectory *);
  void   setObjNotify(RexxMessage *);
  void   resetNext();
  bool   fetchNext(RexxString **name, RexxObject **value);
  void   raiseCondition(RexxString *condition, RexxString *description, RexxObject *additional, RexxObject *result);

  inline void   termination() { this->guardOff();}
  inline RexxActivation *sender() {return (RexxActivation *)this->activity->sender((RexxActivationBase *)this);}
  inline RexxActivation *getCurrentActivation() { return activity->getCurrentActivation(); }
  inline char        getVpavailable()   {return this->vpavailable;}
  inline RexxMethod *getMethod()        {return this->method;}
  inline RexxString *getMessageName()   {return this->msgname;}
  inline int         nextVariable()     {return this->nextvariable;}
  inline RexxStem   *nextStem()         {return this->nextstem;}
  inline RexxVariableDictionary *nextCurrent()     {return this->nextcurrent;}
  inline RexxCompoundElement *compoundElement() {return this->compoundelement; }
  inline void        setNextVariable(size_t value)           {this->nextvariable = value;}
  inline void        setNextCurrent(RexxVariableDictionary *vdict)     {this->nextcurrent = vdict;}
  inline void        setNextStem(RexxStem *stemVar)     {this->nextstem = stemVar;}
  inline void        setCompoundElement(RexxCompoundElement *element)     {this->compoundelement = element;}
  inline RexxActivity *getActivity() { return activity; }

protected:

  RexxMethod     *method;              /* Method to run                     */
  RexxString     *msgname;             /* name of the message running       */
  RexxObject     *receiver;            // the object receiving the message
  RexxActivity   *activity;            /* current activity                  */
  RexxActivation *activation;          /* parent activation                 */
  RexxObject    **arglist;             /* copy of the argument list         */
  RexxArray      *argArray;            /* optionally create argument array  */
  RexxObjectTable *savelist;           /* list of saved objects             */
  RexxObject     *firstSavedObject;    /* first saved object instance       */
  RexxMessage    *objnotify;           /* an object to notify if excep occur*/
  RexxObject     *result;              /* result from RexxRaise call        */
                                       /* running object variable pool      */
  RexxVariableDictionary *objectVariables;
  int             nextvariable;        /* next variable to retrieve         */
  RexxVariableDictionary *nextcurrent; /* current processed vdict           */
  RexxCompoundElement *compoundelement;/* current compound variable value   */
  RexxStem *      nextstem;            /* our working stem variable         */
  size_t          argcount;            /* size of the argument list         */
  bool            vpavailable;         /* Variable pool access flag         */
  int             object_scope;        /* reserve/release state of variables*/
};
#endif

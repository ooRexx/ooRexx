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
/* REXX Kernel                                                  RexxNativeActivation.hpp  */
/*                                                                            */
/* Primitive Native Activation Class Definitions                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxNativeActivation
#define Included_RexxNativeActivation

#include "RexxActivity.hpp"

class RexxNativeActivation : public RexxActivationBase {
 public:
  inline RexxNativeActivation(RESTORETYPE restoreType) { ; };
  void *operator new(size_t, RexxObject *, RexxMethod *, RexxActivity *, RexxString *, RexxActivationBase *);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  inline RexxNativeActivation() {;};
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope *);
  RexxObject *run(size_t, RexxObject **);
  RexxObject *saveObject(RexxObject *);
  RexxVariableDictionary *methodVariables();
  long   isInteger(RexxObject *);
  PCHAR  cstring(RexxObject *);
  double getDoubleValue(RexxObject *);
  long   isDouble(RexxObject *);
  PVOID  cself();
  PVOID  buffer();
  PVOID  pointer(RexxObject *);
  RexxObject *dispatch();
  RexxObject *getReceiver() {return  this->u_receiver;}
  void   traceBack(RexxList *);
  long   digits();
  long   fuzz();
  BOOL   form();
  void   setDigits(long);
  void   setFuzz(long);
  void   setForm(BOOL);
  void   guardOff();
  void   guardOn();
  void   enableVariablepool();
  void   disableVariablepool();
  BOOL   trap (RexxString *, RexxDirectory *);
  void   setObjNotify(RexxMessage *);
  void   resetNext();
  BOOL   fetchNext(RexxString **name, RexxObject **value);

  inline void   termination() { this->guardOff();}
  inline RexxActivation *sender() {return (RexxActivation *)this->activity->sender((RexxActivationBase *)this);}
  inline CHAR        getVpavailable()   {return this->vpavailable;}
  inline RexxMethod *getMethod()        {return this->method;}
  inline RexxString *getMsgname()       {return this->msgname;}
  inline LONG        nextVariable()     {return this->nextvariable;}
  inline RexxStem   *nextStem()         {return this->nextstem;}
  inline RexxVariableDictionary *nextCurrent()     {return this->nextcurrent;}
  inline RexxCompoundElement *compoundElement() {return this->compoundelement; }
  inline void        setNextVariable(LONG value)           {this->nextvariable = value;}
  inline void        setNextCurrent(RexxVariableDictionary *vdict)     {this->nextcurrent = vdict;}
  inline void        setNextStem(RexxStem *stem)     {this->nextstem = stem;}
  inline void        setCompoundElement(RexxCompoundElement *element)     {this->compoundelement = element;}
  inline void        setSyntaxHandler(jmp_buf *buffer)     {this->syntaxHandler = buffer;}


  RexxMethod     *method;              /* Method to run                     */
  RexxString     *msgname;             /* name of the message running       */
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
  LONG            nextvariable;        /* next variable to retrieve         */
  RexxVariableDictionary *nextcurrent; /* current processed vdict           */
  RexxCompoundElement *compoundelement;/* current compound variable value   */
  RexxStem *      nextstem;            /* our working stem variable         */
  SHORT           argcount;            /* size of the argument list         */
  CHAR            vpavailable;         /* Variable pool access flag         */
  CHAR            object_scope;        /* reserve/release state of variables*/
  jmp_buf         conditionjump;       /* condition trap recovery location  */
  jmp_buf        *syntaxHandler;       /* syntax/memory trapper             */
};
#endif

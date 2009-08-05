/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

#include "RexxActivity.hpp"
class RexxNativeCode;
class ActivityDispatcher;
class CallbackDispatcher;
class RexxNativeMethod;
class RexxNativeRoutine;
class RegisteredRoutine;
class RexxStem;
class RexxSupplier;
class StackFrameClass;

#define MAX_NATIVE_ARGUMENTS 16

class RexxNativeActivation : public RexxActivationBase
{
 public:
         void *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  inline void  operator delete(void *, void *) { ; }
  inline void  operator delete(void *) { ; }

  inline RexxNativeActivation(RESTORETYPE restoreType) { ; };
         RexxNativeActivation();
         RexxNativeActivation(RexxActivity *_activity, RexxActivation *_activation);
         RexxNativeActivation(RexxActivity *_activity);
  void live(size_t);
  void liveGeneral(int reason);
  void run(RexxMethod *_method, RexxNativeMethod *_code, RexxObject  *_receiver,
      RexxString  *_msgname, RexxObject **_arglist, size_t _argcount, ProtectedObject &resultObj);
  void run(ActivityDispatcher &dispatcher);
  void run(CallbackDispatcher &dispatcher);
  RexxVariableDictionary *methodVariables();
  bool   isInteger(RexxObject *);
  wholenumber_t signedIntegerValue(RexxObject *o, size_t position, wholenumber_t maxValue, wholenumber_t minValue);
  stringsize_t unsignedIntegerValue(RexxObject *o, size_t position, stringsize_t maxValue);
  int64_t int64Value(RexxObject *o, size_t position);
  uint64_t unsignedInt64Value(RexxObject *o, size_t position);
  const char *cstring(RexxObject *);
  double getDoubleValue(RexxObject *, size_t position);
  bool   isDouble(RexxObject *);
  void  *cself();
  void  *buffer();
  void  *pointer(RexxObject *);
  void  *pointerString(RexxObject *object, size_t position);
  RexxObject *dispatch();
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
  void   resetNext();
  bool   fetchNext(RexxString **name, RexxObject **value);
  void   raiseCondition(RexxString *condition, RexxString *description, RexxObject *additional, RexxObject *result);
  RexxArray *getArguments();
  RexxObject *getArgument(size_t index);
  RexxObject *getSuper();
  RexxObject *getScope();
  RexxStem *resolveStemVariable(RexxObject *s);
  RexxClass *findClass(RexxString *className);
  RexxClass *findCallerClass(RexxString *className);

  inline void   termination() { this->guardOff();}

  void   accessCallerContext();
  inline bool        getVpavailable()   {return this->vpavailable;}
  inline RexxString *getMessageName()   {return this->msgname;}
  inline size_t      nextVariable()     {return this->nextvariable;}
  inline RexxStem   *nextStem()         {return this->nextstem;}
  RexxObject *getContextStem(RexxString *name);
  RexxObject *getContextVariable(const char *name);
  void dropContextVariable(const char *name);
  void setContextVariable(const char *name, RexxObject *value);
  RexxObject *getObjectVariable(const char *name);
  void setObjectVariable(const char *name, RexxObject *value);
  void dropObjectVariable(const char *name);
  RexxDirectory *getAllContextVariables();
  inline void setConditionInfo(RexxDirectory *info) { conditionObj = info; }
  inline RexxDirectory *getConditionInfo() { return conditionObj; }
  inline void clearException() { conditionObj = OREF_NULL; }
  void checkConditions();
  inline RexxVariableDictionary *nextCurrent()     {return this->nextcurrent;}
  inline RexxCompoundElement *compoundElement() {return this->compoundelement; }
  inline void        setNextVariable(size_t value)           {this->nextvariable = value;}
  inline void        setNextCurrent(RexxVariableDictionary *vdict)     {this->nextcurrent = vdict;}
  inline void        setNextStem(RexxStem *stemVar)     {this->nextstem = stemVar;}
  inline void        setCompoundElement(RexxCompoundElement *element)     {this->compoundelement = element;}
  inline RexxObject *getSelf() { return receiver; }
  inline RexxActivity *getActivity() { return activity; }
  virtual bool isStackBase();
  virtual RexxActivation *getRexxContext();
  BaseExecutable *getRexxContextExecutable();
  RexxObject *getRexxContextObject();
  virtual RexxActivation *findRexxContext();
  virtual NumericSettings *getNumericSettings();
  virtual RexxObject *getReceiver();
  virtual SecurityManager *getSecurityManager();
  RexxSource *getSourceObject();
  inline void setStackBase() { stackBase = true; }
  void reportSignatureError();
  void reportStemError(size_t position, RexxObject *object);
  void processArguments(size_t argcount, RexxObject **arglist, uint16_t *argumentTypes, ValueDescriptor *descriptors, size_t maximumArgumentCount);
  RexxObject *valueToObject(ValueDescriptor *value);
  RexxArray *valuesToObject(ValueDescriptor *value, size_t count);
  bool objectToValue(RexxObject *o, ValueDescriptor *value);
  void createLocalReference(RexxObject *objr);
  void removeLocalReference(RexxObject *objr);
  void callNativeRoutine(RoutineClass *routine, RexxNativeRoutine *code, RexxString *functionName,
      RexxObject **list, size_t count, ProtectedObject &result);
  void callRegisteredRoutine(RoutineClass *routine, RegisteredRoutine *code, RexxString *functionName,
      RexxObject **list, size_t count, ProtectedObject &resultObj);

  RexxReturnCode variablePoolInterface(PSHVBLOCK requests);
  RexxVariableBase *variablePoolGetVariable(PSHVBLOCK pshvblock, bool symbolic);
  void variablePoolFetchVariable(PSHVBLOCK pshvblock);
  void variablePoolSetVariable(PSHVBLOCK pshvblock);
  void variablePoolDropVariable(PSHVBLOCK pshvblock);
  void variablePoolNextVariable(PSHVBLOCK pshvblock);
  void variablePoolFetchPrivate(PSHVBLOCK pshvblock);
  void variablePoolRequest(PSHVBLOCK pshvblock);
  RexxReturnCode copyValue(RexxObject * value, RXSTRING *rxstring, size_t *length);
  RexxReturnCode copyValue(RexxObject * value, CONSTRXSTRING *rxstring, size_t *length);
  int stemSort(const char *stemname, int order, int type, size_t start, size_t end, size_t firstcol, size_t lastcol);
  inline void enableConditionTrap() { trapConditions = true; }

  void forwardMessage(RexxObject *to, RexxString *msg, RexxClass *super, RexxArray *args, ProtectedObject &result);
  void enableConditionTraps() { trapErrors = true; }
  void disableConditionTraps() { trapErrors = false; }
  StackFrameClass *createStackFrame();

protected:

    typedef enum
    {
        PROGRAM_ACTIVATION,            // toplevel program entry
        METHOD_ACTIVATION,             // normal method call
        FUNCTION_ACTIVATION,           // function call activation
        DISPATCHER_ACTIVATION,         // running a top-level dispatcher
        CALLBACK_ACTIVATION,           // running a callback, such as an exit
    } ActivationType;

    RexxActivity   *activity;            /* current activity                  */
    RexxNativeCode *code;                // the code object controlling the target
    RexxObject     *receiver;            // the object receiving the message
    RexxString     *msgname;             /* name of the message running       */
    RexxActivation *activation;          /* parent activation                 */
    RexxObject    **arglist;             /* copy of the argument list         */
    RexxArray      *argArray;            /* optionally create argument array  */
    RexxList       *savelist;            /* list of saved objects             */
    RexxObject     *result;              /* result from RexxRaise call        */
    ActivationType  activationType;      // the type of activation
    RexxDirectory  *conditionObj;        // potential condition object
    SecurityManager *securityManager;    // our active security manager
                                         /* running object variable pool      */
    RexxVariableDictionary *objectVariables;
    size_t          nextvariable;        /* next variable to retrieve         */
    RexxVariableDictionary *nextcurrent; /* current processed vdict           */
    RexxCompoundElement *compoundelement;/* current compound variable value   */
    RexxStem *      nextstem;            /* our working stem variable         */
    size_t          argcount;            /* size of the argument list         */
    bool            vpavailable;         /* Variable pool access flag         */
    int             object_scope;        /* reserve/release state of variables*/
    bool            stackBase;           // this is a stack base marker
    bool            trapErrors;          // we're trapping errors from external callers
    bool            trapConditions;      // trap any raised conditions
};
#endif

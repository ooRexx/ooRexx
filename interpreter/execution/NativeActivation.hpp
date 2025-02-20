/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Native Activation Class Definitions                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_NativeActivation
#define Included_NativeActivation

#include "ActivationBase.hpp"
#include "Activity.hpp"

class NativeCode;
class ActivityDispatcher;
class CallbackDispatcher;
class TrappingDispatcher;
class NativeMethod;
class NativeRoutine;
class RegisteredRoutine;
class StemClass;
class SupplierClass;
class StackFrameClass;
class IdentityTable;
class RoutineClass;
class MethodClass;
class VariableReference;


/**
 * An object representing an activation of external
 * native code.  Used for handling external methods and
 * routines, as well as other callouts such as exits.
 */
class NativeActivation : public ActivationBase
{
   public:
           void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    inline NativeActivation(RESTORETYPE restoreType) { ; };
           NativeActivation();
           NativeActivation(Activity *_activity, RexxActivation *_activation);
           NativeActivation(Activity *_activity);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void run(MethodClass *_method, NativeMethod *_code, RexxObject  *_receiver,
        RexxString  *_msgname, RexxObject **_arglist, size_t _argcount, ProtectedObject &resultObj);

    RexxObject *dispatch() override;
    wholenumber_t digits() override;
    wholenumber_t fuzz() override;
    bool form() override;
    void setDigits(wholenumber_t) override;
    void setFuzz(wholenumber_t) override;
    void setForm(bool) override;
    bool trap(RexxString *, DirectoryClass *) override;
    bool willTrap(RexxString *) override;
    void termination() override { guardOff(); }
    bool isStackBase() override;
    RexxActivation *getRexxContext() override;
    RexxActivation *findRexxContext() override;
    RexxObject *getReceiver() override;
    PackageClass *getPackage() override { return getPackageObject(); }
    RexxObject *getContextObject() { return getContextObject(); }
    SecurityManager *getSecurityManager() override;
    const NumericSettings *getNumericSettings() override;

    MethodClass *getMethod();
    void run(ActivityDispatcher &dispatcher);
    void run(CallbackDispatcher &dispatcher);
    void run(TrappingDispatcher &dispatcher);
    VariableDictionary *methodVariables();
    wholenumber_t signedIntegerValue(RexxObject *o, size_t position, wholenumber_t maxValue, wholenumber_t minValue);
    wholenumber_t positiveWholeNumberValue(RexxObject *o, size_t position);
    wholenumber_t nonnegativeWholeNumberValue(RexxObject *o, size_t position);
    stringsize_t unsignedIntegerValue(RexxObject *o, size_t position, stringsize_t maxValue);
    int64_t int64Value(RexxObject *o, size_t position);
    uint64_t unsignedInt64Value(RexxObject *o, size_t position);
    const char *cstring(RexxObject *, size_t position);
    double getDoubleValue(RexxObject *, size_t position);
    bool   isDouble(RexxObject *);
    void  *cself();
    void  *buffer();
    void  *pointer(RexxObject *);
    void  *pointerString(RexxObject *object, size_t position);
    RexxVariableBase *getObjectVariableRetriever(const char *name);
    RexxObject *guardOnWhenUpdated(const char *name);
    RexxObject *guardOffWhenUpdated(const char *name);
    void   guardOff();
    void   guardOn();
    void   guardWait();
    void   enableVariablepool();
    void   disableVariablepool();
    void   resetNext();
    bool   fetchNext(RexxString *&name, RexxObject *&value);
    void   raiseCondition(RexxString *condition, RexxString *description, RexxObject *additional, RexxObject *result);
    ArrayClass *getArguments();
    RexxObject *getArgument(size_t index);
    RexxClass  *getSuper();
    RexxClass  *getScope();
    StemClass *resolveStemVariable(RexxObject *s);
    RexxClass *findClass(RexxString *className);
    RexxClass *findCallerClass(RexxString *className);


    void   accessCallerContext();
    inline bool isVariablePoolEnabled()   {return variablePoolEnabled;}
    inline RexxString *getMessageName()   {return messageName;}
    RexxObject *getContextStem(RexxString *name);
    RexxObject *getContextVariable(const char *name);
    VariableReference *getContextVariableReference(const char *name);
    void dropContextVariable(const char *name);
    void setContextVariable(const char *name, RexxObject *value);
    RexxObject *getObjectVariable(const char *name);
    VariableReference *getObjectVariableReference(const char *name);
    void setObjectVariable(const char *name, RexxObject *value);
    void dropObjectVariable(const char *name);
    DirectoryClass *getAllContextVariables();
    inline void setConditionInfo(RexxString *name, DirectoryClass *info) { conditionName = name; conditionObj = info; }
    void setConditionInfo(DirectoryClass *info);
    inline DirectoryClass *getConditionInfo() { return conditionObj; }
    inline void clearException() { conditionName = OREF_NULL; conditionObj = OREF_NULL; }
    void clearCondition();
    void checkConditions();
    bool checkCondition(RexxString *name);
    inline RexxObject *getSelf() { return receiver; }
    inline Activity *getActivity() { return activity; }
    BaseExecutable *getRexxContextExecutable();
    RexxObject *getRexxContextObject();
    PackageClass *getPackageObject();
    inline void setStackBase() { stackBase = true; }
    void reportSignatureError();
    void reportStemError(size_t position, RexxObject *object);
    void processArguments(size_t argcount, RexxObject **arglist, uint16_t *argumentTypes, ValueDescriptor *descriptors, size_t maximumArgumentCount);
    RexxObject *valueToObject(ValueDescriptor *value);
    ArrayClass *valuesToObject(ValueDescriptor *value, size_t count);
    bool objectToValue(RexxObject *o, ValueDescriptor *value);
    void createLocalReference(RexxInternalObject *objr);
    void removeLocalReference(RexxInternalObject *objr);
    void clearLocalReferences();
    void callNativeRoutine(RoutineClass *routine, NativeRoutine *code, RexxString *functionName,
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
    int stemSort(StemClass *stem, const char *tailExtension, int order, int type, wholenumber_t start, wholenumber_t end, wholenumber_t firstcol, wholenumber_t lastcol);
    inline void enableConditionTrap() { trapConditions = true; captureConditions = false; }
    inline void enableConditionCapture() { trapConditions = true; captureConditions = true; }

    void forwardMessage(RexxObject *to, RexxString *msg, RexxClass *super, ArrayClass *args, ProtectedObject &result);
    void enableConditionTraps() { trapErrors = true; }
    void disableConditionTraps() { trapErrors = false; }
    StackFrameClass *createStackFrame();
    void *allocateObjectMemory(size_t size);
    void  freeObjectMemory(void *data);
    void *reallocateObjectMemory(void *data, size_t newSize);

    inline bool isMethod() { return activationType == METHOD_ACTIVATION; }

    static const size_t MaxNativeArguments = 16;

protected:

    typedef enum
    {
        PROGRAM_ACTIVATION,              // toplevel program entry
        METHOD_ACTIVATION,               // normal method call
        FUNCTION_ACTIVATION,             // function call activation
        DISPATCHER_ACTIVATION,           // running a top-level dispatcher
        CALLBACK_ACTIVATION,             // running a callback, such as an exit
        TRAPPING_ACTIVATION,             // running a protected method call, such as an uninit
    } ActivationType;

    Activity       *activity;            // current activity
    NativeCode     *code;                // the code object controlling the target
    RexxObject     *receiver;            // the object receiving the message
    RexxString     *messageName;         // name of the message running
    RexxActivation *activation;          // parent activation
    size_t          argCount;            // size of the argument list
    RexxObject    **argList;             // copy of the argument list
    ArrayClass     *argArray;            // optionally create argument array
    RexxInternalObject *firstSavedObject;// first saved object...an optimization
    IdentityTable  *saveList;            // list of saved objects
    RexxObject     *result;              // result from RexxRaise call
    ActivationType activationType;       // the type of activation
    RexxString     *conditionName;       // name of the currently trapped condition
    DirectoryClass *conditionObj;        // potential condition object
    SecurityManager *securityManager;    // our active security manager
                                         // running object variable pool
    VariableDictionary *objectVariables;
    bool            stackBase;           // this is a stack base marker
    bool            trapErrors;          // we're trapping errors from external callers
    bool            trapConditions;      // trap any raised conditions
    bool            captureConditions;   // but don't propagate trapped conditions via throw.
    bool            variablePoolEnabled; // Variable pool access flag
                                         // our iterator for the variable pool next function
    VariableDictionary::VariableIterator iterator;
};
#endif

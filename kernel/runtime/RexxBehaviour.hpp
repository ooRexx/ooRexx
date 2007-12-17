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
/* REXX Kernel                                            RexxBehaviour.hpp   */
/*                                                                            */
/* Primitive Behaviour Class Definitions                                      */
/*                                                                            */
/*******************************************************************************/
#ifndef Included_RexxBehaviour
#define Included_RexxBehaviour

#define INTERNALCLASS (((uintptr_t)1) << ((sizeof(uintptr_t) * 8) - 1))

 class RexxBehaviour : public RexxInternalObject
 {
  public:
  void *operator new(size_t, size_t);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  inline void  operator delete(void *) { ; }
  inline void  operator delete(void *, void *) { ; }

  RexxBehaviour(size_t, PCPPM *);
  inline RexxBehaviour() {;};
  inline RexxBehaviour(RESTORETYPE restoreType) { ; };
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);
  RexxObject *copy();
  void        copyBehaviour(RexxBehaviour *source);
  RexxObject *define(RexxString *, RexxMethod *);
  void        addMethod(RexxString *, RexxMethod *);
  void        removeMethod(RexxString *);
  RexxMethod *methodObject(RexxString *);
  RexxMethod *methodLookup( RexxString *);
  RexxMethod *getMethod( RexxString *);
  RexxObject *deleteMethod(RexxString *);
  void restore(RexxBehaviour *);
  RexxClass  *restoreClass();
  RexxObject *superScope( RexxObject *);
  RexxMethod *superMethod(RexxString *, RexxObject *);
  void        setMethodDictionaryScope( RexxObject *);
  RexxObject *setScopes( RexxObjectTable *);
  RexxObject *addScope( RexxObject *);
  RexxObject *mergeScope( RexxObject *);
  bool        checkScope( RexxObject *);
  void        subclass(RexxBehaviour *);
  RexxSupplier *getMethods(RexxObject *scope);

  void        resolveNonPrimitiveBehaviour();

  void merge( RexxBehaviour *);
  void methodDictionaryMerge( RexxTable *);


   inline RexxObjectTable  *getScopes()       { return this->scopes; };
   inline RexxTable  *getMethodDictionary()   { return this->methodDictionary; };
   inline void        setMethodDictionary(RexxTable * m) { OrefSet(this, this->methodDictionary, m); };
   inline void        setInstanceMethodDictionary(RexxTable * m) { OrefSet(this, this->instanceMethodDictionary, m); };
   inline RexxTable  *getInstanceMethodDictionary()   { return this->instanceMethodDictionary; };
   inline RexxClass  *getOwningClass()        { return this->owningClass;};
   inline void        setOwningClass(RexxClass *c)  { OrefSet(this, this->owningClass,  c); };

   inline void setClassType(size_t n) { classType = (uint16_t)n; }
   inline size_t getClassType()  { return (size_t)classType; }
   inline bool  isPrimitive()    {  return (behaviourFlags & NON_PRIMITIVE_BEHAVIOUR) == 0; };
   inline bool  isNonPrimitive() {  return (behaviourFlags & NON_PRIMITIVE_BEHAVIOUR) != 0; };
   inline bool  isNotResolved()  {  return (behaviourFlags & BEHAVIOUR_NOT_RESOLVED) != 0; };
   inline bool  isResolved()     {  return (behaviourFlags & BEHAVIOUR_NOT_RESOLVED) == 0; };
   inline bool  isEnhanced()     {  return (behaviourFlags & ENHANCED_OBJECT) != 0; };
   inline bool  isInternalClass()  {  return (behaviourFlags & INTERNAL_CLASS) != 0; };
   inline bool  isTransientClass()  {  return (behaviourFlags & INTERNAL_CLASS) != 0; };
   inline void  setResolved()    {  behaviourFlags &= ~BEHAVIOUR_NOT_RESOLVED; };
   inline void  setNotResolved() {  behaviourFlags |= BEHAVIOUR_NOT_RESOLVED; };
   inline void  setEnhanced()    {  behaviourFlags |= ENHANCED_OBJECT; };
   inline void  setNonPrimitive() {  behaviourFlags |= NON_PRIMITIVE_BEHAVIOUR; };
   inline void  setInternalClass() { behaviourFlags |= INTERNAL_CLASS; };
   inline void  setTransientClass() { behaviourFlags |= TRANSIENT_CLASS; };

   inline RexxBehaviour *getSavedPrimitiveBehaviour()
   {
       uintptr_t behaviourID = (uintptr_t)this->getClassType();
       // if this is an internal class, normalize this so we can
       // restore this to the correct value if we add additional internal classes.
       if (isInternalClass())
       {
           behaviourID -= T_Last_Exported_Class;
           behaviourID |= INTERNALCLASS;
       }
       return (RexxBehaviour *)behaviourID;
   }

   static inline RexxBehaviour *restoreSavedPrimitiveBehaviour(RexxBehaviour *b)
   {
       uintptr_t behaviourID = (uintptr_t)b;
       // if this is an internal class, we need to convert back to
       // the relative internal class id
       if ((behaviourID & INTERNALCLASS) != 0)
       {
           behaviourID &= ~INTERNALCLASS;    // turn off the internal marker
           behaviourID += T_Last_Exported_Class; // turn back into an internal class
       }
       return &primitiveBehaviours[behaviourID];          // translate back into proper behaviour
   }

   inline PCPPM getOperatorMethod(size_t index) { return operatorMethods[index]; }
   static inline RexxBehaviour *getPrimitiveBehaviour(size_t index) { return &primitiveBehaviours[index]; }
   static inline PCPPM *getOperatorMethods(size_t index) { return getPrimitiveBehaviour(index)->operatorMethods; }
   // table of primitive behaviour objects
   static RexxBehaviour primitiveBehaviours[];

 protected:

   enum
   {
       NON_PRIMITIVE_BEHAVIOUR = 0x0001,
       ENHANCED_OBJECT         = 0x0002,
       INTERNAL_CLASS          = 0x0004,
       TRANSIENT_CLASS         = 0x0004,
       BEHAVIOUR_NOT_RESOLVED  = 0x0010
   };


   uint16_t classType;         // primitive class identifier
   uint16_t behaviourFlags;    // various behaviour flag types
   RexxObjectTable  *scopes;           /* scopes table                      */
   RexxTable  *methodDictionary;       /* method dictionary                 */
   PCPPM      *operatorMethods;        /* operator look-a-side table        */
   RexxClass  *owningClass;            /* class that created this object    */
                                       /* methods added via SETMETHOD       */
   RexxTable  *instanceMethodDictionary;

 };


/******************************************************************************/
/* Global Objects - Primitive Behaviour                                       */
/******************************************************************************/

#define TheActivationBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Activation]))
#define TheActivityBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Activity]))
#define TheArrayBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Array]))
#define TheArrayClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ArrayClass]))
#define TheBehaviourBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Behaviour]))
#define TheBufferBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Buffer]))
#define TheClassBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Class]))
#define TheClassClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ClassClass]))
#define TheDirectoryBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Directory]))
#define TheDirectoryClassBehaviour  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_DirectoryClass]))
#define TheEnvelopeBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Envelope]))
#define TheHashTableBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_HashTable]))
#define TheIntegerBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Integer]))
#define TheIntegerClassBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_IntegerClass]))
#define TheListBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_List]))
#define TheListClassBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ListClass]))
#define TheListTableBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ListTable]))
#define TheMemoryBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Memory]))
#define TheMessageBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Message]))
#define TheMessageClassBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_MessageClass]))
#define TheMethodBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Method]))
#define TheMethodClassBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_MethodClass]))
#define TheNativeCodeBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_NativeCode]))
#define TheRexxCodeBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_RexxCode]))
#define TheNativeActivationBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_NativeActivation]))
#define TheNumberStringBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_NumberString]))
#define TheNumberStringClassBehaviour  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_NumberStringClass]))
#define TheObjectBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Object]))
#define TheObjectClassBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ObjectClass]))
#define TheQueueBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Queue]))
#define TheQueueClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_QueueClass]))
#define TheSmartBufferBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_SmartBuffer]))
#define TheStackBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Stack]))
#define TheStemBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Stem]))
#define TheStemClassBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_StemClass]))
#define TheStringBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_String]))
#define TheStringClassBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_StringClass]))
#define TheSupplierBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Supplier]))
#define TheSupplierClassBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_SupplierClass]))
#define TheTableBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Table]))
#define TheTableClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_TableClass]))
#define TheRelationBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Relation]))
#define TheRelationClassBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_RelationClass]))
#define TheVariableBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Variable]))
#define TheCompoundElementBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_CompoundElement]))
#define TheVariableDictionaryBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_VariableDictionary]))
#define TheMutableBufferBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_MutableBuffer]))
#define TheMutableBufferClassBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_MutableBufferClass]))

#define TheAddressInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_AddressInstruction]))
#define TheAssignmentInstructionBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_AssignmentInstruction]))
#define TheDoBlockBehaviour                 ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_DoBlock]))
#define TheCallInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_CallInstruction]))
#define TheCommandInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_CommandInstruction]))
#define TheCompoundVariableBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_CompoundVariableTerm]))
#define TheDoInstructionBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_DoInstruction]))
#define TheDotVariableBehaviour             ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_DotVariableTerm]))
#define TheDropInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_DropInstruction]))
#define TheElseInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ElseInstruction]))
#define TheEndInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_EndInstruction]))
#define TheEndIfInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_EndIfInstruction]))
#define TheExitInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ExitInstruction]))
#define TheExposeInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ExposeInstruction]))
#define TheForwardInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ForwardInstruction]))
#define TheFunctionBehaviour                ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_FunctionCallTerm]))
#define TheLogicalBehaviour                 ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_LogicalTerm]))
#define TheGuardInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_GuardInstruction]))
#define TheIfInstructionBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_IfInstruction]))
#define TheInstructionBehaviour             ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Instruction]))
#define TheInterpretInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_InterpretInstruction]))
#define TheLabelInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_LabelInstruction]))
#define TheLeaveInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_LeaveInstruction]))
#define TheMessageInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_MessageInstruction]))
#define TheMessageSendBehaviour             ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_MessageSendTerm]))
#define TheNopInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_NopInstruction]))
#define TheNumericInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_NumericInstruction]))
#define TheUnaryOperatorBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_UnaryOperatorTerm]))
#define TheBinaryOperatorBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_BinaryOperatorTerm]))
#define TheOptionsInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_OptionsInstruction]))
#define TheOtherWiseInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_OtherwiseInstruction]))
#define TheParseInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ParseInstruction]))
#define TheProcedureInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ProcedureInstruction]))
#define TheQueueInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_QueueInstruction]))
#define TheRaiseInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_RaiseInstruction]))
#define TheReplyInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ReplyInstruction]))
#define TheReturnInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ReturnInstruction]))
#define TheSayInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_SayInstruction]))
#define TheSelectInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_SelectInstruction]))
#define TheSignalInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_SignalInstruction]))
#define TheStemVariableBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_StemVariableTerm]))
#define TheThenInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ThenInstruction]))
#define TheTraceInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_TraceInstruction]))
#define TheParseTriggerBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ParseTrigger]))
#define TheUseInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_UseInstruction]))
#define TheParseVariableBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_VariableTerm]))
#define TheVariableReferenceBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_IndirectVariableTerm]))
#define TheSourceBehaviour                  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_RexxSource]))
#define TheClauseBehaviour                  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Clause]))
#define TheTokenBehaviour                   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_Token]))
#define TheInternalStackBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_InternalStack]))
#define TheActivationFrameBufferBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_ActivationFrameBuffer]))

#endif

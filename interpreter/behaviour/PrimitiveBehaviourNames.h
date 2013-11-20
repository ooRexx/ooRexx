

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* REXX  Support                                                              */
/*                                                                            */
/* Defines for mapping class ids to behaviours                                */
/*                                                                            */
/*        -- DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST! --            */
/******************************************************************************/

#ifndef PrimitiveBehaviourNames_Included
#define PrimitiveBehaviourNames_Included

    
#define TheObjectBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Object])
#define TheObjectClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_ObjectClass])
#define TheClassBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Class])
#define TheClassClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_ClassClass])
#define TheArrayBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Array])
#define TheArrayClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_ArrayClass])
#define TheDirectoryBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Directory])
#define TheDirectoryClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_DirectoryClass])
#define TheIntegerBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Integer])
#define TheIntegerClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_IntegerClass])
#define TheListBehaviour      (&RexxBehaviour::primitiveBehaviours[T_List])
#define TheListClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_ListClass])
#define TheMessageBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Message])
#define TheMessageClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_MessageClass])
#define TheMethodBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Method])
#define TheMethodClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_MethodClass])
#define TheNumberStringBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NumberString])
#define TheNumberStringClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_NumberStringClass])
#define TheQueueBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Queue])
#define TheQueueClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_QueueClass])
#define TheStemBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Stem])
#define TheStemClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_StemClass])
#define TheStringBehaviour      (&RexxBehaviour::primitiveBehaviours[T_String])
#define TheStringClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_StringClass])
#define TheSupplierBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Supplier])
#define TheSupplierClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_SupplierClass])
#define TheTableBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Table])
#define TheTableClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_TableClass])
#define TheRelationBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Relation])
#define TheRelationClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_RelationClass])
#define TheMutableBufferBehaviour      (&RexxBehaviour::primitiveBehaviours[T_MutableBuffer])
#define TheMutableBufferClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_MutableBufferClass])
#define ThePointerBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Pointer])
#define ThePointerClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_PointerClass])
#define TheBufferBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Buffer])
#define TheBufferClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_BufferClass])
#define TheWeakReferenceBehaviour      (&RexxBehaviour::primitiveBehaviours[T_WeakReference])
#define TheWeakReferenceClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_WeakReferenceClass])
#define TheRoutineBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Routine])
#define TheRoutineClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_RoutineClass])
#define ThePackageBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Package])
#define ThePackageClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_PackageClass])
#define TheRexxContextBehaviour      (&RexxBehaviour::primitiveBehaviours[T_RexxContext])
#define TheRexxContextClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_RexxContextClass])
#define TheIdentityTableBehaviour      (&RexxBehaviour::primitiveBehaviours[T_IdentityTable])
#define TheIdentityTableClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_IdentityTableClass])
#define TheStackFrameBehaviour      (&RexxBehaviour::primitiveBehaviours[T_StackFrame])
#define TheStackFrameClassBehaviour    (&RexxBehaviour::primitiveBehaviours[T_StackFrameClass])
#define TheNilObjectBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NilObject])
#define TheBehaviourBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Behaviour])
#define TheRexxSourceBehaviour      (&RexxBehaviour::primitiveBehaviours[T_RexxSource])
#define TheLibraryPackageBehaviour      (&RexxBehaviour::primitiveBehaviours[T_LibraryPackage])
#define TheRexxCodeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_RexxCode])
#define TheNativeMethodBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NativeMethod])
#define TheNativeRoutineBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NativeRoutine])
#define TheRegisteredRoutineBehaviour      (&RexxBehaviour::primitiveBehaviours[T_RegisteredRoutine])
#define TheCPPCodeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_CPPCode])
#define TheAttributeGetterCodeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_AttributeGetterCode])
#define TheAttributeSetterCodeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_AttributeSetterCode])
#define TheConstantGetterCodeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ConstantGetterCode])
#define TheAbstractCodeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_AbstractCode])
#define TheHashTableBehaviour      (&RexxBehaviour::primitiveBehaviours[T_HashTable])
#define TheListTableBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ListTable])
#define TheSmartBufferBehaviour      (&RexxBehaviour::primitiveBehaviours[T_SmartBuffer])
#define TheVariableBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Variable])
#define TheVariableDictionaryBehaviour      (&RexxBehaviour::primitiveBehaviours[T_VariableDictionary])
#define TheVariableTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_VariableTerm])
#define TheCompoundVariableTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_CompoundVariableTerm])
#define TheStemVariableTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_StemVariableTerm])
#define TheDotVariableTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_DotVariableTerm])
#define TheIndirectVariableTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_IndirectVariableTerm])
#define TheFunctionCallTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_FunctionCallTerm])
#define TheMessageSendTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_MessageSendTerm])
#define TheUnaryOperatorTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_UnaryOperatorTerm])
#define TheBinaryOperatorTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_BinaryOperatorTerm])
#define TheLogicalTermBehaviour      (&RexxBehaviour::primitiveBehaviours[T_LogicalTerm])
#define TheInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Instruction])
#define TheAddressInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_AddressInstruction])
#define TheAssignmentInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_AssignmentInstruction])
#define TheCallInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_CallInstruction])
#define TheCommandInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_CommandInstruction])
#define TheDoInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_DoInstruction])
#define TheDropInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_DropInstruction])
#define TheElseInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ElseInstruction])
#define TheEndInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_EndInstruction])
#define TheEndIfInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_EndIfInstruction])
#define TheExitInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ExitInstruction])
#define TheExposeInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ExposeInstruction])
#define TheForwardInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ForwardInstruction])
#define TheGuardInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_GuardInstruction])
#define TheIfInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_IfInstruction])
#define TheInterpretInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_InterpretInstruction])
#define TheLabelInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_LabelInstruction])
#define TheLeaveInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_LeaveInstruction])
#define TheMessageInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_MessageInstruction])
#define TheNopInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NopInstruction])
#define TheNumericInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NumericInstruction])
#define TheOptionsInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_OptionsInstruction])
#define TheOtherwiseInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_OtherwiseInstruction])
#define TheParseInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ParseInstruction])
#define TheProcedureInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ProcedureInstruction])
#define TheQueueInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_QueueInstruction])
#define TheRaiseInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_RaiseInstruction])
#define TheReplyInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ReplyInstruction])
#define TheReturnInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ReturnInstruction])
#define TheSayInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_SayInstruction])
#define TheSelectInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_SelectInstruction])
#define TheSignalInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_SignalInstruction])
#define TheThenInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ThenInstruction])
#define TheTraceInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_TraceInstruction])
#define TheUseInstructionBehaviour      (&RexxBehaviour::primitiveBehaviours[T_UseInstruction])
#define TheClassDirectiveBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ClassDirective])
#define TheLibraryDirectiveBehaviour      (&RexxBehaviour::primitiveBehaviours[T_LibraryDirective])
#define TheRequiresDirectiveBehaviour      (&RexxBehaviour::primitiveBehaviours[T_RequiresDirective])
#define TheCompoundElementBehaviour      (&RexxBehaviour::primitiveBehaviours[T_CompoundElement])
#define TheParseTriggerBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ParseTrigger])
#define TheMemoryBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Memory])
#define TheInternalStackBehaviour      (&RexxBehaviour::primitiveBehaviours[T_InternalStack])
#define TheStackBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Stack])
#define TheActivityBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Activity])
#define TheActivationBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Activation])
#define TheNativeActivationBehaviour      (&RexxBehaviour::primitiveBehaviours[T_NativeActivation])
#define TheActivationFrameBufferBehaviour      (&RexxBehaviour::primitiveBehaviours[T_ActivationFrameBuffer])
#define TheEnvelopeBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Envelope])
#define TheClauseBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Clause])
#define TheTokenBehaviour      (&RexxBehaviour::primitiveBehaviours[T_Token])
#define TheDoBlockBehaviour      (&RexxBehaviour::primitiveBehaviours[T_DoBlock])
#define TheInterpreterInstanceBehaviour      (&RexxBehaviour::primitiveBehaviours[T_InterpreterInstance])
#define TheSecurityManagerBehaviour      (&RexxBehaviour::primitiveBehaviours[T_SecurityManager])
#define TheCommandHandlerBehaviour      (&RexxBehaviour::primitiveBehaviours[T_CommandHandler])


/* -------------------------------------------------------------------------- */
/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */
#endif


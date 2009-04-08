

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
/* REXX Kernel                                                                */
/*                                                                            */
/* Build the table of virtual functions assigned to Rexx class instances      */
/*                                                                            */
/******************************************************************************/

/* -------------------------------------------------------------------------- */
/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */

#include "RexxCore.h"

   
#include "ObjectClass.hpp"
#include "ClassClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "IntegerClass.hpp"
#include "ListClass.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "NumberStringClass.hpp"
#include "QueueClass.hpp"
#include "StemClass.hpp"
#include "StringClass.hpp"
#include "SupplierClass.hpp"
#include "TableClass.hpp"
#include "RelationClass.hpp"
#include "MutableBufferClass.hpp"
#include "PointerClass.hpp"
#include "BufferClass.hpp"
#include "WeakReferenceClass.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "ContextClass.hpp"
#include "IdentityTableClass.hpp"
#include "RexxBehaviour.hpp"
#include "SourceFile.hpp"
#include "LibraryPackage.hpp"
#include "RexxCode.hpp"
#include "RexxNativeCode.hpp"
#include "CPPCode.hpp"
#include "RexxHashTable.hpp"
#include "RexxListTable.hpp"
#include "RexxSmartBuffer.hpp"
#include "RexxVariable.hpp"
#include "RexxVariableDictionary.hpp"
#include "ExpressionVariable.hpp"
#include "ExpressionCompoundVariable.hpp"
#include "ExpressionStem.hpp"
#include "ExpressionDotVariable.hpp"
#include "IndirectVariableReference.hpp"
#include "ExpressionFunction.hpp"
#include "ExpressionMessage.hpp"
#include "ExpressionOperator.hpp"
#include "ExpressionLogical.hpp"
#include "RexxInstruction.hpp"
#include "AddressInstruction.hpp"
#include "AssignmentInstruction.hpp"
#include "CallInstruction.hpp"
#include "CommandInstruction.hpp"
#include "DoInstruction.hpp"
#include "DropInstruction.hpp"
#include "ElseInstruction.hpp"
#include "EndInstruction.hpp"
#include "EndIf.hpp"
#include "ExitInstruction.hpp"
#include "ExposeInstruction.hpp"
#include "ForwardInstruction.hpp"
#include "GuardInstruction.hpp"
#include "IfInstruction.hpp"
#include "InterpretInstruction.hpp"
#include "LabelInstruction.hpp"
#include "LeaveInstruction.hpp"
#include "MessageInstruction.hpp"
#include "NopInstruction.hpp"
#include "NumericInstruction.hpp"
#include "OptionsInstruction.hpp"
#include "OtherwiseInstruction.hpp"
#include "ParseInstruction.hpp"
#include "ProcedureInstruction.hpp"
#include "QueueInstruction.hpp"
#include "RaiseInstruction.hpp"
#include "ReplyInstruction.hpp"
#include "ReturnInstruction.hpp"
#include "SayInstruction.hpp"
#include "SelectInstruction.hpp"
#include "SignalInstruction.hpp"
#include "ThenInstruction.hpp"
#include "TraceInstruction.hpp"
#include "UseStrictInstruction.hpp"
#include "ClassDirective.hpp"
#include "LibraryDirective.hpp"
#include "RequiresDirective.hpp"
#include "RexxCompoundElement.hpp"
#include "ParseTrigger.hpp"
#include "RexxMemory.hpp"
#include "RexxInternalStack.hpp"
#include "StackClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxActivationStack.hpp"
#include "RexxEnvelope.hpp"
#include "Clause.hpp"
#include "Token.hpp"
#include "DoBlock.hpp"
#include "InterpreterInstance.hpp"
#include "SecurityManager.hpp"
#include "CommandHandler.hpp"


void *RexxMemory::virtualFunctionTable[T_Last_Class_Type + 1] = {NULL};

void RexxMemory::buildVirtualFunctionTable()
/******************************************************************************/
/* Function:  This routine will build an array of the virtualFunctions        */
/*            There will be one for each Class.                               */
/******************************************************************************/
{
    uintptr_t objectBuffer[256];       /* buffer for each object            */
    void *objectPtr;

    objectPtr = objectBuffer;
    // instantiate an instance of each class into the buffer and
    // grab the resulting virtual function table
   
   objectPtr = new (objectPtr) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Object] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ObjectClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_Class] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ClassClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxArray(RESTOREIMAGE);
   virtualFunctionTable[T_Array] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxDirectory(RESTOREIMAGE);
   virtualFunctionTable[T_Directory] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_DirectoryClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInteger(RESTOREIMAGE);
   virtualFunctionTable[T_Integer] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxIntegerClass(RESTOREIMAGE);
   virtualFunctionTable[T_IntegerClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxList(RESTOREIMAGE);
   virtualFunctionTable[T_List] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ListClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxMessage(RESTOREIMAGE);
   virtualFunctionTable[T_Message] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MessageClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxMethod(RESTOREIMAGE);
   virtualFunctionTable[T_Method] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MethodClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxNumberString(RESTOREIMAGE);
   virtualFunctionTable[T_NumberString] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_NumberStringClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxQueue(RESTOREIMAGE);
   virtualFunctionTable[T_Queue] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_QueueClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxStem(RESTOREIMAGE);
   virtualFunctionTable[T_Stem] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StemClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxString(RESTOREIMAGE);
   virtualFunctionTable[T_String] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StringClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxSupplier(RESTOREIMAGE);
   virtualFunctionTable[T_Supplier] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_SupplierClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxTable(RESTOREIMAGE);
   virtualFunctionTable[T_Table] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_TableClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxRelation(RESTOREIMAGE);
   virtualFunctionTable[T_Relation] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RelationClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxMutableBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBuffer] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBufferClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxPointer(RESTOREIMAGE);
   virtualFunctionTable[T_Pointer] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PointerClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_Buffer] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_BufferClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) WeakReference(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReference] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReferenceClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RoutineClass(RESTOREIMAGE);
   virtualFunctionTable[T_Routine] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RoutineClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) PackageClass(RESTOREIMAGE);
   virtualFunctionTable[T_Package] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PackageClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxContext(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContext] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContextClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxIdentityTable(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTable] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTableClass] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxNilObject(RESTOREIMAGE);
   virtualFunctionTable[T_NilObject] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxBehaviour(RESTOREIMAGE);
   virtualFunctionTable[T_Behaviour] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxSource(RESTOREIMAGE);
   virtualFunctionTable[T_RexxSource] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) LibraryPackage(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryPackage] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxCode(RESTOREIMAGE);
   virtualFunctionTable[T_RexxCode] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxNativeMethod(RESTOREIMAGE);
   virtualFunctionTable[T_NativeMethod] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxNativeRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_NativeRoutine] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RegisteredRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_RegisteredRoutine] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) CPPCode(RESTOREIMAGE);
   virtualFunctionTable[T_CPPCode] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) AttributeGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeGetterCode] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) AttributeSetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeSetterCode] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) ConstantGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_ConstantGetterCode] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) AbstractCode(RESTOREIMAGE);
   virtualFunctionTable[T_AbstractCode] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxHashTable(RESTOREIMAGE);
   virtualFunctionTable[T_HashTable] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxListTable(RESTOREIMAGE);
   virtualFunctionTable[T_ListTable] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxSmartBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_SmartBuffer] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxVariable(RESTOREIMAGE);
   virtualFunctionTable[T_Variable] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxVariableDictionary(RESTOREIMAGE);
   virtualFunctionTable[T_VariableDictionary] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxParseVariable(RESTOREIMAGE);
   virtualFunctionTable[T_VariableTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxCompoundVariable(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundVariableTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxStemVariable(RESTOREIMAGE);
   virtualFunctionTable[T_StemVariableTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxDotVariable(RESTOREIMAGE);
   virtualFunctionTable[T_DotVariableTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxVariableReference(RESTOREIMAGE);
   virtualFunctionTable[T_IndirectVariableTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxExpressionFunction(RESTOREIMAGE);
   virtualFunctionTable[T_FunctionCallTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxExpressionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageSendTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxUnaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_UnaryOperatorTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxBinaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_BinaryOperatorTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxExpressionLogical(RESTOREIMAGE);
   virtualFunctionTable[T_LogicalTerm] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstruction(RESTOREIMAGE);
   virtualFunctionTable[T_Instruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionAddress(RESTOREIMAGE);
   virtualFunctionTable[T_AddressInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionAssignment(RESTOREIMAGE);
   virtualFunctionTable[T_AssignmentInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionCall(RESTOREIMAGE);
   virtualFunctionTable[T_CallInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionCommand(RESTOREIMAGE);
   virtualFunctionTable[T_CommandInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionDo(RESTOREIMAGE);
   virtualFunctionTable[T_DoInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionDrop(RESTOREIMAGE);
   virtualFunctionTable[T_DropInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionElse(RESTOREIMAGE);
   virtualFunctionTable[T_ElseInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionEnd(RESTOREIMAGE);
   virtualFunctionTable[T_EndInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionEndIf(RESTOREIMAGE);
   virtualFunctionTable[T_EndIfInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionExit(RESTOREIMAGE);
   virtualFunctionTable[T_ExitInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionExpose(RESTOREIMAGE);
   virtualFunctionTable[T_ExposeInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionForward(RESTOREIMAGE);
   virtualFunctionTable[T_ForwardInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionGuard(RESTOREIMAGE);
   virtualFunctionTable[T_GuardInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionIf(RESTOREIMAGE);
   virtualFunctionTable[T_IfInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionInterpret(RESTOREIMAGE);
   virtualFunctionTable[T_InterpretInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionLabel(RESTOREIMAGE);
   virtualFunctionTable[T_LabelInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionLeave(RESTOREIMAGE);
   virtualFunctionTable[T_LeaveInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionNop(RESTOREIMAGE);
   virtualFunctionTable[T_NopInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionNumeric(RESTOREIMAGE);
   virtualFunctionTable[T_NumericInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionOptions(RESTOREIMAGE);
   virtualFunctionTable[T_OptionsInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionOtherwise(RESTOREIMAGE);
   virtualFunctionTable[T_OtherwiseInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionParse(RESTOREIMAGE);
   virtualFunctionTable[T_ParseInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionProcedure(RESTOREIMAGE);
   virtualFunctionTable[T_ProcedureInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionQueue(RESTOREIMAGE);
   virtualFunctionTable[T_QueueInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionRaise(RESTOREIMAGE);
   virtualFunctionTable[T_RaiseInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionReply(RESTOREIMAGE);
   virtualFunctionTable[T_ReplyInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionReturn(RESTOREIMAGE);
   virtualFunctionTable[T_ReturnInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionSay(RESTOREIMAGE);
   virtualFunctionTable[T_SayInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionSelect(RESTOREIMAGE);
   virtualFunctionTable[T_SelectInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionSignal(RESTOREIMAGE);
   virtualFunctionTable[T_SignalInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionThen(RESTOREIMAGE);
   virtualFunctionTable[T_ThenInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionTrace(RESTOREIMAGE);
   virtualFunctionTable[T_TraceInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInstructionUseStrict(RESTOREIMAGE);
   virtualFunctionTable[T_UseInstruction] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) ClassDirective(RESTOREIMAGE);
   virtualFunctionTable[T_ClassDirective] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) LibraryDirective(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryDirective] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RequiresDirective(RESTOREIMAGE);
   virtualFunctionTable[T_RequiresDirective] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxCompoundElement(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundElement] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxTrigger(RESTOREIMAGE);
   virtualFunctionTable[T_ParseTrigger] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Memory] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxInternalStack(RESTOREIMAGE);
   virtualFunctionTable[T_InternalStack] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxStack(RESTOREIMAGE);
   virtualFunctionTable[T_Stack] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxActivity(RESTOREIMAGE);
   virtualFunctionTable[T_Activity] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxActivation(RESTOREIMAGE);
   virtualFunctionTable[T_Activation] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxNativeActivation(RESTOREIMAGE);
   virtualFunctionTable[T_NativeActivation] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxActivationFrameBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_ActivationFrameBuffer] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxEnvelope(RESTOREIMAGE);
   virtualFunctionTable[T_Envelope] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxClause(RESTOREIMAGE);
   virtualFunctionTable[T_Clause] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxToken(RESTOREIMAGE);
   virtualFunctionTable[T_Token] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) RexxDoBlock(RESTOREIMAGE);
   virtualFunctionTable[T_DoBlock] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) InterpreterInstance(RESTOREIMAGE);
   virtualFunctionTable[T_InterpreterInstance] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) SecurityManager(RESTOREIMAGE);
   virtualFunctionTable[T_SecurityManager] = *((void **)objectPtr);
   
   objectPtr = new (objectPtr) CommandHandler(RESTOREIMAGE);
   virtualFunctionTable[T_CommandHandler] = *((void **)objectPtr);
   
};


/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */


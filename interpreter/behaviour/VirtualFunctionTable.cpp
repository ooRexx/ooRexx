

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
#include "StackFrameClass.hpp"
#include "ExceptionClass.hpp"
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

/******************************************************************************/
/* Function:  This small function is necessary to void optimizer problems on  */
/*            some versions of GCC.  The optimizer appears to keep storing    */
/*            the same value in the VFT rather than picking up the new VFT    */
/*            for each class.  Making this a separate routine avoids this.    */
/******************************************************************************/
void *getVftPointer(void *loc)
{
    return *((void **)loc);
}

void RexxMemory::buildVirtualFunctionTable()
/******************************************************************************/
/* Function:  This routine will build an array of the virtualFunctions        */
/*            There will be one for each Class.                               */
/******************************************************************************/
{
    uintptr_t objectBuffer[256];       /* buffer for each object            */
    volatile void *objectPtr;

    void *objectLoc = objectBuffer;
    // instantiate an instance of each class into the buffer and
    // grab the resulting virtual function table
   
   objectPtr = new (objectLoc) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Object] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ObjectClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_Class] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ClassClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxArray(RESTOREIMAGE);
   virtualFunctionTable[T_Array] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxDirectory(RESTOREIMAGE);
   virtualFunctionTable[T_Directory] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_DirectoryClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInteger(RESTOREIMAGE);
   virtualFunctionTable[T_Integer] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxIntegerClass(RESTOREIMAGE);
   virtualFunctionTable[T_IntegerClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxList(RESTOREIMAGE);
   virtualFunctionTable[T_List] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ListClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxMessage(RESTOREIMAGE);
   virtualFunctionTable[T_Message] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MessageClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxMethod(RESTOREIMAGE);
   virtualFunctionTable[T_Method] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MethodClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxNumberString(RESTOREIMAGE);
   virtualFunctionTable[T_NumberString] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_NumberStringClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxQueue(RESTOREIMAGE);
   virtualFunctionTable[T_Queue] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_QueueClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxStem(RESTOREIMAGE);
   virtualFunctionTable[T_Stem] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StemClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxString(RESTOREIMAGE);
   virtualFunctionTable[T_String] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StringClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxSupplier(RESTOREIMAGE);
   virtualFunctionTable[T_Supplier] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_SupplierClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxTable(RESTOREIMAGE);
   virtualFunctionTable[T_Table] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_TableClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxRelation(RESTOREIMAGE);
   virtualFunctionTable[T_Relation] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RelationClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxMutableBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBuffer] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBufferClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxPointer(RESTOREIMAGE);
   virtualFunctionTable[T_Pointer] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PointerClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_Buffer] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_BufferClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) WeakReference(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReference] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReferenceClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RoutineClass(RESTOREIMAGE);
   virtualFunctionTable[T_Routine] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RoutineClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) PackageClass(RESTOREIMAGE);
   virtualFunctionTable[T_Package] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PackageClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxContext(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContext] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContextClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxIdentityTable(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTable] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTableClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) StackFrameClass(RESTOREIMAGE);
   virtualFunctionTable[T_StackFrame] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StackFrameClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) ExceptionClass(RESTOREIMAGE);
   virtualFunctionTable[T_Exception] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ExceptionClass] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxNilObject(RESTOREIMAGE);
   virtualFunctionTable[T_NilObject] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxBehaviour(RESTOREIMAGE);
   virtualFunctionTable[T_Behaviour] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxSource(RESTOREIMAGE);
   virtualFunctionTable[T_RexxSource] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) LibraryPackage(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryPackage] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxCode(RESTOREIMAGE);
   virtualFunctionTable[T_RexxCode] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxNativeMethod(RESTOREIMAGE);
   virtualFunctionTable[T_NativeMethod] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxNativeRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_NativeRoutine] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RegisteredRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_RegisteredRoutine] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) CPPCode(RESTOREIMAGE);
   virtualFunctionTable[T_CPPCode] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) AttributeGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeGetterCode] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) AttributeSetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeSetterCode] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) ConstantGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_ConstantGetterCode] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) AbstractCode(RESTOREIMAGE);
   virtualFunctionTable[T_AbstractCode] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxHashTable(RESTOREIMAGE);
   virtualFunctionTable[T_HashTable] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxListTable(RESTOREIMAGE);
   virtualFunctionTable[T_ListTable] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxSmartBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_SmartBuffer] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxVariable(RESTOREIMAGE);
   virtualFunctionTable[T_Variable] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxVariableDictionary(RESTOREIMAGE);
   virtualFunctionTable[T_VariableDictionary] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxParseVariable(RESTOREIMAGE);
   virtualFunctionTable[T_VariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxCompoundVariable(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxStemVariable(RESTOREIMAGE);
   virtualFunctionTable[T_StemVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxDotVariable(RESTOREIMAGE);
   virtualFunctionTable[T_DotVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxVariableReference(RESTOREIMAGE);
   virtualFunctionTable[T_IndirectVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxExpressionFunction(RESTOREIMAGE);
   virtualFunctionTable[T_FunctionCallTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxExpressionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageSendTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxUnaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_UnaryOperatorTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxBinaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_BinaryOperatorTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxExpressionLogical(RESTOREIMAGE);
   virtualFunctionTable[T_LogicalTerm] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstruction(RESTOREIMAGE);
   virtualFunctionTable[T_Instruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionAddress(RESTOREIMAGE);
   virtualFunctionTable[T_AddressInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionAssignment(RESTOREIMAGE);
   virtualFunctionTable[T_AssignmentInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionCall(RESTOREIMAGE);
   virtualFunctionTable[T_CallInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionCommand(RESTOREIMAGE);
   virtualFunctionTable[T_CommandInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionDo(RESTOREIMAGE);
   virtualFunctionTable[T_DoInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionDrop(RESTOREIMAGE);
   virtualFunctionTable[T_DropInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionElse(RESTOREIMAGE);
   virtualFunctionTable[T_ElseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionEnd(RESTOREIMAGE);
   virtualFunctionTable[T_EndInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionEndIf(RESTOREIMAGE);
   virtualFunctionTable[T_EndIfInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionExit(RESTOREIMAGE);
   virtualFunctionTable[T_ExitInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionExpose(RESTOREIMAGE);
   virtualFunctionTable[T_ExposeInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionForward(RESTOREIMAGE);
   virtualFunctionTable[T_ForwardInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionGuard(RESTOREIMAGE);
   virtualFunctionTable[T_GuardInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionIf(RESTOREIMAGE);
   virtualFunctionTable[T_IfInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionInterpret(RESTOREIMAGE);
   virtualFunctionTable[T_InterpretInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionLabel(RESTOREIMAGE);
   virtualFunctionTable[T_LabelInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionLeave(RESTOREIMAGE);
   virtualFunctionTable[T_LeaveInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionNop(RESTOREIMAGE);
   virtualFunctionTable[T_NopInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionNumeric(RESTOREIMAGE);
   virtualFunctionTable[T_NumericInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionOptions(RESTOREIMAGE);
   virtualFunctionTable[T_OptionsInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionOtherwise(RESTOREIMAGE);
   virtualFunctionTable[T_OtherwiseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionParse(RESTOREIMAGE);
   virtualFunctionTable[T_ParseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionProcedure(RESTOREIMAGE);
   virtualFunctionTable[T_ProcedureInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionQueue(RESTOREIMAGE);
   virtualFunctionTable[T_QueueInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionRaise(RESTOREIMAGE);
   virtualFunctionTable[T_RaiseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionReply(RESTOREIMAGE);
   virtualFunctionTable[T_ReplyInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionReturn(RESTOREIMAGE);
   virtualFunctionTable[T_ReturnInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionSay(RESTOREIMAGE);
   virtualFunctionTable[T_SayInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionSelect(RESTOREIMAGE);
   virtualFunctionTable[T_SelectInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionSignal(RESTOREIMAGE);
   virtualFunctionTable[T_SignalInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionThen(RESTOREIMAGE);
   virtualFunctionTable[T_ThenInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionTrace(RESTOREIMAGE);
   virtualFunctionTable[T_TraceInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInstructionUseStrict(RESTOREIMAGE);
   virtualFunctionTable[T_UseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) ClassDirective(RESTOREIMAGE);
   virtualFunctionTable[T_ClassDirective] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) LibraryDirective(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryDirective] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RequiresDirective(RESTOREIMAGE);
   virtualFunctionTable[T_RequiresDirective] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxCompoundElement(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundElement] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxTrigger(RESTOREIMAGE);
   virtualFunctionTable[T_ParseTrigger] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Memory] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxInternalStack(RESTOREIMAGE);
   virtualFunctionTable[T_InternalStack] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxStack(RESTOREIMAGE);
   virtualFunctionTable[T_Stack] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxActivity(RESTOREIMAGE);
   virtualFunctionTable[T_Activity] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxActivation(RESTOREIMAGE);
   virtualFunctionTable[T_Activation] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxNativeActivation(RESTOREIMAGE);
   virtualFunctionTable[T_NativeActivation] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxActivationFrameBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_ActivationFrameBuffer] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxEnvelope(RESTOREIMAGE);
   virtualFunctionTable[T_Envelope] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxClause(RESTOREIMAGE);
   virtualFunctionTable[T_Clause] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxToken(RESTOREIMAGE);
   virtualFunctionTable[T_Token] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) RexxDoBlock(RESTOREIMAGE);
   virtualFunctionTable[T_DoBlock] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) InterpreterInstance(RESTOREIMAGE);
   virtualFunctionTable[T_InterpreterInstance] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) SecurityManager(RESTOREIMAGE);
   virtualFunctionTable[T_SecurityManager] = getVftPointer(objectLoc);
   
   objectPtr = new (objectLoc) CommandHandler(RESTOREIMAGE);
   virtualFunctionTable[T_CommandHandler] = getVftPointer(objectLoc);
   
};


/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */




/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

#include <new>
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
#include "StringTableClass.hpp"
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
#include "SetClass.hpp"
#include "BagClass.hpp"
#include "RexxInfoClass.hpp"
#include "VariableReference.hpp"
#include "EventSemaphore.hpp"
#include "MutexSemaphore.hpp"
#include "RexxBehaviour.hpp"
#include "MethodDictionary.hpp"
#include "LibraryPackage.hpp"
#include "RexxCode.hpp"
#include "NativeCode.hpp"
#include "CPPCode.hpp"
#include "SmartBuffer.hpp"
#include "HashContents.hpp"
#include "ListContents.hpp"
#include "RexxVariable.hpp"
#include "VariableDictionary.hpp"
#include "ExpressionVariable.hpp"
#include "ExpressionCompoundVariable.hpp"
#include "ExpressionStem.hpp"
#include "ExpressionDotVariable.hpp"
#include "IndirectVariableReference.hpp"
#include "ExpressionFunction.hpp"
#include "ExpressionMessage.hpp"
#include "ExpressionOperator.hpp"
#include "ExpressionLogical.hpp"
#include "ExpressionList.hpp"
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
#include "WhenCaseInstruction.hpp"
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
#include "UseInstruction.hpp"
#include "UseLocalInstruction.hpp"
#include "ClassDirective.hpp"
#include "LibraryDirective.hpp"
#include "RequiresDirective.hpp"
#include "CompoundTableElement.hpp"
#include "ParseTrigger.hpp"
#include "ProgramSource.hpp"
#include "NumberArray.hpp"
#include "ExpressionClassResolver.hpp"
#include "ExpressionQualifiedFunction.hpp"
#include "PointerBucket.hpp"
#include "PointerTable.hpp"
#include "SpecialDotVariable.hpp"
#include "VariableReferenceOp.hpp"
#include "UseArgVariableRef.hpp"
#include "CommandIOConfiguration.hpp"
#include "AddressWithInstruction.hpp"
#include "ConstantDirective.hpp"
#include "RexxMemory.hpp"
#include "InternalStack.hpp"
#include "MemoryStack.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "ActivationStack.hpp"
#include "Envelope.hpp"
#include "LanguageParser.hpp"
#include "Clause.hpp"
#include "Token.hpp"
#include "DoBlock.hpp"
#include "InterpreterInstance.hpp"
#include "SecurityManager.hpp"
#include "CommandHandler.hpp"
#include "MapBucket.hpp"
#include "MapTable.hpp"
#include "TrapHandler.hpp"
#include "CommandIOContext.hpp"
#include "OutputRedirector.hpp"
#include "InputRedirector.hpp"


void *MemoryObject::virtualFunctionTable[T_Last_Class_Type + 1] = {NULL};

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

void MemoryObject::buildVirtualFunctionTable()
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
   
   objectPtr = ::new (objectLoc) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Object] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ObjectClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_Class] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ClassClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ArrayClass(RESTOREIMAGE);
   virtualFunctionTable[T_Array] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) DirectoryClass(RESTOREIMAGE);
   virtualFunctionTable[T_Directory] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_DirectoryClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInteger(RESTOREIMAGE);
   virtualFunctionTable[T_Integer] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxIntegerClass(RESTOREIMAGE);
   virtualFunctionTable[T_IntegerClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ListClass(RESTOREIMAGE);
   virtualFunctionTable[T_List] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ListClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MessageClass(RESTOREIMAGE);
   virtualFunctionTable[T_Message] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MessageClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MethodClass(RESTOREIMAGE);
   virtualFunctionTable[T_Method] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MethodClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) NumberString(RESTOREIMAGE);
   virtualFunctionTable[T_NumberString] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_NumberStringClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) QueueClass(RESTOREIMAGE);
   virtualFunctionTable[T_Queue] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_QueueClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StemClass(RESTOREIMAGE);
   virtualFunctionTable[T_Stem] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StemClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxString(RESTOREIMAGE);
   virtualFunctionTable[T_String] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StringClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) SupplierClass(RESTOREIMAGE);
   virtualFunctionTable[T_Supplier] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_SupplierClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) TableClass(RESTOREIMAGE);
   virtualFunctionTable[T_Table] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_TableClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StringTable(RESTOREIMAGE);
   virtualFunctionTable[T_StringTable] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StringTableClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RelationClass(RESTOREIMAGE);
   virtualFunctionTable[T_Relation] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RelationClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MutableBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBuffer] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBufferClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) PointerClass(RESTOREIMAGE);
   virtualFunctionTable[T_Pointer] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PointerClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) BufferClass(RESTOREIMAGE);
   virtualFunctionTable[T_Buffer] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_BufferClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) WeakReference(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReference] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReferenceClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RoutineClass(RESTOREIMAGE);
   virtualFunctionTable[T_Routine] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RoutineClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) PackageClass(RESTOREIMAGE);
   virtualFunctionTable[T_Package] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PackageClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxContext(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContext] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContextClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) IdentityTable(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTable] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTableClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StackFrameClass(RESTOREIMAGE);
   virtualFunctionTable[T_StackFrame] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StackFrameClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) SetClass(RESTOREIMAGE);
   virtualFunctionTable[T_Set] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_SetClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) BagClass(RESTOREIMAGE);
   virtualFunctionTable[T_Bag] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_BagClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInfo(RESTOREIMAGE);
   virtualFunctionTable[T_RexxInfo] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RexxInfoClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) VariableReference(RESTOREIMAGE);
   virtualFunctionTable[T_VariableReference] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_VariableReferenceClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) EventSemaphoreClass(RESTOREIMAGE);
   virtualFunctionTable[T_EventSemaphore] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_EventSemaphoreClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MutexSemaphoreClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutexSemaphore] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutexSemaphoreClass] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxNilObject(RESTOREIMAGE);
   virtualFunctionTable[T_NilObject] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxBehaviour(RESTOREIMAGE);
   virtualFunctionTable[T_Behaviour] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MethodDictionary(RESTOREIMAGE);
   virtualFunctionTable[T_MethodDictionary] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) LibraryPackage(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryPackage] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxCode(RESTOREIMAGE);
   virtualFunctionTable[T_RexxCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) NativeMethod(RESTOREIMAGE);
   virtualFunctionTable[T_NativeMethod] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) NativeRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_NativeRoutine] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RegisteredRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_RegisteredRoutine] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) CPPCode(RESTOREIMAGE);
   virtualFunctionTable[T_CPPCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) AttributeGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeGetterCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) AttributeSetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeSetterCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ConstantGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_ConstantGetterCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) AbstractCode(RESTOREIMAGE);
   virtualFunctionTable[T_AbstractCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) DelegateCode(RESTOREIMAGE);
   virtualFunctionTable[T_DelegateCode] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) SmartBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_SmartBuffer] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) IdentityHashContents(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityHashContents] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) EqualityHashContents(RESTOREIMAGE);
   virtualFunctionTable[T_EqualityHashContents] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MultiValueContents(RESTOREIMAGE);
   virtualFunctionTable[T_MultiValueContents] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StringHashContents(RESTOREIMAGE);
   virtualFunctionTable[T_StringHashContents] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ListContents(RESTOREIMAGE);
   virtualFunctionTable[T_ListContents] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxVariable(RESTOREIMAGE);
   virtualFunctionTable[T_Variable] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) VariableDictionary(RESTOREIMAGE);
   virtualFunctionTable[T_VariableDictionary] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxSimpleVariable(RESTOREIMAGE);
   virtualFunctionTable[T_VariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxCompoundVariable(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxStemVariable(RESTOREIMAGE);
   virtualFunctionTable[T_StemVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxDotVariable(RESTOREIMAGE);
   virtualFunctionTable[T_DotVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxVariableReference(RESTOREIMAGE);
   virtualFunctionTable[T_IndirectVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxExpressionFunction(RESTOREIMAGE);
   virtualFunctionTable[T_FunctionCallTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxExpressionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageSendTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxUnaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_UnaryOperatorTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxBinaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_BinaryOperatorTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxExpressionLogical(RESTOREIMAGE);
   virtualFunctionTable[T_LogicalTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxExpressionList(RESTOREIMAGE);
   virtualFunctionTable[T_ListTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstruction(RESTOREIMAGE);
   virtualFunctionTable[T_Instruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionAddress(RESTOREIMAGE);
   virtualFunctionTable[T_AddressInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionAssignment(RESTOREIMAGE);
   virtualFunctionTable[T_AssignmentInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionCall(RESTOREIMAGE);
   virtualFunctionTable[T_CallInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDynamicCall(RESTOREIMAGE);
   virtualFunctionTable[T_DynamicCallInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionQualifiedCall(RESTOREIMAGE);
   virtualFunctionTable[T_QualifiedCallInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionCallOn(RESTOREIMAGE);
   virtualFunctionTable[T_CallOnInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionCommand(RESTOREIMAGE);
   virtualFunctionTable[T_CommandInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionSimpleDo(RESTOREIMAGE);
   virtualFunctionTable[T_SimpleDoInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoForever(RESTOREIMAGE);
   virtualFunctionTable[T_DoForeverInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoOver(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoOverUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoOverWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoOverFor(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverForInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoOverForUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverForUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoOverForWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverForWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionControlledDo(RESTOREIMAGE);
   virtualFunctionTable[T_ControlledDoInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionControlledDoUntil(RESTOREIMAGE);
   virtualFunctionTable[T_ControlledDoUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionControlledDoWhile(RESTOREIMAGE);
   virtualFunctionTable[T_ControlledDoWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoCount(RESTOREIMAGE);
   virtualFunctionTable[T_DoCountInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoCountUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoCountUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoCountWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoCountWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDrop(RESTOREIMAGE);
   virtualFunctionTable[T_DropInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionElse(RESTOREIMAGE);
   virtualFunctionTable[T_ElseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionEnd(RESTOREIMAGE);
   virtualFunctionTable[T_EndInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionEndIf(RESTOREIMAGE);
   virtualFunctionTable[T_EndIfInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionExit(RESTOREIMAGE);
   virtualFunctionTable[T_ExitInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionExpose(RESTOREIMAGE);
   virtualFunctionTable[T_ExposeInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionForward(RESTOREIMAGE);
   virtualFunctionTable[T_ForwardInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionGuard(RESTOREIMAGE);
   virtualFunctionTable[T_GuardInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionIf(RESTOREIMAGE);
   virtualFunctionTable[T_IfInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionCaseWhen(RESTOREIMAGE);
   virtualFunctionTable[T_CaseWhenInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionInterpret(RESTOREIMAGE);
   virtualFunctionTable[T_InterpretInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionLabel(RESTOREIMAGE);
   virtualFunctionTable[T_LabelInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionLeave(RESTOREIMAGE);
   virtualFunctionTable[T_LeaveInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionNop(RESTOREIMAGE);
   virtualFunctionTable[T_NopInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionNumeric(RESTOREIMAGE);
   virtualFunctionTable[T_NumericInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionOptions(RESTOREIMAGE);
   virtualFunctionTable[T_OptionsInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionOtherwise(RESTOREIMAGE);
   virtualFunctionTable[T_OtherwiseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionParse(RESTOREIMAGE);
   virtualFunctionTable[T_ParseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionProcedure(RESTOREIMAGE);
   virtualFunctionTable[T_ProcedureInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionQueue(RESTOREIMAGE);
   virtualFunctionTable[T_QueueInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionRaise(RESTOREIMAGE);
   virtualFunctionTable[T_RaiseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionReply(RESTOREIMAGE);
   virtualFunctionTable[T_ReplyInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionReturn(RESTOREIMAGE);
   virtualFunctionTable[T_ReturnInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionSay(RESTOREIMAGE);
   virtualFunctionTable[T_SayInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionSelect(RESTOREIMAGE);
   virtualFunctionTable[T_SelectInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionSelectCase(RESTOREIMAGE);
   virtualFunctionTable[T_SelectCaseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionSignal(RESTOREIMAGE);
   virtualFunctionTable[T_SignalInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDynamicSignal(RESTOREIMAGE);
   virtualFunctionTable[T_DynamicSignalInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionSignalOn(RESTOREIMAGE);
   virtualFunctionTable[T_SignalOnInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionThen(RESTOREIMAGE);
   virtualFunctionTable[T_ThenInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionTrace(RESTOREIMAGE);
   virtualFunctionTable[T_TraceInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionUse(RESTOREIMAGE);
   virtualFunctionTable[T_UseInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionUseLocal(RESTOREIMAGE);
   virtualFunctionTable[T_UseLocalInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWith(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWithUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWithWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWithFor(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithForInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWithForUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithForUntilInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionDoWithForWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithForWhileInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ClassDirective(RESTOREIMAGE);
   virtualFunctionTable[T_ClassDirective] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) LibraryDirective(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryDirective] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RequiresDirective(RESTOREIMAGE);
   virtualFunctionTable[T_RequiresDirective] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) CompoundTableElement(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundElement] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ParseTrigger(RESTOREIMAGE);
   virtualFunctionTable[T_ParseTrigger] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_ProgramSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ArrayProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayProgramSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) BufferProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_BufferProgramSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) FileProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_FileProgramSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) NumberArray(RESTOREIMAGE);
   virtualFunctionTable[T_NumberArray] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ClassResolver(RESTOREIMAGE);
   virtualFunctionTable[T_ClassResolver] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) QualifiedFunction(RESTOREIMAGE);
   virtualFunctionTable[T_QualifiedFunction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) PointerBucket(RESTOREIMAGE);
   virtualFunctionTable[T_PointerBucket] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) PointerTable(RESTOREIMAGE);
   virtualFunctionTable[T_PointerTable] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) SpecialDotVariable(RESTOREIMAGE);
   virtualFunctionTable[T_SpecialDotVariableTerm] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) VariableReferenceOp(RESTOREIMAGE);
   virtualFunctionTable[T_VariableReferenceOp] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) UseArgVariableRef(RESTOREIMAGE);
   virtualFunctionTable[T_UseArgVariableRef] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) CommandIOConfiguration(RESTOREIMAGE);
   virtualFunctionTable[T_CommandIOConfiguration] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxInstructionAddressWith(RESTOREIMAGE);
   virtualFunctionTable[T_AddressWithInstruction] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ConstantDirective(RESTOREIMAGE);
   virtualFunctionTable[T_ConstantDirective] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Memory] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) InternalStack(RESTOREIMAGE);
   virtualFunctionTable[T_InternalStack] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) PushThroughStack(RESTOREIMAGE);
   virtualFunctionTable[T_PushThroughStack] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) Activity(RESTOREIMAGE);
   virtualFunctionTable[T_Activity] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxActivation(RESTOREIMAGE);
   virtualFunctionTable[T_Activation] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) NativeActivation(RESTOREIMAGE);
   virtualFunctionTable[T_NativeActivation] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ActivationFrameBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_ActivationFrameBuffer] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) Envelope(RESTOREIMAGE);
   virtualFunctionTable[T_Envelope] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) LanguageParser(RESTOREIMAGE);
   virtualFunctionTable[T_LanguageParser] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxClause(RESTOREIMAGE);
   virtualFunctionTable[T_Clause] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxToken(RESTOREIMAGE);
   virtualFunctionTable[T_Token] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) DoBlock(RESTOREIMAGE);
   virtualFunctionTable[T_DoBlock] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) InterpreterInstance(RESTOREIMAGE);
   virtualFunctionTable[T_InterpreterInstance] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) SecurityManager(RESTOREIMAGE);
   virtualFunctionTable[T_SecurityManager] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) CommandHandler(RESTOREIMAGE);
   virtualFunctionTable[T_CommandHandler] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MapBucket(RESTOREIMAGE);
   virtualFunctionTable[T_MapBucket] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) MapTable(RESTOREIMAGE);
   virtualFunctionTable[T_MapTable] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) TrapHandler(RESTOREIMAGE);
   virtualFunctionTable[T_TrapHandler] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) CommandIOContext(RESTOREIMAGE);
   virtualFunctionTable[T_CommandIOContext] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StemOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_StemOutputTarget] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StreamObjectOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_StreamObjectOutputTarget] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StreamOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_StreamOutputTarget] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) CollectionOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_CollectionOutputTarget] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) BufferingOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_BufferingOutputTarget] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StemInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_StemInputSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StreamObjectInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_StreamObjectInputSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) StreamInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_StreamInputSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) ArrayInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayInputSource] = getVftPointer(objectLoc);
   
   objectPtr = ::new (objectLoc) RexxQueueOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_RexxQueueOutputTarget] = getVftPointer(objectLoc);
   
};


/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */


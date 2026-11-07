

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
    // grab the resulting virtual function table.  A RESTOREIMAGE
    // constructor may write more than the vtable pointer (a member
    // with a non-trivial default constructor does), and the buffer
    // has no check of its own, so each class is checked to fit.
   
   static_assert(sizeof(RexxObject) <= sizeof(objectBuffer), "objectBuffer is too small for RexxObject");
   objectPtr = ::new (objectLoc) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Object] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ObjectClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_Class] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ClassClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ArrayClass) <= sizeof(objectBuffer), "objectBuffer is too small for ArrayClass");
   objectPtr = ::new (objectLoc) ArrayClass(RESTOREIMAGE);
   virtualFunctionTable[T_Array] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(DirectoryClass) <= sizeof(objectBuffer), "objectBuffer is too small for DirectoryClass");
   objectPtr = ::new (objectLoc) DirectoryClass(RESTOREIMAGE);
   virtualFunctionTable[T_Directory] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_DirectoryClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInteger) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInteger");
   objectPtr = ::new (objectLoc) RexxInteger(RESTOREIMAGE);
   virtualFunctionTable[T_Integer] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxIntegerClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxIntegerClass");
   objectPtr = ::new (objectLoc) RexxIntegerClass(RESTOREIMAGE);
   virtualFunctionTable[T_IntegerClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ListClass) <= sizeof(objectBuffer), "objectBuffer is too small for ListClass");
   objectPtr = ::new (objectLoc) ListClass(RESTOREIMAGE);
   virtualFunctionTable[T_List] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_ListClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MessageClass) <= sizeof(objectBuffer), "objectBuffer is too small for MessageClass");
   objectPtr = ::new (objectLoc) MessageClass(RESTOREIMAGE);
   virtualFunctionTable[T_Message] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MessageClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MethodClass) <= sizeof(objectBuffer), "objectBuffer is too small for MethodClass");
   objectPtr = ::new (objectLoc) MethodClass(RESTOREIMAGE);
   virtualFunctionTable[T_Method] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MethodClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(NumberString) <= sizeof(objectBuffer), "objectBuffer is too small for NumberString");
   objectPtr = ::new (objectLoc) NumberString(RESTOREIMAGE);
   virtualFunctionTable[T_NumberString] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_NumberStringClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(QueueClass) <= sizeof(objectBuffer), "objectBuffer is too small for QueueClass");
   objectPtr = ::new (objectLoc) QueueClass(RESTOREIMAGE);
   virtualFunctionTable[T_Queue] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_QueueClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StemClass) <= sizeof(objectBuffer), "objectBuffer is too small for StemClass");
   objectPtr = ::new (objectLoc) StemClass(RESTOREIMAGE);
   virtualFunctionTable[T_Stem] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StemClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxString) <= sizeof(objectBuffer), "objectBuffer is too small for RexxString");
   objectPtr = ::new (objectLoc) RexxString(RESTOREIMAGE);
   virtualFunctionTable[T_String] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StringClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(SupplierClass) <= sizeof(objectBuffer), "objectBuffer is too small for SupplierClass");
   objectPtr = ::new (objectLoc) SupplierClass(RESTOREIMAGE);
   virtualFunctionTable[T_Supplier] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_SupplierClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(TableClass) <= sizeof(objectBuffer), "objectBuffer is too small for TableClass");
   objectPtr = ::new (objectLoc) TableClass(RESTOREIMAGE);
   virtualFunctionTable[T_Table] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_TableClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StringTable) <= sizeof(objectBuffer), "objectBuffer is too small for StringTable");
   objectPtr = ::new (objectLoc) StringTable(RESTOREIMAGE);
   virtualFunctionTable[T_StringTable] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StringTableClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RelationClass) <= sizeof(objectBuffer), "objectBuffer is too small for RelationClass");
   objectPtr = ::new (objectLoc) RelationClass(RESTOREIMAGE);
   virtualFunctionTable[T_Relation] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RelationClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MutableBuffer) <= sizeof(objectBuffer), "objectBuffer is too small for MutableBuffer");
   objectPtr = ::new (objectLoc) MutableBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBuffer] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutableBufferClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(PointerClass) <= sizeof(objectBuffer), "objectBuffer is too small for PointerClass");
   objectPtr = ::new (objectLoc) PointerClass(RESTOREIMAGE);
   virtualFunctionTable[T_Pointer] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PointerClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(BufferClass) <= sizeof(objectBuffer), "objectBuffer is too small for BufferClass");
   objectPtr = ::new (objectLoc) BufferClass(RESTOREIMAGE);
   virtualFunctionTable[T_Buffer] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_BufferClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(WeakReference) <= sizeof(objectBuffer), "objectBuffer is too small for WeakReference");
   objectPtr = ::new (objectLoc) WeakReference(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReference] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_WeakReferenceClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RoutineClass) <= sizeof(objectBuffer), "objectBuffer is too small for RoutineClass");
   objectPtr = ::new (objectLoc) RoutineClass(RESTOREIMAGE);
   virtualFunctionTable[T_Routine] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RoutineClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(PackageClass) <= sizeof(objectBuffer), "objectBuffer is too small for PackageClass");
   objectPtr = ::new (objectLoc) PackageClass(RESTOREIMAGE);
   virtualFunctionTable[T_Package] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_PackageClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxContext) <= sizeof(objectBuffer), "objectBuffer is too small for RexxContext");
   objectPtr = ::new (objectLoc) RexxContext(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContext] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RexxContextClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(IdentityTable) <= sizeof(objectBuffer), "objectBuffer is too small for IdentityTable");
   objectPtr = ::new (objectLoc) IdentityTable(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTable] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityTableClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StackFrameClass) <= sizeof(objectBuffer), "objectBuffer is too small for StackFrameClass");
   objectPtr = ::new (objectLoc) StackFrameClass(RESTOREIMAGE);
   virtualFunctionTable[T_StackFrame] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_StackFrameClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(SetClass) <= sizeof(objectBuffer), "objectBuffer is too small for SetClass");
   objectPtr = ::new (objectLoc) SetClass(RESTOREIMAGE);
   virtualFunctionTable[T_Set] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_SetClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(BagClass) <= sizeof(objectBuffer), "objectBuffer is too small for BagClass");
   objectPtr = ::new (objectLoc) BagClass(RESTOREIMAGE);
   virtualFunctionTable[T_Bag] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_BagClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInfo) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInfo");
   objectPtr = ::new (objectLoc) RexxInfo(RESTOREIMAGE);
   virtualFunctionTable[T_RexxInfo] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_RexxInfoClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(VariableReference) <= sizeof(objectBuffer), "objectBuffer is too small for VariableReference");
   objectPtr = ::new (objectLoc) VariableReference(RESTOREIMAGE);
   virtualFunctionTable[T_VariableReference] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_VariableReferenceClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(EventSemaphoreClass) <= sizeof(objectBuffer), "objectBuffer is too small for EventSemaphoreClass");
   objectPtr = ::new (objectLoc) EventSemaphoreClass(RESTOREIMAGE);
   virtualFunctionTable[T_EventSemaphore] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_EventSemaphoreClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MutexSemaphoreClass) <= sizeof(objectBuffer), "objectBuffer is too small for MutexSemaphoreClass");
   objectPtr = ::new (objectLoc) MutexSemaphoreClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutexSemaphore] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClass) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClass");
   objectPtr = ::new (objectLoc) RexxClass(RESTOREIMAGE);
   virtualFunctionTable[T_MutexSemaphoreClass] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxNilObject) <= sizeof(objectBuffer), "objectBuffer is too small for RexxNilObject");
   objectPtr = ::new (objectLoc) RexxNilObject(RESTOREIMAGE);
   virtualFunctionTable[T_NilObject] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxBehaviour) <= sizeof(objectBuffer), "objectBuffer is too small for RexxBehaviour");
   objectPtr = ::new (objectLoc) RexxBehaviour(RESTOREIMAGE);
   virtualFunctionTable[T_Behaviour] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MethodDictionary) <= sizeof(objectBuffer), "objectBuffer is too small for MethodDictionary");
   objectPtr = ::new (objectLoc) MethodDictionary(RESTOREIMAGE);
   virtualFunctionTable[T_MethodDictionary] = getVftPointer(objectLoc);
   
   static_assert(sizeof(LibraryPackage) <= sizeof(objectBuffer), "objectBuffer is too small for LibraryPackage");
   objectPtr = ::new (objectLoc) LibraryPackage(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryPackage] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxCode) <= sizeof(objectBuffer), "objectBuffer is too small for RexxCode");
   objectPtr = ::new (objectLoc) RexxCode(RESTOREIMAGE);
   virtualFunctionTable[T_RexxCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(NativeMethod) <= sizeof(objectBuffer), "objectBuffer is too small for NativeMethod");
   objectPtr = ::new (objectLoc) NativeMethod(RESTOREIMAGE);
   virtualFunctionTable[T_NativeMethod] = getVftPointer(objectLoc);
   
   static_assert(sizeof(NativeRoutine) <= sizeof(objectBuffer), "objectBuffer is too small for NativeRoutine");
   objectPtr = ::new (objectLoc) NativeRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_NativeRoutine] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RegisteredRoutine) <= sizeof(objectBuffer), "objectBuffer is too small for RegisteredRoutine");
   objectPtr = ::new (objectLoc) RegisteredRoutine(RESTOREIMAGE);
   virtualFunctionTable[T_RegisteredRoutine] = getVftPointer(objectLoc);
   
   static_assert(sizeof(CPPCode) <= sizeof(objectBuffer), "objectBuffer is too small for CPPCode");
   objectPtr = ::new (objectLoc) CPPCode(RESTOREIMAGE);
   virtualFunctionTable[T_CPPCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(AttributeGetterCode) <= sizeof(objectBuffer), "objectBuffer is too small for AttributeGetterCode");
   objectPtr = ::new (objectLoc) AttributeGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeGetterCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(AttributeSetterCode) <= sizeof(objectBuffer), "objectBuffer is too small for AttributeSetterCode");
   objectPtr = ::new (objectLoc) AttributeSetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_AttributeSetterCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ConstantGetterCode) <= sizeof(objectBuffer), "objectBuffer is too small for ConstantGetterCode");
   objectPtr = ::new (objectLoc) ConstantGetterCode(RESTOREIMAGE);
   virtualFunctionTable[T_ConstantGetterCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(AbstractCode) <= sizeof(objectBuffer), "objectBuffer is too small for AbstractCode");
   objectPtr = ::new (objectLoc) AbstractCode(RESTOREIMAGE);
   virtualFunctionTable[T_AbstractCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(DelegateCode) <= sizeof(objectBuffer), "objectBuffer is too small for DelegateCode");
   objectPtr = ::new (objectLoc) DelegateCode(RESTOREIMAGE);
   virtualFunctionTable[T_DelegateCode] = getVftPointer(objectLoc);
   
   static_assert(sizeof(SmartBuffer) <= sizeof(objectBuffer), "objectBuffer is too small for SmartBuffer");
   objectPtr = ::new (objectLoc) SmartBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_SmartBuffer] = getVftPointer(objectLoc);
   
   static_assert(sizeof(IdentityHashContents) <= sizeof(objectBuffer), "objectBuffer is too small for IdentityHashContents");
   objectPtr = ::new (objectLoc) IdentityHashContents(RESTOREIMAGE);
   virtualFunctionTable[T_IdentityHashContents] = getVftPointer(objectLoc);
   
   static_assert(sizeof(EqualityHashContents) <= sizeof(objectBuffer), "objectBuffer is too small for EqualityHashContents");
   objectPtr = ::new (objectLoc) EqualityHashContents(RESTOREIMAGE);
   virtualFunctionTable[T_EqualityHashContents] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MultiValueContents) <= sizeof(objectBuffer), "objectBuffer is too small for MultiValueContents");
   objectPtr = ::new (objectLoc) MultiValueContents(RESTOREIMAGE);
   virtualFunctionTable[T_MultiValueContents] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StringHashContents) <= sizeof(objectBuffer), "objectBuffer is too small for StringHashContents");
   objectPtr = ::new (objectLoc) StringHashContents(RESTOREIMAGE);
   virtualFunctionTable[T_StringHashContents] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ListContents) <= sizeof(objectBuffer), "objectBuffer is too small for ListContents");
   objectPtr = ::new (objectLoc) ListContents(RESTOREIMAGE);
   virtualFunctionTable[T_ListContents] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxVariable) <= sizeof(objectBuffer), "objectBuffer is too small for RexxVariable");
   objectPtr = ::new (objectLoc) RexxVariable(RESTOREIMAGE);
   virtualFunctionTable[T_Variable] = getVftPointer(objectLoc);
   
   static_assert(sizeof(VariableDictionary) <= sizeof(objectBuffer), "objectBuffer is too small for VariableDictionary");
   objectPtr = ::new (objectLoc) VariableDictionary(RESTOREIMAGE);
   virtualFunctionTable[T_VariableDictionary] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxSimpleVariable) <= sizeof(objectBuffer), "objectBuffer is too small for RexxSimpleVariable");
   objectPtr = ::new (objectLoc) RexxSimpleVariable(RESTOREIMAGE);
   virtualFunctionTable[T_VariableTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxCompoundVariable) <= sizeof(objectBuffer), "objectBuffer is too small for RexxCompoundVariable");
   objectPtr = ::new (objectLoc) RexxCompoundVariable(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundVariableTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxStemVariable) <= sizeof(objectBuffer), "objectBuffer is too small for RexxStemVariable");
   objectPtr = ::new (objectLoc) RexxStemVariable(RESTOREIMAGE);
   virtualFunctionTable[T_StemVariableTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxDotVariable) <= sizeof(objectBuffer), "objectBuffer is too small for RexxDotVariable");
   objectPtr = ::new (objectLoc) RexxDotVariable(RESTOREIMAGE);
   virtualFunctionTable[T_DotVariableTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxVariableReference) <= sizeof(objectBuffer), "objectBuffer is too small for RexxVariableReference");
   objectPtr = ::new (objectLoc) RexxVariableReference(RESTOREIMAGE);
   virtualFunctionTable[T_IndirectVariableTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxExpressionFunction) <= sizeof(objectBuffer), "objectBuffer is too small for RexxExpressionFunction");
   objectPtr = ::new (objectLoc) RexxExpressionFunction(RESTOREIMAGE);
   virtualFunctionTable[T_FunctionCallTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxExpressionMessage) <= sizeof(objectBuffer), "objectBuffer is too small for RexxExpressionMessage");
   objectPtr = ::new (objectLoc) RexxExpressionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageSendTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxUnaryOperator) <= sizeof(objectBuffer), "objectBuffer is too small for RexxUnaryOperator");
   objectPtr = ::new (objectLoc) RexxUnaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_UnaryOperatorTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxBinaryOperator) <= sizeof(objectBuffer), "objectBuffer is too small for RexxBinaryOperator");
   objectPtr = ::new (objectLoc) RexxBinaryOperator(RESTOREIMAGE);
   virtualFunctionTable[T_BinaryOperatorTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxExpressionLogical) <= sizeof(objectBuffer), "objectBuffer is too small for RexxExpressionLogical");
   objectPtr = ::new (objectLoc) RexxExpressionLogical(RESTOREIMAGE);
   virtualFunctionTable[T_LogicalTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxExpressionList) <= sizeof(objectBuffer), "objectBuffer is too small for RexxExpressionList");
   objectPtr = ::new (objectLoc) RexxExpressionList(RESTOREIMAGE);
   virtualFunctionTable[T_ListTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstruction) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstruction");
   objectPtr = ::new (objectLoc) RexxInstruction(RESTOREIMAGE);
   virtualFunctionTable[T_Instruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionAddress) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionAddress");
   objectPtr = ::new (objectLoc) RexxInstructionAddress(RESTOREIMAGE);
   virtualFunctionTable[T_AddressInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionAssignment) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionAssignment");
   objectPtr = ::new (objectLoc) RexxInstructionAssignment(RESTOREIMAGE);
   virtualFunctionTable[T_AssignmentInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionCall) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionCall");
   objectPtr = ::new (objectLoc) RexxInstructionCall(RESTOREIMAGE);
   virtualFunctionTable[T_CallInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDynamicCall) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDynamicCall");
   objectPtr = ::new (objectLoc) RexxInstructionDynamicCall(RESTOREIMAGE);
   virtualFunctionTable[T_DynamicCallInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionQualifiedCall) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionQualifiedCall");
   objectPtr = ::new (objectLoc) RexxInstructionQualifiedCall(RESTOREIMAGE);
   virtualFunctionTable[T_QualifiedCallInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionCallOn) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionCallOn");
   objectPtr = ::new (objectLoc) RexxInstructionCallOn(RESTOREIMAGE);
   virtualFunctionTable[T_CallOnInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionCommand) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionCommand");
   objectPtr = ::new (objectLoc) RexxInstructionCommand(RESTOREIMAGE);
   virtualFunctionTable[T_CommandInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionSimpleDo) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionSimpleDo");
   objectPtr = ::new (objectLoc) RexxInstructionSimpleDo(RESTOREIMAGE);
   virtualFunctionTable[T_SimpleDoInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoForever) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoForever");
   objectPtr = ::new (objectLoc) RexxInstructionDoForever(RESTOREIMAGE);
   virtualFunctionTable[T_DoForeverInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoOver) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoOver");
   objectPtr = ::new (objectLoc) RexxInstructionDoOver(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoOverUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoOverUntil");
   objectPtr = ::new (objectLoc) RexxInstructionDoOverUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoOverWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoOverWhile");
   objectPtr = ::new (objectLoc) RexxInstructionDoOverWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoOverFor) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoOverFor");
   objectPtr = ::new (objectLoc) RexxInstructionDoOverFor(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverForInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoOverForUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoOverForUntil");
   objectPtr = ::new (objectLoc) RexxInstructionDoOverForUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverForUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoOverForWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoOverForWhile");
   objectPtr = ::new (objectLoc) RexxInstructionDoOverForWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoOverForWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionControlledDo) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionControlledDo");
   objectPtr = ::new (objectLoc) RexxInstructionControlledDo(RESTOREIMAGE);
   virtualFunctionTable[T_ControlledDoInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionControlledDoUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionControlledDoUntil");
   objectPtr = ::new (objectLoc) RexxInstructionControlledDoUntil(RESTOREIMAGE);
   virtualFunctionTable[T_ControlledDoUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionControlledDoWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionControlledDoWhile");
   objectPtr = ::new (objectLoc) RexxInstructionControlledDoWhile(RESTOREIMAGE);
   virtualFunctionTable[T_ControlledDoWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWhile");
   objectPtr = ::new (objectLoc) RexxInstructionDoWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoUntil");
   objectPtr = ::new (objectLoc) RexxInstructionDoUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoCount) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoCount");
   objectPtr = ::new (objectLoc) RexxInstructionDoCount(RESTOREIMAGE);
   virtualFunctionTable[T_DoCountInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoCountUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoCountUntil");
   objectPtr = ::new (objectLoc) RexxInstructionDoCountUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoCountUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoCountWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoCountWhile");
   objectPtr = ::new (objectLoc) RexxInstructionDoCountWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoCountWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDrop) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDrop");
   objectPtr = ::new (objectLoc) RexxInstructionDrop(RESTOREIMAGE);
   virtualFunctionTable[T_DropInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionElse) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionElse");
   objectPtr = ::new (objectLoc) RexxInstructionElse(RESTOREIMAGE);
   virtualFunctionTable[T_ElseInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionEnd) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionEnd");
   objectPtr = ::new (objectLoc) RexxInstructionEnd(RESTOREIMAGE);
   virtualFunctionTable[T_EndInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionEndIf) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionEndIf");
   objectPtr = ::new (objectLoc) RexxInstructionEndIf(RESTOREIMAGE);
   virtualFunctionTable[T_EndIfInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionExit) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionExit");
   objectPtr = ::new (objectLoc) RexxInstructionExit(RESTOREIMAGE);
   virtualFunctionTable[T_ExitInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionExpose) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionExpose");
   objectPtr = ::new (objectLoc) RexxInstructionExpose(RESTOREIMAGE);
   virtualFunctionTable[T_ExposeInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionForward) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionForward");
   objectPtr = ::new (objectLoc) RexxInstructionForward(RESTOREIMAGE);
   virtualFunctionTable[T_ForwardInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionGuard) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionGuard");
   objectPtr = ::new (objectLoc) RexxInstructionGuard(RESTOREIMAGE);
   virtualFunctionTable[T_GuardInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionIf) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionIf");
   objectPtr = ::new (objectLoc) RexxInstructionIf(RESTOREIMAGE);
   virtualFunctionTable[T_IfInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionCaseWhen) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionCaseWhen");
   objectPtr = ::new (objectLoc) RexxInstructionCaseWhen(RESTOREIMAGE);
   virtualFunctionTable[T_CaseWhenInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionInterpret) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionInterpret");
   objectPtr = ::new (objectLoc) RexxInstructionInterpret(RESTOREIMAGE);
   virtualFunctionTable[T_InterpretInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionLabel) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionLabel");
   objectPtr = ::new (objectLoc) RexxInstructionLabel(RESTOREIMAGE);
   virtualFunctionTable[T_LabelInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionLeave) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionLeave");
   objectPtr = ::new (objectLoc) RexxInstructionLeave(RESTOREIMAGE);
   virtualFunctionTable[T_LeaveInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionMessage) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionMessage");
   objectPtr = ::new (objectLoc) RexxInstructionMessage(RESTOREIMAGE);
   virtualFunctionTable[T_MessageInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionNop) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionNop");
   objectPtr = ::new (objectLoc) RexxInstructionNop(RESTOREIMAGE);
   virtualFunctionTable[T_NopInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionNumeric) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionNumeric");
   objectPtr = ::new (objectLoc) RexxInstructionNumeric(RESTOREIMAGE);
   virtualFunctionTable[T_NumericInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionOptions) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionOptions");
   objectPtr = ::new (objectLoc) RexxInstructionOptions(RESTOREIMAGE);
   virtualFunctionTable[T_OptionsInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionOtherwise) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionOtherwise");
   objectPtr = ::new (objectLoc) RexxInstructionOtherwise(RESTOREIMAGE);
   virtualFunctionTable[T_OtherwiseInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionParse) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionParse");
   objectPtr = ::new (objectLoc) RexxInstructionParse(RESTOREIMAGE);
   virtualFunctionTable[T_ParseInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionProcedure) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionProcedure");
   objectPtr = ::new (objectLoc) RexxInstructionProcedure(RESTOREIMAGE);
   virtualFunctionTable[T_ProcedureInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionQueue) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionQueue");
   objectPtr = ::new (objectLoc) RexxInstructionQueue(RESTOREIMAGE);
   virtualFunctionTable[T_QueueInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionRaise) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionRaise");
   objectPtr = ::new (objectLoc) RexxInstructionRaise(RESTOREIMAGE);
   virtualFunctionTable[T_RaiseInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionReply) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionReply");
   objectPtr = ::new (objectLoc) RexxInstructionReply(RESTOREIMAGE);
   virtualFunctionTable[T_ReplyInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionReturn) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionReturn");
   objectPtr = ::new (objectLoc) RexxInstructionReturn(RESTOREIMAGE);
   virtualFunctionTable[T_ReturnInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionSay) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionSay");
   objectPtr = ::new (objectLoc) RexxInstructionSay(RESTOREIMAGE);
   virtualFunctionTable[T_SayInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionSelect) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionSelect");
   objectPtr = ::new (objectLoc) RexxInstructionSelect(RESTOREIMAGE);
   virtualFunctionTable[T_SelectInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionSelectCase) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionSelectCase");
   objectPtr = ::new (objectLoc) RexxInstructionSelectCase(RESTOREIMAGE);
   virtualFunctionTable[T_SelectCaseInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionSignal) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionSignal");
   objectPtr = ::new (objectLoc) RexxInstructionSignal(RESTOREIMAGE);
   virtualFunctionTable[T_SignalInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDynamicSignal) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDynamicSignal");
   objectPtr = ::new (objectLoc) RexxInstructionDynamicSignal(RESTOREIMAGE);
   virtualFunctionTable[T_DynamicSignalInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionSignalOn) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionSignalOn");
   objectPtr = ::new (objectLoc) RexxInstructionSignalOn(RESTOREIMAGE);
   virtualFunctionTable[T_SignalOnInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionThen) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionThen");
   objectPtr = ::new (objectLoc) RexxInstructionThen(RESTOREIMAGE);
   virtualFunctionTable[T_ThenInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionTrace) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionTrace");
   objectPtr = ::new (objectLoc) RexxInstructionTrace(RESTOREIMAGE);
   virtualFunctionTable[T_TraceInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionUse) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionUse");
   objectPtr = ::new (objectLoc) RexxInstructionUse(RESTOREIMAGE);
   virtualFunctionTable[T_UseInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionUseLocal) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionUseLocal");
   objectPtr = ::new (objectLoc) RexxInstructionUseLocal(RESTOREIMAGE);
   virtualFunctionTable[T_UseLocalInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWith) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWith");
   objectPtr = ::new (objectLoc) RexxInstructionDoWith(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWithUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWithUntil");
   objectPtr = ::new (objectLoc) RexxInstructionDoWithUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWithWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWithWhile");
   objectPtr = ::new (objectLoc) RexxInstructionDoWithWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWithFor) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWithFor");
   objectPtr = ::new (objectLoc) RexxInstructionDoWithFor(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithForInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWithForUntil) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWithForUntil");
   objectPtr = ::new (objectLoc) RexxInstructionDoWithForUntil(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithForUntilInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionDoWithForWhile) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionDoWithForWhile");
   objectPtr = ::new (objectLoc) RexxInstructionDoWithForWhile(RESTOREIMAGE);
   virtualFunctionTable[T_DoWithForWhileInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ClassDirective) <= sizeof(objectBuffer), "objectBuffer is too small for ClassDirective");
   objectPtr = ::new (objectLoc) ClassDirective(RESTOREIMAGE);
   virtualFunctionTable[T_ClassDirective] = getVftPointer(objectLoc);
   
   static_assert(sizeof(LibraryDirective) <= sizeof(objectBuffer), "objectBuffer is too small for LibraryDirective");
   objectPtr = ::new (objectLoc) LibraryDirective(RESTOREIMAGE);
   virtualFunctionTable[T_LibraryDirective] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RequiresDirective) <= sizeof(objectBuffer), "objectBuffer is too small for RequiresDirective");
   objectPtr = ::new (objectLoc) RequiresDirective(RESTOREIMAGE);
   virtualFunctionTable[T_RequiresDirective] = getVftPointer(objectLoc);
   
   static_assert(sizeof(CompoundTableElement) <= sizeof(objectBuffer), "objectBuffer is too small for CompoundTableElement");
   objectPtr = ::new (objectLoc) CompoundTableElement(RESTOREIMAGE);
   virtualFunctionTable[T_CompoundElement] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ParseTrigger) <= sizeof(objectBuffer), "objectBuffer is too small for ParseTrigger");
   objectPtr = ::new (objectLoc) ParseTrigger(RESTOREIMAGE);
   virtualFunctionTable[T_ParseTrigger] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ProgramSource) <= sizeof(objectBuffer), "objectBuffer is too small for ProgramSource");
   objectPtr = ::new (objectLoc) ProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_ProgramSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ArrayProgramSource) <= sizeof(objectBuffer), "objectBuffer is too small for ArrayProgramSource");
   objectPtr = ::new (objectLoc) ArrayProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayProgramSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(BufferProgramSource) <= sizeof(objectBuffer), "objectBuffer is too small for BufferProgramSource");
   objectPtr = ::new (objectLoc) BufferProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_BufferProgramSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(FileProgramSource) <= sizeof(objectBuffer), "objectBuffer is too small for FileProgramSource");
   objectPtr = ::new (objectLoc) FileProgramSource(RESTOREIMAGE);
   virtualFunctionTable[T_FileProgramSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(NumberArray) <= sizeof(objectBuffer), "objectBuffer is too small for NumberArray");
   objectPtr = ::new (objectLoc) NumberArray(RESTOREIMAGE);
   virtualFunctionTable[T_NumberArray] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ClassResolver) <= sizeof(objectBuffer), "objectBuffer is too small for ClassResolver");
   objectPtr = ::new (objectLoc) ClassResolver(RESTOREIMAGE);
   virtualFunctionTable[T_ClassResolver] = getVftPointer(objectLoc);
   
   static_assert(sizeof(QualifiedFunction) <= sizeof(objectBuffer), "objectBuffer is too small for QualifiedFunction");
   objectPtr = ::new (objectLoc) QualifiedFunction(RESTOREIMAGE);
   virtualFunctionTable[T_QualifiedFunction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(PointerBucket) <= sizeof(objectBuffer), "objectBuffer is too small for PointerBucket");
   objectPtr = ::new (objectLoc) PointerBucket(RESTOREIMAGE);
   virtualFunctionTable[T_PointerBucket] = getVftPointer(objectLoc);
   
   static_assert(sizeof(PointerTable) <= sizeof(objectBuffer), "objectBuffer is too small for PointerTable");
   objectPtr = ::new (objectLoc) PointerTable(RESTOREIMAGE);
   virtualFunctionTable[T_PointerTable] = getVftPointer(objectLoc);
   
   static_assert(sizeof(SpecialDotVariable) <= sizeof(objectBuffer), "objectBuffer is too small for SpecialDotVariable");
   objectPtr = ::new (objectLoc) SpecialDotVariable(RESTOREIMAGE);
   virtualFunctionTable[T_SpecialDotVariableTerm] = getVftPointer(objectLoc);
   
   static_assert(sizeof(VariableReferenceOp) <= sizeof(objectBuffer), "objectBuffer is too small for VariableReferenceOp");
   objectPtr = ::new (objectLoc) VariableReferenceOp(RESTOREIMAGE);
   virtualFunctionTable[T_VariableReferenceOp] = getVftPointer(objectLoc);
   
   static_assert(sizeof(UseArgVariableRef) <= sizeof(objectBuffer), "objectBuffer is too small for UseArgVariableRef");
   objectPtr = ::new (objectLoc) UseArgVariableRef(RESTOREIMAGE);
   virtualFunctionTable[T_UseArgVariableRef] = getVftPointer(objectLoc);
   
   static_assert(sizeof(CommandIOConfiguration) <= sizeof(objectBuffer), "objectBuffer is too small for CommandIOConfiguration");
   objectPtr = ::new (objectLoc) CommandIOConfiguration(RESTOREIMAGE);
   virtualFunctionTable[T_CommandIOConfiguration] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxInstructionAddressWith) <= sizeof(objectBuffer), "objectBuffer is too small for RexxInstructionAddressWith");
   objectPtr = ::new (objectLoc) RexxInstructionAddressWith(RESTOREIMAGE);
   virtualFunctionTable[T_AddressWithInstruction] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ConstantDirective) <= sizeof(objectBuffer), "objectBuffer is too small for ConstantDirective");
   objectPtr = ::new (objectLoc) ConstantDirective(RESTOREIMAGE);
   virtualFunctionTable[T_ConstantDirective] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxObject) <= sizeof(objectBuffer), "objectBuffer is too small for RexxObject");
   objectPtr = ::new (objectLoc) RexxObject(RESTOREIMAGE);
   virtualFunctionTable[T_Memory] = getVftPointer(objectLoc);
   
   static_assert(sizeof(InternalStack) <= sizeof(objectBuffer), "objectBuffer is too small for InternalStack");
   objectPtr = ::new (objectLoc) InternalStack(RESTOREIMAGE);
   virtualFunctionTable[T_InternalStack] = getVftPointer(objectLoc);
   
   static_assert(sizeof(PushThroughStack) <= sizeof(objectBuffer), "objectBuffer is too small for PushThroughStack");
   objectPtr = ::new (objectLoc) PushThroughStack(RESTOREIMAGE);
   virtualFunctionTable[T_PushThroughStack] = getVftPointer(objectLoc);
   
   static_assert(sizeof(Activity) <= sizeof(objectBuffer), "objectBuffer is too small for Activity");
   objectPtr = ::new (objectLoc) Activity(RESTOREIMAGE);
   virtualFunctionTable[T_Activity] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxActivation) <= sizeof(objectBuffer), "objectBuffer is too small for RexxActivation");
   objectPtr = ::new (objectLoc) RexxActivation(RESTOREIMAGE);
   virtualFunctionTable[T_Activation] = getVftPointer(objectLoc);
   
   static_assert(sizeof(NativeActivation) <= sizeof(objectBuffer), "objectBuffer is too small for NativeActivation");
   objectPtr = ::new (objectLoc) NativeActivation(RESTOREIMAGE);
   virtualFunctionTable[T_NativeActivation] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ActivationFrameBuffer) <= sizeof(objectBuffer), "objectBuffer is too small for ActivationFrameBuffer");
   objectPtr = ::new (objectLoc) ActivationFrameBuffer(RESTOREIMAGE);
   virtualFunctionTable[T_ActivationFrameBuffer] = getVftPointer(objectLoc);
   
   static_assert(sizeof(Envelope) <= sizeof(objectBuffer), "objectBuffer is too small for Envelope");
   objectPtr = ::new (objectLoc) Envelope(RESTOREIMAGE);
   virtualFunctionTable[T_Envelope] = getVftPointer(objectLoc);
   
   static_assert(sizeof(LanguageParser) <= sizeof(objectBuffer), "objectBuffer is too small for LanguageParser");
   objectPtr = ::new (objectLoc) LanguageParser(RESTOREIMAGE);
   virtualFunctionTable[T_LanguageParser] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxClause) <= sizeof(objectBuffer), "objectBuffer is too small for RexxClause");
   objectPtr = ::new (objectLoc) RexxClause(RESTOREIMAGE);
   virtualFunctionTable[T_Clause] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxToken) <= sizeof(objectBuffer), "objectBuffer is too small for RexxToken");
   objectPtr = ::new (objectLoc) RexxToken(RESTOREIMAGE);
   virtualFunctionTable[T_Token] = getVftPointer(objectLoc);
   
   static_assert(sizeof(DoBlock) <= sizeof(objectBuffer), "objectBuffer is too small for DoBlock");
   objectPtr = ::new (objectLoc) DoBlock(RESTOREIMAGE);
   virtualFunctionTable[T_DoBlock] = getVftPointer(objectLoc);
   
   static_assert(sizeof(InterpreterInstance) <= sizeof(objectBuffer), "objectBuffer is too small for InterpreterInstance");
   objectPtr = ::new (objectLoc) InterpreterInstance(RESTOREIMAGE);
   virtualFunctionTable[T_InterpreterInstance] = getVftPointer(objectLoc);
   
   static_assert(sizeof(SecurityManager) <= sizeof(objectBuffer), "objectBuffer is too small for SecurityManager");
   objectPtr = ::new (objectLoc) SecurityManager(RESTOREIMAGE);
   virtualFunctionTable[T_SecurityManager] = getVftPointer(objectLoc);
   
   static_assert(sizeof(CommandHandler) <= sizeof(objectBuffer), "objectBuffer is too small for CommandHandler");
   objectPtr = ::new (objectLoc) CommandHandler(RESTOREIMAGE);
   virtualFunctionTable[T_CommandHandler] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MapBucket) <= sizeof(objectBuffer), "objectBuffer is too small for MapBucket");
   objectPtr = ::new (objectLoc) MapBucket(RESTOREIMAGE);
   virtualFunctionTable[T_MapBucket] = getVftPointer(objectLoc);
   
   static_assert(sizeof(MapTable) <= sizeof(objectBuffer), "objectBuffer is too small for MapTable");
   objectPtr = ::new (objectLoc) MapTable(RESTOREIMAGE);
   virtualFunctionTable[T_MapTable] = getVftPointer(objectLoc);
   
   static_assert(sizeof(TrapHandler) <= sizeof(objectBuffer), "objectBuffer is too small for TrapHandler");
   objectPtr = ::new (objectLoc) TrapHandler(RESTOREIMAGE);
   virtualFunctionTable[T_TrapHandler] = getVftPointer(objectLoc);
   
   static_assert(sizeof(CommandIOContext) <= sizeof(objectBuffer), "objectBuffer is too small for CommandIOContext");
   objectPtr = ::new (objectLoc) CommandIOContext(RESTOREIMAGE);
   virtualFunctionTable[T_CommandIOContext] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StemOutputTarget) <= sizeof(objectBuffer), "objectBuffer is too small for StemOutputTarget");
   objectPtr = ::new (objectLoc) StemOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_StemOutputTarget] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StreamObjectOutputTarget) <= sizeof(objectBuffer), "objectBuffer is too small for StreamObjectOutputTarget");
   objectPtr = ::new (objectLoc) StreamObjectOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_StreamObjectOutputTarget] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StreamOutputTarget) <= sizeof(objectBuffer), "objectBuffer is too small for StreamOutputTarget");
   objectPtr = ::new (objectLoc) StreamOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_StreamOutputTarget] = getVftPointer(objectLoc);
   
   static_assert(sizeof(CollectionOutputTarget) <= sizeof(objectBuffer), "objectBuffer is too small for CollectionOutputTarget");
   objectPtr = ::new (objectLoc) CollectionOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_CollectionOutputTarget] = getVftPointer(objectLoc);
   
   static_assert(sizeof(BufferingOutputTarget) <= sizeof(objectBuffer), "objectBuffer is too small for BufferingOutputTarget");
   objectPtr = ::new (objectLoc) BufferingOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_BufferingOutputTarget] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StemInputSource) <= sizeof(objectBuffer), "objectBuffer is too small for StemInputSource");
   objectPtr = ::new (objectLoc) StemInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_StemInputSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StreamObjectInputSource) <= sizeof(objectBuffer), "objectBuffer is too small for StreamObjectInputSource");
   objectPtr = ::new (objectLoc) StreamObjectInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_StreamObjectInputSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(StreamInputSource) <= sizeof(objectBuffer), "objectBuffer is too small for StreamInputSource");
   objectPtr = ::new (objectLoc) StreamInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_StreamInputSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(ArrayInputSource) <= sizeof(objectBuffer), "objectBuffer is too small for ArrayInputSource");
   objectPtr = ::new (objectLoc) ArrayInputSource(RESTOREIMAGE);
   virtualFunctionTable[T_ArrayInputSource] = getVftPointer(objectLoc);
   
   static_assert(sizeof(RexxQueueOutputTarget) <= sizeof(objectBuffer), "objectBuffer is too small for RexxQueueOutputTarget");
   objectPtr = ::new (objectLoc) RexxQueueOutputTarget(RESTOREIMAGE);
   virtualFunctionTable[T_RexxQueueOutputTarget] = getVftPointer(objectLoc);
   
};


/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */


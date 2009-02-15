

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
#include "RexxBehaviour.hpp"
#include "StringClass.hpp"
#include "IntegerClass.hpp"
#include "NumberStringClass.hpp"

// the table of primitive behaviours
RexxBehaviour RexxBehaviour::primitiveBehaviours[T_Last_Primitive_Class + 1] =
{
    RexxBehaviour(T_Object, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ObjectClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Class, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ClassClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Array, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ArrayClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Directory, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_DirectoryClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Integer, (PCPPM *)RexxInteger::operatorMethods),
    RexxBehaviour(T_IntegerClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_List, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ListClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Message, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_MessageClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Method, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_MethodClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NumberString, (PCPPM *)RexxNumberString::operatorMethods),
    RexxBehaviour(T_NumberStringClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Queue, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_QueueClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Stem, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_StemClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_String, (PCPPM *)RexxString::operatorMethods),
    RexxBehaviour(T_StringClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Supplier, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_SupplierClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Table, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_TableClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Relation, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RelationClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_MutableBuffer, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_MutableBufferClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Pointer, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_PointerClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Buffer, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_BufferClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_WeakReference, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_WeakReferenceClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Routine, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RoutineClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Package, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_PackageClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RexxContext, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RexxContextClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_IdentityTable, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_IdentityTableClass, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NilObject, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Behaviour, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RexxSource, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_LibraryPackage, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RexxCode, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NativeMethod, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NativeRoutine, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RegisteredRoutine, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_CPPCode, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_AttributeGetterCode, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_AttributeSetterCode, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ConstantGetterCode, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_AbstractCode, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_HashTable, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ListTable, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_SmartBuffer, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Variable, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_VariableDictionary, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_VariableTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_CompoundVariableTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_StemVariableTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_DotVariableTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_IndirectVariableTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_FunctionCallTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_MessageSendTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_UnaryOperatorTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_BinaryOperatorTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_LogicalTerm, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Instruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_AddressInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_AssignmentInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_CallInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_CommandInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_DoInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_DropInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ElseInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_EndInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_EndIfInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ExitInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ExposeInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ForwardInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_GuardInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_IfInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_InterpretInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_LabelInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_LeaveInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_MessageInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NopInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NumericInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_OptionsInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_OtherwiseInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ParseInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ProcedureInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_QueueInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RaiseInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ReplyInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ReturnInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_SayInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_SelectInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_SignalInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ThenInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_TraceInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_UseInstruction, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ClassDirective, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_LibraryDirective, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_RequiresDirective, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_CompoundElement, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ParseTrigger, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Memory, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_InternalStack, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Stack, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Activity, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Activation, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_NativeActivation, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_ActivationFrameBuffer, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Envelope, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Clause, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_Token, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_DoBlock, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_InterpreterInstance, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_SecurityManager, (PCPPM *)RexxObject::operatorMethods),
    RexxBehaviour(T_CommandHandler, (PCPPM *)RexxObject::operatorMethods),
};


/* --            ==================================================        -- */
/* --            DO NOT CHANGE THIS FILE, ALL CHANGES WILL BE LOST!        -- */
/* --            ==================================================        -- */
/* -------------------------------------------------------------------------- */


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
/****************************************************************************/
/* REXX Kernel                                                              */
/*                                                                          */
/* Primitive object declarations                                            */
/****************************************************************************/
                                       /* IMPORTANT NOTE:  The includes in  */
                                       /* this file must be in the same     */
                                       /* order as the class type number    */
                                       /* definitions in RexxCore.H           */
                                       /* Object MUST be included 1st       */
CLASS_EXTERNAL(object, RexxObject)
CLASS_EXTERNAL(object_class, RexxClass)

CLASS_EXTERNAL(class, RexxClass)
CLASS_EXTERNAL(class_class, RexxClass)

CLASS_EXTERNAL(array, RexxArray)
CLASS_EXTERNAL(array_class, RexxClass)

CLASS_EXTERNAL(directory, RexxDirectory)
CLASS_EXTERNAL(directory_class, RexxClass)

CLASS_EXTERNAL(envelope, RexxEnvelope)
CLASS_EXTERNAL(envelope_class, RexxClass)

CLASS_EXTERNAL_STRING(integer, RexxInteger)
CLASS_EXTERNAL(integer_class, RexxIntegerClass)

CLASS_EXTERNAL(list, RexxList)
CLASS_EXTERNAL(list_class, RexxClass)

CLASS_EXTERNAL(message, RexxMessage)
CLASS_EXTERNAL(message_class, RexxClass)

CLASS_EXTERNAL(method,  RexxMethod)
CLASS_EXTERNAL(method_class,  RexxClass)

CLASS_EXTERNAL_STRING(numberstring, RexxNumberString)
CLASS_EXTERNAL(numberstring_class, RexxClass)

CLASS_EXTERNAL(queue, RexxQueue)
CLASS_EXTERNAL(queue_class, RexxClass)

CLASS_EXTERNAL(stem, RexxStem)
CLASS_EXTERNAL(stem_class, RexxClass)

CLASS_EXTERNAL_STRING(string, RexxString)
CLASS_EXTERNAL(string_class, RexxStringClass)

CLASS_EXTERNAL_STRING(somproxy, RexxSOMProxy)
CLASS_EXTERNAL(somproxy_class, RexxSOMProxyClass)

CLASS_EXTERNAL(supplier, RexxSupplier)
CLASS_EXTERNAL(supplier_class, RexxClass)

CLASS_EXTERNAL(table, RexxTable)
CLASS_EXTERNAL(table_class, RexxClass)

CLASS_EXTERNAL(relation, RexxRelation)
CLASS_EXTERNAL(relation_class, RexxClass)
                                       // Avoid nasty constructor problem.
                                       //  Beside don't need this anyway.
CLASS_EXTERNAL(memory, RexxObject)

CLASS_EXTERNAL(mutablebuffer, RexxMutableBuffer) // warning: order important!!!
CLASS_EXTERNAL(mutablebuffer_class, RexxClass)   // RexxMutableBufferClass

/* start of internal only classes */
CLASS_INTERNAL(intstack, RexxInternalStack)

CLASS_INTERNAL(activation, RexxActivation)

CLASS_INTERNAL(activity, RexxActivity)

CLASS_INTERNAL(activity_class, RexxActivityClass)

CLASS_INTERNAL(behaviour, RexxBehaviour)

CLASS_INTERNAL(buffer, RexxBuffer)

CLASS_INTERNAL(corral, RexxObject)

CLASS_INTERNAL(hashtab, RexxHashTable)

CLASS_INTERNAL(listtable, RexxListTable)

CLASS_INTERNAL(rexxmethod, RexxCode)

CLASS_INTERNAL(nmethod, RexxNativeCode)

CLASS_INTERNAL(nmethod_class, RexxNativeCodeClass)

CLASS_INTERNAL(nativeact, RexxNativeActivation)

CLASS_INTERNAL(smartbuffer, RexxSmartBuffer)

CLASS_INTERNAL(sommethod, RexxSOMCode)

CLASS_INTERNAL(stack, RexxStack)

CLASS_INTERNAL(variable, RexxVariable)

CLASS_INTERNAL(vdict, RexxVariableDictionary)

CLASS_INTERNAL(clause, RexxClause)

CLASS_INTERNAL(source, RexxSource)

CLASS_INTERNAL(token, RexxToken)

CLASS_INTERNAL(parse_instruction, RexxInstruction)

CLASS_INTERNAL(parse_address, RexxInstructionAddress)

CLASS_INTERNAL(parse_assignment, RexxInstructionAssignment)

CLASS_INTERNAL(parse_block, RexxDoBlock)

CLASS_INTERNAL(parse_call, RexxInstructionCall)

CLASS_INTERNAL(parse_command, RexxInstructionCommand)

CLASS_INTERNAL(parse_compound, RexxCompoundVariable)

CLASS_INTERNAL(parse_do,RexxInstructionDo)

CLASS_INTERNAL(parse_dot_variable, RexxDotVariable)

CLASS_INTERNAL(parse_drop, RexxInstructionDrop)

CLASS_INTERNAL(parse_else, RexxInstructionElse)

CLASS_INTERNAL(parse_end, RexxInstructionEnd)

CLASS_INTERNAL(parse_endif, RexxInstructionEndIf)

CLASS_INTERNAL(parse_exit, RexxInstructionExit)

CLASS_INTERNAL(parse_expose, RexxInstructionExpose)

CLASS_INTERNAL(parse_forward, RexxInstructionForward)

CLASS_INTERNAL(parse_function, RexxExpressionFunction)

CLASS_INTERNAL(parse_guard, RexxInstructionGuard)

CLASS_INTERNAL(parse_if, RexxInstructionIf)

CLASS_INTERNAL(parse_interpret, RexxInstructionInterpret)

CLASS_INTERNAL(parse_label, RexxInstructionLabel)

CLASS_INTERNAL(parse_leave, RexxInstructionLeave)

CLASS_INTERNAL(parse_message, RexxInstructionMessage)

CLASS_INTERNAL(parse_message_send, RexxExpressionMessage)

CLASS_INTERNAL(parse_nop, RexxInstructionNop)

CLASS_INTERNAL(parse_numeric, RexxInstructionNumeric)

CLASS_INTERNAL(parse_operator, RexxExpressionOperator)

CLASS_INTERNAL(parse_options, RexxInstructionOptions)

CLASS_INTERNAL(parse_otherwise, RexxInstructionOtherWise)

CLASS_INTERNAL(parse_parse, RexxInstructionParse)

CLASS_INTERNAL(parse_procedure, RexxInstructionProcedure)

CLASS_INTERNAL(parse_queue, RexxInstructionQueue)

CLASS_INTERNAL(parse_raise, RexxInstructionRaise)

CLASS_INTERNAL(parse_reply, RexxInstructionReply)

CLASS_INTERNAL(parse_return, RexxInstructionReturn)

CLASS_INTERNAL(parse_say, RexxInstructionSay)

CLASS_INTERNAL(parse_select, RexxInstructionSelect)

CLASS_INTERNAL(parse_signal, RexxInstructionSignal)

CLASS_INTERNAL(parse_stem, RexxStemVariable)

CLASS_INTERNAL(parse_then, RexxInstructionThen)

CLASS_INTERNAL(parse_trace, RexxInstructionTrace)

CLASS_INTERNAL(parse_trigger, RexxTrigger)

CLASS_INTERNAL(parse_use, RexxInstructionUse)

CLASS_INTERNAL(parse_variable, RexxParseVariable)

CLASS_INTERNAL(parse_varref, RexxVariableReference)

CLASS_INTERNAL(compound_element, RexxCompoundElement)

CLASS_INTERNAL(activation_frame_buffer, RexxActivationFrameBuffer)

CLASS_INTERNAL(parse_unary_operator, RexxUnaryOperator)

CLASS_INTERNAL(parse_binary_operator, RexxBinaryOperator)

CLASS_INTERNAL(parse_labeled_select, RexxInstructionLabeledSelect)

CLASS_INTERNAL(parse_logical, RexxExpressionLogical)

CLASS_INTERNAL(parse_use_strict, RexxInstructionUseStrict)

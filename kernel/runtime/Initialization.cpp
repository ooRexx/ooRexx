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
/* REXX Kernel                                                  okinit.c      */
/*                                                                            */
/* Initialization and termination                                             */
/*                                                                            */
/******************************************************************************/
#include <string.h>
#include "RexxCore.h"

 #include "StackClass.hpp"                /* stack is used by memory           */
 #include "RexxHashTable.hpp"                 /* now bring in collection classes   */
 #include "RexxCollection.hpp"                 /* needed for some inlines           */
 #include "TableClass.hpp"
 #include "RexxMemory.hpp"               /* memory next, to get OrefSet       */
 #include "RexxBehaviour.hpp"                /* now behaviours and                */
 #include "ClassClass.hpp"                /* classes, which everything needs   */

 #include "NumberStringClass.hpp"               /* numstr is needed by string        */
 #include "IntegerClass.hpp"                  /* so is integer                     */
 #include "StringClass.hpp"               /* string is also needed early       */
 #include "MutableBufferClass.hpp"        /* A mutable buffer class       */

 #include "ArrayClass.hpp"                /* array is needed early             */
 #include "DirectoryClass.hpp"               /* the other hash table collections  */
 #include "RelationClass.hpp"

 #include "RexxListTable.hpp"               /* now list based classes            */
 #include "ListClass.hpp"
 #include "QueueClass.hpp"

 #include "SupplierClass.hpp"                 /* supplier, used by all collections */

 #include "MethodClass.hpp"               /* method used early                 */
 #include "RexxCode.hpp"                /* now various code classes          */
 #include "RexxNativeMethod.hpp"
 #include "RexxSOMCode.hpp"
 #include "ExpressionStack.hpp"               /* now main driver classes           */
 #include "RexxActivation.hpp"
 #include "RexxActivity.hpp"
 #include "RexxNativeActivation.hpp"
 #include "RexxSOMProxy.hpp"
 #include "RexxEnvelope.hpp"
 #include "MessageClass.hpp"

 #include "RexxCompoundTable.hpp"
 #include "StemClass.hpp"                 /* variable management classes       */
 #include "RexxVariable.hpp"
 #include "RexxCompoundElement.hpp"
 #include "RexxVariableDictionary.hpp"

 #include "RexxBuffer.hpp"
 #include "RexxSmartBuffer.hpp"
 #include "RexxMisc.hpp"

 #include "Token.hpp"               /* translator classes                */
 #include "Clause.hpp"
 #include "SourceFile.hpp"

 #include "ExpressionFunction.hpp"                /* expression terms                  */
 #include "ExpressionMessage.hpp"
 #include "ExpressionOperator.hpp"
 #include "ExpressionLogical.hpp"                /* multi-logical expressions         */

 #include "ExpressionBaseVariable.hpp"                  /* base variable management class    */
 #include "ExpressionCompoundVariable.hpp"
 #include "ExpressionDotVariable.hpp"
 #include "ExpressionVariable.hpp"
 #include "IndirectVariableReference.hpp"
 #include "ExpressionStem.hpp"

 #include "RexxInstruction.hpp"               /* base REXX instruction class       */

 #include "AssignmentInstruction.hpp"               /* keyword <expression> instructions */
 #include "CommandInstruction.hpp"
 #include "ExitInstruction.hpp"
 #include "InterpretInstruction.hpp"
 #include "OptionsInstruction.hpp"
 #include "ReplyInstruction.hpp"
 #include "ReturnInstruction.hpp"
 #include "SayInstruction.hpp"

 #include "AddressInstruction.hpp"                /* other instructions                */
 #include "DropInstruction.hpp"
 #include "ExposeInstruction.hpp"
 #include "ForwardInstruction.hpp"
 #include "GuardInstruction.hpp"
 #include "LabelInstruction.hpp"
 #include "LeaveInstruction.hpp"
 #include "MessageInstruction.hpp"
 #include "NopInstruction.hpp"
 #include "NumericInstruction.hpp"
 #include "ProcedureInstruction.hpp"
 #include "QueueInstruction.hpp"
 #include "RaiseInstruction.hpp"
 #include "TraceInstruction.hpp"
 #include "UseInstruction.hpp"
 #include "UseStrictInstruction.hpp"

 #include "CallInstruction.hpp"                /* call/signal instructions          */
 #include "SignalInstruction.hpp"

 #include "ParseInstruction.hpp"               /* parse instruction and associates  */
 #include "ParseTarget.hpp"
 #include "ParseTrigger.hpp"

 #include "ElseInstruction.hpp"                /* control type instructions         */
 #include "EndIf.hpp"
 #include "IfInstruction.hpp"
 #include "ThenInstruction.hpp"

 #include "DoBlock.hpp"               /* block type instructions           */
 #include "DoInstruction.hpp"
 #include "EndInstruction.hpp"
 #include "OtherwiseInstruction.hpp"
 #include "SelectInstruction.hpp"

/******************************************************************************/
/* Initialisation                                                             */
/******************************************************************************/
RexxString * kernel_name (char* value);
void         REXX_terminate (void);
void         restoreStrings(void);     /* restore "name" strings            */
void         kernelBuildVirtualFunctionTableArray(void);

RexxString * kernel_name (
    char* value)                       /* ASCII-Z string value              */
/******************************************************************************/
/* Function:  Create a common string value during image build and save        */
/******************************************************************************/
{
  RexxString * stringValue;            /* string value                      */
  RexxString * result;                 /* result value                      */

  stringValue = new_cstring(value);    /* get a string object               */
  if (TheGlobalStrings == OREF_NULL)   /* no longer collecting strings?     */
    return stringValue;                /* just return the string            */
                                       /* check the global table first      */
  result = (RexxString *)TheGlobalStrings->at(stringValue);
  if (result == OREF_NULL) {           /* not in the table                  */
                                       /* add this to the table             */
    TheGlobalStrings->put((RexxObject *)stringValue, stringValue);
    result = stringValue;              /* also the final value              */
  }
  return result;                       /* return the string                 */
}

void kernelRestore(void)
/******************************************************************************/
/* Function:  Restore the kernel state from a saved image.                    */
/******************************************************************************/
{
                                       /* go build the VFT Array            */
  kernelBuildVirtualFunctionTableArray();
  memoryRestore();                     /* go restore the save image         */

                                       /* If first one through, generate all*/
  IntegerZero   = new_integer(0L);     /*  static integers we want to use...*/
  IntegerOne    = new_integer(1L);     /* This will allow us to use static  */
  IntegerTwo    = new_integer(2L);     /* integers instead of having to do a*/
  IntegerThree  = new_integer(3L);     /* new_integer evrytime....          */
  IntegerFour   = new_integer(4L);
  IntegerFive   = new_integer(5L);
  IntegerSix    = new_integer(6L);
  IntegerSeven  = new_integer(7L);
  IntegerEight  = new_integer(8L);
  IntegerNine   = new_integer(9L);
  IntegerMinusOne = new_integer(-1);
  restoreStrings();                    /* restore the global strings        */
  nmethod_restore();                   /* fix up native methods             */
  activity_restore();                  /* do activity restores              */
  memoryObject.enableOrefChecks();     /* enable setCheckOrefs...           */
}

void kernelNewProcess(void)
/******************************************************************************/
/* Function:  Restore the kernel state from a saved image.                    */
/******************************************************************************/
{
  memoryNewProcess();                  /* go restore the save image         */

}
#undef CLASS_EXTERNAL
#undef CLASS_INTERNAL
#undef CLASS_EXTERNAL_STRING

                                       /* Following macros will generate    */
                                       /* code to call each class construct */
                                       /* so that the VFT info gets filled  */
                                       /*  NOTE: this code assumes that the */
                                       /*   very 1st thing in a C++ object  */
                                       /*   is its VFT table.  If that      */
                                       /*   assumption proves wrong we will */
                                       /*   need to redo this stuff.        */
#define CLASS_EXTERNAL(b,c)  objectPtr = new (objectPtr) c (RESTOREIMAGE); \
                             VFTArray[T_##b] = *((void **)objectPtr);

#define CLASS_INTERNAL(b,c)         CLASS_EXTERNAL(b,c)
#define CLASS_EXTERNAL_STRING(b,c)  CLASS_EXTERNAL(b,c)

void kernelBuildVirtualFunctionTableArray(void)
/******************************************************************************/
/* Function:  This routine will build an array of the virtualFunctions        */
/*            There will be one for each Class.                               */
/******************************************************************************/
{
  char objectBuffer[256];              /* buffer for each object            */
  void *objectPtr;
                                       /* Get enough memory to call empty   */
                                       /*constructors                       */
                                       /* All that should happen is the VFT */
                                       /*  gets filled in by the constructor*/
                                       /*  but get a little bigger to be on */
                                       /*  the safe side.                   */
  objectPtr = objectBuffer;
                                       /* now we include OKPRIM.H to get all*/
                                       /*  the class, definitions.          */
  #include "PrimitiveClasses.h"
}

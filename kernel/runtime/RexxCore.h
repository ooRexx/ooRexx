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
/* REXX Kernel                                                RexxCore.h      */
/*                                                                            */
/* Global Declarations                                                        */
/******************************************************************************/

/******************************************************************************/
/* Globally required include files                                            */
/******************************************************************************/
#ifndef RexxCore_INCLUDED
#define RexxCore_INCLUDED

/* ANSI C definitions */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* REXX Library definitions */
#include "RexxLibrary.h"

/******************************************************************************/
/* Literal definitions                                                        */
/******************************************************************************/
#include "RexxConstants.hpp"

/******************************************************************************/
/* Kernel Internal Limits                                                     */
/******************************************************************************/

const int MAX_ERROR_NUMBER = 99999;        /* maximum error code number         */
const int MAX_SYMBOL_LENGTH = 250;         /* length of a symbol name           */

/******************************************************************************/
/* Defines for argument error reporting                                       */
/******************************************************************************/

const int ARG_ONE    = 1;
const int ARG_TWO    = 2;
const int ARG_THREE  = 3;
const int ARG_FOUR   = 4;
const int ARG_FIVE   = 5;
const int ARG_SIX    = 6;
const int ARG_SEVEN  = 7;
const int ARG_EIGHT  = 8;
const int ARG_NINE   = 9;
const int ARG_TEN    = 10;

/******************************************************************************/
/* Constants used for trace prefixes                                          */
/******************************************************************************/

enum TracePrefixes {
    TRACE_PREFIX_CLAUSE   ,
    TRACE_PREFIX_ERROR    ,
    TRACE_PREFIX_RESULT   ,
    TRACE_PREFIX_DUMMY    ,
    TRACE_PREFIX_VARIABLE ,
    TRACE_PREFIX_DOTVARIABLE ,
    TRACE_PREFIX_LITERAL  ,
    TRACE_PREFIX_FUNCTION ,
    TRACE_PREFIX_PREFIX   ,
    TRACE_PREFIX_OPERATOR ,
    TRACE_PREFIX_COMPOUND ,
    TRACE_PREFIX_MESSAGE  ,
    TRACE_PREFIX_ARGUMENT ,
};

#define MAX_TRACEBACK_LIST 80      /* 40 messages are displayed */
#define MAX_TRACEBACK_INDENT 20    /* 10 messages are indented */

/******************************************************************************/
/* Constants used for setting trace                                           */
/******************************************************************************/

const char TRACE_ALL           = 'A';
const char TRACE_COMMANDS      = 'C';
const char TRACE_LABELS        = 'L';
const char TRACE_NORMAL        = 'N';
const char TRACE_FAILURES      = 'F';
const char TRACE_ERRORS        = 'E';
const char TRACE_RESULTS       = 'R';
const char TRACE_INTERMEDIATES = 'I';
const char TRACE_OFF           = 'O';
const char TRACE_IGNORE        = '0';

/******************************************************************************/
/* Constants used for setting trace interactive debug                         */
/******************************************************************************/
const int DEBUG_IGNORE      =  0x00;
const int DEBUG_ON          =  0x01;
const int DEBUG_OFF         =  0x02;
const int DEBUG_TOGGLE      =  0x04;

/******************************************************************************/
/* Random number generation constants                                         */
/******************************************************************************/

const long RANDOM_FACTOR = 1664525L;   /* random multiplication factor      */
                                       /* randomize a seed number           */
inline long RANDOMIZE(long seed) { return (seed * RANDOM_FACTOR + 1); }

/* Object Reference Assignment */
#ifndef CHECKOREFS
#define OrefSet(o,r,v) ((o)->isOldSpace() ? memoryObject.setOref((void *)&(r),(RexxObject *)v) : (RexxObject *)(r=v))
#else
#define OrefSet(o,r,v) memoryObject.checkSetOref((RexxObject *)o, (RexxObject **)&(r), (RexxObject *)v, __FILE__, __LINE__)
#endif

/******************************************************************************/
/* Object creation macros                                                     */
/******************************************************************************/

#define new_behaviour(t)                  (new (t) RexxBehaviour)
#define new_buffer(s)                     (new (s) RexxBuffer)
#define new_clause()                      (new RexxClause)
#define new_counter(v)                    (new RexxInteger (v))
#define new_envelope()                    (new RexxEnvelope)
#define new_list()                        (new RexxList)
#define new_queue()                       (new RexxQueue)
#define new_integer(v)                    (TheIntegerClass->newCache(v))
#define new_message(t,m,a)                (new RexxMessage ((RexxObject *)t, (RexxObject *)m, (RexxArray *)a))
#define new_method(i,e,a,c)               (new RexxMethod (i, e, a, c))
#define new_CPPmethod(p,s,c)              (new RexxMethod (p, s, c))
#define new_nmethod(p,l)                  (TheNativeCodeClass->newClass(p, l))
#define new_pointer(p)                    (TheIntegerClass->newCache((LONG)p))
#define new_smartbuffer()                 (new RexxSmartBuffer(1024))
#define new_sizedSmartBuffer(size)        (new RexxSmartBuffer(size))
#define new_stack(s)                      (new(s) RexxStack (s))
#define new_savestack(s,a)                (new(a) RexxSaveStack (s, a))
#define new_instance()                    (TheObjectClass->newObject())
#define new_supplier(c,f)                 (new RexxSupplier (c,f))

#define MCPP   0                       /* C++ method start index            */
#define MSSCPP 0                       /* C++ class method start index      */


typedef struct internalmethodentry {   /* internal method table entry       */
  const char *entryName;               /* internal entry point name         */
  PFN    entryPoint;                   /* method entry point                */
} internalMethodEntry;

// a couple of convience typedefs to make it easier to write code that can be
// moved to the 4.0 codebase.
typedef size_t stringsize_t;
typedef int    wholenumber_t;
typedef size_t arraysize_t;

                                       /* builtin function prototype        */
typedef RexxObject *builtin_func(RexxActivation *, int, RexxExpressionStack *);
typedef builtin_func *pbuiltin;        /* pointer to a builtin function     */

                                       /*  as "overLoading" of hashValue  */


/******************************************************************************/
/* Change EXTERN definition if not already created by GDATA                   */
/******************************************************************************/

#ifndef INITGLOBALPTR                  // if not the global, this is a NOP.
#define INITGLOBALPTR
#endif
#ifndef EXTERN
#define EXTERN extern                  /* turn into external definition     */
#endif

#ifndef EXTERNMEM
#define EXTERNMEM extern               /* turn into external definition     */
#endif

/******************************************************************************/
/* Primitive Method Type Definition Macros                                    */
/******************************************************************************/
                                       /* following two are used by OKINIT  */
                                       /*  to build the VFT Array.          */
#define CLASS_EXTERNAL(b,c)
#define CLASS_INTERNAL(b,c)

#define koper(name) RexxObject *name(RexxObject *);

                                       /* declare a class creation routine  */
                                       /* for classes with their own        */
                                       /* explicit class objects            */
#define CLASS_CREATE(name, id, className) The##name##Class = (className *)new (0, id, The##name##ClassBehaviour, The##name##Behaviour) RexxClass;
#define SUBCLASS_CREATE(c, id, t)         The##c##Class = new (sizeof(t), id, The##c##ClassBehaviour, The##c##Behaviour) t;
                                       /* restore a class from its          */
                                       /* associated primitive behaviour    */
                                       /* (already restored by memory_init) */
#define RESTORE_CLASS(name, location, className) The##name##Class = (className *)RexxBehaviour::getPrimitiveBehaviour(T_##location)->restoreClass();



/******************************************************************************/
/* Global Objects - General                                                   */
/******************************************************************************/
#ifdef SCRIPTING
EXTERN RexxObject* (__stdcall *NovalueCallback)(const char *) INITGLOBALPTR;
#endif

EXTERN RexxClass  * TheArrayClass INITGLOBALPTR;     /* array class                       */
EXTERN RexxClass  * TheClassClass INITGLOBALPTR;     /* class of classes                  */
EXTERN RexxClass  * TheDirectoryClass INITGLOBALPTR; /* directory class                   */
EXTERN RexxDirectory * TheEnvironment INITGLOBALPTR; /* environment object                */

EXTERN RexxDirectory * ThePublicRoutines INITGLOBALPTR; /* public_routines directory                */
EXTERN RexxDirectory * TheStaticRequires INITGLOBALPTR; /* static_requires directory                */

EXTERN MemorySegmentPool *GlobalPoolBase INITGLOBALPTR;

EXTERN RexxDirectory * TheEnvironmentBase INITGLOBALPTR; // environment object base ptr
                                       /* function table                    */
EXTERN RexxDirectory * TheFunctionsDirectory INITGLOBALPTR;
                                       /* integer class                     */
EXTERN RexxIntegerClass  * TheIntegerClass INITGLOBALPTR;
EXTERN RexxDirectory  * TheKernel INITGLOBALPTR;     /* kernel directory                  */
EXTERN RexxListClass  * TheListClass INITGLOBALPTR;  /* list class                        */
EXTERN RexxMemory * TheMemoryObject INITGLOBALPTR;   /* memory object                     */
EXTERNMEM RexxMemory  memoryObject;   /* memory object                     */
EXTERN RexxClass  * TheMessageClass INITGLOBALPTR;   /* message class                     */
                                       /* method class                      */
EXTERN RexxMethodClass  * TheMethodClass INITGLOBALPTR;
                                       /* native method class               */
EXTERN RexxNativeCodeClass  * TheNativeCodeClass INITGLOBALPTR;
EXTERN RexxObject * TheNilObject INITGLOBALPTR;      /* nil object                        */
EXTERN RexxArray  * TheNullArray INITGLOBALPTR;      /* null arg list                     */
                                       /* null pointer object, pointer to   */
                                       /*NULL                               */
EXTERN RexxInteger* TheNullPointer INITGLOBALPTR;
                                       /* NumberString class                */
EXTERN RexxNumberStringClass  * TheNumberStringClass INITGLOBALPTR;
EXTERN RexxClass  * TheObjectClass INITGLOBALPTR;    /* generic object class              */
EXTERN RexxClass  * TheQueueClass INITGLOBALPTR;     /* queue class                       */
                                       /* Predefined variable retrievers    */
EXTERN RexxDirectory * TheCommonRetrievers INITGLOBALPTR;
EXTERN RexxClass  * TheStemClass INITGLOBALPTR;      /* stem class                        */
                                       /* string class                      */
EXTERN RexxStringClass  * TheStringClass INITGLOBALPTR;
                                       /* mutablebuffer class                */
EXTERN RexxClass  * TheMutableBufferClass INITGLOBALPTR;
                                       /* saved array of primitive          */
                                       /*behaviours                         */
EXTERN RexxObject * TheSavedBehaviours INITGLOBALPTR;
EXTERN RexxClass  * TheSupplierClass INITGLOBALPTR;  /* supplier class                    */
EXTERN RexxDirectory * TheSystem INITGLOBALPTR;     /* system directory                  */
EXTERN RexxClass  * TheTableClass INITGLOBALPTR;     /* table class                       */
EXTERN RexxClass  * TheRelationClass INITGLOBALPTR;  /* relation class                    */

EXTERN RexxInteger * TheFalseObject INITGLOBALPTR;   /* false object                      */
EXTERN RexxInteger * TheTrueObject INITGLOBALPTR;    /* true object                       */
EXTERN RexxInteger * IntegerZero INITGLOBALPTR;      /* Static integer 0                  */
EXTERN RexxInteger * IntegerOne INITGLOBALPTR;       /* Static integer 1                  */
EXTERN RexxInteger * IntegerTwo INITGLOBALPTR;       /* Static integer 2                  */
EXTERN RexxInteger * IntegerThree INITGLOBALPTR;     /* Static integer 3                  */
EXTERN RexxInteger * IntegerFour INITGLOBALPTR;      /* Static integer 4                  */
EXTERN RexxInteger * IntegerFive INITGLOBALPTR;      /* Static integer 5                  */
EXTERN RexxInteger * IntegerSix INITGLOBALPTR;       /* Static integer 6                  */
EXTERN RexxInteger * IntegerSeven INITGLOBALPTR;     /* Static integer 7                  */
EXTERN RexxInteger * IntegerEight INITGLOBALPTR;     /* Static integer 8                  */
EXTERN RexxInteger * IntegerNine INITGLOBALPTR;      /* Static integer 9                  */
EXTERN RexxInteger * IntegerMinusOne INITGLOBALPTR;  /* Static integer -1                 */


/******************************************************************************/
/* Primitive Object Types (keep in sync with behaviour_id in RexxBehaviour.c)       */
/******************************************************************************/
                                       /* IMPORTANT NOTE:  The includes in  */
                                       /* PrimitiveClasses.h MUST be included in the */
                                       /* same order as the T_behaviour     */
                                       /* defines created here.  Also,      */
                                       /* the table in behaviour_id in      */
                                       /* RexxBehaviour.c must also have the names*/
                                       /* in exactly the same order         */

                                       /* SECOND IMPORTANT NOTE:  T_        */
                                       /* defines ending in "_class" do not */
                                       /* have their own include files in   */
                                       /* PrimitiveClasses.h, but are rather in the   */
                                       /* corresponding "major" class       */
                                       /* definition file.  For example,    */
                                       /* T_array and T_array_class are both  */
                                       /* covered by including ArrayClass.h     */
#define lowest_T                     0   /* lowest type number */
#define T_object                     0
#define T_object_class               T_object                     + 1
#define T_class                      T_object_class               + 1
#define T_class_class                T_class                      + 1
#define T_array                      T_class_class                + 1
#define T_array_class                T_array                      + 1
#define T_directory                  T_array_class                + 1
#define T_directory_class            T_directory                  + 1
#define T_envelope                   T_directory_class            + 1
#define T_envelope_class             T_envelope                   + 1
#define T_integer                    T_envelope_class             + 1
#define T_integer_class              T_integer                    + 1
#define T_list                       T_integer_class              + 1
#define T_list_class                 T_list                       + 1
#define T_message                    T_list_class                 + 1
#define T_message_class              T_message                    + 1
#define T_method                     T_message_class              + 1
#define T_method_class               T_method                     + 1
#define T_numberstring               T_method_class               + 1
#define T_numberstring_class         T_numberstring               + 1
#define T_queue                      T_numberstring_class         + 1
#define T_queue_class                T_queue                      + 1
#define T_stem                       T_queue_class                + 1
#define T_stem_class                 T_stem                       + 1
#define T_string                     T_stem_class                 + 1
#define T_string_class               T_string                     + 1
#define T_supplier                   T_string_class               + 1
#define T_supplier_class             T_supplier                   + 1
#define T_table                      T_supplier_class             + 1
#define T_table_class                T_table                      + 1
#define T_relation                   T_table_class                + 1
#define T_relation_class             T_relation                   + 1
#define T_memory                     T_relation_class             + 1
#define T_mutablebuffer              T_memory                     + 1
#define T_mutablebuffer_class        T_mutablebuffer              + 1

// nil object is a one-off.  It doesn't have a special class object

                                       /* define to the top point for       */
                                       /* classes of objects that are       */
                                       /* exposed as REXX objects           */

#define highest_exposed_T            T_mutablebuffer_class

#define T_nil_object                 highest_exposed_T            + 1
#define T_intstack                   T_nil_object                 + 1
#define T_activation                 T_intstack                   + 1
#define T_activity                   T_activation                 + 1
#define T_behaviour                  T_activity                   + 1
#define T_buffer                     T_behaviour                  + 1
#define T_corral                     T_buffer                     + 1
#define T_hashtab                    T_corral                     + 1
#define T_listtable                  T_hashtab                    + 1
#define T_rexxmethod                 T_listtable                  + 1
#define T_nmethod                    T_rexxmethod                 + 1
#define T_nmethod_class              T_nmethod                    + 1
#define T_nativeact                  T_nmethod_class              + 1
#define T_smartbuffer                T_nativeact                  + 1
#define T_stack                      T_smartbuffer                + 1
#define T_variable                   T_stack                      + 1
#define T_vdict                      T_variable                   + 1
#define T_clause                     T_vdict                      + 1
#define T_source                     T_clause                     + 1
#define T_token                      T_source                     + 1
#define T_parse_instruction          T_token                      + 1
#define T_parse_address              T_parse_instruction          + 1
#define T_parse_assignment           T_parse_address              + 1
#define T_parse_block                T_parse_assignment           + 1
#define T_parse_call                 T_parse_block                + 1
#define T_parse_command              T_parse_call                 + 1
#define T_parse_compound             T_parse_command              + 1
#define T_parse_do                   T_parse_compound             + 1
#define T_parse_dot_variable         T_parse_do                   + 1
#define T_parse_drop                 T_parse_dot_variable         + 1
#define T_parse_else                 T_parse_drop                 + 1
#define T_parse_end                  T_parse_else                 + 1
#define T_parse_endif                T_parse_end                  + 1
#define T_parse_exit                 T_parse_endif                + 1
#define T_parse_expose               T_parse_exit                 + 1
#define T_parse_forward              T_parse_expose               + 1
#define T_parse_function             T_parse_forward              + 1
#define T_parse_guard                T_parse_function             + 1
#define T_parse_if                   T_parse_guard                + 1
#define T_parse_interpret            T_parse_if                   + 1
#define T_parse_label                T_parse_interpret            + 1
#define T_parse_leave                T_parse_label                + 1
#define T_parse_message              T_parse_leave                + 1
#define T_parse_message_send         T_parse_message              + 1
#define T_parse_nop                  T_parse_message_send         + 1
#define T_parse_numeric              T_parse_nop                  + 1
#define T_parse_operator             T_parse_numeric              + 1
#define T_parse_options              T_parse_operator             + 1
#define T_parse_otherwise            T_parse_options              + 1
#define T_parse_parse                T_parse_otherwise            + 1
#define T_parse_procedure            T_parse_parse                + 1
#define T_parse_queue                T_parse_procedure            + 1
#define T_parse_raise                T_parse_queue                + 1
#define T_parse_reply                T_parse_raise                + 1
#define T_parse_return               T_parse_reply                + 1
#define T_parse_say                  T_parse_return               + 1
#define T_parse_select               T_parse_say                  + 1
#define T_parse_signal               T_parse_select               + 1
#define T_parse_stem                 T_parse_signal               + 1
#define T_parse_then                 T_parse_stem                 + 1
#define T_parse_trace                T_parse_then                 + 1
#define T_parse_trigger              T_parse_trace                + 1
#define T_parse_use                  T_parse_trigger              + 1
#define T_parse_variable             T_parse_use                  + 1
#define T_parse_varref               T_parse_variable             + 1
#define T_compound_element           T_parse_varref               + 1
#define T_activation_frame_buffer    T_compound_element           + 1
#define T_parse_unary_operator       T_activation_frame_buffer    + 1
#define T_parse_binary_operator      T_parse_unary_operator       + 1
#define T_parse_labeled_select       T_parse_binary_operator      + 1
#define T_parse_logical              T_parse_labeled_select       + 1
#define T_parse_use_strict           T_parse_logical              + 1
#define highest_T                    T_parse_use_strict


/******************************************************************************/
/* Define location of objects saved in SaveArray during Saveimage processing  */
/*  and used during restart processing.                                       */
/* Currently only used in OKMEMORY.C                                          */
/******************************************************************************/
#define saveArray_ENV                1
#define saveArray_KERNEL             saveArray_ENV               + 1
#define saveArray_NAME_STRINGS       saveArray_KERNEL            + 1
#define saveArray_TRUE               saveArray_NAME_STRINGS      + 1
#define saveArray_FALSE              saveArray_TRUE              + 1
#define saveArray_NIL                saveArray_FALSE             + 1
#define saveArray_GLOBAL_STRINGS     saveArray_NIL               + 1
#define saveArray_CLASS              saveArray_GLOBAL_STRINGS    + 1
#define saveArray_PBEHAV             saveArray_CLASS             + 1
#define saveArray_NMETHOD            saveArray_PBEHAV            + 1
#define saveArray_NULLA              saveArray_NMETHOD           + 1
#define saveArray_NULLPOINTER        saveArray_NULLA             + 1
#define saveArray_SYSTEM             saveArray_NULLPOINTER       + 1
#define saveArray_FUNCTIONS          saveArray_SYSTEM            + 1
#define saveArray_COMMON_RETRIEVERS  saveArray_FUNCTIONS         + 1
#define saveArray_STATIC_REQ         saveArray_COMMON_RETRIEVERS + 1
#define saveArray_PUBLIC_RTN         saveArray_STATIC_REQ        + 1
#define saveArray_highest            saveArray_PUBLIC_RTN

/******************************************************************************/
/* Global Objects - Primitive Behaviour                                       */
/******************************************************************************/

#define TheActivationBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_activation]))
#define TheActivityBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_activity]))
#define TheArrayBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_array]))
#define TheArrayClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_array_class]))
#define TheBehaviourBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_behaviour]))
#define TheBufferBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_buffer]))
#define TheClassBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_class]))
#define TheClassClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_class_class]))
#define TheCorralBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_corral]))
#define TheDirectoryBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_directory]))
#define TheDirectoryClassBehaviour  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_directory_class]))
#define TheEnvelopeBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_envelope]))
#define TheHashTableBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_hashtab]))
#define TheIntegerBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_integer]))
#define TheIntegerClassBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_integer_class]))
#define TheListBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_list]))
#define TheListClassBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_list_class]))
#define TheListTableBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_listtable]))
#define TheMemoryBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_memory]))
#define TheMessageBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_message]))
#define TheMessageClassBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_message_class]))
#define TheMethodBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_method]))
#define TheMethodClassBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_method_class]))
#define TheNativeCodeBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_nmethod]))
#define TheNativeCodeClassBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_nmethod_class]))
#define TheRexxCodeBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_rexxmethod]))
#define TheNativeActivationBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_nativeact]))
#define TheNumberStringBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_numberstring]))
#define TheNumberStringClassBehaviour  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_numberstring_class]))
#define TheObjectBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_object]))
#define TheObjectClassBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_object_class]))
#define TheQueueBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_queue]))
#define TheQueueClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_queue_class]))
#define TheSmartBufferBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_smartbuffer]))
#define TheStackBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_stack]))
#define TheStemBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_stem]))
#define TheStemClassBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_stem_class]))
#define TheStringBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_string]))
#define TheStringClassBehaviour     ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_string_class]))
#define TheSupplierBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_supplier]))
#define TheSupplierClassBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_supplier_class]))
#define TheTableBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_table]))
#define TheTableClassBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_table_class]))
#define TheRelationBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_relation]))
#define TheRelationClassBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_relation_class]))
#define TheVariableBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_variable]))
#define TheCompoundElementBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_compound_element]))
#define TheVariableDictionaryBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_vdict]))
#define TheMutableBufferBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_mutablebuffer]))
#define TheMutableBufferClassBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_mutablebuffer_class]))

#define TheAddressInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_address]))
#define TheAssignmentInstructionBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_assignment]))
#define TheDoBlockBehaviour                 ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_block]))
#define TheCallInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_call]))
#define TheCommandInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_command]))
#define TheCompoundVariableBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_compound]))
#define TheDoInstructionBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_do]))
#define TheDotVariableBehaviour             ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_dot_variable]))
#define TheDropInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_drop]))
#define TheElseInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_else]))
#define TheEndInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_end]))
#define TheEndIfInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_endif]))
#define TheExitInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_exit]))
#define TheExposeInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_expose]))
#define TheForwardInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_forward]))
#define TheFunctionBehaviour                ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_function]))
#define TheLogicalBehaviour                 ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_logical]))
#define TheGuardInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_guard]))
#define TheIfInstructionBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_if]))
#define TheInstructionBehaviour             ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_instruction]))
#define TheInterpretInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_interpret]))
#define TheLabelInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_label]))
#define TheLeaveInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_leave]))
#define TheMessageInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_message]))
#define TheMessageSendBehaviour             ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_message_send]))
#define TheNopInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_nop]))
#define TheNumericInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_numeric]))
#define TheOperatorBehaviour                ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_operator]))
#define TheUnaryOperatorBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_unary_operator]))
#define TheBinaryOperatorBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_binary_operator]))
#define TheOptionsInstructionBehaviour      ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_options]))
#define TheOtherWiseInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_otherwise]))
#define TheParseInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_parse]))
#define TheProcedureInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_procedure]))
#define TheQueueInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_queue]))
#define TheRaiseInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_raise]))
#define TheReplyInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_reply]))
#define TheReturnInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_return]))
#define TheSayInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_say]))
#define TheSelectInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_select]))
#define TheLabeledSelectInstructionBehaviour ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_labeled_select]))
#define TheSignalInstructionBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_signal]))
#define TheStemVariableBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_stem]))
#define TheThenInstructionBehaviour         ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_then]))
#define TheTraceInstructionBehaviour        ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_trace]))
#define TheParseTriggerBehaviour            ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_trigger]))
#define TheUseInstructionBehaviour          ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_use]))
#define TheUseStrictInstructionBehaviour    ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_use_strict]))
#define TheParseVariableBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_variable]))
#define TheVariableReferenceBehaviour       ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_parse_varref]))
#define TheSourceBehaviour                  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_source]))
#define TheClauseBehaviour                  ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_clause]))
#define TheTokenBehaviour                   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_token]))
#define TheInternalStackBehaviour           ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_intstack]))
#define TheActivationFrameBufferBehaviour   ((RexxBehaviour *)(&RexxBehaviour::primitiveBehaviours[T_activation_frame_buffer]))

/******************************************************************************/
/* Utility Macros                                                             */
/******************************************************************************/

#define RXROUNDUP(n,to)  ((((n)+(to-1))/(to))*to)
#define rounddown(n,to)  (((n)/(to))*to)
#define same_behaviour(oref) ((oref)->behaviour==this->behaviour)

                                       /* current object's behaviour        */
#define THIS_BEHAVIOUR (this->behaviour)
#define PASTE3(a1,a2,a3) a1##a2##a3

#define isOfClass(t,r) (r)->isObjectType(The##t##Behaviour)
#define isOfClassType(t,r) (r)->isObjectType(T_##t)

                                       /* access an object's hash value     */
#define HASHVALUE(r) ((ULONG)((r)->hashvalue))

/******************************************************************************/
/* Utility Functions                                                          */
/******************************************************************************/

void logic_error (const char *desc);
                                       /* do a case insensitive compare     */
int  CaselessCompare(const char *, const char *, size_t);
const char *mempbrk(const char *, const char *, size_t);     /* search for characters             */

                                       /* find an environment symbol        */
#define env_find(s) (TheEnvironment->entry(s))
                                       /* various exception/condition       */
                                       /* reporting routines                */
void missing_argument(LONG position);
int  message_number(RexxString *);
                                       /* verify argument presence          */
#define required_arg(arg, position) if (arg == OREF_NULL) missing_argument(ARG_##position)

/******************************************************************************/
/* Thread constants                                                           */
/******************************************************************************/

#define NO_THREAD       -1

/******************************************************************************/
/* Constant GLobal values (for general use)                                   */
/******************************************************************************/

/* Also in RexxNativeAPI.h */
#ifndef NO_INT
# define NO_INT                0x80000000
#endif
#ifndef NO_LONG
# define NO_LONG               0x80000000
#endif
#ifndef NO_CSTRING
# define NO_CSTRING            NULL
#endif
#define NO_RSTRING       NULL

extern double NO_DOUBLE;

/******************************************************************************/
/* Global Objects - Names                                                     */
/******************************************************************************/
#undef GLOBAL_NAME
#define GLOBAL_NAME(name, value) EXTERN RexxString * OREF_##name INITGLOBALPTR;
#include "GlobalNames.h"

#ifndef GDATA_BUILD_BEHAVIOURS

#include "ObjectClass.hpp"               /* get real definition of Object     */


/******************************************************************************/
/* Method pointer special types                                               */
/******************************************************************************/

 typedef RexxObject *  (RexxObject::*PCPPM0)();
 typedef RexxObject *  (RexxObject::*PCPPM1)(RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM2)(RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM3)(RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM4)(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM5)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM6)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPM7)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
 typedef RexxObject *  (RexxObject::*PCPPMA1)(RexxArray *);
 typedef RexxObject *  (RexxObject::*PCPPMC1)(RexxObject **, size_t);
                                       /* pointer to method function        */
 typedef RexxObject *  (VLAENTRY RexxObject::*PCPPM) (...);
 #define CPPM(n) ((PCPPM)&n)

 #include "TableClass.hpp"
 #include "StackClass.hpp"
 #include "RexxMemory.hpp"               /* memory next, to get OrefSet       */
 #include "RexxBehaviour.hpp"                /* now behaviours and                */
 #include "ClassClass.hpp"                /* classes, which everything needs   */
 #include "RexxEnvelope.hpp"                /* envelope is needed for flattens   */
 #include "RexxActivity.hpp"               /* activity is needed for errors     */
 #include "NumberStringClass.hpp"               /* added to make 'number_digits()'   */

#endif
                                       /* in 'ArrayClass.c' visible            */
/******************************************************************************/
/* Method arguments special codes                                             */
/******************************************************************************/

const size_t A_COUNT   = 127;            /* pass arguments as pointer/count pair */

/******************************************************************************/
/* Return codes                                                               */
/******************************************************************************/

const int RC_OK         = 0;
const int RC_LOGIC_ERROR  = 2;

RexxString *last_msgname (void);       /* last message issued               */
RexxMethod *last_method  (void);       /* last method invoked               */

                                       /* data converstion and validation   */
                                       /* routines                          */
void process_new_args(RexxObject **, size_t, RexxObject ***, size_t *, size_t, RexxObject **, RexxObject **);

const int POSITIVE    = 1;             /* integer must be positive          */
const int NONNEGATIVE = 2;             /* integer must be non-negative      */
const int WHOLE       = 3;             /* integer must be whole             */


#ifndef GDATA

// some very common class tests
inline bool isString(RexxObject *o) { return isOfClass(String, o); }
inline bool isInteger(RexxObject *o) { return isOfClass(Integer, o); }
inline bool isArray(RexxObject *o) { return isOfClass(Array, o); }
inline bool isStem(RexxObject *o) { return isOfClass(Stem, o); }
inline bool isActivation(RexxObject *o) { return isOfClass(Activation, o); }
inline bool isMethod(RexxObject *o) { return isOfClass(Method, o); }
#endif


/* The next macro is specifically for REQUESTing a STRING, since there are    */
/* four primitive classes that are equivalents for strings.  It will trap on  */
/* OREF_NULL. */
#ifndef GDATA
inline RexxString *REQUEST_STRING(RexxObject *object)
{
  return (isOfClass(String, object) ? (RexxString *)object : (object)->requestString());
}
#endif

/* The next routine is specifically for REQUESTing a STRING needed as a method*/
/* argument.  This raises an error if the object cannot be converted to a     */
/* string value.                                                              */
inline RexxString * REQUIRED_STRING(RexxObject *object, LONG position)
{
  if (object == OREF_NULL)             /* missing argument?                 */
    missing_argument(position);        /* raise an error                    */
                                       /* force to a string value           */
  return object->requiredString(position);
}


/* The next macro is specifically for REQUESTing an ARRAY, since there are    */
/* six primitive classes that can produce array equivalents.  It will trap on */
/* OREF_NULL. */
inline RexxArray * REQUEST_ARRAY(RexxObject *obj) { return ((obj)->requestArray()); }

/* The next macro is specifically for REQUESTing an INTEGER,                  */
inline RexxInteger * REQUEST_INTEGER(RexxObject *obj) { return ((obj)->requestInteger(Numerics::DEFAULT_DIGITS));}

/* The next macro is specifically for REQUESTing a LONG value                 */
inline long REQUEST_LONG(RexxObject *obj, int precision) { return ((obj)->requestLong(precision)); }

/* The next macro is specifically for REQUESTing an LONG value                */
inline long REQUIRED_LONG(RexxObject *obj, int precision, int position) { return ((obj)->requiredLong(position, precision)); }

/******************************************************************************/
/* Floating-point conversions                                                 */
/******************************************************************************/

void db2st (double source, char *target);
int  st2db (char *source, int length, double *target);
void ln2db (long source, double *target);
BOOL double2Float(double value, float *newValue);

/******************************************************************************/
/* Version number (okver.c)                                                   */
/******************************************************************************/

RexxString *version_number (void);

/******************************************************************************/
/* Typed method invocation macros                                             */
/******************************************************************************/

inline RexxObject * callOperatorMethod(RexxObject *object, LONG methodOffset, RexxObject *argument) {
                                       /* get the entry point               */
  PCPPM cppEntry = object->behaviour->getOperatorMethod(methodOffset);
                                       /* go issue the method               */
  return (object->*((PCPPM1)cppEntry))(argument);
}

/******************************************************************************/
/* Native method and external interface macros                                */
/******************************************************************************/
                                       /* macros for creating methods that  */
                                       /* are part of the native code       */
                                       /* interface.  These are used to     */
                                       /* create directly callable methods  */
                                       /* that can be called from native    */
                                       /* methods (via the RexxNativeAPI.h macros)   */
#define native0(result, name) result REXXENTRY REXX_##name(REXXOBJECT self)
#define native1(result, name, t1, a1) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1)
#define native2(result, name, t1, a1, t2, a2) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2)
#define native3(result, name, t1, a1, t2, a2, t3, a3) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3)
#define native4(result, name, t1, a1, t2, a2, t3, a3, t4, a4) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4)
#define native5(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define native6(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define native7(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) result REXXENTRY REXX_##name(REXXOBJECT self, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)

#define nativei0(result, name) result REXXENTRY REXX_##name(void)
#define nativei1(result, name, t1, a1) result REXXENTRY REXX_##name(t1 a1)
#define nativei2(result, name, t1, a1, t2, a2) result REXXENTRY REXX_##name(t1 a1, t2 a2)
#define nativei3(result, name, t1, a1, t2, a2, t3, a3) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3)
#define nativei4(result, name, t1, a1, t2, a2, t3, a3, t4, a4) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4)
#define nativei5(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define nativei6(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define nativei7(result, name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, t7, a7) result REXXENTRY REXX_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)

                                       /* native method cleanup             */
#define native_release(value)          ActivityManager::currentActivity->nativeRelease(value)
                                       /* macro for common native entry     */
#define native_entry  ActivityManager::getActivity();
                                       /* value termination routine         */
#define return_oref(value)  return (REXXOBJECT)native_release(value);
                                       /* return for no value returns       */
#define return_void native_release(OREF_NULL); return;
                                       /* return for non-oref values        */
#define return_value(value) native_release(OREF_NULL); return value;
#endif

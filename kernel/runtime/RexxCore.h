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

#include "rexx.h"                 // this is the core to everything

/* ANSI C definitions */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* REXX Library definitions */
#define OREF_NULL NULL                 /* definition of a NULL REXX object  */

#include "RexxPlatformDefinitions.h"

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

const size_t RANDOM_FACTOR = 1664525;   /* random multiplication factor      */
                                       /* randomize a seed number           */
inline size_t RANDOMIZE(size_t seed) { return (seed * RANDOM_FACTOR + 1); }

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
#define new_smartbuffer()                 (new RexxSmartBuffer(1024))
#define new_sizedSmartBuffer(size)        (new RexxSmartBuffer(size))
#define new_stack(s)                      (new(s) RexxStack (s))
#define new_savestack(s,a)                (new(a) RexxSaveStack (s, a))
#define new_instance()                    (TheObjectClass->newObject())
#define new_supplier(c,f)                 (new RexxSupplier (c,f))

#define MCPP   0                       /* C++ method start index            */
#define MSSCPP 0                       /* C++ class method start index      */




class RexxExpressionStack;
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


/******************************************************************************/
/* Global Objects - General                                                   */
/******************************************************************************/
#ifdef SCRIPTING
EXTERN RexxObject* (__stdcall *NovalueCallback)(const char *) INITGLOBALPTR;
#endif

class RexxClass;
class RexxDirectory;
class RexxIntegerClass;
class RexxListClass;
class RexxMethodClass;
class RexxArray;
class RexxNumberStringClass;
class RexxStringClass;
class RexxMemory;

// this one is special, and is truly global.
EXTERNMEM RexxMemory  memoryObject;   /* memory object                     */

// TODO:  make these into statics inside classes.

#define TheArrayClass RexxArray::classInstance
#define TheClassClass RexxClass::classInstance
#define TheDirectoryClass RexxDirectory::classInstance
#define TheIntegerClass RexxInteger::classInstance
#define TheListClass RexxList::classInstance
#define TheMessageClass RexxMessage::classInstance
#define TheMethodClass RexxMethod::classInstance
#define TheNumberStringClass RexxNumberString::classInstance
#define TheObjectClass RexxObject::classInstance
#define TheQueueClass RexxQueue::classInstance
#define TheStemClass RexxStem::classInstance
#define TheStringClass RexxString::classInstance
#define TheMutableBufferClass RexxMutableBuffer::classInstance
#define TheSupplierClass RexxSupplier::classInstance
#define TheTableClass RexxTable::classInstance
#define TheRelationClass RexxRelation::classInstance
#define ThePointerClass RexxPointer::classInstance
#define TheBufferClass RexxBuffer::classInstance
#define TheWeakReferenceClass WeakReference::classInstance

#define TheEnvironment RexxMemory::environment
#define ThePublicRoutines RexxMemory::publicRoutines
#define TheStaticRequires RexxMemory::staticRequires
#define TheFunctionsDirectory RexxMemory::functionsDir
#define TheCommonRetrievers RexxMemory::commonRetrievers
#define TheKernel RexxMemory::kernel
#define TheSystem RexxMemory::system

#define TheNilObject RexxNilObject::nilObject

#define TheNullArray RexxArray::nullArray

#define TheFalseObject RexxInteger::falseObject
#define TheTrueObject RexxInteger::trueObject
#define TheNullPointer RexxPointer::nullPointer

#define IntegerZero RexxInteger::integerZero
#define IntegerOne RexxInteger::integerOne
#define IntegerTwo RexxInteger::integerTwo
#define IntegerThree RexxInteger::integerThree
#define IntegerFour RexxInteger::integerFour
#define IntegerFive RexxInteger::integerFive
#define IntegerSix RexxInteger::integerSix
#define IntegerSeven RexxInteger::integerSeven
#define IntegerEight RexxInteger::integerEight
#define IntegerNine RexxInteger::integerNine
#define IntegerMinusOne RexxInteger::integerMinusOne

#include "ClassTypeCodes.h"



/******************************************************************************/
/* Utility Macros                                                             */
/******************************************************************************/

#define RXROUNDUP(n,to)  ((((n)+(to-1))/(to))*to)
#define rounddown(n,to)  (((n)/(to))*to)

#define isOfClass(t,r) (r)->isObjectType(The##t##Behaviour)
#define isOfClassType(t,r) (r)->isObjectType(T_##t)

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
void missing_argument(int position);
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

#ifndef NO_CSTRING
# define NO_CSTRING            NULL
#endif
#define NO_RSTRING       NULL

// TODO:  Make these a name space

/******************************************************************************/
/* Global Objects - Names                                                     */
/******************************************************************************/
#undef GLOBAL_NAME
#define GLOBAL_NAME(name, value) EXTERN RexxString * OREF_##name INITGLOBALPTR;
#include "GlobalNames.h"

#include "ObjectClass.hpp"               /* get real definition of Object     */

 #include "TableClass.hpp"
 #include "StackClass.hpp"
 #include "RexxMemory.hpp"               /* memory next, to get OrefSet       */
 #include "RexxBehaviour.hpp"                /* now behaviours and                */
 #include "ClassClass.hpp"                /* classes, which everything needs   */
 #include "RexxEnvelope.hpp"                /* envelope is needed for flattens   */
 #include "RexxActivity.hpp"               /* activity is needed for errors     */
 #include "NumberStringClass.hpp"               /* added to make 'number_digits()'   */
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

// TODO:  These belong on an activity basis

RexxString *last_msgname (void);       /* last message issued               */
RexxMethod *last_method  (void);       /* last method invoked               */


// MAKE a static method on ClassClass.
                                       /* data converstion and validation   */
                                       /* routines                          */
void process_new_args(RexxObject **, size_t, RexxObject ***, size_t *, size_t, RexxObject **, RexxObject **);

const int POSITIVE    = 1;             /* integer must be positive          */
const int NONNEGATIVE = 2;             /* integer must be non-negative      */
const int WHOLE       = 3;             /* integer must be whole             */


// some very common class tests
inline bool isString(RexxObject *o) { return isOfClass(String, o); }
inline bool isInteger(RexxObject *o) { return isOfClass(Integer, o); }
inline bool isArray(RexxObject *o) { return isOfClass(Array, o); }
inline bool isStem(RexxObject *o) { return isOfClass(Stem, o); }
inline bool isActivation(RexxObject *o) { return isOfClass(Activation, o); }
inline bool isMethod(RexxObject *o) { return isOfClass(Method, o); }


/* The next macro is specifically for REQUESTing a STRING, since there are    */
/* four primitive classes that are equivalents for strings.  It will trap on  */
/* OREF_NULL. */
inline RexxString *REQUEST_STRING(RexxObject *object)
{
  return (isOfClass(String, object) ? (RexxString *)object : (object)->requestString());
}

/* The next routine is specifically for REQUESTing a STRING needed as a method*/
/* argument.  This raises an error if the object cannot be converted to a     */
/* string value.                                                              */
inline RexxString * REQUIRED_STRING(RexxObject *object, int position)
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

/******************************************************************************/
/* Version number (okver.c)                                                   */
/******************************************************************************/

RexxString *version_number (void);

/******************************************************************************/
/* Typed method invocation macros                                             */
/******************************************************************************/

inline RexxObject * callOperatorMethod(RexxObject *object, size_t methodOffset, RexxObject *argument) {
                                       /* get the entry point               */
  PCPPM cppEntry = object->behaviour->getOperatorMethod(methodOffset);
                                       /* go issue the method               */
  return (object->*((PCPPM1)cppEntry))(argument);
}

#endif

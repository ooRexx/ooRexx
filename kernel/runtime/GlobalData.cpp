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
/* REXX Kernel                                                  okgdata.c     */
/*                                                                            */
/* Global Data                                                                */
/*                                                                            */
/******************************************************************************/
#define GDATA                          /* prevent some RexxCore.h declares    */
#define EXTERN                         /* keep RexxCore.h from using extern   */
// explicitly initialize global variable declares.
#define INITGLOBALDATA = NULL

#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxSOMProxy.hpp"
#include "RexxNativeAPI.h"


                                       /* following tables define quick     */
                                       /* access methods for operator       */
                                       /* methods.  The methods here MUST   */
                                       /* be defined in the same order as   */
                                       /* the operator defines in Token.h*/

PCPPM stringOperatorMethods[] = {      /* table of string operator methods  */
   NULL,                               /* first entry not used              */
   (PCPPM)(PCPPSTR)&RexxString::plus,
   (PCPPM)(PCPPSTR)&RexxString::minus,
   (PCPPM)(PCPPSTR)&RexxString::multiply,
   (PCPPM)(PCPPSTR)&RexxString::divide,
   (PCPPM)(PCPPSTR)&RexxString::integerDivide,
   (PCPPM)(PCPPSTR)&RexxString::remainder,
   (PCPPM)(PCPPSTR)&RexxString::power,
   (PCPPM)(PCPPSTR)&RexxString::concatRexx,
   (PCPPM)(PCPPSTR)&RexxString::concatRexx,
   (PCPPM)(PCPPSTR)&RexxString::concatBlank,
   (PCPPM)(PCPPSTR)&RexxString::equal,
   (PCPPM)(PCPPSTR)&RexxString::notEqual,
   (PCPPM)(PCPPSTR)&RexxString::isGreaterThan,
   (PCPPM)(PCPPSTR)&RexxString::isLessOrEqual,
   (PCPPM)(PCPPSTR)&RexxString::isLessThan,
   (PCPPM)(PCPPSTR)&RexxString::isGreaterOrEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPSTR)&RexxString::isGreaterOrEqual,
   (PCPPM)(PCPPSTR)&RexxString::isLessOrEqual,
   (PCPPM)(PCPPSTR)&RexxString::strictEqual,
   (PCPPM)(PCPPSTR)&RexxString::strictNotEqual,
   (PCPPM)(PCPPSTR)&RexxString::strictGreaterThan,
   (PCPPM)(PCPPSTR)&RexxString::strictLessOrEqual,
   (PCPPM)(PCPPSTR)&RexxString::strictLessThan,
   (PCPPM)(PCPPSTR)&RexxString::strictGreaterOrEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPSTR)&RexxString::strictGreaterOrEqual,
   (PCPPM)(PCPPSTR)&RexxString::strictLessOrEqual,
   (PCPPM)(PCPPSTR)&RexxString::notEqual,
   (PCPPM)(PCPPSTR)&RexxString::notEqual, /* Duplicate entry neccessary        */
   (PCPPM)(PCPPSTR)&RexxString::andOp,
   (PCPPM)(PCPPSTR)&RexxString::orOp,
   (PCPPM)(PCPPSTR)&RexxString::xorOp,
   (PCPPM)(PCPPSTR)&RexxString::operatorNot,
};

                                       /* numberstring operator methods     */
PCPPM numberstringOperatorMethods[] = {
   NULL,                               /* first entry not used              */
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::plus,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::minus,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::multiply,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::divide,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::integerDivide,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::remainder,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::power,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::concat,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::concat, /* Duplicate entry neccessary        */
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::concatBlank,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::equal,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::notEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::isGreaterThan,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::isLessOrEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::isLessThan,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::isGreaterOrEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::isGreaterOrEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::isLessOrEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictNotEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictGreaterThan,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictLessOrEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictLessThan,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictGreaterOrEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictGreaterOrEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::strictLessOrEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::notEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::notEqual,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::andOp,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::orOp,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::xorOp,
   (PCPPM)(PCPPNUMSTR)&RexxNumberString::operatorNot,
};

PCPPM integerOperatorMethods[] = {     /* integer operator methods          */
   NULL,                               /* first entry not used              */
   (PCPPM)(PCPPINT)&RexxInteger::plus,
   (PCPPM)(PCPPINT)&RexxInteger::minus,
   (PCPPM)(PCPPINT)&RexxInteger::multiply,
   (PCPPM)(PCPPINT)&RexxInteger::divide,
   (PCPPM)(PCPPINT)&RexxInteger::integerDivide,
   (PCPPM)(PCPPINT)&RexxInteger::remainder,
   (PCPPM)(PCPPINT)&RexxInteger::power,
   (PCPPM)(PCPPINT)&RexxInteger::concat,
   (PCPPM)(PCPPINT)&RexxInteger::concat, /* Duplicate entry neccessary        */
   (PCPPM)(PCPPINT)&RexxInteger::concatBlank,
   (PCPPM)(PCPPINT)&RexxInteger::equal,
   (PCPPM)(PCPPINT)&RexxInteger::notEqual,
   (PCPPM)(PCPPINT)&RexxInteger::isGreaterThan,
   (PCPPM)(PCPPINT)&RexxInteger::isLessOrEqual,
   (PCPPM)(PCPPINT)&RexxInteger::isLessThan,
   (PCPPM)(PCPPINT)&RexxInteger::isGreaterOrEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPINT)&RexxInteger::isGreaterOrEqual,
   (PCPPM)(PCPPINT)&RexxInteger::isLessOrEqual,
   (PCPPM)(PCPPINT)&RexxInteger::strictEqual,
   (PCPPM)(PCPPINT)&RexxInteger::strictNotEqual,
   (PCPPM)(PCPPINT)&RexxInteger::strictGreaterThan,
   (PCPPM)(PCPPINT)&RexxInteger::strictLessOrEqual,
   (PCPPM)(PCPPINT)&RexxInteger::strictLessThan,
   (PCPPM)(PCPPINT)&RexxInteger::strictGreaterOrEqual,
                                       /* Duplicate entry neccessary        */
   (PCPPM)(PCPPINT)&RexxInteger::strictGreaterOrEqual,
   (PCPPM)(PCPPINT)&RexxInteger::strictLessOrEqual,
   (PCPPM)(PCPPINT)&RexxInteger::notEqual,
   (PCPPM)(PCPPINT)&RexxInteger::notEqual,
   (PCPPM)(PCPPINT)&RexxInteger::andOp,
   (PCPPM)(PCPPINT)&RexxInteger::orOp,
   (PCPPM)(PCPPINT)&RexxInteger::xorOp,
   (PCPPM)(PCPPINT)&RexxInteger::operatorNot,
};

PCPPM somproxyOperatorMethods[] = {    /* object operator methods           */
   NULL,                               /* first entry not used              */
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_plus,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_minus,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_multiply,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_divide,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_integerDivide,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_remainder,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_power,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_concat,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_concat,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_concatBlank,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_equal,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_notEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_isGreaterThan,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_isLessOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_isLessThan,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_isGreaterOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_isGreaterOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_isLessOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictNotEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictGreaterThan,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictLessOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictLessThan,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictGreaterOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictGreaterOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_strictLessOrEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_notEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_notEqual,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_and,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_or,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_xor,
   (PCPPM)(PCPPSOM)&RexxSOMProxy::operator_not,
};


PCPPM objectOperatorMethods[] = {      /* object operator methods           */
   NULL,                               /* first entry not used              */
   (PCPPM)&RexxObject::operator_plus,
   (PCPPM)&RexxObject::operator_minus,
   (PCPPM)&RexxObject::operator_multiply,
   (PCPPM)&RexxObject::operator_divide,
   (PCPPM)&RexxObject::operator_integerDivide,
   (PCPPM)&RexxObject::operator_remainder,
   (PCPPM)&RexxObject::operator_power,
   (PCPPM)&RexxObject::operator_abuttal,
   (PCPPM)&RexxObject::operator_concat,
   (PCPPM)&RexxObject::operator_concatBlank,
   (PCPPM)&RexxObject::operator_equal,
   (PCPPM)&RexxObject::operator_notEqual,
   (PCPPM)&RexxObject::operator_isGreaterThan,
   (PCPPM)&RexxObject::operator_isBackslashGreaterThan,
   (PCPPM)&RexxObject::operator_isLessThan,
   (PCPPM)&RexxObject::operator_isBackslashLessThan,
   (PCPPM)&RexxObject::operator_isGreaterOrEqual,
   (PCPPM)&RexxObject::operator_isLessOrEqual,
   (PCPPM)&RexxObject::operator_strictEqual,
   (PCPPM)&RexxObject::operator_strictNotEqual,
   (PCPPM)&RexxObject::operator_strictGreaterThan,
   (PCPPM)&RexxObject::operator_strictBackslashGreaterThan,
   (PCPPM)&RexxObject::operator_strictLessThan,
   (PCPPM)&RexxObject::operator_strictBackslashLessThan,
   (PCPPM)&RexxObject::operator_strictGreaterOrEqual,
   (PCPPM)&RexxObject::operator_strictLessOrEqual,
   (PCPPM)&RexxObject::operator_lessThanGreaterThan,
   (PCPPM)&RexxObject::operator_greaterThanLessThan,
   (PCPPM)&RexxObject::operator_and,
   (PCPPM)&RexxObject::operator_or,
   (PCPPM)&RexxObject::operator_xor,
   (PCPPM)&RexxObject::operator_not,
};

                                       /* undefine CLASS definition macros  */
#undef CLASS_INTERNAL
#undef CLASS_EXTERNAL
#undef CLASS_EXTERNAL_STRING
                                       /* redefine to create a behaviour    */
#define CLASS_INTERNAL(n, t) RexxBehaviour((HEADINFO)RXROUNDUP(sizeof(RexxBehaviour), 4)<<ObjectSizeShift, (short)T_##n, (PCPPM *)objectOperatorMethods),
#define CLASS_EXTERNAL(n, t) RexxBehaviour((HEADINFO)RXROUNDUP(sizeof(RexxBehaviour), 4)<<ObjectSizeShift, (short)T_##n, (PCPPM *)objectOperatorMethods),
#define CLASS_EXTERNAL_STRING(n, t) RexxBehaviour((HEADINFO)RXROUNDUP(sizeof(RexxBehaviour), 4)<<ObjectSizeShift, (short)T_##n, (PCPPM *)n##OperatorMethods),

RexxBehaviour pbehav[highest_T + 1] = {/* table of primitive behaviours     */
#include "PrimitiveClasses.h"          /* generate table from header        */
};
                                       /* an initial value to force it to   */
                                       /* all zeros, which is a non-valid   */
                                       /* float number                      */
double NO_DOUBLE;                      /* non-exsistent double value        */
/* MHES
int  NO_INT  = 0x80000000;
long NO_LONG = 0x80000000;
PCHAR NO_CSTRING = NULL;
*/

/* Array for valid whole number at various digits settings */
/*  for value 1-8.                                         */
extern long validMaxWhole[] = {10,
                               100,
                               1000,
                               10000,
                               100000,
                               1000000,
                               10000000,
                               100000000,
                               1000000000};



MemorySegmentPool *GlobalCurrentPool = NULL;   //wge NULL

SysSharedSemaphoreDefn                 /* semaphore definitions             */
                                       /* defined in xxxdef.h               */

INT   rexx_waiting_activity_count = 0; /* number of waiting activities      */

void *VFTArray[highest_T + 1] = {NULL};   //wge NULL
                                       /* Most currently allocated memoryPoo*/


extern "C" {

#define INTERNAL_METHOD(name) extern "C" char * REXXENTRY name(void **);

#include "NativeMethods.h"             /* bring in the internal list        */

#undef  INTERNAL_METHOD
#define INTERNAL_METHOD(name) #name , (PFN)name,

internalMethodEntry internalMethodTable[] = {
#include "NativeMethods.h"             /* bring in the internal method table*/
   NULL, NULL                          /* final empty entry                 */
};

}

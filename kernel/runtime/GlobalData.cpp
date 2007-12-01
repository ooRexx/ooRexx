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
#include "RexxNativeAPI.h"


                                       /* following tables define quick     */
                                       /* access methods for operator       */
                                       /* methods.  The methods here MUST   */
                                       /* be defined in the same order as   */
                                       /* the operator defines in Token.h*/

PCPPM stringOperatorMethods[] = {      /* table of string operator methods  */
   NULL,                               /* first entry not used              */
   (PCPPM)&RexxString::plus,
   (PCPPM)&RexxString::minus,
   (PCPPM)&RexxString::multiply,
   (PCPPM)&RexxString::divide,
   (PCPPM)&RexxString::integerDivide,
   (PCPPM)&RexxString::remainder,
   (PCPPM)&RexxString::power,
   (PCPPM)&RexxString::concatRexx,
   (PCPPM)&RexxString::concatRexx,
   (PCPPM)&RexxString::concatBlank,
   (PCPPM)&RexxString::equal,
   (PCPPM)&RexxString::notEqual,
   (PCPPM)&RexxString::isGreaterThan,
   (PCPPM)&RexxString::isLessOrEqual,
   (PCPPM)&RexxString::isLessThan,
   (PCPPM)&RexxString::isGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxString::isGreaterOrEqual,
   (PCPPM)&RexxString::isLessOrEqual,
   (PCPPM)&RexxString::strictEqual,
   (PCPPM)&RexxString::strictNotEqual,
   (PCPPM)&RexxString::strictGreaterThan,
   (PCPPM)&RexxString::strictLessOrEqual,
   (PCPPM)&RexxString::strictLessThan,
   (PCPPM)&RexxString::strictGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxString::strictGreaterOrEqual,
   (PCPPM)&RexxString::strictLessOrEqual,
   (PCPPM)&RexxString::notEqual,
   (PCPPM)&RexxString::notEqual, /* Duplicate entry neccessary        */
   (PCPPM)&RexxString::andOp,
   (PCPPM)&RexxString::orOp,
   (PCPPM)&RexxString::xorOp,
   (PCPPM)&RexxString::operatorNot,
};

                                       /* numberstring operator methods     */
PCPPM numberstringOperatorMethods[] = {
   NULL,                               /* first entry not used              */
   (PCPPM)&RexxNumberString::plus,
   (PCPPM)&RexxNumberString::minus,
   (PCPPM)&RexxNumberString::multiply,
   (PCPPM)&RexxNumberString::divide,
   (PCPPM)&RexxNumberString::integerDivide,
   (PCPPM)&RexxNumberString::remainder,
   (PCPPM)&RexxNumberString::power,
   (PCPPM)&RexxNumberString::concat,
   (PCPPM)&RexxNumberString::concat, /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::concatBlank,
   (PCPPM)&RexxNumberString::equal,
   (PCPPM)&RexxNumberString::notEqual,
   (PCPPM)&RexxNumberString::isGreaterThan,
   (PCPPM)&RexxNumberString::isLessOrEqual,
   (PCPPM)&RexxNumberString::isLessThan,
   (PCPPM)&RexxNumberString::isGreaterOrEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::isGreaterOrEqual,
   (PCPPM)&RexxNumberString::isLessOrEqual,
   (PCPPM)&RexxNumberString::strictEqual,
   (PCPPM)&RexxNumberString::strictNotEqual,
   (PCPPM)&RexxNumberString::strictGreaterThan,
   (PCPPM)&RexxNumberString::strictLessOrEqual,
   (PCPPM)&RexxNumberString::strictLessThan,
   (PCPPM)&RexxNumberString::strictGreaterOrEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::strictGreaterOrEqual,
   (PCPPM)&RexxNumberString::strictLessOrEqual,
   (PCPPM)&RexxNumberString::notEqual,
                           /* Duplicate entry neccessary        */
   (PCPPM)&RexxNumberString::notEqual,
   (PCPPM)&RexxNumberString::andOp,
   (PCPPM)&RexxNumberString::orOp,
   (PCPPM)&RexxNumberString::xorOp,
   (PCPPM)&RexxNumberString::operatorNot,
};

PCPPM integerOperatorMethods[] = {     /* integer operator methods          */
   NULL,                               /* first entry not used              */
   (PCPPM)&RexxInteger::plus,
   (PCPPM)&RexxInteger::minus,
   (PCPPM)&RexxInteger::multiply,
   (PCPPM)&RexxInteger::divide,
   (PCPPM)&RexxInteger::integerDivide,
   (PCPPM)&RexxInteger::remainder,
   (PCPPM)&RexxInteger::power,
   (PCPPM)&RexxInteger::concat,
   (PCPPM)&RexxInteger::concat, /* Duplicate entry neccessary        */
   (PCPPM)&RexxInteger::concatBlank,
   (PCPPM)&RexxInteger::equal,
   (PCPPM)&RexxInteger::notEqual,
   (PCPPM)&RexxInteger::isGreaterThan,
   (PCPPM)&RexxInteger::isLessOrEqual,
   (PCPPM)&RexxInteger::isLessThan,
   (PCPPM)&RexxInteger::isGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxInteger::isGreaterOrEqual,
   (PCPPM)&RexxInteger::isLessOrEqual,
   (PCPPM)&RexxInteger::strictEqual,
   (PCPPM)&RexxInteger::strictNotEqual,
   (PCPPM)&RexxInteger::strictGreaterThan,
   (PCPPM)&RexxInteger::strictLessOrEqual,
   (PCPPM)&RexxInteger::strictLessThan,
   (PCPPM)&RexxInteger::strictGreaterOrEqual,
                              /* Duplicate entry neccessary        */
   (PCPPM)&RexxInteger::strictGreaterOrEqual,
   (PCPPM)&RexxInteger::strictLessOrEqual,
   (PCPPM)&RexxInteger::notEqual,
   (PCPPM)&RexxInteger::notEqual,
   (PCPPM)&RexxInteger::andOp,
   (PCPPM)&RexxInteger::orOp,
   (PCPPM)&RexxInteger::xorOp,
   (PCPPM)&RexxInteger::operatorNot,
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
#define CLASS_INTERNAL(n, t) RexxBehaviour(T_##n, (PCPPM *)objectOperatorMethods),
#define CLASS_EXTERNAL(n, t) RexxBehaviour(T_##n, (PCPPM *)objectOperatorMethods),
#define CLASS_EXTERNAL_STRING(n, t) RexxBehaviour(T_##n, (PCPPM *)n##OperatorMethods),

RexxBehaviour RexxBehaviour::primitiveBehaviours[highest_T + 1] = {/* table of primitive behaviours     */
#include "PrimitiveClasses.h"          /* generate table from header        */
};
                                       /* an initial value to force it to   */
                                       /* all zeros, which is a non-valid   */
                                       /* float number                      */
double NO_DOUBLE;                      /* non-exsistent double value        */

SysSharedSemaphoreDefn                 /* semaphore definitions             */
                                       /* defined in xxxdef.h               */

int   rexx_waiting_activity_count = 0; /* number of waiting activities      */


extern "C" {

#define INTERNAL_METHOD(name) extern "C" char * REXXENTRY name(void **);

#include "NativeMethods.h"             /* bring in the internal list        */

#undef  INTERNAL_METHOD
#define INTERNAL_METHOD(name) {#name , (PFN)name},

internalMethodEntry internalMethodTable[] = {
#include "NativeMethods.h"             /* bring in the internal method table*/
   {NULL, NULL}                        /* final empty entry                 */
};

}

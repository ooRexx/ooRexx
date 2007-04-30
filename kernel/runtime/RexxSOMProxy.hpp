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
/* REXX Kernel                                                  RexxSOMProxy.hpp    */
/*                                                                            */
/* Primitive SOM Proxy Class Definitions                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxSOMProxy
#define Included_RexxSOMProxy

#ifdef SOM
#include "somobj.xh"
#endif

void somproxy_create (void);
void somproxy_setup (void);
                                       /* Following defined such that a     */
                                       /* A test for DLFObject is true for  */
                                       /* BasicTypes and Numbers in 1 test  */
                                       /* Low order bits.                   */
                                       /*    1111 1111 DLF Object           */
                                       /*    0000 0011 DLF BasicType        */
                                       /*    0000 0001 DLF Number           */
#define flagDLFObject  0x00000001
#define flagDLFBasic   0x00000003
#define flagDLFNumber  0x00000007
#define flagDSOMProxy  0x10000000

#define isDLFNumber(o) ((flagDLFNumber & ((RexxSOMProxy *)o)->proxyFlags) == flagDLFNumber)
#define isDLFBasic(o)  ((flagDLFBasic  & ((RexxSOMProxy *)o)->proxyFlags) == flagDLFBasic )
#define isDLFObject(o) ((flagDLFObject & ((RexxSOMProxy *)o)->proxyFlags) == flagDLFObject)
#define isDSOMProxy(o)  (flagDSOMProxy & ((RexxSOMProxy *)o)->proxyFlags)


class RexxSOMProxy : public RexxObject {
 public:
  inline RexxSOMProxy(RESTORETYPE restoreType) { ; };
  inline RexxSOMProxy() {;};
  void *operator new(size_t size, long size1, RexxBehaviour *classBehave, RexxBehaviour *instance) { return new (size, classBehave, instance) RexxClass; }
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  void *operator new(size_t);
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);

  RexxString  *stringValue();
  RexxString  *makeString();
  long         longValue(long);
  double       doubleValue();
  RexxNumberString *numberString();
  RexxInteger *integer(RexxInteger *);
  RexxString  *defaultName();

  RexxObject  *unknown(RexxString *, RexxArray *);
  RexxObject  *initProxy(RexxInteger *);
  RexxObject  *freeSOMObj();
  RexxObject  *server();
  RexxObject  *SOMObj();
  RexxInteger *hasMethod(RexxString *);
  void        *realSOMObject();

 // Define operator methods here.

   koper  (operator_plus)
   koper  (operator_minus)
   koper  (operator_multiply)
   koper  (operator_divide)
   koper  (operator_integerDivide)
   koper  (operator_remainder)
   koper  (operator_power)
   koper  (operator_abuttal)
   koper  (operator_concat)
   koper  (operator_concatBlank)
   koper  (operator_equal)
   koper  (operator_notEqual)
   koper  (operator_isGreaterThan)
   koper  (operator_isBackslashGreaterThan)
   koper  (operator_isLessThan)
   koper  (operator_isBackslashLessThan)
   koper  (operator_isGreaterOrEqual)
   koper  (operator_isLessOrEqual)
   koper  (operator_strictEqual)
   koper  (operator_strictNotEqual)
   koper  (operator_strictGreaterThan)
   koper  (operator_strictBackslashGreaterThan)
   koper  (operator_strictLessThan)
   koper  (operator_strictBackslashLessThan)
   koper  (operator_strictGreaterOrEqual)
   koper  (operator_strictLessOrEqual)
   koper  (operator_lessThanGreaterThan)
   koper  (operator_greaterThanLessThan)
   koper  (operator_and)
   koper  (operator_or)
   koper  (operator_xor)
   koper  (operator_not)
// These are the external(exported) versions.
   koper  (operator_plusRexx)
   koper  (operator_minusRexx)
   koper  (operator_multiplyRexx)
   koper  (operator_divideRexx)
   koper  (operator_integerDivideRexx)
   koper  (operator_remainderRexx)
   koper  (operator_powerRexx)
   koper  (operator_abuttalRexx)
   koper  (operator_concatRexx)
   koper  (operator_concatBlankRexx)
   koper  (operator_equalRexx)
   koper  (operator_notEqualRexx)
   koper  (operator_isGreaterThanRexx)
   koper  (operator_isBackslashGreaterThanRexx)
   koper  (operator_isLessThanRexx)
   koper  (operator_isBackslashLessThanRexx)
   koper  (operator_isGreaterOrEqualRexx)
   koper  (operator_isLessOrEqualRexx)
   koper  (operator_strictEqualRexx)
   koper  (operator_strictNotEqualRexx)
   koper  (operator_strictGreaterThanRexx)
   koper  (operator_strictBackslashGreaterThanRexx)
   koper  (operator_strictLessThanRexx)
   koper  (operator_strictBackslashLessThanRexx)
   koper  (operator_strictGreaterOrEqualRexx)
   koper  (operator_strictLessOrEqualRexx)
   koper  (operator_lessThanGreaterThanRexx)
   koper  (operator_greaterThanLessThanRexx)
   koper  (operator_andRexx)
   koper  (operator_orRexx)
   koper  (operator_xorRexx)
   koper  (operator_notRexx)

  ULONG       proxyFlags;
  RexxObject *serverObject;            /* Server to handle this proxy       */
  RexxObject *RexxSOMObj;              /* SOMObject as a RexxINteger        */
#ifdef SOM
  SOMObject  *somObj;                  /* associated SOMObject for proxy    */
#endif
};

class RexxSOMProxyClass : public RexxClass {
 public:
  inline RexxSOMProxyClass(RESTORETYPE restoreType) { ; };
  RexxSOMProxyClass();
  void *operator new(size_t size, long size1, RexxBehaviour *classBehave, RexxBehaviour *instance) { return new (size, classBehave, instance) RexxClass; }
  void *operator new(size_t size, void *ptr) {return ptr;};
  void *operator new(size_t);
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);

  RexxObject  *unknown(RexxString *, RexxArray *);
  RexxObject  *initProxy(RexxInteger *);
  RexxObject  *freeSOMObj();
  RexxObject  *server();
  RexxObject  *SOMObj();
  RexxInteger *hasMethod(RexxString *);
  void        *realSOMObject();

  RexxSOMProxy *newRexx(RexxObject **, size_t);
  RexxObject   *init(RexxObject **, size_t);
  RexxSOMProxy *somdNew();


  RexxObject *serverObject;            /* Server to handle this proxy       */
  RexxObject *RexxSOMObj;              /* SOMObject as a RexxINteger        */
#ifdef SOM
  SOMObject  *somObj;                  /* associated SOMObject for proxy    */
#endif
};
#endif

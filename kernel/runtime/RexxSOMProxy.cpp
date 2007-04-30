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
/*******************************************************************************/
/* REXX Kernel                                                  RexxSOMProxy.c       */
/*                                                                             */
/* Primitive SOM Proxy Class                                                   */
/*                                                                             */
/*******************************************************************************/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "MethodClass.hpp"
#include "RexxSOMProxy.hpp"

#ifdef SOM
 #include <som.xh>
 #include "dlfobj.xh"
 #include "orxsminf.xh"
 #include "orxsargl.xh"
 #ifdef DSOM
  #include <somd.xh>
 #endif
#include "SOMUtilities.h"
#endif


extern RexxObject *ProcessLocalServer;

                                       /* define an operator forwarding     */
                                       /* method                            */
#define numberstring_forwarder(method, numberStringMethod)                          \
  RexxObject *RexxSOMProxy::method(                                                 \
      RexxObject *operand)             /* operator "right hand side"        */      \
{                                                                                   \
  if (isDLFBasic(this)) {                                                           \
                                       /* do a real message send            */      \
    return this->numberString()->numberStringMethod(operand);                       \
  }                                                                                 \
  else {                                                                            \
    return this->method(operand);                                                   \
  }                                                                                 \
}
                                       /* define an operator forwarding     */
                                       /* method                            */
#define string_forwarder(method, stringMethod)                                \
RexxObject  *RexxSOMProxy::method(                                            \
      RexxObject *operand)             /* operator "right hand side"        */\
{                                                                             \
  if (isDLFBasic(this)) {                                                     \
                                       /* do a real message send            */\
    return this->stringValue()->stringMethod(operand);                        \
  }                                                                           \
  else {                                                                      \
    return this->RexxObject::method(operand);                                 \
  }                                                                           \
}

                                       /* define an operator forwarding     */
                                       /* method                            */
#define string_forwarderEqual(method, stringMethod)                           \
RexxObject  *RexxSOMProxy::method(                                            \
      RexxObject *operand)             /* operator "right hand side"        */\
{                                                                             \
  if (isDLFBasic(this)) {                                                     \
                                       /* do a real message send            */\
    return this->stringValue()->stringMethod(operand);                        \
  }                                                                           \
  else {                                                                      \
    return this->RexxObject::equal(operand);                                  \
  }                                                                           \
}

                                       /* define an operator forwarding     */
                                       /* method                            */
#define numberstring_forwarderRexx(method, numberStringMethod, message)             \
  RexxObject *RexxSOMProxy::method##Rexx(                                           \
      RexxObject *operand)             /* operator "right hand side"        */      \
{                                                                                   \
  if (isDLFBasic(this)) {                                                           \
                                       /* do a real message send            */      \
    return this->numberString()->numberStringMethod(operand);                       \
  }                                                                                 \
  else {                                                                            \
    report_nomethod(OREF_##message, this);                                          \
    return OREF_NULL;                                                               \
  }                                                                                 \
}
                                       /* define an operator forwarding     */
                                       /* method                            */
#define string_forwarderRexx(method, stringMethod, message)                   \
RexxObject  *RexxSOMProxy::method##Rexx(                                      \
      RexxObject *operand)             /* operator "right hand side"        */\
{                                                                             \
  if (isDLFBasic(this)) {                                                     \
                                       /* do a real message send            */\
    return this->stringValue()->stringMethod(operand);                        \
  }                                                                           \
  else {                                                                      \
    report_nomethod(OREF_##message, this);                                    \
    return OREF_NULL;                                                         \
  }                                                                           \
}

                                       /* define an operator forwarding     */
                                       /* method                            */
#define string_forwarderRexxObject(method, stringMethod, message)             \
RexxObject  *RexxSOMProxy::method##Rexx(                                      \
      RexxObject *operand)             /* operator "right hand side"        */\
{                                                                             \
  if (isDLFBasic(this)) {                                                     \
                                       /* do a real message send            */\
    return this->stringValue()->stringMethod(operand);                        \
  }                                                                           \
  else {                                                                      \
    return this->stringMethod(operand);                                       \
  }                                                                           \
}



void   RexxSOMProxy::live()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->objectVariables);
  memory_mark(this->serverObject);
  memory_mark(this->RexxSOMObj);
  cleanUpMemoryMark
}

void   RexxSOMProxy::liveGeneral()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->serverObject);
  memory_mark_general(this->RexxSOMObj);
  cleanUpMemoryMarkGeneral
}

void   RexxSOMProxy::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  setUpFlatten(RexxSOMProxy)

   flatten_reference(newThis->objectVariables, envelope);
   flatten_reference(newThis->serverObject, envelope);
   flatten_reference(newThis->RexxSOMObj, envelope);
  cleanUpFlatten
}

RexxString *RexxSOMProxy::defaultName()
/******************************************************************************/
/* Arguments:  String type option                                             */
/*                                                                            */
/*  Returned:  String object for integer value                                */
/******************************************************************************/
{
#ifdef SOM
                                       /* Do we have a SOM class name       */
                                       /* and on correct server.            */
  if (this->somObj && ProcessLocalServer == this->serverObject) {
    RexxString *myName = new_cstring(this->somObj->somGetClassName());
                                       /* check if it is from an enhanced   */
                                       /* class                             */
    if (this->behaviour->isEnhanced()) {
                                       /* return the 'enhanced' id          */
     return myName->concatToCstring("enhanced ");
    }
    switch (myName->getChar(0)) {      /* process based on first character  */
      case 'a':                        /* vowels                            */
      case 'A':
      case 'e':
      case 'E':
      case 'i':
      case 'I':
      case 'o':
      case 'O':
      case 'u':
      case 'U':
                                       /* prefix with "an"                  */
        myName = myName->concatToCstring("an ");
        break;

      default:                         /* consonants                        */
                                       /* prefix with "a"                   */
        myName = myName->concatToCstring("a ");
        break;
    }
    return myName;                     /* return that value                 */
  }
  else {
                                       /* nope, use Objects defaultName meth*/
    return this->RexxObject::defaultName();
  }
#else
    return this->RexxObject::defaultName();
#endif
}

RexxString *RexxSOMProxy::stringValue()
/******************************************************************************/
/* Arguments:  String type option                                             */
/*                                                                            */
/*  Returned:  String object for integer value                                */
/******************************************************************************/
{
#ifdef SOM
  char *stringValue;
  RexxString *stringObject;
  Environment *ev = somGetGlobalEnvironment();

  if (this->somObj) {                  /* is there an associated SOMObj     */
                                       /* Yes, is it a DLF Number           */
   if (isDLFBasic(this)) {
                                       /* on the correct server?            */
     if (ProcessLocalServer != this->serverObject) {
                                       /* process switch and get string     */
      return (RexxString *)send_message3(this->serverObject, OREF_SEND, this, OREF_STRINGSYM, TheNullArray);
     }
     else {
                                       /* Get string value of DLF obj       */
      stringValue = ((DLFBasicType *)this->somObj)->asString(ev);
      stringObject = (RexxString *)new_cstring(stringValue);
      SOMFree(stringValue);            /* Be sure to free string storage    */
      return stringObject;
     }
   }
  }
#endif
                                       /* just return the objects name      */
  return this->RexxObject::stringValue();
}

RexxString *RexxSOMProxy::makeString()
/******************************************************************************/
/* Arguments:  String type option                                             */
/*                                                                            */
/*  Returned:  String object for integer value                                */
/******************************************************************************/
{
#ifdef SOM
  char *stringValue;
  RexxString *stringObject;
  Environment *ev = somGetGlobalEnvironment();

  if (this->somObj) {                  /* is there an associated SOMObj     */
                                       /* Yes, is it a DLF Number           */
   if (isDLFBasic(this)) {
                                       /* on the correct server?            */
     if (ProcessLocalServer != this->serverObject) {
                                       /* process switch and get string     */
      return (RexxString *)send_message3(this->serverObject, OREF_SEND, this, OREF_MAKESTRING, TheNullArray);
     }
     else {
                                       /* Get string value of DLF obj       */
      stringValue = ((DLFBasicType *)this->somObj)->asString(ev);
      stringObject = (RexxString *)new_cstring(stringValue);
      SOMFree(stringValue);            /* Be sure to free string storage    */
      return stringObject;
     }
   }
  }
#endif
                                       /* just return the objects name      */
  return (RexxString *)TheNilObject;
}

double    RexxSOMProxy::doubleValue()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Number value                                                   */
/******************************************************************************/
{
#ifdef SOM
  Environment *ev = somGetGlobalEnvironment();
  if (this->somObj) {                  /* is there an associated SOMObj     */
                                       /* on the correct server?            */
    if (ProcessLocalServer != this->serverObject) {
                                       /* just return the objects name      */
      return NO_DOUBLE;
    }
                                       /* Yes, is it a DLF Number           */
    if (isDLFNumber(this)) {
                                       /* Get double value of DLF obj       */
      return ((DLFNumber *)this->somObj)->asDouble(ev);
    }
  }
#endif
  return NO_DOUBLE;
}

RexxNumberString *RexxSOMProxy::numberString()
/******************************************************************************/
/* Arguments:  none                                                           */
/*                                                                            */
/*  Returned:  Numberstring object for integer value                          */
/******************************************************************************/
{
#ifdef SOM
  char *stringValue;
  RexxNumberString *stringObject;
  Environment *ev = somGetGlobalEnvironment();

  if (this->somObj) {                  /* is there an associated SOMObj     */
                                       /* Yes, is it a DLF Number           */
    if (isDLFNumber(this)) {
                                       /* on the correct server?            */
      if (ProcessLocalServer != this->serverObject) {
                                       /* process switch and get string     */
         stringObject = (RexxNumberString *)send_message3(this->serverObject, OREF_SEND, this, OREF_STRINGSYM, TheNullArray);
                                       /* now convert to numberstring       */
         return stringObject->numberString();
      }
      else {
                                       /* Get numberString value of DLF obj */
        stringValue = ((DLFBasicType *)this->somObj)->asString(ev);
        stringObject = (RexxNumberString *)new_numberstring(stringValue, strlen(stringValue));
        SOMFree(stringValue);            /* Be sure to free string storage    */
        return stringObject;
      }
    }
    else
       return (RexxNumberString *)TheNilObject;
  }
#endif
  return (RexxNumberString *)TheNilObject;
}

long   RexxSOMProxy::longValue(long digits)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Number value                                                   */
/******************************************************************************/
{
#ifdef SOM
  Environment *ev = somGetGlobalEnvironment();
  if (this->somObj) {                  /* is there an associated SOMObj     */
                                       /* on the correct server?            */
    if (ProcessLocalServer != this->serverObject) {
                                       /* just return the objects name      */
      return NO_LONG;
    }
                                       /* Yes, is it a DLF Number           */
    if ( isDLFNumber(this)) {
                                       /* Get double value of DLF obj       */
      return ((DLFNumber *)this->somObj)->asLong(ev);
    }
  }
#endif
  return NO_LONG;
}

RexxObject *RexxSOMProxy::unknown(RexxString *msgname, RexxArray *arguments)
/******************************************************************************/
/* Arguments:  Message name, arguments array                                  */
/*                                                                            */
/*  Returned:  Result of the corresponding string method                      */
/******************************************************************************/
{
#ifdef SOM
  RexxObject *rc;
  somId messageId;
  SOMClass *classobj;
  RexxString *stringObject;
                                       /* Associated SOMObject ?            */
  if (!this->somObj)
                                       /* Nope, report error                */
    report_nomethod(msgname, this);
                                       /* on the correct server?            */
  if (ProcessLocalServer != this->serverObject) {
                                       /* nope, resend on correct server    */
    return send_message3(this->serverObject, OREF_SEND, this, msgname, arguments);
  }
                                       /* See if method was removed on the  */
                                       /*   OREXX side.                     */
  if (this->behaviour->getMethod(msgname) == TheNilObject) {
                                       /* It was so we report no method     */
    report_nomethod(msgname, this);
  }
                                       /* Get message name into SOMId       */
  messageId = somIdFromString(msgname->getStringData());
                                       /* Does somobj understand msgName?   */
  if (!this->somObj->somRespondsTo(messageId)) {
                                       /* nope, free id.,                   */
    SOMFree(messageId);
    if (isDLFBasic(this)) {
      stringObject = this->stringValue();
                                       /* See if string objects knows messag*/
      if (stringObject->methodLookup(msgname) != TheNilObject) {
                                       /* yup, send message to string       */
        return send_message(stringObject, msgname, arguments);
      }
    }
                                       /* report unknown method             */
    report_nomethod(msgname, this);
  }
  SOMFree(messageId);                  /* Done with message ID. Free it.    */
  classobj = SOM_GetClass(this->somObj);
#ifdef DSOM
  if (isDSOMProxy(this)) {             /* Is it a DSOM Proxy?               */
                                       /* Send via DSOM                     */
    rc = dsom_send((SOMDObject *)this->somObj, classobj, this, msgname, arguments->size(), arguments->data(), NULL, NULL);
  }
  else {
#endif
                                       /* Go dispatch the message to SOM    */
   rc = som_send(this->somObj, classobj, this, msgname, arguments->size(), arguments->data(), NULL, NULL, NULL);
#ifdef DSOM
  }
#endif

  return rc;
#else
  report_nomethod(msgname, this);
  return OREF_NULL;
#endif
}

RexxInteger *RexxSOMProxy::integer(RexxInteger *digits)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
  long longValue;
  longValue = this->longValue(NO_LONG);
  return new_integer(longValue);
}

RexxObject *RexxSOMProxy::initProxy(RexxInteger *somobj)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef SOM
  if (somobj != TheNilObject) {
                                       /* Server is current local server    */
    OrefSet(this, this->serverObject, ProcessLocalServer);
    OrefSet(this, this->RexxSOMObj, somobj);
                                       /* Get address of actual SOMObject   */
    this->somObj = (SOMObject *)somobj->value;

    if (this->somObj->somIsA(_DLFNumber)) {
      this->proxyFlags |= flagDLFNumber;
    }
    else if (this->somObj->somIsA(_DLFBasicType)) {
      this->proxyFlags |= flagDLFBasic;
    }
    else if (this->somObj->somIsA(_DLFObject)) {
      this->proxyFlags |= flagDLFObject;
    }
#ifdef DSOM
    else if (this->somObj->somIsA(_SOMDObject)) {
      this->proxyFlags |= flagDSOMProxy;
    }
#endif
  }
  else {
    this->somObj = NULL;
  }
#endif
  return OREF_NULL;
}

RexxObject *RexxSOMProxy::freeSOMObj()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef SOM
  if (this->somObj) {                  /* is there an assiated SOMObject    */
                                       /* on the correct server?            */
    if (ProcessLocalServer != this->serverObject) {
                                       /* nope, resend on correct server    */
      return send_message3(this->serverObject, OREF_SEND, this, new_cstring(CHAR_FREESOMOBJ), TheNullArray);
    }
    this->somObj->somFree();           /* Yes, free it.                     */
    this->somObj = NULL;               /* Indicate no more assicative obj   */
    OrefSet(this, this->RexxSOMObj, TheNilObject);
    OrefSet(this, this->serverObject, TheNilObject);
    this->proxyFlags = 0;
  }
#endif
  return OREF_NULL;
}

RexxObject *RexxSOMProxy::server()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
   return this->serverObject;
}

RexxObject *RexxSOMProxy::SOMObj()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
                                       /* is this somObject known on this   */
                                       /* server/process?                   */
  if (ProcessLocalServer == this->serverObject) {
                                       /* yup, return it.                   */
    return this->RexxSOMObj;
  }
  else {
                                       /* nope, then its dioesn't exist !!  */
    return TheNilObject;
  }

}

void       *RexxSOMProxy::realSOMObject()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef SOM
   return this->somObj;
#else
   return NULL;
#endif
}

RexxInteger *RexxSOMProxy::hasMethod(RexxString *msgName)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
  RexxString *stringObject;
                                       /* Does this object know about method*/
                                       /* Normal hasmethond....             */
  if (this->RexxObject::hasMethod(msgName) == TheFalseObject) {
                                       /* on the correct server?            */
    if (ProcessLocalServer != this->serverObject) {
                                       /* nope, resend on correct server    */
      return (RexxInteger *)send_message3(this->serverObject, OREF_SEND, this, OREF_HASMETHOD, TheNullArray);
    }
    else {
      if (isDLFBasic(this)) {
        stringObject = this->stringValue();
                                       /* See if string objects knows messag*/
        return stringObject->hasMethod(msgName);
      }
      else {
        return TheFalseObject;
      }
    }
                                       /* yup, send message to string       */
  }
  return TheTrueObject;
}

numberstring_forwarder(operator_plus, plus)
numberstring_forwarder(operator_minus, minus)
numberstring_forwarder(operator_multiply, multiply)
numberstring_forwarder(operator_divide, divide)
numberstring_forwarder(operator_integerDivide, integerDivide)
numberstring_forwarder(operator_remainder, remainder)
numberstring_forwarder(operator_power, power)


string_forwarder(operator_abuttal, concatRexx)
string_forwarder(operator_concat, concatRexx)
string_forwarder(operator_concatBlank, concatBlank)
string_forwarderEqual(operator_equal, equal)
string_forwarder(operator_notEqual, notEqual)
string_forwarder(operator_isGreaterThan, isGreaterThan)
string_forwarder(operator_isBackslashGreaterThan, isLessOrEqual)
string_forwarder(operator_isLessThan, isLessThan)
string_forwarder(operator_isBackslashLessThan, isGreaterOrEqual)
string_forwarder(operator_isGreaterOrEqual, isGreaterOrEqual)
string_forwarder(operator_isLessOrEqual, isLessOrEqual)
string_forwarder(operator_strictEqual, strictEqual)
string_forwarder(operator_strictNotEqual, strictNotEqual)
string_forwarder(operator_strictGreaterThan, strictGreaterThan)
string_forwarder(operator_strictBackslashGreaterThan, strictLessOrEqual)
string_forwarder(operator_strictLessThan, strictLessThan)
string_forwarder(operator_strictBackslashLessThan, strictGreaterOrEqual)
string_forwarder(operator_strictGreaterOrEqual, strictGreaterOrEqual)
string_forwarder(operator_strictLessOrEqual, strictLessOrEqual)
string_forwarder(operator_lessThanGreaterThan, notEqual)
string_forwarder(operator_greaterThanLessThan, notEqual)
string_forwarder(operator_and, andOp)
string_forwarder(operator_or, orOp)
string_forwarder(operator_xor, xorOp)
string_forwarder(operator_not, operator_not)

numberstring_forwarderRexx(operator_plus, plus, PLUS)
numberstring_forwarderRexx(operator_minus, minus, SUBTRACT)
numberstring_forwarderRexx(operator_multiply, multiply, MULTIPLY)
numberstring_forwarderRexx(operator_divide, divide, DIVIDE)
numberstring_forwarderRexx(operator_integerDivide, integerDivide, INTDIV)
numberstring_forwarderRexx(operator_remainder, remainder, REMAINDER)
numberstring_forwarderRexx(operator_power, power, POWER)


string_forwarderRexx(operator_abuttal, concatRexx, NULLSTRING)
string_forwarderRexx(operator_concat, concatRexx, CONCATENATE)
string_forwarderRexx(operator_concatBlank, concatBlank, BLANK)
string_forwarderRexx(operator_isGreaterThan, isGreaterThan, GREATERTHAN)
string_forwarderRexx(operator_isBackslashGreaterThan, isLessOrEqual, BACKSLASH_GREATERTHAN)
string_forwarderRexx(operator_isLessThan, isLessThan, LESSTHAN)
string_forwarderRexx(operator_isBackslashLessThan, isGreaterOrEqual, BACKSLASH_LESSTHAN)
string_forwarderRexx(operator_isGreaterOrEqual, isGreaterOrEqual, GREATERTHAN_EQUAL)
string_forwarderRexx(operator_isLessOrEqual, isLessOrEqual, LESSTHAN_EQUAL)
string_forwarderRexx(operator_strictGreaterThan, strictGreaterThan, STRICT_GREATERTHAN)
string_forwarderRexx(operator_strictBackslashGreaterThan, strictLessOrEqual, STRICT_BACKSLASH_GREATERTHAN)
string_forwarderRexx(operator_strictLessThan, strictLessThan, STRICT_LESSTHAN)
string_forwarderRexx(operator_strictBackslashLessThan, strictGreaterOrEqual, STRICT_BACKSLASH_LESSTHAN)
string_forwarderRexx(operator_strictGreaterOrEqual, strictGreaterOrEqual, STRICT_GREATERTHAN_EQUAL)
string_forwarderRexx(operator_strictLessOrEqual, strictLessOrEqual, STRICT_LESSTHAN_EQUAL)
string_forwarderRexx(operator_and, andOp, AND)
string_forwarderRexx(operator_or, orOp, OR)
string_forwarderRexx(operator_xor, xorOp, XOR)
string_forwarderRexx(operator_not, operator_not, BACKSLASH)

string_forwarderRexxObject(operator_equal, equal, EQUAL)
string_forwarderRexxObject(operator_notEqual, notEqual, BACKSLASH_EQUAL)
string_forwarderRexxObject(operator_strictEqual, strictEqual, STRICT_EQUAL)
string_forwarderRexxObject(operator_strictNotEqual, strictNotEqual, STRICT_BACKSLASH_EQUAL)
string_forwarderRexxObject(operator_lessThanGreaterThan, notEqual, LESSTHAN_GREATERTHAN)
string_forwarderRexxObject(operator_greaterThanLessThan, notEqual, GREATERTHAN_LESSTHAN)

/* **************************************** */
/*  SOMProxy class methods begin here ..... */
/* **************************************** */


RexxSOMProxyClass::RexxSOMProxyClass()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
 this->hashvalue = HASHOREF(this);
 this->RexxSOMObj = TheNilObject;
}

RexxObject *  RexxSOMProxyClass::init(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
 this->hashvalue = HASHOREF(this);
 this->RexxSOMObj = TheNilObject;
 return OREF_NULL;
}

void   RexxSOMProxyClass::live()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{

  this->RexxClass::live();            /* 1st call our parent(super) to do its */
                                      /*  live.                               */

  setUpMemoryMark
  memory_mark(this->serverObject);
  memory_mark(this->RexxSOMObj);
  cleanUpMemoryMark
}


void   RexxSOMProxyClass::liveGeneral()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  this->RexxClass::liveGeneral();   /* 1st call our parent(super) to do its */
                                    /*  live.                               */
  setUpMemoryMarkGeneral
  memory_mark_general(this->serverObject);
  memory_mark_general(this->RexxSOMObj);
  cleanUpMemoryMarkGeneral
}

void   RexxSOMProxyClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/*                                                                            */
/******************************************************************************/
{
                                       /* just pass on the flatten message  */
   this->RexxClass::flatten(envelope);
}

RexxObject *RexxSOMProxyClass::unknown(RexxString *msgname, RexxArray *arguments)
/******************************************************************************/
/* Arguments:  Message name, arguments array                                  */
/*                                                                            */
/*  Returned:  Result of the corresponding string method                      */
/******************************************************************************/
{
#ifdef SOM
  RexxObject *rc;
  somId messageId;
  SOMClass *classobj;
                                       /* Associated SOMObject ?            */
  if (!this->somObj)
                                       /* Nope, report error                */
    report_exception1(Error_Execution_nosomobj, this);
                                       /* on the correct server?            */
  if (ProcessLocalServer != this->serverObject) {
                                       /* nope, resend on correct server    */
    return send_message3(this->serverObject, OREF_SEND, this, msgname, arguments);
  }
                                       /* Get message name into SOMId       */
  messageId = somIdFromString(msgname->getStringData());
                                       /* Doex somobj understand msgName?   */
  if (!this->somObj->somRespondsTo(messageId)) {
                                       /* nope, free id.,                   */
    SOMFree(messageId);
                                       /* report unknown method             */
    report_nomethod(msgname, this);
  }
  SOMFree(messageId);                  /* Done with message ID. Free it.    */
                                       /* Get class object for this object. */
  classobj = SOM_GetClass(this->somObj);
#ifdef DSOM
                                       /* is Object a DSOM Proxy            */
  if (this->somObj->somIsA(_SOMDObject)) {
                                       /* Go dispatch the message to SOM    */
    rc = dsom_send((SOMDObject *)this->somObj, classobj, this, msgname, arguments->size(), arguments->data(), NULL, NULL);
  }
  else {
#endif
                                       /* Go dispatch the message to SOM    */
    rc = som_send(this->somObj, classobj, this, msgname, arguments->size(), arguments->data(), NULL, NULL, NULL);
#ifdef DSOM
  }
#endif

  return rc;
#else
  report_nomethod(msgname, this);
  return 0;
#endif
}

RexxObject *RexxSOMProxyClass::initProxy(RexxInteger *somobj)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef SOM
 if (somobj != TheNilObject) {
                                       /* Server is current local server    */
  OrefSet(this, this->serverObject, ProcessLocalServer);
  OrefSet(this, this->RexxSOMObj, somobj);
                                       /* Get address of actual SOMObject   */
  this->somObj = (SOMObject *)somobj->value;
 }
 else {
   this->somObj = NULL;
 }
#endif
  return OREF_NULL;
}

RexxObject *RexxSOMProxyClass::freeSOMObj()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef SOM
  if (this->somObj) {                  /* is there an assiated SOMObject    */
                                       /* on the correct server?            */
    if (ProcessLocalServer != this->serverObject) {
                                       /* nope, resend on correct server    */
      return send_message3(this->serverObject, OREF_SEND, this, new_cstring(CHAR_FREESOMOBJ), TheNullArray);
    }
    this->somObj->somFree();           /* Yes, free it.                     */
    this->somObj = NULL;               /* Indicate no more assicative obj   */
    OrefSet(this, this->RexxSOMObj, TheNilObject);
    OrefSet(this, this->serverObject, TheNilObject);
  }
#endif
  return OREF_NULL;
}

RexxObject *RexxSOMProxyClass::server()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
   return this->serverObject;
}

RexxObject *RexxSOMProxyClass::SOMObj()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
                                       /* is this somObject known on this   */
                                       /* server/process?                   */
  if (ProcessLocalServer == this->serverObject) {
                                       /* yup, return it.                   */
    return this->RexxSOMObj;
  }
  else {
                                       /* nope, then its dioesn't exist !!  */
    return TheNilObject;
  }
}

void       *RexxSOMProxyClass::realSOMObject()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef SOM
   return this->somObj;
#else
   return NULL;
#endif
}

RexxInteger *RexxSOMProxyClass::hasMethod(RexxString *msgname)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
  return this->RexxObject::hasMethod(msgname);
}

RexxSOMProxy *RexxSOMProxyClass::somdNew()
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  self                                                           */
/******************************************************************************/
{
#ifdef DSOM
  SOMObject *newSomObj;
  Environment *ev = somGetGlobalEnvironment();
  Identifier classId;
  RexxActivity *myActivity;


  if (this->somObj) {                  /* is there an assiated SOMObject    */
                                       /* on the correct server?            */
    if (ProcessLocalServer != this->serverObject) {
                                       /* nope, resend on correct server    */
      return (RexxSOMProxy *)send_message3(this->serverObject, OREF_SEND, this, new_cstring(CHAR_SOMDNEW), TheNullArray);
    }
    else {
                                       /* Remember out Activity.            */
      myActivity = CurrentActivity;
      ReleaseKernelAccess(myActivity); /* Give up kernel while go to DSOM   */
                                       /* get id for class                  */
      classId = ((SOMClass *)this->somObj)->somGetName();
                                       /* Create new proxy Object           */
      newSomObj = SOMD_ObjectMgr->somdNewObject(ev, classId, "");
                                       /* Get kernel access again.          */
      RequestKernelAccess(myActivity);
                                       /* Create OREXX proxy for DSOM Proxy */
      return (RexxSOMProxy *)send_message1(this->serverObject, OREF_MAKE_PROXY, new_pointer((long)newSomObj));
    }
  }
  else
#endif
   return (RexxSOMProxy *)TheNilObject;
}


RexxSOMProxy *RexxSOMProxyClass::newRexx(RexxObject **init_args, size_t argCount)
/******************************************************************************/
/* Function:  Create a new integer object                                     */
/******************************************************************************/
{
  RexxSOMProxy *newObject;             /* newly create object               */

                                       /* get a new object                  */
  newObject = new RexxSOMProxy;
                                       /* add in the integer behaviour, and */
                                       /* make sure old2new knows about it  */
  BehaviourSet(newObject, this->instanceBehaviour);

                                       /* set the default hash value        */
                                       /* does object have an UNINT method  */
  if (this->uninitDefined())  {
                                       /* Make sure everyone is notified.   */
     newObject->hasUninit();
  }
  return newObject;                    /* return the new object.            */

}

void *RexxSOMProxy::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new SOMProxy object                                    */
/******************************************************************************/
{
  RexxSOMProxy *newObject;             /* newly create object               */

                                       /* get a new object                  */
  newObject = (RexxSOMProxy *)new_object(size);
                                       /* add in the integer behaviour, and */
                                       /* make sure old2new knows about it  */
  BehaviourSet(newObject, TheSomProxyBehaviour);
  ClearObject(newObject);              /* clear the object                  */
  newObject->RexxSOMObj = TheNilObject;/* set the integer value             */
  newObject->hashvalue = HASHOREF(newObject);
  return newObject;
}

void somproxy_create (void)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
                                       /* create base SOMPROXY class.       */
  create_udsubClass(SomProxy, RexxSOMProxyClass);
                                       /* class specific initialization     */
  new (TheSomProxyClass) RexxSOMProxyClass;
}


void somproxy_setup (void)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  RexxClass *myClass;
  RexxClass *myMetaClass;

                                       /* now we will hand construct the    */
                                       /*  SOMPROXY Meta class M_SOMPROXY   */
                                       /* copy SOMPROXY class for metaClass */
    myMetaClass = TheMSomProxyClass = (RexxSOMProxyClass *)TheSomProxyClass->copy();
    myClass = TheSomProxyClass;

                                       /* setUp M_Somproxy to be a metaclass*/
    myMetaClass->setMeta();
                                       /* Give M_SOMPROXY its own behaviour */
    myMetaClass->behaviour = (RexxBehaviour *)TheSomProxyClassBehaviour->copy();
                                       /* and its own instance behaviour,   */
                                       /*  don't forget this is a Meta !!   */
    myMetaClass->setInstanceBehaviour((RexxBehaviour *)TheSomProxyClassBehaviour->copy());
                                       /*   somproxy no longer primitive cls*/
    myClass->setNotPrimitive();
                                       /* M_somproxy no longer primitive cls*/
    myMetaClass->setNotPrimitive();
                                       /* Make M_SOMPROXY the meta for SOMP */
    myClass->setMetaClass(myMetaClass);
                                       /* Make M_SOMPROXY its own Meta      */
    myMetaClass->setMetaClass(myMetaClass);

}

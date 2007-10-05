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
/* REXX Kernel                                                  oksetup.c     */
/*                                                                            */
/* Setup initial class definitions during an image build                      */
/*                                                                            */
/******************************************************************************/
#include <string.h>
#include "RexxCore.h"
 #include "TableClass.hpp"
 #include "RexxMemory.hpp"
 #include "RexxBehaviour.hpp"
 #include "ClassClass.hpp"
 #include "NumberStringClass.hpp"
 #include "IntegerClass.hpp"
 #include "StringClass.hpp"
 #include "MutableBufferClass.hpp"
 #include "ArrayClass.hpp"
 #include "DirectoryClass.hpp"
 #include "RelationClass.hpp"
 #include "ListClass.hpp"
 #include "QueueClass.hpp"
 #include "SupplierClass.hpp"
 #include "RexxSOMProxy.hpp"
 #include "MethodClass.hpp"
 #include "RexxEnvelope.hpp"
 #include "MessageClass.hpp"
 #include "StemClass.hpp"
 #include "RexxMisc.hpp"
 #include "RexxNativeMethod.hpp"
 #include "RexxSOMCode.hpp"
 #include "RexxActivity.hpp"
 #include "RexxNativeActivation.hpp"
 #include "RexxVariableDictionary.hpp"
 #include "ExpressionVariable.hpp"
 #include "RexxLocalVariables.hpp"

PCPPM ExportedMethods[] = {            /* start of exported methods table   */
                                       /* NOTE:  getAttribute and           */
                                       /* setAttribute need to be the first */
                                       /* entries in the table, since these */
                                       /* need to be created dynamically, so*/
                                       /* they need to have a fixed,        */
                                       /* predictable location              */
CPPM(RexxObject::getAttribute),
CPPM(RexxObject::setAttribute),
CPPM(RexxObject::abstractMethod),      // also must be at this location ALWAYS
CPPM(RexxObject::objectName),
CPPM(RexxObject::objectNameEquals),
CPPM(RexxObject::run),
CPPM(RexxObject::stringRexx),
CPPM(RexxObject::notEqual),
CPPM(RexxObject::setMethod),
CPPM(RexxObject::hasMethodRexx),
CPPM(RexxObject::start),
CPPM(RexxObject::unsetMethod),
CPPM(RexxObject::requestRexx),
CPPM(RexxObject::makeStringRexx),
CPPM(RexxObject::makeArrayRexx),
CPPMIO(RexxInternalObject::hasUninit),
CPPM(RexxObject::classObject),
CPPM(RexxObject::equal),
CPPM(RexxObject::strictEqual),
CPPM(RexxObject::hashCode),
CPPM(RexxObject::init),
CPPM(RexxObject::strictNotEqual),
CPPM(RexxObject::copyRexx),
CPPM(RexxObject::defaultNameRexx),
CPPM(RexxObject::unknownRexx),
CPPM(RexxObject::isInstanceOfRexx),
CPPM(RexxObject::instanceMethodRexx),
CPPM(RexxObject::instanceMethodsRexx),
CPPM(RexxObject::initProxyRexx),
CPPM(RexxObject::freeSOMObjRexx),
CPPM(RexxObject::SOMObjRexx),
CPPM(RexxObject::serverRexx),
CPPM(RexxObject::identityHashRexx),
CPPM(RexxObject::concatRexx),
CPPM(RexxObject::concatBlank),

CPPM(RexxObject::newRexx),

CPPMC(RexxClass::setRexxDefined),       /* Class methods                     */
CPPMC(RexxClass::defaultNameRexx),
CPPMC(RexxClass::queryMixinClass),
CPPMC(RexxClass::getId),
CPPMC(RexxClass::getBaseClass),
CPPMC(RexxClass::getMetaClass),
CPPMC(RexxClass::getSomClass),
CPPMC(RexxClass::setSomClass),
CPPMC(RexxClass::getSuperClasses),
CPPMC(RexxClass::getSuperClass),
CPPMC(RexxClass::getSubClasses),
CPPMC(RexxClass::defmeths),
CPPMC(RexxClass::defineMethod),
CPPMC(RexxClass::defineMethods),
CPPMC(RexxClass::deleteMethod),
CPPMC(RexxClass::method),
CPPMC(RexxClass::methods),
CPPMC(RexxClass::somSuperClass),
CPPMC(RexxClass::inherit),
CPPMC(RexxClass::uninherit),
CPPMC(RexxClass::enhanced),
CPPMC(RexxClass::mixinclass),
CPPMC(RexxClass::subclass),
CPPMC(RexxClass::newOpart),
CPPMC(RexxClass::importMethod),
CPPMC(RexxClass::importedRexx),
CPPMC(RexxClass::equal),
CPPMC(RexxClass::strictEqual),
CPPMC(RexxClass::notEqual),
CPPMC(RexxClass::isSubclassOf),

CPPMC1(RexxClass::newRexx),

CPPMA(RexxArray::sizeRexx),             /* Array methods                     */
CPPMA(RexxArray::items),
CPPMA(RexxArray::dimension),
CPPMA(RexxArray::supplier),
CPPMC1(RexxArray::getRexx),
CPPMC1(RexxArray::putRexx),
CPPMC1(RexxArray::hasIndexRexx),
CPPMA(RexxArray::sectionRexx),
CPPMA(RexxArray::removeRexx),
CPPMA(RexxArray::firstRexx),
CPPMA(RexxArray::lastRexx),
CPPMA(RexxArray::nextRexx),
CPPMA(RexxArray::previousRexx),
CPPMA(RexxArray::append),
CPPMA(RexxArray::allIndexes),
CPPMA(RexxArray::allItems),
CPPMA(RexxArray::empty),
CPPMA(RexxArray::isEmpty),
CPPMA(RexxArray::index),
CPPMA(RexxArray::hasItem),
CPPMA(RexxArray::removeItem),
CPPMA(RexxArray::toString),
CPPMA(RexxArray::sortRexx),
CPPMA(RexxArray::stableSortRexx),
CPPMA(RexxArray::sortWithRexx),
CPPMA(RexxArray::stableSortWithRexx),

CPPMC1(RexxArray::newRexx),
CPPMA(RexxArray::makeString),
CPPMC1(RexxArray::of),

CPPMD(RexxDirectory::atRexx),           /* Directory methods                 */
CPPMD(RexxDirectory::put),
CPPMD(RexxDirectory::entryRexx),
CPPMD(RexxDirectory::hasEntry),
CPPMD(RexxDirectory::hasIndex),
CPPMD(RexxDirectory::itemsRexx),
CPPMHC(RexxHashTableCollection::merge),
CPPMD(RexxDirectory::remove),
CPPMD(RexxDirectory::setEntry),
CPPMD(RexxDirectory::setMethod),
CPPMD(RexxDirectory::supplier),
CPPMA(RexxDirectory::allIndexes),
CPPMA(RexxDirectory::allItems),
CPPMA(RexxDirectory::empty),
CPPMA(RexxDirectory::isEmpty),
CPPMA(RexxDirectory::indexRexx),
CPPMA(RexxDirectory::hasItem),
CPPMA(RexxDirectory::removeItem),

CPPMD(RexxDirectory::newRexx),

CPPME(RexxEnvelope::unpack),            /* Envelope method(s)                */

CPPMI(RexxInteger::plus),               /* Integer methods                   */
CPPMI(RexxInteger::minus),
CPPMI(RexxInteger::multiply),
CPPMI(RexxInteger::divide),
CPPMI(RexxInteger::integerDivide),
CPPMI(RexxInteger::remainder),
CPPMI(RexxInteger::power),
CPPMI(RexxInteger::notOp),
CPPMI(RexxInteger::andOp),
CPPMI(RexxInteger::orOp),
CPPMI(RexxInteger::xorOp),
CPPMI(RexxInteger::equal),
CPPMI(RexxInteger::notEqual),
CPPMI(RexxInteger::strictEqual),
CPPMI(RexxInteger::hashCode),
CPPMI(RexxInteger::strictNotEqual),
CPPMI(RexxInteger::isGreaterThan),
CPPMI(RexxInteger::isLessThan),
CPPMI(RexxInteger::isGreaterOrEqual),
CPPMI(RexxInteger::isLessOrEqual),
CPPMI(RexxInteger::strictGreaterThan),
CPPMI(RexxInteger::strictLessThan),
CPPMI(RexxInteger::strictGreaterOrEqual),
CPPMI(RexxInteger::strictLessOrEqual),
CPPMI(RexxInteger::abs),
CPPMI(RexxInteger::sign),
CPPMI(RexxInteger::Max),
CPPMI(RexxInteger::Min),
CPPMI(RexxInteger::d2x),
CPPMI(RexxInteger::d2c),
CPPMI(RexxInteger::format),
CPPMI(RexxInteger::trunc),

CPPML(RexxList::value),                 /* list methods                      */
CPPML(RexxList::remove),
CPPML(RexxList::firstRexx),
CPPML(RexxList::lastRexx),
CPPML(RexxList::next),
CPPML(RexxList::previous),
CPPML(RexxList::hasIndex),
CPPML(RexxList::supplier),
CPPML(RexxList::itemsRexx),
CPPML(RexxList::put),
CPPML(RexxList::section),
CPPML(RexxList::firstItem),
CPPML(RexxList::lastItem),
CPPML(RexxList::insertRexx),
CPPML(RexxList::append),
CPPML(RexxList::allIndexes),
CPPML(RexxList::allItems),
CPPMA(RexxList::empty),
CPPMA(RexxList::isEmpty),
CPPMA(RexxList::index),
CPPMA(RexxList::hasItem),
CPPMA(RexxList::removeItem),

CPPMLC(RexxListClass::newRexx),
CPPMLC(RexxListClass::classOf),

CPPMSG(RexxMessage::notify),             /* Message methods                   */
CPPMSG(RexxMessage::result),
CPPMSG(RexxMessage::send),
CPPMSG(RexxMessage::start),
CPPMSG(RexxMessage::completed),
CPPMSG(RexxMessage::hasError),
CPPMSG(RexxMessage::errorCondition),
CPPMSG(RexxMessage::messageTarget),
CPPMSG(RexxMessage::messageName),
CPPMSG(RexxMessage::arguments),

CPPMSG(RexxMessage::newRexx),

CPPMTD(RexxMethod::setUnGuardedRexx),    /* Method methods                    */
CPPMTD(RexxMethod::setInterface),
CPPMTD(RexxMethod::getInterface),
CPPMTD(RexxMethod::setGuardedRexx),
CPPMTD(RexxMethod::source),
CPPMTD(RexxMethod::setPrivateRexx),
CPPMTD(RexxMethod::setProtectedRexx),
CPPMTD(RexxMethod::setSecurityManager),
CPPMTD(RexxMethod::isGuardedRexx),
CPPMTD(RexxMethod::isPrivateRexx),
CPPMTD(RexxMethod::isProtectedRexx),

CPPMTDC(RexxMethodClass::newFileRexx),
CPPMTDC(RexxMethodClass::newRexx),

CPPMNM(RexxNumberString::formatRexx),    /* NumberString methods              */
CPPMNM(RexxNumberString::trunc),
CPPMNM(RexxNumberString::equal),
CPPMNM(RexxNumberString::notEqual),
CPPMNM(RexxNumberString::isLessThan),
CPPMNM(RexxNumberString::isGreaterThan),
CPPMNM(RexxNumberString::isGreaterOrEqual),
CPPMNM(RexxNumberString::isLessOrEqual),
CPPMNM(RexxNumberString::strictNotEqual),
CPPMNM(RexxNumberString::strictLessThan),
CPPMNM(RexxNumberString::strictGreaterThan),
CPPMNM(RexxNumberString::strictGreaterOrEqual),
CPPMNM(RexxNumberString::strictLessOrEqual),
CPPMNM(RexxNumberString::plus),
CPPMNM(RexxNumberString::minus),
CPPMNM(RexxNumberString::multiply),
CPPMNM(RexxNumberString::divide),
CPPMNM(RexxNumberString::integerDivide),
CPPMNM(RexxNumberString::remainder),
CPPMNM(RexxNumberString::power),
CPPMNM(RexxNumberString::abs),
CPPMNM(RexxNumberString::Sign),
CPPMNM(RexxNumberString::notOp),
CPPMNM(RexxNumberString::andOp),
CPPMNM(RexxNumberString::orOp),
CPPMNM(RexxNumberString::xorOp),
CPPMNM(RexxNumberString::Max),
CPPMNM(RexxNumberString::Min),
CPPMNM(RexxNumberString::isInteger),
CPPMNM(RexxNumberString::d2c),
CPPMNM(RexxNumberString::d2x),
CPPMNM(RexxNumberString::d2xD2c),
CPPMNM(RexxNumberString::strictEqual),
CPPMNM(RexxNumberString::hashCode),

CPPMQ(RexxQueue::supplier),             /* Queue methods                     */
CPPMQ(RexxQueue::pushRexx),
CPPMQ(RexxQueue::queueRexx),
CPPMQ(RexxQueue::pullRexx),
CPPMQ(RexxQueue::peek),
CPPMQ(RexxQueue::put),
CPPMQ(RexxQueue::at),
CPPMQ(RexxQueue::hasindex),
CPPMQ(RexxQueue::remove),
CPPML(RexxQueue::append),
CPPML(RexxQueue::allIndexes),
CPPML(RexxQueue::index),
CPPML(RexxQueue::firstRexx),
CPPML(RexxQueue::lastRexx),
CPPML(RexxQueue::next),
CPPML(RexxQueue::previous),
CPPML(RexxQueue::insert),

CPPMQ(RexxQueue::newRexx),
CPPMQ(RexxQueue::ofRexx),

CPPMSTEM(RexxStem::bracket),               /* Stem methods                      */
CPPMSTEM(RexxStem::bracketEqual),
CPPMSTEM(RexxStem::request),
CPPMSTEM(RexxStem::supplier),
CPPMSTEM(RexxStem::allIndexes),
CPPMSTEM(RexxStem::allItems),
CPPMSTEM(RexxStem::empty),
CPPMSTEM(RexxStem::isEmpty),
CPPMSTEM(RexxStem::itemsRexx),
CPPMSTEM(RexxStem::hasIndex),
CPPMSTEM(RexxStem::remove),
CPPMSTEM(RexxStem::index),
CPPMSTEM(RexxStem::hasItem),
CPPMSTEM(RexxStem::removeItem),

CPPMSTEM(RexxStem::newRexx),

CPPMSTR(RexxString::lengthRexx),          /* String methods                    */
CPPMSTR(RexxString::concatRexx),
CPPMSTR(RexxString::concatBlank),
CPPMSTR(RexxString::concatWith),
CPPMSTR(RexxString::equal),
CPPMSTR(RexxString::notEqual),
CPPMSTR(RexxString::isLessThan),
CPPMSTR(RexxString::isGreaterThan),
CPPMSTR(RexxString::isGreaterOrEqual),
CPPMSTR(RexxString::isLessOrEqual),
CPPMSTR(RexxString::strictEqual),
CPPMSTR(RexxString::strictNotEqual),
CPPMSTR(RexxString::strictLessThan),
CPPMSTR(RexxString::strictGreaterThan),
CPPMSTR(RexxString::strictGreaterOrEqual),
CPPMSTR(RexxString::strictLessOrEqual),
CPPMSTR(RexxString::plus),
CPPMSTR(RexxString::minus),
CPPMSTR(RexxString::multiply),
CPPMSTR(RexxString::divide),
CPPMSTR(RexxString::integerDivide),
CPPMSTR(RexxString::remainder),
CPPMSTR(RexxString::power),
CPPMSTR(RexxString::abs),
CPPMSTR(RexxString::sign),
CPPMSTR(RexxString::notOp),
CPPMSTR(RexxString::andOp),
CPPMSTR(RexxString::orOp),
CPPMSTR(RexxString::xorOp),
CPPMSTR(RexxString::Max),
CPPMSTR(RexxString::Min),
CPPMSTR(RexxString::isInteger),
CPPMSTR(RexxString::upperRexx),
CPPMSTR(RexxString::lowerRexx),

                                          /* All BIF methods start here.  They */
                                          /*  will be arranged according to the*/
                                          /*  they are defined in.             */

                                          /* following methods are in OKBSUBS  */
CPPMSTR(RexxString::center),
CPPMSTR(RexxString::delstr),
CPPMSTR(RexxString::insert),
CPPMSTR(RexxString::left),
CPPMSTR(RexxString::overlay),
CPPMSTR(RexxString::reverse),
CPPMSTR(RexxString::right),
CPPMSTR(RexxString::strip),
CPPMSTR(RexxString::substr),
CPPMSTR(RexxString::subchar),

                                          /* following methods are in OKBWORD  */
CPPMSTR(RexxString::delWord),
CPPMSTR(RexxString::space),
CPPMSTR(RexxString::subWord),
CPPMSTR(RexxString::word),
CPPMSTR(RexxString::wordIndex),
CPPMSTR(RexxString::wordLength),
CPPMSTR(RexxString::wordPos),
CPPMSTR(RexxString::caselessWordPos),
CPPMSTR(RexxString::words),

                                          /* following methods are in OKBMISC  */

CPPMSTR(RexxString::changeStr),
CPPMSTR(RexxString::caselessChangeStr),
CPPMSTR(RexxString::countStrRexx),
CPPMSTR(RexxString::caselessCountStrRexx),
CPPMSTR(RexxString::abbrev),
CPPMSTR(RexxString::caselessAbbrev),
CPPMSTR(RexxString::compare),
CPPMSTR(RexxString::caselessCompare),
CPPMSTR(RexxString::copies),
CPPMSTR(RexxString::dataType),
CPPMSTR(RexxString::lastPosRexx),
CPPMSTR(RexxString::posRexx),
CPPMSTR(RexxString::caselessLastPosRexx),
CPPMSTR(RexxString::caselessPosRexx),
CPPMSTR(RexxString::translate),
CPPMSTR(RexxString::verify),

                                          /* following methods are in OKBBITS  */
CPPMSTR(RexxString::bitAnd),
CPPMSTR(RexxString::bitOr),
CPPMSTR(RexxString::bitXor),

                                          /* following methods are in OKBCONV  */

CPPMSTR(RexxString::b2x),
CPPMSTR(RexxString::c2d),
CPPMSTR(RexxString::c2x),
CPPMSTR(RexxString::d2c),
CPPMSTR(RexxString::d2x),
CPPMSTR(RexxString::x2b),
CPPMSTR(RexxString::x2c),
CPPMSTR(RexxString::x2d),
CPPMSTR(RexxString::format),
CPPMSTR(RexxString::trunc),
CPPMSTR(RexxString::x2dC2d),
CPPMSTR(RexxString::encodeBase64),
CPPMSTR(RexxString::decodeBase64),

CPPMSTR(RexxString::match),
CPPMSTR(RexxString::caselessMatch),
CPPMSTR(RexxString::matchChar),
CPPMSTR(RexxString::caselessMatchChar),
CPPMSTR(RexxString::equals),
CPPMSTR(RexxString::caselessEquals),
CPPMSTR(RexxString::compareToRexx),
CPPMSTR(RexxString::caselessCompareToRexx),
                                          /* End of BIF methods                */
CPPMSTR(RexxString::makeArray),

CPPMSTRCL(RexxStringClass::newRexx),
CPPMMUTBCL(RexxMutableBufferClass::newRexx),
CPPMMUTB(RexxMutableBuffer::lengthRexx),
CPPMMUTB(RexxMutableBuffer::requestString),
CPPMMUTB(RexxMutableBuffer::requestRexx),
CPPMMUTB(RexxMutableBuffer::append),
CPPMMUTB(RexxMutableBuffer::insert),
CPPMMUTB(RexxMutableBuffer::overlay),
CPPMMUTB(RexxMutableBuffer::mydelete),
CPPMMUTB(RexxMutableBuffer::substr),
CPPMMUTB(RexxMutableBuffer::subchar),
CPPMMUTB(RexxMutableBuffer::posRexx),
CPPMMUTB(RexxMutableBuffer::lastPos),
CPPMMUTB(RexxMutableBuffer::getBufferSize),
CPPMMUTB(RexxMutableBuffer::setBufferSize),
CPPMMUTB(RexxMutableBuffer::uninitMB),

CPPMSUP(RexxSupplier::available),         /* Supplier methods                  */
CPPMSUP(RexxSupplier::next),
CPPMSUP(RexxSupplier::value),
CPPMSUP(RexxSupplier::index),
CPPMSUP(RexxSupplier::initRexx),

CPPMSUPCL(RexxSupplierClass::newRexx),

                                       /* Table methods                     */
CPPMHC(RexxHashTableCollection::removeRexx),
CPPMHC(RexxHashTableCollection::getRexx),
CPPMHC(RexxHashTableCollection::put),
CPPMHC(RexxHashTableCollection::add),
CPPMHC(RexxHashTableCollection::allAt),
CPPMHC(RexxHashTableCollection::hasIndex),
CPPMHC(RexxHashTableCollection::merge),
CPPMHC(RexxHashTableCollection::supplier),
CPPMHC(RexxHashTableCollection::allItems),
CPPMHC(RexxHashTableCollection::allIndexes),
CPPMHC(RexxHashTableCollection::empty),
CPPMHC(RexxHashTableCollection::isEmpty),
CPPMHC(RexxHashTableCollection::indexRexx),
CPPMHC(RexxHashTableCollection::hasItem),
CPPMHC(RexxHashTableCollection::removeItem),

CPPMTBL(RexxTable::itemsRexx),
CPPMTBL(RexxTable::newRexx),

CPPMREL(RexxRelation::put),               /* Relation methods                  */
CPPMREL(RexxRelation::removeItemRexx),
CPPMREL(RexxRelation::allIndex),
CPPMREL(RexxRelation::itemsRexx),
CPPMREL(RexxRelation::supplier),
CPPMREL(RexxRelation::hasItem),

CPPMREL(RexxRelation::newRexx),

CPPMMEM(RexxMemory::reclaim),             /* Memory methods                    */
CPPMMEM(RexxMemory::setParms),
CPPMMEM(RexxMemory::dump),
CPPMMEM(RexxMemory::setDump),
CPPMMEM(RexxMemory::gutCheck),

CPPMLOC(RexxLocal::local),                /* the .local environment methods    */
CPPMLOC(RexxLocal::runProgram),
CPPMLOC(RexxLocal::callProgram),
CPPMLOC(RexxLocal::callString),

CPPMSND(RexxSender::getPid),              /* the .sender class methods         */
CPPMSND(RexxSender::sendMessage),

CPPMSRV(RexxServer::messageWait),         /* the .server class methods */
NULL                                   /* final terminating method          */
};

LONG resolveExportedMethod(
    PCPPM   targetMethod )             /* method needed to resolve          */
/******************************************************************************/
/* Function:  Resolve a method address to numeric index                       */
/******************************************************************************/
{
  LONG   i;                            /* loop counter                      */

  if (targetMethod == NULL)            /* unresolved method address?        */
                                       /* this is a bad error               */
    logic_error("Unresolved exported method");
                                       /* scan the method address table     */
  for (i = 0; ExportedMethods[i] != NULL; i++) {
                                       /* found the one we want?            */
    if (ExportedMethods[i] == targetMethod)
      return i;                        /* return the index                  */
  }
                                       /* this is a bad error               */
  logic_error("Unresolved exported method");
  return 0;                            /* needs a return value              */
}

/*****************************************************************************/
/* Initialisation and build of the Object REXX image                         */
/*****************************************************************************/
RexxString * kernel_name (char* value);
void         kernelBuildVirtualFunctionTableArray(void);
void         createStrings(void);      /* create "name" strings             */
extern RexxDirectory *ProcessLocalEnv; /* process local environment (.local)*/

void kernelInit (void)
/******************************************************************************/
/* Function:  Initialize the kernel on image build                            */
/******************************************************************************/
{
                                       /* go build the VFT Array            */
  kernelBuildVirtualFunctionTableArray();
  memoryCreate();                      /* create initial memory stuff       */
                                       /* RexxNumberString                  */
  CLASS_CREATE(NumberString, RexxNumberStringClass);
  CLASS_CREATE(Array, RexxClass);      /* RexxArray                         */
  TheNullArray = new_array(0);         /* set up a null array               */

  CLASS_CREATE(Directory, RexxClass);  /* RexxDirectory                     */
  TheEnvironment = new_directory();    /* create the environment directory  */
                                       /* setup OREF_ENV as the mark start  */
                                       /* point                             */
  memoryObject.setMarkTable((RexxTable *)TheEnvironment);
  TheKernel = new_directory();         /* now add the kernel and system     */
  TheSystem = new_directory();         /* directories                       */
                                       /* Indicate these objects will not be*/
                                       /*  moved to another system, rather  */
                                       /*  will re-establish themselves on  */
                                       /*  the remote system.               */
  TheEnvironment->header |= MakeProxyObject;
  TheKernel->header |= MakeProxyObject;
  TheSystem->header |= MakeProxyObject;

  ThePublicRoutines = new_directory();
  TheStaticRequires = new_directory();

  createStrings();                     /* create all of the OREF_ strings   */
                                       /* RexxMethod                        */
  CLASS_CREATE(Method, RexxMethodClass);
  nmethod_create();                    /* RexxNativeCode                    */
                                       /* create the som method template    */
  TheGenericSomMethod = new RexxSOMCode (TRUE);
  CLASS_CREATE(Queue, RexxClass);      /* RexxQueue                         */
  TheNullPointer = new_pointer(NULL);  /* a NULL pointer object             */
  CLASS_CREATE(List, RexxListClass);   /* RexxList                          */
  CLASS_CREATE(Stem, RexxClass);       /* RexxStem                          */
  activity_create();                   /* RexxActivity                      */
  CLASS_CREATE(Supplier, RexxClass);   /* RexxSupplier                      */
  CLASS_CREATE(Envelope, RexxClass);   /* RexxEnvelope                      */
  CLASS_CREATE(Message, RexxClass);    /* RexxMessage                       */
  CLASS_CREATE(MutableBuffer, RexxClass);

                                       /* build the common retriever tables */
  TheCommonRetrievers = (RexxDirectory *)new_directory();
                                       /* add all of the special variables  */
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_SELF, VARIABLE_SELF), OREF_SELF);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_SUPER, VARIABLE_SUPER), OREF_SUPER);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_SIGL, VARIABLE_SIGL), OREF_SIGL);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_RC, VARIABLE_RC), OREF_RC);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_RESULT, VARIABLE_RESULT), OREF_RESULT);
  memoryObject.enableOrefChecks();     /* enable setCheckOrefs...           */
}

LONG resolveExportedMethod(PCPPM);

RexxMethod * createKernelMethod(
    PCPPM           entryPoint,        /* method entry point                */
    LONG            arguments)         /* count of arguments                */
/******************************************************************************/
/* Function:  Create a primitive, C++ method object                           */
/******************************************************************************/
{
                                       /* create a new kernel method        */
  return new RexxMethod(resolveExportedMethod(entryPoint), entryPoint, arguments, OREF_NULL);
}

RexxMethod * createProtectedKernelMethod(
    PCPPM           entryPoint,        /* method entry point                */
    LONG            arguments)         /* count of arguments                */
/******************************************************************************/
/* Function:  Create a primitive, C++ method object                           */
/******************************************************************************/
{
  RexxMethod *method;                  /* created method                    */
                                       /* create a new kernel method        */
  method = new RexxMethod(resolveExportedMethod(entryPoint), entryPoint, arguments, OREF_NULL);
  method->setProtected();              /* make this protected               */
  return method;                       /* return the method                 */
}

RexxMethod * createPrivateKernelMethod(
    PCPPM           entryPoint,        /* method entry point                */
    LONG            arguments)         /* count of arguments                */
/******************************************************************************/
/* Function:  Create a primitive, C++ method object                           */
/******************************************************************************/
{
  RexxMethod *method;                  /* created method                    */
                                       /* create a new kernel method        */
  method = new RexxMethod(resolveExportedMethod(entryPoint), entryPoint, arguments, OREF_NULL);
  method->setProtected();              /* make this protected               */
  method->setPrivate();                /* and private                       */
  return method;                       /* return the method                 */
}

void defineKernelMethod(
    char          * name,              /* ASCII-Z name for the method       */
    RexxBehaviour * behaviour,         /* behaviour to use                  */
    PCPPM           entryPoint,        /* method's entry point              */
    LONG            arguments )        /* count of arguments                */
/******************************************************************************/
/* Function:  Add a C++ method to an object's behaviour                       */
/******************************************************************************/
{
  behaviour->define(kernel_name(name), createKernelMethod(entryPoint, arguments));
}

void defineProtectedKernelMethod(
    char          * name,              /* ASCII-Z name for the method       */
    RexxBehaviour * behaviour,         /* behaviour to use                  */
    PCPPM           entryPoint,        /* method's entry point              */
    LONG            arguments )        /* count of arguments                */
/******************************************************************************/
/* Function:  Add a C++ method to an object's behaviour                       */
/******************************************************************************/
{
  behaviour->define(kernel_name(name), createProtectedKernelMethod(entryPoint, arguments));
}

void definePrivateKernelMethod(
    char          * name,              /* ASCII-Z name for the method       */
    RexxBehaviour * behaviour,         /* behaviour to use                  */
    PCPPM           entryPoint,        /* method's entry point              */
    LONG            arguments )        /* count of arguments                */
/******************************************************************************/
/* Function:  Add a C++ method to an object's behaviour                       */
/******************************************************************************/
{
  behaviour->define(kernel_name(name), createPrivateKernelMethod(entryPoint, arguments));
}

bool kernel_setup (void)
/******************************************************************************/
/* Function:  Build the Object REXX image, including hand building the        */
/*            base set of REXX classes.                                       */
/******************************************************************************/
{
  RexxString *symb;                    /* symbolic name for added methods   */
  RexxString *programName;             /* name of the image file            */
  RexxMethod *meth;                    /* added method object               */
  RexxDirectory *kernel_methods;       /* table of exported kernel_methods  */
/******************************************************************************/
/* The following Rexx classes that are exposed to the users are set up as    */
/* subclassable classes.                                                     */
/*****************************************************************************/

     /* The NEW method is exposed for the CLASS class behaviour.             */
     /* The CLASS class needs the methods of the CLASS instance behaviour    */
     /* so the instance behaviour methods are also in the CLASS class mdict. */
     /*                                                                      */
     /* Also Since the CLASS class needs OBJECT instance methods the         */
     /* OBJECT class is setup. Then the class method SUBCLASSABLE can be     */
     /* invoked on OBJECT then CLASS and then all the subclassable classes.  */

                                       /* add the Rexx class NEW method     */
  defineKernelMethod(CHAR_NEW, TheClassClassBehaviour, CPPMC(RexxClass::newRexx), A_COUNT);

                                       /* set the scope of the method to    */
                                       /* the CLASS scope                   */
  TheClassClassBehaviour->setMethodDictionaryScope(TheClassClass);

                                       /* add the instance methods to the   */
                                       /* class's instance mdict            */
  defineProtectedKernelMethod(CHAR_BASECLASS       ,TheClassBehaviour, CPPMC(RexxClass::getBaseClass), 0);
  defineProtectedKernelMethod(CHAR_DEFINE          ,TheClassBehaviour, CPPMC(RexxClass::defineMethod), 2);
  defineProtectedKernelMethod(CHAR_DEFINE_METHODS  ,TheClassBehaviour, CPPMC(RexxClass::defineMethods), 1);
  defineProtectedKernelMethod(CHAR_DELETE ,TheClassBehaviour, CPPMC(RexxClass::deleteMethod), 1);
  defineKernelMethod(CHAR_ENHANCED        ,TheClassBehaviour, CPPMC(RexxClass::enhanced), A_COUNT);
  defineKernelMethod(CHAR_ID              ,TheClassBehaviour, CPPMC(RexxClass::getId), 0);
  defineKernelMethod(CHAR_INHERIT         ,TheClassBehaviour, CPPMC(RexxClass::inherit), 2);
  defineProtectedKernelMethod(CHAR_METACLASS       ,TheClassBehaviour, CPPMC(RexxClass::getMetaClass), 0);
  defineKernelMethod(CHAR_METHOD          ,TheClassBehaviour, CPPMC(RexxClass::method), 1);
  defineKernelMethod(CHAR_METHODS         ,TheClassBehaviour, CPPMC(RexxClass::methods), 1);
  defineKernelMethod(CHAR_MIXINCLASS      ,TheClassBehaviour, CPPMC(RexxClass::mixinclass), 3);
  defineKernelMethod(CHAR_QUERYMIXINCLASS ,TheClassBehaviour, CPPMC(RexxClass::queryMixinClass), 0);
  defineKernelMethod(CHAR_SUBCLASS        ,TheClassBehaviour, CPPMC(RexxClass::subclass), 3);
  defineProtectedKernelMethod(CHAR_SUBCLASSES      ,TheClassBehaviour, CPPMC(RexxClass::getSubClasses), 0);
  defineProtectedKernelMethod(CHAR_SUPERCLASSES    ,TheClassBehaviour, CPPMC(RexxClass::getSuperClasses), 0);
  defineProtectedKernelMethod(CHAR_SUPERCLASS      ,TheClassBehaviour, CPPMC(RexxClass::getSuperClass), 0);
  defineProtectedKernelMethod(CHAR_UNINHERIT       ,TheClassBehaviour, CPPMC(RexxClass::uninherit), 1);
                                       /* Class operator methods....        */
  defineKernelMethod(CHAR_EQUAL                  ,TheClassBehaviour, CPPMC(RexxClass::equal), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL           ,TheClassBehaviour, CPPMC(RexxClass::strictEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL        ,TheClassBehaviour, CPPMC(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN   ,TheClassBehaviour, CPPMC(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN   ,TheClassBehaviour, CPPMC(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL ,TheClassBehaviour, CPPMC(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_ISSUBCLASSOF           ,TheClassBehaviour, CPPMC(RexxClass::isSubclassOf), 1);
  defineProtectedKernelMethod(CHAR_SHRIEKREXXDEFINED,TheClassBehaviour, CPPMC(RexxClass::setRexxDefined), 0);
  defineKernelMethod(CHAR_DEFAULTNAME            ,TheClassBehaviour, CPPM(RexxClass::defaultNameRexx), 0);
  // this is explicitly inserted into the class behaviour because it gets used
  // prior to the instance behavior merges.
  defineKernelMethod(CHAR_HASHCODE               ,TheClassBehaviour, CPPM(RexxObject::hashCode), 0);

                                       /* set the scope of the methods to   */
                                       /* the CLASS scope                   */
  TheClassBehaviour->setMethodDictionaryScope(TheClassClass);

     /************************************************************************/
     /*                                                                      */
     /* The OBJECT class and instance behaviour mdict's are needed next      */
     /*                                                                      */
     /************************************************************************/

                                       /* add the NEW method to the OBJECT  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheObjectClassBehaviour, CPPM(RexxObject::newRexx), A_COUNT);

                                       /* set the scope of the method to    */
                                       /* the OBJECT scope                  */
  TheObjectClassBehaviour->setMethodDictionaryScope(TheObjectClass);

                                       /* now set up the instance behaviour */
                                       /* mdict                             */
  defineKernelMethod(CHAR_INIT                   ,TheObjectBehaviour, CPPM(RexxObject::init), 0);
  defineKernelMethod(CHAR_EQUAL                  ,TheObjectBehaviour, CPPM(RexxObject::equal), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL           ,TheObjectBehaviour, CPPM(RexxObject::strictEqual), 1);
  defineKernelMethod(CHAR_HASHCODE               ,TheObjectBehaviour, CPPM(RexxObject::hashCode), 0);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL        ,TheObjectBehaviour, CPPM(RexxObject::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN   ,TheObjectBehaviour, CPPM(RexxObject::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN   ,TheObjectBehaviour, CPPM(RexxObject::notEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL ,TheObjectBehaviour, CPPM(RexxObject::strictNotEqual), 1);
  defineKernelMethod(CHAR_NULLSTRING             ,TheObjectBehaviour, CPPM(RexxObject::concatRexx), 1);
  defineKernelMethod(CHAR_BLANK                  ,TheObjectBehaviour, CPPM(RexxObject::concatBlank), 1);
  defineKernelMethod(CHAR_CONCATENATE            ,TheObjectBehaviour, CPPM(RexxObject::concatRexx), 1);
  defineKernelMethod(CHAR_COPY                   ,TheObjectBehaviour, CPPM(RexxObject::copyRexx), 0);
  defineKernelMethod(CHAR_CLASS                  ,TheObjectBehaviour, CPPM(RexxObject::classObject), 0);
  defineKernelMethod(CHAR_HASMETHOD              ,TheObjectBehaviour, CPPM(RexxObject::hasMethodRexx), 1);
  defineKernelMethod(CHAR_DEFAULTNAME            ,TheObjectBehaviour, CPPM(RexxObject::defaultNameRexx), 0);
  defineKernelMethod(CHAR_OBJECTNAME             ,TheObjectBehaviour, CPPM(RexxObject::objectName), 0);
  defineKernelMethod(CHAR_OBJECTNAMEEQUALS       ,TheObjectBehaviour, CPPM(RexxObject::objectNameEquals), 1);
  defineKernelMethod(CHAR_REQUEST                ,TheObjectBehaviour, CPPM(RexxObject::requestRexx), 1);
  defineKernelMethod(CHAR_START                  ,TheObjectBehaviour, CPPM(RexxObject::start), A_COUNT);
  defineKernelMethod(CHAR_STRING                 ,TheObjectBehaviour, CPPM(RexxObject::stringRexx), 0);
  defineKernelMethod(CHAR_ISINSTANCEOF           ,TheObjectBehaviour, CPPM(RexxObject::isInstanceOfRexx), 1);
  defineKernelMethod(CHAR_ISA                    ,TheObjectBehaviour, CPPM(RexxObject::isInstanceOfRexx), 1);
  defineKernelMethod(CHAR_INSTANCEMETHOD         ,TheObjectBehaviour, CPPM(RexxObject::instanceMethodRexx), 1);
  defineKernelMethod(CHAR_INSTANCEMETHODS        ,TheObjectBehaviour, CPPM(RexxObject::instanceMethodsRexx), 1);
  defineKernelMethod(CHAR_IDENTITYHASH           ,TheObjectBehaviour, CPPM(RexxObject::identityHashRexx), 0);
  definePrivateKernelMethod(CHAR_RUN             ,TheObjectBehaviour, CPPM(RexxObject::run), A_COUNT);
  definePrivateKernelMethod(CHAR_SETMETHOD       ,TheObjectBehaviour, CPPM(RexxObject::setMethod), 3);
  definePrivateKernelMethod(CHAR_UNSETMETHOD     ,TheObjectBehaviour, CPPM(RexxObject::unsetMethod), 1);
                                       /* set the scope of the methods to   */
                                       /* the OBJECT scope                  */
  TheObjectBehaviour->setMethodDictionaryScope(TheObjectClass);
                                       /* Now call the class subclassable   */
                                       /* method for OBJECT then CLASS      */
  TheObjectClass->subClassable("Object", true);
  TheClassClass->subClassable("Class", true);

  /************************************** The rest of the classes can now be */
  /************************************** set up.                            */

  /***************************************************************************/
  /*           ARRAY                                                         */
  /***************************************************************************/

  defineKernelMethod(CHAR_NEW, TheArrayClassBehaviour, CPPMC(RexxArray::newRexx), A_COUNT);
  defineKernelMethod(CHAR_OF,  TheArrayClassBehaviour, CPPMC(RexxArray::of), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheArrayClassBehaviour->setMethodDictionaryScope(TheArrayClass);

  defineKernelMethod(CHAR_BRACKETS     ,TheArrayBehaviour, CPPMC(RexxArray::getRexx), A_COUNT);
  defineKernelMethod(CHAR_BRACKETSEQUAL,TheArrayBehaviour, CPPMC(RexxArray::putRexx), A_COUNT);
  defineKernelMethod(CHAR_AT           ,TheArrayBehaviour, CPPMC(RexxArray::getRexx), A_COUNT);
  defineKernelMethod(CHAR_DIMENSION    ,TheArrayBehaviour, CPPMC(RexxArray::dimension), 1);
  defineKernelMethod(CHAR_HASINDEX     ,TheArrayBehaviour, CPPMC(RexxArray::hasIndexRexx), A_COUNT);
  defineKernelMethod(CHAR_ITEMS        ,TheArrayBehaviour, CPPMA(RexxArray::items), 0);
  defineKernelMethod(CHAR_MAKEARRAY    ,TheArrayBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_PUT          ,TheArrayBehaviour, CPPMC(RexxArray::putRexx), A_COUNT);
  defineKernelMethod(CHAR_REMOVE       ,TheArrayBehaviour, CPPMC(RexxArray::removeRexx), A_COUNT);
  defineKernelMethod(CHAR_SECTION      ,TheArrayBehaviour, CPPMA(RexxArray::sectionRexx), 2);
  defineKernelMethod(CHAR_SIZE         ,TheArrayBehaviour, CPPMA(RexxArray::sizeRexx), 0);
  defineKernelMethod(CHAR_SUPPLIER     ,TheArrayBehaviour, CPPMA(RexxArray::supplier), 0);
  defineKernelMethod(CHAR_FIRST        ,TheArrayBehaviour, CPPMA(RexxArray::firstRexx), 0);
  defineKernelMethod(CHAR_LAST         ,TheArrayBehaviour, CPPMA(RexxArray::lastRexx), 0);
  defineKernelMethod(CHAR_NEXT         ,TheArrayBehaviour, CPPMA(RexxArray::nextRexx), A_COUNT);
  defineKernelMethod(CHAR_PREVIOUS     ,TheArrayBehaviour, CPPMA(RexxArray::previousRexx), A_COUNT);
  defineKernelMethod(CHAR_APPEND       ,TheArrayBehaviour, CPPMA(RexxArray::append), 1);
  defineKernelMethod(CHAR_MAKESTRING   ,TheArrayBehaviour, CPPMA(RexxArray::makeString), 2);
  defineKernelMethod(CHAR_TOSTRING     ,TheArrayBehaviour, CPPMA(RexxArray::toString), 2);
  defineKernelMethod(CHAR_ALLINDEXES   ,TheArrayBehaviour, CPPMA(RexxArray::allIndexes), 0);
  defineKernelMethod(CHAR_ALLITEMS     ,TheArrayBehaviour, CPPMA(RexxArray::allItems), 0);
  defineKernelMethod(CHAR_EMPTY        ,TheArrayBehaviour, CPPMA(RexxArray::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      ,TheArrayBehaviour, CPPMA(RexxArray::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        ,TheArrayBehaviour, CPPMA(RexxArray::index), 1);
  defineKernelMethod(CHAR_HASITEM      ,TheArrayBehaviour, CPPMA(RexxArray::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   ,TheArrayBehaviour, CPPMA(RexxArray::removeItem), 1);
  defineKernelMethod(CHAR_SORT         ,TheArrayBehaviour, CPPMA(RexxArray::sortRexx), 0);
  defineKernelMethod(CHAR_SORTWITH     ,TheArrayBehaviour, CPPMA(RexxArray::sortWithRexx), 1);
  defineKernelMethod(CHAR_STABLESORT   ,TheArrayBehaviour, CPPMA(RexxArray::stableSortRexx), 0);
  defineKernelMethod(CHAR_STABLESORTWITH ,TheArrayBehaviour, CPPMA(RexxArray::stableSortWithRexx), 1);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheArrayBehaviour->setMethodDictionaryScope(TheArrayClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheArrayClass->subClassable("Array", false);

  /***************************************************************************/
  /*           DIRECTORY                                                     */
  /***************************************************************************/

  defineKernelMethod(CHAR_NEW           , TheDirectoryClassBehaviour, CPPMD(RexxDirectory::newRexx), A_COUNT);

                                       /* set the scope of the method to    */
                                       /* this classes oref                 */
  TheDirectoryClassBehaviour->setMethodDictionaryScope(TheDirectoryClass);

                                       /* add the instance methods          */
  defineKernelMethod(CHAR_BRACKETS      , TheDirectoryBehaviour, CPPMD(RexxDirectory::atRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL , TheDirectoryBehaviour, CPPMD(RexxDirectory::put), 2);
  defineKernelMethod(CHAR_AT            , TheDirectoryBehaviour, CPPMD(RexxDirectory::atRexx), 1);
  defineKernelMethod(CHAR_ENTRY         , TheDirectoryBehaviour, CPPMD(RexxDirectory::entryRexx), 1);
  defineKernelMethod(CHAR_HASENTRY      , TheDirectoryBehaviour, CPPMD(RexxDirectory::hasEntry), 1);
  defineKernelMethod(CHAR_HASINDEX      , TheDirectoryBehaviour, CPPMD(RexxDirectory::hasIndex), 1);
  defineKernelMethod(CHAR_ITEMS         , TheDirectoryBehaviour, CPPMD(RexxDirectory::itemsRexx), 0);
  defineKernelMethod(CHAR_MAKEARRAY     , TheDirectoryBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_ALLITEMS      , TheDirectoryBehaviour, CPPMD(RexxDirectory::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES    , TheDirectoryBehaviour, CPPMD(RexxDirectory::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY         , TheDirectoryBehaviour, CPPMD(RexxDirectory::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY       , TheDirectoryBehaviour, CPPMD(RexxDirectory::isEmpty), 0);
  defineKernelMethod(CHAR_PUT           , TheDirectoryBehaviour, CPPMD(RexxDirectory::put), 2);
  defineKernelMethod(CHAR_REMOVE        , TheDirectoryBehaviour, CPPMD(RexxDirectory::remove), 1);
  defineKernelMethod(CHAR_SETENTRY      , TheDirectoryBehaviour, CPPMD(RexxDirectory::setEntry), 2);
  defineProtectedKernelMethod(CHAR_SETMETHOD   , TheDirectoryBehaviour, CPPMD(RexxDirectory::setMethod), 2);
  defineKernelMethod(CHAR_SUPPLIER      , TheDirectoryBehaviour, CPPMD(RexxDirectory::supplier), 0);
  defineKernelMethod(CHAR_UNKNOWN       , TheDirectoryBehaviour, CPPM(RexxObject::unknownRexx), 2);
  defineProtectedKernelMethod(CHAR_UNSETMETHOD   , TheDirectoryBehaviour, CPPMD(RexxDirectory::remove), 1);
  defineKernelMethod(CHAR_INDEX        , TheDirectoryBehaviour, CPPMD(RexxDirectory::indexRexx), 1);
  defineKernelMethod(CHAR_HASITEM      , TheDirectoryBehaviour, CPPMD(RexxDirectory::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheDirectoryBehaviour, CPPMD(RexxDirectory::removeItem), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheDirectoryBehaviour->setMethodDictionaryScope(TheDirectoryClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheDirectoryClass->subClassable("Directory", false);

  /***************************************************************************/
  /*       ENVELOPE                                                          */
  /***************************************************************************/
                                       /* the envelope class is not         */
                                       /* externalized at this time         */
                                       /* and has no class methods          */

                                       /* add the instance method           */
  defineKernelMethod(CHAR_UNPACK, TheEnvelopeBehaviour, CPPME(RexxEnvelope::unpack), 0);

                                       /* set the scope of the method to    */
                                       /* this classes oref                 */
  TheEnvelopeBehaviour->setMethodDictionaryScope(TheEnvelopeClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheEnvelopeClass->subClassable("Envelope", true);

  /***************************************************************************/
  /*           LIST                                                          */
  /***************************************************************************/

                                       /* add the class behaviour methods   */
  defineKernelMethod(CHAR_NEW           , TheListClassBehaviour, CPPMLC(RexxListClass::newRexx), A_COUNT);
  defineKernelMethod(CHAR_OF            , TheListClassBehaviour, CPPMLC(RexxListClass::classOf), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheListClassBehaviour->setMethodDictionaryScope(TheListClass);

                                       /* add the instance behaviour methods*/
  defineKernelMethod(CHAR_BRACKETS     ,TheListBehaviour, CPPML(RexxList::value), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL,TheListBehaviour, CPPML(RexxList::put), 2);
  defineKernelMethod(CHAR_MAKEARRAY    ,TheListBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           ,TheListBehaviour, CPPML(RexxList::value), 1);
  defineKernelMethod(CHAR_FIRSTITEM    ,TheListBehaviour, CPPML(RexxList::firstItem), 0);
  defineKernelMethod(CHAR_HASINDEX     ,TheListBehaviour, CPPML(RexxList::hasIndex), 1);
  defineKernelMethod(CHAR_INSERT       ,TheListBehaviour, CPPML(RexxList::insertRexx), 2);
  defineKernelMethod(CHAR_ITEMS        ,TheListBehaviour, CPPML(RexxList::itemsRexx), 0);
  defineKernelMethod(CHAR_LASTITEM     ,TheListBehaviour, CPPML(RexxList::lastItem), 0);
  defineKernelMethod(CHAR_FIRST        ,TheListBehaviour, CPPML(RexxList::firstRexx), 0);
  defineKernelMethod(CHAR_LAST         ,TheListBehaviour, CPPML(RexxList::lastRexx), 0);
  defineKernelMethod(CHAR_NEXT         ,TheListBehaviour, CPPML(RexxList::next), 1);
  defineKernelMethod(CHAR_PREVIOUS     ,TheListBehaviour, CPPML(RexxList::previous), 1);
  defineKernelMethod(CHAR_PUT          ,TheListBehaviour, CPPML(RexxList::put), 2);
  defineKernelMethod(CHAR_REMOVE       ,TheListBehaviour, CPPML(RexxList::remove), 1);
  defineKernelMethod(CHAR_SECTION      ,TheListBehaviour, CPPML(RexxList::section), 2);
  defineKernelMethod(CHAR_SUPPLIER     ,TheListBehaviour, CPPML(RexxList::supplier), 0);
  defineKernelMethod(CHAR_APPEND       ,TheListBehaviour, CPPMA(RexxList::append), 1);
  defineKernelMethod(CHAR_ALLITEMS     ,TheListBehaviour, CPPML(RexxList::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   ,TheListBehaviour, CPPML(RexxList::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        ,TheListBehaviour, CPPML(RexxList::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      ,TheListBehaviour, CPPML(RexxList::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        ,TheListBehaviour, CPPML(RexxList::index), 1);
  defineKernelMethod(CHAR_HASITEM      ,TheListBehaviour, CPPML(RexxList::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   ,TheListBehaviour, CPPML(RexxList::removeItem), 1);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheListBehaviour->setMethodDictionaryScope(TheListClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheListClass->subClassable("List", false);

  /***************************************************************************/
  /*           MESSAGE                                                       */
  /***************************************************************************/

                                       /* Define the NEW method in the      */
                                       /* class behaviour mdict             */
  defineKernelMethod(CHAR_NEW      , TheMessageClassBehaviour, CPPMSG(RexxMessage::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMessageClassBehaviour->setMethodDictionaryScope(TheMessageClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */

  defineKernelMethod(CHAR_COMPLETED, TheMessageBehaviour, CPPMSG(RexxMessage::completed), 0);
  defineKernelMethod(CHAR_HASERROR,  TheMessageBehaviour, CPPMSG(RexxMessage::hasError), 0);
  defineKernelMethod(CHAR_NOTIFY   , TheMessageBehaviour, CPPMSG(RexxMessage::notify), 1);
  defineKernelMethod(CHAR_RESULT   , TheMessageBehaviour, CPPMSG(RexxMessage::result), 0);
  defineKernelMethod(CHAR_TARGET   , TheMessageBehaviour, CPPMSG(RexxMessage::messageTarget), 0);
  defineKernelMethod(CHAR_MESSAGENAME  , TheMessageBehaviour, CPPMSG(RexxMessage::messageName), 0);
  defineKernelMethod(CHAR_ARGUMENTS  , TheMessageBehaviour, CPPMSG(RexxMessage::arguments), 0);
  defineKernelMethod(CHAR_ERRORCONDITION , TheMessageBehaviour, CPPMSG(RexxMessage::errorCondition), 0);
  defineKernelMethod(CHAR_SEND     , TheMessageBehaviour, CPPMSG(RexxMessage::send), 1);
  defineKernelMethod(CHAR_START    , TheMessageBehaviour, CPPMSG(RexxMessage::start), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMessageBehaviour->setMethodDictionaryScope(TheMessageClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheMessageClass->subClassable("Message", true);

  /***************************************************************************/
  /*           METHOD                                                        */
  /***************************************************************************/

                                       /* Add the NEW methods to the        */
                                       /* class behaviour                   */
  defineKernelMethod(CHAR_NEW     , TheMethodClassBehaviour, CPPMTDC(RexxMethodClass::newRexx), A_COUNT);
  defineKernelMethod(CHAR_NEWFILE , TheMethodClassBehaviour, CPPMTDC(RexxMethodClass::newFileRexx), 1);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMethodClassBehaviour->setMethodDictionaryScope(TheMethodClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_INTERFACE    ,TheMethodBehaviour, CPPMTD(RexxMethod::getInterface), 0);
  defineKernelMethod(CHAR_SETINTERFACE ,TheMethodBehaviour, CPPMTD(RexxMethod::setInterface), 1);
  defineKernelMethod(CHAR_SETUNGUARDED ,TheMethodBehaviour, CPPMTD(RexxMethod::setUnGuardedRexx), 0);
  defineKernelMethod(CHAR_SETGUARDED   ,TheMethodBehaviour, CPPMTD(RexxMethod::setGuardedRexx), 0);
  defineKernelMethod(CHAR_SETPRIVATE   ,TheMethodBehaviour, CPPMTD(RexxMethod::setPrivateRexx), 0);
  defineKernelMethod(CHAR_ISGUARDED    ,TheMethodBehaviour, CPPMTD(RexxMethod::isGuardedRexx), 0);
  defineKernelMethod(CHAR_ISPRIVATE    ,TheMethodBehaviour, CPPMTD(RexxMethod::isPrivateRexx), 0);
  defineKernelMethod(CHAR_ISPROTECTED  ,TheMethodBehaviour, CPPMTD(RexxMethod::isProtectedRexx), 0);
  defineProtectedKernelMethod(CHAR_SETPROTECTED ,TheMethodBehaviour, CPPMTD(RexxMethod::setProtectedRexx), 0);
  defineProtectedKernelMethod(CHAR_SETSECURITYMANAGER ,TheMethodBehaviour, CPPMTD(RexxMethod::setSecurityManager), 1);
  defineKernelMethod(CHAR_SOURCE       ,TheMethodBehaviour, CPPMTD(RexxMethod::source), 0);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMethodBehaviour->setMethodDictionaryScope(TheMethodClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheMethodClass->subClassable("Method", true);

  /***************************************************************************/
  /*           QUEUE                                                         */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheQueueClassBehaviour, CPPMQ(RexxQueue::newRexx), A_COUNT);
  defineKernelMethod(CHAR_OF,  TheQueueClassBehaviour, CPPMQ(RexxQueue::ofRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheQueueClassBehaviour->setMethodDictionaryScope(TheQueueClass);

                                       /* Add the instance methods to the   */
                                       /* instance method mdict             */

  defineKernelMethod(CHAR_MAKEARRAY     ,TheQueueBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_ITEMS         ,TheQueueBehaviour, CPPML(RexxList::itemsRexx), 0);
  defineKernelMethod(CHAR_SUPPLIER      ,TheQueueBehaviour, CPPMQ(RexxQueue::supplier), 0);
  defineKernelMethod(CHAR_PUSH          ,TheQueueBehaviour, CPPMQ(RexxQueue::pushRexx), 1);
  defineKernelMethod(CHAR_PEEK          ,TheQueueBehaviour, CPPMQ(RexxQueue::peek), 0);
  defineKernelMethod(CHAR_PULL          ,TheQueueBehaviour, CPPMQ(RexxQueue::pullRexx), 0);
  defineKernelMethod(CHAR_QUEUE         ,TheQueueBehaviour, CPPMQ(RexxQueue::queueRexx), 1);
  defineKernelMethod(CHAR_BRACKETS      ,TheQueueBehaviour, CPPMQ(RexxQueue::at), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL ,TheQueueBehaviour, CPPMQ(RexxQueue::put), 2);
  defineKernelMethod(CHAR_AT            ,TheQueueBehaviour, CPPMQ(RexxQueue::at), 1);
  defineKernelMethod(CHAR_HASINDEX      ,TheQueueBehaviour, CPPMQ(RexxQueue::hasindex), 1);
  defineKernelMethod(CHAR_PUT           ,TheQueueBehaviour, CPPMQ(RexxQueue::put), 2);
  defineKernelMethod(CHAR_REMOVE        ,TheQueueBehaviour, CPPMQ(RexxQueue::remove), 1);
  defineKernelMethod(CHAR_APPEND        ,TheQueueBehaviour, CPPMQ(RexxQueue::append), 1);
  defineKernelMethod(CHAR_ALLITEMS      ,TheQueueBehaviour, CPPML(RexxList::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES    ,TheQueueBehaviour, CPPMQ(RexxQueue::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY         ,TheQueueBehaviour, CPPMQ(RexxList::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY       ,TheQueueBehaviour, CPPMQ(RexxList::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX         ,TheQueueBehaviour, CPPML(RexxQueue::index), 1);
  defineKernelMethod(CHAR_HASITEM       ,TheQueueBehaviour, CPPML(RexxList::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM    ,TheQueueBehaviour, CPPML(RexxList::removeItem), 1);
  defineKernelMethod(CHAR_FIRST         ,TheQueueBehaviour, CPPMQ(RexxQueue::firstRexx), 0);
  defineKernelMethod(CHAR_LAST          ,TheQueueBehaviour, CPPMQ(RexxQueue::lastRexx), 0);
  defineKernelMethod(CHAR_NEXT          ,TheQueueBehaviour, CPPMQ(RexxQueue::next), 1);
  defineKernelMethod(CHAR_PREVIOUS      ,TheQueueBehaviour, CPPMQ(RexxQueue::previous), 1);
  defineKernelMethod(CHAR_INSERT        ,TheQueueBehaviour, CPPML(RexxQueue::insert), 2);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheQueueBehaviour->setMethodDictionaryScope(TheQueueClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheQueueClass->subClassable("Queue", false);

  /***************************************************************************/
  /*           RELATION                                                      */
  /***************************************************************************/

                                       /* Add the NEW method to the         */
                                       /* class behaviour mdict             */
  defineKernelMethod(CHAR_NEW          , TheRelationClassBehaviour, CPPMREL(RexxRelation::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRelationClassBehaviour->setMethodDictionaryScope(TheRelationClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS     , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL, TheRelationBehaviour, CPPMREL(RexxRelation::put), 2);
  defineKernelMethod(CHAR_ALLAT        , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::allAt), 1);
  defineKernelMethod(CHAR_ALLINDEX     , TheRelationBehaviour, CPPMREL(RexxRelation::allIndex), 1);
  defineKernelMethod(CHAR_MAKEARRAY    , TheRelationBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_HASINDEX     , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::hasIndex), 1);
  defineKernelMethod(CHAR_HASITEM      , TheRelationBehaviour, CPPMREL(RexxRelation::hasItem), 2);
  defineKernelMethod(CHAR_INDEX        , TheRelationBehaviour, CPPMREL(RexxHashTableCollection::indexRexx), 1);
  defineKernelMethod(CHAR_ITEMS        , TheRelationBehaviour, CPPMREL(RexxRelation::itemsRexx), 1);
  defineKernelMethod(CHAR_PUT          , TheRelationBehaviour, CPPMREL(RexxRelation::put), 2);
  defineKernelMethod(CHAR_REMOVE       , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::removeRexx), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheRelationBehaviour, CPPMREL(RexxRelation::removeItemRexx), 2);
  defineKernelMethod(CHAR_SUPPLIER     , TheRelationBehaviour, CPPMREL(RexxRelation::supplier), 1);
  defineKernelMethod(CHAR_ALLITEMS     , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      , TheRelationBehaviour, CPPMHC(RexxHashTableCollection::isEmpty), 0);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRelationBehaviour->setMethodDictionaryScope(TheRelationClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheRelationClass->subClassable("Relation", false);

  /***************************************************************************/
  /*           STEM                                                          */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheStemClassBehaviour, CPPMSTEM(RexxStem::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStemClassBehaviour->setMethodDictionaryScope(TheStemClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS      ,TheStemBehaviour, CPPMSTEM(RexxStem::bracket), A_COUNT);
  defineKernelMethod(CHAR_BRACKETSEQUAL ,TheStemBehaviour, CPPMSTEM(RexxStem::bracketEqual), A_COUNT);
  defineKernelMethod(CHAR_AT            ,TheStemBehaviour, CPPMSTEM(RexxStem::bracket), A_COUNT);
  defineKernelMethod(CHAR_PUT           ,TheStemBehaviour, CPPMSTEM(RexxStem::bracketEqual), A_COUNT);
  defineKernelMethod(CHAR_MAKEARRAY     ,TheStemBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_REQUEST       ,TheStemBehaviour, CPPMSTEM(RexxStem::request), 1);
  defineKernelMethod(CHAR_SUPPLIER      ,TheStemBehaviour, CPPMSTEM(RexxStem::supplier), 0);
  defineKernelMethod(CHAR_ALLINDEXES    ,TheStemBehaviour, CPPMSTEM(RexxStem::allIndexes), 0);
  defineKernelMethod(CHAR_ALLITEMS      ,TheStemBehaviour, CPPMSTEM(RexxStem::allItems), 0);
  defineKernelMethod(CHAR_EMPTY         ,TheStemBehaviour, CPPMSTEM(RexxStem::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY       ,TheStemBehaviour, CPPMSTEM(RexxStem::isEmpty), 0);
  defineKernelMethod(CHAR_UNKNOWN       ,TheStemBehaviour, CPPM(RexxObject::unknownRexx), 2);

  defineKernelMethod(CHAR_ITEMS         ,TheStemBehaviour, CPPMSTEM(RexxStem::itemsRexx), 0);
  defineKernelMethod(CHAR_HASINDEX      ,TheStemBehaviour, CPPMSTEM(RexxStem::hasIndex), A_COUNT);
  defineKernelMethod(CHAR_REMOVE        ,TheStemBehaviour, CPPMSTEM(RexxStem::remove), A_COUNT);
  defineKernelMethod(CHAR_INDEX         ,TheStemBehaviour, CPPMSTEM(RexxStem::index), 1);
  defineKernelMethod(CHAR_HASITEM       ,TheStemBehaviour, CPPMSTEM(RexxStem::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM    ,TheStemBehaviour, CPPMSTEM(RexxStem::removeItem), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStemBehaviour->setMethodDictionaryScope(TheStemClass);

                                       /* delete these methods from stems by*/
                                       /* using .nil as the methobj         */
  TheStemBehaviour->define(kernel_name(CHAR_STRICT_EQUAL)          , (RexxMethod *)TheNilObject);
  TheStemBehaviour->define(kernel_name(CHAR_EQUAL)                 , (RexxMethod *)TheNilObject);
  TheStemBehaviour->define(kernel_name(CHAR_STRICT_BACKSLASH_EQUAL), (RexxMethod *)TheNilObject);
  TheStemBehaviour->define(kernel_name(CHAR_BACKSLASH_EQUAL)       , (RexxMethod *)TheNilObject);
  TheStemBehaviour->define(kernel_name(CHAR_LESSTHAN_GREATERTHAN)  , (RexxMethod *)TheNilObject);
  TheStemBehaviour->define(kernel_name(CHAR_GREATERTHAN_LESSTHAN)  , (RexxMethod *)TheNilObject);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheStemClass->subClassable("Stem", false);

  /***************************************************************************/
  /*           STRING                                                        */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheStringClassBehaviour, CPPMSTRCL(RexxStringClass::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStringClassBehaviour->setMethodDictionaryScope(TheStringClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_NULLSTRING                   ,TheStringBehaviour, CPPMSTR(RexxString::concatRexx), 1);
  defineKernelMethod(CHAR_BLANK                        ,TheStringBehaviour, CPPMSTR(RexxString::concatBlank), 1);
  defineKernelMethod(CHAR_CONCATENATE                  ,TheStringBehaviour, CPPMSTR(RexxString::concatRexx), 1);
  defineKernelMethod(CHAR_LENGTH                       ,TheStringBehaviour, CPPMSTR(RexxString::lengthRexx), 0);
  defineKernelMethod(CHAR_CENTER                       ,TheStringBehaviour, CPPMSTR(RexxString::center), 2);
  defineKernelMethod(CHAR_CENTRE                       ,TheStringBehaviour, CPPMSTR(RexxString::center), 2);
  defineKernelMethod(CHAR_DATATYPE                     ,TheStringBehaviour, CPPMSTR(RexxString::dataType), 1);
  defineKernelMethod(CHAR_DELSTR                       ,TheStringBehaviour, CPPMSTR(RexxString::delstr), 2);
  defineKernelMethod(CHAR_FORMAT                       ,TheStringBehaviour, CPPMSTR(RexxString::format), 4);
  defineKernelMethod(CHAR_INSERT                       ,TheStringBehaviour, CPPMSTR(RexxString::insert), 4);
  defineKernelMethod(CHAR_LEFT                         ,TheStringBehaviour, CPPMSTR(RexxString::left), 2);
  defineKernelMethod(CHAR_OVERLAY                      ,TheStringBehaviour, CPPMSTR(RexxString::overlay), 4);
  defineKernelMethod(CHAR_REVERSE                      ,TheStringBehaviour, CPPMSTR(RexxString::reverse), 0);
  defineKernelMethod(CHAR_RIGHT                        ,TheStringBehaviour, CPPMSTR(RexxString::right), 2);
  defineKernelMethod(CHAR_STRIP                        ,TheStringBehaviour, CPPMSTR(RexxString::strip), 2);
  defineKernelMethod(CHAR_SUBSTR                       ,TheStringBehaviour, CPPMSTR(RexxString::substr), 3);
  defineKernelMethod(CHAR_SUBCHAR                      ,TheStringBehaviour, CPPMSTR(RexxString::subchar), 1);
  defineKernelMethod(CHAR_DELWORD                      ,TheStringBehaviour, CPPMSTR(RexxString::delWord), 2);
  defineKernelMethod(CHAR_SPACE                        ,TheStringBehaviour, CPPMSTR(RexxString::space), 2);
  defineKernelMethod(CHAR_SUBWORD                      ,TheStringBehaviour, CPPMSTR(RexxString::subWord), 2);
  defineKernelMethod(CHAR_TRUNC                        ,TheStringBehaviour, CPPMSTR(RexxString::trunc), 1);
  defineKernelMethod(CHAR_WORD                         ,TheStringBehaviour, CPPMSTR(RexxString::word), 1);
  defineKernelMethod(CHAR_WORDINDEX                    ,TheStringBehaviour, CPPMSTR(RexxString::wordIndex), 1);
  defineKernelMethod(CHAR_WORDLENGTH                   ,TheStringBehaviour, CPPMSTR(RexxString::wordLength), 1);
  defineKernelMethod(CHAR_WORDPOS                      ,TheStringBehaviour, CPPMSTR(RexxString::wordPos), 2);
  defineKernelMethod(CHAR_CASELESSWORDPOS              ,TheStringBehaviour, CPPMSTR(RexxString::caselessWordPos), 2);
  defineKernelMethod(CHAR_WORDS                        ,TheStringBehaviour, CPPMSTR(RexxString::words), 0);
  defineKernelMethod(CHAR_ABBREV                       ,TheStringBehaviour, CPPMSTR(RexxString::abbrev), 2);
  defineKernelMethod(CHAR_CASELESSABBREV               ,TheStringBehaviour, CPPMSTR(RexxString::caselessAbbrev), 2);
  defineKernelMethod(CHAR_CHANGESTR                    ,TheStringBehaviour, CPPMSTR(RexxString::changeStr), 3);
  defineKernelMethod(CHAR_CASELESSCHANGESTR            ,TheStringBehaviour, CPPMSTR(RexxString::caselessChangeStr), 3);
  defineKernelMethod(CHAR_COMPARE                      ,TheStringBehaviour, CPPMSTR(RexxString::compare), 2);
  defineKernelMethod(CHAR_CASELESSCOMPARE              ,TheStringBehaviour, CPPMSTR(RexxString::caselessCompare), 2);
  defineKernelMethod(CHAR_COPIES                       ,TheStringBehaviour, CPPMSTR(RexxString::copies), 1);
  defineKernelMethod(CHAR_COUNTSTR                     ,TheStringBehaviour, CPPMSTR(RexxString::countStrRexx), 1);
  defineKernelMethod(CHAR_CASELESSCOUNTSTR             ,TheStringBehaviour, CPPMSTR(RexxString::caselessCountStrRexx), 1);
  defineKernelMethod(CHAR_LASTPOS                      ,TheStringBehaviour, CPPMSTR(RexxString::lastPosRexx), 2);
  defineKernelMethod(CHAR_POS                          ,TheStringBehaviour, CPPMSTR(RexxString::posRexx), 2);
  defineKernelMethod(CHAR_CASELESSLASTPOS              ,TheStringBehaviour, CPPMSTR(RexxString::caselessLastPosRexx), 2);
  defineKernelMethod(CHAR_CASELESSPOS                  ,TheStringBehaviour, CPPMSTR(RexxString::caselessPosRexx), 2);
  defineKernelMethod(CHAR_TRANSLATE                    ,TheStringBehaviour, CPPMSTR(RexxString::translate), 3);
  defineKernelMethod(CHAR_VERIFY                       ,TheStringBehaviour, CPPMSTR(RexxString::verify), 3);
  defineKernelMethod(CHAR_BITAND                       ,TheStringBehaviour, CPPMSTR(RexxString::bitAnd), 2);
  defineKernelMethod(CHAR_BITOR                        ,TheStringBehaviour, CPPMSTR(RexxString::bitOr), 2);
  defineKernelMethod(CHAR_BITXOR                       ,TheStringBehaviour, CPPMSTR(RexxString::bitXor), 2);
  defineKernelMethod(CHAR_B2X                          ,TheStringBehaviour, CPPMSTR(RexxString::b2x), 0);
  defineKernelMethod(CHAR_C2D                          ,TheStringBehaviour, CPPMSTR(RexxString::c2d), 1);
  defineKernelMethod(CHAR_C2X                          ,TheStringBehaviour, CPPMSTR(RexxString::c2x), 0);
  defineKernelMethod(CHAR_D2C                          ,TheStringBehaviour, CPPMSTR(RexxString::d2c), 1);
  defineKernelMethod(CHAR_D2X                          ,TheStringBehaviour, CPPMSTR(RexxString::d2x), 1);
  defineKernelMethod(CHAR_X2B                          ,TheStringBehaviour, CPPMSTR(RexxString::x2b), 0);
  defineKernelMethod(CHAR_X2C                          ,TheStringBehaviour, CPPMSTR(RexxString::x2c), 0);
  defineKernelMethod(CHAR_X2D                          ,TheStringBehaviour, CPPMSTR(RexxString::x2d), 1);
  defineKernelMethod(CHAR_ENCODEBASE64                 ,TheStringBehaviour, CPPMSTR(RexxString::encodeBase64), 0);
  defineKernelMethod(CHAR_DECODEBASE64                 ,TheStringBehaviour, CPPMSTR(RexxString::decodeBase64), 0);
  defineKernelMethod(CHAR_MAKESTRING                   ,TheStringBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_ABS                          ,TheStringBehaviour, CPPMSTR(RexxString::abs), 0);
  defineKernelMethod(CHAR_MAX                          ,TheStringBehaviour, CPPMSTR(RexxString::Max), A_COUNT);
  defineKernelMethod(CHAR_MIN                          ,TheStringBehaviour, CPPMSTR(RexxString::Min), A_COUNT);
  defineKernelMethod(CHAR_SIGN                         ,TheStringBehaviour, CPPMSTR(RexxString::sign), 0);
  defineKernelMethod(CHAR_EQUAL                        ,TheStringBehaviour, CPPMSTR(RexxString::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,TheStringBehaviour, CPPMSTR(RexxString::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN         ,TheStringBehaviour, CPPMSTR(RexxString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN         ,TheStringBehaviour, CPPMSTR(RexxString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN                  ,TheStringBehaviour, CPPMSTR(RexxString::isGreaterThan), 1);
  defineKernelMethod(CHAR_LESSTHAN                     ,TheStringBehaviour, CPPMSTR(RexxString::isLessThan), 1);
  defineKernelMethod(CHAR_GREATERTHAN_EQUAL            ,TheStringBehaviour, CPPMSTR(RexxString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_LESSTHAN           ,TheStringBehaviour, CPPMSTR(RexxString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_EQUAL               ,TheStringBehaviour, CPPMSTR(RexxString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_GREATERTHAN        ,TheStringBehaviour, CPPMSTR(RexxString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,TheStringBehaviour, CPPMSTR(RexxString::strictEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,TheStringBehaviour, CPPMSTR(RexxString::strictNotEqual), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN           ,TheStringBehaviour, CPPMSTR(RexxString::strictGreaterThan), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN              ,TheStringBehaviour, CPPMSTR(RexxString::strictLessThan), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN_EQUAL     ,TheStringBehaviour, CPPMSTR(RexxString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_LESSTHAN    ,TheStringBehaviour, CPPMSTR(RexxString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN_EQUAL        ,TheStringBehaviour, CPPMSTR(RexxString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN ,TheStringBehaviour, CPPMSTR(RexxString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_PLUS                         ,TheStringBehaviour, CPPMSTR(RexxString::plus), 1);
  defineKernelMethod(CHAR_SUBTRACT                     ,TheStringBehaviour, CPPMSTR(RexxString::minus), 1);
  defineKernelMethod(CHAR_MULTIPLY                     ,TheStringBehaviour, CPPMSTR(RexxString::multiply), 1);
  defineKernelMethod(CHAR_POWER                        ,TheStringBehaviour, CPPMSTR(RexxString::power), 1);
  defineKernelMethod(CHAR_DIVIDE                       ,TheStringBehaviour, CPPMSTR(RexxString::divide), 1);
  defineKernelMethod(CHAR_INTDIV                       ,TheStringBehaviour, CPPMSTR(RexxString::integerDivide), 1);
  defineKernelMethod(CHAR_REMAINDER                    ,TheStringBehaviour, CPPMSTR(RexxString::remainder), 1);
  defineKernelMethod(CHAR_BACKSLASH                    ,TheStringBehaviour, CPPMSTR(RexxString::notOp), 0);
  defineKernelMethod(CHAR_AND                          ,TheStringBehaviour, CPPMSTR(RexxString::andOp), 1);
  defineKernelMethod(CHAR_OR                           ,TheStringBehaviour, CPPMSTR(RexxString::orOp), 1);
  defineKernelMethod(CHAR_XOR                          ,TheStringBehaviour, CPPMSTR(RexxString::xorOp), 1);
  defineKernelMethod(CHAR_MAKEARRAY                    ,TheStringBehaviour, CPPMSTR(RexxString::makeArray), 1);
  defineKernelMethod(CHAR_LOWER                        ,TheStringBehaviour, CPPMSTR(RexxString::lowerRexx), 2);
  defineKernelMethod(CHAR_UPPER                        ,TheStringBehaviour, CPPMSTR(RexxString::upperRexx), 2);
  defineKernelMethod(CHAR_MATCH                        ,TheStringBehaviour, CPPMSTR(RexxString::match), 4);
  defineKernelMethod(CHAR_CASELESSMATCH                ,TheStringBehaviour, CPPMSTR(RexxString::caselessMatch), 4);
  defineKernelMethod(CHAR_MATCHCHAR                    ,TheStringBehaviour, CPPMSTR(RexxString::matchChar), 2);
  defineKernelMethod(CHAR_CASELESSMATCHCHAR            ,TheStringBehaviour, CPPMSTR(RexxString::caselessMatchChar), 2);
  defineKernelMethod(CHAR_EQUALS                       ,TheStringBehaviour, CPPMSTR(RexxString::equals), 1);
  defineKernelMethod(CHAR_CASELESSEQUALS               ,TheStringBehaviour, CPPMSTR(RexxString::caselessEquals), 1);
  defineKernelMethod(CHAR_COMPARETO                    ,TheStringBehaviour, CPPMSTR(RexxString::compareToRexx), 3);
  defineKernelMethod(CHAR_CASELESSCOMPARETO            ,TheStringBehaviour, CPPMSTR(RexxString::caselessCompareToRexx), 3);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStringBehaviour->setMethodDictionaryScope(TheStringClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheStringClass->subClassable("String", false);


  /***************************************************************************/
  /*           MUTABLEBUFFER                                                 */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheMutableBufferClassBehaviour, CPPMMUTBCL(RexxMutableBufferClass::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMutableBufferClassBehaviour->setMethodDictionaryScope(TheMutableBufferClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_APPEND                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::append), 1);
  defineKernelMethod(CHAR_INSERT                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::insert), 4);
  defineKernelMethod(CHAR_OVERLAY                      ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::overlay), 4);
  defineKernelMethod(CHAR_DELETE                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::mydelete), 2);
  defineKernelMethod(CHAR_SUBSTR                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::substr), 3);
  defineKernelMethod(CHAR_POS                          ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::posRexx), 2);
  defineKernelMethod(CHAR_LASTPOS                      ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::lastPos), 2);
  defineKernelMethod(CHAR_SUBCHAR                      ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::subchar), 1);
  defineKernelMethod(CHAR_GETBUFFERSIZE                , TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::getBufferSize), 0);
  defineKernelMethod(CHAR_SETBUFFERSIZE                , TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::setBufferSize), 1);

  defineKernelMethod(CHAR_LENGTH                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::lengthRexx), 0);
  defineKernelMethod(CHAR_REQUEST                      ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::requestRexx), 1);
  defineKernelMethod(CHAR_STRING                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::requestString), 0);
  defineKernelMethod(CHAR_UNINIT                       ,TheMutableBufferBehaviour, CPPMMUTB(RexxMutableBuffer::uninitMB), 0);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMutableBufferBehaviour->setMethodDictionaryScope(TheMutableBufferClass);
                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheMutableBufferClass->subClassable("MutableBuffer", true);

  /***************************************************************************/
  /*             INTEGER                                                     */
  /***************************************************************************/

    /* If the integer class was set up correctly it would have the           */
    /* class_id method in its own class but instead it points to the one     */
    /* in the string class.                                                 .*/

  defineKernelMethod(CHAR_NEW, TheIntegerClassBehaviour, CPPMSTRCL(RexxStringClass::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheIntegerClassBehaviour->setMethodDictionaryScope(TheIntegerClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_PLUS                         ,TheIntegerBehaviour, CPPMI(RexxInteger::plus), 1);
  defineKernelMethod(CHAR_SUBTRACT                     ,TheIntegerBehaviour, CPPMI(RexxInteger::minus), 1);
  defineKernelMethod(CHAR_MULTIPLY                     ,TheIntegerBehaviour, CPPMI(RexxInteger::multiply), 1);
  defineKernelMethod(CHAR_POWER                        ,TheIntegerBehaviour, CPPMI(RexxInteger::power), 1);
  defineKernelMethod(CHAR_DIVIDE                       ,TheIntegerBehaviour, CPPMI(RexxInteger::divide), 1);
  defineKernelMethod(CHAR_INTDIV                       ,TheIntegerBehaviour, CPPMI(RexxInteger::integerDivide), 1);
  defineKernelMethod(CHAR_REMAINDER                    ,TheIntegerBehaviour, CPPMI(RexxInteger::remainder), 1);
  defineKernelMethod(CHAR_BACKSLASH                    ,TheIntegerBehaviour, CPPMI(RexxInteger::notOp), 0);
  defineKernelMethod(CHAR_AND                          ,TheIntegerBehaviour, CPPMI(RexxInteger::andOp), 1);
  defineKernelMethod(CHAR_OR                           ,TheIntegerBehaviour, CPPMI(RexxInteger::orOp), 1);
  defineKernelMethod(CHAR_XOR                          ,TheIntegerBehaviour, CPPMI(RexxInteger::xorOp), 1);
  defineKernelMethod(CHAR_UNKNOWN                      ,TheIntegerBehaviour, CPPM(RexxObject::unknownRexx), 2);
  defineKernelMethod(CHAR_D2C                          ,TheIntegerBehaviour, CPPMI(RexxInteger::d2c), 1);
  defineKernelMethod(CHAR_D2X                          ,TheIntegerBehaviour, CPPMI(RexxInteger::d2x), 1);
  defineKernelMethod(CHAR_ABS                          ,TheIntegerBehaviour, CPPMI(RexxInteger::abs), 0);
  defineKernelMethod(CHAR_MAX                          ,TheIntegerBehaviour, CPPMI(RexxInteger::Max), A_COUNT);
  defineKernelMethod(CHAR_MIN                          ,TheIntegerBehaviour, CPPMI(RexxInteger::Min), A_COUNT);
  defineKernelMethod(CHAR_SIGN                         ,TheIntegerBehaviour, CPPMI(RexxInteger::sign), 0);
  defineKernelMethod(CHAR_EQUAL                        ,TheIntegerBehaviour, CPPMI(RexxInteger::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,TheIntegerBehaviour, CPPMI(RexxInteger::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN         ,TheIntegerBehaviour, CPPMI(RexxInteger::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN         ,TheIntegerBehaviour, CPPMI(RexxInteger::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN                  ,TheIntegerBehaviour, CPPMI(RexxInteger::isGreaterThan), 1);
  defineKernelMethod(CHAR_LESSTHAN                     ,TheIntegerBehaviour, CPPMI(RexxInteger::isLessThan), 1);
  defineKernelMethod(CHAR_GREATERTHAN_EQUAL            ,TheIntegerBehaviour, CPPMI(RexxInteger::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_LESSTHAN           ,TheIntegerBehaviour, CPPMI(RexxInteger::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_EQUAL               ,TheIntegerBehaviour, CPPMI(RexxInteger::isLessOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_GREATERTHAN        ,TheIntegerBehaviour, CPPMI(RexxInteger::isLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,TheIntegerBehaviour, CPPMI(RexxInteger::strictEqual), 1);
  defineKernelMethod(CHAR_HASHCODE                     ,TheIntegerBehaviour, CPPMI(RexxInteger::hashCode), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,TheIntegerBehaviour, CPPMI(RexxInteger::strictNotEqual), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN           ,TheIntegerBehaviour, CPPMI(RexxInteger::strictGreaterThan), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN              ,TheIntegerBehaviour, CPPMI(RexxInteger::strictLessThan), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN_EQUAL     ,TheIntegerBehaviour, CPPMI(RexxInteger::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_LESSTHAN    ,TheIntegerBehaviour, CPPMI(RexxInteger::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN_EQUAL        ,TheIntegerBehaviour, CPPMI(RexxInteger::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN ,TheIntegerBehaviour, CPPMI(RexxInteger::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_MAKESTRING                   ,TheIntegerBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_FORMAT                       ,TheIntegerBehaviour, CPPMI(RexxInteger::format), 4);
  defineKernelMethod(CHAR_TRUNC                        ,TheIntegerBehaviour, CPPMI(RexxInteger::trunc), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheIntegerBehaviour->setMethodDictionaryScope(TheIntegerClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheIntegerClass->subClassable("String", true);

  /***************************************************************************/
  /*             NUMBERSTRING                                                */
  /***************************************************************************/

     /* If the numberstring class was set up correctly it should have the    */
     /* class_id method in its own class but instead it points to the one    */
     /* in the string class.                                                 */

  defineKernelMethod(CHAR_NEW, TheNumberStringClassBehaviour, CPPMSTRCL(RexxStringClass::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheNumberStringClassBehaviour->setMethodDictionaryScope(TheNumberStringClass);

                                       /* Add the instance methods to this  */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_UNKNOWN                      ,TheNumberStringBehaviour, CPPM(RexxObject::unknownRexx), 2);
  defineKernelMethod(CHAR_ABS                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::abs), 0);
  defineKernelMethod(CHAR_MAX                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::Max), A_COUNT);
  defineKernelMethod(CHAR_MIN                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::Min), A_COUNT);
  defineKernelMethod(CHAR_SIGN                         ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::Sign), 0);
  defineKernelMethod(CHAR_D2C                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::d2c), 1);
  defineKernelMethod(CHAR_D2X                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::d2x), 1);
  defineKernelMethod(CHAR_EQUAL                        ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN         ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN         ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN                  ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::isGreaterThan), 1);
  defineKernelMethod(CHAR_LESSTHAN                     ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::isLessThan), 1);
  defineKernelMethod(CHAR_GREATERTHAN_EQUAL            ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_LESSTHAN           ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_EQUAL               ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_GREATERTHAN        ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictEqual), 1);
  defineKernelMethod(CHAR_HASHCODE                     ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::hashCode), 0);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictNotEqual), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN           ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictGreaterThan), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN              ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictLessThan), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN_EQUAL     ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_LESSTHAN    ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN_EQUAL        ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_PLUS                         ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::plus), 1);
  defineKernelMethod(CHAR_SUBTRACT                     ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::minus), 1);
  defineKernelMethod(CHAR_MULTIPLY                     ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::multiply), 1);
  defineKernelMethod(CHAR_POWER                        ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::power), 1);
  defineKernelMethod(CHAR_DIVIDE                       ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::divide), 1);
  defineKernelMethod(CHAR_INTDIV                       ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::integerDivide), 1);
  defineKernelMethod(CHAR_REMAINDER                    ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::remainder), 1);
  defineKernelMethod(CHAR_BACKSLASH                    ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::notOp), 0);
  defineKernelMethod(CHAR_AND                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::andOp), 1);
  defineKernelMethod(CHAR_OR                           ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::orOp), 1);
  defineKernelMethod(CHAR_XOR                          ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::xorOp), 1);
  defineKernelMethod(CHAR_MAKESTRING                   ,TheNumberStringBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_FORMAT                       ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::formatRexx), 4);
  defineKernelMethod(CHAR_TRUNC                        ,TheNumberStringBehaviour, CPPMNM(RexxNumberString::trunc), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheNumberStringBehaviour->setMethodDictionaryScope(TheNumberStringClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheNumberStringClass->subClassable("String", true);


  /***************************************************************************/
  /*           SUPPLIER                                                      */
  /***************************************************************************/
                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheSupplierClassBehaviour, CPPMSUPCL(RexxSupplierClass::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheSupplierClassBehaviour->setMethodDictionaryScope(TheSupplierClass);


                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */

  defineKernelMethod(CHAR_AVAILABLE ,TheSupplierBehaviour, CPPMSUP(RexxSupplier::available), 0);
  defineKernelMethod(CHAR_INDEX     ,TheSupplierBehaviour, CPPMSUP(RexxSupplier::index), 0);
  defineKernelMethod(CHAR_NEXT      ,TheSupplierBehaviour, CPPMSUP(RexxSupplier::next), 0);
  defineKernelMethod(CHAR_ITEM      ,TheSupplierBehaviour, CPPMSUP(RexxSupplier::value), 0);
  defineKernelMethod(CHAR_INIT      ,TheSupplierBehaviour, CPPMSUP(RexxSupplier::initRexx), 2);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheSupplierBehaviour->setMethodDictionaryScope(TheSupplierClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheSupplierClass->subClassable("Supplier", false);

  /***************************************************************************/
  /*           TABLE                                                         */
  /***************************************************************************/

                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW          , TheTableClassBehaviour, CPPMTBL(RexxTable::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheTableClassBehaviour->setMethodDictionaryScope(TheTableClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS     , TheTableBehaviour, CPPMHC(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL, TheTableBehaviour, CPPMHC(RexxHashTableCollection::put), 2);
  defineKernelMethod(CHAR_MAKEARRAY    , TheTableBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           , TheTableBehaviour, CPPMHC(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_HASINDEX     , TheTableBehaviour, CPPMHC(RexxHashTableCollection::hasIndex), 1);
  defineKernelMethod(CHAR_ITEMS        , TheTableBehaviour, CPPMTBL(RexxTable::itemsRexx), 0);
  defineKernelMethod(CHAR_PUT          , TheTableBehaviour, CPPMHC(RexxHashTableCollection::put), 2);
  defineKernelMethod(CHAR_REMOVE       , TheTableBehaviour, CPPMHC(RexxHashTableCollection::removeRexx), 1);
  defineKernelMethod(CHAR_SUPPLIER     , TheTableBehaviour, CPPMHC(RexxHashTableCollection::supplier), 0);
  defineKernelMethod(CHAR_ALLITEMS     , TheTableBehaviour, CPPMHC(RexxHashTableCollection::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   , TheTableBehaviour, CPPMHC(RexxHashTableCollection::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        , TheTableBehaviour, CPPMHC(RexxHashTableCollection::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      , TheTableBehaviour, CPPMHC(RexxHashTableCollection::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        , TheTableBehaviour, CPPMHC(RexxHashTableCollection::indexRexx), 1);
  defineKernelMethod(CHAR_HASITEM      , TheTableBehaviour, CPPMHC(RexxHashTableCollection::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheTableBehaviour, CPPMHC(RexxHashTableCollection::removeItem), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheTableBehaviour->setMethodDictionaryScope(TheTableClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheTableClass->subClassable("Table", false);

  /***************************************************************************/
  /***************************************************************************/
  /***************************************************************************/
    /* These classes don't have any class methods                            */
    /*  and are not subclassed from object                                   */

  /* put the kernel-provided public objects in the environment directory */
  kernel_public(CHAR_ARRAY            ,TheArrayClass   ,TheEnvironment);
  kernel_public(CHAR_CLASS            ,TheClassClass   ,TheEnvironment);
  kernel_public(CHAR_DIRECTORY        ,TheDirectoryClass ,TheEnvironment);
  kernel_public(CHAR_ENVIRONMENT      ,TheEnvironment  ,TheEnvironment);
  kernel_public(CHAR_FALSE            ,TheFalseObject  ,TheEnvironment);
  kernel_public(CHAR_KERNEL           ,TheKernel       ,TheEnvironment);
  kernel_public(CHAR_LIST             ,TheListClass    ,TheEnvironment);
  kernel_public(CHAR_MESSAGE          ,TheMessageClass ,TheEnvironment);
  kernel_public(CHAR_METHOD           ,TheMethodClass  ,TheEnvironment);
  kernel_public(CHAR_NIL              ,TheNilObject    ,TheEnvironment);
  kernel_public(CHAR_OBJECT           ,TheObjectClass  ,TheEnvironment);
  kernel_public(CHAR_QUEUE            ,TheQueueClass   ,TheEnvironment);
  kernel_public(CHAR_RELATION         ,TheRelationClass,TheEnvironment);
  kernel_public(CHAR_STRING           ,TheStringClass  ,TheEnvironment);
  kernel_public(CHAR_MUTABLEBUFFER    ,TheMutableBufferClass  ,TheEnvironment);
  kernel_public(CHAR_STEM             ,TheStemClass    ,TheEnvironment);
  kernel_public(CHAR_SUPPLIER         ,TheSupplierClass,TheEnvironment);
  kernel_public(CHAR_SYSTEM           ,TheSystem       ,TheEnvironment);
  kernel_public(CHAR_TABLE            ,TheTableClass   ,TheEnvironment);
  kernel_public(CHAR_TRUE             ,TheTrueObject   ,TheEnvironment);

  kernel_public(CHAR_PUBLIC_ROUTINES  ,ThePublicRoutines, TheEnvironment);
  kernel_public(CHAR_STATIC_REQUIRES  ,TheStaticRequires, TheEnvironment);

  /* set up the kernel directory (MEMORY done elsewhere) */
  kernel_public(CHAR_INTEGER          ,TheIntegerClass     , TheKernel);
  kernel_public(CHAR_ENVELOPE         ,TheEnvelopeClass    , TheKernel);
  kernel_public(CHAR_NUMBERSTRING     ,TheNumberStringClass, TheKernel);
  kernel_public(CHAR_ACTIVITY         ,TheActivityClass    , TheKernel);
  kernel_public(CHAR_NMETHOD          ,TheNativeCodeClass  , TheKernel);

  kernel_public(CHAR_FUNCTIONS        ,TheFunctionsDirectory  ,TheKernel);
  kernel_public(CHAR_GLOBAL_STRINGS   ,TheGlobalStrings       ,TheKernel);
  kernel_public(CHAR_NULLA            ,TheNullArray           ,TheKernel);
  kernel_public(CHAR_NULLPOINTER      ,TheNullPointer         ,TheKernel);
  kernel_public(CHAR_GENERIC_SOMMETHOD,TheGenericSomMethod    ,TheKernel);
  kernel_public(CHAR_COMMON_RETRIEVERS,TheCommonRetrievers    ,TheKernel);
  kernel_public(CHAR_ENVIRONMENT      ,TheEnvironment         ,TheKernel);

  /* make the memory and environment objects public */
  kernel_public(CHAR_MEMORY      ,TheMemoryObject   ,TheKernel);

                                       /* set Oryx version                  */
  kernel_public(CHAR_VERSION,version_number(),TheKernel);
                                       /* set the system name               */
  kernel_public(CHAR_NAME, SysName(), TheSystem);
                                       /* set the internal system name      */
  kernel_public(CHAR_INTNAME, SysINTName(), TheSystem);
                                       /* and the system version info       */
  kernel_public(CHAR_VERSION,SysVersion(),TheSystem);

/******************************************************************************/
/*      Complete the image build process, calling BaseClasses to establish    */
/*      the rest of the REXX image.                                           */
/******************************************************************************/

  /* set up the kernel methods that will be defined on OBJECT classes in  */
  /*  BaseClasses.ORX and ORYXJ.ORX.                                            */
                                       /* create a kernel methods directory */
  kernel_methods = (RexxDirectory *)save(new_directory());
  kernel_methods->put(createKernelMethod(CPPMLOC(RexxLocal::local), 0), kernel_name(CHAR_LOCAL));
  kernel_methods->put(createKernelMethod(CPPMLOC(RexxLocal::runProgram), 1), kernel_name(CHAR_RUN_PROGRAM));
  kernel_methods->put(createKernelMethod(CPPMLOC(RexxLocal::callString), A_COUNT), kernel_name(CHAR_CALL_STRING));
  kernel_methods->put(createKernelMethod(CPPMLOC(RexxLocal::callProgram), A_COUNT), kernel_name(CHAR_CALL_PROGRAM));
  kernel_methods->put(createKernelMethod(CPPMSND(RexxSender::getPid), 0), kernel_name(CHAR_SENDER_GETPID));
  kernel_methods->put(createKernelMethod(CPPMSND(RexxSender::sendMessage), 2), kernel_name(CHAR_SENDER_SENDMESSAGE));

                                       /* create the BaseClasses method and run it*/
  symb = kernel_name(BASEIMAGELOAD);   /* get a name version of the string  */
                                       /* go resolve the program name       */
  programName = SysResolveProgramName(symb, OREF_NULL);
                                       /* Push marker onto stack so we know */
  CurrentActivity->pushNil();          /* what level we entered.            */
                                       /* set up setjmp environment         */
  if (setjmp(CurrentActivity->nestedInfo.jmpenv)) {
    CurrentActivity->error(0);         /* do error cleanup                  */
    return false;                      /* this is a setup failure           */
  }
                                       /* create a method object out of this*/
  meth = TheMethodClass->newFile(programName);
                                       /* now call BaseClasses to finish the image*/
  ((RexxObject *)CurrentActivity)->shriekRun(meth, OREF_NULL, OREF_NULL, (RexxObject **)&kernel_methods, 1);
  discard(kernel_methods);             /* release the directory lock        */

  /* define and suppress methods in the nil object */
  TheNilObject->defMethod(kernel_name(CHAR_COPY), (RexxMethod *)TheNilObject);
  TheNilObject->defMethod(kernel_name(CHAR_START), (RexxMethod *)TheNilObject);
  TheNilObject->defMethod(kernel_name(CHAR_OBJECTNAMEEQUALS), (RexxMethod *)TheNilObject);

  RexxClass *ordered = (RexxClass *)TheEnvironment->get(kernel_name(CHAR_ORDEREDCOLLECTION));

  TheArrayClass->inherit(ordered, OREF_NULL);
  TheArrayClass->setRexxDefined();

  TheQueueClass->inherit(ordered, OREF_NULL);
  TheQueueClass->setRexxDefined();

  TheListClass->inherit(ordered, OREF_NULL);
  TheListClass->setRexxDefined();

  RexxClass *map = (RexxClass *)TheEnvironment->get(kernel_name(CHAR_MAPCOLLECTION));

  TheTableClass->inherit(map, OREF_NULL);
  TheTableClass->setRexxDefined();

  TheRelationClass->inherit(map, OREF_NULL);
  TheRelationClass->setRexxDefined();

  TheDirectoryClass->inherit(map, OREF_NULL);
  TheDirectoryClass->setRexxDefined();

  TheStemClass->inherit(map, OREF_NULL);
  TheStemClass->setRexxDefined();

  RexxClass *comparable = (RexxClass *)TheEnvironment->get(kernel_name(CHAR_COMPARABLE));

  TheStringClass->inherit(comparable, OREF_NULL);
  TheStringClass->setRexxDefined();

  // this has been protecting every thing critical
  // from GC events thus far, but now we remove it because
  // it contains things we don't want to save in the image.
  TheEnvironment->remove(kernel_name(CHAR_KERNEL));
  return true;
}

void createImage(void)
/******************************************************************************/
/* Function:  Build and save the REXX image                                   */
/******************************************************************************/
{
  kernelInit();                        /* initialize the kernel             */
                                       /* get the local environment         */
  ProcessLocalEnv = (RexxDirectory *)save(new_directory());
                                       /* Find an activity for this thread  */
  TheActivityClass->getActivity();     /* (will create one if necessary)    */
  // go build the rest of the image, but don't save if there is a failure.
  if (!kernel_setup()) {
      logic_error("Error building kernel image.  Image not saved.");
  }
  discard(ProcessLocalEnv);            /* remove the local env              */
  ProcessLocalEnv = OREF_NULL;         /* clear this out                    */
                                       /* release the kernel semaphore      */
  TheActivityClass->returnActivity(CurrentActivity);
  memoryObject.saveImage();            /* will not return                   */
  exit(RC_OK);                         // successful build
}

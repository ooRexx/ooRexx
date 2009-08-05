/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Setup initial class definitions during an image build                      */
/*                                                                            */
/* NOTE:  The methods contained here are part of the RexxMemory class, but    */
/* are in a separate file because of the extensive #include requirements      */
/* for these particular methods and tables.                                   */
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
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "RexxEnvelope.hpp"
#include "MessageClass.hpp"
#include "StemClass.hpp"
#include "RexxMisc.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxVariableDictionary.hpp"
#include "ExpressionVariable.hpp"
#include "RexxLocalVariables.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "BufferClass.hpp"
#include "WeakReferenceClass.hpp"
#include "CPPCode.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "PackageManager.hpp"
#include "PackageClass.hpp"
#include "ContextClass.hpp"
#include "StackFrameClass.hpp"


void RexxMemory::defineKernelMethod(
    const char    * name,              /* ASCII-Z name for the method       */
    RexxBehaviour * behaviour,         /* behaviour to use                  */
    PCPPM           entryPoint,        /* method's entry point              */
    size_t          arguments )        /* count of arguments                */
/******************************************************************************/
/* Function:  Add a C++ method to an object's behaviour                       */
/******************************************************************************/
{
    behaviour->define(name, entryPoint, arguments);
}

void RexxMemory::defineProtectedKernelMethod(
    const char    * name,              /* ASCII-Z name for the method       */
    RexxBehaviour * behaviour,         /* behaviour to use                  */
    PCPPM           entryPoint,        /* method's entry point              */
    size_t          arguments )        /* count of arguments                */
/******************************************************************************/
/* Function:  Add a C++ method to an object's behaviour                       */
/******************************************************************************/
{
    RexxMethod *method = behaviour->define(name, entryPoint, arguments);
    method->setProtected();              /* make this protected               */
}


void RexxMemory::definePrivateKernelMethod(
    const char    * name,              /* ASCII-Z name for the method       */
    RexxBehaviour * behaviour,         /* behaviour to use                  */
    PCPPM           entryPoint,        /* method's entry point              */
    size_t          arguments )        /* count of arguments                */
/******************************************************************************/
/* Function:  Add a C++ method to an object's behaviour                       */
/******************************************************************************/
{
    RexxMethod *method = behaviour->define(name, entryPoint, arguments);
    method->setProtected();              /* make this protected               */
    method->setPrivate();                /* make this protected               */
}


void RexxMemory::createImage()
/******************************************************************************/
/* Function:  Initialize the kernel on image build                            */
/******************************************************************************/
{
  RexxMemory::create();                /* create initial memory stuff       */

  Interpreter::init();                 // the interpreter subsystem first
  ActivityManager::init();             /* Initialize the activity managers  */
  // Get an instance.  This also gives the root activity of the instance
  // the kernel lock.
  Interpreter::createInterpreterInstance();
  memoryObject.createStrings();        /* create all of the OREF_ strings   */
  // initializer for native libraries
  PackageManager::initialize();

                                       /* avoid that through caching        */
                                       /* TheTrueObject == IntegerOne etc.  */
  TheTrueObject  = new RexxInteger(1);
  TheFalseObject = new RexxInteger(0);


  TheNilObject = new RexxNilObject;
                                       /* We don't move the NIL object, we  */
                                       /*will use the remote systems NIL    */
                                       /*object.                            */
  TheNilObject->makeProxiedObject();

                                       /* create string first               */
  RexxString::createInstance();
  RexxObject::createInstance();
  RexxTable::createInstance();
  RexxIdentityTable::createInstance();
  RexxRelation::createInstance();

  TheFunctionsDirectory = new_directory();

                                       /* If first one through, generate all   */
  IntegerZero    = new_integer(0);    /*  static integers we want to use...   */
  IntegerOne     = new_integer(1);    /* This will allow us to use the static */
  IntegerTwo     = new_integer(2);    /* integers instead of having to do a   */
  IntegerThree   = new_integer(3);    /* new_integer every time....           */
  IntegerFour    = new_integer(4);
  IntegerFive    = new_integer(5);
  IntegerSix     = new_integer(6);
  IntegerSeven   = new_integer(7);
  IntegerEight   = new_integer(8);
  IntegerNine    = new_integer(9);
  IntegerMinusOne = new_integer(-1);


                                       /* RexxNumberString                  */
  // NOTE:  The number string class lies about its identity
  RexxNumberString::createInstance();
  RexxArray::createInstance();

  // The pointer class needs to be created early because other classes
  // use the instances to store information.
  RexxPointer::createInstance();

  RexxDirectory::createInstance();
  CLASS_CREATE(Directory, "Directory", RexxClass);  /* RexxDirectory                     */
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
  TheEnvironment->makeProxiedObject();
  TheKernel->makeProxiedObject();
  TheSystem->makeProxiedObject();

                                       /* RexxMethod                        */
  RexxMethod::createInstance();
  RoutineClass::createInstance();
  PackageClass::createInstance();
  RexxContext::createInstance();
  RexxQueue::createInstance();
  RexxList::createInstance();
  RexxStem::createInstance();
  RexxSupplier::createInstance();
  RexxMessage::createInstance();
  RexxMutableBuffer::createInstance();

  RexxBuffer::createInstance();
  WeakReference::createInstance();
  StackFrameClass::createInstance();

                                       /* build the common retriever tables */
  TheCommonRetrievers = (RexxDirectory *)new_directory();
                                       /* add all of the special variables  */
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_SELF, VARIABLE_SELF), OREF_SELF);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_SUPER, VARIABLE_SUPER), OREF_SUPER);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_SIGL, VARIABLE_SIGL), OREF_SIGL);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_RC, VARIABLE_RC), OREF_RC);
  TheCommonRetrievers->put((RexxObject *)new RexxParseVariable(OREF_RESULT, VARIABLE_RESULT), OREF_RESULT);
  memoryObject.enableOrefChecks();     /* enable setCheckOrefs...           */

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
  defineKernelMethod(CHAR_NEW, TheClassClassBehaviour, CPPM(RexxClass::newRexx), A_COUNT);

                                       /* set the scope of the method to    */
                                       /* the CLASS scope                   */
  TheClassClassBehaviour->setMethodDictionaryScope(TheClassClass);

                                       /* add the instance methods to the   */
                                       /* class's instance mdict            */
  defineProtectedKernelMethod(CHAR_BASECLASS       ,TheClassBehaviour, CPPM(RexxClass::getBaseClass), 0);
  defineProtectedKernelMethod(CHAR_DEFINE          ,TheClassBehaviour, CPPM(RexxClass::defineMethod), 2);
  defineProtectedKernelMethod(CHAR_DEFINE_METHODS  ,TheClassBehaviour, CPPM(RexxClass::defineMethods), 1);
  defineProtectedKernelMethod(CHAR_DELETE ,TheClassBehaviour, CPPM(RexxClass::deleteMethod), 1);
  defineKernelMethod(CHAR_ENHANCED        ,TheClassBehaviour, CPPM(RexxClass::enhanced), A_COUNT);
  defineKernelMethod(CHAR_ID              ,TheClassBehaviour, CPPM(RexxClass::getId), 0);
  defineKernelMethod(CHAR_INHERIT         ,TheClassBehaviour, CPPM(RexxClass::inherit), 2);
  defineProtectedKernelMethod(CHAR_METACLASS       ,TheClassBehaviour, CPPM(RexxClass::getMetaClass), 0);
  defineKernelMethod(CHAR_METHOD          ,TheClassBehaviour, CPPM(RexxClass::method), 1);
  defineKernelMethod(CHAR_METHODS         ,TheClassBehaviour, CPPM(RexxClass::methods), 1);
  defineKernelMethod(CHAR_MIXINCLASS      ,TheClassBehaviour, CPPM(RexxClass::mixinclass), 3);
  defineKernelMethod(CHAR_QUERYMIXINCLASS ,TheClassBehaviour, CPPM(RexxClass::queryMixinClass), 0);
  defineKernelMethod(CHAR_SUBCLASS        ,TheClassBehaviour, CPPM(RexxClass::subclass), 3);
  defineProtectedKernelMethod(CHAR_SUBCLASSES      ,TheClassBehaviour, CPPM(RexxClass::getSubClasses), 0);
  defineProtectedKernelMethod(CHAR_SUPERCLASSES    ,TheClassBehaviour, CPPM(RexxClass::getSuperClasses), 0);
  defineProtectedKernelMethod(CHAR_SUPERCLASS      ,TheClassBehaviour, CPPM(RexxClass::getSuperClass), 0);
  defineProtectedKernelMethod(CHAR_UNINHERIT       ,TheClassBehaviour, CPPM(RexxClass::uninherit), 1);
                                       /* Class operator methods....        */
  defineKernelMethod(CHAR_EQUAL                  ,TheClassBehaviour, CPPM(RexxClass::equal), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL           ,TheClassBehaviour, CPPM(RexxClass::strictEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL        ,TheClassBehaviour, CPPM(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN   ,TheClassBehaviour, CPPM(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN   ,TheClassBehaviour, CPPM(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL ,TheClassBehaviour, CPPM(RexxClass::notEqual), 1);
  defineKernelMethod(CHAR_ISSUBCLASSOF           ,TheClassBehaviour, CPPM(RexxClass::isSubclassOf), 1);
  defineProtectedKernelMethod(CHAR_SHRIEKREXXDEFINED,TheClassBehaviour, CPPM(RexxClass::setRexxDefined), 0);
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
  defineKernelMethod("STARTWITH"                 ,TheObjectBehaviour, CPPM(RexxObject::startWith), 2);
  defineKernelMethod("SEND"                      ,TheObjectBehaviour, CPPM(RexxObject::send), A_COUNT);
  defineKernelMethod("SENDWITH"                  ,TheObjectBehaviour, CPPM(RexxObject::sendWith), 2);
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
  TheObjectClass->subClassable(true);
  TheClassClass->subClassable(true);

  /************************************** The rest of the classes can now be */
  /************************************** set up.                            */

  /***************************************************************************/
  /*           ARRAY                                                         */
  /***************************************************************************/

  defineKernelMethod(CHAR_NEW, TheArrayClassBehaviour, CPPM(RexxArray::newRexx), A_COUNT);
  defineKernelMethod(CHAR_OF,  TheArrayClassBehaviour, CPPM(RexxArray::of), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheArrayClassBehaviour->setMethodDictionaryScope(TheArrayClass);

  defineKernelMethod(CHAR_BRACKETS     ,TheArrayBehaviour, CPPM(RexxArray::getRexx), A_COUNT);
  defineKernelMethod(CHAR_BRACKETSEQUAL,TheArrayBehaviour, CPPM(RexxArray::putRexx), A_COUNT);
  defineKernelMethod(CHAR_AT           ,TheArrayBehaviour, CPPM(RexxArray::getRexx), A_COUNT);
  defineKernelMethod(CHAR_DIMENSION    ,TheArrayBehaviour, CPPM(RexxArray::dimension), 1);
  defineKernelMethod(CHAR_HASINDEX     ,TheArrayBehaviour, CPPM(RexxArray::hasIndexRexx), A_COUNT);
  defineKernelMethod(CHAR_ITEMS        ,TheArrayBehaviour, CPPM(RexxArray::itemsRexx), 0);
  defineKernelMethod(CHAR_MAKEARRAY    ,TheArrayBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_PUT          ,TheArrayBehaviour, CPPM(RexxArray::putRexx), A_COUNT);
  defineKernelMethod(CHAR_REMOVE       ,TheArrayBehaviour, CPPM(RexxArray::removeRexx), A_COUNT);
  defineKernelMethod(CHAR_SECTION      ,TheArrayBehaviour, CPPM(RexxArray::sectionRexx), 2);
  defineKernelMethod(CHAR_SIZE         ,TheArrayBehaviour, CPPM(RexxArray::sizeRexx), 0);
  defineKernelMethod(CHAR_SUPPLIER     ,TheArrayBehaviour, CPPM(RexxArray::supplier), 0);
  defineKernelMethod(CHAR_FIRST        ,TheArrayBehaviour, CPPM(RexxArray::firstRexx), 0);
  defineKernelMethod(CHAR_LAST         ,TheArrayBehaviour, CPPM(RexxArray::lastRexx), 0);
  defineKernelMethod(CHAR_NEXT         ,TheArrayBehaviour, CPPM(RexxArray::nextRexx), A_COUNT);
  defineKernelMethod(CHAR_PREVIOUS     ,TheArrayBehaviour, CPPM(RexxArray::previousRexx), A_COUNT);
  defineKernelMethod(CHAR_APPEND       ,TheArrayBehaviour, CPPM(RexxArray::appendRexx), 1);
  defineKernelMethod(CHAR_MAKESTRING   ,TheArrayBehaviour, CPPM(RexxArray::makeString), 2);
  defineKernelMethod(CHAR_TOSTRING     ,TheArrayBehaviour, CPPM(RexxArray::toString), 2);
  defineKernelMethod(CHAR_ALLINDEXES   ,TheArrayBehaviour, CPPM(RexxArray::allIndexes), 0);
  defineKernelMethod(CHAR_ALLITEMS     ,TheArrayBehaviour, CPPM(RexxArray::allItems), 0);
  defineKernelMethod(CHAR_EMPTY        ,TheArrayBehaviour, CPPM(RexxArray::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      ,TheArrayBehaviour, CPPM(RexxArray::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        ,TheArrayBehaviour, CPPM(RexxArray::index), 1);
  defineKernelMethod(CHAR_HASITEM      ,TheArrayBehaviour, CPPM(RexxArray::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   ,TheArrayBehaviour, CPPM(RexxArray::removeItem), 1);
  defineKernelMethod(CHAR_SORT         ,TheArrayBehaviour, CPPM(RexxArray::sortRexx), 0);
  defineKernelMethod(CHAR_SORTWITH     ,TheArrayBehaviour, CPPM(RexxArray::sortWithRexx), 1);
  defineKernelMethod(CHAR_STABLESORT   ,TheArrayBehaviour, CPPM(RexxArray::stableSortRexx), 0);
  defineKernelMethod(CHAR_STABLESORTWITH ,TheArrayBehaviour, CPPM(RexxArray::stableSortWithRexx), 1);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheArrayBehaviour->setMethodDictionaryScope(TheArrayClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheArrayClass->subClassable(false);

  /***************************************************************************/
  /*           DIRECTORY                                                     */
  /***************************************************************************/

  defineKernelMethod(CHAR_NEW           , TheDirectoryClassBehaviour, CPPM(RexxDirectory::newRexx), A_COUNT);

                                       /* set the scope of the method to    */
                                       /* this classes oref                 */
  TheDirectoryClassBehaviour->setMethodDictionaryScope(TheDirectoryClass);

                                       /* add the instance methods          */
  defineKernelMethod(CHAR_BRACKETS      , TheDirectoryBehaviour, CPPM(RexxDirectory::atRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL , TheDirectoryBehaviour, CPPM(RexxDirectory::put), 2);
  defineKernelMethod(CHAR_AT            , TheDirectoryBehaviour, CPPM(RexxDirectory::atRexx), 1);
  defineKernelMethod(CHAR_ENTRY         , TheDirectoryBehaviour, CPPM(RexxDirectory::entryRexx), 1);
  defineKernelMethod(CHAR_HASENTRY      , TheDirectoryBehaviour, CPPM(RexxDirectory::hasEntry), 1);
  defineKernelMethod(CHAR_HASINDEX      , TheDirectoryBehaviour, CPPM(RexxDirectory::hasIndex), 1);
  defineKernelMethod(CHAR_ITEMS         , TheDirectoryBehaviour, CPPM(RexxDirectory::itemsRexx), 0);
  defineKernelMethod(CHAR_MAKEARRAY     , TheDirectoryBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_ALLITEMS      , TheDirectoryBehaviour, CPPM(RexxDirectory::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES    , TheDirectoryBehaviour, CPPM(RexxDirectory::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY         , TheDirectoryBehaviour, CPPM(RexxDirectory::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY       , TheDirectoryBehaviour, CPPM(RexxDirectory::isEmpty), 0);
  defineKernelMethod(CHAR_PUT           , TheDirectoryBehaviour, CPPM(RexxDirectory::put), 2);
  defineKernelMethod(CHAR_REMOVE        , TheDirectoryBehaviour, CPPM(RexxDirectory::removeRexx), 1);
  defineKernelMethod(CHAR_SETENTRY      , TheDirectoryBehaviour, CPPM(RexxDirectory::setEntry), 2);
  defineProtectedKernelMethod(CHAR_SETMETHOD   , TheDirectoryBehaviour, CPPM(RexxDirectory::setMethod), 2);
  defineKernelMethod(CHAR_SUPPLIER      , TheDirectoryBehaviour, CPPM(RexxDirectory::supplier), 0);
  defineKernelMethod(CHAR_UNKNOWN       , TheDirectoryBehaviour, CPPM(RexxObject::unknownRexx), 2);
  defineProtectedKernelMethod(CHAR_UNSETMETHOD   , TheDirectoryBehaviour, CPPM(RexxDirectory::removeRexx), 1);
  defineKernelMethod(CHAR_INDEX        , TheDirectoryBehaviour, CPPM(RexxDirectory::indexRexx), 1);
  defineKernelMethod(CHAR_HASITEM      , TheDirectoryBehaviour, CPPM(RexxDirectory::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheDirectoryBehaviour, CPPM(RexxDirectory::removeItem), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheDirectoryBehaviour->setMethodDictionaryScope(TheDirectoryClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheDirectoryClass->subClassable(false);


  /***************************************************************************/
  /*           LIST                                                          */
  /***************************************************************************/

                                       /* add the class behaviour methods   */
  defineKernelMethod(CHAR_NEW           , TheListClassBehaviour, CPPM(RexxList::newRexx), A_COUNT);
  defineKernelMethod(CHAR_OF            , TheListClassBehaviour, CPPM(RexxList::classOf), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheListClassBehaviour->setMethodDictionaryScope(TheListClass);

                                       /* add the instance behaviour methods*/
  defineKernelMethod(CHAR_BRACKETS     ,TheListBehaviour, CPPM(RexxList::value), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL,TheListBehaviour, CPPM(RexxList::put), 2);
  defineKernelMethod(CHAR_MAKEARRAY    ,TheListBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           ,TheListBehaviour, CPPM(RexxList::value), 1);
  defineKernelMethod(CHAR_FIRSTITEM    ,TheListBehaviour, CPPM(RexxList::firstItem), 0);
  defineKernelMethod(CHAR_HASINDEX     ,TheListBehaviour, CPPM(RexxList::hasIndex), 1);
  defineKernelMethod(CHAR_INSERT       ,TheListBehaviour, CPPM(RexxList::insertRexx), 2);
  defineKernelMethod(CHAR_ITEMS        ,TheListBehaviour, CPPM(RexxList::itemsRexx), 0);
  defineKernelMethod(CHAR_LASTITEM     ,TheListBehaviour, CPPM(RexxList::lastItem), 0);
  defineKernelMethod(CHAR_FIRST        ,TheListBehaviour, CPPM(RexxList::firstRexx), 0);
  defineKernelMethod(CHAR_LAST         ,TheListBehaviour, CPPM(RexxList::lastRexx), 0);
  defineKernelMethod(CHAR_NEXT         ,TheListBehaviour, CPPM(RexxList::next), 1);
  defineKernelMethod(CHAR_PREVIOUS     ,TheListBehaviour, CPPM(RexxList::previous), 1);
  defineKernelMethod(CHAR_PUT          ,TheListBehaviour, CPPM(RexxList::put), 2);
  defineKernelMethod(CHAR_REMOVE       ,TheListBehaviour, CPPM(RexxList::remove), 1);
  defineKernelMethod(CHAR_SECTION      ,TheListBehaviour, CPPM(RexxList::section), 2);
  defineKernelMethod(CHAR_SUPPLIER     ,TheListBehaviour, CPPM(RexxList::supplier), 0);
  defineKernelMethod(CHAR_APPEND       ,TheListBehaviour, CPPM(RexxList::append), 1);
  defineKernelMethod(CHAR_ALLITEMS     ,TheListBehaviour, CPPM(RexxList::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   ,TheListBehaviour, CPPM(RexxList::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        ,TheListBehaviour, CPPM(RexxList::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      ,TheListBehaviour, CPPM(RexxList::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        ,TheListBehaviour, CPPM(RexxList::index), 1);
  defineKernelMethod(CHAR_HASITEM      ,TheListBehaviour, CPPM(RexxList::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM   ,TheListBehaviour, CPPM(RexxList::removeItem), 1);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheListBehaviour->setMethodDictionaryScope(TheListClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheListClass->subClassable(false);

  /***************************************************************************/
  /*           MESSAGE                                                       */
  /***************************************************************************/

                                       /* Define the NEW method in the      */
                                       /* class behaviour mdict             */
  defineKernelMethod(CHAR_NEW      , TheMessageClassBehaviour, CPPM(RexxMessage::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMessageClassBehaviour->setMethodDictionaryScope(TheMessageClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */

  defineKernelMethod(CHAR_COMPLETED, TheMessageBehaviour, CPPM(RexxMessage::completed), 0);
  defineKernelMethod(CHAR_HASERROR,  TheMessageBehaviour, CPPM(RexxMessage::hasError), 0);
  defineKernelMethod(CHAR_NOTIFY   , TheMessageBehaviour, CPPM(RexxMessage::notify), 1);
  defineKernelMethod(CHAR_RESULT   , TheMessageBehaviour, CPPM(RexxMessage::result), 0);
  defineKernelMethod(CHAR_TARGET   , TheMessageBehaviour, CPPM(RexxMessage::messageTarget), 0);
  defineKernelMethod(CHAR_MESSAGENAME  , TheMessageBehaviour, CPPM(RexxMessage::messageName), 0);
  defineKernelMethod(CHAR_ARGUMENTS  , TheMessageBehaviour, CPPM(RexxMessage::arguments), 0);
  defineKernelMethod(CHAR_ERRORCONDITION , TheMessageBehaviour, CPPM(RexxMessage::errorCondition), 0);
  defineKernelMethod(CHAR_SEND     , TheMessageBehaviour, CPPM(RexxMessage::send), 1);
  defineKernelMethod(CHAR_START    , TheMessageBehaviour, CPPM(RexxMessage::start), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMessageBehaviour->setMethodDictionaryScope(TheMessageClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheMessageClass->subClassable(true);

  /***************************************************************************/
  /*           METHOD                                                        */
  /***************************************************************************/

                                       /* Add the NEW methods to the        */
                                       /* class behaviour                   */
  defineKernelMethod(CHAR_NEW     , TheMethodClassBehaviour, CPPM(RexxMethod::newRexx), A_COUNT);
  defineKernelMethod(CHAR_NEWFILE , TheMethodClassBehaviour, CPPM(RexxMethod::newFileRexx), 1);
  defineKernelMethod("LOADEXTERNALMETHOD" , TheMethodClassBehaviour, CPPM(RexxMethod::loadExternalMethod), 2);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMethodClassBehaviour->setMethodDictionaryScope(TheMethodClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_SETUNGUARDED ,TheMethodBehaviour, CPPM(RexxMethod::setUnguardedRexx), 0);
  defineKernelMethod(CHAR_SETGUARDED   ,TheMethodBehaviour, CPPM(RexxMethod::setGuardedRexx), 0);
  defineKernelMethod(CHAR_SETPRIVATE   ,TheMethodBehaviour, CPPM(RexxMethod::setPrivateRexx), 0);
  defineKernelMethod(CHAR_ISGUARDED    ,TheMethodBehaviour, CPPM(RexxMethod::isGuardedRexx), 0);
  defineKernelMethod(CHAR_ISPRIVATE    ,TheMethodBehaviour, CPPM(RexxMethod::isPrivateRexx), 0);
  defineKernelMethod(CHAR_ISPROTECTED  ,TheMethodBehaviour, CPPM(RexxMethod::isProtectedRexx), 0);
  defineProtectedKernelMethod(CHAR_SETPROTECTED ,TheMethodBehaviour, CPPM(RexxMethod::setProtectedRexx), 0);
  defineProtectedKernelMethod(CHAR_SETSECURITYMANAGER ,TheMethodBehaviour, CPPM(RexxMethod::setSecurityManager), 1);
  defineKernelMethod(CHAR_SOURCE       ,TheMethodBehaviour, CPPM(BaseExecutable::source), 0);
  defineKernelMethod(CHAR_PACKAGE      ,TheMethodBehaviour, CPPM(BaseExecutable::getPackage), 0);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMethodBehaviour->setMethodDictionaryScope(TheMethodClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheMethodClass->subClassable(true);

  /***************************************************************************/
  /*           ROUTINE                                                       */
  /***************************************************************************/

                                       /* Add the NEW methods to the        */
                                       /* class behaviour                   */
  defineKernelMethod(CHAR_NEW     , TheRoutineClassBehaviour, CPPM(RoutineClass::newRexx), A_COUNT);
  defineKernelMethod(CHAR_NEWFILE , TheRoutineClassBehaviour, CPPM(RoutineClass::newFileRexx), 1);
  defineKernelMethod("LOADEXTERNALROUTINE" , TheRoutineClassBehaviour, CPPM(RoutineClass::loadExternalRoutine), 2);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRoutineClassBehaviour->setMethodDictionaryScope(TheRoutineClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineProtectedKernelMethod(CHAR_SETSECURITYMANAGER ,TheRoutineBehaviour, CPPM(RoutineClass::setSecurityManager), 1);
  defineKernelMethod(CHAR_SOURCE       ,TheRoutineBehaviour, CPPM(BaseExecutable::source), 0);
  defineKernelMethod(CHAR_PACKAGE      ,TheRoutineBehaviour, CPPM(BaseExecutable::getPackage), 0);
  defineKernelMethod(CHAR_CALL         ,TheRoutineBehaviour, CPPM(RoutineClass::callRexx), A_COUNT);
  defineKernelMethod(CHAR_CALLWITH     ,TheRoutineBehaviour, CPPM(RoutineClass::callWithRexx), 1);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRoutineBehaviour->setMethodDictionaryScope(TheRoutineClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheRoutineClass->subClassable(true);


  /***************************************************************************/
  /*           Package                                                       */
  /***************************************************************************/

                                       /* Add the NEW methods to the        */
                                       /* class behaviour                   */
  defineKernelMethod(CHAR_NEW     , ThePackageClassBehaviour, CPPM(PackageClass::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  ThePackageClassBehaviour->setMethodDictionaryScope(ThePackageClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineProtectedKernelMethod(CHAR_SETSECURITYMANAGER ,ThePackageBehaviour, CPPM(PackageClass::setSecurityManager), 1);
  defineKernelMethod(CHAR_SOURCE      ,ThePackageBehaviour, CPPM(PackageClass::getSource), 0);
  defineKernelMethod(CHAR_SOURCELINE  ,ThePackageBehaviour, CPPM(PackageClass::getSourceLineRexx), 1);
  defineKernelMethod(CHAR_SOURCESIZE  ,ThePackageBehaviour, CPPM(PackageClass::getSourceSize), 0);
  defineKernelMethod(CHAR_CLASSES     ,ThePackageBehaviour, CPPM(PackageClass::getClasses), 0);
  defineKernelMethod(CHAR_PUBLICCLASSES ,ThePackageBehaviour, CPPM(PackageClass::getPublicClasses), 0);
  defineKernelMethod(CHAR_IMPORTEDCLASSES ,ThePackageBehaviour, CPPM(PackageClass::getImportedClasses), 0);
  defineKernelMethod(CHAR_DEFINEDMETHODS, ThePackageBehaviour, CPPM(PackageClass::getMethods), 0);
  defineKernelMethod(CHAR_ROUTINES    ,ThePackageBehaviour, CPPM(PackageClass::getRoutines), 0);
  defineKernelMethod(CHAR_PUBLICROUTINES    ,ThePackageBehaviour, CPPM(PackageClass::getPublicRoutines), 0);
  defineKernelMethod(CHAR_IMPORTEDROUTINES    ,ThePackageBehaviour, CPPM(PackageClass::getImportedRoutines), 0);
  defineKernelMethod(CHAR_IMPORTEDPACKAGES    ,ThePackageBehaviour, CPPM(PackageClass::getImportedPackages), 0);
  defineKernelMethod(CHAR_LOADPACKAGE         ,ThePackageBehaviour, CPPM(PackageClass::loadPackage), 2);
  defineKernelMethod(CHAR_ADDPACKAGE          ,ThePackageBehaviour, CPPM(PackageClass::addPackage), 1);
  defineKernelMethod(CHAR_FINDCLASS           ,ThePackageBehaviour, CPPM(PackageClass::findClassRexx), 1);
  defineKernelMethod(CHAR_FINDROUTINE         ,ThePackageBehaviour, CPPM(PackageClass::findRoutineRexx), 1);
  defineKernelMethod(CHAR_ADDROUTINE          ,ThePackageBehaviour, CPPM(PackageClass::addRoutine), 2);
  defineKernelMethod(CHAR_ADDPUBLICROUTINE    ,ThePackageBehaviour, CPPM(PackageClass::addPublicRoutine), 2);
  defineKernelMethod(CHAR_ADDCLASS            ,ThePackageBehaviour, CPPM(PackageClass::addClass), 2);
  defineKernelMethod(CHAR_ADDPUBLICCLASS      ,ThePackageBehaviour, CPPM(PackageClass::addPublicClass), 2);
  defineKernelMethod(CHAR_NAME                ,ThePackageBehaviour, CPPM(PackageClass::getName), 0);
  defineKernelMethod("LOADLIBRARY"            ,ThePackageBehaviour, CPPM(PackageClass::loadLibrary), 1);
  defineKernelMethod("DIGITS"                 ,ThePackageBehaviour, CPPM(PackageClass::digits), 0);
  defineKernelMethod("FORM"                   ,ThePackageBehaviour, CPPM(PackageClass::form), 0);
  defineKernelMethod("FUZZ"                   ,ThePackageBehaviour, CPPM(PackageClass::fuzz), 0);
  defineKernelMethod("TRACE"                  ,ThePackageBehaviour, CPPM(PackageClass::trace), 0);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  ThePackageBehaviour->setMethodDictionaryScope(ThePackageClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  ThePackageClass->subClassable(true);


  /***************************************************************************/
  /*           RexxContext                                                   */
  /***************************************************************************/

                                       /* Add the NEW methods to the        */
                                       /* class behaviour                   */
  defineKernelMethod(CHAR_NEW     ,TheRexxContextClassBehaviour, CPPM(RexxContext::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRexxContextBehaviour->setMethodDictionaryScope(TheRexxContextClass);

  defineKernelMethod(CHAR_COPY          ,TheRexxContextBehaviour, CPPM(RexxContext::copyRexx), 0);
  defineKernelMethod(CHAR_PACKAGE       ,TheRexxContextBehaviour, CPPM(RexxContext::getPackage), 0);
  defineKernelMethod(CHAR_EXECUTABLE    ,TheRexxContextBehaviour, CPPM(RexxContext::getExecutable), 0);
  defineKernelMethod(CHAR_FORM          ,TheRexxContextBehaviour, CPPM(RexxContext::getForm), 0);
  defineKernelMethod(CHAR_FUZZ          ,TheRexxContextBehaviour, CPPM(RexxContext::getFuzz), 0);
  defineKernelMethod(CHAR_DIGITS        ,TheRexxContextBehaviour, CPPM(RexxContext::getDigits), 0);
  defineKernelMethod(CHAR_VARIABLES     ,TheRexxContextBehaviour, CPPM(RexxContext::getVariables), 0);
  defineKernelMethod(CHAR_ARGS          ,TheRexxContextBehaviour, CPPM(RexxContext::getArgs), 0);
  defineKernelMethod(CHAR_CONDITION     ,TheRexxContextBehaviour, CPPM(RexxContext::getCondition), 0);
  defineKernelMethod("LINE"             ,TheRexxContextBehaviour, CPPM(RexxContext::getLine), 0);
  defineKernelMethod("RS"               ,TheRexxContextBehaviour, CPPM(RexxContext::getRS), 0);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRexxContextBehaviour->setMethodDictionaryScope(TheRexxContextClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheRexxContextClass->subClassable(true);

  /***************************************************************************/
  /*           QUEUE                                                         */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheQueueClassBehaviour, CPPM(RexxQueue::newRexx), A_COUNT);
  defineKernelMethod(CHAR_OF,  TheQueueClassBehaviour, CPPM(RexxQueue::ofRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheQueueClassBehaviour->setMethodDictionaryScope(TheQueueClass);

                                       /* Add the instance methods to the   */
                                       /* instance method mdict             */

  defineKernelMethod(CHAR_MAKEARRAY     ,TheQueueBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_ITEMS         ,TheQueueBehaviour, CPPM(RexxList::itemsRexx), 0);
  defineKernelMethod(CHAR_SUPPLIER      ,TheQueueBehaviour, CPPM(RexxQueue::supplier), 0);
  defineKernelMethod(CHAR_PUSH          ,TheQueueBehaviour, CPPM(RexxQueue::pushRexx), 1);
  defineKernelMethod(CHAR_PEEK          ,TheQueueBehaviour, CPPM(RexxQueue::peek), 0);
  defineKernelMethod(CHAR_PULL          ,TheQueueBehaviour, CPPM(RexxQueue::pullRexx), 0);
  defineKernelMethod(CHAR_QUEUE         ,TheQueueBehaviour, CPPM(RexxQueue::queueRexx), 1);
  defineKernelMethod(CHAR_BRACKETS      ,TheQueueBehaviour, CPPM(RexxQueue::at), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL ,TheQueueBehaviour, CPPM(RexxQueue::put), 2);
  defineKernelMethod(CHAR_AT            ,TheQueueBehaviour, CPPM(RexxQueue::at), 1);
  defineKernelMethod(CHAR_HASINDEX      ,TheQueueBehaviour, CPPM(RexxQueue::hasindex), 1);
  defineKernelMethod(CHAR_PUT           ,TheQueueBehaviour, CPPM(RexxQueue::put), 2);
  defineKernelMethod(CHAR_REMOVE        ,TheQueueBehaviour, CPPM(RexxQueue::remove), 1);
  defineKernelMethod(CHAR_APPEND        ,TheQueueBehaviour, CPPM(RexxQueue::append), 1);
  defineKernelMethod(CHAR_ALLITEMS      ,TheQueueBehaviour, CPPM(RexxList::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES    ,TheQueueBehaviour, CPPM(RexxQueue::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY         ,TheQueueBehaviour, CPPM(RexxList::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY       ,TheQueueBehaviour, CPPM(RexxList::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX         ,TheQueueBehaviour, CPPM(RexxQueue::index), 1);
  defineKernelMethod(CHAR_HASITEM       ,TheQueueBehaviour, CPPM(RexxList::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM    ,TheQueueBehaviour, CPPM(RexxList::removeItem), 1);
  defineKernelMethod(CHAR_FIRST         ,TheQueueBehaviour, CPPM(RexxQueue::firstRexx), 0);
  defineKernelMethod(CHAR_LAST          ,TheQueueBehaviour, CPPM(RexxQueue::lastRexx), 0);
  defineKernelMethod(CHAR_NEXT          ,TheQueueBehaviour, CPPM(RexxQueue::next), 1);
  defineKernelMethod(CHAR_PREVIOUS      ,TheQueueBehaviour, CPPM(RexxQueue::previous), 1);
  defineKernelMethod(CHAR_INSERT        ,TheQueueBehaviour, CPPM(RexxQueue::insert), 2);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheQueueBehaviour->setMethodDictionaryScope(TheQueueClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheQueueClass->subClassable(false);

  /***************************************************************************/
  /*           RELATION                                                      */
  /***************************************************************************/

                                       /* Add the NEW method to the         */
                                       /* class behaviour mdict             */
  defineKernelMethod(CHAR_NEW          , TheRelationClassBehaviour, CPPM(RexxRelation::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRelationClassBehaviour->setMethodDictionaryScope(TheRelationClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS     , TheRelationBehaviour, CPPM(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL, TheRelationBehaviour, CPPM(RexxRelation::put), 2);
  defineKernelMethod(CHAR_ALLAT        , TheRelationBehaviour, CPPM(RexxHashTableCollection::allAt), 1);
  defineKernelMethod(CHAR_ALLINDEX     , TheRelationBehaviour, CPPM(RexxRelation::allIndex), 1);
  defineKernelMethod(CHAR_MAKEARRAY    , TheRelationBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           , TheRelationBehaviour, CPPM(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_HASINDEX     , TheRelationBehaviour, CPPM(RexxHashTableCollection::hasIndexRexx), 1);
  defineKernelMethod(CHAR_HASITEM      , TheRelationBehaviour, CPPM(RexxRelation::hasItem), 2);
  defineKernelMethod(CHAR_INDEX        , TheRelationBehaviour, CPPM(RexxHashTableCollection::indexRexx), 1);
  defineKernelMethod(CHAR_ITEMS        , TheRelationBehaviour, CPPM(RexxRelation::itemsRexx), 1);
  defineKernelMethod(CHAR_PUT          , TheRelationBehaviour, CPPM(RexxRelation::put), 2);
  defineKernelMethod(CHAR_REMOVE       , TheRelationBehaviour, CPPM(RexxHashTableCollection::removeRexx), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheRelationBehaviour, CPPM(RexxRelation::removeItemRexx), 2);
  defineKernelMethod(CHAR_SUPPLIER     , TheRelationBehaviour, CPPM(RexxRelation::supplier), 1);
  defineKernelMethod(CHAR_ALLITEMS     , TheRelationBehaviour, CPPM(RexxHashTableCollection::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   , TheRelationBehaviour, CPPM(RexxHashTableCollection::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        , TheRelationBehaviour, CPPM(RexxHashTableCollection::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      , TheRelationBehaviour, CPPM(RexxHashTableCollection::isEmpty), 0);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheRelationBehaviour->setMethodDictionaryScope(TheRelationClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheRelationClass->subClassable(false);

  /***************************************************************************/
  /*           STEM                                                          */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheStemClassBehaviour, CPPM(RexxStem::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStemClassBehaviour->setMethodDictionaryScope(TheStemClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS      ,TheStemBehaviour, CPPM(RexxStem::bracket), A_COUNT);
  defineKernelMethod(CHAR_BRACKETSEQUAL ,TheStemBehaviour, CPPM(RexxStem::bracketEqual), A_COUNT);
  defineKernelMethod(CHAR_AT            ,TheStemBehaviour, CPPM(RexxStem::bracket), A_COUNT);
  defineKernelMethod(CHAR_PUT           ,TheStemBehaviour, CPPM(RexxStem::bracketEqual), A_COUNT);
  defineKernelMethod(CHAR_MAKEARRAY     ,TheStemBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_REQUEST       ,TheStemBehaviour, CPPM(RexxStem::request), 1);
  defineKernelMethod(CHAR_SUPPLIER      ,TheStemBehaviour, CPPM(RexxStem::supplier), 0);
  defineKernelMethod(CHAR_ALLINDEXES    ,TheStemBehaviour, CPPM(RexxStem::allIndexes), 0);
  defineKernelMethod(CHAR_ALLITEMS      ,TheStemBehaviour, CPPM(RexxStem::allItems), 0);
  defineKernelMethod(CHAR_EMPTY         ,TheStemBehaviour, CPPM(RexxStem::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY       ,TheStemBehaviour, CPPM(RexxStem::isEmpty), 0);
  defineKernelMethod(CHAR_UNKNOWN       ,TheStemBehaviour, CPPM(RexxObject::unknownRexx), 2);

  defineKernelMethod(CHAR_ITEMS         ,TheStemBehaviour, CPPM(RexxStem::itemsRexx), 0);
  defineKernelMethod(CHAR_HASINDEX      ,TheStemBehaviour, CPPM(RexxStem::hasIndex), A_COUNT);
  defineKernelMethod(CHAR_REMOVE        ,TheStemBehaviour, CPPM(RexxStem::remove), A_COUNT);
  defineKernelMethod(CHAR_INDEX         ,TheStemBehaviour, CPPM(RexxStem::index), 1);
  defineKernelMethod(CHAR_HASITEM       ,TheStemBehaviour, CPPM(RexxStem::hasItem), 1);
  defineKernelMethod(CHAR_REMOVEITEM    ,TheStemBehaviour, CPPM(RexxStem::removeItem), 1);
  defineKernelMethod(CHAR_TODIRECTORY   ,TheStemBehaviour, CPPM(RexxStem::toDirectory), 0);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStemBehaviour->setMethodDictionaryScope(TheStemClass);

                                       /* delete these methods from stems by*/
                                       /* using .nil as the methobj         */
  TheStemBehaviour->define(getGlobalName(CHAR_STRICT_EQUAL)          , OREF_NULL);
  TheStemBehaviour->define(getGlobalName(CHAR_EQUAL)                 , OREF_NULL);
  TheStemBehaviour->define(getGlobalName(CHAR_STRICT_BACKSLASH_EQUAL), OREF_NULL);
  TheStemBehaviour->define(getGlobalName(CHAR_BACKSLASH_EQUAL)       , OREF_NULL);
  TheStemBehaviour->define(getGlobalName(CHAR_LESSTHAN_GREATERTHAN)  , OREF_NULL);
  TheStemBehaviour->define(getGlobalName(CHAR_GREATERTHAN_LESSTHAN)  , OREF_NULL);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheStemClass->subClassable(false);

  /***************************************************************************/
  /*           STRING                                                        */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheStringClassBehaviour, CPPM(RexxString::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStringClassBehaviour->setMethodDictionaryScope(TheStringClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_NULLSTRING                   ,TheStringBehaviour, CPPM(RexxString::concatRexx), 1);
  defineKernelMethod(CHAR_BLANK                        ,TheStringBehaviour, CPPM(RexxString::concatBlank), 1);
  defineKernelMethod(CHAR_CONCATENATE                  ,TheStringBehaviour, CPPM(RexxString::concatRexx), 1);
  defineKernelMethod(CHAR_LENGTH                       ,TheStringBehaviour, CPPM(RexxString::lengthRexx), 0);
  defineKernelMethod(CHAR_CENTER                       ,TheStringBehaviour, CPPM(RexxString::center), 2);
  defineKernelMethod(CHAR_CENTRE                       ,TheStringBehaviour, CPPM(RexxString::center), 2);
  defineKernelMethod(CHAR_DATATYPE                     ,TheStringBehaviour, CPPM(RexxString::dataType), 1);
  defineKernelMethod(CHAR_DELSTR                       ,TheStringBehaviour, CPPM(RexxString::delstr), 2);
  defineKernelMethod(CHAR_FORMAT                       ,TheStringBehaviour, CPPM(RexxString::format), 4);
  defineKernelMethod(CHAR_INSERT                       ,TheStringBehaviour, CPPM(RexxString::insert), 4);
  defineKernelMethod(CHAR_LEFT                         ,TheStringBehaviour, CPPM(RexxString::left), 2);
  defineKernelMethod(CHAR_OVERLAY                      ,TheStringBehaviour, CPPM(RexxString::overlay), 4);
  defineKernelMethod(CHAR_REPLACEAT                    ,TheStringBehaviour, CPPM(RexxString::replaceAt), 4);
  defineKernelMethod(CHAR_REVERSE                      ,TheStringBehaviour, CPPM(RexxString::reverse), 0);
  defineKernelMethod(CHAR_RIGHT                        ,TheStringBehaviour, CPPM(RexxString::right), 2);
  defineKernelMethod(CHAR_STRIP                        ,TheStringBehaviour, CPPM(RexxString::strip), 2);
  defineKernelMethod(CHAR_SUBSTR                       ,TheStringBehaviour, CPPM(RexxString::substr), 3);
  defineKernelMethod(CHAR_SUBCHAR                      ,TheStringBehaviour, CPPM(RexxString::subchar), 1);
  defineKernelMethod(CHAR_DELWORD                      ,TheStringBehaviour, CPPM(RexxString::delWord), 2);
  defineKernelMethod(CHAR_SPACE                        ,TheStringBehaviour, CPPM(RexxString::space), 2);
  defineKernelMethod(CHAR_SUBWORD                      ,TheStringBehaviour, CPPM(RexxString::subWord), 2);
  defineKernelMethod(CHAR_TRUNC                        ,TheStringBehaviour, CPPM(RexxString::trunc), 1);
  defineKernelMethod(CHAR_WORD                         ,TheStringBehaviour, CPPM(RexxString::word), 1);
  defineKernelMethod(CHAR_WORDINDEX                    ,TheStringBehaviour, CPPM(RexxString::wordIndex), 1);
  defineKernelMethod(CHAR_WORDLENGTH                   ,TheStringBehaviour, CPPM(RexxString::wordLength), 1);
  defineKernelMethod(CHAR_WORDPOS                      ,TheStringBehaviour, CPPM(RexxString::wordPos), 2);
  defineKernelMethod(CHAR_CASELESSWORDPOS              ,TheStringBehaviour, CPPM(RexxString::caselessWordPos), 2);
  defineKernelMethod(CHAR_WORDS                        ,TheStringBehaviour, CPPM(RexxString::words), 0);
  defineKernelMethod(CHAR_ABBREV                       ,TheStringBehaviour, CPPM(RexxString::abbrev), 2);
  defineKernelMethod(CHAR_CASELESSABBREV               ,TheStringBehaviour, CPPM(RexxString::caselessAbbrev), 2);
  defineKernelMethod(CHAR_CHANGESTR                    ,TheStringBehaviour, CPPM(RexxString::changeStr), 3);
  defineKernelMethod(CHAR_CASELESSCHANGESTR            ,TheStringBehaviour, CPPM(RexxString::caselessChangeStr), 3);
  defineKernelMethod(CHAR_COMPARE                      ,TheStringBehaviour, CPPM(RexxString::compare), 2);
  defineKernelMethod(CHAR_CASELESSCOMPARE              ,TheStringBehaviour, CPPM(RexxString::caselessCompare), 2);
  defineKernelMethod(CHAR_COPIES                       ,TheStringBehaviour, CPPM(RexxString::copies), 1);
  defineKernelMethod(CHAR_COUNTSTR                     ,TheStringBehaviour, CPPM(RexxString::countStrRexx), 1);
  defineKernelMethod(CHAR_CASELESSCOUNTSTR             ,TheStringBehaviour, CPPM(RexxString::caselessCountStrRexx), 1);
  defineKernelMethod(CHAR_LASTPOS                      ,TheStringBehaviour, CPPM(RexxString::lastPosRexx), 3);
  defineKernelMethod(CHAR_POS                          ,TheStringBehaviour, CPPM(RexxString::posRexx), 3);
  defineKernelMethod(CHAR_CASELESSLASTPOS              ,TheStringBehaviour, CPPM(RexxString::caselessLastPosRexx), 3);
  defineKernelMethod(CHAR_CASELESSPOS                  ,TheStringBehaviour, CPPM(RexxString::caselessPosRexx), 3);
  defineKernelMethod(CHAR_TRANSLATE                    ,TheStringBehaviour, CPPM(RexxString::translate), 5);
  defineKernelMethod(CHAR_VERIFY                       ,TheStringBehaviour, CPPM(RexxString::verify), 4);
  defineKernelMethod(CHAR_BITAND                       ,TheStringBehaviour, CPPM(RexxString::bitAnd), 2);
  defineKernelMethod(CHAR_BITOR                        ,TheStringBehaviour, CPPM(RexxString::bitOr), 2);
  defineKernelMethod(CHAR_BITXOR                       ,TheStringBehaviour, CPPM(RexxString::bitXor), 2);
  defineKernelMethod(CHAR_B2X                          ,TheStringBehaviour, CPPM(RexxString::b2x), 0);
  defineKernelMethod(CHAR_C2D                          ,TheStringBehaviour, CPPM(RexxString::c2d), 1);
  defineKernelMethod(CHAR_C2X                          ,TheStringBehaviour, CPPM(RexxString::c2x), 0);
  defineKernelMethod(CHAR_D2C                          ,TheStringBehaviour, CPPM(RexxString::d2c), 1);
  defineKernelMethod(CHAR_D2X                          ,TheStringBehaviour, CPPM(RexxString::d2x), 1);
  defineKernelMethod(CHAR_X2B                          ,TheStringBehaviour, CPPM(RexxString::x2b), 0);
  defineKernelMethod(CHAR_X2C                          ,TheStringBehaviour, CPPM(RexxString::x2c), 0);
  defineKernelMethod(CHAR_X2D                          ,TheStringBehaviour, CPPM(RexxString::x2d), 1);
  defineKernelMethod(CHAR_ENCODEBASE64                 ,TheStringBehaviour, CPPM(RexxString::encodeBase64), 0);
  defineKernelMethod(CHAR_DECODEBASE64                 ,TheStringBehaviour, CPPM(RexxString::decodeBase64), 0);
  defineKernelMethod(CHAR_MAKESTRING                   ,TheStringBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_ABS                          ,TheStringBehaviour, CPPM(RexxString::abs), 0);
  defineKernelMethod(CHAR_MAX                          ,TheStringBehaviour, CPPM(RexxString::Max), A_COUNT);
  defineKernelMethod(CHAR_MIN                          ,TheStringBehaviour, CPPM(RexxString::Min), A_COUNT);
  defineKernelMethod(CHAR_SIGN                         ,TheStringBehaviour, CPPM(RexxString::sign), 0);
  defineKernelMethod(CHAR_EQUAL                        ,TheStringBehaviour, CPPM(RexxString::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,TheStringBehaviour, CPPM(RexxString::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN         ,TheStringBehaviour, CPPM(RexxString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN         ,TheStringBehaviour, CPPM(RexxString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN                  ,TheStringBehaviour, CPPM(RexxString::isGreaterThan), 1);
  defineKernelMethod(CHAR_LESSTHAN                     ,TheStringBehaviour, CPPM(RexxString::isLessThan), 1);
  defineKernelMethod(CHAR_GREATERTHAN_EQUAL            ,TheStringBehaviour, CPPM(RexxString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_LESSTHAN           ,TheStringBehaviour, CPPM(RexxString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_EQUAL               ,TheStringBehaviour, CPPM(RexxString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_GREATERTHAN        ,TheStringBehaviour, CPPM(RexxString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,TheStringBehaviour, CPPM(RexxString::strictEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,TheStringBehaviour, CPPM(RexxString::strictNotEqual), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN           ,TheStringBehaviour, CPPM(RexxString::strictGreaterThan), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN              ,TheStringBehaviour, CPPM(RexxString::strictLessThan), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN_EQUAL     ,TheStringBehaviour, CPPM(RexxString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_LESSTHAN    ,TheStringBehaviour, CPPM(RexxString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN_EQUAL        ,TheStringBehaviour, CPPM(RexxString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN ,TheStringBehaviour, CPPM(RexxString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_PLUS                         ,TheStringBehaviour, CPPM(RexxString::plus), 1);
  defineKernelMethod(CHAR_SUBTRACT                     ,TheStringBehaviour, CPPM(RexxString::minus), 1);
  defineKernelMethod(CHAR_MULTIPLY                     ,TheStringBehaviour, CPPM(RexxString::multiply), 1);
  defineKernelMethod(CHAR_POWER                        ,TheStringBehaviour, CPPM(RexxString::power), 1);
  defineKernelMethod(CHAR_DIVIDE                       ,TheStringBehaviour, CPPM(RexxString::divide), 1);
  defineKernelMethod(CHAR_INTDIV                       ,TheStringBehaviour, CPPM(RexxString::integerDivide), 1);
  defineKernelMethod(CHAR_REMAINDER                    ,TheStringBehaviour, CPPM(RexxString::remainder), 1);
  defineKernelMethod(CHAR_BACKSLASH                    ,TheStringBehaviour, CPPM(RexxString::notOp), 0);
  defineKernelMethod(CHAR_AND                          ,TheStringBehaviour, CPPM(RexxString::andOp), 1);
  defineKernelMethod(CHAR_OR                           ,TheStringBehaviour, CPPM(RexxString::orOp), 1);
  defineKernelMethod(CHAR_XOR                          ,TheStringBehaviour, CPPM(RexxString::xorOp), 1);
  defineKernelMethod(CHAR_MAKEARRAY                    ,TheStringBehaviour, CPPM(RexxString::makeArray), 1);
  defineKernelMethod(CHAR_LOWER                        ,TheStringBehaviour, CPPM(RexxString::lowerRexx), 2);
  defineKernelMethod(CHAR_UPPER                        ,TheStringBehaviour, CPPM(RexxString::upperRexx), 2);
  defineKernelMethod(CHAR_MATCH                        ,TheStringBehaviour, CPPM(RexxString::match), 4);
  defineKernelMethod(CHAR_CASELESSMATCH                ,TheStringBehaviour, CPPM(RexxString::caselessMatch), 4);
  defineKernelMethod(CHAR_MATCHCHAR                    ,TheStringBehaviour, CPPM(RexxString::matchChar), 2);
  defineKernelMethod(CHAR_CASELESSMATCHCHAR            ,TheStringBehaviour, CPPM(RexxString::caselessMatchChar), 2);
  defineKernelMethod(CHAR_EQUALS                       ,TheStringBehaviour, CPPM(RexxString::equals), 1);
  defineKernelMethod(CHAR_CASELESSEQUALS               ,TheStringBehaviour, CPPM(RexxString::caselessEquals), 1);
  defineKernelMethod(CHAR_COMPARETO                    ,TheStringBehaviour, CPPM(RexxString::compareToRexx), 3);
  defineKernelMethod(CHAR_CASELESSCOMPARETO            ,TheStringBehaviour, CPPM(RexxString::caselessCompareToRexx), 3);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStringBehaviour->setMethodDictionaryScope(TheStringClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheStringClass->subClassable(false);


  /***************************************************************************/
  /*           MUTABLEBUFFER                                                 */
  /***************************************************************************/

                                       /* Add the NEW method to the class   */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheMutableBufferClassBehaviour, CPPM(RexxMutableBufferClass::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMutableBufferClassBehaviour->setMethodDictionaryScope(TheMutableBufferClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_APPEND                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::append), 1);
  defineKernelMethod(CHAR_INSERT                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::insert), 4);
  defineKernelMethod(CHAR_OVERLAY                      ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::overlay), 4);
  defineKernelMethod(CHAR_REPLACEAT                    ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::replaceAt), 4);
  defineKernelMethod(CHAR_DELETE                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::mydelete), 2);
  defineKernelMethod(CHAR_DELSTR                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::mydelete), 2);
  defineKernelMethod(CHAR_SUBSTR                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::substr), 3);
  defineKernelMethod(CHAR_POS                          ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::posRexx), 3);
  defineKernelMethod(CHAR_LASTPOS                      ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::lastPos), 3);
  defineKernelMethod(CHAR_CASELESSPOS                  ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessPos), 3);
  defineKernelMethod(CHAR_CASELESSLASTPOS              ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessLastPos), 3);
  defineKernelMethod(CHAR_SUBCHAR                      ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::subchar), 1);
  defineKernelMethod(CHAR_GETBUFFERSIZE                ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::getBufferSize), 0);
  defineKernelMethod(CHAR_SETBUFFERSIZE                ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::setBufferSize), 1);

  defineKernelMethod(CHAR_LENGTH                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::lengthRexx), 0);
  defineKernelMethod(CHAR_MAKEARRAY                    ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::makearray), 1);
  defineKernelMethod(CHAR_STRING                       ,TheMutableBufferBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_COUNTSTR                     ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::countStrRexx), 1);
  defineKernelMethod(CHAR_CASELESSCOUNTSTR             ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessCountStrRexx), 1);
  defineKernelMethod(CHAR_CHANGESTR                    ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::changeStr), 3);
  defineKernelMethod(CHAR_CASELESSCHANGESTR            ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessChangeStr), 3);
  defineKernelMethod(CHAR_UPPER                        ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::upper), 2);
  defineKernelMethod(CHAR_LOWER                        ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::lower), 2);
  defineKernelMethod(CHAR_TRANSLATE                    ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::translate), 5);
  defineKernelMethod(CHAR_MATCH                        ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::match), 4);
  defineKernelMethod(CHAR_CASELESSMATCH                ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessMatch), 4);
  defineKernelMethod(CHAR_MATCHCHAR                    ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::matchChar), 2);
  defineKernelMethod(CHAR_CASELESSMATCHCHAR            ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessMatchChar), 2);
  defineKernelMethod(CHAR_VERIFY                       ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::verify), 4);
  defineKernelMethod(CHAR_SUBWORD                      ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::subWord), 2);
  defineKernelMethod(CHAR_WORD                         ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::word), 1);
  defineKernelMethod(CHAR_WORDINDEX                    ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::wordIndex), 1);
  defineKernelMethod(CHAR_WORDLENGTH                   ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::wordLength), 1);
  defineKernelMethod(CHAR_WORDS                        ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::words), 0);
  defineKernelMethod(CHAR_WORDPOS                      ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::wordPos), 2);
  defineKernelMethod(CHAR_CASELESSWORDPOS              ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::caselessWordPos), 2);
  defineKernelMethod(CHAR_DELWORD                      ,TheMutableBufferBehaviour, CPPM(RexxMutableBuffer::delWord), 2);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheMutableBufferBehaviour->setMethodDictionaryScope(TheMutableBufferClass);
                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheMutableBufferClass->subClassable(true);

  /***************************************************************************/
  /*             INTEGER                                                     */
  /***************************************************************************/

    /* If the integer class was set up correctly it would have the           */
    /* class_id method in its own class but instead it points to the one     */
    /* in the string class.                                                 .*/

  defineKernelMethod(CHAR_NEW, TheIntegerClassBehaviour, CPPM(RexxString::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheIntegerClassBehaviour->setMethodDictionaryScope(TheIntegerClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_PLUS                         ,TheIntegerBehaviour, CPPM(RexxInteger::plus), 1);
  defineKernelMethod(CHAR_SUBTRACT                     ,TheIntegerBehaviour, CPPM(RexxInteger::minus), 1);
  defineKernelMethod(CHAR_MULTIPLY                     ,TheIntegerBehaviour, CPPM(RexxInteger::multiply), 1);
  defineKernelMethod(CHAR_POWER                        ,TheIntegerBehaviour, CPPM(RexxInteger::power), 1);
  defineKernelMethod(CHAR_DIVIDE                       ,TheIntegerBehaviour, CPPM(RexxInteger::divide), 1);
  defineKernelMethod(CHAR_INTDIV                       ,TheIntegerBehaviour, CPPM(RexxInteger::integerDivide), 1);
  defineKernelMethod(CHAR_REMAINDER                    ,TheIntegerBehaviour, CPPM(RexxInteger::remainder), 1);
  defineKernelMethod(CHAR_BACKSLASH                    ,TheIntegerBehaviour, CPPM(RexxInteger::notOp), 0);
  defineKernelMethod(CHAR_AND                          ,TheIntegerBehaviour, CPPM(RexxInteger::andOp), 1);
  defineKernelMethod(CHAR_OR                           ,TheIntegerBehaviour, CPPM(RexxInteger::orOp), 1);
  defineKernelMethod(CHAR_XOR                          ,TheIntegerBehaviour, CPPM(RexxInteger::xorOp), 1);
  defineKernelMethod(CHAR_UNKNOWN                      ,TheIntegerBehaviour, CPPM(RexxObject::unknownRexx), 2);
  defineKernelMethod(CHAR_D2C                          ,TheIntegerBehaviour, CPPM(RexxInteger::d2c), 1);
  defineKernelMethod(CHAR_D2X                          ,TheIntegerBehaviour, CPPM(RexxInteger::d2x), 1);
  defineKernelMethod(CHAR_ABS                          ,TheIntegerBehaviour, CPPM(RexxInteger::abs), 0);
  defineKernelMethod(CHAR_MAX                          ,TheIntegerBehaviour, CPPM(RexxInteger::Max), A_COUNT);
  defineKernelMethod(CHAR_MIN                          ,TheIntegerBehaviour, CPPM(RexxInteger::Min), A_COUNT);
  defineKernelMethod(CHAR_SIGN                         ,TheIntegerBehaviour, CPPM(RexxInteger::sign), 0);
  defineKernelMethod(CHAR_EQUAL                        ,TheIntegerBehaviour, CPPM(RexxInteger::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,TheIntegerBehaviour, CPPM(RexxInteger::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN         ,TheIntegerBehaviour, CPPM(RexxInteger::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN         ,TheIntegerBehaviour, CPPM(RexxInteger::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN                  ,TheIntegerBehaviour, CPPM(RexxInteger::isGreaterThan), 1);
  defineKernelMethod(CHAR_LESSTHAN                     ,TheIntegerBehaviour, CPPM(RexxInteger::isLessThan), 1);
  defineKernelMethod(CHAR_GREATERTHAN_EQUAL            ,TheIntegerBehaviour, CPPM(RexxInteger::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_LESSTHAN           ,TheIntegerBehaviour, CPPM(RexxInteger::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_EQUAL               ,TheIntegerBehaviour, CPPM(RexxInteger::isLessOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_GREATERTHAN        ,TheIntegerBehaviour, CPPM(RexxInteger::isLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,TheIntegerBehaviour, CPPM(RexxInteger::strictEqual), 1);
  defineKernelMethod(CHAR_HASHCODE                     ,TheIntegerBehaviour, CPPM(RexxInteger::hashCode), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,TheIntegerBehaviour, CPPM(RexxInteger::strictNotEqual), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN           ,TheIntegerBehaviour, CPPM(RexxInteger::strictGreaterThan), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN              ,TheIntegerBehaviour, CPPM(RexxInteger::strictLessThan), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN_EQUAL     ,TheIntegerBehaviour, CPPM(RexxInteger::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_LESSTHAN    ,TheIntegerBehaviour, CPPM(RexxInteger::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN_EQUAL        ,TheIntegerBehaviour, CPPM(RexxInteger::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN ,TheIntegerBehaviour, CPPM(RexxInteger::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_MAKESTRING                   ,TheIntegerBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_FORMAT                       ,TheIntegerBehaviour, CPPM(RexxInteger::format), 4);
  defineKernelMethod(CHAR_TRUNC                        ,TheIntegerBehaviour, CPPM(RexxInteger::trunc), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheIntegerBehaviour->setMethodDictionaryScope(TheIntegerClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheIntegerClass->subClassable(true);

  /***************************************************************************/
  /*             NUMBERSTRING                                                */
  /***************************************************************************/

     /* If the numberstring class was set up correctly it should have the    */
     /* class_id method in its own class but instead it points to the one    */
     /* in the string class.                                                 */

  defineKernelMethod(CHAR_NEW, TheNumberStringClassBehaviour, CPPM(RexxString::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheNumberStringClassBehaviour->setMethodDictionaryScope(TheNumberStringClass);

                                       /* Add the instance methods to this  */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_UNKNOWN                      ,TheNumberStringBehaviour, CPPM(RexxObject::unknownRexx), 2);
  defineKernelMethod(CHAR_ABS                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::abs), 0);
  defineKernelMethod(CHAR_MAX                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::Max), A_COUNT);
  defineKernelMethod(CHAR_MIN                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::Min), A_COUNT);
  defineKernelMethod(CHAR_SIGN                         ,TheNumberStringBehaviour, CPPM(RexxNumberString::Sign), 0);
  defineKernelMethod(CHAR_D2C                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::d2c), 1);
  defineKernelMethod(CHAR_D2X                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::d2x), 1);
  defineKernelMethod(CHAR_EQUAL                        ,TheNumberStringBehaviour, CPPM(RexxNumberString::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,TheNumberStringBehaviour, CPPM(RexxNumberString::notEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_GREATERTHAN         ,TheNumberStringBehaviour, CPPM(RexxNumberString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN_LESSTHAN         ,TheNumberStringBehaviour, CPPM(RexxNumberString::notEqual), 1);
  defineKernelMethod(CHAR_GREATERTHAN                  ,TheNumberStringBehaviour, CPPM(RexxNumberString::isGreaterThan), 1);
  defineKernelMethod(CHAR_LESSTHAN                     ,TheNumberStringBehaviour, CPPM(RexxNumberString::isLessThan), 1);
  defineKernelMethod(CHAR_GREATERTHAN_EQUAL            ,TheNumberStringBehaviour, CPPM(RexxNumberString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_LESSTHAN           ,TheNumberStringBehaviour, CPPM(RexxNumberString::isGreaterOrEqual), 1);
  defineKernelMethod(CHAR_LESSTHAN_EQUAL               ,TheNumberStringBehaviour, CPPM(RexxNumberString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_BACKSLASH_GREATERTHAN        ,TheNumberStringBehaviour, CPPM(RexxNumberString::isLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictEqual), 1);
  defineKernelMethod(CHAR_HASHCODE                     ,TheNumberStringBehaviour, CPPM(RexxNumberString::hashCode), 0);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictNotEqual), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN           ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictGreaterThan), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN              ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictLessThan), 1);
  defineKernelMethod(CHAR_STRICT_GREATERTHAN_EQUAL     ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_LESSTHAN    ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictGreaterOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_LESSTHAN_EQUAL        ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN ,TheNumberStringBehaviour, CPPM(RexxNumberString::strictLessOrEqual), 1);
  defineKernelMethod(CHAR_PLUS                         ,TheNumberStringBehaviour, CPPM(RexxNumberString::plus), 1);
  defineKernelMethod(CHAR_SUBTRACT                     ,TheNumberStringBehaviour, CPPM(RexxNumberString::minus), 1);
  defineKernelMethod(CHAR_MULTIPLY                     ,TheNumberStringBehaviour, CPPM(RexxNumberString::multiply), 1);
  defineKernelMethod(CHAR_POWER                        ,TheNumberStringBehaviour, CPPM(RexxNumberString::power), 1);
  defineKernelMethod(CHAR_DIVIDE                       ,TheNumberStringBehaviour, CPPM(RexxNumberString::divide), 1);
  defineKernelMethod(CHAR_INTDIV                       ,TheNumberStringBehaviour, CPPM(RexxNumberString::integerDivide), 1);
  defineKernelMethod(CHAR_REMAINDER                    ,TheNumberStringBehaviour, CPPM(RexxNumberString::remainder), 1);
  defineKernelMethod(CHAR_BACKSLASH                    ,TheNumberStringBehaviour, CPPM(RexxNumberString::notOp), 0);
  defineKernelMethod(CHAR_AND                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::andOp), 1);
  defineKernelMethod(CHAR_OR                           ,TheNumberStringBehaviour, CPPM(RexxNumberString::orOp), 1);
  defineKernelMethod(CHAR_XOR                          ,TheNumberStringBehaviour, CPPM(RexxNumberString::xorOp), 1);
  defineKernelMethod(CHAR_MAKESTRING                   ,TheNumberStringBehaviour, CPPM(RexxObject::makeStringRexx), 0);
  defineKernelMethod(CHAR_FORMAT                       ,TheNumberStringBehaviour, CPPM(RexxNumberString::formatRexx), 4);
  defineKernelMethod(CHAR_TRUNC                        ,TheNumberStringBehaviour, CPPM(RexxNumberString::trunc), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheNumberStringBehaviour->setMethodDictionaryScope(TheNumberStringClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheNumberStringClass->subClassable(true);


  /***************************************************************************/
  /*           SUPPLIER                                                      */
  /***************************************************************************/
                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheSupplierClassBehaviour, CPPM(RexxSupplierClass::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheSupplierClassBehaviour->setMethodDictionaryScope(TheSupplierClass);


                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */

  defineKernelMethod(CHAR_AVAILABLE ,TheSupplierBehaviour, CPPM(RexxSupplier::available), 0);
  defineKernelMethod(CHAR_INDEX     ,TheSupplierBehaviour, CPPM(RexxSupplier::index), 0);
  defineKernelMethod(CHAR_NEXT      ,TheSupplierBehaviour, CPPM(RexxSupplier::next), 0);
  defineKernelMethod(CHAR_ITEM      ,TheSupplierBehaviour, CPPM(RexxSupplier::value), 0);
  defineKernelMethod(CHAR_INIT      ,TheSupplierBehaviour, CPPM(RexxSupplier::initRexx), 2);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheSupplierBehaviour->setMethodDictionaryScope(TheSupplierClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheSupplierClass->subClassable(false);

  /***************************************************************************/
  /*           TABLE                                                         */
  /***************************************************************************/

                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW          , TheTableClassBehaviour, CPPM(RexxTable::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheTableClassBehaviour->setMethodDictionaryScope(TheTableClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS     , TheTableBehaviour, CPPM(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL, TheTableBehaviour, CPPM(RexxHashTableCollection::putRexx), 2);
  defineKernelMethod(CHAR_MAKEARRAY    , TheTableBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           , TheTableBehaviour, CPPM(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_HASINDEX     , TheTableBehaviour, CPPM(RexxHashTableCollection::hasIndexRexx), 1);
  defineKernelMethod(CHAR_ITEMS        , TheTableBehaviour, CPPM(RexxTable::itemsRexx), 0);
  defineKernelMethod(CHAR_PUT          , TheTableBehaviour, CPPM(RexxHashTableCollection::putRexx), 2);
  defineKernelMethod(CHAR_REMOVE       , TheTableBehaviour, CPPM(RexxHashTableCollection::removeRexx), 1);
  defineKernelMethod(CHAR_SUPPLIER     , TheTableBehaviour, CPPM(RexxHashTableCollection::supplier), 0);
  defineKernelMethod(CHAR_ALLITEMS     , TheTableBehaviour, CPPM(RexxHashTableCollection::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   , TheTableBehaviour, CPPM(RexxHashTableCollection::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        , TheTableBehaviour, CPPM(RexxHashTableCollection::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      , TheTableBehaviour, CPPM(RexxHashTableCollection::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        , TheTableBehaviour, CPPM(RexxHashTableCollection::indexRexx), 1);
  defineKernelMethod(CHAR_HASITEM      , TheTableBehaviour, CPPM(RexxHashTableCollection::hasItemRexx), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheTableBehaviour, CPPM(RexxHashTableCollection::removeItemRexx), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheTableBehaviour->setMethodDictionaryScope(TheTableClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheTableClass->subClassable(false);

  /***************************************************************************/
  /*           IDENTITYTABLE                                                 */
  /***************************************************************************/

                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW          , TheIdentityTableClassBehaviour, CPPM(RexxIdentityTable::newRexx), A_COUNT);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheIdentityTableClassBehaviour->setMethodDictionaryScope(TheIdentityTableClass);

                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_BRACKETS     , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_BRACKETSEQUAL, TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::putRexx), 2);
  defineKernelMethod(CHAR_MAKEARRAY    , TheIdentityTableBehaviour, CPPM(RexxObject::makeArrayRexx), 0);
  defineKernelMethod(CHAR_AT           , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::getRexx), 1);
  defineKernelMethod(CHAR_HASINDEX     , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::hasIndexRexx), 1);
  defineKernelMethod(CHAR_ITEMS        , TheIdentityTableBehaviour, CPPM(RexxTable::itemsRexx), 0);
  defineKernelMethod(CHAR_PUT          , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::putRexx), 2);
  defineKernelMethod(CHAR_REMOVE       , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::removeRexx), 1);
  defineKernelMethod(CHAR_SUPPLIER     , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::supplier), 0);
  defineKernelMethod(CHAR_ALLITEMS     , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::allItems), 0);
  defineKernelMethod(CHAR_ALLINDEXES   , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::allIndexes), 0);
  defineKernelMethod(CHAR_EMPTY        , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::empty), 0);
  defineKernelMethod(CHAR_ISEMPTY      , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::isEmpty), 0);
  defineKernelMethod(CHAR_INDEX        , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::indexRexx), 1);
  defineKernelMethod(CHAR_HASITEM      , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::hasItemRexx), 1);
  defineKernelMethod(CHAR_REMOVEITEM   , TheIdentityTableBehaviour, CPPM(RexxHashTableCollection::removeItemRexx), 1);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheIdentityTableBehaviour->setMethodDictionaryScope(TheIdentityTableClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheIdentityTableClass->subClassable(false);


  /***************************************************************************/
  /*           POINTER                                                       */
  /***************************************************************************/
                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, ThePointerClassBehaviour, CPPM(RexxPointer::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  ThePointerClassBehaviour->setMethodDictionaryScope(ThePointerClass);


                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_EQUAL                        ,ThePointerBehaviour, CPPM(RexxPointer::equal), 1);
  defineKernelMethod(CHAR_BACKSLASH_EQUAL              ,ThePointerBehaviour, CPPM(RexxPointer::notEqual), 1);
  defineKernelMethod(CHAR_STRICT_EQUAL                 ,ThePointerBehaviour, CPPM(RexxPointer::equal), 1);
  defineKernelMethod(CHAR_STRICT_BACKSLASH_EQUAL       ,ThePointerBehaviour, CPPM(RexxPointer::notEqual), 1);
  defineKernelMethod(CHAR_ISNULL                       ,ThePointerBehaviour, CPPM(RexxPointer::isNull), 0);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  ThePointerBehaviour->setMethodDictionaryScope(ThePointerClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  ThePointerClass->subClassable(false);


  /***************************************************************************/
  /*           BUFFER                                                        */
  /***************************************************************************/
                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheBufferClassBehaviour, CPPM(RexxBuffer::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheBufferClassBehaviour->setMethodDictionaryScope(TheBufferClass);


                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */

  // NO instance methods on buffer

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheBufferBehaviour->setMethodDictionaryScope(TheBufferClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheBufferClass->subClassable(false);


  /***************************************************************************/
  /*           WEAKREFERENCE                                                 */
  /***************************************************************************/
                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheWeakReferenceClassBehaviour, CPPM(WeakReference::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheWeakReferenceClassBehaviour->setMethodDictionaryScope(TheWeakReferenceClass);


                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod(CHAR_VALUE                        ,TheWeakReferenceBehaviour, CPPM(WeakReference::value), 0);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheWeakReferenceBehaviour->setMethodDictionaryScope(TheWeakReferenceClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheWeakReferenceClass->subClassable(false);


  /***************************************************************************/
  /*           STACKFRAME                                                    */
  /***************************************************************************/
                                       /* Add the NEW methods to the class  */
                                       /* behaviour mdict                   */
  defineKernelMethod(CHAR_NEW, TheStackFrameClassBehaviour, CPPM(StackFrameClass::newRexx), A_COUNT);
                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStackFrameClassBehaviour->setMethodDictionaryScope(TheStackFrameClass);


                                       /* Add the instance methods to the   */
                                       /* instance behaviour mdict          */
  defineKernelMethod("NAME", TheStackFrameBehaviour, CPPM(StackFrameClass::getName), 0);
  defineKernelMethod("EXECUTABLE", TheStackFrameBehaviour, CPPM(StackFrameClass::getExecutable), 0);
  defineKernelMethod("LINE", TheStackFrameBehaviour, CPPM(StackFrameClass::getLine), 0);
  defineKernelMethod("TRACELINE", TheStackFrameBehaviour, CPPM(StackFrameClass::getTraceLine), 0);
  defineKernelMethod("TYPE", TheStackFrameBehaviour, CPPM(StackFrameClass::getType), 0);

                                       /* set the scope of the methods to   */
                                       /* this classes oref                 */
  TheStackFrameBehaviour->setMethodDictionaryScope(TheStackFrameClass);

                                       /* Now call the class subclassable   */
                                       /* method                            */
  TheWeakReferenceClass->subClassable(false);

  /***************************************************************************/
  /***************************************************************************/
  /***************************************************************************/
    /* These classes don't have any class methods                            */
    /*  and are not subclassed from object                                   */

#define kernel_public(name, object, dir)  ((RexxDirectory *)dir)->setEntry(getGlobalName(name), (RexxObject *)object)

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
  kernel_public(CHAR_ROUTINE          ,TheRoutineClass ,TheEnvironment);
  kernel_public(CHAR_PACKAGE          ,ThePackageClass ,TheEnvironment);
  kernel_public(CHAR_REXXCONTEXT      ,TheRexxContextClass ,TheEnvironment);
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
  kernel_public(CHAR_IDENTITYTABLE    ,TheIdentityTableClass,TheEnvironment);
  kernel_public(CHAR_POINTER          ,ThePointerClass ,TheEnvironment);
  kernel_public(CHAR_BUFFER           ,TheBufferClass  ,TheEnvironment);
  kernel_public(CHAR_WEAKREFERENCE    ,TheWeakReferenceClass  ,TheEnvironment);
  kernel_public("STACKFRAME"          ,TheStackFrameClass  ,TheEnvironment);
  kernel_public(CHAR_TRUE             ,TheTrueObject   ,TheEnvironment);

  /* set up the kernel directory (MEMORY done elsewhere) */
  kernel_public(CHAR_INTEGER          ,TheIntegerClass     , TheKernel);
  kernel_public(CHAR_NUMBERSTRING     ,TheNumberStringClass, TheKernel);

  // TODO:  Make the kernel directory part of the memory object, but not in the
  // environment.

  kernel_public(CHAR_FUNCTIONS        ,TheFunctionsDirectory  ,TheKernel);
  kernel_public(CHAR_NULLARRAY        ,TheNullArray           ,TheKernel);
  kernel_public(CHAR_NULLPOINTER      ,TheNullPointer         ,TheKernel);
  kernel_public(CHAR_COMMON_RETRIEVERS,TheCommonRetrievers    ,TheKernel);
  kernel_public(CHAR_ENVIRONMENT      ,TheEnvironment         ,TheKernel);

                                       /* set Oryx version                  */
  kernel_public(CHAR_VERSION, Interpreter::getVersionNumber(), TheKernel);
                                       /* set the system name               */
  kernel_public(CHAR_NAME, SystemInterpreter::getSystemName(), TheSystem);
                                       /* set the internal system name      */
  kernel_public(CHAR_INTERNALNAME, SystemInterpreter::getInternalSystemName(), TheSystem);
                                       /* and the system version info       */
  kernel_public(CHAR_VERSION, SystemInterpreter::getSystemVersion(), TheSystem);
  // initialize our thread vector for external calls.
  RexxActivity::initializeThreadContext();

/******************************************************************************/
/*      Complete the image build process, calling BaseClasses to establish    */
/*      the rest of the REXX image.                                           */
/******************************************************************************/

  /* set up the kernel methods that will be defined on OBJECT classes in  */
  /*  BaseClasses.ORX and ORYXJ.ORX.                                            */
  {
                                           /* create a kernel methods directory */
      RexxDirectory *kernel_methods = new_directory();
      ProtectedObject p1(kernel_methods);   // protect from GC
      kernel_methods->put(new RexxMethod(getGlobalName(CHAR_LOCAL), CPPCode::resolveExportedMethod(CHAR_LOCAL, CPPM(RexxLocal::local), 0)), getGlobalName(CHAR_LOCAL));

                                           /* create the BaseClasses method and run it*/
      RexxString *symb = getGlobalName(BASEIMAGELOAD);   /* get a name version of the string  */
                                           /* go resolve the program name       */
      RexxString *programName = ActivityManager::currentActivity->resolveProgramName(symb, OREF_NULL, OREF_NULL);
      // create a new stack frame to run under
      ActivityManager::currentActivity->createNewActivationStack();
      try
      {
                                               /* create a method object out of this*/
          RoutineClass *loader = new RoutineClass(programName);


          RexxObject *args = kernel_methods;   // temporary to avoid type-punning warnings
          ProtectedObject result;
                                               /* now call BaseClasses to finish the image*/
          loader->runProgram(ActivityManager::currentActivity, OREF_PROGRAM, OREF_NULL, (RexxObject **)&args, 1, result);
      }
      catch (ActivityException )
      {
          ActivityManager::currentActivity->error();          /* do error cleanup                  */
          Interpreter::logicError("Error building kernel image.  Image not saved.");
      }

  }

  /* define and suppress methods in the nil object */
  TheNilObject->defMethod(getGlobalName(CHAR_COPY), (RexxMethod *)TheNilObject);
  TheNilObject->defMethod(getGlobalName(CHAR_START), (RexxMethod *)TheNilObject);
  TheNilObject->defMethod(getGlobalName(CHAR_OBJECTNAMEEQUALS), (RexxMethod *)TheNilObject);

  // ok, .NIL has been constructed.  As a last step before saving the image, we need to change
  // the type identifier in the behaviour so that this will get the correct virtual function table
  // restored when the image reloads.
  TheNilObject->behaviour->setClassType(T_NilObject);

  RexxClass *ordered = (RexxClass *)TheEnvironment->get(getGlobalName(CHAR_ORDEREDCOLLECTION));

  TheArrayClass->inherit(ordered, OREF_NULL);
  TheArrayClass->setRexxDefined();

  TheQueueClass->inherit(ordered, OREF_NULL);
  TheQueueClass->setRexxDefined();

  TheListClass->inherit(ordered, OREF_NULL);
  TheListClass->setRexxDefined();

  RexxClass *map = (RexxClass *)TheEnvironment->get(getGlobalName(CHAR_MAPCOLLECTION));

  TheTableClass->inherit(map, OREF_NULL);
  TheTableClass->setRexxDefined();

  TheIdentityTableClass->inherit(map, OREF_NULL);
  TheIdentityTableClass->setRexxDefined();

  TheRelationClass->inherit(map, OREF_NULL);
  TheRelationClass->setRexxDefined();

  TheDirectoryClass->inherit(map, OREF_NULL);
  TheDirectoryClass->setRexxDefined();

  TheStemClass->inherit(map, OREF_NULL);
  TheStemClass->setRexxDefined();

  RexxClass *comparable = (RexxClass *)TheEnvironment->get(getGlobalName(CHAR_COMPARABLE));

  TheStringClass->inherit(comparable, OREF_NULL);
  TheStringClass->setRexxDefined();


  // now save the image
  memoryObject.saveImage();
  ActivityManager::returnActivity(ActivityManager::currentActivity);
  exit(RC_OK);                         // successful build
}

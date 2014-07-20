/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
#include "Envelope.hpp"
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
#include "LanguageParser.hpp"


/**
 * Add a C++ method to an object's behaviour.
 *
 * @param name       The name of the method.
 * @param behaviour  The target behaviour.
 * @param entryPoint The entry point of the C++ method that implements the
 *                   method.
 * @param arguments  The method argument style (argument count or array indicator).
 */
void MemoryObject::defineMethod(const char *name, RexxBehaviour *behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName)
{
    behaviour->defineMethod(name, entryPoint, arguments, entryPointName);
}


/**
 * Add a C++ method to an object's behaviour.  This method is
 * marked as protected.
 *
 * @param name       The name of the method.
 * @param behaviour  The target behaviour.
 * @param entryPoint The entry point of the C++ method that implements the
 *                   method.
 * @param arguments  The method argument style (argument count or array indicator).
 */
void MemoryObject::defineProtectedMethod(const char *name, RexxBehaviour * behaviour, PCPPM entryPoint, size_t arguments)
{
    MethodClass *method = behaviour->defineMethod(name, entryPoint, arguments);
    // mark as protected after the fact
    method->setProtected();
}


/**
 * Add a C++ method to an object's behaviour.  This method is
 * marked as private.
 *
 * @param name       The name of the method.
 * @param behaviour  The target behaviour.
 * @param entryPoint The entry point of the C++ method that implements the
 *                   method.
 * @param arguments  The method argument style (argument count or array indicator).
 */
void MemoryObject::definePrivateKernelMethod(const char *name, RexxBehaviour * behaviour, PCPPM entryPoint, size_t arguments)
{
    MethodClass *method = behaviour->define(name, entryPoint, arguments);
    // mark the method as private
    method->setPrivate();
}


/**
 * Add a object to the environment using the provided name.
 *
 * @param name   The name of the new environment entry.
 * @param value  The value added.
 */
void MemoryObject::addToEnvironment(const char *name, RexxInternalObject *value);
{
    TheEnvironment->put(getUpperGlobalName(name), object);
}


/**
 * Add a object to the kernel directory using the provided name.
 *
 * @param name   The name of the new environment entry.
 * @param value  The value added.
 */
void MemoryObject::addToKernel(const char *name, RexxInternalObject *value);
{
    TheSystem->put(getUpperGlobalName(name), object);
}


/**
 * Initialize the Rexx memory environment during an image built.
 */
void MemoryObject::createImage()
{
    // perform the initial memory environment setup.  We can create
    // new objects once this is done.

    // NOTE:  this creates both the Integer and Class class objects, so those
    // don't appear in the list below

    // TODO:  Assess whether that is still really appropriate.
    MemoryObject::create();

    // initialize the base interpreter subsystem
    Interpreter::init();
    // initialize the activity manager.  We can create active threads once this is done.
    ActivityManager::init();
    // Get an instance.  This also gives the root activity of the instance
    // the kernel lock.
    Interpreter::createInterpreterInstance();
    // create our table of constant string values
    memoryObject.createStrings();
    // initializer for native libraries
    PackageManager::initialize();


    // Create some special Rexx objects.
    TheTrueObject  = new RexxInteger(1);
    TheFalseObject = new RexxInteger(0);

    TheNilObject = new RexxNilObject;

    // start creating the various class objects so that Rexx code can create instances.

    // string and object are fairly critical
    RexxString::createInstance();
    RexxObject::createInstance();

    // The pointer class needs to be created early because other classes
    // use the instances to store information.
    PointerClass::createInstance();

    // Buffer also can be used for internal data
    BufferClass::createInstance();

    // the different collection classes
    ArrayClass::createInstance();
    TableClass::createInstance();
    IdentityTable::createInstance();
    RelationClass::createInstance();
    StringTable::createInstance();
    DirectoryClass::createInstance();
    SetClass::createinstance();
    BagClass::createInstance();
    ListClass::createInstance();
    QueueClass::createInstance();

    // functions directory used for functions like rxqueue.
    TheFunctionsDirectory = new_string_table();

    // We keep handy references to a number of commonly used
    // integer objects.
    IntegerZero    = new_integer(0);
    IntegerOne     = new_integer(1);
    IntegerTwo     = new_integer(2);
    IntegerThree   = new_integer(3);
    IntegerFour    = new_integer(4);
    IntegerFive    = new_integer(5);
    IntegerSix     = new_integer(6);
    IntegerSeven   = new_integer(7);
    IntegerEight   = new_integer(8);
    IntegerNine    = new_integer(9);
    IntegerMinusOne = new_integer(-1);

    // NOTE:  The number string class lies about its identity
    NumberString::createInstance();

    // create the environment directory
    TheEnvironment = new_directory();

    // add kernel and system directories, and mark all of these as proxied objects.
    TheSystem = new_string_table();

    TheEnvironment->makeProxiedObject();
    TheSystem->makeProxiedObject();

    // create more of the exported classes
    MethodClass::createInstance();
    RoutineClass::createInstance();
    PackageClass::createInstance();
    RexxContext::createInstance();
    StemClass::createInstance();
    SupplierClass::createInstance();
    MessageClass::createInstance();
    MutableBuffer::createInstance();

    WeakReference::createInstance();
    StackFrameClass::createInstance();

    // build the common retrievers table.  This is needed before we can parse an
    // Rexx code.
    TheCommonRetrievers = new_string_table();

    // These special variables are always assigned the same slot positions in all
    // Rexx code contexts.
    TheCommonRetrievers->put((RexxObject *)new RexxSimpleVariable(OREF_SELF, VARIABLE_SELF), OREF_SELF);
    TheCommonRetrievers->put((RexxObject *)new RexxSimpleVariable(OREF_SUPER, VARIABLE_SUPER), OREF_SUPER);
    TheCommonRetrievers->put((RexxObject *)new RexxSimpleVariable(OREF_SIGL, VARIABLE_SIGL), OREF_SIGL);
    TheCommonRetrievers->put((RexxObject *)new RexxSimpleVariable(OREF_RC, VARIABLE_RC), OREF_RC);
    TheCommonRetrievers->put((RexxObject *)new RexxSimpleVariable(OREF_RESULT, VARIABLE_RESULT), OREF_RESULT);

//***************************************************************************
// The following Rexx classes that are exposed to the users are set up as
// primitive classes.  These all inherit from object
//***************************************************************************

// macros for simplifying building each class definition.

// start a definition for a new class.  This sets up some variables in a
// local variable scope that identify which behaviours we are working with.
#define StartClassDefinition(name) \
    {\
        // set up variables with both the class and instance behaviours. \
        RexxBehaviour *currentClassBehaviour = The##name##ClassBehaviour; \
        RexxBehaviour *currentInstanceBehaviour = The##name##Behaviour;   \
        RexxClass *currentClass = The##name##Class;

// define a new class method
#define AddClassMethod(name, entryPoint, args) defineMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddClassProtectedMethod(name, entryPoint, args) defineProtectedMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddClassPrivateMethod(name, entryPoint, args) definePrivateMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define HideClassMethod(name) currentClassBehaviour->hideMethod(name);

// inherit class method definitions from a previously created class
#define InheritClassMethods(source) currentClassBehaviour->inheritInstanceMethods(The##source##ClassBehaviour);

// define a new instance method
#define AddMethod(name, entryPoint, args) defineMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddProtectedMethod(name, entryPoint, args) defineProtectedMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddPrivateMethod(name, entryPoint, args) definePrivateMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define HideMethod(name) currentInstanceBehaviour->hideMethod(name);

// inherit instance method definitions from a previously created class
#define InheritInstanceMethods(source) currentInstanceBehaviour->inheritInstanceMethods(The##source##Behaviour);

// do final initialization after done defining class methods
#define CompleteClassMethodDefinitions() \
    currentClassBehaviour->setMethodDictionaryScope(currentClass);

// do final initialization after done defining instance methods
#define CompleteMethodDefinitions() \
    currentInstanceBehaviour->setMethodDictionaryScope(currentClass);

// finalize a simple class definition
#define CompleteClassDefinition(name) The##name##Class->buildFinalClassBehaviour();

// finalize a class definition where a class is subclassed from something other than object.
#define CompleteSubclassedClassDefinition(name, subclass) The##name##Class->buildFinalClassBehaviour(The##subclass##Class);

// Add the created class object to the environment under its name and close
// the local variable scope
#define EndClassDefinition(name) \
    addToEnvironment(#name, currentClass); \
}


// CLASS and OBJECT get some special treatment.  The process of building the final behaviour
// for all of the primitive classes requires information from the CLASS and OBJECT behaviours.
// This is true for CLASS and OBJECT as well (it gets a little recursive at the beginning.
// So, for these two, we define all of the methods that are to be implemented by the classes,
// then after both are built, we can finally build the final behaviours of both classes.
// For all other classes, their behaviours can be built as soon as the definitions are complete.

    /************************************************************************/
    /*                                                                      */
    /* The CLASS  class                                                     */
    /*                                                                      */
    /************************************************************************/
StartClassDefinition(Class);
    // for the CLASS object, we only add the NEW method.
        AddClassMethod(CHAR_NEW, RexxClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // now the normal instance methods for a CLASS object.
        AddProtectedMethod(CHAR_BASECLASS, RexxClass::getBaseClass, 0);
        AddProtectedMethod(CHAR_DEFINE, RexxClass::defineMethod, 2);
        AddProtectedMethod(CHAR_DEFINE_METHODS, RexxClass::defineMethods, 1);
        AddProtectedMethod("!DEFINE_CLASS_METHOD", RexxClass::defineClassMethod, 2);
        AddProtectedMethod("INHERITINSTANCEMETHODS", RexxClass::inheritInstanceMethods, 1)
        AddProtectedMethod(CHAR_DELETE, RexxClass::deleteMethod, 1);
        AddMethod(CHAR_ENHANCED, RexxClass::enhanced, A_COUNT);
        AddMethod(CHAR_ID, RexxClass::getId, 0);
        AddMethod(CHAR_INHERIT, RexxClass::inherit, 2);
        AddProtectedMethod(CHAR_METACLASS, RexxClass::getMetaClass, 0);
        AddMethod(CHAR_METHOD, RexxClass::method, 1);
        AddMethod(CHAR_METHODS, RexxClass::methods, 1);
        AddMethod(CHAR_MIXINCLASS. RexxClass::mixinclassRexx, 3);
        AddMethod(CHAR_QUERYMIXINCLASS, RexxClass::queryMixinClass, 0);
        AddMethod(CHAR_SUBCLASS, RexxClass::subclassRexx, 3);
        AddProtectedMethod(CHAR_SUBCLASSES, RexxClass::getSubClasses, 0);
        AddProtectedMethod(CHAR_SUPERCLASSES, RexxClass::getSuperClasses, 0);
        AddProtectedMethod(CHAR_SUPERCLASS, RexxClass::getSuperClass, 0);
        AddProtectedMethod(CHAR_UNINHERIT, RexxClass::uninherit, 1);

        AddMethod(CHAR_ISSUBCLASSOF, RexxClass::isSubclassOf, 1);
        AddMethod(CHAR_SHRIEKREXXDEFINED, RexxClass::setRexxDefined, 0);
        AddMethod(CHAR_DEFAULTNAME, RexxClass::defaultNameRexx, 0);
        AddMethod(CHAR_PACKAGE, RexxClass::getPackage, 0);

    // operator methods
        AddMethod(CHAR_EQUAL, RexxClass::equal, 1);
        AddMethod(CHAR_STRICT_EQUAL, RexxClass::strictEqual, 1);
        AddMethod(CHAR_BACKSLASH_EQUAL, RexxClass::notEqual, 1);
        AddMethod(CHAR_LESSTHAN_GREATERTHAN, RexxClass::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN_LESSTHAN, RexxClass::notEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_EQUAL, RexxClass::notEqual, 1);

    // this is explicitly inserted into the class behaviour because it gets used
    // prior to the instance behavior merges.
        AddMethod(CHAR_HASHCODE, hashCode, 0);
    // this is a NOP by default, so we'll just use the object init method as a fill in.
        AddMethod(CHAR_ACTIVATE, RexxObject::initRexx, 0);

    CompleteMethodDefinitions();

EndClassDefinition(Class);

       /************************************************************************/
       /*                                                                      */
       /* The OBJECT class                                                     */
       /*                                                                      */
       /************************************************************************/

StartClassDefinition(Object);

    // for the OBJECT class object, we only add the NEW method.
        AddClassMethod(CHAR_NEW, RexxObject::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // Now Object instance methods

        AddMethod(CHAR_INIT, RexxObject::initRexx, 0);
        AddMethod(CHAR_EQUAL, RexxObject::equal, 1);
        AddMethod(CHAR_STRICT_EQUAL, RexxObject::strictEqual, 1);
        AddMethod(CHAR_HASHCODE, RexxObject::hashCode, 0);
        AddMethod(CHAR_BACKSLASH_EQUAL, RexxObject::notEqual, 1);
        AddMethod(CHAR_LESSTHAN_GREATERTHAN, RexxObject::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN_LESSTHAN, RexxObject::notEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_EQUAL, RexxObject::strictNotEqual, 1);
        AddMethod(CHAR_NULLSTRING, RexxObject::concatRexx, 1);
        AddMethod(CHAR_BLANK, RexxObject::concatBlank, 1);
        AddMethod(CHAR_CONCATENATE, RexxObject::concatRexx, 1);
        AddMethod(CHAR_COPY, RexxObject::copyRexx, 0);
        AddMethod(CHAR_CLASS, RexxObject::classObject, 0);
        AddMethod(CHAR_HASMETHOD, RexxObject::hasMethodRexx, 1);
        AddMethod(CHAR_DEFAULTNAME, RexxObject::defaultNameRexx, 0);
        AddMethod(CHAR_OBJECTNAME, RexxObject::objectName, 0);
        AddMethod(CHAR_OBJECTNAMEEQUALS, RexxObject::objectNameEquals, 1);
        AddMethod(CHAR_REQUEST, RexxObject::requestRexx, 1);
        AddMethod(CHAR_START, RexxObject::start, A_COUNT);
        AddMethod("STARTWITH", RexxObject::startWith, 2);
        AddMethod("SEND", RexxObject::send, A_COUNT);
        AddMethod("SENDWITH", RexxObject::sendWith, 2);
        AddMethod(CHAR_STRING, RexxObject::stringRexx, 0);
        AddMethod(CHAR_ISINSTANCEOF, RexxObject::isInstanceOfRexx, 1);
        AddMethod(CHAR_ISA, RexxObject::isInstanceOfRexx, 1);
        AddMethod(CHAR_INSTANCEMETHOD, RexxObject::instanceMethodRexx, 1);
        AddMethod(CHAR_INSTANCEMETHODS, RexxObject::instanceMethodsRexx, 1);
        AddMethod(CHAR_IDENTITYHASH, RexxObject::identityHashRexx, 0);
        AddPrivateMethod(CHAR_RUN, RexxObject::run, A_COUNT);
        AddPrivateMethod(CHAR_SETMETHOD, RexxObject::setMethod, 3);
        AddPrivateMethod(CHAR_UNSETMETHOD, RexxObject::unsetMethod, 1);

    CompleteMethodDefinitions();

EndClassDefinition(Object);

// now we can complete the definitions of both Object and Class (must be in this order)

CompleteClassDefinition(Object);
CompleteClassDefinition(Class);


// Now we can set up the rest of the defintions and complete them normally.


    /***************************************************************************/
    /*           STRING                                                        */
    /***************************************************************************/

StartClassDefinition(String)

        AddClassMethod(CHAR_NEW, RexxString::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_NULLSTRING, RexxString::concatRexx, 1);
        AddMethod(CHAR_BLANK, RexxString::concatBlank, 1);
        AddMethod(CHAR_CONCATENATE, RexxString::concatRexx, 1);
        AddMethod(CHAR_LENGTH, RexxString::lengthRexx, 0);
        AddMethod(CHAR_CENTER, RexxString::center, 2);
        AddMethod(CHAR_CENTRE, RexxString::center, 2);
        AddMethod(CHAR_DATATYPE, RexxString::dataType, 1);
        AddMethod(CHAR_DELSTR, RexxString::delstr, 2);
        AddMethod(CHAR_FORMAT, RexxString::format, 4);
        AddMethod(CHAR_INSERT, RexxString::insert, 4);
        AddMethod(CHAR_LEFT, RexxString::left, 2);
        AddMethod(CHAR_OVERLAY, RexxString::overlay, 4);
        AddMethod(CHAR_REPLACEAT, RexxString::replaceAt, 4);
        AddMethod(CHAR_REVERSE, RexxString::reverse, 0);
        AddMethod(CHAR_RIGHT, RexxString::right, 2);
        AddMethod(CHAR_STRIP, RexxString::strip, 2);
        AddMethod(CHAR_SUBSTR, RexxString::substr, 3);
        AddMethod(CHAR_SUBCHAR, RexxString::subchar, 1);
        AddMethod(CHAR_DELWORD, RexxString::delWord, 2);
        AddMethod(CHAR_SPACE, RexxString::space, 2);
        AddMethod(CHAR_SUBWORD, RexxString::subWord, 2);
        AddMethod("SUBWORDS", RexxString::subWords, 2);
        AddMethod("FLOOR", RexxString::floor, 0);
        AddMethod("CEILING", RexxString::ceiling, 0);
        AddMethod("ROUND", RexxString::round, 0);
        AddMethod(CHAR_TRUNC, RexxString::trunc, 1);
        AddMethod(CHAR_WORD, RexxString::word, 1);
        AddMethod(CHAR_WORDINDEX, RexxString::wordIndex, 1);
        AddMethod(CHAR_WORDLENGTH, RexxString::wordLength, 1);
        AddMethod(CHAR_WORDPOS, RexxString::wordPos, 2);
        AddMethod(CHAR_CASELESSWORDPOS, RexxString::caselessWordPos, 2);
        AddMethod(CHAR_WORDS, RexxString::words, 0);
        AddMethod(CHAR_ABBREV, RexxString::abbrev, 2);
        AddMethod(CHAR_CASELESSABBREV, RexxString::caselessAbbrev, 2);
        AddMethod(CHAR_CHANGESTR, RexxString::changeStr, 3);
        AddMethod(CHAR_CASELESSCHANGESTR, RexxString::caselessChangeStr, 3);
        AddMethod(CHAR_COMPARE, RexxString::compare, 2);
        AddMethod(CHAR_CASELESSCOMPARE, RexxString::caselessCompare, 2);
        AddMethod(CHAR_COPIES, RexxString::copies, 1);
        AddMethod(CHAR_COUNTSTR, RexxString::countStrRexx, 1);
        AddMethod(CHAR_CASELESSCOUNTSTR, RexxString::caselessCountStrRexx, 1);
        AddMethod(CHAR_LASTPOS, RexxString::lastPosRexx, 3);
        AddMethod(CHAR_POS, RexxString::posRexx, 3);
        AddMethod("CONTAINS", RexxString::containsRexx, 3);
        AddMethod("CASELESSCONTAINS", RexxString::caselessContains, 3);
        AddMethod("CONTAINSWORD", RexxString::containsWord, 2);
        AddMethod("CASELESSCONTAINSWORD", RexxString::caselessContainsWord, 2);
        AddMethod(CHAR_CASELESSLASTPOS, RexxString::caselessLastPosRexx, 3);
        AddMethod(CHAR_CASELESSPOS, RexxString::caselessPosRexx, 3);
        AddMethod(CHAR_TRANSLATE, RexxString::translate, 5);
        AddMethod(CHAR_VERIFY, RexxString::verify, 4);
        AddMethod(CHAR_BITAND, RexxString::bitAnd, 2);
        AddMethod(CHAR_BITOR, RexxString::bitOr, 2);
        AddMethod(CHAR_BITXOR, RexxString::bitXor, 2);
        AddMethod(CHAR_B2X, RexxString::b2x, 0);
        AddMethod(CHAR_C2D, RexxString::c2d, 1);
        AddMethod(CHAR_C2X, RexxString::c2x, 0);
        AddMethod(CHAR_D2C, RexxString::d2c, 1);
        AddMethod(CHAR_D2X, RexxString::d2x, 1);
        AddMethod(CHAR_X2B, RexxString::x2b, 0);
        AddMethod(CHAR_X2C, RexxString::x2c, 0);
        AddMethod(CHAR_X2D, RexxString::x2d, 1);
        AddMethod(CHAR_ENCODEBASE64, RexxString::encodeBase64, 0);
        AddMethod(CHAR_DECODEBASE64, RexxString::decodeBase64, 0);
        AddMethod(CHAR_MAKESTRING, RexxObject::makeStringRexx, 0);
        AddMethod(CHAR_ABS, RexxString::abs, 0);
        AddMethod(CHAR_ORXMAX, RexxString::Max, A_COUNT);
        AddMethod(CHAR_ORXMIN, RexxString::Min, A_COUNT);
        AddMethod(CHAR_SIGN, RexxString::sign, 0);
        AddMethod(CHAR_EQUAL, RexxString::equal, 1);
        AddMethod(CHAR_BACKSLASH_EQUAL, RexxString::notEqual, 1);
        AddMethod(CHAR_LESSTHAN_GREATERTHAN, RexxString::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN_LESSTHAN, RexxString::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN, RexxString::isGreaterThan, 1);
        AddMethod(CHAR_LESSTHAN, RexxString::isLessThan, 1);
        AddMethod(CHAR_GREATERTHAN_EQUAL, RexxString::isGreaterOrEqual, 1);
        AddMethod(CHAR_BACKSLASH_LESSTHAN, RexxString::isGreaterOrEqual, 1);
        AddMethod(CHAR_LESSTHAN_EQUAL, RexxString::isLessOrEqual, 1);
        AddMethod(CHAR_BACKSLASH_GREATERTHAN, RexxString::isLessOrEqual, 1);
        AddMethod(CHAR_STRICT_EQUAL, RexxString::strictEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_EQUAL, RexxString::strictNotEqual, 1);
        AddMethod(CHAR_STRICT_GREATERTHAN, RexxString::strictGreaterThan, 1);
        AddMethod(CHAR_STRICT_LESSTHAN, RexxString::strictLessThan, 1);
        AddMethod(CHAR_STRICT_GREATERTHAN_EQUAL, RexxString::strictGreaterOrEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_LESSTHAN, RexxString::strictGreaterOrEqual, 1);
        AddMethod(CHAR_STRICT_LESSTHAN_EQUAL, RexxString::strictLessOrEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN, RexxString::strictLessOrEqual, 1);
        AddMethod(CHAR_PLUS, RexxString::plus, 1);
        AddMethod(CHAR_SUBTRACT, RexxString::minus, 1);
        AddMethod(CHAR_MULTIPLY, RexxString::multiply, 1);
        AddMethod(CHAR_POWER, RexxString::power, 1);
        AddMethod(CHAR_DIVIDE, RexxString::divide, 1);
        AddMethod(CHAR_INTDIV, RexxString::integerDivide, 1);
        AddMethod(CHAR_REMAINDER, RexxString::remainder, 1);
        AddMethod(CHAR_BACKSLASH, RexxString::notOp, 0);
        AddMethod(CHAR_AND, RexxString::andOp, 1);
        AddMethod(CHAR_OR, RexxString::orOp, 1);
        AddMethod(CHAR_XOR, RexxString::xorOp, 1);
        AddMethod(CHAR_MAKEARRAY, RexxString::makeArrayRexx, 1);
        AddMethod(CHAR_LOWER, RexxString::lowerRexx, 2);
        AddMethod(CHAR_UPPER, RexxString::upperRexx, 2);
        AddMethod(CHAR_MATCH, RexxString::match, 4);
        AddMethod(CHAR_CASELESSMATCH, RexxString::caselessMatch, 4);
        AddMethod(CHAR_MATCHCHAR, RexxString::matchChar, 2);
        AddMethod(CHAR_CASELESSMATCHCHAR, RexxString::caselessMatchChar, 2);
        AddMethod(CHAR_EQUALS, RexxString::equals, 1);
        AddMethod(CHAR_CASELESSEQUALS, RexxString::caselessEquals, 1);
        AddMethod(CHAR_COMPARETO, RexxString::compareToRexx, 3);
        AddMethod(CHAR_CASELESSCOMPARETO, RexxString::caselessCompareToRexx, 3);

    CompleteMethodDefinitions();

    CompleteClassDefinition(String);

EndClassDefinition(String));


    /***************************************************************************/
    /* ARRAY class                                                             */
    /***************************************************************************/

StartClassDefinition(Array);

        AddClassMethod(CHAR_NEW, ArrayClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_OF, ArrayClass::of, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_BRACKETS, ArrayClass::getRexx, A_COUNT);
        AddMethod(CHAR_BRACKETSEQUAL, ArrayClass::putRexx, A_COUNT);
        AddMethod(CHAR_AT, ArrayClass::getRexx, A_COUNT);
        AddMethod(CHAR_DIMENSION, ArrayClass::dimensionRexx, 1);
        AddMethod("DIMENSIONS", ArrayClass::getDimensionsRexx, 0);
        AddMethod(CHAR_HASINDEX, ArrayClass::hasIndexRexx, A_COUNT);
        AddMethod(CHAR_ITEMS, ArrayClass::itemsRexx, 0);
        AddMethod(CHAR_MAKEARRAY, RexxObject::makeArrayRexx, 0);
        AddMethod(CHAR_PUT, ArrayClass::putRexx, A_COUNT);
        AddMethod(CHAR_REMOVE, ArrayClass::removeRexx, A_COUNT);
        AddMethod(CHAR_SECTION, ArrayClass::sectionRexx, 2);
        AddMethod(CHAR_SIZE, ArrayClass::sizeRexx, 0);
        AddMethod(CHAR_SUPPLIER, ArrayClass::supplier, 0);
        AddMethod(CHAR_FIRST, ArrayClass::firstRexx, 0);
        AddMethod(CHAR_FIRSTITEM, ArrayClass::firstItem, 0);
        AddMethod(CHAR_LAST, ArrayClass::lastRexx, 0);
        AddMethod(CHAR_LASTITEM, ArrayClass::lastItem, 0);
        AddMethod(CHAR_NEXT, ArrayClass::nextRexx, A_COUNT);
        AddMethod(CHAR_PREVIOUS, ArrayClass::previousRexx, A_COUNT);
        AddMethod(CHAR_APPEND, ArrayClass::appendRexx, 1);
        AddMethod(CHAR_MAKESTRING, ArrayClass::makeString, 2);
        AddMethod(CHAR_TOSTRING, ArrayClass::toString, 2);
        AddMethod(CHAR_ALLINDEXES, ArrayClass::allIndexes, 0);
        AddMethod(CHAR_ALLITEMS, ArrayClass::allItems, 0);
        AddMethod(CHAR_EMPTY, ArrayClass::empty, 0);
        AddMethod(CHAR_ISEMPTY, ArrayClass::isEmptyRexx, 0);
        AddMethod(CHAR_INDEX, ArrayClass::indexRexx, 1);
        AddMethod(CHAR_HASITEM, ArrayClass::hasItemRexx, 1);
        AddMethod(CHAR_REMOVEITEM, ArrayClass::removeItem, 1);
        AddMethod(CHAR_INSERT, ArrayClass::insertRexx, 2);
        AddMethod(CHAR_DELETE, ArrayClass::deleteRexx, 1);
        AddMethod("FILL", ArrayClass::fill, 1);

    // there have been some problems with the quick sort used as the default sort, so map everything
    // to the stable sort.  The stable sort, in theory, uses more memory, but in practice, this is not true.
        AddMethod(CHAR_SORT, ArrayClass::stableSortRexx, 0);
        AddMethod(CHAR_SORTWITH, ArrayClass::stableSortWithRexx, 1);
        AddMethod(CHAR_STABLESORT, ArrayClass::stableSortRexx, 0);
        AddMethod(CHAR_STABLESORTWITH ,TheArrayBehaviour, ArrayClass::stableSortWithRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Array);

EndClassDefinition(Array);


    /***************************************************************************/
    /*           QUEUE                                                         */
    /***************************************************************************/

StartClassDefinition(Queue);

        AddClassMethod(CHAR_NEW, QueueClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_OF, QueueClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // QUEUE is implemented as a subclass of Array, so most of its instance
    // methods are the same as the Array ones.  We can inherit those in one shot,
    // then make whatever required additions or replacements.

        InheritInstanceMethods(Array);

        AddMethod(CHAR_PUSH, QueueClass::pushRexx, 1);
        AddMethod(CHAR_PEEK, QueueClass::peek, 0);
        AddMethod(CHAR_PULL, QueueClass::pullRexx, 0);
        AddMethod(CHAR_QUEUE, QueueClass::queueRexx, 1);
        AddMethod(CHAR_PUT, QueueClass::putRexx, 2);
        AddMethod(CHAR_REMOVE, QueueClass::removeRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Queue);

EndClassDefinition(Queue);

    /***************************************************************************/
    /*           IDENTITYTABLE                                                 */
    /***************************************************************************/

// Do all of the hash-based collections as a group, since we can directly inherit a
// lot of the methods from previous collections.

StartClassDefinition(IdentityTable);

        AddClassMethod(CHAR_NEW, IdentityTable::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_BRACKETS, HashCollection::getRexx, 1);
        AddMethod(CHAR_BRACKETSEQUAL, HashCollection::putRexx, 2);
        AddMethod(CHAR_MAKEARRAY, RexxObject::makeArrayRexx, 0);
        AddMethod(CHAR_AT, HashCollection::getRexx, 1);
        AddMethod(CHAR_HASINDEX, HashCollection::hasIndexRexx, 1);
        AddMethod(CHAR_ITEMS, HashCollection::itemsRexx, 0);
        AddMethod(CHAR_PUT, HashCollection::putRexx, 2);
        AddMethod(CHAR_REMOVE, HashCollection::removeRexx, 1);
        AddMethod(CHAR_SUPPLIER, HashCollection::supplier, 0);
        AddMethod(CHAR_ALLITEMS, HashCollection::allItems, 0);
        AddMethod(CHAR_ALLINDEXES, HashCollection::allIndexes, 0);
        AddMethod(CHAR_EMPTY, HashCollection::emptyRexx, 0);
        AddMethod(CHAR_ISEMPTY, HashCollection::isEmptyRexx, 0);
        AddMethod(CHAR_INDEX, HashCollection::indexRexx, 1);
        AddMethod(CHAR_HASITEM, HashCollection::hasItemRexx, 1);
        AddMethod(CHAR_REMOVEITEM, HashCollection::removeItemRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(IdentityTable);

EndClassDefinition(IdentityTable);


    /***************************************************************************/
    /*           TABLE                                                         */
    /***************************************************************************/

StartClassDefinition(Table);

        AddClassMethod(CHAR_NEW, TableClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        // most of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Table);

EndClassDefinition(Table);


    /***************************************************************************/
    /*           STRINGTABLE                                                   */
    /***************************************************************************/

StartClassDefinition(StringTable);

        AddClassMethod(CHAR_NEW, StringTable::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        // most of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

    CompleteMethodDefinitions();

    CompleteClassDefinition(StringTable);

EndClassDefinition(StringTable);


    /***************************************************************************/
    /*           SET                                                           */
    /***************************************************************************/

StartClassDefinition(Set)

        AddClassMethod(CHAR_NEW, SetClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_NEW, SetClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        // most of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Set);

EndClassDefinition(Set);


    /***************************************************************************/
    /*           DIRECTORY                                                     */
    /***************************************************************************/

StartClassDefinition(Directory)

        AddClassMethod(CHAR_NEW, DirectoryClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // many of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

        AddMethod(CHAR_UNKNOWN, DirectoryClass::unknown, 2);
        AddMethod(CHAR_INIT, DirectoryClass::initRexx, 1);

        AddMethod(CHAR_ENTRY, DirectoryClass::entryRexx, 1);
        AddMethod(CHAR_HASENTRY, DirectoryClass::hasEntry, 1);
        AddMethod(CHAR_SETENTRY, DirectoryClass::setEntry, 2);

        AddProtectedMethod(CHAR_SETMETHOD, DirectoryClass::setMethodRexx, 2);
        AddProtectedMethod(CHAR_UNSETMETHOD, DirectoryClass::removeRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Directory);

EndClassDefinition(Directory);


    /***************************************************************************/
    /*           RELATION                                                      */
    /***************************************************************************/

StartClassDefinition(Relation)

        AddClassMethod(CHAR_NEW, Relation::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // many of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

        AddMethod(CHAR_REMOVEITEM, RelationClass::removeItemRexx, 2);
        AddMethod(CHAR_SUPPLIER, RelationClass::supplierRexx, 1);
        AddMethod(CHAR_ITEMS, RelationClass::itemsRexx, 1);
        AddMethod(CHAR_HASITEM, RelationClass::hasItemRexx, 2);
        AddMethod(CHAR_ALLINDEX, RelationClass::allIndexRexx, 1);
        AddMethod("REMOVEALL", RelationClass::removeAll, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Relation);

EndClassDefinition(Relation);


    /***************************************************************************/
    /*           Bag                                                           */
    /***************************************************************************/

StartClassDefinition(Bag)

        AddClassMethod(CHAR_NEW, BagClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_OF, BagClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // many of the hash collection methods can be inherited
        InheritInstanceMethods(Relation);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Bag);

EndClassDefinition(Bag);


    /***************************************************************************/
    /*           LIST                                                          */
    /***************************************************************************/

StartClassDefinition(List)

        AddClassMethod(CHAR_NEW, ListClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_OF, ListClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_INIT, ListClass::initRexx, 1);
        AddMethod(CHAR_BRACKETS, ListClass::getRexx, 1);
        AddMethod(CHAR_BRACKETSEQUAL, ListClass::putRexx, 2);
        AddMethod(CHAR_MAKEARRAY, RexxObject::makeArrayRexx, 0);
        AddMethod(CHAR_AT, ListClass::getRexx, 1);
        AddMethod(CHAR_FIRSTITEM, ListClass::firstItemRexx, 0);
        AddMethod(CHAR_HASINDEX, ListClass::hasIndexRexx, 1);
        AddMethod(CHAR_INSERT, ListClass::insertRexx, 2);
        AddMethod(CHAR_ITEMS, ListClass::itemsRexx, 0);
        AddMethod(CHAR_LASTITEM, ListClass::lastItemRexx, 0);
        AddMethod(CHAR_FIRST, ListClass::firstRexx, 0);
        AddMethod(CHAR_LAST, ListClass::lastRexx, 0);
        AddMethod(CHAR_NEXT, ListClass::nextRexx, 1);
        AddMethod(CHAR_PREVIOUS, ListClass::previousRexx, 1);
        AddMethod(CHAR_PUT, ListClass::putRexx, 2);
        AddMethod(CHAR_REMOVE, ListClass::removeRexx, 1);
    // DELETE is the same as REMOVE for the List class
        AddMethod(CHAR_DELETE, ListClass::removeRexx, 1);
        AddMethod(CHAR_SECTION, ListClass::sectionRexx, 2);
        AddMethod(CHAR_SUPPLIER, ListClass::supplier, 0);
        AddMethod(CHAR_APPEND, ListClass::appendRexx, 1);
        AddMethod(CHAR_ALLITEMS, ListClass::allItems, 0);
        AddMethod(CHAR_ALLINDEXES, ListClass::allIndexes, 0);
        AddMethod(CHAR_EMPTY, ListClass::emptyRexx, 0);
        AddMethod(CHAR_ISEMPTY, ListClass::isEmptyRexx, 0);
        AddMethod(CHAR_INDEX, ListClass::indexRexx, 1);
        AddMethod(CHAR_HASITEM, ListClass::hasItemRexx, 1);
        AddMethod(CHAR_REMOVEITEM, ListClass::removeItemRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(List);

EndClassDefinition(List);

    /***************************************************************************/
    /*           MESSAGE                                                       */
    /***************************************************************************/

StartClassDefinition(Message)

        AddClassMethod(CHAR_NEW, MessageClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_COMPLETED, MessageClass::completed, 0);
        AddMethod(CHAR_HASERROR, MessageClass::hasError, 0);
        AddMethod(CHAR_NOTIFY, MessageClass::notify, 1);
        AddMethod(CHAR_RESULT, MessageClass::result, 0);
        AddMethod(CHAR_TARGET, MessageClass::messageTarget, 0);
        AddMethod(CHAR_MESSAGENAME, MessageClass::messageName, 0);
        AddMethod(CHAR_ARGUMENTS, MessageClass::arguments, 0);
        AddMethod(CHAR_ERRORCONDITION, MessageClass::errorCondition, 0);
        AddMethod(CHAR_SEND, MessageClass::send, 1);
        AddMethod(CHAR_START, MessageClass::start, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Message);

EndClassDefinition(Message);


    /***************************************************************************/
    /*           METHOD                                                        */
    /***************************************************************************/

StartClassDefinition(Method)

        AddClassMethod(CHAR_NEW, MethodClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_NEWFILE, MethodClass::newFileRexx, 1);
        AddClassMethod("LOADEXTERNALMETHOD", MethodClass::loadExternalMethod, 2);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_SETUNGUARDED, MethodClass::setUnguardedRexx, 0);
        AddMethod(CHAR_SETGUARDED, MethodClass::setGuardedRexx, 0);
        AddMethod(CHAR_SETPRIVATE, MethodClass::setPrivateRexx, 0);
        AddMethod(CHAR_ISGUARDED, MethodClass::isGuardedRexx, 0);
        AddMethod(CHAR_ISPRIVATE, MethodClass::isPrivateRexx, 0);
        AddMethod(CHAR_ISPROTECTED, MethodClass::isProtectedRexx, 0);
        AddProtectedMethod(CHAR_SETPROTECTED, MethodClass::setProtectedRexx, 0);
        AddProtectedMethod(CHAR_SETSECURITYMANAGER, MethodClass::setSecurityManager, 1);
        AddMethod(CHAR_SOURCE, BaseExecutable::source, 0);
        AddMethod(CHAR_PACKAGE, BaseExecutable::getPackage, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Method);

EndClassDefinition(Method);


    /***************************************************************************/
    /*           ROUTINE                                                       */
    /***************************************************************************/

StartClassDefinition(Routine)

        AddClassMethod(CHAR_NEW, RoutineClass::newRexx, A_COUNT);
        AddClassMethod(CHAR_NEWFILE, RoutineClass::newFileRexx, 1);
        AddClassMethod("LOADEXTERNALROUTINE", RoutineClass::loadExternalRoutine, 2);

    CompleteClassMethodDefinitions();

        AddProtectedMethod(CHAR_SETSECURITYMANAGER, RoutineClass::setSecurityManager, 1);
        AddMethod(CHAR_SOURCE, BaseExecutable::source, 0);
        AddMethod(CHAR_PACKAGE, BaseExecutable::getPackage, 0);
        AddMethod(CHAR_CALL, RoutineClass::callRexx, A_COUNT);
        AddMethod(CHAR_CALLWITH, RoutineClass::callWithRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Routine);

EndClassDefinition(Routine);


    /***************************************************************************/
    /*           Package                                                       */
    /***************************************************************************/

StartClassDefinition(Package)

        AddClassMethod(CHAR_NEW, PackageClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddProtectedMethod(CHAR_SETSECURITYMANAGER, PackageClass::setSecurityManager, 1);
        AddMethod(CHAR_SOURCE, PackageClass::getSource, 0);
        AddMethod(CHAR_SOURCELINE, PackageClass::getSourceLineRexx, 1);
        AddMethod(CHAR_SOURCESIZE, PackageClass::getSourceSize, 0);
        AddMethod(CHAR_CLASSES, PackageClass::getClasses, 0);
        AddMethod(CHAR_PUBLICCLASSES, PackageClass::getPublicClasses, 0);
        AddMethod(CHAR_IMPORTEDCLASSES, PackageClass::getImportedClasses, 0);
        AddMethod(CHAR_DEFINEDMETHODS, PackageClass::getMethods, 0);
        AddMethod(CHAR_ROUTINES, PackageClass::getRoutines, 0);
        AddMethod(CHAR_PUBLICROUTINES, PackageClass::getPublicRoutines, 0);
        AddMethod(CHAR_IMPORTEDROUTINES, PackageClass::getImportedRoutines, 0);
        AddMethod(CHAR_IMPORTEDPACKAGES, PackageClass::getImportedPackages, 0);
        AddMethod(CHAR_LOADPACKAGE, PackageClass::loadPackage, 2);
        AddMethod(CHAR_ADDPACKAGE, PackageClass::addPackage, 1);
        AddMethod(CHAR_FINDCLASS, PackageClass::findClassRexx, 1);
        AddMethod(CHAR_FINDROUTINE, PackageClass::findRoutineRexx, 1);
        AddMethod(CHAR_ADDROUTINE, PackageClass::addRoutine, 2);
        AddMethod(CHAR_ADDPUBLICROUTINE, PackageClass::addPublicRoutine, 2);
        AddMethod(CHAR_ADDCLASS, PackageClass::addClass, 2);
        AddMethod(CHAR_ADDPUBLICCLASS, PackageClass::addPublicClass, 2);
        AddMethod(CHAR_NAME, PackageClass::getName, 0);
        AddMethod("LOADLIBRARY", PackageClass::loadLibrary, 1);
        AddMethod("DIGITS", PackageClass::digits, 0);
        AddMethod("FORM", PackageClass::form, 0);
        AddMethod("FUZZ", PackageClass::fuzz, 0);
        AddMethod("TRACE", PackageClass::trace, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Package);

EndClassDefinition(Package);


    /***************************************************************************/
    /*           RexxContext                                                   */
    /***************************************************************************/

StartClassDefinition(RexxContext)

        AddClassMethod(CHAR_NEW, RexxContext::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_COPY, RexxContext::copyRexx, 0);
        AddMethod(CHAR_PACKAGE, RexxContext::getPackage, 0);
        AddMethod(CHAR_EXECUTABLE, RexxContext::getExecutable, 0);
        AddMethod(CHAR_FORM, RexxContext::getForm, 0);
        AddMethod(CHAR_FUZZ, RexxContext::getFuzz, 0);
        AddMethod(CHAR_DIGITS, RexxContext::getDigits, 0);
        AddMethod(CHAR_VARIABLES, RexxContext::getVariables, 0);
        AddMethod(CHAR_ARGS, RexxContext::getArgs, 0);
        AddMethod(CHAR_CONDITION, RexxContext::getCondition, 0);
        AddMethod("LINE", RexxContext::getLine, 0);
        AddMethod("RS", RexxContext::getRS, 0);
        AddMethod(CHAR_NAME, RexxContext::getName, 0);
        AddMethod("STACKFRAMES", RexxContext::getStackFrames, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(RexxContext);

EndClassDefinition(RexxContext);


    /***************************************************************************/
    /*           STEM                                                          */
    /***************************************************************************/

StartClassDefinition(Stem)

        AddClassMethod(CHAR_NEW, StemClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_BRACKETS, StemClass::bracket, A_COUNT);
        AddMethod(CHAR_BRACKETSEQUAL, StemClass::bracketEqual, A_COUNT);
        AddMethod(CHAR_AT, StemClass::bracket, A_COUNT);
        AddMethod(CHAR_PUT, StemClass::bracketEqual, A_COUNT);
        AddMethod(CHAR_MAKEARRAY, RexxObject::makeArrayRexx, 0);
        AddMethod(CHAR_REQUEST, StemClass::request, 1);
        AddMethod(CHAR_SUPPLIER, StemClass::supplier, 0);
        AddMethod(CHAR_ALLINDEXES, StemClass::allIndexes, 0);
        AddMethod(CHAR_ALLITEMS, StemClass::allItems, 0);
        AddMethod(CHAR_EMPTY, StemClass::empty, 0);
        AddMethod(CHAR_ISEMPTY, StemClass::isEmptyRexx, 0);
        AddMethod(CHAR_UNKNOWN, RexxObject::unknownRexx, 2);

        AddMethod(CHAR_ITEMS, StemClass::itemsRexx, 0);
        AddMethod(CHAR_HASINDEX, StemClass::hasIndex, A_COUNT);
        AddMethod(CHAR_REMOVE, StemClass::remove, A_COUNT);
        AddMethod(CHAR_INDEX, StemClass::index, 1);
        AddMethod(CHAR_HASITEM, StemClass::hasItem, 1);
        AddMethod(CHAR_REMOVEITEM, StemClass::removeItem, 1);
        AddMethod(CHAR_TODIRECTORY, StemClass::toDirectory, 0);

    // We want various operator methods that we inherit from the object
    // class to be redirected to our unknown method, so we block these methods
    // in our instance method directory.
    HideMethod(CHAR_STRICT_EQUAL);
    HideMethod(CHAR_EQUAL);
    HideMethod(CHAR_STRICT_BACKSLASH_EQUAL);
    HideMethod(CHAR_BACKSLASH_EQUAL);
    HideMethod(CHAR_LESSTHAN_GREATERTHAN);
    HideMethod(CHAR_GREATERTHAN_LESSTHAN);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Stem);

EndClassDefinition(Stem);

    /***************************************************************************/
    /*           MUTABLEBUFFER                                                 */
    /***************************************************************************/

StartClassDefinition(MutableBuffer)

        AddClassMethod(CHAR_NEW, MutableBuffer::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_APPEND, MutableBuffer::append, 1);
        AddMethod(CHAR_INSERT, MutableBuffer::insert, 4);
        AddMethod(CHAR_OVERLAY, MutableBuffer::overlay, 4);
        AddMethod(CHAR_REPLACEAT, MutableBuffer::replaceAt, 4);
        AddMethod(CHAR_DELETE, MutableBuffer::mydelete, 2);
        AddMethod(CHAR_DELSTR, MutableBuffer::mydelete, 2);
        AddMethod(CHAR_SUBSTR, MutableBuffer::substr, 3);
        AddMethod(CHAR_POS, MutableBuffer::posRexx, 3);
        AddMethod(CHAR_LASTPOS, MutableBuffer::lastPos, 3);
        AddMethod("CONTAINS", MutableBuffer::containsRexx, 3);
        AddMethod("CASELESSCONTAINS", MutableBuffer::caselessContains, 3);
        AddMethod("CONTAINSWORD", MutableBuffer::containsWord, 2);
        AddMethod("CASELESSCONTAINSWORD", MutableBuffer::caselessContainsWord, 2);
        AddMethod(CHAR_CASELESSPOS, MutableBuffer::caselessPos, 3);
        AddMethod(CHAR_CASELESSLASTPOS, MutableBuffer::caselessLastPos, 3);
        AddMethod(CHAR_SUBCHAR, MutableBuffer::subchar, 1);
        AddMethod(CHAR_GETBUFFERSIZE, MutableBuffer::getBufferSize, 0);
        AddMethod(CHAR_SETBUFFERSIZE, MutableBuffer::setBufferSize, 1);

        AddMethod(CHAR_LENGTH, MutableBuffer::lengthRexx, 0);
        AddMethod(CHAR_MAKEARRAY, MutableBuffer::makeArrayRexx, 1);
        AddMethod(CHAR_STRING, RexxObject::makeStringRexx, 0);
        AddMethod(CHAR_COUNTSTR, MutableBuffer::countStrRexx, 1);
        AddMethod(CHAR_CASELESSCOUNTSTR, MutableBuffer::caselessCountStrRexx, 1);
        AddMethod(CHAR_CHANGESTR, MutableBuffer::changeStr, 3);
        AddMethod(CHAR_CASELESSCHANGESTR, MutableBuffer::caselessChangeStr, 3);
        AddMethod(CHAR_UPPER, MutableBuffer::upper, 2);
        AddMethod(CHAR_LOWER, MutableBuffer::lower, 2);
        AddMethod(CHAR_TRANSLATE, MutableBuffer::translate, 5);
        AddMethod(CHAR_MATCH, MutableBuffer::match, 4);
        AddMethod(CHAR_CASELESSMATCH, MutableBuffer::caselessMatch, 4);
        AddMethod(CHAR_MATCHCHAR, MutableBuffer::matchChar, 2);
        AddMethod(CHAR_CASELESSMATCHCHAR, MutableBuffer::caselessMatchChar, 2);
        AddMethod(CHAR_VERIFY, MutableBuffer::verify, 4);
        AddMethod(CHAR_SPACE, MutableBuffer::space, 2);
        AddMethod(CHAR_SUBWORD, MutableBuffer::subWord, 2);
        AddMethod("SUBWORDS", MutableBuffer::subWords, 2);
        AddMethod(CHAR_WORD, MutableBuffer::word, 1);
        AddMethod(CHAR_WORDINDEX, MutableBuffer::wordIndex, 1);
        AddMethod(CHAR_WORDLENGTH, MutableBuffer::wordLength, 1);
        AddMethod(CHAR_WORDS, MutableBuffer::words, 0);
        AddMethod(CHAR_WORDPOS, MutableBuffer::wordPos, 2);
        AddMethod(CHAR_CASELESSWORDPOS, MutableBuffer::caselessWordPos, 2);
        AddMethod(CHAR_DELWORD, MutableBuffer::delWord, 2);

    CompleteMethodDefinitions();

    CompleteClassDefinition(MutableBuffer);

EndClassDefinition(MutableBuffer);

    /***************************************************************************/
    /*             INTEGER                                                     */
    /***************************************************************************/

StartClassDefinition(Integer)

    // NOTE that we are pointing the new method at the String version...part of the
    // fakeout that the Integer class does.
        AddClassMethod(CHAR_NEW, RexxString::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_PLUS, RexxInteger::plus, 1);
        AddMethod(CHAR_SUBTRACT, RexxInteger::minus, 1);
        AddMethod(CHAR_MULTIPLY, RexxInteger::multiply, 1);
        AddMethod(CHAR_POWER, RexxInteger::power, 1);
        AddMethod(CHAR_DIVIDE, RexxInteger::divide, 1);
        AddMethod(CHAR_INTDIV, RexxInteger::integerDivide, 1);
        AddMethod(CHAR_REMAINDER, RexxInteger::remainder, 1);
        AddMethod(CHAR_BACKSLASH, RexxInteger::notOp, 0);
        AddMethod(CHAR_AND, RexxInteger::andOp, 1);
        AddMethod(CHAR_OR, RexxInteger::orOp, 1);
        AddMethod(CHAR_XOR, RexxInteger::xorOp, 1);
        AddMethod(CHAR_UNKNOWN, RexxObject::unknownRexx, 2);
        AddMethod(CHAR_D2C, RexxInteger::d2c, 1);
        AddMethod(CHAR_D2X, RexxInteger::d2x, 1);
        AddMethod(CHAR_ABS, RexxInteger::abs, 0);
        AddMethod(CHAR_ORXMAX, RexxInteger::Max, A_COUNT);
        AddMethod(CHAR_ORXMIN, RexxInteger::Min, A_COUNT);
        AddMethod(CHAR_SIGN, RexxInteger::sign, 0);
        AddMethod(CHAR_EQUAL, RexxInteger::equal, 1);
        AddMethod(CHAR_BACKSLASH_EQUAL, RexxInteger::notEqual, 1);
        AddMethod(CHAR_LESSTHAN_GREATERTHAN, RexxInteger::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN_LESSTHAN, RexxInteger::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN, RexxInteger::isGreaterThan, 1);
        AddMethod(CHAR_LESSTHAN, RexxInteger::isLessThan, 1);
        AddMethod(CHAR_GREATERTHAN_EQUAL, RexxInteger::isGreaterOrEqual, 1);
        AddMethod(CHAR_BACKSLASH_LESSTHAN, RexxInteger::isGreaterOrEqual, 1);
        AddMethod(CHAR_LESSTHAN_EQUAL, RexxInteger::isLessOrEqual, 1);
        AddMethod(CHAR_BACKSLASH_GREATERTHAN, RexxInteger::isLessOrEqual, 1);
        AddMethod(CHAR_STRICT_EQUAL, RexxInteger::strictEqual, 1);
        AddMethod(CHAR_HASHCODE, RexxInteger::hashCode, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_EQUAL, RexxInteger::strictNotEqual, 1);
        AddMethod(CHAR_STRICT_GREATERTHAN, RexxInteger::strictGreaterThan, 1);
        AddMethod(CHAR_STRICT_LESSTHAN, RexxInteger::strictLessThan, 1);
        AddMethod(CHAR_STRICT_GREATERTHAN_EQUAL, RexxInteger::strictGreaterOrEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_LESSTHAN, RexxInteger::strictGreaterOrEqual, 1);
        AddMethod(CHAR_STRICT_LESSTHAN_EQUAL, RexxInteger::strictLessOrEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN, RexxInteger::strictLessOrEqual, 1);
        AddMethod(CHAR_MAKESTRING, RexxObject::makeStringRexx, 0);
        AddMethod(CHAR_FORMAT, RexxInteger::format, 4);
        AddMethod(CHAR_TRUNC, RexxInteger::trunc, 1);
        AddMethod("FLOOR", RexxInteger::floor, 0);
        AddMethod("CEILING", RexxInteger::ceiling, 0);
        AddMethod("ROUND", RexxInteger::round, 0);
        AddMethod(CHAR_CLASS, RexxInteger::classObject, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Integer);

EndClassDefinition(Integer);


    /***************************************************************************/
    /*             NUMBERSTRING                                                */
    /***************************************************************************/

StartClassDefinition(NumberString)

    // NOTE that we are pointing the new method at the String version...part of the
    // fakeout that the NumberString class does.
        AddClassMethod(CHAR_NEW, RexxString::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_UNKNOWN, RexxObject::unknownRexx, 2);
        AddMethod(CHAR_ABS, NumberString::abs, 0);
        AddMethod(CHAR_ORXMAX, NumberString::Max, A_COUNT);
        AddMethod(CHAR_ORXMIN, NumberString::Min, A_COUNT);
        AddMethod(CHAR_SIGN, NumberString::Sign, 0);
        AddMethod(CHAR_D2C, NumberString::d2c, 1);
        AddMethod(CHAR_D2X, NumberString::d2x, 1);
        AddMethod(CHAR_EQUAL, NumberString::equal, 1);
        AddMethod(CHAR_BACKSLASH_EQUAL, NumberString::notEqual, 1);
        AddMethod(CHAR_LESSTHAN_GREATERTHAN, NumberString::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN_LESSTHAN, NumberString::notEqual, 1);
        AddMethod(CHAR_GREATERTHAN, NumberString::isGreaterThan, 1);
        AddMethod(CHAR_LESSTHAN, NumberString::isLessThan, 1);
        AddMethod(CHAR_GREATERTHAN_EQUAL, NumberString::isGreaterOrEqual, 1);
        AddMethod(CHAR_BACKSLASH_LESSTHAN, NumberString::isGreaterOrEqual, 1);
        AddMethod(CHAR_LESSTHAN_EQUAL, NumberString::isLessOrEqual, 1);
        AddMethod(CHAR_BACKSLASH_GREATERTHAN, NumberString::isLessOrEqual, 1);
        AddMethod(CHAR_STRICT_EQUAL, NumberString::strictEqual, 1);
        AddMethod(CHAR_HASHCODE, NumberString::hashCode, 0);
        AddMethod(CHAR_STRICT_BACKSLASH_EQUAL, NumberString::strictNotEqual, 1);
        AddMethod(CHAR_STRICT_GREATERTHAN, NumberString::strictGreaterThan, 1);
        AddMethod(CHAR_STRICT_LESSTHAN, NumberString::strictLessThan, 1);
        AddMethod(CHAR_STRICT_GREATERTHAN_EQUAL, NumberString::strictGreaterOrEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_LESSTHAN, NumberString::strictGreaterOrEqual, 1);
        AddMethod(CHAR_STRICT_LESSTHAN_EQUAL, NumberString::strictLessOrEqual, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_GREATERTHAN, NumberString::strictLessOrEqual, 1);
        AddMethod(CHAR_PLUS, NumberString::plus, 1);
        AddMethod(CHAR_SUBTRACT, NumberString::minus, 1);
        AddMethod(CHAR_MULTIPLY, NumberString::multiply, 1);
        AddMethod(CHAR_POWER, NumberString::power, 1);
        AddMethod(CHAR_DIVIDE, NumberString::divide, 1);
        AddMethod(CHAR_INTDIV, NumberString::integerDivide, 1);
        AddMethod(CHAR_REMAINDER, NumberString::remainder, 1);
        AddMethod(CHAR_BACKSLASH, NumberString::notOp, 0);
        AddMethod(CHAR_AND, NumberString::andOp, 1);
        AddMethod(CHAR_OR, NumberString::orOp, 1);
        AddMethod(CHAR_XOR, NumberString::xorOp, 1);
        AddMethod(CHAR_MAKESTRING, RexxObject::makeStringRexx, 0);
        AddMethod(CHAR_FORMAT, NumberString::formatRexx, 4);
        AddMethod(CHAR_TRUNC, NumberString::trunc, 1);
        AddMethod("FLOOR", NumberString::floor, 0);
        AddMethod("CEILING", NumberString::ceiling, 0);
        AddMethod("ROUND", NumberString::round, 0);
        AddMethod(CHAR_CLASS, NumberString::classObject, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(NumberString);

EndClassDefinition(NumberString);


    /***************************************************************************/
    /*           SUPPLIER                                                      */
    /***************************************************************************/

StartClassDefinition(Supplier)

        AddClassMethod(CHAR_NEW, SupplierClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_AVAILABLE, SupplierClass::available, 0);
        AddMethod(CHAR_INDEX, SupplierClass::index, 0);
        AddMethod(CHAR_NEXT, SupplierClass::next, 0);
        AddMethod(CHAR_ITEM, SupplierClass::value, 0);
        AddMethod(CHAR_INIT, SupplierClass::initRexx, 2);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Supplier);

EndClassDefinition(Supplier);


    /***************************************************************************/
    /*           POINTER                                                       */
    /***************************************************************************/

StartClassDefinition(Pointer)

        AddClassMethod(CHAR_NEW, PointerClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_EQUAL, PointerClass::equal, 1);
        AddMethod(CHAR_BACKSLASH_EQUAL, PointerClass::notEqual, 1);
        AddMethod(CHAR_STRICT_EQUAL, PointerClass::equal, 1);
        AddMethod(CHAR_STRICT_BACKSLASH_EQUAL, PointerClass::notEqual, 1);
        AddMethod(CHAR_ISNULL, PointerClass::isNull, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Pointer);

EndClassDefinition(Pointer);


    /***************************************************************************/
    /*           BUFFER                                                        */
    /***************************************************************************/

StartClassDefinition(Buffer)

        AddClassMethod(CHAR_NEW, BufferClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // NO instance methods on buffer

    CompleteMethodDefinitions();

    CompleteClassDefinition(Buffer);

EndClassDefinition(Buffer);


    /***************************************************************************/
    /*           WEAKREFERENCE                                                 */
    /***************************************************************************/

StartClassDefinition(WeakReference)

        AddClassMethod(CHAR_NEW, WeakReference::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod(CHAR_VALUE, WeakReference::value, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(WeakReference);

EndClassDefinition(WeakReference);


    /***************************************************************************/
    /*           STACKFRAME                                                    */
    /***************************************************************************/

StartClassDefinition(StackFrame)

        AddClassMethod(CHAR_NEW, StackFrameClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("NAME", StackFrameClass::getName, 0);
        AddMethod("EXECUTABLE", StackFrameClass::getExecutable, 0);
        AddMethod("LINE", StackFrameClass::getLine, 0);
        AddMethod("TARGET", StackFrameClass::getTarget, 0);
        AddMethod("TRACELINE", StackFrameClass::getTraceLine, 0);
        AddMethod("TYPE", StackFrameClass::getType, 0);
        AddMethod("ARGUMENTS", StackFrameClass::getArguments, 0);
        // the string method just maps to TRACELINE
        AddMethod("STRING", StackFrameClass::getTraceLine, 0);
        AddMethod("MAKESTRING", StackFrameClass::getTraceLine, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(StackFrame);

EndClassDefinition(StackFrame);

    /***************************************************************************/
    /***************************************************************************/
    /***************************************************************************/

    // now add entries to the environment
    addToEnvironment(CHAR_ENVIRONMENT, TheEnvironment);
    addToEnvironment(CHAR_NIL ,TheNilObject);
    addToEnvironment(CHAR_FALSE, TheFalseObject, TheEnvironment);
    addToEnvironment(CHAR_TRUE, TheTrueObject, TheEnvironment);

    // TODO:  Make sure INTEGER and NUMBERSTRING are removed from the environment.

    // set up the kernel directory
    addToSystem(CHAR_INTEGER, TheIntegerClass);
    addToSystem(CHAR_NUMBERSTRING, TheNumberStringClass);

    addToSystem(CHAR_NULLARRAY, TheNullArray);
    addToSystem(CHAR_NULLPOINTER, TheNullPointer);
    addToSystem(CHAR_COMMON_RETRIEVERS, TheCommonRetrievers);
    addToSystem(CHAR_ENVIRONMENT, TheEnvironment);
    addToSystem(CHAR_FUNCTIONS, TheFunctionsDirectory);

    addToSystem(CHAR_VERSION, Interpreter::getVersionNumber());
    addToSystem(CHAR_NAME, SystemInterpreter::getSystemName());
    addToSystem(CHAR_INTERNALNAME, SystemInterpreter::getInternalSystemName());
    addToSystem(CHAR_VERSION, SystemInterpreter::getSystemVersion());
    // initialize our thread vector for external calls.
    RexxActivity::initializeThreadContext();

/******************************************************************************/
/*      Complete the image build process, calling BaseClasses to establish    */
/*      the rest of the REXX image.                                           */
/******************************************************************************/

  // set up the kernel methods that will be defined on OBJECT classes in
  // CoreClasses.orx
  {
      // create a method used to retrieve the .Local environment.  We set this on the
      // .Environment directory.
      Protected<MethodClass> localMethod = new MethodClass(getGlobalName(CHAR_LOCAL), CPPCode::resolveExportedMethod(CHAR_LOCAL, CPPM(RexxLocal::local), 0));

      // add this to the environment directory.
      TheEnvironment->setMethodRexx(getGlobalName(CHAR_LOCAL), localMethod);

                                           /* create the BaseClasses method and run it*/
      RexxString *symb = getGlobalName(BASEIMAGELOAD);   /* get a name version of the string  */
                                           /* go resolve the program name       */
      RexxString *programName = ActivityManager::currentActivity->resolveProgramName(symb, OREF_NULL, OREF_NULL);
      // create a new stack frame to run under
      ActivityManager::currentActivity->createNewActivationStack();
      try
      {
          // create an executable object for this.
          Protected<RoutineClass> loader = LanguageParser::createProgram(programName);

          // we pass TheSystem as an argument to the core classes.
          RexxObject *args = TheSystem;
          ProtectedObject result;
          // now create the core program objects.
          loader->runProgram(ActivityManager::currentActivity, OREF_PROGRAM, OREF_NULL, (RexxObject **)&args, 1, result);
      }
      catch (ActivityException )
      {
          ActivityManager::currentActivity->error();          /* do error cleanup                  */
          Interpreter::logicError("Error building kernel image.  Image not saved.");
      }

  }

  /* define and suppress methods in the nil object */
  TheNilObject->defineMethod(getGlobalName(CHAR_COPY), (MethodClass *)TheNilObject);
  TheNilObject->defineMethod(getGlobalName(CHAR_START), (MethodClass *)TheNilObject);
  TheNilObject->defineMethod(getGlobalName(CHAR_OBJECTNAMEEQUALS), (MethodClass *)TheNilObject);

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

  TheStringTableClass->inherit(map, OREF_NULL);
  TheStringTableClass->setRexxDefined();

  TheIdentityTableClass->inherit(map, OREF_NULL);
  TheIdentityTableClass->setRexxDefined();

  TheRelationClass->inherit(map, OREF_NULL);
  TheRelationClass->setRexxDefined();

  TheDirectoryClass->inherit(map, OREF_NULL);
  TheDirectoryClass->setRexxDefined();

  TheStemClass->inherit(map, OREF_NULL);
  TheStemClass->setRexxDefined();

  // TODO:  Add Set and Bag class processing here.


  RexxClass *comparable = (RexxClass *)TheEnvironment->get(getGlobalName(CHAR_COMPARABLE));

  TheStringClass->inherit(comparable, OREF_NULL);
  TheStringClass->setRexxDefined();

  // disable the special class methods we only use during the image build phase.
  // this removes this from all of the subclasses as well
  TheObjectClass->removeClassMethod(new_string(CHAR_DEFINE_METHODS));
  TheObjectClass->removeClassMethod(new_string(CHAR_SHRIEKREXXDEFINED));
  TheObjectClass->removeClassMethod(new_string("!DEFINE_CLASS_METHOD"));
  TheObjectClass->removeClassMethod(new_string("INHERITINSTANCEMETHODS"));

  // now save the image
  memoryObject.saveImage();
  ActivityManager::returnActivity(ActivityManager::currentActivity);
  exit(RC_OK);                         // successful build
}

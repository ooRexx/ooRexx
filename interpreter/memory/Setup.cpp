/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
#include "NativeCode.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "NativeActivation.hpp"
#include "VariableDictionary.hpp"
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
#include "RexxInfoClass.hpp"
#include "LanguageParser.hpp"
#include "SetClass.hpp"
#include "BagClass.hpp"
#include "ActivityManager.hpp"
#include "ProgramSource.hpp"
#include "VariableReference.hpp"
#include "EventSemaphore.hpp"
#include "MutexSemaphore.hpp"


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
void MemoryObject::defineProtectedMethod(const char *name, RexxBehaviour *behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName)
{
        MethodClass *method = behaviour->defineMethod(name, entryPoint, arguments, entryPointName);
        // mark as protected after the fact
        method->setProtected();
}


/**
 * Add a C++ method to an object's behaviour.  This method is
 * marked as unguarded.
 *
 * @param name       The name of the method.
 * @param behaviour  The target behaviour.
 * @param entryPoint The entry point of the C++ method that implements the
 *                   method.
 * @param arguments  The method argument style (argument count or array indicator).
 */
void MemoryObject::defineUnguardedMethod(const char *name, RexxBehaviour *behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName)
{
        MethodClass *method = behaviour->defineMethod(name, entryPoint, arguments, entryPointName);
        // mark as protected after the fact
        method->setUnguarded();
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
void MemoryObject::definePrivateMethod(const char *name, RexxBehaviour * behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName)
{
    MethodClass *method = behaviour->defineMethod(name, entryPoint, arguments, entryPointName);
    // mark the method as private
    method->setPrivate();
}


/**
 * Add a object to the environment using the provided name.
 *
 * @param name   The name of the new environment entry.
 * @param value  The value added.
 */
void MemoryObject::addToEnvironment(const char *name, RexxInternalObject *value)
{
    TheEnvironment->put(value, getUpperGlobalName(name));
}


/**
 * Add a object to the kernel directory using the provided name.
 *
 * @param name   The name of the new environment entry.
 * @param value  The value added.
 */
void MemoryObject::addToSystem(const char *name, RexxInternalObject *value)
{
    TheSystem->put(value, getUpperGlobalName(name));
}


/**
 * Finalize a system class during image construction.  This
 * places the class in the Environment and also adds it to
 * the Rexx package.
 *
 * @param name     The class name.
 * @param classObj The class object.
 */
void MemoryObject::completeSystemClass(const char *name, RexxClass *classObj)
{
    // this gets added to the environment and the package in an uppercase name.
    RexxString *className = getUpperGlobalName(name);
    TheEnvironment->put(classObj, className);
    // this is added as a public class in this package.
    TheRexxPackage->addInstalledClass(className, classObj, true);
}


/**
 * Create the base Rexx package object.  All Rexx-defined classes
 * in the image will be added to this package.
 */
void MemoryObject::createRexxPackage()
{
    // this is a dummy package named "REXX" with the place holder
    // sourceless program source
    rexxPackage = new PackageClass(GlobalNames::REXX, new ProgramSource());
}


/**
 * Initialize the Rexx memory environment during an image built.
 *
 * @param imageTarget
 *               The location to save the created image file.
 */
void MemoryObject::createImage(const char *imageTarget)
{
    // perform the initial memory environment setup.  We can create
    // new objects once this is done.

    MemoryObject::create();

    // Class and integer has some special stuff, so get them created first
    RexxClass::createInstance();
    RexxInteger::createInstance();

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
    SetClass::createInstance();
    BagClass::createInstance();
    ListClass::createInstance();
    QueueClass::createInstance();

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
    RexxInfo::createInstance();
    VariableReference::createInstance();
    EventSemaphoreClass::createInstance();
    MutexSemaphoreClass::createInstance();

    // build the common retrievers table.  This is needed before we can parse an
    // Rexx code.
    TheCommonRetrievers = new_string_table();

    // These special variables are always assigned the same slot positions in all
    // Rexx code contexts.
    TheCommonRetrievers->put(new RexxSimpleVariable(GlobalNames::SELF, VARIABLE_SELF), GlobalNames::SELF);
    TheCommonRetrievers->put(new RexxSimpleVariable(GlobalNames::SUPER, VARIABLE_SUPER), GlobalNames::SUPER);
    TheCommonRetrievers->put(new RexxSimpleVariable(GlobalNames::SIGL, VARIABLE_SIGL), GlobalNames::SIGL);
    TheCommonRetrievers->put(new RexxSimpleVariable(GlobalNames::RC, VARIABLE_RC), GlobalNames::RC);
    TheCommonRetrievers->put(new RexxSimpleVariable(GlobalNames::RESULT, VARIABLE_RESULT), GlobalNames::RESULT);

    // create the Rexx package so created classes can get added to it.
    createRexxPackage();

//***************************************************************************
// The following Rexx classes that are exposed to the users are set up as
// primitive classes.  These all inherit from object
//***************************************************************************

// macros for simplifying building each class definition.

// start a definition for a new class.  This sets up some variables in a
// local variable scope that identify which behaviours we are working with.
#define StartClassDefinition(name) \
    {\
        RexxBehaviour *currentClassBehaviour = The##name##ClassBehaviour; \
        RexxBehaviour *currentInstanceBehaviour = The##name##Behaviour;   \
        RexxClass *currentClass = The##name##Class;

// define a new class method
#define AddClassMethod(name, entryPoint, args) defineMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddClassProtectedMethod(name, entryPoint, args) defineProtectedMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddClassPrivateMethod(name, entryPoint, args) definePrivateMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddClassUnguardedMethod(name, entryPoint, args) defineUnguardedMethod(name, currentClassBehaviour, CPPM(entryPoint), args, #entryPoint);
#define HideClassMethod(name) currentClassBehaviour->hideMethod(name);
#define RemoveClassMethod(name) currentClassBehaviour->removeMethod(name);

// inherit class method definitions from a previously created class
#define InheritClassMethods(source) currentClassBehaviour->inheritInstanceMethods(The##source##ClassBehaviour);

// define a new instance method
#define AddMethod(name, entryPoint, args) defineMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddProtectedMethod(name, entryPoint, args) defineProtectedMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddPrivateMethod(name, entryPoint, args) definePrivateMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define AddUnguardedMethod(name, entryPoint, args) defineUnguardedMethod(name, currentInstanceBehaviour, CPPM(entryPoint), args, #entryPoint);
#define HideMethod(name) currentInstanceBehaviour->hideMethod(name);
#define RemoveMethod(name) currentInstanceBehaviour->removeMethod(name);

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

// Add the created class object to the environment under its name and close
// the local variable scope
#define EndClassDefinition(name) \
    completeSystemClass(#name, currentClass); \
}

// Finish up one of the special classes (Integer and NumberString).  Those are
// real classes, but are kept hidden.
#define EndSpecialClassDefinition(name) \
    addToSystem(#name, currentClass); \
}

// Notes on defining external methods.  The defined methods create a mapping from
// a Rexx method name to a C++ method that implements a method.  These are created
// via function pointers that are stored in the CPPCode objects backing the object
// method.  The following conditions must be met for an method to be used:
//
// 1)  The method name must be unique on the class (i.e., no multiple methods with
// the same name and different signatures.
// 2)  With the exception of methods define with A_COUNT (see below), all method
// arguments must be RexxObject types and must have a return value that is also a
// RexxObject type.  If the method does not have a return value, the method needs
// to return OREF_NULL.
// 3)  When the method is defined, it identifies the maximum number of arguments it
// can be called with.  This needs to match the number of arguments in the C++ method
// signature.  This dispatcher will sort out omitted arguments and pass OREF_NULL
// for those.  The target method needs to sort out required vs. optional arguments.
// 4)  For methods with an open-ended number of arguments, A_COUNT is specified.  These
// methods must have an signature of "RexxObject *name(RexxObject **args, size_t argCount)"
// to handing being passed a variable-sized argument list.
// 5)  The target method can be defined as virtual, but it is called directory using
// the function pointer and not the object virtual function table.  To get virtual function
// dynamices, create a stub method to act as a bridge between the Rexx method call and
// the virtual method call.
// 6)  These methods are stored in the rexx.img file and they need to reestablish
// their method pointers at startup time.  This is done via a table in CPPCode that
// allows a mapping to be created between the function pointer and a numeric index.
// When you add a new method to the setup below, you also need to update the CPPCode
// table to include the function pointer.  A build error will result if the table is
// not updated.
// 7)  A lot of objects like string or the collection classes are used internally in
// the interpreter.  These typically have a low-level method that performs a function
// and a Rexx stub method that does error checking and any needed conversion before
// calling the lower level method.  These methods by convention have a "Rexx" suffix
// in the name.



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
        AddClassMethod("New", RexxClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // now the normal instance methods for a CLASS object.
        AddProtectedMethod("BaseClass", RexxClass::getBaseClass, 0);
        AddProtectedMethod("Define", RexxClass::defineMethod, 2);
        AddProtectedMethod("DefineMethods", RexxClass::defineMethodsRexx, 1);
        // these two are special and will be removed at the end of
        // the image build
        AddProtectedMethod("DefineClassMethod", RexxClass::defineClassMethod, 2);
        AddProtectedMethod("InheritInstanceMethods", RexxClass::inheritInstanceMethodsRexx, 1)

        AddProtectedMethod("Delete", RexxClass::deleteMethod, 1);
        AddMethod("Enhanced", RexxClass::enhanced, A_COUNT);
        AddMethod("ID", RexxClass::getId, 0);
        AddMethod("Inherit", RexxClass::inherit, 2);
        AddProtectedMethod("MetaClass", RexxClass::getMetaClass, 0);
        AddMethod("Method", RexxClass::method, 1);
        AddMethod("Methods", RexxClass::methods, 1);
        AddMethod("MixinClass", RexxClass::mixinClassRexx, 3);
        AddMethod("QueryMixinClass", RexxClass::queryMixinClass, 0);
        AddMethod("IsMetaClass", RexxClass::isMetaClassRexx, 0);
        AddMethod("IsAbstract", RexxClass::isAbstractRexx, 0);
        AddMethod("Subclass", RexxClass::subclassRexx, 3);
        AddProtectedMethod("Subclasses", RexxClass::getSubClasses, 0);
        AddProtectedMethod("Superclasses", RexxClass::getSuperClasses, 0);
        AddProtectedMethod("Superclass", RexxClass::getSuperClass, 0);
        AddProtectedMethod("Uninherit", RexxClass::uninherit, 1);

        AddMethod("IsSubclassOf", RexxClass::isSubclassOf, 1);
        AddMethod("DefaultName", RexxClass::defaultNameRexx, 0);
        AddMethod("Package", RexxClass::getPackage, 0);
        AddMethod("Copy", RexxClass::copyRexx, 0);

    // operator methods
        AddMethod("=", RexxClass::equal, 1);
        AddMethod("==", RexxClass::strictEqual, 1);
        AddMethod("\\=", RexxClass::notEqual, 1);
        AddMethod("<>", RexxClass::notEqual, 1);
        AddMethod("><", RexxClass::notEqual, 1);
        AddMethod("\\==", RexxClass::notEqual, 1);

    // this is explicitly inserted into the class behaviour because it gets used
    // prior to the instance behavior merges.
        AddMethod("HashCode", RexxClass::hashCode, 0);
    // this is a NOP by default, so we'll just use the object init method as a fill in.
        AddMethod("Activate", RexxObject::initRexx, 0);
        AddMethod("Annotations", RexxClass::getAnnotations, 0);
        AddMethod("Annotation", RexxClass::getAnnotationRexx, 1);

    CompleteMethodDefinitions();

EndClassDefinition(Class);

       /************************************************************************/
       /*                                                                      */
       /* The OBJECT class                                                     */
       /*                                                                      */
       /************************************************************************/

StartClassDefinition(Object);

    // for the OBJECT class object, we only add the NEW method.
        AddClassMethod("New", RexxObject::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // Now Object instance methods

        AddMethod("Init", RexxObject::initRexx, 0);
        AddMethod("=", RexxObject::equal, 1);
        AddMethod("==", RexxObject::strictEqual, 1);
        AddMethod("HashCode", RexxObject::hashCode, 0);
        AddMethod("\\=", RexxObject::notEqual, 1);
        AddMethod("<>", RexxObject::notEqual, 1);
        AddMethod("><", RexxObject::notEqual, 1);
        AddMethod("\\==", RexxObject::strictNotEqual, 1);
        AddMethod("", RexxObject::concatRexx, 1);
        AddMethod(" ", RexxObject::concatBlank, 1);
        AddMethod("||", RexxObject::concatRexx, 1);
        AddMethod("Copy", RexxObject::copyRexx, 0);
        AddMethod("Class", RexxObject::classObject, 0);
        AddMethod("HasMethod", RexxObject::hasMethodRexx, 1);
        AddMethod("DefaultName", RexxObject::defaultNameRexx, 0);
        AddMethod("ObjectName", RexxObject::objectName, 0);
        AddMethod("ObjectName=", RexxObject::objectNameEquals, 1);
        AddMethod("Request", RexxObject::requestRexx, 1);
        AddMethod("Start", RexxObject::start, A_COUNT);
        AddMethod("StartWith", RexxObject::startWith, 2);
        AddMethod("Send", RexxObject::send, A_COUNT);
        AddMethod("SendWith", RexxObject::sendWith, 2);
        AddMethod("String", RexxObject::stringRexx, 0);
        AddMethod("IsInstanceOf", RexxObject::isInstanceOfRexx, 1);
        AddMethod("isNil", RexxObject::isNilRexx, 0);
        AddMethod("IsA", RexxObject::isInstanceOfRexx, 1);
        AddMethod("InstanceMethod", RexxObject::instanceMethodRexx, 1);
        AddMethod("InstanceMethods", RexxObject::instanceMethodsRexx, 1);
        AddMethod("IdentityHash", RexxObject::identityHashRexx, 0);
        AddPrivateMethod("Run", RexxObject::run, A_COUNT);
        AddPrivateMethod("SetMethod", RexxObject::setMethod, 3);
        AddPrivateMethod("UnsetMethod", RexxObject::unsetMethod, 1);

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

        AddClassMethod("New", RexxString::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("", RexxString::concatRexx, 1);
        // this is a synonym
        AddMethod("Append", RexxString::concatRexx, 1);
        AddMethod(" ", RexxString::concatBlank, 1);
        AddMethod("||", RexxString::concatRexx, 1);
        AddMethod("Length", RexxString::lengthRexx, 0);
        AddMethod("Center", RexxString::center, 2);
        AddMethod("Centre", RexxString::center, 2);
        AddMethod("Datatype", RexxString::dataType, 1);
        AddMethod("Delstr", RexxString::delstr, 2);
        AddMethod("Format", RexxString::format, 4);
        AddMethod("Insert", RexxString::insert, 4);
        AddMethod("Left", RexxString::left, 2);
        AddMethod("Overlay", RexxString::overlay, 4);
        AddMethod("ReplaceAt", RexxString::replaceAt, 4);
        AddMethod("Reverse", RexxString::reverse, 0);
        AddMethod("Right", RexxString::right, 2);
        AddMethod("Strip", RexxString::strip, 2);
        AddMethod("Substr", RexxString::substr, 3);
        AddMethod("[]", RexxString::brackets, 2);
        AddMethod("SubChar", RexxString::subchar, 1);
        AddMethod("DelWord", RexxString::delWord, 2);
        AddMethod("Space", RexxString::space, 2);
        AddMethod("SubWord", RexxString::subWord, 2);
        AddMethod("SUBWORDS", RexxString::subWords, 2);
        AddMethod("FLOOR", RexxString::floor, 0);
        AddMethod("CEILING", RexxString::ceiling, 0);
        AddMethod("ROUND", RexxString::round, 0);
        AddMethod("Trunc", RexxString::trunc, 1);
        AddMethod("modulo", RexxString::modulo, 1);
        AddMethod("Word", RexxString::word, 1);
        AddMethod("WordIndex", RexxString::wordIndex, 1);
        AddMethod("WordLength", RexxString::wordLength, 1);
        AddMethod("WordPos", RexxString::wordPos, 2);
        AddMethod("CaselessWordPos", RexxString::caselessWordPos, 2);
        AddMethod("Words", RexxString::words, 0);
        AddMethod("Abbrev", RexxString::abbrev, 2);
        AddMethod("CaselessAbbrev", RexxString::caselessAbbrev, 2);
        AddMethod("ChangeStr", RexxString::changeStr, 3);
        AddMethod("CaselessChangeStr", RexxString::caselessChangeStr, 3);
        AddMethod("Compare", RexxString::compare, 2);
        AddMethod("CaselessCompare", RexxString::caselessCompare, 2);
        AddMethod("Copies", RexxString::copies, 1);
        AddMethod("CountStr", RexxString::countStrRexx, 1);
        AddMethod("CaselessCountStr", RexxString::caselessCountStrRexx, 1);
        AddMethod("LastPos", RexxString::lastPosRexx, 3);
        AddMethod("Pos", RexxString::posRexx, 3);
        AddMethod("CONTAINS", RexxString::containsRexx, 3);
        AddMethod("CASELESSCONTAINS", RexxString::caselessContains, 3);
        AddMethod("CONTAINSWORD", RexxString::containsWord, 2);
        AddMethod("CASELESSCONTAINSWORD", RexxString::caselessContainsWord, 2);
        AddMethod("CaselessLastPos", RexxString::caselessLastPosRexx, 3);
        AddMethod("CaselessPos", RexxString::caselessPosRexx, 3);
        AddMethod("Translate", RexxString::translate, 5);
        AddMethod("Verify", RexxString::verify, 4);
        AddMethod("BitAnd", RexxString::bitAnd, 2);
        AddMethod("BitOr", RexxString::bitOr, 2);
        AddMethod("BitXor", RexxString::bitXor, 2);
        AddMethod("B2X", RexxString::b2x, 0);
        AddMethod("C2D", RexxString::c2d, 1);
        AddMethod("C2X", RexxString::c2x, 0);
        AddMethod("D2C", RexxString::d2c, 1);
        AddMethod("D2X", RexxString::d2x, 1);
        AddMethod("X2B", RexxString::x2b, 0);
        AddMethod("X2C", RexxString::x2c, 0);
        AddMethod("X2D", RexxString::x2d, 1);
        AddMethod("EncodeBase64", RexxString::encodeBase64, 0);
        AddMethod("DecodeBase64", RexxString::decodeBase64, 0);
        AddMethod("MakeString", RexxObject::makeStringRexx, 0);
        AddMethod("Abs", RexxString::abs, 0);
        AddMethod("Max", RexxString::Max, A_COUNT);
        AddMethod("Min", RexxString::Min, A_COUNT);
        AddMethod("Sign", RexxString::sign, 0);
        AddMethod("=", RexxString::equal, 1);
        AddMethod("\\=", RexxString::notEqual, 1);
        AddMethod("<>", RexxString::notEqual, 1);
        AddMethod("><", RexxString::notEqual, 1);
        AddMethod(">", RexxString::isGreaterThan, 1);
        AddMethod("<", RexxString::isLessThan, 1);
        AddMethod(">=", RexxString::isGreaterOrEqual, 1);
        AddMethod("\\<", RexxString::isGreaterOrEqual, 1);
        AddMethod("<=", RexxString::isLessOrEqual, 1);
        AddMethod("\\>", RexxString::isLessOrEqual, 1);
        AddMethod("==", RexxString::strictEqual, 1);
        AddMethod("\\==", RexxString::strictNotEqual, 1);
        AddMethod(">>", RexxString::strictGreaterThan, 1);
        AddMethod("<<", RexxString::strictLessThan, 1);
        AddMethod(">>=", RexxString::strictGreaterOrEqual, 1);
        AddMethod("\\<<", RexxString::strictGreaterOrEqual, 1);
        AddMethod("<<=", RexxString::strictLessOrEqual, 1);
        AddMethod("\\>>", RexxString::strictLessOrEqual, 1);
        AddMethod("+", RexxString::plus, 1);
        AddMethod("-", RexxString::minus, 1);
        AddMethod("*", RexxString::multiply, 1);
        AddMethod("**", RexxString::power, 1);
        AddMethod("/", RexxString::divide, 1);
        AddMethod("%", RexxString::integerDivide, 1);
        AddMethod("//", RexxString::remainder, 1);
        AddMethod("\\", RexxString::notOp, 0);
        AddMethod("&", RexxString::andOp, 1);
        AddMethod("|", RexxString::orOp, 1);
        AddMethod("&&", RexxString::xorOp, 1);
        AddMethod("?", RexxString::choiceRexx, 2);
        AddMethod("MakeArray", RexxString::makeArrayRexx, 1);
        AddMethod("Lower", RexxString::lowerRexx, 2);
        AddMethod("Upper", RexxString::upperRexx, 2);
        AddMethod("Match", RexxString::match, 4);
        AddMethod("CaselessMatch", RexxString::caselessMatch, 4);
        AddMethod("MatchChar", RexxString::matchChar, 2);
        AddMethod("CaselessMatchChar", RexxString::caselessMatchChar, 2);
        AddMethod("Equals", RexxString::equals, 1);
        AddMethod("CaselessEquals", RexxString::caselessEquals, 1);
        AddMethod("CompareTo", RexxString::compareToRexx, 3);
        AddMethod("CaselessCompareTo", RexxString::caselessCompareToRexx, 3);
        AddMethod("StartsWith", RexxString::startsWithRexx, 1);
        AddMethod("EndsWith", RexxString::endsWithRexx, 1);
        AddMethod("CaselessStartsWith", RexxString::caselessStartsWithRexx, 1);
        AddMethod("CaselessEndsWith", RexxString::caselessEndsWithRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(String);

EndClassDefinition(String);


    /***************************************************************************/
    /* ARRAY class                                                             */
    /***************************************************************************/

StartClassDefinition(Array);

        AddClassMethod("New", ArrayClass::newRexx, A_COUNT);
        AddClassMethod("Of", ArrayClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("[]", ArrayClass::getRexx, A_COUNT);
        AddMethod("[]=", ArrayClass::putRexx, A_COUNT);
        AddMethod("At", ArrayClass::getRexx, A_COUNT);
        AddMethod("Dimension", ArrayClass::dimensionRexx, 1);
        AddMethod("Dimensions", ArrayClass::getDimensionsRexx, 0);
        AddMethod("HasIndex", ArrayClass::hasIndexRexx, A_COUNT);
        AddMethod("Items", ArrayClass::itemsRexx, 0);
        AddMethod("MakeArray", RexxObject::makeArrayRexx, 0);
        AddMethod("Put", ArrayClass::putRexx, A_COUNT);
        AddMethod("Remove", ArrayClass::removeRexx, A_COUNT);
        AddMethod("Section", ArrayClass::sectionRexx, 2);
        AddMethod("Size", ArrayClass::sizeRexx, 0);
        AddMethod("Supplier", ArrayClass::supplier, 0);
        AddMethod("First", ArrayClass::firstRexx, 0);
        AddMethod("FirstItem", ArrayClass::getFirstItem, 0);
        AddMethod("Last", ArrayClass::lastRexx, 0);
        AddMethod("LastItem", ArrayClass::getLastItem, 0);
        AddMethod("Next", ArrayClass::nextRexx, A_COUNT);
        AddMethod("Previous", ArrayClass::previousRexx, A_COUNT);
        AddMethod("Append", ArrayClass::appendRexx, 1);
        AddMethod("MakeString", ArrayClass::toString, 2);
        AddMethod("ToString", ArrayClass::toString, 2);
        AddMethod("AllIndexes", ArrayClass::allIndexes, 0);
        AddMethod("AllItems", ArrayClass::allItems, 0);
        AddMethod("Empty", ArrayClass::empty, 0);
        AddMethod("IsEmpty", ArrayClass::isEmptyRexx, 0);
        AddMethod("Index", ArrayClass::indexRexx, 1);
        AddMethod("HasItem", ArrayClass::hasItemRexx, 1);
        AddMethod("RemoveItem", ArrayClass::removeItemRexx, 1);
        AddMethod("Insert", ArrayClass::insertRexx, 2);
        AddMethod("Delete", ArrayClass::deleteRexx, 1);
        AddMethod("Fill", ArrayClass::fillRexx, 1);

    // there have been some problems with the quick sort used as the default sort, so map everything
    // to the stable sort.  The stable sort, in theory, uses more memory, but in practice, this is not true.
        AddMethod("Sort", ArrayClass::stableSortRexx, 0);
        AddMethod("SortWith", ArrayClass::stableSortWithRexx, 1);
        AddMethod("StableSort", ArrayClass::stableSortRexx, 0);
        AddMethod("StableSortWith", ArrayClass::stableSortWithRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Array);

EndClassDefinition(Array);


    /***************************************************************************/
    /*           QUEUE                                                         */
    /***************************************************************************/

StartClassDefinition(Queue);

        AddClassMethod("New", QueueClass::newRexx, A_COUNT);
        AddClassMethod("Of", QueueClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // QUEUE is implemented as a subclass of Array, so most of its instance
    // methods are the same as the Array ones.  We can inherit those in one shot,
    // then make whatever required additions or replacements.

        InheritInstanceMethods(Array);

        AddMethod("Init", QueueClass::initRexx, 1);
        AddMethod("Push", QueueClass::pushRexx, 1);
        AddMethod("Peek", QueueClass::peek, 0);
        AddMethod("Pull", QueueClass::pullRexx, 0);
        AddMethod("Queue", QueueClass::queueRexx, 1);
        AddMethod("Put", QueueClass::putRexx, 2);
        // the queue size is always the number of items, so remap that call
        // to the array items method.
        AddMethod("Size", ArrayClass::itemsRexx, 0);

        // remove some inherited array methods that make no sense for queues
        RemoveMethod("Dimension");
        RemoveMethod("Dimensions");
        RemoveMethod("Fill");

        // to be consistent with our other Collections, also
        // - remove all four sort methods (will always be inherited from OrderedCollection)
        // - remove makeString, toString
        RemoveMethod("sort");
        RemoveMethod("sortWith");
        RemoveMethod("stableSort");
        RemoveMethod("stableSortWith");
        RemoveMethod("makeString");
        RemoveMethod("toString");

    CompleteMethodDefinitions();

    CompleteClassDefinition(Queue);

EndClassDefinition(Queue);

    /***************************************************************************/
    /*           IDENTITYTABLE                                                 */
    /***************************************************************************/

// Do all of the hash-based collections as a group, since we can directly inherit a
// lot of the methods from previous collections.

StartClassDefinition(IdentityTable);

        AddClassMethod("New", IdentityTable::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("[]", HashCollection::getRexx, 1);
        AddMethod("[]=", HashCollection::putRexx, 2);
        AddMethod("MakeArray", RexxObject::makeArrayRexx, 0);
        AddMethod("At", HashCollection::getRexx, 1);
        AddMethod("HasIndex", HashCollection::hasIndexRexx, 1);
        AddMethod("Items", HashCollection::itemsRexx, 0);
        AddMethod("Put", HashCollection::putRexx, 2);
        AddMethod("Remove", HashCollection::removeRexx, 1);
        AddMethod("Supplier", HashCollection::supplier, 0);
        AddMethod("AllItems", HashCollection::allItems, 0);
        AddMethod("AllIndexes", HashCollection::allIndexes, 0);
        AddMethod("Empty", HashCollection::emptyRexx, 0);
        AddMethod("IsEmpty", HashCollection::isEmptyRexx, 0);
        AddMethod("Index", HashCollection::indexRexx, 1);
        AddMethod("HasItem", HashCollection::hasItemRexx, 1);
        AddMethod("RemoveItem", HashCollection::removeItemRexx, 1);
        AddMethod("Init", HashCollection::initRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(IdentityTable);

EndClassDefinition(IdentityTable);


    /***************************************************************************/
    /*           TABLE                                                         */
    /***************************************************************************/

StartClassDefinition(Table);

        AddClassMethod("New", TableClass::newRexx, A_COUNT);

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

        AddClassMethod("New", StringTable::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        // most of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

        AddMethod("Unknown", StringHashCollection::unknownRexx, 2);
        AddMethod("Entry", StringHashCollection::entryRexx, 1);
        AddMethod("HasEntry", StringHashCollection::hasEntryRexx, 1);
        AddMethod("SetEntry", StringHashCollection::setEntryRexx, 2);
        AddMethod("RemoveEntry", StringHashCollection::removeEntryRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(StringTable);

EndClassDefinition(StringTable);


    /***************************************************************************/
    /*           SET                                                           */
    /***************************************************************************/

StartClassDefinition(Set)

        AddClassMethod("New", SetClass::newRexx, A_COUNT);
        AddClassMethod("Of", SetClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        // most of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

        // hasItem() is the same as hasIndex() for the Set class
        AddMethod("HasItem", IdentityTable::hasIndexRexx, 1);
        // removeItem() is the same as remove() for the Set class
        AddMethod("RemoveItem", IdentityTable::removeRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Set);

EndClassDefinition(Set);


    /***************************************************************************/
    /*           DIRECTORY                                                     */
    /***************************************************************************/

StartClassDefinition(Directory)

        AddClassMethod("New", DirectoryClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // many of the hash collection methods can be inherited
        InheritInstanceMethods(StringTable);

        AddMethod("Init", DirectoryClass::initRexx, 1);

        AddProtectedMethod("SetMethod", DirectoryClass::setMethodRexx, 2);
        AddProtectedMethod("UnsetMethod", DirectoryClass::unsetMethodRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Directory);

EndClassDefinition(Directory);


    /***************************************************************************/
    /*           RELATION                                                      */
    /***************************************************************************/

StartClassDefinition(Relation)

        AddClassMethod("New", RelationClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // many of the hash collection methods can be inherited
        InheritInstanceMethods(IdentityTable);

        AddMethod("RemoveItem", RelationClass::removeItemRexx, 2);
        AddMethod("Supplier", RelationClass::supplierRexx, 1);
        AddMethod("Items", RelationClass::itemsRexx, 1);
        AddMethod("HasItem", RelationClass::hasItemRexx, 2);
        AddMethod("AllIndex", RelationClass::allIndexRexx, 1);
        AddMethod("AllAt", RelationClass::allAt, 1);
        AddMethod("RemoveAll", RelationClass::removeAll, 1);
        AddMethod("UniqueIndexes", RelationClass::uniqueIndexes, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Relation);

EndClassDefinition(Relation);


    /***************************************************************************/
    /*           Bag                                                           */
    /***************************************************************************/

StartClassDefinition(Bag)

        AddClassMethod("New", BagClass::newRexx, A_COUNT);
        AddClassMethod("Of", BagClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        // many of the hash collection methods can be inherited
        InheritInstanceMethods(Relation);

        AddMethod("HasItem", BagClass::hasItemRexx, 2);
        AddMethod("RemoveItem", BagClass::removeItemRexx, 2);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Bag);

EndClassDefinition(Bag);


    /***************************************************************************/
    /*           LIST                                                          */
    /***************************************************************************/

StartClassDefinition(List)

        AddClassMethod("New", ListClass::newRexx, A_COUNT);
        AddClassMethod("Of", ListClass::ofRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Init", ListClass::initRexx, 1);
        AddMethod("[]", ListClass::getRexx, 1);
        AddMethod("[]=", ListClass::putRexx, 2);
        AddMethod("MakeArray", RexxObject::makeArrayRexx, 0);
        AddMethod("At", ListClass::getRexx, 1);
        AddMethod("FirstItem", ListClass::firstItemRexx, 0);
        AddMethod("HasIndex", ListClass::hasIndexRexx, 1);
        AddMethod("Insert", ListClass::insertRexx, 2);
        AddMethod("Items", ListClass::itemsRexx, 0);
        AddMethod("LastItem", ListClass::lastItemRexx, 0);
        AddMethod("First", ListClass::firstRexx, 0);
        AddMethod("Last", ListClass::lastRexx, 0);
        AddMethod("Next", ListClass::nextRexx, 1);
        AddMethod("Previous", ListClass::previousRexx, 1);
        AddMethod("Put", ListClass::putRexx, 2);
        AddMethod("Remove", ListClass::removeRexx, 1);
    // DELETE is the same as REMOVE for the List class
        AddMethod("Delete", ListClass::removeRexx, 1);
        AddMethod("Section", ListClass::sectionRexx, 2);
        AddMethod("Supplier", ListClass::supplier, 0);
        AddMethod("Append", ListClass::appendRexx, 1);
        AddMethod("AllItems", ListClass::allItems, 0);
        AddMethod("AllIndexes", ListClass::allIndexes, 0);
        AddMethod("Empty", ListClass::emptyRexx, 0);
        AddMethod("IsEmpty", ListClass::isEmptyRexx, 0);
        AddMethod("Index", ListClass::indexRexx, 1);
        AddMethod("HasItem", ListClass::hasItemRexx, 1);
        AddMethod("RemoveItem", ListClass::removeItemRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(List);

EndClassDefinition(List);

    /***************************************************************************/
    /*           MESSAGE                                                       */
    /***************************************************************************/

StartClassDefinition(Message)

        AddClassMethod("New", MessageClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Completed", MessageClass::completed, 0);
        AddMethod("HasError", MessageClass::hasError, 0);
        AddMethod("HasResult", MessageClass::hasResult, 0);
        AddMethod("Notify", MessageClass::notify, 1);
        AddMethod("Result", MessageClass::result, 0);
        AddMethod("Target", MessageClass::messageTarget, 0);
        AddMethod("MessageName", MessageClass::messageName, 0);
        AddMethod("Arguments", MessageClass::arguments, 0);
        AddMethod("ErrorCondition", MessageClass::errorCondition, 0);
        AddMethod("Send", MessageClass::sendRexx, A_COUNT);
        AddMethod("Start", MessageClass::startRexx, A_COUNT);
        AddMethod("Reply", MessageClass::replyRexx, A_COUNT);
        AddMethod("SendWith", MessageClass::sendWithRexx, 2);
        AddMethod("StartWith", MessageClass::startWithRexx, 2);
        AddMethod("ReplyWith", MessageClass::replyWithRexx, 2);
        // these are both just for triggering a send event for event
        // completions.  The map to the same method
        AddMethod("MessageComplete", MessageClass::messageCompleted, 1);
        AddMethod("Triggered", MessageClass::messageCompleted, 1);
        AddMethod("Wait", MessageClass::wait, 0);
        AddMethod("Halt", MessageClass::halt, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Message);

EndClassDefinition(Message);


    /***************************************************************************/
    /*           METHOD                                                        */
    /***************************************************************************/

StartClassDefinition(Method)

        AddClassMethod("New", MethodClass::newRexx, A_COUNT);
        AddClassMethod("NewFile", MethodClass::newFileRexx, 2);
        AddClassMethod("LoadExternalMethod", MethodClass::loadExternalMethod, 2);

    CompleteClassMethodDefinitions();

        AddMethod("SetUnguarded", MethodClass::setUnguardedRexx, 0);
        AddMethod("SetGuarded", MethodClass::setGuardedRexx, 0);
        AddMethod("SetPrivate", MethodClass::setPrivateRexx, 0);
        AddMethod("IsGuarded", MethodClass::isGuardedRexx, 0);
        AddMethod("IsPrivate", MethodClass::isPrivateRexx, 0);
        AddMethod("IsPackage", MethodClass::isPackageRexx, 0);
        AddMethod("IsProtected", MethodClass::isProtectedRexx, 0);
        AddMethod("IsAbstract", MethodClass::isAbstractRexx, 0);
        AddMethod("IsConstant", MethodClass::isConstantRexx, 0);
        AddMethod("IsAttribute", MethodClass::isAttributeRexx, 0);
        AddProtectedMethod("SetProtected", MethodClass::setProtectedRexx, 0);
        AddProtectedMethod("SetSecurityManager", MethodClass::setSecurityManager, 1);
        AddMethod("Source", BaseExecutable::source, 0);
        AddMethod("Package", BaseExecutable::getPackage, 0);
        AddMethod("Annotations", BaseExecutable::getAnnotations, 0);
        AddMethod("Annotation", BaseExecutable::getAnnotationRexx, 1);
        AddMethod("Scope", MethodClass::getScopeRexx, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Method);

EndClassDefinition(Method);


    /***************************************************************************/
    /*           ROUTINE                                                       */
    /***************************************************************************/

StartClassDefinition(Routine)

        AddClassMethod("New", RoutineClass::newRexx, A_COUNT);
        AddClassMethod("NewFile", RoutineClass::newFileRexx, 2);
        AddClassMethod("LoadExternalRoutine", RoutineClass::loadExternalRoutine, 2);

    CompleteClassMethodDefinitions();

        AddProtectedMethod("SetSecurityManager", RoutineClass::setSecurityManager, 1);
        AddMethod("Source", BaseExecutable::source, 0);
        AddMethod("Package", BaseExecutable::getPackage, 0);
        AddMethod("Call", RoutineClass::callRexx, A_COUNT);
        AddMethod("[]", RoutineClass::callRexx, A_COUNT);
        AddMethod("CallWith", RoutineClass::callWithRexx, 1);
        AddMethod("Annotations", BaseExecutable::getAnnotations, 0);
        AddMethod("Annotation", BaseExecutable::getAnnotationRexx, 1);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Routine);

EndClassDefinition(Routine);


    /***************************************************************************/
    /*           Package                                                       */
    /***************************************************************************/

StartClassDefinition(Package)

        AddClassMethod("New", PackageClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddProtectedMethod("SetSecurityManager", PackageClass::setSecurityManagerRexx, 1);
        AddMethod("Source", PackageClass::getSourceRexx, 0);
        AddMethod("SourceLine", PackageClass::getSourceLineRexx, 1);
        AddMethod("SourceSize", PackageClass::getSourceSizeRexx, 0);
        AddMethod("Classes", PackageClass::getClassesRexx, 0);
        AddMethod("PublicClasses", PackageClass::getPublicClassesRexx, 0);
        AddMethod("ImportedClasses", PackageClass::getImportedClassesRexx, 0);
        AddMethod("DefinedMethods", PackageClass::getMethodsRexx, 0);
        AddMethod("Resources", PackageClass::getResourcesRexx, 0);
        AddMethod("Resource", PackageClass::getResourceRexx, 1);
        AddMethod("Namespaces", PackageClass::getNamespacesRexx, 0);
        AddMethod("Annotations", PackageClass::getAnnotations, 0);
        AddMethod("Annotation", PackageClass::getAnnotationRexx, 1);
        AddMethod("Routines", PackageClass::getRoutinesRexx, 0);
        AddMethod("PublicRoutines", PackageClass::getPublicRoutinesRexx, 0);
        AddMethod("ImportedRoutines", PackageClass::getImportedRoutinesRexx, 0);
        AddMethod("ImportedPackages", PackageClass::getImportedPackagesRexx, 0);
        AddMethod("LoadPackage", PackageClass::loadPackageRexx, 2);
        AddMethod("AddPackage", PackageClass::addPackageRexx, 2);
        AddMethod("FindClass", PackageClass::findClassRexx, 1);
        AddMethod("FindRoutine", PackageClass::findRoutineRexx, 1);
        AddMethod("FindPublicClass", PackageClass::findPublicClassRexx, 1);
        AddMethod("FindPublicRoutine", PackageClass::findPublicRoutineRexx, 1);
        AddMethod("FindNamespace", PackageClass::findNamespaceRexx, 1);
        AddMethod("AddRoutine", PackageClass::addRoutineRexx, 2);
        AddMethod("AddPublicRoutine", PackageClass::addPublicRoutineRexx, 2);
        AddMethod("AddClass", PackageClass::addClassRexx, 2);
        AddMethod("AddPublicClass", PackageClass::addPublicClassRexx, 2);
        AddMethod("Name", PackageClass::getProgramName, 0);
        AddMethod("LoadLibrary", PackageClass::loadLibraryRexx, 1);
        AddMethod("Digits", PackageClass::digitsRexx, 0);
        AddMethod("Form", PackageClass::formRexx, 0);
        AddMethod("Fuzz", PackageClass::fuzzRexx, 0);
        AddMethod("Trace", PackageClass::traceRexx, 0);
        AddMethod("Prolog", PackageClass::getMainRexx, 0);
        AddMethod("FindProgram", PackageClass::findProgramRexx, 1);
        AddMethod("Local", PackageClass::getPackageLocal, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Package);

EndClassDefinition(Package);


    /***************************************************************************/
    /*           RexxContext                                                   */
    /***************************************************************************/

StartClassDefinition(RexxContext)

        AddClassMethod("New", RexxContext::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Copy", RexxContext::copyRexx, 0);
        AddMethod("Package", RexxContext::getPackage, 0);
        AddMethod("Executable", RexxContext::getExecutable, 0);
        AddMethod("Form", RexxContext::getForm, 0);
        AddMethod("Fuzz", RexxContext::getFuzz, 0);
        AddMethod("Digits", RexxContext::getDigits, 0);
        AddMethod("Variables", RexxContext::getVariables, 0);
        AddMethod("Args", RexxContext::getArgs, 0);
        AddMethod("Condition", RexxContext::getCondition, 0);
        AddMethod("Line", RexxContext::getLine, 0);
        AddMethod("RS", RexxContext::getRS, 0);
        AddMethod("Name", RexxContext::getName, 0);
        AddMethod("StackFrames", RexxContext::getStackFrames, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(RexxContext);

EndClassDefinition(RexxContext);


    /***************************************************************************/
    /*           RexxInfo                                                      */
    /***************************************************************************/

StartClassDefinition(RexxInfo)

AddClassMethod("New", RexxInfo::newRexx, A_COUNT);

CompleteClassMethodDefinitions();

AddMethod("Copy", RexxInfo::copyRexx, 0);
AddMethod("Package", RexxInfo::getPackage, 0);
AddMethod("Digits", RexxInfo::getDigits, 0);
AddMethod("InternalDigits", RexxInfo::getInternalDigits, 0);
AddMethod("Form", RexxInfo::getForm, 0);
AddMethod("Fuzz", RexxInfo::getFuzz, 0);
AddMethod("LanguageLevel", RexxInfo::getLanguageLevel, 0);
AddMethod("Version", RexxInfo::getInterpreterVersion, 0);
AddMethod("Name", RexxInfo::getInterpreterName, 0);
AddMethod("Date", RexxInfo::getInterpreterDate, 0);
AddMethod("Platform", RexxInfo::getPlatform, 0);
AddMethod("Architecture", RexxInfo::getArchitecture, 0);
AddMethod("EndOfLine", RexxInfo::getFileEndOfLine, 0);
AddMethod("PathSeparator", RexxInfo::getPathSeparator, 0);
AddMethod("DirectorySeparator", RexxInfo::getDirectorySeparator, 0);
AddMethod("CaseSensitiveFiles", RexxInfo::getCaseSensitiveFiles, 0);
AddMethod("MajorVersion", RexxInfo::getMajorVersion, 0);
AddMethod("Release", RexxInfo::getRelease, 0);
AddMethod("Modification", RexxInfo::getModification, 0);
AddMethod("Revision", RexxInfo::getRevision, 0);
AddMethod("internalMaxNumber", RexxInfo::getInternalMaxNumber, 0);
AddMethod("internalMinNumber", RexxInfo::getInternalMinNumber, 0);
AddMethod("maxExponent", RexxInfo::getMaxExponent, 0);
AddMethod("minExponent", RexxInfo::getMinExponent, 0);
AddMethod("maxPathLength", RexxInfo::getMaxPathLength, 0);
AddMethod("maxArraySize", RexxInfo::getMaxArraySize, 0);
        AddMethod("executable", RexxInfo::getRexxExecutable, 0);
        AddMethod("libraryPath", RexxInfo::getRexxLibrary, 0);
AddMethod("debug", RexxInfo::getDebug, 0);

CompleteMethodDefinitions();

CompleteClassDefinition(RexxInfo);

EndSpecialClassDefinition(RexxInfo);


/***************************************************************************/
/*           VariableReference                                             */
/***************************************************************************/

StartClassDefinition(VariableReference)

AddClassMethod("New", VariableReference::newRexx, A_COUNT);

CompleteClassMethodDefinitions();

AddMethod("Name", VariableReference::getName, 0);
AddMethod("Value", VariableReference::getValue, 0);
AddMethod("Value=", VariableReference::setValueRexx, 1);
AddMethod("Unknown", VariableReference::unknownRexx, 2);
AddMethod("Request", VariableReference::request, 1);

// We want various operator methods that we inherit from the object
// class to be redirected to our unknown method, so we block these methods
// in our instance method directory.
HideMethod("==");
HideMethod("=");
HideMethod("\\==");
HideMethod("\\=");
HideMethod("<>");
HideMethod("><");

CompleteMethodDefinitions();

CompleteClassDefinition(VariableReference);

EndClassDefinition(VariableReference);


/***************************************************************************/
/*           EventSemaphore                                                */
/***************************************************************************/

StartClassDefinition(EventSemaphore)

AddClassMethod("New", EventSemaphoreClass::newRexx, A_COUNT);

CompleteClassMethodDefinitions();

AddMethod("Uninit", EventSemaphoreClass::close, 0);
AddUnguardedMethod("Post", EventSemaphoreClass::post, 0);
AddUnguardedMethod("Reset", EventSemaphoreClass::reset, 0);
AddUnguardedMethod("Wait", EventSemaphoreClass::wait, 1);
AddUnguardedMethod("IsPosted", EventSemaphoreClass::posted, 0);

CompleteMethodDefinitions();

CompleteClassDefinition(EventSemaphore);

EndClassDefinition(EventSemaphore);


/***************************************************************************/
/*           MutexSemaphore                                                */
/***************************************************************************/

StartClassDefinition(MutexSemaphore)

AddClassMethod("New", MutexSemaphoreClass::newRexx, A_COUNT);

CompleteClassMethodDefinitions();

AddMethod("Uninit", MutexSemaphoreClass::close, 0);
AddUnguardedMethod("Release", MutexSemaphoreClass::release, 0);
AddUnguardedMethod("Acquire", MutexSemaphoreClass::request, 1);

CompleteMethodDefinitions();

CompleteClassDefinition(MutexSemaphore);

EndClassDefinition(MutexSemaphore);


/***************************************************************************/
/*           STEM                                                          */
/***************************************************************************/

StartClassDefinition(Stem)

        AddClassMethod("New", StemClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("[]", StemClass::bracket, A_COUNT);
        AddMethod("[]=", StemClass::bracketEqual, A_COUNT);
        AddMethod("At", StemClass::bracket, A_COUNT);
        AddMethod("Put", StemClass::bracketEqual, A_COUNT);
        AddMethod("MakeArray", RexxObject::makeArrayRexx, 0);
        AddMethod("Request", StemClass::request, 1);
        AddMethod("Supplier", StemClass::supplier, 0);
        AddMethod("AllIndexes", StemClass::allIndexes, 0);
        AddMethod("AllItems", StemClass::allItems, 0);
        AddMethod("Empty", StemClass::empty, 0);
        AddMethod("IsEmpty", StemClass::isEmptyRexx, 0);
        AddMethod("Unknown", StemClass::unknownRexx, 2);

        AddMethod("Items", StemClass::itemsRexx, 0);
        AddMethod("HasIndex", StemClass::hasIndex, A_COUNT);
        AddMethod("Remove", StemClass::remove, A_COUNT);
        AddMethod("Index", StemClass::index, 1);
        AddMethod("HasItem", StemClass::hasItem, 1);
        AddMethod("RemoveItem", StemClass::removeItemRexx, 1);
        AddMethod("ToDirectory", StemClass::toDirectory, 0);

    // We want various operator methods that we inherit from the object
    // class to be redirected to our unknown method, so we block these methods
    // in our instance method directory.
        HideMethod("==");
        HideMethod("=");
        HideMethod("\\==");
        HideMethod("\\=");
        HideMethod("<>");
        HideMethod("><");

    CompleteMethodDefinitions();

    CompleteClassDefinition(Stem);

EndClassDefinition(Stem);

    /***************************************************************************/
    /*           MUTABLEBUFFER                                                 */
    /***************************************************************************/

StartClassDefinition(MutableBuffer)

        AddClassMethod("New", MutableBuffer::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Append", MutableBuffer::appendRexx, A_COUNT);
        AddMethod("Insert", MutableBuffer::insert, 4);
        AddMethod("Overlay", MutableBuffer::overlay, 4);
        AddMethod("ReplaceAt", MutableBuffer::replaceAt, 4);
        AddMethod("[]=", MutableBuffer::bracketsEqual, 3);
        AddMethod("Delete", MutableBuffer::mydelete, 2);
        AddMethod("DelStr", MutableBuffer::mydelete, 2);
        AddMethod("Substr", MutableBuffer::substr, 3);
        AddMethod("[]", MutableBuffer::brackets, 2);
        AddMethod("Pos", MutableBuffer::posRexx, 3);
        AddMethod("LastPos", MutableBuffer::lastPos, 3);
        AddMethod("Contains", MutableBuffer::containsRexx, 3);
        AddMethod("CaselessContains", MutableBuffer::caselessContains, 3);
        AddMethod("ContainsWord", MutableBuffer::containsWord, 2);
        AddMethod("CaselessContainsWord", MutableBuffer::caselessContainsWord, 2);
        AddMethod("CaselessPos", MutableBuffer::caselessPos, 3);
        AddMethod("CaselessLastPos", MutableBuffer::caselessLastPos, 3);
        AddMethod("SubChar", MutableBuffer::subchar, 1);
        AddMethod("GetBufferSize", MutableBuffer::getBufferSize, 0);
        AddMethod("SetBufferSize", MutableBuffer::setBufferSize, 1);
        AddMethod("SetText", MutableBuffer::setTextRexx, 1);

        AddMethod("Length", MutableBuffer::lengthRexx, 0);
        AddMethod("MakeArray", MutableBuffer::makeArrayRexx, 1);
        AddMethod("String", RexxObject::makeStringRexx, 0);
        AddMethod("CountStr", MutableBuffer::countStrRexx, 1);
        AddMethod("CaselessCountStr", MutableBuffer::caselessCountStrRexx, 1);
        AddMethod("ChangeStr", MutableBuffer::changeStr, 3);
        AddMethod("CaselessChangeStr", MutableBuffer::caselessChangeStr, 3);
        AddMethod("Upper", MutableBuffer::upper, 2);
        AddMethod("Lower", MutableBuffer::lower, 2);
        AddMethod("Translate", MutableBuffer::translate, 5);
        AddMethod("Match", MutableBuffer::match, 4);
        AddMethod("CaselessMatch", MutableBuffer::caselessMatch, 4);
        AddMethod("MatchChar", MutableBuffer::matchChar, 2);
        AddMethod("CaselessMatchChar", MutableBuffer::caselessMatchChar, 2);
        AddMethod("Verify", MutableBuffer::verify, 4);
        AddMethod("Space", MutableBuffer::space, 2);
        AddMethod("SubWord", MutableBuffer::subWord, 2);
        AddMethod("SubWords", MutableBuffer::subWords, 2);
        AddMethod("Word", MutableBuffer::word, 1);
        AddMethod("WordIndex", MutableBuffer::wordIndex, 1);
        AddMethod("WordLength", MutableBuffer::wordLength, 1);
        AddMethod("Words", MutableBuffer::words, 0);
        AddMethod("WordPos", MutableBuffer::wordPos, 2);
        AddMethod("CaselessWordPos", MutableBuffer::caselessWordPos, 2);
        AddMethod("DelWord", MutableBuffer::delWord, 2);
        AddMethod("StartsWith", MutableBuffer::startsWithRexx, 1);
        AddMethod("EndsWith", MutableBuffer::endsWithRexx, 1);
        AddMethod("CaselessStartsWith", MutableBuffer::caselessStartsWithRexx, 1);
        AddMethod("CaselessEndsWith", MutableBuffer::caselessEndsWithRexx, 1);
        AddMethod("makeString", MutableBuffer::makeStringRexx, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(MutableBuffer);

EndClassDefinition(MutableBuffer);

    /***************************************************************************/
    /*             INTEGER                                                     */
    /***************************************************************************/

StartClassDefinition(Integer)

// NOTE that we are pointing the new method at the String version...part of the
// fakeout that the Integer class does.
AddClassMethod("New", RexxString::newRexx, A_COUNT);

CompleteClassMethodDefinitions();

AddMethod("+", RexxInteger::plus, 1);
AddMethod("-", RexxInteger::minus, 1);
AddMethod("*", RexxInteger::multiply, 1);
AddMethod("**", RexxInteger::power, 1);
AddMethod("/", RexxInteger::divide, 1);
AddMethod("%", RexxInteger::integerDivide, 1);
AddMethod("//", RexxInteger::remainder, 1);
AddMethod("\\", RexxInteger::notOp, 0);
AddMethod("&", RexxInteger::andOp, 1);
AddMethod("|", RexxInteger::orOp, 1);
AddMethod("&&", RexxInteger::xorOp, 1);
AddMethod("?", RexxInteger::choiceRexx, 2);
AddMethod("D2C", RexxInteger::d2c, 1);
AddMethod("D2X", RexxInteger::d2x, 1);
AddMethod("Abs", RexxInteger::abs, 0);
AddMethod("Max", RexxInteger::Max, A_COUNT);
AddMethod("Min", RexxInteger::Min, A_COUNT);
AddMethod("Sign", RexxInteger::sign, 0);
AddMethod("Equal", RexxInteger::equal, 1);
AddMethod("=", RexxInteger::equal, 1);
AddMethod("\\=", RexxInteger::notEqual, 1);
AddMethod("<>", RexxInteger::notEqual, 1);
AddMethod("><", RexxInteger::notEqual, 1);
AddMethod(">", RexxInteger::isGreaterThan, 1);
AddMethod("<", RexxInteger::isLessThan, 1);
AddMethod(">=", RexxInteger::isGreaterOrEqual, 1);
AddMethod("\\<", RexxInteger::isGreaterOrEqual, 1);
AddMethod("<=", RexxInteger::isLessOrEqual, 1);
AddMethod("\\>", RexxInteger::isLessOrEqual, 1);
AddMethod("==", RexxInteger::strictEqual, 1);
AddMethod("HashCode", RexxInteger::hashCode, 0);
AddMethod("\\==", RexxInteger::strictNotEqual, 1);
AddMethod(">>", RexxInteger::strictGreaterThan, 1);
AddMethod("<<", RexxInteger::strictLessThan, 1);
AddMethod(">>=", RexxInteger::strictGreaterOrEqual, 1);
AddMethod("\\<<", RexxInteger::strictGreaterOrEqual, 1);
AddMethod("<<=", RexxInteger::strictLessOrEqual, 1);
AddMethod("\\>>", RexxInteger::strictLessOrEqual, 1);
AddMethod("MakeString", RexxObject::makeStringRexx, 0);
AddMethod("Format", RexxInteger::format, 4);
AddMethod("Trunc", RexxInteger::trunc, 1);
AddMethod("modulo", RexxInteger::modulo, 1);
AddMethod("Floor", RexxInteger::floor, 0);
AddMethod("Ceiling", RexxInteger::ceiling, 0);
AddMethod("Round", RexxInteger::round, 0);
AddMethod("Class", RexxInteger::classObject, 0);

CompleteMethodDefinitions();

CompleteClassDefinition(Integer);

EndSpecialClassDefinition(Integer);


/***************************************************************************/
/*             NUMBERSTRING                                                */
/***************************************************************************/

StartClassDefinition(NumberString)

// NOTE that we are pointing the new method at the String version...part of the
// fakeout that the NumberString class does.
AddClassMethod("New", RexxString::newRexx, A_COUNT);

CompleteClassMethodDefinitions();

AddMethod("Abs", NumberString::abs, 0);
AddMethod("Max", NumberString::Max, A_COUNT);
AddMethod("Min", NumberString::Min, A_COUNT);
AddMethod("Sign", NumberString::Sign, 0);
AddMethod("D2C", NumberString::d2c, 1);
AddMethod("D2X", NumberString::d2x, 1);
AddMethod("Equal", NumberString::equal, 1);
AddMethod("=", NumberString::equal, 1);
AddMethod("\\=", NumberString::notEqual, 1);
AddMethod("<>", NumberString::notEqual, 1);
AddMethod("><", NumberString::notEqual, 1);
AddMethod(">", NumberString::isGreaterThan, 1);
AddMethod("<", NumberString::isLessThan, 1);
AddMethod(">=", NumberString::isGreaterOrEqual, 1);
AddMethod("\\<", NumberString::isGreaterOrEqual, 1);
AddMethod("<=", NumberString::isLessOrEqual, 1);
AddMethod("\\>", NumberString::isLessOrEqual, 1);
AddMethod("==", NumberString::strictEqual, 1);
AddMethod("HashCode", NumberString::hashCode, 0);
AddMethod("\\==", NumberString::strictNotEqual, 1);
AddMethod(">>", NumberString::strictGreaterThan, 1);
AddMethod("<<", NumberString::strictLessThan, 1);
AddMethod(">>=", NumberString::strictGreaterOrEqual, 1);
AddMethod("\\<<", NumberString::strictGreaterOrEqual, 1);
AddMethod("<<=", NumberString::strictLessOrEqual, 1);
AddMethod("\\>>", NumberString::strictLessOrEqual, 1);
AddMethod("+", NumberString::plus, 1);
AddMethod("-", NumberString::minus, 1);
AddMethod("*", NumberString::multiply, 1);
AddMethod("**", NumberString::power, 1);
AddMethod("/", NumberString::divide, 1);
AddMethod("%", NumberString::integerDivide, 1);
AddMethod("//", NumberString::remainder, 1);
AddMethod("\\", NumberString::notOp, 0);
AddMethod("&", NumberString::andOp, 1);
AddMethod("|", NumberString::orOp, 1);
AddMethod("&&", NumberString::xorOp, 1);
AddMethod("MakeString", RexxObject::makeStringRexx, 0);
AddMethod("Format", NumberString::formatRexx, 4);
AddMethod("Trunc", NumberString::trunc, 1);
AddMethod("modulo", NumberString::modulo, 1);
AddMethod("Floor", NumberString::floor, 0);
AddMethod("Ceiling", NumberString::ceiling, 0);
AddMethod("Round", NumberString::round, 0);
AddMethod("Class", NumberString::classObject, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(NumberString);

EndSpecialClassDefinition(NumberString);


    /***************************************************************************/
    /*           SUPPLIER                                                      */
    /***************************************************************************/

StartClassDefinition(Supplier)

        AddClassMethod("New", SupplierClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Available", SupplierClass::available, 0);
        AddMethod("Index", SupplierClass::index, 0);
        AddMethod("Next", SupplierClass::next, 0);
        AddMethod("Item", SupplierClass::item, 0);
        AddMethod("Init", SupplierClass::initRexx, 2);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Supplier);

EndClassDefinition(Supplier);


    /***************************************************************************/
    /*           POINTER                                                       */
    /***************************************************************************/

StartClassDefinition(Pointer)

        AddClassMethod("New", PointerClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("=", PointerClass::equal, 1);
        AddMethod("\\=", PointerClass::notEqual, 1);
        AddMethod("==", PointerClass::equal, 1);
        AddMethod("\\==", PointerClass::notEqual, 1);
        AddMethod("IsNull", PointerClass::isNull, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(Pointer);

EndClassDefinition(Pointer);


    /***************************************************************************/
    /*           BUFFER                                                        */
    /***************************************************************************/

StartClassDefinition(Buffer)

        AddClassMethod("New", BufferClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

    // NO instance methods on buffer

    CompleteMethodDefinitions();

    CompleteClassDefinition(Buffer);

EndClassDefinition(Buffer);


    /***************************************************************************/
    /*           WEAKREFERENCE                                                 */
    /***************************************************************************/

StartClassDefinition(WeakReference)

        AddClassMethod("New", WeakReference::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Value", WeakReference::value, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(WeakReference);

EndClassDefinition(WeakReference);


    /***************************************************************************/
    /*           STACKFRAME                                                    */
    /***************************************************************************/

StartClassDefinition(StackFrame)

        AddClassMethod("New", StackFrameClass::newRexx, A_COUNT);

    CompleteClassMethodDefinitions();

        AddMethod("Name", StackFrameClass::getName, 0);
        AddMethod("Executable", StackFrameClass::getExecutable, 0);
        AddMethod("Line", StackFrameClass::getLine, 0);
        AddMethod("Target", StackFrameClass::getTarget, 0);
        AddMethod("TraceLine", StackFrameClass::getTraceLine, 0);
        AddMethod("Type", StackFrameClass::getType, 0);
        AddMethod("Arguments", StackFrameClass::getArguments, 0);
        // the string method just maps to TRACELINE
        AddMethod("String", StackFrameClass::getTraceLine, 0);
        AddMethod("MakeString", StackFrameClass::getTraceLine, 0);

    CompleteMethodDefinitions();

    CompleteClassDefinition(StackFrame);

EndClassDefinition(StackFrame);

    /***************************************************************************/
    /***************************************************************************/
    /***************************************************************************/

    // now add entries to the environment
    addToEnvironment("ENVIRONMENT", TheEnvironment);
    addToEnvironment("NIL" ,TheNilObject);
    addToEnvironment("FALSE", TheFalseObject);
    addToEnvironment("TRUE", TheTrueObject);

    // create and add a RexxInfo object to the environment
    RexxInfo *info = new RexxInfo;
    addToEnvironment("REXXINFO", info);
    // now create the cached objects in the info object.
    info->initialize();
    // and add the ENDOFLINE entry using the RexxInfo value
    addToEnvironment("ENDOFLINE", info->getFileEndOfLine());


    // set up the kernel directory
    addToSystem("INTEGER", TheIntegerClass);
    addToSystem("NUMBERSTRING", TheNumberStringClass);

    addToSystem("NULLARRAY", TheNullArray);
    addToSystem("NULLPOINTER", TheNullPointer);
    addToSystem("COMMONRETRIEVERS", TheCommonRetrievers);
    addToSystem("ENVIRONMENT", TheEnvironment);

    addToSystem("VERSION", Interpreter::getVersionString());
    // initialize our thread vector for external calls.
    Activity::initializeThreadContext();

    // define and suppress methods in the nil object
    TheNilObject->defineInstanceMethod(getGlobalName("COPY"), (MethodClass *)TheNilObject, OREF_NULL);
    TheNilObject->defineInstanceMethod(getGlobalName("START"), (MethodClass *)TheNilObject, OREF_NULL);
    TheNilObject->defineInstanceMethod(getGlobalName("OBJECTNAME="), (MethodClass *)TheNilObject, OREF_NULL);
    TheNilObject->objectNameEquals(new_string("The NIL object"));

    // ok, .NIL has been constructed.  As a last step before saving the image, we need to change
    // the type identifier in the behaviour so that this will get the correct virtual function table
    // restored when the image reloads.
    TheNilObject->behaviour->setClassType(T_NilObject);

/******************************************************************************/
/*      Complete the image build process, calling BaseClasses to establish    */
/*      the rest of the REXX image.                                           */
/******************************************************************************/

    // set up the kernel methods that will be defined on OBJECT classes in
    // CoreClasses.orx
    {
            // create a method used to retrieve the .Local environment.  We set this on the
            // .Environment directory.
            Protected<MethodClass> localMethod = new MethodClass(getGlobalName("LOCAL"), CPPCode::resolveExportedMethod("Local", CPPM(ActivityManager::getLocalRexx), 0, "ActivityManager::getLocalRexx"));

            // add this to the environment directory.
            TheEnvironment->setMethodRexx(getGlobalName("LOCAL"), localMethod);

            // CoreClasses contains additional classes written in Rexx and enhances some of the
            // base classes with methods written in Rexx.
            RexxString *symb = getGlobalName(BASEIMAGELOAD);
            RexxString *programName = ActivityManager::currentActivity->resolveProgramName(symb, OREF_NULL, OREF_NULL, RESOLVE_DEFAULT);
            // create a new stack frame to run under
            ActivityManager::currentActivity->createNewActivationStack();
            try
            {
                    // create an executable object for this.
                    Protected<RoutineClass> loader = LanguageParser::createProgram(programName);

                    // we pass the internal Rexx package as an argument to the setup program.
                    RexxObject *args = TheRexxPackage;
                    ProtectedObject result;
                    // now create the core program objects.
                    loader->runProgram(ActivityManager::currentActivity, GlobalNames::PROGRAM, OREF_NULL, (RexxObject **)&args, 1, result);
            }
            catch (ActivityException)
            {
                    ActivityManager::currentActivity->error();          /* do error cleanup                  */
                    Interpreter::logicError("Error building kernel image.  Image not saved.");
            }

    }

    // remove the special setup methods we created for the image build.
    TheClassClass->removeSetupMethods();

    // now save the image
    memoryObject.saveImage(imageTarget);
    ActivityManager::returnActivity(ActivityManager::currentActivity);
    exit(0);                         // successful build
}

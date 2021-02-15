/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                RexxCore.h      */
/*                                                                            */
/* Global Declarations                                                        */
/******************************************************************************/

/******************************************************************************/
/* Globally required include files                                            */
/******************************************************************************/
#ifndef RexxCore_INCLUDED
#define RexxCore_INCLUDED

#include "oorexxapi.h"                 // this is the core to everything

// base definition of a null object pointer
// NOTE:  This needs to remain untyped because of differences between
// RexxObject and RexxInternalObject.
#define OREF_NULL NULL

// platform customizations
#include "PlatformDefinitions.h"       // include platform definitions first

// Object Reference Assignment
// OrefSet handles reference assignment for situations where an
// object exists in the oldspace (rexx image) area and the fields is being updated
// to point to an object in the normal Rexx heap.  Since oldspace objects do
// not participate in the mark-and-sweep operation, we need to keep track of these
// references in a special table.
//
// OrefSet (or the setField() shorter version) needs to be used to set values in any object that
// a) might be part of the saved imaged (transient objects like the LanguageParser, RexxActivation,
// and Activity are examples of classes that are not...any class that is visible to the Rexx programmer
// are classes that will be part of the image, as well as any of the instruction/expresson objects
// created by the LanguageParser).  Note that as a general rule, fields that are set in an object's constructor
// do not need this...the object, by definition, is being newly created and cannot be part of the saved image.
// Other notible exceptions are the instruction/expression objects.  These object, once created, are immutable.
// Therefore, any fields that are set in these objects can only occur while a program is getting translated.  Once
// the translation is complete, all of the references are set and these can be safely included in the image
// without needing to worry about oldspace issues.  If you are uncertain how a given set should be happen,
// use OrefSet().  It is never an error to use in places where it is not required, but it certainly can be an
// error to use in places where it is required.

#define OrefSet(o,r,v) \
{                      \
    if (o->isOldSpace()) \
    {                    \
        memoryObject.setOref(r, v); \
    }                               \
    r = v;                          \
}

// short cut version of OrefSet().  99% of the uses specify this as the object pointer...this version
// saves a little typing :-)
#define setField(r, v)  OrefSet(this, this->r, v)
// OK, I'm lazy...a version of this for nulling out a field.
#define clearField(r) setField(r, OREF_NULL)

// cleaner version for setting in another object
#define setOtherField(o, r, v) OrefSet(o, o->r, v)

// forward declaration of commonly used classes
class ExpressionStack;
class RexxActivation;
class RexxObject;
class RexxClass;
class DirectoryClass;
class RexxIntegerClass;
class ArrayClass;
class MemoryObject;
class RexxString;

/******************************************************************************/
/* Global Objects - General                                                   */
/******************************************************************************/
// this one is special, and is truly global.
extern MemoryObject memoryObject;

// Guide to adding new classes to ooRexx
// 1) Determine the type of class. We have three categories,
// a) exported classes (classes visible to the Rexx programmer), b)
// external classes (classes that are not directly visible to the
// Rexx programmer, but can end up in a saved image or in a program
// compiled by rexxc, and c) transient classes that never get saved, but
// manage the run time state of the program. All classes must be defined
// in the file PrimitiveClasses.xml and the .cpp files must also be
// added to CMakeList.txt.
//
// Some requirements for each of the classes.
//
// a) exported classes.
//   - are subclasses of RexxObject
//   - implement a flatten() method so that they can be saved.
//   - the class must implement a createInstance() static method
//   - the class must define a classInstance static variable that
//     will hold a reference to the class object for the type.
//   - All methods visible to the Rexx programmer are defined in Setup.cpp
//   - All assignments to instance variables within the class after
//     initial creation must use setField() to handle old-to-new memory
//     references.
//
// b) internal classes
//   - are subclasses of RexxInternalObject
//   - also implement a flatten method
//   - do not have an associated class object or external methods.
//   - All assignments to instance variables within the class after
//     initial creation must use setField() to handle old-to-new memory
//     references.
//
// c) transient classes
//   - are also subclasses of RexxInternalObject
//   - do not require a flatten method.
//   - Since transient classes are never included in the saved
//     image, they do not need to use setField() for instance variable
//     assignments.

// Steps to adding a new class
// 1) Create the .hpp file and .cpp file. This does not need to be
// one-to-one. An hpp file can define multiple classes, but remember that
// each class you defined will require the additional steps.
//
// 2) Add the class definition to the appropriate section of
// PrimitiveClasses.xml. The build uses this information to generate
// header files and .cpp files that are part of the build.
//
// 3) Add the new .cpp files to CMakeLists.txt
//
// 4) If this is an exported class,
//    a) add a define below mapping the  TheXxxxxxXxxxxxxClass to
//       the static variable holding the class object.
//    b) add a call to the createInstance() method for the new class
//       object in Setup.cpp
//    c) define the exported methods for the class in Setup.cpp.
//    d) The method mapping table in CppCode.cpp needs to be updated
//       with all new native code methods exported by the class.
//    e) Implement a newRexx method with the following signature
//       that will be the NEW method that is called to create new
//       instances:
//
//       RexxObject *newRexx(RexxObject **args, size_t argc);
//
//    f) add a RESTORE_CLASS() macro call for the class in RexxMemory::restore()
//       so that the TheXxxxxxXxxxxxxClass variable gets restored when the image
//       is loaded.

// Steps for writing a new class:
// All objects allocated from the object heap must implement
// some common features:
//
// 1) Are subclasses of either RexxInternalObject or RexxObject (only for
// exported classes.
// 2) Define a new and delete method as part of the class definition.
//
//    void  *operator new(size_t);
//    inline void  operator delete(void *) { ; }
//
// 3) The new method should look like this:
//
//    void *VariableReferenceOp::operator new(size_t size)
//    {
//        return new_object(size, T_VariableReferenceOp);
//    }
//
//    where the T_XxxxxxXxxxxxxXxxxxxx symbol is the class
//    identifier that is automatically generated from the name
//    given on PrimitiveClass.xml
//
// 4) The new class must implement live() and liveGeneral() methods
//    to interact with garbage collection. These methods look like
//    this:
//
//
//    /**
//     * Perform garbage collection on a live object.
//     *
//     * @param liveMark The current live mark.
//     */
//    void VariableReferenceOp::live(size_t liveMark)
//    {
//        memory_mark(variable);
//    }
//
//
//    /**
//     * Perform generalized live marking on an object.  This is
//     * used when mark-and-sweep processing is needed for purposes
//     * other than garbage collection.
//     *
//     * @param reason The reason for the marking call.
//     */
//    void VariableReferenceOp::liveGeneral(MarkReason reason)
//    {
//        memory_mark_general(variable);
//    }
//
//    The live methods must mark each of the object variables that
//    are references to other Rexx objects (of all types). Note that
//    each class must also mark the symbols defined by the superclass.
//    In particular, exported classes must mark the symbol "objectVariables"
//    inherited from the RexxObject class.
//
//  5) Exported (a) and internal (b) classes must implement a flatten method.
//
//
//    /**
//     * Flatten a source object.
//     *
//     * @param envelope The envelope that will hold the flattened object.
//     */
//    void VariableReferenceOp::flatten(Envelope *envelope)
//    {
//        setUpFlatten(VariableReferenceOp)
//
//        flattenRef(variable);
//
//        cleanUpFlatten
//    }
//
//    Where the name specified in the setUpFlatten() macro matches name of
//    the class.
//
//    the garbage collection methods are crucial to proper functioning of the
//    interpreter. A lot of crashes during development can be traced to failure
//    in updating the GC methods when a new field is added to the class.
//
//  6) define the special constructor for the class. This constructor is used to
//     the virtual function table needed to restore classes during unflattening
//     operations. This special constructor looks like this:
//
//    inline VariableReference(RESTORETYPE restoreType) { ; };
//

// short hand references to internal class objects.

#define TheArrayClass ArrayClass::classInstance
#define TheBagClass BagClass::classInstance
#define TheSetClass SetClass::classInstance
#define TheClassClass RexxClass::classInstance
#define TheDirectoryClass DirectoryClass::classInstance
#define TheIntegerClass RexxInteger::classInstance
#define TheListClass ListClass::classInstance
#define TheMessageClass MessageClass::classInstance
#define TheMethodClass MethodClass::classInstance
#define TheRoutineClass RoutineClass::classInstance
#define ThePackageClass PackageClass::classInstance
#define TheRexxContextClass RexxContext::classInstance
#define TheNumberStringClass NumberString::classInstance
#define TheObjectClass RexxObject::classInstance
#define TheQueueClass QueueClass::classInstance
#define TheStemClass StemClass::classInstance
#define TheStringClass RexxString::classInstance
#define TheMutableBufferClass MutableBuffer::classInstance
#define TheSupplierClass SupplierClass::classInstance
#define TheTableClass TableClass::classInstance
#define TheStringTableClass StringTable::classInstance
#define TheIdentityTableClass IdentityTable::classInstance
#define TheRelationClass RelationClass::classInstance
#define ThePointerClass PointerClass::classInstance
#define TheBufferClass BufferClass::classInstance
#define TheWeakReferenceClass WeakReference::classInstance
#define TheStackFrameClass StackFrameClass::classInstance
#define TheRexxInfoClass RexxInfo::classInstance
#define TheVariableReferenceClass VariableReference::classInstance
#define TheEventSemaphoreClass EventSemaphoreClass::classInstance
#define TheMutexSemaphoreClass MutexSemaphoreClass::classInstance

// shorthand access to some important objects.
#define TheEnvironment memoryObject.environment
#define TheCommonRetrievers memoryObject.commonRetrievers
#define TheSystem memoryObject.system
#define TheRexxPackage memoryObject.rexxPackage

#define TheNilObject RexxNilObject::nilObject

#define TheNullArray ArrayClass::nullArray

#define TheFalseObject RexxInteger::falseObject
#define TheTrueObject RexxInteger::trueObject
#define TheNullPointer PointerClass::nullPointer

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

// class type identifiers
#include "ClassTypeCodes.h"

/******************************************************************************/
/* Global Objects - Names                                                     */
/******************************************************************************/
#include "ObjectClass.hpp"               // get real definition of Object

#include "TableClass.hpp"                // memory has inline methods to these
#include "RexxMemory.hpp"                // memory next, to get OrefSet
#include "RexxBehaviour.hpp"             // now behaviours and
#include "ClassClass.hpp"                // classes, which everything needs
#include "Envelope.hpp"                  // envelope is needed for flattens

#endif

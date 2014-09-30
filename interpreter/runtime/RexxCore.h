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

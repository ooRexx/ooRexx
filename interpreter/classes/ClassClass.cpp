/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* Primitive Class Class                                                      */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "ListClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ClassClass.hpp"
#include "MethodClass.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "WeakReferenceClass.hpp"
#include "PackageClass.hpp"
#include "MethodArguments.hpp"
#include "MethodDictionary.hpp"
#include "StringTableClass.hpp"
#include "PackageClass.hpp"
#include "NumberStringClass.hpp"
#include <new>


// singleton class instance
RexxClass *RexxClass::classInstance = OREF_NULL;


/**
 * Allocate new memory for a class object.
 *
 * @param size   the size of the class object.
 *
 * @return Storage for creating a new class object.
 */
void  *RexxClass::operator new(size_t size)
{
    return new_object(size, T_Class);
}


/**
 * Initialize a new class instance.
 *
 * @param className The id of class
 * @param classBehaviour
 *                  The class behaviour pointer.
 * @param instanceBehaviour
 *                  The class instance behaviour pointer.
 */
RexxClass::RexxClass(const char *className, RexxBehaviour *classBehaviour,
    RexxBehaviour *_instanceBehaviour)
{
    id = new_string(className);
    setBehaviour(classBehaviour);
    behaviour->setOwningClass(this);
    instanceBehaviour = _instanceBehaviour;
    instanceBehaviour->setOwningClass(this);
    // class objects need to be proxied
    makeProxiedObject();
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxClass::live(size_t liveMark)
{
    memory_mark(objectVariables);
    memory_mark(id);
    memory_mark(classMethodDictionary);
    memory_mark(instanceBehaviour);
    memory_mark(instanceMethodDictionary);
    memory_mark(baseClass);
    memory_mark(metaClass);
    memory_mark(superClass);
    memory_mark(superClasses);
    memory_mark(subClasses);
    memory_mark(package);
    memory_mark(annotations);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxClass::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
        // this class can no longer be altered
        setRexxDefined();
    }

    memory_mark_general(objectVariables);
    memory_mark_general(id);
    memory_mark_general(classMethodDictionary);
    memory_mark_general(instanceBehaviour);
    memory_mark_general(instanceMethodDictionary);
    memory_mark_general(baseClass);
    memory_mark_general(metaClass);
    memory_mark_general(superClass);
    memory_mark_general(superClasses);
    memory_mark_general(subClasses);
    memory_mark_general(package);
    memory_mark_general(annotations);
}


/**
 * An override for the copy method to keep Class objects from
 * being copied.
 *
 * @return Never returns.
 */
RexxObject *RexxClass::copyRexx()
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_copy_method, this);
    return TheNilObject;
}


/**
 * Make a proxy object from a class.
 *
 * @param envelope The envelope we're flattening into.
 *
 * @return A string proxy name for this object.
 */
RexxObject *RexxClass::makeProxy(Envelope *envelope)
{
    return new_proxy(id->getStringData());
}


/**
 * Hash a class object.  Because behaviors don't always get set
 * up properly with this, we'll always use the primitive one for
 * class objects.  We want to ensure that this is not overridden
 * on us.
 *
 * @return A "hashed hash" that can be used by the map collections.
 */
HashCode RexxClass::hash()
{
    // always, always, always return the hash value, which will be the
    // hash value of our id string.  This is important, since we need to
    // have a hash value that will be the same before and after the image save
    return getHashValue();
}


/**
 * Get the primitive hash value of this String object.
 *
 * @return The calculated string hash for the string.
 */
HashCode RexxClass::getHashValue()
{
    // always, always, always return the hash value, which will be the
    // hash value of our id string.  This is important, since we need to
    // have a hash value that will be the same before and after the image save
    return id->getHashValue();
}


/**
 * Compare two classes for equality.
 *
 * @param other  The other class to compare.
 *
 * @return The True object if they are equal, the False object
 *         otherwise.
 */
RexxObject *RexxClass::strictEqual(RexxObject *other)
{
    // a direct equality comparison
    return equal(other);
}


/**
 * Compare two classes
 *
 * @param other  The other class to compare.
 *
 * @return The True object if they are equal, the False object if not.
 */
RexxObject *RexxClass::equal(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);

    // We have two "pseudo classes" (Integer and NumberString) that run around
    // pretending to be the String class.  We mask this here by making all of them
    // equal.
    if ((this == TheStringClass || this == TheIntegerClass || this == TheNumberStringClass) &&
        (other == (RexxObject *)TheStringClass || other == (RexxObject *)TheIntegerClass || other == (RexxObject *)TheNumberStringClass))
    {
        return TheTrueObject;
    }
    else
    {
        // this is a direct identity compare
        return booleanObject(this == other);
    }
}


/**
 * Inequality comparison between two class objects.
 *
 * @param other  The other class for the comparison.
 *
 * @return The True object if they are not equal, the False
 *         object otherwise.
 */
RexxObject *RexxClass::notEqual(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);

    // We have two "pseudo classes" (Integer and NumberString) that run around
    // pretending to be the String class.  We mask this here by making all of them
    // equal.
    if ((this == TheStringClass || this == TheIntegerClass || this == TheNumberStringClass) &&
        (other == (RexxObject *)TheStringClass || other == (RexxObject *)TheIntegerClass || other == (RexxObject *)TheNumberStringClass))
    {
        return TheFalseObject;
    }
    else
    {
        return booleanObject(this != other);
    }
}


/**
 * Test if a class is defined as a MIXIN class.
 *
 * @return True if this is a mixin, False if not.
 */
RexxObject *RexxClass::queryMixinClass()
{
    return booleanObject(isMixinClass());
}


/**
 * Test if a class can be used as a metaclass
 *
 * @return True if this is a metaclass, False if not.
 */
RexxObject *RexxClass::isMetaClassRexx()
{
    return booleanObject(isMetaClass());
}


/**
 * Test if this class is marked as abstract
 *
 * @return True if this is abstract, false if not
 */
RexxObject *RexxClass::isAbstractRexx()
{
    return booleanObject(isAbstract());
}


/**
 * Retrieve the read/write annotation table for a class object.
 *
 * @return The class annotations.  This might be an empty string table.
 */
StringTable *RexxClass::getAnnotations()
{
    // this is a user-modifiable table.  If we have no
    // table created, then add one to this package.
    if (annotations == OREF_NULL)
    {
        setField(annotations, new_string_table());
    }

    return annotations;
}


/**
 * Set the annotations on an installed class object.
 *
 * @param a      The annotations table (used by the ClassDirective object)
 */
void RexxClass::setAnnotations(StringTable *a)
{
    // just set the table
    setField(annotations, a);
}


/**
 * Get a specific named annotation.
 *
 * @param name   The annotation name
 *
 * @return The annotation value, or OREF_NULL if it doesn't exist.
 */
RexxString *RexxClass::getAnnotation(RexxString *name)
{
    if (annotations == OREF_NULL)
    {
        return OREF_NULL;
    }
    return (RexxString *)annotations->entry(name);
}


/**
 * The Rexx stub for the get annotation method
 *
 * @param name   The name of the target annotation.
 *
 * @return The annotation value, or .nil if it does not exist.
 */
RexxObject *RexxClass::getAnnotationRexx(RexxObject *name)
{
    return resultOrNil(getAnnotation(stringArgument(name, "name")));
}


/**
 * Retrieve the String ID for a class object.
 *
 * @return The string ID.
 */
RexxString *RexxClass::getId()
{
    return id;
}


/**
 * Set a class as a Rexx defined, non-alterable class.
 *
 * @return Dummy OREF_NULL.
 */
RexxObject *RexxClass::setRexxDefined()
{
    classFlags.set(REXX_DEFINED);
    return OREF_NULL;
}


/**
 * Return the base class for this class.
 *
 * @return The baseclass (the lowest non-mixin class).
 */
RexxClass *RexxClass::getBaseClass()
{
    return baseClass;
}


/**
 * Get the MetaClass for the class object.
 *
 * @return The class metaclass.
 */
RexxClass *RexxClass::getMetaClass()
{
    return metaClass;
}


/**
 * Set the instance behaviour of this class object.
 *
 * @param b      the new instance behaviour.
 */
void  RexxClass::setInstanceBehaviour(RexxBehaviour *b)
{
    setField(instanceBehaviour, b);
}


/**
 * Return the immediate superclass of this class.
 *
 * @return The first superclass of the class.
 */
RexxClass *RexxClass::getSuperClass()
{
    // object has no superclasses
    if (this == TheObjectClass)
    {
        return (RexxClass *)TheNilObject;
    }
    // get the first item from the immediate list.
    return (RexxClass *)superClasses->getFirstItem();
}


/**
 * return an array of the class superclasses.
 *
 * @return An array containing all of the superclasses.
 */
ArrayClass *RexxClass::getSuperClasses()
{
    // return a copy so it can't be modified.
    return (ArrayClass *)superClasses->copy();
}


/**
 * Get a list of all current subclasses of this class.
 *
 * @return An array of the subclasses.
 */
ArrayClass *RexxClass::getSubClasses()
{
    // remove any gc classes from the list now, and return the array
    return subClasses->weakReferenceArray();
}


/**
 * Add a subclass to the list of subclasses.
 *
 * @param subClass The new subclass.
 */
void RexxClass::addSubClass(RexxClass *subClass)
{
    // wrap a weak reference around the subclass
    WeakReference *ref = new WeakReference(subClass);
    // add this to the front of the subclass list
    subClasses->addFirst(ref);
}


/**
 * Add a full table of methods to a class definition.  This is
 * only used during the initial image build.
 *
 * @param newMethods The new methods to add.
 */
RexxObject *RexxClass::defineMethods(StringTable *newMethods)
{
    // add these to the instance method dictionary we use to
    // build the behaviour.
    instanceMethodDictionary->addMethods(newMethods, this);

    // now update the instance behaviour from the superclass list
    instanceBehaviour->setMethodDictionary(OREF_NULL);
    createInstanceBehaviour(instanceBehaviour);

    // see if we have an uninit method defined now that this is done
    checkUninit();
    return OREF_NULL;
}


/**
 * Add a full table of methods to a class definition.
 *
 * @param newMethods The new methods to add.
 */
RexxObject *RexxClass::defineMethodsRexx(RexxObject *newMethods)
{
    // Rexx defined classes are not allowed to be update.  We report this as
    // a NOMETHOD problem, as if the define method did not even exist.
    if ( isRexxDefined())
    {
        reportException(Error_Execution_rexx_defined_class);
    }

    requiredArgument(newMethods, "methods");
    // create a method dictionary and merge this into the method dictionary
    Protected<MethodDictionary> enhancing_methods = createMethodDictionary(newMethods, this);

    // make a copy of the instance behaviour so any previous objects
    // aren't enhanced
    setField(instanceBehaviour, (RexxBehaviour *)instanceBehaviour->copy());

    // replace all of the methods in the method dictionary
    instanceMethodDictionary->replaceMethods(enhancing_methods, this);

    // any subclasses that we have need to redo their instance behaviour
    // this also updates our own behaviour table
    updateInstanceSubClasses();

    // see if we have an uninit method defined now that this is done
    checkUninit();
    return OREF_NULL;
}


/**
 * Inherit the instance methods from another class definition.
 * This is not an true inherit operation where the class is part
 * of the hierarchy.  This directly grabs the instance methods
 * defined directly by the other class and merges them into the
 * class method dictionary.  This is a special method that is
 * only done during the initial image build.
 *
 * @param newMethods The new methods to add.
 */
void RexxClass::inheritInstanceMethods(RexxClass *source)
{
    MethodDictionary *sourceMethods = source->instanceMethodDictionary;

    // we want all methods to be "inherited" with this scope
    sourceMethods->setMethodScope(this);

    // loop through the table with an iterator.
    HashContents::TableIterator iterator = sourceMethods->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        // get the name and the value, then add to this class object
        RexxString *methodName = (RexxString *)iterator.index();
        // if this is the Nil object, that's an override.  Make it OREF_NULL.
        MethodClass *method = (MethodClass *)iterator.value();
        // we just add this to the instance method dictionary for when we
        // build the behaviour
        instanceMethodDictionary->addMethod(methodName, method);
    }

    // now update the instance behaviour from our superclasses and
    // merge in the new methods.
    instanceBehaviour->setMethodDictionary(OREF_NULL);
    createInstanceBehaviour(instanceBehaviour);

    // see if we have an uninit method defined now that this is done
    checkUninit();
}


/**
 * Inherit the instance methods from another class definition.
 * This is not an true inherit operation where the class is part
 * of the hierarchy.  This directly grabs the instance methods
 * defined directly by the other class and merges them into the
 * class method dictionary.  This is a special method that is
 * only done during the initial image build.
 *
 * @param newMethods The new methods to add.
 */
RexxObject *RexxClass::inheritInstanceMethodsRexx(RexxClass *source)
{
    // add the methods to the instance method dictionary
    inheritInstanceMethods(source);

    return OREF_NULL;
}


/**
 * Retrieve a class default name value...this is composed
 * from the class id value.
 *
 * @return The default object name.
 */
RexxString *RexxClass::defaultName()
{
    // start with the id...
    RexxString *defaultname = id;
    // prefix with "The"
    defaultname = defaultname->concatToCstring("The ");
    // add on "class"
    defaultname = defaultname->concatWithCstring(" class");
    return defaultname;
}


/**
 * Get the method dictionary for the instance behaviour.
 *
 *
 * @return The instance behaviour dictionary for the class.
 */
MethodDictionary *RexxClass::getInstanceBehaviourDictionary()
{
    // always return a copy of the dictionary
    return instanceBehaviour->copyMethodDictionary();
}



/**
 * Get a copy of the method dictionary from the class behaviour.
 *
 * @return A copy of the behaviour method dictionary.
 */
MethodDictionary *RexxClass::getBehaviourDictionary()
{
    return behaviour->copyMethodDictionary();
}


/**
 * Initialize a base Rexx class.
 */
void RexxClass::buildFinalClassBehaviour()
{
    // get a copy of the class instance
    // behaviour mdict before the merge
    // with OBJECT.  This unmerged mdict
    // is kept in this class's
    // class_instance_mdict field.
    instanceMethodDictionary = getInstanceBehaviourDictionary();
    // now clear the instance behaviour and start building anew
    instanceBehaviour->clearMethodDictionary();
    // Add OBJECT to the behaviour scope table
    instanceBehaviour->addScope(TheObjectClass);
    // if this is not the object class, then we need to add object
    // to the different behaviours before we merge in our methods
    if (this != TheObjectClass)
    {
        // add to the instance behaviour scope
        instanceBehaviour->addScope(TheObjectClass);
        // merge the class instance behaviour with object.
        instanceBehaviour->merge(TheObjectBehaviour);
    }

    // now add in the methods defined for this class
    instanceBehaviour->mergeMethodDictionary(instanceMethodDictionary);

    // add this class to the scope table
    instanceBehaviour->addScope(this);
    // now we do the same thing with the class behaviour
    classMethodDictionary = getBehaviourDictionary();
    // now add the scope levels to this class behaviour
    // If this isn't OBJECT put OBJECT in first
    if (this != TheObjectClass)
    {
        behaviour->addScope(TheObjectClass);
    }
    // if this is OBJECT - merge the object instance
    // methods with the object class methods
    else
    {
        behaviour->merge(TheObjectBehaviour);
        // and put them into the class mdict
        // so all the classes will inherit
        classMethodDictionary = getBehaviourDictionary();
    }
    // The merge of the mdict's is order specific. By processing OBJECT
    // first then CLASS and then the rest of the subclassable classes
    // the mdict's will be set up correctly.
    behaviour->merge(TheClassBehaviour);
    // if this isn't CLASS put CLASS in next
    if (this != TheClassClass)
    {
        behaviour->addScope(TheClassClass);
    }
    // now add this class to the scope
    behaviour->addScope(this);

    // that's the behaviour information...now fill in other state data.

    // All primitive methods have TheClassClass as the meta class.
    metaClass = TheClassClass;

    // The Baseclass for non-mixin classes is self
    baseClass = this;
    // as is the instance superclasses list
    superClasses = new_array();
    // create the subclasses list
    subClasses = new_list();
    // is this is not the object classs, we have superclass information to add
    if (this != TheObjectClass)
    {
        // The instance superclasses for all except OBJECT is OBJECT
        superClasses->addLast(TheObjectClass);
        // and for OBJECT we need to add all the other classes as
        // subclasses except for the pseudo classes of integer and number string.
        if (this != TheIntegerClass && this != TheNumberStringClass)
        {
            TheObjectClass->addSubClass(this);
        }
    }
    // and point the instance behaviour back to this class
    instanceBehaviour->setOwningClass(this);
    // and the class behaviour to CLASS
    behaviour->setOwningClass(TheClassClass);
    // these are primitive classes
    setPrimitive();

    // check to see if we have an uninit methods.
    checkUninit();

    // if this is the CLASS class, make it a meta class.
    if (this == TheClassClass)
    {
        setMetaClass();
    }
}


/**
 * Initialize a base Rexx class that inherits from a primitive
 * class other than Object.
 *
 * @param superClass The immediate superclass of the created
 *                   class.
 */
void RexxClass::buildFinalClassBehaviour(RexxClass *superClass)
{
    // get a copy of the class instance behaviour mdict before the merge
    // with OBJECT.  This unmerged mdict is kept in this class's
    // class_instance_mdict field.
    setField(instanceMethodDictionary, getInstanceBehaviourDictionary());

    // set up the superclass/subclass relationships
    setField(superClasses, new_array(superClass));
    // create the subclasses list
    setField(subClasses, new_list());
    // and add this as a subclass to our superclass
    superClass->addSubClass(this);

    // create the merged method dictionary for the instancebehavior
    // and update all of the scopes.
    mergeBehaviour(instanceBehaviour);

    // get a copy of the class behaviour mdict before the merge with the
    // CLASS instance behaviour. This unmerged mdict is kept in the
    // class_mdict field
    setField(classMethodDictionary, getBehaviourDictionary());
    // The merge of the mdict's is order specific. By processing OBJECT
    // first then CLASS and then the rest of the subclassable classes
    // the mdict's will be set up correctly.In this way merging the CLASS
    // behaviour will only be the CLASS instance methods when OBJECT is
    // processed, but will be CLASS's and OBJECT's after CLASS is
    // processed                          */
    behaviour->merge(TheClassBehaviour);
    // now add the scope levels to this class behaviour
    behaviour->addScope(TheObjectClass);
    // add the class scope levels
    behaviour->addScope(TheClassClass);
    // and finally the new class.
    behaviour->addScope(this);

    // now fill in some state data for the class object.
    // set up the new metaclass list
    setField(metaClass, TheClassClass);

    // The Baseclass for non-mixin classes is self
    baseClass = this;
    // and point the instance behaviour back to this class
    instanceBehaviour->setOwningClass(this);
    // and the class behaviour to CLASS
    behaviour->setOwningClass(TheClassClass);
    // these are primitive classes
    classFlags.set(PRIMITIVE_CLASS);
}


/**
 * Add a new instance method to this class object.
 *
 * @param method_name
 *               The name the method will be added under.
 * @param method_object
 *               The associated method object.
 *
 * @return Aways returns nothing.
 */
RexxObject *RexxClass::defineMethod(RexxString *method_name, RexxObject *methodSource)
{
    // Rexx defined classes are not allowed to be update.  We report this as
    // a NOMETHOD problem, as if the define method did not even exist.
    if ( isRexxDefined())
    {
        reportException(Error_Execution_rexx_defined_class);
    }

    // the name is required and must be a string.  We always
    // use the uppercase name for updating the method table,
    // but use the original name if we have to create a new method object.
    method_name = stringArgument(method_name, "method name");
    Protected<RexxString> dictionaryName = method_name->upper();

    Protected<MethodClass> methodObject;

    // if the second argument is omitted, then we are "hiding"
    // this method definition.  We add the method object to the
    // method dictionary as .nil, which will cause a lookup failure
    // when an attempt is made to invoke this method.
    if (OREF_NULL == methodSource)
    {
        methodObject = (MethodClass *)TheNilObject;
    }
    // We need to convert this into a method object if it is not
    // one already.  .nil is a special case (same as an omitted argument)
    else if (TheNilObject != methodSource)
    {
        methodObject = MethodClass::newMethodObject(method_name, methodSource, this, "method");
    }
    // if we have a real method object, then the scope has already been set
    // and alse check if this is an uninit method, which is a special case.
    if ((MethodClass *)TheNilObject != methodObject)
    {
        if (method_name->strCompare("UNINIT"))
        {
            setHasUninitDefined();
        }
    }

    // make a copy of the instance behaviour so any previous objects
    // aren't enhanced
    setField(instanceBehaviour, (RexxBehaviour *)instanceBehaviour->copy());
    // add method to the instance method dictionary
    instanceMethodDictionary->replaceMethod(dictionaryName, methodObject);
    // any subclasses that we have need to redo their instance behaviour
    // this also updates our own behaviour table
    updateInstanceSubClasses();
    // this is a Rexx method, so we need to have a return value.
    return OREF_NULL;
}


/**
 * special method to allow a class method to be added
 * to a primitive class during image build.
 *
 * @param method_name
 *                  The name of the new method.
 * @param newMethod The method object to add
 *
 * @return always returns OREF_NULL
 */
RexxObject *RexxClass::defineClassMethod(RexxString *method_name, MethodClass *newMethod)
{
    // validate the arguments
    Protected<RexxString> name = stringArgument(method_name, ARG_ONE)->upper();
    requiredArgument(newMethod, ARG_TWO);
    Protected<MethodClass> addedMethod = newMethod->newScope(this);
    // now add this directly to the behaviour
    behaviour->defineMethod(name, addedMethod);
    // also add to the class method dictionary
    classMethodDictionary->addMethod(name, addedMethod);
    // called as a Rexx method, so we need a return value.
    return OREF_NULL;
}


/**
 * Remove a class method from a class and all of its class methods.
 *
 * @param method_name
 *               The target method name.
 */
void RexxClass::removeClassMethod(RexxString *method_name)
{
    // remove from our behaviour
    behaviour->deleteMethod(method_name);
    instanceBehaviour->deleteMethod(method_name);

    // propagate to all subclasses
    ArrayClass *subclass_list = getSubClasses();
    for (size_t i = 1; i < subclass_list->size(); i++)
    {
        ((RexxClass *)subclass_list->get(i))->removeClassMethod(method_name);
    }
}


/**
 * Remove the special class methods that are defined just for
 * image building.
 */
void RexxClass::removeSetupMethods()
{
    RexxString *defineClassMethodName = new_string("DEFINECLASSMETHOD");
    RexxString *inheritInstanceMethodsName = new_string("INHERITINSTANCEMETHODS");

    // remove from the base class behaviour first
    behaviour->deleteMethod(defineClassMethodName);
    instanceBehaviour->deleteMethod(defineClassMethodName);
    instanceMethodDictionary->removeMethod(defineClassMethodName);

    behaviour->deleteMethod(inheritInstanceMethodsName);
    instanceBehaviour->deleteMethod(inheritInstanceMethodsName);
    instanceMethodDictionary->removeMethod(inheritInstanceMethodsName);

    // now we need to remove this from all class objects in the image.  We start with
    // object and move all the way up the hierarchy for both methods
    TheObjectClass->removeClassMethod(defineClassMethodName);
    TheObjectClass->removeClassMethod(inheritInstanceMethodsName);
}


/**
 * Delete an instance method from this class definition.
 *
 * @param method_name
 *               The target method name.
 *
 * @return Returns nothing.
 */
RexxObject *RexxClass::deleteMethod(RexxString  *method_name)
{
    // we pretend this method does not exist for rexx defined classes.
    if (isRexxDefined())
    {
        reportException(Error_Execution_rexx_defined_class);
    }

    // the method name must be a string, and we use the uppercase version...always!
    method_name = stringArgument(method_name, "method name")->upper();
    // we work on a copy of the instance behaviour so that this changed
    // does not suddenly show up in existing instances of this class.
    setField(instanceBehaviour, (RexxBehaviour *)instanceBehaviour->copy());
    // if there is a method to remove, then we need to propagate this update.
    if (instanceMethodDictionary->removeMethod(method_name))
    {
        // the dictionary changed, we need to update our behaviour and
        // propagate to our subclasses.
        updateInstanceSubClasses();
    }
    return OREF_NULL;
}


/**
 * Returns the method object associated with a given name.
 *
 * @param method_name
 *               The target method name.
 *
 * @return The method object, or .nil if the method does not exist.
 */
MethodClass *RexxClass::method(RexxString  *method_name)
{
    // make sure we have a proper name
    method_name = stringArgument(method_name, "method name")->upper();
    // we keep the instance methods defined at this level in a separate
    // method dictionary that is used to build the behaviour.  We can retrieve
    // the method directly from there.
    MethodClass *method_object = instanceMethodDictionary->getMethod(method_name);
    // this is an error if it is not in the method dictionary.
    // Note that is could be there, but as .nil.  We will return that value
    if ( OREF_NULL == method_object)
    {
        reportException(Error_No_method_name, this, method_name);
    }
    return method_object;
}


/**
 * If no qualification parameter entered
 * return all the methods that an instance of this class
 * will inherit.
 *
 * If TheNilObject  is the qualification parameter
 * return just the methods introduced at this class scope
 *
 * For any other qualification parameter
 * return just the methods introduced at that class scope
 *
 * @param class_object
 *               The class object qualifier.
 *
 * @return A supplier for iterating the requested methods.
 */
SupplierClass *RexxClass::methods(RexxClass *class_object)
{
    // if the argument is .nil, then change the scope to us.  The
    // method dictionary handles everything.
    if (class_object == TheNilObject)
    {
        class_object = this;
    }
    // the instance behaviour will generate the necessary supplier
    return instanceBehaviour->getMethods(class_object);
}


/**
 * A change to this class' definitions has occurred.  We
 * need to update the behaviour, and propagate this change
 * to all of our subclasses as well.
 */
void  RexxClass::updateSubClasses()
{
    // clear the method dictionary from our behaviour
    behaviour->clearMethodDictionary();
    // and also from our instance behaviour
    instanceBehaviour->clearMethodDictionary();

    // create a new instance behaviour
    createInstanceBehaviour(instanceBehaviour);
    // This time, we update the class behaviour
    // after building the instance behaviour
    // because the added methods may have an
    // impact on metaclasses.
    createClassBehaviour(behaviour);

    // check to see if we have an uninit method.
    checkUninit();

    // ok, we are all updated, now touch our superclasses to update as well.

    // we're updated, now nudge each of our subclasses
    // to let them know they need to update too.
    Protected<ArrayClass> subClassList = getSubClasses();
    for (size_t index = 1; index <= subClassList->size(); index++)
    {
        // each of our subclasses will do the same thing we just did
        ((RexxClass *)subClassList->get(index))->updateSubClasses();
    }
}


/**
 * Update my instance behaviour and have my subclasses
 * do the same.
 */
void RexxClass::updateInstanceSubClasses()
{
    // clear, and rebuild the instance behaviour
    instanceBehaviour->clearMethodDictionary();
    createInstanceBehaviour(instanceBehaviour);

    // see if we have an uninit method defined now that this is done
    checkUninit();

    // tell all of our subclasses to do this same step
    Protected<ArrayClass> subClassList = getSubClasses();
    for (size_t index = 1; index <= subClassList->size(); index++)
    {
        ((RexxClass *)subClassList->get(index))->updateInstanceSubClasses();
    }
}


/**
 * Create the class behaviour (the behavior for the class object
 * itself).  This merges all of the information from our
 * superclasses with the information defined at this class
 * scope.
 *
 * @param target_class_behaviour
 *               The behaviour object we're going to build into.
 */
void RexxClass::createClassBehaviour(RexxBehaviour *target_class_behaviour)
{
    // we are going to call each of our superclasses, start from the last to the
    // first asking them to merge their information.  The last superclass should be
    // Object, the first will be our immediate superclass.
    for (size_t index = superClasses->items(); index > 0; index--)
    {
        RexxClass *superclass = (RexxClass *)superClasses->get(index);
        // if there is a superclass and this hasn't been added into this
        // behaviour yet, ask it to merge it's information into this.  We
        // can have dups when mixin classes are involved, since we inherit the
        // same baseClass chain from each mixin.
        if (!target_class_behaviour->hasScope(superclass))
        {
            superclass->createClassBehaviour(target_class_behaviour);
        }
    }

    // now see if we need to merge our information into this behaviour (we likely do)
    if (!target_class_behaviour->hasScope(this))
    {
        // Object is a special case, since it is top dog.
        if (TheObjectClass != this)
        {
            // add whichever metaclasses have not been added yet
            if (!target_class_behaviour->hasScope(metaClass))
            {
                // merge in the meta class instance method dictionary into our method dictionary.
                // this also merges the scopes.
                metaClass->mergeInstanceBehaviour(target_class_behaviour);
            }
        }

        // Merge this class mdict with the target behaviour class mdict
        target_class_behaviour->mergeMethodDictionary(classMethodDictionary);

        // update the target behaviour scopes with this class, if necessary.
        target_class_behaviour->addScope(this);
    }
}


/**
 * Create the instance behaviour for a class.  The instance
 * behaviour in this context is the behaviour that the
 * class bestows upon its instances when instantiated.
 *
 * @param target_instance_behaviour
 *               The behaviour we are merging information into.
 */
void RexxClass::createInstanceBehaviour(RexxBehaviour *target_instance_behaviour)
{
    // like building the class behaviour, we process the superclasses in reverse
    // order, starting with Object, and overlay the information from each class
    // on top of the previous.
    for (size_t index = superClasses->size(); index > 0; index--)
    {
        RexxClass *superclass = (RexxClass *)superClasses->get(index);
        // it is possible for a superclass to have already been processed
        // during the recursive processes, so make sure we only do each class once.
        if (!target_instance_behaviour->hasScope(superclass))
        {
            superclass->createInstanceBehaviour(target_instance_behaviour);
        }
    }

    // and finally, our instance information.
    if (!target_instance_behaviour->hasScope(this))
    {
        // merge our information into the target
        target_instance_behaviour->mergeMethodDictionary(instanceMethodDictionary);
        // and make sure our scope is also added.
        target_instance_behaviour->addScope(this);
    }
}


/**
 * Merge the behaviours from the superclasses into a target
 * primitive class.
 *
 * @param target_instance_behaviour
 *               The target behavior to update.
 */
void RexxClass::mergeBehaviour(RexxBehaviour *target_instance_behaviour)
{
    // Call each of the superclasses in this superclass list starting from
    // the last going to the first
    for (size_t index = superClasses->size(); index > 0; index--)
    {
        RexxClass *superclass = (RexxClass *)superClasses->get(index);
        // if there is a superclass and it hasn't been added into this
        // behaviour yet, call and have it add itself                        */
        if (!target_instance_behaviour->hasScope(superclass))
        {
            superclass->mergeBehaviour(target_instance_behaviour);
        }
    }
    // now add in the scope for this class, if still needed.
    if (!target_instance_behaviour->hasScope(this))
    {
        // this merges the instance methods defined by this class.
        target_instance_behaviour->merge(instanceBehaviour);
        // and make sure the scope is updated
        target_instance_behaviour->addScope(this);
    }
}


/**
 * Check if a class definition has an UNINIT method defined.
 */
void RexxClass::checkUninit()
{
    // we have to things we need to do here.  First we check to see if there is an UNINIT
    // instance method defined, and if there is, we mark the class as creating instances
    // that need an UNINIT run so that they will get added to the special table at creation time.
    if (instanceBehaviour->methodLookup(GlobalNames::UNINIT) != OREF_NULL)
    {
        setHasUninitDefined();
    }

    // if the class object has an UNINIT method defined, make sure we
    // add this to the table of classes to be processed.
    if (hasUninitMethod())
    {
        requiresUninit();
    }
}


/**
 * Process a collection of methods that will be added to a class
 * as class methods, or will be added to an enhanced object.  In
 * either case, this is an arbitrary collection of objects that
 * may need conversion into method objects and given a scope.
 *
 * @param sourceCollection
 *               the table containing the defined method objects.  This
 *               can be any object that supports a Supplier method.
 * @param scope  The scope these method objects belong to.
 *
 * @return A method dictionary built from this collection of methods.
 */
MethodDictionary *RexxClass::createMethodDictionary(RexxObject *sourceCollection, RexxClass *scope )
{
    // get a method dictionary large enough to handle this set of methods
    Protected<MethodDictionary> newDictionary = new MethodDictionary();

    // it would be nice to just grab a table iterator, but we need to use
    // a supplier here.
    ProtectedObject p2;
    sourceCollection->sendMessage(GlobalNames::SUPPLIER, p2);
    SupplierClass *supplier = (SupplierClass *)(RexxObject *)p2;
    for (; supplier->available() == TheTrueObject; supplier->next())
    {
        MethodClass *newMethod = (MethodClass *)supplier->item();
        Protected<RexxString> method_name = supplier->index()->requestString();;
        // we add the methods to the table in uppercase, but create method objects using
        // the original name.
        Protected<RexxString> table_method_name = method_name->upper();

        // a method can be included in the table as the Nil object...this
        // hides the method of that name and is allowed.
        if (newMethod != (MethodClass *)TheNilObject)
        {
            // if this isn't a method object already, try to create one
            newMethod = MethodClass::newMethodObject(method_name, newMethod, this, "method source");
            newMethod->setScope(scope);
        }
        // now add the method to the target dictionary
        newDictionary->addMethod(table_method_name, newMethod);
    }

    return newDictionary;
}


/**
 * To add the mixin class (parameter one) to the superclass
 * hierarchy of the receiver class (this), at the last position
 * or the specified position (parameter two).
 *
 * @param mixin_class
 *                 The class to inherit from.
 * @param position The position to insert the class into the hierarchy (optional)
 *
 * @return returns nothing.
 */
RexxObject *RexxClass::inherit(RexxClass *mixin_class, RexxClass  *position)
{
    // another operation not permitted on Rexx defined classes.
    if (isRexxDefined())
    {
        reportException(Error_Execution_rexx_defined_class);
    }

    // the mixin class is required
    requiredArgument(mixin_class, "mixin class");

    // this must be a class object and must be marked as a mixin
    if (!mixin_class->isInstanceOf(TheClassClass) || !mixin_class->isMixinClass())
    {
        reportException(Error_Execution_mixinclass, mixin_class);
    }

    // make sure this is not being done recursively.
    if (this == mixin_class )
    {
        reportException(Error_Execution_recursive_inherit, this, mixin_class);
    }

    // this also cannot already be part of the existing class hierarchy
    if (behaviour->hasScope(mixin_class))
    {
        reportException(Error_Execution_recursive_inherit, this, mixin_class);
    }

    // and it can't go the other way either
    if (mixin_class->behaviour->hasScope(this))
    {
        reportException(Error_Execution_recursive_inherit, this, mixin_class);
    }

    // ok, now we need to have a common base class.
    if (!behaviour->hasScope(mixin_class->getBaseClass()))
    {
        reportException(Error_Execution_baseclass, this, mixin_class, mixin_class->getBaseClass());
    }

    // and also the instance class hiearchy (slight different because of metaclasses)
    if (!instanceBehaviour->hasScope(mixin_class->getBaseClass()))
    {
        reportException(Error_Execution_baseclass, this, mixin_class, mixin_class->getBaseClass());
    }

    // ok, a lot of work to validate this was good.  Now we need to validate
    // the position.

    // if not specified, we're adding to the end of the inheritance list (typical)
    if (position == OREF_NULL)
    {
        superClasses->addLast(mixin_class);
    }
    else
    {
        // we have an insertion position, find the target class in the superclasses
        // list and insert it after that point.
        size_t instanceIndex = superClasses->indexOf(position);

        if (instanceIndex == 0)
        {
            reportException(Error_Execution_uninherit, this, position);
        }

        superClasses->insertAfter(mixin_class, instanceIndex);
    }

    // tell the mixin class that it has a new subclass
    mixin_class->addSubClass(this);

    // now we need to to rebuild the behaviour and also
    // propagate the change to the subclasses.
    updateSubClasses();

    // If the mixin class has an uninit defined, the new class must have one, too
    if (mixin_class->hasUninitDefined() || mixin_class->parentHasUninitDefined())
    {
        setParentHasUninitDefined();
    }
    return OREF_NULL;
}

/**
 * Remove a mixin class from the class hierarchy.
 *
 * @param mixin_class
 *               The target mixin
 *
 * @return Returns nothing.
 */
RexxObject *RexxClass::uninherit(RexxClass  *mixin_class)
{
    // modifying Rexx defined classes is forbidden.
    if (isRexxDefined())
    {
        reportException(Error_Execution_rexx_defined_class);
    }

    // the target class is required
    requiredArgument(mixin_class, "mixin class");

    // this must be a class object and must be marked as a mixin
    if (!mixin_class->isInstanceOf(TheClassClass) || !mixin_class->isMixinClass())
    {
        reportException(Error_Execution_mixinclass, mixin_class);
    }

    // this class must be a superclass of this class, but not the
    // immeidate superclass.
    size_t instance_index = superClasses->indexOf(mixin_class);

    // if good for both, go ahead and remove
    if  (instance_index > 1)
    {
        superClasses->deleteItem(instance_index);
    }
    else
    {
        reportException(Error_Execution_uninherit, this, mixin_class);
    }

    // update the mixin class subclass list to not have this class
    removeSubclass(mixin_class);
    // and rebuild the class behaviour and broadcast to all of the subclasses
    updateSubClasses();
    return OREF_NULL;
}


/**
 * Remove a subclass from the uninherit list after an uninherit
 * operation.
 *
 * @param c      The class to remove.
 */
void RexxClass::removeSubclass(RexxClass *c)
{
    subClasses->removeItem(c);
}


/**
 * Create a new object of the receive class that has
 * had additional instance methods added to the created
 * instance.  This creates a one-off object instance.
 *
 * @param args     The array of arguments to the method.
 * @param argCount The count of method arguments.
 *
 * @return The inhanced object instance.
 */
RexxObject *RexxClass::enhanced(RexxObject **args, size_t argCount)
{
    // we need at least a source for the method table.
    if (argCount == 0)
    {
        reportException(Error_Incorrect_method_minarg, IntegerOne);
    }

    // ok, get the table argument and make sure we really got something.
    RexxObject *enhanced_methods = args[0];
    requiredArgument(enhanced_methods, "methods");

    // create a dummy subclass of the receiver class
    Protected<RexxClass> dummy_subclass = subclass(OREF_NULL, new_string("Enhanced Subclass"), OREF_NULL, OREF_NULL);
    // create a method dictionary from the collection of methods. We use .nil, so that these additional methods will look like
    // they were added with setMethod
    Protected<MethodDictionary> enhanced_instance_mdict = dummy_subclass->createMethodDictionary(enhanced_methods, (RexxClass *)TheNilObject);
    // enhance the instance behaviour of the dummy subclass with the new methods
    dummy_subclass->instanceMethodDictionary->merge(enhanced_instance_mdict);
    // and record the changes in behavior
    dummy_subclass->instanceBehaviour->addInstanceMethods(enhanced_instance_mdict);
    // recreate the instance behaviour
    dummy_subclass->instanceBehaviour->setMethodDictionary(OREF_NULL);
    dummy_subclass->createInstanceBehaviour(dummy_subclass->instanceBehaviour);

    // see if we have an uninit method defined now that this is done
    dummy_subclass->checkUninit();

    ProtectedObject r;
    // now create an instance of the enhanced subclass
    dummy_subclass->sendMessage(GlobalNames::NEW, args + 1, argCount - 1, r);
    RexxObject *enhanced_object = (RexxObject *)r;
    // change the create_class in the instance behaviour to point to the
    // original class object
    enhanced_object->behaviour->setOwningClass(this);
    // remember it was enhanced
    enhanced_object->behaviour->setEnhanced();

    // and return this one-off class.
    return enhanced_object;
}


/**
 * Create a mixinclass of a class directly from Rexx code.
 *
 * @param class_id   The id of the created class.
 * @param meta_class The meta class to create this from.
 * @param enhancing_class_methods
 *                   Additional class methods.
 *
 * @return A created class object.
 */
RexxClass  *RexxClass::mixinClassRexx(RexxString  *class_id, RexxClass *meta_class, RexxObject *enhancing_class_methods)
{
    // just forward with no source object specified
    return mixinClass(OREF_NULL, class_id, meta_class, enhancing_class_methods);
}


/**
 * Create a mixin class that can be used for INHERIT.
 *
 * @param package    The source this is created from (can be
 *                   null if created using methods.
 * @param mixin_id   The id of the class object.
 * @param meta_class The metaclass this is created from
 * @param enhancing_class_methods
 *                   Additional class methods to be added to
 *                   this class.  This can be any collection
 *                   supports a supplier method.
 *
 * @return A created class object.
 */
RexxClass *RexxClass::mixinClass(PackageClass *package, RexxString *mixin_id,
    RexxClass *meta_class, RexxObject *enhancing_class_methods)
{
    // go create the subclass, then convert to a mixin type
    RexxClass *mixin_subclass = subclass(package, mixin_id, meta_class, enhancing_class_methods);
    mixin_subclass->setMixinClass();

    // the mixin subclass is our baseclass
    mixin_subclass->baseClass = baseClass;

    // If the mixin's parent class has an uninit defined, the new mixin class must have one, too
    if (hasUninitDefined() || parentHasUninitDefined())
    {
        mixin_subclass->setParentHasUninitDefined();
    }
    return mixin_subclass;               /* return the new mixin class        */
}


/**
 * Create a subclass of a class directly from Rexx code.
 *
 * @param class_id   The id of the created class.
 * @param meta_class The meta class to create this from.
 * @param enhancing_class_methods
 *                   Additional class methods.
 *
 * @return A created class object.
 */
RexxClass *RexxClass::subclassRexx(RexxString  *class_id, RexxClass *meta_class, RexxObject *enhancing_class_methods)
{
    // just forward with no source object specified
    return subclass(OREF_NULL, class_id, meta_class, enhancing_class_methods);
}


/**
 * Create a subclass of a class.
 *
 * @param source     The source containing the directive this is created from.
 *                   If created dynamically from a method, this will be null.
 * @param class_id   The id of the created class.
 * @param meta_class The meta class to create this from.
 * @param enhancing_class_methods
 *                   Additional class methods.
 *
 * @return A created class object.
 */
RexxClass  *RexxClass::subclass(PackageClass *package, RexxString *class_id,
    RexxClass *meta_class, RexxObject *enhancing_methods)
{
    // no explicit metaclass specified?  Then use our metaclass
    if (meta_class == OREF_NULL)
    {
        meta_class = getMetaClass();
    }

    // check that it is a meta class
    if (!meta_class->isInstanceOf(TheClassClass) || !meta_class->isMetaClass())
    {
        reportException(Error_Translation_bad_metaclass, meta_class);
    }

    ProtectedObject p;
    // now get an instance of the meta class
    RexxClass *new_class = (RexxClass *)meta_class->sendMessage(GlobalNames::NEW, class_id, p);

    // hook this up with the source as early as possible.
    new_class->setPackage(package);

    // if the superclass (the classes processing the subclass operation)
    // then the new class is a metaclass too
    if (isMetaClass())
    {
        new_class->setMetaClass();
        // add the class instance info to the metaclass lists
        new_class->metaClass = this;
    }

    // set up the new_class behaviour to match the subclass reciever
    new_class->instanceBehaviour->subclass(instanceBehaviour);
    // set this class as the superclass new class superclass list
    new_class->superClass = this;
    // and also make this the superclass list...inherits will add to this
    new_class->superClasses = new_array(this);

    // if we have enhancing methods, create an instance method dictionary using the
    // new class as the scope.
    if (enhancing_methods != OREF_NULL)
    {
        // create a method dictionary and merge this into the class method dictionary
        Protected<MethodDictionary> enhancing_class_methods = new_class->createMethodDictionary(enhancing_methods, new_class);
        // these are methods of the class object, not instances
        new_class->classMethodDictionary->merge(enhancing_class_methods);
    }

    // start out the class behaviour clean
    new_class->behaviour->clearMethodDictionary();
    // build the class behaviour for the class object
    new_class->createClassBehaviour(new_class->behaviour);
    // indicate that the behaviour actually belongs to the meta_class
    new_class->behaviour->setOwningClass(meta_class);

    // now create the instance behaviour
    new_class->instanceBehaviour->clearMethodDictionary();
    new_class->createInstanceBehaviour(new_class->instanceBehaviour);

    // this behaviour is owned by the new class
    new_class->instanceBehaviour->setOwningClass(new_class);
    // record that we have a new subclass to worry about if
    // something changes in this class

    addSubClass(new_class);
    // we need to look for an uninit method and record if we have it
    new_class->checkUninit();
    ProtectedObject result;
    // drive the new class INIT method
    new_class->sendMessage(GlobalNames::INIT, result);

    // If the parent class has an uninit defined, the new child class must have one, too
    if (hasUninitDefined() || parentHasUninitDefined())
    {
        new_class->setParentHasUninitDefined();
    }

    // notify activity this object has an UNINIT that needs to be called when collecting the object
    if (new_class->hasUninitDefined())
    {
        new_class->setHasUninitDefined();
    }

    return new_class;
}


/**
 * Test if the target class is a "compatible" with the argument
 * class.  To be compatible, the target class must either A)
 * be the same class, B) be a direct subclass of the argument
 * class, or C) inherit the argument class as a mixin.  This
 * rule gets applied recursively down the hierarchy.
 *
 * @param other  The comparison class.
 *
 * @return True if the two classes are compatible, false otherwise.
 */
bool RexxClass::isCompatibleWith(RexxClass *other)
{
    // if asking for a match here, this is true
    if (other == this)
    {
        return true;
    }

    // if this is .object, there are no superclasses.  Otherwise, ask each of the superclasses
    // the same question.
    if (superClasses != OREF_NULL)
    {
        for (size_t i = 1; i <= superClasses->size(); i++)
        {
            if (((RexxClass *)superClasses->get(i))->isCompatibleWith(other))
            {
                return true;
            }
        }
    }
    return false;
}


/**
 * A stub to test compatibility of two classes.
 *
 * @param other  The class for the superclass test.
 *
 * @return True if the class is a subclass of the argument class (or IS
 *         the argument class).
 */
RexxObject *RexxClass::isSubclassOf(RexxClass *other)
{
    // verify we have a valid class object to check
    classArgument(other, TheClassClass, "class");
    return booleanObject(isCompatibleWith(other));
}


/**
 * Exported access to an object virtual function
 *
 * @return The default name of the class object.
 */
RexxString *RexxClass::defaultNameRexx()
{
    return defaultName();          // forward to the virtual function
}


/**
 * Set the source object what a class was created in.  This
 * will be the source that contains the ::class directive
 * that defined the class.
 *
 * @param s      The package file containing the ::class directive that
 *               created this class.
 */
void RexxClass::setPackage(PackageClass *s)
{
    setField(package, s);
}

/**
 * Return the package containing the directive that
 * defined a class.
 *
 * @return The package containing the directive that defined this
 *         class, or .nil if this class was not created from a
 *         directive.
 */
PackageClass *RexxClass::getPackage()
{
    // return the package we've been associated with.
    return (PackageClass *)resultOrNil(package);
}


// all of the new methods need to check if they are marked as
// abstract as a subclass...this centralizes the check.
void RexxClass::checkAbstract()
{
    if (isAbstract())
    {
        reportException(Error_Execution_abstract_class, id);
    }
}


/**
 * Mark a class as abstract, if this is allowed for this
 * type of class.
 */
void RexxClass::makeAbstract()
{
    if (isMetaClass())
    {
        reportException(Error_Execution_abstract_metaclass, id);
    }
    setAbstract();
}


/**
 * Create a new class for a rexx class
 * A copy of this class object is made
 * This class' behaviour, class_mdict, metaclass, and class_info
 * are used in the new class. All the rest of the object state
 * data is updated to reflect a new class object
 *
 * @param args     The args to the new method.
 * @param argCount The argument count.
 *
 * @return A new class object.
 */
RexxClass  *RexxClass::newRexx(RexxObject **args, size_t argCount)
{
    // we need at least one argument
    if (argCount == 0)
    {
        reportException(Error_Incorrect_method_minarg, IntegerOne);
    }

    // first argument is the class id...make sure it is a string value
    Protected<RexxString> class_id = (RexxString *)args[0];
    class_id = stringArgument(class_id, "class id");

    // get a copy of this class object
    Protected<RexxClass> new_class = (RexxClass *)clone();

    new_class->id = class_id;
    // the new class does not inherit annotations
    new_class->annotations = OREF_NULL;

    // no new class objects start out as abstract.
    new_class->clearAbstract();

    // make this into an instance of the
    // meta class
    new_class->behaviour = (RexxBehaviour *)new_class->instanceBehaviour->copy();
    // don't give access to this class' class mdict
    new_class->classMethodDictionary = new MethodDictionary();
    // and set the behaviour class
    new_class->behaviour->setOwningClass(this);
    // if this is a primitive class then there isn't any metaclass info yet
    if (isPrimitiveClass())
    {
        // if this is a primitive class, then Class is always the metaclass
        new_class->metaClass = TheClassClass;
    }
    else
    {
        // use this non-primitive class as the metaclass
        new_class->metaClass = this;
    }

    // create the subclasses list
    new_class->subClasses = new_list();
    // set up the instance behaviour with object's instance methods
    new_class->instanceBehaviour = (RexxBehaviour *)TheObjectClass->instanceBehaviour->copy();
    // don't give access to this class' instance mdict
    new_class->instanceMethodDictionary = new MethodDictionary();
    // the immediate superclass is always object
    new_class->superClass = TheObjectClass;
    // make the instance_superclass list with OBJECT in it
    new_class->superClasses = new_array(TheObjectClass);
    // and set the behaviour class
    new_class->instanceBehaviour->setOwningClass(TheObjectClass);
    // set the scoping info
    new_class->instanceBehaviour->addScope(TheObjectClass);
    // don't give access to this class' ovd's
    new_class->objectVariables = OREF_NULL;
    // set the new class as it's own baseclass
    new_class->baseClass = new_class;
    // clear the info area except for uninit
    new_class->setInitialFlagState();
    // if the class object has an UNINIT method defined, make sure we
    // add this to the table of classes to be processed.
    if (new_class->hasUninitDefined())
    {
        new_class->setHasUninitDefined();
    }

    ProtectedObject result;
    // send the new class the INIT method
    new_class->sendMessage(GlobalNames::INIT, args + 1, argCount - 1, result);
    return new_class;
}


/**
 * Create the initial class object
 */
void RexxClass::createInstance()
{
    // create a class object
    TheClassClass = (RexxClass *)new_object(sizeof(RexxClass));
    // set the instance behaviour
    TheClassClass->setBehaviour(TheClassClassBehaviour);
    // set the instance behaviour
    TheClassClass->setInstanceBehaviour(TheClassBehaviour);

    // the initial class needs to have an ID before it can be used for
    // other purposes.
    TheClassClass->id = new_string("Class");

    // tell the mobile support to just make a proxy for this class
    TheClassClass->makeProxiedObject();
    ::new ((void *)TheClassClass) RexxClass;
}

/**
 * Perform common initialization steps on an object created
 * by a new method from Rexx.  This handles subclass
 * behaviour issues, uninit processing, etc.
 *
 * @param obj      The newly created object.  NOTE:  this assumes the
 *                 caller has protected this object from garbage collection.
 * @param initArgs A pointer to arguments intended for the INIT method.
 * @param argCount The count of arguments.
 */
void RexxClass::completeNewObject(RexxObject *obj, RexxObject **initArgs, size_t argCount)
{
    // this is a good common place to perform the abstract checks
    checkAbstract();

    // set the behaviour (this might be a subclass, so don't assume the
    // one from the base class is correct).
    obj->setBehaviour(getInstanceBehaviour());
    // a subclass might define an uninit method, so we need to
    // check that also.
    if (hasUninitDefined())
    {
        obj->requiresUninit();
    }

    ProtectedObject result;
    // now send an INIT message to complete initialization.
    obj->sendMessage(GlobalNames::INIT, initArgs, argCount, result);
}


/**
 * Common routine for processing class new method arguments.
 *
 * @param arg_array The original array of arguments.
 * @param argCount  The count of arguments passed to the new method.
 * @param init_args The arguments left for an INIT method after processing the new argument.
 * @param remainderSize
 *                  The count of arguments remaining after stripping off the
 *                  new arguments.
 * @param required  The count of required arguments.
 * @param argument1 Pointer to the first argument.
 * @param argument2 Pointer to the second argument.
 */
void RexxClass::processNewArgs(RexxObject **arg_array, size_t argCount, RexxObject**&init_args,
    size_t &remainderSize, size_t required, RexxObject *&argument1, RexxObject **argument2 )
{
    // we only get called if we have at least one argument to process.  The second argument
    // is optional.  If we have an argument, then set it to the first item.
    argument1 = OREF_NULL;
    if (argCount >= 1)
    {
        argument1 = arg_array[0];
    }

    // if we need at least two arguments, handle this too.
    if (required == 2)
    {
        *argument2 = OREF_NULL;
        if (argCount >= 2)                 /* get at least 2?                   */
        {
            *argument2 = arg_array[1];       /* get the second argument           */
        }
    }
    // now update the argument pointer and count
    init_args = arg_array + required;
    // if we have at least the required arguments, reduce the count.
    // Otherwise, set this to zero.
    if (argCount >= required)
    {
        remainderSize = argCount - required;
    }
    else
    {
        remainderSize = 0;
    }
}


/**
 * Copy the instance method dicitionary for a class
 *
 * @return The array of all scopes
 */
MethodDictionary *RexxClass::copyInstanceMethods()
{
    return (MethodDictionary *)instanceMethodDictionary->copy();
}


/**
 * Merge the instance behaviour for this class into another
 * behaviour.
 *
 * @param targetBehaviour
 *               The target behaviour.
 */
void RexxClass::mergeInstanceBehaviour(RexxBehaviour *targetBehaviour)
{
    targetBehaviour->merge(instanceBehaviour);
}


/**
 * Merge this classes method dictionary into a behaviour.
 *
 * @param targetBehaviour
 *               The target behaviour.
 */
void RexxClass::mergeClassMethodDictionary(RexxBehaviour *targetBehaviour)
{
    targetBehaviour->mergeMethodDictionary(classMethodDictionary);
}

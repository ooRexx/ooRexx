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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Behaviour Class                                                  */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxBehaviour.hpp"
#include "StringClass.hpp"
#include "MethodClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ProtectedObject.hpp"
#include "CPPCode.hpp"
#include "MethodArguments.hpp"
#include "Memory.hpp"
#include "MethodDictionary.hpp"


/**
 * Construct a statically defined primitive behaviour.
 * Behaviours are created originally in a table of objects that
 * are not allocated from object heap memory.  There is one
 * primitive behaviour for every class defined in
 * PrimitiveClasses.xml.  After that, all additional behaviours
 * are created using copies of one of the primitive behaviours.
 *
 * @param newTypenum The primitive type number.
 * @param operator_methods
 *                   The associated operator methods.
 */
RexxBehaviour::RexxBehaviour(ClassTypeCode newTypenum, PCPPM *operator_methods)
{
    // All behaviour objects have a behaviour object too.
    behaviour = getPrimitiveBehaviour(T_Behaviour);
    // these are not created via normal means, so we need to hand construct
    // the header information.
    header.setObjectSize(sizeof(RexxBehaviour));
    setClassType(newTypenum);
    behaviourFlags.reset();
    methodDictionary = OREF_NULL;
    operatorMethods = operator_methods;
    owningClass = OREF_NULL;

    // if this is an internal class, normalize this so we can
    // restore this to the correct value if we add additional internal classes.
    if (newTypenum > T_Last_Exported_Class && newTypenum < T_First_Transient_Class)
    {

        behaviourFlags.set(INTERNAL_CLASS);
    }
    else if (newTypenum >= T_First_Transient_Class)
    {

        behaviourFlags.set(TRANSIENT_CLASS);
    }
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void RexxBehaviour::live(size_t liveMark)
{
    // This is an object that is in the image itself, so is is already
    // protected from collection:
    // primitiveBehaviours

    memory_mark(methodDictionary);
    memory_mark(owningClass);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxBehaviour::liveGeneral(MarkReason reason)
{
    // special handling if marking during a save image.
    if (reason == SAVINGIMAGE)
    {
        // if non primitive, this will need extra processing during restore.
        if (isNonPrimitive())
        {
            // mark this as needing resolution when restored.
            setNotResolved();
        }
        // even though we re-resolve this on restore, we null this out so what
        // we create a consistent image build
        operatorMethods = NULL;
    }
    // the other side of the process?
    else if (reason == RESTORINGIMAGE)
    {
    // if we have a non-primitive here on a restore image, we need to fix this up.
        if (isNonPrimitive())
        {
            resolveNonPrimitiveBehaviour();
        }
    }

    memory_mark_general(methodDictionary);
    memory_mark_general(owningClass);
}


/**
 * Flatten the behaviour contents
 *
 * @param envelope The envelope we're flattening into.
 */
void RexxBehaviour::flatten(Envelope *envelope)
{
    setUpFlatten(RexxBehaviour)

    flattenRef(methodDictionary);
    flattenRef(owningClass);

    // if this is a non-primitive behaviour, we need to mark this for restore
    // during the puff operation.
    if (isNonPrimitive())
    {
        newThis->setNotResolved();
    }
    cleanUpFlatten
}


/**
 * Set a new method dictionary in the behaviour.
 *
 * @param m      The new dictionary.
 */
void RexxBehaviour::setMethodDictionary(MethodDictionary *m)
{
    setField(methodDictionary, m);
};


/**
 * Set a new owning class for this behaviour.
 *
 * @param c      The new class.
 */
void RexxBehaviour::setOwningClass(RexxClass *c)
{
    setField(owningClass,  c);
};


/**
 * Do fix ups for non-primitive behaviours, ensuring they
 * get all of the appropriate information from their parent
 * primitive behaviour types.
 */
void RexxBehaviour::resolveNonPrimitiveBehaviour()
{
    if (isNotResolved())
    {
        setResolved();
        operatorMethods = getOperatorMethods(getClassType());
    }
}


/**
 * Copy a behaviour object.  This will make copies of all
 * of the contained tables so that the relevant information
 * can be changed independently of the original behaviour.
 *
 * @return The new behaviour object.
 */
RexxInternalObject *RexxBehaviour::copy()
{
    // first, clone the existing object
    Protected<RexxBehaviour> newBehaviour = (RexxBehaviour *)clone();
    // complete the copy process
    newBehaviour->copyBehaviour();
    // all copies are non-primitive.
    newBehaviour->setNonPrimitive();
    // a copy operation generally means we're subclassing, so revert to the
    // default operator methods:
    newBehaviour->operatorMethods = RexxObject::operatorMethods;
    return newBehaviour;
}


/**
 * Copy the internal tables of a behaviour object...used to
 * finish up the copy() operation.
 */
void RexxBehaviour::copyBehaviour()
{
    // we already have a method that copies information from only instance into
    // a target instance.  We'll just copy back into ourselves.
    copyBehaviour(this);
}


/**
 * Copy the source behaviour object into this, inheriting all of
 * the method dictionaries.  Generally done during an image
 * restore to restore the behaviours from the image.
 *
 * @param source The source behaviour.
 */
void RexxBehaviour::copyBehaviour(RexxBehaviour *source)
{
    setField(methodDictionary, source->copyMethodDictionary());
    // this is the same class as the source also
    setField(owningClass, source->owningClass);
    // copy the same operator methods.
    operatorMethods = (PCPPM *)source->operatorMethods;
}


/**
 * Define a native kernel method on this behaviour.
 *
 * @param name       The method name.
 * @param entryPoint The method entry point
 * @param arguments  The argument definition.
 *
 * @return The created method object.
 */
MethodClass *RexxBehaviour::defineMethod(const char *name, PCPPM entryPoint, size_t arguments, const char *entryPointName)
{
    // we're doing this during an image build, so make sure we use the interned string name.
    RexxString *n = memoryObject.getUpperGlobalName(name);
    // create a method object using the resolved method pointer.
    MethodClass *method = new MethodClass(n, CPPCode::resolveExportedMethod(name, entryPoint, arguments, entryPointName));
    // now add this to the method dictionary, ensuring it is the only method by this name.
    replaceMethod(n, method);
    // we need the created method object if adding modifiers after creation.
    return method;
}


/**
 * Block use of an inherited method by adding TheNilObject
 * as an entry in the table.
 *
 * @param name   The target name.
 */
void RexxBehaviour::hideMethod(const char *name)
{
    // we're doing this during an image build, so make sure we use the interned string name.
    RexxString *n = memoryObject.getUpperGlobalName(name);
    // create a method dictionary if we don't have one yet.
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, new MethodDictionary());
    }

    methodDictionary->hideMethod(n);
}


/**
 * Remove an object entirely from a method dictionary.
 *
 * @param name   The target name.
 */
void RexxBehaviour::removeMethod(const char *name)
{
    // nothing to remove if we don't have a dictionary yet.
    if (methodDictionary != OREF_NULL)
    {
        // we're doing this during an image build, so make sure we use the interned string name.
        RexxString *n = memoryObject.getUpperGlobalName(name);
        methodDictionary->removeMethod(n);
    }

}


/**
 * Add a method to the method dictionary during image setup.
 * This occurs while we are constructing the instance behaviours
 * of the different classes.  If we've inherited a set of
 * methods from another class and then define a replacement
 * method, we want to completely replace the inherited method
 * rather than leave it in the method dictionary.
 *
 * @param methodName The method name.
 * @param method     The target method object.
 */
void RexxBehaviour::replaceMethod(RexxString *methodName, MethodClass *method)
{
    // create a method dictionary if we don't have one yet.
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, new MethodDictionary());
    }

    methodDictionary->replaceMethod(methodName, method);
}


/**
 * Inherit a set of instance methods from another behaviour.
 * This occurs early in building up the primitive classes,
 * so we just update the defintions at this point.  Completion
 * of the processing will get methods of the correct scope
 * created.
 *
 * @param source The source behaviour we're inheriting from.
 */
void RexxBehaviour::inheritInstanceMethods(RexxBehaviour *source)
{
    // create a method dictionary if we don't have one yet.
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, new MethodDictionary());
    }
    // have this merge all of the methods from the other dictionary into
    // ours.  This will replace any existing methods (although we generally
    // only use this on an empty dictionary).
    methodDictionary->replaceMethods(source->getMethodDictionary(), source->getOwningClass(), getOwningClass());
}


/**
 * Add a method to the method dictionary.l
 *
 * @param methodName The method name.
 * @param method     The target method object.
 */
void RexxBehaviour::defineMethod(RexxString *methodName, MethodClass *method)
{
    // create a method dictionary if we don't have one yet.
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, new MethodDictionary());
    }

    methodDictionary->addMethod(methodName, method);
}


/**
 * Remove a method from the behaviour.  this must be an instance
 * method defined via SETMETHOD
 *
 * @param methodName The name of the method to remove.
 */
void RexxBehaviour::removeInstanceMethod(RexxString *methodName)
{
    methodDictionary->removeInstanceMethod(methodName);
}


/**
 * Add an instance method to an object's behaviour.
 *
 * @param methodName The name of the method to add.
 * @param method
 */
void RexxBehaviour::addInstanceMethod(RexxString *methodName, MethodClass *method)
{
    // create a method dictionary if we don't have one yet (highly unusual for that to
    // be the case).
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, new MethodDictionary());
    }

    methodDictionary->addInstanceMethod(methodName, method);
}


/**
 * Retrieve a method object associated with a given name.
 *
 * @param messageName
 *               The name of the desired method.
 *
 * @return Any associated method object.
 */
MethodClass *RexxBehaviour::getMethodObject(RexxString *messageName )
{
    // force to a string version (upper case required)
    messageName = stringArgument(messageName, ARG_ONE)->upper();
    return methodLookup(messageName);
}


/**
 * Perform method lookup on a object.  This version filters out
 * suppressed methods.
 *
 * @param messageName
 *               The target message name.
 *
 * @return The associated method object (if any)
 */
MethodClass *RexxBehaviour::methodLookup(RexxString *messageName)
{
    // just get the object directly.  Unknown methods will return OREF_NULL.  However,
    // explicit overrides are indicated by putting .nil in the table.  Our callers
    // are dependent upon getting OREF_NULL back for unknown methods.
    MethodClass *method = methodDictionary->getMethod(messageName);
    if (method != TheNilObject)
    {
        return method;
    }
    return OREF_NULL;
}


/**
 * Get a method object from the method dictionary.  If the
 * object is suppressed by putting .nil into the table, this
 * is still returned.
 *
 * @param messageName
 *               The target message name.
 *
 * @return Any value from the method dictionary.
 */
MethodClass *RexxBehaviour::getMethod(RexxString *messageName)
{
    return methodDictionary->getMethod(messageName);
}


/**
 * Delete a method from an object's behaviour.
 *
 * @param messageName
 *               The name of the method.
 *
 * @return The deleted method, if any.
 */
void RexxBehaviour::deleteMethod(RexxString *messageName)
{
    // this is a class definition we're removing, so just delete from the
    // table.
    methodDictionary->remove(messageName);
}


/**
 * Subclass a behaviour from another classes base type.
 * Used to subclass the primitive classes.
 *
 * @param subclass_behaviour
 *               The source behaviour for the subclass.
 */
void RexxBehaviour::subclass(RexxBehaviour *subclass_behaviour)
{
    setClassType(subclass_behaviour->getClassType());
}


/**
 * Restore a primitive behaviour after an image restore.
 *
 * @param saved  The behaviour that was stored in the saved image.
 */
void RexxBehaviour::restore(RexxBehaviour * saved)
{
    // set our object type
    setBehaviour(getPrimitiveBehaviour(T_Behaviour));
    // fix up the memory management bits, and also turn on
    // oldspace.
    setObjectSize(Memory::roundObjectBoundary(sizeof(RexxBehaviour)));
    setOldSpace();

    // NOTE:  In this situation, we're assigning into the static
    // behaviour from an oldspace saved version.  We don't want to
    // use setField() to set these right now because memory might
    // not be completely set up yet.

    // now pull in the method dictionary from the saved copy.
    methodDictionary = saved->getMethodDictionary();
    owningClass = saved->getOwningClass();
}


/**
 * Update a behaviour in a class objects behaviour
 * during image restore.
 *
 * @return Owning class.
 */
RexxClass *RexxBehaviour::restoreClass()
{
    // Adjust the instance behaviour.  Note that we don't use
    // OrefSet() for this.  When we're restoring the classes, the
    // class objects are in oldspace, and the behaviours are
    // primitive objects, not subject to sweeping.  We do a direct
    // assignment to avoid creating a reference entry in the old2new
    // table.
    owningClass->setInstanceBehaviour(this);
    return owningClass;            /* return the associated class       */
}


/**
 * Locate the scope following a given scope.
 *
 * @param start_scope
 *               The starting scope.
 *
 * @return The following scope, or .nil if not found.
 */
RexxClass *RexxBehaviour::superScope(RexxClass *start_scope)
{
    // methods executing after a setMethod or via RUN have a scope of
    // .nil.  The superscope for those methods are the owning class
    if (start_scope == TheNilObject)
    {
        return owningClass;
    }
    // class objects maintain this directly
    return methodDictionary->resolveSuperScope(start_scope);
}


/**
 * Get the immediate superscope defined for this behaviour.
 *
 * @return The superscope that a method defined by this class would
 *         use to set the SUPER variable for lookups.
 */
RexxClass *RexxBehaviour::immediateSuperScope()
{
    return methodDictionary->resolveSuperScope(owningClass);
}


/**
 * Locate a super class method given the starting lookup
 * information.
 *
 * @param messageName
 *                   The target message name.
 * @param startScope The starting lookup scope.
 *
 * @return The matching method (if any)
 */
MethodClass *RexxBehaviour::superMethod(RexxString * messageName, RexxClass *startScope)
{
    // delegate this to the method dictionary.
    return methodDictionary->findSuperMethod(messageName, startScope);
}


/**
 * Set a new set of scoping information for all methods in a
 * method dictionary.  Used during image setup processing.
 *
 * @param scope  The scope to set.
 */
void RexxBehaviour::setMethodDictionaryScope(RexxClass *scope)
{
    // we might not have instance methods to process
    if (methodDictionary != OREF_NULL)
    {
        methodDictionary->setMethodScope(scope);
    }
}


/**
 * Extract from the method dictionary all methods defined with
 * a given scope.
 *
 * @param scope  The target scope.  If null, then all methods
 *               are returned.
 *
 * @return A supplier holding the names and methods with the target
 *         scope.  This supplier can be empty.
 */
SupplierClass *RexxBehaviour::getMethods(RexxClass *scope)
{
    // the method dictionary handles all of this.
    return methodDictionary->getMethods(scope);
}


/**
 * Add a new scope to the set used by the behaviour.
 *
 * @param scope  The new scope class
 */
void RexxBehaviour::addScope(RexxClass *scope)
{
    // create a method dictionary if we don't have one yet.
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, new MethodDictionary());
    }
    // scoping is handled by the method dictionary.
    methodDictionary->addScope(scope);
}


/**
 * Merge another behaviour's method dictionary into this
 * one.  Our information will take precedence over the source
 * behaviour.
 *
 * @param source_behav
 *               The source behaviour to merge in.
 */
void RexxBehaviour::merge(RexxBehaviour *source_behav)
{
    // merge the method dictionaries
    mergeMethodDictionary(source_behav->methodDictionary);
}


/**
 * Merge a method dictionary without method dictionary.
 * The target dictionary methods will take lookup priority
 * over the source dictionary methods.
 *
 * @param sourceDictionary
 *               The source for the merge.
 */
void RexxBehaviour::mergeMethodDictionary(MethodDictionary *sourceDictionary)
{
    // no source is a NOP
    if (sourceDictionary == OREF_NULL)
    {
        return;
    }

    // if we have nothing to merge yet, then just use the source
    // method dictionary.
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, (MethodDictionary *)sourceDictionary->copy());
    }
    else
    {
        // merge our methods and scope into the copy
        methodDictionary->merge(sourceDictionary);
    }
}


/**
 * Get an array of all scopes defined in a behaviour.
 *
 * @return An array of the behaviour scopes.
 */
ArrayClass *RexxBehaviour::allScopes()
{
    return methodDictionary->allScopes();
}


/**
 * Test if a scope is defined in a behaviour.
 *
 * @param scope  The target scope.
 *
 * @return True if this scope has already been added, false otherwise.
 */
bool RexxBehaviour::hasScope(RexxClass *scope)
{
    if (methodDictionary == OREF_NULL)
    {
        return false;
    }

    return methodDictionary->hasScope(scope);
}


/**
 * Make a copy of the current method dictionary.
 *
 * @return The copy of the method dictionary, or OREF_NULL if this
 *         behaviour does not have one.
 */
MethodDictionary *RexxBehaviour::copyMethodDictionary()
{
    if (methodDictionary == OREF_NULL)
    {
        return OREF_NULL;
    }
    return (MethodDictionary *)methodDictionary->copy();
}


/**
 * Test if the behaviour has additional instance methods defined.
 *
 * @return true if the object has specific instance methods, defined,
 *         false otherwise.
 */
bool RexxBehaviour::hasInstanceMethods()
{
    return methodDictionary == OREF_NULL ? false : methodDictionary->hasInstanceMethods();
}


/**
 * Add a collection of object-scope instance methods to
 * a behaviour.
 *
 * @param source The source collection of instance methods.
 */
void RexxBehaviour::addInstanceMethods(MethodDictionary *source)
{
    methodDictionary->addInstanceMethods(source);
}


/**
 * Add a full table of methods to a class definition.  This is
 * only used during the initial image build.
 *
 * @param newMethods The new methods to add.
 */
RexxObject *RexxBehaviour::defineMethods(StringTable *newMethods)
{
    // loop through the table with an iterator.
    HashContents::TableIterator iterator = newMethods->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        // get the name and the value, then add to this class object
        RexxString *method_name = (RexxString *)iterator.index();
        // if this is the Nil object, that's an override.  Make it OREF_NULL.
        MethodClass *method = (MethodClass *)iterator.value();
        if (method == TheNilObject)
        {
            method = OREF_NULL;
        }
        // define this method
        defineMethod(method_name, method);
    }
    return OREF_NULL;
}

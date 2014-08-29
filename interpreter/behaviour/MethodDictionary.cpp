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
/****************************************************************************/
/* REXX Kernel                                                              */
/*                                                                          */
/* A table of methods for an object behaviour                               */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "MethodDictionary.hpp"
#include "MethodClass.hpp"
#include "SupplierClass.hpp"

/**
 * Create a new table object.
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating a table object.
 */
void *MethodDictionary::operator new (size_t size)
{
    return new_object(size, T_MethodDictionary);
}


/**
 * Initialize a MethodDictionary.
 *
 * @param capacity The desired capacity.
 */
MethodDictionary::MethodDictionary(size_t capacity) : StringHashCollection(capacity)
{
    // Method dictionaries don't get created until we're defining/adding
    //. methods to a class.  We can assume that once created, we'll need most of the
    // items (except maybe the instance methods).
    instanceMethods = OREF_NULL;
    // this keeps our main lookup order, defined by the inheritance order of the
    // superclasses and mixins.
    scopeList = new_array(DefaultScopeListSize);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void MethodDictionary::live(size_t liveMark)
{
    memory_mark(contents);
    memory_mark(scopeList);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void MethodDictionary::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
    memory_mark_general(scopeList);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void MethodDictionary::flatten(Envelope *envelope)
{
    setUpFlatten(MethodDictionary)

    flattenRef(contents);
    flattenRef(scopeList);
    flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 *
 * Copy a method dictionary
 *
 * @return A new instance of this collection.
 */
RexxInternalObject *MethodDictionary::copy()
{
    // make a copy of the base object
    MethodDictionary *newObj = (MethodDictionary *)RexxObject::copy();
    // and copy the contents as well
    newObj->contents = (HashContents *)contents->copy();
    newObj->scopeList = (ArrayClass *)scopeList->copy();
    return newObj;
}


/**
 * Add a method to the main method dictionary
 *
 * @param methodName The target method name.
 * @param method     The method object to add.
 */
void MethodDictionary::addMethod(RexxString *methodName, MethodClass *method)
{
    // if there is no method, we're removing this method.
    // write .nil to the table
    if (method == OREF_NULL || method == TheNilObject)
    {
        put(TheNilObject, methodName);
    }
    else
    {
        // we need to check to see if we have an existing method.
        // how this gets handled differs depending on what we find in the table.
        MethodClass *tableMethod = (MethodClass *)getMethod(methodName);
        // if this is a new method, just put it into the table.
        if (tableMethod == OREF_NULL)
        {
            put(method, methodName);
        }
        else
        {
            // if the scopes are the same, then we are replacing this
            // method in the table.
            if (tableMethod->isScope(method->getScope()))
            {
               put(method, methodName);

            }
            else
            {
                // this is a method added at a new scope level, so
                // we insert this into the table so it is in front of the existing method
                // with the same name.
                addFront(method, methodName);
            }
        }
    }
}


/**
 * Add or completely replace a method in the method dictionary.
 * This is used during construction of the primitive classes
 * during image construction.  We might be replacing a method
 * inherited from another class, so we want to ensure that any
 * existing method by that name is completely removed.
 *
 * @param methodName The target method name.
 * @param method     The method object to add.
 */
void MethodDictionary::replaceMethod(RexxString *methodName, MethodClass *method)
{
    // We only use this to add methods, not hide them, so this is fairly simple. We
    // just put the new method into the table.
    put(method, methodName);
}


/**
 * Overlay a collection of methods on top of this dictionary.
 * At this time, we're just adding the entries to the table.
 * These will be updated with correct scopes later.
 *
 * @param source The source method dictionary.
 */
void MethodDictionary::replaceMethods(MethodDictionary *source, RexxClass *scope)
{
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = source->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        // copy these methods over any of our own.
        MethodClass *method = (MethodClass *)iterator.value();
        RexxString *name = (RexxString *)iterator.index();
        if (isMethod(method))
        {
            method->setScope(scope);
        }
        replaceMethod(name, method);
    }
}


/**
 * Overlay a collection of methods on top of this dictionary.
 *
 * @param source The source method dictionary.
 */
void MethodDictionary::replaceMethods(StringTable *source, RexxClass *scope)
{
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = source->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        // copy these methods over any of our own.
        MethodClass *method = (MethodClass *)iterator.value();
        RexxString *name = (RexxString *)iterator.index();
        if (isMethod(method))
        {
            method->setScope(scope);
        }
        replaceMethod(name, method);
    }
}


/**
 * Add a collection of methods on top of this dictionary. These
 * methods will override any methods already defined in this
 * dictionary.
 *
 * @param source The source method dictionary.
 * @param scope  The added method scopes.
 */
void MethodDictionary::addMethods(StringTable *source, RexxClass *scope)
{
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = source->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        // copy these methods over any of our own.
        MethodClass *method = (MethodClass *)iterator.value();
        RexxString *name = (RexxString *)iterator.index();
        if (isMethod(method))
        {
            method->setScope(scope);
        }
        addMethod(name, method);
    }
}


/**
 * Remove a method from the main method dictionary
 *
 * @param methodName The target method name.
 *
 * @return True if there was a method to remove, false otherwise.
 */
bool MethodDictionary::removeMethod(RexxString *methodName)
{
    return remove(methodName) != OREF_NULL;
}


/**
 * Block a method from execution by inserting a .nil
 * into the dictionary.  This is only used while constructing
 * the initial table and unconditionally places .nil under
 * the target name.
 *
 * @param methodName The method name to hid.
 */
void MethodDictionary::hideMethod(RexxString *methodName)
{
    put(TheNilObject, methodName);
}


/**
 * Reverse a SETMETHOD call on an object.
 *
 * @param name   The name of the method to remove.
 */
void MethodDictionary::removeInstanceMethod(RexxString *name)
{
    // This is only possible if we've had prior calls...
    if (instanceMethods != OREF_NULL)
    {
        // When we add a method to the behaviour dynamically, we add it
        // to both the instance methods and the regular methods.  The
        // instance methods one allows us to track what we've added, the
        // method dictionary version is just for the lookups.
        if (instanceMethods->remove(name) != OREF_NULL)
        {
            remove(name);
        }
    }
}


/**
 * Add an instance method to the dictionary.
 *
 * @param name   The name of the method.
 * @param method The target method object.
 */
void MethodDictionary::addInstanceMethod(RexxString *name, MethodClass *method)
{
    // this could be our first one (rather likely, actually)
    if (instanceMethods == OREF_NULL)
    {
        setField(instanceMethods, new_string_table());
    }
    else
    {
        // if we've already added one of these, remove
        // if from the main dictionary also
        if (instanceMethods->hasIndex(name))
        {
            remove(name);
        }
    }

    // add this to the front of the search order in the main dictionary
    addFront(method, name);
    // and also add to the instance dictionary, replacing any
    // existing method.
    instanceMethods->put(method, name);
}


/**
 * Merge a collection of methods into our object instance
 * behaviour.
 *
 * @param source The source method dictionary.
 */
void MethodDictionary::addInstanceMethods(MethodDictionary *source)
{
    HashContents::TableIterator iterator = source->iterator();

    // add all of the source methods as instance methods.
    for (; iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        RexxString *name = (RexxString *)iterator.index();
        addInstanceMethod(name, method);
    }
}


/**
 * Locate a method from the method dictionary with a
 * specific starting scope.
 *
 * @param name       The name of the target method.
 * @param startScope The starting scope value.
 */
MethodClass *MethodDictionary::findSuperMethod(RexxString *name, RexxClass *startScope)
{
    // get the list of scopes "visible" from this starting scope.
    ArrayClass *scopes = startScope->getScopeOrder();

    // do we have a list to search through?  Now search the matching methods
    // for one with a scope in this list.
    // use an interator that does not create a new object array.
    HashContents::IndexIterator iterator = contents->iterator(name);

    for (; iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        // we might have .nil in here as well as method objects.
        if (method != (MethodClass *)TheNilObject)
        {
            // if this methos has a scope that's in the allowed list, return it.
            if (scopes->hasItem(method->getScope()))
            {
                return method;
            }
        }
    }
    // nothing found
    return OREF_NULL;
}


/**
 * Set all methods in the method dictionary to the target scope.
 *
 * @param scope  The scope class.
 */
void MethodDictionary::setMethodScope(RexxClass *scope)
{
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = contents->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        // we might have .nil in here as well as method objects.
        if (method != (MethodClass *)TheNilObject)
        {
            // this is generally used while constructing the primitive classes
            // to set all methods in a method dictionary to the owning class scope.
            // because we can "inherit" methods enmass from other class behaviours,
            // we need to use newScope() to set the scope and replace the instance
            // in the table with the returned version.
            iterator.replace(method->newScope(scope));
        }
    }
}


/**
 * Get all methods with the target scope, or all methods
 * if the scope is omitted.
 *
 * @param scope  The target scope (can be NULL)
 *
 * @return A supplier to iterate over the requested methods.
 */
SupplierClass *MethodDictionary::getMethods(RexxClass *scope)
{
    // manually count first, then traverse again to fill the supplier arrays
    size_t count = 0;

    // use an iterator to traverse the table
    HashContents::TableIterator iterator = contents->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        // we're only interested in the real method objects.
        if (isMethod(method))
        {
            // only count if this is a scope match
            if (scope == OREF_NULL || method->isScope(scope))
            {
                count++;
            }
        }
    }

    ArrayClass *names = new_array(count);
    ArrayClass *methods = new_array(count);

    count = 1;


    // traverse again, filling in the methods
    iterator = contents->iterator();

    for (; iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        // we're only interested in the real method objects.
        if (isMethod(method))
        {
            // only count if this is a scope match
            if (scope == OREF_NULL || method->isScope(scope))
            {
                names->put(iterator.index(), count);
                methods->put(method, count);
                count++;
            }
        }
    }

    // and return as a supplier
    return (SupplierClass *)new_supplier(methods, names);
}


/**
 * Retrieve the immediate super scope item from the
 * behaviour class list.  This determines what value is
 * used for the super variable on method calls.
 *
 * @return The immediate superclass value (return .nil for the Object
 *         class).
 */
RexxClass *MethodDictionary::immediateSuperScope()
{
    // the owning class is going to be last class added to this list.
    size_t ourClass = scopeList->items();
    // Object will only have the single item defined.
    // TODO:  Better check this!  Might be better to
    // handle this elsewhere.
    if (ourClass == 1)
    {
        return (RexxClass *)TheNilObject;
    }
    // return the previously added item
    return (RexxClass *)scopeList->get(ourClass - 1);
}


/**
 * Find the superscope for a class object.
 *
 * @param scope  The starting scope.
 *
 * @return The value set to SUPER for a method call.
 */
RexxClass *MethodDictionary::findSuperScope(RexxClass *scope)
{
    // this is stored dirctly in the class object, so this is pretty
    // easy.
    return scope->getSuperScope();
}


/**
 * Add a scope to the scope table.  This is done unconditionall
 *
 * @param scope  The new scope to add.
 */
void MethodDictionary::addScope(RexxClass *scope)
{
    // only add a scope to the list if this is not already here.
    // this merges things together in the right order.
    if (!scopeList->hasIdentityItem(scope))
    {
        // append this to the scope list for this behaviour.  This is
        // our total class lookup scope and will be attached to the class
        // object once the behaviour is fully constructed.
        scopeList->append(scope);
    }
}


/**
 * Merge our methods into another method dictionary.  Our
 * methods take precedence in the search order.
 *
 * @param target The target MethodDictionary.
 */
void MethodDictionary::mergeMethods(MethodDictionary *target)
{
    // allow the target to process this without needing to expand
    // multiple times
    target->ensureCapacity(items());
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = contents->iterator();

    // just copy in all of the method entries.
    for (; iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        target->addMethod((RexxString *)iterator.index(), method);
    }
}


/**
 * Merge the scopy information from a method dictionary
 * into ours.  We only add in new scope classes.
 *
 * @param target The target method dictionary.
 */
void MethodDictionary::mergeScopes(MethodDictionary *target)
{
    size_t count = scopeList->items();
    // we merge these in using the same order they were added
    // to our method dictionary.  The target method dictionary
    // will ignore any that is already has seen...the new ones will
    // get added to the end of the list.
    for (size_t i = 1; i <= count; i++)
    {
        target->addScope((RexxClass *)scopeList->get(i));
    }
}


/**
 * Merge all of the method and scope information in this
 * method dictionary into the target dictionary.
 *
 * @param target The target we're merging into.
 */
void MethodDictionary::merge(MethodDictionary *target)
{
    // merge both the scopes and the target
    mergeMethods(target);
    mergeScopes(target);
}


/**
 * Test if the method dictionary already accounts for a given scope class.
 *
 * @param scope  The target scope object.
 *
 * @return True if this scope has already been merged into this
 *         method dictionary, false otherwise.
 */
bool MethodDictionary::hasScope(RexxClass *scope)
{
    return scopeList->hasIdentityItem(scope);
}

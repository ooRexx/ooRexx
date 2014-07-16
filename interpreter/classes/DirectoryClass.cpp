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
/* Primitive Directory Class                                                  */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "MethodClass.hpp"
#include "RexxActivation.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *DirectoryClass::classInstance = OREF_NULL;

/**
 * Create initial class object at bootstrap time.
 */
void DirectoryClass::createInstance()
{
    CLASS_CREATE(Directory, "Directory", RexxClass);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void DirectoryClass::live(size_t liveMark)
{
    memory_mark(contents);
    memory_mark(method_table);
    memory_mark(unknown_method);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void DirectoryClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
    memory_mark_general(method_table);
    memory_mark_general(unknown_method);
    memory_mark_general(objectVariables);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void DirectoryClass::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(DirectoryClass)

      flattenRef(contents);
      flattenRef(method_table);
      flattenRef(unknown_method);
      flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * Copy a directory item.
 *
 * @return A copy of the item.
 */
RexxObject *DirectoryClass::copy()
{
    // first copy via the superclass copy method
    Protected<DirectoryClass> newObj = (DirectoryClass *)HashCollection::copy();
    // if we have a method table, make a copy of that too.
    if (method_table != OREF_NULL)
    {
        newObj->method_table = (TableClass *)method_table->copy();
    }
    return newObj;
}


/**
 * Retrieve an entry from a directory, using the uppercase
 * version of the index.
 *
 * @param index The entry index.
 *
 * @return The indexed item, or OREF_NULL if the index was not found.
 */
RexxObject *DirectoryClass::entry(RexxString *index)
{
    // do the lookup with the upper case name
    return at(index->upper());
}


// TODO: make sure items is a virtual method in the base class.

/**
 * Return the count of items in the directory, including the
 * number of methods added via set method calls
 *
 * @return the count of items.
 */
size_t DirectoryClass::items()
{
    // get the direct table size
    size_t count = contents->items();
    // if we have a method table, add in its count as well.
    if (>method_table != OREF_NULL)
    {
        count += this->method_table->items();
    }
    return count;
}

/**
 * TODO:  check out how the supplier override is done in the base class.
 *
 * Create a supplier for a directory, including the results of all */
 * of the SETMETHOD methods as values                              */
 *
 * @return An appropriate supplier object.
 */
SupplierClass *DirectoryClass::supplier()
{
    // get the supplier for the base collection.
    Protected<SupplierClass> supplier = contents->supplier();

    // do we have a method table?  We need to include these also, which
    // requires running each of the methods to obtain the value.
    if (method_table != OREF_NULL)
    {
        Protected<RexxArray> indexes = new_array(method_table->items());
        Protected<RexxArray> values = new_array(method_table->items());

        size_t count = 1;

        HashContents::TableIterator iterator = method_table->iterator();

        for (; iterator.available(); iterator.next())
        {
            RexxString *name = (RexxString *)iterator.index();
            MethodClass *method = (MethodClass *)iterator.value_type();

            ProtectedObject v;
            // run the method, using the directory as the receiver
            method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);

            indexes->put(name, count);
            values->put((RexxObject *)v, i);
        }
        // append the method table part to the existing supplier
        supplier->append(values, indexes);
    }
    return supplier;
}


// TODO:  This should be part of the baseclass overrides.

RexxArray *DirectoryClass::makeArray()
/******************************************************************************/
/* Function:  Create an array of all of the directory indices, including those*/
/*            of all the SETMETHOD methods.                                   */
/******************************************************************************/
{
    return allIndexes();
}


/**
 * Create an array of all of the directory indices, including those
 * of all the SETMETHOD methods.
 *
 * @return An array containing all of the directory indices.
 */
RexxArray *DirectoryClass::allIndexes()
{
    // get the base set
    Protected<RexxArray> indexes = content->allIndexes();

    // if we have a method table, we need to append those indices also
    if (method_table != OREF_NULL)
    {
        indexes->appendAll(method_table->allIndexes());
    }
    return result;
}


/**
 * Create an array of all of the directory values, including the
 * values of all the SETMETHOD methods
 *
 * @return An array of all item values.
 */
RexxArray *DirectoryClass::allItems()
{
    // get the base set
    Protected<RexxArray> itemArray = content->allIndexes();
    // have a method table? we need to run the methods an append to the result
    if (method_table != OREF_NULL)
    {
        HashContents::TableIterator iterator = method_table->iterator();

        for (; iterator.available(); iterator.next())
        {
            MethodClass *method = (MethodClass *)iterator.value_type();

            ProtectedObject v;
            // run the method, using the directory as the receiver
            method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);

            itemArray->append((RexxObject *)v);
        }
    }

    return itemArray;
}


/**
 * This is the REXX version of entry.  It issues a STRINGREQUEST
 * message to the entryname parameter if it isn't already a
 * string or a name object.  Thus, this may raise NOSTRING.
 *
 * @param entryName The entry name.
 *
 * @return The entry value, if it has one.
 */
RexxObject *DirectoryClass::entryRexx(RexxString *entryName)
{
    // make sure we have a string argument and upper case it.
    return atRexx(stringArgument(entryName, ARG_ONE)->upper());
}


/**
 * Determine if the directory has an entry with this name (used
 * without uppercasing)
 *
 * @param indexName
 *
 * @return
 */
RexxObject *DirectoryClass::hasIndex(RexxString *indexName)
{
    /* get as a string parameter         */
    indexName = stringArgument(indexName, ARG_ONE);
    /* got a value?                      */
    if (this->contents->stringGet(indexName) != OREF_NULL)
    {
        return(RexxObject *)TheTrueObject;/* return true                       */
    }
    else
    {
        /* have a table?                     */
        if (this->method_table != OREF_NULL)
        {
            /* look for a method                 */
            MethodClass *method = (MethodClass *)this->method_table->stringGet(indexName);
            if (method != OREF_NULL)         /* have a method?                    */
            {
                /* then we have the index            */
                return(RexxObject *)TheTrueObject;
            }
        }
        /* not in the directory              */
        return(RexxObject *)TheFalseObject;
    }
}


RexxObject *DirectoryClass::hasEntry(
    RexxString *entryName)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Determine if an entry exists in the directory (name will be     */
/*            upper cased)                                                    */
/******************************************************************************/
{
    /* get as a string parameter         */
    entryName = stringArgument(entryName, ARG_ONE)->upper();
    /* in the table?                     */
    if (this->contents->stringGet(entryName) != OREF_NULL)
    {
        return(RexxObject *)TheTrueObject;/* this is true                      */
    }
    else
    {
        /* have a table?                     */
        if (this->method_table != OREF_NULL)
        {
            /* look for a method                 */
            MethodClass *method = (MethodClass *)this->method_table->stringGet(entryName);
            if (method != OREF_NULL)         /* have a method?                    */
            {
                /* then we have the index            */
                return(RexxObject *)TheTrueObject;
            }
        }
        /* not in the directory              */
        return(RexxObject *)TheFalseObject;
    }
}

RexxObject *DirectoryClass::setEntry(
  RexxString *entryname,               /* directory entry name              */
  RexxObject *entryobj)                /* associated object                 */
/******************************************************************************/
/* Function:  Add an entry to the directory (under an upper case name)        */
/******************************************************************************/
{
    /* get as a string parameter         */
    entryname = stringArgument(entryname, ARG_ONE)->upper();
    if (entryobj != OREF_NULL)
    {         /* have a new value?                 */
              /* try to place in existing hashtab  */
        RexxHashTable *newHash = this->contents->stringPut(entryobj, entryname);
        if (newHash  != OREF_NULL)         /* have a reallocation occur?        */
        {
            /* hook on the new hash table        */
            OrefSet(this, this->contents, newHash);
        }
        if (this->method_table != OREF_NULL)
        {
            this->method_table->remove(entryname);
        }
    }
    else
    {
        this->remove(entryname);           /* remove this entry                 */
    }
    return OREF_NULL;                    /* don't return a value              */
}


/**
 * ooRexx exported version of the directory remove method.
 *
 * @param entryname The index name.
 *
 * @return The removed item.  Returns .nil if the item did not exist
 *         in the directory.
 */
RexxObject *DirectoryClass::removeRexx(RexxString *entryname)
{
    /* get as a string parameter         */
    entryname = stringArgument(entryname, ARG_ONE);
    RexxObject *oldVal = remove(entryname);
    if (oldVal == OREF_NULL)
    {
        oldVal = TheNilObject;
    }
    return oldVal;
}

RexxObject *DirectoryClass::remove(
    RexxString *entryname)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Remove an entry from a directory.                               */
/******************************************************************************/
{
    RexxObject *oldVal = this->at(entryname);        /* go get the directory value        */
                                           /* have a real entry?                */
    if (this->contents->stringGet(entryname) != OREF_NULL)
    {
        this->contents->remove(entryname); /* remove the entry                  */
    }
    /* if there's a method entry, remove that one too! */
    /* have methods?                     */
    if (this->method_table != OREF_NULL)
    {
        /* remove this method                */
        this->method_table->remove(entryname->upper());
    }
    return oldVal;                       /* return the directory value        */
}

RexxObject *DirectoryClass::unknown(
  RexxString *msgname,                 /* name of unknown message           */
  RexxArray  *arguments)               /* arguments to the message          */
/******************************************************************************/
/* Function:     This is the REXX version of unknown.  It invokes entry_rexx  */
/*               instead of entry, to ensure the proper error checking and    */
/*               return value handling is performed.                          */
/******************************************************************************/
{
    /* validate the name                 */
    RexxString *message_value = stringArgument(msgname, ARG_ONE);
    requiredArgument(arguments, ARG_TWO);        /* need an argument array            */
                                         /* get the length                    */
    stringsize_t message_length = message_value->getLength();
    /* assignment form of access?        */
    if (message_length > 0 && message_value->getChar(message_length - 1) == '=')
    {
        /* get this as an array              */
        arguments = (RexxArray  *)REQUEST_ARRAY(arguments);
        /* not an array item or a multiple   */
        /* dimension one, or more than one   */
        /* argument (CHM)                    */
        if (arguments == TheNilObject || arguments->getDimension() != 1 || arguments->size() != 1 )
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_noarray, IntegerTwo);
        }
        /* extract the name part of the msg  */
        message_value = (RexxString *)message_value->extract(0, message_length - 1);
        /* do this as an assignment          */
        return this->setEntry(message_value, arguments->get(1));
    }
    else                                 /* just a lookup form                */
    {
        return this->entryRexx(message_value);
    }
}

RexxObject *DirectoryClass::setMethod(
    RexxString *entryname,             /* name to set this under            */
    MethodClass *methodobj)             /* new method argument               */
/******************************************************************************/
/* Function:  Add a method to the directory method table.                     */
/******************************************************************************/
{
    /* get as a string parameter         */
    entryname = stringArgument(entryname, ARG_ONE)->upper();
    if (methodobj != OREF_NULL)          /* have a method object?             */
    {
        // not given as a method object already?  Could be a string
        // or array.
        if (!isOfClass(Method, methodobj))
        {
            // create a new method and set the scope
            methodobj = MethodClass::newMethodObject(entryname, methodobj, IntegerTwo);
            methodobj->setScope((RexxClass *)this);
        }
        else
        {
            // might return a new method if this has a scope already.
            methodobj = methodobj->newScope((RexxClass *)this);
        }
        /* the unknown method?               */
        if (entryname->strCompare(CHAR_UNKNOWN))
        {
            /* stash this is a special place     */
            setField(unknown_method, methodobj);
        }
        else
        {
            /* no table yet?                     */
            if (this->method_table == OREF_NULL)
            {
                /* create one                        */
                setField(method_table, new_table());
            }
            /* now add the method                */
            this->method_table->stringPut(methodobj, entryname);
        }
    }
    else
    {
        /* the unknown method?               */
        if (entryname->strCompare(CHAR_UNKNOWN))
        {
            /* cast off the unknown method       */
            OrefSet(this, this->unknown_method, OREF_NULL);
        }
        else
        {
            /* if we have a table                */
            if (this->method_table != OREF_NULL)
            {
                /* remove this entry                 */
                this->method_table->remove(entryname);
            }
        }
    }
    this->contents->remove(entryname);   /* remove any table entry            */
    return OREF_NULL;                    /* this always returns nothing       */
}

RexxObject *DirectoryClass::mergeItem(
    RexxObject *_value,                /* value to add                      */
    RexxObject *_index)                /* index to use                      */
/******************************************************************************/
/* Function:  Merge a single item during merge processing                     */
/******************************************************************************/
{
                                       /* just add the item                 */
    return this->put(_value, (RexxString *)_index);
}

RexxObject *DirectoryClass::at(
    RexxString *_index)                /* index to retrieve                 */
/******************************************************************************/
/* Function:  Retrieve an item from a directory                               */
/******************************************************************************/
{
    /* try to retrieve the value         */
    RexxObject *result = this->contents->stringGet(_index);
    if (result == OREF_NULL)
    {           /* no value?                         */
                /* have a table?                     */
        if (this->method_table != OREF_NULL)
        {
            /* look for a method                 */
            MethodClass *method = (MethodClass *)this->method_table->stringGet(_index);
            if (method != OREF_NULL)         /* have a method?                    */
            {
                ProtectedObject v;
                /* run the method                    */
                method->run(ActivityManager::currentActivity, this, _index, NULL, 0, v);
                return(RexxObject *)v;

            }
        }
        /* got an unknown method?            */
        if (this->unknown_method != OREF_NULL)
        {
            RexxObject *arg = _index;
            ProtectedObject v;
            /* run it                            */
            this->unknown_method->run(ActivityManager::currentActivity, this, OREF_UNKNOWN, (RexxObject **)&arg, 1, v);
            return(RexxObject *)v;
        }
    }
    return result;                       /* return a result                   */
}

RexxObject *DirectoryClass::atRexx(
    RexxString *_index)                 /* index to retrieve                 */
/******************************************************************************/
/* Function:     This is the REXX version of at.  It issues a                 */
/*               STRINGREQUEST message to the index parameter if it isn't     */
/*               already a string or a name object.  Thus, this may raise     */
/*               NOSTRING.                                                    */
/******************************************************************************/
{
    RexxObject *temp;                    /* Temporary holder for return value */

                                         /* get as a string parameter         */
    _index = stringArgument(_index, ARG_ONE);
    // is this the .local object?  We'll need to check with the security manager
    if (ActivityManager::getLocal() == this)
    {
        SecurityManager *manager = ActivityManager::currentActivity->getEffectiveSecurityManager();
        temp = manager->checkLocalAccess(_index);
        if (temp != OREF_NULL)
        {
            return temp;
        }
    }
    temp = this->at(_index);             /* run real AT                       */
                                         /* if we found nothing or the method */
    if (temp == OREF_NULL)               /* we ran returned nothing           */
    {
        temp = TheNilObject;               /* return TheNilObject as a default  */
    }
    return temp;                         /* return the value                  */
}

RexxObject *DirectoryClass::put(
    RexxObject *_value,                /* value to add                      */
    RexxString *_index)                /* string index of the value         */
/******************************************************************************/
/* Function:  Add an item to a directory                                      */
/******************************************************************************/
{
    /* get as a string parameter         */
    _index = stringArgument(_index, ARG_TWO);
    if (this->method_table != OREF_NULL) /* have a table?                     */
    {
        this->method_table->remove(_index);/* remove any method                 */
    }
                                           /* try to place in existing hashtab  */
    RexxHashTable *newHash = this->contents->stringPut(_value, _index);
    if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
    {
        /* hook on the new hash table        */
        OrefSet(this, this->contents, newHash);
    }
    return OREF_NULL;                    /* this returns nothing              */
}

void DirectoryClass::reset()
/******************************************************************************/
/* Function:  Reset a directory to a "pristine" empty state                   */
/******************************************************************************/
{
    // empty the hashtables without reallocating.
    contents->empty();
    if (method_table != OREF_NULL)
    {
        method_table->empty();
    }
    // clear out the unknown method.
    OrefSet(this, this->unknown_method, OREF_NULL);
}


/**
 * Empty a hash table collection.
 *
 * @return nothing
 */
RexxObject *DirectoryClass::empty()
{
    reset();
    return OREF_NULL;
}


/**
 * Test if a HashTableCollection is empty.
 *
 * @return
 */
bool DirectoryClass::isEmpty()
{
    return items() == 0;
}


/**
 * Test if a HashTableCollection is empty.
 *
 * @return
 */
RexxObject *DirectoryClass::isEmptyRexx()
{
    return items() == 0 ? TheTrueObject : TheFalseObject;
}



/**
 * Retrieve an index for a given item.  Which index is returned
 * is indeterminate.
 *
 * @param target The target object.
 *
 * @return The index for the target object, or .nil if no object was
 *         found.
 */
RexxObject *DirectoryClass::indexRexx(RexxObject *target)
{
    // required argument
    requiredArgument(target, ARG_ONE);
    // retrieve this from the hash table
    RexxObject *result = this->contents->getIndex(target);
    // not found, return .nil
    if (result == OREF_NULL)
    {
        // rats, we might need to do this the hardway
        if (this->method_table != OREF_NULL)
        {
            TableClass *methodTable = this->method_table;

            for (HashLink i = methodTable->first(); methodTable->available(i); i = methodTable->next(i))
            {
                // we need to run each method, looking for a value that matches
                RexxString *name = (RexxString *)methodTable->index(i);
                MethodClass *method = (MethodClass *)methodTable->value(i);
                ProtectedObject v;
                method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);
                // got a match?
                if (target->equalValue((RexxObject *)v))
                {
                    return name;    // the name is the index
                }
            }
        }
        return TheNilObject;          // the nil object is the unknown index
    }
    return result;
}


/**
 * Test if a given item exists in the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxObject *DirectoryClass::hasItem(RexxObject *target)
{
    requiredArgument(target, ARG_ONE);
    // the lookup is more complicated, so just delegate to the index lookup code.
    return indexRexx(target) != TheNilObject ? TheTrueObject : TheFalseObject;
}


/**
 * Remove a given item from the collection.
 *
 * @param target The target object.
 *
 * @return .true if the object exists, .false otherwise.
 */
RexxObject *DirectoryClass::removeItem(RexxObject *target)
{
    requiredArgument(target, ARG_ONE);
    // the lookup is more complicated, so just delegate to the index lookup code.
    RexxObject *i = indexRexx(target);
    // just use the retrieved index to remove.
    if (i != TheNilObject)
    {
        return remove((RexxString *)i);
    }
    return TheNilObject;     // nothing removed.
}


/**
 * Create a directory object from Rexx code.
 *
 * @param init_args The arguments to the NEW method.
 * @param argCount  The argument count.
 *
 * @return A new directory object.
 */
RexxObject *DirectoryClass::newRexx(RexxObject **init_args, size_t       argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    DirectoryClass *newDirectory = new_directory();
    ProtectedObject p(newDirectory);

    // handle Rexx class completion
    classThis->completeNewObject(newDirectory, init_args, argCount);

    return newDirectory;                 /* return the new directory          */
}


DirectoryClass *DirectoryClass::newInstance()
/******************************************************************************/
/* Create a new directory item                                                */
/******************************************************************************/
{
                                       /* get a new object and hash         */
    return (DirectoryClass *)new_hashCollection(RexxHashTable::DEFAULT_HASH_SIZE, sizeof(DirectoryClass), T_Directory);
}


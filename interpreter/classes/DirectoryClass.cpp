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

// singleton class instance
RexxClass *RexxDirectory::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxDirectory::createInstance()
{
    CLASS_CREATE(Directory, "Directory", RexxClass);
}

void RexxDirectory::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    this->RexxHashTableCollection::live(liveMark);
    memory_mark(this->method_table);
    memory_mark(this->unknown_method);
}

void RexxDirectory::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    this->RexxHashTableCollection::liveGeneral(reason);
    memory_mark_general(this->method_table);
    memory_mark_general(this->unknown_method);
}

void RexxDirectory::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxDirectory)

      flatten_reference(newThis->contents, envelope);
      flatten_reference(newThis->method_table, envelope);
      flatten_reference(newThis->unknown_method, envelope);
      flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

RexxObject *RexxDirectory::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
    envelope->addTable(this);            /* add to the envelope table         */
    return this;                         /* and just return out selves        */
}

RexxObject *RexxDirectory::copy()
/******************************************************************************/
/* Function:  Copy a directory                                                */
/******************************************************************************/
{
                                       /* copy object via Collection copy   */
    RexxDirectory *newObj = (RexxDirectory *)this->RexxHashTableCollection::copy();
                                       /* No specifics for Directory.       */
    if (this->method_table != OREF_NULL)
    {
                                       /* copy it too                       */
        OrefSet(newObj, newObj->method_table, (RexxTable *)this->method_table->copy());
    }
    return newObj;                       /* return the copy                   */
}

RexxObject *RexxDirectory::entry(
    RexxString *entryName)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Retrieve an entry from a directory...with upper casing          */
/*                                                                            */
/*  Returned:  The entry, or the result of running the method                 */
/******************************************************************************/
{
    entryName = entryName->upper();      /* force to uppercase                */
    return this->at(entryName);          /* just return the "at" form         */
}

size_t RexxDirectory::items(void)
/******************************************************************************/
/* Function:  Return the count of items in the directory, including the       */
/*            number of methods added via set method calls                    */
/******************************************************************************/
{
    /* get the direct table size         */
    size_t count = this->contents->totalEntries();
    /* have a method table?              */
    if (this->method_table != OREF_NULL)
    {
        /* add in the count of methods       */
        count += this->method_table->items();
    }
    return count;                        /* return this amount                */
}

RexxObject *RexxDirectory::itemsRexx(void)
/******************************************************************************/
/* Function:  Return the count of items in the directory, including the       */
/*            number of methods added via set method calls                    */
/******************************************************************************/
{
    return (RexxObject *)new_integer(this->items());
}

RexxSupplier *RexxDirectory::supplier(void)
/******************************************************************************/
/* Function:  Create a supplier for a directory, including the results of all */
/*            of the SETMETHOD methods as values                              */
/******************************************************************************/
{
    RexxTable *result = new_table();     /* get a table for the supplier      */
    ProtectedObject p(result);
    RexxHashTable *hashTab = this->contents;  /* point to the contents             */
    /* now traverse the entire table     */
    for (HashLink i = hashTab->first(); hashTab->index(i) != OREF_NULL; i = hashTab->next(i))
    {
        /* get the directory index           */
        RexxString *name = (RexxString *)hashTab->index(i);
        /* add to the table                  */
        result->put(hashTab->value(i), name);
    }
    /* have a method table?              */
    if (this->method_table != OREF_NULL)
    {
        RexxTable *methodTable = this->method_table;
        /* need to extract method values     */
        for (HashLink i = methodTable->first(); methodTable->available(i); i = methodTable->next(i))
        {
            /* get the directory index           */
            RexxString *name = (RexxString *)methodTable->index(i);
            /* get the method                    */
            RexxMethod *method = (RexxMethod *)methodTable->value(i);
            ProtectedObject v;
            /* run the method                    */
            method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);
            result->put((RexxObject *)v, name);  /* add to the table                  */
        }
    }
    return result->supplier();           /* convert this to a supplier        */
}

RexxArray *RexxDirectory::requestArray()
/******************************************************************************/
/* Function:  Primitive level request('ARRAY') fast path                      */
/******************************************************************************/
{
    if (isOfClass(Directory, this))          /* primitive level object?           */
    {
        return this->makeArray();          /* just do the makearray             */
    }
    else                                 /* need to so full request mechanism */
    {
        return (RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
    }
}

RexxArray *RexxDirectory::makeArray(void)
/******************************************************************************/
/* Function:  Create an array of all of the directory indices, including those*/
/*            of all the SETMETHOD methods.                                   */
/******************************************************************************/
{
    return this->allIndexes();
}


/**
 * Create an array of all of the directory indices, including those
 * of all the SETMETHOD methods.
 *
 * @return An array containing all of the directory indices.
 */
RexxArray *RexxDirectory::allIndexes(void)
{
    // get a result array of the appropriate size
    wholenumber_t count = this->items();
    RexxArray *result = (RexxArray *)new_array(count);
    ProtectedObject p(result);
    size_t out = 1;
    // we're working directly off of the contents.
    RexxHashTable *hashTab = this->contents;

    // traverse the entire table coping over the items.
    for (HashLink i = hashTab->first(); hashTab->index(i) != OREF_NULL; i = hashTab->next(i))
    {
        RexxString *name = (RexxString *)hashTab->index(i);
        result->put(name, out++);
    }
    // if e hae amethod table, we need to copy those indices also
    if (this->method_table != OREF_NULL)
    {
        RexxTable *methodTable = this->method_table;
        for (HashLink i = methodTable->first(); methodTable->available(i); i = methodTable->next(i))
        {
           RexxString *name = (RexxString *)methodTable->index(i);
           result->put(name, out++);
        }
    }
    return result;                       /* send back the array               */
}

RexxArray *RexxDirectory::allItems()
/******************************************************************************/
/* Function:  Create an array of all of the directory values, including the   */
/*            values of all the SETMETHOD methods                             */
/******************************************************************************/
{
    size_t count = this->items();        /* get the array size                */
                                         /* get result array of correct size  */
    RexxArray *result = (RexxArray *)new_array(count);
    ProtectedObject p(result);
    size_t i = 1;                        /* position in array                 */
    RexxHashTable *hashTab = this->contents;
    /* now traverse the entire table     */
    for (HashLink j = hashTab->first(); hashTab->index(j) != OREF_NULL; j = hashTab->next(j))
    {
        /* add to the array                  */
        result->put(hashTab->value(j), i++);
    }
    /* have a method table?              */
    if (this->method_table != OREF_NULL)
    {
        RexxTable *methodTable = this->method_table;  /* grab the table                    */
        /* need to extract method values     */
        for (HashLink j = methodTable->first(); methodTable->available(j); j = methodTable->next(j))
        {
            /* get the directory index           */
            RexxString *name = (RexxString *)methodTable->index(j);
            /* need to extract method values     */
            RexxMethod *method = (RexxMethod *)methodTable->value(j);
            ProtectedObject v;
            /* run the method                    */
            method->run(ActivityManager::currentActivity, this, name, NULL, 0, v);
            result->put((RexxObject *)v, i++);  /* add to the array                  */
        }
    }
    return result;                       /* send back the array               */
}

RexxObject *RexxDirectory::entryRexx(
    RexxString *entryName)             /* name to retrieve                  */
/******************************************************************************/
/* Function:     This is the REXX version of entry.  It issues a STRINGREQUEST*/
/*               message to the entryname parameter if it isn't already a     */
/*               string or a name object.  Thus, this may raise NOSTRING.     */
/******************************************************************************/
{
                                         /* get a string parameter (uppercase)*/
    entryName = stringArgument(entryName, ARG_ONE)->upper();
    RexxObject *temp = this->at(entryName);          /* retrieve the name                 */

                                         /* if we found nothing or the method */
    if (temp == OREF_NULL)               /* we ran returned nothing,          */
    {
        temp = TheNilObject;               /* return TheNilObject as a default  */
    }
    return temp;                         /* return the value                  */
}

RexxObject *RexxDirectory::hasIndex(
    RexxString *indexName)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Determine if the directory has an entry with this name (used    */
/*            without uppercasing)                                            */
/******************************************************************************/
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
            RexxMethod *method = (RexxMethod *)this->method_table->stringGet(indexName);
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

RexxObject *RexxDirectory::hasEntry(
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
            RexxMethod *method = (RexxMethod *)this->method_table->stringGet(entryName);
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

RexxObject *RexxDirectory::setEntry(
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
RexxObject *RexxDirectory::removeRexx(RexxString *entryname)
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

RexxObject *RexxDirectory::remove(
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

RexxObject *RexxDirectory::unknown(
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

RexxObject *RexxDirectory::setMethod(
    RexxString *entryname,             /* name to set this under            */
    RexxMethod *methodobj)             /* new method argument               */
/******************************************************************************/
/* Function:  Add a method to the directory method table.                     */
/******************************************************************************/
{
    /* get as a string parameter         */
    entryname = stringArgument(entryname, ARG_ONE)->upper();
    if (methodobj != OREF_NULL)          /* have a method object?             */
    {
        if (!isOfClass(Method, methodobj))     /* given as a string?                */
        {
            /* convert to a method               */
            methodobj = RexxMethod::newMethodObject(entryname, methodobj, IntegerTwo, OREF_NULL);
            /* set a new scope on this           */
            methodobj->setScope((RexxClass *)this);
        }
        else
        {
            /* set a new scope on this           */
            methodobj = methodobj->newScope((RexxClass *)this);
        }
        /* the unknown method?               */
        if (entryname->strCompare(CHAR_UNKNOWN))
        {
            /* stash this is a special place     */
            OrefSet(this, this->unknown_method, methodobj);
        }
        else
        {
            /* no table yet?                     */
            if (this->method_table == OREF_NULL)
            {
                /* create one                        */
                OrefSet(this, this->method_table, new_table());
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

RexxObject *RexxDirectory::mergeItem(
    RexxObject *_value,                /* value to add                      */
    RexxObject *_index)                /* index to use                      */
/******************************************************************************/
/* Function:  Merge a single item during merge processing                     */
/******************************************************************************/
{
                                       /* just add the item                 */
    return this->put(_value, (RexxString *)_index);
}

RexxObject *RexxDirectory::at(
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
            RexxMethod *method = (RexxMethod *)this->method_table->stringGet(_index);
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

RexxObject *RexxDirectory::atRexx(
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

RexxObject *RexxDirectory::put(
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

void RexxDirectory::reset(void)
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
RexxObject *RexxDirectory::empty()
{
    reset();
    return OREF_NULL;
}


/**
 * Test if a HashTableCollection is empty.
 *
 * @return
 */
RexxObject *RexxDirectory::isEmpty()
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
RexxObject *RexxDirectory::indexRexx(RexxObject *target)
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
            RexxTable *methodTable = this->method_table;

            for (HashLink i = methodTable->first(); methodTable->available(i); i = methodTable->next(i))
            {
                // we need to run each method, looking for a value that matches
                RexxString *name = (RexxString *)methodTable->index(i);
                RexxMethod *method = (RexxMethod *)methodTable->value(i);
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
RexxObject *RexxDirectory::hasItem(RexxObject *target)
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
RexxObject *RexxDirectory::removeItem(RexxObject *target)
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

RexxObject *RexxDirectory::newRexx(
    RexxObject **init_args,
    size_t       argCount)
/******************************************************************************/
/* Function:  Create a new directory for a REXX program                       */
/******************************************************************************/
{
    /* get a new directory               */
    /* NOTE:  this does not use the      */
    /* macro version because the class   */
    /* object might actually be for a    */
    /* subclass                          */
    RexxDirectory *newDirectory = new_directory();
    newDirectory->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    /* does object have an UNINT method  */
    if (((RexxClass *)this)->hasUninitDefined())
    {
        newDirectory->hasUninit();         /* Make sure everyone is notified.   */
    }
    /* call any rexx level init's        */
    newDirectory->sendMessage(OREF_INIT, init_args, argCount);
    return newDirectory;                 /* return the new directory          */
}

RexxDirectory *RexxDirectory::newInstance()
/******************************************************************************/
/* Create a new directory item                                                */
/******************************************************************************/
{
                                       /* get a new object and hash         */
    return (RexxDirectory *)new_hashCollection(RexxHashTable::DEFAULT_HASH_SIZE, sizeof(RexxDirectory), T_Directory);
}


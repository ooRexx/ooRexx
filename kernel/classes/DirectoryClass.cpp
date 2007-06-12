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
/* REXX Kernel                                                  DirectoryClass.c    */
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

void RexxDirectory::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  this->RexxHashTableCollection::live();
  setUpMemoryMark
  memory_mark(this->method_table);
  memory_mark(this->unknown_method);
  cleanUpMemoryMark
}

void RexxDirectory::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  this->RexxHashTableCollection::liveGeneral();
  setUpMemoryMarkGeneral
  memory_mark_general(this->method_table);
  memory_mark_general(this->unknown_method);
  cleanUpMemoryMarkGeneral
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
  RexxDirectory *newObject;            /* new directory copy                */

                                       /* copy object via Collection copy   */
  newObject = (RexxDirectory *)this->RexxHashTableCollection::copy();
                                       /* No specifics for Directory.       */
  if (this->method_table != OREF_NULL) {
                                       /* copy it too                       */
    OrefSet(newObject, newObject->method_table, (RexxTable *)this->method_table->copy());
  }
  return newObject;                    /* return the copy                   */
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

long RexxDirectory::items(void)
/******************************************************************************/
/* Function:  Return the count of items in the directory, including the       */
/*            number of methods added via set method calls                    */
/******************************************************************************/
{
  LONG    count;                       /* count of items                    */

                                       /* get the direct table size         */
  count = this->contents->totalEntries();
                                       /* have a method table?              */
  if (this->method_table != OREF_NULL) {
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
  long tempCount;                      /* temporary count                   */

  tempCount = this->items();           /* return the count as an object     */
  return (RexxObject *)new_integer(tempCount);
}

RexxSupplier *RexxDirectory::supplier(void)
/******************************************************************************/
/* Function:  Create a supplier for a directory, including the results of all */
/*            of the SETMETHOD methods as values                              */
/******************************************************************************/
{
  RexxTable  *result;                  /* Constructed results table         */
  RexxTable  *methodTable;             /* constructed method table          */
  RexxHashTable *hashTab;              /* contents hash table               */
  RexxString *name;                    /* table index                       */
  RexxMethod *method;                  /* method to run                     */
  RexxObject *value;                   /* method run value                  */
  LONG    index;                       /* table index                       */

  result = new_table();                /* get a table for the supplier      */
  save(result);                        /* protect this                      */
  hashTab = this->contents;            /* point to the contents             */
                                       /* now traverse the entire table     */
  for (index = hashTab->first(); hashTab->index(index) != OREF_NULL; index = hashTab->next(index)) {
                                       /* get the directory index           */
    name = (RexxString *)hashTab->index(index);
                                       /* add to the table                  */
    result->put(hashTab->value(index), name);
  }
                                       /* have a method table?              */
  if (this->method_table != OREF_NULL) {
    methodTable = this->method_table;
                                       /* need to extract method values     */
    for (index = methodTable->first(); methodTable->available(index); index = methodTable->next(index)) {
                                       /* get the directory index           */
      name = (RexxString *)methodTable->index(index);
                                       /* get the method                    */
      method = (RexxMethod *)methodTable->value(index);
                                       /* run the method                    */
      value = method->run(CurrentActivity, this, name, 0, NULL);
      result->put(value, name);        /* add to the table                  */
    }
  }
  discard_hold(result);                /* unlock the result                 */
  return result->supplier();           /* convert this to a supplier        */
}

RexxArray *RexxDirectory::requestArray()
/******************************************************************************/
/* Function:  Primitive level request('ARRAY') fast path                      */
/******************************************************************************/
{
  if (OTYPE(Directory, this))          /* primitive level object?           */
    return this->makeArray();          /* just do the makearray             */
  else                                 /* need to so full request mechanism */
    return (RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
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
    save(result);
    arraysize_t i = 1;
    // we're working directly off of the contents.
    RexxHashTable *hashTab = this->contents;

    // traverse the entire table coping over the items.
    for (HashLink index = hashTab->first(); hashTab->index(index) != OREF_NULL; index = hashTab->next(index))
    {
        RexxString *name = (RexxString *)hashTab->index(index);
        result->put(name, i++);
    }
    // if e hae amethod table, we need to copy those indices also
    if (this->method_table != OREF_NULL)
    {
        RexxTable *methodTable = this->method_table;
        for (HashLink index = methodTable->first(); methodTable->available(index); index = methodTable->next(index))
        {
           RexxString *name = (RexxString *)methodTable->index(index);
           result->put(name, i++);
        }
    }
    discard_hold(result);
    return result;                       /* send back the array               */
}

RexxArray *RexxDirectory::allItems()
/******************************************************************************/
/* Function:  Create an array of all of the directory values, including the   */
/*            values of all the SETMETHOD methods                             */
/******************************************************************************/
{
  LONG    count;                       /* count of items in the directory   */
  LONG    i;                           /* loop counter                      */
  LONG    index;                       /* table index                       */
  RexxArray *result;                   /* returned result                   */
  RexxHashTable *hashTab;              /* contents hash table               */
  RexxTable *methodTable;              /* contents method table             */
  RexxString *name;                    /* table index                       */
  RexxMethod *method;                  /* method to run                     */
  RexxObject *value;                   /* value added                       */

                                       /* return the count as an object     */
  count = this->items();               /* get the array size                */
                                       /* get result array of correct size  */
  result = (RexxArray *)new_array(count);
  save(result);                        /* protect this                      */
  i = 1;                               /* position in array                 */
  hashTab = this->contents;
                                       /* now traverse the entire table     */
  for (index = hashTab->first(); hashTab->index(index) != OREF_NULL; index = hashTab->next(index)) {
                                       /* get the directory index           */
    name = (RexxString *)hashTab->index(index);
                                       /* add to the array                  */
    result->put(hashTab->value(index), i++);
  }
                                       /* have a method table?              */
  if (this->method_table != OREF_NULL) {
    methodTable = this->method_table;  /* grab the table                    */
                                       /* need to extract method values     */
    for (index = methodTable->first(); methodTable->available(index); index = methodTable->next(index)) {
                                       /* get the directory index           */
      name = (RexxString *)methodTable->index(index);
                                       /* need to extract method values     */
      method = (RexxMethod *)methodTable->value(index);
                                       /* run the method                    */
      value = method->run(CurrentActivity, this, name, 0, NULL);
      result->put(name, i++);          /* add to the array                  */
    }
  }
  discard_hold(result);                /* unlock the result                 */
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
  RexxObject *temp;                    /* Temporary holder for return value */

                                       /* get a string parameter (uppercase)*/
  entryName = REQUIRED_STRING(entryName, ARG_ONE)->upper();
  temp = this->at(entryName);          /* retrieve the name                 */

                                       /* if we found nothing or the method */
  if (temp == OREF_NULL)               /* we ran returned nothing,          */
    temp = TheNilObject;               /* return TheNilObject as a default  */
  return temp;                         /* return the value                  */
}

RexxObject *RexxDirectory::hasIndex(
    RexxString *indexName)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Determine if the directory has an entry with this name (used    */
/*            without uppercasing)                                            */
/******************************************************************************/
{
  RexxMethod *method;                  /* retrieved method                  */

                                       /* get as a string parameter         */
  indexName = REQUIRED_STRING(indexName, ARG_ONE);
                                       /* got a value?                      */
  if (this->contents->stringGet(indexName) != OREF_NULL)
    return (RexxObject *)TheTrueObject;/* return TRUE                       */
  else {
                                       /* have a table?                     */
    if (this->method_table != OREF_NULL) {
                                       /* look for a method                 */
      method = (RexxMethod *)this->method_table->stringGet(indexName);
      if (method != OREF_NULL)         /* have a method?                    */
                                       /* then we have the index            */
        return (RexxObject *)TheTrueObject;
    }
                                       /* not in the directory              */
    return (RexxObject *)TheFalseObject;
  }
}

RexxObject *RexxDirectory::hasEntry(
    RexxString *entryName)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Determine if an entry exists in the directory (name will be     */
/*            upper cased)                                                    */
/******************************************************************************/
{
  RexxMethod *method;                  /* retrieved method name             */

                                       /* get as a string parameter         */
  entryName = REQUIRED_STRING(entryName, ARG_ONE)->upper();
                                       /* in the table?                     */
  if (this->contents->stringGet(entryName) != OREF_NULL)
    return (RexxObject *)TheTrueObject;/* this is true                      */
  else {
                                       /* have a table?                     */
    if (this->method_table != OREF_NULL) {
                                       /* look for a method                 */
      method = (RexxMethod *)this->method_table->stringGet(entryName);
      if (method != OREF_NULL)         /* have a method?                    */
                                       /* then we have the index            */
        return (RexxObject *)TheTrueObject;
    }
                                       /* not in the directory              */
    return (RexxObject *)TheFalseObject;
  }
}

RexxObject *RexxDirectory::setEntry(
  RexxString *entryname,               /* directory entry name              */
  RexxObject *entryobj)                /* associated object                 */
/******************************************************************************/
/* Function:  Add an entry to the directory (under an upper case name)        */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* new hash tab                      */

                                       /* get as a string parameter         */
  entryname = REQUIRED_STRING(entryname, ARG_ONE)->upper();
  if (entryobj != OREF_NULL) {         /* have a new value?                 */
                                       /* try to place in existing hashtab  */
    newHash = this->contents->stringPut(entryobj, entryname);
    if (newHash  != OREF_NULL)         /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
      OrefSet(this, this->contents, newHash);
    if (this->method_table != OREF_NULL)
      this->method_table->remove(entryname);
  }
  else
    this->remove(entryname);           /* remove this entry                 */
  return OREF_NULL;                    /* don't return a value              */
}

RexxObject *RexxDirectory::remove(
    RexxString *entryname)             /* name to retrieve                  */
/******************************************************************************/
/* Function:  Remove an entry from a directory.                               */
/******************************************************************************/
{
  RexxObject *value;                   /* returned value                    */

                                       /* get as a string parameter         */
  entryname = REQUIRED_STRING(entryname, ARG_ONE);
  value = this->at(entryname);         /* go get the directory value        */
  if (value == OREF_NULL)              /* nothing to return?                */
    value = TheNilObject;              /* return TheNilObject as a default  */
                                       /* have a real entry?                */
  if (this->contents->stringGet(entryname) != OREF_NULL)
    this->contents->remove(entryname); /* remove the entry                  */
  /* if there's a method entry, remove that one too! */
//  else {                               /* may need to remove a method       */
                                       /* have methods?                     */
    if (this->method_table != OREF_NULL)
                                       /* remove this method                */
      this->method_table->remove(entryname->upper());
//  }
  return value;                        /* return the directory value        */
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
  RexxString * message_value;          /* value of the message name         */
  LONG         message_length;         /* length of the message name        */

                                       /* validate the name                 */
  message_value = REQUIRED_STRING(msgname, ARG_ONE);
  required_arg(arguments, TWO);        /* need an argument array            */
                                       /* get the length                    */
  message_length = message_value->length;
                                       /* assignment form of access?        */
  if (message_length > 0 && message_value->getChar(message_length - 1) == '=') {
                                       /* get this as an array              */
    arguments = (RexxArray  *)REQUEST_ARRAY(arguments);
                                       /* not an array item or a multiple   */
                                       /* dimension one, or more than one   */
                                       /* argument (CHM)                    */
    if (arguments == TheNilObject || arguments->getDimension() != 1 ||
        arguments->size() != 1 )
                                       /* raise an error                    */
      report_exception1(Error_Incorrect_method_noarray, IntegerTwo);
                                       /* extract the name part of the msg  */
    message_value = (RexxString *)message_value->extract(0, message_length - 1);
                                       /* do this as an assignment          */
    return this->setEntry(message_value, arguments->get(1));
  }
  else                                 /* just a lookup form                */
    return this->entryRexx(message_value);
}

RexxObject *RexxDirectory::setMethod(
    RexxString *entryname,             /* name to set this under            */
    RexxMethod *methodobj)             /* new method argument               */
/******************************************************************************/
/* Function:  Add a method to the directory method table.                     */
/******************************************************************************/
{
                                       /* get as a string parameter         */
  entryname = REQUIRED_STRING(entryname, ARG_ONE)->upper();
  if (methodobj != OREF_NULL) {        /* have a method object?             */
    if (!OTYPE(Method, methodobj)) {   /* given as a string?                */
                                       /* convert to a method               */
      methodobj = TheMethodClass->newRexxCode(entryname, methodobj, IntegerTwo, OREF_NULL);
                                       /* set a new scope on this           */
      methodobj->setScope((RexxClass *)this);
    }
    else
                                       /* set a new scope on this           */
      methodobj = methodobj->newScope((RexxClass *)this);
                                       /* the unknown method?               */
    if (entryname->strCompare(CHAR_UNKNOWN)) {
                                       /* stash this is a special place     */
      OrefSet(this, this->unknown_method, methodobj);
    }
    else {
                                       /* no table yet?                     */
      if (this->method_table == OREF_NULL) {
                                       /* create one                        */
        OrefSet(this, this->method_table, new_table());
      }
                                       /* now add the method                */
      this->method_table->stringPut(methodobj, entryname);
//      this->contents->remove(entryname); // remove entry in contents!
    }
  }
  else {
                                       /* the unknown method?               */
    if (entryname->strCompare(CHAR_UNKNOWN)) {
                                       /* cast off the unknown method       */
      OrefSet(this, this->unknown_method, OREF_NULL);
    }
    else {
                                       /* if we have a table                */
      if (this->method_table != OREF_NULL)
                                       /* remove this entry                 */
        this->method_table->remove(entryname);
    }
  }
  this->contents->remove(entryname);   /* remove any table entry            */
  return OREF_NULL;                    /* this always returns nothing       */
}

RexxObject *RexxDirectory::mergeItem(
    RexxObject *value,                 /* value to add                      */
    RexxObject *index)                 /* index to use                      */
/******************************************************************************/
/* Function:  Merge a single item during merge processing                     */
/******************************************************************************/
{
                                       /* just add the item                 */
  return this->put(value, (RexxString *)index);
}

RexxObject *RexxDirectory::at(
    RexxString *index)                 /* index to retrieve                 */
/******************************************************************************/
/* Function:  Retrieve an item from a directory                               */
/******************************************************************************/
{
  RexxObject *result;                  /* returned result                   */
  RexxMethod *method;                  /* method to run                     */

                                       /* try to retrieve the value         */
  result = this->contents->stringGet(index);
  if (result == OREF_NULL) {           /* no value?                         */
                                       /* have a table?                     */
    if (this->method_table != OREF_NULL) {
                                       /* look for a method                 */
      method = (RexxMethod *)this->method_table->stringGet(index);
      if (method != OREF_NULL)         /* have a method?                    */
                                       /* run the method                    */
        return method->run(CurrentActivity, this, index, 0, NULL);
    }
                                       /* got an unknown method?            */
    if (this->unknown_method != OREF_NULL)
                                       /* run it                            */
      return this->unknown_method->run(CurrentActivity, this, OREF_UNKNOWN, 1, (RexxObject **)&index);
  }
  return result;                       /* return a result                   */
}

RexxObject *RexxDirectory::atRexx(
    RexxString *index)                 /* index to retrieve                 */
/******************************************************************************/
/* Function:     This is the REXX version of at.  It issues a                 */
/*               STRINGREQUEST message to the index parameter if it isn't     */
/*               already a string or a name object.  Thus, this may raise     */
/*               NOSTRING.                                                    */
/******************************************************************************/
{
  RexxObject *temp;                    /* Temporary holder for return value */

                                       /* get as a string parameter         */
  index = REQUIRED_STRING(index, ARG_ONE);
  // is this the .local object?
  if ((RexxDirectory *)(CurrentActivity->local) == this &&
      CurrentActivity->currentActivation->hasSecurityManager()) {
    RexxDirectory *securityArgs;       /* security check arguments          */
    securityArgs = new_directory();
    securityArgs->put(index, OREF_NAME);
    securityArgs->put(TheNilObject, OREF_RESULT);
    if (CurrentActivity->currentActivation->callSecurityManager(OREF_LOCAL, securityArgs))
                                       /* get the result and return         */
      return securityArgs->fastAt(OREF_RESULT);
  }
  temp = this->at(index);              /* run real AT                       */
                                       /* if we found nothing or the method */
  if (temp == OREF_NULL)               /* we ran returned nothing           */
    temp = TheNilObject;               /* return TheNilObject as a default  */
  return temp;                         /* return the value                  */
}

RexxObject *RexxDirectory::put(
    RexxObject *value,                 /* value to add                      */
    RexxString *index)                 /* string index of the value         */
/******************************************************************************/
/* Function:  Add an item to a directory                                      */
/******************************************************************************/
{
  RexxHashTable *newHash;              /* newly created hash table          */

                                       /* get as a string parameter         */
  index = REQUIRED_STRING(index, ARG_TWO);
  if (this->method_table != OREF_NULL) /* have a table?                     */
    this->method_table->remove(index); /* remove any method                 */
                                       /* try to place in existing hashtab  */
  newHash = this->contents->stringPut(value, index);
  if (newHash  != OREF_NULL)           /* have a reallocation occur?        */
                                       /* hook on the new hash table        */
    OrefSet(this, this->contents, newHash);
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
    required_arg(target, ONE);
    // retrieve this from the hash table
    RexxObject *result = this->contents->getIndex(target);
    // not found, return .nil
    if (result == OREF_NULL)
    {
        // rats, we might need to do this the hardway
        if (this->method_table != OREF_NULL)
        {
            RexxTable *methodTable = this->method_table;

            for (HashLink index = methodTable->first(); methodTable->available(index); index = methodTable->next(index))
            {
                // we need to run each method, looking for a value that matches
                RexxString *name = (RexxString *)methodTable->index(index);
                RexxMethod *method = (RexxMethod *)methodTable->value(index);
                RexxObject *value = method->run(CurrentActivity, this, name, 0, NULL);
                // got a match?
                if (target->equalValue(value))
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
    required_arg(target, ONE);
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
    required_arg(target, ONE);
    // the lookup is more complicated, so just delegate to the index lookup code.
    RexxObject *index = indexRexx(target);
    // just use the retrieved index to remove.
    if (index != TheNilObject)
    {
        return remove((RexxString *)index);
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
  RexxDirectory *newDirectory;         /* new directory item                */

                                       /* get a new directory               */
                                       /* NOTE:  this does not use the      */
                                       /* macro version because the class   */
                                       /* object might actually be for a    */
                                       /* subclass                          */
  newDirectory = new_directory();
  BehaviourSet(newDirectory, ((RexxClass *)this)->instanceBehaviour);
                                       /* does object have an UNINT method  */
  if (((RexxClass *)this)->uninitDefined()) {
    newDirectory->hasUninit();         /* Make sure everyone is notified.   */
  }
                                       /* call any rexx level init's        */
  newDirectory->sendMessage(OREF_INIT, init_args, argCount);
  return newDirectory;                 /* return the new directory          */
}

RexxDirectory *RexxMemory::newDirectory()
/******************************************************************************/
/* Create a new directory item                                                */
/******************************************************************************/
{
  RexxDirectory *newDirectory;         /* new directory object              */

                                       /* get a new object and hash         */
  newDirectory = (RexxDirectory *)new_hashCollection(DEFAULT_HASH_SIZE, sizeof(RexxDirectory));
                                       /* Give new object its behaviour     */
  BehaviourSet(newDirectory, TheDirectoryBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newDirectory, T_directory);
                                       /* set the hash value                */
  newDirectory->hashvalue = HASHOREF(newDirectory);
  return newDirectory;                 /* return the new directory          */
}


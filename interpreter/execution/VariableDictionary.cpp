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
/* REXX Kernel                                                                */
/*                                                                            */
/* A variable dictionary used to store object variables or variables in an    */
/* activation context.                                                        */
/*                                                                            */
/******************************************************************************/
#include <algorithm>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "Activity.hpp"
#include "ArrayClass.hpp"
#include "VariableDictionary.hpp"
#include "StemClass.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ExpressionStem.hpp"
#include "ExpressionVariable.hpp"
#include "ExpressionCompoundVariable.hpp"
#include "ExpressionDotVariable.hpp"
#include "ProtectedObject.hpp"
#include "SupplierClass.hpp"
#include "CompoundVariableTail.hpp"
#include "LanguageParser.hpp"
#include "HashCollection.hpp"
#include "DirectoryClass.hpp"
#include "GlobalNames.hpp"
#include "CompoundTableElement.hpp"


/**
 * Create a new variable dictionary object.
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating the object.
 */
void *VariableDictionary::operator new (size_t size)
{
    return new_object(size, T_VariableDictionary);
}


/**
 * construct a variable dictionary with a given size.
 *
 * @param capacity The required capacity.
 */
VariableDictionary::VariableDictionary(size_t capacity)
{
    initialize(capacity);
}


/**
 * construct a variable dictionary for a given scope.
 *
 * @param s      The scope associated with this instance.
 */
VariableDictionary::VariableDictionary(RexxClass *s)
{
    initialize(DefaultObjectDictionarySize);
    scope = s;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void VariableDictionary::live(size_t liveMark)
{
    memory_mark(contents);
    memory_mark(reservingActivity);
    memory_mark(waitingActivities);
    memory_mark(nextDictionary);
    memory_mark(scope);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void VariableDictionary::liveGeneral(MarkReason reason)
{
    memory_mark_general(contents);
    memory_mark_general(reservingActivity);
    memory_mark_general(waitingActivities);
    memory_mark_general(nextDictionary);
    memory_mark_general(scope);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void VariableDictionary::flatten(Envelope *envelope)
{
    setUpFlatten(VariableDictionary)

    flattenRef(contents);
    flattenRef(nextDictionary);
    flattenRef(scope);

    // these are references to activities and will give an error
    // if flattened.  Clear them out now.
    newThis->reservingActivity = OREF_NULL;
    newThis->waitingActivities = OREF_NULL;

    cleanUpFlatten
}


/**
 * Initialize the list contents, either directly from the
 * low level constructor or from the INIT method.
 *
 * @param capacity The requested capacity.
 */
void VariableDictionary::initialize(size_t capacity)
{
    // only do this if we have no contents already
    if (contents == OREF_NULL)
    {
        size_t bucketSize = HashCollection::calculateBucketSize(capacity);
        contents = allocateContents(bucketSize, bucketSize * 2);
    }
}


/**
 * Virtual method for allocating a new contents item for this
 * collection.  Collections with special requirements should
 * override this and return the appropriate subclass.
 *
 * @param bucketSize The bucket size of the collection.
 * @param totalSize  The total capacity of the collection.
 *
 * @return A new HashContents object appropriate for this collection type.
 */
StringHashContents *VariableDictionary::allocateContents(size_t bucketSize, size_t totalSize)
{
    return new (totalSize) StringHashContents(bucketSize, totalSize);
}


/**
 * Expand the contents of a collection.  We try for double the
 * size.
 */
void VariableDictionary::expandContents()
{
    // just double the bucket size...or there abouts
    expandContents(contents->capacity() * 2);
}


/**
 * Expand the contents of a collection to a given bucket
 * capacity.
 */
void VariableDictionary::expandContents(size_t capacity )
{
    size_t bucketSize = HashCollection::calculateBucketSize(capacity);
    Protected<StringHashContents> newContents = allocateContents(bucketSize, bucketSize * 2);
    // copy all of the items into the new table
    contents->reMerge(newContents);
    // if this is a contents item in the old space, we need to
    // empty this so any old-to-new table entries in the contents can get updated.
    if (contents->isOldSpace())
    {
        contents->empty();
    }
    // replace the contents

    setField(contents, newContents);
}


/**
 * Ensure that the collection has sufficient space for a
 * mass incoming addition.
 *
 * @param delta  The number of entries we wish to add.
 */
void VariableDictionary::ensureCapacity(size_t delta)
{
    // not enough space?  time to expand.  We'll go to
    // the current total capacity plus the delta...or the standard
    // doubling if the delta is a small value.
    if (!contents->hasCapacity(delta))
    {
        expandContents(contents->capacity() + std::max(delta, contents->capacity()));
    }
}


/**
 * If the hash collection thinks it is time to expand, then
 * increase the contents size.
 */
void VariableDictionary::checkFull()
{
    if (contents->isFull())
    {
        expandContents();
    }
}


/**
 * Copy a variable dictionary.
 *
 * @return The new dictionary.
 */
RexxInternalObject *VariableDictionary::copy()
{
    // clone the top level object
    Protected<VariableDictionary> copyObj = (VariableDictionary *)clone();
    // now copy the contents
    copyObj->contents =  (StringHashContents *)contents->copy();
    // copy all of the values in the table and return
    copyObj->copyValues();
    return copyObj;
}

/**
 * Perform a deep copy of a variable dictionary.  This copies the object,
 * all variable objects stored in the object, AND any variable dictionaries chained
 * off of this one in an object's variable set.
 *
 * @return A complete chain copy of the object.
 */
VariableDictionary *VariableDictionary::deepCopy()
{
    // make a copy of ourselves first.  This also copies the values.
    Protected<VariableDictionary> newDictionary = (VariableDictionary *)copy();
    // We might be copying an object with a guard lock, so we need to
    // clear this out in the new copy.
    newDictionary->reservingActivity = OREF_NULL;
    newDictionary->waitingActivities = OREF_NULL;
    // and propagate this if we're chained
    if (nextDictionary != OREF_NULL)
    {
        newDictionary->setNextDictionary(nextDictionary->deepCopy());
    }

    return newDictionary;
}


/**
 * Copy all of the values in a variable dictionary.  This
 * copies the variable objects, but does not copy the
 * values stored in the variables.
 */
void VariableDictionary::copyValues()
{
    // the contents handle this for us.
    contents->copyValues();
}


/**
 * Retrieve a variable's value WITHOUT returning the default
 * variable name if it doesn't exist.
 *
 * @param name   The variable name.
 *
 * @return The true assigned value of the variable.
 */
RexxObject *VariableDictionary::realValue(RexxString *name)
{
    // locate the variable in the dictionary. If we don't get one, return null.
    RexxVariable *variable = resolveVariable(name);
    if (variable == OREF_NULL)
    {
        return OREF_NULL;
    }
    return variable->getVariableValue();
}


/**
 * Drop the value of a named variable in the method dictionary.
 *
 * @param name   The string name of the variable.
 */
void VariableDictionary::drop(RexxString *name)
{
    // if the variable exists, drop the value
    RexxVariable *variable = resolveVariable(name);
    if (variable != OREF_NULL)
    {
        variable->drop();
    }
}


/**
 * Drop the value of a named variable in the method dictionary.
 *
 * @param name   The string name of the variable.
 */
void VariableDictionary::dropStemVariable(RexxString *name)
{
    // if the variable exists, drop the value
    RexxVariable *variable = resolveVariable(name);
    if (variable != OREF_NULL)
    {
        variable->drop();
        // we cast off the old stem value, but stem variables
        // always have a value, so we immediately create a replacement
        // stem.
        variable->set(new StemClass(name));
    }
}


/**
 * Retrieve a compound variable, returning OREF_NULL if the
 * variable does not exist.
 *
 * @param stemName  The name of the stem variable.
 * @param tail      The elements used to construct the compound variable tail.
 * @param tailCount The number of tail sections.
 *
 * @return The value of the compound variable, or OREF_NULL if it does not exist.
 */
CompoundTableElement *VariableDictionary::getCompoundVariable(RexxString *stemName, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    // we get the stem first, then retrieve the compound variable from the stem object.
    StemClass *stem_table = getStem(stemName);
    return stem_table->getCompoundVariable(resolved_tail);
}


/**
 * Retrieve a compound variable, returning default value if the
 * variable does not exist.  This does not raise NOVALUE.
 *
 * @param stemName  The name of the stem variable.
 * @param tail      The elements used to construct the compound variable tail.
 * @param tailCount The number of tail sections.
 *
 * @return The variable value, including substituting of the string
 *         name of the variable if it does not exist.
 */
RexxObject *VariableDictionary::getCompoundVariableValue(RexxString *stemName, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getStem(stemName);
    // get the value from the stem...we pass OREF_NULL
    // for the dictionary to bypass NOVALUE handling
    return stem_table->evaluateCompoundVariableValue(OREF_NULL, stemName, resolved_tail);
}


/**
 * Retrieve the "real" value of a compound variable.  This
 * return OREF_NULL for any situation where the compound variable
 * name would be returned.
 *
 * @param stem      The name of the stem.
 * @param tail      The set of tails used for the lookup.
 * @param tailCount The number of tail elements.
 *
 * @return Either the variable value, or OREF_NULL for unassigned
 *         variables.
 */
RexxObject *VariableDictionary::getCompoundVariableRealValue(RexxString *stem,
     RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getStem(stem);
    return stem_table->getCompoundVariableRealValue(resolved_tail);
}


/**
 * Retrieve the "real" value of a stem variable.  OREF_NULL is
 * returned if the stem does not exist.
 *
 * @param stemName The stem name.
 *
 * @return The backing stem object, or OREF_NULL if this stem
 *         variable has not been used in this context.
 */
RexxObject *VariableDictionary::realStemValue(RexxString *stemName)
{
  return getStem(stemName);
}


/**
 * Insert a variable into the dictionary hash table.  This
 * is used for inserting both stem and regular variables.
 *
 * @param variable The variable object backing the variable.
 * @param name     The variable name.
 */
void VariableDictionary::addVariable(RexxString *name, RexxVariable *variable)
{
    // make sure we have room to add this
    checkFull();
    // and just put into the table.
    contents->put(variable, name);
}


/**
 * Create a stem variable and add it to the variable context.
 *
 * @param stemName The name of the stem.
 *
 * @return The backing variable object.
 */
RexxVariable *VariableDictionary::createStemVariable(RexxString *stemName)
{
    RexxVariable *variable =  new_variable(stemName);
    StemClass *stemtable = new StemClass (stemName);
    // the stem object is the value of the stem variable.  This is created automatically
    // when a stem variable is first used.
    variable->set(stemtable);
    // add the variable to the table
    addVariable(stemName, variable);
    // return the backing variable
    return variable;
}


/**
 * Create a new variable item and add it to the dictionary.
 *
 * @param name   The variable name.
 *
 * @return The backing variable object.
 */
RexxVariable  *VariableDictionary::createVariable(RexxString *name)
{
    RexxVariable *variable =  new_variable(name);
    addVariable(name, variable);
    return variable;
}


/**
 * Set a new variable value.
 *
 * @param name   The variable name.
 * @param value  The value object.
 */
void VariableDictionary::set(RexxString *name, RexxObject *value)
{
    // this will create the variable object if it does not exist.
    RexxVariable *variable = getVariable(name);
    variable->set(value);
}


/**
 * Reserve a scope on an object, waiting for completion if this
 * is already reserved by another activity
 *
 * @param activity The activity reserving the scope.
 */
void VariableDictionary::reserve(Activity *activity)
{
    // not reserved at all?  This is easy.
    if (reservingActivity == OREF_NULL)
    {
        // Activity objects are automatically safe from
        // garbage collection and this is something we
        // want to occur as quickly a possible, so just assign
        // this value directly without worrying about old-to-new issues.
        reservingActivity = activity;
        reserveCount = 1;
    }
    // doing this again on the same stack?  Just bump the
    // nesting count. Note that nested activities created via
    // attach thread will count as being part of the same activity
    // stack since they are on the same system thread.
    else if (activity->isSameActivityStack(reservingActivity))
    {
        reserveCount++;
    }
    // we have an access collision.  We need to wait on this one.
    else
    {
        reservingActivity->checkDeadLock(activity);
        // this one we need to use setField()
        if (waitingActivities == OREF_NULL)
        {
            setField(waitingActivities, new_array());
        }
        // add to the end of the queue
        waitingActivities->append(activity);
        // ok, now we wait
        activity->waitReserve(this);
    }
}


/**
 * Release the lock on an object's variable dictionary.
 *
 * @param activity The activity owning the lock.
 */
void VariableDictionary::release(Activity *activity)
{
    reserveCount--;
    // if this is the last reserver on this activity, we just clear everything out
    if (reserveCount == 0)
    {
        // setField() not required here.  See comment in reserve()
        reservingActivity = OREF_NULL;
        // potential waiters?
        if (waitingActivities != OREF_NULL && !waitingActivities->isEmpty())
        {
            // remove the first item and make it the new reserver
            reservingActivity = (Activity *)waitingActivities->removeFirst();
            reserveCount = 1;
            // wake up the waiting activity.
            reservingActivity->guardPost();
        }
    }
}


/**
 * Transfer the variable dictionary lock to another
 * activity.  Occurs when a reply has been issued.
 *
 * @param activity The new activity.
 *
 * @return true if the reservation could transfer, false if there
 *         is nesting taking place that prevents this from occurring.
 */
bool VariableDictionary::transfer(Activity *activity)
{
    // if there is only a single level of nesting, just change the reserving activity
    if (reserveCount == 1)
    {
        // NOTE:  Not using setField
        reservingActivity = activity;
        // indicate the transfer is complete
        return true;
    }
    // multiple nesting levels...we perform a release for this activation level,
    // but this activity still owns the lock.
    else
    {
        release(activity);
        // could not release.
        return false;
    }
}


/**
 * Chain up a dictionary associated with an object.  An object
 * has a single variable dictionary for each scope, but only
 * a single anchoring field in the object.  The multiple
 * scopes are handled by chaining the variable dictionaries
 * together.
 *
 * @param next   The next dictionary in our chain.
 */
void VariableDictionary::setNextDictionary(VariableDictionary *next)
{
    setField(nextDictionary, next);
}


/**
 * Get all of the variables in the local context.  This returns
 * just the top-level variables (i.e., simple variables and stems).
 *
 * @return A StringTable of the variables.
 */
StringTable *VariableDictionary::getAllVariables()
{
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = contents->iterator();
    // create a string table with room for the number of variables we have here.
    Protected<StringTable> result = new_string_table(contents->items());

    for (; iterator.isAvailable(); iterator.next())
    {
        // get the next variable from the dictionary
        RexxVariable *variable = (RexxVariable *)iterator.value();
        // if this variable has a value, add to the result
        if (variable->getVariableValue() != OREF_NULL)
        {
            result->put(variable->getVariableValue(), variable->getName());
        }
    }

    return result;
}


/**
 * Get all of the variables in the local context.  This returns
 * just the top-level variables (i.e., simple variables and stems).
 *
 * @return A directory of the variables
 */
DirectoryClass *VariableDictionary::getVariableDirectory()
{
    // use an iterator to traverse the table
    HashContents::TableIterator iterator = contents->iterator();
    // create a string table with room for the number of variables we have here.
    Protected<DirectoryClass> result = new_directory(contents->items());

    for (; iterator.isAvailable(); iterator.next())
    {
        // get the next variable from the dictionary
        RexxVariable *variable = (RexxVariable *)iterator.value();
        // if this variable has a value, add to the result
        if (variable->getVariableValue() != OREF_NULL)
        {
            result->put(variable->getVariableValue(), variable->getName());
        }
    }

    return result;
}


/**
 * Set a compound variable in the dictionary.
 *
 * @param stemName  The name of the stem.
 * @param tail      The tail elements.
 * @param tailCount The count of tail elements.
 * @param value     The value to set.
 */
void VariableDictionary::setCompoundVariable(RexxString *stemName, RexxInternalObject **tail, size_t tailCount, RexxObject *value)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);
    // get, and potentially create, the stem object.
    StemClass *stem_table = getStem(stemName);
    // the value is set in the stem object
    stem_table->setCompoundVariable(resolved_tail, value);
}


/**
 * Drop a compound variable in the dictionary.
 *
 * @param stemName  The name of the stem.
 * @param tail      The tail elements.
 * @param tailCount The count of tail elements.
 * @param value     The value to set.
 */
void VariableDictionary::dropCompoundVariable(RexxString *stemName, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);
    // get the backing stem
    StemClass *stem_table = getStem(stemName);
    // the stem handles the drop operation.
    stem_table->dropCompoundVariable(resolved_tail);
}


/**
 * Generate a variable retriever to access a variable from a
 * string name.  Used for dynamic access such as the value
 * function or the variable pool interface.
 *
 * @param variable The variable name.
 *
 * @return An appropriate variable retriever.  Returns NULL if the
 *         name is not a valid variable name.
 */
RexxVariableBase *VariableDictionary::getVariableRetriever(RexxString *variable )
{
    // All variables for symbolic access have uppercase names.
    variable = variable->upper();
    // analyze the the string content and decide what sort of variable we have
    switch (variable->isSymbol())
    {
        // totally not a valid variable name
        case STRING_BAD_VARIABLE:
            return OREF_NULL;

        // this is a symbol, but a constant one.  Just return the name as
        // the retriever.
        case STRING_LITERAL_DOT:
        case STRING_NUMERIC:
            return(RexxVariableBase *)variable;

        // Dot variables retrieve from the environment
        case STRING_LITERAL:
            // this is only a dot variable if it begins with a period
            if (variable->getChar(0) == '.')
            {
                return (RexxVariableBase *)new RexxDotVariable(variable->extract(1, variable->getLength() - 1));
            }
            // this is a literal symbol not beginning with a period
            return (RexxVariableBase *)variable;

        // have a stem name
        case STRING_STEM:
            // create a stem retriever with a zero index
            return(RexxVariableBase *)new RexxStemVariable(variable, 0);

        // a compound name
        case STRING_COMPOUND_NAME:
            // this needs to break the name up into the component parts for retrieva.
            return(RexxVariableBase *)buildCompoundVariable(variable, false);

        // just a simple variable
        case STRING_NAME:
            return(RexxVariableBase *)new RexxSimpleVariable(variable, 0);

        // unknown...give back nothing
        default:
            return OREF_NULL;
    }
}


/**
 * Return a retriever for a variable using direct access (i.e.
 * no substitution in compound variable tails)
 *
 * @param variable The full name of the variable.  The tail portion of
 *                 any compound variables is taken as a literal.
 *
 * @return A retriever for the variable type.
 */
RexxVariableBase  *VariableDictionary::getDirectVariableRetriever(RexxString *variable)
{
    // we don't perform any uppercasing here...the name is either good, nor not.
    size_t length = variable->getLength();
    // get the first character
    char character = variable->getChar(0);
    bool        literal = false;

    // is this a constant symbol?  Remember for later
    if (character == '.' || (character >= '0' && character <= '9'))
    {
        literal = true;
    }

    // only process as a potential variable if the name has a good length
    if (length <= LanguageParser::MAX_SYMBOL_LENGTH && length > 0)
    {
        // now we need to scan the name and figure out the type.
        size_t compound = 0;                      // no periods yet
        size_t scan = 0;                          // start at string beginning
        size_t nonnumeric = 0;                    // count of non-numeric characters
        char last = 0;                            // no last character

        while (scan < length)
        {
            // a character-by-character scan
            character = variable->getChar(scan);
            // hit a period...could be a stem, compound, or just part of a number.
            if (character == '.')
            {
                if (!literal)
                {
                    // since this is a direct retriever, all variable names with a period
                    // are compound variables...this could a compound variable with a null tail.
                    return(RexxVariableBase *)buildCompoundVariable(variable, true);
                }
                else
                {
                    compound++;
                }
            }
            // may have a special character  We also allow valid numeric symbols, which means dealing
            // with an exponent.
            else if (!LanguageParser::isSymbolCharacter(character))
            {
                // check for the signs first
                if (character == '+' || character == '-')
                {
                    // we've tracked periods and non-numeric characters,
                    // so we know if this can be a number.  The last character
                    // must also be an E to be accepted.
                    if (compound > 1 || nonnumeric > 1 || last != 'E')
                    {
                        return OREF_NULL;
                    }
                    scan++;
                    // the sign can't be the last character either
                    if (scan >= length)
                    {
                        return OREF_NULL;
                    }
                    // to be valid, we can only have digits from this point
                    while (scan < length)
                    {

                        character = variable->getChar(scan);
                        if (character < '0' || character > '9')
                        {
                            return OREF_NULL;
                        }
                        scan++;
                    }
                    break;
                }
                else
                {
                    // invalid character in a symbol
                    return OREF_NULL;
                }
            }
            // non-special character...keep count of any non-digit characters
            else if (character < '0' || character > '9')
            {
                nonnumeric++;
            }
            // lower case characters fail on a symbolic lookup
            else if (LanguageParser::translateChar(character) != character)
            {
                return OREF_NULL;
            }
            // keep track of the last character and continue
            last = character;
            scan++;
        }
    }
    // we've screend out all of the invalid cases.  This is either just a
    // literal value or a real simple variable.
    if (literal)
    {
        // literals are their own retrievers.
        return(RexxVariableBase *)variable;
    }
    // a simple variable
    else
    {
        return(RexxVariableBase *)new RexxSimpleVariable(variable, 0);
    }
}


/**
 * Build a retriever for a compound variable (either direct or symbolic)
 *
 * @param variable_name
 *               The full name of the variable.
 * @param direct Indicates whether we build this with direct or symbolic rules.
 *               For direct rules, everything after the first period is
 *               used as a literal tail value.  for symbolic rules, we
 *               need to evaluate each of the tail sections.
 *
 * @return A retriever for this variable.
 */
RexxVariableBase *VariableDictionary::buildCompoundVariable(RexxString *variable_name, bool direct)
{
    size_t length = variable_name->getLength();
    size_t position = 0;

    // ok, we need to scan to the first period.  We know we have a
    // good variable up to the stem name start.
    while (variable_name->getChar(position) != '.')
    {
        position++;
        length--;
    }

    // get the stem part
    Protected<RexxString> stem = variable_name->extract(0, position + 1);

    // start decomposing this into the component parts
    Protected<QueueClass> tails = new_queue();
    // the first period is part of the stem name.
    position++;
    length--;

    // if we're buildind a direct retriever, there is just one
    // tail piece that is a literal...and it could be a null string
    if (direct == true)
    {
        // get the remainder as a tail and use that
        RexxString *tail = variable_name->extract(position, length);
        tails->push(tail);
    }
    // need to scan for component pieces
    else
    {
        size_t endPosition = position + length;

        while (position < endPosition)
        {
            size_t start = position;
            // find the next period (or the end of the name)
            while (position < endPosition && variable_name->getChar(position) != '.')
            {
                position++;                    /* step to the next character        */
            }
            // get the piece as a string
            RexxString *tail = variable_name->extract(start, position - start);

            // a null string, or constant symbol (starting with a digit) can be used directly.
            RexxVariableBase *tailPart;
            if (tail->getLength() == 0 || (tail->getChar(0) >= '0' && tail->getChar(0) <= '9'))
            {
                tailPart = (RexxVariableBase *)tail;
            }
            // simple variable, we need a retriever for this
            else
            {
                tailPart = new RexxSimpleVariable(tail, 0);
            }
            tails->push(tailPart);
            position++;
        }

        // since we build up the name from pieces and only add dots between sections, add
        // a null string for the final section to pick up the trailing period
        if (variable_name->getChar(position - 1) == '.')
        {
            tails->push(GlobalNames::NULLSTRING);
        }
    }

    // create and return a new compound
    return new (tails->items()) RexxCompoundVariable(stem, 0, tails, tails->items());
}


/**
 * Retrieve an iterator for this variable dictionary.
 * This iterates over both the simple variables and the
 * stem variables.
 *
 * @return An iterator instance for this dictionary.
 */
VariableDictionary::VariableIterator VariableDictionary::iterator()
{
    // the iterator handles all of the details
    return VariableIterator(this);
}


/**
 * constructor for an index iterator
 *
 * @param d      The dictionary we're created from.
 */
VariableDictionary::VariableIterator::VariableIterator(VariableDictionary *d)
{
    dictionary = d;
    dictionaryIterator = dictionary->contents->iterator();
    currentStem = OREF_NULL;
    returnStemValue = false;

    // we need to skip over any dropped variables
    while (dictionaryIterator.isAvailable() &&
      ((RexxVariable *)dictionaryIterator.value())->isDropped())
    {
       dictionaryIterator.next();
    }

    // now we're set up, but it is possible that the
    // first item in the dictionary is a stem.  We need to
    // check this here so we're set up properly to
    // treat it as such
    if (dictionaryIterator.isAvailable())
    {
        // if we've hit a stem variable, switch the iterator to
        // the stem version.
        RexxVariable *variable = (RexxVariable *)dictionaryIterator.value();
        if (variable->isStem())
        {
            currentStem = (StemClass *)variable->getVariableValue();
            stemIterator = currentStem->iterator();
            // if the stem has an explicitly signed value, return it first
            returnStemValue = currentStem->hasValue();
        }
    }
}


/**
 * Step to the next position while iterating through a
 * variable dictionary.
 */
void VariableDictionary::VariableIterator::next()
{
    // if if our last entry was a stem with a value, turn off the flag
    // and leave everything else as is.
    if (returnStemValue)
    {
        returnStemValue = false;
        return;
    }

    if (currentStem != OREF_NULL)
    {
        // step and then check if we have anything left.  If not, we need to
        // revert to normal iteration mode
        stemIterator.next();
        while (stemIterator.isAvailable())
        {
            // if this is a non-dropped variable, we're at a good point.  Otherwise
            // skip over this in the iteration
            CompoundTableElement *variable = stemIterator.variable();
            if (!variable->isDropped())
            {
                return;
            }

            stemIterator.next();
        }
        // switch back to the main collection
        currentStem = OREF_NULL;
    }

    // this is a little more complicated.  We need to step
    // to the next variable and determine if this is a stem variable so
    // we can switch iteration modes.
    dictionaryIterator.next();
    while (dictionaryIterator.isAvailable())
    {
        // if we've hit a stem variable, switch the iterator to
        // the stem version.
        RexxVariable *variable = (RexxVariable *)dictionaryIterator.value();
        // we only process variables with a value, so skip over dropped ones
        if (!variable->isDropped())
        {
            if (variable->isStem())
            {
                currentStem = (StemClass *)variable->getVariableValue();
                stemIterator = currentStem->iterator();
                // if the stem has an explicitly signed value, return it first
                returnStemValue = currentStem->hasValue();
            }
            return;
        }
        // hit a dropped variable, try again.
        dictionaryIterator.next();
    }
}


/**
 * Retrieve the value of the current variable.
 *
 * @return Return the name of the current variable.
 */
RexxObject *VariableDictionary::VariableIterator::value()
{
    // if we have a stem with an explicit value, return it now
    if (returnStemValue)
    {
        return currentStem->getValue();
    }

    if (currentStem != OREF_NULL)
    {
        return stemIterator.value();
    }

    return ((RexxVariable *)dictionaryIterator.value())->getVariableValue();
}


/**
 * Return the name of the current variable.
 *
 * @return The variable name.
 */
RexxString *VariableDictionary::VariableIterator::name()
{
    // if we have a stem with an explicit value, return its name
    if (returnStemValue)
    {
        return currentStem->getName();
    }

    if (currentStem != OREF_NULL)
    {
        // need to construct this name from the stem variable name and the tail
        return (RexxString *)stemIterator.name((RexxString *)dictionaryIterator.index());
    }

    return (RexxString *)dictionaryIterator.index();
}

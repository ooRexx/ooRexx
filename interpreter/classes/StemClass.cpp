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
/* REXX Kernel                                               StemClass.cpp    */
/*                                                                            */
/* Primitive Stem variable class                                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "VariableDictionary.hpp"
#include "RexxVariable.hpp"
#include "SupplierClass.hpp"
#include "DirectoryClass.hpp"
#include "StemClass.hpp"
#include "CompoundVariableTail.hpp"
#include "MethodArguments.hpp"
#include "CompoundTableElement.hpp"

// singleton class instance
RexxClass *StemClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void StemClass::createInstance()
{
    CLASS_CREATE(Stem);
}


/**
 * Allocate memory for a new stem instance.
 *
 * @param size   The size of the object.
 *
 * @return Backing storage for an object.
 */
void *StemClass::operator new(size_t size)
{
    return new_object(size, T_Stem);
}


/**
 * Allocate a new Stem object from Rexx code.
 *
 * @param init_args The standard variable argument pointer.
 * @param argCount  The count of arguments.
 *
 * @return A new stem instance.
 */
RexxObject *StemClass::newRexx(RexxObject **init_args, size_t       argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // the name is an optional argument, we'll just pass along to the constructor which will
    // perform the validation.
    RexxObject *name;
    RexxClass::processNewArgs(init_args, argCount, init_args, argCount, 1, name, NULL);

    Protected<StemClass> newObj = new StemClass ((RexxString *)name);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, init_args, argCount);
    return newObj;
}


/**
 * Initialize a stem instance.
 *
 * @param name   The name of the stem (optional)
 */
StemClass::StemClass(RexxString *name)
{
    // get the name as a real string...we default to a null string.
    name = optionalStringArgument(name, GlobalNames::NULLSTRING, ARG_ONE);

    stemName = name;
    value = name;
    // initialize the tail table so it knows who owns it.
    tails.init(this);
    // no explicit value at this point
    dropped = true;
}


/**
 * Copy a stem collection object.
 *
 * @return A copy of the stem object.
 */
RexxInternalObject *StemClass::copy()
{
    // do the base object copy
    Protected<StemClass> newObj = (StemClass *)RexxObject::copy();
    // have the object copy the tail table
    newObj->copyFrom(tails);
    return newObj;
}


/**
 * Copy the tails from another stem object into this stem.
 *
 * @param source The source tail collection.
 */
void StemClass::copyFrom(CompoundVariableTable &source)
{
    // initialize the tails so that it points back to the correct parent
    // table.
    tails.init(this);
    // and perform the copy operation
    tails.copyFrom(source);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void StemClass::live(size_t liveMark)
{
    memory_mark(value);
    memory_mark(stemName);
    memory_mark(objectVariables);
    markCompoundTable();
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void StemClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(value);
    memory_mark_general(stemName);
    memory_mark_general(objectVariables);
    markGeneralCompoundTable();
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void StemClass::flatten(Envelope *envelope)
{
    setUpFlatten(StemClass)

    flattenRef(value);
    flattenRef(stemName);
    flattenRef(objectVariables);
    flattenCompoundTable();

    cleanUpFlatten
}


/**
 * Set a new stem default value.
 *
 * @param _value The new value to set.
 */
void StemClass::setValue(RexxObject *newValue)
{
    setField(value, newValue);           // set the new value
    dropped = false;                     // now have an explict value
}


/**
 * Drop the stem value and revert to the default initial name.
 */
void StemClass::dropValue()
{
    // revert back to the default stem value of the stem name.
    setField(value, stemName);
    dropped = true;                // we no longer have an explicit value
}


/**
 * Retrieve the assigned stem value.
 *
 * @return The default stem value.
 */
RexxInternalObject *StemClass::getStemValue()
{
    return value;
}


/**
 * Process UNKNOWN messages for a string hash collection object,
 * forwarding them to the string value.  We need this and
 * processUnknown because the collections are documented as
 * having an UNKNKOWN method.
 *
 * @param message   The message target.
 * @param arguments The message arguments.
 *
 * @return The message result.
 */
RexxObject *StemClass::unknownRexx(RexxString *message, ArrayClass *arguments)
{
    Protected<RexxString> messageName = stringArgument(message, ARG_ONE);
    Protected<ArrayClass> argumentList = arrayArgument(arguments, ARG_TWO);

    ProtectedObject result;
    return value->sendMessage(messageName, argumentList, result);
}


/**
 * Process an unknown message condition on an object.  This is
 * an optimized bypass for the Object default method that can
 * bypass creating an array for the arguments and sending the
 * UNKNOWN message to the object.  Since many things funnel
 * through the integer unknown method, this is a big
 * optimization.
 *
 * @param messageName
 *                  The target message name.
 * @param arguments The message arguments.
 * @param count     The count of arguments.
 * @param result    The return result protected object.
 */
void StemClass::processUnknown(RexxErrorCodes error, RexxString *messageName, RexxObject **arguments, size_t count, ProtectedObject &result)
{
    // just send this as a message directly to the string object.
    value->messageSend(messageName, arguments, count, result);
}


/**
 * Perform a lookup on a stem object.
 *
 * @param tailElements
 *                 The array of tail elements used to construct
 *                 the tail.  All tail elements are constants at
 *                 this point.
 * @param argCount The count of tail arguments.
 *
 * @return The compound variable lookup value.
 */
RexxInternalObject *StemClass::bracket(RexxObject **tailElements, size_t argCount)
{
    // no arguments just returns the default value
    if (argCount == 0)
    {
        return value;
    }
    // create a searchable tail, and perform the lookup
    CompoundVariableTail resolved_tail((RexxInternalObject **)tailElements, argCount);
    return evaluateCompoundVariableValue(OREF_NULL, stemName, resolved_tail);
}


/**
 * Test if this compound variable has a given index.
 *
 * @param tails    The set of tail expressions.
 * @param argCount The argument count
 *
 * @return True if the fully resolved tail exists in the stem, false
 *         otherwise.
 */
RexxObject *StemClass::hasIndex(RexxObject **tailElements, size_t argCount)
{
    if (argCount == 0)
    {
        return TheTrueObject;          // we always have something here
    }
    // compose the tail element
    CompoundVariableTail resolved_tail((RexxInternalObject **)tailElements, argCount);
    // see if we have a compound
    CompoundTableElement *compound = findCompoundVariable(resolved_tail);
    // if there's a variable there, and it has a real value, then
    // this is true.
    return booleanObject(compound != OREF_NULL && compound->getVariableValue() != OREF_NULL);
}


/**
 * Remove an item from the collection.  This is essentially
 * equivalent to a drop operation on the stem variable.
 *
 * @param tails    The set of tail indexes.
 * @param argCount The number of indexes.
 *
 * @return The removed object.  If nothing was removed, this returns
 *         .nil.
 */
RexxInternalObject *StemClass::remove(RexxObject **tailElements, size_t argCount)
{
    // if asked to remove the default value, reset this back to the name
    if (argCount == 0)
    {
        // replace with the name and return the old value.
        RexxInternalObject *oldValue = value;
        setField(value, getName());
        return oldValue;
    }

    // compose the tail element
    CompoundVariableTail resolved_tail((RexxInternalObject **)tailElements, argCount);
    CompoundTableElement *compound = findCompoundVariable(resolved_tail);
    // if there's a variable there, and it has a real value, then
    // we have something to remove
    if (compound != OREF_NULL && compound->getVariableValue() != OREF_NULL)
    {
        // get the value, which is the return value, and drop the variable.
        RexxInternalObject *oldValue = compound->getVariableValue();
        compound->drop();
        return oldValue;
    }
    return TheNilObject;       // nothing dropped.
}


/**
 * Search for any index that matches the target object.
 *
 * @param target The object of interest.
 *
 * @return .true if the object is in the collection, .false otherwise.
 */
RexxObject *StemClass::hasItem(RexxInternalObject *target)
{
    CompoundTableElement *variable = findByValue(target);
    return booleanObject(variable != OREF_NULL);
}


/**
 * Remove the target object from the stem.
 *
 * @param target The target object.
 *
 * @return The removed object (same as target).
 */
RexxInternalObject *StemClass::removeItemRexx(RexxObject *target)
{
    // we require the index to be there.
    requiredArgument(target, ARG_ONE);
    // do the removal
    return removeItem(target);
}


/**
 * Remove the target object from the stem.
 *
 * @param target The target object.
 *
 * @return The removed object (same as target).
 */
RexxInternalObject *StemClass::removeItem(RexxInternalObject *target)
{
    CompoundTableElement *compound = findByValue(target);
    // if there's a variable there, and it has a real value, then
    // we have something to remove
    if (compound != OREF_NULL && compound->getVariableValue() != OREF_NULL)
    {
        // get the value, which is the return value, and drop the variable.
        RexxInternalObject *oldValue = compound->getVariableValue();
        compound->drop();
        return oldValue;
    }
    return TheNilObject;       // nothing dropped.
}


/**
 * Return the index for a target item.
 *
 * @param target The target object.
 *
 * @return The tail name for the match, or .nil if it was not found.
 */
RexxObject *StemClass::index(RexxInternalObject *target)
{
    CompoundTableElement *variable = findByValue(target);
    if (variable != OREF_NULL)
    {
        return variable->getName();
    }
    return TheNilObject;
}


/**
 * Return the number of items set in the collection.
 *
 * @return The count of items in the collection, not counting the
 *         default value.
 */
RexxObject *StemClass::itemsRexx()
{
    return new_integer(items());
}


/**
 * Resolve the "stem.[a,b,c]=" to the equivalent stem.a.b.c= form,
 * with all of the indices taken as constants
 *
 * @param tailElements
 *                 The array of arguments.  the first argument is the
 *                 assigned value.  All additional arguments are the
 *                 tail pieces.
 * @param argCount The count of arguments.
 *
 * @return Returns nothing.
 */
RexxObject *StemClass::bracketEqual(RexxObject **tailElements, size_t argCount)
{

    // no value to set?  This is an error
    if (argCount == 0)
    {
        reportException(Error_Incorrect_method_noarg, IntegerOne);
    }
    RexxObject *newValue = requiredArgument(tailElements[0], ARG_ONE);

    // if the argument count is 1, we're just setting the default value
    if (argCount == 1)
    {
        // stem value as default?  don't allow this as it leads to recursion loops
        if (isStem(newValue))
        {
            reportException(Error_Execution_nostem);
        }

        setField(value, newValue);
        // this clears out all of our elements and gets marked as having an
        // explicit value.
        tails.clear();
        dropped = false;
        return OREF_NULL;
    }

    // create a searchable tail from the array elements
    // and set the variable value
    CompoundVariableTail resolved_tail((RexxInternalObject **)(tailElements + 1), argCount - 1);
    CompoundTableElement *variable = getCompoundVariable(resolved_tail);
    variable->set(newValue);
    return OREF_NULL;
}


/**
 * Extract an array of tail items.
 *
 * @return An array of all tails in the stem.
 */
ArrayClass  *StemClass::makeArray()
{
    return tailArray();
}


/**
 * Get the string value for the stem, which is just
 * the string value of our default value.
 *
 * @return The associated string value.
 */
RexxString *StemClass::stringValue()
{
    return value->stringValue();
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 * @param digits The precision to convert under.
 *
 * @return true if this converted, false otherwise.
 */
bool StemClass::numberValue(wholenumber_t &result, wholenumber_t digits)
{
    return value->numberValue(result, digits);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 *
 * @return true if this converted, false otherwise.
 */
bool StemClass::numberValue(wholenumber_t &result)
{
    return value->numberValue(result);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 * @param digits The precision to convert under.
 *
 * @return true if this converted, false otherwise.
 */
bool StemClass::unsignedNumberValue(size_t &result, wholenumber_t digits)
{
    return value->unsignedNumberValue(result, digits);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 *
 * @return true if this converted, false otherwise.
 */
bool StemClass::unsignedNumberValue(size_t &result)
{
    return value->unsignedNumberValue(result);
}


/**
 * Convert the stem default value to a numeric value.
 *
 * @param result the place to put the converted number.
 *
 * @return true if this converted, false otherwise.
 */
bool StemClass::doubleValue(double &result)
{
    return value->doubleValue(result);
}


/**
 * Convert this stem object to a number string value.
 *
 * @return A number string object, or OREF_NULL if non-numeric
 */
NumberString *StemClass::numberString()
{
   return value->numberString();
}


/**
 * Convert the stem to an integer value.
 *
 * @param precision The precision to convert under.
 *
 * @return The Integer object or OREF_NULL if not valid.
 */
RexxInteger *StemClass::integerValue(wholenumber_t precision)
{
    return value->integerValue(precision);
}


/**
 * Forward all request requests to the target value.  We
 * handle some of these directly, the rest are passed on
 * to the default value.
 *
 * @param requestclass The name of the class for the request.
 *
 * @return The appropriate result for the request.
 */
RexxObject *StemClass::request(RexxString *requestclass)
{
    ProtectedObject result;
    Protected<RexxString> makeclass = stringArgument(requestclass, ARG_ONE)->upper();
    // take care of any ARRAY requests, the rest go to the default value
    if (makeclass->strCompare("ARRAY"))
    {
        // if we have a real stem object (not a subclass), handle directly,
        // otherwise send to the subclass.
        if (isStem(this))
        {
            return makeArray();
        }
        else
        {
            sendMessage(GlobalNames::MAKEARRAY, result);
            return result;
        }
    }
    // let the default value handle everything else
    value->sendMessage(GlobalNames::REQUEST, makeclass, result);
    return result;
}


/**
 * Get an item from the variable dictionary, adding a new empty
 * variable entry if it wasn't found.
 *
 * @param name   The target tail name.
 *
 * @return The variable object corresponding to the the variable entry.
 */
CompoundTableElement *StemClass::getCompoundVariable(CompoundVariableTail &name)
{
    return tails.findEntry(name, true)->realVariable();
}


/**
 * Perform an expose operation on a Compound variable.
 *
 * @param name   The compound tail name.
 *
 * @return The corresponding table element.
 */
CompoundTableElement *StemClass::exposeCompoundVariable(CompoundVariableTail &name)
{
    // first see if the compound variable already exists.  If it does, then
    // it might represent an explicitly dropped varaible.  We leave this along
    // in that case.
    CompoundTableElement *variable = tails.findEntry(name, false);
    // ok, it's already there, return the real variable value.
    if (variable != OREF_NULL)
    {
        return variable->realVariable();
    }

    // we're creating a real variable.  If the stem has an explicit value,
    // then we need to set that now.
    variable = tails.findEntry(name, true)->realVariable();
    // if this variable does not have a value, we need to check the
    // stem for a default value and assign it.
    if (variable->getVariableValue() == OREF_NULL)
    {
        if (!dropped)
        {
            variable->set(value);
        }
    }
    return variable;
}


/**
 * Get an item from the variable dictionary, without creating a
 * new variable if it doesn't exist.
 *
 * @param name   The target tail name.
 *
 * @return the target table element, or OREF_NULL if the variable doesn't exist.
 */
CompoundTableElement *StemClass::findCompoundVariable(CompoundVariableTail &name)
{
    // get, but don't create an entry in the table
    CompoundTableElement *variable = tails.findEntry(name);
    if (variable != OREF_NULL)
    {
        return variable->realVariable();
    }
    return OREF_NULL;
}


/**
 * Mark a variable as dropped.
 *
 * @param name   The target compound name.
 */
void StemClass::dropCompoundVariable(CompoundVariableTail &name)
{
    RexxVariable *variable = getCompoundVariable(name);
    variable->drop();
}


/**
 * Set a new variable value
 *
 * @param name     The target name.
 * @param newValue The new value to assign.
 */
void StemClass::setCompoundVariable(CompoundVariableTail &name, RexxObject *newValue)
{
    CompoundTableElement *variable = getCompoundVariable(name);
    variable->set(newValue);
}


/**
 * Return all indices as an array
 *
 * @return An array of the assigned tails.
 */
ArrayClass *StemClass::tailArray()
{
    ArrayClass *array = new_array(items());

    CompoundTableElement *variable = tails.first();
    // do a tree traversal
    while (variable != OREF_NULL)
    {
        // skip dropped variables, we only want the assigned values
        if (variable->getVariableValue() != OREF_NULL)
        {
            array->append(variable->getName());
        }
        variable = tails.next(variable);
    }
    return array;
}


/**
 * Retrieve a compound variable, returning the default value if the
 * variable does not exist.  This includes NOVALUE handling.
 *
 * @param context The current execution context.
 * @param stemVariableName
 *                The name of the stem variable (needed to compose default value)
 * @param resolved_tail
 *                The resolved tail.
 *
 * @return The variable value.
 */
RexxObject *StemClass::evaluateCompoundVariableValue(RexxActivation *context, RexxString *stemVariableName, CompoundVariableTail &resolved_tail)
{
    CompoundTableElement *variable = findCompoundVariable(resolved_tail);
    // if we do not have a variable, then we need to sort out what the
    // variable value nees to be
    if (variable == OREF_NULL)
    {
        // if the stem has been explicitly assigned avalue, then
        // just return the stem default.  This does not raise the
        // NOVALUE condition.
        if (!dropped)
        {
            return value;
        }
        // need to compose the value from the stem name.
        else
        {
            // create a compound name from the stem variable name and the resolved tail
            RexxString *tail_name = resolved_tail.createCompoundName(stemVariableName);
            // the tail_name is the fully resolved variable, used for NOVALUE reporting.
            // the defaultValue is the value that's returned as the expression result,
            // which is derived from the stem object.
            RexxObject *defaultValue = resolved_tail.createCompoundName(stemName);
            // take care of any novalue situations
            return handleNovalue(context, tail_name, defaultValue, variable);
        }
    }
    else
    {
        // get the variable value.  If there is no value, we've been
        // explicitly dropped so we need to do the novalue bit.
        RexxObject *varValue = variable->getVariableValue();
        if (varValue == OREF_NULL)
        {

            RexxString *tail_name = resolved_tail.createCompoundName(stemVariableName);
            // the tail_name is the fully resolved variable, used for NOVALUE reporting.
            // the defaultValue is the value that's returned as the expression result,
            // which is derived from the stem object.
            RexxObject *defaultValue = resolved_tail.createCompoundName(stemName);
            // take care of any novalue situations
            return handleNovalue(context, tail_name, defaultValue, variable);
        }
        return varValue;
    }
}

/**
 * Retrieve a compound variable, returning the default value if the
 * variable does not exist.  This does NOT raise NOVALUE conditions
 *
 * @param resolved_tail
 *               The resolved compound tail.
 *
 * @return The variable value, or OREF_NULL if the variable is not set.
 */
RexxObject *StemClass::getCompoundVariableValue(CompoundVariableTail &resolved_tail)
{
    CompoundTableElement *variable = findCompoundVariable(resolved_tail);
    if (variable == OREF_NULL)
    {
        if (!dropped)
        {
            return value;
        }
        else
        {
            // we create the value using the stem object name, not the
            // name of any variable we are attached to.
            return  resolved_tail.createCompoundName(stemName);
        }
    }
    else
    {
        // have a variable, now see if this has a value assigned.
        RexxObject *varValue = variable->getVariableValue();
        if (varValue == OREF_NULL)
        {
            return resolved_tail.createCompoundName(stemName);
        }
        return varValue;
    }
}


/**
 * Evaluate the real value of a compound variable.  The real
 * value is either its explicitly assigned value or a stem
 * assigned value.  This returns OREF_NULL if neither is
 * available.  This does not raise NOVALUE conditions.
 *
 * @param resolved_tail
 *               The target tail value.
 *
 * @return The variable value, or OREF_NULL if the variable does not
 *         have an explicit value.
 */
RexxObject *StemClass::getCompoundVariableRealValue(CompoundVariableTail &resolved_tail)
{
    // first resolve the compound variable.
    RexxVariable *variable = findCompoundVariable(resolved_tail);
    // if the variable is not found, return the stem's default value if it has one.
    // If there is no default value, return OREF_NULL.
    if (variable == OREF_NULL)           /* variable does not exist?          */
    {
        if (!dropped)
        {
            return value;
        }
        return OREF_NULL;
    }
    else
    {
        // just return the variable value (which may be OREF_NULL if explicitly dropped)
        return variable->getVariableValue();
    }
}


/**
 * Retrieve a compound variable, returning the default value if the
 * variable does not exist.  This does not handle NOVALUEs.
 *
 * @param resolved_tail
 *               The resolved tail.
 *
 * @return The real assigned value (no default processing)
 */
RexxObject *StemClass::realCompoundVariableValue(CompoundVariableTail &resolved_tail)
{
    CompoundTableElement *variable = findCompoundVariable(resolved_tail);
    // if no variable found, then its value is either an explictly set stem
    // value or nothing.
    if (variable == OREF_NULL)
    {
        if (!dropped)
        {
            return value;
        }
        else
        {
            return OREF_NULL;
        }
    }
    // return the variable value (which might be NULL if dropped)
    return variable->getVariableValue();
}


/**
 * Process a nonvalue condition for a stem variable retrieval.
 * If a context is provided, the novalue is handled in its
 * context.
 *
 * @param context  The variable context.
 * @param name     The fully resolved variable name.
 * @param defaultValue
 *                 The default value to use.
 * @param variable The variable entry
 *
 * @return Either the name, or a novalue handler replacement value.
 */
RexxObject *StemClass::handleNovalue(RexxActivation *context, RexxString *name,
    RexxObject *defaultValue, CompoundTableElement *variable)
{
    if (context != OREF_NULL)
    {
        return context->handleNovalueEvent(name, defaultValue, variable);
    }
    else
    {
        return name;                 /* just use the name                 */
    }
}


/**
 * Complete a hook up for exposing a compound variable.
 *
 * @param old_variable
 *               The old variable entry in the orignal stem.
 */
void StemClass::expose(CompoundTableElement *old_variable)
{
    // create the equivalent in this stem
    CompoundTableElement *new_variable = tails.findEntry(old_variable->getName(), true);
    // create an association between the two variables.
    new_variable->expose(old_variable);
}


/**
 * Return all items in the stem.
 *
 * @return An array of all items in the stem.
 */
ArrayClass *StemClass::allItems()
{
    // now we know how big the return result will be, get an array and
    // populate it, using the same traversal logic as before
    ArrayClass *array = new_array(items());
    // we index the array with a origin-one index, so we start with one this time
    size_t count = 1;

    CompoundTableElement *variable = tails.first();
    while (variable != OREF_NULL)
    {
        // only add the real values
        if (variable->getVariableValue() != OREF_NULL)
        {
            array->put(variable->getVariableValue(), count++);
        }
        variable = tails.next(variable);
    }
    return array;    // tada, finished
}


/**
 * Locate a stem item by value.
 *
 * @return The compound item for the located element.
 */
CompoundTableElement *StemClass::findByValue(RexxInternalObject *target)
{
    CompoundTableElement *variable = tails.first();
    while (variable != OREF_NULL)
    {
        RexxInternalObject *varValue = variable->getVariableValue();
        // if this has a value, and we have a match, then return it
        if (varValue != OREF_NULL && target->equalValue(varValue))
        {
            return variable;
        }
        variable = tails.next(variable);
    }
    return OREF_NULL;     // not here, oh dear
}


/**
 * Get the count of non-dropped items in the stem.
 *
 * @return The number of non-dropped items.
 */
size_t StemClass::items()
{
    size_t count = 0;

    CompoundTableElement *variable = tails.first();
    while (variable != OREF_NULL)
    {
        // we only want to include the non-dropped compounds, so we only count
        // elements with real values.
        if (variable->getVariableValue() != OREF_NULL)
        {
            count++;
        }
        // and keep iterating
        variable = tails.next(variable);
    }
    return count;
}


/**
 * Empty the stem.  This also clears dropped and exposed tails,
 *
 * @return Nothing.
 */
RexxObject *StemClass::empty()
{
    tails.clear();      // just clear the tails.
    return this;        // return receiving Array
}


/**
 * Test if the stem is empty.
 *
 * @return True if the stem is empty, false otherwise.
 */
RexxObject *StemClass::isEmptyRexx()
{
    return booleanObject(isEmpty());
}


/**
 * Test if the stem is empty.
 *
 * @return True if the stem is empty, false otherwise.
 */
bool StemClass::isEmpty()
{
    return (items() == 0);
}



/**
 * Create an array of all indexes of the stem.
 *
 * @return An array of all tail names used in the stem.
 */
ArrayClass  *StemClass::allIndexes()
{
    return this->tailArray();            /* extract the array item            */
}


/**
 * Create a supplier for the stem, returning the tail names as
 * the indexes and the values as the items.
 *
 * @return A supplier instance.
 */
SupplierClass *StemClass::supplier()
{
    // essentially the same logic as allItems(), but both the item and the
    // tail value are accumulated.
    size_t count = 0;
    CompoundTableElement *variable = tails.first();
    while (variable != OREF_NULL)
    {
        // again, get the variable count
        if (variable->getVariableValue() != OREF_NULL)
        {
            count++;                     /* count this variable               */
        }
        variable = tails.next(variable);
    }

    // to create the supplier, we need 2 arrays
    ArrayClass *tailValues = new_array(count);
    ArrayClass *values = new_array(count);
    count = 1;                           // we fill in using 1-based indexes

    variable = tails.first();
    while (variable != OREF_NULL)
    {
        // now grab both the tail and value and put them in the respective arrays
        if (variable->getVariableValue() != OREF_NULL)
        {
            tailValues->put(variable->getName(), count);
            values->put(variable->getVariableValue(), count++);
        }
        variable = tails.next(variable);
    }
    // two arrays become one supplier
    return new_supplier(values, tailValues);
}


/**
 * Create a directory from the stem.  Each tail with an
 * assigned value will be an entry within the directory.
 *
 * @return A directory instance.
 */
DirectoryClass *StemClass::toDirectory()
{
    DirectoryClass *result = new_directory();
    ProtectedObject p1(result);
    CompoundTableElement *variable = tails.first();
    while (variable != OREF_NULL)
    {
        // again, get the variable count
        if (variable->getVariableValue() != OREF_NULL)
        {
            result->put(variable->getVariableValue(), variable->getName());
        }
        variable = tails.next(variable);
    }

    return result;
}


/**
 * Set a single stem variable object using a simple string
 * value tail as a result of an api call.
 *
 * @param tail   The index of the target value.
 * @param value  The new value to assign.
 */
void StemClass::setElement(const char *tail, RexxObject *newValue)
{
    CompoundVariableTail resolved_tail(tail);
    RexxVariable *variable = getCompoundVariable(resolved_tail);
    variable->set(newValue);
}


/**
 * Set a single stem variable object using a simple string
 * value tail as a result of an api call.
 *
 * @param tail   The index of the target value.
 * @param value  The new value to assign.
 */
void StemClass::setElement(size_t tail, RexxObject *newValue)
{
    CompoundVariableTail resolved_tail(tail);
    RexxVariable *variable = getCompoundVariable(resolved_tail);
    variable->set(newValue);
}


/**
 * Evaluate an array element for an API class.
 *
 * @param tail   The direct tail value.
 *
 * @return The object value.  If the stem element does not exist or
 *         has been dropped, this returns OREF_NULL.
 */
RexxObject *StemClass::getElement(size_t tail)
{
    CompoundVariableTail resolved_tail(tail);

    return getElement(resolved_tail);
}


/**
 * Evaluate an array element for an API class.
 *
 * @param tail   The direct tail value.
 *
 * @return The object value.  If the stem element does not exist or
 *         has been dropped, this returns OREF_NULL.
 */
RexxObject *StemClass::getElement(const char *tail)
{

    CompoundVariableTail resolved_tail(tail);
    return getElement(resolved_tail);
}


/**
 * Resolve a compound variable as a result of an api call.
 *
 * @param resolved_tail
 *               The resolved tail value.
 *
 * @return The variable value.  Returns OREF_NULL if not assigned or
 *         dropped.
 */
RexxObject *StemClass::getElement(CompoundVariableTail &resolved_tail)
{
    // see if we have a variable...if we do, return its value (a dropped variable
    // has a value of OREF_NULL).  If not found, return OREF_NULL;
    CompoundTableElement *variable = findCompoundVariable(resolved_tail);
    if (variable != OREF_NULL)
    {
        return variable->getVariableValue();
    }
    return OREF_NULL;
}


/**
 * Evaluate an array element including default value resolution
 *
 * @param tail   The direct tail value.
 *
 * @return The object value.  If the stem element does not exist or
 *         has been dropped, this returns the default value.
 */
RexxObject *StemClass::getFullElement(size_t tail)
{
    CompoundVariableTail resolved_tail(tail);

    return getFullElement(resolved_tail);
}


/**
 * Resolve a compound variable as a result of an api call.
 *
 * @param resolved_tail
 *               The resolved tail value.
 *
 * @return The variable value.  Returns default value if not
 *         assigned or dropped.
 */
RexxObject *StemClass::getFullElement(CompoundVariableTail &resolved_tail)
{
    // see if we have a variable...if we do, return its value (a dropped variable
    // has a value of OREF_NULL).  If not found, return OREF_NULL;
    CompoundTableElement *variable = findCompoundVariable(resolved_tail);

    RexxObject *varValue = OREF_NULL;
    if (variable != OREF_NULL)
    {
        varValue = variable->getVariableValue();
    }
    return varValue != OREF_NULL ? varValue : value;
}


/**
 * Drop an array element for an API class.
 *
 * @param tail   The direct tail value.
 */
void StemClass::dropElement(size_t tail)
{
    CompoundVariableTail resolved_tail(tail);
    dropElement(resolved_tail);
}

/**
 * Drop an array element for an API class.
 *
 * @param tail   The direct tail value.
 */
void StemClass::dropElement(const char *tail)
{
    CompoundVariableTail resolved_tail(tail);
    dropElement(resolved_tail);
}


/**
 * Drop an element using a resolved tail value.
 *
 * @param resolved_tail
 *               The target tail element.
 */
void StemClass::dropElement(CompoundVariableTail &resolved_tail)
{
    CompoundTableElement *variable = findCompoundVariable(resolved_tail);
    if (variable != OREF_NULL)
    {
        variable->drop();
    }
}


/**
 * Create a full compound name from a constructed compound taile.
 *
 * @param tailPart The constructed tail element.
 *
 * @return The fully resolved string name of the element.
 */
RexxString *StemClass::createCompoundName(CompoundVariableTail &tailPart)
{
    return tailPart.createCompoundName(stemName);
}


/******************************************************************************/
/* Function:  Below are a series of comparison routines used by the merge sort*/
/*            library function when sorting stems.                            */
/******************************************************************************/
int compare_asc_i(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return arg1->sortCaselessCompare(arg2);
}


int compare_asc_i_cols(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return arg1->sortCaselessCompare(arg2, sd->startColumn, sd->columnLength);
}


int compare_asc(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return arg1->sortCompare(arg2);
}


int compare_asc_cols(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return arg1->sortCompare(arg2, sd->startColumn, sd->columnLength);
}


int compare_desc(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return -arg1->sortCompare(arg2);
}


int compare_desc_cols(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return -arg1->sortCompare(arg2, sd->startColumn, sd->columnLength);
}


int compare_desc_i(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return -arg1->sortCaselessCompare(arg2);
}


int compare_desc_i_cols(SortData *sd, RexxString *arg1, RexxString *arg2)
{
    return -arg1->sortCaselessCompare(arg2, sd->startColumn, sd->columnLength);
}


/**
 * The merge sort routine.  This will partition the data in to
 * two sections, mergesort each partition, then merge the two
 * partitions together.
 *
 * @param sd         The sort information.
 * @param comparator The comparator object used for the compares.
 * @param strings    The input set of strings.
 * @param working    The working array (same size as the sorted array).
 * @param left       The left bound of the partition.
 * @param right      The right bounds of the parition.
 */
void StemClass::mergeSort(SortData *sd, int (*comparator)(SortData *, RexxString *, RexxString *), RexxString **strings, RexxString **working, size_t left, size_t right)
{
    size_t len = right - left + 1;
    // use insertion sort for small arrays
    if (len <= 7) {
        for (size_t i = left + 1; i <= right; i++) {
            RexxString *current = strings[i];
            RexxString *prev = strings[i - 1];
            if (comparator(sd, current, prev) < 0) {
                size_t j = i;
                do {
                    strings[j--] = prev;
                } while (j > left && comparator(sd, current, prev = strings[j - 1]) < 0);
                strings[j] = current;
            }
        }
        return;
    }

    size_t mid = (right + left) / 2;
    mergeSort(sd, comparator, strings, working, left, mid);
    mergeSort(sd, comparator, strings, working, mid + 1, right);
    merge(sd, comparator, strings, working, left, mid + 1, right);
}


/**
 * Perform a merge of two sort partitions.
 *
 * @param sd         The sort descriptor.
 * @param comparator The comparator used to compare elements.
 * @param strings    The input strings.
 * @param working    A working array used for the merge operations.
 * @param left       The left bound for the merge.
 * @param mid        The midpoint for the merge.
 * @param right      The right bound of merge (inclusive).
 */
void StemClass::merge(SortData *sd, int (*comparator)(SortData *, RexxString *, RexxString *), RexxString **strings, RexxString **working, size_t left, size_t mid, size_t right)
{
    size_t leftEnd = mid - 1;
    // merging

    // if arrays are already sorted - no merge
    if (comparator(sd, strings[leftEnd], strings[mid]) <= 0) {
        return;
    }

    size_t leftCursor = left;
    size_t rightCursor = mid;
    size_t workingPosition = left;

    // use merging with exponential search
    do
    {
        RexxString *fromVal = strings[leftCursor];
        RexxString *rightVal = strings[rightCursor];
        if (comparator(sd, fromVal, rightVal) <= 0)
        {
            size_t leftInsertion = find(sd, comparator, strings, rightVal, -1, leftCursor + 1, leftEnd);
            size_t toCopy = leftInsertion - leftCursor + 1;
            arraycopy(strings, leftCursor, working, workingPosition, toCopy);
            workingPosition += toCopy;
            working[workingPosition++] = rightVal;
            // now we've added this
            rightCursor++;
            // step over the section we just copied...which might be
            // all of the remaining section
            leftCursor = leftInsertion + 1;
        }
        else
        {
            size_t rightInsertion = find(sd, comparator, strings, fromVal, 0, rightCursor + 1, right);
            size_t toCopy = rightInsertion - rightCursor + 1;
            arraycopy(strings, rightCursor, working, workingPosition, toCopy);
            workingPosition += toCopy;
            // insert the right-hand value
            working[workingPosition++] = fromVal;
            leftCursor++;
            rightCursor = rightInsertion + 1;
        }
    } while (right >= rightCursor && mid > leftCursor);

    // copy rest of array.  If we've not used up the left hand side,
    // we copy that.  Otherwise, there are items on the right side
    if (leftCursor < mid)
    {
        arraycopy(strings, leftCursor, working, workingPosition, mid - leftCursor);
    }
    else
    {
        arraycopy(strings, rightCursor, working, workingPosition, right - rightCursor + 1);
    }

    arraycopy(working, left, strings, left, right - left + 1);
}


/**
 * copy segments of one array into another.
 *
 * @param source The source array
 * @param start  The starting index of the source copy
 * @param target The target array
 * @param index  The target copy index
 * @param count  The number of items to count.
 */
void StemClass::arraycopy(RexxString **source, size_t start, RexxString **target, size_t index, size_t count)
{
    for (size_t i = start; i < start + count; i++)
    {
        target[index++] = source[i];
    }
}


/**
 * Finds the place in the given range of specified sorted array, where the
 * element should be inserted for getting sorted array. Uses exponential
 * search algorithm.
 *
 * @param sd         The sort descriptor
 * @param comparator The comparator used to compare pairs of elements.
 * @param strings    The input set of strings.
 * @param val        object to be inserted
 * @param bnd        possible values 0,-1. "-1" - val is located at index more then
 *                   elements equals to val. "0" - val is located at index less
 *                   then elements equals to val.
 * @param left       The left bound of the insert operation.
 * @param right      The right bound for the insert.
 *
 * @return The insertion point.
 */
size_t StemClass::find(SortData *sd, int (*comparator)(SortData *, RexxString *, RexxString *), RexxString **strings, RexxString *val, int limit, size_t left, size_t right)
{
    size_t checkPoint = left;
    size_t delta = 1;
    while (checkPoint <= right)
    {
        // if this is too big, then we're moving to the right
        if (comparator(sd, val, strings[checkPoint]) > limit)
        {
            // the left bound is at least this
            left = checkPoint + 1;
        }
        else
        {
            // we've found a right limit.  We can stop scanning here
            right = checkPoint - 1;
            break;
        }
        // step the delta amount
        checkPoint += delta;
        // and double the movement amount
        delta = delta * 2;
    }
    // we should have now limited the bounds for the insertion point
    // now start in the middle and shrink the range with each comparison
    while (left <= right)
    {
        // start in the middle of the current range
        checkPoint = (left + right) / 2;
        if (comparator(sd, val, strings[checkPoint]) > limit)
        {
            // pull in the left end of the range
            left = checkPoint + 1;
        }
        else
        {
            // chop the right range
            right = checkPoint - 1;
        }
    }
    // the left bound is the insertion point
    return left - 1;
}


/**
 * Sort elements of a stem variable as if it was an array.  This
 * routine assumes that element ".0" of the stem contains a size
 * value for the array portion of the elements, and that tail
 * indices ".start" to ".end", inclusive all exist in the tail.
 * Sorting will be performed on the string values of all elements,
 * and in the final result, all values will be replaced by the
 * string values.
 *
 * @param prefix
 * @param order
 * @param type
 * @param _first
 * @param last
 * @param firstcol
 * @param lastcol
 *
 * @return
 */
bool StemClass::sort(RexxString *prefix, int order, int type, size_t _first, size_t last, size_t firstcol, size_t lastcol)
{
    SortData sd;

    sd.startColumn = 0;
    sd.columnLength = 0;

    CompoundVariableTail stem_size(prefix, (size_t)0);
    CompoundTableElement *size_element = findCompoundVariable(stem_size);
    if (size_element == OREF_NULL)
    {
        reportException(Error_Incorrect_call_stem_size);
        return false;
    }
    RexxInternalObject *size_value = size_element->getVariableValue();
    if (size_value == OREF_NULL)
    {
        reportException(Error_Incorrect_call_stem_size);
        return false;
    }

    size_t count;
    // get the integer value of this.  It must be a valid numeric value.
    if (!size_value->unsignedNumberValue(count, Numerics::DEFAULT_DIGITS))
    {
        reportException(Error_Incorrect_call_stem_size);
        return false;
    }
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return true;
    }

    // if this is not specified, sort to the end
    if (last == (size_t)Numerics::MAX_WHOLENUMBER)
    {
        last = count;
    }

    // verify we're fully within the bounds
    if (_first > count || last > count)
    {
        reportException(Error_Incorrect_call_stem_range, count);
        return false;
    }
    size_t bounds = last - _first + 1;

    // get an array item and protect it.  We need to have space for
    // the the variable anchors, the variable values, and a working buffer for the merge. */
    ArrayClass *array = new_array(bounds * 3);
    ProtectedObject p1(array);

    size_t i;
    size_t j;
    for (j = 1, i = _first; i <= last; i++, j++)
    {
        CompoundVariableTail nextStem(prefix, (size_t)i);
        CompoundTableElement *next_element = findCompoundVariable(nextStem);

        if (next_element == OREF_NULL)
        {
            reportException(Error_Incorrect_call_stem_sparse_array, i);
            return false;
        }

        RexxInternalObject *nextValue = next_element->getVariableValue();
        if (nextValue == OREF_NULL)
        {
            reportException(Error_Incorrect_call_stem_sparse_array, i);
            return false;
        }
        // force this to a string value.
        nextValue = nextValue->requestString();
        // now anchor both in the sorting array
        array->put(next_element, j);
        array->put(nextValue, j + bounds);
    }

    // the data to be sorted
    RexxString **aData = (RexxString **)array->data(bounds + 1);
    // the merge sort work area
    RexxString **working = (RexxString **)array->data((bounds * 2) + 1);

    {
        // we're releasing kernel access during the process.  The sort is being
        // done on a locally allocated array, so this will not be accessed by another thread.
        // All the rest of the operations are thread safe.
        UnsafeBlock block;

        if ((firstcol == 1) && (lastcol == (size_t)Numerics::MAX_WHOLENUMBER))
        {
            /* no special columns to check */
            switch (type)
            {
                case SORT_CASESENSITIVE:
                    mergeSort(&sd, order == SORT_ASCENDING ? compare_asc : compare_desc,
                        aData, working, 0, bounds - 1);
                    break;
                case SORT_CASEIGNORE:
                    mergeSort(&sd, order == SORT_ASCENDING ? compare_asc_i : compare_desc_i,
                        aData, working, 0, bounds - 1);
                  break;
            }
        }
        else
        {
            /* set columns to sort */
            sd.startColumn = firstcol - 1;   // zero base for the sort compare
            sd.columnLength = lastcol - firstcol + 1;

            switch (type)
            {
              case SORT_CASESENSITIVE:
                  mergeSort(&sd, order == SORT_ASCENDING ? compare_asc_cols : compare_desc_cols,
                      aData, working, 0, bounds - 1);
                  break;
              case SORT_CASEIGNORE:
                  mergeSort(&sd, order == SORT_ASCENDING ? compare_asc_i_cols : compare_desc_i_cols,
                      aData, working, 0, bounds - 1);
                  break;
            }
        }
    }


    /* The values have now been sorted.  We now need to set each */
    /* each variable back to its new value. */
    for (i = 1; i <= bounds; i++)
    {
        CompoundTableElement *element = (CompoundTableElement *)array->get(i);
        RexxObject *_value = (RexxObject *)array->get(i + bounds);
        element->set(_value);
    }
    return true;
}


/**
 * Retrieve an iterator for the stem object.
 *
 * @return An iterator for traversing the stem object.
 */
CompoundVariableTable::TableIterator StemClass::iterator()
{
    return tails.iterator();
}

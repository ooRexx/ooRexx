/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                               StemClass.cpp    */
/*                                                                            */
/* Primitive Stem variable class                                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxVariable.hpp"
#include "SupplierClass.hpp"
#include "DirectoryClass.hpp"
#include "StemClass.hpp"
#include "RexxCompoundTail.hpp"

// singleton class instance
RexxClass *RexxStem::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxStem::createInstance()
{
    CLASS_CREATE(Stem, "Stem", RexxClass);
}


RexxStem::RexxStem(
    RexxString *name)                  /* the variable name                 */
/******************************************************************************/
/* Function:  Initialize a STEM class item                                    */
/******************************************************************************/
{
    if (name == OREF_NULL)               /* no explicit default value?        */
    {
        name = OREF_NULLSTRING;            /* set a null string                 */
    }
    else
    {
        /* must get a string here            */
        name = stringArgument(name, ARG_ONE);
    }
    OrefSet(this, this->stemName, name); /* fill in the name                  */
    OrefSet(this, this->value, name);    /* fill in the default value         */
    tails.init(this);                    /* initialize the tail table         */
                                         /* create a tails table              */
    this->dropped = true;                /* no explicit value                 */
}

RexxObject *RexxStem::copy(void)
/******************************************************************************/
/* Function:  Copy a stem collection object                                   */
/******************************************************************************/
{
    /* make a copy of ourself (this also */
    /* copies the object variables       */
    RexxStem *newObj = (RexxStem *)this->RexxObject::copy();
    ProtectedObject p(newObj);
    newObj->copyFrom(tails);             /* have the tail table copy itself   */
    return newObj;                       /* return the new object             */
}

/**
 * Copy the tails from another stem object into this stem.
 *
 * @param _tails The source tail collection.
 */
void RexxStem::copyFrom(RexxCompoundTable &_tails)
{
    tails.init(this);
    tails.copyFrom(_tails);
}


void RexxStem::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->value);
    memory_mark(this->stemName);
    memory_mark(this->objectVariables);
    markCompoundTable();
}

void RexxStem::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->value);
    memory_mark_general(this->stemName);
    memory_mark_general(this->objectVariables);
    markGeneralCompoundTable();
}

void RexxStem::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxStem)

   flatten_reference(newThis->value, envelope);
   flatten_reference(newThis->stemName, envelope);
   flatten_reference(newThis->objectVariables, envelope);
   flattenCompoundTable();

  cleanUpFlatten
}

void RexxStem::setValue(
    RexxObject *_value)                 /* new variable value                */
/******************************************************************************/
/* Function:  Set a new stem default value                                    */
/******************************************************************************/
{
    OrefSet(this, this->value, _value);  /* set the new value                 */
    this->dropped = false;               /* now have an explict value         */
}

void RexxStem::dropValue()
/******************************************************************************/
/* Function:  Drop a stem value -- this reinitializes it                      */
/******************************************************************************/
{
    /* reset to the default value        */
    OrefSet(this, this->value, this->stemName);
    this->dropped = true;                /* no explict value any more         */
}


/**
 * Retrieve the assigned stem value.
 *
 * @return The default stem value.
 */
RexxObject *RexxStem::getStemValue()
{
    return value;
}


RexxObject *RexxStem::unknown(
    RexxString *msgname,               /* unknown message name              */
    RexxArray  *arguments)             /* message arguments                 */
/******************************************************************************/
/* Function:  Forward an unknown message to the value of the stem.            */
/******************************************************************************/
{
    /* validate the name                 */
    msgname = stringArgument(msgname, ARG_ONE);
    requiredArgument(arguments, ARG_TWO);        /* need an argument array            */
                                         /* get this as an array              */
    arguments = (RexxArray  *)REQUEST_ARRAY(arguments);
    if (arguments == TheNilObject)       /* didn't convert?                   */
    {
        /* raise an error                    */
        reportException(Error_Incorrect_method_noarray, IntegerTwo);
    }
    /* just send the message on          */
    return this->value->sendMessage(msgname, arguments);
}

RexxObject *RexxStem::bracket(
    RexxObject **tailElements,         /* tail elements                     */
    size_t      argCount)              /* number of tail elements           */
/******************************************************************************/
/* Function:  Resolve the "stem.[a,b,c]" to the equivalent stem.a.b.c form,   */
/*            with all of the indices taken as constants                      */
/******************************************************************************/
{
    if (argCount == 0)                   /* default value request?            */
    {
        return this->value;                /* just return the default value     */
    }
                                           /* create a searchable tail from the array elements */
    RexxCompoundTail resolved_tail(tailElements, argCount);
    /* now look up this element */
    return evaluateCompoundVariableValue(OREF_NULL, stemName, &resolved_tail);
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
RexxObject *RexxStem::hasIndex(RexxObject **tailElements, size_t argCount)
{
    if (argCount == 0)
    {
        return TheTrueObject;          // we always have something here
    }
    // compose the tail element
    RexxCompoundTail resolved_tail(tailElements, argCount);
    // see if we have a compound
    RexxCompoundElement *compound = findCompoundVariable(&resolved_tail);
    // if there's a variable there, and it has a real value, then
    // this is true.
    if (compound != OREF_NULL && compound->getVariableValue() != OREF_NULL)
    {
        return TheTrueObject;
    }
    // nope, we got nuttin'
    return TheFalseObject;
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
RexxObject *RexxStem::remove(RexxObject **tailElements, size_t argCount)
{
    // if asked to remove the default value, reset this back to the name
    if (argCount == 0)
    {
        // replace with the name and return the old value.
        RexxObject *oldValue = this->value;
        OrefSet(this, value, getName());
        return oldValue;
    }

    // compose the tail element
    RexxCompoundTail resolved_tail(tailElements, argCount);
    RexxCompoundElement *compound = findCompoundVariable(&resolved_tail);
    // if there's a variable there, and it has a real value, then
    // we have something to remove
    if (compound != OREF_NULL && compound->getVariableValue() != OREF_NULL)
    {
        // get the value, which is the return value, and drop the variable.
        RexxObject *oldValue = compound->getVariableValue();
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
RexxObject *RexxStem::hasItem(RexxObject *target)
{
    RexxCompoundElement *variable = findByValue(target);
    return variable == OREF_NULL ? TheFalseObject : TheTrueObject;
}



/**
 * Remove an item from the collection.
 *
 * @param target The object of interest.
 *
 * @return .true if the object is in the collection, .false otherwise.
 */
RexxObject *RexxStem::removeItem(RexxObject *target)
{
    RexxCompoundElement *compound = findByValue(target);
    // if there's a variable there, and it has a real value, then
    // we have something to remove
    if (compound != OREF_NULL && compound->getVariableValue() != OREF_NULL)
    {
        // get the value, which is the return value, and drop the variable.
        RexxObject *oldValue = compound->getVariableValue();
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
RexxObject *RexxStem::index(RexxObject *target)
{
    RexxCompoundElement *variable = findByValue(target);
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
RexxObject *RexxStem::itemsRexx()
{
    return new_integer(items());
}


RexxObject *RexxStem::bracketEqual(
    RexxObject **tailElements,         /* tail elements                     */
    size_t      argCount)              /* number of tail elements           */
/******************************************************************************/
/* Function:  Resolve the "stem.[a,b,c]=" to the equivalent stem.a.b.c= form, */
/*            with all of the indices taken as constants                      */
/******************************************************************************/
{
    RexxVariable * variable;             /* resolved variable element         */
    RexxObject   * new_value;            /* newly assigned value              */

    if (argCount == 0)                   /* have nothing at all?              */
    {
        /* this is an error                  */
        reportException(Error_Incorrect_method_noarg, IntegerOne);
    }
    new_value = tailElements[0];         /* get the new value                 */
    if (new_value == OREF_NULL)          /* nothing given?                    */
    {
        /* this is an error also             */
        reportException(Error_Incorrect_method_noarg, IntegerOne);
    }

    if (argCount == 1)
    {                 /* just setting the default value?   */
        if (isOfClass(Stem, new_value))        // stem value as default?  don't allow this as it leads to recursion loops
        {
            reportException(Error_Execution_nostem);
        }
        /* set the new default value         */
        OrefSet(this, this->value, new_value);
        this->tails.clear();               /* clear out the dictionary          */
        this->dropped = false;             /* now have an explicit value        */
        return this->value;                /* just return the default value     */
    }
    /* create a searchable tail from the array elements */
    RexxCompoundTail resolved_tail(tailElements + 1, argCount - 1);
    variable = getCompoundVariable(&resolved_tail);
    variable->set(new_value);            /* set the new value                 */
    return OREF_NULL;                    /* never returns anything            */
}


RexxArray  *RexxStem::makeArray()
/******************************************************************************/
/* Function:  Extract as an array the tails of a stem.                        */
/******************************************************************************/
{
    return this->tailArray();            /* extract the array item            */
}

RexxString *RexxStem::stringValue()
/******************************************************************************/
/* Function:  Forward string request on to the default value                  */
/******************************************************************************/
{
                                       /* return the objects string value   */
    return (RexxString *)this->value->stringValue();
}

bool RexxStem::numberValue(wholenumber_t &result, stringsize_t digits)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
    return value->numberValue(result, digits);
}

bool RexxStem::numberValue(wholenumber_t &result)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
    return value->numberValue(result);
}

bool RexxStem::unsignedNumberValue(stringsize_t &result, stringsize_t digits)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
    return value->unsignedNumberValue(result, digits);
}

bool RexxStem::unsignedNumberValue(stringsize_t &result)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
    return value->unsignedNumberValue(result);
}

bool RexxStem::doubleValue(double &result)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a double value           */
/******************************************************************************/
{
    return value->doubleValue(result);
}

RexxNumberString *RexxStem::numberString()
/******************************************************************************/
/* Function:  Forward numberstring request on to the default value            */
/******************************************************************************/
{
                                       /* just forward to default value     */
   return this->value->numberString();
}

RexxInteger *RexxStem::integerValue(size_t precision)
/******************************************************************************/
/* Function:  Forward integer request on to the default value                 */
/******************************************************************************/
{
                                       /* just forward to default value     */
    return this->value->integerValue(precision);
}

RexxObject *RexxStem::request(
    RexxString *makeclass)             /* required class name               */
/******************************************************************************/
/* Function:  Forward all REQUEST messages on to the default value            */
/******************************************************************************/
{
    ProtectedObject result;
    /* Verify we have a string parm      */
    makeclass = stringArgument(makeclass, ARG_ONE)->upper();
    /* array request?                    */
    if (makeclass->strCompare(CHAR_ARRAY))
    {
        if (isOfClass(Stem, this))             /* a real stem object?               */
        {
            /* process here directly             */
            return(RexxObject *)this->makeArray();
        }
        else                               /* go to the real make array method  */
        {
            this->sendMessage(OREF_MAKEARRAY, result);
            return(RexxObject *)result;
        }
    }
    /* just forward on                   */
    this->value->sendMessage(OREF_REQUEST, makeclass, result);
    return(RexxObject *)result;
}

RexxObject *RexxStem::newRexx(
    RexxObject **init_args,           /* subclass init arguments           */
    size_t       argCount)            /* the number of arguments           */
/******************************************************************************/
/* Function:  Create an instance of a stem                                    */
/******************************************************************************/
{
    RexxObject * newObj;              /* newly created queue object        */
    RexxObject * name;                   /* name of the stem item             */

                                         /* break up the arguments            */
    RexxClass::processNewArgs(init_args, argCount, &init_args, &argCount, 1, (RexxObject **)&name, NULL);
    newObj = new RexxStem ((RexxString *)name);   /* get a new stem                    */
    newObj->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    /* does object have an UNINT method  */
    if (((RexxClass *)this)->hasUninitDefined())
    {
        newObj->hasUninit();              /* Make sure everyone is notified.   */
    }
                                          /* Initialize the new instance       */
    newObj->sendMessage(OREF_INIT, init_args, argCount);
    return newObj;                       /* return the new object             */
}

void *RexxStem::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new stem object                                        */
/******************************************************************************/
{
    /* Get new object                    */
    return new_object(size, T_Stem);
}


RexxCompoundElement *RexxStem::getCompoundVariable(
    RexxCompoundTail *name)             /* tail name                         */
/******************************************************************************/
/* Function:  Get an item from the variable dictionary, adding a new empty    */
/*             variable entry if it wasn't found.                             */
/******************************************************************************/
{
                                        /* get/create an entry in the table */
    return tails.findEntry(name, true)->realVariable();
}


RexxCompoundElement *RexxStem::exposeCompoundVariable(
    RexxCompoundTail *name)             /* tail name                         */
/******************************************************************************/
/* Function:  Get an item from the variable dictionary, adding a new empty    */
/*             variable entry if it wasn't found.                             */
/******************************************************************************/
{
    // first see if the compound variable already exists.  If it does, then
    // it might represent an explicitly dropped varaible.  We leave this along
    // in that case.
    RexxCompoundElement *variable = tails.findEntry(name, false);
    // ok, it's already there, return the real variable value.
    if (variable != OREF_NULL)
    {
        return variable->realVariable();
    }

    // we're creating a real variable.  If the stem has an explicit value,
    // then we need to set that now.
    variable = tails.findEntry(name, true)->realVariable();
    /* if this variable does not have a value, we need to check the */
    /* stem for a default value and assign it. */
    if (variable->getVariableValue() == OREF_NULL) {
        if (!dropped) {                 /* stem have a default value?        */
            variable->set(value);       /* get the stem's value explicitly   */
        }
    }
    return variable;
}


RexxCompoundElement *RexxStem::findCompoundVariable(
    RexxCompoundTail *name)             /* tail name                         */
/******************************************************************************/
/* Function:  Get an item from the variable dictionary, without creating a    */
/*            new variable if it doesn't exist.                               */
/******************************************************************************/
{
    /* get, but don't create an entry in the table */
    RexxCompoundElement *variable = tails.findEntry(name);
    if (variable != OREF_NULL)
    {
        return variable->realVariable();/* return the real target */
    }
    return OREF_NULL;
}


void RexxStem::dropCompoundVariable(
    RexxCompoundTail *name)             /* tail name                         */
/******************************************************************************/
/* Function:  Mark a variable as dropped.                                     */
/******************************************************************************/
{
    RexxVariable *variable = getCompoundVariable(name);/* look up this tail element         */
    variable->drop();                    /* drop the variable entry           */
}


RexxCompoundElement *RexxStem::nextVariable(
  RexxNativeActivation *activation)    /* Hosting Native Act.               */
/******************************************************************************/
/* Function:  Return the "next" variable of a variable traversal              */
/******************************************************************************/
{
    RexxObject *_value;                  /* variable value                    */

                                         /* get the last saved stem           */
    RexxCompoundElement *variable = activation->compoundElement();

    while (variable != OREF_NULL)
    {      /* while we have more to process     */
           /* get the value                     */
        _value = variable->getVariableValue();
        if (_value != OREF_NULL)
        {         /* not a dropped variable?           */
            activation->setCompoundElement(tails.next(variable));
            return variable;                 /* give this one back                */
        }
        variable = tails.next(variable);   /* step to the next table item       */
    }
    /* clear out the saved element       */
    activation->setCompoundElement(OREF_NULL);
    activation->setNextStem(OREF_NULL);

    return OREF_NULL;                    /* return end of table indicator     */
}

void RexxStem::setCompoundVariable(
    RexxCompoundTail *name,            /* tail name                         */
    RexxObject *_value)                /* value to assign to variable name  */
/******************************************************************************/
/* Function:  Set a new variable value                                        */
/******************************************************************************/
{
    /* see if we have an entry for this  */
    RexxCompoundElement *variable = getCompoundVariable(name);
    variable->set(_value);               /* and perform the set               */
}

RexxArray *RexxStem::tailArray()
/******************************************************************************/
/* Function:  Return all indices as an array                                  */
/******************************************************************************/
{
    RexxCompoundElement *variable;       /* table variable entry              */
    RexxArray  *array;                   /* returned array                    */
    size_t      count;                   /* count of variables                */

    array = new_array(items());          /* get the array                     */
    count = 1;                           /* start at the beginning again      */

    variable = tails.first();            /* get the first variable            */
    while (variable != OREF_NULL)
    {      /* while more values to process      */
           /* this a real variable?             */
        if (variable->getVariableValue() != OREF_NULL)
        {
            /* add to our array                  */
            array->put(variable->getName(), count++);
        }
        variable = tails.next(variable); /* go get the next one               */
    }
    return array;                        /* return the array item             */
}

RexxObject *RexxStem::evaluateCompoundVariableValue(
     RexxActivation *context,           /* the evaluation context (can be NULL) */
     RexxString *stemVariableName,     // the stem variable name this was evaluated from (used for noValue)
     RexxCompoundTail *resolved_tail)   /* the search tail                   */
/******************************************************************************/
/* Function:  Retrieve a compound variable, returning the default value if the*/
/*            variable does not exist.  This includes NOVALUE handling.       */
/******************************************************************************/
{
    RexxCompoundElement *variable;       /* the real variable                 */
    RexxObject   *_value;                /* final variable value              */
    RexxString   *tail_name;             /* formatted tail name               */

                                         /* get the compound variable         */
    variable = findCompoundVariable(resolved_tail);
    if (variable == OREF_NULL)
    {         /* variable does not exist?          */
        if (!dropped)                      /* stem have a default value?        */
        {
            _value = this->value;            /* get the stems value               */
        }
        else
        {                             /* need to use name                  */
                                      /* create a string version of the name */
            tail_name = resolved_tail->createCompoundName(stemVariableName);
            // the tail_name is the fully resolved variable, used for NOVALUE reporting.
            // the defaultValue is the value that's returned as the expression result,
            // which is derived from the stem object.
            RexxObject *defaultValue = resolved_tail->createCompoundName(stemName);
            /* take care of any novalue situations */
            _value = handleNovalue(context, tail_name, defaultValue, variable);
        }
    }
    else
    {
        /* get the variable value            */
        _value = variable->getVariableValue();
        if (_value == OREF_NULL)
        {         /* explicitly dropped variable?      */
                  /* create a string version of the name */
            tail_name = resolved_tail->createCompoundName(stemName);
            // the tail_name is the fully resolved variable, used for NOVALUE reporting.
            // the defaultValue is the value that's returned as the expression result,
            // which is derived from the stem object.
            RexxObject *defaultValue = resolved_tail->createCompoundName(stemName);
            /* take care of any novalue situations */
            _value = handleNovalue(context, tail_name, defaultValue, variable);
        }
    }
    return _value;                       /* and finally return the value */
}

RexxObject *RexxStem::getCompoundVariableValue(
     RexxCompoundTail *resolved_tail)   /* the search tail                   */
/******************************************************************************/
/* Function:  Retrieve a compound variable, returning the default value if the*/
/*            variable does not exist.  This does NOT raise NOVALUE conditions*/
/******************************************************************************/
{
    RexxCompoundElement *variable;       /* the real variable                 */
    RexxObject   *_value;                /* final variable value              */

                                         /* get the compound variable         */
    variable = findCompoundVariable(resolved_tail);
    if (variable == OREF_NULL)
    {         /* variable does not exist?          */
        if (!dropped)                      /* stem have a default value?        */
        {
            return this->value;              /* get the stems value               */
        }
        else
        {                             /* need to use name                  */
                                      /* create a string version of the name */
            return(RexxObject *)resolved_tail->createCompoundName(stemName);
        }
    }
    else
    {
        /* get the variable value            */
        _value = variable->getVariableValue();
        if (_value == OREF_NULL)
        {         /* explicitly dropped variable?      */
                  /* create a string version of the name */
            _value = (RexxObject *)resolved_tail->createCompoundName(stemName);
        }
    }
    return _value;                       /* and finally return the value */
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
RexxObject *RexxStem::getCompoundVariableRealValue(RexxCompoundTail *resolved_tail)
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



RexxObject *RexxStem::realCompoundVariableValue(
     RexxCompoundTail *resolved_tail)   /* the search tail                   */
/******************************************************************************/
/* Function:  Retrieve a compound variable, returning the default value if the*/
/*            variable does not exist.  This does not handle NOVALUEs.        */
/******************************************************************************/
{
    RexxCompoundElement *variable;      /* the real variable                 */

    /* get the compound variable         */
    variable = findCompoundVariable(resolved_tail);
    if (variable == OREF_NULL)
    {         /* variable does not exist?          */
        if (!dropped)                      /* stem have a default value?        */
        {
            return this->value;              /* get the stems value               */
        }
        else
        {                             /* need to use name                  */
            return OREF_NULL;                /* nothing to return                 */
        }
    }
    /* get the variable value            */
    return variable->getVariableValue();
}


RexxObject *RexxStem::handleNovalue(
    RexxActivation *context,           /* the execution context for the request */
    RexxString *name,                  // the fully resolved compound name
    RexxObject *defaultValue,          // the default value to use
    RexxCompoundElement *variable)     // the resolved variable element
/******************************************************************************/
/* Function:  Process a nonvalue condition for a stem variable retrieval.     */
/*            If a context is provided, the novalue is handled in its         */
/*            context.                                                        */
/******************************************************************************/
{
    /* are we doing this directly for method execution? */
    if (context != OREF_NULL)
    {
        /* the context may need to do additional work */
        return context->handleNovalueEvent(name, defaultValue, variable);
    }
    else
    {
        return name;                 /* just use the name                 */
    }
}


void RexxStem::expose(
    RexxCompoundElement *old_variable) /* the parent compound variable     */
/******************************************************************************/
/* Function:  Retrieve a compound variable, returning the default value if the*/
/*            variable does not exist.  This includes NOVALUE handling.       */
/******************************************************************************/
{
    /* create the equivalent in this stem */
    RexxCompoundElement *new_variable = tails.findEntry(old_variable->getName(), true);
    new_variable->expose(old_variable);  /* make the association between the two */
}


/**
 * Return all items in the stem.
 *
 * @return An array of all items in the stem.
 */
RexxArray *RexxStem::allItems()
{
    // now we know how big the return result will be, get an array and
    // populate it, using the same traversal logic as before
    RexxArray *array = new_array(items());
    // we index the array with a origin-one index, so we start with one this time
    size_t count = 1;

    RexxCompoundElement *variable = tails.first();
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
RexxCompoundElement *RexxStem::findByValue(RexxObject *target)
{
    RexxCompoundElement *variable = tails.first();
    while (variable != OREF_NULL)
    {
        RexxObject *_value = variable->getVariableValue();
        // if this has a value, and we have a match, then return it
        if (_value != OREF_NULL && target->equalValue(_value))
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
size_t RexxStem::items()
{
    size_t count = 0;

    RexxCompoundElement *variable = tails.first();
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
RexxObject *RexxStem::empty()
{
    tails.clear();      // just clear the tails.
    return OREF_NULL;
}


/**
 * Test if the stem is empty.
 *
 * @return True if the stem is empty, false otherwise.
 */
RexxObject *RexxStem::isEmpty()
{
    return (items() == 0) ? TheTrueObject : TheFalseObject;
}



/**
 * Create an array of all indexes of the stem.
 *
 * @return An array of all tail names used in the stem.
 */
RexxArray  *RexxStem::allIndexes()
{
    return this->tailArray();            /* extract the array item            */
}


/**
 * Create a supplier for the stem, returning the tail names as
 * the indexes and the values as the items.
 *
 * @return A supplier instance.
 */
RexxSupplier *RexxStem::supplier()
{
    // essentially the same logic as allItems(), but both the item and the
    // tail value are accumulated.
    size_t count = 0;
    RexxCompoundElement *variable = tails.first();
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
    RexxArray *tailValues = new_array(count);
    RexxArray *values = new_array(count);
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
RexxDirectory *RexxStem::toDirectory()
{
    RexxDirectory *result = new_directory();
    ProtectedObject p1(result);
    RexxCompoundElement *variable = tails.first();
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
void RexxStem::setElement(const char *_tail, RexxObject *_value)
{
    RexxCompoundTail resolved_tail(_tail);
    RexxVariable *variable = getCompoundVariable(&resolved_tail);
    variable->set(_value);                /* set the new value                 */
}


/**
 * Set a single stem variable object using a simple string
 * value tail as a result of an api call.
 *
 * @param tail   The index of the target value.
 * @param value  The new value to assign.
 */
void RexxStem::setElement(size_t _tail, RexxObject *_value)
{
    RexxCompoundTail resolved_tail(_tail);
    RexxVariable *variable = getCompoundVariable(&resolved_tail);
    variable->set(_value);              /* set the new value                 */
}


/**
 * Evaluate an array element for an API class.
 *
 * @param tail   The direct tail value.
 *
 * @return The object value.  If the stem element does not exist or
 *         has been dropped, this returns OREF_NULL.
 */
RexxObject *RexxStem::getElement(size_t _tail)
{

    RexxCompoundTail resolved_tail(_tail);

    return getElement(&resolved_tail);
}

/**
 * Evaluate an array element for an API class.
 *
 * @param tail   The direct tail value.
 *
 * @return The object value.  If the stem element does not exist or
 *         has been dropped, this returns OREF_NULL.
 */
RexxObject *RexxStem::getElement(const char *_tail)
{

    RexxCompoundTail resolved_tail(_tail);

    return getElement(&resolved_tail);
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
RexxObject *RexxStem::getElement(RexxCompoundTail *resolved_tail)
{
    // see if we have a variable...if we do, return its value (a dropped variable
    // has a value of OREF_NULL).  If not found, return OREF_NULL;
    RexxCompoundElement *variable = findCompoundVariable(resolved_tail);
    if (variable != OREF_NULL)
    {
        return variable->getVariableValue();
    }
    return OREF_NULL;
}


/**
 * Drop an array element for an API class.
 *
 * @param tail   The direct tail value.
 */
void RexxStem::dropElement(size_t _tail)
{

    RexxCompoundTail resolved_tail(_tail);

    dropElement(&resolved_tail);
}

/**
 * Drop an array element for an API class.
 *
 * @param tail   The direct tail value.
 */
void RexxStem::dropElement(const char *_tail)
{

    RexxCompoundTail resolved_tail(_tail);

    dropElement(&resolved_tail);
}


/**
 * Drop an element using a resolved tail value.
 *
 * @param resolved_tail
 *               The target tail element.
 */
void RexxStem::dropElement(RexxCompoundTail *resolved_tail)
{
    RexxCompoundElement *variable = findCompoundVariable(resolved_tail);
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
RexxString *RexxStem::createCompoundName(RexxCompoundTail *tailPart)
{
    return tailPart->createCompoundName(stemName);
}


/******************************************************************************/
/* Function:  Below are a series of comparison routines used by the qsort()   */
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
void RexxStem::mergeSort(SortData *sd, int (*comparator)(SortData *, RexxString *, RexxString *), RexxString **strings, RexxString **working, size_t left, size_t right)
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
void RexxStem::merge(SortData *sd, int (*comparator)(SortData *, RexxString *, RexxString *), RexxString **strings, RexxString **working, size_t left, size_t mid, size_t right)
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
void RexxStem::arraycopy(RexxString **source, size_t start, RexxString **target, size_t index, size_t count)
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
size_t RexxStem::find(SortData *sd, int (*comparator)(SortData *, RexxString *, RexxString *), RexxString **strings, RexxString *val, int limit, size_t left, size_t right)
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


bool RexxStem::sort(RexxString *prefix, int order, int type, size_t _first, size_t last, size_t firstcol, size_t lastcol)
/******************************************************************************/
/* Function:  Sort elements of a stem variable as if it was an array.  This   */
/*            routine assumes that element ".0" of the stem contains a size   */
/*            value for the array portion of the elements, and that tail      */
/*            indices ".start" to ".end", inclusive all exist in the tail.    */
/*            Sorting will be performed on the string values of all elements, */
/*            and in the final result, all values will be replaced by the     */
/*            string values.                                                  */
/******************************************************************************/
{
    SortData sd;

    sd.startColumn = 0;
    sd.columnLength = 0;

    RexxCompoundTail stem_size(prefix, (size_t)0);
    RexxCompoundElement *size_element = findCompoundVariable(&stem_size);
    if (size_element == OREF_NULL) {
        return false;
    }
    RexxObject *size_value = size_element->getVariableValue();
    if (size_value == OREF_NULL) {
        return false;
    }

    stringsize_t count;
    /* get the integer value of this.  It must be a valid numeric */
    /* value. */
    if (!size_value->unsignedNumberValue(count, Numerics::DEFAULT_DIGITS))
    {
        return false;
    }
    if (count == 0)         // if the count is zero, sorting is easy!
    {
        return true;
    }

    /* if this is not specified, sort to the end */
    if (last == SIZE_MAX)
    {
        last = count;
    }

    /* verify we're fully within the bounds */
    if (_first > count || last > count) {
        return false;
    }
    size_t bounds = last - _first + 1;

    // get an array item and protect it.  We need to have space for
    // the the variable anchors, the variable values, and a working buffer for the merge. */
    RexxArray *array = new_array(bounds * 3);
    ProtectedObject p1(array);

    size_t i;
    size_t j;
    for (j = 1, i = _first; i <= last; i++, j++)
    {
        RexxCompoundTail nextStem(prefix, (size_t)i);
        RexxCompoundElement *next_element = findCompoundVariable(&nextStem);

        if (next_element == OREF_NULL) {
            return false;
        }

        RexxObject *nextValue = next_element->getVariableValue();
        if (nextValue == OREF_NULL) {
            return false;
        }
        /* force this to a string value. */
        nextValue = REQUEST_STRING(nextValue);
        /* now anchor both in the sorting array */
        array->put((RexxObject *)next_element, j);
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

        if ((firstcol == 0) && (lastcol == SIZE_MAX)) {
          /* no special columns to check */
          switch (type) {

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
          sd.startColumn = firstcol;
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
    for (i = 1; i <= bounds; i++) {
        RexxCompoundElement *element = (RexxCompoundElement *)array->get(i);
        RexxObject *_value = array->get(i + bounds);
        element->set(_value);
    }
    return true;
}

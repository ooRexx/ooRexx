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
/* REXX Kernel                                         SupplierClass.c        */
/*                                                                            */
/* Primitive Supplier Class                                                   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"

// singleton class instance
RexxClass *RexxSupplier::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxSupplier::createInstance()
{
    CLASS_CREATE(Supplier, "Supplier", RexxClass);
}


RexxSupplier::RexxSupplier(
  RexxArray  *_values,                 /* array of values                   */
  RexxArray  *_indexes )               /* array of indexes                  */
/****************************************************************************/
/* Function:  Initialize a supplier                                         */
/****************************************************************************/
{
  OrefSet(this, this->values, _values); /* store the values array            */
                                       /* and the index array also          */
  OrefSet(this, this->indexes, _indexes);
  this->position = 1;                  /* set the first position            */
}


RexxSupplier::RexxSupplier()
/****************************************************************************/
/* Function:  Initialize a supplier                                         */
/****************************************************************************/
{
    values = OREF_NULL;
    indexes = OREF_NULL;
}


void RexxSupplier::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->values);
  memory_mark(this->indexes);
  memory_mark(this->objectVariables);
}

void RexxSupplier::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->values);
  memory_mark_general(this->indexes);
  memory_mark_general(this->objectVariables);
}

void RexxSupplier::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxSupplier)

   flatten_reference(newThis->values, envelope);
   flatten_reference(newThis->indexes, envelope);

  cleanUpFlatten
}

RexxInteger *RexxSupplier::available()
/****************************************************************************/
/* Function:  Return indication of object availability                      */
/****************************************************************************/
{
                                       /* check the array size              */
  return (this->position > this->values->size()) ? TheFalseObject : TheTrueObject;
}

RexxObject  *RexxSupplier::next()
/****************************************************************************/
/* Function:  Step to the next element of the supplier                      */
/****************************************************************************/
{
                                       /* already gone past the end?        */
  if (this->position > this->values->size())
                                       /* oops, give an error               */
    reportException(Error_Incorrect_method_supplier);
  this->position++;                    /* step the position pointer         */
  return OREF_NULL;                    /* this returns nothing              */
}

RexxObject  *RexxSupplier::value()
/****************************************************************************/
/* Function:  Retrieve the value of a collection item                       */
/****************************************************************************/
{
    /* already gone past the end?        */
    if (this->position > this->values->size())
    {
        /* oops, give an error               */
        reportException(Error_Incorrect_method_supplier);
    }
    /* get the value                     */
    RexxObject *_value = this->values->get(this->position);
    if (_value == OREF_NULL)              /* returned nothing?                 */
    {
        _value = TheNilObject;              /* change this to .nil               */
    }
    return _value;                        /* return this value                 */
}

RexxObject  *RexxSupplier::index()
/****************************************************************************/
/* Function:  Retrieve the index of a collection item                       */
/****************************************************************************/
{
    RexxObject *_value;                   /* supplier value                    */

    /* already gone past the end?        */
    if (this->position > this->values->size())
    {
        /* oops, give an error               */
        reportException(Error_Incorrect_method_supplier);
    }
    if (this->indexes == OREF_NULL)      /* no index array given?             */
    {
        /* just return current position      */
        return(RexxObject *)new_integer(this->position);
    }
    /* already gone past the end?        */
    if (this->position > this->indexes->size())
    {
        _value = TheNilObject;              /* no value to return                */
    }
    else
    {
        /* get the value                     */
        _value = this->indexes->get(this->position);
        if (_value == OREF_NULL)            /* returned nothing?                 */
        {
            _value = TheNilObject;            /* change this to .nil               */
        }
    }
    return _value;                        /* and return the value              */
}

void *RexxSupplier::operator new(size_t size)
/****************************************************************************/
/* Function:  Create a new supplier object                                  */
/****************************************************************************/
{
                                       /* Get new object                    */
    return new_object(size, T_Supplier);
}


/**
 * Supplier initializer for suppliers created via
 * .supplier~new(values, indexes).
 *
 * @param values  The values array object
 * @param indexes The indexes array object
 *
 * @return Nothing
 */
RexxObject *RexxSupplier::initRexx(RexxArray *_values, RexxArray *_indexes)
{
    requiredArgument(_values, ARG_ONE);           // both values are required
    requiredArgument(_indexes, ARG_TWO);

    // now verify both values
    RexxArray *new_values = REQUEST_ARRAY(_values);
    RexxArray *new_indexes = REQUEST_ARRAY(_indexes);
    if (new_values == (RexxArray  *)TheNilObject || new_values->getDimension() != 1)
    {
        reportException(Error_Incorrect_method_noarray, values);
    }
    if (new_indexes == (RexxArray  *)TheNilObject || new_indexes->getDimension() != 1)
    {
        reportException(Error_Incorrect_method_noarray, indexes);
    }

    OrefSet(this, this->values, new_values);
    OrefSet(this, this->indexes, new_indexes);
    this->position = 1;
    return OREF_NULL;
}



RexxObject  *RexxSupplierClass::newRexx(
    RexxObject **init_args,            /* subclass init arguments           */
    size_t argCount)                   /* count of arguments                */
/****************************************************************************/
/* Function:  Public REXX supplier new method                               */
/****************************************************************************/
{
    RexxObject *newObj = new RexxSupplier();
    newObj->setBehaviour(this->getInstanceBehaviour());
    if (this->hasUninitDefined())
    {
        newObj->hasUninit();
    }
                                       /* Initialize the new instance       */
    newObj->sendMessage(OREF_INIT, init_args, argCount);
    return newObj;                       /* return the new supplier           */
}

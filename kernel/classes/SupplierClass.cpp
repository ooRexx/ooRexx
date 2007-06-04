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
/* REXX Kernel                                                SupplierClass.c        */
/*                                                                            */
/* Primitive Supplier Class                                                   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"


RexxSupplier::RexxSupplier(
  RexxArray  *values,                  /* array of values                   */
  RexxArray  *indexes )                /* array of indexes                  */
/****************************************************************************/
/* Function:  Initialize a supplier                                         */
/****************************************************************************/
{
  OrefSet(this, this->values, values); /* store the values array            */
                                       /* and the index array also          */
  OrefSet(this, this->indexes, indexes);
  this->position = 1;                  /* set the first position            */
}


RexxSupplier::RexxSupplier()
/****************************************************************************/
/* Function:  Initialize a supplier                                         */
/****************************************************************************/
{
}


void RexxSupplier::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->values);
  memory_mark(this->indexes);
  memory_mark(this->objectVariables);
  cleanUpMemoryMark
}

void RexxSupplier::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->values);
  memory_mark_general(this->indexes);
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral
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
    report_exception(Error_Incorrect_method_supplier);
  this->position++;                    /* step the position pointer         */
  return OREF_NULL;                    /* this returns nothing              */
}

RexxObject  *RexxSupplier::value()
/****************************************************************************/
/* Function:  Retrieve the value of a collection item                       */
/****************************************************************************/
{
  RexxObject *value;                   /* supplier value                    */

                                       /* already gone past the end?        */
  if (this->position > this->values->size())
                                       /* oops, give an error               */
    report_exception(Error_Incorrect_method_supplier);
                                       /* get the value                     */
  value = this->values->get(this->position);
  if (value == OREF_NULL)              /* returned nothing?                 */
    value = TheNilObject;              /* change this to .nil               */
  return value;                        /* return this value                 */
}

RexxObject  *RexxSupplier::index()
/****************************************************************************/
/* Function:  Retrieve the index of a collection item                       */
/****************************************************************************/
{
  RexxObject *value;                   /* supplier value                    */

                                       /* already gone past the end?        */
  if (this->position > this->values->size())
                                       /* oops, give an error               */
    report_exception(Error_Incorrect_method_supplier);
  if (this->indexes == OREF_NULL)      /* no index array given?             */
                                       /* just return current position      */
    return (RexxObject *)new_integer(this->position);
                                       /* already gone past the end?        */
  if (this->position > this->indexes->size())
    value = TheNilObject;              /* no value to return                */
  else {
                                       /* get the value                     */
    value = this->indexes->get(this->position);
    if (value == OREF_NULL)            /* returned nothing?                 */
      value = TheNilObject;            /* change this to .nil               */
  }
  return value;                        /* and return the value              */
}

void *RexxSupplier::operator new(size_t size)
/****************************************************************************/
/* Function:  Create a new supplier object                                  */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* Get new object                    */
  newObject = new_object(size);
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheSupplierBehaviour);
                                       /* fill in the hash value            */
  newObject->hashvalue = HASHOREF(newObject);
  ClearObject(newObject);              /* clear out the state data area     */
                                       /* Initialize this new method        */
  return newObject;                    /* return the new object             */
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
RexxObject *RexxSupplier::initRexx(RexxArray *values, RexxArray *indexes)
{
    required_arg(values, ONE);           // both values are required
    required_arg(indexes, TWO);

    // now verify both values
    RexxArray *new_values = REQUEST_ARRAY(values);
    RexxArray *new_indexes = REQUEST_ARRAY(indexes);
    if (new_values == (RexxArray  *)TheNilObject || new_values->getDimension() != 1)
    {
        report_exception1(Error_Incorrect_method_noarray, values);
    }
    if (new_indexes == (RexxArray  *)TheNilObject || new_indexes->getDimension() != 1)
    {
        report_exception1(Error_Incorrect_method_noarray, indexes);
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
    RexxObject *newObject = new RexxSupplier();
    BehaviourSet(newObject, this->instanceBehaviour);
    if (this->uninitDefined())
    {
        newObject->hasUninit();
    }
                                       /* Initialize the new instance       */
    newObject->sendMessage(OREF_INIT, init_args, argCount);
    return newObject;                    /* return the new supplier           */
}

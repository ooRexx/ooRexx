/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* REXX Kernel                                                RexxPointer.cpp */
/*                                                                            */
/* Primitive Pointer Class                                                    */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "PointerClass.hpp"
#include "ActivityManager.hpp"


RexxClass *RexxPointer::classInstance = OREF_NULL;   // singleton class instance
RexxPointer *RexxPointer::nullPointer = OREF_NULL;  // single version of a null pointer


void RexxPointer::createInstance()
/******************************************************************************/
/* Function:  Create initial bootstrap objects                                */
/******************************************************************************/
{
    CLASS_CREATE(Pointer, "Pointer", RexxClass);
    TheNullPointer = new_pointer(NULL);       // a NULL pointer object
}


/**
 * Primitive-level comparison of pointer values.
 *
 * @param other  The other comparison value.
 *
 * @return True if the two objects are equal, false otherwise.
 */
RexxObject *RexxPointer::equal(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);            /* must have the other argument      */

    if (!isOfClass(Pointer, other))
    {
        return TheFalseObject;
    }

    return this->pointer() == ((RexxPointer *)other)->pointer() ? TheTrueObject : TheFalseObject;
}


/**
 * Primitive-level comparison of pointer values.
 *
 * @param other  The other comparison value.
 *
 * @return True if the two objects are equal, false otherwise.
 */
RexxObject *RexxPointer::notEqual(RexxObject *other)
{
    requiredArgument(other, ARG_ONE);            /* must have the other argument      */

    if (!isOfClass(Pointer, other))
    {
        return TheTrueObject;
    }

    return this->pointer() != ((RexxPointer *)other)->pointer() ? TheTrueObject : TheFalseObject;
}


/**
 * Override of the default hash value method.
 */
HashCode RexxPointer::getHashValue()
{
    // generate a hash from the pointer value...but obscure this a touch to get
    // a better bit distribution
    return (HashCode)(~((uintptr_t)pointerData));
}


void *RexxPointer::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new pointer object                                     */
/******************************************************************************/
{
                                       /* Get new object                    */
  RexxObject *newObject = new_object(size, T_Pointer);
  newObject->setHasNoReferences();     /* this has no references            */
  return (void *)newObject;            /* return the new object             */
}


RexxObject *RexxPointer::newRexx(RexxObject **args, size_t argc)
/******************************************************************************/
/* Function:  Allocate a pointer object from Rexx code.                       */
/******************************************************************************/
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}


/**
 * Format this as a character string value.
 *
 * @return The character string value.
 */
RexxString *RexxPointer::stringValue()
{
    return Numerics::pointerToString(pointer());
}


/**
 * Test if this is a null pointer value.
 *
 * @return True if the pointer value is NULL, false for non-null.
 */
RexxObject *RexxPointer::isNull()
{
    return pointer() == NULL ? TheTrueObject : TheFalseObject;
}

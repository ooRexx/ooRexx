/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* REXX Kernel                                                              */
/*                                                                          */
/* The StringTable class code                                               */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "SetClass.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *SetClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void SetClass::createInstance()
{
    CLASS_CREATE(Set);
}


/**
 * Create a new table object.
 *
 * @param size   the size of the object.
 *
 * @return Storage for creating a table object.
 */
void *SetClass::operator new (size_t size)
{
    return new_object(size, T_Set);
}


/**
 * Create a new set instance from Rexx code.
 *
 * @param args     The new arguments.
 * @param argCount The count of new arguments.
 *
 * @return The constructed instance.
 */
RexxObject *SetClass::newRexx(RexxObject **args, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // create the new identity table item (this version does not have a backing contents yet).
    Protected<SetClass> temp = new SetClass(true);
    // finish setting this up.
    classThis->completeNewObject(temp, args, argCount);

    // make sure this has been completely initialized
    temp->initialize();
    return temp;
}


/**
 * Create a list item and populate with the argument items.
 *
 * @param args     Pointer to the arguments.
 * @param argCount The number of arguments.
 *
 * @return The populated list object.
 */
RexxObject *SetClass::ofRexx(RexxObject **args, size_t  argCount)
{
    // create a list item of the appopriate type.
    Protected<SetClass> newSet = (SetClass *)newRexx(NULL, 0);

    // add all of the arguments
    for (size_t i = 0; i < argCount; i++)
    {
        RexxObject *item = args[i];
        // omitted arguments not allowed here.
        if (item == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, i + 1);
        }
        newSet->put(item);
    }
    return newSet;
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*                                                UseArgVariableRef.cpp       */
/*                                                                            */
/* The USE ARG side of variable reference handling                            */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "UseArgVariableRef.hpp"
#include "RexxVariable.hpp"


/**
 * Allocate storage for a use arg reference handler
 *
 * @param size   The base object size.
 *
 * @return Rexx object storage for the object.
 */
void *UseArgVariableRef::operator new(size_t size)
{
    return new_object(size, T_UseArgVariableRef);
}


/**
 * Construct a variable reference handler for USE ARG
 *
 * @param variable_name
 *                  The name of the variable.
 * @param var_index The variable index in the local context.
 */
UseArgVariableRef::UseArgVariableRef(RexxVariableBase *v)
{
    variable = v;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void UseArgVariableRef::live(size_t liveMark)
{
    memory_mark(variable);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void UseArgVariableRef::liveGeneral(MarkReason reason)
{
    memory_mark_general(variable);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void UseArgVariableRef::flatten(Envelope *envelope)
{
    setUpFlatten(UseArgVariableRef)

    flattenRef(variable);

    cleanUpFlatten
}


/**
 * Alias a local variable name to a supplied variable reference
 * from another context.
 *
 * @param context The variable context
 * @param var     The variable obtained from a VariableReference that we are
 *                going to alias.
 */
void UseArgVariableRef::aliasVariable(RexxActivation *context, RexxVariable *var)
{
    // just send this
    variable->alias(context, var);
}


/**
 * Indicate if this variable is a stem variable.
 *
 * @return True if the variable to reference is a stem.
 */
bool UseArgVariableRef::isStem()
{
    return isOfClass(StemVariableTerm, variable);
}


/**
 * Get the name of the indirect variable for tracing
 *
 * @return The string name of the variable
 */
RexxString *UseArgVariableRef::getName()
{
    return ((RexxVariable *)variable)->getName();
}

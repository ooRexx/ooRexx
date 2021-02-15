/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                        RexxLocalVariables.cpp  */
/*                                                                            */
/* Primitive Local Variable Cache                                             */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxLocalVariables.hpp"
#include "RexxActivation.hpp"


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void RexxLocalVariables::live(size_t liveMark)
{
    // We can only mark if full initialized
    if (locals != NULL)
    {
       RexxVariable **entry;
       RexxVariable **top;

       // mark all of the stack entries;
       for (entry = locals, top = entry + size; entry < top; entry++)
       {
           memory_mark(*entry);
       }
    }
    // also mark any created variable dictionary
    memory_mark(dictionary);
    memory_mark(owner);
    memory_mark(objectVariables);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxLocalVariables::liveGeneral(MarkReason reason)
{
    // We can only mark if full initialized
    if (locals != NULL)
    {
        RexxVariable **entry;
        RexxVariable **top;

        for (entry = locals, top = entry + size; entry < top; entry++)
        {
            memory_mark_general(*entry);
        }
    }
    memory_mark_general(dictionary);
    memory_mark_general(owner);
    memory_mark_general(objectVariables);
}


/**
 * Migrate the local variable cache to a new activity.
 *
 * @param activity The new target activity.
 */
void RexxLocalVariables::migrate(Activity *activity)
{
    RexxVariable **oldFrame = locals;
    // ask the activity to allocate a new stack frame.
    activity->allocateLocalVariableFrame(this);
    // copy the existing entries over to the new stack.  Our
    // VariableDictionary pointer is fine.
    memcpy(locals, oldFrame, sizeof(RexxVariable *) * size);
}


/**
 * Do a more extensive search for a variable, without creating
 * one if it doesn't exist.  This is only called after we've had
 * a miss on the variable table.
 *
 * @param name   The target name.
 * @param index  The target index (can be 0, which requires a search).
 *
 * @return The variable object corresponding to the name, or NULL if
 *         not found.
 */
RexxVariable *RexxLocalVariables::findVariable(RexxString *name, size_t index)
{
    // we need to search for this by name, so if we have a variable
    // dictionary already created, this is a quick search.
    if (dictionary != OREF_NULL)
    {
        // the lookup might be using a non-zero index and failing
        // because the variable might have been created dynamically (using
        // VALUE() or interpret, for example).  If we have a non-zero
        // index, store the value from the dictionary into the slot.
        RexxVariable *variable = dictionary->resolveVariable(name);

        // if we've had a miss on the local directory but we're auto exposing,
        // we need to check the object dictionary
        if (variable == OREF_NULL && autoExpose())
        {
            // Important note: If we are auto exposing, then all additional references
            // need to come from the object variables, creating one if needed, so
            // we are using getVariable() rather than resolveVariable() here.
            variable = objectVariables->getVariable(name);
            // if we found this in the object variables, add to the
            // local dictionary too
            if (variable != OREF_NULL)
            {
                dictionary->addVariable(name, variable);
            }
        }

        // if we have an index, fill in the cache entry.
        if (index != 0)
        {
            locals[index] = variable;
        }
        return variable;
    }
    // first dynamic lookup...we might need to create the variable
    // dictionary.
    else
    {
        // if this is a non-targetted lookup, we don't know the slot
        // this variable needs to be stored in, or this is a
        // dynamically accessed variable that may not have a slot.  We
        // might need to create a variable dictionary for this.
        if (index == 0)
        {
            // if we haven't created a variable dictionary yet, scan
            // the set of variables looking for one we may have created
            // earlier.
            for (size_t i = 0; i < size; i++)
            {
                RexxVariable *variable = locals[i];
                // if the slot exists at this position, check the name
                if (variable != OREF_NULL)
                {
                    if (name->strCompare(variable->getName()))
                    {
                        return variable;
                    }
                }
            }
        }
        // we have an index, check the slot directly
        else
        {
            // check the slot...it might be there
            if (locals[index] != OREF_NULL)
            {
                return locals[index];
            }
        }

        // not found in the local table, but this could be an autoexpose situation
        // we need to check the object dictionary
        if (autoExpose())
        {
            // Important note: If we are auto exposing, then all additional references
            // need to come from the object variables, creating one if needed, so
            // we are using getVariable() rather than resolveVariable() here.
            RexxVariable *variable = objectVariables->getVariable(name);
            // we did not have a dictionary up to this point, so create it now and
            // add it.
            createDictionary();
            dictionary->addVariable(name, variable);
            // add this to the slot now
            if (index != 0)
            {
                locals[index] = variable;
            }
            return variable;
        }
        // a non-zero index with no created variable dictionary means this
        // variable cannot exist.  Just return NULL.
    }
    // not found
    return OREF_NULL;
}



/**
 * Enable auto exposing of variables.  This will switch lookup of
 * variables on the first reference to perform an implicit
 * expose operation.
 *
 * @param ov     The object variables from the current scope used
 *               for the expose operation.
 */
void RexxLocalVariables::setAutoExpose(VariableDictionary *ov)
{
    objectVariables = ov;
}


/**
 * Create a local variable object of the given name and store
 * it at the given location.
 *
 * @param name   The variable name.
 * @param index  The target index position.
 *
 * @return The created or resolved variable object.
 */
RexxVariable *RexxLocalVariables::lookupVariable(RexxString *name, size_t index)
{
    // if this is a non-targetted lookup, we don't know the slot
    // this variable needs to be stored in, or this is a
    // dynamically accessed variable that may not have a slot.  We
    // might need to create a variable dictionary for this.
    if (index == 0)
    {
        // if we haven't created a variable dictionary yet, scan
        // the set of variables looking for one we may have created
        // earlier.
        if (dictionary == OREF_NULL)
        {
            for (size_t i = 0; i < size; i++)
            {
                // we're looking for a variable where the names match.
                RexxVariable *variable = locals[i];
                if (variable != OREF_NULL)
                {
                    if (name->strCompare(variable->getName()))
                    {
                        return variable;
                    }
                }
            }
            // go create the dictionary and populate it with our variable set.
            createDictionary();
        }

        // if we're not auto exposing, just lookup the variable which will also
        // create it.
        if (!autoExpose())
        {
            // retrieve this from the variable dictionary now, which will create
            // the item we need.
            return dictionary->getVariable(name);
        }
        else
        {
            // this might have been previously created.  check the dictionary
            // first, then create if necessary
            RexxVariable *variable = dictionary->resolveVariable(name);
            if (variable != OREF_NULL)
            {
                return variable;
            }

            // get the variable from the object variable dictionary
            // and add it to the local dictionary.
            variable = objectVariables->getVariable(name);
            dictionary->addVariable(name, variable);
            return variable;
        }
    }
    else
    {
        // normal local variable lookup, just create the local variable
        // and return it.
        if (!autoExpose())
        {
            RexxVariable *variable;
            // if we've had to create a dictionary for this because of
            // prior dynamic access, then we need to retrieve the
            // variable from the dictionary.
            if (dictionary != OREF_NULL)
            {
                variable = dictionary->getVariable(name);
            }
            // we've already had a cache miss, so we're creating a variable.
            else
            {
                variable = owner->newLocalVariable(name);
            }
            // fill in the cache slot for the next lookup and return
            // the new variable.
            locals[index] = variable;
            return variable;
        }
        else
        {
            // if we've had to create a dictionary for this because of
            // prior dynamic access, then we need to retrieve the
            // variable from the dictionary.
            if (dictionary != OREF_NULL)
            {
                // this might have been previously created.  check the dictionary
                // first, then create if necessary
                RexxVariable *variable = dictionary->resolveVariable(name);
                if (variable != OREF_NULL)
                {
                    return variable;
                }
            }

            // get the variable from the object variable dictionary
            RexxVariable *variable = objectVariables->getVariable(name);
            // if we've had to create a dictionary for this because of
            // prior dynamic access, then we need to retrieve the
            // variable from the dictionary.
            if (dictionary != OREF_NULL)
            {
                dictionary->addVariable(name, variable);
            }
            // fill in the cache slot for the next lookup and return
            // the new variable.
            locals[index] = variable;
            return variable;
        }
    }
}


/**
 * Create a local stem variable variable object of the given name and store
 * it at the given location.
 *
 * @param name   The variable name.
 * @param index  The variable cache slot (can be zero)
 *
 * @return The located or created variable object.
 */
RexxVariable *RexxLocalVariables::lookupStemVariable(RexxString *name, size_t index)
{
    // if this is a non-targetted lookup, we don't know the slot
    // this variable needs to be stored in, or this is a
    // dynamically accessed variable that may not have a slot.  We
    // might need to create a variable dictionary for this.
    if (index == 0)
    {
        // if we haven't created a variable dictionary yet, scan
        // the set of variables looking for one we may have created
        // earlier.
        if (dictionary == OREF_NULL)
        {
            for (size_t i = 0; i < size; i++)
            {
                // we if find a matching variable, return it immediately
                RexxVariable *variable = locals[i];
                if (variable != OREF_NULL)
                {
                    if (name->strCompare(variable->getName()))
                    {
                        return variable;
                    }
                }
            }

            // go create the dictionary and populate it with our variable set.
            createDictionary();
        }

        // if we're not auto exposing, just lookup the variable which will also
        // create it.
        if (!autoExpose())
        {
            // retrieve this from the variable dictionary now, which will create
            // the item we need.

            // have the dictionary create this for us
            return dictionary->getStemVariable(name);
        }
        else
        {
            // this might have been previously created.  check the dictionary
            // first, then create if necessary
            RexxVariable *variable = dictionary->resolveVariable(name);
            if (variable != OREF_NULL)
            {
                return variable;
            }

            // get the variable from the object variable dictionary
            // and add it to the local dictionary.
            variable = objectVariables->getStemVariable(name);
            dictionary->addVariable(name, variable);
            return variable;
        }
    }
    else
    {
        if (!autoExpose())
        {
            // if we've had to create a dictionary for this because of
            // prior dynamic access, then we need to retrieve the
            // variable from the dictionary.

            RexxVariable *variable;
            if (dictionary != OREF_NULL)
            {
                // create from the dictionary and add this to the cache at
                // the target location.
                variable = dictionary->getStemVariable(name);
                locals[index] = variable;
            }
            else
            {
                // create a new variable from the local context and
                // add it to the cache.
                variable = owner->newLocalVariable(name);
                locals[index] = variable;

                // stem variables are initialized as soon as they
                // are created, using a stem object with the same name.
                StemClass *stemtable = new StemClass(name);
                variable->set(stemtable);
            }
            // and return the new variable
            return variable;
        }
        else
        {
            // if we've had to create a dictionary for this because of
            // prior dynamic access, then we need to retrieve the
            // variable from the dictionary.
            if (dictionary != OREF_NULL)
            {
                // this might have been previously created.  check the dictionary
                // first, then create if necessary
                RexxVariable *variable = dictionary->resolveVariable(name);
                if (variable != OREF_NULL)
                {
                    return variable;
                }
            }
            // get the variable from the object variable dictionary
            RexxVariable *variable = objectVariables->getStemVariable(name);
            if (dictionary != OREF_NULL)
            {
                dictionary->addVariable(name, variable);
            }
            // fill in the cache slot for the next lookup and return
            // the new variable.
            locals[index] = variable;
            return variable;
        }
    }
}


/**
 * Update a local variable context with a provided variable object.
 *
 * @param variable The variable used for the replacement.
 */
void RexxLocalVariables::updateVariable(RexxVariable *variable)
{
    RexxVariable *oldVariable = OREF_NULL;
    RexxString *name = variable->getName();

    // scan the set of variables looking for one we may have created
    // earlier.
    for (size_t i = 0; i < size; i++)
    {
        oldVariable = locals[i];
        if (oldVariable != OREF_NULL)
        {
            // if we find a match, replace the slot variable at this
            // location with the new variable object.
            if (name->strCompare(oldVariable->getName()))
            {
                locals[i] = variable;

                // if we have a dictionary created, replace the entry in that as well.
                if (dictionary != OREF_NULL)
                {
                    dictionary->addVariable(name, variable);
                    return;
                }
                break;
            }
        }
    }


    // we didn't find a static one, so this is a completely dynamic
    // update.  Make sure we have a dynamic dictionary and insert
    // this entry.
    if (dictionary == OREF_NULL)
    {
        createDictionary();
    }
    // add the variable to the dictionary
    dictionary->addVariable(name, variable);
}


/**
 * Create a local variable object of the given name and store
 * it at the given location.
 *
 * @param name   The variable name.
 * @param index  The target index position.
 *
 * @return The created or resolved variable object.
 */
void RexxLocalVariables::aliasVariable(RexxString *name, size_t index, RexxVariable *variable)
{
    // we're going to be putting a variable into the local table using a name
    // other than the actual variable name. This means we need to get the dictionary
    // created so we can get this added into the dictionary under the aliased name.
    // if we don't then the dictionary will get created using the real variable name.
    // Just requesting the dictionary is sufficient to do this.
    getDictionary();

    // see if there is already a variable with this name in the context (which might be
    // an autoExposed object variable
    RexxVariable *oldVar = findVariable(name, index);

    // we can only perform the aliasing if this an unassigned local variable
    if (oldVar != OREF_NULL && !oldVar->isAliasable())
    {
        reportException(Error_Execution_reference_variable_in_use, name);
    }

    // if we have a local variable with no value, this would have been created
    // with USE LOCAL, so just overwrite the existing variable with the aliased one


    // Put uses the name from the variable, so we repeat that here.
    // We will most likly have an index since this is only used on the USE ARG
    // instruction, which tends not to be interpreted.
    if (index != 0)
    {
        // plug the referenced variable into the target slot.
        locals[index] = variable;
    }

    // we already created the dictionary, so we can unconditionally add
    // this value. Note that we are using the alias name, not the name
    // from the referenced variable. This will also replace any variable created via
    // use arg.
    dictionary->addVariable(name, variable);
}


/**
 * Create a variable dictionary for this activation to
 * support dynamic access to variables.  This is created only
 * when dynamic lookup cannot be avoided.
 */
void RexxLocalVariables::createDictionary()
{
    // create a dictionary with the recommended size
    dictionary = new_variableDictionary(size);
    for (size_t i = 0; i < size; i++)
    {
        // if we have a variable at this position, insert into the dictionary
        RexxVariable *variable = locals[i];
        if (variable != OREF_NULL)
        {
            dictionary->addVariable(variable->getName(), variable);
        }
    }
}

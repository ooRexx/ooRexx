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
/* REXX Kernel                                        RexxLocalVariables.cpp  */
/*                                                                            */
/* Primitive Local Variable Cache                                             */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxLocalVariables.hpp"
#include "RexxActivation.hpp"

void RexxLocalVariables::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
   RexxVariable **entry;                /* marked stack entry                */
   RexxVariable **top;

                                       /* loop through the stack entries    */
   for (entry = locals, top = entry + size; entry < top; entry++)
   {
       memory_mark(*entry);            /* marking each one                  */
   }

   memory_mark(dictionary);            /* also mark any created vdict       */
}

void RexxLocalVariables::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
   RexxVariable **entry;                /* marked stack entry                */
   RexxVariable **top;

                                       /* loop through the stack entries    */
   for (entry = locals, top = entry + size; entry < top; entry++)
   {
       memory_mark_general(*entry);    /* marking each one                  */
   }
   memory_mark_general(dictionary);    /* also mark any created vdict       */
}


void RexxLocalVariables::migrate(RexxActivity *activity)
/******************************************************************************/
/* Function:  Migrate the expression stack to a new activity                  */
/******************************************************************************/
{
    RexxVariable **oldFrame = locals;
    /* allocate a new frame */
    activity->allocateLocalVariableFrame(this);
    /* copy the enties over to the new stack. */
    memcpy(locals, oldFrame, sizeof(RexxVariable *) * size);
}


RexxVariable *RexxLocalVariables::findVariable(RexxString *name, size_t index)
/******************************************************************************/
/* Function:  Do a more extensive search for a variable, without creating     */
/*            one if it doesn't exist.                                        */
/******************************************************************************/
{
    RexxVariable *variable = OREF_NULL;

    /* if we have a dictionary already, we can do a fast lookup. */
    if (dictionary != OREF_NULL)
    {
        variable = dictionary->resolveVariable(name);
        /* if we have an index, fill in the cache entry. */
        if (index != 0)
        {
            /* add this to the variable cache */
            locals[index] = variable;
        }
    }
    else
    {
        /* if this is a non-targetted lookup, we don't know the slot */
        /* this variable needs to be stored in, or this is a */
        /* dynamically accessed variable that may not have a slot.  We */
        /* might need to create a variable dictionary for this. */
        if (index == 0)
        {
            /* if we haven't created a variable dictionary yet, scan */
            /* the set of variables looking for one we may have created */
            /* earlier. */
            size_t i;
            for (i = 0; i < size; i++)
            {
                /* grab the item */
                variable = locals[i];
                /* if the variable at this position exists, check the name. */
                if (variable != OREF_NULL)
                {
                    /* if the names match, this is our target */
                    if (name->memCompare(variable->getName()))
                    {
                        return variable;
                    }
                }
            }
        }
        variable = NULL;      // force this to null, otherwise it returns last variable examined
    }
    return variable;
}

RexxVariable *RexxLocalVariables::lookupVariable(RexxString *name, size_t index)
/******************************************************************************/
/* Function:  Create a local variable object of the given name and store      */
/*            it at the given location.                                       */
/******************************************************************************/
{
    RexxVariable *variable = OREF_NULL;

    /* if this is a non-targetted lookup, we don't know the slot */
    /* this variable needs to be stored in, or this is a */
    /* dynamically accessed variable that may not have a slot.  We */
    /* might need to create a variable dictionary for this. */
    if (index == 0)
    {
        /* if we haven't created a variable dictionary yet, scan */
        /* the set of variables looking for one we may have created */
        /* earlier. */
        if (dictionary == OREF_NULL)
        {
            size_t i;
            for (i = 0; i < size; i++)
            {
                /* grab the item */
                variable = locals[i];
                /* if the variable at this position exists, check the name. */
                if (variable != OREF_NULL)
                {
                    /* if the names match, this is our target */
                    if (name->memCompare(variable->getName()))
                    {
                        return variable;
                    }
                }
            }
            /* go create the dictionary and populate it with our variable set. */
            createDictionary();
        }


        /* get the variable item for this name */
        return dictionary->getVariable(name);
    }
    else
    {
        /* if we've had to create a dictionary for this because of */
        /* prior dynamic access, then we need to retrieve the */
        /* variable from the dictionary. */
        if (dictionary != OREF_NULL)
        {
            variable = dictionary->getVariable(name);
        }
        else
        {
            /* create a new variable item for this */
            variable = owner->newLocalVariable(name);
        }
        /* add this to the variable cache */
        locals[index] = variable;
        /* and return the new variable */
        return variable;
    }
}


RexxVariable *RexxLocalVariables::lookupStemVariable(RexxString *name, size_t index)
/******************************************************************************/
/* Function:  Create a local variable object of the given name and store      */
/*            it at the given location.                                       */
/******************************************************************************/
{
    RexxVariable *variable;

    /* if this is a non-targetted lookup, we don't know the slot */
    /* this variable needs to be stored in, or this is a */
    /* dynamically accessed variable that may not have a slot.  We */
    /* might need to create a variable dictionary for this. */
    if (index == 0)
    {
        /* if we haven't created a variable dictionary yet, scan */
        /* the set of variables looking for one we may have created */
        /* earlier. */
        if (dictionary == OREF_NULL)
        {
            size_t i;
            for (i = 0; i < size; i++)
            {
                /* grab the item */
                variable = locals[i];
                /* if the variable at this position exists, check the name. */
                if (variable != OREF_NULL)
                {
                    /* if the names match, this is our target */
                    if (name->memCompare(variable->getName()))
                    {
                        return variable;
                    }
                }
            }

            /* go create the dictionary and populate it with our variable set. */
            createDictionary();
        }

        /* get the variable item for this name */
        return dictionary->getStemVariable(name);
    }
    else
    {
        /* if we've had to create a dictionary for this because of */
        /* prior dynamic access, then we need to retrieve the */
        /* variable from the dictionary. */
        if (dictionary != OREF_NULL)
        {
            variable = dictionary->getStemVariable(name);
            /* add this to the variable cache */
            locals[index] = variable;
        }
        else
        {
            /* create a new variable item for this */
            variable = owner->newLocalVariable(name);
            /* add this to the variable cache */
            locals[index] = variable;
            /* create a stem object as value     */
            RexxStem *stemtable = new RexxStem(name);
            /* the stem object is the value of   */
            /* stem variable                     */
            variable->set((RexxObject *)stemtable);
        }
        /* and return the new variable */
        return variable;
    }
}

void RexxLocalVariables::updateVariable(RexxVariable *variable)
/******************************************************************************/
/* Function:  Do a more extensive search for a variable, without creating     */
/*            one if it doesn't exist.                                        */
/******************************************************************************/
{
    RexxVariable *oldVariable = OREF_NULL;
    RexxString *name = variable->getName();

    /* if we haven't created a variable dictionary yet, scan    */
    /* the set of variables looking for one we may have created */
    /* earlier.                                                 */
    size_t i;
    for (i = 0; i < size; i++)
    {
        /* grab the item */
        oldVariable = locals[i];
        /* if the variable at this position exists, check the name. */
        if (oldVariable != OREF_NULL)
        {
            /* if the names match, this is our target */
            if (name->memCompare(oldVariable->getName()))
            {
                /* overwrite this */
                locals[i] = variable;

                /* if we have a dictionary, we update that entry too */
                if (dictionary != OREF_NULL)
                {
                    dictionary->put(variable, name);
                    return;
                }
                break;
            }
        }
    }

    /* we didn't find a static one, so this is a completely dynamic */
    /* update.  Make sure we have a dynamic dictionary and insert   */
    /* this entry. */
    if (dictionary == OREF_NULL)
    {
        createDictionary();
    }
    /* add the variable to the dictionary */
    dictionary->put(variable, name);
}

void RexxLocalVariables::createDictionary()
/******************************************************************************/
/* Function:  Create a variable dictionary for this method activation to      */
/*            support dynamic access to variables.  This is created only      */
/*            when dynamic lookup cannot be avoided.                          */
/******************************************************************************/
{
    /* create a dictionary with the recommended size */
    dictionary = new_variableDictionary(size);
    for (size_t i = 0; i < size; i++)
    {
        /* grab the item */
        RexxVariable *variable = locals[i];
        /* if the variable at this position exists, check the name. */
        if (variable != OREF_NULL)
        {
            /* add the variable to the dictionary */
            dictionary->put(variable, variable->getName());
        }
    }
}

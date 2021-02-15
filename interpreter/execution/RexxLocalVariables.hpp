/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Run time local variable cache                                    */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxLocalVariables
#define Included_RexxLocalVariables

#include "FlagSet.hpp"
#include "VariableDictionary.hpp"
#include <string.h>

/**
 * Predefined index values for a stack frame.
 */
typedef enum
{
   VARIABLE_SELF = 1,          // NOTE:  The slots start at 1, not 0.
   VARIABLE_SUPER,
   VARIABLE_RESULT,
   VARIABLE_RC,
   VARIABLE_SIGL,
} VariableFrameIndex;

/**
 * Local variable frame managed by an activation.
 */
class RexxLocalVariables
{
 friend class RexxActivation;
 public:

    typedef enum
    {
        VDICT_NOVALUE,           // novalue traps enabled
        NESTED_INTERNAL,         // this is an internal call without procedure
        METHOD_CONTEXT,          // this is a method context
    } VDictFlag;

    RexxLocalVariables(RexxObject **frames, size_t items) { locals = (RexxVariable **)frames; size = items; }
    RexxLocalVariables() { locals = OREF_NULL; size = 0; }

    // NOTE, not virtual because this is not a subclass of RexxInternalObject.
    void live(size_t);
    void liveGeneral(MarkReason reason);

    void migrate(Activity *);

    /* NOTE:  we add one because the size is actually the index */
    /* number of the last variable in the cache.   The zero-th */
    /* element is used to trigger cache lookup failures. */
    inline void init(RexxActivation *creator, size_t poolSize) { owner = creator; size = poolSize + 1; dictionary = OREF_NULL; objectVariables = OREF_NULL; flags.reset(); }
    inline void setFrame(RexxInternalObject **frame)
    {
        locals = (RexxVariable **)frame;
        memset(locals, 0, sizeof(RexxVariable *) * size);
        // NOTE:  We do NOT reset the variable dictionary.  For a new activation,
        // init() has already reset this.  If we're migrating to a new frame after a reply,
        // then we need to keep the old set of variables active.
    }

    RexxVariable *lookupVariable(RexxString *name, size_t index);

    RexxVariable *findVariable(RexxString *name, size_t index);
    RexxVariable *lookupStemVariable(RexxString *name, size_t index);

    void createDictionary();

    inline VariableDictionary *getDictionary()
    {
        if (dictionary == OREF_NULL)
        {
            createDictionary();
        }
        return dictionary;
    }

    inline void putVariable(RexxVariable *variable, size_t index)
    {
        // this may be a dynamic addition, so we might not know the index
        if (index != 0)
        {
            locals[index] = variable;
            if (dictionary != OREF_NULL)
            {
                dictionary->addVariable(variable->getName(), variable);
            }
        }
        else
        {
            if (dictionary == OREF_NULL)
            {
                createDictionary();
            }
            dictionary->addVariable(variable->getName(), variable);
        }
    }

    void updateVariable(RexxVariable*);
    void setAutoExpose(VariableDictionary *ov);
    void aliasVariable(RexxString *n, size_t index, RexxVariable *var);

    inline RexxVariable *get(size_t index) { return locals[index]; }
    inline RexxVariable *find(RexxString *name, size_t index)
    {
        RexxVariable *variable = get(index);
        if (variable == OREF_NULL)
        {
            variable = findVariable(name, index);
        }
        return variable;
    }

    inline size_t     getSize() { return size; }
    inline void       setNovalueOn() { flags.set(VDICT_NOVALUE); }
    inline void       setNovalueOff() { flags.reset(VDICT_NOVALUE); }
    inline bool       getNovalue() {return flags[VDICT_NOVALUE]; }
    inline void       setNested()  { flags.set(NESTED_INTERNAL); }
    inline void       clearNested()  { flags.reset(NESTED_INTERNAL); }
    inline bool       isNested() { return flags[NESTED_INTERNAL]; }
    inline bool       autoExpose() { return objectVariables != OREF_NULL; }

    inline void       procedure(RexxActivation *activation) { owner = activation; dictionary = OREF_NULL; objectVariables = OREF_NULL; clearNested(); }
    inline void       setDictionary(VariableDictionary *dict) { dictionary = dict; }
    inline VariableDictionary *getNestedDictionary() { return dictionary; }

    static const size_t FIRST_VARIABLE_INDEX = VARIABLE_SIGL;

 protected:

    FlagSet<VDictFlag, 32> flags;        // dictionary control flags
    size_t size;                         // size of the expstack
    RexxActivation *owner;               // the owning activation
    RexxVariable **locals;               // the frame of local variables
    VariableDictionary *dictionary;      // dictionary used for dynamic lookups
    VariableDictionary *objectVariables; // dictionary used for automatic expose operations
};
#endif

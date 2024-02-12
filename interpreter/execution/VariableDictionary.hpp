/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                       VariableDictionary.hpp   */
/*                                                                            */
/* Primitive Variable Dictionary Class Definition                             */
/*                                                                            */
/******************************************************************************/
#ifndef Included_VariableDictionary
#define Included_VariableDictionary

#include "CompoundVariableTable.hpp"
#include "RexxVariable.hpp"
#include "HashContents.hpp"

class StemClass;
class SupplierClass;
class RexxVariableBase;
class DirectoryClass;
class RexxClass;

/**
 * A variable dictionary, used both for object variables
 * and dynamic access to context variables.
 */
class VariableDictionary : public RexxInternalObject
{
 public:

     /**
      * an iterator for iterating over all entries of the variable
      * dictionary.  This will iterate over both simple and compound
      * variables.
      */
     class VariableIterator
     {
         friend class VariableDictionary;

     public:
         inline VariableIterator() {}
         inline ~VariableIterator() {}

         inline bool isAvailable()
         {
             if (dictionary == OREF_NULL)
             {
                 return false;
             }

             // if we're supposed to return a stem value yet, something is available.
             if (returnStemValue)
             {
                 return true;
             }

             if (currentStem != OREF_NULL && stemIterator.isAvailable())
             {
                 return true;
             }
             // unconditionally clear this
             currentStem = OREF_NULL;
             return dictionaryIterator.isAvailable();
         }

         RexxObject *value();
         RexxString *name();
         void next();

         // explicitly terminate an iterator
         inline void terminate()
         {
             dictionary = OREF_NULL;
             currentStem = OREF_NULL;
         }

         // indicate if this is an active iterator or not.
         inline bool isActive()
         {
             return dictionary != OREF_NULL;
         }

     private:
         // constructor for an index iterator
         VariableIterator(VariableDictionary *d);

         VariableDictionary *dictionary;
         HashContents::TableIterator dictionaryIterator;
         StemClass *currentStem;
         CompoundVariableTable::TableIterator stemIterator;
         bool returnStemValue;
     };


           void *operator new(size_t size);
    inline void  operator delete(void *) { }

           VariableDictionary(size_t capacity);
           VariableDictionary(RexxClass *scope);
    inline VariableDictionary(RESTORETYPE restoreType) { ; };

    StringHashContents *allocateContents(size_t bucketSize, size_t capacity);

    void initialize(size_t capacity = DefaultObjectDictionarySize);
    void expandContents();
    void expandContents(size_t capacity );
    void ensureCapacity(size_t delta);
    void checkFull();

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;

    RexxInternalObject  *copy() override;
    void copyValues();
    VariableDictionary *deepCopy();

    RexxObject  *realValue(RexxString *name);
    inline StemClass *getStem(RexxString *stemName) { return (StemClass *)getStemVariable(stemName)->getVariableValue(); }
    RexxVariable *createStemVariable(RexxString *stemName);
    RexxVariable *createVariable(RexxString *stemName);
    void addVariable(RexxString *name, RexxVariable *variable);

    // resolve a variable name entry
    inline RexxVariable *resolveVariable(RexxString *name)
    {
        return (RexxVariable *)contents->get(name);
    }

    // get a variable entry, creating a new one if necessary
    inline RexxVariable *getVariable(RexxString *name)
    {
        // find the entry
        RexxVariable *variable = resolveVariable(name);
        if (variable == OREF_NULL)
        {
            // create a new one if not there.
            variable = createVariable(name);
        }
        return variable;
    }

    // resolve a stem variable entry, creating a new one if not found
    inline RexxVariable *getStemVariable(RexxString *stemName)
    {
        RexxVariable *variable = resolveVariable(stemName);
        if (variable == OREF_NULL)
        {
            variable = createStemVariable(stemName);
        }
        return variable;
    }

    void setCompoundVariable(RexxString *stemName, RexxInternalObject **tail, size_t tailCount, RexxObject *value);
    void dropCompoundVariable(RexxString *stemName, RexxInternalObject **tail, size_t tailCount);
    StringTable *getAllVariables();
    DirectoryClass *getVariableDirectory();
    inline void remove(RexxString *n) { contents->remove(n); }

    void         set(RexxString *, RexxObject *);
    void         drop(RexxString *);
    void         dropStemVariable(RexxString *);
    void         reserve(Activity *);
    void         release(Activity *);
    bool         transfer(Activity *);

    CompoundTableElement *getCompoundVariable(RexxString *stemName, RexxInternalObject **tail, size_t tailCount);
    RexxObject  *getCompoundVariableValue(RexxString *stemName, RexxInternalObject **tail, size_t tailCount);
    RexxObject  *getCompoundVariableRealValue(RexxString *stem, RexxInternalObject **tail, size_t tailCount);

    RexxObject  *realStemValue(RexxString *stemName);

    uint32_t getIdntfr();
    inline unsigned short getReserveCount() { return reserveCount; } // for concurrency trace

    inline bool isScope(RexxClass *otherScope) { return scope == otherScope; }
    inline VariableDictionary *getNextDictionary() { return nextDictionary; }
    inline Activity *getReservingActivity() { return reservingActivity; }

    void setNextDictionary(VariableDictionary *next);
    VariableIterator iterator();

    static const size_t DefaultObjectDictionarySize = 7;

    static RexxVariableBase *getVariableRetriever(RexxString  *variable);
    static RexxVariableBase *getDirectVariableRetriever(RexxString  *variable);
    static RexxVariableBase *buildCompoundVariable(RexxString *variable_name, bool direct);

protected:

    Activity *reservingActivity;         // current reserving activity
    StringHashContents *contents;        // variable dictionary contents
    ArrayClass *waitingActivities;       // list of waiting activities
    unsigned short flags;                // dictionary control flags
    unsigned short reserveCount;         // number of times reserved
    VariableDictionary *nextDictionary;  // chained object dictionary
    RexxClass *scope;                    // scopy of this object dictionary
    uint32_t idntfr;                     // idntfr for concurrency trace
};

inline VariableDictionary *new_variableDictionary(size_t s) { return new VariableDictionary(s); }
inline VariableDictionary *new_objectVariableDictionary(RexxClass *s) { return new VariableDictionary(s); }
#endif

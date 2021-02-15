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
/******************************************************************************/
/* REXX Kernel                                    CompoundVariableTable.hpp   */
/*                                                                            */
/* Balanced binary tree table for stem variables                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_CompoundVariableTable
#define Included_CompoundVariableTable

class StemClass;
class CompoundTableElement;

// macros for embedding within the stem object
#define markCompoundTable() { \
  memory_mark(tails.root); \
  memory_mark(tails.parent);  \
}

#define markGeneralCompoundTable() { \
  memory_mark_general(tails.root); \
  memory_mark_general(tails.parent); \
}

#define flattenCompoundTable() { \
  flattenRef(tails.root); \
  flattenRef(tails.parent); \
}


/**
 * Compound table object embedded within a Stem object.
 * This is not a Rexx internal object, but rather a
 * helper object used as a field within another object
 */
class CompoundVariableTable
{
 friend class StemClass;
 public:
    /**
     * an iterator for iterating over all entries of the table
     */
    class TableIterator
    {
        friend class CompoundVariableTable;

    public:
        inline TableIterator() : contents(OREF_NULL), current(OREF_NULL) { }
        inline ~TableIterator() {}

        inline bool isAvailable()  { return current != OREF_NULL; }
               RexxObject *value();
               CompoundTableElement *variable() { return current; }
               RexxString *name();
               RexxString *name(RexxString *stemName);
               void replace(RexxObject *v);
        inline void next() { current = contents->next(current); }

    private:
        // constructor for an index iterator
        TableIterator(CompoundVariableTable *c)
            : contents(c) { current = contents->first(); }

        CompoundVariableTable *contents;
        CompoundTableElement  *current;
    };

    inline CompoundVariableTable() { ; };

    void copyFrom(CompoundVariableTable &other);
    void init(StemClass *parent);
    void clear();

    inline CompoundTableElement *get(CompoundVariableTail &name) { return findEntry(name); }
    CompoundTableElement *findEntry(CompoundVariableTail &tail);
    CompoundTableElement *findEntry(CompoundVariableTail &tail, bool create);
    CompoundTableElement *findEntry(RexxString *tail, bool create = false);
    void balance(CompoundTableElement *node);
    void moveNode(CompoundTableElement *&anchor, bool toright);
    CompoundTableElement *first();
    CompoundTableElement *findLeaf(CompoundTableElement *node);
    CompoundTableElement *next(CompoundTableElement *node);

    void setParent(StemClass *parent);
    void setRoot(CompoundTableElement *newRoot);
    TableIterator iterator() { return TableIterator(this); }

protected:

    CompoundTableElement *root;               // the root node
    StemClass *parent;                        // link back to the hosting stem
};

#endif

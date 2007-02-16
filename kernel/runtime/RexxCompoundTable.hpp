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
/* REXX Kernel                                          RexxCompoundTable.hpp   */
/*                                                                            */
/* Balanced binary tree table for stem variables                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxCompoundTable
#define Included_RexxCompoundTable

#include "RexxCompoundTail.hpp"

class RexxStem;
class RexxCompoundElement;
                                           /* macros for embedding within the stem object */
#define markCompoundTable() { \
  memory_mark(this->tails.root); \
  memory_mark(this->tails.parent);  \
}

#define markGeneralCompoundTable() { \
  memory_mark_general(this->tails.root); \
  memory_mark_general(this->tails.parent); \
}

#define flattenCompoundTable() { \
  flatten_reference(newThis->tails.root, envelope); \
  flatten_reference(newThis->tails.parent, envelope); \
}

class RexxCompoundTable {
 public:
  inline RexxCompoundTable() { ; };
  void         copy(RexxStem *newObject, RexxStem *oldObject);
  void         init(RexxStem *parent);
  void         clear();
  void dump();
  void dump(RexxCompoundElement*);
  inline RexxCompoundElement *get(RexxCompoundTail *name) { return findEntry(name); }
  RexxCompoundElement *findEntry(RexxCompoundTail *tail)
    /******************************************************************************/
    /* Function:  Search for a compound entry.  This version is optimized for     */
    /*            "find-but-don't create" usage.                                  */
    /******************************************************************************/
    {

        INT          rc;                   /* comparison result          */
        RexxCompoundElement *anchor;       /* pointer to current block   */

        anchor = root;                     /* get root block             */
                                           /* loop through on left branch*/
        while (anchor != NULL) {
                                           /* do the name comparison */
            rc = tail->compare(anchor->getName());
            if (rc > 0) {                  /* left longer?               */
                                           /* take the right branch      */
               anchor =  anchor->right;
               continue;                   /* loop                       */
             }
             else if (rc < 0) {            /* left shorter?              */
                                           /* the the left branch        */
                 anchor = anchor->left;
                 continue;                 /* loop                       */
             }
             else {                        /* names match                */
                 return anchor;            /* return the anchor          */
             }
        }
        return OREF_NULL;                  /* return var not found       */
    }
  RexxCompoundElement *findEntry(RexxCompoundTail *tail, BOOL create);
  RexxCompoundElement *findEntry(RexxString *tail, BOOL create = FALSE);
  void         balance(RexxCompoundElement *node);
  void         moveNode(RexxCompoundElement **anchor, BOOL toright);
  RexxCompoundElement *first();
  RexxCompoundElement *findLeaf(RexxCompoundElement *node);
  RexxCompoundElement *next(RexxCompoundElement *node);

  void         setParent(RexxStem *parent);
  void         setRoot(RexxCompoundElement *newRoot);

  RexxCompoundElement *root;               /* the root node */
  RexxStem *parent;                        /* link back to the hosting stem */
};

#endif

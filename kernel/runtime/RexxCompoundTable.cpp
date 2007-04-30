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
/****************************************************************************/
/* REXX Kernel                                        RexxCompoundTable.c     */
/*                                                                          */
/* Stem object table of compound variables                                  */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "RexxCompoundTable.hpp"
#include "RexxCompoundElement.hpp"

void RexxCompoundTable::init(
    RexxStem *parent)                  /* the parent object we're embedded in */
{
    setParent(parent);                 /* make the parent hook up */
    setRoot(OREF_NULL);                /* no root member */
}

void RexxCompoundTable::copy(
    RexxStem *newObject,               /* the target new object */
    RexxStem *oldObject)               /* the source stem */
{
    newObject->tails.init(newObject);  /* clear the new table */

    RexxCompoundElement *entry = oldObject->tails.first();/* grab the first element */
    while (entry != NULL) {            /* while we have more entry to process */
                                       /* insert an entry in the new table */
        RexxCompoundElement *newEntry = newObject->tails.findEntry(entry->getName(), TRUE);
                                       /* copy over the value */
        newEntry->setValue(entry->variableValue);
        entry = oldObject->tails.next(entry);
    }

}

void RexxCompoundTable::clear()
{
    setRoot(OREF_NULL);                /* clear the anchor */
}


RexxCompoundElement *RexxCompoundTable::findEntry(RexxString *tail, BOOL create)
{
                                       /* process the tail and search */
    RexxCompoundTail resolved_tail(tail);
    return findEntry(&resolved_tail, create);
}


RexxCompoundElement *RexxCompoundTable::findEntry(
    RexxCompoundTail *tail,            /* our tail search target     */
    BOOL create)                       /* we're creating a variable  */
{

    INT          rc;                   /* comparison result          */
    RexxCompoundElement *previous;     /* pointer for tree traversal */
    RexxCompoundElement *anchor;       /* pointer to current block   */

    anchor = root;                     /* get root block             */
    previous = anchor;                 /* save starting point        */
                                       /* loop through on left branch*/
    while (anchor) {
                                       /* do the name comparison */
        rc = tail->compare(anchor->getName());
        if (rc > 0) {                  /* left longer?               */
           previous = anchor;          /* save the previous          */
                                       /* take the right branch      */
           anchor =  anchor->right;
           continue;                   /* loop                       */
         }
         else if (rc < 0) {            /* left shorter?              */
             previous = anchor;        /* save the previous          */
                                       /* the the left branch        */
             anchor = anchor->left;
             continue;                 /* loop                       */
         }
         else {                        /* names match                */
             return anchor;            /* return the anchor          */
         }
    }
    if (!create) {                     /* not a SET operation,       */
      return OREF_NULL;                /* return var not found       */
    }
                                       /* create a new compound variable */
    anchor = new_compoundElement(tail->makeString());

    if (!previous) {                   /* if first insertion         */
      anchor->setParent(OREF_NULL);    /* no parent                  */
      setRoot(anchor);                 /* set the tree top           */
    }
    else {                             /* real insertion             */

      anchor->setParent(previous);     /* set our parent entry       */
      if (rc > 0)                      /* should this be left or     */
                                       /* right on parent tree       */
        previous->setRight(anchor);    /* right                      */
      else
        previous->setLeft(anchor);     /* left                       */
      balance(anchor);                 /* Balance the tree from here */
                                       /* up                         */
    }
    return anchor;                     /* return new block pointer   */
}


void RexxCompoundTable::balance(
    RexxCompoundElement *node)                    /* starting point             */
{

     RexxCompoundElement *parent;                 /* block parent pointer       */
     unsigned short depth;             /* current depth              */
     unsigned short wd;                /* working depth              */

     if (node == root)                 /* this the root?             */
         return;                       /* nothing to Balance         */

     parent = node->parent;            /* step up to block's parent  */
     depth = 1;                        /* initial depth is 1         */

     while (parent) {                  /* while still have a parent  */

         if (parent->right == node) {      /* if on right branch         */
             parent->rightdepth = depth;   /* set right depth            */
                                           /* deeper on left?            */
             if (depth > (wd = parent->leftdepth + (unsigned short)1)) {
                 moveNode(&parent, FALSE);   /* adjust right branch        */
                 depth = parent->rightdepth; /* readjust depth             */
             }
             else {
                 if (wd < depth)           /* left shorter               */
                     return ;              /* done                       */
             }
         }
         else {
             parent->leftdepth = depth;    /* set left depth             */
                                           /* if right shorter           */
             if (depth > (wd = parent->rightdepth + (USHORT)1)) {
                 moveNode(&parent, TRUE);       /* adjust left branch         */
                 depth = parent->leftdepth;     /* readjust depth             */
             }
             else {
                 if (wd < depth)           /* right shorter              */
                     return ;              /* done                       */
             }
         }
         depth++;                           /* increment the depth        */
         node = parent;                     /* step up to current         */
         parent = parent->parent;           /* and lift up one more       */
     }
}

void RexxCompoundTable::moveNode(
    RexxCompoundElement   **anchor,        /* node to move               */
    BOOL         toright)                  /* move direction             */
{
    RexxCompoundElement   *temp;           /* anchor save position       */
    RexxCompoundElement   *work;           /* working block pointer      */
    RexxCompoundElement   *work1;          /* working block pointer      */
    RexxCompoundElement   *work2;          /* working block pointer      */

    temp = *anchor;                        /* save where we are          */

    if (toright) {                         /* move right?                */
        work = temp->left;                 /* get left branch pointer    */
        work1 = temp->left = work->right;  /* move right to left         */
        temp->leftdepth = work->rightdepth;/* adjust left depth value    */
        if (work1)                         /* was a right moved          */
            work1->setParent(temp);        /* set its parent correctly   */
        work->setRight(temp);              /* set new right              */
        work->rightdepth++;                /* adjust its depth           */
    }
    else {
        work = temp->right;                /* get right node             */
        work1 = temp->right = work->left;  /* move rights left node      */
        temp->rightdepth = work->leftdepth;/* set correct depth on left  */
        if (work1)                         /* moved a node               */
            work1->setParent(temp);        /* set its parent correctly   */
        work->setLeft(temp);               /* set left node              */
        work->leftdepth++;                 /* adjust its depth           */
    }
    work->setParent(temp->parent);         /* move node's parent around  */
    work2 = temp->parent;
    temp->setParent(work);                 /* so that top is correct     */
    if (work2 == OREF_NULL)                /* is this new root?          */
        setRoot(work);                     /* yes, adjust the root       */
    else if (work2->left == temp)          /* was it on left             */
        work2->setLeft(work);              /* make it left               */
    else
        work2->setRight(work);             /* else make it right         */
    *anchor = work;                        /* return correct position    */
}


RexxCompoundElement *RexxCompoundTable::first()
{
    if (root == OREF_NULL) {
        return OREF_NULL;
    }
    else {
        return findLeaf(root);
    }
}


RexxCompoundElement *RexxCompoundTable::findLeaf(
    RexxCompoundElement *node)             /* starting point we're drilling from */
{
    for (;;) {
        while (node->left != OREF_NULL) {  /* go as far left as we can */
            node = node->left;
        }
        if (node->right == OREF_NULL) {    /* if there is no right child, stop here */
            return node;
        }
        node = node->right;                /* go right one level and repeat */
    }
}


RexxCompoundElement *RexxCompoundTable::next(
    RexxCompoundElement *node)             /* starting point we're drilling from */
{
                                           /* get the parent node */
    RexxCompoundElement *parent = node->parent;
    if (parent != OREF_NULL) {
        if (parent->right == node) {       /* if coming back up from the right */
            return parent;                 /* this node's turn has come */
        }
        if (parent->right != OREF_NULL) {  /* if no right child, do this one immediately */
            return findLeaf(parent->right);/* drill down the other branch */
        }
        return parent;
    }
    return OREF_NULL;                      /* we've reached the top */
}


void RexxCompoundTable::setParent(
    RexxStem *parent)
/******************************************************************************/
/* Function:  Set the parent for a compound table.  N.B., this cannot be an   */
/* inline method because of circular header file dependencies between         */
/* RexxCompoundTable and RexxStem.                                            */
/******************************************************************************/
{
                                           /* NOTE: we're setting this as a field in the parent object */
    OrefSet(parent, parent->tails.parent, parent);
}


void RexxCompoundTable::setRoot(
    RexxCompoundElement *newRoot)
/******************************************************************************/
/* Function:  Set the root node for a compound table.  N.B., this cannot be an*/
/* inline method because of circular header file dependencies between         */
/* RexxCompoundTable and RexxStem.                                            */
/******************************************************************************/
{
                                           /* NOTE: we're setting this as a field in the parent object */
    OrefSet(parent, parent->tails.root, newRoot);
}

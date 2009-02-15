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
/****************************************************************************/
/* REXX Kernel                                      RexxCompoundTable.c     */
/*                                                                          */
/* Stem object table of compound variables                                  */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "RexxCompoundTable.hpp"
#include "RexxCompoundElement.hpp"
#include "RexxCompoundTail.hpp"

void RexxCompoundTable::init(
    RexxStem *parentStem)              /* the parent object we're embedded in */
{
    setParent(parentStem);             /* make the parent hook up */
    setRoot(OREF_NULL);                /* no root member */
}

/**
 * Copy all of the entries from another compound table into this
 * table.
 *
 * @param other  The source tail collection.
 */
void RexxCompoundTable::copyFrom(
    RexxCompoundTable &other)
{
    RexxCompoundElement *entry = other.first();/* grab the first element */
    while (entry != NULL)
    {            /* while we have more entry to process */
                 /* insert an entry in our table     */
        RexxCompoundElement *newEntry = findEntry(entry->getName(), true);
        /* copy over the value */
        newEntry->setValue(entry->variableValue);
        entry = other.next(entry);
    }
}

void RexxCompoundTable::clear()
{
    setRoot(OREF_NULL);                /* clear the anchor */
}


RexxCompoundElement *RexxCompoundTable::findEntry(RexxString *tail, bool create)
{
                                       /* process the tail and search */
    RexxCompoundTail resolved_tail(tail);
    return findEntry(&resolved_tail, create);
}


RexxCompoundElement *RexxCompoundTable::findEntry(
    RexxCompoundTail *tail,            /* our tail search target     */
    bool create)                       /* we're creating a variable  */
{
    int          rc = 0;               /* comparison result          */
    RexxCompoundElement *previous;     /* pointer for tree traversal */
    RexxCompoundElement *anchor;       /* pointer to current block   */

    anchor = root;                     /* get root block             */
    previous = anchor;                 /* save starting point        */
                                       /* loop through on left branch*/
    while (anchor)
    {
        /* do the name comparison */
        rc = tail->compare(anchor->getName());
        if (rc > 0)
        {                  /* left longer?               */
            previous = anchor;          /* save the previous          */
                                        /* take the right branch      */
            anchor =  anchor->right;
            continue;                   /* loop                       */
        }
        else if (rc < 0)
        {            /* left shorter?              */
            previous = anchor;        /* save the previous          */
                                      /* the the left branch        */
            anchor = anchor->left;
            continue;                 /* loop                       */
        }
        else
        {                        /* names match                */
            return anchor;            /* return the anchor          */
        }
    }
    if (!create)
    {                     /* not a SET operation,       */
        return OREF_NULL;                /* return var not found       */
    }
    /* create a new compound variable */
    anchor = new_compoundElement(tail->makeString());

    if (!previous)
    {                   /* if first insertion         */
        anchor->setParent(OREF_NULL);    /* no parent                  */
        setRoot(anchor);                 /* set the tree top           */
    }
    else
    {                             /* real insertion             */

        anchor->setParent(previous);     /* set our parent entry       */
        if (rc > 0)                      /* should this be left or     */
        {
            /* right on parent tree       */
            previous->setRight(anchor);    /* right                      */
        }
        else
        {
            previous->setLeft(anchor);     /* left                       */
        }
        balance(anchor);                 /* Balance the tree from here */
                                         /* up                         */
    }
    return anchor;                     /* return new block pointer   */
}


void RexxCompoundTable::balance(
    RexxCompoundElement *node)         /* starting point             */
{
    RexxCompoundElement *_parent;     /* block parent pointer       */
    unsigned short depth;             /* current depth              */
    unsigned short wd;                /* working depth              */

    if (node == root)                 /* this the root?             */
    {
        return;                       /* nothing to Balance         */
    }

    _parent = node->parent;           /* step up to block's parent  */
    depth = 1;                        /* initial depth is 1         */

    while (_parent != OREF_NULL)
    {    /* while still have a parent  */

        if (_parent->right == node)
        {      /* if on right branch         */
            _parent->rightdepth = depth;   /* set right depth            */
                                           /* deeper on left?            */
            if (depth > (wd = _parent->leftdepth + (unsigned short)1))
            {
                moveNode(&_parent, false);   /* adjust right branch        */
                depth = _parent->rightdepth; /* readjust depth             */
            }
            else
            {
                if (wd < depth)           /* left shorter               */
                {
                    return ;              /* done                       */
                }
            }
        }
        else
        {
            _parent->leftdepth = depth;    /* set left depth             */
                                           /* if right shorter           */
            if (depth > (wd = _parent->rightdepth + (unsigned short)1))
            {
                moveNode(&_parent, true);       /* adjust left branch         */
                depth = _parent->leftdepth;     /* readjust depth             */
            }
            else
            {
                if (wd < depth)           /* right shorter              */
                {
                    return ;              /* done                       */
                }
            }
        }
        depth++;                           /* increment the depth        */
        node = _parent;                    /* step up to current         */
        _parent = _parent->parent;         /* and lift up one more       */
    }
}

void RexxCompoundTable::moveNode(
    RexxCompoundElement   **anchor,        /* node to move               */
    bool         toright)                  /* move direction             */
{
    RexxCompoundElement   *temp;           /* anchor save position       */
    RexxCompoundElement   *work;           /* working block pointer      */
    RexxCompoundElement   *work1;          /* working block pointer      */
    RexxCompoundElement   *work2;          /* working block pointer      */

    temp = *anchor;                        /* save where we are          */

    if (toright)
    {                         /* move right?                */
        work = temp->left;                 /* get left branch pointer    */
        work1 = temp->left = work->right;  /* move right to left         */
        temp->leftdepth = work->rightdepth;/* adjust left depth value    */
        if (work1)                         /* was a right moved          */
        {
            work1->setParent(temp);        /* set its parent correctly   */
        }
        work->setRight(temp);              /* set new right              */
        work->rightdepth++;                /* adjust its depth           */
    }
    else
    {
        work = temp->right;                /* get right node             */
        work1 = temp->right = work->left;  /* move rights left node      */
        temp->rightdepth = work->leftdepth;/* set correct depth on left  */
        if (work1)                         /* moved a node               */
        {
            work1->setParent(temp);        /* set its parent correctly   */
        }
        work->setLeft(temp);               /* set left node              */
        work->leftdepth++;                 /* adjust its depth           */
    }
    work->setParent(temp->parent);         /* move node's parent around  */
    work2 = temp->parent;
    temp->setParent(work);                 /* so that top is correct     */
    if (work2 == OREF_NULL)                /* is this new root?          */
    {
        setRoot(work);                     /* yes, adjust the root       */
    }
    else if (work2->left == temp)          /* was it on left             */
    {
        work2->setLeft(work);              /* make it left               */
    }
    else
    {
        work2->setRight(work);             /* else make it right         */
    }
    *anchor = work;                        /* return correct position    */
}


RexxCompoundElement *RexxCompoundTable::first()
{
    if (root == OREF_NULL)
    {
        return OREF_NULL;
    }
    else
    {
        return findLeaf(root);
    }
}


RexxCompoundElement *RexxCompoundTable::findLeaf(
    RexxCompoundElement *node)             /* starting point we're drilling from */
{
    for (;;)
    {
        while (node->left != OREF_NULL)
        {  /* go as far left as we can */
            node = node->left;
        }
        if (node->right == OREF_NULL)
        {    /* if there is no right child, stop here */
            return node;
        }
        node = node->right;                /* go right one level and repeat */
    }
}


RexxCompoundElement *RexxCompoundTable::next(
    RexxCompoundElement *node)             /* starting point we're drilling from */
{
    /* get the parent node */
    RexxCompoundElement *_parent = node->parent;
    if (_parent != OREF_NULL)
    {
        if (_parent->right == node)
        {      /* if coming back up from the right */
            return _parent;                /* this node's turn has come */
        }
        if (_parent->right != OREF_NULL)
        {  /* if no right child, do this one immediately */
            return findLeaf(_parent->right);/* drill down the other branch */
        }
        return _parent;
    }
    return OREF_NULL;                      /* we've reached the top */
}


void RexxCompoundTable::setParent(
    RexxStem *parentStem)
/******************************************************************************/
/* Function:  Set the parent for a compound table.  N.B., this cannot be an   */
/* inline method because of circular header file dependencies between         */
/* RexxCompoundTable and RexxStem.                                            */
/******************************************************************************/
{
    // NOTE:  This seems a little weird, but we're doing the set using the parent
    // object...which will actually set the value in our own object instance.
    // This is done because the if we have checkSetOref turned on, the validity
    // checker won't recognize our address as being a valid object because it's
    // embedded within another Rexx object.
    OrefSet(parentStem, parentStem->tails.parent, parentStem);
}


void RexxCompoundTable::setRoot(
    RexxCompoundElement *newRoot)
/******************************************************************************/
/* Function:  Set the root node for a compound table.  N.B., this cannot be an*/
/* inline method because of circular header file dependencies between         */
/* RexxCompoundTable and RexxStem.                                            */
/******************************************************************************/
{
    // NOTE:  This seems a little weird, but we're doing the set using the parent
    // object...which will actually set the value in our own object instance.
    // This is done because the if we have checkSetOref turned on, the validity
    // checker won't recognize our address as being a valid object because it's
    // embedded within another Rexx object.
    OrefSet(parent, parent->tails.root, newRoot);
}


RexxCompoundElement *RexxCompoundTable::findEntry(RexxCompoundTail *tail)
/******************************************************************************/
/* Function:  Search for a compound entry.  This version is optimized for     */
/*            "find-but-don't create" usage.                                  */
/******************************************************************************/
{
    int          rc;                   /* comparison result          */
    RexxCompoundElement *anchor;       /* pointer to current block   */

    anchor = root;                     /* get root block             */
                                       /* loop through on left branch*/
    while (anchor != NULL)
    {
        /* do the name comparison */
        rc = tail->compare(anchor->getName());
        if (rc > 0)
        {                  /* left longer?               */
                           /* take the right branch      */
            anchor =  anchor->right;
            continue;                   /* loop                       */
        }
        else if (rc < 0)
        {            /* left shorter?              */
                     /* the the left branch        */
            anchor = anchor->left;
            continue;                 /* loop                       */
        }
        else
        {                        /* names match                */
            return anchor;            /* return the anchor          */
        }
    }
    return OREF_NULL;                  /* return var not found       */
}

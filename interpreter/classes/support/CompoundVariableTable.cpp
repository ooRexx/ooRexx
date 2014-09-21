/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* Stem object table of compound variables                                  */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "CompoundVariableTable.hpp"
#include "CompoundTableElement.hpp"
#include "CompoundVariableTail.hpp"
#include "StemClass.hpp"


/**
 * Initialize a compound table when a Stem object is
 * created.
 *
 * @param parentStem Our stem parent.
 */
void CompoundVariableTable::init(StemClass *parentStem)
{
    // record the parent object and clear out the root element.
    setParent(parentStem);
    setRoot(OREF_NULL);
}

/**
 * Copy all of the entries from another compound table into this
 * table.
 *
 * @param other  The source tail collection.
 */
void CompoundVariableTable::copyFrom(CompoundVariableTable &other)
{
    // do a traversal of the other table an insert an identical entry into our
    // table.
    CompoundTableElement *entry = other.first();
    while (entry != OREF_NULL)
    {
        // create an entry in this table with the same tail name
        CompoundTableElement *newEntry = findEntry(entry->getName(), true);
        // copy the value into the entry, and continue the iteration
        newEntry->setValue(entry->variableValue);
        entry = other.next(entry);
    }
}


/**
 * Clear the entire table.
 */
void CompoundVariableTable::clear()
{
    // just casting off the root is sufficient...GC does the rest.
    setRoot(OREF_NULL);
}


/**
 * Locate an entry in the compound table.
 *
 * @param tail   The string name of the tail.
 * @param create a flag indicating whether we should create a new
 *               element if this is not found.  True indicates we're
 *               setting a value in the table.
 *
 * @return The located (or created) element.  Returns NULL if
 *         not located and create is false.
 */
CompoundTableElement *CompoundVariableTable::findEntry(RexxString *tail, bool create)
{
    CompoundVariableTail resolved_tail(tail);
    return findEntry(resolved_tail, create);
}


/**
 * Locate a compound variable item and optionally create
 * a new table entry if not found.
 *
 * @param tail   The constructed tail for comparisons.
 * @param create indicates whether we create a new entry if this is not found.
 *
 * @return The entry for the given name, or NULL if not found and
 *         not asked to create the entry.
 */
CompoundTableElement *CompoundVariableTable::findEntry(CompoundVariableTail &tail, bool create)
{
    CompoundTableElement *anchor = root;        // get our anchor position and keep a previous
    CompoundTableElement *previous = anchor;    // pointer for backing up and insertions.

    int rc = 0;                                 // used for the traversal, but also used for insertion afterward

    // do a standard binary tree search.
    while (anchor)
    {
        // do the name comparison with the tail
        // our comparison is done first on length, second on value.
        // the rc will be 1, 0, or -1 based on which name is the "larger".
        rc = tail.compare(anchor->getName());
        // compare greater, take the right branch
        if (rc > 0)
        {
            previous = anchor;
            anchor =  anchor->right;
            continue;
        }
        // if the target is shorter, go to the left
        else if (rc < 0)
        {
            previous = anchor;
            anchor = anchor->left;
            continue;
        }
        // the names matched, found the one we're looking for.
        else
        {
            return anchor;
        }
    }

    // if not a create operation, we return a failure result
    if (!create)
    {
        return OREF_NULL;
    }


    // need to insert a new element into the tree
    anchor = new_compoundElement(tail.makeString());
    // no previous means the tree was empty, so this is our new
    // anchor value
    if (!previous)
    {
        // the anchor does not have a parent for backing up.
        anchor->setParent(OREF_NULL);
        setRoot(anchor);
    }
    else
    {
        // a real insertion.  hook us up to the parent
        anchor->setParent(previous);
        // we use the last decision point to determine whether
        // we're setting the left child or the right child.  We got
        // here because that child link was null.
        previous->setChild(rc, anchor);
        // balance the tree from the inserted node, if necessary
        balance(anchor);
    }
    return anchor;
}


/**
 * Balance the compound variable tree.
 *
 * @param node   The location of the just inserted node.  We balance
 *               from here.
 */
void CompoundVariableTable::balance(CompoundTableElement *node)
{
    // if this is the root node, nothing to do here.
    if (node == root)
    {
        return;
    }

    // we want the inserted node's parent.  Since this is not the
    // root node, this is non-null.
    CompoundTableElement *parentNode = node->parent;
    // the initial depth is 1
    unsigned short depth = 1;

    while (parentNode != OREF_NULL)
    {
        if (parentNode->isRightChild(node))
        {
            // update the parent nodes depth
            parentNode->rightDepth = depth;

            unsigned short workingDepth = parentNode->leftDepth + 1;
            // if the updated depth is greater than one more than the left
            // depth, we need to move stuff around
            if (depth > workingDepth)
            {
                // we need to adjust the right branch
                moveNode(parentNode, false);
                // adjust our depth and go around again.
                depth = parentNode->rightDepth;
            }
            else
            {
                // the left is shorter, we're still playing catchup.  No
                // balancing required.
                if (workingDepth < depth)
                {
                    return ;
                }
            }
        }
        // coming up from the left
        else
        {
            // same process as the other branch
            parentNode->leftDepth = depth;
            unsigned short workingDepth = parentNode->rightDepth + 1;
            if (depth > workingDepth)
            {
                // adjusting the left depth this time
                moveNode(parentNode, true);
                depth = parentNode->leftDepth;
            }
            else
            {
                // no adjustment needed
                if (workingDepth < depth)
                {
                    return ;
                }
            }
        }
        depth++;                           // increment the depth and go up a level to check again.
        node = parentNode;
        parentNode = parentNode->parent;
    }
}


/**
 * Move a node of the tree.
 *
 * @param anchor  The anchor position for the movement (which might
 *                get updated).
 * @param toright A flag indicating movement to the right or left.
 */
void CompoundVariableTable::moveNode(CompoundTableElement *&anchor, bool toright)
{
    // save where we are
    CompoundTableElement *temp = anchor;
    CompoundTableElement *work;

    // moving to the right?
    if (toright)
    {
        // move the right banch to the left side.
        work = temp->left;
        CompoundTableElement *work1 = temp->left = work->right;
        // adjust the depth values
        temp->leftDepth = work->rightDepth;
        // was the right moved.  Need to adjust it's parent
        if (work1 != OREF_NULL)
        {
            work1->setParent(temp);        /* set its parent correctly   */
        }
        // the old left node is now the parent of the starting point
        // and is one level deeper
        work->setRight(temp);
        work->rightDepth++;
    }
    else
    {
        // moving left branch to the rigth
        work = temp->right;
        CompoundTableElement *work1 = temp->right = work->left;
        temp->rightDepth = work->leftDepth;
        // if we moved a real node, set its parent
        if (work1 != OREF_NULL)
        {
            work1->setParent(temp);
        }
        // old right node is now the parent of our starting point
        work->setLeft(temp);
        work->leftDepth++;
    }
    // now we need to adjust the parents a bit
    // the new parent of the moved node is the anchor node's
    // parent (work is now the parent of the anchor node)
    work->setParent(temp->parent);
    CompoundTableElement *work2 = temp->parent;
    temp->setParent(work);
    // the original parent might have been NULL, which means we just
    // moved the root element
    if (work2 == OREF_NULL)
    {
        setRoot(work);
    }
    // if the anchor was originally on the left, set the new left node
    else if (work2->isLeftChild(temp))
    {
        work2->setLeft(work);
    }
    else
    {
        work2->setRight(work);
    }
    // this is the new anchor position
    anchor = work;
}


/**
 * Find the first node of the traversal.
 *
 * @return The "first" node in search order.
 */
CompoundTableElement *CompoundVariableTable::first()
{
    // if we have an empty tree, return null
    if (root == OREF_NULL)
    {
        return OREF_NULL;
    }
    // go find the "first" leaf node.
    else
    {
        return findLeaf(root);
    }
}


/**
 * Find a the "first" leaf node starting from a given
 * node.  The first node is the one that first in iteration
 * order, which will be the deepest node.  Since we balance,
 * this will either be the left-mode node or the right child
 * of the left-most node.
 *
 * @param node   The starting node.
 *
 * @return The final end node.
 */
CompoundTableElement *CompoundVariableTable::findLeaf(CompoundTableElement *node)
{
    for (;;)
    {
        // go as far left as we can.

        // It's just a jump to the left...
        while (node->left != OREF_NULL)
        {
            node = node->left;
        }
        // if we have no right node, we're done
        if (node->right == OREF_NULL)
        {
            return node;
        }

        // And then a step to the right.
        // With your hand on your hips.
        // You bring your knees in tight.
        // But it's the pelvic thrust.
        // They really drive you insane.
        // Let's do the Time Warp again.
        // Let's do the Time Warp again.
        node = node->right;
    }
}


/**
 * Step to the "next" node while iterating.
 *
 * @param node   The node we're moving from.
 *
 * @return The next node in iteration order.
 */
CompoundTableElement *CompoundVariableTable::next(CompoundTableElement *node)
{
    // get the parent node of our starting point.  We're generally
    // coming up from a leaf node while doing this, so we'll have already
    // visited the nodes below us.
    CompoundTableElement *parentNode = node->parent;
    // not at the root?
    if (parentNode != OREF_NULL)
    {
        // if coming up from the right side, it is time to visit the
        // parent node.
        if (parentNode->isRightChild(node))
        {
            return parentNode;
        }
        // we're coming up from the left side.  If the parent node has
        // a right child, then drill down from that point to find another
        // leaf node.
        if (parentNode->right != OREF_NULL)
        {
            return findLeaf(parentNode->right);
        }
        // coming up from the left and no right child.  Time to visit this node.
        return parentNode;
    }
    // we've reached the top, nothing more to do.
    return OREF_NULL;
}


/**
 * Set the parent for a compound table.  N.B., this cannot be an
 * inline method because of circular header file dependencies between
 * CompoundVariableTable and StemClass.
 *
 * @param parentStem The owning Stem object.
 */
void CompoundVariableTable::setParent(StemClass *parentStem)
{
    // We are embedded in the parent stem as the tails item. By setting this
    // value in the parentStem object, we're really setting it in ourselves.  Since
    // OrefSet needs to test the header field of the object getting set into, we're
    // forced to do it this way.
    setOtherField(parentStem, tails.parent, parentStem);
}


/**
 * Set the root node for a compound table.  N.B., this cannot be an
 * inline method because of circular header file dependencies between
 * CompoundVariableTable and StemClass.
 *
 * @param newRoot The new root node.
 */
void CompoundVariableTable::setRoot(CompoundTableElement *newRoot)
{
    // NOTE:  This seems a little weird, but we're doing the set using the parent
    // object...which will actually set the value in our own object instance.
    setOtherField(parent, tails.root, newRoot);
}


/**
 * Search for a compound entry.  This version is optimized for
 * "find-but-don't create" usage.
 *
 * @param tail   The tail name we're searching for.
 *
 * @return The located position, if any.
 */
CompoundTableElement *CompoundVariableTable::findEntry(CompoundVariableTail &tail)
{
    CompoundTableElement *anchor = root;

    while (anchor != NULL)
    {
        // do the name comparison
        int rc = tail.compare(anchor->getName());
        // right side larter
        if (rc > 0)
        {
            anchor = anchor->right;
        }
        // left side smaller
        else if (rc < 0)
        {
            anchor = anchor->left;
        }
        // equal, honey, I'm home....
        else
        {
            return anchor;
        }
    }
    // not found
    return OREF_NULL;
}


/**
 * Get the value from a compound table iterator.
 *
 * @return The current variable value.
 */
RexxObject *CompoundVariableTable::TableIterator::value()
{
    return current->getVariableValue();
}


/**
 * Get the name from a compound table iterator.
 *
 * @return The current variable name.
 */
RexxString *CompoundVariableTable::TableIterator::name()
{
    return current->getName();
}


/**
 * Get the full compound name from a compound table iterator.
 *
 * @return The current variable name.
 */
RexxString *CompoundVariableTable::TableIterator::name(RexxString *stemName)
{
    return current->createCompoundName(stemName);
}


/**
 * Get the full compound name from a compound table iterator.
 *
 * @return The current variable name.
 */
void CompoundVariableTable::TableIterator::replace(RexxObject *v)
{
    current->setValue(v);
}

#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
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
/*  treeTraversal.rex        Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  Build a binary tree structure and demonstrate different traversal         */
/*  methods                                                                   */
/******************************************************************************/

  -- the tree is built by hand by linking up a series of tree nodes.
  one = .Node~new(1);
  two = .Node~new(2);
  three = .Node~new(3);
  four = .Node~new(4);
  five = .Node~new(5);
  six = .Node~new(6);
  seven = .Node~new(7);
  eight = .Node~new(8);
  nine = .Node~new(9);

  one~left = two
  one~right = three
  two~left = four
  two~right = five
  three~left = six
  four~left = seven
  six~left = eight
  six~right = nine

  -- now traverse the tree in different orders, printing out the
  -- nodes in the order visited.
  out = .array~new
  .treetraverser~preorder(one, out);
  say "Preorder:" out~toString("l", ", ")
  out~empty
  .treetraverser~inorder(one, out);
  say "Inorder:" out~toString("l", ", ")
  out~empty
  .treetraverser~postorder(one, out);
  say "Postorder:" out~toString("l", ", ")
  out~empty
  .treetraverser~levelorder(one, out);
  say "Levelorder:" out~toString("l", ", ")


-- a node in the tree.  A node has data, plus
-- left and right children
::class node
::method init
  expose left right data
  use strict arg data
  left = .nil
  right = .nil

::attribute left
::attribute right
::attribute data

-- a tree traverser.  All methods here a class methods, as we do not have
-- to create instances to uses the traversers.
::class treeTraverser
-- preorder traversal.  The data is added to the set
-- when the node is first visited, then it visits the left
-- and right children
::method preorder class
  use arg node, out
  -- a .nil value means this branch doesn't exist.
  if node \== .nil then do
      -- append the node data, and recurse on each branch
      out~append(node~data)
      self~preorder(node~left, out)
      self~preorder(node~right, out)
  end

-- in order traversal.  The code visits the left branch first,
-- adds it's own data, then takes the right branch.
::method inorder class
  use arg node, out
  if node \== .nil then do
      -- go to the left, then add our data, then go right
      self~inorder(node~left, out)
      out~append(node~data)
      self~inorder(node~right, out)
  end

-- post order traversal.  The nodes data is not added until both
-- the left and right branches are taken
::method postorder class
  use arg node, out
  if node \== .nil then do
      self~postorder(node~left, out)
      self~postorder(node~right, out)
      out~append(node~data)
  end

-- level order traversal visits each node at the same level before visiting
-- the next level down.
::method levelorder class
  use arg node, out

  if node == .nil then return
  nodequeue = .queue~new

  -- method of operation.  We add the current node, to the queue
  -- then start pulling items off the queue, adding each of their
  -- children to the end of the queue.  So, for the root, we push
  -- that on the queue, immediately pull it off, visit the node, then
  -- push both of its children on the queue.  The next two nodes visited
  -- will be at level two, which will push four children on to the queue.
  -- The queuing ensures we can visit each node at a common level before
  -- stepping further down the tree.
  nodequeue~queue(node)
  loop while \nodequeue~isEmpty
      next = nodequeue~pull
      out~append(next~data)
      if next~left \= .nil then
          nodequeue~queue(next~left)
      if next~right \= .nil then
          nodequeue~queue(next~right)
  end

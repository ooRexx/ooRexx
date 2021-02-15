/*----------------------------------------------------------------------------*/
/*                                                                            */
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
/*  treeDirectory.rex       Open Object Rexx Samples                          */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  A directory implementation using a balanced binary tree.  Unlike the      */
/*  directory class, indexes are maintained in sorted order.                  */
/******************************************************************************/
::class "TreeDirectory" public inherit MapCollection

-- initialize a treetable item.  This just gives us an
-- empty node
::method init
  expose root items
  root = .nil
  items = 0

-- insert a key/value pair into the tree.
::method put
  expose root items
  use strict arg value, key

  -- first item inserted, this is easy
  if .nil == root then do
      root = .treenode~new(key, value)
      items = items + 1
  end
  else do
      -- need to locate the intersion point, which requires a key compare
      current = root
      do forever
          order = current~compare(key)

          -- this is an update to an existing item
          if order == 0 then do
              current~value = value
              return
          end
          -- the target belongs on the left
          else if order == 1 then do
              -- no left child, we've found the insertion point
              if current~leftchild == .nil then do
                  current~leftchild = .treenode~new(key, value)
                  current~leftchild~parent = current
                  items = items + 1
                  -- balance is key
                  self~balance(current~leftChild)
              end
              -- keep searching with the left node.
              else do
                  current = current~leftchild
              end
          end
          else do
              -- need to go to the right, same deal with either insert here
              -- or look to the right side
              if current~rightchild == .nil then do
                  current~rightchild = .treenode~new(key, value)
                  current~rightchild~parent = current
                  items = items + 1
                  -- always balance after an insert
                  self~balance(current~rightChild)
              end
              else do
                  current = current~rightchild
              end
          end
      end
  end

-- look up an item in the directory
::method at
  use strict arg key
  entry = self~findEntry(key)

  if entry \= .nil then do
      return entry~value
  end

  return .nil

-- synonym for AT
::method "[]"
  forward message 'AT'

-- synonum for PUT
::method "[]="
  forward message 'PUT'

-- check to see if an index exists
::method hasIndex
  use strict arg key

  return self~findEntry(key) \= .nil

-- check to see if an item exists


-- return the count of items in the directory
::method items
  expose items
  use strict arg
  return items

-- return an array of all items in the directory
::method allItems
  expose root items
  use strict arg

  itemsArray = .array~new(items)

  self~processNode(root, itemsArray)
  -- This gave us the list of nodes, in traversal order
  -- run the list and replace the node by its key
  do i = 1 to items
      itemsArray[i] = itemsArray[i]~value
  end

  return itemsArray

-- return an array of all items in the directory
::method allIndexes
  expose root items
  use strict arg

  indexes = .array~new(items)

  self~processNode(root, indexes)
  -- This gave us the list of nodes, in traversal order
  -- run the list and replace the node by its key
  do i = 1 to items
      indexes[i] = indexes[i]~key
  end

  return indexes


-- get a supplier for iterating over the directory items
::method supplier
  expose root items
  use strict arg

  -- get a pair of arrays for the supplier to use
  indices = .array~new(items)
  values  = .array~new(items)

  -- this will give us the entire list of nodes, we need to
  -- turn this into two arrays
  self~processNode(root, indexes)
  -- This gave us the list of nodes, in traversal order
  -- run the list and extract the keys and values
  do i = 1 to items
      values[i] = indices[i]~value
      indices[i] = indices[i]~key
  end

  return .supplier~new(indices, values)

-- remove an item from the directory
::method remove
  use strict arg key

  -- locate the node entry
  entry = self~findEntry(key)

  if entry == .nil then do
      return .nil
  end

  result = entry~value   -- the removal process may reuse the node object,
                         -- get the old value first

  -- now safe to delete the node
  self~deleteEntry(entry)
  return value

-- completely empty the directory
::method empty
  expose root items
  use strict arg
  root = .nill
  items = 0

-- test if the directory is empty
::method isEmpty
  expose items
  use strict arg
  return items == 0

-- return the index for an item that might be in the directory
::method index
  use strict arg item

  node = self~findItem(item)
  if node \= .nil then do
      return node~index
  end
  return .nil

-- test if an item is in the directory at all
::method hasItem
  use strict arg item

  node = self~findItem(item)
  return node \= .nil

-- prcess a single node visitation during an inorder traversal of the tree
::method processNode private
  use arg node, list

  if node == .nil then do
      return
  end
  -- vist left,
  self~processNode(node~leftChild, list)
  -- visit here
  list~put(node, list~items + 1)
  -- visit right
  self~processNode(node~rightChild, list)


-- find an individual item in the tree
::method findItem private
  expose root items

  use arg item

  current = root

  if root \= .nil then do
      current = self~findLeaf(root)
      do while current \= .nil
          if current~matchesItem(item) then
              return current
          current = self~next(current)
      end
  end

  return .nil

-- drill down to find a leaf node during a traversal operation
::method findLeaf private
  use arg node

  do forever
      do while node~leftChild \= .nil
          node = node~leftChild
      end

      if node~rightChild == .nil then do
          return node
      end

      node = node~rightChild
  end


-- get the next item in the tree.  This is a POSTORDER traversal process
::method next private
  use arg node

  parent = node~parent

  if parent \= .nil then do
      -- coming up from the right side, our parent node is the one we want
      if parent~rightChild == node then do
          return parent
      end
      -- coming up from the left, we may need to drill back down to a leaf again
      else if parent~rightChild \= .nil then do
          return self~findLeaf(parent~rightChild)
      end
      -- returning from the left, but no right child. This is parent time
      else do
          return parent
      end
  end

  return .nil     -- unwound to the root


-- find a give entry node in the tree
::method findEntry private
  expose root
  use arg key

  current = root

  do while current \= .nil
      order = current~compare(key)

      -- all done using the order
      if order == 0 then do
          return current
      end
      else if order > 0 then do
          current = current~leftchild
      end
      else do
          current = current~rightchild
      end
  end

  return .nil

-- delete an entry from the tree, doing a rebalance afterwards
::method deleteEntry private
  expose items
  use arg current


  items = items - 1

  case = current~hasLeftChild + current~hasRightChild
  -- processing differs depending on whether we have 0, 1, or 2 children
  select
      -- no children.  We're deleting a leaf node, which is easy.
      when case == 0 then do
          if root == current then do
              root = .nil
          end
          else do
              current~unlinkFromParent
              return;
          end
      end
      -- single child, we can just remove the node from the middle
      when case == 1 then do
          replacement = current~leftchild
          if replacement == .nil then do
              replacement = current~rightchild
          end
          if current == root then do
              root = replacement
          end
          else do
              current~parent~replaceChild(current, replacement);
          end
      end
      when case == 2 then do
          -- ok, we have two child nodes, which is a pain.  We need to
          -- find the next node in the sorted sequence.  This node will
          -- be either A) our right child (if the right child does not
          -- have a left child, or B) the left-most child of our right
          -- child.  In either case, this successor node will have only
          -- one child, which makes things easy.  So, we handle this by
          -- moving the key and value from the successor node, then
          -- deleting that successor node entry instead.

          successor = current~rightchild
          if successor~leftchild == .nil then do
              -- ok, all we need to do is remove the left child from the null
              current~key = successor~key
              current~value = successor~value
              current~rightchild = successor~rightchild
              current~rightchild~parent = current
          end
          else do
              -- drill down to the left-most leaf.
              do while successor~leftchild \= .nil
                  successor = successor~leftchild
              end
              -- copy the key and value items to the deleted node.
              current~key = successor~key
              current~value = successor~value
              -- now toss the swapped node and close up the chain
              successor~parent~leftchild = successor~rightchild
              successor~parent = current~parent
          end
      end
  end

-- balancing routine used for shifting a node to the right
::method moveNodeToRight private
  expose root
  use arg anchor

  work = anchor~leftChild;
  work1 = work~rightChild
  anchor~leftChild = work1
  anchor~leftDepth = work~rightDepth;

  if work1 \= .nil then do
      work1~parent = anchor
  end
  work~rightChild = anchor
  work~rightDepth = work~rightDepth + 1

  work~parent = anchor~parent
  work2 = anchor~parent
  anchor~parent = work;
  if work2 == .nil then do
      root = work
  end
  else if work~leftChild == anchor then do
      work2~leftChild = work
  end
  else do
      work2~rightChild = work
  end

  return work

-- balancing routine for shifting a node to the left
::method moveNodeToLeft private
  expose root
  use arg anchor

  work = anchor~rightChild;
  work1 = work~leftChild
  anchor~rightChild = work1
  anchor~rightDepth = work~leftDepth;
  if work1 \= .nil then do
      work1~parent = anchor
  end
  work~leftChild = anchor
  work~leftDepth = work~leftDepth + 1

  work~parent = anchor~parent
  work2 = anchor~parent
  anchor~parent = work
  if work2 == .nil then do
      root = work
  end
  else if work~leftChild == anchor then do
      work2~leftChild = work
  end
  else do
      work2~rightChild = work
  end

  return work

-- balance the tree after an insertion operation
::method balance private
  expose root

  use arg node

  if root == node then do
      return
  end

  parent = node~parent
  depth = 1

  do while parent \= .nil
      -- if on the right branch
      if parent~rightChild == node then do
          parent~rightDepth = depth

          workingDepth = parent~leftDepth + 1
          if depth > workingDepth then do
              parent = self~moveNodeToLeft(parent)
          end
          else if workingDepth < depth then do
              return
          end
      end
      else do
          parent~leftDepth = depth

          workingDepth = parent~rightDepth + 1
          if depth > workingDepth then do
              parent = self~moveNodeToRight(parent)
              depth = parent~leftDepth
          end
          else do
              if workingDepth < depth then do
                  return
              end
          end
      end
      depth = depth + 1
      node = parent
      parent = parent~parent
  end


-- private class for an individual node in a tree
::class treenode
::method init
  expose key value leftchild rightchild parent leftdepth rightdepth
  use arg key, value

  leftchild = .nil
  rightchild = .nil
  parent = .nil
  leftdepth = 0
  rightdepth = 0

-- various attributs of a node
::attribute parent
::attribute leftChild
::attribute rightChild
::attribute leftDepth
::attribute rightDepth
::attribute key
::attribute value

-- test for index equivalency
::method matches
  expose key
  use arg otherkey
  return key == otherkey

-- test for an item match
::method matchesItem
  expose value
  use arg otherValue
  return value == otherValue

-- perform an ordering comparison of the node key
::method compare
  expose key
  use arg otherkey

  -- just use the sort ordering method
  return key~compareto(otherKey)

-- unhook the node from its parent node
::method unlinkFromParent
  expose parent
  parent~unlinkChild(self)
  parent = .nil

-- remove a child node from this node
::method unlinkChild
  expose leftchild rightchild
  use arg child
  if child == leftchild then do
      leftchild = .nil
  end
  else do
      rightchild = .nil
  end

-- replace a given child node with a replacement node.
::method replaceChild
  expose leftchild rightchild
  use arg child, replacement
  if child == leftchild then do
      leftchild = replacement
  end
  else do
      rightchild = replacement
  end

-- test if a node has a left child
::method hasLeftChild
  expose leftChild
  return .nil \= leftChild

-- test if a node has a right child
::method hasRightChild
  expose rightChild
  return .nil \= rightChild

-- convert this node to a string
::method makestring
  return self~string

-- return a string value for the node, displaying
-- the key/value pair.
::method string
  expose key value
  return key~string":" value~string


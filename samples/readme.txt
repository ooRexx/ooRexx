/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
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

  README

  Sample Programs
  ---------------

  To help you exploring programming in Open Object Rexx, the following
  sample programs are provided (with source code):

- arithmeticEvaluation.rex    an expression evaluator that builds a parse tree
- arrayCallback.rex           perform a function over all elements in an array.
- ccreply.rex                 concurrent program using REPLY
- complex.cls                 complex number class
- concurrency.rex             demonstrate multi-threaded execution with
                              syncronization
- constrained.rex             Show how to use the isA method to check object
                              types
- delegation.rex              Show the concept of object method delegation
- dynamicMethod.rex           methods dynamically added to an object instance
- factor.rex                  factorial program
- getOoRexxDocs.rex           downloads the ooRexx documentation from SourceForge
- greply.rex                  concurrent program using WAIT and NOWAIT
- guess.rex                   a guessing game
- interface.rex               define an interface class in ooRexx
- ktguard.rex                 concurrent program using START and GUARD
- makestring.rex              program that uses makestring method
- month.rex                   displays days of the month of January
- native.api                  folder with examples using the C++ native APIs
- philfork.rex                a console version of the Philosophers' Forks
- pipe.cls                    a pipeline implementation
- properties.rex              an example of the Properties class
- qdate.rex                   date query program
- qtime.rex                   time query program
- readme.txt                  this file
- rexxcps.rex                 measuring REXX clauses/second
- scclient.rex                simple socket client that uses the socket class
- scserver.rex                simple socket server that uses the socket class
- semcls.cls                  semaphore class, see sections 12.4.4.1, 8.11 and
                              8.12 in rexxref for further information
- sfclient.rex                simple socket client that uses the socket
                              function package
- sfserver.rex                simple socket server that uses the socket
                              function package
- singleLinkedList.rex        a linked list using objects
- singleton.cls               implements a singleton class
- sortComposite.rex           sorting non-string objects
- stack.rex                   a program that uses a stack class
- synchronousConcurrency.rex  work queue to synchronize activity between
                              threads
- timezone.rex                performing timezone manipulations.
- treeDirectory.cls           directory implementation using a balanced
                              binary tree
- treeTraversal.rex           build a binary tree and demonstrate traversal
- usecomp.rex                 program that uses complex.cls
- usepipe.rex                 program that uses pipe.cls
- usesingleton.rex            program that uses singleton.cls
- usetree.rex                 show that a treeDirectory is polymorphic with
                              a built-in directory

  These programs are executable without any change on all supported platforms.

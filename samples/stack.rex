#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/*  stack.rex           Open Object Rexx Samples                              */
/*                                                                            */
/*  A stack class                                                             */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates how to implement a stack class using ::class    */
/*  and ::method directives.  Also included is a short example of the use of  */
/*  a stack.                                                                  */
/******************************************************************************/

mystack = .stack~new                        /* Create a new instance of stack */
mystack~push('John Smith')
mystack~push('Bill Brown')
say 'Size of stack:' mystack~items          /* Displays size of stack         */
say 'Pop stack:    ' mystack~pop            /* Displays item popped off stack */
say 'Pop stack:    ' mystack~pop            /* Displays next item popped off  */
say 'Pop stack:    ' mystack~pop            /* Stack now empty - displays     */
                                            /*   "The NIL Object"             */

/* Define the stack class (a subclass of Object Class) */
::class stack

/* Define the init method on the stack class */
::method init
expose contents                             /* Object var with stack contents */
self~init:super                             /* Run init method of superclass  */
contents = .list~new                        /* Use a list to keep stack items */

/* Define the push method on the stack class */
::method push
expose contents
use arg item
contents~insert(item, .nil)

/* Define the pop method on the stack class */
::method pop
expose contents
if contents~items > 0 then
  return contents~remove(contents~first)
else
  return .nil

/* Define the items method on the stack class to return number of items */
::method items
expose contents
return contents~items

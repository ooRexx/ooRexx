/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2014 Rexx Language Association. All rights reserved.    */
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

  	ReadMe

  1.  ooDialog - Dialog Control Example Programs
  -----------------------------------------------

  This directory contains example programs that demonstrate how to use the
  list-view control in ooDialog.  The examples range from simple to medium
  complex.

    - columnClickListView.rex

    An example program that shows how to determine which row and which
    column in a list view control the user has clicked on.

    - customDrawListView.rex

    Shows how to use a list-view with emphasis on custom draw.  Custom
    draw with a list-view allows you set the text color, background color,
    and font for individuals list-view items.  And when the list-view is
    in report view, this can be done for individual subitems of each row.

    - columnIcons.rex

    This example shows how to use icons for all columns in a list-view,
    how to use LvFullRow objects to populate a list-view, and how to use
    the internal sorting feature of the ooDialog framework.  In report
    view, the columns can be drag and droppred to arrange the order.
    Clicking on a column sorts the column.

  The subitem.editing subdirectory contains several examples of embedding
  dialog controls within the list-view to allow in-place editing of the
  subitems in report view.

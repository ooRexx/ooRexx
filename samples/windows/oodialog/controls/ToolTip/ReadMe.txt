/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
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

  1.  ooDialog - ToolTip Example Programs
  ---------------------------------------

  This directory contains example programs that demonstrate how to use the
  ToolTip control in ooDialog.  It is intended that the examples range from
  simple programs to medium complex programs, and cover most of the
  important methods of the ToolTip class.


    - toolTip.rex

    Shows how to use a ToolTip control, including responding to the SHOW
    event to position the ToolTip where desired.  Also shows how to:
    enumerate the tools contained in the ToolTip, add a tool simply to the
    ToolTip, add a tool as a rectangular area, create a multiline ToolTip,
    and create a balloon ToolTip

    - comboBoxToolTip.rex

    Shows how to add tool tips to a combo box.  When the list items in a
    combo box are longer than the combo box is wide, it is difficult for
    the user to know what they say.  The tool tip control is an elegant
    solution to this problem.  But, adding tool tips and getting them to
    work is a little more difficult than one might think.  This example
    shows how to do it.


    - customPositionToolTip.rex

    Demonstrates how to do custom positioning of the info tip in a
    tree-view control.  If the program is just displayed, it appears to be
    exactly the same as the manageControlTool.rex example.  The source
    code for both examples needs to be examined to see the differences.

    - manageControlTool.rex

    Demonstrates how create a completely custom ToolTip for a dialog
    control.  If the program is just displayed, it appears to be exactly
    the same as the manageControlTool.rex example.  The source code for
    both examples needs to be examined to see the differences.


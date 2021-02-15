/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007-2014 Rexx Language Association. All rights reserved.    */
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

  	ReadMe

  1.  ooDialog Example Programs
  -------------------------------

  This directory contains example ooDialog programs.  They are intended to
  be relatively short and simple programs that demonstrate how to use some
  feature of ooDialog.

    - addManyRows.rex

    This example's main purpose is to show how to use the ProgressDialog
    class.  It allows the user to select how many list-view items are to be
    added to the list-view before it starts.  It also does some interesting
    timings and displays the times in the dialog

    - browsePrinters.rex

    Shows how to use the BrowseForFolder class and how to customize the
    browse dialog so that the user can select a printer.

    - fileNameDialog_demo.rex

    The FileNameDialog public routine allows a programmer to present the
    user with the standard Windows Open or Save file dialog.  The
    fileNameDialog_demo program demonstrates how to use this routine.

    - imageButton.rex

    This example program demonstrates some of the new features introduced
    in ooRexx 4.0.0, including the .Image, .Imagelist classes, and the
    setImageList() method of the button class.

    - publicRoutines_demo.rex

    ooDialog contains a number of standard dialog and public routines.  The
    standard dialogs and public routines are designed to be easy to use.
    They allow a programmer to added simple graphical elements to a program
    without any detailed knowledge of the ooDialog framework.  The
    publicRoutines_demo program demonstrates how to use these public
    routines.

    - simpleFolderBrowse.rex

    Demonstrates how to use the SimpleFolderBrowse class to allow the user
    to pick a folder.

    - stopWatch.rex

    An intersting implmentation of a stop watch in Rexx.

    - useTools.rex

    This example program shows how to use a dialog that is an "owned"
    window.  Owned windows have several constraints, one of which is that
    they always remain above their owner window.  This makes them useful to
    create "tool palette" types of programs.  The example program does just
    that, demonstrates a main dialog with a tool palette.


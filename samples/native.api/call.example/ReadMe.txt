/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2010-2010 Rexx Language Association. All rights reserved.    */
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
    ======

    This subdirectory provides two examples: runRexxProgram and
    stackOverflow.  Both programs will compile and run on any platform
    supported by ooRexx. Also included are 2 make files.

    Makefile.windows
    ----------------
    This is a nMake compatible make file for building the examples on
    Windows.  Provided your build environment is set up correctly for a
    MicroSoft's Visual C++ compile, you should be able to compile the
    examples without problem.  Read the comments in Makefile.windows if
    your have problems and / or to see how to build a debug version (the
    default) or a non-debug version of the samples.

    Makefile.linux
    --------------
    This make file should build the examples on any Unix-like system that
    has gcc installed.


    runRexxProgram
    ==============
    This example creates an instance of the interpreter and uses it to call
    a Rexx program.  The Rexx program name is supplied on the command line.
    For instance:

    runRexxProgram HelloWorld.rex

    Read the comments in the program code for additional details.  A few
    sample Rexx programs are provided, including backward.fnc


    stackOverflow
    =============
    This example creates an instance of the interpreter and then
    dynamically creates and calls a Rexx routine.  Two routines are
    provided.  Execute stackOverflow without any arguments to use the first
    routine and with one (any) argument to use the second routine.  I.e.:

    stackOverflow

    or

    stackOverflow 1

    More details are provided in comments within the C++ code.

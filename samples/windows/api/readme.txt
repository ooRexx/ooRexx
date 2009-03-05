/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
Open Object Rexx API samples

This is the read me for the three API samples, callrxwn, callrxnt, and
rexxexit.  Each sample is installed in its own directory. The wpipe samples
have their own read me file in the wpipe directory.

The following notes on compiling the samples also apply to the wpipe samples.

Make files, which are compatible with nmake, are included in each sample
directory so that you can rebuild the samples.  In order to use the make files
to build the samples, you must set up the environment variables INCLUDE and
LIB.

Compilation:
------------
The environment variable INCLUDE must contain the path to rexx.h as well as
the regular compiler include paths.  The rexx.h file is located in the top
level api\ directory of the ooRexx installation.

Linking:
--------
The environment variable LIB must contain the path to rexx.lib and rexxapi.lib
as well as the regular compiler library paths.  These library files are
included in the same location as rexx.h, that is, in the top level api\
directory of the ooRexx installation.

Samples:
--------

callrxwn   - Windows mode application

           - Provides a sample call to the REXX interpreter, passing
             in an environment name, a file name, and a single argument
             string. A dialog box is created for the output.

           - Note that you must define a REXX standard input and
             output exit handler for a Windows mode application.
             This is not necessary for Console mode applications, however.


callrxnt   - Console mode application

           - Has similar functionality to callrxwn

           - Console mode applications calling Object REXX do not
             require input and output exit handlers.

rexxexit   - Console mode application

           - Provides a sample call to the REXX interpreter, passing
             in arguments from the command line. A REXX input and
             output exit handler is also demonstrated.

           - An example input file, testRexxExit is provided.  Using that file
             an invocation of rexxexit.exe would look like:

             rexxexit testRexxExit "18 9"

             The two numbers need to be in quotes because rexxexit no more
             than two arguments, the input file name and a single argument for
             the input file.

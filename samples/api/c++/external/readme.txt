This directory includes c++ source code files with Makefiles for the different
platforms and Rexx code which demonstrate how to create routines/functions and
methods in native code and using it directly from ooRexx code.

        external_routines.cpp
        =====================

        This example creates external functions/routines in a DLL which get
        used from

        - useExternalRoutines.rex: uses the routine directive to individually
          load the native routines (would allow to use a different name in the
          ooRexx program)

        - useExternalRoutines.rex: uses the "external" subkeyword of the routine
          directive to load the native routines

        - useExternalRoutines.rex: uses the "library" subkeyword of the requires
          directive to load all external routines at once


        external_methods.cpp
        ====================

        This example creates external methods in a DLL which get used from

        - useExternalMethods.rex: uses the "external" subkeyword of the method
          directive to load the native methods


        external_methods_routines.cpp
        =============================

        This example creates external routines and methods in a DLL which get
        used from

        - useExternalMethodsRoutines.rex: uses the "external" subkeyword of the
          routine and method directives

Maybe some useful hints:

        On Linux you may want to run the ooRexx scripts with:

                LD_LIBRARY_PATH=`pwd` rexx useExternalRoutines.rex
                LD_LIBRARY_PATH=`pwd` rexx useExternalRoutines2.rex
                LD_LIBRARY_PATH=`pwd` rexx useExternalMethods.rex
                LD_LIBRARY_PATH=`pwd` rexx useExternalMethodsRoutines.rex

        Unix: to recreate the shared libraries you would enter:

                make -f Makefile.unix clean
                make -f Makefile.unix

        Windows: to recreate the DLLs you would enter:

                nmake /f Makefile.windows clean
                nmake /f Makefile.windows


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2021 Rexx Language Association. All rights reserved.         */
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
/* THIS SOFTWARE IS PROVIDED BY THE COPYright HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYright   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/


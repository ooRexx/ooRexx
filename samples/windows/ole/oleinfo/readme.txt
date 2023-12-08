/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2002-2022 Rony G. Flatscher. All rights reserved.            */
/* Copyright (c) 2023      Rexx Language Association. All rights reserved.    */
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

"oleinfo"
=========

ooRexx set of Window programs for analyzing and documenting registered Windows
OLE/COM components.

Please note: Windows will expose only those ProgIDs that match the bitness of
             the program (ooRexx). Therefore running these little utilities with
             32-bit and 64-bit versions of the ooRexx interpreter may yield
             different ProgIDs. (ooRexx 5 allows 32-bit and 64-bit versions to run
             in parallel on the same Windows computer.)


- listProgIds.rex   ... program lists all registered Windows OLE/COM components

        - optional argument 1: supplying the digit 1 adds CLSID to the output
        - optional argument "needle": lists only ProgIDs containing the needle

        - examples:

                rexx listProgIds.rex
                rexx listProgIds.rex policy
                rexx listProgIds.rex 1
                rexx listProgIds.rex 1 policy

- createOleInfo.rex ... program to create the documentation on the fly, if a
                        typelib got installed with the OLE/COM component

        - argument 1:          ProgID or CLSID
        - optional argument 2: 0 (default) long, 1 short (reference) documentation
        - optional argument 3: 0 no display, 1 (default) display documentation

        - examples:

                rexx createOleInfo.rex InternetExplorer.Application
                rexx createOleInfo.rex InternetExplorer.Application 1
                rexx createOleInfo.rex InternetExplorer.Application 1 0
                rexx createOleInfo.rex {0002DF01-0000-0000-C000-000000000046}
                rexx createOleInfo.rex {0002DF01-0000-0000-C000-000000000046} 1 0
                rexx createOleInfo.rex {0002DF01-0000-0000-C000-000000000046} 0 1



- getOleConstants.rex ... program creates a Rexx program that puts all constant values
                          into .local~ole.const; requiring or calling that created
                          Rexx program makes all constants of that OLE/COM component
                          available to the programmer. To retrieve a specific constant
                          send the constant name to ".ole.const" which will return its
                          value.

        - argument 1:          ProgID or CLSID
        - optional argument 2: name of the file the generated generated Rexx code
                               should be saved to; if omitted output goes to stdout (the screen)
                               ... if file gets generated incorporate it either with the
                                   "::requires file.rex" or "call file.rex" statement; therafter
                                   fetch any constant sending its name .ole.const

        - examples:

                rexx getOleConstants.rex Excel.Application excel_constants.rex
                ... inspect "excel_constants.rex", use in Rexx program:
                        ...
                        say .ole.const~xlColumnHeader   -- access Excel constant "xlColumnHeader"
                        ...
                        ::requires excel_constants.rex  -- incorporate all Excel constants

                rexx getOleConstants.rex Word.Application word_constants.rex
                ... inspect "word_constants.rex", use in Rexx program: call word_constants.rex
                        ...
                        call word_constants.rex         -- incorporate all Word constants
                        ...
                        say .ole.const~wdReadingView    -- access Word constant "wdReadingView"
                        ...

--- supporting programs

        - oleinfo.css   ... the cascading stylesheet for the generated documentation

        - oleinfo.cls   ... support package for these utilities representing each OLE/COM component
                            with the most important information about it

        - oleinfo.properties ... a property (text) file defining the CSS file name and whether its
                            content should be incorporated into the head elelment of the generated
                            HTML documentation

        - oleinfo2html.frm ... program to create the long and short/reference HTML documentation

        - reg_classids4ole.cls ... support package to analyze the registry for OLE/COM components

--- NOTE: you can invoke createOleInfo as a subroutine, function from an ooRexx program with the
          following arguments:

        - argument 1:          an OleObject or a string with the ProgID or CLSID; this way
                               it becomes possible to have the documentation created of an OLE/COM
                               object returned by a windows method or property

        - optional argument 2: title to use for the HTML rendering and filename

        - optional argument 3: 0 (default) long, 1 short (reference) documentation

        - optional argument 4: 0 no display, 1 (default) display documentation

        - example:

                ... cut ...
                oleobject = winapp~getSomething(...) -- returns an OLE object
                    -- document the returned OLE object as HTML with a supplied title
                call createOleInfo oleobject, "From winapp~getSomething(..)"
                ... cut ...

        Cf. the accompanying test.rex program

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* Object REXX Support                                          rxregexp.cpp  */
/* Regular Expression Utility functions                                       */
/*                                                                            */
/******************************************************************************/
#include "dblqueue.hpp"
#include "automaton.hpp"
#include "regexp.hpp"

#include "oorexxapi.h"
#include <string.h>

RexxMethod2(int, RegExp_Init, OPTIONAL_CSTRING, expression, OPTIONAL_CSTRING, matchtype)
{
    int         iResult = 0;
    automaton *pAutomaton = new automaton();

    // optional matchtype given?
    if (matchtype != NULL)
    {
        if (strcmp(matchtype, "MINIMAL") == 0)
        {
            pAutomaton->setMinimal(true);
        }
    }

    // optional expression given?
    if (expression != NULL)
    {
        iResult = pAutomaton->parse(expression);
        if (iResult != 0)
        {
            context->RaiseException0(Rexx_Error_Invalid_template);
        }
    }

    // this will be passed back into us on calls
    context->SetObjectVariable("CSELF", context->NewPointer(pAutomaton));

    return 0;
}

RexxMethod1(int, RegExp_Uninit, CSELF, self)
{
    automaton  *pAutomaton = (automaton *)self;
    if (pAutomaton != NULL)
    {
        delete pAutomaton;
    }
    // ensure we don't do this twice
    context->DropObjectVariable("CSELF");
    return 0;
}

RexxMethod3(int,                            // Return type
            RegExp_Parse,                   // Object_method name
            CSELF, self,                    // Pointer to automaton control block
            CSTRING, expression,            // regular expression to parse
            OPTIONAL_CSTRING, matchtype)    // optional match type (MAXIMAL (def.) or MINIMAL)
{
    automaton  *pAutomaton = (automaton *)self;
    // moved some ptrs to re-use variables
    // optional matchtype given?
    if (matchtype != NULL)
    {
        if ( strcmp(matchtype, "MINIMAL") == 0)
        {
            pAutomaton->setMinimal(true); // set minimal matching
        }
        else if (strcmp(matchtype, "CURRENT") != 0)
        {
            pAutomaton->setMinimal(false); // set maximal matching
        }
    }
    int i = pAutomaton->parse( expression);
    context->SetObjectVariable("!POS", context->WholeNumber(pAutomaton->getCurrentPos()));
    return i;
}

RexxMethod2(int,                          // Return type
            RegExp_Match,                 // Object_method name
            CSELF, self,                  // Pointer to self
            RexxStringObject, string)     // string to match
{
    automaton  *pAutomaton = (automaton *)self;
    int i = pAutomaton->match( context->StringData(string), (int)context->StringLength(string));
    context->SetObjectVariable("!POS", context->WholeNumber(pAutomaton->getCurrentPos()));
    return i;
}

RexxMethod2(int,                          // Return type
            RegExp_Pos,                   // Object_method name
            CSELF, self,                  // Pointer to self
            RexxStringObject, string)     // string to match
{
    automaton  *pAutomaton = (automaton *)self;
    bool        fOldState;
    const char *pszString;
    size_t      strlength;
    int         i;

    pszString = context->StringData(string);
    strlength = context->StringLength(string);
    int matchPosition = 0;

    /* only check when input > 0 */
    if (strlength > 0)
    {
        fOldState = pAutomaton->getMinimal();

        // we start out matching minimal
        pAutomaton->setMinimal(true);
        do
        {
            i = pAutomaton->match(pszString, (int)strlength);
            strlength--;
            pszString++;
        } while (i == 0 && strlength != 0);
        // can we match at all?
        if (i != 0)
        {
            i = (int) (pszString - context->StringData(string));
            // want a maximal match within string?
            if (fOldState == false)
            {
                pAutomaton->setMinimal(false);
                pszString--; // correct starting pos
                strlength++; // correct starting len
                while (strlength != 0)
                {
                    if (pAutomaton->match(pszString, (int)strlength) != 0)
                    {
                        break;
                    }
                    strlength--;
                }
            }
            matchPosition = i + pAutomaton->getCurrentPos() - 1;
        }

        context->SetObjectVariable("!POS", context->WholeNumber(matchPosition));
        pAutomaton->setMinimal(fOldState);  // restore to state at POS invocation time
        return i;
    }

    return 0;
}

// now build the actual entry list
RexxMethodEntry rxregexp_methods[] =
{
    REXX_METHOD(RegExp_Init,    RegExp_Init),
    REXX_METHOD(RegExp_Uninit,  RegExp_Uninit),
    REXX_METHOD(RegExp_Parse,   RegExp_Parse),
    REXX_METHOD(RegExp_Pos,     RegExp_Pos),
    REXX_METHOD(RegExp_Match,   RegExp_Match),
    REXX_LAST_METHOD()
};


RexxPackageEntry rxregexp_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "RXREGEXP",                          // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    NULL,                                // no functions in this package
    rxregexp_methods                     // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(rxregexp);

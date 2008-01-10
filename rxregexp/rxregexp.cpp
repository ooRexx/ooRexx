/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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

#include "rexx.h"
#include "RexxNativeAPI.h"                      // REXX native interface
#include "RexxErrorCodes.h"
#include <string.h>

RexxMethod3(REXXOBJECT,                // Return type
            RegExp_Init,               // Object_method name
            OSELF, self,               // Pointer to self
            REXXSTRING, expression,    // optional regular expression
            REXXSTRING, matchtype)     // optional match type (MAXIMAL (def.) or MINIMAL)
{
  int         iResult = 0;
  automaton  *pAutomaton;

  pAutomaton = new automaton();

  // optional matchtype given?
  if (matchtype)
  {
    if ( strcmp(string_data(matchtype), "MINIMAL") == 0) {
      pAutomaton->setMinimal(true);
    }
  }

  // optional expression given?
  if (expression)
  {
    iResult = pAutomaton->parse( string_data(expression) );
  }

  REXX_SETVAR("!AUTOMATON", ooRexxPointer(pAutomaton));

  if (iResult)
    rexx_exception(Error_Invalid_template);

  return ooRexxNil;
}

RexxMethod1(REXXOBJECT,                // Return type
            RegExp_Uninit,             // Object_method name
            OSELF, self)               // Pointer to self
{
  automaton  *pAutomaton = NULL;

  pAutomaton = (automaton *)pointer_value(REXX_GETVAR("!AUTOMATON") );
  if (pAutomaton) delete pAutomaton;

  return ooRexxNil;
}

RexxMethod3(REXXOBJECT,                // Return type
            RegExp_Parse,              // Object_method name
            OSELF, self,               // Pointer to self
            REXXSTRING, expression,    // regular expression to parse
            REXXSTRING, matchtype)     // optional match type (MAXIMAL (def.) or MINIMAL)
{
  automaton  *pAutomaton = NULL;
  REXXOBJECT  result;

  if (!expression)
    rexx_exception1(Error_Incorrect_method_noarg, ooRexxString("1"));

  REXXOBJECT value = REXX_GETVAR("!AUTOMATON");
  if (value != NULLOBJECT)
  {
      pAutomaton = (automaton *)pointer_value(value);
  }

  if (pAutomaton) {
    const char *pszString = string_data(expression);
    // moved some ptrs to re-use variables
    // optional matchtype given?
    if (matchtype) {
      if ( strcmp(string_data(matchtype), "MINIMAL") == 0) {
        pAutomaton->setMinimal(true); // set minimal matching
      } else if (strcmp(string_data(matchtype), "CURRENT") != 0) {
        pAutomaton->setMinimal(false); // set maximal matching
      }
    }
    int i = pAutomaton->parse( pszString );
    REXX_SETVAR("!POS", ooRexxInteger(pAutomaton->getCurrentPos()));
    result = ooRexxInteger(i);
  } else {
    result = ooRexxInteger(-1);
  }

  return result;
}

RexxMethod2(REXXOBJECT,                // Return type
            RegExp_Match,              // Object_method name
            OSELF, self,               // Pointer to self
            REXXSTRING, string)        // string to match
{
  automaton  *pAutomaton = NULL;
  REXXOBJECT  result;

  if (!string)
    rexx_exception1(Error_Incorrect_method_noarg, ooRexxString("1"));

  REXXOBJECT value = REXX_GETVAR("!AUTOMATON");
  if (value != NULLOBJECT)
  {
      pAutomaton = (automaton *)pointer_value(value);
  }
  if (pAutomaton) {
    int i = pAutomaton->match( string_data(string), (int)string_length(string) );
    REXX_SETVAR("!POS", ooRexxInteger(pAutomaton->getCurrentPos()));
    result = ooRexxInteger(i);
  } else {
    result = ooRexxInteger(0);
  }

  return result;
}

RexxMethod2(REXXOBJECT,                // Return type
            RegExp_Pos,                // Object_method name
            OSELF, self,               // Pointer to self
            REXXSTRING, string)        // string to match
{
  automaton  *pAutomaton = NULL;
  bool        fOldState;
  const char *pszString;
  size_t      strlength;
  REXXOBJECT  result;
  REXXOBJECT  pArgString = NULL;
  int         i;

  REXXOBJECT value = REXX_GETVAR("!AUTOMATON");
  if (value != NULLOBJECT)
  {
      pAutomaton = (automaton *)pointer_value(value);
  }

  pszString = string_data(string);
  strlength = string_length(string);
  int matchPosition = 0;

  if (pAutomaton && strlength > 0) {  /* only check when input > 0 */
    fOldState = pAutomaton->getMinimal();

    // we start out matching minimal
    pAutomaton->setMinimal(true);
    do {
      i = pAutomaton->match(pszString, (int)strlength);
      strlength--;
      pszString++;
    } while (i == 0 && strlength != 0);
    // can we match at all?
    if (i != 0) {
      i = (int) (pszString - string_data(pArgString));
      // want a maximal match within string?
      if (fOldState == false) {
        pAutomaton->setMinimal(false);
        pszString--; // correct starting pos
        strlength++; // correct starting len
        while (strlength != 0) {
          if (pAutomaton->match(pszString, (int)strlength) != 0) {
            break;
          }
          strlength--;
        }
      }
      matchPosition = i + pAutomaton->getCurrentPos() - 1;
    }

    REXX_SETVAR("!POS", ooRexxInteger(matchPosition));
    result =  ooRexxInteger(i);
    pAutomaton->setMinimal(fOldState);  // restore to state at POS invocation time
  } else {
    result = ooRexxInteger(0);
  }

  return result;
}

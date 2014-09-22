/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Unit Support                                                          */
/*                                                                            */
/* Unix system specific VALUE() built-in function routine                     */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"
#include "MethodArguments.hpp"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define  SELECTOR  "ENVIRONMENT"       /* environment selector              */


/**
 * Process a value function call for external repositories.
 *
 * @param Name     The variable name.
 * @param NewValue The new value to assign to that variable.
 * @param Selector The variable pool selector.
 * @param result   The returned existing value.
 *
 * @return true if this worked, false otherwise.
 */
bool SystemInterpreter::valueFunction(RexxString *name, RexxObject *newValue, RexxString *selector, RexxObject *&result)
{
    // we only recognize the environemnt selector
    if (!selector->strCaselessCompare(SELECTOR))
    {
        return false;                    // we can't handle this one
    }

    // if not there, we return a null string
    result = GlobalNames::NULLSTRING;

    // see if we have an existing variable first
    const char *oldValue = getenv(name->getStringData());
    // either return the existing variable value or a null string.
    if (oldValue != NULL)
    {
        result = new_string(oldValue);
    }

    // set the variable if we have a new value
    if (newValue != OREF_NULL)
    {
        // .nil is special, it removes the variable
        if (newValue == (RexxString *)TheNilObject)
        {
            unsetenv(name->getStringData());
        }
        // we need a string value for the set.
        else
        {
            setenv(name->getStringData(), stringArgument(newValue, ARG_TWO)->getStringData(), true) ;
        }
    }
    return true;
}



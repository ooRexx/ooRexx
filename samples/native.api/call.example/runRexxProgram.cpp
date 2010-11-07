/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2010 Rexx Language Association. All rights reserved.    */
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

/**
 * A simple example that creates an instance of the interpreter and uses that
 * instance to execute a Rexx program.  The program name must be passed in the
 * command line.  For example:
 *
 * runRexxProgram HelloWorld.rex
 * runRexxProgram backward.fnc
 * runRexxProgram tooRecursiveTrapped.rex
 * runRexxProgram tooRecursiveUnhandled.rex
 *
 * backward.fnc demonstrates how to pass arguments into the called program.
 */

#include "oorexxapi.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#define _CDECL __cdecl
#else
#define _CDECL
#endif

/* Prototypes for several simple helper functions that demonstrate usage of some
 * of the native C++ APIs. The functions themselves are at the bottom of the
 * file.
 */
bool checkForCondition(RexxThreadContext *c, bool clear);
void printInterpreterVersion(RexxInstance *);

int _CDECL main(int argc, char **argv)
{
    char *programName = NULL;
    if ( argc == 2 )
    {
        programName = argv[1];
    }

    if ( programName == NULL || *programName == '\0' )
    {
        printf("You must pass in the name of a Rexx program on the command line.\n");
        printf("  For example: %s crash.rex.\n", argv[0]);
        return 9;
    }

    // These are the arguments to RexxCreateInterpreter().  An array of any
    // number of Rexx options can be passed in, but for this example we do not
    // need any options.  So, we use NULL.
    RexxInstance *interpreter;
    RexxThreadContext *threadContext;
    RexxOption *options = NULL;

    if ( RexxCreateInterpreter(&interpreter, &threadContext, options) == 0 )
    {
        printf("Failed to create interpreter, aborting.\n");
        exit(1);
    }
    printInterpreterVersion(interpreter);

    // If we want to pass arguments to the program we need to put them into an
    // array of Rexx objects.  We can pass null if there are no arguments.
    RexxArrayObject args = NULL;

    if ( stricmp("backward.fnc", programName) == 0 )
    {
        RexxStringObject str = threadContext->String("These words will be swapped");
        args = threadContext->ArrayOfOne(str);
    }

    // Execute the program and get the result returned to us.
    printf("Using interpreter to execute %s\n\n", programName);
    RexxObjectPtr result = threadContext->CallProgram(programName, args);

    // During the program execution, a condition can be raised if there is an
    // unexpected error. If an exception occurred and is pending,
    // CheckCondtion() will return true. In this case we print out some
    // information on the condition, otherwise we print out the return, if any,
    // from the program.
    if (threadContext->CheckCondition())
    {
        checkForCondition(threadContext, true);
    }
    else if (result != NULLOBJECT)
    {
        // Note that we use ObjectToStringValue().  That is guarenteed to return
        // the ASCII-Z string representation of the object. If we passed in, say
        // an .array object to the CString() function, we would get a crash.
        printf("\nProgram result=%s\n\n", threadContext->ObjectToStringValue(result));
    }

    // Now wait for the interpreter to terminate and we are done.
    interpreter->Terminate();

    return 0;
}

/**
 * Below are several helper functions that demonstrate how to use some of the
 * different C++ native APIs.
 */

/**
 * Given an interpreter instance, prints out the interpreter version and
 * language version.  The documentation in the ooRexx programming guide explains
 * the byte encoding of the version numbers.
 */
void printInterpreterVersion(RexxInstance *interpreter)
{
    wholenumber_t ver = interpreter->InterpreterVersion();
    wholenumber_t lang = interpreter->LanguageLevel();
    printf("Created interpreter instance version=%d.%d.%d language level=%d.%02d\n\n",
           (ver & 0xff0000) >> 16, (ver & 0x00ff00) >> 8, ver & 0x0000ff, (lang & 0xff00) >> 8, lang & 0x00ff);
}


/**
 * Given a condition object, extracts and returns as a whole number the subcode
 * of the condition.
 */
inline wholenumber_t conditionSubCode(RexxCondition *condition)
{
    return (condition->code - (condition->rc * 1000));
}


/**
 * Outputs the typical condition message.  For example:
 *
 *      4 *-* say dt~number
 * Error 97 running C:\work\qTest.rex line 4:  Object method not found
 * Error 97.1:  Object "a DateTime" does not understand message "NUMBER"
 *
 * @param c          The thread context we are operating in.
 * @param condObj    The condition information object.  The object returned from
 *                   the C++ API GetConditionInfo()
 * @param condition  The RexxCondition struct.  The filled in struct from the
 *                   C++ API DecodeConditionInfo().
 *
 * @assumes  There is a condition and that condObj and condition are valid.
 */
void standardConditionMsg(RexxThreadContext *c, RexxDirectoryObject condObj, RexxCondition *condition)
{
    RexxObjectPtr list = c->SendMessage0(condObj, "TRACEBACK");
    if ( list != NULLOBJECT )
    {
        RexxArrayObject a = (RexxArrayObject)c->SendMessage0(list, "ALLITEMS");
        if ( a != NULLOBJECT )
        {
            size_t count = c->ArrayItems(a);
            for ( size_t i = 1; i <= count; i++ )
            {
                RexxObjectPtr o = c->ArrayAt(a, i);
                if ( o != NULLOBJECT )
                {
                    printf("%s\n", c->ObjectToStringValue(o));
                }
            }
        }
    }
    printf("Error %d running %s line %d: %s\n", condition->rc, c->CString(condition->program),
           condition->position, c->CString(condition->errortext));

    printf("Error %d.%03d:  %s\n", condition->rc, conditionSubCode(condition), c->CString(condition->message));
}


/**
 * Given a thread context, checks for a raised condition, and prints out the
 * standard condition message if there is a condition.
 *
 * @param c      Thread context we are operating in.
 * @param clear  Whether to clear the condition or not.
 *
 * @return True if there was a condition, otherwise false.
 */
bool checkForCondition(RexxThreadContext *c, bool clear)
{
    if ( c->CheckCondition() )
    {
        RexxCondition condition;
        RexxDirectoryObject condObj = c->GetConditionInfo();

        if ( condObj != NULLOBJECT )
        {
            c->DecodeConditionInfo(condObj, &condition);
            standardConditionMsg(c, condObj, &condition);

            if ( clear )
            {
                c->ClearCondition();
            }
            return true;
        }
    }
    return false;
}

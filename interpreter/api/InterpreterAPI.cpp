/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Startup                                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "MethodClass.hpp"
#include "Interpreter.hpp"
#include "TranslateDispatcher.hpp"
#include "RexxStartDispatcher.hpp"
#include "InterpreterInstance.hpp"
#include "NativeActivation.hpp"
#include "RexxInternalApis.h"
#include "SystemInterpreter.hpp"

#include <stdio.h>


int REXXENTRY RexxTerminate()
/******************************************************************************/
/* Function:  Terminate the REXX interpreter...will only terminate if the     */
/*            call nesting level has reached zero.                            */
/******************************************************************************/
{
    // terminate and clean up the interpreter runtime.  This only works
    // if there are no active instances
    return Interpreter::terminateInterpreter() ? 0 : 1;
}

int REXXENTRY RexxInitialize ()
/******************************************************************************/
/* Function:  Perform main kernel initializations                             */
/******************************************************************************/
{
    // start this up for normal execution
    Interpreter::startInterpreter(Interpreter::RUN_MODE, NULL);
    // this always returns true
    return true;
}


/**
 * Create the Rexx saved image during build processing.
 *
 * @return Nothing
 */
void REXXENTRY RexxCreateInterpreterImage(const char *target)
{
    // start this up and save the image.  This never returns to here
    Interpreter::startInterpreter(Interpreter::SAVE_IMAGE_MODE, target);
}



/******************************************************************************/
/* Name:       RexxMain                                                       */
/*                                                                            */
/* Arguments:  argcount - Number of args in arglist                           */
/*             arglist - Array of args (array of RXSTRINGs)                   */
/*             programname - REXX program to run                              */
/*             instore - Instore array (array of 2 RXSTRINGs)                 */
/*             envname - Initial cmd environment                              */
/*             calltype - How the program is called                           */
/*             exits - Array of system exit names (array of RXSTRINGs)        */
/*                                                                            */
/* Returned:   result - Result returned from program                          */
/*             rc - Return code from program                                  */
/*                                                                            */
/* Notes:  Primary path into Object REXX.  Makes sure Object REXX is up       */
/*   and runs the requested program.                                          */
/*                                                                            */
/******************************************************************************/
int REXXENTRY RexxStart(
    size_t argcount,                     /* Number of args in arglist         */
    PCONSTRXSTRING arglist,              /* Array of args                     */
    const char *programname,             /* REXX program to run               */
    PRXSTRING instore,                   /* Instore array                     */
    const char *envname,                 /* Initial cmd environment           */
    int   calltype,                      /* How the program is called         */
    PRXSYSEXIT exits,                    /* Array of system exit names        */
    short *retcode,                     /* Integer form of result            */
    PRXSTRING result)                    /* Result returned from program      */
{
    if (calltype == RXCOMMAND && argcount == 1 && arglist[0].strptr != NULL && arglist[0].strlength > 0 &&
        StringUtil::caselessCompare(arglist[0].strptr, "//T", arglist[0].strlength) == 0)
    {
        TranslateDispatcher arguments;
        arguments.programName = programname;
        arguments.instore = instore;
        // this just translates and gives the error, potentially returning
        // the instore image
        arguments.outputName = NULL;
        arguments.encode = false;
        // go run this program
        arguments.invoke(exits, envname);

        return (int)arguments.rc;      /* return the error code (negated)   */
    }


    // this is the dispatcher that handles the actual
    // interpreter call.  This gets all of the RexxStart arguments, then
    // gets dispatched on the other side of the interpreter boundary
    RexxStartDispatcher arguments;
    /* copy all of the arguments into    */
    /* the info control block, which is  */
    /* passed across the kernel boundary */
    /* into the real RexxStart method    */
    /* this is a real execution          */
    arguments.argcount = argcount;
    arguments.arglist = arglist;
    arguments.programName = programname;
    arguments.instore = instore;
    arguments.calltype = calltype;
    arguments.retcode = 0;
    arguments.result = result;

    // go run this program
    arguments.invoke(exits, envname);
    *retcode = arguments.retcode;

    // terminate and clean up the interpreter runtime.  This only works
    // if there are no active instances
    Interpreter::terminateInterpreter();

    return (int)arguments.rc;          /* return the error code (negated)   */
}


/**
 * Translate a program and store the translated results in an
 * external file.
 *
 * @param inFile  The input source file.
 * @param outFile The output source.
 * @param exits   The exits to use during the translation process.
 *
 * @return The error return code (if any).
 */
RexxReturnCode REXXENTRY RexxCompileProgram(const char *inFile, const char *outFile, PRXSYSEXIT exits, bool encode)
{
    TranslateDispatcher arguments;
    // this gets processed from disk, always.
    arguments.programName = inFile;
    arguments.instore = NULL;
    // this saves to a file, possible base65 encoded
    arguments.outputName = outFile;
    arguments.encode = encode;

    // go run this program
    arguments.invoke(exits, NULL);

    // terminate and clean up the interpreter runtime.  This only works
    // if there are no active instances
    Interpreter::terminateInterpreter();

    return (RexxReturnCode)arguments.rc;       /* return the error code (negated)   */
}


/**
 * Translate a program and store the translated results in an
 * external file.
 *
 * @param inFile  The input source file.
 * @param outFile The output source.
 * @param exits   The exits to use during the translation process.
 *
 * @return The error return code (if any).
 */
RexxReturnCode REXXENTRY RexxTranslateProgram(const char *inFile, const char *outFile, PRXSYSEXIT exits)
{
    return RexxCompileProgram(inFile, outFile, exits, false);
}


/**
 * Translate a program and store the translated results in an
 * external file.
 *
 * @param inFile The input name.
 * @param source The source RXSTRING
 * @param image  The returned image RXSTRING
 *
 * @return The error return code (if any).
 */
RexxReturnCode REXXENTRY RexxTranslateInstoreProgram(const char *inFile, CONSTRXSTRING *source, RXSTRING *image)
{
    TranslateInstoreDispatcher arguments;
    // this gets processed from disk, always.
    arguments.programName = inFile;
    arguments.source = source;
    arguments.image = image;
    // go run this program
    arguments.invoke(NULL, NULL);
    return (RexxReturnCode)arguments.rc;       /* return the error code (negated)   */
}


/**
 * Retrieve the interpreter version information.
 *
 * @return
 */
char *REXXENTRY RexxGetVersionInformation()
{
    char ver[100];
    snprintf(ver, sizeof(ver), " %d.%d.%d r%d", ORX_VER, ORX_REL, ORX_MOD, ORX_BLD);
    char header[] = "Open Object Rexx Version";
  #ifdef _DEBUG
    char build[] = " - Internal Test Version\nBuild date: ";
  #else
    char build[] = "\nBuild date: ";
  #endif
  #ifdef __REXX64__
    char mode[] = "\nAddressing mode: 64";
  #else
    char mode[] = "\nAddressing mode: 32";
  #endif
    char copy1[] = "\nCopyright (c) 1995, 2004 IBM Corporation. All rights reserved.";
    char copy2[] = "\nCopyright (c) " OOREXX_COPY_YEAR " Rexx Language Association. All rights reserved.";
    char copy3[] = "\nThis program and the accompanying materials are made available under the terms";
    char copy4[] = "\nof the Common Public License v1.0 which accompanies this distribution or at";
    char copy5[] = "\nhttps://www.oorexx.org/license.html";
    size_t length = strlen(header) + strlen(ver) + strlen(build) + strlen(__DATE__) +
        strlen(mode) + strlen(copy1) + strlen(copy2) + strlen(copy3) + strlen(copy4) + strlen(copy5) + 1;
    char *ptr = (char *)SystemInterpreter::allocateResultMemory(length);
    if (ptr != NULL)
    {
        snprintf(ptr, length, "%s%s%s%s%s%s%s%s%s%s", header, ver, build, __DATE__, mode, copy1, copy2, copy3, copy4, copy5);
    }
    return ptr;
}


/**
 * Raise a halt condition for a target thread.
 *
 * @param threadid The target threadid.
 *
 * @return RXARI_OK if this worked, RXARI_NOT_FOUND if the thread isn't
 *         active.
 */
RexxReturnCode REXXENTRY RexxHaltThread(thread_id_t threadid)
{
    if (Interpreter::isActive())
    {                        /* Are we up?                     */
       if (!ActivityManager::haltActivity(threadid, OREF_NULL))
       {
           return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
       }
       return (RXARI_OK);
    }
    return RXARI_NOT_FOUND;     /* REXX not running, error...     */
}


/**
 * Compatibility function for doing a RexxHaltThread().
 *
 * @param procid   The process id (ignored).
 * @param threadid The target threadid
 *
 * @return the success/failure return code.
 */
RexxReturnCode REXXENTRY RexxSetHalt(process_id_t procid, thread_id_t threadid)
{
    return RexxHaltThread(threadid);
}


/**
 * Turn on tracing for a given interpreter thread.
 *
 * @param threadid The target thread identifier.
 *
 * @return the success/failure return code.
 */
RexxReturnCode REXXENTRY RexxSetThreadTrace(thread_id_t threadid)
{
    if (Interpreter::isActive())
    {
       if (!ActivityManager::setActivityTrace(threadid, true))
       {
           return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
       }
       return (RXARI_OK);
    }
    return RXARI_NOT_FOUND;     /* REXX not running, error...     */
}


/**
 * Reset the external trace for a target thread.
 *
 * @param threadid The target thread id.
 *
 * @return The success/failure indicator.
 */
RexxReturnCode REXXENTRY RexxResetThreadTrace(thread_id_t threadid)
{
    if (Interpreter::isActive())
    {
       if (!ActivityManager::setActivityTrace(threadid, false))
       {
           return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
       }
       return (RXARI_OK);
    }
    return RXARI_NOT_FOUND;     /* REXX not running, error...     */
}


/**
 * Compatibility stub for the old signature of RexxSetTrace.
 *
 * @param procid   The process id (ignored).
 * @param threadid The target thread identifier.
 *
 * @return the success/failure return code.
 */
RexxReturnCode REXXENTRY RexxSetTrace(process_id_t procid, thread_id_t threadid)
{
    return RexxSetThreadTrace(threadid);
}


/**
 * The compatibility stub for the reset trace API.
 *
 * @param procid   The target process id (ignored).
 * @param threadid The thread id of the target thread.
 *
 * @return The success/failure indicator.
 */
RexxReturnCode REXXENTRY RexxResetTrace(process_id_t procid, thread_id_t threadid)
{
    return RexxResetThreadTrace(threadid);
}


/**
 * Create an interpreter instance at the API level.
 *
 * @param instance The returned instance pointer.
 * @param context  The initial thread context for this instance.
 * @param options  An array of interpreter instance options.
 *
 * @return 1 if the instance was created, 0 for any creation errors.
 */
RexxReturnCode RexxEntry RexxCreateInterpreter(RexxInstance **instance, RexxThreadContext **context, RexxOption *options)
{
    return Interpreter::createInstance(*instance, *context, options) ? 0 : 1;
}

/**
 * Main entry point for processing variable pool requests
 *
 * @param pshvblock The shaved variable block chain for the request.
 *
 * @return The composite request return code.
 */
RexxReturnCode RexxEntry RexxVariablePool(PSHVBLOCK pshvblock)
{
    NativeContextBlock context;
    // the variable pool interface handles its own try/catches.
    return context.self->variablePoolInterface(pshvblock);
}

/**
 * Process a stemsort call for the rexxutil SysStemSort function.
 *
 * @param stemname The name of the stem.
 * @param order    The sort order.
 * @param type     The type of sort (case sensitivity).
 * @param start    The starting element number.
 * @param end      The end element number.
 * @param firstcol The first sort column.
 * @param lastcol  The last sort column.
 *
 * @return The sort return code result.
 */
RexxReturnCode RexxEntry RexxStemSort(RexxStemObject stem, const char *tailExtension, int order, int type,
    wholenumber_t start, wholenumber_t end, wholenumber_t firstcol, wholenumber_t lastcol)
{
    NativeContextBlock context;
    // the variable pool interface handles its own try/catches.
    return context.self->stemSort((StemClass *)stem, tailExtension, order, type, start, end, firstcol, lastcol);
}

/**
 * Wait for Rexx termination.  This is a nop in 4.0 since
 * the APIs do the proper thing with respect to threading
 * termination.  This is maintained solely for binary
 * compatibility.
 *
 * @return
 */
void RexxEntry RexxWaitForTermination()
{
}


/**
 * Test if the interpreter environment has terminated.  This is
 * a nop in 4.0 since the APIs do the proper thing with respect
 * to threading termination.  This is maintained solely for
 * binary compatibility.
 *
 * @return always returns true
 */
RexxReturnCode RexxEntry RexxDidRexxTerminate()
{
    return true;
}

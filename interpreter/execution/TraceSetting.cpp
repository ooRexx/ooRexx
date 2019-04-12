/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Backing constants and methods for the trace settings                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "TraceSetting.hpp"
#include <ctype.h>

const TraceSetting::TraceFlags TraceSetting::defaultTraceFlags(TraceSetting::traceNormal, TraceSetting::traceFailures);

// now the flag sets for different settings
const TraceSetting::TraceFlags TraceSetting::traceAllFlags(TraceSetting::traceAll, TraceSetting::traceLabels, TraceSetting::traceCommands);
const TraceSetting::TraceFlags TraceSetting::traceResultsFlags(TraceSetting::traceAll, TraceSetting::traceLabels, TraceSetting::traceResults, TraceSetting::traceCommands);
const TraceSetting::TraceFlags TraceSetting::traceIntermediatesFlags(TraceSetting::traceAll, TraceSetting::traceLabels, TraceSetting::traceResults, TraceSetting::traceCommands, TraceSetting::traceIntermediates);


/**
 * Format an encoded trace setting back into human readable form.
 *
 * @param setting The source setting.
 *
 * @return The string representation of the trace setting.
 */
RexxString *TraceSetting::toString()
{
    char         setting[3];
    setting[0] = '\0';
    int index = 0;

    // do we have a debug setting?
    // add a '?' prefix to the formatted setting
    if (isDebug())
    {
        setting[index++] = '?';
    }
    // we need to decode the flags to generate the actual setting.

    if (flags[traceOff])
    {
        setting[index++] = 'O';
    }
    // The following must be done in this order because N and E both
    // trace failures.
    else if (flags[traceNormal])
    {
        setting[index++] = 'N';
    }
    else if (flags[traceErrors])
    {
        setting[index++] = 'E';
    }
    else if (flags[traceFailures])
    {
        setting[index++] = 'F';
    }
    // these also must be done in this order...
    // each option also includes the setting below
    else if (flags[traceIntermediates])
    {
        setting[index++] = 'I';
    }
    else if (flags[traceResults])
    {
        setting[index++] = 'R';
    }
    // ALL includes Label and Command, so it comes before those
    else if (flags[traceAll])
    {
        setting[index++] = 'A';
    }
    else if (flags[traceLabels])
    {
        setting[index++] = 'L';
    }
    else if (flags[traceCommands])
    {
        setting[index++] = 'C';
    }

    // null terminate, then return as a string
    setting[index] = '\0';
    return new_string(setting);
}


/**
 * Parse a trace setting value into a decoded setting
 * and the RexxActivation debug flag set to allow
 * new trace settings to be processed more quickly.
 *
 * @param value      The string source of the trace setting.
 * @param newSetting The returned setting in binary form.
 * @param debugFlags The debug flag representation of the trace setting.
 */
bool TraceSetting::parseTraceSetting(RexxString *value, char &badOption)
{
    bool turnDebugOn = false;

    size_t length = value->getLength();

    // null string?  This just turns tracing off.
    if (length == 0)
    {
        setTraceOff();
        return true;
    }

    // turn off entirely so we know if we have just a debug toggle request.
    clear();

    // scan the characters.  We only recognize the first characters of
    // words, but this can also have a prefix.
    for (size_t pos = 0; pos < length; pos++)
    {
        switch (toupper(value->getChar(pos)))
        {
            // Toggle the debug character...we can have any number of these, we
            // only perform an operation if we have an odd number of them.
            case '?':
                // flip the debug setting.  We figure out what to do once
                // we've processed the whole string.
                turnDebugOn = !turnDebugOn;
                continue;

            // TRACE ALL
            case 'A':
                setTraceAll();
                break;

            // TRACE COMMANDS
            case 'C':
                setTraceCommands();
                break;

            // TRACE LABELS
            case 'L':
                setTraceLabels();
                break;

            // TRACE ERRORS
            case 'E':
                setTraceErrors();
                break;

            // TRACE FAILURES
            case 'F':
                setTraceFailures();
                break;

            // TRACE NORMAL
            case 'N':
                // default setting is the same as failure
                setTraceNormal();
                break;

            // TRACE OFF
            case 'O':
                setTraceOff();
                break;

            // TRACE RESULTS
            case 'R':
                setTraceResults();
                break;

            // TRACE INTERMEDIATES
            case 'I':
                setTraceIntermediates();
                break;

            // unknown trace setting
            default:
                // each context handles it's own error reporting, so give back the
                // information needed for the message.
                badOption = value->getChar(pos);
                return false;
                break;
        }
        // we break out of the loop if we get here.  Situations that
        // need additional parsing do a continute.
        break;
    }

    // we need to somehow set the debug setting.  If there is not other
    // trace setting, this is a debug toggle.  Otherwise, turn on the debug flag in
    // the setting.
    if (turnDebugOn)
    {
        if (isNoSetting())
        {
            setDebugToggle();
        }
        // trace OFF is special...it unconditionally turns off debug, so
        // don't set the debug flag on if we have OFF.
        else if (!isTraceOff())
        {
            // this turns on special optimization flags also.
            setDebug();
        }
    }
    return true;
}

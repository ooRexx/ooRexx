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
/* REXX Kernel                                                                */
/*                                                                            */
/* Backing constants and methods for the trace settings                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "TraceSetting.hpp"

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



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
/*                                                                            */
/* Class to encapuslate knowledge of various trace settings in an activation. */
/* These are frequently constructed by the language parser, but processed by  */
/* the activation.  This removes ownership of this information to a central   */
/* place.                                                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_TraceSetting
#define Included_TraceSetting

#include "FlagSet.hpp"

class RexxString;

/**
 * A class for processing different trace settings as set by the
 * TRACE command.
 */
class TraceSetting
{
 public:

    TraceSetting() { }

    /**
     * Flag settings within the trace flags.
     */
    typedef enum
    {
        traceOff,                         // trace nothing
        traceNormal,                      // the default normal trace setting.
        traceDebug,                       // interactive trace mode flag
        traceAll,                         // trace all instructions
        traceResults,                     // trace all results
        traceIntermediates,               // trace all instructions
        traceCommands,                    // trace all commands
        traceLabels,                      // trace all labels
        traceErrors,                      // trace all command errors
        traceFailures,                    // trace all command failures
        pauseInstructions,                // pause for instructions (optimization of traceDebug and traceAll)
        pauseLabels,                      // pause for labels
        pauseCommands,                    // pause for commands,
        debugToggle,                      // requesting a debug toggle
    } TraceFlag;

    /**
     * The type for trace flags.  We create a few constant versions
     * of these for simple setting and testing.
     */
    typedef FlagSet<TraceFlag, 32> TraceFlags;

    static const TraceFlags defaultTraceFlags;

    // now the flag sets for different settings
    static const TraceFlags traceAllFlags;
    static const TraceFlags traceResultsFlags;
    static const TraceFlags traceIntermediatesFlags;

    inline void clear() { flags.reset(); }
    inline bool tracingResults() const { return flags[traceResults]; }
    inline bool tracingIntermediates() const { return flags[traceIntermediates]; }
    inline bool isDebug() const { return flags[traceDebug]; }
    inline bool tracingAll() const { return flags[traceAll]; }
    inline bool tracingErrors() const { return flags[traceErrors]; }
    inline bool tracingFailures() const { return flags[traceFailures]; }
    inline bool tracingLabels() const { return flags[traceLabels]; }
    inline bool tracingCommands() const { return flags[traceCommands]; }
    inline bool pausingInstructions() const { return flags[pauseInstructions]; }
    inline bool pausingLabels() const { return flags[pauseLabels]; }
    inline bool pausingCommands() const { return flags[pauseCommands]; }
    inline bool isDebugToggle() const { return flags[debugToggle]; }
    inline bool isTraceOff() const { return flags[traceOff]; }

    // reset all debug settings
    inline void resetDebug()
    {
        // turn off the debug and all pause flags
        flags.reset(traceDebug, pauseInstructions, pauseLabels, pauseCommands);
    }
    // turn on debug settings
    inline void setDebug()
    {
        // turn on the debug flag and any pause indicators.
        flags[traceDebug] = true;
        flags[pauseInstructions] = tracingAll();
        flags[pauseLabels] = tracingLabels();
        flags[pauseCommands] = tracingCommands();
    }
    // toggle the debug setting
    inline void toggleDebug()
    {
        if (isDebug())
        {
            resetDebug();
        }
        else
        {
            setDebug();
        }
    }

    // turn tracing off
    inline void setTraceOff()
    {
        // reset all of the flags, then set the explicit Off flag
        flags.reset();
        flags[traceOff] = true;
    }

    // merge the flag settings, keeping existing debug settings
    inline void merge(const TraceSetting &s)
    {
        if (s.isTraceOff())
        {
            setTraceOff();
        }
        else
        {
            // pull in the settings from the other and keep
            // our debug setting
            bool debug = isDebug();
            flags = s.flags;
            // if we were debug, flip on again to make sure
            // the pausing flags are set correctly with the new settings.
            if (debug)
            {
                setDebug();
            }
        }
    }

    // unconditionally set the flag settings
    inline void set(const TraceSetting &s)
    {
        // copy the flags
        flags = s.flags;
        // if the debug flag is set, do the set processing again to
        // make sure the pause flags are in sync with the new bits.
        if (isDebug())
        {
            setDebug();
        }
    }

    /**
     * Test if we have no explict value set.
     *
     * @return True if all flags are off.
     */
    inline bool isNoSetting() const
    {
        return flags.none();
    }

    // set the default trace setting
    inline void setDefault()
    {
        flags.reset();
        // trace normal really traces failures, but we set the
        // special flag so we know the setting too.
        flags = defaultTraceFlags;
    }

    // we're tracing all
    inline void setTraceAll()
    {
        flags.reset();
        flags = traceAllFlags;
    }

    // we're tracing just commands
    inline void setTraceCommands()
    {
        flags.reset();
        // Just the single instruction type here.
        flags[traceCommands] = true;
    }

    // we're tracing just labels
    inline void setTraceLabels()
    {
        flags.reset();
        // just interested in labels.
        flags[traceLabels] = true;
    }

    // we're tracing just command errors
    inline void setTraceErrors()
    {
        flags.reset();
        // just errors includes failures.
        flags[traceErrors] = true;
        flags[traceFailures] = true;
    }

    // we're tracing just command failures
    inline void setTraceFailures()
    {
        flags.reset();
        // failures, but not errors
        flags[traceFailures] = true;
    }

    // we're tracing just command failures
    inline void setTraceNormal()
    {
        flags.reset();
        // failures, but not errors, but remember that this is
        // the Normal version for display
        flags = defaultTraceFlags;
    }

    // we're tracing results
    inline void setTraceResults()
    {
        flags.reset();
        // results is trace all with results traced also.
        flags = traceResultsFlags;
    }

    // we're tracing everything
    inline void setTraceIntermediates()
    {
        flags.reset();
        // intermediates is trace results with intermediates traced also.
        flags = traceIntermediatesFlags;
    }

    // flip the debug setting
    inline void flipDebug()
    {
        flags.flip(traceDebug);
    }

    // set the debug toggle setting
    inline void setDebugToggle()
    {
        flags.reset();
        flags[debugToggle] = true;
    }

    // set trace to the external tracing form.
    inline void setExternalTrace()
    {
        // The external trace value is Trace ?R
        setTraceResults();
        setDebug();
    }

    RexxString *toString();
    bool parseTraceSetting(RexxString *value, char &badOption);


protected:

    TraceFlags flags;                    // the flag settings associated with the selected trace.
};

#endif


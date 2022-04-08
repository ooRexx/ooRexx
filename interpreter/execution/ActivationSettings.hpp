/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Class to encapsulate the various settings that are shared between          */
/* activation instances.                                                      */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ActivationSetting
#define Included_ActivationSetting

#include "RexxLocalVariables.hpp"        // local variable cache definitions
#include "RexxDateTime.hpp"
#include "PackageSetting.hpp"

class StringTable;
class DirectoryClass;
class RexxObject;
class RexxString;
class MethodClass;
class RexxCode;
class VariableDictionary;
class SecurityManager;
class RexxClass;

// activationContext values
// these are done as bit settings to
// allow multiple potential values
// to be checked with a single test
typedef enum
{
    DEBUGPAUSE   = 0x00000001,
    METHODCALL   = 0x00000002,
    INTERNALCALL = 0x00000004,
    INTERPRET    = 0x00000008,
    PROGRAMCALL  = 0x00000010,
    EXTERNALCALL = 0x00000020,
                                   // check for top level execution
    TOP_LEVEL_CALL = (PROGRAMCALL | METHODCALL | EXTERNALCALL),
                                   // non-method top level execution
    PROGRAM_LEVEL_CALL = (PROGRAMCALL | EXTERNALCALL),
                                   // non-method top level execution
    PROGRAM_OR_METHOD = (PROGRAMCALL | METHODCALL),
                                   // call is within an activation
    INTERNAL_LEVEL_CALL = (INTERNALCALL | INTERPRET),
} ActivationContext;


// command return status.
typedef enum
{
    /**
     * Guard scope settings.
     */
    RETURN_STATUS_NORMAL = 0,
    RETURN_STATUS_ERROR = 1,
    RETURN_STATUS_FAILURE = -1
}  ReturnStatus;

/**
 * Main activation settings section created for easy
 * copying between related activations.
 */
class ActivationSettings
{
    typedef enum
    {
        singleStep,              // we are single stepping execution
        singleStepNested,        // this is a nested stepping
        debugPromptIssued,       // debug prompt already issued
        debugBypass,             // skip next debug pause
        procedureValid,          // procedure instruction is valid
        haltCondition,           // a HALT condition occurred
        traceOn,                 // external trace ON condition occurred
        traceOff,                // external trace OFF condition occurred
        sourceTraced,            // source string has been traced
        clauseExits,             // need to call clause boundary exits
        forwarded,               // forward instruction active
        replyIssued,             // reply has already been issued
        setTraceOn,              // trace turned on externally
        setTraceOff,             // trace turned off externally
        trapsCopied,             // copy of trap info has been made
        returnStatusSet,         // had our first host command
        transferFailed,          // transfer of variable lock failure
        traceSuppress,           // tracing is currently suppressed
        elapsedReset,            // The elapsed time stamp was reset via time('r')
        guardedMethod,           // this is a guarded method
        ioConfigCopied,          // We have made a copy of the config table
    } ActivationFlag;


    public:
      inline ActivationSettings() {}

      void live(size_t);
      void liveGeneral(MarkReason reason);

      inline bool isForwarded() { return stateFlags[forwarded]; }
      inline void setForwarded(bool v = true) { stateFlags[forwarded] = v; }
      inline bool isGuarded() { return stateFlags[guardedMethod]; }
      inline void setGuarded(bool v = true) { stateFlags[guardedMethod] = v; }
      inline bool isDebugBypassed() { return stateFlags[debugBypass]; }
      inline void setDebugBypass(bool v = true) { stateFlags[debugBypass] = v; }
      inline bool isTraceSuppressed() { return stateFlags[traceSuppress]; }
      inline void setTraceSuppressed(bool v = true) { stateFlags[traceSuppress] = v; }
      inline bool haveExternalTraceOn() { return stateFlags[traceOn]; }
      inline void setExternalTraceOn(bool v = true) { stateFlags[traceOn] = v; }
      inline bool haveExternalTraceOff() { return stateFlags[traceOff]; }
      inline void setExternalTraceOff(bool v = true) { stateFlags[traceOff] = v; }
      inline bool isElapsedTimerReset() { return stateFlags[elapsedReset]; }
      inline void setElapsedTimerReset(bool v = true) { stateFlags[elapsedReset] = v; }
      inline bool isReplyIssued() { return stateFlags[replyIssued]; }
      inline void setReplyIssued(bool v = true) { stateFlags[replyIssued] = v; }
      inline bool areTrapsCopied() { return stateFlags[trapsCopied]; }
      inline void setTrapsCopied(bool v = true) { stateFlags[trapsCopied] = v; }
      inline bool isIOConfigCopied() { return stateFlags[ioConfigCopied]; }
      inline void setIOConfigCopied(bool v = true) { stateFlags[ioConfigCopied] = v; }
      inline bool haveClauseExits() { return stateFlags[clauseExits]; }
      inline void setHaveClauseExits(bool v = true) { stateFlags[clauseExits] = v; }
      inline bool hasTransferFailed() { return stateFlags[transferFailed]; }
      inline void setTransferFailed(bool v = true) { stateFlags[transferFailed] = v; }
      inline bool isProcedureValid() { return stateFlags[procedureValid]; }
      inline void setProcedureValid(bool v = true) { stateFlags[procedureValid] = v; }
      inline bool wasDebugPromptIssued() { return stateFlags[debugPromptIssued]; }
      inline void setDebugPromptIssued(bool v = true) { stateFlags[debugPromptIssued] = v; }
      inline bool isReturnStatusSet() { return stateFlags[returnStatusSet]; }
      inline void setReturnStatus(bool v = true) { stateFlags[returnStatusSet] = v; }
      inline bool wasSourceTraced() { return stateFlags[sourceTraced]; }
      inline void setSourceTraced(bool v = true) { stateFlags[sourceTraced] = v; }
      inline bool haveHaltCondition() { return stateFlags[haltCondition]; }
      inline void setHaltCondition(bool v = true) { stateFlags[haltCondition] = v; }

      StringTable    *traps;               // enabled condition traps
      StringTable    *ioConfigs;           // address environment io configurations
      DirectoryClass *conditionObj;        // current condition object
      RexxObject    **parentArgList;       // arguments to top level program
      size_t          parentArgCount;      // number of arguments to the top level program
      RexxCode       *parentCode;          // source of the parent method
      RexxString     *currentAddress;      // current address environment
      RexxString     *alternateAddress;    // alternate address environment
      RexxString     *messageName;         // message sent to the receiver
                                           // object variable dictionary
      VariableDictionary *objectVariables;
      RexxString     *calltype;            // (COMMAND/METHOD/FUNCTION/ROUTINE)
      StringTable    *streams;             // table of opened streams
      RexxString     *haltDescription;     // description from a HALT condition
      SecurityManager *securityManager;    // security manager object
      RexxClass      *scope;               // scope of the method call
      PackageSetting packageSettings;      // inherited package settings
      FlagSet<ActivationFlag, 32> stateFlags;  // trace/numeric and other settings
      wholenumber_t traceSkip;             // count of trace events to skip
      bool intermediateTrace;              // very quick test for intermediate trace
      size_t  traceIndent;                 // trace indentation
      ReturnStatus returnStatus;           // command return status

      int64_t elapsedTime;                 // elapsed time clock
      RexxDateTime timeStamp;              // current timestamp
      RexxLocalVariables localVariables;   // the local variables for this activation
      StringTable    *fileNames;           // table of stream file names
      bool caseInsensitive;                // !SysFileSystem::isCaseSensitive();
};

#endif

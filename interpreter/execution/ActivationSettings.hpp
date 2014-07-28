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
/* Class to encapuslate the various settings that are shared between          */
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
        traceOn,                 // external trace condition occurred
        sourceTraced,            // source string has been traced
        clauseExits,             // need to call clause boundary exits
        externalYield,           // activity wants us to yield
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
    } ActivationFlag;

// TODO:  fix up the default settings


    public:
      inline ActivationSettings() {}

      StringTable    *traps;               // enabled condition traps
      DirectoryClass *conditionObj;        // current condition object
      RexxObject    **parentArgList;       // arguments to top level program
      size_t          parentArgCount;      // number of arguments to the top level program
      // TODO:  should this be a base executable?
      MethodClass    *parentMethod;        // method object for top level
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
      FlagSet<32, ActivationFlag> stateFlags;  // trace/numeric and other settings
      wholenumber_t traceSkip;             // count of trace events to skip
      bool intermediateTrace;              // very quick test for intermediate trace
      size_t  traceIndent;                 // trace indentation
      int  returnStatus;                   // command return status

      // TODO:  encapsulate the timing stuff in a class.
      int64_t elapsedTime;                 // elapsed time clock
      RexxDateTime timeStamp;              // current timestamp
      RexxLocalVariables localVariables;   // the local variables for this activation
};

#endif

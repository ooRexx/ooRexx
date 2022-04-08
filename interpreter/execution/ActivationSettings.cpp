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
#include "RexxCore.h"
#include "ActivationSettings.hpp"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "StringTableClass.hpp"
#include "RexxCode.hpp"


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ActivationSettings::live(size_t liveMark)
{
    memory_mark(traps);
    memory_mark(ioConfigs);
    memory_mark(conditionObj);
    memory_mark_array(parentArgCount, parentArgList);
    memory_mark(parentCode);
    memory_mark(currentAddress);
    memory_mark(alternateAddress);
    memory_mark(messageName);
    memory_mark(objectVariables);
    memory_mark(calltype);
    memory_mark(streams);
    memory_mark(haltDescription);
    memory_mark(securityManager);
    memory_mark(scope);
    memory_mark(fileNames);
    // local variables handle their own marking.
    localVariables.live(liveMark);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ActivationSettings::liveGeneral(MarkReason reason)
{
    memory_mark_general(traps);
    memory_mark_general(ioConfigs);
    memory_mark_general(conditionObj);
    memory_mark_general_array(parentArgCount, parentArgList);
    memory_mark_general(parentCode);
    memory_mark_general(currentAddress);
    memory_mark_general(alternateAddress);
    memory_mark_general(messageName);
    memory_mark_general(objectVariables);
    memory_mark_general(calltype);
    memory_mark_general(streams);
    memory_mark_general(haltDescription);
    memory_mark_general(securityManager);
    memory_mark_general(scope);
    memory_mark_general(fileNames);
    // local variables handle their own marking.
    localVariables.liveGeneral(reason);
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* State of an activation condition trap.                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "TrapHandler.hpp"

#include "CallInstruction.hpp"
#include "DirectoryClass.hpp"
#include "GlobalNames.hpp"


/**
 * Allocate a new TrapHandler item
 *
 * @param size    The base object size.
 *
 * @return The storage for creating a MapBucket.
 */
void *TrapHandler::operator new(size_t size)
{
   return new_object(size, T_TrapHandler);
}


/**
 * Construct a new trap handler object.
 *
 * @param c      The string condition name.
 * @param h      The SIGNAL or CALL instruction that will handle the
 *               condition.
 */
TrapHandler::TrapHandler(RexxString *c, RexxInstructionTrapBase *h)
{
    condition = c;
    handler = h;
    state = ON;
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void TrapHandler::live(size_t liveMark)
{
    memory_mark(condition);
    memory_mark(handler);
    memory_mark(conditionObject);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void TrapHandler::liveGeneral(MarkReason reason)
{
    memory_mark_general(condition);
    memory_mark_general(handler);
    memory_mark_general(conditionObject);
}


/**
 * Determine if a given trap handler can process a given
 * condition with an ANY condition.
 *
 * @param c      The condition name.
 *
 * @return true if this handler can process the condition, false otherwise.
 */
bool TrapHandler::canHandle(RexxString *c)
{
    // if the trap is CALL ON, there are only a few
    // conditions we can trap with an ANY
    if (handler->isType(KEYWORD_CALL_ON) &&
        (c->strCompare(GlobalNames::SYNTAX) ||
         c->strCompare(GlobalNames::NOVALUE) ||
         c->strCompare(GlobalNames::LOSTDIGITS) ||
         c->strCompare(GlobalNames::NOMETHOD) ||
         c->strCompare(GlobalNames::NOSTRING)))
    {
        return false;
    }
    return true;
}


/**
 * Return the instruction name to be used in the
 * condition object.
 *
 * @return The string name of the instruction (CALL or SIGNAL)
 */
RexxString *TrapHandler::instructionName()
{
    return handler->isType(KEYWORD_CALL_ON) ? GlobalNames::CALL : GlobalNames::SIGNAL;
}


/**
 * Attach the condition object to the trap handler.
 *
 * @param c      The condition object associated with the trapped condition.
 */
void TrapHandler::setConditionObject(DirectoryClass *c)
{
    conditionObject = c;
}


/**
 * Test the type of instruction is handling this condition.
 *
 * @return true if this is a SIGNAL ON, false if this is a CALL ON.
 */
bool TrapHandler::isSignal()
{
    return handler->isType(KEYWORD_SIGNAL_ON);
}


/**
 * Test if the handler is in a delayed state for a CALL ON condition.
 *
 * @return true if the trap is delayed, false otherwise.
 */
bool TrapHandler::isDelayed()
{
    return state == DELAYED;
}


/**
 * Get the condition object attached to this handler.
 *
 * @return The attached condition object.
 */
DirectoryClass *TrapHandler::getConditionObject()
{
    return conditionObject;
}


/**
 * Process a trapped condition.
 *
 * @param context The trapping activation context.
 */
void TrapHandler::trap(RexxActivation *context)
{
    // just forward to the handler instruction.
    handler->trap(context, conditionObject);
}


/**
 * Get the trap state as a string.
 *
 * @return The tray state, either ON or DELAY.
 */
RexxString *TrapHandler::getState()
{
    return state == ON ? GlobalNames::ON : GlobalNames::DELAY;
}


/**
 * Disable the trap during a CALL ON trap.
 */
void TrapHandler::disable()
{
    state = DELAYED;
}


/**
 * Reenable a trap after a CALL ON trap call.
 */
void TrapHandler::enable()
{
    state = ON;
    // this gets called after a CALL ON has been processed.  We clear
    // the attched condition object once we are done.
    conditionObject = OREF_NULL;
}


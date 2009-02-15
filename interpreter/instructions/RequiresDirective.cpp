/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* REXX Kernel                                        RequiresDirective.cpp   */
/*                                                                            */
/* Primitive Translator Abstract Directive Code                               */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RequiresDirective.hpp"
#include "Clause.hpp"
#include "RexxActivation.hpp"


/**
 * Construct a RequiresDirective.
 *
 * @param n      The name of the requires target.
 * @param clause The source file clause containing the directive.
 */
RequiresDirective::RequiresDirective(RexxString *n, RexxClause *clause) : RexxDirective(clause, KEYWORD_REQUIRES)
{
    name = n;
}

/**
 * Normal garbage collecting live mark.
 *
 * @param liveMark The current live object mark.
 */
void RequiresDirective::live(size_t liveMark)
{
    memory_mark(this->nextInstruction);  // must be first one marked (though normally null)
    memory_mark(this->name);
}


/**
 * The generalized object marking routine.
 *
 * @param reason The processing faze we're running the mark on.
 */
void RequiresDirective::liveGeneral(int reason)
{
    memory_mark_general(this->nextInstruction);  // must be first one marked (though normally null)
    memory_mark_general(this->name);
}


/**
 * Flatten the directive instance.
 *
 * @param envelope The envelope we're flattening into.
 */
void RequiresDirective::flatten(RexxEnvelope *envelope)
{
    setUpFlatten(RequiresDirective)

        flatten_reference(newThis->nextInstruction, envelope);
        flatten_reference(newThis->name, envelope);
    cleanUpFlatten
}


/**
 * Allocate a new requires directive.
 *
 * @param size   The size of the object.
 *
 * @return The memory for the new object.
 */
void *RequiresDirective::operator new(size_t size)
{
    return new_object(size, T_RequiresDirective); /* Get new object                    */
}


/**
 * Do install-time processing of the ::requires directive.  This
 * will resolve the directive and merge all of the public information
 * from the resolved file into this program context.
 *
 * @param activation The activation we're running under for the install.
 */
void RequiresDirective::install(RexxActivation *context)
{
    context->loadRequired(name, this);
}

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
/* REXX Kernel                                       RexxActivationStack.c    */
/*                                                                            */
/* Primitive Activation Frame Stack support classes                           */
/*                                                                            */
/* NOTE:  activations are an execution time only object.  They are never      */
/*        flattened or saved in the image, and hence will never be in old     */
/*        space.  Because of this, activations "cheat" and do not use         */
/*        OrefSet to assign values to get better performance.  Care must be   */
/*        used to maintain this situation.                                    */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "RexxActivationStack.hpp"


void RexxActivationFrameBuffer::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  /* we only mark housekeeping type fields.  The main buffer */
  /* entries are marked by the owning activations. */
  memory_mark(this->previous);
}

void RexxActivationFrameBuffer::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark_general(this->previous);
}

void RexxActivationFrameBuffer::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxActivationFrameBuffer)
  /* we only mark housekeeping type fields.  The main buffer */
  /* entries are marked by the owning activations. */

   flatten_reference(newThis->previous, envelope);

  cleanUpFlatten
}


void RexxActivationStack::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->current);
  memory_mark(this->unused);
}

void RexxActivationStack::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark_general(this->current);
  memory_mark_general(this->unused);
}

void RexxActivationStack::init()
/******************************************************************************/
/* Function:  Initialize a frame stack for a new activity.                    */
/******************************************************************************/
{
    /* create a new frame buffer with the default size. */
    current = new_activationFrameBuffer(DefaultFrameBufferSize);
    unused = OREF_NULL;
}

void RexxActivationStack::expandCapacity(size_t entries)
/******************************************************************************/
/* Function:  Expand the capacity of the stack to add at least entries        */
/*            additional values on the stack.                                 */
/******************************************************************************/
{
    RexxActivationFrameBuffer *next;
    entries = Numerics::maxVal(entries, (stringsize_t)DefaultFrameBufferSize);
    /* do we have an unused one we're holding ready that has enough */
    /* room? */
    if (unused != OREF_NULL && unused->hasCapacity(entries))
    {
        /* just activate this one for use */
        next = unused;
        unused = OREF_NULL;
    }
    else
    {
        /* create a new frame buffer */
        next = new_activationFrameBuffer(entries);
    }
    /* chain the existing buffer off of the new one */
    next->push(current);
    /* set this up as the new current stack */
    current = next;
}


RexxActivationFrameBuffer *RexxActivationFrameBuffer::newInstance(
    size_t entries)                   /* space for entries in the fame     */
/******************************************************************************/
/* Function:  Create a new expression stack                                   */
/******************************************************************************/
{
  RexxActivationFrameBuffer *newObj;   /* newly created buffer              */

                                       /* Get new object                    */
  newObj = (RexxActivationFrameBuffer *)new_object(sizeof(RexxActivationFrameBuffer) + (entries * sizeof(RexxObject *)), T_ActivationFrameBuffer);
  newObj->size = entries;              /* set the size                      */
  newObj->next = 0;                    /* set the top element               */
  newObj->previous = OREF_NULL;        /* no previous element yet           */
  return newObj;                       /* return the new stack item         */
}

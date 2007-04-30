/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                           MutableBufferClass.hpp  */
/*                                                                            */
/* Primitive MutableBuffer Class Definition                                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxMutableBuffer
#define Included_RexxMutableBuffer

#include "StringClass.hpp"
#include "IntegerClass.hpp"

class RexxMutableBuffer;

class RexxMutableBufferClass : public RexxClass {
 public:
   RexxMutableBufferClass(RESTORETYPE restoreType) { ; };
   void *operator new(size_t size, void *ptr) { return ptr; };
   RexxMutableBuffer *newRexx(RexxObject**, size_t);
};

 class RexxMutableBuffer : public RexxObject {
  public:
   inline void       *operator new(size_t size, void *ptr){return ptr;};
   inline             RexxMutableBuffer() {;} ;
   inline             RexxMutableBuffer(RESTORETYPE restoreType) { ; };

   void               live();
   void               liveGeneral();
   void               flatten(RexxEnvelope *envelope);
   RexxObject        *unflatten(RexxEnvelope *);

   RexxObject        *copy();

   RexxObject        *lengthRexx() { return this->data->lengthRexx(); }
   RexxString        *requestString() { return new_string(this->data->stringData, this->data->length); } /* NEVER return the reference we hold, always a copy! */
   RexxObject        *requestRexx(RexxString*);

   RexxMutableBuffer *append(RexxObject*);
   RexxMutableBuffer *insert(RexxObject*, RexxObject*, RexxObject*, RexxObject*);
   RexxMutableBuffer *overlay(RexxObject*, RexxObject*, RexxObject*, RexxObject*);
   RexxMutableBuffer *mydelete(RexxObject*, RexxObject*);
   RexxString        *substr(RexxInteger *startPosition, RexxInteger *len, RexxString *pad) { return this->data->substr(startPosition,len,pad); }
   RexxInteger       *lastPos(RexxString *needle, RexxInteger *start) { return this->data->lastPosRexx(needle, start); }
   RexxInteger       *posRexx(RexxString *needle, RexxInteger *start) { return this->data->posRexx(needle, start); }
   RexxString        *subchar(RexxInteger *startPosition) { return this->data->subchar(startPosition); }

   RexxInteger       *getBufferSize() { return new_integer(bufferLength); }
   RexxObject        *setBufferSize(RexxInteger*);
   void               uninitMB();

   size_t             bufferLength;    /* buffer length                   */
   size_t             defaultSize;     /* default size when emptied       */
   RexxString        *data;            /* string (not to be shared with   */
                                       /* the world as a string, NEVER!!) */
 };
#endif

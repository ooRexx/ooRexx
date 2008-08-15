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
/* REXX Kernel                                        MutableBufferClass.hpp  */
/*                                                                            */
/* Primitive MutableBuffer Class Definition                                   */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxMutableBuffer
#define Included_RexxMutableBuffer

#include "StringClass.hpp"
#include "IntegerClass.hpp"
#include "BufferClass.hpp"

class RexxMutableBuffer;
class RexxClass;

class RexxMutableBufferClass : public RexxClass {
 public:
   RexxMutableBufferClass(RESTORETYPE restoreType) { ; };
   void *operator new(size_t size, void *ptr) { return ptr; };
   RexxMutableBuffer *newRexx(RexxObject**, size_t);
};

 class RexxMutableBuffer : public RexxObject {
     friend class RexxMutableBufferClass;
  public:
   inline void       *operator new(size_t size, void *ptr){return ptr;};
          void       *operator new(size_t size, RexxClass *bufferClass);
          void       *operator new(size_t size);
                      RexxMutableBuffer();
                      RexxMutableBuffer(size_t, size_t);
   inline             RexxMutableBuffer(RESTORETYPE restoreType) { ; };

   void               live(size_t);
   void               liveGeneral(int reason);
   void               flatten(RexxEnvelope *envelope);

   RexxObject        *copy();
   void               ensureCapacity(size_t addedLength);

   RexxObject        *lengthRexx();

   RexxMutableBuffer *append(RexxObject*);
   RexxMutableBuffer *insert(RexxObject*, RexxObject*, RexxObject*, RexxObject*);
   RexxMutableBuffer *overlay(RexxObject*, RexxObject*, RexxObject*, RexxObject*);
   RexxMutableBuffer *replaceAt(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad);
   RexxMutableBuffer *mydelete(RexxObject*, RexxObject*);
   RexxString        *substr(RexxInteger *startPosition, RexxInteger *len, RexxString *pad);
   RexxInteger       *lastPos(RexxString *needle, RexxInteger *_start);
   RexxInteger       *posRexx(RexxString *needle, RexxInteger *_start);
   RexxString        *subchar(RexxInteger *startPosition);

   RexxInteger       *getBufferSize() { return new_integer(bufferLength); }
   RexxObject        *setBufferSize(RexxInteger*);
   RexxArray         *makearray(RexxString *div);
   RexxString        *makeString();
   RexxInteger       *countStrRexx(RexxString *needle);
   RexxInteger       *caselessCountStrRexx(RexxString *needle);

   inline const char *getStringData() { return data->getData(); }
   inline size_t      getLength()     { return dataLength; }

   static void createInstance();
   static RexxClass *classInstance;

 protected:

   size_t             bufferLength;    /* buffer length                   */
   size_t             defaultSize;     /* default size when emptied       */
   size_t             dataLength;      // current length of data
   RexxBuffer        *data;            /* buffer used for the data        */
 };
#endif

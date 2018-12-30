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
/* Primitive Compound Tail Class Definitions                                  */
/*                                                                            */
/******************************************************************************/

#ifndef Included_CompoundVariableTail
#define Included_CompoundVariableTail

#include "LanguageParser.hpp"
#include "BufferClass.hpp"
#include "ProtectedObject.hpp"

class BufferClass;

/**
 * A builder for constructing a tail for a compound variable.
 */
class CompoundVariableTail
{
  public:
   // build up a tail using a variable dictionary for resolving variable references
   inline CompoundVariableTail(VariableDictionary *dictionary, RexxInternalObject **tails, size_t tailCount)
   {
       init();                                  // do the common initialization
       buildTail(dictionary, tails, tailCount); // build the full tail up
   }

   // build up a tail using an activation variable context for resolving references
   inline CompoundVariableTail(RexxActivation *context, RexxInternalObject **tails, size_t tailCount)
   {
       init();
       buildTail(context, tails, tailCount);
   }

   // build up a tail using an array of resolved object (an argument list from a [] method call)
   inline CompoundVariableTail(RexxInternalObject **tails, size_t count)
   {
       init();
       buildTail(tails, count);
   }

   // build a tail from a single string value (easiest form)
   inline CompoundVariableTail(RexxString *tails, size_t count)
   {
       init();
       buildTail(tails, count);
   }

   // build a tail from a single string value (easiest form)
   inline CompoundVariableTail(RexxString *tailString)
   {
       init();
       buildTail(tailString);
   }

   // build a tail from a ASCII-Z string value
   inline CompoundVariableTail(const char *tailString)
   {
       init();
       buildTail(tailString);
   }

   // build a compound tail from a numeric index value (pseudo-array usage)
   inline CompoundVariableTail(size_t index)
   {
       init();
       buildTail(index);
   }

   // build a tail from an array of objects.  The resolve flag indicates whether
   // these are constants or variables.
   inline CompoundVariableTail(RexxInternalObject **tails, size_t count, bool resolve)
   {
       init();
       if (resolve)
       {
           buildTail(tails, count);
       }
       else
       {
           buildUnresolvedTail(tails, count);
       }
   }

   inline ~CompoundVariableTail() { }

   inline void ensureCapacity(size_t needed) { if (remainder < needed) expandCapacity(needed); }
   void expandCapacity(size_t needed);

   // apend data to the constructed tail
   inline void append(const char *newData, size_t stringLen)
   {
        ensureCapacity(stringLen);               // make sure have have space
        memcpy(current, newData, stringLen);     // copy this into the buffer
        current += stringLen;                    // step the pointer
        remainder -= stringLen;                  // adjust the lengths
   }

   // append a single character to the tail name
   inline void append(char newData)
   {
       ensureCapacity(1);
       *current = newData;
       current++;
       remainder--;
   }

   RexxString *createCompoundName(RexxString *);
   RexxString *makeString();

   inline void init()
   {
       length = 0;                              // set the initial lengths
       remainder = LanguageParser::MAX_SYMBOL_LENGTH;
       tail = buffer;                           // the default tail is the buffer
       current = tail;                          // the current pointer is the beginning
       temp = OREF_NULL;                        // we don't have a temporary here
       value = OREF_NULL;                       // and no string value yet
   }

   void buildTail(VariableDictionary *dictionary, RexxInternalObject **tails, size_t tailCount);
   void buildTail(RexxActivation *context, RexxInternalObject **tails, size_t tailCount);
   void buildTail(RexxInternalObject **tails, size_t count);
   void buildTail(RexxString *tail);
   void buildTail(RexxString *tail, size_t index);
   void buildTail(size_t index);
   void buildTail(const char *index);
   void buildUnresolvedTail(RexxInternalObject **tails, size_t count);

   inline void addDot() { append('.'); }
   inline int compare(RexxString *name)
   {
       // since we sort using both the length and comparison result, we can't just do
       // a string compare.
       wholenumber_t rc = (wholenumber_t)length - (wholenumber_t)name->getLength();
       if (rc == 0)
       {
           rc = memcmp(tail, name->getStringData(), length);
       }
       return (int)rc;
   }

   // use a single string value as a tail name directly
   inline void useStringValue(RexxString *rep)
   {
       // point directly to the value and the length
       tail = rep->getWritableData();
       length = rep->getLength();
       remainder = 0;                       // belt and braces...this will force a reallocation if we append
       value = rep;                         // save this reference in case we're asked for it later
   }

   inline size_t getLength() { return length; }
   inline const char *getTail()  { return tail; }

   static const size_t ALLOCATION_PAD = 100;

 protected:

     size_t  length;                     // length of the buffer (current)
     size_t  remainder;                  // remaining length in the buffer
     char   *tail;                       // the start of the tail buffer
     char   *current;                    // current write position
     char    buffer[LanguageParser::MAX_SYMBOL_LENGTH];  // the default buffer
     RexxString  *value;                 // a created string value
     Protected<BufferClass> temp;        // potential temporary buffer
 };
#endif

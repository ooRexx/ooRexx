/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* REXX Kernel                                        RexxCompoundTail.hpp    */
/*                                                                            */
/* Primitive Compound Tail Class Definitions                                  */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxCompoundTail
#define Included_RexxCompoundTail

#include "ProtectedObject.hpp"

class RexxBuffer;

 class RexxCompoundTail {

    enum { ALLOCATION_PAD = 100 } ;   /* amount of padding added when the buffer size is increased */

  public:
   inline RexxCompoundTail(RexxVariableDictionary *dictionary, RexxObject **tails, size_t tailCount)
   {
       init();                                  /* do the common initialization */
       buildTail(dictionary, tails, tailCount); /* build the full tail up */
   }

   inline RexxCompoundTail(RexxActivation *context, RexxObject **tails, size_t tailCount)
   {
       init();                                  /* do the common initialization */
       buildTail(context, tails, tailCount);    /* build the full tail up */
   }

   inline RexxCompoundTail(RexxObject **tails, size_t count)
   {
       init();                                  /* do the common initialization */
       buildTail(tails, count);                 /* build the full tail up */
   }
   inline RexxCompoundTail(RexxString *tails, size_t count)
   {
       init();                                  /* do the common initialization */
       buildTail(tails, count);                 /* build the full tail up */
   }

   inline RexxCompoundTail(RexxString *tailString)
   {
       init();                                  /* do the common initialization */
       buildTail(tailString);                   /* build the full tail up */
   }

   inline RexxCompoundTail(const char *tailString)
   {
       init();                                  /* do the common initialization */
       buildTail(tailString);                   /* build the full tail up */
   }

   inline RexxCompoundTail(size_t index)
   {
       init();                                  /* do the common initialization */
       buildTail(index);                        /* build the full tail up */
   }


   inline RexxCompoundTail(RexxObject **tails, size_t count, bool resolve) {
       init();                                  /* do the common initialization */
       if (resolve)
       {
           buildTail(tails, count);             /* build the full tail up */
       }
       else
       {
           buildUnresolvedTail(tails, count);   /* build the full tail up */
       }
   }

   inline ~RexxCompoundTail()
   {
   }

   inline void ensureCapacity(size_t needed) { if (remainder < needed) expandCapacity(needed); }
   void expandCapacity(size_t needed);
   inline void append(const char  *newData, size_t stringLen)
   {
        ensureCapacity(stringLen);               /* make sure have have space */
        memcpy(current, newData, stringLen);     /* copy this into the buffer */
        current += stringLen;                    /* step the pointer          */
        remainder -= stringLen;                  /* adjust the lengths        */
    }
   inline void append(char newData)
   {
       ensureCapacity(1);                       /* make sure have have space */
       *current = newData;                      /* store the character       */
       current++;                               /* step the pointer          */
       remainder--;                             /* adjust the lengths        */
   }

   RexxString *createCompoundName(RexxString *);
   RexxString *makeString();
   void init()
   {
       length = 0;                              /* set the initial lengths */
       remainder = MAX_SYMBOL_LENGTH;
       tail = buffer;                           /* the default tail is the buffer */
       current = tail;                          /* the current pointer is the beginning */
       temp = OREF_NULL;                        /* we don't have a temporary here */
       value = OREF_NULL;                       /* and no string value yet */
   }

   void buildTail(RexxVariableDictionary *dictionary, RexxObject **tails, size_t tailCount);
   void buildTail(RexxActivation *context, RexxObject **tails, size_t tailCount);
   void buildTail(RexxObject **tails, size_t count);
   void buildTail(RexxString *tail);
   void buildTail(RexxString *tail, size_t index);
   void buildTail(size_t index);
   void buildTail(const char *index);
   void buildUnresolvedTail(RexxObject **tails, size_t count);

   inline void addDot() { append('.'); }
   inline int compare(RexxString *name)
   {
       wholenumber_t rc = (wholenumber_t)length - (wholenumber_t)name->getLength();
       if (rc == 0)
       {
           rc = memcmp(tail, name->getStringData(), length);
       }
       return (int)rc;
   }

   inline size_t getLength() { return length; }
   inline const char *getTail()  { return tail; }

 protected:

   size_t  length;                     /* length of the buffer (current) */
   size_t  remainder;                  /* remaining length in the buffer */
   char   *tail;                       /* the start of the tail buffer   */
   char   *current;                    /* current write position         */
   char    buffer[MAX_SYMBOL_LENGTH];  /* the default buffer             */
   RexxString  *value;                 /* a created string value         */
   RexxBuffer *temp;                   // potential temporary buffer
   ProtectedObject p;                  // used to protect the temp buffer
 };
#endif

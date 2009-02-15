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
/* REXX Kernel                                           RexxCompoundTail.cpp */
/*                                                                            */
/* Support class for building a compound variable tail.                       */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxCompoundTail.hpp"
#include "BufferClass.hpp"
#include "Numerics.hpp"
#include "ExpressionVariable.hpp"


void RexxCompoundTail::buildUnresolvedTail(
    RexxObject **tails,                /* tail elements                     */
    size_t count)                      /* number of tail elements           */
/******************************************************************************/
/* Function:  Build a tail value from a set of tail elements without doing    */
/*            variable resolution.                                            */
/******************************************************************************/
{
    bool         first = true;           /* first tail piece indicator        */

    for (size_t i = 0; i < count; i++)
    {
        if (!first)
        {                    /* if not the first tail piece       */
            addDot();                    /* add a dot to the buffer           */
        }
        first = false;                   /* we need to add a dot from here on */
        RexxObject *part = tails[i];
        // this could be ommitted
        if (part != OREF_NULL)
        {
            // if this is a variable, just copy the name.  Otherwixe, copy the value
            if (isOfClass(VariableTerm, part))
            {
                ((RexxParseVariable *)part)->getName()->copyIntoTail(this);
            }
            else
            {
                part->stringValue()->copyIntoTail(this);
            }
        }
    }
    length = current - tail;             /* set the final, updated length     */
}


void RexxCompoundTail::buildTail(
    RexxVariableDictionary *dictionary,      /* source dictionary for tail piece values */
    RexxObject **tails,                      /* the count of tail pieces */
    size_t tailCount)
/******************************************************************************/
/* Function:  Construct a tail from its elements                              */
/******************************************************************************/
{
  if (tailCount == 1)
  {
      /* get the tail value */
      RexxObject *_tail = tails[0]->getValue(dictionary);
      /* if it is an integer type, we might be able to address the string representation directly. */
      if (isInteger(_tail))
      {
          RexxString *rep = ((RexxInteger *)_tail)->getStringrep();
          if (rep != OREF_NULL)
          {
              /* point directly to the value       */
              /* and the length */
              this->tail = rep->getWritableData();
              length = rep->getLength();
              remainder = 0;                       /* belt and braces...this will force a reallocation if we append */
              value = rep;                         /* save this reference in case we're asked for it later */
              return;
          }
      }
      /* if this is directly a string, we can use this directly */
      if (isString(_tail))
      {
          /* point directly to the value       */
          /* and the length */
          this->tail = ((RexxString *)_tail)->getWritableData();
          length = ((RexxString *)_tail)->getLength();
          remainder = 0;                       /* belt and braces...this will force a reallocation if we append */
          value = (RexxString *)_tail;         /* save this reference in case we're asked for it later */
          return;
      }
      /* some other type of object, or an integer without a string */
      /* rep.  We need to have it do the copy operation. */
      _tail->copyIntoTail(this);
      length = current - this->tail;             /* set the final, updated length     */
  }
  else
  {
      tails[0]->getValue(dictionary)->copyIntoTail(this);
      for (size_t i = 1; i < tailCount; i++)      /* process each element              */
      {
          addDot();                        /* add a dot to the buffer           */
          tails[i]->getValue(dictionary)->copyIntoTail(this);
      }

      length = current - tail;             /* set the final, updated length     */
  }
}


void RexxCompoundTail::buildTail(
    RexxActivation *context,                 /* source context for tail piece values */
    RexxObject **tails,                      /* the count of tail pieces */
    size_t tailCount)
/******************************************************************************/
/* Function:  Construct a tail from its elements                              */
/******************************************************************************/
{
    if (tailCount == 1)
    {
        /* get the tail value */
        RexxObject *_tail = tails[0]->getValue(context);
        /* if it is an integer type, we might be able to address the string representation directly. */
        if (isInteger(_tail))
        {
            RexxString *rep = ((RexxInteger *)_tail)->getStringrep();
            if (rep != OREF_NULL)
            {
                /* point directly to the value       */
                /* and the length */
                this->tail = rep->getWritableData();
                length = rep->getLength();
                remainder = 0;                       /* belt and braces...this will force a reallocation if we append */
                value = rep;                         /* save this reference in case we're asked for it later */
                return;
            }
        }
        /* if this is directly a string, we can use this directly */
        if (isString(_tail))
        {
            /* point directly to the value       */
            /* and the length */
            this->tail = ((RexxString *)_tail)->getWritableData();
            length = ((RexxString *)_tail)->getLength();
            remainder = 0;                       /* belt and braces...this will force a reallocation if we append */
            value = (RexxString *)_tail;         /* save this reference in case we're asked for it later */
            return;
        }
        /* some other type of object, or an integer without a string */
        /* rep.  We need to have it do the copy operation. */
        _tail->copyIntoTail(this);
        length = current - this->tail;             /* set the final, updated length     */
    }
    else
    {
        /* tail building buffer              */
        tails[0]->getValue(context)->copyIntoTail(this);
        for (size_t i = 1; i < tailCount; i++)      /* process each element              */
        {
            addDot();                        /* add a dot to the buffer           */
            tails[i]->getValue(context)->copyIntoTail(this);
        }

        length = current - tail;             /* set the final, updated length     */
    }
}


void RexxCompoundTail::buildTail(
    RexxObject **tails,                /* tail elements                     */
    size_t count)                      /* number of tail elements           */
/******************************************************************************/
/* Function:  Resolve the "stem.[a,b,c]=" pieces to a fullly qualified stem   */
/*            name.                                                           */
/******************************************************************************/
{
    bool         first = true;           /* first tail piece indicator        */

    for (size_t i = 0; i < count; i++)
    {
        if (!first)                      /* if not the first tail piece       */
        {
            addDot();                    /* add a dot to the buffer           */
        }
        first = false;                   /* we need to add a dot from here on */
        RexxString *part = (RexxString *)tails[i];   /* get the next element              */
        if (part == OREF_NULL)           /* omitted piece?                    */
        {
            part = OREF_NULLSTRING;      /* use a null string                 */
        }
        part->copyIntoTail(this);        /* add this to our tail              */
    }
    length = current - tail;             /* set the final, updated length     */
}


void RexxCompoundTail::buildTail(
    RexxString *_tail)                       /* the single string index */
/******************************************************************************/
/* Function:  Construct a tail from a single string index                     */
/******************************************************************************/
{
  /* point directly to the value       */
  this->tail = _tail->getWritableData();
  length = _tail->getLength();          /* and the length */
  remainder = 0;                        /* belt and braces...this will force a reallocation if we append */
  value = _tail;                        /* save this reference in case we're asked for it later */
}


/**
 * Build a tail directly from a single character string value.
 *
 * @param t      The pointer to the tail name (as an asciiz string);
 */
void RexxCompoundTail::buildTail(const char *t)
{
    this->tail = const_cast<char *>(t);
    length = strlen(t);
    remainder = 0;
}


void RexxCompoundTail::buildTail(
    RexxString *_tail, size_t index)                        /* the single string index */
/******************************************************************************/
/* Function:  Construct a tail from a string and an index                     */
/******************************************************************************/
{
    /* point directly to the value       */
    if (_tail->getStringData() != OREF_NULL)
    {
        _tail->copyIntoTail(this);        /* add this to our tail              */
    }
    length = length + _tail->getLength();
    length = length + Numerics::formatWholeNumber(index, current);
    current += length;
    remainder -= length;
}

void RexxCompoundTail::buildTail(
    size_t index)                            /* the single numeric index */
/******************************************************************************/
/* Function:  Construct a tail from a single numeric index                    */
/******************************************************************************/
{
    length = Numerics::formatWholeNumber(index, current);
    length = strlen((char *)current);
    current += length;
    remainder -= length;
}


void RexxCompoundTail::expandCapacity(
    size_t needed)                       /* length we require */
/******************************************************************************/
/* Function:  Ensure the buffer has sufficient space                          */
/******************************************************************************/
{
    length = current - tail;             /* update the accumulated length */

    if (temp != OREF_NULL)               /* have we already allocated a buffer? */
    {
                                         /* expand the size of our existing buffer  */
        temp->expand(needed + ALLOCATION_PAD);
        tail = temp->getData();
        current = tail + length;
        remainder += needed + ALLOCATION_PAD;
    }
    else
    {
                                         /* get a new buffer size */
        size_t newLength = length + needed + ALLOCATION_PAD;
        temp = (RexxBuffer *)new_buffer(newLength);
        p = temp;                  // this protects the buffer
        tail = temp->getData();
        current = tail + length;
        memcpy(tail, buffer, length);    /* make sure we copy the old data */
        remainder = newLength - length;  /* set the new remainder       */
    }
}


RexxString *RexxCompoundTail::createCompoundName(RexxString *stem)
/******************************************************************************/
/* Function:  Create a fully resolved compound name from a tail buffer        */
/******************************************************************************/
{
    size_t len1 = stem->getLength();                 /* get the stem length               */
    /* create a new string               */
    RexxString *result = (RexxString *)raw_string(len1 + length);
    char *data = result->getWritableData(); /* point to the string data          */
    if (len1 != 0)                       /* have real data?                   */
    {
        /* copy the front part               */
        memcpy(data, stem->getStringData(), len1);
        data += len1;                      /* step past the length              */
    }
    if (length != 0)                     /* have a second length              */
    {
        /* and the second part               */
        memcpy(data, tail, length);
    }
    return result;                       /* return the result                 */

}

RexxString *RexxCompoundTail::makeString()
{
    if (value == NULL)
    {
                                             /* create a new string               */
        value = (RexxString *)new_string(tail, length);
    }
    return value;                            /* return the result                 */
}

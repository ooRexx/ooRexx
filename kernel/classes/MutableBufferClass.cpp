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
/* REXX Kernel                                           MutableBufferClass.c    */
/*                                                                            */
/* Primitive MutableBuffer Class                                              */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "StringClass.hpp"
#include "MutableBufferClass.hpp"
#include "RexxBuiltinFunctions.h"                          /* Gneral purpose BIF Header file       */

#define CRIT_SIZE 0x400000

#define DEFAULT_BUFFER_LENGTH 256
static int        counter = 0;        /* is counting memory                */

RexxMutableBuffer *RexxMutableBufferClass::newRexx(RexxObject **args, size_t argc)
/******************************************************************************/
/* Function:  Allocate (and initialize) a string object                       */
/******************************************************************************/
{
  RexxString        *string;
  RexxMutableBuffer *newBuffer;         /* new mutable buffer object         */
  size_t            bufferLength = DEFAULT_BUFFER_LENGTH;
  size_t            defaultSize;
  if (argc >= 1) {
    if (args[0] != NULL) {
                                        /* force argument to string value    */
      string = (RexxString *)get_string(args[0], ARG_ONE);
    }
    else
    {
      string = OREF_NULLSTRING;           /* default to empty content          */
    }
  }
  else                                      /* minimum buffer size given?        */
  {
     string = OREF_NULLSTRING;
  }

  if (argc >= 2) {
    bufferLength = optional_length(args[1], DEFAULT_BUFFER_LENGTH, ARG_TWO);
  }

  defaultSize = bufferLength;           /* remember initial default size     */

                                        /* input string longer than demanded */
                                        /* minimum size? expand accordingly  */
  if (string->length > bufferLength) {
    bufferLength = string->length;
  }


/* It can be critical to handle memory out of Object REXX memory management  */
/* if memory processing depends on Object REXX Garbage collection and uninit */
/* cycle time. GC is always triggered by need in case segment borders are    */
/* reached, Objects are too big, aso. Privat managed memory can never trigger*/
/* garbage collection. This could lead to the problem, in the following situ-*/
/* ation:                                                                    */
/* - Lots of objects (that reside in Object REXX memory) need private memory.*/
/* - GC is not forced as the Objects memory is not managed by Object REXX    */
/* - Objects may not marked dead                                             */
/* - No uninit is started on that objects, so that no free is processed on   */
/*   private memory                                                          */
/* In case Objects have big private memory consumption, a GC and runUninits  */
/* must be forced                                                            */
  counter += bufferLength;

  if (counter > CRIT_SIZE )
  {
     counter = 0;
     TheMemoryObject->clearSaveStack();   /* be forced to remove any potential */
     TheMemoryObject->collect();          /* locks from the UNINIT table       */
     TheActivityClass->runUninits();      /* be sure to finish UNINIT methods  */
  }
                                        /* allocate the new object           */
  newBuffer = (RexxMutableBuffer *)new_object(sizeof(RexxMutableBuffer));
                                        /* set the behaviour from the class  */
  BehaviourSet(newBuffer, this->instanceBehaviour);
                                        /* set the virtual function table    */
  setVirtualFunctions(newBuffer, T_mutablebuffer);
                                        /* clear the front part              */
  ClearObjectLength(newBuffer, sizeof(RexxMutableBuffer));
  newBuffer->hashvalue = (long) newBuffer;
  newBuffer->bufferLength = bufferLength;/* save the length of the buffer    */
  newBuffer->defaultSize  = defaultSize; /* store the default buffer size    */
                                        /* create a string of that length    */
  /* the next line requires a "small novel" to describe what is being done   */
  /* here. this is very important. the buffer contents are represented by a  */
  /* string object. this string object IS NOT UNDER THE CONTROL of the Rexx  */
  /* memory management. the object should be considered INVALID and must     */
  /* NEVER be passed out as a pointer. it is done this way for two reasons:  */
  /*  a) for data that goes extremely large, not putting it into the regular */
  /*     memory management will avoid a permanent allocation of memory, as   */
  /*     Rexx memory will never be freed but reused once allocated. this may */
  /*     not be desirable for large data because large storage may not be    */
  /*     needed all the time.                                                */
  /*     note: as malloc is used here, an UNINIT must be defined where the   */
  /*           memory is freed.                                              */
  /*  b) to save the duplicate implementation of SUBSTR this "fake" string   */
  /*     object is used; only methods of the string class can be called,     */
  /*     others will fail.                                                   */
  /*                                                                         */
  /* as the string object that represents the contents is "faked" is must not*/
  /* be inserted into the mark/life cycle of the garbage collector!!!        */


  newBuffer->data = (RexxString *) malloc(sizeof(RexxString) + bufferLength);
                                        /* copy the content                  */
  memcpy(newBuffer->data->stringData, string->stringData, string->length);
                                        /* set the string length to the      */
                                        /* original length                   */
  newBuffer->data->length = string->length;
  newBuffer->data->generateHash();      /* recalculate hash value            */

  save(newBuffer);                      /* protect new object from GC        */
  newBuffer->hasUninit();               /* important! we have an UNINT method*/
  newBuffer->sendMessage(OREF_INIT, args, argc > 2 ? argc - 2 : 0);
  discard_hold(newBuffer);
  return newBuffer;
}


void RexxMutableBuffer::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->objectVariables);
  cleanUpMemoryMark
}

void RexxMutableBuffer::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral
}

void RexxMutableBuffer::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten a mutable buffer                                        */
/******************************************************************************/
{
  setUpFlatten(RexxMutableBuffer)

  flatten_reference(newThis->data, envelope);
  flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

RexxObject *RexxMutableBuffer::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten a mutable buffer                                      */
/******************************************************************************/
{
  envelope->addTable(this);
  return this;
}

RexxObject *RexxMutableBuffer::copy()
/******************************************************************************/
/* Function:  copy an object                                                  */
/******************************************************************************/
{

  RexxMutableBuffer *newObj = ((RexxMutableBufferClass*) TheMutableBufferClass)->newRexx(NULL, 0);
  free(newObj->data);                    /* we need our own buffer!           */

                                         /* see the comments in ::newRexx()!! */
  newObj->data = (RexxString *) malloc(bufferLength + sizeof(RexxString));
  newObj->data->length = this->data->length;
  memcpy(newObj->data->stringData, this->data->stringData, this->data->length);

  newObj->defaultSize = this->defaultSize;
  newObj->bufferLength = this->bufferLength;

  newObj->hashvalue = (long) newObj;

  return newObj;
}


RexxMutableBuffer *RexxMutableBuffer::append(RexxObject *obj)
/******************************************************************************/
/* Function:  append to the mutable buffer                                    */
/******************************************************************************/
{

  RexxString *string = get_string(obj, ARG_ONE);
  save(string);

  size_t      resultLength = this->data->length + string->length;

  if (resultLength > bufferLength) {     /* need to enlarge?                  */
    counter = counter - bufferLength;
    bufferLength *= 2;                   /* double the buffer                 */
    if (bufferLength < resultLength) {   /* still too small? use new length   */
      bufferLength = resultLength;
    }
    counter = counter + bufferLength;
//    if(bufferLength > CRIT_SIZE)            /* see comments in object creation method THU004A begin*/
    if(counter > CRIT_SIZE)            /* see comments in object creation method THU004A begin*/
    {
       counter = 0;
       TheMemoryObject->clearSaveStack();   /* be forced to remove any potential */
       TheMemoryObject->collect();          /* locks from the UNINIT table       */
       TheActivityClass->runUninits();      /* be sure to finish UNINIT methods  */
    }                                       /* THU004A end */
                                         /* see the comments in ::newRexx()!! */
    this->data = (RexxString *) realloc(this->data, bufferLength + sizeof(RexxString));
  }
  memcpy(this->data->stringData + this->data->length, string->stringData, string->length);
  this->data->length += string->length;
  discard(string);
  return this;
}

RexxMutableBuffer *RexxMutableBuffer::insert(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad)
/******************************************************************************/
/* Function:  insert string at given position                                 */
/******************************************************************************/
{
  // WARNING: no real DBCS support (pos == bytepos (maybe != character pos))
  size_t begin = 0;
  size_t insertLength;
  RexxString *string;
  char   padChar = ' ';

  string = get_string(str, ARG_ONE);

  begin = optional_length(pos, 0, ARG_TWO);
  insertLength = optional_length(len, string->getLength(), ARG_THREE);

  padChar = get_pad(pad, ' ', ARG_FOUR);
                                              /* does it fit into buffer?     */
  if (bufferLength < this->data->length + insertLength) {
                                              /* no, then enlarge buffer      */
    bufferLength *= 2;
    if (bufferLength < this->data->length + insertLength) {
      bufferLength = this->data->length + insertLength;
    }
                                         /* see the comments in ::newRexx()!! */
    this->data = (RexxString *) realloc(this->data, bufferLength + sizeof(RexxString));
  }
                                              /* create space in the buffer   */
  if (begin < this->data->length) {
    memmove(this->data->stringData + begin + insertLength, this->data->stringData + begin, this->data->length - begin);
  } else if (begin > this->data->length) {
                                              /* pad before insertion         */
    memset(this->data->stringData + this->data->length, (int) padChar, begin - this->data->length);
  }
  if (insertLength <= string->length) {
                                              /* insert string contents       */
    memcpy(this->data->stringData + begin, string->stringData, insertLength);
  } else {
                                              /* insert string contents       */
    memcpy(this->data->stringData + begin, string->stringData, string->length);
                                              /* pad after insertion          */
    memset(this->data->stringData + begin + string->length, (int) padChar, insertLength - string->length);
  }
  if (begin > this->data->length) {
    this->data->length = begin + insertLength;
  } else {
    this->data->length += insertLength;
  }

  return this;
}

RexxMutableBuffer *RexxMutableBuffer::overlay(RexxObject *str, RexxObject *pos, RexxObject *len, RexxObject *pad)
/******************************************************************************/
/* Function:  replace characters in buffer contents                           */
/******************************************************************************/
{
  size_t begin = 0;
  size_t replaceLength;
  RexxString *string;
  char   padChar = ' ';

  string = get_string(str, ARG_ONE);
  begin = optional_position(pos, 1, ARG_TWO) - 1;
  replaceLength = optional_length(len, string->getLength(), ARG_THREE);

  padChar = get_pad(pad, ' ', ARG_FOUR);

                                              /* does it fit into buffer?     */
  if (begin + replaceLength > bufferLength) {
                                              /* no, then enlarge buffer      */
    bufferLength *= 2;
    if (begin + replaceLength > bufferLength) {
      bufferLength = begin + replaceLength;
    }
                                         /* see the comments in ::newRexx()!! */
    this->data = (RexxString *) realloc(this->data, bufferLength + sizeof(RexxString));
  }

  if (begin > this->data->length) {           /* pad before overlay           */
    memset(this->data->stringData + this->data->length, (int) padChar, begin - this->data->length);
  }
  if (replaceLength <= string->length) {
                                              /* overlay buffer contents      */
    memcpy(this->data->stringData + begin, string->stringData, replaceLength);
  } else {
                                              /* insert string contents       */
    memcpy(this->data->stringData + begin, string->stringData, string->length);
                                              /* pad after overlay            */
    memset(this->data->stringData + begin + string->length, (int) padChar, replaceLength - string->length);
  }

  if (begin > this->data->length) {
    this->data->length = begin + replaceLength;
  } else if (begin + replaceLength > this->data->length) {
    this->data->length = begin + replaceLength;
  }

  return this;
}

RexxMutableBuffer *RexxMutableBuffer::mydelete(RexxObject *start, RexxObject *len)
/******************************************************************************/
/* Function:  delete character range in buffer                                */
/******************************************************************************/
{
  // WARNING: not DBCS enabled, works only on 8-bit data
  size_t begin = get_position(start, ARG_ONE) - 1;
  size_t range = optional_length(len, this->data->length - begin, ARG_TWO);


  if (begin < this->data->length) {           /* got some work to do?         */
    if (begin + range < this->data->length) { /* delete in the middle?        */
      memmove(this->data->stringData + begin, this->data->stringData + begin + range,
              this->data->length - (begin + range));
      this->data->length -= range;
    } else {
      this->data->length = begin;
    }
  }

  return this;
}

RexxObject *RexxMutableBuffer::setBufferSize(RexxInteger *start)
/******************************************************************************/
/* Function:  set the size of the buffer                                      */
/******************************************************************************/
{
  size_t newsize = get_length(start, ARG_ONE);

  if (newsize == 0) {                       /* reset contents?                      */
    bufferLength = defaultSize;
    free(this->data);                       /* see the comments in ::newRexx()!!!   */
    this->data = (RexxString *) malloc(sizeof(RexxString) + bufferLength);
    this->data->length = 0;
  } else if (newsize != bufferLength) {
    this->data = (RexxString *) realloc(this->data, newsize + sizeof(RexxString));
    if (newsize < this->data->length) {
      this->data->length = newsize;         /* truncate contents                    */
    }
    bufferLength = newsize;
  }
  return OREF_NULL;
}


RexxObject *RexxMutableBuffer::requestRexx(RexxString *classname)
/******************************************************************************/
/* Function:   convert mutable buffer into a string or an array               */
/******************************************************************************/
{
  RexxObject *result = TheNilObject;

  if (classname != OREF_NULL) {
    if (classname->length > 0) {
                                            /* convert to a string?                 */
      if (strcmp("STRING", classname->stringData) == 0) {
                                            /* return a copy of the contents        */
        result = new_string(this->data->stringData, this->data->length);
      }
                                            /* convert to an array?                 */
      else if (strcmp("ARRAY", classname->stringData) == 0) {
                                            /* make array of single lines of string */
        result = (RexxObject*) this->data->makeArray(OREF_NULL);
      }
    }
  }

  return result;
}

void RexxMutableBuffer::uninitMB()
/******************************************************************************/
/* Function:   free the allocated memory at the end of object's lifetime      */
/******************************************************************************/
{
  if (this->data) free(data);
}

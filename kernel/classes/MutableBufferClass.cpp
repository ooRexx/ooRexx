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
/* REXX Kernel                                        MutableBufferClass.c    */
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
#include "ProtectedObject.hpp"

#define DEFAULT_BUFFER_LENGTH 256

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
  if (string->getLength() > bufferLength) {
    bufferLength = string->getLength();
  }

                                        /* allocate the new object           */
  newBuffer = (RexxMutableBuffer *)new_object(sizeof(RexxMutableBuffer));
                                        /* set the behaviour from the class  */
  newBuffer->setBehaviour(this->getInstanceBehaviour());
                                        /* set the virtual function table    */
  newBuffer->setVirtualFunctions(RexxMemory::VFTArray[T_mutablebuffer]);
                                        /* clear the front part              */
  newBuffer->clearObject(sizeof(RexxMutableBuffer));
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
  memcpy(newBuffer->data->getWritableData(), string->getStringData(), string->getLength());
                                        /* set the string length to the      */
                                        /* original length                   */
  newBuffer->data->setLength(string->getLength());

  ProtectedObject p(newBuffer);
  newBuffer->hasUninit();               /* important! we have an UNINT method*/
  newBuffer->sendMessage(OREF_INIT, args, argc > 2 ? argc - 2 : 0);
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
  newObj->data->setLength(this->data->getLength());
  memcpy(newObj->data->getWritableData(), this->data->getStringData(), this->data->getLength());

  newObj->defaultSize = this->defaultSize;
  newObj->bufferLength = this->bufferLength;
  return newObj;
}


RexxMutableBuffer *RexxMutableBuffer::append(RexxObject *obj)
/******************************************************************************/
/* Function:  append to the mutable buffer                                    */
/******************************************************************************/
{

  RexxString *string = get_string(obj, ARG_ONE);
  size_t      resultLength = this->data->getLength() + string->getLength();

  if (resultLength > bufferLength) {     /* need to enlarge?                  */
    bufferLength *= 2;                   /* double the buffer                 */
    if (bufferLength < resultLength) {   /* still too small? use new length   */
      bufferLength = resultLength;
    }
                                         /* see the comments in ::newRexx()!! */
    this->data = (RexxString *) realloc(this->data, bufferLength + sizeof(RexxString));
  }
  memcpy(this->data->getWritableData() + this->data->getLength(), string->getStringData(), string->getLength());
  this->data->setLength(data->getLength() + string->getLength());
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
  if (bufferLength < this->data->getLength() + insertLength) {
                                              /* no, then enlarge buffer      */
    bufferLength *= 2;
    if (bufferLength < this->data->getLength() + insertLength) {
      bufferLength = this->data->getLength() + insertLength;
    }
                                         /* see the comments in ::newRexx()!! */
    this->data = (RexxString *) realloc(this->data, bufferLength + sizeof(RexxString));
  }
                                              /* create space in the buffer   */
  if (begin < this->data->getLength()) {
    memmove(this->data->getWritableData() + begin + insertLength, this->data->getWritableData() + begin, this->data->getLength() - begin);
  } else if (begin > this->data->getLength()) {
                                              /* pad before insertion         */
    memset(this->data->getWritableData() + this->data->getLength(), (int) padChar, begin - this->data->getLength());
  }
  if (insertLength <= string->getLength()) {
                                              /* insert string contents       */
    memcpy(this->data->getWritableData() + begin, string->getStringData(), insertLength);
  } else {
                                              /* insert string contents       */
    memcpy(this->data->getWritableData() + begin, string->getStringData(), string->getLength());
                                              /* pad after insertion          */
    memset(this->data->getWritableData() + begin + string->getLength(), (int) padChar, insertLength - string->getLength());
  }
  if (begin > this->data->getLength()) {
    this->data->setLength(begin + insertLength);
  } else {
    this->data->setLength(data->getLength() + insertLength);
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

  if (begin > this->data->getLength()) {           /* pad before overlay           */
    memset(this->data->getWritableData() + this->data->getLength(), (int) padChar, begin - this->data->getLength());
  }
  if (replaceLength <= string->getLength()) {
                                              /* overlay buffer contents      */
    memcpy(this->data->getWritableData() + begin, string->getStringData(), replaceLength);
  } else {
                                              /* insert string contents       */
    memcpy(this->data->getWritableData() + begin, string->getStringData(), string->getLength());
                                              /* pad after overlay            */
    memset(this->data->getWritableData() + begin + string->getLength(), (int) padChar, replaceLength - string->getLength());
  }

  if (begin > this->data->getLength()) {
    this->data->setLength(begin + replaceLength);
  } else if (begin + replaceLength > this->data->getLength()) {
    this->data->setLength(begin + replaceLength);
  }

  return this;
}

RexxMutableBuffer *RexxMutableBuffer::mydelete(RexxObject *_start, RexxObject *len)
/******************************************************************************/
/* Function:  delete character range in buffer                                */
/******************************************************************************/
{
  // WARNING: not DBCS enabled, works only on 8-bit data
  size_t begin = get_position(_start, ARG_ONE) - 1;
  size_t range = optional_length(len, this->data->getLength() - begin, ARG_TWO);


  if (begin < this->data->getLength()) {           /* got some work to do?         */
    if (begin + range < this->data->getLength()) { /* delete in the middle?        */
      memmove(this->data->getWritableData() + begin, this->data->getStringData() + begin + range,
              this->data->getLength() - (begin + range));
      this->data->setLength(data->getLength() - range);
    } else {
      this->data->setLength(begin);
    }
  }

  return this;
}

RexxObject *RexxMutableBuffer::setBufferSize(RexxInteger *size)
/******************************************************************************/
/* Function:  set the size of the buffer                                      */
/******************************************************************************/
{
  size_t newsize = get_length(size, ARG_ONE);

  if (newsize == 0) {                       /* reset contents?                      */
    bufferLength = defaultSize;
    free(this->data);                       /* see the comments in ::newRexx()!!!   */
    this->data = (RexxString *) malloc(sizeof(RexxString) + bufferLength);
    this->data->setLength(0);
  } else if (newsize != bufferLength) {
    this->data = (RexxString *) realloc(this->data, newsize + sizeof(RexxString));
    if (newsize < this->data->getLength()) {
      this->data->setLength(newsize);            /* truncate contents                    */
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
    if (classname->getLength() > 0) {
                                            /* convert to a string?                 */
      if (strcmp("STRING", classname->getStringData()) == 0) {
                                            /* return a copy of the contents        */
        result = new_string(this->data->getStringData(), this->data->getLength());
      }
                                            /* convert to an array?                 */
      else if (strcmp("ARRAY", classname->getStringData()) == 0) {
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

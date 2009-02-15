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
/*********************************************************************/
/*                                                                   */
/*  Function:  Queue support routines                                */
/*                                                                   */
/*********************************************************************/

#include <string.h>                    /* Get strcpy, strcat, etc.          */
#include <stdlib.h>

#include "RexxCore.h"                  /* global REXX declarations          */
#include "StringClass.hpp"

/********************************************************************************************/
/* Rexx_query_queue                                                                         */
/********************************************************************************************/
RexxMethod0(size_t, rexx_query_queue)
{
   size_t count = 0;                   /* count of lines                    */

                                       /* get the queue name                */
   RexxObjectPtr queue_name = (RexxStringObject)context->GetObjectVariable("NAMED_QUEUE");
                                       /* query the queue                   */
   RexxQueryQueue(context->ObjectToStringValue(queue_name), &count);

   return count;
}

/********************************************************************************************/
/* Rexx_pull_queue                                                                          */
/********************************************************************************************/
RexxMethod0(RexxObjectPtr, rexx_pull_queue)
{
   RXSTRING buf;                       /* pulled line buffer                */
   RexxReturnCode rc;                  /* pull return code                  */

                                       /* get the queue name                */
   RexxObjectPtr queue_name = context->GetObjectVariable("NAMED_QUEUE");

   buf.strptr = NULL;                  /* ask for a returned buffer         */
   buf.strlength = 0;
                                       /* pull a line                       */
   rc = RexxPullFromQueue(context->ObjectToStringValue(queue_name), &buf, NULL, RXQUEUE_NOWAIT);

   if (!rc)
   {                                   /* get a pulled line?                */
       RexxObjectPtr result = context->NewString(buf.strptr, buf.strlength);
       if (buf.strptr != OREF_NULL)
       {
           RexxFreeMemory(buf.strptr);
       }
       return result;
   }
   return context->Nil();              /* give back a failure               */
}

/********************************************************************************************/
/* Rexx_linein_queue                                                                        */
/********************************************************************************************/
RexxMethod0(RexxObjectPtr, rexx_linein_queue)
{
   RexxReturnCode rc;                  /* pull return code                  */
   RexxObjectPtr queue_name;           /* current queue name                */
   RXSTRING buf;

                                       /* get the queue name                */
   queue_name = context->GetObjectVariable("NAMED_QUEUE");

   buf.strptr = NULL;                  /* ask for a returned buffer         */
   buf.strlength = 0;
                                       /* pull a line                       */
   rc = RexxPullFromQueue(context->ObjectToStringValue(queue_name), &buf, NULL, RXQUEUE_WAIT);

   if (!rc)                            /* get a pulled line?                */
   {
       RexxObjectPtr result = context->NewString(buf.strptr, buf.strlength);
       if (buf.strptr != OREF_NULL)
       {
           RexxFreeMemory(buf.strptr);
       }
       return result;
   }
   return context->Nil();        /* give back a failure               */
}

/********************************************************************************************/
/* add a line to a rexx queue                                                               */
/********************************************************************************************/
wholenumber_t rexx_add_queue(
  RexxMethodContext *context,          // the call context
  RexxStringObject  queue_line,        /* line to add                       */
  int         order )                  /* queuing order                     */
{
    char buffer = 0;                   // buffer for an empty string
    CONSTRXSTRING rx_string;           // rxstring to push
    RexxReturnCode rc;                 // queue return code

   if (queue_line == NULLOBJECT)       /* no line given?                    */
   {
       // just use a null string value
       MAKERXSTRING(rx_string, &buffer, 0);
   }
   else
   {
       MAKERXSTRING(rx_string, context->StringData(queue_line), context->StringLength(queue_line));
   }
                                       /* get the queue name                */
   RexxObjectPtr queue_name = context->GetObjectVariable("NAMED_QUEUE");
                                       /*  move the line to the queue       */
   rc = RexxAddQueue(context->ObjectToStringValue(queue_name), &rx_string, order);
   if (rc != 0)                        /* stream error?                     */
   {
       context->RaiseException1(Rexx_Error_System_service_service, context->NewStringFromAsciiz("SYSTEM QUEUE"));
   }
   return rc;                          /* return the result                 */
}

/********************************************************************************************/
/* Rexx_push_queue                                                                          */
/********************************************************************************************/
RexxMethod1(wholenumber_t, rexx_push_queue,
   OPTIONAL_RexxStringObject, queue_line)  /* line to queue                     */
{
                                       /* push a line onto the queue        */
   return rexx_add_queue(context, queue_line, RXQUEUE_LIFO);
}

/********************************************************************************************/
/* Rexx_queue_queue                                                                         */
/********************************************************************************************/
RexxMethod1(wholenumber_t, rexx_queue_queue,
   OPTIONAL_RexxStringObject, queue_line)  /* line to queue                     */
{
                                       /* queue a line onto the queue       */
   return rexx_add_queue(context, queue_line, RXQUEUE_FIFO);
}

/********************************************************************************************/
/* Rexx_create_queue                                                                        */
/********************************************************************************************/
RexxMethod1(RexxStringObject, rexx_create_queue,
  CSTRING, queue_name)                 /* current queue name                */
{
   char buf[MAX_QUEUE_NAME_LENGTH+1];  /* creation buffer                   */
   RexxReturnCode rc;                  /* creation return code              */
   size_t        dup_flag = 0;         /* duplicate name flag               */

                                       /* create a queue                    */
   rc = RexxCreateQueue(buf, sizeof(buf), queue_name, &dup_flag);

   if (!rc)                            /* work ok?                          */
   {
       return context->NewStringFromAsciiz(buf);
   }
   return context->NullString();       /* just return a null string         */
}

/********************************************************************************************/
/* Rexx_open_queue                                                                        */
/********************************************************************************************/
RexxMethod1(logical_t, rexx_open_queue, CSTRING, queue_name)
{
   size_t        dup_flag = 0;         /* duplicate name flag               */
                                       /* create a queue                    */
   return RexxOpenQueue(queue_name, &dup_flag);
}

/********************************************************************************************/
/* Rexx_queue_exists                                                                      */
/********************************************************************************************/
RexxMethod1(logical_t, rexx_queue_exists, CSTRING, queue_name)
{
                                       /* create a queue                    */
   return RexxQueueExists(queue_name) == 0;
}

/********************************************************************************************/
/* Rexx_delete_queue                                                                        */
/********************************************************************************************/
RexxMethod1(wholenumber_t, rexx_delete_queue,
  CSTRING, queue_name)
{
                                       /* just delete the queue             */
  return RexxDeleteQueue(queue_name);
}

/********************************************************************************************/
/* Rexx_clear_queue                                                                         */
/********************************************************************************************/
RexxMethod0(int, rexx_clear_queue)
{
                                       /* get the queue name                */
  RexxObjectPtr queue_name = context->GetObjectVariable("NAMED_QUEUE");
                                       /* Clear the queue                   */
  return RexxClearQueue(context->ObjectToStringValue(queue_name));
}

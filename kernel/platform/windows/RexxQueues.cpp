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
/*********************************************************************/
/*                                                                   */
/*  Function:  System dependent queue support routines               */
/*                                                                   */
/*********************************************************************/

#include <string.h>                    /* Get strcpy, strcat, etc.          */
#include <stdlib.h>

#define INCL_RXSUBCOM                  /* Include subcom declares           */
#define INCL_RXFUNC                    /* and external function...          */
#define INCL_RXSYSEXIT                 /* and system exits                  */
#define INCL_RXQUEUE                   /* enable RxQueue_x() ability        */

#include "RexxCore.h"                    /* global REXX declarations          */
#include "StringClass.hpp"
#include "RexxNativeAPI.h"                      /* Lot's of useful REXX macros       */
#include "StreamNative.h"

#include SYSREXXSAA                    /* Include REXX header               */
#include "SubcommandAPI.h"             /* Get private REXXAPI API's         */

/********************************************************************************************/
/* Rexx_query_queue                                                                         */
/********************************************************************************************/
RexxMethod0(REXXOBJECT, rexx_query_queue)
{
   REXXOBJECT queue_name;              /* current queue name                */
   unsigned long count = 0;            /* count of lines                    */
   APIRET rc;                          /* queue query return code           */

                                       /* get the queue name                */
   queue_name = RexxVarValue("NAMED_QUEUE");
   REXX_GUARD_OFF();                   /* turn off the guard lock           */
                                       /* query the queue                   */
   rc = RexxQueryQueue((PSZ)string_data(queue_name), &count);
                                       /* return zero for any errors        */
   return rc ? IntegerZero : RexxInteger(count);
}

/********************************************************************************************/
/* Rexx_pull_queue                                                                          */
/********************************************************************************************/
RexxMethod0(REXXOBJECT, rexx_pull_queue)
{
   RXSTRING buf;                       /* pulled line buffer                */
   SYSTEMTIME dt;                      /* line time stamp                   */
   APIRET rc;                          /* pull return code                  */
   REXXOBJECT oref_buf;                /* returned string object            */
   REXXOBJECT queue_name;              /* current queue name                */

                                       /* get the queue name                */
   queue_name = RexxVarValue("NAMED_QUEUE");
   REXX_GUARD_OFF();                   /* turn off the guard lock           */

   buf.strptr = NULL;                  /* ask for a returned buffer         */
   buf.strlength = 0;
                                       /* pull a line                       */
   rc = RexxPullQueue((PSZ)string_data(queue_name), &buf, &dt, RXQUEUE_NOWAIT);

   if (!rc) {                          /* get a pulled line?                */
     oref_buf = RexxStringL(buf.strptr, buf.strlength);
     if (buf.strptr && buf.strlength)  /* have a queue item?                */
                                       /* free the buffer item              */
       SysReleaseResultMemory(buf.strptr);
     return oref_buf;                  /* return the item                   */
   }
   return TheNilObject;                /* give back a failure               */
}

/********************************************************************************************/
/* Rexx_linein_queue                                                                        */
/********************************************************************************************/
RexxMethod0(REXXOBJECT, rexx_linein_queue)
{
   RXSTRING buf;                       /* pulled line buffer                */
   SYSTEMTIME dt;                      /* line time stamp                   */
   APIRET rc;                          /* pull return code                  */
   REXXOBJECT oref_buf;                /* returned string object            */
   REXXOBJECT queue_name;              /* current queue name                */

                                       /* get the queue name                */
   queue_name = RexxVarValue("NAMED_QUEUE");
   REXX_GUARD_OFF();                   /* turn off the guard lock           */

   buf.strptr = NULL;                  /* ask for a returned buffer         */
   buf.strlength = 0;
                                       /* pull a line                       */
   rc = RexxPullQueue((PSZ)string_data(queue_name), &buf, &dt, RXQUEUE_WAIT);

   if (!rc) {                          /* get a pulled line?                */
     oref_buf = RexxStringL(buf.strptr, buf.strlength);
     if (buf.strptr)                   /* have a queue item?                */
                                       /* free the buffer item              */
       SysReleaseResultMemory(buf.strptr);
     return oref_buf;                  /* return the item                   */
   }
   return TheNilObject;                /* give back a failure               */
}

/********************************************************************************************/
/* add a line to a rexx queue                                                               */
/********************************************************************************************/
long rexx_add_queue(
  REXXOBJECT  queue_line,              /* line to add                       */
  INT         order )                  /* queuing order                     */
{
   RXSTRING rx_string;                 /* rxstring to return                */
   APIRET rc;                          /* queue return code                 */
   REXXOBJECT queue_name;              /* current queue name                */

   if (queue_line == NULLOBJECT)       /* no line given?                    */
     queue_line = OREF_NULLSTRING;     /* just add a null line              */
                                       /* get the queue name                */
   queue_name = RexxVarValue("NAMED_QUEUE");
   REXX_GUARD_OFF();                   /* turn off the guard lock           */
                                       /*  move the info to rxstring        */
   rx_string.strptr = string_data(queue_line);
   rx_string.strlength = string_length(queue_line);
                                       /*  move the line to the queue       */
   rc = RexxAddQueue((PSZ)string_data(queue_name), &rx_string, order);
   if (rc != 0)                        /* stream error?                     */
     send_exception1(Error_System_service_service, RexxArray1(RexxString("SYSTEM QUEUE")));
   return rc;                          /* return the result                 */
}

/********************************************************************************************/
/* Rexx_push_queue                                                                          */
/********************************************************************************************/
RexxMethod1(long, rexx_push_queue,
   REXXOBJECT, queue_line)             /* line to queue                     */
{
                                       /* push a line onto the queue        */
   return rexx_add_queue(queue_line, RXQUEUE_LIFO);
}

/********************************************************************************************/
/* Rexx_queue_queue                                                                         */
/********************************************************************************************/
RexxMethod1(long, rexx_queue_queue,
   REXXOBJECT, queue_line)             /* line to queue                     */
{
                                       /* queue a line onto the queue       */
   return rexx_add_queue(queue_line, RXQUEUE_FIFO);
}

/********************************************************************************************/
/* Rexx_create_queue                                                                        */
/********************************************************************************************/
RexxMethod1(REXXOBJECT, rexx_create_queue,
  CSTRING, queue_name)                 /* current queue name                */
{
   char buf[name_parameter_length+1];  /* creation buffer                   */
   APIRET rc;                          /* creation return code              */
   unsigned long dup_flag = 0;         /* duplicate name flag               */

                                       /* create a queue                    */
   rc = RexxCreateQueue((PSZ)buf, sizeof(buf), (PSZ)queue_name, &dup_flag);

   if (!rc)                            /* work ok?                          */
     return RexxString(buf);           /* return the created name           */

   return OREF_NULLSTRING;             /* just return a null string         */
}

/********************************************************************************************/
/* Rexx_delete_queue                                                                        */
/********************************************************************************************/
RexxMethod1(long, rexx_delete_queue,
  CSTRING, queue_name)
{
                                       /* just delete the queue             */
  return RexxDeleteQueue((PSZ)queue_name);
}

// retrofit by IH
/********************************************************************************************/
/* function_queueExit                                                                       */
/********************************************************************************************/
RexxMethod1(REXXOBJECT, function_queueExit,
  STRING, queue_name)                  /* the requested name                */
{
  RexxActivation *activation;          /* top level real activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  activation = (RexxActivation *)CurrentActivity->currentAct();
                                       /* call the exit                     */
  CurrentActivity->sysExitMsqNam(activation, (RexxString **)&queue_name);
  return_oref(queue_name);             /* and just return the exit result   */
}

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
/* REXX Kernel                                           MessageClass.hpp     */
/*                                                                            */
/* Primitive Message Class Definitions                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxMessage
#define Included_RexxMessage

#define  flagResultReturned   0x00000001
#define  flagRaiseError       0x00000002
#define  flagErrorReported    0x00000004
#define  flagAllNotified      0x00000008
#define  flagStartPending     0x00000010
#define  flagMsgSent          0x00000020

 class RexxMessage : public RexxObject {
  public:
   void * operator new(size_t);
   inline void * operator new(size_t size, void *objectPtr) { return objectPtr; };
                                        /* So it doesn't need to do anythin*/
   RexxMessage(RexxObject *, RexxString *, RexxObject *, RexxArray *);
   inline RexxMessage(RESTORETYPE restoreType) { ; };

   void          live(size_t);
   void          liveGeneral(int reason);
   void          flatten(RexxEnvelope *);
   RexxObject   *notify(RexxMessage *);
   RexxObject   *result();
   RexxObject   *send(RexxObject *);
   RexxObject   *start(RexxObject *);
   RexxObject   *completed();
   void          sendNotification();
   void          error(RexxDirectory *);
   RexxObject   *messageTarget();
   RexxString   *messageName();
   RexxArray    *arguments();
   RexxObject   *hasError();
   RexxObject   *errorCondition();
   RexxObject   *newRexx(RexxObject **, size_t);
   RexxActivity *getActivity() { return startActivity; }

   inline bool          resultReturned() { return (this->dataFlags & flagResultReturned) != 0; };
   inline bool          raiseError()     { return (this->dataFlags & flagRaiseError) != 0;     };
   inline bool          errorReported()  { return (this->dataFlags & flagErrorReported) != 0;  };
   inline bool          allNotified()    { return (this->dataFlags & flagAllNotified) != 0;    };
   inline bool          startPending()   { return (this->dataFlags & flagStartPending) != 0;   };
   inline bool          msgSent()        { return (this->dataFlags & flagMsgSent) != 0;        };
   inline void          setResultReturned() { this->dataFlags |= flagResultReturned; };
   inline void          setRaiseError()     { this->dataFlags |= flagRaiseError;     };
   inline void          setErrorReported()  { this->dataFlags |= flagErrorReported;  };
   inline void          setAllNotified()    { this->dataFlags |= flagAllNotified;    };
   inline void          setStartPending()   { this->dataFlags |= flagStartPending;   };
   inline void          setMsgSent()        { this->dataFlags |= flagMsgSent;        };

   static void createInstance();
   static RexxClass *classInstance;

 protected:

   RexxObject    *receiver;            /* Real receiver of message.         */
   RexxObject    *target;              /* Target object specified           */
   RexxString    *message;             /* Message to be sent                */
   RexxObject    *startscope;          /* Starting scope for method lookup  */
   RexxArray     *args;
   RexxObject    *resultObject;
   RexxList      *interestedParties;   /* message objects to be notified    */
   RexxDirectory *condition;           /* condition object, generated by    */
   RexxActivity *startActivity;        /* Activity created to run msg       */
   RexxList     *waitingActivities;    /* waiting activities list           */
   size_t dataFlags;                   /* flags to control processing       */
   SysSemaphore  waitResultSem;        /* Semophore used to wait on result  */
   size_t NumWaiting;                  /* activities waiting on result      */
 };

#endif

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
/* REXX Kernel                                           MessageClass.hpp     */
/*                                                                            */
/* Primitive Message Class Definitions                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_MessageClass
#define Included_MessageClass

#include "FlagSet.hpp"

/**
 * A dispatchable message object.
 */
class MessageClass : public RexxObject
{
  public:

    /**
     * Internal flag values.
     */
    typedef enum
    {
        flagResultReturned,
        flagRaiseError,
        flagErrorReported,
        flagAllNotified,
        flagStartPending,
        flagMsgActivated,
    } MessageFlag;

    void * operator new(size_t);

    MessageClass(RexxObject *, RexxString *, RexxClass *, ArrayClass *);
    inline MessageClass(RESTORETYPE restoreType) { ; };

    void  live(size_t) override;
    void  liveGeneral(MarkReason reason) override;
    void  flatten(Envelope *) override;
    RexxInternalObject *copy() override;

    RexxObject   *notify(RexxObject *);
    RexxObject   *result();
    RexxObject   *wait();
    RexxObject   *send();
    RexxObject   *dispatch();
    RexxObject   *start();
    MessageClass *reply();
    RexxObject   *startRexx(RexxObject **, size_t);
    RexxObject   *startWithRexx(RexxObject *, ArrayClass *);
    RexxObject   *replyRexx(RexxObject **, size_t);
    RexxObject   *replyWithRexx(RexxObject *, ArrayClass *);
    RexxObject   *sendRexx(RexxObject **, size_t);
    RexxObject   *sendWithRexx(RexxObject *, ArrayClass *);
    RexxObject   *completed();
    void          sendNotification();
    void          error(DirectoryClass *);
    RexxObject   *messageTarget();
    RexxString   *messageName();
    ArrayClass   *arguments();
    RexxObject   *hasError();
    RexxObject   *hasResult();
    RexxObject   *errorCondition();
    RexxObject   *newRexx(RexxObject **, size_t);
    Activity     *getActivity() { return startActivity; }
    RexxObject   *messageCompleted(RexxObject *messageSource);
    RexxObject   *halt(RexxString *description);

    inline bool isActivated()    { return dataFlags[flagMsgActivated]; }
    inline bool isComplete()     { return resultReturned() || raiseError(); }
    inline bool resultReturned() { return dataFlags[flagResultReturned]; }
    inline bool raiseError()     { return dataFlags[flagRaiseError]; }
    inline bool errorReported()  { return dataFlags[flagErrorReported]; }
    inline bool allNotified()    { return dataFlags[flagAllNotified]; }
    inline bool startPending()   { return dataFlags[flagStartPending]; }
    inline void setResultReturned() { dataFlags.set(flagResultReturned); }
    inline void setRaiseError()     { dataFlags.set(flagRaiseError); }
    inline void setErrorReported()  { dataFlags.set(flagErrorReported); }
    inline void setAllNotified()    { dataFlags.set(flagAllNotified); }
    inline void clearStartPending() { dataFlags.reset(flagMsgActivated); dataFlags.reset(flagStartPending); }
    inline void setStartPending()   { setMsgActivated(); dataFlags.set(flagStartPending); }
    inline void setMsgActivated()   { dataFlags.set(flagMsgActivated); }
    void clearCompletion();
    void checkReuse();

    static void createInstance();
    static RexxClass *classInstance;

 protected:

    RexxObject    *receiver;              // Real receiver of message (can change at start time)
    RexxObject    *target;                // Target object specified when created
    RexxString    *message;               // Message to be sent
    RexxClass     *startscope;            // Starting scope for method lookup
    ArrayClass    *args;                  // the message arguments
    RexxObject    *resultObject;          // the message result
    ArrayClass    *interestedParties;     // message objects to be notified
    DirectoryClass *condition;            // any condition object generated by sending this message
    Activity      *startActivity;         // Activity created to run msg
    ArrayClass    *waitingActivities;     // waiting activities list
    FlagSet <MessageFlag, 32> dataFlags;  // flags to control processing
};

#endif

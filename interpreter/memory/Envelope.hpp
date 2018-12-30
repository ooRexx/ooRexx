/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                 Envelope.hpp   */
/*                                                                            */
/* Primitive Envelope Class Definitions                                       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_Envelope
#define Included_Envelope

class SmartBuffer;
class BufferClass;
class MapTable;


class Envelope : public RexxInternalObject
{
  public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    Envelope() { }
    inline Envelope(RESTORETYPE restoreType) { }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void flattenReference(void *, size_t, void *);
    BufferClass *pack(RexxInternalObject *);
    void        puff(BufferClass *, char *, size_t length);
    size_t queryObj(RexxInternalObject *);
    size_t copyBuffer(RexxInternalObject *);
    void   rehash();
    char  *bufferStart();
    void   associateObject(RexxInternalObject *, size_t);
    void   addTable(RexxInternalObject *obj);

    inline SmartBuffer *getBuffer() {return buffer;}
    inline RexxInternalObject  *getReceiver() {return receiver;}
    inline void setReceiver(RexxInternalObject *r) { receiver = r; }
    inline size_t      getCurrentOffset() { return currentOffset; }
    inline MapTable *getDuptable() {return dupTable;}
    inline IdentityTable *getRehashtable() {return rehashTable;}

    size_t      currentOffset;            // current flattening offset

    // default size of the buffer used for flatten operations
    static const size_t DefaultEnvelopeBuffer = (256*1024);
    // default number initial dup table size (allow for a lot of objects)
    static const size_t DefaultDupTableSize = 32 * 1024;

protected:

    RexxInternalObject *home;
    RexxInternalObject *receiver;        // object to receive the message
    MapTable           *dupTable;        // table of duplicates
    IdentityTable      *saveTable;       // table of protected objects created during flattening
    SmartBuffer        *buffer;          // smart buffer wrapper
    IdentityTable      *rehashTable;     // table to rehash
    LiveStack          *flattenStack;    // the flattening stack
};
#endif

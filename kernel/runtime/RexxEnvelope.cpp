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
/* REXX Kernel                                                  RexxEnvelope.c     */
/*                                                                            */
/* Primitive Envelope Class                                                   */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include "RexxCore.h"
#include "StackClass.hpp"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxSmartBuffer.hpp"
#include "ArrayClass.hpp"
#include "RexxEnvelope.hpp"
#include "MethodClass.hpp"

#ifdef AIX
extern void ic_setVirtualFunctions(char *,int);
#endif
extern void *VFTArray[highest_T];      /* table of virtual function tables  */

RexxEnvelope::RexxEnvelope()
/******************************************************************************/
/* Function:  Initialize a REXX envelope object                               */
/******************************************************************************/
{
  ClearObject(this);                   /* just clear and                    */
  this->hashvalue = HASHOREF(this);    /* set the hash                      */
}

void RexxEnvelope::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/*                                                                            */
/*  NOTE: Do not mark flattenStack                                            */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->home);
  memory_mark(this->destination);
  memory_mark(this->receiver);
  memory_mark(this->message);
  memory_mark(this->arguments);
  memory_mark(this->result);
  memory_mark(this->duptable);
  memory_mark(this->savetable);
  memory_mark(this->buffer);
  memory_mark(this->rehashtable);
  memory_mark(this->objectVariables);
  cleanUpMemoryMark

}

void RexxEnvelope::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/*                                                                            */
/*  NOTE: Do not mark flattenStack                                            */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->home);
  memory_mark_general(this->destination);
  memory_mark_general(this->receiver);
  memory_mark_general(this->message);
  memory_mark_general(this->arguments);
  memory_mark_general(this->result);
  memory_mark_general(this->duptable);
  memory_mark_general(this->savetable);
  memory_mark_general(this->buffer);
  memory_mark_general(this->rehashtable);
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral

}

void RexxEnvelope::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxEnvelope)

     flatten_reference(newThis->home, envelope);
     flatten_reference(newThis->destination, envelope);
     flatten_reference(newThis->receiver, envelope);
     flatten_reference(newThis->message, envelope);
     flatten_reference(newThis->arguments, envelope);
     flatten_reference(newThis->result, envelope);
     flatten_reference(newThis->rehashtable, envelope);
     flatten_reference(newThis->objectVariables, envelope);

                                            /* following if test determines if this */
                                            /* envelope is the top level, or if the */
                                            /* envelope is actually part of another */
#ifdef NESTED
     if (envelope == this) {
#endif
                                            /* Top level envelope, no need to send  */
       this->buffer = OREF_NULL;            /* dupTable or smartbuffer, useless on  */
                                            /* other side ...                       */

                                            /* Special self reference so that we get*/
                                            /* Marked (SetLive) correctly during the*/
                                            /* unpacking of the envelope on the     */
                                            /* remote system                        */
                                            /* compute our offset.                  */
       this->duptable = (RexxObjectTable *)((PCHAR)this - envelope->bufferStart());

#ifdef NESTED
     } else {
#endif
                                            /* Part of a greater envelope, we need  */
                                            /* all our buffer information.  but not */
                                            /* the table.                           */
#ifdef NESTED
       flatten_reference(this->buffer, envelope);
       this->duptable = OREF_NULL;
     }
#endif
  cleanUpFlatten
}

RexxObject *RexxEnvelope::unflatten(RexxEnvelope * envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
   return this;                        /* this does nothing                 */
}

void RexxEnvelope::flattenReference(
    RexxObject **newThis,              /* current pointer to flattening obj */
    LONG         newSelf,              /* offset of the flattening object   */
    RexxObject **objRef)               /* object to process                 */
/******************************************************************************/
/* Function: This method does the copy buffer,                                */
/******************************************************************************/
{
 RexxObject *newObj;                   /* new object in buffer              */
 RexxObject *obj = *objRef;            /* get working pointer to object     */
 RexxObject *proxyObj;                 /* proxy of the flattened object     */
 long        referenceOffset;          /* offset of the reference           */
 PCHAR       buffer;                   /* buffer location                   */
 PCHAR       newBuffer;                /* location after a copy operation   */

                                       /* See if object has already been    */
 newObj  = this->queryObj(obj);        /* Flattened/proxied.                */
 if (!newObj) {
   buffer = this->bufferStart();       /* get the entry buffer location     */
                                       /* not known, is this a proxy?       */
                                       /* compute actual off to be updated  */
                                       /* since buffer may have to grow     */
                                       /* we cannot rely on a address       */
   referenceOffset = (ULONG)objRef - (ULONG)buffer;

   if (obj->header & MakeProxyObject) {/* need to make a proxy?             */
     proxyObj = obj->makeProxy(this);  /* Yup. get the proxy.               */
     /* protect this from garbage collection */
     savetable->put(proxyObj, proxyObj);
                                       /* copy proxy into buffer            */
     newObj = this->copyBuffer(proxyObj);
                                       /* associate original object with    */
     this->associateProxy(obj, newObj);/* flattened proxy.                  */
   }
   else {
                                       /* copy ourself/proxy into           */
     newObj = this->copyBuffer(obj);   /*  buffer.                          */
   }
                                       /* Disable CheckSetOREF around       */
                                       /* The flattenStack Push             */
   memoryObject.disableOrefChecks();
                                       /* push the new obj onto start       */
                                       /* We'll send flatten later.         */
   this->flattenStack->fastPush(newObj);
   memoryObject.enableOrefChecks();    /* Enable CheckSet again.            */
   newBuffer = this->bufferStart();    /* get the buffer start position     */
   if (newBuffer != buffer)            /* did the buffer move?              */
     *newThis = (RexxObject *) (newBuffer + newSelf);
                                       /* update reference to new obj       */
   *(RexxObject **)(newBuffer + referenceOffset) = newObj;
 }
 else
   *objRef = newObj;                   /* update reference to newObj offset */
}

RexxEnvelope *RexxEnvelope::pack(
    RexxString *location,              /* name of the destination           */
    RexxObject *receiver,              /* the receiver object               */
    RexxString *message,               /* the message to execute            */
    RexxArray  *arguments)             /* message arguments                 */
/******************************************************************************/
/* Function:  Pack an envelope item                                           */
/******************************************************************************/
{
  RexxObject *flattenObj;              /* flattened object                  */
  RexxObject *newSelf;                 /* the flattened envelope            */
  RexxObject *firstObject;             /* first object to flatten           */
  short behaviourTypeNum;              /* type of flattened behaviour       */

  OrefSet(this, this->destination, location);
  OrefSet(this, this->receiver, receiver);
  OrefSet(this, this->message, message);
  OrefSet(this, this->arguments, arguments);
  /* create a save table to protect any objects (such as proxy */
  /* objects) we create during flattening. */
  OrefSet(this, this->savetable, new_object_table());
  OrefSet(this, this->duptable, new_object_table());
                                       /* mark the hash table as not having */
                                       /*references ..                      */
  SetObjectHasNoReferences(this->duptable->contents);
  OrefSet(this, this->buffer, new_smartbuffer());
                                       /* get our stack, don't use OrefSet */
  this->flattenStack = memoryObject.getFlattenStack();
                                       /* push unique terminator onto stack*/
  this->flattenStack->fastPush(OREF_NULL);

  /* First, put a header into the buffer.  This is necessary because without    */
  /* it, the envelope object would be at 0 offset into the buffer, which is not */
  /* distinguishable from OREF_NULL when the objects are unpacked from buffer.  */
  this->copyBuffer(new_instance());    /* and copy into the buffer          */
  if (this->destination != OREF_NULL)  /* "mailing" an object?              */
    firstObject = this;                /* include the envelope              */
  else
    firstObject = this->receiver;      /* start with the receiver           */
                                       /* copy the first object             */
  this->currentOffset = (LONG)this->copyBuffer(firstObject);
                                       /* point to the copied one           */
  newSelf = (RexxObject *)(this->bufferStart() + this->currentOffset);

                                       /* primitive behaviour?              */
  if (ObjectHasNonPrimitiveBehaviour(newSelf)) {
                                       /* no, we can do regular mr call     */
    behaviourTypeNum = (short)(((RexxBehaviour *)((((ULONG)(newSelf->behaviour)) & ~BEHAVIOUR_NON_PRIMITIVE) + this->bufferStart()))->typenum());
  } else {
                                       /* primitive, typenum is in          */
                                       /* behaviour field                   */
    behaviourTypeNum = (short)((ULONG)(newSelf->behaviour));
  }
  newSelf->flatten(this);              /* start the flatten process.        */

  for (flattenObj = this->flattenStack->fastPop();
       flattenObj != OREF_NULL;
       flattenObj = this->flattenStack->fastPop()) {

                                       /* save the working offset           */
    this->currentOffset = (LONG)flattenObj;
                                       /* compute actual addr of obj        */
    flattenObj = (RexxObject *)((ULONG)flattenObj + this->bufferStart());
                                       /* primitive behaviour?              */
    if (ObjectHasNonPrimitiveBehaviour(flattenObj)) {
                                       /* no, we can do regular mr call     */
      behaviourTypeNum = (short)(((RexxBehaviour *)(((ULONG)(flattenObj->behaviour) & ~BEHAVIOUR_NON_PRIMITIVE) + this->bufferStart()))->typenum());
    } else {
                                       /* primitive, typenum is in          */
                                       /* behaviour field                   */
      behaviourTypeNum = (short)((ULONG)(flattenObj->behaviour));
    }
    flattenObj->flatten(this);         /* let this obj flatten its refs     */
  }
  memoryObject.returnFlattenStack();   /* done with the flatten stack       */
  return this;
}

RexxObject *RexxEnvelope::unpack()
/******************************************************************************/
/* Function:  Unpack an envelope                                              */
/******************************************************************************/
{
  long objsize;
  FILE *objfile;
  char *op;

  /* ******************************************************************************/
  /* ***   Temporary - Just read object from a file for now.                      */
  /* ******************************************************************************/
  printf("Reading object from the file flatten.obj.\n");

  objfile = fopen("flatten.obj","rb"); /* open for binary read              */
  fseek(objfile,0,SEEK_SET);           /* Start at beginning of file.       */
                                       /* Read the buffer size buffer       */
  fread(&objsize, 1, sizeof(objsize), objfile);
                                       /* Create new buffer of objSize      */
  OrefSet(this, this->buffer, (RexxSmartBuffer *)new_buffer(objsize));
                                       /* Starting point of buffer, relative*/
                                       /* to the end of the buffer          */
  op = ((PCHAR)(this->buffer) + ObjectSize(this->buffer)) - objsize;
  fread(op,1,objsize,objfile);         /* Now read in entire flattened obj  */
  fclose(objfile);                     /* close the file                    */

                                       /* puff up flattened buffer/objects  */
  this->puff((RexxBuffer *)this->buffer, op);
  OrefSet(this,this->buffer,OREF_NULL);/* allow the crippled buffer to be   */
                                       /* garbage collected--not safe to use*/

  /* And finally, tell the envelope to send the saved message to the receiver.      */
  /*  NOTE: this->receiver is actaully the original envelope, so we tell the        */
  /*    original envelope to send message to real receiver....                      */

  return this->execute();
}

void RexxEnvelope::puff(
    RexxBuffer *buffer,                /* buffer object to unflatten        */
    PCHAR startPointer)                /* start of buffered data            */
/******************************************************************************/
/* Function:  Puff into an envelope and remove its contents (unflatten the    */
/*            stuff )                                                         */
/******************************************************************************/
{

  char *endPointer;                    /* end of the buffer                 */
  char *bufferPointer;                 /* current pointer within the buffer */
  RexxBehaviour *objBehav;             /* behaviour of current object       */
  long  primitiveTypeNum;              /* primitive behaviour type number   */

  bufferPointer = startPointer;        /* copy the starting point           */
                                       /* point to end of buffer            */
  endPointer = (PCHAR)buffer + ObjectSize(buffer);

  /* Set objoffset to the real address of the new objects.  This tells              */
  /* mark_general to fix the object's refs and set their live flags.                */
  memoryObject.setObjectOffset((long)bufferPointer);
  /* Now traverse the buffer fixing all of the behaviour pointers of the objects.   */
  while (bufferPointer < endPointer) {
                                       /* a non-primitive behaviour         */
                                       /* These are actually in flattened   */
                                       /* storgage.                         */
    if (ObjectHasNonPrimitiveBehaviour(bufferPointer)) {
                                       /* Yes, lets get the behaviour Object*/
      objBehav = (RexxBehaviour *)(((long)(((RexxObject *)bufferPointer)->behaviour) & ~BEHAVIOUR_NON_PRIMITIVE) + ((RexxBuffer *)buffer)->address());
                                       /* Resolve the static behaviour info */
      resolveNonPrimitiveBehaviour(((RexxBehaviour *)objBehav));
                                       /* Set this objects behaviour.       */
      ((RexxObject *)bufferPointer)->behaviour = (RexxBehaviour *)objBehav;
                                       /* get the behaviour's type number   */
      primitiveTypeNum = (long)objBehav->typenum();

    }
    else {
                                       /* originally a primitive;  the      */
                                       /* type number is the behaviour      */
      primitiveTypeNum = (long)((RexxObject *)bufferPointer)->behaviour;
                                       /* was a primitive, stays a primitive*/
      ((RexxObject *)bufferPointer)->behaviour = (RexxBehaviour *)(&pbehav[primitiveTypeNum]);
    }
                                       /* Force fix-up of                   */
                                       /*VirtualFunctionTable,              */
#ifdef AIX
/* The very first thing of an Object is its VFT. It seems, that if this VFT */
/* of an Object is used several times, the AIX-optimizer stores it in a     */
/* new register, as it is assumed, that the VFT-pointer of an Object can    */
/* never change. For tokenized scripts we do not know the VFT of the object */
/* as we do not no what kind of object we have. With the primitiveTypeNum   */
/* we are able to set the VFT-pointer for the object. (The VFT is stored    */
/* in the rexx.img and restored at startup-time).                           */
/* The AIX-optimizer seems to store the VFT to a special register           */
/* so that a change of the pointer has no affect. If the routine is exe-    */
/* cuted in a separate module the optimizer is not able to do so,           */
/* overrides the pointer to the VFT in the tokenized script and uses the    */
/* correct on. This behaviour is only seen in AIX                           */
    ic_setVirtualFunctions(bufferPointer, primitiveTypeNum);
#else
    setVirtualFunctions(bufferPointer, primitiveTypeNum);
#endif
    SetObjectLive(bufferPointer);      /* Then Mark this object as live.    */
                                       /* Mark other referenced objs        */
                                       /* Note that this flavor of          */
                                       /* mark_general should update the    */
                                       /* mark fields in the objects.       */
    ((RexxObject *)bufferPointer)->liveGeneral();
                                       /* Point to next object in image.    */
    bufferPointer += ObjectSize(bufferPointer);
  }
  memoryObject.setObjectOffset(0);     /* all done with the fixups!         */

                                       /* Prepare to reveal the objects in  */
                                       /*the buffer.                        */
                                       /* our receiver object is the inital */
                                       /* envelope.  This also keeps a      */
                                       /* reference to original envelope so */
                                       /* we don't loose it.                */
  OrefSet(this,this->receiver,(OREF)(startPointer + ObjectSize(startPointer)));
                                       /* chop off end of buffer to reveal  */
                                       /* its contents to memory obj        */
//  SetObjectSize(buffer, startPointer - (ULONG)buffer);

  SetObjectSize(buffer, (unsigned long)startPointer - (unsigned long)buffer + ObjectSize(startPointer));
                                        /* HOL: otherwise an object with 20 bytes will be left */

  /* Now we have real objects in real memory.  Time to send unflatten to activate   */
  /* any proxies,                                                                   */
                                       /* the dup table is used by        */
                                       /*proxies to if they have already  */
                                       /*been run, is so they can just    */
                                       /*get their newthis from the table */
                                       /*rather than running again.       */
  OrefSet(this, this->duptable, new_object_table());
  OrefSet(this, this->savetable, new_object_table());

                                       /* move past header to envelope    */
  bufferPointer = startPointer + ObjectSize(startPointer);
  /* Set envelope to the real address of the new objects.  This tells               */
  /* mark_general to send unflatten to run any proxies.                             */
  memoryObject.setEnvelope(this);      /* tell memory to send unflatten     */

  /* Now traverse the buffer running any proxies.                                   */
  while (bufferPointer < endPointer) {
                                       /* Since a GC could happen at anytime*/
                                       /*  we need to check to make sure the*/
                                       /*  we are going now unflatten is    */
                                       /*  still alive, since all who       */
                                       /*  reference it may have already    */
                                       /*  run and gotten the info from it  */
                                       /*  and no longer reference it.      */
    if (ObjectIsLive(bufferPointer))
                                       /* Mark other referenced objs        */
      ((RexxObject *)bufferPointer)->liveGeneral();
                                       /* Note that this flavor of          */
                                       /* mark_general will run any proxies */
                                       /* created by unflatten and fixup    */
                                       /* the refs to them.                 */
                                       /* Point to next object in image.    */
    bufferPointer += ObjectSize(bufferPointer);
  }

                                       /* Tell memory we're done            */
  memoryObject.setEnvelope(OREF_NULL); /* unflattening.                     */

                                       /* Before we run the method we need  */
                                       /* to give the tables a chance to    */
  this->rehash();                      /* rehash...                         */
}

RexxObject *RexxEnvelope::queryObj(
    RexxObject *obj)                   /* object to check                   */
/******************************************************************************/
/* Function:  Check to see if we've already flattened an object               */
/******************************************************************************/
{
   return this->duptable->get(obj);
}

RexxObject *RexxEnvelope::queryProxy(
    RexxObject *obj)                   /* target object needing a proxy     */
/******************************************************************************/
/* Function:  See if we already have a proxy for an object in out proxy table */
/******************************************************************************/
{
   return this->duptable->get(obj);    /* retrieve proxies new address..    */
}

RexxObject *RexxEnvelope::copyBuffer(
    RexxObject *obj)                   /* object to copy                    */
/******************************************************************************/
/* Function:  Copy an object into our flatten buffer                          */
/******************************************************************************/
{
  LONG            objOffset;           /* offset of the buffered object     */
  RexxObject    * newObj;              /* new buffered object               */
                                       /* Copy the object into the buffer   */
  objOffset = this->buffer->copyData((PVOID)obj, ObjectSize(obj));
                                       /* Add object and its new address    */
                                       /* to the dublicate table            */
  newObj = (RexxObject *) (this->buffer->getBuffer()->address() + objOffset);
  this->duptable->addOffset((RexxObject *)objOffset, obj);
                                       /* Is this a non primitive Behav     */
  if (newObj->behaviour->isNonPrimitiveBehaviour()) {
                                       /* Yes, we will actually flatten it. */
   this->flattenReference(&newObj, objOffset, (RexxObject **)&newObj->behaviour);
   newObj->behaviour = (RexxBehaviour *)((long)(newObj->behaviour) | BEHAVIOUR_NON_PRIMITIVE);
  }
  else {
                                       /* we now wipe out the behaviour and */
                                       /* set it to its typenum, we use     */
                                       /* destination behaviours            */
    newObj->behaviour = (RexxBehaviour *)newObj->behaviour->typenum();
  }

                                       /* Make sure we clear out the        */
  SetNewSpace(newObj);                 /* OldSpace bit                      */
  return (RexxObject *)objOffset;      /* and return the new offset         */
}

RexxObject *RexxEnvelope::execute()
/******************************************************************************/
/* Function:  Execute an unflattened "startat" message                        */
/******************************************************************************/
{
                                       /* just issue the sendmessage        */
   return this->receiver->sendMessage(this->message,this->arguments);
}

void  RexxEnvelope::rehash()
/******************************************************************************/
/* Function:  Rehash flattened tables                                         */
/******************************************************************************/
{
  long         i;                      /* loop index                        */
  RexxTable    * index;                /* table to flatten                  */

  if (this->rehashtable != OREF_NULL) {/* tables to rehash here?            */
                                       /* Before we run the method we need  */
                                       /* to give the tables a chance to    */
                                       /* rehash...                         */
    for (i = this->rehashtable->first(); (index = (RexxTable *)this->rehashtable->index(i)) != OREF_NULL; i = this->rehashtable->next(i)) {
      index->reHash();                 /* rehash the table                  */
    }
  }
}

ULONG   RexxEnvelope::queryType()
/******************************************************************************/
/* Function: this method returns the type of envelope we are creating         */
/*  this is to be used during the flattening process by other objects         */
/*  Once we have real location objects, we will query the location object     */
/*  to get this info, for now we assume is for mobile unless the location     */
/*  string is exactly "EA"                                                    */
/******************************************************************************/
{
  if (this->destination == OREF_NULL) {/* no location given?                */
    return METHOD_ENVELOPE;            /* Yes, flattening a program         */
  }
  else {
    return MOBILE_ENVELOPE;            /* nope, this is a mobile packaging  */
  }
}

PCHAR RexxEnvelope::bufferStart()
/******************************************************************************/
/* Return the start of the envelope buffer                                    */
/******************************************************************************/
{
  return this->buffer->getBuffer()->address();
}

void  RexxEnvelope::associateProxy(
    RexxObject *o,                     /* original object                   */
    RexxObject *p)                     /* new proxy object                  */
/******************************************************************************/
/* Function:  Map an object to a flattened proxy object                       */
/******************************************************************************/
{
  this->duptable->addOffset(p,o);      /* just add to the offset table      */
}

void RexxEnvelope::addTable(
    RexxObject *obj)                   /* table object to rehash            */
/******************************************************************************/
/* Function:  Add an object to the rehash table for later processing          */
/******************************************************************************/
{
                                       /*  the following table will be used */
                                       /* by the table_unflatten method.    */
                                       /*                                   */
                                       /* Every table that gets unflattened */
                                       /* place itself in this table.  Once */
                                       /* every object has been unflattened */
                                       /* we traverse this table and allow  */
                                       /* the hashtables to re-hash their   */
                                       /* since some hash value may have    */
                                       /* change                            */
  if (this->rehashtable == OREF_NULL)  /* first table added?                */
                                       /* create the table now              */
    OrefSet(this, this->rehashtable, new_object_table());
                                       /* use put to make sure we only get  */
                                       /* a single version of each table    */
  this->rehashtable->put(TheNilObject, obj);
}

void  RexxEnvelope::addProxy(
    RexxObject *o,                     /* original object                   */
    RexxObject *p)                     /* proxy object                      */
/******************************************************************************/
/* Function:  Add a proxy object to the dup table for flatten processing      */
/******************************************************************************/
{
  this->duptable->add(p,o);
}

void *RexxEnvelope::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* Get new object                    */
  newObject = new_object(sizeof(RexxEnvelope));
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheEnvelopeBehaviour);
  return newObject;                    /* return the new object             */
}


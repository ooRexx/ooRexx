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
/* REXX Kernel                                                  RexxSOMCode.c     */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxSOMCode.hpp"
#ifdef SOM
  #include <somcls.xh>
#ifdef DSOM
  #include <somd.xh>
#endif
  #include "orxsminf.xh"               /* OREXX/SOM interface class       */
  #include "orxsargl.xh"
#endif
#include "SOMUtilities.h"                   /* SOM utilites routines.          */
#include <ctype.h>


extern RexxObject *ProcessLocalServer;

#ifdef SOM
SOMObject *get_SomObject(OREF receiver);

/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
SOMObject *get_SomObject(RexxObject *receiver)
{
  RexxObject *somoref;
  RexxClass *oclass;
  RexxInteger *sclass;
  SOMClass *objclass;
  SOMObject *somobj;

                                       /* get my SOM object counterpart     */
  somobj = (SOMObject *)receiver->realSOMObject();
  if (NULL == somobj) {
                                       /* see if the local server knows     */
                                       /*about me                           */
    somoref = ProcessLocalServer->sendMessage(OREF_SOMLOOK,receiver);

                                       /* if object returned is the special */
                                       /* value .nil, there is no SOM object*/
                                       /* and we must create one            */
    if (somoref == TheNilObject) {
                                       /* get class name for this object    */
      oclass = receiver->classObject();
                                       /* get SOM class this class represen */
      sclass = oclass->somClass;
                                       /* convert object to class pointer   */
      objclass = (SOMClass *)sclass->value;
      somobj = make_somproxy(receiver,objclass);

                                       /* any other value is address of an  */
                                       /* existing SOM object               */
    } else {
      receiver->initProxy((RexxInteger *)somoref);
      somobj = (SOMObject *)((RexxInteger *)somoref)->value;
    }

  }
  return somobj;
}
#endif

RexxSOMCode::RexxSOMCode(BOOL generic)
/******************************************************************************/
/* Arguments:  none.                                                          */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/*                                                                            */
/******************************************************************************/
{
    ClearObject(this);                 /* clear all fields first.         */
    this->hashvalue = HASHOREF(this);
    if (generic)                       /* is this to be a generic method  */
                                       /* indicate generic (unresolved)   */
      this->SOMflags |= GENERIC_FLAG;
}

void RexxSOMCode::uninit()
/******************************************************************************/
/* Arguments:  none.                                                          */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/*                                                                            */
/******************************************************************************/
{
#ifdef SOM
    if (this->methodInfo != NULL) {    /* is there method Information?    */
      (this->methodInfo)->somFree();   /* free the SOM method Information */
      this->methodInfo = NULL;
    }
    if (this->argList != NULL) {       /* is there method Information?      */
      (this->argList)->somFree();      /* free the SOM method Information   */
      this->argList = NULL;
    }
                                       /* CLear method Class info         */
    this->methodClass = NULL;

    if (this->msgId)      {            /* a message somId ?               */
      SOMFree(this->msgId);            /* free the msgId                  */
      this->msgId = NULL;
    }
#endif
                                       /* turn off the resolved flag      */
    this->SOMflags &= ~SOMRESOLVED_FLAG;
}

#ifdef SOM
void RexxSOMCode::setResolvedInfo(OrxSOMMethodInformation *methInfo,
                                   SOMClass *methClass,
                                   somId *msgId)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  this->SOMflags |= SOMRESOLVED_FLAG;  /* has this method been resolved   */
  this->methodInfo = methInfo;
  this->methodClass =  methClass;
  this->msgId = *msgId;
                                       /* and create an Argumentlist obj    */
  this->argList = new OrxSOMArgumentList;
}

#endif

void  RexxSOMCode::resolve(RexxObject *receiver,
                            RexxString *msgname,
                            RexxClass  *mscope)
/******************************************************************************/
/* Arguments:  receiver and messagname                                        */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
#ifdef SOM
  RexxInteger *mclass;
  SOMClass *methclass;
  somId    msgId;
                                       /* the scope for this method was passed */
                                       /* to us, this is a OREXX class object  */
                                       /* so wee need to determine which SOM   */
                                       /* class level this method is introduced*/
                                       /* which is simply the scope SOM class  */
  mclass = mscope->somClass;
                                       /* conver OREXX object to actual SOMClas*/
  methclass = (SOMClass *)mclass->value;
                                       /* Resolve the method information, */
                                       /* and retain the methodInformation*/
  this->setResolvedInfo(
       som_resolveMethodInfo(get_SomObject(receiver), methclass, receiver, msgname, &msgId),
       methclass,                      /* set SOM class information       */
       &msgId);                        /* messageId.                      */
                                       /* make sure we drive the uninit   */
                                       /*  method for this object when its*/
                                       /*  Garbage Collected.             */
  this->hasUninit();
                                       /* retain the SOm class this method*/
                                       /* is defined in.                  */
#endif
}

RexxObject *RexxSOMCode::run(
     RexxObject *receiver,             /* object receiving the message      */
     RexxString *msgname,              /* message to be run                 */
     RexxClass  *mscope,               /* method scope                      */
     int   count,                      /* count of arguments                */
     RexxObject **arguments)           /* arguments to the method           */
/******************************************************************************/
/* Arguments:  Activity, receiver, message name, arguments                    */
/*                                                                            */
/*  Returned:  Result                                                         */
/******************************************************************************/
{
#ifdef SOM
  RexxObject *rc;
  RexxMessage *messageObj;             /* Message Obj to queue up.          */
  RexxArray *argList;                  /* Array of Arguments.               */
  int  i;
  RexxObject *receiverServer;          /* object representing the server    */
                                       /*  for this SOM object.             */
  SOMObject *somReceiver;

                                       /* Get this somObjects Server        */

  receiverServer = receiver->server();

                                       /* is the local server receiver's    */
                                       /* server?                           */
  if (receiverServer == ProcessLocalServer) {
                                       /* have we resolved this method?     */
                                       /* method info must be resolved      */
                                       /* on SOMObjects process.            */
    if (!this->isResolved()) {
                                       /* no, do it now.                    */
      this->resolve(receiver, msgname, mscope);
    }
                                       /* Retrieve actual SOM object        */
    somReceiver = get_SomObject(receiver);
                                       /* Now run the method.               */
                                       /* actually invoke the method on     */
                                       /* my SOM object                     */
                                       /* Sending to a DSOM object?         */
#ifdef DSOM
    if (somReceiver->somIsA(_SOMDObject))
                                       /* send via DSOm route.              */
      rc = dsom_send((SOMDObject *)somReceiver, this->methodClass, receiver,msgname,count,arguments, this->methodInfo, this->msgId);
    else
#endif
      rc = som_send(somReceiver, this->methodClass, receiver,msgname,count,arguments, this->methodInfo, this->msgId, this->argList);
  }
  else {
                                       /* not our server, so not our        */
                                       /* process Switch to server's        */
                                       /* Get array for arguments           */
    argList = (RexxArray *)save(new_array(count));
                                       /* build up argument array w/ arg    */
    for (i=1; i <= count ; i++ ) {
      argList->put(arguments[i - 1], i);
    }
                                       /* Send the message to the server    */
    rc = receiverServer->sendMessage(OREF_SEND, receiver, msgname, argList);
  }
  return rc;                           /* return result                     */
#else
  return TheNilObject;
#endif

}

void *RexxSOMCode::operator new(size_t size)
/******************************************************************************/
/* Arguments:  nothing                                                        */
/*                                                                            */
/*  Returned:  New method                                                     */
/******************************************************************************/
{
  RexxObject *newMethod;

  newMethod = new_object(size);        /* Get new object                    */
                                       /* Give new object method behaviour  */
  OrefSet(newMethod, newMethod->behaviour, TheSomCodeBehaviour);
  return newMethod;                    /* return the new method             */
}


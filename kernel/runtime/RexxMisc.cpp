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
/* REXX Kernel                                                  RexxMisc.c    */
/*                                                                            */
/* Primitive Corral  Class  - This is a general purpose class, we never expect*/
/*  to create instances of this class. Its  only purpose is to have a place   */
/*  to hang/define kernel methods to be use by various ORX OBJECT classes,    */
/*  needed by normal OREXX kernel/core stuff.                                 */
/*                                                                            */
/******************************************************************************/

#define INCL_WINWORKPLACE
#define INCL_PM

#include "RexxCore.h"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "StringClass.hpp"
#include "RexxMisc.hpp"
#ifdef SOM
  #include <somcls.xh>
  #include <somcm.xh>
  #include "dlfcls.h"
 #ifdef DSOM
  #include <somd.xh>
  #include <somtcnst.xh>

  #include <wpobject.xh>
  #include <wpdesk.xh>
  #include <wpabs.xh>
  #include <wpfolder.xh>
  #include <wptrans.xh>
  #include <wpmet.xh>
  #include <wpclrpal.xh>
  #include <wpdataf.xh>
  #include <wpclsmgr.xh>
  #include <wpdserv.xh>
 #endif
#endif

#include "SOMUtilities.h"
                                       /* Since self will ALWAYS be OBJECT  */

enum { STOP, START };

#ifdef DSOM
WPClassManager *vWPClassManagerObject;
#endif

#ifdef SOM
extern  RexxObject *ProcessLocalServer;
#endif
extern RexxInteger *ProcessName;
extern SEV  RexxServerWait;
extern BOOL ProcessTerminating;

void SysRunProgram(PVOID arguments);   /* system dependent program startup  */

RexxObject *RexxSOMServer::initDSom()
/******************************************************************************/
/* Arguments:  none                                                           */
/******************************************************************************/
{
#ifdef DSOM
  Environment *ev;                     /* create environment struct         */
  RexxObject  *proxySOMD;
  RexxDirectory  *somdMethods ;

  ev = SOM_CreateLocalEnvironment();
  SOMD_Init(ev);                       /* Initialize the DSOM env           */
  SOM_DestroyLocalEnvironment(ev);
                                       /* Create proxy object               */
  proxySOMD = save(ProcessLocalServer->sendMessage(OREF_MAKE_PROXY, new_integer((LONG)SOMD_ObjectMgr)));
                                       /* Get set method methods we need to */
                                       /* add to the SOMD_ObjectMgr         */
  somdMethods = (RexxDirectory *)TheEnvironment->at(new_cstring(CHAR_SOMDOBJECTMGR_METHODS));
                                       /* Now add then to the proxy.        */
  proxySOMD->defMethods(somdMethods);
  discard(hold(proxySOMD));
  return proxySOMD;                    /* return the DSOM obj mgr         */
#else
  return TheNullPointer;
#endif
}

RexxObject *RexxSOMServer::initDSomWPS()
/******************************************************************************/
/* Arguments:  none                                                           */
/******************************************************************************/
{
  RexxObject *objMgr = OREF_NULL;
#ifdef DSOM_WPS
  APIRET     rc;
  BOOL       serverUp;

                                       /* Check if the DSOM daemon active.  */
                                       /* if not, we need to start it       */
                                       /*before we can continue.            */
  if( WinIsSOMDDReady() )
  {
     rc = WinRestartSOMDD( START );
     if( rc )
     {
//      report_exception();
     }
  }

                                       /* Check if the Workplace Shell DSOM */
                                       /*server is up and running,          */
                                       /* if not, we need to start it       */
                                       /*before we can continue.            */
  if( WinIsWPDServerReady() )
  {
     rc = WinRestartWPDServer( START );
     if( rc )
     {
//      report_exception();
     }
  }


                                       /* Create our local DSOM environment.*/
  objMgr = this->initDSom();


                                       /* Merge the Workplace Shell Class   */
                                       /*manager with the SOM Class manager.*/
  vWPClassManagerObject = new WPClassManager;
  SOMClassMgrObject->somMergeInto(vWPClassManagerObject );


                                       /* Initialize all the Workplace      */
                                       /*Shell Classes we are interested in.*/

  WPObjectNewClass( 1, 1 );
  WPTransientNewClass( 1, 1 );
  WPAbstractNewClass( 1, 1 );
  WPFileSystemNewClass( 1, 1 );
  WPDataFileNewClass( 1, 1 );
  WPFolderNewClass( 1, 1 );
  WPDesktopNewClass( 1, 1 );

  M_WPObjectNewClass( 0, 0 );
  M_WPTransientNewClass( 0, 0 );
  M_WPAbstractNewClass( 0, 0 );
  M_WPFileSystemNewClass( 0, 0 );
  M_WPDataFileNewClass( 0, 0 );
  M_WPFolderNewClass( 0, 0 );
  M_WPDesktopNewClass( 0, 0 );

//  WPDServerNewClass(1, 1);
  SOMDServerNewClass(0, 0);
#endif
  return objMgr;
}

RexxObject *RexxServer::messageWait()
/******************************************************************************/
/* Function:  This method NEVER returns.  Is sits in a loop waiting for       */
/*   the RexxServerWait Semaphore to be posted by the servers sender object,  */
/*   This indicates a message is queued for this server, so tell the activity */
/*   to run all queued messages, then wait on the semaphore again.            */
/******************************************************************************/
{
  RexxActivity *myActivity;
  myActivity = CurrentActivity;        /* Remember Activity running on.     */
                                       /* Do until terminating process.     */
  while (!ProcessTerminating) {
                                       /* Run any messgae for this process. */
    myActivity->startMessages();
    ReleaseKernelAccess(myActivity);   /* Give up access to kernel.         */
    EVWAIT(RexxServerWait);            /* Now wait for more queued messages */
    RequestKernelAccess(myActivity);   /* Msg Queued,  get acces to kernel  */
    EVSET(RexxServerWait);             /* Clear the ServerWait Sem          */
  }
  TheActivityClass->killMessageList(ProcessName);
  return OREF_NULL;                    /* return stmt, keep compiler happy  */
}

RexxArray *RexxSender::getPid()
/******************************************************************************/
/* Function:  return the processInfo   for current activity                   */
/*   This method returns a 2 element array.                                   */
/*     Element 1 is the processName for this process                          */
/*     Element 2 is an Integer object, value is the RexxServerWait Semaphore  */
/*                                                                            */
/******************************************************************************/
{
#ifdef SOM
#ifdef POSIX_THREADS
  return new_array2(ProcessName, new_integer((long)&RexxServerWait));
#else
  return new_array2(ProcessName, new_integer((long)RexxServerWait));
#endif
#else
  return TheNullArray;
#endif
}

RexxObject *RexxSender::sendMessage(RexxArray *pid, RexxMessage *messageObj)
/******************************************************************************/
/* Function:  Queu up the message object to be run on correct process         */
/******************************************************************************/
{
#ifdef SOM
 RexxObject *serverProcessName;
 SEV  serverSem;


                                       /* retrieve processname from ProcInfo*/
 serverProcessName = pid->get(1);
                                       /* Are we currently on the right     */
                                       /*process?                           */
 if (ProcessName == serverProcessName) {
                                       /* Yes, then start the message now.  */
   messageObj->start(OREF_NULL);
 }
 else {
                                       /* Nope, queue it up on correct      */
                                       /*process if still around            */
   if (TheActivityClass->addMessageObject(messageObj, serverProcessName)) {
                                       /* Get serverWait Semaphore          */
#ifdef POSIX_THREADS
// comment out for purposes of compilation
//   serverSem = (SEV *)((RexxInteger *)pid->get(2))->value;
#else
     serverSem = (SEV)((RexxInteger *)pid->get(2))->value;
#endif
     EVOPEN(serverSem);                /* open the Semahphore               */
     EVPOST(serverSem);                /* Post it, to wake up server.       */
     EVCL(serverSem);                  /* Done with Sem, Close it.          */
   }
   else
                                       /* Process terminated.  report error */
     report_exception1(Error_Execution_nosomobj, messageObj->receiver);

 }
#endif
 return OREF_NULL;
}


RexxObject *RexxSOMDServer::getClassObj(RexxString *id)
/******************************************************************************/
/* Function:  get the SOMClass proxy, from the DSOM server                    */
/******************************************************************************/
{
#ifdef DSOM
 SOMDServer  *dsomServer;
 SOMClass    *classObj;
 char        *classId;
 RexxActivity *myActivity;
 Environment *ev = somGetGlobalEnvironment();
                                       /* Get the actual SOMD Server for    */
                                       /*class                              */
 dsomServer = (SOMDServer *)this->realSOMObject();
 classId = id->stringData;             /* get classId as string value.      */
 myActivity = CurrentActivity;
                                       /* Let kernel do other things while  */
                                       /* we go through DSOM to get Class   */
 ReleaseKernelAccess(myActivity);
 classObj = dsomServer->somdGetClassObj(ev, classId);
 RequestKernelAccess(myActivity);
                                       /* Create OREXX proxy for DSOM Proxy */
 return ProcessLocalServer->sendMessage(OREF_MAKE_PROXY, new_pointer((long)classObj));
#else
 return TheNullPointer;
#endif
}

RexxObject *RexxSOMDServer::createObj(RexxString *id)
/******************************************************************************/
/* Function:  get the SOMClass proxy, from the DSOM server                    */
/******************************************************************************/
{
#ifdef DSOM
 SOMDServer  *dsomServer;
 SOMObject   *obj;
 char        *classId;
 RexxActivity *myActivity;
 Environment *ev = somGetGlobalEnvironment();
                                       /* Get the actual SOMD Server for    */
                                       /*class                              */
 dsomServer = (SOMDServer *)this->realSOMObject();
 classId = id->stringData;             /* get classId as string value.      */
 myActivity = CurrentActivity;
                                       /* Let kernel do other things while  */
                                       /* we go through DSOM to get Class   */
 ReleaseKernelAccess(myActivity);
 obj = dsomServer->somdCreateObj(ev, classId, NULL);
 RequestKernelAccess(myActivity);
                                       /* Create OREXX proxy for DSOM Proxy */
 return ProcessLocalServer->sendMessage(OREF_MAKE_PROXY, new_pointer((long)obj));
#else
 return TheNullPointer;
#endif
}

RexxObject *RexxSOMDServer::deleteObj(RexxSOMProxy *proxyObj)
/******************************************************************************/
/* Function:  get the SOMClass proxy, from the DSOM server                    */
/******************************************************************************/
{
#ifdef DSOM
 SOMDServer   *dsomServer;
 SOMDObject   *obj;
 RexxActivity *myActivity;
 Environment  *ev = somGetGlobalEnvironment();
                                       /* Get the actual SOMD Server for    */
                                       /*class                              */
 dsomServer = (SOMDServer *)this->realSOMObject();
 obj        = (SOMDObject *)proxyObj->realSOMObject();
 myActivity = CurrentActivity;
                                       /* Let kernel do other things while  */
                                       /* we go through DSOM to get Class   */
 ReleaseKernelAccess(myActivity);
 dsomServer->somdDeleteObj(ev, obj);
 RequestKernelAccess(myActivity);
                                       /* Create OREXX proxy for DSOM Proxy */
#endif
 return OREF_NULL;
}

RexxObject *RexxSOMDObjectMgr::enhanceServer(RexxSOMProxy *somdServer, RexxDirectory *methods)
/******************************************************************************/
/* Function:  add Orexx overrides to a somd Server.                           */
/******************************************************************************/
{
                                       /* Define all RexxSOMDServer methods */
                                       /*on proxy                           */
  somdServer->defMethods(methods);
  return OREF_NULL;
}

RexxDirectory *RexxLocal::local()
/******************************************************************************/
/* Function:  Return the current activation's local environment pointer       */
/******************************************************************************/
{
                                       /* just return the current local     */
                                       /* environment                       */
  return CurrentActivity->local;
}

RexxObject *RexxLocal::runProgram(
  RexxInteger *arguments)              /* system specific arguments         */
/******************************************************************************/
/* Function:  Bootstrap the process of running a REXX program                 */
/******************************************************************************/
{
  PVOID    argument_block;             /* system dependent argument block   */

                                       /* get the argument pointer          */
  argument_block = (PVOID)arguments->value;
  SysRunProgram(argument_block);       /* go run the program                */
  return OREF_NULL;                    /* always returns null               */
}

RexxObject *RexxLocal::callProgram(
  RexxObject **arguments,              /* call program arguments            */
  size_t       argCount)               /* number of arguments               */
/******************************************************************************/
/* Function:  Call a program through the RexxCallProgram interface            */
/******************************************************************************/
{
  RexxMethod *routine;                 /* method to call                    */
  RexxString *filename;                /* file to call                      */
  RexxObject *result;                  /* call result                       */

  result = OREF_NULL;
                                       /* go resolve the name               */
  filename = SysResolveProgramName((RexxString *)arguments[0], OREF_NULL);
  if (filename != OREF_NULL) {         /* found something?                  */
                                       /* try to restore saved image        */
    routine = (RexxMethod *)SysRestoreProgram(filename);
    if (routine == OREF_NULL) {        /* unable to restore?                */
                                       /* go translate the image            */
      routine = TheMethodClass->newFile(filename);
                                       /* go save this method               */
      SysSaveProgram(filename, routine);
    }
    if (routine != OREF_NULL)          /* Do we have a method???            */
                                       /* run as a call                     */
      result = ((RexxObject *)CurrentActivity)->shriekRun(routine, OREF_COMMAND, OREF_INITIALADDRESS, arguments + 1, argCount - 1);
  }
  else
    report_exception1(Error_Routine_not_found_name, arguments[0]);
                                       /* run and get the result            */
  return result;
}

RexxObject *RexxLocal::callString(
    RexxObject **arguments,              /* call program arguments            */
    size_t       argCount)               /* number of arguments               */
/******************************************************************************/
/* Function:  Call a program through the RexxCallString  interface            */
/******************************************************************************/
{
  RexxMethod *method;                  /* method to call                    */
  RexxObject *result;                  /* call result                       */

                                       /* go translate the image            */
  method = TheMethodClass->newRexxCode(OREF_COMMAND, arguments[0], IntegerOne);
                                       /* run and get the result            */
  result = ((RexxObject *)CurrentActivity)->shriekRun(method, OREF_COMMAND, OREF_INITIALADDRESS, arguments + 1, argCount - 1);
  return result;
}

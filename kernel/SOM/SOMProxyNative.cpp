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
/* Rexx Base                                                    obdserv.c     */
/*                                                                            */
/* Rexx Domain Server                                                         */
/*                                                                            */
/******************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "RexxNativeAPI.h"                    /* Method macros */
#ifdef SOM
  #include <somcm.xh>
  #include "orxsminf.xh"
  #include "orxsargl.xh"
#endif //SOM

extern RexxObject * ProcessLocalServer;/* local server object reference     */

#ifdef SOM
#define SOMVA_LIST_MAX 1000

SOMObject *make_somproxy (OREF oryxobj, SOMClass *sclass);
extern "C" {
void _System  initializeOrxSom(integer4 majorVersion, integer4 minorVersion);
}

RexxMethod1 (void, server_init_local, OSELF, self)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  ProcessLocalServer = (RexxObject *)self;
}


RexxMethod0 (long, server_c_sominit)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  somId dlfId;

  somEnvironmentNew();                 /* Initialize the SOM environment    */
  dlfId = somIdFromString("DLFNumber");
                                       /* lookup/Load the DLF Classes       */
//  SOMClassMgrObject->somFindClsInFile(dlfId, 0, 0, "ORXSOM.DLL");
                                       /* Make sure w initiali ORXSOM.      */
                                       /* NOTE: followin made necessary     */
                                       /* because of problems on 2.1 with   */
                                       /* SOMInitModule                     */
  initializeOrxSom(0,0);

  SOMFree(dlfId);                      /* DOne with DLFNumber id.           */
  return (long)SOMClassMgrObject;      /* Return pointer to SOMClassMgrObjec*/
}

RexxMethod2 (long, server_somproxy, REXXOBJECT, oryxobj, long, classObj)
/******************************************************************************/
/* Arguments:  Oryx object, SOM class object id                               */
/*                                                                            */
/*  Returned:  Proxy object id                                                */
/******************************************************************************/
{
  return (long)make_somproxy((OREF) oryxobj,(SOMClass *)classObj);
}

RexxMethod4 (REXXOBJECT, server_findsomclass, OSELF, self, CSTRING, className, long, majorVersion, long, minorVersion)
/******************************************************************************/
/* Arguments:  Class name, Major Version, minor Verison                       */
/*                                                                            */
/*  Returned:  SOM Class                                                      */
/******************************************************************************/
{
  SOMClass *somClass;
  somId     somClassId;

  somClassId = somIdFromString(className);
  somClass = SOMClassMgrObject->somFindClass(somClassId, majorVersion, minorVersion);
  SOMFree(somClassId);
                                       /* return the REXX proxied object    */
  return RexxSend1(self,CHAR_MAKE_PROXY,RexxInteger((long)somClass));
}

RexxMethod1 (long, server_somclass, long, somobj)
/******************************************************************************/
/* Arguments:  Object id                                                      */
/*                                                                            */
/*  Returned:  Class id                                                       */
/******************************************************************************/
{
  return (long)SOM_GetClass(((SOMObject *)somobj));  /* extra () for macro bug */
}

RexxMethod1 (long, server_somparent, long, somclass)
/******************************************************************************/
/* Arguments:  Class id                                                       */
/*                                                                            */
/*  Returned:  Parent class id                                                */
/******************************************************************************/
{
  return (long)((SOMClass *)somclass)->somGetParent();
}

RexxMethod1 (CSTRING, server_somname, long, somclass)
/******************************************************************************/
/* Arguments:  Class id                                                      */
/*                                                                            */
/*  Returned:  Class name                                                     */
/******************************************************************************/
{
  return ((SOMClass *)somclass)->somGetName();
}

RexxMethod1 (void, server_somtrace, int, level)
/******************************************************************************/
/* Arguments:  Trace level                                                    */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  SOM_TraceLevel = level;
}

#else                                  // not SOM, define dummy  methods

RexxMethod1 (void, server_init_local, OSELF, self)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  ProcessLocalServer = (RexxObject *)self;
}

RexxMethod0 (long, server_c_sominit)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  return 0;
}

RexxMethod2 (long, server_somproxy, REXXOBJECT, oryxobj, long, classObj)
/******************************************************************************/
/* Arguments:  Oryx object, SOM class object id                               */
/*                                                                            */
/*  Returned:  Proxy object id                                                */
/******************************************************************************/
{
  return 0L;
}

RexxMethod4 (REXXOBJECT, server_findsomclass, OSELF, self, CSTRING, className, long, majorVersion, long, minorVersion)
/******************************************************************************/
/* Arguments:  Class name, Major Version, minor Verison                       */
/*                                                                            */
/*  Returned:  SOM Class                                                      */
/******************************************************************************/
{
  return RexxInteger(0L);
}

RexxMethod1 (long, server_somclass, long, somobj)
/******************************************************************************/
/* Arguments:  Object id                                                      */
/*                                                                            */
/*  Returned:  Class id                                                       */
/******************************************************************************/
{
  return 0L;
}

RexxMethod1 (long, server_somparent, long, somclass)
/******************************************************************************/
/* Arguments:  Class id                                                       */
/*                                                                            */
/*  Returned:  Parent class id                                                */
/******************************************************************************/
{
  return 0L;
}

RexxMethod1 (CSTRING, server_somname, long, somclass)
/******************************************************************************/
/* Arguments:  Class id                                                      */
/*                                                                            */
/*  Returned:  Class name                                                     */
/******************************************************************************/
{
  return "";
}

RexxMethod1 (void, server_somtrace, int, level)
/******************************************************************************/
/* Arguments:  Trace level                                                    */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  ;
}

#endif

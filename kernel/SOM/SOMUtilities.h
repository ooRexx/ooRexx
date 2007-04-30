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
/* REXX Kernel SOM                                              somutil.h     */
/*                                                                            */
/* SOM Utility Function Prototypes                                            */
/*                                                                            */
/******************************************************************************/
#ifndef SOMUTIL_INCLUDED
#define SOMUTIL_INCLUDED

#ifdef SOM
#define SOMVA_LIST_MAX 1000

  SOMObject *make_somproxy (RexxObject *oryxobj, SOMClass *sclass);
  OrxSOMMethodInformation *som_resolveMethodInfo (SOMObject *somobj,
                                                 SOMClass  *classobj,
                                                 RexxObject *oproxy,
                                                 RexxObject *msgname,
                                                 somId *msgId);
  RexxObject *som_send (SOMObject *somobj,
                 SOMClass  *classobj,
                 RexxObject *oproxy,
                 RexxObject *msgname,
                 long count,
                 RexxObject **args,
                 OrxSOMMethodInformation *methInfo,
                 somId msgId,
                 OrxSOMArgumentList *argList);

  void oryxSomClassDispatchX(SOMObject *somSelf,
                              SOMClass *classObj,
                              void **retVal,
                              somId methodId,
                              va_list *ap);

#ifdef DSOM
  RexxObject *dsom_send (SOMDObject *somobj,
                 SOMClass  *classobj,
                 RexxObject *oproxy,
                 RexxObject *msgname,
                 long count,
                 RexxObject **args,
                 OrxSOMMethodInformation *methInfo,
                 somId msgId);

#endif

#else
                                       /* non-SOM case, routines are just */
                                       /*  macros and return nothing/NIL  */
#define som_resolveMethodInfo(s,c,p,m,g)  NULL
#define som_send(s,c,p,m,a,i,g,l)         OREF_NIL
#define make_somproxy(o,s)                OREF_NULL
#endif

#endif

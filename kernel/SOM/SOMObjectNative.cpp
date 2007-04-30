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
/* Rexx Interface                                               oisom.c       */
/*                                                                            */
/* SOM Client                                                                 */
/*                                                                            */
/******************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "RexxCore.h"
#include "RexxNativeAPI.h"

#ifdef SOM
  #include "orxsminf.xh"
  #include <somcls.xh>
  OREF resolve_proxy(SOMObject *SOMobject);
#include "orxsom.h"                  /* SOM client declarations */

extern "C" {
void SOMLINK oryx_class_dispatch2 (SOMObject *somSelf, IN SOMClass *classobj,
                    OUT somToken *resultp,
                    IN somId msgid, IN va_list ap);
void SOMLINK oryx_dispatch2 (SOMObject *somSelf, OUT void **resultp,
                    OUT somId msgid, va_list ap);
}
extern OREF ProcessLocalServer;        /* local server object reference     */
static BOOL doneinit = FALSE;

int RexxSomInitialize (SOMObject *envdict)
/******************************************************************************/
/* Arguments:  Global environment SOM dictionary                              */
/*                                                                            */
/*  Returned:  1 if initialized, 0 if already initialized, -1 if error        */
/******************************************************************************/
{
  int rc;
  if (RexxInitialize()) {
    doneinit = TRUE;
                                       /* initialize the SOM environment*/
    RexxSendMessage(ProcessLocalServer, CHAR_SOMINITIALIZE, OREF_NULL, "vl", NULL, FALSE);
    rc = 1;
  }
  else {
    rc = 0;
  }
  return rc;
}

SOMObject *RexxSomFreeObject (SOMObject *object)
/******************************************************************************/
/* Arguments:  SOM object reference                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  if (doneinit) {
    RexxSendMessage(ProcessLocalServer, "FREE", OREF_NULL, "vO", NULL, object);
    return NULL;
  } else {
    return NULL;
  }
}


char *type_char (
  TypeCode type,                       /* base SOM type                     */
  char *cp)                            /* type info output location         */
/******************************************************************************/
/* Function:  Convert a SOM type code into a REXX interface type code.        */
/******************************************************************************/
{
  Environment ev;                      /* SOM environment block             */
  TCKind kind, parmkind;
  long parms, i;
  ULONG size;
  any parm;

  SOM_InitEnvironment(&ev);
  kind = TypeCode_kind(type,&ev);

  switch (kind) {
    case tk_void:                      /* void                              */
      *cp++ = 'v';
      break;
    case tk_short:                     /* SHORT                             */
      *cp++ = 's';
      break;
    case tk_long:                      /* LONG                              */
      *cp++ = 'l';
      break;
    case tk_ushort:                    /* USHORT                            */
      *cp++ = 'h';
      break;
    case tk_ulong:                     /* ULONG                             */
      *cp++ = 'g';
      break;
    case tk_float:                     /* FLOAT                             */
      *cp++ = 'f';
      break;
    case tk_double:                    /* DOUBLE                            */
      *cp++ = 'd';
      break;
    case tk_boolean:                   /* BOOLEAN                           */
      *cp++ = 'b';
      break;
    case tk_char:                      /* CHAR                              */
      *cp++ = 'c';
      break;
    case tk_octet:                     /* BYTE                              */
      *cp++ = 'c';
      break;
    case tk_objref:                    /* SOM OBJECT                        */
      *cp++ = 'O';
      break;
    case tk_string:                    /* ASCII_Z string                    */
      *cp++ = 'Z';
      break;
    case tk_pointer:                   /* POINTER                           */
      *cp++ = 'p';
      break;
    case tk_foreign:
      if ((parms = TypeCode_param_count(type,&ev)) == 3) {
        parm = TypeCode_parameter(type,&ev,0);
        if (TypeCode_kind(parm._type,&ev) == tk_string &&
            strcmp(*(string *)parm._value,"somId") == 0) {
          *cp++ = 'n';
          break;
        } /*if*/
      } /*if*/
      fprintf(stderr,"Unsupported SOM foreign type kind:\n");
      fprintf(stderr,"tk_foreign: number of parameters = %i\n",parms);
      for (i = 0; i < parms; i++) {
        parm = TypeCode_parameter(type,&ev,i);
        parmkind = TypeCode_kind(parm._type,&ev);
        if (parmkind == tk_string)
          fprintf(stderr,"tk_foreign: parm %l string: %s\n",i,*(string *)parm._value);
        else if (parmkind == tk_long)
          fprintf(stderr,"tk_foreign: parm %l long: %l\n",i,*(long *)parm._value);
        else
          fprintf(stderr,"tk_foreign: parm %l unexpected type %lu!\n",i,parmkind);
      } /* for */
      exit(-1);
      break;
    default:
      fprintf(stderr,"Unsupported SOM type kind %lu\n",kind);
      TypeCode_print(type,&ev);
      exit(-1);
      break;
  }

  return cp;
}


BOOL type_info (
  somId msgId,                         /* method identifier                 */
  somId descId,                        /* descriptor identifier             */
  char *return_types,                  /* method return/dispatch types      */
  char *argument_types)                /* types of arguments returned       */
/******************************************************************************/
/* Arguments:  Message id, descriptor id, interface types, argument types     */
/*                                                                            */
/*  Returned:  CallStyle TRUE is IDL style.                                   */
/******************************************************************************/
{
  OrxSOMMethodInformation *methInf;    /* method information                */
  int i;                               /* loop counter                      */
  ULONG numArgs;                       /* number of arguments               */
  TypeCode type;                       /* type of an argument               */
  ParameterMode parmMode;              /* mode of a parameter               */
  OperationMode returnMode;            /* method operation mode             */
  char *itp, *atp;
  BOOL calltype = FALSE;               /* TRue is IDL callStyle (include EV)*/
  string cstyle;
  Environment ev;                      /* SOM environment vector            */


  SOM_InitEnvironment(&ev);            /* initialize the SOM environment    */
                                       /* get the method information        */
  methInf = new OrxSOMMethodInformation;
                                       /* Initial the method Information obj*/
  methInf->setMethodFromDescriptor(&ev, somStringFromId(descId), somStringFromId(msgId));
                                       /* Get the return information        */
  methInf->getReturnInfo(&ev, &type, &returnMode);
  cstyle = methInf->_get_callstyle(&ev);

                                       /* is this an IDL style call?      */
  if (!strcmp(CALLSTYLE_IDL,cstyle))
    calltype = TRUE;
                                       /* Env parm may be passed in.      */

  itp = return_types;
  *itp++ = '&';                        /* Indicate receiver is a SOM obj    */
                                       /* set the return type               */
  itp = type_char(type,itp);
  *itp++ = '*';                        /* set the indirection flag          */
  *itp = '\0';                         /* add the null terminator           */
                                       /* Get number of args for method     */
  numArgs = methInf->getNumArguments(&ev);

  atp = argument_types;                /* point to the argument start       */
  for (i = 1; i <= numArgs; i++) {     /* process each argument             */
                                       /* Get this argument info.           */
    methInf->getNthArgInfo(&ev, i, &type, &parmMode);
    atp = type_char(type,atp);         /* convert into character spec       */
  }
  *atp = '\0';                         /* add a terminator                  */
  methInf->somFree();                  /* free the method information       */

  return calltype;
}

/* @CHM001M modify max number of arguments (less troublesome way for SOM) */
#define MAXNUMARGS 100

void oryx_dispatch (SOMObject *somSelf, somId msgid, somId descid, va_list ap,
                    void *resultp)
/******************************************************************************/
/* Arguments:  SOM receiver, message id, descriptor id, arguments pointer,    */
/*             result pointer                                                 */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  char return_types[10];               /* returned SOM method types         */
  char argument_types[MAXNUMARGS+1];   /* types of the arguments            */

                                       /* create the interface spec         */
  type_info(msgid, descid, return_types, argument_types);
                                       /* send the message                  */
  RexxSendMessage((OREF)somSelf, somStringFromId(msgid), OREF_NULL, return_types, resultp, argument_types, &ap);
}

void SOMLINK oryx_dispatch2 (SOMObject *somSelf, void **resultp, somId msgid, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, result pointer, message id, arguments pointer    */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  char return_types[10];               /* returned SOM method types         */
  char argument_types[MAXNUMARGS+1];   /* types of the arguments            */
  somId descid;                        /* descriptor id                     */
  SOMClass *classobj;                  /* associated class object           */
  BOOL callStyle;                      /* true f IDL style calls.           */


  classobj = SOM_GetClass(somSelf);    /* get the class                     */
                                       /* get the descriptor id             */
  descid = classobj->somGetMethodDescriptor(msgid);
                                       /* get the interface descriptors     */
  callStyle = type_info(msgid, descid, return_types, argument_types);
  va_arg(ap,SOMObject *);              /* step over the first argument      */
  if (callStyle) {                     /* IDL style calling?                */
    va_arg(ap, Environment *);         /* yup, skip over EV parm.           */
  }

                                       /* forward to the object             */
  RexxSendMessage((OREF)somSelf, somStringFromId(msgid), OREF_NULL, return_types, resultp, argument_types, &ap);
}

void SOMLINK oryx_class_dispatch2 (SOMObject *somSelf, SOMClass *classobj, somToken *resultp, somId msgid, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, result pointer, message id, arguments pointer    */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  char return_types[10];               /* returned SOM method types         */
  char argument_types[MAXNUMARGS+1];   /* types of the arguments            */
  somId descid;
  BOOL callStyle;                      /* true f IDL style calls.           */

                                       /* get the descriptor id             */
  descid = classobj->somGetMethodDescriptor(msgid);
                                       /* get the interface descriptors     */
  callStyle = type_info(msgid, descid, return_types, argument_types);
  va_arg(ap,SOMObject *);              /* step over the first argument      */
  if (callStyle) {                     /* IDL style calling?                */
    va_arg(ap, Environment *);         /* yup, skip over EV parm.           */
  }
                                       /* forward to the object             */
  RexxSendMessage((OREF)somSelf, somStringFromId(msgid), (OREF) classobj, return_types, resultp, argument_types, &ap);
}


void SOMLINK oryx_class_dispatch (SOMObject *somSelf, SOMClass *classObj, void *resultp,
                          somId msgid, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, dispatch class, result pointer, message id,      */
/*             arguments pointer                                              */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  somId descid;                        /* method descriptor id              */
  char return_types[10];               /* returned SOM method types         */
  char argument_types[MAXNUMARGS+1];   /* types of the arguments            */
  char *msgName;
  char *msgIdString;

                                       /* get the method descriptor id      */
  descid = classObj->somGetMethodDescriptor(msgid);
                                       /* get the interface descriptor      */
  type_info(msgid,descid,return_types,argument_types);
                                       /* find actual message name.         */
  msgIdString = somStringFromId(msgid);
                                       /* msgId may be in form className::msg*/
  msgName = strstr(msgIdString, "::");
  if (NULL == msgName ) {
                                       /* nope, msgId is the message name.  */
    msgName = msgIdString;
  }
  else
    msgName += 2;                      /* bump past the :: delimiter.       */

                                       /* forward the class message         */
  RexxSendMessage((OREF)somSelf, msgName, OREF_NULL, return_types, resultp, argument_types, &ap);
}

void *SOMLINK oryx_dispatch_a (SOMObject *somSelf, INOUT somId methodId,
                        INOUT somId descriptor, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, message id, descriptor id, arguments pointer     */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  void *result;

  oryx_dispatch(somSelf,methodId,descriptor,ap,&result);
  return result;
}

float8 SOMLINK oryx_dispatch_d (SOMObject *somSelf, INOUT somId methodId,
                        INOUT somId descriptor, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, message id, descriptor id, arguments pointer     */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  float8 result;

  oryx_dispatch(somSelf,methodId,descriptor,ap,(void *)&result);
  return result;
}

integer4 SOMLINK oryx_dispatch_l (SOMObject *somSelf, INOUT somId methodId,
                          INOUT somId descriptor, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, message id, descriptor id, arguments pointer     */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  integer4 result;

  oryx_dispatch(somSelf,methodId,descriptor,ap,(void *)&result);
  return result;
}

void SOMLINK oryx_dispatch_v (SOMObject *somSelf, INOUT somId methodId,
                      INOUT somId descriptor, va_list ap)
/******************************************************************************/
/* Arguments:  SOM receiver, message id, descriptor id, arguments pointer     */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  oryx_dispatch(somSelf,methodId,descriptor,ap,NULL);
}

void *RexxSomSendA (SOMObject *somSelf, SOMClass *classObj, somId methodId, ...)
/******************************************************************************/
/* Arguments:  SOM receiver, SOM class, message id, message arguments         */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  va_list ap;
  void *result;

  va_start(ap,methodId);
  oryx_class_dispatch(somSelf, classObj, &result, methodId, ap);
  va_end(ap);

  return result;
}

float8  RexxSomSendD (SOMObject *somSelf, SOMClass *classObj, somId methodId, ...)
/******************************************************************************/
/* Arguments:  SOM receiver, SOM class, message id, message arguments         */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  va_list ap;
  float8 result;

  va_start(ap,methodId);
  oryx_class_dispatch(somSelf, classObj, &result, methodId, ap);
  va_end(ap);

  return result;
}

integer4  RexxSomSendL (SOMObject *somSelf, SOMClass *classObj, somId methodId, ...)
/******************************************************************************/
/* Arguments:  SOM receiver, SOM class, message id, message arguments         */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  va_list ap;
  integer4 result;

  va_start(ap,methodId);
  oryx_class_dispatch(somSelf, classObj, &result, methodId, ap);
  va_end(ap);

  return result;
}

void  RexxSomSendV (SOMObject *somSelf, SOMClass *classObj, somId methodId, ...)
/******************************************************************************/
/* Arguments:  SOM receiver, SOM class, message id, message arguments         */
/*                                                                            */
/*  Returned:  Result of method                                               */
/******************************************************************************/
{
  va_list ap;

  va_start(ap, methodId);
  oryx_class_dispatch(somSelf, classObj, NULL, methodId, ap);
  va_end(ap);
}


LONG REXXENTRY RexxCallProgram (
  PCHAR name,                          /* program name                      */
  PCHAR interface,                     /* interface definition string       */
  PVOID result,                        /* returned argument                 */
  ... )                                /* variable number of arguments      */
/******************************************************************************/
/* Function:  Perform a REXX call, passing a variable number of arguments     */
/******************************************************************************/
{
  va_list    arguments;                /* variable argument list            */
  LONG       rc;                       /* REXX return code                  */
  CHAR       temp_interface[10];       /* temporary interface list          */


  va_start(arguments, result);         /* get the start of the valist       */
  temp_interface[0] = interface[0];    /* copy over the return type         */
  temp_interface[1] = 'z';             /* program name is a string          */
  temp_interface[2] = '*';             /* third one is an indirection       */
  temp_interface[3] = '\0';            /* end of the interface list         */

                                       /* go call the program               */
  rc = RexxSendMessage(ProcessLocalServer, "CALL_PROGRAM", OREF_NULL, temp_interface, result, name, interface + 1, &arguments);
  va_end(arguments);                   /* end of variable list processing   */
  return rc;                           /* return the return code            */
}

LONG REXXENTRY RexxCallString (
  PCHAR program,                       /* program string                    */
  PCHAR interface,                     /* interface definition string       */
  PVOID result,                        /* returned argument                 */
  ... )                                /* variable number of arguments      */
/******************************************************************************/
/* Function:  Perform a REXX call, passing a variable number of arguments     */
/******************************************************************************/
{
  va_list    arguments;                /* variable argument list            */
  LONG       rc;                       /* REXX return code                  */
  CHAR       temp_interface[10];       /* temporary interface list          */


  va_start(arguments, result);         /* get the start of the valist       */
  temp_interface[0] = interface[0];    /* copy over the return type         */
  temp_interface[1] = 'z';             /* program name is a string          */
  temp_interface[2] = '*';             /* third one is an indirection       */
  temp_interface[3] = '\0';            /* end of the interface list         */

                                       /* go call the program               */
  rc = RexxSendMessage(ProcessLocalServer, "CALL_PROGRAM", OREF_NULL, temp_interface, program, interface + 1, &arguments);
  va_end(arguments);                   /* end of variable list processing   */
  return rc;                           /* return the return code            */
}

#endif /*SOM*/

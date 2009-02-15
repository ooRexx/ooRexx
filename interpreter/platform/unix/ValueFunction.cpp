/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX AIX Support                                             aixvalue.c    */
/*                                                                            */
/* AIX system specific VALUE() built-in function routine                      */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "ActivityManager.hpp"
#include "SystemInterpreter.hpp"
#include <unistd.h>
#include <stdlib.h>

#define  SELECTOR  "ENVIRONMENT"       /* environment selector              */

extern int putflag;

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SetEnvironmentVariable                       */
/*                                                                   */
/*   Descriptive Name:  set value of environment variable            */
/*                                                                   */
/*********************************************************************/

int SetEnvironmentVariable(
  RexxString * Name,                   /* variable name                     */
  RexxString * Value )                 /* new variable value                */

{
  char  *Env_Var_String = NULL;        /* Environment variable string for   */
  size_t size;                         /* size of the string                */
  char **Environment;                  /* environment pointer               */
  char  *del = NULL;                   /* ptr to old unused memory          */
  char  *np;
  size_t i;
  char   namebufcurr[256];             /* buf for extracted name            */

  Environment = getEnvironment();      /* get the environment               */
  if(!putflag){                        /* first change in the environment ? */
    /* copy all entries to dynamic memory                                   */
    for(;*Environment != NULL;Environment++){/*for all entries in the env   */
      size = strlen(*Environment)+1;   /* get the size of the string        */
      Env_Var_String = (char *)malloc(size); /* and alloc space for it      */
      memcpy(Env_Var_String,*Environment,size);/* copy the string           */
      putenv(Env_Var_String);          /* and chain it in                   */
    }
    putflag = 1;                       /* prevent do it again               */
    Environment = getEnvironment();    /* reset the environment pointer     */
  }
                                       /* calculate the size                */
  size = strlen(Name->getStringData())+strlen(Value->getStringData())+2;
  Env_Var_String = (char *)malloc(size); /* get the memory                  */
                                       /* find the entry in the environ     */
  for(;*Environment != NULL;Environment++){
    np = *Environment;
                                       /* extract the the name              */
                                       /* from the current env string       */
    for(i=0;(*np!='=')&&(i<255);np++,i++){
      memcpy(&(namebufcurr[i]),np,1);  /* copy the character                */
    }
    memcpy(&(namebufcurr[i]),"\0",1);      /* copy the terminator           */

    if(!strcmp(Name->getStringData(),namebufcurr))/* have a match ?         */
      del = *Environment;              /* remember it for deletion          */
  }

  if (Value != (RexxString *) TheNilObject)
  {
    sprintf(Env_Var_String, "%s=%s",Name->getStringData(),Value->getStringData());
    putenv(Env_Var_String);
  }
  if(del)                              /* if there was a old one            */
    free(del);                         /* free it                           */
  return 0;                            /* return success                    */
}

bool SystemInterpreter::valueFunction(
    RexxString *Name,                  /* variable name                     */
    RexxObject *NewValue,              /* new assigned value                */
    RexxString *Selector,              /* variable selector                 */
    RexxObject *&result)
{
  const char * OldValue;               /* old environment value             */

  Selector = Selector->upper();        /* upper case the selector           */
                                       /* Linux environment variables can   */
                                       /* be lowercased                     */
                                       /* and the name too                  */
  if (!Selector->strCompare(SELECTOR)) /* correct selector?                 */
  {
      return false;                    // we can't handle this one
  }
                                       /* scan for the variable             */
  OldValue = getenv(Name->getStringData());
  if (OldValue != NULL)                /* have a value already?   */
    result = new_string(OldValue);    /* Yes -  convert to Rexx string     */
  else
    result = OREF_NULLSTRING;          /* otherwise, return null            */

  if (NewValue != OREF_NULL)           /* if there's a new value, set it    */
  {
    if(NewValue == (RexxString *) TheNilObject)
    {
      SetEnvironmentVariable(Name, (RexxString *) TheNilObject);
    }
    else
    {
    SetEnvironmentVariable(Name, stringArgument(NewValue, ARG_TWO));
    }
  }
  return true;
}



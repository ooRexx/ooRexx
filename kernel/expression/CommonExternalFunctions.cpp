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

#include "RexxCore.h"                         /* global REXX definitions        */
#include "RexxNativeAPI.h"                  /* Lot's of useful REXX macros    */


/******************************************************************************/
/* activation_rxfuncadd - Method to support RXFUNCADD function                */
/******************************************************************************/
RexxMethod3(REXXOBJECT,sysRxfuncadd,CSTRING,name,CSTRING,module,CSTRING,proc)
{
    /* must have two arguments           */
    if (name == NO_CSTRING || module == NO_CSTRING)
    {
        /* raise an error                    */
        rexx_exception(Error_Incorrect_call);
    }
    if (proc == NO_CSTRING)              /* no procedure given?               */
    {
        proc = name;                       /* use the defined name              */
    }
    /* try to register the function      */
    return RexxRegisterFunctionDll(name, module, proc) != 0 ? ooRexxTrue : ooRexxFalse;
}


/******************************************************************************/
/* activation_rxfuncdrop - Method to support RXFUNCDROP function              */
/******************************************************************************/
RexxMethod1(REXXOBJECT,sysRxfuncdrop,CSTRING,name)
{
    if (name == NO_CSTRING)              /* must have a name                  */
    {
        /* raise an error                    */
        rexx_exception(Error_Incorrect_call);
    }
    /* try to drop the function          */
    return RexxDeregisterFunction(name) != 0 ? ooRexxTrue : ooRexxFalse;
}


/******************************************************************************/
/* activation_rxfuncquery - Method to support RXFUNCQUERY function            */
/******************************************************************************/
RexxMethod1(REXXOBJECT,sysRxfuncquery,CSTRING,name)
{
    if (name == NO_CSTRING)              /* must have a name                  */
    {
        /* raise an error                    */
        rexx_exception(Error_Incorrect_call);
    }
    /* try to drop the function          */
    return RexxQueryFunction(name) != 0 ? ooRexxTrue : ooRexxFalse;
}

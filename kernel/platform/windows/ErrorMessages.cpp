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
/*********************************************************************
 *                                                                   *
 *                                                                   *
 *  Module   : okerr.c                                               *
 *                                                                   *
 *  Purpose  : Retrieve message from hard coded message table        *
 *  Notes    :                                                       *
 *                                                                   *
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "RexxMessageNumbers.h"
#include "RexxCore.h"
#include "StringClass.hpp"

extern HINSTANCE horyxkDll;            /* Handle to oryxk dll               */
RexxString *  SysMessageText(
    INT code )                         /* message code to extract           */
{
CHAR           DataArea[256];          /* buf addr to return message        */

                                       /* loop through looking for the      */
                                       /* error code                        */
 if (LoadString(horyxkDll, code, DataArea, 255))
    return new_string(DataArea, strlen(DataArea));
 else
    return OREF_NULL;                  /* no message retrieved              */
 }

/* Change Activity
 * $Log: ErrorMessages.cpp,v $
 * Revision 1.3  2006/03/07 15:37:19  wdashley
 * ArtifactId: None
 * Comment: Removed all references to the developerworks site.
 *
 * Revision 1.2  2006/02/16 16:43:48  wdashley
 * ArtifactId: None
 * Comment: Updated copyright notice on all files.
 *
 * Revision 1.1  2005/03/27 01:38:04  rexx
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/16 02:53:24  adminorx
 * Mass change - added standard RexxLA copyright notice.
 *
 * Revision 1.1  2005/01/13 23:54:15  adminorx
 * rename Windows .c source files to proper .cpp names
 *
 * Revision 1.2  2005/01/06 23:51:25  oorexx
 * Restructure generated message header files
 *
 * Revision 1.1.1.1  2004/10/19 21:44:28  oorexx
 * Initial import without docs.
 *
 * Revision 1.1.1.1  2004/10/12 13:15:09  oorexx
 * Initial import.
 *

      Rev 1.2   24 May 1995 17:07:48   cmvc_cs

      Rev 1.1   02 May 1995 14:29:58   cmvc_cs
   Initial revision.

      Rev 1.6   09 Nov 1994 12:57:28   cmvc_cs

      Rev 1.5   13 Oct 1994 16:21:02   cmvc_cs

      Rev 1.3   23 Sep 1994 11:00:12   cmvc_cs

      Rev 1.2   23 Aug 1994 18:45:44   cmvc_cs
 */


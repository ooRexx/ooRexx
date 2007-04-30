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
/*                                                                            */
/* Function: DBCS_Setup                                                       */
/*                                                                            */
/* Descriptive name: Set up DBCS environment info.                            */
/*                                                                            */
/******************************************************************************/

#define  INCL_DOSNLS
#define  INCL_DOSMISC
#include "RexxCore.h"
#include <string.h>
#include <winnls.h>
#define DBCS_TableSize         256     /* size of DBCS vector table         */

BOOL SysDBCSSetup(
  ULONG  *CodePage,                    /* current code page                 */
  PUCHAR  DBCS_Table)                  /* DBCS first byte table             */
/******************************************************************************/
/* Function:  Setup the DBCS ASCII first-byte table.                          */
/******************************************************************************/
{
  BOOL   HaveDBCS;                     /* DBCS codepage flag                */
  INT    i;                            /* loop counter                      */
  INT    j;                            /* loop counter                      */
  CPINFO CPInfo;

  HaveDBCS = FALSE;                    /* not a DBCS codepage (yet)         */
                                       /* get code page info                */
  *CodePage = GetOEMCP();              /* get current codepage              */
  GetCPInfo(*CodePage, &CPInfo);       /* use current codepage              */
                                       /* clear out DBCS table              */
  memset((PCHAR)DBCS_Table, FALSE, DBCS_TableSize);
                                       /* fill in first byte table          */
  for (i = 0; CPInfo.LeadByte[i]; i += 2) {
    HaveDBCS = TRUE;                   /* got possible DBCS characters      */
                                       /* fill in the vector range          */
    for (j = CPInfo.LeadByte[i]; j <= CPInfo.LeadByte[i + 1]; j++)
      DBCS_Table[j] = (UCHAR)TRUE;     /* turn on the byte                  */
  }
  return HaveDBCS;                     /* return DBCS indicator             */
}

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
/* REXX Kernel                                                                */
/*                                                                            */
/*   Save and Restore all global "name" strings                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
RexxString *kernel_name (char* value);

void createStrings (void)
/******************************************************************************/
/* Function:  Create all globally available string objects                    */
/******************************************************************************/
{
  LONG        stringCount;             /* number of strings                 */
  RexxArray * stringArray;             /* saved array of strings            */

                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to create and give a count of the */
                                       /* strings created                   */
  #undef GLOBAL_NAME
  #define GLOBAL_NAME(name, value) OREF_##name = kernel_name(value); stringCount++;

  stringCount = 0;                     /* no strings yet                    */
  #include "GlobalNames.h"                 /* now create the strings            */

  stringArray = new_array(stringCount);/* get a large array                 */
                                       /* save the strings                  */
  kernel_public(CHAR_NAME_STRINGS, stringArray, TheKernel);

                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to save each string in the array  */
                                       /* at its relative offset            */
  #undef GLOBAL_NAME
  #define GLOBAL_NAME(name, value) stringArray->put((RexxObject *)OREF_##name, stringCount); stringCount++;

  stringCount = 1;                     /* start with the first string       */
  #include "GlobalNames.h"                 /* now save the strings              */
}

void restoreStrings (void)
/******************************************************************************/
/* Function:  Create all globally available string objects                    */
/******************************************************************************/
{
  RexxArray * stringArray;             /* array of strings                  */
  RexxString **strings;                /* pointer to individual strings     */
  long         stringCount;

                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to create and give a count of the */
                                       /* strings created                   */
  #undef GLOBAL_NAME
  #define GLOBAL_NAME(name, value) OREF_##name = *strings++;

  stringCount = 0;                     /* start with the first string       */
  stringArray = (RexxArray *)TheSaveArray->get(saveArray_NAME_STRINGS);
                                       /* point to the actual array data    */
  strings = (RexxString **)stringArray->data();
  #include "GlobalNames.h"                 /* now restore the strings           */
}

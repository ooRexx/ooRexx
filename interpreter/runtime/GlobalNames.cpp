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
/* REXX Kernel                                                                */
/*                                                                            */
/*   Save and Restore all global "name" strings                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
void RexxMemory::createStrings()
/******************************************************************************/
/* Function:  Create all globally available string objects                    */
/******************************************************************************/
{
    // if we're calling this, then we're building the image.  Make sure the
    // global string directory is created first.
    globalStrings = new_directory();
                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to create each of the strings     */
#undef GLOBAL_NAME
#define GLOBAL_NAME(name, value) OREF_##name = getGlobalName(value);

#include "GlobalNames.h"             /* now create the strings            */
}


RexxArray *RexxMemory::saveStrings()
/******************************************************************************/
/* Function:  Create all globally available string objects                    */
/******************************************************************************/
{
                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to count the number of string     */
                                       /* objects we need to save           */
  #undef GLOBAL_NAME
  #define GLOBAL_NAME(name, value) stringCount++;

  size_t stringCount = 0;              /* no strings yet                    */
  #include "GlobalNames.h"             /* now create the strings            */

  // get an array to contain all of the string values
  RexxArray *stringArray = new_array(stringCount);
                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to save each string in the array  */
                                       /* at its relative offset            */
  #undef GLOBAL_NAME
  #define GLOBAL_NAME(name, value) stringArray->put((RexxObject *)OREF_##name, stringCount); stringCount++;

  stringCount = 1;                     /* start with the first string       */
  #include "GlobalNames.h"             /* now save the strings              */

  return stringArray;                  // and return the saved string array
}

void RexxMemory::restoreStrings(RexxArray *stringArray)
/******************************************************************************/
/* Function:  Create all globally available string objects                    */
/******************************************************************************/
{
                                       /* redefine the GLOBAL_NAME macro    */
                                       /* to create and give a count of the */
                                       /* strings created                   */
  #undef GLOBAL_NAME
  #define GLOBAL_NAME(name, value) OREF_##name = *strings++;

  RexxString **strings = (RexxString **)stringArray->data();
  #include "GlobalNames.h"                 /* now restore the strings           */
}

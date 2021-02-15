/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
#include "StringTableClass.hpp"
#include "GlobalNames.hpp"


// create all globally available string objects.
void MemoryObject::createStrings()
{
    // if we're calling this, then we're building the image.  Make sure the
    // global string directory is created first.
    globalStrings = new_string_table();

    // redefine the GLOBAL_NAME macro to create each of the strings
    #undef GLOBAL_NAME
    #define GLOBAL_NAME(name, value) GlobalNames::name = getGlobalName(value);

    #include "GlobalNames.h"
}


/**
 * Save all globally available string objects in the image.
 *
 * @return An array of all global string objects.
 */
ArrayClass *MemoryObject::saveStrings()
{
    // pass one, count how many string objects we have
    #undef GLOBAL_NAME
    #define GLOBAL_NAME(name, value) stringCount++;

    size_t stringCount = 0;
    #include "GlobalNames.h"

    // get an array to contain all of the string values
    ArrayClass *stringArray = new_array(stringCount);

    // redefine GLOBAL_NAME to save each string in the array
    #undef GLOBAL_NAME
    #define GLOBAL_NAME(name, value) stringArray->put(GlobalNames::name, stringCount); stringCount++;

    // the index gets incremented as we go
    stringCount = 1;
    #include "GlobalNames.h"

    return stringArray;                  // and return the saved string array
}


/**
 * Restore all globally available string objects during an image restore.
 *
 * @param stringArray
 *               The string array from the image.
 */
void MemoryObject::restoreStrings(ArrayClass *stringArray)
{
    // redefine GLOBAL_NAME to restore each string pointer
    #undef GLOBAL_NAME
    #define GLOBAL_NAME(name, value) GlobalNames::name = *strings++;

    RexxString **strings = (RexxString **)stringArray->data();
    #include "GlobalNames.h"
}


// now actual locations of the constants declared in
// GlobalNames.h.
#undef GLOBAL_NAME
#define GLOBAL_NAME(name, value) RexxString *GlobalNames::name = OREF_NULL;

#include "GlobalNames.h"

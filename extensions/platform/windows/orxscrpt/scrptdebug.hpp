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
#ifndef SCRPTDEBUG
#define SCRPTDEBUG

#include "WObaseproto.h"     // Need for the advanced data types (DWORD, etc.)
#include <dispex.h>

/****************************************************************************
 *
 *     Define all datatypes and constants first.
 *
 *****************************************************************************/

/****************************************************************************
 *
 *     Since classes cannot be initialized with a list (={}), a structure
 *  must be used.  This structure is a simple as possible, so no length
 *  indicator is provided.  Lists built with this structure can handle
 *  flags that are both horizontal (where each bit has a separate meaning)
 *  and vertical (where all bits combined form a value with an unique meaning).
 *     So that no assumption is made about the value of a flag, and that the
 *  end of the list can be detected without a length count, the last item in
 *  the list should have a null pointer for the character string.
 *
 *****************************************************************************/
typedef struct FLAGLISTstruct {
  DWORD Flag;           // DEFINEd or literal value of the flag.
  const char *Meaning;  // Printable defnition.
  } FL, *pFL;




/* Defined in winnls.h  cb should be sizeof(a) is what they said.
    However, since w is the wide character string, I think cb
    should be wcslen(w).
    w  - pointer to Wide (Unicode) character string.
    a  - pointer to buffer to puts Ascii string.  NB MS considers all non-
         Unicode strings to be MultiByte.
    cb - Character (b?) count.
  Following the precedent of other C functions, the destination is listed first.
*/
#define W2C(a, w, cb)     WideCharToMultiByte(                              \
                                               CP_ACP,                      \
                                               0,                           \
                                               w,                           \
                                               -1,                          \
                                               a,                           \
                                               cb,                          \
                                               NULL,                        \
                                               NULL)

// Defined in winnls.h  cb should be sizeof(w)
#define C2W(w, a, cb)     MultiByteToWideChar(                              \
                                               CP_ACP,                      \
                                               0,                           \
                                               a,                           \
                                               -1,                          \
                                               w,                           \
                                               (int)(cb))

#ifdef SCRIPTDEBUG
void FPRINTF(FILE *Stream, const char *Format,...);
void FPRINTF2(FILE *Stream, const char *Format, ...);
void FPRINTF3(FILE *Stream, const char *Format, ...);
#else
inline void FPRINTF(FILE *Stream, const char *Format,...) { }
inline void FPRINTF2(FILE *Stream, const char *Format, ...) { }
inline void FPRINTF3(FILE *Stream, const char *Format, ...) { }
#endif

/****************************************************************************
 *
 *     Reverses the characters in the memory location pointed to by Buffer
 *  for BufLen bytes in length.  Buffer may point to any location, the Z
 *  string terminator ('\00') is treated as any other character.
 *
 *
 *****************************************************************************/
void Reverse(char *Buffer, int BufLen);


/****************************************************************************
 *
 *     Every byte, for Len bytes, in Source are expanded to two bytes that
 *  represent the hexidecimal encoding of the byte that was expanded.  The
 *  resulting bytes are stored in Dest.
 *
 *
 *****************************************************************************/
void C2X(char *Dest, char *Source, int Len);


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void X2C(char *Dest, char *Source, int DLen, int SLen);


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void pIID2Reg(char *Dest, char *Source);


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void strstrip(char *Dest,char *Source);


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
int Dump(FILE *Stream,char *Mem, int Length);


/****************************************************************************
 *
 *     This function returns a pointer to a character string representing
 *  the given flag value.  The flag can be either horizontal or vertical,
 *  as denoted by the first parameter.  If the flag is vertical, then
 *  the string is the single value within the list that flag represents.
 *  However, if the flag is horizontal, then the string is composed of a
 *  concatenation of all meanings for the flag and each meaning is separated
 *  by a plus sign (+).
 *     As this is intended for internal use, the buffer that is used to
 *  hold the return for the horizontal flags is limited in size.  If a flag
 *  has too many meanings, it is possible to overrun the static buffer
 *  allocated to hold this.  Subsequent calls to this procedure will
 *  destroy the results of previous calls.  Consquently, strings needed
 *  for extended periods should be copied to safer storage.
 *
 *****************************************************************************/
const char *FlagMeaning(char FlagType, DWORD Flag, pFL List);


/****************************************************************************
 *
 *     NameThatInterface accepts a UUID to an interface that has been
 *  converted into a string using the Win32 function, StringFromGUID2().
 *  It returns a pointer to the data of the default Value for the key
 *  HKEY_CLASSES_ROOT\Interface\{UUID}, which by convention is the name
 *  of the interface identified by this UUID.
 *     If the key cannot be found, the name is too long, the data type
 *  is not REG_SZ (a string), or not enough dynamic memory (i.e., any
 *  error) a NULL pointer is returned.
 *
 *     It is the responsibility of the caller to free the dynamic
 *  memory that is pointed to on the return.
 *
 *****************************************************************************/
char *NameThatInterface(OLECHAR *IID);



/****************************************************************************
 *
 *
 *****************************************************************************/
void PrntMutant(FILE *Stream, VARIANT *Mutant);


/****************************************************************************
 *
 *
 *****************************************************************************/
void PrntDispParams(FILE *Stream, DISPPARAMS *Parms);


/****************************************************************************
 *
 *
 *****************************************************************************/
void EnumerateProperty(FILE *Stream, IDispatchEx *DispEx, LCID Lang);

#include "orxscrpt_main.hpp"           //  Needed for some global variables that this sets,
                                       // however, this can't come first since some items in
                                       // it depend on these definitions.
#endif

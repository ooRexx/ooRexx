/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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

#ifndef Included_Utilities
#define Included_Utilities

#include <sys/types.h>

class Utilities
{
public:
    static inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
    static inline bool isAlpha(char c) { return isUpper(c) || isLower(c); }
    static inline bool isUpper(char c) { return c >= 'A' && c <= 'Z'; }
    static inline bool isLower(char c) { return c >= 'a' && c <= 'z'; }
    static inline char toUpper(char c) { return isLower(c) ? c & ~0x20 : c; }
    static inline char toLower(char c) { return isUpper(c) ? c | 0x20 : c; }
    static int strCaselessCompare(const char *opt1, const char *opt2);
    static int memicmp(const void *opt1, const void *opt2, size_t len);
    static void strupper(char *str);
    static void strlower(char *str);
    static const char *strnchr(const char *, size_t n, char ch);
    static const char *locateCharacter(const char *s, const char *set, size_t l);
    static bool strncpy(char *dest, const char *src, size_t len);
};



/**
 * A simple implemenation of a smart pointer to prevent memory leaks.
 */
class AutoFree
{
 public:
     AutoFree() : value(NULL) { };
     AutoFree(char *p) : value(p) { }
     ~AutoFree()
     {
         if (value != NULL)
         {
             free(value);
         }
         value = NULL;
     }
     AutoFree & operator=(char *p)
     {
         if (value != NULL)
         {
             free(value);
         }
         value = p;
         return *this;
     }
     operator char *() const { return value; }
     int operator==(char *p) { return value == p; }

     bool realloc(size_t newLen)
     {
         // on failure, realloc returns NULL, but doesn't free the old buffer
         char *newValue;
         newValue = (char *)::realloc(value, newLen);
         if (newValue == NULL)
         {
             return false;
         }
         value = newValue;
         return true;
     }
 private:
     char *value;
};

#endif


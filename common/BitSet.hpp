/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* Handy format-portable template class for bit sets                          */
/*                                                                            */
/******************************************************************************/

#ifndef BitSet_Included
#define BitSet_Included

/**
 * This is a replacement for the std::bitset class that is more
 * portable. For classes that are saved via rexxc compilation
 * that use FlagSets, the different compiler implementations of
 * the bitset class so not interoperate well. This ensures we
 * have a single data format for saved bitsets across all of the
 * interpreters.
 *
 * This is also simpler than the std bitset, limited to just 32
 * bits in the set and only implementing the operations required
 * by the interpreter.
 */
template<size_t TMaxBits> class BitSet
{   // store fixed-length sequence of Boolean elements
 public:

     // default constructor...uses all false values
     BitSet() : bits(0) { }

     BitSet& set()
     {
         bits = ~((uint32_t)0);
         return (*this);
     }

     inline BitSet& set(size_t pos, bool val = true)
     {
         // belt and braces, don't allow beyond the specified number of bits
         if (pos <= TMaxBits)
         {
             if (val)
             {
                 bits |= (uint32_t)1 << pos;
             }
             else
             {
                 bits &= ~((uint32_t)1 << pos);
             }
         }

         return (*this);
     }

     BitSet& reset()
     {
         bits = 0;
         return (*this);
     }

     inline BitSet& reset(size_t pos)
     {
         return set(pos, false);
     }

     BitSet& flip()
     {
         // flip the bits individually so we don't turn on any bits out of range
         for (size_t i = 0; i < TMaxBits; i++)
         {
             flip(i);
         }
         return (*this);
     }

     inline BitSet& flip(size_t pos)
     {
         // belt and braces, don't allow beyond the specified number of bits
         if (pos <= TMaxBits)
         {
             bits ^= (uint32_t)1 << pos;
         }
         return (*this);
     }

     inline bool test(size_t pos) const
     {
         // belt and braces, don't allow beyond the specified number of bits
         if (pos <= TMaxBits)
         {
             return (bits & ((uint32_t)1 << pos)) != 0;
         }
         else
         {
             return false;
         }
     }

     bool any() const
     {
         return bits != 0;
     }

     bool none() const
     {
         return bits == 0;
     }

     bool all() const
     {
         return count() == TMaxBits;
     }


 private:
     size_t count() const
     {
         size_t c = 0;
         for (size_t i = 0; i < TMaxBits; i++)
         {
             if (test(i))
             {
                 c++;
             }
         }
         return c;
     }



    uint32_t bits;    // the bits in our bitset
};


#endif


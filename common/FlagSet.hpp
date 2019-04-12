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
/* Handy type-safe template class for flag sets.                              */
/*                                                                            */
/******************************************************************************/

#ifndef FlagSet_Included
#define FlagSet_Included
#include "BitSet.hpp"

// a useful type-safe bit flag class where the allowed settings are based
// on enum values.

// this is defined as a template so we can specify what enums are stored here.
template<typename TEnum, int TMaxFlags = 8>
class FlagSet
{
 public:
     // a reference to a flag set item.  This is a proxy that allows
     // assignment semantics
     class FlagSetReference
     {
          friend class FlagSet<TEnum, TMaxFlags>;

      public:
          inline ~FlagSetReference() { }

          // assign bool to the a flag position.
          FlagSetReference & operator=(bool val)
          {
              refSet->set(refPos, val);
              return *this;
          }

          // allows a flag value to be retrieved.
          operator bool() const
          {
              return refSet->test(refPos);
          }

      private:
          // constructor for a reference proxy
          FlagSetReference(FlagSet<TEnum, TMaxFlags> &theSet, TEnum pos)
              : refSet(&theSet), refPos(pos) { }

          // pointer to the set we're proxying
          FlagSet<TEnum, TMaxFlags> *refSet;
          // the element we test
          TEnum refPos;
     };

     FlagSet() { }
     // some constructors for creating constant sets for
     // testing setting
     FlagSet(const TEnum f1)
     {
         set(f1);
     }

     FlagSet(const TEnum f1, const TEnum f2)
     {
         set(f1);
         set(f2);
     }

     FlagSet(const TEnum f1, const TEnum f2, const TEnum f3)
     {
         set(f1);
         set(f2);
         set(f3);
     }

     FlagSet(const TEnum f1, const TEnum f2, const TEnum f3, const TEnum f4)
     {
         set(f1);
         set(f2);
         set(f3);
         set(f4);
     }

     FlagSet(const TEnum f1, const TEnum f2, const TEnum f3, const TEnum f4, const TEnum f5)
     {
         set(f1);
         set(f2);
         set(f3);
         set(f4);
         set(f5);
     }

     FlagSet(const TEnum f1, const TEnum f2, const TEnum f3, const TEnum f4, const TEnum f5, const TEnum f6)
     {
         set(f1);
         set(f2);
         set(f3);
         set(f4);
         set(f5);
         set(f6);
     }

     // set a flag value to true
     inline void set(const TEnum flag, bool val = true)
     {
         flags.set(flag, val);
     }


     // set all flag values to true
     inline void set()
     {
         flags.set();
     }


     // set multiple flags values on
     inline void set(const TEnum f1, const TEnum f2)
     {
         flags.set(f1);
         flags.set(f2);
     }


     // set multiple flag values on
     inline void set(const TEnum f1, const TEnum f2, const TEnum f3)
     {
         flags.set(f1);
         flags.set(f2);
         flags.set(f3);
     }


     // set multiple flag values on
     inline void set(const TEnum f1, const TEnum f2, const TEnum f3, const TEnum f4)
     {
         flags.set(f1);
         flags.set(f2);
         flags.set(f3);
         flags.set(f4);
     }


     // turn all flag values off
     inline void reset()
     {
         flags.reset();
     }

     // turn a flag value off
     inline void reset(const TEnum flag)
     {
         flags.reset(flag);
     }


     // turn a flag value off
     inline void reset(const TEnum f1, const TEnum f2)
     {
         flags.reset(f1);
         flags.reset(f2);
     }


     // turn a flag value off
     inline void reset(const TEnum f1, const TEnum f2, const TEnum f3)
     {
         flags.reset(f1);
         flags.reset(f2);
         flags.reset(f3);
     }


     // turn a flag value off
     inline void reset(const TEnum f1, const TEnum f2, const TEnum f3, const TEnum f4)
     {
         flags.reset(f1);
         flags.reset(f2);
         flags.reset(f3);
         flags.reset(f4);
     }


     // flip the value of a bit
     inline void flip(const TEnum flag)
     {
         flags.flip(flag);
     }


     // flip all flag values
     inline void flip()
     {
         flags.flip();
     }


     // test a flag value
     inline bool test(const TEnum flag)
     {
         return flags.test(flag);
     }


     // access the value of a flag
     inline bool operator [](const TEnum flag) const
     {
         return flags.test(flag);
     }

     // proxied access of a flag (allows assignment)
     FlagSetReference operator [](const TEnum flag)
     {
         return FlagSetReference(*this, flag);
     }


     // test if any flags are set
     inline bool any() const
     {
         return flags.any();
     }

     // test for any of the specified flags being set
     inline bool any(const TEnum flag1, const TEnum flag2)
     {
         return flags.test(flag1) || flags.test(flag2);
     }

     // test for any of the specified flags being set
     inline bool any(const TEnum flag1, const TEnum flag2, const TEnum flag3)
     {
         return flags.test(flag1) || flags.test(flag2) || flags.test(flag3);
     }


     // test if all flags are set
     inline bool all() const
     {
         return flags.all();
     }



    // test for all of the specified flags being set
    inline bool all(const TEnum flag1, const TEnum flag2)
    {
        return flags.test(flag1) && flags.test(flag2);
    }

    // test for all of the specified flags being set
    inline bool all(const TEnum flag1, const TEnum flag2, const TEnum flag3)
    {
        return flags.test(flag1) && flags.test(flag2) && flags.test(flag3);
    }

    // test if no flags are set
    inline bool none() const
    {
        return flags.none();
    }

private:

    BitSet<TMaxFlags> flags;
};


#endif


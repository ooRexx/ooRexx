/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*  complex.rex         Open Object Rexx Samples                              */
/*                                                                            */
/*  A complex number class.                                                   */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates how to create a complex number class using the  */
/*  ::class and ::method directives.  Also shown is an example of subclassing */
/*  the complex number class (the vector subclass).  Finally, the stringlike  */
/*  class demonstrates the use of a mixin to provide some string behavior to  */
/*  the complex number class.                                                 */
/******************************************************************************/

                                              /* complex number data class      */
::class "Complex" public inherit stringlike orderable

::method init                                 /* initialize a complex number    */
  expose real imaginary                       /* expose the state data          */
  use strict arg real, imaginary = 0          /* access the two numbers         */
  real += 0                                   /* force rounding                 */
  imaginary += 0

::method '[]' class                           /* create a new complex number    */
  forward message("NEW")                      /* just a synonym for NEW         */

-- read-only attributes for the real and imaginary parts
::attribute real GET
::attribute imaginary GET

::method '+'                                  /* addition method                */
  expose real imaginary                       /* access the state values        */
  use strict arg adder = .nil                 /* get the operand                */
  if arg(1,'o') then                          /* prefix plus operation?         */
    return self                               /* don't do anything with this    */

  if adder~isa(.string) then                  /* if just a simple number,       */
    adder = self~class~new(adder)             /* convert to complex             */

                                              /* return a new item of same class*/
                                              /* (this could potentially be a   */
                                              /* subclass of Complex)           */
  return self~class~new(real + adder~real, imaginary + adder~imaginary )

::method '-'                                  /* subtraction method             */
  expose real imaginary                       /* access the state values        */
  use strict arg adder = .nil                 /* get the operand                */
  if arg(1,'o') then                          /* prefix minus operation?        */
                                              /* negate the number              */
    return self~class~new(-real, -imaginary)

  if adder~isa(.string) then                  /* if just a simple number,       */
    adder = self~class~new(adder)             /* convert to complex             */

                                              /* return a new item              */
  return self~class~new(real - adder~real, imaginary - adder~imaginary)

::method '*'                                  /* multiplication method          */
  expose real imaginary                       /* access the state values        */
  use strict arg multiplier                   /* get the operand                */

  if multiplier~isa(.string) then             /* if just a simple number,       */
    multiplier = self~class~new(multiplier)   /* convert to complex             */
                                              /* calculate the real part        */
  tempreal = (real * multiplier~real) - (imaginary * multiplier~imaginary)
                                              /* calculate the imaginary part   */
  tempimaginary = (real * multiplier~imaginary) + (imaginary * multiplier~real)
                                              /* return a new item              */
  return self~class~new(tempreal, tempimaginary)

::method '/'                                  /* division method                */
  expose real imaginary                       /* access the state values        */
  use strict arg divisor                      /* get the operand                */

  if divisor~isa(.string) then                /* if just a simple number,       */
    divisor = self~class~new(divisor)         /* convert to complex             */

  a=real                                      /* get real and imaginaries for   */
  b=imaginary                                 /* both numbers                   */
  c=divisor~real
  d=divisor~imaginary
  qr=((b*d)+(a*c))/(c**2+d**2)                /* generate the new result values */
  qi=((b*c)-(a*d))/(c**2+d**2)
  return self~class~new(qr,qi)                /* return the new value           */

::method '%'                                  /* integer division method        */
  expose real imaginary                       /* access the state values        */
  use strict arg divisor                      /* get the operand                */

  if divisor~isa(.string) then                /* if just a simple number,       */
    divisor = self~class~new(divisor)         /* convert to complex             */

  a=real                                      /* get real and imaginaries for   */
  b=imaginary                                 /* both numbers                   */
  c=divisor~real
  d=divisor~imaginary
  qr=((b*d)+(a*c))%(c**2+d**2)                /* generate the new result values */
  qi=((b*c)-(a*d))%(c**2+d**2)
  return self~class~new(qr,qi)                /* return the new value           */

::method '//'                                 /* remainder method               */
  expose real imaginary                       /* access the state values        */
  use strict arg divisor                      /* get the operand                */

  if divisor~isa(.string) then                /* if just a simple number,       */
    divisor = self~class~new(divisor)         /* convert to complex             */

  intdiv = self%divisor                       /* do the integer division        */

  return self - (intdiv * divisor)            /* remultiply and subtract        */

::method string                               /* format as a string value       */
  expose real imaginary                       /* get the state info             */
  -- format as real+/-imaginaryi, but use short format if possible
  select
    when imaginary = 0 then return real
    when real = 0, imaginary = -1 then return "-i"
    when real = 0, imaginary = 1 then return "i"
    when real = 0 then return imaginary"i"
    when imaginary = 1 then return real"+i"
    when imaginary = -1 then return real"-i"
    when imaginary > 0 then return real"+"imaginary"i"
    when imaginary < 0 then return real || imaginary"i"
  end

-- Technically, complex numbers are not mathematically orderable.  This is considered
-- a "lexical" ordering, which is still a useful operation and demonstrates the use
-- of the orderable mixin class.  Ordering is defined by first comparing the real parts,
-- and if they are equal, then compare the imaginary parts
::method compareTo
  expose real imaginary
  use strict arg other

  -- we need this to be a numeric comparison, so subtract other from
  -- ourself and check the resulting real and imaginary parts
  diff = self - other
  -- the sign of the real part gives us the result we want
  res = diff~real~sign
  -- if the real part is zero, then use the imaginary sign
  if res = 0 then return diff~imaginary~sign
  return res

::class "Vector" subclass complex public      /* vector subclass of complex     */
-- NOTE:  This inherits the "[]" and init methods from the parent...nothing additional
-- required

::method string                               /* format as a string value       */
-- The subclass cannot access the real and imaginary object variables directly,
-- so it uses the attribute methods to get the values
  return '('self~real','self~imaginary')'     /* always format as '(a,b)'       */

                                              /* class for adding generalized   */
                                              /* string support to an object    */
::class "Stringlike" PUBLIC MIXINCLASS object

-- This unknown method forwards all method invocations to the object's string value,
-- effectively adding all of the string methods to the class
::method unknown UNGUARDED                    /* create an unknown method       */
  use arg msgname, args                       /* get the message and arguments  */
                                              /* just forward to the string val.*/
  forward to(self~string) message(msgname) arguments(args)

::method makestring                           /* add MAKESTRING capability      */
  return self~string                          /* return the string value        */

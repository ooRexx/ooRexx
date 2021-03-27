#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/*  usecomp.rex         Open Object Rexx Samples                              */
/*                                                                            */
/*  A simple demonstration of the complex number class                        */
/*                                                                            */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates use of the ::requires directive using the       */
/*  complex number class included in the samples.                             */
/******************************************************************************/

Say 'A simple example of complex number arithmetic:'
comp1 = .complex~new(-6,4)                  /* create two complex numbers     */
comp2 = .complex[4,1]                       /* alternate syntax               */
comp3 = .complex[-8]                        /* imaginary part omitted         */
comp4 = .complex[, -1]                      /* real part omitted              */

/* Note that SAY uses the STRING method of the COMPLEX class for display      */
say '-('comp1') is' (-comp1)
say '('comp2')~abs is' comp2~abs
say '('comp1') + ('comp2') is' comp1+comp2
say '('comp1') - ('comp2') is' comp1-comp2
say '('comp1') * ('comp2') is' comp1*comp2
say '('comp1') / ('comp2') is' comp1/comp2

/* Note that comparisons and SORT use the COMPARETO method for ordering       */
say '('comp1') > ('comp2') is' (comp1>comp2)
say '.Array~of(('comp1'), ('comp2'), ('comp3'), ('comp4'))~sort[1] is' -
    .Array~of(comp1, comp2, comp3, comp4)~sort[1]

::REQUIRES 'complex.cls'

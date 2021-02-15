#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/*  greply.rex          Open Object Rexx Samples                              */
/*                                                                            */
/*  Using the GUARD instruction to control methods                            */
/*                                                                            */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates the difference between Guarded and UnGuarded    */
/*  methods.  In the first example, the method is guarded, so it does not     */
/*  begin execution until the final result is tallied.  In the second         */
/*  example, the method is unguarded so it can begin execution while method   */
/*  sum_up is still looping.  In fact, unguarded_get often runs immediately   */
/*  after the Reply, so we use a guard instruction to ensure sum_up runs for  */
/*  a bit before unguarded_get returns with the intermediate sum.             */
/******************************************************************************/

/* guarded_get will wait until sum_up has completed                           */
Say 'Using GUARDED method to wait until sum is complete:'
Say 'Result should be 12502500:' .counter~new~~sum_up(5000)~guarded_get
Say ''

/* unguarded_get will begin execution before sum_up has completed             */
Say 'Using UNGUARDED method to obtain an intermediate result:'
Say 'Result should be less than 12502500:' .counter~new~~sum_up(5000)~unguarded_get
Exit


::CLASS counter
::Method sum_up
  Expose o_var
  o_var = 0                                 /* Initialize our counter         */
  Reply o_var                               /* Early reply so others may run  */
  Do i = 1 to arg(1)                        /* Loop here for a bit            */
    o_var = o_var + i
                                            /* Let them know we're still here */
    If i//1000 = 0 Then Say 'Inside method sum_up:  loop iteration' i
  End

::Method guarded_get                        /* GUARD is default               */
  Expose o_var
  Return o_var

::Method unguarded_get unguarded
  Expose o_var
  Guard off when o_var > 9999               /* Wait until count progresses a bit */
  Return o_var                              /* Return intermediate result     */

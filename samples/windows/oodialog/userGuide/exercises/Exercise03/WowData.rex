/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
/* ooDialog User Guide
   Exercise03 Part 2: The WowData component.			  v01-00 31May12

   Contains:       Classes: WowData.

   Pre-requisites: None.

   Description:    The data component of the "Words of Wisdom" app.

   Changes:
     v01-00 31May12: First version.

------------------------------------------------------------------------------*/

/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  WowData							  v01-00 31May12
  -------
  Has access to WOW data, and returns a set to requester.

  Potential enhancement: Define the size of a set through configuration.

  Changes:
    v01-01: First version.
  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

::CLASS WowData PUBLIC

  /*----------------------------------------------------------------------------
    activate - reads initial Wow Set from disk (but not in this version)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD activate
    expose arrWow
    arrWow = .array~new
    arrWow[1] = "Agnes Allen's Law:" -
	      	"Almost anything is easier to get into than out of."
    arrWow[2] = "Airplane Law:" -
    		"When the plane you are on is late," -
    		"the plane you want to transfer to is on time."
    arrWow[3] = "Fourteenth Corollary of Atwood's General Law of Dynamic Negatives:" -
  	      "No books are lost by loaning" -
  	      "except those you particularly wanted to keep."
    arrWow[4] = "Baker's Byroad: When you're over the hill, you pick up speed."
    arrWow[5] = "First Law of Bicycling:" -
  	      "No matter which way you ride, it's uphill and against the wind."
    arrWow[6] = "Brooks's Law:" -
  	      "Adding manpower to a late software project makes it later."
    arrWow[7] = "Grossman's Misquote of H. L. Mencken:" -
  	      "Complex problems have simple, easy-to-understand wrong answers."
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    readWowSet - returns the Wow set as an array.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD readWowSet
    expose arrWow
    return arrWow
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/

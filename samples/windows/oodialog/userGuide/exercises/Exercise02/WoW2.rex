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
   Exercise 02: Making the Controls Work	 		  v01-00 31May12

   File Contents:   class "WordsOfWisdom2"
   Pre-requisites:  None.
   Description:     Simple Words of Wisdom app - displays hard-coded 'words of
                    wisdom' in a static text control.

   Changes:
   v01-00: First version.

------------------------------------------------------------------------------*/

dlg = .WordsOfWisdom2~new
dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

::REQUIRES "ooDialog.cls"

/*---------------------------------------------------------------------------*/
::CLASS 'WordsOfWisdom2' SUBCLASS UserDialog

  ::METHOD init
    forward class (super) continue
    self~create(30, 30, 257, 123, "Words of Wisdom 2", "CENTER")


  ::METHOD defineDialog		-- Invoked automatically by ooDialog.
    self~createPushButton(901, 142, 99, 50, 14, "DEFAULT", "More wisdom", OkClicked)
    self~createPushButton(IDCANCEL, 197, 99, 50, 14, ,"Cancel")
    self~createStaticText(101, 40, 40, 200, 40, , "Click 'More wisdom'")

  ::METHOD okClicked
    arrWow = .array~new

    arrWow[1] = "Agnes Allen's Law:"||.endofline|| -
	        "Almost anything is easier to get into than out of."
    arrWow[2] = "Airplane Law:"||.endofline||"When the plane you are on is late," -
                ||.endofline||"the plane you want to transfer to is on time."
    arrWow[3] = "Fourteenth Corollary of Atwood's General Law of Dynamic Negatives:" -
                ||.endofline||"No books are lost by loaning"||.endofline|| -
  	        "except those you particularly wanted to keep."
    arrWow[4] = "Baker's Byroad:"||.endofline||"When you're over the hill, you pick up speed."
    arrWow[5] = "First Law of Bicycling:"||.endofline|| -
                "No matter which way you ride, it's uphill and against the wind."
    arrWow[6] = "Brooks's Law:"||.endofline|| -
                "Adding manpower to a late software project makes it later."
    arrWow[7] = "Grossman's Misquote of H. L. Mencken"||.endofline|| -
                "Complex problems have simple, easy-to-understand wrong answers."

    say "'More wisdom' button clicked"

    newText = self~newStatic(101)
    i = random(1,7)
    newText~setText(arrWow[i])
    return

/*----------------------------------------------------------------------------*/


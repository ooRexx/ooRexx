/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2012 Rexx Language Association. All rights reserved.    */
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
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* ooDialog\Samples\oopet.rex     Animals Riddle (bitmaps from files)       */
/*                                                                          */
/*--------------------------------------------------------------------------*/

 -- Use the global .constDir for symbolic IDs
 .application~useGlobalConstDir('O')
 .constDir[IDD_PET_DLG      ]  = 103
 .constDir[IDC_BMP_RHINO    ]  = 2001
 .constDir[IDC_BMP_TIGER    ]  = 2002
 .constDir[IDC_BMP_ELEPH    ]  = 2003
 .constDir[IDC_BMP_MOOSE    ]  = 2004
 .constDir[IDC_BMP_GOAT     ]  = 2005
 .constDir[IDC_BMP_CHIHUAHUA]  = 2006
 .constDir[IDC_BMP_SEAL     ]  = 2007
 .constDir[IDC_BMP_HORSE    ]  = 2008
 .constDir[IDC_EDIT_RHINO    ] = 1001
 .constDir[IDC_EDIT_TIGER    ] = 1002
 .constDir[IDC_EDIT_ELEPH    ] = 1003
 .constDir[IDC_EDIT_MOOSE    ] = 1004
 .constDir[IDC_EDIT_GOAT     ] = 1005
 .constDir[IDC_EDIT_CHIHUAHUA] = 1006
 .constDir[IDC_EDIT_SEAL     ] = 1007
 .constDir[IDC_EDIT_HORSE    ] = 1008

 -- A directory manager saves the current directory and can later go back to
 -- that directory.  It also sets up the environment we need.  The class
 -- itself is located in samplesSetup.rex
 mgr = .DirectoryManager~new()

 first = .constDir[IDC_EDIT_RHINO]
 last  = .constDir[IDC_EDIT_HORSE]
 do i = first to last
    b.i = "unknown animal"
 end

 dlg = .PetDialog~new("res\OOPet.DLL", IDD_PET_DLG, b., first, last)
 if dlg~initCode \= 0 then exit
 dlg~execute("SHOWTOP")
 mgr~goBack
 return

/*------------------------------- requires ---------------------------*/

::requires "ooDialog.cls"
::requires "samplesSetup.rex"

/*------------------------------- dialog class -----------------------*/

::class 'PetDialog' subclass Resdialog

::method init
  expose first last
  use arg resFile, id, dataStem., first, last

  self~init:super(resFile, id, dataStem.)

::method initDialog
   expose correct beenHelped

   ret = Play("guess.wav", yes)

   self~installBitmapButton(IDC_BMP_RHINO    , "IDRHINO",     "bmp\rhinoce.bmp" , ,                  , , "FRAME STRETCH")
   self~installBitmapButton(IDC_BMP_TIGER    , "IDTIGER",     "bmp\tiger.bmp"   , ,                  , , "FRAME USEPAL STRETCH")
   self~installBitmapButton(IDC_BMP_ELEPH    , "IDELEPH",     "bmp\eleph2.bmp"  , ,                  , , "FRAME STRETCH" )
   self~installBitmapButton(IDC_BMP_MOOSE    , "IDMOOSE",     "bmp\moose.bmp"   , ,                  , , "FRAME STRETCH")
   self~installBitmapButton(IDC_BMP_GOAT     , "IDGOAT",      "bmp\goat.bmp"    , ,                  , , "FRAME STRETCH")
   self~installBitmapButton(IDC_BMP_CHIHUAHUA, "IDCHIHUAHUA", "bmp\chihuahu.bmp", , "bmp\kanguru.bmp", , "FRAME STRETCH")
   self~installBitmapButton(IDC_BMP_SEAl     , "IDSEA",       "bmp\sealion.bmp" , ,                  , , "FRAME STRETCH")
   self~installBitmapButton(IDC_BMP_HORSE    , "IDHORSE",     "bmp\horse.bmp"   , ,                  , , "FRAME STRETCH")

   correct = .array~of("rhinoceros", "tiger", "elephant", "moose", "goat", "chihuahua", "seal", "horse")
   beenHelped = .false

::method validate
   expose correct beenHelped first last

   -- Disable the Ok button until we are through.
   self~newPushButton(IDOK)~disable

   self~getDataStem(A.)
   wrongstr = ''

   do i = first to last
      if \ A.i~strip~caseLessEquals(correct[i - 1000]) then do
         wrongstr ||= '09'x || i-1000": "A.i || .endOfLine
      end
   end
   if wrongstr = '' then do
      if \ beenHelped then ret = Play("clap.wav")

      ret = Play("yourgood.wav", "YES")

      if beenHelped then
         ret = TimedMessage("You got them all right.... with my help ","E N D",2000)
      else
        ret = TimedMessage("You got them all right","B R A V O",2000)
      return .true
   end
   else do
      ret = Play("nope.wav")

      msg = "The following answer(s) is/are incorrect:" || .endOfLine~copies(2) || wrongstr
      title = "Incorrect Answers for the ooRexx Animal Game"
      ret = MessageDialog(msg, self~hwnd, title, 'OK', 'WARNING', 'DEFBUTTON1')

      -- We are through
      self~newPushButton(IDOK)~enable

      return .false
   end

::method IDTIGER
   ret = Play("TIGER.WAV","YES")
   ret = TimedMessage("Hold that t...., hold that .i...", "A song about me", 2000)

::method IDELEPH
   ret = Play("ELEPHANT.WAV","YES")
   ret = TimedMessage("I blow my nose like a trumpet", "African Heavy Weight", 2000)

::method IDMOOSE
   ret = Play("MOOSE.WAV","YES")
   ret = TimedMessage("My name rhymes with a sweet brown dessert", "Chocolate ......", 2000)

::method IDRHINO
   ret = Play("RHINO.WAV","YES")
   ret = TimedMessage("I only fear the 2 guys on my right", "I am strong", 2000)

::method IDGOAT
   ret = Play("GOAT.WAV","YES")
   ret = TimedMessage("My relatives climb the Matterhorn", "Mountain ....", 2000)

::method IDCHIHUAHUA
   ret = Play("NOPE.WAV","YES")

::method IDSEA
   ret = Play("SEALION.WAV","YES")
   ret = TimedMessage("I am slick in the water","Hint 4 you",2000)

::method IDHORSE
   ret = Play("HORSE.WAV","YES")
   ret = infoDialog("My son won the Kentucky Derby")

::method help
   expose correct beenHelped first last

   -- Disable the help button so that it can not be clicked until we are through
   -- here.
   self~newPushButton(IDHELP)~disable

   beenHelped = .true
   ret = Play("help.wav")

   do i = first to last
      A.i = correct[i-1000]
   end

   self~setDataStem(A.)

   -- Now enable Help again.
   self~newPushButton(IDHELP)~enable

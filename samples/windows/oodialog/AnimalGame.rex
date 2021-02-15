/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

/**
 * samples\oodialog\AnimalGame.rex    Animal Riddles (bitmaps come from a DLL.)
 *
 * This example is based on the original oopet1.rex example.  However, rather
 * than use the installBitmapButton() method it uses a Windows feature that
 * allows the programmer to assign an image list to a button.  The images in the
 * image list are bitmaps that the operating system uses to paint the button
 * face.
 *
 * This is a better way to get the same effect as installBitmapButton() because
 * the OS draws the button with the same look and feel of other buttons.  On
 * Windows 7, the buttons look like Windows 7 buttons.  Whereas with
 * installBitmapButton() the buttons look like Windows 95 buttons.
 *
 * Note: this program uses the public routine, locate(), to get the full path
 * name to the directory this source code file is located. In places, the
 * variable holding this value has been callously abbreviated to 'sd' which
 * stands for source directory.
 *
 */

 -- Ensure this program can be executed from any directory.
 prgDir = locate()

 -- Use the global .constDir for symbolic IDs and the symbol definitions are in
 -- AnimalGame.h
 .application~useGlobalConstDir('O', prgDir'rc\AnimalGame.h')

 firstEditID = .constDir[IDC_EDIT_RHINO]
 lastEditID  = .constDir[IDC_EDIT_HORSE]
 do i = firstEditID to lastEditID
    b.i = "unknown animal"
 end

 dlg = .AnimalDialog~new(prgDir"res\AnimalGame.dll", IDD_ANIMALS, b., firstEditID, lastEditID)

 if dlg~initCode \= 0 then do
   msg   = 'Failed to create dialog, aborting'
   title = 'Unexpected Error'
   ret = MessageDialog(msg, , title, 'OK', 'WARNING')
   mgr~goBack
   return 99
 end

 dlg~execute("SHOWTOP")

 return 0

/*------------------------------- requires ---------------------------*/

::requires "ooDialog.cls"
::requires "samplesSetup.rex"  -- Sets up the sound path

/*------------------------------- dialog class -----------------------*/

::class 'AnimalDialog' subclass Resdialog

-- This constant is the diffence between the firstEditID edit control resource ID
-- and 1.
::constant EDIT_CONTROL_ID_OFFSET   2020

-- The pixel width and height of our bitmaps.
::constant BITMAP_WIDTH              152
::constant BITMAP_HEIGHT             178

::method init
  expose firstEditID lastEditID bitmaps buttonIDs
  use arg resFile, id, dataStem., firstEditID, lastEditID

  self~init:super(resFile, id, dataStem.)

  imageIDs = .array~of(IDBMP_RHINO, IDBMP_TIGER, IDBMP_ELEPHANT, IDBMP_MOOSE,  -
                       IDBMP_GOAT,  IDBMP_DOG,   IDBMP_SEAL,     IDBMP_HORSE,  -
                       IDBMP_KANGURU)

  buttonIDs = .array~of(IDC_PB_RHINO, IDC_PB_TIGER, IDC_PB_ELEPHANT, IDC_PB_MOOSE,  -
                        IDC_PB_GOAT,  IDC_PB_DOG,   IDC_PB_SEAL,     IDC_PB_HORSE)

  -- Load all our bitmaps from a resource only DLL.  Since the .Size argument is
  -- omitted from the getImages() method, the operating system uses the actual
  -- size of the bitmap.
  sd = locate()
  res = .ResourceImage~new(sd'res\AnimalGame.dll')
  bitmaps = res~getImages(imageIDs)

  -- Connect each button to a method with the same name as the symbolic ID of
  -- the button
  do i = 1 to buttonIDs~items
    self~connectButtonEvent(buttonIDs[i], 'CLICKED', buttonIDs[i])
  end


::method initDialog
   expose correct beenHelped bitmaps buttonIDs

   ret = Play("guess.wav", yes)

   self~configButtons(bitmaps, buttonIDs)

   correct = .array~of("rhinoceros", "tiger", "elephant", "moose", "goat", "chihuahua", "seal", "horse")
   beenHelped = .false


::method configButtons private
  use strict arg bitmaps, buttonIDs

  size   = .Size~new(self~BITMAP_WIDTH, self~BITMAP_HEIGHT)
  flags  = 'COLOR8 MASK'
  margin = .Rect~new(2)
  cRef   = .Image~colorRef(255, 255, 153)

  do i = 1 to buttonIDs~items
    if i = 6 then do
      il = .ImageList~create(size, flags, 6)
      normal    = bitMaps[i]
      press     = bitMaps[9]
      whiteCRef = .Image~colorRef(255, 255, 255)

      ret = il~addMasked(normal, cRef)
      ret = il~addMasked(normal, cRef)
      ret = il~addMasked(press, whiteCRef)
      ret = il~addMasked(normal, cRef)
      ret = il~addMasked(normal, cRef)

      self~newPushButton(buttonIDs[i])~setImageList(il, margin)
    end
    else do
      il = .ImageList~create(size, flags, 1)
      ret = il~addMasked(bitmaps[i], cRef)
      self~newPushButton(buttonIDs[i])~setImageList(il, margin)
    end
  end


::method validate
   expose correct beenHelped firstEditID lastEditID

   -- Get the edit control ID offset:
   eos = self~EDIT_CONTROL_ID_OFFSET

   -- Disable the Ok button until we are through.
   self~newPushButton(IDOK)~disable

   self~getDataStem(A.)
   wrongstr = ''

   do i = firstEditID to lastEditID
      if \ A.i~strip~caseLessEquals(correct[i - eos]) then do
         wrongstr ||= '09'x || i - eos": "A.i || .endOfLine
      end
   end
   if wrongstr = '' then do
      if \ beenHelped then ret = Play("clap.wav")

      ret = Play("yourgood.wav", "YES")

      -- Put the timed message in the center of the dialog.
      p = self~getRealPos
      s = self~getRealSize
      p~incr(s~width % 2, s~height % 2)

      if beenHelped then
         ret = TimedMessage("You got them all right.... with my help ","E N D", 3000, , p)
      else
        ret = TimedMessage("You got them all right","B R A V O", 3000, , p)
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


::method IDC_PB_RHINO unguarded
   ret = Play("RHINO.WAV","YES")
   return self~giveHint("I only fear the 2 guys on my right", "I am strong", 2000)


::method IDC_PB_TIGER unguarded
   ret = Play("TIGER.WAV","YES")
   return self~giveHint("Hold that t...., hold that .i...", "A song about me", 2000)


::method IDC_PB_ELEPHANT unguarded
   ret = Play("ELEPHANT.WAV","YES")
   return self~giveHint("I blow my nose like a trumpet", "African Heavy Weight", 2000)


::method IDC_PB_MOOSE unguarded
   ret = Play("MOOSE.WAV","YES")
   return self~giveHint("My name rhymes with a sweet brown dessert", "Chocolate ......", 2000)


::method IDC_PB_GOAT unguarded
   ret = Play("GOAT.WAV","YES")
   return self~giveHint("My relatives climb the Matterhorn", "Mountain ....", 2000)


::method IDC_PB_DOG unguarded
   ret = Play("TAKE.WAV","YES")


::method IDC_PB_SEAL unguarded
   ret = Play("SEALION.WAV","YES")
   return self~giveHint("I am slick in the water","Hint 4 you",2000)


::method IDC_PB_HORSE unguarded
   ret = Play("HORSE.WAV","YES")
   return self~giveHint("My son won the Kentucky Derby", "Giddyup 'N Go Pony", 2000)


::method giveHint unguarded private
   use strict arg msg, title, ms

   pos = .Mouse~new(self)~getCursorPos
   pos~incr(10, 10)
   ret = TimedMessage(msg, title, ms, .true, pos)
   return 0


::method help unguarded
   expose correct beenHelped firstEditID lastEditID

   -- Disable the help button so that it can not be clicked until we are through
   -- here.
   self~newPushButton(IDHELP)~disable

   beenHelped = .true
   ret = Play("help.wav")

   -- Get the edit control ID offset:
   eos = self~EDIT_CONTROL_ID_OFFSET

   do i = firstEditID to lastEditID
      A.i = correct[i - eos]
   end

   self~setDataStem(A.)

   -- Now enable Help again.
   self~newPushButton(IDHELP)~enable



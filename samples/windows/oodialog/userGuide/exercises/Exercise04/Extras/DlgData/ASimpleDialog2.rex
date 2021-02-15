/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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

/* ooDialog User Guide
   Samples\DlgData
   ASimpleDialog2.rex 	v01-00 12JLY11

   A Simple Dialog 2 - identical to ASimpleDialog.rex except that:
   - this is a subclass of ResDialog
   - uses .Application to specify the *.h file.

   This Dialog is intended to illustrate the use of DlgData. It is a very simple
   dialog that displays some text, allows the user to change the text and to
   indicate agreement or disagreement with the text. On pressing OK or Cancel,
   a messagebox provides a response.

   Associated files: ASimpleDialog.dll  ASimpleDialog.h
------------------------------------------------------------------------------*/

-- (0) Add the symbolic IDs in ASimpleDialog.h file to GlobalConstDir:
.Application~useGlobalConstDir("O", "ASimpleDialog.h")

-- (1) Set text in the edit control:
statement = "It's a fine day today."
dlgData.IDC_EDIT1 = statement

-- (2a) Create the dialog defined by the .rc file:
dlg = .ASimpleDialog~new("res\ASimpleDialog.dll", IDD_DIALOG1, dlgData.)

-- (2b) Display the dialog:
ret = dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

-- (3) When the dialog is closed, and if the user pressed OK, then retrieve
--     the data provided by the user:
if ret == 1 then do			-- if the user pressed OK
  statement2 = dlgData.1002		-- get data from the edit control
  agree = dlgData.IDC_RADIO1		-- get the state of the radio buttons:
  disagree = dlgData.1004

-- (4) Set up the appropriate message to display:
  choice = .true				-- Assume user selected a radio button
  if \agree & \disagree then choice = .false	-- If neither radio button selected.
  if statement2 \= statement then -		-- If data in edit control changed
    newStmt = .true
  else newStmt = .false

  -- Set some initial values for the message:
  title  = 'Response Received'
  icon   = 'INFORMATION'

  -- Build the appropriate response message:
  msg = "Thank you for"
  if newStmt then do			-- New statement entered:
    msg = msg "your statement,"
    if \choice then do
      msg = msg "but you have not indicated whether or not you agree with it."
    end
    else do  -- a choice was made
      if agree then msg = msg "and for your agreement with it."
      else msg = msg "even though you disagree with it."
    end
  end
  else do				-- Original stament unchanged:
    if \choice then do
      msg = msg "neither agreeing nor disagreeing."
    end
    else do
      if agree then msg = msg "your agreement."
      else msg = msg "your disagreement."
    end
  end
end
else do							-- if Cancel Pressed
  title = "Dialog Canceled"
  icon =  "ERROR"
  msg = "Thank you for canceling."
end

-- (5) Display a message to respond to the user's choices:
ret = MessageDialog(msg, 0, title, 'OK', icon, 'SETFOREGROUND')


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::CLASS ASimpleDialog SUBCLASS ResDialog

/*============================================================================*/


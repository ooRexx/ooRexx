/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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

/**
 * Simple Dialog showing how to: create a font. change the font of a control,
 * manage buttons to prevent the user from getting the dialog in the wrong
 * state.
 *
 * In this simple dialog, we don't want to manage the creation of lots of fonts,
 * so after the user creates the first new font, we disable the button.  That
 * way the user can not create a new font thousands of times.
 */

  dlg = .FileViewDialog~new
  if dlg~initCode <> 0 then do
    say "Dialog initialization failed.  Program Abort."
    return 99
  end

  dlg~create(30, 30, 260, 180, "File Viewing Dialog", "VISIBLE")
  dlg~execute("SHOWTOP")

return 0
-- End of entry point.

::requires "ooDialog.cls"

::class 'FileViewDialog' subclass UserDialog

::method defineDialog

  self~autoDetect = .false
  self~constDir[IDC_EDIT] = 110
  self~constDir[IDC_PB_OPEN] = 111
  self~constDir[IDC_PB_CHANGE_FONT] = 112

  styles = "VSCROLL HSCROLL MULTILINE"
  self~createEdit(IDC_EDIT, 5, 5, 250, 150, styles)

  self~createPushButton(IDC_PB_OPEN,          5, 160, 35, 15, ,          "Open", onOpen)
  self~createPushButton(IDC_PB_CHANGE_FONT,  50, 160, 45, 15, ,          "Change Font", onFont)
  self~createPushButton(IDOK,               220, 160, 35, 15, "DEFAULT", "OK")

::method initDialog
  expose editControl newFont

  newFont = .nil
  editControl = self~newEdit(IDC_EDIT)

  helpMsg = "Use the Open button to open a text file" || .endOfLine || -
            "and then use the Change Font button to"  || .endOfLine || -
            "change the font."

  editControl~setText(helpMsg)
  self~newPushButton(IDC_PB_CHANGE_FONT)~disable

::method onOpen
  expose editControl newFont

  editControl~setText("")

  fileName = fileNameDialog( , self~dlgHandle, , 1, "Open a File for Editing")
  if fileName == "" then return

  fObj = .stream~new(fileName)
  fObj~open
  if fObj~state \== 'READY' then return self~abort

  text = fObj~charin(1, fObj~chars)
  fObj~close

  editControl~setText(text)
  if newFont == .nil then self~newPushButton(IDC_PB_CHANGE_FONT)~enable

::method onFont
  expose editControl newFont

  newFont = self~createFontEx("Courier", 12)
  editControl~setFont(newFont)
  self~newPushButton(IDC_PB_CHANGE_FONT)~disable

::method cancel
  expose newFont

  if newFont \= .nil then self~deleteFont(newFont)
  self~cancel:super

::method ok
  expose newFont

  if newFont \= .nil then self~deleteFont(newFont)
  self~ok:super

::method abort private
  j = errorDialog("There is an unexplained error.  Program Abort")
  return self~cancel

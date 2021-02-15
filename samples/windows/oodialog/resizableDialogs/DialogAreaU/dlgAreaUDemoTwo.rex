/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2014 Rexx Language Association. All rights reserved.    */
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
 *  DlgAreaDemoTwo.Rex
 *
 *  Demonstrates a second approach to resizable dialogs.  Essentially what this
 *  approach does is to defer the redrawing of the dialog controls until the
 *  user has finished resizing the dialog.
 *
 *  By default, the DialogAreaU class calls a dialog method (the update method)
 *  that forces all the dialog controls to redraw themselves every time a resize
 *  event ocurrs.  This causes the constant flicker seen in DlgAreaDemo.rex.
 *
 *  The approach taken here is to tell the DialogAreaU object to *not* invoke
 *  the update method during the resize event.  This is done by setting the
 *  updateOnResize attribute to false in the DialogAreaU object.
 *
 *  Then we connect the size / move ended event to a method in our dialog.  This
 *  event is called exactly once when the user has stopped resizing or moving
 *  the dialog.
 *
 *  We keep track of whether the user is resizing, or not.  When we get the size
 *  move ended event, if the user was resizing, we invoke the redrawClient
 *  method of the dialog, forcing all the dialog controls to redraw themselves
 *  in their new, final, position.
 *
 *  This eliminates the flicker, but also makes it appear as though the dialog
 *  controls are not changing while the user is actively resizing.  When the
 *  user stops resizing, the dialog controls "magically" appear in their new
 *  size and position.
 *
 *  Which approach is better is probably a matter of personal preference.
 *
 *  After some use of this example program, a refinement to the above algorithm
 *  was made.  This is what is used now.  Since no repainting of the dialog
 *  controls is done until the last resize event, there is no sense in re-
 *  calculating the size and position of each control on every resize event.
 *
 *  Rather than invoke u~resize(self, sizeinfo) every single time in onResize()
 *  u~resize is only invoked when the dialog client area is going to be forced
 *  to redraw.
 */

  sd = locate()
  .application~useGlobalConstDir("O", sd'dlgAreaUDemo.h')

  dlg = .ResizableDialog~new
  dlg~execute('ShowTop')

  return 0

::requires "ooDialog.cls"

::class 'ResizableDialog' subclass UserDialog

::method init

  forward class (super) continue
  success = self~createCenter(250, 250, 'My Flicker Free Resizable Dialog',     -
                                        'ThickFrame MinimizeBox MaximizeBox', , -
                                        'MS Sans Serif', 8)
  if \ success then do
    self~initCode = 1
    return
  end

  self~connectResize('onResize', .true)
  self~connectSizeMoveEnded('onSizeMoveEnded')


::method defineDialog
  expose u sizing minMaximized

  u = .dlgAreaU~new(self)
  if u~lastError \= .nil then call errorDialog u~lastError

  -- Tell the DialogAreaU object to not invoke the update method.
  u~updateOnResize = .false

  -- We use these variables to track when to redraw, or not.
  sizing = .false
  minMaximized = .false

  u~noResizePut(IDC_PB_0)
  e = .dlgArea~new(u~x       , u~y       , u~w('70%'), u~h('90%'))   -- edit   area
  s = .dlgArea~new(u~x       , u~y('90%'), u~w('70%'), u~hr      )   -- status area
  b = .dlgArea~new(u~x('70%'), u~y       , u~wr      , u~hr      )   -- button area

  self~createEdit(IDC_EDIT, e~x, e~y, e~w, e~h, 'multiline')
  self~createStaticText(IDC_ST_STATUS, s~x, s~y, s~w, s~h, , 'Status info appears here')

  self~createPushButton(IDC_PB_0, b~x, b~y('00%'), b~w, b~h('9%'),          , 'Button' 0 , 'Button'||0)
  self~createPushButton(IDC_PB_1, b~x, b~y('10%'), b~w, b~h('9%'),          , 'Button' 1 , 'Button'||1)
  self~createPushButton(IDC_PB_2, b~x, b~y('20%'), b~w, b~h('9%'),          , 'Button' 2 , 'Button'||2)
  self~createPushButton(IDC_PB_3, b~x, b~y('30%'), b~w, b~h('9%'),          , 'Button' 3 , 'Button'||3)
  self~createPushButton(IDC_PB_4, b~x, b~y('40%'), b~w, b~h('9%'),          , 'Button' 4 , 'Button'||4)
  self~createPushButton(IDC_PB_5, b~x, b~y('50%'), b~w, b~h('9%'),          , 'Button' 5 , 'Button'||5)
  self~createPushButton(IDC_PB_6, b~x, b~y('60%'), b~w, b~h('9%'),          , 'Button' 6 , 'Button'||6)
  self~createPushButton(IDOK,     b~x, b~y('90%'), b~w, b~h('9%'), 'DEFAULT', 'Ok')


::method initDialog

  -- The underlying edit controls internally resize themselves as the dialog
  -- they are contained in is resized.  We don't want that, so we disable that
  -- behavior in the underlying edit control.
  self~newEdit(IDC_EDIT)~disableInternalResize


::method onResize unguarded
  expose u sizing minMaximized lastSizeInfo
  use arg sizingType, sizeinfo

  -- Save the size information so we know the final size of the dialog.
  lastSizeInfo = sizeInfo

  -- The size / move ended event does not occur when the user maximizes,
  -- minimizes, or restores from maximized / minimized.  Because of that, we
  -- need to redraw the client area under those conditions.

  if sizingType == self~SIZE_MAXIMIZED | sizingType == self~SIZE_MINIMIZED then do
    minMaximized = .true
    if sizingType == self~SIZE_MAXIMIZED then do
      u~resize(self, sizeinfo)
      self~redrawClient(.true)
    end
  end
  else if sizingType == self~SIZE_RESTORED, minMaximized then do
    minMaximized = .false
    u~resize(self, sizeinfo)
    self~redrawClient(.true)
  end
  else do
    -- We are resizing now.
    sizing = .true
  end

  return 0


::method onSizeMoveEnded unguarded
  expose u sizing lastSizeInfo

  -- If we were resizing, force the dialog controls to redraw themselves.
  if sizing then do
    u~resize(self, lastSizeInfo)
    self~redrawClient(.true)
  end

  -- We are not resizing anymore.
  sizing = .false
  return 0


::method unknown
  use arg msgName, args

  if msgName~abbrev("BUTTON") then
    self~newStatic(IDC_ST_STATUS)~setText('You Pressed Button' msgName~right(1))

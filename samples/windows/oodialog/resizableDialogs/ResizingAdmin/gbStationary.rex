/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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
 *  The gbStationary.rex example uses a very similar dialog to the one used in
 *  the augmentedResize.rex example.  It then does a few things differently to
 *  demonstrate how some of the other methods of the ResizingAdmin class work.
 *  It also is used to show how the XCENTER and YCENTER pin types work.
 *
 *  These are some of the differences to note.
 *
 *  1.)  The 3 push buttons in the dialog use the XCENTER and YCENTER pin types.
 *  The Test push button uses the YCENTER pin type to center it vertically with
 *  the list-view control.  The Ok and Cancel push buttons use the XCENTER pin
 *  type to center themselves horizontally within the dialog window.
 *
 *  2.)  The list-view control starts in the Icon view instead of the report
 *  view.  When the dialog is resized, the list-view does not rearrange the
 *  items.  This may look "wrong" to some people.  So, this example uses that
 *  as an opportunity to show the wantSizeEnded() method.  The programmer can
 *  use that method to have the ooDialog framework invoke a method in the Rexx
 *  dialog when the user has finished resizing the dialog.  In this program, we
 *  use that notification to do a many rearranging of the list-view items.  The
 *  push button in the center right of the dialog can be used to toggle the
 *  behaviour back in forth to do the automatic rearranging or to not do the
 *  rearranging.  This allows you to see what the dialog looks like either way.
 *
 *  3.)  In the augmentedResize.rex example, the group box and the controls with
 *  in the group box keep the same height but stretch horizontally as the dialog
 *  gets wider.  In this example, the group box and the controls within it keep
 *  the same size ans the dialog changes size and they remain fixed to the left
 *  side of the dialog and the bottom of the list-view.  Since these are the
 *  majority of the controls in this dialog, the default sizing is set to those
 *  values, and the sizing for the individual controls is omitted.  This reduces
 *  the actual number of lines the programer has to type.
 *
 *  4.)  By default the minimum size of the dialog is set to its initial size
 *  when it first is created, and the maximum size is unlimited.  This example
 *  removes any minimum size restriction and sets a maximum size to slightly
 *  smaller than the screen.  Note that the ability to maximize or minimize the
 *  dialog is set by the presence or absence of the minimize or maximize buttons
 *  and has nothing to do with the minimum or maximum size.  However, if the
 *  dialog has a maximize button and a maximum size smaller than the screen is
 *  set, the maximize button will maximize the dialog to the set size.  If the
 *  dialog has a minimize button, it behaves the same with or without a minimum
 *  size set.  Note also that the operating system will not let the user size a
 *  window smaller than the height of the caption bar plus the sizing borders,
 *  or smaller than the width of the system menu icon and buttons in the caption
 *  bar, even if the size is set to 1 x 1 pixels.
 */

  sd = locate()
  .application~setDefaults('O', sd'rc\gbStationary.h', .false)
  say sd"rc\gbStationary.rc"
  dlg = .ResizableDlg~new(sd"rc\gbStationary.rc", IDD_RESIZABLE)
  dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

-- Make the dialog resizable by inheriting the ResizingAdmin class.
::class 'ResizableDlg' subclass RcDialog inherit ResizingAdmin

/** defineSizing()
 *
 * Define the sizing needed for our controls.  Please read the commets for this
 * method in the augmentedResize.rex example for a lengthy description of this
 * method.
 */
::method defineSizing

  ID_DLG = self~IDC_DEFAULT_PINTO_WINDOW

  self~controlSizing(IDC_LV_MAIN,                                          -
                     .array~of('STATIONARY', 'LEFT'),                      -
                     .array~of('STATIONARY', 'TOP'),                       -
                     .array~of('STATIONARY', 'RIGHT'),                     -
                     .array~of('STATIONARY', 'BOTTOM')                     -
                    )

  -- Note the YCENTER keyword for the top edge of the push button.  The effect
  -- of this is to center the top edge with the center of the pin to window,
  -- the list-view in this case.

  self~controlSizing(IDC_PB_TEST,                                          -
                     .array~of('STATIONARY', 'RIGHT',  IDC_LV_MAIN),       -
                     .array~of('STATIONARY', 'YCENTER', IDC_LV_MAIN),      -
                     .array~of('MYLEFT',     'LEFT'),                      -
                     .array~of('MYTOP',      'TOP')                        -
                    )

  -- Here we set the default sizing for the left top to be the left side of the
  -- dialog and the bottom of the list-view.  The default sizing for the right
  -- bottom is set to the same thing.  Every control edge not specifically
  -- assigned a value in this program will be assigned the default sizing when
  -- the underlying dialog is created.
  --
  -- The control edges not defined here are all the edges of the group box, the
  -- 3 edit controls and the 3 labels for those controls.  The effect of this
  -- then is to pin those controls to the left bottom of the list-view and to
  -- keep them fixed in size.  For this dialog, this actually makes the most
  -- sense.  At least to the author. ;-)

  self~defaultSizing(.array~of('STATIONARY', 'LEFT'),                     -
                     .array~of('STATIONARY', 'BOTTOM', IDC_LV_MAIN),      -
                     .array~of('STATIONARY', 'LEFT'),                     -
                     .array~of('STATIONARY', 'BOTTOM', IDC_LV_MAIN)       -
                    )

  -- The Ok and Cancel buttons are specified explicitly.  Note that the XCENTER
  -- edge keyword has the effect of keeping the 2 buttons centered horizonatally
  -- in the dialog.
  self~controlSizing(IDOK,                                                 -
                     .array~of('STATIONARY', 'XCENTER'),                   -
                     .array~of('STATIONARY', 'BOTTOM'),                    -
                     .array~of('MYLEFT',     'LEFT'),                      -
                     .array~of('MYTOP',      'TOP')                        -
                    )

  self~controlSizing(IDCANCEL,                                             -
                     .array~of('STATIONARY', 'XCENTER'),                   -
                     .array~of('STATIONARY', 'BOTTOM'),                    -
                     .array~of('MYLEFT',     'LEFT'),                      -
                     .array~of('MYTOP',      'TOP')                        -
                    )

  -- We specify that we want the size ended notification to be sent to us, using
  -- the method name of onSizeEnded and that we will return a value from that
  -- method.
  self~wantSizeEnded(onSizeEnded, .true)

  -- We also connect the test push button.  Then when the dialog is showing, you
  -- can toggle between rearranging the list-view item when the user has ended
  -- sizing, and not.  To see the difference.
  self~connectButtonEvent(IDC_PB_TEST, 'CLICKED', onPbPushed)

  return 0


/** initDialog()
 *
 * A pretty typical initDialog() method, we add some items to the list-view and
 * set up the initial state for the push button.
 *
 */
::method initDialog
  expose list arrangeWhenSizeEnds pushButton

  -- Set the initial state so that when a sizing operation ends, the items in
  -- the list-view are automatically rearranged.
  arrangeWhenSizeEnds = .true
  pushButton = self~newPushButton(IDC_PB_TEST)
  pushButton~setText('Do Arrange')


  -- Get the size of the work area of the screen and make our maximum size 15
  -- pixels smaller on each side. Then set the minimum size to no minimum.
  workArea = .SPI~workArea
  maxSize = .Size~new(workArea~right - workArea~left - 15, workArea~bottom - workArea~top - 15)

  self~maxSize = maxSize
  self~noMinSize

  -- Fill the list-view with items.
  list = self~newListView(IDC_LV_MAIN)

  columnNames = .array~of("Line", "Number")
  list~addExtendedStyle("FULLROWSELECT GRIDLINES CHECKBOXES HEADERDRAGDROP")

  list~InsertColumn(0, "Row (Column 1)", 95)
  list~InsertColumn(1, "Column 2", 95)
  list~InsertColumn(2, "Column 3", 95)

  do i = 1 to 200
    list~addRow(i, , "Line" i, "Column" 2, "Column" 3)
  end


/** onPbPushed()
 *
 * The button was pushed, toggle our state.
 */
::method onPbPushed unguarded
  expose arrangeWhenSizeEnds pushButton

  -- Toggle our state
  arrangeWhenSizeEnds = \ arrangeWhenSizeEnds
  if arrangeWhenSizeEnds then pushButton~setText('Do Arrange')
  else pushButton~setText('No Arrange')


/** onSizeEnded()
 *
 * The event handler for the sizing ended notification.  We look at the value of
 * the flag and either tell the list-view to rearrange its items, or not.
 *
 * Note that we must return a value, because we said we would, but the actual
 * value is ignored.  The main purpose of having the ooDialog framework wait for
 * the return is to "sync" our code execution with the actions of the user.
 */
::method onSizeEnded unguarded
  expose list arrangeWhenSizeEnds

  if arrangeWhenSizeEnds then list~arrange
  return 0

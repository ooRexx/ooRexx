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
 *  The augmentedResize.rex example has the same initial dialog layout as the
 *  basicResize.rex example.  However, to give a better appearance to the dialog
 *  as it is enlarged or shrunk, the precise way each dialog control is to be
 *  changed is defined in the program code.
 *
 *  All code that defines the sizing of the resizable dialog has to be defined
 *  in the defineSizing() method.  This method is invoked automatically by the
 *  ooDialog framework.  The default implementation of the method does nothing.
 *  The method is meant to be over-ridden by the programmer to change the
 *  default sizing parameters of the ResizingAdmin class.  The method is invoked
 *  before the underlying dialog is created.  The programmer can not inovke any
 *  method that requires the underlying dialog to exist from the defineSizing()
 *  method.
 */

    -- Get the directory our source code files are located in.
    sd = locate()

    --  Use the global .constDir, only, for symbolic IDs, load the symbols from
    --  the basicResize.h file.
    .application~setDefaults('O', sd'rc\basicResize.h', .false)

    dlg = .ResizableDlg~new(sd"rc\basicResize.rc", IDD_RESIZABLE)
    dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

-- Inherit the ResizingAdmin class to make resizing available.
::class 'ResizableDlg' subclass RcDialog inherit ResizingAdmin

/** defineSizing()
 *
 * This method is invoked automatically by the ooDialog framework.  A default
 * implmentation is provided.  That implmentation does not do anything.  To
 * change how the default implmentation works, the program must over-ride the
 * defineSizing() method and invoke ResizingAdmin methods to change the
 * defaults.
 *
 * Each dialog control must be referenced by its resource ID.  The ID can be
 * numeric or symbolic.  This is no different than any other use of ooDialog.
 *
 * The basic premise of the resizing admin is that each side, left, top, right,
 * etc., of every control window can be "pinned" to the side of another window.
 * By default each side is pinned to a side of the dialog window itself.  But,
 * any side of a dialog control window can be pinned to a side of any other
 * dialog control in the dialog.
 *
 * There are several different "types" of pins.  The programmer defines the
 * "sizing" of any dialog control by defining how each "edge" of the control
 * should be sized.  The definition consists of: the id of the control, which
 * side of the control is to be pined, which side of another window the control
 * edge is pinned to, the type of the pin, and the ID of the other window.  For
 * instance:
 *
 * The left side of a button control could be "pinned" to the left side of the
 * dialog window using a 'proportional' pin.
 *
 *   or
 *
 * The right side of a list-view control could be "pinned" the left side of an
 * edit control using a 'stationary' pin.
 *
 * The control windows are identified by the resource ID of the control.  The
 * dialog window is identified by a special constant provided by the
 * ResizingAdmin class, the IDC_DEFAULT_PINTO_WINDOW constant.
 *
 * The edges (sides) of windows are identified by the keywords: LEFT, TOP,
 * RIGHT, and BOTTOM.  In addition there are two other edge keywords: XCENTER
 * and YCENTER.
 *
 * XCENTER specifies that the edge is pinned to the horizontal center of the
 * pinned to edge.  YCENTER specifies that the edge is pinned to the vertical
 * center of the pinned to edge.
 *
 * The types of pins are identified by the kewords: PROPORTIONAL, STATIONARY,
 * MYTOP, and MYLEFT.
 *
 * PROPORTIONAL :  The distance between the edge and the pinned to edge grows,
 *                 (or shrinks,) proportionally to the size of the pinned to
 *                 window.
 *
 * STATIONARY   :  The distance between the edge and the pinned to edge remains
 *                 equal to what it was when the dialog was initially created.
 *
 * MYLEFT       :  A special keyword, can only be used for the right edge of a
 *                 control.  Pins the right edge of the control to its left
 *                 edge.  This has the effect of keeping the width of the
 *                 control constant.
 *
 * MYTOP        :  A special keyword, can only be used for the bottom edge of a
 *                 control.  Pins the bottom edge of the control to its top
 *                 edge.  This has the effect of keeping the height of the
 *                 control constant.
 *
 * The ooDialog framework uses an algorithm to resize / reposition the dialog
 * control windows that virtually eliminates flicker.  This code requires that
 * the sizing for every control in the dialog be specified.  When the underlying
 * dialog is first created, the framework enumerates every dialog control.  If a
 * a control did not have its sizing defined by the programmer, the framework
 * assigns the default sizing to the control and adds the sizing definition to
 * the end of the sizing definitions list.
 *
 * When the sizing event notification is received, the ooDialog framework
 * resizes and repositions every dialog control in the order of the sizing
 * defintions in the list.
 *
 * Because of this, any "pinned to" window must come before the window that pins
 * to it in the list.  I.e., if a side of a list-view control is to be "pinned"
 * to the left side of an edit control, the sizing definition for the edit
 * control must be specified before the sizing definition for the list-view is
 * specified.  Sizing definitions are placed in the list in the order the
 * programmer defines the control sizing.  The sizing definitions for all
 * controls not defined by the programmer are appended to the end of the list,
 * in the order they are enumerated by the operating system.
 *
 * The code in the following defineSizing() method shows how to apply the above
 * explanation.  The ooDialog reference manual should be consulted for the
 * complete documentation on the ResizingAdmin class.
 */
::method defineSizing

    -- Assign the IDC_DEFAULT_PINTO_WINDOW constant to a short variable name.
    -- This is just done here to keep the code a little more readable.  In
    -- addition, IDC_DEFAULT_PINTO_WINDOW is the default.  In almost all cases,
    -- IDC_DEFAULT_PINTO_WINDOW can simply be omitted.

    ID_DLG = self~IDC_DEFAULT_PINTO_WINDOW

    -- The sizing for each edge of a dialog control can be specified
    -- individually.  Here we define the sizing for the list-view.  It is pinned
    -- to the left top corner and right bottom corner of the dialog.  This means
    -- the pixel distance between the edges of the list-view and the edges of
    -- the dialog will remain constant.

    self~controlLeft(IDC_LV_MAIN,   'STATIONARY', 'LEFT',   ID_DLG)
    self~controlTop(IDC_LV_MAIN,    'STATIONARY', 'TOP',    ID_DLG)
    self~controlRight(IDC_LV_MAIN,  'STATIONARY', 'RIGHT',  ID_DLG)
    self~controlBottom(IDC_LV_MAIN, 'STATIONARY', 'BOTTOM', ID_DLG)

    -- The complete sizing for a single control can be specified at one time.
    -- The first argument must be the resource ID of the control.  Following
    -- that are four arguments defining the sizing for the 4 edges of the
    -- control.  The must be in order: left top right bottom.  However, each
    -- argument is optional.  If it is omitted, the default sizing for that edge
    -- is used.
    --
    -- Each of the 4 arguments must be an array.  The fist item in the array is
    -- required and is the type of pin.  The second item in the array is also
    -- required.  It is the edge of the other window that the edge of the
    -- control is pinned to.  The third item in the array is the id of the
    -- window that the control's edge is pinned to.  This item can be omitted,
    -- it defaults to the dialog window.
    --
    -- This definition pins the Test push button to the right top corner of the
    -- dialog.  Since it is a 'stationary' pin the push button will 'stay' in
    -- place.  The right side of the button is pinned to its own left side.
    -- This makes the width of the button constant, it will not change as the
    -- dialog is sized.  The bottom edge of the button is pinned to its top.
    -- This makes the height of the button constant.
    --
    -- Note that when the pin type is MYLEFT or MYRIGHT, the other 2
    -- specifications, the edge and window, are always ignored.  However, the
    -- controlSizing() method requires that the first 2 items in the array not
    -- be omitted.
    self~controlSizing(IDC_PB_TEST,                                          -
                       .array~of('STATIONARY', 'RIGHT', IDC_LV_MAIN),        -
                       .array~of('STATIONARY', 'TOP'),                       -
                       .array~of('MYLEFT',     'LEFT'),                      -
                       .array~of('MYTOP',      'TOP')                        -
                      )

    -- The following just defines the sizing for all of the rest of the
    -- controls.

    self~controlSizing(IDC_GB_TEST,                                          -
                       .array~of('STATIONARY', 'LEFT'),                      -
                       .array~of('STATIONARY', 'BOTTOM', IDC_LV_MAIN),       -
                       .array~of('STATIONARY', 'RIGHT'),                     -
                       .array~of('MYTOP',      'TOP')                        -
                      )

    self~controlSizing(IDC_EDIT_LABELS,                                      -
                       .array~of('PROPORTIONAL', 'LEFT',  IDC_GB_TEST),      -
                       .array~of('STATIONARY',   'TOP',   IDC_GB_TEST),      -
                       .array~of('STATIONARY',   'RIGHT', IDC_GB_TEST),      -
                       .array~of('MYTOP',        'TOP')                      -
                       )

    self~controlSizing(IDC_ST_LABELS,                                        -
                       .array~of('STATIONARY', 'LEFT', IDC_EDIT_LABELS),     -
                       .array~of('STATIONARY', 'TOP',  IDC_GB_TEST),         -
                       .array~of('MYLEFT',     'LEFT'),                      -
                       .array~of('MYTOP',      'TOP')                        -
                      )

    self~controlSizing(IDC_ST_NAMES,                                         -
                       .array~of('STATIONARY', 'LEFT', IDC_GB_TEST),         -
                       .array~of('STATIONARY', 'TOP',  IDC_GB_TEST),         -
                       .array~of('MYLEFT',     'LEFT'),                      -
                       .array~of('MYTOP',      'TOP')                        -
                      )

    self~controlSizing(IDC_EDIT_NAMES,                                       -
                       .array~of('STATIONARY', 'LEFT', IDC_ST_NAMES),        -
                       .array~of('STATIONARY', 'TOP',  IDC_GB_TEST),         -
                       .array~of('STATIONARY', 'LEFT', IDC_ST_LABELS),       -
                       .array~of('MYTOP',      'TOP')                        -
                       )

    self~controlSizing(IDC_ST_PLACES,                                        -
                       .array~of('STATIONARY', 'LEFT', IDC_GB_TEST),         -
                       .array~of('STATIONARY', 'TOP',  IDC_GB_TEST),         -
                       .array~of('MYLEFT',     'LEFT'),                      -
                       .array~of('MYTOP',      'TOP')                        -
                      )

    self~controlSizing(IDC_EDIT_PLACES,                                      -
                       .array~of('STATIONARY', 'LEFT', IDC_ST_PLACES),       -
                       .array~of('STATIONARY', 'TOP',  IDC_GB_TEST),         -
                       .array~of('STATIONARY', 'LEFT', IDC_EDIT_NAMES),      -
                       .array~of('MYTOP',      'TOP')                        -
                       )

    self~controlSizing(IDOK,                                                 -
                       .array~of('STATIONARY', 'RIGHT'),                     -
                       .array~of('STATIONARY', 'BOTTOM'),                    -
                       .array~of('MYLEFT',     'LEFT'),                      -
                       .array~of('MYTOP',      'TOP')                        -
                      )

    self~controlSizing(IDCANCEL,                                             -
                       .array~of('STATIONARY', 'RIGHT'),                     -
                       .array~of('STATIONARY', 'BOTTOM'),                    -
                       .array~of('MYLEFT',     'LEFT'),                      -
                       .array~of('MYTOP',      'TOP')                        -
                      )

  return 0


/** initDialog()
 *
 * This is just a typical initDialog() method.  We merely put some items into
 * the list-view to give the dialog some texture.
 */
::method initDialog

  self~newGroupBox(IDC_GB_TEST)~setText('Group Box Stationary Left / Right')

  list = self~newListView(IDC_LV_MAIN)
  list~setView('REPORT')

  columnNames = .array~of("Line", "Number")
  list~addExtendedStyle("FULLROWSELECT GRIDLINES CHECKBOXES HEADERDRAGDROP")

  list~InsertColumn(0, "Row (Column 1)", 95)
  list~InsertColumn(1, "Column 2", 95)
  list~InsertColumn(2, "Column 3", 95)

  do i = 1 to 200
    list~addRow(i, , "Line" i, "Column" 2, "Column" 3)
  end

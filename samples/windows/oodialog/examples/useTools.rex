/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
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

/** Owned window example
 *
 * This example demonstrates how to make a dialog window owned by another dialog
 * window.  It simulates a tool palette, which is owned by the main dialog.
 *
 * Windows that are owned by another window have several constraints.  The owned
 * window is always above its owner window.  If the owner window is minimized,
 * the owned window is hidden.  When the owner window is destroyed, its owned
 * windows are also destroyed.
 *
 * The programmer creates an owned window by setting its ownerDialog attribute
 * to the owner window.  Once set, the owner window can not be changed.  The
 * ownerDialog attribute can not be set once the Windows dialog of the owned
 * windows has been created.  The underlying Windows dialog of the owner window
 * must be created before the underlying Windows dialog of the owned window is
 * created.  However, the owner Windows dialog does not have to be created yet
 * when it is assigned to the owned window.
 *
 * The owned dialog always behaves as if it were executed using the popupAsChild
 * method.  That is, it is a modeless dialog that operates indepenedently of its
 * owner dialog and it is destroyed when its owner (parent) dialog is destroyed.
 * But, the programmer does not have to use the popupAsChild method to execute
 * the owned dialog, the programmer can use any of the methods that start the
 * underlying Windows dialog.
 */

  dlg = .MainDialog~new( , "resources\useTools.h")

  dlgTool = .ToolPaletteDlg~new( , "resources\useTools.h" )
  dlgTool~ownerDialog = dlg

  dlg~executeAsync("SHOWTOP", IDI_DLG_OOREXX)

  dlgTool~popup("SHOWTOP")

  dlg~endAsyncExecution


::requires "ooDialog.cls"

::class 'MainDialog' subclass UserDialog

::method init
  forward class (super) continue

  self~create(30, 30, 466, 323, "Main Dialog", "CENTER MINIMIZEBOX MAXIMIZEBOX")

::method defineDialog

  self~createStaticText(IDC_ST_TOOL_SIMULATOR, 50, 50, 366, 223, "CENTER", "")
  self~createPushButton(IDC_PB_SHOW_PALETTE, 20, 299, 50, 14, "HIDDEN", "Show Palette", onShowPalette)
  self~createPushButton(IDOK, 342, 299, 50, 14, "DEFAULT", "Ok")
  self~createPushButton(IDCANCEL, 397, 299, 50, 14, , "Cancel")

::method initDialog
  expose font toolText

  d = .directory~new
  d~weight = 700
  d~quality = 2
  font = self~createFontEx("Arial", 40, d)

  toolText = self~newStatic(IDC_ST_TOOL_SIMULATOR)
  toolText~setFont(font)


::method onShowPalette unguarded

  self~paletteButton(.false)

  dlgTool = .ToolPaletteDlg~new( , "resources\useTools.h" )
  dlgTool~ownerDialog = self

  dlgTool~popup("SHOWTOP")


-- The leaving method is invoked automatically by ooDialog when the underlying
-- Windows dialog is being closed.  It can be used to do some final clean up,
-- such as releasing resources.  Note that in this program, it is not really
-- necessary to release the created font.  As soon as the program ends, the
-- operating system will automatically release the font resource.
::method leaving
  expose font
  self~deleteFont(font)


-- The following methods just display the tool being used to simulate the
-- tool the user selected.  A real program would need code to actually use
-- the selected tool.

::method lockModule
  expose toolText
  toolText~setText('Running lockModule tool')

::method lockProject
  expose toolText
  toolText~setText('Running lockProject tool')

::method splitModule
  expose toolText
  toolText~setText('Running splitModule tool')

::method saveModule
  expose toolText
  toolText~setText('Running saveModule tool')

::method saveProject
  expose toolText
  toolText~setText('Running saveProject tool')

::method saveAll
  expose toolText
  toolText~setText('Running saveAll tool')

::method checkIn
  expose toolText
  toolText~setText('Running checkIn tool')

::method linkToWeb
  expose toolText
  toolText~setText('Running linkToWeb tool')

::method codeReview
  expose toolText
  toolText~setText('Running codeReview tool')

::method update
  expose toolText
  toolText~setText('Running update tool')

::method projectReview
  expose toolText
  toolText~setText('Running projectReview tool')

::method closePalette
  expose toolText

  toolText~setText('')
  self~paletteButton(.true)

::method paletteButton private
  use strict arg showButton

  if showButton then self~newPushButton(IDC_PB_SHOW_PALETTE)~show
  else self~newPushButton(IDC_PB_SHOW_PALETTE)~hide


::class 'ToolPaletteDlg' subclass UserDialog

::method init
  expose s buttonIDs

  forward class (super) continue

  self~populateButtons

  -- The bitmaps are 32 x 32, we want the button to have a 4 pixel margin so
  -- that the button high-lighting for hot, pressed, default, etc., shows.
  -- Convert the size in pixels to dialog units
  s = .Size~new(40, 40)
  self~pixel2dlgUnit(s)

  count = buttonIDs~items

  -- The last button is the close button.  We palace an empty spot the width
  -- of a button between the tool buttons and the last button.
  count += 1

  -- Now size the dialog to exactly fit the buttons
  self~create(0, 30, count * s~width, s~height, "Tool Pallete", "NOMENU")

::method defineDialog
  expose s buttonIDs

  do i = 1 to buttonIDs~items
    x = (i - 1) * s~width
    if i == buttonIDs~items then x += s~width
    self~createPushButton(buttonIDs[i], x, 0, s~width, s~height, 'BITMAP', , onToolClick)
  end

::method initDialog
   expose buttonIDs buttonNames

   tempButton = self~newPushButton(buttonIDs[1])

   size = tempButton~getRealSize
   size~width  -= size~width - 32
   size~height -= size~height - 32

   flags = .DlgUtil~or(.Image~toID(ILC_COLOR24), .Image~toID(ILC_MASK))
   type  = .Image~toID(IMAGE_BITMAP)
   cRef  = .Image~colorRef(0, 0, 0)

   align  = .Image~toID(BUTTON_IMAGELIST_ALIGN_CENTER)
   margin = .Rect~new(4)

   do i = 1 to buttonIDs~items
      bmpButton = self~newPushButton(buttonIDs[i])

      image = .Image~getImage(buttonNames[i], type)
      imageList = .ImageList~create(size, flags, 1, 0)
      imageList~addMasked(image, cRef)

      bmpButton~setImageList(imageList, margin, align)
      image~release
   end


-- The event handler for the button click event.  We map which button was
-- clicked to the proper method in the parent / owner dialog and invoke that
-- method
::method onToolClick
   use arg id, hwnd

   select
      when id == self~constdir[IDC_PB_LOCKMODULE   ] then self~ownerDialog~lockModule
      when id == self~constdir[IDC_PB_LOCKPROJECT  ] then self~ownerDialog~lockProject
      when id == self~constdir[IDC_PB_SPLITMODULE  ] then self~ownerDialog~splitModule
      when id == self~constdir[IDC_PB_SAVEMODULE   ] then self~ownerDialog~saveModule
      when id == self~constdir[IDC_PB_SAVEPROJECT  ] then self~ownerDialog~saveProject
      when id == self~constdir[IDC_PB_SAVEALL      ] then self~ownerDialog~saveAll
      when id == self~constdir[IDC_PB_CHECKIN      ] then self~ownerDialog~checkIn
      when id == self~constdir[IDC_PB_LINKTOWEB    ] then self~ownerDialog~linkToWeb
      when id == self~constdir[IDC_PB_CODEREVIEW   ] then self~ownerDialog~codeReview
      when id == self~constdir[IDC_PB_UPDATE       ] then self~ownerDialog~update
      when id == self~constdir[IDC_PB_PROJECTREVIEW] then self~ownerDialog~projectReview
      when id == self~constdir[IDC_PB_CLOSE        ] then do
        self~cancel:super
        self~ownerDialog~closePalette
      end
      otherwise nop
   end
   -- End select

-- Over-riding the ok and cancel methods, but not doing anything, prevents the
-- user from closing the tool palette unless she uses the close tool button.
::method ok
::method cancel

-- Private method to fill 2 arrays with the button IDs and the button bitmap
-- names.
::method populateButtons private
  expose buttonIDs buttonNames

  buttonIDs   = .array~of(IDC_PB_LOCKMODULE,    -
                          IDC_PB_LOCKPROJECT,   -
                          IDC_PB_SPLITMODULE,   -
                          IDC_PB_SAVEMODULE,    -
                          IDC_PB_SAVEPROJECT,   -
                          IDC_PB_SAVEALL,       -
                          IDC_PB_CHECKIN,       -
                          IDC_PB_LINKTOWEB,     -
                          IDC_PB_CODEREVIEW,    -
                          IDC_PB_UPDATE,        -
                          IDC_PB_PROJECTREVIEW, -
                          IDC_PB_CLOSE          -
                          )

  buttonNames = .array~of('resources\LockModule.bmp',     -
                          'resources\LockProject.bmp',    -
                          'resources\SplitModule.bmp',    -
                          'resources\SaveModule.bmp',     -
                          'resources\SaveProject.bmp',    -
                          'resources\SaveAll.bmp',        -
                          'resources\CheckIn.bmp',        -
                          'resources\LinkToWeb.bmp',      -
                          'resources\CodeReview.bmp',     -
                          'resources\Update.bmp',         -
                          'resources\ProjectReview.bmp',  -
                          'resources\ClosePalette.bmp'    -
                          )


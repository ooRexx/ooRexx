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

  srcDir = locate()

  .application~useGlobalConstDir("O", srcDir"resources\useTools.h")

  dlg = .MainDialog~new

  dlgTool = .ToolPaletteDlg~new(srcDir)
  dlgTool~ownerDialog = dlg

  -- Start the main dialog asynchronously so we continue and then can start the
  -- tool palette.
  dlg~executeAsync("SHOWTOP", IDI_DLG_OOREXX)

  -- Start the tool palette now.  It can not be started until its owner dialog
  -- has been started.
  dlgTool~popup("SHOWTOP")

  -- End async execution which will not return until the main dialog closes.
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


-- The event handling method for the Show Palette button click.  We hide the
-- button and execute a new tool palette dialog.  The button is always hidden
-- while a tool palette dialog is executing and is only visible when the user
-- has closed the currently executing tool palette dialog.
::method onShowPalette unguarded
  expose dlgTool

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

-- Invoked by the tool palette dialog when the user presses the close tool
-- button.  That is the only way for the user to close the tool palette.  Here
-- we set the tool text to the empty string and then make the Show Palette
-- button visible.  This allows the user to click on the button and reshow a
-- tool palette.
::method closePalette
  expose toolText

  toolText~setText('')
  self~paletteButton(.true)

-- This private method either hides or makes visible the Show Palette button
-- depending on the value of the argument.
::method paletteButton private
  use strict arg showButton

  if showButton then self~newPushButton(IDC_PB_SHOW_PALETTE)~show
  else self~newPushButton(IDC_PB_SHOW_PALETTE)~hide


::class 'ToolPaletteDlg' subclass UserDialog

::method init
  expose s buttonIDs

  forward class (super) continue

  -- Populate the buttonIDs and buttonNames arrays.
  self~populateButtons()

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
  self~create(0, 30, count * s~width, s~height, "Tool Palette", "NOMENU")

::method defineDialog
  expose s buttonIDs

  -- Create all our tool buttons and connect their button click event with the
  -- onToolClick method in this dialog.
  do i = 1 to buttonIDs~items
    x = (i - 1) * s~width
    if i == buttonIDs~items then x += s~width
    self~createPushButton(buttonIDs[i], x, 0, s~width, s~height, 'BITMAP', , onToolClick)
  end


-- In the initDialog method we create a bitmap Image object from the bitmap
-- files for each button.  Then an ImageList for each button is created and
-- assigned to the button.  Each image list only has one image in it, so the
-- operating system uses that one image for all the states of the button.
--
-- Note that when the bitmap image is added to the image list, the image list
-- makes a copy of the image, so we release the image resource in our copy once
-- it is added to the image list.
::method initDialog
   expose buttonIDs buttonNames

   tempButton = self~newPushButton(buttonIDs[1])

   size = tempButton~getRealSize
   size~width  -= size~width - 32
   size~height -= size~height - 32

   flags = 'COLOR24 MASK'
   type  = 'BITMAP'
   cRef  = .Image~colorRef(0, 0, 0)

   align  = 'CENTER'
   margin = .Rect~new(4)

   do i = 1 to buttonIDs~items
      bmpButton = self~newPushButton(buttonIDs[i])

      image = .Image~getImage(buttonNames[i], type)
      imageList = .ImageList~create(size, flags, 1, 0)
      imageList~addMasked(image, cRef)

      bmpButton~setImageList(imageList, margin, align)
      image~release
   end


-- The leaving method is called automatically by the ooDialog framework when the
-- underlying Windows dialog is closing.  The leaving method is a good place to
-- do any final clean up.  We use it here to release the operating system
-- resouces used by the image lists.
--
-- Note this.  For some controls like the list-view control, Microsoft's
-- documentation explicityly states that the list-view control will release
-- the image list resources.  However, for the button control, the documentation
-- does not say one way or the other.
::method leaving unguarded
   expose buttonIDs

   do id over buttonIDs
     btn = self~newPushButton(id)
     btn~getImageList~release
   end

-- The event handler for the button click event.  We map which button was
-- clicked to the proper method in the parent / owner dialog and invoke that
-- method
::method onToolClick unguarded
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

  srcDir = .application~srcDir

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

  buttonNames = .array~of(srcDir'resources\LockModule.bmp',     -
                          srcDir'resources\LockProject.bmp',    -
                          srcDir'resources\SplitModule.bmp',    -
                          srcDir'resources\SaveModule.bmp',     -
                          srcDir'resources\SaveProject.bmp',    -
                          srcDir'resources\SaveAll.bmp',        -
                          srcDir'resources\CheckIn.bmp',        -
                          srcDir'resources\LinkToWeb.bmp',      -
                          srcDir'resources\CodeReview.bmp',     -
                          srcDir'resources\Update.bmp',         -
                          srcDir'resources\ProjectReview.bmp',  -
                          srcDir'resources\ClosePalette.bmp'    -
                          )


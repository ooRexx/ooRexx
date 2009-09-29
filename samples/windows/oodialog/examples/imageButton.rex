/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2009 Rexx Language Association. All rights reserved.    */
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
 * File:     imageButton.rex
 *
 * Category: ooDialog example
 *
 * Syntax:
 *   rc = imageButton
 *
 * Purpose:
 *   Provides some example code of new ooDialog feature introduced in 4.0.0
 *
 *   This program uses a resource script (.rc) file for the dialog definition.
 *   The dialog was designed using a "what you see is what you get" dialog
 *   editor.  The resource script can be compiled into a binary image for use
 *   with a ResDialog.  In this case it is just left as a .rc file for use in
 *   a RcDialog.
 *
 *   Dialogs built using a dialog editor are much easier to create and maintain
 *   then UserDialogs.
 *
 * Input:
 *   None.
 *
 * Returns:
 *   0 on success, non-zero if the dialog could not be created for some reason.
 *
 * Notes:
 *   This is a short program written to amuse my niece.  Two of the button
 *   controls in the dialog use an image list.  The text for the buttons is 'Add
 *   Pictures' and 'View Pictures' with a picture of a camera on them.  When the
 *   user moves the mouse pointer over either one of the buttons, the picture
 *   changes to a skull and the text changes to 'Death if you touch me.' When
 *   the user pushes either of the buttons the picture changes to a giant bee.
 *
 *   My niece and I think this is hilarious.
 *
 *   The dialog shows how to use a button control image list.  Since I am not
 *   artistic, I produced the images by cutting them out of digital pictures
 *   I have taken over the last few years.
 *
 *   A professional application would of course use professional graphics.  This
 *   dialog just gives an idea of what the button control can do with an image
 *   list.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
use arg

  rcFile = "resources\imageButton.rc"
  symbolFile = "resources\imageButton.h"

  dlg = .ImageListDlg~new(rcFile, IDD_IMAGELIST_BUTTON, , symbolFile)

  if dlg~initCode <> 0 then do
    say "The Image List Dialog was not created correctly"
    say "Init code:" dlg~initCode
    return 99
  end

  dlg~Execute("SHOWTOP", IDI_DLG_OOREXX)

return 0
-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::requires "ooDialog.cls"

::class 'ImageListDlg' subclass RcDialog inherit AdvancedControls MessageExtensions

::method initDialog
  expose pbPush pbView pbAdd stStatus ctr imagesLoaded

  self~connectButton(IDC_PB_PUSHME, "onPushMe")
  self~connectButton(IDC_PB_VIEW, "onView")
  self~connectButton(IDC_PB_ADD, "onAdd")

  self~connectButtonNotify(IDC_PB_VIEW, "HOTITEM", onHover)
  self~connectButtonNotify(IDC_PB_ADD, "HOTITEM", onHover)

  imagesLoaded = .false
  ctr = 0

  pbPush = self~getButtonControl(IDC_PB_PUSHME)
  pbView  = self~getButtonControl(IDC_PB_VIEW)
  pbAdd  = self~getButtonControl(IDC_PB_ADD)
  stStatus = self~getStaticControl(IDC_ST_STATUS)

  stStatus~setText("")

  self~setPictureButtons

::method setPictureButtons private
  expose pbView pbAdd stStatus imagesLoaded imageList

  -- Using an image list with the button controls needs at least comctl32
  -- version 6.  See what version we are running under and don't use the image
  -- list if we are on an older version of Windows.
  if .DlgUtil~comCtl32Version < 6 then return self~oldSetButtons

  pbView~style = "MULTILINE BOTTOM"

  -- In the following code, there are a lot of if tests.  You can change any of
  -- the if tests to its converse to get an idea of the dialog behavior when
  -- errors happen.  Have fun, experiment.

  -- The images are loaded from files.
  files = .array~new()
  files[1] = "resources\Normal.bmp"       -- Normal
  files[2] = "resources\Hot.bmp"          -- Hot (hover)
  files[3] = "resources\Pushed.bmp"       -- Pushed
  files[4] = "resources\Disabled.bmp"     -- Disabled
  files[5] = "resources\Default.bmp"      -- Default button
  files[6] = "resources\Hot.bmp"          -- Stylus hot, tablet PC only

  -- .ImageList~create() exposes the Windows API ImageList_Create().  Use the
  -- MSDN documentation to fully understand this API.  Simply do a Google search
  -- with: MSDN ImageList_Create
  --
  -- The IIC_COLOR24 flag and the size values correspond to the values the
  -- bitmaps were created with.  You can look at the properties page for one
  -- of the bitamp files in Explorer to see these values.

  -- We set the flags to create a 24 bit color, masked image list.
  flags = .DlgUtil~or(.Image~toID(ILC_COLOR24), .Image~toID(ILC_MASK))

  -- Create an empty .ImageList object:
  imageList = .ImageList~create(.Size~new(61, 46), flags, 10, 10);

  -- Add the images to the image list, only if the image list is not null.  If
  -- the .ImageList object is null, an error happened when it was created and it
  -- is not usable.
  --
  -- When an image is added to the image list, the index of the image is
  -- returned.  -1 signals that the image was not added.  We use that as a flag
  -- to indicate some images are not added.
  index = -1
  lastGoodIndex = -1
  if \ imageList~isNull then do
    stStatus~setText('Created image list:' imageList~handle)

    -- We use a colorref when adding the image to the image list.  The 255,255,
    -- 255 value is white.  Essentially it tells the image list to create a mask
    -- that causes every  white pixel in the image to be transparent.
    --
    -- The only image it has any effect on is the camera.  In the camera image,
    -- the background uses white pixels.  So, when the camera image is drawn,
    -- the background becomes transparent, which allows the button color to show
    -- through.
    cRef = .Image~colorRef(255, 255, 255)

    -- For each image file, get an .Image object and add it to our image list.
    do f over files

      -- The defaults for the optional arguments to getImage() are good for this
      -- application.  Again, isNull() signals the .Image object is not valid.
      image = .Image~getImage(f)
      if \ image~isNull then do

        -- Add the image, quit if we get back a -1.
        index = imageList~addMasked(image, cRef)
        if index == -1 then leave

        stStatus~setText('Added image index:' index)
        lastGoodIndex = index

        -- The underlying ImageList control makes a copy of the image passed to
        -- it.  We can now relase the image to free up the system resource.
        -- Note that when this program ends, the OS will free the resource
        -- automatically, so this is NOT necessary.  Note also that you should
        -- not release SHARED images.  However, when an image is loaded from a
        -- file, it can not be a shared image.
        image~release

        -- Once you release the image, the .Image object is no longer valid and
        -- trying to use it will raise a syntax error.  You can use the isNull()
        -- method on any image object at any time to test if it is valid without
        -- raising a syntax error.  In addition, if you invoke the release()
        -- method after the image has already been released, it is just ignored.
      end
      else do
        say 'Failed to load bitmap:' f 'system error code:' .SystemErrorCode
      end
    end
  end
  else do
    stStatus~setText('Error creating .ImageList.  Images are not available.')
    return
  end

  if lastGoodIndex == -1 then do
    stStatus~setText("Failed to add any images.  Images are not available")
    imageList~release
    return
  end

  imagesLoaded = .true

  if index == -1 then do
    stStatus~setText("Not all images were loaded.  Last good image at index" lastGoodIndex)
  end

  -- Set up the alignment and margin around the image on the 'View Pictures'
  -- button.  Then set the image list in the button control
  align = .Image~toID(BUTTON_IMAGELIST_ALIGN_LEFT)
  margin = .Rect~new(1)

  pbView~setImageList(imageList, margin, align)

  if .SystemErrorCode <> 0 then do
    stStatus~setText('setImageList() failed. SystemErrorCode:' .SystemErrorCode 'Images not available.')
    imageList~release
    return
  end
  else do
    stStatus~setText("View Pictures button image list loaded.")
  end

  -- Temporarily set the title to the longest text, to calculate ideal size.
  pbView~setTitle("DEATH if you touch me")
  bestSize = pbView~getIdealSize

  -- Write this to the screen, example only.
  say 'Got ideal size:' bestSize
  say '  width: ' bestSize~width
  say '  height:' bestSize~height
  say

  -- Now reset the button label and set the size to the ideal size.
  pbView~setTitle("View Pictures")
  pbView~setRect(0, 0, bestSize~width, bestSize~height, "NOMOVE")

  -- Example only of the getImageList() method.  A .Directory object is
  -- returned.
  d = pbView~getImageList

  -- Just write this to the screen.
  say 'Got View Picutes button control image list information.'
  say 'Returned:' d
  if d <> .nil then do
    say '  image list:' d~imageList
    say '  handle:    ' d~imageList~handle
    say '  rect:      ' d~rect
    say '  align:     ' self~alignment2text(d~alignment)
    say 'Image margins:' d~rect~left',' d~rect~top',' d~rect~right',' d~rect~bottom
  end
  else do
    stStatus~setText("Error getting View Pictures button control image list information.")
  end
  say

  -- For the Add Pictures button, use the same image list created for the View
  -- Pictures button.  We just change the margins and the alignment.
  align = .Image~toID(BUTTON_IMAGELIST_ALIGN_CENTER)
  margin~left = 10
  margin~right = 10
  margin~top = 30
  margin~bottom = 15

  pbAdd~setImageList(imageList, margin, align)

  if .SystemErrorCode <> 0 then do
    stStatus~setText('setImageList() failed. SystemErrorCode:' .SystemErrorCode 'Images not available.')

    -- Releasing the image list now, after it has been successfully set for the
    -- View Pictures button, causes the images to disappear from that button.
    --
    -- In general, DON'T release an image list that is in use.  This is just
    -- done here to demonstrate that things won't blow up.
    imageList~release
    return
  end
  else do
    stStatus~setText("Add Pictures button image list loaded.")
  end

  -- Now, the same as above, figure out the ideal size and then adjust the
  -- button size.
  pbAdd~style = "BOTTOM MULTILINE"
  pbAdd~setTitle("DEATH if you touch me")
  s = pbAdd~getIdealSize
  pbAdd~setTitle("Add Pictures")
  pbAdd~setRect(0, 0, s~width, s~height, "NOMOVE")

  -- Again, just for an example of the getImageList() method.
  d = pbAdd~getImageList

  say 'Got Add Picutes button control image list information.'
  say 'Returned:' d
  if d <> .nil then do
    -- Write this to the screen.  Note that the image list hand is the same as
    -- the above.
    say '  image list:' d~imageList
    say '  handle:    ' d~imageList~handle
    say '  rect:      ' d~rect
    say '  align:     ' self~alignment2text(d~alignment)
    say 'Image margins:' d~rect~left',' d~rect~top',' d~rect~right',' d~rect~bottom
  end
  else do
    stStatus~setText("Error getting Add Pictures button control image list information.")
  end
  say

return

-- This method allows the dialog to work on systems prior to XP.  We just use
-- the old fashioned buttons.
::method oldSetButtons private
  expose pbView pbAdd

  pbView~setTitle("View Pictures")
  pbAdd~setTitle("Add Pictures")

  pbView~resize(50, 15)
  pbAdd~resize(50, 15)

return .true

-- As the user pushes the Push Me button, we cycle through some different
-- examples of changing the button state and style.  This changes the pictures
-- displayed on the buttons.  (Provided of course that there were no errors in
-- setting the button control image list(s).)  The state and style changes work
-- whether we have images or not, so we don't check for pictures loaded or not
-- on those.
::method onPushMe unguarded
  expose pbView pbAdd stStatus ctr imagesLoaded oldImageList

  select
    when ctr == 0 then do
      pbView~state = "FOCUS"
      stStatus~setText('View Pictures button set to the Focused state.')
      ctr += 1
    end
    when ctr == 1 then do
      pbAdd~style = "DEFPUSHBUTTON"
      stStatus~setText('Add Pictures button now the default push button.')
      ctr += 1
    end
    when ctr == 2 then do
      pbView~style = "DEFPUSHBUTTON"
      stStatus~setText('View Pictures button now the default push button.')
      ctr += 1
    end
    when ctr == 3 then do
      pbView~disable
      stStatus~setText('View Pictures button now disabled.')
      ctr += 1
    end
    when ctr == 4 then do
      pbView~enable
      cancel = self~getButtonControl(IDCANCEL)
      cancel~style = "DEFPUSHBUTTON"
      stStatus~setText('View Pictures button now enabled, should not be default push button.')
      ctr = 5
    end
    when ctr == 5 then do
      if imagesLoaded then do
        oldImageList = pbView~setImageList(.nil)
        stStatus~setText('Removed View Pictures button images.  Push "Push Me one more time to restore".')
      end
      ctr = 6
    end
    when ctr == 6 then do
      if imagesLoaded, oldImageList \== .nil then do
        pbView~setImageList(oldImageList~imageList, oldImageList~rect, oldImageList~alignment)
        stStatus~setText('Restored View Pictures button images.')
      end
      ctr = 0
    end
    otherwise do
      -- Should be impossible to happen.  Just do the same as if ctr were 6.
      if imagesLoaded, oldImageList \== .nil then do
        pbView~setImageList(oldImageList~imageList, oldImageList~rect, oldImageList~alignment)
        stStatus~setText('Restored View Pictures button images.')
      end
      ctr = 0
    end
  end
  -- End select

-- This is the amusing part.  We get this event message when the mouse is first
-- moved over a button (entering will be .true) or when the mouse moves off a
-- button (entering will be .false.)  We change the text and style of the
-- button, the button control itself changes the picture.
::method onHover unguarded
  expose imagesLoaded pbView pbAdd
  use arg id, entering

  if \ imagesLoaded then return

  -- The low word of the id argument is the button resource ID.  The high word
  -- is the notification ID.  In this case, we know it will be:
  --   BCN_HOTITEMCHANGE

  isViewButton = (.DlgUtil~loWord(id) == self~constDir[IDC_PB_VIEW])

  if entering then do
    text = "DEATH if you touch me"
    style = "TOP"
  end
  else do
    if isViewButton then text = "View Pictures"
    else text = "Add Pictures"
    style = "BOTTOM"
  end

  if isViewButton then do
    pbView~style = style
    pbView~setTitle(text)
  end
  else do
    pbAdd~style = style
    pbAdd~setTitle(text)
  end

::method onView unguarded
  expose stStatus
  stStatus~setText('In onView, no actions defined.')

::method onAdd unguarded
  expose stStatus
  stStatus~setText('In onAdd, no actions defined.')


/* When you are done using an image list you can release it to free up system
 * resources.
 *
 * Please note that when this program ends, the process will disappear and the
 * OS will free up the resources automatically.
 *
 * As noted in a comment above, if you release the image list when it is in
 * use, the pictures disappear.  Testing has shown that releasing an image list
 * when it is in use, does not crash the system.  However, releasing an image
 * list while it is in use would seem to be a foolish practice.
 *
 * When would you want to release an image list?  In this case if this dialog
 * was a part of a bigger, long running application.  When this dialog ends, it
 * makes sense to free up the system resources used for the image list.  Since
 * the main application is still running, the OS will not release the resources
 * used in this dialog.
 *
 * Microsoft says, you should release an image list when you are done with it.
 *
 * In this case that would be when the dialog is closed.  There are at least 2
 * strategies that could be employed.
 *
 * 1.) Save a reference to the image list handle returned by .ImageList~create()
 * and pass it to release().
 *
 * 2.) Use getImageList() to ask the button to give you the image list
 * information and use the image list object that is returned as part of the
 * .Directory object.  The image list object in strategy 1 and 2 are the same
 * object.
 *
 * Don't forget to release the handle if either the dialog is canceled or if it
 * is closed.  Below is a demonstration of both strategies.
 */

::method cancel
  expose imageList imagesLoaded

  if imagesLoaded then imageList~release
  return self~cancel:super

::method ok
  expose imagesLoaded pbView

  if imagesLoaded then do
    imageListInfo = pbView~getImageList
    if imageListInfo <> .nil then imageListInfo~imageList~release
  end

  return self~ok:super

::method initAutoDetection
   self~noAutoDetection

::method alignment2text private
  use strict arg alignment

  select
    when alignment == .Image~toID("BUTTON_IMAGELIST_ALIGN_LEFT") then return 'Left'
    when alignment == .Image~toID("BUTTON_IMAGELIST_ALIGN_RIGHT") then return 'Right'
    when alignment == .Image~toID("BUTTON_IMAGELIST_ALIGN_TOP") then return 'Top'
    when alignment == .Image~toID("BUTTON_IMAGELIST_ALIGN_BOTTOM") then return 'Bottom'
    when alignment == .Image~toID("BUTTON_IMAGELIST_ALIGN_CENTER") then return 'Center'
    otherwise return 'Error'
  end

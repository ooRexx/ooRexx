/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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

/** Propery Sheet / Dialog Controls Example
 *
 * This example demonstrates how to use the PropertySheetDialog and has examples
 * of using 5 dialog controls, the: List View, Tree View, Progress Bar, Track
 * Bar, and Tab.
 */

  -- To run correctly, this program needs to be able to find its support files.
  -- But, we allow starting the program from anywhere.  To do this we:
  -- get the directory we are executing from, switch to the directory this
  -- program is installed in, and then switch back to the directory we started
  -- from when we quit.

  curdir = directory();                       -- Directory we started from.
  parse source . . me
  mydir = me~left(me~lastpos('\')-1)          -- Directory we are installed in.
  mydir = directory(mydir)                    -- CD to the install direcotry.

  -- Create the 5 dialog pages.
  p1 = .ListViewDlg~new("rc\PropertySheetDemo.rc", IDD_LISTVIEW_DLG, , "rc\PropertySheetDemo.h")
  p2 = .TreeViewDlg~new("rc\PropertySheetDemo.rc", IDD_TREEVIEW_DLG, , "rc\PropertySheetDemo.h")
  p3 = .ProgressBarDlg~new("rc\PropertySheetDemo.rc", IDD_PROGRESSBAR_DLG, , "rc\PropertySheetDemo.h")
  p4 = .TrackBarDlg~new("rc\PropertySheetDemo.rc", IDD_TRACKBAR_DLG, , "rc\PropertySheetDemo.h")
  p5 = .TabDlg~new("rc\PropertySheetDemo.rc", IDD_TAB_DLG, , "rc\PropertySheetDemo.h")

  -- Create the PropertySheetDialog using an array of the 5 dialog pages.  The
  -- order of the pages in the array will be the order of the pages in the tab
  -- control of the property sheet.
  pages = .array~of(p1, p2, p3, p4, p5)
  propDlg = .PropertySheetDemoDlg~new(pages, "NOAPPLYNOW", "ooRexx Property Sheet with Controls", "rc\PropertySheetDemo.h")

  -- For the appliation icon of the dialog we will use the ooRexx icon which is
  -- available for general use from oodialog.dll.
  propDlg~resources = .ResourceImage~new("oodialog.dll", propDlg)
  propDlg~appIcon = "IDI_DLG_OOREXX"

  -- Show the property sheet.
  propDlg~execute

  -- Switch back to the intial directory and quit.
  ret = directory(curdir)
  return 0

::requires "ooDialog.cls"

::class 'ListViewDlg' subclass RcPSPDialog

::method initDialog
    expose lv

    -- Instantiate a Rexx list view object that represents the underlying
    -- Windows list-view.  The list-view style is report.
    lv = self~newListView(IDC_LV_MAIN)

    -- Set the column headers
    lv~insertColumn(0, "Symbol", 40)
    lv~insertColumn(1, "Quote", 50)
    lv~insertColumn(2, "Year high", 50)
    lv~insertColumn(3, "Year low", 50)
    lv~insertColumn(4, "Description", 120)

    -- Set the images for the items in the list-view.  The list-view control was
    -- created without the SHAREIMAGES styles, so it takes care of releasing the
    -- image list when the program ends.
    image = .Image~getImage("bmp\propertySheetDemoListView.bmp")
    imageList = .ImageList~create(.Size~new(16, 16), .Image~toID(ILC_COLOR8), 4, 0)
    if \image~isNull,  \imageList~isNull then do
        imageList~add(image)
        lv~setImageList(imageList, .Image~toID(LVSIL_SMALL))

        -- The image list makes a copy of the bitmap, so we can release it now
        -- to free up some (small) amount of system resources.  This is not
        -- necessary, the OS will release the resource automatically when the
        -- program ends.
        image~release
    end

    -- Fill the list-view with random data.
    do ch = "A"~c2d to "Z"~c2d
        q = random(200)
        call msSleep 1
        yh = random(400)
        yh = max(yh, q)
        yl = random(100)
        yl = min(yl, q)
        lv~addRow( , random(3), "_" || ch~d2c~copies(3) || "_", "$" || q, "$" || yh, "$" || yl, -
                  ch~d2c~copies(3) "is a fictitious company.")
    end

    -- Add full row select and the ability to drag and drop the columns to the
    -- list-view.
    lv~addExtendedStyle("FULLROWSELECT HEADERDRAGDROP")

    -- Connect 2 list-view events to Rexx methods in this dialog.  The double-
    -- click on a list-view item, and the click on a column header events.
    self~connectListViewEvent(IDC_LV_MAIN, "ACTIVATE", "onActivate")
    self~connectListViewEvent(IDC_LV_MAIN, "COLUMNCLICK")

-- Invoked when a list-view item is double-clicked.  We display a message and
-- set the focus to the next item in the list.
::method onActivate
    expose lv

    selectedItem = lv~focused
    symbol = lv~itemText(selectedItem)
    price = lv~itemText(selectedItem, 1)

    question = "You have selected the stock with symbol" symbol". Do you want to order" || .endOfLine || -
               "50 shares of stock at" price"?"
    placeOrder = MessageDialog(question, self~hwnd, "Place Order for Stock", "YESNO", "QUESTION", "DEFBUTTON2" )

    cost = 50 * price~substr(2)
    if placeOrder == self~constDir["IDYES"] then do
        j = MessageDialog("Okay, your bank account will be debited $"cost "dollars.", self~hwnd, -
                         "Order Confirmation", "OK", "INFORMATION")
    end
    else do
        j = MessageDialog("That saved you $"cost "dollars.", self~hwnd, "Order Canceled", "OK", "EXCLAMATION")
    end

    lv~deselect(selectedItem)
    selectedItem += 1
    lv~focus(selectedItem)
    lv~select(selectedItem)

-- Invoked when a column header of the list-view is clicked.  We just show a
-- message box so that the user has some feedback.
::method OnColumnClick
   use arg id, column
   j = MessageDialog("Column" column + 1 "was clicked in control" id, self~hwnd, "List-View Notification")


::class 'TreeViewDlg' subclass RcPSPDialog

::method initDialog

    -- Instantiate a Rexx tree view object that represents the Windows tree-view
    -- control.
    tv = self~newTreeView(IDC_TV_MAIN)

    -- Create and set the ImageList for the tree view items
    image = .Image~getImage("bmp\propertySheetDemoTreeView.bmp")
    imageList = .ImageList~create(.Size~new(32, 32), .Image~toID(ILC_COLOR8), 10, 0)
    if \image~isNull,  \imageList~isNull then do
          imageList~add(image)
          tv~setImageList(imageList, .Image~toID(TVSIL_NORMAL))
          image~release
    end

    -- Add the tree view items.  Toys will be the root (the first argument is
    -- not omitted.  Subitems are added by omitting the first arguments.  The
    -- number of arguments omitted indicates the depth of the subitem.
    --
    -- The last numeric argument in some of the items is the index for the icon
    -- for that item in the image list.  Those items without a number will not
    -- display an icon.

    tv~add("Toys", 1)
    tv~add(, "Indoor", 14)
    tv~add(, , "Boys", 19)
    tv~add(, , , "Cowboys", 13)
    tv~add(, , , "Cars", 8)
    tv~add(, , , "Starwars", 9)
    tv~add(, , "Girls", 0)
    tv~add(, , , "Barby", 19)
    tv~add(, , , "Painting", 15)
    tv~add(, , , "Cooking", 13)
    tv~add(, , "Adults", 17)
    tv~add(, , , "Poker", 15)
    tv~add(, , "Technical", 16)
    tv~add(, , , "Racing cars", 8)
    tv~add(, , , "Trains", 7)
    tv~add(, "Outdoor", 11)
    tv~add(, , "Water", 22)
    tv~add(, , , "Ball", 5)
    tv~add(, , , "Soft tennis", 6)
    tv~add(, , "Sand", 12)
    tv~add(, , , "Shovel", 12)
    tv~add(, , , "Bucket", 19)
    tv~add(, , , "Sandbox", 12)
    tv~add(, , "Technical", 16)
    tv~add(, , , "Trains", 7)
    tv~add(, , , "Remote controlled", 8)
    tv~add("Office Articles", 2)
    tv~add( , "Tools", 16)
    tv~add( , "Books", 19)
    tv~add( , , "Introduction", 14)
    tv~add( , , "Advanced Programming", 17)
    tv~add( , , "Tips & Tricks", 16)
    tv~add("Hardware", 4)
    tv~add( , "Garden", 0)
    tv~add( , "Handyman", 18)
    tv~add( , "Household", 18)
    tv~add("Furniture", 3)
    tv~add( , "Standard", 12)
    tv~add( , "Luxury", 21)

    -- Connecting the begin drag event and using the default tree drag handler
    -- allows us to suppport drag and drop (using the default behaviour.
    self~connectTreeViewEvent(IDC_TV_MAIN, "BeginDrag", "DefTreeDragHandler")


::class 'ProgressBarDlg' subclass RcPSPDialog

::method initDialog
  expose threadsStarted processes

  threadsStarted = 0
  processes = .array~of('animateProgressA', 'animateProgressB', 'animateProgressC',  -
                        'animateProgressD', 'animateProgressE')


-- The setActive event notification is sent by the property sheet to the page
-- that is about to become the active page.  It is sent before the page is
-- visible.  This allows the page to do any initialization necessary.  For this
-- program, we use the notification to start the progress bar animation threads.
::method setActive unguarded
    expose threadsStarted processes
    use arg propSheet

    reply 0

    -- If no threads are running, start a thread to run each progress bar
    -- asynchronously.
    if threadsStarted < 1 then do
        threadsStarted = processes~items
        do methodName over processes
            self~start(methodName)
        end
    end

-- This is the generic method that simulates some type of processing that takes
-- a long time.  The progress of this processing is displayed by the progress
-- bar. to simulate a process of which the progress is displayed by a progress bar */
::method animateProgress unguarded
   use arg progressBar, label, step, iterations, tsleep

   progressBar~setRange(0, iterations * step)
   progressBar~setStep(step)
   do i = 1 to iterations
       progressBar~step
       if (iterations * step == 100) then label~setText(i * step "%")
       else label~setText(i * step)

       call msSleep tsleep
       if self~finished | \ self~isDialogActive then return
   end

-- The following 5 methods are started asynchronously to animate the progress
-- bars.
::method animateProgressA unguarded
   expose threadsStarted pbA labelA

   if \ pbA~isA(.ProgressBar) then do
      pbA = self~newProgressBar(IDC_PBAR_PROCESSA)
      labelA = self~newStatic(IDC_ST_PERCENTA)
   end

   self~animateProgress(pbA, labelA, 5, 20, 600)
   threadsStarted -= 1

::method animateProgressB unguarded
   expose threadsStarted pbB labelB

   if \ pbB~isA(.ProgressBar) then do
      pbB = self~newProgressBar(IDC_PBAR_PROCESSB)
      labelB = self~newStatic(IDC_ST_PERCENTB)
   end

   self~animateProgress(pbB, labelB, 1, 100, 150)
   threadsStarted -= 1

::method animateProgressC unguarded
   expose threadsStarted pbC labelC

   if \ pbC~isA(.ProgressBar) then do
      pbC = self~newProgressBar(IDC_PBAR_PROCESSC)
      labelC = self~newStatic(IDC_ST_PERCENTC)
   end

   self~animateProgress(pbC, labelC, 2, 50, 200)
   threadsStarted -= 1

::method animateProgressD unguarded
    expose threadsStarted pbD labelD

    if \ pbD~isA(.ProgressBar) then do
        pbD = self~newProgressBar(IDC_PBAR_PROCESSD)
        labelD = self~newStatic(IDC_ST_PERCENTD)
    end

    self~animateProgress(pbD, labelD, 10, 40, 300)
    threadsStarted -= 1

::method animateProgressE unguarded
    expose threadsStarted pbE labelE

    if \ pbE~isA(.ProgressBar) then do
        pbE = self~newProgressBar(IDC_PBAR_PROCESSE)
        labelE = self~newStatic(IDC_ST_PERCENTE)
    end

    self~animateProgress(pbE, labelE, 20, 50, 500)
    threadsStarted -= 1


::class 'TrackBarDlg' subclass RcPSPDialog

::method initDialog
    expose font1 trackBars tbLabels

    -- As we initialize each track bar we'll stash the Rexx object in a table
    -- for easy access later, indexed by its numeric resource id.  The same
    -- thing is done for the static control that is the label for the track bar.
    trackBars = .table~new
    tbLabels = .table~new

    -- For the horizonatal track bars we'll use a big font for the label.
    font1 = self~CreateFontEx("Arial", 24, "BOLD")

    -- The symbolic IDs for the track bars / labels are named after the style
    -- of the track bar.  Vertical or horizontal and where the ticks are placed.

    -- Initialize the horizontal track bar with ticks on the bottom.
    tb = self~newTrackBar(IDC_TB_HORZ_BOTTOM)
    label = self~newStatic(IDC_ST_HORZ_BOTTOM)

    tb~setTickFrequency(10)
    tb~setPos(20, .true)
    label~setText(20)
    label~setFont(font1)

    id = self~constDir[IDC_TB_HORZ_BOTTOM]
    trackBars[id] = tb
    tbLabels[id] = label

    -- Initialize the horizontal track bar with ticks on the top.
    tb = self~newTrackBar(IDC_TB_HORZ_TOP)
    label = self~newStatic(IDC_ST_HORZ_TOP)

    tb~initRange(0, 200)
    tb~setTickFrequency(50)
    tb~setPos(40, .true)
    label~setText(40)
    label~setFont(font1)

    id = self~constDir[IDC_TB_HORZ_TOP]
    trackBars[id] = tb
    tbLabels[id] = label

    -- Initialize the horizontal track bar with ticks on the both sides.
    tb = self~newTrackBar(IDC_TB_HORZ_BOTH)
    label = self~newStatic(IDC_ST_HORZ_BOTH)

    tb~initSelRange(20, 60)
    tb~setTickFrequency(10)
    tb~setPos(80, .true)
    label~setText(80)
    label~setFont(font1)

    id = self~constDir[IDC_TB_HORZ_BOTH]
    trackBars[id] = tb
    tbLabels[id] = label

    -- Initialize the vertical track bar with ticks on the right.
    tb = self~newTrackBar(IDC_TB_VERT_RIGHT)
    label = self~newStatic(IDC_ST_VERT_RIGHT)

    tb~setTickFrequency(10)
    tb~setPos(30, .true)
    label~setText(30)

    id = self~constDir[IDC_TB_VERT_RIGHT]
    trackBars[id] = tb
    tbLabels[id] = label

    -- Initialize the vertical track bar with ticks on the left.
    tb = self~newTrackBar(IDC_TB_VERT_LEFT)
    label = self~newStatic(IDC_ST_VERT_LEFT)

    tb~setTickFrequency(10)
    tb~initRange(0,400)
    tb~setLineStep(5)
    tb~setPageStep(50)
    tb~setPos(90, .true)
    label~setText(90)

    id = self~constDir[IDC_TB_VERT_LEFT]
    trackBars[id] = tb
    tbLabels[id] = label

    -- Initialize the vertical track bar with ticks on the both sides.
    tb = self~newTrackBar(IDC_TB_VERT_BOTH)
    label = self~newStatic(IDC_ST_VERT_BOTH)

    tb~setTickFrequency(5)
    tb~setPos(70, .true)
    label~setText(70)

    id = self~constDir[IDC_TB_VERT_BOTH]
    trackBars[id] = tb
    tbLabels[id] = label

    -- Connect the event notification that is sent when a track bar is moved to
    -- the onEndTrack() method.  That method will update the text label for the
    -- track bar with the new postition.
    self~connectTrackBarEvent(IDC_TB_HORZ_BOTH, "EndTrack", "onEndTrack")
    self~connectTrackBarEvent(IDC_TB_HORZ_TOP, "EndTrack", "onEndTrack")
    self~connectTrackBarEvent(IDC_TB_HORZ_BOTTOM, "EndTrack", "onEndTrack")
    self~connectTrackBarEvent(IDC_TB_VERT_RIGHT, "EndTrack", "onEndTrack")
    self~connectTrackBarEvent(IDC_TB_VERT_LEFT, "EndTrack", "onEndTrack")
    self~connectTrackBarEvent(IDC_TB_VERT_BOTH, "EndTrack", "onEndTrack")

-- Update the static contol that shows the position for a slider when the
-- user is done moving it.
::method onEndTrack
    expose trackBars tbLabels
    use arg code, hwndTrackBar

    -- hwndTrackBar is the handle to the track bar that was moved.  Get its
    -- resource ID and use that as an index into the table of track bar objects
    -- and the table of the labels.
    id = self~getControlID(hwndTrackBar)
    tbLabels[id]~setText(trackBars[id]~pos)

-- We use the leaving() method to clean up (delete) the font we created.  In
-- this program there is really no need to do this.  As soon as the interpreter
-- terminates, the OS cleans up the resources automatically.  The only time
-- cleaning up resources makes sense is in a long-running program that creates
-- and ends a lot of dialogs.  Then, over time, the memory usge of the program
-- would keep growing.
::method leaving
    expose font1
    self~deleteFont(font1)

::method initAutoDetection
    self~noAutoDetection


::class 'TabDlg' subclass RcPSPDialog

::method initDialog
   expose font2 font3 imageList iconsRemoved needWrite

   -- Set the iconsRemoved and needWrite to false.  These flags are used in
   -- the OnDrawTabRect() method.
   iconsRemoved = .false
   needWrite = .false

   -- Connect the draw event of the owner-drawn button.  This is sent when the
   -- button needs to be drawn.  Then connect the selection changed event of the
   -- tab control.  This is sent when the user clicks on a different tab.
   self~connectDraw(IDC_PB_OWNERDRAW, "onDrawTabRect")
   self~connectTabEvent(IDC_TAB_MAIN, "SELCHANGE", "onTabSelChange")

   tc = self~newTab(IDC_TAB_MAIN)
   if tc == .nil then return

   -- Create a font used to display the name of the color in the owner-drawn
   -- button.  Create another font used to display some informative text.
   font2 = self~createFontEX("Arial", 48, "BOLD ITALIC")
   font3 = self~createFontEx("Arial", 16, "BOLD")

   -- Add all the tabs, including the index into the image list for an icon for
   -- each tab.
   tc~AddFullSeq("Red", 0, ,"Green", 1, , "Moss", 2, , "Blue", 3, , "Purple", 4, , "Cyan", 5, , "Gray", 6)

   -- Create a COLORREF (pure white) and load our bitmap.  The bitmap is a
   -- series of 16x16 images, each one a colored letter.
   cRef = .Image~colorRef(255, 255, 255)
   image = .Image~getImage("bmp\propertySheetDemoTab.bmp")

   -- Create our image list, as a masked image list.
   flags = .DlgUtil~or(.Image~toID(ILC_COLOR24), .Image~toID(ILC_MASK))
   imageList = .ImageList~create(.Size~new(16, 16), flags, 10, 0)
   if \image~isNull,  \imageList~isNull then do
      -- The bitmap is added and the image list deduces the number of images
      -- from the width of the bitmap.  For each image, the image list creates a
      -- mask using the color ref.  In essence, the mask is used to turn each
      -- white pixel in the image to transparent.  In this way, only the letter
      -- part of the image shows and the rest of the image lets the under-lying
      -- color show through.
      imageList~addMasked(image, cRef)
      tc~setImageList(imageList)

      -- The image list makes a copy of each image added to it.  So, we can now
      -- release the original image to free up some small amount of system
      -- resoureces.
      image~release
   end
   else do
      iconsRemoved = .true
   end

-- When a new tab is selected, we have the owner-drawn button update itself.
-- This causes the button to redraw and the onDrawTabRect() method gets invoked,
-- which actually does the drawing.
::method onTabSelChange
   button = self~newPushButton(IDC_PB_OWNERDRAW)
   button~update


-- Fill the owner-drawn button with the color matching the tab's label and write
-- the name of the color.
::method onDrawTabRect
   expose font2 font3 imageList iconsRemoved needWrite
   use arg id

   button = self~newPushButton(id)
   if button == .nil then return
   tc = self~newTab(IDC_TAB_MAIN)
   if tc == .nil then return

   -- Each time the 'Gray' tab is selected, we remove the tab icons.  Then, when
   -- one of the other tabs is selected we set the  image list back.
   currentTab = tc~selected
   if currentTab == 'Gray' then do
      tc~setImageList(.nil)
      iconsRemoved = .true
      needWrite = .true
   end
   else do
      if iconsRemoved then do
         tc~setImageList(imageList)
         iconsRemoved = .false
         needWrite = .true
      end
   end

   -- Get the button's device context, create pen and brush, and assign pen,
   -- brush and font to the device context.
   dc = button~getDC
   pen = button~createPen(1, "SOLID", 0)
   brush = button~createBrush(tc~SelectedIndex + 1)

   oldPen = button~objectToDc(dc, pen)
   oldBrush = button~objectToDc(dc, brush)
   oldFont = button~fontToDC(dc, font2)
   button~transparentText(dc)

   -- Draw a filled in rectangle and write text
   button~rectangle(dc, 5, 5, button~SizeX * button~FactorX - 10, button~SizeY * button~FactorY - 10, "FILL")
   button~writeDirect(dc, 30, 50, tc~Selected)

   -- Add informative text if needed.
   if needWrite then do
      button~fontToDC(dc, font3)
      if currentTab == 'Gray' then button~writeDirect(dc, 30, 120, "(Tab icons are removed)")
      else button~writeDirect(dc, 30, 120, "(Tab icons are restored)")
      needWrite = .false
   end

   -- Restore pen, brush, and font, then release the device context.
   button~objectToDc(dc, oldPen)
   button~objectToDc(dc, oldBrush)
   button~fontToDC(dc, oldFont)
   button~opaqueText(dc)

   button~deleteObject(pen)
   button~deleteObject(brush)
   button~freeDC(dc)

-- We use the leaving() method to clean up (delete) the fonts and the image list
-- we created.  In  this program there is really no need to do this.  As soon as
-- the interpreter terminates, the OS cleans up the resources automatically.
-- The only time cleaning up resources makes sense is in a long-running program
-- that creates and ends a lot of dialogs.  Then, over time, the memory usge of
-- the program would keep growing.
::method leaving
    expose font2 font3 imageList

    self~deleteFont(font2)
    self~deleteFont(font3)
    imageList~release

::class 'PropertySheetDemoDlg' subclass PropertySheetDialog


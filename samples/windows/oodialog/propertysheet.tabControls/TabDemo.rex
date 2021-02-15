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

/** Tab Control / Dialog Controls / ControlDialog Example
 *
 * This example demonstrates how to use the ControlDialog class to provide the
 * pages / content of a Tab control.  It is basically the same as the
 * PropertySheetDemo.rex example, but uses a Tab control with ControlDialog
 * objects rather than using a PropertySheetDialog.
 *
 * Much of the code is simply copied from the PropertySheetDemo.rex program.
 */

  -- To run correctly, this program needs to be able to find its support files.
  -- But, we allow starting the program from anywhere.  To do this we:
  -- get the directory we are executing from using the locate() convenience
  -- routine.  This routine also sets the .application object's srcDir
  -- attribute.  We can then easily construct complete path names for any of our
  -- support files.
  sd = locate()
  .application~setDefaults("O", sd"rc\PropertySheetDemo.h", .false)


  -- The original version of this example would take a longish period of time to
  -- appear on the screen.  Too much was being done in the initDialog() method
  -- of the .NewControlsDialog.  To improve the time between starting the
  -- program and the dialog's appearance on the screen, several steps were
  -- taken.
  --
  -- 1.) All dialogs were made ResDialog dialogs.
  --
  -- 2.) The instantiation of the dialogs used for the pages of the tab control
  --     was moved out of the initDialog() method and placed here.  Some of the
  --     work done in the initDialog() method was moved to the prep() method,
  --     which is invoked before the dialog is started executing.
  --
  -- 3.) The dialog is created with the VISIBLE style.  This causes the dialog
  --     to appear on the screen before the list view is populated, rather than
  --     having the list view populated first and then showing the dialog.
  --
  -- Instantiate all the control dialogs and pass them to the prep() method in
  -- an array.
  t1 = .ListViewDlg~new(sd"rc\PropertySheetDemo.dll", IDD_LISTVIEW_DLG)
  t2 = .TreeViewDlg~new(sd"rc\PropertySheetDemo.dll", IDD_TREEVIEW_DLG)
  t3 = .ProgressBarDlg~new(sd"rc\PropertySheetDemo.rc", IDD_PROGRESSBAR_DLG)
  t4 = .TrackBarDlg~new(sd"rc\PropertySheetDemo.dll", IDD_TRACKBAR_DLG)
  t5 = .TabDlg~new(sd"rc\PropertySheetDemo.dll", IDD_TAB_DLG)

  tabContent = .array~of(t1, t2, t3, t4, t5)

  -- Create the main dialog.
  dlg = .NewControlsDialog~new(sd'rc\PropertySheetDemo.dll', IDD_NEWCONTROLS_DLG)

  -- Invoke the prep() methods of the list view and progress bar dialogs to do
  -- some initial set up before we start executing the main dialog.  Note that
  -- the list view (t1) dialog needs to have its owner dialog set before
  -- invoking the prep method.
  t1~ownerDialog = dlg
  t1~prep
  t3~prep

  -- Invoke the main dialog's prep() method to do some initial set up before the
  -- dialog is started executing.
  dlg~prep(tabContent)

  -- Show and run the dialog.
  dlg~execute('SHOWTOP', IDI_DLG_OODIALOG)

  return 0

::requires "ooDialog.cls"

::class 'NewControlsDialog' subclass ResDialog

::attribute tabContent

/** prep()
 *
 * This method is invoked before the dialog is executed.  It does some initial
 * set up that would normally be done (by the author) in initDialog().  The
 * tabContent argument is an array of the 5 ControlDialog dialogs used as the
 * content for the 5 pages of the tab control.  This array is saved in the
 * tabContent attribute so the dialogs can be accessed when needed.
 */
::method prep
  expose tabContent lastSelected havePositioned
  use strict arg tabContent

  -- The havePositioned array is used to determine if the page dialogs have been
  -- positioned or not.  Mark all 5 dialogs as not having been positioned yet.
  havePositioned = .array~of(.false, .false, .false, .false, .false)

  -- No tab has been selected yet
  lastSelected = 0

  -- Connect the event handling methods to the events we are interested in.
  self~connectButtonEvent(IDC_PB_PREVIOUS, CLICKED, onPrevious)
  self~connectButtonEvent(IDC_PB_NEXT, CLICKED, onNext)
  self~connectTabEvent(IDC_TAB, SELCHANGE, onNewTab)
  self~connectTabEvent(IDC_TAB, "KEYDOWN", "onTabKeyDown", 'SYNC')


/** initDialog()
 *
 * Initialize the underlying Windows dialog.  This includes setting up the tab
 * control tabs, executing the dialog used for the first tab, and positioning
 * the dialog over the display area of the tab control.  There are 5
 * .ControlDialog dialogs.  The dialogs are used for the display
 * area of the tab control, one control dialog for each tab of the tab control.
 */
::method initDialog
  expose tabContent tabControl pbNext pbPrevious

  -- Start executing the control dialog for the first tab in the tab control.
  -- We can not resize and reposition the control dialog until the underlying
  -- dialog is created, so we get it started, then do our other tasks.
  dlg = tabContent[1]
  dlg~execute

  -- Add the tabs to the tab control.
  tabControl = self~newTab(IDC_TAB)
  tabControl~addSequence("List View", "Tree View", "Progress Bar", "Track Bar", "Tab")

  -- Save a reference to the push buttons.
  pbNext = self~newPushButton(IDC_PB_NEXT)
  pbPrevious = self~newPushButton(IDC_PB_PREVIOUS)

  -- Determine the position and size of the display area of the tab control.
  self~calculateDisplayArea

  -- Position and show the control dialog used for the first page of the tab.
  self~positionAndShow(1)


/** calculateDisplayArea()
 *
 * Tab controls contain two areas, the tabs themselves and the display area.
 * The display area is where the content for each tab is drawn.
 *
 * We need to match the control dialog(s) size and position with the display
 * area size and position.  There are two approaches here:
 *
 * We could calculate the size of the largest dialog, resize the tab control to
 * match, and position the control dialog over the display area.
 *
 * We can get the size and position of the tab control's display area and resize
 * and reposition the control dialog(s) to match.  This is the approach we use
 * here.
 */
::method calculateDisplayArea private
  expose tabControl displayRect

  -- Given a rectangle describing the tab control's size and position, the tab
  -- control itself will calculate the display area's size and position.
  r = tabControl~windowRect
  tabControl~calcDisplayRect(r)

  -- Save the size of the display area, we need it later.
  s = .Size~new(r~right - r~left, r~bottom - r~top)

  -- Now we need to map the display area's position on the screen, to the client
  -- co-ordinates of the main dialog. The control dialog(s) are children windows
  -- of the main dialog, which is why we need to use the client-area of the
  -- dialog, not the client area of the tab control.
  p = .Point~new(r~left, r~top)
  self~screen2client(p)

  -- Create our display rectangle.  This is used in setWindowPosition(), which
  -- takes a point / size rectangle.  ooDialog defines a point / size rectangle
  -- as using the left and top attributes for the position of the upper left
  -- corner of a rectangle, using the right attribute for the width of the
  -- rectangle, and using the bottom attribute for the height of the rectangle.
  displayRect = .Rect~new(p~x, p~y, s~width, s~height)


/** positionAndShow()
 *
 * Used to resize and reposition one of the control dialogs so it occupies the
 * display area of the tab control.
 *
 */
::method positionAndShow private
  expose tabControl tabContent displayRect lastSelected havePositioned
  use strict arg index

  -- We can not position the control dialog until the underlying Windows dialog
  -- is created. If the system is heavily loaded for some reason, this may not
  -- have happened yet.  We need to wait for it.
  dlg = tabContent[index]
  do i = 1 to 10
    if dlg~hwnd <> 0 then leave
    z = SysSleep(.01)
  end

  if dlg~hwnd == 0 then do
    say "Error creating dialog for the tab with index:" index", aborting"
    return self~cancel
  end

  if lastSelected <> 0 then tabContent[lastSelected]~hide

  -- Now resize and reposition the control dialog to the tab control's display
  -- area.  We need to position the control dialog *above* the tab control in
  -- the Z-order so that it shows.
  dlg~setWindowPos(tabControl~hwnd, displayRect, "SHOWWINDOW NOOWNERZORDER")

  lastSelected = index
  havePositioned[index] = .true

  self~checkButtons


::method onNewTab
  expose tabControl tabContent havePositioned lastSelected

  index = tabControl~selectedIndex + 1
  dlg = tabContent[index]

  if havePositioned[index] then do
    last = tabContent[lastSelected]
    last~hide
    dlg~show
    lastSelected = index
  end
  else do
    dlg~ownerDialog = self
    dlg~execute
    self~positionAndShow(index)
  end
  if index == 3 then dlg~activateThreads

  self~checkButtons


::method onNext unguarded
  expose tabControl

  tabControl~selectIndex(tabControl~selectedIndex + 1)
  self~onNewTab


::method goTo private unguarded
  use strict arg tabControl, index

  tabControl~selectIndex(index)
  self~onNewTab


::method onPrevious unguarded
  expose tabControl

  tabControl~selectIndex(tabControl~selectedIndex - 1)
  self~onNewTab

::method onTabKeyDown unguarded
  use arg id, vKey, tabControl

  lastTab = tabControl~items

  -- We have to reply or the message loop can not process messages and if the
  -- dialog for the new tab has not been started yet, it can not execute.
  reply 0

  select
    when vKey == .VK~N then self~onNext
    when vKey == .VK~P then self~onPrevious
    when vKey == .VK~home then self~goTo(tabControl, 0)
    when vKey == .VK~end then self~goTo(tabControl, lastTab)
    otherwise nop  -- ignore all other keys
  end
  -- End select

  return


::method checkButtons private
  expose tabControl pbNext pbPrevious

  index = tabControl~selectedIndex + 1
  if index == 1 then do
    pbPrevious~disable
    pbNext~enable
  end
  else if index == 5 then do
    pbPrevious~enable
    pbNext~disable
  end
  else do
    pbPrevious~enable
    pbNext~enable
  end


::method cancel
  expose tabContent

  do dlg over tabContent
    dlg~endExecution(.false)
  end

  return self~cancel:super

::method ok
  expose tabContent

  do dlg over tabContent
    dlg~endExecution(.true)
  end

  return self~ok:super



::class 'ListViewDlg' subclass ResControlDialog

::method initDialog
    expose lv imageList listData

    -- Instantiate a Rexx list view object that represents the underlying
    -- Windows list-view.  The list-view style is report.
    lv = self~newListView(IDC_LV_MAIN)

    -- Set the column headers
    lv~insertColumn(0, "Symbol", 40)
    lv~insertColumn(1, "Quote", 50)
    lv~insertColumn(2, "Year high", 50)
    lv~insertColumn(3, "Year low", 50)
    lv~insertColumn(4, "Description", 120)

    lv~setImageList(imageList, SMALL)

    -- Fill the list-view with random data.
    do row over listData
        lv~addRow( , row[1], row[2], row[3], row[4], row[5], row[6])
    end

    -- Add full row select and the ability to drag and drop the columns to the
    -- list-view.
    lv~addExtendedStyle("FULLROWSELECT HEADERDRAGDROP")

    -- There is a known redrawing problem when a list view is used in a tab
    -- control.  ooDialog has an internal fix for that (see initUpdateListView.)
    -- But, when the list view is the first page of the tab control, the list
    -- view needs to have gained the focus before the dialog is covered up by
    -- another window, for the fix to work.  Assigning the focus here prevents
    -- the very rare occurrence of a user opening the dialog, immediately
    -- switching to another window, and then switching back to the dialog, and
    -- the fix not working.
    lv~assignFocus

/** onActivate
 *
 * Invoked when a list-view item is double-clicked.  We display a message and
 * set the focus to the next item in the list.
 */
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


/** onColumnClick()
 *
 * Invoked when a column header of the list-view is clicked.  We just show a
 * message box so that the user has some feedback.
 */
::method onColumnClick
   use arg id, column

   msg = "Column" column + 1 "was clicked in control" id
   j = MessageDialog(msg, self~hwnd, "List-View Notification")


/** prep()
 *
 * Does some initial set up for the list view dialog.  This is moved out of the
 * initDialog() method to, perhaps, help populate the list view a little
 * quicker.
 */
::method prep
    expose imageList listData

    -- Initialize the internal fix for the list-view redrawing problem when a
    -- list-view is used in a tab control.
    self~initUpdateListView(IDC_LV_MAIN)

    -- Create the image list for the list-view. The image list will consist of
    -- 4 images.  The images are used as the icons for each item in the list
    -- view.  Each item is assigned 1 image, at random, when the item is added
    -- to the list-view.
    --
    -- The list-view control is created without the SHAREIMAGES styles, so it
    -- takes care of releasing the image list when the program ends.
    image = .Image~getImage(.application~srcDir"rc\propertySheetDemoListView.bmp")
    imageList = .ImageList~create(.Size~new(16, 16), COLOR8, 4, 0)
    if \image~isNull,  \imageList~isNull then do
        imageList~add(image)

        -- The image list makes a copy of the bitmap, so we can release it now
        -- to free up some (small) amount of system resources.  This is not
        -- necessary, the OS will release the resource automatically when the
        -- program ends.
        image~release
    end

    -- Create the data for each item (row) in the list view.  The rows are
    -- added to the list view in the initDialog() method
    listData = .array~new(26)
    do ch = "A"~c2d to "Z"~c2d
        q = random(200)
        yh = random(400)
        yh = max(yh, q)
        yl = random(100)
        yl = min(yl, q)
        row = .array~new(6)
        row[1] = random(3)
        row[2] = "_" || ch~d2c~copies(3) || "_"
        row[3] = "$" || q
        row[4] = "$" || yh
        row[5] = "$" || yl
        row[6] = ch~d2c~copies(3) "is a fictitious company."
        listData~append(row)
    end

    -- Connect 2 list-view events to Rexx methods in this dialog.  The double-
    -- click on a list-view item, and the click on a column header events.
    self~connectListViewEvent(IDC_LV_MAIN, "ACTIVATE", "onActivate")
    self~connectListViewEvent(IDC_LV_MAIN, "COLUMNCLICK")


::class 'TreeViewDlg' subclass ResControlDialog

::method initDialog

    -- Instantiate a Rexx tree view object that represents the Windows tree-view
    -- control.
    tv = self~newTreeView(IDC_TV_MAIN)

    -- Create and set the ImageList for the tree view items
    image = .Image~getImage(.application~srcDir"rc\propertySheetDemoTreeView.bmp")
    imageList = .ImageList~create(.Size~new(32, 32), COLOR8, 10, 0)
    if \image~isNull,  \imageList~isNull then do
          imageList~add(image)
          tv~setImageList(imageList, NORMAL)
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


::class 'ProgressBarDlg' subclass RcControlDialog

::method prep
  expose threadsStarted processes

  threadsStarted = 0
  processes = .array~of('animateProgressA', 'animateProgressB', 'animateProgressC',  -
                        'animateProgressD', 'animateProgressE')


-- This message is sent to us by the owner dialog, the .NewControlsDialog dialog
-- to notify us that we are about to become visible.  We use the notification to
-- start the progress bar animation threads.
::method activateThreads unguarded
    expose threadsStarted processes

    reply 0

    -- If no threads are running, start a thread to run each progress bar
    -- asynchronously.
    if threadsStarted < 1 then do
        threadsStarted = processes~items
        do methodName over processes
            self~start(methodName)
        end
    end

-- This is a generic method that simulates some type of processing that takes a
-- long time.  The progress of this processing is displayed by the progress bar.
::method animateProgress unguarded
   use arg progressBar, label, step, iterations, tsleep

   progressBar~setRange(0, iterations * step)
   progressBar~setStep(step)
   do i = 1 to iterations
       progressBar~step
       if (iterations * step == 100) then label~setText(i * step "%")
       else label~setText(i * step)

       call msSleep tsleep
       if \ self~isDialogActive then return
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


::class 'TrackBarDlg' subclass ResControlDialog

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


::class 'TabDlg' subclass ResControlDialog

::method initDialog
   expose font2 font3 imageList iconsRemoved needWrite pb

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
   image = .Image~getImage(.application~srcDir"rc\propertySheetDemoTab.bmp")

   -- Create our image list, as a masked image list.
   flags = 'COLOR24 MASK'
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

  -- Have the tab control calculate its display area's size and position.
  r = tc~windowRect
  tc~calcDisplayRect(r)
  s = .Size~new(r~right - r~left, r~bottom - r~top)

  -- Get the owner draw push button. We'll resize and position it so that it
  -- completely takes up the display area of the tab control.
  pb = self~newPushButton(IDC_PB_OWNERDRAW)

  -- Map the display area's position on the screen, to the client co-ordinates
  -- of this control dialog.
  p = .Point~new(r~left, r~top)
  self~screen2client(p)

  -- Now resize and reposition the button so it exactly over-lays the display
  -- area of the tab control.  We specify that the tab control window is behind
  -- the button and use the flag that prevents the button's owner window, this
  -- z-order from changing.  This leaves the tab control on top of the dialog,
  -- and our push button on top of the tab control.  Which of course is what we
  -- want.
  pb~setWindowPos(tc~hwnd, p~x, p~y, s~width, s~height, "SHOWWINDOW NOOWNERZORDER")



-- When a new tab is selected, we have the owner-drawn button update itself.
-- This causes the button to redraw and the onDrawTabRect() method gets invoked,
-- which actually does the drawing.
::method onTabSelChange
   expose pb
   pb~update


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

   -- Draw a filled in rectangle, with a border of 5 around it, and write text.
   size = button~getRealSize
   button~rectangle(dc, 5, 5, size~width - 5, size~height - 5, "FILL")
   button~writeDirect(dc, trunc(size~width / 4), trunc(size~height / 4), tc~Selected)

   -- Add informative text if needed.
   if needWrite then do
      button~fontToDC(dc, font3)
      x = trunc(size~width / 4)
      y = trunc(size~height / 2)

      if currentTab == 'Gray' then
        button~writeDirect(dc, x, y, "(Tab icons are removed)")
      else
        button~writeDirect(dc, x, y, "(Tab icons are restored)")
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


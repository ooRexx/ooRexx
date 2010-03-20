/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* Name: PROPDEMO.REX                                                       */
/* Type: Object REXX Script                                                 */
/*                                                                          */
/* Description:                                                             */
/* Sample that demonstrates the new classes defined in OODWIN32.CLS         */
/*                                                                          */
/****************************************************************************/

 /* get source directory */
 curdir = directory()
 parse source . . me
 mydir = me~left(me~lastpos('\')-1)             /* install directory */
 mydir = directory(mydir)                       /* current is "my"   */

 dlg = .NewControlsDialog~new(,,,,"WIZARD")    /* create property sheet */
 if dlg~InitCode \= 0 then do; say "Dialog init did not work"; exit; end
 /* create a centered property sheet with 9pt Arial font */
 dlg~createcenter(325, 290, "New Win32 Controls",,,"Arial",9)
 dlg~execute("SHOWTOP")
 dlg~deinstall
 ret = directory(curdir)    /* switch back to stored directory */
 return

::requires "ooDialog.cls"

    /* define subclass of PropertySheet */
::class 'NewControlsDialog' subclass PropertySheet

    /* set the categories: used as tab label, definition method names and inititalization method names */
::method InitCategories
   expose threadstarted
   threadstarted = 0
   self~catalog['names'] = .array~of("List View", "Tree View", "Progress Bar", "Track Bar", "Tab Control")
   self~catalog['page']['font'] = "Arial"   /* use 9pt Arial for the pages as well */
   self~catalog['page']['fsize'] = 9
   /* change wizard button labels and set button width  */
   self~catalog['page']['btnwidth'] = 60
   self~catalog['page']['leftbtntext'] = "&Previous Control"
   self~catalog['page']['rightbtntext'] = "&Next Control"

::method Deinstall
   expose font1 font2 font3 imageList threadstarted
   /* delete created fonts and release the image list */
   self~DeleteFont(font1)
   self~DeleteFont(font2)
   imageList~release

   /* wait until progress bar threads are finished */
   if threadstarted > 0 then call msSleep 500
   self~deinstall:super


/* The following 5 methods are called by DefineDialog to add dialog items to the single cetagory pages */
/* The page layout is loaded from the PROPDEMO.RC resource script */

::method ListView                                        /* page 1 */
   self~loaditems("rc\propdemo.rc", "ReportDialog")

::method TreeView                                        /* page 2 */
   self~loaditems("rc\propdemo.rc", "TreeDialog")

::method ProgressBar                                     /* page 3 */
   self~loaditems("rc\propdemo.rc", "ProgressDialog")

::method TrackBar                                        /* page 4 */
   self~loaditems("rc\propdemo.rc", "SliderDialog")

::method TabControl                                      /* page 5 */
   self~loaditems("rc\propdemo.rc", "TabDialog")


/* The following 4 methods are called by InitDialog to initialize the category pages */
/* InitProgressBar is not defined because there is no need to initialize teh progress bars */

::method InitListView
   self~connectListViewEvent(100,"Activate")     /* connect double-click on a list item with OnActivate */
   self~connectListViewEvent(100,"ColumnClick")  /* connect click on a column header with OnColumnClick */
   lc = self~newListView(100,1)               /* Get an object associated with list control 100 in the first page */
   if lc == .nil then return
   /* Set the column headers */
   lc~InsertColumn(0,"Symbol", 40)
   lc~InsertColumn(1,"Quote", 50)
   lc~InsertColumn(2,"Year high", 50)
   lc~InsertColumn(3,"Year low", 50)
   lc~InsertColumn(4,"Description", 120)

   -- Set the images for the items in the list-view.  The list-view control was
   -- created without the SHAREIMAGES styles, so it take care of releasing the
   -- image list when the program ends.
   image = .Image~getImage("bmp\psdemolv.bmp")
   imageList = .ImageList~create(.Size~new(16, 16), .Image~toID(ILC_COLOR8), 4, 0)
   if \image~isNull,  \imageList~isNull then do
      imageList~add(image)
      lc~setImageList(imageList, .Image~toID(LVSIL_SMALL))

      -- The image list makes a copy of the bitmap, so we can release it now to
      -- free up some (small) amount of system resources.  This is not
      -- necessary, the OS will release the resource automatically when the
      -- program ends.
      image~release
   end

   /* fill the report with random data */
   do ch = "A"~c2d to "Z"~c2d
       q = Random(200)
       call msSleep 1
       yh = Random(400)
       yh = max(yh, q)
       yl = Random(100)
       yl = min(yl, q)
       lc~AddRow(,Random(3),"_" || ch~d2c~copies(3) || "_","$" || q, "$" || yh, "$" || yl, ch~d2c~copies(3) "is a fictitious company.")
   end


::method InitTreeView
   self~connectTreeViewEvent(100,"BeginDrag","DefTreeDragHandler")   /* support drag and drop (default behaviour) */
   tc = self~newTreeView(100)  /* category specifier is not required in InitXXX methods */
   if tc == .nil then return

   /* set images for the items */
   image = .Image~getImage("bmp\psdemotv.bmp")
   imageList = .ImageList~create(.Size~new(32, 32), .Image~toID(ILC_COLOR8), 10, 0)
   if \image~isNull,  \imageList~isNull then do
      imageList~add(image)
      tc~setImageList(imageList, .Image~toID(TVSIL_NORMAL))
      image~release
   end

   /* add the tree */
   tc~Add("Toys",1)       /* this is a root (first argument specified) */
   tc~Add(,"Indoor")      /* this is a subitem (leading arguments omitted) */
   tc~Add(,,"Boys")       -- the last numeric argument in some of the items is
   tc~Add(,,,"Cowboys")   -- the index for the icon in the image list.  Those
   tc~Add(,,,"Cars",8)    -- items without a number will not display an icon.
   tc~Add(,,,"Starwars",9)
   tc~Add(,,"Girls")
   tc~Add(,,,"Barby")
   tc~Add(,,,"Painting")
   tc~Add(,,,"Cooking")
   tc~Add(,,"Adults")
   tc~Add(,,,"Poker")
   tc~Add(,,"Technical")
   tc~Add(,,,"Racing cars",8)
   tc~Add(,,,"Trains",7)
   tc~Add(,"Outdoor")
   tc~Add(,,"Water")
   tc~Add(,,,"Ball",5)
   tc~Add(,,,"Soft tennis",6)
   tc~Add(,,"Sand")
   tc~Add(,,,"Shovel")
   tc~Add(,,,"Bucket")
   tc~Add(,,,"Sandbox")
   tc~Add(,,"Technical")
   tc~Add(,,,"Trains",7)
   tc~Add(,,,"Remote controlled",8)
   tc~Add("Office Articles",2)
   tc~Add(,"Tools")
   tc~Add(,"Books")
   tc~Add(,,"Introduction")
   tc~Add(,,"Advanced Programming")
   tc~Add(,,"Tips & Tricks")
   tc~Add("Hardware",4)
   tc~Add(,"Garden")
   tc~Add(,"Handyman")
   tc~Add(,"Household")
   tc~Add("Furniture",3)
   tc~Add(,"Standard")
   tc~Add(,"Luxury")


::method InitTrackBar
   expose font1
   /* set the initial slider positions (using the associated object attributes) */
   self~Slider1 = 20
   self~Slider2 = 40
   self~Slider3 = 80
   self~vSlider1 = 30
   self~vSlider2 = 90
   self~vSlider3 = 70

   /* connect notification that is sent when a slider is moved with OnEndTrack
      which updates the text labels that display the positions */
   self~connectTrackBarEvent("SLIDER1","EndTrack","OnEndTrack")
   self~connectTrackBarEvent("SLIDER2","EndTrack","OnEndTrack")
   self~connectTrackBarEvent("SLIDER3","EndTrack","OnEndTrack")
   self~connectTrackBarEvent("VSLIDER1","EndTrack","OnEndTrack")
   self~connectTrackBarEvent("VSLIDER2","EndTrack","OnEndTrack")
   self~connectTrackBarEvent("VSLIDER3","EndTrack","OnEndTrack")

   /* Initialize slider SLIDER1 */
   curSL = self~newTrackBar("SLIDER1")
   if curSL \= .nil then curSL~SetTickFrequency(10)

   /* Initialize slider SLIDER2 */
   curSL = self~newTrackBar("SLIDER2")
   if curSL \= .nil then do
     curSL~InitRange(0,200)
     curSL~SetTickFrequency(50)
   end

   /* Initialize slider SLIDER3 */
   curSL = self~newTrackBar("SLIDER3")
   if curSL \= .nil then do
     curSL~InitSelRange(20,60)
     curSL~SetTickFrequency(10)
   end

   /* Initialize slider VSLIDER1 */
   curSL = self~newTrackBar("VSLIDER1")
   if curSL \= .nil then curSL~SetTickFrequency(10)

   /* Initialize slider VSLIDER2 */
   curSL = self~newTrackBar("VSLIDER2")
   if curSL \= .nil then do
     curSL~InitRange(0,400)
     curSL~SetLineStep(5)
     curSL~SetPageStep(50)
     curSL~SetTickFrequency(10)
   end

   /* Initialize slider VSLIDER3 */
   curSL = self~newTrackBar("VSLIDER3")
   if curSL \= .nil then do
     curSL~SetTickFrequency(5)
   end

   /* initialize label values */
   font1 = self~CreateFont("Arial", 24, "BOLD")
   initarray = .array~of(self~Slider1, self~Slider2, self~Slider3, self~vSlider1, self~vSlider2, self~vSlider3)
   do i = 1 to 6
       label = self~newStatic(i+300,4)
       if label \= .nil then do
           label~setText(initarray[i])           /* display initial value */
           if i < 4 then label~setFont(font1)    /* use large font for labels associated with horizontal sliders */
       end
   end



::method InitTabControl
   expose font2 font3 imageList iconsRemoved needWrite

   self~ConnectDraw(200,"OnDrawTabRect")    /* sent when the owner-drawn button is to be redrawn */
   self~connectTabEvent(100, "SELCHANGE", "OnTabSelChange")    /* sent when another tab is selected */
   tc = self~newTab(100,5)
   if tc == .nil then return

   /* font used to display the color name in the owner-drawn button */
   font2 = self~CreateFont("Arial", 48, "BOLD ITALIC")
   /* font used to display some informative text in the owner-drawn button */
   font3 = self~CreateFont("Arial", 16, "BOLD")

   -- Add all the tabs, including the index into the image list for an icon for
   -- each tab.
   tc~AddFullSeq("Red", 0, ,"Green", 1, , "Moss", 2, , "Blue", 3, , "Purple", 4, , "Cyan", 5, , "Gray", 6)

   -- Create a COLORREF (pure white) and load our bitmap.  The bitmap is a
   -- series of 16x16 images, each one a colored letter.
   cRef = .Image~colorRef(255, 255, 255)
   image = .Image~getImage("bmp\psdemoTab.bmp")

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

      -- Set the iconsRemoved and needWrite to false.  These flags are used in
      -- the OnDrawTabRect() method.
      iconsRemoved = .false
      needWrite = .false
   end
   else do
      iconsRemoved = .true
   end


-- Update the static contol that shows the position for a slider when the
-- user is done moving it.
::method onEndTrack
   use arg code, hSlider

   -- hSlider is the handle to the slider that was moved.  Get its resource ID
   -- and get a SliderControl object using that ID.
   id = self~getControlID(hSlider)
   sl = self~newTrackBar(id, 4)

   label = self~newStatic(id + 100, 4)
   label~setText(sl~pos)

   /* Force the owner-drawn button to be redrawn with the selected color */
::method OnTabSelChange
   but = self~newPushButton(200,5)
   if but == .nil then return
   but~update


   /* fill the owner-drawn button with the selected color and display the color name */
::method OnDrawTabRect
   expose font2 font3 imageList iconsRemoved needWrite
   use arg id

   but = self~newPushButton(id,5)
   if but == .nil then return
   tc = self~newTab(100,5)
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

   /* get button's device context, create pen and brush and assign to device context */
   dc = but~GetDC
   pen = but~CreatePen(1, "SOLID", 0)
   oldpen = but~ObjectToDc(dc, pen)
   brush = but~CreateBrush(tc~SelectedIndex+1)
   oldbrush = but~ObjectToDc(dc, brush)
   oldfont = but~FontToDC(dc, font2)
   but~TransparentText(dc)
   /* draw rectangle and write text */
   but~Rectangle(dc, 5,5,but~SizeX * but~FactorX - 10, but~SizeY * but~FactorY - 10,"FILL")
   but~WriteDirect(dc,30,50,tc~Selected)

   -- Add informative text if needed.
   if needWrite then do
      but~FontToDC(dc, font3)
      if currentTab == 'Gray' then but~WriteDirect(dc, 30, 120, "(Tab icons are removed)")
      else but~WriteDirect(dc, 30, 120, "(Tab icons are restored)")
      needWrite = .false
   end

   /* restore pen, brush, and font and release device context */
   but~FontToDC(dc, oldfont)
   but~OpaqueText(dc)
   but~ObjectToDc(dc, oldpen)
   but~DeleteObject(pen)
   but~ObjectToDc(dc, oldbrush)
   but~DeleteObject(brush)
   but~FreeDC(dc)


   /* check if page 3 (progress bar category) was selected and start progress bar animation threads */
::method PageHasChanged
   expose threadstarted
   if self~CurrentCategory = 3 then do
       if threadstarted < 1 then do
           threadstarted = 5
           /* start a thread for each progress bar to run asynchronously */
           do i = 1 to 5; self~Start("AnimateProgress" || i); end
       end
   end


   /* generic method to simulate a process of which the progress is displayed by a progress bar */
::method AnimateProgress unguarded
   use arg id, step, iterations, tsleep
   pb = self~newProgressBar(id,3)
   lab = self~newStatic(id+200,3)
   if pb \= .nil & lab \= .nil then do
      pb~SetRange(0, iterations*step)
      pb~SetStep(step)
      do i = 1 to iterations
          pb~Step
          if (iterations*step = 100) then lab~Title = i*step "%"
          else lab~Title = i*step
          call msSleep tsleep
          if self~Finished \= 0 then return
          if self~IsDialogActive = 0 then return
      end
   end

   /* this following 5 methods are started asynchronously and animate the 5 progress bars */
::method AnimateProgress1 unguarded
   expose threadstarted
   self~AnimateProgress(101,5, 20, 600)
   threadstarted = threadstarted - 1

::method AnimateProgress2 unguarded
   expose threadstarted
   self~AnimateProgress(102,1, 100, 150)
   threadstarted = threadstarted - 1

::method AnimateProgress3 unguarded
   expose threadstarted
   self~AnimateProgress(103,2, 50, 200)
   threadstarted = threadstarted - 1

::method AnimateProgress4 unguarded
   expose threadstarted
   self~AnimateProgress(104, 10, 40, 300)
   threadstarted = threadstarted - 1

::method AnimateProgress5 unguarded
   expose threadstarted
   self~AnimateProgress(105,20, 50, 500)
   threadstarted = threadstarted - 1


   /* called when an item of report is double-clicked to display a message and focus the next item in the list */
::method OnActivate
   lc = self~newListView(100,1)
   if lc == .nil then return
   si = lc~Focused
   order = askDialog("You have selected" lc~ItemText(si)". Do you want to order 50 stocks at" lc~ItemText(si,1) "? ")
   lc~Deselect(si)
   lc~Focus(si+1)
   lc~Select(si+1)


   /* called when a column header of the report is clicked */
::method OnColumnClick
   use arg id, column
   call infoDialog "Column" column+1 "was clicked in control" id

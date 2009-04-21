/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* OODialog\Samples\ooticket.rex  Let's go to the movies - Cat.dialog       */
/*                                                                          */
/*--------------------------------------------------------------------------*/

 curdir = directory()
 parse source . . me
 mydir = me~left(me~lastpos('\')-1)              /* where is code     */
 mydir = directory(mydir)                        /* current is "my"   */

 -- Let the user select a font for the program.
 oldFont = setFont()

 dlg = .TimedMessage~new("The upcoming dialog demonstrates the use of categorized pages", ,
                         "Categorized Dialog", 2000)
 dlg~execute
 drop dlg

 dlstyle.1 = "Default"
 dlstyle.2 = "Topline"
 dlstyle.3 = "Dropdown"
 dlstyle.4 = "Wizard"
 dlstyle.5 = "Topline and Wizard"
 dlstyle.6 = "Dropdown and Wizard"

 dlg = .SingleSelection~new("Please select a style for the category dialog","Dialog Style",dlstyle.,2)
 s = dlg~execute

 if (s = 0) then do
    call errorDialog "Selection error!";
    dlg~deinstall
    ret = directory(curdir)
    ret = restoreFont(oldFont)
    return
 end

 posx = 0
 title = .Nil
 addsize = 0
 select
    when s = 1 then do
       dlgst = ""
       posx = 2
       addsize = 70
    end
    when s = 2 then do
       dlgst = "TOPLINE"
    end
    when s = 3 then do
       dlgst = "DROPDOWN"
       posx = 5
       title = "Dialog panel:"
    end
    when s = 4 then do
       dlgst = "WIZARD"
       posx = 2
       addsize = 70
    end
    when s = 5 then do
       dlgst = "TOPLINE WIZARD"
    end
    when s = 6 then do
       dlgst = "DROPDOWN WIZARD"
       posx = 5
       title = "Dialog panel:"
    end
    otherwise
    do
      dlg~deinstall
      ret = directory(curdir)
      ret = restoreFont(oldFont)
      return
    end
 end

 dlg = .TicketDialog~new(data.,posx,,,dlgst,,title)

 if dlg~InitCode \= 0 then do; say "Dialog init did not work"; exit; end

 dlg~createcenter(200+addsize, 180, "Let's Go to the Movies")
 dlg~execute("SHOWTOP")
 dlg~deinstall
 ret = directory(curdir)
 ret = restoreFont(oldFont)
 return

/*-------------------------------- requires -----------------------------------*/

::requires "oodWin32.cls"

/*-------------------------------- Ticket Dialog ------------------------------*/

::class 'TicketDialog' subclass CategoryDialog inherit AdvancedControls

::method InitDialog
   self~InitDialog:super
   self~setFilmData

::method InitCategories
   self~catalog['names'] = .array~of("Movies", "Cinemas", "Days", "Ticket")
    /* set the width of the button row at the bottom to 35 */
   self~catalog['page']['btnwidth'] = 35
        /* change name of wizard buttons, default is &Backward and &Forward */
   self~catalog['page']['leftbtntext'] = "&Previous"
   self~catalog['page']['rightbtntext'] = "&Next"

::method Movies                                      /* page 1 */
   self~loaditems("rc\movies.rc")
   self~connectList(31,"MovieClick")

::method Cinemas                                     /* page 2 */
   expose cinema.
   cinema.1 = "&Downtown San Jose Cinemas"
   cinema.2 = "&Palo Alto News Scene"
   cinema.3 = "&Charlie's Suburban Barn Theatre"
   cinema.4 = "&Drive-in above Highway 101"
   cinema.5 = "&Premiere Black + White Movies Monterey"
   cinema.6 = "&Broadway Cinema San Francisco"
   self~AddCheckBoxStem(51, 25, 10, 0, cinema., 6)
   self~AddText(10,self~SizeY - 65,0,0,"Make your choice of one or more cinemas you prefer")
   self~AddBlackFrame(1, self~SizeY -68, self~catalog['page']['w'] - 2, 14)

::method Days                                        /* page 3 */
   expose daynames
   self~AddText(10,self~SizeY - 65,0,0,"Please select the day you like most")
   self~AddRadioGroup(31, 5, 5,0, "&Monday &Tuesday &Wednesday T&hursday &Friday &Saturday S&unday")
   self~AddBlackFrame(1, self~SizeY -68, self~catalog['page']['w'] - 2, 14)
   self~addImage(145, 73, 10, 125, 100, "BITMAP SIZEIMAGE CENTERIMAGE")
   daynames = .array~of('Monday','Tuesday','Wednesday','Thursday','Friday','Saturday','Sunday')

::method InitDays
  staticImage = self~getStaticControl(145)
  parse value staticImage~getRect with x y x2 y2
  size = .Size~new(x2 - x, y2 - y)
  image = .Image~getImage('bmp\movie.bmp', .Image~toID(IMAGE_BITMAP), size)
  staticImage~setImage(image)

::method Ticket                                      /* page 4 */
   self~loaditems("rc\ticket.rc")
   self~connectList(41,"FilmClick")

   if .DlgUtil~comCtl32Version < 6 then do
      self~addBitmapButton(45, 13, 87, 102, 40, "Get the Ticket", 'printTicket', -
                           "bmp\ticket.bmp",,,, "FRAME USEPAL STRETCH GROUP")
   end
   else do
      self~addButton(45, 13, 87, 102, 40, "", 'printTicket', "GROUP")
   end

::method InitTicket
   if .DlgUtil~comCtl32Version  < 6 then return

   bmpButton = self~getButtonControl(45)
   parse value bmpButton~getRect with x y x2 y2

   size = .Size~new(x2 - x - 10, y2 - y - 10)
   image = .Image~getImage('bmp\ticket.bmp', .Image~toID(IMAGE_BITMAP), size)
   imageList = .ImageList~create(size, .Image~toID(ILC_COLOR8), 1, 0)
   imageList~add(image)

   align = .Image~toID(BUTTON_IMAGELIST_ALIGN_CENTER)
   margin = .Rect~new(5)
   bmpButton~setImageList(imageList, margin, align)

::method SetFilmData
   expose sel. films
   films = .array~of("Vertigo","Taxi Driver","Superman","Larger than Life",,
                     "Hair","Cinderella","True Lies","E.T.",,
                     "Twister","Lawnmower Duel")
   do i = 1 to films~items
      self~addCategoryListEntry(31, films[i], 1)
   end
   sel.1.32 = "Paramount";      sel.1.33 = "Kim Novak";         sel.1.34 = "Alfred Hitchcock"
   sel.1.35 = "James Stewart"
   sel.2.32 = "Universal";      sel.2.33 = "Robert De Niro";    sel.2.34 = "Martin Scorsese"
   sel.2.35 = "Jodie Foster,Albert Brooks,Harvey Keitel,Cybill Shepherd"
   sel.3.32 = "Warner Bros";    sel.3.33 = "Christopher Reeves";sel.3.34 = "Richard Donner"
   sel.3.35 = "Marlon Brando,Gene Hackman,Margot Kidder"
   sel.4.32 = "United Artists"; sel.4.33 = "The Elephant";      sel.4.34 = "Howard Franklin"
   sel.4.35 = "Bill Murray"
   sel.5.32 = "Hippies";        sel.5.33 = "John Savage";       sel.5.34 = "Milos Forman"
   sel.5.35 = "Beverly D'Angelo,Dorsey Wright,Cheryl Barnes,Treat Williams,Annie Golden"
   sel.6.32 = "Disney";         sel.6.33 = "Cinderella";        sel.6.34 = "Walt"
   sel.6.35 = "Step mother,Witch,Prince,King"
   sel.7.32 = "20. Century Fox";sel.7.33 = "Arnold Schwarzenegger"; sel.7.34 = "James Cameron"
   sel.7.35 = "Jamie Lee Curtis,Tom Arnold,Richard Harris"
   sel.8.32 = "United Artists"; sel.8.33 = "The Extra-Terrestrial"; sel.8.34 = "Steven Spielberg"
   sel.8.35 = "Henry Thomas,Drew Barrymore,Dee Wallace,Peter Coyote"
   sel.9.32 = "Warner Bros";    sel.9.33 = "Helen Hunt";        sel.9.34 = "Jan De Bont"
   sel.9.35 = "Bill Paxten"
   sel.10.32= "Sunset Mowers";  sel.10.33= "The Lawn Mower";    sel.10.34= "Mower Pusher"
   sel.10.35= "Lawn,Garden hose,Sprinkler,Shredder"

::method MovieClick
   expose sel.
   f = self~getCurrentCategoryListIndex(31,1)
   self~setCategoryEntryLine(32,sel.f.32,1)
   self~setCategoryEntryLine(33,sel.f.33,1)
   self~setCategoryEntryLine(34,sel.f.34,1)
   self~CategoryComboDrop(35,1)
   txt = sel.f.35
   do until txt = ''
      parse var txt actor ',' txt
      self~addCategoryComboEntry(35,actor,1)
   end
   self~setCurrentCategoryComboIndex(35,1,1)

::method run
   expose films daynames today
   self~setCategoryMultiList(31,'1',1)
   self~MovieClick
   today = date('W')       /* set today on page 3 - days */
   do id=31 to 37
      if today = daynames[id-30] then self~setCategoryRadioButton(id,1,3)
   end
   self~run:super

::method OK
   page = self~CurrentCategory
   if page \= 4 then self~changePage(page+1)
                else self~printTicket

/* -------------------------------------------------------------------------- */
/* PageHasChanged:                                                            */
/* this method will be called every time the category page is changed         */
/* if this method is ommited the one from OOdialog is called instead          */
/* -------------------------------------------------------------------------- */

::method PageHasChanged
   expose films
   NewPage = self~CurrentCategory
   if NewPage \= 4 then return
   /* films */
      self~categoryComboDrop(41, 4)
      Lines = self~getCategoryValue(31, 1)
      if Lines = '' then do
              self~addCategoryComboEntry(41,"==> No movie was selected",4)
              self~setCurrentCategoryComboIndex(41,1,4)
           end
      else do
           do i = 1 to words(Lines)
               self~addCategoryComboEntry(41, films[word(Lines,i)] ,4)
           end
           self~setCategoryComboLine(41, films[word( Lines,random(1,words(Lines)) )] ,4)
      end
      self~FilmClick

::method FilmClick
   expose cinema. daynames selectedCinema
   /* cinemas */
      selectedCinema = "==> No cinema was selected"
      self~setCategoryStaticText(44, , 4)
      ic = random(1,6)
      do i=ic to ic+5
         id = i // 6 + 1
         if self~getCategoryCheckBox(50+id,2) then do
            selectedCinema = cinema.id
            leave
         end
      end
      self~setCategoryStaticText(44, selectedCinema, 4)
   /* days */
      do id=31 to 37
         if self~getCategoryRadioButton(id,3) then leave
      end
      today = date('W')
      if today = daynames[id-30] then
           self~setCategoryStaticText(42, "Today" daynames[id-30], 4)
      else self~setCategoryStaticText(42, "Next" daynames[id-30], 4)
   /* time */
      id = random(1,4) + 45
      self~setCategoryRadioButton(id,1,4)

::method printTicket
   selectedFilm   = self~getCategoryComboLine(41,4)
   selectedCinema = self~getCategoryValue(44,4)
   if selectedFilm~left(3) = '==>' | selectedCinema~left(3) = '==>' then
        ret = TimedMessage("You have to select a movie and a cinema first", ,
                           "Incomplete Selections", 3000)
   else ret = TimedMessage("This is where we would ask for money!.... and print the ticket", ,
                           selectedFilm '-at-' selectedCinema~substr(2), 3000)


::routine setFont

  oldFont = .directory~new
  oldFont~name = .PlainBaseDialog~getFontName
  oldFont~size = .PlainBaseDialog~getFontSize

  dlg = .FontPicker~new("rc\movies.rc", IDD_FONT_PICKER, , , , 6)
  if dlg~initCode == 0 then do
     dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
     dlg~deinstall
  end

  return oldFont

::routine restoreFont
  use strict arg oldFont
  .PlainBaseDialog~setDefaultFont(oldFont~name, oldFont~size)
  return oldFont

::class 'FontPicker' subclass RcDialog inherit AdvancedControls MessageExtensions

::method initAutoDetection
   self~noAutoDetection

::method initDialog
  expose nameCB sizeCB

  nameCB = self~getComboBox(IDC_COMBO_NAME)
  sizeCB = self~getComboBox(IDC_COMBO_SIZE)

  names = .array~of("Default", "Tahoma", "Courier", "MS Sans Serif")
  sizes = .array~of("Default", '8 point', '10 point', '12 point', '16 point')

  do name over names
     nameCB~add(name)
  end
  nameCB~select("Default")

  do size over sizes
     sizeCB~add(size)
  end
  sizeCB~select("Default")

::method ok
  expose nameCB sizeCB

  fontName = nameCB~selected
  if fontName == "Default" then fontName = .PlainBaseDialog~getFontName

  fontSize = sizeCB~selected~word(1)
  if fontSize == "Default" then fontSize = .PlainBaseDialog~getFontSize

  .PlainBaseDialog~setDefaultFont(fontName, fontSize)
  return self~ok:super

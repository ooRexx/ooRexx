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

/** Propery Sheet / Wizard97 example
 *
 * This example demonstrates how to use the PropertySheetDialog to create a
 * Wizard.  There are two main wizard styles for a property sheet, the Wizard97
 * style and the Aero style.  Wizard97 will run on any Windows version supported
 * by ooRexx.  The Aero wizard needs Windows Vista or later.
 *
 * This example creates all the pages as UserDialogs where the programmer lays
 * each dialog template in the defineDialog() method.  This is really rather
 * tedious.  It is much easier to use a resource editor, especially here where
 * the programmer wants to create a number of page dialogs that are all the same
 * size.
 *
 * However, it is a good opportunity to give examples of how to correctly
 * convert between pixels and dialog units in the defineDialog() method to get
 * the dialog template laid out correctly when different fonts are used for the
 * dialog.  To facilate that part of the example, the user is allowed to pick
 * different fonts and font sizes for the wizard before it is displayed.  This
 * may not be the best sample program for that, because it is not possible, at
 * this time, to change the font of the wizard itself.
 *
 * This is a rather complex program.  The commenting is not 100%.  Be sure to
 * read the ooDialog documentation on the PropertySheetDialog to fully under-
 * stand this program.
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

  -- Let the user select a font for the example program.
  oldFont = setFont()

  -- We show a timed message to explain what the upcoming example is about.
  -- This is a hold-over from the original ooRexx examples.  The reality is, it
  -- gets rather annoying after the first 2 or 3 times you run the example.  I
  -- would prefer to eliminate this code.
  msg = "The upcoming dialog demonstrates a Wizard 97 style dialog."
  title = "Wizard 97 Dialog"
  dlg = .TimedMessage~new(msg, title, 2000)
  dlg~execute

  -- First we create the dialogs for each page, and set any of the attributes
  -- we need to.  We want each dialog to be the same size.
  outPageSize = .Size~new(267, 193)
  inPageSize = .Size~new(267, 143)

  intro = .IntroDlg~new( , "rc\ticketWizard.h", "", "ooRexx Movie Ticket Selectitron")
  intro~setSize(outPageSize)

  p1 = .MoviesDlg~new( , "rc\ticketWizard.h", "", "Movies")
  p1~setSize(inPageSize)
  p1~headerTitle = "Select the movie(s) you want tickets for."
  p1~headerSubtitle = "When a movie is selected, information on the movie, such as director, actors, " -
                      "etc., will also be displayed."

  p2 = .CinemasDlg~new( , "rc\ticketWizard.h", "", "Cinemas" )
  p2~setSize(inPageSize)
  p2~headerTitle = "Pick a movie theater for the movie(s)."
  p2~headerSubtitle = "Pick a theater for each movie you've selected.  Use the drop down list to switch movies.  " -
                      "Theaters in your vicinity showing the movie(s) this week are listed.  "

  p3 = .DaysDlg~new( , "rc\ticketWizard.h", "", "Days" )
  p3~setSize(inPageSize)
  p3~headerTitle = "Choose a day to watch the movie(s)."
  p3~headerSubtitle = "Use the drop down list to switch movies.  Pick the day most suitable for the movie."

  p4 = .TicketDlg~new( , "rc\ticketWizard.h", "", "Ticket Information" )
  p4~setSize(inPageSize)
  p4~headerTitle = "Validate your selections and indicate the time desired."
  p4~headerSubtitle = "Pick a time for each movie selected.  Ticket information for each movie can be displayed " -
                      "using the drop down list. To change any selections use the Back button."

  complete = .CompletionDlg~new( , "rc\ticketWizard.h", "", "Enjoy Your Movie")
  complete~setSize(outPageSize)

  pages = .array~of(intro, p1, p2, p3, p4, complete)

  -- Create the property sheet using the dialog pages and set its attributes.
  wizDlg = .TicketWizard97~new(pages, "Wizard97", "Let's Go To The Movies", "rc\ticketWizard.h")

  wizDlg~header = .Image~getImage("bmp\ticketWizardTheater.bmp")
  wizDlg~watermark = .Image~getImage("bmp\ticketWizardRexxLA.bmp")

  -- Execute the wizard.
  wizDlg~execute

  -- Return to the directory we started from and restore the default font.
  ret = directory(curdir)
  ret = restoreFont(oldFont)
  return

::requires "ooDialog.cls"

-- TicketWizard97
--   The property sheet for the example program.  Although the
--   PropertySheetDialog has a number of methods, we do not need to over-ride
--   any of the methods.  So, we just need the class definition line here.
::class 'TicketWizard97' subclass PropertySheetDialog


-- IntroDlg
--   Dialog for first page (exterior page, introduction.)
::class 'IntroDlg' subclass UserPSPDialog

::method defineDialog

    -- Watermark bitmap is 164 x 316.  The wizard does not stretch the bitmap,
    -- its size remains constant.  So, the size taken up by the bitmap, in
    -- dialog units, changes as the font size changes.  Use pixel2dlgUnit() to
    -- convert the pixel size of the watermark to the correct dialog units for
    -- the font this dialog is using.
    s = .Size~new(164, 314)
    self~pixel2dlgUnit(s)

    x  = s~width + 6
    y = 8                     -- From the Wizard97 specification.
    cx = self~sizeX - x       -- To the right side of the template, the wizard automatically adds 14 dlus.
    cy = 3 * 12               -- Using 12 point font, takes 3 lines for the msg.

    self~createStaticText(IDC_ST_INTRO_MSG, x, y, cx, cy, "", "")

    y += cy + 8               -- Space the next static 8 dlus below the first.
    cy = 8 * 8                -- Arbitray height to fit the info text.
    self~createStaticText(IDC_ST_INTRO_INFO, x, y, cx, cy, "", "")

    cy = 8                    -- Normal font size height.
    y  = self~sizeY - 7 - cy  -- From the Wizard97 specification, leave 7 dlus margin on bottom.
    self~createStaticText(IDC_ST_INTRO_LAST_MSG, x, y, cx, cy, "", "")

::method setActive unguarded
  use arg propSheet

  -- Back button disabled, Next button enabled.
  propSheet~setWizButtons("NEXT")
  return 0

::method initDialog

    msg = "Welcome to the ooRexx Movie Ticket Selectitron Wizard"
    font1 = self~createFontEx("Verdana", 12, "BOLD")  -- From the Wizard97 specification.

    st1 = self~newStatic(IDC_ST_INTRO_MSG)
  st1~setFont(font1)
  st1~setText(msg)

    info = "This wizard will help you select a ticket, or tickets, for the movie(s) of your choice.  For each" -
           "ticket you will specify the theater and time for the movie you wish to see." || .endOfLine~copies(2) || -
           "Tickets for any movies playing in the upcoming week are available.  Select as many" -
           "movies as you wish to see."
    self~newStatic(IDC_ST_INTRO_INFO)~setText(info)

    msg = "Select the Next button to continue"
    self~newStatic(IDC_ST_INTRO_LAST_MSG)~setText(msg)


-- MoviesDlg
--   Dialog for second page (interior page, movie selection.)
::class 'MoviesDlg' subclass UserPSPDialog

::method defineDialog

    x = 21                     -- From the Wizard97 specification.
    y = 1                      -- From the Wizard97 specification, the Wizard
                               -- automatically adds a 7 dlu marging on all sides.
    self~createGroupBox(-1, x, y, 113, 125, , "The upcoming movies")
    self~createListBox(31, x + 10, y + 15, 93, 105, "MULTI NOTIFY GROUP", "MovieList")

    x += 125 + 10
    self~createStaticText(-1, x, y, 60, 8, , "Produced by:")
    self~createEdit(32, x, y + 12, 84, 12, "READONLY GROUP", "Produced")

    y += 15 * 2
    self~createStaticText(-1, x, y, 60, 8, , "Starring:")
    self~createEdit(33, x, y + 12, 84, 12, "READONLY GROUP", "Star")

    y += 15 * 2
    self~createStaticText(-1, x, y, 60, 8, , "Director:")
    self~createEdit(34, x, y + 12, 84, 12, "READONLY GROUP", "Director")

    y += 15 * 2
    self~createStaticText(-1, x, y, 60, 8, , "With:")
    self~createComboBox(35, x, y + 12, 84, 33, "LIST GROUP", "COMBOBOX")

    self~connectListBoxEvent(31, "SELCHANGE", "onMovieClick")

::method pageCreate unguarded
    self~setMovieData
    return .true

::method setMovieData private
    expose movieData films lastSelected

    films = .array~of("Vertigo", "Taxi Driver", "Superman", "Larger than Life",  -
                      "Hair", "Cinderella", "True Lies", "E.T.", "Twister", "Lawnmower Duel")
    movieData = .table~new

    do i = 1 to films~items
        d = .directory~new
        movieData[films[i]] = d
    end

    d = movieData[films[1]]
    d~produced = "Paramount"; d~star = "Kim Novak"; d~director = "Alfred Hitchcock"
    d~with = .array~of("James Stewart")

    d = movieData[films[2]]
    d~produced = "Universal"; d~star = "Robert De Niro"; d~director = "Martin Scorsese"
    d~with = .array~of("Jodie Foster", 'Albert Brooks', 'Harvey Keitel', 'Cybill Shepherd')

    d = movieData[films[3]]
    d~produced = "Warner Bros"; d~star = "Christopher Reeves"; d~director = "Richard Donner"
    d~with = .array~of('Marlon Brando', 'Gene Hackman', 'Margot Kidder')

    d = movieData[films[4]]
    d~produced = "United Artists"; d~star = "The Elephant"; d~director = "Howard Franklin"
    d~with = .array~of("Bill Murray")

    d = movieData[films[5]]
    d~produced = "Hippies"; d~star = "John Savage"; d~director = "Milos Forman"
    d~with = .array~of("Beverly D'Angelo", 'Dorsey Wright', 'Cheryl Barnes', 'Treat Williams', 'Annie Golden')

    d = movieData[films[6]]
    d~produced = "Disney"; d~star = "Cinderella"; d~director = "Walt"
    d~with = .array~of('Step mother', 'Witch', 'Prince', 'King')

    d = movieData[films[7]]
    d~produced = "20. Century Fox"; d~star = "Arnold Schwarzenegger"; d~director = "James Cameron"
    d~with = .array~of('Jamie Lee Curtis', 'Tom Arnold', 'Richard Harris')

    d = movieData[films[8]]
    d~produced = "United Artists"; d~star = "The Extra-Terrestrial"; d~director = "Steven Spielberg"
    d~with = .array~of('Henry Thomas', 'Drew Barrymore', 'Dee Wallace', 'Peter Coyote')

    d = movieData[films[9]]
    d~produced = "Warner Bros"; d~star = "Helen Hunt"; d~director = "Jan De Bont"
    d~with = .array~of("Bill Paxten")

    d = movieData[films[10]]
    d~produced= "Sunset Mowers"; d~star = "The Lawn Mower"; d~director = "Mower Pusher"
    d~with = .array~of('Lawn', 'Garden hose', 'Sprinkler', 'Shredder')

    self~movieList = 0

::method setActive unguarded
  use arg propSheet

  propSheet~setWizButtons("BACK NEXT")
  return 0

-- wizNext()
--   The user clicked the next button.  If the user does not pick a movie, we
--   don't let her proceed to the next page.  There is no point in using the
--   wizard without selecting a movie.   We use wizNext() rather than
--   killActive() because killActive is invoked both when the user clicks the
--   Back and the Next buttons.  If the user wants to go back to the intor page,
--   we don't want to put up an annoying message box.
::method wizNext
  expose lb
  use arg propSheet

  count = lb~selectedItems
  if count < 1 then do
    msg = "To purchase a ticket, you must select at".endOfLine"least one movie."
    j = MessageDialog(msg, propSheet~hwnd, "ooRexx Movie Ticket Selectitron", "OK", "INFORMATION")
    return -1
  end

  return 0

::method initDialog
    expose films lb

    lb = self~newListBox(31)
    do i = 1 to films~items
        lb~add(films[i])
    end

    lb~deselectIndex

::method onMovieClick
   expose movieData lb

   movieName = lb~selected

   combo = self~newComboBox(35)
   combo~deleteAll

   -- If no movie in the list box is selected, set eveything blank, otherwise,
   -- set everything to match the selected movie.
   if movieName == "" then do
     self~setEditData(32, "")
     self~setEditData(33, "")
     self~setEditData(34, "")
     combo~add("")
   end
   else do
     d = movieData[movieName]
     self~setEditData(32, d~produced)
     self~setEditData(33, d~star)
     self~setEditData(34, d~director)

     do n over d~with
        combo~add(n)
     end
     combo~selectIndex(1)
   end

::method queryFromSibling unguarded
  expose lb films
  use arg arg1, arg2, propSheet

  select
    when arg2 == "FILMS" then do
      -- arg1 is an array that we fill in with all the movies
      arg1~empty
      do i = 1 to films~items
        arg1[i] = films[i]
      end
    end

    when arg2 == "SELECTED" then do
      -- arg1 is a table, we set an index for each selected movie.  Note that
      -- the item for the
      arg1~empty
      currentSelectedMovies = self~getSelectedMovies
      do i = 1 to currentSelectedMovies~items
        arg1[currentSelectedMovies[i]] = .true
      end
    end

    when arg2 == "DATA" then do
      -- arg1 is a table whose indexes are the films.  Each item is a directory
      -- indexes are theater, date, and time.  The other pages fill in the
      -- directory.  We just make sure only the selected movies are in the
      -- table.
      arg1~empty
      currentSelectedMovies = self~getSelectedMovies

      do movie over currentSelectedMovies
        d = .directory~new
        arg1[movie] = d
      end
    end

    otherwise do
      -- We do not handle any other queries.
      nop
    end
  end
  -- End select

  return 0

::method getSelectedMovies unguarded
  expose lb

  count = lb~selectedItems
  if count < 1 then return .array~new

  a = .array~new(count)
  indexes = lb~selectedIndexes
  do i over indexes~makeArray(" ")
    a~append(lb~getText(i))
  end

  return a



-- CinemasDlg
--  Dialog for the third page (interior page, theater selection.)
::class 'CinemasDlg' subclass UserPSPdialog

::method defineDialog
   expose buttonTable

   x = 21; y = 1
   self~createStaticText(-1, x, y, 100, 8, , "Selected movies:")
   self~createComboBox(IDC_COMBO_CINEMAS, x, y + 12, 170, 75, "LIST SORT", "COMBOBOX")

   y += 15 * 2
   self~createGroupBox(-1, x, y, 170, 105, "", "Theaters")

   y += 10; x += 10
   self~createRadioButton(IDC_RB_CINEMAS_UPTOWN, x, y, 145, 12, "TABSTOP", "&Uptown San Jose Cinemas")

   y += 15
   self~createRadioButton(IDC_RB_CINEMAS_PALO, x, y, 145, 12, "", "&Palo Alto News Scene")

   y += 15
   self~createRadioButton(IDC_RB_CINEMAS_CHARLIES, x, y, 145, 12, "", "&Charlie's Suburban Barn Theater")

   y += 15
   self~createRadioButton(IDC_RB_CINEMAS_DRIVEIN, x, y, 145, 12, "", "&Drive-in above Highway 101")

   y += 15
   self~createRadioButton(IDC_RB_CINEMAS_GRAND, x, y, 145, 12, "", "&Grand Black n White Movies Monterey")

   y += 15
   self~createRadioButton(IDC_RB_CINEMAS_BROADWAY, x, y, 145, 12, "", "&Broadway Cinema San Francisco")

   first = self~resolveSymbolicID(IDC_RB_CINEMAS_FIRST)
   last = self~resolveSymbolicID(IDC_RB_CINEMAS_LAST)

   self~connectComboBoxEvent(IDC_COMBO_CINEMAS, "SELENDOK", "onComboUpdate")
   do i = first to last
     self~connectButtonEvent(i, "CLICKED", onRBClick)
   end

   buttonTable = .table~new
   buttonTable["Uptown San Jose Cinemas"]             = IDC_RB_CINEMAS_UPTOWN
   buttonTable["Palo Alto News Scene"]                = IDC_RB_CINEMAS_PALO
   buttonTable["Charlie's Suburban Barn Theater"]     = IDC_RB_CINEMAS_CHARLIES
   buttonTable["Drive-in above Highway 101"]          = IDC_RB_CINEMAS_DRIVEIN
   buttonTable["Grand Black n White Movies Monterey"] = IDC_RB_CINEMAS_GRAND
   buttonTable["Broadway Cinema San Francisco"]       = IDC_RB_CINEMAS_BROADWAY

::method initDialog
  expose filmArray movieTheaters selectedMovies movieCombo

  filmArray = .array~new(20)
  self~propSheet~querySiblings(filmArray, "FILMS")

  movieTheaters = .table~new
  do movie over filmArray
    movieTheaters[movie] = .nil
  end

  selectedMovies = .array~new

  movieCombo = self~newComboBox(IDC_COMBO_CINEMAS)

::method setActive unguarded
  expose filmArray movieTheaters selectedMovies movieCombo
  use arg propSheet

  propSheet~setWizButtons("BACK NEXT")

  selected = .table~new
  selectedMovies~empty
  propSheet~querySiblings(selected, "SELECTED")

  movieCombo~deleteAll

  -- For every movie, check if it is now selected.  If it is, we add it to the
  -- selected moveies combo box.  If it isn't, we mark that movie as not having
  -- a movie theater selected.  We don't care what the previous setting was.
  do movie over filmArray
    if selected~hasIndex(movie) then do
      movieCombo~add(movie)
      selectedMovies~append(movie)
    end
    else do
      movieTheaters[movie] = .nil
    end
  end

  if selected~items > 0 then do
    movieCombo~selectIndex(1)
    movie = movieCombo~selected
    self~setRadioButtons(movie)
  end

  return 0

-- wizNext()
--   The user clicked the next button.  If there is not a theater selected for
--   each selected movie, we warn the user.  We use wizNext() rather than
--   killActive() because killActive is invoked both when the user clicks the
--   Back and the Next buttons.  If the user is going back, we don't want to put
--   up an annoying message box.
::method wizNext unguarded
  expose selectedMovies movieTheaters movieCombo
  use arg propSheet

  do movie over selectedMovies
    if movieTheaters[movie] == .nil then do
      msg = "Warning.  You have not selected a movie theater for all".endOfLine"selected movies." || -
            .endOfLine~copies(2) || "Are you sure you want to go to the next page?"
      title = "Theater Selection is not Complete"
      buttons = "YESNO"
      icon = "WARNING"
      style = "DEFBUTTON2 TASKMODAL"

      ans = MessageDialog(msg, propSheet~dlgHandle, title, buttons, icon, style)

      if ans == self~constDir[IDYES] then return 0

      movieCombo~select(movie)
      self~setRadioButtons(movie)
      return -1
    end
  end

  return 0

::method queryFromSibling unguarded
  expose movieTheaters
  use arg arg1, arg2, propSheet

  select
    when arg2 == "DATA" then do
      -- arg1 is a table whose indexes are the films.  Each item is a directory
      -- indexes are theater, date, and time.  We only fill in the theater index
      -- for any movie in the table.  We do not care what our status is for the
      -- theater, we just fill it in.

      do movie over arg1
        d = arg1[movie]
        d~theater = movieTheaters[movie]
      end
    end

    otherwise do
      -- We do not handle any of the other queries.
      nop
    end
  end
  -- End select

  return 0

::method onComboUpdate
  expose movieTheaters movieCombo

  movie = movieCombo~selected
  self~setRadioButtons(movie)

::method onRBClick
  expose movieTheaters movieCombo
  use arg id, hwnd

  movie = movieCombo~selected
  movieTheaters[movie] = self~newRadioButton(id)~getText~changestr("&", "")

::method setRadioButtons private
  expose movieTheaters buttonTable
  use arg movie

  movieTheater = movieTheaters[movie]

  if movieTheater == .nil then id = 0
  else id = buttonTable[movieTheater]

  .RadioButton~checkInGroup(self, IDC_RB_CINEMAS_FIRST, IDC_RB_CINEMAS_LAST, id)



-- DaysDlg
--  Dialog for the fourth page (interior page, date selection.)
::class 'DaysDlg' subclass UserPSPdialog

::method defineDialog
   expose buttonTable

   dayNames = .array~of('&Monday', '&Tuesday', '&Wednesday', 'T&hursday', '&Friday', '&Saturday', 'S&unday')

   x = 21; y = 1
   self~createStaticText(-1, x, y, 100, 8, , "Selected movies:")
   self~createComboBox(IDC_COMBO_DAYS, x, y + 12, 170, 75, "LIST SORT", "COMBOBOX")

   -- The space on this page is rather tight, so we try to make the group box as
   -- narrow as possible without clipping any of the radio buttons.  I like a
   -- margin of 10 within the group box.
   y += 15 * 2
   cx = self~calcWidthRadioButtons(dayNames)
   cy = self~sizeY - y
   self~createGroupBox(-1, x, y, cx + 20, cy, "", "Day for Movie")

   -- We create the radio buttons in a loop, so we take advantage of that to do
   -- the button event connections and other related tasks.
   buttonTable = .table~new
   first = self~resolveSymbolicID(IDC_RB_DAYS_FIRST)
   last = self~resolveSymbolicID(IDC_RB_DAYS_LAST)

   y += 10; x += 10
   do id = first to last
     day = dayNames[id - first + 1]

     self~createRadioButton(id, x, y, cx, 12, "", day)

     self~connectButtonEvent(id, "CLICKED", onRBClick)
     buttonTable[day~changestr("&", "")] = id
     y += 14
   end

   -- The movie bitmap is 238 X 178, convert that to dialog units and place at
   -- the lower right corner.
   s = .Size~new(238, 178)
   self~pixel2dlgUnit(s)

   x = self~sizeX - s~width
   y = self~sizeY - s~height
   self~createStaticImage(IDC_ST_MOVIE_BMP, x, y, s~width, s~height, "BITMAP SIZEIMAGE CENTERIMAGE")

   self~connectComboBoxEvent(IDC_COMBO_DAYS, "SELENDOK", "onComboUpdate")

-- calcWidthRadioButtons()
--   Given an array of the labels for a group of radio buttons, calculates the
--   smallest width necessary for the buttons so that the widest label is not
--   clipped.
::method calcWidthRadioButtons private
  use strict arg labels

  width = 0

  do l over labels
    size = self~getTextSizeDu(l)
    if size~width > width then width = size~width
  end

  -- There doesn't seem to be any direct way to find out the width of the radio
  -- button bitmap, the little round circle.  But, some of the MSDN doc
  -- *suggests* that it is 10 x 10 in dlus.
  width += 10
  return width

::method initDialog
  expose filmArray movieDays selectedMovies movieCombo

  staticImage = self~newStatic(IDC_ST_MOVIE_BMP)
  size = staticImage~getRealSize
  image = .Image~getImage('bmp\ticketWizardMovie.bmp', .Image~toID(IMAGE_BITMAP), size)
  staticImage~setImage(image)

  filmArray = .array~new(20)
  self~propSheet~querySiblings(filmArray, "FILMS")

  movieDays = .table~new
  do movie over filmArray
    movieDays[movie] = .nil
  end

  selectedMovies = .array~new
  movieCombo = self~newComboBox(IDC_COMBO_DAYS)

::method setActive unguarded
  expose movieCombo selectedMovies filmArray movieDays
  use arg propSheet

  propSheet~setWizButtons("BACK NEXT")

  selected = .table~new
  selectedMovies~empty
  propSheet~querySiblings(selected, "SELECTED")

  movieCombo~deleteAll

  do movie over filmArray
    if selected~hasIndex(movie) then do
      selectedMovies~append(movie)
      movieCombo~add(movie)
    end
    else do
      movieDays[movie] = .nil
    end
  end

  if selected~items > 0 then do
    movieCombo~selectIndex(1)
    movie = movieCombo~selected
    self~setRadioButtons(movie)
  end

  return 0

-- wizNext()
--  We check if the user has set the day for each selected movie.  If not, we
-- warn the user, let the user decide if he wants to continue or not.
::method wizNext unguarded
  expose selectedMovies movieDays movieCombo
  use arg propSheet

  do movie over selectedMovies
    if movieDays[movie] == .nil then do
      msg = "Warning.  You have not selected a day to attend the" || .endOfLine || -
            "movie for all selected movies." || .endOfLine~copies(2) ||             -
            "Are you sure you want to go to the next page?"
      title = "Date Selection is not Complete"
      buttons = "YESNO"
      icon = "WARNING"
      style = "DEFBUTTON2 TASKMODAL"

      ans = MessageDialog(msg, propSheet~dlgHandle, title, buttons, icon, style)

      if ans == self~constDir[IDYES] then return 0

      movieCombo~select(movie)
      self~setRadioButtons(movie)
      return -1
    end
  end

  return 0

::method queryFromSibling unguarded
  expose movieDays
  use arg arg1, arg2, propSheet

  select
    when arg2 == "DATA" then do
      -- arg1 is a table whose indexes are the films.  Each item is a directory
      -- indexes are theater, date, and time.  We only fill in the date index
      -- for any movie in the table.  We do not care what our status is for the
      -- theater, we just fill it in.

      do movie over arg1
        d = arg1[movie]
        d~date = movieDays[movie]
      end
    end

    otherwise do
      -- We do not handle any of the other queries.
      nop
    end
  end
  -- End select
  return 0

::method onComboUpdate
  expose movieCombo

  movie = movieCombo~selected
  self~setRadioButtons(movie)

::method onRBClick
  expose movieDays movieCombo
  use arg id, hwnd

  movie = movieCombo~selected
  movieDays[movie] = self~newRadioButton(id)~getText~changestr("&", "")

::method setRadioButtons private
  expose movieDays buttonTable
  use arg movie

  movieDay = movieDays[movie]

  if movieDay == .nil then id = 0
  else id = buttonTable[movieDay]

  .RadioButton~checkInGroup(self, IDC_RB_DAYS_FIRST, IDC_RB_DAYS_LAST, id)



-- TicketDlg
--  Dialog for the fifth page (interior page, time selection and review.)
::class 'TicketDlg' subclass UserPSPdialog

::method defineDialog
   expose buttonTable

   x = 21; y = 1
   self~createStaticText(-1, x, y, 100, 8, , "Selected movies:")
   self~createComboBox(IDC_COMBO_TIMES, x, y + 12, 170, 75, "LIST SORT", "COMBOBOX")

   label = "Theater:"
   size = self~getTextSizeDu(label)

   y += 15 *2
   x1  = x + size~width + 1
   cx1 = 170 - size~width - 1

   self~createStaticText(-1, x, y + 2, size~width,     size~height, "RIGHT", label)
   self~createBlackFrame(-1,     x1,     y,        cx1, size~height + 6, "")
   self~createStaticText(IDC_ST_TIMES_THEATER, x1 + 4, y + 2,    cx1 - 8,     size~height, "", "")

   y += 20
   self~createStaticText(-1, x, y + 2, size~width,     size~height, "RIGHT", "Date:")
   self~createBlackFrame(-1,     x1,     y,        cx1, size~height + 6, "")
   self~createStaticText(IDC_ST_TIMES_DAY, x1 + 4, y + 2,    cx1 - 8,     size~height, "", "")

   y += size~height + 6 + 4
   cy = self~sizeY - y
   self~createGroupBox(-1, x, y, 60, cy, "", "Time:")

   times = .array~of("11:00", "17:00", "20:00", "22:00")
   buttonTable = .table~new
   first = self~resolveSymbolicID(IDC_RB_TIMES_FIRST)
   last = self~resolveSymbolicID(IDC_RB_TIMES_LAST)

   x += 10; y += 10; cx = 40; cy = 12
   do id = first to last
     time = times[id - first + 1]

     self~createRadioButton(id, x, y, cx, cy, "", time)

     self~connectButtonEvent(id, "CLICKED", onRBClick)
     buttonTable[time] = id
     y +=15
   end

   self~connectComboBoxEvent(IDC_COMBO_TIMES, "SELENDOK", "onComboUpdate")

::method initDialog
  expose filmArray selectedMovies movieTimes movieData movieCombo staticTheater staticDay

  filmArray = .array~new(20)
  self~propSheet~querySiblings(filmArray, "FILMS")

  movieTimes = .table~new
  do movie over filmArray
    movieTimes[movie] = .nil
  end

  -- We also need a table to keep track of the selected theaters and days sent
  -- to us from the Cinemas and Days pages.
  movieData = .table~new

  selectedMovies = .array~new

  movieCombo = self~newComboBox(IDC_COMBO_TIMES)
  staticTheater = self~newStatic(IDC_ST_TIMES_THEATER)
  staticDay = self~newStatic(IDC_ST_TIMES_DAY)

::method setActive unguarded
  expose filmArray selectedMovies movieCombo movieTimes movieData
  use arg propSheet

  propSheet~setWizButtons("BACK NEXT")

  propSheet~querySiblings(movieData, "DATA")

  selected = .table~new
  selectedMovies~empty
  movieCombo~deleteAll

  propSheet~querySiblings(selected, "SELECTED")

  do movie over filmArray
    if selected~hasIndex(movie) then do
      movieCombo~add(movie)
      selectedMovies~append(movie)
    end
    else do
      movieTimes[movie] = .nil
    end
  end

  if selected~items > 0 then do
    movieCombo~selectIndex(1)
    movie = movieCombo~selected
    self~setMovieData(movie)
  end

  return 0

-- wizNext()
--  This is the last page before completion.  Here we do not let the user
--  proceed unless every selected movie has each item (theater, day, and time)
--  specified.
::method wizNext unguarded
  expose selectedMovies movieTimes movieData movieCombo
  use arg propSheet

  -- We make two loops over the selected movies.  The first is to force the
  -- user to select a time for very selected movie.  Once that passes, we then
  -- loop over the movie data and check that the day and theater is specified
  -- for each selected movie.  If we find a piece of information missing in the
  -- second loop, we return the user to the proper page to fill in the info.

  do movie over selectedMovies
    if movieTimes[movie] == .nil then do
      msg = "You have not selected a time to attend the" || .endOfLine || -
            "movie:" movie || .endOfLine~copies(2) ||             -
            "You must specify a time to attend each movie."
      title = "Time Selection is not Complete"
      buttons = "OK"
      icon = "EXCLAMATION"

      ans = MessageDialog(msg, propSheet~dlgHandle, title, buttons, icon)

      movieCombo~select(movie)
      self~setMovieData(movie)
      return -1
    end
  end

  do movie over selectedMovies
    d = movieData[movie]
    if d~theater == .nil then do
      msg = "You have not selected a theater for the" || .endOfLine ||   -
            "movie:" movie || .endOfLine~copies(2) ||                    -
            "You must specify a theater, day, and time" || .endOfLine || -
            "for each movie ticket."
      title = "Ticket Information is not Complete"
      buttons = "OK"
      icon = "EXCLAMATION"

      ans = MessageDialog(msg, propSheet~dlgHandle, title, buttons, icon)

      -- Return the page index of the Cinemas page to switch to that page
      return 3
    end

    if d~date == .nil then do
      msg = "You have not selected a day to attend the" || .endOfLine ||   -
            "movie:" movie || .endOfLine~copies(2) ||                    -
            "You must specify a theater, day, and time" || .endOfLine || -
            "for each movie ticket."
      title = "Ticket Information is not Complete"
      buttons = "OK"
      icon = "EXCLAMATION"

      ans = MessageDialog(msg, propSheet~dlgHandle, title, buttons, icon)

      -- Return the page index of the Days page to switch to that page
      return 4
    end
  end

  return 0

::method queryFromSibling unguarded
  expose movieTimes
  use arg arg1, arg2, propSheet

  select
    when arg2 == "DATA" then do
      do movie over arg1
        d = arg1[movie]
        d~time = movieTimes[movie]
      end
    end

    otherwise do
      nop
    end
  end
  -- End select

  return 0

::method onComboUpdate
  expose movieCombo

  movie = movieCombo~selected
  self~setMovieData(movie)

::method onRBClick
  expose movieTimes movieCombo
  use arg id, hwnd

  movie = movieCombo~selected
  movieTimes[movie] = self~newRadioButton(id)~getText~changestr("&", "")

::method setMovieData private
  expose movieTimes buttonTable movieData staticTheater staticDay
  use arg movie

  movieTime = movieTimes[movie]

  if movieTime == .nil then id = 0
  else id = buttonTable[movieTime]

  .RadioButton~checkInGroup(self, IDC_RB_TIMES_FIRST, IDC_RB_TIMES_LAST, id)

  d = movieData[movie]

  if d~theater == .nil then text = "==> No theater was selected"
  else text = d~theater
  staticTheater~setText(text)

  if d~date == .nil then text = "==> No day selected"
  else text = d~date
  staticDay~setText(text)



-- CompletionDlg
--  Dialog for the sixth page (exterior page, review and finish.)
::class 'CompletionDlg' subclass UserPSPDialog

::method defineDialog

    -- Watermark bitmap is 164 x 316.  The wizard does not stretch the bitmap,
    -- its size remains constant.  So, the width of the bitmap, in dialog units,
    -- changes as the font changes.  pixel2dlgUnit() will convert the pixel size
    -- of the bitmap to the correct dialog units for the font in use by this
    -- dialog.
    s = .Size~new(164, 314)
    self~pixel2dlgUnit(s)

    x = s~width + 6           -- 6 dlus right of the edge of the watermark bitmap.
    y = 8                     -- From the Wizard97 specification.
    cx = self~sizeX - x       -- To the right side of the template, the wizard automatically adds 14 dlus.
    cy = 3 * 12               -- Using 12 point font, takes 3 lines for the msg.

    self~createStaticText(IDC_ST_COMPLETE_MSG, x, y, cx, cy, "", "")

    -- Things are a little crowded on this page, so only add 4 dlus of space
    -- between the top message and the next control.  (Contrast this to the
    -- IntroDlg.)
    y += cy + 4
    self~createStaticText(-1, x, y, 165, 8, , "Ticket information for your selected movie(s):")
    self~createComboBox(IDC_COMBO_COMPLETE, x, y + 12, 165, 63, "LIST SORT PARTIAL", "COMBOBOX")

    label = "Theater:"
    size = self~getTextSizeDu(label)

    y  += 15 * 2
    x1  = x + size~width + 1
    cx1 = 165 - size~width - 1

    self~createStaticText(-1,      x, y + 2, size~width,     size~height, "", label)
    self~createBlackFrame(-1,     x1,     y,        cx1, size~height + 6)
    self~createStaticText(IDC_ST_COMPLETE_THEATER, x1 + 4, y + 2,    cx1 - 8,     size~height)

    y += 20
    self~createStaticText(-1,      x, y + 2, size~width,     size~height, "", "Date:")
    self~createBlackFrame(-1,     x1,     y,        cx1, size~height + 6)
    self~createStaticText(IDC_ST_COMPLETE_DAY, x1 + 4, y + 2,    cx1 - 8,     size~height)

    y += 20
    self~createStaticText(-1,      x, y + 2, size~width,     size~height, "", "Time:")
    self~createBlackFrame(-1,     x1,     y,        cx1, size~height + 6)
    self~createStaticText(IDC_ST_COMPLETE_TIME, x1 + 4, y + 2,    cx1 - 8,     size~height)

    -- ticket.bmp is 204 x 80 pixels, convert to dialog units and place it in
    -- the lower left corner.  At the x margin.
    s = .Size~new(204, 80)
    self~pixel2dlgUnit(s)

    y = self~sizeY - s~height - 4

    if .DlgUtil~comCtl32Version < 6 then do
      self~createBitmapButton(IDC_PB_TICKET_BMP, x, y, s~width, s~height, "FRAME USEPAL STRETCH GROUP", "Get the Ticket", -
                              'onGetTicket', "bmp\ticketWizardTicket.bmp")
   end
   else do
      self~createPushButton(IDC_PB_TICKET_BMP, x, y, s~width, s~height, "GROUP", "", 'onGetTicket')
   end

   self~connectComboBoxEvent(IDC_COMBO_COMPLETE, "SELENDOK", "onComboUpdate")

::method initDialog
  expose movieCombo staticTheater staticDay staticTime

  msg = "Thank You For Using the ooRexx Movie Ticket Selectitron Wizard"
    font1 = self~createFontEx("Verdana", 12, "BOLD")

    st1 = self~newStatic(IDC_ST_COMPLETE_MSG)
    st1~setFont(font1)
    st1~setText(msg)

  movieCombo = self~newComboBox(IDC_COMBO_COMPLETE)
  staticTheater = self~newStatic(IDC_ST_COMPLETE_THEATER)
  staticDay = self~newStatic(IDC_ST_COMPLETE_DAY)
  staticTime = self~newStatic(IDC_ST_COMPLETE_TIME)

  if .DlgUtil~comCtl32Version  < 6 then return

   bmpButton = self~newPushButton(IDC_PB_TICKET_BMP)
   size = bmpButton~getRealSize
   size~width -= 10;
   size~height -= 10;

   image = .Image~getImage('bmp\ticketWizardTicket.bmp', .Image~toID(IMAGE_BITMAP), size)
   imageList = .ImageList~create(size, .Image~toID(ILC_COLOR8), 1, 0)
   imageList~add(image)

   align = .Image~toID(BUTTON_IMAGELIST_ALIGN_CENTER)
   margin = .Rect~new(5)
   bmpButton~setImageList(imageList, margin, align)

::method setActive unguarded
  expose movieCombo staticTheater staticDay staticTime info
  use arg propSheet

  propSheet~setWizButtons("BACK FINISH")

  info = .table~new
  movieCombo~deleteAll

  ret = propSheet~querySiblings(info, "DATA")

  do movie over info
    movieCombo~add(movie)
  end

  movieCombo~selectIndex(1)
  movie = movieCombo~selected
  self~setMovieData(movie)

  return 0

-- onGetTicket()
--   This method is invoked when the user pushes the fancy bitmap button.  We
--   simply, programmatically, press the Finish button because the action to
--   take is the same.
::method onGetTicket unguarded
  self~propSheet~pressButton("FINISH")

-- wizFinish()
--   Invoked when the user presses the Finish button.  We use the ticket
--   information to display a, (slightly,) fancy message to the user telling her
--   where and when she can get her ticket(s).
::method wizFinish unguarded
  expose info
  use arg propSheet

  indexes = info~allIndexes
  count = indexes~items

  theaterFirst = info[indexes[1]]~theater
  smallTab = " "~copies(4)
  doubleSpace = .endOfLine~copies(2)

  if count == 1 then ticketWord = "ticket"
  else ticketWord = count "tickets"

  msg = "You can pick up your" ticketWord "now at the"   || doubleSpace || -
        smallTab || theaterFirst                         || doubleSpace || -
        "box office.  Or at"

  if count > 1 then do
    theaterList = ""
    do i = 2 to count
      index = indexes[i]
      theaterList ||= smallTab || info[index]~theater || .endOfLine
    end

    msg ||= " one of the following"   || .endOfLine  || -
            "theaters:"               || doubleSpace || -
            theaterList               || .endOfLine  || -
           "within an hour from now." || doubleSpace  || doubleSpace

    cost = "$" || (count * 15) "dollars."
  end
  else do
    msg ||= " any other convenient time." || doubleSpace || doubleSpace
    cost = "$15 dollars."
  end

  msg ||= "Your bank account is debited" cost

  title = "ooRexx Movie Ticket Selectitron"
  buttons = "OK"
  icon = "INFORMATION"

  ans = MessageDialog(msg, propSheet~dlgHandle, title, buttons, icon)

  return 0

::method onComboUpdate
  expose movieCombo

  movie = movieCombo~selected
  self~setMovieData(movie)

::method setMovieData private
  expose staticTheater staticDay staticTime info
  use arg movie

  d = info[movie]
  staticTheater~setText(d~theater)
  staticDay~setText(d~date)
  staticTime~setText(d~time)



-- FontPicker
--  Dialog that allows the user to pick different fonts for the example program.
--  This lets the user see how defineDialog() works with different fonts.
::class 'FontPicker' subclass RcDialog

::method initAutoDetection
   self~noAutoDetection

::method initDialog
  expose nameCB sizeCB

  nameCB = self~newComboBox(IDC_COMBO_NAME)
  sizeCB = self~newComboBox(IDC_COMBO_SIZE)

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


2
::routine setFont

    oldFont = .directory~new
    oldFont~name = .PlainBaseDialog~getFontName
    oldFont~size = .PlainBaseDialog~getFontSize

    dlg = .FontPicker~new("rc\ticketWizard.rc", IDD_FONT_PICKER, , "rc\ticketWizard.h", , 6)
    if dlg~initCode == 0 then do
        dlg~execute("SHOWTOP", IDI_DLG_OOREXX)
    end

    return oldFont

::routine restoreFont
    use strict arg oldFont
    .PlainBaseDialog~setDefaultFont(oldFont~name, oldFont~size)
    return oldFont



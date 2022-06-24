/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/**********************************************************************/
/*                                                                    */
/* MSInternetExplorer_search.rex: OLE Automation with ooRexx          */
/*                                                                    */
/* Using events with the Internet Explorer:                           */
/* Search for the string "Cloud" on the IBM web page and go randomly   */
/* to one of the found sites.                                         */
/*                                                                    */
/* Note that this sample no longer seems to work using IE9 on         */
/* Windows.  Invoking the click() method on the Search button seems   */
/* have no effect.  Geting the form instead fo the button and         */
/* invoking submit() also doesn't work.                               */
/**********************************************************************/


/* instantiate an instance of the Internet Explorer */
myIE = .controlledIE~new("InternetExplorer.Application","WITHEVENTS")

myIE~visible = .true
myIE~navigate("http://www.ibm.com/")

myIE~wait /* wait for page to be loaded */

doc = myIE~document

/* set query field on the IBM page to Cloud */
textInput = doc~GetElementById("q")
if textInput == .nil then do
  say "Failed to get text input object."
  say "Website may have changed, aborting."
  myIE~quit
  return 99
end

textInput~value = "Cloud"

/* click on the search image to submit the query */
submitInput = doc~getElementById("ibm-search")
if submitIput == .nil then do
  say "Failed to get submit input object."
  say "Website may have changed, aborting."

  myIE~quit
  return 99
end

submitInput~click

myIE~wait /* wait for page to be loaded */

/* get the new page */
doc = myIE~document
all = doc~GetElementsByTagName("A")   /* get all <A HREF="..."> elements */

/* get all links to Cloud pages into a table */
cloudlinks = .table~new

j = 1
do i over all
  if i~innertext~pos("Cloud") > 0 |,
     i~innertext~pos("Downloads") > 0 |,
     i~innertext~pos("Learning") > 0 |,
     i~innertext~pos("Produkte") > 0 then do
    cloudlinks~put(i~href,j)
    j = j + 1
  end
end
if cloudlinks~items < 1 then do
  msg = "No links were found.  Submitting the search"   || .endOfLine || -
        "probably failed.  This happens with IE9"       || .endOfLine || -
        "on Windows 7."
  ret = RxMessageBox(msg, "Done", "OK", "INFORMATION")
  signal on syntax   -- in case the user closes the MSIE window before the RxMessageBox
  myIE~quit
syntax:
  return 99
end

/* select one of the Cloud links randomly... */
choice = random(1,cloudlinks~items)
hint="randomly picked Cloud page link #" choice"/"cloudlinks~items
say "Navigating to" hint

/* ...and go there */
myIE~navigate(cloudlinks~at(choice))
myIE~wait

/* wait for user to acknowledge, then shut down example */
call RxMessageBox "We're now at a" hint, "Done", "OK", "INFORMATION"

signal on syntax  -- in case the user closes the MSIE window before the RxMessageBox
myIE~quit
syntax:

exit



/* this class is derived from OLEObject and contains two methods */
/* that will be called when certain events take place.           */
::CLASS controlledIE SUBCLASS OLEObject

::METHOD init
  self~block = .true
  forward class(super)

::METHOD block ATTRIBUTE

/* this is a helper method */
::METHOD wait
  do until self~block == .false
    call SysSleep 1
  end
  return

/* this event takes several parameters, but all can be ignored here */
::METHOD BeforeNavigate2
  self~block = .true

/* when this event takes place, the block attribute is set to false */
::METHOD DocumentComplete
  self~block = .false

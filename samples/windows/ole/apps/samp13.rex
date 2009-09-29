/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* SAMP13.REX: OLE Automation with Object REXX - Sample 13            */
/*                                                                    */
/* Using events with the Internet Explorer:                           */
/* Search for the string "REXX" on the IBM web page and go randomly   */
/* to one of the found sites.                                         */
/*                                                                    */
/**********************************************************************/


/* instantiate an instance of the Internet Explorer */
myIE = .controlledIE~new("InternetExplorer.Application","WITHEVENTS")

myIE~visible = .true
myIE~navigate("http://www.ibm.com/")

myIE~wait /* wait for page to be loaded */

doc = myIE~document

/* set query field on the IBM page to REXX */
doc~GetElementById("q")~value = "REXX"

/* click on the go image to submit the query */
doc~GetElementById("Search")~click
myIE~wait /* wait for page to be loaded */

/* get the new page */
doc = myIE~document
all = doc~GetElementsByTagName("A")   /* get all <A HREF="..."> elements */

/* get all links to REXX pages into a table */
rexxlinks = .table~new

j = 1
do i over all
  if i~innertext~pos("REXX") > 0 then do
    rexxlinks~put(i~href,j)
    j = j + 1
  end
end

/* select one of the REXX links randomly... */
choice = random(1,rexxlinks~items)

/* ...and go there */
myIE~navigate(rexxlinks~at(choice))
myIE~wait

/* wait for user to acknowledge, then shut down example */
call RxMessageBox "We're now at a randomly picked REXX page", "Done", "OK", "INFORMATION"

myIE~quit

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

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
/* SAMP08.REX: OLE Automation with Open Object REXX - Sample 8        */
/*                                                                    */
/* Create a Word document, enter some text, save it. Load it again    */
/* and modify it.                                                     */
/*                                                                    */
/**********************************************************************/

/* get a Word application object */
Word = .OLEObject~New("Word.Application")

/* set Word visible to see what happens */
Word~Visible = .TRUE

/* new document */
Document = Word~Documents~Add

/* get the selection object */
Selection = Word~Selection

/* type two lines of text and new paragraph */
Selection~TypeText("This is a text sent to MS Word with the Open Object Rexx OLE interface")
Selection~TypeParagraph
Selection~TypeText("This is a text in a new line")

/* select all */
Selection~WholeStory

/* get the font object */
Font = Selection~Font

/* change the font size and behavior */
Font~Name="Arial"
Font~Size="12"
Font~Bold = .TRUE
Font~Italic = .TRUE

/* get the environment variable TEMP and create filename to save */
TempDir =  VALUE( "TEMP",,ENVIRONMENT)
FileName = TempDir || "\" || "OLERexx"

/* "save as..." and close Word */
Document~SaveAs(FileName)
Word~Quit

say "Created" FileName". Press enter to continue"
pull

/* get a Word application object */
Word = .OLEObject~New("Word.Application")

/* set word visible to see what happens */
Word~Visible = .TRUE

/* get the documents object and open a file */
Documents = Word~Documents
Documents~Open(FileName)

/* get the selection object and go to end of document */
Selection = Word~Selection
Selection~EndKey("6")

/* add a paragraph */
Selection~TypeParagraph

/* get active document object */
ActiveDocument = Word~ActiveDocument

/* invoke print preview */
ActiveDocument~PrintPreview

/* get active window object */
ActiveWindow = Word~ActiveWindow

/* change zoom percentage */
ActiveWindow~ActivePane~View~Zoom~Percentage = "200"

/* wait a little bit */
say "Showing Print Preview with 200%. Press enter to continue."
pull

/* change zoom percentage */
ActiveWindow~ActivePane~View~Zoom~Percentage = "100"

/* wait a little bit */
call syssleep 2

/* close print preview */
ActiveDocument~ClosePrintPreview

/* add a paragraph */
Selection~TypeParagraph

/* add a table */
ActiveDocument~Tables~Add(Selection~Range,4,5)

/* autoformat table to color */
Selection~Tables(1)~AutoFormat(9)

/* fill table */

header = .array~of("Year", "January", "February", "March", "April")
do i over header
  Selection~TypeText(i)
  Selection~MoveRight
end

do i=2000 to 2002
  Selection~MoveRight(12)
  Selection~TypeText(i)
  Selection~MoveRight
  do 4
    Selection~TypeText(random(31))
    Selection~MoveRight
  end
end

say "Done. Press enter to quit."
pull

Word~Quit(.false)

::REQUIRES "OREXXOLE.CLS"

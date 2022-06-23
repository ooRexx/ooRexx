/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2022 Rexx Language Association. All rights reserved.         */
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
/*********************************************************************

 MSWord_useStyles.rex: using OLE (object linking and embedding) with ooRexx

 Links:  <https://docs.microsoft.com/en-us/office/vba/api/overview/word>
         <https://docs.microsoft.com/en-us/office/vba/word/concepts/miscellaneous/concepts-word-vba-reference>
         <https://docs.microsoft.com/en-us/office/vba/api/overview/word/object-model>

 Using OLE create a Microsoft Word document, add text using various
 styles and wait for the user to press the enter (return) key before
 changing the font attributes of the styles named "Title" and "Heading 1".

*********************************************************************/

-- Start Word with empty document
Word = .OLEObject~New("Word.Application")
Word~Visible = .TRUE                      -- make Word visible
Document = Word~Documents~Add             -- add document
Selection = Word~Selection                -- selection object
say "# 1: Create: title style..."
Selection~Style = "Title"                 -- Create selection with style: Title
Selection~TypeText("TITLE")               -- give selection a text
Selection~TypeParagraph                   -- add paragraph (go to next line)
say "# 2: Create: heading 1 style..."
Selection~Style = "Heading 1"             -- Create selection with style: Heading 1
Selection~TypeText("HEADING 1")           -- give selection a text
Selection~TypeParagraph                   -- add paragraph (go to next line)
say "# 3: Create: heading 2 style..."
Selection~Style = "Heading 2"             -- Create selection with style: Heading 2
Selection~TypeText("HEADING 2")           -- give selection a text
Selection~TypeParagraph                   -- add paragraph (go to next line)
   -- Note: Usually the style "Normal" follows heading styles
Selection~TypeText("Reset to normal ...") -- "Normal" follows "Heading 2"
Selection~TypeParagraph                   -- add paragraph (go to next line)
say "# 4: Create: normal text style..."
Selection~Style = "Normal"                -- Create selection with style: normal
Selection~TypeText("I am Normal Text.")   -- give selection a text

say "Press any key to change style!"
parse pull                              -- wait for key press

-- Go through each sentence
SentenceCount = Document~Sentences~Count        -- get sentence count
Font = Selection~Font                           -- get font object
do SentenceNumber = 1 to SentenceCount          -- go through each sentence

   Document~Sentences(SentenceNumber)~Select   -- select sentence
   StyleName = Selection~Style~NameLocal       -- get style name of sentence
   Select case StyleName                       -- make changes depending on style name
      when "Title" then do
          Font~Name="Arial"
          Font~Size="38"
          Font~Bold = .TRUE
      end
      when "Heading 1" then do
          Font~Name="Times New Roman"
          Font~Size="22"
          Font~Italic = .TRUE
      end
      otherwise NOP
   end
end

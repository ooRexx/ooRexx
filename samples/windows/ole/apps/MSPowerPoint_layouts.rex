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

 MSPowerPoint.rex: using OLE (object linking and embedding) with ooRexx

 Using OLE open MSPowerPoint and add a Title slide with Title and SubTitle
 shapes (sections).
 Append slides with various styles, get slide count and amount of shapes per
 slide, list names of shapes on each slide.
 Save  pptx file with a unique name and close PowerPoint.

*********************************************************************/

-- Start PowerPoint
PP = .OLEObject~New("PowerPoint.Application")
PP~Visible = .TRUE                                                    -- make PP visible
-- Add first Slide
PP~Presentations~Add~Slides~Add(1, 1)                                 -- add: Title-Slide
FirstSlide = PP~ActivePresentation~Slides(1)
FirstSlide~Shapes(1)~TextFrame~TextRange = "New Presentation Title"   -- first Shape is Title
FirstSlide~Shapes(2)~TextFrame~TextRange =  "Subtitle at:" .dateTime~new -- second Shape is SubTitle
-- Append several Slides
ActivePresentation = PP~ActivePresentation
ActivePresentation~Slides~Add(2, 1)                                   -- append: second Title-Slide
ActivePresentation~Slides~Add(3, 2)                                   -- append: Title and Text Slide
ActivePresentation~Slides~Add(4, 3)                                   -- append: Title and 2-Columne Text
ActivePresentation~Slides~Add(5, 5)                                   -- append: Title, Text and Chart
-- Check what on Slide
SlideCount = ActivePresentation~Slides~Range~Count                    -- get amount of Slides
say "Your Presentation has" SlideCount "Slides"
do SlideNumber = 1 to SlideCount                                      -- go through each Slide
    ShapeCount = ActivePresentation~Slides(SlideNumber)~Shapes~Range~Count  -- get amount of Shapes per Slide
    say " Slide" SlideNumber "has" ShapeCount "Shapes"
    do ShapeNumber = 1 to ShapeCount                                        -- go through each Shape
        -- List names of Shapes on Slide
        say "  ("ShapeNumber"/"ShapeCount"):" ActivePresentation~Slides(SlideNumber)~Shapes(ShapeNumber)~name
    end
end

say "Press any key to save and close!"
parse pull                              -- wait for key press

-- Save and Close
homeDir = VALUE('USERPROFILE',,'ENVIRONMENT')   -- get user's home directory
fileOut = homedir"\Desktop\PP_" || date("S") || "_" || changeStr(":",time(),"") || ".pptx"
say "File to save to:" fileOut
ActivePresentation~SaveAs(fileOut)                                    -- save pptx file
PP~Quit                                                               -- close PP

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

/**********************************************************************
 AOO_swriter_table.rex using OLE (object linking and embedding) with ooRexx

 Links:  <https://OpenOffice.org>
         <https://wiki.openoffice.org/wiki/Documentation/DevGuide/ProUNO/Bridge/Automation_Bridge>
         <https://www.pitonyak.org/oo.php>
         <https://www.openoffice.org/udk/common/man/spec/ole_bridge.html>

 This is the ooRexx version (which includes corrections) of the VBScript
 "A Quick Tour" example from the AOO (Apache OpenOffice) DevGuide, chapter
 "Automation_Bridge" documentation.

 Using OLE create a new swriter document, a TextTable, a TextFrame, paragraphs
 and apply various formatings.
***********************************************************************/

  -- The service manager is always the starting point
  -- If there is no office running then an office is started up
  objServiceManager= .OleObject~new("com.sun.star.ServiceManager")

  -- Create the Desktop
  objDesktop= objServiceManager~createInstance("com.sun.star.frame.Desktop")

  -- Open a new empty writer document
  args=.array~new
  objDocument= objDesktop~loadComponentFromURL("private:factory/swriter", "_blank", 0, args)

  -- Create a text object
  objText= objDocument~getText

  -- Create a cursor object
  objCursor= objText~createTextCursor

  -- Inserting some Text
  vbLf = "0a"x    -- line-feed character
  objText~insertString( objCursor, "The first line in the newly created text document."vbLf, .false)

  -- Inserting a second line
  objText~insertString( objCursor, "Now we-- re in the second line", .false)

  -- Create instance of a text table with 4 columns and 4 rows
  objTable= objDocument~createInstance( "com.sun.star.text.TextTable")
  objTable~initialize( 4, 4 )

  -- Insert the table
  objText~insertTextContent( objCursor, objTable, .false)

  -- Get first row
  objRows= objTable~getRows
  objRow= objRows~getByIndex( 0)

  -- Set the table background color
  objTable~setPropertyValue( "BackTransparent", .false)
  objTable~setPropertyValue( "BackColor", 13421823)

  -- Set a different background color for the first row
  objRow~setPropertyValue( "BackTransparent", .false)
  objRow~setPropertyValue( "BackColor", 6710932)

  -- Fill the first table row
  call insertIntoCell "A1","FirstColumn", objTable -- insertIntoCell is a helper function, see below
  call insertIntoCell "B1","SecondColumn", objTable
  call insertIntoCell "C1","ThirdColumn", objTable
  call insertIntoCell "D1","SUM", objTable

  objTable~getCellByName("A2")~setValue( 22.5     )
  objTable~getCellByName("B2")~setValue( 5615.3   )
  objTable~getCellByName("C2")~setValue( -2315.7  )
  objTable~getCellByName("D2")~setFormula( "=sum <A2>+<B2>+<C2>"  )

  objTable~getCellByName("A3")~setValue( 21.5     )
  objTable~getCellByName("B3")~setValue( 615.3    )
  objTable~getCellByName("C3")~setValue( -315.7   )
  objTable~getCellByName("D3")~setFormula( "sum <A3>+<B3>+<C3>" )

  objTable~getCellByName("A4")~setValue( 121.5    )
  objTable~getCellByName("B4")~setValue( -615.3   )
  objTable~getCellByName("C4")~setValue( 415.7    )
  objTable~getCellByName("D4")~setFormula( "sum <A4>+<B4>+<C4>" )

  range=objTable~getCellRangeByName("A2:D4")
  range~setPropertyValue("NumberFormat", 4)  -- set number format
  -- use ParaAdjust: com.sun.star.style.ParagraphAdjust.RIGHT
  range~setPropertyValue("ParaAdjust", 1)    -- align right

  -- Change the CharColor and add a Shadow
  objCursor~setPropertyValue( "CharColor", 255)
  objCursor~setPropertyValue( "CharShadowed", .true)

  -- Create a paragraph break
  -- The second argument is a com::sun::star::text::ControlCharacter::PARAGRAPH_BREAK constant
  objText~insertControlCharacter( objCursor, 0 , .false)

  -- Inserting colored Text.
  objText~insertString( objCursor, " This is a colored Text - blue with shadow"vbLf, .false)

  -- Create a paragraph break ( ControlCharacter::PARAGRAPH_BREAK).
  objText~insertControlCharacter( objCursor, 0, .false)

  -- Create a TextFrame~
  objTextFrame= objDocument~createInstance("com.sun.star.text.TextFrame")

  -- Create a Size struct~
  objSize = objServiceManager~Bridge_GetStruct("com.sun.star.awt.Size")
  objSize~Width= 15000
  objSize~Height= 400
  objTextFrame~setSize( objSize)

  --  TextContentAnchorType.AS_CHARACTER = 1
  objTextFrame~setPropertyValue( "AnchorType", 1)

  -- insert the frame
  objText~insertTextContent( objCursor, objTextFrame, .false)

  -- Get the text object of the frame
  objFrameText= objTextFrame~getText

  -- Create a cursor object
  objFrameTextCursor= objFrameText~createTextCursor

  -- Inserting some Text
  objFrameText~insertString( objFrameTextCursor, "The first line in the newly created text frame.", -
                             .false)
  objFrameText~insertString( objFrameTextCursor, -
           vbLf"With this second line the height of the frame raises.", .false)

  -- Create a paragraph break
  -- The second argument is a com::sun::star::text::ControlCharacter::PARAGRAPH_BREAK constant
  objFrameText~insertControlCharacter( objCursor, 0 , .false)

  -- Change the CharColor and remove the Shadow
  objCursor~setPropertyValue( "CharColor", 65536)
  objCursor~setPropertyValue( "CharShadowed", .false)

  -- Insert another string
  objText~insertString( objCursor, " That-- s all for now !!", .false)

::routine insertIntoCell
  use arg strCellName, strText, objTable

  objCellText= objTable~getCellByName( strCellName)
  objCellCursor= objCellText~createTextCursor
  objCellCursor~setPropertyValue( "CharColor",16777215)
  objCellText~insertString( objCellCursor, strText, .false)

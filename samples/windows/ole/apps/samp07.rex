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
/* SAMP07.REX: OLE Automation with Open Object REXX - Sample 7        */
/*                                                                    */
/* Create a new spreadsheet in 1-2-3 and fill in a table with fictive */
/* revenue numbers. The table will also contain a calculated field    */
/* and different styles. A second sheet is added with a 3D chart      */
/* displaying the revenue data.                                       */
/*                                                                    */
/**********************************************************************/

/* create a new 123 Workbook */
Workbook = .OLEObject~New("Lotus123.Workbook")
Range = Workbook~Ranges("A1")
Range~Contents = "Open Object Rexx OLE Sample spreadsheet"

/* fill the first column with month names */
Col = "A"
Row = 3
Range = Workbook~Ranges(Col || Row)
Range~Contents = "Month"
Range~Font~FontName = "Times New Roman"
Range~Font~Bold = "True"

Do Month = 1 To 12
  Row = Row + 1
  Range = Workbook~Ranges(Col || Row)
  Range~Contents = '"' || Right(Month, 2, "0") || "/98"
  Range~TextHorizontalAlign = "$AlignLeft"
End

Row = Row + 1
Range = Workbook~Ranges(Col || Row)
Range~Contents = "All 1998"
Range~Font~Bold = "True"

/* fill the second column with random revenue numbers */
Col = "B"
Row = 3
Range = Workbook~Ranges(Col || Row)
Range~Contents = "Revenue"
Range~Font~FontName = "Times New Roman"
Range~Font~Bold = "True"

Do Month = 1 To 12
  Row = Row + 1
  Range = Workbook~Ranges(Col || Row)
  Range~Contents = Random(20000, 100000)
  Range~FormatName = "US Dollar"
  Range~FormatDecimals = 0
End

Row = Row + 1
Range = Workbook~Ranges(Col || Row)
Range~Contents = "@SUM(B4..B15)"
Range~Font~Bold = "True"
Range~FormatName = "US Dollar"
Range~FormatDecimals = 0

/* put a grid around the table */
Range = Workbook~Ranges("A3..B16")
Range~GridBorder~Style = "$SolidBorder"

/* use thick bottom lines for title and sum lines */
Range = Workbook~Ranges("A3..B3")
Range~BottomBorder~Style = "$DoubleBorder"
Range = Workbook~Ranges("A16..B16")
Range~TopBorder~Style = "$DoubleBorder"


/* create a 3d chart showing revenue over month */
Sheet = Workbook~NewSheet("$Last", 1, "False")
Range = Workbook~Ranges("A:A4..A:B15")
Chart = Sheet~NewChart(0, 0, 12800, 9600, Range)
Chart~Is3D = .True
Chart~Title~Lines(1)~Text = "Revenue development 1998"
Chart~Legend~Visible = .False

Chart~XAxis~Title~Text = "Month"
Chart~XAxis~Title~Font~Size = "36"
Chart~XAxis~TickLabels~Font~Size = "36"

Chart~YAxis~Title~Text = "Revenue"
Chart~YAxis~Title~Font~Size = "36"
Chart~YAxis~TickLabels~Font~Size = "36"
Chart~YAxis~SubTitle~Visible = .False

/* save spreadsheet as file OLETest */
Workbook~SaveAs("OLETest")

say "Created" workbook~name

Workbook~Close



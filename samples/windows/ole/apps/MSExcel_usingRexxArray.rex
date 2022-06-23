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
 MSExcel_usingRexxArray.rex: using OLE (object linking and embedding) with ooRexx

 Links:  <https://docs.microsoft.com/en-us/office/vba/api/overview/excel>
         <https://docs.microsoft.com/en-us/office/vba/excel/concepts/miscellaneous/concepts-excel-vba-reference>
         <https://docs.microsoft.com/en-us/office/vba/api/overview/excel/object-model>

 Using OLE create a new Microsoft Excel, create and place additional
 sheets, rename sheets, iterate over sheets, fill in first sheet with generated data
 from an ooRexx array, create a chart from the data.

***********************************************************************/

excel = .OleObject~new("Excel.Application")
excel~visible = .true               -- show Excel
book  = excel~workbooks~add         -- create a workbook
sheet  = book~worksheets(1)         -- get first sheet (exists always) from workbook
sheet1 = book~worksheets~add(sheet) -- add and get a sheet before previous "sheet"
book~worksheets~add(,sheet)         -- add and get sheet after "sheet"

do counter c ws over book~worksheets-- rename the worksheets sequentially
   ws~name="Sheet #" c
end
sheet1~name = sheet1~name "(data)"  -- add hint to name of first work sheet

say "do i=1 to book~worksheets~count:" -- use: do i=1 ...
do i=1 to book~worksheets~count     -- list all worksheets by name
   say "... i="i":" book~worksheets(i)~name
end
say

say "do counter c ws over book~worksheets:"  -- use: do...over
do counter c ws over book~worksheets   -- list all worksheets by name
   say "... c="c":" ws~name
end

-- set titles from an ooRexx array
sheet1~range("A1:C1")~value = ("Delivered", "En route", "To be shipped")
-- define data range and set its data from an ooRexx array
sheet1~range("A2:C7")~value = createData(6)  -- create and assign ooRexx array

/* cf. <https://docs.microsoft.com/en-us/office/vba/api/excel.chart(object)> */
chart = excel~charts~add            -- get the chart collection, add and get a new chart
chart~chartType = excel~getConstant("xlAreaStacked")
plotBy = excel~getConstant("xlColumns")
chart~setSourceData(sheet1~range("A1:C7"), plotBy) -- includes titles
chart~hasTitle = .true
chart~chartTitle~text = "A Chart by ooRexx (".DateTime~new")"

::routine createData    -- create arbitrary data using Rexx' random(min,max) function
  use arg items=5
  arr=.array~new        -- create Rexx array
  do i=1 to items       -- create random numbers
     arr[i,1] = random( 50,2000)
     arr[i,2] = random(100,2500)
     arr[i,3] = random(150,1500)
  end
  return arr            -- return Rexx array

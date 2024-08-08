/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2022-2024 Rexx Language Association. All rights reserved.    */
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
 MSExcel_cURL.rex: using OLE (object linking and embedding) with ooRexx

 Get temperature from <https://wttr.in> using cURL, documentation can be found
 at <https://github.com/chubin/wttr.in#one-line-output>.

 Note: cURL is included by default since Windows 10 build 17063.

 Links:  <https://docs.microsoft.com/en-us/office/vba/api/overview/excel>
         <https://docs.microsoft.com/en-us/office/vba/excel/concepts/miscellaneous/concepts-excel-vba-reference>
         <https://docs.microsoft.com/en-us/office/vba/api/overview/excel/object-model>

 Using OLE create a new Microsoft Excel worksheet, query the weather with
 the cURL command and insert the received data. Using the worksheet temperature
 data create a simple chart.

***********************************************************************/

-- Get information using curl, save results in array
cityArr = .array~of("Vienna", "Graz", "Linz", "Salzburg", "Innsbruck", "Klagenfurt", "Bregenz", "Eisenstadt", "Sankt-Poelten", "Wien")
cityWeather = .array~new                                    -- array for weather information
do counter i name over cityArr
    command='curl https://wttr.in/'name'?format="%l:+%t"'   -- https://github.com/chubin/wttr.in
    say "#" i":" command                                    -- give user feedback
    outArr = .array~new                                     -- array for stdout
    ADDRESS SYSTEM command with output using (outArr) error using (.array~new)
    cityWeather~append(outArr[1])                           -- append output to array
end

-- Start Excel with empty worksheet
excelApplication = .OLEObject~new("Excel.Application")
excelApplication~visible = .true                            -- make Excel visible
Worksheet = excelApplication~Workbooks~Add~Worksheets[1]    -- add worksheet
-- Create bold collum header in first row
colhead = .array~of("City", "Celsius")                      -- array with header text
do counter col name over colhead
    Worksheet~cells(1,col)~Value = name                     -- insert items from array
    Worksheet~cells(1,col)~font~bold = .true                -- make bold
end
-- Insert information from gained with curl
do counter row name over cityWeather
    parse var name city ":" temperature "°C"               -- parse curled informations
    Worksheet~cells(row+1,1)~Value = city                   -- city in collum 1
    Worksheet~cells(row+1,2)~Value = temperature            -- temperature in collum 2
end
-- Select range for chart, use left upper and lower right cell location
Worksheet~Range("A1:B" || (row+1))~Select                   -- include title row
-- Add Chart
excelApplication~Charts~Add                                 -- create new chart
excelApplication~ActiveChart~HasTitle = .True               -- add title
excelApplication~ActiveChart~ChartTitle~Characters~Text = "Temperature"

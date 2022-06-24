/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/* MSExcel.rex: OLE Automation with ooRexx                            */
/*                                                                    */
/* Create Microsoft Excel sheet, enter some data and save it.         */
/*                                                                    */
/**********************************************************************/

excelApplication = .OLEObject~new("Excel.Application")
excelApplication~visible = .true             -- make Excel visible
Worksheet = excelApplication~Workbooks~Add~Worksheets[1]

colTitles = "ABCDEFGHI"                      -- define first nine column letters
lastLine = 12                                -- number of lines to process

   /* HINT: if your local Excel user interface language is not English, you may need to rename the
            function name 'sum' to your user interface language, e.g. in German to 'summe' */
sumFormula = "=sum(?2:?"lastLine-1")"        -- English formula: question marks will be changed to column letter
say "sumFormula:      " sumFormula "(question marks will be changed to column letter)"

xlHAlignRight = excelApplication~getConstant("xlHAlignRight") -- get value of "horizontal align right" constant

do line = 1 to lastLine                      -- iterate over lines
  do col = 1 to colTitles~length             -- iterate over columns
    colLetter = colTitles[col]               -- get column letter
    cell = Worksheet~Range(colLetter||line)  -- e.g. ~Range("A1")

    if line = 1 then do                -- first row? yes, build title
      cell~value = "Type" colLetter          -- header in first row
      cell~font~bold = .true                 -- make font bold
      cell~Interior~ColorIndex = 36          -- light yellow
      cell~style~horizontalAlignment = xlHAlignRight  -- right adjust title
    end
    else if line = lastLine then do    -- last row? yes, build sums
      /* set formula, e.g. "=sum(B2:B9)" */
      cell~formula = sumFormula~changeStr("?",colLetter) -- adjust formula to column to sum up
      cell~Interior~ColorIndex = 8           -- light blue
    end
    else do -- a row between 2 and 9: fill with random values
      cell~value = random(999999) / 100      -- create a random decimal value
      cell~font~ColorIndex = 11              -- set from black to violet
    end
  end
end

   -- check whether Excel's user interface language causes the "#NAME?" error, if so advice
sumCell = WorkSheet~range("A"lastLine)       -- get sum-cell of column A
if sumCell~text = "#NAME?" then
do
   say
   say "** Excel reports a '#NAME?' error for the 'sum' function! Probable cause: **"
   say "** your local Excel user interface language is not set to English, therefore you need **"
   say "** to adjust the function name 'sum' in the variable 'sumFormula' to your user interface **"
   say "** language and rerun this program (e.g. in German you need to rename 'sum' to 'summe') **"
   say "** sumCell~formula:" sumCell~formula
   say "** sumCell~text:   " sumCell~text
   say "** sumCell~value:  " sumCell~value
   say
end

   -- create a format string for our numbers, use thousands and decimal separators
formatString = "#"excelApplication~thousandsSeparator"##0"excelApplication~decimalSeparator"00"
say "formatString:    " formatString           -- show format string

excelApplication~useSystemSeparators = .false   -- allow our format string to be used everywhere
stringRange="A2:"colTitles~right(1)lastLine
say "formatting range:" stringRange
WorkSheet~range(stringRange)~numberFormat = formatString -- get range and set its number format

   -- make sure that file gets quietly overwritten in case it exists already
excelApplication~DisplayAlerts = .false      -- no alerts from now on

/* save sheet in user's home directory */
homeDir = value("USERPROFILE",,"ENVIRONMENT")-- get value for environment variable "USERPROFILE"
fileName = homeDir"\samp09_ooRexx.xlsx"      -- build fully qualified filename
say "fully qualified fileName:" fileName     -- show fully qualifed filename
Worksheet~SaveAs(fileName)                   -- save file

   -- let the user inspect the Excel file
say "Excel sheet got saved to file, press enter to continue ..."
parse pull .                                 -- wait for user to press enter
excelApplication~Quit                        -- close Excel


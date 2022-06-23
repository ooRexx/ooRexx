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
 AOO_scalc_chart.rex using OLE (object linking and embedding) with ooRexx

 Links:  <https://OpenOffice.org>
         <https://wiki.openoffice.org/wiki/Documentation/DevGuide/ProUNO/Bridge/Automation_Bridge>
         <https://www.pitonyak.org/oo.php>
         <https://www.openoffice.org/udk/common/man/spec/ole_bridge.html>

 Using OLE create a new scalc worksheet, create random data and a chart based on it.
 Demonstrates how to use UNO reflection and incorporate UNO_CONSTANTS and UNO_ENUM
 values into a Rexx directory for easier use.
***********************************************************************/

/* create a spreadsheet, add data for comparing two years by quarter, add a chart */
serviceManager = .OLEObject~new('com.sun.star.ServiceManager')
/* create spreadsheet, get first sheet */
desktop  = serviceManager~createInstance('com.sun.star.frame.Desktop')
noProps  = .array~new   /* empty array (no properties)   */
document = desktop~loadComponentFromURL('private:factory/scalc', '_blank', 0, noProps)
sheet    = document~sheets~getByIndex(0)  -- get first spreadsheet

/* note values can be assigned to cells with "string", "formula" or "value" (for numbers) */
/* create the titles, pretend the last two yers */
year   = date()~right(4)-2
titles = "Quarter", year, year+1 /* title array */
do col = 1 to titles~items
   sheet~getCellByPosition(col-1,0)~string = titles[col]
end
/* get all UNO_ENUM values in a Rexx directory */
justify = getAsDirectory(serviceManager, "com.sun.star.table.CellHoriJustify")
say "justify:" justify
/* right adjust the last two years
   - possibility 1: sheet~getCellRangeByName("B1:C1")~setPropertyValue("HoriJustify", justify~right)
   - or: */
sheet~getCellRangeByName("B1:C1")~HoriJustify = justify~right

/* get all UNO_CONSTANTS values in a Rexx directory */
weights = getAsDirectory(serviceManager,"com.sun.star.awt.FontWeight")
say "weights:" weights
sheet~getCellRangeByName("A1:C1")~CharWeight = weights~bold /* column headings   */

/* create random values for the quarter numbers  */
do line = 1 to 4
   sheet~getCellByPosition(0,line)~string = "Q"line   /* title in first column   */
   sheet~getCellByPosition(1,line)~value = random(0,500000)/100
   sheet~getCellByPosition(2,line)~value = random(0,750000)/100
end
sheet~getCellRangeByName("A2:A5")~CharWeight = weights~bold /* column headings   */
/* format numbers, predefined style, format: "#,##0.00" */
sheet~getCellRangeByName("B2:C5")~setPropertyValue("NumberFormat",4)

/* create a chart from the data */
structRect = serviceManager~bridge_getStruct("com.sun.star.awt.Rectangle")
structRect~X      =   300           -- x-offset:  0.300 cm
structRect~Y      =  2250           -- y-offset:  2.250 cm
structRect~Width  = 16000           -- width:    16.000 cm
structRect~Height =  8000           -- height:    8.000 cm

range       = sheet~getCellRangeByName("A1:C5")    /* data to be used for the chart */
rangeAddr   = range~getRangeAddress
arrOfAddr   = .array~of(rangeAddr)  /* create array with the range address       */
tableCharts = sheet~getCharts       /* get chart collection & insert             */
tableCharts~addNewByName("FirstChart", structRect, arrOfAddr, .true, .true)

/* Routine returns a Rexx directory containing all names and values of the supplied
   UNO_CONSTANTS or UNO_ENUM class name (needs to be fully qualified).  */
::routine getAsDirectory
  use strict arg serviceManager, unoClzName

  dir = .Directory~new              -- directory will get
  dir~objectName = unoClzName       -- allows to show the uno class it represents

  ctxt = serviceManager~defaultContext
  tdm = ctxt~getValueByName("/singletons/com.sun.star.reflection.theTypeDescriptionManager")
  reflClz= tdm~getByHierarchicalName(unoClzName)
  if reflClz~isNil then return dir  -- return empty directory

  typeClass = reflClz~getTypeClass
  if typeClass = 30 then         -- UNO_CONSTANTS
  do
     dir~objectName = unoClzName "(UNO_CONSTANTS)" -- supply type info to name
     do c over reflClz~getConstants -- iterate over constant fields
        name = c~getName            -- fully qualified
        name = name~substr(name~lastPos('.')+1) -- extract last word
        dir[name] = c~getConstantValue -- store constant values with their names
        -- say "name:" name "->" c~getConstantValue
     end
  end
  else if typeClass = 15 then    -- UNO_ENUMERATION
  do
     dir~objectName = unoClzName "(UNO_ENUM)"   -- supply type info to name
     enumNames = reflClz~getEnumNames     -- get all enumeration names
     enumValues = reflClz~getEnumValues   -- get all enumeration values
     do i=1 to enumNames~items
        name = enumNames[i]
        name = name~substr(name~lastPos('.')+1) -- extract last word
        dir[name] = enumValues[i]   -- store enum values with their names
        -- say "name:" name "->" enumValues[i]
     end
  end
  return dir

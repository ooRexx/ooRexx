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
 AOO_swriter_paragraphs.rex using OLE (object linking and embedding) with ooRexx

 Links:  <https://OpenOffice.org>
         <https://wiki.openoffice.org/wiki/Documentation/DevGuide/ProUNO/Bridge/Automation_Bridge>
         <https://www.pitonyak.org/oo.php>
         <https://www.openoffice.org/udk/common/man/spec/ole_bridge.html>

 Using OLE create a new swriter document, add paragraphs that get aligned
 in four different ways. Demonstrates how to use UNO reflection and
 incorporate UNO_CONSTANTS and UNO_ENUM values into a Rexx directory for
 easier use.
***********************************************************************/

/* create a text document, demonstrate how to align paragraphs */
serviceManager = .OLEObject~new('com.sun.star.ServiceManager')
/* create text document */
desktop  = serviceManager~createInstance('com.sun.star.frame.Desktop')
noProps  = .array~new   /* empty array (no properties)   */
document = desktop~loadComponentFromURL('private:factory/swriter', '_blank', 0, noProps)

text = document~getText                -- get text object
text~setString("Hello, this is ooRexx on:" .DateTime~new"!")
cursor = text~createTextCursor

ctlChars = getAsDirectory(serviceManager, "com.sun.star.text.ControlCharacter")  -- UNO_CONSTANT
paraBreak = ctlChars~paragraph_break   -- get paragraph break constant

paraAdj = getAsDirectory(serviceManager, "com.sun.star.style.ParagraphAdjust")   -- UNO_ENUM

arr = .array~of("right", "center", "block", "left")   -- adjustments
do adj over arr   -- iterate over adjustments, create string, adjust
   cursor~gotoEnd(.false)     -- position at end
   text~insertControlCharacter(cursor, paraBreak, .false)
   string = ("This paragraph will be" adj"-adjusted. ")~copies(8)
   text~insertString(cursor, string, .true)
   -- fetch appropriate adjust enum value from directory
   cursor~setPropertyValue("ParaAdjust", paraAdj~send(adj))
end


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

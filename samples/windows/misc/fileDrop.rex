/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

/**
 * Demonstrates handling files 'dropped' on this program.  In Windows explorer,
 * drag any file over this program and drop it.  The OS will then execute this
 * program and provide the complete path name of the 'dropped' file as the
 * argument to this program.  The program then 'handles' the dropped file.  Use
 * fileDrop.input to simulate processing a data input file.
 *
 * Note that you can also run this program from the command prompt by supplying
 * the correct file name argument.  I.e.:
 *
 * rexx fileDrop.rex fileDrop.input
 *
 */
use arg fileName

  -- Note that the OS seems to tack an extra space on the end of the file name
  -- argument.  So, we strip it off to start with.
  fileName = fileName~strip

  -- Display the fileName argument.
  msg = "Got file name argument:" || .endOfLine || .endOfLine || fileName
  j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")

  -- Check if the file exists.  If not, this program was probably invoked from
  -- the command line rather than through drag-and-drop.
  if \ SysFileExists(fileName) then do
    msg = "The file does not exist."  || .endOfLine   || .endOfLine || -
          "Perhaps you are not using drag and drop?"  || .endOfLine || -
          "In Windows explorer, drag a file and drop" || .endOfLine || -
          "it on this program's icon."
    j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")
    return 9
  end

  -- Check if it is a directory.
  if SysIsFileDirectory(fileName) then do
    msg = 'Dropped file is a directory.  Will print out directory contents.'
    j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")

    count = printDirectory(fileName)
    msg = 'There were' count 'entries in the directory.  Quitting.'
    j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")
    return 0
  end

  -- Should be a file, be sure we can read it.
  f = .stream~new(fileName)
  if f~open("READ") \== "READY:" then do
    msg = "Could not open:" || .endOfLine || .endOfLine || -
          fileName          || .endOfLine || .endOfLine || -
          "for reading. Quitting"
    j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")
    return 9
  end

  -- Collect some data on the file and then display it.
  count = f~lines
  if count >= 3 then do
    line = f~linein(3)
    number = 3
  end
  else if count >= 1 then do
    line = f~linein(1)
    number = 1
  end
  else do
    line = "The file is empty."
    number = 0
  end
  f~close

  msg = "The file has" count "lines." || .endOfLine || .endOfLine
  if number == 0 then do
    msg = msg || line
  end
  else do
    msg = msg || "Line number" number "reads as follows:" || .endOfLine || .endOfLine || line
  end

  j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")

  -- This would be a check that the file is a correct data input file.
  if line == "----data input file----" then do
    recordCount = updateDataBase(f)
    msg = "Processed" recordCount "database records.  Finished."
  end
  else do
    msg = "Invalid data file.  Quitting"
  end

  j = RxMessageBox(msg, "ooRexx File Drop Handling", "OK", "INFORMATION")
  return 0

-- Print out the directory entries in the specified directory
::routine printDirectory
  use strict arg directoryName

  dir = directoryName || '\*.*'
  count = -1

  j = SysFileTree(dir, f., 'BT')
  if j == 0 then do
    do i = 1 to f.0
      say f.i
    end
    count = f.0
  end

  return count

-- Update the database use the specified information.
::routine updateDataBase
  use strict arg f

  -- Simulate processing / updating database records using the data input file.
  -- We just fake it.
  countProcessed = 0
  ret = f~seek('=1 READ')
  do while f~state == "READY"
    line = f~linein
    countProcessed += 1
  end

  countProcessed *= random(2, 9)

  f~close
  return countProcessed

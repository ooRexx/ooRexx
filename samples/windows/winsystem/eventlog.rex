/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* Name: eventlog.rex                                                       */
/* Type: Open Object Rexx Script                                            */
/*                                                                          */
/* Description: Example of how to use the WindowsEventLog class             */
/*                                                                          */
/****************************************************************************/

/* main program */

log = .WindowsEventLog~new

/******************************************************************************/
say "=============================================================================="
say "  Display some basic attibutes of a WindowsEventLog object.  Consult the"
say "  documentation for an explanation of the meaning of these attributes."
say
/******************************************************************************/
say "  Events attribute:                " log~events
say "  Current minimum read buffer size:" log~minimumReadBuffer "bytes"
say "  Current minimum read buffer size:" log~minimumRead "KBs"
say "  Minimum read buffer range:       " log~minimumReadMin "KBs to" log~minimumReadMax "KBs"
say

/******************************************************************************/
say "=============================================================================="
say "  Open each event log on this computer and display information about the"
say "  log.  Then close the log."
say
/******************************************************************************/
logNames = .array~new
ret = log~getLogNames(logNames)

if ret == 0 then do name over logNames
  ret = log~open( , name)

  if ret == 0 then do
    say "  Event log:         " name
    say "  Total records:     " log~getNumber
    say "  First record number" log~getFirst
    say "  Last record number " log~getLast
    say "  Log is full?       " logicalToText(log~isFull)
    log~close
  end
  else do
     say "  Error opening the" name "event log rc:" ret
  end
  say
end


/******************************************************************************/
say "=============================================================================="
say "Going to read the 3 oldest records in the Application event Log,"
say "oldest first.  The log is not opened before reading.  The open and"
say "close are done implicitly."
/******************************************************************************/
ok = doPause(.false)

count = 0

if log~getNumber < 3 then do
  say "To few records ("log~getNumber") for this demonstration."
end
else do
  -- If the user has set the overwrite option for the event log, the first
  -- record is not always 1.   FORWARDS is the default.
  firstRec = log~getFirst
  ret = log~readRecords( , , , firstRec, 3)

  if ret == 0 then do
    count = displayRecords(log~events)
  end
  else do
    say "Error reading the Application event log rc:" ret
  end
end

say "=========================================================================="
say count "records read"
say

/******************************************************************************/
say "=========================================================================="
say "Going to read the 3 most recent records in the System event Log, the"
say "most recent first.  The log is not opened before reading.  The open"
say "and close are done implicitly."
/******************************************************************************/
ok = doPause(.false)

count = 0

if log~getNumber("", "System") < 3 then do
  say "To few records (" || log~getNumber("", "System") || ") for this demonstration."
end
else do
  -- If the user has set the overwrite option for the event log, the most recent
  -- record is not always the same as the record number.
  lastRec = log~getLast( , "System")
  ret = log~readRecords("BACKWARDS", , "System", lastRec, 3)

  if ret == 0 then do
    count = displayRecords(log~events)
  end
  else do
    say "Error reading the System event log rc:" ret
  end
end

say "=========================================================================="
say count "records read"
say

/******************************************************************************/
say "=========================================================================="
say "Going to read the 3 oldest records in the System event Log, in reverse"
say "chronological older.  The log is opened before any operation, then"
say "closed when all operations are done.  This allows specifying the log"
say "name only once."
/******************************************************************************/
ok = doPause(.false)

count = 0

ret = log~open("", "System")

if ret == 0 then do
  totalRecs = log~getNumber

  if totalRecs < 3 then do
    say "To few records (" || totalRecs || ") for this demonstration."
  end
  else do
    -- If the user has set the overwrite option for the event log, the first
    -- record is not always number 1.
    firstRec = log~getFirst

    -- Now calculate the record number we are going to start at.
    startRec = firstRec + 3 - 1  -- Same as firstRec + 2 of course.

    ret = log~readRecords("BACKWARDS", , , startRec, 3)

    if ret == 0 then do
      count = displayRecords(log~events)
    end
    else do
      say "Error reading the System event log rc:" ret
    end
  end
end
else do
  say "Error opening the System event log rc:" ret
end

-- It is always safe to invoke the close() method.  If no log is open, then the
-- method does nothing.
log~close

say "=========================================================================="
say count "records read"
say


/******************************************************************************/
say "=========================================================================="
say "Going to read the complete Security event log forwards.  If there are"
say "a log of records, this will produce substantial output.  You can cancel"
say "if you wish."
/******************************************************************************/
ok = doPause(.true)
count = 0

if ok then do

  ret = log~readRecords("FORWARDS", , "Security")
  select
    when when ret == 0 & log~events~items == 0 then do
      -- On many systems the security log is empty.
      say "The Security event log has no event records."
    end

    when ret <> 0 then do
      say "Error reading the Security event log rc:" ret
    end

    otherwise do
      count = displayRecords(log~events)
    end
  end
  -- End select
end
else do
  say "Skipping this demonstartion."
end

say "=========================================================================="
say count "records read"
say


/******************************************************************************/
say "=========================================================================="
say "Going to read the complete System event log in reverse chronological"
say "order.  If there are a lot of records, this will produce substantial"
say "output.  You can cancel if you wish."
/******************************************************************************/
ok = doPause(.true)
count = 0

if ok then do

  ret = log~readRecords("BACKWARDS", , "System")
  select
    when when ret == 0 & log~events~items == 0 then do
      say "The System event log has no event records."
    end

    when ret <> 0 then do
      say "Error reading the System event log rc:" ret
    end

    otherwise do
      count = displayRecords(log~events)
    end
  end
  -- End select
end
else do
  say "Skipping this demonstartion."
end

say "=========================================================================="
say count "records read"
say


/******************************************************************************/
say "=========================================================================="
say "Going to write some records to the Application event log."
say "You can skip this step if you do not want records added to"
say "your Application event log."

/* Some notes on how the event logging service works.  Typically, applications
   that want to write records to one of the event logs, add entries to the
   registry.  These registry entries control how the event logging service
   handles the writing of a record.

   The registry entries are written to subkeys under the Event log service.

   If an application has not added the registry entries, then the event records
   are *always* written to the Application log.  This can not be over-ridden.

   So, say there is an application, MyApplication that wants to write a record
   using the source name of MyApplication to an event log.  To do that the
   source name must be a subkey of a logfile entry under the EventLog key in the
   registry.

   If MyApplication should be written to the System log the registry key must
   be added like this:

   HKEY_LOCAL_MACHINE
   	  System
   	     CurrentControlSet
   	        Services
           	   EventLog
   	              System
   							     MyApplication
   	           Security
           	   System

   If the key does not exists, the Application event log is always used.
*/

/******************************************************************************/
ok = doPause(.true)

if ok then do
  source     = "MyLog"
  type       = 4  -- Information
  category   = 22
  id         = 33
  binaryData = "01 1a ff 4b 0C"x

  ret = log~write( , source, type, category, id, binaryData, "String1", "sTring2")
  if ret == 0 then
    say "Record" source "successfully written"
  else
    say "Error writing record" source "rc:" ret

  -- The default type for writing a record is 'Error' This might look a little
  -- scary to a user, so we will use Information for the type.
  type = 4  -- Information

  ret = log~write(, "Application2", type,,, "1A 1B 1C 0000 00"x, "First String", "Second String")
  if ret == 0 then
    say "Record Application2 successfully written"
  else
    say "Error writing record Application2 rc:" ret

  ret = log~write(, "Security1", type,,,"1A1B1C000000"x, "Tom", "John", "Larry", "Frank")
  if ret == 0 then
    say "Record Security1 successfully written"
  else
    say "Error writing record Security1 rc:" ret

  ret = log~write(,"Security2", type,,,"04 03 01 1B 1C 00 FF 00 FE 00 FD"x, "One", "Two")
  if ret == 0 then
    say "Record Security2 successfully written"
  else
    say "Error writing record Security2 rc:" ret

  ret = log~write(, "System1", type,,,"1A 1B 1C 00 00 00"x, "1", "2", "3", "4")
  if ret == 0 then
    say "Record System1 successfully written"
  else
    say "Error writing record System1 rc:" ret

  ret = log~write(, "System2", type,,,"1A1B1C000000"x, "House", "Apartment")
  if ret == 0 then
    say "Record System2 successfully written"
  else
    say "Error writing record System2 rc:" ret

  ret = log~write(, "NotRegisteredAppliaction1", type,,,"1A1B1C0000FF00"x, "Cat", "Dog", "Rabbit")
  if ret == 0 then
    say "Record NotRegisteredAppliaction1 successfully written"
  else
    say "Error writing record NotRegisteredAppliaction1 rc:" ret

  rawData = "4d 61 72 6b 20 4d 69 65 73 66 65 6c 64 0d 0a"x
  ret = log~write(, "NotRegisteredAppliaction2", type,,, rawData, "ooRexx", "example")
  if ret == 0 then
    say "Record NotRegisteredAppliaction2 successfully written "
  else
    say "Error writing record NotRegisteredAppliaction2 rc:" ret
end
else do
  say "Skipping this demonstartion."
end

say "=========================================================================="
say


/******************************************************************************/
say "=============================================================================="
say "The next and last demonstration will clear the Application event log."
/******************************************************************************/

backupFile = getWriteableFileName()

if backupFile == "" then do
  say "Could not determine with certainty a writeable location for a backup file."
  say "You may want to skip this demonstration, the log will not be backed up."
end
else do
  say "The log will first be backed up to this file:"
  say " " backupFile
end

say
say "You can skip this step if you do not want to clear"
say "your Application event log."
say
say "If this is the first time you ran this example, you"
say "may want to skip this test so that you can see the"
say "records written to the log in the previous demonstration."
say "(If you elected to write any records.)"
say
say "If you previously ran this example and saved the Application"
say "log, you may want to move the back up file to a different"
say "location.  The same back up file name is used each time the"
say "example executes."

ok = doPause(.true)
if ok then do
  if backupFile \== "" then ret = log~clear(, , backupFile)
  else ret = log~clear

  if ret == 0 then say "The Application event log was cleared successfully."
  else say "Error clearing the Application event log rc:" ret
end

return 0   /* leave program */

::requires "winsystm.cls"

/* Routine to display the event log records */
::routine displayRecords
  use strict arg records

  say records~items "records read"

  do record over records
    say "=========================================================================="
    parse var record type date time "'" sourcename"'" id userid computer "'" string "'" "'" data "'"
    say 'Type     : 'type
    say 'Date     : 'date
    say 'Time     : 'time
    say 'Source   : 'sourcename
    say 'ID       : 'id
    say 'UserId   : 'userid
    say 'Computer : 'computer
    say 'Detail   : 'string
    say 'Data     : 'data
  end

return records~items

/* Simple routine to do a logical into text */
::routine logicalToText
  use strict arg logical
  if logical then return "True"
  else return "False"

/* Simple routine to pause the program */
::routine doPause
  use arg confirm

  say
  if confirm then do
    say "Continue? (y/n)"
    parse lower pull answer
    ok = (answer~left(1) == 'y')
  end
  else do
    say "Press Enter to continue"
    pull
    ok = .true
  end
  say

return ok

::routine getWriteableFileName

  -- Find this user's Documents directory, surely that is writeable.

  shell = .oleObject~new('Shell.Application')
  folderConstant = '5'~x2d()
  folderObj = shell~nameSpace(folderConstant)
  folderItem = folderObj~self
  documentsFolder = folderItem~path

  backupFile = ""

  -- Check that we got a good directory
  if SysIsFileDirectory(documentsFolder) then do
    -- Unfortunately, we can not use a network mapped drive and it is not
    -- uncommon for people to have their My Documents directory on a network
    -- share.
    drv = documentsFolder~left(2)
    if SysDriveMap(drv, "REMOTE")~caselessWordPos(drv) == 0 then do
      backupFile = documentsFolder || '\applicationBackup_01.evt'
    end
  end

  if backupFile == "" then do
    -- Keep looking, see if we can write to the current directory.
    currentDir = directory()
    if canWriteToDir(currentDir) then do
      backupFile = currentDir || "\applicationBackup_01.evt"
    end
    else do
      -- As a last resort, try the temp directory
      tempDir = value("TEMP", , 'ENVIRONMENT')
      if canWriteToDir(tempDir) then do
        backupFile = tempDir || "\applicationBackup_01.evt"
      end
    end
  end

return backupFile

::routine canWriteToDir
  use strict arg dir

  tmpFile = dir || '\' || "tmpFileWriteTest.deleteMe"

  -- Write to the file, close it, test it, delete it.
  j = lineout(tmpFile, "Delete this file if you find it")
  j = lineout(tmpFile)
  if SysIsFile(tmpFile) then do
    answer = .true
  end
  else do
    answer = .false
  end

  'del /F /Q' tmpFile '1>nul 2>&1'

return answer

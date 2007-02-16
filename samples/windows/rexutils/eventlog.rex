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
/****************************************************************************/
/* Name: eventlog.rex                                                       */
/* Type: Open Object Rexx Script                                            */
/*                                                                          */
/* Description: Sample how to use the WindowsEventLog class                 */
/*                                                                          */
/****************************************************************************/

/* main program */

evl = .WindowsEventLog~new

if evl~InitCode \= 0 then exit


/******************************************************************************/
say "=============================================================================="
say "Open System event log and get number of event log records and close it"
say
/******************************************************************************/
rc = evl~Open(,"System")

if rc = 0 then
do
   say "Number of records within the System Event Log : "evl~GetNumber
   say "=============================================================================="
   evl~Close
end
else
   say "Error opening Event Log"

/******************************************************************************/
say "=============================================================================="
say "Reading complete Application Event Log forwards without opening before"
say "Open and close is done implicit"
/******************************************************************************/
say "Press Enter to continue"
pull

events = evl~Read

if events \= .nil then
  call DisplayRecords
else
  say "$$Error reading Application Event Log"

/******************************************************************************/
say "=============================================================================="
say "Reading complete Security Log forwards without opening before"
say "Open and close is done implicit"
/******************************************************************************/
say "Press Enter to continue"
pull

events = evl~Read("FORWARDS",,"Security")

if events \= .nil then
  call DisplayRecords
else

say "$$Error reading Security Log or Log is empty"

/******************************************************************************/
say "=============================================================================="
say "Reading complete System log forwards without opening before"
say "Open and close is done implicit"
/******************************************************************************/
say "Press Enter to continue"
pull

events = evl~Read("FORWARDS",,"System")

if events \= .nil then
  call DisplayRecords
else
  say "$$Error reading System Log"

/******************************************************************************/
say "=============================================================================="
say "Writing records to Application Log"
say "The source names must be a subkey of a logfile entry under the EventLog key in the registry."
/* If Application1 should be written to the System log the registry key must be added:
HKEY_LOCAL_MACHINE
	  System
	    CurrentControlSet
	      Services
        	EventLog
	          System
							Application1
	          Security
        	  System

If the key does not exists, the Application Log is used !
So if the proram runs without changing the registry, all records arw written into the
Application Log !
*/
/******************************************************************************/
say "Press Enter to continue"
pull

rc = evl~Write(,"MyLog",11,22,33,"01 1a ff 4b 0C"x,"String1","sTring2")
if rc = 0 then
	say "Record Application1 successfully written"
else
	say "$$Error writing record Application1 !"

rc = evl~Write(,"Application2",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record Application2 successfully written"
else
	say "$$Error writing record Application2 !"

rc = evl~Write(,"Security1",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record Security1 successfully written"
else
	say "$$Error writing record Security1 !"

rc = evl~Write(,"Security2",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record Security2 successfully written"
else
	say "$$Error writing record Security2 !"

rc = evl~Write(,"System1",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record System1 successfully written"
else
	say "$$Error writing record System1 !"

rc = evl~Write(,"System2",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record System2 successfully written"
else
	say "$$Error writing record System2 !"

rc = evl~Write(,"NotRegisteredAppliaction1",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record NotRegisteredAppliaction1 successfully written"
else
	say "$$Error writing record NotRegisteredAppliaction1 !"

rc = evl~Write(,"NotRegisteredAppliaction2",,,,"1A 1B 1C 0000 00"x,"First String", "Second String")
if rc = 0 then
	say "Record NotRegisteredAppliaction2 successfully written "
else
	say "$$Error writing record NotRegisteredAppliaction2 !"


/* Example to clear the Application Log                                       */
/* To avoid unintentional clearing of the log, the code is commented          */
/* To try it please uncomment it                                              */
/******************************************************************************/
--say "=============================================================================="
--say "Clearing the Application Event log "
--say
/******************************************************************************/
/*
say "Press Enter to continue"
pull

rc = evl~Clear(,,"e:\temp\application_1.evt")
say rc
*/


evl~deinstall

exit 0   /* leave program */



/* Routine to display the event log records */
DisplayRecords:

say evl~Events~items "records read"

do i=1 to evl~Events~items
  say "====================================================================================="
  temp = evl~Events[i]
  parse var temp type date time "'" sourcename"'" id userid computer "'" string "'" "'" data "'"
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

return

::requires "winsystm.cls"

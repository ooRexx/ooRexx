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
/* Name: EditRex.rex                                                        */
/* Type: Open Object Rexx OODialog Script                                   */
/*                                                                          */
/* Description:                                                             */
/* This script reads the registry entries for the ftype REXXScript EDIT     */
/* type and allows to change it between Notepad.EXE and any                 */
/* given EDITOR                                                             */
/*                                                                          */
/* It uses the WindowsRegistry Class and the PlainUserDialog Class.         */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/* See if user gave directions */
parse arg Interface

/* The user may decide between Console or OODIALOG Version */
/* Default is OODIALOG Version */
/* If the ftype Open Command Setting is to rexxhide, the Console Version will NOT work */
/* Verify if the given parm starts with "C", TRANSLATE changes it to UPPER CASE */
if Interface~LEFT(1)~TRANSLATE = "C" then Interface = "CONSOLE"
else Interface = "OODIALOG"

/* create a new registry object */
r = .WindowsRegistry~new

/* leave if init failed */
if r~InitCode \= 0 then do
  call errorDialog 'Error open the registry. Program aborted.'
  exit
end

/* Set from default HKEY_LOCAL_MACHINE to HKEY_CLASSES_ROOT */
r~Current_Key = r~CLASSES_ROOT

/* Open the Key with Options QUERY and WRITE */
if r~open(,"REXXScript\Shell\Edit\Command","QUERY WRITE") == 0 then do
  call errorDialog 'Error opening the registry key with write access.' || '0d0a0d0a'x || -
                    'If you are on Vista you must run this program with' || '0d0a'x || -
                    'elevated privileges to see it work.'
  exit
end

/* Retrieves information about a given key in a compound variable */
/* q.values holds the number of value entries. */
q. = r~Query

/* Retrieves all value entries of a given key into a compound variable */
if r~ListValues(,vals.) = 0 then do
  /* There are 3 possible values for each entry:
     vals.i.name  the Name of the value
     vals.i.data  the data of the entry
     vals.i.type  the type : NORMAL for alphabetic values,
                             EXPAND for expandable strings such as a path,
                             NONE for no specified type,
                             MULTI for multiple strings,
                             NUMBER for a 4-byte value, and
                             BINARY for any data format.
  */
  /* get the current program only. Do not show the "%1" parameter */
  /* TRANSLATE it to UPPER CASE */
  program = vals.1.data~word(1)~TRANSLATE
end
else do
  call errorDialog 'Error reading the registry. Program aborted.'
  exit
end

/* Ask user what to do */
if Interface = "CONSOLE" then do
  /* The next lines use an plain text interface */
  /* Show the curent content */
  say 'Current Edit Setting is : ' || program
  say 'Please enter a number + <Enter> to set it to :'
  say '1 to EDIT it with NOTEPAD.EXE'
  say '2 to EDIT with another program'
  say 'Any other number to leave without changes'
  say
  /* Get selection */
  pull answer
end
else do
  /* Get the current state, to make preselection in the dialog */
  if program~LASTPOS('NOTEPAD.EXE') > 0 then answer = '1'
  else answer = '2'
  /* The next lines use an "Single Selection Dialog" as interface to the user */
  sel.1 = "EDIT it with NOTEPAD.EXE"
  sel.2 = "SELECT another program, currently set to : " || program
  /* prepare the dialog */
  dlg = .SingleSelection~new("Please select what to do","Ftype EDIT setting for ooRexx",sel.,answer,,answer)
  /* show the dialog */
  answer = dlg~execute

  /* end of user interaction */
end

/* verify what to do */
select

  when answer = '1'  then do
    newval = 'notepad.exe "%1"'
  end

  when answer = '2' then do
    if Interface = "CONSOLE" then do
      say
      say 'Please enter Path and name of the wanted editor '
      pull program
    end
    else do
      /* Use the FilenameDialog to let the user select an other program */
      program = FilenameDialog('',,'Executables (*.EXE)'||'0'x||'*.EXE'||'0'x||'All Files (*.*)'||'0'x||'*.*','LOAD','Select program to use','EXE')
    end
    if program = '0' then exit  /* Canceled by user */
    newval = program || ' "%1"'
  end
  otherwise exit
end

/* Set the new value */
/* Sets a named value of a given key */
/* If name is blank or omitted, the default value is set */
r~SetValue(r~Current_Key,"",newval,NORMAL)

/* Forces the system to write the cache buffer of a given key to disk */
/* If key_handle is omitted, CURRENT_KEY is flushed */
r~flush(r~Current_Key)

/* Closes a previously opened key specified by its handle */
/* Since it can take several seconds before all data is written to disk,*/
/* FLUSH was used before to empty the cache */
/* If key_handle is omitted, CURRENT_KEY is closed */
r~close(r~Current_Key)

msg = 'The EDIT setting for ooRexx should now be' newVal

if Interface = "CONSOLE" then do
  say msg
end
else do
  call InfoDialog msg
end

::requires "winsystm.cls" -- required for the registry class
::requires "OODPLAIN.cls" -- required for the dialog class

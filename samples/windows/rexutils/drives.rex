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
/*****************************************************************************/
/* Name: DRIVES.REX                                                          */
/* Type: Open Object Rexx Script                                             */
/*                                                                           */
/* Description: Sample program to display information about drives using the */
/*              Open Object Rexx utility functions SysDriveMap, SysDriveInfo,*/
/*              SysFileSystemType, SysBootDrive                              */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* Main Programm                                                             */
/*****************************************************************************/
parse arg InputDrives               -- get the input                        --
                                                                            --
Drives. = ""                        -- initialize Drives stem               --
                                                                            --
call CheckInput                     -- check input                          --
                                                                            --
call InfoParsing                    -- get and parse the drive data         --
call InfoDisplay "BOX"              -- display data with box characters     --
--call InfoDisplay "LINE"           -- display data without box characters  --

exit

/*****************************************************************************/
/* Procedure   :  InfoParsing                                                */
/*                                                                           */
/* Description : Get the informations of each drive using the Object REXX    */
/*               utility functions.                                          */
/*                                                                           */
/* Parameters  :  EXPOSE:  Drives. -  stem to hold all drives informations   */
/*                                                                           */
/*****************************************************************************/
InfoParsing: PROCEDURE EXPOSE Drives.

UsedDrives = SysDriveMap()                         -- get all used drives     --
                                                                              --
RemoteDrives = SysDriveMap(, 'REMOTE')             -- get all remote drives   --
                                                                              --
do i = 1 to Drives.0                               -- loop over all drives    --
  DriveInfo = SysDriveInfo(Drives.i.InputLetter)   -- get drive information   --
                                                                              --
  parse var DriveInfo,                             -- parse ...               --
            Drives.i.OutputLetter,                 -- drive letter            --
            Drives.i.Free,                         -- free space              --
            Drives.i.Total,                        -- total space             --
            Drives.i.Label                         -- label                   --
                                                                              --
  Drives.i.Label = strip(Drives.i.Label)           -- eliminate spaces        --
  Drives.i.Free  = BeautifyNumber(Drives.i.Free)   -- enhance readability of  --
  Drives.i.Total = BeautifyNumber(Drives.i.Total)  -- the numbers             --

  if Drives.i.OutputLetter = "" & pos(Drives.i.InputLetter, UsedDrives) >0  then do
    -- This is a CD-ROM drive, set the label and the drive letter
    Drives.i.Label = "CD-ROM"
    Drives.i.OutputLetter = Drives.i.InputLetter
  end
  else do
    -- if no available drive set all information data to unknown
    if pos(Drives.i.InputLetter, UsedDrives) = 0 then do
      Drives.i.OutputLetter = Drives.i.InputLetter
      Drives.i.Label        = "Unknown-Drive"
      Drives.i.Free         = "???"
      Drives.i.Total        = "???"
      Drives.i.Location     = "???"
      Drives.i.Filesys      = "???"
    end
    else do
      -- check for remote or local drive
      if pos(Drives.i.OutputLetter, RemoteDrives) > 0 then
        Drives.i.Location = "remote"
      else
        Drives.i.Location = "local"
      -- get the file system
      Drives.i.Filesys = SysFileSystemType(Drives.i.OutputLetter)
    end
  end
end

return


/*****************************************************************************/
/*                                                                           */
/* Procedure   : CheckInput                                                  */
/*                                                                           */
/* Description : if no input specified, get all available drives             */
/*               parse the input string and ignore all characters which are  */
/*               not alphanumeric                                            */
/*               save all alphanumeric characters to "Drives" stem           */
/*                                                                           */
/* Parameters  : EXPOSE : InputDrives  - string from the input               */
/*                        Drives.      - stem for dirves information         */
/*                                                                           */
/*****************************************************************************/

CheckInput: PROCEDURE EXPOSE InputDrives Drives.


if InputDrives = "" then
  InputDrives = SysDriveMap()

InputDrives = translate(InputDrives,' ', ':')
j = 1
do i = 1 to length(InputDrives)
  Drive = substr(InputDrives,i,1)
  if datatype(Drive,'A') = 1 then
    if datatype(Drive,'N') = 0 then do
      Drives.j.InputLetter = translate(Drive) || ":"
      Drives.0 = j
      j = j + 1
    end
end

return


/*****************************************************************************/
/*                                                                           */
/* Procedure   : BeautifyNumber                                              */
/*                                                                           */
/* Description : Insert dots to the numer for better readability like :      */
/*               2.000.000 instead of 2000000                                */
/*                                                                           */
/* Parameters  : in     : Number  - number to be beautified                  */
/*               return : beautified number                                  */
/*                                                                           */
/*****************************************************************************/
BeautifyNumber: PROCEDURE

use arg Number

do i = (length(Number)-3) to 1 by -3
  Number = insert('.', Number, i)
end

return Number


/*****************************************************************************/
/*                                                                           */
/* Procedure   : InfoDisplay                                                 */
/*                                                                           */
/* Description : Display all drive information in a well formatted form      */
/*                                                                           */
/* Parameters  : EXPOSE : Drives.  - stem which hold all drive information   */
/*               in     : DisplayType : "BOX"  - use box characters          */
/*                                      ""     - use box characters          */
/*                                      "LINE" - use '-'                     */
/*                                       and '|'                             */
/*                                                                           */
/*****************************************************************************/
InfoDisplay: PROCEDURE EXPOSE Drives.

use arg DisplayType

-- set the lines for drawing in dependency of the dislay type
if DisplayType = "BOX" | type = "" then do
  -- use box characters to display vertical and horizontal lines
  DisplayType    = "BOX"
  HorizontalLine = "旼컴컴쩡컴컴컴컴컴컴컴컫컴컴컴컴컴컴컴컴쩡컴컴컴컴컴컴컫컴컴컴컴쩡컴컴컴컴커"
  MidleLine      = "쳐컴컴탠컴컴컴컴컴컴컴컵컴컴컴컴컴컴컴컴탠컴컴컴컴컴컴컵컴컴컴컴탠컴컴컴컴캑"
  EndLine        = "읕컴컴좔컴컴컴컴컴컴컴컨컴컴컴컴컴컴컴컴좔컴컴컴컴컴컴컨컴컴컴컴좔컴컴컴컴켸"
  Vb = "�" -- verical bar character
end
else do
  -- use '-' and '|' characters to draw vertical and horizontal lines
  HorizontalLine = copies("-", 76)
  Vb = "|"
end

-- display and format the drive information
say HorizontalLine

say Vb || CENTER('Drive' ,5)     ||,
    Vb || CENTER('Free' ,16)     ||,
    Vb || CENTER('Total',16)     ||,
    Vb || CENTER('Label',14)     ||,
    Vb || CENTER('Location',8)   ||,
    Vb || CENTER('Filesystem',10)||,
    Vb

if DisplayType = "BOX" then HorizontalLine = MidleLine

say HorizontalLine

do i=1 to Drives.0
  say Vb || CENTER(Drives.i.OutputLetter,5)||,
      Vb || right(Drives.i.Free,15)          ,
      Vb || RIGHT(Drives.i.Total,15)         ,
      Vb    LEFT(Drives.i.Label,13)        ||,
      Vb    LEFT(Drives.i.Location,7)      ||,
      Vb    LEFT(Drives.i.Filesys,9)       ||,
      Vb
end

if DisplayType = "BOX" then HorizontalLine = EndLine

say HorizontalLine

-- display additional information
RamDrives = SysDriveMap(,'RAMDISK')
if RamDrives = "" then RamDrives = "No RAM Drives"

say ""
say "Operating System Version     :  "SysVersion()
say "Bootdrive                    :  "SysBootDrive()
say "Used drives                  :  "SysDriveMap(,'USED')
say "Local drives                 :  "SysDriveMap(,'LOCAL')
say "RAM drives                   :  "RamDrives
say "Remote drives                :  "SysDriveMap(,'REMOTE')
say "Drive letters not used       :  "SysDriveMap(,'FREE')

return

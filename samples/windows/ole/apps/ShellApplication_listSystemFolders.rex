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
 ShellApplication_listSystemFolders.rex: using OLE (object linking and embedding) with ooRexx

 Links:  <https://docs.microsoft.com/en-us/windows/win32/shell/shell>
         <https://docs.microsoft.com/en-us/windows/win32/api/shldisp/ne-shldisp-shellspecialfolderconstants>

 Using OLE and Windows' 'Shell.Application' query all Windows system
 folders.
***********************************************************************/

-- "ShellSpecialFolderConstants" (shldisp.h), cf. (2022-06-23) <https://docs.microsoft.com/en-us/windows/win32/api/shldisp/ne-shldisp-shellspecialfolderconstants?redirectedfrom=MSDN>
-- an array of name/folderID arrays
arr = .array~of(                      -
       ("ALTSTARTUP      " , "1d"x ), -
       ("APPDATA         " , "1a"x ), -
       ("BITBUCKET       " , "0a"x ), -
       ("COMMONALTSTARTUP" , "1e"x ), -
       ("COMMONAPPDATA   " , "23"x ), -
       ("COMMONDESKTOPDIR" , "19"x ), -
       ("COMMONFAVORITES " , "1f"x ), -
       ("COMMONPROGRAMS  " , "17"x ), -
       ("COMMONSTARTMENU " , "16"x ), -
       ("COMMONSTARTUP   " , "18"x ), -
       ("CONTROLS        " , "03"x ), -
       ("COOKIES         " , "21"x ), -
       ("DESKTOP         " , "00"x ), -
       ("DESKTOPDIRECTORY" , "10"x ), -
       ("DRIVES          " , "11"x ), -
       ("FAVORITES       " , "06"x ), -
       ("FONTS           " , "14"x ), -
       ("HISTORY         " , "22"x ), -
       ("INTERNETCACHE   " , "20"x ), -
       ("LOCALAPPDATA    " , "1c"x ), -
       ("MYPICTURES      " , "27"x ), -
       ("NETHOOD         " , "13"x ), -
       ("NETWORK         " , "12"x ), -
       ("PERSONAL        " , "05"x ), -
       ("PRINTERS        " , "04"x ), -
       ("PRINTHOOD       " , "1b"x ), -
       ("PROFILE         " , "28"x ), -
       ("PROGRAMFILES    " , "26"x ), -
       ("PROGRAMFILESx86 " , "30"x ), -
       ("PROGRAMS        " , "02"x ), -
       ("RECENT          " , "08"x ), -
       ("SENDTO          " , "09"x ), -
       ("STARTMENU       " , "0b"x ), -
       ("STARTUP         " , "07"x ), -
       ("SYSTEM          " , "25"x ), -
       ("SYSTEMx86       " , "29"x ), -
       ("TEMPLATES       " , "15"x ), -
       ("WINDOWS         " , "24"x )  -
      )

objShell = .oleObject~new('Shell.Application')
say "Windows system folders on this computer:"
do counter c obj over arr  -- iterate over array
   -- query path of special folder in hand
   name    =obj[1]
   folderId=obj[2]
   val=objShell~nameSpace(folderId~c2d)~self~path
   say c~right(2)":" name "("""folderId~c2x"""x)="val
end

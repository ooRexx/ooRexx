/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007 Rexx Language Association. All rights reserved.         */
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

/* fileNameDialog_demo.rex */

/*
Purpose.:   Demonstrate possible ways to use FileNameDialog
Who.....:   Lee Peedin with input from
                Mark Miesfeld
When....:   August 13, 2007
*/

-- Define a path most likely to be common to anyone using this demo - change as necessary
    ooRexxHome = value("REXX_HOME", , 'ENVIRONMENT' )
    if ooRexxHome~length == 0 then
        path = 'C:\Program Files\ooRexx\'
    else
        path = ooRexxHome || '\'

-- Define a couple of variables to use in the code
    delimiter = '0'x

-- Remind the user of what FileNameDialog actually does
    msg = 'REMEMBER, FileNameDialog does NOT actually open or save your specified file,'||.endOfLine||-
          'it simply provides a dialog that will return the file path and name!'
    call infoDialog msg

-- Provide a menu of different examples - use the built in SingleSelection dialog
    preselect = 1
    do until op = ''
        option.1 = '(Open) Select any file in a pre-determined folder'
        option.2 = '(Open) Show only *.cls files in a pre-determined folder'
        option.3 = '(Open) Allow either a *.cls file or a *.dll file to be selected'
        option.4 = '(Open) Allow *.cls, *.dll, or *.* to be selected'
        option.5 = '(Open) Allow multiple *.cls files to be selected'
        option.6 = '(Save) Create a save dialog where the appended extension is the default (.txt)'
        option.7 = '(Save) Create a save dialog where the default extension is changed (.cls)'

        max = 7
        ssdlg = .SingleSelection~new('Select A Demonstration','FileNameDialog Demonstration',option.,preselect,,max)
        op = ssdlg~execute
        if op \= '' then
            do
                preselect = op + 1
                if preselect > max then preselect = 1
                call ('OPTION'op)
            end
    end

    -- We return an unique code to indicate that we did execute and are exiting
    -- normally here.  37 is my lucky prime number for August 2007.
return 37

Option1:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'All Files (*.*)'delimiter'*.*'delimiter
    loadorsave    = ''                      -- Load is the default
    title         = ''                      -- See documentation for default
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder

    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Select A File'
    else
        call infoDialog 'You Selected' a_file
return
----------------------------------------------------------------------------------------------------------------
Option2:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'Class Files (*.cls)'delimiter'*.cls'delimiter
    loadorsave    = 'LOAD'                  -- Load is the default
    title         = 'FileNameDialog-Option2'-- Define our own title
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder

    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Select A File'
    else
        call infoDialog 'You Selected' a_file
return
----------------------------------------------------------------------------------------------------------------
Option3:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'Class Files (*.cls)'delimiter'*.cls'delimiter||-
                    'DLL Files (*.dll)'delimiter'*.dll'delimiter
    loadorsave    = 'LOAD'                  -- Load is the default
    title         = 'FileNameDialog-Option3'-- Define our own title
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder
    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Select A File'
    else
        call infoDialog 'You Selected' a_file
return
----------------------------------------------------------------------------------------------------------------
Option4:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'Class Files (*.cls)'delimiter'*.cls'delimiter||-
                    'DLL Files (*.dll)'delimiter'*.dll'delimiter||-
                    'All Files (*.*)'delimiter'*.*'delimiter
    loadorsave    = 'LOAD'                  -- Load is the default
    title         = 'FileNameDialog-Option4'-- Define our own title
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder
    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Select A File'
    else
        call infoDialog 'You Selected' a_file
return
----------------------------------------------------------------------------------------------------------------
Option5:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'Class Files (*.cls)'delimiter'*.cls'delimiter'All Files (*.*)'delimiter'*.*'delimiter
    loadorsave    = 'LOAD'                  -- Load is the default
    title         = 'FileNameDialog-Option5'-- Define our own title
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = 'MULTI'                 -- Specify the ability to select multiple files
    sepChar       = '^'                     -- Define the charact to separate the multiple files
    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Select A File'
    else
        do
            if a_file~pos('^') == 0 then do
                msg = 'You Selected one file:'.endOfLine
                msg ||= '  ' a_file
            end
            else do
                msg = 'You Selected multiple files.'.endOfLine~copies(2)

                parse var a_file dir'^'a_file
                if dir~length < 30 then msg ||= 'The files are in the directory:' dir
                else msg ||= 'The files are in the directory:'.endOfLine || '  ' dir
                msg ||= .endOfLine~copies(2)

                do until a_file = ''
                    parse var a_file file_a'^'a_file
                    msg ||= '  ' file_a||.endOfLine
                end
            end
            call infoDialog msg
        end
return
----------------------------------------------------------------------------------------------------------------
Option6:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'All Files (*.*)'delimiter'*.*'delimiter
    loadorsave    = 'SAVE'                  -- Load is the default
    title         = 'FileNameDialog-Option6'-- Define our own title
    defExtension  = ''                      -- don't need this in this example - just a place holder
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder
    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Provide A Save Name'
    else
        call infoDialog 'You File Will Be Saved As' a_file
return
----------------------------------------------------------------------------------------------------------------
Option7:
    selfile       = path
    parent        = ''                      -- don't need this in this example - just a place holder
    filemask      = 'Class Files (*.cls)'delimiter'*.cls'delimiter'All Files (*.*)'delimiter'*.*'delimiter
    loadorsave    = 'SAVE'                  -- Load is the default
    title         = 'FileNameDialog-Option7'-- Define our own title
    defExtension  = 'cls'                   -- Define an extension to append to the user's input
    multiSelect   = ''                      -- don't need this in this example - just a place holder
    sepChar       = ''                      -- don't need this in this example - just a place holder
    a_file = FileNameDialog(selfile,parent,filemask,loadorsave,title,defExtension,multiSelect,sepChar)
    if a_file = 0 then
        call errorDialog 'You Did Not Provide A Save Name'
    else
        call infoDialog 'You File Will Be Saved As' a_file
return
----------------------------------------------------------------------------------------------------------------
::requires 'oodplain.cls'


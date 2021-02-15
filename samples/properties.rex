#!@OOREXX_SHEBANG_PROGRAM@
/* properties.rex */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007 Rexx Language Association. All rights reserved.         */
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

    say 'Going to execute the properties demo and display the results'
    .stdout~charout('Clear the screen first? (y/n) ')
    parse lower pull ans
    say
    if ans~left(1) == 'y' then do
      call SysCls
    end

-- Determine the OS
    .local~isWindows = .false
    parse upper source os .
    if os~abbrev("WIN") then .local~isWindows = .true

-- Create a default path for any OS / ooRexx Installation, along with a different
-- data path to use in the demo.
    default_path = value("REXX_HOME", , 'ENVIRONMENT' )
    if .isWindows then
        do
            if default_path == "" then
                default_path = "C:\Program Files\ooRexx"
            if default_path~left(1) \== '\' then default_path = default_path'\'
        end
    else
        do
            if default_path == "" then
                default_path = "/opt/ooRexx"
            if default_path~left(1) \== '/' then default_path = default_path'/'
        end

    data_path = default_path || 'data'

/*
In this demonstration we will expect to find 3 "properties" to use within our code
1) data_path - use its "string" value IF it is set, if not use the default_path
2) start_dow - use its "whole number" value IF it is set, if not use a default value of 1
    Sun = 1
    Mon = 2
    Tue = 3
    ...
    Sat = 7
3) save_on_exit - use its "logical value" IF it is set, if not use logical .false
*/

-- Define the name of our "properties" file
    my_prop_file = getPropertiesFileName('initdata.txt')

/*
Note: This first section of code is only applicable to this demonstration.  It will delete our properties file
      if it exists.  This code should NOT be included in a live application.
*/
    p_stream = .stream~new(my_prop_file)
    if p_stream~query('exists') \= '' then
        do
            rv = SysFileDelete(my_prop_file)
            if rv = 0 then
                say 'The existing properties file was deleted.'
            else
                say 'Error deleting the existing properties file.  RC:' rv
        end
    else
        say 'An existing properties file did not exist.'
    say
-- End of "demostration only" code

-- At this point there should NOT be an existing file with our property file's name.
-- Load the file's contents in our properties collection (remember, it doesn't even exist now)
    props = .properties~load(my_prop_file)

-- Load each of our properties if they exist, if they do not exists, then use a default
    .local~d_p   = props~getProperty('data_path',default_path)
    .local~s_d   = props~getWhole('start_dow',1)
    .local~s_o_e = props~getLogical('save_on_exit',0)

-- We should see that each property is set to its default value
    say "All properties are set to their default value since"
    say "they didn't exist on startup."
    say 'd_p   =' .d_p
    say 's_d   =' .s_d
    say 's_o_e =' .s_o_e
    say

-- Set the values of each property to a value different than the default
    props~setProperty('data_path',data_path)
    props~setWhole('start_dow',2)
    props~setLogical('save_on_exit',.true)
-- Another way to add an item to our collection
    props~put(value("REXX_HOME",,'ENVIRONMENT'),'ooRexxHome')

-- Load each of our properties again, note that the optional 'default' arg is
-- different than the values used above to set the properties.  Since in each
-- case below, the property already has a set value, the default value will be
-- ignored.
    .local~d_p   = props~getProperty('data_path','c:\program files\oorexx\')
    .local~s_d   = props~getWhole('start_dow',1)
    .local~s_o_e = props~getLogical('save_on_exit',0)
    .local~r_h   = props~getProperty('ooRexxHome','')

-- We should see that each property is set to its new value even though we have NOT yet saved the values
    say "All properties now have a new value."
    say 'd_p   =' .d_p
    say 's_d   =' .s_d
    say 's_o_e =' .s_o_e
    say 'r_h   =' .r_h
    say

-- Test the value of s_o_e - if it's .true then save our values
    if .s_o_e then
        props~save(my_prop_file)

    say 'The properties file should now exist.'
    say '  File:' my_prop_file

return 0

/**
 * On Windows the normal user can not write to the directory the samples are
 * installed in.  So, if it is Windows, we write to the user's documents
 * directory.
 *
 * This routine could be expanded to do a similar thing on other operateing
 * systems.
 *
 */
::routine getPropertiesFileName
    use strict arg fileName

    -- Insert code for other operating systems here, if needed.
    select
        when .isWindows then do
          -- Find this user's Documents directory.

          shell = .oleObject~new('Shell.Application')
          folderConstant = '5'~x2d()
          folderObj = shell~nameSpace(folderConstant)
          documentsFolder = folderObj~self~path
          if documentsFolder~right(1) \== '\' then documentsFolder = documentsFolder'\'

          fileName = documentsFolder || fileName
        end

        otherwise
            nop
    end

return fileName

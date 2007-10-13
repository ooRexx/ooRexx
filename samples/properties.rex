#!/usr/bin/rexx
/* properties.rex */
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

    call SysCls

-- Determine the OS
    .local~isWindows = .false
    parse upper source os .
    if os~abbrev("WIN") then .local~isWindows = .true

-- Create a default path for any OS / ooRexx Installation
    default_path = value("REXX_HOME", , 'ENVIRONMENT' )
    if .isWindows then
        do
            if default_path == "" then
                default_path = "C:\Program Files\ooRexx"
            default_path = default_path'\'
        end
    else
        do
            if default_path == "" then
                default_path = "/opt/ooRexx"
            default_path = default_path'/'
        end

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

/*
Note: This first section of code is only applicable to this demonstration.  It will delete our properties file
      if it exists.  This code should NOT be included in a live application.
*/
    p_stream = .stream~new('initdata.txt')
    if p_stream~query('exists') \= '' then
        do
            rv = SysFileDelete('initdata.txt')
            if rv = 0 then
                say 'Existing Properties File Deleted'
            else
                say 'Error In Deleting Existing Properties File.' rv
        end
    else
        say 'Properties File Did Not Exist'
    say
-- End of "demostration only" code

-- At this point there should NOT be a file in the current folder named 'initdata.txt'
-- Define the name of our "properties" file
    my_prop_file = 'initdata.txt'

-- Load the file's contents in our properties collection (remember, it doesn't even exist now)
    props = .properties~load(my_prop_file)

-- Load each of our properties if they exist, if they do not exists, then use a default
    .local~d_p   = props~getProperty('data_path',default_path)
    .local~s_d   = props~getWhole('start_dow',1)
    .local~s_o_e = props~getLogical('save_on_exit',0)

-- We should see that each property is set to its default value
    say "All Properties Are Set To Their Default Value Since"
    say "They Didn't Exist On Startup"
    say 'd_p   =' .d_p
    say 's_d   =' .s_d
    say 's_o_e =' .s_o_e
    say

-- Set the values of each property
    props~setProperty('data_path',default_path)
    props~setWhole('start_dow',2)
    props~setLogical('save_on_exit',.true)
-- Another way to add an item to our collection
    props~put(value("REXX_HOME",,'ENVIRONMENT'),'ooRexxHome')

-- Load each of our properties again
    .local~d_p   = props~getProperty('data_path','c:\program files\oorexx\')
    .local~s_d   = props~getWhole('start_dow',1)
    .local~s_o_e = props~getLogical('save_on_exit',1)
    .local~r_h   = props~getProperty('ooRexxHome','')

-- We should see that each property is set to its new value even though we have NOT yet saved the values
    say "All Properties Now Have A New Value"
    say 'd_p   =' .d_p
    say 's_d   =' .s_d
    say 's_o_e =' .s_o_e
    say 'r_h   =' .r_h
    say

-- Test the value of s_o_e - if it's .true then save our values
    if .s_o_e then
        props~save(my_prop_file)

    say 'There should now be a file named "initdata.txt" in the directory'
    say 'this program was executed from.'


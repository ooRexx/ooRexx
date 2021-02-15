#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/***************************************************************************/
/*                                                                         */
/*  makestring.rex      Open Object Rexx samples                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/* Description:                                                            */
/* A sample to demonstrate a combination of makearray method of the stream */
/* and makestring method of the array class.                               */
/*                                                                         */
/* The sample creates a temporary file with text, reads in all data using  */
/* the makearray method of the stream class, removes all comments of the   */
/* file and writes back data to a temporary file using the makestring      */
/* method of the array class. After displaying the content of the input    */
/* and output file, the temporary files are removed from the system.       */
/*                                                                         */
/*  Input:              None                                               */
/*                                                                         */
/*  Output:             generated Input and Output file                    */
/*                                                                         */
/***************************************************************************/


--creating temporary input and output file

file_name_in = getTempFileName('tst_input.???')
file_name_out = getTempFileName('tst_output.???')
file_in = .stream~new(file_name_in)
file_out = .stream~new(file_name_out)

if file_in~open \= "READY:" then do
   say "Unable to create test file : " file_name_in
   exit
end

if file_out~open \= "READY:" then do
   say "Unable to create test file : " file_name_out
   exit
end

--write data containing comments into the input file

file_in~lineout('/*This is a comment line*/')
file_in~lineout('This i/**/s a line containing one comment')
file_in~lineout('/**/ /**/')
file_in~lineout('no/**/ /*This i*/com/*comment line*//**//**/ment/**//* asdf*//*sadfasdf*/')
file_in~lineout('/*This is a comment line*/')
file_in~lineout('/*This is a comment line*/')
file_in~lineout('/*This is a comment line*/')
file_in~lineout('This is a line containing no comments')
file_in~lineout('This is a line containing no comments')
file_in~lineout('This is a line /*containing */ one comment')
file_in~lineout('This is a line containing no comments')
file_in~lineout('/*This is a comment that spread several lines')
file_in~lineout('This is data within a comment')
file_in~lineout(' ')
file_in~lineout('This is the end of the comment*/')
file_in~lineout('This /* is a line*/ containing /* two */ comments')
file_in~lineout(' ')
file_in~lineout('This is a line containing no comments')
file_in~close

--read all lines of the input file into an array using makearray method

arr_in = file_in~makearray
arr_out = .array~new

--regular expression template to search for '/*' and '*/'

re_in = .RegularExpression~new("\/\*")
re_out = .RegularExpression~new("\*\/")
in_out_flag = in
run_var_in = 1
run_var_out= 1

--loop through input file, search for templates, remove comments and
--write back data

do forever
   if run_var_in > arr_in~items then                       -- loop until all lines are processed
      leave
   else do
      if in_out_flag = in then                             -- comment flag = in
      do
         comment_pos = re_in~pos(arr_in[run_var_in])       -- search for '/*' in line
         if comment_pos > 0 then                           -- do i have a hit?
         do                                                -- yep, then copy the part before the '/*'
           if comment_pos > 1 then                         -- no copy if comment is at first position
           do
              if arr_out[run_var_out] = .nil then          -- is the output array item still empty
              do                                           -- yep
                arr_out[run_var_out] =  arr_in[run_var_in]~substr(1, comment_pos-1) -- copy the part before the hit
              end
              else do                                      -- no, then concatinate at the end
                arr_out[run_var_out] = arr_out[run_var_out] || arr_in[run_var_in]~substr(1, comment_pos - 1)
              end
           end                                             -- make new positioning in the input line (after '/*')
           arr_in[run_var_in] =  arr_in[run_var_in]~substr(re_in~position + 1,,
                                 length(arr_in[run_var_in]) - (re_in~position))
           in_out_flag = out                               -- now i search for '*/'. Nesting comment levels
                                                           -- are not treated in this sample
         end
         else do                                           -- there is no hit in line
           if arr_in[run_var_in]~length > 0 then           -- if line is not empty
           do
             if arr_out[run_var_out] = .nil then           -- if output array item still empty (see above)
                arr_out[run_var_out] = arr_in[run_var_in]  -- copy the whole item
             else                                          -- if output array not empty, concatenate at the end
                arr_out[run_var_out] = arr_out[run_var_out] || arr_in[run_var_in]
           end
           run_var_in = run_var_in + 1                     -- increase input array item in any case
           if arr_out[run_var_out] \= .nil then          -- we are at the end of the input array item, if
           do                                            -- output array item not empty, increase counter
              run_var_out = run_var_out + 1
           end
         end
      end                                                  -- end comment flag = in
      else do                                              -- looking for comment flag = out
         comment_pos = re_out~pos(arr_in[run_var_in])      -- search for a hit
         if comment_pos > 0 then
         do                                                -- have a hit
                                                           -- skip comment
           arr_in[run_var_in] = arr_in[run_var_in]~substr(re_out~position + 1,,
                                length(arr_in[run_var_in]) - re_out~position)
           in_out_flag = in                                -- now looking for comment flag = in again
         end
         else do                                           -- no hit
          run_var_in = run_var_in + 1                      -- search in next input array item
         end
      end
   end
end

-- make a string out of the array and write string to output file
if arr_out~items > 0 then
  file_out~lineout(arr_out~makestring)

file_out~close
file_in~close

-- show results
say 'going to display the results'
.stdout~charout('clear the screen? (y/n) ')
parse lower pull ans
say
if ans~left(1) == 'y' then do
  call SysCls
end

say '***************** content of input file: ' file_name_in  '**************'
file_in~open
do while file_in~lines > 0
   say file_in~linein
end
file_in~close
say '*****************        end of input file                **************'
say '***************** content of output file: ' file_name_out'**************'
file_out~open
do while file_out~lines > 0
   say file_out~linein
end
file_out~close
say '*****************        end of output file               **************'

-- do the cleanup

say 'press enter to continue'
pull
say 'now removing input file'

call SysFileDelete file_name_in

say 'now removing output file'

call SysFileDelete file_name_out

return

::requires "rxregexp.cls"

/**
 * Typically, the user can not write to the directory the samples are installed
 * in.  So this function returns the name of a writeable temporary file.
 */
::routine getTempFileName
  use strict arg template

  return SysTempFileName(.File~new(template, .File~temporaryPath)~string)

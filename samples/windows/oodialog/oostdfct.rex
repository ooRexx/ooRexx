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
/*--------------------------------------------------------------------------*/
/*                                                                          */
/* OODialog\Samples\oostdfct.rex   Standard Dialog Functions Demo           */
/*                                                                          */
/*--------------------------------------------------------------------------*/

commonTitle = "Standard Functions Demo"

say
say 'Starting standard dialog function demonstration...'

x = TimedMessage('Hey please read this', 'Message', 2000)

say 'Input=' Inputbox('value', commonTitle)
say 'Input=' Inputbox('Please enter a value', commonTitle)
say 'Input=' Inputbox('Please enter a value to be honest with you', commonTitle, 'aa')
say 'Input=' Inputbox('Please enter...', commonTitle, 'bb', 200)

say 'Integer=' Integerbox('Numeric',commonTitle)
say 'Integer=' Integerbox('Please enter a numeric value now ...', commonTitle)

say 'Password=' Passwordbox('Please enter a password now and quick', commonTitle)
say 'Password=' Passwordbox('Password', commonTitle)

ar = MultiInputBox('Enter address', commonTitle,                             -
                   .array~of("&First name","Last &name now","&State"),       -
                   .array~of("Ueli","Wahli",''))
if ar \= .NIL then do v over ar
                       say 'Multi-input[]=' v
                   end
ar = MultiInputBox('Enter address', commonTitle,                             -
                   .array~of("&First name", "Last &name now", "&State"),     -
                   .array~of("Ueli", "Wahli", ''), 100)
if ar \= .NIL then do v over ar
                       say 'Multi-input[]=' v
                   end

say 'ListChoice=' ListChoice('Select', commonTitle,                                 -
                             .array~of("Monday", "Tuesday", "Wednesday", "Tursday", -
                                       "Friday", "Saturday", "Sunday"))
say 'ListChoice=' ListChoice('Select a line', commonTitle,                          -
                             .array~of("Monday", "Tuesday", "Wednesday", "Tursday", -
                                       "Friday", "Saturday", "Sunday"), 100)

ar = MultiListChoice('Select a few lines', commonTitle,                             -
                     .array~of("Monday","Tuesday","Wednesday","Tursday",            -
                               "Friday","Saturday","Sunday"))
if ar \= .nil then do v over ar
                      say 'Multi-list[]=' v
                   end

chks = .array~new
chks[3] = .true
chks[4] = .true
ar = CheckList('Select a few', commonTitle,                                  -
               .array~of("Jan", "Feb", "Mar", "April in Paris is nice",      -
                         "May", "Jun", "Jul", "Aug",                         -
                         "Sep", "Oct", "Nov", "Dec"),                        -
                         chks, , 4)
if ar \= .nil then do i=1 to ar~items
                      if ar[i]=1 then say 'CheckList['i']'
                   end

say 'Single=' SingleSelection('Select one', commonTitle,                            -
                              .array~of("Jan", "Feb", "Mar", "Apr", "May", "Jun",  -
                                        "July is the hottest month again", "Aug",  -
                                        "Sep","Oct","Nov","Dec"), 7)

say 'Single=' SingleSelection('Select one again', commonTitle,                     -
                              .array~of("Jan", "Feb", "Mar", "Apr", "May", "Jun",  -
                                        "July is the hottest month again", "Aug",  -
                                        "Sep","Oct","Nov","Dec"), 3, , 6)

say 'End of standard dialog function demonstration...'

::requires 'OODPLAIN.cls'

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/**********************************************************************/
/*                                                                    */
/* MSInternetExplorer_navigate.rex: OLE Automation with ooRexx        */
/*                                                                    */
/* Start Internet Explorer and show the IBM homepage. After 10 seconds*/
/* the RexxLA events page will be displayed for 10 seconds.           */
/*                                                                    */
/**********************************************************************/

/* create an object for IE */
myIE = .OLEObject~New("InternetExplorer.Application")

myIE~Width  = 1000
myIE~Height = 800
Say "current dimensions of Internet Explorer (IE) are:" myIE~Width "by" myIE~Height

/* set new dimensions and browse IBM homepage */
Say "changing IE dimensions ..."
myIE~Width  = 1280
myIE~Height = 1024
Say "IE dimensions changed to:" myIE~Width "by" myIE~Height
Say "making IE visible ..."
myIE~Visible = .True    -- now show the window
say "navigating to:" "https://www.ibm.com"
myIE~Navigate("https://www.ibm.com")

/* wait for 10 seconds */
say "now sleeping for 10 seconds ..."
Call SysSleep 10

/* browse RexxLA event page */
say "navigating to:" "https://www.rexxla.org/events"
myIE~Navigate("https://www.rexxla.org/events")

/* wait for 10 seconds */
say "now sleeping for 10 seconds before quitting ..."
Call SysSleep 10
myIE~quit

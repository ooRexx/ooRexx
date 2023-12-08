/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2002-2022 Rony G. Flatscher. All rights reserved.            */
/* Copyright (c) 2023      Rexx Language Association. All rights reserved.    */
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

/***********************************************************************
	program: createOleInfo.rex
	type:    Object Rexx for Windows
	purpose: Query the OLE/ActiveX automation interface, create HTML-renderings,
            optionally display HTML-renderings with Internet Explorer

	needs:   oleinfo2html.frm

   usage:   can be used as a function or as a command

    - can be called from the command line as well:

      rexx createOleInfo.rex app_name [mode [display]]

         where:
         app_name   ... OLE/ActiveX: a string denominating either PROGID or CLSID
         mode       ... 0|1 full=default (0) or compact (1) listing
         display    ... 0|1 no display (0) or display=default (1) with Internet Explorer

            example (command-line): rexx createOleInfo InternetExplorer.Application 1 1

    - called as an external function:

       res=createOleInfo(app_name | {ole_object [, [header]} [, [mode] [, [display]]]] )
       ... invocation as a function, e.g. from *any* running Object Rexx program,
                returns .true, if OLEObject could get created|interrogated, .false else
         where:
            app_name   ... OLE/ActiveX: a string denominating either PROGID or CLSID
         or
            ole_object ... OLE/ActiveX-object to interrogate

             header     ... HTML-heading, if 1st argument is an OLE-object
             mode       ... 0|1 full (0) or compact=default (1) listing
             display    ... 0|1 no display (0) or display=default (1) with Internet Explorer

             example:   ole=.OleObject~new("InternetExplorer.Application")
                        success=createOleInfo(ole, "Interfaces for the Internet Explorer")

***********************************************************************/

   if arg()=0 then -- show usage
   do
      do i=1 to 999 until pos("usage:", sourceline(i)) > 0; nop; end
      do i=i to 999 while pos("*/", sourceline(i))=0;say strip(sourceline(i));end
      exit
   end

	parse source . calltype .		-- get type of invocation

   bCommand=(calltype="COMMAND")    -- called as function ?
	if bCommand then
               do
                  parse arg app bCompact bShowInBrowser		-- called from the commandline
                  fn=sanitize(app)
               end
		   		else  -- called from another Rexx program
               do
                  use arg app, header=("Name of OLE-Object Not Supplied by Programmer!<br>created on:" date("S") time()), -
                          bCompact=.false, bShowInBrowser=.true -- called as a function
                  fn=sanitize(header)
               end

   if bCompact = "" then bCompact=.false  -- default to .false
                    else bCompact = (bCompact=.true | translate(bCompact)="TRUE")

   if bShowInBrowser = "" then
      bShowInBrowser=.true    -- default to .true
	else
      bShowInBrowser = \(bShowInBrowser=.false | translate(bShowInBrowser)="FALSE")	-- default to .true

   if app~class=.string then  -- first argument is a String (PROGID or ClSID), not an OLEObject !
   do
   	resArr=oleinfo2html(app, .nil, bCompact)	   -- create OLE object and render interrogation results
   end
   else
   do    -- 'app' is assumed to be an OleObject
      resArr=oleinfo2html(header, app, bCompact)	-- pass OLE object and render interrogation results
   end

	if resArr~items=0 then
   do
      .error~say('Problem: could not create an OLEObject for supplied ProgID/CLSID "'app'"')
      return .false	-- "app": not an OLEobject, return .false
   end

   -- create html-file, add extension ".html" to given name
   hint=""
   if bCompact then hint="reference_"

	-- fn= app || "_" || hint || date("S") || "_" || changestr(" ", translate(time(),"",":"), "") || ".html"
	fn= fn || "_" || hint || date("S") || "_" || changestr(" ", translate(time(),"",":"), "") || ".html"


		-- write received data into file, close it
	str=.stream~new(fn)
	str~~open("write replace")~~lineout("<html>")~~arrayout(resArr)~~lineout("</html>")~close

   if \bShowInBrowser then return .true -- do not run IE, hence exit

/* --- although works as of 2022-04-28, Microsoft actively deprecates its Internet explorer
       in favor of its Chromium based replacement ("Edge"), which cannot be addressed via OLE
      -- still, the InternetExplorer.Application PROGID may still be operable
		--	create an instance of Internet Explorer
	myIE=.OLEObject~new("InternetExplorer.Application")
	myIE~visible=.true				-- make it visible
	myIE~navigate2(str~qualify)	-- load freshly created html file, needs full path with filename
   return .true
-- */

   address system qu(fn)         -- let Windows open file in default browser
   return .true                  -- indicate success

::requires "oleinfo2html.frm"



/* Make sure we create a valid name for Windows, cf. <https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file>
   defining the illegal chars:
    < (less than)
    > (greater than)
    : (colon)
    " (double quote)
    / (forward slash)
    \ (backslash)
    | (vertical bar or pipe)
    ? (question mark)
    * (asterisk)
*/

::routine sanitize
  parse arg fn
  return translate(fn, "", "<>:""/\|?*", " ")~space(1)~changestr(" ","_")

/* Put quotes around filename and make sure parentheses get escaped with ^. */
::routine qu
  return '"'arg(1)~changeStr('(','^(')~changeStr(')','^)')'"'


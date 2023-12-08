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
	program: oleinfo2html.frm
	purpose: Query the OLE/ActiveX automation interface, create HTML renderings of results

	needs:	oleinfo.cls

	usage:   require this file

				oleinfo(app_name[, [oleobj] [, [bCompact] [, HTMLString]]] )

				app_name		OLE/ActiveX program name or class-id
				oleobj		OLE object, .nil or empty string
								... if .nil or empty string, an OLE object is created with app_name
				bCompact    ... .true=compact rendering, .false=full rendering including constants
            HTMLString  ... if given, gets inserted right before tables begin

	returns: array with two elements: string of <head> and string of <body>

***********************************************************************/

nl="0d0a"x
parse source . . thisPgm
thisLocation=filespec('location',thisPgm)

   -- create a stylesheet link with a fully quailified name in addition to one without a path

-- oleinfo.properties
propsFile="oleinfo.properties"
if \sysFileExists(propsFile) then propsFile=thisLocation||propsFile

if sysFileExists(propsFile) then
do
   props=.Properties~load(propsFile)
   stylesheet     =props~getProperty("cssFileName", stylesheet)~strip
   bIncorporateCss=props~getLogical("incorporateCSS", .true)
end
else  -- no cssFile found, just refer to it, do not attempt to incorporate
do
   stylesheet     ="oleinfo.css"
   bIncorporateCss=.false
end

if bIncorporateCss then    -- does stylesheet exist?
do
   if \sysFileExists(stylesheet) then
   do
      tmpStyleSheet=thisLocation||stylesheet
      if sysFileExists(tmpStyleSheet) then
         styleSheet=tmpStyleSheet
      else     -- does not exist, do not incorporate CSS, leave original name intact
         bIncorporateCss=.false
   end
end

tmpLocalCss=""
if bIncorporateCss then -- if .true get css-definitions and copy them into the head
do
   s=.stream~new(stylesheet)~~open("read")
   tmp="<style>" nl s~arrayIn nl "</style>" nl -- incorporate css definitions
   s~close
end
else
do
   tmp=stream(stylesheet, "c", "query exists")

   if tmp="" then    -- hmm, not found, maybe wer are not in our home directory, try it with that
   do
      errMsg="Problem, cannot locate stylesheet:" stylesheet
      if window <> "WINDOW" then call alert msg
                            else .error~say(msg)
   end

   tmpLocalCss='<link rel="stylesheet" type="text/css" href="'stylesheet'" id="rgf_css">    '
   if tmp <> "" then
   do
      tmp='              <!-- just to make sure that an absolute path exists also: -->    'nl ,
          '<link rel="stylesheet" type="text/css" href="'||changestr("\", tmp, "/")||'" id="rgf_css2">    '
   end
   else
      tmp=""
end

	-- HTML head text
.local~head.text = 									                                                ,
			'<head>                                                                         'nl ,
			'<!-- Rony G. Flatscher, Wirtschaftsuniversitaet Wien, Austria, Europe 2022     'nl ,
			'                        http://www.wu.ac.at, version 2022-05-05            --> 'nl ,
			'                                                                               'nl ,
			'<meta name="keywords" content="\\//, OLE, ActiveX, interfaces, HTML, listing,  'nl ,
			'							    COM, Rexx, Object Rexx, ooRexx">                       'nl ,
			'<title>OLE/ActiveX Automation Interfaces for "\\//" </title>                   'nl ,
			tmpLocalCss                                                                      nl ,
                                                                                          nl ,
         tmp                                                                              nl ,
			'                                                                               'nl ,
         .resources~flip.js                                                               nl ,
			'</head>                                                                        'nl ,
			'<body>                                                                         'nl ,
			'<div id="root">                                                                'nl

	-- HTML body text leadin
.local~leadin.body.text  = '<body><div id="root">                                        'nl

	-- HTML body text leadout
.local~leadout.body.text = '</div></body>                                                'nl


   -- flip-code
.local ~ flip.code =                                                                      nl ,
			'                                                                               'nl ,
  			'<script language="Object Rexx">                                                'nl ,
  			'	-- displays/hides thead, tbody, makes sure checkbox is appropriately checked 'nl ,
  			'	::routine flip public                                                        'nl ,
  			'	  use arg base_id				-- basename of the id-values needed in here       'nl ,
  			'                                                                               'nl ,
  			'	 all	= document~all				-- get collection of all available objects	  'nl ,
  			'	 th		= all~at( base_id || 1 )	-- get table-head-object                 'nl ,
  			'	 tbody	= all~at( base_id || 2 )	-- get table-body                        'nl ,
  			'	 cb		= all~at( "CB_" || base_id ) 	-- get appropriate checkbox object    'nl ,
  			'	                                                                             'nl ,
  			'	 if th~style~display="none" then                                             'nl ,
  			'	 do                                                                          'nl ,
  			'		th~style~display=""			-- display again                               'nl ,
  			'		tbody~style~display=""		-- display again                               'nl ,
  			'		cb~checked=.true			-- just make sure that checkbox is checked        'nl ,
  			'	 end                                                                         'nl ,
  			'	 else                                                                        'nl ,
  			'	 do                                                                          'nl ,
  			'		th~style~display="none"		-- display again                               'nl ,
  			'		tbody~style~display="none"	-- display again                               'nl ,
  			'		cb~checked=.false			-- just make sure that checkbox is not checked    'nl ,
  			'	 end                                                                         'nl ,
  			'	 return                                                                      'nl ,
  			'</script>                                                                      'nl


      -- query computer, domain and user-name, save info with the local environment for later referral
if .rgf.info=".RGF.INFO" then -- not set yet, query WSH for user data
do
	wn = .OLEObject~New("WScript.Network")
	.local~rgf.info = wn~userName"/"wn~userDomain"@\\"wn~computerName
end


::requires "oleinfo.cls"		-- class which queries and keeps the OLE-infos

	/* create the HTML rendered output of the ole-infos	*/
::routine oleinfo2html	public
  use arg olestring, oleobj, bCompact, htmlString

  bCompact = (bCompact=.true)	-- default to .false, i.e. more verbose output
  bBrowser = (window<>"WINDOW")	-- determine whether running under MS Internet Explorer
  if bBrowser then	-- assuming to run under WWW-browser
	  window~status='Interrogating the OLE object automation interface ...'

  if arg(2)="" | arg(2)=.nil then a=.rgf.oleinfo~new(.nil, olestring)   -- create a parsed OLEObject object
                             else a=.rgf.oleinfo~new(oleobj, olestring)

  if a~oleobject=.nil then return .array~new  -- no OLEObject available, return empty array

  ts. = a~allMethodSortedStem	-- stem with all methods sorted

--  outArray = .array~new			-- create array which contains the HTML-lines
--  aIdx     = 1						-- set array index to 1

  outMB=.mutableBuffer~new
  -- call time "r"

  call sag '<h1 class="hilite">' a~oleString '</h1>'
  call sag
  call sag 'Definitions from typelib:' '[<span class="doc hilite">'a~libname'</span>]'
  call sag 'with the brief documentation:'
  call sag '[<span class="doc hilite">'a~libdoc'</span>]'

   -- determine date of ooRexx interpreter, if newer than 20220516, then the attributes CLSID and ProgId are available
  sdate=date("s",.rexxinfo~date) -- turns ooRexx production date into a sorted date (YYYYMMDD)
  if sdate>"20220516" then       -- show attributes ProgID and CLSID
  do
     call sag '<p style="font-size: 75%">Effective ProgID: [<span class="doc hilite">'a~oleobject~progid'</span>]'
     call sag 'CLSID: [<span class="doc hilite">'a~oleobject~clsid'</span>]</p>'
  end

  call sag '<br> <span style="font-size: 50%;">(These published interfaces got retrieved using the <em>.OLEInfo</em> class from ooRexx with the exact name: <em>&quot;'.rexxinfo~name'&quot;</em>.)</span>'
  call sag

  if arg(4, "Exists") then call sag htmlString  -- insert received HTML-string

  if bCompact then call do_the_work_compact
				  else
              do
--                 call sag .flip.code      -- add flip-code for showing/hiding table bodies
                 call do_the_work
              end

  call sag '<p>'
  call sag '<div class="bottom_info">'
  call sag '<hr>'
  call sag 'Created with <span class="hilite">'
  call sag '<a href="https://sourceforge.net/projects/oorexx/">ooRexx</a> (Open Object'
  call sag '<strong><a href="https://www.RexxLA.org">Rexx)</a></strong> </span> (&quot;createOleInfo.rex&quot;)'

  call sag 'on' '<span class="hilite">' pp(date("s") time()) '</span>'
  call sag 'run by' '<span class="hilite">' pp(.rgf.info) '</span>'
  call sag '<hr>'
  call sag '</div>'

  call sag

  tmp=outMB~string

  -- broken up into two pieces: a string for <head> and one for <body>
  return .array~of( changestr( "\\//", .head.text, olestring), .leadin.body.text tmp .leadout.body.text )


	/* create the HTML text for documenting the OLE interfaces: methods, get/put properties, events, constants */
do_the_work:

  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* dump methods	*/

  call sag '<p>'

  o=a~methodDir		-- get method directory from OLEInfo object
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'methods ...'

  call sag
  call sag '<table id="meth0" border width="100%" cellpadding="3" class="main_table">'
  -- call sag '<table id="meth0" width="100%" class="main_table">'

  if o~items=0 then
  do
     tmp1=''
     tmp2='style="display:none"'
  end
  else
  do
     tmp1='checked'
     tmp2=''
  end

  call sag
  call sag '<tr><th width="10%" onclick="flip(''meth'')" language="Object Rexx">show',
               '<input type="CHECKBOX"' tmp1 'id="CB_meth">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">' a~methodDir~items  'Method[s]'  '</span>'
  call sag '<tr id="meth1"' tmp2'><th>No. <th>Name <th>Documentation, Argument[s], Return Value'
  call sag '<col align="right" class="number"> <col class="name"> <col>'
  call sag '<tbody id="meth2"' tmp2'>'
  call sag

  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'methods ...'

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)
        call sag '<tr class="' || choose(m//2, "odd", "even") || '"><td>'m '<td>' ts.i ,
		         '<td> <span class="doc">' ot~doc~string '</span>' '<br>'

      call sag
		call sag '<table>'
		if ot~params~items > 0 then
		do
			call sag '<tr> <td align="right">arg: <td>'
			call write_arguments ot~params
		end
        call sag '<tr class="return"><td align="right">returns: <td class="type">' ot~retType choose(ot~retType="VT_VOID", "( no return value )", "")
		call sag '</table>'
     end
     call sag
  end
  call sag '</tbody>'
  call sag '</table>'
  call sag

  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* readonly properties	*/
  call sag '<p>'

  o=a~getOnlyPropertyDir	-- get directory with the read-only properties from OLEinfo object
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'read-only properties ...'


  call sag
  call sag '<table id="getOnly0" border width="100%" cellpadding="3" class="main_table">'

  if o~items=0 then
  do
     tmp1=''
     tmp2='style="display:none"'
  end
  else
  do
     tmp1='checked'
     tmp2=''
  end

  call sag '<tr><th width="10%" onclick="flip(''getOnly'')" language="Object Rexx">show',
               '<input type="CHECKBOX"' tmp1 'id="CB_getOnly">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">'  a~getOnlyPropertyDir~items 'Read-only Properties' '</span>'

  call sag '<tr id="getOnly1"' tmp2'><th>No. <th>Name <th>Documentation, Return Value'
  call sag '<col align="right" class="number"> <col class="name"> <col>'
  call sag
  call sag '<tbody id="getOnly2"' tmp2'>'

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)
        -- call sag '<tr><td>'m '<td>' ts.i '<td> <span class="doc">' ot~doc~string '</span>' '<br>'

        call sag '<tr class="' || choose(m//2, "odd", "even") || '"><td>'m '<td>' ts.i ,
		         '<td> <span class="doc">' ot~doc~string '</span>' '<br>'


      call sag
		call sag '<table>'
		if ot~params~items > 0 then
		do
			call sag '<tr> <td align="right">arg: <td>'
			call write_arguments ot~params
		end
        call sag '<tr class="return"><td align="right">returns: <td class="type">' ot~retType choose(ot~retType="VT_VOID", "( no return value )", "")

		call sag '</table>'
      call sag
     end
  end
  call sag '</tbody>'
  call sag '</table>'
  call sag

  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* write-only properties	*/
  call sag '<p>'

  o=a~putOnlyPropertyDir	-- get write-only property from OLEinfo object
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'write-only properties ...'

  call sag
  call sag '<table id="putOnly0" border width="100%" cellpadding="3" class="main_table">'

  if o~items=0 then
  do
     tmp1=''
     tmp2='style="display:none"'
  end
  else
  do
     tmp1='checked'
     tmp2=''
  end

  call sag '<tr><th width="10%" onclick="flip(''putOnly'')" language="Object Rexx">show',
               '<input type="CHECKBOX"' tmp1 'id="CB_putOnly">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">'  a~putOnlyPropertyDir~items 'Write-only Properties' '</span>'

  call sag '<tr id="putOnly1"' tmp2'><th>No. <th>Name <th>Documentation, Argument[s], Return Value'
  call sag '<col align="right" class="number"> <col class="name"> <col>'
  call sag
  call sag '<tbody id="putOnly2"' tmp2'>'

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)
        -- call sag '<tr><td>'m '<td>' ts.i '<td> <span class="doc">' ot~doc~string '</span>' '<br>'

        call sag '<tr class="' || choose(m//2, "odd", "even") || '"><td>'m '<td>' ts.i ,
		         '<td> <span class="doc">' ot~doc~string '</span>' '<br>'


      call sag
		call sag '<table>'
		if ot~params~items > 0 then
		do
			call sag '<tr> <td align="right">arg: <td>'
			call write_arguments ot~params
		end
        call sag '<tr class="return"><td align="right">returns: <td class="type">' ot~retType choose(ot~retType="VT_VOID", "( no return value )", "")
		call sag '</table>'
     end
     call sag
  end
  call sag '</tbody>'
  call sag '</table>'

  call sag


  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* read/write properties	*/
  call sag '<p>'
  o=a~getAndPutPropertyDir
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'read/write properties ...'

  call sag
  call sag '<table id="getAndPut0" border width="100%" cellpadding="3" class="main_table">'

  if o~items=0 then
  do
     tmp1=''
     tmp2='style="display:none"'
  end
  else
  do
     tmp1='checked'
     tmp2=''
  end

  call sag '<tr><th width="10%" onclick="flip(''getAndPut'')" language="Object Rexx">show',
               '<input type="CHECKBOX"' tmp1 'id="CB_getAndPut">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">'  a~getAndPutPropertyDir~items 'Read/Write Properties' '</span>'

  call sag '<tr id="getAndPut1"' tmp2'><th>No. <th>Name <th>Documentation, Argument[s], Return Value'
  call sag '<col align="right" class="number"> <col class="name"> <col>'
  call sag
  call sag '<tbody id="getAndPut2"' tmp2'>'

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)
        -- call sag '<tr><td>'m '<td>' ts.i '<td> <span class="doc">' ot~doc~string'</span>'  '<br>'

        call sag '<tr class="' || choose(m//2, "odd", "even") || '"><td>'m '<td>' ts.i ,
		         '<td> <span class="doc">' ot~doc~string '</span>' '<br>'

      call sag
		call sag '<table>'
		if ot~params~items > 0 then
		do
			call sag '<tr> <td align="right">arg: <td>'
			call write_arguments ot~params
		end
        call sag '<tr class="return"><td align="right">needs/returns: <td class="type">' ot~retType choose(ot~retType="VT_VOID", "( no return value )", "")
		call sag '</table>'
      call sag
     end
  end
  call sag '</tbody>'
  call sag '</table>'
  call sag



  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* unknown properties/methods, i.e. unknown invocation type */
  call sag '<p>'
  o=a~naDir
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'unknown invocation types ...'

  call sag
  call sag '<table id="unknownMeth0" border width="100%" cellpadding="3" class="main_table">'

  if o~items=0 then
  do
     tmp1=''
     tmp2='style="display:none"'
  end
  else
  do
     tmp1='checked'
     tmp2=''
  end

  call sag '<tr><th width="10%" onclick="flip(''unknownMeth'')" language="Object Rexx">show',
               '<input type="CHECKBOX"' tmp1 'id="CB_unknownMeth">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">'  a~naDir~items 'Methods with Unknown Invocation Type Properties' '</span>'

  call sag '<tr id="unknownMeth1"' tmp2'><th>No. <th>Name <th>Documentation, Argument[s], Return Value'
  call sag '<col align="right" class="number"> <col class="name"> <col>'
  call sag
  call sag '<tbody id="unknownMeth2"' tmp2'>'

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)
        -- call sag '<tr><td>'m '<td>' ts.i '<td> <span class="doc"' ot~doc~string '</span>' '<br>'
        call sag '<tr class="' || choose(m//2, "odd", "even") || '"><td>'m '<td>' ts.i ,
		         '<td> <span class="doc">' ot~doc~string '</span>' '<br>'

        call sag 'Invocation type:' ot~invkind  "(".rgf.invKind[ot~invkind]")" '<br>'

        call sag
        call sag '<table>'

        if ot~params~items > 0 then
        do
        	call sag '<tr> <td align="right">arg: <td>'
        	call write_arguments ot~params
        end

        call sag '<tr class="return"><td align="right">returns: <td class="type">' ot~retType choose(ot~retType="VT_VOID", "( no return value )", "")
		  call sag '</table>'
        call sag '<br>'
     end
     call sag
  end
  call sag '</tbody>'
  call sag '</table>'
  call sag


  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* events */
  call sag '<p>'
  o=a~eventDir
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'events ...'

  call sag
  call sag '<table id="events0" border width="100%" cellpadding="3" class="main_table">'

  if o~items=0 then
  do
     tmp1=''
     tmp2='style="display:none"'
  end
  else
  do
     tmp1='checked'
     tmp2=''
  end

  call sag '<tr><th width="10%" onclick="flip(''events'')" language="Object Rexx">show',
               '<input type="CHECKBOX"' tmp1 'id="CB_events">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">'  a~eventDir~items 'Event(s)' '</span>'

  call sag '<tr id="events1"' tmp2'><th>No. <th>Name <th>Documentation, Argument[s], Return Value'
  call sag '<col align="right" class="number"> <col class="name"> <col>'
  call sag
  call sag '<tbody id="events2"' tmp2'>'

  ts. = a~eventSortedStem

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)
        call sag '<tr class="' || choose(m//2, "odd", "even") || '"><td>'m '<td>' ts.i ,
		         '<td> <span class="doc">' ot~doc~string '</span>' '<br>'

        if ot~params~items > 0 then
        do

           call sag
           call sag '<table>'
           call sag '<tr> <td align="right">arg: <td>'
           call write_arguments ot~params
           call sag '</table>'
        end
        else call sag '&nbsp;'
        call sag
     end
  end
  call sag "</tbody>"
  call sag '</table>'
  call sag



  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  /* constants	*/
  ts.=a~constantSortedStem

  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' ts.0 'constants ...'

  call sag '<p>'
  call sag
  call sag '<table id="constants0" border width="60%" cellpadding="3" class="main_table">'
  call sag '<tr><th width="17%" onclick="flip(''constants'')" language="Object Rexx">show',
               '<input type="CHECKBOX" id="CB_constants">'

  call sag '<th colspan="2" class="caption">' '<span class="hilite">'  ts.0 'Constant(s)' '</span>'

  tmp2='style="display:none"'
  call sag '<tr id="constants1"' tmp2'><th>No. <th>Name <th>Value'
  call sag '<col align="right" class="number"> <col> <col align="right" class="number">'
  call sag
  call sag '<tbody id="constants2"' tmp2'>'


  do i=1 to ts.0
     call sag '<tr class="' || choose(i//2, "odd", "even") || '"><td>'i '<td>' ts.i '<td>' a~oleobject~getconstant( ts.i )
  end
  call sag "</tbody>"
  call sag "</table>"
  call sag


  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  call sag

  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Done.'
  return



	/* create a compact (terse) output: methods, properties, events */
do_the_work_compact:
	/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  call sag '<p>'

  o=a~methodDir
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'methods ...'

  call sag
  call sag '<table id="meth0" cellspacing="0" width="100%" class="compact_table">'

  call sag '   <th colspan="4" class="caption">' '<span class="hilite">' a~methodDir~items  'Method[s]'  '</span>'

  call sag '<tr id="meth1"><th align="right">No. <th align="right">  <th align="left">Name'
  call sag '   <th align="left">Argument[s], Documentation'
  call sag '   <col class="number"> <col class="type"> <col class="name"> <col>'
  call sag
  call sag '   <tbody id="meth2">'



  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)

		p_items=ot~params~items	-- get number of arguments/parameters

	 	evenOdd=choose( m//2, "odd", "even")

      call sag '<tr class="' || evenOdd || '">'
		call sag '    <td class="number">' m
		retType=ot~retType
		call sag '    <td align="right" valign="top"> <span class="type">'choose(retType="VT_VOID", "", retType)'</span>'
		call sag '    <td               valign="top"> <span class="name">' ts.i'</span>'
		-- call sag '    <td>'
		call sag '    <td' choose(p_items=0, 'class="indent"', "") ||'>'
		if p_items > 0 then
		do
			call sag '<strong>(</strong> '
			call write_arguments_compact ot~params
			call sag ' <strong>)</strong>'
		end

		if wordpos( ot~doc~string, "n/a (null)") = 0 then
		do
		   if p_items>0 then call sag '<tr class="' evenOdd '"> <td colspan="3"> <td class="indent">'
		   call sag '<span class="doc">' ot~doc~string '</span>'
		end
      call sag
     end
  end
  call sag '</tbody>'
  call sag '</table>'


	/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  call sag '<p><hr><p>'

  Prop_totals=a~getPropertyDir~items + a~putPropertyDir~items + a~naDir~items

  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' prop_totals 'properties ...'

  call sag
  call sag '<table id="prop0" cellspacing="0" width="100%" class="compact_table">'
  call sag '   <th colspan="4" class="caption">' '<span class="hilite">' prop_totals  'Property[ies]'  '</span>'

  call sag '<tr id="meth1"><th align="right">No. <th align="right"> <th align="left">Name'
  call sag '   <th align="left">Argument[s], Documentation'
  call sag '   <col class="number"> <col class="type"> <col class="name"> <col>'
  call sag
  call sag '   <tbody id="prop2">'

  -- o=a~methodDir

  m=0
  do i=1 to ts.0
  	  tmpList=.list~new
	  tmp=a~getPropertyDir~entry(ts.i)		-- a get property?
	  getName=""
	  if tmp <> .nil then
	  do
	     tmpList~insert(tmp)
		 getName=ts.i
	  end

	  tmp=a~putPropertyDir~entry(ts.i)		-- a put property?
	  if tmp <> .nil then tmpList~insert(tmp)

	  tmp=a~naDir~entry(ts.i)				-- an unknown invoked method property?
	  if tmp <> .nil then tmpList~insert(tmp)

     do ot over tmpList			-- get all entries for
        m=m+1
        -- ot=aMD~entry(ts.i)		-- get property

		p_items=ot~params~items	-- get number of arguments/parameters

	 	evenOdd=choose( m//2, "odd", "even")

      call sag '<tr class="' || evenOdd || '">'
		call sag '    <td class="number">' m
		retType=ot~retType
		call sag '    <td align="right" valign="top"> <span class="type">'choose(retType="VT_VOID", "", retType)'</span>'

			-- indicate the put-property by appending the assing operator "="
		call sag '    <td valign="top"> <span class="name">' ts.i || choose(ot~invKind=4, "=", "") '</span>'
		-- call sag '    <td>'
		call sag '    <td' choose(p_items=0, 'class="indent"', "") ||'>'
		if p_items > 0 then
		do
			call sag '<strong>(</strong> '
			call write_arguments_compact ot~params
			call sag ' <strong>)</strong>'
		end

		if wordpos( ot~doc~string, "n/a (null)") = 0 & \(ot~invKind=4 & getName=ts.i) then
		do
		   if p_items>0 then call sag '<tr class="' evenOdd '"> <td colspan="3"> <td class="indent">'
		   call sag '<span class="doc">' ot~doc~string '</span>'
		end
     end
     call sag
  end
  call sag '</tbody>'
  call sag '</table>'



  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  call sag '<p><hr><p>'

  o=a~eventDir
  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Analyzing and creating HTML-text for available' o~items 'events ...'

  call sag
  call sag '<table id="events0" cellspacing="0" width="100%" class="compact_table">'
  call sag '   <th colspan="4" class="caption">' '<span class="hilite">' o~items 'Event(s)' '<span>'
  call sag '<tr id="meth1"><th align="right">No. <th align="right">  <th align="left">Name'
  call sag '   <th align="left">Argument[s], Documentation'
  call sag '   <col valign="top" class="number"> <col class="type"> <col class="name"> <col>'
  call sag
  call sag '   <tbody id="events2">'

  ts. = a~eventSortedStem

  m=0
  do i=1 to ts.0
     if o~hasentry(ts.i) then
     do
        m=m+1
        ot=o~entry(ts.i)

		p_items=ot~params~items	-- get number of arguments/parameters

	 	evenOdd=choose( m//2, "odd", "even")

        call sag '<tr class="' || evenOdd || '">'
		call sag '    <td class="number">' m

		call sag '    <td align="right" valign="top">'
		call sag '    <td               valign="top"> <span class="name">' ts.i'</span>'
		-- call sag '    <td>'
		call sag '    <td' choose(p_items=0, 'class="indent"', "") ||'>'
		if p_items > 0 then
		do
			call sag '<strong>(</strong> '
			call write_arguments_compact ot~params
			call sag ' <strong>)</strong>'
		end

		if wordpos( ot~doc~string, "n/a (null)") = 0 then
		do
		   if p_items>0 then call sag '<tr class="' evenOdd '"> <td colspan="3"> <td class="indent">'

		   call sag '<span class="doc">' ot~doc~string '</span>'
		end
     end
     call sag
  end
  call sag "</tbody>"
  call sag '</table>'
  call sag


  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

  /* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
  call sag

  if bBrowser then	-- assuming to run under WWW-browser
     window~status='Done.'
  return



  -- write all arguments of this method, event
write_arguments: procedure expose outMB -- outArray aIdx
  use arg ot
  if ot~items=0 then return

  call sag
  call sag '<table class="arguments">'
  call sag '<col align="right" class="number"> <col class="name">' ,
                '<col align="right" class="in"> <col class="out">' ,
                '<col class="opt"> <col class="type"'

  t=ot~items
  m=0
  do o over ot
     m=m+1
     -- call sag '<tr><td> arg #' m 'of' t':' '<td>' choose(o~opt, pp(o~name), o~name) ,
     -- call sag '<tr><td>' choose(o~opt, pp(o~name), o~name)	,
     call sag '<tr><td> #' m':' '<td>' choose(o~opt, '<span class="optional">[ ' || o~name || ' ]</span>', o~name) ,
                  '<td>' choose(o~in, "in", "")     		,
                  '<td>' choose(o~out, "/out", "") 	   ,
				  '<td>' choose(o~opt, '<span class="optional">[ optional ]</span>', "") 		,
                  '<td class="type">' o~type
  end

  call sag '</table>'

  return



  -- write all arguments of this method, event
write_arguments_compact : procedure expose outMB -- outArray aIdx
  use arg ot
  if ot~items=0 then return

  t=ot~items
  m=0
  do o over ot
     m=m+1
	 tmp =     '<span class="name">'o~name'</span>'

	 tmp = tmp || '&nbsp;<span class="type"><sup>'  || o~type || "</sup></span>"

	 if o~out then
	    tmp = tmp || '&nbsp;<span class="inout">' || choose(o~in, "in/", "---") || "out</span>"


	 call sag choose( o~opt, '<span class="optional">'pp('&nbsp;'tmp'&nbsp;')"</span>" , tmp)
	 if m <> t then		-- if not last argument, add comma
	    call sag " <strong>,</strong>"
  end
  return



	-- save text in array
sag: procedure expose outMB -- outArray aIdx

  outMB~~append(arg(1))~~append("0d0a"x)
  return

	-- cheap "pretty" print
pp: procedure
  return "[" || arg(1)~string || "]"


::resource flip.js
<script>
    // 2022-04-26, rgf: replace ooRexx with Javascript code (runs on all browsers)
 	// displays/hides thead, tbody, makes sure checkbox is appropriately checked
function flip(base_id)                       // basename of the id-values we need
{
   th    = document.getElementById(base_id+1);
   tbody = document.getElementById(base_id+2);
   cb    = document.getElementById('CB_'+base_id);

   // alert("base_id=["+base_id+"]: th="+th+", tbody="+tbody+", cb="+cb+" | th.style.display='none'="+(th.style.display=='none'));
   // alert("base_id=["+base_id+"]: before: th.style.display=["+th.style.display+"], tbody.style.display=["+tbody.style.display+"], cb.checked="+cb.checked);

   if (th.style.display=='none')
   {
       th.style.display='';    // display again
       tbody.style.display=''; // display again
       cb.checked=true;        // make sure that checkbox is checked
   }
   else
   {
       th.style.display='none';    // hide
       tbody.style.display='none'; // hide
       cb.checked=false;           // sure that checkbox is not checked
   }
   // alert("base_id=["+base_id+"]: AFTER: th.style.display=["+th.style.display+"], tbody.style.display=["+tbody.style.display+"], cb.checked="+cb.checked);
}
</script>
::END

::resource flip.rex
			'                                                                               'nl ,
  			'<script language="Object Rexx">                                                'nl ,
  			'	-- displays/hides thead, tbody, makes sure checkbox is appropriately checked 'nl ,
  			'	::routine flip public                                                        'nl ,
  			'	  use arg base_id				-- basename of the id-values needed in here       'nl ,
  			'                                                                               'nl ,
  			'	 all	= document~all				-- get collection of all available objects	  'nl ,
  			'	 th		= all~at( base_id || 1 )	-- get table-head-object                 'nl ,
  			'	 tbody	= all~at( base_id || 2 )	-- get table-body                        'nl ,
  			'	 cb		= all~at( "CB_" || base_id ) 	-- get appropriate checkbox object    'nl ,
  			'	                                                                             'nl ,
  			'	 if th~style~display="none" then                                             'nl ,
  			'	 do                                                                          'nl ,
  			'		th~style~display=""			-- display again                               'nl ,
  			'		tbody~style~display=""		-- display again                               'nl ,
  			'		cb~checked=.true			-- just make sure that checkbox is checked        'nl ,
  			'	 end                                                                         'nl ,
  			'	 else                                                                        'nl ,
  			'	 do                                                                          'nl ,
  			'		th~style~display="none"		-- display again                               'nl ,
  			'		tbody~style~display="none"	-- display again                               'nl ,
  			'		cb~checked=.false			-- just make sure that checkbox is not checked    'nl ,
  			'	 end                                                                         'nl ,
  			'	 return                                                                      'nl ,
  			'</script>                                                                      'nl
::END


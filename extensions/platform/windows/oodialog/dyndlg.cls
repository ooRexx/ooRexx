/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/*                                                                            */
/* Dynamic Dialog Class                                                       */
/*                                                                            */
/******************************************************************************/

::class 'DynamicDialog' Mixinclass Object public

::method init class external "LIBRARY oodialog dyndlg_init_cls"

::attribute basePtr get private external "LIBRARY oodialog dyndlg_getBasePtr"
::attribute basePtr set private external "LIBRARY oodialog dyndlg_setBasePtr"

::attribute activePtr get private external "LIBRARY oodialog dyndlg_getActivePtr"
::attribute activePtr set private external "LIBRARY oodialog dyndlg_setActivePtr"

::attribute dialogItemCount get private external "LIBRARY oodialog dyndlg_getDialogItemCount"
::attribute dialogItemCount set private external "LIBRARY oodialog dyndlg_setDialogItemCount"

::method dynamicInit private external "LIBRARY oodialog dyndlg_dynamicInit"
::method stop private external "LIBRARY oodialog dyndlg_stop"

::method create external "LIBRARY oodialog dyndlg_create"
::method createCenter
   arga = arg(1,"A")
   newarg = .array~new(arg()+2)
   do i = arg() to 1 by -1; if arga~hasindex(i) = 1 then newarg[i+2] = arga[i]; end   /* shift arguments up by 2 */
   -- Let OS center the dialog.
   if \ newarg~hasindex(6) then newarg[6] = "CENTER"
   else newarg[6] = newarg[6] || " CENTER"
   -- When dialog has centered style, the OS will ignore the x, y co-ordinates
   newarg[1] = 0
   newarg[2] = 0
   forward message "create" Arguments (newarg)

::method startParentDialog external "LIBRARY oodialog dyndlg_startParentDialog"
::method startChildDialog external "LIBRARY oodialog dyndlg_startChildDialog"

::method itemAdd private external "LIBRARY oodialog dyndlg_itemAdd_pvt"


   /* This method is provided to be used in any subclass of 'UserDialog' */

::method defineDialog protected
   return 0


-- This method creates a dialog from a resource script file (usually with extension of '.rc'.)
-- It calls loadFrame() and loadItems() The argument 'loadOptions' can be one or more of:e
--    "CENTER"         : this will create the dialog on a centered position */
--    "CONNECTBUTTONS" :
--    "CONNECTRADIOS"
--    "CONNECTCHECKS"
::method load
   forward message "loadFrame" continue
   ret = result
   if ret == 0 then do
      forward message "loadItems" continue
      ret = result
   end
   else do
      self~stop
   end
   return ret


::method internGetRCStyle
   use arg style, kind, spec_kind
   ret = ""
   if style~wordpos("NOT WS_VISIBLE") > 0 then ret = "HIDDEN"
   else if style~wordpos("WS_VISIBLE") > 0 then ret = "VISIBLE"
   if style~wordpos("WS_DISABLED") > 0 & style~wordpos("NOT WS_DISABLED") == 0 then ret = ret || " DISABLED"
   if style~wordpos("WS_GROUP") > 0 & style~wordpos("NOT WS_GROUP") == 0 then ret = ret || " GROUP"

   if kind = "DLG" then do
      if style~wordpos("DS_CENTER") > 0 then ret = ret ||" CENTER"
      if style~wordpos("DS_SYSMODAL") > 0 then ret = ret ||" SYSTEMMODAL"
      if style~wordpos("DS_MODALFRAME") = 0 then ret = ret ||" NOTMODAL"
      if style~wordpos("WS_SYSMENU") = 0 then ret = ret ||" NOMENU"
      if style~wordpos("WS_THICKFRAME") > 0 then ret = ret ||" THICKFRAME"
      if style~wordpos("WS_MINIMIZEBOX") > 0 | style~wordpos("WS_GROUP") > 0 then ret = ret ||" MINIMIZEBOX"
      if style~wordpos("WS_MAXIMIZEBOX") > 0 | style~wordpos("WS_TABSTOP") > 0  then ret = ret ||" MAXIMIZEBOX"
      if style~wordpos("WS_OVERLAPPED") > 0 then ret = ret ||" OVERLAPPED"
   end
   else if kind = "BTN" then do
       if style~wordpos("WS_OWNERDRAW") > 0 | style~wordpos("BS_OWNERDRAW") > 0 then ret = ret || " OWNER"
       if ret~wordpos("OWNER") = 0 then if style~wordpos("BS_AUTO3STATE") > 0 then ret = ret || " 3STATE"
       if style~wordpos("BS_DEFPUSHBUTTON") > 0 then ret = ret || " DEFAULT"
       if style~wordpos("BS_LEFTTEXT") > 0 | style~wordpos("BS_RIGHTBUTTON") > 0 then ret = ret || " LEFTTEXT"
       if style~wordpos("BS_BITMAP") > 0 then ret = ret || " BITMAP"
       if style~wordpos("BS_ICON") > 0 then ret = ret || " ICON"
       if style~wordpos("BS_LEFT") > 0 then ret = ret || " LEFT"
       if style~wordpos("BS_RIGHT") > 0 then ret = ret || " RIGHT"
       if style~wordpos("BS_CENTER") > 0 then ret = ret || " HCENTER"
       if style~wordpos("BS_TOP") > 0 then ret = ret || " TOP"
       if style~wordpos("BS_BOTTOM") > 0 then ret = ret || " BOTTOM"
       if style~wordpos("BS_VCENTER") > 0 then ret = ret || " VCENTER"
       if style~wordpos("BS_PUSHLIKE") > 0 then ret = ret || " PUSHLIKE"
       if style~wordpos("BS_MULTILINE") > 0 then ret = ret || " MULTILINE"
       if style~wordpos("BS_NOTIFY") > 0 then ret = ret || " NOTIFY"
       if style~wordpos("BS_FLAT") > 0 then ret = ret || " FLAT"
       if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
       if style~wordpos("NOT WS_TABSTOP") > 0 then ret = ret || " NOTAB"
       if spec_kind == "R" then do  /* radio buttons need explicit WS_TABSTOP to be tab stops */
           if style~wordpos("WS_TABSTOP") = 0 then ret = ret || " NOTAB"
       end
   end
   else if kind = "GB" then do
       if style~wordpos("BS_LEFT") > 0 then ret = ret || " LEFT"
       if style~wordpos("BS_RIGHT") > 0 then ret = ret || " RIGHT"
       if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
       if style~wordpos("WS_TABSTOP") > 0 then ret = ret || " TAB"
   end
   else if kind = "EDIT" then do
      if style~wordpos("ES_AUTOHSCROLL") > 0 then ret = ret || " AUTOSCROLLH"
      if style~wordpos("ES_AUTOVSCROLL") > 0 then ret = ret || " AUTOSCROLLV"
      if style~wordpos("ES_PASSWORD") > 0 then ret = ret || " PASSWORD"
      if style~wordpos("ES_MULTILINE") > 0 then ret = ret || " MULTILINE"
      if style~wordpos("ES_READONLY") > 0 then ret = ret || " READONLY"
      if style~wordpos("ES_WANTRETURN") > 0 then ret = ret || " WANTRETURN"
      if style~wordpos("ES_NOHIDESEL") > 0 then ret = ret || " KEEPSELECTION"
      if style~wordpos("ES_CENTER") > 0 then ret = ret || " CENTER"
      if style~wordpos("ES_RIGHT") > 0 then ret = ret || " RIGHT"
      if style~wordpos("ES_UPPERCASE") > 0 then ret = ret || " UPPER"
      if style~wordpos("ES_LOWERCASE") > 0 then ret = ret || " LOWER"
      if style~wordpos("ES_NUMBER") > 0 then ret = ret || " NUMBER"
      if style~wordpos("ES_OEMCONVERT") > 0 then ret = ret || " OEM"
      if style~wordpos("NOT WS_TABSTOP") > 0 then ret = ret || " NOTAB"
   end
   else if kind = "CB" then do
      if style~wordpos("CBS_SIMPLE") > 0 then ret = ret || " SIMPLE"
      if style~wordpos("CBS_DROPDOWNLIST") > 0 then ret = ret || " LIST"
      if style~wordpos("CBS_SORT") > 0 then ret = ret || " SORT"
      if style~wordpos("CBS_AUTOHSCROLL") = 0 then ret = ret || " NOHSCROLL"
      if style~wordpos("CBS_NOINTEGRALHEIGHT") > 0 then ret = ret || " PARTIAL"
      if style~wordpos("WS_TABSTOP") = 0 then ret = ret || " NOTAB"
   end
   else if kind = "LB" then do
      if style~wordpos("LBS_USETABSTOPS") > 0 then ret = ret || " COLUMNS"
      if style~wordpos("LBS_STANDARD") > 0 then ret = ret|| " SORT"
      if style~wordpos("LBS_NOTIFY") > 0 then ret = ret || " NOTIFY"
      if style~wordpos("LBS_MULTIPLESEL") > 0 then ret = ret || " MULTI"
      if style~wordpos("LBS_MULTICOLUMN") > 0 then ret = ret || " MCOLUMN"
      if style~wordpos("LBS_NOINTEGRALHEIGHT") > 0 then ret = ret || " PARTIAL"
      if style~wordpos("LBS_DISABLENOSCROLL") > 0 then ret = ret || " SBALWAYS"
      if style~wordpos("LBS_WANTKEYBOARDINPUT") > 0 then ret = ret || " KEYINPUT"
      if style~wordpos("LBS_EXTENDEDSEL") > 0 then ret = ret || " EXTSEL"
      if style~wordpos("WS_TABSTOP") = 0 then ret = ret || " NOTAB"
   end
   else if kind = "ST" then do
       if style~wordpos("SS_CENTERIMAGE") > 0 then ret = ret || " CENTERIMAGE"
       if style~wordpos("SS_EDITCONTROL") > 0 then ret = ret || " EDITCONTROL"
       if style~wordpos("SS_ENDELLIPSIS") > 0 then ret = ret || " ENDELLIPSIS"
       if style~wordpos("SS_NOPREFIX") > 0 then ret = ret || " NOPREFIX"
       if style~wordpos("SS_NOTIFY") > 0 then ret = ret || " NOTIFY"
       if style~wordpos("SS_PATHELLIPSIS") > 0 then ret = ret || " PATHELLIPSIS"
       if style~wordpos("SS_RIGHTJUST") > 0 then ret = ret || " RIGHTJUST"
       if style~wordpos("SS_REALSIZECONTROL") > 0 then ret = ret || " SIZECONTROL"
       if style~wordpos("SS_REALSIZEIMAGE") > 0 then ret = ret || " SIZEIMAGE"
       if style~wordpos("SS_SUNKEN") > 0 then ret = ret || " SUNKEN"
       if style~wordpos("SS_WORDELLIPSIS") > 0 then ret = ret || " WORDELLIPSIS"

       if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
       if style~wordpos("WS_TABSTOP") > 0 then ret = ret || " TAB"
   end
   else if kind = "SB" then do
      if style~wordpos("SBS_HORZ") > 0 then ret = ret || " HORIZONTAL"
      if style~wordpos("SBS_TOPALIGN") > 0 then ret = ret || " TOPLEFT"
      if style~wordpos("SBS_BOTTOMALIGN") > 0 then ret = ret || " BOTTOMRIGHT"
      if style~wordpos("WS_TABSTOP") > 0 then ret = ret || " TAB"
      if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
   end

   if kind = "EDIT" | kind = "LB" | kind = "CB" then
      if style~wordpos("NOT WS_BORDER") > 0 then ret = ret || " NOBORDER"
   if kind = "EDIT" | kind = "CB" | kind = "LB" | kind = "DLG" then do
      if style~wordpos("WS_VSCROLL") > 0 then ret = ret || " VSCROLL"
      if style~wordpos("WS_HSCROLL") > 0 then ret = ret || " HSCROLL"
   end

   return ret


::method internAddRCButton
   use arg id, x, y, w, h, name, style, lOpts
   id = self~checkID(id)
   if id < 0 then return
   opts = self~internGetRCStyle(style, "BTN")
   self~dialogItemCount += 1
   self~activePtr = UsrAddControl("BUT", self~activePtr, id, x, y, w, h, name, opts)
   if lOpts~wordpos("CONNECTBUTTONS") > 0 then do
      methName = name~space(0)~changestr('&','')~changestr(':','')~strip
      if methName \== "" then self~ConnectButton(id, methName)
   end


::method internAddRC_RBCB
   use arg id, x, y, w, h, name, style, lOpts, kind
   opts = self~internGetRCStyle(style, "BTN", kind)
   if (kind = "R" & lOpts~wordpos("CONNECTRADIOS") > 0) | lOpts~wordpos("CONNECTCHECKS") > 0 then
   do
      self~ConnectControl(id, "ID"||name~space(0)~changestr('&','')~changestr(':',''))
      if kind = "R" then self~addRadioButton(id,, x, y, w, h, name, opts)
      else self~addCheckBox(id,, x, y, w, h, name, opts)
   end
   else do
      if kind = "R" then self~addRadioButton(id,name,x, y, w, h, name, opts)
      else self~addCheckBox(id, name, x, y, w, h, name, opts)
   end


-- This method starts the in-memory dialog template using the specified resource script file.
-- It is usually called from the load() method.  The return was not documented prior to
-- 4.1.0, but it returned 0 for success and 1 for error.  Need to preserve that, I guess,
-- although .true and .false make much more sense.
::method loadFrame protected
   use arg resfile, dialogid, options, expected
   if arg(4,"o") | expected = 0 then expected = 200
   if options~translate~wordpos("CENTER") > 0 then internopts = "CENTER "
   else internopts = ""

   file = SysSearchPath("PATH",resfile)
   f = .stream~new(file)
   op = f~open(read)
   if op \= "READY:" then do
      say "Resource file not found -->" resfile "("op")"
      return 1
   end

   found = 0; n = 0
   fl = f~lines
   do while found = 0
      do while n = 0 & fl > 0
         s = f~linein; fl = fl -1
         select
            when s~wordpos("DIALOG ") > 0 then n = s~wordpos("DIALOG ")
            when s~wordpos("DIALOGEX ") > 0 then n = s~wordpos("DIALOGEX ")
            when s~wordpos("ICON") = 2 then do
               -- ICON RESOURCE statement as opposed to an ICON CONTROL statement.
               -- filename could have spaces and *should* be in quotes.
               parse var s nameID ic fileName
               self~addIcon(nameID, fileName)
            end
            /* premission: #define only at the top of the RC and separated by a tab or a blank */
            when s~wordpos("#define") > 0 then do
               s = s~translate(" ", "9"x)
               parse var s "#define " symb iid
               self~ConstDir[symb~space(0)~translate] = iid
            end
            otherwise
               nop
         end
         -- End select
      end
      if (self~checkfile(f) = 1) then return 1
      if arg(2,'o') = 1 | dialogid = "DIALOGID" then found = 1
      else do
         foundID = s~word(n-1)~translate
         if foundID = dialogid~translate then found = 1
         else if self~checkID(foundID) = dialogid then found = 1
         else if foundID = self~checkID(dialogid~translate) then found = 1
         else n = 0
      end
   end

   n = s~Pos(",")
   parse var s prev "," dlg.y "," dlg.w "," dlg.h
   dlg.x = s~substr(n-3,3)
   if dlg.x~datatype('N') = 0 then dlg.x = s~substr(n-2,2)

   s = f~linein; fl = fl -1
   do while s~wordpos("STYLE ") = 0 & fl > 0
      s = f~linein; fl = fl -1
   end
   if (self~checkfile(f) = 1) then return 1
   parse var s prev dlg.style
   dlg.style = dlg.style~translate(" ", "|")

   internopts = internopts || self~internGetRCStyle(dlg.style, "DLG")

   s = f~linein; fl = fl -1
   do while s~wordpos("CAPTION ") = 0 & s~wordpos("FONT ") = 0 & fl > 0
      s = f~linein; fl = fl -1
   end
   if (self~checkfile(f) = 1) then return 1
   if s~wordpos("CAPTION ") > 0 then do
      parse var s prev '"'dlg.title'"'
      s = f~linein; fl = fl -1
      do while s~wordpos("FONT ") = 0 & fl > 0
         s = f~linein; fl = fl -1
      end
      if fl > 0 then
         parse var s prev dlg.fsize "," '"'dlg.font'"'
      else do
         dlg.fsize = ""
         dlg.font = ""
      end
   end
   else do
      parse var s prev dlg.fsize "," '"'dlg.font'"'
      dlg.title = ""
   end
   f~close

   if self~create(dlg.x, dlg.y, dlg.w, dlg.h, dlg.title, internopts,, dlg.font, dlg.fsize, expected) then return 0
   else return 1


-- This method creates the dialog's controls from a resource script file. It is usuall called by the load() method.
-- Although its return was not documented prior to 4.1.0, it has returned 0 for success and non-zero for failure.
::method loadItems protected
   use arg resfile, dialogid, loadOptions
   if self~activePtr = 0 then return -2

   file = SysSearchPath("PATH",resfile)
   f = .stream~new(file)
   op = f~open(read)
   if op \= "READY:" then do
      say "Resource file not found --> " op
      return 1
   end
   if arg(3, "o") = 1 then loadOptions = ""; else loadOptions = loadOptions~translate
   found = 0; n = 0
   fl = f~lines
   do while found = 0 & fl > 0
      do while n = 0 & fl > 0
         s = f~linein; fl = fl - 1
         /* premission: #define only at the top of the RC and separated by a tab or a blank */
         if s~wordpos("#define") > 0 then do
             s = s~translate(" ", "9"x)
             parse var s "#define " symb iid
             self~ConstDir[symb~space(0)~translate] = iid
         end
         n = s~wordpos("DIALOG")
         if n = 0 then n = s~wordpos("DIALOGEX")
      end
      if self~checkfile(f)=1 then return 1
      if arg(2,'o') = 1 | dialogid = "DIALOGID" then found = 1
      else do
         foundID = s~word(n-1)~translate
         if foundID = dialogid~translate then found = 1
         else if self~checkID(foundID) = dialogid then found = 1
         else if foundID = self~checkID(dialogid~translate) then found = 1
         else n = 0
      end
   end

   s = f~linein; fl = fl - 1
   do while s~wordpos("BEGIN") = 0 & s~pos("{") = 0 & fl > 0
       s = f~linein; fl = fl - 1
   end
   if self~checkfile(f)=1 then return 1

   self~ProcessingLoad = 1
   do while fl > 0 & s~wordpos("END") = 0 & s~pos("}") = 0
      s = f~linein; fl = fl - 1
      if s~wordpos("END") > 0 | s~pos("}") > 0 then leave;

      if s~wordpos("CONTROL") > 0 then do
         parse var s type '"'name'"' "," id "," class "," style "," x "," y "," w "," h "," extStyle
         opts = ""
         id = id~strip
         class = class~strip~translate
         style = style~translate(" ", "|")
         extStyle = extStyle~translate(" ", "|")
         select
            when class = '"BUTTON"' then do
               if style~wordpos("BS_AUTORADIOBUTTON") > 0 | style~wordpos("BS_RADIOBUTTON") > 0 then
                  self~internAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "R")
               else if style~wordpos("BS_AUTOCHECKBOX") > 0 | style~wordpos("BS_CHECKBOX") > 0 then
                  self~internAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "C")
               else if style~wordpos("BS_GROUPBOX") > 0 then
                  self~addGroupBox(x, y, w, h, name, self~internGetRCStyle(style, "GB"), id)
               else if style~pos("BS_OWNERDRAW") = 0 & style~wordpos("BS_AUTO3STATE") > 0 then  /* if 3state is not ownerdrawn */
                  self~internAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "C")
               else self~internAddRCButton(id, x, y, w, h, name, style, loadOptions)
            end

            when class = '"STATIC"' | class = WC_STATIC then do
               opts = self~internGetRCStyle(style, "ST")
               select
                  when style~wordpos("SS_LEFT") > 0 then
                     self~addText(x, y, w, h, name, opts, id)
                  when style~wordpos("SS_RIGHT") > 0 then
                     self~addText(x, y, w, h, name, opts || " RIGHT", id)
                  when style~wordpos("SS_CENTER") > 0 then
                     self~addText(x, y, w, h, name, opts || " CENTER", id)
                  when style~wordpos("SS_SIMPLE") > 0 then
                     self~addText(x, y, w, h, name, opts || " SIMPLE", id)
                  when style~wordpos("SS_LEFTNOWORDWRAP") > 0 then
                     self~addText(x, y, w, h, name, opts || " LEFTNOWRAP", id)
                  when style~wordpos("SS_WHITERECT") > 0 then
                     self~addWhiteRect(x, y, w, h, opts, id)
                  when style~wordpos("SS_GRAYRECT") > 0 then
                     self~addGrayRect(x, y, w, h, opts, id)
                  when style~wordpos("SS_BLACKRECT") > 0 then
                     self~addBlackRect(x, y, w, h, opts, id)
                  when style~wordpos("SS_WHITEFRAME") > 0 then
                     self~addWhiteFrame(x, y, w, h, opts, id)
                  when style~wordpos("SS_GRAYFRAME") > 0 then
                     self~addGrayFrame(x, y, w, h, opts, id)
                  when style~wordpos("SS_BLACKFRAME") > 0 then
                     self~addBlackFrame(x, y, w, h, opts, id)
                  when style~wordpos("SS_ETCHEDFRAME") > 0 then
                     self~addEtchedFrame(x, y, w, h, opts, id)
                  when style~wordpos("SS_ETCHEDHORZ") > 0 then
                     self~addEtchedHorizontal(x, y, w, h, opts, id)
                  when style~wordpos("SS_ETCHEDVERT") > 0 then
                     self~addEtchedVertical(x, y, w, h, opts, id)
                  when style~wordpos("SS_BITMAP") > 0 then
                     self~addImage(id, x, y, w, h, "BITMAP " || opts)
                  when style~wordpos("SS_ENHMETAFILE") > 0 then
                     self~addImage(id, x, y, w, h, "METAFILE " || opts)
                  when style~wordpos("SS_ICON") > 0 then
                     self~addImage(id, x, y, w, h, "ICON " || opts)
                  otherwise
                      ret = errorDialog("Static item ignored due to unknown static style option in:" || -
                                         '0d0a'x || f "("s")")
               end
            end
            when class = '"EDIT"' then
               self~addEntryLine(id, name, x, y, w, h, self~internGetRCStyle(style, "EDIT"))

            when class = '"COMBOBOX"' then
               self~addComboBox(id, name, x, y, w, h, self~internGetRCStyle(style, "CB"))

            when class = '"LISTBOX"' then
               self~addListBox(id, name, x, y, w, h, self~internGetRCStyle(style, "LB"))

            when class = '"SCROLLBAR"' then
               self~addScrollBar(id, x, y, w, h, self~internGetRCStyle(style, "SB"))

            when class = 'WC_TREEVIEW' | class = '"SYSTREEVIEW32"' then
               self~AddTreeControl(id, name, x, y, w, h, self~getNamedCtrlStyle(style, "TREE"))

            when class = 'WC_LISTVIEW' | class = '"SYSLISTVIEW32"' then
               self~AddListControl(id, name, x, y, w, h, self~getNamedCtrlStyle(style, "LIST"))

            when class = 'PROGRESS_CLASS' | class = '"MSCTLS_PROGRESS32"' then
               self~AddProgressBar(id, x, y, w, h, self~getNamedCtrlStyle(style, "PROGRESS"))

            when class = 'TRACKBAR_CLASS' | class = '"MSCTLS_TRACKBAR32"' then
               self~AddSliderControl(id, name, x, y, w, h, self~getNamedCtrlStyle(style, "SLIDER"))

            when class = 'WC_TABCONTROL' | class = '"SYSTABCONTROL32"' then
               self~AddTabControl(id, name, x, y, w, h, self~getNamedCtrlStyle(style, "TAB"))

            when class = 'DATETIMEPICK_CLASS' then
               self~AddDateTimePicker(id, name, x, y, w, h, self~getNamedCtrlStyle(style, "DTP"))

            when class = 'MONTHCAL_CLASS' then
               self~AddMonthCalendar(id, name, x, y, w, h, self~getNamedCtrlStyle(style, "MONTH"))

            otherwise do
               self~errorFile(f, s)
               return 1
            end
         end /* select 2*/
      end
      else do
         parse var s type '"'name'"' "," id "," x "," y "," w "," h "," style "," extStyle
         if id~space(0) = "" then parse var s type id "," x "," y "," w "," h "," style "," extStyle
         id = id~strip
         type = type~strip~translate
         style = style~translate(" ", "|")
         extStyle = extStyle~translate(" ", "|")

         select
            when type = "DEFPUSHBUTTON" | type = "PUSHBUTTON" then
            do
               if type~left(3) = ("DEF") then style = style || " | BS_DEFPUSHBUTTON"
               self~internAddRCButton(id, x, y, w, h, name, style, loadOptions)
            end

            when type = "EDITTEXT" then
               self~addEntryLine(id, name, x, y, w, h, self~internGetRCStyle(style, "EDIT"))

            when type = "LTEXT" | type = "CTEXT" | type = "RTEXT" then
            do
               opts = self~internGetRCStyle(style, "ST")
               if type = "CTEXT" then opts = opts || " CENTER";
               else if type = "RTEXT" then opts = opts || " RIGHT";
               else if style~wordpos("SS_LEFTNOWORDWRAP") > 0 then opts = opts || " LEFTNOWRAP";
               self~addText(x, y, w, h, name, opts, id)
            end

            when type = "ICON" then do
               opts = self~internGetRCStyle(style, "ST")
               self~addImage(id, x, y, w, h, "ICON " || opts)
            end

            when type = "GROUPBOX" then
               self~addGroupBox(x, y, w, h, name, self~internGetRCStyle(style, "GB"), id)

            when type = "COMBOBOX" then
               self~addComboBox(id, name, x, y, w, h, self~internGetRCStyle(style, "CB"))

            when type = "LISTBOX" then
               self~addListBox(id, name, x, y, w, h, self~internGetRCStyle(style, "LB"))

            when type = "SCROLLBAR" then
               self~addScrollBar(id, x, y, w, h, self~internGetRCStyle(style, "SB"))

            when type = "AUTORADIOBUTTON" | type = "RADIOBUTTON" then
               self~internAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "R")

            when type = "AUTOCHECKBOX" | type = "CHECKBOX" then
               self~internAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "C")

            when type = "AUTO3STATE" then
               if style~pos("BS_OWNERDRAW") > 0 then   /* sometimes AUTO3STATE is used for an owner draw button */
                   self~internAddRCButton(id, x, y, w, h, name, style, loadOptions, "C")  /* if so, treat as normal button */
               else  /* otherwise (not owner drawn) treat as 3state */
                   self~internAddRC_RBCB(id, x, y, w, h, name, style || " | BS_AUTO3STATE", loadOptions, "C")

            otherwise do
               self~errorFile(f, s)
               return 1
            end
         end /*select 1*/
      end
   end /* do while */
   f~close
   self~ProcessingLoad = 0
   return 0

   /* This method adds a push button to the dialog and connect it with */
   /* a message that will be rised if the butten is pushed. */
   /* argument 'options' can be one of: */

::method addButton
   use strict arg id, x, y, cx, cy, text = "", msgToRise = "", options = ""
   id = self~itemAdd(id)
   if id < 0 then return id
   options = self~getDefaultOpts(options)
   self~activePtr = UsrAddControl("BUT", self~activePtr, id, x, y, cx, cy, text, options)
   if arg(7, 'O') then return 0

   methName = msgToRise~space(0)
   if methName \== '' then return self~connectButton(id, methName)
   else return 1

::method addGroupBox
   use arg x, y, cx, cy, text = "", opts, id
   id = self~itemAdd(id, \(arg(7,"o") | id = "ID"))
   if id = -2 then return id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("GB",self~activePtr, x, y, cx, cy, opts, text, id)
   return 0


::method addStatic
   use strict arg id = (-1), x, y, cx = 0, cy = 0, opts = "TEXT", text = ""
   argA = .array~new
   opts = self~getDefaultOpts(opts)
   argA[1] = x
   argA[2] = y
   argA[3] = cx
   argA[4] = cy
   argA[5] = opts
   argA[6] = id

   select
     when opts~wordpos("TEXT") > 0 then return self~addText(x, y, cx, cy, text, opts, id)

     when opts~wordpos("BITMAP") > 0 | opts~wordpos("METAFILE") > 0 | opts~wordpos("ICON") > 0 then do
        forward message "addImage"
     end

     when opts~wordpos("WHITERECT") > 0 then return self~internAddStatic(argA, 0)
     when opts~wordpos("GRAYRECT") > 0 then return self~internAddStatic(argA, 1)
     when opts~wordpos("BLACKRECT") > 0 then return self~internAddStatic(argA, 2)
     when opts~wordpos("WHITEFRAME") > 0 then return self~internAddStatic(argA, 3)
     when opts~wordpos("GRAYFRAME") > 0 then return self~internAddStatic(argA, 4)
     when opts~wordpos("BLACKFRAME") > 0 then return self~internAddStatic(argA, 5)
     when opts~wordpos("ETCHED") > 0 then return self~internAddStatic(argA, 6)
     when opts~wordpos("HORZ") > 0 then return self~internAddStatic(argA, 7)
     when opts~wordpos("VERT") > 0 then return self~internAddStatic(argA, 8)

     otherwise return self~addText(x, y, cx, cy, text, opts, id)
   end
   -- End select

::method addImage
   use strict arg id, x, y, cx, cy, options = ""
   id = self~itemAdd(id)
   if id < 0 then return id
   options = self~getDefaultOpts(options)
   self~activePtr = UsrAddControl("IMG", self~activePtr, id, x, y, cx, cy, options)
   return 0

::method addText
   use arg x, y, cx = 0, cy = 0, text = "", opts = "", id = (-1)

   if cx = 0 | cy = 0 then do
      size = self~getTextSizeDlg(text)
      if cx = 0 then cx = size~width + 2
      if cy = 0 then cy = size~height
   end
   id = self~itemAdd(id, 1)
   if id = -2 then return id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("TXT", self~activePtr, x, y, cx, cy, opts, text, id)
   return 0

::method addWhiteRect
   return self~internAddStatic(arg(1,"A"), 0)

::method addGrayRect
   return self~internAddStatic(arg(1,"A"), 1)

::method addBlackRect
   return self~internAddStatic(arg(1,"A"), 2)

::method addWhiteFrame
   return self~internAddStatic(arg(1,"A"), 3)

::method addGrayFrame
   return self~internAddStatic(arg(1,"A"), 4)

::method addBlackFrame
   return self~internAddStatic(arg(1,"A"), 5)

::method addEtchedFrame
   return self~internAddStatic(arg(1,"A"), 6)

::method addEtchedHorizontal
   return self~internAddStatic(arg(1,"A"), 7)

::method addEtchedVertical
   return self~internAddStatic(arg(1,"A"), 8)

::method internAddStatic private
   use arg arga, type
   if arga~hasindex(6) = 0 then id = self~itemAdd(-1, 0); else id = self~itemAdd(arga[6],1)
   if id = -2 then return id
   opts = self~getDefaultOpts(arga[5])
   self~activePtr = UsrAddControl("FRM", self~activePtr, arga[1], arga[2], arga[3], arga[4], type, opts, id)
   return 0

::method addEntryLine
   use arg id, fname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then fname = "DATA"id
   if arg(6,'o') = 1 then cy = 0
   if cy = 0 then do
      size = self~getTextSizeDlg("Tg")
      cy = size~height + 4
   end
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("EL", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then return self~ConnectEntryLine(symbid, fname)
   else return 0


::method addPasswordLine
   newarg = arg(1, 'A')
   if arg(7,'o') = 1 then newarg[7] = ""
   newarg[7] = newarg[7] || " PASSWORD"
   forward message "addEntryLine" Arguments (newarg)


::method addListBox
   use arg id, fname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then fname = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("LB", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then do
      if opts~wordpos("MULTI") > 0 then ret = self~ConnectMultiListBox(symbid, fname)
      else ret = self~ConnectListBox(symbid, fname)
   end
   return ret


::method addComboBox
   use arg id, fname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then fname = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("CB", self~activePtr, id, x, y, cx, cy, opts)
   if opts~wordpos("CAT") = 0 & self~autoDetect then return self~ConnectComboBox(symbid, fname, opts)
   else return 0


::method addCheckBox
   use arg id, fname, x, y, cx, cy, text, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then fname = text
   if arg(5, 'o') = 1 then cx = 0
   if arg(6, 'o') = 1 then cy = 0
   if cx = 0 | cy = 0 then do
      size = self~getTextSizeDlg(text)
      if cx = 0 then cx = size~width + 12
      if cy = 0 then cy = size~height
   end
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("CH", self~activePtr, id, x, y, cx, cy, text, opts)
   if self~autoDetect then return self~ConnectCheckBox(symbid, fname)
   else return 0


::method addRadioButton
   use arg id, fname, x, y, cx, cy, text, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then fname = text
   if arg(5, 'o') = 1 then cx = 0
   if arg(6, 'o') = 1 then cy = 0
   if cx = 0 | cy = 0 then do
      size =self~getTextSizeDlg(text)
      if cx=0 then cx=size~width + 12
      if cy=0 then cy=size~height
   end
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("RB", self~activePtr, id, x, y, cx, cy, text, opts)
   if opts~wordpos("CAT") = 0 & self~autoDetect then return self~ConnectRadioButton(symbid, fname)
   else return 0


::method addScrollBar
   use arg id, x, y, cx, cy, opts
   id = self~itemAdd(id)
   if id < 0 then return id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddControl("SB", self~activePtr, id, x, y, cx, cy, opts)
   return 0


::method addRadioGroup
   arr = arg(1,"A")
   arr[8] = "R"
   forward message "internAddRBCBGroup" Arguments (arr)

::method addCheckGroup
   arr = arg(1,"A")
   arr[8] = "C"
   forward message "internAddRBCBGroup" Arguments (arr)

::method internAddRBCBGroup
   use arg startid, x, y, cx, inp, opts, idstat, kind

   if self~activePtr = 0 then return -2
   if self~checkID(startid) = -1 then return
   if arg(4, 'o') = 1 then cx = 0
   parse var inp rb inp
   size = self~getTextSizeDlg(rb)
   len = size~width
   stepy = size~height
   lastlen = len; oldy = stepy; stepy = trunc(stepy * 1.5)
   opts = self~getDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then my = y + oldy; else my = y
   newopts = opts~changestr("NOBORDER","")
   if cx \= 0 then len = cx-20
   if arg(7,'o') = 1 then idstat = self~getStaticID

   i = 0
   do while rb \= ""
      if kind = "R" then do
          if i = 0 then self~addRadioButton(startid+i,, x+4, my, len+12, stepy, rb, newopts || " GROUP")
          else self~addRadioButton(startid + i,, x+4, my, len+12, stepy, rb, newopts)
      end
      else do
          self~addCheckBox(startid + i,, x+4, my, len+12, stepy, rb, newopts)
      end
      my = my + stepy; i = i + 1
      parse var inp rb inp
      if cx = 0 then do
         s = self~getTextSizeDlg(rb)
         len = s~width
         if len > lastlen then lastlen = len
      end
   end
   if cx \= 0 then LastLen = cx-20
   size = .Size~new(lastlen+20, my-y+trunc(oldy / 2))
   if nob = 0 then self~addGroupBox(x,y,size~width,size~height,"", newopts, idstat)
   return size


::method addInput
   use arg id, fname, x, y, cx1, cx2, cy, text, opts, idstat
   if self~activePtr = 0 then return -2
   if self~checkID(id) = -1 then return
   if arg(2,'o') = 1 then fname = text
   if arg(5, 'o') = 1 then cx1 = 0
   if arg(7, 'o') = 1 then cy = 0
   if cy = 0 | cx1 = 0 then do
      size = self~getTextSizeDlg(text)
      if cy = 0 then cy = size~height + 4
      if cx1 = 0 then cx1 = size~width + 4
   end
   if arg(10,'o') = 1 then idstat = self~getStaticID
   opts = self~getDefaultOpts(opts)
   ret = self~addText(x, y+2, cx1, cy, text, opts, idstat)
   if ret == 0 then ret = self~addEntryLine(id, fname, x+cx1+1, y, cx2, cy, opts)
   return ret


::method addInputGroup
   use arg startid, x, y, cx1, cx2, inp, opts, idstat
   parse var inp rb inp

   if self~activePtr = 0 then return -2
   if self~checkID(startid) = -1 then return
   if arg(4, 'o') = 1 then cx1 = 0
   count = 0; ll = 0
   do while rb \= ""
      count = count + 1
      data.count = rb
      if cx1 = 0 then do
         size = self~getTextSizeDlg(rb)
         if size~width > ll then ll = size~width
      end
      parse var inp rb inp
   end
   if cx1 = 0 then cx1 = ll+4
   if arg(8,'o') = 1 then idstat = self~getStaticID
   is = idstat

   s = self~getTextSizeDlg("Tg")
   stepy = s~height
   opts = self~getDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then my = y + trunc(stepy * 1.25); else my = y
   newopts = opts~changestr("NOBORDER", "")
   do i=1 to count
      self~addInput(startid + i -1,, x+4, my, cx1-4, cx2-6, stepy+4, data.i, newopts, is)
      if is \= -1 then is = is + 1
      my = my + trunc(stepy * 2.25)
   end
   if nob = 0 then return self~addGroupBox(x,y,cx1+cx2,my-y,"", newopts, is)
   else return 0


::method addComboInput
   use arg id, attname, x, y, cx1, cx2, clines, text, opts, idstat

   if self~activePtr = 0 then return -2
   if self~checkID(id) = -1 then return
   if arg(2, 'o') = 1 then attname = text
   if arg(5, 'o') = 1 then cx1 = 0
   if arg(7, 'o') = 1 then clines = 5

   s = self~getTextSizeDlg(text)
   cy = s~height + 4
   if cx1 = 0 then cx1 = s~width + 4
   if arg(10,'o') = 1 then idstat = self~getStaticID
   opts = self~getDefaultOpts(opts)
   ret = self~addText(x, y, cx1, cy, text, opts, idstat)
   if ret == 0 then return self~addComboBox(id, attname, x+cx1+2, y, cx2, cy*clines, opts)
   else return 0


::method addInputStem
   use arg startid, x, y, cx1, cx2, inp., opts, idstat

   if self~activePtr = 0 then return -2
   if self~checkID(startid)= -1 then return
   if arg(4, 'o') = 1 then cx1 = 0
   count = 0; ll = 0
   do while var("inp."count+1) = 1
      count = count + 1
      if cx1 = 0 then do
         s = self~getTextSizeDlg(inp.count)
         if s~width > ll then ll = s~width
      end
   end
   if cx1 = 0 then cx1 = ll+4

   if idstat = "IDSTAT" then idstat = self~getStaticID
   is = idstat
   s = self~getTextSizeDlg("Tg")
   stepy = s~height

   opts = self~getDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then my = y + trunc(stepy * 1.25); else my = y
   newopts = opts~changestr("NOBORDER", "")
   do i=1 to count
      self~addInput(startid + i -1,, x+4, my, cx1-4, cx2-6, stepy+4, inp.i, newopts, is)
      if is \= -1 then is = is + 2
      my = my + trunc(stepy * 2.25)
   end
   if nob = 0 then return self~addGroupBox(x,y,cx1+cx2,my-y,"", newopts, is)
   else return 0


::method addCheckBoxStem
   arr = arg(1,"A")
   arr[11] = "C"
   forward message "internAddRBCBStem" Arguments (arr)

::method addRadioStem
   arr = arg(1,"A")
   arr[11] = "R"
   forward message "internAddRBCBStem" Arguments (arr)

::method internAddRBCBStem private
   use arg startid, x, y, cxx, inp., max, opts, idstat, fn, fs, kind
   -- Note that font name and font size (fn, fs) are no longer used.  They are
   -- here for backwards compatibility.

   if self~activePtr = 0 then return -2
   if self~checkID(startid) = -1 then return
   if arg(4, 'o') = 1 then cxx = 0
   count = 0; ll = 0
   do while var("inp."count+1) = 1
      count = count + 1
      if cxx = 0 then do
         s = self~getTextSizeDlg(inp.count)
         if s~width > ll then ll = s~width
      end
   end
   if var("max") = 0 | max = 0 then max = count
   if cxx = 0 & max \= 1 then cx = ll+20; else cx = cxx
   if arg(8, 'o') = 1 then idstat = self~getStaticID
   s = self~getTextSizeDlg("Tg")
   stepx = s~width
   oldy = s~height
   mx = 0; highy = 0
   opts = self~getDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then do
      stepy = trunc(oldy * 1.5)
      my = y + oldy
      newopts = opts
   end
   else do
      stepy = oldy
      my = y
      newopts = delword(opts, nob, 1)
   end
   do i=1 to count
      if (i>1) & ((i-1)//max = 0) then do
         highy = my
         if nob = 0 then my = y + oldy; else my = y
         mx = mx + cx
      end
      if max=1 & cxx=0 then do
         s = self~getTextSizeDlg(inp.i)
         cx = s~width + 24
      end
      if kind = "R" then
      do
         if i = 1 then tmpopts = newopts || " GROUP"; else tmpopts = newopts
         self~addRadioButton(startid + i -1,, x+mx+4, my, cx-8, stepy, inp.i, tmpopts)
      end; else self~addCheckBox(startid + i -1,, x+mx+4, my, cx-8, stepy, inp.i, newopts)
      my = my + stepy
   end
   if highy = 0 then highy = my
   if nob = 0 then return self~addGroupBox(x,y,mx+cx, highy-y+(stepy % 2),"", newopts, idstat)
   else return 0


::method addButtonGroup
   use arg x, y, cx, cy, inp, bottom, opts
   parse var inp bt bid bfnk newinp

   if self~activePtr = 0 then return -2
   if arg(6,"o") = 1 then bottom = 0
   if cx = 0 | arg(3,"o") = 1 then cx = 40
   if cy = 0 | arg(4,"o") = 1 then cy = 12

   opts = self~getDefaultOpts(opts)
   do while bt \= ""
      if bt~pos("'") > 0 then parse var inp "'"bt"'" bid bfnk newinp
      else if bt~pos('"') > 0 then parse var inp '"'bt'"' bid bfnk newinp
      if bfnk = 0 then bfnk = ""
      self~AddButton(bid, x, y, cx, cy, bt, bfnk, opts)
      n = opts~wordpos("DEFAULT")
      if n \= 0 then opts = opts~delword(n,1)
      if bottom = 0 then y = y + trunc(cy * 1.5); else x = x + trunc(cx * 1.25)
      inp = newinp
      parse var inp bt bid bfnk newinp
   end
   return 0


::method addOkCancelRightBottom
   return self~addButtonGroup(self~sizeX-95, self~sizeY - 15,,, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method addOkCancelLeftBottom
   return self~addButtonGroup(5, self~sizeY - 15,,, "&Ok 1 OK &Cancel 2 CANCEL", 1, "DEFAULT")

::method addOkCancelRightTop
   return self~addButtonGroup(self~sizeX-45, 5,,, "&Ok 1 OK &Cancel 2 CANCEL", 0, " DEFAULT")

::method addOkCancelLeftTop
   return self~addButtonGroup(5, 5,,, "&Ok 1 OK &Cancel 2 CANCEL", 0, "DEFAULT")

::method addIconFile private external "LIBRARY oodialog dyndlg_addIconFile_pvt"
::method addIcon
   use strict arg id, fileName
   return self~addIconFile(id, fileName~strip~strip('B', '"'))


::method getDefaultOpts private
   use strict arg opts
   if opts == "" | opts == .nil | opts == "OPTS" | opts == "OPTIONS" then return ""
   else return opts~translate


::method getStaticID private
   return -1


::method startIt
   use arg icon = 0, modeless = .false
   if self~basePtr = 0 then return 0

   icon = self~resolveIconID(icon)
   if modeless <> 0 & modless <> 1 then do
     if modeless~translate = "NOTMODAL" then modeless = .true
     else modeless = .false
   end

   if \ self~startParentDialog(icon, modeless) then return 0

   self~initDialog
   return self~dlgHandle

::method stopDynamic private external "LIBRARY oodialog dyndlg_stopDynamic_pvt"


::method getNamedCtrlStyle private
   use arg style, kind
   ret = ""
   if style~wordpos("NOT WS_VISIBLE") > 0 then ret = "HIDDEN"
   else if style~wordpos("WS_VISIBLE") > 0 then ret = "VISIBLE"
   if style~wordpos("WS_DISABLED") > 0 & style~wordpos("NOT WS_DISABLED") == 0 then ret = ret || " DISABLED"
   if kind = "TREE" then do
      if style~wordpos("TVS_DISABLEDRAGDROP") > 0 then ret = ret || " NODRAG"
      if style~wordpos("TVS_EDITLABELS") > 0 then ret = ret || " EDIT"
      if style~wordpos("TVS_HASBUTTONS") > 0 then ret = ret || " BUTTONS"
      if style~wordpos("TVS_HASLINES") > 0 then ret = ret || " LINES"
      if style~wordpos("TVS_LINESATROOT") > 0 then ret = ret || " ATROOT"
      if style~wordpos("TVS_SHOWSELALWAYS") > 0 then ret = ret || " SHOWSELALWAYS"
      if style~wordpos("WS_BORDER") = 0 then ret = ret || " NOBORDER"
   end
   else if kind = "LIST" then do
      if style~wordpos("LVS_EDITLABELS") > 0 then ret = ret || " EDIT"
      if style~wordpos("LVS_SHOWSELALWAYS") > 0 then ret = ret || " SHOWSELALWAYS"
      if style~wordpos("LVS_ALIGNLEFT") > 0 then ret = ret || " ALIGNLEFT"
      if style~wordpos("LVS_ALIGNTOP") > 0 then ret = ret || " ALIGNTOP"
      if style~wordpos("LVS_AUTOARRANGE") > 0 then ret = ret || " AUTOARRANGE"
      if style~wordpos("LVS_ICON") > 0 then ret = ret || " ICON"
      if style~wordpos("LVS_SMALLICON") > 0 then ret = ret || " SMALLICON"
      if style~wordpos("LVS_LIST") > 0 then ret = ret || " LIST"
      if style~wordpos("LVS_REPORT") > 0 then ret = ret || " REPORT"
      if style~wordpos("LVS_NOCOLUMNHEADER") > 0 then ret = ret || " NOHEADER"
      if style~wordpos("LVS_NOLABELWRAP") > 0 then ret = ret || " NOWRAP"
      if style~wordpos("LVS_NOSCROLL") > 0 then ret = ret || " NOSCROLL"
      if style~wordpos("LVS_NOSORTHEADER") > 0 then ret = ret || " NOSORTHEADER"
      if style~wordpos("LVS_SHAREIMAGELISTS") > 0 then ret = ret || " SHAREIMAGES"
      if style~wordpos("LVS_SINGLESEL") > 0 then ret = ret || " SINGLESEL"
      if style~wordpos("LVS_SORTASCENDING") > 0 then ret = ret || " ASCENDING"
      if style~wordpos("LVS_SORTDESCENDING") > 0 then ret = ret || " DESCENDING"
      if style~wordpos("WS_BORDER") = 0 then ret = ret || " NOBORDER"
   end
   else if kind = "SLIDER" then do
       if style~wordpos("TBS_AUTOTICKS") > 0 then ret = ret || " AUTOTICKS"
       if style~wordpos("TBS_NOTICKS") > 0 then ret = ret || " NOTICKS"
       if style~wordpos("TBS_VERT") > 0 then ret = ret || " VERTICAL"
       if style~wordpos("TBS_HORZ") > 0 then ret = ret || " HORIZONTAL"
       if style~wordpos("TBS_TOP") > 0 then ret = ret || " TOP"
       if style~wordpos("TBS_BOTTOM") > 0 then ret = ret || " BOTTOM"
       if style~wordpos("TBS_LEFT") > 0 then ret = ret || " LEFT"
       if style~wordpos("TBS_RIGHT") > 0 then ret = ret || " RIGHT"
       if style~wordpos("TBS_BOTH") > 0 then ret = ret || " BOTH"
       if style~wordpos("TBS_ENABLESELRANGE") > 0 then ret = ret || " ENABLESELRANGE"
       if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
   end
   else if kind = "TAB" then do
        if style~wordpos("WS_GROUP") > 0 then ret = ret || " GROUP"
        if style~wordpos("TCS_BUTTONS") > 0 then ret = ret || " BUTTONS"
        if style~wordpos("TCS_FIXEDWIDTH") > 0 then ret = ret || " FIXED"
        if style~wordpos("TCS_FOCUSNEVER") > 0 then ret = ret || " FOCUSNEVER"
        if style~wordpos("TCS_FOCUSONBUTTONDOWN") > 0 then ret = ret || " FOCUSONDOWN"
        if style~wordpos("TCS_FORCEICONLEFT") > 0 then ret = ret || " ICONLEFT"
        if style~wordpos("TCS_FORCELABELLEFT") > 0 then ret = ret || " LABELLEFT"
        if style~wordpos("TCS_MULTILINE") > 0 then ret = ret || " MULTILINE"
        if style~wordpos("TCS_RIGHTJUSTIFY") > 0 then ret = ret || " ALIGNRIGHT"
        if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
   end
   else if kind = "PROGRESS" then do
        if style~wordpos("PBS_VERTICAL") > 0 then ret = ret || " VERTICAL"
        if style~wordpos("PBS_SMOOTH") > 0 then ret = ret || " SMOOTH"
        if style~wordpos("PBS_MARQUEE") > 0 then ret = ret || " MARQUEE"
        if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
        if style~wordpos("WS_TABSTOP") > 0 then ret = ret || " TAB"
   end
   else if kind = "DTP" then do
        if style~wordpos("DTS_APPCANPARSE") > 0 then ret = ret || " PARSE"
        if style~wordpos("DTS_RIGHTALIGN;") > 0 then ret = ret || " RIGHT"
        if style~wordpos("DTS_SHOWNONE;  ") > 0 then ret = ret || " NONE"
        if style~wordpos("DTS_UPDOWN;    ") > 0 then ret = ret || " UPDOWN"
        if style~wordpos("DTS_LONGDATEFORMAT") > 0 then ret = ret || " LONG"
        if style~wordpos("DTS_SHORTDATEFORMAT") > 0 then ret = ret || " SHORT"
        if style~wordpos("DTS_SHORTDATECENTURYFORMAT") > 0 then ret = ret || " CENTURY"
        if style~wordpos("DTS_TIMEFORMAT") > 0 then ret = ret || " TIME"
        if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
   end
   else if kind = "MONTH" then do
        if style~wordpos("MCS_DAYSTATE")  > 0 then ret = ret || " DAYSTATE"
        if style~wordpos("MCS_MULTISELECT")  > 0 then ret = ret || " MULTI"
        if style~wordpos("MCS_NOTODAY")  > 0 then ret = ret || " NOTODAY"
        if style~wordpos("MCS_NOTODAYCIRCLE")  > 0 then ret = ret || " NOCIRCLE"
        if style~wordpos("MCS_WEEKNUMBERS")  > 0 then ret = ret || " WEEKNUMBERS"
        if style~wordpos("WS_BORDER")  > 0 then ret = ret || " BORDER"
   end
   if style~wordpos("WS_VSCROLL") > 0 then ret = ret || " VSCROLL"
   if style~wordpos("WS_HSCROLL") > 0 then ret = ret || " HSCROLL"
   if style~wordpos("WS_TABSTOP") = 0 then ret = ret || " NOTAB"
   return ret


::method addTreeControl
   use arg id, attname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then attname = "DATA"id
   opts = self~getDefaultOpts(opts)
   if opts = "ALL" then opts = "LINES VSCROLL HSCROLL EDIT BUTTONS ATROOT SHOWSELALWAYS"
   self~activePtr = UsrAddNewCtrl("TREE", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then do
       return self~ConnectTreeControl(symbid, attname)
   end
   return 0

::method addListControl
   use arg id, attname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then attname = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddNewCtrl("LIST", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then do
       return self~ConnectListControl(symbid, attname)
   end
   return 0

::method addProgressBar
   use arg id, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddNewCtrl("PROGRESS", self~activePtr, id, x, y, cx, cy, opts)
   return 0

::method addSliderControl
   use arg id, attname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then attname = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddNewCtrl("SLIDER", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then do
       return self~ConnectSliderControl(symbid, attname)
   end
   return 0

::method addTabControl
   use arg id, attname, x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if arg(2,'o') = 1 then attname = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddNewCtrl("TAB", self~activePtr, id, x, y, cx, cy, opts)
   if opts~wordpos("CAT") = 0 & self~autoDetect then do
       return self~ConnectTabControl(symbid, attname)
   end
   return 0

::method addDateTimePicker
   use strict arg id, attributeName = "", x, y, cx, cy, opts = ""
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if attributeName = "" then attributeName = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddNewCtrl("DTP", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then do
      return self~connectDateTimePicker(symbid, attributeName)
   end
   return 0

::method addMonthCalendar
   use strict arg id, attributeName = "", x, y, cx, cy, opts
   symbid = id
   id = self~itemAdd(id)
   if id < 0 then return id
   if attributeName = "" then attributeName = "DATA"id
   opts = self~getDefaultOpts(opts)
   self~activePtr = UsrAddNewCtrl("MONTH", self~activePtr, id, x, y, cx, cy, opts)
   if self~autoDetect then do
      return self~connectMonthCalendar(symbid, attributeName)
   end
   return 0



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
/******************************************************************************/
/*                                                                            */
/*       Windows Dialog Interface for Object REXX                             */
/*                                                                            */
/* Dynamic Dialog Class                                                       */
/*                                                                            */
/******************************************************************************/

::class DynamicDialog Mixinclass Object public

    /* attributes for internal use */
::method BasePtr attribute protected
::method AktPtr attribute protected
::method DialogItemCount attribute protected

    /* attributes for internal use (menu) */
::method MenuBasePtr attribute protected
::method MenuAktPtr attribute protected
::method MenuMemHandle attribute protected


::method DynamicInit private
   if RxFuncQuery("InstUserMMFuncs") \= 0 then do
      ret = Rxfuncadd("InstUserMMFuncs", "oodialog", "InstUserMMFuncs")
      if (ret = 0) then ret = InstUserMMFuncs()
      if ret \= 0 then
      do
         self~deinstall
         say "Couldn't load function library -->" ret
         self~InitCode = 1
         return 1
      end
   end
   self~AktPtr = 0
   self~MenuAktPtr = 0
   self~MenuBasePtr = 0
   self~MenuMemHandle = 0
   self~DialogItemCount = 0
   if Arg(1,'o') = 1 then self~UseStem = 0; else self~UseStem = 1
   self~ConstDir = .directory~new
   self~ConstDir["IDOK"] = 1
   self~ConstDir["IDCANCEL"] = 2
   self~ConstDir["IDHELP"] = 9
   return 0

   /*  */
::method InitAutoDetection protected
   self~AutoDetection

   /* This method creates the dialog you previously defined via Add... methods */
   /* You can set the font and the number of expected items */
   /* argument 'options' can be one of:
         "CHILD"       -  create a child dialog
         "NOMENU"      -  create a dialog without menu
         "VISIBLE"     -  make the dialog visible before executing
         "SYSTEMMODAL" -  create a dialog that is always on top
         "NOTMODAL"    -  create a dialog with a normal window frame
         "THICKFRAME"  -  create a dialog with a thick frame */

::method Create
   use arg x, y, cx, cy, title, options, dlgClass, fontName, fontSize, expected

   /* need to check for uppercase name because of method forwarding */
   if Arg(6,"o") = 1 then options = ""; else options = options~translate
   if Arg(10,"o") = 1 | expected = 0 then expected = 200
   if Arg(7,"o") = 1 | dlgClass = 0 then dlgClass = ""
   if Arg(8,"o") = 1 then fontName = "System"
   if Arg(9,"o") = 1 then fontSize = 8
   ret = UsrDefineDialog(x, y, cx, cy, dlgclass, title, fontName, fontSize,,
                                      options, expected)
   parse var ret base ptr
   self~BasePtr = base
   self~AktPtr = ptr
   if ptr = 0 then call HandleDlg("STOP")
   self~SizeX = cx
   self~SizeY = cy
   self~DefineDialog
   return ptr  /* self~AktPtr could have been set to 0 for a CategoryDialog */

   /* This method will create the dialog and place it in a centered position */
   /* See 'Create' for possible 'options' */

::method CreateCenter
   arga = Arg(1,"A")
   newarg = .array~new(Arg()+2)
   do i = Arg() to 1 by -1; if arga~hasindex(i) = 1 then newarg[i+2] = arga[i]; end   /* shift arguments up by 2 */
   parse value GetScreenSize() with sx sy .
   newarg[1] = (sx - Arg(1)) % 2   /* centered x position */
   newarg[2] = (sy - Arg(2)) % 2   /* centered y position */
   forward message "Create" Arguments (newarg)

   /* This method is privat !!! */

::method Checkfile private
   use arg f
   if (f~lines = 0) then do
      f~close
      self~ProcessingLoad = 0
      ret = ErrorMessage("Error reading resource file!" f)
      return 1
   end
   return 0


::method Errorfile private
   use arg f, s
   f~close
   self~ProcessingLoad = 0
   ret = ErrorMessage("Error reading resource file:" f "("s")")


   /* This method is provided to be used in any subclass of 'UserDialog' */

::method DefineDialog protected
   return 0


   /* This method creates the dialog on the data of a given resource file   */
   /* (extention '.rc'). It calls 'LoadFrame' and 'LoadItems' */
   /* argument 'loadOptions' can be one or more of:                         */
   /*    "CENTER"      : this will create the dialog on a centered position */
   /*    "CONNECTBUTTONS" :  */
   /*    "CONNECTRADIOS"  */
   /*    "CONNECTCHECKS" */

::method Load
   forward message "LoadFrame" continue
   hnd = result
   if hnd \= 0 then forward message "LoadItems" continue; else call HandleDlg("STOP")
   return (hnd=0)


::method InternGetRCStyle
   use arg style, kind, spec_kind
   ret = ""
   if style~wordpos("NOT WS_VISIBLE") > 0 then ret = "HIDDEN"
   else if style~wordpos("WS_VISIBLE") > 0 then ret = "VISIBLE"
   if style~wordpos("WS_DISABLED") > 0 then ret = ret || " DISABLED"
   if style~wordpos("WS_GROUP") > 0 then ret = ret || " GROUP"
   if kind = "DLG" then do
      if style~wordpos("DS_SYSMODAL") > 0 then ret = ret ||" SYSTEMMODAL"
      if style~wordpos("DS_MODALFRAME") = 0 then ret = ret ||" NOTMODAL"
      if style~wordpos("WS_SYSMENU") = 0 then ret = ret ||" NOMENU"
      if style~wordpos("WS_THICKFRAME") > 0 then ret = ret ||" THICKFRAME"
      if style~wordpos("WS_MINIMIZEBOX") > 0 then ret = ret ||" MINIMIZEBOX"
      if style~wordpos("WS_MAXIMIZEBOX") > 0 then ret = ret ||" MAXIMIZEBOX"
      if style~wordpos("WS_OVERLAPPED") > 0 then ret = ret ||" OVERLAPPED"
   end
   else if kind = "BTN" then do
       if style~wordpos("WS_OWNERDRAW") > 0 then ret = ret || " OWNER"
       else if style~wordpos("BS_OWNERDRAW") > 0 then ret = ret || " OWNER"
       if ret~wordpos("OWNER") = 0 then if style~wordpos("BS_AUTO3STATE") > 0 then ret = ret || " 3STATE"
       if style~wordpos("BS_DEFPUSHBUTTON") > 0 then ret = ret || " DEFAULT"
       if style~wordpos("BS_NOTIFY") > 0 then ret = ret || " NOTIFY"
       if style~wordpos("BS_LEFTTEXT") > 0 then ret = ret || " LEFTTEXT"
       if style~wordpos("BS_BITMAP") > 0 then ret = ret || " BITMAP"
       if style~wordpos("BS_ICON") > 0 then ret = ret || " BITMAP"
       if spec_kind \= "R" then do  /* not a radio button */
           if style~wordpos("NOT WS_TABSTOP") > 0 then ret = ret || " NOTAB"
       end
       else if style~wordpos("WS_TABSTOP") = 0 then ret = ret || " NOTAB"
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
       if style~wordpos("WS_BORDER") > 0 then ret = ret || " BORDER"
   end
   else if kind = "SB" then do
      if style~wordpos("SBS_HORZ") > 0 then ret = ret || " HORIZONTAL"
      if style~wordpos("SBS_TOPALIGN") > 0 then ret = ret || " TOPLEFT"
      if style~wordpos("SBS_BOTTOMALIGN") > 0 then ret = ret || " BOTTOMRIGHT"
      if style~wordpos("WS_TABSTOP") > 0 then ret = ret || " TAB"
   end

   if kind = "EDIT" | kind = "LB" then
      if style~wordpos("NOT WS_BORDER") > 0 then ret = ret || " NOBORDER"
   if kind = "EDIT" | kind = "CB" | kind = "LB" | kind = "DLG" then do
      if style~wordpos("WS_VSCROLL") > 0 then ret = ret || " VSCROLL"
      if style~wordpos("WS_HSCROLL") > 0 then ret = ret || " HSCROLL"
   end

   return ret


::method InternAddRCButton
   use arg id, x, y, w, h, name, style, lOpts
   id = self~CheckId(id)
   if id < 0 then return
   opts = self~InternGetRCStyle(style, "BTN")
   self~DialogItemCount = self~DialogItemCount + 1
   self~AktPtr = UsrAddControl("BUT", self~AktPtr, id, x, y, w, h, name, opts)
   if lOpts~wordpos("CONNECTBUTTONS") > 0 then
      self~ConnectButton(id, name~space(0)~changestr('&','')~changestr(':',''))
   else self~ConnectButton(id)


::method InternAddRC_RBCB
   use arg id, x, y, w, h, name, style, lOpts, kind
   opts = self~InternGetRCStyle(style, "BTN", kind)
   if (kind = "R" & lOpts~wordpos("CONNECTRADIOS") > 0) | lOpts~wordpos("CONNECTCHECKS") > 0 then
   do
      self~ConnectControl(id, "ID"||name~space(0)~changestr('&','')~changestr(':',''))
      if kind = "R" then self~AddRadioButton(id,, x, y, w, h, name, opts)
      else self~AddCheckBox(id,, x, y, w, h, name, opts)
   end
   else do
      if kind = "R" then self~AddRadioButton(id,name,x, y, w, h, name, opts)
      else self~AddCheckBox(id, name, x, y, w, h, name, opts)
   end


   /* This method creates the dialog main window using the data of the */
   /* given resource file. It is called by the 'Load' method. */

::method LoadFrame protected
   use arg resfile, dialogid, options, expected
   if Arg(4,"o") = 1 | expected = 0 then expected = 200
   internopts = ""
   if Arg(3,'o') = 1 then options = ""; else options = options~translate

   file = SysSearchPath("PATH",resfile)
   f = .stream~new(file)
   op = f~open(read)
   if op \= "READY:" then do
      say "Resource file not found -->" resfile "("op")"
      return 0
   end

   found = 0; n = 0
   fl = f~lines
   do while found = 0
      do while n = 0 & fl > 0
         s = f~linein; fl = fl -1
         n = s~wordpos("DIALOG ")
         /* premission: #define only at the top of the RC and separated by a tab or a blank */
         if s~wordpos("#define") > 0 then do
             s = s~translate(" ", "9"x)
             parse var s "#define " symb iid
             self~ConstDir[symb~space(0)~translate] = iid
         end
      end
      if (self~checkfile(f) = 1) then return 1
      if Arg(2,'o') = 1 | dialogid = "DIALOGID" then found = 1
      else do
         if s~word(n-1)~translate = dialogid~translate then found = 1
         else if self~CheckId(s~word(n-1)) = dialogid then found = 1
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

   internopts = internopts || self~InternGetRCStyle(dlg.style, "DLG")

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

   if options~wordpos("CENTER") > 0 then
   do
      parse value GetScreenSize() with sx sy .
      dlg.x = (sx - dlg.w) % 2
      dlg.y = (sy - dlg.h) % 2
   end
   return self~create(dlg.x, dlg.y, dlg.w, dlg.h, dlg.title, internopts,, dlg.font, dlg.fsize, expected)




   /* This method creates the dialog's items using the data of the */
   /* given resource file. It is called by the 'Load' method. */
   /* */

::method LoadItems protected
   use arg resfile, dialogid, loadOptions
   if self~AktPtr = 0 then return -2

   file = SysSearchPath("PATH",resfile)
   f = .stream~new(file)
   op = f~open(read)
   if op \= "READY:" then do
      say "Resource file not found --> " op
      return 1
   end
   if Arg(3, "o") = 1 then loadOptions = ""; else loadOptions = loadOptions~translate
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
      end
      if self~checkfile(f)=1 then return 1
      if Arg(2,'o') = 1 | dialogid = "DIALOGID" then found = 1
      else do
         if s~word(n-1)~translate = dialogid~translate then found = 1
         else if self~CheckId(s~word(n-1)) = dialogid then found = 1
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

      if s~wordpos("CONTROL") > 0 then
      do
         parse var s type '"'name'"' "," id "," class "," style "," x "," y "," w "," h
         opts = ""
         class = class~strip~translate
         style = style~translate(" ", "|")
         select
            when class = '"BUTTON"' then do
               if style~wordpos("BS_AUTORADIOBUTTON") > 0 | style~wordpos("BS_RADIOBUTTON") > 0 then
                  self~InternAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "R")
               else if style~wordpos("BS_AUTOCHECKBOX") > 0 | style~wordpos("BS_CHECKBOX") > 0 then
                  self~InternAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "C")
               else if style~wordpos("BS_GROUPBOX") > 0 then
                  self~AddGroupBox(x, y, w, h, name, self~InternGetRCStyle(style, "ST"), id)
               else if style~pos("BS_OWNERDRAW") = 0 & style~wordpos("BS_AUTO3STATE") > 0 then  /* if 3state is not ownerdrawn */
                  self~InternAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "C")
               else self~InternAddRCButton(id, x, y, w, h, name, style, loadOptions)
            end

            when class = '"STATIC"' then do
               opts = self~InternGetRCStyle(style, "ST")
               select
                  when style~wordpos("SS_LEFT") > 0 then
                     self~AddText(x, y, w, h, name, opts, id)
                  when style~wordpos("SS_RIGHT") > 0 then
                     self~AddText(x, y, w, h, name, opts || " RIGHT", id)
                  when style~wordpos("SS_CENTER") > 0 then
                     self~AddText(x, y, w, h, name, opts || " CENTER", id)
                  when style~wordpos("SS_SIMPLE") > 0 then
                     self~AddText(x, y, w, h, name, opts || " SIMPLE", id)
                  when style~wordpos("SS_LEFTNOWORDWRAP") > 0 then
                     self~AddText(x, y, w, h, name, opts || " LEFTNOWRAP", id)
                  when style~wordpos("SS_WHITERECT") > 0 then
                     self~AddWhiteRect(x, y, w, h, opts, id)
                  when style~wordpos("SS_GRAYRECT") > 0 then
                     self~AddGrayRect(x, y, w, h, opts, id)
                  when style~wordpos("SS_BLACKRECT") > 0 then
                     self~AddBlackRect(x, y, w, h, opts, id)
                  when style~wordpos("SS_WHITEFRAME") > 0 then
                     self~AddWhiteFRAME(x, y, w, h, opts, id)
                  when style~wordpos("SS_GRAYFRAME") > 0 then
                     self~AddGrayFRAME(x, y, w, h, opts, id)
                  when style~wordpos("SS_BLACKFRAME") > 0 then
                     self~AddBlackFRAME(x, y, w, h, opts, id)
                  otherwise
                      ret = ErrorMessage("Item ignored due to unknown option in:" f "("s")")
               end
            end
            when class = '"EDIT"' then
               self~AddEntryLine(id, name, x, y, w, h, self~InternGetRCStyle(style, "EDIT"))

            when class = '"COMBOBOX"' then
               self~AddComboBox(id, name, x, y, w, h, self~InternGetRCStyle(style, "CB"))

            when class = '"LISTBOX"' then
               self~AddListBox(id, name, x, y, w, h, self~InternGetRCStyle(style, "LB"))

            when class = '"SCROLLBAR"' then
               self~AddScrollBar(id, x, y, w, h, self~InternGetRCStyle(style, "SB"))

            when class = '"SYSTREEVIEW32"' then
               self~AddTreeControl(id, name, x, y, w, h, self~InternGetNewCtrlStyle(style, "TREE"))

            when class = '"SYSLISTVIEW32"' then
               self~AddListControl(id, name, x, y, w, h, self~InternGetNewCtrlStyle(style, "LIST"))

            when class = '"MSCTLS_PROGRESS32"' then
               self~AddProgressBar(id, x, y, w, h, self~InternGetNewCtrlStyle(style, "PROGRESS"))

            when class = '"MSCTLS_TRACKBAR32"' then
               self~AddSliderControl(id, name, x, y, w, h, self~InternGetNewCtrlStyle(style, "SLIDER"))

            when class = '"SYSTABCONTROL32"' then
               self~AddTabControl(id, name, x, y, w, h, self~InternGetNewCtrlStyle(style, "TAB"))

            otherwise do
               self~errorFile(f, s)
               return 1
            end
         end /* select 2*/
      end
      else do
         parse var s type '"'name'"' "," id "," x "," y "," w "," h "," style
         if id~space(0) = "" then parse var s type id "," x "," y "," w "," h "," style
         type = type~strip~translate
         style = style~translate(" ", "|")

         select
            when type = "DEFPUSHBUTTON" | type = "PUSHBUTTON" then
            do
               if type~strip~left(3) = ("DEF") then style = style || " | BS_DEFPUSHBUTTON"
               self~InternAddRCButton(id, x, y, w, h, name, style, loadOptions)
            end

            when type = "EDITTEXT" then
               self~AddEntryLine(id, name, x, y, w, h, self~InternGetRCStyle(style, "EDIT"))

            when type = "LTEXT" | type = "CTEXT" | type = "RTEXT" then
            do
               opts = self~InternGetRCStyle(style, "ST")
               if type = "CTEXT" then opts = opts || " CENTER";
               else if type = "RTEXT" then opts = opts || " RIGHT";
               else if style~wordpos("SS_LEFTNOWORDWRAP") > 0 then opts = opts || " LEFTNOWRAP";
               self~AddText(x, y, w, h, name, opts, id)
            end

            when type = "GROUPBOX" then
               self~AddGroupBox(x, y, w, h, name, self~InternGetRCStyle(style, "ST"), id)

            when type = "COMBOBOX" then
               self~AddComboBox(id, name, x, y, w, h, self~InternGetRCStyle(style, "CB"))

            when type = "LISTBOX" then
               self~AddListBox(id, name, x, y, w, h, self~InternGetRCStyle(style, "LB"))

            when type = "SCROLLBAR" then
               self~AddScrollBar(id, x, y, w, h, self~InternGetRCStyle(style, "SB"))

            when type = "AUTORADIOBUTTON" | type = "RADIOBUTTON" then
               self~InternAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "R")

            when type = "AUTOCHECKBOX" | type = "CHECKBOX" then
               self~InternAddRC_RBCB(id, x, y, w, h, name, style, loadOptions, "C")

            when type = "AUTO3STATE" then
               if style~pos("BS_OWNERDRAW") > 0 then   /* sometimes AUTO3STATE is used for an owner draw button */
                   self~InternAddRCButton(id, x, y, w, h, name, style, loadOptions, "C")  /* if so, treat as normal button */
               else  /* otherwise (not owner drawn) treat as 3state */
                   self~InternAddRC_RBCB(id, x, y, w, h, name, style || " | BS_AUTO3STATE", loadOptions, "C")

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



::method CheckId
   use arg id

   if id~datatype("N") = 0 then do
      if self~ConstDir = .Nil then do
         ret = ErrorMessage("Adding dialog item: undefined non-numeric identification number!")
         return -1
      end
      else if self~ConstDir[id~space(0)~translate] = .Nil then do
          ret = ErrorMessage("Adding dialog item: undefined non-numeric identification number!")
          return -1
      end
      id = self~ResolveSymbolicId(id)
   end
   return id


::method ItemAdd
   use arg id, staticId
   if self~AktPtr = 0 then return -2
   if Arg(2,'o') = 1 then do
       id = self~CheckId(id)
       if id = -1 then return -1
   end
   else do
       if staticId = 1 then id = self~CheckId(id)
       else id = self~GetStaticID
   end
   self~DialogItemCount = self~DialogItemCount + 1
   return id


   /* This method adds a push button to the dialog and connect it with */
   /* a message that will be rised if the butten is pushed. */
   /* argument 'options' can be one of: */

::method AddButton
   use arg id, x, y, cx, cy, text, msgToRise, options
   id = self~ItemAdd(id)
   if id < 0 then return id
   options = self~GetDefaultOpts(options)
   self~AktPtr = UsrAddControl("BUT", self~AktPtr, id, x, y, cx, cy, text, options)
   if Arg(7,"o") = 0 then
      self~ConnectButton(id, msgToRise~space(0))


::method AddGroupBox
   use arg x, y, cx, cy, text, opts, id
   id = self~ItemAdd(id, \(Arg(7,"o") | id = "ID"))
   if id = -2 then return id
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("GB",self~AktPtr, x, y, cx, cy, opts, text, id)



::method AddText
   use arg x, y, cx, cy, text, opts, id
   if Arg(5, 'o') = 1 then return 1
   if Arg(3, 'o') = 1 then cx = 0
   if Arg(4, 'o') = 1 then cy = 0
   if cx = 0 | cy = 0 then do
      parse value GetStdTextSize(text) with icx icy
      if cx = 0 then cx = icx+2
      if cy = 0 then cy = icy
   end
   id = self~ItemAdd(id, \(Arg(7,"o") | id = "ID"))
   if id = -2 then return id
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("TXT", self~AktPtr, x, y, cx, cy, opts, text, id)



::method InternAddStatic
   use arg arga, type
   if arga~hasindex(6) = 0 then id = self~ItemAdd(-1, 0); else id = self~ItemAdd(arga[6],1)
   if id = -2 then return id
   opts = self~GetDefaultOpts(arga[5])
   self~AktPtr = UsrAddControl("FRM", self~AktPtr, arga[1], arga[2], arga[3], arga[4], type, opts, id)
   return 0

::method AddWhiteRect
   return self~InternAddStatic(Arg(1,"A"), 0)

::method AddGrayRect
   return self~InternAddStatic(Arg(1,"A"), 1)

::method AddBlackRect
   return self~InternAddStatic(Arg(1,"A"), 2)

::method AddWhiteFrame
   return self~InternAddStatic(Arg(1,"A"), 3)

::method AddGrayFrame
   return self~InternAddStatic(Arg(1,"A"), 4)

::method AddBlackFrame
   return self~InternAddStatic(Arg(1,"A"), 5)


::method AddEntryLine
   use arg id, fname, x, y, cx, cy, opts
   symbid = id
   id = self~ItemAdd(id)
   if id < 0 then return id
   if Arg(2,'o') = 1 then fname = "DATA"id
   if Arg(6,'o') = 1 then cy = 0
   if cy = 0 then do
      parse value GetStdTextSize("Tg") with . cy
      cy = cy +4
   end
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("EL", self~AktPtr, id, x, y, cx, cy, opts)
   if self~AutoDetect = 1 then self~ConnectEntryLine(symbid, fname)


::method AddPasswordLine
   newarg = Arg(1, 'A')
   if Arg(7,'o') = 1 then newarg[7] = ""
   newarg[7] = newarg[7] || " PASSWORD"
   forward message "AddEntryLine" Arguments (newarg)


::method AddListBox
   use arg id, fname, x, y, cx, cy, opts
   symbid = id
   id = self~ItemAdd(id)
   if id < 0 then return id
   if Arg(2,'o') = 1 then fname = "DATA"id
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("LB", self~AktPtr, id, x, y, cx, cy, opts)
   if self~AutoDetect = 1 then do
      if opts~wordpos("MULTI") > 0 then self~ConnectMultiListBox(symbid, fname)
      else self~ConnectListBox(symbid, fname)
   end


::method AddComboBox
   use arg id, fname, x, y, cx, cy, opts
   symbid = id
   id = self~ItemAdd(id)
   if id < 0 then return id
   if Arg(2,'o') = 1 then fname = "DATA"id
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("CB", self~AktPtr, id, x, y, cx, cy, opts)
   if opts~wordpos("CAT") = 0 & self~AutoDetect = 1 then self~ConnectComboBox(symbid, fname, opts)


::method AddCheckBox
   use arg id, fname, x, y, cx, cy, text, opts
   symbid = id
   id = self~ItemAdd(id)
   if id < 0 then return id
   if Arg(2,'o') = 1 then fname = text
   if Arg(5, 'o') = 1 then cx = 0
   if Arg(6, 'o') = 1 then cy = 0
   if cx = 0 | cy = 0 then do
      parse value GetStdTextSize(text) with icx icy
      if cx = 0 then cx = icx + 12
      if cy = 0 then cy = icy
   end
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("CH", self~AktPtr, id, x, y, cx, cy, text, opts)
   if self~AutoDetect = 1 then self~ConnectCheckBox(symbid, fname)


::method AddRadioButton
   use arg id, fname, x, y, cx, cy, text, opts
   symbid = id
   id = self~ItemAdd(id)
   if id < 0 then return id
   if Arg(2,'o') = 1 then fname = text
   if Arg(5, 'o') = 1 then cx = 0
   if Arg(6, 'o') = 1 then cy = 0
   if cx = 0 | cy = 0 then do
      parse value GetStdTextSize(text) with icx icy
      if cx=0 then cx=icx + 12
      if cy=0 then cy=icy
   end
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("RB", self~AktPtr, id, x, y, cx, cy, text, opts)
   if opts~wordpos("CAT") = 0 & self~AutoDetect = 1 then self~ConnectRadioButton(symbid, fname)


::method AddScrollBar
   use arg id, x, y, cx, cy, opts
   id = self~ItemAdd(id)
   if id < 0 then return id
   opts = self~GetDefaultOpts(opts)
   self~AktPtr = UsrAddControl("SB", self~AktPtr, id, x, y, cx, cy, opts)


::method AddRadioGroup
   arr = Arg(1,"A")
   arr[8] = "R"
   forward message "InternAddRBCBGroup" Arguments (arr)

::method AddCheckGroup
   arr = Arg(1,"A")
   arr[8] = "C"
   forward message "InternAddRBCBGroup" Arguments (arr)

::method InternAddRBCBGroup
   use arg startid, x, y, cx, inp, opts, idstat, kind

   if self~AktPtr = 0 then return -2
   if self~CheckID(startid) = -1 then return
   if Arg(4, 'o') = 1 then cx = 0
   parse var inp rb inp
   parse value GetStdTextSize(rb) with len stepy
   lastlen = len; oldy = stepy; stepy = trunc(stepy * 1.5)
   opts = self~GetDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then my = y + oldy; else my = y
   newopts = opts~changestr("NOBORDER","")
   if cx \= 0 then len = cx-20
   if Arg(7,'o') = 1 then idstat = self~GetStaticID(idstat)

   i = 0
   do while rb \= ""
      if kind = "R" then do
          if i = 0 then self~AddRadioButton(startid+i,, x+4, my, len+12, stepy, rb, newopts || " GROUP")
          else self~AddRadioButton(startid + i,, x+4, my, len+12, stepy, rb, newopts)
      end; else self~AddCheckBox(startid + i,, x+4, my, len+12, stepy, rb, newopts)
      my = my + stepy; i = i + 1
      parse var inp rb inp
      if cx = 0 then do
         parse value GetStdTextSize(rb) with len .
         if len > lastlen then lastlen = len
      end
   end
   if cx \= 0 then LastLen = cx-20
   if nob = 0 then self~AddGroupBox(x,y,lastlen+20,my-y+trunc(oldy / 2),"", newopts, idstat)


::method AddInput
   use arg id, fname, x, y, cx1, cx2, cy, text, opts, idstat
   if self~AktPtr = 0 then return -2
   if self~CheckID(id) = -1 then return
   if Arg(2,'o') = 1 then fname = text
   if Arg(5, 'o') = 1 then cx1 = 0
   if Arg(7, 'o') = 1 then cy = 0
   if cy = 0 | cx1 = 0 then do
      parse value GetStdTextSize(text) with stepx icy
      if cy = 0 then cy = icy + 4
      if cx1 = 0 then cx1 = stepx + 4
   end
   if Arg(10,'o') = 1 then idstat = self~GetStaticID(idstat)
   opts = self~GetDefaultOpts(opts)
   self~AddText(x, y+2, cx1, cy, text, opts, idstat)
   self~AddEntryLine(id, fname, x+cx1+1, y, cx2, cy, opts)


::method AddInputGroup
   use arg startid, x, y, cx1, cx2, inp, opts, idstat
   parse var inp rb inp

   if self~AktPtr = 0 then return -2
   if self~CheckID(startid) = -1 then return
   if Arg(4, 'o') = 1 then cx1 = 0
   count = 0; ll = 0
   do while rb \= ""
      count = count + 1
      data.count = rb
      if cx1 = 0 then do
         parse value GetStdTextSize(rb) with len .
         if len > ll then ll = len
      end
      parse var inp rb inp
   end
   if cx1 = 0 then cx1 = ll+4
   if Arg(8,'o') = 1 then idstat = self~GetStaticID(idstat)
   is = idstat

   parse value GetStdTextSize("Tg") with stepx stepy
   opts = self~GetDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then my = y + trunc(stepy * 1.25); else my = y
   newopts = opts~changestr("NOBORDER", "")
   do i=1 to count
      self~AddInput(startid + i -1,, x+4, my, cx1-4, cx2-6, stepy+4, data.i, newopts, is)
      if is \= -1 then is = is + 1
      my = my + trunc(stepy * 2.25)
   end
   if nob = 0 then self~AddGroupBox(x,y,cx1+cx2,my-y,"", newopts, is)


::method AddComboInput
   use arg id, attname, x, y, cx1, cx2, clines, text, opts, idstat

   if self~AktPtr = 0 then return -2
   if self~CheckID(id) = -1 then return
   if Arg(2, 'o') = 1 then attname = text
   if Arg(5, 'o') = 1 then cx1 = 0
   if Arg(7, 'o') = 1 then clines = 5

   parse value GetStdTextSize(text) with stepx cy
   cy = cy + 4
   if cx1 = 0 then cx1 = stepx + 4
   if Arg(10,'o') = 1 then idstat = self~GetStaticID(idstat)
   opts = self~GetDefaultOpts(opts)
   self~AddText(x, y, cx1, cy, text, opts, idstat)
   self~AddComboBox(id, attname, x+cx1+2, y, cx2, cy*clines, opts)


::method AddInputStem
   use arg startid, x, y, cx1, cx2, inp., opts, idstat

   if self~AktPtr = 0 then return -2
   if self~CheckID(startid)= -1 then return
   if Arg(4, 'o') = 1 then cx1 = 0
   count = 0; ll = 0
   do while var("inp."count+1) = 1
      count = count + 1
      if cx1 = 0 then do
         parse value GetStdTextSize(inp.count) with len .
         if len > ll then ll = len
      end
   end
   if cx1 = 0 then cx1 = ll+4

   if idstat = "IDSTAT" then idstat = self~GetStaticID(idstat)
   is = idstat
   parse value GetStdTextSize("Tg") with stepx stepy

   opts = self~GetDefaultOpts(opts)
   nob = opts~wordpos("NOBORDER")
   if nob = 0 then my = y + trunc(stepy * 1.25); else my = y
   newopts = opts~changestr("NOBORDER", "")
   do i=1 to count
      self~AddInput(startid + i -1,, x+4, my, cx1-4, cx2-6, stepy+4, inp.i, newopts, is)
      if is \= -1 then is = is + 2
      my = my + trunc(stepy * 2.25)
   end
   if nob = 0 then self~AddGroupBox(x,y,cx1+cx2,my-y,"", newopts, is)


::method AddCheckBoxStem
   arr = Arg(1,"A")
   arr[11] = "C"
   forward message "InternAddRBCBStem" Arguments (arr)

::method AddRadioStem
   arr = Arg(1,"A")
   arr[11] = "R"
   forward message "InternAddRBCBStem" Arguments (arr)

::method InternAddRBCBStem private
   use arg startid, x, y, cxx, inp., max, opts, idstat, fn, fs, kind
   if self~AktPtr = 0 then return -2
   if self~CheckID(startid) = -1 then return
   if Arg(4, 'o') = 1 then cxx = 0
   count = 0; ll = 0
   do while var("inp."count+1) = 1
      count = count + 1
      if cxx = 0 then do
         if Arg(9,'o') = 1 then ret = GetStdTextSize(inp.count)
         else ret = GetStdTextSize(inp.count, fn, fs)
         parse var ret len .
         if len > ll then ll = len
      end
   end
   if var("max") = 0 | max = 0 then max = count
   if cxx = 0 & max \= 1 then cx = ll+20; else cx = cxx
   if Arg(8, 'o') = 1 then idstat = self~GetStaticID(idstat)
   if Arg(9,'o') = 1 then ret = GetStdTextSize("Tg"); else ret = GetStdTextSize("Tg", fn, fs)
   parse var ret stepx oldy
   mx = 0; highy = 0
   opts = self~GetDefaultOpts(opts)
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
         if Arg(9,'o') = 1 then ret = GetStdTextSize(inp.i)
         else ret = GetStdTextSize(inp.i, fn, fs)
         parse var ret cx .
         cx = cx + 24
      end
      if kind = "R" then
      do
         if i = 1 then tmpopts = newopts || " GROUP"; else tmpopts = newopts
         self~AddRadioButton(startid + i -1,, x+mx+4, my, cx-8, stepy, inp.i, tmpopts)
      end; else self~AddCheckBox(startid + i -1,, x+mx+4, my, cx-8, stepy, inp.i, newopts)
      my = my + stepy
   end
   if highy = 0 then highy = my
   if nob = 0 then self~AddGroupBox(x,y,mx+cx, highy-y+(stepy % 2),"", newopts, idstat)


::method AddButtonGroup
   use arg x, y, cx, cy, inp, bottom, opts
   parse var inp bt bid bfnk newinp

   if self~AktPtr = 0 then return -2
   if Arg(6,"o") = 1 then bottom = 0
   if cx = 0 | Arg(3,"o") = 1 then cx = 40
   if cy = 0 | Arg(4,"o") = 1 then cy = 12

   opts = self~GetDefaultOpts(opts)
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



::method AddOkCancelRightBottom
   self~AddButtonGroup(self~SizeX-95, self~SizeY - 15,,, "&Ok 1 OK &Cancel 2 CANCEL", 1, "FIX DEFAULT")

::method AddOkCancelLeftBottom
   self~AddButtonGroup(5, self~SizeY - 15,,, "&Ok 1 OK &Cancel 2 CANCEL", 1, "FIX DEFAULT")

::method AddOkCancelRightTop
   self~AddButtonGroup(self~SizeX-45, 5,,, "&Ok 1 OK &Cancel 2 CANCEL", 0, "FIX DEFAULT")

::method AddOkCancelLeftTop
   self~AddButtonGroup(5, 5,,, "&Ok 1 OK &Cancel 2 CANCEL", 0, "FIX DEFAULT")


::method GetDefaultOpts protected
   use arg opts
   if opts = .NIL | opts = "OPTS" | opts = "OPTIONS" then return ""
   else return opts~translate


::method GetStaticID protected
   return -1


::method StartIt
   use arg icon, modal

   if Arg(1,"o") = 1 | icon~Datatype("N") = 0 then icon = 0
   if Arg(2,"o") = 1 then modal = 0; else if modal~translate = "NOTMODAL" then modal = 1
   if self~BasePtr = 0 then return 0
   self~DlgHandle = UsrCreateDialog(self~Adm, "PARENT", self~DialogItemCount, 0, self~BasePtr, 0 /* no autodetection */, 1, icon, modal)
   self~AktPtr = 0
   if self~DlgHandle = 0 then return 0
   else self~InitDialog
   return self~DlgHandle

::method StopDynamic protected
   self~DialogItemCount = 0
   self~BasePtr = 0
   return ret


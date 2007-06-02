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
/* User Dialog Class                                                          */
/*                                                                            */
/******************************************************************************/
/* include the base dialog class */
::requires "basedlg.cls"
::requires "dyndlg.cls"


/* This class extends the 'BaseDialog' class */
::class 'UserDialog' subclass BaseDialog public inherit DynamicDialog

  /* The init method */
::method Init
   select
      when Arg() >= 2 then InitRet = self~init:super(0, 0, Arg(1),Arg(2))
      when Arg() == 1 then InitRet = self~init:super(0, 0, Arg(1))
      otherwise InitRet = self~init:super
   end

   if InitRet = 0 then forward message "DynamicInit"
   else return InitRet

::method StopIt
   forward class (super) continue
   forward message "StopDynamic" continue
   ret = result
   if self~BkgBrushBmp \= 0 then self~DeleteObject(self~BkgBrushBmp)
   if self~BkgBitmap \= 0 then self~RemoveBitmap(self~BkgBitmap)
   return ret

/******************************************************************************/
/* Menu/Action bar */

::method SetMenu
    if self~MenuMemHandle <> 0 then
       return UsrMenu("SET", self~DlgHandle, self~MenuMemHandle, self~MenuBasePtr)
    else return 1

::method CreateMenu
    use arg count
    if arg(1,'o') = 1 then count = 100
    parse value UsrMenu("INIT", count) with hmem base curptr

    self~MenuBasePtr = base
    self~MenuAktPtr = curptr
    self~MenuMemHandle = hmem
    return 0;


::method AddPopupMenu
    use arg name, opt
    if arg(2,'o') = 1 then opt = ""
    self~MenuAktPtr = UsrMenu("ADD", self~MenuAktPtr, 0, name, "POPUP" || opt)

::method AddMenuItem
    use arg name, id, opt, msgToRaise
   id = self~CheckId(id)
   if  id = -1 then return
    if arg(3,'o') = 1 then opt = ""
    self~MenuAktPtr = UsrMenu("ADD", self~MenuAktPtr, id, name, opt)
    if arg(4,'o') = 0 then self~ConnectMenuItem(id, msgToRaise)


::method AddMenuSeparator
    self~MenuAktPtr = UsrMenu("ADD", self~MenuAktPtr, 0, "", "SEPARATOR")


::method LoadMenu protected
   use arg resfile, menuid, loadopts, count
   if arg(4,'o') = 1 then count = 50
   if arg(3,'o') = 1 then loadopts = ""; else loadopts = loadopts~Translate

   file = SysSearchPath("PATH",resfile)
   f = .stream~new(file)
   op = f~open(read)
   if op \= "READY:" then do
      say "Resource file not found --> " op
      return 1
   end

   found = 0; n = 0
   fl = f~lines
   do while found = 0 & fl > 0
      do while n = 0 & fl > 0
         s = f~linein; fl = fl - 1
         /* premission: #define only at the top of the RC and separated by a tab or a blank */
         /* need this here as well because menu resource could be in separate file */
         if s~wordpos("#define") > 0 then do
            s = s~translate(" ", "9"x)
            parse var s "#define " symb iid
            self~ConstDir[symb~space(0)~translate] = iid
         end
         n = s~wordpos("MENU")
      end

      if (self~checkfile(f) = 1) then return 1

      if Arg(2,'o') = 1 | menuid = "MENUID" then found = 1
      else do
         if s~word(n-1)~translate = menuid~translate then found = 1
         else if self~CheckId(s~word(n-1)) = menuid then found = 1
         else n = 0
      end
   end

   s = f~linein; fl = fl - 1
   do while s~wordpos("BEGIN") = 0 & s~pos("{") = 0 & fl > 0
      s = f~linein; fl = fl - 1
   end

   if (self~checkfile(f) = 1) then return 1

   rcarray = .array~new(50)
   bracket = 1
   cur = 0
   endcount = 0

   prevs = ""
   s = f~linein; fl = fl - 1
   do while bracket > 0 & fl >= 0
      if s~wordpos("END") > 0 | s~pos("}") > 0 then
      do
         bracket = bracket - 1;
         endcount = endcount + 1
         cur = cur + 1
         rcarray[cur] = s
      end
      else if s~wordpos("BEGIN") > 0 | s~pos("{") > 0 then do
         bracket = bracket + 1;
         cur = cur + 1
         rcarray[cur] = s
      end
      else if s~strip \= "" then do
         cur = cur + 1
         rcarray[cur] = s
      end
      s = f~linein; fl = fl - 1
   end

   f~close
   arrcount = cur;

   if (count < cur - endcount) then count = cur - endcount +5;

   if self~CreateMenu(count) <> 0 then return 1

   self~ProcessingLoad = 1

   do i = 1 to arrcount
      s = rcarray[i]

      select
         when s~wordpos("POPUP") > 0 then
         do
            parse var s type '"'name'"'    "," opt

            j = i + 1;
            bracket = 0
            do until bracket = 0
              if rcarray[j]~wordpos("BEGIN") > 0 | rcarray[j]~pos("{") > 0 then
                bracket = bracket + 1
              else if rcarray[j]~wordpos("END") > 0 | rcarray[j]~pos("}") > 0 then
                bracket = bracket - 1
              j = j +1
            end

            if rcarray[j]~wordpos("END") > 0 | rcarray[j]~pos("}") > 0 then opt = opt || "END"

            self~AddPopupMenu(name, opt)
         end

         when s~wordpos("SEPARATOR") > 0 then
         do
            parse var s type sep opt
            if rcarray[i+1]~wordpos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
             self~AddMenuSeparator(opt)
         end

         when s~wordpos("MENUITEM") > 0 then
         do
            parse var s type '"'name'"'    "," id "," opt
            if rcarray[i+1]~wordpos("END") > 0 | rcarray[i+1]~pos("}") > 0 then opt = opt || " END"
            if (loadopts~wordpos("CONNECTITEMS") > 0) then
            do
               amper = name~pos('&')
               if amper > 0 then name = name~delstr(amper, 1)
               self~AddMenuItem(name, id, opt, name~space(0))
            end
            else
               self~AddMenuItem(name, id, opt)
         end
         when s~wordpos("END") > 0 | s~pos("}") > 0 | s~wordpos("BEGIN") > 0 | s~pos("{") > 0 then nop;

         otherwise do
            self~errorFile(f, s)
            return 1
         end
     end /*select 1*/
  end /* do while */
  self~ProcessingLoad = 0

  return 0

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2009 Rexx Language Association. All rights reserved.    */
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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * The DlgArea and DlgAreaU classes.
 */

/* ========================================================================= */
::class 'DlgArea' public
/* ========================================================================= */
/*  define an area of your dialog to place controls on                       */
/*                                                                           */
/*  parms x, y, width, height [,margin]                                      */
/*                                                                           */
/*  x & y take offset as an argument,                                        */
/*  a -ve argument is from the right/bottom                                  */
/*                                                                           */
/*  methods & attributes                                                     */
/*  x()        offset from left (right if -ve)                               */
/*  y()        offset from top  (bottom if -ve)                              */
/*  w()        width                                                         */
/*  h()        height                                                        */
/*  wr         width remaining                                               */
/*  hr         height remaining                                              */
/*  l          left    }                                                     */
/*  t          top     }  Dimensions                                         */
/*  b          bottom  }  within margins                                     */
/*  r          right   }                                                     */
/*  move()     Specify new x,y coordinates                                   */
/*  resize()   Specify new width and height                                  */
/*                                                                           */
/*                                                                           */
/*  methods                                                                  */
/*  x & y  can take an offset argument which may be absolute or a percentage */
/*  w & h  can take a percentage as argument                                 */
/*         a parm of R to W or H means the remainder of the width or height  */
/*                                                                           */
/*  so to add a button 80% across your margined dialog for the remainder     */
/*  a = .DlgArea~new(10,10,150,110,5)                                        */
/*  self~createPushButton(a~x(80%),10,a~w(20%),a~h(10%), ,'Text','Method')   */
/*    or not using percentages is equivalent to                              */
/*  self~createPushButton(a~x(112),10,a~w * 0.2,a~h(10), ,'Text','Method')   */
/*    or not using the object would be equivalent to                         */
/*  self~createPushButton(117, 10, 28, 10, , 'Text','Method')                */
/*                                                                           */
/* ------------------------------------------------------------------------- */
/*               :                                  ^                        */
/*              top                                 :                        */
/*              <v.......width.............>     bottom                      */
/*              +--------------------------+   ^    :                        */
/* ......left..>|                       r  |   :    :                        */
/*              |                       v  |   :    :                        */
/*              |  +--------------------+  |   :    :                        */
/*              |  |+.x.>             ^ |  |   :    :                        */
/*              |  |:                 : |  |   :    :                        */
/*              |  |y                 : |  | height :                        */
/*              |  |:                 h |  |   :    :                        */
/*              |  |v                 : |  |   :    :                        */
/*              |  |<........w........+>|  |   :    :                        */
/*              |  |                  v |  |   :    :                        */
/*             ^|b>+--------------------+  |   :    :                        */
/*             m|                          |   :    :                        */
/*             v+--------------------------+   v    v                        */
/*              <m>                                                          */
/*.....right...............................>                                 */
/*                                                                           */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
::method Init
/* ------------------------------------------------------------------------- */
arg left,top,width,height,margin

if margin~datatype('n')
then self~Margin = margin~trunc
else self~margin = max(min(height % 7,5),1)

if top~datatype('n')
then self~top       = top~trunc
else self~top       = 10

if left~datatype('n')
then self~left      = left~trunc
else self~left      = 10

if width~datatype('n')
then self~width     = width~trunc
else self~width     = 100

if height~datatype('n')
then self~height    = height~trunc
else self~height    = 100


self~T = self~Top  + self~Margin
self~L = self~left + self~Margin
self~B = self~top  + self~height - self~margin
self~R = self~Left + self~Width  - self~Margin

self~LastX = 0
self~LastY = 0

/* ------------------------------------------------------------------------- */
::method X
/* ------------------------------------------------------------------------- */
arg offset
self~LastX='0'||Offset

if offset~pos('%') > 0
then do
   offset = offset~changestr('%','')
   if offset~datatype('n')
   then do
      if offset < 0 then offset = 100 + offset          /* from right margin */
      return trunc(self~left + self~margin + (self~w * (offset / 100)))
   end
   else return self~left + self~margin
end
else if offset~datatype('n')
     then if offset >= 0
          then return trunc(self~left + self~margin + offset) /* x from left */
          else return trunc(self~left + self~width - self~margin + offset) /* right */
else return self~left + self~margin

/* ------------------------------------------------------------------------- */
::method Y
/* ------------------------------------------------------------------------- */
arg offset
self~LastY = '0'||offset

if offset~pos('%') > 0
then do
   offset = offset~changestr('%','')
   if offset~datatype('n')
   then do
      if offset < 0 then offset = 100 + offset          /* from right margin */
      return trunc(self~top + self~margin + (self~h * (offset / 100)))
   end
   else return self~top + self~margin
end
else if offset~datatype('n')
     then if offset >= 0
          then return trunc(self~top + self~margin + offset)
          else return trunc(self~top + self~height - self~margin + offset)
else return self~top + self~margin

/* ------------------------------------------------------------------------- */
::method H
/* ------------------------------------------------------------------------- */
arg pc

if pc = 'R' then return self~hr                       /* height is remainder */

h = self~height - (2 * self~margin)

if pc~pos('%') > 0
then return trunc(h * (pc~changestr('%','') / 100))
else return trunc(h)

/* ------------------------------------------------------------------------- */
::method HR
/* ------------------------------------------------------------------------- */

if self~LastY~pos('%') = 0                                 /* Y was absolute */
then return self~Top + self~Height - self~Margin - self~LastY
else return self~h(100 - self~LastY~changestr('%','')||'%')       /* Y was % */

/* ------------------------------------------------------------------------- */
::method W
/* ------------------------------------------------------------------------- */
arg pc

if pc = 'R' then return self~wr                       /* width  is remainder */

w = self~width - (2 * self~margin)

if pc~pos('%') > 0
then return trunc(w * (pc~changestr('%','') / 100))
else return w

/* ------------------------------------------------------------------------- */
::method WR
/* ------------------------------------------------------------------------- */

if self~LastX~pos('%') = 0                                 /* X was absolute */
then return self~Left + self~Width - self~Margin - self~LastX
else return self~w(100 - self~LastX~changestr('%','')||'%')       /* X was % */

/* ----------------------- compatibility methods --------------------------- */
::method cx    ; return self~w(arg(1))         -- Compatibility version of w
::method cy    ; return self~h(arg(1))         -- Compatibility version of h
::method sizeX ; return self~width
::method sizeY ; return self~height
/* ------------------------------------------------------------------------- */
::method T              Attribute
::method L              Attribute
::method B              Attribute
::method R              Attribute
::method Top            Attribute
::method Left           Attribute
::method Width          Attribute
::method Height         Attribute
::method Margin         Attribute
::method LastX          Attribute Private
::method LastY          Attribute Private

/* ========================================================================= */
::Class 'DlgAreaU' SubClass DlgArea Public -- DlgAreaUniversal - Whole of Dialog
/* ========================================================================= */
/*  define the Area of the calling Dialog                                    */
/*                                                                           */
/*  parms callingDialog [,margin]                                            */
/*                                                                           */
/*  methods & attributes                                                     */
/*  resize     Aids in Dynamically Resizing Dialogs - See Documentation      */
/*  CorrectionFactor                                - See Documentation      */
/*                                                                           */
::method Init
/* ------------------------------------------------------------------------- */
Expose DlgObjList

use arg DlgObj,Margin,NoResize,NoMove

if NoResize~String \= 'a Set' then NoResize = .Set~new
self~NoResize = NoResize
if NoMove~String   \= 'a Set' then NoMove   = .Set~new
self~NoMove   = NoMove

if \margin~datatype('n') then margin = 5

self~CorrectionFactor = 1.05

self~init:super(0,0,DlgObj~sizeX,DlgObj~sizeY,margin)

self~OriginalWidth  = DlgObj~sizeX
self~OriginalHeight = DlgObj~sizeY
self~LastError      = .nil

                                  /* Analyze the callers defineDialog Method */
                                       /* Remove comments prefixed by --     */
DDSourceArr=DlgObj~class~method('defineDialog')~source
DDSBuff=.mutableBuffer~new
do line over DDSourceArr
   parse var line line '--' .       -- strip out this style inline comments
   DDSBuff~Append(' '||line)
end /* DO */
DDSource = DDSBuff~String ; Drop DDSBuff
                                       /* remove comments within /*  */      */
do forever
   commEnd = DDsource~pos('*'||'/')
   if commEnd = 0 then leave
   commStart = DDSource~SubStr(1,CommEnd-1)~lastPos('/'||'*')
   if comStart=0
   then do
      self~LastError='Error in DlgAreaU Class: Unbalanced comments in defineDialog class of' Dlg
      leave
   end /* DO */

   parse var DDSource front =(commStart) . =(commEnd) . +2 back
   DDSource = front back
end /* DO */

DDSource1 = DDSource           /* copy of de-commented defineDialog source   */

                               /* re-create DlgArea objects within scope     */
do forever
   parse upper var DDSource1 front '.DLGAREA'DAMsg'('DAparms')'DDSource1
   do while DAParms~Countstr('(') > DAParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      DAParms = DAParms||')'||fragment
   end /* DO */
   if DAParms = '' then leave          /* we have got them all!              */

                                     /* parse out the refferer variable name */
   parse value front~reverse with . '='DAVar .
   DAVar = DAVar~reverse~strip
   DAObj = DAVar
   sourceline = DAVar'=.DLGAREA'||DAMsg||'('||DAParms||')'
   select
      when DAMsg~Space(0) = 'U~NEW'  /* This is the DlgAreaU (self) obj def  */
         then interpret DAVar'=.DlgArea~new(0,0,'||self~w||','||self~h||','||self~margin||')'
      when DAMsg~Space(0) = '~NEW'   /* This is a DlgArea Obj definition     */
         then interpret DAVar'=.DlgArea~new('daparms')'
      otherwise
         self~LastError='DlgAreaU Class: Unable to parse:' SourceLine
   end /* select */
end

                                    /* Now we parse out resizable widgets    */
DlgObjList=.List~new
delim = '00'x

signal on syntax name parseerror
Parseerroroccured = 1 /* provisionally */

do forever
   parse upper var DDSource front 'ADD'ObjName'('ObjParms')'DDSource1
   do while ObjParms~Countstr('(') > ObjParms~CountStr(')')
      parse var DDSource1 fragment ')' DDSource1
      ObjParms = ObjParms||')' fragment
   end /* DO */

   if ObjParms = '' then leave
   if front~space(0)~right(5)='SELF~'
   then do
      sourcelinetxt = 'SELF~ADD'||Objname||'('||Objparms||')'
      DDSource=DDSource1
      select
         when 'BITMAPBUTTON BUTTON PROGRESSBAR SCROLLBAR'~Wordpos(ObjName~strip) >0
            then parse var ObjParms ObjId','ObjX','ObjY','ObjW','ObjH','.
         when 'CHECKBOX COMBOBOX ENTRYLINE LISTBOX LISTCONTROL PASSWORDLINE RADIOBUTTON SLIDERCONTROL TABCONTROL'~Wordpos(ObjName~strip) > 0
            then parse var ObjParms ObjId','.','ObjX','ObjY','ObjW','ObjH','.
         when 'BLACKFRAME BLACKRECT GRAYRECT GRAYFRAME TREECONTROL WHITEFRAME WHITERECT'~Wordpos(ObjName~strip) > 0
            then parse var ObjParms ObjX','ObjY','ObjW','ObjH','.','ObjId','.
         when 'GROUPBOX TEXT'~Wordpos(ObjName~strip) > 0
            then parse var ObjParms ObjX','ObjY','ObjW','ObjH','.','.','ObjId','.
         otherwise
            ObjId = ''                    /* This type is on the ignore list */
      end /* select */
      if ObjId \= ''
      then interpret 'DlgObjList~Insert(ObjId "@"' ObjX '"@"' ObjY '"@"' ObjW '"@"' ObjH ')'
   end /* DO */
   else parse var DDSource 'ADD'DDSource            /* skip past this add */
end /* DO */

Parseerroroccured = 0
parseerror:
signal off syntax

if parseerroroccured then self~lastError = 'DlgAreaU class could not parse' sourcelinetxt

/* ------------------------------- ignore list -----------------------------
ATTRIBUTE AUTOSTARTMETHOD BUTTONGROUP CATEGORYCOMBOENTRY CATEGORYLISTENTRY
CHECKBOXSTEM CHECKGROUP COMBOENTRY COMBOINPUT FULLSEQ INPUT INPUTGROUP
INPUTSTEM LISTENTRY MENUITEM MENUSEPERATOR OKCANCELLEFTBOTTOM OKCANCELLEFTTOP
OKCANCELRIGHTBOTTOM OKCANCELREGHTTOP POPUPMENU RADIOGROUP RADIOSTEM ROW
SEQUENCE STYLE USERMSG
   ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
::method Resize
/* ------------------------------------------------------------------------- */
/* this method needs to be called as follows:                                */
/* in your dialog init method place this line after call to the superclass   */
/* self~connectResize('OnResize')                                            */
/* the method resize would contain the following                             */
/* expose u /* our topmost dlgAreaClass exposed in defineDialog */           */
/* use arg dummy, sizeinfo    /* sizeinfo contains information about the     */
/*                                                  new width and height */  */
/* if self~PeekDialogMessage~left(8)\="OnResize" then u~resize(self,sizeinfo)*/
/* ------------------------------------------------------------------------- */
expose DlgObjList
  use arg dlg,data
  dlg~sizeX = .DlgUtil~loWord(data) % dlg~factorX
  dlg~sizeY = .DlgUtil~hiWord(data) % dlg~factorY

  wFactor   = (dlg~sizeX / self~OriginalWidth ) * self~CorrectionFactor
  hFactor   = (dlg~sizeY / self~OriginalHeight) * self~CorrectionFactor


  do dlgObjDef over DlgObjList
     parse var dlgObjdef DOid'@'DOx'@'DOy'@'DOw'@'DOh
     if \self~NoResize~HasIndex(DOid~strip)
     then dlg~ResizeItem(Doid,DOw * wFactor,DOh * hFactor,"NOREDRAW")
     if \self~NoMove~HasIndex(DOid)
     then dlg~MoveItem(DOid,DOx * wFactor,DOy * hFactor,"NOREDRAW")
  end /* DO */
  dlg~update
/* ------------------------------------------------------------------------- */

::method OriginalWidth    Attribute Private
::method OriginalHeight   Attribute Private
::method NoResize         Attribute
::method NoMove           Attribute
::method CorrectionFactor Attribute
::method LastError        Attribute

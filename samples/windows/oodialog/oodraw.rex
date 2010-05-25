/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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
 *  oodraw.rex, an ooDialog sample program.
 *
 *  This example demonstrates how to do custom drawing in ooDialog.  It uses
 *  a large button as the drawing surface.
 */

 d = .drawDlg~new( , 'ooDraw.h')
 if d~initCode <> 0 then do
    say 'The Draw dialog was not created correctly.  Aborting.'
    return d~initCode
 end
 d~Execute("SHOWTOP")

 return 0

/*---------------------------------- requires ------------------------*/

::requires "ooDialog.cls"

/*---------------------------------- dialog class --------------------*/

::class 'drawDlg' subclass UserDialog

::attribute interrupted unguarded

::method init
   expose colornames

   forward class (super) continue

   colornames = .array~of('dark red', 'dark green', 'dark yellow', 'dark blue', -
                          'purple', 'blue grey', 'light grey', 'pale green',    -
                          'light blue', 'white', 'grey', 'dark grey', 'red',    -
                          'light green', 'yellow', 'blue', 'pink', 'turquoise')
   self~interrupted = .true

   if \ self~createcenter(200, 235, "ooDialog Color Drawing Demonstration", , , "System", 8) then
      self~initCode = 1

::method defineDialog

   self~createPushButton(IDC_PB_DRAW, 0,0,200,200, "DISABLED NOTAB")  -- The drawing surface.

   self~createPushButton(IDC_PB_BRUSH_RECTANGLES,   5, 205, 55, 12, ,"&Rectangles","brushRectangles")
   self~createPushButton(IDC_PB_PEN_RECTANGLES,    62, 205, 55, 12, ,"&Pen Rectangles","penRectangles")
   self~createPushButton(IDC_PB_PIXELS,           119, 205, 36, 12, ,"Pi&xels","pixels")
   self~createPushButton(IDC_PB_STAR_LINES,         5, 220, 36, 12, ,"S&tar","starLines")
   self~createPushButton(IDC_PB_RANDOM_LINES,      43, 220, 36, 12, ,"&Lines","randomLines")
   self~createPushButton(IDC_PB_RANDOM_SQUARES,    81, 220, 36, 12, ,"&Squares","randomSquares")
   self~createPushButton(IDC_PB_ELLIPSES,         119, 220, 36, 12, ,"&Ellipses","circleEllipses")

   self~createPushButton(IDC_PB_INTERRUPT, 160, 205, 35, 12, "DEFAULT", "&Interrupt", "interrupt")
   self~createPushButton(IDCANCEL, 160, 220, 35, 12, ,"&Cancel")

   do i=5 to 49
      self~createBlackRect(-1, 2*i, 2*i, 200-4*i, 200-4*i, "BORDER")
   end
   do i=51 to 99
      self~createWhiteRect(-1, i+25, i+25, 200-2*i, 200-2*i, "BORDER")
   end

::method initDialog unguarded
   expose x y dc myBrush myPen recThick linthick ranthick kpix change sysFont oldFont

   self~disableControl(IDC_PB_INTERRUPT)                    /* cannot interrupt yet */
   change = 0
   recThick = 0; linthick = 1; ranthick=0; kpix = 1;
   x = self~factorx
   y = self~factory

   dc = self~getButtonDC(IDC_PB_DRAW)
   myBrush = self~createBrush(10)         /* white      */
   myPen   = self~createPen(1,'solid',0)  /* thin black */
   sysFont = self~createFontEx("System",10)
   oldFont = self~FontToDC(dc,sysFont)
   self~writeToButton(IDC_PB_DRAW,45*x,30*y,"Black rectangles","Arial",12,"BOLD")
   self~writeToButton(IDC_PB_DRAW,80*x,80*y,"White rectangles","Arial",12,"BOLD")

::method interrupt unguarded
   self~interrupted = .true
   self~disableControl(IDC_PB_INTERRUPT)

::method cancel unguarded   -- Stop the drawing program and quit.
   expose x y dc myBrush myPen sysFont oldFont

   drawSurface = self~newPushButton(IDC_PB_DRAW)
   self~interrupted = .true
   j=msSleep(1)

   opts = .directory~new
   opts~weight = 700
   tmpFont = self~CreateFontEx("Arial",28,opts)
   xFont = self~FontToDC(dc,tmpFont)

   do i = 2 to 6
      if i // 2 = 1 then self~TransparentText(dc)
      else self~OpaqueText(dc)
      self~writeDirect(dc, 30 * x, 25 * i * y, "Good bye !")
      j = SysSleep(.5)
   end

   self~fontToDC(dc,oldFont)
   self~deletefont(tmpFont)
   self~deletefont(sysFont)
   self~deleteObject(myBrush)
   self~deleteObject(myPen)
   self~FreeButtonDC(IDC_PB_DRAW,dc)

   call msSleep 1000

   self~cancel:super

::method enableButtons       -- Enable the selection buttons, disable interrupt.
   self~enableControl(IDCANCEL)
   self~enableControl(IDC_PB_BRUSH_RECTANGLES)
   self~enableControl(IDC_PB_PEN_RECTANGLES)
   self~enableControl(IDC_PB_PIXELS)
   self~enableControl(IDC_PB_STAR_LINES)
   self~enableControl(IDC_PB_RANDOM_LINES)
   self~enableControl(IDC_PB_RANDOM_SQUARES)
   self~enableControl(IDC_PB_ELLIPSES)
   self~disableControl(IDC_PB_INTERRUPT)

::method disableButtons      -- Disable the selection buttons, enable interrupt.
   self~disableControl(IDCANCEL)
   self~disableControl(IDC_PB_BRUSH_RECTANGLES)
   self~disableControl(IDC_PB_PEN_RECTANGLES)
   self~disableControl(IDC_PB_PIXELS)
   self~disableControl(IDC_PB_STAR_LINES)
   self~disableControl(IDC_PB_RANDOM_LINES)
   self~disableControl(IDC_PB_RANDOM_SQUARES)
   self~disableControl(IDC_PB_ELLIPSES)
   self~enableControl(IDC_PB_INTERRUPT)

::method greyButton          -- Repaint the drawing surface all grey.
   expose x y dc change
   change = 0
   brush = self~createBrush(7)
   pen = self~createPen(1, 'solid', 7)
   ob = self~objectToDC(dc, brush)
   op = self~objectToDC(dc, pen)

   self~rectangle(dc, x, y, 199 * x, 199 * y, 'FILL')

   self~objectToDC(dc, op)
   self~objectToDC(dc, ob)
   self~deleteObject(brush)
   self~deleteObject(pen)

::method brushRectangles                                 /* draw rectangles */
   expose x y dc myBrush myPen recThick change

   self~disableButtons

   if change \= 3 then do
      self~greyButton
      self~rectangle(dc, 26 * x, 26 * y, 174 * x, 174 * y, 'FILL')
   end

   op = self~objectToDC(dc, myPen)
   ob = self~objectToDC(dc, myBrush)
   if change = 3 then signal doRectangles

   do col=0 to 18
      i = col + 1
      brush = self~createBrush(col)
      self~objectToDC(dc,brush)
      j = 5*i
      self~rectangle(dc,j*x,j*y,(200-j)*x,(200-j)*y,'FILL')
      self~objectToDC(dc,ob)
      self~deleteObject(brush)
   end

   self~writeToButton(IDC_PB_DRAW,25*x,15*y,"Colored brush rectangles","Arial",12,"BOLD")
   call msSleep 2000
   change = 3

doRectangles:
   self~transparentText(dc)
   self~interrupted = .false
   do recThick = recThick+1 to 100 while \ self~interrupted
      self~objectToDC(dc,myPen)
      self~objectToDC(dc,myBrush)
      self~rectangle(dc,7*x,7*y,193*x,193*y,'FILL')

      do k=1 to 25 until self~finished
         brushcol = random(1,18)
         brush = self~createBrush(brushcol)
         if recThick//3 = 0 then pencol = brushcol
                            else pencol = random(1,18)
         thick = recThick%3 + 1
         pen = self~createPen(thick, 'solid', pencol)
         self~objectToDC(dc,pen)
         self~objectToDC(dc,brush)
         px = random(10,190)*x
         py = random(10,180)*y
         pxx= random(10,190)*x
         pyy= random(10,180)*y

         if recThick//3 = 2 then self~rectangle(dc,px,py,pxx,pyy)
         else self~rectangle(dc,px,py,pxx,pyy,'FILL')

         self~objectToDC(dc,ob)
         self~objectToDC(dc,op)
         self~deleteObject(pen)
         self~deleteObject(brush)
      end

      if recThick//3 = 1 then do
         self~writedirect(dc,15*x,183*y,"Random filled rectangles of thickness" thick)
      end
      else do
         if recThick//3 = 2 then self~writedirect(dc,15*x,183*y,"Random rectangles of thickness" thick)
         else self~writedirect(dc,15*x,183*y,"Random unicolor rectangles")
      end

      if \ self~interrupted then do
         call msSleep 2000
      end
   end
   self~objectToDC(dc,op)
   self~objectToDC(dc,ob)
   self~opaqueText(dc)
   self~interrupted = .true
   self~enableButtons

::method penRectangles unguarded                                  /* draw rectangles with pen */
   expose x y dc myBrush myPen
   self~disableButtons
   self~greyButton
   op = self~objectToDC(dc,myPen)
   ob = self~objectToDC(dc,myBrush)
   do col=0 to 18
      i = col+1
      pen = self~createPen(5,'solid',col)
      self~objectToDC(dc,pen)
      j = 5*i + 1
      self~rectangle(dc,j*x,j*y,(200-j)*x,(200-j)*y,'FILL')
      self~objectToDC(dc,op)
      self~deleteObject(pen)
   end

   self~writeToButton(IDC_PB_DRAW,35*x,24*y,"Colored pen rectangles","Arial",12,"BOLD")
   styles = .array~of('solid','dash','dot','dashdot','dashdotdot','null')
   call msSleep 2000

   self~interrupted = .false
   do k = 1 by 1 while \ self~interrupted
      self~objectToDC(dc,myPen)
      self~objectToDC(dc,myBrush)
      self~rectangle(dc,7*x,7*y,193*x,193*y,'FILL')
      do i=1 to 19
         if random(1,4) = 1 | k//2 = 0 then th = 1; else th = random(1,8)
         pen = self~createPen( th, styles[random(1,6)], random(1,18) )
         self~objectToDC(dc,pen)
         j = 5*i
         if k//2 = 1 then self~rectangle(dc,j*x,j*y,(200-j)*x,(200-j)*y)
                     else self~rectangle(dc,random(10,190)*x,random(10,180)*y, ,
                                            random(10,190)*x,random(10,180)*y)
         self~objectToDC(dc,op)
         self~deleteObject(pen)
      end
      self~writeToButton(IDC_PB_DRAW,25*x,24*y,"Solid and dotted rectangles","Arial",12,"BOLD")
      if \ self~interrupted then do
         call msSleep 2000
      end
   end

   self~objectToDC(dc,ob)
   self~interrupted = .true
   self~enableButtons

::method pixels                                        /* draw individual pixels */
   expose x y dc myBrush myPen kpix change
   mx = trunc(100*x); my = trunc(100*y); size=100
   self~disableButtons
   if change \= 2 then do
      self~greyButton
      self~rectangle(dc,mx-size,my-size,mx+size+2,my+size+2,'FILL')
   end
   change = 2
   self~writeToButton(IDC_PB_DRAW,20*x,20*y,"Drawing colored pixels","Arial",14,"BOLD")
   call msSleep 1000

   self~interrupted = .false
   do kpix =  kpix to size until self~interrupted
      px = mx + kpix
      do py = my-kpix+2 to my+kpix while \ self~interrupted
         if \self~interrupted then self~pause
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      py = my + kpix
      do px = mx+kpix-1 by -1 to mx-kpix+1 while \ self~interrupted
         if \self~interrupted then self~pause
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      px = mx - kpix + 1
      do py = my+kpix-1 by -1 to my-kpix+1 while \ self~interrupted
         if \self~interrupted then self~pause
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      py = my - kpix + 1
      do px = mx-kpix+2 to mx+kpix while \ self~interrupted
         if \self~interrupted then self~pause
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      if \ self~interrupted then self~writedirect(dc,125*x,170*y,"Done" kpix)
   end
   if kpix>=size then do; change=0; kpix=1; end

   self~interrupted = .true
   self~enableButtons

::method starLines                                         /* draw rotating lines */
   expose x y dc myBrush myPen linthick change
   self~disableButtons
   self~interrupted = .false

   op = self~objectToDC(dc,myPen)
   ob = self~objectToDC(dc,myBrush)
   if change \= 1 then do
      self~greyButton
      self~rectangle(dc,26*x,26*y,174*x,174*y,'FILL')
   end
   change = 1
   self~writeToButton(IDC_PB_DRAW,20*x,6*y,"Drawing colored lines","Arial",14,"BOLD")
   px = 100 * x
   py = px
   call msSleep 1000
   maxthick = 140
   do linthick=linthick by 1 to maxthick until self~interrupted
     self~writeToButton(IDC_PB_DRAW,20*x,185*y,"Lines are" linthick "thick","System",10)
     do j=1 to 4 until self~interrupted
       do i=30 by linthick to 170 until self~interrupted
          if \ self~interrupted then self~pause
          if j=1 then do; pxx = 170*x; pyy = i*y      ; end
          if j=2 then do; pyy = 170*x; pxx = (200-i)*y; end
          if j=3 then do; pxx =  30*x; pyy = (200-i)*y; end
          if j=4 then do; pyy =  30*x; pxx = i*y      ; end
          col= random(0,18)
          pen = self~createPen(linthick,'solid',col)
          self~objectToDC(dc,pen)
          self~drawLine(dc,px,py,pxx,pyy)
          self~objectToDC(dc,op)
          self~deleteObject(pen)
       end
     end
   end
   if linthick>=maxthick then do; change=0; linthick=1; end
   self~objectToDC(dc,ob)
   self~interrupted = .true
   self~enableButtons

::method randomLines                                       /* draw random lines */
   expose x y dc myBrush myPen change ranthick

   self~disableButtons
   self~interrupted = .false

   if change \= 5 then do
      self~greyButton
      self~rectangle(dc,26*x,26*y,174*x,174*y,'FILL')
   end
   change = 5
   op = self~objectToDC(dc,myPen)
   ob = self~objectToDC(dc,myBrush)
   self~writeToButton(IDC_PB_DRAW,20*x,5*y,"Drawing random lines","Arial",14,"BOLD")
   call msSleep 1000

   do ranthick=ranthick by 1 until self~interrupted
     self~rectangle(dc,15*x,15*y,185*x,185*y,'FILL')
     pxx = 30*x; pyy = 30*y
     maxthick = max(1,random(1,150)-10)  /* favor 1 */
     do i=0  to 49 until self~interrupted
       if \ self~interrupted then self~pause
       col= random(0,18)
       if ranthick//2 = 1 then thickd = ranthick%2 // 50 + 1
                          else thickd= (maxthick - i*maxthick/50) % 3 + 1
       pen = self~createPen(thickd,'solid',col)
       if i=0 then do
          if ranthick//2 = 1 then
               self~writeToButton(IDC_PB_DRAW,15*x,188*y,"Connected lines of even thickness" thickd,"System",10)
          else self~writeToButton(IDC_PB_DRAW,15*x,188*y,"Random lines of thickness" thickd "to 1        ","System",10)
       end
       ix = trunc(thickd / 2 / x)
       iy = trunc(thickd / 2 / y)
       pxx= random(15+ix,185-ix)*x
       pyy= random(15+iy,185-iy)*y
       self~objectToDC(dc,pen)
       if ranthick//2 = 1 then                        /* connected lines */
          self~drawLine(dc,,,pxx,pyy)
       else do
          px = random(15+ix,185-ix)*x                 /* random lines */
          py = random(15+iy,185-iy)*y
          self~drawLine(dc,px,py,pxx,pyy)
       end
       self~objectToDC(dc,op)
       self~deleteObject(pen)
     end
     if \self~interrupted then do
        call msSleep 2000
     end
   end

   self~objectToDC(dc,ob)
   self~interrupted = .true
   self~enableButtons

::method randomSquares                                     /* draw colored squares */
   expose x y dc myBrush myPen colornames
   self~disableButtons
   self~interrupted = .false
   self~greyButton
   blackbrush = self~createBrush(0)
   op = self~objectToDC(dc,myPen)
   ob = self~objectToDC(dc,blackbrush)
   self~rectangle(dc,x,y,199*x,199*y,'FILL')
   self~objectToDC(dc,ob)
   self~deleteObject(blackbrush)
   self~writeToButton(IDC_PB_DRAW,15*x,5*y,"Drawing colored squares","Arial",14,"BOLD")
   colorx = .array~of(1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,10)
   colors = colorx~copy
   sx = 10*x; sy=10*y; sx5=5*x; sy5=5*y
   do i=1 by 1 until self~interrupted
      if \ self~interrupted then self~pause
      px = random(1,17)*sx + sx5
      py = random(1,17)*sy + sy5
      col= colors[random(1,18)]
      brush = self~createBrush(col)
      self~objectToDC(dc,brush)
      self~rectangle(dc,px,py,px+sx,py+sy,'FILL')
      self~objectToDC(dc,ob)
      self~deleteObject(brush)
      if i//50 = 0 then do
          if i<1000 then do
             c1 = colors[min(i/50,18)]; c2 = 10
             colors[i/50] = 10     /* make whiter */
             end
          else do
             i1 = i/50 //18 + 1; i2 = 18- i%250//18
             c1 = colors[i1]; c2 = colorx[i2]
             colors[i1] = colorx[i2]     /* shift colors */
          end
          txt = '- ' colornames[c1]~left(20) '+ ' colornames[c2]~left(20)
          self~writeToButton(IDC_PB_DRAW,15*x,188*y,txt,"System")
          self~writeToButton(IDC_PB_DRAW,150*x,188*y,"Done" i,"System")
      end
   end

   self~objectToDC(dc,op)
   self~interrupted = .true
   self~enableButtons

::method circleEllipses                                    /* draw circles and ellipses */
   expose x y dc myBrush myPen colornames change jellipse

   self~interrupted = .false
   self~disableButtons

   variat = 6
   if change \= 4 then do
      self~greyButton
      jellipse = 1
   end
   op = self~objectToDC(dc,myPen)
   ob = self~objectToDC(dc,myBrush)
   if change = 4 then signal doellipses
   self~writeToButton(IDC_PB_DRAW,15*x,2*y,"Drawing colored ellipses","Arial",14,"BOLD")
   self~rectangle(dc,10*x,10*y,190*x,190*y,'FILL')
   do i=1 to 18 until self~interrupted
      if \ self~interrupted then self~pause
      px = i*5+5; py = i*3+7
      self~drawellipse(i,3,px,py,200-px,200-py)
   end

   call msSleep 2000
   if self~interrupted then signal restore

   self~rectangle(dc,10*x,10*y,190*x,190*y,'FILL')
   do i=1 to 18 until self~interrupted
      if \ self~interrupted then self~pause
      px = i*4+6; py = i*5+5
      self~drawellipse(random(1,18),random(1,4),px,py,200-px,200-py)
      self~drawellipse(random(1,18),random(1,4),py,px,200-py,200-px)
   end
   call msSleep 2000
   change = 4
DOELLIPSES:
   if self~interrupted then signal restore
   do jellipse = jellipse by 1 until self~interrupted
      self~rectangle(dc,10*x,10*y,190*x,190*y,'FILL')
      self~setarcdirection(dc,'COUNTERCLOCKWISE')
      if jellipse//variat = 1 then                       /* concentric partial circles */
         do i=1 to 18 until self~interrupted
            px = i*5+5; py = i*5+5
            if i//4 = 3 then
               if random(1,2)=1 then
                    self~drawellipse(random(1,18),random(1,10),px,py,200-px,200-py,200,100,0,100)
               else self~drawellipse(random(1,18),random(1,10),px,py,200-px,200-py,0,100,200,100)
            else self~drawellipse(random(1,18),random(1,10),px,py,200-px,200-py, ,
                                   (random(1,2)-1)*200,(random(1,2)-1)*200, ,
                                   (random(1,2)-1)*200,(random(1,2)-1)*200)
            if i=1 then self~writeToButton(IDC_PB_DRAW,15*x,190*y,"concentric partial circles  ","System",10)
         end
      else if jellipse//variat = 2 then                  /* random full ellipses */
         do i = 1 to 50 until self~interrupted
            if \ self~interrupted then self~pause
            px = random(15,185);  py = random(15,185)
            pxx= random(15,185);  pyy= random(15,185)
            col = random(1,18)
            self~drawellipse(col,random(1,10),px,py,pxx,pyy)
            if i//3 = 1 & col \= 10 & abs(px-pxx)>1 & abs(py-pyy)>2 then do
               brush = self~createBrush(random(1,18))
               self~objectToDC(dc,brush)
               self~fillDrawing(dc, (px+pxx)%2*x,(py+pyy)%2*x, col)
               self~objectToDC(dc,ob)
               self~deleteObject(brush)
            end
            if i=1 then self~writeToButton(IDC_PB_DRAW,15*x,190*y,"random complete ellipses","System",10)
         end
      else if jellipse//variat = 3 then                  /* random partial ellipses */
         do i = 1 to 40 until self~interrupted
            if \ self~interrupted then self~pause
            if i//3 = 0 then self~setarcdirection(dc,'COUNTERCLOCKWISE')
                        else self~setarcdirection(dc,'CLOCKWISE')
            px = random(15,185);  py = random(15,185)
            pxx= random(15,185);  pyy= random(15,185)
            sx = random(15,185);  sy = random(15,185)
            ex = random(15,185);  ey = random(15,185)
            self~drawEllipse(random(1,18),random(1,16),px,py,pxx,pyy,sx,sy,ex,ey)
            if i=1 then self~writeToButton(IDC_PB_DRAW,15*x,190*y,"random partial ellipses     ","System",10)
         end
      else if jellipse//variat = 4 then                  /* nice pies */
         do i = 1 to 20 until self~interrupted
            if \ self~interrupted then self~pause
            px = 15;  py = 15; pxx = 185; pyy = 185
            if i<11 then do; sx = 60*i - 260; ex = sx+50; end
                    else do; sx = 900 - 50*i; ex = sx-40; end
            if i=1 | i=11 then sy = 100; else if i<11 then sy = 0; else sy = 200
            if i<10 then ey = 0; else if i=10 then ey = 95
                    else if i<20 then ey = 200; else ey = 105
            col = random(1,18)
            self~drawPieX(col,col,1,px,py,pxx,pyy,ex,ey,sx,sy)
            if i=1 then self~writeToButton(IDC_PB_DRAW,15*x,190*y,"arranged pies                 ","System",10)
         end
      else if jellipse//variat = 5 then                  /* random pies */
         do i = 1 to 30 until self~interrupted
            if \ self~interrupted then self~pause
            px = random(15,185);  py = random(15,185)
            pxx= random(15,185);  pyy= random(15,185)
            sx = random(15,185);  sy = random(15,185)
            ex = random(15,185);  ey = random(15,185)
            col = random(1,18)
            if i//2 = 1 then
                 self~drawPieX(col,random(1,18),random(1,8),px,py,pxx,pyy,sx,sy,ex,ey)
            else self~drawPieX(col,col,1,px,py,pxx,pyy,sx,sy,ex,ey)
            if i=1 then self~writeToButton(IDC_PB_DRAW,15*x,190*y,"random pies                   ","System",10)
         end
      else                                              /* random angle arcs */
         do i = 1 to 10 until self~interrupted | ret>0
            if \ self~interrupted then self~pause
            sx = 150-random(1,100); sy = 150-random(1,100)
            px = 130-random(1,60); py = 130-random(1,60)
            radius = random(20, min(px,200-px,py,200-py)-10)
            angle = random(1,360); sweep = random(1,360)
            col = random(1,18)
            pen = self~createPen(random(3,10),'solid',col)
            self~objectToDC(dc,pen)
            ret = self~drawAngleArc(dc,sx*x,sy*y,px*x,py*y,radius*x,angle,sweep)
            if i//2 = 1 then self~drawline(dc,,,sx*x,sy*y)
            self~objectToDC(dc,myPen)
            self~deleteObject(pen)
            if i=1 then self~writeToButton(IDC_PB_DRAW,15*x,190*y,"random angle arcs           ","System",10)
            if ret>0 then self~writeToButton(IDC_PB_DRAW,50*x,100*y,"Not working on Windows 95","System",10)
         end
      if \ self~interrupted then do
         call msSleep 2000
      end
   end
restore:
   self~objectToDC(dc, op)
   self~objectToDC(dc, ob)
   self~setArcDirection(dc, 'COUNTERCLOCKWISE')
   self~interrupted = .true
   self~enableButtons

::method drawEllipse private
   expose x y dc myBrush myPen
   use arg col, penth, px, py, pxx, pyy, sx, sy, ex, ey

   pen = self~createPen(penth, 'solid', col)
   self~objectToDC(dc, pen)

   if arg()<7 then self~drawArc(dc, px * x, py * y, pxx * x, pyy * y)
   else self~drawArc(dc, px * x, py * y, pxx * x, pyy * y, sx * x, sy * y, ex * x, ey * y)

   self~objectToDC(dc, myPen)
   self~deleteObject(pen)

::method drawPieX
   expose x y dc myBrush myPen jellipse
   use arg col, col2, penth, px, py, pxx, pyy, sx, sy, ex, ey

   pen = self~createPen(penth, 'solid', col)
   if jellipse // 3 = 1 then brush = self~createBrush(col2)   /* no args */
                        else brush = self~createBrush(col)

   self~objectToDC(dc, pen)
   self~objectToDC(dc, brush)

   self~drawPie(dc, px * x, py * y, pxx * x, pyy * y, sx * x, sy * y, ex * x, ey * y)

   self~objectToDC(dc, myPen)
   self~objectToDC(dc, myBrush)

   self~deleteObject(pen)
   self~deleteObject(brush)

::method pause
   j = msSleep(10)


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
/* OODialog\Samples\oodraw.rex    Drawing demonstration with colors         */
/*                                                                          */
/*--------------------------------------------------------------------------*/


 d = .drawdlg~new
 if d~initCode <> 0 then do
    say 'The Draw dialog was not created correctly.  Aborting.'
    return d~initCode
 end
 d~Execute("SHOWTOP")
 d~deinstall
 return

/*---------------------------------- requires ------------------------*/

::requires "OODIALOG.CLS"

/*---------------------------------- dialog class --------------------*/

::class drawdlg subclass UserDialog

::method init
   expose colornames
   colornames = .array~of('dark red','dark green','dark yellow',,
                'dark blue','purple','blue grey','light grey','pale green',,
                'light blue','white','grey','dark grey','red','light green',,
                'yellow','blue','pink','turquoise')
   self~init:super()
   if \ self~createcenter(200,235,"OODialog Color Drawing Demonstration",,,"System", 8) then
      self~initCode = 1

::method DefineDialog
   self~DefineDialog:super
   self~createPushButton(100, 0,0,200,200, "DISABLED NOTAB")  /* draw button */
   self~createPushButton(11,5,  205,55,12, ,"&Rectangles","brushRectangles")
   self~createPushButton(12,62, 205,55,12, ,"&Pen Rectangles","penRectangles")
   self~createPushButton(13,119,205,36,12, ,"Pi&xels","pixels")
   self~createPushButton(14,5,  220,36,12, ,"S&tar","starLines")
   self~createPushButton(15,43, 220,36,12, ,"&Lines","randomLines")
   self~createPushButton(16,81, 220,36,12, ,"&Squares","randomSquares")
   self~createPushButton(17,119,220,36,12, ,"&Ellipses","circleEllipses")
   self~createPushButton(1, 160,205,35,12, "DEFAULT","&Interrupt","OK")
   self~createPushButton(2, 160,220,35,12, ,"&Cancel","Cancel")
   do i=5 to 49
      self~addBlackRect(2*i,2*i,200-4*i,200-4*i,"BORDER")
   end
   do i=51 to 99
      self~addWhiteRect(i+25,i+25,200-2*i,200-2*i,"BORDER")
   end

::method Run
   expose x y dc mybrush mypen recthick linthick ranthick kpix change sysfont oldfont
   self~disableItem(1)                    /* cannot interrupt yet */
   change = 0
   recthick = 0; linthick = 1; ranthick=0; kpix = 1;
   x = self~factorx
   y = self~factory
   /* say 'dialog units =' x y 'pixels' */
   dc = self~getButtonDC(100)
   mybrush = self~createbrush(10)         /* white      */
   mypen   = self~createpen(1,'solid',0)  /* thin black */
   sysFont = self~CreateFont("System",10)
   oldFont = self~FontToDC(dc,sysFont)
   self~writetobutton(100,45*x,30*y,"Black rectangles","Arial",12,"BOLD")
   self~writetobutton(100,80*x,80*y,"White rectangles","Arial",12,"BOLD")
   do i=1 by 1 until self~finished
      self~handleMessages
      call msSleep 500
   end

::method OK                                              /* Interrupt button */
   self~disableItem(1)  /*interrupt*/
   self~ok:super

::method cancel                                          /* stop drawing program */
   expose x y dc mybrush mypen sysfont oldfont
   self~finished = 1
   tmpFont = self~CreateFont("Arial",28,"BOLD")
   xFont = self~FontToDC(dc,tmpFont)
   do i=2 to 6
      if i//2 = 1 then self~TransparentText(dc)
                  else self~OpaqueText(dc)
      self~WriteDirect(dc,30*x,25*i*y,"Good bye !")
      call msSleep 500
   end
   self~fontToDC(dc,oldfont)
   self~deletefont(tmpFont)
   self~deletefont(sysfont)
   self~deleteobject(mybrush)
   self~deleteobject(mypen)
   self~FreeButtonDC(100,dc)
   call msSleep 1000
   return self~finished

::method enableButtons                                   /* enable demo buttons */
   self~enableItem(2)   /*cancel*/
   self~enableItem(11)
   self~enableItem(12)
   self~enableItem(13)
   self~enableItem(14)
   self~enableItem(15)
   self~enableItem(16)
   self~enableItem(17)
   self~disableItem(1)  /*interrupt*/

::method disableButtons                                  /* disable demo buttons */
   self~disableItem(2)  /*cancel*/
   self~disableItem(11)
   self~disableItem(12)
   self~disableItem(13)
   self~disableItem(14)
   self~disableItem(15)
   self~disableItem(16)
   self~disableItem(17)
   self~enableItem(1)   /*interrupt*/

::method greyButton                                      /* restore the grey button */
   expose x y dc change
   change = 0
   brush = self~createbrush(7)
   pen = self~createpen(1,'solid',7)
   ob = self~objecttodc(dc,brush)
   op = self~objecttodc(dc,pen)
   self~rectangle(dc,x,y,199*x,199*y,'FILL')
   self~objecttodc(dc,op)
   self~objecttodc(dc,ob)
   self~deleteobject(brush)
   self~deleteobject(pen)

::method brushRectangles                                 /* draw rectangles */
   expose x y dc mybrush mypen recthick change
   self~disableButtons
   if change \= 3 then do
      self~greybutton
      self~rectangle(dc,26*x,26*y,174*x,174*y,'FILL')
   end
   op = self~objecttodc(dc,mypen)
   ob = self~objecttodc(dc,mybrush)
   if change = 3 then signal dorectangles
   do col=0 to 18
      i = col+1
      brush = self~createbrush(col)
      self~objecttodc(dc,brush)
      j = 5*i
      self~rectangle(dc,j*x,j*y,(200-j)*x,(200-j)*y,'FILL')
      self~objecttodc(dc,ob)
      self~deleteobject(brush)
   end
   self~writetobutton(100,25*x,15*y,"Colored brush rectangles","Arial",12,"BOLD")
   call msSleep 2000
   change = 3
  DORECTANGLES:
   self~HandleMessages
   self~transparentText(dc)
   do recthick = recthick+1 to 100 while self~finished = 0
      self~objecttodc(dc,mypen)
      self~objecttodc(dc,mybrush)
      self~rectangle(dc,7*x,7*y,193*x,193*y,'FILL')
      do k=1 to 25 until self~finished
         brushcol = random(1,18)
         brush = self~createbrush(brushcol)
         if recthick//3 = 0 then pencol = brushcol
                            else pencol = random(1,18)
         thick = recthick%3 + 1
         pen = self~createpen(thick, 'solid', pencol)
         self~objecttodc(dc,pen)
         self~objecttodc(dc,brush)
         px = random(10,190)*x
         py = random(10,180)*y
         pxx= random(10,190)*x
         pyy= random(10,180)*y
         if recthick//3 = 2 then self~rectangle(dc,px,py,pxx,pyy)
                            else self~rectangle(dc,px,py,pxx,pyy,'FILL')
         self~objecttodc(dc,ob)
         self~objecttodc(dc,op)
         self~deleteobject(pen)
         self~deleteobject(brush)
      end
      if self~finished = 0 then self~HandleMessages
      if recthick//3 = 1 then
           self~writedirect(dc,15*x,183*y,"Random filled rectangles of thickness" thick)
      else if recthick//3 = 2
           then self~writedirect(dc,15*x,183*y,"Random rectangles of thickness" thick)
           else self~writedirect(dc,15*x,183*y,"Random unicolor rectangles")
      if self~finished=0 then do
         call msSleep 2000
         self~HandleMessages
      end
   end
   self~objecttodc(dc,op)
   self~objecttodc(dc,ob)
   self~opaqueText(dc)
   self~finished = 0
   self~enableButtons

::method penRectangles                                  /* draw rectangles with pen */
   expose x y dc mybrush mypen
   self~disableButtons
   self~greybutton
   op = self~objecttodc(dc,mypen)
   ob = self~objecttodc(dc,mybrush)
   do col=0 to 18
      i = col+1
      pen = self~createpen(5,'solid',col)
      self~objecttodc(dc,pen)
      j = 5*i + 1
      self~rectangle(dc,j*x,j*y,(200-j)*x,(200-j)*y,'FILL')
      self~objecttodc(dc,op)
      self~deleteobject(pen)
   end
   self~writetobutton(100,35*x,24*y,"Colored pen rectangles","Arial",12,"BOLD")
   call msSleep 2000
   self~HandleMessages
   styles = .array~of('solid','dash','dot','dashdot','dashdotdot','null')
   do k = 1 by 1 while self~finished = 0
      if self~finished = 0 then self~HandleMessages
      self~objecttodc(dc,mypen)
      self~objecttodc(dc,mybrush)
      self~rectangle(dc,7*x,7*y,193*x,193*y,'FILL')
      do i=1 to 19
         if random(1,4) = 1 | k//2 = 0 then th = 1; else th = random(1,8)
         pen = self~createpen( th, styles[random(1,6)], random(1,18) )
         self~objecttodc(dc,pen)
         j = 5*i
         if k//2 = 1 then self~rectangle(dc,j*x,j*y,(200-j)*x,(200-j)*y)
                     else self~rectangle(dc,random(10,190)*x,random(10,180)*y, ,
                                            random(10,190)*x,random(10,180)*y)
         self~objecttodc(dc,op)
         self~deleteobject(pen)
      end
      self~writetobutton(100,25*x,24*y,"Solid and dotted rectangles","Arial",12,"BOLD")
      if self~finished=0 then do
         call msSleep 2000
         self~HandleMessages
      end
   end
   self~objecttodc(dc,ob)
   self~finished = 0
   self~enableButtons

::method pixels                                        /* draw individual pixels */
   expose x y dc mybrush mypen kpix change
   mx = trunc(100*x); my = trunc(100*y); size=100
   self~disableButtons
   if change \= 2 then do
      self~greybutton
      self~rectangle(dc,mx-size,my-size,mx+size+2,my+size+2,'FILL')
   end
   change = 2
   self~writetobutton(100,20*x,20*y,"Drawing colored pixels","Arial",14,"BOLD")
   call msSleep 1000
   do kpix =  kpix to size until self~finished
      px = mx + kpix
      do py = my-kpix+2 to my+kpix while self~finished=0
         if self~finished = 0 then self~HandleMessages
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      py = my + kpix
      do px = mx+kpix-1 by -1 to mx-kpix+1 while self~finished=0
         if self~finished = 0 then self~HandleMessages
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      px = mx - kpix + 1
      do py = my+kpix-1 by -1 to my-kpix+1 while self~finished=0
         if self~finished = 0 then self~HandleMessages
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      py = my - kpix + 1
      do px = mx-kpix+2 to mx+kpix while self~finished=0
         if self~finished = 0 then self~HandleMessages
         col= random(0,18); self~drawpixel(dc,px,py,col)
      end
      if self~finished=0 then self~writedirect(dc,125*x,170*y,"Done" kpix)
   end
   if kpix>=size then do; change=0; kpix=1; end
   self~finished = 0
   self~enableButtons

::method starLines                                         /* draw rotating lines */
   expose x y dc mybrush mypen linthick change
   self~disableButtons
   op = self~objecttodc(dc,mypen)
   ob = self~objecttodc(dc,mybrush)
   if change \= 1 then do
      self~greybutton
      self~rectangle(dc,26*x,26*y,174*x,174*y,'FILL')
   end
   change = 1
   self~writetobutton(100,20*x,6*y,"Drawing colored lines","Arial",14,"BOLD")
   px = 100 * x
   py = px
   call msSleep 1000
   maxthick = 140
   do linthick=linthick by 1 to maxthick until self~finished
     self~writetobutton(100,20*x,185*y,"Lines are" linthick "thick","System",10)
     do j=1 to 4 until self~finished
       do i=30 by linthick to 170 until self~finished
          if self~finished = 0 then self~HandleMessages
          if j=1 then do; pxx = 170*x; pyy = i*y      ; end
          if j=2 then do; pyy = 170*x; pxx = (200-i)*y; end
          if j=3 then do; pxx =  30*x; pyy = (200-i)*y; end
          if j=4 then do; pyy =  30*x; pxx = i*y      ; end
          col= random(0,18)
          pen = self~createpen(linthick,'solid',col)
          self~objecttodc(dc,pen)
          self~drawLine(dc,px,py,pxx,pyy)
          self~objecttodc(dc,op)
          self~deleteobject(pen)
       end
     end
   end
   if linthick>=maxthick then do; change=0; linthick=1; end
   self~objecttodc(dc,ob)
   self~finished = 0
   self~enableButtons

::method randomLines                                       /* draw random lines */
   expose x y dc mybrush mypen change ranthick
   self~disableButtons
   if change \= 5 then do
      self~greybutton
      self~rectangle(dc,26*x,26*y,174*x,174*y,'FILL')
   end
   change = 5
   op = self~objecttodc(dc,mypen)
   ob = self~objecttodc(dc,mybrush)
   self~writetobutton(100,20*x,5*y,"Drawing random lines","Arial",14,"BOLD")
   call msSleep 1000
   do ranthick=ranthick by 1 until self~finished
     self~rectangle(dc,15*x,15*y,185*x,185*y,'FILL')
     pxx = 30*x; pyy = 30*y
     maxthick = max(1,random(1,150)-10)  /* favor 1 */
     do i=0  to 49 until self~finished
       if self~finished = 0 then self~HandleMessages
       col= random(0,18)
       if ranthick//2 = 1 then thickd = ranthick%2 // 50 + 1
                          else thickd= (maxthick - i*maxthick/50) % 3 + 1
       pen = self~createpen(thickd,'solid',col)
       if i=0 then do
          if ranthick//2 = 1 then
               self~writetobutton(100,15*x,188*y,"Connected lines of even thickness" thickd,"System",10)
          else self~writetobutton(100,15*x,188*y,"Random lines of thickness" thickd "to 1        ","System",10)
       end
       ix = trunc(thickd / 2 / x)
       iy = trunc(thickd / 2 / y)
       pxx= random(15+ix,185-ix)*x
       pyy= random(15+iy,185-iy)*y
       self~objecttodc(dc,pen)
       if ranthick//2 = 1 then                        /* connected lines */
          self~drawLine(dc,,,pxx,pyy)
       else do
          px = random(15+ix,185-ix)*x                 /* random lines */
          py = random(15+iy,185-iy)*y
          self~drawLine(dc,px,py,pxx,pyy)
       end
       self~objecttodc(dc,op)
       self~deleteobject(pen)
     end
     if self~finished=0 then do
        call msSleep 2000
        self~HandleMessages
     end
   end
   self~objecttodc(dc,ob)
   self~finished = 0
   self~enableButtons

::method randomSquares                                     /* draw colored squares */
   expose x y dc mybrush mypen colornames
   self~disableButtons
   self~greybutton
   blackbrush = self~createbrush(0)
   op = self~objecttodc(dc,mypen)
   ob = self~objecttodc(dc,blackbrush)
   self~rectangle(dc,x,y,199*x,199*y,'FILL')
   self~objecttodc(dc,ob)
   self~deleteobject(blackbrush)
   self~writetobutton(100,15*x,5*y,"Drawing colored squares","Arial",14,"BOLD")
   colorx = .array~of(1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,10)
   colors = colorx~copy
   sx = 10*x; sy=10*y; sx5=5*x; sy5=5*y
   do i=1 by 1 until self~finished
      if self~finished = 0 then self~HandleMessages
      px = random(1,17)*sx + sx5
      py = random(1,17)*sy + sy5
      col= colors[random(1,18)]
      brush = self~createbrush(col)
      self~objecttodc(dc,brush)
      self~rectangle(dc,px,py,px+sx,py+sy,'FILL')
      self~objecttodc(dc,ob)
      self~deleteobject(brush)
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
          self~writetobutton(100,15*x,188*y,txt,"System")
          self~writetobutton(100,150*x,188*y,"Done" i,"System")
      end
   end
   self~objecttodc(dc,op)
   self~finished = 0
   self~enableButtons

::method circleEllipses                                    /* draw circles and ellipses */
   expose x y dc mybrush mypen colornames change jellipse
   self~disableButtons
   variat = 6
   if change \= 4 then do
      self~greybutton
      jellipse = 1
   end
   op = self~objecttodc(dc,mypen)
   ob = self~objecttodc(dc,mybrush)
   if change = 4 then signal doellipses
   self~writetobutton(100,15*x,2*y,"Drawing colored ellipses","Arial",14,"BOLD")
   self~rectangle(dc,10*x,10*y,190*x,190*y,'FILL')
   do i=1 to 18 until self~finished
      if self~finished = 0 then self~HandleMessages
      px = i*5+5; py = i*3+7
      self~drawellipse(i,3,px,py,200-px,200-py)
   end
   call msSleep 2000
   self~HandleMessages
   if self~finished then signal restore
   self~rectangle(dc,10*x,10*y,190*x,190*y,'FILL')
   do i=1 to 18 until self~finished
      if self~finished = 0 then self~HandleMessages
      px = i*4+6; py = i*5+5
      self~drawellipse(random(1,18),random(1,4),px,py,200-px,200-py)
      self~drawellipse(random(1,18),random(1,4),py,px,200-py,200-px)
   end
   call msSleep 2000
   change = 4
  DOELLIPSES:
   self~HandleMessages
   if self~finished then signal restore
   do jellipse = jellipse by 1 until self~finished
      self~rectangle(dc,10*x,10*y,190*x,190*y,'FILL')
      self~setarcdirection(dc,'COUNTERCLOCKWISE')
      if jellipse//variat = 1 then                       /* concentric partial circles */
         do i=1 to 18 until self~finished
            px = i*5+5; py = i*5+5
            if i//4 = 3 then
               if random(1,2)=1 then
                    self~drawellipse(random(1,18),random(1,10),px,py,200-px,200-py,200,100,0,100)
               else self~drawellipse(random(1,18),random(1,10),px,py,200-px,200-py,0,100,200,100)
            else self~drawellipse(random(1,18),random(1,10),px,py,200-px,200-py, ,
                                   (random(1,2)-1)*200,(random(1,2)-1)*200, ,
                                   (random(1,2)-1)*200,(random(1,2)-1)*200)
            if i=1 then self~writetobutton(100,15*x,190*y,"concentric partial circles  ","System",10)
         end
      else if jellipse//variat = 2 then                  /* random full ellipses */
         do i = 1 to 50 until self~finished
            if self~finished = 0 then self~HandleMessages
            px = random(15,185);  py = random(15,185)
            pxx= random(15,185);  pyy= random(15,185)
            col = random(1,18)
            self~drawellipse(col,random(1,10),px,py,pxx,pyy)
            if i//3 = 1 & col \= 10 & abs(px-pxx)>1 & abs(py-pyy)>2 then do
               brush = self~createbrush(random(1,18))
               self~objecttodc(dc,brush)
               self~fillDrawing(dc, (px+pxx)%2*x,(py+pyy)%2*x, col)
               self~objecttodc(dc,ob)
               self~deleteobject(brush)
            end
            if i=1 then self~writetobutton(100,15*x,190*y,"random complete ellipses","System",10)
         end
      else if jellipse//variat = 3 then                  /* random partial ellipses */
         do i = 1 to 40 until self~finished
            if self~finished = 0 then self~HandleMessages
            if i//3 = 0 then self~setarcdirection(dc,'COUNTERCLOCKWISE')
                        else self~setarcdirection(dc,'CLOCKWISE')
            px = random(15,185);  py = random(15,185)
            pxx= random(15,185);  pyy= random(15,185)
            sx = random(15,185);  sy = random(15,185)
            ex = random(15,185);  ey = random(15,185)
            self~drawEllipse(random(1,18),random(1,16),px,py,pxx,pyy,sx,sy,ex,ey)
            if i=1 then self~writetobutton(100,15*x,190*y,"random partial ellipses     ","System",10)
         end
      else if jellipse//variat = 4 then                  /* nice pies */
         do i = 1 to 20 until self~finished
            if self~finished = 0 then self~HandleMessages
            px = 15;  py = 15; pxx = 185; pyy = 185
            if i<11 then do; sx = 60*i - 260; ex = sx+50; end
                    else do; sx = 900 - 50*i; ex = sx-40; end
            if i=1 | i=11 then sy = 100; else if i<11 then sy = 0; else sy = 200
            if i<10 then ey = 0; else if i=10 then ey = 95
                    else if i<20 then ey = 200; else ey = 105
            col = random(1,18)
            self~drawPieX(col,col,1,px,py,pxx,pyy,ex,ey,sx,sy)
            if i=1 then self~writetobutton(100,15*x,190*y,"arranged pies                 ","System",10)
         end
      else if jellipse//variat = 5 then                  /* random pies */
         do i = 1 to 30 until self~finished
            if self~finished = 0 then self~HandleMessages
            px = random(15,185);  py = random(15,185)
            pxx= random(15,185);  pyy= random(15,185)
            sx = random(15,185);  sy = random(15,185)
            ex = random(15,185);  ey = random(15,185)
            col = random(1,18)
            if i//2 = 1 then
                 self~drawPieX(col,random(1,18),random(1,8),px,py,pxx,pyy,sx,sy,ex,ey)
            else self~drawPieX(col,col,1,px,py,pxx,pyy,sx,sy,ex,ey)
            if i=1 then self~writetobutton(100,15*x,190*y,"random pies                   ","System",10)
         end
      else                                              /* random angle arcs */
         do i = 1 to 10 until self~finished | ret>0
            if self~finished = 0 then self~HandleMessages
            sx = 150-random(1,100); sy = 150-random(1,100)
            px = 130-random(1,60); py = 130-random(1,60)
            radius = random(20, min(px,200-px,py,200-py)-10)
            angle = random(1,360); sweep = random(1,360)
            col = random(1,18)
            pen = self~createpen(random(3,10),'solid',col)
            self~objecttodc(dc,pen)
            ret = self~drawAngleArc(dc,sx*x,sy*y,px*x,py*y,radius*x,angle,sweep)
            if i//2 = 1 then self~drawline(dc,,,sx*x,sy*y)
            self~objecttodc(dc,mypen)
            self~deleteobject(pen)
            if i=1 then self~writetobutton(100,15*x,190*y,"random angle arcs           ","System",10)
            if ret>0 then self~writetobutton(100,50*x,100*y,"Not working on Windows 95","System",10)
         end
      if self~finished = 0 then do
         call msSleep 2000
         self~HandleMessages
      end
   end
  restore:
   self~objecttodc(dc,op)
   self~objecttodc(dc,ob)
   self~setarcdirection(dc,'COUNTERCLOCKWISE')
   self~finished = 0
   self~enableButtons

::method drawEllipse
   expose x y dc mybrush mypen
   use arg col, penth, px, py, pxx, pyy, sx, sy, ex, ey
   pen = self~createpen(penth,'solid',col)
   self~objecttodc(dc,pen)
   if arg()<7 then self~drawarc(dc,px*x,py*y,pxx*x,pyy*y)
              else self~drawarc(dc,px*x,py*y,pxx*x,pyy*y,sx*x,sy*y,ex*x,ey*y)
   self~objecttodc(dc,mypen)
   self~deleteobject(pen)

::method drawPieX
   expose x y dc mybrush mypen jellipse
   use arg col, col2, penth, px, py, pxx, pyy, sx, sy, ex, ey
   pen = self~createpen(penth,'solid',col)
   if jellipse //3 = 1 then brush = self~createbrush(col2)   /* no args */
                       else brush = self~createbrush(col2)
   self~objecttodc(dc,pen)
   self~objecttodc(dc,brush)
   self~drawPie(dc,px*x,py*y,pxx*x,pyy*y,sx*x,sy*y,ex*x,ey*y)
   self~objecttodc(dc,mypen)
   self~objecttodc(dc,mybrush)
   self~deleteobject(pen)
   self~deleteobject(brush)


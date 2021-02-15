/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2014 Rexx Language Association. All rights reserved.    */
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

/*----------------------------------------------------------------------------*/
/* How to use this template:                                                 */
/*                                                                           */
/* This template allows you to create dialogs with ooDialog without need     */
/* for the resource workshop.                                                */
/*                                                                           */
/* It sets out the class directive and the basic methods for a dialog        */
/*                                                                           */
/* ooDialog is very fully featured and it would not be useful to try to      */
/* anticipate and include every possible activity.                           */
/*                                                                           */
/* Not all Dialogs require all the methods listed in the template.           */
/* If you are not adding any control through a particular method then you    */
/* can delete that method and let the superclass handle it                   */
/*                                                                           */
/* ========================================================================= */

/*      FIRST - Use   Save As   to give the template it's new filename       */

signal on any name any          /* error handling if you want                */

                        /* ------------------------------------------------- */
                        /* Code that you want to execute before creating     */
                        /* the dialog goes here.                             */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* You can pre-define dialog object default values   */
                        /* here by setting them in the a. stem               */
                        /*                                                   */
                        /* For instance if you have an entry  box with ID 20 */
                        /* a.20=[default] presets it                         */
                        /* ------------------------------------------------- */

MyDialog=.MyDlgClass~new /*(a.)*/          /* Create ooDialog Class instance */

                        /* ------------------------------------------------- */
                        /* Here you can initialise attributes that you       */
                        /* associate with your controls below                */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* If you want to create a non-modal dialog or have  */
                        /* it start minimised etc. then change the following */
                        /* clause.  See Show, Execute, ExecuteAsync, Popup&  */
                        /* PopupasChild in the ooDialog Reference            */
MyDialog~execute('ShowTop')     /* Create, show and run the Windows Object   */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Here you have access to methods & Attributes of   */
                        /* the ooDialog Object after OK/Cancel has been      */
                        /* pressed.  ie: MyDialog~Attribute                  */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Code that you want to execute after the Dialog    */
                        /* has been cleared goes here.                       */
                        /*                                                   */
                        /* The stem a. can be used to pass values back from  */
                        /* the dialog.                                       */
                        /* ------------------------------------------------- */

exit
/* ========================================================================= */
/* Error Handling routine - returns full information to console              */
/*                                                                           */
/* All error information available is written to STDOUT (usually the console)*/
/* As this may not be present (running a GUI with REXXHIDE) a ooDialog       */
/* errorDialog popup is also presented                                       */
/*                                                                           */
/* While this type of error handling is useful to some people, it also will  */
/* mask the print out of many syntax errors that happen while your dialog    */
/* is executing.                                                             */
/*                                                                           */
/* If you are having trouble debugging problems in your dialog, 1.) Comment  */
/* out the 'signal on any' line above.  2.) Execute your dialog from a       */
/* console window so that you will see any syntax messages printed out by    */
/* the interpreter.                                                          */
/*                                                                           */
/* Those two steps will solve many of your debugging problems.               */
/* ========================================================================= */
any:

  signal off any

  errObj=condition("o")            /*                   get exception object */
  errObj~"_SIGL_"= SIGL            /*      add value of SIGL and  sourceline */
  errObj~"_sourceline_"= sourceline(SIGL)
  errQ=.queue~new
  Say "******************* An Error has occurred *******************"
  do err.val over errObj
     if errObj~at(err.val)~hasMethod('HasIndex')
     then do                         /* display them after the single liners */
             errq~queue(err.val~right(13,'-')||':')
             do err.line over errObj~at(err.val)
                errq~queue("             :" err.line)
             end /* DO */
             errq~queue("---")
          end /* DO */
     else say err.val~Right(13)||':['errObj~at(err.val)||']'
  end

  do while errq~items>0
     say errq~pull
  end /* DO */

  Say "****************** End of error diagnostics ***************"
                         /* in case there is no console - show error message */

  call errorDialog 'Error' errObj~rc errObj~errortext '0a'x ,
                    'occurred on line' errObj~_sigl_ 'of' ,
                     errObj~program '0a'x ,
                     errObj~Message '0a'x

exit -1
/* ========================================================================= */
::requires "ooDialog.cls"
/* ========================================================================= */
::class MyDlgClass subclass userdialog
/*                                                                           */
/* The class directive                                                       */
/* Rename your object from MyDlgClass to something that makes sense to you   */
/* you will need to change the reference to it in the main program above     */
/*                                                                           */
/* If you are going to be calling this object from other programs using a    */
/* REQUIRES directive then add the keyword PUBLIC to the class directive     */
/*                                                                           */
/*                          ----------------------                           */
/*                                                                           */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/* The init method is called when the dialog is instantiated (by ~new above) */
::method init
/* ------------------------------------------------------------------------- */
/* expose a.  */                /* Give these variables scope of the Object  */
/* use arg a. */                /* Uncomment if you passed a. as a stem      */

/*if you do not want to define initial values with a. do not pass it to super*/
/* otherwise rogue default values may appear in your dialog                  */
  self~init:super /*(a.)*/      /* we call the Super Class (userdialog)      */
  width=300 ; height=200        /* Set the Width and height of dialog        */

                                /* Now we create the Windows Object          */
  success=self~createCenter(width,height,                                    -
                            'This text appears in the Dialog Title',,,       -
                            'MS Sans Serif',8)
  /* The above line creates a dialog in the centre of the screen, if you     */
  /* Would rather specify values for x & y use the line below instead        */
  /* success=self~create(x,y,width,height,Title)                             */

  if \success then do
    self~initCode=1
    return
  end
                        /* ------------------------------------------------- */
                        /* Here we can initialise any attributes of our      */
                        /* dialog.                                           */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Here we can 'connect' dialog item events to       */
                        /* Methods or Attributes.                            */
                        /* i.e.: self~connectListViewEvent(id,"Changed",,    */
                        /*                            "ItemSelectedMethod")  */
                        /*                                                   */
                        /* NB: Many createXXX Methods (which appear in the   */
                        /* defineDialog method below) also provide a way to  */
                        /* define connections.                               */
                        /* ------------------------------------------------- */

/* ------------------------------------------------------------------------- */
::method defineDialog
/* expose menuBar */            /* Perhaps save a menu object if you create  */
                                /* one below.                                */
/* ------------------------------------------------------------------------- */
/* This is where we lay out the controls (widgets) in our dialog             */
/* Refer to the ooDialog manual for the create methods and their parameters  */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* DlgArea is a class of imaginary rectangles with an inner margin that      */
/* allows you to reference coordinates within them by dialog units or        */
/* percentages.                                                              */
/* ------------------------------------------------------------------------- */
/* u = .DlgAreaU~new(self) -- Whole dialog           */
/* a = .DlgArea~new(u~x(''),u~y(''),u~w(''),u~h('')) */

                        /* ------------------------------------------------- */
                        /* Here we add the control objects to our dialog     */
                        /* Dialog Width  is available to us as self~sizeX &  */
                        /* Dialog Height is available to us as self~sizeY    */
                        /* ------------------------------------------------- */

   self~createPushButton(IDOK,self~sizeX-60 ,self~sizeY-20,50,15,'DEFAULT','OK')
   self~createPushButton(IDCANCEL,self~sizeX-120,self~sizeY-20,50,15,,'Cancel')

/* examples to cut & paste:                                                  */
/* self~createPushButton(id,x,y,cx,cy,'options','text','method')             */
/* self~createCheckBox(id,x,y,cx,cy,'options',text,'attribute')              */
/* self~createComboBox(id,x,y,cx,cy,'options','attribute')                   */
/* self~createEdit(id,x,y,cx,cy,'options','attribute')                       */
/* self~createListView(id,x,y,cx,cy,'options','attribute')                   */
/* self~createTreeView(id,x,y,cx,cy,'options','attribute')                   */
/* self~createStaticText(id, x,y,cx,cy,'options','text')                     */
/* self~create[Black|White|Gray]Rect(id,x,y,cx,cy,'options')                 */
/* etc. etc. ...                                                             */
/* other controls should have unique ids over 100 (or -1 for static text)    */

                        /* ------------------------------------------------- */
                        /* You could create a menu here, or really anywhere, */
                        /* using one of the menu classes such as the         */
                        /* .UserMenuBar, and the menu methods.               */
                        /*                                                   */
                        /* Save the menu and have it attach to the dialog in */
                        /* the initDialog() method.  It could be saved in an */
                        /* exposed variable for instance.                    */
                        /* ------------------------------------------------- */

/* menuBar = .UserMenuBar~new(200, self, ...)                                */
/* menuBar~addPopUp(id, 'text', options, ...)                                */
/* menuBar~addItem(id, 'someText', options, ...)                             */
/* menuBar~addSeparator                                                      */
/* menuBar~complete                                                          */
/* ------------------------------------------------------------------------- */
::method initDialog
/*  expose menuBar */   /* If you are adding a menu perhaps.                 */
/* ------------------------------------------------------------------------- */
/* If you have no need to initialise/populate items delete this method       */

                        /* ------------------------------------------------- */
                        /* Code here is run after the underlying windows     */
                        /* dialog object has been created.  Whether the      */
                        /* dialog is displayed depends on the style keywords */
                        /* used in the createCenter(), (or create()) method. */
                        /* By default the dialog will be created invisible.  */
                        /* If you use the VISIBLE keyword, the dialog will   */
                        /* be visisble at this point.                        */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Here we can populate list viws etc.               */
                        /* ------------------------------------------------- */

/*  for example:                                                             */
/*  List = self~newListView([id])                                            */
/*  if List \= .Nil then do                                                  */
/*    list~setImageList(imageList, SMALL)                                    */
/*    list~addStyle("[Style1 style2...]")                                    */
/*    list~insertColumn(0,"[Title]",[width],[style])                         */
/*    list~insertColumn(1,"[Title]",[width],[style])                         */
/*    do data over dataset                                                   */
/*       ordinal=list~addrow(,[icon_no],[column 0 text],[column 1 text]...)  */
/*    end                                                                    */
/*  end                                                                      */

                        /* ------------------------------------------------- */
                        /* If you created a menu in defineDialog(), attach   */
                        /* it:                                               */
                        /* ------------------------------------------------- */

/*  menuBar~attachTo(self) */

/* ------------------------------------------------------------------------- */
::method ok
/* ------------------------------------------------------------------------- */
/* The ok() method is invoked automatically by the ooDialog framework when   */
/* the user pushes or clicks a button, or a menu item, with the resource ID  */
/* of IDOK (1).  If you do not need to do, or do not want to do, any         */
/* processing here, you can delete this method.  The ooDialog framework      */
/* provides the correct implementation for you.  The framework also provides */
/* a default implementation of the validate method.  By default validate()   */
/* returns true.  If you want to have a chance to validate the user's input, */
/* and perhaps prevent the dialog from closing, then over-ride the validate  */
/* method.  From validate() return true to close the dialog, or false to     */
/* prevent the dialog from closing.                                          */

                        /* ------------------------------------------------- */
                        /* If you want to do your validation here, you can   */
                        /* add the valdiation code here.  Then, if you want  */
                        /* to allow the dialog to close normally, invoke the */
                        /* super class's ok() method.  If you want to        */
                        /* prevent dialog from closing, simply return 0 with */
                        /* out invoking the super class ok.                  */
                        /*                                                   */
                        /* By invoking the super class ok() method you       */
                        /* ensure the dialog is closed properly.  That is    */
                        /* really the best way to end the dialog.  The best  */
                        /* way to not end the dialog at this point is to     */
                        /* simply return 0.                                  */
                        /* ------------------------------------------------- */

return self~oK:super

/* ------------------------------------------------------------------------- */
::method cancel
/* ------------------------------------------------------------------------- */
/* The cancel() method is invoked automatically by the ooDialog framework    */
/* when the user pushes or clicks a button, or a menu item, with the         */
/* resource ID of IDCANCEL (2), or the user hits the escape key.  If you do  */
/* not need to do, or do not want to do, any processing here, you can delete */
/* this method.  The ooDialog framework provides the correct implementation  */
/* of the cancel method for you.                                             */

                        /* ------------------------------------------------- */
                        /* You can add code for closing with cancel here, if */
                        /* you want to.  Then, to prevent the dialog from    */
                        /* closing at this point, simply return 0.  To       */
                        /* continue with the normal closing of the dialog,   */
                        /* invoke the super class's cancel method.           */
                        /*                                                   */
                        /* By invoking the super class cancel() method you   */
                        /* ensure the dialog is closed properly.  That is    */
                        /* really the best way to end the dialog.  The best  */
                        /* to not end the dialog at this point is to simply  */
                        /* return 0.                                         */
                        /* ------------------------------------------------- */
return self~cancel:super

/* ------------------------------------------------------------------------- */
::method validate
/* ------------------------------------------------------------------------- */
/* This is called by the ok:super method.  Returning .false stops the dialog */
/* from closing.  Returning .true allows the dialog to close. If you do not  */
/* need this method - delete it                                              */
valid=.true

/*
  valid=.false
  select
     when [error_condition] then call errorDialog [error_condition_message]
     when [error_condition] then call errorDialog [error_condition_message]
     otherwise
        valid=.true
  end /* select */
*/

return valid
/* ------------------------------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* here we can define other attributes and methods   */
                        /* ------------------------------------------------- */


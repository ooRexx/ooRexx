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
/* ========================================================================= */
/* Template for an OODialog GUI by Jon Wolfers www.sahananda.fwbo.net/rexx   */
/* ========================================================================= */
/*                                                                           */
/* How to use this template:                                                 */
/*                                                                           */
/* This template allows you to create dialogs with OODialog without need     */
/* for the resource workshop.                                                */
/*                                                                           */
/* It sets out the class directive and the basic methods for a dialog        */
/*                                                                           */
/* OODialog is very fully featured and it would not be useful to try to      */
/* anticipate and include every possible activity.                           */
/*                                                                           */
/* Category dialogs & Property sheets subclass differently -see the Manual.  */
/*                                                                           */
/* Not all Dialogs require all the methods listed in the template.           */
/* If you are not adding any control through a particular method then you    */
/* can delete that method and let the superclass handle it                   */
/*                                                                           */
/* There is no copywrite applied to the work of preparing this template      */
/* You may copy and adapt it as you like removing whatever you like and      */
/* you are under no obligation to credit me in your work.                    */
/* At the same time, this is provided as-is and I cannot Guarantee that any  */
/* particular Dialog based on this template will work.                       */
/*                                                                           */
/* I am happy to accept corrections or suggestions for improvements          */
/*                                                                           */
/* Sahananda @ wlbc.co.uk                                    May 2005        */
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

MyDialog=.MyDlgClass~new /*(a.)*/          /* Create OODialog Class instance */

                        /* ------------------------------------------------- */
                        /* Here you can initialise attributes that you       */
                        /* associate with your controls below                */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* If you want to create a non-modal dialog or have  */
                        /* it start minimised etc. then change the following */
                        /* clause.  See Show, Execute, ExecuteAsync, Popup&  */
                        /* PopupasChild in the OODialog Reference            */
MyDialog~Execute('ShowTop')     /* Create, show and run the Windows Object   */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Here you have access to methods & Attributes of   */
                        /* the OODialog Object after OK/Cancel has been      */
                        /* pressed.  ie: MyDialog~Attribute                  */
                        /* ------------------------------------------------- */

MyDialog~DeInstall              /* Clean Up                                  */

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
/* As this may not be present (running a GUI with REXXHIDE) a OODialog       */
/* errorDialog popup is also presented                                      */
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
/* ::requires 'winsystm.cls'    -- Uncomment if you inherit VirtualKeyCodes  */
/* ========================================================================= */
::class MyDlgClass subclass userdialog
/* inherit AdvancedControls MessageExtensions VirtualKeyCodes *//*cut & paste*/
/*                                                                           */
/* The class directive                                                       */
/* Rename your object from MyDlgClass to something that makes sense to you   */
/* you will need to change the reference to it in the main program above     */
/*                                                                           */
/* If you are going to be calling this object from other programs using a    */
/* REQUIRES directive then add the keyword PUBLIC to the class directive     */
/*                                                                           */
/*                          ----------------------                           */
/* Multiple inheritance:                                                     */
/*                                                                           */
/* If you are going to use any of these methods:                             */
/*                                                                           */
/* GETCOMBOBOX              GETSCROLLBAR              GETTREECONTROL         */
/* GETRADIOCONTROL          GETLISTCONTROL            GETPROGRESSBAR         */
/* GETSTATICCONTROL         GETSLIDERCONTROL          GETCHECKCONTROL        */
/* GETLISTBOX               GETTABCONTROL             GETEDITCONTROL         */
/* GETBUTTONCONTROL                                                          */
/*                                                                           */
/* Then you must add INHERIT ADVANCEDCONTROLS to the class directive above   */
/*                                                                           */
/*                          ----------------------                           */
/*                                                                           */
/* If you are going to use any of these methods:                             */
/*                                                                           */
/* DEFTREEDRAGHANDLER       CONNECTSLIDERNOTIFY       CONNECTEDITNOTIFY      */
/* DEFLISTDRAGHANDLER       CONNECTTABNOTIFY          CONNECTBUTTONNOTIFY    */
/* DEFLISTEDITHANDLER       CONNECTSCROLLBARNOTIFY    CONNECTLISTNOTIFY      */
/* DEFLISTEDITSTARTER       CONNECTCOMBOBOXNOTIFY     CONNECTTREENOTIFY      */
/* DEFTREEEDITHANDLER       CONNECTLISTBOXNOTIFY      CONNECTCOMMONNOTIFY    */
/* DEFTREEEDITSTARTER                                                        */
/*                                                                           */
/* Then you must add INHERIT MESSAGEEXTENSIONS to the class directive above  */
/*                                                                           */
/*                          ----------------------                           */
/*                                                                           */
/* If you are going to use either of these methods:                          */
/*                                                                           */
/* VCODE                    KEYNAME                                          */
/*                                                                           */
/* Then you must add INHERIT VIRTUALKEYCODES to the class directive above    */
/*                                                                           */
/*                          ----------------------                           */
/*                                                                           */
/* If you inherit more than one mixin class you only need to use the keyword */
/* INHERIT once.  The order of inheritance is not important in this case.    */
/* i.e.: INHERIT ADVANCEDCONTROLS MESSAGEEXTENSIONS VIRTUALKEYCODES          */
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
  rc=self~createCenter(width,height,'This text appears in the Dialog Title',,,,
                                    'MS Sans Serif',8)
  /* The above line creates a dialog in the centre of the screen, if you     */
  /* Would rather specify values for x & y use the line below instead        */
  /* rc=self~Create(x,y,width,height,Title)                                  */

  self~initCode=(rc=0)
                        /* ------------------------------------------------- */
                        /* Here we can initialise any attributes of our      */
                        /* dialog.                                           */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Here we can 'connect' dialog item events to       */
                        /* Methods or Attributes.                            */
                        /* i.e.: self~ConnectListNotify(id,"Changed",,       */
                        /*                            "ItemSelectedMethod")  */
                        /*                                                   */
                        /* NB: Many createXXX Methods (which appear in the   */
                        /* DefineDialog method below) also provide a way to  */
                        /* define connections.                               */
                        /* ------------------------------------------------- */

/* ------------------------------------------------------------------------- */
::method defineDialog
/* ------------------------------------------------------------------------- */
/* This is where we lay out the controls (widgets) in our dialog             */
/* Refer to the OODialog manual for the create methods and their parameters  */
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

   self~createPushButton( 1,self~sizeX-60 ,self~sizeY-20,50,15,'DEFAULT','OK','Ok')
   self~createPushButton( 2,self~sizeX-120,self~sizeY-20,50,15,,'Cancel','Cancel')

/* examples to cut & paste:                                                  */
/* self~createPushButton(id,x,y,cx,cy,'options','text','method')             */
/* self~createCheckBox(id,x,y,cx,cy,'options',text,'attribute')              */
/* self~createComboBox(id,x,y,cx,cy,'options','attribute')                   */
/* self~createEdit(id,x,y,cx,cy,'options','attribute')                       */
/* self~createListView(id,x,y,cx,cy,'options','attribute')                   */
/* self~createTreeView(id,x,y,cx,cy,'options','attribute')                   */
/* self~createStaticText(id, x,y,cx,cy,'options','text')                     */
/* self~create[Black|White|Gray]Rect,x,y,cx,cy,'options',id)                 */
/* etc. etc. ...                                                             */
/* other controls should have unique ids over 10 (or -1 for static text)     */

                        /* ------------------------------------------------- */
                        /* You can add a menu here using self~addMenuItem &  */
                        /* self~addMenuSeperator.                            */
                        /* To display it add a call to setMenu in the        */
                        /* initDialog method                                 */
                        /* ------------------------------------------------- */

/* self~AddPopupMenu('name','options')                                       */
/* self~AddMenuItem('text',id,'options','method')                            */
/* self~AddMenuSeperator                                                     */
/* ------------------------------------------------------------------------- */
::method initDialog
/* ------------------------------------------------------------------------- */
/* If you have no need to initialise/populate items delete this method       */
  self~initDialog:super
                        /* ------------------------------------------------- */
                        /* Here we can populate list boxes etc.              */
                        /* ------------------------------------------------- */

/*  for example:                                                             */
/*  List = self~GetListControl([id])                                         */
/*  if List \= .Nil then do                                                  */
/*    list~setImageList(imageList, .Image~toID(LVSIL_SMALL))                 */
/*    list~AddStyle("[Style1 style2...]")                                    */
/*    list~InsertColumn(0,"[Title]",[width],[style])                         */
/*    list~InsertColumn(1,"[Title]",[width],[style])                         */
/*    do data over dataset                                                   */
/*       ordinal=list~addrow(,[icon_no],[column 0 text],[column 1 text]...)  */
/*    end                                                                    */
/*  end                                                                      */

                        /* ------------------------------------------------- */
                        /* If you defined a menu in Define Dialog, show it:  */
                        /* self~SetMenu                                      */
                        /* ------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* Code here is run after the windows dialog object  */
                        /* has been created, but before it is displayed      */
                        /* ------------------------------------------------- */

/* ------------------------------------------------------------------------- */
::method run
/* ------------------------------------------------------------------------- */
/* If you do not need to add processing to this class you can delete it      */

                        /* ------------------------------------------------- */
                        /* add code here to run after the windows dialog     */
                        /* object is displayed, & before it handles messages */
                        /* ------------------------------------------------- */

  self~run:super               /* this handles message calls/key presses etc */

                        /* ------------------------------------------------- */
                        /* add code here to run just after OK/Cancel methods */
                        /* ------------------------------------------------- */

/* ------------------------------------------------------------------------- */
::method ok
/* ------------------------------------------------------------------------- */
/* If you do not need to add processing to this class you can delete it      */
  self~oK:super                 /* call Self~Validate, set self~InitCode to 1*/

                        /* ------------------------------------------------- */
                        /* add code for closing with OK here                 */
                        /* Self~Finished will be 0 if Self~Validate failed   */
                        /* You can set self~Finished=0 to stop dlg closing   */
                        /* ------------------------------------------------- */

return self~finished
/* ------------------------------------------------------------------------- */
::method cancel
/* ------------------------------------------------------------------------- */
/* If you do not need to add processing to this class you can delete it      */
  self~cancel:super             /*call Self~Validate, set self~InitCode to 2 */

                        /* ------------------------------------------------- */
                        /* add code for closing with cancel here             */
                        /* Self~Finished will be 0 if Self~Validate failed   */
                        /* You can set self~Finished=0 to stop dlg closing   */
                        /* ------------------------------------------------- */
return self~finished
/* ------------------------------------------------------------------------- */
::method validate
/* ------------------------------------------------------------------------- */
/* This is called by the OK:Super Method.  Returning 0 stops dialog closing. */
/* If you do not need this method - delete it.                               */
valid=1

/*
  valid=0
  select
     when [error_condition] then call errorDialog [error_condition_message]
     when [error_condition] then call errorDialog [error_condition_message]
     otherwise
        valid=1
  end /* select */
*/

return valid
/* ------------------------------------------------------------------------- */

                        /* ------------------------------------------------- */
                        /* here we can define other attributes and methods   */
                        /* ------------------------------------------------- */


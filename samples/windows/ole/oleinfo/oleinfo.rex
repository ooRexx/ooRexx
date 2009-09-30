/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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
/****************************************************************************/
/* Name: OLEINFO.REX                                                        */
/* Type: Object REXX Script using OODialog                                  */
/* Resource: OLEINFO.RC                                                     */
/*                                                                          */
/* Description:                                                             */
/* A "small" browser for OLE objects                                        */
/*                                                                          */
/* Attention: You need the IBM Object REXX OODialog to run this.            */
/*                                                                          */
/****************************************************************************/

/* start the main dialog */
MainDialog = .OLEINFO~new
if MainDialog~InitCode = 0 then do
  rc = MainDialog~Execute("SHOWTOP")
  MainDialog~deinstall
end

exit   /* leave program */

::requires "ooDialog.cls"    /* contains the ooDialog classes           */
::requires "WINSYSTM.CLS"    /* used for registry lookup                */

/*****************************************/
/* This routine creates the OLE object   */
/* the code has been placed outside the  */
/* object code to safely catch any error */
/* during creation of the OLE object     */
/*****************************************/
::routine createObject
  use arg target, name
  target~currentObject=.nil
  signal on syntax name returnToObject
  target~currentObject=.OLEObject~new(name,"NOEVENTS")
  signal on syntax
  return
returnToObject:
  signal on syntax
  call RxMessageBox "Error" rc":" errortext(rc)||'0a'x||condition('o')~message, "Error", "OK", "EXCLAMATION"
  return

/*******************************************/
/* This routine calls a method of the OLE  */
/* object. The code has been placed        */
/* outside the object code to safely catch */
/* any error during the invocation         */
/*******************************************/
::routine callMethod
  use arg target, method, self
  resultOfCall = ""
  signal on syntax name callFailed
  interpret "resultOfCall = target~"method
  signal on syntax
  return resultOfCall
callFailed:
  call RxMessageBox "Error" rc":" errortext(rc)||'0a'x||condition('o')~message, "Error", "OK", "EXCLAMATION"
  self~lastError=rc
  signal on syntax
  return resultOfCall


/**************************/
/* Main Dialog of OLEINFO */
/**************************/
::class OLEINFO subclass UserDialog inherit AdvancedControls MessageExtensions

::method Init
  expose cache
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  cache = .nil
  /* load main dialog */
  if self~Load("OLEINFO.rc", 4711 ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  /* Connect dialog control items to class methods */
  self~ConnectButton(200,"MyOk")
  self~ConnectButton(9,"Help")
  self~ConnectButton(103,"Lookup")

  self~ConnectComboBoxNotify(100,"SELCHANGE","Ok")
  self~ConnectListNotify(104,"ACTIVATE","selectDoubleClick")
  self~ConnectListNotify(104,"CHANGED","selectionChange")

  self~ConnectButton(107,"selectionChange")
  self~ConnectButton(108,"selectionChange")
  self~ConnectButton(109,"selectionChange")

  self~currentObject = .nil
  self~currentObjectName = ""
  return InitRet

::method InitDialog
  self~newListView(104)~setImageList(self~getImages, .Image~toID(LVSIL_SMALL))
  cb = self~newComboBox(100)
  default = .array~of("InternetExplorer.Application","Excel.Application","Freelance.Application",,
                      "Notes.NotesSession","Lotus123.Workbook","Outlook.Application",,
                      "Word.Application","WordPro.Application","Access.Application")
  do i over default
    cb~add(i)
  end

  /* Method Ok will be called if enter is pressed in dialog */
::method Ok
  cb = self~newComboBox(100)
  if cb \= .nil then do
    OLEID = cb~Title          /* get ProgID or ClassID of OLE Object */
    if OLEID \= self~currentObjectName then do
      call createObject self, OLEID
      if self~currentObject \= .nil then do
        self~currentObjectName=OLEID
        self~updateView
      end
      else do
        call RxMessageBox "Could not create OLE object", "Error", "OK", "EXCLAMATION"
        cb~title = self~currentObjectName
      end
    end
  end
  return 0  /* don't leave dialog, this is done via method MyOk */

  /* Method MyOk is connected to item 200 */
::method MyOk
  resOK = self~OK:super  /* make sure self~Validate is called and self~InitCode is set to 1 */
  self~Finished = resOK  /* 1 means close dialog, 0 means keep open                         */
  return resOK

  /* Method Help is connected to item 9 */
::method Help
  self~Help:super
  file = .stream~new("help.txt")
  data.500 = file~charin(,file~chars)
  file~close
  temp = .HelpDialog~new(data.)
  if temp~InitCode = 0 then do
    rc = temp~execute("SHOWTOP")
    temp~deinstall
  end

  /* Method Lookup is connected to item 103 */
::method Lookup
  expose cache

  self~Cursor_Wait

  progressBar = self~newProgressBar(110)
  if cache == .nil then do
    cache = .list~new
    registry = .WindowsRegistry~new
    if registry~InitCode \= 0 then return     /* no access to registry? return */
    handle = registry~open(registry~classes_root, "CLSID", "READ")

    clslist. = registry~query(handle)

    registry~list(handle,info.)

    if progressBar \= .nil then do
      progressBar~SetStep(1)
      progressBar~SetRange(0,clslist.subkeys)
    end

    do i =1 to clslist.subkeys
      temphandle = registry~open(handle, info.i, "READ")
      if temphandle \= 0 then do
        templist. = registry~query(temphandle)
        value = self~getProgID(registry,temphandle,progressBar)
        registry~close(temphandle)
        if value \= .nil then cache~insert(value)
      end
    end
    registry~close(handle)
  end

  if progressBar \= .nil then progressBar~SetPos(0)
  self~Cursor_Arrow


  picked = ""
  temp = .RegistryDialog~new(,cache)
  if temp~InitCode = 0 then do
    rc = temp~execute("SHOWTOP")
    if rc =1 then do
      combo = self~newComboBox(100)
      combo~title = temp~data200
      picked = combo~title
      self~ok
    end
    temp~deinstall
  end
  cb = self~newComboBox(100)
  if cb \= .nil then do
    cb~DeleteAll
    do i over cache
      cb~add(i)
    end
    if picked \== "" then cb~title = picked
  end

  /* extract ProgID from registry */
::method getProgID
  use arg registry, handle, progressBar
  res = .nil

  /* try to get a version independent ProgID first */
  temphandle = registry~open(handle, "VersionIndependentProgID", "READ")

  if progressBar \= .nil then progressBar~step

  if temphandle \= 0 then do
    registry~listvalues(temphandle,info.)

    res = info.1.data

    registry~close(temphandle)
  end
  else do
    /* this failed, so maybe there's a "normal" ProgID? */
    temphandle = registry~open(handle, "ProgID", "READ")

    if temphandle \= 0 then do
      registry~listvalues(temphandle,info.)

      res = info.1.data

      registry~close(temphandle)
    end
  end

  return res


::method currentObject ATTRIBUTE                    /* store the current object */
::method currentObjectName ATTRIBUTE                /* store object's name      */

  /* update the list of methods and events */
::method updateView
  expose indexStem. methods. events.
  lc = self~newListView(104)
  if lc \= .nil then do
    methods. = self~currentObject~GetKnownMethods   /* retrieve info on methods   */
    if methods. = .nil then do
      temp = RxMessageBox("OLE Object did not return any information on known methods.","Information","OK","INFORMATION")
      lc~DeleteAll                                  /* remove all items from list */
      return
    end
    events. = self~currentObject~GetKnownEvents     /* retrieve info on events    */
    if events. = .nil then events.0 = 0

    lc~DeleteAll                                    /* remove all items from list */
    if var("methods.!LIBNAME") = 1 then
      self~newEdit(101)~title = methods.!LIBNAME
    else
      self~newEdit(101)~title = "unavailable"
    if var("methods.!LIBDOC") = 1 then
      self~newEdit(102)~title = methods.!LIBDOC
    else
      self~newEdit(102)~title = "unavailable"

    /* collect the indices of the info stem ordered according to their method names */
    indexStem.0 = 0

    self~Cursor_Wait
    pbc = self~newProgressBar(110)
    if pbc \= .nil then do
      pbc~SetStep(1)
      pbc~SetRange(0,methods.0 + events.0)
    end

    do i = 1 to methods.0 + events.0
      if i <= methods.0 then do
        /* add method name to list box */
        if methods.i.!INVKIND \= 4 then
          j = lc~add(methods.i.!NAME, (methods.i.!INVKIND)/2)
        else                                     /* this is a property put, symbolize with "=" */
          j = lc~add(methods.i.!NAME||"=",2)
        end
      else do
        k = i - methods.0
        j = lc~add(events.k.!NAME, 3)
      end

      j=j+1
      if i \= j then do
        do k = indexStem.0 to j by -1
          t = k + 1
          indexStem.t = indexStem.k
        end
        indexStem.j = i
      end
      else
        indexStem.i = i
      indexStem.0 = indexStem.0 + 1
      if pbc \= .nil then pbc~Step
    end
    self~Cursor_Arrow
    if pbc \= .nil then pbc~SetPos(0)
  end


  /* displays information on the selected method */
::method selectionChange
  expose indexStem. methods. events.

  listbox=self~newListView(104)

  j = 1 + listbox~Selected
  if j < 1 then return                              /* return if nothing was selected */
  i = indexStem.j

  types = self~getCheckBoxData(107)
  flags = self~getCheckBoxData(108)

  if i > methods.0 then do
    workstem. = events.
    i = i - methods.0
    infostring = ""
  end
  else do
    workstem. = methods.
    memberID = self~getCheckBoxData(109)
    /* show member ID? */
    if memberID \= 0 then
      infostring = "['"||workstem.i.!MEMID||"'x] "
    else
      infostring = ""
    /* show return type? */
    if types \= 0 then
      infostring = infostring||workstem.i.!RETTYPE||" "
  end

  if methods.i.!INVKIND = 4 then                      /* property put */
    infostring = infostring||workstem.i.!NAME||"="
  else                                                /* normal method or property get */
  do
    /* build method signature with name(...) */
    infostring = infostring||workstem.i.!NAME
    do j = 1 to workstem.i.!PARAMS.0
      if j = 1 then infostring = infostring||"("
      /* show flags? */
      if flags \= 0 then
        infostring = infostring||workstem.i.!PARAMS.j.!FLAGS||" "
      /* show types? */
      if types \= 0 then
        infostring = infostring||workstem.i.!PARAMS.j.!TYPE||" "
      /* show name of argument */
      infostring = infostring||workstem.i.!PARAMS.j.!NAME
      if j < workstem.i.!PARAMS.0 then
        infostring = infostring||", "
      else infostring = infostring||")"
    end
  end

  /* set string to dialog */
  signature = self~newEdit(105)
  if signature \= .nil then
    signature~title = infostring
  desc = self~newEdit(106)
  /* show documentation if available */
  if desc \= .nil then do
    interpret 'exists = var("workstem.'i'.!DOC")'
    if exists = 1 then
      desc~title = workstem.i.!DOC
    else
      desc~title="unavailable"
  end

  /* invoke method */
::method selectDoubleClick
  expose indexStem. methods.

  listbox=self~newListView(104)

  j = 1 + listbox~Selected
  i = indexStem.j

  if i > methods.0 then do
    call RxMessageBox "This is an event!"||'0a'x||"Build a subclass of OLEObject and add a method with",
                      "this name if you wish REXX to call it when this event occurs.", "Information", "OK", "EXCLAMATION"
    return
  end

  params.0 = methods.i.!PARAMS.0
  do j = 1 to params.0
    params.j.!NAME = methods.i.!PARAMS.j.!NAME
    params.j.!FLAGS = methods.i.!PARAMS.j.!FLAGS
    params.j.!TYPE = methods.i.!PARAMS.j.!TYPE
  end

  aDialog=.invokeDialog~new(,params.)
  aDialog~create(100,100,200,26+13*params.0,"Method invocation:" methods.i.!NAME)
  rc = aDialog~execute("showtop")
  aDialog~deinstall
  if rc = 1 then do
    execString = methods.i.!name
    usesOutParms = .FALSE
    self~LastError = 0
    /* method call? */
    if methods.i.!INVKIND = 1 then do
      if params.0 > 0 then execString = execString"("
      do i = 1 to params.0
        interpret "value = aDialog~param"i
        execString = execString || value
        if i < params.0 then execString = execString", "
        if params.i.!FLAGS~pos("out") > 0 then usesOutParms = .TRUE
      end
      if params.0 > 0 then execString = execString")"
      resultOfCall = callMethod(self~currentObject, execstring, self)
    end
    /* property put? */
    if methods.i.!INVKIND = 4 then do
      value = aDialog~param1                 /* can only have one argument */
      interpret "self~currentObject~"execString "=" value
      resultOfCall = ""
    end
    /* property get? */
    if methods.i.!INVKIND = 2 then do
      interpret "resultOfCall = self~currentObject~"execString
    end

    if usesOutParms = .TRUE then
      outp = self~currentObject~GetOutParameters
    else
      outp = .nil

    if self~LastError \= 0 then call RxMessageBox ERRORTEXT(self~LastError), "Error", "OK", "EXCLAMATION"
    else do
      temp = .ResultDialog~new(resultOfCall,outp)
      if temp~initcode = 0 then do
        temp~execute("showtop")
        /* if useOLEobject attribute is filled in, change browser to this object */
        if temp~useoleobject \= .nil then do
           self~currentObject = temp~useOLEobject
           self~currentObjectName = "??? (from execution)"
           self~newComboBox(100)~title = self~currentObjectName
           self~updateView
        end
        temp~deinstall
      end
    end

  end

::method getImages private
  image = .Image~getImage("icons.bmp")
  imageList = .ImageList~create(.Size~new(16, 12), .Image~toID(ILC_COLOR4), 6, 0)
  if \image~isNull,  \imageList~isNull then do
      imageList~add(image)
      image~release
      return imageList
  end
  return .nil

::method LastError ATTRIBUTE



/*************************************/
/* Dialog for invoking an OLE method */
/*************************************/
::CLASS invokeDialog SUBCLASS UserDialog INHERIT AdvancedControls

::METHOD resultObject ATTRIBUTE     /* takes the result of the invocation */

::METHOD init
  expose params.
  use arg initstem., params.
  self~init:super

::METHOD DefineDialog
  expose params.

  self~DefineDialog:super

  do i = 1 to params.0
    self~createEdit(300+i, 64, -5+(13*i), 128, 11, "AUTOSCROLLH", "Param"i)
    self~createStaticText(-1, 8, -5+(13*i), 56, 11, , params.i.!NAME)
  end
  self~addOkCancelRightBottom

::METHOD InitDialog
  expose params.
  self~resultObject = .nil
  do i = 1 to params.0
    /* plain out parameters can not be edited */
    if params.i.!FLAGS = "[out]" then do
      interpret "self~Param"i"='.NIL'"
      self~newEdit(300+i)~disable
    end
    /* set .true if BOOL expected */
    if params.i.!TYPE = "VT_BOOL" then interpret "self~param"i"='.TRUE'"
    /* set empty string if string expected */
    if params.i.!TYPE = "VT_BSTR" then interpret "self~param"i"='""""'"
  end


::METHOD ok
  resOK = self~OK:super  /* make sure self~Validate is called and self~InitCode is set to 1 */
  self~Finished = resOK  /* 1 means close dialog, 0 means keep open                         */
  return resOK





/*******************************************************/
/* Dialog that shows the result of a method invocation */
/*******************************************************/
::class ResultDialog subclass UserDialog inherit AdvancedControls

::method Init
  expose outarray
  use arg rvalue, outarray
  InitRet = self~init:super

  if self~Load("OLEINFO.rc", 4713 ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  self~ConnectButton(1,"Ok")

  self~data400=rvalue~string
  if self~data400 = "an OLEOBJECT" then self~useOLEobject = rvalue
  else self~useOLEobject = .nil
  return InitRet

::method InitDialog
  expose outarray

  lc = self~newListBox(401)

  if outarray \= .nil then do
    if lc \= .nil then do
      i = 1
      do j over outarray
        lc~add(i||'09'x||j~string)
        i = i + 1
      end
    end
  end
  else
    if lc \= .nil then do lc~add("object did not return out parameters")
  end

::METHOD ok
  resOK = self~OK:super  /* make sure self~Validate is called and self~InitCode is set to 1 */
  self~Finished = resOK  /* 1 means close dialog, 0 means keep open                         */
  if (resOK = 1) & self~useOLEobject \= .nil then do
    keep = RxMessageBox("An OLE object was returned from the method invocation. Do you want to use it as the active object?","Question","OKCANCEL","QUESTION")
    if keep \= 1 then self~useOLEobject = .nil
  end
  return resOK

::method useOLEobject ATTRIBUTE






/********************************************************/
/* Dialog that shows all ProgIDs obtained from Registry */
/********************************************************/
::class RegistryDialog subclass UserDialog inherit AdvancedControls

::method Init
  expose cache
  use arg stem., cache
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~Load("OLEINFO.rc", 4712 ) \= 0 then do
     self~InitCode = 1
     return 1
  end
  self~ConnectListLeftDoubleClick(200,"selectDoubleClick")
  self~ConnectButton(201,"search")

  return InitRet

::method GetCache
  expose cache
  return cache

::method InitDialog
  expose cache
  lc = self~newListBox(200)
  if lc \= .nil then do
    do item over cache
      lc~add(item)
    end
  end

::method OK
  res = self~OK:super
  return res

  /* double clicking in the list closes this dialog window */
::method selectDoubleClick
  self~OK  /* make sure self~Validate is called and self~InitCode is set to 1 */

::method search
  dlg = .InputBox~new("String:", "String search", "", 150)
  value = dlg~Execute
  drop dlg

  lb = self~newListBox(200)
  startindex = lb~selectedindex
  lb~selectindex(lb~find(value,startindex,0))



/******************************/
/* Dialog that shows the help */
/******************************/
::class HelpDialog subclass UserDialog

::method Init
  forward class (super) continue /* call parent constructor */
  InitRet = Result

  if self~Load("OLEINFO.rc", 4714 ) \= 0 then do
     self~InitCode = 1
     return 1
  end

  return InitRet

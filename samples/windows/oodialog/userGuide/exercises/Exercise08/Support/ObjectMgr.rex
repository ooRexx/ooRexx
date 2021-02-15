/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2014 Rexx Language Association. All rights reserved.    */
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
/* ooDialog User Guide

   Support - ObjectMgr						 v01-01  06Jun13
   -------------------
   A singleton component that manages model objects.

   Should ensure that only one instance is created. This version doesn't.

   Provides an object reference given a class and an instance name. Stores these
   in an "object bag", which is a table of classes, where each table item is a
   table of instance names, the item being the object reference.

   Interface ObjectMgr {
     bool    init
     cmptId  getComponentId( in string className, in string instanceName )
     bool    addCmptRef( in string className, in string instanceName, in objref cmptRef )
     void    list( )
     bool    showModel( in string modelClass, in string modelInstance )
  }

  Changes:
    v01-00 23Apr12: First version.
           11Jan13: Commented-out 'say' instructions.
           21Jan13: Make 'addView' private and 'removeView' explicitly public.
                    Minor typos in comments corrected.
    v01-01 06Jun13: Added methods 'modelClassFromView and 'modelIdFromView -
                    given a view (a dialog), which return the model class name
                    and model object id respectively. Purpose: to support
                    Drag/Drop.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


call "RequiresList.rex"


/*//////////////////////////////////////////////////////////////////////////////
  ============================================================================*/
::CLASS 'ObjectMgr' PUBLIC

  ::ATTRIBUTE objectBag PRIVATE-- a bag of objects - i.e. instances of Distributed
                        -- Components or DCs.

  /*----------------------------------------------------------------------------
    init - initialises the ObjectManager
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    expose objectBag viewBag
    --say "ObjectMgr-init. Classes:" .CustomerModel .CustomerData .CustomerView
    .local~my.ObjectMgr = self
    objectBag = .directory~new		-- holds "class-inst" "id-ViewName"
    viewBag = .directory~new	-- holds view class and latest view instance.
    return .true
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    getComponentId - Returns a Component Id if it's in the ObjectBag,
                     else calls doNewInstance to get id, else returns .false.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD getComponentId PUBLIC
    expose objectBag
    use strict arg className, instanceName
    className = className~upper; instanceName = instanceName~upper
    --say "ObjectMgr-getComponentId-00: className =" className "instanceName =" instanceName
    ObjectName = className||"-"||instanceName
    if objectBag~hasIndex(objectName) then do		-- if class-instance already registered:
      --say "ObjectMgr-getComponentId-01: Class Found:" objectName
      arr = objectBag[objectName]			-- Get info array for this class-instance.
      componentId = arr[1]
      return componentId				-- return component id
    end
    -- If we've got to here, then there's no id stored. So go get one:
    componentId = self~doNewInstance(className,instanceName)
    --say "ObjectMgr-getComponentId-03: componentId =" componentId
    if componentId = .false then return .false		-- Bad object name
    self~addComponentId(className,instanceName,componentId)
    return componentId
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*----------------------------------------------------------------------------
    doNewInstance - Instantiates the requested object and returns the object id
                     (aka component id). If class not found, or class returns
                     .false to ~newInstance, then returns .false.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD doNewInstance PRIVATE
    use strict arg className, instanceName
    SIGNAL ON NOMETHOD NAME catchIt
    interpret "componentId = ."||className||"~newInstance("||"'"||instanceName||"'"||")"
    --say "ObjectMgr-doNewInstance-01: componentId =" componentId
    -- add to object bag:
    return componentId
    catchIt:
      say "ObjectMgr-doNewInstance-02: component" className instanceName "not found."
      return .false
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*----------------------------------------------------------------------------
    addComponentId - Adds an Instance of a Class to the Object Bag.
                     If instance already exists, return .false
     Format of ObjectBag (note - directory indices are case-sensitive, i.e.
     if entry is created with index "AbC" then 'say dir["ABC")' may return nil.

     +---------------------------------+
     | Index      |  Item (an Array)   |
     +---------------------------------+
     |            |       | View       |
     | Class-Inst | objId | Class-Inst |
     |---------------------------------|
     | PersM-AB12 | x'12' | PersV-1    |
     |- - - - - - |- - - -|- - - - - - |
     | PersM-CD34 | x'5A' | .nil       |
     |- - - - - - |- - - -|- - - - - - |
     | PersV-1    | x'7B' | .nil       |
     |- - - - - - |- - - -|- - - - - - |
     |            |       |            |
     +---------------------------------+

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::method addComponentId PRIVATE
    expose objectBag
    use strict arg className, instanceName, componentId
    --say "ObjectMgr-addComponentId-01:" classname instancename componentId
    objectName = className||"-"||instanceName
    arr = .Array~new
    arr[1] = componentId
    arr[2] = .nil		-- Space for a View Name ('class-inst')
    objectBag[objectName] = arr
    --self~list
    return
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/


  /*----------------------------------------------------------------------------
    showModel - Shows a view of a model. Lacking a config file that links
                model class to one or more View classes, we heroically assume
                that all models are called "xxxModel" and its View is called
                xxxView. Assume only one View per Model.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showModel PUBLIC
    expose objectBag
    use arg modelClass, modelInstance, parentDlg
    --say "ObjectMgr-showModel-01a - modelNames:" modelClass modelInstance
    --say "ObjectMgr-showModel-01b - parentDlg: " parentDlg
    --say "ObjectMgr-showModel-02 - modelClass: '"||modelClass||"';  modelInstance: '"||modelInstance||"'"
    -- If this is an "anonymous" component (instance name "A"|"a"), ask its
    -- class object for an instance name:
    if modelInstance = "A" | modelInstance = "a" then do
      anonModelClass = "."||modelClass
      interpret "modelInstance = "||anonModelClass||"~getInstanceName"
      --say "ObjectMgr-showModel-02 - modelInstance: " modelInstance
    end
    modelId = self~getComponentId(modelClass, modelInstance)
    --say "ObjectMgr-showModel-03 - modelId:" modelId
    if modelId = .false then do
      say "ObjectMgr-showModel-03b: Model" modelClass modelInstance "could not be found."
      return .false
    end

    -- Check if View exists - if so, surface it:
    modelName = modelClass||"-"||modelInstance
    modelName = modelName~upper
    arr = objectBag[modelName]
    viewName = arr[2]
    --say "ObjectMgr-showModel-03c: ViewName =" viewName
    if viewName \= .nil then do		-- if view exists
      arr = objectBag[viewName]
      viewId = arr[1]
      viewId~show("SHOWTOP")		-- if minimized, doesn't work.
      viewId~show("NORMAL")		-- if hidden, doesn't work
      return .true
    end

    -- View does not exist - so construct the view name and send the class a
    --   newInstance(modelId, parentDlg).
    parse var modelClass root "Model"
    viewClass = root||"View"
    viewClassId = "."||viewClass
    --say "ObjectMgr-showModel-04: viewClassId =" viewClassId
    interpret "targetObject =" viewClassId
    --say "ObjectMgr-showModel-05: parentDlg =" parentDlg
    msg = .Message~new(targetObject, "newInstance", "I", modelId, parentDlg)
    --say "ObjectMgr-showModel-06: Class is:" .CustomerView .ObjectMgr
    viewId = msg~send
    --say "ObjectMgr-showModel-07 - viewId:" viewId

    self~addView(modelClass, modelInstance, viewClass, viewId)
    --self~addView(modelClass, modelInstance, "CustomerView", viewId)
    return .true
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    modelIdFromView - Returns the model for a given view.
                        (Added to support drag/drop)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD modelIdFromView PUBLIC
    expose objectBag
    use strict arg viewId
    parse var viewId . viewClassName
    --say "ObjectMgr-modelIdFromView-01: viewName =" viewClassName
    viewInstanceName = viewId~identityHash
    searchClassInst = viewClassName||"-"||viewInstanceName
    do label myLoop i over objectBag
      arr = objectBag[i]
      viewClassInst = arr[2]
      if viewClassInst = searchClassInst then do
        modelId = arr[1]
        leave myLoop
      end
    end myLoop
    --say "ObjectMgr-modelIdFromView-02: modelId =" modelId
    return modelId

  /*----------------------------------------------------------------------------
    modelClassFromView - Returns the class name of the model for a given view.
                        (Added to support drag/drop)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD modelClassFromView PUBLIC
    expose objectBag
    use strict arg viewId	-- the object id of a View dialog.
    modelName = .nil
    parse var viewId . viewClassName
    --say "ObjectMgr-modelClassFromView-01: viewName =" viewClassName
    viewInstanceName = viewId~identityHash
    searchClassInst = viewClassName||"-"||viewInstanceName
    do label myLoop i over objectBag
      arr = objectBag[i]
      viewClassInst = arr[2]
      if viewClassInst = searchClassInst then do
        modelName = i
        leave myLoop
      end
    end myLoop
    parse var modelName className "-" .
    --say "ObjectMgr-modelClassFromView-02: modelName, className =" modelName||"," classname
    return className


  /*----------------------------------------------------------------------------
    addView - Adds a View to the ObjectBag.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD addView PRIVATE
    expose objectBag --viewBag
    use strict arg modelClass, modelInstance, viewClass, viewId
    --say "ObjectMgr-addView-01."
    -- Get view's instanceName
    viewInstance = viewId~identityHash

    -- First, add View name to arr[2] of Model's entry in ObjectBag:
    modelName = modelClass||"-"||modelInstance
    modelName = modelName~upper()
    viewClass = viewClass~upper()
    viewName  = viewClass||"-"||viewInstance
    arr = objectBag[modelName]
    arr[2] = viewName
    -- Now add the View to the ObjectBag:
    self~addComponentId(viewClass, viewInstance, viewId)
    --say "ObjectMgr-addView-02: list with new View class:"
    --self~list
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    removeView - Removes a view from the ObjectBag. (Used by RcView & ResView.)
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD removeView PUBLIC
    expose objectBag
    use arg viewClass, viewInstance
    viewClass = viewClass~upper()	-- View class was uppered in addView method.
    viewClassInst = viewClass||"-"||viewInstance
    --say "ObjectMgr-removeView-01: viewClassInst:" "'"||viewClass||"'" "'"||viewClassInst||"'"
    r = objectBag~remove(viewClassInst)
    --say "ObjectMgr-removeView-02: r =" r
    do i over objectBag
      arr = objectBag[i]
      if arr[2] = viewClassInst then arr[2] = .nil
    end
    --say "ObjectMgr-removeView-03: ObjectBag List:"
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*-----------------------------------------------------------------------
    list - Produces a list on stdout of the classes and their Instances
           that are in the Object Bag.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ::METHOD list PUBLIC
    expose objectBag viewBag
    say; say "Object Bag List:"
    say "  ----------------------------------------------------------------------------"
    say "  Class-Instance            Model Id                  ViewClass-Inst"
    say "  ------------------------  ------------------------  ------------------------"
    do name over objectBag
      arrItems = objectBag[name]
      viewClass = arrItems[2]
      if viewClass = .nil then viewClass = ".nil"
      modelId = arrItems[1]
      --say "ObjectBag-list-01: ModelID =" modelId ";" modelId~objectName
      say " " name~left(25) modelId~objectName~left(25) viewClass~left(25)
    end
    say "  ----------------------------------------------------------------------------"
    say
    /*say; say "Latest View Instances:"
    say "  View Class                 Instance"
    say "  -------------------------    ---"
    do viewClass over viewBag
      viewNo = viewBag[viewClass]
      say viewClass~left(25) viewNo~right(7)
    end
    say
    */
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/


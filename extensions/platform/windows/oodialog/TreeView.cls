/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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

/**
 * Windows Dialog Interface for Open Object Rexx (ooRexx.)
 *
 * TreeView Class.
 */

::class 'TreeControl' subclass DialogControl public
::class 'TreeView' subclass TreeControl public

-- TV_FIRST == 0x1100  (4352)
::constant TVM_DELETEITEM          "0x1101"
::constant TVM_EDITLABELA          "0x110E"
::constant TVM_EDITLABELW          "0x1141"
::constant TVM_ENDEDITLABELNOW     "0x1116"
::constant TVM_ENSUREVISIBLE       "0x1114"
::constant TVM_GETCOUNT            "0x1105"
::constant TVM_GETINDENT           "0x1106"
::constant TVM_GETUNICODEFORMAT    "0x2006"
::constant TVM_GETVISIBLECOUNT     "0x1110"
::constant TVM_SETINDENT           "0x1107"
::constant TVM_SORTCHILDREN        "0x1113"

::attribute rootArray private

::method init
   forward class (super) continue
   self~rootArray = .array~new(4)


::method add unguarded
   do i = 1 to arg()
      if arg(i, 'E') then do
          if arg(i + 1, 'E') then image    = arg(i + 1); else image = -1
          if arg(i + 2, 'E') then selImage = arg(i + 2); else selImage = image
          if arg(i + 3, 'E') then opts     = arg(i + 3); else opts = ""
          if arg(i + 4, 'E') then children = arg(i + 4); else children = 0
          if arg(i + 5, 'E') then itemData = arg(i + 5); else itemData = .nil
          if i = 1 then do
              self~rootArray[i] = self~insert("ROOT", , arg(i), image, selimage, opts~translate, children, itemData)
              return self~rootArray[i]
          end
          else if self~rootArray~hasIndex(i - 1) then do
              if i = 1 then parent = "ROOT"; else parent = self~rootArray[i - 1]
              self~rootArray[i] = self~insert(parent, , arg(i), image, selimage, opts~translate, children, itemData)
              return self~rootArray[i]
          end
          else return 0
      end
   end

::method child unguarded external "LIBRARY oodialog tv_getNextItem"
::method collapse unguarded external "LIBRARY oodialog tv_expand"
::method collapseAndReset unguarded external "LIBRARY oodialog tv_expand"
::method delete unguarded external "LIBRARY oodialog tv_delete"
::method deleteAll unguarded external "LIBRARY oodialog tv_deleteAll"
::method dropHighlight unguarded external "LIBRARY oodialog tv_selectItem"
::method dropHighlighted unguarded external "LIBRARY oodialog tv_getSpecificItem"
::method edit unguarded
   use strict arg hitem
   if self~usingUnicode then return self~sendWinHandle2MsgH(self~TVM_EDITLABELW, 0, hItem)
   else return self~sendWinHandle2MsgH(self~TVM_EDITLABELA, 0, hItem)

::method endEdit unguarded
   use strict arg cancel = .false
   return (self~sendWinIntMsg(self~TVM_ENDEDITLABELNOW , cancel, 0) <> 0)

::method ensureVisible unguarded
   use strict arg hItem
   return self~sendWinHandle2Msg(self~TVM_ENSUREVISIBLE, 0, hItem)

::method expand unguarded external "LIBRARY oodialog tv_expand"
::method find unguarded external "LIBRARY oodialog tv_find"
::method firstVisible unguarded external "LIBRARY oodialog tv_getSpecificItem"
::method getImageList unguarded external "LIBRARY oodialog tv_getImageList"
::method getItemData unguarded external "LIBRARY oodialog tv_getItemData"
::method getItemHeight unguarded external "LIBRARY oodialog tv_getItemHeight"
::method getItemRect unguarded external "LIBRARY oodialog tv_getItemRect"
::method getStateImage unguarded external "LIBRARY oodialog tv_getStateImage"
::method getToolTips unguarded external "LIBRARY oodialog generic_getToolTips"
::method hitTestInfo unguarded external "LIBRARY oodialog tv_hitTestInfo"
::method indent unguarded
   use strict arg
   return self~sendWinIntMsg(self~TVM_GETINDENT, 0, 0)

::method "indent=" unguarded
   use strict arg indentAmount
   return self~sendWinIntMsg(self~TVM_SETINDENT, indentAmount, 0)

::method insert unguarded external "LIBRARY oodialog tv_insert"
::method isAncestor unguarded
   use strict arg hParent, hItem
   if arg(1, 'O') | hParent = 0 then return .false
   if arg(2, 'O') | hItem = 0 then return .false
   if hItem = hParent then return .false
   ancestor = self~parent(hItem)
   do while ancestor \= 0 & ancestor \= hParent
       ancestor = self~Parent(ancestor)
   end
   if ancestor = hParent then return .true
   else return .false

::method itemInfo unguarded external "LIBRARY oodialog tv_itemInfo"
::method items unguarded
   use strict arg
   return self~sendWinIntMsg(self~TVM_GETCOUNT, 0, 0)

::method itemText unguarded external "LIBRARY oodialog tv_itemText"
::method makeFirstVisible unguarded external "LIBRARY oodialog tv_selectItem"
::method modify unguarded external "LIBRARY oodialog tv_modify"
::method moveItem unguarded
   use strict arg hItem, hNewParent, redraw = .true, extended = ""
   if hItem = 0 then return 0
   if hNewParent = 0 then return 0

   extended = extended~translate
   if hItem = hNewParent | hNewParent = self~parent(hItem) | self~isAncestor(hItem, hNewParent) then return 0
   iinfo. = self~itemInfo(hItem)
   newRoot = self~insert(hNewParent, , iinfo.!Text, iinfo.!Image, iinfo.!SelectedImage, iinfo.!State, iinfo.!Children, -
                         iinfo.!itemData)

   if iinfo.!Children = 1 then do
      child = self~child(hItem)
      if child \= 0 then self~moveItem(child, newRoot, 0, "SIBLINGS")
   end

   if extended = "SIBLINGS" then do
      sibling = self~next(hItem)
      do while sibling \= 0
         self~moveItem(sibling, hNewParent, 0, "NODELETE")
         oldSib = sibling
         sibling = self~next(sibling)
         self~delete(oldSib)
      end
   end

   if extended \= "NODELETE" then self~delete(hItem)
   if redraw then self~update
   return newRoot

::method next unguarded external "LIBRARY oodialog tv_getNextItem"
::method nextVisible unguarded external "LIBRARY oodialog tv_getNextItem"
::method parent unguarded external "LIBRARY oodialog tv_getNextItem"
::method previous unguarded external "LIBRARY oodialog tv_getNextItem"
::method previousVisible unguarded external "LIBRARY oodialog tv_getNextItem"
::method removeItemData external "LIBRARY oodialog tv_removeItemData"
::method root unguarded external "LIBRARY oodialog tv_getSpecificItem"
::method select unguarded external "LIBRARY oodialog tv_selectItem"
::method selected unguarded external "LIBRARY oodialog tv_getSpecificItem"
::method setImageList unguarded external "LIBRARY oodialog tv_setImageList"
::method setItemData unguarded external "LIBRARY oodialog tv_setItemData"
::method setItemHeight unguarded external "LIBRARY oodialog tv_setItemHeight"
::method setStateImage unguarded external "LIBRARY oodialog tv_setStateImage"
::method setToolTips unguarded external "LIBRARY oodialog generic_setToolTips"

-- The old code did not have recurse arg and had a comment saying "recursive not yet supported"
-- Although MSDN documents this message as recursively sorting the children if WPARAM is true,
-- it doesn't seem to work.
::method sortChildren unguarded
   use strict arg hitem, recurse = .false
   return self~sendWinHandle2Msg(self~TVM_SORTCHILDREN, recurse, hItem)

::method sortChildrenCB external "LIBRARY oodialog tv_sortChildrenCB"
::method toggle unguarded external "LIBRARY oodialog tv_expand"

::method visibleItems unguarded
   use strict arg
   return self~sendWinIntMsg(self~TVM_GETVISIBLECOUNT, 0, 0)

::method test unguarded external "LIBRARY oodialog tv_test"


-- DEPRECATED
::method hitTest unguarded
   forward message "hitTestInfo" continue
   info = result
   if info~hItem = 0 then return 0
   else return info~hItem info~location  -- This looks goofy, but it is what the original code did

::method removeImages unguarded
   return self~setImageList(.nil, 0)

-- DEPRECATED
::method restoreEditClass

-- DEPRECATED
::method setImages unguarded external "LIBRARY oodialog tv_setImageList"

-- DEPRECATED
::method subclassEdit


::class 'TvCustomDrawSimple' public
::method init class external       "LIBRARY oodialog tvcds_init_cls"
::method init external             "LIBRARY oodialog tvcds_init"
::attribute clrText set external   "LIBRARY oodialog tvcds_setClrText"
::attribute clrTextBk set external "LIBRARY oodialog tvcds_setClrTextBk"
::attribute drawStage get external "LIBRARY oodialog tvcds_getDrawStage"
::attribute font set external      "LIBRARY oodialog tvcds_setFont"
::attribute id get external        "LIBRARY oodialog tvcds_getID"
::attribute item get external      "LIBRARY oodialog tvcds_getItem"
::attribute itemData get external  "LIBRARY oodialog tvcds_getItemData"
::attribute reply set external     "LIBRARY oodialog tvcds_setReply"
::attribute level get external     "LIBRARY oodialog tvcds_getLevel"

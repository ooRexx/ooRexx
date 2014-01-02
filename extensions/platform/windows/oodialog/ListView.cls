/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * ListView and list-view helper Classes.
 */

::class 'ListControl' subclass DialogControl public
::class 'ListView' subclass ListControl public

-- LVM_FIRST = 0x1000  == 4096
::constant LVM_GETCOLUMNWIDTH     "0x101D"
::constant LVM_SETHOVERTIME       "0x1047"
::constant LVM_GETHOVERTIME       "0x1048"
::constant LVM_DELETECOLUMN       "0x101C"
::constant LVM_GETITEMSPACING     "0x1033"
::constant LVM_SETITEMCOUNT       "0x102F"
::constant LVM_SCROLL             "0x1014"
::constant LVM_GETCOUNTPERPAGE    "0x1028"
::constant LVM_ENSUREVISIBLE      "0x1013"
::constant LVM_UPDATE             "0x102A"
::constant LVM_REDRAWITEMS        "0x1015"
::constant LVM_GETSELECTEDCOUNT   "0x1032"
::constant LVM_GETITEMCOUNT       "0x1004"
::constant LVM_EDITLABELA         "0x1017"
::constant LVM_EDITLABELW         "0x1076"
::constant LVM_GETTOPINDEX        "0x1027"

::constant IMAGECALLBACK          -1
::constant IMAGENONE              -2
::constant GROUPIDCALLBACK        -1
::constant GROUPIDNONE            -2

::constant LVCFMT_LINE_BREAK          1048576   -- 0x100000
::constant LVCFMT_FILL                2097152   -- 0x200000
::constant LVCFMT_WRAP                4194304   -- 0x400000
::constant LVCFMT_NO_TITLE            8388608   -- 0x800000
::constant LVCFMT_TILE_PLACEMENTMASK  3145728   -- (LVCFMT_LINE_BREAK | LVCFMT_FILL)

::method add unguarded external "LIBRARY oodialog lv_add"
::method addExtendedStyle unguarded external "LIBRARY oodialog lv_addClearExtendStyle"
::method addFullRow unguarded external "LIBRARY oodialog lv_addFullRow"
::method addRow unguarded external "LIBRARY oodialog lv_addRow"
::method addRowFromArray unguarded external "LIBRARY oodialog lv_addRowFromArray"
::method addStyle unguarded external "LIBRARY oodialog lv_addRemoveStyle"
::method alignLeft unguarded external "LIBRARY oodialog lv_arrange"
::method alignTop unguarded external "LIBRARY oodialog lv_arrange"
::method arrange unguarded external "LIBRARY oodialog lv_arrange"
::method bkColor unguarded external "LIBRARY oodialog lv_getColor"
::method "bkColor=" unguarded external "LIBRARY oodialog lv_setColor"
::method check unguarded external "LIBRARY oodialog lv_checkUncheck"
::method checkAll unguarded
   return self~check(-1)
::method columnInfo unguarded  -- Dialog units, not accurate.
   use strict arg index
   d = .Directory~new
   if self~getColumnInfo(index, d) then do
     colInfo.!TEXT = d~text
     colInfo.!WIDTH = d~width / self~factorX
     colInfo.!COLUMN = d~subItem
     colInfo.!ALIGN = d~alignment
     return colInfo.
   end

   colInfo.!WIDTH = 0
   return colInfo.

::method columnWidth unguarded  -- Dialog units, not accurate.
   use strict arg colNum
   return self~columnWidthPx(colNum) / self~factorX

::method columnWidthPx unguarded
   use strict arg column
   return self~sendWinIntMsg(self~LVM_GETCOLUMNWIDTH, column, 0)

::method delete unguarded external "LIBRARY oodialog lv_delete"
::method deleteAll unguarded external "LIBRARY oodialog lv_deleteAll"
::method deleteColumn unguarded external "LIBRARY oodialog lv_deleteColumn"
::method deselect unguarded external "LIBRARY oodialog lv_setSpecificState"
::method deselectAll unguarded external "LIBRARY oodialog lv_deselectAll"
::method dropHighlighted unguarded external "LIBRARY oodialog lv_getNextItemWithState"
::method edit unguarded
   use strict arg index
   if self~usingUnicode then return self~sendWinHandle2MsgH(self~LVM_EDITLABELW, index, 0)
   else return self~sendWinHandle2MsgH(self~LVM_EDITLABELA, index, 0)

::method endEdit unguarded
   use strict arg
   if self~usingUnicode then return self~sendWinHandle2MsgH(self~LVM_EDITLABELW, -1, 0)
   else return self~sendWinHandle2MsgH(self~LVM_EDITLABELA, -1, 0)

::method ensureVisible unguarded
   use strict arg index, partial = .false
   return self~sendWinIntMsg(self~LVM_ENSUREVISIBLE, index, partial) <> 0

::method find unguarded external "LIBRARY oodialog lv_find"
::method findNearestXY unguarded external "LIBRARY oodialog lv_findNearestXY"
::method findPartial unguarded external "LIBRARY oodialog lv_find"
::method firstVisible unguarded
   use strict arg
   return self~sendWinIntMsg(self~LVM_GETTOPINDEX , 0, 0)

::method fixFullRowColumns unguarded external "LIBRARY oodialog lv_fixFullRowColumns"
::method focus unguarded external "LIBRARY oodialog lv_setSpecificState"
::method focused unguarded external "LIBRARY oodialog lv_getNextItemWithState"
::method getCheck unguarded external "LIBRARY oodialog lv_getCheck"
::method getColumnCount unguarded external "LIBRARY oodialog lv_getColumnCount"
::method getColumnInfo unguarded  external "LIBRARY oodialog lv_getColumnInfo"
::method getColumnOrder unguarded external "LIBRARY oodialog lv_getColumnOrder"
::method getColumnText unguarded external "LIBRARY oodialog lv_getColumnText"
::method getExtendedStyle unguarded external "LIBRARY oodialog lv_getExtendedStyle"
::method getExtendedStyleRaw unguarded external "LIBRARY oodialog lv_getExtendedStyle"
::method getFullRow unguarded external "LIBRARY oodialog lv_getFullRow"
::method getHoverTime unguarded
   return self~sendWinIntMsg(self~LVM_GETHOVERTIME, 0, 0)

::method getImageList unguarded external "LIBRARY oodialog lv_getImageList"
::method getItem unguarded external "LIBRARY oodialog lv_getItem"
::method getItemData unguarded external "LIBRARY oodialog lv_getItemData"
::method getItemInfo unguarded external "LIBRARY oodialog lv_getItemInfo"
::method getItemRect unguarded external "LIBRARY oodialog lv_getItemRect"
::method getItemPos unguarded external "LIBRARY oodialog lv_getItemPos"
::method getSubitem unguarded external "LIBRARY oodialog lv_getSubitem"
::method getSubitemRect unguarded external "LIBRARY oodialog lv_getSubitemRect"
::method getToolTips unguarded external "LIBRARY oodialog generic_getToolTips"
::method getView unguarded external "LIBRARY oodialog lv_getView"
::method hasCheckBoxes unguarded external "LIBRARY oodialog lv_hasCheckBoxes"
::method hitTestInfo unguarded external "LIBRARY oodialog lv_hitTestInfo"
::method insert unguarded external "LIBRARY oodialog lv_insert"
::method insertColumn unguarded -- Dialog units, not accurate.
   use strict arg nr = 0, text, width, fmt = "L", adjustFullRows = .false
   width = trunc(width * self~factorX)
   return self~insertColumnPx(nr, text, width, fmt, adjustFullRows)

::method insertColumnPx unguarded external "LIBRARY oodialog lv_insertColumnPx"
::method insertFullRow unguarded external "LIBRARY oodialog lv_insertFullRow"
::method isChecked unguarded external "LIBRARY oodialog lv_isChecked"
::method itemInfo
   use strict arg index, subItem = 0
   d = .Directory~new
   if self~getItemInfo(index, d, subItem) then do
     itemInfo.!TEXT = d~text
     itemInfo.!IMAGE = d~image
     itemInfo.!STATE = d~state
     return itemInfo.
   end
   return -1

::method itemPos unguarded
   use strict arg item
   p = self~getItemPos(item)
   if p == 0 then return p
   else return p~x p~y

::method items unguarded
   return self~sendWinIntMsg(self~LVM_GETITEMCOUNT , 0, 0)

::method itemsPerPage unguarded
   return self~sendWinIntMsg(self~LVM_GETCOUNTPERPAGE, 0, 0)

::method itemState unguarded external "LIBRARY oodialog lv_itemState"
::method itemText unguarded external "LIBRARY oodialog lv_itemText"
::method last unguarded
   return self~items - 1

::method lastSelected unguarded
   if self~itemState(self~last)~wordpos("SELECTED") > 0 then  return self~last
   return self~previousSelected(self~last)

::method modify unguarded external "LIBRARY oodialog lv_modify"
::method modifyColumn unguarded  -- Dialog units, not accurate.
   use strict arg index, label = "", width = (-8), align = ""
   if arg(3,'E'), width~datatype('W') then width = width * self~factorX
   return self~modifyColumnPx(index, label, width, align)

::method modifyColumnPx unguarded external "LIBRARY oodialog lv_modifyColumnPx"
::method modifyFullRow unguarded external "LIBRARY oodialog lv_modifyFullRow"
::method modifyItem unguarded external "LIBRARY oodialog lv_modifyItem"
::method modifySubitem unguarded external "LIBRARY oodialog lv_modifySubitem"
::method next unguarded external "LIBRARY oodialog lv_getNextItem"
::method nextLeft unguarded external "LIBRARY oodialog lv_getNextItem"
::method nextRight unguarded external "LIBRARY oodialog lv_getNextItem"
::method nextSelected unguarded external "LIBRARY oodialog lv_getNextItem"
::method prepare4nItems unguarded
   use strict arg countItems
   return self~sendWinIntMsg(self~LVM_SETITEMCOUNT, countItems, 0)

::method prependFullRow unguarded external "LIBRARY oodialog lv_prependFullRow"
::method previous unguarded external "LIBRARY oodialog lv_getNextItem"
::method previousSelected unguarded external "LIBRARY oodialog lv_getNextItem"
::method redrawItems unguarded
   use strict arg first = 0, last = (self~items)
   return self~sendWinIntMsg(self~LVM_REDRAWITEMS, first, last) <> 0

::method removeExtendedStyle unguarded external "LIBRARY oodialog lv_addClearExtendStyle"
::method removeItemData unguarded external "LIBRARY oodialog lv_removeItemData"
::method removeStyle unguarded external "LIBRARY oodialog lv_addRemoveStyle"
::method replaceExtendedStyle unguarded external "LIBRARY oodialog lv_replaceExtendStyle"
::method replaceStyle unguarded external "LIBRARY oodialog lv_replaceStyle"
::method scroll unguarded
   use strict arg x = 0, y = 0
   return self~sendWinIntMsg(self~LVM_SCROLL, x, y) <> 0

::method select unguarded external "LIBRARY oodialog lv_setSpecificState"
::method selected unguarded external "LIBRARY oodialog lv_getNextItemWithState"
::method selectedItems unguarded
   return self~sendWinIntMsg(self~LVM_GETSELECTEDCOUNT , 0, 0)

::method setColumnOrder external "LIBRARY oodialog lv_setColumnOrder"
::method setColumnWidth unguarded  -- Dialog units, not accurate.
   use strict arg index, width = "AUTO"
   if width~datatype("W") then width = width * self~factorX
   return self~setColumnWidthPx(index, width)

::method setColumnWidthPx unguarded external "LIBRARY oodialog lv_setColumnWidthPx"
::method setFullRowText unguarded external "LIBRARY oodialog lv_setFullRowText"
::method setHoverTime unguarded
   use strict arg time = (-1)
   if time < -1 then time = -1
   return self~sendWinIntMsg(self~LVM_SETHOVERTIME, 0, time)

::method setImageList unguarded external "LIBRARY oodialog lv_setImageList"
::method setItemData unguarded external "LIBRARY oodialog lv_setItemData"
::method setItemPos unguarded external "LIBRARY oodialog lv_setItemPos"
::method setItemState unguarded external "LIBRARY oodialog lv_setItemState"
::method setItemText unguarded external "LIBRARY oodialog lv_setItemText"
::method setToolTips unguarded external "LIBRARY oodialog generic_setToolTips"
::method setView unguarded external "LIBRARY oodialog lv_setView"
::method smallSpacing unguarded
   return self~sendWinIntMsg(self~LVM_GETITEMSPACING, 1, 0)

::method snapToGrid unguarded external "LIBRARY oodialog lv_arrange"
::method sortItems unguarded external "LIBRARY oodialog lv_sortItems"
::method spacing unguarded
   return self~sendWinIntMsg(self~LVM_GETITEMSPACING, 0, 0)

::method stringWidth unguarded  -- Dialog units, not accurate.
   use strict arg text
   return self~stringWidthPx(text) / self~factorX

::method stringWidthPx unguarded external "LIBRARY oodialog lv_stringWidthPx"
::method textBkColor unguarded external "LIBRARY oodialog lv_getColor"
::method "textBkColor=" unguarded external "LIBRARY oodialog lv_setColor"
::method textColor unguarded external "LIBRARY oodialog lv_getColor"
::method "textColor=" unguarded external "LIBRARY oodialog lv_setColor"
::method uncheck unguarded external "LIBRARY oodialog lv_checkUncheck"
::method uncheckAll unguarded
   return self~uncheck(-1)

::method updateItem unguarded
   use strict arg index
   return self~sendWinIntMsg(self~LVM_UPDATE, index, 0) == 0


::method zTest unguarded external "LIBRARY oodialog lv_zTest"             -- internal use only

-- DEPRECATED to end of Class
::method removeImages unguarded                                           -- DEPRECATED
   return self~removeImageList(.nil, 0)


::method removeSmallImages unguarded                                      -- DEPRECATED
   return self~removeImageList(.nil, 1)

::method restoreEditClass                                                 -- DEPRECATED

::method setImages unguarded external "LIBRARY oodialog lv_setImageList"  -- DEPRECATED

::method setSmallImages unguarded                                         -- DEPRECATED
   newArgs = arg(1, 'A')
   newArgs[4] = 1
   forward message 'setImageList' arguments (newArgs)

::method subclassEdit                                                     -- DEPRECATED


::class 'LvCustomDrawSimple' public
::method init class external       "LIBRARY oodialog lvcds_init_cls"
::method init external             "LIBRARY oodialog lvcds_init"

::attribute clrText set external   "LIBRARY oodialog lvcds_setClrText"
::attribute clrTextBk set external "LIBRARY oodialog lvcds_setClrTextBk"
::attribute drawStage get external "LIBRARY oodialog lvcds_getDrawStage"
::attribute font set external      "LIBRARY oodialog lvcds_setFont"
::attribute id get external        "LIBRARY oodialog lvcds_getID"
::attribute item get external      "LIBRARY oodialog lvcds_getItem"
::attribute itemData get external  "LIBRARY oodialog lvcds_getItemData"
::attribute reply set external     "LIBRARY oodialog lvcds_setReply"
::attribute subItem get external   "LIBRARY oodialog lvcds_getSubItem"



::class 'LvItem' public

::method init class external "LIBRARY oodialog lvi_init_cls"

::method init external "LIBRARY oodialog lvi_init"
::method unInit external "LIBRARY oodialog lvi_unInit"

::attribute columnFormats get external "LIBRARY oodialog lvi_columnFormats"
::attribute columnFormats set external "LIBRARY oodialog lvi_setColumnFormats"
::attribute columns get external "LIBRARY oodialog lvi_columns"
::attribute columns set external "LIBRARY oodialog lvi_setColumns"
::attribute groupID get external "LIBRARY oodialog lvi_groupID"
::attribute groupID set external "LIBRARY oodialog lvi_setGroupID"
::attribute imageIndex get external "LIBRARY oodialog lvi_imageIndex"
::attribute imageIndex set external "LIBRARY oodialog lvi_setImageIndex"
::attribute indent get external "LIBRARY oodialog lvi_indent"
::attribute indent set external "LIBRARY oodialog lvi_setIndent"
::attribute index get external "LIBRARY oodialog lvi_index"
::attribute index set external "LIBRARY oodialog lvi_setIndex"
::attribute itemData get external "LIBRARY oodialog lvi_itemData"
::attribute itemData set external "LIBRARY oodialog lvi_setItemData"
::attribute itemState get external "LIBRARY oodialog lvi_itemState"
::attribute itemState set external "LIBRARY oodialog lvi_setItemState"
::attribute itemStateMask get external "LIBRARY oodialog lvi_itemStateMask"
::attribute itemStateMask set external "LIBRARY oodialog lvi_setItemStateMask"
::attribute mask get external "LIBRARY oodialog lvi_mask"
::attribute mask set external "LIBRARY oodialog lvi_setMask"
::attribute overlayImageIndex get external "LIBRARY oodialog lvi_overLayImageIndex"
::attribute overlayImageIndex set external "LIBRARY oodialog lvi_setOverlayImageIndex"
::attribute stateImageIndex get external "LIBRARY oodialog lvi_stateImageIndex"
::attribute stateImageIndex set external "LIBRARY oodialog lvi_setStateImageIndex"
::attribute text get external "LIBRARY oodialog lvi_text"
::attribute text set external "LIBRARY oodialog lvi_setText"


::class 'LvFullRow' public
::method init class external "LIBRARY oodialog lvfr_init_cls"
::method fromArray class external "LIBRARY oodialog lvfr_fromArray_cls"

::method init external "LIBRARY oodialog lvfr_init"
::method unInit external "LIBRARY oodialog lvfr_unInit"

::attribute userData get external "LIBRARY oodialog lvfr_userData"
::attribute userData set external "LIBRARY oodialog lvfr_setUserData"

::method addSubItem external "LIBRARY oodialog lvfr_addSubitem"
::method insertSubitem external "LIBRARY oodialog lvfr_insertSubitem"
::method item external "LIBRARY oodialog lvfr_item"
::method removeSubItem external "LIBRARY oodialog lvfr_removeSubitem"
::method subItem external "LIBRARY oodialog lvfr_subitem"
::method subItems external "LIBRARY oodialog lvfr_subitems"


::class 'LvSubItem' public
::method init class external "LIBRARY oodialog lvsi_init_cls"

::method init external "LIBRARY oodialog lvsi_init"
::method unInit external "LIBRARY oodialog lvsi_unInit"

::attribute imageIndex get external "LIBRARY oodialog lvsi_imageIndex"
::attribute imageIndex set external "LIBRARY oodialog lvsi_setImageIndex"
::attribute item get external "LIBRARY oodialog lvsi_item"
::attribute item set external "LIBRARY oodialog lvsi_setItem"
::attribute mask get external "LIBRARY oodialog lvsi_mask"
::attribute mask set external "LIBRARY oodialog lvsi_setMask"
::attribute subItem get external "LIBRARY oodialog lvsi_subItem"
::attribute subItem set external "LIBRARY oodialog lvsi_setSubItem"
::attribute text get external "LIBRARY oodialog lvsi_text"
::attribute text set external "LIBRARY oodialog lvsi_setText"



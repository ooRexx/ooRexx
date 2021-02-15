/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
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
 * ToolTip control example.
 *
 * This example demonstrates use info tips with a tree-view and do custom
 * positiong of the tool tip.
 *
 * The appearance and functionality of the dialog in this example is exactly the
 * same as that produced by the manageControlTool.rex example.  It simple shows
 * an easier way to achieve the same effect as that example.
 *
 * Here we use a simple application of the manageAtypicalTool() method.  The
 * manageControlTool.rex example is provided to better help the ooDialog
 * programmer understand the manageAtypicalTool() method.  The programmer should
 * really examine both example programs.
 *
 * To customize the positioning of the info tip, we need to write an event
 * handler for the ToolTips SHOW event.  However, the ToolTip for the tree-views
 * info tips is owned by the tree-view.  Event notifications are sent to the
 * tree-view and our dialog has no knowledge of them.
 *
 * To fix that, we use the getToolTips() method to get the tree-views ToolTip.
 * We then use the manageAtypicalTool() method to monitor the event
 * notifications sent to the tree-view and send the SHOW notification to our
 * dialog instead of to the tree-view.  All other events, including the mouse
 * relay event, are left to go to the tree-view.  This gives us the same
 * functionality as the custom ToolTip that is set up in the manageControlToo
 * example, with less work.
 */

  sd = locate()
  .application~setDefaults('O', sd'customPositionToolTip.h', .false)

  dlg = .TreeViewDialog~new(sd'customPositionToolTip.rc', IDD_TREEVIEW)
  dlg~execute("SHOWTOP", IDI_DLG_OOREXX)


::requires "ooDialog.cls"

::class 'TreeViewDialog' subclass RcDialog

::method initDialog
  expose currentItem

  tv = self~newTreeView(IDC_TREE)
  tv~setItemHeight(20)

  self~connectTreeViewEvent(IDC_TREE, 'GETINFOTIP')
  self~populateTree(tv)

  -- Get the ToolTip the tree-view has created and is using.
  toolTip = tv~getToolTips

  -- Set up the monitoring of the ToolTip notifications sent to the tree-view.
  -- We only want to intercept the SHOW notification.  Also, we do not want the
  -- automatic monitoring and relaying of the mouse messages.  In this way we
  -- achieve what we want with minimal interference with the tree-view's
  -- ToolTip.
  --
  -- We have to use the NORELAY keyword to turn off the automatic
  -- relaying of the mouse messages.  NORELAY does not connect an event handler,
  -- it simply turns relaying off.

  toolTip~manageAtypicalTool(tv, .array~of('NORELAY', 'SHOW'))


/** onGetInfoTip()
 *
 * This is the event handler for the tree-view's GETINFOTIP notification.  It is
 * invoked when the tree-view wants the text to display for a ToolTip.
 *
 * Note that we are sent the handle of the tree-view item that the ToolTip is
 * for and the user data (item data) for that item.  We set the item data for
 * each item to the text to display for the ToolTip for that item.  Which makes
 * this very easy.  We just return the userData sent to us.
 *
 * Note this also.  Our onShow() event handler is invoked by our monitoring of
 * the ToolTip notifications.  That monitor has no knowledge of which item the
 * ToolTip is going to be displayed for.  So, we use the exposed currentItem
 * variable to pass on our knowledge here of which item the ToolTip is going to
 * display for.
 */
::method onGetInfoTip unguarded
    expose currentItem
    use arg id, hItem, text, maxLen, userData

    currentItem = hItem
    return userData


/** onShow()
 *
 * This is our event handler for the tree-view's ToolTip SHOW event
 * notification.  The method is invoked when our process monitoring the
 * notifications sent to the tree-view, intercepts the SHOW notification.
 *
 * Normally the ToolTip window is placed below the item.  This often over-laps
 * and obscures the other items.  What we want is to place the ToolTip window to
 * the right and centered with the item.  This way nothing is obscured.
 *
 * The custom positioning of the ToolTip window is straight-forward.  We get the
 * bounding rectangle of the tree-view item's label, convert the point to client
 * coordinates, and then reposition the ToolTip window to where we want it.
 *
 * Note that we must return .true then so that the ToolTip is informed not to
 * position the window in its default position.
 */
::method onShow unguarded
    expose currentItem
    use arg toolTip, treeView

    r = .Rect~new
    if treeView~getItemRect(currentItem, r) then do
        treeView~client2screen(r)

        toolTip~setWindowPos(0, r~right + 5, r~top, 0, 0, "NOACTIVATE NOSIZE NOZORDER")
        return .true
    end

    return .false


/** populateTree()
 *
 * This is a private method used to populate the tree-view control with our
 * items.
 *
 * Note that each item has an item data object assigned to it.  In this case the
 * object is a string.  The string is the text to display for the tool tip for
 * that item.
 */
::method populateTree private
    use strict arg tv

    hItem = tv~add("Peter", , ,"BOLD EXPANDED", , 'Data unique to Peter.')

    hItem = tv~add(,"Mike", , ,"EXPANDED", , 'Mike talks a lot.')

    data = 'George was famous once.'.endOfLine'He is unknown to my'.endOfLine'niece.'
    hItem = tv~add(, ,"George", , , , , data)

    hItem = tv~add(, ,"Monique", , , , , 'Monique has never been to France.')

    hItem = tv~add(, , ,"John", , , , , 'John owns 4 dogs.')

    hItem = tv~add(, ,"Chris", , , , , 'Chris will never be president.')

    hItem = tv~add( , "Maud", , , , , 'Maud likes to bake.')

    hItem = tv~add( , "Ringo", , , , , 'Ringo has a LOT of money.')

    hItem = tv~add("Paul", , ,"BOLD EXPANDED", , "Two roads lead to Paul's house.")

    hItem = tv~add( , "Dave", , , , , 'No one lives with Dave.')

    hItem = tv~add( , "Sam", , , , , 'Sam loves pizza.')

    hItem = tv~add( , "Jeff", , , , , 'Jeff rides his bike to work.')

    hItem = tv~add("Mary", , ,"BOLD EXPANDED", , 'Mary owes $49.50 to her dentist.')

    hItem = tv~add( , "Helen", , , , , 'A closer look at Helen is warranted.')

    hItem = tv~add( , "Michelle", , , , , 'The Toyota Camray belongs to Michelle.')

    hItem = tv~add( , "Diana", , , , , 'Go go Chargers.')

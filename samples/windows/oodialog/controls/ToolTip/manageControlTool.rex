/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2014 Rexx Language Association. All rights reserved.    */
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
 * ToolTip control example.
 *
 * This example demonstrates several advanced techniques in using ToolTips.
 *
 * Typically a single tool is created for a dialog control.  This works fine
 * for creating tools for, say, buttons.  Even if you have 20 buttons, it is
 * pretty reasonable to create 20 tools in a ToolTip.
 *
 * But, what if you want to display a tip for each item in a tree-view?  What if
 * the tree-view has hundreds, or even thousands, of items.  Creating several
 * thousand tools is no longer reasonable.  One technique is to create one tool
 * and dynamically make it appear to be multiple tools.
 *
 * That technique is demonstrated in this example.  We use a tree-view control
 * to show the technique.  The basic principle is to track where the mouse is,
 * and dynamically update the tool each time it moves from one item to another.
 *
 * Normally you would program this by monitoring the mouse moves and updating
 * the tool when the mouse moves require it.  To do this, you can not use the
 * SUBCLASS flag for the tool, because you need to dynamically update tool
 * _before_ the tool sees the mouse move.  You would monitor the mouse moves,
 * decide whether to update or not the tool, and then forward every mouse move
 * on to the tool.  For the puposes of discussion, we will refer to the
 * forwarding of the mouse moves as 'relaying' the moves.  And, actually for the
 * ToolTip to work correctly it needs to see all the mouse related messages.
 * So, the relaying is really relaying all mouse messages, not just the mouse
 * move message.
 *
 * This presents a problem in ooDialog because mouse messages are sent to the
 * window under the mouse, in this case the tree-view.  In addition, if we set
 * up a tool for the tree-view, but can not use the SUBCLASS flags, the ToolTip
 * event notifications are sent to the tree-view, not to our dialog.
 *
 * In this case, we need a way to monitor the mouse messages sent to the
 * tree-view and a way to intercept the ToolTip event notifications that would
 * be sent to the tree-view.  For ToolTips, ooDialog provides a method, the
 * manageAtypicalTool() method, that provides this functionality.
 *
 * This method monitors all mouse messages and relays the messages to the tool.
 * Before it relays the mouse messages to the took, it will invoke a method in
 * the Rexx dialog for the relay event.  In addition, it intercepts all the
 * ToolTip event notifications sent to the dialog control and invokes a method
 * in the Rexx dialog for each event.
 *
 * Of course, the application may not need to have an event handler for all the
 * events, or even for the relay event.  So, manageAtypicalTool() allows the
 * programmer to just connect the events it needs.
 *
 * Although this example just focuses on a specific use of manageAtypicalTool(),
 * the method can be used in any situation where custom ToolTips are needed for
 * dialog controls.  The same principles that necessitate its use here, will be
 * found in many situations.
 *
 * Note that in this specific case, there is an easier way to produce the same
 * results as this example does.  The customPositionToolTip.rex example shows
 * how to use the easier method.  However, this example is very useful in
 * showing how the manageAtypicalToo() works.  The principles here can be used
 * in other situations.
 */

  sd = locate()
  .application~setDefaults('O', sd'manageControlTool.h', .false)

  dlg = .TreeViewDialog~new(sd'manageControlTool.rc', IDD_TREEVIEW)
  dlg~execute("SHOWTOP", IDI_DLG_OOREXX)


::requires "ooDialog.cls"

::class 'TreeViewDialog' subclass RcDialog

::method initDialog
  expose currentItem currentLocation currentText

  -- These variables are used to help track which tool needs to be shown.
  currentItem = 0
  currentText = ''
  currentLocation = 'NOWHERE'

  tv = self~newTreeView(IDC_TREE)

  -- Set up our ToolTip
  tt = self~createToolTip(IDC_TT)

  rect = tv~clientRect

  -- We need to specify very precisely the attributes of the tool we are going
  -- to add.  We can not do that using the addTool() or addToolRect()
  -- convenience methods.  So we need a ToolInfo object and to use the
  -- addToolEx() method.

  toolInfo = .ToolInfo~new(tv, 10, '', 'TRANSPARENT', rect)
  tt~addToolEx(toolInfo)

  tt~setMaxTipWidth(250)

  -- Set up our tree-view and populate it.
  tv~setItemHeight(20)
  self~populateTree(tv)

  -- Now start the monitoring process.
  tt~manageAtypicalTool(tv, .array~of('RELAY', 'NEEDTEXT', 'SHOW'))


/** onRelay()
 *
 * This is the event handler for the mouse message relaying event.  It is
 * invoked by our monitoring process for every mouse message.  The method is
 * invoked _before_ the monitor relays the mouse message to the ToolTip.
 *
 * This method is crucial to how this example works.  For every mouse message,
 * we calculate if the mouse is still over the label of the same item it was
 * over previously.  If it is not, we hide the ToolTip window by invoking the
 * pop().
 *
 * After we return form this method, the mouse message is forwarded on to the
 * ToolTip to let it do its ToolTip thing.  When the ToolTip decides the mouse
 * has hovered long enough in one spot, it displays its window.  Since we have
 * set the current text to the string for the tree-view item when the mouse
 * first moved over the item, the correct "tool" text is displayed.
 *
 * The text to display is actually passed on to the ToolTip from our onNeedTex()
 * method.
 */
::method onRelay unguarded
    expose currentItem currentText currentLocation
    use arg toolTip, pos, mouseMsg, treeView

    -- Determine which, if any, item the mouse is over.
    d = treeView~hitTestInfo(pos)

    -- Save the old item and location and sent the current item and location to
    -- where the mouse is at this moment.
    oldItem = currentItem
    oldLocation = currentLocation
    currentItem = d~hItem
    currentLocation = d~location

    -- If mouse is over a new item or location, hide the ToolTip.  Set our
    -- current text to the text for the item we are now over.  If we are not
    -- over the label of an item, set the current text to the empty string.
    if oldItem \== currentItem | oldLocation \== currentLocation then do
        if currentItem <> 0, currentLocation~wordPos('ONLABEL') <> 0 then do
            currentText = treeView~getItemData(currentItem)
        end
        else do
            currentText = ''
        end
        toolTip~pop
    end

    return 0


/** onNeedText()
 *
 * This is the event handler for the ToolTip's NEEDTEXT event notification.
 *
 * The notification was actually sent to the tree-view, but, our monitor process
 * has intercepted the notification and sent it to us instead of the tree-view.
 *
 * The real work was already done in the onRelay() method.  All we need to do is
 * set the TEXT index in the info .Directory object to what the onRelay() method
 * set the current text to.
 */
::method onNeedText unguarded
    expose currentItem currentText
    use arg toolTip, treeView, info, userData, flags

    info~text = currentText

    -- Return false so that the ToolTip does not store the text.  We want it to
    -- always ask us for the text because the mouse could be over any item.
    return .false



/** onShow()
 *
 * This is our event handler for the ToolTip's SHOW event notification.
 *
 * The notification was actually sent to the tree-view, but our monitor process
 * has intercepted the notificatio and sent it to us instead of the tree-view.
 *
 * Normally the ToolTip places its window below the tree-view item.  This often
 * over-laps and obscures the other items.  What we want is to place the ToolTip
 * window to the right and centered with the item.  This way nothing is
 * obscured.
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

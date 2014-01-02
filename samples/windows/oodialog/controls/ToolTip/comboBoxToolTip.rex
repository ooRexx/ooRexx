/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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
 *  This example shows how to add tool tips to a combo box.
 *
 *  When the items in a combo box are longer than the width of the combo box,
 *  they can be difficult for the user to read.  Tool tips are a good solution
 *  to this, but combo boxes do not have built in support for tool tips like
 *  list-views or tree-views.
 *
 *  Adding the support to a combo box is doable, but it takes a little work.
 *  This example shows how it is done.  The example also shows how to custom
 *  position the tool tips.  To see the difference in where the tool tip
 *  positions the tips by default and where this program positions them, the
 *  dialog has 2 radio buttons that allow switching between the default
 *  positioning and the custom positioning.
 *
 *  One approach to adding tool tips to a combo box would be to just have the
 *  tool tips display for the items in the drop down list.  This example goes
 *  beyond that to also show tool tips for the selection field and the drop down
 *  button.
 *
 *  When the drop down is closed, adding tool tips to the selection field and
 *  the button is relatively easy.  But, when the drop down is open, it is hard
 *  because the list box has the mouse captured.  This prevents tool tips from
 *  activating when the mouse goes over the selection field or the button.  What
 *  we do here is create tracking tool tips for the selection field and the
 *  button to use when the drop down is open.  Tracking tool tips need to be
 *  shown and hidden manually, which is what is done in this program.
 */

  sd = locate()
  .application~setDefaults('O', sd'comboBoxToolTip.h', .false)

  dlg = .CitiesOfTheWorldDlg~new(sd'comboBoxToolTip.rc', IDD_MAJOR_CITIES)
  dlg~execute("SHOWTOP", IDI_DLG_OOREXX)

  return 0

::requires "ooDialog.cls"

::class 'CitiesOfTheWorldDlg' subclass RcDialog

/** init()
 *
 * Normally for RcDialog dialogs there is no need for an init() method.  Here,
 * we use the init() method to set up our state varialbes and to connect a
 * combo box event.
 *
 * There is no real need to do it here, it could just as well been done in the
 * initDialog() method.
 */
::method init
  expose currentItem currentText currentOnClient ttEditActive ttButtonActive doCustomPositioning

  forward class (super) continue

  -- These variables are used to help track which tool needs to be shown.
  currentItem     = 0
  currentText     = ''
  currentOnClient = .false
  ttEditActive    = .false
  ttButtonActive  = .false

  doCustomPositioning = .true

  self~connectComboBoxEvent(IDC_COMBOBOX, 'CLOSEUP', onCloseUP)

  self~connectButtonEvent(IDC_RB_DEFAULT, 'CLICKED', doDefault)
  self~connectButtonEvent(IDC_RB_CUSTOM, 'CLICKED', doCustom)


/** initDialog()
 *
 *  This is a busy initDialog().  We need to set up and configure the combo box
 *  and create and configure the tool tips.
 */
::method initDialog
  expose comboBox cbEdit cbListBox lbRect editRect buttonRect ttEditTrack ttButtonTrack itemHeight

  comboBox = self~newComboBox(IDC_COMBOBOX)

  -- The tool tips are slightly higher than the default height of the combo box
  -- items.  So, when we position the tool tip over an item, it does not quite
  -- fit.  Making the item height slightly bigger makes things look a lot
  -- better.
  --
  -- If the item height is to be changed, be sure to do it before the items
  -- are inserted.  Otherwise when we get the client rect for the list box, its
  -- vertical height will be incorrect.
  itemHeight = comboBox~getItemHeight
  itemHeight += 2
  comboBox~setItemHeight(itemHeight)

  entries = self~getComboItems
  do e over entries
    comboBox~add(e)
  end

  comboBox~setCue("Select a city")
  comboBox~setMinVisible(10)

  d = comboBox~getComboBoxInfo
  cbEdit    = d~textObj
  cbListBox = d~listBoxObj

  self~calcRects(d)

  self~newRadioButton(IDC_RB_CUSTOM)~check

  -- Create a tracking tool tip for the selection field of the combo box.  For a
  -- drop-down list combo box this is actually the combo box itself.  We still
  -- use cbEdit, even though we know cbEdit and comboBox are the same.
  --
  -- 1.) This tool tip is used for the selection field when the dropdown is open
  ttEditTrack = self~createToolTip(IDC_TT_EDIT_TRACK)

  toolInfo = .ToolInfo~new(self, cbEdit, '', 'ABSOLUTE TRACK IDISHWND', .Rect~new)
  ttEditTrack~addToolEx(toolInfo)

  self~connectToolTipEvent(IDC_TT_EDIT_TRACK, 'NEEDTEXT', onEditTrackNeedText)
  self~connectToolTipEvent(IDC_TT_EDIT_TRACK, 'SHOW', onEditTrackShow)

  -- Create a tracking tool tip for the button portion of the combo box.
  --
  -- 2.) This tool tip is used for the button when the dropdown is open.
  ttButtonTrack = self~createToolTip(IDC_TT_BUTTON_TRACK)

  text = 'Press the button to close the drop down'
  toolInfo = .ToolInfo~new(self, comboBox, text, 'ABSOLUTE TRACK IDISHWND', .Rect~new)
  ttButtonTrack~addToolEx(toolInfo)

  self~connectToolTipEvent(IDC_TT_BUTTON_TRACK, 'SHOW', onButtonTrackShow)

  -- Create a tool tip for the combo box itself.
  --
  -- 3.) This tool tip is used for the selection and button components when the
  -- dropdown is closed.
  ttComboBox = self~createToolTip(IDC_TT_CB_MAIN)

  toolInfo = .ToolInfo~new(comboBox, IDC_TI_EDITRECT, '', 'TRANSPARENT', editRect, IDC_TI_EDITRECT)
  ret = ttComboBox~addToolEx(toolInfo)

  text = 'Press the button to open the drop down'
  toolInfo = .ToolInfo~new(comboBox, IDC_TI_BUTTONRECT, text, 'TRANSPARENT', buttonRect, IDC_TI_BUTTONRECT)
  ttComboBox~addToolEx(toolInfo)

  ttComboBox~manageAtypicalTool(comboBox, .array~of('NEEDTEXT', 'SHOW'), .array~of('ONCBNEEDTEXT', 'ONCBSHOW'))

  -- Create tool tip for the items of the combo box.
  --
  -- 4.) This tool tip is for the individual items when the dropdown is open.
  ttList = self~createToolTip(IDC_TT_CB_LISTBOX)

  toolInfo = .ToolInfo~new(cbListBox, IDC_TI_LISTBOX, '', 'TRANSPARENT', lbRect)
  ttList~addToolEx(toolInfo)

  -- Now start the monitoring process for the list box tool.
  ttList~manageAtypicalTool(cbListBox, .array~of('RELAY', 'NEEDTEXT', 'SHOW'))


/** onRelay()
 *
 * The onRelay() event handler is invoked by the monitor that the
 * manageAtypicalTool() method sets up.  It is invoked for every mouse message
 * that is sent to the list box control.
 *
 * The monitor first informs us of the mouse message.  This gives us a chance to
 * examine the message *before* the tool tip sees it.
 *
 * This event handler here is the key to how this whole program works.
 *
 * All the rest of the code in the program is simply busy work, keeping track of
 * what state we are in.  In this method we determine where the mouse is and
 * then determine how we have to configure all the tool tips we are using.
 *
 * We need to know, or to take into account, some facts about the combo box.  1
 * fact is that when the list box is dropped down, it captures the mouse.  All
 * mouse messages are then sent to the list box until it releases the capture
 * when it closes up.  This means that the pos argument can be anywhere on the
 * screen.
 *
 * Fact number 2 is that the combo box will always tell us the index of the item
 * that is *closest* to the mouse position.  So, even if the mouse is on the
 * other side of the screen, the combo box will still return a valid item index.
 * It will be what ever item is closest to that mouse position, even if it is
 * 500 pixels away.
 *
 * We need to track what item the mouse is over, and whether the mouse is over
 * the client area of the combo box.  Every time the location changes from over
 * the clien to not over the client, and every time the item the mouse is over
 * changes, the tool tips need to be updated.
 *
 * To handle the selection field and the button, we treat them a pseudo list box
 * item.  When the mouse is over either of them, it will not be over the list
 * box client area.  We *pretend* that it is by changing the value in the
 * directory object returned by the hit test info method.  We do a similar thing
 * for the item index.  We *pretend* that the selection field has an item index
 * of 0 and that the button has an item index of -1.
 *
 * If we are, newly, over the selection field or button, we need to manually
 * activate the tracking tool tip and send it the mouse position.  In screen
 * coordinates, not client coordinates.  Other code in this program is
 * responsible for hiding the tracking tool tips at the proper time.
 *
 * The monitor will pass all mouse messages on to the tool tip after we return
 * from this method.  This allows the tool tip to 'do its thing.'
 */
::method onRelay unguarded
    expose currentItem currentText currentOnClient editOffsetRect buttonOffsetRect -
           ttEditTrack ttEditActive cbEdit ttButtonTrack ttButtonActive comboBox
    use arg toolTip, pos, mouseMsg, listBox

    -- Determine which, if any, item the mouse is over.
    d = .directory~new
    ret = listBox~hitTestInfo(pos, d)

    -- Treat the edit and button rects as though they are in the client area of
    -- the list box and assign them a, fake, item index.
    if pos~inRect(editOffsetRect) then do
      d~inClientArea = .true
      d~itemIndex = 0
    end
    else if pos~inRect(buttonOffsetRect) then do
      d~inClientArea = .true
      d~itemIndex = -1
    end

    -- Save the old item and location and sent the current item and location to
    -- where the mouse is at this moment.
    oldItem         = currentItem
    oldOnClient     = currentOnClient
    currentItem     = d~itemIndex
    currentOnClient = d~inClientArea

    -- If mouse is over a new item or location, hide the ToolTip.  Set our
    -- current text to the text for the item we are now over.  If we are not
    -- on the client area, set the current text to the empty string.
    if oldItem \== currentItem | oldOnClient \== currentOnClient then do
        if currentOnClient then do
            if d~itemIndex == -1 then do
                p = pos~copy
                listBox~client2Screen(p)
                ttButtonTrack~trackActivate(self, comboBox, .true)
                ttButtonActive        = .true
                turnTTEditActiveOff   = .true
                turnTTButtonActiveOff = .false
                ttEditTrack~trackPosition(p)
            end
            else if d~itemIndex == 0 then do
                p = pos~copy
                listBox~client2Screen(p)
                ttEditTrack~trackActivate(self, cbEdit, .true)
                ttEditActive = .true
                turnTTEditActiveOff   = .false
                turnTTButtonActiveOff = .true
                ttEditTrack~trackPosition(p)
            end
            else do
                turnTTButtonActiveOff = .true
                turnTTEditActiveOff   = .true
                currentText = listBox~getText(d~itemIndex)
            end
        end
        else do
            turnTTButtonActiveOff = .true
            turnTTEditActiveOff    = .true
            currentText = ''
        end

        -- Each time we have a change, we close the current tool tip.  The tool
        -- tips are now configured for the *new* item the mouse is over.  Which
        -- may be not over an item at all, of course.  When we return from this
        -- handler, the mouse message will be relayed on to the tool tip, and it
        -- will do its tool tip thing.
        toolTip~pop

        if turnTTEditActiveOff, ttEditActive then do
            ttEditTrack~trackActivate(self, cbEdit, .false)
            ttEditActive = .false
        end

        if turnTTButtonActiveOff, ttButtonActive then do
            ttButtonTrack~trackActivate(self, comboBox, .false)
            ttButtonActive = .false
        end
    end

    return 0


/** onNeedText()
 *
 * The event handler for the NEEDTEXT event.  This method is invoked by the
 * monitor for the list box tool when that tool is requesting the text it should
 * display.
 *
 * We set the text to the text of the current item the mouse is over.
 */
::method onNeedText unguarded
    expose currentItem currentText
    use arg toolTip, listBox, info

    info~text = currentText

    -- Return false so that the ToolTip does not store the text.  We want it to
    -- always ask us for the text because the mouse could be over any item.
    return .false


/** onShow()
 *
 * The event handler for the SHOW event.  This method is invoked by the monitor
 * for the list box tool when that tool is notifiying the list box that it is
 * about to show the tip.  This gives us the chance to do a custom positioning
 * of the tip.
 *
 * If we are not doing custom positioning we just return false.  False tells the
 * tool tip that it should position the tip itself.
 *
 * If we are doing the positioning, we set the postion we want and return true.
 * True tells the tool tip that we already positioned the tip and it should not
 * change its postion.
 *
 * To get the position we determine which item the mouse is over, in list box
 * client coordinates and convert the client coordinates to screen coordinates.
 * The position we want is the top left corner of the list box item.  But, the
 * tool tip tips are slightly taller than a list box item, so we shift the
 * position up 3 pixels, which gives us a position that does not obscure either
 * the item above or the item below.
 */
::method onShow unguarded
    expose currentItem comboBox lbRect itemHeight doCustomPositioning
    use arg toolTip, listBox, toolID

    if \ doCustomPositioning then return .false

    if currentItem > 0 then do
        topIndex = comboBox~getFirstVisible
        r = lbRect~copy
        r~top = (currentItem - topIndex) * itemHeight
        r~top -= 3
        listBox~client2screen(r)

        toolTip~setWindowPos(0, r~left, r~top, 0, 0, "NOACTIVATE NOSIZE")
        return .true
    end

    return .false


/** onCBNeetText()
 *
 * The event handler for the NEEDTEXT event.  This method is invoked when the
 * combo box tool tip is requesting the text for the tip.
 *
 * The combo box tool tip is a normal tool tip, there is no manageAtypicalTool
 * monitor for this tool tip.  Recall, that we set this tool tip up with 2
 * tools.  One for the rectangle of the selection field and one for the
 * rectangle of the button.  We use the tool ID to detemine which tool we are
 * invoked for.  If it is the selection field, we return the selected item text.
 * If it is the button, we return the button tip.
 *
 * We can only be invoked when the drop down is closed.
 */
::method onCBNeedText unguarded
    use arg toolTip, comboBox, info

    if info~toolID == .constDir[IDC_TI_EDITRECT] then do
        info~text = comboBox~selected
    end
    else if info~toolID == .constDir[IDC_TI_BUTTONRECT] then do
        info~text = 'Press the button to open the drop down'
    end
    else do
        -- Can not happen
        info~text = ''
        return .true
    end

    return .false


/** onCBShow()
 *
 * The event handler for the SHOW event.  This method is invoked when the
 * combo box tool tip is about to show the tip.  This gives us a chance to do
 * custom positiong.  If we are not doing custom positioning, we simple return
 * false, telling the tool tip to position the tip itself.
 *
 * The combo box tool tip is a normal tool tip, there is no manageAtypicalTool
 * monitor for this tool tip.  Recall, that we set this tool tip up with 2
 * tools.  One for the rectangle of the selection field and one for the
 * rectangle of the button.  We use the tool ID to detemine which tool we are
 * invoked for.  If it is the selection field, we use the postion of the
 * selection field rectangle, convert it to screen coordinates and position the
 * tip at the top left corner.
 *
 * If it is the button, we do the same thing, but position the tip immediately
 * to right of the button.
 *
 * We can only be invoked when the drop down is closed.
 */
::method onCBShow unguarded
    expose editRect buttonRect doCustomPositioning
    use arg toolTip, comboBox, toolID

    if \ doCustomPositioning then return .false

    if toolID == .constDir[IDC_TI_EDITRECT] then do
        r = editRect~copy

        comboBox~client2screen(r)
        toolTip~setWindowPos(0, r~left, r~top, 0, 0, "NOACTIVATE NOSIZE NOZORDER")
        return .true
    end
    else if toolID == .constDir[IDC_TI_BUTTONRECT] then do
        r = buttonRect~copy

        comboBox~client2screen(r)
        toolTip~setWindowPos(0, r~right, r~top, 0, 0, "NOACTIVATE NOSIZE NOZORDER")
        return .true
    end

    return .false


/** onEditTrackNeedText()
 *
 * This is the event handler for the NEEDTEXT event.  It is invoked when the
 * edit tracking tool tip needs the text to display for the selection field.
 *
 * We simply send it the text of the currently selected item.
 */
::method onEditTrackNeedText unguarded
    use arg comboBox, toolTip, info

    info~text = comboBox~selected
    return .false


/** onEditTrackShow
 *
 * This is the event handler for the SHOW event.  It is invoked when the edit
 * tracking tool tip is about to show the tip.  It gives us the chance to do
 * custom positioning of the tip.  If we are not doing custom positioning of the
 * tip we simply return false to tell the tool tip to position the tip itself.
 *
 * If we are positioing the tip, we place it directly over the selection field.
 */
::method onEditTrackShow unguarded
    expose editRect doCustomPositioning
    use arg comboBox, toolTip

    if \ doCustomPositioning then return .false

    r = editRect~copy

    comboBox~client2screen(r)
    toolTip~setWindowPos(0, r~left, r~top, 0, 0, "NOACTIVATE NOSIZE NOZORDER")
    return .true



/** onButtonTrackShow
 *
 * This is the event handler for the SHOW event.  It is invoked when the button
 * tracking tool tip is about to show the tip.  It gives us the chance to do
 * custom positioning of the tip.  If we are not doing custom positioning of the
 * tip we simply return false to tell the tool tip to position the tip itself.
 *
 * If we are positioing the tip, we place it directly to the right of the
 * button.
 */
::method onButtonTrackShow unguarded
    expose buttonRect doCustomPositioning
    use arg comboBox, toolTip

    if \ doCustomPositioning then return .false

    r = buttonRect~copy

    comboBox~client2screen(r)
    toolTip~setWindowPos(0, r~right, r~top, 0, 0, "NOACTIVATE NOSIZE NOZORDER")
    return .true


/** onCloseUp()
 *
 * Event handler for the CLOSEUP event.  This method is invoked when the drop
 * down list is closed.
 *
 * We have 2 tracking tool tips in this program.  They must be manually shown
 * and hidden.  If one or the other is visble and the drop down is closed, it
 * will remain on the screen.  So, here we hide the tool tip if it is currently
 * visible.
 */
::method onCloseup unguarded
    expose ttEditTrack ttEditActive cbEdit ttButtonTrack ttButtonActive comboBox

    if ttEditActive then do
        ttEditTrack~trackActivate(self, cbEdit, .false)
        ttEditActive = .false
    end

    if ttButtonActive then do
        ttButtonTrack~trackActivate(self, comboBox, .false)
        ttButtonActive = .false
    end

    return 0


/** doDefault()
 *
 * The event handler for the 'Tool Tip positions the tool tips' radio button
 * click event.  We just need to flip our flag.
 */
::method doDefault unguarded
    expose doCustomPositioning
    doCustomPositioning = .false
    return 0


/** doCustom()
 *
 * The event handler for the 'Application positions the tool tips' radio button
 * click event.  We just need to flip our flag.
 */
::method doCustom unguarded
    expose doCustomPositioning
    doCustomPositioning = .true
    return 0


/** getComboItems()
 *
 * Simple convenience method, returns an array of the items we will add to the
 * combo box.
 */
::method getComboItems private

  entries = .array~of("New York, United States of America, North America",     -
                      "Johannesburg, South Africa, Africa",                    -
                      "Cape Town, South Africa, Africa",                       -
                      "Saint Petersburg, Russia, Europe",                      -
                      "Mexico City, Mexico, North America",                    -
                      "Los Angles, United States of America, North America",   -
                      "Casablanca, Morocco, Africa",                           -
                      "Pyongyang, North Korea, Asia",                          -
                      "New Taipei, Republic of China (Taiwan), Asia",          -
                      "Addis Ababa, Ethiopia, Africa",                         -
                      "Moscow, Russia, Europe",                                -
                      "Yokohama, Japan, Asia",                                 -
                      "Chongqing, China, Asia",                                -
                      "Berlin, Germany, Europe",                               -
                      "Rio de Janeiro, Brazil, South America",                 -
                      "Guangzhou, China, Asia",                                -
                      "São Paulo, Brazil, South America",                      -
                      "Karachi, Pakistan, Asia",                               -
                      "Istanbul, Turkey, Europe / Asia",                       -
                     )

  return entries


/** calcRects()
 *
 *  To properly set the tool info for our tool tips, and to properly position
 *  the tool tips when we do custom positioning, we need to calculate and save
 *  a number of rectangles.
 *
 *  We start off with the client rectangles of the combo box, the drop down list
 *  box, the selection field, and the button.  The client rects within the combo
 *  box do not include the borders of the controls.  We want to adjust those
 *  rects so that they do include the borders.
 *
 *  The other problem is that when the list box is dropped down, the list box
 *  has the mouse captured, so all mouse messages go to the list box and all
 *  mouse positions are relative to the client area of the list box.  So, we
 *  need to calculate a rectangle for the selection field and the button that is
 *  in client coordinates of the list box.
 *
 *  The 'd' argument passed in here is the .Directory object returned from
 *  getComboBoxInfo().  This directory object contains both the selection field
 *  client rect and the button client rect.
 */
::method calcRects private
    expose comboBox cbEdit cbListBox lbRect editRect buttonRect editOffsetRect buttonOffsetRect
    use arg d

    -- The button rect, edit rect, and lb rect do not include borders.  We
    -- basically want to adjust them to the combo box width and height.
    cbRect = comboBox~clientRect

    lbRect     = cbListBox~clientRect
    editRect   = d~textRect
    buttonRect = d~buttonRect

    lbRect~left  = cbRect~left
    lbRect~right = cbRect~right

    -- The top and bottom coordinates of both the text and button rects have to
    -- be in relationship to the client rect of the list box.  I.e. the
    -- bottoms are at the 0 coordinate of the list box client area and the tops
    -- are at 0 - the height of the combo box.
    height = cbRect~bottom - cbRect~top

    editRect~left   = cbRect~left
    editRect~top    = cbRect~top
    editRect~right  = buttonRect~left
    editRect~bottom = cbRect~bottom

    -- Button rect left stays the same.
    buttonRect~top    = cbRect~top
    buttonRect~right  = cbRect~right
    buttonRect~bottom = cbRect~bottom

    editOffsetRect = editRect~copy
    editOffsetRect~top -= height
    editOffSetRect~bottom -= height

    buttonOffsetRect = buttonRect~copy
    buttonOffsetRect~top -= height
    buttonOffsetRect~bottom -= height




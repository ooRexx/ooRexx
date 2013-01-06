/*//////////////////////////////////////////////////////////////////////////////
  ==============================================================================
  ViewMgr							 v00-01  23Apr12
  ----------
  A singleton component that manages Views and view-related function
  such as Popup Offsetting.

  = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

--::REQUIRES "ObjectMgr.rex"

::CLASS 'ViewMgr' PUBLIC

  :: ATTRIBUTE mvTable		-- Table of Models and their Views
  				--   NOT USED (it's in the ObjectMgr).
  :: ATTRIBUTE dlgOffset	-- A single number of dialog units by which a
  				--   child dialog is offset (vertically and
  				--   horizontally) from a parent.
  :: ATTRIBUTE parentOffsetDlg	-- The dialog from which a "child" dialog is
  				--   "popped up".

  /*----------------------------------------------------------------------------
    init - initialises the ViewMgr
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD init
    --expose dlgOffset
    say "ViewMgr-init."
    .local~my.ViewMgr = self
    self~mvTable = .Table~new
    self~dlgOffset = 200
    return
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    setPopupParent - Remembers the id of a parent dlg that is "popping up"
                     a child dialog.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD setPopupParent PUBLIC
    use strict arg parentDlg	-- the dialog id of the Parent View
    self~parentDlg = parentDlg
    say "ViewMgr-setPopupParent-01. Parent View =" parentDlg

  /*----------------------------------------------------------------------------
    offsetDlg  - Calculates the desired position of a "child" dialog given
                 the dlg id of the "parent" dialog and offsets it from the
                 parent dialog by the "dlgOffset".
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*  ::METHOD offsetDlg PUBLIC
    use strict arg childDlg
    say "ViewMgr-popupChild-01. Parent View, Child View:" parentDlg childDlg
    parentPos = parentView~getRealPos
    childPos = incr(dlgOffset, dlgOffset)
    return
*/

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    addView - Adds a View to mvTable.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD addView PUBLIC
    use strict arg model, view
    say "ViewMgr-addView-01."
    -- Check if view already exists:

    -- QUESTION! Does the ObjectMgr know about Views? Why should it?
    -- Answer - in this version, OvhjectMgr handles the View Table -
    --          'cos that's the way I did it (not really correct but
    --          there you go...).
/*
    dlg = .local~my.ObjectMgr~queryView(modelClass, modelInstance)
    if dlg \= .false then do		-- if View (dialog) exists
      dlg~show
      return .true
    end

    if exists then do
      -- surface view
      return .true
    end
    -- Create View:
*/
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  /*----------------------------------------------------------------------------
    showModel - Surface a View. Uses the ObjectMgr to see if the view already
                exists, and if so, to surface it; else the ObjectMgr causes the
                required view to be created.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ::METHOD showModel PUBLIC
    use strict arg modelClass, modelInstance
    say "ViewMgr-showModel-01: class / instance:" modelClass "/" modelInstance
    -- Get the ObjectMgr to do the work:
    dlg = .local~my.ObjectMgr~showModel(modelClass, modelInstance)
    if dlg = .false then do
      say "ViewMgr-showModel-02: bad response from ObjectMgr."
      return .false
    end
    else do
      say "ViewMgr-showModel-03: good response from ObjectMgr."
      return .true
    end

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------
    listMVTable - list the MV Table on the console.
                  NOT USED.
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*  ::METHOD listMVTable PUBLIC
    do i over self~mvTable
      say i tbl[i]
    end
*/
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*----------------------------------------------------------------------------

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*============================================================================*/

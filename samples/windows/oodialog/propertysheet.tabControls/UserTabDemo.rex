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
 * UserTabDemo.rex
 */
    sd = locate()
    .application~useGlobalConstDir('O', sd'rc\UserTabDemo.h')
    .application~autoDetection(.false)
    .application~defaultFont('Tahoma', 10)

    .constDir[IDC_TAB_MAIN]  = 500

    dlg = .UserTabDialog~new
    dlg~execute

::requires 'ooDialog.cls'

::class 'UserTabDialog' public subclass UserDialog

::method init
    expose pages descriptions

    self~init:super

    descriptions = self~getDescriptions

    title = "Acme Health Clinic Patient Intake Form  Version 1.00.0"
    opts  = 'MINIMIZEBOX MAXIMIZEBOX'

    if self~createCenter(500, 265, title, opts) then self~initCode = 0
    else self~initCode = 1

    pages = .array~new(4)

    return self~initCode


::method defineDialog

    self~createPushButton(IDOK, 380, 240, 50, 15, "DEFAULT", 'O&k')
    self~createPushButton(IDCANCEL, 440, 240, 50, 15, , '&Cancel')

    self~createTab(IDC_TAB_MAIN, 10, 10, 480, 220, 'FIXED FOCUSDOWN')

    self~connectTabEvent(IDC_TAB_MAIN, SELCHANGE, onSelChange)


::method initDialog unguarded
    expose tab patientDlg contactsDlg insuranceDlg historyDlg dlgRect displayRect isPositioned

    self~connectKeyPress(Cancel, .VK~F3, "NONE" )
    self~connectKeyPress(Cancel, .VK~F3, "SHIFT")

    tab = self~newTab(IDC_TAB_MAIN)

    tabs = .array~of("Patient", "Contacts", "Insurance", "History")
    do t over tabs
      tab~insert( , t)
    end

    tab~setSize(160,28)

    dlgRect     = self~calcPageSize
    displayRect = self~getDisplayRect

    isPositioned = .array~of(.false, .false, .false, .false)

    self~activatePatient


::method calcPageSize private
    expose tab

    r = tab~windowRect
    tab~calcDisplayRect(r)

    -- r is now the bounding rectangle of the display area of the tab control.
    -- We want to convert this to a point size rectangle, in dialog units.  The
    -- different page dialogs will use this rectangle to set their initial size.
    r~right = r~right - r~left
    r~bottom = r~bottom - r~top
    self~pixel2dlgUnit(r)

    return r


::method getDisplayRect private
    expose tab

    r = tab~windowRect;
    tab~calcDisplayRect(r);

    s = .Size~new(r~right - r~left, r~bottom - r~top)

    p = .Point~new(r~left, r~top)
    self~screen2client(p)

    return .Rect~new(p~x, p~y, s~width, s~height)


::method onSelChange unguarded
    expose tab patientDlg contactsDlg insuranceDlg historyDlg displayRect

    tabName = tab~selected

    select
      when tabName == "Patient"  then self~select(1)
      when tabName == "Contacts" then self~select(2)
      when tabName == "Insurance" then self~select(3)
      when tabName == "History" then self~select(4)
      otherwise nop
    end


::method select private unguarded
    expose tab patientDlg contactsDlg insuranceDlg historyDlg displayRect isPositioned dlgShowing
    use arg page

    select
      when page == 1 then do
          dlgShowing~hideFast
          patientDlg~showFast
          dlgShowing = patientDlg
          dlgShowing~update
      end

      when page == 2 then do
          if \ contactsDlg~isA(.PlainBaseDialog) then do
              self~activateContacts
              return
          end

          dlgShowing~hideFast

          if \ isPositioned[page] then do
              contactsDlg~setWindowPos(tab~hwnd, displayRect, "SHOWWINDOW NOREDRAW NOOWNERZORDER")
              isPositioned[page] = .true
          end
          else do
              contactsDlg~showFast
          end

          dlgShowing = contactsDlg
          dlgShowing~update
      end

      when page == 3 then do
          if \ insuranceDlg~isA(.PlainBaseDialog) then do
              self~activateInsurance
              return
          end

          dlgShowing~hideFast

          if \ isPositioned[page] then do
              insuranceDlg~setWindowPos(tab~hwnd, displayRect, "SHOWWINDOW NOREDRAW NOOWNERZORDER")
              isPositioned[page] = .true
          end
          else do
              insuranceDlg~showFast
          end

          dlgShowing = insuranceDlg
          dlgShowing~update
      end

      when page == 4 then do
          if \ historyDlg~isA(.PlainBaseDialog) then do
              self~activateHistory
              return
          end

          dlgShowing~hideFast

          if \ isPositioned[page] then do
              historyDlg~setWindowPos(tab~hwnd, displayRect, "SHOWWINDOW NOREDRAW NOOWNERZORDER")
              isPositioned[page] = .true
          end
          else do
              historyDlg~showFast
          end

          dlgShowing = historyDlg
          dlgShowing~update
      end

      otherwise nop
    end
    -- End select

    return 0


::method initialShowDlg unguarded
    expose patientDlg tab displayRect isPositioned dlgShowing

    patientDlg~setWindowPos(tab~hwnd, displayRect, "SHOWWINDOW NOOWNERZORDER")
    isPositioned[1] = .true
    dlgShowing = patientDlg


::method activatePatient private unguarded
    expose patientDlg dlgRect pages

    patientDlg = .PatientDlg~new(dlgRect, self)
    patientDlg~execute

    pages[1] = patientdlg

    -- .01 seconds == 10000 microseconds
    .Alarm~new(.TimeSpan~fromMicroseconds(20000), .message~new(self, "initialShowDlg"))


::method activateContacts private unguarded
    expose contactsDlg dlgRect pages

    reply

    contactsDlg = .ContactsDlg~new(dlgRect, self)
    contactsDlg~execute

    pages[2] = contactsDlg

    self~postOnSelChange(10)


::method activateInsurance private unguarded
    expose insuranceDlg dlgRect pages

    reply

    insuranceDlg = .InsuranceDlg~new(dlgRect, self)
    insuranceDlg~execute

    pages[3] = insuranceDlg

    self~postOnSelChange(10)


::method activateHistory private unguarded
    expose historyDlg dlgRect pages descriptions

    reply

    historyDlg = .HistoryDlg~new(dlgRect, self, descriptions)
    historyDlg~execute

    pages[4] = historyDlg

    self~postOnSelChange(10)


::method postOnSelChange private unguarded
    use strict arg microseconds

    .Alarm~new(.TimeSpan~fromMicroseconds(microseconds), .message~new(self, "onSelChange"))


::method ok unguarded
    expose pages

    do page over pages
      if page \== .nil then page~endExecution(.true)
    end

    return self~ok:super


/** cancel()
 *
 * A problem can happen if the user hits cancel immediately after selecting the
 * History tab.  The initDialog() method may be still executing and a syntax
 * condition can be raised if the underlying Windows dialog is ended.  To
 * prevent this we check for that condition and sleep a little bit if it
 * happens.
 */
::method cancel unguarded
    expose pages

    do page over pages
        if page \== .nil then do
            if page~isA(.HistoryDlg), \ page~initDialogDone then do while \ page~initDialogDone
                j = SysSleep(.5)
            end
            page~endExecution(.false)
        end
    end
    return self~cancel:super


::method getDescriptions private

    d = .array~new
    d[ 1] = 'Hearing loss / ringing in ears'
    d[ 2] = 'Heart diease / circulatory proplems'
    d[ 3] = 'Liver cancer / lung cancer'
    d[ 4] = 'Breast cancer / uterine cancer'
    d[ 5] = 'Jaundice, hepatitis'
    d[ 6] = 'Kidney stones / kidney failure'
    d[ 7] = 'High blood pressure / high chorlosterol'
    d[ 8] = 'Gall bladder trouble (gallstones)'
    d[ 9] = 'Anemia/blood disorder'
    d[10] = 'Sugar or albumin in urine'
    d[11] = 'Frequent or severe headache'
    d[12] = 'Periods of unconsciousness'
    d[13] = 'Chronic or frequent colds'
    d[14] = 'Loss of memory or amnesia'
    d[15] = 'Depression or excessive worry'
    d[16] = 'Car, train, sea or air sickness'

    return d



::class 'PatientDlg' subclass UserControlDialog

::method init
    use arg r, parent

    self~init:super( , , parent)
    self~create(r~left, r~top, r~right, r~bottom)

::method defineDialog

    self~createGroupBox(    IDC_STATIC,                 5,   5, 244,  65,                        , "Patient"                      )
    self~createStaticText(  IDC_STATIC,                14,  16,  95,   8, LEFT                   , "Name (Last, First, Middle):"  )
    self~createEdit(        IDC_EDIT_NAME,             14,  26, 110,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               136,  16, 105,   8, LEFT                   , "Also known as / Maiden name:" )
    self~createEdit(        IDC_EDIT_AKA,             136,  26, 105,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,                14,  43,  48,   8, LEFT                   , "Date of birth:"               )
    self~createEdit(        IDC_EDIT_DOB1,             14,  53,  17,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,                33,  55,   8,   8, CENTER                 , "/"                            )
    self~createEdit(        IDC_EDIT_DOB2,             44,  53,  17,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,                64,  55,   8,   8, CENTER                 , "/"                            )
    self~createEdit(        IDC_EDIT_DOB3,             74,  53,  17,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               102,  43,  22,   8, LEFT                   , "Age:"                         )
    self~createEdit(        IDC_EDIT_AGE,             102,  53,  22,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               136,  43,  86,   8, LEFT                   , "Social Security Number:"      )
    self~createEdit(        IDC_EDIT_SSN1,            136,  53,  19,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               195,  54,   8,   8, CENTER                 , "-"                            )
    self~createEdit(        IDC_EDIT_SSN2,            172,  53,  19,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               159,  54,   8,   8, CENTER                 , "-"                            )
    self~createEdit(        IDC_EDIT_SSN3,            209,  53,  31,  12, AUTOSCROLLH NUMBER                                      )
    self~createGroupBox(    IDC_STATIC,               258,   6,  35,  37,                        , "Sex"                          )
    self~createRadioButton( IDC_RB_M,                 269,  16,  15,   8, GROUP TAB              , "M"                            )
    self~createRadioButton( IDC_RB_F,                 269,  30,  15,   8,                        , "F"                            )
    self~createGroupBox(    IDC_STATIC,               311,   6, 159,  37,                        , "Language"                     )
    self~createRadioButton( IDC_RB_CHINESE,           322,  16,  39,   8, GROUP TAB              , "Chinese"                      )
    self~createRadioButton( IDC_RB_ENGLISH,           322,  30,  39,   8,                        , "English"                      )
    self~createRadioButton( IDC_RB_FRENCH,            374,  16,  39,   8,                        , "French"                       )
    self~createRadioButton( IDC_RB_GERMAN,            374,  30,  39,   8,                        , "German"                       )
    self~createRadioButton( IDC_RB_SPANISH,           422,  16,  39,   8,                        , "Spanish"                      )
    self~createRadioButton( IDC_RB_OTHER,             422,  30,  39,   8,                        , "Other"                        )
    self~createStaticText(  IDC_STATIC,               258,  54,  30,   8, LEFT                   , "Religon:"                     )
    self~createEdit(        IDC_EDIT_RELIGON,         289,  53,  72,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               373,  54,  30,   8, LEFT                   , "Church:"                      )
    self~createEdit(        IDC_EDIT_CHURCH,          405,  53,  61,  12, AUTOSCROLLH                                             )
    self~createGroupBox(    IDC_STATIC,                 5,  75, 184,  37,                        , "Race"                         )
    self~createRadioButton( IDC_RB_AFRICAN_AMERICAN,   14,  84,  68,   8, GROUP TAB              , "African American"             )
    self~createRadioButton( IDC_RB_NATIVE_AMERICAN,    14,  98,  68,   8,                        , "Native American"              )
    self~createRadioButton( IDC_RB_ASIAN,              91,  84,  39,   8,                        , "Asian"                        )
    self~createRadioButton( IDC_RB_CAUCASIAN,          91,  98,  47,   8,                        , "Caucasian"                    )
    self~createRadioButton( IDC_RB_HISPANIC,          146,  84,  39,   8,                        , "Hispanic"                     )
    self~createRadioButton( IDC_RB_OTHER_RACE,        146,  98,  39,   8,                        , "Other"                        )
    self~createGroupBox(    IDC_STATIC,               204,  75, 265,  37,                        , "Marital Status"               )
    self~createRadioButton( IDC_RB_MARRIED,           214,  84,  41,   8, GROUP TAB              , "Married"                      )
    self~createRadioButton( IDC_RB_SINGLE,            214,  98,  41,   8,                        , "Single"                       )
    self~createRadioButton( IDC_RB_NEVER_MARRIED,     269,  84,  65,   8,                        , "Never Married"                )
    self~createRadioButton( IDC_RB_LEGALLY_SEPARATED, 269,  98,  73,   8,                        , "Legally Separated"            )
    self~createRadioButton( IDC_RB_LIFEPARTNER,       351,  83,  53,   8,                        , "Life Partner"                 )
    self~createRadioButton( IDC_RB_DIVORCED,          415,  83,  48,   8,                        , "Divorced"                     )
    self~createRadioButton( IDC_RB_WIDOWED,           351,  98,  54,   8,                        , "Widowed"                      )
    self~createRadioButton( IDC_RB_OTHER_MARITAL,     415,  98,  39,   8,                        , "Other"                        )
    self~createGroupBox(    IDC_STATIC,                 5, 119, 144,  77,                        , "Address"                      )
    self~createStaticText(  IDC_STATIC,                12, 132,  24,   8, RIGHT                  , "Street:"                      )
    self~createEdit(        IDC_EDIT_PATIENT_ADDRESS,  38, 130, 107,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,                12, 148,  24,   8, RIGHT                  , "City:"                        )
    self~createEdit(        IDC_EDIT_PATIENT_CITY,     38, 146,  63,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               105, 148,  24,   8, RIGHT                  , "State:"                       )
    self~createEdit(        IDC_EDIT_PATIENT_STATE,   131, 146,  13,  12, AUTOSCROLLH UPPERCASE                                   )
    self~createStaticText(  IDC_STATIC,                12, 164,  24,   8, RIGHT                  , "Zip:"                         )
    self~createEdit(        IDC_EDIT_PATIENT_ZIP,      38, 162,  26,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,                77, 164,  24,   8, RIGHT                  , "Email:"                       )
    self~createEdit(        IDC_EDIT_EMAIL,           103, 162,  42,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,                12, 180,  24,   8, RIGHT                  , "Phone:"                       )
    self~createEdit(        IDC_EDIT_PATIENT_PHONE,    38, 178,  35,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,                77, 180,  24,   8, RIGHT                  , "Cell:"                        )
    self~createEdit(        IDC_EDIT_PATIENT_CELL,    103, 178,  42,  12, AUTOSCROLLH NUMBER                                      )
    self~createGroupBox(    IDC_STATIC,               165, 119, 208,  77,                        , "Employment"                   )
    self~createStaticText(  IDC_STATIC,               168, 132,  34,   8, RIGHT                  , "Employer:"                    )
    self~createEdit(        IDC_EDIT_EMPLOYER,        206, 130, 100,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               168, 147,  34,   8, RIGHT                  , "Street:"                      )
    self~createEdit(        IDC_EDIT_EMPLOYER_STREET, 206, 145, 100,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               168, 163,  34,   8, RIGHT                  , "City:"                        )
    self~createEdit(        IDC_EDIT_EMPLOYER_CITY,   206, 161,  60,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               269, 163,  21,   8, RIGHT                  , "State:"                       )
    self~createEdit(        IDC_EDIT_EMPLOYER_STATE,  293, 161,  13,  12, AUTOSCROLLH UPPER                                       )
    self~createStaticText(  IDC_STATIC,               168, 180,  34,   8, RIGHT                  , "Zip:"                         )
    self~createEdit(        IDC_EDIT_EMPLOYER_ZIP,    206, 178,  26,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               307, 133,  24,   8, RIGHT                  , "Phone:"                       )
    self~createEdit(        IDC_EDIT_EMPLOYER_PHONE,  333, 131,  35,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               307, 147,  24,   8, RIGHT                  , "Ext:"                         )
    self~createEdit(        IDC_EDIT_EMPLOYER_EXT,    333, 145,  35,  12, AUTOSCROLLH NUMBER                                      )
    self~createStaticText(  IDC_STATIC,               238, 180,  43,   8, RIGHT                  , "Occupation:"                  )
    self~createEdit(        IDC_EDIT_OCCUPATION,      283, 178,  84,  12, AUTOSCROLLH                                             )
    self~createGroupBox(    IDC_STATIC,               385, 119,  85,  77,                        , "Primary Care"                 )
    self~createStaticText(  IDC_STATIC,               395, 132,  66,   8, LEFT                   , "Primary Physician:"           )
    self~createEdit(        IDC_EDIT_PHYSICIAN,       395, 145,  71,  12, AUTOSCROLLH                                             )
    self~createStaticText(  IDC_STATIC,               388, 163,  66,   8, RIGHT                  , "Phone - Extension:"           )
    self~createEdit(        IDC_EDIT_PHYSICIAN_PHONE, 395, 178,  71,  12, AUTOSCROLLH NUMBER                                      )



::class 'ContactsDlg' subclass UserControlDialog

::method init
    use arg r, parent

    self~init:super( , , parent)
    self~create(r~left, r~top, r~right, r~bottom)

::method defineDialog

    self~createGroupBox(    IDC_STATIC,                    5,   5, 244, 192,                    , "Guarantor"                )
    self~createStaticText(  IDC_STATIC,                   14,  16,  95,   8, LEFT               , "Guarantor Name: "         )
    self~createEdit(        IDC_EDIT_NAME_G,              14,  26, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  136,  16, 105,   8, LEFT               , "Relationship to patient:" )
    self~createEdit(        IDC_EDIT_RELATION_G,         136,  26, 105,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                   14,  43,  48,   8, LEFT               , "Date of birth:"           )
    self~createEdit(        IDC_EDIT_DOB1_G,              14,  53,  17,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                   33,  55,   8,   8, CENTER             , "/"                        )
    self~createEdit(        IDC_EDIT_DOB2_G,              44,  53,  17,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                   64,  55,   8,   8, CENTER             , "/"                        )
    self~createEdit(        IDC_EDIT_DOB3_G,              74,  53,  17,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                  102,  43,  22,   8, LEFT               , "Age:"                     )
    self~createEdit(        IDC_EDIT_AGE_G,              102,  53,  22,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                  136,  43,  86,   8, LEFT               , "Social Security Number:"  )
    self~createEdit(        IDC_EDIT_SSN1_G,             136,  53,  19,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                  195,  54,   8,   8, CENTER             , "-"                        )
    self~createEdit(        IDC_EDIT_SSN2_G,             172,  53,  19,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                  159,  54,   8,   8, CENTER             , "-"                        )
    self~createEdit(        IDC_EDIT_SSN3_G,             209,  53,  31,  12, AUTOSCROLLH NUMBER                              )
    self~createGroupBox(    IDC_STATIC,                    6,  76,  35,  45,                    , "Sex"                      )
    self~createRadioButton( IDC_RB_M_G,                   17,  87,  15,   8, GROUP TAB          , "M"                        )
    self~createRadioButton( IDC_RB_F_G,                   17, 103,  15,   8,                    , "F"                        )
    self~createGroupBox(    IDC_STATIC,                   44,  76, 198,  45,                    , "Guarantor Employment"     )
    self~createStaticText(  IDC_STATIC,                   47,  89,  41,   8, RIGHT              , "Employer:"                )
    self~createEdit(        IDC_EDIT_EMPLOYER_G,          91,  87, 100,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                   47, 105,  41,   8, RIGHT              , "Occupation:"              )
    self~createEdit(        IDC_EDIT_OCCUPATION_G,        91, 103, 100,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  202,  90,  24,   8, RIGHT              , "Phone:"                   )
    self~createEdit(        IDC_EDIT_EMPLOYER_PHONE_G,   202, 103,  35,  12, AUTOSCROLLH NUMBER                              )
    self~createGroupBox(    IDC_STATIC,                   44, 127, 198,  70,                    , "Guarantor Address"        )
    self~createStaticText(  IDC_STATIC,                   64, 140,  24,   8, RIGHT              , "Street:"                  )
    self~createEdit(        IDC_EDIT_ADDRESS_G,           91, 138, 143,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                   64, 156,  24,   8, RIGHT              , "City:"                    )
    self~createEdit(        IDC_EDIT_CITY_G,              90, 154,  59,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  152, 156,  20,   8, RIGHT              , "State:"                   )
    self~createEdit(        IDC_EDIT_STATE_G,            175, 154,  13,  12, AUTOSCROLLH UPPER                               )
    self~createStaticText(  IDC_STATIC,                  189, 156,  15,   8, RIGHT              , "Zip:"                     )
    self~createEdit(        IDC_EDIT_ZIP_G,              207, 154,  26,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                   64, 172,  24,   8, RIGHT              , "Phone:"                   )
    self~createEdit(        IDC_EDIT_PHONE_G,             90, 170,  51,  12, AUTOSCROLLH NUMBER                              )
    self~createStaticText(  IDC_STATIC,                  148, 172,  24,   8, RIGHT              , "Cell:"                    )
    self~createEdit(        IDC_EDIT_CELL_G,             175, 170,  58,  12, AUTOSCROLLH NUMBER                              )
    self~createGroupBox(    IDC_STATIC,                  265,   5, 205,  91,                    , "Emergency Contact"        )
    self~createStaticText(  IDC_STATIC,                  274,  16,  77,   8, LEFT               , "Name: "                   )
    self~createEdit(        IDC_EDIT_NAME_EC,            274,  26, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  274,  43,  77,   8, LEFT               , "Relationship to patient:" )
    self~createEdit(        IDC_EDIT_RELATION_EC,        274,  53, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  274,  69,  41,   8, LEFT               , "Employer:"                )
    self~createEdit(        IDC_EDIT_EMPLOYER_EC,        274,  78, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  391,  16,  66,   8, LEFT               , "Phone:"                   )
    self~createEdit(        IDC_EDIT_PHONE_EC,           391,  26,  71,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  391,  43,  66,   8, LEFT               , "Cell Phone:"              )
    self~createEdit(        IDC_EDIT_CELL_EC,            391,  53,  71,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  391,  69,  66,   8, RIGHT              , "Employment Phone:"        )
    self~createEdit(        IDC_EDIT_EMPLOYER_PHONE_EC,  391,  78,  71,  12, AUTOSCROLLH NUMBER                              )
    self~createGroupBox(    IDC_STATIC,                  265, 106, 205,  91,                    , "Next of Kin"              )
    self~createStaticText(  IDC_STATIC,                  274, 117,  77,   8, LEFT               , "Name: "                   )
    self~createEdit(        IDC_EDIT_NAME_NOK,           274, 127, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  274, 144,  77,   8, LEFT               , "Relationship to patient:" )
    self~createEdit(        IDC_EDIT_RELATION_NOK,       274, 154, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  274, 170,  41,   8, LEFT               , "Employer:"                )
    self~createEdit(        IDC_EDIT_EMPLOYER_NOK,       274, 179, 110,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  391, 117,  66,   8, LEFT               , "Phone:"                   )
    self~createEdit(        IDC_EDIT_PHONE_NOK,          391, 127,  71,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  391, 144,  66,   8, LEFT               , "Cell Phone:"              )
    self~createEdit(        IDC_EDIT_CELL_NOK,           391, 154,  71,  12, AUTOSCROLLH                                     )
    self~createStaticText(  IDC_STATIC,                  391, 170,  66,   8, RIGHT              , "Employment Phone:"        )
    self~createEdit(        IDC_EDIT_EMPLOYER_PHONE_NOK, 391, 179,  71,  12, AUTOSCROLLH NUMBER                              )




::class 'InsuranceDlg' subclass UserControlDialog

::method init
    use arg r, parent

    self~init:super( , , parent)
    self~create(r~left, r~top, r~right, r~bottom)


::method defineDialog

    self~createGroupBox(   IDC_STATIC,                5,   5, 225, 192,                    , "Primary Insurance"         )
    self~createStaticText( IDC_STATIC,               28,  16,  95,   8, LEFT               , "Insurance Company Name: "  )
    self~createEdit(       IDC_EDIT_NAME_PI,         28,  26, 110,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              152,  16,  65,   8, LEFT               , "Group"                     )
    self~createEdit(       IDC_EDIT_GROUP_PI,       152,  26,  65,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,               28,  43,  48,   8, LEFT               , "Effectiv Date:"            )
    self~createEdit(       IDC_EDIT_DATE1_PI,        28,  53,  17,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,               47,  55,   8,   8, CENTER             , "/"                         )
    self~createEdit(       IDC_EDIT_DATE2_PI,        58,  53,  17,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,               78,  55,   8,   8, CENTER             , "/"                         )
    self~createEdit(       IDC_EDIT_DATE3_PI,        88,  53,  17,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              124,  43,  65,   8, LEFT               , "Certificate Number:"       )
    self~createEdit(       IDC_EDIT_CERTIFICATE_PI, 124,  53,  93,  12, AUTOSCROLLH NUMBER                               )
    self~createGroupBox(   IDC_STATIC,               20, 138, 198,  58,                    , "Subscriber"                )
    self~createStaticText( IDC_STATIC,               23, 150,  41,   8, RIGHT              , "Name:"                     )
    self~createEdit(       IDC_EDIT_SUBSCRIBER_PI,   67, 148, 144,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,               23, 166,  41,   8, RIGHT              , "Employer:"                 )
    self~createEdit(       IDC_EDIT_EMPLOYER_PI,     67, 164, 144,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,               23, 184,  78,   8, RIGHT              , "Relationship to patient:"  )
    self~createEdit(       IDC_EDIT_RELATION_PI,    104, 182, 107,  12, AUTOSCROLLH NUMBER                               )
    self~createGroupBox(   IDC_STATIC,               19,  69, 198,  62,                    , "Insurance Company Address" )
    self~createStaticText( IDC_STATIC,               28,  82,  35,   8, RIGHT              , "Street:"                   )
    self~createEdit(       IDC_EDIT_ADDRESS_PI,      66,  80, 143,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,               28,  98,  35,   8, RIGHT              , "City:"                     )
    self~createEdit(       IDC_EDIT_CITY_PI,         65,  96,  59,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              127,  98,  20,   8, RIGHT              , "State:"                    )
    self~createEdit(       IDC_EDIT_STATE_PI,       150,  96,  13,  12, AUTOSCROLLH UPPER                                )
    self~createStaticText( IDC_STATIC,              164,  98,  15,   8, RIGHT              , "Zip:"                      )
    self~createEdit(       IDC_EDIT_ZIP_PI,         182,  96,  26,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,               28, 114,  35,   8, RIGHT              , "Phone 1:"                  )
    self~createEdit(       IDC_EDIT_PHONE1_PI,       65, 112,  51,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              116, 114,  31,   8, RIGHT              , "Phone 2:"                  )
    self~createEdit(       IDC_EDIT_PHONE2_PI,      150, 112,  58,  12, AUTOSCROLLH NUMBER                               )
    self~createGroupBox(   IDC_STATIC,              245,   5, 225, 192,                    , "Secondary Insurance"       )
    self~createStaticText( IDC_STATIC,              268,  16,  95,   8, LEFT               , "Insurance Company Name: "  )
    self~createEdit(       IDC_EDIT_NAME_SI,        268,  26, 110,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              392,  16,  65,   8, LEFT               , "Group"                     )
    self~createEdit(       IDC_EDIT_GROUP_SI,       392,  26,  65,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              268,  43,  48,   8, LEFT               , "Effectiv Date:"            )
    self~createEdit(       IDC_EDIT_DATE1_SI,       268,  53,  17,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              287,  53,   8,   8, CENTER             , "/"                         )
    self~createEdit(       IDC_EDIT_DATE2_SI,       298,  53,  17,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              318,  53,   8,   8, CENTER             , "/"                         )
    self~createEdit(       IDC_EDIT_DATE3_SI,       328,  53,  17,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              364,  43,  65,   8, LEFT               , "Certificate Number:"       )
    self~createEdit(       IDC_EDIT_CERTIFICATE_SI, 364,  53,  93,  12, AUTOSCROLLH NUMBER                               )
    self~createGroupBox(   IDC_STATIC,              259, 138, 198,  58,                    , "Subscriber"                )
    self~createStaticText( IDC_STATIC,              263, 150,  41,   8, RIGHT, "Name:"                                   )
    self~createEdit(       IDC_EDIT_SUBSCRIBER_SI,  307, 148, 144,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              263, 166,  41,   8, RIGHT              , "Employer:"                 )
    self~createEdit(       IDC_EDIT_EMPLOYER_SI,    307, 164, 144,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              263, 184,  78,   8, RIGHT              , "Relationship to patient:"  )
    self~createEdit(       IDC_EDIT_RELATION_SI,    343, 182, 107,  12, AUTOSCROLLH NUMBER                               )
    self~createGroupBox(   IDC_STATIC,              259,  69, 198,  62,                    , "Insurance Company Address" )
    self~createStaticText( IDC_STATIC,              268,  80,  35,   8, RIGHT              , "Street:"                   )
    self~createEdit(       IDC_EDIT_ADDRESS_SI,     306,  80, 143,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              268,  96,  35,   8, RIGHT              , "City:"                     )
    self~createEdit(       IDC_EDIT_CITY_SI,        305,  96,  59,  12, AUTOSCROLLH                                      )
    self~createStaticText( IDC_STATIC,              367,  96,  20,   8, RIGHT              , "State:"                    )
    self~createEdit(       IDC_EDIT_STATE_SI,       390,  96,  13,  12, AUTOSCROLLH UPPER                                )
    self~createStaticText( IDC_STATIC,              404,  96,  15,   8, RIGHT              , "Zip:"                      )
    self~createEdit(       IDC_EDIT_ZIP_SI,         422,  96,  26,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              268, 112,  35,   8, RIGHT              , "Phone 1:"                  )
    self~createEdit(       IDC_EDIT_PHONE1_SI,      305, 112,  51,  12, AUTOSCROLLH NUMBER                               )
    self~createStaticText( IDC_STATIC,              356, 112,  31,   8, RIGHT              , "Phone 2:"                  )
    self~createEdit(       IDC_EDIT_PHONE2_SI,      390, 112,  58,  12, AUTOSCROLLH NUMBER                               )



::class 'HistoryDlg' subclass UserControlDialog

::attribute initDialogDone unguarded

::method init
    expose descriptions
    use arg r, parent, descriptions

    self~init:super( , , parent)

    self~initDialogDone = .false
    self~create(r~left, r~top, r~right, r~bottom)

::method defineDialog

    gbText = "Indicate if you or anyone in your family has (or has had) any of these conditions:"
    st1Txt = "Yourself"
    st2Txt = "Family Member"

    self~createGroupBox(   IDC_STATIC   ,   5,   5, 465, 192,                                , gbText)
    self~createStaticText( IDC_STATIC   ,   6,  17, 229,  10, CENTER                         , st1Txt)
    self~createListView(   IDC_LV_SELF  ,   5,  30, 229, 167, TAB ALIGNLEFT SINGLESEL REPORT         )
    self~createStaticText( IDC_STATIC   , 241,  17, 229,  10, CENTER                         , st2Txt)
  	self~createListView(   IDC_LV_FAMILY, 241,  30, 229, 167, TAB ALIGNLEFT SINGLESEL REPORT         )

::method initDialog unguarded
    expose lvSelf lvFamily descriptions

    lvSelf = self~newListView(IDC_LV_SELF)

    lvSelf~addExtendedStyle('CHECKBOXES')

    s = lvSelf~getRealSize
    s~width -= (.SM~cxVscroll * 2)

    lvSelf~insertColumnPx(0, "Description", s~width)

    do d over descriptions
      lvSelf~addRow( , , d)
    end

    lvFamily = self~newListView(IDC_LV_FAMILY)

    lvFamily~addExtendedStyle('CHECKBOXES')

    s = lvFamily~getRealSize
    s~width -= (.SM~cxVscroll * 2)

    lvFamily~insertColumnPx(0, "Description", s~width)

    do d over descriptions
      lvFamily~addRow( , , d)
    end

    self~initDialogDone = .true


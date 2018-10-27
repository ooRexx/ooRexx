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
 * oodViewControls.cpp
 *
 * Contains methods for the DateTimePicker, MonthCalendar, and Tab controls.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"
#include "oodShared.hpp"

/**
 * Methods for the DateTimePicker class.
 */
#define DATETIMEPICKER_CLASS     "DateTimePicker"
#define DATETIMEPICKER_WINNAME   "Date and Time Picker"

// The following are used for DateTimePicker and MonthCalendar.
#define SYSTEMTIME_MIN_YEAR               1601
#define SYSTEMTIME_RANGE_EXCEPTION_MSG    "indexes 1 and 2 of argument 1, the array object, can not both be missing"

#define MC_PART_NAMES                     "BACKGROUND, MONTHBK, TEXT, TITLEBK, TITLETEXT, or TRAILINGTEXT"
#define MC_VIEW_NAMES                     "MONTHLY ANNUAL DECADE CENTURY"
#define MC_GRIDINFO_PART_NAMES            "control, next, prev, footer, calendard, header, body, row, cell"
#define MC_GRIDINFO_WHAT_FLAG_ERR_MSG     "must contain at least one of the keywords: date, rect, or name"

/**
 * Converts a DateTime object to a SYSTEMTIME structure.  The fields of the
 * struct are filled in with the corresponding values of the DateTime object.
 *
 * @param c         The method context we are operating in.
 * @param dateTime  An ooRexx DateTime object.
 * @param sysTime   [in/out] The SYSTEMTIME struct to fill in.
 * @param part      Specifies which fields of the SYSTEMTIME struct fill in.
 *                  Unspecified fields are left alone.
 *
 * @return True if no errors, false if a condition is raised.
 *
 * @note  Assumes the dateTime object is not null and is actually a DateTime
 *        object.
 *
 * @note The year part of the DateTime object must be in range for a SYSTEMTIME.
 *       The lower range for SYSTEMTIME is 1601. The upper range of a DateTime
 *       object is 9999 and of a SYSTEMTIME 30827, so we only check the lower
 *       range.  An exception is raised if out of range.
 */
bool dt2sysTime(RexxThreadContext *c, RexxObjectPtr dateTime, SYSTEMTIME *sysTime, DateTimePart part)
{
    if ( part == dtNow )
    {
        GetLocalTime(sysTime);
    }
    else
    {
        // format: yyyy-dd-mmThh:mm:ss.uuuuuu.
        RexxObjectPtr dt = c->SendMessage0(dateTime, "ISODATE");
        const char *isoDate = c->CString(dt);

        sscanf(isoDate, "%4hu-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &(*sysTime).wYear, &(*sysTime).wMonth, &(*sysTime).wDay,
               &(*sysTime).wHour, &(*sysTime).wMinute, &(*sysTime).wSecond, &(*sysTime).wMilliseconds);

        SYSTEMTIME st = {0};
        sscanf(isoDate, "%4hu-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &st.wYear, &st.wMonth, &st.wDay,
               &st.wHour, &st.wMinute, &st.wSecond, &st.wMilliseconds);

        if ( st.wYear < SYSTEMTIME_MIN_YEAR )
        {
            userDefinedMsgException(c, "The DateTime object can not represent a year prior to 1601");
            goto failed_out;
        }

        switch ( part )
        {
            case dtTime :
                sysTime->wHour = st.wHour;
                sysTime->wMinute = st.wMinute;
                sysTime->wSecond = st.wSecond;
                sysTime->wMilliseconds = st.wMilliseconds;
                break;

            case dtDate :
                sysTime->wYear = st.wYear;
                sysTime->wMonth = st.wMonth;
                sysTime->wDay = st.wDay;
                break;

            case dtFull :
                sysTime->wYear = st.wYear;
                sysTime->wMonth = st.wMonth;
                sysTime->wDay = st.wDay;
                sysTime->wHour = st.wHour;
                sysTime->wMinute = st.wMinute;
                sysTime->wSecond = st.wSecond;
                sysTime->wMilliseconds = st.wMilliseconds;
                break;
        }
    }
    return true;

failed_out:
    return false;
}


/**
 * Given an array of DateTime objects, converts them to an an array of
 * SYSTEMTIME structs.
 *
 * The array is assumed to be argument 1 of a method and only the first two
 * indexes are converted.
 *
 * @param c          The method context we are operating in.
 * @param dateTimes  The Rexx array of DateTime objects.
 * @param sysTimes   [IN/OUT] On entry an array of 2 SYSTEMTIMEs, on return the
 *                   SYSTEMTIME structs will be filled in according to the other
 *                   arguments.
 * @param part       Should the conversion be just the time portion, just the
 *                   date portion, or both.
 * @param needBoth   Are both index 1 and index 2 in the Rexx array required.
 * @param gdtr       [IN/OUT] Pointer to a variable to receive the GDTR_xxx
 *                   flags for the conversion.  This is ignored if NULL.
 *
 * @return True on success, false otherwise.  If false, an exceptions has been
 *         raised.
 */
bool dt2sysTimeRange(RexxMethodContext *c, RexxArrayObject dateTimes, SYSTEMTIME *sysTimes,
                     DateTimePart part, bool needBoth, uint32_t *gdtr)
{
    memset(sysTimes, 0, 2 * sizeof(SYSTEMTIME));

    RexxObjectPtr startDate = c->ArrayAt(dateTimes, 1);
    RexxObjectPtr endDate = c->ArrayAt(dateTimes, 2);

    if ( needBoth && (startDate == NULLOBJECT || endDate == NULLOBJECT) )
    {
        sparseArrayException(c->threadContext, 1, (startDate == NULLOBJECT ? 1 : 2));
        goto err_out;
    }

    uint32_t gdtrVal = 0;

    if ( startDate != NULLOBJECT )
    {
        if ( ! c->IsOfType(startDate, "DATETIME") )
        {
            wrongObjInArrayException(c->threadContext, 1, 1, "a DateTime object", startDate);
            goto err_out;
        }

        if ( ! dt2sysTime(c->threadContext, startDate, sysTimes, part) )
        {
            goto err_out;
        }
        gdtrVal |= GDTR_MIN;
    }

    if ( endDate != NULLOBJECT )
    {
        if ( ! c->IsOfType(endDate, "DATETIME") )
        {
            wrongObjInArrayException(c->threadContext, 1, 2, "a DateTime object", endDate);
            goto err_out;
        }

        if ( ! dt2sysTime(c->threadContext, endDate, sysTimes + 1, part) )
        {
            goto err_out;
        }
        gdtrVal |= GDTR_MAX;
    }

    if ( gdtrVal == 0 )
    {
        userDefinedMsgException(c->threadContext, SYSTEMTIME_RANGE_EXCEPTION_MSG);
    }

    if ( gdtr != NULL )
    {
        *gdtr = gdtrVal;
    }
    return true;

err_out:
    return false;
}

/**
 * Gets the time range (minimum and maximum allowable times) for a MonthCalendar
 * or a DateTimePicker control
 *
 * @param c
 * @param range
 * @param hCtrl
 * @param ctrlType
 *
 * @return CSTRING
 */
CSTRING getTimeRange(RexxMethodContext *c, RexxArrayObject range, HWND hCtrl, oodControl_t ctrlType)
{
    SYSTEMTIME sysTime[2];
    memset(&sysTime, 0, 2 * sizeof(SYSTEMTIME));

    DateTimePart dtPart;
    uint32_t ret;

    if ( ctrlType == winMonthCalendar )
    {
        dtPart = dtDate;
        ret =  MonthCal_GetRange(hCtrl, &sysTime);
    }
    else
    {
        dtPart = dtFull;
        ret =  DateTime_GetRange(hCtrl, &sysTime);
    }

    RexxObjectPtr minDate;
    RexxObjectPtr maxDate;

    CSTRING result;
    switch ( ret )
    {
        case 0 :
            result = "none";
            minDate = TheZeroObj;
            maxDate = TheZeroObj;
            break;
        case (GDTR_MIN | GDTR_MAX) :
            sysTime2dt(c->threadContext, (SYSTEMTIME *)&sysTime, &minDate, dtPart);
            sysTime2dt(c->threadContext, (SYSTEMTIME *)&sysTime + 1, &maxDate, dtPart);
            result = "both";
            break;
        case GDTR_MIN :
            result = "min";
            sysTime2dt(c->threadContext, (SYSTEMTIME *)&sysTime, &minDate, dtPart);
            maxDate = TheZeroObj;
            break;
        case GDTR_MAX :
            result = "max";
            minDate = TheZeroObj;
            sysTime2dt(c->threadContext, (SYSTEMTIME *)&sysTime + 1, &maxDate, dtPart);
            break;
        default :
            result = "error";  // I don'think this is possible.
            minDate = TheZeroObj;
            maxDate = TheZeroObj;
            break;
    }

    c->ArrayPut(range, minDate, 1);
    c->ArrayPut(range, maxDate, 2);

    return result;
}


/**
 * Creates a DateTime object that represents the time set in a SYSTEMTIME
 * struct.
 *
 * @param c
 * @param sysTime
 * @param dateTime  [in/out]
 */
void sysTime2dt(RexxThreadContext *c, SYSTEMTIME *sysTime, RexxObjectPtr *dateTime, DateTimePart part)
{
    RexxClassObject dtClass = c->FindClass("DATETIME");

    if ( part == dtNow )
    {
        *dateTime = c->SendMessage0(dtClass, "NEW");
    }
    else
    {
        char buf[64];
        switch ( part )
        {
            case dtDate :
                _snprintf(buf, sizeof(buf), "%hu%02hu%02hu", sysTime->wYear, sysTime->wMonth, sysTime->wDay);
                *dateTime = c->SendMessage1(dtClass, "FROMSTANDARDDATE", c->String(buf));
                break;

            case dtTime :
                _snprintf(buf, sizeof(buf), "%02hu:%02hu:%02hu.%03hu000",
                          sysTime->wHour, sysTime->wMinute, sysTime->wSecond, sysTime->wMilliseconds);
                *dateTime = c->SendMessage1(dtClass, "FROMLONGTIME", c->String(buf));
                break;

            case dtFull :
                _snprintf(buf, sizeof(buf), "%hu-%02hu-%02huT%02hu:%02hu:%02hu.%03hu000",
                          sysTime->wYear, sysTime->wMonth, sysTime->wDay,
                          sysTime->wHour, sysTime->wMinute, sysTime->wSecond, sysTime->wMilliseconds);
                *dateTime = c->SendMessage1(dtClass, "FROMISODATE", c->String(buf));
                break;
        }
    }
}


/**
 * Converts the string part to the month calendar part flage
 *
 * @param part   Keyword for the calendar part.
 *
 * @return The calendar part flag, or -1 on error.
 */
static uint32_t calPart2flag(RexxMethodContext *c, CSTRING part, size_t argPos)
{
    // This is an invalid flag.  When used in the DateTime_xx or MonthCalenddar_XX
    // macros, the macros then return the error code.
    uint32_t flag = (uint32_t)-1;

    if (      stricmp(part, "BACKGROUND"  ) == 0 ) flag = MCSC_BACKGROUND;
    else if ( stricmp(part, "MONTHBK"     ) == 0 ) flag = MCSC_MONTHBK;
    else if ( stricmp(part, "TEXT"        ) == 0 ) flag = MCSC_TEXT;
    else if ( stricmp(part, "TITLEBK"     ) == 0 ) flag = MCSC_TITLEBK;
    else if ( stricmp(part, "TITLETEXT"   ) == 0 ) flag = MCSC_TITLETEXT;
    else if ( stricmp(part, "TRAILINGTEXT") == 0 ) flag = MCSC_TRAILINGTEXT;
    else
    {
        wrongArgValueException(c->threadContext, argPos, MC_PART_NAMES, part);
    }

    return flag;
}


/**
 * Produce a string representation of a Month Calendar's style.
 */
static RexxStringObject mcStyle2String(RexxMethodContext *c, uint32_t style)
{
    char buf[256];
    buf[0] = '\0';

    if ( style & MCS_DAYSTATE )         strcat(buf, "DAYSTATE "   );
    if ( style & MCS_MULTISELECT )      strcat(buf, "MULTI "      );
    if ( style & MCS_NOTODAY )          strcat(buf, "NOTODAY "    );
    if ( style & MCS_NOTODAYCIRCLE )    strcat(buf, "NOCIRCLE "   );
    if ( style & MCS_WEEKNUMBERS )      strcat(buf, "WEEKNUMBERS ");
    if ( style & MCS_NOTRAILINGDATES )  strcat(buf, "NOTRAILING " );
    if ( style & MCS_SHORTDAYSOFWEEK )  strcat(buf, "SHORTDAYS "  );
    if ( style & MCS_NOSELCHANGEONNAV ) strcat(buf, "NOSELCHANGE ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}


/** DateTimePicker::closeMonthCal()
 *
 *  Closes the drop down month calendar control of the date time picker.
 *
 *  @return  This method always returns zero
 *
 *  @note    Requires Vista or later.  This method causes the date time picker
 *           to destroy the month calendar control and to send a DTN_CLOSEUP
 *           notification (the CLOSEUP event notification) that the control is
 *           closing.
 */
RexxMethod2(RexxObjectPtr, dtp_closeMonthCal, RexxObjectPtr, _size, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "closeMonthCal", "Vista", Vista_OS) )
    {
        return TheZeroObj;
    }

    DateTime_CloseMonthCal(getDChCtrl(pCSelf));
    return TheZeroObj;
}


/** DateTimePicker::getDateTime()
 *
 *  Retrieves the current selected system time of the date time picker and
 *  returns it as a DateTime object.
 *
 *  If the date time picker has the DTS_SHOWNONE style, it can also be set to
 *  "no date" when the user has unchecked the check box.  If the control is in
 *  this state, the .nil object is returned to the user.
 *
 *  @returns  A DateTime object representing the current selected system time of
 *            the control, or the .nil object if the control is in the
 *            'no date' state.
 *
 *  @note    The ooDialog framework will set this .systemErrorCode if GDT_ERROR
 *           is returned from  DateTime_GetSystemtime()
 *
 *           1002  ERROR_INVALID_MESSAGE
 *
 *           The window cannot act on the sent message.
 */
RexxMethod1(RexxObjectPtr, dtp_getDateTime, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    SYSTEMTIME sysTime = {0};
    RexxObjectPtr dateTime = TheNilObj;

    switch ( DateTime_GetSystemtime(getDChCtrl(pCSelf), &sysTime) )
    {
        case GDT_VALID:
            sysTime2dt(context->threadContext, &sysTime, &dateTime, dtFull);
            break;

        case GDT_NONE:
            // This is valid.  It means the DTP is using the DTS_SHOWNONE  style
            // and that the user has the check box is not checked.  We return
            // the .nil object.
            break;

        case GDT_ERROR:
        default :
            // Some error with the DTP, set .systemErrorCode.
            oodSetSysErrCode(context->threadContext, 1002);
            dateTime = TheZeroObj;
            break;
    }
    return dateTime;
}

/** DateTimePicker::getInfo()
 *
 *  Returns a Directory object with information about the date time picker.
 *
 *  @note  The directory object will contain the following indexes:
 *
 *         CHECKRECT:  A .Rect object describing location of the checkbox, if
 *         the date time picker has the SHOWNONE style.  If a checkbox is
 *         displayed and checked, an edit control should be available to update
 *         the selected date-time value.
 *
 *         CHECKSTATE:  A list of <object state> keywords describing the state
 *         of the check box.
 *
 *         BUTTONRECT:  A RECT structure describing the location of the
 *         drop-down grid or the up/down control.
 *
 *         BUTTONSTATE:  A list of <object state> keywords describing the state
 *         of the check box.
 *
 *         EDIT:  The Edit control object.
 *
 *         DROPDOWN:  The MonthCalendar control object.
 *
 *         UPDOWN:  The UpDown control object.
 */
RexxMethod1(RexxObjectPtr, dtp_getInfo, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getInfo", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hDTP = getDChCtrl(pCSelf);
    RexxDirectoryObject result = context->NewDirectory();

    DATETIMEPICKERINFO info = {0};
    info.cbSize = sizeof(DATETIMEPICKERINFO);

    DateTime_GetDateTimePickerInfo(hDTP, &info);

    context->DirectoryPut(result, rxNewRect(context, (PORXRECT)&(info.rcCheck)), "CHECKRECT");
    context->DirectoryPut(result, objectStateToString(context, info.stateCheck), "CHECKSTATE");

    context->DirectoryPut(result, rxNewRect(context, (PORXRECT)&(info.rcButton)), "BUTTONRECT");
    context->DirectoryPut(result, objectStateToString(context, info.stateButton), "BUTTONSTATE");

    RexxObjectPtr ctrl = createControlFromHwnd(context, (pCDialogControl)pCSelf, info.hwndDropDown, winMonthCalendar, false);
    context->DirectoryPut(result, ctrl, "DROPDOWN");

    ctrl = createControlFromHwnd(context, (pCDialogControl)pCSelf, info.hwndEdit, winEdit, true);
    context->DirectoryPut(result, ctrl, "EDIT");

    ctrl = createControlFromHwnd(context, (pCDialogControl)pCSelf, info.hwndUD, winUpDown, true);
    context->DirectoryPut(result, ctrl, "UPDOWN");

    return result;
}


/** DateTimePicker::getIdealSize()
 *
 *  Gets the size needed to display the date time picker without clipping.
 *
 *  @param  size  [IN/OUT] A .Size object, on return the object will be set to
 *                the ideal size.
 *
 *  @return  This method always returns true, becuase it will always succeed.
 *           However, if the size argument is not a .Size object, a syntax
 *           condition is raised.
 *
 *  @note    Requires Vista or later.
 */
RexxMethod2(RexxObjectPtr, dtp_getIdealSize, RexxObjectPtr, _size, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getIdealSize", "Vista", Vista_OS) )
    {
        return TheZeroObj;
    }

    PSIZE s = (PSIZE)rxGetSize(context, _size, 1);
    if ( s != NULL )
    {
        DateTime_GetIdealSize(getDChCtrl(pCSelf), s);
    }
    return TheTrueObj;
}


/** DateTimePicker::getMonthCal()
 *
 *  Gets the MonthCalendar object for a date and time picker's (DTP) child month
 *  calendar control.
 *
 *  One reason for gettng the MonthCalendar object is to customize the dropdown
 *  month-calendar control. For instance, if you don't want the "Go To Today,"
 *  you need to set the control's NOTODAY style.  Use this method to retrieve
 *  the MonthCalendar object.  You can then use this object to set the desired
 *  month-calendar style, or otherwise customize the control.
 *
 *  @return  A MonthCalendar object for the underlying child month calendar, if
 *           the child control exists, otherwise the .nil object.
 *
 *  @note  DTP controls create a child month calendar control when the user
 *         clicks the drop-down arrow.  When the month calendar is no longer
 *         needed, it is destroyed.  So your program must not rely on a using a
 *         single MonthCalendar object for the DTP's child month calendar.
 *
 *         Rather, you should instantiate a new MonthCalendar, if you need one,
 *         each time the user clicks the drop-down arrow.  Connect the DROPDOWN
 *         event to know when a new month calendar control is created.  After
 *         the month calendar is created, use this method to instantiate a new
 *         MonthCalendar object.  Connect the CLOSEUP event to know when the
 *         month calendar control is destroyed. Once the close up event is
 *         received, the MonthCalendar object will no longer be valid.  Invoking
 *         methods on the object will raise a syntax condition.
 *
 *  @note  We do *not* have the new Rexx month calendar object put in the Rexx
 *         dialog's control bag to protect it from garbage collection.
 */
RexxMethod1(RexxObjectPtr, dtp_getMonthCal, CSELF, pCSelf)
{
    HWND hDTP = getDChCtrl(pCSelf);
    return createControlFromHwnd(context, (pCDialogControl)pCSelf, DateTime_GetMonthCal(hDTP), winMonthCalendar, false);
}


/** DateTimePicker::getMonthCalColor()
 *
 *  Gets the color for a given portion of the month calendar within a date and
 *  time picker (DTP) control.
 *
 *  @param  calPart  Keyword specifying which part of the month calendar to get
 *                   the color for.
 *
 *  @return  The color for the part specified as a COLORREF, or CLR_NONE on
 *           error.
 *
 *  @note  Using the wrong keyword, mispelled, etc., will result in CLR_NONE
 *         being returned.
 */
RexxMethod2(uint32_t, dtp_getMonthCalColor, CSTRING, calPart, CSELF, pCSelf)
{
    int32_t flag = calPart2flag(context, calPart, 1);
    if ( flag == -1 )
    {
        return CLR_INVALID;
    }
    return (COLORREF)DateTime_GetMonthCalColor(getDChCtrl(pCSelf), flag);
}


/** DateTimePicker::getMonthCalStyle()
 *
 *  Gets the month calendar style for the date and time picker.
 *
 *  @return  A string of keywords that consist of the style of the month
 *           calendar control used by the date time picker.
 *
 *  @note  See <LINK to DynamicDialog::createMonthCalendar> for the month
 *         calendar style keywords.
 *
 *         Vista or later only.
 */
RexxMethod1(RexxObjectPtr, dtp_getMonthCalStyle, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getMonthCalStyle", "Vista", Vista_OS) )
    {
        return TheZeroObj;
    }

    uint32_t style = (uint32_t)DateTime_GetMonthCalStyle(getDChCtrl(pCSelf));
    return mcStyle2String(context, style);
}


/** DateTimePicker::getRange()
 *
 *  Gets the current minimum and maximum allowable system times for a date and
 *  time picker (DTP) control.
 *
 *  @param  range  [IN/OUT] An array in which the minimum and maximum times are
 *                 returned as .DateTime objects.  The minimum time will be at
 *                 index 1 and the maximum at index 2.  If either index is set
 *                 to zero, then no corresponding limit is set for the date time
 *                 picker control.
 *
 *  @return  A keyword indicating the result.  See below for the possible
 *           keywords
 *
 *  @note  The returned keyword indicates whether a minimum or maximum limit has
 *         been set for the DTP.  The keyword will be one of: none, min, max,
 *         both, or possibly error.  (An error is unlikely.)
 */
RexxMethod2(CSTRING, dtp_getRange, RexxArrayObject, range, CSELF, pCSelf)
{
    return getTimeRange(context, range, getDChCtrl(pCSelf), winDateTimePicker);
}

/** DateTimePicker::setDateTime()
 *
 *  Sets the system time for the date time picker to the time represented by the
 *  DateTime object.  If, and only if, the date time picker has the DTS_SHOWNONE
 *  style, it can also be set to "no date."  The Rexx user can set this state by
 *  passing in the .nil object.
 *
 *  @param dateTime  The date and time to set the control to.
 *
 *  @return   Returns 0, always
 *
 *  @note  The minimum year a date time picker can be set to is 1601.  If the
 *         DateTime object represents a year prior to 1601, an exception is
 *         raised.
 *
 */
RexxMethod2(RexxObjectPtr, dtp_setDateTime, RexxObjectPtr, dateTime, CSELF, pCSelf)
{
    SYSTEMTIME sysTime = {0};
    HWND hwnd = getDChCtrl(pCSelf);

    if ( isShowNoneDTP(hwnd) && dateTime == TheNilObj )
    {
        DateTime_SetSystemtime(hwnd, GDT_NONE, &sysTime);
    }
    else
    {
        if ( requiredClass(context->threadContext, dateTime, "DATETIME", 1) )
        {
            if ( dt2sysTime(context->threadContext, dateTime, &sysTime, dtFull) )
            {
                if ( DateTime_SetSystemtime(hwnd, GDT_VALID, &sysTime) == 0 )
                {
                    controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG, "DateTime_SetSystemtime", DATETIMEPICKER_WINNAME);
                }
            }
        }
    }
    return TheZeroObj;
}


/** DateTimePicker::setFormat()
 *
 *  Sets the display of the date and time picker (DTP) control based on the
 *  given format string.
 *
 *  @param  format  The format string that the DTP should use.
 *
 *  @return  Returns true on success, otherwise false.
 *
 *  @note  It is acceptable to include extra characters within the format string
 *         to produce a more rich display. However, any nonformat characters
 *         must be enclosed within single quotes. For example, the format string
 *         "'Today is: 'hh':'m':'s ddddMMMdd', 'yyy" would produce output like
 *         "Today is: 04:22:31 Tuesday Mar 23, 1996".
 *
 *         A DTP control tracks locale changes when it is using the default
 *         format string. If you set a custom format string, it will not be
 *         updated in response to locale changes.
 *
 *         <for docs @see point reader to section on format strings>
 */
RexxMethod2(logical_t, dtp_setFormat, CSTRING, format, CSELF, pCSelf)
{
    return DateTime_SetFormat(getDChCtrl(pCSelf), format);
}


/** DateTimePicker::setMonthCalColor()
 *
 *  Sets the color for a given portion of the month calendar within a date and
 *  time picker (DTP) control.
 *
 *  @param  calPart  Keyword specifying which part of the month calendar to set
 *                   the color.
 *
 *  @param  color    The color, as a COLORREF, to use for the specified part of
 *                   the month calendar.
 *
 *  @return  The previous color for the part specified as a COLORREF, or
 *           CLR_INVALID on error.
 */
RexxMethod3(uint32_t, dtp_setMonthCalColor, CSTRING, calPart, uint32_t, color, CSELF, pCSelf)
{
    int32_t flag = calPart2flag(context, calPart, 1);
    if ( flag == -1 )
    {
        return CLR_INVALID;
    }
    return (COLORREF)DateTime_SetMonthCalColor(getDChCtrl(pCSelf), flag, color);
}


/** DateTimePicker::setMonthCalStyle()
 *
 *  Sets the month calendar style for the date and time picker.
 *
 *  @return  A string of keywords that consist of the style of the month
 *           calendar control used by the date time picker.
 *
 *  @note  See <LINK to DynamicDialog::createMonthCalendar> for the month
 *         calendar style keywords.
 *
 *         Vista or later only.
 */
RexxMethod2(RexxObjectPtr, dtp_setMonthCalStyle, CSTRING, newStyle, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "setMonthCalStyle", "Vista", Vista_OS) )
    {
        return TheZeroObj;
    }

    uint32_t style = (uint32_t)DateTime_SetMonthCalStyle(getDChCtrl(pCSelf), monthCalendarStyle(newStyle, 0));
    return mcStyle2String(context, style);
}


/** DateTimePicker::setRange()
 *
 *  Sets the minimum and maximum allowable dates / times for the date time
 *  picker control.
 *
 *  @param dateTimes  An array of DateTime objects used to set the minimum and
 *                    maximum dates.  The DateTime object at index 1 sets the
 *                    minimum date and the DateTime object at index 2 sets the
 *                    maximum date.
 *
 *  @return  True on success, otherwise false.
 *
 *  @note  The array must contain at least one of the indexes.  If it contains
 *         neither, an exception is raised. If one of the array indexes is
 *         empty, then the corresponding date is not set.
 *
 *         Exceptions are raised for invalid arguments.
 */
RexxMethod2(RexxObjectPtr, dtp_setRange, RexxArrayObject, dateTimes, CSELF, pCSelf)
{
    SYSTEMTIME sysTime[2];
    uint32_t which = 0;

    if ( dt2sysTimeRange(context, dateTimes, (SYSTEMTIME *)&sysTime, dtDate, false, &which) )
    {
        return  (DateTime_SetRange(getDChCtrl(pCSelf), which, &sysTime) == 0 ? TheFalseObj : TheTrueObj);
    }
    return TheFalseObj;
}


/**
 * Methods for the MonthCalendar class.
 *
 * Note that a MonthCalendar object can be created from a DateTimePicker when
 * the date time picker displays the month calendar.  The underlying month
 * calendar control is destroyed when the date time picker closes it up.  For
 * this reason, extra care is used in the MonthCalendar class to ensure the
 * window handle for the control is still valid.
 */
#define MONTHCALENDAR_CLASS    "MonthCalendar"
#define MONTHCALENDAR_WINNAME  "Month Calendar"

inline HWND getMonthCalendar(RexxMethodContext *c, void *pCSelf)
{
    HWND hMC = getDChCtrl(pCSelf);
    if ( hMC == NULL )
    {
        invalidWindowException(c, getDCrexxSelf(pCSelf));
    }
    return hMC;
}


/* Determine if a month calendar is a multi-selection month calendar. */
inline bool isMultiSelectionMonthCalendar(HWND hCtrl)
{
    return ((GetWindowLong(hCtrl, GWL_STYLE) & MCS_MULTISELECT) == MCS_MULTISELECT);
}

inline RexxObjectPtr setDayState(HWND hMC, LPMONTHDAYSTATE pmds, int count, RexxObjectPtr result)
{
    if ( result != NULLOBJECT )
    {
        result = (MonthCal_SetDayState(hMC, count, pmds) == 0 ? TheFalseObj : TheTrueObj);
    }
    return result;
}

/* Convert Month Calendar integer day to its string name. */
inline CSTRING day2dayName(int32_t iDay)
{
    switch ( iDay )
    {
        case 0 : return "Monday";
            break;
        case 1 : return "Tuesday";
            break;
        case 2 : return "Wednesday";
            break;
        case 3 : return "Thursday";
            break;
        case 4 : return "Friday";
            break;
        case 5 : return "Saturday";
            break;
        case 6 : return "Sunday";
            break;
        default : return  "";
            break;
    }
}

/*
 * Hit test flags for areas that are part of the calendar grid, and thus have
 * valid row and column info.
 */
inline bool isGridPart(LRESULT hit)
{
    if ( hit == MCHT_CALENDARDATE     || hit == MCHT_CALENDARDATENEXT ||
         hit == MCHT_CALENDARDATEPREV || hit == MCHT_CALENDARDATEMAX  ||
         hit == MCHT_CALENDARDATEMIN )
    {
        return true;
    }
    return false;
}


static int32_t dayName2day(CSTRING day)
{
    if (      StrCmpI(day, "MONDAY"   ) == 0 ) return 0;
    else if ( StrCmpI(day, "TUESDAY"  ) == 0 ) return 1;
    else if ( StrCmpI(day, "WEDNESDAY") == 0 ) return 2;
    else if ( StrCmpI(day, "THURSDAY" ) == 0 ) return 3;
    else if ( StrCmpI(day, "FRIDAY"   ) == 0 ) return 4;
    else if ( StrCmpI(day, "SATURDAY" ) == 0 ) return 5;
    else if ( StrCmpI(day, "SUNDAY"   ) == 0 ) return 6;
    else return -1;
}


/**
 * Change a month calendar's style.
 *
 * @param c
 * @param pCSelf
 * @param _style
 * @param _additionalStyle
 * @param remove
 *
 * @return uint32_t
 *
 *  @remarks  MSDN suggests setting last error to 0 before calling
 *            GetWindowLong() as the correct way to determine error.
 */
static RexxStringObject mcChangeStyle(RexxMethodContext *c, pCDialogControl pCSelf, CSTRING _style,
                                      CSTRING _additionalStyle, bool remove)
{
    oodResetSysErrCode(c->threadContext);
    SetLastError(0);

    HWND hMC = getMonthCalendar(c, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    uint32_t newStyle = 0;
    uint32_t oldStyle = (uint32_t)GetWindowLong(hMC, GWL_STYLE);

    if ( oldStyle == 0 && GetLastError() != 0 )
    {
        goto err_out;
    }

    if ( remove )
    {
        newStyle = oldStyle & ~monthCalendarStyle(_style, 0);

        if ( _additionalStyle != NULL )
        {
            newStyle = monthCalendarStyle(_additionalStyle, newStyle);
        }
    }
    else
    {
        newStyle = monthCalendarStyle(_style, oldStyle);
    }

    if ( SetWindowLong(hMC, GWL_STYLE, newStyle) == 0 && GetLastError() != 0 )
    {
        goto err_out;
    }
    return mcStyle2String(c, oldStyle);

err_out:
    oodSetSysErrCode(c->threadContext);
    return 0;
}


static void firstDay2directory(RexxMethodContext *c, uint32_t firstDay, RexxDirectoryObject *pDirectory)
{
    int32_t       iDay = LOWORD(firstDay);
    RexxObjectPtr usesLocale = HIWORD(firstDay) == 0 ? TheTrueObj : TheFalseObj;
    CSTRING       dayName = day2dayName(iDay);

    RexxDirectoryObject result = *pDirectory;

    c->DirectoryPut(result, c->Int32(iDay), "DAY");
    c->DirectoryPut(result, usesLocale, "USINGLOCALE");
    c->DirectoryPut(result, c->String(dayName), "DAYNAME");
}


bool putHitInfo(RexxMethodContext *c, RexxDirectoryObject hitInfo, MCHITTESTINFO *info)
{
    bool done = false;
    bool needDate = false;

    switch ( info->uHit )
    {
        case MCHT_CALENDARBK :
            c->DirectoryPut(hitInfo, c->String("CalendarBackground"), "HIT");
            break;

        case MCHT_CALENDARCONTROL :
            c->DirectoryPut(hitInfo, c->String("CalendarControl"), "HIT");
            break;

        case MCHT_CALENDARDATE :
            c->DirectoryPut(hitInfo, c->String("CalendarDate"), "HIT");
            needDate = true;
            break;

        case MCHT_CALENDARDATEMIN :
            c->DirectoryPut(hitInfo, c->String("CalendarDateMin"), "HIT");
            break;

        case MCHT_CALENDARDATEMAX :
            c->DirectoryPut(hitInfo, c->String("CalendarDateMax"), "HIT");
            break;

        case MCHT_CALENDARDATENEXT :
            c->DirectoryPut(hitInfo, c->String("CalendarDateNext"), "HIT");
            break;

        case MCHT_CALENDARDATEPREV :
            c->DirectoryPut(hitInfo, c->String("CalendarDatePrev"), "HIT");
            break;

        case MCHT_CALENDARDAY :
            c->DirectoryPut(hitInfo, c->String("CalendarDay"), "HIT");
            needDate = true;
            break;

        case MCHT_CALENDARWEEKNUM :
            c->DirectoryPut(hitInfo, c->String("CalendarWeekNum"), "HIT");
            needDate = true;
            break;

        case MCHT_NOWHERE :
            c->DirectoryPut(hitInfo, c->String("NoWhere"), "HIT");
            done = true;
            break;

        case MCHT_TITLEBK :
            c->DirectoryPut(hitInfo, c->String("TitleBackground"), "HIT");
            break;

        case MCHT_TITLEBTNNEXT :
            c->DirectoryPut(hitInfo, c->String("TitleButtonNext"), "HIT");
            break;

        case MCHT_TITLEBTNPREV :
            c->DirectoryPut(hitInfo, c->String("TitleButtonPrev"), "HIT");
            break;

        case MCHT_TITLEMONTH :
            c->DirectoryPut(hitInfo, c->String("TitleMonth"), "HIT");
            break;

        case MCHT_TITLEYEAR :
            c->DirectoryPut(hitInfo, c->String("TitleYear"), "HIT");
            break;

        case MCHT_TODAYLINK :
            c->DirectoryPut(hitInfo, c->String("TodayLink"), "HIT");
            break;

        default :
            // Shouldn't happen, but if it does this is okay, the HIT index is
            // already the empty string and we are done.
            done = true;
            break;
    }

    if ( needDate )
    {
        RexxObjectPtr date;

        sysTime2dt(c->threadContext, &(info->st), &date, dtDate);
        c->DirectoryPut(hitInfo, date, "DATE");
    }
    return done;
}


/** MonthCalendar::date   [Attribute Get]
 *
 *  Returns the currently selected date for the month calendar.
 *
 *  @note  This attribute was not originally meant for month calendars with the
 *         multi-selection style.  When used for multi-selection, the value will
 *         be the first selected date in the selection range for the calendar.
 *
 *         Use the getSelectionRange() method for multi-selection calendars.
 */
RexxMethod1(RexxObjectPtr, get_mc_date, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr dateTime = NULLOBJECT;

    SYSTEMTIME sysTime[2];
    LRESULT result = 0;
    memset(&sysTime, 0, 2 * sizeof(SYSTEMTIME));

    if ( isMultiSelectionMonthCalendar(hMC) )
    {
        result = MonthCal_GetSelRange(hMC, &sysTime);
    }
    else
    {
        result = MonthCal_GetCurSel(hMC, &sysTime);
    }

    if ( result == 0 )
    {
        controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG, "MonthCal_GetCurSel", MONTHCALENDAR_WINNAME);
    }
    else
    {
        sysTime2dt(context->threadContext, (SYSTEMTIME *)&sysTime, &dateTime, dtDate);
    }
    return dateTime;
}

/** MonthCalendar::date=  [Attribute Set]
 *
 *  Sets the currently selected date for the month calendar.
 *
 *  @param dateTime  A DateTime object used to set the selected date.  The time
 *                   portion of the object is ignored.
 *
 *  @note  This attribute was not originally meant for month calendars with the
 *         multi-selection style.  When used for multi-selection, the first and
 *         last date of the selection is set to the same date.
 *
 *         Use the setSelectionRange() method for multi-selection calendars to
 *         set a full range.
 */
RexxMethod2(RexxObjectPtr, set_mc_date, RexxObjectPtr, dateTime, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    SYSTEMTIME sysTime[2];
    LRESULT result = FALSE;
    memset(&sysTime, 0, 2 * sizeof(SYSTEMTIME));

    if ( requiredClass(context->threadContext, dateTime, "DATETIME", 1) )
    {
        if ( dt2sysTime(context->threadContext, dateTime, (SYSTEMTIME *)&sysTime, dtDate) )
        {
            if ( isMultiSelectionMonthCalendar(hMC) )
            {
                sysTime[1].wDay   = sysTime[0].wDay;
                sysTime[1].wMonth = sysTime[0].wMonth;
                sysTime[1].wYear  = sysTime[0].wYear;

                result = MonthCal_SetSelRange(hMC, &sysTime);
            }
            else
            {
                result = MonthCal_SetCurSel(hMC, &sysTime);
            }

            if ( result == 0 )
            {
                controlFailedException(context->threadContext, FUNC_WINCTRL_FAILED_MSG,
                                       "MonthCal_SetCurSel", MONTHCALENDAR_WINNAME);
            }
        }
    }
    return NULLOBJECT;
}


/** MonthCalendar::addStyle()
 *  MonthCalendar::removeStyle()
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxStringObject, mc_addRemoveStyle, CSTRING, style, NAME, method, CSELF, pCSelf)
{
    return mcChangeStyle(context, (pCDialogControl)pCSelf, style, NULL, (*method == 'R'));
}

/** MonthCalendar::replaceStyle()
 *
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxStringObject, mc_replaceStyle, CSTRING, removeStyle, CSTRING, additionalStyle, CSELF, pCSelf)
{
    return mcChangeStyle(context, (pCDialogControl)pCSelf, removeStyle, additionalStyle, true);
}


/** MonthCalendar::getStyle()
 *
 */
RexxMethod1(RexxStringObject, mc_getStyle, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    uint32_t _style = GetWindowLong(hMC, GWL_STYLE);
    return mcStyle2String(context, _style);
}


RexxMethod1(uint32_t, mc_getCalendarBorder, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getBorder", "Vista", Vista_OS) )
    {
        return 0;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    return MonthCal_GetCalendarBorder(hMC);
}


RexxMethod1(uint32_t, mc_getCalendarCount, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getCount", "Vista", Vista_OS) )
    {
        return 0;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    return MonthCal_GetCalendarCount(hMC);
}


RexxMethod1(RexxObjectPtr, mc_getCALID, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getCALID", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    CSTRING id = "";
    switch( MonthCal_GetCALID(hMC) )
    {
        case CAL_GREGORIAN              : id = "GREGORIAN"; break;
        case CAL_GREGORIAN_US           : id = "GREGORIAN_US"; break;
        case CAL_JAPAN                  : id = "JAPAN"; break;
        case CAL_TAIWAN                 : id = "TAIWAN"; break;
        case CAL_KOREA                  : id = "KOREA"; break;
        case CAL_HIJRI                  : id = "HIJRI"; break;
        case CAL_THAI                   : id = "THAI"; break;
        case CAL_HEBREW                 : id = "HEBREW"; break;
        case CAL_GREGORIAN_ME_FRENCH    : id = "GREGORIAN_ME_FRENCH"; break;
        case CAL_GREGORIAN_ARABIC       : id = "GREGORIAN_ARABIC"; break;
        case CAL_GREGORIAN_XLIT_ENGLISH : id = "CAL_GREGORIAN_XLIT_ENGLISH"; break;
        case CAL_GREGORIAN_XLIT_FRENCH  : id = "CAL_GREGORIAN_XLIT_FRENCH"; break;
        case CAL_UMALQURA               : id = "UMALQURA"; break;
    }

    return context->String(id);
}


/** MonthCalendar::getColor()
 *
 *  Retrieves the color for a given portion of a month calendar control.
 *
 *  @param  calPart  Specifies which part of the calendar to get the color for.
 *
 *  @return  The color for the portion of the month calendar specified, or
 *           CLR_INVALID on error.
 *
 *  @notes  You can use .Image~colorRef(CLR_INVALID) to test for error.  (An
 *          error is not very likely.)  I.e.:
 *
 *          color = monthCalendar~getColor("TRAILINGTEXT")
 *          if color == .Image~colorRef(CLR_INVALID) then do
 *            -- some error routine
 *          end
 *
 */
RexxMethod2(uint32_t, mc_getColor, CSTRING, calPart, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    int32_t flag = calPart2flag(context, calPart, 1);
    if ( flag == -1 )
    {
        return CLR_INVALID;
    }
    return (COLORREF)MonthCal_GetColor(hMC, flag);
}

RexxMethod1(RexxObjectPtr, mc_getCurrentView, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getCurrentView", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    CSTRING id = "";
    switch( MonthCal_GetCurrentView(hMC) )
    {
        case MCMV_MONTH   : id = "Monthly"; break;
        case MCMV_YEAR    : id = "Annual"; break;
        case MCMV_DECADE  : id = "Decade"; break;
        case MCMV_CENTURY : id = "Century"; break;
    }

    return context->String(id);
}


/** MonthCalendar::getFirstDayOfWeek()
 *
 *  Retrieves the first day of the week for the month calendar control.
 *
 *  @param  info  [OPTIONAL]  A directory object which will be filled with more
 *                detailed information than just the first day.
 *
 *  @return  A number specifying the first day of the week.  0 for Monday, 1 for
 *           Tuesday, etc..
 *
 *  @note  If the optional directory object is passed to the method, on return
 *         it will contain these indexes:
 *
 *         day          The number specifying the first day of the week.  This
 *                      is the same number as the return.
 *
 *         usingLocale  True or false, specifying whether if the first day of
 *                      the week is set to the LOCALE_IFIRSTDAYOFWEEK.
 *
 *         dayName      The string name of the day, Monday, Tuesday, etc..
 */
RexxMethod2(int32_t, mc_getFirstDayOfWeek, OPTIONAL_RexxObjectPtr, result, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    uint32_t ret = MonthCal_GetFirstDayOfWeek(hMC);

    if ( argumentExists(1) )
    {
        if ( ! context->IsOfType(result, "DIRECTORY") )
        {
            wrongClassException(context->threadContext, 1, "Directory");
        }
        else
        {
            firstDay2directory(context, ret, (RexxDirectoryObject *)&result);
        }
    }
    return LOWORD(ret);
}


/** MonthCalendar::getGridInfo()
 *
 *
 *  @note  Indexes for row, column, and calendar offset are 1-based.
 *
 */
RexxMethod2(RexxObjectPtr, mc_getGridInfo, RexxObjectPtr, _gridInfo, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "getGridInfo", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    MCGRIDINFO info = {0};
    info.cbSize = sizeof(MCGRIDINFO);

    if ( ! context->IsOfType(_gridInfo, "DIRECTORY") )
    {
        wrongClassException(context->threadContext, 1, "Directory");
        goto err_out;
    }
    RexxDirectoryObject gridInfo = (RexxDirectoryObject)_gridInfo;
    int32_t num;

    RexxObjectPtr _part = context->DirectoryAt(gridInfo, "PART");
    RexxObjectPtr _what = context->DirectoryAt(gridInfo, "WHAT");
    RexxObjectPtr _calIndex = context->DirectoryAt(gridInfo, "INDEX");

    if ( _part == NULLOBJECT )
    {
        missingIndexInDirectoryException(context->threadContext, 1, "PART");
        goto err_out;
    }
    if ( _what == NULLOBJECT )
    {
        missingIndexInDirectoryException(context->threadContext, 1, "WHAT");
        goto err_out;
    }

    if ( _calIndex == NULLOBJECT )
    {
        info.iCalendar = 0;
        context->DirectoryPut(gridInfo, TheOneObj, "INDEX");
    }
    else
    {
        if ( ! context->Int32(_calIndex, &num) )
        {
            wrongObjInDirectoryException(context->threadContext, 1, "INDEX", "whole number", _calIndex);
            goto err_out;
        }
        info.iCalendar = num - 1;
    }

    CSTRING partName = context->ObjectToStringValue(_part);
    CSTRING whatFlag = context->ObjectToStringValue(_what);

    if ( stricmp(partName,      "CONTROL")  == 0 ) info.dwPart = MCGIP_CALENDARCONTROL;
    else if ( stricmp(partName, "NEXT")     == 0 ) info.dwPart = MCGIP_NEXT;
    else if ( stricmp(partName, "PREV")     == 0 ) info.dwPart = MCGIP_PREV;
    else if ( stricmp(partName, "FOOTER")   == 0 ) info.dwPart = MCGIP_FOOTER;
    else if ( stricmp(partName, "CALENDAR") == 0 ) info.dwPart = MCGIP_CALENDAR;
    else if ( stricmp(partName, "HEADER")   == 0 ) info.dwPart = MCGIP_CALENDARHEADER;
    else if ( stricmp(partName, "BODY")     == 0 ) info.dwPart = MCGIP_CALENDARBODY;
    else if ( stricmp(partName, "ROW")      == 0 ) info.dwPart = MCGIP_CALENDARROW;
    else if ( stricmp(partName, "CELL")     == 0 ) info.dwPart = MCGIP_CALENDARCELL;
    else
    {
        directoryIndexExceptionList(context->threadContext, 1, "PART", MC_GRIDINFO_PART_NAMES, _part);
        goto err_out;
    }

    if ( StrStrI(whatFlag,      "DATE") != NULL ) info.dwFlags |= MCGIF_DATE;
    else if ( stricmp(whatFlag, "RECT") != NULL ) info.dwFlags |= MCGIF_RECT;
    else if ( stricmp(whatFlag, "NAME") != NULL ) info.dwFlags |= MCGIF_NAME;

    if ( info.dwFlags == 0 )
    {
        directoryIndexExceptionMsg(context->threadContext, 1, "WHAT", MC_GRIDINFO_WHAT_FLAG_ERR_MSG, _what);
        goto err_out;
    }

    if ( info.dwPart == MCGIP_CALENDARROW || info.dwPart == MCGIP_CALENDARCELL )
    {
        RexxObjectPtr row = context->DirectoryAt(gridInfo, "ROW");
        if ( row == NULLOBJECT )
        {
            missingIndexInDirectoryException(context->threadContext, 1, "ROW");
            goto err_out;
        }
        if ( ! context->Int32(row, &num) )
        {
            wrongObjInDirectoryException(context->threadContext, 1, "ROW", "whole number", row);
            goto err_out;
        }
        info.iRow = num - 1;

        if ( info.dwPart == MCGIP_CALENDARCELL )
        {
            RexxObjectPtr col = context->DirectoryAt(gridInfo, "COL");
            if ( col == NULLOBJECT )
            {
                missingIndexInDirectoryException(context->threadContext, 1, "COL");
                goto err_out;
            }
            if ( ! context->Int32(col, &num) )
            {
                wrongObjInDirectoryException(context->threadContext, 1, "COL", "whole number", col);
                goto err_out;
            }
            info.iCol = num - 1;
        }
    }

    if ( ! MonthCal_GetCalendarGridInfo(hMC, &info) )
    {
        goto err_out;
    }

    if ( info.dwPart == MCGIP_CALENDARCELL )
    {
        context->DirectoryPut(gridInfo, (info.bSelected ? TheTrueObj : TheFalseObj), "SELECTED");
    }

    if ( info.dwFlags & MCGIF_DATE )
    {
        RexxObjectPtr startDate;
        RexxObjectPtr endDate;
        sysTime2dt(context->threadContext, &info.stStart, &startDate, dtDate);
        sysTime2dt(context->threadContext, &info.stEnd, &endDate, dtDate);

        context->DirectoryPut(gridInfo, startDate, "STARTDATE");
        context->DirectoryPut(gridInfo, endDate, "ENDDATE");
    }

    if ( info.dwFlags & MCGIF_RECT )
    {
        context->DirectoryPut(gridInfo, rxNewRect(context, (PORXRECT)&info.rc), "RECT");
    }

    if ( (info.dwFlags & MCGIF_NAME) && (info.dwPart == MCGIP_CALENDAR || info.dwPart == MCGIP_CALENDARCELL || info.dwPart == MCGIP_CALENDARHEADER) )
    {
        context->DirectoryPut(gridInfo, unicode2string(context, info.pszName), "NAME");
    }

    return TheTrueObj;

err_out:
    return TheFalseObj;
}


RexxMethod2(RexxObjectPtr, mc_getMinRect, RexxObjectPtr, _rect, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    PRECT r = (PRECT)rxGetRect(context, _rect, 1);
    if ( r != NULL )
    {
        return (MonthCal_GetMinReqRect(hMC, r) == 0 ? TheFalseObj : TheTrueObj);
    }
    return TheFalseObj;
}


/** MonthCalendar::getMonthRange()
 *
 *  Retrieves date information (using DateTime objects) that represents the
 *  high and low limits of a month calendar control's display.
 *
 *  @param  range  [IN / OUT] An array object in which the range is returned.
 *                 The lower limit (a DateTime object) will be returned at index
 *                 1 and the upper limit (a DateTime object) will be returned at
 *                 index 2.
 *
 *  @param  span   [OPTIONAL]  A keyword specifying whether the range should
 *                 include only months that are ENTIRELY displayed or to include
 *                 trailing and following months that are only PARTIALLY
 *                 displayed.  The default if omitted is PARTIALLY.  Only the
 *                 first letter of ENTIRELY or PARTIALLY are required and case
 *                 is insignificant.
 *
 *  @return  The number of months in the range.
 */
RexxMethod3(int32_t, mc_getMonthRange, RexxArrayObject, range, OPTIONAL_CSTRING, span, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    uint32_t flag = GMR_DAYSTATE;
    if ( argumentExists(2) )
    {
        switch ( toupper(*span) )
        {
            case 'E' : flag = GMR_VISIBLE;
                break;
            case 'P' : flag = GMR_DAYSTATE;
                break;
            default :
                goto err_out;
        }
    }

    SYSTEMTIME sysTime[2];
    memset(&sysTime, 0, 2 * sizeof(SYSTEMTIME));

    int32_t ret = MonthCal_GetMonthRange(hMC, flag, &sysTime);

    RexxObjectPtr lowMonth, highMonth;
    sysTime2dt(context->threadContext, (SYSTEMTIME *)&sysTime, &lowMonth, dtDate);
    sysTime2dt(context->threadContext, (SYSTEMTIME *)&sysTime + 1, &highMonth, dtDate);

    context->ArrayPut(range, lowMonth, 1);
    context->ArrayPut(range, highMonth, 2);

    return ret;

err_out:
    wrongArgOptionException(context->threadContext, 2, "[P]artially, or [E]ntirely", span);
    return -1;
}


/** MonthCalendar::getRange()
 *
 *  Gets the current minimum and maximum allowable dates for a month calendar
 *  control.
 *
 *  @param  range  [IN/OUT] An array in which the minimum and maximum dates are
 *                 returned as .DateTime objects.  The minimum date will be at
 *                 index 1 and the maximum at index 2.  If either index is set
 *                 to zero, then no corresponding limit is set for the month
 *                 calendar control.
 *
 *  @return  A keyword indicating the result.  See below for the possible
 *           keywords.
 *
 *  @note  The returned keyword indicates whether a minimum or maximum limit has
 *         been set for the month calendar control.  The keyword will be one of:
 *         none, min, max, both, or possibly error.  (An error is unlikely.)
 */
RexxMethod2(CSTRING, mc_getRange, RexxArrayObject, range, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return "";
    }
    return getTimeRange(context, range, hMC, winMonthCalendar);

}

RexxMethod2(RexxObjectPtr, mc_getSelectionRange, RexxArrayObject, range, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    if ( ! isMultiSelectionMonthCalendar(hMC) )
    {
        goto err_out;
    }

    SYSTEMTIME sysTime[2];
    memset(&sysTime, 0, 2 * sizeof(SYSTEMTIME));

    if ( MonthCal_GetSelRange(hMC, &sysTime) == 0 )
    {
        goto err_out;
    }

    RexxObjectPtr startDate;
    sysTime2dt(context->threadContext, (SYSTEMTIME *)&sysTime, &startDate, dtDate);
    context->ArrayPut(range, startDate, 1);

    RexxObjectPtr endDate;
    sysTime2dt(context->threadContext, (SYSTEMTIME *)&sysTime + 1, &endDate, dtDate);
    context->ArrayPut(range, endDate, 2);

    return TheTrueObj;

err_out:
    return TheFalseObj;
}


/** MonthCalendar::getToday()
 *
 *  Retrieves the date information for the date specified as "today" for a month
 *  calendar control.
 *
 *  @return  The "today" date as a DateTime object or .nil on error.
 */
RexxMethod1(RexxObjectPtr, mc_getToday, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr result = TheNilObj;
    SYSTEMTIME sysTime = {0};

    if ( MonthCal_GetToday(hMC, &sysTime) != 0 )
    {
        sysTime2dt(context->threadContext, &sysTime, &result, dtDate);
    }
    return result;
}

/** MonthCalendar::hitTestInfo()
 *
 *
 *  @note  Indexes for row, column, and calendar offset are 1-based.
 *
 *  @remarks  If we are not on Vista or later, we need to be sure to set cbSize
 *            for MCHITTESTINFO to exclude the Vista only fields.  Otherwise
 *            MonthCal_HitTest fails completely.
 *
 *            Erroneously called hitTest in ooDialog 4.2.0.  Now both hitTest
 *            and hitTestInfo are mapped to this method.  Need to preserve
 *            hitTest for backwards compatibility, but only hitTestInfo is
 *            documented from 4.2.2 onwards.
 */
RexxMethod2(RexxObjectPtr, mc_hitTestInfo, RexxObjectPtr, _pt, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    RexxDirectoryObject hitInfo = context->NewDirectory();

    MCHITTESTINFO info = {0};
    if ( ComCtl32Version <=  COMCTL32_6_0 )
    {
        info.cbSize = MCHITTESTINFO_V1_SIZE;
    }
    else
    {
        info.cbSize = sizeof(MCHITTESTINFO);
    }

    PPOINT pt = (PPOINT)rxGetPoint(context, _pt, 1);
    if ( pt == NULL )
    {
        goto done_out;
    }
    context->DirectoryPut(hitInfo, _pt, "POINT");

    info.pt.x = pt->x;
    info.pt.y = pt->y;

    LRESULT hit = MonthCal_HitTest(hMC, &info);

    bool done = putHitInfo(context, hitInfo, &info);

    if ( info.cbSize > MCHITTESTINFO_V1_SIZE && ! done )
    {
        context->DirectoryPut(hitInfo, rxNewRect(context, (PORXRECT)&info.rc), "RECT");

        context->DirectoryPut(hitInfo, context->WholeNumber(info.iOffset + 1), "OFFSET");

        if ( isGridPart(info.uHit)  )
        {
            context->DirectoryPut(hitInfo, context->WholeNumber(info.iRow + 1), "ROW");
            context->DirectoryPut(hitInfo, context->WholeNumber(info.iCol + 1), "COLUMN");
        }
        else if ( info.uHit == MCHT_CALENDARDAY)
        {
            context->DirectoryPut(hitInfo, context->WholeNumber(info.iCol + 1), "COLUMN");
        }
        else if ( info.uHit == MCHT_CALENDARWEEKNUM )
        {
            context->DirectoryPut(hitInfo, context->WholeNumber(info.iRow + 1), "ROW");
        }
    }

done_out:
    return hitInfo;
}


RexxMethod2(RexxObjectPtr, mc_setCalendarBorder, OPTIONAL_uint32_t, border, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "setBorder", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    if ( argumentExists(1) )
    {
        MonthCal_SetCalendarBorder(hMC, TRUE, border);
    }
    else
    {
        MonthCal_SetCalendarBorder(hMC, FALSE, 0);
    }
    return TheZeroObj;
}


/** MonthCalendar::setCALID()
 *
 *  Sets the calendar ID for the month calendar control.
 *
 * @param id  Keyword specifying which calendar ID to use.
 *
 * @return  0, always.
 */
RexxMethod2(RexxObjectPtr, mc_setCALID, CSTRING, id, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "setCALID", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    uint32_t calID = CAL_GREGORIAN;

    if (      stricmp(id, "GREGORIAN"                 ) == 0 ) calID = CAL_GREGORIAN;
    else if ( stricmp(id, "GREGORIAN_US"              ) == 0 ) calID = CAL_GREGORIAN_US;
    else if ( stricmp(id, "JAPAN"                     ) == 0 ) calID = CAL_JAPAN;
    else if ( stricmp(id, "TAIWAN"                    ) == 0 ) calID = CAL_TAIWAN;
    else if ( stricmp(id, "KOREA"                     ) == 0 ) calID = CAL_KOREA;
    else if ( stricmp(id, "HIJRI"                     ) == 0 ) calID = CAL_HIJRI;
    else if ( stricmp(id, "THAI"                      ) == 0 ) calID = CAL_THAI;
    else if ( stricmp(id, "HEBREW"                    ) == 0 ) calID = CAL_HEBREW;
    else if ( stricmp(id, "GREGORIAN_ME_FRENCH"       ) == 0 ) calID = CAL_GREGORIAN_ME_FRENCH;
    else if ( stricmp(id, "GREGORIAN_ARABIC"          ) == 0 ) calID = CAL_GREGORIAN_ARABIC;
    else if ( stricmp(id, "CAL_GREGORIAN_XLIT_ENGLISH") == 0 ) calID = CAL_GREGORIAN_XLIT_ENGLISH;
    else if ( stricmp(id, "CAL_GREGORIAN_XLIT_FRENCH" ) == 0 ) calID = CAL_GREGORIAN_XLIT_FRENCH;
    else if ( stricmp(id, "UMALQURA"                  ) == 0 ) calID = CAL_UMALQURA;

    MonthCal_SetCALID(hMC, calID);

    return TheZeroObj;
}


/** MonthCalendar::setColor()
 *
 *  Sets the color for a given part of a month calendar control.
 *
 *  @param  calPart  Specifies which portion will have its color set.
 *  @param  color    A COLORREF specifying the color for the calendar part.
 *
 *  @return  The previous color for the part of the month calendar specified,
 *           or CLR_INVALID on error.
 *
 *  @notes  You can use .Image~colorRef(CLR_INVALID) to test for error.  (An
 *          error is not very likely.)  I.e.:
 *
 *          oldColor = monthCalendar~setColor("TRAILINGTEXT", color)
 *          if oldColor == .Image~colorRef(CLR_INVALID) then do
 *            -- some error routine
 *          end
 *
 *          A syntax condition is raised if calPart is not valid.
 *
 *          setColor() only seems to work if the theme is Windows Classic.
 *
 */
RexxMethod3(uint32_t, mc_setColor, CSTRING, calPart, uint32_t, color, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return 0;
    }

    int32_t flag = calPart2flag(context, calPart, 1);
    if ( flag == -1 )
    {
        return CLR_INVALID;
    }
    return (COLORREF)MonthCal_SetColor(hMC, flag, color);
}

RexxMethod2(RexxObjectPtr, mc_setCurrentView, CSTRING, view, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "setCurrentView", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    uint32_t mcmv = MCMV_MONTH;

    if (      stricmp(view, "MONTHLY") == 0 ) mcmv = MCMV_MONTH;
    else if ( stricmp(view, "ANNUAL")  == 0 ) mcmv = MCMV_YEAR;
    else if ( stricmp(view, "DECADE")  == 0 ) mcmv = MCMV_DECADE;
    else if ( stricmp(view, "CENTURY") == 0 ) mcmv = MCMV_CENTURY;
    else
    {
        return wrongArgValueException(context->threadContext, 1, MC_VIEW_NAMES, view);
    }

    return (MonthCal_SetCurrentView(hMC, mcmv) ? TheTrueObj : TheFalseObj);
}

RexxMethod2(RexxObjectPtr, mc_setDayState, RexxArrayObject, list, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    LPMONTHDAYSTATE pmds;
    size_t count = context->ArrayItems(list);

    RexxObjectPtr result = makeDayStateBuffer(context, list, count, &pmds);
    return setDayState(hMC, pmds, (int)count, result);
}

RexxMethod4(RexxObjectPtr, mc_setDayStateQuick, RexxObjectPtr, _ds1, RexxObjectPtr, _ds2, RexxObjectPtr, _ds3, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    LPMONTHDAYSTATE pmds;
    RexxObjectPtr result = makeQuickDayStateBuffer(context, _ds1, _ds2, _ds3, &pmds);
    return setDayState(hMC, pmds, 3, result);
}

/** MonthCalendar::setFirstDayOfWeek()
 *
 *  Sets the first day of the week for the month calendar control.
 *
 *  @param  firstDay  Which day is to be the first day of the week.  This can
 *                    either be the name of the day (Monday, Tuesday, etc., case
 *                    insignificant) or the number of the day (0 for Monday, 1
 *                    for Tuesday, etc..)
 *
 *  @return  A directory object with information concerning the previous first
 *           day of the week. @see <getfirstDayOfWeek>
 *
 *  @note  The returned directory object will contain these indexes with the
 *         information for the previous first day of the week:
 *
 *         day          The number specifying the first day of the week. 0 for
 *                      Monday, etc..
 *
 *         usingLocale  True or false, specifying if the first day of the week
 *                      is set to the LOCALE_IFIRSTDAYOFWEEK.
 *
 *         dayName      The string name of the day, Monday, Tuesday, etc..
 */
RexxMethod2(RexxObjectPtr, mc_setFirstDayOfWeek, RexxObjectPtr, firstDay, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    int32_t iDay = -1;
    if ( ! context->Int32(firstDay, &iDay) )
    {
        iDay = dayName2day(context->ObjectToStringValue(firstDay));
    }

    if ( iDay < 0 || iDay > 6 )
    {
        return wrongArgValueException(context->threadContext, 1, "name of the day or a number from 0 to 6", firstDay);
    }

    uint32_t ret = (uint32_t)MonthCal_SetFirstDayOfWeek(hMC, iDay);

    RexxDirectoryObject result = context->NewDirectory();
    firstDay2directory(context, ret, &result);

    return result;
}

/** MonthCalendar::setRange()
 *
 *  Sets the minimum and maximum allowable dates for the month calendar control.
 *
 *  @param dateTimes  An array of DateTime objects used to set the minimum and
 *                    maximum dates.  The DateTime object at index 1 sets the
 *                    minimum date and the DateTime object at index 2 sets the
 *                    maximum date.
 *
 *  @return  True on success, otherwise false.
 *
 *  @note  The array must contain at least one of the indexes.  If it contains
 *         neither, and exceptions is raised. If one of the array indexes is
 *         empty, then the corresponding date is not set.  The time portion of
 *         the DateTime object(s) is ignored.
 *
 *         Exceptions are raised for invalid arguments.
 */
RexxMethod2(RexxObjectPtr, mc_setRange, RexxArrayObject, dateTimes, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC != NULL )
    {
        SYSTEMTIME sysTime[2];
        uint32_t which = 0;

        if ( dt2sysTimeRange(context, dateTimes, (SYSTEMTIME *)&sysTime, dtDate, false, &which) )
        {
            return  (MonthCal_SetRange(hMC, which, &sysTime) == 0 ? TheFalseObj : TheTrueObj);
        }
    }
    return TheFalseObj;
}


/** MonthCalendar::setSelectionRange()
 *
 *  Sets the selection for a month calendar control to a given date range.
 *
 *  @param dateTimes  An array of DateTime objects used to set the minimum and
 *                    maximum dates.  The DateTime oject at index 1 must be the
 *                    first date in the selection, and the DateTime object at
 *                    index 2 must be the last date in the selection.  Both
 *                    indexes are required.
 *
 *  @return  True on success, otherwise false.
 *
 *  @note  The time portion of the DateTime object(s) is ignored.  Exceptions
 *         are raised for invalid arguments.
 *
 *         This method will fail if applied to a month calendar control that
 *         does not have the MULTI (MCS_MULTISELECT) style.
 */
RexxMethod2(RexxObjectPtr, mc_setSelectionRange, RexxArrayObject, dateTimes, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC != NULL )
    {
        SYSTEMTIME sysTime[2];

        if ( dt2sysTimeRange(context, dateTimes, (SYSTEMTIME *)&sysTime, dtDate, true, NULL) )
        {
            return  (MonthCal_SetSelRange(hMC, &sysTime) == 0 ? TheFalseObj : TheTrueObj);
        }
    }
    return TheFalseObj;
}


/** MonthCalendar::setToday()
 *
 *  Sets the "today" selection for a month calendar control.
 *
 *  @param date  [OPTIONAL]  A DateTime object specifying the "today" date.  If
 *               this argument is omitted, then  the control returns to the
 *               default setting.
 *
 *  @return  0 always.  The return has no meaning.
 */
RexxMethod2(RexxObjectPtr, mc_setToday, OPTIONAL_RexxObjectPtr, date, CSELF, pCSelf)
{
    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    SYSTEMTIME sysTime = {0};
    SYSTEMTIME *pSysTime = NULL;

    if ( argumentExists(1) )
    {
        if ( ! context->IsOfType(date, "DATETIME") )
        {
            wrongClassException(context->threadContext, 1, "DateTime");
            goto done_out;
        }
        dt2sysTime(context->threadContext, date, &sysTime, dtDate);
    }

    MonthCal_SetToday(hMC, &sysTime);

done_out:
    return TheZeroObj;
}


/** MonthCalendar::sizeRectToMin()
 *
 *  Calculates how many calendars will fit in the given rectangle, and then
 *  returns the minimum size that a rectangle needs to be to fit that number of
 *  calendars.
 *
 *  @parm  _rect  [IN / OUT]  On entry, contains a .Rect object that describes a
 *         region that is greater than or equal to the size necessary to fit the
 *         desired number of calendars. When this method returns, the .Rect
 *         object will contains the minimum size needed for this number of
 *         calendars.
 *
 *  @return  0, always.  The return has no meaning.
 *
 */
RexxMethod2(RexxObjectPtr, mc_sizeRectToMin, RexxObjectPtr, _rect, CSELF, pCSelf)
{
    if ( ! requiredOS(context, "sizeRectToMin", "Vista", Vista_OS) )
    {
        goto done_out;
    }

    HWND hMC = getMonthCalendar(context, pCSelf);
    if ( hMC == NULL )
    {
        return NULLOBJECT;
    }

    PRECT r = (PRECT)rxGetRect(context, _rect, 1);
    if ( r != NULL )
    {
        MonthCal_SizeRectToMin(hMC, r);
    }

done_out:
    return TheZeroObj;
}


/**
 *  Methods for the .Tab class.
 */
#define TAB_CLASS                 "Tab"

#define TABIMAGELIST_ATTRIBUTE    "TAB!IMAGELIST"


/** Tab::setItemSize()
 *
 *  Sets the width and height of the tabs.
 *
 *  @param  size  The new size (cx, cy), in pixels.  The amount can be specified
 *                in these formats:
 *
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *  @return  The previous size of the tabs, as a .Size object
 *
 *  @note  You can use a .Point object instead of a .Size object to specify the
 *         new size, although semantically that is incorrect.
 */
RexxMethod2(RexxObjectPtr, tab_setItemSize, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    uint32_t oldSize = TabCtrl_SetItemSize(hwnd, point.x, point.y);
    return rxNewSize(context, LOWORD(oldSize), HIWORD(oldSize));
}


/** Tab::setPadding()
 *
 *  Sets the amount of space (padding) around each tab's icon and label.
 *
 *  @param  size  The padding size (cx, cy), in pixels.  The amount can be
 *                specified in these formats:
 *
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *  @return  0, always.
 *
 *  @note  You can use a .Point object instead of a .Size object to specify the
 *         new size, although semantically that is incorrect.
 */
RexxMethod2(RexxObjectPtr, tab_setPadding, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    TabCtrl_SetPadding(hwnd, point.x, point.y);
    return TheZeroObj;
}


RexxMethod5(int32_t, tab_insert, OPTIONAL_int32_t, index, OPTIONAL_CSTRING, label, OPTIONAL_int32_t, imageIndex,
            OPTIONAL_RexxObjectPtr, userData, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    if ( argumentOmitted(1) )
    {
        index = ((pCDialogControl)pCSelf)->lastItem;
    }

    TCITEM ti = {0};
    index++;

    ti.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
    ti.pszText = (argumentOmitted(2) ? "" : label);
    ti.iImage  = (argumentOmitted(3) ? -1 : imageIndex);
    ti.lParam  = (LPARAM)(argumentOmitted(4) ? TheZeroObj : userData);

    int32_t ret = TabCtrl_InsertItem(hwnd, index, &ti);
    if ( ret != -1 )
    {
        ((pCDialogControl)pCSelf)->lastItem = ret;
    }
    return ret;
}


RexxMethod5(int32_t, tab_modify, int32_t, index, OPTIONAL_CSTRING, label, OPTIONAL_int32_t, imageIndex,
            OPTIONAL_RexxObjectPtr, userData, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    TCITEM ti = {0};

    if ( argumentExists(2) )
    {
        ti.mask |= TCIF_TEXT;
        ti.pszText = (LPSTR)label;
    }
    if ( argumentExists(3) )
    {
        ti.mask |= TCIF_IMAGE;
        ti.iImage = imageIndex;
    }
    if ( argumentExists(4) )
    {
        ti.mask |= TCIF_PARAM;
        ti.lParam = (LPARAM)userData;
    }

    if ( ti.mask == 0 )
    {
        return 1;
    }

    return (TabCtrl_SetItem(hwnd, index, &ti) ? 0 : 1);
}


RexxMethod2(int32_t, tab_addSequence, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    TCITEM ti = {0};
    ti.mask = TCIF_TEXT;

    int32_t ret = -1;
    int32_t index = ((pCDialogControl)pCSelf)->lastItem;
    size_t count = context->ArraySize(args);

    for ( size_t i = 1; i <= count; i++ )
    {
        index++;
        RexxObjectPtr arg = context->ArrayAt(args, i);
        if ( arg == NULLOBJECT )
        {
            missingArgException(context->threadContext, i);
            goto done_out;
        }

        ti.pszText = (LPSTR)context->ObjectToStringValue(arg);

        ret = TabCtrl_InsertItem(hwnd, index, &ti);
        if ( ret == -1 )
        {
            goto done_out;
        }

        ((pCDialogControl)pCSelf)->lastItem = ret;
    }

done_out:
    return ret;
}


RexxMethod2(int32_t, tab_addFullSeq, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    TCITEM ti = {0};
    ti.mask = TCIF_TEXT;

    int32_t ret = -1;
    int32_t index = ((pCDialogControl)pCSelf)->lastItem;
    size_t count = context->ArraySize(args);

    for ( size_t i = 1; i <= count; i += 3 )
    {
        index++;
        RexxObjectPtr label = context->ArrayAt(args, i);
        if ( label == NULLOBJECT )
        {
            missingArgException(context->threadContext, i);
            goto done_out;
        }

        ti.pszText = (LPSTR)context->ObjectToStringValue(label);

        RexxObjectPtr _imageIndex = context->ArrayAt(args, i + 1);
        RexxObjectPtr userData = context->ArrayAt(args, i + 2);

        if ( _imageIndex != NULLOBJECT )
        {
            int32_t imageIndex;
            if ( ! context->Int32(_imageIndex, &imageIndex) )
            {
                notPositiveArgException(context->threadContext, i + 1, _imageIndex);
                goto done_out;
            }

            ti.mask |= TCIF_IMAGE;
            ti.iImage = imageIndex;
        }
        if ( userData != NULLOBJECT )
        {
            ti.mask |= TCIF_PARAM;
            ti.lParam = (LPARAM)userData;
        }

        int32_t ret = TabCtrl_InsertItem(hwnd, index, &ti);
        if ( ret == -1 )
        {
            goto done_out;
        }

        ((pCDialogControl)pCSelf)->lastItem = ret;
    }

done_out:
    return ret;
}


RexxMethod2(RexxObjectPtr, tab_itemInfo, int32_t, index, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    char buff[256];
    TCITEM ti;

    ti.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
    ti.pszText = buff;
    ti.cchTextMax = 255;

    RexxObjectPtr result = TheNegativeOneObj;

    if ( TabCtrl_GetItem(hwnd, index, &ti) )
    {
        RexxStemObject stem = context->NewStem("ItemInfo");
        context->SetStemElement(stem, "!TEXT", context->String(ti.pszText));
        context->SetStemElement(stem, "!IMAGE", context->Int32(ti.iImage));
        context->SetStemElement(stem, "!PARAM", (ti.lParam == 0 ? TheZeroObj : (RexxObjectPtr)ti.lParam));
        result = stem;
    }
    return result;
}


RexxMethod1(RexxObjectPtr, tab_selected, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    char buff[256];
    TCITEM ti = {0};

    ti.mask = TCIF_TEXT;
    ti.pszText = buff;
    ti.cchTextMax = 255;

    if ( TabCtrl_GetItem(hwnd, TabCtrl_GetCurSel(hwnd), &ti) == 0 )
    {
        return TheZeroObj;
    }
    return context->String(buff);
}


RexxMethod2(int32_t, tab_select, CSTRING, text, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    int32_t result = -1;

    char buff[256];
    TCITEM ti = {0};
    size_t count;

    count = TabCtrl_GetItemCount(hwnd);
    if ( count == 0 )
    {
        goto done_out;
    }

    ti.mask = TCIF_TEXT;
    ti.cchTextMax = 255;

    size_t i = 0;
    while ( i < count)
    {
        // Note that MSDN says: If the TCIF_TEXT flag is set in the mask member
        // of the TCITEM structure, the control may change the pszText member of
        // the structure to point to the new text instead of filling the buffer
        // with the requested text. The control may set the pszText member to
        // NULL to indicate that no text is associated with the item.
        ti.pszText = buff;

        if ( TabCtrl_GetItem(hwnd, i, &ti) == 0 )
        {
            goto done_out;
        }

        if ( ti.pszText != NULL && stricmp(ti.pszText, text) == 0 )
        {
            result = TabCtrl_SetCurSel(hwnd, i);
            break;
        }
        i++;
    }

done_out:
    return result;
}


RexxMethod3(RexxObjectPtr, tab_getItemRect, uint32_t, item, RexxObjectPtr, rect, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    PRECT r = (PRECT)rxGetRect(context, rect, 2);
    if ( r == NULL )
    {
        return NULLOBJECT;
    }

    return (TabCtrl_GetItemRect(hwnd, item, r) == 0 ? TheFalseObj : TheTrueObj);
}


/** Tab::calcWindowRect()
 *
 *  calcWindowRect() takes a display rectangle and adjusts the rectangle to be
 *  the window rect of the tab control needed for that display size.
 *
 *  Therefore, if the display size must be a fixed size, use calcWindowRect() to
 *  receive the size the tab control needs to be and use it to set the size for
 *  the control.
 *
 *  @param  [IN / OUT] On entry, a .Rect object specifying the display rectangle
 *                     and on return the corrsponding window rect for the tab.
 *
 *  @return  The return is 0 and has no meaning.
 *
 *  Tab::calcDisplayRect()
 *
 *  caclDisplayRect() takes the window rect of the tab control, and adjusts the
 *  rectangle to the size the display will be.
 *
 *  So, if the tab control needs to be a fixed size, use calcDisplayRect() to
 *  get the size the display rect will be for the fixed size of the tab control
 *  and use that to set the size of the control or dialog set into the tab
 *  control
 *
 *  @param  [IN / OUT] On entry, a .Rect object specifying the window rect of
 *                     the tab, and on return the corrsponding display rect.
 *
 *  @return  The return is 0 and has no meaning.
 *
 *  @remarks  MSDN says of the second arg to TabCtrl_AdjustRect():
 *
 *            Operation to perform. If this parameter is TRUE, prc specifies a
 *            display rectangle and receives the corresponding window rectangle.
 *            If this parameter is FALSE, prc specifies a window rectangle and
 *            receives the corresponding display area.
 */
RexxMethod3(RexxObjectPtr, tab_calcRect, RexxObjectPtr, rect, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    PRECT r = (PRECT)rxGetRect(context, rect, 1);
    if ( r == NULL )
    {
        return NULLOBJECT;
    }

    BOOL calcWindowRect = (method[4] == 'W');

    TabCtrl_AdjustRect(hwnd, calcWindowRect, r);
    return TheZeroObj;
}


/** Tab::setImageList()
 *
 *  Sets or removes the image list for a Tab control.
 *
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg is only used if ilSrc is a single bitmap.
 *                Then this arg is the width of a single image.  The default is
 *                the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in
 *                which case it is the height of the bitmap.  The default is the
 *                height of the actual bitmap
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 */
RexxMethod4(RexxObjectPtr, tab_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    oodResetSysErrCode(context->threadContext);

    HIMAGELIST himl = NULL;
    RexxObjectPtr imageList = NULLOBJECT;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
    }
    else if ( context->IsOfType(ilSrc, "IMAGELIST") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
    }
    else
    {
        imageList = oodILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }
    }

    TabCtrl_SetImageList(hwnd, himl);
    return rxSetObjVar(context, TABIMAGELIST_ATTRIBUTE, imageList);

err_out:
    return NULLOBJECT;
}

/** Tab::getImageList()
 *
 *  Gets the Tab control's image list.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod1(RexxObjectPtr, tab_getImageList, OSELF, self)
{
    RexxObjectPtr result = context->GetObjectVariable(TABIMAGELIST_ATTRIBUTE);
    return (result == NULLOBJECT) ? TheNilObj : result;
}


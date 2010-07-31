/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
 * oodData.cpp
 *
 * Implements the function that "sets" and "gets" data.
 *
 * The original ooDialog design used the abstraction that there were only two
 * objects involved.  The underlying Windows dialog object and the Rexx dialog
 * object. The Rexx dialog object would exchange information (data) with the
 * underlying dialog object by "setting" or "getting" data.  The data itself was
 * the state of the various dialog controls.
 *
 * Although ooDialog design has evolved away from that abstraction, the
 * functionality of this module needs to remain to retain backwards
 * compatibility.
 */

#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>

#include <commctrl.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif

#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodData.hpp"

static inline oodControl_t searchDataTable(pCPlainBaseDialog pcpbd, uint32_t id)
{
    if ( pcpbd->DataTab != NULL )
    {
        register size_t ndx = 0;
        while ( ndx < pcpbd->DT_size && pcpbd->DataTab[ndx].id != id )
        {
            ndx++;
        }
        if ( ndx < pcpbd->DT_size )
        {
            return pcpbd->DataTab[ndx].type;
        }
    }
    return winUnknown;
}

/* The assumption is that if UnsignedInt32() fails, number remains unchanged. */
static inline uint32_t dlgDataToNumber(RexxMethodContext *c, RexxObjectPtr data)
{
    uint32_t number = 0;
    c->UnsignedInt32(data, &number);
    return number;
}

/* Is the control type one used with a data attribute. */
inline bool isDataAttributeControl(oodControl_t control)
{
    switch ( control )
    {
        case winEdit :
        case winCheckBox :
        case winRadioButton :
        case winComboBox :
        case winListBox :
        case winTreeView :
        case winListView :
        case winTrackBar :
        case winTab :
        case winDateTimePicker :
        case winMonthCalendar :
        case winUpDown :
            return true;

        default :
            return false;
    }
}

/* Does the item in the data table have the WS_GROUP style. */
static inline bool hasGroupStyle(HWND hwnd, pCPlainBaseDialog pcpbd, size_t index)
{
    return ((GetWindowLong(GetDlgItem(hwnd, pcpbd->DataTab[index].id), GWL_STYLE) & WS_GROUP) == WS_GROUP);
}

/* Is control 1 in the same dialog as control 2. Needed for CategoryDialogs. */
static inline bool isInSameDlg(pCPlainBaseDialog pcpbd, size_t control1, size_t control2)
{
    return (pcpbd->DataTab[control1].category == pcpbd->DataTab[control2].category);
}

/*
 * The manualCheckRadioButton() function is used to check one radio button
 * within a WS_GROUP group and uncheck all the others.
 */
static bool manualCheckRadioButton(pCPlainBaseDialog pcpbd, HWND hW, ULONG id, ULONG value)
{
   size_t beg, en, i;
   bool rc, ordered;
   size_t ndx = 0;

   if ( value == 0 )
   {
       // This function only checks a radio button, not unchecks a radio button.
       return true;
   }
   while ( (ndx < pcpbd->DT_size) && (pcpbd->DataTab[ndx].id != id) )
   {
       ndx++;
   }

   if ( ndx >= pcpbd->DT_size )
   {
       // Not found.
       return false;
   }
   if ( pcpbd->DataTab[ndx].type != winRadioButton )
   {
       // The one to check is not a radio button.
       return false;
   }

   // Search for first and last radio button in the same group, in the same
   // dialog. (There may be other dialog controls in the group.)
   beg = ndx;
   while ( beg > 0 && isInSameDlg(pcpbd, beg - 1, ndx) )
   {
       // Check must be before decrement (beg--)
       if ( hasGroupStyle(hW, pcpbd, beg) )
       {
           break;
       }
       beg--;
   }
   en = ndx;
   while ( ((en + 1) < pcpbd->DT_size) && isInSameDlg(pcpbd, en + 1, ndx) && ! hasGroupStyle(hW, pcpbd, en + 1) )
   {
       en++;
   }

   // Check whether the ids are all radio buttons in ascending order.
   ordered = true;
   for ( i = beg; i < en; i++ )
   {
       if ( pcpbd->DataTab[i].id >= pcpbd->DataTab[i+1].id || pcpbd->DataTab[i].type != winRadioButton )
       {
           ordered = false;
           break;
       }
   }

   // If the ids are ordered, use the Windows API, otherwise do it manually.
   if ( ordered )
   {
       rc = CheckRadioButton(hW, pcpbd->DataTab[beg].id, pcpbd->DataTab[en].id, pcpbd->DataTab[ndx].id) != 0;
   }
   else
   {
       // Uncheck all radio buttons ...
       for ( i = beg; i <= en; i++ )
       {
           if ( pcpbd->DataTab[i].type == winRadioButton )
           {
               CheckDlgButton(hW, pcpbd->DataTab[i].id, 0);
           }
       }
       // ... and check the specified one.
       rc = CheckDlgButton(hW, pcpbd->DataTab[ndx].id, 1) != 0;
   }
   return rc;
}

static bool getMultiListBoxSelections(HWND hDlg, uint32_t id, char * data)
{
    int sel[1500];
    char buffer[NR_BUFFER];

    data[0] = '\0';

    // 1500 elements should not be a problem because the data buffer size is 8
    // KB and 1500 times approximately 5 Bytes is less than 8 KB.
    LRESULT result = SendDlgItemMessage(hDlg, id, LB_GETSELITEMS, 1500, (LPARAM)sel);
    if ( result == LB_ERR )
    {
        return false;
    }
    for ( LRESULT j = 0; j < result; j++ )
    {
        strcat(data, itoa(sel[j]+1, buffer, 10));
        strcat(data, " ");
    }
    return true;
}


static bool setMultiListBoxSelections(HWND hDlg, ULONG id, const char * data)
{
    char buffer[NR_BUFFER];
    const char * p = data;

    // First set all items to not selected.
    LRESULT i = SendDlgItemMessage(hDlg, id, LB_GETCOUNT, 0, 0);
    for ( LRESULT j = 0; j < i; j++ )
    {
        SendDlgItemMessage(hDlg, id, LB_SETSEL, (WPARAM)FALSE, (LPARAM)j);
    }

    // Now set the items passed to us as selected.  Note that the empty string
    // results in no items selected.
    while ( (p) && (*p) )
    {
        buffer[0] = '\0';
        size_t j = 0;
        while ( p && (j < NR_BUFFER) && (*p != ' ') && (*p != '\0') )
        {
            buffer[j++] = *p++;
        }
        buffer[j] = '\0';

        if ( atoi(buffer) > 0 )
        {
            if ( SendDlgItemMessage(hDlg, id, LB_SETSEL, TRUE, (LPARAM)atoi(buffer) - 1) == LB_ERR )
            {
                return false;
            }
        }
        if ( *p )
        {
            p++;
        }
    }
    return true;
}


static bool getListBoxData(HWND hwnd, uint32_t itemID, char *data)
{
    if ( isSingleSelectionListBox(hwnd, itemID) )
    {
        LRESULT result = SendDlgItemMessage(hwnd, itemID, LB_GETCURSEL, 0, 0);
        if ( result != LB_ERR && (SendDlgItemMessage(hwnd, itemID, LB_GETTEXTLEN, result, 0) < DATA_BUFFER) )
        {
            result = SendDlgItemMessage(hwnd, itemID, LB_GETTEXT, result, (LPARAM)data);
        }
        return (result != LB_ERR);
    }
    else
    {
        return getMultiListBoxSelections(hwnd, itemID, data);
    }
}

/**
 * Sets the 'data' in a single selection or multiple selection list box.  For
 * single selection list boxes, the 'data' is considered to be the text of an
 * item.  For multiple selection the 'data' is considered to be the indexs.
 *
 * @param hwnd
 * @param itemID
 * @param itemText
 *
 * @return bool
 *
 * @remarks  Prior to 4.1.0 there was no way to start a dialog with no item
 *           selected in a single selection list box.  Which was annoying.  To
 *           fix that we check if itemText is exactly the string "0".  If so, we
 *           make sure no item is selected.  (Once the dialog is initialized,
 *           the deselectIndex() method can be used to ensure no item is
 *           selected.)
 */
static bool setListBoxData(HWND hwnd, uint32_t itemID, CSTRING itemText)
{
    if ( isSingleSelectionListBox(hwnd, itemID) )
    {
        LRESULT result;

        if ( strcmp(itemText, "0") == 0 )
        {
            SendDlgItemMessage(hwnd, itemID, LB_SETCURSEL, -1, 0);
            result = LB_OKAY;
        }
        else
        {
            result = SendDlgItemMessage(hwnd, itemID, LB_FINDSTRING, 0, (LPARAM)itemText);
            if ( result != LB_ERR)
            {
               result = SendDlgItemMessage(hwnd, itemID, LB_SETCURSEL, result, 0);
            }

            // If we have, or still have an error at this point, try to select
            // the first itme.
            if ( result == LB_ERR )
            {
                SendDlgItemMessage(hwnd, itemID, LB_SETCURSEL, 0, 0);
            }
        }
        return result != LB_ERR;
    }
    else
    {
        return setMultiListBoxSelections(hwnd, itemID, itemText);
    }
}

/**
 * Gets the text in the selection field of a combo box.
 *
 * For drop down list combo boxes, this is the same as the current selection.
 *
 * However, for simple and drop down combo boxes, the text could be text the
 * user typed in and there may be no current selection.
 *
 * @param hwnd    Window handle of the dialog.
 * @param itemID  Resource ID of the combo box.
 * @param data    Buffer to return the text in.
 *
 * @return True on success, otherwise false.
 *
 * @remarks The text in the selection field could be the empty string and / or
 *          there might not be a current selection.  In these cases false is
 *          returned and the caller returns the empty string to Rexx.  The empty
 *          string is the 'correct' result in these cases.
 */
static bool getComboBoxData(HWND hwnd, uint32_t itemID, char *data)
{
    LRESULT result = 0;

    if ( isDropDownList(hwnd, itemID) )
    {
        result = SendDlgItemMessage(hwnd, itemID, CB_GETCURSEL, 0, 0);
        if ( result != CB_ERR && (SendDlgItemMessage(hwnd, itemID, CB_GETLBTEXTLEN, result, 0) < DATA_BUFFER) )
        {
            result = SendDlgItemMessage(hwnd, itemID, CB_GETLBTEXT, result, (LPARAM)data);
        }
    }
    else
    {
        if ( GetDlgItemText(hwnd, itemID, data, (DATA_BUFFER-1)) == 0 )
        {
            result = CB_ERR;
        }
    }
    return result != CB_ERR;
}

/**
 * Sets the text in the selection field of a combo box.
 *
 * For drop down list combo boxes this is the same as the current selection.
 *
 * However, since simple and drop down combo boxes can have text that the user
 * types in, that is not one of the list items, for these combo boxes the text
 * is just set.
 *
 * @param hwnd      Dialog window handle.
 * @param itemID    Combo box resource id.
 * @param itemText  The text to set.
 *
 * @return 0 on success, otherwise 1.
 */
static uint32_t setComboBoxData(HWND hwnd, uint32_t itemID, CSTRING itemText)
{
    if ( isDropDownList(hwnd, itemID) )
    {
        LRESULT index = SendDlgItemMessage(hwnd, itemID, CB_FINDSTRING, 0, (LPARAM)itemText);
        if ( index != LB_ERR )
        {
            index = SendDlgItemMessage(hwnd, itemID, CB_SETCURSEL, index, 0);
        }
        if ( index == CB_ERR )
        {
            SendDlgItemMessage(hwnd, itemID, CB_SETCURSEL, 0, 0);
            return 1;
        }
        return 0;
    }

    return (SetDlgItemText(hwnd, itemID, itemText) ? 0 : 1);
}


/* TODO these stub functions for Get / Set data need to be filled in. */
static bool getDateTimeData(HWND hDlg, char *data, int ctrlID)
{
   return false;
}

static bool setDateTimeData(HWND hDlg, const char *data, int ctrlID)
{
   return false;
}

static bool getMonthCalendarData(HWND hDlg, char *data, int ctrlID)
{
   return false;
}

static bool setMonthCalendarData(HWND hDlg, const char *data, int ctrlID)
{
   return false;
}


static bool getTreeViewData(HWND hW, char * ldat, INT item)
{
   TV_ITEM tvi;
   tvi.hItem = TreeView_GetNextItem(GetDlgItem(hW, item), NULL, TVGN_CARET);
   if (tvi.hItem)
   {
       tvi.mask = TVIF_HANDLE | TVIF_TEXT;
       tvi.pszText = ldat;
       tvi.cchTextMax = DATA_BUFFER-1;
       if (TreeView_GetItem(GetDlgItem(hW, item), &tvi)) return true;
   }
   return false;
}

static bool setTreeViewData(HWND hDlg, const char * ldat, uint32_t ctrlID)
{
   TVITEM tvi = {0};
   CHAR data[DATA_BUFFER];

   HWND hCtrl = GetDlgItem(hDlg, ctrlID);
   if ( hCtrl != NULL && *ldat != '\0' )
   {
       HTREEITEM hTreeItem, root = TreeView_GetRoot(hCtrl);
       tvi.hItem = root;
       while ( tvi.hItem != NULL )
       {
            tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_CHILDREN;
            tvi.pszText = data;
            tvi.cchTextMax = DATA_BUFFER - 1;
            if ( TreeView_GetItem(hCtrl, &tvi) )
            {
                if ( stricmp(tvi.pszText, ldat) == 0 )
                {
                    return (TreeView_SelectItem(hCtrl, tvi.hItem) ? true : false);
                }
                else
                {
                    if ( tvi.cChildren > 0 )
                    {
                        hTreeItem = TreeView_GetChild(hCtrl, tvi.hItem);
                    }
                    else
                    {
                        hTreeItem = TreeView_GetNextSibling(hCtrl, tvi.hItem);
                    }

                    while ( hTreeItem == NULL && tvi.hItem != NULL )
                    {
                        tvi.hItem = TreeView_GetParent(hCtrl, tvi.hItem);
                        hTreeItem = TreeView_GetNextSibling(hCtrl, tvi.hItem);
                        if ( hTreeItem == root )
                        {
                            return false;
                        }
                    }

                    if ( tvi.hItem == NULL )
                    {
                        return false;
                    }
                    tvi.hItem = hTreeItem;
                }
            }
            else
            {
                tvi.hItem = NULL;
            }
       }
   }
   return false;
}


static bool getListViewData(HWND hW, char * ldat, INT item)
{
   LONG it = -1, cnt, j;
   CHAR buffer[NR_BUFFER];
   HWND iW = GetDlgItem(hW, item);
   size_t len = 0;

   ldat[0] = '\0';
   cnt  = ListView_GetSelectedCount(iW);
   if (!cnt) return TRUE;

   for (j=0; j<cnt; j++)
   {
      it = ListView_GetNextItem(iW, it, LVNI_ALL | LVNI_SELECTED);
      if ((it != -1) && (len < DATA_BUFFER - 10))
      {
          strcat(ldat, ltoa(it, buffer, 10));
          strcat(ldat, " ");
          len += strlen(buffer)+1;
      }
      else return false;
   }
   return true;
}


static bool setListViewData(HWND hW, const char * ldat, INT item)
{
   INT i, j;
   CHAR buffer[NR_BUFFER];
   HWND iW = GetDlgItem(hW, item);

   const char *p = ldat;

   i = ListView_GetItemCount(iW);
   for (j=0; j<i; j++)
      ListView_SetItemState(iW, j, 0, LVIS_SELECTED);

   i = 0;
   while ((p) && (*p))
   {
      buffer[0] = '\0';
      j = 0;
      while (p && (j<NR_BUFFER) && (*p != ' ')) buffer[j++] = *p++;
      buffer[j] = '\0';
      ListView_SetItemState(iW, atol(buffer), LVIS_SELECTED, LVIS_SELECTED);
      if (*p) p++;
   }
   return true;
}


static bool getTrackBarData(HWND hW, char * ldat, INT item)
{
   ltoa((long)SendMessage(GetDlgItem(hW, item), TBM_GETPOS, 0,0), ldat, 10);
   return true;
}


static bool setTrackBarData(HWND hW, const char * ldat, INT item)
{
   SendMessage(GetDlgItem(hW, item), TBM_SETPOS, TRUE, atol(ldat));
   return true;
}


static bool getUpDownData(HWND hDlg, char * buffer, uint32_t ctrlID)
{
    BOOL error;
    int32_t pos = (int32_t)SendMessage(GetDlgItem(hDlg, ctrlID), UDM_GETPOS32, 0, (LPARAM)&error);
    if ( ! error )
    {
        itoa(pos, buffer, 10);
        return true;
    }
   return false;
}


static bool setUpDownData(HWND hDlg, const char * data, uint32_t ctrlID)
{
   SendMessage(GetDlgItem(hDlg, ctrlID), UDM_SETPOS32, 0, atoi(data));
   return true;
}


static bool getTabData(HWND hW, char * ldat, INT item)
{
   TC_ITEM tab;
   LONG cur;
   HWND iw = GetDlgItem(hW, item);

   cur = TabCtrl_GetCurSel(iw);
   if ( cur == -1 )
   {
       return false;
   }

   tab.mask = TCIF_TEXT;
   tab.pszText = ldat;
   tab.cchTextMax = DATA_BUFFER - 1;
   return TabCtrl_GetItem(iw, cur, &tab) ? true : false;
}


static bool setTabData(HWND hW, const char * ldat, INT item)
{
   TC_ITEM tab;
   LONG cnt, i = 0;
   CHAR data[DATA_BUFFER];
   HWND iw = GetDlgItem(hW, item);

   cnt = TabCtrl_GetItemCount(iw);
   if ( cnt == 0 )
   {
       return false;
   }

   while ( i < cnt )
   {
       tab.mask = TCIF_TEXT;
       tab.pszText = data;
       tab.cchTextMax = DATA_BUFFER - 1;
       if ( ! TabCtrl_GetItem(iw, i, &tab) )
       {
           return false;
       }
       if ( stricmp(tab.pszText, ldat) == 0 )
       {
           return (TabCtrl_SetCurSel(iw, i) == -1 ? false : true);
       }
       i++;
   }
   return false;
}


RexxObjectPtr getControlData(RexxMethodContext *c, pCPlainBaseDialog pcpbd, uint32_t id, HWND hDlg, oodControl_t ctrlType)
{
    if ( ctrlType == winUnknown )
    {
        ctrlType = searchDataTable(pcpbd, id);
        if ( ctrlType == winUnknown )
        {
            ctrlType = winEdit;
        }
    }

    char data[DATA_BUFFER];
    data[0] = '\0';
    RexxStringObject result = c->NullString();

    switch ( ctrlType )
    {
        case winEdit:
        case winStatic:
            // We don't check for errors, result will end up being the "right"
            // thing and we just return it.
            rxGetWindowText(c, GetDlgItem(hDlg, id), &result);
            return result;

        case winRadioButton:
        case winCheckBox:
            return c->UnsignedInt32(IsDlgButtonChecked(hDlg, id));

        case winListBox:
            if ( ! getListBoxData(hDlg, id, data) )
            {
                return result;
            }
            break;

        case winComboBox:
            if ( ! getComboBoxData(hDlg, id, data) )
            {
                return result;
            }
            break;

        case winTreeView:
            if ( ! getTreeViewData(hDlg, data, id) )
            {
                return result;
            }
            break;

        case winListView:
            if ( ! getListViewData(hDlg, data, id) )
            {
                return result;
            }
            break;

        case winTrackBar:
            if ( ! getTrackBarData(hDlg, data, id) )
            {
                return result;
            }
            break;

        case winTab:
            if ( ! getTabData(hDlg, data, id) )
            {
                return result;
            }
            break;

        case winDateTimePicker:
            if ( ! getDateTimeData(hDlg, data, id) )
            {
                return result;
            }
            break;

        case winMonthCalendar:
            if ( ! getMonthCalendarData(hDlg, data, id) )
            {
                return result;
            }
            break;

        case winUpDown:
            if ( ! getUpDownData(hDlg, data, id) )
            {
                return result;
            }
            break;

        default:
            return result;
    }

    return c->String(data);
}

/**
 *
 * @param c
 * @param pcpbd
 * @param id
 * @param data
 * @param hDlg
 * @param ctrlType
 *
 * @return uint32_t
 */
int32_t setControlData(RexxMethodContext *c, pCPlainBaseDialog pcpbd, uint32_t id, CSTRING data,
                       HWND hDlg, oodControl_t ctrlType)
{
    if ( ctrlType == winUnknown )
    {
        ctrlType = searchDataTable(pcpbd, id);
        if ( ctrlType == winUnknown )
        {
            ctrlType = winEdit;
        }
    }

    switch ( ctrlType )
    {
        case winEdit:
        case winStatic:
            return (SetDlgItemText(hDlg, id, data) ? 0 : 1);
        case winCheckBox:
            return (CheckDlgButton(hDlg, id, atoi(data)) ? 0 : 1);
        case winRadioButton:
            return (manualCheckRadioButton(pcpbd, hDlg, id, atoi(data)) ? 0 : 1);
        case winListBox:
            return (setListBoxData(hDlg, id, data) ? 0 : 1);
        case winComboBox:
            return setComboBoxData(hDlg, id, data);
        case winTreeView:
            return (setTreeViewData(hDlg, data, id) ? 0 : 1);
        case winListView:
            return (setListViewData(hDlg, data, id) ? 0 : 1);
        case winTrackBar:
            return (setTrackBarData(hDlg, data, id) ? 0 : 1);
        case winTab:
            return (setTabData(hDlg, data, id) ? 0 : 1);
        case winDateTimePicker:
            return (setDateTimeData(hDlg, data, id) ? 0 : 1);
        case winMonthCalendar:
            return (setMonthCalendarData(hDlg, data, id) ? 0 : 1);
        case winUpDown:
            return (setUpDownData(hDlg, data, id) ? 0 : 1);
        default:
            return 1;
    }
}


/**
 * The implementation for PlainBaseDialog::setDlgDataFromStem().
 *
 * @param c              Method context we are operating in.
 * @param pcpbd          Pointer to the PlainBaseDialog CSelf.
 * @param internDlgData  Stem object.  The set tails are the dialog control
 *                       resource IDs and the values of the tails represent how
 *                       to set the "data" of the control.
 */
uint32_t setDlgDataFromStem(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxStemObject internDlgData)
{
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(c, pcpbd->rexxSelf);
        return 1;
    }

    size_t j;
    HWND hwnd;
    uint32_t itemID;
    RexxObjectPtr dataObj;
    oodControl_t controlType;

    for ( j = 0; j < pcpbd->DT_size; j++ )
    {
        hwnd        = pcpbd->childDlg[pcpbd->DataTab[j].category];
        itemID      = pcpbd->DataTab[j].id;
        controlType = pcpbd->DataTab[j].type;

        dataObj = c->GetStemArrayElement(internDlgData, itemID);
        if ( dataObj == NULLOBJECT )
        {
            // The pre-4.0.0 code just ignored a stem index not set and would
            // use the string, say INTERNDLGDATA.31.  We will use the empty
            // string.
            dataObj = c->NullString();
        }

        if ( controlType == winStatic || controlType == winEdit )
        {
            SetDlgItemText(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == winCheckBox )
        {
            CheckDlgButton(hwnd, itemID, dlgDataToNumber(c, dataObj));
        }
        else if ( controlType == winRadioButton )
        {
            manualCheckRadioButton(pcpbd, hwnd, itemID, dlgDataToNumber(c, dataObj));
        }
        else if ( controlType == winListBox )
        {
            setListBoxData(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == winComboBox )
        {
            setComboBoxData(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == winTreeView )
        {
            setTreeViewData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == winListView )
        {
            setListViewData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == winTrackBar )
        {
            setTrackBarData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == winTab )
        {
            setTabData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == winDateTimePicker )
        {
            setDateTimeData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == winMonthCalendar )
        {
            setMonthCalendarData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == winUpDown )
        {
            setUpDownData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
    }
    return 0;
}



uint32_t putDlgDataInStem(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxStemObject internDlgData)
{
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(c, pcpbd->rexxSelf);
        return 1;
    }

    size_t j;
    char data[DATA_BUFFER];
    HWND hwnd;
    uint32_t itemID;
    oodControl_t controlType;

    for ( j = 0; j < pcpbd->DT_size; j++ )
    {
        data[0] = '\0';

        hwnd =        pcpbd->childDlg[pcpbd->DataTab[j].category];
        itemID =      pcpbd->DataTab[j].id;
        controlType = pcpbd->DataTab[j].type;

        if ( controlType == winEdit || controlType == winStatic )
        {
            GetDlgItemText(hwnd, itemID, data, (DATA_BUFFER-1));
        }
        else if ( controlType == winCheckBox || controlType == winRadioButton )
        {
            c->SetStemArrayElement(internDlgData, itemID, c->UnsignedInt32(IsDlgButtonChecked(hwnd, itemID)));
            continue;
        }
        else if ( controlType == winListBox )
        {
            if ( ! getListBoxData(hwnd, itemID, data) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winComboBox )
        {
            if ( ! getComboBoxData(hwnd, itemID, data) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winTreeView )
        {
            if ( ! getTreeViewData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winListView )
        {
            if ( ! getListViewData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winTrackBar )
        {
            if ( ! getTrackBarData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winTab )
        {
            if ( ! getTabData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winDateTimePicker )
        {
            if ( ! getDateTimeData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winMonthCalendar )
        {
            if ( ! getMonthCalendarData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == winUpDown )
        {
            if ( ! getUpDownData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else
        {
            data[0] = '\0';
        }

        c->SetStemArrayElement(internDlgData, itemID, c->String(data));
    }
    return 0;
}

/**
 * Adds an entry to the Rexx dialog's "data table."  The data table essentially
 * maps a dialog control's resource ID to the type of control it is.  An edit
 * control, a check box control, etc..  This in turn is used by the Rexx
 * dialog's "get" and "set" data functions.  Note that the 'data' is really the
 * state of the control.
 *
 * The table entry is either made through the Rexx dialog's connectXXX()
 * methods, or by the data autodetection feature.
 *
 * Data autodetection is only used for ResDialogs.  When the underlying dialog
 * is first created, its child windows are enumerated and an entry is made in
 * the data table for each control found. The connectXXX() methods allow the
 * Rexx programmer to manually connect the controls she wants.
 *
 * In both scenarios, a Rexx dialog attribute that represents the 'data' of a
 * specific control is created and the data table entry allows the get / set
 * data methods to change the value of the attribute through the data table
 * mapping.
 *
 * The data table entry contains the resource id, the 'type' identifier, and a
 * CatalogDialog's category number.  (These numbers are the page number of the
 * control in the catalog dialog.)  The category numbers in the table entry are
 * not used anywhere in the code, as of 4.0.0.  It looks like they never were.
 * It could be that the original developers had some future use in mind for this
 * field that never got implemented.
 *
 * The pre 4.1.0 code also had "get" and "set" data table functions beside the
 * "add" function.  The set function was not called anywhere in the code. The
 * set function would change the value of an existing data table entry. Since
 * there does not seem to be any purpose to that, the function was dropped.
 *
 * The get function returned the values of a single data table entry.  But it
 * was only used in one place, in a loop to get all entries.  And only the ID
 * value was used. So that function was replaced by getDataTableIDs() which
 * returns an array of all the resource IDs for every table entry.
 *
 * @param c       Method context we are operating in, or null.
 * @param pcpbd
 * @param id      The resource ID of the control.
 * @param type
 * @param category
 *
 * @return 0 on succes, and 1 for error.
 *
 * @remarks  addToDataTable() can be called from the WindowLoopThread, where we
 *           don't have a valid method context.  In this case, we don't do an
 *           out of memory exception, just pass back the return code and let the
 *           higher level deal with it.
 */
uint32_t addToDataTable(RexxMethodContext *c, pCPlainBaseDialog pcpbd, int id, oodControl_t type, uint32_t category)
{
    if ( pcpbd->DataTab == NULL )
    {
        pcpbd->DataTab = (DATATABLEENTRY *)LocalAlloc(LPTR, sizeof(DATATABLEENTRY) * MAX_DT_ENTRIES);
        if ( !pcpbd->DataTab )
        {
            if ( c != NULL )
            {
                outOfMemoryException(c->threadContext);
            }
            return OOD_MEMORY_ERR;
        }
        pcpbd->DT_size = 0;
    }

    if ( pcpbd->DT_size < MAX_DT_ENTRIES )
    {
        pcpbd->DataTab[pcpbd->DT_size].id = id;
        pcpbd->DataTab[pcpbd->DT_size].type = type;
        pcpbd->DataTab[pcpbd->DT_size].category = category;
        pcpbd->DT_size ++;
        return OOD_NO_ERROR;
    }

    MessageBox(0, "Dialog data items have exceeded the maximum number of\n"
               "allocated table entries. No data item can be added.",
               "Error", MB_OK | MB_ICONHAND);
    return OOD_DATATABLE_FULL;
}

/**
 * Return an array containing, in order, the resource ID of each table entry.
 *
 * @param c      Method context we are operating in.
 * @param pcpbd  Pointer to the PlainBaseDialog CSelf.
 * @param self   The Rexx dialog object.
 *
 * @return An array containing all the resource IDs in the data table.
 */
RexxArrayObject getDataTableIDs(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr self)
{
    size_t count = pcpbd->DT_size;
    RexxArrayObject result = c->NewArray(count);

    for ( size_t i = 0; i < count; i++ )
    {
        c->ArrayPut(result, c->UnsignedInt32(pcpbd->DataTab[i].id), i + 1);
    }
    return result;
}


/**
 * Searches all the child windows in a dialog and adds a data table entry for
 * each dialog control that is applicable.
 *
 * This is done when 'auto detection' is set on (the default) by the Rexx
 * programmer. This function is called when a dialog is created from a resource
 * DLL, i.e. the dialog template is a compiled binary.  In UserDialogs (and
 * subclasses) the data table entry is done for each createXXX() method when the
 * dialog control is added to the in-memory template.
 *
 * @param pcpbd  Pointer to the CSelf struct of the ResDialog being created.
 *
 * @return A code indication success, or not.  The only failure here is an out
 *         of memory problem or the data table is full.
 *
 * @remarks  This function is only called from the WindowLoopThread, which is
 *           creating a dialog from a resource DLL.  There is no valid method
 *           context.  If there is a failure in addToDataTable() we just pass
 *           the result code back and let the higher level handle it.
 */
uint32_t doDataAutoDetection(pCPlainBaseDialog pcpbd)
{
    uint32_t result = OOD_NO_ERROR;

    HWND parent, current, next;
    oodControl_t itemToAdd;

    parent = pcpbd->hDlg;
    current = parent;
    next = GetTopWindow(current);

    while ( next && ((HWND)getWindowPtr(next, GWLP_HWNDPARENT) == parent) )
    {
       current = next;
       itemToAdd = winNotAControl;

       if ( (GetWindowLong(current, GWL_STYLE) & WS_VISIBLE) != 0 )
       {
           itemToAdd = control2controlType(current);
           if ( ! isDataAttributeControl(itemToAdd) )
           {
               itemToAdd = winNotAControl;
           }
       }

       if ( itemToAdd != winNotAControl )
       {
           result = addToDataTable(NULL, pcpbd, GetWindowLong(current, GWL_ID), itemToAdd, 0);
           if ( result != OOD_NO_ERROR )
           {
               break;
           }
       }
       next = GetNextWindow(current, GW_HWNDNEXT);
    }
    return result;
}

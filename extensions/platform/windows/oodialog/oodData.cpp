/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>

#include <commctrl.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif

#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodData.hpp"

static inline int searchDataTable(DIALOGADMIN *dlgAdm, uint32_t id)
{
    if ( dlgAdm->DataTab != NULL )
    {
        register int ndx = 0;
        while ( ndx < dlgAdm->DT_size && dlgAdm->DataTab[ndx].id != id )
        {
            ndx++;
        }
        if ( ndx < dlgAdm->DT_size )
        {
            return dlgAdm->DataTab[ndx].typ;
        }
    }
    return -1;
}

/* The assumption is that if UnsignedInt32() fails, number remains unchanged. */
static inline uint32_t dlgDataToNumber(RexxMethodContext *c, RexxObjectPtr data)
{
    uint32_t number = 0;
    c->UnsignedInt32(data, &number);
    return number;
}

/*
 * The manualCheckRadioButton() function is used to check one radio button
 * within a WS_GROUP group and uncheck all the others.
 */

static inline bool hasGroupStyle(HWND hwnd, DIALOGADMIN *dlgAdm, uint32_t index)
{
    return ((GetWindowLong(GetDlgItem(hwnd, dlgAdm->DataTab[index + 1].id), GWL_STYLE) & WS_GROUP) == WS_GROUP);
}

static inline bool isInSameDlg(DIALOGADMIN *dlgAdm, uint32_t control1, uint32_t control2)
{
    return (dlgAdm->DataTab[control1].category == dlgAdm->DataTab[control2].category);
}

static bool manualCheckRadioButton(DIALOGADMIN * dlgAdm, HWND hW, ULONG id, ULONG value)
{
   LONG beg, en, ndx, i;
   bool rc, ordered;
   ndx = 0;

   if ( value == 0 )
   {
       // This function only checks a radio button, not unchecks a radio button.
       return true;
   }
   while ( (ndx < dlgAdm->DT_size) && (dlgAdm->DataTab[ndx].id != id) )
   {
       ndx++;
   }

   if ( ndx >= dlgAdm->DT_size )
   {
       // Not found.
       return false;
   }
   if ( dlgAdm->DataTab[ndx].typ != 2 )
   {
       // The one to check is not a radio button.
       return false;
   }

   // Search for first and last radio button in the same group, in the same
   // dialog. (There may be other dialog controls in the group.)
   beg = ndx;
   while ( beg > 0 && isInSameDlg(dlgAdm, beg - 1, ndx) )
   {
       // Check must be before decrement (beg--)
       if ( hasGroupStyle(hW, dlgAdm, beg) )
       {
           break;
       }
       beg--;
   }
   en = ndx;
   while ( ((en + 1) < dlgAdm->DT_size) && isInSameDlg(dlgAdm, en + 1, ndx) && ! hasGroupStyle(hW, dlgAdm, en + 1) )
   {
       en++;
   }

   // Check whether the ids are all radio buttons in ascending order.
   ordered = true;
   for ( i = beg; i < en; i++ )
   {
       if ( dlgAdm->DataTab[i].id >= dlgAdm->DataTab[i+1].id || dlgAdm->DataTab[i].typ != 2 )
       {
           ordered = false;
           break;
       }
   }

   // If the ids are ordered, use the Windows API, otherwise do it manually.
   if ( ordered )
   {
       rc = CheckRadioButton(hW, dlgAdm->DataTab[beg].id, dlgAdm->DataTab[en].id, dlgAdm->DataTab[ndx].id) != 0;
   }
   else
   {
       // Uncheck all radio buttons ...
       for ( i = beg; i <= en; i++ )
       {
           if ( dlgAdm->DataTab[i].typ == 2 )
           {
               CheckDlgButton(hW, dlgAdm->DataTab[i].id, 0);
           }
       }
       // ... and check the specified one.
       rc = CheckDlgButton(hW, dlgAdm->DataTab[ndx].id, 1) != 0;
   }
   return rc;
}


static void getMultiListBoxSelections(HWND hW, ULONG id, char * data)
{
    INT sel[1500];
    CHAR buffer[NR_BUFFER];
    LRESULT i;

    /* 1500 elements should not be a problem because data is 8 KB (1500 * approx. 5 Byte < 8 KB) */
    i = SendDlgItemMessage(hW, id, LB_GETSELITEMS, 1500, (LPARAM)sel);
    data[0] = '\0';
    if (i == LB_ERR)
    {
        return;
    }
    for (LRESULT j=0; j < i; j++)
    {
        strcat(data, itoa(sel[j]+1, buffer, 10));
        strcat(data, " ");
    }
}


static bool setMultiListBoxSelections(HWND hW, ULONG id, const char * data)
{
    CHAR buffer[NR_BUFFER];
    const char * p;

    p = data;

    LRESULT i = SendDlgItemMessage(hW, id, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    for (LRESULT j=0; j<i; j++)
    {
        SendDlgItemMessage(hW, id, LB_SETSEL, (WPARAM) FALSE, (LPARAM) j);
    }

    i = 0;
    while ((p) && (*p))
    {
        buffer[0] = '\0';
        size_t j = 0;
        while (p && (j<NR_BUFFER) && (*p != ' ') && (*p != '\0')) buffer[j++] = *p++;
        buffer[j] = '\0';
        if (atoi(buffer) > 0)
        {
            i = SendDlgItemMessage(hW, id, LB_SETSEL, TRUE, (LPARAM)atoi(buffer)-1);
            if (i == LB_ERR) return false;
        }
        if (*p) p++;
    }
    return true;
}


static int getListBoxData(HWND hwnd, uint32_t itemID, char *data)
{
    int i = (int)SendDlgItemMessage(hwnd, itemID, LB_GETCURSEL, 0, 0);
    if ( i != LB_ERR && (SendDlgItemMessage(hwnd, itemID, LB_GETTEXTLEN, i, 0) < DATA_BUFFER) )
    {
        return (int)SendDlgItemMessage(hwnd, itemID, LB_GETTEXT, i, (LPARAM)data);
    }
    return i;                                                                                                                               \
}

static uint32_t setListBoxData(HWND hwnd, uint32_t itemID, CSTRING itemText)
{
    int i = (int)SendDlgItemMessage(hwnd, itemID, LB_FINDSTRING, 0, (LPARAM)itemText);
    if ( i != LB_ERR)
    {                                                                                                                                  \
       i = (int)SendDlgItemMessage(hwnd, itemID, LB_SETCURSEL, i, 0);
       if ( i == LB_ERR )
       {                                                                                                                            \
           SendDlgItemMessage(hwnd, itemID, LB_SETCURSEL, 0, 0);
       }
    }                                                                                                                                   \
    else
    {
        SendDlgItemMessage(hwnd, itemID, LB_SETCURSEL, 0, 0);
    }
    return 0;
}

/*
 * The following functions are to get the value of a combo box that has the
 * CBS_DROPDOWNLIST flag enabled and behaves like a list box.
 */

static int getComboBoxData(HWND hwnd, uint32_t itemID, char *data)
{
    int i = (int)SendDlgItemMessage(hwnd, itemID, CB_GETCURSEL, 0, 0);
    if ( i != LB_ERR && (SendDlgItemMessage(hwnd, itemID, CB_GETLBTEXTLEN, i, 0) < DATA_BUFFER) )
    {
        return (int)SendDlgItemMessage(hwnd, itemID, CB_GETLBTEXT, i, (LPARAM)data);
    }
    return i;                                                                                                                                      \
}

static uint32_t setComboBoxData(HWND hwnd, uint32_t itemID, CSTRING itemText)
{
    int index = (int)SendDlgItemMessage(hwnd, itemID, CB_FINDSTRING, 0, (LPARAM)itemText);
    if ( index != LB_ERR )
    {                                                                                                                                  \
        index = (int)SendDlgItemMessage(hwnd, itemID, CB_SETCURSEL, index, 0);
        if ( index == CB_ERR )
        {                                                                                                                            \
            SendDlgItemMessage(hwnd, itemID, CB_SETCURSEL, 0, 0);
        }
    }                                                                                                                                   \
    else
    {
        SendDlgItemMessage(hwnd, itemID, CB_SETCURSEL, 0, 0);
    }
    return 0;
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


static bool getTreeData(HWND hW, char * ldat, INT item)
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

// TODO this seems like it might be missing a return true.
static bool setTreeData(HWND hW, const char * ldat, INT item)
{
   TV_ITEM tvi;
   CHAR data[DATA_BUFFER];

   HWND iW = GetDlgItem(hW, item);
   if (iW && strlen(ldat)) {
       HTREEITEM it, root = TreeView_GetRoot(iW);
       tvi.hItem = root;
       while (tvi.hItem)
       {
            tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_CHILDREN;
            tvi.pszText = data;
            tvi.cchTextMax = DATA_BUFFER-1;
            if (TreeView_GetItem(iW, &tvi))
            {
                if (!stricmp(tvi.pszText, ldat))
                {
                    if (TreeView_SelectItem(iW, tvi.hItem)) return true;
                }
                else
                {
                    if (tvi.cChildren) it = TreeView_GetChild(iW, tvi.hItem);
                    else it = TreeView_GetNextSibling(iW, tvi.hItem);
                    while (!it && tvi.hItem)
                    {
                        tvi.hItem = TreeView_GetParent(iW, tvi.hItem);
                        it = TreeView_GetNextSibling(iW, tvi.hItem);
                        if (it == root) return false;
                    }
                    if (!tvi.hItem) return false;
                    tvi.hItem = it;
                }
            }
            else tvi.hItem = NULL;
       }
   }
   return false;
}


static bool getListData(HWND hW, char * ldat, INT item)
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


static bool setListData(HWND hW, const char * ldat, INT item)
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


static bool getSliderData(HWND hW, char * ldat, INT item)
{
   ltoa((long)SendMessage(GetDlgItem(hW, item), TBM_GETPOS, 0,0), ldat, 10);
   return true;
}


static bool setSliderData(HWND hW, const char * ldat, INT item)
{
   SendMessage(GetDlgItem(hW, item), TBM_SETPOS, TRUE, atol(ldat));
   return true;
}


static bool getTabCtrlData(HWND hW, char * ldat, INT item)
{
   TC_ITEM tab;
   LONG cur;
   HWND iw = GetDlgItem(hW, item);

   cur = TabCtrl_GetCurSel(iw);
   if (cur == -1) return FALSE;

   tab.mask = TCIF_TEXT;
   tab.pszText = ldat;
   tab.cchTextMax = DATA_BUFFER-1;
   return TabCtrl_GetItem(iw, cur, &tab) ? true : false;
}


static bool setTabCtrlDatas(HWND hW, const char * ldat, INT item)
{
   TC_ITEM tab;
   LONG cnt, i = 0;
   CHAR data[DATA_BUFFER];
   HWND iw = GetDlgItem(hW, item);

   cnt = TabCtrl_GetItemCount(iw);
   if (!cnt) return false;

   while (i<cnt)
   {
       tab.mask = TCIF_TEXT;
       tab.pszText = data;
       tab.cchTextMax = DATA_BUFFER-1;
       if (!TabCtrl_GetItem(iw, i, &tab)) return false;
       if (!stricmp(tab.pszText, ldat))
       {
           return (TabCtrl_SetCurSel(iw, i) == -1 ? false : true);
       }
       i++;
   }
   return false;
}


RexxObjectPtr internalGetItemData(RexxMethodContext *c, pCPlainBaseDialog pcpbd, uint32_t id, HWND hDlg, int ctrlType)
{
    if ( pcpbd->dlgAdm == NULL || pcpbd->hDlg == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    DIALOGADMIN *dlgAdm = seekDlgAdm(pcpbd->hDlg);
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    if ( ctrlType == -1 )
    {
        ctrlType = searchDataTable(dlgAdm, id);
        if ( ctrlType == -1 )
        {
            ctrlType = 0;
        }
    }

    char data[DATA_BUFFER];
    data[0] = '\0';
    RexxStringObject result = c->NullString();

    switch ( ctrlType )
    {
        case 0:
        {
            // We don't check for errors, result will end up being the "right"
            // thing and we just return it.
            rxGetWindowText(c, GetDlgItem(hDlg, id), &result);
            return result;
        }
        case 1:
        case 2:
            return c->UnsignedInt32(IsDlgButtonChecked(hDlg, id));
        case 3:
            if ( getListBoxData(hDlg, id, data) == LB_ERR )
            {
                return result;
            }
            break;
        case 4:
            getMultiListBoxSelections(hDlg, id, data);
            break;
        case 5:
            if ( getComboBoxData(hDlg, id, data) == CB_ERR )
            {
                return result;
            }
            break;
        case 6:
            if ( ! getTreeData(hDlg, data, id) )
            {
                return result;
            }
            break;
        case 7:
            if ( ! getListData(hDlg, data, id) )
            {
                return result;
            }
            break;
        case 8:
            if ( ! getSliderData(hDlg, data, id) )
            {
                return result;
            }
            break;
        case 9:
            if ( ! getTabCtrlData(hDlg, data, id) )
            {
                return result;
            }
            break;
        case 10:
            if ( ! getDateTimeData(hDlg, data, id) )
            {
                return result;
            }
            break;
        case 11:
            if ( ! getMonthCalendarData(hDlg, data, id) )
            {
                return result;
            }
            break;

        default:
            return result;
    }

    return c->String(data);
}


uint32_t internalSetItemData(RexxMethodContext *c, pCPlainBaseDialog pcpbd, uint32_t id, CSTRING data,
                             HWND hDlg, int ctrlType)
{
    if ( pcpbd->dlgAdm == NULL || pcpbd->hDlg == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    DIALOGADMIN *dlgAdm = seekDlgAdm(pcpbd->hDlg);
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    if ( ctrlType == -1 )
    {
        ctrlType = searchDataTable(dlgAdm, id);
        if ( ctrlType == -1 )
        {
            ctrlType = 0;
        }
    }

    switch ( ctrlType )
    {
        case 0:
            return (SetDlgItemText(hDlg, id, data) ? 0 : 1);
        case 1:
            return (CheckDlgButton(hDlg, id, atoi(data)) ? 0 : 1);
        case 2:
            return (manualCheckRadioButton(dlgAdm, hDlg, id, atoi(data)) ? 0 : 1);
        case 3:
            return setListBoxData(hDlg, id, data);
        case 4:
            return (setMultiListBoxSelections(hDlg, id, data) ? 0 : 1);
        case 5:
            return setComboBoxData(hDlg, id, data);
        case 6:
            return (setTreeData(hDlg, data, id) ? 0 : 1);
        case 7:
            return (setListData(hDlg, data, id) ? 0 : 1);
        case 8:
            return (setSliderData(hDlg, data, id) ? 0 : 1);
        case 9:
            return (setTabCtrlDatas(hDlg, data, id) ? 0 : 1);
        case 10:
            return (setDateTimeData(hDlg, data, id) ? 0 : 1);
        case 11:
            return (setMonthCalendarData(hDlg, data, id) ? 0 : 1);
        default:
            return 1;
    }
}


/**
 * The implementation for PlainBaseDialog::setDlgDataFromStem() which is a private
 * method.
 *
 * @param c              Method context we are operating in.
 * @param pcpbd          Pointer to the PlainBaseDialog CSelf.
 * @param internDlgData  Stem object.  The set tails are the dialog control
 *                       resource IDs and the values of the tails represent how
 *                       to set the "data" of the control.
 */
uint32_t setDlgDataFromStem(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxStemObject internDlgData)
{
    // TODO  Since this is an implementation of a private mehtod, these error
    // conditions should be impossible.  But, while converting from the old APIs
    // to the new APIs, the following checks exactly mimic the old code.  Once
    // things are fully converted, it should be sufficient to just check that
    // hDlg is not null.
    if ( pcpbd->dlgAdm == NULL || pcpbd->hDlg == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    DIALOGADMIN *dlgAdm = seekDlgAdm(pcpbd->hDlg);
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    size_t j;
    HWND hwnd;
    uint32_t itemID;
    RexxObjectPtr dataObj;
    USHORT controlType;

    for ( j = 0; j < dlgAdm->DT_size; j++ )
    {
        if ( dlgAdm->DataTab[j].typ == 999 )
        {
            // See the connectSeparator() method and the manualCheckRadioButton
            // above. Used to separate two groups of radio buttons, there is no
            // real control involved.
            continue;
        }

        hwnd        = dlgAdm->ChildDlg[dlgAdm->DataTab[j].category];
        itemID      = dlgAdm->DataTab[j].id;
        controlType = dlgAdm->DataTab[j].typ;

        dataObj = c->GetStemArrayElement(internDlgData, itemID);
        if ( dataObj == NULLOBJECT )
        {
            // The pre-4.0.0 code just ignored a stem index not set and would
            // use the string, say INTERNDLGDATA.31.  We will use the empty
            // string.
            dataObj = c->NullString();
        }

        if ( controlType == 0 )
        {
            SetDlgItemText(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == 1 )
        {
            CheckDlgButton(hwnd, itemID, dlgDataToNumber(c, dataObj));
        }
        else if ( controlType == 2 )
        {
            manualCheckRadioButton(dlgAdm, hwnd, itemID, dlgDataToNumber(c, dataObj));
        }
        else if ( controlType == 3 )
        {
            setListBoxData(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == 4 )
        {
            setMultiListBoxSelections(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == 5 )
        {
            setComboBoxData(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == 6 )
        {
            setTreeData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == 7 )
        {
            setListData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == 8 )
        {
            setSliderData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == 9 )
        {
            setTabCtrlDatas(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
    }
    return 0;
}



uint32_t putDlgDataInStem(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxStemObject internDlgData)
{
    if ( pcpbd->dlgAdm == NULL || pcpbd->hDlg == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    DIALOGADMIN *dlgAdm = seekDlgAdm(pcpbd->hDlg);
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return 0;
    }

    size_t j;
    CHAR data[DATA_BUFFER];
    HWND hwnd;
    uint32_t itemID;
    USHORT controlType;

    for ( j = 0; j < dlgAdm->DT_size; j++ )
    {
        if ( dlgAdm->DataTab[j].typ == 999 )
        {
            // See the connectSeparator() method and the manualCheckRadioButton
            // above. Used to separate two groups of radio buttons, there is no
            // real control involved.
            continue;
        }

        data[0] = '\0';

        hwnd =        dlgAdm->ChildDlg[dlgAdm->DataTab[j].category];
        itemID =      dlgAdm->DataTab[j].id;
        controlType = dlgAdm->DataTab[j].typ;

        if ( controlType == 0 )
        {
            GetDlgItemText(hwnd, itemID, data, (DATA_BUFFER-1));
        }
        else if ( controlType == 2 || controlType == 1 )
        {
            c->SetStemArrayElement(internDlgData, itemID, c->UnsignedInt32(IsDlgButtonChecked(hwnd, itemID)));
            continue;
        }
        else if ( controlType == 3 )
        {
            if ( getListBoxData(hwnd, itemID, data) == LB_ERR )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 4 )
        {
            getMultiListBoxSelections(hwnd, itemID, data);
        }
        else if ( controlType == 5 )
        {
            if ( getComboBoxData(hwnd, itemID, data) == LB_ERR )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 6 )
        {
            if ( ! getTreeData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 7 )
        {
            if ( ! getListData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 8 )
        {
            if ( ! getSliderData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 9 )
        {
            if ( ! getTabCtrlData(hwnd, data, itemID) )
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
 * methods, or by the data autodetection feature.  For data autodetection, when
 * the underlying dialog is first created, its child windows are enumerated and
 * an entry is made in the data table for each control found.  The connectXXX()
 * methods allow the Rexx programmer to manually connect the controls she wants.
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
 * The pre 4.1.0 code also had "get" and "set" functions beside the "add"
 * function.  The set function was not used anywhere in the code.  Set was used
 * to change the value of an existing data table entry.  Since there does not
 * seem to be any purpose to that, the function was dropped.
 *
 * The get function returned the values of a single data table entry.  But it
 * was only used in one place, in a loop to get all entries.  And only the ID
 * value was used. So that function was replaced by getDataTableIDs() which
 * returns an array of all the resource IDs for every table entry.
 *
 * @param c
 * @param dlgAdm
 * @param rxID     The resource ID of the control.  This has already been
 *                 resolved to a numeric id by the caller, but, it has not been
 *                 checked to see if it is -1 yet.
 * @param typ
 * @param category
 *
 * @return 0 on succes, -1 for a bad resource ID, and 1 for error.
 */
RexxObjectPtr addToDataTable(RexxMethodContext *c, DIALOGADMIN *dlgAdm, RexxObjectPtr rxID, uint32_t typ, uint32_t category)
{
    int id;
    if ( ! c->Int32(rxID, &id) || id == -1 )
    {
        return TheNegativeOneObj;
    }

    if ( dlgAdm->DataTab == NULL )
    {
        dlgAdm->DataTab = (DATATABLEENTRY *)LocalAlloc(LPTR, sizeof(DATATABLEENTRY) * MAX_DT_ENTRIES);
        if ( !dlgAdm->DataTab )
        {
            outOfMemoryException(c->threadContext);
            return TheOneObj;
        }
        dlgAdm->DT_size = 0;
    }

    if ( dlgAdm->DT_size < MAX_DT_ENTRIES )
    {
        dlgAdm->DataTab[dlgAdm->DT_size].id = id;
        dlgAdm->DataTab[dlgAdm->DT_size].typ = typ;
        dlgAdm->DataTab[dlgAdm->DT_size].category = category;
        dlgAdm->DT_size ++;
        return TheZeroObj;
    }

    MessageBox(0, "Dialog data items have exceeded the maximum number of\n"
               "allocated table entries. No data item can be added.",
               "Error", MB_OK | MB_ICONHAND);
    return TheOneObj;
}

/**
 * Return an array containing, in order, the resource ID of each table entry.
 *
 * @param c
 * @param pcpbd
 * @param self
 *
 * @return An array containing all the resource IDs in the data table.
 */
RexxArrayObject getDataTableIDs(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr self)
{
    if ( pcpbd == NULL || pcpbd->dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, self);
        return NULLOBJECT;
    }

    uint32_t count = pcpbd->dlgAdm->DT_size;
    RexxArrayObject result = c->NewArray(count);
    for ( size_t i = 0; i < count; i++ )
    {
        c->ArrayPut(result, c->UnsignedInt32(pcpbd->dlgAdm->DataTab[i].id), i + 1);
    }
    return result;
}

/* search for all the child windows in the dialog and add them to the data list */
bool DataAutodetection(DIALOGADMIN * dlgAdm)
{
    HWND parent, current, next;
    LONG style;
    CHAR classname[64];
    INT itemtoadd;

    parent = dlgAdm->TheDlg;
    current = parent;
    next = GetTopWindow(current);
    while ((next) && ((HWND)getWindowPtr(next, GWLP_HWNDPARENT) == parent))
    {
       current = next;

       itemtoadd = -1;
       style = GetWindowLong(current, GWL_STYLE);
       if (GetClassName(current, classname, 64))
       {
           strcpy(classname, strupr(classname));
           if ((!strcmp(classname, "EDIT")) && (style & WS_VISIBLE))
              itemtoadd = 0;
           else
           if ((!strcmp(classname, "COMBOBOX")) && (style & WS_VISIBLE) && (style & CBS_DROPDOWNLIST))
              itemtoadd = 5;
           else
           if ((!strcmp(classname, "COMBOBOX")) && (style & WS_VISIBLE))
              itemtoadd = 0;
           else
           if ((!strcmp(classname, "BUTTON")) && (style & WS_VISIBLE)
           && (((style & 0x0000000F) == BS_CHECKBOX) || ((style & 0x0000000F) == BS_AUTOCHECKBOX)))
              itemtoadd = 1;
           else
           if ((!strcmp(classname, "BUTTON")) && (style & WS_VISIBLE)
           && (((style & 0x0000000F) == BS_RADIOBUTTON) || ((style & 0x0000000F) == BS_AUTORADIOBUTTON)))
              itemtoadd = 2;
           else
           if ((!strcmp(classname, "LISTBOX")) && (style & WS_VISIBLE) && (style & LBS_MULTIPLESEL))
              itemtoadd = 4;
           else
           if ((!strcmp(classname, "LISTBOX")) && (style & WS_VISIBLE))
              itemtoadd = 3;
           else
           if ((!strcmp(classname, WC_TREEVIEW)) && (style & WS_VISIBLE))
              itemtoadd = 6;
           else
           if ((!strcmp(classname, WC_LISTVIEW)) && (style & WS_VISIBLE))
              itemtoadd = 7;
           else
           if ((!strcmp(classname, TRACKBAR_CLASS)) && (style & WS_VISIBLE))
              itemtoadd = 8;
           else
           if ((!strcmp(classname, WC_TABCONTROL)) && (style & WS_VISIBLE))
              itemtoadd = 9;
           else
           if ((!strcmp(classname, DATETIMEPICK_CLASS)) && (style & WS_VISIBLE))
              itemtoadd = 10;
           else
           if ((!strcmp(classname, MONTHCAL_CLASS)) && (style & WS_VISIBLE))
              itemtoadd = 11;
       }

       if (itemtoadd >= 0)
       {
          if (!dlgAdm->DataTab)
          {
              dlgAdm->DataTab = (DATATABLEENTRY *)LocalAlloc(LPTR, sizeof(DATATABLEENTRY) * MAX_DT_ENTRIES);
              if (!dlgAdm->DataTab)
              {
                   MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
                   return false;
              }
              dlgAdm->DT_size = 0;
          }
          if (dlgAdm->DT_size < MAX_DT_ENTRIES)
          {
              dlgAdm->DataTab[dlgAdm->DT_size].id = GetWindowLong(current, GWL_ID);
              dlgAdm->DataTab[dlgAdm->DT_size].typ = itemtoadd;
              dlgAdm->DataTab[dlgAdm->DT_size].category = 0;
              dlgAdm->DT_size ++;
          }
          else
          {
              MessageBox(0, "Dialog data items have exceeded the maximum\n"
                            "number of allocated table entries. Data\n"
                            "autodetection has failed.",
                         "Error",MB_OK | MB_ICONHAND);
              return false;
          }
       }
       next = GetNextWindow(current, GW_HWNDNEXT);
    }
    return true;
}

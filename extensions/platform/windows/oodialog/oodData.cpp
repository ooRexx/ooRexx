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

#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>

#include <commctrl.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif

#include "APICommon.h"
#include "oodCommon.h"
#include "oodData.hpp"


/*
 * The manualCheckRadioButton() function is used to check one radio button
 * within a WS_GROUP group and uncheck all the others.
 */

inline bool hasGroupStyle(HWND hwnd, DIALOGADMIN *dlgAdm, uint32_t index)
{
    return ((GetWindowLong(GetDlgItem(hwnd, dlgAdm->DataTab[index + 1].id), GWL_STYLE) & WS_GROUP) == WS_GROUP);
}

inline bool isInSameDlg(DIALOGADMIN *dlgAdm, uint32_t control1, uint32_t control2)
{
    return (dlgAdm->DataTab[control1].category == dlgAdm->DataTab[control2].category);
}

bool manualCheckRadioButton(DIALOGADMIN * dlgAdm, HWND hW, ULONG id, ULONG value)
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


void GetMultiListBoxSelections(HWND hW, ULONG id, char * data)
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


BOOL SetMultiListBoxSelections(HWND hW, ULONG id, const char * data)
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
            if (i == LB_ERR) return FALSE;
        }
        if (*p) p++;
    }
    return TRUE;
}


int getListBoxData(HWND hwnd, uint32_t itemID, char *data)
{
    int i = (int)SendDlgItemMessage(hwnd, itemID, LB_GETCURSEL, 0, 0);
    if ( i != LB_ERR && (SendDlgItemMessage(hwnd, itemID, LB_GETTEXTLEN, i, 0) < DATA_BUFFER) )
    {
        return (int)SendDlgItemMessage(hwnd, itemID, LB_GETTEXT, i, (LPARAM)data);
    }
    return i;                                                                                                                               \
}

#define GETLBDATA(ldat, item, quit) \
                {\
                   i = (int)SendDlgItemMessage(hW, item, LB_GETCURSEL, 0, 0); \
                   if ((i!=LB_ERR) && (SendDlgItemMessage(hW, item, LB_GETTEXTLEN, i, 0) < DATA_BUFFER))    \
                   {                                                                                                                                 \
                       i = (int)SendDlgItemMessage(hW, item, LB_GETTEXT, i, (LPARAM)ldat); \
                       if (i!=LB_ERR)                                                                                                         \
                       {                                                                                                                              \
                          if (quit) RETC(0);                                                                                                                \
                       }                                                                                                                              \
                   }                                                                                                                                         \
                }

uint32_t setListBoxData(HWND hwnd, uint32_t itemID, CSTRING itemText)
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
#define SETLBDATA(ldat, item, quit) \
                {\
                   i = (int)SendDlgItemMessage(hW, item, LB_FINDSTRING, 0, (LPARAM)ldat); \
                   if (i!=LB_ERR)                                                                                                    \
                   {                                                                                                                                  \
                      i = (int)SendDlgItemMessage(hW, item, LB_SETCURSEL, i, 0);            \
                      if (i!=LB_ERR)                                                                                               \
                      {                                                                                                                            \
                         if (quit) RETC(0);                                                                                             \
                      } \
                      else                                                                                                                               \
                      { \
                          i = (int)SendDlgItemMessage(hW, item, LB_SETCURSEL, 0, 0);     \
                      }  \
                    }                                                                                                                                   \
                    else i = (int)SendDlgItemMessage(hW, item, LB_SETCURSEL, 0, 0);     \
                 }


/*
 * The following functions are to get the value of a combo box that has the
 * CBS_DROPDOWNLIST flag enabled and behaves like a list
 */


int getComboBoxData(HWND hwnd, uint32_t itemID, char *data)
{
    int i = (int)SendDlgItemMessage(hwnd, itemID, CB_GETCURSEL, 0, 0);
    if ( i != LB_ERR && (SendDlgItemMessage(hwnd, itemID, CB_GETLBTEXTLEN, i, 0) < DATA_BUFFER) )
    {
        return (int)SendDlgItemMessage(hwnd, itemID, CB_GETLBTEXT, i, (LPARAM)data);
    }
    return i;                                                                                                                                      \
}
#define GETCBDATA(ldat, item, quit) \
                {\
                   i = (int)SendDlgItemMessage(hW, item, CB_GETCURSEL, 0, 0); \
                   if ((i!=LB_ERR) && (SendDlgItemMessage(hW, item, CB_GETLBTEXTLEN, i, 0) < DATA_BUFFER)) \
                   {                                                                                                                                 \
                       i = (int)SendDlgItemMessage(hW, item, CB_GETLBTEXT, i, (LPARAM)ldat); \
                       if (i!=LB_ERR)                                                                                                         \
                       {                                                                                                                              \
                          if (quit) RETC(0)                                                                                                               \
                       }                                                                                                                              \
                   }                                                                                                                                         \
                }

uint32_t setComboBoxData(HWND hwnd, uint32_t itemID, CSTRING itemText)
{
    int index = (int)SendDlgItemMessage(hwnd, itemID, CB_FINDSTRING, 0, (LPARAM)itemText);
    if ( index != LB_ERR )
    {                                                                                                                                  \
        index = (int)SendDlgItemMessage(hwnd, itemID, CB_SETCURSEL, index, 0);
        if ( index == LB_ERR )
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

#define SETCBDATA(ldat, item, quit) \
                {\
                   i = (int)SendDlgItemMessage(hW, item, CB_FINDSTRING, 0, (LPARAM)ldat); \
                   if (i!=LB_ERR)                                                                                                    \
                   {                                                                                                                                  \
                      i = (int)SendDlgItemMessage(hW, item, CB_SETCURSEL, i, 0);            \
                      if (i!=LB_ERR)                                                                                               \
                      {                                                                                                                            \
                         if (quit) RETC(0)                                                                                             \
                      } else                                                                                                                               \
                      i = (int)SendDlgItemMessage(hW, item, CB_SETCURSEL, 0, 0);     \
                    }                                                                                                                                   \
                    else i = (int)SendDlgItemMessage(hW, item, CB_SETCURSEL, 0, 0);     \
                 }


/* TODO these stub functions for Get / Set data need to be filled in. */
BOOL GetDateTimeData(HWND hDlg, char *data, int ctrlID)
{
   return FALSE;
}

BOOL SetDateTimeData(HWND hDlg, const char *data, int ctrlID)
{
   return FALSE;
}

BOOL GetMonthCalendarData(HWND hDlg, char *data, int ctrlID)
{
   return FALSE;
}

BOOL SetMonthCalendarData(HWND hDlg, const char *data, int ctrlID)
{
   return FALSE;
}


BOOL GetTreeData(HWND hW, char * ldat, INT item)
{
   TV_ITEM tvi;
   tvi.hItem = TreeView_GetNextItem(GetDlgItem(hW, item), NULL, TVGN_CARET);
   if (tvi.hItem)
   {
       tvi.mask = TVIF_HANDLE | TVIF_TEXT;
       tvi.pszText = ldat;
       tvi.cchTextMax = DATA_BUFFER-1;
       if (TreeView_GetItem(GetDlgItem(hW, item), &tvi)) return TRUE;
   }
   return FALSE;
}

BOOL SetTreeData(HWND hW, const char * ldat, INT item)
{
   TV_ITEM tvi;
   CHAR data[DATA_BUFFER];

   HWND iW = GetDlgItem(hW, item);
   if (iW && strlen(ldat)) {
       HTREEITEM it, root = TreeView_GetRoot(iW);
       tvi.hItem = root;
       while (tvi.hItem) {
            tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_CHILDREN;
            tvi.pszText = data;
            tvi.cchTextMax = DATA_BUFFER-1;
            if (TreeView_GetItem(iW, &tvi))
            {
                if (!stricmp(tvi.pszText, ldat))
                {
                    if (TreeView_SelectItem(iW, tvi.hItem)) return TRUE;
                }
                else
                {
                    if (tvi.cChildren) it = TreeView_GetChild(iW, tvi.hItem);
                    else it = TreeView_GetNextSibling(iW, tvi.hItem);
                    while (!it && tvi.hItem)
                    {
                        tvi.hItem = TreeView_GetParent(iW, tvi.hItem);
                        it = TreeView_GetNextSibling(iW, tvi.hItem);
                        if (it == root) return FALSE;
                    }
                    if (!tvi.hItem) return FALSE;
                    tvi.hItem = it;
                }
            }
            else tvi.hItem = NULL;
       }
   }
   return FALSE;
}


BOOL GetListData(HWND hW, char * ldat, INT item)
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
      else return FALSE;
   }
   return TRUE;
}


BOOL SetListData(HWND hW, const char * ldat, INT item)
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
   return TRUE;
}


BOOL GetSliderData(HWND hW, char * ldat, INT item)
{
   ltoa((long)SendMessage(GetDlgItem(hW, item), TBM_GETPOS, 0,0), ldat, 10);
   return TRUE;
}


BOOL SetSliderData(HWND hW, const char * ldat, INT item)
{
   SendMessage(GetDlgItem(hW, item), TBM_SETPOS, TRUE, atol(ldat));
   return TRUE;
}


BOOL GetTabCtrlData(HWND hW, char * ldat, INT item)
{
   TC_ITEM tab;
   LONG cur;
   HWND iw = GetDlgItem(hW, item);

   cur = TabCtrl_GetCurSel(iw);
   if (cur == -1) return FALSE;

   tab.mask = TCIF_TEXT;
   tab.pszText = ldat;
   tab.cchTextMax = DATA_BUFFER-1;
   return TabCtrl_GetItem(iw, cur, &tab);
}


BOOL SetTabCtrlData(HWND hW, const char * ldat, INT item)
{
   TC_ITEM tab;
   LONG cnt, i = 0;
   CHAR data[DATA_BUFFER];
   HWND iw = GetDlgItem(hW, item);

   cnt = TabCtrl_GetItemCount(iw);
   if (!cnt) return FALSE;

   while (i<cnt)
   {
       tab.mask = TCIF_TEXT;
       tab.pszText = data;
       tab.cchTextMax = DATA_BUFFER-1;
       if (!TabCtrl_GetItem(iw, i, &tab)) return FALSE;
       if (!stricmp(tab.pszText, ldat)) return (!TabCtrl_SetCurSel(iw, i));
       i++;
   }
   return FALSE;
}


size_t RexxEntry GetItemData(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)

{
   CHAR data[DATA_BUFFER];
   HWND hW;
   INT i,id,k;
   DEF_ADM;

   CHECKARGL(2);

   GET_ADM;
   if (!dlgAdm) RETERR

   if (argc > 2)
      hW = GET_HWND(argv[2]);
   else
      hW = dlgAdm->TheDlg;

   id = atoi(argv[1].strptr);

   k = 0;
   if (argc > 3)
       k = atoi(argv[3].strptr);
   else
   {
       SEARCHDATA(dlgAdm, i, id)
       if (VALIDDATA(dlgAdm, i, id)) k = dlgAdm->DataTab[i].typ;
   }

   data[0] = '\0';
   switch (k)
   {
      case 0:
        {
          /* DATA_BUFFER was used to get the text which is a */
          /* hardcoded limit. Now get the text lenght and allocate */
          LPTSTR lpBuffer;
          size_t len = (size_t)SendDlgItemMessage(hW,id,WM_GETTEXTLENGTH,0,0)+1;
          lpBuffer = (LPTSTR)GlobalAlloc(GMEM_FIXED, len);
          if (!lpBuffer) return 1;
          len = GetDlgItemText(hW, id, lpBuffer, (int)len);
          retstr->strptr = lpBuffer;
          strcpy(retstr->strptr, lpBuffer);
          retstr->strlength = len;
          return 0;
          break;
        }
      case 1:
      case 2:
         i = IsDlgButtonChecked(hW, id);
         itoa(i, data, 10);
         break;
      case 3:
         GETLBDATA(data, id, FALSE)
         if (i==LB_ERR) data[0] = '\0';
         break;
      case 4:
         GetMultiListBoxSelections(hW, id, data);
         break;
      case 5:
         GETCBDATA(data, id, FALSE)
         if (i==CB_ERR) data[0] = '\0';
         break;
      case 6:
         if (!GetTreeData(hW, data, id)) data[0] = '\0';
         break;
      case 7:
         if (!GetListData(hW, data, id)) data[0] = '\0';
         break;
      case 8:
         if (!GetSliderData(hW, data, id)) data[0] = '\0';
         break;
      case 9:
         if (!GetTabCtrlData(hW, data, id)) data[0] = '\0';
         break;
      case 10:
         if (!GetDateTimeData(hW, data, id)) data[0] = '\0';
         break;
      case 11:
         if (!GetMonthCalendarData(hW, data, id)) data[0] = '\0';
         break;

      default:
         data[0] = '\0';
   }

   size_t len = strlen(data);
   if (len > 255)
   {
       CHAR * p;
       p = (char *)GlobalAlloc(GMEM_FIXED, len + 1);
       if (!p) return 1;
       retstr->strptr = p;
       strcpy(retstr->strptr, data);
       retstr->strlength = len;
   }
   else
   {
       strcpy(retstr->strptr, data);
       retstr->strlength = len;
   }
   return 0;
}



size_t RexxEntry SetItemData(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)

{
    HWND hW;
    INT i, k, id;

    DEF_ADM;

    CHECKARGL(3);

    GET_ADM;
    if (!dlgAdm)
    {
        RETERR;
    }

    if (argc > 3)
    {
        hW = GET_HWND(argv[3]);
    }
    else
    {
        hW = dlgAdm->TheDlg;
    }

    id = atoi(argv[1].strptr);

    k = 0;
    if (argc > 4)
    {
        k = atoi(argv[4].strptr);
    }
    else
    {
        SEARCHDATA(dlgAdm, i, id)
        if (VALIDDATA(dlgAdm, i, id))
        {
            k = dlgAdm->DataTab[i].typ;
        }
    }

    const char *data = argv[2].strptr;
    switch (k)
    {
        case 0:
            if (SetDlgItemText(hW, id, data))
            {
                RETC(0);
            }
            else
            {
                RETC(1);
            }
            break;
        case 1:
            if (CheckDlgButton(hW, id, atoi(data)))
            {
                RETC(0);
            }
            else
            {
                RETC(1);
            }
            break;
        case 2:
            if (manualCheckRadioButton(dlgAdm, hW, id, atoi(data)))
            {
                RETC(0);
            }
            else
            {
                RETC(1);
            }
            break;
        case 3:
            SETLBDATA(data, id, TRUE)
            RETC(0);
        case 4:
            if (SetMultiListBoxSelections(hW, id, data))
            {
                RETC(0);
            }
            else
            {
                RETC(1);
            }
        case 5:
            SETCBDATA(data, id, TRUE)
            RETC(0);
        case 6:
            RETC(!SetTreeData(hW, data, id))
        case 7:
            RETC(!SetListData(hW, data, id))
        case 8:
            RETC(!SetSliderData(hW, data, id))
        case 9:
            RETC(!SetTabCtrlData(hW, data, id))
        case 10:
            RETC(!SetDateTimeData(hW, data, id))
        case 11:
            RETC(!SetMonthCalendarData(hW, data, id))

        default:
            RETC(1);
    }
}


/* The assumption is that if WholeNumber() fails, number remains unchanged. */
inline uint32_t dlgDataToNumber(RexxMethodContext *c, RexxObjectPtr data)
{
    uint32_t number = 0;
    c->UnsignedInt32(data, &number);
    return number;
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
            continue;
        }

        hwnd        = dlgAdm->ChildDlg[dlgAdm->DataTab[j].category];
        itemID      = dlgAdm->DataTab[j].id;
        controlType = dlgAdm->DataTab[j].typ;

        dataObj = c->GetStemArrayElement(internDlgData, itemID);
        if ( dataObj == NULLOBJECT )
        {
            // TOOD need better exception here, or maybe not.
            char buf[128];
            sprintf(buf, "setDlgDataFromStem() %s.%d has no value", c->GetStemValue(internDlgData), itemID);
            executionErrorException(c->threadContext, buf);
            return 0;
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
            SetMultiListBoxSelections(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == 5 )
        {
            setComboBoxData(hwnd, itemID, c->ObjectToStringValue(dataObj));
        }
        else if ( controlType == 6 )
        {
            SetTreeData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == 7 )
        {
            SetListData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == 8 )
        {
            SetSliderData(hwnd, c->ObjectToStringValue(dataObj), itemID);
        }
        else if ( controlType == 9 )
        {
            SetTabCtrlData(hwnd, c->ObjectToStringValue(dataObj), itemID);
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
            /* Old comment: "no separator"  What does that mean? */
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
            GetMultiListBoxSelections(hwnd, itemID, data);
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
            if ( ! GetTreeData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 7 )
        {
            if ( ! GetListData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 8 )
        {
            if ( ! GetSliderData(hwnd, data, itemID) )
            {
                data[0] = '\0';
            }
        }
        else if ( controlType == 9 )
        {
            if ( ! GetTabCtrlData(hwnd, data, itemID) )
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


size_t RexxEntry DataTable(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   DEF_ADM;

   CHECKARGL(3);

   GET_ADM;
   if (!dlgAdm) RETERR
   if (!strcmp(argv[1].strptr, "ADD"))    /* add a dialog data item to the table */
   {
       CHECKARGL(4);
       if (!dlgAdm->DataTab)
       {
          dlgAdm->DataTab = (DATATABLEENTRY *)LocalAlloc(LPTR, sizeof(DATATABLEENTRY) * MAX_DT_ENTRIES);
          if (!dlgAdm->DataTab)
          {
             MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
             RETC(1);
          }
          dlgAdm->DT_size = 0;
       }
       if (dlgAdm->DT_size < MAX_DT_ENTRIES)
       {
          dlgAdm->DataTab[dlgAdm->DT_size].id = atoi(argv[2].strptr);
          dlgAdm->DataTab[dlgAdm->DT_size].typ = atoi(argv[3].strptr);
          if (argc > 4)
              dlgAdm->DataTab[dlgAdm->DT_size].category = atoi(argv[4].strptr);
          else
              dlgAdm->DataTab[dlgAdm->DT_size].category = 0;
          dlgAdm->DT_size ++;
          RETC(0);
       }
       MessageBox(0, "Dialog data items have exceeded the maximum number of\n"
                     "allocated table entries. No data item can be added.",
                  "Error",MB_OK | MB_ICONHAND);
       RETC(1);
   }
   else
   if (!strcmp(argv[1].strptr, "GET"))     /* get a dialog data item from the table */
   {
       INT i;
       if (!dlgAdm->DataTab) RETC(0)

       i = atoi(argv[2].strptr);

       if ((i >= 0) && (i < dlgAdm->DT_size))
       {
           sprintf(retstr->strptr, "%ld %d %d", dlgAdm->DataTab[i].id,
                                            dlgAdm->DataTab[i].typ,
                                            dlgAdm->DataTab[i].category);
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       } else RETC(0)
   }
   else
   if (!strcmp(argv[1].strptr, "SET"))     /* replace a dialog data item in the table */
   {
       INT sl;

       CHECKARGL(5);

       if (!dlgAdm->DataTab)
       {
          MessageBox(0,"No data table available","Error",MB_OK | MB_ICONHAND);
          RETC(1);
       }
       sl = atoi(argv[2].strptr);

       dlgAdm->DataTab[sl].id = atoi(argv[3].strptr);
       dlgAdm->DataTab[sl].typ = atoi(argv[4].strptr);
       if (argc > 5)
           dlgAdm->DataTab[sl].category = atoi(argv[5].strptr);
       else
           dlgAdm->DataTab[sl].category = 0;
       RETC(0);
   }
   RETERR
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

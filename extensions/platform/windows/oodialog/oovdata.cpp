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


#include <windows.h>
#include <rexx.h>
#include <stdio.h>
#include <dlgs.h>
#include <commctrl.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "oovutil.h"


/* I do most of the radio button handling myself now so I know it works */
BOOL MyCheckRadioButton(DIALOGADMIN * aDlg, HWND hW, ULONG id, ULONG value)
{
   LONG beg, en, ndx, i;
   BOOL rc, ordered;
   ndx = 0;

   if (!value) return TRUE;
   while ((ndx < aDlg->DT_size) && (aDlg->DataTab[ndx].id != id)) ndx++;

   if (ndx >= aDlg->DT_size) return FALSE;
   if (aDlg->DataTab[ndx].typ != 2) return FALSE;  /* the selected one is no radio button */

   /* search for first and last radio button in group (there may be other items in the group also) */
   beg = ndx;
   while ((beg>0) && (aDlg->DataTab[beg-1].category == aDlg->DataTab[ndx].category))
   {
        if ((GetWindowLong(GetDlgItem(hW, aDlg->DataTab[beg].id), GWL_STYLE) & WS_GROUP) == WS_GROUP) break; /* must be before beg--*/
        beg--;
   }
   en = ndx;
   while ((en+1 < aDlg->DT_size) && (aDlg->DataTab[en+1].category == aDlg->DataTab[ndx].category) &&
   ((GetWindowLong(GetDlgItem(hW, aDlg->DataTab[en+1].id), GWL_STYLE)  & WS_GROUP) != WS_GROUP)) en++;

   /* check whether or not the ids are ordered ascending */
   /* also check whether or not any other kind of dialog item is in the group */
   ordered = TRUE;
   for (i = beg; i<en; i++) if ((aDlg->DataTab[i].id >= aDlg->DataTab[i+1].id) || (aDlg->DataTab[i].typ != 2))
   {
       ordered = FALSE;
       break;
   }
   if (ordered)  /* ids are ordered and no other item in group so we can use Windows API */
       rc = CheckRadioButton(hW, aDlg->DataTab[beg].id, aDlg->DataTab[en].id, aDlg->DataTab[ndx].id);
   else {  /* we have to do it ourselves */
       for (i = beg; i<=en; i++) if (aDlg->DataTab[i].typ == 2)
           CheckDlgButton(hW, aDlg->DataTab[i].id, 0);  /* uncheck all radio buttons */
       rc = CheckDlgButton(hW, aDlg->DataTab[ndx].id, 1);  /* check selected */
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


/* The following #defines are to get the value of a combo box that has
   the CBS_DROPDOWNLIST flag enabled and behaves like a list */

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
            if (MyCheckRadioButton(dlgAdm, hW, id, atoi(data)))
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




size_t RexxEntry SetStemData(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   INT i,j, c, rc;
   CHAR data[DATA_BUFFER];
   SHVBLOCK shvb;
   CHAR name[64];
   CHAR sname[64];
   HWND hW;
   DEF_ADM;

   CHECKARG(2);

   GET_ADM;
   if (!dlgAdm) RETERR

   rxstrlcpy(name, argv[1]);
   if (name[strlen(name)-1] == '.') name[strlen(name)-1] = '\0';

   c = 0;
   for (j=0;j<dlgAdm->DT_size;j++)
   {
      if (dlgAdm->DataTab[j].typ != 999)   /* no separator */
          {
            c++;
            sprintf(sname,"%s.%d",name,dlgAdm->DataTab[j].id);
            shvb.shvnext = NULL;
            shvb.shvname.strptr = sname;
            shvb.shvname.strlength = strlen(sname);
            shvb.shvnamelen = shvb.shvname.strlength;
            shvb.shvvalue.strptr = data;
            shvb.shvvalue.strlength = (DATA_BUFFER-1);
            shvb.shvvaluelen = (DATA_BUFFER-1);
            shvb.shvcode = RXSHV_SYFET;
            shvb.shvret = 0;

            if ((rc = RexxVariablePool(&shvb)) == RXSHV_BADN)
            {
               sprintf(data, "Variable %s is not declared!", sname);
               MessageBox(0,data,"Error",MB_OK | MB_ICONHAND);
               RETERR
            }
            shvb.shvvalue.strlength = shvb.shvvaluelen;

            rxdatacpy(data,shvb.shvvalue);
            hW = dlgAdm->ChildDlg[dlgAdm->DataTab[j].category];

            if (dlgAdm->DataTab[j].typ == 0)
            {
                SetDlgItemText(hW, dlgAdm->DataTab[j].id, data);
            }
            else if (dlgAdm->DataTab[j].typ == 1)
            {
                CheckDlgButton(hW, dlgAdm->DataTab[j].id, atoi(data));
            }
            else if (dlgAdm->DataTab[j].typ == 2)
            {
                MyCheckRadioButton(dlgAdm, hW, dlgAdm->DataTab[j].id, atoi(data)) ;
            }
            else if (dlgAdm->DataTab[j].typ == 3)
            {
               SETLBDATA(data, dlgAdm->DataTab[j].id, FALSE)
            }
            else if (dlgAdm->DataTab[j].typ == 4)
            {
               SetMultiListBoxSelections(hW, dlgAdm->DataTab[j].id, data);
            }
            else if (dlgAdm->DataTab[j].typ == 5)
            {
               SETCBDATA(data, dlgAdm->DataTab[j].id, FALSE)
            }
            else if (dlgAdm->DataTab[j].typ == 6)
            {
               SetTreeData(hW, data, dlgAdm->DataTab[j].id);
            }
            else if (dlgAdm->DataTab[j].typ == 7)
            {
               SetListData(hW, data, dlgAdm->DataTab[j].id);
            }
            else if (dlgAdm->DataTab[j].typ == 8)
            {
               SetSliderData(hW, data, dlgAdm->DataTab[j].id);
            }
            else if (dlgAdm->DataTab[j].typ == 9)
            {
               SetTabCtrlData(hW, data, dlgAdm->DataTab[j].id);
            }
          }
   }
   RETC(0);
}



size_t RexxEntry GetStemData(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   INT i,j, c;
   CHAR data[DATA_BUFFER];
   SHVBLOCK shvb;
   CHAR name[64];
   CHAR sname[64];
   ULONG da;
   HWND hW;
   DEF_ADM;

   CHECKARG(2);

   GET_ADM;
   if (!dlgAdm) RETERR

   rxstrlcpy(name, argv[1]);
   if (name[strlen(name)-1] == '.') name[strlen(name)-1] = '\0';

   c = 0;
   for (j=0;j<dlgAdm->DT_size;j++)
   {
      if (dlgAdm->DataTab[j].typ != 999)   /* no separator */
      {
        c++;
        data[0] = '\0';
        hW = dlgAdm->ChildDlg[dlgAdm->DataTab[j].category];
        if (dlgAdm->DataTab[j].typ == 0)
            da = GetDlgItemText(hW, dlgAdm->DataTab[j].id, data, (DATA_BUFFER-1));
        else
        if (dlgAdm->DataTab[j].typ == 2 || dlgAdm->DataTab[j].typ == 1)
        {
           da = IsDlgButtonChecked(hW, dlgAdm->DataTab[j].id);
           itoa(da, data, 10);
        }
        else
        if (dlgAdm->DataTab[j].typ == 3)
        {
            GETLBDATA(data, dlgAdm->DataTab[j].id, FALSE)
            if (i == LB_ERR) data[0] = '\0';
        } else
        if (dlgAdm->DataTab[j].typ == 4)
        {
           GetMultiListBoxSelections(hW, dlgAdm->DataTab[j].id, data);
        } else
        if (dlgAdm->DataTab[j].typ == 5)
        {
            GETCBDATA(data, dlgAdm->DataTab[j].id, FALSE)
            if (i == CB_ERR) data[0] = '\0';
        } else
        if (dlgAdm->DataTab[j].typ == 6)
        {
            if (!GetTreeData(hW, data, dlgAdm->DataTab[j].id)) data[0] = '\0';
        } else
        if (dlgAdm->DataTab[j].typ == 7)
        {
            if (!GetListData(hW, data, dlgAdm->DataTab[j].id)) data[0] = '\0';
        } else
        if (dlgAdm->DataTab[j].typ == 8)
        {
            if (!GetSliderData(hW, data, dlgAdm->DataTab[j].id)) data[0] = '\0';
        } else
        if (dlgAdm->DataTab[j].typ == 9)
        {
            if (!GetTabCtrlData(hW, data, dlgAdm->DataTab[j].id)) data[0] = '\0';
        } else
        {
            data[0] = '\0';
        }


        sprintf(sname,"%s.%d",name,dlgAdm->DataTab[j].id);
        shvb.shvnext = NULL;
        shvb.shvname.strptr = sname;
        shvb.shvname.strlength = strlen(sname);
        shvb.shvnamelen = shvb.shvname.strlength;
        shvb.shvvalue.strptr = data;
        shvb.shvvalue.strlength = strlen(data);
        shvb.shvvaluelen = strlen(data);
        shvb.shvcode = RXSHV_SYSET;
        shvb.shvret = 0;
        if (RexxVariablePool(&shvb) == RXSHV_BADN) {
           sprintf(data, "Variable %s could not be declared", sname);
           MessageBox(0,data,"Error",MB_OK | MB_ICONHAND);
           RETERR
        }
      }
   }
   RETC(0)
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
BOOL DataAutodetection(DIALOGADMIN * aDlg)
{
    HWND parent, current, next;
    LONG style;
    CHAR classname[64];
    INT itemtoadd;

    parent = aDlg->TheDlg;
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
          if (!aDlg->DataTab)
          {
              aDlg->DataTab = (DATATABLEENTRY *)LocalAlloc(LPTR, sizeof(DATATABLEENTRY) * MAX_DT_ENTRIES);
              if (!aDlg->DataTab)
              {
                   MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
                   return FALSE;
              }
              aDlg->DT_size = 0;
          }
          if (aDlg->DT_size < MAX_DT_ENTRIES)
          {
              aDlg->DataTab[aDlg->DT_size].id = GetWindowLong(current, GWL_ID);
              aDlg->DataTab[aDlg->DT_size].typ = itemtoadd;
              aDlg->DataTab[aDlg->DT_size].category = 0;
              aDlg->DT_size ++;
          }
          else
          {
              MessageBox(0, "Dialog data items have exceeded the maximum\n"
                            "number of allocated table entries. Data\n"
                            "autodetection has failed.",
                         "Error",MB_OK | MB_ICONHAND);
              return FALSE;
          }
       }
       next = GetNextWindow(current, GW_HWNDNEXT);
    }
    return TRUE;
}

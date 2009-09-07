/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
 * Open Object REXX OODialog - ooDialog Messaging function
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <commctrl.h>
#include "oodCommon.hpp"


inline bool isHex(CSTRING c)
{
    return strlen(c) > 1 && *c == '0' && toupper(c[1]) == 'X';
}

BOOL AddDialogMessage(CHAR * msg, CHAR * Qptr)
{
   if (strlen(Qptr) + strlen(msg) + 1 < MAXLENQUEUE)
   {
      strcat(Qptr, msg);
      strcat(Qptr, ";");
      return 1;
   } else return 0;
}


char *getDlgMessage(DIALOGADMIN *addressedTo, char *buffer, bool peek)
{
   size_t i = 0, l;
   MSG msg;

   if ( addressedTo->pMessageQueue )
   {
       char * pMsgQ = addressedTo->pMessageQueue;
       l = strlen(pMsgQ);

       // Don't sleep for just a peek.
       if ( !l &&  !PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && !peek )
       {
           Sleep(1);
       }

       // Copy up to the ';'
       while ( i < l && pMsgQ[i] != ';' )
       {
           buffer[i] = pMsgQ[i];
           i++;
       }
       buffer[i]='\0';

       if ( l &&  !peek )
       {
           if ( i >= l )
           {
               pMsgQ[0] = '\0';
           }
           else
           {
               memmove(&pMsgQ[0], &pMsgQ[i + 1], l - i);
           }
       }
   }
   return buffer;
}


#define SelectionDidChange(p) ((p->uNewState & LVIS_SELECTED) != (p->uOldState & LVIS_SELECTED))
#define FocusDidChange(p)     ((p->uNewState & LVIS_FOCUSED) != (p->uOldState & LVIS_FOCUSED))

/* MatchSelectFocus
 * Check that: (a) tag is for select change and focuse change, and (b) that
 * either the selection or the focus actually changed.
 */
#define MatchSelectFocus(tag, p)    \
( ((tag & TAG_SELECTCHANGED) && (tag & TAG_FOCUSCHANGED)) && (SelectionDidChange(p) || FocusDidChange(p)) )

/* MatchSelect
 * Check that: (a) tag is only for selection change and not focuse change, and (b)
 * that the selection actually changed.
 */
#define MatchSelect(tag, p)    \
( ((tag & TAG_SELECTCHANGED) && !(tag & TAG_FOCUSCHANGED)) && (SelectionDidChange(p)) )

/* MatchFocus
 * Check that: (a) tag is only for focus change and not selection change, and (b)
 * that the focus actually changed.
 */
#define MatchFocus(tag, p)    \
( ((tag & TAG_FOCUSCHANGED) && !(tag & TAG_SELECTCHANGED)) && (FocusDidChange(p)) )


MsgReplyType SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo)
{
   register LONG i = 0;
   MESSAGETABLEENTRY * m = addressedTo->MsgTab;

   if (m)
   for (i=0; i<addressedTo->MT_size; i++)
      if ( ((message & m[i].filterM) == m[i].msg)  &&
           ((param & m[i].filterP) == m[i].wParam) &&
           ( ((message == WM_NOTIFY) && ((((NMHDR *)lparam)->code & m[i].filterL) == (UINT)m[i].lParam)) ||
             ((message != WM_NOTIFY) && ((lparam & m[i].filterL) == m[i].lParam))
           )
         )
      {
         if (param || lparam)  /* if one of the params is <> 0, build argument string */
         {
            char msgstr[512];
            CHAR tmp[20];
            PCHAR np = NULL;
            int item;
            HANDLE handle = NULL;

            /* do we have a notification where we have to extract some information ? */
            if (message == WM_NOTIFY)
            {
                UINT code = ((NMHDR *)lparam)->code;

                /* do we have a left mouse click */
                if ( code == NM_CLICK )
                {
                    /* on a tagged List-View control? */
                    if ( (m[i].tag & TAG_CTRLMASK) == TAG_LISTVIEW )
                    {
                        LPNMITEMACTIVATE pIA = (LPNMITEMACTIVATE)lparam;

                        if ( pIA->uKeyFlags == 0 )
                        {
                            strcpy(tmp, "NONE");
                        }
                        else
                        {
                            tmp[0] = '\0';

                            if ( pIA->uKeyFlags & LVKF_SHIFT )
                                strcpy(tmp, "SHIFT");
                            if ( pIA->uKeyFlags & LVKF_CONTROL )
                                tmp[0] == '\0' ? strcpy(tmp, "CONTROL") : strcat(tmp, " CONTROL");
                            if ( pIA->uKeyFlags & LVKF_ALT )
                                tmp[0] == '\0' ? strcpy(tmp, "ALT") : strcat(tmp, " ALT");
                        }
                        np = tmp;

                        /* Don't drop through, use AddDialogMessage here and
                         * return because we need to send 4 args to ooRexx.
                         */

                        _snprintf(msgstr, 511, "%s(%u,%d,%d,\"%s\")", m[i].rexxProgram,
                                  pIA->hdr.idFrom, pIA->iItem, pIA->iSubItem, np);
                        AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                        return ReplyFalse;
                    }
                }
                else if ( code == LVN_ITEMCHANGED )
                {
                    if ( (m[i].tag & TAG_CTRLMASK) == TAG_LISTVIEW )
                    {
                        LPNMLISTVIEW pLV = (LPNMLISTVIEW)lparam;

                        if ( (m[i].tag & TAG_STATECHANGED) && (pLV->uChanged == LVIF_STATE) )
                        {

                            item = pLV->iItem;
                            param = pLV->hdr.idFrom;

                            if ( (m[i].tag & TAG_CHECKBOXCHANGED) && (pLV->uNewState & LVIS_STATEIMAGEMASK) )
                            {
                                np = pLV->uNewState == INDEXTOSTATEIMAGEMASK(2) ? "CHECKED" : "UNCHECKED";
                            }
                            else if ( MatchSelectFocus(m[i].tag, pLV) )
                            {
                                tmp[0] = '\0';

                                if ( SelectionDidChange(pLV) )
                                {
                                    (pLV->uNewState & LVIS_SELECTED) ?
                                        strcpy(tmp, "SELECTED") : strcpy(tmp, "UNSELECTED");
                                }

                                if ( FocusDidChange(pLV) )
                                {
                                    if ( (pLV->uNewState & LVIS_FOCUSED) )
                                        tmp[0] == '\0' ? strcpy(tmp, "FOCUSED") : strcat(tmp, " FOCUSED");
                                    else
                                        tmp[0] == '\0' ? strcpy(tmp, "UNFOCUSED") : strcat(tmp, " UNFOCUSED");
                                }
                                np = tmp;
                            }

                            /* We continue in the 2 following cases to allow a
                             * user to have separate method connections for
                             * selected and focused.
                             */
                            else if ( MatchSelect(m[i].tag, pLV) )
                            {
                                np = (pLV->uNewState & LVIS_SELECTED) ? "SELECTED" : "UNSELECTED";
                                _snprintf(msgstr, 511, "%s(%u,%d,\"%s\")", m[i].rexxProgram, param, item, np);
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                continue;
                            }
                            else if ( MatchFocus(m[i].tag, pLV) )
                            {
                                np = (pLV->uNewState & LVIS_FOCUSED) ? "FOCUSED" : "UNFOCUSED";
                                _snprintf(msgstr, 511, "%s(%u,%d,\"%s\")", m[i].rexxProgram, param, item, np);
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                continue;
                            }
                            else
                            {
                                /* This message in the message table does not
                                 * match, keep searching.
                                 */
                                continue;
                            }
                        }
                    }
                }
                /* do we have an end label edit for tree or list view? */
                else if ((code == TVN_ENDLABELEDIT) && ((TV_DISPINFO *)lparam)->item.pszText)
                {
                    np = ((TV_DISPINFO *)lparam)->item.pszText;
                    handle = ((TV_DISPINFO *)lparam)->item.hItem;
                }
                else if ((code == LVN_ENDLABELEDIT) && ((LV_DISPINFO *)lparam)->item.pszText)
                {
                    np = ((LV_DISPINFO *)lparam)->item.pszText;
                    item = ((LV_DISPINFO *)lparam)->item.iItem;
                }
                /* do we have a tree expand/collapse? */
                else if ((code == TVN_ITEMEXPANDED) || (code == TVN_ITEMEXPANDING))
                {
                    handle = ((NM_TREEVIEW *)lparam)->itemNew.hItem;
                    if (((NM_TREEVIEW *)lparam)->itemNew.state & TVIS_EXPANDED) np = "EXPANDED";
                    else np = "COLLAPSED";
                }
                /* do we have a key_down? */
                else if ((code == TVN_KEYDOWN) || (code == LVN_KEYDOWN) || (code == TCN_KEYDOWN))
                {
                    lparam = (ULONG)((TV_KEYDOWN *)lparam)->wVKey;
                }
                /* do we have a list drag and drop? */
                else if ((code == LVN_BEGINDRAG) || (code == LVN_BEGINRDRAG))
                {
                    item = ((NM_LISTVIEW *)lparam)->iItem;
                    param = ((NMHDR *)lparam)->idFrom;
                    sprintf(tmp, "%d %d", ((NM_LISTVIEW *)lparam)->ptAction.x, ((NM_LISTVIEW *)lparam)->ptAction.y);
                    np = tmp;
                }
                /* do we have a tree drag and drop? */
                else if ((code == TVN_BEGINDRAG) || (code == TVN_BEGINRDRAG))
                {
                    handle = ((NM_TREEVIEW *)lparam)->itemNew.hItem;
                    param = ((NMHDR *)lparam)->idFrom;
                    sprintf(tmp, "%d %d", ((NM_TREEVIEW *)lparam)->ptDrag.x, ((NM_TREEVIEW *)lparam)->ptDrag.y);
                    np = tmp;
                }
                /* do we have a column click in a report? */
                else if (code == LVN_COLUMNCLICK)
                {
                    param = ((NMHDR *)lparam)->idFrom;
                    lparam = (ULONG)((NM_LISTVIEW *)lparam)->iSubItem;  /* which column is pressed */
                }
                else if ( code == BCN_HOTITEMCHANGE )
                {
                    /* Args to ooRexx will be the control ID, entering = true or false. */
                    lparam = (((NMBCHOTITEM *)lparam)->dwFlags & HICF_ENTERING) ? 1 : 0;
                }
            }
            else if ( m[i].tag )
            {
                switch ( m[i].tag & TAG_CTRLMASK )
                {
                    case TAG_DIALOG :
                        switch ( m[i].tag & TAG_FLAGMASK )
                        {
                            case TAG_HELP :
                            {
                                LPHELPINFO phi = (LPHELPINFO)lparam;

                                if ( phi->iContextType == HELPINFO_WINDOW )
                                    np = "WINDOW";
                                else
                                    np = "MENU";

                                /* Use AddDialogMessage directely to send 5 args to ooRexx. */
                                _snprintf(msgstr, 511, "%s(%u,\"%s\",%d,%d,%d)", m[i].rexxProgram,
                                          phi->iCtrlId, np, phi->MousePos.x, phi->MousePos.y, phi->dwContextId);
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                return ReplyFalse;
                            }
                                break;

                            case TAG_CONTEXTMENU :
                            {
                                /* Use AddDialogMessage directely to send 3 args to
                                 * ooRexx. On WM_CONTEXTMENU, if the message is
                                 * generated by the keyboard (say SHIFT-F10) then
                                 * the x and y coordinates are sent as -1 and -1.
                                 * Args to ooRexx: hwnd, x, y
                                 */
                                _snprintf(msgstr, 511, "%s('0x%p',%d,%d)", m[i].rexxProgram, param,
                                          ((int)(short)LOWORD(lparam)), ((int)(short)HIWORD(lparam)));
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                return ((m[i].tag & TAG_MSGHANDLED) ? ReplyTrue : ReplyFalse);
                            }
                                break;

                            case TAG_MENUCOMMAND :
                            {
                                /* Args to ooRexx: index, hMenu
                                 */
                                _snprintf(msgstr, 511, "%s(%d,'0x%p')", m[i].rexxProgram, param, lparam);
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                return ReplyFalse;
                            }
                                break;

                            case TAG_SYSMENUCOMMAND :
                            {
                                /* Args to ooRexx: index, x, y, sysInfo
                                 */
                                int x, y;

                                if ( lparam == -1 )
                                {
                                    x = -1;
                                    y = -1;
                                }
                                else if ( lparam == 0)
                                {
                                    x = 0;
                                    y = 0;
                                }
                                else
                                {
                                    x = ((int)(short)LOWORD(lparam));
                                    y = ((int)(short)HIWORD(lparam));
                                }

                                _snprintf(msgstr, 511, "%s(%d,%d,%d,%d)", m[i].rexxProgram, (param & 0xFFF0), x, y, (param & 0x000F));
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);

                                return ((m[i].tag & TAG_MSGHANDLED) ? ReplyTrue : ReplyFalse);
                            }
                                break;

                            case TAG_MENUMESSAGE :
                            {
                                // Right now there is only WM_INITMENU and WM_INITMENUPOPUP,
                                // but in the future there could be more.  Both
                                // of these messages are handled the exact same
                                // wasy as far as what is sent to ooRexx.

                                /* Args to ooRexx: hMenu as a pointer.  Drat !
                                 * we don't have a context. ;-(
                                 */

                                _snprintf(msgstr, 511, "%s('0x%p')", m[i].rexxProgram, param);
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);

                                return ((m[i].tag & TAG_MSGHANDLED) ? ReplyTrue : ReplyFalse);
                            }
                                break;

                            default :
                                break;

                        }
                        break;

                    default :
                        break;

                }
            }
            else if ( message == WM_HSCROLL || message == WM_VSCROLL)
            {
                _snprintf(msgstr, 511, "%s(%u,\"0x%p\")", m[i].rexxProgram, param, lparam);
                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                return ReplyFalse;
            }

            if (np)
            {
                if ( handle != NULL )
                {
                    _snprintf(msgstr, 511, "%s(%u,\"0x%p\",\"%s\")", m[i].rexxProgram, param, handle, np);
                }
                else
                {
                    _snprintf(msgstr, 511, "%s(%u,%d,\"%s\")", m[i].rexxProgram, param, item, np);
                }
            }
            else
            {
                sprintf(msgstr, "%s(%u,%u)", m[i].rexxProgram, param, lparam);
            }
            AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
         }
         else
         {
             AddDialogMessage((char *)m[i].rexxProgram, addressedTo->pMessageQueue);
         }
         return ReplyFalse;
      }
   return NotMatched;
}


/**
 *
 *
 *
 * @param aDlg
 * @param winMsg
 * @param wmFilter
 * @param wParam
 * @param wpFilter
 * @param lParam
 * @param lpFilter
 * @param prog
 * @param ulTag
 *
 * @return BOOL
 *
 * @remarks  Caller must ensure that 'prog' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.
 */
BOOL AddTheMessage(DIALOGADMIN * aDlg, UINT winMsg, UINT wmFilter, WPARAM wParam, ULONG_PTR wpFilter,
                   LPARAM lParam, ULONG_PTR lpFilter, CSTRING prog, ULONG ulTag)
{
    if ( ! aDlg->MsgTab )
    {
        aDlg->MsgTab = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * MAX_MT_ENTRIES);
        if ( ! aDlg->MsgTab )
        {
            MessageBox(0,"No memory available","Error",MB_OK | MB_ICONHAND);
            return 0;
        }
        aDlg->MT_size = 0;
    }

    if ( aDlg->MT_size < MAX_MT_ENTRIES )
    {
        aDlg->MsgTab[aDlg->MT_size].rexxProgram = (PCHAR)LocalAlloc(LMEM_FIXED, strlen(prog) + 1);
        if ( aDlg->MsgTab[aDlg->MT_size].rexxProgram == NULL )
        {
            return FALSE;
        }
        strcpy(aDlg->MsgTab[aDlg->MT_size].rexxProgram, prog);

        aDlg->MsgTab[aDlg->MT_size].msg = winMsg;
        aDlg->MsgTab[aDlg->MT_size].filterM = wmFilter;
        aDlg->MsgTab[aDlg->MT_size].wParam = wParam;
        aDlg->MsgTab[aDlg->MT_size].filterP = wpFilter;
        aDlg->MsgTab[aDlg->MT_size].lParam = lParam;
        aDlg->MsgTab[aDlg->MT_size].filterL = lpFilter;
        aDlg->MsgTab[aDlg->MT_size].tag = ulTag;

        aDlg->MT_size ++;
        return TRUE;
    }
    else
    {
        MessageBox(0, "Messages have exceeded the maximum number of allocated\n"
                   "table entries. No message can be added.\n",
                   "Error", MB_OK | MB_ICONHAND);
    }
    return FALSE;
}


size_t RexxEntry SendWinMsg(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    LONG i;
    ULONG n[5];
    HWND hWnd;

    CHECKARGL(5);
    hWnd = GET_HWND(argv[1]);

    if ( strcmp(argv[0].strptr, "DLG") == 0 )
    {
        CHECKARG(6);

        for (i=1; i<5; i++)
        {
           if (isHex(argv[i+1].strptr))
               n[i] = strtol(argv[i+1].strptr,'\0',16);
           else
               n[i] = atol(argv[i+1].strptr);
        }

        // TODO need to decide what to do here.  Return is LRESULT which could
        // possibly be a 64-bit number.
        ltoa((long)SendDlgItemMessage((HWND)hWnd, n[1], n[2], (WPARAM)n[3], (LPARAM)n[4]), retstr->strptr, 10);
        retstr->strlength = strlen(retstr->strptr);
        return 0;
    }
    else if (!strcmp(argv[0].strptr,"PTR"))
    {
        LONG ret, lBuffer;
        LPARAM lP;

        CHECKARG(6);

        for (i=0; i<4; i++)
        {
           if (isHex(argv[i+1].strptr))
               n[i] = strtol(argv[i+1].strptr,'\0',16);
           else
               n[i] = atol(argv[i+1].strptr);
        }
        if (isHex(argv[5].strptr)) lP = (LPARAM) strtol(argv[5].strptr,'\0',16);
        else if (argv[5].strptr[0] == 'T') lP = (LPARAM) &argv[5].strptr[1];
        else if (argv[5].strptr[0] == 'L')  /* e.g. used to set tab stops for edit control */
        {
            lBuffer = atol(&argv[5].strptr[1]);
            lP = (LPARAM)&lBuffer;
        }
        else if (argv[5].strptr[0] == 'G')     /* buffered get e.g. to get a text line of an edit control */
        {
            ULONG len = atoi(&argv[5].strptr[1]);
            if (len > retstr->strlength) {
                lP = (LPARAM)GlobalAlloc(GMEM_FIXED, len+1);
                if (!lP) return GetLastError();
                retstr->strptr = (char *)lP;
            }
            else lP = (LPARAM)retstr->strptr;
            memcpy(retstr->strptr, (char *)&len, sizeof(INT));  /* set the buffer size at the beginning of the buffer */
        }
        else
           lP = (LPARAM) atol(argv[5].strptr);

        /* Special handling for this message because it do not wotk for multiple selection lb's */
        if ( n[2] == LB_SETCURSEL )
        {
          // at first check if it is an multiple selection lb
          LONG style = GetWindowLong(GetDlgItem( hWnd, n[1] ), GWL_STYLE);

          if ( style & LBS_MULTIPLESEL )
            if ( argv[5].strptr[0] == 'D' )
              // deselect item in muliple selection lb
              ret = (LONG)SendDlgItemMessage(hWnd, n[1], LB_SETSEL, 0, (LPARAM)n[3]);
            else
              // select item in muliple selection lb
              ret = (LONG)SendDlgItemMessage(hWnd, n[1], LB_SETSEL, 1, (LPARAM)n[3]);
          else
            // select item in single selection lb
            ret = (LONG)SendDlgItemMessage(hWnd, n[1], n[2], (WPARAM)n[3], lP);
        }
        else

          ret = (LONG)SendDlgItemMessage(hWnd, n[1], n[2], (WPARAM)n[3], lP);

       if (argv[5].strptr[0] != 'G')
       {
           ltoa(ret, retstr->strptr, 10);
           retstr->strlength = strlen(retstr->strptr);
       }
       else retstr->strlength = ret;  /* for get text because \0 isn't copied always */
       if (retstr->strlength < 0) retstr->strlength = 0;   /* could be LB_ERR = -1 */
       return 0;
    }
    else if ( strcmp(argv[0].strptr,"ANY") == 0 )
    {
        /* Currently, all SendWinMsg("ANY") calls from ooDialog classes have
         * this format:  SendWinMsg("ANY", handle, msgID, handle, decimal)
         * where the msgID has the hex format.  Handling the decimal as a long
         * works for now.
         */
        LRESULT ret;
        UINT msgID = strtoul(argv[2].strptr, '\0', 16);
        HANDLE wParam = GET_HANDLE(argv[3].strptr);
        LONG lParam = atol(argv[4].strptr);

       ret = SendMessage(hWnd, msgID, (WPARAM)wParam, (LPARAM)lParam);
       if ( ret == 0 )
       {
           RETVAL(0)
       }

       RETHANDLE(ret);
    }
    return 0;
}


/** getDlgMsg()
 *
 *  Retrieves a windows event message from the message queue buffer.
 *
 *  Each Rexx dialog object has a C/C++ string buffer used to store strings
 *  representing window event messages.  The Rexx programmer "connects" a
 *  windows event by supplying a filter to apply to window messages and the name
 *  of a method in the Rexx dialog to invoke when / if the window message is
 *  sent to the underlying Windows dialog.
 *
 *  Each window message sent to the underlying Windows dialog is checked against
 *  the set of message filters.  If a match is found, a string event message is
 *  constructed using the method name and the parameters of the window message.
 *  This string is then placed in the message queue buffer.
 *
 *  On the Rexx side, the Rexx dialog object periodically checks the message
 *  queue using this routine.  If a message is waiting, it is then dispatched to
 *  the Rexx dialog method using interpret.
 *
 *  @param  adm    Pointer to the dialog administration block for the Rexx
 *                 dialog.
 *  @param  peek   [optional]  Whether to just do a message "peek" which returns
 *                 the message but does not remove it.  The default is false.
 *
 *  @return  The next message in the queue, or the empty string if the queue is
 *           empty.
 *
 */
RexxRoutine2(RexxStringObject, getDlgMsg, CSTRING, adm, OPTIONAL_logical_t, doPeek)
{
    DIALOGADMIN * dlgAdm = (DIALOGADMIN *)string2pointer(adm);
    if ( dlgAdm == NULL )
    {
        context->RaiseException1(Rexx_Error_Incorrect_call_user_defined,
                                 context->String("getDlgMsg() argument 1 must not be a null Pointer"));
        return NULLOBJECT;
    }

    char msg[256];
    RexxStringObject result;
    bool peek = doPeek != 0 ? true : false;

    EnterCriticalSection(&crit_sec);

    // Is the dialog admin valid?
    if ( dialogInAdminTable(dlgAdm) )
    {
        getDlgMessage(dlgAdm, msg, peek);
        result = context->String(msg);
    }
    else
    {
        result = context->String(MSG_TERMINATE);
    }
    LeaveCriticalSection(&crit_sec);

    return result;
}


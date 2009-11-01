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
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"


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
   }
   else
   {
       printf("MESSAGE QUEUE OVERFLOW\n");
   }
   return 0;
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


/**
 *
 *
 * @param message
 * @param param
 * @param lparam
 * @param addressedTo
 *
 * @return MsgReplyType
 *
 * @remarks  Pre 4.0.1 the "message" put into the message queue, i.e., the
 *           method invocation string such as onEndTrack(101, 0x00CA23F0), was
 *           used in an interpret command.  Therefore, all string arguments were
 *           enclosed in quotes to prevent errors.  Now, the message string is
 *           used with sendWith(), no interpret is involved.  The string
 *           arguments now must not be enclosed in quotes.
 */
MsgReplyType SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo)
{
   register LONG i = 0;
   MESSAGETABLEENTRY * m = addressedTo->MsgTab;
   static int count = 0;

   if ( m == NULL )
   {
       return NotMatched;
   }

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

                        _snprintf(msgstr, 511, "%s(%u,%d,%d,%s)", m[i].rexxProgram,
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
                                _snprintf(msgstr, 511, "%s(%u,%d,%s)", m[i].rexxProgram, param, item, np);
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                continue;
                            }
                            else if ( MatchFocus(m[i].tag, pLV) )
                            {
                                np = (pLV->uNewState & LVIS_FOCUSED) ? "FOCUSED" : "UNFOCUSED";
                                _snprintf(msgstr, 511, "%s(%u,%d,%s)", m[i].rexxProgram, param, item, np);
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
                                _snprintf(msgstr, 511, "%s(%u,%s,%d,%d,%d)", m[i].rexxProgram,
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
                                _snprintf(msgstr, 511, "%s(0x%p,%d,%d)", m[i].rexxProgram, param,
                                          ((int)(short)LOWORD(lparam)), ((int)(short)HIWORD(lparam)));
                                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                                return ((m[i].tag & TAG_MSGHANDLED) ? ReplyTrue : ReplyFalse);
                            }
                                break;

                            case TAG_MENUCOMMAND :
                            {
                                /* Args to ooRexx: index, hMenu
                                 */
                                _snprintf(msgstr, 511, "%s(%d,0x%p)", m[i].rexxProgram, param, lparam);
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

                                _snprintf(msgstr, 511, "%s(0x%p)", m[i].rexxProgram, param);
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
                _snprintf(msgstr, 511, "%s(%u,0x%p)", m[i].rexxProgram, param, lparam);
                AddDialogMessage((char *)msgstr, addressedTo->pMessageQueue);
                return ReplyFalse;
            }

            if (np)
            {
                if ( handle != NULL )
                {
                    _snprintf(msgstr, 511, "%s(%u,0x%p,%s)", m[i].rexxProgram, param, handle, np);
                }
                else
                {
                    _snprintf(msgstr, 511, "%s(%u,%d,%s)", m[i].rexxProgram, param, item, np);
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


/**
 *  Methods for the .EventNotification mixin class.
 */
#define EVENTNOTIFICATION_CLASS       "EventNotification"


/**
 * The keyboard hook procedure.
 *
 * This is a thread specific hook, not a global hook. This function executes in
 * the same thread as the dialog's window procedure.  The dialog admin structure
 * stores the key press data, the thread ID is used to locate the correct dialog
 * admin.
 *
 * The key is examined to see if the user has set an ooDialog method for it and
 * if it is a key down event.  If so, the key data is sent on to
 * processKeyData() where the actual ooDialog method invocation is set up.  If
 * the user has also set a filter, there may be no method invocation after all.
 *
 */
LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    register int i;
    DWORD id = GetCurrentThreadId();
    DIALOGADMIN * dlgAdm;

    /* No matter what, we need to find the dialog admin struct, it is the only
     * way to get the hook handle.
     */
    for ( i = 0; i < StoredDialogs; i++ )
    {
        if ( DialogTab[i]->threadID == id )
        {
            break;
        }
    }

    /* If we can't find it, there is nothing to do about it.  We can't call the
     * next hook, so just return 0.
     */
    if ( i >= StoredDialogs )
    {
        return 0;
    }

    dlgAdm = DialogTab[i];

    if ( (code == HC_ACTION) && dlgAdm->pKeyPressData->key[wParam] )
    {
        if ( !(lParam & KEY_REALEASE) && !(lParam & KEY_WASDOWN) )
        {
            processKeyPress(dlgAdm->pKeyPressData, wParam, lParam, dlgAdm->pMessageQueue);
        }
    }
	return CallNextHookEx(dlgAdm->hHook, code, wParam, lParam);
}


/**
 * Sets the Windows keyboard hook (WH_KEYBOARD.)  SetWindowsHookEx() has to run
 * in the same thread as the dialog, so a user message is sent to the dialog
 * window procedure to do the actual work.
 *
 * If the hook is not set, all the memory allocation is cleaned up.
 */
static keyPressErr_t setKBHook(DIALOGADMIN *dlgAdm, HWND hDlg)
{
    dlgAdm->hHook = (HHOOK)SendMessage(hDlg, WM_USER_HOOK, (WPARAM)&KeyboardHookProc, (LPARAM)0);
    if ( ! dlgAdm->hHook )
    {
        freeKeyPressData(dlgAdm->pKeyPressData);
        dlgAdm->pKeyPressData = NULL;
        return winAPIErr;
    }
    return noErr;
}


/**
 * Allocates memory for the key press structure and sets up all the data used by
 * the keyboard hook procedure.  Once everything is good the hook is set.
 *
 */
static keyPressErr_t installKBHook(DIALOGADMIN *dlgAdm, HWND hDlg, CSTRING method, CSTRING keys, CSTRING filter)
{
    KEYPRESSDATA *pData;
    LONG        ret = 0;

    pData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
    if ( pData == NULL )
    {
        return memoryErr;
    }

    keyPressErr_t result = setKeyPressData(pData, method, keys, filter);
    if ( result == noErr || result == badFilterErr || result == keyMapErr )
    {
        dlgAdm->pKeyPressData = pData;
    }
    else
    {
        LocalFree(pData);
        return result;
    }

    // Try to retain an existing, non-fatal, error code.
    if ( setKBHook(dlgAdm, hDlg) == winAPIErr )
    {
        result = winAPIErr;
    }
    return result;
}


static keyPressErr_t connectKeyPressHook(RexxMethodContext *c, pCEventNotification pcen, CSTRING methodName,
                                   CSTRING keys, CSTRING filter)
{
    if ( *methodName == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        return nameErr;
    }
    if ( *keys == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        return nameErr;
    }

    HWND hDlg = pcen->hDlg;
    if ( hDlg == NULL || ! IsWindow(hDlg) )
    {
        noWindowsDialogException(c, pcen->rexxSelf);
        return nameErr;
    }

    /* If there is no existing hook, install one and connect the method
     * to it, otherwise connect the method to the existing hook.
     */
    DIALOGADMIN *dlgAdm = pcen->dlgAdm;
    if ( dlgAdm->hHook == NULL )
    {
        return installKBHook(dlgAdm, hDlg, methodName, keys, filter);
    }

    return setKeyPressData(dlgAdm->pKeyPressData, methodName, keys, filter);
}


/**
 * If the hook exists, unhook.  If the key press data exists, free it.
 */
void removeKBHook(DIALOGADMIN *dlgAdm)
{
    if ( dlgAdm->hHook )
    {
        UnhookWindowsHookEx(dlgAdm->hHook);
    }

    freeKeyPressData(dlgAdm->pKeyPressData);
    dlgAdm->hHook = 0;
    dlgAdm->pKeyPressData = NULL;
}


/**
 * Takes a key event that has an ooDialog method connected to it, sets up the
 * method invocation message, and places it in the ooDialog message queue.
 *
 * It is possible for the key event to be filtered out and no ooDialog method is
 * then invoked.
 *
 * The ooDialog event method gets 5 arguments:
 *   key:      decimal value of the key code.
 *   shift:    true / false, true if the shift key was depressed for this event.
 *   control:  true / false, true if control key was depressed.
 *   alt:      true / false, ditto.
 *   info:     Keyword string that specifies if right or left shift / control /
 *             alt were down and the state of the num lock, caps lock, and
 *             scroll lock keys.  The string contains some combination of:
 *
 *             rShift, lShift, rControl lControl, rAlt, lAlt, numOn, numOff,
 *             capsOn, capsOff, scrollOn, scrollOf
 */
void processKeyPress(KEYPRESSDATA *pKeyData, WPARAM wParam, LPARAM lParam, PCHAR pMessageQueue)
{
    /* Method name can not be longer than 197 chars.  This is checked for in
     * setKeyPressData()
     */
    CHAR oodMsg[256];
    BOOL passed = TRUE;
    INT i = pKeyData->key[wParam];
    PCHAR pMethod = pKeyData->pMethods[i];
    KEYFILTER *pFilter = pKeyData->pFilters[i];

    BOOL bShift = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
    BOOL bControl = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;
    BOOL bAlt = (GetAsyncKeyState(VK_MENU) & ISDOWN) ? 1 : 0;

    if ( pFilter )
    {
        if ( pFilter->none )
        {
            passed = !bShift && !bControl && !bAlt;
        }
        else if ( pFilter->and )
        {
            passed = (pFilter->shift ? bShift : !bShift) &&
                     (pFilter->control ? bControl : !bControl) &&
                     (pFilter->alt ? bAlt : !bAlt);
        }
        else
        {
            passed = (pFilter->shift && bShift) ||
                     (pFilter->control && bControl) ||
                     (pFilter->alt && bAlt);
        }
    }

    if ( passed )
    {
        CHAR info[64] = {'\0'};

        if ( GetKeyState(VK_NUMLOCK) & KEY_TOGGLED )
            strcpy(info, "numOn");
        else
            strcpy(info, "numOff");

        if ( GetKeyState(VK_CAPITAL) & KEY_TOGGLED )
            strcat(info, " capsOn");
        else
            strcat(info, " capsOff");

        if ( bShift )
        {
            if ( GetAsyncKeyState(VK_RSHIFT) & ISDOWN )
                strcat(info, " rShift");
            else
                strcat(info, " lShift");
        }
        if ( bControl )
        {
            if ( GetAsyncKeyState(VK_RCONTROL) & ISDOWN )
                strcat(info, " rControl");
            else
                strcat(info, " lControl");
        }
        if ( bAlt )
        {
            if ( GetAsyncKeyState(VK_RMENU) & ISDOWN )
                strcat(info, " rAlt");
            else
                strcat(info, " lAlt");
        }

        if ( GetKeyState(VK_SCROLL) & KEY_TOGGLED )
            strcat(info, " scrollOn");
        else
            strcat(info, " scrollOff");

        sprintf(oodMsg, "%s(%u,%u,%u,%u,%s)", pMethod, wParam, bShift, bControl, bAlt, info);
        AddDialogMessage((char *)oodMsg, pMessageQueue);
    }
}


/**
 * Removes the method at index from the key press data structure.  Assumes that
 * the keyboad hook is unhooked, or that the window subclass is removed.
 *
 */
void removeKeyPressMethod(KEYPRESSDATA *pData, uint32_t index)
{
    uint32_t i;

    if ( index >= 1 && index <= MAX_KEYPRESS_METHODS )
    {
        for ( i = 0; i < COUNT_KEYPRESS_KEYS; i++)
        {
            if ( pData->key[i] == index )
            {
                pData->key[i] = 0;
            }
        }

        if ( pData->pMethods[index] )
        {
            LocalFree(pData->pMethods[index]);
        }
        if ( pData->pFilters[index] )
        {
            LocalFree(pData->pFilters[index]);
        }
        pData->pMethods[index] = NULL;
        pData->pFilters[index] = NULL;
        pData->nextFreeQ[pData->topOfQ] = index;

        /* Theoretically we can't walk off the end of the array, but make sure
         * we don't.
         */
        if ( pData->topOfQ <= MAX_KEYPRESS_METHODS )
        {
            pData->topOfQ++;
        }
        pData->usedMethods--;
    }
}

/**
 * Frees the key press data structure.  Note that methods can be removed leaving
 * holes in the array.
 */
void freeKeyPressData(KEYPRESSDATA *pData)
{
    size_t i;
    if ( pData )
    {
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++ )
        {
            safeLocalFree((void *)pData->pMethods[i]);
            safeLocalFree((void *)pData->pFilters[i]);
        }
        LocalFree((void *)pData);
    }
}

/**
 * Searches the event method table for a matching method.  Returns 0 if no match
 * is found, otherwise the index into the table for the method.
 *
 */
uint32_t seekKeyPressMethod(KEYPRESSDATA *pData, CSTRING method)
{
    uint32_t i;
    uint32_t index = 0;

    if ( pData )
    {
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++)
        {
            if ( pData->pMethods[i] && strcmp(pData->pMethods[i], method) == 0 )
            {
                index = i;
                break;
            }
        }
    }
    return index;
}

/**
 * Parses a key token which could be: a keyword, a single number, or a number
 * range (n-m)  The results are checked to guarentee they are within bounds.
 *
 * Returns true on success, false on error.
 *
 * On true: *pFirst is guarenteed to be non-zero.  If the token represents a
 * single digit, *pLast is guarenteed to be zero.
 *
 * On false: *pFirst and *pLast are undefined.
 */
static BOOL parseKeyToken(PCHAR token, PUINT pFirst, PUINT pLast)
{
    PCHAR  p;
    BOOL   ret = TRUE;

    *pFirst = 0;
    *pLast = 0;

    if ( !strcmp(token, "ALL") )
    {
        *pFirst = VK_LBUTTON;
        *pLast =  VK_OEM_CLEAR;
    }
    else if ( !strcmp(token, "FKEYS") )
    {
        *pFirst = VK_F2;    /* F1 is handled by connectHelp in ooDialog */
        *pLast =  VK_F24;
    }
    else if ( !strcmp(token, "ALPHA") )
    {
        *pFirst = VK_A;
        *pLast =  VK_Z;
    }
    else if ( !strcmp(token, "NUMERIC") )
    {
        *pFirst = VK_0;
        *pLast =  VK_9;
    }
    else if ( !strcmp(token, "ALPHANUMERIC") )
    {
        *pFirst = VK_0;
        *pLast =  VK_Z;
    }
    else if ( (p = strchr(token, '-')) != NULL )
    {
        *p++ = '\0';
        *pFirst = atol(token);
        *pLast = atol(p);
        if ( (! *pFirst || ! *pLast) || (*pFirst > *pLast)       ||
             (*pFirst < VK_LBUTTON)  || (*pFirst > VK_OEM_CLEAR) ||
             (*pLast < VK_LBUTTON)   || (*pLast > VK_OEM_CLEAR)) ret = FALSE;
    }
    else
    {
        *pFirst = atol(token);
        *pLast = 0;
        if ( ! *pFirst || (*pFirst < VK_LBUTTON)   || (*pFirst > VK_OEM_CLEAR) ) ret = FALSE;
    }
    return ret;
}


/**
 *
 * @param pData
 * @param method
 * @param ppMethodName
 *
 * @return keyPressErr_t
 *
 * @remarks There has to be a limit on the length of a method name.  The size of
 *          the message being sent to AddDialogMessage() is set at 256 (for the
 *          key press event.)  Because of the arg string being sent to the
 *          method, this leaves less than that for the method name.
 */
static keyPressErr_t kpCheckMethod(KEYPRESSDATA *pData, CSTRING method, char **ppMethodName)
{
    // We need room, a copy of the method name, and an unique method name.
    if ( pData->usedMethods >= (MAX_KEYPRESS_METHODS) )
    {
        return maxMethodsErr;
    }

    size_t cch = strlen(method);
    if ( cch++ > CCH_METHOD_NAME )
    {
        return nameErr;
    }

    char *tmpName = (char *)LocalAlloc(LPTR, cch);
    if ( tmpName == NULL )
    {
        return memoryErr;
    }

    strcpy(tmpName, method);
    strupr(tmpName);

    if ( seekKeyPressMethod(pData, tmpName) )
    {
        LocalFree(tmpName);
        return dupMethodErr;
    }

    *ppMethodName = tmpName;
    return noErr;
}


static keyPressErr_t kpCheckFilter(CSTRING filter, KEYFILTER **ppFilter)
{
    if ( filter == NULL )
    {
        return noErr;
    }

    KEYFILTER *tmpFilter = (KEYFILTER *)LocalAlloc(LPTR, sizeof(KEYFILTER));
    if ( tmpFilter == NULL )
    {
        return memoryErr;
    }

    if ( StrStrI(filter, "NONE" ) )
    {
        tmpFilter->none = TRUE;
    }
    else
    {
        if ( strstr(filter, "AND"    ) ) tmpFilter->and = TRUE;
        if ( strstr(filter, "SHIFT"  ) ) tmpFilter->shift = TRUE;
        if ( strstr(filter, "CONTROL") ) tmpFilter->control = TRUE;
        if ( strstr(filter, "ALT"    ) ) tmpFilter->alt = TRUE;
    }

    // Some combinations are not filters, so they are ignored.
    if ( ((! tmpFilter->and) && tmpFilter->shift && tmpFilter->control && tmpFilter->alt) ||
         (tmpFilter->and && ! tmpFilter->shift && ! tmpFilter->control && ! tmpFilter->alt) )
    {
        LocalFree(tmpFilter);
        return badFilterErr;
    }

    // Okay, we are good.
    *ppFilter = tmpFilter;
    return noErr;
}

static bool kpMapKeys(KEYPRESSDATA *pData, CSTRING keys, uint32_t index)
{
    uint32_t firstKey, lastKey;
    char *token = NULL;
    char *str = NULL;
    bool success = false;

    str = strdupupr_nospace(keys);
    if ( str != NULL )
    {
        success = true;
        token = strtok(str, ",");
        while( token != NULL )
        {
            if ( parseKeyToken(token, &firstKey, &lastKey) )
            {
                EnterCriticalSection(&crit_sec);
                if ( lastKey )
                {
                    UINT i;
                    for ( i = firstKey; i <= lastKey; i++)
                        pData->key[i] = index;
                }
                else
                {
                    pData->key[firstKey] = index;
                }
                LeaveCriticalSection(&crit_sec);
            }
            else
            {
                success = false;
            }
            token = strtok(NULL, ",");
        }
    }

    safeFree(str);
    return success;
}

/**
 * Set up the key press data for the specified method.
 *
 * It is possible that a key press comes in to the UI thread while this function
 * is executing:
 *
 * The key will have an already existing index or 0 while the method and filter
 * are being set up.  When a key slot is to be updated with the index of the new
 * method, it is protected by a critical section.
 *
 */
keyPressErr_t setKeyPressData(KEYPRESSDATA *pData, CSTRING method, CSTRING keys, CSTRING filter)
{
    char      *pMethod = NULL;
    KEYFILTER *pFilter = NULL;

    keyPressErr_t  result = kpCheckMethod(pData, method, &pMethod);
    if ( result != noErr )
    {
        goto err_out;
    }

    result = kpCheckFilter(filter, &pFilter);
    if ( result == memoryErr )
    {
        goto err_out;
    }

    // In order to map the keys, we need to set the method name into the table
    // and get the index we will use.  So, from here on out, we no longer try to
    // clean up.  Any errors are just ignored, other than returning an error
    // code that will indicate some, or maybe all, of the keys did not get
    // mapped.

    // Get the index of where to put the method.  If the next free queue is not
    // empty, pull the index from the queue, otherwise we are still adding
    //methods sequentially.
    uint32_t index;
    if ( pData->topOfQ )
    {
        index = pData->nextFreeQ[0];
        memmove(pData->nextFreeQ, pData->nextFreeQ + 1, (pData->topOfQ - 1) * sizeof(UINT));
        pData->topOfQ--;
    }
    else
    {
        index = pData->usedMethods + 1;
    }
    pData->pMethods[index] = pMethod;
    pData->usedMethods++;

    if ( pFilter )
    {
        pData->pFilters[index] = pFilter;
    }

    if ( ! kpMapKeys(pData, keys, index) )
    {
        result = keyMapErr;
    }

    goto done_out;

err_out:
    safeFree(pMethod);

done_out:
    return result;
}


bool convert2PointerSize(RexxMethodContext *c, RexxObjectPtr obj, uint64_t *number, int argPos)
{
    if ( obj == NULLOBJECT )
    {
        *number = 0;
        return true;
    }

    if ( c->IsPointer(obj) )
    {
        *number = (uint64_t)c->PointerValue((RexxPointerObject)obj);
        return true;
    }

    return rxStr2Number(c, c->ObjectToStringValue(obj), number, argPos);
}

/** EventNotification::init_eventNotification()
 *
 *  Private method whose sole purpose is to set the cSelf buffer.
 *
 *  This method is invoked from PlainBaseDialog::init() and the
 *  CEventNotification struct is allocated and set up there.
 *
 *  Can not be successfully invoked from Rexx code, by design, because it
 *  requires a Rexx Buffer object as its argument.
 */
RexxMethod1(logical_t, en_init_eventNotification, RexxObjectPtr, cSelf)
{
    RexxMethodContext *c = context;
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
}

RexxMethod4(int32_t, en_connectKeyPress, CSTRING, methodName, CSTRING, keys, OPTIONAL_CSTRING, filter, CSELF, pCSelf)
{
    keyPressErr_t result = connectKeyPressHook(context, (pCEventNotification)pCSelf, methodName, keys, filter);
    if ( result == memoryErr )
    {
        outOfMemoryException(context->threadContext);
    }
    return -(int32_t)result;
}


RexxMethod2(int32_t, en_connectFKeyPress, CSTRING, methodName, CSELF, pCSelf)
{
    keyPressErr_t result = connectKeyPressHook(context, (pCEventNotification)pCSelf, methodName, "FKEYS", NULL);
    if ( result == memoryErr )
    {
        outOfMemoryException(context->threadContext);
    }
    return -(int32_t)result;
}


RexxMethod2(int32_t, en_disconnectKeyPress, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;
    keyPressErr_t result = nameErr;
    char *tmpName = NULL;

    HWND hDlg = pcen->hDlg;
    if ( hDlg == NULL || ! IsWindow(hDlg) )
    {
        noWindowsDialogException(context, pcen->rexxSelf);
        goto done_out;
    }

    DIALOGADMIN *dlgAdm = pcen->dlgAdm;
    if ( dlgAdm->hHook )
    {
        // If there is no methodName argument, remove the entire hook, otherwise
        // disconnect the named method.
        if ( argumentOmitted(1) )
        {
            removeKBHook(dlgAdm);
            result = noErr;
        }
        else
        {
            tmpName = strdupupr(methodName);
            if ( tmpName == NULL )
            {
                result = memoryErr;
                goto done_out;
            }

            uint32_t index = seekKeyPressMethod(dlgAdm->pKeyPressData, tmpName);
            if ( index == 0 )
            {
                result = nameErr;
                goto done_out;
            }

            // If there is only 1 method connected to the hook, remove the hook
            // completely.  Otherwise, unhook the hook, fix up the key press
            //data, and reset the hook.
            if ( dlgAdm->pKeyPressData->usedMethods == 1 )
            {
                removeKBHook(dlgAdm);
                result = noErr;
            }
            else
            {
                UnhookWindowsHookEx(dlgAdm->hHook);
                removeKeyPressMethod(dlgAdm->pKeyPressData, index);
                result = setKBHook(dlgAdm, hDlg);
            }
        }
    }

done_out:
    safeFree(tmpName);
    return -(int32_t)result;
}


RexxMethod2(logical_t, en_hasKeyPressConnection, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    DIALOGADMIN *dlgAdm = pcen->dlgAdm;
    if ( dlgAdm->hHook == NULL )
    {
        return FALSE;
    }

    if ( argumentOmitted(1) )
    {
        return TRUE;
    }

    char *tmpName = strdupupr(methodName);
    if ( tmpName == NULL )
    {
        return FALSE;
    }

    BOOL exists = (seekKeyPressMethod(dlgAdm->pKeyPressData, tmpName) > 0);
    free(tmpName);
    return exists;
}


/** EventNotification::connectCommandEvents()
 *
 *  Connects a Rexx dialog method to the WM_COMMAND event notifications sent by
 *  a Windows dialog control to its parent.
 *
 *  The number of different notifications and the meanings of the notifications
 *  are dependent on the type of dialog control specified.  Therefore, it is
 *  more advisable to use the connectXXXEvent() method for the specific control.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *  @param  methodName  The name of the method to be invoked in the Rexx dialog
 *                      when this event occurs.  The method name can not be the
 *                      empty string.  If the programmer does not provide a
 *                      matching method in the Rexx dialog, a syntax condition
 *                      will be raised if this event happens.
 *
 *  @return 0 on success, -1 for a resource ID error and 1 for other errors.
 *
 *  @details  Two arguments are sent to methodName.  The first contains the
 *            notification code in the high word and the ID of the control in
 *            the low word.  The second argument is the window handle of the
 *            control.
 */
RexxMethod3(int32_t, en_connectCommandEvents, RexxObjectPtr, rxID,  CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return -1;
    }
    if ( *methodName == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        return 1;
    }
    return (AddTheMessage(pcen->dlgAdm, WM_COMMAND, 0xFFFFFFFF, id, 0x0000FFFF, 0, 0, methodName, 0) ? 0 : 1);
}


/** EventNotification::addUserMessage()
 *
 *  Adds a message to the message table.
 *
 *  Each entry in the message table connects a Windows event message to a method
 *  in a Rexx dialog.  The fields for the entry consist of the Windows message,
 *  the WPARAM and LPARAM for the message, a filter for the message and its
 *  parameters, and the method name. Using the proper filters for the Windows
 *  message and its parameters allows the mapping of a very specific Windows
 *  event to the named method.
 *
 *  @param  methodName   [required]  The method name to be connected.
 *  @param  wm           [required]  The Windows event message
 *  @param  _wmFilter    [optional]  Filter applied to the Windows message.  If
 *                       omitted the filter is 0xFFFFFFFF.
 *  @param  wp           [optional]  WPARAM for the message
 *  @param  _wpFilter    [optional]  Filter applied to the WPARAM.  If omitted a
 *                       filter of all hex Fs is applied
 *  @param  lp           [optional]  LPARAM for the message.
 *  @param  _lpFilter    [optional]  Filter applied to LPARAM.  If omitted the
 *                       filter is all hex Fs.
 *  @param  -tag         [optional]  A tag that allows a further differentiation
 *                       between messages.  This is an internal mechanism not to
 *                       be documented publicly.
 *
 *  @return  0 on success, 1 on failure.  One possible source of error is the
 *           message table being full.
 *
 *  @remarks  Although it would make more sense to return true on succes and
 *            false on failure, there is too much old code that relies on 0 for
 *            success and 1 for error.
 */
RexxMethod9(uint32_t, en_addUserMessage, CSTRING, methodName, CSTRING, wm, OPTIONAL_CSTRING, _wmFilter,
            OPTIONAL_RexxObjectPtr, wp, OPTIONAL_CSTRING, _wpFilter, OPTIONAL_RexxObjectPtr, lp, OPTIONAL_CSTRING, _lpFilter,
            OPTIONAL_CSTRING, _tag, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;
    uint32_t result = 1;

    if ( *methodName == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        goto done_out;
    }

    uint32_t winMessage;
    uint32_t wmFilter;
    if ( ! rxStr2Number32(context, wm, &winMessage, 2) )
    {
        goto done_out;
    }

    if ( argumentOmitted(3) )
    {
        wmFilter = 0xFFFFFFFF;
    }
    else
    {
        if ( ! rxStr2Number32(context, _wmFilter, &wmFilter, 3) )
        {
            goto done_out;
        }
    }

    uint64_t number;  // TODO redo this function using oodGetWParam() / oodGetLParam()
                      // Fix those 2 functions to handle optional args. Throw
                      // convert2PointerSize() away

    WPARAM    wParam;
    ULONG_PTR wpFilter;

    if ( ! convert2PointerSize(context, wp, &number, 4) )
    {
        goto done_out;
    }
    wParam = (WPARAM)number;

    if ( argumentOmitted(5) )
    {
        wpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, _wpFilter, &number, 5) )
        {
            goto done_out;
        }
        wpFilter = (number == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)number);
    }

    LPARAM    lParam;
    ULONG_PTR lpFilter;

    if ( ! convert2PointerSize(context, lp, &number, 6) )
    {
        goto done_out;
    }
    lParam = (WPARAM)number;

    if ( argumentOmitted(7) )
    {
        lpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, _lpFilter, &number, 7) )
        {
            goto done_out;
        }
        lpFilter = (number == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)number);
    }

    ULONG tag = 0;
    if ( argumentExists(8) )
    {
        if ( ! rxStr2Number(context, _tag, &number, 8) )
        {
            goto done_out;
        }
        tag = (ULONG)number;
    }

    if ( (winMessage | wParam | lParam) == 0 )
    {
        userDefinedMsgException(context->threadContext, "The wm, wp, and lp arguements can not all be 0" );
    }
    else
    {
        if ( AddTheMessage(pcen->dlgAdm, winMessage, wmFilter, wParam, wpFilter, lParam, lpFilter, methodName, tag) )
        {
            result = 0;
        }
    }

done_out:
    return result;
}



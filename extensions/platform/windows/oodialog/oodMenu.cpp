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
#include "oovutil.h"     // Must be first, includes windows.h and oorexxapi.h
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <limits.h>
#include "APICommon.h"
#include "oodCommon.h"

LPWORD lpwAlign(LPWORD lpIn);
BOOL AddTheMessage(DIALOGADMIN *, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, CSTRING, ULONG);

/**
 * Convenience function to return a menu handle to ooRexx.
 *
 * @param hMenu   Menu handle to return
 * @param retstr  ooRexx return string in which the handle is returned.
 *
 * @return 0, always.
 */
ULONG menuHandleToRexx(HMENU hMenu, PRXSTRING retstr)
{
    sprintf(retstr->strptr, "0x%p", hMenu);
    retstr->strlength = strlen(retstr->strptr);
    return 0;
}

ULONG dwordToRexx(DWORD val, PRXSTRING retstr)
{
    sprintf(retstr->strptr, "%lu", val);
    retstr->strlength = strlen(retstr->strptr);
    return 0;
}

ULONG itemTypeToRexx(UINT type, PRXSTRING retstr)
{
    retstr->strptr[0] = '\0';

    if ( type & MFT_BITMAP )
    {
        strcat(retstr->strptr, "BITMAP");
    }
    else if ( type & MFT_STRING )
    {
        strcat(retstr->strptr, "STRING");
    }
    else
    {
        strcat(retstr->strptr, "SEPARATOR");
    }

    if ( type & MFT_MENUBARBREAK)
    {
        strcat(retstr->strptr, " MENUBARBREAK");
    }
    if ( type & MFT_MENUBREAK )
    {
        strcat(retstr->strptr, " MENUBREAK");
    }
    if ( type & MFT_OWNERDRAW )
    {
        strcat(retstr->strptr, " OWNERDRAW");
    }
    if ( type & MFT_RADIOCHECK )
    {
        strcat(retstr->strptr, " RADIO");
    }
    if ( type & MFT_RIGHTJUSTIFY )
    {
        strcat(retstr->strptr, " RIGHTJUSTIFY");
    }
    if ( type & MFT_RIGHTORDER )
    {
        strcat(retstr->strptr, " RIGHTORDER");
    }

    retstr->strlength = strlen(retstr->strptr);
    return 0;
}

ULONG itemStateToRexx(UINT state, PRXSTRING retstr)
{
    retstr->strptr[0] = '\0';

    if ( state & MFS_CHECKED )
    {
        strcat(retstr->strptr, "CHECKED");
    }
    else
    {
        strcat(retstr->strptr, "UNCHECKED");
    }

    /* Grayed and disabled flags are equal. */
    if ( state & MFS_DISABLED )
    {
        strcat(retstr->strptr, " DISABLED GRAYED");
    }
    else
    {
        strcat(retstr->strptr, " ENABLED");
    }

    if ( state & MFS_DEFAULT )
    {
        strcat(retstr->strptr, " DEFAULT");
    }

    if ( state & MFS_HILITE )
    {
        strcat(retstr->strptr, " HILITE");
    }
    else
    {
        strcat(retstr->strptr, " UNHILITE");
    }

    retstr->strlength = strlen(retstr->strptr);
    return 0;
}

/**
 * Return a valid menu handle.  This function is used with classic style native
 * API.
 *
 * @param str  Menu handle represented as a string
 *
 * @return A validated menu handle, or null.
 */
HMENU getMenuHandle(const char *str)
{
    HMENU hMenu = NULL;

    if ( sscanf(str, "0x%p", &hMenu) == 1 )
    {
        if ( IsMenu(hMenu) == 0 )
        {
            hMenu = NULL;
        }
    }
    return hMenu;
}

HWND getWindowHandle(const char *str)
{
    HWND hWnd = NULL;

    if ( sscanf(str, "0x%p", &hWnd) == 1 )
    {
        if ( IsWindow(hWnd) == 0 )
        {
            hWnd = NULL;
        }
    }
    return hWnd;
}

/**
 * Convert a string to a DWORD value.
 *
 * @param str  String to convert.
 *
 * @return str converted to an unsigned long. Return 0 on detected error.
 */
DWORD getDWord(const char *str)
{
    DWORD val;
    if ( sscanf(str, "%lu", &val) != 1 )
    {
        val = 0;
    }
    return val;
}

// maybe inline
bool _isSubMenu(LPMENUITEMINFO pMii)
{
    if ( pMii->hSubMenu != NULL && pMii->cch == 0 )
    {
        return true;
    }
    return false;
}

bool _isMenuCommandItem(LPMENUITEMINFO pMii)
{
    if ( pMii->hSubMenu == NULL && (pMii->fType & MFT_SEPARATOR) == 0 )
    {
        return true;
    }
    return false;
}

HMENU menuGetHandle(RexxMethodContext * context, RexxObjectPtr self)
{
    HMENU hMenu = (HMENU)rxGetPointerAttribute(context, self, "HMENU");
    if ( IsMenu(hMenu) == 0 )
    {
        TCHAR buf[64];
        _snprintf(buf, sizeof(buf), "The menu handle (0x%p) is not valid at this time", hMenu);
        systemServiceException(context, buf);
        hMenu = NULL;
    }
    return hMenu;
}

RexxObjectPtr menuGetDialogObj(RexxMethodContext *context, RexxObjectPtr menu, RexxObjectPtr dlg, int argPos)
{
    if ( argumentOmitted(argPos) )
    {
        dlg  = context->SendMessage0(menu, "DLG");
        if ( dlg == NULLOBJECT || dlg == context->Nil() )
        {
            context->RaiseException1(Rexx_Error_Incorrect_method_user_defined,
                                     context->NewStringFromAsciiz("This menu has no assigned dialog"));
            return NULLOBJECT;
        }
    }

    if ( ! requiredClass(context, dlg, "BaseDialog", argPos) )
    {
        return NULLOBJECT;
    }
    return dlg;
}

int menuConnectItems(HMENU hMenu, DIALOGADMIN *dlgAdm, CSTRING msg)
{
    DWORD rc = 0;
    int count = GetMenuItemCount(hMenu);

    MENUITEMINFO mii = {0};
    unsigned int miiSize = sizeof(MENUITEMINFO);
    unsigned int miiFMask = MIIM_FTYPE | MIIM_SUBMENU;

    for ( int i = 0; i < count; i++)
    {
        ZeroMemory(&mii, miiSize);
        mii.cbSize = miiSize;
        mii.fMask = miiFMask;

        if ( GetMenuItemInfo(hMenu, i, TRUE, &mii) )
        {
            if ( _isSubMenu(&mii) )
            {
                rc = menuConnectItems(mii.hSubMenu, dlgAdm, msg);
                if ( rc != 0 )
                {
                    return 0;
                }
            }
            else if ( _isMenuCommandItem(&mii) )
            {
                TCHAR buf[256];
                mii.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE;
                mii.cch = sizeof(buf);
                mii.dwTypeData = buf;

                if ( GetMenuItemInfo(hMenu, i, TRUE, &mii) )
                {
                    char *pMsg = (char *)msg;
                    if ( pMsg == NULL )
                    {
                        pMsg = strdupupr_nospace(mii.dwTypeData);
                        if ( ! pMsg )
                        {
                            // need to raise exception
                            return 0;
                        }
                    }
                    //                             WM_COMMAND  msg filt    wparam   filt      lparam f prog tag
                    BOOL s = AddTheMessage(dlgAdm, 0x00000111, 0xFFFFFFFF, mii.wID, 0x0000FFFF,  0,  0, pMsg, 0);

                    if ( pMsg != msg )
                    {
                        safeFree(pMsg);
                    }
                    if ( ! s )
                    {
                        return 1;
                    }
                }
                else
                {
                    return GetLastError();
                }
            }
        }
        else
        {
            return GetLastError();
        }

    }
    return 0;
}

/** Menu::connectAllItems()
 *
 * Connects all menu command items in this menu to a method.
 *
 * @param  msg   OPTIONAL  Connect all menu command items to the method by this
 *               name.  The default is to connect all menu command items to a
 *               method name composed from the text of the command item.
 *
 * @param  _dlg  OPTIONAL  Connect the command items to the method(s) of this
 *               dialog object.  The default is to connect the command items to
 *               the owner dialog of this menu.  An exception is raised if a
 *               valid dialog object can not be resolved.
 *
 * @return       0 on success, 1 if AddTheMessage() failed, system error code on
 *               other errors.
 */
RexxMethod3(int32_t, menu_connectAllItems, OSELF, self, OPTIONAL_CSTRING, msg, OPTIONAL_RexxObjectPtr, _dlg)
{
    HMENU hMenu = menuGetHandle(context, self);
    if ( hMenu == NULL )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr dlg = menuGetDialogObj(context, self, _dlg, 2);
    if ( dlg == NULLOBJECT )
    {
        return NULLOBJECT;
    }

    DIALOGADMIN *dlgAdm = rxGetDlgAdm(context, dlg);
    if ( dlgAdm == NULL )
    {
        return NULLOBJECT;
    }

    return menuConnectItems(hMenu, dlgAdm, msg);
}


/* This method is used as a convenient way to test code. */
RexxMethod2(RexxStringObject, menu_test, OSELF, self, OPTIONAL_CSTRING, id)
{
    return NULLOBJECT;
}

size_t RexxEntry WinMenu(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
   HWND hWnd = NULL;
   HMENU hMenu = NULL;

   CHECKARGL(2);

   if ( strcmp(argv[0].strptr, "LOAD") == 0 )   /* Load a menu resource. */
   {
       INT rc = 0;

       CHECKARGL(4)

       if ( argv[1].strptr[0] == '0' )          /* From TheInstance (ResDialog.) */
       {
           DIALOGADMIN *dlgAdm = (DIALOGADMIN *)string2pointer(&argv[2]);
           if ( dlgAdm == 0 )
           {
               RETVAL(-1)
           }

           hMenu = LoadMenu(dlgAdm->TheInstance, MAKEINTRESOURCE(atoi(argv[3].strptr)));
           if ( hMenu == NULL )
           {
               rc = -(INT)GetLastError();
           }
       }
       else if ( argv[1].strptr[0] == '1' )     /* From a module. */
       {
           HINSTANCE hinst;

           hinst = LoadLibrary(TEXT(argv[2].strptr));
           if ( hinst == NULL )
           {
               rc = -(INT)GetLastError();
           }
           else
           {
               hMenu = LoadMenu(hinst, MAKEINTRESOURCE(atoi(argv[3].strptr)));
               if ( hMenu == NULL )
               {
                   rc = -(INT)GetLastError();
               }

               FreeLibrary(hinst);
           }
       }
       else if ( argv[1].strptr[0] == '2' )     /* From an in-memory template. */
       {
           HANDLE hMem = (HANDLE)GET_HANDLE(argv[2]);
           PVOID *p = (PVOID *)GET_POINTER(argv[3]);

           if ( p == 0 )
           {
               RETVAL(-1)
           }

           hMenu = LoadMenuIndirect(p);
           if ( hMenu == NULL )
           {
               rc = -(INT)GetLastError();
           }

           /* Free the memory allocated for the menu template. */
           GlobalUnlock(hMem);
           GlobalFree(hMem);
       }
       else
       {
           rc = -3;
       }

       if ( rc == 0 )
       {
           return menuHandleToRexx(hMenu, retstr);
       }
       else
       {
           RETVAL(rc)
       }
   }
   else if ( strcmp(argv[0].strptr, "COUNT") == 0 )   /* Returns the count of menu items in a menu. */
   {
       int count;
       hMenu = getMenuHandle(argv[1].strptr);
       if ( hMenu == NULL )
       {
           RETVAL(-3)
       }

       count = GetMenuItemCount(hMenu);
       if ( count == -1 )
       {
           RETVAL(-(INT)GetLastError())
       }

       RETVAL(count)
   }
   else if ( strcmp(argv[0].strptr, "DET") == 0 )     /* Detaches a menu bar from a window. */
   {
       hWnd = getWindowHandle(argv[1].strptr);

       if ( hWnd == NULL )
       {
           RETVAL(-3)
       }

       /* Return the handle of the existing menu. */
       hMenu = GetMenu(hWnd);
       SetMenu(hWnd, NULL);
       return menuHandleToRexx(hMenu, retstr);
   }
   else if ( strcmp(argv[0].strptr, "DEL") == 0 )     /* Releases (deletes) a menu resource. */
   {
       hMenu = getMenuHandle(argv[1].strptr);
       if ( hMenu == NULL )
       {
           RETVAL(-3)
       }

       if ( DestroyMenu(hMenu) == 0 )
       {
           RETVAL(-(INT)GetLastError())
       }

       RETC(0)
   }
   else if ( strcmp(argv[0].strptr, "DELMENU") == 0 )   /* Removes (deletes) a menu item. */
   {
       int  itemID;
       UINT byPosition;

       CHECKARGL(4)

       hMenu = getMenuHandle(argv[1].strptr);
       if ( hMenu == NULL )
       {
           RETVAL(-3)
       }

       itemID = atoi(argv[2].strptr);
       byPosition = argv[3].strptr[0] == '1' ? MF_BYPOSITION : MF_BYCOMMAND;

       if ( DeleteMenu(hMenu, itemID, byPosition) == 0 )
       {
           RETVAL(-(INT)GetLastError())
       }

       RETC(0)
   }
   else if ( strcmp(argv[0].strptr, "ISMENU") == 0 )  /* Is value a valid menu handle? */
   {
       hMenu = getMenuHandle(argv[1].strptr);
       RETVAL(hMenu == NULL ? 0 : 1)
   }
   else if ( strcmp(argv[0].strptr, "DRAW") == 0 )  /* Redraw a menu. */
   {
       hWnd = getWindowHandle(argv[1].strptr);

       if ( hWnd == NULL )
       {
           RETVAL(-3)
       }
       if ( DrawMenuBar(hWnd) == 0 )
       {
           RETVAL(-(INT)GetLastError())
       }
       RETC(0)
   }
   else if ( strcmp(argv[0].strptr, "CREATE") == 0 )  /* Create an empty menu. */
   {
       if ( argv[1].strptr[0] == '0' )
       {
           hMenu = CreateMenu();
       }
       else
       {
           hMenu = CreatePopupMenu();
       }

       if ( hMenu == NULL )
       {
           RETVAL(-(INT)GetLastError())
       }
       return menuHandleToRexx(hMenu, retstr);
   }
   else if ( strcmp(argv[0].strptr, "ASSOC") == 0 )   /* Attaches a menu to a dialog window */
   {
       HMENU oldMenu = NULL;

       CHECKARG(3)

       hMenu = getMenuHandle(argv[1].strptr);
       hWnd = getWindowHandle(argv[2].strptr);

       if ( (hMenu == NULL) || (hWnd == NULL) )
       {
           RETVAL(-3)
       }

       /* Get and return the handle of an existing menu if there is one. */
       oldMenu = GetMenu(hWnd);

       if ( SetMenu(hWnd, hMenu) == 0 )
       {
           RETVAL(-(INT)GetLastError())
       }

       if ( oldMenu == NULL )
       {
           RETC(0)
       }
       else
       {
           return menuHandleToRexx(oldMenu, retstr);
       }
   }
   else if ( strcmp(argv[0].strptr, "CHECKRADIO") == 0 )   /* Check a radio  menu item */
   {
       int  idStart;
       int  idEnd;
       int  idCheck;
       UINT byPosition;

       CHECKARGL(6);

       hMenu = getMenuHandle(argv[1].strptr);
       if ( hMenu == NULL )
       {
           RETVAL(-3)
       }

       idStart = atoi(argv[2].strptr);
       idEnd   = atoi(argv[3].strptr);
       idCheck = atoi(argv[4].strptr);

       byPosition = argv[5].strptr[0] == '1' ? MF_BYPOSITION : MF_BYCOMMAND;

       if ( CheckMenuRadioItem(hMenu, idStart, idEnd, idCheck, byPosition) == 0 )
       {
           RETVAL(-(INT)GetLastError())
       }
       RETC(0)
   }
   RETERR
}


#define MFR_END    0x80
#define MFR_POPUP  0x01

size_t RexxEntry MemMenu(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr )
{
   WORD *p, *pTemplate;
   HANDLE hMem;

   CHECKARGL(1);

   if ( strcmp(argv[0].strptr,"INIT") == 0 )
   {
       INT i = 100;
       DWORD dwHelpID = 0;
       DWORD *dp;

       if ( argc >= 2 )
       {
          i = atoi(argv[1].strptr);
       }
       if ( argc >= 3)
       {
          dwHelpID = atoi(argv[2].strptr);
       }

       hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (i+1)*128);
       if ( hMem == NULL )
       {
           RETVAL(-(INT)GetLastError())
       }

       p = (PWORD)GlobalLock(hMem);

       if ( p == NULL)
       {
           RETVAL(-(INT)GetLastError())
       }

       /* Write the menu header template.  Extended menu headers and extended
        * menut item templates must be DWORD aligned. Begin by aligning the
        * starting pointer.  Note that lpwAlign aligns to a DWORD, not a word.
        */
       p = lpwAlign(p);
       pTemplate = p;

       *p++ = 1;  /* wVersion: 1 for extended menu. */
       *p++ = 4;  /* wOffset:  4 since we are already aligned. */

       /* The context help ID is a DWORD value.  Note this does not work as
        * Microsoft documents it.
        */
       dp = (DWORD *)p;
       *dp++ = dwHelpID;

       /* dp ends up pointing to the start of the 1st menu item, already
        * aligned.
        */
       sprintf(retstr->strptr, "0x%p 0x%p 0x%p", hMem, pTemplate, dp);
       retstr->strlength = strlen(retstr->strptr);
       return 0;
   }
   else if ( strcmp(argv[0].strptr,"ADD") == 0 )
   {
       ULONG lState = MFS_ENABLED | MFS_UNCHECKED;
       ULONG lType = MFT_STRING;
       WORD  lResInfo = 0;
       DWORD dwHelpID = 0;
       DWORD *dp;
       int   nchar;

       CHECKARGL(5);

       p = (WORD *)GET_POINTER(argv[1]);
       if ( p <= 0)
       {
           RETVAL(-1)
       }

       if (strstr(argv[4].strptr, "CHECK")) lState |= MFS_CHECKED;
       if (strstr(argv[4].strptr, "GRAY")) lState |= MFS_GRAYED;
       if (strstr(argv[4].strptr, "DISABLE")) lState |= MFS_DISABLED;
       if (strstr(argv[4].strptr, "HILITE")) lState |= MFS_HILITE;
       if (strstr(argv[4].strptr, "DEFAULT")) lState |= MFS_DEFAULT;

       if (strstr(argv[4].strptr, "POPUP")) lResInfo |= MFR_POPUP;
       if (strstr(argv[4].strptr, "END")) lResInfo |= MFR_END;

       if (strstr(argv[4].strptr, "SEPARATOR")) lType = MFT_SEPARATOR;
       if (strstr(argv[4].strptr, "RIGHT")) lType |= MFT_RIGHTJUSTIFY;
       if (strstr(argv[4].strptr, "RADIO")) lType |= MFT_RADIOCHECK;

       if ( argc > 5 )
       {
           dwHelpID = atoi(argv[5].strptr);
       }
       /* Point to start of extended menu item template. */
       dp = (DWORD *)p;

       *dp++ = lType;                 /* dwType */
       *dp++ = lState;                /* dwState */
       *dp++ = atoi(argv[2].strptr);  /* menuId (DWORD in size)*/

       /* Next fields are WORD in size. */
       p = (WORD *)dp;

       *p++ = lResInfo;               /* bResInfo */

       /* The menu item strings must be unicode.  Total bytes for the in-memory
        * templates is limited.  If the string is longer than some X, simply
        * skip it.  Currently there are 128 bytes allocated per menu item.  In
        * almost all cases, the bytes used will be much smaller, so there is
        * quite a bit of wiggle room here.
        *
        * A string of length 48, will use up 96 bytes.
        */
       if (strlen(argv[3].strptr) >= 48)
       {
           *p++ = 0;
           *p++ = 0;
       }
       else
       {
           nchar = MultiByteToWideChar(CP_ACP, 0, argv[3].strptr, -1, (LPWSTR)p, 48);
           if ( nchar == 0 )
           {
               /* Unlikely that this failed, but if it did, just use an empty
                * string.  The user won't know what it is going on though.
                */
               *p++ = 0;
               *p++ = 0;
           }
           else
           {
               p += nchar;
           }
       }

       /* Need to be double word aligned now. */
       dp = (DWORD *)lpwAlign(p);

       /* The dwHelpId field must be included, even if it is 0, if the item is a
        * popup, must not be included otherwise.
        */
       if (lResInfo & MFR_POPUP)
       {
           *dp++ = dwHelpID;
       }

       RETPTR(dp)
   }

   RETVAL(-1)
}

static UINT checkCommonTypeOpts(const char *opts, UINT type)
{
    if ( strstr(opts, "NOTMENUBARBREAK") != NULL )
    {
        type &= ~MFT_MENUBARBREAK;
    }
    else if ( strstr(opts, "MENUBARBREAK") != NULL )
    {
        type |= MFT_MENUBARBREAK;
    }

    if ( strstr(opts, "NOTMENUBREAK") != NULL )
    {
        type &= ~MFT_MENUBREAK;
    }
    else if ( strstr(opts, "MENUBREAK") != NULL )
    {
        type |= MFT_MENUBREAK;
    }

    if ( strstr(opts, "NOTRIGHTJUSTIFY") != NULL )
    {
        type &= ~MFT_RIGHTJUSTIFY;
    }
    else if ( strstr(opts, "RIGHTJUSTIFY") != NULL )
    {
        type |= MFT_RIGHTJUSTIFY;
    }
    return type;
}

static UINT checkCommonStateOpts(const char *opts, UINT state)
{
    if ( strstr(opts, "NOTDEFAULT") != NULL )
    {
        state &= ~MFS_DEFAULT;
    }
    else if ( strstr(opts, "DEFAULT") != NULL )
    {
        state |= MFS_DEFAULT;
    }
    if ( strstr(opts, "DISABLED") != NULL )
    {
        state |= MFS_DISABLED;
    }
    if ( strstr(opts, "GRAYED") != NULL )
    {
        state |= MFS_GRAYED;
    }
    if ( strstr(opts, "ENABLED") != NULL )
    {
        state &= ~MFS_DISABLED;
    }
    if ( strstr(opts, "UNHILITE") != NULL )
    {
        state &= ~MFS_HILITE;
    }
    else if ( strstr(opts, "HILITE") != NULL )
    {
        state |= MFS_HILITE;
    }
    return state;
}

/**
 * Parses an option string to determine a popup menu's type flags.
 *
 * @param opts Keywords signaling the different MFT_* flags.  These are the
 *             valid keyworkds: MENUBARBREAK MENUBREAK RIGHTJUSTIFY RIGHTORDER
 *
 *             To remove these types from an existing item, use NOT, i.e.
 *             NOTRIGHTORDER will remvoe the MFT_RIGHTORDER flag from a menu
 *             item.
 *
 * @return The combined MFT_* flags for a popup menu.
 */
static UINT getPopupTypeOpts(const char *opts, UINT type)
{
    type = checkCommonTypeOpts(opts, type);
    if ( strstr(opts, "NOTRIGHTORDER") != NULL )
    {
        type &= ~MFT_RIGHTORDER;
    }
    if ( strstr(opts, "RIGHTORDER") != NULL )
    {
        type |= MFT_RIGHTORDER;
    }
    return type;
}

/**
 * Parses an option string to determine a popu menu's state flags.
 *
 * @param opts Keywords signaling the different MFS_* flags.  These are the
 *             valid keyworkds: DEFAULT NOTDEFAULT DISABLED GRAYED ENABLED
 *             HILITE UNHILITE
 *
 * @return The combined MFS_* flags for a popup menu.
 *
 * Note that with extended menus disabled and grared are the same thing.
 */
static UINT getPopupStateOpts(const char *opts, UINT state)
{
    state = checkCommonStateOpts(opts, state);
    return state;
}

/**
 * Parses an option string to determine a menu item's type flags.
 *
 * @param opts Keywords signaling the different MFT_* flags.  These are the
 *             valid keyworkds: MENUBARBREAK MENUBREAK RIGHTJUSTIFY RADIO
 *
 *             To remove these types from an existing item, use NOT, i.e.
 *             NOTRADIO will remvoe the MFT_RADIOCHECK flag from a menu item.
 *
 * @return The combined MFT_* flags for a menu item.
 *
 * Note that RIGHTJUSTIFY is only valid in a menu bar.  Normally menu bars only
 * contain submenus, but menu items are perfectly valid in a menu bar.  If the
 * right justify flag is used in a submenu, it has no effect.
 */
static UINT getItemTypeOpts(const char *opts, UINT type)
{
    type = checkCommonTypeOpts(opts, type);
    if ( strstr(opts, "NOTRADIO") != NULL )
    {
        type &= ~MFT_RADIOCHECK;
    }
    else if ( strstr(opts, "RADIO") != NULL )
    {
        type |= MFT_RADIOCHECK;
    }
    return type;
}

/**
 * Parses an option string to determine a menu item's state flags.
 *
 * @param opts The valid keywords are DEFAULT NOTDEFAULT DISABLED GRAYED ENABLED
 *             UNCHECKED CHECKED HILITE UNHILITE.
 * @param state The parsed state from the options is combined with this state,
 *              pesumably the current state.  Use 0 to get the exact state
 *              specified by the opts string.
 *
 * @return UINT
 *
 * Note that with extended menus grayed and disabled are the same thing.
 */
static UINT getItemStateOpts(const char *opts, UINT state)
{
    state = checkCommonStateOpts(opts, state);

    if ( strstr(opts, "UNCHECKED") != NULL )
    {
        state &= ~MFS_CHECKED;
    }
    else if ( strstr(opts, "CHECKED") != NULL )
    {
        state |= MFS_CHECKED;
    }
    return state;
}


/**
 * InsertMII()  Insert Menu Item Info.  Handles all function that can be done
 * through a call to InsertMenuItem() using a MENUITEMINFO struct.
 *
 *
 */
size_t RexxEntry InsertMII(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
    HMENU hMenu;
    UINT  itemBefore;
    BOOL  byPosition = FALSE;
    MENUITEMINFO mii = {0};

    CHECKARGL(4);

    hMenu = getMenuHandle(argv[1].strptr);
    if ( hMenu == NULL )
    {
        RETVAL(-3)
    }

    itemBefore = (UINT)atoi(argv[2].strptr);
    byPosition = argv[3].strptr[0] == '1' ? TRUE : FALSE;
    mii.cbSize = sizeof(MENUITEMINFO);

    if ( strcmp(argv[0].strptr, "POP") == 0 )
    {
        // Have these args: hPopup, text, id, state options, type options.
        CHECKARGL(9);

        mii.fMask = MIIM_STRING | MIIM_SUBMENU | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.hSubMenu = getMenuHandle(argv[4].strptr);
        mii.dwTypeData = (LPSTR)argv[5].strptr;  // TODO should this be a copy of the string?
        mii.wID = (UINT)atoi(argv[6].strptr);

        if ( argv[7].strlength > 0 )
        {
            mii.fState = getPopupStateOpts(argv[7].strptr, 0);
            mii.fMask |= MIIM_STATE;
        }

        if ( argv[8].strlength > 0 )
        {
            mii.fType |= getPopupTypeOpts(argv[8].strptr, 0);
            mii.fMask |= MIIM_FTYPE;
        }
    }
    else if ( strcmp(argv[0].strptr, "ITEM") == 0 )
    {
        // Have these args: text, id, [state options], [type options].

        mii.fMask = MIIM_STRING | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.dwTypeData = (LPSTR)argv[4].strptr;  // TODO should this be a copy of the string?
        mii.wID = (UINT)atoi(argv[5].strptr);


        if ( argc > 6 && argv[6].strlength > 0 )
        {
            mii.fState = getItemStateOpts(argv[6].strptr, 0);
            mii.fMask |= MIIM_STATE;
        }

        if ( argc > 7 && argv[7].strlength > 0 )
        {
            mii.fType |= getItemTypeOpts(argv[7].strptr, 0);
            mii.fMask |= MIIM_FTYPE;
        }
    }
    else if ( strcmp(argv[0].strptr, "SEP") == 0 )
    {
        mii.fMask = MIIM_FTYPE;
        mii.fType = MFT_SEPARATOR;
    }
    else
    {
        RETVAL(-1)
    }

    if ( InsertMenuItem(hMenu, itemBefore, byPosition, &mii) == 0 )
    {
        RETVAL(-(INT)GetLastError())
    }

    RETC(0)
}

/**
 * SetMII()  Set Menu Item Info.  Handles all function that can be done
 * through a call to SetMenuItemInfo() using a MENUITEMINFO struct.
 *
 *
 */
size_t RexxEntry SetMII(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
    HMENU hMenu;
    UINT  itemID;
    BOOL  byPosition = FALSE;
    MENUITEMINFO mii = {0};

    CHECKARGL(5);

    hMenu = getMenuHandle(argv[1].strptr);
    if ( hMenu == NULL )
    {
        RETVAL(-3)
    }

    itemID = (UINT)atoi(argv[2].strptr);
    byPosition = argv[3].strptr[0] == '1' ? TRUE : FALSE;

    mii.cbSize = sizeof(MENUITEMINFO);

    if ( strcmp(argv[0].strptr, "ID") == 0 )
    {
        mii.fMask = MIIM_ID;
        mii.wID = (UINT)atoi(argv[4].strptr);
    }
    else if ( strcmp(argv[0].strptr, "TEXT") == 0 )
    {
        mii.fMask = MIIM_STRING;
        mii.dwTypeData = (LPSTR)argv[4].strptr;  // TODO should this be a copy of string?
    }
    else if ( strcmp(argv[0].strptr, "ISTATE") == 0 )  /* Item State */
    {
        mii.fMask = MIIM_STATE;

        /* Get the current state and modify it using the option string. */
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        mii.fState = getItemStateOpts(argv[4].strptr, mii.fState);
    }
    else if ( strcmp(argv[0].strptr, "SSTATE") == 0 )  /* SubMenu State */
    {
        mii.fMask = MIIM_STATE;

        /* Get the current state and modify it using the option string. */
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        mii.fState = getPopupStateOpts(argv[4].strptr, mii.fState);
    }
    else if ( strcmp(argv[0].strptr, "ITYPE") == 0 )  /* Item Type */
    {
        mii.fMask = MIIM_FTYPE;

        /* Get the current type and modify it using the option string. */
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        mii.fType = getItemTypeOpts(argv[4].strptr, mii.fType);
    }
    else if ( strcmp(argv[0].strptr, "STYPE") == 0 )  /* SubMenu Type */
    {
        mii.fMask = MIIM_FTYPE;

        /* Get the current type and modify it using the option string. */
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        mii.fType = getPopupTypeOpts(argv[4].strptr, mii.fType);
    }
    else
    {
        RETVAL(-1)
    }

    if ( SetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
    {
        RETVAL(-(INT)GetLastError())
    }

    RETC(0)
}


/**
 * GetMII()  Get Menu Item Info.  Handles all function that can be done through
 * a call to GetMenuItemInfo() using a MENUITEMINFO struct.
 *
 *
 */
size_t RexxEntry GetMII(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
    HMENU hMenu;
    UINT  itemID;
    BOOL  byPosition = FALSE;
    MENUITEMINFO mii = {0};

    CHECKARGL(4);

    hMenu = getMenuHandle(argv[1].strptr);
    if ( hMenu == NULL )
    {
        RETVAL(-3)
    }

    itemID = (UINT)atoi(argv[2].strptr);
    byPosition = argv[3].strptr[0] == '1' ? TRUE : FALSE;

    mii.cbSize = sizeof(MENUITEMINFO);

    if ( strcmp(argv[0].strptr, "ISTATE") == 0 )  /* Get the state of a menu item. */
    {
        mii.fMask = MIIM_STATE;

        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        return itemStateToRexx(mii.fState, retstr);
    }
    else if ( strcmp(argv[0].strptr, "TYPE") == 0 )  /* Get the type of a menu item. */
    {
        mii.fMask = MIIM_FTYPE;

        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        return itemTypeToRexx(mii.fState, retstr);
    }
    else if ( strcmp(argv[0].strptr, "ID") == 0 )  /* Get the resource ID of a menu item. */
    {
        mii.fMask = MIIM_ID;

        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        RETVAL(mii.wID);
    }
    else if ( strcmp(argv[0].strptr, "SUB") == 0 )  /* Get the handle of a submen. */
    {
        mii.fMask = MIIM_SUBMENU;
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        return menuHandleToRexx(mii.hSubMenu, retstr);
    }
    else if ( strcmp(argv[0].strptr, "ISSUB") == 0 )  /* Determine if the item is a submenu. */
    {
        mii.fMask = MIIM_SUBMENU;
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) != 0 )
        {
            if ( mii.hSubMenu != NULL && mii.cch == 0 )
            {
                RETVAL(1)
            }
        }
        else
        {
            /* TODO FIXME remove temp debugging */
            //printf("%s ISSUB last error=%d\n", __FUNCTION__, GetLastError());
        }
        RETVAL(0)
    }
    else if ( strcmp(argv[0].strptr, "ISITEM") == 0 )  /* Determine if the item is not a submenu or separator. */
    {
        mii.fMask = MIIM_SUBMENU | MIIM_FTYPE;
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) != 0 )
        {
            if ( mii.hSubMenu == NULL && (mii.fType & MFT_SEPARATOR) == 0 )
            {
                RETVAL(1)
            }
        }
        else
        {
            /* TODO FIXME remove temp debugging */
            //printf("%s ISITEM last error=%d\n", __FUNCTION__, GetLastError());
        }
        RETVAL(0)
    }
    else if ( strcmp(argv[0].strptr, "ISSEP") == 0 )  /* Determine if the item is a separator. */
    {
        mii.fMask = MIIM_FTYPE;
        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) != 0 )
        {
            if ( (mii.fType & MFT_SEPARATOR) == MFT_SEPARATOR )
            {
                RETVAL(1)
            }
        }
        else
        {
            /* TODO FIXME remove temp debugging */
            // printf("%s ISSEP last error=%d\n", __FUNCTION__, GetLastError());
        }
        RETVAL(0)
    }
    else if ( strcmp(argv[0].strptr, "TEXT") == 0 )
    {
        retstr->strptr[0] = '\0';

        mii.fMask = MIIM_STRING;
        mii.cch = RXAUTOBUFLEN;
        mii.dwTypeData = retstr->strptr;

        if ( GetMenuItemInfo(hMenu, itemID, byPosition, &mii) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }

        retstr->strlength = mii.cch;
        return 0;
    }
    else
    {
        RETVAL(-1)
    }

    RETC(0)
}

/**
 * SetMI()  Set Menu Info.  Handles all function that can be done through a call
 * to SetMenu() using a MENUINFO struct.
 *
 *
 */
size_t RexxEntry SetMI(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
    HMENU hMenu;
    MENUINFO mi = {0};

    CHECKARGL(3);

    hMenu = getMenuHandle(argv[1].strptr);
    if ( hMenu == NULL )
    {
        RETVAL(-3)
    }

    mi.cbSize = sizeof(MENUINFO);

    if ( strcmp(argv[0].strptr, "HELP") == 0 )
    {
        mi.fMask = MIM_HELPID;
        mi.dwContextHelpID = getDWord(argv[2].strptr);

        if ( argc > 3 && argv[3].strptr[0] == '1' )
        {
            mi.fMask |= MIM_APPLYTOSUBMENUS;
        }
    }
    else if ( strcmp(argv[0].strptr, "") == 0 )
    {
        // TODO FIXME what's this about?
    }
    else if ( strcmp(argv[0].strptr, "") == 0 )
    {
        // TODO FIXME what's this about?
    }
    else
    {
        RETVAL(-1)
    }

    if ( SetMenuInfo(hMenu, &mi) == 0 )
    {
        RETVAL(-(INT)GetLastError())
    }

    RETC(0)
}

/**
 * GetMI()  Get Menu Info.  Handles all function that can be done through a call
 * to GetMenu() using a MENUINFO struct.
 *
 *
 */
size_t RexxEntry GetMI(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
    HMENU hMenu;
    MENUINFO mi = {0};

    CHECKARGL(2);

    hMenu = getMenuHandle(argv[1].strptr);
    if ( hMenu == NULL )
    {
        RETVAL(-3)
    }

    mi.cbSize = sizeof(MENUINFO);

    if ( strcmp(argv[0].strptr, "HELP") == 0 )
    {
        mi.fMask = MIM_HELPID;
        if ( GetMenuInfo(hMenu, &mi) == 0 )
        {
            RETVAL(-(INT)GetLastError())
        }
        return dwordToRexx(mi.dwContextHelpID, retstr);
    }
    else if ( strcmp(argv[0].strptr, "") == 0 )
    {
        // TODO FIXME what's this about?
    }
    else if ( strcmp(argv[0].strptr, "") == 0 )
    {
        // TODO FIXME what's this about?
    }
    else
    {
        RETVAL(-1)
    }

    RETC(0)
}

static UINT getTrackHorizontal(const char *opt)
{
    UINT flag = TPM_RIGHTALIGN;

    if ( strstr(opt, "LEFT") != NULL )
    {
        flag = TPM_LEFTALIGN;
    }
    else if ( strstr(opt, "CENTER") != NULL )
    {
        flag = TPM_CENTERALIGN;
    }
    return flag;
}

static UINT getTrackVertical(const char *opt)
{
    UINT flag = TPM_BOTTOMALIGN;

    if ( strstr(opt, "TOP") != NULL )
    {
        flag = TPM_TOPALIGN;
    }
    else if ( strstr(opt, "CENTER") != NULL )
    {
        flag = TPM_VCENTERALIGN;
    }
    return flag;
}

static UINT getTrackMiscOpt(const char *opt)
{
    /*UINT flag = TPM_BOTTOMALIGN;

    if ( strstr(opt, "TOP") != NULL )
    {
        flag = TPM_TOPALIGN;
    }
    else if ( strstr(opt, "CENTER") != NULL )
    {
        flag = TPM_VCENTERALIGN;
    }
    return flag;*/
    return 0;  // for now
}

size_t RexxEntry TrackPopup(const char *f, size_t argc, CONSTRXSTRING *argv, const char *q, RXSTRING *retstr)
{
    UINT flags;
    int ret;
    TRACKPOP tp;
    TPMPARAMS tpm;

    tp.lptpm = &tpm;
    tp.lptpm = NULL; /* The TPMPARAMS arg is not used yet.  TODO FIXME add functionality */

    CHECKARGL(6);

    tp.hMenu = getMenuHandle(argv[1].strptr);
    tp.hWnd = getWindowHandle(argv[2].strptr);
    if ( tp.hMenu == NULL || (tp.hWnd == NULL) )
    {
        RETVAL(-3)
    }

    /* TrackPopupEx expects screen coordinates, be sure to document that and how
     * to use dlg~clientToScreen() to get the right values.  Also note that MS
     * doc on WM_CONTEXTMENU is wrong.  x value is in low word, not high word of
     * LPARAM.  TODO FIXME remove this comment and put into doc.
     */
    tp.point.x = atoi(argv[3].strptr);
    tp.point.y = atoi(argv[4].strptr);

    flags = getTrackHorizontal(argv[5].strptr);
    flags |= getTrackVertical(argv[6].strptr);

    if ( argc > 7 && argv[7].strlength > 0 )
    {
        flags |= getTrackMiscOpt(argv[7].strptr);
    }

    if ( strstr(argv[0].strptr, "RIGHT") != NULL )
    {
        flags |= TPM_RIGHTBUTTON;
    }
    else
    {
        flags |= TPM_LEFTBUTTON;
    }

    if ( strstr(argv[0].strptr, "NONOTIFY") != NULL )
    {
        flags |= TPM_NONOTIFY | TPM_RETURNCMD;
    }
    tp.flags = flags;

    /* Save for debug.   TODO FIXME remove when debugging is done.
    printf("%s hMenu=%p hWnd=%p x=%d y=%d flags=0x%08x lptpm=%p\n", __FUNCTION__,
           tp.hMenu, tp.hWnd, tp.point.x, tp.point.y, tp.flags, tp.lptpm); */

    ret = (int)SendMessage(tp.hWnd, WM_USER_CONTEXT_MENU, (WPARAM)&tp, 0);
    if ( (!(tp.flags & TPM_RETURNCMD)) && ret == 1 )
    {
        ret = 0;
    }
    RETVAL(ret)
}


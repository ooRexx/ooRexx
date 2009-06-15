/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2009 Rexx Language Association. All rights reserved.    */
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

#ifndef oodMenu_Included
#define oodMenu_Included

// These defines are needed / used when creating a menu template in memory.
#define MFR_END    0x80
#define MFR_POPUP  0x01
#define DEFAULT_MENUITEM_COUNT       100
#define ARBITRARY_MENU_ITEM_SIZE     128
#define EXTENDED_MENU_VERSION          1

// The size of the extended menu item template is variable, depending on the
// length of the string part of the template.  The fixed portion contains 4
// DWORDs, 1 WORD, and 1 WCHAR == 5 * 4 bytes
#define MENUEX_TEMPLATE_ITEM_SIZE     20

#define TEMPLATE_TOO_SMALL_MSG       "the number of menu items has exceeded the storage allocated for the menu"
#define CAN_NOT_ATTACH_ON_INIT_MSG   "can not attach menu unless arg 1 'src' is a ResDialog or arg 2 'symbolSrc' is a dialog object"
#define INVALID_MENU_HANDLE_MSG      " menu handle"


inline UINT byPositionFlag(logical_t byPosition)
{
    return byPosition ? MF_BYPOSITION : MF_BYCOMMAND;
}

inline bool _isSubMenu(LPMENUITEMINFO pMii)
{
    return (pMii->hSubMenu != NULL);
}

inline bool _isCommandItem(LPMENUITEMINFO pMii)
{
    return (pMii->hSubMenu == NULL && (pMii->fType & MFT_SEPARATOR) == 0);
}

inline bool _isSeparator(LPMENUITEMINFO pMii)
{
    return (pMii->fType & MFT_SEPARATOR) != 0;
}

typedef enum
{
    BinaryMenuBar       = 0,
    PopupMenu           = 1,
    ScriptMenuBar       = 2,
    UserMenuBar         = 3,
    SystemMenu          = 4,
} MenuType;

typedef struct _mapItem
{
    uint32_t  id;
    char *    methodName;
} MapItem;

class CppMenu
{
    // If there were a friend class it could go here:
    // friend class className;
public:
    CppMenu(RexxObjectPtr s, MenuType t, RexxMethodContext *context);
    ~CppMenu();
    inline void setContext(RexxMethodContext *context, RexxObjectPtr d)
    {
        c = context;
        defaultResult = d;
    }
    inline MapItem  **getConnectionQ() { return connectionQ; }
    inline HMENU getHMenu() { return hMenu; }
    inline void setHMenu(HMENU h) { hMenu = h; }
    inline RexxObjectPtr getSelf() { return self; }
    inline RexxMethodContext *getContext() { return c; }
    inline bool isMenuBar() { return type != PopupMenu && type != SystemMenu; }
    inline bool isPopup() { return type == PopupMenu; }
    inline bool isSystemMenu() { return type == SystemMenu; }
    inline bool isAssigned() { return type == PopupMenu && dlg != TheNilObj; }
    inline bool isAttached() { return isMenuBar() && dlg != TheNilObj && GetMenu(dlgHwnd) == hMenu; }

    inline bool argExists(size_t i) { return (c->arguments[i].flags & ARGUMENT_EXISTS) != 0; }
    inline bool argOmitted(size_t i) { return ! argExists(i); }

    bool menuInit(RexxObjectPtr rxID, RexxObjectPtr symbolSrc, RexxObjectPtr rcFile);
    bool setUpSysMenu(RexxObjectPtr dialog);
    logical_t revertSysMenu();

    bool initTemplate(uint32_t count, uint32_t helpID);
    BOOL addTemplateMenuItem(DWORD menuID, DWORD dwType, DWORD dwState, DWORD dwHelpID, WORD resInfo, CSTRING text);
    bool finishTemplate();
    void deleteTemplate();
    logical_t addTemplateSepartor(RexxObjectPtr rxID, CSTRING opts);
    logical_t addTemplateItem(RexxObjectPtr rxID, CSTRING text, CSTRING opts, CSTRING method);
    logical_t addTemplatePopup(RexxObjectPtr rxID, CSTRING text, CSTRING opts, RexxObjectPtr helpID);
    inline logical_t templateIsComplete() { return isFinal ? TRUE : FALSE; }
    inline void noTempHelpID() { helpID = (uint32_t)-1; }

    // The MENUEX_TEMPLATE_ITEM struct has room for 1 wide charcter, so we don't
    // count the terminating null in text.
    inline bool haveTemplateRoom(int cchWideChar, byte *pos)
    {
        return (pos + MENUEX_TEMPLATE_ITEM_SIZE + (2 * cchWideChar)) <= endOfTemplate;
    }

    RexxDirectoryObject autoConnectionStatus();
    void setAutoConnection(logical_t on, CSTRING methodName);
    BOOL maybeConnectItem(uint32_t id, CSTRING text, logical_t connect, CSTRING methodName);
    logical_t attachToDlg(RexxObjectPtr dialog);
    logical_t assignToDlg(RexxObjectPtr dialog, logical_t autoConnect, CSTRING methodName);
    bool addToConnectionQ(uint32_t id, CSTRING methodName);
    BOOL checkPendingConnections();
    BOOL checkAutoConnect();
    void releaseConnectionQ();
    BOOL detach(bool skipChecks);
    BOOL destroy();

    logical_t connectAllItems(CSTRING msg, RexxObjectPtr dialog, logical_t handles);
    logical_t connectItem(RexxObjectPtr rxID, CSTRING methodName, RexxObjectPtr dialog, logical_t handles);
    logical_t connectSomeItems(RexxObjectPtr, CSTRING, logical_t, RexxObjectPtr, logical_t);
    logical_t connectMenuMessage(CSTRING, CSTRING, HWND, RexxObjectPtr, logical_t);
    uint32_t string2WM(CSTRING keyWord);

    DIALOGADMIN *getAdminBlock(RexxObjectPtr dialog);
    DIALOGADMIN *basicConnectSetup(RexxObjectPtr dialog);
    uint32_t *getAllIDs(RexxObjectPtr rxItemIDs, size_t *items, logical_t byPosition);
    RexxObjectPtr trackPopup(RexxObjectPtr, RexxObjectPtr, CSTRING, logical_t, RexxObjectPtr, bool);
    logical_t getItemText(uint32_t id, logical_t byPosition, char *text, uint32_t cch, MENUITEMINFO *mii);
    void putSysCommands();

    logical_t setMaxHeight(uint32_t height, logical_t recurse);
    int getMaxHeight();

    BOOL maybeRedraw(bool failOnNoDlg);
    void test(CppMenu *other);

    CSTRING name();
    int wID;

protected:
   RexxMethodContext *c;               // our current execution context
   RexxObjectPtr self;                 // our real Rexx object instance
   RexxObjectPtr defaultResult;        // default result

   MenuType type;

   HMENU hMenu;
   RexxObjectPtr dlg;
   DIALOGADMIN *dlgAdm;
   HWND dlgHwnd;

   HANDLE hTemplateMemory;        // Handle to allocated template memory. ("MEMHANDLE")
   DWORD *pTemplate;              // Pointer to aligned, start of template. ("BASEPTR")
   DWORD *pCurrentTemplatePos;    // Pointer template offset for next item addition.  ("CURRENTPTR")
   byte  *endOfTemplate;          // Last allocated byte of template memory
   bool isFinal;
   uint32_t helpID;

   MapItem **connectionQ;
   bool connectionRequested;
   size_t connectionQSize;
   size_t connectionQIndex;

   CSTRING connectionMethod;
   bool autoConnect;
};

#endif

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
 * oodToolBar.cpp
 *
 * Implementation for the ToolBar dialog control.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <shlwapi.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"
#include "oodMessaging.hpp"
#include "oodShared.hpp"


/**
 *  Methods for the .TbButton class.
 */
#define TBBUTTON_CLASS           "TbButton"

#define TBBUTTON_OBJ_MAGIC       "BPwcbLJ1c"
#define TBBUTTON_TEXT_MAX        127
#define TBB_TEXTLEN_ATTRIBUTE    "TBB!!TEXT!!LEN!!ATTRIBUTE"
#define ADD_BITMAP_SRC_OBJ       "Image object, ResourceImage object, or documented keyword"
#define ADDLOAD_SYSTEM_IMAGES    "StdLarge, StdSmall, ViewLarge, ViewSmall, HistLarge, HistSmall, HistNormal, HistHot, HistDisabled, or HistPressed"

enum ImageListType {iltNormal, iltHot, iltDisabled, iltPressed, iltInvalid};

#define TB_ILNORMAL_ATTRIBUTE    "TB!!NORMAL!IMAGELIST"
#define TB_ILHOT_ATTRIBUTE       "TB!!HOT!IMAGELIST"
#define TB_ILDISABLED_ATTRIBUTE  "TB!!DISABLED!IMAGELIST"
#define TB_ILPRESSED_ATTRIBUTE   "TB!!PRESSED!IMAGELIST"

inline CSTRING iltype2attrName(ImageListType t)
{
    switch (t)
    {
        case iltNormal   : return TB_ILNORMAL_ATTRIBUTE;
        case iltHot      : return TB_ILHOT_ATTRIBUTE;
        case iltDisabled : return TB_ILDISABLED_ATTRIBUTE;
        case iltPressed  : return TB_ILPRESSED_ATTRIBUTE;
    }
    return "";
}

inline bool isTbbInternalInit(RexxMethodContext *context, RexxObjectPtr rxCmdID, CSTRING text)
{
    return argumentExists(1) && context->IsBuffer(rxCmdID) && argumentOmitted(2) &&
           argumentExists(3) && strcmp(text, TBBUTTON_OBJ_MAGIC) == 0;
}

/**
 * Converts a string of keywords to the proper TBSTATE_* state flag.
 *
 * @param flags
 *
 * @return uint32_t
 */
uint32_t keyword2tbstate(CSTRING flags)
{
    uint32_t val = 0;

    if (      StrStrI(flags, "CHECKED"       ) != NULL ) val |= TBSTATE_CHECKED       ;
    else if ( StrStrI(flags, "ELLIPSES"      ) != NULL ) val |= TBSTATE_ELLIPSES      ;
    else if ( StrStrI(flags, "ENABLED"       ) != NULL ) val |= TBSTATE_ENABLED       ;
    else if ( StrStrI(flags, "HIDDEN"        ) != NULL ) val |= TBSTATE_HIDDEN        ;
    else if ( StrStrI(flags, "INDETERMINATE" ) != NULL ) val |= TBSTATE_INDETERMINATE ;
    else if ( StrStrI(flags, "MARKED"        ) != NULL ) val |= TBSTATE_MARKED        ;
    else if ( StrStrI(flags, "PRESSED"       ) != NULL ) val |= TBSTATE_PRESSED       ;
    else if ( StrStrI(flags, "WRAP"          ) != NULL ) val |= TBSTATE_WRAP          ;

    return val;
}

/**
 * Converts a set of TBSTATE_* state flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject tbstate2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];  // This is big enough.
    *buf = '\0';

    if ( flags & TBSTATE_CHECKED        ) strcat(buf, "CHECKED"          );
    if ( flags & TBSTATE_ELLIPSES       ) strcat(buf, "ELLIPSES "        );
    if ( flags & TBSTATE_ENABLED        ) strcat(buf, "ENABLED "         );
    if ( flags & TBSTATE_HIDDEN         ) strcat(buf, "HIDDEN "          );
    if ( flags & TBSTATE_INDETERMINATE  ) strcat(buf, "INDETERMINATE "   );
    if ( flags & TBSTATE_MARKED         ) strcat(buf, "MARKED "          );
    if ( flags & TBSTATE_PRESSED        ) strcat(buf, "PRESSED "         );
    if ( flags & TBSTATE_WRAP           ) strcat(buf, "WRAP "            );

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    if ( strlen(buf) == 0 )
    {
        return c->String("nil");
    }
    return c->String(buf);
}

/**
 * Converts a string of keywords to the proper BTNS_* style flag.
 *
 * @param flags
 *
 * @return uint32_t
 */
uint32_t keyword2btns(CSTRING flags)
{
    uint32_t val      = 0;
    char     buf[512] = {'\0'};

    // Add a space to the end of the string, then we can test for "keywored "
    _snprintf(buf, sizeof(buf), "%s ", flags);

    if ( StrStrI(flags, "BUTTON "        ) != NULL ) val |= BTNS_BUTTON        ;
    if ( StrStrI(flags, "SEP "           ) != NULL ) val |= BTNS_SEP           ;
    if ( StrStrI(flags, "CHECK "         ) != NULL ) val |= BTNS_CHECK         ;
    if ( StrStrI(flags, "GROUP "         ) != NULL ) val |= BTNS_GROUP         ;
    if ( StrStrI(flags, "CHECKGROUP "    ) != NULL ) val |= BTNS_CHECKGROUP    ;
    if ( StrStrI(flags, "DROPDOWN "      ) != NULL ) val |= BTNS_DROPDOWN      ;
    if ( StrStrI(flags, "AUTOSIZE "      ) != NULL ) val |= BTNS_AUTOSIZE      ;
    if ( StrStrI(flags, "NOPREFIX "      ) != NULL ) val |= BTNS_NOPREFIX      ;
    if ( StrStrI(flags, "SHOWTEXT "      ) != NULL ) val |= BTNS_SHOWTEXT      ;
    if ( StrStrI(flags, "WHOLEDROPDOWN " ) != NULL ) val |= BTNS_WHOLEDROPDOWN ;

    return val;
}

/**
 * Converts a set of BTNS_* style flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject btns2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    // BTNS_BUTTON == 0
    if ( (flags & BTNS_BUTTON) == BTNS_BUTTON ) strcat(buf, "BUTTON "         );
    if ( flags & BTNS_SEP                     ) strcat(buf, "SEP "            );
    if ( flags & BTNS_CHECK                   ) strcat(buf, "CHECK "          );
    if ( flags & BTNS_GROUP                   ) strcat(buf, "GROUP "          );
    if ( flags & BTNS_CHECKGROUP              ) strcat(buf, "CHECKGROUP "     );
    if ( flags & BTNS_DROPDOWN                ) strcat(buf, "DROPDOWN "       );
    if ( flags & BTNS_AUTOSIZE                ) strcat(buf, "AUTOSIZE "       );
    if ( flags & BTNS_NOPREFIX                ) strcat(buf, "NOPREFIX "       );
    if ( flags & BTNS_SHOWTEXT                ) strcat(buf, "SHOWTEXT "       );
    if ( flags & BTNS_WHOLEDROPDOWN           ) strcat(buf, "WHOLEDROPDOWN "  );

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}

/**
 * Converts a keyword to the proper IDB_* flag.
 *
 * @param keyword
 *
 * @return uint32_t
 */
uint32_t keyword2idb(RexxMethodContext *c, CSTRING keyword, size_t argPos)
{
    uint32_t val = OOD_INVALID_ITEM_ID;

    if (      StrCmpI(keyword, "STDSMALL"      ) == 0 ) val = IDB_STD_SMALL_COLOR      ;
    else if ( StrCmpI(keyword, "STDLARGE"      ) == 0 ) val = IDB_STD_LARGE_COLOR      ;
    else if ( StrCmpI(keyword, "VIEWSMALL"     ) == 0 ) val = IDB_VIEW_SMALL_COLOR     ;
    else if ( StrCmpI(keyword, "VIEWLARGE"     ) == 0 ) val = IDB_VIEW_LARGE_COLOR     ;
    else if ( StrCmpI(keyword, "HISTSMALL"     ) == 0 ) val = IDB_HIST_SMALL_COLOR     ;
    else if ( StrCmpI(keyword, "HISTLARGE"     ) == 0 ) val = IDB_HIST_LARGE_COLOR     ;
    else if ( StrCmpI(keyword, "HISTNORMAL"    ) == 0 ) val = IDB_HIST_NORMAL          ;
    else if ( StrCmpI(keyword, "HISTHOT"       ) == 0 ) val = IDB_HIST_HOT             ;
    else if ( StrCmpI(keyword, "HISTDISABLED"  ) == 0 ) val = IDB_HIST_DISABLED        ;
    else if ( StrCmpI(keyword, "HISTPRESSED"   ) == 0 ) val = IDB_HIST_PRESSED         ;
    else
    {
        wrongArgValueException(c->threadContext, argPos, ADDLOAD_SYSTEM_IMAGES, keyword);
    }

    return val;
}

/**
 * Converts a IDB_* flag to its keyword.
 *
 * @param c
 * @param flag
 *
 * @return A Rexx string object.
 */
 RexxStringObject idb2keyword(RexxMethodContext *c, uint32_t flag)
{
    if ( flag == IDB_STD_SMALL_COLOR    ) return c->String("STDSMALL"         );
    if ( flag == IDB_STD_LARGE_COLOR    ) return c->String("STDLARGE"         );
    if ( flag == IDB_VIEW_SMALL_COLOR   ) return c->String("VIEWSMALL"        );
    if ( flag == IDB_VIEW_LARGE_COLOR   ) return c->String("VIEWLARGE"        );
    if ( flag == IDB_HIST_SMALL_COLOR   ) return c->String("HISTSMALL"        );
    if ( flag == IDB_HIST_LARGE_COLOR   ) return c->String("HISTLARGE"        );
    if ( flag == IDB_HIST_NORMAL        ) return c->String("HISTNORMAL"       );
    if ( flag == IDB_HIST_HOT           ) return c->String("HISTHOT"          );
    if ( flag == IDB_HIST_DISABLED      ) return c->String("HISTDISABLED"     );
    if ( flag == IDB_HIST_PRESSED       ) return c->String("HISTPRESSED"      );
    return c->NullString();
}

/**
 * Constructs the proper bitmap index from the inputs.
 *
 * @param ilID
 * @param offset
 * @param argPos
 *
 * @return A bitmap index on success, on error return OOD_ID_EXCEPTION
 */
int32_t constructBitmapID(RexxMethodContext *context, RexxObjectPtr rxBitmapID, uint32_t ilID,
                          uint32_t offset, size_t argPos)
{
    int32_t index = oodGlobalID(context, rxBitmapID, argPos, false);
    if ( index == OOD_ID_EXCEPTION )
    {
        return OOD_INVALID_ITEM_ID;
    }
    index--;

    if ( index < 0 )
    {
        return index;
    }

    index += offset;
    return MAKELONG(index, ilID);
}

/**
 * Trys to determine if the iString field of the TBBUTTON struct is unicode.
 * This is not foolproof.
 *
 * @param c
 * @param ptbb
 *
 * @return bool
 *
 * @assumes  That iString has aleady been checked by IS_INTRESOURCE and is not a
 *           number.  This will blow up of course if it is an index.
 */
static bool isUnicodeText(RexxMethodContext *c, LPTBBUTTON ptbb)
{
    RexxObjectPtr rxLen = c->GetObjectVariable(TBB_TEXTLEN_ATTRIBUTE);

    size_t len;
    if ( rxLen == NULLOBJECT || (! c->StringSize(rxLen, &len)) )
    {
        return false;
    }

    if ( rxLen == TheZeroObj )
    {
        return true;
    }
    if ( len == 0 )
    {
        // The user set text to ""
        return false;
    }

    char *t = (char *)ptbb->iString;
    if ( (strlen(t) == 1 && len != 1) || strlen(t) < len )
    {
        return true;
    }

    return false;
}

/**
 * Convert the iString field of the TBBUTTON stuct to text, or an index.
 *
 * @param c
 * @param ptbb
 *
 * @return RexxObjectPtr
 *
 * @remarks  We have a problem when using the TBBUTTON struct.  When we use it
 *           to recieve information, the OS retuns the iString text as unicode.
 *           When we set text, we must send it as ANSI.  When the use accesses
 *           the text attribute, we need to return an ANSI string, but there is
 *           no good way to test the INT_PTR to tell if it is unicode or not.
 *
 *           So, we are doing this.  If iString is unicode, then the strlen() of
 *           the pointer cast to (char *) will be shorter than the length of an
 *           ANSI string that the attribute was set to.  Because of '\0' in the
 *           unicode string.  Each time we set the text attribute, we record the
 *           length of the string. If it is an index, or when the TbButton
 *           object is first instantiated, we record the length as 0.  We then
 *           test that here to decide if we have the same ANSI string as the
 *           attribute, or if it has been reset to a unicode string.  This is
 *           not foolproof, but probably adequate.
 */
RexxObjectPtr getTbbText(RexxMethodContext *c, LPTBBUTTON ptbb)
{
    if ( IS_INTRESOURCE(ptbb->iString) )
    {
        return c->UnsignedInt32((uint32_t)ptbb->iString + 1);
    }

    if ( ptbb->iString == NULL || ptbb->iString == -1 )
    {
        return TheNilObj;
    }

    if ( isUnicodeText(c, ptbb) )
    {
        char *text = unicode2ansi((LPWSTR)ptbb->iString);
        if ( text != NULL )
        {
            RexxObjectPtr result = c->String(text);
            safeLocalFree(text);
            return result;
        }
    }

    return c->String((char *)ptbb->iString);
}

/**
 * Sets the text attribute for the TbButton object.
 *
 * However, this attribute can either be an index in the toolbar's internal
 * string pool, or the the actual sting.  This makes things a little difficult.
 *
 * In many of the Win32 APIs when a struct is used to receive information, the
 * caller needs to provide a buffer to recieve the text. But, the MSDN doc
 * doesn't read that way for this API.
 *
 * We are going to assume that if we are setting text, the OS makes a copy of
 * the string.  When we are recieving text we can see that the OS sets iString
 * to a different pointer then we allocated.  So, it makes no sense to allocate
 * a buffer to recieve text.
 *
 * Also, we set an arbitrary length of the text to 260 characters.  This is
 * similar to the maximum length of a string that will be displayed in other
 * controls.
 *
 * @param c
 * @param ptbb
 * @param text
 *
 * @return bool
 */
bool setTbbText(RexxMethodContext *c, LPTBBUTTON ptbb, RexxObjectPtr rxText, size_t argPos)
{
    ValueDescriptor desc   = {0};
    uint16_t        index  = 0xFFFF;
    CSTRING         text   = NULL;

    desc.type = REXX_VALUE_uint16_t;

    if ( c->ObjectToValue(rxText, &desc) )
    {
        index = desc.value.value_uint16_t - 1;
    }
    else
    {
        text = c->ObjectToStringValue(rxText);
    }

    if ( text != NULL && index == 0xFFFF )
    {
        size_t len = strlen(text);
        if ( len > TBBUTTON_TEXT_MAX )
        {
            stringTooLongException(c->threadContext, argPos, TBBUTTON_TEXT_MAX, len);
            return false;
        }

        c->SetObjectVariable(TBB_TEXTLEN_ATTRIBUTE, c->StringSize(len));
        ptbb->iString = (INT_PTR)text;
    }
    else
    {
        c->SetObjectVariable(TBB_TEXTLEN_ATTRIBUTE, TheZeroObj);
        ptbb->iString = index;
    }

    return true;
}

bool setTbbBitmapID(RexxMethodContext *c, LPTBBUTTON ptbb, RexxObjectPtr rxBitmapID, size_t argPos)
{
    ptbb->iBitmap = constructBitmapID(c, rxBitmapID, 0, 0, argPos);
    if ( ptbb->iBitmap == OOD_INVALID_ITEM_ID )
    {
        ptbb->iBitmap = I_IMAGENONE;
        return false;
    }
    return true;
}

/**
 * A TbButton object is not related to a dialog.  The user has to use the global
 * .constDir to use symbolic IDs.
 *
 * @param c
 * @param ptbb
 * @param rxCmdID
 * @param argPos
 *
 * @return bool
 */
bool setTbbCmdID(RexxMethodContext *c, LPTBBUTTON ptbb, RexxObjectPtr rxCmdID, size_t argPos)
{
    ptbb->idCommand = oodGlobalID(c, rxCmdID, argPos, true);
    if ( ptbb->idCommand == OOD_ID_EXCEPTION )
    {
        return false;
    }
    return true;
}


/** TbButton::init()      [Class]
 *
 */
RexxMethod1(RexxObjectPtr, tbb_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, TBBUTTON_CLASS) )
    {
        TheTbButtonClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheTbButtonClass);
    }
    return NULLOBJECT;
}


/** TbButton::init()
 *
 *
 *  @param  cmdID
 *  @param  text
 *  @param  style
 *  @param  state
 *  @param  itemDate
 *  @param  bitmapID
 */
RexxMethod6(RexxObjectPtr, tbb_init, OPTIONAL_RexxObjectPtr, rxCmdID, OPTIONAL_RexxObjectPtr, rxText, OPTIONAL_CSTRING, style,
            OPTIONAL_CSTRING, state, OPTIONAL_RexxObjectPtr, itemData, OPTIONAL_RexxObjectPtr, rxBitmapID)
{
    context->SetObjectVariable(TBB_TEXTLEN_ATTRIBUTE, TheZeroObj);

    if ( isTbbInternalInit(context, rxCmdID, style) )
    {
        context->SetObjectVariable("CSELF", rxCmdID);
        goto done_out;
    }

    RexxBufferObject obj = context->NewBuffer(sizeof(TBBUTTON));
    context->SetObjectVariable("CSELF", obj);

    LPTBBUTTON ptbb = (LPTBBUTTON)context->BufferData(obj);
    memset(ptbb, 0, sizeof(TBBUTTON));

    if ( argumentExists(1) )
    {
        if ( ! setTbbCmdID(context, ptbb, rxCmdID, 1) )
        {
            goto done_out;
        }
    }

    // rxText can be a string or an index.  If the user omitted this arg, we
    // leave things along.  Othewise we have setTbbText sort it out.
    if ( argumentExists(2) && ! setTbbText(context, ptbb, rxText, 2) )
    {
        goto done_out;
    }

    if ( argumentExists(3) )
    {
        ptbb->fsStyle = keyword2btns(style);
    }

    if ( argumentExists(4) )
    {
        ptbb->fsState = keyword2tbstate(state);
    }

    if ( argumentExists(5) )
    {
        ptbb->dwData = (DWORD_PTR)itemData;
    }

    if ( argumentExists(6) )
    {
        setTbbBitmapID(context, ptbb, rxBitmapID, 6);
    }
    else
    {
        ptbb->iBitmap = I_IMAGENONE;
    }

done_out:
    return NULLOBJECT;
}

/** TbButton::bitmapID                [attribute]
 *
 *  We are using 1-based indexes.
 */
RexxMethod1(int32_t, tbb_bitmapID, CSELF, pCSelf)
{
    return ((LPTBBUTTON)pCSelf)->iBitmap + 1;
}
RexxMethod2(RexxObjectPtr, tbb_setBitmapID, RexxObjectPtr, rxBitmapID, CSELF, pCSelf)
{
    setTbbBitmapID(context, (LPTBBUTTON)pCSelf, rxBitmapID, 1);
    return NULLOBJECT;
}

/** TbButton::cmdID                 [attribute]
 */
RexxMethod1(uint32_t, tbb_cmdID, CSELF, pCSelf)
{
    return (uint32_t)((LPTBBUTTON)pCSelf)->idCommand;
}
RexxMethod2(RexxObjectPtr, tbb_setCmdID, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    setTbbCmdID(context, (LPTBBUTTON)pCSelf, rxCmdID, 1);
    return NULLOBJECT;
}

/** TbButton::itemData               [attribute]
 */
RexxMethod1(RexxObjectPtr, tbb_itemData, CSELF, pCSelf)
{
    LPTBBUTTON ptbb = (LPTBBUTTON)pCSelf;
    return ((ptbb->dwData == NULL) ? TheNilObj : (RexxObjectPtr)ptbb->dwData);
}
RexxMethod2(RexxObjectPtr, tbb_setItemData, RexxObjectPtr, userData, CSELF, pCSelf)
{
    ((LPTBBUTTON)pCSelf)->dwData = (DWORD_PTR)userData;
    return NULLOBJECT;
}

/** TbButton::state                   [attribute]
 */
RexxMethod1(RexxStringObject, tbb_state, CSELF, pCSelf)
{
    return tbstate2keyword(context, ((LPTBBUTTON)pCSelf)->fsState);
}
RexxMethod2(RexxObjectPtr, tbb_setState, CSTRING, state, CSELF, pCSelf)
{
    ((LPTBBUTTON)pCSelf)->fsState = keyword2tbstate(state);
    return NULLOBJECT;
}

/** TbButton::style                   [attribute]
 */
RexxMethod1(RexxStringObject, tbb_style, CSELF, pCSelf)
{
    return btns2keyword(context, ((LPTBBUTTON)pCSelf)->fsStyle);
}
RexxMethod2(RexxObjectPtr, tbb_setStyle, CSTRING, style, CSELF, pCSelf)
{
    ((LPTBBUTTON)pCSelf)->fsStyle = keyword2btns(style);
    return NULLOBJECT;
}

/** TbButton::text                   [attribute]
 */
RexxMethod1(RexxObjectPtr, tbb_text, CSELF, pCSelf)
{
    return getTbbText(context, (LPTBBUTTON)pCSelf);
}
RexxMethod2(RexxObjectPtr, tbb_setText, RexxObjectPtr, rxText, CSELF, pCSelf)
{
    setTbbText(context, (LPTBBUTTON)pCSelf, rxText, 1);
    return NULLOBJECT;
}

RexxMethod4(RexxObjectPtr, tbb_assignBitmapID, RexxObjectPtr, rxBitmapID, OPTIONAL_uint32_t, ilID,
            OPTIONAL_uint32_t, offset, CSELF, pCSelf)
{
    LPTBBUTTON ptbb = (LPTBBUTTON)pCSelf;
    ptbb->iBitmap = constructBitmapID(context, rxBitmapID, ilID, offset, 1);
    if ( ptbb->iBitmap == OOD_INVALID_ITEM_ID )
    {
        ptbb->iBitmap = I_IMAGENONE;
    }
    return NULLOBJECT;
}


/**
 * Methods for the ToolBar class.
 */
#define TOOLBAR_CLASS   "ToolBar"



/**
 * Adds some symbolic IDs that can be used when working with a toolbar to the
 * specified constDir.
 *
 * @param c
 * @param constDir
 *
 * @remarks  We are trying to use 1-based indexes for the bitmapID.  So, we add
 *           1 to the system symbolic IDs, so that we can decrement all ids
 *           passed in by the user.
 */
void putToolBarSymbols(RexxMethodContext *c, RexxDirectoryObject constDir)
{
    c->DirectoryPut(constDir, c->Int32(STD_CUT             + 1), "STD_CUT"            );
    c->DirectoryPut(constDir, c->Int32(STD_COPY            + 1), "STD_COPY"           );
    c->DirectoryPut(constDir, c->Int32(STD_PASTE           + 1), "STD_PASTE"          );
    c->DirectoryPut(constDir, c->Int32(STD_UNDO            + 1), "STD_UNDO"           );
    c->DirectoryPut(constDir, c->Int32(STD_REDOW           + 1), "STD_REDOW"          );
    c->DirectoryPut(constDir, c->Int32(STD_DELETE          + 1), "STD_DELETE"         );
    c->DirectoryPut(constDir, c->Int32(STD_FILENEW         + 1), "STD_FILENEW"        );
    c->DirectoryPut(constDir, c->Int32(STD_FILEOPEN        + 1), "STD_FILEOPEN"       );
    c->DirectoryPut(constDir, c->Int32(STD_FILESAVE        + 1), "STD_FILESAVE"       );
    c->DirectoryPut(constDir, c->Int32(STD_PRINTPRE        + 1), "STD_PRINTPRE"       );
    c->DirectoryPut(constDir, c->Int32(STD_PROPERTIES      + 1), "STD_PROPERTIES"     );
    c->DirectoryPut(constDir, c->Int32(STD_HELP            + 1), "STD_HELP"           );
    c->DirectoryPut(constDir, c->Int32(STD_FIND            + 1), "STD_FIND"           );
    c->DirectoryPut(constDir, c->Int32(STD_REPLACE         + 1), "STD_REPLACE"        );
    c->DirectoryPut(constDir, c->Int32(STD_PRINT           + 1), "STD_PRINT"          );
    c->DirectoryPut(constDir, c->Int32(VIEW_LARGEICONS     + 1), "VIEW_LARGEICONS"    );
    c->DirectoryPut(constDir, c->Int32(VIEW_SMALLICONS     + 1), "VIEW_SMALLICONS"    );
    c->DirectoryPut(constDir, c->Int32(VIEW_LIST           + 1), "VIEW_LIST"          );
    c->DirectoryPut(constDir, c->Int32(VIEW_DETAILS        + 1), "VIEW_DETAILS"       );
    c->DirectoryPut(constDir, c->Int32(VIEW_SORTNAME       + 1), "VIEW_SORTNAME"      );
    c->DirectoryPut(constDir, c->Int32(VIEW_SORTSIZE       + 1), "VIEW_SORTSIZE"      );
    c->DirectoryPut(constDir, c->Int32(VIEW_SORTDATE       + 1), "VIEW_SORTDATE"      );
    c->DirectoryPut(constDir, c->Int32(VIEW_SORTTYPE       + 1), "VIEW_SORTTYPE"      );
    c->DirectoryPut(constDir, c->Int32(VIEW_PARENTFOLDER   + 1), "VIEW_PARENTFOLDER"  );
    c->DirectoryPut(constDir, c->Int32(VIEW_NETCONNECT     + 1), "VIEW_NETCONNECT"    );
    c->DirectoryPut(constDir, c->Int32(VIEW_NETDISCONNECT  + 1), "VIEW_NETDISCONNECT" );
    c->DirectoryPut(constDir, c->Int32(VIEW_NEWFOLDER      + 1), "VIEW_NEWFOLDER"     );
    c->DirectoryPut(constDir, c->Int32(VIEW_VIEWMENU       + 1), "VIEW_VIEWMENU"      );
    c->DirectoryPut(constDir, c->Int32(HIST_BACK           + 1), "HIST_BACK"          );
    c->DirectoryPut(constDir, c->Int32(HIST_FORWARD        + 1), "HIST_FORWARD"       );
    c->DirectoryPut(constDir, c->Int32(HIST_FAVORITES      + 1), "HIST_FAVORITES"     );
    c->DirectoryPut(constDir, c->Int32(HIST_ADDTOFAVORITES + 1), "HIST_ADDTOFAVORITES");
    c->DirectoryPut(constDir, c->Int32(HIST_VIEWTREE       + 1), "HIST_VIEWTREE"      );
    c->DirectoryPut(constDir, c->Int32(I_IMAGECALLBACK     + 1), "I_IMAGECALLBACK"    );
    c->DirectoryPut(constDir, c->Int32(I_IMAGENONE         + 1), "I_IMAGENONE"        );
}

/**
 * Stores the Rexx image list in an attribute to prevent it from GC.
 *
 * For the normal image list, the toolbar allows the user to use multiple image
 * lists by indexing them.  There is no requirement that I see that the indexes
 * be consecutive, MSDN calls them IDs.  Because of this we use a table to store
 * then in rather than an array.
 *
 * This function both sets and gets an existing image list.  Both the set and the
 * get functions always return the existing image list.  If there is no
 * existing, .nil is returned.
 *
 * @param c         Method context we are executing in.
 * @param il        Rexx image list object.
 * @param ilIndex   Numeric index value
 * @param set       If true, set il as the value of the attribut.  If false,
 *                  ignore il and only return the existing.
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr setGetNormalImageList(RexxMethodContext *c, RexxObjectPtr il, uint32_t ilIndex, bool set)
{
    RexxObjectPtr existing = TheNilObj;
    RexxObjectPtr index    = c->UnsignedInt32(ilIndex);
    CSTRING       atrName  = iltype2attrName(iltPressed);

    RexxObjectPtr table = c->GetObjectVariable(atrName);
    if ( table == NULLOBJECT )
    {
        if ( ! set )
        {
            goto done_out;
        }

        table = rxNewBuiltinObject(c, "TABLE");
        if ( table == NULLOBJECT )
        {
            goto done_out;
        }
        c->SetObjectVariable(atrName, table);
        c->SendMessage2(table, "PUT", il, index);
        goto done_out;
    }

    existing = c->SendMessage1(table, "AT", index);
    printf("Table::at() existing=%p is .nil? %s", existing, existing == TheNilObj ? "true" : "false");
    if ( set )
    {
        c->SendMessage2(table, "PUT", il, index);
    }

done_out:
    return existing;
}

/**
 * Used as generic code to get one of the 4 image lists the toolbar uses.
 *
 * @param context    Method context we are executing in.
 *
 * @param ilType     Identifies which image list to set.
 * @param ilID       Image list identifier, only for the normal image list
 * @param pCSelf     DialogControl CSelf
 *
 * @return  The specified image list, if one is set, otherwise .nil.
 *
 * @assumes   The caller only passes in a valid ilType.
 */
RexxObjectPtr getImageList(RexxMethodContext *context, ImageListType ilType, RexxObjectPtr ilID, void *pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ilType == iltNormal )
    {
        int32_t ilIndex = 0;
        if ( argumentExists(1) )
        {
            ilIndex = oodGlobalID(context, ilID, 2, false);
            if ( ilIndex == OOD_ID_EXCEPTION )
            {
                goto done_out;
            }
            else if ( ilIndex == -1 )
            {
                wrongArgValueException(context->threadContext, 1, "a non-negative numeric ID or a valid symbolic ID", ilID);
                goto done_out;
            }
        }

        result = setGetNormalImageList(context, NULLOBJECT, ilIndex, false);
    }
    else
    {
        result = context->GetObjectVariable(iltype2attrName(ilType));
    }

done_out:
    return result == NULLOBJECT ? TheNilObj : result;
}

/**
 * Used as generic code to set one of the 4 image lists the toolbar uses.
 *
 * @param context    Method context we are executing in.
 *
 * @param ilType     Identifies which image list to set.
 * @param il         .ImageList object
 * @param ilID       Image list identifier, only for the normal image list
 * @param pCSelf     DialogControl CSelf
 *
 * @return RexxObjectPtr
 *
 * @assumes   The caller only passes in a valid ilType.
 */
RexxObjectPtr setImageList(RexxMethodContext *context, ImageListType ilType, RexxObjectPtr il, RexxObjectPtr ilID, void *pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    RexxObjectPtr imageList = NULLOBJECT;
    HIMAGELIST    himl      = NULL;
    HWND          hTb       = pcdc->hCtrl;

    if ( il == TheNilObj )
    {
        ; // Do nothing, this is fine, removes the current image list, if any.
    }
    else if ( context->IsOfType(il, "IMAGELIST") )
    {
        himl = rxGetImageList(context, il, 1);
        if ( himl == NULL )
        {
            goto done_out;
        }
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, "ImageList or .nil", il);
        goto done_out;
    }

    switch ( ilType )
    {
        case iltNormal :
        {
            int32_t ilIndex = 0;
            if ( argumentExists(2) )
            {
                ilIndex = oodGlobalID(context, ilID, 2, false);
                if ( ilIndex == OOD_ID_EXCEPTION )
                {
                    goto done_out;
                }
                else if ( ilIndex == -1 )
                {
                    wrongArgValueException(context->threadContext, 2, "a non-negative numeric ID or a valid symbolic ID", ilID);
                    goto done_out;
                }
            }

            SendMessage(hTb, TB_SETIMAGELIST, ilIndex, (LPARAM)himl);
            result = setGetNormalImageList(context, il, ilIndex, true);
        }
            break;

        case iltHot :
            SendMessage(hTb, TB_SETHOTIMAGELIST, 0, (LPARAM)himl);
            result = rxSetObjVar(context, iltype2attrName(iltHot), il);
            break;

        case iltDisabled :
            SendMessage(hTb, TB_SETDISABLEDIMAGELIST, 0, (LPARAM)himl);
            result = rxSetObjVar(context, iltype2attrName(iltDisabled), il);
            break;

        case iltPressed :
            SendMessage(hTb, TB_SETPRESSEDIMAGELIST, 0, (LPARAM)himl);
            result = rxSetObjVar(context, iltype2attrName(iltPressed), il);
            break;

        default :
            // Can't happen unless our code is screwed up.
            break;
    }

done_out:
    return result;
}

/**
 * Converts a string of keywords to the proper TBSTYLE_EX_* extended style flag.
 *
 * @param flags
 *
 * @return uint32_t
 */
uint32_t keyword2tbsEx(CSTRING flags)
{
    uint32_t val      = 0;
    char     buf[512] = {'\0'};

    if ( StrStrI(flags, "DRAWDDARROWS"        ) != NULL ) val |= TBSTYLE_EX_DRAWDDARROWS         ;
    if ( StrStrI(flags, "HIDECLIPPEDBUTTONS"  ) != NULL ) val |= TBSTYLE_EX_HIDECLIPPEDBUTTONS   ;
    if ( StrStrI(flags, "MIXEDBUTTONS"        ) != NULL ) val |= TBSTYLE_EX_MIXEDBUTTONS         ;
    if ( StrStrI(flags, "DOUBLEBUFFER"        ) != NULL ) val |= TBSTYLE_EX_DOUBLEBUFFER         ;

    return val;
}

/**
 * Converts a set of TBSTYLE_EX_* style flags to their keyword string.
 *
 * @param c
 * @param flags
 *
 * @return A Rexx string object.
 */
 RexxStringObject tbsEx2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[256];
    *buf = '\0';

    if ( flags & TBSTYLE_EX_DRAWDDARROWS        ) strcat(buf, "DRAWDDARROWS"        );
    if ( flags & TBSTYLE_EX_HIDECLIPPEDBUTTONS  ) strcat(buf, "HIDECLIPPEDBUTTONS"  );
    if ( flags & TBSTYLE_EX_MIXEDBUTTONS        ) strcat(buf, "MIXEDBUTTONS"        );
    if ( flags & TBSTYLE_EX_DOUBLEBUFFER        ) strcat(buf, "DOUBLEBUFFER"        );

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}

RexxObjectPtr rxTbbFromTbb(RexxThreadContext *c, TBBUTTON *pTbb)
{
    RexxObjectPtr    result   = TheNilObj;
    RexxBufferObject rxTbbBuf = c->NewBuffer(sizeof(TBBUTTON));
    if ( rxTbbBuf == NULLOBJECT )
    {
        outOfMemoryException(c);
        goto done_out;
    }

    LPTBBUTTON ptbbBuff = (LPTBBUTTON)c->BufferData(rxTbbBuf);
    memcpy(ptbbBuff, pTbb, sizeof(TBBUTTON));

    RexxArrayObject args = c->NewArray(3);
    c->ArrayPut(args, rxTbbBuf, 1);
    c->ArrayPut(args, c->String(TBBUTTON_OBJ_MAGIC), 3);

    RexxObjectPtr rxTbButton = c->SendMessage(TheTbButtonClass, "NEW", args);
    if ( rxTbButton != NULLOBJECT )
    {
        result = rxTbButton;
    }

done_out:
    return result;
}

void genericTbnInvoke(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, RexxArrayObject args, uint32_t tag)
{
    switch ( tag & TAG_EXTRAMASK )
    {
        case TAG_REPLYFROMREXX :
            invokeDirect(c, pcpbd, methodName, args);
            break;

        case TAG_SYNC :
            invokeSync(c, pcpbd, methodName, args);
            break;

        default :
            invokeDispatch(c, pcpbd, methodName, args);
            break;
    }
}


MsgReplyType tbnDeletingButton(RexxThreadContext *c, CSTRING methodName, uint32_t tag, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr   idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr   rxTB   = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winToolBar, true);

    LPNMTOOLBAR nmtb = (LPNMTOOLBAR)lParam;

    RexxObjectPtr   cmdID = c->UnsignedInt32(nmtb->iItem);
    RexxArrayObject args  = c->ArrayOfThree(idFrom, cmdID, rxTB);

    genericTbnInvoke(c, pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(cmdID);
    c->ReleaseLocalReference(rxTB);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


/**
 * Handles the TBN_GETBUTTONINFO notification.  This is sent from the toolbar
 * customization dialog to collect the information for the buttons it puts in
 * the dialog.
 *
 * The user is suppossed to fill in the TBBUTTON struct with the information for
 * a button and return true.  The dialog keeps sending the message until false
 * is returned.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks  The OS allocates a buffer and sends a point to it, along with the
 *           size of the buffer.  On Win7, it has always been 128, so we
 *           restrict the ToolBar text limit to 127.  Need to check if it is 128
 *           on XP.
 *
 *           We have the user sent the iString attribute in the TBUTTON sent to
 *           them, and then here we copy it to the system buffer.  (Hmm, what if
 *           the user is using the string pool?)
 *
 *           In NMTOOLBAR, the rcButton is always all 0's or garbage.  In
 *           tbButton field, the dwData and iString fields are always garbage
 *           and the others 0's. Although the fState field seems to increment by
 *           1 each time though.  We just set everything in the tbButton struct
 *           to zero, and then set the
 */
MsgReplyType tbnGetButtonInfo(RexxThreadContext *c, CSTRING methodName, uint32_t tag, LPARAM lParam,
                              pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxTB   = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winToolBar, true);

    LPNMTOOLBAR nmtb = (LPNMTOOLBAR)lParam;

    memset(&nmtb->tbButton, 0, sizeof(TBBUTTON));
    nmtb->tbButton.iBitmap  = I_IMAGENONE;

    RexxObjectPtr index   = c->Int32(nmtb->iItem + 1);
    RexxObjectPtr textLen = c->Int32(nmtb->cchText - 1);
    RexxObjectPtr rxTbb   = rxTbbFromTbb(c, &nmtb->tbButton);

    RexxArrayObject args = c->ArrayOfFour(idFrom, index, textLen, rxTbb);
    c->ArrayPut(args, rxTB, 5);

    // The Rexx programmer returns .true, to indicate the tbButton struct is
    // filled in, and then false to indicate she is has no more buttons.
    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
    if ( msgReply == TheTrueObj )
    {
        TBBUTTON *tbb = (TBBUTTON *)c->ObjectToCSelf(rxTbb);
        memcpy(&nmtb->tbButton, tbb, sizeof(TBBUTTON));

        strcpy(nmtb->pszText, (char *)tbb->iString);
        tbb->iString = NULL;
        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, TRUE);
    }
    else if ( msgReply == TheFalseObj )
    {
        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, FALSE);
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(index);
    c->ReleaseLocalReference(textLen);
    c->ReleaseLocalReference(rxTbb);
    c->ReleaseLocalReference(rxTB);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


MsgReplyType tbnInitCustomize(RexxThreadContext *c, CSTRING methodName, uint32_t tag, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxTB   = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winToolBar, true);
    RexxArrayObject args = c->ArrayOfTwo(idFrom, rxTB);

    // The Rexx programmer returns .true, to indicate the help button should be
    // shown, or false, it shouldn'be shown.  The return to Windows is
    // TBNRF_HIDEHELP to hide the button.
    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
    if ( msgReply == TheFalseObj )
    {
        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, TBNRF_HIDEHELP);
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxTB);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


/**
 * Handles the TBN_QUERYDELETE and TBN_QUERYINSERT notifications.
 *
 * The user returns true to allow the delete or insert and false to disallow it.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param lParam
 * @param pcpbd
 * @param code
 *
 * @return MsgReplyType
 *
 * @remarks  In the NMTOOLBAR struct pszText, rcButton, and cchText are not
 *           valid, we ignore them.
 */
MsgReplyType tbnQuery(RexxThreadContext *c, CSTRING methodName, uint32_t tag, LPARAM lParam,
                      pCPlainBaseDialog pcpbd, uint32_t code)
{
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr rxTB   = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winToolBar, true);

    LPNMTOOLBAR nmtb = (LPNMTOOLBAR)lParam;

    RexxObjectPtr index  = c->Int32(nmtb->iItem + 1);
    RexxObjectPtr rxTbb  = rxTbbFromTbb(c, &nmtb->tbButton);
    RexxArrayObject args = c->ArrayOfFour(idFrom, index, rxTbb, rxTB);

    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
    if ( msgReply == TheTrueObj )
    {
        setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, TRUE);
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(index);
    c->ReleaseLocalReference(rxTbb);
    c->ReleaseLocalReference(rxTB);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


MsgReplyType tbnSimple(RexxThreadContext *c, CSTRING methodName, uint32_t tag, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr nCode  = c->UnsignedInt32(((NMHDR *)lParam)->code);
    RexxObjectPtr rxTB   = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winToolBar, true);
    RexxArrayObject args = c->ArrayOfThree(idFrom, nCode, rxTB);

    genericTbnInvoke(c, pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(nCode);
    c->ReleaseLocalReference(rxTB);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


/** ToolBar::addBitmap()
 *
 *  Adds one or more images to the list of button images available for a
 *  toolbar.
 *
 *  @param  src  [required] Source of the bitmap.  This can be a ResourceImage
 *               object, a bitmap .Image object, or the special keyword
 *               hInstCommCtrl, case not significant.
 *
 *  @param count [optional] The count / number of images in the bitmap.  If src
 *               is an Image or a ResourceImage, then count is required. If src
 *               is hInstCommCtrl, then count is ignored and can be omitted.
 *
 *  @param id    [optional] The resource ID for the image. id is optional and
 *               ignored if src is an Image object.  It is required for a
 *               ResourceImage or hInstCommCtrl.
 *
 *               For a ResourceImage, id may be numeric or symbolic.  To use a
 *               symbolic ID, the symbol must be in the global .constDir.
 *
 *               For hInstCommCtrl id must be one of the four following
 *               keywords, case is not significant: STDLARGE, STDSMALL,
 *               VIEWLARGE, or VIEWSMALL.
 *
 *  @return  The index of the first new image, or -1 on error.
 *
 *  @note  If an Image object is the source, do not release the image until
 *         after the dialog is ended, or the replaceBitmap() method has been
 *         used to replace the bitmap.  Otherwise the tool bar is destroyed.
 */
RexxMethod4(int32_t, tb_addBitmap, RexxObjectPtr, src, OPTIONAL_uint32_t, count, OPTIONAL_RexxObjectPtr, rxID,
            CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    TBADDBITMAP tbab   = { 0 };
    int32_t     result = -1;

    if ( c->IsOfType(src, "IMAGE") )
    {
        if ( argumentOmitted(2) )
        {
            missingArgException(context, 2);
            goto done_out;
        }

        POODIMAGE poi = rxGetImageBitmap(context, src, 1);
        if ( poi == NULL )
        {
            goto done_out;
        }
        if ( ! poi->isValid )
        {
            nullObjectException(context->threadContext, "Image", 1);
            goto done_out;
        }
        tbab.nID = (UINT_PTR)poi->hImage;
    }
    else if ( c->IsOfType(src, "RESOURCEIMAGE") )
    {
        if ( argumentOmitted(2) )
        {
            missingArgException(context, 2);
            goto done_out;
        }
        if ( argumentOmitted(3) )
        {
            missingArgException(context, 3);
        }

        PRESOURCEIMAGE pri = rxGetResourceImage(context, src, 1);
        if ( pri == NULL )
        {
            goto done_out;
        }
        if ( ! pri->isValid )
        {
            nullObjectException(context->threadContext, "ResourceImage", 1);
            goto done_out;
        }

        tbab.nID = oodGlobalID(context, rxID, 3, true);
        if ( tbab.nID == OOD_ID_EXCEPTION )
        {
            goto done_out;
        }
        tbab.hInst = pri->hMod;
    }
    else
    {
        CSTRING srcWord = c->ObjectToStringValue(src);
        if ( StrCmpI(srcWord, "HINSTCOMMCTRL") != 0 )
        {
            wrongArgValueException(context->threadContext, 1, ADD_BITMAP_SRC_OBJ, src);
            goto done_out;
        }

        if ( argumentOmitted(3) )
        {
            missingArgException(context, 3);
            goto done_out;
        }
        tbab.hInst = HINST_COMMCTRL;

        CSTRING keyword = c->ObjectToStringValue(rxID);

        tbab.nID = keyword2idb(context, keyword, 3);
        if ( tbab.nID == OOD_INVALID_ITEM_ID )
        {
            goto done_out;
        }
    }

    result = (int32_t)SendMessage(getDChCtrl(pCSelf), TB_ADDBITMAP, count, (LPARAM)&tbab) + 1;

done_out:
    return result;
}

/** ToolBar::addButtons()
 */
RexxMethod2(RexxObjectPtr, tb_addButtons, RexxArrayObject, buttons, CSELF, pCSelf)
{
    RexxMethodContext *c     = context;
    RexxObjectPtr result     = TheFalseObj;
    LPTBBUTTON    pTbButtons = NULL;

    size_t count = c->ArrayItems(buttons);
    if ( count == 0 )
    {
        emptyArrayException(c->threadContext, 1);
        goto done_out;
    }

    pTbButtons = (LPTBBUTTON)LocalAlloc(LPTR, count * sizeof(TBBUTTON));
    if ( pTbButtons == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    for ( size_t i = 1; i <= count; i++ )
    {
        LPTBBUTTON pttb;

        RexxObjectPtr rxTbb = c->ArrayAt(buttons, i);
        if ( rxTbb == NULLOBJECT )
        {
            sparseArrayException(c->threadContext, 1, i);
            goto done_out;
        }
        if ( ! c->IsOfType(rxTbb, TBBUTTON_CLASS) )
        {
            wrongObjInArrayException(c->threadContext, 1, i, "a TbButton object", rxTbb);
            goto done_out;
        }

        pttb = (LPTBBUTTON)context->ObjectToCSelf(rxTbb);
        memcpy(pTbButtons + i - 1, pttb, sizeof(TBBUTTON));
    }

    if ( SendMessage(getDChCtrl(pCSelf), TB_ADDBUTTONS, count, (LPARAM)pTbButtons) )
    {
        result = TheTrueObj;
    }

done_out:
    safeLocalFree(pTbButtons);
    return result;
}

/** ToolBar::addString()
 *
 *  Adds one or more strings to the toolbar's string pool.
 *
 *  @param   strSrc      [required]  The strings to add.  Can either be an array
 *                       of strings, or a ResourceImage that contains the
 *                       strings.
 *
 *  @param   resourceID  [optional] If strSrc is a ResourceImage, this is the
 *                       resource ID of the string table in the resource module.
 *                       If strSrc is an array, the resourceID arg is ignored.
 *
 *  @returns  The one-based index of the first string this method adds to the
 *            toolbar's internal string pool.  I.e., if the string pool already
 *            contains 3 strings and this method succeeds, the return will be 4.
 *            If the method fails, 0 is returned.
 */
RexxMethod3(uint32_t, tb_addString, RexxObjectPtr, strSrc, OPTIONAL_RexxObjectPtr, rxID, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    uint32_t   result  = 0;
    CSTRING   *strings = NULL;
    char      *strPool = NULL;
    HINSTANCE  wParam  = NULL;
    LPARAM     lParam  = NULL;

    if ( c->IsArray(strSrc) )
    {
        RexxArrayObject strs = (RexxArrayObject)strSrc;

        size_t count = c->ArrayItems(strs);
        if ( count == 0 )
        {
            emptyArrayException(c->threadContext, 1);
            goto done_out;
        }

        strings = (CSTRING *)LocalAlloc(LPTR, count * sizeof(CSTRING *));
        if ( strings == NULL )
        {
            outOfMemoryException(c->threadContext);
            goto done_out;
        }

        size_t bufSize = 0;
        for ( size_t i = 1; i <= count; i++ )
        {
            CSTRING string;

            RexxObjectPtr rxString = c->ArrayAt(strs, i);
            if ( rxString == NULLOBJECT )
            {
                sparseArrayException(c->threadContext, 1, i);
                goto done_out;
            }

            string   = context->ObjectToStringValue(rxString);
            bufSize += strlen(string) + 1;

            strings[i - 1] = string;
        }
        bufSize++;

        strPool = (char *)LocalAlloc(LPTR, bufSize);
        if ( strPool == NULL )
        {
            outOfMemoryException(c->threadContext);
            goto done_out;
        }

        size_t  curLen;
        char   *dst = strPool;
        for ( size_t i = 0; i < count; i++ )
        {
            curLen = strlen(strings[i]) + 1;
            memcpy(dst, strings[i], curLen);
            dst += curLen;
        }

        // strPool must end wiht 2 NULLs, but since we allocated it filled with
        // zeros, we don't need to do anything.
        lParam = (LPARAM)strPool;
    }
    else if ( c->IsOfType(strSrc, "RESOURCEIMAGE") )
    {
        if ( argumentOmitted(2) )
        {
            missingArgException(context, 2);
        }

        PRESOURCEIMAGE pri = rxGetResourceImage(context, strSrc, 1);
        if ( pri == NULL )
        {
            goto done_out;
        }
        if ( ! pri->isValid )
        {
            nullObjectException(context->threadContext, "ResourceImage", 1);
            goto done_out;
        }

        uint32_t id = oodGlobalID(context, rxID, 2, true);
        if ( id == OOD_ID_EXCEPTION )
        {
            goto done_out;
        }
        wParam = pri->hMod;
        lParam = MAKELONG(id, 0);
    }
    else
    {
        wrongClassException(context->threadContext, 1, "an Array or a ResourceImage", strSrc);
        goto done_out;
    }

    result = (uint32_t)SendMessage(getDChCtrl(pCSelf), TB_ADDSTRING, (WPARAM)wParam, lParam);
    result++;

done_out:
    safeLocalFree(strings);
    safeLocalFree(strPool);
    return result;
}

/** ToolBar::autoSize()
 */
RexxMethod1(RexxObjectPtr, tb_autoSize, CSELF, pCSelf)
{
    SendMessage(getDChCtrl(pCSelf), TB_AUTOSIZE, 0, 0);
    return TheZeroObj;
}

/** ToolBar::buttonCount()
 */
RexxMethod1(uint32_t, tb_buttonCount, CSELF, pCSelf)
{
    return (uint32_t)SendMessage(getDChCtrl(pCSelf), TB_BUTTONCOUNT, 0, 0);
}

/** ToolBar::changeBitmap()
 */
RexxMethod5(logical_t, tb_changeBitmap, RexxObjectPtr, rxCmdID, RexxObjectPtr, rxBitmapID,
            OPTIONAL_uint32_t, ilID, OPTIONAL_uint32_t, offset, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }

    int32_t bitmapID = constructBitmapID(context, rxBitmapID, ilID, offset, 2);
    if ( bitmapID == OOD_INVALID_ITEM_ID )
    {
        return FALSE;
    }

    return SendMessage(getDChCtrl(pCSelf), TB_CHANGEBITMAP, id, bitmapID);
}

/** ToolBar::checkButton()
 */
RexxMethod3(logical_t, tb_checkButton, RexxObjectPtr, rxCmdID, OPTIONAL_logical_t, check, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }

    if ( argumentOmitted(2) )
    {
        check = TRUE;
    }

    return SendMessage(getDChCtrl(pCSelf), TB_CHECKBUTTON, id, check);
}

/** ToolBar::commandToIndex()
 *
 *  Retrieves the one-based index for the button associated with the specified
 *  command identifier.
 *
 *  @param cmdID
 *
 *  @returns  The one-based index of the button with the specified command ID,
 *            or zero if there is no such command ID.
 */
RexxMethod2(uint32_t, tb_commandToIndex, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return 0;
    }

    LPARAM index = SendMessage(getDChCtrl(pCSelf), TB_COMMANDTOINDEX, id, 0);
    index++;
    return (uint32_t)index;
}

/** ToolBar::enableButton()
 */
RexxMethod3(logical_t, tb_enableButton, RexxObjectPtr, rxCmdID, OPTIONAL_logical_t, enable, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }

    if ( argumentOmitted(2) )
    {
        enable = TRUE;
    }

    return SendMessage(getDChCtrl(pCSelf), TB_ENABLEBUTTON, id, enable);
}

/** ToolBar::indexToCommand()
 *
 *  Retrieves the command ID for the button specified by the. one-based index.
 *
 *  @param index
 *
 *  @returns  The command ID for the button with the specified index, or zero if
 *         there is no such command ID for the button, or .nil on error.
 */
RexxMethod2(RexxObjectPtr, tb_indexTocommand, uint32_t, index, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    TBBUTTON      tbb    = { 0 };

    RexxMethodContext *c = context;
    index--;
    if ( SendMessage(getDChCtrl(pCSelf), TB_GETBUTTON, index, (LPARAM)&tbb) )
    {
        result = c->UnsignedInt32(tbb.idCommand);
    }
    return result;
}

/** ToolBar::customize()
 *
 *  Displays the Customize Toolbar dialog box.
 *
 *  @returns  Zero, always.
 *
 *  @note  The dialog must connect and handle the QUERYINSERT and QUERYDELETE
 *         notifications for the Customize Toolbar dialog box to appear. If the
 *         toolbar does not handle those notifications, the Customize Toolbar
 *         dialog starts to appear and closes immediately.
 */
RexxMethod1(uint32_t, tb_customize, CSELF, pCSelf)
{
    SendMessage(getDChCtrl(pCSelf), TB_CUSTOMIZE, 0, 0);
    return 0;
}

/** ToolBar::getButton()
 *
 *  Returns a TbButton object containing the button information for the
 *  specified button, or .nil on error.
 *
 *  Note that the button is specified using its 1-based index, not its command
 *  ID.
 */
RexxMethod2(RexxObjectPtr, tb_getButton, uint32_t, index, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    RexxObjectPtr result = TheNilObj;

    RexxBufferObject tbbBuf = context->NewBuffer(sizeof(TBBUTTON));
    if ( tbbBuf == NULLOBJECT )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    LPTBBUTTON ptbb = (LPTBBUTTON)context->BufferData(tbbBuf);
    memset(ptbb, 0, sizeof(TBBUTTON));

    RexxArrayObject args = c->NewArray(3);
    c->ArrayPut(args, tbbBuf, 1);
    c->ArrayPut(args, c->String(TBBUTTON_OBJ_MAGIC), 3);

    RexxObjectPtr tbButton = c->SendMessage(TheTbButtonClass, "NEW", args);
    if ( tbButton == NULLOBJECT )
    {
        goto done_out;
    }

    index--;
    if ( ! SendMessage(getDChCtrl(pCSelf), TB_GETBUTTON, index, (LPARAM)ptbb) )
    {
        goto done_out;
    }
    result = tbButton;

done_out:
    return result;
}

/** ToolBar::getButtonText()
 *
 *  Returns the display text for the specified button, or .nil on error.
 *
 *  Note that the button is specified using its command ID, not its index.
 */
RexxMethod2(RexxObjectPtr, tb_getButtonText, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto done_out;
    }

    LRESULT count = SendMessage(getDChCtrl(pCSelf), TB_GETBUTTONTEXT, id, NULL);
    if ( count == -1 )
    {
        goto done_out;
    }

    char *buf = (char *)LocalAlloc(LPTR, count + 1);
    if ( buf == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    count = SendMessage(getDChCtrl(pCSelf), TB_GETBUTTONTEXT, id, (LPARAM)buf);
    if ( count == -1 )
    {
        goto done_out;
    }

    result = context->String(buf);
    LocalFree(buf);

done_out:
    return result;
}

/** ToolBar::getButtonTextEx()
 *
 *  Gets the text for the specified button.
 *
 *  @param  cmdID  [reguired]  Specifies the button, by its command ID, to get
 *                 the text for.
 *
 *  @return  The text for the button, or .nil on error.
 *
 *  @note  Once the setButtonText() method has been used to set the text of a
 *         button, the getButtonText() method will no longer return the text of
 *         the button. The getButtonTextEx() method has to be used.
 *
 *  @remarks
 */
RexxMethod2(RexxObjectPtr, tb_getButtonTextEx, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    uint32_t      id     = oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return result;
    }

    TBBUTTONINFO  info = { sizeof(TBBUTTONINFO), TBIF_TEXT };
    char          buf[TBBUTTON_TEXT_MAX + 1] = { '\0' };

    info.pszText = (LPSTR)buf;
    info.cchText = TBBUTTON_TEXT_MAX + 1;

    if ( SendMessage(getDChCtrl(pCSelf), TB_GETBUTTONINFO, id, (LPARAM)&info) != -1 )
    {
        result = context->String((CSTRING)buf);
    }

    return result;
}

/** ToolBar::getDisabledImageList()
 *
 *  Gets the image list that the toolbar uses to display buttons that are in
 *  their disabled state.
 *
 *  @return  The image list, or .nil if there is no image list.
 */
RexxMethod1(RexxObjectPtr, tb_getDisabledImageList, CSELF, pCSelf)
{
    return getImageList(context, iltDisabled, NULLOBJECT, pCSelf);
}

/** ToolBar::getExtenedeStyle()
 *
 *  Gets the current extended styles for a toolbar control.
 *
 *  @return  A keyword list that represents the current extended styles.
 */
RexxMethod1(RexxObjectPtr, tb_getExtendedStyle, CSELF, pCSelf)
{
    uint32_t exStyle = (uint32_t)SendMessage(getDChCtrl(pCSelf), TB_GETEXTENDEDSTYLE, 0, 0);
    return tbsEx2keyword(context, exStyle);
}

/** ToolBar::getHotImageList()
 *
 *  Gets the image list that the toolbar uses to display buttons that are in the
 *  hot state.
 *
 *  @return  The image list, or .nil if there is no image list.
 */
RexxMethod1(RexxObjectPtr, tb_getHotImageList, CSELF, pCSelf)
{
    return getImageList(context, iltHot, NULLOBJECT, pCSelf);
}

/** ToolBar::getImageList()
 *
 *  Gets the image list that the toolbar uses to display buttons that are in
 *  their default state.
 *
 *  @param  ilID       [optional]  Toolbars allow multiple image lists.  If the
 *                     application is using multiple image lists, ilID
 *                     identifies the image list to the toolbar.  If the
 *                     application is not using multiple image lists, omit this
 *                     argument or specify 0.
 *
 *  @return  The specified image list, or .nil if there is no image list.
 *
 */
RexxMethod2(RexxObjectPtr, tb_getImageList, OPTIONAL_RexxObjectPtr, ilID, CSELF, pCSelf)
{
    return getImageList(context, iltNormal, ilID, pCSelf);
}

/** ToolBar::getPressedImageList()
 *
 *  Sets the image list that the toolbar uses to display buttons that are in the
 *  pressed state.
 *
 *  @param  imageList  [required]  The new image list for the toolbar, or .nil
 *                     to remove an existing image list.
 *
 *  @return  The previous image list, or .nil if there was no previous image
 *           list.
 */
RexxMethod2(RexxObjectPtr, tb_getPressedImageList, RexxObjectPtr, il, CSELF, pCSelf)
{
    if ( requiredOS(context, "getPressedImageList", "Vista", Vista_OS) )
    {
        return getImageList(context, iltPressed, NULLOBJECT, pCSelf);
    }
    return TheNilObj;
}

/** ToolBar::insertButton()
 */
RexxMethod3(logical_t, tb_insertButton, RexxObjectPtr, tbButton, uint32_t, index, CSELF, pCSelf)
{
    if ( requiredClass(context->threadContext, tbButton, "TBBUTTON", 1) )
    {
        index--;
        return SendMessage(getDChCtrl(pCSelf), TB_INSERTBUTTON, index, (LPARAM)context->ObjectToCSelf(tbButton));
    }
    return FALSE;
}

/** ToolBar::isButtonChecked()
 */
RexxMethod2(logical_t, tb_isButtonChecked, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }
    return SendMessage(getDChCtrl(pCSelf), TB_ISBUTTONCHECKED, id, 0);
}

/** ToolBar::isButtonEnabled()
 */
RexxMethod2(logical_t, tb_isButtonEnabled, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }
    return SendMessage(getDChCtrl(pCSelf), TB_ISBUTTONENABLED, id, 0);
}

/** ToolBar::isButtonHidden()
 */
RexxMethod2(logical_t, tb_isButtonHidden, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }
    return SendMessage(getDChCtrl(pCSelf), TB_ISBUTTONHIDDEN, id, 0);
}

/** ToolBar::isButtonHighlighted()
 */
RexxMethod2(logical_t, tb_isButtonHighlighted, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }
    return SendMessage(getDChCtrl(pCSelf), TB_ISBUTTONHIGHLIGHTED, id, 0);
}

/** ToolBar::isButtonIndeterminate()
 */
RexxMethod2(logical_t, tb_isButtonIndeterminate, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }
    return SendMessage(getDChCtrl(pCSelf), TB_ISBUTTONINDETERMINATE, id, 0);
}

/** ToolBar::isButtonPressed()
 */
RexxMethod2(logical_t, tb_isButtonPressed, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }
    return SendMessage(getDChCtrl(pCSelf), TB_ISBUTTONPRESSED, id, 0);
}

/** ToolBar::loadImages()
 *
 *  Loads system-defined button images into a toolbar control's image list.
 *
 *  @param  [required]  A keyword specifying which system define images to load.
 *
 *  @return  A number ????
 */
RexxMethod2(int32_t, tb_loadImages, CSTRING, _index, CSELF, pCSelf)
{
    uint32_t index = keyword2idb(context, _index, 1);
    if ( index == OOD_INVALID_ITEM_ID )
    {
        return -1;
    }

    return (int32_t)SendMessage(getDChCtrl(pCSelf), TB_LOADIMAGES, index, (LPARAM)HINST_COMMCTRL);
}

/** ToolBar::setBitmapSize()
 *
 *  Sets the size, in pixels, of the bitmapped images to be added to this
 *  toolbar.
 *
 *  @param  size  [required]  The size, width and height, of the bitmap images.
 *           This can be specified in two forms.
 *
 *      Form 1:  arg 1 is a .Size object.
 *
 *      Form 2:  arg 1 is the width and arg2 is the height.
 *
 *  @return  True on success, false on error.
 *
 *  @note  The size can only be set before adding any bitmaps to the toolbar. If
 *         the program does not explicitly set the bitmap size, the size
 *         defaults to 16 by 15 pixels.
 *
 *  @remarks  Toolbar icons: 16x16, 24x24, 32x32. Note that toolbar icons are
 *            always flat, not 3D, even at the 32x32 size.  From MSDN.
 */
RexxMethod2(RexxObjectPtr, tb_setBitmapSize, ARGLIST, args, CSELF, pCSelf)
{
    RexxObjectPtr result = TheFalseObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    HWND hwnd = pcdc->hCtrl;

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getSizeFromArglist(context, args, (PORXPOINT)&point, 1, 2, &sizeArray, &argsUsed) )
    {
        goto done_out;
    }

    // Check arg count against expected.
    if ( sizeArray > argsUsed )
    {
        tooManyArgsException(context->threadContext, argsUsed);
        goto done_out;
    }

    if ( SendMessage(hwnd, TB_SETBITMAPSIZE, 0, MAKELPARAM(point.x , point.y)) )
    {
        result = TheTrueObj;
    }

done_out:
    return result;
}

/** ToolBar::setButtonText()
 *
 *  Sets the text for the specified button.
 *
 *  @param  text   [required]  The new text for the button.
 *
 *  @param  cmdID  [reguired]  Specifies the button, by its command ID, that is
 *                 to have its text set.
 *
 *  @return  True on success, false on error.
 *
 *  @note  Once this method is invoked to set the text of a button, the
 *         getButtonText() method will no longer return the text of the button.
 *         The getButtonTextEx() method will have to be used.
 */
RexxMethod3(logical_t, tb_setButtonText, CSTRING, text, RexxObjectPtr, rxCmdID, CSELF, pCSelf)
{
    uint32_t id =  oodGlobalID(context, rxCmdID, 2, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }

    TBBUTTONINFO info = { sizeof(TBBUTTONINFO), TBIF_TEXT };
    info.pszText = (char *)text;

    return SendMessage(getDChCtrl(pCSelf), TB_SETBUTTONINFO, id, (LPARAM)&info) != -1;
}

/** ToolBar::setDisabledImageList()
 *
 *  Sets the image list that the toolbar uses to display buttons that are in
 *  their disabled state.
 *
 *  @param  imageList  [required]  The new image list for the toolbar, or .nil
 *                     to remove an existing image list.
 *
 *  @return  The previous image list, or .nil if there was no previous image
 *           list.
 */
RexxMethod2(RexxObjectPtr, tb_setDisabledImageList, RexxObjectPtr, il, CSELF, pCSelf)
{
    return setImageList(context, iltDisabled, il, NULLOBJECT, pCSelf);
}

/** ToolBar::setExtenedeStyle()
 *
 *  Sets the extended styles for a toolbar control.
 *
 *  @param  [required]  A keyword list specifying which extended styles to set.
 *
 *  @return  A keyword list that represents the previous extended styles.
 */
RexxMethod2(RexxObjectPtr, tb_setExtendedStyle, CSTRING, _exStyle, CSELF, pCSelf)
{
    uint32_t exStyle = keyword2tbsEx(_exStyle);

    // The return is the old style
    exStyle = (uint32_t)SendMessage(getDChCtrl(pCSelf), TB_SETEXTENDEDSTYLE, 0, exStyle);
    return tbsEx2keyword(context, exStyle);
}

/** ToolBar::setHotImageList()
 *
 *  Sets the image list that the toolbar uses to display buttons that are in the
 *  hot state.
 *
 *  @param  imageList  [required]  The new image list for the toolbar, or .nil
 *                     to remove an existing image list.
 *
 *  @return  The previous image list, or .nil if there was no previous image
 *           list.
 */
RexxMethod2(RexxObjectPtr, tb_setHotImageList, RexxObjectPtr, il, CSELF, pCSelf)
{
    return setImageList(context, iltHot, il, NULLOBJECT, pCSelf);
}

/** ToolBar::setImageList()
 *
 *  Sets the image list that the toolbar uses to display buttons that are in
 *  their default state.
 *
 *  @param  imageList  [required]  The new image list for the toolbar, or .nil
 *                     to remove an existing image list.
 *
 *  @param  ilID       [optional]  Toolbars allow multiple image lists.  If the
 *                     application is using multiple image lists, ilID
 *                     identifies the image list to the toolbar.  If the
 *                     application is not using multiple image lists, omit this
 *                     argument or specify 0.
 *
 *  @return  The previous image list, or .nil if there was no previous image
 *           list.
 *
 *  @note
 *
 */
RexxMethod3(RexxObjectPtr, tb_setImageList, RexxObjectPtr, il, OPTIONAL_RexxObjectPtr, ilID, CSELF, pCSelf)
{
    return setImageList(context, iltNormal, il, ilID, pCSelf);
}

/** ToolBar::setPressedImageList()
 *
 *  Sets the image list that the toolbar uses to display buttons that are in the
 *  pressed state.
 *
 *  @param  imageList  [required]  The new image list for the toolbar, or .nil
 *                     to remove an existing image list.
 *
 *  @return  The previous image list, or .nil if there was no previous image
 *           list.
 */
RexxMethod2(RexxObjectPtr, tb_setPressedImageList, RexxObjectPtr, il, CSELF, pCSelf)
{
    if ( requiredOS(context, "setPressedImageList", "Vista", Vista_OS) )
    {
        return setImageList(context, iltPressed, il, NULLOBJECT, pCSelf);
    }
    return TheNilObj;
}


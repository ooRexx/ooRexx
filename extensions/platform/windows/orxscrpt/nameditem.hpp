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

class OrxNamedItem;         // Forward declaration
#ifndef NAMED_ITEM
#define NAMED_ITEM

#include "orxscrpt.hpp"
class OrxScript;             // Forward declaration

//   Wrapper class that hides the linked list holding the NamedItems.
// named item class: stores a named item including the flags with
// which it was added.
//
//    There are two types of items that this list stores.  One is a named item
//  that was passed to the engine through IActiveScript::AddNamedItem().  The
//  other is a subitem that belongs to one of the named items.  Example: Using
//  the Internet Explorer as the host, it will add "window" as a named item.
//  Later, the code might ask if there is a "document" named item.  The answer is
//  "No, there isn't, but.... "window" knows "document"".  So "document" is
//  added to the list as a subitem.
//    In the named item list, these are distinguished by the DispID field of the
//  NID (NAMEDITEM_STRUCT).  For items added through the IActiveScript method,
//  the DispID is -1.  For subitems added, the DispID will be anything other
//  than -1.
class OrxNamedItem {


public:
  OrxNamedItem(OrxScript *pEngine, FILE *Stream); // CTOR
  virtual ~OrxNamedItem();          // DTOR - Define in code file to avoid forward reference.

  //    A special LinkedList class, NILList (Named Item Linked List), was created for OrxNamedItem.
  //  The NILList::DropContent() has been made a friend, so that it can have access to the
  //  NAMEDITEM_STRUCT, which is necessary for it to perform the ->Release() on the interfaces
  //  it is holding, before freeing the structure to global memory.  A more Object Oriented
  //  friendly solution would be to provide an OrxNamedItem method to do this, however,
  //  this scheme seemed more efficient.
  friend void NILList::DropContent(void *Content);

  //    This entry point is called directly from AddNamedItem().  Therefore, it saves an
  //  item (not a subitem).  The DispID is forced to -1.  The wide character is required
  //  in the parameter list.  It is how we get the name from AddNamedItem(), and how
  //  GetIDsOfNames() wishes to see the name.  Therefore, to avoid needless conversion
  //  overhead, the wide charatecter set is passed.
  STDMETHODIMP AddItem(
    /* in  */ LPCOLESTR  pName,  // Item name
    /* in  */ DWORD      pFlags, // Flags from parm list identifying Item
    /* in  */ IUnknown * pIUnk,  // Associated IUnknown
    /* in  */ ITypeInfo *pITyIn);// Associated ITypeInfo

  //    This is called by the engine when a name is encountered during
  //  execution that needs resolution.  All of the known names are checked
  //  first.  If found, the appropriate information is copied from the NID.
  //  if not found, the list is scanned, and each Item entry is asked if it
  //  knows the name in question.  If so, then the internal AddItem() is
  //  called to add the subitem to the list.  Otherwise, an error is returned.
  //    The subitem inherits the IDispatch of the item that knows it.  The
  //  flags of the Item are not replicated with the SubItem.  0 is sent
  //  instead.
  //            Chart to determine what has been returned:
  //  Return Code            DispID         Return Type
  //  =====================  ==========     ===================
  //  S_OK                           -1     Item
  //  S_OK                        != -1     SubItem
  //  DISP_E_UNKNOWNNAME             -1     Not a valid/known name
  //
  //    Currently, this does not query a subitem to see if it supports the
  //  IDispatch interface.  Therefore, only Items, not SubItems are queried
  //  to see if a name is known.
  STDMETHODIMP WhoKnows(
    /* in  */ const char *pName,       // Item/SubItem name
    /* in  */ LCID      Lang,          // User Language code (English, Deutsch, etc.)
    /* out */ DISPID    *pbDispID,     // DispID of the SubItem
    /* out */ DWORD     *pbFlags,      // Flags stored with the Item, or SubItem
    /* out */ IDispatch **pbDispatch,  // IDispatch ptr of the item that knows the SubItem.
    /* out */ ITypeInfo **pbTypeInfo); // ITypeInfo ptr that contains the found variable (constant)


  // this returns an array of char* with the names of the
  // named items. both the char** and it's contents (char*)
  // must be free'd by the caller
  char ** getNamedItems(int *num);


  private:

    NILList    Items;
    OrxScript *Engine;
    FILE      *logfile;
/*
 *     I don't know if this truely hides the struct from the outside world, but
 *  it is worth a try.  It does hide it.
 *
 *     This defines the "content" of the NamedItem that will be stored on the
 *  LinkedList.
 *
 *
 *
 */
typedef struct NAMEDITEM_STRUCT {
  //  Copy of the AddItem() parameters
  LPOLESTR   Name;                // name of item
  DWORD      Flags ;              // flags of item
  IUnknown  *Unknown;             // unknown pointer
  ITypeInfo *TypeInfo;
  //  Generated values
  IDispatch *Dispatch;            // IDispatch pointer
  bool       Ex;                  // true if IDispatch is IDispatchEx. (Probably should use as IDispatch only)
  DISPID     DispID;              // -1 if this is a main named item, anything else and it is a sub-item.
  } NID, *PNID;


  //    This entry point is called by WhoKnows().  It saves a subitem, not an item.
  //  It is an internal interface used by WhoKnows() to add subitems when they are
  //  found.  It returns to WhoKnows() a pointer to the NID that was created for
  //  the subitem.
  STDMETHODIMP AddItem(
    /* in  */ LPCOLESTR  pName,   // SubItem name
    /* in  */ DWORD      pFlags,  // Flags from parm list identifying Item
    /* in  */ IDispatch *pIDisp,  // The Item's IDispatch
    /* in  */ ITypeInfo *pTypeInfo, // The Item's ITypeInfo
    /* in  */ DISPID     pDispID, // DispID of the SubItem
    /* out */ PNID      *pbNamedItem); //
};

#endif

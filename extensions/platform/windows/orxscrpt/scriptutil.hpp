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
#ifndef UTILITIES_HPP
#define UTILITIES_HPP

// #include "WObaseproto.h"
#include "orxscrpt_main.hpp"
// #define NULL 0

class ListItem {

friend class LinkedList;

protected:
  ListItem *FwdPtr;
  ListItem *BckPtr;
  char     *Name;
  void     *Content;

public:
   ListItem();                  // Default constructor
  ~ListItem();                  // Default destructor

  void *GetContent(void);
  char *GetName(void);
  void SetContentPtr(void *NewContent) {Content = NewContent;}


  };






class LinkedList  {

public:
  LinkedList();                 // Default constructor
  virtual ~LinkedList();        // Default destructor

enum InsertionPoint{Beginning,End};


  ListItem *AddItem(
    /*  in   */ char          *Name,
    /*  in   */ InsertionPoint Place,
    /*  in   */ void          *Content);

  char *GetName(
    /*  in   */ ListItem *Item);

  void *GetContent(
    /*  in   */ ListItem *Item);


/*******************************************************************************
 *
 *    FindItem() locates an item in the list via Name, or Index.  Each list
 *  contains the concept of a sequential index; i.e., as each item is inserted
 *  into the list it is numbered in accordance with its position in the list.
 *  The list is numbered from the beginning to the end using a zero based
 *  index.
 *     To reduce the scan of the list, each time it is accessed, the index
 *  number, and the address of that item are retained.  This gives the list
 *  three points of reference, the beginning, the end, and the last refferenced.
 *  Access will be obtained from the closest point.
 *     However, several things cause the index to be set to unknown: asking for
 *  an index that does not exist, and deleting one or more items.  Adding to the
 *  beginning of the list moves the index, and an unsuccessful Name search does
 *  not change the index.
 *     The FindItem() with no parms returns the next sequential item from the
 *  current index.
 *
 *******************************************************************************/
  ListItem *FindItem(
    /*  in   */ const char *Name);

  ListItem *FindItem(
    /*  in   */ int Index);

  ListItem *FindItem(void);

/*******************************************************************************
 *
 *     FindContent() functions the same as FindItem(), except that it returns
 *  the Content pointer, instead of a pointer to the Item.  This can save
 *  several steps when it is the content that is desired.  Note that it does
 *  make the check for a NULL item (one that is not found), and returns
 *  a NULL pointer for the content in those cases.
 *
 *
 *******************************************************************************/
  void *FindContent(
    /*  in   */ const char *Name);

  void *FindContent(
    /*  in   */ int Index);

  void *FindContent(void);

/*******************************************************************************
 *
 *     Logically this is on the Item level, however, everything is accessed from
 *  the Linked List level.  By exposing this function at the list level, the user
 *  of the list now has the option of overriding this functionality.
 *
 *
 *******************************************************************************/
  virtual void DropContent(
    /*  in   */ void *Content);

/*******************************************************************************
 *
 *     DropItem() destroys (or causes to be destroyed) the Item.
 *
 *     DeleteList() traverses the list calling DropItem().
 *
 *
 *******************************************************************************/
  void DropItem(
    /*  in   */ ListItem *Item);

  void DropItem(
    /*  in   */ void *Content);        // First find the Item to drop by its Content ptr.

  void DeleteList(void);



/*******************************************************************************
 *
 *  These are temporary, and for debugging purposes.
 *
 *           Remove before production.
 *
 *
 *******************************************************************************/
  void PrintList(void);
  void PrintItem(
    /*  in   */ ListItem *Item);



private:
  /*  Helper methods      */
  void InitList(void);
  ListItem *NameSearch(const char *Name);
  ListItem *IndexIncrement(void);
  ListItem *IndexSearch(int Index);


  /*  Actual properties   */
  ListItem  MasterLink;
  int       FixedSize;
  int       ListSize;                   // Number of items in this list.
  int       CurrIndex;                  // Position of IndexItem
  ListItem *IndexItem;                  // Pointer into the list.
  };




/*==========================================================================================*/


/*
 *    The following are specialty chains that are based on the previously defined
 *  LinkedList's.  They all differ in the way that they handle the Contents
 *  during the DropContent() method.
 *
 *  Note that at least three methods have to be overridden for this to be effective.
 *  1 The constructor, which doesn't do anything but exist.  The inherited constructor
 *   takes over to build the object.
 *  2 The destructor, which must explicitly call DeleteList(), otherwise the inherited
 *   destructor takes over, and all calls are made from the base level.  By calling
 *   DeleteList() from the overridden destructor all calls made in DeleteList() are
 *   made in the context of the new class and the overridden DropContent() is called.
 *   the default is for all calls to be made in the context of the base class.
 *  3 DropContent() must be overridden to perform the desired action.
 *
 *  The majority of the modifications for these special lists are contained in this
 *  header file.  Those that are too long to be contained on one brief line are
 *  at the end of the *.cpp file folllowing the "vvvvvvvvv Specialty DropContent()'s vvvvvvvvv"
 *  locator comment.
 */

//  This class of linked lists uses delete to release the memory pointed to by Content.
class NewLinkedList : public LinkedList {
public:
  NewLinkedList() : LinkedList() {};
  ~NewLinkedList() { DeleteList(); }
  void DropContent(void *Content) {if(Content) delete Content;};
  };



//  This class of linked lists uses delete [] to release the memory pointed to by Content.
class NewArrayLinkedList : public LinkedList {
public:
  NewArrayLinkedList() : LinkedList() {};
  ~NewArrayLinkedList() { DeleteList(); }
  void DropContent(void *Content) {if(Content) delete[] Content;};
  };



//  This class of linked lists uses delete to release the memory pointed to by Content,
// after it has SysFreeString()'ed the BSTR in the Content.  The BSTR must be the first
// item in the Content structure, or the overcasting to the Generic BSTR will not work.
typedef struct GENERICBSTR_struct {
  BSTR    String;
  } GBSTR, *PGBSTR;

class BSTRLinkedList : public LinkedList {
public:
  BSTRLinkedList() : LinkedList() {};
  ~BSTRLinkedList() { DeleteList(); }
  void DropContent(void *Content);     //  Defined in the *.cpp
  };



//  This class of linked lists uses delete to release the memory pointed to by Content,
// after it has VariantClear()'ed the VARIANT in the Content.  The VARIANT must be the first
// item in the Content structure, or the overcasting to the Generic VARIANT will not work.
typedef struct GENERICVARIANT_struct {
  VARIANT    Mutant;
  } GVARIANT, *PGVARIANT;

class VariantLList : public LinkedList {
public:
  VariantLList() : LinkedList() {};
  ~VariantLList() { DeleteList(); }
  void DropContent(void *Content);     //  Defined in the *.cpp
  };




//  This class of linked lists does not release the memory pointed to by Content.
class Index : public LinkedList {
public:
  Index() : LinkedList() {};
  virtual ~Index() { DeleteList(); }
  void DropContent(void *a) {;}
  };




//  This class of linked lists does not release the memory pointed to by Content.
//  It is loosely coupled, so that the list can go away, but the Content remains.
//  However, this prevents some logistical problems:
//  1   if the content goes away before the list, the content needs a way to
//    inform the list that it is gone.
//  2   if the list goes away the content, then it needs a way to tell the
//    the content, don't call me when you go away, I won't be here.
//
//  To make this coupling scheme work, a prototype Content is defined that
//  all Content's on the LooseLinkedList must inherit from.  In addition,
//  the method SetListState() must be overridden.  This allows the cast on
//  the Content pointer in LooseLinkedList::DropContent() to find the correct
//  entry for the data in the v-table.
//

class LooseLinkedList;

class LLLContent {
public:

  enum LLLState{Exists,Destroyed};

  LLLContent() {ListState = Destroyed; ListRoot = NULL;  Ourselves = NULL;}
  ~LLLContent();

  void SetDestructor(LooseLinkedList *Root, void *Container);
  virtual void SetListState(LLLState Status) {ListState = Status;}

private:
  LLLState         ListState;
  LooseLinkedList *ListRoot;
  void            *Ourselves;          //   This is the pointer to the object that inherits this.
  };

class LooseLinkedList : public LinkedList {
public:
  LooseLinkedList() : LinkedList() {};
  virtual ~LooseLinkedList() { DeleteList(); }
  void DropContent(void *a) {((LLLContent *)a)->SetListState(LLLContent::Destroyed);}


  };



//  This class of linked lists has special tear downs for NamedItems.
class NILList : public LinkedList {
public:
  NILList() : LinkedList() {};
  virtual ~NILList() { DeleteList(); }
  void DropContent(void *Content);          // Defined in NamedItem.cpp
  };




/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *****       E n d   o f   L i n k e d L i s t ' s                         *****
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *******************************************************************************
 *******************************************************************************
 *******************************************************************************/



STDMETHODIMP GetProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *RetInfo);
STDMETHODIMP GetProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *RetInfo,DISPID *DispID);
STDMETHODIMP GetProperty(FILE *Stream, IDispatch *Disp, LCID Lang, VARIANT *RetInfo,DISPID DispID);

STDMETHODIMP PutProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *NewValue);
STDMETHODIMP PutProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *NewValue,DISPID *DispID);
STDMETHODIMP PutProperty(FILE *Stream, IDispatch *Disp, LCID Lang, VARIANT *NewValue,DISPID DispID);

int CharsRequired(int Number);

STDMETHODIMP InvokeNamedParms(
  /*  in   */ DISPPARAMS *Params,
  /*in/out */ char **CallString,
  /*in/out */ int  *MaxStrLen,
  /*in/out */ char **NameList,
  /*in/out */ int *MaxListLen);

STDMETHODIMP NewBuffer(
  /*in/out */ char **OldBuffer,
  /*in/out */ int *MaxBufLen);

/******************************************************************************
*                 AddMutant
*
*   Inserts an unnamed OLECHAR* argument as the first parameter into an IDispatch
*  invocation Variant list (an array of VARIANT, not a variant VT_ARRAY.
*
*   Do not forget to free the BSTR that was allocated for the parameter.
*
*    AddMutant(L"First!",pArgs,&DP);
*    Invoke(DispID, IID_NULL, 0, pFlags,  &DP, pbResults, pbErrInfo,pArgErr);
*    FCmd = DP.rgvarg;
*    VariantClear(&FCmd[pArgs->cArgs]);  // freestring
*    delete FCmd;                        // Remove the created command list
*
******************************************************************************/
STDMETHODIMP AddMutant(
  /*  [in]  */ OLECHAR     *FirstArg,     //  Pointer to the string to insert.
  /*  [in]  */ DISPPARAMS  *OrigDP,       //  Original DISPPARMS
  /*[in/out]*/ DISPPARAMS  *DP);          //  DISPPARMS that will contain the new argument


/******************************************************************************
*                 DropNamedPut
*
*    Deletes the DISP_PROPERTYPUT from the Named Parameter List.
*
******************************************************************************/
STDMETHODIMP DropNamedPut(
  /*  [in]  */ DISPPARAMS  *OrigDP,
  /*[in/out]*/ DISPPARAMS  *DP);

#endif    //    ifndef UTILITIES_HPP

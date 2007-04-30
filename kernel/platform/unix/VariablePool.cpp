/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX AIX Support                                             aixvpool.c    */
/*                                                                            */
/* AIX system specific RexxVariablePool processing function                   */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* aixvpool.c - Contains routines to handle external and system dependent     */
/*              access to REXX variable pools.                                */
/*                                                                            */
/* Entry points:                                                              */
/*   RexxVariablePool - 32-bit documented API into REXX variable pool         */
/*                                                                            */
/* Internal routines:                                                         */
/*   copy_value -                                                             */
/*                                                                            */
/* C methods:                                                                 */
/*   SysVariablePool - Native activation method to provide access to variable */
/*                     pools                                                  */
/*                                                                            */
/******************************************************************************/

#define INCL_RXSHV                          /* Get shared variable pool stuff */
#include "RexxCore.h"                         /* Object REXX kernel declares    */
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxVariableDictionary.hpp"
#include "ExpressionBaseVariable.hpp"
#include "RexxNativeAPI.h"                           /* Get C-method declares, etc.    */
#include SYSREXXSAA                         /* Documented REXX API's          */
#include <stdlib.h>

extern "C" {
   APIRET APIENTRY RexxStemSort(PCHAR stemname, INT order, INT type,
       size_t start, size_t end, size_t firstcol, size_t lastcol);
}

#define IS_EQUAL(s,l)  (s->strCompare(l))

/******************************************************************************/
/* RexxStemSort - 32-bit undocumented API to sort stem elements.              */
/*                                                                            */
/* Input:                                                                     */
/*   stemname - an ASCII-Z stem name.                                         */
/*   order    - the sort order                                                */
/*   type     - The type of sort                                              */
/*   start    - index of the first element to sort                            */
/*   end      - index of the last element to sort                             */
/*   firstcol - first column position to sort                                 */
/*   lastcol -  last column position to sort                                  */
/*                                                                            */
/* Output:                                                                    */
/*   TRUE if the sort succeeded, FALSE for any parameter errors.              */
/******************************************************************************/
ULONG REXXENTRY RexxStemSort(PCHAR stemname, INT order, INT type,
    size_t start, size_t end, size_t firstcol, size_t lastcol)
{
    if (!RexxQuery())                         /* Are we up?                     */
      return FALSE;                           /*   No, send nastygram.          */
    else                                      /*   Yes, ship request to kernel  */
      return REXX_STEMSORT(stemname, order, type, start, end, firstcol, lastcol);
}

/******************************************************************************/
/* RexxVariablePool - 32-bit documented API to access REXX variable pools.    */
/*                                                                            */
/* Input:                                                                     */
/*   pshvblock - Pointer to one or more variable request blocks.              */
/*                                                                            */
/* Output:                                                                    */
/*   rc - Composite return code for all request blocks (individual rc's are   */
/*        set within the shvret fields of each request block).                */
/******************************************************************************/
ULONG APIENTRY RexxVariablePool(PSHVBLOCK pshvblock)
{

  if (!RexxQuery())                         /* Are we up?                     */
    return RXSHV_NOAVL;                     /*   No, send nastygram.          */
  else                                      /*   Yes, ship request to kernel  */
    return REXX_VARIABLEPOOL((PVOID)pshvblock);

} /* end RexxVariablePool */

/******************************************************************************/
/* copy_value -                                                               */
/******************************************************************************/
static ULONG copy_value(
    RexxObject * value,                /* value to copy                     */
    RXSTRING   * rxstring,             /* target rxstring                   */
    ULONG      * length )              /* length field to update            */
{
   RexxString * stringValue;           /* converted object value            */
   ULONG        string_length;         /* length of the string              */
   ULONG        rc;                    /* return code                       */

   rc = 0;                             /* default to success                */
                                       /* get the string value              */
   stringValue = value->stringValue();
   string_length = stringValue->length;/* get the string length             */
   if (rxstring->strptr == NULL) {     /* no target buffer?                 */
                                       /* allocate a new one                */

     if (NULL == (rxstring->strptr = (char *)malloc(string_length+1)))/* add one */

       return RXSHV_MEMFL;             /* couldn't allocate, return flag    */
     else                              /* rxstring is same as string        */
       rxstring->strlength = string_length +1;
   }
                                       /* buffer too short?                 */
   if (string_length > rxstring->strlength){
     rc = RXSHV_TRUNC;                 /* set truncated return code         */
                                       /* copy the short piece              */
     memcpy(rxstring->strptr, stringValue->stringData, rxstring->strlength);
   }
   else {
                                       /* copy entire string                */
     memcpy(rxstring->strptr, stringValue->stringData, string_length);
                                       /* room for a null?                  */
     if (rxstring->strlength > string_length)
                                       /* yes, add one                      */
       rxstring->strptr[string_length] = '\0';
                                       /* fill in the string length         */
     rxstring->strlength = string_length;
   }
   *length = string_length;            /* return actual string length       */
   return rc;                          /* give back the return code         */
}

/******************************************************************************/
/*                                                                            */
/* SysVariablePool - Native activation method that handles external variable  */
/*                   pool requests.                                           */
/*                                                                            */
/* Inputs:                                                                    */
/*   self - Reference to current native activation.                           */
/*   requests - Pointer to a chain of one or more shared variable pool        */
/*              request blocks.                                               */
/*                                                                            */
/* Outputs:                                                                   */
/*   ULONG - Composite return code that results from processing request       */
/*           blocks.                                                          */
/*                                                                            */
/* Notes:                                                                     */
/******************************************************************************/
ULONG SysVariablePool(
    RexxNativeActivation * self,       /* current native activation         */
    PVOID                  requests,   /* shared variable request           */
    BOOL                   enabled)    /* is VP fully enabled               */
{
  RexxString       * variable;         /* name of the variable              */
  RexxVariableBase * retriever;        /* variable retriever                */
//RexxVariableDictionary * dictionary; /* variable dictionary               */
  RexxActivation   * activation;       /* most recent REXX activation       */
  RexxObject       * value;            /* fetched value                     */
//RexxArray        * valuePair;        /* name/value pair from fetch next   */
  ULONG              retcode;          /* composite return code             */
  LONG               arg_position;     /* requested argument position       */
  INT                code;             /* variable request code             */
  PSHVBLOCK          pshvblock;        /* variable pool request block       */
  LONG               tempSize;

 retcode = 0;                          /* initialize composite rc           */

 pshvblock = (PSHVBLOCK)requests;      /* copy the request block pointer    */
                                       /* get the variable dictionary       */
// dictionary = self->activationVariables();
 activation = self->activity->currentAct();

 while (pshvblock) {                   /* while more request blocks         */
  pshvblock->shvret = 0;               /* set the block return code         */
  code = pshvblock->shvcode;           /* get the request code              */

                                       /* one of the access forms?          */
                                       /* and VP is enabled                 */
//  if ((code==RXSHV_FETCH || code==RXSHV_SYFET || code==RXSHV_SET || code==RXSHV_SYSET
//      code==RXSHV_DROPV || code==RXSHV_SYDRO) && (enabled)) {
  if ((code==RXSHV_FETCH || code==RXSHV_SYFET) || (code==RXSHV_SET || code==RXSHV_SYSET ||
      code==RXSHV_DROPV || code==RXSHV_SYDRO) && (enabled)) {

                                       /* no name given?                    */
    if (pshvblock->shvname.strptr==NULL)
      pshvblock->shvret|=RXSHV_BADN;   /* this is bad                       */
    else {
                                       /* get the variable as a string      */
      variable = new_string((PCHAR)pshvblock->shvname.strptr, pshvblock->shvname.strlength);
                                       /* symbolic access?                  */
      if (code == RXSHV_SYFET || code == RXSHV_SYSET || code == RXSHV_SYDRO)
                                       /* get a symbolic retriever          */
//      retriever = dictionary->retriever(variable);
        retriever = activation->getVariableRetriever(variable);
      else                             /* need a direct retriever           */
//      retriever = dictionary->directRetriever(variable);
        retriever = activation->getDirectVariableRetriever(variable);
      if (retriever == OREF_NULL)      /* have a bad name?                  */
        pshvblock->shvret|=RXSHV_BADN; /* this is bad                       */
      else {
        self->resetNext();             /* reset any next operations         */
                                       /* have a non-name retriever         */
                                       /* and a new variable?               */
//      if (!OTYPE(String, retriever) && !retriever->exists(dictionary))
        if (!OTYPE(String, retriever) && !retriever->exists(activation))
                                       /* flag this in the block            */
          pshvblock->shvret |= RXSHV_NEWV;

        switch (code) {                /* now process the operations        */

          case RXSHV_SYFET:            /* fetch operations                  */
          case RXSHV_FETCH:
                                       /* have a non-name retriever?        */
            if (OTYPE(String, retriever))
                                       /* the value is the retriever        */
              value = (RexxObject *)retriever;
            else
                                       /* get the variable value            */
//            value = retriever->getValue(dictionary);
              value = retriever->getValue(activation);
                                       /* copy the value                    */
            pshvblock->shvret |= copy_value(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
            break;

          case RXSHV_SYSET:            /* set operations                    */
          case RXSHV_SET:
                                       /* have a non-name retriever?        */
            if (OTYPE(String, retriever))
                                       /* this is bad                       */
              pshvblock->shvret = RXSHV_BADN;
            else
                                       /* do the assignment                 */
//            retriever->set(dictionary, new_string((PCHAR)pshvblock->shvvalue.strptr, pshvblock->shvvalue.strlength));
              retriever->set(activation, new_string((PCHAR)pshvblock->shvvalue.strptr, pshvblock->shvvalue.strlength));
            break;

          case RXSHV_SYDRO:            /* drop operations                   */
          case RXSHV_DROPV:
                                       /* have a non-name retriever?        */
            if (OTYPE(String, retriever))
                                       /* this is bad                       */
              pshvblock->shvret = RXSHV_BADN;
            else
                                       /* perform the drop                  */
//            retriever->drop(dictionary);
              retriever->drop(activation);
            break;
        }
      }
    }
  }
//else if ((code == RXSHV_NEXTV) &&    /* need to process a NEXT request?   */
//         (enabled)) {                /* and VP is enabled                 */
//  MICMICD                            /* get the next variable             */
//  valuePair = dictionary->fetchNext(self);
//  if (valuePair == OREF_NULL)        /* last one?                         */
  else if (code == RXSHV_NEXTV) {      /* need to process a NEXT request?   */
    RexxString *name;
    RexxObject *value;
                                       /* get the next variable             */
    if (!self->fetchNext(&name, &value)) {
      pshvblock->shvret |= RXSHV_LVAR; /* flag as such                      */
    }
    else {                             /* need to copy the name and value   */
                                       /* copy the name                     */
//    pshvblock->shvret |= copy_value(valuePair->get(1), &pshvblock->shvname, &pshvblock->shvnamelen);
      pshvblock->shvret |= copy_value(name, &pshvblock->shvname, &pshvblock->shvnamelen);
                                       /* copy the value                    */
//    pshvblock->shvret |= copy_value(valuePair->get(2), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
      pshvblock->shvret |= copy_value(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
    }
  }
  else if ((code == RXSHV_PRIV) &&     /* need to process PRIVATE block?    */
           (enabled || TRUE)) {        /* and VP is enabled                 */
                                       /* private block should always be enabled */
                                       /* no name given?                    */
    if (pshvblock->shvname.strptr==NULL)
      pshvblock->shvret|=RXSHV_BADN;   /* this is bad                       */
    else {
                                       /* get the variable as a string      */
      variable = new_string((PCHAR)pshvblock->shvname.strptr, pshvblock->shvname.strlength);
                                       /* want the version string?          */
      if (IS_EQUAL(variable, "VERSION")) {
                                       /* copy the value                    */
        pshvblock->shvret |= copy_value(version_number(), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
      }
                                       /* want the current exit?            */
      else if (IS_EQUAL(variable, "EXITNAME")) {
                                       /* get the exit name                 */
        value = CurrentActivity->getCurrentExit();
        if (value == OREF_NULL)        /* is this a null?                   */
          value = OREF_NULLSTRING;     /* this is a null string value       */
                                       /* copy the value                    */
        pshvblock->shvret |= copy_value(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
      }
                                       /* want the the current queue?       */
      else if (IS_EQUAL(variable, "QUENAME")) {
                                       /* copy the value                    */
        pshvblock->shvret |= copy_value(SysGetCurrentQueue(), &pshvblock->shvvalue, &pshvblock->shvvaluelen);
      }
                                       /* want the version string?          */
      else if (IS_EQUAL(variable, "SOURCE")) {
                                       /* retrieve the source string        */
//      value = self->activity->currentAct()->sourceString();
        value = activation->sourceString();
                                       /* copy the value                    */
        pshvblock->shvret |= copy_value(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
      }
                                       /* want the parameter count?         */
      else if (IS_EQUAL(variable, "PARM")) {
                                       /* get the REXX activation           */
//      activation = self->activity->currentAct();
                                       /* get the argument list size        */
//      tempSize = activation->getEntryArglist()->size();
        tempSize = activation->getProgramArgumentCount();
        value = new_integer(tempSize);
                                       /* copy the value                    */
        pshvblock->shvret |= copy_value(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
      }
                                       /* some other parm form              */
      else if (!memcmp(variable->stringData, "PARM.", sizeof("PARM.") - 1)) {
                                       /* extract the numeric piece         */
        value = variable->extract(strlen("PARM."), variable->length - strlen("PARM."));
                                       /* get the binary value              */
        arg_position = value->longValue(DEFAULT_DIGITS);
                                       /* not a good number?                */
        if (arg_position == NO_LONG || arg_position <= 0)
                                       /* this is a bad name                */
          pshvblock->shvret|=RXSHV_BADN;
        else {
                                       /* get the REXX activation           */
//        activation = self->activity->currentAct();
//        valuePair = activation->getEntryArglist();
//                                     /* out of the bounds?                */
//        if (arg_position > valuePair->size())
//          value = OREF_NULLSTRING;   /* return a null string              */
//        else {
//                                     /* get the argument                  */
          /* get the arcgument from the parent activation */
          value = activation->getProgramArgument(arg_position);
          if (value == OREF_NULL) {    /* doesn't exist?                    */
//          value = valuePair->get(arg_position);
//          if (value == OREF_NULL)    /* doesn't exist?                    */
              value = OREF_NULLSTRING; /* return a null string              */
          }
                                       /* copy the value                    */
          pshvblock->shvret |= copy_value(value, &pshvblock->shvvalue, &pshvblock->shvvaluelen);
        }
      }
      else
        pshvblock->shvret|=RXSHV_BADN; /* this is a bad name                */
    }
  }
  else if (code == RXSHV_EXIT) {       /* need to set a return value for    */
                                       /* external function or sys exit     */

                                       /* Set ret value in the activity     */
     self->activity->setShvVal(new_string((PCHAR)pshvblock->shvvalue.strptr, pshvblock->shvvalue.strlength));
  }
  else if (enabled)                    /* if get here and VP is enabled     */
    pshvblock->shvret |= RXSHV_BADF;   /* bad function                      */
  else
    pshvblock->shvret |= RXSHV_NOAVL;  /* VP is not available               */
  retcode |= pshvblock->shvret;        /* accumulate the return code        */
  pshvblock = pshvblock->shvnext;      /* step to the next block            */
 }
 return retcode;                       /* return composite return code      */
}


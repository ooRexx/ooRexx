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
/* REXX Kernel                                      RexxNativeActivation.c    */
/*                                                                            */
/* Primitive Native Activation Class                                          */
/*                                                                            */
/******************************************************************************/
#define INCL_REXXSAA

#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "MethodClass.hpp"
#include "RexxNativeMethod.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxBuffer.hpp"
#include "MessageClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "SourceFile.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ProtectedObject.hpp"


#include "RexxNativeAPI.h"                      /* bring in the native code defines  */
#undef   RexxTable                     /* remove a conflict                 */

#include SYSREXXSAA

static size_t tsize[] = { 0, 0,
                          sizeof(RexxObject *),
                          sizeof(int),
                          sizeof(long),
                          sizeof(double),
                          sizeof(CSTRING),
                          sizeof(RexxObject *),      /* OSELF */
                          sizeof(RexxObject *),      /* ARGLIST */
                          sizeof(RexxObject *),      /* MSGNAME */
                          sizeof(RexxObject *),      /* SCOPE */
                          sizeof(void *),            /* POINTER */
                          sizeof(void *),            /* CSELF */
                          sizeof(RexxObject *),      /* STRING */
                          sizeof(void *)             /* BUFFER */
                        };

void RexxNativeActivation::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->argArray);
  memory_mark(this->receiver);
  memory_mark(this->method);
  memory_mark(this->activity);
  memory_mark(this->activation);
  memory_mark(this->msgname);
  memory_mark(this->savelist);
  memory_mark(this->objnotify);
  memory_mark(this->result);
  memory_mark(this->nextstem);
  memory_mark(this->compoundelement);
  memory_mark(this->nextcurrent);
  memory_mark(this->objectVariables);
  memory_mark(this->firstSavedObject);

  /* We're hold a pointer back to our arguments directly where they */
  /* are created.  Since in some places, this argument list comes */
  /* from the C stack, we need to handle the marker ourselves. */
  size_t i;
  for (i = 0; i < argcount; i++) {
      memory_mark(arglist[i]);
  }
  cleanUpMemoryMark
}

void RexxNativeActivation::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->argArray);
  memory_mark_general(this->receiver);
  memory_mark_general(this->method);
  memory_mark_general(this->activity);
  memory_mark_general(this->activation);
  memory_mark_general(this->msgname);
  memory_mark_general(this->savelist);
  memory_mark_general(this->objnotify);
  memory_mark_general(this->result);
  memory_mark_general(this->nextstem);
  memory_mark_general(this->compoundelement);
  memory_mark_general(this->nextcurrent);
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->firstSavedObject);

  /* We're hold a pointer back to our arguments directly where they */
  /* are created.  Since in some places, this argument list comes */
  /* from the C stack, we need to handle the marker ourselves. */
  size_t i;
  for (i = 0; i < argcount; i++) {
      memory_mark_general(arglist[i]);
  }
  cleanUpMemoryMarkGeneral
}

void RexxNativeActivation::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNativeActivation)

     flatten_reference(newThis->argArray, envelope);
     flatten_reference(newThis->receiver, envelope);
     flatten_reference(newThis->method, envelope);
     flatten_reference(newThis->activity, envelope);
     flatten_reference(newThis->activation, envelope);
     flatten_reference(newThis->msgname, envelope);
     flatten_reference(newThis->savelist, envelope);
     flatten_reference(newThis->objnotify, envelope);
     flatten_reference(newThis->result, envelope);
     flatten_reference(newThis->nextstem, envelope);
     flatten_reference(newThis->compoundelement, envelope);
     flatten_reference(newThis->nextcurrent, envelope);
     flatten_reference(newThis->objectVariables, envelope);
     flatten_reference(newThis->firstSavedObject, envelope);

  cleanUpFlatten
}


RexxObject *RexxNativeActivation::run(
    size_t       _argcount,             /* argument count                    */
    RexxObject **_arglist)              /* method argument list              */
/******************************************************************************/
/* Function:  Execute a REXX native method                                    */
/******************************************************************************/
{
  PNMF         methp;                  /* method entry point                */
  char       * itypes;                 /* pointer to method types           */
  void       * ivalues[16];            /* array of converted arguments      */
  void      ** ivalp = ivalues;        /* pointer to current value          */
  char         ibuffer[100];           /* arguments buffer                  */
  char       * ibufp = ibuffer;        /* current buffer pointer            */
  char       * tp;                     /* current type                      */
  size_t       i;                      /* current argument position         */
  RexxObject * argument;               /* current argument object           */
  LONG         tempValues;             /* temporary value                   */
  BOOL         used_arglist;           /* method requested an arglist       */

  this->arglist = _arglist;            /* save the argument information     */
  this->argcount = _argcount;          /* set the argument count            */
  used_arglist = FALSE;                /* no arglist requested              */
                                       /* get the entry point address       */
  methp = (PNMF)this->method->getNativeCode()->getEntry();
  itypes = (*methp)(0);                /* get type information from method  */
  *ivalues = ibufp;
  ibufp += tsize[(int)(*itypes)];      /* step over first type              */
  i = 0;                               /* no arguments used yet             */
                                       /* loop through the type information */
  for (tp = itypes+1, ivalp = ivalues+1; *tp; tp++, ivalp++) {
    *ivalp = ibufp;
    ibufp += tsize[(int)(*tp)];        /* step over the type                */
    switch (*tp) {                     /* switch based on this type         */

      case REXXD_OSELF:                /* reference to SELF                 */
        *((RexxObject **)*ivalp) = this->receiver;
        break;

      case REXXD_CSELF:                /* reference to CSELF                */
        *((void **)*ivalp) = this->cself();
        break;

      case REXXD_BUFFER:               /* reference to Buffered storage     */
        *((void **)*ivalp) = this->buffer();
        break;

      case REXXD_ARGLIST:              /* need the argument list            */
        /* create the argument array */
        argArray = new (argcount, arglist) RexxArray;
        *((RexxObject **)*ivalp) = argArray;
        used_arglist = TRUE;           /* passing along everything          */
        break;

      case REXXD_MSGNAME:              /* the message name                  */
        *((RexxObject **)*ivalp) = this->msgname;
        break;

      default:                         /* still within argument bounds?     */
        if (i < argcount && arglist[i] != OREF_NULL) {
          argument = arglist[i];       /* get the next argument             */
          switch (*tp) {               /* process this type                 */

            case REXXD_OBJECT:         /* arbitrary object reference        */
              *((RexxObject **)*ivalp) = argument;
              break;

            case REXXD_int:            /* integer value                     */
                                       /* try to convert the value          */
              tempValues = argument->longValue(this->digits());
                                       /* not convertable?                  */
              if (tempValues == (long)NO_LONG)
                                       /* this is an error                  */
                reportException(Error_Incorrect_method_whole, i+1, argument);
                                       /* copy over the info                */
              *((int *)*ivalp) = (int)tempValues;
              break;

            case REXXD_long:           /* long value                        */
                                       /* try to convert the value          */
              tempValues = argument->longValue(this->digits());
                                       /* not convertable?                  */
              if (tempValues == (long)NO_LONG)
                                       /* this is an error                  */
                reportException(Error_Incorrect_method_whole, i+1, argument);
                                       /* copy over the info                */
              *((long *)*ivalp) = tempValues;
              break;

            case REXXD_double:         /* double value                      */
              *((double *)*ivalp) = this->getDoubleValue(argument);
              break;

            case REXXD_CSTRING:        /* ASCII-Z string value              */
              *((const char **)*ivalp) = this->cstring(argument);
              break;

            case REXXD_STRING:         /* Required STRING object            */
            {
                                         /* force to a string value           */
                RexxObject *temp = REQUIRED_STRING(argument, i + 1) ;
                if (temp != argument)    /* new object created?               */
                                         /* make it safe                      */
                  this->saveObject(temp);
                                         /* set the result in                 */
                *((RexxObject **)*ivalp) = temp;
                break;

            }

            case REXXD_POINTER:
              *((void **)*ivalp) = this->pointer(argument);
              break;

            default:                   /* something messed up               */
              logic_error("unsupported native method argument type");
              break;
          }
        }
        else {                         /* no value provided, use default    */
          switch (*tp) {

            case REXXD_STRING:         /* no object here                    */
            case REXXD_OBJECT:         /* no object here                    */
              *((RexxObject **)*ivalp) = OREF_NULL;
              break;

            case REXXD_int:            /* non-integer value                 */
              *((int *)*ivalp) = NO_INT;
              break;

            case REXXD_long:           /* non-existent long                 */
              *((long *)*ivalp) = NO_LONG;
              break;

            case REXXD_double:         /* non-existent double               */
              *((double *)*ivalp) = NO_DOUBLE;
              break;

            case REXXD_CSTRING:        /* missing character string          */
              *((CSTRING *)*ivalp) = NO_CSTRING;
              break;

            case REXXD_POINTER:
              *((void **)*ivalp) = NULL;
              break;

            default:                   /* still an error if not there       */
              logic_error("unsupported native method argument type");
              break;
          }
        }
        i++;                           /* step to the next argument         */
        break;
    }
  }
  if (i < argcount && !used_arglist)   /* extra, unwanted arguments?        */
                                       /* got too many                      */
    reportException(Error_Incorrect_method_maxarg, i);

  size_t activityLevel = this->activity->getActivationLevel();
                                       /* get a RAISE type return?          */
  if (setjmp(this->conditionjump) != 0) {
    // TODO  Use protected object on the result
    if (this->result != OREF_NULL)     /* have a value?                     */
      holdObject(this->result);        /* get result held longer            */
    this->guardOff();                  /* release any variable locks        */
    this->argcount = 0;                /* make sure we don't try to mark any arguments */
    // the lock holder gets here by longjmp from a kernel reentry.  We need to
    // make sure the activation count gets reset, else we'll accumulate bogus
    // nesting levels that will make it look like this activity is still in use
    // when in fact we're done with it.
    this->activity->restoreActivationLevel(activityLevel);
    this->activity->pop(FALSE);        /* pop this from the activity        */
    this->setHasNoReferences();        /* mark this as not having references in case we get marked */
    return this->result;               /* and finished                      */
  }

  try
  {
      activity->releaseAccess();           /* force this to "safe" mode         */
      (*methp)(ivalues);                   /* process the method call           */
      activity->requestAccess();           /* now in unsafe mode again          */
  }
  catch (RexxActivation *a)
  {
      // it's possible that we can get terminated by a throw during condition processing.
      // we intercept this here, perform any cleanup we need to perform, then let the
      // condition trap propagate.
      this->guardOff();                  /* release any variable locks        */
      this->argcount = 0;                /* make sure we don't try to mark any arguments */
      // the lock holder gets here by longjmp from a kernel reentry.  We need to
      // make sure the activation count gets reset, else we'll accumulate bogus
      // nesting levels that will make it look like this activity is still in use
      // when in fact we're done with it.
      this->activity->restoreActivationLevel(activityLevel);
      // IMPORTANT NOTE:  We don't pop our activation off the stack.  This will be
      // handled by the catcher.  Attempting to pop the stack when an error or condition has
      // occurred can munge the activation stack, resulting bad stuff.
      this->setHasNoReferences();        /* mark this as not having references in case we get marked */

      // now rethrow the trapped condition so that real target can handle this.
      throw;
  }

  // belt and braces...this restores the activity level to whatever
  // level we had when we made the callout.
  this->activity->restoreActivationLevel(activityLevel);

  /* give up reference to receiver so that it can be garbage collected */
  this->receiver = OREF_NULL;

  // set a default result
  result = OREF_NULL;

  switch (*itypes) {                   /* now process the return value      */
    case REXXD_void:                   /* void, this is just a NULL OREF    */
      result = OREF_NULL;
      break;

    case REXXD_OBJECT:                 /* Object reference                  */
                                       /* no result returned?               */
      if (*((RexxObject **)*ivalues) != OREF_NULL)
      {
                                       /* give a direct pointer to this     */
          result = *((RexxObject **)*ivalues);
      }
      break;

    case REXXD_int:                    /* integer value                     */
      if (*((int *)*ivalues) != (int)NO_INT)/* no value?                         */
      {
                                         /* return result as an integer object*/
          tempValues = *((int *)*ivalues);
          result = new_integer(tempValues);
      }
      break;

    case REXXD_long:                   /* long integer value                */
                                       /* no return value given?            */
      if (*((long *)*ivalues) != (long)NO_LONG)
      {
                                         /* return as an integer value        */
          tempValues = *((long *)*ivalues);
          result = new_integer(tempValues);
      }
      break;

    case REXXD_double:                 /* double value                      */
                                       /* no value returned?                */
      if (*((double *)*ivalues) != NO_DOUBLE)
      {
                                         /* format the result as a string     */
          result = new_string(*((double *)*ivalues));
      }
      break;

    case REXXD_CSTRING:                /* ASCII-Z string                    */
                                       /* no string returned?               */
      if (*((CSTRING *)*ivalues) != NO_CSTRING)
      {
                                         /* convert to a string object        */
          result = new_string(*((CSTRING *)*ivalues));
      }
      break;

    case REXXD_POINTER:
      if (*((void **)*ivalues) != NULL)/* no pointer?                       */
      {
                                         /* make this an integer value        */
          result = new_pointer(*((void **)*ivalues));
      }
      break;

    default:
      logic_error("unsupported native method result type");
      break;
  }

  // Use protected object to pass back the result
  holdObject(result);                  /* get result held longer            */
  this->guardOff();                    /* release any variable locks        */
  this->argcount = 0;                  /* make sure we don't try to mark any arguments */
  this->activity->pop(FALSE);          /* pop this from the activity        */
  this->setHasNoReferences();          /* mark this as not having references in case we get marked */
  return (RexxObject *)result;         /* and finished                      */
}

RexxObject *RexxNativeActivation::saveObject(
    RexxObject *objr)                  /* object to save                    */
/******************************************************************************/
/* Function:  Protect an object until the native activation terminates        */
/******************************************************************************/
{
  if (objr != OREF_NULL) {             /* have an object?                   */
                                       /* this the first object?            */
    if (this->firstSavedObject == OREF_NULL)
      this->firstSavedObject = objr;   /* save this reference here          */

    else {
      if (this->savelist == OREF_NULL) /* second saved object?              */
                                       /* create the save list now          */
        this->savelist = new_object_table();
                                       /* add to the save table             */
      this->savelist->put(TheNilObject, objr);
    }
  }
  return objr;                         /* return this object                */
}

RexxVariableDictionary *RexxNativeActivation::methodVariables()
/******************************************************************************/
/* Function:  Retrieve the set of method variables                            */
/******************************************************************************/
{
                                       /* first retrieval?                  */
  if (this->objectVariables == OREF_NULL) {
                                       /* is the receiver an activation?    */
    if (isOfClass(Activation,this->receiver))
                                       /* retrieve the method variables     */
      this->objectVariables = ((RexxActivation *)this->receiver)->getLocalVariables();
    else {
                                       /* must be wanting the ovd set of    */
                                       /*variables                          */
      this->objectVariables = (RexxVariableDictionary *)this->receiver->getObjectVariables(this->method->getScope());
                                       /* guarded method?                   */
      if (this->object_scope == SCOPE_RELEASED && this->method->isGuarded()) {
                                       /* reserve the variable scope        */
        this->objectVariables->reserve(this->activity);
                                       /* and remember for later            */
        this->object_scope = SCOPE_RESERVED;
      }
    }
  }
  return this->objectVariables;        /* return the dictionary             */
}


bool RexxNativeActivation::isInteger(
    RexxObject *object)                /* object to validate                */
/******************************************************************************/
/* Function:  Validate that an object has an integer value                    */
/******************************************************************************/
{
                                       /* does it convert?                  */
  return object->longValue(this->digits()) != (long)NO_LONG;
}

const char *RexxNativeActivation::cstring(
    RexxObject *object)                /* object to convert                 */
/******************************************************************************/
/* Function:  Return an object as a CSTRING                                   */
/******************************************************************************/
{
  RexxString *string;                  /* object string value               */

                                       /* force to a string value           */
  string = (RexxString *)object->stringValue();
  if (string != object)                /* different value?                  */
                                       /* make it safe                      */
    this->saveObject((RexxObject *)string);
  return string->getStringData();           /* just point to the string data     */
}

double RexxNativeActivation::getDoubleValue(
    RexxObject *object)                /* object to convert                 */
/******************************************************************************/
/* Function:  Convert an object to a double                                   */
/******************************************************************************/
{
  double r;                            /* returned result                   */

                                       /* convert and check result          */
  if ((r = object->doubleValue()) == NO_DOUBLE)
                                       /* conversion error                  */
    reportException(Error_Execution_nodouble, object);
  return r;                            /* return converted number           */
}

bool RexxNativeActivation::isDouble(
    RexxObject *object)                /* object to check                   */
/******************************************************************************/
/* Function:  Test to see if an object is a valid double                      */
/******************************************************************************/
{
                                       /* does it convert?                  */
  return object->doubleValue() != NO_DOUBLE;
}

PVOID RexxNativeActivation::cself()
/******************************************************************************/
/* Function:  Returns "unwrapped" C or C++ object associated with this        */
/*            object instance.  If the variable CSELF does not exist, then    */
/*            NULL is returned.                                               */
/******************************************************************************/
{
  RexxInteger *C_self;                 /* accessed pointer object           */

                                       /* retrieve from object dictionary   */
  C_self = (RexxInteger *)this->methodVariables()->realValue(OREF_CSELF);
  if (C_self != OREF_NULL)             /* got an item?                      */
    return (PVOID)C_self->getValue();  /* return the pointer value          */
  else
    return NULL;                       /* no object available               */
}

PVOID RexxNativeActivation::buffer()
/******************************************************************************/
/* Function:  Returns "unwrapped" C or C++ object stored in a buffer object.  */
/*            If the variable CSELF does not exist, then NULL is returned.    */
/******************************************************************************/
{
  RexxBuffer  *C_self;                 /* accessed pointer object           */

                                       /* retrieve from object dictionary   */
  C_self = (RexxBuffer *)this->methodVariables()->realValue(OREF_CSELF);
  if (C_self != OREF_NULL)             /* got an item?                      */
    return (PVOID)C_self->address();   /* return a pointer to the address   */
  else
    return NULL;                       /* no object available               */
}

PVOID RexxNativeActivation::pointer(
    RexxObject *object)                /* object to convert                 */
/******************************************************************************/
/* Function:  Return as a pointer the value of an integer                     */
/******************************************************************************/
{
                                       /* just "unwrap" the pointer         */
  return (PVOID)((RexxInteger *)object)->getValue();
}

RexxObject   *RexxNativeActivation::dispatch()
/******************************************************************************/
/* Function:  Redispatch an activation on a different activity                */
/******************************************************************************/
{
  return this->run(0, NULL);           /* just do a method run              */
}

void RexxNativeActivation::traceBack(
    RexxList *traceback_list)          /* list of traceback items           */
/******************************************************************************/
/* Function:  Add a trace back item to the list.  For native activations,     */
/*            this is a no-op.                                                */
/******************************************************************************/
{
  return;                              /* just return                       */
}

size_t RexxNativeActivation::digits()
/******************************************************************************/
/* Function:  Return the current digits setting                               */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct == (RexxActivation *)TheNilObject)
    return Numerics::DEFAULT_DIGITS;   /*  no, just return default value    */
  else
    return senderAct->digits();        /* pass on the the sender            */
}

size_t RexxNativeActivation::fuzz()
/******************************************************************************/
/* Function:  Return the current fuzz setting                                 */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct == (RexxActivation *)TheNilObject)
    return Numerics::DEFAULT_FUZZ;     /*  no, just return default value    */
  else
    return senderAct->fuzz();          /* pass on the the sender            */
}

bool RexxNativeActivation::form()
/******************************************************************************/
/* Function:  Return the curren form setting                                  */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct == (RexxActivation *)TheNilObject)
    return Numerics::DEFAULT_FORM;     /*  no, just return default value    */
  else
    return senderAct->form();          /* pass on the the sender            */
}

void RexxNativeActivation::setDigits(
    size_t _digits)                     /* new NUMERIC DIGITS value          */
/******************************************************************************/
/* Function:  Set a new numeric digits setting                                */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct != (RexxActivation *)TheNilObject)
    senderAct->setDigits(_digits);      /* just forward the set              */
}

void RexxNativeActivation::setFuzz(
    size_t _fuzz )                     /* new NUMERIC FUZZ value            */
/******************************************************************************/
/* Function:  Set a new numeric fuzz setting                                  */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct != (RexxActivation *)TheNilObject)
    senderAct->setFuzz(_fuzz);         /* just forward the set              */
}

void RexxNativeActivation::setForm(
    bool _form )                        /* new NUMERIC FORM value            */
/******************************************************************************/
/* Function:  Set a new numeric form setting                                  */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct != (RexxActivation *)TheNilObject)
    senderAct->setForm(_form);          /* just forward the set              */
}

void      RexxNativeActivation::guardOff()
/******************************************************************************/
/* Function:  Release a variable pool guard lock                              */
/******************************************************************************/
{
                                       /* currently locked?                 */
  if (this->object_scope == SCOPE_RESERVED) {
                                       /* release the variable dictionary   */
    this->objectVariables->release(this->activity);
                                       /* set the state to released         */
    this->object_scope = SCOPE_RELEASED;
  }
}

void RexxNativeActivation::guardOn()
/******************************************************************************/
/* Function:  Acquire a variable pool guard lock                              */
/******************************************************************************/
{
    /* is the receiver an activation?  Just return without locking */
    if (isOfClass(Activation,this->receiver)) {
        return;
    }
    /* first retrieval? */
    if (this->objectVariables == OREF_NULL) {
        /* grab the object variables associated with this object */
        this->objectVariables = (RexxVariableDictionary *)this->receiver->getObjectVariables(this->method->getScope());
    }
    /* not currently holding the lock? */
    if (this->object_scope == SCOPE_RELEASED) {
      /* reserve the variable scope */
      this->objectVariables->reserve(this->activity);
      /* and remember for later */
      this->object_scope = SCOPE_RESERVED;
    }
}

void RexxNativeActivation::enableVariablepool()
/******************************************************************************/
/* Function:  Enable the variable pool                                        */
/******************************************************************************/
{
  this->resetNext();                   /* reset fetch next calls            */
  this->vpavailable = true;            /* allow the calls                   */
}

void RexxNativeActivation::disableVariablepool()
/******************************************************************************/
/* Function:  Disable the variable pool                                       */
/******************************************************************************/
{
  this->resetNext();                   /* reset fetch next calls            */
  this->vpavailable = false;           /* no more external calls            */
}

void RexxNativeActivation::resetNext()
/******************************************************************************/
/* Function: Reset the next state of the variable pool                        */
/******************************************************************************/
{
  this->nextvariable = -1;             /* turn off next index               */
  this->nextcurrent = OREF_NULL;       /* clear the next value              */
  this->nextstem = OREF_NULL;          /* clear the secondary pointer       */
  this->compoundelement = OREF_NULL;
}


BOOL RexxNativeActivation::fetchNext(
    RexxString **name,                 /* the returned name                 */
    RexxObject **value)                /* the return value                  */
/******************************************************************************/
/* Function:  Fetch the next variable of a variable pool traversal            */
/******************************************************************************/
{
  RexxVariable *variable;              /* retrieved variable value          */
  RexxCompoundElement *compound;       /* retrieved variable value          */
  RexxStem     *stemVar;               /* a potential stem variable collection */

                                       /* starting off fresh?               */
  if (nextCurrent() == OREF_NULL) {
    /* grab the activation context */
    RexxActivation *act = activity->getCurrentActivation();
    setNextVariable(-1);               /* request the first item            */
    /* Get the local variable dictionary from the context. */
    setNextCurrent(act->getLocalVariables());
                                       /* we are not on a stem              */
    setNextStem(OREF_NULL);
    setCompoundElement(OREF_NULL);
  }

  for (;;) {                           /* loop until we get something       */
    stemVar = nextStem();              /* see if we're processing a stem variable */
    if (stemVar != OREF_NULL) {        /* were we in the middle of processing a stem? */
        compound = stemVar->nextVariable(this);
        if (compound != OREF_NULL) {   /* if we still have elements here */
                                       /* create a full stem name           */
          *name = compound->createCompoundName(stemVar->getName());
                                       /* get the value                     */
          *value = compound->getVariableValue();
          return TRUE;
        }
        else {                         /* we've reached the end of the stem, reset */
                                       /* to the main dictionary and continue */
            setNextStem(OREF_NULL);
            setCompoundElement(OREF_NULL);
        }
    }
                                       /* get the next variable             */
    variable = nextCurrent()->nextVariable(this);
    if (variable == OREF_NULL) {       /* reached the end of the table      */
        return FALSE;
    }
    else {                             /* have a real variable              */
                                       /* get the value                     */
      RexxObject *variable_value = variable->getVariableValue();
                                       /* found a stem item?                */
      if (isOfClass(Stem, variable_value)) {
                                       /* we are not on a stem              */
        setNextStem((RexxStem *)variable_value);
        setCompoundElement(((RexxStem *)variable_value)->first());
                                       /* set up an iterator for the stem   */
      }
      else {                           /* found a real variable             */
        *value = variable_value;       /* pass back the value (name already set) */
        *name = variable->getName();
        return TRUE;                   /* we have another variable to return */
      }
    }
  }
}


bool RexxNativeActivation::trap(
    RexxString    * condition,         /* name of the condition             */
    RexxDirectory * exception_object)  /* exception information             */
/******************************************************************************/
/*                                                                            */
/*  Function: In almost all cases NativeActs don't care about conditions      */
/*     however in the case of Message objects, they need to know about        */
/*     conditions so cleanups can be done.  We know we are to notify a        */
/*     message object by checking our objnotify field, if ther is anything    */
/*     there, it will be a message object and we simple send a error message  */
/*     to this object.                                                        */
/*                                                                            */
/******************************************************************************/
{
                                       /* is this a syntax condition?       */
  if (condition->strCompare(CHAR_SYNTAX)) {
                                       /* do we need to notify a message    */
                                       /*obj?                               */
    if (this->objnotify != OREF_NULL)
                                       /* yes, send error message and       */
                                       /* condition to the object           */
      this->objnotify->error(exception_object);
  }
  return FALSE;                        /* this wasn't handled               */
}

void RexxNativeActivation::setObjNotify(
    RexxMessage *notify)               /* message object to notify          */
/******************************************************************************/
/* Function:  Put a notification trap on syntax conditions on this activation.*/
/******************************************************************************/
{
  this->objnotify = notify;            /* save the notification             */
}


void * RexxNativeActivation::operator new(size_t size,
     RexxObject         * receiver,    /* receiver object                   */
     RexxMethod         * method,      /* method to run                     */
     RexxActivity       * activity,    /* current activity                  */
     RexxString         * msgname,     /* invoked message                   */
     RexxActivationBase * activation)  /* current activation                */
/******************************************************************************/
/* Function:  Create a new native activation object                           */
/******************************************************************************/
{
  RexxNativeActivation * newObject;    /* new activation object             */

                                       /* Get new object                    */
  newObject = (RexxNativeActivation *)new_object(size);
                                       /* Give new object its behaviour     */
  newObject->setBehaviour(TheNativeActivationBehaviour);
  newObject->clearObject();            /* clear out at start                */
  newObject->receiver = receiver;      /* the receiving object              */
  newObject->method = method;          /* the method to run                 */
  newObject->activity = activity;      /* the activity running on           */
  newObject->msgname = msgname;        /* the message name                  */
  newObject->argcount = 0;             /* no arguments until we've been given some */
  return (RexxObject *)newObject;      /* return the new object             */
}

#define this ((RexxNativeActivation *)self)

nativei0 (REXXOBJECT, MSGNAME)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxNativeActivation *self;          /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  return_object(this->msgname);          /* just forward and return           */
}

nativei0 (REXXOBJECT, RECEIVER)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  return_object(this->receiver);         /* just forward and return           */
}

nativei1 (int, INTEGER, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  int     result;                      /* returned result                   */

  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  result = ((RexxObject *)object)->longValue(((RexxNativeActivation *)ActivityManager::currentActivity->current())->digits());
  return_value(result);                /* return converted value            */
}

nativei1 (size_t, UNSIGNED_INTEGER, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  size_t            result;            /* returned result                   */
  RexxNumberString *argNumStr;         /* numberstring version of this      */

  native_entry;                        /* synchronize access                */

  result = (ULONG)NO_LONG;             /* set a default return value        */
                                       /* First convert to numberstring     */
  argNumStr = ((RexxObject *)object)->numberString();
                                       /* Did object convert?               */
  if (argNumStr != OREF_NULL)          /* convert to a numberstring?        */
    argNumStr->ULong(&result);         /* now convert to a ulong            */
  return_value(result);                /* return converted value            */
}

nativei1 (BOOL, ISINTEGER, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  BOOL result;                         /* returned result                   */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
                                       /* just forward and return           */
  result = this->isInteger((RexxObject *)object);
  return_value(result);                /* return converted value            */
}

nativei1 (BOOL, ISASTRING,
    REXXOBJECT, object )                /* object to check                   */
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  BOOL   result;                       /* returned result                   */

  native_entry;                        /* synchronize access                */
                                       /* check that this has correct       */
                                       /* instance behavior                 */
  result = ((RexxObject *)object)->getObjectType() == TheStringBehaviour;
  return_value(result);                /* return indicator                  */
}

nativei1 (CSTRING, STRING, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  const char *result;                  /* returned result                   */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
                                       /* just forward and return           */
  result = this->cstring((RexxObject *)object);
  return_value(result);                /* return converted value            */
}

nativei1 (double, DOUBLE, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  double  result;                      /* returned result                   */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
                                       /* just forward and return           */
  result = this->getDoubleValue((RexxObject *)object);
  return_value(result);                /* return converted value            */
}

nativei1 (BOOL, ISDOUBLE, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  BOOL    result;                      /* returned result                   */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
                                       /* just forward and return           */
  result = this->isDouble((RexxObject *)object);
  return_value(result);                /* return converted value            */
}

nativei3 (REXXOBJECT, SEND,
     REXXOBJECT,  receiver,            /* receiver of the message           */
     CSTRING,     msgname,             /* the name of the message           */
     REXXOBJECT,  arguments)           /* message arguments                 */
/******************************************************************************/
/* Function:  Issue a full scale send_message from native code                */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
  return_object((RexxObject *)receiver->sendMessage((RexxString *)new_string(msgname), (RexxArray *)arguments));
}

nativei2 (REXXOBJECT, SUPER,
     char *,       msgname,            /* message name                      */
     REXXOBJECT,  arguments)           /* argument array                    */
/******************************************************************************/
/* Function:  Forward a message to a super class from native code             */
/******************************************************************************/
{
  size_t          count;               /* count of arguments                */
  RexxObject     *argarray[10];        /* C array of arguments              */
  RexxArray      *args = (RexxArray *)arguments;
  size_t          i;                   /* loop counter                      */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  count = args->size();                /* get the argument count            */
  for (i = 1; i <= count; i++)         /* loop through the array            */
                                       /* copying each OREF                 */
    argarray[i-1] = args->get(i);
                                       /* now send the message              */
  return_object(this->receiver->messageSend((RexxString *)new_string(msgname), count, argarray, this->receiver->superScope(this->method->getScope())));
}

nativei2 (REXXOBJECT, SETVAR,
     CSTRING, name,                    /* variable name                     */
     REXXOBJECT,  value )              /* new variable value                */
/******************************************************************************/
/* Function:  Set the value of an object variable                             */
/******************************************************************************/
{
  RexxNativeActivation   * self;       /* current native activation         */
  RexxVariableDictionary * dictionary; /* target dictionary                 */
                                       /* variable name                     */
  RexxString             * variableName;

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  dictionary = self->methodVariables();/* get the method variables          */
  variableName = new_string(name);    /* get a string version of this      */
                                       /* do the assignment                 */
  dictionary->set(variableName, (RexxObject *)value);
  return_object(OREF_NULL);              /* return nothing                    */
}

nativei2 (REXXOBJECT, SETVAR2,
     char *, name,                     /* variable name                     */
     REXXOBJECT,  value )              /* new variable value                */
/******************************************************************************/
/* Function:  Set the value of a variable in current variable pool            */
/******************************************************************************/
{
  RexxNativeActivation   * self;       /* current native activation         */
                                       /* variable name                     */
  RexxString             * variableName;
  RexxVariableBase       * retriever;

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  RexxActivation *context = (RexxActivation *)this->activity->sender(this);
  variableName = new_string(name);    /* get a string version of this      */
                                       /* do the assignment throug retriever*/
  retriever = context->getVariableRetriever(variableName);
  retriever->set(context, (RexxObject *)value);
  return_object(OREF_NULL);              /* return nothing                    */
}


nativei2 (REXXOBJECT, SETFUNC,
     char *, name,                     /* function name                     */
     REXXOBJECT,  value )              /* method                            */
{
  RexxNativeActivation   * self;       /* current native activation         */

  RexxActivity           * activity;
  RexxActivation         * activation;

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */

  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  activity = self->activity;           /* find the current activity         */
                                       /* get the current activation        */
  activation = self->activity->getCurrentActivation();

  activation->addLocalRoutine(new_string(name), (RexxMethod *) value);
  return_object(OREF_NULL);              /* return nothing                    */
}

/*******************************************************1***/
/* return the names of all public routines in the current */
/* activation context. *the caller* must free the memory! */
/**********************************************************/
nativei2 (REXXOBJECT, GETFUNCTIONNAMES,
     char***, names,            /* array to hold names         */
     int *,  num)               /* number of elements in array */
{
  RexxNativeActivation   * self;       /* current native activation         */

  RexxActivity           * activity;
  RexxActivation         * activation;
  RexxArray              * funcArray;
  RexxString             * name;
  int                      i;
  int                      j;

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */

  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  activity = self->activity;           /* find the current activity         */
                                       /* get the current activation        */
  activation = self->activity->getCurrentActivation();

  *num = 0;
  RexxDirectory *routines = activation->getPublicRoutines();

  if (routines != OREF_NULL) {
    funcArray = routines->makeArray();
    if (funcArray != OREF_NULL) {
      *num = j = funcArray->numItems();
      *names = (char**) SysAllocateExternalMemory(sizeof(char*)*j);
      for (i=0;i<j;i++) {
        name = ((RexxString*) funcArray->get(i+1));
        (*names)[i] = (char*) SysAllocateExternalMemory(1+sizeof(char)*name->getLength());
        memcpy((*names)[i], name->getStringData(), name->getLength());
        (*names)[i][name->getLength()] = 0; // zero-terminate
      }
    }
  }

  return_object(OREF_NULL);              /* return nothing                    */
}

nativei1 (REXXOBJECT, GETVAR,
    CSTRING, name )                      /* variable name                     */
/******************************************************************************/
/* Function:  Retrieve the value of an object variable                        */
/******************************************************************************/
{
  RexxNativeActivation   * self;       /* current native activation         */
  RexxVariableDictionary * dictionary; /* target dictionary                 */
                                       /* variable name                     */
  RexxString             * variableName;
  RexxObject             * value;      /* returned variable value           */


  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  dictionary = self->methodVariables();/* get the method variables          */
  variableName = new_string(name);    /* get a string version of this      */
                                       /* go get the variable               */
  value = dictionary->realValue(variableName);
  if (value == OREF_NULL)              /* uninitialized?                    */
    value = (RexxObject *)variableName;/* just return the name              */
  native_release(OREF_NULL);           /* release the kernel access         */
                                       /* now return WITHOUT saving the     */
                                       /* variable's value.  This is already*/
                                       /* protected by the object's variable*/
  return value;                        /* pool                              */
}

nativei2 (void, EXCEPT,
     int,  errorcode,                  /* error message number              */
     REXXOBJECT, value )               /* substitution value                */
/******************************************************************************/
/* Function:  Raise an exception on behalf of native code                     */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
  if (value == OREF_NULL)              /* just a NULL value?                */
    reportException(errorcode);        /* no substitution parameters        */
  else                                 /* use the substitution form         */
    reportException(errorcode, (RexxArray *)value);
}

nativei4 (void, RAISE,
     CSTRING, condition,               /* name of the condition             */
     REXXOBJECT,  description,         /* description object                */
     REXXOBJECT,  additional,          /* additional information            */
     REXXOBJECT,  result )             /* optional result                   */
/******************************************************************************/
/* Function:  Raise a condition on behalf of native code                      */
/******************************************************************************/
{
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  this->result = (RexxObject *)result; /* save the result                   */
                                       /* go raise the condition            */
  ActivityManager::currentActivity->raiseCondition(new_string(condition), OREF_NULL, (RexxString *)description, (RexxObject *)additional, (RexxObject *)result, OREF_NULL);

  // Using throw would be preferred here, but the compiler won't allow us to
  // do a throw across the C function call boundary.  We need to use longjmp
  // instead. This won't effect our object unwinding yet, as we're in control
  // of things.  It would be better if the functions used RAISE and then
  // returned to the caller, but that's a bit more involved at this point.
  longjmp(this->conditionjump, 1);     /* now go process the return         */
}

nativei3 (REXXOBJECT, CONDITION,
     REXXOBJECT, condition,            /* condition to raise              */
     REXXOBJECT, description,          /* description information         */
     REXXOBJECT, additional )          /* additional exception info       */
/******************************************************************************/
/* Function:  Raise a condition on behalf of native code                      */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
                                       /* pass on and raise the condition   */
  return_object((ActivityManager::currentActivity->raiseCondition((RexxString *)condition, OREF_NULL, (RexxString *)description, (RexxObject *)additional, OREF_NULL, OREF_NULL)) ? TheTrueObject : TheFalseObject);
}

nativei1 (ULONG, VARIABLEPOOL,
     PVOID, pshvblock )                /* chain of variable request blocks  */
/******************************************************************************/
/* Function:  If variable pool is enabled, return result from SysVariablePool */
/*             method, otherwise return RXSHV_NOAVL.                          */
/******************************************************************************/
{
  ULONG    result;                     /* variable pool result              */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
                                       /* if access is enabled              */
  if (this->getVpavailable())
                                       /* go process the requests           */
    result = SysVariablePool(self, pshvblock, TRUE);
  else                                 /* Else VP is disabled so...         */
                                       /* call VP only allowing shv_exit    */
    result = SysVariablePool(self, pshvblock, FALSE);
  return_value(result);                /* return this                       */
}

nativei1 (ULONG, VARIABLEPOOL2,
     PVOID, pshvblock )                /* chain of variable request blocks  */
/******************************************************************************/
/* Arguments:  pshvblock - Pointer to a variable request block with           */
/*                         shvnext being either zero (which means use         */
/*                         current activation), or an OREF for a              */
/*                         nativeactobj when we left the kernel.              */
/*                                                                            */
/* Returned:  If variable pool is enabled, return result from SysVariablePool */
/*             method, otherwise return RXSHV_NOAVL.                          */
/******************************************************************************/
{
  ULONG     result;                    /* variable pool result              */
  RexxNativeActivation * self;         /* current native activation         */
  RexxNativeActivation * work;         /* nativeact or zero                 */
  PSHVBLOCK shv;                       /* shvblock address                  */

  native_entry;                        /* synchronize access                */
                                       /* get activation                    */
  shv  = (PSHVBLOCK)pshvblock;
  work = (RexxNativeActivation *)(shv->shvnext);
  if (work) {
     self = work;
     shv->shvnext = 0;                 /* remember, only 1 shvblock         */
     }
                                       /* pick up current activation        */
     else self = (RexxNativeActivation *)ActivityManager::currentActivity->current();

                                       /* if access is enabled              */
  if (this->getVpavailable())
                                       /* go process the requests           */
    result = SysVariablePool(self, pshvblock, TRUE);
  else                                 /* Else VP is disabled so...         */
                                       /* call VP only allowing shv_exit    */
    result = SysVariablePool(self, pshvblock, FALSE);
  return_value(result);                /* return this                       */
}

nativei0 (void, ENABLE_VARIABLEPOOL)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  this->enableVariablepool();          /* go enable the pool                */
  return_void;                         /* no return value                   */
}

nativei0 (void, DISABLE_VARIABLEPOOL)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
  this->vpavailable = false;           /* dis-allow the calls               */
  return_void;                         /* no return value                   */
}

 nativei1 (void, PUSH_ENVIRONMENT, REXXOBJECT, environment)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxActivation *activation;          /* top level real activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  activation = (RexxActivation *)ActivityManager::currentActivity->getCurrentActivation();
  activation->pushEnvironment((RexxObject *)environment);
  return_void;                         /* no return value                   */
}

nativei0 (REXXOBJECT, POP_ENVIRONMENT)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxActivation *activation;          /* top level real activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  activation = (RexxActivation *)ActivityManager::currentActivity->getCurrentActivation();
  return_object(activation->popEnvironment());
}

BOOL REXXENTRY REXX_ISDIRECTORY(REXXOBJECT object)
/******************************************************************************/
/* Function:  Validate that an object is a directory                          */
/******************************************************************************/
{
                                       /* do the validation                 */
  return object != NULL && isOfClass(Directory, object);
}

REXXOBJECT REXXENTRY REXX_NIL(void)
/******************************************************************************/
/* Function:  Return REXX .nil object to native code                          */
/******************************************************************************/
{
  return TheNilObject;                 /* just return the object            */
}

REXXOBJECT REXXENTRY REXX_TRUE(void)
/******************************************************************************/
/* Function:  Return REXX TRUE object to native code                          */
/******************************************************************************/
{
  return TheTrueObject;                /* just return the object            */
}

REXXOBJECT REXXENTRY REXX_FALSE(void)
/******************************************************************************/
/* Function:  Return REXX FALSE object to native code                         */
/******************************************************************************/
{
  return TheFalseObject;               /* just return the object            */
}

REXXOBJECT REXXENTRY REXX_LOCAL(void)
/******************************************************************************/
/* Function:  Return REXX .local object to native code                        */
/******************************************************************************/
{
  return ActivityManager::localEnvironment;  /* just return the local environment */
}

REXXOBJECT REXXENTRY REXX_ENVIRONMENT(void)
/******************************************************************************/
/* Function:  Return REXX .environment object to native code                  */
/******************************************************************************/
{
  return TheEnvironment;               /* just return the object            */
}

nativei7 (ULONG, STEMSORT,
     CSTRING, stemname, int, order, int, type, size_t, start, size_t, end,
     size_t, firstcol, size_t, lastcol)
/******************************************************************************/
/* Function:  Perform a sort on stem data.  If everything works correctly,    */
/*             this returns zero, otherwise an appropriate error value.       */
/******************************************************************************/
{
  size_t  position;                    /* scan position within compound name */
  size_t  length;                      /* length of tail section            */
  int result;

  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
                                       /* if access is enabled              */
  if (!self->getVpavailable()) {       /* access must be enabled for this to work */
      return_value(FALSE);
  }

  // NB:  The braces here are to ensure the ProtectedObjects get released before the
  // currentActivity gets zeroed out.
  {
      /* get the REXX activation */
      RexxActivation *activation = self->activity->getCurrentActivation();

      /* get the stem name as a string */
      RexxString *variable = new_string(stemname);
      ProtectedObject p1(variable);
      /* and get a retriever for this variable */
      RexxStemVariable *retriever = (RexxStemVariable *)activation->getVariableRetriever(variable);

      /* this must be a stem variable in order for the sorting to work. */

      if ( (!isOfClass(StemVariable, retriever)) && (!isOfClass(CompoundVariable, retriever)) )
      {
          return FALSE;
      }

    //RexxString *tail = (RexxString *) new_string(OREF_NULL);
      RexxString *tail = OREF_NULLSTRING ;
      ProtectedObject p2(tail);

      if (isOfClass(CompoundVariable, retriever))
      {
        length = variable->getLength();      /* get the string length             */
        position = 0;                        /* start scanning at first character */
                                           /* scan to the first period          */
        while (variable->getChar(position) != '.')
        {
          position++;                        /* step to the next character        */
          length--;                          /* reduce the length also            */
        }
        position++;                          /* step past previous period         */
        length--;                            /* adjust the length                 */
        tail = variable->extract(position, length);
        tail = tail->upper();
      }

      result = retriever->sort(activation, tail, order, type, start, end, firstcol, lastcol);
  }

  return_value(result);
}

nativei0 (void, GUARD_ON)
/******************************************************************************/
/* Function:  External interface to implement method guarding                 */
/******************************************************************************/
{
    RexxNativeActivation   * self;       /* current native activation         */

    native_entry;                        /* synchronize access                */
                                         /* pick up current activation        */
    self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
    self->guardOn();                     /* turn on the guard                 */
    native_release(OREF_NULL);           /* release the kernel access         */
}

nativei0 (void, GUARD_OFF)
/******************************************************************************/
/* Function:  External interface to implement method guarding                 */
/******************************************************************************/
{
    RexxNativeActivation   * self;       /* current native activation         */

    native_entry;                        /* synchronize access                */
                                         /* pick up current activation        */
    self = (RexxNativeActivation *)ActivityManager::currentActivity->current();
    self->guardOff();                    /* turn off the guard                 */
    native_release(OREF_NULL);           /* release the kernel access         */
}

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
/* REXX Kernel                                                  RexxNativeActivation.c    */
/*                                                                            */
/* Primitive Native Activation Class                                          */
/*                                                                            */
/******************************************************************************/
#define INCL_REXXSAA
#include <setjmp.h>

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


#include "RexxNativeAPI.h"                      /* bring in the native code defines  */
#undef   RexxTable                     /* remove a conflict                 */

#include "ASCIIDBCSStrings.hpp"
#include SYSREXXSAA
#ifdef SOM
  #include "orxsminf.xh"
  #include "repostry.xh"
  #include "somcls.xh"
  #include "orxsom.h"                  /* SOM client declarations */

  RexxObject *resolve_proxy(SOMObject *);
#endif

extern RexxObject *ProcessLocalServer;
extern RexxDirectory *ProcessLocalEnv; /* process local environment (.local)*/
extern ACTIVATION_SETTINGS *current_settings;

typedef void *somRef;
typedef void *somTok;
                                       /* sizes of different argument       */
                                       /* types                             */
static size_t tsize[] = { 0, 0,
                          sizeof(RexxObject *),
                          sizeof(int),
                          sizeof(long),
                          sizeof(double),
                          sizeof(CSTRING),
                          sizeof(RexxObject *),      /* OSELF */
                          sizeof(somRef),            /* SOMSELF */
                          sizeof(somRef),
                          sizeof(somTok),
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
  memory_mark(this->u_receiver);
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
  SHORT  i;
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
  memory_mark_general(this->u_receiver);
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
  SHORT  i;
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
     flatten_reference(newThis->u_receiver, envelope);
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

somRef nativeact_somref(
   RexxObject *objr)                   /* object to proxy                   */
/******************************************************************************/
/* Function:  Produce a SOM object reference from a REXX object reference     */
/******************************************************************************/
{
  RexxInteger *somReference;           /* SOM reference object              */
                                       /* ask the server to convert         */
  somReference = (RexxInteger *)send_message1(ProcessLocalServer,OREF_SOMSYM,objr);
  return (somRef)somReference->value;  /* return the actual reference part  */
}

RexxObject *RexxNativeActivation::run(
    size_t       argcount,             /* argument count                    */
    RexxObject **arglist)              /* method argument list              */
/******************************************************************************/
/* Function:  Execute a REXX native method                                    */
/******************************************************************************/
{
  RexxObject * result;                 /* returned method result            */
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

  this->arglist = arglist;             /* save the argument information     */
  this->argcount = (SHORT)argcount;    /* set the argument count            */
  used_arglist = FALSE;                /* no arglist requested              */
                                       /* get the entry point address       */
  methp = (PNMF)this->method->nativeCode->getEntry();
  itypes = (*methp)(0);                /* get type information from method  */
  *ivalues = ibufp;
  ibufp += tsize[*itypes];             /* step over first type              */
  i = 0;                               /* no arguments used yet             */
                                       /* loop through the type information */
  for (tp = itypes+1, ivalp = ivalues+1; *tp; tp++, ivalp++) {
    *ivalp = ibufp;
    ibufp += tsize[*tp];               /* step over the type                */
    switch (*tp) {                     /* switch based on this type         */

      case REXXD_OSELF:                /* reference to SELF                 */
        *((RexxObject **)*ivalp) = this->u_receiver;
        break;

      case REXXD_CSELF:                /* reference to CSELF                */
        *((void **)*ivalp) = this->cself();
        break;

      case REXXD_BUFFER:               /* reference to Buffered storage     */
        *((void **)*ivalp) = this->buffer();
        break;

      case REXXD_SOMSELF:              /* this is a SOM reference           */
        *((somRef *)*ivalp) = nativeact_somref(this->u_receiver);
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
              if (tempValues == NO_LONG)
                                       /* this is an error                  */
                report_exception2(Error_Incorrect_method_whole, new_integer(i+1), argument);
                                       /* copy over the info                */
              *((int *)*ivalp) = (int)tempValues;
              break;

            case REXXD_long:           /* long value                        */
                                       /* try to convert the value          */
              tempValues = argument->longValue(this->digits());
                                       /* not convertable?                  */
              if (tempValues == NO_LONG)
                                       /* this is an error                  */
                report_exception2(Error_Incorrect_method_whole, new_integer(i+1), argument);
                                       /* copy over the info                */
              *((long *)*ivalp) = tempValues;
              break;

            case REXXD_double:         /* double value                      */
              *((double *)*ivalp) = this->getDoubleValue(argument);
              break;

            case REXXD_CSTRING:        /* ASCII-Z string value              */
              *((CSTRING *)*ivalp) = this->cstring(argument);
              break;

            case REXXD_somRef:         /* a SOM reference                   */
              *((somRef *)*ivalp) = nativeact_somref(argument);
              break;

            case REXXD_somTok:         /* SOM token - pointer to SOM object */
              *((somTok *)*ivalp) = (somTok)this->pointer(argument);
              break;

            case REXXD_STRING:         /* Required STRING object            */
                                       /* force to a string value           */
              result = REQUIRED_STRING(argument, i + 1) ;
              if (result != argument)  /* new object created?               */
                                       /* make it safe                      */
                this->saveObject(result);
                                       /* set the result in                 */
              *((OREF *)*ivalp) = result;
              break;

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

            case REXXD_somRef:         /* no SOM pointer                    */
              *((somRef *)*ivalp) = NULL;
              break;

            case REXXD_somTok:         /* no SOM pointer                    */
              *((somTok *)*ivalp) = NULL;
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
    report_exception1(Error_Incorrect_method_maxarg, new_integer(i));

                                       /* get a RAISE type return?          */
  if (setjmp(this->conditionjump) != 0) {
    if (this->result != OREF_NULL)     /* have a value?                     */
      hold(this->result);              /* get result held longer            */
    this->guardOff();                  /* release any variable locks        */
    this->argcount = 0;                /* make sure we don't try to mark any arguments */
    this->activity->pop(FALSE);        /* pop this from the activity        */
    SetObjectHasNoReferences(this);    /* mark this as not having references in case we get marked */
    return this->result;               /* and finished                      */
  }

  ReleaseKernelAccess(this->activity); /* force this to "safe" mode         */
  (*methp)(ivalues);                   /* process the method call           */
  RequestKernelAccess(this->activity); /* now in unsafe mode again          */

  /* give up reference to receiver so that it can be garbage collected */
  this->u_receiver = OREF_NULL;

  switch (*itypes) {                   /* now process the return value      */
    case REXXD_void:                   /* void, this is just a NULL OREF    */
      result = OREF_NULL;
      break;

    case REXXD_OBJECT:                 /* Object reference                  */
                                       /* no result returned?               */
      if (*((RexxObject **)*ivalues) == OREF_NULL)
        result = OREF_NULL;            /* just give a null pointer          */
      else                             /* give a direct pointer to this     */
        result = *((RexxObject **)*ivalues);
      break;

    case REXXD_int:                    /* integer value                     */
      if (*((int *)*ivalues) == NO_INT)/* no value?                         */
        result = OREF_NULL;            /* no result                         */
      else {                           /* return result as an integer object*/
        tempValues = *((int *)*ivalues);
        result = new_integer(tempValues);
      }
      break;

    case REXXD_long:                   /* long integer value                */
                                       /* no return value given?            */
      if (*((long *)*ivalues) == NO_LONG)
        result = OREF_NULL;            /* don't give a value                */
      else {                           /* return as an integer value        */
        tempValues = *((long *)*ivalues);
        result = new_integer(tempValues);
      }
      break;

    case REXXD_double:                 /* double value                      */
                                       /* no value returned?                */
      if (*((double *)*ivalues) == NO_DOUBLE)
        result = OREF_NULL;            /* make this a null return           */
      else                             /* format the result as a string     */
        result = new_stringd((double *)*ivalues);
      break;

    case REXXD_CSTRING:                /* ASCII-Z string                    */
                                       /* no string returned?               */
      if (*((CSTRING *)*ivalues) == NO_CSTRING)
        result = OREF_NULL;            /* no return value                   */
      else                             /* convert to a string object        */
        result = new_cstring(*((CSTRING *)*ivalues));
      break;

    case REXXD_somRef:                 /* SOM reference                     */
#ifdef SOM
                                       /* no reference as result            */
      if (*((somRef *)*ivalues) == NULL)
        result = OREF_NULL;            /* this had no return                */
      else                             /* resolve this to a proxy object    */
        result = resolve_proxy((SOMObject *)(*((somRef *)*ivalues)));
#else
      result = OREF_NULL;              /* always NULL if SOM is disabled    */
#endif
      break;

    case REXXD_somTok:                 /* SOM token                         */
                                       /* no token returned?                */
      if (*((somTok *)*ivalues) == NULL)
        result = OREF_NULL;            /* null return value                 */
      else                             /* make a new pointer for this       */
        result = new_pointer((PVOID)*((somTok *)*ivalues));
      break;

    case REXXD_POINTER:
      if (*((void **)*ivalues) == NULL)/* no pointer?                       */
        result = OREF_NULL;            /* no value!                         */
      else                             /* make this an integer value        */
        result = new_pointer(*((void **)*ivalues));
      break;

    default:
      logic_error("unsupported native method result type");
      break;
  }

  hold(result);                        /* get result held longer            */
  this->guardOff();                    /* release any variable locks        */
  this->argcount = 0;                  /* make sure we don't try to mark any arguments */
  this->activity->pop(FALSE);          /* pop this from the activity        */
  SetObjectHasNoReferences(this);      /* mark this as not having references in case we get marked */
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
    if (OTYPE(Activation,this->u_receiver))
                                       /* retrieve the method variables     */
      this->objectVariables = ((RexxActivation *)this->u_receiver)->getLocalVariables();
    else {
                                       /* must be wanting the ovd set of    */
                                       /*variables                          */
      this->objectVariables = (RexxVariableDictionary *)this->u_receiver->getObjectVariables(this->method->scope);
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


long RexxNativeActivation::isInteger(
    RexxObject *object)                /* object to validate                */
/******************************************************************************/
/* Function:  Validate that an object has an integer value                    */
/******************************************************************************/
{
                                       /* does it convert?                  */
  if (object->longValue(this->digits()) != NO_LONG)
    return TRUE;                       /* have an integer                   */
  else
    return FALSE;                      /* must not be an integer            */
}

PCHAR RexxNativeActivation::cstring(
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
  return string->stringData;           /* just point to the string data     */
}

double RexxNativeActivation::getDoubleValue(
    RexxObject *object)                /* object to convert                 */
/******************************************************************************/
/* Function:  Convert an object to a double                                   */
/******************************************************************************/
{
  double result;                       /* returned result                   */

                                       /* convert and check result          */
  if ((result = object->doubleValue()) == NO_DOUBLE)
                                       /* conversion error                  */
    report_exception1(Error_Execution_nodouble, object);
  return result;                       /* return converted number           */
}

long  RexxNativeActivation::isDouble(
    RexxObject *object)                /* object to check                   */
/******************************************************************************/
/* Function:  Test to see if an object is a valid double                      */
/******************************************************************************/
{
                                       /* does it convert?                  */
  if (object->doubleValue() != NO_DOUBLE)
    return TRUE;                       /* must be good                      */
  else
    return FALSE;                      /* doesn't work                      */
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
    return (PVOID)C_self->value;       /* return the pointer value          */
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
  return (PVOID)((RexxInteger *)object)->value;
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

long  RexxNativeActivation::digits()
/******************************************************************************/
/* Function:  Return the current digits setting                               */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct == (RexxActivation *)TheNilObject)
    return DEFAULT_DIGITS;             /*  no, just return default value    */
  else
    return senderAct->digits();        /* pass on the the sender            */
}

long RexxNativeActivation::fuzz()
/******************************************************************************/
/* Function:  Return the current fuzz setting                                 */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct == (RexxActivation *)TheNilObject)
    return DEFAULT_FUZZ;               /*  no, just return default value    */
  else
    return senderAct->fuzz();          /* pass on the the sender            */
}

BOOL RexxNativeActivation::form()
/******************************************************************************/
/* Function:  Return the curren form setting                                  */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct == (RexxActivation *)TheNilObject)
    return DEFAULT_FORM;               /*  no, just return default value    */
  else
    return senderAct->form();          /* pass on the the sender            */
}

void RexxNativeActivation::setDigits(
    long digits)                       /* new NUMERIC DIGITS value          */
/******************************************************************************/
/* Function:  Set a new numeric digits setting                                */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct != (RexxActivation *)TheNilObject)
    senderAct->setDigits(digits);      /* just forward the set              */
}

void RexxNativeActivation::setFuzz(
    long fuzz )                        /* new NUMERIC FUZZ value            */
/******************************************************************************/
/* Function:  Set a new numeric fuzz setting                                  */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct != (RexxActivation *)TheNilObject)
    senderAct->setFuzz(fuzz);          /* just forward the set              */
}

void RexxNativeActivation::setForm(
    BOOL form )                        /* new NUMERIC FORM value            */
/******************************************************************************/
/* Function:  Set a new numeric form setting                                  */
/******************************************************************************/
{
                                       /* get the sender object             */
  RexxActivation *senderAct = this->sender();
                                       /* have a real one?                  */
  if (senderAct != (RexxActivation *)TheNilObject)
    senderAct->setForm(form);          /* just forward the set              */
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
    if (OTYPE(Activation,this->u_receiver)) {
        return;
    }
    /* first retrieval? */
    if (this->objectVariables == OREF_NULL) {
        /* grab the object variables associated with this object */
        this->objectVariables = (RexxVariableDictionary *)this->u_receiver->getObjectVariables(this->method->scope);
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
  this->vpavailable = TRUE;            /* allow the calls                   */
}

void RexxNativeActivation::disableVariablepool()
/******************************************************************************/
/* Function:  Disable the variable pool                                       */
/******************************************************************************/
{
  this->resetNext();                   /* reset fetch next calls            */
  this->vpavailable = FALSE;           /* no more external calls            */
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
  RexxStem     *stem;                  /* a potential stem variable collection */
  BOOL result = FALSE;                 /* default to the "we've reached the end" scenario */

                                       /* starting off fresh?               */
  if (nextCurrent() == OREF_NULL) {
    /* grab the activation context */
    RexxActivation *activation = activity->currentAct();
    setNextVariable(-1);               /* request the first item            */
    /* Get the local variable dictionary from the context. */
    setNextCurrent(activation->getLocalVariables());
                                       /* we are not on a stem              */
    setNextStem(OREF_NULL);
    setCompoundElement(OREF_NULL);
  }

  for (;;) {                           /* loop until we get something       */
    stem = nextStem();                 /* see if we're processing a stem variable */
    if (stem != OREF_NULL) {           /* were we in the middle of processing a stem? */
        compound = stem->nextVariable(this);
        if (compound != OREF_NULL) {   /* if we still have elements here */
                                       /* create a full stem name           */
          *name = compound->createCompoundName(stem->getName());
                                       /* get the value                     */
          *value = compound->getVariableValue();
          result = TRUE;
          break;
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
        break;                         /* reached the end                   */
    }
    else {                             /* have a real variable              */
                                       /* get the value                     */
      RexxObject *variable_value = variable->getVariableValue();
                                       /* found a stem item?                */
      if (OTYPE(Stem, variable_value)) {
                                       /* we are not on a stem              */
        setNextStem((RexxStem *)variable_value);
        setCompoundElement(((RexxStem *)variable_value)->first());
                                       /* set up an iterator for the stem   */
      }
      else {                           /* found a real variable             */
        *value = variable_value;       /* pass back the value (name already set) */
        *name = variable->getName();
        result = TRUE;                 /* we have another variable to return */
        break;                         /* finished                          */
      }
    }
  }
  return result;                       /* return the next item              */
}


BOOL RexxNativeActivation::trap(
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
    if (this->syntaxHandler != NULL)   /* have a syntax handler?            */
      longjmp(*this->syntaxHandler, 1);/* jump back to the handler          */
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
  BehaviourSet(newObject, TheNativeActivationBehaviour);
  ClearObject(newObject);              /* clear out at start                */
  newObject->u_receiver = receiver;    /* the receiving object              */
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
  self = (RexxNativeActivation *)CurrentActivity->current();
  return_oref(this->msgname);          /* just forward and return           */
}

nativei0 (BOOL, CURRENT_EXMODE)
/******************************************************************************/
/* Function:  External interface for say to get the activation with exmode set*/
/******************************************************************************/
{
  RexxNativeActivation *self;          /* nativeact object                  */
  RexxActivity        *activity;       /* current activity object           */
  RexxActivation      *activation;     /* current activation object         */
  RexxActivation      *prev_activation;/* previous activation objects       */
  INT    count;                        /* counter for loop                  */
  BOOL   exmode_option;                /* activation settings exmode value  */

  native_entry;                        /* synchronize access                */
                                       /* get nativeact object              */
  self = (RexxNativeActivation *)CurrentActivity->current();
  activity = this->activity;           /* find the current activity         */
                                       /* get the current activation        */
  activation = this->activity->currentAct();
                                       /* the 2nd previous activation will  */
                                       /* contain exmode setting (for say   */
                                       /* instruction only)                 */
  for (count = 1, exmode_option = DBCS_MODE, prev_activation = activation;
       count < 3;  count++) {
                                       /* get next previous activation      */
    prev_activation = (RexxActivation *)this->activity->sender(prev_activation);
                                       /* found activation, get exmode value*/
    if (OTYPE(Activation,prev_activation) || (prev_activation != OREF_NULL))
      exmode_option = prev_activation->settings.global_settings.exmode;
    else                               /* not activation, use prev exmode   */
      break;
  }
  return_value(exmode_option);         /* return first exmode found         */
}

nativei0 (REXXOBJECT, RECEIVER)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)CurrentActivity->current();
  return_oref(this->u_receiver);       /* just forward and return           */
}

nativei1 (long, INTEGER, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  long    result;                      /* returned result                   */

  native_entry;                        /* synchronize access                */
                                       /* just forward and return           */
  result = ((RexxObject *)object)->longValue(((RexxNativeActivation *)CurrentActivity->current())->digits());
  return_value(result);                /* return converted value            */
}

nativei1 (ULONG, UNSIGNED_INTEGER, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  ULONG             result;            /* returned result                   */
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
  self = (RexxNativeActivation *)CurrentActivity->current();
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
  result = HASBEHAV(TheStringBehaviour, (RexxObject *)object);
  return_value(result);                /* return indicator                  */
}

nativei1 (PCHAR, STRING, REXXOBJECT, object)
/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  PCHAR   result;                      /* returned result                   */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)CurrentActivity->current();
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
  self = (RexxNativeActivation *)CurrentActivity->current();
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
  self = (RexxNativeActivation *)CurrentActivity->current();
                                       /* just forward and return           */
  result = this->isDouble((RexxObject *)object);
  return_value(result);                /* return converted value            */
}

nativei3 (REXXOBJECT, SEND,
     REXXOBJECT,  receiver,            /* receiver of the message           */
     PCHAR,       msgname,             /* the name of the message           */
     REXXOBJECT,  arguments)           /* message arguments                 */
/******************************************************************************/
/* Function:  Issue a full scale send_message from native code                */
/******************************************************************************/
{
  native_entry;                        /* synchronize access                */
  return_oref(send_message((RexxObject *)receiver,(RexxString *)new_cstring(msgname),(RexxArray *)arguments));
}

nativei2 (REXXOBJECT, SUPER,
     PCHAR,       msgname,             /* message name                      */
     REXXOBJECT,  arguments)           /* argument array                    */
/******************************************************************************/
/* Function:  Forward a message to a super class from native code             */
/******************************************************************************/
{
  INT             count;               /* count of arguments                */
  RexxObject     *argarray[10];        /* C array of arguments              */
  RexxArray      *args = (RexxArray *)arguments;
  INT             i;                   /* loop counter                      */
  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)CurrentActivity->current();
  count = args->size();                /* get the argument count            */
  for (i = 1; i <= count; i++)         /* loop through the array            */
                                       /* copying each OREF                 */
    argarray[i-1] = args->get(i);
                                       /* now send the message              */
  return_oref(this->u_receiver->messageSend((RexxString *)new_cstring(msgname), count, argarray, this->u_receiver->superScope(this->method->scope)));
}

nativei2 (REXXOBJECT, SETVAR,
     PCHAR, name,                      /* variable name                     */
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
  self = (RexxNativeActivation *)CurrentActivity->current();
  dictionary = self->methodVariables();/* get the method variables          */
  variableName = new_cstring(name);    /* get a string version of this      */
                                       /* do the assignment                 */
  dictionary->set(variableName, (RexxObject *)value);
  return_oref(OREF_NULL);              /* return nothing                    */
}

nativei2 (REXXOBJECT, SETVAR2,
     PCHAR, name,                      /* variable name                     */
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
  self = (RexxNativeActivation *)CurrentActivity->current();
  RexxActivation *context = (RexxActivation *)this->activity->sender(this);
  variableName = new_cstring(name);    /* get a string version of this      */
                                       /* do the assignment throug retriever*/
  retriever = context->getVariableRetriever(variableName);
  retriever->set(context, (RexxObject *)value);
  return_oref(OREF_NULL);              /* return nothing                    */
}

nativei2 (REXXOBJECT, SETFUNC,
     PCHAR, name,                      /* function name                     */
     REXXOBJECT,  value )              /* method                            */
{
  RexxNativeActivation   * self;       /* current native activation         */

  RexxActivity           * activity;
  RexxActivation         * activation;
  RexxDirectory          * labels;
  RexxString             * methodName;

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */

  self = (RexxNativeActivation *)CurrentActivity->current();
  activity = self->activity;           /* find the current activity         */
                                       /* get the current activation        */
  activation = self->activity->currentAct();

  // get the directory of external functions
  labels = activation->settings.parent_source->routines;

  // if it does not exist, it will be created
  if (!labels) {
    OrefSet(activation->settings.parent_source,
            activation->settings.parent_source->routines,
            new_directory());
    labels = activation->settings.parent_source->routines;
  }

  if (labels) {
    methodName = new_cstring(name);
    // if a method by that name exists, it will be OVERWRITTEN!
    labels->setEntry(methodName, (RexxMethod *) value);
  }

  return_oref(OREF_NULL);              /* return nothing                    */
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

  self = (RexxNativeActivation *)CurrentActivity->current();
  activity = self->activity;           /* find the current activity         */
                                       /* get the current activation        */
  activation = self->activity->currentAct();

  *num = 0;
  if (activation->source->public_routines) {
    funcArray = activation->source->public_routines->makeArray();
    if (funcArray) {
      *num = j = funcArray->numItems();
      *names = (char**) SysAllocateExternalMemory(sizeof(char*)*j);
      for (i=0;i<j;i++) {
        name = ((RexxString*) funcArray->get(i+1));
        (*names)[i] = (char*) SysAllocateExternalMemory(1+sizeof(char)*name->length);
        memcpy((*names)[i], name->stringData, name->length);
        (*names)[i][name->length] = 0; // zero-terminate
      }
    }
  }

  return_oref(OREF_NULL);              /* return nothing                    */
}

nativei1 (REXXOBJECT, GETVAR,
    PCHAR, name )                      /* variable name                     */
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
  self = (RexxNativeActivation *)CurrentActivity->current();
  dictionary = self->methodVariables();/* get the method variables          */
  variableName = new_cstring(name);    /* get a string version of this      */
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
    report_exception(errorcode);       /* no substitution parameters        */
  else                                 /* use the substitution form         */
    report_exceptiond(errorcode, (RexxArray *)value);
}

nativei4 (void, RAISE,
     PCHAR, condition,                 /* name of the condition             */
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
  self = (RexxNativeActivation *)CurrentActivity->current();
  this->result = (RexxObject *)result; /* save the result                   */
                                       /* go raise the condition            */
  CurrentActivity->raiseCondition(new_cstring(condition), OREF_NULL, (RexxString *)description, (RexxObject *)additional, (RexxObject *)result, OREF_NULL);
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
  return_oref((CurrentActivity->raiseCondition((RexxString *)condition, OREF_NULL, (RexxString *)description, (RexxObject *)additional, OREF_NULL, OREF_NULL)) ? TheTrueObject : TheFalseObject);
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
  self = (RexxNativeActivation *)CurrentActivity->current();
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
     else self = (RexxNativeActivation *)CurrentActivity->current();

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
  self = (RexxNativeActivation *)CurrentActivity->current();
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
  self = (RexxNativeActivation *)CurrentActivity->current();
  this->vpavailable = FALSE;           /* dis-allow the calls               */
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
  activation = (RexxActivation *)CurrentActivity->currentAct();
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
  activation = (RexxActivation *)CurrentActivity->currentAct();
  return_oref(activation->popEnvironment());
}

BOOL REXXENTRY REXX_ISDIRECTORY(REXXOBJECT object)
/******************************************************************************/
/* Function:  Validate that an object is a directory                          */
/******************************************************************************/
{
                                       /* do the validation                 */
  return object != NULL && OTYPE(Directory, object);
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
  return ProcessLocalEnv;              /* just return the local environment */
}

REXXOBJECT REXXENTRY REXX_ENVIRONMENT(void)
/******************************************************************************/
/* Function:  Return REXX .environment object to native code                  */
/******************************************************************************/
{
  return TheEnvironment;               /* just return the object            */
}


/* HOL001A begin */
nativei3(ULONG, EXECUTIONINFO,
     PULONG, line,
    PSZ, fname,
    BOOL, next)/* chain of variable request blocks  */
/******************************************************************************/
/* Function:  If variable pool is enabled, return result from SysVariablePool */
/*             method, otherwise return RXSHV_NOAVL.                          */
/******************************************************************************/
{
  ULONG    result;                     /* variable pool result              */
  RexxActivation * self;         /* current native activation         */
  RexxActivationBase * current = NULL;
  RexxActivationBase * newact = NULL;
  RexxString * r;

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = CurrentActivity->currentAct();
                                       /* if access is enabled              */
  result = 1;
  if (next)
  {
        if (!self->next)
     {
         *line = self->code->start->lineNumber;
      }
     else *line = self->next->lineNumber;

      r = self->code->getProgramName();
     strncpy(fname, r->getStringData(), r->getLength());
      fname[r->getLength()] = '\0';
      result = 0;
  }
  else
  {
     r = self->code->getProgramName();
     strncpy(fname, r->getStringData(), r->getLength());
      fname[r->getLength()] = '\0';
      *line = self->getCurrent()->lineNumber;
      result = 0;
  }



  return_value(result);                /* return this                       */
}

nativei7 (ULONG, STEMSORT,
     PCHAR, stemname, INT, order, INT, type, size_t, start, size_t, end,
     size_t, firstcol, size_t, lastcol)
/******************************************************************************/
/* Function:  Perform a sort on stem data.  If everything works correctly,    */
/*             this returns zero, otherwise an appropriate error value.       */
/******************************************************************************/
{
  INT     position;                    /* scan position within compound name */
  INT     length;                      /* length of tail section            */

  RexxNativeActivation * self;         /* current native activation         */

  native_entry;                        /* synchronize access                */
                                       /* pick up current activation        */
  self = (RexxNativeActivation *)CurrentActivity->current();
                                       /* if access is enabled              */
  if (!this->getVpavailable()) {       /* access must be enabled for this to work */
      return FALSE;
  }

  jmp_buf syntaxHandler;               /* syntax return point               */

  if (setjmp(syntaxHandler) != 0) {    /* get a storage error?              */
      return FALSE;                    /* return a failure                  */
  }
  /* get the REXX activation */
  RexxActivation *activation = self->activity->currentAct();

  /* get the stem name as a string */
  RexxString *variable = new_cstring(stemname);
  this->saveObject(variable);
  /* and get a retriever for this variable */
  RexxStemVariable *retriever = (RexxStemVariable *)activation->getVariableRetriever(variable);

  /* this must be a stem variable in order for the sorting to work. */

  if ( (!OTYPE(StemVariable, retriever)) && (!OTYPE(CompoundVariable, retriever)) )
  {
      return FALSE;
  }

//RexxString *tail = (RexxString *) new_cstring(OREF_NULL);
  RexxString *tail = OREF_NULLSTRING ;
  this->saveObject(tail);

  if (OTYPE(CompoundVariable, retriever))
  {
    length = variable->length;      /* get the string length             */
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

//  return_value( retriever->sort(activation, order, type, start, end, firstcol, lastcol));
  return_value( retriever->sort(activation, tail, order, type, start, end, firstcol, lastcol));
}

nativei0 (void, GUARD_ON)
/******************************************************************************/
/* Function:  External interface to implement method guarding                 */
/******************************************************************************/
{
    RexxNativeActivation   * self;       /* current native activation         */

    native_entry;                        /* synchronize access                */
                                         /* pick up current activation        */
    self = (RexxNativeActivation *)CurrentActivity->current();
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
    self = (RexxNativeActivation *)CurrentActivity->current();
    self->guardOff();                    /* turn off the guard                 */
    native_release(OREF_NULL);           /* release the kernel access         */
}

/* HOL001A end */

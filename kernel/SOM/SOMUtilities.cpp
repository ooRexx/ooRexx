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
/* REXX Kernel SOM                                              somutil.c     */
/*                                                                            */
/* SOM Utility Functions                                                      */
/*                                                                            */
/******************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxActivity.hpp"
#include "RexxSOMProxy.hpp"
#ifdef SOM
  #include <somcls.xh>
  #ifdef DSOM
   #include <somd.xh>
  #endif
#include "dlfcls.h"
#include "SOMUtilities.h"

extern RexxObject *ProcessLocalServer;

RexxString *lastMessage = OREF_NULL;
RexxString *currentMessage = OREF_NULL;



void bad_argtype(
  LONG   argument_position,            /* argument position location        */
  PCHAR  argument_name )               /* ascii-z argument name             */
/******************************************************************************/
/* Function:  raise a bad argument error                                      */
/******************************************************************************/
{
   report_exception2(Error_Incorrect_method_argType, new_integer(argument_position), new_cstring(argument_name));
}

void unsupported_type(
  LONG   argument_position,            /* argument position location        */
  PCHAR  argument_name )               /* ascii-z argument name             */
/******************************************************************************/
/* Function:  raise an unsupported type message                               */
/******************************************************************************/
{
  LONG   error;                        /* error to issue                    */

  if (argument_position == -1)         /* return type?                      */
                                       /* use the return error              */
    error = Error_Execution_SOM_unsupportedReturnType;
  else                                 /* actual argument type              */
    error = Error_Execution_SOM_unsupportedType;
                                       /* issue the error                   */
  report_exception2(error, new_cstring(argument_name), new_integer(argument_position));
}

char foreign_arg (TypeCode tc, Environment *pEv)
/******************************************************************************/
/* Arguments:  Type code, environment pointer                                 */
/*                                                                            */
/*  Returned:  's' if somId                                                   */
/******************************************************************************/
{
  any parm;
  int i;
  LONG tempVal;
                                       /* Retrive the type of tk_foreign    */
                                       /*type                               */
   if (TypeCode_param_count(tc,pEv) == 3) {
      parm = TypeCode_parameter(tc,pEv,0);
      if (TypeCode_kind(parm._type,pEv) == tk_string)
                                       /* is it of type somId?              */
         if (strcmp(*(string *)parm._value,"somId") == 0) {
                                       /*  yes indicate somId type to caller*/
            return 's';
         }
   }
                                       /* unsupported type.                 */
                                       /* if we can get a string rep for    */
                                       /*type we get it an display it, other*/
                                       /*use ??                             */
   parm = TypeCode_parameter(tc,pEv,1);
   if (TypeCode_kind(parm._type,pEv) == tk_string) {
     tempVal = TypeCode_kind(parm._type,pEv);
     report_exception2(Error_Execution_SOM_unsupportedType, new_integer(tempVal),
                                                            new_integer(1));
   }
   return '\0';                        /* Return null char                  */
}

RexxObject *getReturnObject(TypeCode tc, void *bp, Environment *evp)
/******************************************************************************/
/* Arguments:  Type code, pointer to buffer pointer, environment pointer      */
/*                                                                            */
/*  Returned:  Output value                                                   */
/******************************************************************************/
{
  TCKind     kind;
  TypeCode   typeCode;
  RexxObject *returnObject;
  long       i;
  long       size;
  long       tempVal;
  ULONG      tempULong;
  char       ULongStr[11];
  long       typeCodeSize;
  DLFStruct *structParm;
  _IDL_SEQUENCE_void *seq;
  any        parm;


  kind = TypeCode_kind(tc,evp);
                                       /* is outtype and any.               */
  if (tk_any == kind) {
                                       /* Yes, then actual info is in       */
                                       /*  the any struct.                  */
                                       /* Get the real kind.                */
    kind = TypeCode_kind(((any *)bp)->_type, evp);
    bp = ((any *)bp)->_value;          /* Get real pointer to data          */
  }


  switch (kind) {
    case tk_void:
      returnObject = OREF_NULL;
      break;
    case tk_short:
      tempVal = *(integer2 *)bp;
      returnObject = new_integer(tempVal);
      break;
    case tk_long:
      tempVal = *(integer4 *)bp;
      returnObject = new_integer(tempVal);
      break;
    case tk_ushort:
      tempVal = *(uinteger2 *)bp;
      returnObject = new_integer(tempVal);
      break;
    case tk_ulong:
                                       /* ULONG's need to be a string       */
      tempULong = *(uinteger4 *)bp;
      sprintf(ULongStr, "%u", tempULong);
      returnObject = new_cstring(ULongStr);
      break;
    case tk_float:
      returnObject = new_numberstring(*(float*)bp);
      break;
    case tk_double:
      returnObject = new_numberstring((double *)bp);
      break;
    case tk_boolean:
      tempVal = *(boolean *)bp;
      returnObject = new_integer(tempVal);
      break;
    case tk_char:
      returnObject = new_string((char *)bp,1);
      break;
    case tk_octet:
      returnObject = new_string((char *)bp,1);
      break;
    case tk_objref:
      returnObject = ProcessLocalServer->sendMessage(OREF_MAKE_PROXY,new_integer(*(long *)bp));
      break;
    case tk_struct:
       structParm = new DLFStruct;     /* Create DLFStruct Object           */
                                       /* Initialize with TypeCode and      */
                                       /*struct ptr                         */
       structParm->initTypeCode(evp, tc, *(void **)bp);
                                       /* ANd create OREXX counterpart.     */
       returnObject = ProcessLocalServer->sendMessage(OREF_MAKE_PROXY,new_integer((long)structParm));
      break;
    case tk_string:
      if (*(char **)bp) {
        returnObject = new_cstring(*(char **)bp);
      } else {
        returnObject = TheNilObject;
      }
      break;
    case tk_array:
                                       /* get typeCode for array            */
      parm = TypeCode_parameter(tc, evp, 0);
                                       /* Retrieve typeCode from Any.       */
      typeCode = *(TypeCode *)parm._value;
                                       /* get any rep of bound              */
      parm = TypeCode_parameter(tc, evp, 1);
      size = *(long *)parm._type;      /* Get upper bound of array          */
                                       /* Get array to hold all possible    */
                                       /*objects                            */
      returnObject = new_array(size);
      save(returnObject);              /* protect this from collection      */
      bp = *(void **)bp;               /* get addressability to array elemen*/
                                       /* Get size of the typeCode.         */
      typeCodeSize = TypeCode_size(typeCode, evp);
                                       /* For each object in array          */
      for (i = 1; i <= size; i++ ) {
                                       /* convert IDL element to OREF and   */
                                       /* place in RexxArray.               */
        ((RexxArray *)returnObject)->put(getReturnObject(typeCode, bp, evp), i);
                                       /* bump to next array element.       */
        bp = (void *)((char *)bp + typeCodeSize);
      }
      discard_hold(returnObject);      /* and release the lock              */
      break;
    case tk_sequence:
                                       /* get typeCode for sequence         */
      parm = TypeCode_parameter(tc, evp, 0);
                                       /* Retrieve typeCode from Any.       */
      typeCode = *(TypeCode *)parm._value;
      seq =  (_IDL_SEQUENCE_void *)bp; /* Get the Sequence                  */
      size =  seq->_length;            /* Get actual number of elements     */
                                       /* Get array to hold all possible    */
                                       /*objects                            */
      returnObject = new_array(size);
      save(returnObject);              /* protect this from collection      */
      bp = seq->_buffer;               /* get address of buffer of elements */
                                       /* Get size of the typeCode.         */
      typeCodeSize = TypeCode_size(typeCode, evp);
                                       /* For each object in array          */
      for (i = 1; i <= size; i++ ) {
                                       /* convert IDL element to OREF and   */
                                       /* place in RexxArray.               */
        ((RexxArray *)returnObject)->put(getReturnObject(typeCode, bp, evp), i);
                                       /* bump to next array element.       */
        bp = (void *)((char *)bp + typeCodeSize);
      }
      SOMFree(seq->_buffer);           /* Free the buffer                   */
      discard_hold(returnObject);      /* and release the lock              */
      break;
    case tk_pointer:
      returnObject = new_pointer(*(void **)bp);
      break;
    case tk_foreign:
      if (foreign_arg(tc,evp) == 's') {
        if (*(void **)bp) {
          returnObject = new_cstring(SOM_StringFromId(*(somId *)bp));
        } else {
          returnObject = TheNilObject;
        }
      }
      break;
    case tk_enum:
                                       /* Get actual Vallue                 */
      tempULong = *(uinteger4 *)bp;
                                       /* See if value is within expected   */
                                       /*  values for this enum.            */
      if (TypeCode_param_count(tc, evp) >= tempULong) {
                                       /* Yup, get parm struct for this     */
                                       /*param                              */
        parm = TypeCode_parameter(tc, evp, tempULong);
                                       /* Convert value to string object    */
        returnObject = new_cstring(*(string *)parm._value);
      }
      else
                                       /* report unknown enum value.        */
        returnObject = new_cstring(CHAR_UNKNOWN);
      break;
    default:
      unsupported_type(-1, "???");
      break;
  } /*switch*/

  return returnObject;
}

void *getReturnArea (TypeCode tc, Environment *evp)
/******************************************************************************/
/* Arguments:  Type code, buffer pointer, environment pointer, arg position   */
/*                                                                            */
/*  Returned:  Output area pointer                                            */
/******************************************************************************/
{
  TCKind kind;
  void *outarea = NULL;

  kind = TypeCode_kind(tc,evp);

  switch (kind) {
    case tk_short:
    case tk_long:
    case tk_ushort:
    case tk_ulong:
    case tk_boolean:
    case tk_char:
    case tk_octet:
    case tk_objref:
    case tk_string:
    case tk_pointer:
    case tk_any:
    case tk_enum:
    case tk_array:
    case tk_void:
      outarea = SOMMalloc(sizeof(void *));
      break;
    case tk_float:
    case tk_double:
      outarea = SOMMalloc(sizeof(double));
      break;
    case tk_struct:
    case tk_sequence:
      outarea = SOMMalloc(TypeCode_size(tc,evp));
      break;
    case tk_foreign:
      if (foreign_arg(tc,evp) == 's')
        outarea = SOMMalloc(sizeof(void *));
      break;
    default:
      unsupported_type(-1, "???");
      break;
  } /*switch*/

  return outarea;
}

         /* ********************************************************* */
         /* ***    C++ Version                                    *** */
         /* ********************************************************* */

extern "C" {
 BOOL  convertInputString(
  RexxObject         *argument,        /* argument to convert               */
  PCHAR              *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  Environment        *ev );            /* pointer to the environment        */

 BOOL  convertInputArray(
  RexxObject         *argument,        /* argument to convert               */
  TypeCode            tc,              /* Array TypeCode                    */
  PVOID              *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  Environment        *ev );            /* pointer to the environment        */

 BOOL  convertInputSequence(
  RexxObject         *argument,        /* argument to convert               */
  TypeCode            tc,              /* Array TypeCode                    */
  _IDL_SEQUENCE_void *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  Environment        *ev );            /* pointer to the environment        */
}

/******************************************************************************/
/* Arguments:  SOM receiver, receiver class, REXX proxy, message name,        */
/*             number of arguments, argument list                             */
/*                                                                            */
/* Function:  We are going to send the message (msgname) the the SOM object   */
/*            somobj.  We are passed the SOM class that somobj is an instance */
/*            of.  We will use this class to do method resolution and         */
/*            obtain the the method descriptor information(signature).  We    */
/*            the iterate through the arguments provided (args) and comvert   */
/*            these OREXX objects into the SOM types/objects required for     */
/*            this method.  This information is maintainedin the SOM          */
/*            Interface Repository (IR)                                       */
/*                                                                            */
/*  Returned:  Result of SOM method                                           */
/******************************************************************************/
OrxSOMMethodInformation *som_resolveMethodInfo(SOMObject *somobj,
                                              SOMClass   *classobj,
                                              RexxObject *oproxy,
                                              RexxObject *msgnameObj,
                                              somId *msgId)
{
  somId desc;                          /* somId for the method Descriptor   */
  string msgstr;                       /* message name as a string          */
  OrxSOMMethodInformation *descobj;    /* the method descriptor object      */
  RexxActivity *myActivity;
  RexxString *msgname;
                                       /*  envronment obj passed            */
  Environment ev;                      /* SOM environment object            */

  SOM_InitEnvironment(&ev);            /* initialize the SOM environment    */

  msgname = REQUEST_STRING(msgnameObj);/* force msgname to a string obj     */
                                       /* Retrieve the actual MSGNAME       */
  msgstr = msgname->stringData;

                                       /* Retrieve activity for this meth   */
                                       /*  curracti not reliable since we   */
                                       /*  go in and out of kernel.  Also   */
                                       /* want to avoid repeated calls to   */
                                       /* activity_find()                   */
  myActivity= CurrentActivity;
                                       /* Release kernel access before      */
                                       /* Since the retrival of this info   */
                                       /* may take awhile                   */
  ReleaseKernelAccess(myActivity);

  *msgId = somIdFromString(msgstr);    /* get a somId for this message name */
                                       /* Get the method Descrip, from IR   */
  desc = classobj->somGetMethodDescriptor(*msgId);

  if (desc == NULL) {                  /* Did we get the descriptor info?   */
                                       /* No, its an error                  */
                                       /* get kernel access agine           */
    RequestKernelAccess(myActivity);
    report_exception2(Error_Execution_SOM_noMethodDescriptor, msgname,
                                        new_cstring(classobj->somGetName()));
  }
                                       /* Create a SOM Descriptor object    */
  descobj = new OrxSOMMethodInformation;
                                       /* Fill in the descriptor with the   */
                                       /* method descriptor info.           */
  if (!descobj->setMethodFromDescriptor(&ev, somStringFromId(desc), msgstr) ){
                                       /* Error settiung method info,       */
                                       /* get kernel access agine           */
    RequestKernelAccess(myActivity);
    report_exception2(Error_Execution_SOM_noMethodDescriptor, msgname,
                                        new_cstring(classobj->somGetName()));
  }

  RequestKernelAccess(myActivity);
  return descobj;                      /* return MethodInformation obj      */

}

/******************************************************************************/
/* Arguments: SOM Environment structure.                                      */
/* Function:  Check for any SOm exception and if one was raised, raise        */
/*      equivalent USER SOM condition.                                        */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
void CheckSOMError(Environment *env)
{
    DLFEnvironment *dlfEnv;            /* SOM Environment object            */
    RexxSOMProxy *envProxy;
    RexxInteger  *somProxy;

    switch (env->_major) {             /* get the major exception code      */
      case NO_EXCEPTION:               /* was an exception raised?          */
        return;                        /* no, just return all done.         */
     default:

      dlfEnv = new DLFEnvironment;     /* Create new object                 */
      dlfEnv->setExceptionFromException(env, env);

                                       /* create new object REXX object */
      envProxy = new RexxSOMProxy;
      somProxy = new_integer((long)dlfEnv);
                                       /* initalize the Proxy portion       */
      envProxy->initProxy(somProxy);

      // Cannot free exception here, since we may use value later, and
      // somExcetionFree will free this value.  Since its now contained in
      // the DLFEnvironment object, it will do the free for us.
                                       /* raise the USER SOM condition.     */
                                       /* with DLFEnvironemnt object as     */
                                       /* the additional object.            */
      CurrentActivity->raiseCondition(OREF_USER_SOM, OREF_NULL, OREF_NULL, envProxy, OREF_NULL, OREF_NULL);
      return ;
    }
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
DLFObject    * getDLFObject(RexxObject *object)
{
  RexxObject *somobj = OREF_NULL;
  DLFObject *dlfObj;

                                       /* if object is primitive SOMProxy   */
  if (OTYPENUM(somproxy, object) || OTYPENUM(somproxy_class, object)) {
                                       /* it will contain SOMOBject         */
      somobj = object->SOMObj();
 }
#if (0)
                                       /* This should no longer be needed.  */
                                       /* Lets wait an see.......  7/12/95  */
                                       /* Can delete if not needed....      */
 else {
                                       /* convert OREXX object to Object    */
                                       /* Ref done by looking up the obj    */
                                       /* in the server and getting the     */
                                       /* SOM Object counterpart            */
      somobj = ProcessLocalServer->sendMessage(OREF_SOMOBJ, object);
  }
#endif
                                       /* able to get the SOMObject?        */
  if (somobj != TheNilObject && somobj != OREF_NULL) {
                                       /* resolve pointer to SOMObject *    */
    dlfObj = (DLFObject *)((RexxInteger *)somobj)->value;
                                       /* is this at least a DLFNumber objec*/
    if (!dlfObj->somIsA(_DLFObject)) {
                                       /* nope, will return NULL            */
      dlfObj = NULL;
    }
  } else                               /* not a somObject so not a DLFNumber*/
    dlfObj = NULL;

  return dlfObj;                       /* return                            */

}


/******************************************************************************/
/*                                                                            */
/******************************************************************************/
DLFNumber    * getDLFNumberObj(RexxObject *object)
{
  DLFNumber *dlfObj;
                                       /* Make sure it at least DLFObject   */
  dlfObj = (DLFNumber *)getDLFObject(object);
                                       /* if a DLFObject and DLfNumber      */
  if (dlfObj && dlfObj->somIsA(_DLFNumber)) {
    return dlfObj;                     /* Return object.                    */
  }
  else {
    return NULL;                       /* otherise return NULL, not object  */
  }

}


/******************************************************************************/
/*                                                                            */
/******************************************************************************/
DLFBasicType * getDLFBasicTypeObj(RexxObject *object)
{
  DLFBasicType *dlfObj;

                                       /* Make sure it at least DLFObject   */
  dlfObj = (DLFBasicType *)getDLFObject(object);
                                       /* if a DLFObject and DLFBasciType   */
  if (dlfObj && dlfObj->somIsA(_DLFBasicType)) {
    return dlfObj;                     /* Return object.                    */
  }
  else {
    return NULL;                       /* otherise return NULL, not object  */
  }

}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
DLFStruct    * getDLFStructObj(RexxObject *object)
{
  DLFStruct *dlfObj;
                                       /* Make sure it at least DLFObject   */
  dlfObj = (DLFStruct *)getDLFObject(object);
                                       /* if a DLFObject and DLFStruct      */
  if (dlfObj && dlfObj->somIsA(_DLFStruct)) {
    return dlfObj;                     /* Return object.                    */
  }
  else {
    return NULL;                       /* otherise return NULL, not object  */
  }
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
DLFEnvironment * getDLFEnvironment(RexxObject *object)
{
  DLFEnvironment *dlfObj;

                                       /* Make sure it at least DLFObject   */
  dlfObj = (DLFEnvironment *)getDLFObject(object);
                                       /* if a DLFObject and DLFEnvironemnt */
  if (dlfObj && dlfObj->somIsA(_DLFEnvironment)) {
    return dlfObj;                     /* Return object.                    */
  }
  else {
    return NULL;                       /* otherise return NULL, not object  */
  }

}

BOOL  pointerCheckDLF(RexxObject *object, TCKind kind, Environment *ev, somToken *outputArgument)
/******************************************************************************/
/***                                                                        ***/
/******************************************************************************/
{
  DLFObject *dlfObject;

                                       /* Go see if its a DLF object        */
  dlfObject = getDLFObject(object);
  if (dlfObject == NULL) {
     return FALSE;
  }
  else {
    switch (kind) {                    /* determine type of SOM arg and     */
                                       /* convert it.                       */
      case tk_boolean:
                                       /* Is object a DLFBoolean            */
        if (dlfObject->somIsA(_DLFBoolean)) {
                                     /* Add reference to boolean output     */
          *outputArgument = ((DLFBoolean *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_char:
                                     /* Is object a DLFChar               */
        if (dlfObject->somIsA(_DLFChar)) {
                                     /* Add reference to boolean output   */
          *outputArgument = ((DLFChar *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_octet:
                                     /* Is object a DLFOctet              */
        if (dlfObject->somIsA(_DLFOctet)) {
                                     /* Add reference to octet   output   */
          *outputArgument = ((DLFOctet *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_short:
                                     /* Is object a DLFShort              */
        if (dlfObject->somIsA(_DLFShort)) {
                                     /* Add reference to short   output   */
          *outputArgument = ((DLFShort *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_ushort:
                                     /* Is object a DLFUShort             */
        if (dlfObject->somIsA(_DLFUShort)) {
                                     /* Add reference to UShort  output   */
          *outputArgument = ((DLFUShort *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_long:
                                     /* Is object a DLFLong               */
        if (dlfObject->somIsA(_DLFLong)) {
                                     /* Add reference to long    output   */
          *outputArgument = ((DLFLong *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_ulong:
                                     /* Is object a DLFULong              */
        if (dlfObject->somIsA(_DLFULong)) {
                                     /* Add reference to ULong   output   */
          *outputArgument = ((DLFULong *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_float:
                                     /* Is object a DLFUFloat             */
        if (dlfObject->somIsA(_DLFFloat)) {
                                     /* Add reference to float   output   */
          *outputArgument = ((DLFFloat *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_double:
                                     /* Is object a DLFDouble             */
        if (dlfObject->somIsA(_DLFDouble)) {
                                     /* Add reference to double  output   */
          *outputArgument = ((DLFDouble *)dlfObject)->_get_address(ev);
          return TRUE;
        }
        break;
      case tk_string:
                                     /* Is object a DLFString             */
        if (dlfObject->somIsA(_DLFString)) {
                                     /* Add reference to string  output   */
          *outputArgument = *(((DLFString *)dlfObject)->_get_address(ev));
          return TRUE;
        }
        break;
      case tk_struct:
                                     /* Is object a DLFStruct             */
        if (dlfObject->somIsA(_DLFStruct)) {
                                     /* Add reference to struct  output   */
          *outputArgument =((DLFStruct *)dlfObject)->_get_value(ev);
          return TRUE;
        }
        break;
      case tk_objref:
      case tk_pointer:
                                       /* Arrays are currently not          */
                                       /*supported.                         */
      case tk_array:
      case tk_any:                     /* The any type.                     */
      case tk_foreign:
      default:
       break;
    }
    return FALSE;
  }
}

BOOL  convertInputBoolean(
  RexxObject         *argument,        /* argument to convert               */
  LONG               *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a boolean input argument type                           */
/******************************************************************************/
{
  LONG          tempLong;              /* temporary converted value         */

                                       /* convert for boolean type.         */
  tempLong = argument->longValueNoNOSTRING(1);
                                       /* did it convert to single digit    */
                                       /*  and its value is 0 or 1.         */
  if (tempLong != NO_LONG && (tempLong == 0 || tempLong == 1)) {
    *outputArgument = tempLong;        /*  yes, return the converted arg    */
    return TRUE;                       /* got a good conversion             */
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputChar(
  RexxObject         *argument,        /* argument to convert               */
  CHAR               *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a character input argument type                         */
/******************************************************************************/
{
  RexxString  * tempString;            /* temporary converted value         */

                                       /* convert OREXX object to char      */
                                       /* use needed_string so we don't     */
                                       /*raise NOSTRING                     */
  tempString = argument->stringValue();
                                       /* if resulting string length is one */
  if (tempString->length  == 1) {
                                       /*  yes, return the converted arg    */
    *outputArgument = (CHAR)*tempString->stringData;
    return TRUE;                       /* got a good conversion             */
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputOctet(
  RexxObject         *argument,        /* argument to convert               */
  octet              *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert an octet input argument type                            */
/******************************************************************************/
{
  RexxString *  tempString;            /* temporary converted value         */

                                       /* convert OREXX object to octet     */
                                       /* use needed_string so we don't     */
                                       /*raise NOSTRING                     */
  tempString = argument->stringValue();
                                       /* if resulting string length is one */
  if (tempString->length == 1) {
                                       /*  yes, return the converted arg    */
    *outputArgument = (octet)*tempString->stringData;
    return TRUE;                       /* got a good conversion             */
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputShort(
  RexxObject         *argument,        /* argument to convert               */
  SHORT              *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a short input argument type                             */
/******************************************************************************/
{
  LONG          tempLong;              /* temporary converted value         */

                                       /* convert OREXX object to short(int)*/
  tempLong = argument->longValueNoNOSTRING(5);

  if (tempLong != NO_LONG) {           /* did it convert to integer         */
                                       /*  and its value in range           */
    if (tempLong >= SHRT_MIN && tempLong <= SHRT_MAX) {
                                       /*  yes, return the converted arg    */
      *outputArgument = (short)tempLong;
      return TRUE;                     /* got a good one                    */
    }
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputUShort(
  RexxObject         *argument,        /* argument to convert               */
  USHORT             *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert an unsigned short input argument type                   */
/******************************************************************************/
{
  LONG          tempLong;              /* temporary converted value         */

                                       /* convert OREXX object to short(int)*/
  tempLong = argument->longValueNoNOSTRING(5);

  if (tempLong != NO_LONG) {           /* did it convert to integer         */
                                       /*  and its value in range           */
    if (tempLong >= 0 && tempLong <= USHRT_MAX) {
                                       /*  yes, return the converted arg    */
      *outputArgument = (unsigned short)tempLong;
      return TRUE;                     /* got a good one                    */
    }
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputLong(
  RexxObject         *argument,        /* argument to convert               */
  LONG               *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a long input argument type                              */
/******************************************************************************/
{
  LONG          tempLong;              /* temporary converted value         */
  LONG          sign;                  /* sign of the number                */
  RexxNumberString *argNumStr;

                                       /* First convert to numberstring     */
  argNumStr = argument->numberString();
                                       /* Did object convert?               */
  if (argNumStr == OREF_NULL)          /* did the object convert?           */
    return FALSE;                      /* can't be a long value             */
                                       /* Get sign of NumberString          */
  sign = argNumStr->sign;

  tempLong = argNumStr->longValueNoNOSTRING(10); /* convert OREXX object to long      */
  if (tempLong != NO_LONG) {           /* did it convert to integer         */
                                       /* is number positive and long value */
                                       /*Negative?                          */
    if (!(sign == 1 && tempLong < 0)) {
      *outputArgument = tempLong;      /* pass back the converted type      */
      return TRUE;                     /* this is a good conversion         */
    }
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputULong(
  RexxObject         *argument,        /* argument to convert               */
  ULONG              *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert an unsigned long input argument type                    */
/******************************************************************************/
{
  ULONG         tempULong;             /* temporary converted value         */
  RexxNumberString *argNumStr;

                                       /* First convert to numberstring     */
  argNumStr = argument->numberString();
                                       /* Did object convert?               */
  if (argNumStr == OREF_NULL)          /* did the object convert?           */
    return FALSE;                      /* can't be a long value             */
                                       /* convert OREXX object to ulong     */
  if (argNumStr->ULong(&tempULong)) {
    *outputArgument = tempULong;       /* pass back the converted type      */
    return TRUE;                       /* good conversion                   */
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputFloat(
  RexxObject         *argument,        /* argument to convert               */
  float              *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a float input argument type                             */
/******************************************************************************/
{
  double        tempDouble;            /* temporary converted value         */

                                       /* convert OREXX object to float     */
  tempDouble = argument->doubleValueNoNOSTRING();
                                       /* did it convert to a double        */
                                       /*  and its within range.            */
                                       /* force double to float and compar  */
  if (tempDouble != NO_DOUBLE) {       /* as double,if equal its OK.        */
                                       /* Can the double be represented     */
                                       /*  as a float?                      */
    if (double2Float(tempDouble, outputArgument))
      return TRUE;                     /* got a good conversion             */
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputDouble(
  RexxObject         *argument,        /* argument to convert               */
  double             *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a double input argument type                            */
/******************************************************************************/
{
                                       /* convert REXX object to double     */
  *outputArgument = argument->doubleValueNoNOSTRING();
                                       /* did it convert to a double        */
                                       /*  and its within range.            */
                                       /* force double to float and compar  */
  if (*outputArgument != NO_DOUBLE)    /* as double,if equal its OK.        */
    return TRUE;                       /* good conversion                   */
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputObjectRef(
  RexxObject         *argument,        /* argument to convert               */
  somToken           *outputArgument ) /* converted argument                */
/******************************************************************************/
/* Function:  Convert a double input argument type                            */
/******************************************************************************/
{
  RexxObject *proxy;                   /* referenced proxy object           */

                                       /* if object is primitive SOMProxy   */
  if (OTYPENUM(somproxy, argument) || OTYPENUM(somproxy_class, argument))
                                       /* Get somObject from arg.           */
      proxy = argument->SOMObj();
  else {
                                       /* convert OREXX object to Object    */
                                       /* Ref done by looking up the obj    */
                                       /* in the server and getting the     */
                                       /* SOM Object counterpart            */
    proxy = ProcessLocalServer->sendMessage(OREF_SOMOBJ, argument);
  }
                                       /* able to get the SOMObject?        */
  if (proxy != TheNilObject && proxy != OREF_NULL) {
                                       /* "unwrap" the proxy                */
    *outputArgument = (somToken)((RexxInteger *)proxy)->value;
    return TRUE;                       /* good conversion                   */
  }
  return FALSE;                        /* bad conversion                    */
}

BOOL  convertInputPointer(
  RexxObject         *argument,        /* argument to convert               */
  somToken           *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  TypeCode            tc,              /* pointer target type               */
  Environment        *ev )             /* pointer to the environment        */
/******************************************************************************/
/* Function:  Convert a pointer input argument type                           */
/******************************************************************************/
{
  TCKind    kind;
  any       parm;
  TypeCode  pointerType;

                                       /* We know tc is a tk_pointer        */
                                       /* has only one parameter, get it.   */
  parm = TypeCode_parameter(tc, ev, 0);
                                       /* Get actual pointer typeCode       */
  pointerType = *(TypeCode *)parm._value;
                                       /* Get TCKind value                  */
  kind = TypeCode_kind(pointerType, ev);
                                       /* looking for char * or UCHAR *     */
  if (tk_char == kind || tk_octet == kind ) {
                                       /* Yes, treat as a string.           */
    return convertInputString(argument, (PCHAR *)outputArgument, saveTable, ev);
  }
                                       /* convert OREXX object to pointer   */
                                       /*  and add it to the SOM ArgLIst    */
  else if (OTYPE(Buffer, argument))    /* if OREXX object is buffer, we get */
                                       /* a pointer to what buffer holds    */
                                       /*  ie. its data,                    */
    *outputArgument = (somToken)((RexxBuffer *)argument)->data;
  else if (OTYPE(Integer, argument))
    *outputArgument = (somToken)((RexxInteger *)argument)->value;
                                       /* See if a DLF object passed and    */
                                       /* it can handle type.               */
  else if (!pointerCheckDLF(argument, kind, ev, outputArgument))
    return FALSE;                      /* bad conversion                    */
  return TRUE;                         /* got a good argument               */
}

BOOL  convertInputStructure(
  RexxObject         *argument,        /* argument to convert               */
  PVOID              *outputArgument,  /* converted argument                */
  TypeCode            tc,              /* structure target type             */
  Environment        *ev )             /* pointer to the environment        */
/******************************************************************************/
/* Function:  Convert a structure input argument type                         */
/******************************************************************************/
{
  PVOID      tempAddress;              /* address to real structure         */
  LONG       structureSize;            /* size of the structure             */
  RexxBuffer *structureBuffer;         /* buffer object for the structure   */

  if (OTYPE(Buffer, argument))         /* in a buffer?                      */
                                       /* pass the buffer address           */
    tempAddress = ((RexxBuffer *)argument)->data;
  else if (OTYPE(Integer, argument))   /* need to "unwrap" an integer value */
    tempAddress = (void *)((RexxInteger *)argument)->value;
  else
    return FALSE;                      /* not a good one                    */
                                       /* get the structure size            */
  structureSize = TypeCode_size(tc, ev);
                                       /* get a buffer for this object      */
  structureBuffer = new_buffer(structureSize);
                                       /* address the structure             */
  *outputArgument = structureBuffer->data;
                                       /* copy the structure over           */
  memcpy(*outputArgument, tempAddress, structureSize);
  return TRUE;                         /* this always succeeds              */
}

BOOL  convertInputString(
  RexxObject         *argument,        /* argument to convert               */
  PCHAR              *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  Environment        *ev )             /* pointer to the environment        */
/******************************************************************************/
/* Function:  Convert a string input argument type                            */
/******************************************************************************/
{
  DLFBasicType *dlfObject;             /* converted DLF object              */

                                       /* convert OREXX object to string    */
                                       /*  and add it to the SOM ArgLIst    */
  if (argument == TheNilObject)        /* is it a NULL string?              */
    *outputArgument = NULL;            /* no argument                       */
                                       /* See if obj is a SOM DLFObject     */
  else if ((dlfObject = getDLFBasicTypeObj(argument)) && dlfObject->somIsA(_DLFString))
                                       /* Yes, get address, for string      */
                                       /*  Allows updating of String directl*/
    *outputArgument = (PCHAR)*(((DLFString *)dlfObject)->_get_address(ev));
  else {                               /* get the argument string value     */
    RexxString *tempString = argument->stringValue();
    saveTable->put(tempString, tempString);
    *outputArgument = tempString->stringData;
  }

  return TRUE;                         /* always converts properly          */
}

BOOL  convertInputAny(
  RexxObject         *argument,        /* argument to convert               */
  any                *outputArgument,  /* converted argument                */
  LONG               *tempLong,        /* temp long location                */
  double             *tempDouble )     /* temporary double location         */
/******************************************************************************/
/* Function:  Convert a a SOM ANY input argument type                         */
/******************************************************************************/
{
  RexxObject *proxy;                   /* referenced proxy object           */

                                       /* Type to coerce our OREXX object   */
                                       /* in the following order.           */
                                       /*  SOMObject                        */
  proxy = ProcessLocalServer->sendMessage(OREF_SOMOBJ, argument);
  if (proxy != TheNilObject) {         /* able to get the SOMObject?        */
                                       /* yup, set up any structure.        */
    outputArgument->_value = (void *)((RexxInteger *)proxy)->value;
    outputArgument->_type = TC_Object; /* this is an object                 */
  }
  else {
                                       /* Now try for a LONG.               */
                                       /* convert OREXX object to long      */
    *tempLong = argument->longValueNoNOSTRING(10);
    if (*tempLong != NO_LONG ){        /* did it convert to integer         */
                                       /* set up the pointer                */
      outputArgument->_value = tempLong;
      outputArgument->_type = TC_long; /* this is a long type               */
    }
    else {
                                       /* Next, try for a double.           */
                                       /* convert OREXX object to double    */
      *tempDouble = argument->doubleValueNoNOSTRING();
      if (*tempDouble != NO_DOUBLE ) { /* did it convert to a double        */
                                       /* set it up accordingly             */
        outputArgument->_value = tempDouble;
        outputArgument->_type = TC_double;
      }
      else {
                                       /* when all else fails, go with a    */
                                       /* STRING.                           */
        outputArgument->_type = TC_string;
                                       /*  and add it to the SOM ArgLIst*/
        if (argument == TheNilObject)  /* is it a NULL string?              */
                                       /* clear it out                      */
          outputArgument->_value = NULL;
        else
          outputArgument->_value =   argument->stringValue()->stringData;
      }                                /* End String coercion.              */
    }                                  /* ENd DOuble attempt.               */
  }                                    /* End Long attempt.                 */
  return TRUE;                         /* always succeeds                   */
}

BOOL  convertInputEnum(
  RexxObject         *argument,        /* argument to convert               */
  TypeCode            tc,              /* Enum Type Code.                   */
  ULONG              *outputArgument,  /* converted argument                */
  Environment        *ev )             /* pointer to the environment        */
/******************************************************************************/
/* Function:  Convert a a SOM ANY input argument type                         */
/******************************************************************************/
{
  long    maxValue;
  long    i;
  RexxString *tempString;
  any     thisValue;

                                       /* Get total number of parameters.   */
                                       /*  so total number of enum values.  */
                                       /*  is count - 1, (ist parmam is name*/
  maxValue = TypeCode_param_count(tc, ev) - 1;
                                       /* 1st see if can convert to ULONG   */
                                       /*  directly.                        */
  if (convertInputULong(argument, outputArgument)) {
    if (*outputArgument <= maxValue) { /* Converted and within range?       */
      return TRUE;                     /* Yup, have out value, return sucess*/
    }
  }
                                       /* Convert Argument to a String      */
  tempString = argument->stringValue();
                                       /* For every string name of ENUM     */
  for (i = 1; i <= maxValue ; i++) {
                                       /* Get the current string name       */
    thisValue = TypeCode_parameter(tc, ev, i);
                                       /* do we have a match here?          */
    if (tempString->strCompare(*(char **)thisValue._value)) {
      *outputArgument = i;             /* Yup, store ULONG value            */
      return TRUE;                     /* indicate sucess.                  */
    }
  }
  return FALSE;                        /* no match on string name either!   */
}

BOOL  convertInputArray(
  RexxObject         *argument,        /* argument to convert               */
  TypeCode            tc,              /* Array TypeCode                    */
  PVOID              *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  Environment        *ev )             /* pointer to the environment        */
/******************************************************************************/
/* Function:  Convert a string input argument type                            */
/******************************************************************************/
{
  TypeCode   embeddedTc;
  long       upperBound;
  long       embeddedTcSize;
  any        tempAny;
  void      *tempAddr;
  char      *bufferPtr;
  long       i;
  RexxArray *argumentArray;

  long     tempLong;
  double   tempDouble;
  RexxObject *thisObject;


                                       /* Get the max upper bound of array  */
                                       /* Elements.                         */
   tempAny = TypeCode_parameter(tc, ev, 1);
   upperBound = *(long *)tempAny._value;
                                       /* Get typeCode array is made of.    */
   tempAny = TypeCode_parameter(tc, ev, 0);
   embeddedTc = *(TypeCode *)tempAny._value;
                                       /* Get the Size of Array Elements.   */
   embeddedTcSize = TypeCode_size(embeddedTc, ev);
                                       /* try and convert to array object   */
   argumentArray = (RexxArray *)REQUEST_ARRAY(argument);
   if (argument == TheNilObject)       /* Did it convert?                   */
     return FALSE;                     /* Nope, return to report error      */
                                       /* Is provided array too big?        */
   if (argumentArray->size() > upperBound)
                                       /* yes, report error                 */
     //report_exception(Array_too_big)

                                       /* Get buffer/Array for elements.    */
   *outputArgument = SOMMalloc(upperBound * embeddedTcSize);
                                       /* for each element, bump to next    */
                                       /*  Element position and fill in arry*/
   for (i = 1 , bufferPtr = (char *)tempAddr; i <= upperBound ; i++, bufferPtr += embeddedTcSize ) {
                                       /* Retrieve Ith element from array   */
      thisObject = argumentArray->get(i);
                                       /* determine type of SOM arg and     */
      switch (TypeCode_kind(embeddedTc, ev)) {
                                       /* convert it.                       */
        case tk_boolean:
                                       /* do the boolean conversion         */
          if (!convertInputBoolean(thisObject, (long *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_BOOLEAN);
          break;

        case tk_char:
                                       /* do the character conversion       */
          if (!convertInputChar(thisObject, (char *)bufferPtr))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
          break;

        case tk_octet:
                                       /* do the octet conversion           */
          if (!convertInputOctet(thisObject, (octet *)bufferPtr))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
          break;

        case tk_short:
                                       /* do the short conversion           */
          if (!convertInputShort(thisObject, (short *)bufferPtr))
            bad_argtype(i, CHAR_SHORT);/* have a bad argument               */
          break;

        case tk_ushort:
                                       /* do the short conversion           */
          if (!convertInputUShort(thisObject, (USHORT *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_USHORT);
          break;

       case tk_long:
                                       /* do the long conversion            */
          if (!convertInputLong(thisObject, (long *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_LONG);
          break;

        case tk_ulong:
                                       /* do the long conversion            */
          if (!convertInputULong(thisObject, (ULONG *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ULONG);
          break;

        case tk_float:
                                       /* do the long conversion            */
          if (!convertInputFloat(thisObject, (float *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_FLOAT);
          break;

        case tk_double:
                                       /* do the long conversion            */
          if (!convertInputDouble(thisObject, (double *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_DOUBLE);
          break;

       case tk_objref:
                                       /* do the long conversion            */
          if (!convertInputObjectRef(thisObject, (somToken *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SOMObject);
          break;

        case tk_pointer:
                                       /* do the long conversion            */
          if (!convertInputPointer(thisObject, (somToken *)bufferPtr, saveTable, embeddedTc, ev))
                                       /* have a bad argument               */
            bad_argtype(i, "pointer");
          break;

        case tk_struct:
                                       /* do the long conversion            */
          convertInputStructure(thisObject, (PVOID *)bufferPtr, embeddedTc, ev);
          break;

        case tk_string:
                                       /* do the conversion                 */
          convertInputString(thisObject, (PCHAR *)bufferPtr, saveTable, ev);
          break;

                                       /* Arrays are currently not          */
                                       /*supported.                         */
        case tk_array:
          if (!convertInputArray(thisObject, embeddedTc, (PVOID *)bufferPtr, saveTable, ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ARRAY);
          break;

        case tk_sequence:
          if (!convertInputSequence(thisObject, embeddedTc, (_IDL_SEQUENCE_void *)bufferPtr, saveTable, ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SEQUENCE);
          break;

        case tk_any:                   /* The any type.                     */
                                       /* do the conversion                 */
          convertInputAny(thisObject, (any *)bufferPtr, &tempLong, &tempDouble);
          break;

        case tk_foreign:
          {
                                       /* Is it a somID?                    */
            if (foreign_arg(tc, ev) == 's')
                                       /*  yes add it to the SOM ArgLIst    */
                                       /* NOTE: at some point should        */
                                       /* actually remember and SOMFree     */
                                       /* these ids.                        */
              *(somToken *)bufferPtr = (somToken)somIdFromString(thisObject->stringValue()->stringData);
            else
              unsupported_type(i, "???");
            break;
          }
        default:
          unsupported_type(i, "???");
          break;
      } /*switch*/
   }

   return TRUE;
}

BOOL  convertInputSequence(
  RexxObject         *argument,        /* argument to convert               */
  TypeCode            tc,              /* Array TypeCode                    */
  _IDL_SEQUENCE_void *outputArgument,  /* converted argument                */
  RexxTable          *saveTable,
  Environment        *ev )             /* pointer to the environment        */
/******************************************************************************/
/* Function:  Convert a string input argument type                            */
/******************************************************************************/
{
  TypeCode   embeddedTc;
  long       upperBound;
  long       embeddedTcSize;
  any        tempAny;
  void      *tempAddr;
  char      *bufferPtr;
  long       i;
  RexxArray *argumentArray;

  long     tempLong;
  double   tempDouble;
  RexxObject* thisObject;


                                       /* Get the max upper bound of array  */
                                       /* Elements.                         */
   tempAny = TypeCode_parameter(tc, ev, 1);
   upperBound = *(long *)tempAny._value;
                                       /* Get typeCode array is made of.    */
   tempAny = TypeCode_parameter(tc, ev, 0);
   embeddedTc = *(TypeCode *)tempAny._value;
                                       /* Get the Size of Array Elements.   */
   embeddedTcSize = TypeCode_size(embeddedTc, ev);
                                       /* try and convert to array object   */
   argumentArray = (RexxArray *)REQUEST_ARRAY(argument);
   if (argument == TheNilObject)       /* Did it convert?                   */
     return FALSE;                     /* Nope, return to report error      */
                                       /* Is provided array too big?        */
   if (upperBound && argumentArray->size() > upperBound)
                                       /* yes, report error                 */
     //report_exception(Array_too_big)

   if (!upperBound) {                  /* Was an upperBound specified?      */
                                       /* no, so we use actual arrray size. */
     upperBound = argumentArray->size();
   }
                                       /* Fill in Sequence Static Data.     */
   outputArgument->_maximum = argumentArray->size();
   outputArgument->_length  = argumentArray->size();
                                       /* Get buffer       for elements.    */
   outputArgument->_buffer = SOMMalloc(upperBound * embeddedTcSize);
                                       /* for each element, bump to next    */
                                       /*  Element position and fill in arry*/
   for (i = 1 , bufferPtr = (char *)outputArgument->_buffer; i <= upperBound ; i++, bufferPtr += embeddedTcSize ) {
                                       /* Retrieve Ith element from array   */
      thisObject = argumentArray->get(i);
                                       /* determine type of SOM arg and     */
      switch (TypeCode_kind(embeddedTc, ev)) {
                                       /* convert it.                       */
        case tk_boolean:
                                       /* do the boolean conversion         */
          if (!convertInputBoolean(thisObject, (long *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_BOOLEAN);
          break;

        case tk_char:
                                       /* do the character conversion       */
          if (!convertInputChar(thisObject, (char *)bufferPtr))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
          break;

        case tk_octet:
                                       /* do the octet conversion           */
          if (!convertInputOctet(thisObject, (octet *)bufferPtr))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
          break;

        case tk_short:
                                       /* do the short conversion           */
          if (!convertInputShort(thisObject, (short *)bufferPtr))
            bad_argtype(i, CHAR_SHORT);/* have a bad argument               */
          break;

        case tk_ushort:
                                       /* do the short conversion           */
          if (!convertInputUShort(thisObject, (USHORT *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_USHORT);
          break;

       case tk_long:
                                       /* do the long conversion            */
          if (!convertInputLong(thisObject, (long *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_LONG);
          break;

        case tk_ulong:
                                       /* do the long conversion            */
          if (!convertInputULong(thisObject, (ULONG *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ULONG);
          break;

        case tk_float:
                                       /* do the long conversion            */
          if (!convertInputFloat(thisObject, (float *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_FLOAT);
          break;

        case tk_double:
                                       /* do the long conversion            */
          if (!convertInputDouble(thisObject, (double *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_DOUBLE);
          break;

       case tk_objref:
                                       /* do the long conversion            */
          if (!convertInputObjectRef(thisObject, (somToken *)bufferPtr))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SOMObject);
          break;

        case tk_pointer:
                                       /* do the long conversion            */
          if (!convertInputPointer(thisObject, (somToken *)bufferPtr, saveTable, embeddedTc, ev))
                                       /* have a bad argument               */
            bad_argtype(i, "pointer");
          break;

        case tk_struct:
                                       /* do the long conversion            */
          convertInputStructure(thisObject, (PVOID *)bufferPtr, embeddedTc, ev);
          break;

        case tk_string:
                                       /* do the conversion                 */
          convertInputString(thisObject, (PCHAR *)bufferPtr, saveTable, ev);
          break;

                                       /* Arrays are currently not          */
                                       /*supported.                         */
        case tk_array:
          if (!convertInputArray(thisObject, embeddedTc, (PVOID *)bufferPtr, saveTable, ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ARRAY);
          break;

        case tk_sequence:
          if (!convertInputSequence(thisObject, embeddedTc, (_IDL_SEQUENCE_void *)bufferPtr, saveTable, ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SEQUENCE);
          break;

        case tk_any:                   /* The any type.                     */
                                       /* do the conversion                 */
          convertInputAny(thisObject, (any *)bufferPtr, &tempLong, &tempDouble);
          break;

        case tk_foreign:
          {
                                       /* Is it a somID?                    */
            if (foreign_arg(tc,ev) == 's')
                                       /*  yes add it to the SOM ArgLIst    */
                                       /* NOTE: at some point should        */
                                       /* actually remember and SOMFree     */
                                       /* these ids.                        */
              *(somToken *)bufferPtr = (somToken)somIdFromString(thisObject->stringValue()->stringData);
            else
              unsupported_type(i, "???");
            break;
          }
        default:
          unsupported_type(i, "???");
          break;
      } /*switch*/
   }

   return TRUE;
}


/******************************************************************************/
/* Arguments:  SOM receiver, receiver class, REXX proxy, message name,        */
/*             number of arguments, argument list                             */
/*                                                                            */
/* Function:  We are going to send the message (msgname) the the SOM object   */
/*            somobj.  We are passed the SOM class that somobj is an instance */
/*            of.  We will use this class to do method resolution and         */
/*            obtain the the method descriptor information(signature).  We    */
/*            the iterate through the arguments provided (args) and comvert   */
/*            these OREXX objects into the SOM types/objects required for     */
/*            this method.  This information is maintainedin the SOM          */
/*            Interface Repository (IR)                                       */
/*                                                                            */
/*  Returned:  Result of SOM method                                           */
/******************************************************************************/
RexxObject *som_send (SOMObject *somobj,
               SOMClass  *classobj,
               RexxObject *oproxy,
               RexxObject *msgname,
               long numPassedArgs,
               RexxObject **args,
               OrxSOMMethodInformation *descobj,
               somId msgid,
               OrxSOMArgumentList *somargs)
{
  BOOL  descobjProvided = TRUE;        /* Assume method descriptor provided */
  BOOL  argListProvided = TRUE;        /* Assume argumentList      provided */
                                       /* number of arguments this method   */
                                       /*expec                              */
  uinteger4 numargs;
                                       /* Assume arguments start at 1st     */
                                       /*positio                            */
  long  startArg = 1;
  RexxActivity *myActivity;
                                       /*  an exception                     */
  RexxObject *thisObject;
  RexxObject *returnObj;
  void *returnBuff;
  void *addr;
  long  tsize;
  long  maxSize;
                                       /* Following are used as temp        */
                                       /* values from object conversions    */
  char     tempChar;
  octet    tempOctet;
  short    tempShort;
  USHORT   tempUShort;
  long     tempLong;
  ULONG    tempULong;
  double   tempDouble;
  float    tempFloat;
  RexxString *tempString;
  any      tempAny;
  somToken tempToken;
  PVOID    tempAddr;
  _IDL_SEQUENCE_void tempSeq;

  int rc, i;
  TCKind kind;
  TypeCode tc;
  TypeCode embeddedTc;
  ParameterMode mode;
  string cstyle;                       /* callstyle of method (oidl - no  */
                                       /*  envronment obj passed          */
  SOMObject    *somObj;
  DLFNumber    *dlfNum;
  DLFBasicType *dlfObject;
  DLFStruct    *dlfStruct;
  DLFEnvironment *dlfEnv;
  va_list      *argumentList;
  RexxTable    *saveTable;             /* Table to hold temp objects        */

  Environment ev;                      /* SOM environment object          */
  Environment callEnv;                 /* SOM environment object          */

  saveTable = new_table();
  save(saveTable);

  SOM_InitEnvironment(&ev);            /* initialize the SOM environment  */

  if (!descobj) {                      /* were we provided a Method Info  */
    descobjProvided = FALSE;           /* nope, make sure we flag to free */
                                       /* now resolve the methodInfo      */
    descobj = som_resolveMethodInfo(somobj, classobj, oproxy, msgname, &msgid);
  }
  if (!somargs) {                      /* were we provided an arg List    */
    argListProvided = FALSE;           /* nope, make sure we flag to free */
                                       /* now create the argument list    */
    somargs = new OrxSOMArgumentList;
  }
  else
    somargs->reInit(&ev);              /* be sure to reinitialize arglist   */
  somargs->addObjRef(&ev, somobj);     /* Add receiver as 1st thing to argl */

                                       /* Get the call style for this meth*/
  cstyle = descobj->_get_callstyle(&ev);
                                       /* get the number of arguments this*/
                                       /* method expects.                 */
  numargs = descobj->getNumArguments(&ev);

                                       /* is this an IDL style call?      */
  if (!strcmp(CALLSTYLE_IDL,cstyle)) {
                                       /* Env parm may be passed in.      */
    if (numargs + 1 == numPassedArgs) {
                                       /* Yes, then see if arguement is     */
                                       /*   a DLFEnvironment object         */
      dlfEnv = getDLFEnvironment(args[0]);
      if (dlfEnv != NULL) {            /* is it a DLFEnvironment            */
        somargs->addSOMEnv(&ev, dlfEnv->getEnvironment(&ev));
        startArg = 2;                  /* remaining arg start with 2nd arg  */
      }
      else {
        bad_argtype(1, CHAR_DLFEnvironment);
      }
    }
    else {
      SOM_InitEnvironment(&callEnv);   /* initialize the SOM environment    */
                                       /* Yes, then add the environment   */
                                       /* object to the arg list.         */
      somargs->addSOMEnv(&ev, &callEnv);
    }
  }
                                       /* Is it the number SOM is expectin*/
  if (numargs != (numPassedArgs - startArg + 1)) {
   if (!descobjProvided) {
    descobj->somFree();                /* No, free up an SOM objects we   */
    SOMFree(msgid);
   }
   if (!argListProvided)
     somargs->somFree();               /* won't need anymore                */

   if (numPassedArgs > numargs)        /* were we passed too many args?   */
     report_exception1(Error_Incorrect_method_maxarg, new_integer(numargs));
   else                                /* nope not enough.                */
     report_exception1(Error_Incorrect_method_minarg, new_integer(numargs));
  }

                                       /* for all argument to method.     */
  for (i = startArg; i <= numargs; i++) {
                                       /* Determine the type of argument  */
    descobj->getNthArgInfo(&ev, i - startArg + 1, &tc, &mode);
    kind = TypeCode_kind(tc, &ev);

                                       /* retrive the matching OREXX obj  */
                                       /* Was this argument omitted?      */
    if ((thisObject = args[i -1]) == OREF_NULL) {
                                       /* Yes, not allowed to omit args   */
     if (!descobjProvided) {
      descobj->somFree();              /* free up any SOM objects we won't*/
     }
     if (!argListProvided) {
       somargs->somFree();
     }
     report_exception1(Error_Incorrect_method_noarg, new_integer(i));
    }
    if (ParameterDef_IN == mode) {     /* is it an in type parameter?     */
      switch (kind) {                  /* determine type of SOM arg and   */
                                       /* convert it.                     */
        case tk_boolean:
                                       /* do the boolean conversion         */
          if (!convertInputBoolean(thisObject, &tempLong))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_BOOLEAN);
                                       /* add to the argument list          */
          somargs->addBoolean(&ev, (boolean)tempLong);
          break;

        case tk_char:
                                       /* do the character conversion       */
          if (!convertInputChar(thisObject, &tempChar))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
                                       /* add to the argument list          */
          somargs->addChar(&ev, tempChar);
          break;

        case tk_octet:
                                       /* do the octet conversion           */
          if (!convertInputOctet(thisObject, &tempOctet))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
                                       /* add to the argument list          */
          somargs->addOctet(&ev, tempOctet);
          break;

        case tk_short:
                                       /* do the short conversion           */
          if (!convertInputShort(thisObject, &tempShort))
            bad_argtype(i, CHAR_SHORT);/* have a bad argument               */
                                       /* add to the argument list          */
          somargs->addShort(&ev, tempShort);
          break;

        case tk_ushort:
                                       /* do the short conversion           */
          if (!convertInputUShort(thisObject, &tempUShort))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_USHORT);
                                       /* add to the argument list          */
          somargs->addUShort(&ev, tempUShort);
          break;

       case tk_long:
                                       /* do the long conversion            */
          if (!convertInputLong(thisObject, &tempLong))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_LONG);
                                       /* add to the argument list          */
          somargs->addLong(&ev, tempLong);
          break;

        case tk_ulong:
                                       /* do the long conversion            */
          if (!convertInputULong(thisObject, &tempULong))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ULONG);
                                       /* add to the argument list          */
          somargs->addULong(&ev, tempULong);
          break;

        case tk_float:
                                       /* do the long conversion            */
          if (!convertInputFloat(thisObject, &tempFloat))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_FLOAT);
                                       /* add to the argument list          */
          somargs->addFloat(&ev, tempFloat);
          break;

        case tk_double:
                                       /* do the long conversion            */
          if (!convertInputDouble(thisObject, &tempDouble))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_DOUBLE);
                                       /* add to the argument list          */
          somargs->addDouble(&ev, tempDouble);
          break;

       case tk_objref:
                                       /* do the long conversion            */
          if (!convertInputObjectRef(thisObject, &tempToken))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SOMObject);
                                       /* add to the argument list          */
          somargs->addObjRef(&ev, tempToken);
          break;

        case tk_pointer:
                                       /* do the long conversion            */
          if (!convertInputPointer(thisObject, &tempToken, saveTable, tc, &ev))
                                       /* have a bad argument               */
            bad_argtype(i, "pointer");
                                       /* add to the argument list          */
          somargs->addPointer(&ev, (somToken)tempToken);
          break;

        case tk_struct:
                                       /* do the long conversion            */
          convertInputStructure(thisObject, &tempAddr, tc, &ev);
                                       /* add to the argument list          */
          somargs->addStruct(&ev, tempAddr);
          break;

        case tk_string:
                                       /* do the conversion                 */
          convertInputString(thisObject, (PCHAR *)&tempAddr, saveTable, &ev);
                                       /* add to the argument list          */
          somargs->addString(&ev, (PCHAR)tempAddr);
          break;

        case tk_array:
          if (!convertInputArray(thisObject, tc, &tempAddr, saveTable, &ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ARRAY);
          somargs->addPointer(&ev, (somToken)tempAddr);
          break;

        case tk_sequence:
          if (!convertInputSequence(thisObject, tc, &tempSeq, saveTable, &ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SEQUENCE);
          somargs->addPointer(&ev, (somToken)&tempSeq);
          break;

        case tk_any:                   /* The any type.                     */
                                       /* do the conversion                 */
          convertInputAny(thisObject, &tempAny, &tempLong, &tempDouble);
                                       /* add to the argument list          */
          somargs->addAny(&ev, &tempAny);
          break;

        case tk_enum:                  /* The enum type.                    */
                                       /* do the conversion                 */
          if (!convertInputEnum(thisObject, tc, &tempULong, &ev))
            bad_argtype(i, "ENUM");
                                       /* add to the argument list          */
          somargs->addULong(&ev, tempULong);
          break;

        case tk_foreign:
          {
                                       /* Is it a somID?                    */
            if (foreign_arg(tc,&ev) == 's')
                                       /*  yes add it to the SOM ArgLIst    */
                                       /* NOTE: at some point should        */
                                       /* actually remember and SOMFree     */
                                       /* these ids.                        */
              somargs->addForeign(&ev, (somToken)somIdFromString(thisObject->stringValue()->stringData));
            else
              unsupported_type(i, "???");
            break;
          }
        default:
          unsupported_type(i, "???");
          break;
      } /*switch*/
    }
    else {                             /* Nope it must be INOUT or OUT      */
                                       /* INOUT/OUT parameters must be      */
                                       /*  at a DLFObject/DLFNumber         */


                                       /* convert OREXX object to Object  */
                                       /* Ref done by looking up the obj  */
                                       /* in the server and getting the   */
                                       /* SOM Object counterpart          */
      thisObject = ProcessLocalServer->sendMessage(OREF_SOMOBJ,thisObject);
                                       /* able to get the SOMObject?      */
      if (thisObject != TheNilObject) {
                                       /* resolve pointer to SOMObject *    */
        somObj = (SOMObject *)((RexxInteger *)thisObject)->value;
                                       /* is this at least a DLFNumber objec*/
        if (!somObj->somIsA(_DLFObject)) {
          bad_argtype(i, CHAR_DLFObject);

        }
      } else {
                                       /* nope report Exception.            */
        bad_argtype(i, CHAR_DLFObject);
      }


      switch (kind) {                  /* determine type of SOM arg and     */
                                       /* convert it.                       */
        case tk_boolean:
                                       /* Is object a DLFBoolean            */
          if (somObj->somIsA(_DLFBoolean)) {
                                       /* Add reference to boolean output   */
            somargs->addPointer(&ev, ((DLFBoolean *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFBoolean);
          }
          break;
        case tk_char:
                                       /* Is object a DLFChar               */
          if (somObj->somIsA(_DLFChar)) {
                                       /* Add reference to boolean output   */
            somargs->addPointer(&ev, ((DLFChar *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFChar);
          }
          break;
        case tk_octet:
                                       /* Is object a DLFOctet              */
          if (somObj->somIsA(_DLFOctet)) {
                                       /* Add reference to octet   output   */
            somargs->addPointer(&ev, ((DLFOctet *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFOctet);
          }
          break;
        case tk_short:
                                       /* Is object a DLFShort              */
          if (somObj->somIsA(_DLFShort)) {
                                       /* Add reference to short   output   */
            somargs->addPointer(&ev, ((DLFShort *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFShort);
          }
          break;
        case tk_ushort:
                                       /* Is object a DLFUShort             */
          if (somObj->somIsA(_DLFUShort)) {
                                       /* Add reference to UShort  output   */
            somargs->addPointer(&ev, ((DLFUShort *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFUShort);
          }
          break;
        case tk_long:
                                       /* Is object a DLFLong               */
          if (somObj->somIsA(_DLFLong)) {
                                       /* Add reference to long    output   */
            somargs->addPointer(&ev, ((DLFLong *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFLong);
          }
          break;
        case tk_ulong:
                                       /* Is object a DLFULong              */
          if (somObj->somIsA(_DLFULong)) {
                                       /* Add reference to ULong   output   */
            somargs->addPointer(&ev, ((DLFULong *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFULong);
          }
          break;
        case tk_float:
                                       /* Is object a DLFUFloat             */
          if (somObj->somIsA(_DLFFloat)) {
                                       /* Add reference to float   output   */
            somargs->addPointer(&ev, ((DLFFloat *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFFloat);
          }
          break;
        case tk_double:
                                       /* Is object a DLFDouble             */
          if (somObj->somIsA(_DLFDouble)) {
                                       /* Add reference to double  output   */
            somargs->addPointer(&ev, ((DLFDouble *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFDouble);
          }
          break;
        case tk_string:
                                       /* Is object a DLFString             */
          if (somObj->somIsA(_DLFString)) {
                                       /* Add reference to string  output   */
            somargs->addPointer(&ev, ((DLFString *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFString);
          }
          break;
        case tk_struct:
                                       /* Is object a DLFStruct             */
          if (somObj->somIsA(_DLFStruct)) {
                                       /* Add reference to string  output   */
            somargs->addPointer(&ev, ((DLFStruct *)somObj)->_get_address(&ev));
          }else {
            bad_argtype(i, CHAR_DLFStruct);
          }
          break;
        case tk_objref:
        case tk_pointer:
        case tk_array:                        /* Arrays are currently not supported.  */
        case tk_any:                          /* The any type.                        */
        case tk_foreign:
        default:
          unsupported_type(i, "OUT or INOUT");
         break;
      }
    }
  } /*for*/
                                       /* GEt return Information            */
  descobj->getReturnInfo(&ev, &tc, &mode);
  returnBuff = getReturnArea(tc, &ev); /* Allocate Buffer for return        */
                                       /* Retrieve activity for this meth   */
                                       /*  curracti not reliable since we   */
                                       /*  go in and out of kernel.  Also   */
                                       /* want to avoid repeated calls to   */
                                       /* activity_find()                   */
  myActivity= CurrentActivity;
  ReleaseKernelAccess(myActivity);     /* Release kernel access before      */
                                       /* Get the va_list or arguemnts      */
  argumentList = somargs->getArgumentList(&ev);
                                       /* Go call SOMApply                  */
  oryxSomClassDispatchX(somobj, classobj, (void **)returnBuff, msgid, argumentList);
  SOMFree(argumentList);               /* Free the va_list block            */
                                       /* Re-aquire kernel access before    */
  RequestKernelAccess(myActivity);     /*  continuing.                      */
                                       /* callling SomDispatch              */
  returnObj = getReturnObject(tc, returnBuff, &ev);
  SOMFree(returnBuff);
                                            /* free any object/ids usedin routine   */
  if (!descobjProvided) {
    SOMFree(msgid);
    descobj->somFree();
  }
  if (!argListProvided) {
    somargs->somFree();
  }
                                       /* is this an IDL style call?      */
  if (!strcmp(CALLSTYLE_IDL,cstyle)) {
                                       /* Yes, check for error conditions */
                                       /* check   for SOM exception and   */
                                       /*   raise USER exception, may or  */
                                       /*   may not return.               */
    CheckSOMError(&callEnv);
  }
  discard(saveTable);

  return returnObj;                    /* Return the result object.         */
}

#ifdef DSOM
/******************************************************************************/
/* Arguments:  DSOM receiver, receiver class, REXX proxy, message name,       */
/*             number of arguments, argument list                             */
/*                                                                            */
/* Function:  We are going to send the message (msgname) the the SOM object   */
/*            somobj.  We are passed the SOM class that somobj is an instance */
/*            of.  We will use this class to do method resolution and         */
/*            obtain the the method descriptor information(signature).  We    */
/*            the iterate through the arguments provided (args) and comvert   */
/*            these OREXX objects into the SOM types/objects required for     */
/*            this method.  This information is maintainedin the SOM          */
/*            Interface Repository (IR)                                       */
/*                                                                            */
/*  NOTE: Still need to make sure we free anything from the ORB that may be   */
/*   left lying around.                                                       */
/*                                                                            */
/*                                                                            */
/*  Returned:  Result of SOM method                                           */
/******************************************************************************/
RexxObject *dsom_send (SOMDObject *somobj,
               SOMClass  *classobj,
               RexxObject *oproxy,
               RexxObject *msgnameObj,
               long numPassedArgs,
               RexxObject **args,
               OrxSOMMethodInformation *descobj,
               somId msgid)
{
  BOOL descobjProvided = TRUE;         /* Assume method descriptor provided */
  BOOL argListProvided = TRUE;         /* Assume argumentList      provided */
                                       /* number of arguments this method   */
                                       /*expec                              */
  long numargs;
                                       /* Assume arguments start at 1st     */
                                       /*positio                            */
  long startArg = 1;
  RexxActivity *myActivity;
  RexxObject *thisObject;
  RexxObject *returnObj;
  RexxString *msgname;
  void *returnBuff;
  void *addr;
  long tsize;
                                       /* Following are used as temp        */
                                       /* values from object conversions    */
  char     tempChar;
  octet    tempOctet;
  short    tempShort;
  USHORT   tempUShort;
  long     tempLong;
  ULONG    tempULong;
  double   tempDouble;
  float    tempFloat;
  RexxString *tempString;
  any      tempAny;
  somToken tempToken;
  PVOID    tempAddr;
  somId    tempId;
  _IDL_SEQUENCE_void tempSeq;
  char    *msgstr;

  int rc,       i;
  int           argNum;
  TCKind        kind;
  TypeCode      tc;
  ParameterMode mode;
  string        cstyle;                /* callstyle of method (oidl - no    */
                                       /*  envronment obj passed            */
  SOMObject    *somObj;
  DLFNumber    *dlfNum;
  DLFBasicType *dlfObject;
  DLFStruct    *dlfStruct;
  DLFEnvironment *dlfEnv;

  NVList       *argList;
  NamedValue    NVResult;
  Request      *requestObj;
  Identifier    argName;
  Flags         flags;
  void         *dummy;
  long          argLength;
  RexxTable    *saveTable;

  Environment ev;                      /* SOM environment object            */
  Environment *callEnv;                /* SOM environment object            */

  SOM_InitEnvironment(&ev);            /* initialize the SOM environment    */
  saveTable = new_table();
  save(saveTable);

  if (!descobj) {                      /* were we provided a Method Info    */
    descobjProvided = FALSE;           /* nope, make sure we flag to free   */
                                       /* now resolve the methodInfo        */
    descobj = som_resolveMethodInfo(somobj, classobj, oproxy, msgnameObj, &msgid);
  }

  msgname = REQUEST_STRING(msgnameObj);/* force msgname to a string obj     */
                                       /* Retrieve the actual MSGNAME       */
  msgstr = msgname->stringData;
                                       /* Create NVlist and result NV       */
  somobj->create_request_args(&ev, msgstr, &argList, &NVResult);


                                       /* Get the call style for this meth*/
  cstyle = descobj->_get_callstyle(&ev);
                                       /* get the number of arguments this*/
                                       /* method expects.                 */
//argList->get_count(&ev, &numargs);
  numargs = descobj->getNumArguments(&ev);

                                       /* is this an IDL style call?        */
  if (!strcmp(CALLSTYLE_IDL,cstyle)) {
                                       /* Env parm may be passed in.        */
    if (numargs + 1 == numPassedArgs) {
                                       /* Yes, then see if arguement is     */
                                       /*   a DLFEnvironment object         */
      dlfEnv = getDLFEnvironment(args[0]);
      if (dlfEnv != NULL) {            /* is it a DLFEnvironment            */
        callEnv = SOM_CreateLocalEnvironment();
        startArg = 2;                  /* remaining arg start with 2nd arg  */
      }
      else {
        bad_argtype(1, CHAR_DLFEnvironment);
      }
    }
    else {
      callEnv = &ev;                   /* initialize the SOM environment    */
    }
  }
  else
    callEnv = &ev;                     /* initialize the SOM environment    */
                                       /* Is it the number SOM is expectin*/
  if (numargs != (numPassedArgs - startArg + 1)) {
   if (!descobjProvided) {
    descobj->somFree();                /* No, free up an SOM objects we   */
    SOMFree(msgid);
   }

   if (numPassedArgs > numargs)        /* were we passed too many args?   */
     report_exception1(Error_Incorrect_method_maxarg, new_integer(numargs));
   else                                /* nope not enough.                */
     report_exception1(Error_Incorrect_method_minarg, new_integer(numargs));
  }

                                       /* for all argument to method.     */
  for (i = startArg; i <= numargs; i++) {
    argNum = i - startArg;             /* Compute 0 index arg num.          */
                                       /* Determine the type of argument  */
    argList->get_item(&ev, argNum, &argName, &tc, &dummy, &argLength, &flags);
    kind = TypeCode_kind(tc, &ev);

                                       /* retrive the matching OREXX obj  */
                                       /* Was this argument omitted?      */
    if ((thisObject = args[i -1]) == OREF_NULL) {
                                       /* Yes, not allowed to omit args   */
     if (!descobjProvided) {
      descobj->somFree();              /* free up any SOM objects we won't*/
     }
     report_exception1(Error_Incorrect_method_noarg, new_integer(i));
    }
    if (flags & ARG_IN) {              /* is it an IN type parameter?     */
      switch (kind) {                  /* determine type of SOM arg and   */
                                       /* convert it.                     */
        case tk_boolean:
                                       /* do the boolean conversion         */
          if (!convertInputBoolean(thisObject, &tempLong))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_BOOLEAN);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempLong, argLength, flags);
          break;

        case tk_char:
                                       /* do the character conversion       */
          if (!convertInputChar(thisObject, &tempChar))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempChar, argLength, flags);
          break;

        case tk_octet:
                                       /* do the octet conversion           */
          if (!convertInputOctet(thisObject, &tempOctet))
            bad_argtype(i, CHAR_CHAR); /* have a bad argument               */
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempOctet, argLength, flags);
          break;

        case tk_short:
                                       /* do the short conversion           */
          if (!convertInputShort(thisObject, &tempShort))
            bad_argtype(i, CHAR_SHORT);/* have a bad argument               */
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempShort, argLength, flags);
          break;

        case tk_ushort:
                                       /* do the short conversion           */
          if (!convertInputUShort(thisObject, &tempUShort))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_USHORT);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempUShort, argLength, flags);
          break;

       case tk_long:
                                       /* do the long conversion            */
          if (!convertInputLong(thisObject, &tempLong))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_LONG);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempLong, argLength, flags);
          break;

        case tk_ulong:
                                       /* do the long conversion            */
          if (!convertInputULong(thisObject, &tempULong))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ULONG);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempULong, argLength, flags);
          break;

        case tk_float:
                                       /* do the long conversion            */
          if (!convertInputFloat(thisObject, &tempFloat))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_FLOAT);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempFloat, argLength, flags);
          break;

        case tk_double:
                                       /* do the long conversion            */
          if (!convertInputDouble(thisObject, &tempDouble))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_DOUBLE);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempDouble, argLength, flags);
          break;

       case tk_objref:
                                       /* do the long conversion            */
          if (!convertInputObjectRef(thisObject, &tempToken))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_SOMObject);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempToken, argLength, flags);
          break;

        case tk_pointer:
                                       /* do the long conversion            */
          if (!convertInputPointer(thisObject, &tempToken, saveTable, tc, &ev))
                                       /* have a bad argument               */
            bad_argtype(i, "pointer");
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempToken, argLength, flags);
          break;

        case tk_struct:
                                       /* do the long conversion            */
          convertInputStructure(thisObject, &tempAddr, tc, &ev);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempAddr, argLength, flags);
          break;

        case tk_string:
                                       /* do the conversion                 */
          convertInputString(thisObject, (PCHAR *)&tempAddr, saveTable, &ev);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempAddr, argLength, flags);
          break;

        case tk_array:
          if (!convertInputArray(thisObject, tc, &tempAddr, saveTable, &ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ARRAY);
          argList->set_item(&ev, argNum, argName, tc, &tempAddr, argLength, flags);
          break;

        case tk_sequence:
          if (!convertInputSequence(thisObject, tc, &tempSeq, saveTable, &ev))
                                       /* have a bad argument               */
            bad_argtype(i, CHAR_ARRAY);
          argList->set_item(&ev, argNum, argName, tc, &tempAddr, argLength, flags);
          break;

        case tk_any:                   /* The any type.                     */
                                       /* do the conversion                 */
          convertInputAny(thisObject, &tempAny, &tempLong, &tempDouble);
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempAny, argLength, flags);
          break;

        case tk_enum:                  /* The enum type.                    */
                                       /* do the conversion                 */
          if (!convertInputEnum(thisObject, tc, &tempULong, &ev))
            bad_argtype(i, "ENUM");
                                       /* add to the argument list          */
          argList->set_item(&ev, argNum, argName, tc, &tempULong, argLength, flags);
          break;

        case tk_foreign:
          {
                                       /* Is it a somID?                    */
            if (foreign_arg(tc,&ev) == 's') {
                                       /*  yes add it to the SOM ArgLIst    */
                                       /* NOTE: at some point should        */
                                       /* actually remember and SOMFree     */
                                       /* these ids.                        */
              tempId = somIdFromString(thisObject->stringValue()->stringData);
              argList->set_item(&ev, argNum, argName, tc, &tempId, argLength, flags);
//            SOMFree(tempId);         /* Not sure of owner when added....  */
                                       /* Don't  free just yet.             */
            }
            else
              unsupported_type(i, "???");
            break;
          }
        default:
          unsupported_type(i, "???");
          break;
      } /*switch*/
    }
    else {                             /* Nope it must be INOUT or OUT      */
                                       /* INOUT/OUT parameters must be      */
                                       /*  at a DLFObject/DLFNumber         */


                                       /* convert OREXX object to Object  */
                                       /* Ref done by looking up the obj  */
                                       /* in the server and getting the   */
                                       /* SOM Object counterpart          */
      thisObject = ProcessLocalServer->sendMessage(OREF_SOMOBJ,thisObject);
                                       /* able to get the SOMObject?      */
      if (thisObject != TheNilObject) {
                                       /* resolve pointer to SOMObject *    */
        somObj = (SOMObject *)((RexxInteger *)thisObject)->value;
                                       /* is this at least a DLFNumber objec*/
        if (!somObj->somIsA(_DLFObject)) {
          bad_argtype(i, CHAR_DLFObject);

        }
      } else {
                                       /* nope report Exception.            */
        bad_argtype(i, CHAR_DLFObject);
      }


      switch (kind) {                  /* determine type of SOM arg and     */
                                       /* convert it.                       */
        case tk_boolean:
                                       /* Is object a DLFBoolean            */
          if (somObj->somIsA(_DLFBoolean)) {
                                       /* Add reference to boolean output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFBoolean *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFBoolean);
          }
          break;
        case tk_char:
                                       /* Is object a DLFChar               */
          if (somObj->somIsA(_DLFChar)) {
                                       /* Add reference to boolean output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFChar *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFChar);
          }
          break;
        case tk_octet:
                                       /* Is object a DLFOctet              */
          if (somObj->somIsA(_DLFOctet)) {
                                       /* Add reference to octet   output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFOctet *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFOctet);
          }
          break;
        case tk_short:
                                       /* Is object a DLFShort              */
          if (somObj->somIsA(_DLFShort)) {
                                       /* Add reference to short   output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFShort *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFShort);
          }
          break;
        case tk_ushort:
                                       /* Is object a DLFUShort             */
          if (somObj->somIsA(_DLFUShort)) {
                                       /* Add reference to UShort  output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFUShort *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFUShort);
          }
          break;
        case tk_long:
                                       /* Is object a DLFLong               */
          if (somObj->somIsA(_DLFLong)) {
                                       /* Add reference to long    output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFLong *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFLong);
          }
          break;
        case tk_ulong:
                                       /* Is object a DLFULong              */
          if (somObj->somIsA(_DLFULong)) {
                                       /* Add reference to ULong   output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFULong *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFULong);
          }
          break;
        case tk_float:
                                       /* Is object a DLFUFloat             */
          if (somObj->somIsA(_DLFFloat)) {
                                       /* Add reference to float   output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFFloat *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFFloat);
          }
          break;
        case tk_double:
                                       /* Is object a DLFDouble             */
          if (somObj->somIsA(_DLFDouble)) {
                                       /* Add reference to double  output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFDouble *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFDouble);
          }
          break;
        case tk_string:
                                       /* Is object a DLFString             */
          if (somObj->somIsA(_DLFString)) {
                                       /* Add reference to string  output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFString *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFString);
          }
          break;
        case tk_struct:
                                       /* Is object a DLFStruct             */
          if (somObj->somIsA(_DLFStruct)) {
                                       /* Add reference to string  output   */
            argList->set_item(&ev, argNum, argName, tc, ((DLFStruct *)somObj)->_get_address(&ev), argLength, flags);
          }else {
            bad_argtype(i, CHAR_DLFStruct);
          }
          break;
        case tk_objref:
        case tk_pointer:
        case tk_array:                        /* Arrays are currently not supported.  */
        case tk_any:                          /* The any type.                        */
        case tk_foreign:
        default:
          unsupported_type(i, "OUT or INOUT");
         break;
      }
    }
  } /*for*/
                                       /* Retrieve activity for this meth   */
                                       /*  curracti not reliable since we   */
                                       /*  go in and out of kernel.  Also   */
                                       /* want to avoid repeated calls to   */
                                       /* activity_find()                   */
  myActivity= CurrentActivity;
  ReleaseKernelAccess(myActivity);     /* Release kernel access before      */
                                       /* callling SomDispatch              */
  somobj->create_request(&ev, NULL, msgstr, argList, &NVResult, &requestObj, (Flags) 0);
  requestObj->invoke(callEnv, (Flags) 0);
// Should check for errors in callEnv here.....
  tc = NVResult.argument._type;        /* Get return TypeCode.              */
  returnBuff = NVResult.argument._value;
  requestObj->destroy(&ev);
                                       /* Re-aquire kernel access before    */
  RequestKernelAccess(myActivity);     /*  continuing.                      */
                                       /* callling SomDispatch              */
  returnObj = getReturnObject(tc, returnBuff, &ev);
  SOMFree(returnBuff);
                                       /* free any object/ids usedin routine*/
  if (!descobjProvided) {
    SOMFree(msgid);
    descobj->somFree();
  }
// Free up any of the NV, result, request that needs to be

                                       /* is this an IDL style call?        */
  if (!strcmp(CALLSTYLE_IDL,cstyle)) {
                                       /* Yes, check for error conditions   */
                                       /* check   for SOM exception and     */
                                       /*   raise USER exception, may or    */
                                       /*   may not return.                 */
    CheckSOMError(callEnv);
  }
  discard(saveTable);
  return returnObj;                    /* Return the result object.         */
}
#endif

RexxObject *resolve_proxy(
  SOMObject *SOMobject)                /* object to resolve                 */
/******************************************************************************/
/* Function:  Resolve a SOM object to a REXX object (potentially creating     */
/*            a new REXX proxy for this)                                      */
/******************************************************************************/
{
                                       /* pass request to the server        */
 return ProcessLocalServer->sendMessage(OREF_MAKE_PROXY, new_integer((LONG)SOMobject));
}

SOMObject *make_somproxy (RexxObject *oryxobj, SOMClass *sclass)
/******************************************************************************/
/* Arguments:  REXX object, SOM class                                         */
/*                                                                            */
/*  Returned:  SOM object                                                     */
/******************************************************************************/
{
  SOMObject *somobj;
  RexxObject *somoref;

  /* We can't call somNew because we're not set up to handle somInit */
  /* yet (but we need the memory address of the SOM object to        */
  /* complete our setup - almost catch 22).                          */
  somobj = (SOMObject *)calloc(1,(size_t)sclass->somGetInstanceSize());
  somoref = new_pointer(somobj);

  /* When we have the REXX metaclass in play we can use its instance */
  /* data to store the address of the REXX object, but for now we    */
  /* put an entry in clientdir (and servertab, but this is less      */
  /* necessary because we can use the SOMOBJ instance variable to    */
  /* go the other way - if we take care of the dependency on the     */
  /* servertab entry in the server SOMOBJ method).                   */
  ProcessLocalServer->sendMessage(OREF_SETUID,oryxobj,somoref);

  /* Now we're ready to receive messages from SOM, so prove it by    */
  /* causing somInit to be sent.                                     */
  sclass->somRenew(somobj);

  return somobj;
}

/******************************************************************************/
/* Functions Dispatch a message to a SOM object.                              */
/******************************************************************************/
void oryxSomClassDispatchX(SOMObject *target,
                            SOMClass *classObj,
                            void **retVal,
                            somId methodId,
                            va_list *ap)
{
    somMethodData  methodData;

    classObj->somGetMethodData(methodId, &methodData);
    somApply(target, retVal, &methodData, *ap);
}

#endif

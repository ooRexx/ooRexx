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
/* REXX Kernel                                                  ClassClass.c     */
/*                                                                            */
/* Primitive Class Class                                                      */
/*                                                                            */
/******************************************************************************/

#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ListClass.hpp"
#include "TableClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "RexxSOMProxy.hpp"
#include "ClassClass.hpp"
#include "MethodClass.hpp"
#include "RexxActivity.hpp"
#ifdef SOM
  #include "somcls.xh"
  #include "somcm.xh"
  #ifdef DSOM
  #include "somd.xh"
  #endif
#endif

extern RexxObject *ProcessLocalServer; /* local data server object          */
extern RexxDirectory *ProcessLocalEnv; /* local Environment                 */

void RexxClass::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->objectVariables);
  memory_mark(this->id);
  memory_mark(this->classMethodDictionary);
  memory_mark(this->instanceBehaviour);
  memory_mark(this->instanceMethodDictionary);
  memory_mark(this->baseClass);
  memory_mark(this->metaClass);
  memory_mark(this->metaClassMethodDictionary);
  memory_mark(this->metaClassScopes);
  memory_mark(this->somClass);
  memory_mark(this->classSuperClasses);
  memory_mark(this->instanceSuperClasses);
  cleanUpMemoryMark
}

void RexxClass::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->id);
  memory_mark_general(this->classMethodDictionary);
  memory_mark_general(this->instanceBehaviour);
  memory_mark_general(this->instanceMethodDictionary);
  memory_mark_general(this->baseClass);
  memory_mark_general(this->metaClass);
  memory_mark_general(this->metaClassMethodDictionary);
  memory_mark_general(this->metaClassScopes);
  memory_mark_general(this->somClass);
  memory_mark_general(this->classSuperClasses);
  memory_mark_general(this->instanceSuperClasses);
  cleanUpMemoryMarkGeneral
}

void RexxClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
 ;
}

RexxObject *RexxClass::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflaatten an object                                            */
/******************************************************************************/
{
 return this;
}

RexxObject *RexxClass::makeProxy(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Make a proxy object                                             */
/******************************************************************************/
{

                                       /* Following code is pulled from     */
                                       /*  object_primitive, to get class id*/
                                       /*  as a string object.              */
                                       /* get the class id                  */
   return new_proxy(this->id->stringData);
}


/**
 * Hash a class object.  Because behaviors don't always get set
 * up properly with this, we'll always use the primitive one for
 * class objects.
 *
 * @return A "hashed hash" that can be used by the map collections.
 */
ULONG RexxClass::hash()
{
    // always, always, always return the hash value
    return HASHVALUE(this);
}


RexxObject *RexxClass::strictEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two classes                                             */
/******************************************************************************/
{
    return this->equal(other);         /* this is direct object equality    */
}

BOOL RexxClass::isEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two class objects as a strict compare (==)              */
/******************************************************************************/
{
                                       /* If a non-copied (Primitive)       */
                                       /*behaviour Then we can directly     */
                                       /*call primitive method              */
  if (this->behaviour->isPrimitiveBehaviour())
                                       /* can compare at primitive level    */
    return this->equal(other) == TheTrueObject;
  else
                                       /* other wise giveuser version a     */
                                       /*chance                             */
    return this->sendMessage(OREF_STRICT_EQUAL, other)->truthValue(Error_Logical_value_method);
}

RexxObject *RexxClass::equal(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two classes                                             */
/******************************************************************************/
{
  required_arg(other, ONE);            /* must have the other argument      */
                                       /* this is direct object equality    */

                                       /* comparing string/int/numstr to    */
                                       /*  string/int/numstr?               */
  if ((this == TheStringClass || this == TheIntegerClass || this == TheNumberStringClass) &&
      (other == (RexxObject *)TheStringClass || other == (RexxObject *)TheIntegerClass || other == (RexxObject *)TheNumberStringClass))
    return TheTrueObject;              /* YES, then equal....               */
  else                                 /* other wise, do a direct compare   */
    return ((this == other) ? TheTrueObject: TheFalseObject);
}

RexxObject *RexxClass::notEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two classes                                             */
/******************************************************************************/
{
  required_arg(other, ONE);            /* must have the other argument      */
                                       /* this is direct object equality    */

                                       /* comparing string/int/numstr to    */
                                       /*  string/int/numstr?               */
  if ((this == TheStringClass || this == TheIntegerClass || this == TheNumberStringClass) &&
      (other == (RexxObject *)TheStringClass || other == (RexxObject *)TheIntegerClass || other == (RexxObject *)TheNumberStringClass))
    return TheFalseObject;             /* YES, then equal....               */
  else                                 /* other wise, do a direct compare   */
    return ((this != other) ? TheTrueObject: TheFalseObject);
}

RexxInteger *RexxClass::queryMixinClass()
/*****************************************************************************/
/* Function: To check if class_info MIXIN has been set                       */
/*****************************************************************************/
{
                                       /* return TRUE/FALSE indicator       */
  return this->class_info & MIXIN ? TheTrueObject : TheFalseObject;
}

RexxString *RexxClass::getId()
/*****************************************************************************/
/* Function:  Return the ID for the class                                    */
/*****************************************************************************/
{
  return this->id;
}

RexxObject *RexxClass::setRexxDefined(void)
/*****************************************************************************/
/* Function:  Set a class as a Rexx defined class                            */
/*****************************************************************************/
{
  this->class_info |= REXX_DEFINED;    /* flag the class                    */
  return OREF_NULL;
}

RexxClass *RexxClass::getBaseClass()
/*****************************************************************************/
/* Function:  Return the classes base class                                  */
/*****************************************************************************/
{
  return this->baseClass;              /* return the base class for this    */
}

RexxClass *RexxClass::getMetaClass()
/*****************************************************************************/
/* Function:   return the classes metaclass                                  */
/*****************************************************************************/
{
  if (this->isPrimitive())             /* primitive class?                  */
    return TheClassClass;              /* this is always .class             */
  else                                 /* return first member of the list   */
    return (RexxClass *)this->metaClass->get(1);
}

RexxInteger *RexxClass::getSomClass()
/*****************************************************************************/
/* Function:  Retrieve the SOM class                                         */
/*****************************************************************************/
{
  return this->somClass;               /* return the SOM class              */
}

void  RexxClass::setSomClass(
    RexxInteger *somclass)             /* new SOM Class                     */
/*****************************************************************************/
/* Function:  Assoctiate a SOM class with a REXX class                       */
/*****************************************************************************/
{
  OrefSet(this, this->somClass, somclass);
}

void  RexxClass::setInstanceBehaviour(
    RexxBehaviour *behaviour)          /* new instance behaviour            */
/*****************************************************************************/
/* Function:  Give a class a new instance behaviour                          */
/*****************************************************************************/
{
  OrefSet(this, this->instanceBehaviour, behaviour);
}

RexxClass *RexxClass::getSuperClass()
/*****************************************************************************/
/* Function:  Return the first superclass in the superclass list             */
/*****************************************************************************/
{
    // object has no superclasses
    if (this == TheObjectClass)
    {
        return (RexxClass *)TheNilObject;
    }
    // get the first item from the immediate list.
    return (RexxClass *)this->instanceSuperClasses->get(1);
}


RexxArray *RexxClass::getSuperClasses()
/*****************************************************************************/
/* Function:  Return an array of the superclasses                            */
/*****************************************************************************/
{
                                       /* return a copy of the list          */
  return (RexxArray *)this->instanceSuperClasses->copy();
}

RexxArray *RexxClass::getSubClasses()
/*****************************************************************************/
/* Function:  Return an array of the subclasses                              */
/*****************************************************************************/
{
                                       /* return a copy of the list         */
  return TheActivityClass->getSubClassTable()->allAt(this);
}

void RexxClass::addSubClass(RexxClass *subClass)
/*****************************************************************************/
/* Function:  Add a subclass to a class                                      */
/*****************************************************************************/
{
                                       /* just add to the global list       */
  TheActivityClass->newSubClass(subClass, this);
}

void RexxClass::defmeths(
    RexxTable *methods)                /* methods to add                    */
/*****************************************************************************/
/* Function:  Add a table of methods to a primitive class behaviour          */
/*****************************************************************************/
{
  LONG i;                              /* table index                       */
  RexxString * method_name;            /* name of an added method           */

                                       /* loop through the list of methods  */
  for (i = methods->first(); methods->available(i); i = methods->next(i)) {
                                       /* get the method name               */
    method_name = (RexxString *)methods->index(i);
                                       /* add this method to the classes    */
                                       /* class behaviour                   */
    this->behaviour->define(method_name, (RexxMethod *)methods->value(i));
  }
}

RexxString *RexxClass::defaultName()
/******************************************************************************/
/* Function:  retrieve a classes default name value                           */
/******************************************************************************/
{
  RexxString * defaultname;            /* returned default name             */

  defaultname = this->id;              /* use the id directly               */
                                       /* prefix with "The"                 */
  defaultname = defaultname->concatToCstring("The ");
                                       /* add on "class"                    */
  defaultname = defaultname->concatWithCstring(" class");
  return defaultname;                  /* return that value                 */
}

RexxTable *RexxClass::getInstanceBehaviourDictionary()
/*****************************************************************************/
/* Function:   Return the instance behaviour's method dictionary             */
/*****************************************************************************/
{
  RexxTable *methods;                  /* returned method dictioanry        */

                                       /* get the method dictionary         */
  methods = this->instanceBehaviour->getMethodDictionary();
  if (methods == OREF_NULL)            /* no methods defined yet?           */
    return new_table();                /* create a new method table         */
  else
                                       /* just copy the method dictionary   */
    return (RexxTable *)methods->copy();
}

RexxTable *RexxClass::getBehaviourDictionary()
/*****************************************************************************/
/* Function:   Return the class behaviour's method dictionary                */
/*****************************************************************************/
{
  RexxTable *methods;                  /* returned method dictioanry        */

                                       /* get the method dictionary         */
  methods = this->behaviour->getMethodDictionary();
  if (methods == OREF_NULL)            /* no methods defined yet?           */
    return new_table();                /* create a new method table         */
  else
                                       /* just copy the method dictionary   */
    return (RexxTable *)methods->copy();
}

/**
 * Initialize a base Rexx class.
 *
 * @param class_id   The name of the class.
 * @param restricted Whether we should turn the RexxRestricted flag on at this time.
 *                   Some classes get additional customization after initial
 *                   creation, so we delay setting this attribute until the
 *                   class is fully constructed.
 */
void RexxClass::subClassable(PCHAR class_id, bool restricted)
{
                                       /* get a copy of the class instance   */
                                       /* behaviour mdict before the merge   */
                                       /* with OBJECT.  This unmerged mdict  */
                                       /* is kept in this class's            */
                                       /* class_instance_mdict field.        */
  OrefSet(this, this->instanceMethodDictionary, this->getInstanceBehaviourDictionary());
                                       /* Add OBJECT to the behaviour scope  */
                                       /* table                              */
  this->instanceBehaviour->addScope(TheObjectClass);
  if (this != TheObjectClass) {        /* if this isn't the OBJECT class    */
                                       /* Add OBJECT to the behaviour scope  */
                                       /* table                              */
    this->instanceBehaviour->addScope(TheObjectClass);
                                       /* and merge this class's instance    */
                                       /* behaviour with that of OBJECT's    */
    this->instanceBehaviour->merge(TheObjectBehaviour);
  }
                                       /* add self to the scope table        */
  this->instanceBehaviour->addScope(this);
                                       /* get a copy of the class behaviour  */
                                       /* mdict before the merge with the    */
                                       /* CLASS instance behaviour. This     */
                                       /* unmerged mdict is kept in the      */
                                       /* class_mdict field                  */
  OrefSet(this, this->classMethodDictionary, this->getBehaviourDictionary());
                                       /* The merge of the mdict's is order  */
                                       /* specific. By processing OBJECT     */
                                       /* first then CLASS and then the      */
                                       /* rest of the subclassable classes   */
                                       /* the mdict's will be set up         */
                                       /* correctly.                         */
                                       /* In this way merging the CLASS      */
                                       /* behaviour will only be the CLASS   */
                                       /* instance methods when OBJECT is    */
                                       /* processed, but will be CLASS's     */
                                       /* and OBJECT's after CLASS is        */
                                       /* processed                          */
  this->behaviour->merge(TheClassBehaviour);
                                       /* now add the scope levels to this   */
                                       /* class behaviour                    */
                                       /* If this isn't OBJECT put OBJECT    */
                                       /* in first                           */
  if (this != TheObjectClass)
    this->behaviour->addScope(TheObjectClass);
                                       /* if this is OBJECT - merge the      */
                                       /* object instance methods with the   */
                                       /* object class methods               */
  else {
    this->behaviour->merge(TheObjectBehaviour);
                                       /* and put them into the class mdict  */
                                       /* so all the classes will inherit    */
    OrefSet(this, this->classMethodDictionary, this->getBehaviourDictionary());
  }
                                       /* if this isn't CLASS put CLASS in   */
                                       /* next                               */
  if (this != TheClassClass)
    this->behaviour->addScope(TheClassClass);
  this->behaviour->addScope(this);     /* put self into the scope table     */
                                       /* That finishes the class behaviour  */
                                       /* initialization.                    */
                                       /* Now fill in the state data         */

  if (TheObjectClass != this ) {
                                       /* set up the new metaclass list      */
    OrefSet(this, this->metaClass, new_array1(TheClassClass));
                                       /* the metaclass mdict list           */
    OrefSet(this, this->metaClassMethodDictionary, new_array1(TheClassClass->instanceMethodDictionary->copy()));
                                       /* and the metaclass scopes list      */
    OrefSet(this, this->metaClassScopes, (RexxObjectTable *)TheClassClass->behaviour->scopes->copy());
  }

                                       /* The Baseclass for non-mixin classes*/
                                       /* is self                            */
  OrefSet(this, this->baseClass, this);
                                       /* The class superclasses list for    */
                                       /* OBJECT is an empty list.           */
  OrefSet(this, this->classSuperClasses, new_array(0));
                                       /* as is the instance superclasses    */
                                       /* list.                              */
  OrefSet(this, this->instanceSuperClasses, new_array(0));
  if (this != TheObjectClass) {        /* not .object?                      */
                                       /* add object to the list             */
    this->classSuperClasses->addLast(TheObjectClass);
                                       /* The instance superclasses for all  */
                                       /* except OBJECT is OBJECT            */
    this->instanceSuperClasses->addLast(TheObjectClass);
                                       /* and for OBJECT we need to add all  */
                                       /* the other classes                  */
                                       /* except integer and numberstring    */
    if (this != TheIntegerClass && this != TheNumberStringClass)
      TheObjectClass->addSubClass(this);
  }
                                       /* initialize the class id            */
  OrefSet(this, this->id, new_cstring(class_id));
                                       /* and point the instance behaviour   */
                                       /* back to this class                 */
  this->instanceBehaviour->setClass(this);
                                       /* and the class behaviour to CLASS   */
  this->behaviour->setClass(TheClassClass);
                                       /* set the somclass to .nil           */
  OrefSet(this, this->somClass, (RexxInteger *)TheNilObject);
                                       /* these are primitive classes       */
  this->class_info |= PRIMITIVE_CLASS;

  if (this == TheClassClass)           /* mark CLASS as a meta class        */
    this->setMeta();
//                                     /* still causing problems            */
//this->hashvalue = HASHOREF(this);    /* with internal classes!            */
}

RexxObject *RexxClass::defineMethod(
    RexxString * method_name,          /*define method name                 */
    RexxMethod *method_object)         /* returned method object            */
/*****************************************************************************/
/* Function:  Define an instance method on this class object                 */
/*****************************************************************************/
{
  long i;                              /* position in table                 */
                                       /* check if this is a rexx class     */
  if ( this->class_info & REXX_DEFINED )
                                       /* report as a nomethod condition    */
    report_nomethod(last_msgname(), this);
                                       /* make sure there is at least one   */
                                       /* parameter                         */
  method_name = REQUIRED_STRING(method_name, ARG_ONE)->upper();
  if ( OREF_NULL == method_object)     /* 2nd arg omitted?                  */
                                       /* Yes, remove all message with this */
                                       /* name from our instanceMdict       */
                                       /*                 (method lookup)   */
                                       /* done by defining the method       */
                                       /* to be .nil at this class level, so*/
                                       /* when message lookup is attempted  */
                                       /* we get .nil, telling us not found */
    method_object = (RexxMethod *)TheNilObject;
                                       /* not a method type already?        */
                                       /* and not TheNilObject              */
  else if (TheNilObject != method_object && !OTYPE(Method, method_object))
                                       /* make one from a string            */
    method_object = TheMethodClass->newRexxCode(method_name, method_object, IntegerTwo);
  if (TheNilObject != method_object) { /* if the method is not TheNilObject */
                                       /* set the scope of the method to self*/
    method_object = method_object->newScope(this);
                                       /* Installing UNINIT?                */
    if (method_name->strCompare(CHAR_UNINIT)) {
      this->class_info |= HAS_UNINIT;  /* and turn on uninit if so          */
    }
  }

                                       /* make a copy of the instance       */
                                       /* behaviour so any previous objects */
                                       /* aren't enhanced                   */
  OrefSet(this, this->instanceBehaviour, (RexxBehaviour *)this->instanceBehaviour->copy());
                                       /* add method to the instance mdict  */
  this->instanceMethodDictionary->stringPut((RexxObject *)method_object, method_name);
                                       /* any subclasses that we have need  */
                                       /* to redo their instance behaviour  */
                                       /* this also updates our own         */
  this->updateInstanceSubClasses();    /* behaviour table                   */
                                       /* if a SOM mixin, and not imported   */
                                       /* yet, override any Som methods      */
                                       /* that will be imported              */
  if ((this->somClass != TheNilObject) && !(this->class_info & IMPORTED)){
                                       /* loop through the list of methods*/
     for (i = this->instanceMethodDictionary->first();
          this->instanceMethodDictionary->available(i);
          i = this->instanceMethodDictionary->next(i)) {
                                       /* define the methods as som methods */
       this->somDefine((RexxString *)this->instanceMethodDictionary->index(i), this->somClass);
     }
  }
  return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::defineMethods(
    RexxTable * methods)               /* new table of methods to define    */
/*****************************************************************************/
/* Function:  Define instance methods on this class object                   */
/*****************************************************************************/
{
  long i;                              /* loop counter                      */
  RexxString * index;                  /* method name                       */
  RexxMethod * method;                 /* new method to process             */
                                       /* loop thru the methods setting the */
                                       /* method scopes to SELF and then    */
                                       /* adding them to SELF's instance    */
                                       /* mdict                             */
  for (i = methods->first(); (index = (RexxString *)methods->index(i)) != OREF_NULL; i = methods->next(i)) {
                                       /* get the method                    */
    method = (RexxMethod *)methods->value(i);
    if (OTYPE(Method, method))         /* if this is a method object        */
      method->setScope(this);          /* change the scope                  */
                                       /* add method to the instance mdict   */
    this->instanceMethodDictionary->stringPut(method, index);
                                       /* Installing UNINIT?                */
    if (index->strCompare(CHAR_UNINIT)) {
      this->class_info |= HAS_UNINIT;  /* and turn on uninit if so          */
    }
  }
                                       /* create the instance behaviour from */
                                       /* the instance superclass list       */
  this->instanceBehaviour->setMethodDictionary(OREF_NULL);
  this->instanceBehaviour->setScopes(OREF_NULL);
  this->createInstanceBehaviour(this->instanceBehaviour);

                                       /* if a SOM mixin, and not imported   */
                                       /* yet, override any Som methods      */
                                       /* that will be imported              */
  if ((this->somClass != (RexxInteger *)TheNilObject) && !(this->class_info & IMPORTED)) {
                                       /* loop through the list of methods   */
     for (i = methods->first(); (index = (RexxString *)methods->index(i)) != OREF_NULL; i = methods->next(i)) {
                                       /* define the methods as som methods  */
        this->somDefine(index, this->somClass);
     }
  }

  return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::deleteMethod(
    RexxString  *method_name)          /* deleted method name               */
/*****************************************************************************/
/* Function:  Delete an instance method on this class object                 */
/*****************************************************************************/
{
  if (this->class_info & REXX_DEFINED) /* check if this is a rexx class     */
                                       /* report as a nomethod condition    */
    report_nomethod(last_msgname(), this);
                                       /* and that it can be a string        */
  method_name = REQUIRED_STRING(method_name, ARG_ONE)->upper();
                                       /* make a copy of the instance        */
                                       /* behaviour so any previous objects  */
                                       /* aren't enhanced                    */
  OrefSet(this, this->instanceBehaviour, (RexxBehaviour *)this->instanceBehaviour->copy());
                                       /* if there is a method to remove     */
                                       /* from the instance mdict            */
                                       /* remove it                          */
  if (OREF_NULL != this->instanceMethodDictionary->remove(method_name))
                                       /* and update our instance behaviour  */
    this->updateInstanceSubClasses();  /* along with our subclasses         */
  return OREF_NULL;                    /* returns nothing                   */
}

RexxMethod *RexxClass::method(
    RexxString  *method_name)
/*****************************************************************************/
/* Function:  Return the method object for the method name                   */
/*****************************************************************************/
{
  RexxMethod * method_object;          /* mdict value for the method name   */
                                       /* make sure we have a proper name    */
  method_name = REQUIRED_STRING(method_name, ARG_ONE)->upper();
                                       /* check if it is in the mdict        */
  if ( OREF_NULL == (method_object = (RexxMethod *)this->instanceBehaviour->getMethodDictionary()->stringGet(method_name)))
                                       /* if not return an error             */
    report_exception2(Error_No_method_name, this, method_name);
  return method_object;                /* if it was - return the value      */
}

RexxSupplier *RexxClass::methods(
    RexxClass *class_object)           /* target class object               */
/*****************************************************************************/
/* Function:  If no qualification parameter entered                          */
/*              return all the methods that an instance of this class        */
/*              will inherit                                                 */
/*            If TheNilObject  is the qualification parameter                */
/*              return just the methods introduced at this class scope       */
/*            For any other qualification parameter                          */
/*              return just the methods introduced at that class scope       */
/*****************************************************************************/
{
                                       /* if no parameter specified         */
                                       /* return my  behaviour mdict as a   */
                                       /* supplier object                   */
  if (class_object == OREF_NULL)
    return this->instanceBehaviour->getMethodDictionary()->supplier();
                                       /* if TheNilObject specified         */
                                       /*  return my instance mdict as a    */
                                       /*  supplier object                  */
  if (class_object == TheNilObject)
    return this->instanceMethodDictionary->supplier();
                                       /* if not one of the above           */
                                       /* check if it is a  superclass      */
  if (this->behaviour->checkScope(class_object))
                                       /*  let the class specified return   */
                                       /*  it's own methods                 */
    return (RexxSupplier *)send_message1(class_object, OREF_METHODS, TheNilObject);
                                       /* or just return a null supplier    */
  return (RexxSupplier *)TheNullArray->supplier();
}

void  RexxClass::updateSubClasses()
/******************************************************************************/
/* Function: Update my behaviours and call each subclass to do the same       */
/******************************************************************************/
{
  size_t       index;                  /* subclass index number             */
  RexxArray   *subClasses;             /* array of class subclasses         */
                                       /* start out the class mdict with    */
                                       /* a clear mdict and scopes tables   */
  this->behaviour->setMethodDictionary(OREF_NULL);
  this->behaviour->setScopes(OREF_NULL);
                                       /* create the instance behaviour from*/
                                       /* the instance superclass list      */
  this->instanceBehaviour->setMethodDictionary(OREF_NULL);
  this->instanceBehaviour->setScopes(OREF_NULL);
  this->createInstanceBehaviour(this->instanceBehaviour);
                                       // This time, we update the class behaviour
                                       // after building the instance behaviour
                                       // because the added methods may have an
                                       // impact on metaclasses.
  this->createClassBehaviour(this->behaviour);

  subClasses = this->getSubClasses();  /* get the subclasses list           */
  save(subClasses);                    /* protect this                      */
                                       /* loop thru the subclass doing the  */
                                       /* same for each of them             */
  for (index = 1; index <= subClasses->size(); index++) {
                                       /* get the next subclass             */
                                       /* and recursively update them       */
    ((RexxClass *)subClasses->get(index))->updateSubClasses();
  }
  discard_hold(subClasses);            /* release the lock                  */
}

void RexxClass::updateInstanceSubClasses()
/******************************************************************************/
/* Function: Update my instance behaviour and have the subclasses do the same */
/******************************************************************************/
{
  size_t        index;                 /* working index                     */
  RexxArray   *subClasses;             /* array of class subclasses         */
                                       /* create the instance behaviour from*/
                                       /* the instance superclass list      */
  this->instanceBehaviour->setMethodDictionary(OREF_NULL);
  this->instanceBehaviour->setScopes(OREF_NULL);
  this->createInstanceBehaviour(this->instanceBehaviour);
  subClasses = this->getSubClasses();  /* get the subclasses list           */
  save(subClasses);                    /* protect this                      */
                                       /* loop thru the subclass doing the  */
                                       /* same for each of them             */
  for (index = 1; index <= subClasses->size(); index++) {
                                       /* get the next subclass             */
                                       /* recursively update these          */
    ((RexxClass *)subClasses->get(index))->updateInstanceSubClasses();
  }
  discard_hold(subClasses);            /* release the lock                  */
}

void RexxClass::createClassBehaviour(
    RexxBehaviour *target_class_behaviour)
/*****************************************************************************/
/* Funcion:  To call the superclasses and have them update this classes      */
/*           class behaviour mdict and scopes table                          */
/*****************************************************************************/
{
  RexxClass   * superclass;            /* superclass being called           */
  LONG          index;                 /* index into list                   */
  RexxClass   * metaclass;             /* metaclass to use                  */


                                       /* Call each of the superclasses in  */
                                       /* this superclass list starting from*/
                                       /* the last to the first             */
  for (index = this->classSuperClasses->size(); index > 0; index--) {
                                       /* get the next superclass           */
    superclass = (RexxClass *)this->classSuperClasses->get(index);
                                       /* if there is a superclass and      */
                                       /* it hasn't been added into this    */
                                       /* behaviour yet, call and have it   */
                                       /* add itself                        */
    if (superclass != TheNilObject && !target_class_behaviour->checkScope(superclass))
      superclass->createClassBehaviour(target_class_behaviour);
  }
                                       /* If this class mdict has not been   */
                                       /* merged into this target behaviour  */
  if (!target_class_behaviour->checkScope(this)) {
    if (TheObjectClass != this) {      /* if this isn't OBJECT              */
                                       /* go through each of the metaclasses*/
                                       /* in list to see which one's have   */
                                       /* not been added yet                */
      for (index = this->metaClass->size(); index > 0; index--) {
        metaclass = (RexxClass *)this->metaClass->get(index);
                                       /* add which ever metaclasses have    */
                                       /* not been added yet                 */
        if (metaclass != TheNilObject && !target_class_behaviour->checkScope(metaclass)) {
                                       /* merge in the meta class mdict      */
          target_class_behaviour->methodDictionaryMerge(metaclass->instanceBehaviour->getMethodDictionary());
          // now we need to merge in the scopes.  For each metaclass, starting
          // from the bottom of the hierarchy down, merge in each of the scope
          // values.
          RexxArray *addedScopes = metaclass->behaviour->scopes->allAt(TheNilObject);
          save(addedScopes);
          LONG i;

          // these need to be processed in reverse order
          for (i = addedScopes->size(); i > 0; i--)
          {
              RexxClass *scope = (RexxClass *)addedScopes->get(i);
              target_class_behaviour->mergeScope(scope);
          }

          discard(addedScopes);
        }
      }
    }
                                       /* only merge the mdict for CLASS     */
                                       /* if this is a capable of being a    */
                                       /* metaclass                          */
    if ((this != TheClassClass) || (this == TheClassClass && this->queryMeta())) {
                                       /* Merge this class mdict with the    */
                                       /* target behaviour class mdict       */
      target_class_behaviour->methodDictionaryMerge(this->classMethodDictionary);
    }
                                       /* And update the target behaviour    */
                                       /* scopes table with this class       */
    if (this != TheClassClass && !target_class_behaviour->checkScope(this))
      target_class_behaviour->addScope(this);
  }
}


void RexxClass::createInstanceBehaviour(
                                       /* target behaviour to create        */
    RexxBehaviour *target_instance_behaviour)
/*****************************************************************************/
/* Funcion:  To call the superclasses and have them update this classes      */
/*           instance behaviour mdict and scopes table                       */
/*****************************************************************************/
{
  RexxClass   * superclass;            /* superclass being called           */
  LONG          index;                 /* index into list                   */
                                       /* Call each of the superclasses in  */
                                       /* this superclass list starting from*/
                                       /* the last going to the first       */
  for (index = this->instanceSuperClasses->size(); index > 0; index--) {
                                       /* get the next super class          */
    superclass = (RexxClass *)this->instanceSuperClasses->get(index);
                                       /* if there is a superclass and      */
                                       /* it hasn't been added into this    */
                                       /* behaviour yet, call and have it   */
                                       /* add itself                        */
    if (superclass != TheNilObject && !target_instance_behaviour->checkScope(superclass))
      superclass->createInstanceBehaviour(target_instance_behaviour);
  }
                                       /* If this class mdict has not been   */
                                       /* merged into this target behaviour  */
  if (!target_instance_behaviour->checkScope(this)) {
                                       /* Merge this class mdict with the    */
                                       /* target behaviour class mdict       */
    target_instance_behaviour->methodDictionaryMerge(this->instanceMethodDictionary);
                                       /* And update the target behaviour    */
                                       /* scopes table with this class       */
    target_instance_behaviour->addScope(this);
  }
}

void RexxClass::methodDictionaryMerge(
    RexxTable  *source_mdict,          /* source method dictionary          */
    RexxTable  *target_mdict)          /* target method dictionary          */
/*****************************************************************************/
/* Function:  Merge the source mdict methods into the target mdict after     */
/*            getting copies of the methods with a new scope                 */
/*            After this merge the method search order will find the source  */
/*            mdict methods prior to the target methods                      */
/*****************************************************************************/
{

  RexxMethod   *method;                /* method to be added                */
  RexxString   *method_name;           /* method name to be added           */
  LONG          i;                     /* table index for traversal         */

  if (source_mdict == OREF_NULL)       /* check for a source mdict          */
    return;                            /* there isn't anything to do        */
                                       /* just loop through the entries     */
  for (i = source_mdict->first(); source_mdict->available(i); i = source_mdict->next(i)) {
                                       /* get the method name               */
    method_name = REQUEST_STRING(source_mdict->index(i));
                                       /* get the method                    */
    method = (RexxMethod *)source_mdict->value(i);
                                       /* add the method to the target mdict */
    target_mdict->stringAdd(method, method_name);
                                       /* check if the method that was added */
                                       /* is the uninit method               */
    if ( method_name->strCompare(CHAR_UNINIT))
      this->class_info |= HAS_UNINIT;  /* and turn on uninit if so          */
  }
}

RexxTable *RexxClass::methodDictionaryCreate(
    RexxTable  *sourceCollection,      /* source method collection          */
    RexxClass  *scope )                /* required method scope             */
/*****************************************************************************/
/* Function:  Process a collection of methods that will be added to a class  */
/*            as class methods, or will be added to an enhanced object.  In  */
/*            either case, this is an arbitrary collection of objects that   */
/*            may need conversion into method objects and given a scope.     */
/*****************************************************************************/
{
  RexxTable    *newDictionary;         /* returned merged mdict             */
  RexxMethod   *method;                /* method to be added                */
  RexxString   *method_name;           /* method name to be added           */
  RexxSupplier *supplier;              /* working supplier object           */

  newDictionary = new_table();         /* get a new table for this          */
  save(newDictionary);                 /* and save this                     */
                                       /* loop thru the supplier object     */
                                       /* obtained from the source mdict    */
  supplier = (RexxSupplier *)sourceCollection->sendMessage(OREF_SUPPLIERSYM);
  save(supplier);                      /* save the supplier too             */
  for (; supplier->available() == TheTrueObject; supplier->next()) {
                                       /* get the method name (uppercased)  */
    method_name = REQUEST_STRING(supplier->index())->upper();
                                       /* get the method                    */
    method = (RexxMethod *)supplier->value();
                                       /* if the method is not TheNilObject */
    if (method != (RexxMethod *)TheNilObject) {
                                       /* and it isn't a primitive method   */
      if (!OTYPE(Method, method)) {    /* object                            */
                                       /* make it into a method object      */
         method = TheMethodClass->newRexxCode(method_name, method, IntegerOne);
         method->setScope(scope);      /* and set the scope to the given    */
      }
      else {
                                       /* if it is a primitive method object */
                                       /* let the newscope method copy it    */
         method = method->newScope(scope);
      }
    }
                                       /* add the method to the target mdict */
    newDictionary->stringAdd(method, method_name);
  }
  discard(supplier);                   /* done with the supplier            */
  discard_hold(newDictionary);         /* and also the dictionary           */
  return newDictionary;                /* and return the new version        */
}

RexxObject *RexxClass::somSuperClass(
    RexxClass  *superclass)
/*****************************************************************************/
/* Funcion:  Check if the superclass is a somclass and the reciever is not   */
/*           if so make a proxy of the superclass                            */
/*                 Import the superclass' somclass                           */
/*                 Subclass the somclass                                     */
/*                 Somdefine any methods at the recievers scope              */
/*                 Export the somclass subclass                              */
/*                 Keep track of it in the localserver directory             */
/*****************************************************************************/
{
#ifdef SOM
  RexxClass   * somproxy;              /* used to hold the som proxy        */
  RexxClass   * som_super;             /* and the som proxy super class     */
  long i;                              /* loop counter                      */
  SOMClass    * superMeta;             /* super class meta class            */

                                       /* Check if this is a som subclassing*/
                                       /* To a non som class                */
  if (TheNilObject != (RexxObject *)superclass->getSomClass() && TheNilObject == (RexxObject *)this->somClass) {
                                       /* and there is some class methods   */
    if (this->classMethodDictionary->items() > 0) {
                                       /* create the some proxy             */
      superMeta = SOM_GetClass(superclass->getSomClass()->value);
                                       /* Import the superclasses class     */
      somproxy = (RexxClass *)ProcessLocalServer->sendMessage(OREF_IMPORT, new_cstring(superMeta->somGetName()));
                                       /* now create new subclass for this  */
                                       /*  classes Meta.                    */
      som_super = somproxy->subclass(this->id->concatToCstring("M_"), OREF_NULL, OREF_NULL);
                                       /* loop through the list of methods  */
      for (i = this->classMethodDictionary->first(); this->classMethodDictionary->available(i); i = this->classMethodDictionary->next(i)) {
                                       /* defining them to SOM super class  */
        som_super->defineMethod((RexxString *)this->classMethodDictionary->index(i), (RexxMethod *)this->classMethodDictionary->value(i));
      }
    }
    else
      som_super = (RexxClass *)TheNilObject;
                                       /* now export the new SOM class.     */
    OrefSet(this, this->somClass, (RexxInteger *)this->exportMethod(this->id, superclass->id, this->somInterfaces(), som_super));
                                       /* add the class to the SOM list     */
    ProcessLocalServer->sendMessage(OREF_ADDCLASS, this, this->somClass);
    if (OTYPENUM(somproxy, this) || OTYPENUM(somproxy_class, this))
      this->initProxy(this->somClass);
  }
#endif
  return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::inherit(
    RexxClass  *mixin_class,           /* target class                      */
    RexxClass  *position)              /* target inherit position           */
/*****************************************************************************/
/* Function:  To add the mixin class (parameter one) to the superclass       */
/*            hierarchy of the receiver class (this), at the last position   */
/*            or the specified position (parameter two).                     */
/*****************************************************************************/
{
  LONG          class_index;           /* index for class superclasses list */
  LONG          instance_index;        /* index for instance superclasses   */
  long i;                              /* loop counter                      */
                                       /* make sure this isn't a rexx       */
  if (this->rexxDefined())             /* defined class being changed       */
                                       /* report as a nomethod condition    */
    report_nomethod(last_msgname(), this);
  required_arg(mixin_class, ONE);      /* make sure it was passed in        */

                                       /* check the mixin class is really a */
                                       /* good class for this               */
  if (!OTYPENUM(class, mixin_class) || !mixin_class->queryMixin())
                                       /* if it isn't raise an error        */
    report_exception1(Error_Execution_mixinclass, mixin_class);

                                       /* if the mixin class is also the    */
  if (this == mixin_class )            /* reciever class                    */
                                       /*  raise an error                   */
    report_exception2(Error_Execution_recursive_inherit, this, mixin_class);
                                       /* check that the mixin class is not */
                                       /* a superclass of the reciever      */
  if (this->behaviour->checkScope(mixin_class))
                                       /* if it is raise an error           */
    report_exception2(Error_Execution_recursive_inherit, this, mixin_class);
                                       /* check if the reciever class is a  */
                                       /* superclass of the mixin class     */
  if (mixin_class->behaviour->checkScope(this))
                                       /* if it is it's an error            */
    report_exception2(Error_Execution_recursive_inherit, this, mixin_class);

                                       /* Now ensure the mixin class        */
                                       /* baseclass is in the reciever's    */
                                       /* class superclass hierarchy        */
  if (!this->behaviour->checkScope(mixin_class->getBaseClass()))
                                       /* if it isn't raise an error        */
    report_exception3(Error_Execution_baseclass, this, mixin_class, mixin_class->getBaseClass());

                                       /* and the reciever's                */
                                       /* instance superclass hierarchy     */
  if (!this->instanceBehaviour->checkScope(mixin_class->getBaseClass()))
                                       /* if it isn't raise an error        */
    report_exception3(Error_Execution_baseclass, this, mixin_class, mixin_class->getBaseClass());
  if ( position == OREF_NULL ){        /* if position was not specified     */
                                       /* insert the mixin class last in the*/
                                       /* reciever's superclasses list      */
    this->classSuperClasses->addLast(mixin_class);
    this->instanceSuperClasses->addLast(mixin_class);
  }
  else {                               /* if it was specified               */
                                       /* check that it's a valid superclass*/
                                       /* in the class superclasses list    */
                                       /* and the reciever's                */
                                       /* instance superclasses list        */
    if (((class_index = this->classSuperClasses->indexOf(position)) == 0) ||
       ((instance_index = this->instanceSuperClasses->indexOf(position)) == 0))
                                       /* if it isn't raise an error        */
      report_exception2(Error_Execution_uninherit, this, position);
                                       /* insert the mixin class into the   */
                                       /* superclasses list's               */
    this->classSuperClasses->insertItem(mixin_class, class_index + 1);
    this->instanceSuperClasses->insertItem(mixin_class, instance_index + 1);
    this->somSuperClass(mixin_class);  /* see if any som stuff needs doing  */
  }

                                       /* update the mixin class subclass   */
  mixin_class->addSubClass(this);      /* list to reflect this class        */
                                       /* any subclasses that we have need  */
                                       /* to redo their behaviour's         */
                                       /* this also updates our own         */
                                       /* behaviour tables.                 */
  this->updateSubClasses();
                                       /* if a SOM mixin, and not imported  */
                                       /* yet, override any Som methods     */
                                       /* that will be imported             */
  if ((this->somClass != TheNilObject) && !(this->class_info & IMPORTED)) {
                                       /* loop through the list of methods  */
     for (i = this->instanceMethodDictionary->first();
          this->instanceMethodDictionary->available(i);
          i = this->instanceMethodDictionary->next(i)) {
                                       /* define the methods as som methods */
       this->somDefine((RexxString *)this->instanceMethodDictionary->value(i), this->somClass);
     }
  }
  /* If the mixin class has an uninit defined, the new class must have one, too */
  if (mixin_class->uninitDefined() || mixin_class->parentUninitDefined()) {
     this->parentHasUninit();
  }
  return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::uninherit(
    RexxClass  *mixin_class)           /* target subclass to remove         */
/*****************************************************************************/
/* Function:  To remove a mixin class (parameter one) from the superclass    */
/*            hierarchy of the receiver class (this).                        */
/*****************************************************************************/
{
  LONG         class_index;            /* index for class superclasses list */
  LONG         instance_index;         /* index for instance superclasses   */
                                       /* make sure this isn't rexx defined */
  if (this->rexxDefined())             /* class that is being changed       */
                                       /* report as a nomethod condition    */
    report_nomethod(last_msgname(), this);
  required_arg(mixin_class, ONE);      /* make sure it was passed in        */

                                       /* check that the mixin class is a   */
                                       /* superclass of the receiver class  */
                                       /* and not the superclass            */
  if ( ((class_index = this->classSuperClasses->indexOf(mixin_class)) > 1) &&
       ((instance_index = this->instanceSuperClasses->indexOf(mixin_class)) > 1)) {
                                       /* and remove it                     */
    this->classSuperClasses->deleteItem(class_index);
    this->instanceSuperClasses->deleteItem(instance_index);
  }
  else
                                       /* else     raise an error           */
    report_exception2(Error_Execution_uninherit, this, mixin_class);
                                       /* update the mixin class subclass    */
                                       /* list to not have this class        */
  TheActivityClass->getSubClassTable()->removeItem(mixin_class, this);
                                       /* any subclasses that we have need   */
                                       /* to redo their behaviour's          */
                                       /* this also updates our own behaviour*/
  this->updateSubClasses();            /* tables.                           */
  return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::enhanced(
    RexxObject **args,                 /* enhanced arguments                */
    size_t       argCount)             /* the number of arguments           */
/*****************************************************************************/
/* Function:  Create a new object that is an instance of the receiver class  */
/*            object that has had it's instance mdict enhanced.              */
/*****************************************************************************/
{
                                       /* parameter passed in array         */
  RexxTable     * enhanced_instance_mdict;
  RexxClass     * dummy_subclass;      /* new subclass to enhance           */
  RexxObject    * enhanced_object;     /* enhanced object                   */

  if (argCount == 0)                   /* make sure an arg   was passed in  */
                                       /* if not report an error            */
    report_exception1(Error_Incorrect_method_minarg, IntegerOne);
                                       /* get the value of the arg          */
  enhanced_instance_mdict = (RexxTable *)args[0];
                                       /* make sure it was a real value     */
  required_arg(enhanced_instance_mdict, ONE);
                                       /* subclass the reciever class       */
  dummy_subclass = (RexxClass *)save(this->subclass(new_cstring("Enhanced Subclass"), OREF_NULL, OREF_NULL));
                                       /* turn into a real method dictionary*/
  enhanced_instance_mdict = dummy_subclass->methodDictionaryCreate(enhanced_instance_mdict, (RexxClass *)TheNilObject);
                                       /* enhance the instance behaviour    */
  dummy_subclass->methodDictionaryMerge(enhanced_instance_mdict, dummy_subclass->instanceMethodDictionary);
                                       /* and record the changes in behavior*/
  dummy_subclass->instanceBehaviour->setInstanceMethodDictionary(enhanced_instance_mdict);
                                       /* recreate the instance behaviour   */
  dummy_subclass->instanceBehaviour->setMethodDictionary(OREF_NULL);
  dummy_subclass->instanceBehaviour->setScopes(OREF_NULL);
  dummy_subclass->createInstanceBehaviour(dummy_subclass->instanceBehaviour);
                                       /* get an instance of the enhanced   */
                                       /* subclass                          */
  enhanced_object = dummy_subclass->sendMessage(OREF_NEW, args + 1, argCount - 1);
                                       /* change the create_class in the    */
                                       /* instance behaviour to point to the*/
                                       /* original class object             */
  enhanced_object->behaviour->setClass(this);
                                       /* remember it was enhanced          */
  enhanced_object->behaviour->setEnhanced();
  discard(dummy_subclass);             /* now the dummy is not needed       */

  return enhanced_object;              /* send back the new improved version*/
}

RexxClass  *RexxClass::mixinclass(
    RexxString  * id,                  /* ID name of the class              */
    RexxClass   * meta_class,          /* source meta class                 */
                                       /* extra class methods               */
    RexxTable   * enhancing_class_methods)
/*****************************************************************************/
/* Function:  Create a new class object containng class and instance methods */
/*            to be used for multiple inheritance .                          */
/*****************************************************************************/
{
  RexxClass * mixin_subclass;          /* new mixin subclass                */
                                       /* call subclass with the parameters */
  mixin_subclass = this->subclass(id, meta_class, enhancing_class_methods);
  mixin_subclass->setMixinClass();     /* turn on the mixin info            */
                                       /* change the base class to the base */
                                       /* class of the reciever             */
  OrefSet(mixin_subclass, mixin_subclass->baseClass, this->baseClass);
  /* If the mixin's parent class has an uninit defined, the new mixin class must have one, too */
  if (this->uninitDefined() || this->parentUninitDefined()) {
     mixin_subclass->parentHasUninit();
  }
  return mixin_subclass;               /* return the new mixin class        */
}


RexxClass  *RexxClass::subclass(
    RexxString  * id,                  /* ID name of the class              */
    RexxClass   * meta_class,          /* source meta class                 */
                                       /* extra class methods               */
    RexxTable   * enhancing_class_methods)
/*****************************************************************************/
/* Function:  Create a new class object that is a subclass of this class     */
/*            object.                                                        */
/*****************************************************************************/
{
  RexxClass  *new_class;               /* newly created class               */

  if (meta_class == OREF_NULL)         /* if there is no metaclass specified*/
    meta_class = this->getMetaClass(); /* use the default metaclass         */

  if (!meta_class->queryMeta())        /* check that it is a meta class     */
    report_exception1(Error_Translation_bad_metaclass, meta_class);
                                       /* get a copy of the metaclass class */
  new_class = (RexxClass *)save(meta_class->sendMessage(OREF_NEW, id));
  new_class->hashvalue = HASHOREF(new_class);
  if (this->queryMeta()) {             /* if the superclass is a metaclass  */
    new_class->setMeta();              /* mark the new class as a meta class*/
                                       /* and if the metaclass lists haven't */
                                       /* been updated yet                   */
    if (new_class->metaClassScopes->get(this) == OREF_NULL) {
                                       /* add the class instance info to the */
                                       /* metaclass lists                    */
      new_class->metaClass->addFirst(this);
                                       /* the metaclass mdict list           */
      new_class->metaClassMethodDictionary->addFirst(this->instanceMethodDictionary);
                                       /* and the metaclass scopes list      */
                                       /* this is done by adding all the     */
                                       /* scope information of the new class */
      new_class->metaClassScopes->add(this, TheNilObject);
                                       /* add the scope list for this scope  */
      new_class->metaClassScopes->add(new_class->metaClassScopes->allAt(TheNilObject), this);
    }
  }
                                       /* set up the new_class behaviour     */
                                       /* to match the subclass reciever     */
  new_class->instanceBehaviour->subclass(this->instanceBehaviour);
                                       /* set this class as the superclass   */
                                       /* for the new class'                 */
                                       /* class_superclasses list            */
  OrefSet(new_class, new_class->classSuperClasses, new_array1(this));
                                       /* make the receiver class the        */
                                       /* superclass for the instance behav  */
  OrefSet(new_class, new_class->instanceSuperClasses, new_array1(this));
                                       /* if there was enhancing methods     */
                                       /* specified                          */
  if (enhancing_class_methods != OREF_NULL && enhancing_class_methods != TheNilObject) {
                                       /* convert into a real method dict.  */
    enhancing_class_methods = new_class->methodDictionaryCreate(enhancing_class_methods, new_class);
                                       /* merge them into the class mdict    */
    new_class->methodDictionaryMerge(enhancing_class_methods, new_class->classMethodDictionary);
  }
                                       /* start out the class behaviour clean*/
  new_class->behaviour->setMethodDictionary(OREF_NULL);
  new_class->behaviour->setScopes(OREF_NULL);
                                       /* create the class behaviour from    */
                                       /* the class superclass list          */
  new_class->createClassBehaviour(new_class->behaviour);
                                       /* set the class behaviour created    */
                                       /* class to the meta class            */
  new_class->behaviour->setClass(meta_class);
                                       /* create the instance behaviour from */
                                       /* the instance superclass list       */
  new_class->instanceBehaviour->setMethodDictionary(OREF_NULL);
  new_class->instanceBehaviour->setScopes(OREF_NULL);
  new_class->createInstanceBehaviour(new_class->instanceBehaviour);
                                       /* set the instance behaviour created */
                                       /* class to the reciever class        */
  new_class->instanceBehaviour->setClass(new_class);
  new_class->somSuperClass(this);      /* see if any som stuff needs doing  */
                                       /* update the receiver class' subclass*/

  this->addSubClass(new_class);        /* list to reflect the new class     */
  /* if the class object has an UNINIT method defined, make sure we */
  /* add this to the table of classes to be processed. */
  if (new_class->hasUninitMethod()) {
      new_class->hasUninit();
  }
  new_class->sendMessage(OREF_INIT);   /* now drive any user INIT methods   */
                                       /* now the new class object should   */
  /* If the parent class has an uninit defined, the new child class must have one, too */
  if (this->uninitDefined() || this->parentUninitDefined()) {
     new_class->parentHasUninit();
  }
  discard_hold(new_class);             /* be safe                           */
  /* notify activity this object has an UNINIT that needs to be called
              when collecting the object */
  if (new_class->class_info & HAS_UNINIT)
    new_class->hasUninit();

  return new_class;                    /* return the new class              */
}

RexxInteger *RexxClass::importedRexx()
/******************************************************************************/
/* Function:  Check if a class has been imported                              */
/******************************************************************************/
{
   return this->imported() ? TheTrueObject : TheFalseObject;
}

RexxClass  *RexxClass::external(
    RexxString *externalString,        /* external class name               */
    RexxClass  *metaClass,             /* external meta class               */
    RexxTable  *enhancingClassMethods) /* additional CLass methods          */
/******************************************************************************/
/* Function:  Process an external class definition                            */
/******************************************************************************/
{
  long        words;                   /* words in the external string      */
  RexxString *classModel;              /* name of the class model           */
  RexxString *className;               /* name of the class                 */
                                       /* external class object             */
  RexxClass  *externalClass = OREF_NULL;
  RexxObject *classServer;             /* workplace server object           */

                                       /* See how many words are in string  */
  words = externalString->words()->value;
  if (words > 2) {                     /* More than 2 words in string?      */
                                       /* This is an Error.                 */
    report_exception(Error_Translation_class_external_bad_parameters);
  }
  else {
    if (0 == words)                    /* Nothing specific after EXTERNAL   */
      report_exception(Error_Translation_class_external_bad_class_name);

    if (1 == words) {                  /* Only one Specified?               */
      classModel = OREF_SOM;           /* default server is SOM             */
                                       /* class name is the first word      */
      className = externalString->word(IntegerOne);
    }
    else {
                                       /* Both ModelName  and               */
      classModel  = externalString->word(IntegerOne);
                                       /* class name specified.             */
      className = externalString->word(IntegerTwo);
    }
                                       /* Null string for classname?        */
    if (className->getLength() == 0 ) {
                                       /* Not allowed.                      */
      report_exception(Error_Translation_class_external_bad_class_name);
    }
                                       /* Force classMethods to NIL         */
    if (enhancingClassMethods == OREF_NULL) {
      enhancingClassMethods = (RexxTable *)TheNilObject;
    }

                                       /* is external Model SOM ?           */

    if (classModel->strCompare(CHAR_SOM)) {
                                       /* Yes, then import from localserver */
      externalClass = (RexxClass *)ProcessLocalServer->sendMessage(OREF_IMPORT, className, metaClass, enhancingClassMethods);
    }
                                       /* Not SOM, is it WPS?               */
    else if (classModel->strCompare(CHAR_WPS)) {
                                       /* YES, is WPS server installed?     */
       classServer = TheEnvironment->at(OREF_WPS);
       if (OREF_NULL != classServer) {
                                       /* Yes, import the class through WPS */
         externalClass = (RexxClass *)classServer->sendMessage(OREF_IMPORT, className, metaClass, enhancingClassMethods);
       }
       else
         report_exception1(Error_Execution_class_server_not_installed, classModel);
    }
    else if (classModel->strCompare(CHAR_DSOM)) {
                                       /* YES, is WPS server installed?     */
       classServer = ProcessLocalEnv->at(OREF_DSOM);
                                       /* Is DSOM initialized ?             */
       if (OREF_NULL == classServer) {
                                       /* Nope, then initialize it.         */
         save(className);              /* Keep className from being GCed    */
         classServer = ProcessLocalServer->sendMessage(new_cstring(CHAR_SOMD_INIT));
         discard_hold(className);      /* done with perm hold, on classname */
       }
                                       /* now load up the class through DSOM*/
       externalClass = (RexxClass *)classServer->sendMessage(OREF_IMPORT, className, metaClass, enhancingClassMethods);
    }
    else
      report_exception1(Error_Translation_class_external_bad_class_server, classModel);
  }
  return externalClass;                /* return the external class         */
}


RexxObject *RexxClass::importMethod()
/******************************************************************************/
/* Function: Import the SOM class (id) from to to the OREXX environment       */
/*           and the OREXX class (this) will be the "proxy" for the imported  */
/*           class.  So use a forwarder method somsend to forward all SOM     */
/*           messages to instances of this class to SOM.  We do this by       */
/*           creating an NMETHOD for somsend and use ourself as the scope     */
/*           (since we intoduce this method).  It is assumed that all our     */
/*           parent SOM classes have already been imported.                   */
/*           So we iterate across all the methods this SOM class knows about  */
/*           and see which methods are new to this class, by checking against */
/*           all method this class already knows about (FullInstanceMdict).   */
/*           Any that are new will be added to this class instanceMdict using */
/*           the somsend nmethod as the method for this message.              */
/*                                                                            */
/*  NOTE: When adding code to this method be very careful.  Since this method */
/*   frequently goes out to SOM, it release Kernel access at various points   */
/*   to avoid tying up the kernel while accessing SOM information.  This means*/
/*   that there are times within the method that it isn't safe to do kernel   */
/*   things.  Be sure you have the kernel semophore before accesing kernel.   */
/*                                                                            */
/*  Returned:  SOM class object address                                       */
/******************************************************************************/
{
#ifdef SOM
  SOMClass *classobj;
  long nmeths, i;
  RexxString   *methname;              /* name of the method                */
  RexxTable    *fmdict;                /* full method dictionary            */
  RexxTable    *imdict;                /* instance method dictionary        */
  somId mdesc;
  somId classId;
  somId methId;
  char *methn;
  somMToken methtok;
  somMethodPtr classMethod;
  RexxActivity *myActivity;            /* current activity                  */
  RexxArray    *arrayOfMethods;        /* Array of SOM methods              */
  RexxMethod   *newMethod;             /* new SOM method                    */
  long  remainingMethods;              /* Num of unused methods in Array    */
  long  requestSize;                   /* amount of SOM methods to request  */
  RexxTable    *som_methods;           /* methods gained from som           */

  somEnvironmentNew();                 /* make sure the SOM ennvironment    */
                                       /*  is initialized.                  */
                                       /* Before give up kernel access.     */
                                       /* get a fresh table for methods     */
  som_methods = (RexxTable *)save(new_table());
                                       /* Retrieve activity for this meth   */
                                       /*  curracti not reliable since we   */
                                       /*  go in and out of kernel.  Also   */
                                       /* want to avoid repeated calls to   */
                                       /* activity_find()                   */
  myActivity = CurrentActivity;

                                       /* We are done temporarily with      */
                                       /* the kernel objects, so release    */
                                       /* kernel access while we go out to  */
                                       /* SOM and get info.                 */
  ReleaseKernelAccess(myActivity);
                                       /* for all the methods of this SOM   */
                                       /*class                              */
  classId = somIdFromString(this->id->stringData);
                                       /* lookup and get access to the SOM  */
                                       /* class we want to import           */
  classobj = SOMClassMgrObject->somFindClass(classId, 0, 0);
  if (classobj == NULL) {              /* was class object actually found?  */
                                       /* No, its an error.                 */
    RequestKernelAccess(myActivity);   /* 1st regain access to kernel.      */
    discard(som_methods);              /* release somMethods table          */
                                       /* now report the error.             */
    report_exception1(Error_Execution_noSomclass, this->id);
  }
  SOMFree(classId);                    /* Free the id for the class.        */
                                       /* get the number of methods the     */
                                       /*  SOM class has.                   */
  nmeths = classobj->somGetNumMethods();
                                       /* get the OREXX class current       */
  remainingMethods = 0;                /* no methods at this time.          */
  arrayOfMethods = OREF_NULL;
  for (i = 0; i < nmeths; i++) {
                                       /* get the method id for this method */
    methId = classobj->somGetNthMethodInfo(i,&mdesc);
    methn = somStringFromId(methId);   /* convert somId to a string.        */
                                       /* get the token for this method     */
    methtok = classobj->somGetMethodToken(methId);
                                       /* Resolve this method and get the   */
                                       /* methodPtr this message would call */
    classMethod = classobj->somDefinedMethod(methtok);

                                       /* enter kernel code again, and      */
                                       /* lookup this method in OREXX class */
    RequestKernelAccess(myActivity);   /* Gain exclusive access to kernel   */
                                       /* get method name as UpperCase name */
    methname = new_cstring(methn)->upper();
                                       /* Is this method defined/overidden  */
    if (classMethod ||                 /*  overridden by this SOM class     */
                                       /* Or, not  current instance method  */
       (this->instanceBehaviour->methodLookup(methname) == TheNilObject)) {

                                       /* Add this method to the            */
                                       /*   OREXX class instanceMdict.      */
      if (!remainingMethods) {         /* any methods available to use?     */
                                       /* no, need to get another bunch     */
        if (arrayOfMethods)            /* if we have an arrayofMethods      */
          discard(arrayOfMethods);     /* remove it from save table.        */
                                       /*  assume half remaining methods    */
                                       /*  to process, will be new at this  */
                                       /*  level.                           */
        requestSize = ((nmeths - i) / 2) > 10 ? (nmeths -i) / 2: 10;
                                       /* Get new array of SOM Methods      */
        arrayOfMethods = TheMethodClass->newArrayOfSOMMethods(this, requestSize);
        remainingMethods = requestSize;/* all newly gotten methods remain   */
      }
                                       /* Allocate new method from Array    */
      newMethod = (RexxMethod *)arrayOfMethods->get(remainingMethods--);
                                       /* add this method w/ methName to    */
                                       /* table of method to be added for   */
                                       /* this classes instance MDICT.      */
      som_methods->add(newMethod, methname);
    }
    ReleaseKernelAccess(myActivity);   /* release kernel access. again.     */


  }
  RequestKernelAccess(myActivity);     /* get access to kernel once again   */
  if (arrayOfMethods)                  /* if allocated array of methods     */
    discard(arrayOfMethods);           /* remove it from save table.        */

  this->setImported();                 /* mark class as being imported.     */
  this->defineMethods(som_methods);    /* Rebuild behaviour                 */
                                       /* remove som_methods table from     */
  discard(som_methods);                /*save table                         */
                                       /* Remember the SOM Class for proxy  */
  OrefSet(this, this->somClass, new_pointer(classobj));
  return this->somClass;               /* return the SOM class              */
#else
  return TheNullPointer;               /* return nullPointer object         */
#endif
}

#ifdef SOM
extern "C" {
void SOMLINK oryx_class_dispatch2 (SOMObject *somSelf, IN SOMClass *classobj,
                    OUT somToken *resultp,
                    IN somId msgid, IN va_list ap);
void SOMLINK oryx_dispatch2 (SOMObject *somSelf, OUT void **resultp,
                    OUT somId msgid, va_list ap);
}
void *SOMLINK  oryx_dispatch_a (SOMObject *somSelf, INOUT somId methodId,
                        INOUT somId descriptor, va_list ap);
float8 SOMLINK oryx_dispatch_d (SOMObject *somSelf, INOUT somId methodId,
                        INOUT somId descriptor, va_list ap);
integer4 SOMLINK oryx_dispatch_l (SOMObject *somSelf, INOUT somId methodId,
                          INOUT somId descriptor, va_list ap);
void SOMLINK oryx_dispatch_v (SOMObject *somSelf, INOUT somId methodId,
                      INOUT somId descriptor, va_list ap);
#endif

RexxObject *RexxClass::exportMethod(
    RexxString *cid,                   /* class ID                          */
    RexxString *sId,                   /* super class ID                    */
    long numMeths,                     /* number of static methods          */
    RexxClass  *metaClass)             /* SOM meta class                    */
/******************************************************************************/
/* Function:  Export a SOM method                                             */
/******************************************************************************/
{
#ifdef SOM
  SOMClass *classobj, *pclsobj, *mclsobj;
  somId         classId;
  RexxInteger  *mclass;
  RexxActivity *myActivity;
#ifndef SOMV3
  somId dispatcha = somIdFromString("somDispatchA");
  somId dispatchd = somIdFromString("somDispatchD");
  somId dispatchl = somIdFromString("somDispatchL");
  somId dispatchv = somIdFromString("somDispatchV");
#endif
  somId dispatch = somIdFromString("somDispatch");
  somId classDispatch = somIdFromString("somClassDispatch");
  char *id, *superId;

  id = REQUIRED_STRING(cid, ARG_ONE)->stringData;
  superId = REQUIRED_STRING(sId, ARG_TWO)->stringData;

  myActivity = CurrentActivity;
  /****************************************/
  /** Let Kernel, don't do kernel things **/
  /****************************************/
  ReleaseKernelAccess(myActivity);
                                       /* See if the class object already   */
                                       /*exists                             */
                                       /* for now use somFindClass, should  */
                                       /* revert to somClassFromID          */
  classId = somIdFromString(id);
  classobj = SOMClassMgrObject->somClassFromId(classId);
  SOMFree(classId);

  if (classobj == (SOMClass *)NULL) {
                                       /* Get the SOM parent class object   */
    classId = somIdFromString(superId);
    pclsobj = SOMClassMgrObject->somClassFromId(classId);
    SOMFree(classId);
    if (pclsobj == (SOMClass *) NULL) {
                                       /* Need kernel for messsage send.    */
      RequestKernelAccess(myActivity);
      report_exception1(Error_Execution_noSomclass, sId);
    }

                                       /* create the SOM class              */
    if (metaClass == TheNilObject) {
                                       /* use parent's metaclass            */
      mclsobj = (SOMClass *)SOM_GetClass(pclsobj);
    }
    else {
      mclass = metaClass->somClass;
      mclsobj = (SOMClass *)mclass->value;
    }
    classobj = (SOMClass *)mclsobj->somNew();

                                       /* initialize the class              */
                                       /* size of the data of the new class */
                                       /* is 0, since we keep the         */
                                       /*instance data on the Oryx side   */
    classobj->somInitClass(id,pclsobj, 0,
        numMeths,                      /* classMaxNoMethods */
        1,                             /* classMajorVersion */
        1);                            /* classMinorVersion */

    /* override the somDispatch pointers */
#ifndef SOMV3
    classobj->somOverrideSMethod(dispatcha,(somMethodProc *)oryx_dispatch_a);
    classobj->somOverrideSMethod(dispatchd,(somMethodProc *)oryx_dispatch_d);
    classobj->somOverrideSMethod(dispatchl,(somMethodProc *)oryx_dispatch_l);
    classobj->somOverrideSMethod(dispatchv,(somMethodProc *)oryx_dispatch_v);
#endif
    classobj->somOverrideSMethod(dispatch,(somMethodProc *)oryx_dispatch2);
    classobj->somOverrideSMethod(classDispatch,(somMethodProc *)oryx_class_dispatch2);

    /* the class is now ready for use */
    classobj->somClassReady();
  } /*if*/
  else {
#ifndef SOMV3
    classobj->somOverrideSMethod(dispatcha,(somMethodProc *)oryx_dispatch_a);
    classobj->somOverrideSMethod(dispatchd,(somMethodProc *)oryx_dispatch_d);
    classobj->somOverrideSMethod(dispatchl,(somMethodProc *)oryx_dispatch_l);
    classobj->somOverrideSMethod(dispatchv,(somMethodProc *)oryx_dispatch_v);
#endif
    classobj->somOverrideSMethod(dispatch,(somMethodProc *)oryx_dispatch2);
    classobj->somOverrideSMethod(classDispatch,(somMethodProc *)oryx_class_dispatch2);
  }

  RequestKernelAccess(myActivity);
  return (RexxObject *)new_integer((long)classobj);
#else
  return TheNilObject;
#endif
}

RexxObject *RexxClass::somDefine(
    RexxString  *methn,                /* method name                       */
    RexxInteger *clsobj)               /* SOM class object                  */
/******************************************************************************/
/* Function:  Define a SOM class object                                       */
/******************************************************************************/
{
#ifdef SOM
  somId msgid;
  int rc;
  somMethodData md;
  somMethodProc *redispatchstub;
  SOMClass *classObj;
  char *methName;

  methName = REQUIRED_STRING(methn, ARG_ONE)->stringData;
  classObj = (SOMClass *)clsobj->value;
  msgid = somIdFromString(methName);

  rc = classObj->somGetMethodData(msgid,&md);
  if (rc != 0) {
    redispatchstub = classObj->somGetRdStub(msgid);
    classObj->somOverrideSMethod(msgid,redispatchstub);
  } /*if*/
#endif
  return TheNilObject;
}


long RexxClass::somInterfaces()
/******************************************************************************/
/* Function: Return number of methods this class introduces that has          */
/*  an interface string associated with it.                                   */
/******************************************************************************/
{
  long count;
  long iterator;

  count = 0;                           /* initially no method interfaces    */
                                       /* for all methods introduced by     */
                                       /* this class.                       */
  for (iterator = this->instanceMethodDictionary->first();
       this->instanceMethodDictionary->index(iterator) != OREF_NULL;
       iterator = this->instanceMethodDictionary->next(iterator)) {
                                       /* If this method has an             */
                                       /* interface definition              */
    if (((RexxMethod *)this->instanceMethodDictionary->value(iterator))->getInterface() != OREF_NULL ) {
      count++;                         /* bump the count.                   */
    }
  }
                                       /* return total method that have     */
  return count;                        /* interface definitions.            */
}


RexxSOMProxy *RexxClass::newOpart(
    RexxInteger  *somObj)
/******************************************************************************/
/*                                                                            */
/******************************************************************************/
{
  RexxSOMProxy * newObject;            /* newly created object              */
                                       /* create new object REXX object     */
  newObject = new RexxSOMProxy;
                                       /* Set the behaviour                 */
  BehaviourSet(newObject, this->instanceBehaviour);
                                       /* does object have an UNINT method  */
  if (this->uninitDefined())  {
                                       /* Make sure everyone is notified.   */
     newObject->hasUninit();
  }
                                       /* initalize the Proxy portion       */
  newObject->initProxy(somObj);
  return newObject;                    /* return new object.                */
}

void RexxClass::setMetaClass(
    RexxClass *metaClass )             /* new meta class to add             */
/******************************************************************************/
/* Function:  Set a metaclass for a class                                     */
/******************************************************************************/
{
    OrefSet(this, this->metaClass, new_array1(TheClassClass));
    this->metaClass->addFirst(metaClass);
                                       /* the metaclass mdict list           */
    OrefSet(this, this->metaClassMethodDictionary, new_array1(TheClassClass->instanceMethodDictionary->copy()));
    this->metaClassMethodDictionary->addFirst(metaClass->instanceMethodDictionary);
                                       /* and the metaclass scopes list      */
    OrefSet(this, this->metaClassScopes, (RexxObjectTable *)TheClassClass->behaviour->scopes->copy());
                                       /* add the scope list for this scope  */
    this->metaClassScopes->add(metaClass, TheNilObject);
    this->metaClassScopes->add(this->metaClassScopes->allAt(TheNilObject), metaClass);
}


/**
 * Test if the target class is a "compatible" with the argument
 * class.  To be compatible, the target class must either A)
 * be the same class, B) be a direct subclass of the argument
 * class, or C) inherit the argument class as a mixin.  This
 * rule gets applied recursively down the hierarchy.
 *
 * @param other  The comparison class.
 *
 * @return True if the two classes are compatible, false otherwise.
 */
bool RexxClass::isCompatibleWith(RexxClass *other)
{
    // if asking for a match here, this is true
    if (other == this)
    {
        return true;
    }

    // if this is .object, there are no superclasses.  Otherwise, ask each of the superclasses
    // the same question.
    if (instanceSuperClasses != OREF_NULL)
    {
        for (size_t i = 1; i <= instanceSuperClasses->size(); i++)
        {
            if (((RexxClass *)instanceSuperClasses->get(i))->isCompatibleWith(other))
            {
                return true;
            }
        }
    }
    return false;
}


/**
 * A stub to test compatibility of two classes.
 *
 * @param other  The class for the superclass test.
 *
 * @return True if the class is a subclass of the argument class (or IS
 *         the argument class).
 */
RexxObject *RexxClass::isSubclassOf(RexxClass *other)
{
    required_arg(other, ONE);            // must have the other argument
    return isCompatibleWith(other) ? TheTrueObject : TheFalseObject;
}

RexxString *RexxClass::defaultNameRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->defaultName();          /* forward to the virtual function   */
}



void  *RexxClass::operator new(size_t size,
    long size1,                        /* additional size                   */
    RexxBehaviour *class_behaviour,    /* new class behaviour               */
    RexxBehaviour *instanceBehaviour)  /* instance behaviour info           */
/*****************************************************************************/
/* Function:  Create a new primitive class                                   */
/*            for the subclassable classes the rest of the class information */
/*            will be filled in when oksetup.c is run                        */
/*****************************************************************************/
{
  RexxClass  *new_class;               /* newly create class                */

  if (size1 == 0)                      /* want the default?                 */
                                       /* Get new class object              */
    new_class = (RexxClass *)new_object(size);
  else
                                       /* use the specified size            */
    new_class = (RexxClass *)new_object(size1);
  ClearObject(new_class);              /* clear out the state data          */
                                       /* set the class specific behaviour  */
  BehaviourSet(new_class, class_behaviour);
                                       /* set the class into the behaviour  */
  new_class->behaviour->setClass(new_class);
                                       /* set the instance behaviour        */
  OrefSet(new_class, new_class->instanceBehaviour, instanceBehaviour);
                                       /* and the class of this behaviour   */
  new_class->instanceBehaviour->setClass(new_class);
                                       /* tell the mobile support to just   */
  new_class->header |= MakeProxyObject;/* make a proxy for this class       */
  return (void *)new_class;            /* should be ready                   */
}

RexxClass  *RexxClass::newRexx(RexxObject **args, size_t argCount)
/*****************************************************************************/
/* Function:  Create a new class for a rexx class                            */
/*            A copy of this class object is made                            */
/*            This class' behaviour, class_mdict, metaclass, and class_info  */
/*             are used in the new class. All the rest of the object state   */
/*             data is updated to reflect a new class object                 */
/*****************************************************************************/
{
  RexxClass  * new_class;              /* holder for the new class          */
  RexxString * id;                     /* id parameter                      */
  if (argCount == 0)                   /* make sure an arg   was passed in  */
                                       /* if not report an error            */
    report_exception1(Error_Incorrect_method_minarg, IntegerOne);
  id = (RexxString *)args[0];          /* get the id parameter              */
  id = REQUIRED_STRING(id, ARG_ONE);   /* and that it can be a string       */
                                       /* get a copy of this class object   */
  new_class = (RexxClass *)memoryObject.clone(this);
                                       /* update cloned hashvalue           */
  new_class->hashvalue = HASHOREF(new_class);
  save(new_class);                     /* better protect this               */
                                       /* make this into an instance of the */
                                       /* meta class                        */
  OrefSet(new_class, new_class->behaviour, (RexxBehaviour *)new_class->instanceBehaviour->copy());
                                       /* don't give access to this class'   */
                                       /* class mdict                        */
  OrefSet(new_class, new_class->classMethodDictionary, new_table());
                                       /* make this class the superclass     */
  OrefSet(new_class, new_class->classSuperClasses, new_array1(this));
  new_class->behaviour->setClass(this);/* and set the behaviour class       */
                                       /* if this is a primitive class then  */
                                       /* there isn't any metaclass info     */
  if (this->isPrimitive()) {           /* set up yet                        */
                                       /* set up the new metaclass list      */
    OrefSet(new_class, new_class->metaClass, new_array1(TheClassClass));
                                       /* the metaclass mdict list           */
    OrefSet(new_class, new_class->metaClassMethodDictionary, new_array1(TheClassClass->instanceMethodDictionary->copy()));
                                       /* and the metaclass scopes list      */
    OrefSet(new_class, new_class->metaClassScopes, (RexxObjectTable *)TheClassClass->behaviour->scopes->copy());
  }
  else {
                                       /* add this class to the new class    */
                                       /* metaclass list                     */
    OrefSet(new_class, new_class->metaClass, (RexxArray *)new_class->metaClass->copy());
    new_class->metaClass->addFirst(this);
                                       /* the metaclass mdict list           */
    OrefSet(new_class, new_class->metaClassMethodDictionary, (RexxArray *)new_class->metaClassMethodDictionary->copy());
    new_class->metaClassMethodDictionary->addFirst(this->instanceMethodDictionary);
                                       /* and the metaclass scopes list      */
                                       /* this is done by adding all the     */
                                       /* scope information of the new class */
    OrefSet(new_class, new_class->metaClassScopes, (RexxObjectTable *)new_class->metaClassScopes->copy());
                                       /* and update the scopes to include   */
                                       /* the metaclass scopes               */
    new_class->metaClassScopes->add(this, TheNilObject);
    new_class->metaClassScopes->add(this->behaviour->scopes->allAt(TheNilObject), this);
  }
                                       /* set up the instance behaviour with */
                                       /*  object's instance methods         */
  OrefSet(new_class, new_class->instanceBehaviour, (RexxBehaviour *)TheObjectClass->instanceBehaviour->copy());
                                       /* don't give access to this class'   */
                                       /*  instance mdict                    */
  OrefSet(new_class, new_class->instanceMethodDictionary, new_table());
                                       /* make the instance_superclass list  */
                                       /* with OBJECT in it                  */
  OrefSet(new_class, new_class->instanceSuperClasses, new_array1(TheObjectClass));
                                       /* and set the behaviour class        */
  new_class->instanceBehaviour->setClass(TheObjectClass);
                                       /* and the instance behaviour scopes  */
  new_class->instanceBehaviour->setScopes(new_object_table());
                                       /* set the scoping info               */
  new_class->instanceBehaviour->addScope(TheObjectClass);
                                       /* don't give access to this class'   */
                                       /*  ovd's                             */
  OrefSet(new_class, new_class->objectVariables, OREF_NULL);
                                       /* set the new class as it's own      */
                                       /* baseclass                          */
  OrefSet(new_class, new_class->baseClass, new_class);
                                       /* set the som class to .nil          */
  OrefSet(new_class, new_class->somClass, (RexxInteger *)TheNilObject);
                                       /* set the id into the class object   */
  OrefSet(new_class, new_class->id, id);
                                       /* clear the info area except for     */
                                       /* uninit and imported                */
  new_class->class_info &= (HAS_UNINIT & IMPORTED);
  /* if the class object has an UNINIT method defined, make sure we */
  /* add this to the table of classes to be processed. */
  if (new_class->hasUninitMethod()) {
      new_class->hasUninit();
  }
  discard_hold(new_class);             /* remove the protection             */
                                       /* go do any inits                   */
  new_class->sendMessage(OREF_INIT, args + 1, argCount - 1);
  return new_class;                    /* return the new class              */
}

void class_create (void)
/******************************************************************************/
/* Function:  Create the initial class object                                 */
/******************************************************************************/
{
                                       /* create a class object             */
  TheClassClass = (RexxClass *)new_object(sizeof(RexxClass));
                                       /* set the instance behaviour         */
  BehaviourSet(TheClassClass, TheClassClassBehaviour);
                                       /* set the instance behaviour         */
  OrefSet(TheClassClass, TheClassClass->instanceBehaviour, TheClassBehaviour);
                                       /* tell the mobile support to just    */
                                       /* make a proxy for this class        */
  TheClassClass->header |= MakeProxyObject;
  new (TheClassClass) RexxClass;
}

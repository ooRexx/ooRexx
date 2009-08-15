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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Class Class                                                      */
/*                                                                            */
/******************************************************************************/

#include <stdarg.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ListClass.hpp"
#include "TableClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ClassClass.hpp"
#include "MethodClass.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "WeakReferenceClass.hpp"


// singleton class instance
RexxClass *RexxClass::classInstance = OREF_NULL;


void RexxClass::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->objectVariables);
    memory_mark(this->id);
    memory_mark(this->classMethodDictionary);
    memory_mark(this->instanceBehaviour);
    memory_mark(this->instanceMethodDictionary);
    memory_mark(this->baseClass);
    memory_mark(this->metaClass);
    memory_mark(this->metaClassMethodDictionary);
    memory_mark(this->metaClassScopes);
    memory_mark(this->classSuperClasses);
    memory_mark(this->instanceSuperClasses);
    memory_mark(this->subClasses);
}

void RexxClass::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->objectVariables);
    memory_mark_general(this->id);
    memory_mark_general(this->classMethodDictionary);
    memory_mark_general(this->instanceBehaviour);
    memory_mark_general(this->instanceMethodDictionary);
    memory_mark_general(this->baseClass);
    memory_mark_general(this->metaClass);
    memory_mark_general(this->metaClassMethodDictionary);
    memory_mark_general(this->metaClassScopes);
    memory_mark_general(this->classSuperClasses);
    memory_mark_general(this->instanceSuperClasses);
    memory_mark_general(this->subClasses);
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
   return new_proxy(this->id->getStringData());
}


/**
 * Hash a class object.  Because behaviors don't always get set
 * up properly with this, we'll always use the primitive one for
 * class objects.
 *
 * @return A "hashed hash" that can be used by the map collections.
 */
HashCode RexxClass::hash()
{
    // always, always, always return the hash value, which will be the
    // hash value of our id string.  This is important, since we need to
    // have a hash value that will be the same before and after the image save
    return getHashValue();
}


/**
 * Get the primitive hash value of this String object.
 *
 * @return The calculated string hash for the string.
 */
HashCode RexxClass::getHashValue()
{
    // always, always, always return the hash value, which will be the
    // hash value of our id string.  This is important, since we need to
    // have a hash value that will be the same before and after the image save
    return id->getHashValue();
}


RexxObject *RexxClass::strictEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two classes                                             */
/******************************************************************************/
{
    return this->equal(other);         /* this is direct object equality    */
}

bool RexxClass::isEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two class objects as a strict compare (==)              */
/******************************************************************************/
{
    /* If a non-copied (Primitive)       */
    /*behaviour Then we can directly     */
    /*call primitive method              */
    if (this->behaviour->isPrimitive())
    {
        /* can compare at primitive level    */
        return this->equal(other) == TheTrueObject;
    }
    else
    {
        ProtectedObject r;
        /* other wise giveuser version a     */
        /*chance                             */
        this->sendMessage(OREF_STRICT_EQUAL, other, r);
        return((RexxObject *)r)->truthValue(Error_Logical_value_method);
    }
}

RexxObject *RexxClass::equal(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two classes                                             */
/******************************************************************************/
{
    requiredArgument(other, ARG_ONE);            /* must have the other argument      */
                                         /* this is direct object equality    */

                                         /* comparing string/int/numstr to    */
                                         /*  string/int/numstr?               */
    if ((this == TheStringClass || this == TheIntegerClass || this == TheNumberStringClass) &&
        (other == (RexxObject *)TheStringClass || other == (RexxObject *)TheIntegerClass || other == (RexxObject *)TheNumberStringClass))
    {
        return TheTrueObject;              /* YES, then equal....               */
    }
    else                                 /* other wise, do a direct compare   */
    {
        return((this == other) ? TheTrueObject: TheFalseObject);
    }
}

RexxObject *RexxClass::notEqual(
    RexxObject *other)                 /* other comparison object           */
/******************************************************************************/
/* Function:  Compare two classes                                             */
/******************************************************************************/
{
    requiredArgument(other, ARG_ONE);            /* must have the other argument      */
                                         /* this is direct object equality    */

                                         /* comparing string/int/numstr to    */
                                         /*  string/int/numstr?               */
    if ((this == TheStringClass || this == TheIntegerClass || this == TheNumberStringClass) &&
        (other == (RexxObject *)TheStringClass || other == (RexxObject *)TheIntegerClass || other == (RexxObject *)TheNumberStringClass))
    {
        return TheFalseObject;             /* YES, then equal....               */
    }
    else                                 /* other wise, do a direct compare   */
    {
        return((this != other) ? TheTrueObject: TheFalseObject);
    }
}

RexxInteger *RexxClass::queryMixinClass()
/*****************************************************************************/
/* Function: To check if class_info MIXIN has been set                       */
/*****************************************************************************/
{
                                       /* return true/false indicator       */
    return this->isMixinClass() ? TheTrueObject : TheFalseObject;
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
    this->classFlags |= REXX_DEFINED;    /* flag the class                    */
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
    if (this->isPrimitiveClass())        /* primitive class?                  */
    {
        return TheClassClass;              /* this is always .class             */
    }
    else                                 /* return first member of the list   */
    {
        return(RexxClass *)this->metaClass->get(1);
    }
}

void  RexxClass::setInstanceBehaviour(
    RexxBehaviour *b)                   /* new instance behaviour            */
/*****************************************************************************/
/* Function:  Give a class a new instance behaviour                          */
/*****************************************************************************/
{
    OrefSet(this, this->instanceBehaviour, b);
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
    // remove any gc classes from the list now, and return the array
    return subClasses->weakReferenceArray();
}

void RexxClass::addSubClass(RexxClass *subClass)
/*****************************************************************************/
/* Function:  Add a subclass to a class                                      */
/*****************************************************************************/
{
    // wrap a weak reference around the subclass
    WeakReference *ref = new WeakReference(subClass);
    // add this to the front of the subclass list
    subClasses->addFirst((RexxObject *)ref);
}

void RexxClass::defmeths(
    RexxTable *newMethods)             /* methods to add                    */
/*****************************************************************************/
/* Function:  Add a table of methods to a primitive class behaviour          */
/*****************************************************************************/
{
    /* loop through the list of methods  */
    for (HashLink i = newMethods->first(); newMethods->available(i); i = newMethods->next(i))
    {
        /* get the method name               */
        RexxString *method_name = (RexxString *)newMethods->index(i);
        /* add this method to the classes    */
        /* class behaviour                   */

        // if this is the Nil object, that's an override.  Make it OREF_NULL.
        RexxObject *_method = (RexxMethod *)newMethods->value(i);
        if (_method == TheNilObject)
        {
            _method = OREF_NULL;
        }

        this->behaviour->define(method_name, OREF_NULL);
    }
}

RexxString *RexxClass::defaultName()
/******************************************************************************/
/* Function:  retrieve a classes default name value                           */
/******************************************************************************/
{
    RexxString *defaultname = this->id;  /* use the id directly               */
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
    /* get the method dictionary         */
    RexxTable *methodTable = this->instanceBehaviour->getMethodDictionary();
    if (methodTable == OREF_NULL)        /* no methods defined yet?           */
    {
        return new_table();                /* create a new method table         */
    }
    else
    {
        /* just copy the method dictionary   */
        return(RexxTable *)methodTable->copy();
    }
}

RexxTable *RexxClass::getBehaviourDictionary()
/*****************************************************************************/
/* Function:   Return the class behaviour's method dictionary                */
/*****************************************************************************/
{
    /* get the method dictionary         */
    RexxTable *methodTable = this->behaviour->getMethodDictionary();
    if (methodTable == OREF_NULL)        /* no methods defined yet?           */
    {
        return new_table();                /* create a new method table         */
    }
    else
    {
        /* just copy the method dictionary   */
        return(RexxTable *)methodTable->copy();
    }
}


/**
 * Initialize a base Rexx class.
 *
 * @param restricted Whether we should turn the RexxRestricted flag on at this time.
 *                   Some classes get additional customization after initial
 *                   creation, so we delay setting this attribute until the
 *                   class is fully constructed.
 */
void RexxClass::subClassable(bool restricted)
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
    if (this != TheObjectClass)          /* if this isn't the OBJECT class    */
    {
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
    {
        this->behaviour->addScope(TheObjectClass);
    }
    /* if this is OBJECT - merge the      */
    /* object instance methods with the   */
    /* object class methods               */
    else
    {
        this->behaviour->merge(TheObjectBehaviour);
        /* and put them into the class mdict  */
        /* so all the classes will inherit    */
        OrefSet(this, this->classMethodDictionary, this->getBehaviourDictionary());
    }
    /* if this isn't CLASS put CLASS in   */
    /* next                               */
    if (this != TheClassClass)
    {
        this->behaviour->addScope(TheClassClass);
    }
    this->behaviour->addScope(this);     /* put self into the scope table     */
                                         /* That finishes the class behaviour  */
                                         /* initialization.                    */
                                         /* Now fill in the state data         */

    if (TheObjectClass != this )
    {
        /* set up the new metaclass list      */
        OrefSet(this, this->metaClass, new_array(TheClassClass));
        /* the metaclass mdict list           */
        OrefSet(this, this->metaClassMethodDictionary, new_array(TheClassClass->instanceMethodDictionary->copy()));
        /* and the metaclass scopes list      */
        OrefSet(this, this->metaClassScopes, (RexxIdentityTable *)TheClassClass->behaviour->getScopes()->copy());
    }

    /* The Baseclass for non-mixin classes*/
    /* is self                            */
    OrefSet(this, this->baseClass, this);
    /* The class superclasses list for    */
    /* OBJECT is an empty list.           */
    OrefSet(this, this->classSuperClasses, new_array((size_t)0));
    /* as is the instance superclasses    */
    /* list.                              */
    OrefSet(this, this->instanceSuperClasses, new_array((size_t)0));
    // create the subclasses list
    OrefSet(this, this->subClasses, new_list());
    if (this != TheObjectClass)          /* not .object?                      */
    {
        /* add object to the list             */
        this->classSuperClasses->addLast(TheObjectClass);
        /* The instance superclasses for all  */
        /* except OBJECT is OBJECT            */
        this->instanceSuperClasses->addLast(TheObjectClass);
        /* and for OBJECT we need to add all  */
        /* the other classes                  */
        /* except integer and numberstring    */
        if (this != TheIntegerClass && this != TheNumberStringClass)
        {
            TheObjectClass->addSubClass(this);
        }
    }
    /* and point the instance behaviour   */
    /* back to this class                 */
    this->instanceBehaviour->setOwningClass(this);
    /* and the class behaviour to CLASS   */
    this->behaviour->setOwningClass(TheClassClass);
    /* these are primitive classes       */
    this->classFlags |= PRIMITIVE_CLASS;

    if (this == TheClassClass)           /* mark CLASS as a meta class        */
    {
        this->setMetaClass();
    }
}


/**
 * Initialize a base Rexx class that inherits from a primitive
 * class other than Object.
 *
 * @param superClass The immediate superclass of the created
 *                   class.
 * @param restricted Whether we should turn the RexxRestricted flag on at this time.
 *                   Some classes get additional customization after initial
 *                   creation, so we delay setting this attribute until the
 *                   class is fully constructed.
 */
void RexxClass::subClassable(RexxClass *superClass, bool restricted)
{
    // get a copy of the class instance behaviour mdict before the merge
    // with OBJECT.  This unmerged mdict is kept in this class's
    // class_instance_mdict field.
    OrefSet(this, this->instanceMethodDictionary, this->getInstanceBehaviourDictionary());

    // set up the superclass/subclass relationships
    OrefSet(this, this->classSuperClasses, new_array(superClass));
    OrefSet(this, this->instanceSuperClasses, new_array(superClass));
    // create the subclasses list
    OrefSet(this, this->subClasses, new_list());
    // and add this as a subclass to our superclass
    superClass->addSubClass(this);

    // create the merged method dictionary for the instancebehavior
    // and update all of the scopes.
    mergeSuperClassScopes(this->instanceBehaviour);

    /* add self to the scope table        */
    this->instanceBehaviour->addScope(this);

    // get a copy of the class behaviour mdict before the merge with the
    // CLASS instance behaviour. This unmerged mdict is kept in the
    // class_mdict field
    OrefSet(this, this->classMethodDictionary, this->getBehaviourDictionary());
    // The merge of the mdict's is order specific. By processing OBJECT
    // first then CLASS and then the rest of the subclassable classes
    // the mdict's will be set up correctly.In this way merging the CLASS
    // behaviour will only be the CLASS instance methods when OBJECT is
    // processed, but will be CLASS's and OBJECT's after CLASS is
    // processed                          */
    this->behaviour->merge(TheClassBehaviour);
    // now add the scope levels to this class behaviour
    this->behaviour->addScope(TheObjectClass);
    // add the class scope levels
    this->behaviour->addScope(TheClassClass);
    // and finally the new class.
    this->behaviour->addScope(this);

    // now fill in some state data for the class object.
    // set up the new metaclass list
    OrefSet(this, this->metaClass, new_array(TheClassClass));
    // the metaclass mdict list
    OrefSet(this, this->metaClassMethodDictionary, new_array(TheClassClass->instanceMethodDictionary->copy()));
    // and the metaclass scopes list
    OrefSet(this, this->metaClassScopes, (RexxIdentityTable *)TheClassClass->behaviour->getScopes()->copy());

    // The Baseclass for non-mixin classes is self
    OrefSet(this, this->baseClass, this);
    // and point the instance behaviour back to this class
    this->instanceBehaviour->setOwningClass(this);
    // and the class behaviour to CLASS
    this->behaviour->setOwningClass(TheClassClass);
    // these are primitive classes
    this->classFlags |= PRIMITIVE_CLASS;
}


RexxObject *RexxClass::defineMethod(
    RexxString * method_name,          /*define method name                 */
    RexxMethod *method_object)         /* returned method object            */
/*****************************************************************************/
/* Function:  Define an instance method on this class object                 */
/*****************************************************************************/
{
    /* check if this is a rexx class     */
    if ( this->isRexxDefined())
    {
        /* report as a nomethod condition    */
        reportNomethod(lastMessageName(), this);
    }
    /* make sure there is at least one   */
    /* parameter                         */
    method_name = stringArgument(method_name, ARG_ONE)->upper();
    if ( OREF_NULL == method_object)     /* 2nd arg omitted?                  */
    {
        /* Yes, remove all message with this */
        /* name from our instanceMdict       */
        /*                 (method lookup)   */
        /* done by defining the method       */
        /* to be .nil at this class level, so*/
        /* when message lookup is attempted  */
        /* we get .nil, telling us not found */
        method_object = (RexxMethod *)TheNilObject;
    }
    /* not a method type already?        */
    /* and not TheNilObject              */
    else if (TheNilObject != method_object && !isOfClass(Method, method_object))
    {
        /* make one from a string            */
        method_object = RexxMethod::newMethodObject(method_name, method_object, IntegerTwo, OREF_NULL);
    }
    if (TheNilObject != method_object)   /* if the method is not TheNilObject */
    {
        /* set the scope of the method to self*/
        method_object = method_object->newScope(this);
        /* Installing UNINIT?                */
        if (method_name->strCompare(CHAR_UNINIT))
        {
            this->setHasUninitDefined();     /* and turn on uninit if so          */
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
    return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::defineMethods(
    RexxTable * newMethods)            /* new table of methods to define    */
/*****************************************************************************/
/* Function:  Define instance methods on this class object                   */
/*****************************************************************************/
{
    RexxString * index;                  /* method name                       */
                                         /* loop thru the methods setting the */
                                         /* method scopes to SELF and then    */
                                         /* adding them to SELF's instance    */
                                         /* mdict                             */
    for (HashLink i = newMethods->first(); (index = (RexxString *)newMethods->index(i)) != OREF_NULL; i = newMethods->next(i))
    {
        /* get the method                    */
        RexxMethod *newMethod = (RexxMethod *)newMethods->value(i);
        if (isOfClass(Method, newMethod))      /* if this is a method object        */
        {
            newMethod->setScope(this);        /* change the scope                  */
        }
        /* add method to the instance mdict   */
        this->instanceMethodDictionary->stringPut(newMethod, index);
        /* Installing UNINIT?                */
        if (index->strCompare(CHAR_UNINIT))
        {
            this->setHasUninitDefined();     /* and turn on uninit if so          */
        }
    }
    /* create the instance behaviour from */
    /* the instance superclass list       */
    this->instanceBehaviour->setMethodDictionary(OREF_NULL);
    this->instanceBehaviour->setScopes(OREF_NULL);
    this->createInstanceBehaviour(this->instanceBehaviour);

    return OREF_NULL;                    /* returns nothing                   */
}

RexxObject *RexxClass::deleteMethod(
    RexxString  *method_name)          /* deleted method name               */
/*****************************************************************************/
/* Function:  Delete an instance method on this class object                 */
/*****************************************************************************/
{
    if (this->isRexxDefined())           /* check if this is a rexx class     */
    {
        /* report as a nomethod condition    */
        reportNomethod(lastMessageName(), this);
    }
    /* and that it can be a string        */
    method_name = stringArgument(method_name, ARG_ONE)->upper();
    /* make a copy of the instance        */
    /* behaviour so any previous objects  */
    /* aren't enhanced                    */
    OrefSet(this, this->instanceBehaviour, (RexxBehaviour *)this->instanceBehaviour->copy());
    /* if there is a method to remove     */
    /* from the instance mdict            */
    /* remove it                          */
    if (OREF_NULL != this->instanceMethodDictionary->remove(method_name))
    {
        /* and update our instance behaviour  */
        this->updateInstanceSubClasses();  /* along with our subclasses         */
    }
    return OREF_NULL;                    /* returns nothing                   */
}

RexxMethod *RexxClass::method(
    RexxString  *method_name)
/*****************************************************************************/
/* Function:  Return the method object for the method name                   */
/*****************************************************************************/
{
    /* make sure we have a proper name    */
    method_name = stringArgument(method_name, ARG_ONE)->upper();
    RexxMethod *method_object = (RexxMethod *)this->instanceBehaviour->getMethodDictionary()->stringGet(method_name);
    /* check if it is in the mdict        */
    if ( OREF_NULL == method_object)
    {
        /* if not return an error             */
        reportException(Error_No_method_name, this, method_name);
    }
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
    {
        return this->instanceBehaviour->getMethodDictionary()->supplier();
    }
    /* if TheNilObject specified         */
    /*  return my instance mdict as a    */
    /*  supplier object                  */
    if (class_object == TheNilObject)
    {
        return this->instanceMethodDictionary->supplier();
    }
    /* if not one of the above           */
    /* check if it is a  superclass      */
    if (this->behaviour->checkScope(class_object))
    {
        /*  let the class specified return   */
        /*  it's own methods                 */
        ProtectedObject r;
        class_object->sendMessage(OREF_METHODS, TheNilObject, r);
        return(RexxSupplier *)(RexxObject *)r;
    }
    /* or just return a null supplier    */
    return(RexxSupplier *)TheNullArray->supplier();
}

void  RexxClass::updateSubClasses()
/******************************************************************************/
/* Function: Update my behaviours and call each subclass to do the same       */
/******************************************************************************/
{
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

    RexxArray *subClassList = this->getSubClasses(); /* get the subclasses list           */
    ProtectedObject p(subClassList);
    /* loop thru the subclass doing the  */
    /* same for each of them             */
    for (size_t index = 1; index <= subClassList->size(); index++)
    {
        /* get the next subclass             */
        /* and recursively update them       */
        ((RexxClass *)subClassList->get(index))->updateSubClasses();
    }
}

void RexxClass::updateInstanceSubClasses()
/******************************************************************************/
/* Function: Update my instance behaviour and have the subclasses do the same */
/******************************************************************************/
{
    /* create the instance behaviour from*/
    /* the instance superclass list      */
    this->instanceBehaviour->setMethodDictionary(OREF_NULL);
    this->instanceBehaviour->setScopes(OREF_NULL);
    this->createInstanceBehaviour(this->instanceBehaviour);
    RexxArray *subClassList = this->getSubClasses(); /* get the subclasses list           */
    ProtectedObject p(subClassList);
    /* loop thru the subclass doing the  */
    /* same for each of them             */
    for (size_t index = 1; index <= subClassList->size(); index++)
    {
        /* get the next subclass             */
        /* recursively update these          */
        ((RexxClass *)subClassList->get(index))->updateInstanceSubClasses();
    }
}

void RexxClass::createClassBehaviour(
    RexxBehaviour *target_class_behaviour)
/*****************************************************************************/
/* Funcion:  To call the superclasses and have them update this classes      */
/*           class behaviour mdict and scopes table                          */
/*****************************************************************************/
{
    RexxClass   * superclass;            /* superclass being called           */
    RexxClass   * metaclass;             /* metaclass to use                  */


                                         /* Call each of the superclasses in  */
                                         /* this superclass list starting from*/
                                         /* the last to the first             */
    for (HashLink index = this->classSuperClasses->size(); index > 0; index--)
    {
        /* get the next superclass           */
        superclass = (RexxClass *)this->classSuperClasses->get(index);
        /* if there is a superclass and      */
        /* it hasn't been added into this    */
        /* behaviour yet, call and have it   */
        /* add itself                        */
        if (superclass != TheNilObject && !target_class_behaviour->checkScope(superclass))
        {
            superclass->createClassBehaviour(target_class_behaviour);
        }
    }
    /* If this class mdict has not been   */
    /* merged into this target behaviour  */
    if (!target_class_behaviour->checkScope(this))
    {
        if (TheObjectClass != this)        /* if this isn't OBJECT              */
        {
            // we only process the first item in the metaclass list, since it
            // will properly pull in the scopes for all of the rest, in the correct order.
            metaclass = (RexxClass *)this->metaClass->get(1);
            /* add which ever metaclasses have    */
            /* not been added yet                 */
            if (metaclass != TheNilObject && !target_class_behaviour->checkScope(metaclass))
            {
                /* merge in the meta class mdict      */
                target_class_behaviour->methodDictionaryMerge(metaclass->instanceBehaviour->getMethodDictionary());
                // now we need to merge in the scopes.  For each metaclass, starting
                // from the bottom of the hierarchy down, merge in each of the scope
                // values.
                RexxArray *addedScopes = metaclass->behaviour->getScopes()->allAt(TheNilObject);
                ProtectedObject p(addedScopes);

                // these need to be processed in reverse order
                for (size_t i = addedScopes->size(); i > 0; i--)
                {
                    RexxClass *scope = (RexxClass *)addedScopes->get(i);
                    target_class_behaviour->mergeScope(scope);
                }
            }
        }
        /* only merge the mdict for CLASS     */
        /* if this is a capable of being a    */
        /* metaclass                          */
        if ((this != TheClassClass) || (this == TheClassClass && this->isMetaClass()))
        {
            /* Merge this class mdict with the    */
            /* target behaviour class mdict       */
            target_class_behaviour->methodDictionaryMerge(this->classMethodDictionary);
        }
        /* And update the target behaviour    */
        /* scopes table with this class       */
        if (this != TheClassClass && !target_class_behaviour->checkScope(this))
        {
            target_class_behaviour->addScope(this);
        }
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
    /* Call each of the superclasses in  */
    /* this superclass list starting from*/
    /* the last going to the first       */
    for (HashLink index = this->instanceSuperClasses->size(); index > 0; index--)
    {
        /* get the next super class          */
        RexxClass *superclass = (RexxClass *)this->instanceSuperClasses->get(index);
        /* if there is a superclass and      */
        /* it hasn't been added into this    */
        /* behaviour yet, call and have it   */
        /* add itself                        */
        if (superclass != TheNilObject && !target_instance_behaviour->checkScope(superclass))
        {
            superclass->createInstanceBehaviour(target_instance_behaviour);
        }
    }
    /* If this class mdict has not been   */
    /* merged into this target behaviour  */
    if (!target_instance_behaviour->checkScope(this))
    {
        /* Merge this class mdict with the    */
        /* target behaviour class mdict       */
        target_instance_behaviour->methodDictionaryMerge(this->instanceMethodDictionary);
        /* And update the target behaviour    */
        /* scopes table with this class       */
        target_instance_behaviour->addScope(this);
    }
}


/**
 * Merge the scopes from the superclasses into a target primitive class.
 *
 * @param target_instance_behaviour
 *               The target behavior to update.
 */
void RexxClass::mergeSuperClassScopes(RexxBehaviour *target_instance_behaviour)
{
    // Call each of the superclasses in this superclass list starting from
    // the last going to the first
    for (HashLink index = this->instanceSuperClasses->size(); index > 0; index--)
    {
        RexxClass *superclass = (RexxClass *)this->instanceSuperClasses->get(index);
        // if there is a superclass and it hasn't been added into this
        // behaviour yet, call and have it add itself                        */
        if (superclass != TheNilObject && !target_instance_behaviour->checkScope(superclass))
        {
            superclass->mergeSuperClassScopes(target_instance_behaviour);
        }
    }
    // now add in the scope for this class, if still needed.
    if (!target_instance_behaviour->checkScope(this))
    {
        /* Merge this class mdict with the    */
        /* target behaviour class mdict       */
        target_instance_behaviour->merge(this->instanceBehaviour);
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
    if (source_mdict == OREF_NULL)       /* check for a source mdict          */
    {
        return;                            /* there isn't anything to do        */
    }
    /* just loop through the entries     */
    for (HashLink i = source_mdict->first(); source_mdict->available(i); i = source_mdict->next(i))
    {
        /* get the method name               */
        RexxString *method_name = REQUEST_STRING(source_mdict->index(i));
        /* get the method                    */
        RexxMethod *method_instance = (RexxMethod *)source_mdict->value(i);
        /* add the method to the target mdict */
        target_mdict->stringAdd(method_instance, method_name);
        /* check if the method that was added */
        /* is the uninit method               */
        if ( method_name->strCompare(CHAR_UNINIT))
        {
            this->setHasUninitDefined();     /* and turn on uninit if so          */
        }
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
    RexxTable *newDictionary = new_table(); /* get a new table for this          */
    ProtectedObject p(newDictionary);
    /* loop thru the supplier object     */
    /* obtained from the source mdict    */
    ProtectedObject p2;
    sourceCollection->sendMessage(OREF_SUPPLIERSYM, p2);
    RexxSupplier *supplier = (RexxSupplier *)(RexxObject *)p2;
    for (; supplier->available() == TheTrueObject; supplier->next())
    {
        /* get the method name (uppercased)  */
        RexxString *method_name = REQUEST_STRING(supplier->index())->upper();
        /* get the method                    */
        RexxMethod *newMethod = (RexxMethod *)supplier->value();
        /* if the method is not TheNilObject */
        if (newMethod != (RexxMethod *)TheNilObject)
        {
            /* and it isn't a primitive method   */
            if (!isOfClass(Method, newMethod))   /* object                            */
            {
                /* make it into a method object      */
                newMethod = RexxMethod::newMethodObject(method_name, newMethod, IntegerOne, OREF_NULL);
                newMethod->setScope(scope);   /* and set the scope to the given    */
            }
            else
            {
                /* if it is a primitive method object */
                /* let the newscope method copy it    */
                newMethod = newMethod->newScope(scope);
            }
        }
        /* add the method to the target mdict */
        newDictionary->stringAdd(newMethod, method_name);
    }
    return newDictionary;                /* and return the new version        */
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
                                         /* make sure this isn't a rexx       */
    if (this->isRexxDefined())           /* defined class being changed       */
    {
        /* report as a nomethod condition    */
        reportNomethod(lastMessageName(), this);
    }
    requiredArgument(mixin_class, ARG_ONE);      /* make sure it was passed in        */

                                         /* check the mixin class is really a */
                                         /* good class for this               */
    if (!mixin_class->isInstanceOf(TheClassClass) || !mixin_class->isMixinClass())
    {
        /* if it isn't raise an error        */
        reportException(Error_Execution_mixinclass, mixin_class);
    }

    /* if the mixin class is also the    */
    if (this == mixin_class )            /* reciever class                    */
    {
        /*  raise an error                   */
        reportException(Error_Execution_recursive_inherit, this, mixin_class);
    }
    /* check that the mixin class is not */
    /* a superclass of the reciever      */
    if (this->behaviour->checkScope(mixin_class))
    {
        /* if it is raise an error           */
        reportException(Error_Execution_recursive_inherit, this, mixin_class);
    }
    /* check if the reciever class is a  */
    /* superclass of the mixin class     */
    if (mixin_class->behaviour->checkScope(this))
    {
        /* if it is it's an error            */
        reportException(Error_Execution_recursive_inherit, this, mixin_class);
    }

    /* Now ensure the mixin class        */
    /* baseclass is in the reciever's    */
    /* class superclass hierarchy        */
    if (!this->behaviour->checkScope(mixin_class->getBaseClass()))
    {
        /* if it isn't raise an error        */
        reportException(Error_Execution_baseclass, this, mixin_class, mixin_class->getBaseClass());
    }

    /* and the reciever's                */
    /* instance superclass hierarchy     */
    if (!this->instanceBehaviour->checkScope(mixin_class->getBaseClass()))
    {
        /* if it isn't raise an error        */
        reportException(Error_Execution_baseclass, this, mixin_class, mixin_class->getBaseClass());
    }
    if ( position == OREF_NULL )         /* if position was not specified     */
    {
        /* insert the mixin class last in the*/
        /* reciever's superclasses list      */
        this->classSuperClasses->addLast(mixin_class);
        this->instanceSuperClasses->addLast(mixin_class);
    }
    else                                 /* if it was specified               */
    {
        /* check that it's a valid superclass*/
        /* in the class superclasses list    */
        /* and the reciever's                */
        /* instance superclasses list        */
        HashLink class_index = this->classSuperClasses->indexOf(position);
        HashLink instance_index = this->instanceSuperClasses->indexOf(position);
        if (class_index == 0 || instance_index == 0)
        {
            /* if it isn't raise an error        */
            reportException(Error_Execution_uninherit, this, position);
        }
        /* insert the mixin class into the   */
        /* superclasses list's               */
        this->classSuperClasses->insertItem(mixin_class, class_index + 1);
        this->instanceSuperClasses->insertItem(mixin_class, instance_index + 1);
    }

    /* update the mixin class subclass   */
    mixin_class->addSubClass(this);      /* list to reflect this class        */
                                         /* any subclasses that we have need  */
                                         /* to redo their behaviour's         */
                                         /* this also updates our own         */
                                         /* behaviour tables.                 */
    this->updateSubClasses();
    /* If the mixin class has an uninit defined, the new class must have one, too */
    if (mixin_class->hasUninitDefined() || mixin_class->parentHasUninitDefined())
    {
        this->setParentHasUninitDefined();
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
    HashLink     class_index;            /* index for class superclasses list */
    HashLink     instance_index;         /* index for instance superclasses   */
                                         /* make sure this isn't rexx defined */
    if (this->isRexxDefined())           /* class that is being changed       */
    {
        /* report as a nomethod condition    */
        reportNomethod(lastMessageName(), this);
    }
    requiredArgument(mixin_class, ARG_ONE);      /* make sure it was passed in        */

                                         /* check that the mixin class is a   */
                                         /* superclass of the receiver class  */
                                         /* and not the superclass            */
    if ( ((class_index = this->classSuperClasses->indexOf(mixin_class)) > 1) &&
         ((instance_index = this->instanceSuperClasses->indexOf(mixin_class)) > 1))
    {
        /* and remove it                     */
        this->classSuperClasses->deleteItem(class_index);
        this->instanceSuperClasses->deleteItem(instance_index);
    }
    else
    {
        /* else     raise an error           */
        reportException(Error_Execution_uninherit, this, mixin_class);
    }
    /* update the mixin class subclass    */
    /* list to not have this class        */
    removeSubclass(mixin_class);
    /* any subclasses that we have need   */
    /* to redo their behaviour's          */
    /* this also updates our own behaviour*/
    this->updateSubClasses();            /* tables.                           */
    return OREF_NULL;                    /* returns nothing                   */
}


/**
 * Remove a subclass from the uninherit list after an uninherit
 * operation.
 *
 * @param c      The class to remove.
 */
void RexxClass::removeSubclass(RexxClass *c)
{
    size_t index = subClasses->firstIndex();
    // scan the subclasses list looking for the removed class
    while (index != LIST_END)
    {
        WeakReference *ref = (WeakReference *)subClasses->getValue(index);
        RexxObject *sc = ref->get();
        if (sc == c)
        {
            subClasses->removeIndex(index);
            return;
        }
        index = subClasses->nextIndex(index);
    }
}

RexxObject *RexxClass::enhanced(
    RexxObject **args,                 /* enhanced arguments                */
    size_t       argCount)             /* the number of arguments           */
/*****************************************************************************/
/* Function:  Create a new object that is an instance of the receiver class  */
/*            object that has had it's instance mdict enhanced.              */
/*****************************************************************************/
{
    if (argCount == 0)                   /* make sure an arg   was passed in  */
    {
        /* if not report an error            */
        reportException(Error_Incorrect_method_minarg, IntegerOne);
    }
    /* get the value of the arg          */
    RexxTable *enhanced_instance_mdict = (RexxTable *)args[0];
    /* make sure it was a real value     */
    requiredArgument(enhanced_instance_mdict, ARG_ONE);
    /* subclass the reciever class       */
    RexxClass *dummy_subclass = this->subclass(new_string("Enhanced Subclass"), OREF_NULL, OREF_NULL);
    ProtectedObject p(dummy_subclass);
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
    ProtectedObject r;
    /* get an instance of the enhanced   */
    /* subclass                          */
    dummy_subclass->sendMessage(OREF_NEW, args + 1, argCount - 1, r);
    RexxObject *enhanced_object = (RexxObject *)r;
    /* change the create_class in the    */
    /* instance behaviour to point to the*/
    /* original class object             */
    enhanced_object->behaviour->setOwningClass(this);
    /* remember it was enhanced          */
    enhanced_object->behaviour->setEnhanced();

    return enhanced_object;              /* send back the new improved version*/
}

RexxClass  *RexxClass::mixinclass(
    RexxString  * mixin_id,            /* ID name of the class              */
    RexxClass   * meta_class,          /* source meta class                 */
                                       /* extra class methods               */
    RexxTable   * enhancing_class_methods)
/*****************************************************************************/
/* Function:  Create a new class object containng class and instance methods */
/*            to be used for multiple inheritance .                          */
/*****************************************************************************/
{
    /* call subclass with the parameters */
    RexxClass *mixin_subclass = this->subclass(mixin_id, meta_class, enhancing_class_methods);
    mixin_subclass->setMixinClass();     /* turn on the mixin info            */
                                         /* change the base class to the base */
                                         /* class of the reciever             */
    OrefSet(mixin_subclass, mixin_subclass->baseClass, this->baseClass);
    /* If the mixin's parent class has an uninit defined, the new mixin class must have one, too */
    if (this->hasUninitDefined() || this->parentHasUninitDefined())
    {
        mixin_subclass->setParentHasUninitDefined();
    }
    return mixin_subclass;               /* return the new mixin class        */
}


RexxClass  *RexxClass::subclass(
    RexxString  * class_id,            /* ID name of the class              */
    RexxClass   * meta_class,          /* source meta class                 */
                                       /* extra class methods               */
    RexxTable   * enhancing_class_methods)
/*****************************************************************************/
/* Function:  Create a new class object that is a subclass of this class     */
/*            object.                                                        */
/*****************************************************************************/
{
    if (meta_class == OREF_NULL)         /* if there is no metaclass specified*/
    {
        meta_class = this->getMetaClass(); /* use the default metaclass         */
    }

    /* check that it is a meta class     */
    if (!meta_class->isInstanceOf(TheClassClass) || !meta_class->isMetaClass())
    {
        reportException(Error_Translation_bad_metaclass, meta_class);
    }
    ProtectedObject p;
    /* get a copy of the metaclass class */
    meta_class->sendMessage(OREF_NEW, class_id, p);
    RexxClass *new_class = (RexxClass *)(RexxObject *)p;
    if (this->isMetaClass())             /* if the superclass is a metaclass  */
    {
        new_class->setMetaClass();         /* mark the new class as a meta class*/
                                           /* and if the metaclass lists haven't */
                                           /* been updated yet                   */
        if (new_class->metaClassScopes->get(this) == OREF_NULL)
        {
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
    OrefSet(new_class, new_class->classSuperClasses, new_array(this));
    /* make the receiver class the        */
    /* superclass for the instance behav  */
    OrefSet(new_class, new_class->instanceSuperClasses, new_array(this));
    /* if there was enhancing methods     */
    /* specified                          */
    if (enhancing_class_methods != OREF_NULL && enhancing_class_methods != TheNilObject)
    {
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
    new_class->behaviour->setOwningClass(meta_class);
    /* create the instance behaviour from */
    /* the instance superclass list       */
    new_class->instanceBehaviour->setMethodDictionary(OREF_NULL);
    new_class->instanceBehaviour->setScopes(OREF_NULL);
    new_class->createInstanceBehaviour(new_class->instanceBehaviour);
    /* set the instance behaviour created */
    /* class to the reciever class        */
    new_class->instanceBehaviour->setOwningClass(new_class);
    /* update the receiver class' subclass*/

    this->addSubClass(new_class);        /* list to reflect the new class     */
    /* if the class object has an UNINIT method defined, make sure we */
    /* add this to the table of classes to be processed. */
    if (new_class->hasUninitMethod())
    {
        new_class->hasUninit();
    }
    new_class->sendMessage(OREF_INIT);   /* now drive any user INIT methods   */
                                         /* now the new class object should   */
    /* If the parent class has an uninit defined, the new child class must have one, too */
    if (this->hasUninitDefined() || this->parentHasUninitDefined())
    {
        new_class->setParentHasUninitDefined();
    }
    /* notify activity this object has an UNINIT that needs to be called
                when collecting the object */
    if (new_class->hasUninitDefined())
    {
        new_class->setHasUninitDefined();
    }

    return new_class;                    /* return the new class              */
}

void RexxClass::setMetaClass(
    RexxClass *new_metaClass )         /* new meta class to add             */
/******************************************************************************/
/* Function:  Set a metaclass for a class                                     */
/******************************************************************************/
{
    OrefSet(this, this->metaClass, new_array(TheClassClass));
    this->metaClass->addFirst(new_metaClass);
                                       /* the metaclass mdict list           */
    OrefSet(this, this->metaClassMethodDictionary, new_array(TheClassClass->instanceMethodDictionary->copy()));
    this->metaClassMethodDictionary->addFirst(new_metaClass->instanceMethodDictionary);
                                       /* and the metaclass scopes list      */
    OrefSet(this, this->metaClassScopes, (RexxIdentityTable *)TheClassClass->behaviour->getScopes()->copy());
                                       /* add the scope list for this scope  */
    this->metaClassScopes->add(new_metaClass, TheNilObject);
    this->metaClassScopes->add(this->metaClassScopes->allAt(TheNilObject), new_metaClass);
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
    requiredArgument(other, ARG_ONE);            // must have the other argument
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
    size_t size1,                      /* additional size                   */
    const char *className,             // The id string of the class
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
    {
        /* Get new class object              */
        new_class = (RexxClass *)new_object(size);
    }
    else
    {
        /* use the specified size            */
        new_class = (RexxClass *)new_object(size1);
    }
                                         // set this value immediately
    new_class->id = new_string(className);
    /* set the class specific behaviour  */
    new_class->setBehaviour(class_behaviour);
    /* set the class into the behaviour  */
    new_class->behaviour->setOwningClass(new_class);
    /* set the instance behaviour        */
    OrefSet(new_class, new_class->instanceBehaviour, instanceBehaviour);
    /* and the class of this behaviour   */
    new_class->instanceBehaviour->setOwningClass(new_class);
    /* tell the mobile support to just   */
    new_class->makeProxiedObject();      /* make a proxy for this class       */
    return(void *)new_class;            /* should be ready                   */
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
    if (argCount == 0)                   /* make sure an arg   was passed in  */
    {
        /* if not report an error            */
        reportException(Error_Incorrect_method_minarg, IntegerOne);
    }
    RexxString *class_id = (RexxString *)args[0];    /* get the id parameter              */
    class_id = stringArgument(class_id, ARG_ONE);   /* and that it can be a string       */
    /* get a copy of this class object   */
    RexxClass *new_class = (RexxClass *)this->clone();

    // NOTE:  we do this before save() is called.  The class object hash value
    // is based off of the string name, so we need to set this before we
    // attempt putting this into a hash collection.
    OrefSet(new_class, new_class->id, class_id);
    /* update cloned hashvalue           */
    ProtectedObject p(new_class);        /* better protect this               */
                                         /* make this into an instance of the */
                                         /* meta class                        */
    OrefSet(new_class, new_class->behaviour, (RexxBehaviour *)new_class->instanceBehaviour->copy());
    /* don't give access to this class'   */
    /* class mdict                        */
    OrefSet(new_class, new_class->classMethodDictionary, new_table());
    /* make this class the superclass     */
    OrefSet(new_class, new_class->classSuperClasses, new_array(this));
    new_class->behaviour->setOwningClass(this);/* and set the behaviour class       */
    /* if this is a primitive class then  */
    /* there isn't any metaclass info     */
    if (this->isPrimitiveClass())        /* set up yet                        */
    {
        /* set up the new metaclass list      */
        OrefSet(new_class, new_class->metaClass, new_array(TheClassClass));
        /* the metaclass mdict list           */
        OrefSet(new_class, new_class->metaClassMethodDictionary, new_array(TheClassClass->instanceMethodDictionary->copy()));
        /* and the metaclass scopes list      */
        OrefSet(new_class, new_class->metaClassScopes, (RexxIdentityTable *)TheClassClass->behaviour->getScopes()->copy());
    }
    else
    {
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
        OrefSet(new_class, new_class->metaClassScopes, (RexxIdentityTable *)new_class->metaClassScopes->copy());
        /* and update the scopes to include   */
        /* the metaclass scopes               */
        new_class->metaClassScopes->add(this, TheNilObject);
        new_class->metaClassScopes->add(this->behaviour->getScopes()->allAt(TheNilObject), this);
    }

    // create the subclasses list
    OrefSet(new_class, new_class->subClasses, new_list());
    /* set up the instance behaviour with */
    /*  object's instance methods         */
    OrefSet(new_class, new_class->instanceBehaviour, (RexxBehaviour *)TheObjectClass->instanceBehaviour->copy());
    /* don't give access to this class'   */
    /*  instance mdict                    */
    OrefSet(new_class, new_class->instanceMethodDictionary, new_table());
    /* make the instance_superclass list  */
    /* with OBJECT in it                  */
    OrefSet(new_class, new_class->instanceSuperClasses, new_array(TheObjectClass));
    /* and set the behaviour class        */
    new_class->instanceBehaviour->setOwningClass(TheObjectClass);
    /* and the instance behaviour scopes  */
    new_class->instanceBehaviour->setScopes(new_identity_table());
    /* set the scoping info               */
    new_class->instanceBehaviour->addScope(TheObjectClass);
    /* don't give access to this class'   */
    /*  ovd's                             */
    OrefSet(new_class, new_class->objectVariables, OREF_NULL);
    /* set the new class as it's own      */
    /* baseclass                          */
    OrefSet(new_class, new_class->baseClass, new_class);
    /* clear the info area except for     */
    /* uninit                             */
    new_class->setInitialFlagState();
    /* if the class object has an UNINIT method defined, make sure we */
    /* add this to the table of classes to be processed. */
    if (new_class->hasUninitDefined())
    {
        new_class->setHasUninitDefined();
    }
    new_class->sendMessage(OREF_INIT, args + 1, argCount - 1);
    return new_class;                    /* return the new class              */
}

void RexxClass::createInstance()
/******************************************************************************/
/* Function:  Create the initial class object                                 */
/******************************************************************************/
{
    /* create a class object             */
    TheClassClass = (RexxClass *)new_object(sizeof(RexxClass));
    /* set the instance behaviour         */
    TheClassClass->setBehaviour(TheClassClassBehaviour);
    /* set the instance behaviour         */
    TheClassClass->setInstanceBehaviour(TheClassBehaviour);

    // the initial class needs to have an ID before it can be used for
    // other purposes.
    TheClassClass->id = new_string("Class");

    /* tell the mobile support to just    */
    /* make a proxy for this class        */
    TheClassClass->makeProxiedObject();
    new (TheClassClass) RexxClass;
}


void RexxClass::processNewArgs(
    RexxObject **arg_array,            /* source argument array             */
    size_t       argCount,             /* size of the argument array        */
    RexxObject***init_args,            /* remainder arguments               */
    size_t      *remainderSize,        /* remaining count of arguments      */
    size_t       required,             /* number of arguments we require    */
    RexxObject **argument1,            /* first returned argument           */
    RexxObject **argument2 )           /* second return argument            */
/******************************************************************************/
/* Function:  Divide up a class new arglist into new arguments and init args  */
/******************************************************************************/
{
    *argument1 = OREF_NULL;              /* clear the first argument          */
    if (argCount >= 1)                   /* have at least one argument?       */
    {
        *argument1 = arg_array[0];         /* get the first argument            */
    }
    if (required == 2)
    {                 /* processing two arguments?         */
        if (argCount >= 2)                 /* get at least 2?                   */
        {
            *argument2 = arg_array[1];       /* get the second argument           */
        }
        else
        {
            *argument2 = OREF_NULL;          /* clear the second argument         */
        }
    }
    /* get the init args part            */
    *init_args = arg_array + required;
    /* if we have at least the required arguments, reduce the count. */
    /* Otherwise, set this to zero. */
    if (argCount >= required)
    {
        *remainderSize = argCount - required;
    }
    else
    {
        *remainderSize = 0;
    }
}

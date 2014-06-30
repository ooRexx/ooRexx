/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Behaviour Class                                                  */
/*                                                                            */
/******************************************************************************/
#include <string.h>
#include "RexxCore.h"
#include "RexxBehaviour.hpp"
#include "StringClass.hpp"
#include "MethodClass.hpp"
#include "ArrayClass.hpp"
#include "SupplierClass.hpp"
#include "ProtectedObject.hpp"
#include "CPPCode.hpp"


RexxBehaviour::RexxBehaviour(
    size_t          newTypenum,        /* class type number                 */
    PCPPM *         operator_methods ) /* operator lookaside table          */
/******************************************************************************/
/* Function:  Construct C++ methods in OKGDATA.C                              */
/******************************************************************************/
{
    behaviour = getPrimitiveBehaviour(T_Behaviour);
    header.setObjectSize(sizeof(RexxBehaviour));
    setClassType(newTypenum);
    behaviourFlags = 0;
    scopes = OREF_NULL;
    methodDictionary = OREF_NULL;
    operatorMethods = operator_methods;
    owningClass = OREF_NULL;
    instanceMethodDictionary = OREF_NULL;

    // if this is an internal class, normalize this so we can
    // restore this to the correct value if we add additional internal classes.
    if (newTypenum > T_Last_Exported_Class && newTypenum < T_First_Transient_Class)
    {

        behaviourFlags |=  INTERNAL_CLASS;
    }
    else if (newTypenum >= T_First_Transient_Class)
    {

        behaviourFlags |=  TRANSIENT_CLASS;
    }


}

void RexxBehaviour::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(methodDictionary);
  memory_mark(instanceMethodDictionary);
  memory_mark(scopes);
  memory_mark(owningClass);
}

void RexxBehaviour::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    /* Save image processing?        */
    if (memoryObject.savingImage() && isNonPrimitive())
    {
        // mark this as needing resolution when restored.
        setNotResolved();
    }
    // the other side of the process?
    else if (memoryObject.restoringImage())
    {
        // if we have a non-primitive here on a restore image, we need to fix this up.
        if (isNonPrimitive())
        {
            resolveNonPrimitiveBehaviour();
        }
    }

    memory_mark_general(methodDictionary);
    memory_mark_general(instanceMethodDictionary);
    memory_mark_general(scopes);
    memory_mark_general(owningClass);
}

void RexxBehaviour::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxBehaviour)

   flatten_reference(newThis->methodDictionary, envelope);
   flatten_reference(newThis->instanceMethodDictionary, envelope);
   flatten_reference(newThis->scopes, envelope);
   flatten_reference(newThis->owningClass, envelope);

                                       /* Is this a non-primitive behav */
   if (isNonPrimitive())
   {
                                       /* yes, mark that we need to be  */
                                       /*  resolved on the puff.        */
       newThis->setNotResolved();
   }
  cleanUpFlatten
}

/**
 * Do fix ups for non-primitive behaviours, ensuring they
 * get all of the appropriate information from their parent
 * primitive behaviour types.
 */
void RexxBehaviour::resolveNonPrimitiveBehaviour()
{
    if (isNotResolved())
    {
        setResolved();
        operatorMethods = getOperatorMethods(getClassType());
    }
}


RexxObject *RexxBehaviour::copy()
/******************************************************************************/
/*  Function:  Copy the behaviour object with an independent named method     */
/*             dictionary, but leave the original create_class.               */
/******************************************************************************/
{
    /* Instead of calling new_object and memcpy, ask the memory object to make*/
    /* a copy of ourself.  This way, any header information can be correctly  */
    /* initialized by memory.                                                 */

    /* first, clone the existing object  */
    RexxBehaviour *newBehaviour = (RexxBehaviour *)clone();
    /* have an method dictionary         */
    if (methodDictionary != OREF_NULL)
    {
        /* make a copy of this too           */
        OrefSet(newBehaviour, newBehaviour->methodDictionary, (RexxTable *)methodDictionary->copy());
    }
    if (scopes != OREF_NULL)       /* scope information?                */
    {
        /* make a copy of it too             */
        OrefSet(newBehaviour, newBehaviour->scopes, (RexxIdentityTable *)scopes->copy());
    }
    /* do we have added methods?         */
    if (instanceMethodDictionary != OREF_NULL)
    {
        /* copy those also                   */
        OrefSet(newBehaviour, newBehaviour->instanceMethodDictionary, (RexxTable *)instanceMethodDictionary->copy());
    }
    /* use default operator methods set  */
    newBehaviour->operatorMethods = RexxObject::operatorMethods;
    /* all copied behaviours are         */
    /* non-primitive ones                */
    newBehaviour->setNonPrimitive();
    return(RexxObject *)newBehaviour;   /* return the copied behaviour       */
}


void RexxBehaviour::copyBehaviour(RexxBehaviour *source)
/******************************************************************************/
/*  Function:  Copy the source behaviour object into this, inheriting all of  */
/*             the method dictionaries.                                       */
/******************************************************************************/
{
    /* have an method dictionary         */
    if (source->methodDictionary != OREF_NULL)
    {
        /* make a copy of this too           */
        setField(methodDictionary, (RexxTable *)source->methodDictionary->copy());
    }
    if (source->scopes != OREF_NULL)       /* scope information?                */
    {
        /* make a copy of it too             */
        setField(scopes, (RexxIdentityTable *)source->scopes->copy());
    }
    /* do we have added methods?         */
    if (source->instanceMethodDictionary != OREF_NULL)
    {
        /* copy those also                   */
        setField(instanceMethodDictionary, (RexxTable *)source->instanceMethodDictionary->copy());
    }
    // this is the same class as the source also
    setField(owningClass, source->owningClass);
    /* use default operator methods set  */
    operatorMethods = (PCPPM *)source->operatorMethods;
}


/**
 * Define a native kernel method on this behaviour.
 *
 * @param name       The method name.
 * @param entryPoint The method entry point
 * @param arguments  The argument definition.
 *
 * @return The created method object.
 */
RexxMethod *RexxBehaviour::define(const char *name, PCPPM entryPoint, size_t arguments)
{
    RexxString *n = RexxMemory::getGlobalName(name);
    RexxMethod *method = new RexxMethod(n, CPPCode::resolveExportedMethod(name, entryPoint, arguments));
    define(n, method);
    return method;
}


RexxObject *RexxBehaviour::define(
    RexxString *methodName,            /* name of the defined method        */
    RexxMethod *method)                /* method to add to the behaviour    */
/******************************************************************************/
/* Function:  Add or remove a method from an object's behaviour               */
/******************************************************************************/
{
    RexxMethod  * tableMethod;           /* method from the table             */

                                         /* no method dictionary yet?         */
    if (methodDictionary == OREF_NULL)
    {
        /* allocate a table                  */
        setField(methodDictionary, new_table());
    }


    if (method == OREF_NULL || method == TheNilObject)
    {
        /* replace the method with .nil      */
        methodDictionary->stringPut(TheNilObject, methodName);

    }
    else
    {
        /* already have this method?         */
        if ((tableMethod = (RexxMethod *)methodDictionary->stringGet(methodName)) == OREF_NULL)
        {
            /* No, just add this directly        */
            methodDictionary->stringAdd(method, methodName);
        }
        else
        {
            /* are the scopes the same?          */
            if (tableMethod->getScope() == method->getScope())
            {
                /* same scope, so replace existing   */
                /* method with the new one           */
                methodDictionary->stringPut(method, methodName);

            }
            else
            {
                /* new scope, for this, just replace */
                methodDictionary->stringAdd(method, methodName);

            }
        }
    }
    return OREF_NULL;                    /* always return nothing             */
}

void RexxBehaviour::removeMethod(
    RexxString *methodName )           /* name of the removed method        */
/******************************************************************************/
/* Function:  Reverse a SETMETHOD operation                                   */
/******************************************************************************/
{
    /* actually done SETMETHOD calls?    */
    if (instanceMethodDictionary != OREF_NULL)
    {
        /* do we have one of these?          */
        if (instanceMethodDictionary->remove(methodName) != OREF_NULL)
        {
            /* remove from the real dictionary   */
            methodDictionary->remove(methodName);
        }
    }
}

void RexxBehaviour::addMethod(
    RexxString *methodName,            /* name of the defined method        */
    RexxMethod *method)                /* method to add to the behaviour    */
/******************************************************************************/
/* Function:  Add a method to an object's behaviour                           */
/******************************************************************************/
{
    /* no method dictionary yet?         */
    if (methodDictionary == OREF_NULL)
    {
        /* allocate a table                  */
        setField(methodDictionary, new_table());
    }
    /* now repeat for the instance       */
    if (instanceMethodDictionary == OREF_NULL)
    {
        /* methods to track additions        */
        setField(instanceMethodDictionary, new_table());
    }
    /* already added one by this name?   */
    if (instanceMethodDictionary->stringGet(methodName) != OREF_NULL)
    {
        /* remove from the method dictionary */
        methodDictionary->remove(methodName);
    }

    /* now just add this directly        */
    methodDictionary->stringAdd(method, methodName);
    /* and also add to the instance one  */
    instanceMethodDictionary->stringPut(method, methodName);
}

RexxMethod *RexxBehaviour::methodObject(
    RexxString *messageName )          /* name of method to retrieve        */
/******************************************************************************/
/* Function:  Retrieve a method associated with the given name                */
/******************************************************************************/
{
    /* force to a string version (upper  */
    /* case required)                    */
    messageName = stringArgument(messageName, ARG_ONE)->upper();
    /* now just do a method lookup       */
    return methodLookup(messageName);
}

RexxMethod *RexxBehaviour::methodLookup(
    RexxString *messageName )          /* name of method to retrieve        */
/******************************************************************************/
/* Function:  Perform lowest level method lookup on an object                 */
/******************************************************************************/
{
    /* have a method dictionary?         */
    if (methodDictionary != OREF_NULL)
    {
        // just get the object directly.  Unknown methods will return OREF_NULL.  However,
        // explicit overrides are indicated by putting .nil in the table.  Our callers
        // are dependent upon getting OREF_NULL back for unknown methods.
        RexxMethod *method = (RexxMethod *)methodDictionary->stringGet(messageName);
        if (method != TheNilObject)
        {
            return method;
        }
    }
    return OREF_NULL;
}

RexxMethod *RexxBehaviour::getMethod(
    RexxString *messageName )          /* name of method to retrieve        */
/******************************************************************************/
/* Function:  Retrieve a method object from the method dictionary.  This      */
/*            returns OREF_NULL if the method does not exist.                 */
/******************************************************************************/
{
    if (methodDictionary != OREF_NULL)
    {
        /* try to get the method             */
        return(RexxMethod *)methodDictionary->stringGet(messageName);
    }
    return OREF_NULL;                    /* return the method object          */
}

RexxObject *RexxBehaviour::deleteMethod(
    RexxString *messageName )          /* name of method to delete          */
/******************************************************************************/
/* Function:  Delete a method from an object's behaviour                      */
/******************************************************************************/
{
    /* have a dictionary?                */
    if (methodDictionary != OREF_NULL)
    {
        /* just remove from the table        */
        methodDictionary->remove(messageName);
    }
    return OREF_NULL;                    /* always return nothing             */
}

void RexxBehaviour::subclass(
     RexxBehaviour *subclass_behaviour)/* source behaviour                  */
/******************************************************************************/
/* Function:  Replace the fields in a new behaviour attached to a new         */
/*              subclass class object from the subclassed class behaviour.    */
/******************************************************************************/
{
                                       /* replace the typenum               */
    setClassType(subclass_behaviour->getClassType());
}

void RexxBehaviour::restore(
    RexxBehaviour * saved)             /* the saved behaviour info          */
/******************************************************************************/
/* Function:  Restore primtive behaviours                                                                                            */
/******************************************************************************/
{
    /* set the behaviour behaviour       */
    setBehaviour(getPrimitiveBehaviour(T_Behaviour));
    /* set proper size                   */
    setObjectSize(roundObjectBoundary(sizeof(RexxBehaviour)));
    setOldSpace();
    /* Make sure we pick up additional   */
    /*  methods defined during saveimage */
    /* Don't use OrefSet here            */
    methodDictionary = saved->getMethodDictionary();
    scopes = saved->getScopes();   /* and the scopes that are there     */
                                         /* copy over the associated class    */
    owningClass = saved->getOwningClass();
}

RexxClass *RexxBehaviour::restoreClass()
/******************************************************************************/
/* Function:  Update and return a primitive behaviour's primitive class       */
/******************************************************************************/
{
    /* Adjust the instance behaviour.  Note that we don't use */
    /* OrefSet() for this.  When we're restoring the classes, the */
    /* class objects are in oldspace, and the behaviours are */
    /* primitive objects, not subject to sweeping.  We do a direct */
    /* assignment to avoid creating a reference entry in the old2new */
    /* table. */
    owningClass->setInstanceBehaviour(this);
    return owningClass;            /* return the associated class       */
}

void *RexxBehaviour::operator new(size_t size,
    size_t typenum)                     /* target behaviour type number      */
/******************************************************************************/
/* Function:  Create and initialize a target primitive behaviour              */
/******************************************************************************/
{
    // return a pointer to the static primitive one
    return (void *)getPrimitiveBehaviour(typenum);
}

RexxObject * RexxBehaviour::superScope(
    RexxObject * start_scope)          /* requested current scope           */
/******************************************************************************/
/* Function:  Return the scope following a give scope                         */
/******************************************************************************/
{
    if (scopes == OREF_NULL)       /* no scopes defined?                */
    {
        return TheNilObject;               /* no super scoping possible         */
    }
    /* go get the super scope            */
    return scopes->findSuperScope(start_scope);
}

RexxMethod *RexxBehaviour::superMethod(
    RexxString * messageName,          /* target method name                */
    RexxObject * startScope)           /* starting scope                    */
/******************************************************************************/
/* Function:   Find a method using the given starting scope information       */
/******************************************************************************/
{
    /* if we have scopes defined and we  */
    /* have a good start scope           */
    if (scopes != OREF_NULL && startScope != TheNilObject)
    {
        /* get the scope list for the given  */
        /* starting scope                    */
        RexxArray *scopeList = (RexxArray *)scopes->get(startScope);
        if (scopeList != OREF_NULL)        /* have a matching list?             */
        {
            /* get a list of methods             */
            RexxArray *methods = methodDictionary->stringGetAll(messageName);
            size_t scopes_size = scopeList->size(); /* get the two array sizes           */
            size_t methods_size = methods->size();
            /* search through the methods list   */
            /* for the first one with a          */
            /* conforming scope                  */
            for (size_t i = 1; i <= methods_size; i++)
            {
                /* get the next method               */
                RexxMethod *method = (RexxMethod *)methods->get(i);
                /* now loop through the scopes list  */
                for (size_t j = 1; j <= scopes_size; j++)
                {
                    /* got a matching scope here?        */
                    if (scopeList->get(j) == method->getScope())
                    {
                        return method;             /* return the method                 */
                    }
                }
            }
        }
    }
    return OREF_NULL;                    /* nothing found                     */
}

void RexxBehaviour::setMethodDictionaryScope(
    RexxObject *scope)                 /* new scopy for all methods         */
/******************************************************************************/
/* Function:  Set a new set of scoping information for an object              */
/******************************************************************************/
{
    // we might not have instance methods to process
    if (methodDictionary == OREF_NULL)
    {
        return;
    }

                                         /* traverse the method dictionary    */
    for (HashLink i = methodDictionary->first();
          methodDictionary->index(i) != OREF_NULL;
          i = methodDictionary->next(i))
    {
                                         /* setting each scope                */
        ((RexxMethod *)methodDictionary->value(i))->setScope((RexxClass *)scope);
    }
}


/**
 * Extract from the method dictionary all methods defined with
 * a given scope.
 *
 * @param scope  The target scope.  If null, then all methods
 *               are returned.
 *
 * @return A supplier holding the names and methods with the target
 *         scope.  This supplier can be empty.
 */
RexxSupplier *RexxBehaviour::getMethods(RexxObject *scope)
{
    // if asking for everything, just return the supplier.
    if (scope == OREF_NULL)
    {
        return methodDictionary->supplier();
    }

    size_t count = 0;
    HashLink i;

    // travese the method dictionary, searching for methods with the target scope
    for (i = methodDictionary->first(); methodDictionary->index(i) != OREF_NULL; i = methodDictionary->next(i))
    {
        if (((RexxMethod *)methodDictionary->value(i))->getScope() == scope)
        {
            count++;
        }
    }

    RexxArray *names = new_array(count);
    RexxArray *methods = new_array(count);
    count = 1;

    // pass two, copy the entries into the array
    for (i = methodDictionary->first(); methodDictionary->index(i) != OREF_NULL; i = methodDictionary->next(i))
    {
        if (((RexxMethod *)methodDictionary->value(i))->getScope() == scope)
        {
            names->put(methodDictionary->index(i), count);
            methods->put(methodDictionary->value(i), count);
            count++;
        }
    }

    return (RexxSupplier *)new_supplier(methods, names);
}


RexxObject *RexxBehaviour::setScopes(
    RexxIdentityTable *newscopes)        /* new table of scopes               */
/******************************************************************************/
/* Function:  Set a new set of scoping information for an object              */
/******************************************************************************/
{
                                       /* set the scoping info              */
    setField(scopes, newscopes);
    return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxBehaviour::addScope(
    RexxObject *scope)                 /* new scope for the scope table     */
/******************************************************************************/
/* Function:  Set a new set of scoping information for an object              */
/******************************************************************************/
{
    if (scopes == OREF_NULL)       /* no scopes set?                     */
    {
        /* add a scope table to add to        */
        setField(scopes, new_identity_table());
    }
    /* set the scoping info              */
    scopes->add(scope, TheNilObject);
    /* add the scope list for this scope */
    scopes->add(scopes->allAt(TheNilObject), scope);
    return OREF_NULL;                    /* return the big nothing            */
}

RexxObject *RexxBehaviour::mergeScope(
    RexxObject *scope)                 /* new scope for the scope table     */
/******************************************************************************/
/* Function:  Set a new set of scoping information for an object              */
/******************************************************************************/
{
    if (checkScope(scope))         // seen this one before?
    {
        return OREF_NULL;                // we're done
    }

    return addScope(scope);        // go and add this
}


bool RexxBehaviour::checkScope(
    RexxObject *scope)                 /* scope to check                    */
/*****************************************************************************/
/* Function: Check if the passed scope is already in the scope table         */
/*****************************************************************************/
{
    if (scopes == OREF_NULL)       /* no scopes set?                    */
    {
        return false;                      /* then it can't be in the table     */
    }
    /* have the table check for the index*/
    return scopes->get(scope) != OREF_NULL;
}

void RexxBehaviour::merge(
    RexxBehaviour * source_behav)      /* new behaviour to add in           */
/*****************************************************************************/
/* Function:  Merge the passed behaviour's mdict into this behaviour's mdict */
/*            The method search order will be for the target(this) behaviour */
/*             to be found before the source behaviour                       */
/*****************************************************************************/
{
                                         /* if there isn't a source mdict     */
                                         /* there isn't anything to do        */
    if (source_behav->methodDictionary == OREF_NULL)
    {
        return;
    }
    /* if there isn't a mdict yet just   */
    /* use  the source for this one      */
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, source_behav->methodDictionary);
    }
    else
    {
        /* get a copy of the source mdict    */
        /* for the merge                     */
        RexxTable *newMethods = (RexxTable *)source_behav->methodDictionary->copy();
        ProtectedObject p(newMethods);
        /* merge this mdict with the copy    */
        methodDictionary->merge(newMethods);
        /* and put it into this behaviour    */
        setField(methodDictionary, newMethods);
    }
}

void RexxBehaviour::methodDictionaryMerge(
    RexxTable *sourceDictionary)       /* dictionary to merge in            */
/*****************************************************************************/
/* Function:  Merge the passed mdict into this behaviour's mdict             */
/*            After this merge the method search order will find the source  */
/*            mdict methods prior to self(target) methods                    */
/*****************************************************************************/
{
    RexxTable *newDictionary;            /* new method dictionary             */

                                         /* if there isn't a source mdict     */
    if (sourceDictionary == OREF_NULL)   /* there isn't anything to do        */
    {
        return;                            /* just return                       */
    }
    /* if there isn't a mdict yet just   */
    /* use  the source for this one      */
    if (methodDictionary == OREF_NULL)
    {
        setField(methodDictionary, sourceDictionary);
    }
    else
    {
        /* get a copy of the target mdict    */
        /* for the merge                     */
        newDictionary = (RexxTable *)methodDictionary->copy();
        ProtectedObject p(newDictionary);
        /* merge the source mdict and copy   */
        sourceDictionary->merge(newDictionary);
        /* and put it into this behaviour    */
        setField(methodDictionary, newDictionary);
    }
}


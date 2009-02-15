/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/* Primitive wrapper around a security manager                                */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "SecurityManager.hpp"
#include "DirectoryClass.hpp"


void *SecurityManager::operator new (size_t size)
{
                                         /* get a new method object           */
    return new_object(size, T_SecurityManager);
}


void SecurityManager::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->manager);
}


void SecurityManager::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->manager);
}

/**
 * Do a security manager check for access to the local environment.
 *
 * @param index    The name of the index.
 *
 * @return A replacement object from the security manager.
 */
RexxObject *SecurityManager::checkLocalAccess(RexxString *index)
{
    if (manager == OREF_NULL)
    {
        return OREF_NULL;
    }

    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(index, OREF_NAME);
    securityArgs->put(TheNilObject, OREF_RESULT);
    if (callSecurityManager(OREF_LOCAL, securityArgs))
    {
                                       /* get the result and return         */
        return securityArgs->fastAt(OREF_RESULT);
    }
    return OREF_NULL;   // not handled
}


/**
 * Do a security manager check for access to the global
 * environment.
 *
 * @param index    The name of the index.
 *
 * @return A replacement object from the security manager.
 */
RexxObject *SecurityManager::checkEnvironmentAccess(RexxString *index)
{
    if (manager == OREF_NULL)
    {
        return OREF_NULL;
    }

    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(index, OREF_NAME);
    securityArgs->put(TheNilObject, OREF_RESULT);
    if (callSecurityManager(OREF_ENVIRONMENT, securityArgs))
    {
                                       /* get the result and return         */
        return securityArgs->fastAt(OREF_RESULT);
    }
    return OREF_NULL;   // not handled
}


/**
 * Do the actual invocation of the security manager.
 *
 * @param methodName The method name to invoke.
 * @param arguments  The arguments to the specific method.
 *
 * @return true if the security manager overrode this, false otherwise.
 */
bool SecurityManager::callSecurityManager(RexxString *methodName, RexxDirectory *arguments)
{
    // invoke the manager
    RexxObject *resultObj = manager->sendMessage(methodName, arguments);
    if (resultObj == OREF_NULL)          /* no return result?                 */
    {
                                         /* need to raise an exception        */
        reportException(Error_No_result_object_message, methodName);
    }
    return resultObj->truthValue(Error_Logical_value_authorization);
}


/**
 * Check for permission to call a protected method.
 *
 * @param target    The target object.
 * @param messageName
 *                  The name of the message.
 * @param count     The return count.
 * @param arguments The method arguments.
 * @param result    The returned result.
 *
 * @return true if the security manager handled this call, false otherwise.
 */
bool SecurityManager::checkProtectedMethod(RexxObject *target, RexxString *messageName, size_t count, RexxObject **arguments, ProtectedObject &result)
{
    // no method here
    if (manager == OREF_NULL)
    {
        return false;
    }
    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(target, OREF_OBJECTSYM);
    securityArgs->put(messageName, OREF_NAME);
    RexxArray *argumentArray = new (count, arguments) RexxArray;
    securityArgs->put(argumentArray, OREF_ARGUMENTS);
    if (callSecurityManager(OREF_METHODNAME, securityArgs))
    {
        // get the result and return
        result = securityArgs->fastAt(OREF_RESULT);
        return true;
    }
    return false;       // not handled
}


/**
 * Check for permission to call an external function.
 *
 * @param functionName
 *                  The name of the target function.
 * @param count     The return count.
 * @param arguments The method arguments.
 * @param result    The returned result.
 *
 * @return true if the security manager handled this call, false otherwise.
 */
bool SecurityManager::checkFunctionCall(RexxString *functionName, size_t count, RexxObject **arguments, ProtectedObject &result)
{
    // no method here
    if (manager == OREF_NULL)
    {
        return false;
    }
    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(functionName, OREF_NAME);
    RexxArray *argumentArray = new (count, arguments) RexxArray;
    securityArgs->put(argumentArray, OREF_ARGUMENTS);
    if (callSecurityManager(OREF_CALL, securityArgs))
    {
        // get the result and return
        result = securityArgs->fastAt(OREF_RESULT);
        return true;
    }
    return false;       // not handled
}


/**
 * Check for permission to call an external command
 *
 * @param functionName
 *                  The name of the target function.
 * @param count     The return count.
 * @param arguments The method arguments.
 * @param result    The returned result.
 *
 * @return true if the security manager handled this call, false otherwise.
 */
bool SecurityManager::checkCommand(RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition)
{
    // no method here
    if (manager == OREF_NULL)
    {
        return false;
    }
    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);
                                       /* add the command                   */
    securityArgs->put(command, OREF_COMMAND);
    /* and the target                    */
    securityArgs->put(address, OREF_ADDRESS);
    /* did manager handle this?          */
    if (callSecurityManager(OREF_COMMAND, securityArgs))
    {
        /* get the return code               */
        result = securityArgs->fastAt(OREF_RC);
        if ((RexxObject *)result == OREF_NULL)     /* no return code provide?           */
        {
            result = IntegerZero;      /* use a zero return code            */
        }
        /* failure indicated?                */
        if (securityArgs->fastAt(OREF_FAILURENAME) != OREF_NULL)
        {
            // raise the condition when things are done
            condition = RexxActivity::createConditionObject(OREF_FAILURENAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }
        /* how about an error condition?     */
        else if (securityArgs->fastAt(OREF_ERRORNAME) != OREF_NULL)
        {
            // raise the condition when things are done
            condition = RexxActivity::createConditionObject(OREF_ERRORNAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }
        return true;
    }

    return false;       // not handled
}


/**
 * Check for stream access permission.
 *
 * @param name   The name of the stream.
 *
 * @return If the security manager handles this, the replacement stream object.
 */
RexxObject *SecurityManager::checkStreamAccess(RexxString *name)
{
    if (manager == OREF_NULL)
    {
        return OREF_NULL;
    }

    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(name, OREF_NAME);
    if (callSecurityManager(OREF_STREAM, securityArgs))
    {
        // get the result and return
        return securityArgs->fastAt(OREF_RESULT);
    }
    // not handled
    return OREF_NULL;
}


/**
 * Check for requires file access
 *
 * @param name   The name of the stream.
 *
 * @return The actual requires file name that should be loaded.  A
 *         return of OREF_NULL means access to this is not permitted.
 */
RexxString *SecurityManager::checkRequiresAccess(RexxString *name, RexxObject *&securityManager)
{
    // just return the same name if no manager object set.
    if (manager == OREF_NULL)
    {
        return name;
    }

    RexxDirectory *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

                                       /* add the program name              */
    securityArgs->put(name, OREF_NAME);
                                       /* did manager handle this?          */
    if (callSecurityManager(OREF_REQUIRES, securityArgs))
    {
        // retrieve any security manager that the security manager wants us to use for
        // a new file.
        RexxObject *secObject = securityArgs->fastAt(OREF_SECURITYMANAGER);
        if (secObject != OREF_NULL && secObject != TheNilObject)
        {
            securityManager = secObject;
        }
        // the name can be replaced by the security manager
        return (RexxString *)securityArgs->fastAt(OREF_NAME);
    }
    // not handled, return the name unchanged
    return name;
}


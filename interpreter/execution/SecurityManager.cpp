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
/* Primitive wrapper around a security manager                                */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "SecurityManager.hpp"
#include "DirectoryClass.hpp"


/**
 * Allocate a new SecurityManager object.
 *
 * @param size   The base size of the object.
 *
 * @return Storage for a security manager.
 */
void *SecurityManager::operator new (size_t size)
{
    return new_object(size, T_SecurityManager);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void SecurityManager::live(size_t liveMark)
{
    memory_mark(manager);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void SecurityManager::liveGeneral(MarkReason reason)
{
    memory_mark_general(manager);
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

    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(index, OREF_NAME);
    securityArgs->put(TheNilObject, OREF_RESULT);
    if (callSecurityManager(OREF_LOCAL, securityArgs))
    {
        // the result is returned in the arguments
        return (RexxObject *)securityArgs->get(OREF_RESULT);
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

    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(index, OREF_NAME);
    securityArgs->put(TheNilObject, OREF_RESULT);
    if (callSecurityManager(OREF_ENVIRONMENT, securityArgs))
    {
        return (RexxObject *)securityArgs->get(OREF_RESULT);
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
bool SecurityManager::callSecurityManager(RexxString *methodName, DirectoryClass *arguments)
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
    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(target, OREF_OBJECTSYM);
    securityArgs->put(messageName, OREF_NAME);
    ArrayClass *argumentArray = new_array(count, arguments);
    securityArgs->put(argumentArray, OREF_ARGUMENTS);
    if (callSecurityManager(OREF_METHODNAME, securityArgs))
    {
        // get the result and return
        result = securityArgs->get(OREF_RESULT);
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
    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(functionName, OREF_NAME);
    ArrayClass *argumentArray = new_array(count, arguments);
    securityArgs->put(argumentArray, OREF_ARGUMENTS);
    if (callSecurityManager(OREF_CALL, securityArgs))
    {
        // get the result and return
        result = securityArgs->get(OREF_RESULT);
        return true;
    }
    return false;       // not handled
}


/**
 * Check for permission to call an external command
 *
 * @param activity  The activity we're running on
 * @param address
 * @param command
 * @param result    The returned result.
 * @param condition
 *
 * @return true if the security manager handled this call, false otherwise.
 */
bool SecurityManager::checkCommand(Activity *activity, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition)
{
    // no method here
    if (manager == OREF_NULL)
    {
        return false;
    }
    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);
    // add the command and the accress target
    securityArgs->put(command, OREF_COMMAND);
    securityArgs->put(address, OREF_ADDRESS);
    // if the manager handled this, we need to decode the return stuff
    if (callSecurityManager(OREF_COMMAND, securityArgs))
    {
        result = securityArgs->get(OREF_RC);
        // if no return code received, use a zero code
        if ((RexxObject *)result == OREF_NULL)
        {
            result = IntegerZero;
        }
        // failure indicated?  Need to raise a failure condition
        if (securityArgs->get(OREF_FAILURENAME) != OREF_NULL)
        {
            // raise the condition when things are done
            condition = activity->createConditionObject(OREF_FAILURENAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }
        // same for an error condition
        else if (securityArgs->get(OREF_ERRORNAME) != OREF_NULL)
        {
            // raise the condition when things are done
            condition = activity->createConditionObject(OREF_ERRORNAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
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

    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

    securityArgs->put(name, OREF_NAME);
    if (callSecurityManager(OREF_STREAM, securityArgs))
    {
        // get the result and return
        return (RexxObject *)securityArgs->get(OREF_RESULT);
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

    DirectoryClass *securityArgs = new_directory();
    ProtectedObject p(securityArgs);

                                       /* add the program name              */
    securityArgs->put(name, OREF_NAME);
                                       /* did manager handle this?          */
    if (callSecurityManager(OREF_REQUIRES, securityArgs))
    {
        // retrieve any security manager that the security manager wants us to use for
        // a new file.
        RexxObject *secObject = (RexxObject *)securityArgs->get(OREF_SECURITYMANAGER);
        if (secObject != OREF_NULL && secObject != TheNilObject)
        {
            securityManager = secObject;
        }
        // the name can be replaced by the security manager
        return (RexxString *)securityArgs->get(OREF_NAME);
    }
    // not handled, return the name unchanged
    return name;
}


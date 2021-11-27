/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* THIS SOFTWARE IS PROVIDED BY THE COPYright HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYright   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <rexx.h>
#include <oorexxapi.h>
#include <string.h>
#include <stdio.h>

// test function for testing the function registration functions
size_t REXXENTRY TestFunction(
   const char *Name,
   size_t Argc,           /* number of arguments */
   CONSTRXSTRING Argv[],  /* list of argument strings */
   const char *Queuename, /* current queue name */
   PRXSTRING Retstr)      /* returned  */
{
    // if registered as an error tester, raise an error
    if (strcmp(Name, "TESTERROR") == 0) {
        return 40;
    }

    // return the name, count of arguments, and first argument as a return value
    sprintf(Retstr->strptr, "%s %zd %s", Name, Argc, Argv[0].strptr);
    Retstr->strlength = strlen(Retstr->strptr);
    return 0;
}



RexxMethod1(RexxObjectPtr,              // Return type
            TestCreateQueue,            // Method name
            OPTIONAL_CSTRING, qname)    // Queue name
{
    char newQueueName[MAX_QUEUE_NAME_LENGTH];
    size_t flag;

    RexxReturnCode rc = RexxCreateQueue(newQueueName, sizeof(newQueueName),
                                        qname, &flag);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    context->SetObjectVariable("FLAG", context->StringSizeToObject(flag));
    return context->NewStringFromAsciiz(newQueueName);
}

RexxMethod1(int,                        // Return type
            TestOpenQueue,              // Method name
            CSTRING, qname)             // Queue name
{
    size_t flag;

    RexxReturnCode rc = RexxOpenQueue(qname, &flag);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    context->SetObjectVariable("FLAG", context->StringSizeToObject(flag));
    return rc;
}

RexxMethod1(int,                        // Return type
            TestQueueExists,            // Method name
            CSTRING, qname)             // Queue name
{
    RexxReturnCode rc = RexxQueueExists(qname);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    return rc;
}

RexxMethod1(int,                        // Return type
            TestDeleteQueue,            // Method name
            CSTRING, qname)             // Queue name
{
    RexxReturnCode rc = RexxDeleteQueue(qname);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    return rc;
}

RexxMethod1(int,                        // Return type
            TestQueryQueue,             // Method name
            CSTRING, qname)             // Queue name
{
    size_t count;

    RexxReturnCode rc = RexxQueryQueue(qname, &count);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    context->SetObjectVariable("FLAG", context->StringSizeToObject(count));
    return rc;
}

RexxMethod3(int,                        // Return type
            TestAddQueue,               // Method name
            CSTRING, qname,             // Queue name
            RexxStringObject, data,     // Queue data to add
            int, type)                  // Queue FIFO/LIFO flag
{
    CONSTRXSTRING rxdata;

    MAKERXSTRING(rxdata, context->StringData(data), context->StringLength(data));
    RexxReturnCode rc = RexxAddQueue(qname, &rxdata, type);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    return rc;
}

RexxMethod1(int,                        // Return type
            TestPullFromQueue,          // Method name
            CSTRING, qname)             // Queue name
{
    RXSTRING data;
    RexxQueueTime timestamp;

    MAKERXSTRING(data, NULL, 0);

    RexxReturnCode rc = RexxPullFromQueue(qname, &data, &timestamp, 0);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    if (rc == RXQUEUE_OK)
    {
        context->SetObjectVariable("FLAG", context->NewString(data.strptr, data.strlength));

        char stamp[1024];
        // the time stamp comes from a struct tm
        // year is years since 1900
        // month is 0 .. 11
        // hundredths and microseconds are not available
        sprintf(stamp, "%d-%d-%d %d:%d:%d (%d, %d)",
          1900 + timestamp.year, 1 + timestamp.month, timestamp.day,
          timestamp.hours, timestamp.minutes, timestamp.seconds,
          timestamp.yearday, timestamp.weekday);
        context->SetObjectVariable("TIMESTAMP", context->NewString(stamp, strlen(stamp)));
    }
    return rc;
}

RexxMethod1(int,                        // Return type
            TestClearQueue,             // Method name
            CSTRING, qname)             // Queue name
{
    RexxReturnCode rc = RexxClearQueue(qname);
    context->SetObjectVariable("RETC", context->Int32ToObject(rc));
    return rc;
}

RexxMethod1(int,                        // Return type
            TestAllocateFreeMemory,     // Method name
            int, size)                  // Size of memory block
{
    void *block = RexxAllocateMemory(size);
    if (block == NULL) {
        return 1;
    }
    return RexxFreeMemory(block);
}

RexxMethod0(int,                        // Return type
            TestMVariablePool)          // Method name
{
    RexxReturnCode retc = RexxVariablePool(NULL);
    return retc;
}

RexxRoutine1(int,                       // Return type
            TestFVariablePool,          // Function name
            RexxArrayObject, arr)       // Array of shvblocks
{
    RexxReturnCode retc;
    size_t members = context->ArrayItems(arr);
    size_t ctr;
    PSHVBLOCK blocks = NULL;
    PSHVBLOCK nextblock = NULL;
    PSHVBLOCK currentblock = NULL;
    unsigned int tempint;

    if (members == 0) {
        return RXSHV_NOAVL;
    }

    // set up the shvblocks from the array
    for (ctr = 1; ctr <= members; ctr++) {
        nextblock = (PSHVBLOCK)malloc(sizeof(SHVBLOCK));
        if (currentblock != NULL) {
            currentblock->shvnext = nextblock;
        }
        currentblock = nextblock;
        if (blocks == NULL) {
            blocks = currentblock;
        }
        currentblock->shvnext = NULL;
        RexxObjectPtr entry = context->ArrayAt(arr, ctr);
        RexxObjectPtr val = context->SendMessage0(entry, "shvcode");
        context->ObjectToUnsignedInt32(val, &tempint);
        currentblock->shvcode = (unsigned char)tempint;
        val = context->SendMessage0(entry, "shvret");
        context->ObjectToUnsignedInt32(val, &tempint);
        currentblock->shvret = (unsigned char)tempint;
        val = context->SendMessage0(entry, "shvnamelen");
        context->ObjectToUnsignedInt32(val, &tempint);
        currentblock->shvnamelen = (size_t)tempint;
        val = context->SendMessage0(entry, "shvvaluelen");
        context->ObjectToUnsignedInt32(val, &tempint);
        currentblock->shvvaluelen = (size_t)tempint;
        switch (currentblock->shvcode) {
        case RXSHV_SET:
        case RXSHV_SYSET:
        {
            val = context->SendMessage0(entry, "shvname");
            currentblock->shvname.strptr = (char*)context->ObjectToStringValue(val);
            currentblock->shvname.strlength = strlen(currentblock->shvname.strptr);
            val = context->SendMessage0(entry, "shvvalue");
            size_t len = strlen(context->ObjectToStringValue(val)) + 1;
            currentblock->shvvalue.strptr = (char *)malloc(len);
            strncpy(currentblock->shvvalue.strptr, context->ObjectToStringValue(val), len);
            currentblock->shvvalue.strlength = len - 1;
            break;
        }
        case RXSHV_FETCH:
        case RXSHV_SYFET:
            val = context->SendMessage0(entry, "shvname");
            currentblock->shvname.strptr = (char*)context->ObjectToStringValue(val);
            currentblock->shvname.strlength = strlen(currentblock->shvname.strptr);
            currentblock->shvvalue.strptr = (char*)malloc(currentblock->shvvaluelen + 1);
            currentblock->shvvalue.strlength = currentblock->shvvaluelen + 1;
            break;
        case RXSHV_DROPV:
        case RXSHV_SYDRO:
            val = context->SendMessage0(entry, "shvname");
            currentblock->shvname.strptr = (char*)context->ObjectToStringValue(val);
            currentblock->shvname.strlength = strlen(currentblock->shvname.strptr);
            currentblock->shvvalue.strptr = NULL;
            currentblock->shvvalue.strlength = 0;
            break;
        case RXSHV_PRIV:
            val = context->SendMessage0(entry, "shvname");
            currentblock->shvname.strptr = (char*)context->ObjectToStringValue(val);
            currentblock->shvname.strlength = strlen(currentblock->shvname.strptr);
            currentblock->shvvalue.strptr = NULL;
            currentblock->shvvalue.strlength = 0;
            break;
        default:
            free(currentblock);
            return RXSHV_NOAVL;
        }
    }

    // call the variable pool interface
    retc = RexxVariablePool(blocks);

    // set the array to the shvblocks
    currentblock = blocks;
    for (ctr = 1; ctr <= members; ctr++) {
        RexxObjectPtr entry = context->ArrayAt(arr, ctr);
        context->SendMessage1(entry, "shvret=", context->UnsignedInt32ToObject((uint32_t)currentblock->shvret));
        switch (currentblock->shvcode) {
        case RXSHV_SET:
        case RXSHV_SYSET:
            free(currentblock->shvvalue.strptr);
            break;
        case RXSHV_FETCH:
        case RXSHV_SYFET:
            context->SendMessage1(entry, "shvvalue=", context->NewStringFromAsciiz(currentblock->shvvalue.strptr));
            context->SendMessage1(entry, "shvvaluelen=", context->UnsignedInt32ToObject((uint32_t)currentblock->shvvalue.strlength));
            free(currentblock->shvvalue.strptr);
            break;
        case RXSHV_DROPV:
        case RXSHV_SYDRO:
            break;
        case RXSHV_PRIV:
            if (currentblock->shvret == 0) {
                context->SendMessage1(entry, "shvvalue=", context->NewStringFromAsciiz(currentblock->shvvalue.strptr));
                context->SendMessage1(entry, "shvvaluelen=", context->UnsignedInt32ToObject((uint32_t)currentblock->shvvalue.strlength));
                // this memory must be freed this way since it was allocated with RexxAllocateMemeory
                RexxFreeMemory(currentblock->shvvalue.strptr);
            }
            else {
                context->SendMessage1(entry, "shvvalue=", context->NewStringFromAsciiz("\0"));
                context->SendMessage1(entry, "shvvaluelen=", context->WholeNumberToObject(0));
            }
            break;
        default:
            break;
        }
        nextblock = currentblock->shvnext;
        free(currentblock);
        currentblock = nextblock;
    }

    return retc;
}

RexxRoutine0(size_t,                    // Return type
            TestFNVariablePool)         // Function name
{
    RexxReturnCode retc = 0;
    size_t ctr = 0;
    SHVBLOCK block;

    while (retc != RXSHV_LVAR) {
        block.shvnext = NULL;
        block.shvname.strptr = NULL;
        block.shvname.strlength = 0;
        block.shvvalue.strptr = NULL;
        block.shvvalue.strlength = 0;
        block.shvnamelen = 0;
        block.shvvaluelen = 0;
        block.shvcode = RXSHV_NEXTV;
        block.shvret = 0;
        retc = RexxVariablePool(&block);
        if (retc != 0 && retc != RXSHV_LVAR) {
            return -1; // indicate an error
        }
        ctr++;
        if (block.shvname.strptr != NULL) {
            // this memory must be freed this way since it was allocated with RexxAllocateMemeory
            RexxFreeMemory((void *)block.shvname.strptr);
        }
        if (block.shvvalue.strptr != NULL) {
            // this memory must be freed this way since it was allocated with RexxAllocateMemeory
            RexxFreeMemory(block.shvvalue.strptr);
        }
    }

    return ctr;
}

RexxMethod3(int,                        // Return type
            TestAddMacro,               // Method name
            CSTRING, name,              // Macro name
            CSTRING, filename,          // Macro file name
            size_t, srchpos)            // Search position
{
    RexxReturnCode retc = RexxAddMacro(name, filename, srchpos);
    return retc;
}

RexxMethod1(int,                        // Return type
            TestDropMacro,              // Method name
            CSTRING, name)              // Macro name
{
    RexxReturnCode retc = RexxDropMacro(name);
    return retc;
}

RexxMethod2(int,                        // Return type
            TestSaveMacroSpace,         // Method name
            RexxArrayObject, names,     // Array of macro names
            CSTRING, filename)          // Macro file name
{
    size_t argc = context->ArrayItems(names);
    const char **cnames = (const char **)calloc(argc, sizeof(const char *));

    for (size_t i = 1; i <= argc; i++) {
        cnames[i - 1] = context->ObjectToStringValue(context->ArrayAt(names, i));
    }

    RexxReturnCode retc = RexxSaveMacroSpace(argc, cnames, filename);
    free(cnames);
    return retc;
}

RexxMethod2(int,                        // Return type
            TestLoadMacroSpace,         // Method name
            RexxArrayObject, names,     // Array of macro names
            CSTRING, filename)          // Macro file name
{
    size_t argc = context->ArrayItems(names);
    const char **cnames = (const char **)calloc(argc, sizeof(const char *));

    for (size_t i = 1; i <= argc; i++) {
        cnames[i - 1] = context->ObjectToStringValue(context->ArrayAt(names, i));
    }

    RexxReturnCode retc = RexxLoadMacroSpace(argc, cnames, filename);
    free(cnames);
    return retc;
}

RexxMethod1(int,                        // Return type
            TestQueryMacro,             // Method name
            CSTRING, name)              // Macro name
{
    unsigned short pos;

    RexxReturnCode retc = RexxQueryMacro(name, &pos);
    return retc;
}

RexxMethod2(int,                        // Return type
            TestReorderMacro,           // Method name
            CSTRING, name,              // Macro name
            size_t, pos)                // New position
{
    RexxReturnCode retc = RexxReorderMacro(name, pos);
    return retc;
}

RexxMethod0(int,                        // Return type
            TestClearMacroSpace)        // Method name
{
    RexxReturnCode retc = RexxClearMacroSpace();
    return retc;
}

RexxMethod1(int,                        // Return type
            TestRegisterFunctionExe,    // Method name
            CSTRING, name)              // function name
{
    RexxReturnCode retc = RexxRegisterFunctionExe(name, (REXXPFN)TestFunction);
    return retc;
}

RexxMethod1(int,                        // Return type
            TestDeregisterFunction,    // Method name
            CSTRING, name)              // function name
{
    RexxReturnCode retc = RexxDeregisterFunction(name);
    return retc;
}

RexxMethod1(int,                        // Return type
            TestQueryFunction,    // Method name
            CSTRING, name)              // function name
{
    RexxReturnCode retc = RexxQueryFunction(name);
    return retc;
}


RexxMethodEntry orxtest_methods[] = {
    REXX_METHOD(TestCreateQueue,        TestCreateQueue),
    REXX_METHOD(TestOpenQueue,          TestOpenQueue),
    REXX_METHOD(TestQueueExists,        TestQueueExists),
    REXX_METHOD(TestDeleteQueue,        TestDeleteQueue),
    REXX_METHOD(TestQueryQueue,         TestQueryQueue),
    REXX_METHOD(TestAddQueue,           TestAddQueue),
    REXX_METHOD(TestPullFromQueue,      TestPullFromQueue),
    REXX_METHOD(TestClearQueue,         TestClearQueue),
    REXX_METHOD(TestAllocateFreeMemory, TestAllocateFreeMemory),
    REXX_METHOD(TestMVariablePool,      TestMVariablePool),
    REXX_METHOD(TestAddMacro,           TestAddMacro),
    REXX_METHOD(TestDropMacro,          TestDropMacro),
    REXX_METHOD(TestSaveMacroSpace,     TestSaveMacroSpace),
    REXX_METHOD(TestLoadMacroSpace,     TestLoadMacroSpace),
    REXX_METHOD(TestQueryMacro,         TestQueryMacro),
    REXX_METHOD(TestReorderMacro,       TestReorderMacro),
    REXX_METHOD(TestClearMacroSpace,    TestClearMacroSpace),
    REXX_METHOD(TestRegisterFunctionExe,TestRegisterFunctionExe),
    REXX_METHOD(TestDeregisterFunction, TestDeregisterFunction),
    REXX_METHOD(TestQueryFunction, TestQueryFunction),
    REXX_LAST_METHOD()
};


RexxRoutineEntry orxtest_routines[] = {
    REXX_TYPED_ROUTINE(TestFVariablePool,  TestFVariablePool),
    REXX_TYPED_ROUTINE(TestFNVariablePool, TestFNVariablePool),
    REXX_LAST_ROUTINE()
};


RexxPackageEntry UnitTest_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "UnitTest",                          // name of the package
    "1.0.0",                             // package information
    NULL,                                // no load/unload functions
    NULL,
    orxtest_routines,                    // the exported routines
    orxtest_methods                      // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(UnitTest);


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2017 Rexx Language Association. All rights reserved.    */
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
#ifndef orxexits_included
#define orxexits_included

#include "oorexxapi.h"
#include <string.h>

class InstanceInfo
{
public:
    inline void * operator new(size_t size, void *objectPtr) { return objectPtr; };
    inline void  operator delete(void *, void *) {;}

    typedef enum
    {
        DISABLED = 0,
        SKIP = 1,
        EXIT_ERROR = 2,
        RAISE = 3,
        HANDLE = 4,
        TRACEON = 5,    // extra actions for specific exit types
        TRACEOFF = 6,
        HALT = 7,
        NOHALT = 8,
        ALL = 9,
        CONSOLE = 10,
        CONSOLE_DEBUG = 11,
    } ExitAction;

    typedef enum
    {
        NONE = 0,
        CONTEXT = 1,
        REGISTERED_DLL = 2,
        REGISTERED_EXE = 3,
    } ExitType;

    typedef enum
    {
        INSTANCE = 0,
        REXXSTART = 1
    } CallType;

    class ExitConfig
    {
        public:
            inline ExitConfig() : action(DISABLED) { }
            inline ExitConfig & operator=(ExitAction a) { action = a; return *this; }
            ExitConfig & operator=(const char *a)
            {
                if (strcmp("DISABLED", a) == 0)
                {
                    action = DISABLED;
                }
                else if (strcmp("SKIP", a) == 0)
                {
                    action = SKIP;
                }
                else if (strcmp("ERROR", a) == 0)
                {
                    action = EXIT_ERROR;
                }
                else if (strcmp("RAISE", a) == 0)
                {
                    action = RAISE;
                }
                else if (strcmp("HANDLE", a) == 0)
                {
                    action = HANDLE;
                }
                else if (strcmp("TRACEON", a) == 0)
                {
                    action = TRACEON;
                }
                else if (strcmp("TRACEOFF", a) == 0)
                {
                    action = TRACEOFF;
                }
                else if (strcmp("HALT", a) == 0)
                {
                    action = HALT;
                }
                else if (strcmp("NOHALT", a) == 0)
                {
                    action = NOHALT;
                }
                else if (strcmp("ALL", a) == 0)
                {
                    action = ALL;
                }
                else if (strcmp("CONSOLE", a) == 0)
                {
                    action = CONSOLE;
                }
                else if (strcmp("DEBUG", a) == 0)
                {
                    action = CONSOLE_DEBUG;
                }
                else
                {
                    action = DISABLED;
                }
                return *this;
            }

            inline bool operator == (ExitAction a)
            {
                return action == a;
            }

            inline bool operator != (ExitAction a)
            {
                return action != a;
            }

            // cast conversion operators for some very common uses of protected object.
            operator const char *()
            {
                switch (action)
                {
                    case DISABLED:
                        return "DISABLED";
                    case SKIP:
                        return "SKIP";
                    case EXIT_ERROR:
                        return "ERROR";
                    case RAISE:
                        return "RAISE";
                    case HANDLE:
                        return "HANDLE";
                    case TRACEON:
                        return "TRACEON";
                    case TRACEOFF:
                        return "TRACEOFF";
                    case HALT:
                        return "HALT";
                    case NOHALT:
                        return "NOHALT";
                    case ALL:
                        return "ALL";
                    case CONSOLE:
                        return "CONSOLE";
                    case CONSOLE_DEBUG:
                        return "DEBUG";
                }
                return "DISABLED";
            }
            inline bool isEnabled() { return action != DISABLED; }

            ExitAction action;
    };

    InstanceInfo() : callType(INSTANCE), exitStyle(NONE), programName(NULL), extensionPath(NULL), initialAddress(NULL),
        extensions(NULL), loadLibrary(NULL), fnc(), cmd(), msq(), sio(), hlt(), trc(), ini(),
        ter(), exf(), var(), val(), ofnc(), code(0), rc(0), argCount(0) { }

    CallType callType;

    // the type of exit use
    ExitType exitStyle;
    // the program name
    const char *programName;
    // additional search path
    const char *extensionPath;
    // the initial address environment
    const char *initialAddress;
    // external function seach extensions
    const char *extensions;
    // an initial library to load
    const char *loadLibrary;

    ExitConfig fnc;
    ExitConfig cmd;
    ExitConfig msq;
    ExitConfig sio;
    ExitConfig hlt;
    ExitConfig trc;
    ExitConfig ini;
    ExitConfig ter;
    ExitConfig exf;
    ExitConfig var;
    ExitConfig val;
    ExitConfig ofnc;

    wholenumber_t code;
    wholenumber_t rc;

    size_t argCount;
    const char *arguments[10];
    char returnResult[1024];
};

void REXXENTRY deregisterExits();
void REXXENTRY registerExeExits(void *data);
void REXXENTRY registerDllExits(void *data);
void REXXENTRY registerSubcomHandler(void *data);
void REXXENTRY deregisterSubcomHandler();
bool REXXENTRY buildRegisteredExitList(InstanceInfo *instanceInfo, RXSYSEXIT *exitList);
RexxReturnCode REXXENTRY createInstance(InstanceInfo *instanceInfo, RexxInstance *&instance, RexxThreadContext *&threadContext);
bool REXXENTRY buildContextExitList(InstanceInfo *instanceInfo, RexxContextExit *exitList);
void REXXENTRY invokeProgram(InstanceInfo *instanceInfo);
void REXXENTRY invokeRexxStart(InstanceInfo *instanceInfo);
void REXXENTRY buildInfo(InstanceInfo *instanceInfo, int argc, char **argv);

#endif

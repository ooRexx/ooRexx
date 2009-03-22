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
/*****************************************************************************/
/* REXX Windows Support                                                      */
/*                                                                           */
/* Main Unix interpreter control.  This is the preferred location for        */
/* all platform dependent global variables.                                  */
/* The interpreter does not instantiate an instance of this                  */
/* class, so most variables and methods should be static.                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#ifndef SystemInterpreter_Included
#define SystemInterpreter_Included

#include "RexxCore.h"

#if defined( HAVE_SIGNAL_H )
# include <signal.h>
#endif

#if defined( HAVE_SYS_SIGNAL_H )
# include <sys/signal.h>
#endif


class InterpreterInstance;
class RexxActivation;
class RexxDateTime;
class RexxBuffer;

class SystemInterpreter
{
public:
    static void live(size_t);
    static void liveGeneral(int reason);

    static void processStartup();
    static void processShutdown();

    static void startInterpreter();
    static void terminateInterpreter();

    static void initializeInstance(InterpreterInstance *instance);
    static void terminateInstance(InterpreterInstance *instance);
    static void getCurrentWorkingDirectory(char *);

    static RexxObject *popEnvironment(RexxActivation *context);
    static RexxObject *pushEnvironment(RexxActivation *context);
    static void restoreEnvironment(void *CurrentEnv);
    static RexxObject *buildEnvlist();
    static RexxString *qualifyFileSystemName(RexxString *name);
    static void getCurrentTime(RexxDateTime *Date );
    static RexxString *getSystemName();
    static RexxString *getSystemVersion();
    static RexxString *getInternalSystemName();
    static RexxString *getSourceString(RexxString *callType, RexxString * programName);
    static RexxString *getUserid();
    static void releaseResultMemory(void *);
    static void *allocateResultMemory(size_t);
    static RexxString *getMessageHeader(wholenumber_t code);
    static RexxString *getMessageText(wholenumber_t code);
    static bool valueFunction(RexxString *name, RexxObject *newValue, RexxString *selector, RexxObject *&result);
    static RexxString *getDefaultAddressName();
    static bool invokeExternalFunction(RexxActivation *, RexxActivity *, RexxString *, RexxObject **, size_t, RexxString *, ProtectedObject &);
    static void validateAddressName(RexxString *name );
    static void setupProgram(RexxActivation *activation);
    static void loadImage(char **imageBuffer, size_t *imageSize);
    static RexxBuffer *readProgram(const char *file_name);

    static sigset_t oldmask;       // masks used for setting signal handlers
    static sigset_t newmask;

};

#endif

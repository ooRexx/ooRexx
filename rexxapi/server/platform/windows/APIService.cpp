/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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

#include "APIServer.hpp"
#include "SysCSNamedPipeStream.hpp"
#include "stdio.h"



/* - - - - Useful debugging help, if'd out under normal circumstances- - - - -*/


APIServer apiServer;             // the real server instance


/**
 * The main entry point for rxapi.exe.
 *
 * When rxapi is installed as a service, the invoker might be the service
 * control manager, or the interpreter could be starting rxapi as a service
 * because rxapi was not running.
 *
 * When rxapi is not installed as a service, then the invoker is most likely the
 * interpreter starting up rxapi.  Although, the user could also be invoking
 * rxapi from the command line.
 *
 * The third possibility is that rxapi is being invoked from the command line
 * with arguments to either install, uninstall, or query rxapi as a service.
 *
 *
 * @param hInstance
 * @param hPrevInstance
 * @param lpCmdLine       The only arg we are interested in, the command line
 *                        arguments.
 * @param nCmdShow
 *
 * @return 0
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        // create a connection object that will be the server target
        SysServerNamedPipeConnectionManager *c = new SysServerNamedPipeConnectionManager();
        // try to create the named pipe used for this server. If this fails, we
        // likely have a instance of the daemon already running, so just fail quietly.
        if (!c->bind())
        {
            delete c;
            return ERROR_ACCESS_DENIED;
        }
        apiServer.initServer(c);              // start up the server
        apiServer.listenForConnections();     // go into the message loop
    }
    catch (ServiceException *e)
    {
        // we don't need this, so delete it now
        delete e;
    }
    apiServer.terminateServer();     // shut everything down

    return 0;
}

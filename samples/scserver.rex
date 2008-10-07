#!/usr/bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Simple socket server using socket class                       */
/*                                                                            */
/* Copyright (c) 2007-2008 Rexx Language Association. All rights reserved.    */
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
/* Author: David Ruggles                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

srv = .myserver~new()
srv~listen()

::requires 'socket.cls'

::class myserver

::method init
    expose sock

/*  instaniate an instance of the socket class  */
    sock = .socket~new()

::method monitor unguarded
    expose sock

    say 'Press Ctrl-C To Shutdown'
    do forever
        if sysgetkey('noecho')~c2x() = '03' then leave
    end

/*  close the socket  */
    if sock~close() < 0 then
        say 'SockClose Failed'

::method listen
    expose sock

/*  instaniate an instance of the inetaddress class
    with the host information of the server we will
    contact: localhost and port 726578
    we use the "gethostid" class method of the socket
    class to determine the localhost address  */
    host = .inetaddress~new(.socket~gethostid(), '726578')

/*  bind to the host information  */
    if sock~bind(host) < 0 then do
        say 'Bind Failed'
        exit
    end

    if sock~listen(256) < 0 then do
        say 'Listen Failed'
        exit
    end

    self~start('monitor')   --  this will allow the server to be shutdown cleanly

    do forever
        csock = sock~accept()   --  prepare to accept a new client
        if .nil = csock then leave  --  if the socket is closed (by monitor) sockaccept will return .nil
    /*  this will spawn a thread to handle the new client and then return to accept the next client  */
        self~start('respond', csock)
    end

::method respond unguarded
    use arg sock

    do forever
        /*  get data from the client  */
        data = sock~recv(1024)
        if data = .nil then leave
        /*  echo that data back to the client  */
        sock~send('Echo:' data)
    end

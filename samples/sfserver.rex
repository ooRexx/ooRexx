#!/usr/bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Simple socket server using socket function package            */
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

::class myserver

::method init
    expose socket shutdown

/*  create a socket  */
    socket = socksocket('AF_INET', 'SOCK_STREAM', '0')

    shutdown = .false

::method monitor unguarded
    expose socket shutdown

/*  this seems to be the only cross platform way of cleanly shutting down.
    this may not be the best method of shutting down, but does work on both
    Linux and Windows  */
    say 'Press [Enter] To Shutdown'
    pull .

    shutdown = .true

/*  create a socket (to connect to ourselves)  */
    socket = socksocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')

/*  specify the host we will connect to  */
    host.!family = 'AF_INET'
    host.!addr = SockGetHostId()
    host.!port = '726578'

/*  connect to the server (if it hasn't already shutdown)  */
    if sockconnect(socket, 'host.!') < 0 then
    /*  close the socket connection  */
        call sockclose(socket)


::method listen
    expose socket shutdown

/*  specify the host we will run as  */
    host.!family = 'AF_INET'        --  Protocol family (only AF_INET is supported)
    host.!addr = SockGetHostId()    --  IP address (use the sockgethostid function to get address of the localhost)
    host.!port = '726578'           --  Port number

/*  bind to the host information  */
    if sockbind(socket, 'host.!') < 0 then do
        say 'SockBind Failed'
        exit
    end

/*  start listening for new connections  */
    if socklisten(socket, 256) < 0 then do
        say 'SockListen Failed'
        exit
    end

    self~start('monitor')   --  this will allow the server to be shutdown cleanly

    do forever
        cs = sockaccept(socket)   --  prepare to accept a new client
        if cs = -1 | shutdown then leave   --  if the socket is closed (by monitor) sockaccept will fail
    /*  this will spawn a thread to handle the new client and then return to accept the next client  */
        self~start('respond', cs)
    end

    if cs <> -1 then
        if sockclose(cs) < 0 then
            say 'SockClose Failed'

    if sockclose(socket) < 0 then
        say 'SockClose Failed'

::method respond unguarded
    use arg socket

    do forever
        /*  get data from the client  */
        if sockrecv(socket, 'data', 1024) < 1 then leave
        /*  echo that data back to the client  */
        call socksend socket, 'Echo:' data
    end

::requires 'rxsock' LIBRARY

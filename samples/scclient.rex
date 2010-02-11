#!/usr/bin/rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description: Simple socket client using socket class                       */
/*                                                                            */
/* Copyright (c) 2007-2010 Rexx Language Association. All rights reserved.    */
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
/* Author: David Ruggles                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*  instaniate an instance of the socket class  */
    sock = .socket~new()

/*  instaniate an instance of the inetaddress class
    with the host information of the server we will
    contact: localhost and port 726578
    we use the "gethostid" class method of the socket
    class to determine the localhost address  */
    host = .inetaddress~new(.socket~gethostid(), '726578')

/*  connect to the server  */
    if sock~connect(host) < 0 then do
        say 'Connect Failed'
        exit
    end

    say 'type "X" to exit'
    do forever
        call charout , 'Send To Server: '
        parse pull message
        if message~upper() = 'X' then leave
    /*  send message to server  */
        if sock~send(message) < 0 then do
            say 'Send Failed'
            leave
        end
    /*  get message from server  */
        ret = sock~recv(1024)
        if ret = .nil then do
            if sock~errno() < 0 then
                say 'Recv Failed'
            else
                say 'Socket Closed'
            leave
        end
        say 'Server Responded:' ret
    end

/*  close the socket connection  */
    if sock~close() < 0 then
        say 'SockClose Failed'

::requires 'socket.cls'
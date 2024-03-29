#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2007-2023 Rexx Language Association. All rights reserved.    */
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

/* Simple socket client using socket function package                         */

/*  create a socket  */
    socket = socksocket('AF_INET', 'SOCK_STREAM', 'IPPROTO_TCP')

/*  specify the host we will connect to  */
    host.!family = 'AF_INET'        --  Protocol family (only AF_INET is supported)
    host.!addr = '127.0.0.1'        --  localhost IP address
    host.!port = '50010'            --  Port number

/*  connect to the server  */
    if sockconnect(socket, 'host.!') < 0 then do
        say 'SockConnect failed:' errno
        exit
    end

    say 'type "X" to exit'
    do forever
        call charout , 'Send To Server: '
        parse pull message
        if message~upper() = 'X' then leave
    /*  send message to server  */
        if socksend(socket, message) < 0 then do
            say 'SockSend failed:' errno
            leave
        end
    /*  get message from server  */
        ret = sockrecv(socket, 'data', 1024)
        if ret < 1 then do
            if ret < 0 then
                say 'SockRecv failed:' errno
            else
                say 'Socket closed:' errno
            leave
        end
        say 'Server responded:' data
    end

/*  close the socket connection  */
    if sockclose(socket) < 0 then
        say 'SockClose failed:' errno

::requires 'rxsock' LIBRARY

  /*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/**********************************************************************/
/*                                                                    */
/* MSOutlook.REX: OLE Automation with Object REXX                     */
/*                                                                    */
/* Open MSOutlook and show inbox. Count mails in inbox.               */
/* Show sender of all mails in inbox. Wait for 10 seconds.            */
/*                                                                    */
/**********************************************************************/

  Outlook = .OLEObject~new("Outlook.Application")
  NameSpace = Outlook~GetNamespace("MAPI")

  -- In Outlook folder have numbers:
  -- DeletedItems (3), Outbox (4), SentMail (5), Inbox (6),
  -- Calendar (9), Contacts (10), Journal (11), Notes (12),
  -- Tasks (13)
  Inbox = NameSpace~GetDefaultFolder("6")      -- selects Inbox

  -- makes Outlook visible and shows Inbox
  Inbox~Display

  InboxItems = Inbox~Items
  MailCount = InboxItems~Count                      -- count items in Inbox
  say "You have" MailCount "Mail(s) in your Inbox:"
  -- may require to allow access to email information
  Do ItemNumber = 1 to MailCount                    -- go through each item
    Item = Inbox~Items(ItemNumber)
    Sender = Item~Sender~Name                       -- sender of item
    say "#" ItemNumber"." Sender
  end

Call SysSleep 10

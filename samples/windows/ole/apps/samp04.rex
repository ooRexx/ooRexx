/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/* SAMP04.REX: OLE Automation with Open Object REXX - Sample 4        */
/*                                                                    */
/* Create a mail message in Lotus Notes and send it to a number of    */
/* recipients automatically.                                          */
/*                                                                    */
/**********************************************************************/

say "Please enter your name"
parse pull yourName

/* create an array of the recipients */
Recipients = .array~new

say "Please enter a list of recipients (email addresses). Press enter ",
    "after each entry (end list with 'Q')."

i = 0
do until answer~translate == "Q"
  parse pull answer
  if answer~translate \= "Q" then do
    i = i + 1
    Recipients[i] = answer
  end
end

/* Create Notes object */
Session = .OLEObject~New("Notes.NotesSession")
MailServer = Session~GetEnvironmentString("MailServer", .True)
MailFile = Session~GetEnvironmentString("MailFile", .True)
MailDb = Session~GetDatabase(MailServer, MailFile)

Say "Creating mail to be sent to" i "recipients..."
MailDoc = MailDb~CreateDocument
MailDoc~Form = "Memo"
MailDoc~Logo = "StdNotesLtr9"
MailDoc~From = yourName
MailDoc~Subject = "Rexx OLE automation test mail"

/* create a new body text with multiple lines */
NewBody = MailDoc~CreateRichTextItem("Body")
NewBody~AppendText("To the readers of this mail message:")
NewBody~AddNewLine(2)
NewBody~AppendText("This mail has been sent with Open Object Rexx for Windows.")
NewBody~AddNewLine(1)
NewBody~AppendText("It was created automatically at" Time("N") "on" Date("N"))
NewBody~AppendText(" and then sent without any user interacting with the program.")

MailDoc~SendTo = Recipients
MailDoc~Save(.False, .False)
MailDoc~Send(.False, Recipients)

Say "Mail has been sent"

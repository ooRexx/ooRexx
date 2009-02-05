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
/****************************************************************************/
/* Name: REGISTRY.REX                                                       */
/* Type: Open Object Rexx Script                                            */
/*                                                                          */
/* Description:                                                             */
/* This examples demonstrates the use of the WindowsRegistry class to read  */
/* and manipulate the Windows registry                                      */
/*                                                                          */
/****************************************************************************/

/* Get system and version */
ret = syswinver()
parse var ret winsys winver

/* The registry editor name is different on NT 3.5 */
if winver \= 4 then
    regeditor = "REGEDT32"
else regeditor = "REGEDIT"

FileControl = "\control"
FileNewKey = "\testuser"
/* do not use file extension on 95 */
if winsys = "WindowsNT" then do
    FileControl = FileControl || ".reg"
    FileNewKey = FileNewKey || ".reg"
end

r = .WindowsRegistry~new   /* create a new registry object */

/* r~Current_Key always refers to the most recently opened or created key
   and is used if the keyhandle argument is omitted. r~Current_Key is initially
   set to HKEY_LOCAL_MACHINE. */

/* Open the HKEY_LOCAL_MACHINE\System key. On NT it is not possible to open
   this key with ALL access rights */
if r~open(,"SYSTEM","QUERY WRITE") \= 0 then do
   syskey = r~Current_Key
   if r~List(,keys.) = 0 then do   /* Get a list of System's subkeys */
      bk = r~open(,keys.1||"\Control") /* open System's first subkey's Control key */
      if bk \= 0 then do

         q. = r~Query     /* Get and displeay information about the Control key. */
         say "Control is of class" q.class", was last modified" q.date "at" q.time,
         || ", has" q.subkeys "subkeys, and" q.values "values."

         if r~List(,vals.) = 0 then /* list all subkeys of Control */
         do i=1 to q.subkeys
             say vals.i
         end
         say "-----"
         drop vals.
         drop name data type   /* these 3 symbols must be uninitialized */

         /* list all values of the first subkey */
         if r~ListValues(,vals.) = 0 then do i = 1 to q.values
            say vals.i.name "=" vals.i.data "("vals.i.type")"
         end
         /* save HKEY_LOCAL_MACHINE\System\XXX\Control to a file */
         saveret = r~Save(bk,FileControl)
         if saveret = 183 then say "The file" FileControl "already exists"
         else say "HKEY_LOCAL_MACHINE\System\"keys.1"\Control has been saved to" FileControl
      end
      r~Close(bk)  /* close Control key */
   end
   r~Close(syskey)   /* close System key */
   say "-----"
end

/* open the HKEY_LOCAL_MACHINE\SOFTWARE key. */
if r~open(r~Local_Machine,"SOFTWARE") \= 0 then do
   /* Get a list of ...\Software's subkeys */
   kinfo. = r~query  /* r~Current_Key is used again */
   say "class name =" kinfo.class
   say "subkeys =" kinfo.subkeys
   say "values =" kinfo.values
   say "date =" kinfo.date
   say "time =" kinfo.time
   say "-- list --"
   if r~List(,keys.) = 0 then do i over keys.
      say keys.i
   end
   say "-----"
   r~Close  /* close HKEY_LOCAL_MACHINE\SOFTWARE */
end

/* create a new subkey under HKEY_CURRENT_USER and modify it */
savekey = 0
if r~create(r~Current_User,"TEST_USER") \= 0 then do

   savekey = r~Current_Key  /* Current_Key was set to HKEY_CURRENT_USER\TEST_USER */
   r~setvalue(,"","Test User")  /* set the default value */

   /* add other values */
   r~setvalue(,"PROFESSION","Developer")
   r~setvalue(,"BIRTHYEAR","1972","NUMBER")   /* type NUMBER for numbers */
   r~setvalue(,"LOCATION","EUROPE","EXPAND")

   /* Add multiple zero-terminated strings. The last character must be a 0x as well */
   A_Multi_SZ = "This is the first line" || "0"x || "This is the second line",
                || "0"x || "And this is the last line!" || "0"x
   r~setvalue(,"ADDITIONAL",A_Multi_SZ,"MULTI")  /* use type MULTI */
   ret = RxMessageBox("Explore key \\HKEY_CURRENT_USER\TEST_USER and close RegEdit to continue.",,
                      "WindowsRegistry", "OK")
   regeditor   /* start registry editor */

   /* retrieve the previously added values */
   say "The following entries have been added to the registry:"
   st. = r~getvalue(,"")
   say "default:" st.data
   st. = r~getvalue(,"PROFESSION")
   say "PROFESSION:" st.data
   st. = r~getvalue(,"BIRTHYEAR")
   say "BIRTHYEAR:" st.data "of type" st.type
   st. = r~getvalue(,"LOCATION")
   say "LOCATION:" st.data

   say "ADDITIONAL:"
   st. = r~getvalue(,"ADDITIONAL")
   /* that's how a multiple zero terminated string can be processed */
   if st.type = "MULTI" then    /* that's what we expect */
   do while st.data~length \= 0
       p = st.data~pos(d2c(0))          /* find hex 0 */
       if p < st.data~length then do
           say "   " st.data~left(p-1)   /* display substring */
           st.data = st.data~substr(p+1) /* remove substring */
       end
       else st.data = ""
   end

   r~DeleteValue(,"BIRTHYEAR")   /* delete a value */
   sk = r~Current_Key
   r~create(,"OREXX\DATA1")  /* Create a key and a subkey at once */
   /* Create another key + subkey (use previously stored key handle) */
   lk = r~create(sk,"OREXX\DATA2")
   if lk \= 0 then r~delete(sk, "OREXX\DATA2") /* Delete subkey DATA2 */
end

if savekey \= 0 then do
   /* save newly created key TEST_USER to a file */
   saveret = r~Save(savekey,FileNewKey)
   if saveret = 183 then say "The file" FileNewKey "already exists"
   if saveret = 0 then
       say "\\HKEY_CURRENT_USER\TEST_USER has been saved to" FileNewKey
   r~close(savekey)
   if winsys = "Windows95" then
       /* On Windows 95 delete key TEST_USER including its subkeys*/
       rc = r~Delete(r~Current_User,"TEST_USER")
   else do
       /* On NT it is not possible to delete a key and its subkeys at once, */
       /* so delete the key's subkeys first step by step */
       rc = r~Delete(r~Current_User,"TEST_USER\OREXX\DATA1")
       rc = r~Delete(r~Current_User,"TEST_USER\OREXX")
       rc = r~Delete(r~Current_User,"TEST_USER")  /* and now delete the key */
   end

   if rc = 0 then do
       ret = RxMessageBox("\\HKEY_CURRENT_USER\TEST_USER has been deleted.",
             || " Explore and close RegEdit to continue.", "WindowsRegistry", "OK")
       regeditor
   end
   else say "Delete of key TEST_USER failed ("rc")"
   /* Key restoring only supported by NT */
   if winsys = "WindowsNT" then do
       if r~create(r~Current_User,"TEST_USER") \= 0 then do   /* create key again to restore */
           rc = r~Restore(,FileNewKey)   /* restore previously saved key */
           if rc = 0 then do
               ret = RxMessageBox("\\HKEY_CURRENT_USER\TEST_USER has been restored.",
                    || " Explore and close RegEdit to continue.", "WindowsRegistry", "OK")
               regeditor
               say "Please remove \\HKEY_CURRENT_USER\TEST_USER using" RegEditor
           end
           else say "Restoring failed ("rc")"
       end
   end
   else do /* Use Load and Unload on Windows 95 */
       /* Load creates the specified key and adds the data stored in the file to it. */
       /* The keys data is backed by the file which protects the file from being accessed (on NT only). */
       /* Load can only create a key under HKEY_LOCAL_MACHINE or HKEY_USERS */
       if r~Load(r~Local_Machine,"TEST_USER_LOAD",FileNewKey) = 0 then do   /* load previous key */
           ret = RxMessageBox("Key TEST_USER has been loaded under \\HKEY_LOCAL_MACHINE\TEST_USER_LOAD.",
                    || " Explore and close RegEdit to continue. Try to delete this key within" regeditor".",
                    , "WindowsRegistry", "OK")
           regeditor
           /* The only way to remove a "loaded" key is by using Unload */
           rc = r~Unload(r~Local_Machine,"TEST_USER_LOAD")
           if rc = 0 then say "TEST_USER_LOAD has been unloaded."
           else say "Unload failed ("rc")"
       end
       else say "Restoring failed ("rc")"
   end
   say "The files" FileControl "and" FileNewKey "have been created, please delete manually to cleanup."
   if winsys = "Windows95" then
       say "Notice that these files are hidden. Use attrib -s -h -r to change the attributes."
end

/* winsystm.cls contains the WindowsRegistry class definition */
::requires "winsystm.cls"

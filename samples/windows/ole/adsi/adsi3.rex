/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/*******************************************************************/
/* ADSI Sample 3:                                                  */
/*                                                                 */
/* Showing the use of ADSI containers.                             */
/*                                                                 */
/*******************************************************************/

computerName = value("COMPUTERNAME",,"ENVIRONMENT")   -- alternatively: '.'
groupName = getLocalAdministratorGroupName(computerName)
container = .OLEObject~GetObject("WinNT://"computerName||"/"groupName",group")
do member over container~members
  say member~class ":" member~name "["member~description"]"
end

/* Cf. (as of 2022-05-07):
   <https://devblogs.microsoft.com/scripting/how-can-i-determine-the-name-of-the-local-administrators-group/>
   <https://docs.microsoft.com/en-US/windows/security/identity-protection/access-control/security-identifiers>,
*/

::routine getLocalAdministratorGroupName  -- return local name of the administrator group
  use arg computer
  sid="S-1-5-32-544"                      -- SID for local administrator group
  wmiService = .oleObject~getObject("winmgmts:\\"computer"\root\cimv2")
   -- will return a collection with a single element
  groups=wmiService~execQuery("select * from Win32_Group where LocalAccount = TRUE and sid='"sid"'")
  if groups=.nil then return "Administrators"   -- default: return English name
  return groups~itemIndex(0)~name         -- return local group name


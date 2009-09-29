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
/* Name: osinfo.rex                                                         */
/* Type: Object REXX Script using OLE interface                             */
/* Resource: sysinfo.rc                                                     */
/*                                                                          */
/* Description:                                                             */
/* A sample script that is using a Windows Management Instrumentation (WMI) */
/* object ("Win32_OperatingSystem") to obtain information about the         */
/* installed operating system(s)                                            */
/*                                                                          */
/* Note:                                                                    */
/* Windows 2000 has WMI pre-installed, on WinNT/98 it has to be installed   */
/* manually. See: http://msdn.microsoft.com/downloads/sdks/wmi/eula.asp     */
/*                                                                          */
/* A complete overview of the used classes is available at:                 */
/* http://msdn.microsoft.com/library/psdk/wmisdk/clascomp_3d4j.htm          */
/*                                                                          */
/****************************************************************************/

/* prepare a few arrays for more human-readable output */
boolArray = .array~of("no","yes")
boostArray = .array~of("None","Minimum","Maximum")
qLengthArray = .array~of("???","Unknown","One tick","Two ticks")
qTypeArray = .array~of("???","Unknown","Fixed","Variable")

/* obtain a WMI object */
WMIObject = .OLEObject~GetObject("WinMgmts:{impersonationLevel=impersonate}")

/* obtain a collection of all Win32_OperatingSystem objects */
objects = WMIObject~InstancesOf("Win32_OperatingSystem")

/* process each OperatingSystem object in the collection */
do object over objects
  call head object~Description, "="

  /************* General Information *************/
  call head "General information"

  call print "Build number:", object~BuildNumber
  call print "Build type:", object~BuildType
  call print "Primary OS:", object~Primary, boolArray
  call print "Debug version:", object~Debug, boolArray
  call print "Distributed:", object~Distributed, boolArray
  call print "Manufacturer:", object~Manufacturer
  call print "Version:", object~Version

  sp = object~ServicePackMajorVersion
  if sp \= .nil then
    if object~ServicePackMinorVersion \= .nil then
      sp = sp"."object~ServicePackMinorVersion
  call print "Service Pack:", sp

  call print "OS Status:", object~Status
  call print "Number of users:", object~NumberOfUsers
  call print "Number of licensed users:", object~NumberOfLicensedUsers
  call print "Serial Number:", object~SerialNumber
  call print "Install Date:", object~InstallDate, "DATE"
  call print "Last BootUp Time:", object~LastBootUpTime, "DATE"

  /************* Setup *************/
  call head "Setup"

  call print "Local Date Time:", object~LocalDateTime, "DATE"
  call print "Locale:", object~Locale
  call print "Boot device:", object~BootDevice
  call print "Windows Directory:", object~WindowsDirectory
  call print "CodeSet (CP):", object~CodeSet
  call print "Country Code:", object~CountryCode
  call print "Current Timezone:", object~CurrentTimeZone
  call print "System device:", object~SystemDevice
  call print "System directory:", object~SystemDirectory

  /************* Memory *************/
  call head "Memory"

  call print "Free physical memory:", object~FreePhysicalMemory
  call print "Free space in paging files:", object~FreeSpaceInPagingFiles
  call print "Size stored in paging files:", object~SizeStoredInPagingFiles
  call print "Free virtual memory:", object~FreeVirtualMemory
  call print "Total swap space:", object~TotalSwapSpaceSize
  call print "Total virtual memory:", object~TotalVirtualMemorySize
  call print "Total visible memory:", object~TotalVisibleMemorySize

  /************* System Settings *************/
  call head "System Settings"

  call print "Foreground Application Boost:", object~ForegroundApplicationBoost, boostArray
  call print "Maximum number of processes:", object~MaxNumberOfProcesses
  call print "Numer of processes:", object~NumberOfProcesses
  call print "Maximal process memory size:", object~MaxProcessMemorySize
  call print "Quantum length:", object~QuantumLength, qLengthArray
  call print "Quantum type:", object~QuantumType, qTypeArray
  call print "Organization:", object~Organization
  call print "Registered User:", object~RegisteredUser
  call print "OS language:", object~OSLanguage
  call print "OS type:", object~OSType
  call print "Plus Product ID:", object~PlusProductID
  call print "Plus version number:", object~PlusVersionNumber

end

exit



/* print a header */
::routine head
  use arg title

  if arg(2,'o') = 1 then do
    say
    say "-"~copies(22) center(title,32,' ') "-"~copies(22)
    say
  end
  else do
    say arg(2)~copies(78)
    say arg(2)~copies(20) center(title,36,' ') arg(2)~copies(20)
    say arg(2)~copies(78)
  end

/* display retrieved information */
::routine print
  desc=arg(1)
  content=arg(2)

  /* see if the information needs to be processed or simply displayed */
  if arg(3,'o') = 0 then do
    /* process date to a more human-readable form */
    if arg(3) = "DATE" then do
      d = date('L',left(content,8),'S')
      parse value substr(content,9) with t '.' .
      content = d"," substr(t,1,2)":"substr(t,3,2)":"substr(t,5,2)
    end
    /* else: use array to display information */
    else
      content = arg(3)[content+1]
  end
  if content \= .nil then
    say left(desc,32,' ') content

  return .nil

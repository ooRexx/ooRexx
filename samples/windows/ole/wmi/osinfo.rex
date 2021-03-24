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
/****************************************************************************/
/* Name: osinfo.rex                                                         */
/* Type: ooRexx script using OLE interface                                  */
/*                                                                          */
/* Description:                                                             */
/* A sample script that is using a Windows Management Instrumentation (WMI) */
/* object ("Win32_OperatingSystem") to obtain information about the         */
/* installed operating system(s)                                            */
/*                                                                          */
/* An overview of the used Win32_OperatingSystem class is available at:     */
/* https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-operatingsystem */
/*                                                                          */
/****************************************************************************/

-- prepare a few arrays for more human-readable output
boolArray = "no", "yes"
boostArray = "None", "Minimum", "Maximum"
ostypeArray = .resources~ostype

-- obtain a WMI object
WMIObject = .OLEObject~GetObject("WinMgmts:{impersonationLevel=impersonate}")

-- obtain a collection of all Win32_OperatingSystem objects
objects = WMIObject~InstancesOf("Win32_OperatingSystem")

-- process each OperatingSystem object in the collection
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

  call print "Free physical memory:", object~FreePhysicalMemory, "MEMORY"
  call print "Free space in paging files:", object~FreeSpaceInPagingFiles, "MEMORY"
  call print "Size stored in paging files:", object~SizeStoredInPagingFiles, "MEMORY"
  call print "Free virtual memory:", object~FreeVirtualMemory, "MEMORY"
  call print "Total swap space:", object~TotalSwapSpaceSize, "MEMORY"
  call print "Total virtual memory:", object~TotalVirtualMemorySize, "MEMORY"
  call print "Total visible memory:", object~TotalVisibleMemorySize, "MEMORY"

  /************* System Settings *************/
  call head "System Settings"

  call print "Foreground Application Boost:", object~ForegroundApplicationBoost, boostArray
  call print "Maximum number of processes:", object~MaxNumberOfProcesses
  call print "Numer of processes:", object~NumberOfProcesses
  call print "Maximal process memory size:", object~MaxProcessMemorySize, "MEMORY"
  call print "Organization:", object~Organization
  call print "Registered User:", object~RegisteredUser
  call print "OS language:", object~OSLanguage
  call print "OS type:", object~OSType, ostypeArray

end


-- print a header
::routine head
  use arg title
  title = " " title " "

  if arg(2, "omitted") then do
    say
    say title~center(78, "-")
    say
  end
  else do
    say arg(2)~copies(78)
    say title~center(78, arg(2))
    say arg(2)~copies(78)
  end

-- display retrieved information
::routine print
  use arg desc, content

  if content == .nil then
    return

  -- see if the information needs to be processed or simply displayed
  if arg(3, "exists") then
    -- we can process some information display a more human-readable form
    select case arg(3)
      when "MEMORY" then do
        -- memory sizes are returned in units of kilobytes
        content = content % 1024 "MB"
      end
      when "DATE" then do
        -- e. g. 20201004184321.000000+060
        parse var content yyyy +4 mm +2 dd +2 hh +2 min +2 ss +2 dot +1 usec +6 offset
        content = .DateTime~new(yyyy, mm, dd, hh, min, ss, usec, offset)~toLocalTime
      end
      otherwise -- use array to translate information
        content = arg(3)[content + 1]
  end
  say left(desc, 30) content


::options digits 20 -- memory sizes can be very large

::resource ostype
Unknown
Other
MACOS
ATTUNIX
DGUX
DECNT
Digital Unix
OpenVMS
HPUX
AIX
MVS
OS400
OS/2
JavaVM
MSDOS
WIN3x
WIN95
WIN98
WINNT
WINCE
NCR3000
NetWare
OSF
DC/OS
Reliant UNIX
SCO UnixWare
SCO OpenServer
Sequent
IRIX
Solaris
SunOS
U6000
ASERIES
TandemNSK
TandemNT
BS2000
LINUX
Lynx
XENIX
VM/ESA
Interactive UNIX
BSDUNIX
FreeBSD
NetBSD
GNU Hurd
OS9
MACH Kernel
Inferno
QNX
EPOC
IxWorks
VxWorks
MiNT
BeOS
HP MPE
NextStep
PalmPilot
Rhapsody
Windows 2000
Dedicated
OS/390
VSE
TPF
::END

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
/* Name: services.rex                                                       */
/* Type: Object REXX Script using OLE interface                             */
/*                                                                          */
/* Description:                                                             */
/* List, start, stop, pause or resume windows services with WMI.            */
/*                                                                          */
/* Note:                                                                    */
/* Windows 2000 has WMI pre-installed, on WinNT/98 it has to be installed   */
/* manually. See: http://msdn.microsoft.com/downloads/sdks/wmi/eula.asp     */
/*                                                                          */
/* A complete overview of the used classes is available at:                 */
/* http://msdn.microsoft.com/library/psdk/wmisdk/clascomp_3d4j.htm          */
/*                                                                          */
/****************************************************************************/

WMIObject = .OLEObject~GetObject("WinMgmts:{impersonationLevel=impersonate}")

/* use Win32_BaseService to get a complete list! */
services = WMIObject~InstancesOf("Win32_Service")

boolArray = .array~of("no ","yes")
serviceArray = .array~new(10)


j = 0
/* collect service objects in REXX array */
do instance over services
  j = j + 1
  serviceArray[j] = instance
end

/* main loop */
do while input \= "Q"
  say "Here is a list of windows services:"
  say
  say " No. " left("Name",37,' ') "pausable" "stoppable" "Status"
  say "-"~copies(76)
  do i = 1 to j
    say "["right(i,3,' ')"]" left(serviceArray[i]~description,40,' '),
        left(boolArray[1+serviceArray[i]~AcceptPause],8,' '),
        left(boolArray[1+serviceArray[i]~AcceptStop],6,' '),
        serviceArray[i]~State
  end
  say
  say "Enter a command: start|stop|pause|resume <number of service>, or enter Q to quit"
  parse upper pull input number
  select
  when input = "START" then do
    if number < 0 | number > j then say "Illegal service number specified!"
    else do
      rc = serviceArray[number]~StartService
      call SysSleep 1
      /* get updated object */
      serviceArray[number] = .OLEObject~GetObject(serviceArray[number]~Path_~displayname)
      select
      when rc = 0 then say "The request was accepted."
      when rc = 1 then say "The request is not supported."
      when rc = 2 then say "The user did not have the necessary access."
      when rc = 3 then say "The service cannot be stopped because other services that are running are dependent on it."
      when rc = 4 then say "The requested control code is not valid, or it is unacceptable to the service."
      when rc = 5 then say "The requested control code cannot be sent to the service because the state of the service is equal to 0, 1, or 2."
      when rc = 6 then say "The service has not been started."
      when rc = 7 then say "The service did not respond to the start request in a timely fashion."
      when rc = 8 then say "Unknown failure when starting the service."
      when rc = 9 then say "The directory path to the service executable file was not found."
      when rc = 10 then say "The service is already running."
      when rc = 11 then say "The database to add a new service is locked."
      when rc = 12 then say "A dependency for which this service relies on has been removed from the system."
      when rc = 13 then say "The service failed to find the service needed from a dependent service."
      when rc = 14 then say "The service has been disabled from the system."
      when rc = 15 then say "The service does not have the correct authentication to run on the system."
      when rc = 16 then say "This service is being removed from the system."
      when rc = 17 then say "There is no execution thread for the service."
      when rc = 18 then say "There are circular dependencies when starting the service."
      when rc = 19 then say "There is a service running under the same name."
      when rc = 20 then say "There are invalid characters in the name of the service."
      when rc = 21 then say "Invalid parameters have been passed to the service."
      when rc = 22 then say "The account which this service is to run under is either invalid or lacks the permissions to run the service."
      when rc = 23 then say "The service exists in the database of services available from the system."
      when rc = 24 then say "The service is currently paused in the system."
      otherwise say "unknown error!"
      end
    end
  end
  when input = "STOP" then do
    if number < 0 | number > j then say "Illegal service number specified!"
    else do
      rc = serviceArray[number]~StopService
      call SysSleep 1
      /* get updated object */
      serviceArray[number] = .OLEObject~GetObject(serviceArray[number]~Path_~displayname)
      if rc = 0 then say "The request was accepted."
      else if rc = 1 then say "The request is not supported."
      else say "An error occured."
    end
  end
  when input = "PAUSE" then do
    if number < 0 | number > j then say "Illegal service number specified!"
    else do
      rc = serviceArray[number]~PauseService
      call SysSleep 1
      /* get updated object */
      serviceArray[number] = .OLEObject~GetObject(serviceArray[number]~Path_~displayname)
      if rc = 0 then say "The request was accepted."
      else if rc = 1 then say "The request is not supported."
      else say "An error occured."
    end
  end
  when input = "RESUME" then do
    if number < 0 | number > j then say "Illegal service number specified!"
    else do
      rc = serviceArray[number]~ResumeService
      call SysSleep 1
      /* get updated object */
      serviceArray[number] = .OLEObject~GetObject(serviceArray[number]~Path_~displayname)
      if rc = 0 then say "The request was accepted."
      else if rc = 1 then say "The request is not supported."
      else say "An error occured."
    end
  end

  otherwise
    if input = "Q" then say "Quitting..."
  end
end

exit

::requires "OREXXOLE.CLS"

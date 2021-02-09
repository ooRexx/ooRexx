			Readme_ooRexx5_USB.txt

Copyright 2005-2021 Rexx Language Association.  All rights reserved.

===================================================================

Open Object Rexx is an object-oriented scripting language. The language
is designed for both beginners and experienced Rexx programmers.
It is easy to learn and use, and provides an excellent vehicle to enter
the world of object-oriented programming without much effort.

It extends the procedural way of Rexx programming with object-oriented
features that allow you to gradually change your programming style as
you learn more about objects.

For more information on ooRexx, visit https://www.oorexx.org/
For more information on Rexx, visit   https://www.rexxla.org/

===================================================================

How To install ooRexx 5 on a USB Stick for use with macOS

1. Copy the ooRexx5 image (.dmg file) to the desktop or to a folder. Image name
   will show version and revision such as ooRexx-5.0.0-12149.macOS.x86_64
2. Mount the image by double clicking on it
3. format a USB Stick and give it the name OOREXX5
4. Copy ooRexx5 folder from image to USB Stick

To use the USB installation

5a. On a MAC with no ooRexx installed -> go to 6

5b. On a MAC with an existing ooRexx installation:
   Uninstall or otherwise disable your current installation of ooRexx.
   Make sure that rxapi or rexx are not lurking behind after uninstall.
   Use the process monitor to check that no rexx or rxapi processes
   are still running. Kill them using kill <PID> if they are still running.

6. Open a Terminal and enter: export PATH="/Volumes/OOREXX5/bin:$PATH"

Confirm that the installation works by entering one of:

rexx -v (display version of Rexx)

rexx (show syntax)

rexx /Volumes/OOREXX5/ooRexx5/share/ooRexx/rexxcps (benchmark)


When done, to stop using the USB install:

- kill any rexx or rxapi process using kill <PID>

- close Terminal window

- eject USB stick

(for safety reboot)

PLEASE ALSO NOTE:
ooRexx 5 can be installed anywhere, without any elevated rights, i.e. it is
"sudo free" as long as it is installed where the user has R/W rights.

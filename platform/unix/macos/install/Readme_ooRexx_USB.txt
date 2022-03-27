			Readme_ooRexx5_USB.txt

Copyright 2005-2022 Rexx Language Association.  All rights reserved.

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

1. Copy the latest ooRexx5 image (.dmg file) from Sourceforge to the desktop
   or to a folder. The image name will show version and revision such as
   ooRexx-5.0.0-12317.macOS.arm64.x86_64.

2. Mount the image by double clicking on it.

3. format a USB Stick and give it the name OOREXX

4. Copy the ooRexx5 folder from the mounted image to the USB Stick using
   drag & drop.

To use the USB installation:

5a. On a MAC with no ooRexx installed -> go to 6

5b. On a MAC with an existing ooRexx installation:
   Uninstall or otherwise disable your current installation of ooRexx.
   Make sure that rxapi or rexx are not lurking behind after uninstall.
   Use the process monitor to check that no rexx or rxapi processes
   are still running. Kill them using kill <PID> if they are still running.

6. Open a Terminal and enter: export PATH=/Volumes/OOREXX/ooRexx5/bin:$PATH

7. On at least Mojave and beyond Apple has decided to set a "Quarantine"
extended attribute on files downloaded from the Internet. Check if
this is the case from the terminal (/Applications/Utilities/Terminal.app):

xattr -r /Volumes/OOREXX/ooRexx5

If you see any "com.apple.quarantine" you need to remove them manually
from the commandline:

xattr -dr com.apple.quarantine /Volumes/OOREXX/ooRexx5


8. Confirm that the installation is working by entering one of:

rexx -v (display version of Rexx)

rexx (show syntax)

rexx /Volumes/OOREXX/ooRexx5/share/ooRexx/samples/rexxcps (benchmark)


9. When done, to stop using ooRexx from USB Stick:

- kill any rexx or rxapi process using kill <PID>

- close Terminal window

- eject USB stick

- eject ooRexx5 image

PLEASE ALSO NOTE:
ooRexx 5 can be installed anywhere, without any elevated rights, i.e. it is
"sudo free" as long as it is installed where the user has R/W rights.

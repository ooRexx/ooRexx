			Readme_ooRexx5.txt

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

How To install ooRexx 5 on macOS as a permanent single user installation

1. Copy the ooRexx5 image (.dmg file) to the desktop or to a folder. Image name
   will show version and revision such as ooRexx-5.0.0-12149.macOS.x86_64
2. Mount the image by double clicking on it
3. Drag ooRexx5 to the Applications icon
For Mojave and before:
4a. Add "export PATH=/Applications/ooRexx5/bin:$PATH" to .bash_profile
For Catalina:
4b. Add "export PATH=/Applications/ooRexx5/bin:$PATH" to .zshrc

Confirm that the installation works by entering one of:

rexx -v (display version of Rexx)

rexx (show syntax)

rexx /Applications/ooRexx5/share/ooRexx/rexxcps (benchmark)

To uninstall just drag the folder /ooRexx5  in /Applications to the garbage
bin and remove the export line from .bash_profile or .zshrc.

NOTE: .bash_profile and .zshrc are invisible files that resides in each users
home folder, to see it press shift command . (dot or period character),
all 3 at the same time.

If you do not have .bash_profile enter touch .bash_profile in a terminal
window to create it. Same for .zshrc if you are using Catalina.

PLEASE ALSO NOTE:
ooRexx 5 can be installed anywhere, without any elevated rights, i.e. it is
"sudo free" as long as it is installed where the user has R/W rights. You
can for instance make a folder "Applications" (if not already existing) in
the home folder (~) and drag ooRexx5 there. The export line in .bash_profile
or .zshrc should be amended correspondingly.

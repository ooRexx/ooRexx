                    Instructions For Using CMake To Build ooRexx
                                  Februari 15, 2021

The ooRexx project uses CMake to build ooRexx.
There are a number of advantages to using CMake which include support for
multiple platforms (Windows, Linux, macOS, AIX, etc.), a simple build
mechanism, and many others.

One of the big advantages for developers is that CMake is designed to
perform "out of source" builds of the target project. This means that
builds should be performed from a disk location that is NOT in the
project source tree. Using this methodology keeps the project source tree
clean of all build output files and also allows different types of builds
(development, debug, release) to be active at the same time.

To build ooRexx using CMake you should follow these instructions. You can build
ooRexx on multiple platforms (Windows, Linux, macOS etc) using CMake.

0. First get hold of a SVN client. For Windows a SVN client can be found here:

      https://tortoisesvn.net/

   For *nix use your distribution's repository.
   For macOS use homebrew or MacPorts.

1. Download and install CMake. For Windows, Linux and macOS go to
   https://cmake.org/ and download the install file.
   Alternatively, for *nix distribution you can also install CMake from your
   distribution's repository.
   For macOS you can also install CMake using homebrew or MacPorts.
   Do note that for macOS the minimally required cmake version is 3.12.
   For all other platforms the minimally required cmake version is 2.8.12 

2. Check out the ooRexx source from the SourceForge SVN repository:

     svn co svn://svn.code.sf.net/p/oorexx/code-0/main/trunk oorexxSVN

3. Now create a subdirectory on your disk that is NOT in the ooRexx source tree
   that you checked out in the previous step. All build files will be placed in
   this location. Preferred is a build dir at the same level as the source dir.

4. You are now ready to build ooRexx for your platform.
   On Windows run the following commands:

     cmake -G "NMake Makefiles" C:\ooRexx\source location
     nmake

   Note that after you have used the cmake command to configure a build
   directory, it is only necessary to issue the nmake command from the root of
   the build directory to perform the build.  You should not need to issue the
   cmake command again.

   The version above does not build an installer.  If you wish to also build the
   installer, you need to configure your build by specifying the location of the
   built document files:

     cmake -G "NMake Makefiles" -DDOC_SOURCE_DIR=C:\doc location C:\source location

   The DOC_SOURCE_DIR directory must have the installed doc .pdf files in that
   location- A recent build version can be downloaded from here:

      https://sourceforge.net/projects/oorexx/files/oorexx-docs/

   Or, if you are not building a real release installer, you can just create
   dummy .pdf files in the source directory for the installer compiler to pick
   up.

   To build the interpreter, you still use the nmake command.  Once you have
   built, you can create the installer by issuing

     nmake nsis_template_installer

   We do not use the CPack command on Windows.

   On Linux run the following commands to build an rpm:

     # OS_DIST is optional on the following command. It is used to
     # modify the install package name.
     cmake -DBUILD_RPM=1 -DOS_DIST=fedora20 /ooRexx/source/location
     make
     cpack ./

   On Linux run the following commands to build a deb:

     # OS_DIST is optional on the following command. It is used to
     # modify the install package name.
     cmake -DBUILD_DEB=1 -DOS_DIST=ubuntu1404 /ooRexx/source/location
     make
     cpack ./

   After the completion of these commands there should be a bin subdirectory
   created in your current directory. It should contain all the binaries
   and install files (if the build succeeded).

   On macOS run the following commands to build to the default installation
   directory ~/Applications/ooRexx5 (the Applications directory in your home
   folder)

     cmake -G "Unix Makefiles" /ooRexx/source/location
     make clean
     make
     make install

   Thereafter you need to append the path to the executables to your path

     export PATH=~/Applications/ooRexx5/bin:$PATH

   We do not use Cpack for macOS. If you want to create a Drag&Drop dmg
   installer for macOS you will have to add -DBUILD_DMG=1 to the CMake command.
   This will create a commandfile build_macOS_dmg.sh for you in the build dir.
   Run it to create a dmg installer for macOS.

   Note, once you have issued the cmake command, you should not have to
   issue this again for that build directory.  Use nmake or make to build the code,
   and cpack to create the package files or the shell script for macOS.

                    Instructions For Using CMake To Build ooRexx
                                  January 2, 2019

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

To build ooRexx using CMake you should follow these instruction. You can build
ooRexx on multiple platforms (Windows, Linux, etc) using CMake.

1. Download and install CMake. For Windows go to http://www.cmake.org/ and
   download the Windows install file. For *nix distribution please install CMake
   from your distribution's repository. For macOS, there is an installer at
   kitware.com/cmake - or use homebrew. Do note that for macOS, the minimally
   required cmake version is 3.12.
2. Check out the ooRexx source from the SourceForge SVN repository.
3. Now create a subdirectory on your disk that is NOT in the ooRexx source tree
   you checked out in the previous step. All build files will be placed in this
   location.
4. You are now ready to create everything necessary in building ooRexx on your
   platform. On Windows run the following commands:

     cmake -G "NMake Makefiles" C:\ooRexx\source location
     nmake

   Note that after you have used the cmake command to configure a build directory,
   it is only necessary to issue the nmake command from the root of the build
   directory to perform the build.  You should not need to issue the cmake command
   again.

   The version above does not build an installer.  If you wish to also build the
   installer, you need to configure your build by specifying the location of the
   built document files:

     cmake -G "NMake Makefiles" -DDOC_SOURCE_DIR=C:\doc location C:\source location

   The DOC_SOURCE_DIR directory must have the installed doc .pdf files in that location.
   A recent build version can be downloaded from here:

      http://build.oorexx.org/builds/docs/

   Or, if you are not building a real release installer, you can just create dummy .pdf
   files in the source directory for the installer compiler to pick up.

   To build the interpreter, you still use the nmake command.  Once you have built, you
   can create the installer by issuing

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

   Note, once you have issued the cmake command, you should not have to
   issue this again for that build directory.  Use nmake to build the code,
   and cpack to create the package files.


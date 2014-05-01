                    Instructions For Using CMake To Build ooRexx
                                  May 1, 2014

To build ooRexx using CMake you should follow these instruction. You can build
ooRexx on multiple platforms (Windows, Linux, etc) using CMake.

1. Download and install CMake. For Windows go to http://www.cmake.org/ and
   download the Windows install file. For *nix distribution please install CMake
   from your distribution's repository.
2. Check out the ooRexx source from the SourceForge SVN repository and change
   directory to the root of the source tree.
3. All actions to build ooRexx using CMake should take place in the build
   subdirectory so do

   cd build

4. You are now ready to create the Makefile and other supporting files for
   building ooRexx. On Windows run the following commands:

   cmake -G "NMake Makefiles" ..
   nmake

   On Linux do:

   cmake ..
   make

   After the completion of these commands there should be a bin subdirectory 
   created in your current directory (the build subdirectory). It should contain
   All the binaries and install files.


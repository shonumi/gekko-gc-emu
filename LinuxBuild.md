# Building Gekko on Linux #

You'll need to download and install [GLFW](http://www.glfw.org/) in order to build Gekko. Note that Gekko requires glfw to be built from git, because the 2.x.x release doesn't provide all of the necessary functionality. The git url to the development repository is git://glfw.git.sourceforge.net/gitroot/glfw/glfw.

Building GLFW works by calling these commands from a terminal:
  * git clone git://glfw.git.sourceforge.net/gitroot/glfw/glfw
  * cd glfw
  * mkdir build && cd build
  * cmake ..
  * make
  * sudo make install

Additionally, SDL 2 is required since revision 103cd9f0b268. You can grab a tarball from [here](http://www.libsdl.org/tmp/SDL-2.0.tar.gz).

Building SDL 2 involves these three steps:
  * ./configure
  * make
  * sudo make install

Gekko uses a CMake based build system, so it should be sufficient to call these commands from a terminal within the Gekko source directory:
  * mkdir build
  * cd build
  * cmake ..
  * make
  * sudo make install (currently doesn't work, needs to be fixed)

Optionally, you can use "cmake -i .." to adjust various options (e.g. disable Qt4 GUI).

Building with debug symbols
  * cmake .. -DCMAKE\_BUILD\_TYPE=Debug
  * make

Running without installing
  * cd data
  * ../build/src/gekko/gekko

Debugging
  * cd data
  * gdb ../build/src/gekko/gekko
  * (gdb) run
  * (gdb) bt
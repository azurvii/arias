## Mac OS X _(Tested on Mac OS X 10.6.4)_ ##

  * Download arias-mac.zip.
  * Unzip.
  * Run the arias.app.


## Windows _(Tested on Windows XP SP3, Vista, and 7)_ ##

  * Download arias-setup.exe.
  * Install and run it from Start menu.


## Linux _(Tested on Ubuntu 10.04 x86)_ ##

  * Given that various distributions exist, no binary is provided; you need to compile it yourself for your specific distro.
  * Download arias-src.tar.gz.
  * Install g++, make, and Qt development tools (at least 4.5) on your system. For example, in Ubuntu, run the command:
> > `sudo apt-get install g++ qt4-dev-tools libqt4-dev make`
  * Unpack the src tarball file.
  * Run the command inside the source directory:
> > `qmake && make`
  * The executable is ready in the `build` subdirectory.
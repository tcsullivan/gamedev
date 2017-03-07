gamedev
=======

gamedev is a high school project between drumsetmonkey and tcsullivan written in C++ and using SDL2/OpenGL. The goal of this project is to have a completed commercial-grade video game available to Linux and Windows users. We plan on profiting off of the game once we finish it, so once the game reaches its final stages it may become closed source (however, what was uploaded will stay for others to use as a resource).

NOTE!!!!
We're always working on this project. Some commits may contain broken code, or some bugs may be undocumented. Try older commits if the latest one doesn't work.

Build Requirements
------------------

The 'gamedev' project can be build on both Linux-based and Windows operating systems. The game has been verified to work on Debian, Arch Linux, Fedora, FreeBSD, and Windows 7 (maybe 8 and 10, too). The following libraries are required to build 'gamedev':

* SDL2, including SDL2_image and SDL2_mixer
* FreeType (2? libfreetype6? who knows...)
* GLEW

Windows builds were done with MinGW. The compiler must support at least C++14.

The Makefile makes calls to g++, echo and rm.

Build Preparation
-----------------

The Makefile expects a file named setup.mk to exist. It needs to define two variables on two lines:
```
TARGET_OS = # either win32 or linux
TARGET_BITS = # either 32 or 64
```

A settings file must also exist. To take the sample one:
```
(from the root directory)
cp config/settings.xml.example config/settings.xml
```

Finally, create an output directory (used for compiling):
```
mkdir out
```

Building
--------
```
make
```

This command may be multithreaded using the -j argument.

To run on Linux, once built:
```
./main
```

To run on Windows:
```
main.exe
```

The executable may take the following arguments:

* -r, which will reset XML and player data
* -d, which will kill the game once initialization has been done
* -x, specify which XML file to load. Looks in the XML folder, defined in config/setting.xml

-d is mainly used in conjunction with -r, to reset the XML files and then exit the game before they can be overwritten.
-x might be buggy, or not work.

Controls
--------

The following are the controls for the game. Any extra controls can be found in ```src/ui.cpp```
Controls can kinda be adjusted using the in-game control menu.

Movement:
* 'a' and 'd' move the player left and right
* 'w' enters buildings
* 'e' opens the inventory
* 'space' make the player jump if he obtains the jumping skill
* 'L-Shift' sprints if the player obtains the running skill
* 'L-Ctrl' decreases the player's speed
* 'h' opens a quest menu

Other:
* 'f3' for debug information
* 'f12' for screenshot
* 't' speeds up time (ticks)

gamedev
=======

gamedev was started as a high school project between drumsetmonkey and tcsullivan, written in C++ and using SDL2/OpenGL. The goal of this project was to have a completed commercial-grade video game available for Linux and Windows users; the plan was to profit off the game once it and the engine were finished. Through the first year and a half of development we quickly learned that the game's engine would need the most focus, as we were starting from complete scratch. Due to this, there is no playable, plot-based game.  
  
However, the engine is still growing. Development has now slowed to a crawl with the start of our college lives, but changes will still be made every now and then. Maybe a true video game will eventually come out of this, but the engine will most likely always remain open source.

Build Requirements
------------------

The 'gamedev' project can be built on both Linux-based and Windows operating systems. The game has been verified (at some point) to work on Debian, Arch Linux, FreeBSD, and Windows 7 through 10. The following programs and libraries are required to build 'gamedev':

* make
* g++
* SDL2, including SDL2_image and SDL2_mixer
* FreeType 2
* GLEW
* giflib
* lua (liblua linked into program)
  
Windows builds are done with msys2, 64-bit. The compiler must support C++17.  
  
Build Preparation
-----------------

The Makefile expects a file named setup.mk to exist. It needs to define two variables on two lines:
```
TARGET_OS = # either win32 or linux
TARGET_BITS = # either 32 or 64
```

A settings file must also exist. Take the sample one to start:
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

EntityX needs to be built only once before building the rest of gamedev, a library file will be outputted that the main Makefile uses for the main build.
```
cd entityx
make
cd ..
```

To build the engine:
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

The executable takes the following arguments:

* -r, which will reset XML data and delete player data
* -d, which will initialize the engine, and then exit
* -x, tells the engine which XML file to load. Looks in the XML folder defined in config/setting.xml

-d is mainly used in conjunction with -r, to reset the XML files and then exit the game before they can be rewritten.

Controls
--------

The following are the controls for the game. Any extra controls can be found somewhere in ```src/ui.cpp``` or ```src/player.cpp```
Some controls can be adjusted using the in-game control menu.

Movement:
* 'a' and 'd' move the player left and right
* 'w' enters buildings
* 'e' opens the inventory
* 'space' makes the player jump if he obtains the jumping skill
* 'L-Shift' sprints if the player obtains the running skill
* 'L-Ctrl' decreases the player's speed
* 'h' opens a current quest menu

Other:
* 'f3' for debug information
* 'f12' for screenshot
* 't' speeds up time (ticks)

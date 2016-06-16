gamedev
=======

gamedev is a high school project between drumsetmonkey and tcsullivan written in C++ and using SDL2/OpenGL. The goal of this project is to have a completed commercial-grade video game available to Linux and Windows users. We plan on profiting off of the game once we finish it, so once the game reaches its final stages it may become closed source (however, what was uploaded will stay for others to use as a resource).


Build Requirements
------------------

The 'gamedev' project can be build on both Linux-based and Windows operating systems. The game has been verified to work on Debian, Arch Linux, 
Fedora, FreeBSD, and Windows 7, 8 and 10. The following libraries are required to build 'gamedev':

* SDL2, including SDL2_image and SDL2_mixer
* FreeType (2? libfreetype6? who knows...)
* GLEW

You will also need the GNU GCC compiler collection, including the programs 'g++' and 'ld'.


Building
--------

To build 'gamedev', first create a directory named 'out' and then run make:

```
make
```

This command may be multithreaded using the -j argument.

To run on not-Windows, once built:

```
./main
```

To run on Windows:

```
win32/main.exe
```

The executable may take the following arguments:

* -r, which will reset XML and player data
* -d, which will kill the game once initialization has been done

-d is mainly used in conjunction with -r, to reset the XML files and then exit the game before they can be overwritten.


Controls
--------

The following are the controls for the game. Any extra controls can be found in ```src/ui.cpp```

Movement:
* 'a' and 'd' move the player left and right respectively
* 'space' make the player jump if he obtains the jumping skill
* 'L-Shift' increase the players speed to a "sprint" if he obtains the running skill
* 'L-Ctrl' decrease the players speed to a "walk"

Other:
* 'f' place a light
* 'l' create a light that follows the player
* 't' hold to quickly move time
* 'f3' for debug information

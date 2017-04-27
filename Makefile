include setup.mk

CC  = gcc
CXX = g++

ifeq ($(TARGET_OS),linux)
	LIBS = -lpthread -lGL -lGLEW -lfreetype \
	       -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2main
endif
ifeq ($(TARGET_OS),win32)
	LIBS = -lopengl32 -lglew32 -lmingw32 \
	       -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lfreetype
endif

CXXFLAGS = -ggdb -m$(TARGET_BITS) -std=c++17 -fext-numeric-literals
CXXINC   = -Iinclude -Iinclude/freetype -I.
CXXWARN  = -Wall -Wextra -Werror -pedantic

CXXSRCDIR = src
CXXOUTDIR = out
CXXSRC    = $(wildcard $(CXXSRCDIR)/*.cpp)
CXXOBJ    = $(patsubst $(CXXSRCDIR)/%.cpp, $(CXXOUTDIR)/%.o, $(CXXSRC))

EXEC = main

all: SPECIAL:=-ggdb game

game: $(EXEC)

clean:
	rm -f $(EXEC)
	rm -f out/*.o

$(EXEC): $(CXXOUTDIR)/$(CXXOBJ) main.cpp
	g++ -I. -std=c++11 -c entityx/help/Pool.cc -o out/Pool.o
	g++ -I. -std=c++11 -c entityx/help/Timer.cc -o out/Timer.o
	g++ -I. -std=c++11 -c entityx/Event.cc -o out/Event.o
	g++ -I. -std=c++11 -c entityx/Entity.cc -o out/Entity.o
	g++ -I. -std=c++11 -c entityx/System.cc -o out/System.o
	
	@echo "  CXX/LD  main"
	@$(CXX) $(SPECIAL) $(CXXFLAGS) $(CXXINC) $(CXXWARN) -o $(EXEC) main.cpp out/*.o $(LIBS)
	@rm -rf xml/*.dat
	@rm -rf storyXML/*.dat

$(CXXOUTDIR)/%.o: $(CXXSRCDIR)/%.cpp
	@echo "  CXX    " $<
	@$(CXX) $(CXXFLAGS) $(CXXINC) $(CXXWARN) $(LIBS) -c $< -o $@

$(CXXOUTDIR)/%.o: $(CXXSRCDIR)/%.cc
	@echo "  CXX    " $<
	@$(CXX) $(CXXFLAGS) $(CXXINC) $(CXXWARN) $(LIBS) -c $< -o $@

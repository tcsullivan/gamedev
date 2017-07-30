include setup.mk

CC  = gcc
CXX = g++

ifeq ($(TARGET_OS),linux)
	LIBS = -Llib -lgif -lentityx -lpthread -lGL -lGLEW -lfreetype \
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
CXXSRC    = $(wildcard $(CXXSRCDIR)/*.cpp) \
			$(wildcard $(CXXSRCDIR)/systems/*.cpp) \
			$(wildcard $(CXXSRCDIR)/components/*.cpp)
CXXOBJ    = $(patsubst $(CXXSRCDIR)/%.cpp, $(CXXOUTDIR)/%.o, $(CXXSRC)) \
			$(patsubst $(CXXSRCDIR)/systems/%.cpp, $(CXXOUTDIR)/systems/%.o, $(CXXSRC)) \
			$(patsubst $(CXXSRCDIR)/components/%.cpp, $(CXXOUTDIR)/components/%.o, $(CXXSRC))

EXEC = main

all: SPECIAL:=-ggdb game

game: $(EXEC)

clean:
	@echo "  CLEAN"
	@rm -f $(EXEC)
	@rm -rf out
	@mkdir out
	@mkdir out/systems
	@mkdir out/components

$(EXEC): $(CXXOUTDIR)/$(CXXOBJ) main.cpp
	@echo "  CXX/LD  main"
	@$(CXX) $(SPECIAL) $(CXXFLAGS) $(CXXINC) $(CXXWARN) -o $(EXEC) main.cpp out/components/*.o out/systems/*.o out/*.o $(LIBS)
	@rm -rf xml/*.dat
	@rm -rf storyXML/*.dat

$(CXXOUTDIR)/%.o: $(CXXSRCDIR)/%.cpp
	@echo "  CXX    " $<
	@$(CXX) $(CXXFLAGS) $(CXXINC) $(CXXWARN) $(LIBS) -c $< -o $@

$(CXXOUTDIR)/%.o: $(CXXSRCDIR)/%.cc
	@echo "  CXX    " $<
	@$(CXX) $(CXXFLAGS) $(CXXINC) $(CXXWARN) $(LIBS) -c $< -o $@

include setup.mk

CC  = gcc
CXX = g++

ifeq ($(TARGET_OS),linux)
	LIBS = -Lentityx -lgif -llua5.3 -lentityx -lpthread -lGL -lGLEW -lfreetype \
	       -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2main
endif
ifeq ($(TARGET_OS),win32)
	LIBS = -Lentityx -lgif -llua -lentityx -lopengl32 -lglew32 -lmingw32 \
	       -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lfreetype
endif

CXXFLAGS = -O0 -g3 -ggdb -m$(TARGET_BITS) -std=c++20 \
           -Wall -Wextra -Werror -pedantic \
           -I. -Iinclude -Iinclude/freetype

CXXSRCDIR = src
CXXOUTDIR = out
CXXSRC    = $(wildcard $(CXXSRCDIR)/*.cpp) \
			$(wildcard $(CXXSRCDIR)/systems/*.cpp) \
			$(wildcard $(CXXSRCDIR)/components/*.cpp)
CXXOBJ    = $(patsubst $(CXXSRCDIR)/%.cpp, $(CXXOUTDIR)/%.o, \
			$(patsubst $(CXXSRCDIR)/systems/%.cpp, $(CXXOUTDIR)/systems/%.o, \
			$(patsubst $(CXXSRCDIR)/components/%.cpp, $(CXXOUTDIR)/components/%.o, $(CXXSRC))))

EXEC = main

all: $(EXEC)

clean:
	@echo "  CLEAN"
	@rm -f $(EXEC)
	@rm -rf out
	@rm -f xml/*.dat

$(EXEC): $(CXXOUTDIR) $(CXXOBJ)
	@echo "  CXX/LD  main"
	@$(CXX) $(CXXFLAGS) -o $(EXEC) $(CXXOBJ) $(LIBS)

$(CXXOUTDIR):
	@mkdir out
	@mkdir out/systems
	@mkdir out/components

$(CXXOUTDIR)/%.o: $(CXXSRCDIR)/%.cpp
	@echo "  CXX    " $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@


LIBS = -lGL -lSDL2_image -lSDL2_mixer

FLAGS = -m32 -std=c++11 -I../include -I../include/freetype2 -lSDL2main -lSDL2 -lfreetype

OUT = `echo "" $$(ls -c $(wildcard *.cpp)) | sed s/.cpp/.o/g | sed 's/ / ..\/out\//g'`

../out/%.o:
	@echo "  CXX " $(shell echo $@ | sed 's/..\/out\///g' | sed 's/\.o/\.cpp/')
	@g++ $(FLAGS) -o $@ -c $(shell echo $@ | sed 's/..\/out\///g' | sed 's/\.o/\.cpp/') $(LIBS)

all:  $(shell echo $(OUT))

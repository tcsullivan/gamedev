LIBS = -lpthread -lGL -lSDL2 -lSDL2_image -lSDL2_mixer -lfreetype

FLAGS = -std=c++11 -I../include -I../include/freetype2 -Wall -Wextra -Werror

OUT = `echo "" $$(ls -c $(wildcard *.cpp)) | sed s/.cpp/.o/g | sed 's/ / ..\/out\//g'`
OUT64 = `echo "" $$(ls -c $(wildcard *.cpp)) | sed s/.cpp/.o/g | sed 's/ / ..\/out64\//g'`

../out/%.o:
	@echo "  CXX " $(shell echo $@ | sed 's/..\/out\///g' | sed 's/\.o/\.cpp/')
	@g++ $(FLAGS) -m32 -o $@ -c $(shell echo $@ | sed 's/..\/out\///g' | sed 's/\.o/\.cpp/') $(LIBS) -lSDL2main

../out64/%.o:
	@echo "  CXX " $(shell echo $@ | sed 's/..\/out64\///g' | sed 's/\.o/\.cpp/')
	@g++ $(FLAGS) -m64 -o $@ -c $(shell echo $@ | sed 's/..\/out64\///g' | sed 's/\.o/\.cpp/') $(LIBS)

all: $(shell echo $(OUT))

64: $(shell echo $(OUT64))

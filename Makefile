LIBS_LINUX = -lGL -lSDL2_image -lSDL2_mixer
LIBS_WIN32 = -lopengl32 -lmingw32 -lSDL2_Image

FLAGS = -m32 -std=c++11 -Iinclude -Iinclude/freetype2 -lSDL2main -lSDL2 -lfreetype

SRCS = $(wildcard src/*.cpp)

OUT = $(SRCS:.cpp=.o)

.cpp.o:
	g++ -o $@ -c $^ $(LIBS_LINUX) $(FLAGS)

all: $(OUT)
	mv ./src/*.o ./out
	g++ -o main main.cpp out/*.o $(LIBS_LINUX) $(FLAGS)

clean:
	-rm main
	-rm out/*.o

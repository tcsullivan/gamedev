LIBS_LINUX = -lGL -lSDL2_image -lSDL2_mixer
LIBS_WIN32 = -lopengl32 -lmingw32 -lSDL2_Image

FLAGS = -m32 -std=c++11 -Iinclude -Iinclude/freetype2 -lSDL2main -lSDL2 -lfreetype

SRCS = $(wildcard src/*.cpp)

OUT = $(SRCS:.cpp=.o)

.cpp.o:
	@echo "  CXX " $^
	@g++ -o $@ -c $^ $(LIBS_LINUX) $(FLAGS)

all: $(OUT)
	@echo "Relocating object files..."
	@mv ./src/*.o ./out
	@echo "  CXX main.cpp"
	@g++ -o main main.cpp out/*.o $(LIBS_LINUX) $(FLAGS)

clean:
	@echo "  RM main"
	@-rm -f main
	@echo "  RM out/*.o"
	@-rm -f out/*.o

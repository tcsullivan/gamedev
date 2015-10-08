LIBS = -lGL
WIN_LIBS = -lopengl32 -lmingw32

FLAGS = -m32 -std=c++11 -Iinclude -Iinclude/freetype2 -lSDL2main -lSDL2 -lfreetype -lSDL2_image -lSDL2_mixer

all:
	@rm -f out/*.o
	@cd src; $(MAKE) $(MFLAGS)
	@echo "  CXX  main.cpp"
	@g++ $(FLAGS) -o main main.cpp out/*.o $(LIBS)

win32:
	@g++ $(FLAGS) -o main main.cpp src/*.cpp $(WIN_LIBS)

clean:
	@echo "  RM main"
	@-rm -f main
	@echo "  RM out/*.o"
	@-rm -f out/*.o

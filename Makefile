LIBS = -lGL -lGLEW -lSDL2 -lfreetype -lSDL2_image -lSDL2_mixer

WIN_LIBS = -lopengl32 -lglew32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lfreetype

FLAGS = -m32 -std=c++11 -Iinclude -Iinclude/freetype2

MFLAGS64 = 64
all:
	@rm -f out/*.o
	@cd src; $(MAKE) $(MFLAGS)
	@echo "  CXX  main.cpp"
	@g++ $(FLAGS) -m32 -o main main.cpp out/*.o $(LIBS) -lSDL2main

64:
	@rm -f out64/*.o
	@cd src; $(MAKE) $(MFLAGS64)
	@echo "  CXX  main.cpp"
	@g++ $(FLAGS) -m64 -o main main.cpp out64/*.o $(LIBS)

win32:
	@g++ $(FLAGS) -o main main.cpp src/*.cpp $(WIN_LIBS)
	
clean:
	@echo "  RM main"
	@-rm -f main
	@echo "  RM out/*.o"
	@-rm -f out/*.o
	@echo "  RM out64/*.o"
	@-rm -f out64/*.o

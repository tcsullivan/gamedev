LIBS = -lGL -lGLEW -lSDL2main -lSDL2 -lfreetype -lSDL2_image -lSDL2_mixer

FLAGS = -m32 -std=c++11 -Iinclude -Iinclude/freetype2

all:
	@rm -f out/*.o
	@cd src; $(MAKE) $(MFLAGS)
	@echo "  CXX  main.cpp"
	@g++ $(FLAGS) -o main main.cpp out/*.o $(LIBS)

clean:
	@echo "  RM main"
	@-rm -f main
	@echo "  RM out/*.o"
	@-rm -f out/*.o

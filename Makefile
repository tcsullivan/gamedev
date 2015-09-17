FLAGS_LINUX = -lGL                 -lSDL2_image
FLAGS_WIN32 = -lopengl32 -lmingw32 -lSDL2_Image
FLAGS = -m32 -std=c++11 -Iinclude -lSDL2main -lSDL2

all:
	@g++ src/*.cpp -o main $(FLAGS_LINUX) $(FLAGS)

win32:
	@g++ -L lib/ src/*.cpp -o main.exe $(FLAGS_WIN32) $(FLAGS)

clean:
	rm main*

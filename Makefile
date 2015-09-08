FLAGS_LINUX = -lGL                 -lSDL_image
FLAGS_WIN32 = -lopengl32 -lmingw32 #-lSDL2_Image
FLAGS = -m32 -Iinclude -Wall -Werror -lSDL2main -lSDL2

all:
	@g++ src/main.cpp src/UIClass.cpp src/windowClass.cpp src/Quest.cpp -o main $(FLAGS_LINUX) $(FLAGS)

win32:
	@g++ -L lib/ src/main.cpp src/UIClass.cpp src/windowClass.cpp src/Quest.cpp -o main.exe $(FLAGS_WIN32) $(FLAGS)

clean:
	rm main*

FLAGS_LINUX = -lGL                 -lSDL_image
FLAGS_WIN32 = -lopengl32 -lmingw32 -lSDL2_Image
FLAGS = -m32 -Wall -Werror -lSDL2main -lSDL2

all:
	@g++ src/main.cpp -o main $(FLAGS_LINUX) $(FLAGS)

win32:
	@g++ src/main.cpp -o main.exe $(FLAGS_WIN32) $(FLAGS)

clean:
	rm main*

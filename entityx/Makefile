# gamedev CHANGE - changed output of libentityx to ../lib/

INC = -I..
FLG = -std=gnu++11

FILES = help/Pool.cc \
	help/Timer.cc \
	Event.cc \
	Entity.cc \
	System.cc
all:
	mkdir out
	g++ $(INC) $(FLG) -c help/Pool.cc -o out/Pool.o
	g++ $(INC) $(FLG) -c help/Timer.cc -o out/Timer.o
	g++ $(INC) $(FLG) -c Event.cc -o out/Event.o
	g++ $(INC) $(FLG) -c Entity.cc -o out/Entity.o
	g++ $(INC) $(FLG) -c System.cc -o out/System.o
	ar rvs ../lib/libentityx.a out/*.o
#	g++ out/*.o -shared -o ../entityx.so
	rm -rf out

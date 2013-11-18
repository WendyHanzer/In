
OS := $(shell uname)

ifeq ($(OS), Linux)
CC=g++
LIBS= -lglut -lGLEW -lGL -lassimp -lfreeimageplus `pkg-config bullet --libs`
CXXFLAGS= -g -Wall -std=c++11
INC= `pkg-config bullet --cflags` -I../src/
RM= 

else #Mac
CC=clang++
LIBS= -L/usr/local/lib/ -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -lassimp -lfreeimageplus -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
CXXFLAGS= -g -Wall -std=c++11 -stdlib=libc++
INC= -I/usr/local/include/bullet -I/usr/local/include/
RM= ../bin/lab.dSYM
endif

OBJ= shaderloader.o engine.o simobject.o modelloader.o

all: ../bin/lab

../bin/lab: ../src/main.cpp $(OBJ)
	$(CC) $(CXXFLAGS) $(DEFS) $(INC) ../src/main.cpp -o ../bin/lab $(OBJ) $(LIBS)

engine.o: ../src/engine.h ../src/engine.cpp
	$(CC) $(CXXFLAGS) $(DEFS) $(INC) -c ../src/engine.cpp

simobject.o: ../src/simobject.h ../src/simobject.cpp
	$(CC) $(CXXFLAGS) $(DEFS) $(INC) -c ../src/simobject.cpp

shaderloader.o: ../src/shaderloader.h ../src/shaderloader.cpp
	$(CC) $(CXXFLAGS) $(DEFS) $(INC) -c ../src/shaderloader.cpp

modelloader.o: ../src/modelloader.h ../src/modelloader.cpp
	$(CC) $(CXXFLAGS) $(DEFS) $(INC) -c ../src/modelloader.cpp

clean:
	rm -rf *.o ../bin/lab $(RM)

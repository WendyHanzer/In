
OS := $(shell uname)

ifeq ($(OS), Linux)
CC=g++
LIBS= -lglut -lGLEW -lGL -lassimp -lfreeimageplus
CXXFLAGS= -g -Wall -std=c++0x
RM= 

else #Mac
CC=clang++
LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -lassimp -lfreeimageplus
CXXFLAGS= -g -Wall -std=c++11 -stdlib=libc++
RM= ../bin/Matrix.dSYM
endif

OBJ= shaderloader.o modelloader.o

all: ../bin/Matrix

../bin/Matrix: ../src/main.cpp $(OBJ)
	$(CC) $(CXXFLAGS) $(DEFS) ../src/main.cpp -o ../bin/Matrix $(OBJ) $(LIBS)

shaderloader.o: ../src/shaderloader.h ../src/shaderloader.cpp
	$(CC) $(CXXFLAGS) $(DEFS) -c ../src/shaderloader.cpp

modelloader.o: ../src/modelloader.h ../src/modelloader.cpp
	$(CC) $(CXXFLAGS) $(DEFS) -c ../src/modelloader.cpp
clean:
	rm -rf *.o ../bin/Matrix $(RM)

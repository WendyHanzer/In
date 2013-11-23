#ifndef VERTEX_H
#define VERTEX_H

// disable 3rd party library warnings on Apple
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>

// re-enable warnings
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

// vertex struct containing all OpenGL data necessary
struct Vertex
{
    GLfloat position[3];
    GLfloat color[9];
    GLfloat textCoord[2];
    GLfloat normal[3];

    GLfloat shininess;
};

#endif // VERTEX_H

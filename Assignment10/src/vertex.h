#ifndef VERTEX_H
#define VERTEX_H

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>

struct Vertex
{
    GLfloat position[3];
    GLfloat color[9];
    GLfloat textCoord[2];
    GLfloat normal[3];

    GLfloat shininess;
/*
    friend std::ostream& operator << (std::ostream& out, const Vertex &vert) {
        out << "Pos: " << vert.position[0] << ' ' << vert.position[1] << ' ' << vert.position[2] << std::endl;
        out << "Col: " << vert.color[0] << ' ' << vert.color[1] << ' ' << vert.color[2];

        return out;
    }
*/
};

#endif // VERTEX_H

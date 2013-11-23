
#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

// disable 3rd party library warnings on Apple
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <initializer_list>

// re-enable warnings
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

class ShaderLoader
{
public:
	// constructor, destructor, and copy constructor
    ShaderLoader(GLenum _shaderType);
    ~ShaderLoader() {}
    ShaderLoader(const ShaderLoader& other);
    
    // load shader file
    bool load(std::string filename);
    
    // get shader ID
    GLuint getShader() const;
    
    // link shaders into OpenGL program
    static GLuint linkShaders(std::initializer_list<ShaderLoader> shaders);
    
private:
	// member variables
    GLuint shader;
    GLenum shaderType;
};

#endif // SHADER_LOADER_H

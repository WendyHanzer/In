
#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <initializer_list>

class ShaderLoader
{
public:
    ShaderLoader(GLenum _shaderType);
    ShaderLoader(const ShaderLoader& other);
    
    bool load(std::string filename);
    
    GLuint getShader() const;
    
    static GLuint linkShaders(std::initializer_list<ShaderLoader> shaders);
    
private:
    GLuint shader;
    GLenum shaderType;
};

#endif // SHADER_LOADER_H

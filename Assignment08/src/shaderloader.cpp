
#include "shaderloader.h"

ShaderLoader::ShaderLoader(GLenum _shaderType) : shader(0), shaderType(_shaderType)
{
}

ShaderLoader::ShaderLoader(const ShaderLoader& other) : shader(other.shader), 
	shaderType(other.shaderType)
{
}

bool ShaderLoader::load(std::string filename)
{
    std::ifstream fin(filename);
    std::string shaderContent;
    GLint shader_status;

    if(!fin) {
        std::cerr << "Unable to open shader file: " << filename << std::endl;
        return false;
    }

    // get size of file
    fin.seekg(0, std::ios::end);
    auto length = fin.tellg();
    fin.seekg(0, std::ios::beg);

    // allocate proper space for file content
    shaderContent.reserve(length);

    // fill string with contents of file
	// solution found at stackoverflow.com from user Tyler McHenry
    auto fileIterator = std::istreambuf_iterator<char>(fin);
    auto iteratorEnd = std::istreambuf_iterator<char>();
    shaderContent.assign(fileIterator, iteratorEnd);

    fin.close();
    
    const char *shaderStr = shaderContent.c_str();
    
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderStr, NULL);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::string shaderMsg = (shaderType == GL_VERTEX_SHADER) ? "VERTEX SHADER" : "FRAGMENT SHADER";
        std::cerr << "[F] FAILED TO COMPILE " 
                  << shaderMsg
                  << std::endl;
        return false;
    }
    
    return true;
}

GLuint ShaderLoader::getShader() const
{
    return shader;
}
    
GLuint ShaderLoader::linkShaders(std::initializer_list<ShaderLoader> shaders)
{
	if(shaders.size() < 1) {
		std::cerr << "No Shaders Provided to Link!!" << std::endl;
		exit(-1);
	}
	
    GLuint program = glCreateProgram();
	
	for(const auto& shader : shaders) {
		glAttachShader(program, shader.getShader());
	}

    glLinkProgram(program);
    
    GLint shader_status;
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        exit(-1);
    }
    
    return program;
}

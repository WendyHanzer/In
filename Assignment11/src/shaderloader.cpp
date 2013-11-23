
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
    // init variables
    std::ifstream fin(filename);
    std::string shaderContent;
    GLint shader_status;

    // if unable to open file, return failure
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

    // close input file
    fin.close();
    
    // get c-string version of file contents
    const char *shaderStr = shaderContent.c_str();
    
    // create OpenGL shader and load shader source
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderStr, NULL);

    // compile shader
    glCompileShader(shader);
    
    // get status of compilation
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        // output error message
        std::string shaderMsg = (shaderType == GL_VERTEX_SHADER) ? "VERTEX SHADER" : "FRAGMENT SHADER";
        std::cerr << "[F] FAILED TO COMPILE " 
                  << shaderMsg
                  << std::endl;

        // get descriptive error message
		char buffer[256];
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		glGetShaderInfoLog(shader, length, &length, buffer);

        // output error
		std::cerr << buffer << std::endl;

        // return failure
        return false;
    }
    
    // return success
    return true;
}

GLuint ShaderLoader::getShader() const
{
    return shader;
}
    
GLuint ShaderLoader::linkShaders(std::initializer_list<ShaderLoader> shaders)
{
    // if no shaders passed, exit
	if(shaders.size() < 1) {
		std::cerr << "No Shaders Provided to Link!!" << std::endl;
		exit(-1);
	}
	
    // create OpenGL program
    GLuint program = glCreateProgram();
	
    // attach each shader to program
	for(const auto& shader : shaders) {
		glAttachShader(program, shader.getShader());
	}

    // link program
    glLinkProgram(program);
    
    // check for linker status
    GLint shader_status;
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);

    // if linker failed, report it and exit
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        exit(-1);
    }
    
    return program;
}

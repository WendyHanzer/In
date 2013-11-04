#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

#ifdef ASSIMP_2
#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiTexture.h>

#else
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#endif

#include <FreeImagePlus.h>

#include "../src/vertex.h"

class ModelLoader
{
public:
	ModelLoader(const char *objectFile = "model.obj");
	void setFileName(std::string fileName);
	std::vector<Vertex> load(int& numTriangles, int& numTextures);

	void loadTexture(const char *fileName);	
	GLuint getTexture(int index) const;

private:
	std::string filename;
	std::vector<GLuint> textures;
};

#endif // MODEL_LOADER_H

#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

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
#include <vector>
#include <deque>

// if using assimp version 2, load different headers
#ifdef ASSIMP_2
#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiTexture.h>

#else // assimp version 3
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#endif

#include <FreeImagePlus.h>

#include "vertex.h"

// re-enable warnings
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

class ModelLoader
{
public:
	// constructor and destructor
	ModelLoader(const char *objectFile = "model.obj");
	~ModelLoader() {}

	// update filename to new name
	void setFileName(std::string fileName);

	// used to load geometry from object file
	std::vector<Vertex> load(int& numTriangles, int& numTextures, Vertex& light);

	// used to load texture from file
	void loadTexture(const char *fileName);

	// return textureID	
	GLuint getTexture(int index) const;

private:
	// member variables
	std::string filename;
	std::vector<GLuint> textures;
};

#endif // MODEL_LOADER_H

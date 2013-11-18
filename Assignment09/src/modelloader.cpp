#include "modelloader.h"

ModelLoader::ModelLoader(const char *objectFile)
    : filename(objectFile), textures()
{
}

void ModelLoader::setFileName(std::string fileName)
{
	filename = fileName;
}

std::vector<Vertex> ModelLoader::load(int& numTriangles, int& numTextures)
{
	std::ifstream fileCheck(filename);
	Vertex tempVert;
	std::vector<Vertex> geometry;
	std::deque<aiVector3D> texCoord;
	numTriangles = 0;
	numTextures = 0;

	for(int i = 0; i < 3; i++)
		tempVert.color[i] = 1.0;

	if(!fileCheck) {
		std::cerr << "Error: Unable to open object file" << std::endl;
		exit(-1);
	}

	fileCheck.close();

	Assimp::Importer importer;

	auto scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);

	if(!scene) {
		std::cerr << "Error: " << importer.GetErrorString() << std::endl;
		exit(-1);
	}

	std::cout << "Material Count: " << scene->mNumMaterials << std::endl;
	
	if(scene->HasMaterials()) {
		const auto& material = scene->mMaterials[0];
		aiColor3D color(0.0f,0.0f,0.0);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		tempVert.color[0] = color.r;
		tempVert.color[1] = color.g;
		tempVert.color[2] = color.b;
	}

	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		auto mesh = scene->mMeshes[i];

		if(scene->mNumMaterials > i+1) {
			auto material = scene->mMaterials[i+1];

			aiColor3D color(1.0f,1.0f,1.0f);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			tempVert.color[0] = color.r;
			tempVert.color[1] = color.g;
			tempVert.color[2] = color.b;

			if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;

				if(material->GetTexture(aiTextureType_DIFFUSE, 0, &path, 
					NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					
					std::cout << "Texture Path: " << path.C_Str() << std::endl;
					numTextures++;
					loadTexture(path.C_Str());
/*
					for(unsigned int j = 0; j < mesh->mNumVertices; j++) {
						const auto textureCoords = mesh->HasTextureCoords(0) ?
							 mesh->mTextureCoords[0][j] : aiVector3D(0.0f,0.0f,0.0f);

						//texCoord.emplace_back(textureCoords);
					}
					*/
				}

			}

		}

		numTriangles += mesh->mNumFaces;
		for(unsigned int j = 0; j < mesh->mNumFaces; j++) {
			const auto& face = mesh->mFaces[j];

			for(unsigned int k = 0; k < face.mNumIndices; k++) {
				const auto& vertex = mesh->mVertices[face.mIndices[k]];
				tempVert.position[0] = vertex.x;
				tempVert.position[1] = vertex.y;
				tempVert.position[2] = vertex.z;
				
				const auto& textureVertex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][face.mIndices[k]]
						: aiVector3D(0,0,0);
				tempVert.textCoord[0] = textureVertex[0];
				tempVert.textCoord[1] = textureVertex[1];
			
				geometry.push_back(tempVert);
			}
		}
	}

	//loadTexture("checkerboard.jpg");
	std::cout << "size: " << geometry.size() << std::endl
			  << "bytes: " << sizeof(tempVert) * geometry.size() << std::endl;
	return geometry;
}

void ModelLoader::loadTexture(const char *fileName)
{
	fipImage image;
	GLuint texId;

	if(image.load(fileName)) {
		
		if(image.getImageType() == FIT_UNKNOWN) {
			std::cerr << "Unkown image type!" << std::endl;
			return;
		}

		image.convertTo32Bits();
	
		glGenTextures(1, &texId);
		//glActiveTexture(GL_TEXTURE0);
		std::cout << "TexId: " << texId << std::endl;
		glBindTexture(GL_TEXTURE_2D, texId);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.getWidth(), image.getHeight(), 
			0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) image.accessPixels());
		textures.push_back(texId);
	}
}

GLuint ModelLoader::getTexture(int index) const {
	return textures.at(index);
}

#include "modelloader.h"

ModelLoader::ModelLoader(const char *objectFile)
    : filename(objectFile), textures()
{
}

void ModelLoader::setFileName(std::string fileName)
{
	filename = fileName;
}

std::vector<Vertex> ModelLoader::load(int& numTriangles, int& numTextures, Vertex& light)
{
	// init variables
	std::ifstream fileCheck(filename);
	Vertex tempVert;
	std::vector<Vertex> geometry;
	std::deque<aiVector3D> texCoord;
	numTriangles = 0;
	numTextures = 0;

	// init color to white
	for(int i = 0; i < 3; i++)
		tempVert.color[i] = 1.0;

	// if model file doesnt exist, exit
	if(!fileCheck) {
		std::cerr << "Error: Unable to open object file: " << filename << std::endl;
		exit(-1);
	}

	fileCheck.close();

	// create Assimp importer
	Assimp::Importer importer;

	// load scene from file
	auto scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);

	// if scene can't load, exit
	if(!scene) {
		std::cerr << "Error: " << importer.GetErrorString() << std::endl;
		exit(-1);
	}

	std::cout << "Material Count: " << scene->mNumMaterials << std::endl;

	// if material file exists, load color
	if(scene->HasMaterials()) {
		const auto& material = scene->mMaterials[0];
		aiColor3D color(0.0f,0.0f,0.0);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		light.color[0] = color.r;
		light.color[1] = color.g;
		light.color[2] = color.b;
		float shininess;
		material->Get(AI_MATKEY_SHININESS, shininess);
		light.shininess = shininess;
	}

	// for each mesh load color and geometry
	for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
		auto mesh = scene->mMeshes[i];

		// if mesh has material, load color
		if(scene->mNumMaterials > i+1) {
			auto material = scene->mMaterials[i+1];
			float shininess;

			aiColor3D color(1.0f,1.0f,1.0f);
			material->Get(AI_MATKEY_COLOR_AMBIENT, color);
			light.color[0] = tempVert.color[0] = color.r;
			light.color[1] = tempVert.color[1] = color.g;
			light.color[2] = tempVert.color[2] = color.b;
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			light.color[3] = tempVert.color[0] = color.r;
			light.color[4] = tempVert.color[1] = color.g;
			light.color[5] = tempVert.color[2] = color.b;
			material->Get(AI_MATKEY_COLOR_SPECULAR, color);
			light.color[6] = tempVert.color[6] = color.r;
			light.color[7] = tempVert.color[7] = color.g;
			light.color[8] = tempVert.color[8] = color.b;
		    material->Get(AI_MATKEY_SHININESS, shininess);
		    light.shininess = tempVert.shininess = shininess;

		    // if texture exists, get path
			if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;

				// if texture found successfully, load it
				if(material->GetTexture(aiTextureType_DIFFUSE, 0, &path,
					NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {

					std::cout << "Texture Path: " << path.C_Str() << std::endl;

					// increment texture count
					numTextures++;

					// load texture
					loadTexture(path.C_Str());
				}

			}

		}

		// get number of triangles for mesh
		numTriangles += mesh->mNumFaces;

		// iterate through mesh faces
		for(unsigned int j = 0; j < mesh->mNumFaces; j++) {
			const auto& face = mesh->mFaces[j];

			// for each index in the face, grab vertex position
			for(unsigned int k = 0; k < face.mNumIndices; k++) {
				const auto& vertex = mesh->mVertices[face.mIndices[k]];
				tempVert.position[0] = vertex.x;
				tempVert.position[1] = vertex.y;
				tempVert.position[2] = vertex.z;

				// grab texture coordinates if they exist
				const auto& textureVertex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][face.mIndices[k]]
						: aiVector3D(0,0,0);
				tempVert.textCoord[0] = textureVertex[0];
				tempVert.textCoord[1] = textureVertex[1];

				// get normal values
				const auto& normals = mesh->mNormals[face.mIndices[k]];
				tempVert.normal[0] = normals.x;
				tempVert.normal[1] = normals.y;
				tempVert.normal[2] = normals.z;

				// add vertex to geometry vector
				geometry.push_back(tempVert);
			}
		}
	}

	// output size of model
	std::cout << "size: " << geometry.size() << std::endl
			  << "bytes: " << sizeof(tempVert) * geometry.size() << std::endl;

	// return object geometry
	return geometry;
}

void ModelLoader::loadTexture(const char *fileName)
{
	// init variables
	fipImage image;
	GLuint texId;

	// if texture found, load it
	if(image.load(fileName)) {

		// if unknown image type, return
		if(image.getImageType() == FIT_UNKNOWN) {
			std::cerr << "Unkown image type!" << std::endl;
			return;
		}

		// convert image to 32 bit pixels
		image.convertTo32Bits();

		// generate OpenGL texture
		glGenTextures(1, &texId);

		// output textureID
		std::cout << "TexId: " << texId << std::endl;

		// bind texture to textureID
		glBindTexture(GL_TEXTURE_2D, texId);

		// set texture parameters
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// load OpenGL texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getWidth(), image.getHeight(),
			0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) image.accessPixels());

		// add texture to textures vector
		textures.push_back(texId);
	}
}

GLuint ModelLoader::getTexture(int index) const {
	return textures.at(index);
}

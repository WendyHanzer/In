#include <iostream>
#include "modelLoader.h"

objLoader::objLoader(std::string objName) {
	fName = objName;
	numTriangles = 0;
	scene = importer.ReadFile(fName, aiProcess_Triangulate);
}

std::vector<Vertex> objLoader::loadModel() {
	Vertex vecTemp;
	std::vector<Vertex> retVertex;
	aiColor3D color;
	aiString name;
	int x;
	aiString path;
	int texIndex;
		
	for(unsigned int i=0; i<scene->mNumMeshes; i++) {
		std::cout << scene->mNumMaterials << " ";
		const aiMesh* mesh = scene->mMeshes[i];
		const aiMaterial* mat = scene->mMaterials[i+1];
		//aiReturn texName = mat->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		
		if(scene->mNumMaterials > 1) {
			mat->Get(AI_MATKEY_NAME,name);
			mat->Get(AI_MATKEY_COLOR_DIFFUSE,color);
			//std::cout << path.length;
		}
		
		for(unsigned int j=0; j<mesh->mNumFaces; j++) {
			const aiFace &face = mesh->mFaces[j];
			for(int k=0; k<3; k++) {
				aiVector3D pos = mesh->mVertices[face.mIndices[k]];
				aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[k]];
				vecTemp.position[0] = pos[0];
				vecTemp.position[1] = pos[1];
				vecTemp.position[2] = pos[2];
				//vecTemp.color[0] = color[0];
				//vecTemp.color[1] = color[1];
				//vecTemp.color[2] = color[2];
				vecTemp.tex.x = uv[0];
				vecTemp.tex.y = uv[1];
				//std::cout << uv[0] << " " << uv[1] << std::endl;
				retVertex.push_back(vecTemp);
				numTriangles++;
			}
		}
	}
	return retVertex;
}

void objLoader::textureLoader() {

}


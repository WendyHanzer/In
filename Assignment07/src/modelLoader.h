#include <string>
#include <GL/glew.h>
#include <vector>
#include "mystructs.h"
#include <assimp/Importer.hpp> //importer
#include <assimp/scene.h> //aiscene
#include <assimp/postprocess.h> //postprocessing

class objLoader {
	public:
		objLoader(std::string objName);
		std::vector<Vertex> loadModel();
		void textureLoader();
		int numTriangles;
		
	private:
		std::string fName;
		Assimp::Importer importer;
		const aiScene *scene;
};

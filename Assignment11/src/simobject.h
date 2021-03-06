#ifndef SIMOBJECT_H
#define SIMOBJECT_H

// disable 3rd party library warnings on Apple
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>

#include <string>
#include <stdexcept>

#include "vertex.h"
#include "modelloader.h"

// re-enable warnings
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

class SimObject
{
public:
	// constructor and destructor
	SimObject(GLuint program = 0, btScalar mass = 1, std::string modelFile = "cube.obj", btVector3 vec = btVector3(0,0,0));
	virtual ~SimObject();

	// functions to update the object
	virtual void update();
	virtual void render(bool ambient, bool specular, bool diffuse);
	void move(btVector3 pos = btVector3(0,0,0));
	void rotate(float angle, btVector3 y = btVector3(0,1,0));
	virtual void reset();

	// getter and setter functions
	virtual btRigidBody* getMesh() const;
	virtual void setModel(glm::mat4 newModel);
	virtual btVector3 getPosition() const;

protected:
	// OpenGL variable locations
	GLint loc_mvp;
	GLint loc_position;
	GLint loc_texture;
	GLint loc_texCoord;
	GLint loc_hasTexture;
	GLint loc_color;
	GLint loc_normals;

	// member variables
	GLuint vbo;
	int triangleCount, textureCount;
    bool hasTexture;
	glm::mat4 model;
	ModelLoader ml;
	Vertex lighting;

	btRigidBody *meshBody;
};

#endif // SIM_OBJECT_H

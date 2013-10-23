#ifndef SIMOBJECT_H
#define SIMOBJECT_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <btBulletDynamicsCommon.h>

#include <string>
#include <stdexcept>

#include "vertex.h"
#include "modelloader.h"

class SimObject
{
public:
	SimObject(GLuint program = 0, btScalar mass = 1, std::string modelFile = "cube.obj", btVector3 vec = btVector3(0,0,0));
	virtual ~SimObject();

	virtual void update(float dt);
	virtual void render();
	
	virtual btRigidBody* getMesh() const;
	virtual void setModel(glm::mat4 newModel);

protected:
	GLint loc_mvp;
	GLint loc_position;
	GLint loc_texture;
	GLint loc_texCoord;
	GLuint vbo;
	int triangleCount, textureCount;

	glm::mat4 model;
	
	btRigidBody *meshBody;
};

#endif // SIM_OBJECT_H

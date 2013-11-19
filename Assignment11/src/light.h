#ifndef LIGHT_H
#define LIGHT_H

#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>

#include "simobject.h"

class Light {
public:
	Light(GLuint program, const glm::vec3& pos = glm::vec3(0,0,0));
	virtual ~Light() {}

	void enableTracking(SimObject *objectToTrack);
	void disableTracking();
	bool tracking() const;

	virtual void render(bool ambient = true, bool specular = true, bool diffuse = true);
	virtual void update();

protected:
	glm::vec3 position;
	SimObject *trackingObject;

	GLint loc_diffuse, loc_specular, loc_ambient;
	GLint loc_shininess;
	GLint loc_lightPos;
};

#endif // LIGHT_H
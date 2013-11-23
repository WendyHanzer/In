#ifndef LIGHT_H
#define LIGHT_H

// disable 3rd party library warnings on Apple
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>

#include "simobject.h"

// re-enable warnings
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

class Light {
public:
	// constructor and destructor
	Light(GLuint program, const glm::vec3& pos = glm::vec3(0,0,0));
	virtual ~Light() {}

	// tracking information
	void enableTracking(SimObject *objectToTrack);
	void disableTracking();
	bool tracking() const;

	// render lighting and update position values
	virtual void render(bool ambient = true, bool specular = true, bool diffuse = true);
	virtual void update();

protected:
	// member variables
	glm::vec3 position;
	SimObject *trackingObject;

	// OpenGL variable locations
	GLint loc_diffuse, loc_specular, loc_ambient;
	GLint loc_shininess;
	GLint loc_lightPos;
};

#endif // LIGHT_H
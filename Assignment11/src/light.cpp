#include "light.h"

Light::Light(GLuint program, const glm::vec3& pos)
	: position(pos), trackingObject(nullptr)
{
	loc_ambient = glGetUniformLocation(program, "ambient");
	loc_diffuse = glGetUniformLocation(program, "diffuse");
	loc_specular = glGetUniformLocation(program, "specular");
	loc_lightPos = glGetUniformLocation(program, "lightPosition");
	loc_shininess = glGetUniformLocation(program, "shininess");

	if(loc_ambient == -1 || loc_diffuse == -1
			|| loc_specular == -1 || loc_lightPos == -1
				|| loc_shininess == -1) {
		std::cerr << loc_ambient << "\n"
				  << loc_diffuse << "\n"
				  << loc_specular << "\n"
				  << loc_lightPos << "\n"
				  << loc_shininess << std::endl;

	  	throw std::runtime_error("Unable to locate objects in Light::Light");
	}
}

void Light::enableTracking(SimObject *objectToTrack)
{
	if(!objectToTrack) {
		std::cerr << "Unable to track object!" << std::endl;
		throw std::runtime_error("Null Object Passed to Light::enableTracking");
	}

	trackingObject = objectToTrack;
}

void Light::disableTracking()
{
	trackingObject = nullptr;
}

bool Light::tracking() const
{
	return trackingObject ? true : false;
}

void Light::render(bool ambient, bool specular, bool diffuse)
{
    if(ambient)
        glUniform4f(loc_ambient, 1.0f, 1.0f, 1.0f, 0.0f);

    else
        glUniform4f(loc_ambient, 0.0f, 0.0f, 0.0f, 0.0f);

    if(diffuse)
        glUniform4f(loc_diffuse, 1.0f, 1.0f, 1.0f, 0.0f);

    else
        glUniform4f(loc_diffuse, 0.0f, 0.0f, 0.0f, 0.0f);

    if(specular)
        glUniform4f(loc_specular, 1.0f, 1.0f, 1.0f, 0.0f);

    else
        glUniform4f(loc_specular, 0.0f, 0.0f, 0.0f, 0.0f);

    glUniform1f(loc_shininess, 0.9f);

    glUniform4f(loc_lightPos, position[0], position[1], position[2], 0.0f);
}

void Light::update()
{
	if(!trackingObject)
		return;

	auto objectPosition = trackingObject->getPosition();

	// do something with position for spotlight
	position[0] = objectPosition.x();
	position[1] = objectPosition.y()+1;
	position[2] = objectPosition.z();
}
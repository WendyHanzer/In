#include "light.h"

// constructor
Light::Light(GLuint program, const glm::vec3& pos)
	: position(pos), trackingObject(nullptr)
{
	// get all attribute locations from OpenGL program
	loc_ambient = glGetUniformLocation(program, "ambient");
	loc_diffuse = glGetUniformLocation(program, "diffuse");
	loc_specular = glGetUniformLocation(program, "specular");
	loc_lightPos = glGetUniformLocation(program, "lightPosition");
	loc_shininess = glGetUniformLocation(program, "shininess");

	// if any location not found, throw an error
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
	// if object passed doesn't exist, throw an error
	if(!objectToTrack) {
		std::cerr << "Unable to track object!" << std::endl;
		throw std::runtime_error("Null Object Passed to Light::enableTracking");
	}

	// set tracking object
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
	// render lights if flag is true, otherwise false
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

    // set shininess value
    glUniform1f(loc_shininess, 0.9f);

    // update light position
    glUniform4f(loc_lightPos, position[0], position[1], position[2], 0.0f);
}

void Light::update()
{
	// if not tracking an object there is no need to update
	if(!trackingObject)
		return;

	// get object position
	auto objectPosition = trackingObject->getPosition();

	// set position from object position
	position[0] = objectPosition.x();
	position[1] = objectPosition.y()+1;
	position[2] = objectPosition.z();
}
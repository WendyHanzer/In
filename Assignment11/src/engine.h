#ifndef ENGINE_H
#define ENGINE_H

// disable 3rd party library warnings on Apple
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

#include <GL/glew.h>
#ifndef __APPLE__
#include <GL/freeglut.h>

#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <chrono>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "shaderloader.h"
#include "simobject.h"
#include "light.h"

// re-enable warnings
#ifdef __APPLE__
#pragma clang diagnostic pop
#endif

class Engine
{
public:
	// critical functions
	static void init(int argc, char **argv);
	static int run();
	static void cleanUp();
	static float getDT();
	static glm::mat4 getView();
	static glm::mat4 getProjection();

	// glut callback functions
	static void render();
	static void update();
	static void score();
	static void reshape(int new_width, int new_height);

	// input functions
	static void keyboard(unsigned char key, int x_pos, int y_pos);
	static void keyboardSpecial(int key, int x_pos, int y_pos);
	static void keyboardUp(unsigned char key, int x_pos, int y_pos);
	static void keyboardSpecialUp(int key, int x_pos, int y_pos);
	static void keyboardHandle();
	static void mouse(int button, int state, int x_pos, int y_pos);
	static void mouseMovement(int x_pos, int y_pos);
	static void createMenus();
	static void menuActions(int option);

	static void renderText(const char *text, glm::vec2 pos = glm::vec2(-0.25f,-0.85f),
	    glm::vec3 color = glm::vec3(1.0f,1.0f,1.0f));


private:
	static void initPhysics();

	// member variables
	static int width, height;
	static ShaderLoader vertexShader, fragmentShader;
	static bool paused, initialized;
	static bool ambient, specular, diffuse;
	static std::string vertexFile;
	static std::string fragmentFile;
	static std::string scoreText;
	static int triangleCount;
	static float zoom;
	static int textureCount;
	static std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
	static GLuint program;
	static std::vector<SimObject*> objects;
	static glm::mat4 view;
	static glm::mat4 projection;
	static bool keyStates[256];
	static bool keyStatesSpecial[256];
	static bool rightClick, leftClick, defaultCam;
	static float mouseX, mouseY, posX, posY, distance, posZ;
	static float boardAngle, boardAngle2;
	static float gameTime;
	static int gameScore;

	static std::vector<Light*> lights;


	// physics
	static btDiscreteDynamicsWorld* simulation;
	static btRigidBody *body1, *body2;
};

#endif // ENGINE_H

#ifndef ENGINE_H
#define ENGINE_H

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

#include "shaderloader.h"
//#include "modelloader.h"
#include "simobject.h"

class Engine
{
public:
	static void init(int argc, char **argv);
	static int run();
	static void cleanUp();
	static float getDT();
	static glm::mat4 getView();
	static glm::mat4 getProjection();

	// glut callback functions
	static void render();
	static void update();
	static void reshape(int new_width, int new_height);
	static void keyboard(unsigned char key, int x_pos, int y_pos);
	static void keyboardSpecial(int key, int x_pos, int y_pos);
	static void mouse(int button, int state, int x_pos, int y_pos);

private:
	static void initPhysics();

	// member variables
	static int width, height;
	static ShaderLoader vertexShader, fragmentShader;
	//static ModelLoader modelLoader;
	static bool paused, initialized;
	static std::string modelFile;
	static std::string vertexFile;
	static std::string fragmentFile;
	static int triangleCount;
	static float zoom;
	static int textureCount;	
	static std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
	static GLuint program;
	static std::vector<SimObject*> objects;
	static glm::mat4 view;
	static glm::mat4 projection;

	// physics
	static btDiscreteDynamicsWorld* simulation;
	static btRigidBody *body1, *body2;
};

#endif // ENGINE_H

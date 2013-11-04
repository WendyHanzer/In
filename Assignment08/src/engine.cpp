#include "engine.h"

int Engine::width = 640, Engine::height = 480;
ShaderLoader Engine::vertexShader(GL_VERTEX_SHADER), 
			Engine::fragmentShader(GL_FRAGMENT_SHADER);
//ModelLoader Engine::modelLoader;
bool Engine::paused = false, Engine::initialized = false;
std::string Engine::modelFile("cube.obj");
std::string Engine::vertexFile("shaders/texvert.glslf"),
			Engine::fragmentFile("shaders/texfrag.glslf");
int Engine::triangleCount = 0;
float Engine::zoom = -20.0f;
int Engine::textureCount = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> Engine::t1, Engine::t2;
std::vector<SimObject*> Engine::objects;
glm::mat4 Engine::view;
glm::mat4 Engine::projection;
GLuint Engine::program;

btDiscreteDynamicsWorld* Engine::simulation = nullptr;
btRigidBody *Engine::body1 = nullptr, *Engine::body2 = nullptr;

void Engine::init(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(width,height);
	glutCreateWindow("Bullet Physics");

	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutMouseFunc(mouse);

    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        throw std::runtime_error("GLEW initialization failed!");
    }

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_TEXTURE_2D);
	
	view = glm::lookAt(glm::vec3(-4.0,10.0,-9.0),
					   glm::vec3(0.0,0.0,0.0),
					   glm::vec3(0.0,1.0,0.0));

	projection = glm::perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);

	btBroadphaseInterface *broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfig);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	
	simulation = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	initPhysics();

    //--load shaders
    if(!vertexShader.load("shaders/vertex.glslv") || !fragmentShader.load("shaders/frag.glslf"))
        return;

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = ShaderLoader::linkShaders({vertexShader, fragmentShader}); 

	objects.push_back(new SimObject(program, 0, "hockeytable2.obj", btVector3(0,1,0)));
	objects.push_back(new SimObject(program, 1, "puck.obj", btVector3(0,5,0)));
	objects.push_back(new SimObject(program, 1, "cylinder.obj", btVector3(-3,15,0)));
	//objects.push_back(new SimObject(program, "table.obj", btVector3(1,3,0)));
	
	for(SimObject *object : objects) {
		simulation->addRigidBody(object->getMesh());
	}
	
	initialized = true;

}

void myMouseFunc(int x, int y)
{
std::cout << x << " " << y;
//mouse_x = x; place current mouse pos in mouse_x
//mouse_y = y;

}

int Engine::run()
{
	if(!initialized)
		throw std::runtime_error("Engine::init() must be called first!");

	t1 = std::chrono::high_resolution_clock::now();
	//glutMouseFunc(int button, int state, int x,int y);
	glutMainLoop();

	cleanUp();
	return 0;	
}

void Engine::cleanUp()
{
	for(SimObject* object : objects) {
		delete object;
	}
}

float Engine::getDT()
{
	t2 = std::chrono::high_resolution_clock::now();
	float ret = std::chrono::duration_cast<std::chrono::duration<float>>(t2-t1).count();
	t1 = std::chrono::high_resolution_clock::now();
	return ret;
}

glm::mat4 Engine::getView()
{
	return view;
}

glm::mat4 Engine::getProjection()
{
	return projection;
}

void Engine::render()
{
	glClearColor(0.0,0.0,0.2,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	for(SimObject* object : objects) {
		object->render();
	}

	
	glutSwapBuffers();
}

void Engine::update()
{
	float dt = getDT();

	btScalar m[16];
	glm::mat4 glMat;
	btTransform trans;
	
	simulation->stepSimulation(dt);

	for(SimObject *object : objects) {

		object->getMesh()->getMotionState()->getWorldTransform(trans);

		trans.getOpenGLMatrix(m);

		int k = 0;
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++) {
				glMat[i][j] = m[k++];
			}

		object->setModel(glMat);
	}
	
	glutPostRedisplay();
}

void Engine::reshape(int new_width, int new_height)
{
	width = new_width;
	height = new_height;

	glViewport(0,0,width,height);

	projection = glm::perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);
}

void Engine::keyboard(unsigned char key, int x_pos, int y_pos)
{
	switch(key) {
		case 27: // ESC
		#ifndef __APPLE__
			glutLeaveMainLoop();
		#else
			exit(0);
		#endif
			break;
			
		case 32: // SPACE
			objects[1]->getMesh()->setLinearVelocity(btVector3(0,10,0));
			break;
					
		case 'w':
		case 'W':
			objects[1]->getMesh()->setLinearVelocity(btVector3(0,0,5));
			break;
			
		case 's':
		case 'S':
			objects[1]->getMesh()->setLinearVelocity(btVector3(0,0,-5));
			break;
			
		case 'd':
		case 'D':
			objects[1]->getMesh()->setLinearVelocity(btVector3(-5,0,0));
			break;
			
		case 'a':
		case 'A':
			objects[1]->getMesh()->setLinearVelocity(btVector3(5,0,0));
			break;
	}
}

void Engine::keyboardSpecial(int key, int x_pos, int y_pos)
{
}

void Engine::mouse(int button, int state, int x_pos, int y_pos)
{

}

void Engine::initPhysics()
{
	simulation->setGravity(btVector3(0,-10,0));
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
	//btCollisionShape* shape1 = new btBoxShape(btVector3(1,1,1));
	//btCollisionShape* shape2 = new btBoxShape(btVector3(1,1,1));

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,-1,0)));

	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	simulation->addRigidBody(groundRigidBody);
/*
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(1,4,0)));
	btScalar mass = 0;
	btVector3 fallInertia(0,0,0);
	shape1->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo shape1CI(mass,fallMotionState,shape1,fallInertia);
	body1 = new btRigidBody(shape1CI);
	simulation->addRigidBody(body1);

	btDefaultMotionState* fallMotionState2 = new btDefaultMotionState(btTransform(btQuaternion(0.5,0,0.5,1), btVector3(2,30,0)));
	btScalar mass2 = 1;
	btVector3 fallInertia2(0,0,0);
	shape2->calculateLocalInertia(mass2, fallInertia2);
	btRigidBody::btRigidBodyConstructionInfo shape2CI(mass2,fallMotionState2,shape2,fallInertia2);
	body2 = new btRigidBody(shape2CI);
	simulation->addRigidBody(body2);	
*/
}

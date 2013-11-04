#include "engine.h"

#define SPACE 32
#define ESC 27

enum MENU_ITEMS {
    MENU_PAUSE,
    MENU_RESUME,
    MENU_CAMERA,
    MENU_RESTART,
    MENU_EXIT
};

int Engine::width = 640, Engine::height = 480;
ShaderLoader Engine::vertexShader(GL_VERTEX_SHADER),
			Engine::fragmentShader(GL_FRAGMENT_SHADER);
//ModelLoader Engine::modelLoader;
bool Engine::paused = false, Engine::initialized = false;
int Engine::playerOneScore = 0, Engine::playerTwoScore = 0;
//std::string Engine::modelFile("cube.obj");
std::string Engine::vertexFile("shaders/texvert.glslv"),
			Engine::fragmentFile("shaders/texfrag.glslf"),
            Engine::scoreText("Score: ");
int Engine::triangleCount = 0;
float Engine::zoom = -20.0f;
int Engine::textureCount = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> Engine::t1, Engine::t2;
std::vector<SimObject*> Engine::objects(4);
glm::mat4 Engine::view;
glm::mat4 Engine::projection;
GLuint Engine::program;
bool Engine::keyStates[256], Engine::keyStatesSpecial[256];
int Engine::camera = 0;
float Engine::dAngle = 0.0;
int Engine::xOrigin = -1;

btDiscreteDynamicsWorld* Engine::simulation = nullptr;
btRigidBody *Engine::body1 = nullptr, *Engine::body2 = nullptr;

void Engine::init(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(width,height);
	glutCreateWindow("Air Hockey");

	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(keyboardSpecial);
	glutSpecialUpFunc(keyboardSpecialUp);
	glutMouseFunc(mouse);
	
	createMenus();

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

	changeCamera(0);

	projection = glm::perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);

	btBroadphaseInterface *broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfig);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	simulation = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	initPhysics();

    //--load shaders
    if(!vertexShader.load(vertexFile) || !fragmentShader.load(fragmentFile))
        return;

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = ShaderLoader::linkShaders({vertexShader, fragmentShader});

	objects[0] = new SimObject(program, 0, "hockeytable3.obj", btVector3(0,0,0));
	objects[1] = new SimObject(program, 5, "paddle.obj", btVector3(5,0.1,0));
	objects[2] = new SimObject(program, 5, "paddle.obj", btVector3(-5,0.1,0));
	objects[3] = new SimObject(program, 1, "puck.obj", btVector3(0,0,0));
	//objects.push_back(new SimObject(program, "table.obj", btVector3(1,3,0)));

	for(SimObject *object : objects) {
		simulation->addRigidBody(object->getMesh());
	}

	initialized = true;
}

int Engine::run()
{
	if(!initialized)
		throw std::runtime_error("Engine::init() must be called first!");

	t1 = std::chrono::high_resolution_clock::now();
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

    glUseProgram(0);
    
    std::string score = std::string("Player One: ") + std::to_string(playerOneScore);
    renderText(score.c_str(), glm::vec2(-0.85f,-0.85f), glm::vec3(0.0f,1.0f,0.0f));
    
    score = std::string("Player Two: ") + std::to_string(playerTwoScore);
    renderText(score.c_str(), glm::vec2(0.5f,-0.85f), glm::vec3(1.0f,0.0f,0.0f));

	glutSwapBuffers();
}

void Engine::update()
{
    if(paused) {
        t1 = std::chrono::high_resolution_clock::now();
        return;
    }
    
	float dt = getDT();

	keyboardHandle();

	simulation->stepSimulation(dt);

	for(SimObject *object : objects) {
		object->update();
	}

	glutPostRedisplay();
}

void Engine::score(int x) {	

    objects[3]->move(btVector3(0,0,0));
    objects[3]->getMesh()->setLinearVelocity(btVector3(0,0,0));
    
	if(x == 1)
		playerOneScore++;
	if(x == 0)
		playerTwoScore++;
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
    keyStates[key] = true;
}

void Engine::keyboardSpecial(int key, int x_pos, int y_pos)
{
    keyStatesSpecial[key] = true;
}

void Engine::keyboardUp(unsigned char key, int x_pos, int y_pos)
{
    keyStates[key] = false;
}

void Engine::keyboardSpecialUp(int key, int x_pos, int y_pos)
{
    keyStatesSpecial[key] = false;
}

void Engine::changeCamera(int x) {
	if(camera == 0) {
		std::cout << camera;
		view = glm::lookAt(glm::vec3(0.0,15,-5),
						   glm::vec3(0.0,0.0,0.0),
						   glm::vec3(0.0,1.0,0.0));
	}
	else if(camera == 1) {
	    view = glm::lookAt(glm::vec3(20,15,0),
					       glm::vec3(0.0,0.0,0.0),
				     	   glm::vec3(0.0,1.0,0.0));

	}
	else if(camera == 2) {
		view = glm::lookAt(glm::vec3(-20,15,0),
						   glm::vec3(0.0,0.0,0.0),
						   glm::vec3(0.0,1.0,0.0));

	}	
}

void Engine::keyboardHandle()
{
    int x = 0, y = 0, z = 0;
    int x2 = 0, y2 = 0, z2 = 0;
    
    if(keyStates[ESC]) {
	#ifndef __APPLE__
	    glutLeaveMainLoop();
	#else
		exit(0);
	#endif
	}
    
    if(keyStates['w'] || keyStates['W']) {
        if(camera == 0) {z = 8;}
        if(camera == 1) {x = -8;}
        if(camera == 2) {x = 8;}
    }
    
    if(keyStates['a'] || keyStates['A']) {
        if(camera == 0) {x = 8;}
       	if(camera == 1) {z = 8;}
        if(camera == 2) {z = -8;}
    }
    
    if(keyStates['s'] || keyStates['s']) {
        if(camera == 0) {z = -8;}
        if(camera == 1) {x = 8;}
        if(camera == 2) {x = -8;}
    }
    
    if(keyStates['d'] || keyStates['d']) {
        if(camera == 0) {x = -8;}
        if(camera == 1) {z = -8;}
        if(camera == 2) {z = 8;}
    }
    
    if(keyStatesSpecial[GLUT_KEY_UP]) {
        if(camera == 0) {z2 = 8;}
        if(camera == 1) {x2 = -8;}
        if(camera == 2) {x2 = 8;}
    }
    
    if(keyStatesSpecial[GLUT_KEY_LEFT]) {
        if(camera == 0) {x2 = 8;}
       	if(camera == 1) {z2 = 8;}
        if(camera == 2) {z2 = -8;}
    }
    
    if(keyStatesSpecial[GLUT_KEY_DOWN]) {
        if(camera == 0) {z2 = -8;}
        if(camera == 1) {x2 = 8;}
        if(camera == 2) {x2 = -8;}
    }
    
    if(keyStatesSpecial[GLUT_KEY_RIGHT]) {
        if(camera == 0) {x2 = -8;}
        if(camera == 1) {z2 = -8;}
        if(camera == 2) {z2 = 8;}
    }
    
    if(keyStates['z'])
        y = -8;
    
    //if(x || y || z)
        objects[1]->getMesh()->setLinearVelocity(btVector3(x,y,z));
        
    //if(x2 || y2 || z2)
        objects[2]->getMesh()->setLinearVelocity(btVector3(x2,y2,z2));
}

void Engine::mouse(int button, int state, int x_pos, int y_pos)
{

}

void Engine::initPhysics()
{
	simulation->setGravity(btVector3(0,-10,0));
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
    btCollisionShape* shape1 = new btBoxShape(btVector3(0.1,10,10));
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,-1,0)));

	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	simulation->addRigidBody(groundRigidBody);
	
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,10.9,0)));
	btScalar mass = 0;
	btVector3 fallInertia(0,0,0);
	shape1->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo shape1CI(mass,fallMotionState,shape1,fallInertia);
	body1 = new btRigidBody(shape1CI);
	simulation->addRigidBody(body1);
}

void Engine::createMenus()
{
	glutCreateMenu(menuActions);
	
	glutAddMenuEntry("Pause Game", MENU_PAUSE);
	glutAddMenuEntry("Resume Game", MENU_RESUME);
	glutAddMenuEntry("Change Camera Angle", MENU_CAMERA);
	glutAddMenuEntry("Restart Game", MENU_RESTART);
	glutAddMenuEntry("Quit Game", MENU_EXIT);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void Engine::menuActions(int option)
{
	switch(option) {
		case MENU_PAUSE:
			paused = true;
			std::cout << "Game Paused" << std::endl;
		break;
		
		case MENU_RESUME:
		    paused = false;
		    std::cout << "Game Resumed" << std::endl;
	    break;
	    
	    case MENU_CAMERA:
	        camera++;
	        camera %= 3;
	        changeCamera(camera);
	        std::cout << "Camera Changed" << std::endl;
        break;
		
		case MENU_RESTART:
		    resetGame();
			std::cout << "Game Restarted" << std::endl;
		break;
		
		case MENU_EXIT:
		#ifndef __APPLE__
		    glutLeaveMainLoop();
	    #else
			exit(0);
		#endif
		break;
	}
}

void Engine::renderText(const char *text, glm::vec2 pos, glm::vec3 color)
{
    glColor3f(color[0],color[1],color[2]);
    glRasterPos2f(pos[0],pos[1]);
    
#ifndef __APPLE__ // this function does not work on Mac... problems with freeglut
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)text);
#else // apple
	std::string textStr = text;

	for(char c : textStr) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, int(c));
	}
#endif
}

void Engine::resetGame()
{
    playerOneScore = playerTwoScore = 0;
    objects[1]->move(btVector3(5,0.1,0));
    objects[2]->move(btVector3(-5,0.1,0));
    objects[3]->move(btVector3(0,0,0));
    objects[3]->getMesh()->setLinearVelocity(btVector3(0,0,0));
    
}

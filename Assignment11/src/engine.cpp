#include "engine.h"

#define SPACE 32
#define ESC 27

enum MENU_ITEMS {
    MENU_PAUSE,
    MENU_RESUME,
    MENU_RESTART,
    MENU_EXIT
};

int Engine::width = 1280, Engine::height = 720;
ShaderLoader Engine::vertexShader(GL_VERTEX_SHADER),
			Engine::fragmentShader(GL_FRAGMENT_SHADER);
//ModelLoader Engine::modelLoader;
bool Engine::paused = false, Engine::initialized = false;
bool Engine::ambient = true, Engine::specular = true, Engine::diffuse = true;
//std::string Engine::modelFile("cube.obj");
std::string Engine::vertexFile("shaders/vert.vs"),
			Engine::fragmentFile("shaders/frag.fs");
float Engine::zoom = -10.0f;
std::chrono::time_point<std::chrono::high_resolution_clock> Engine::t1, Engine::t2;
std::vector<SimObject*> Engine::objects;
glm::mat4 Engine::view;
glm::mat4 Engine::projection;
GLuint Engine::program;
bool Engine::keyStates[256], Engine::keyStatesSpecial[256];
bool Engine::rightClick = false, Engine::leftClick = false, Engine::defaultCam = true;
float Engine::mouseX, Engine::mouseY, Engine::posX = 0, Engine::posY = 0, Engine::distance = 20, Engine::posZ = -5;
float Engine::boardAngle = 0, Engine::boardAngle2 = 0;
std::vector<Light*> Engine::lights;
float Engine::gameTime = 0.0f;
int Engine::gameScore = 0;

btDiscreteDynamicsWorld* Engine::simulation = nullptr;
btRigidBody *Engine::body1 = nullptr, *Engine::body2 = nullptr;

void Engine::init(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(width,height);
	glutCreateWindow("Labyrinth");

	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(keyboardSpecial);
	glutSpecialUpFunc(keyboardSpecialUp);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMovement);

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

	view = glm::lookAt(glm::vec3(0.0,20.0,20),
					   glm::vec3(0.0,0.0,0.0),
					   glm::vec3(0.0,1.0,0.0));

	projection = glm::perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);

	initPhysics();

    //--load shaders
    if(!vertexShader.load(vertexFile) || !fragmentShader.load(fragmentFile))
        return;

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = ShaderLoader::linkShaders({vertexShader, fragmentShader});

    objects.push_back(new SimObject(program, 0, "board.obj", btVector3(0,0,0)));
	objects.push_back(new SimObject(program, 1, "ball.obj", btVector3(0,2,0)));

	objects[0]->getMesh()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);


	for(SimObject *object : objects) {
		simulation->addRigidBody(object->getMesh());
	}

	lights.push_back(new Light(program, glm::vec3(0,5,0)));

	//lights[0]->enableTracking(objects[1]);
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

	for(Light *light : lights) {
		delete light;
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
	glClearColor(0.0,0.5,0.5,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	for(SimObject* object : objects) {
		object->render(ambient, specular, diffuse);
	}

	for(Light *light : lights) {
		light->render(ambient, specular, diffuse);
	}

    glUseProgram(0);

    std::string text = specular ? "Specular: On" : "Specular: Off";
    renderText(text.c_str(), glm::vec2(-0.95,0.8), glm::vec3(0.0,0.0,0.0));

    text = ambient ? "Ambient: On" : "Ambient: Off";
    renderText(text.c_str(), glm::vec2(-0.95,0.73), glm::vec3(0.0,0.0,0.0));

    text = diffuse ? "Diffuse: On" : "Diffuse: Off";
    renderText(text.c_str(), glm::vec2(-0.95,0.66), glm::vec3(0.0,0.0,0.0));

    char textBuffer[256];
    sprintf(textBuffer, "Time: %.2f", gameTime);
    renderText(textBuffer, glm::vec2(0.65,0.85), glm::vec3(0.0,0.0,0.0));

    sprintf(textBuffer, "Ball Count: %d", gameScore);
    renderText(textBuffer, glm::vec2(-0.25, 0.85), glm::vec3(0.0,0.0,0.0));
	glutSwapBuffers();
}

void Engine::update()
{
    if(paused) {
        t1 = std::chrono::high_resolution_clock::now();
        return;
    }

	float dt = getDT();

	gameTime += dt;

	keyboardHandle();

	simulation->stepSimulation(dt);

	for(SimObject *object : objects) {
		object->update();
	}

	for(Light *light : lights) {
		light->update();
	}

	glutPostRedisplay();
}

void Engine::score()
{
	gameScore++;
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
    keyStates[int(key)] = true;

    switch(key) {
        case 'a':
        case 'A':
            ambient = !ambient;
        break;

        case 's':
        case 'S':
            specular = !specular;
        break;

        case 'd':
        case 'D':
            diffuse = !diffuse;
        break;
		case 32:
			defaultCam = true;
			view = glm::lookAt(glm::vec3(0.0,25.0,0.1),
					   glm::vec3(0.0,0.0,0.0),
					   glm::vec3(0.0,1.0,0.0));
		break;
    }
}

void Engine::keyboardSpecial(int key, int x_pos, int y_pos)
{
    keyStatesSpecial[key] = true;
}

void Engine::keyboardUp(unsigned char key, int x_pos, int y_pos)
{
    keyStates[int(key)] = false;
}

void Engine::keyboardSpecialUp(int key, int x_pos, int y_pos)
{
    keyStatesSpecial[key] = false;
}

void Engine::keyboardHandle()
{
    if(keyStates[ESC]) {
	#ifndef __APPLE__
	    glutLeaveMainLoop();
	#else
		exit(0);
	#endif
	}
/*
	if(keyStates[int('a')]) {
		ambient = !ambient;
	}

	if(keyStates[int('s')]) {
	    specular = !specular;
	}

	if(keyStates[int('d')]) {
	    diffuse = !diffuse;
    }
*/
    if(keyStatesSpecial[GLUT_KEY_RIGHT]) {
    	boardAngle -= 0.001;

    }

    if(keyStatesSpecial[GLUT_KEY_LEFT]) {
        boardAngle += 0.001;

    }

    if(keyStatesSpecial[GLUT_KEY_UP]) {
    	boardAngle2 -= 0.001;

    }

    if(keyStatesSpecial[GLUT_KEY_DOWN]) {
    	boardAngle2 += 0.001;

    }
	
        btTransform trans;
        objects[0]->getMesh()->getMotionState()->getWorldTransform(trans);
        auto rotation = trans.getRotation();
        rotation += btQuaternion(btVector3(0,0,1), boardAngle) + btQuaternion(btVector3(1,0,0), boardAngle2);
        trans.setRotation(rotation);
        objects[0]->getMesh()->getMotionState()->setWorldTransform(trans);
	
}


void Engine::mouse(int button, int state, int x_pos, int y_pos)
{
	if(button == GLUT_RIGHT_BUTTON) {
		if(state == GLUT_UP)
			rightClick = false;

		else {
			rightClick = true;
			defaultCam = false;
		}
	}

	if(button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_UP)
			leftClick = false;

		else
			leftClick = true;
	}

}

void Engine::mouseMovement(int x_pos, int y_pos)
{
	static float angle = 0.0f;

	if(paused)
		return;

	if(rightClick) {
		if (x_pos > mouseX)
			angle += 0.1f;
		else
			angle -= 0.1f;

		mouseX = x_pos;
		mouseY = y_pos;

		view = glm::lookAt(glm::vec3(distance * sin(angle),distance,distance * cos(angle)),
						   glm::vec3(0,0,0),
						   glm::vec3(0,1,0));

	}

	else if(leftClick) {
		if(defaultCam) {
			if(x_pos > mouseX)
				boardAngle -= 0.1;
			else
				boardAngle += 0.1;
			if(y_pos > mouseY)
				boardAngle2 += 0.1;
			else
				boardAngle2 -= 0.1;
			
			mouseX = x_pos;
			mouseY = y_pos;
		}
		else if(distance*sin(angle) >= 14 && distance * cos(angle) <= 14 ) {
			if(x_pos > mouseX)
				boardAngle2 -= 0.1;
			else
				boardAngle2 += 0.1;
			if(y_pos > mouseY)
				boardAngle -= 0.1;
			else
				boardAngle += 0.1;
			
			mouseX = x_pos;
			mouseY = y_pos;
			
		}
		
		else if(distance*sin(angle) <= 14 && distance * cos(angle) <= -14 ) {
			if(x_pos > mouseX)
				boardAngle += 0.1;
			else
				boardAngle -= 0.1;
			if(y_pos > mouseY)
				boardAngle2 -= 0.1;
			else
				boardAngle2 += 0.1;
		
			mouseX = x_pos;
			mouseY = y_pos;

		}

		else if(distance*sin(angle) <= -14 && distance * cos(angle) <= 14 ) {
			if(x_pos > mouseX)
				boardAngle2 += 0.1;
			else
				boardAngle2 -= 0.1;
			if(y_pos > mouseY)
				boardAngle += 0.1;
			else
				boardAngle -= 0.1;
			
			mouseX = x_pos;
			mouseY = y_pos;

		}

		else if(distance*sin(angle) >= -14 && distance * cos(angle) >= 14 ) {
			if(x_pos > mouseX)
				boardAngle -= 0.1;
			else
				boardAngle += 0.1;
			if(y_pos > mouseY)
				boardAngle2 += 0.1;
			else
				boardAngle2 -= 0.1;
			
			mouseX = x_pos;
			mouseY = y_pos;

		}

		
		
		btTransform trans;
        objects[0]->getMesh()->getMotionState()->getWorldTransform(trans);
        auto rotation = trans.getRotation();
        rotation += btQuaternion(btVector3(0,0,1), boardAngle) + btQuaternion(btVector3(1,0,0), boardAngle2);
        trans.setRotation(rotation);
        objects[0]->getMesh()->getMotionState()->setWorldTransform(trans);
	}
}

void Engine::initPhysics()
{
	btBroadphaseInterface *broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfig);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
	simulation = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);

	simulation->setGravity(btVector3(0,-50,0));
	//btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
	//btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,-1,0)));

	//btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
	//btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	//simulation->addRigidBody(groundRigidBody);

	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
/*
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,10.9,0)));
	btScalar mass = 0;
	btVector3 fallInertia(0,0,0);
	shape1->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo shape1CI(mass,fallMotionState,shape1,fallInertia);
	body1 = new btRigidBody(shape1CI);
	simulation->addRigidBody(body1);
*/
}

void Engine::createMenus()
{
	glutCreateMenu(menuActions);

	glutAddMenuEntry("Pause", MENU_PAUSE);
	glutAddMenuEntry("Resume", MENU_RESUME);
	glutAddMenuEntry("Restart", MENU_RESTART);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void Engine::menuActions(int option)
{
	switch(option) {
		case MENU_PAUSE:
			paused = true;
		break;

		case MENU_RESUME:
			paused = false;
		break;

		case MENU_RESTART:
			gameTime = 0.0;
			gameScore = 0;

			boardAngle = boardAngle2 = 0.0;

			objects[0]->rotate(0,btVector3(1,1,1));
			objects[1]->reset();
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
    glRasterPos2f(pos[0],pos[1]);
	glColor4f(color[0],color[1],color[2], 1.0f);

#ifndef __APPLE__ // this function does not work on Mac... problems with freeglut
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)text);
#else // apple
	std::string textStr = text;

	for(char c : textStr) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, int(c));
	}
#endif
}

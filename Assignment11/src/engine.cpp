#include "engine.h"

#define SPACE 32
#define ESC 27

enum MENU_ITEMS {
    MENU_PAUSE,
    MENU_RESUME,
    MENU_RESTART,
    MENU_EXIT
};

// initialize all static variables
int Engine::width = 1280, Engine::height = 720;
ShaderLoader Engine::vertexShader(GL_VERTEX_SHADER),
			Engine::fragmentShader(GL_FRAGMENT_SHADER);
bool Engine::paused = false, Engine::initialized = false;
bool Engine::ambient = true, Engine::specular = true, Engine::diffuse = true;
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
std::vector<std::string> Engine::topTenScores(10);

btDiscreteDynamicsWorld* Engine::simulation = nullptr;
btRigidBody *Engine::body1 = nullptr, *Engine::body2 = nullptr;

void Engine::init(int argc, char **argv)
{
	// init glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(width,height);
	glutCreateWindow("Labyrinth");

	// set up glut callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(keyboardSpecial);
	glutSpecialUpFunc(keyboardSpecialUp);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMovement);

	// create popup menu
	createMenus();

	// initialize GLEW
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        throw std::runtime_error("GLEW initialization failed!");
    }

    // init OpenGL functions
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_TEXTURE_2D);

	// init view matrix
	view = glm::lookAt(glm::vec3(0.0,25.0,0.1),
					   glm::vec3(0.0,0.0,0.0),
					   glm::vec3(0.0,1.0,0.0));

	// init projection matrix
	projection = glm::perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);

	// initialize physics engine
	initPhysics();

    // load shaders
    if(!vertexShader.load(vertexFile) || !fragmentShader.load(fragmentFile))
        return;

    // link shaders
    program = ShaderLoader::linkShaders({vertexShader, fragmentShader});

    // create board and ball
    objects.push_back(new SimObject(program, 0, "board.obj", btVector3(0,0,0)));
	objects.push_back(new SimObject(program, 1, "ball.obj", btVector3(0,0.1,0)));
	objects.push_back(new SimObject(program, 0, "boardTop.obj", btVector3(0,0.1,0)));

	// set up board as kinematic object
	objects[0]->getMesh()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
	objects[2]->getMesh()->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);

	// add objects to the simulation enviornment
	for(SimObject *object : objects) {
		simulation->addRigidBody(object->getMesh());
	}

	// create lights
	lights.push_back(new Light(program, glm::vec3(0,5,0)));
	lights.push_back(new Light(program, glm::vec3(0,1,0)));

	lights[1]->enableTracking(objects[1]);

	// initialize top scores
	for(int i = 1; i <= 10; i++) {
			topTenScores[i-1] = std::string("Time: 0.00   Fail Count: 0");
	}

	// set initialized flag to true
	initialized = true;
}

int Engine::run()
{
	// if Engine::init not called or failed, throw exception
	if(!initialized)
		throw std::runtime_error("Engine::init() must be called first!");

	// initialize clock
	t1 = std::chrono::high_resolution_clock::now();

	// enter glut main event loop
	glutMainLoop();

	// clean up objects
	cleanUp();

	return 0;
}

void Engine::cleanUp()
{
	// delete all simulation objects
	for(SimObject* object : objects) {
		delete object;
	}

	// delete all lights
	for(Light *light : lights) {
		delete light;
	}
}

float Engine::getDT()
{
	// get difference in time between ticks
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

void Engine::reset()
{
	// reset objects to inital positions
	objects[0]->rotate(0,btVector3(1,1,1));
	objects[2]->rotate(0,btVector3(1,1,1));
	objects[1]->reset();

	boardAngle = boardAngle2 = 0;
}

void Engine::render()
{
	// text buffer for rendering text
	char textBuffer[256];
	// variables for rendering scores
	int i = 1;
	float height = 0.68;
	
	// init GL background color and clear buffer bits
	glClearColor(0.0,0.5,0.5,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use main shader program
	glUseProgram(program);

	// render all objects
	//for(SimObject* object : objects) {
	//	object->render(ambient, specular, diffuse);
	//}
	// render everything except the cover
	for(int i=0; i<2; i++) {
		objects[i]->render(ambient,specular,diffuse);
	}

	// render all lights
	for(Light *light : lights) {
		light->render(ambient, specular, diffuse);
	}

	// disable main shader program
    glUseProgram(0);

    // render specular light text
    std::string text = specular ? "Specular: On" : "Specular: Off";
    renderText(text.c_str(), glm::vec2(-0.95,0.78), glm::vec3(0.0,0.0,0.0));

    // render ambient light text
    text = ambient ? "Ambient: On" : "Ambient: Off";
    renderText(text.c_str(), glm::vec2(-0.95,0.71), glm::vec3(0.0,0.0,0.0));

    // render diffuse light text
    text = diffuse ? "Diffuse: On" : "Diffuse: Off";
    renderText(text.c_str(), glm::vec2(-0.95,0.64), glm::vec3(0.0,0.0,0.0));

    // render current game text
    text = "Current Game";
    renderText(text.c_str(), glm::vec2(0.6, 0.92), glm::vec3(0.0,0.0,0.0));

    // fill buffer with value and render time text
    sprintf(textBuffer, "Time: %.2f", gameTime);
    renderText(textBuffer, glm::vec2(0.6,0.85), glm::vec3(0.0,0.0,0.0));

    // fill buffer with value and render game score
    sprintf(textBuffer, "Fail Count: %d", gameScore);
    renderText(textBuffer, glm::vec2(0.8, 0.85), glm::vec3(0.0,0.0,0.0));

	text = "Top Ten Scores";
	renderText(text.c_str(), glm::vec2(0.6,0.75), glm::vec3(0.0,0.0,0.0));

	// render top 10 scores
	for(std::string scoreStr : topTenScores) {
		sprintf(textBuffer, "%d.", i++);
		renderText(textBuffer, glm::vec2(0.6,height), glm::vec3(0.0,0.0,0.0));
		renderText(scoreStr.c_str(), glm::vec2(0.64,height), glm::vec3(0.0,0.0,0.0));
		height -= 0.07;
	}

    // draw to the screen
	glutSwapBuffers();
}

void Engine::update()
{
	// if paused update clock tick and do nothing
    if(paused) {
        t1 = std::chrono::high_resolution_clock::now();
        return;
    }

    // get difference in time tick
	float dt = getDT();

	// add change in time to game time
	gameTime += dt;

	// trigger keyboard actions
	keyboardHandle();

	// step physics
	simulation->stepSimulation(dt);

	// update all objects
	for(SimObject *object : objects) {
		object->update();
	}

	// update all lights
	for(Light *light : lights) {
		light->update();
	}

	// trigger render event
	glutPostRedisplay();
}

void Engine::score(int x)
{
	if(x == 0) gameScore++;
	if(x == 1) {
		// comparator function for sorting scores
		auto scoreCmp = [](const std::string& str1, const std::string& str2) -> float {
			std::stringstream ss(str1);
			std::string timeText;
			float t1, t2;

			ss >> timeText >> t1;
			ss.str(str2);
			ss >> timeText >> t2;

			if(t1 == 0.0 && t2 != 0.0)
				return t2 < t1;

			if (t1 != 0.0 && t2 == 0.0)
				return t1 > t2;

			else
				return t1 < t2;
		};

		// buffer for score text
		char buffer[256];

		// load buffer with current values
		sprintf(buffer, "Time: %.2f   Fail Count: %d", gameTime, gameScore);

		// add score to top ten
		topTenScores.push_back(std::string(buffer));

		// sort scores
		std::sort(topTenScores.begin(), topTenScores.end(), scoreCmp);

		// remove 11th score
		topTenScores.pop_back();

		// reset game values
		gameTime = 0.0;
		gameScore = 0;

		// reset board and ball position
		reset();
	}
}

void Engine::reshape(int new_width, int new_height)
{
	// update width and height
	width = new_width;
	height = new_height;

	// reset viewport
	glViewport(0,0,width,height);

	// update projection matrix with new width and height
	projection = glm::perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);
}

void Engine::keyboard(unsigned char key, int x_pos, int y_pos)
{
	// set state of keypress to true
    keyStates[int(key)] = true;

    // special key cases
    switch(key) {
    	// toggle ambient light
        case 'a':
        case 'A':
            ambient = !ambient;
        break;

        // toggle specular light
        case 's':
        case 'S':
            specular = !specular;
        break;

        // toggle diffuse light
        case 'd':
        case 'D':
            diffuse = !diffuse;
        break;

        case 't':
        case 'T':
        	score(1);
        break;
        // if space is pressed reset to default camera
		case SPACE:
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
	// if Escape pressed quit
    if(keyStates[ESC]) {
	#ifndef __APPLE__
	    glutLeaveMainLoop();
	#else
		exit(0);
	#endif
	}

	// set keyboard mapping for precise controls
    if(keyStatesSpecial[GLUT_KEY_RIGHT]) {
    	boardAngle -= 0.01;

    }

    if(keyStatesSpecial[GLUT_KEY_LEFT]) {
        boardAngle += 0.01;

    }

    if(keyStatesSpecial[GLUT_KEY_UP]) {
    	boardAngle2 -= 0.01;

    }

    if(keyStatesSpecial[GLUT_KEY_DOWN]) {
    	boardAngle2 += 0.01;

    }

    // limit board rotation
	if(boardAngle > 0.5) boardAngle = 0.5;
	if(boardAngle < -0.5) boardAngle = -0.5;
	if(boardAngle2 > 0.5) boardAngle2 = 0.5;
	if(boardAngle2 < -0.5) boardAngle2 = -0.5;
	
	// update board with new rotation value
    btTransform trans;
    objects[0]->getMesh()->getMotionState()->getWorldTransform(trans);
    auto rotation = trans.getRotation();
    rotation += btQuaternion(btVector3(0,0,1), boardAngle) + btQuaternion(btVector3(1,0,0), boardAngle2);
    trans.setRotation(rotation);
    objects[0]->getMesh()->getMotionState()->setWorldTransform(trans);

	objects[2]->getMesh()->getMotionState()->getWorldTransform(trans);
    rotation = trans.getRotation();
    rotation += btQuaternion(btVector3(0,0,1), boardAngle) + btQuaternion(btVector3(1,0,0), boardAngle2);
    trans.setRotation(rotation);
    objects[2]->getMesh()->getMotionState()->setWorldTransform(trans);

}


void Engine::mouse(int button, int state, int x_pos, int y_pos)
{
	// if right click set flags
	if(button == GLUT_RIGHT_BUTTON) {
		if(state == GLUT_UP)
			rightClick = false;

		else {
			rightClick = true;
			defaultCam = false;
		}
	}

	// if left click set flags
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
	static float speed = 0.01f;

	// if paused don't update mouse values
	if(paused)
		return;

	// if right click update camera on move
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

	// if left click update board angle
	else if(leftClick) {
		if(defaultCam) {
			if(x_pos > mouseX)
				boardAngle -= speed;
			else
				boardAngle += speed;
			if(y_pos > mouseY)
				boardAngle2 += speed;
			else
				boardAngle2 -= speed;
			
			mouseX = x_pos;
			mouseY = y_pos;
		}
		else if(distance*sin(angle) >= 14 && distance * cos(angle) <= 14 ) {
			if(x_pos > mouseX)
				boardAngle2 -= speed;
			else
				boardAngle2 += speed;
			if(y_pos > mouseY)
				boardAngle -= speed;
			else
				boardAngle += speed;
			
			mouseX = x_pos;
			mouseY = y_pos;
			
		}
		
		else if(distance*sin(angle) <= 14 && distance * cos(angle) <= -14 ) {
			if(x_pos > mouseX)
				boardAngle += speed;
			else
				boardAngle -= speed;
			if(y_pos > mouseY)
				boardAngle2 -= speed;
			else
				boardAngle2 += speed;
		
			mouseX = x_pos;
			mouseY = y_pos;

		}

		else if(distance*sin(angle) <= -14 && distance * cos(angle) <= 14 ) {
			if(x_pos > mouseX)
				boardAngle2 += speed;
			else
				boardAngle2 -= speed;
			if(y_pos > mouseY)
				boardAngle += speed;
			else
				boardAngle -= speed;
			
			mouseX = x_pos;
			mouseY = y_pos;

		}

		else if(distance*sin(angle) >= -14 && distance * cos(angle) >= 14 ) {
			if(x_pos > mouseX)
				boardAngle -= speed;
			else
				boardAngle += speed;
			if(y_pos > mouseY)
				boardAngle2 += speed;
			else
				boardAngle2 -= speed;
			
			mouseX = x_pos;
			mouseY = y_pos;

		}

		// limit board rotation
		if(boardAngle > 0.5) boardAngle = 0.5;
		if(boardAngle < -0.5) boardAngle = -0.5;
		if(boardAngle2 > 0.5) boardAngle2 = 0.5;
		if(boardAngle2 < -0.5) boardAngle2 = -0.5;

		 
		//std::cout << boardAngle << " " << boardAngle2 << std::endl;

		
		// update board with new rotation value
		btTransform trans;
        objects[0]->getMesh()->getMotionState()->getWorldTransform(trans);
        auto rotation = trans.getRotation();
        rotation += btQuaternion(btVector3(0,0,1), boardAngle) + btQuaternion(btVector3(1,0,0), boardAngle2);
        trans.setRotation(rotation);
        objects[0]->getMesh()->getMotionState()->setWorldTransform(trans);

        objects[2]->getMesh()->getMotionState()->getWorldTransform(trans);
        rotation = trans.getRotation();
        rotation += btQuaternion(btVector3(0,0,1), boardAngle) + btQuaternion(btVector3(1,0,0), boardAngle2);
        trans.setRotation(rotation);
        objects[2]->getMesh()->getMotionState()->setWorldTransform(trans);
	}
}

void Engine::initPhysics()
{
	// initialize all variables for creating a physics simulation
	btBroadphaseInterface *broadphase = new btDbvtBroadphase();
	btDefaultCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfig);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

	// create a physics simulation
	simulation = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);

	// initialize simulation gravity to -50
	simulation->setGravity(btVector3(0,-50,0));

	// register GImpact algorithm for collisions
	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
}

void Engine::createMenus()
{
	// create GLUT menu
	glutCreateMenu(menuActions);

	// add menu elements
	glutAddMenuEntry("Pause", MENU_PAUSE);
	glutAddMenuEntry("Resume", MENU_RESUME);
	glutAddMenuEntry("Restart", MENU_RESTART);
	glutAddMenuEntry("Exit", MENU_EXIT);

	// attach menu to scroll wheel
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void Engine::menuActions(int option)
{
	switch(option) {
		// pause simulation
		case MENU_PAUSE:
			paused = true;
		break;

		// resume simulation
		case MENU_RESUME:
			paused = false;
		break;

		// restart game
		case MENU_RESTART:
			// reset game values
			gameTime = 0.0;
			gameScore = 0;
			boardAngle = boardAngle2 = 0.0;

			// reset objects to inital positions
			reset();

			// reset top ten scores
			for(int i = 1; i <= 10; i++) {
				topTenScores[i-1] = std::string("Time: 0.00   Fail Count: 0");
			}
		break;

		// exit game
		case MENU_EXIT:
			// if linux just leave main loop
			#ifndef __APPLE__
				glutLeaveMainLoop();
			// else kill main thread
			#else
				exit(0);
			#endif
		break;

	}
}

void Engine::renderText(const char *text, glm::vec2 pos, glm::vec3 color)
{
	// init text position
    glRasterPos2f(pos[0],pos[1]);
    // init text color
	glColor4f(color[0],color[1],color[2], 1.0f);

	// if linux use glutBitmapString
	#ifndef __APPLE__
	    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)text);

	// else if apple use glutBitmapCharacter because glutBitmapString is not supported
	#else
		std::string textStr = text;

		// render each character individually
		for(char c : textStr) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, int(c));
		}
	#endif
}

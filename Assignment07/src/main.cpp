#include <GL/glew.h> // glew must be included before the main gl libs

// include freeglut on linux for glutBitmapString
#ifndef __APPLE__
#include <GL/freeglut.h> // doing otherwise causes compiler shouting
// else include glut for apple because freeglut does not work
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <streambuf>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier

#include "../src/vertex.h"
#include "../src/shaderloader.h"
#include "../src/modelloader.h"


//--Data types
//This object will define the attributes of a vertex(position, color, etc...)


//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;
GLint loc_texture;

GLint loc_texCoord;
GLuint textureId;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 moon;
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

//shaders
ShaderLoader vertexShader(GL_VERTEX_SHADER);
ShaderLoader fragmentShader(GL_FRAGMENT_SHADER);

ModelLoader ml;
// program control variables
bool paused = false;
bool reversedDirection = false;
bool reversedRotation = false;
std::string directionText = "Orbit Direction: Counter-Clockwise";
std::string modelFile = "cube.obj";
std::string vertexFile = "shaders/texvert.glslv";
std::string fragmentFile = "shaders/texfrag.glslf";
int triangleCount = 0;
float zoom = -16.0f;
int textureCount = 0;

void parseCommandLine(int argc, char **argv);
//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void keyboardSpecial(int key, int x_pos, int y_pos);
void mouse(int button, int state, int x, int y);

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

//--GLUT Menu Stuff
#define MENU_PAUSE 1
#define MENU_REVERSE_DIRECTION 2
#define MENU_REVERSE_ROTATION 3
#define MENU_EXIT 4
void createMenus();
void menuActions(int option);

// font rendering (code found from linux.die.net)
void renderText(const char *text);

//--Main
int main(int argc, char **argv)
{
	parseCommandLine(argc, argv);
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutSpecialFunc(keyboardSpecial); // called for non-ascii keyboard input
    glutMouseFunc(mouse); // called for mouse input

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
    if(init)
    {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    // Clean up after ourselves
    cleanUp();
    return 0;
}

void parseCommandLine(int argc, char **argv)
{
	std::string arg;

	for(int i = 1; i < argc; i++) {
		arg = argv[i];

		if(arg == "-m") {
			i++;
			if(i == argc) {
				std::cerr << "Incorrect use of -m option" << std::endl;
				exit(-1);
			}
			modelFile = argv[i];
		}

		else if(arg == "-v") {
			i++;
			if(i == argc) {
				std::cerr << "Incorrect use of -v option" << std::endl;
				exit(-1);
			}

			vertexFile = argv[i];
		}

		else if(arg == "-f") {
			i++;
			if(i == argc) {
				std::cerr << "Incorrect use of -f option" << std::endl;
				exit(-1);
			}

			fragmentFile = argv[i];
		}

		else {
			std::cerr << "Unknown command line option: " << arg << std::endl;
			exit(-1);
		}
	}
}

//--Implementations
void render()
{
    //--Render the scene

    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));	
		
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_texCoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

	glVertexAttribPointer(loc_texCoord,
						  2,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  (void*) offsetof(Vertex,textCoord));
/*
    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));
*/
	glUniform1i(loc_texture,0);
	glUniform1i(loc_texture,1);
	for(int i = 0; i < textureCount; i++) {
		//glUniform1i(loc_texture,i);
		glActiveTexture(GL_TEXTURE0 + i);	
		glBindTexture(GL_TEXTURE_2D,ml.getTexture(i));
		glUniform1i(loc_texture,i);
	}

    glDrawArrays(GL_TRIANGLES, 0, triangleCount*3);//mode, starting index, count
/*    mvp = projection * view * moon;
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
	glDrawArrays(GL_TRIANGLES, 0, triangleCount*4);
*/
    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texCoord);
    
    renderText(directionText.c_str());
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
	if(paused) {
	    t1 = std::chrono::high_resolution_clock::now();
		return;
	}
    //total time
    static float angle = 0.0f;
    static float rotationAngle = 0.0f;
    //static float moonAngle = 0.0f;
   // static float moonRotation = 0.0f;
    float dt = getDT();// if you have anything moving, use dt.

    
    if(reversedDirection) {
        angle -= dt * M_PI/2; //move through 90 degrees a second
   }
        
    else {
    	angle += dt * M_PI/2;
	}

	//moonAngle += dt * M_PI;

    // rotate the world around center of the frame
    model = glm::translate( glm::mat4(1.0f), glm::vec3(1.0 * sin(angle), 0.0, 1.0 * cos(angle)));
    
    // rotate the moon around the planet
    //moon = glm::translate(model, glm::vec3(4.0 * sin(moonAngle), 0.0, 4.0 * cos(moonAngle)));
    
    if(reversedRotation) {
        rotationAngle -= dt * M_PI/3;	
    }
    
    else {
        rotationAngle += dt * M_PI/3;
	}

	//moonRotation += dt * M_PI;
	
	model = glm::rotate(model, float(rotationAngle * 180.0/M_PI), glm::vec3(0.0,1.0,0.0));
	//moon = glm::rotate(moon, float(moonRotation * 180.0/M_PI), glm::vec3(0.0,1.0,0.0));
    // moon = glm::scale(moon, glm::vec3(0.5,0.5,0.5));   
    // Update the state of the scene
    glutPostRedisplay();//call the display callback

}


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    switch(key) {
    	case 27: // ESC
    		exit(0);
    	break;
    	
    	case 'p':
    	case 'P':
			paused = !paused;
			if(paused) {
				glutChangeToMenuEntry(MENU_PAUSE, "Resume Simulation", MENU_PAUSE);
				std::cout << "Simulation Paused" << std::endl;
			}
			
			else {
				glutChangeToMenuEntry(MENU_PAUSE, "Pause Simulation", MENU_PAUSE);
				std::cout << "Simulation Resumed" << std::endl;
			}
    	break;
    	
    	case 'a':
    	case 'A':
    		reversedDirection = !reversedDirection;
    		
    		directionText = (reversedDirection) ? "Orbit Direction: Clockwise" : "Orbit Direction: Counter-Clockwise";
    	break;

		case '-':
			zoom -= 1.0f;
			if(zoom == 0.0f) {
				zoom -= 1.0f;
			}

			view = glm::lookAt( glm::vec3(0.0, 8.0, zoom), //eye position
					glm::vec3(0.0, 0.0, 0.0), //focus point
					glm::vec3(0.0, 1.0, 0.0)); //positive y is up
		break;

		case '+':
			zoom += 1.0f;
			if(zoom == 0.0f) {
				zoom += 1.0f;
			}
			
			view = glm::lookAt( glm::vec3(0.0, 8.0, zoom), //eye position
					glm::vec3(0.0, 0.0, 0.0), //focus point
					glm::vec3(0.0, 1.0, 0.0)); //positive y is up

		break;
    }
}

void keyboardSpecial(int key, int x_pos, int y_pos)
{
    switch(key) {
        case GLUT_KEY_LEFT: // <-
            reversedDirection = true;
            directionText = "Orbit Direction: Clockwise";
        break;
        
        case GLUT_KEY_RIGHT: // ->
            reversedDirection = false;
            directionText = "Orbit Direction: Counter-Clockwise";
        break;
    }
}

void mouse(int button, int state, int x, int y)
{
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN) {
				reversedDirection = !reversedDirection;

				if(reversedDirection)
					directionText = "Orbit Direction: Clockwise";

				else
					directionText = "Orbit Direction: Counter-Clockwise";
			}
		break;
	}
}

bool initialize()
{                    
    //ModelLoader ml(modelFile.c_str());

	ml.setFileName(modelFile);
    
    auto vec = ml.load(triangleCount, textureCount);
    Vertex *geometry = new Vertex[vec.size()];
    
    for(size_t i = 0; i < vec.size(); i++) {
        geometry[i] = vec[i];
    }

	std::cout << "Vertex Count: " << vec.size() << std::endl 
			  << "Model Size: " << sizeof(*geometry) * vec.size() << std::endl;
    
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*geometry) * vec.size(), geometry, GL_STATIC_DRAW);
    
    //--Geometry done

	//textureId = ml.getTexture();
    //--load shaders
    if(!vertexShader.load(vertexFile) || !fragmentShader.load(fragmentFile))
        return false;

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = ShaderLoader::linkShaders({vertexShader, fragmentShader}); 

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

	loc_texCoord = glGetAttribLocation(program,
					 const_cast<const char*>("v_texCoord"));

	if(loc_texCoord == -1)
	{
		std::cerr << "[F] TEXTURE COORD NOT FOUND" << std::endl;
		return false;
	}

/*
    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }
*/
    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }

	loc_texture = glGetUniformLocation(program,
					const_cast<const char*>("tex"));

	if(loc_texture == -1) {
		std::cerr << "[F] TEXTURE NOT FOUND" << std::endl;
		return false;
	}
    
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, zoom), //eye position
                        glm::vec3(0.0, 0.0, 0.0), //focus point
                        glm::vec3(0.0, 1.0, 0.0)); //positive y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

	glEnable(GL_TEXTURE_2D);

	createMenus();
    //and its done
	
	delete[] geometry;
    return true;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
	//glDeleteTextures(1,&textureId);
	//for(int i = 0; i < textureCount; i++)
		//glDeleteTexture(1,&ml.getTexture(i));	
}

//returns the time delta
float getDT()
{
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}

void createMenus()
{
	glutCreateMenu(menuActions);
	
	glutAddMenuEntry("Pause Simulation", MENU_PAUSE);
	glutAddMenuEntry("Reverse Direction", MENU_REVERSE_DIRECTION);
	glutAddMenuEntry("Reverse Rotation", MENU_REVERSE_ROTATION);
	glutAddMenuEntry("Exit Program", MENU_EXIT);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void menuActions(int option)
{
	switch(option) {
		case MENU_PAUSE:
			paused = !paused;
			if(paused) {
				glutChangeToMenuEntry(MENU_PAUSE, "Resume Simulation", MENU_PAUSE);
				std::cout << "Simulation Paused" << std::endl;
			}
				
			else {
				glutChangeToMenuEntry(MENU_PAUSE, "Pause Simulation", MENU_PAUSE);
				std::cout << "Simulation Resumed" << std::endl;
			}
		break;
		
		case MENU_REVERSE_DIRECTION:
			reversedDirection = !reversedDirection;
		break;
		
		case MENU_REVERSE_ROTATION:
		    reversedRotation = !reversedRotation;
		break;
		
		case MENU_EXIT:
			exit(0);
		break;
	}
}

void renderText(const char *text)
{
	glUseProgram(0);
    glColor3f(1.0f,1.0,1.0);
    glRasterPos2f(-0.25f,-0.85f);
    
#ifndef __APPLE__ // this function does not work on Mac... problems with freeglut
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)text);
#else // apple
	std::string textStr = text;

	for(char c : textStr) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, int(c));
	}
#endif

    glUseProgram(program);
}

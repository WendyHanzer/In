#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::string;

struct Vertex {
	GLfloat position[3];
    GLfloat color[3];
};

// Precious Globals
int w = 640, h = 480;
bool rotate = true;
bool back = false;
float lastRotate = 0.0f;
string vsFile = "vs.txt";
string fsFile = "fs.txt";
GLuint program;
GLuint vbo_geometry;
GLint loc_mvpmat;
GLint loc_position;
GLint loc_color;
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat4 mvp;
void render();
void update();
void reshape(int, int);
void keyboard(unsigned char, int, int);
void menu(int);
void mouseClicks(int, int, int, int);
bool initialize();
void cleanUp();
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

int main(int argc, char **argv) {
	// Initialize things
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutCreateWindow("Matrix Example");
    GLenum status = glewInit();

    if( status != GLEW_OK) {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

	// create menu and entries along with mouse and keyboard
	glutCreateMenu(menu);
	glutAddMenuEntry("Toggle Rotate", 1);
	glutAddMenuEntry("Exit Program", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutMouseFunc(mouseClicks);
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutIdleFunc(update);
    glutKeyboardFunc(keyboard);

    bool init = initialize();

    if(init) {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
    }

    cleanUp();
    return 0;
}

// my menu function
void menu(int value) {
	if(value == 1)
		rotate = !rotate;
	if(value == 2)
		exit(0);
}

// my mouse click function
void mouseClicks(int button, int state, int x, int y) {
	// if left button is pressed then rotate backwards
	if((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		back = !back;
	}
}

void render() {
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// the mvp matrix
    mvp = projection * view * model;

    glUseProgram(program);
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

	// magic
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);

    glVertexAttribPointer(loc_position, 
						  3,
						  GL_FLOAT, GL_FALSE,
						  sizeof(Vertex),
						  0);
    glVertexAttribPointer(loc_color,
						  3,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  (void*)offsetof(Vertex,color));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    glutSwapBuffers();
}

void update() {
    static float angle = 0.0;
    float dt = getDT();

    angle += dt * M_PI/2;
    model = glm::translate( glm::mat4(1.0f), glm::vec3(4.0 * sin(angle), 0.0, 4.0 * cos(angle)));
	model = glm::rotate( model, lastRotate, glm::vec3(0.0f, 1.0f, 0.0f));

	// check for rotate flag
	if(rotate) {
		model = glm::rotate( model, lastRotate, glm::vec3(0.0f, 1.0f, 0.0f));
		
		// check for which direction to rotate
		if(!back) {
			lastRotate += (dt * M_PI/2)*100.0f;
		} else {
			lastRotate -= (dt * M_PI/2)*100.0f;
		}
	}

    glutPostRedisplay();
}

void reshape(int n_w, int n_h) {
    w = n_w;
    h = n_h;
    glViewport( 0, 0, w, h);
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);
}

void keyboard(unsigned char key, int x_pos, int y_pos) {
	// if esc then exit else if a then rotate backwards
    if(key == 27) {
        exit(0);
    } else if(key == 'a') {
		back = !back;
	}
}

bool shaderLoader(string fvs, string ffs){
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::ifstream is1;
	std::ifstream is2;
	int length = 0;

	// load the shaders
	is1.open(fvs, std::ios::binary);
	is1.seekg(0, std::ios::end);
	length = is1.tellg();
	is1.seekg(0, std::ios::beg);
	char *ret = new char[length+1];
	is1.read(ret, length);
	ret[length+1] = '\0';
	is1.close();
	const char *vs = ret;

	is2.open(ffs, std::ios::binary);
	is2.seekg(0, std::ios::end);
	length = is2.tellg();
	is2.seekg(0, std::ios::beg);
	char *ret2 = new char[length+1];
	is2.read(ret2, length);
	ret2[length+1] = '\0';
	is2.close();
	const char *fs = ret2;

    GLint shader_status;

	// compile vertex shader
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status) {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    }

	// compile fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status) {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

	// create the program and attach the shaders
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status) {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

	delete ret;
	delete ret2;
	return true;
}

// more magic
bool initialize() {
    Vertex geometry[] = { {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}},

                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},
                          
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},

                          {{1.0, -1.0, -1.0}, {1.0, 0.0, 0.0}},
                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{1.0, 1.0, -1.0}, {1.0, 1.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},

                          {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}},
                          {{1.0, -1.0, 1.0}, {1.0, 0.0, 1.0}}
                        };

    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

	if(!shaderLoader(vsFile, fsFile)) {
		return false;
	}

    loc_position = glGetAttribLocation(program, const_cast<const char*>("v_position"));
    if(loc_position == -1) {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_color = glGetAttribLocation(program, const_cast<const char*>("v_color"));
    if(loc_color == -1) {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
    }

    loc_mvpmat = glGetUniformLocation(program, const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1) {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
    }

    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    projection = glm::perspective( 45.0f, float(w)/float(h), 0.01f, 100.0f); 

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
}

// Delete things
void cleanUp() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
}

// Time things
float getDT() {
    float ret;
    t2 = std::chrono::high_resolution_clock::now();
    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
    t1 = std::chrono::high_resolution_clock::now();
    return ret;
}

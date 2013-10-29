#include "simobject.h"
#include "engine.h"

SimObject::SimObject(GLuint program, btScalar mass, std::string modelFile, btVector3 vec)
{
/*
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
*/
	ModelLoader ml(modelFile.c_str());
	auto geo = ml.load(triangleCount, textureCount);
	Vertex *geometry = new Vertex[geo.size()];
	for(int i = 0; i < geo.size(); i++)
		geometry[i] = geo[i];
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*geometry) * geo.size(), geometry, GL_STATIC_DRAW);

	loc_mvp = glGetUniformLocation(program, "mvpMatrix");
	loc_position = glGetAttribLocation(program, "v_position");
	loc_texture = glGetAttribLocation(program, "v_color");
	//loc_texture = glGetUniformLocation(program,"tex");
	//loc_texCoord = glGetAttribLocation(program,"v_texCoord");

	if(loc_mvp == -1 || loc_position == -1 ||
		loc_texture == -1 || loc_texCoord == -1)
			throw std::runtime_error("Unable to get locations in SimObject::SimObject()"); 
			
	btTriangleMesh *mesh = new btTriangleMesh();
	for(int i = 0; i < geo.size(); i+=3) {
		mesh->addTriangle(btVector3(geometry[i].position[0], geometry[i].position[1], geometry[i].position[2]),
						  btVector3(geometry[i+1].position[0], geometry[i+1].position[1], geometry[i+1].position[2]),
						  btVector3(geometry[i+2].position[0], geometry[i+2].position[1], geometry[i+2].position[2]));
						  
	}
	btCollisionShape * shape;
	if(mass == 0) {
	  shape  = new btBvhTriangleMeshShape(mesh, true);
	}
	else {
	  shape = new btConvexTriangleMeshShape(mesh);
	}
	
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), vec));
	btVector3 fallInertia(0,0,0);
	shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo shape1CI(mass,fallMotionState,shape,fallInertia);
	meshBody = new btRigidBody(shape1CI);
}

SimObject::~SimObject()
{
}

void SimObject::update(float dt)
{
}

void SimObject::render()
{
	glm::mat4 mvp = Engine::getProjection() * Engine::getView() * model;

	glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));	

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_texture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_texture,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, triangleCount*3);//mode, starting index, count	

	glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texture);
}

btRigidBody* SimObject::getMesh() const
{
	return meshBody;
}
void SimObject::setModel(glm::mat4 newModel)
{
	model = newModel;
}

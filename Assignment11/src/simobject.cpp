#include "simobject.h"
#include "engine.h"

// constructor
SimObject::SimObject(GLuint program, btScalar mass, std::string modelFile, btVector3 vec)
    : ml(modelFile.c_str())
{
	// load geometry from model file
	auto geo = ml.load(triangleCount, textureCount, lighting);
	Vertex *geometry = new Vertex[geo.size()];
	for(unsigned int i = 0; i < geo.size(); i++)
		geometry[i] = geo[i];

    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*geometry) * geo.size(), geometry, GL_STATIC_DRAW);

    // get all attribute locations from OpenGL program
	loc_mvp = glGetUniformLocation(program, "mvpMatrix");
	loc_position = glGetAttribLocation(program, "v_position");
	loc_texture = glGetUniformLocation(program,"tex");
	loc_texCoord = glGetAttribLocation(program,"v_texCoord");
	loc_hasTexture = glGetUniformLocation(program, "hasTexture");
	loc_color = glGetAttribLocation(program, "v_color");
	loc_normals = glGetAttribLocation(program, "v_normal");

	// if any location not found, throw an error
	if(loc_mvp == -1 || loc_position == -1 ||
		loc_texture == -1 || loc_texCoord == -1
		    || loc_hasTexture == -1 || loc_color == -1
		        || loc_normals == -1) {
                    std::cerr << loc_mvp << "\n"
                              << loc_position << "\n"
                              << loc_texture << "\n"
                              << loc_texCoord << "\n"
                              << loc_hasTexture << "\n"
                              << loc_color << "\n"
                              << loc_normals << std::endl;
	                throw std::runtime_error("Unable to get locations in SimObject::SimObject()");
		}

	// initialize physics mesh
	btTriangleMesh *mesh = new btTriangleMesh();
	for(unsigned int i = 0; i < geo.size(); i+=3) {
		mesh->addTriangle(btVector3(geometry[i].position[0], geometry[i].position[1], geometry[i].position[2]),
						  btVector3(geometry[i+1].position[0], geometry[i+1].position[1], geometry[i+1].position[2]),
						  btVector3(geometry[i+2].position[0], geometry[i+2].position[1], geometry[i+2].position[2]));

	}

	// initialize collision shape
	btGImpactMeshShape * shape = new btGImpactMeshShape(mesh);
	shape->setLocalScaling(btVector3(1.0,1.0,1.0));
	shape->updateBound();

	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), vec));
	btVector3 fallInertia(0,0,0);
	if(mass > 0) {
		shape->calculateLocalInertia(mass, fallInertia);
	}
	shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo shape1CI(mass,fallMotionState,shape,fallInertia);
	shape1CI.m_friction = 0.5;
	//shape1CI.m_restitution = 0.0;

	// create rigid body from collision shape
	meshBody = new btRigidBody(shape1CI);

	// disable object from deactivating
	meshBody->setActivationState(DISABLE_DEACTIVATION);
}

// destructor
SimObject::~SimObject()
{
}

void SimObject::update()
{
    float m[16];
    btTransform trans;
    glm::mat4 glMat;

    // get objects position in the world
    meshBody->getMotionState()->getWorldTransform(trans);

    // get openGL matrix from world
	trans.getOpenGLMatrix(m);

	// load OpenGL matrix into glm matrix
	int k = 0;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++) {
			glMat[i][j] = m[k++];
		}

	// update model matrix
	model = glMat;

	// get position and test if score needs to be updated
	auto pos = getPosition();
	if(pos.y() < -15) {
		Engine::score(0);
		reset();
	}
	// win position is around -9x and-6.5z
	pos = getPosition();
	if(pos.x() < -9 && (pos.z() <= -6.3 && pos.z() >= -6.7)) {
		Engine::score(1);
		reset();
	}
		
}

void SimObject::render(bool ambient, bool specular, bool diffuse)
{
	// calculate MVP matrix
	glm::mat4 mvp = Engine::getProjection() * Engine::getView() * model;

	// pass MVP to OpenGL program
	glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_texCoord);
    glEnableVertexAttribArray(loc_color);
    glEnableVertexAttribArray(loc_normals);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //set pointers into the vbo for each of the attributes
    glVertexAttribPointer( loc_position,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,position));

    glVertexAttribPointer( loc_texCoord,
                           2,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,textCoord));

    glVertexAttribPointer( loc_normals,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,normal));

    glVertexAttribPointer( loc_color,
    					   3,
    					   GL_FLOAT,
    					   GL_FALSE,
    					   sizeof(Vertex),
    					   (void*)offsetof(Vertex,color));

    // if textureCount > 0, hasTexture flag set to true, otherwise false
    glUniform1i(loc_hasTexture, textureCount);

    // send texture locations for all textures
	for(int i = 0; i < textureCount; i++) {
		glUniform1i(loc_texture,i);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D,ml.getTexture(i));
		glUniform1i(loc_texture,i);
	}

	// draw object
    glDrawArrays(GL_TRIANGLES, 0, triangleCount*3);//mode, starting index, count

    // disable attribute pointers
	glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texCoord);
    glDisableVertexAttribArray(loc_color);
    glDisableVertexAttribArray(loc_normals);
}

void SimObject::move(btVector3 pos)
{
	// get transform and update with new position
    btTransform trans;
    meshBody->getMotionState()->getWorldTransform(trans);
    trans.setOrigin(pos);

    // update body with new transform
    meshBody->getMotionState()->setWorldTransform(trans);
    meshBody->setCenterOfMassTransform(trans);
}

void SimObject::rotate(float angle, btVector3 y)
{
	// get transform and update with new rotation
    btTransform trans;
    meshBody->getMotionState()->getWorldTransform(trans);
    trans.setRotation(btQuaternion(y, angle));

    // update body with new transform
    meshBody->getMotionState()->setWorldTransform(trans);
    meshBody->setCenterOfMassTransform(trans);
}

void SimObject::reset()
{
	// disable all movement
	meshBody->setLinearVelocity(btVector3(0,0,0));
	meshBody->setAngularVelocity(btVector3(0,0,0));

	// reset position
	move(btVector3(0,0.1,0));
}

btRigidBody* SimObject::getMesh() const
{
	return meshBody;
}

void SimObject::setModel(glm::mat4 newModel)
{
	model = newModel;
}

btVector3 SimObject::getPosition() const
{
	// get transform and return position from it
	btTransform trans;
	meshBody->getMotionState()->getWorldTransform(trans);
	return trans.getOrigin();
}

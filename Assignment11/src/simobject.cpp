#include "simobject.h"
#include "engine.h"

SimObject::SimObject(GLuint program, btScalar mass, std::string modelFile, btVector3 vec)
    : ml(modelFile.c_str())
{
	auto geo = ml.load(triangleCount, textureCount, lighting);
	Vertex *geometry = new Vertex[geo.size()];
	for(unsigned int i = 0; i < geo.size(); i++)
		geometry[i] = geo[i];
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*geometry) * geo.size(), geometry, GL_STATIC_DRAW);

	loc_mvp = glGetUniformLocation(program, "mvpMatrix");
	loc_position = glGetAttribLocation(program, "v_position");
	loc_texture = glGetUniformLocation(program,"tex");
	loc_texCoord = glGetAttribLocation(program,"v_texCoord");
	loc_hasTexture = glGetUniformLocation(program, "hasTexture");
	loc_color = glGetAttribLocation(program, "v_color");
	loc_normals = glGetAttribLocation(program, "v_normal");
	loc_ambient = glGetUniformLocation(program, "ambient");
	loc_diffuse = glGetUniformLocation(program, "diffuse");
	loc_specular = glGetUniformLocation(program, "specular");
	loc_lightPos = glGetUniformLocation(program, "lightPosition");
	loc_shininess = glGetUniformLocation(program, "shininess");

	if(loc_mvp == -1 || loc_position == -1 ||
		loc_texture == -1 || loc_texCoord == -1
		    || loc_hasTexture == -1 || loc_color == -1
		        || loc_ambient == -1 || loc_diffuse == -1
		            || loc_specular == -1 || loc_lightPos == -1
		                || loc_shininess == -1 || loc_normals == -1) {
		                    std::cerr << loc_mvp << "\n"
		                              << loc_position << "\n"
		                              << loc_texture << "\n"
		                              << loc_texCoord << "\n"
		                              << loc_hasTexture << "\n"
		                              << loc_color << "\n"
		                              << loc_ambient << "\n"
		                              << loc_diffuse << "\n"
		                              << loc_specular << "\n"
		                              << loc_lightPos << "\n"
		                              << loc_shininess << "\n"
		                              << loc_normals << std::endl;
			                throw std::runtime_error("Unable to get locations in SimObject::SimObject()");
		}

	btTriangleMesh *mesh = new btTriangleMesh();
	for(unsigned int i = 0; i < geo.size(); i+=3) {
		mesh->addTriangle(btVector3(geometry[i].position[0], geometry[i].position[1], geometry[i].position[2]),
						  btVector3(geometry[i+1].position[0], geometry[i+1].position[1], geometry[i+1].position[2]),
						  btVector3(geometry[i+2].position[0], geometry[i+2].position[1], geometry[i+2].position[2]));

	}

	btCollisionShape * shape;
	if(mass > 0) {
		shape = new btConvexTriangleMeshShape(mesh);
	}
	else {
		shape = new btBvhTriangleMeshShape(mesh,true);
	}

	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), vec));
	btVector3 fallInertia(0,0,0);
	if(mass > 0) {
		shape->calculateLocalInertia(mass, fallInertia);
	}
	shape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo shape1CI(mass,fallMotionState,shape,fallInertia);
	shape1CI.m_friction = 0.5;
	shape1CI.m_restitution = 0.7;

	meshBody = new btRigidBody(shape1CI);
	meshBody->setActivationState(DISABLE_DEACTIVATION);
	meshBody->setAngularVelocity(btVector3(0,8,0));
}

SimObject::~SimObject()
{
}

void SimObject::update()
{
    float m[16];
    btTransform trans;
    glm::mat4 glMat;

    meshBody->getMotionState()->getWorldTransform(trans);

	trans.getOpenGLMatrix(m);

	int k = 0;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++) {
			glMat[i][j] = m[k++];
		}

	model = glMat;
}

void SimObject::render(bool ambient, bool specular, bool diffuse)
{
	glm::mat4 mvp = Engine::getProjection() * Engine::getView() * model;

	glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_texCoord);
    glEnableVertexAttribArray(loc_color);
    glEnableVertexAttribArray(loc_normals);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           (void*)offsetof(Vertex,position));//offset

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


    if(ambient)
        glUniform4f(loc_ambient, 1.0f, 1.0f, 1.0f, 0.0f);

    else
        glUniform4f(loc_ambient, 0.0f, 0.0f, 0.0f, 0.0f);

    if(diffuse)
        glUniform4f(loc_diffuse, 1.0f, 1.0f, 1.0f, 0.0f);

    else
        glUniform4f(loc_diffuse, 0.0f, 0.0f, 0.0f, 0.0f);

    if(specular)
        glUniform4f(loc_specular, 1.0f, 1.0f, 1.0f, 0.0f);

    else
        glUniform4f(loc_specular, 0.0f, 0.0f, 0.0f, 0.0f);

    glUniform1f(loc_shininess, lighting.shininess);
    glUniform1i(loc_hasTexture, textureCount);

    glUniform4f(loc_lightPos, 0.0f, 3.0f, 0.0f, 0.0f);

	for(int i = 0; i < textureCount; i++) {
		glUniform1i(loc_texture,i);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D,ml.getTexture(i));
		glUniform1i(loc_texture,i);
	}

    glDrawArrays(GL_TRIANGLES, 0, triangleCount*3);//mode, starting index, count

	glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texCoord);
    glDisableVertexAttribArray(loc_color);
    glDisableVertexAttribArray(loc_normals);
}

void SimObject::move(btVector3 pos)
{
    btTransform trans;
    meshBody->getMotionState()->getWorldTransform(trans);
    trans.setOrigin(pos);

    meshBody->getMotionState()->setWorldTransform(trans);
    meshBody->setCenterOfMassTransform(trans);
}

void SimObject::rotate(float angle, btVector3 y)
{
    btTransform trans;
    meshBody->getMotionState()->getWorldTransform(trans);
    trans.setRotation(btQuaternion(y, angle));

    meshBody->getMotionState()->setWorldTransform(trans);
    meshBody->setCenterOfMassTransform(trans);
}


btRigidBody* SimObject::getMesh() const
{
	return meshBody;
}
void SimObject::setModel(glm::mat4 newModel)
{
	model = newModel;
}

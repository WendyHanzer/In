#ifndef PUCK_H
#define PUCK_H

#include "simobject.h"

class Puck : public SimObject
{
public:
	Puck(GLuint program = 0);
	virtual ~Puck() {}

	virtual void update(float dt);
	//virtual void render();
};
#endif // PUCK_H

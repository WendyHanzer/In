#ifndef TABLE_H
#define TABLE_H

#include "simobject.h"

class Table : public SimObject
{
public:
	Table(GLuint program = 0);
	virtual ~Table() {}
	virtual void update(float dt);
	//virtual void render();
};
#endif // TABLE_H

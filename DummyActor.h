#ifndef _RATS_DUMMY
#define _RATS_DUMMY

#include "Actor.h"

class DummyActor : public Actor
{
private:
	int _Red;
	int _Green;
	int _Blue;

public:
	DummyActor(double x, double y, double rad, int red, int green, int blue);
	virtual ~DummyActor();

	// Read input and update state accordingly. Rats try to move towards players and away from other rats.
	virtual bool update(Model & model, double deltat);

	// Draw the rat.
	virtual void display(View & view, double offsetx, double offsety, double scale);
};

#endif //_RATS_DUMMY
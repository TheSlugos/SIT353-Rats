#ifndef __RATS_RAT
#define __RATS_RAT

class Rat;

#include "Player.h"

class Rat : public Actor
{
public:
	Rat (double x, double y);
	virtual ~Rat(void);

	// Read input and update state accordingly. Rats try to move towards players and away from other rats.
	virtual bool update (Model & model, double deltat);

	// Draw the rat.
	virtual void display (View & view, double offsetx, double offsety, double scale);
};

#endif // __RATS_RAT



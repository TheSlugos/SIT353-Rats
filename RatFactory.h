#ifndef __RATS_FACTORY
#define __RATS_FACTORY

#include "Actor.h"

class RatFactory : public Actor
{
public:
	RatFactory(void);

	virtual ~RatFactory(void);

	// Read input and update state accordingly. Rats try to move towards players and away from other rats.
	virtual bool update (Model & model, double deltat);

	// Draw the rat.
	virtual void display (View & view, double offsetx, double offsety, double scale);
};

#endif // __RATS_FACTORY


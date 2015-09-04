#ifndef __RATS_PLAYER
#define __RATS_PLAYER

#include "Actor.h"

class Player : public Actor
{
private:
	Controller & controller;

public:
	Player (Controller & cntrller);
	virtual ~Player(void);

	// Read input and update state accordingly.
	virtual bool update (Model & model, double deltat);

	// Show the player.
	virtual void display (View & view, double offsetx, double offsety, double scale);
};

#endif // __RATS_PLAYER
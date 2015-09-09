#ifndef __RATS_PLAYER
#define __RATS_PLAYER

#include "Actor.h"

class Player : public Actor
{
private:
	Controller & controller;
	int _PlayerId;

public:
	Player (Controller & cntrller);
	virtual ~Player(void);

	// Read input and update state accordingly.
	virtual bool update(Model & model, double deltat);
	bool update (Model & model, double deltat, char &command);

	// Show the player.
	virtual void display (View & view, double offsetx, double offsety, double scale);

	void SetPlayerId(int playerId) { _PlayerId = playerId; }
	int GetPlayerId() { return _PlayerId; }
};

#endif // __RATS_PLAYER
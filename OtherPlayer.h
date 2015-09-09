#ifndef __RATS_OTHERPLAYER
#define __RATS_OTHERPLAYER

#include "Actor.h"

class OtherPlayer : public Actor
{
private:
	int _PlayerId;
	int _Command;

public:
	OtherPlayer();
	virtual ~OtherPlayer(void);

	// Read input and update state accordingly.
	virtual bool update(Model & model, double deltat);

	// Show the player.
	virtual void display(View & view, double offsetx, double offsety, double scale);

	void SetPlayerId(int playerId) { _PlayerId = playerId; }
	int GetPlayerId() { return _PlayerId; }

	void SetCommand(char command) { _Command = command; }
};

#endif // __RATS_OTHERPLAYER
#include "Maze.h"
#include "OtherPlayer.h"
#include "Bullet.h"
#include "Player.h"

OtherPlayer::OtherPlayer() : Actor()
{
	_Command = 0;

	posx = 0;
	posy = 0;

	radius = 7.0;
	speed = 50.0;

	type = OTHERPLAYER;
}

OtherPlayer::~OtherPlayer(void)
{
}

bool OtherPlayer::update(Model & model, double deltat)

{
	double newposx = posx;
	double newposy = posy;

	double bulletposx = posx;
	double bulletposy = posy;
	bool bullet = false;

	char c = _Command;
	switch (c)
	{
	case 'W': newposy -= speed * deltat; break;
	case 'S': newposy += speed * deltat; break;
	case 'A': newposx -= speed * deltat; break;
	case 'D': newposx += speed * deltat; break;
	case VK_UP: bulletposy -= 1.3 * radius; bullet = true; break;
	case VK_DOWN: bulletposy += 1.3 * radius; bullet = true; break;
	case VK_LEFT: bulletposx -= 1.3 * radius; bullet = true; break;
	case VK_RIGHT: bulletposx += 1.3 * radius; bullet = true; break;
	default:
		// unknown key.
		;
	}

	if (model.canMove(posx, posy, newposx, newposy))
	{
		posx = newposx;
		posy = newposy;
	}
	if (bullet && model.canMove(posx, posy, bulletposx, bulletposy))
	{
		model.addActor(new Bullet(bulletposx, bulletposy, speed * (bulletposx - posx) / radius, speed * (bulletposy - posy) / radius, this));
	}

	// reset _Command otherwise every update the same command will be applied
	_Command = 0;

	return true;
}

void OtherPlayer::display(View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize(cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int)((posx - (offsetx - cx)) * scale);
	int y = (int)((posy - (offsety - cy)) * scale);

	int index = _PlayerId % maxColours;

	view.drawSolidCircle(x, y, (int)(scale * radius),
		colours[index][RED], colours[index][BLUE], colours[index][GREEN]);
}
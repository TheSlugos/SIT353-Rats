#include "Maze.h"
#include "Player.h"
#include "Bullet.h"

Player::Player (Controller & cntrller) : Actor (), controller (cntrller)
{
	posx = 0;
	posy = 0;

	radius = 7.0;
	speed = 50.0;

	type = PLAYER;
}

Player::~Player(void)
{
}

bool Player::update(Model & model, double deltat)
{
	return true;
}

bool Player::update (Model & model, double deltat, char& command)

{
	double newposx = posx;
	double newposy = posy;

	double bulletposx = posx;
	double bulletposy = posy;
	bool bullet = false;

	command = controller.lastKey ();
	switch (command)
	{
	case 'W': newposy -= speed * deltat; break;
	case 'S': newposy += speed * deltat; break;
	case 'A': newposx -= speed * deltat; break;
	case 'D': newposx += speed * deltat; break;
	/*case VK_UP: bulletposy -= 1.3 * radius; bullet = true; break;
	case VK_DOWN: bulletposy += 1.3 * radius; bullet = true; break;
	case VK_LEFT: bulletposx -= 1.3 * radius; bullet = true; break;
	case VK_RIGHT: bulletposx += 1.3 * radius; bullet = true; break;*/
	default:
		// unknown key.
		;
	}

	// update local copy, server updates overrides
	if (model.canMove (posx, posy, newposx, newposy))
	{
		posx = newposx;
		posy = newposy;
	}
	//if (bullet && model.canMove (posx, posy, bulletposx, bulletposy))
	//{
	//	model.addActor (new Bullet (bulletposx, bulletposy, speed * (bulletposx - posx) / radius, speed * (bulletposy - posy) / radius, this));
	//}

	return true;
}

void Player::display (View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int) ((posx - (offsetx - cx)) * scale);
	int y = (int) ((posy - (offsety - cy)) * scale);

	view.drawSolidCircle (x, y, (int) (scale * radius), 230, 230, 80);
}
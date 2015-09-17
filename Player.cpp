#include "Maze.h"
#include "Player.h"
#include "Bullet.h"

int colours[][3] = {
	{ 173, 255, 47 },
	{ 127, 255, 212 },
	{ 144, 238, 144 },
	{ 255, 228, 196 },
	{ 0, 0, 255 },
	{ 138, 43, 226 },
	{ 165, 42, 42 },
	{ 222, 184, 135 },
	{ 95, 158, 160 },
	{ 127, 255, 0 },
	{ 210, 105, 30 },
	{ 255, 127, 80 },
	{ 100, 149, 237 },
	{ 220, 20, 60 },
	{ 0, 255, 255 },
	{ 0, 0, 139 },
	{ 0, 139, 139 },
	{ 184, 134, 11 },
	{ 0, 100, 0 },
	{ 189, 183, 107 },
	{ 139, 0, 139 },
	{ 85, 107, 47 },
	{ 255, 140, 0 },
	{ 153, 50, 204 },
	{ 139, 0, 0 },
	{ 0, 255, 255 },
	{ 233, 150, 122 },
	{ 143, 188, 143 },
	{ 72, 61, 139 },
	{ 47, 79, 79 },
	{ 0, 206, 209 },
	{ 148, 0, 211 },
	{ 255, 20, 147 },
	{ 0, 191, 255 },
	{ 30, 144, 255 },
	{ 178, 34, 34 },
	{ 34, 139, 34 },
	{ 255, 0, 255 },
	{ 255, 215, 0 },
	{ 218, 165, 32 },
	{ 0, 128, 0 },
	{ 255, 105, 180 },
	{ 205, 92, 92 },
	{ 75, 0, 130 },
	{ 124, 252, 0 },
	{ 173, 216, 230 },
	{ 240, 128, 128 },
	{ 50, 205, 50 },
	{ 255, 0, 255 },
	{ 128, 0, 0 },
	{ 102, 205, 170 },
	{ 0, 0, 205 },
	{ 186, 85, 211 },
	{ 147, 112, 219 },
	{ 60, 179, 113 },
	{ 123, 104, 238 },
	{ 0, 250, 154 },
	{ 72, 209, 204 },
	{ 199, 21, 133 },
	{ 25, 25, 112 },
	{ 255, 228, 181 },
	{ 255, 222, 173 },
	{ 0, 0, 128 },
	{ 159, 175, 223 },
	{ 128, 128, 0 },
	{ 107, 142, 35 },
	{ 255, 165, 0 },
	{ 218, 112, 214 },
	{ 255, 218, 185 },
	{ 205, 133, 63 },
	{ 255, 192, 203 },
	{ 255, 0, 0 },
	{ 188, 143, 143 },
	{ 65, 105, 225 },
	{ 139, 69, 19 },
	{ 250, 128, 114 },
	{ 244, 164, 96 },
	{ 46, 139, 87 },
	{ 160, 82, 45 },
	{ 106, 90, 205 },
	{ 0, 255, 127 },
	{ 70, 130, 180 },
	{ 210, 180, 140 },
	{ 216, 191, 216 },
	{ 255, 99, 71 },
	{ 64, 224, 208 },
	{ 238, 130, 238 }
};

int maxColours = sizeof(colours) / (3 * sizeof(int));

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

	int index = _PlayerId % maxColours;

	view.drawSolidCircle (x, y, (int) (scale * radius), 
		colours[index][RED], colours[index][BLUE], colours[index][GREEN]);
}
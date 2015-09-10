#include "Maze.h"
#include "RatFactory.h"

RatFactory::RatFactory(void)
{
	posx = (double) (rand () % 1000 - 500);
	posy = (double) (rand () % 1000 - 500);

	speed = 0.0;
	radius = 15.0;

	type = RATFACTORY;
}

RatFactory::~RatFactory(void)
{
}

bool RatFactory::update (Model & model, double deltat)

{
	if (radius < 15.0)
		radius = radius + (0.25 * deltat);
	else
	{
		if (rand () % 1000 > 996)
		{
			((Maze &) model).addRat (posx, posy);
		}
	}

	return basicUpdate (model, deltat);
}

void RatFactory::display (View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int) ((posx - (offsetx - cx)) * scale);
	int y = (int) ((posy - (offsety - cy)) * scale);

	view.drawSolidCircle (x, y, (int) (scale * radius), 230, 23, 80);
}
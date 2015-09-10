#include "Maze.h"
#include "DummyActor.h"

DummyActor::DummyActor(double x, double y, double rad, int red, int green, int blue) : Actor()
{
	posx = x;
	posy = y;

	radius = rad;

	_Red = red;
	_Green = green;
	_Blue = blue;
}

DummyActor::~DummyActor()
{
}

// Read input and update state accordingly. Rats try to move towards players and away from other rats.
bool DummyActor::update(Model & model, double deltat)
{
	return basicUpdate(model, deltat);
}

// Draw the rat.
void DummyActor::display(View & view, double offsetx, double offsety, double scale)
{
	// Find center of screen.
	int cx, cy;
	view.screenSize(cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int)((posx - (offsetx - cx)) * scale);
	int y = (int)((posy - (offsety - cy)) * scale);

	view.drawSolidCircle(x, y, (int)(scale * radius), _Red, _Green, _Blue);
}
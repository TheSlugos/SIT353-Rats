#include "Maze.h"
#include "Rat.h"

Rat::Rat (double x, double y) : Actor ()
{
	posx = x;
	posy = y;

	speed = 20.0;
	radius = 5.0;

	type = RAT;
}

Rat::~Rat(void)
{
}

bool Rat::update (Model & model, double deltat)

{
	// How much of current direction to keep.
	double weightorg = 1.0;

	// Create a random vector.
	double randx;
	double randy;
	randx = (rand () % 1000 - 500) / 500.0;
	randy = (rand () % 1000 - 500) / 500.0;
	double weightrand = 1.0;

	double playerx = 0.0;
	double playery = 0.0;
	double weightplayer = 0.0;

	double factx = 0.0;
	double facty = 0.0;
	double weightfact = 0.2;

	const std::vector <Actor *> actors = model.getActors ();

	for (std::vector <Actor *>::const_iterator i = actors.begin (); i != actors.end (); i++)
	{
		if ((*i)->getType () == Actor::PLAYER)
		{
			// get the vector towards the player.
			double px, py;
			(*i)->getPosition (px, py);
			px -= posx;
			py -= posy;
			double plength = sqrt (px * px + py * py);
			px /= plength; // normalize, so all vectors are about the same length.
			py /= plength;
			double w = 80.0 / plength; // chase the player if close.
			if (w > weightplayer)
			{
				playerx = px;
				playery = py;
				weightplayer = w;
			}
		}

		if ((*i)->getType () == Actor::RATFACTORY)
		{
			// Create vector away from factory.
			double px, py;
			(*i)->getPosition (px, py);
			px = posx - px;
			py = posy - px;
			double plength = (px * px + py * py);

			factx += 200.0 * px / plength; // fade with distance - combined with normalization.
			facty += 200.0 * py / plength;
		}
	}

	if (weightplayer > 2.0)
	{
		weightplayer = 2.0;
	}

	if (rand () % 10 > 8)
	{
		weightrand = 1.0;
		weightorg = 0.0;
	}
	if (rand () % 10 > 7)
	{
		weightplayer = 0.0;
	}
	if (rand () % 10 > 5)
	{
		weightfact = 0.0;
	}

	// Mix all the vectors to get movement. Take a weighted average, so as to give the different effects their own
	// strengths.
	vx = (weightorg * vx + weightrand * randx + weightplayer * playerx + weightfact * factx) / (weightorg + weightrand + weightplayer + weightfact);
	vy = (weightorg * vy + weightrand * randy + weightplayer * playery + weightfact * facty) / (weightorg + weightrand + weightplayer + weightfact);

	if (basicUpdate (model, deltat))
	{
		double tmp = vx;
		vx = -vy;
		vy = tmp;
	}

	return true;
}

void Rat::display (View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int) ((posx - (offsetx - cx)) * scale);
	int y = (int) ((posy - (offsety - cy)) * scale);

	view.drawSolidCircle (x, y, (int) (scale * radius), 23, 23, 80);
}
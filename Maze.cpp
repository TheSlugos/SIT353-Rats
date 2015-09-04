#include <iostream>

#include "QuickDraw.h"

#include "Maze.h"
#include "MazeCreator.h"

#include "Player.h"
#include "Rat.h"
#include "Bullet.h"
#include "RatFactory.h"

Wall::Wall (int xa, int ya, int xb, int yb) : x1(xa), y1(ya), x2(xb), y2(yb)
{
}

Wall::~Wall ()
{
}

void Wall::display (View & view, double offsetx, double offsety, double scale)
{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int xa = (int) ((x1 - (offsetx - cx)) * scale);
	int ya = (int) ((y1 - (offsety - cy)) * scale);
	int xb = (int) ((x2 - (offsetx - cx)) * scale);
	int yb = (int) ((y2 - (offsety - cy)) * scale);

	view.drawLine (xa, ya, xb, yb);
}

bool Wall::collides (double xa, double ya, double xb, double yb)
{
	// Wall: x = x1 + t (x2 - x1)
	//       y = y1 + t (y2 - y1)
    //
	// Segment: x = xa + s (xb - xa)
	//          y = ya + s (yb - ya)

	// Intersection where x,y same for both lines. 0 <= s,t <= 1.

	// x1 + t (x2 - x1) = xa + s (xb - xa)
	// y1 + t (y2 - y1) = ya + s (yb - ya)

	// x1 (yb - ya) + t (x2 - x1) (yb - ya) = xa (yb - ya) + s (xb - xa) (yb - ya)
	// y1 (xb - xa) + t (y2 - y1) (xb - xa) = ya (xb - xa) + s (yb - ya) (xb - xa)
	
	// (x1 - xa) (yb - ya) - (y1 - ya) (xb - xa) + t [(x2 - x1) (yb - ya) - (y2 - y1) (xb - xa)] = 0 # Subtract

	double den = (x2 - x1) * (yb - ya) - (y2 - y1) * (xb - xa); // Also the cross product. Zero if parallel.
	if (fabs (den) < 1.0e-6) // close to parallel.
		return false; // assuming they are not on top of each other.

	double num = -((x1 - xa) * (yb - ya) - (y1 - ya) * (xb - xa));
	double t = num / den;
	double s = 0.0;
	if (fabs (xb - xa) < 1.0e-6)
	{
    	if (fabs (yb - ya) < 1.0e-6) // segment is zero length.
			return false; // assuming they are not on top of each other.
		s = ((y1 - ya) + t * (y2 - y1)) / (yb - ya);
	}
	else
	{
		s = ((x1 - xa) + t * (x2 - x1)) / (xb - xa);
	}

	if ((t >= 0.0) && (t <= 1.0) && (s >= 0.0) && (s <= 1.0))
		return true;

	return false;
}

Maze::Maze(void)
{
	walls.push_back (new Wall (-300, 200, 300, 200));
	walls.push_back (new Wall (-300, -200, 300, -200));
	walls.push_back (new Wall (-300, -200, -300, 200));
	walls.push_back (new Wall (300, -200, 300, 200));

	walls.push_back (new Wall (-100, -40, 100, 60));
	walls.push_back (new Wall (100, -40, -100, 60));

	walls = MazeCreator::createMaze (50, 50, 80);

	for (int i = 0; i < 15; i++)
		actors.push_back (new RatFactory ());
}


Maze::~Maze(void)
{
	for (std::vector <Wall *>::iterator i = walls.begin (); i != walls.end (); i++)
	{
		delete (*i);
	}
	for (std::vector <Actor *>::iterator i = actors.begin (); i != actors.end (); i++)
	{
		delete (*i);
	}
}

void Maze::update (double deltat)
{
	// Move the rats.
	double avgratx = 0.0;
	double avgraty = 0.0;
/*	for (std::vector <Rat *>::iterator i = rats.begin (); i != rats.end (); i++)
	{
		double px, py;
		(*i)->getPosition (px, py);
		avgratx += px;
		avgraty += py;
	}
	avgratx /= rats.size ();
	avgraty /= rats.size ();*/

	// Avoid using iterators since the list may grow during updates.
	for (unsigned int i = 0; i < actors.size (); )
	{
		if (!(actors[i])->update (*this, deltat))
		{
			delete (actors[i]);
			actors.erase (actors.begin () + i);
		}
		else
		{
			// Check for collisions between objects.
			bool killi = false;
			for (unsigned int j = 0; j < actors.size (); )
			{
				bool killj = false;
				double rx, ry;
				double bx, by;
				(actors[i])->getPosition (rx, ry);
				(actors[j])->getPosition (bx, by);
				if ((i != j) && (fabs (rx - bx) + fabs (ry - by) < actors[i]->getRadius () + actors[j]->getRadius ()))
				{
					// two objects are colliding.
					if ((actors[j]->getType () == Actor::BULLET) &&
						(actors[i]->getType () == Actor::RAT))
					{
						// remove bullet, kill rat.
						killj = true;
						killi = true;
					}
					if ((actors[j]->getType () == Actor::BULLET) &&
						(actors[i]->getType () == Actor::RATFACTORY))
					{
						// remove bullet, kill rat.
						killj = true;
						actors[i]->setRadius (actors[i]->getRadius () * 0.9);
					}
					if ((actors[j]->getType () == Actor::RAT) &&
						(actors[i]->getType () == Actor::PLAYER))
					{
						// move the player back to the starting point.
						actors[i]->setPosition (0.0, 0.0);
					}
				}

				if (killj)
				{
					// remove the bullet.
					delete (actors[j]);
					actors.erase (actors.begin () + j);
				}
				else
				{
					j++;
				}
			}

			if (killi)
			{
				delete (actors[i]);
				actors.erase (actors.begin () + i);
			}
			else
			{
				i++;
			}
		}
	}
}

void Maze::display (View & view, double offsetx, double offsety, double scale)

{
	for (std::vector <Wall *>::iterator i = walls.begin (); i != walls.end (); i++)
	{
		(*i)->display (view, offsetx, offsety, scale);
	}
	for (std::vector <Actor *>::iterator i = actors.begin (); i != actors.end (); i++)
	{
		(*i)->display (view, offsetx, offsety, scale);
	}
}

bool Maze::canMove (double x1, double y1, double x2, double y2)
{
	for (std::vector <Wall *>::iterator i = walls.begin (); i != walls.end (); i++)
	{
		// False if collides with any single wall.
		if ((*i)->collides (x1, y1, x2, y2))
			return false;
	}
	return true;
}

void Maze::addActor (Actor * actor)
{
	actors.push_back (actor);
}

void Maze::addPlayer (Player * player)
{
	addActor (player);
}

void Maze::addRat (double x, double y)
{
	if (actors.size () < 30) // save our processors!
	  addActor (new Rat (x, y));
}

const std::vector <Actor *> Maze::getActors ()

{
	return actors;
}

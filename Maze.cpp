#include <iostream>

#include "QuickDraw.h"

#include "Maze.h"
#include "MazeCreator.h"

#include "Player.h"
#include "OtherPlayer.h"
#include "Rat.h"
#include "Bullet.h"
#include "RatFactory.h"
#include "DummyActor.h"

using namespace std;

const int NOTAPLAYER = -1;

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

Maze::Maze(bool isServer)
{
	// SLP: only create a new maze if this is the server
	if (isServer)
	{
		walls.push_back(new Wall(-300, 200, 300, 200));
		walls.push_back(new Wall(-300, -200, 300, -200));
		walls.push_back(new Wall(-300, -200, -300, 200));
		walls.push_back(new Wall(300, -200, 300, 200));

		walls.push_back(new Wall(-100, -40, 100, 60));
		walls.push_back(new Wall(100, -40, -100, 60));

		walls = MazeCreator::createMaze(50, 50, 80);

		for (int i = 0; i < 15; i++)
			actors.push_back(new RatFactory());
	}
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

void Maze::removeActor(int index)
{
	if (index >= 0 && index < actors.size())
	{
		actors.erase(actors.begin() + index);
	}
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

char * Maze::serializewalls(int msgCode, size_t& size)
{
	size_t wallSize = sizeof(Wall);
	size_t noOfWalls = walls.size();

	// total size is msgcode + no of walls + wall data
	size = sizeof(int) + sizeof(int) + wallSize * noOfWalls;

	char * data = new char[size];

	int ptr = 0;

	// add message code to message data
	*(int*)data = msgCode;
	ptr += sizeof(int);

	// add number of walls to message data
	*(int*)(data + ptr) = noOfWalls;
	ptr += sizeof(int);

	// add each wall
	for (unsigned int i = 0; i < walls.size(); i++)
	{
		// add x1
		int * x1 = (int*)(data + ptr);
		(*x1) = walls[i]->GetX1();
		ptr += sizeof(int);
		// add y1
		int * y1 = (int*)(data + ptr);
		(*y1) = walls[i]->GetY1();
		ptr += sizeof(int);
		// add x2
		int * x2 = (int*)(data + ptr);
		(*x2) = walls[i]->GetX2();
		ptr += sizeof(int);
		// add y1
		int * y2 = (int*)(data + ptr);
		(*y2) = walls[i]->GetY2();
		ptr += sizeof(int);
	}

	return data;
}

char * Maze::serializeactors(int msgCode, size_t& size)
{
	//cout << "SERIALIZE ACTORS..." << endl;
	// need to store radius
	// need to create a dummy actor for client, with x,y,radius,colour

	// data size: message code + type, id, radius, x, y for each actor
	size_t totalActors = actors.size();

	//cout << " * Total Actors: " << totalActors << endl;

	size = sizeof(int) + sizeof(size_t) + totalActors * (sizeof(int) + sizeof(int) + 
		sizeof(double) + sizeof(double) + sizeof(double));

	//cout << " * Total Bytes: " << size << endl;

	// allocate space to hold actor data, delete outside
	char * data = new char[size];

	// data position pointer
	int ptr = 0;
	
	// store msgcode
	*(int*)data = msgCode;
	ptr += sizeof(int);

	// store number of actors
	*(int*)(data + ptr) = totalActors;
	ptr += sizeof(size_t);

	// store data for each actor
	for (unsigned int i = 0; i < actors.size(); i++)
	{
		double x, y;
		int type;
		double radius;

		type = actors[i]->getType();
		actors[i]->getPosition(x, y);
		radius = actors[i]->getRadius();

		//cout << " * " << i << " - Type: " << type << " (" << x << "," << y << ") Radius: " << radius << endl;
		
		*(int*)(data + ptr) = type;
		ptr += sizeof(int);
		
		// SLP:TODO: if type == player, store id
		// else store -1;
		if (type == actors[i]->OTHERPLAYER)
		{
			OtherPlayer * other = (OtherPlayer*)actors[i];
			*(int*)(data + ptr) = other->GetPlayerId();
		}
		else
		{
			*(int*)(data + ptr) = NOTAPLAYER;
		}

		ptr += sizeof(int);

		*(double*)(data + ptr) = radius;
		ptr += sizeof(double);

		*(double*)(data + ptr) = x;
		ptr += sizeof(double);

		*(double*)(data + ptr) = y;
		ptr += sizeof(double);
	}

	return data;
}

void Maze::deserializewalls(char * wallData)
{
	int ptr = 0;

	// empty walls
	for (unsigned int i = 0; i < walls.size(); i++)
	{
		// pointers so need to delete them
		delete walls[i];
	}

	// clear vector
	walls.clear();

	int noOfWalls = *(int*)wallData;
	ptr += sizeof(int);

	for (int i = 0; i < noOfWalls; i++)
	{
		walls.push_back(
			new Wall(
			*(int*)(wallData + ptr),					// x1
			*(int*)(wallData + ptr + sizeof(int)),		// y1, skip x1
			*(int*)(wallData + ptr + 2 * sizeof(int)),	// x2, skip x1, y1
			*(int*)(wallData + ptr + 3 * sizeof(int))	// y2, skip x1, y1, x2
			));
		ptr += 4 * sizeof(int);
	}
}

void Maze::deserializeactors(char * actorData, Player * player)
{
	//cout << "DESERIALIZE ACTORS..." << endl;

	// data pointer
	int ptr = 0;

	// need to clear all actors, except this player
	// pass player ptr into this method

	//cout << " * Delete stored actors" << endl;

	// for each actor in model,
	// if actor != player delete actor, erase actor, use shaun's method for erasing in a loop
	for (unsigned int i = 0; i < actors.size();)
	{
		// should be one of type PLAYER, rest will be OTHERPLAYER, RAT, etc
		if (actors[i]->getType() != actors[i]->PLAYER)
		{
			delete actors[i];
			actors.erase(actors.begin() + i);
		}
		else
		{
			// it is the player so leave it
			i++;
		}
	}

	// for each actor in data,
	size_t totalActors = *(size_t*)actorData;

	//cout << " * Total Actors: " << totalActors << endl;

	ptr += sizeof(size_t);

	for (unsigned int j = 0; j < totalActors; j++)
	{
		// get type from data, increment ptr
		int type = *(int*)(actorData + ptr);
		ptr += sizeof(int);

		// get id from data, increment ptr
		int actorId = *(int*)(actorData + ptr);
		ptr += sizeof(int);

		// get radius from data, increment ptr
		double radius = *(double*)(actorData + ptr);
		ptr += sizeof(double);

		// get coordinates from data, increment ptr
		double x = *(double*)(actorData + ptr);
		ptr += sizeof(double);
		
		double y = *(double*)(actorData + ptr);
		ptr += sizeof(double);

		//cout << " * " << j << " - Type: " << type << " (" << x << "," << y << ") Radius: " << radius << endl;

		// if id == -1
		if (actorId == player->GetPlayerId())
		{
			// update player coordinates
			player->setPosition(x, y);
		}
		else
		{
			// create new actor of type
			int r = 0, g = 0, b = 0;

			switch (type)
			{
				case RAT:
				{
					// set red,green,blue as required
					r = 23;
					g = 23;
					b = 80;
				} break;

				case OTHERPLAYER:
				{
					// set red, green, blue as required
					r = 230;
					g = 230;
					b = 80;
				} break;

				case BULLET:
				{
					// set red, green, blue as required
					g = 255;
				} break;

				case RATFACTORY:
				{
					// set red, green, blue as required
					r = 230;
					g = 23;
					b = 80;
				} break;
			} // end switch
			
			// create new dummy actor
			actors.push_back(new DummyActor(x, y, radius, r, g, b));
		} // end if
	}

	//cout << " * Bytes read: " << ptr << endl;
}

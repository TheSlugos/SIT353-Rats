#ifndef __RATS_MAZE
#define __RATS_MAZE

#include <vector>

class Maze;

#include "Model.h"
#include "QuickDraw.h"
#include "Actor.h"
#include "Player.h"

/// A maze consists of walls - walls being line segments in a 2D plane. Dimensions of the maze
/// will typically be in units of pixels, and the origin, as an initial starting point, should be within the maze.
class Wall
{
private:
	int x1;
	int y1;
	int x2;
	int y2;

public:
	Wall (int xa, int ya, int xb, int yb);
	~Wall ();

	// Draw the wall.
	void display (View & view, double offsetx, double offsety, double scale);

	// Check if the wall intersects the given line segment.
	bool collides (double xa, double ya, double xb, double yb);
};

class Maze : public Model
{
private:
	// Walls of the maze.
	std::vector <Wall *> walls;

	// List of actors inhabiting the maze.
	std::vector <Actor *> actors;

public:
	Maze(void);
	~Maze(void);

	// Run a simulation step of the game environment.
	void update (double deltat);

	// Render the maze.
	void display (View & view, double offsetx, double offsety, double scale);

	// Check if the move is allowed - i.e. no collisions.
	bool canMove (double x1, double y1, double x2, double y2);

	void addActor (Actor * actor);

	// Add a player to the game. 
	void addPlayer (Player * player);

	// Add another rat to the game.
	void addRat (double x, double y);

	// Retrieve the list of actors.
	const std::vector <Actor *> getActors ();
};

#endif // __RATS_MAZE

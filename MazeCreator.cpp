#include "MazeCreator.h"
#include <ctime>

// Node represent points where adjacent walls can join up.
typedef struct Node
{
	double x;
	double y;
	bool marked;
	std::vector <Node *> neighbours;
} Node;

std::vector <Wall *> MazeCreator::createMaze (int sizex, int sizey, double scale)
{
	// seed the rng
	srand(time(NULL));

	// The resulting set of walls for the maze.
	std::vector <Wall *> walls;

	// The size of wall junction points.
	std::vector <Node *> nodes;

	// Create a closed boundary around the entire maze.
	walls.push_back (new Wall (-(0.5 + (sizex / 2)) * scale, -(0.5 + (sizey / 2)) * scale, (0.5 + (sizex / 2)) * scale, -(0.5 + (sizey / 2)) * scale));
	walls.push_back (new Wall ((0.5 + (sizex / 2)) * scale, -(0.5 + (sizey / 2)) * scale, (0.5 + (sizex / 2)) * scale, (0.5 + (sizey / 2)) * scale));
	walls.push_back (new Wall ((0.5 + (sizex / 2)) * scale, (0.5 + (sizey / 2)) * scale, -(0.5 + (sizex / 2)) * scale, (0.5 + (sizey / 2)) * scale));
	walls.push_back (new Wall (-(0.5 + (sizex / 2)) * scale, (0.5 + (sizey / 2)) * scale, -(0.5 + (sizex / 2)) * scale, -(0.5 + (sizey / 2)) * scale));

	// Set up grid of junction points - in this case, a regular array.
	// These could potentially be perturbed to make life more interesting 
	// with non-horizontal/vertical walls.
    for (int j = 0; j < sizey; j++)
	  {
	    for (int i = 0; i < sizex; i++)
	      {
			Node * node = new Node;
			(*node).x = (0.5 + i - (sizex / 2)) * scale;
			(*node).y = (0.5 + j - (sizey / 2)) * scale;
			(*node).marked = false;

			nodes.push_back (node);
		}
	}
	// Link each node with potential neighbours.
    for (int j = 0; j < sizey; j++)
	  {
	    for (int i = 0; i < sizex; i++)
	      {
			if (i > 0)
				(*nodes[(j) * sizex + (i)]).neighbours.push_back (nodes[(j) * sizex + (i - 1)]);
			if (i < sizex - 1)
				(*nodes[(j) * sizex + (i)]).neighbours.push_back (nodes[(j) * sizex + (i + 1)]);
			if (j > 0)
				(*nodes[(j) * sizex + (i)]).neighbours.push_back (nodes[(j - 1) * sizex + (i)]);
			if (j < sizey - 1)
				(*nodes[(j) * sizex + (i)]).neighbours.push_back (nodes[(j + 1) * sizex + (i)]);
		}
	}

	// Build the maze. This is a corruption of a minimum spanning tree creation algorithm.
	// Add unused nodes to the tree in a random order. For each new node, connect it to
	// one other node in the tree - if possible. (A true minimum spanning tree algorithm
	// would ensure all fragments are connected by the end of the process).
	std::vector <Node *> tree;

	while (nodes.size () > 0)
	{
    	// pick a random node.
	    int n = rand () % nodes.size ();
	
		// Add node n to the tree.
		tree.push_back (nodes[n]);
		(*nodes[n]).marked = true;

		// If it can connected to another tree, then link it.
		for (unsigned int i = 0; i < (*nodes[n]).neighbours.size () * 2; i++)
		{
			// Try to choose a random link first - if no luck, then 
			// search through the list.
			int candidate;
			if (i < (*nodes[n]).neighbours.size ())
				candidate = rand () % (*nodes[n]).neighbours.size ();
			else
				candidate = i - (*nodes[n]).neighbours.size ();

			if ((*(*nodes[n]).neighbours[candidate]).marked)
			{
              walls.push_back (new Wall ((*nodes[n]).x, (*nodes[n]).y, (*(*nodes[n]).neighbours[candidate]).x, (*(*nodes[n]).neighbours[candidate]).y));
			  break;
			}
		}

		// Remove it from the set of contenders. 
		// Erasing nodes from a vector - how rude!
		nodes.erase (nodes.begin () + n);
	}

	return walls;
}


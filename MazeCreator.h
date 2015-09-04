#pragma once
#include "Maze.h"

class MazeCreator
{
public:

  // Create a maze. The size of the grid of cells is provided, the scale factor
  // defines the relative size of the grid relative to the players.
  static std::vector <Wall *> createMaze (int sizex, int sizey, double scale);
};


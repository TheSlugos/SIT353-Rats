#include <QuickDraw.h>
#include <Timer.h>

#include "Maze.h"
#include "Actor.h"

int main(int argc, char * argv [])
{
	QuickDraw window;
	View & view = (View &) window;
	Controller & controller = (Controller &) window;

	Maze model;
	Player * player = new Player (controller);
	model.addPlayer (player);

	// Create a timer to measure the real time since the previous game cycle.
	Timer timer;
	timer.mark (); // zero the timer.
	double lasttime = timer.interval ();
	double avgdeltat = 0.0;

	double scale = 1.0;

	while (true)
	{
		// Calculate the time since the last iteration.
		double currtime = timer.interval ();
		double deltat = currtime - lasttime;

		// Run a smoothing step on the time change, to overcome some of the
		// limitations with timer accuracy.
		avgdeltat = 0.2 * deltat + 0.8 * avgdeltat;
		deltat = avgdeltat;
		lasttime = lasttime + deltat;

		// Allow the environment to update.
		model.update (deltat);

		// Schedule a screen update event.
		view.clearScreen ();
    	double offsetx = 0.0;
	    double offsety = 0.0;
		(*player).getPosition (offsetx, offsety);
		model.display (view, offsetx, offsety, scale);
		view.swapBuffer ();
	}

	return 0;
}


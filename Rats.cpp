#include <iostream>
#include <string>
#include <conio.h>

#include "NetworkManager.h"
#include "Messages.h"

#include "QuickDraw.h"
#include "Timer.h"

#include "Maze.h"
#include "Actor.h"

using namespace std;

// constants
const int SERVERARGS = 2;			// program.exe <server-port>
const int CLIENTARGS = 4;			// program.exe <client-port> <server-ip> <server-port>
const int IDXMYPORT = 1;			// argv position of port of this machine	
const int IDXREMOTEHOST = 2;		// argv position of address of remote machine
const int IDXREMOTEPORT = 3;		// argv position of remote machine port
const int MAX_MSG_SIZE = 50000;		// size of maximum message that is handled

const double REFRESH_RATE = 1.0 / 30.0;	// 30fps

// structure to hold address and id of identified network nodes, servers or clients
typedef struct NetworkNode
{
	short id;						// player id
	unsigned long ipAddress;		// player ip address
	unsigned short portNumber;		// player port #
} NETWORKNODE;

// Runs the server component of the application
void server(int port)
{
	// SLP: Create network manager to manage send & receive over network
	NetworkManager nm;

	// setup and bind socket to supplied port number
	nm.SetupSocket(port);

	if (nm.Failed())
	{
		cout << "Network Error, exiting program" << endl;

		return;
	}

	cout << "Running as SERVER on " << nm.MyIPAddress() << ":"<< nm.MyPortNumber() << endl;

	QuickDraw window;
	View & view = (View &)window;
	
	// SLP: Don't need the controller as we are the server
	// Controller & controller = (Controller &)window;

	// we are the server
	Maze model(true);
	
	// SLP: Player only added once a client connects
	/*Player * player = new Player(controller);
	model.addPlayer(player);*/

	// Create a timer to measure the real time since the previous game cycle.
	Timer timer;
	timer.mark(); // zero the timer.
	double lasttime = timer.interval();
	double avgdeltat = 0.0;

	double scale = 1.0;

	while (true)
	{
		// Calculate the time since the last iteration.
		double currtime = timer.interval();
		double deltat = currtime - lasttime;

		// Run a smoothing step on the time change, to overcome some of the
		// limitations with timer accuracy.
		avgdeltat = 0.2 * deltat + 0.8 * avgdeltat;
		deltat = avgdeltat;
		lasttime = lasttime + deltat;

		// Allow the environment to update.
		model.update(deltat);

		// Schedule a screen update event.
		view.clearScreen();
		double offsetx = 0.0;
		double offsety = 0.0;
		
		// SLP: Don't need to display the map either as we are the server
		// but leaving it for now for testing
		// (*player).getPosition(offsetx, offsety);
		model.display(view, offsetx, offsety, scale);
		view.swapBuffer();

		// handle received data
		unsigned short remotePort;
		unsigned long remoteIP;
		char receivedData[MAX_MSG_SIZE];
		size_t bytesReceived = MAX_MSG_SIZE;

		if (nm.ReceiveData(remoteIP, remotePort, receivedData, bytesReceived))
		{
			int * msgType = (int*)receivedData;

			// handle message based on type
			switch (*msgType)
			{
				case JOIN:
				{
					cout << "JOIN: request from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;

					// send walls to client
					size_t datasize;
					char * data = model.serializewalls(WALLS, datasize);

					// send data to the newly joined client
					nm.SendData(remoteIP, remotePort, data, datasize);

					delete[] data;
				} break;
			} // end switch
		}

		// process console commands
		if (_kbhit())
		{
			string input;

			cout << "> ";

			cin >> input;

			if (input.find("/quit") != string::npos)
			{
				// quit the application
				break;
			}
		}
	}

	// TODO: cleanup sockets, etc
	nm.Close();

} // end server

void client(int port, unsigned long serverIP, int serverPort)
{
	// SLP: create network manager to facilitate communication
	NetworkManager nm;

	// create socket and bind port
	nm.SetupSocket(port);

	if (nm.Failed())
	{
		cout << "Error setting up network, exiting" << endl;
		return;
	}

	cout << "Running as CLIENT on " << nm.MyIPAddress() << ":" << nm.MyPortNumber() << endl;

	// SLP:TEST:Send a join message to the server
	MsgJoin joinMsg;
	nm.SendData(serverIP, serverPort, (char*)&joinMsg, sizeof(joinMsg));

	// SLP: the following lines should only be used once the server has connected
	QuickDraw window;
	View & view = (View &)window;
	Controller & controller = (Controller &)window;

	Maze model(false); // SLP: we are not the server

	Player * player = new Player(controller);
	model.addPlayer(player);
	// SLP: to here

	// Create a timer to measure the real time since the previous game cycle.
	Timer timer;
	timer.mark(); // zero the timer.
	double lasttime = timer.interval();
	double avgdeltat = 0.0;
	double refreshtime = lasttime;

	double scale = 1.0;

	while (true)
	{
		// Calculate the time since the last iteration.
		double currtime = timer.interval();
		double deltat = currtime - lasttime;

		// Run a smoothing step on the time change, to overcome some of the
		// limitations with timer accuracy.
		avgdeltat = 0.2 * deltat + 0.8 * avgdeltat;
		deltat = avgdeltat;
		lasttime = lasttime + deltat;

		// Allow the environment to update.
		// model.update(deltat); // SLP: server only

		// Schedule a screen update event.
		if ( currtime - refreshtime > REFRESH_RATE)
		{
			refreshtime = currtime;

			view.clearScreen();
			double offsetx = 0.0;
			double offsety = 0.0;
			(*player).getPosition(offsetx, offsety);
			model.display(view, offsetx, offsety, scale);
			view.swapBuffer();
		}

		// process player input

		// receive updates from the server
		// handle received data
		unsigned short remotePort;
		unsigned long remoteIP;
		char receivedData[MAX_MSG_SIZE];
		size_t bytesReceived = MAX_MSG_SIZE;

		if (nm.ReceiveData(remoteIP, remotePort, receivedData, bytesReceived))
		{
			int * msgType = (int*)receivedData;

			// handle message based on type
			switch (*msgType)
			{
				case JOIN:
				{
					cout << "JOIN: request from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;

					// send walls to client
					size_t datasize;
					char * data = model.serializewalls(WALLS, datasize);

					// send data to the newly joined client
					nm.SendData(remoteIP, remotePort, data, datasize);

					delete[] data;
				} break;

				case WALLS:
				{
					cout << "Received wall data from server" << endl;
					cout << "Reading wall data" << endl;

					// received data holds wall data
					// first int contains msgcode, skip that
					char * msgData = receivedData + sizeof(int);
					model.deserializewalls(msgData);
				} break;
			} // end switch
		}

		// console commands
		if (_kbhit())
		{
			string input;

			cout << "> ";

			cin >> input;

			if (input.find("/quit") != string::npos)
			{
				// quit the application
				break;
			}
		}
	} // end while

	// TODO: cleanup sockets, etc
} // end client

int main(int argc, char * argv [])
{
	// TODO:SLP:check entered values and display correct usage message
	if (argc == SERVERARGS)
	{
		// run server
		server(atoi(argv[IDXMYPORT]));
	}
	else if (argc == CLIENTARGS)
	{
		// run client
		client(atoi(argv[IDXMYPORT]), inet_addr(argv[IDXREMOTEHOST]), 
			atoi(argv[IDXREMOTEPORT]));
	}

	return 0;
}


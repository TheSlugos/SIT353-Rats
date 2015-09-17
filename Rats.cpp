#include <iostream>
#include <string>
#include <conio.h>
#include <vector>

#include "NetworkManager.h"
#include "Messages.h"

#include "QuickDraw.h"
#include "Timer.h"

#include "Maze.h"
#include "Actor.h"
#include "OtherPlayer.h"

using namespace std;

// constants
const int SERVERARGS = 2;			// program.exe <server-port>
const int DISCOVERARGS = 3;			// program.exe <client-port> <server-port>
const int CLIENTARGS = 4;			// program.exe <client-port> <server-port> <server-ip>
const int IDXMYPORT = 1;			// argv position of port of this machine	
const int IDXREMOTEHOST = 3;		// argv position of address of remote machine
const int IDXREMOTEPORT = 2;		// argv position of remote machine port
const int MAX_MSG_SIZE = 50000;		// size of maximum message that is handled
const double MSG_TIMEOUT = 60.0;	// time to wait for response from server
const double REFRESH_RATE = 1.0 / 30.0;	// 30fps

enum GAMESTATE { DISCOVERY, JOINING, MAPPING, PLAYING, QUITTING, SHUTDOWN };

// structure to hold address and id of identified network nodes, servers or clients
typedef struct NetworkNode
{
	short id;						// player id
	unsigned long ipAddress;		// player ip address
	unsigned short portNumber;		// player port #
	OtherPlayer * player;			// actor representing this player
} NETWORKNODE;

// Runs the server component of the application
void server(int port)
{
	// stores the id of the next player to join the game
	unsigned int nextPlayerId = 0;

	// holds id information for connected players
	vector<NETWORKNODE> players;

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
	double updatetime = lasttime;

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

		// Send world updates to clients.
		if (lasttime - updatetime > REFRESH_RATE)
		{
 			updatetime = lasttime;

			// get update data if there are players
			if (players.size() > 0)
			{
				size_t datasize;
				char * data = model.serializeactors(UPDATEDATA, datasize);

				// go through each connected client
				for (unsigned int i = 0; i < players.size(); i++)
				{
					// send the data
					nm.SendData(players[i].ipAddress, players[i].portNumber, data, datasize);
				}

				delete[] data;
			}
		}

		// Schedule a screen update event. REMOVE THIS
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
					// new join request, allocate id to new player
					int playerId = nextPlayerId;

					cout << "JOIN: request from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;
					
					// check if they are already on the list
					for (unsigned int i = 0; i < players.size(); i++)
					{
						if (players[i].ipAddress == remoteIP && players[i].portNumber == remotePort)
						{
							cout << "Player already established, reconnect player" << endl;
							// allocate existing id, maybe lost join accept packet, or drop out
							playerId = players[i].id;
							break;
						}
					}

					if (playerId == nextPlayerId)
					{
						// create new dummy player
						OtherPlayer * newPlayer = new OtherPlayer();
						newPlayer->SetPlayerId(playerId);
						
						// add new player to the model
						model.addActor(newPlayer);

						// add new connection data for this player
						NETWORKNODE newNode;
						newNode.ipAddress = remoteIP;
						newNode.portNumber = remotePort;
						newNode.id = playerId;
						newNode.player = newPlayer;

						players.push_back(newNode);
						
						// set id for next player
						nextPlayerId++;
					}
					
					cout << "Sending JOINACCEPT msg to " << nm.IPtoString(remoteIP) << ":" << remotePort
						<< " with PlayerId: " << playerId << endl << endl;

					cout << "Joined Players are :" << endl;

					for (unsigned int i = 0; i < players.size(); i++)
					{
						cout << "\t" << "* " << players[i].id << " - (" << nm.IPtoString(players[i].ipAddress)
							<< ":" << players[i].portNumber << ")" << endl;
					}
					cout << endl;

					// create a join accept message
					MsgAccepted * msg = new MsgAccepted(playerId);

					// send join accept message to client
					size_t datasize = sizeof(MsgAccepted);
					char * data = (char*)msg;

					// send data to the client
					nm.SendData(remoteIP, remotePort, data, datasize);

					delete msg;
				} break;

				case MAPREQUEST:
				{
					cout << "MAPDATA: request from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;

					// send walls to client
					size_t datasize;
					char * data = model.serializewalls(MAPDATA, datasize);

					// send data to the newly joined client
					nm.SendData(remoteIP, remotePort, data, datasize);

					delete[] data;
				} break;

				case QUIT:
				{
					cout << "QUIT: received quit from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;

					for (unsigned int i = 0; i < players.size(); i++)
					{
						if (players[i].ipAddress == remoteIP && players[i].portNumber == remotePort)
						{
							// found player, send back quitack message
							QuitAckMessage quitAckMsg;
							nm.SendData(remoteIP, remotePort, (char*)&quitAckMsg, sizeof(quitAckMsg));

							// need to find the actor representing this player
							int index = -1;
							vector<Actor *> actors = model.getActors();

							for (unsigned int j = 0; j < actors.size(); j++)
							{
								if (actors[j] == players[i].player)
								{
									index = j;
									break;
								}
							}

							// remove this actor from the list of actors
							if (index >= 0)
							{
								model.removeActor(index);
							}

							// delete this player object
							delete players[i].player;

							// erase connection info
							players.erase(players.begin() + i);

							// must have found the player
							break;
						}
					}
				} break;

				case COMMAND:
				{
					cout << "COMMAND: received command from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;

					MsgPlayerCommand *msgCommand = (MsgPlayerCommand*)receivedData;

					// SLP: find connection for this ip and port combination and check id
					for (unsigned int i = 0; i < players.size(); i++)
					{
						if (players[i].ipAddress == remoteIP && players[i].portNumber == remotePort &&
							players[i].id == msgCommand->_PlayerId)
						{
							cout << "* Address, Port and Id match... finding player object" << endl;
							// Correct player, process and apply command.
							// Need to find the otherplayer object for this player.
							players[i].player->SetCommand(msgCommand->_Command);

							// send update data back to the client
							size_t datasize;
							char * data = model.serializeactors(UPDATEDATA, datasize);

							// send the data
							nm.SendData(remoteIP, remotePort, data, datasize);

							// found matching player, exit loop
							break;
						} // end if
					} // end for
				} break;

				case FINDSERVER:
				{
					cout << "Message received via SERVER DISCOVERY from (" << nm.IPtoString(remoteIP) <<
						":" << remotePort << ")" << endl;

					ServerFoundMessage serverFound;
					nm.SendData(remoteIP, remotePort, (char*)&serverFound, sizeof(serverFound));

					cout << "Sending found message back, so they can JOIN game" << endl;
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

void client(int gamestate, int port, int serverPort, unsigned long serverIP = 0l)
{
	// stores the id of the server we are communicating with (for multiple)
	int currentServer = -1;
	// number of servers known currently
	int knownServers = 0;

	// stores list of known servers (for multiple)
	vector<NETWORKNODE> servers;

	// SLP: create network manager to facilitate communication
	NetworkManager nm;

	// create socket and bind port
	nm.SetupSocket(port);

	// current state of the game
	int gameState = gamestate;
	double msgSentTime = 0.0;
	bool stateMsgSent = false;

	if (nm.Failed())
	{
		cout << "Error setting up network, exiting" << endl;
		return;
	}

	cout << "Running as CLIENT on " << nm.MyIPAddress() << ":" << nm.MyPortNumber() << endl;

	// Joining a server directly so add server to list of servers and set to default
	if (gameState == JOINING)
	{
		// add the supplied server details
		NETWORKNODE newServer;
		newServer.id = knownServers++;
		newServer.ipAddress = serverIP;
		newServer.portNumber = serverPort;

		// add server details to list of known servers
		servers.push_back(newServer);

		// 
		++currentServer;
	}

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
	double refreshtime = lasttime;	// for drawing

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

		switch (gameState)
		{
			case QUITTING:
			{
				// msg has been sent for this flag to be set
				if (currtime - msgSentTime > MSG_TIMEOUT)
				{
					cout << "QUIT Message timeout, resending" << endl;
					QuitMessage quitMsg;
					nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber, (char*)&quitMsg, sizeof(quitMsg));
					msgSentTime = currtime;
				}
			} break;

			case DISCOVERY:
			{
				// broadcast findserver message
				if (!stateMsgSent)
				{
					cout << "In DISCOVERY mode, send broadcast message" << endl;

					// get broadcast address, port is serverPort
					unsigned long broadcastAddress = nm.GetBroadcastAddress();

					cout << "Broadcast Address is " << nm.IPtoString(broadcastAddress) << endl;

					// Send out message
					FindServerMessage findMsg;
					
					nm.SendBroadCastData(serverPort, (char*)&findMsg, sizeof(findMsg));
					stateMsgSent = true;
					msgSentTime = currtime;
				}
				else
				{
					if (currtime - msgSentTime > MSG_TIMEOUT)
					{
						cout << "DISCOVERY timeout, search again" << endl;

						// get broadcast address, port is serverPort

						// Send out message
						FindServerMessage findMsg;

						nm.SendBroadCastData(serverPort, (char*)&findMsg, sizeof(findMsg));
						msgSentTime = currtime;
					}
				}
			} break;

			case JOINING:
			{
				// do initial setup and send join message
				if (!stateMsgSent)
				{
					cout << "Attempting to JOIN server: " << nm.IPtoString(servers[currentServer].ipAddress)
						<< ":" << servers[currentServer].portNumber << endl;

					// SLP:TEST:Send a join message to the current server
					MsgJoin joinMsg;
					// using currentServer as idx only works if we don't delete servers, and add them in correct order
					if (currentServer >= 0)
					{
						nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber, (char*)&joinMsg, sizeof(joinMsg));
						msgSentTime = currtime;
						stateMsgSent = true;
					}
				}
				else // check for timeout and resend if required
				{
					if (currtime - msgSentTime > MSG_TIMEOUT)
					{
						// resend a join message
						cout << "JOIN timeout, reattempting to JOIN server" << endl;

						// SLP:TEST:Send a join message to the current server
						MsgJoin joinMsg;
						// using currentServer as idx only works if we don't delete servers, and add them in correct order
						if (currentServer >= 0)
						{
							nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber, (char*)&joinMsg, sizeof(joinMsg));
							msgSentTime = currtime;
						}
					}
				}
			} break;

			case MAPPING:
			{
				if (!stateMsgSent)
				{
					// have player id, request map data from server
					MsgMapRequest msgMap;

					cout << "MAPREQUEST: sending to server" << endl;

					char * data = (char*)&msgMap;
					size_t dataSize = sizeof(msgMap);

					if (currentServer >= 0)
					{
						nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber,
							data, dataSize);
						msgSentTime = currtime;
						stateMsgSent = true;
					}
				}
				else
				{
					if (currtime - msgSentTime > MSG_TIMEOUT)
					{
						// resend map request
						cout << "MAPREQUEST timeout: requesting MAP again" << endl;

						MsgMapRequest msgMap;

						cout << "MAPREQUEST: sending to server" << endl;

						char * data = (char*)&msgMap;
						size_t dataSize = sizeof(msgMap);

						if (currentServer >= 0)
						{
							nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber,
								data, dataSize);
							msgSentTime = currtime;
						}
					}
				}
			} break;

			case PLAYING:
			{
				// process player input
				char command;
				player->update(model, deltat, command);

				// send command to the server
				switch (command)
				{
					case 'W':
					case 'S':
					case 'A':
					case 'D':
					case VK_UP:
					case VK_DOWN:
					case VK_LEFT:
					case VK_RIGHT:
					{
						// send command to the server
						MsgPlayerCommand * msgCommand = new MsgPlayerCommand(player->GetPlayerId(), command);
						char * data = (char*)msgCommand;
						size_t dataSize = sizeof(MsgPlayerCommand);

						if (currentServer >= 0)
						{
							nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber,
								data, dataSize);
							cout << "Sending command code: " << command << endl;
						}

						delete msgCommand;

						// reset command
						command = '\0';
					} break;
				} // end switch

				// Schedule a screen update event.
				if (currtime - refreshtime > REFRESH_RATE)
				{
					refreshtime = currtime;

					view.clearScreen();
					double offsetx = 0.0;
					double offsety = 0.0;
					(*player).getPosition(offsetx, offsety);

					model.display(view, offsetx, offsety, scale);
					view.swapBuffer();
				}
			} break;
		} // gamestate

		// all states do the following
		// receive network data, handle console input

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
				case JOINACCEPT:
				{
					cout << "JOINACCEPT msg received from " << nm.IPtoString(remoteIP) << ":" << remotePort << endl;
					MsgAccepted * msg = (MsgAccepted*)receivedData;

					cout << "Received player id " << msg->_PlayerNo << endl;

					// store the given player id, used later for deserializing actor data from server
					player->SetPlayerId(msg->_PlayerNo);

					// change state and reset flag
					gameState = MAPPING;
					stateMsgSent = false;
				} break;

				case MAPDATA:
				{
					cout << "Received MAP data from server" << endl;
					cout << "Reading & storing MAP data" << endl;

					// received data holds wall data
					// first int contains msgcode, skip that
					char * msgData = receivedData + sizeof(int);
					model.deserializewalls(msgData);

					// change game state
					gameState = PLAYING;
					stateMsgSent = false;

					cout << endl <<  "*** GAME READY ***" << endl;
				} break;

				case UPDATEDATA:
				{
					//cout << "UPDATEDATA: received actor data from server" << endl;

					char * msgData = receivedData + sizeof(int);	// skip msgcode
					model.deserializeactors(msgData, player);
				} break;

				case SERVERFOUND:
				{
					cout << "Server response received, storing server details" << endl;
					
					// see if we have discovered that server previously
					bool serverFound = false;
					for (unsigned int i = 0; i < servers.size(); i++)
					{
						if (servers[i].ipAddress == remoteIP && servers[i].portNumber == remotePort)
						{
							cout << "Server at " << nm.IPtoString(remoteIP) << ":" << remotePort <<
								"is known already" << endl;
							serverFound = true;
						}
					}

					if (!serverFound)
					{
						cout << "Adding server " << nm.IPtoString(remoteIP) << ":" << remotePort << " to list of servers" << endl;

						NETWORKNODE serverNode;
						serverNode.id = knownServers++;
						serverNode.ipAddress = remoteIP;
						serverNode.portNumber = remotePort;

						// store server details
						servers.push_back(serverNode);

						// display discovered servers
						cout << endl;
						cout << "Discovered servers:" << endl;
						for (unsigned int i = 0; i < servers.size(); i++)
						{
							cout << "\t* " << i << " - " << nm.IPtoString(servers[i].ipAddress) << ":" << servers[i].portNumber << endl;
						}
						cout << endl;
					}

					// change to JOIN modemove this to /join console command
					//gameState = JOINING;
					//stateMsgSent = false;
				} break;

				case QUITACK:
				{
					cout << "Quit Ack received, exiting" << endl;
					gameState = SHUTDOWN;
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
				// send quit message to the server
				QuitMessage quitMsg;

				// only send if connected to at least one server
				if (servers.size() > 0 && currentServer >= 0)
				{
					// send it to each server
					// SLP: sequence numbers, acks???
					/*for (unsigned int i = 0; i < servers.size(); i++)
					{
						nm.SendData(servers[i].ipAddress, servers[i].portNumber, (char*)&quitMsg, sizeof(quitMsg));
					}*/
					nm.SendData(servers[currentServer].ipAddress, servers[currentServer].portNumber, (char*)&quitMsg, sizeof(quitMsg));
					msgSentTime = currtime;
					gameState = QUITTING;
				}
			}
			else if (input.find("/join") != string::npos)
			{
				if (gameState == DISCOVERY)
				{
					int serverChoice;

					cin >> serverChoice;

					if (serverChoice >= 0 && serverChoice < knownServers)
					{
						// store selected server
						currentServer = serverChoice;

						// reset message flag
						stateMsgSent = false;

						// change state
						gameState = JOINING;
					}
					else
					{
						cout << "You entered and invalid server" << endl;
					}
				}
				else
				{
					cout << "You can only use /join during DISCOVERY" << endl;
				}
			}
		}

		if (gameState == SHUTDOWN)
		{
			// exit game
			break;
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
	else if (argc == DISCOVERARGS)
	{
		// run client in discover mode
		client(DISCOVERY, atoi(argv[IDXMYPORT]), atoi(argv[IDXREMOTEPORT]));
	}
	else if (argc == CLIENTARGS)
	{
		// run client in normal client mode
		// have server already so run in JOIN mode
		client(JOINING, atoi(argv[IDXMYPORT]), atoi(argv[IDXREMOTEPORT]), inet_addr(argv[IDXREMOTEHOST]));
	}

	return 0;
}


// NetworkManager
// Class to setup, maintain, send and receive data on a UDP socket
// Mainly to save me doing this for every practical
#include "NetworkManager.h"

// WinSock library
// Winsock Library <- in main.cpp maybe
#pragma comment(lib, "ws2_32.lib")

// Constructor
// Parameters: none
// Initialises winsock system
NetworkManager::NetworkManager()
{
	// Set socket to invalid
	_Socket = SOCKET_ERROR;

	// Optimistic everything will setup fine
	_Error = false;

	// Set drop out flag
	_IsDropOut = false;

	// Initialise Winsock
	// Start Winsock
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		SetError("Unable to start winsock API");
	}
}

NetworkManager::~NetworkManager()
{
	// ensure that the socket is closed
	if (_Socket != SOCKET_ERROR)
	{
		Close();
	}

	WSACleanup();
}

void NetworkManager::SetupSocket(unsigned short portNumber)
{
	struct sockaddr_in my_addr;
	u_long NONBLOCKINGMODE = 1;

	// Create and bind the socket, set to non-blocking mode

	if (!_Error)
	{
		// Create udp socket
		if ((_Socket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
		{
			SetError("Error creating socket");
		}
	}

	// Bind the socket
	if (!_Error)
	{
		// Fill in my address information
		// IPv4
		my_addr.sin_family = AF_INET;
		// Use the supplied port number
		my_addr.sin_port = htons(portNumber);
		// fill in with my ip address
		my_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(_Socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) == SOCKET_ERROR)
		{
			SetError("Error binding socket");
		}
	}

	// Set socket to non-blocking mode
	if (!_Error)
	{
		if (ioctlsocket(_Socket, FIONBIO, &NONBLOCKINGMODE) != NO_ERROR)
		{
			SetError("Error setting socket to non-blocking mode");
		}
	}

	// Fill in local IP address
	if (!_Error)
	{
		LocalAddressInfo(portNumber);
	}
}

bool NetworkManager::SendData(unsigned long destIP, unsigned short destPort, char * data, size_t dataLength)
{
	bool result = true;

	// reset dropout flag
	_IsDropOut = false;

	//Build destination address
	struct sockaddr_in dest_addr;

	// fill in destination
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = destIP;
	dest_addr.sin_port = htons(destPort);

	// int iResult = send(udpSocket, input.c_str(), strlen(input.c_str()), 0);

	//cout << "Sending message to " << inet_ntoa(dest_addr.sin_addr) << endl;

	int iResult = sendto(_Socket, data, dataLength, 0,
		(const sockaddr *)&dest_addr, sizeof(dest_addr));

	if (iResult == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAECONNRESET)
		{
			cerr << "Destination port was unreachable...remove that address" << endl;
			result = false;
			_IsDropOut = true;
			_DropOutIPAddress = destIP;
			_DropOutPortNumber = destPort;
		}
		else
		{
			cerr << "ERROR: sending data (" << WSAGetLastError() << ")" << endl;
			result = false;
		}
	}
	else
	{
		//cout << iResult << " bytes sent, awaiting reply." << endl;
	}

	return result;
}

bool NetworkManager::ReceiveData(unsigned long &sourceIP, unsigned short &sourcePort, char * data, size_t &dataLength)
{
	bool result = true;

	// reset dropout flag
	_IsDropOut = false;

	// check for data sent to us
	int addr_len;
	int num_bytes;
	//const size_t bufSize = 1024;

	//char recvBuffer[bufSize];

	struct sockaddr_in sender_addr;
	// fill in connector's address
	addr_len = sizeof(struct sockaddr);
	if ((num_bytes = recvfrom(_Socket, data, dataLength, 0, (struct sockaddr*)&sender_addr, &addr_len)) == SOCKET_ERROR)
	{
		result = false;

		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			if (WSAGetLastError() == WSAECONNRESET)
			{
				cerr << "Destination port was unreachable...remove that address" << endl;
				_IsDropOut = true;
				_DropOutIPAddress = sender_addr.sin_addr.s_addr;
				_DropOutPortNumber = ntohs(sender_addr.sin_port);
				result = false;
			}
			else
			{
				cerr << "ERROR: receiving data (" << WSAGetLastError() << ")" << endl;
				result = false;
			}
		}
	}
	else
	{
		// process message
		/*cout << "Message received from " << inet_ntoa(sender_addr.sin_addr) << " (" << num_bytes <<
			" bytes)" << endl;*/

		//memcpy(data, recvBuffer, num_bytes);
		dataLength = num_bytes;

		// store sender details
		sourceIP = sender_addr.sin_addr.s_addr;
		sourcePort = ntohs(sender_addr.sin_port);
	}

	return result;
}

// MyIPAddress
// Parameters: none
// Returns: local IP address as string
// Getter function to obtain the local IP address we are running on
string NetworkManager::MyIPAddress() const
{
	string result;

	if (!_Error)
	{
		result = _MyIPAddress;
	}
	else
	{
		result = "Error: Socket not setup cannot get IP address.";
	}

	return result;
}

// MyPortNumber
// Parameters: none
// Returns: port number as string
// Getter function to obtain the port number we are communicating through
string NetworkManager::MyPortNumber() const
{
	string result;

	if (!_Error)
	{
		result = _MyPortNumber;
	}
	else
	{
		result = "Error: Socket not setup cannot get port number.";
	}

	return result;
}

void NetworkManager::SetError(string errorMsg)
{
	ostringstream oss;

	oss << errorMsg << " - (" << WSAGetLastError() << ")";

	_ErrorMessage = oss.str();

	cerr << _ErrorMessage;

	_Error = true;
}

// LocalAddressInfo
// Parameters: none
// Returns: none
// Determines local address and port number and stores them
void NetworkManager::LocalAddressInfo(unsigned short portNumber)
{
	// Connect to ourselves so just need to determine our address.
	int n;
	struct addrinfo * addrinfo;
	struct addrinfo * current;

	// cout << "Getting address of this machine..." << endl;

	if ((n = getaddrinfo("", NULL, NULL, &addrinfo)) != 0)
	{
		SetError("Error getting address info");
	}

	if (!_Error)
	{
		current = addrinfo;
		while (current != NULL)
		{
			// get the first IPv4 address
			if (current->ai_family == AF_INET)
			{
				// get ip address
				char hostname[NI_MAXHOST];
				char servInfo[NI_MAXSERV];

				if (getnameinfo(current->ai_addr, current->ai_addrlen, hostname, NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST))
				{
					SetError("Error extracting name information");
				}

				_MyIPAddress = hostname;

				// cout << "Host: " << hostname << " - " << servInfo << endl;

				// only want one address
				break;
			}

			// Check next address info
			current = current->ai_next;
		}

		// clear addrinfo
		freeaddrinfo(addrinfo);

		// convert port number
		ostringstream oss;
		oss << portNumber;

		_MyPortNumber = oss.str();
	}
}

// Close
// Parameters: none
// Returns: nothing
// Used to close the network socket when finished
void NetworkManager::Close()
{
	cout << "Closing socket..." << endl;

	if (!_Error)
	{
		// assume socket must be opened and close it
		if (closesocket(_Socket) == SOCKET_ERROR)
		{
			SetError("Problem closing socket");
		}
		else
		{
			_Socket = SOCKET_ERROR;
		}
	}
	
}

// Utility function to convert network IP address into a string
string NetworkManager::IPtoString(unsigned long ipAddress)
{
	size_t size = sizeof(ipAddress);

	int d = (ipAddress >> 24) & 0xFF;
	int c = (ipAddress >> 16) & 0xFF;
	int b = (ipAddress >> 8) & 0xFF;
	int a = ipAddress & 0xFF;

	ostringstream oss;

	oss << a << "." << b << "." << c << "." << d;

	return oss.str();
}
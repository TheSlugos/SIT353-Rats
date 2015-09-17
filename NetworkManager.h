// NetworkManager
// Class to setup, maintain, send and receive data on a UDP socket
// Mainly to save me doing this for every practical
#pragma once

//#define _WINSOCK_DEPRECATED_NO_WARNINGS

// I/O headers
#include <iostream>
#include <string>
#include <sstream>

// Winsock headers
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

class NetworkManager
{
private:
	bool _Error;
	string _ErrorMessage;
	int _Socket;
	string _MyIPAddress;
	string _MyPortNumber;

	// handle drop outs
	bool _IsDropOut;

	// store connection that dropped
	unsigned long _DropOutIPAddress;
	unsigned short _DropOutPortNumber;

	void SetError(string errorMsg);
	void LocalAddressInfo(unsigned short portNumber);

public:
	NetworkManager();
	~NetworkManager();
	bool SendData(unsigned long destIP, unsigned short destPort, char * data, size_t dataLength);
	bool ReceiveData(unsigned long &sourceIP, unsigned short &sourcePort, char * data, size_t &dataLength);
	string MyIPAddress() const;
	string MyPortNumber() const;
	void Close();
	void SetupSocket(unsigned short portNumber);
	bool IsDropout() { return _IsDropOut; }
	unsigned long GetDropOutIPAddress() { return _DropOutIPAddress;  }
	unsigned short GetDropOutPortNumber() { return _DropOutPortNumber; }
	bool Failed() { return _Error; }
	string IPtoString(unsigned long ipAddress);
	unsigned long GetBroadcastAddress();
	bool SendBroadCastData(unsigned short destPort, char * data, size_t dataLength);
};
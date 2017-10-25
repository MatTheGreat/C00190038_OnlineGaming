#pragma once#pragma once

#include <string>
#include <WS2tcpip.h>						// Header file for Winsock functions
#pragma comment(lib, "ws2_32.lib")			// Winsock library file

#define MAX_BUFFER_SIZE (49152)

class tcpListener;

typedef void(*MessageRecievedHandler)(tcpListener* listener, int socketId, std::string msg);

class tcpListener
{
public:
	tcpListener(std::string ipAddress, int port, MessageRecievedHandler handler);

	~tcpListener();

	bool Init();

private:

};


#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "TcpListener.h"
#include <sstream>



CTcpListener::CTcpListener(std::string ipAddress, int port, MessageRecievedHandler handler)
	: m_ipAddress(ipAddress), m_port(port), MessageReceived(handler)
{
	
	m_ipAddress = ipAddress;
	m_port = port;

	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
	return;
	}

	// Create a socket
	m_listening = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listening == INVALID_SOCKET)
	{
	return;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	int hintlen = sizeof(hint);
	hint.sin_addr.s_addr = inet_addr(m_ipAddress.c_str());
	hint.sin_port = htons(m_port);
	hint.sin_family = AF_INET;

	bind(m_listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening
	listen(m_listening, SOMAXCONN);

	FD_ZERO(&master);

	FD_SET(m_listening, &master);

	running = true;
	
}

CTcpListener::~CTcpListener()
{
	Cleanup();
}

// Send a message to the specified client
void CTcpListener::Send(int clientSocket, std::string msg)
{
	send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}

// Initialize winsock
bool CTcpListener::Init()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);
	// TODO: Inform caller the error that occured
	
	return wsInit == 0;

	/*

	*/
}

// The main processing loop
void CTcpListener::Run()
{
	/*
	char buf[MAX_BUFFER_SIZE];

	while (true)
	{
		// Create a listening socket
		SOCKET listening = CreateSocket();
		if (listening == INVALID_SOCKET)
		{
			break;
		}

		SOCKET client = WaitForConnection(listening);
		if (client != INVALID_SOCKET)
		{
			closesocket(listening);

			int bytesReceived = 0;
			do
			{
				ZeroMemory(buf, MAX_BUFFER_SIZE);

				bytesReceived = recv(client, buf, MAX_BUFFER_SIZE, 0);
				if (bytesReceived > 0)
				{
					if (MessageReceived != NULL)
					{
						MessageReceived(this, client, std::string(buf, 0, bytesReceived));
					}
				}

			} while (bytesReceived > 0);
			
			closesocket(client);
		}
	}
	*/

	while (running)
	{
		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		// SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

		fd_set copy = master;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == m_listening)
			{
				// Accept a new connection
				SOCKET client = accept(m_listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				std::string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// Check to see if it's a command. \quit kills the server
					if (buf[0] == '\\')
					{
						// Is the command quit? 
						std::string cmd = std::string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}

						// Unknown command
						continue;
					}

					// Send message to other clients, and definiately NOT the listening socket

					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != m_listening && outSock != sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							std::string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

		
}

void CTcpListener::Cleanup()
{
	WSACleanup();
}

// Create a socket
SOCKET CTcpListener::CreateSocket()
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening != INVALID_SOCKET)
	{
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &hint.sin_addr);

		int bindOk = bind(listening, (sockaddr*)&hint, sizeof(hint));
		if (bindOk != SOCKET_ERROR)
		{
			int listenOk = listen(listening, SOMAXCONN);
			if (listenOk == SOCKET_ERROR)
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}

	return listening;
}

// Wait for a connection
SOCKET CTcpListener::WaitForConnection(SOCKET listening)
{
	SOCKET client = accept(listening, NULL, NULL);
	return client;
}
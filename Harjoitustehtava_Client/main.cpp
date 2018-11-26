#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>

#include <string>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 50000

void ConnectionShutdowner(SOCKET ConnectSocket)
{
	int iResult;
	char buffer[DEFAULT_BUFLEN];

	do {
		iResult = recv(ConnectSocket, buffer, DEFAULT_BUFLEN, 0);
		if (iResult < 0)
		{
			std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
		}

	} while (iResult > 0);
}

int main(int argc, char **argv)
{
	char server[DEFAULT_BUFLEN] = "localhost";
	char buffer[DEFAULT_BUFLEN];

	WSADATA wsaData;

	SOCKET FirstConnectSocket = INVALID_SOCKET;
	SOCKET ConnectSocket = INVALID_SOCKET;

	addrinfo *result = nullptr;
	addrinfo *ptr = nullptr;
	addrinfo hints;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed with error: " << iResult << std::endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(server, std::to_string(DEFAULT_PORT).c_str(), &hints, &result);
	if (iResult != 0)
	{
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		FirstConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (FirstConnectSocket == INVALID_SOCKET)
		{
			std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(FirstConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(FirstConnectSocket);
			FirstConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (FirstConnectSocket == INVALID_SOCKET)
	{
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return 1;
	}

	iResult = recv(FirstConnectSocket, buffer, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
	}
	else if (iResult == 0)
	{
		std::cout << "Connection closed" << std::endl;
	}
	else
	{
		std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
	}

	std::thread connectShutdown(ConnectionShutdowner, FirstConnectSocket);

	// Port resolved

	// Resolve the server address and new port
	iResult = getaddrinfo(server, buffer, &hints, &result);
	if (iResult != 0)
	{
		std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET)
		{
			std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET)
	{
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		return 1;
	}

	iResult = recv(ConnectSocket, buffer, DEFAULT_BUFLEN, 0);
	if (iResult > 0)
	{
		std::cout << "Server: ";
		std::cout.write(buffer, iResult);
		std::cout << std::endl;
	}
	else if (iResult == 0)
	{
		std::cout << "Connection closed" << std::endl;
	}
	else
	{
		std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
	}

	std::fill(buffer, buffer+DEFAULT_BUFLEN, 0);
	std::cin >> buffer;


	// Send an initial buffer
	iResult = send(ConnectSocket, buffer, (int)strlen(buffer), 0);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, buffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			std::cout << "Server: ";
			std::cout.write(buffer, iResult);
			std::cout << std::endl;
		}
		else if (iResult == 0)
		{
			std::cout << "Connection closed" << std::endl;
		}
		else
		{
			std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
		}

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	connectShutdown.join();
	WSACleanup();

	system("pause");

	return 0;
}

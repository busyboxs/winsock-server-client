#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

#pragma comment (lib, "Ws2_32.lib")


int main()
{
	// Initializing Winsock
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed:" << iResult << std::endl;
		system("pause");
		return 1;
	}

	// Create a socket
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;  // for ipv4
	hints.ai_socktype = SOCK_STREAM;  // for tcp
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		system("pause");
		return 1;
	}

	// Bind the socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	freeaddrinfo(result);

	// Listen on the socket for a client
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	//Accept a connection from a client
	SOCKET ClientSocket;
	ClientSocket = INVALID_SOCKET;

	sockaddr_in client;
	int clientSize = sizeof(client);

	ClientSocket = accept(ListenSocket, (sockaddr*)&client, &clientSize);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "accept failed: " << WSAGetLastError() << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Service (i.e. port) the client is connect on

	ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << " connected on port " << service << std::endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
	}

	closesocket(ListenSocket);

	// Receive and send data
	char recvbuf[DEFAULT_BUFLEN];
	char sentbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "\nByte received: " << iResult << std::endl;
			std::cout << "String received:\n\t" << std::string(recvbuf, 0, iResult) << std::endl;

			strcpy_s(sentbuf, "Hi, I am server. I have received your message.");
			iResult = (int)strlen(sentbuf);

 			iSendResult = send(ClientSocket, sentbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				WSACleanup();
				system("pause");
				return 1;
			}
			std::cout << "\nByte sent: " << iSendResult << std::endl;
			std::cout << "String sent:\n\t" << sentbuf << std::endl;
		}
		else if (iResult == 0)
			std::cout << "\nConnection closing..." << std::endl;
		else {
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			system("pause");
			return 1;
		}
	} while (iResult > 0);

	// Disconnect
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed:" << WSAGetLastError() << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(ClientSocket);
	WSACleanup();

	system("pause");
	return 0;
}


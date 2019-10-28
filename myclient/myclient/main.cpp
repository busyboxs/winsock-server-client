#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int main(int argc, char** argv) {

	if (argc != 2) {
		std::cout << "usage: " << argv[0] << "server-name" << std::endl;
		system("pause");
		return 1;
	}
	
	// 1. Initialize Winsock
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		system("pause");
		return 1;
	}

	// 2. Create a socket
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed: " << iResult << std::endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		/* Create a SOCKET for connecting to server*/
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
			freeaddrinfo(result);
			WSACleanup();
			system("pause");
			return 1;
		}

		// 3. Connect to the server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	// 4. Send and receive data
	int recvbuflen = DEFAULT_BUFLEN;

	const char *sendbuf = "Hello, I am client, can you receive my message?";
	char recvbuf[DEFAULT_BUFLEN];

	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	std::cout << "Byte sent: " << iResult << std::endl;
	std::cout << "String sent:\n\t" << sendbuf << std::endl; 

	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			std::cout << "\nBytes received: " << iResult << std::endl;
			std::cout << "String received:\n\t" << std::string(recvbuf, 0, iResult) << std::endl;
		}
		else if (iResult == 0)
			std::cout << "\nConnection closed" << std::endl;
		else
		{
			std::cout << "recv failed: " << WSAGetLastError() << std::endl;
		}
	} while (iResult > 0);

	// 5. Disconnect
	closesocket(ConnectSocket);
	WSACleanup();
	
	system("pause");
	return 0;
}
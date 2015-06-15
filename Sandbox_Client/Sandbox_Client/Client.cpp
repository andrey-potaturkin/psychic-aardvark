#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define DEFAULT_ADDRESS "10.31.36.137"
#define DEFAULT_PORT "4242"
#define DEFAULT_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")

int main() {

	char errorbuf[DEFAULT_BUFLEN];

	WSADATA wsaData;

	int iResult;
	int iSendResult;

	// Initialize Winsock

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		scanf_s(errorbuf);
		return 1;
	}

	// Client socket creation
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(DEFAULT_ADDRESS, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		scanf_s(errorbuf);
		WSACleanup();
		return 1;
	}

	// Creating a socket
	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		scanf_s(errorbuf);
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		scanf_s(errorbuf);
		WSACleanup();
		return 1;
	}

	// Sending and receiving data
	int recvbuflen = DEFAULT_BUFLEN;

	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];

	// Receive the greeting
	/*iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	printf("%.*s\n", iResult, recvbuf);*/
	
	// Send an initial buffer
	/*iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		scanf_s(errorbuf);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	/*iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		scanf_s(errorbuf);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}*/

	// Receive data until the server closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			printf("%.*s\n", iResult, recvbuf);

			scanf_s("%9s", sendbuf, _countof(sendbuf));
			iSendResult = send(ConnectSocket, sendbuf, strlen(sendbuf), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				scanf_s(errorbuf);
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);


	scanf_s(errorbuf);
	return 0;
}
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define DEFAULT_PORT "4242"
#define DEFAULT_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")

int notmain() {

	WSADATA wsaData;

	int iResult;
	char errorbuf[DEFAULT_BUFLEN];

	// Initialize Winsock

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	// Server socket creation

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	// Create a SOCKET for the server to listen for client connections

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	// Start listening on a socket
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("Awaiting connection\n");
	SOCKET ClientSocket;

	ClientSocket = INVALID_SOCKET;

	while (true) {

		// Accepting a client connection
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		printf("Connection established\n");

		// Greeting
		char recvbuf[DEFAULT_BUFLEN];
		int iSendResult;
		int recvbuflen = DEFAULT_BUFLEN;
		char greetbuf[DEFAULT_BUFLEN] = "Hello";

		iSendResult = send(ClientSocket, greetbuf, strlen(greetbuf), 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

		// Receive until the peer shuts down the connection
		do {
				
			iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("Bytes received: %d\n", iResult);
				printf("%.*s\n", iResult, recvbuf);
				
				// Echo the buffer back to the sender
				iSendResult = send(ClientSocket, recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					scanf_s(recvbuf);
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				printf("Bytes sent: %d\n", iSendResult);

				/*iResult = shutdown(ClientSocket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					printf("shutdown failed: %d\n", WSAGetLastError());
					scanf_s(recvbuf);
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				closesocket(ClientSocket);*/

			}
			else if (iResult == 0) {
				printf("Connection closing...\n");
				scanf_s(recvbuf);
				closesocket(ClientSocket);
			}
			else {
				printf("recv failed: %d\n", WSAGetLastError());
				scanf_s(recvbuf);
				closesocket(ClientSocket);
			}

		} while (iResult > 0);
	}

	scanf_s(errorbuf);
	return 0;
}
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define DEFAULT_PORT "4242"
#define DEFAULT_BUFLEN 512
#define CLIENT_SOCKET_COUNT 4

#pragma comment(lib, "Ws2_32.lib")

int main() {

	WSADATA wsaData;

	int iResult;
	int connected_clients = 0;
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

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result); // !!!!!!!!!!!!!!!!!!!!!! another IP
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
	SOCKET ClientSocket[CLIENT_SOCKET_COUNT];

	for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) ClientSocket[i] = INVALID_SOCKET;

	SOCKET ExtraSocket = INVALID_SOCKET;
	
	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char greetbuf[DEFAULT_BUFLEN] = "Hello\n";
	char extrabuf[DEFAULT_BUFLEN] = "No available sockets\n";

	bool input_sockets[CLIENT_SOCKET_COUNT];
	for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) input_sockets[i] = false;

	bool closed_sockets_num[CLIENT_SOCKET_COUNT];
	for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) closed_sockets_num[i] = false;

	bool client_pending = false;
	bool input_available = false;
	bool socket_closed = false;

	
	// Main loop
	while (true) {
				
		for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) {
			input_sockets[i] = false;
			closed_sockets_num[i] = false;
		}

		input_available = false;
		socket_closed = false;
		client_pending = false;

		fd_set fd_read_set;

		FD_ZERO(&fd_read_set);
		FD_SET(ListenSocket, &fd_read_set);
		for (int i = 0; i < CLIENT_SOCKET_COUNT; i++)
		{
			if (ClientSocket[i] != INVALID_SOCKET)
				FD_SET(ClientSocket[i], &fd_read_set);
		}
		
		
		// Select call
		iResult = select(NULL, &fd_read_set, NULL, NULL, NULL);
		if (iResult == SOCKET_ERROR) {
			printf("select failed: %d\n", WSAGetLastError());
			scanf_s(recvbuf);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		
		client_pending = FD_ISSET(ListenSocket, &fd_read_set);
	

		for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) {
			if (FD_ISSET(ClientSocket[i], &fd_read_set)) {

				unsigned long ioresult = 0;
				iResult = ioctlsocket(ClientSocket[i], FIONREAD, &ioresult);
				if (iResult != 0) {
					printf("ioctlsocket failed: %d\n", WSAGetLastError());
					scanf_s(recvbuf);
					closesocket(ClientSocket[i]);
				}

				if (ioresult != 0) {
					input_available = true;
					input_sockets[i] = true;
					continue;
				}

				socket_closed = true;
				closed_sockets_num[i] = true;
			}
		}

		if (client_pending) {
			if (connected_clients == CLIENT_SOCKET_COUNT) {
				ExtraSocket = accept(ListenSocket, NULL, NULL);
				if (ExtraSocket == INVALID_SOCKET) {
					printf("extra accept failed: %d\n", WSAGetLastError());
					scanf_s(recvbuf);
					closesocket(ExtraSocket);
					WSACleanup();
					return 1;
				}

				iSendResult = send(ExtraSocket, extrabuf, strlen(extrabuf), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ExtraSocket);
					WSACleanup();
					return 1;
				}

				shutdown(ExtraSocket, SD_BOTH);
				ExtraSocket = INVALID_SOCKET;
			}

			int i = 0;
			while (i < CLIENT_SOCKET_COUNT) {
				if (ClientSocket[i] == INVALID_SOCKET) {

					ClientSocket[i] = accept(ListenSocket, NULL, NULL);
					if (ClientSocket[i] == INVALID_SOCKET) {
						printf("accept failed: %d\n", WSAGetLastError());
						i++;
						continue;
					}
					printf("Connection established on socket %d\n", i);

					iSendResult = send(ClientSocket[i], greetbuf, strlen(greetbuf), 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed: %d\n", WSAGetLastError());
						closesocket(ClientSocket[i]);
						WSACleanup();
						return 1;
					}
					
					connected_clients++;

					break;
				}
				
				i++;
			}
			
		}
		
		if (input_available) {
			for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) {
				if (input_sockets[i]) {
					iResult = recv(ClientSocket[i], recvbuf, recvbuflen, 0);
					if (iResult > 0) {
						printf("Bytes received: %d\n", iResult);
						printf("%.*s\n", iResult, recvbuf);

						// Echo the buffer back to the sender
						iSendResult = send(ClientSocket[i], recvbuf, iResult, 0);
						if (iSendResult == SOCKET_ERROR) {
							printf("send failed: %d\n", WSAGetLastError());
							scanf_s(recvbuf);
							closesocket(ClientSocket[i]);
							WSACleanup();
							return 1;
						}
						printf("Bytes sent: %d\n", iSendResult);
					}
				}
			}
		}
		
		if (socket_closed) {
			for (int i = 0; i < CLIENT_SOCKET_COUNT; i++) {
				if (closed_sockets_num[i] == true) {
					printf("Connection closed on socket %d\n", i);
					shutdown(ClientSocket[i], SD_BOTH);
					ClientSocket[i] = INVALID_SOCKET;

					connected_clients--;

				}
			}
		}

		
	}

	scanf_s(errorbuf);
	return 0;
}
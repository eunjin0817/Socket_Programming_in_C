#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
void ErrorHandling(char* msg);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN sendAddr;

	WSABUF dataBuf; // Information of data being sent.
	char msg[] = "Network is Computer!";
	int sendBytes = 0; // Variable storing the number of bytes of data transferred.

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped; // Be used for checking completion of data sending.
	// WSAOVERLAPPED is referred from OS during overlapped IO

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// Create socket of  non-blocking mode in overlapped IO.
	sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAddr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	evObj = WSACreateEvent(); // Create Event object(= Kernel object).
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;  // Event Object is to the signaled state when IO is completed.
	dataBuf.len = strlen(msg) + 1;
	dataBuf.buf = msg;

	if (WSASend(sock, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) { // If data is still transferring after WSASend() returns
			puts("Background data send");
			
			// Since Event object becomes to the signaled state if data transfer completly,
			// it's possible to wait with this function untill then. 
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			
			// Check the actual size of data transferred.
			WSAGetOverlappedResult(sock, &overlapped, &sendBytes, FALSE, NULL);
		}
		else {
			ErrorHandling("WSASend() error");
		}
	}

	printf("Send data size: %d\n", sendBytes);
	WSACloseEvent(evObj);
	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char* msg);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET listenSock, recvSock;
	SOCKADDR_IN listenAddr, recvAddr;
	int recvAddrSz;

	WSABUF dataBuf;  // Information of data being received.
	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	char buf[BUF_SIZE];
	int recvBytes = 0, flags = 0;
	
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	listenSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSock == INVALID_SOCKET)
		ErrorHandling("WSASocket() error");

	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(atoi(argv[1]));

	if (bind(listenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(listenSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAddrSz = sizeof(recvAddr);
	recvSock = accept(listenSock, (SOCKADDR*)&recvAddr, &recvAddrSz);

	evObj = WSACreateEvent(); // Create Event object(= Kernel object).
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj; // Event Object is to the signaled state when IO is completed.
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;

	if (WSARecv(recvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) { // If data is still transferring after WSARecv() returns
			puts("Background data receive");

			// Since Event object becomes to the signaled state if data transfer completly,
			// it's possible to wait with this function untill then
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);

			// Check the actual size of data transferred.
			WSAGetOverlappedResult(recvSock, &overlapped, &recvBytes, FALSE, NULL);
		}
		else {
			ErrorHandling("WSARecv() error");
		}
	}

	printf("Received message: %s\n", buf);
	WSACloseEvent(evObj);
	closesocket(recvSock);
	closesocket(listenSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
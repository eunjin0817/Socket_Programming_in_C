#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define BUF_SIZE 1024
void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* msg);

WSABUF dataBuf;
char buf[BUF_SIZE];
int recvBytes = 0;

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET listenSock, recvSock;
	SOCKADDR_IN listenAddr, recvAddr;

	WSAOVERLAPPED overlapped;
	WSAEVENT evObj;

	int idx, recvAddrSz, flags = 0;
	
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
	if (recvSock == INVALID_SOCKET)
		ErrorHandling("accept() error");

	memset(&overlapped, 0, sizeof(overlapped));
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;

	if (WSARecv(recvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR)
		if (WSAGetLastError() == WSA_IO_PENDING)
			puts("Background data receive");

	evObj = WSACreateEvent(); // dummy event object for using to call below function

	// For main thread to be alertable wait state.
	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE);
	if (idx == WAIT_IO_COMPLETION)
		puts("Overlapped I/O Completed");
	else  // if error occurred!
		ErrorHandling("WSARecv() error");

	WSACloseEvent(evObj);
	closesocket(recvSock);
	closesocket(listenSock);
	WSACleanup();
	return 0;
}

void CALLBACK CompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	if (dwError != 0) {
		ErrorHandling("CompRoutine error");
	}
	else {
		recvBytes = szRecvBytes;
		printf("Received message: %s\n", buf);
	}
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
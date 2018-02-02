#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define BUF_SIZE 1024
void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* msg);

typedef struct {
	SOCKET clntSock;
	char buf[BUF_SIZE];
	WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET listenSock, recvSock;
	SOCKADDR_IN listenAddr, recvAddr;
	LPWSAOVERLAPPED lpOvLp;
	DWORD recvBytes;
	LPPER_IO_DATA hbInfo;
	int mode = 1, recvAddrSz, flagInfo = 0; // mode -> 0 : blocking, 1 : non-blocking

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// Create a server socket doing overlapped I/O in non-blocking mode.
	listenSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ioctlsocket(listenSock, FIONBIO, &mode); // Control I/O working of socket in order to become the non-blocking mode.

	memset(&listenAddr, 0, sizeof(listenAddr));
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(atoi(argv[1]));

	if (bind(listenSock, (SOCKADDR*)&listenAddr, sizeof(listenAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(listenSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAddrSz = sizeof(recvAddr);
	while (1) {
		SleepEx(100, TRUE); // Second parameter means main thread to become alertable wait state.

		// Since ioctlsocket() was called, new socket for data transmission created from accept() works in non-blocking mode.
		recvSock = accept(listenSock, (SOCKADDR*)&recvAddr, &recvAddrSz);
		if (recvSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) // When accept() function is executed without taking connecting request from a client
				continue;
			else
				ErrorHandling("accept() error");
		}
		puts("Client connected..................");
		
		// Because WSAOVERLAPPED structure is required to communicate with individual client,  Initialize.
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->clntSock = recvSock; // Save handle of socket for receiving. This socket use saved buffer of next line in I/O.
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// Indeed Event object is unnecessary, so  other thing can be stored.
		lpOvLp->hEvent = (HANDLE)hbInfo; // In this case, information of each client is saved to transfer it into complete routine.
		
		// lpOvLp is affored as third parameter of ReadCompRoutine function(Completion Routine).
		// So completion routine enable to access to handle and buffer of socket completing I/O.
		WSARecv(recvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);

	}

	closesocket(recvSock);
	closesocket(listenSock);
	WSACleanup();
	return 0;
}

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET sock = hbInfo->clntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD sentBytes;

	if (szRecvBytes == 0) { // when received EOF
		closesocket(sock);
		free(lpOverlapped->hEvent);
		free(lpOverlapped);
		puts("Client disconnected.............");
	}
	else {
		bufInfo->len = szRecvBytes;
		WSASend(sock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine); //echo
	}
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped);
	SOCKET sock = hbInfo->clntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo = 0;
	WSARecv(sock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

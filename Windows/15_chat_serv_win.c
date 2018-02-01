#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char* msg, int len);
void ErrorHandling(char* msg);

int clntCnt = 0;
SOCKET clntSocks[MAX_CLNT];
HANDLE mutex;

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET servSock, clntSock;
	SOCKADDR_IN servAddr, clntAddr;
	int clntAddrSz;
	HANDLE hThread;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	mutex = CreateMutex(NULL, FALSE, NULL);
	servSock = socket(PF_INET, SOCK_STREAM, 0);
	if (servSock == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(servSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		clntAddrSz = sizeof(clntAddr);
		clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &clntAddrSz);
		if (clntSock == INVALID_SOCKET)
			ErrorHandling("accept() erorr");

		// Mutex becomes  to the non-signaled again when non-signaled -> signaled
		WaitForSingleObject(mutex, INFINITE);  // gain ownership of mutex
		clntSocks[clntCnt++] = clntSock;
		ReleaseMutex(mutex); // non-signaled  ---> signaled : return mutex.

		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&clntSock, 0, NULL);
		printf("Connected client IP: %s\n", inet_ntoa(clntAddr.sin_addr));
	}
	closesocket(servSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI HandleClnt(void* arg) {
	SOCKET clntSock = *((SOCKET*)arg);
	int strLen = 0, i;
	char msg[BUF_SIZE];

	while ((strLen = recv(clntSock, msg, sizeof(msg), 0)) != 0)
		SendMsg(msg, strLen); // Send to all

	WaitForSingleObject(mutex, INFINITE);
	for (i = 0; i < clntCnt; i++) { // remove disconnected client after sending message
		if (clntSock == clntSocks[i]) {
			while (i++ < clntCnt - 1)
				clntSocks[i] = clntSocks[i + 1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(mutex);
	closesocket(clntSock);
	return 0;
}

void SendMsg(char* msg, int len) {
	int i;
	WaitForSingleObject(mutex, INFINITE);
	for (i = 0; i < clntCnt; i++)
		send(clntSocks[i], msg, len, 0); // send to all clients
	ReleaseMutex(mutex);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
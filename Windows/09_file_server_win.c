#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET servSock, clntSock;
	FILE *fp;
	char buf[BUF_SIZE];
	int readCnt;

	SOCKADDR_IN servAddr, clntAddr;
	int clntAddrSz;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	fp = fopen("file_server_win.c", "rb");
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

	clntAddrSz = sizeof(clntAddr);
	clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &clntAddrSz);

	while (1) {
		readCnt = fread((void*)buf, 1, BUF_SIZE, fp);
		if (readCnt < BUF_SIZE) {
			send(clntSock, (char*)buf, readCnt, 0);
			break;
		}
		send(clntSock, (char*)buf, BUF_SIZE, 0);
	}

	shutdown(clntSock, SD_SEND); // SD_RECEIVE: 입력 스트림 종료, SD_SEND: 출력 스트림 종료, SD_BOTH
	recv(clntSock, buf, BUF_SIZE, 0);
	printf("Message from client: %s\n", buf);

	fclose(fp);
	closesocket(clntSock);
	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\m', stderr);
	exit(1);
}
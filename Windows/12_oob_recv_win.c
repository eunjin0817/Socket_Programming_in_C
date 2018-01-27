#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define BUF_SIZE 30
void ErrorHandling(char** message);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET acptSock, recvSock;

	SOCKADDR_IN recvAddr, sendAddr;
	int sendAddrSize, strLen;
	char buf[BUF_SIZE];
	int result;

	// In Windows, It's impossible to handle signal. Instead, select() function is used.
	fd_set read, except, readCopy, exceptCopy;
	struct timeval timeout;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	acptSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAddr.sin_port = htons(atoi(argv[1]));

	if (bind(acptSock, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(acptSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	sendAddrSize = sizeof(sendAddr);
	recvSock = accept(acptSock, (SOCKADDR_IN*)&sendAddr, &sendAddrSize);

	FD_ZERO(&read);
	FD_ZERO(&except);
	FD_SET(recvSock, &read);
	FD_SET(recvSock, &except);

	while (1) {
		readCopy = read;
		exceptCopy = except;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);

		if (result > 0) {
			if (FD_ISSET(recvSock, &exceptCopy)) {
				strLen = recv(recvSock, buf, BUF_SIZE - 1, MSG_OOB);
				buf[strLen] = 0;
				printf("Urgent message : %s\n", buf);
			}

			if (FD_ISSET(recvSock, &readCopy)) {
				strLen = recv(recvSock, buf, BUF_SIZE - 1, 0);
				if (strLen = 0) {
					closesocket(recvSock);
					break;
				}
				else {
					buf[strLen] = 0;
					puts(buf);
				}
			}
		}
	}
	closesocket(acptSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char** message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN sendAddr;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sendAddr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == SOCKET_ERROR)
		ErrorHandling("connect() error");

	send(sock, "123", 3, 0);
	send(sock, "4", 1, MSG_OOB);
	send(sock, "567", 3, 0);
	send(sock, "8", 1, MSG_OOB);

	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char **message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
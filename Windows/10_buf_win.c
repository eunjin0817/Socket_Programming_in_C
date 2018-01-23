#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
void ErrorHandling(char *message);
void ShowSocketBufSize(SOCKET sock);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET sock;
	int sndBuf, rcvBuf, state;
	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		ErrorHandling("socket() error");
	ShowSocketBufSize(sock);

	sndBuf = 1024 * 3;
	rcvBuf = 1024 * 3;
	state = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuf, sizeof(sndBuf));
	if (state == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	state = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, sizeof(rcvBuf));
	if (state == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	ShowSocketBufSize(sock);
	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void ShowSocketBufSize(SOCKET sock) {
	int sndBuf, rcvBuf, state, len;

	len = sizeof(sndBuf);
	state = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuf, &len);
	if (state == SOCKET_ERROR)
		ErrorHandling("getsockopt() error");

	len = sizeof(rcvBuf);
	state = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, &len);
	if (state == SOCKET_ERROR)
		ErrorHandling("getsockopt() error");

	printf("Input buffer size : %d\n", rcvBuf);
	printf("Output buffer size : %d\n", sndBuf);
}
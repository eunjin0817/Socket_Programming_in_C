#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <WS2tcpip.h>  // for struct ip_mreq

#define BUF_SIZE 30
void ErrorHandling(char** message);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET recvSock;
	SOCKADDR_IN recvAddr;
	struct ip_mreq joinAddr;
	char buf[BUF_SIZE];
	int strLen;

	if (argc != 3) {
		printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	recvSock = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	recvAddr.sin_port = htons(atoi(argv[2]));

	if (bind(recvSock, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// Initialize struct ip_mreq for joining multicast group.
	joinAddr.imr_multiaddr.s_addr = inet_addr(argv[1]);
	joinAddr.imr_interface.s_addr = htonl(INADDR_ANY);
	// Set the option(multicast group IP address) of socket.
	if (setsockopt(recvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(void*)&joinAddr, sizeof(joinAddr)) == SOCKET_ERROR)
		ErrorHandling("setsockopt() error");

	while (1) {
		strLen = recvfrom(recvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0) break;
		buf[strLen] = 0;
		fputs(buf, stdout);
	}
	closesocket(recvSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char** message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
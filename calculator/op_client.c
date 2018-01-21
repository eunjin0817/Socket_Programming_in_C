#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char* argv[]) {
	int sock;
	int strLen, recvLen, recvCnt;
	char message[BUF_SIZE];
	struct sockaddr_in servAddr;

	if (argc != 3) {
		printf("Usage : %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(argv[1]);
	servAddr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
		error_handling("connect() error");
	else
		puts("Connected ................");

	printf("Operand Count: ");
	fgets(message, BUF_SIZE, stdin);
	write(sock, message, BUF_SIZE-1);
	int count = atoi(message);
	for (int i = 1; i <= count; i++) {
		printf("Operand %d: ", i);
		fgets(message, BUF_SIZE, stdin);
		write(sock, message, BUF_SIZE-1);
	}
	printf("Operator: ");
	fgets(message, BUF_SIZE, stdin);
	write(sock, message, BUF_SIZE-1);

	recvLen = read(sock, &message[recvLen], BUF_SIZE-1);
	if (recvLen == -1)
		error_handling("read() error");
	message[recvLen] = 0;
	printf("Operation result: %s\n", message);
	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}	

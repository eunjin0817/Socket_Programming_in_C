#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char* argv[]) {
	int servSock, clntSock;
	int strLen;
	char message[BUF_SIZE];

	struct sockaddr_in servAddr, clntAddr;
	socklen_t clntAddrSize;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	servSock = socket(PF_INET, SOCK_STREAM, 0);
	if (servSock == -1)
		error_handling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
		error_handling("bind() error");

	if (listen(servSock, 5) == -1)
		error_handling("listen() error");

	clntAddrSize = sizeof(clntAddr);
	
	for (int i = 0; i < 5; i++) {
		clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrSize);
		if (clntSock == -1)
			error_handling("accept() error");
		else
			printf("Connected Client %d.\n", i+1);

		strLen = read(clntSock, message, BUF_SIZE);
		int result=0, cnt = atoi(message);
		char operand[BUF_SIZE];
		operand[0] = 0;
		int len = 0;
		while (cnt--) {
			strLen = read(clntSock, message, BUF_SIZE);
			strcat(operand, message);
			int msg_len = strlen(message);
			len += msg_len;
			operand[len-1] = ' ';
			operand[len] = 0;
		}
		read(clntSock, message, BUF_SIZE);
		sleep(1);
		char tmp[BUF_SIZE];
		strcpy(tmp, operand);
		char* ptr = strtok(tmp, " ");
		result = atoi(ptr);
		ptr = strtok(NULL, " ");
		while (ptr != NULL) {
			printf("result : %d", result);
			int num = atoi(ptr);
			switch(message[0]) {
				case '+':
					result += num;
					break;
				case '-':
					result -= num;
					break;
				case 'x':
					result *= num;
					break;
			}
			ptr = strtok(NULL, " ");
		}
		sprintf(message, "%d", result);
		len = strlen(message);
		write(clntSock, message, len);
		close(clntSock);
	}
	close(servSock);
	return 0;
}
void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

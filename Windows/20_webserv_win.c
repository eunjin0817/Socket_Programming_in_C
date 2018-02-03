#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <process.h>

#define BUF_SIZE 2048
#define BUF_SMALL 100

unsigned WINAPI RequestHandler(void* arg);
char* ContentType(char* file);
void SendData(SOCKET sock, char* ct, char* fileName);
void SendErrorMSG(SOCKET sock);
void ErrorHandling(char* msg);

int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET servSock, clntSock;
	SOCKADDR_IN servAddr, clntAddr;

	HANDLE hThread;
	DWORD dwThreadID;
	int clntAddrSz;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

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

	// Request & Response
	while (1) {
		clntAddrSz = sizeof(clntAddr);
		clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &clntAddrSz);
		printf("Connection Request : %s:%d\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
		hThread = (HANDLE)_beginthreadex(NULL, 0, RequestHandler, (void*)clntSock, 0, (unsigned*)&dwThreadID);
	}
	closesocket(servSock);
	WSACleanup();
	return 0;
}

unsigned WINAPI RequestHandler(void* arg) {
	SOCKET clntSock = (SOCKET)arg;
	char buf[BUF_SIZE];
	char method[BUF_SMALL];
	char ct[BUF_SMALL];
	char fileName[BUF_SMALL];

	recv(clntSock, buf, BUF_SIZE, 0);
	if (strstr(buf, "HTTP/") == NULL) { // Check whether request from HTTP or not.
		SendErrorMSG(clntSock);
		closesocket(clntSock);
		return 1;
	}

	strcpy(method, strtok(buf, " /"));
	if (strcmp(method, "GET")) // Check whether request of GET method or not.
		SendErrorMSG(clntSock);

	strcpy(fileName, strtok(NULL, " /")); // Extract name of file from HTTP request header.
	strcpy(ct, ContentType(fileName)); // Check Content-type
	SendData(clntSock, ct, fileName); // Response to a Client.
	return 0;
}

char* ContentType(char* file) {
	char extension[BUF_SMALL];
	char fileName[BUF_SMALL];
	strcpy(fileName, file);
	strtok(fileName, ".");
	strcpy(extension, strtok(NULL, "."));
	if (!strcmp(extension, "html") || !strcmp(extension, "html"))
		return "text/html";
	else
		return "text/plain";
}

void SendData(SOCKET sock, char* ct, char* fileName) {
	char protocol[] = "HTTP/1.0 200 OK\r\n";
	char servName[] = "Server:simple web server\r\n";
	char cntLen[] = "Content-length:2048\r\n";
	char cntType[BUF_SMALL];
	char buf[BUF_SIZE];
	FILE* sendFile;

	sprintf(cntType, "Content-type:%s\r\n\r\n", ct);
	if ((sendFile = fopen(fileName, "r")) == NULL) {
		SendErrorMSG(sock);
		return;
	}

	// Send information of header.
	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0);
	send(sock, cntType, strlen(cntType), 0);

	// Send data requested from a client.
	while (fgets(buf, BUF_SIZE, sendFile) != NULL)
		send(sock, buf, strlen(buf), 0);

	closesocket(sock);
}

void SendErrorMSG(SOCKET sock) {
	char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
	char servName[] = "Server:simple web server\r\n";
	char cntLen[] = "Content-length:2048\r\n";
	char cntType[] = "Content-type:text/html\r\n\r\n";
	char content[] = "<html><head><title>NETWORK/title></head><body><font size=+5><br>오류 발생! 요청 파일명 및 요청 방식 확인!</font></body></html>";

	send(sock, protocol, strlen(protocol), 0);
	send(sock, servName, strlen(servName), 0);
	send(sock, cntLen, strlen(cntLen), 0);
	send(sock, cntType, strlen(cntType), 0);
	send(sock, content, strlen(content), 0);
	closesocket(sock);
}

void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}